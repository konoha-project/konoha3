/****************************************************************************
 * Copyright (c) 2012, the Konoha project authors. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/

#include<minikonoha/minikonoha.h>
#include<minikonoha/sugar.h>
#include <minikonoha/bytes.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <sys/time.h>

#if defined(__linux__)
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 27)
#define __USE_LOCAL_PIPE2__
#endif
#endif /* __linux__ */

#include "subproc_resourcemonitor.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !defined (__APPLE__) && !defined(__USE_LOCAL_PIPE2__)
extern int pipe2 (int __pipedes[2], int __flags);
#endif
extern int sigignore (int __sig);

#define MOD_subproc 23

typedef struct {
	KonohaModule h;
	KonohaClass     *cSubproc;
} kmodsubproc_t;

typedef struct {
	KonohaModuleContext h;
} ctxsubproc_t;

typedef struct {
	int mode;                              // the kind of identifier
	FILE* fp;                              // file stream pointer
} pfd_t;                                   // pipe fd structure


typedef struct {
	kbool_t shell;                         // shell mode [true/false]
	kbool_t closefds;                      // closefds   [true/false]
	kbool_t bg;                            // bg mode    [true/false]
	kArray *env;                           // child process environment
	kString *command;                      // child process command
	kString *cwd;                          // child process current working directory
	pfd_t r;                               // child process output identifier
	pfd_t w;                               // child process input identifier
	pfd_t e;                               // child process error identifier
	int cpid;                              // child process ID
	int bufferSize;                        // buffer size (unused)
	int timeout;                           // child process timeout value
	int status;                            // waitpid status
	int timeoutKill;                       // child process Timeout ending flag [true/false]
	SUBPROC_RESOURCEMON_INSTANCE;
} subprocData_t;                           // subproc data structure


typedef const struct _kSubproc kSubproc;
struct _kSubproc {
	KonohaObjectHeader h;
	subprocData_t *spd;
};

/* ------------------------------------------------------------------------ */
/* [class defs] */

#define ctxsubproc         ((ctxsubproc_t*)kctx->mod[MOD_subproc])
#define kmodsubproc        ((kmodsubproc_t*)kctx->modshare[MOD_subproc])
#define IS_defineSubproc() (kctx->modshare[MOD_subproc] != NULL)
#define CT_Subproc         kmodsubproc->cSubproc
#define TY_Subproc         kmodsubproc->cSubproc->typeId

#define IS_Subproc(O)      ((O)->h.ct == CT_Subproc)

/* ------------------------------------------------------------------------ */
/* [global varibals]
 */

static jmp_buf env;

static void alarmHandler(int sig) {
		siglongjmp(env, 1);
}
static int fgPid;
static void keyIntHandler(int sig) { kill(fgPid, SIGINT); }

/* ------------------------------------------------------------------------ */

// child process IO mode
#define M_DEFAULT          -2			// initialization state
#define M_NREDIRECT        -1			// parent process succession
#define M_PIPE             0			// pipe
#define M_STDOUT           1			// standard OUT
#define M_FILE             2			// file

// child process status code
#define S_RUNNING          -300			// running
#define S_PREEXECUTION     -400			// preexecution
#define S_TIMEOUT          -500			// tiomeout
#define S_EXIT             0			// terminate

// subproc macro
#define MAXARGS            128				// the number maximum of parameters for spSplit
#define BUFSIZE            64 * 1024		// the reading buffer size maximum for pipe
#define DELAY              1000				// the adjustment value at the time of signal transmission
#define DEF_TIMEOUT        10 * 1000		// default timeout valx
//#define DEF_TIMEOUT -1
#define ONEXEC(p)          ( (p != NULL) && (p->cpid > 0) ) ? 1 : 0
#define PREEXEC(p)         ( (p != NULL) && (p->cpid == -1) ) ? 1 : 0
#define WORD2INT(val)      (sizeof(val)==8) ? (val&0x7FFFFFFF)|((val>>32)&0x80000000) : val

/* ------------------------------------------------------------------------ */

#if defined (__APPLE__) || defined(__USE_LOCAL_PIPE2__)
// for fg & bg & exec & restart
static int pipe2( int *fd, int flags ) {
	int val;
	int p[2];
	if ( pipe(p) < 0 ) {
		return -1;
	}
	if ( (val=fcntl(p[0], F_GETFL, 0)) < 0 ) {
		goto L_ERR;
	}
	if ( fcntl(p[0], F_SETFL, val | flags) < 0 ) {
		goto L_ERR;
	}
	if ( (val=fcntl(p[1], F_GETFL, 0)) < 0 ) {
		goto L_ERR;
	}
	if ( fcntl(p[1], F_SETFL, val | flags) < 0 ) {
		goto L_ERR;
	}
	fd[0] = p[0];
	fd[1] = p[1];
	return 0;
L_ERR:;
	{
		close(p[0]);
		close(p[1]);
		return -1;
	}
}
#endif

// for argSplit & fg & bg & exec & restart
// ===========================================================================
// <example>
// char str[12], *args[2];
// strcpy(str, "Hello world");
// int param = spSplit( str, args );
// if ( param > 0 ) {
//    printf("param:%d args[0]:%s args[1]:%s\n", param, args[0], args[1]);
// } else {
//     printf("spSplit error[%d]\n", param);
// }
// ---------------------------------------------------------------------------
// output:
//     param:2 args[0]:Hello args[1]:world
// ===========================================================================
/**
 * @return number of parameter (zero or more)
 *         -1 is Internal Error
 *         -2 is the maximum error of the number of parameters
 */
static int spSplit(char* str, char* args[]) {

	if ( (str == NULL) || (args == NULL) ) {
		return -1;
	}
	int indx;
	char *cp = str;
	for (indx = 0; indx <= MAXARGS; indx++) {
		if ( indx == MAXARGS ) {
			return -2;
		} else if ( (args[indx] = strtok(cp, " ")) == NULL ) {
			break;
		} else {
			cp = NULL;
		}
	}
	// number of parameter
	return indx;
}

// for getIN & getOUT & getERR
/**
 * @return "konoha.posix.File" class id
 */
//static KonohaClass khn_getFileClass(KonohaContext *kctx) {
//	char *name = "konoha.posix.File";
//	kbytes_t lname;
//	lname.ubuf = (kchar_t*)name;
//	lname.len = knh_strlen(lname.text);
//	return knh_getcid(kctx, lname);
//}

// for fg & bg & exec & restart
/**
 * @return child process id
 *         -1 is Internal Error
 */

static int knh_popen(KonohaContext *kctx, kString* command, subprocData_t *spd, int defaultMode)
{
	if (IS_NULL(command)) {
		return -1;
	}
	pid_t pid  = -1;
	int rmode = (spd->r.mode==M_DEFAULT) ? defaultMode : spd->r.mode;
	int wmode = (spd->w.mode==M_DEFAULT) ? defaultMode : spd->w.mode;
	int emode = (spd->e.mode==M_DEFAULT) ? defaultMode : spd->e.mode;
	int c2p[2];
	int p2c[2];
	int err[2];
	if(rmode == M_PIPE) {
		if(pipe2(c2p, O_NONBLOCK) != 0) {
			return -1;
		}
	}
	if(wmode == M_PIPE) {
		if(pipe(p2c) != 0) {
			ktrace(_SystemFault,
					KeyValue_s("@", "pipe"),
					KeyValue_u("errno", errno),
					KeyValue_s("errstr", strerror(errno))
			);
			close(c2p[0]); close(c2p[1]);
			return -1;
		}
	}

	if(emode == M_PIPE) {
		if(pipe2(err, O_NONBLOCK) != 0) {
			ktrace(_SystemFault,
					KeyValue_s("@", "pipe"),
					KeyValue_u("errno", errno),
					KeyValue_s("errstr", strerror(errno))
			);
			close(c2p[0]); close(c2p[1]);
			close(p2c[0]); close(p2c[1]);
			return -1;
		}
	}

	SETUP_RESOURCE_MONITOR(spd);

	switch(pid = fork()) {
	case -1:
		// parent process illegal route
		if(rmode == M_PIPE) {
			close(c2p[0]); close(c2p[1]);
		}
		if(wmode == M_PIPE) {
			close(p2c[0]); close(p2c[1]);
		}
		if(emode == M_PIPE) {
			close(err[0]); close(err[1]);
		}
		CLEANUP_RESOURCE_MONITOR(spd);
		break;
	case 0:
		// child process normal route
		SETUP_RESOURCE_MONITOR_FOR_CHILD(spd);
		if(wmode == M_PIPE){
			close(0);
			if (dup2(p2c[0], 0) == -1) {
				ktrace(_SystemFault,
						KeyValue_s("@", "dup2"),
						KeyValue_u("errno", errno),
						KeyValue_s("errstr", strerror(errno))
				);
			}
			close(p2c[0]); close(p2c[1]);
		}
		else if(wmode == M_FILE) {
			close(0);
			if(dup2(fileno(spd->w.fp), 0) == -1) {
				ktrace(_SystemFault,
						KeyValue_s("@", "dup2"),
						KeyValue_u("errno", errno),
						KeyValue_s("errstr", strerror(errno))
				);
			}
		}
		if(rmode == M_PIPE) {
			close(1);
			if(dup2(c2p[1], 1) == -1){
				ktrace(_SystemFault,
						KeyValue_s("@", "dup2"),
						KeyValue_u("errno", errno),
						KeyValue_s("errstr", strerror(errno))
				);
			}
			close(c2p[0]); close(c2p[1]);
		}
		else if(rmode == M_FILE) {
			close(1);
			if(dup2(fileno(spd->r.fp), 1) == -1) {
				ktrace(_SystemFault,
						KeyValue_s("@", "dup2"),
						KeyValue_u("errno", errno),
						KeyValue_s("errstr", strerror(errno))
				);
			}
		}
		if(emode == M_PIPE) {
			close(2);
			dup2(err[1], 2);
			close(err[0]); close(err[1]);
		}
		else if(emode == M_FILE) {
			close(2);
			dup2(fileno(spd->e.fp), 2);
		}
		else if(emode == M_STDOUT) {
			close(2);
			dup2(1, 2);
		}
		if(spd->closefds == 1) {
			// close other than 0, 1, and 2
			int cfd = 3;
			int maxFd = sysconf(_SC_OPEN_MAX);
			do {
				close(cfd);
			} while (++cfd < maxFd);
		}
		setsid(); // separation from tty
		if(!IS_NULL(spd->cwd)) { // TODO!!
			if(chdir(S_text((spd->cwd))) != 0) {
				ktrace(_ScriptFault,
						KeyValue_s("@", "chdir"),
						KeyValue_s("cwd", S_text(spd->cwd)),
						KeyValue_u("errno", errno),
						KeyValue_s("errstr", strerror(errno))
				);
				_exit(1);
			}
		}
		char *args[MAXARGS];
		if(spd->shell == 0) {
			// division of a commnad parameter
			if(spSplit((char*)S_text(command), args) < 0){
				ktrace(_ScriptFault,
						KeyValue_s("@", "spSplit"),
						KeyValue_s("command", S_text(command))
				);
				args[0] = NULL;
			}
		}
		if(!IS_NULL(spd->env)) {
			// division of a environment parameter
			kArray *a = spd->env;
			int num = kArray_size(a);
			char *envs[num+1];
			int i;
			for(i = 0 ; i<num ; i++) {
				envs[i] = (char*)S_text(a->stringItems[i]);
			}
			envs[num] = NULL;
			// exec load new process image if success.
			// if its not, they will return with -1.
			if(spd->shell == 0) {
				if(execve(args[0], args, envs) == -1) {
					ktrace(_SystemFault | _ScriptFault,
							KeyValue_s("@", "execve"),
							KeyValue_u("errno", errno),
							KeyValue_s("errstr", strerror(errno))
					);
				}
			}
			else {
				if (execle("/bin/sh", "sh", "-c", S_text(command), NULL, envs) == -1) {
					ktrace(_SystemFault | _ScriptFault,
							KeyValue_s("@", "execle"),
							KeyValue_u("errno", errno),
							KeyValue_s("errstr", strerror(errno))
					);
				}
			}
		} else {
			if(spd->shell == 0) {
				if(execvp(args[0], args) == -1) {
					ktrace(_SystemFault | _ScriptFault,
							KeyValue_s("@", "execvp"),
							KeyValue_u("errno", errno),
							KeyValue_s("errstr", strerror(errno))
					);
				}
			}
			else {
				if(execlp("sh", "sh", "-c", S_text(command), NULL) == -1) {
					ktrace(_SystemFault | _ScriptFault,
							KeyValue_s("@", "execlp"),
							KeyValue_u("errno", errno),
							KeyValue_s("errstr", strerror(errno))
					);
				}
			}
		}
		perror("knh_popen :");
		_exit(1);
	default:
		// parent process normal route
#if defined(SUBPROC_ENABLE_RESOURCEMONITOR)
		ATTACH_RESOURCE_MONITOR_FOR_CHILD(spd, pid);
//		size_t mem = FETCH_MEM_FROM_RESOURCE_MONITOR(spd);
//		fprintf(stderr, "menusage:%.1fM\n", (double)mem / (1024.0 * 1024.0));
		CLEANUP_RESOURCE_MONITOR(spd);
#endif
		if(rmode == M_PIPE) {
			spd->r.fp = fdopen(c2p[0], "r");
			close(c2p[1]);
		}
		if(wmode == M_PIPE) {
			spd->w.fp = fdopen(p2c[1], "w");
			close(p2c[0]);
		}
		if(emode == M_PIPE) {
			spd->e.fp = fdopen(err[0], "r");
			close(err[1]);
		}
	}
	return pid;
}


// for wait & fg & exec & communicate
/**
 * @return termination status of a child process
 */
static int knh_wait(KonohaContext *kctx, int pid, int bg, int timeout, int *status ) {

#ifndef __APPLE__
	__sighandler_t alarm_oldset  = SIG_ERR;
	__sighandler_t keyInt_oldset = SIG_ERR;
	__sighandler_t ret = SIG_ERR;
#else
	sig_t alarm_oldset  = SIG_ERR;
	sig_t keyInt_oldset = SIG_ERR;
	sig_t ret = SIG_ERR;
#endif
	if(timeout > 0) {
		if(sigsetjmp(env, 1)) {
			// wait timeout return route
			setitimer(ITIMER_REAL, NULL, NULL);
			if(alarm_oldset != SIG_ERR) {
				ret = signal(SIGALRM, alarm_oldset);
			}
			else {
				ret = signal(SIGALRM, SIG_DFL);
			}
			if(ret == SIG_ERR) {
				ktrace(_SystemFault,
						KeyValue_s("@", "signal"),
						KeyValue_u("errno", errno),
						KeyValue_s("errstr", strerror(errno))
				);
			}
			return S_TIMEOUT;
		}
	}
	if(bg != 1) {
		// SIGINT registration
		fgPid = pid;
		keyInt_oldset = signal(SIGINT, keyIntHandler);
	}
	if(timeout > 0) {
		// SIGALRM registration
		struct itimerval its;
		its.it_value.tv_sec  = timeout / 1000;
		its.it_value.tv_usec = (timeout % 1000) * 1000;
		its.it_interval.tv_sec  = 0;
		its.it_interval.tv_usec = 0;
		setitimer(ITIMER_REAL, &its, NULL);
		alarm_oldset = signal(SIGALRM, alarmHandler);
	}
	int stat;
	waitpid(pid, &stat, WUNTRACED);
	if(timeout > 0) {
		// SIGALRM release
		struct itimerval its;
		its.it_value.tv_sec = 0;
		its.it_value.tv_usec = 0;
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_usec = 0;
		setitimer(ITIMER_REAL, &its, NULL);
		if(alarm_oldset != SIG_ERR) {
			signal(SIGALRM, alarm_oldset);
		}
		else {
			signal(SIGALRM, SIG_DFL);
		}
	}
	if(bg != 1) {
		// SIGINT release
		if(keyInt_oldset != SIG_ERR) {
			ret = signal(SIGINT, keyInt_oldset);
		}
		else {
			ret = signal(SIGINT, SIG_DFL);
		}
		if(ret == SIG_ERR) {
			ktrace(_SystemFault,
					KeyValue_s("@", "signal"),
					KeyValue_u("errno", errno),
					KeyValue_s("errstr", strerror(errno))
			);
		}
	}
	if(status != NULL) {
		*status = stat;
	}
	// return value creation
	if(WIFSIGNALED(stat)) {
		return WTERMSIG(stat) * -1;
	}else if ( WIFSTOPPED(stat) ) {
		return WSTOPSIG(stat) * -1;
	}else {
		return S_EXIT;
	}
}

// for fg & bg & restart
/**
 * @return in the case of foreground, it is start status of a child process
 *         in the case of background, it is termination status of a child process
 */
static int proc_start(KonohaContext *kctx, subprocData_t *spd) {
	int ret = S_PREEXECUTION;
	int pid = knh_popen(kctx, spd->command, spd, M_NREDIRECT );
	if(pid > 0) {
		spd->cpid  = pid;
		if(spd->bg != 1) {
			ret = knh_wait(kctx, spd->cpid, spd->bg, spd->timeout, &spd->status );
		}else {
			// nomal end status for bg
			ret = 0;
		}
	}else {
		DBG_P("failed");
	}
	return ret;
}

// for new & enablePipemodeXXX(false) & enableStandardXXX(false) & enableERR2StdOUT(false)
inline void initFd(pfd_t *p) {
	p->mode = M_DEFAULT;
	p->fp   = NULL;
}

// for setFileXXX & PipemodeXXX(true) & enableStandardXXX(true) & enableERR2StdOUT(true)
static void setFd(KonohaContext *kctx, pfd_t *p, int changeMode, FILE* ptr) {
	if(((p->mode == M_PIPE) || (p->mode == M_FILE)) && !(p->mode == changeMode)) {
		// warning of the pipe or file mode overwrite
		//char *msg = (p->mode == M_PIPE) ? "pipe has already set, but we overwrite it." :
		//"file has already set, but we overwrite it." ;
		//WarnTagPackageMessage(kctx, msg );
		//fprintf(stderr, "%s\n", msg);
	}
	p->mode = changeMode;
	p->fp = ptr;
}

// for exec & restart
static void clearFd(pfd_t *p) {
	if(((p->mode == M_PIPE) || (p->mode == M_DEFAULT)) && (p->fp != NULL)) {
		// a file identification child does not close
		fclose(p->fp);
		p->fp = NULL;
	}
}

// for new
static void initData (KonohaContext *kctx, subprocData_t* p) {
	p->command     = KNULL(String);
	p->cwd         = KNULL(String);
	p->env         = KNULL(Array);
	p->cpid        = -1;
	p->closefds    = 0;
	p->bg          = 0;
	p->shell       = 0;
	p->timeout     = DEF_TIMEOUT;
	p->bufferSize  = 0;
	p->timeoutKill = 0;
	initFd(&p->r);
	initFd(&p->w);
	initFd(&p->e);
	INIT_RESOURCE_MONITOR(p);
}

// for poll
static int getPidStatus(int pid, int *status) {
	return waitpid(pid, status, WNOHANG | WUNTRACED | WCONTINUED);
}

// for Subproc_free & fg & exec & communicate & restart
static void killWait(int pid) {
	int status;
	kill(pid, SIGKILL);
	usleep(DELAY);
	getPidStatus(pid, &status); // this wait is in order not to leave a zombie process.
}

/* ------------------------------------------------------------------------ */

//## Subproc Subproc.new(String cmd, boolean closefds);
static KMETHOD Subproc_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	if ( p != NULL ) {
		initData(kctx, p);
		p->command = (S_size(sfp[1].asString) > 0) ? KLIB new_kString(kctx, S_text(sfp[1].asString), S_size(sfp[1].asString), 0)
											: KNULL(String);
		p->closefds = sfp[2].boolValue;
	}
	RETURN_(sp);
}

//## boolean Subproc.bg();
static KMETHOD Subproc_bg(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	int ret = -1;
	if (PREEXEC(p)) {
		p->timeoutKill = 0;
		p->bg = 1;
		if ( (ret = proc_start(kctx, p)) != 0 ) {
//		KNH_NTRACE2(kctx, "package.subproc.bg ", K_PERROR, KNH_LDATA0);
		}
	}
	RETURNb_( (ret == 0) );
}

//## @Throwable int Subproc.fg();
KMETHOD Subproc_fg(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	int ret = S_PREEXECUTION;
	if ( PREEXEC(p) ) {
		p->timeoutKill = 0;
		p->bg = 0;
		if ( (ret = proc_start(kctx, p)) == S_TIMEOUT ) {
			p->timeoutKill = 1;
			killWait(p->cpid);
//			KNH_NTHROW2(kctx, sfp, "Script!!", "subproc.fg :: timeout", K_FAILED, KNH_LDATA0);
		}
	}
	RETURNi_( ret );
}

//## @Throwable String Subproc.exec(String data);
KMETHOD Subproc_exec(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	kString *ret_s = KNULL(String);
	if(PREEXEC(p)) {
		p->timeoutKill = 0;
		kString *command = (S_size(sfp[1].asString) > 0) ? KLIB new_kString(kctx, S_text(sfp[1].asString),S_size(sfp[1].asString), 0) :
													 KNULL(String);
		int pid = knh_popen(kctx, command, p, M_PIPE );
		if(pid > 0 ) {
			if(knh_wait(kctx, pid, 0, p->timeout, NULL ) == S_TIMEOUT ) {
				p->timeoutKill = 1;
				killWait(pid);
				clearFd(&p->r);
				clearFd(&p->w);
				clearFd(&p->e);
				ktrace(_SystemFault,
						KeyValue_s("@", "TIMEOUT"),
						KeyValue_u("errno", errno),
						KeyValue_s("errstr", strerror(errno))
				);
			}
			else if ( (p->r.mode == M_PIPE) || (p->r.mode == M_DEFAULT) ) {
				char buf[BUFSIZE] = {0};
				if(fread(buf, sizeof(char), sizeof(buf)-1, p->r.fp) > 0) {
					ret_s = KLIB new_kString(kctx, buf, strlen(buf), 0);
				}
				else {
					if(ferror(p->r.fp)) {
						ktrace(_SystemFault,
								KeyValue_s("@", "fread"),
								KeyValue_u("errno", errno),
								KeyValue_s("errstr", strerror(errno))
						);
					}
					else {
						// reached eof?
						// do nothing
					}
				}
				clearFd(&p->r);
				clearFd(&p->w);
				clearFd(&p->e);
			}else if (p->r.mode == M_FILE) {
				char *msg = " will be ignored.";
				char *cmd = (char*)S_text(sfp[1].asString);
				char mbuf[strlen(msg)+strlen(cmd)+1];
				snprintf(mbuf, sizeof(mbuf), "'%s'%s", cmd, msg);
			}
		}else {
			ktrace(_SystemFault,
					KeyValue_s("@", "knh_wait"),
					KeyValue_u("errno", errno),
					KeyValue_s("errstr", strerror(errno))
			);
		}
		struct itimerval val;
		getitimer(ITIMER_REAL, &val);
		val.it_value.tv_sec = 0;
		setitimer(ITIMER_REAL, &val, NULL);
	}
	// remove alarm
	RETURN_( ret_s );
}

//## @Throwable String[] Subproc.communicate(String input);
static KMETHOD Subproc_communicate(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *ret_a = KNULL(Array);
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	if(ONEXEC(p)) {
		if((p->w.mode == M_PIPE) && (S_size(sfp[1].asString) > 0)) {
			kString *s = sfp[1].asString;
			// The measure against panic,
			// if "Broken Pipe" is detected at the time of writing.
#ifndef __APPLE__
			__sighandler_t oldset = signal(SIGPIPE, SIG_IGN);
#else
			sig_t oldset = signal(SIGPIPE, SIG_IGN);
#endif
			// WARN??
			kBytes* ba = (kBytes*)KLIB new_kObject(kctx, CT_Bytes, S_size(s));
			memcpy(ba->buf, s->utext, S_size(s));
			if(fwrite(ba->buf, sizeof(char), ba->bytesize, p->w.fp) > 0) {
				fputc('\n', p->w.fp);
				fflush(p->w.fp);
				fsync(fileno(p->w.fp));
			}else {
				ktrace(_SystemFault,
						KeyValue_s("@", "fwrite"),
						KeyValue_u("errno", errno),
						KeyValue_s("errstr", strerror(errno))
				);
//				KNH_NTRACE2(ctx, "package.subproc.communicate ", K_PERROR, KNH_LDATA0);
			}
			if(oldset != SIG_ERR) {
				signal(SIGPIPE, oldset);
			}
		}
		if(knh_wait(kctx, p->cpid, p->bg, p->timeout, &p->status ) == S_TIMEOUT) {
			p->timeoutKill = 1;
			killWait(p->cpid);
			ktrace(_SystemFault,
					KeyValue_s("@", "knh_wait"),
					KeyValue_u("errno", errno),
					KeyValue_s("errstr", strerror(errno))
			);
		}else {
			ret_a = (kArray*)KLIB new_kObject(kctx, CT_Array, 0);
			if(p->r.mode == M_PIPE) {
				char buf[BUFSIZE];
				memset(buf, 0x00, sizeof(buf));
				// what if there's more than bufsize output?!
				if(fread(buf, sizeof(char), sizeof(buf)-1, p->r.fp) > 0) {
					KLIB kArray_add(kctx, ret_a, KLIB new_kString(kctx, buf, BUFSIZE, 0));//TODO!
				}
				else {
					KLIB kArray_add(kctx, ret_a, KNULL(String));
					ktrace(_SystemFault,
							KeyValue_s("@", "fread"),
							KeyValue_u("errno", errno),
							KeyValue_s("errstr", strerror(errno))
					);
//					KNH_NTRACE2(ctx, "package.subprocess.communicate.fread ", K_PERROR, KNH_LDATA0);
				}
			}
			else {
				KLIB kArray_add(kctx,  ret_a, KNULL(String));
			}
			if(p->e.mode == M_PIPE) {
				char buf[BUFSIZE];
				memset(buf, 0x00, sizeof(buf));
				if(fread(buf, sizeof(char), sizeof(buf)-1, p->e.fp) > 0) {
					KLIB kArray_add(kctx, ret_a, KLIB new_kString(kctx, buf, BUFSIZE, 0)); // TODO!
				} else {
					KLIB kArray_add(kctx, ret_a, KNULL(String));
//					KNH_NTRACE2(ctx, "package.subprocess.communicate.fread ", K_PERROR, KNH_LDATA0);
				}
			}
			else {
				KLIB kArray_add(kctx, ret_a, KNULL(Object));
			}
		}
	}
	RETURN_( ret_a );
}


//## @Restricted boolean Subproc.enableShellmode(boolean isShellmode);
KMETHOD Subproc_enableShellmode(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	int ret = PREEXEC(p);
	if(ret) {
		p->shell = sfp[1].boolValue;
	}
	RETURNb_( ret );
}

//## boolean Subproc.setEnv(Map env);
//KMETHOD Subproc_setEnv(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kSubproc *sp = (kSubproc*)sfp[0].asObject;
//	subprocData_t *p = sp->spd;
//	int ret = PREEXEC(p);
//	if ( ret ) {
//		kDictMap *env = (kDictMap *)sfp[1].asObject;
//		int i;
//		size_t msize = env->spi->size(ctx, env->mapptr);
//		if ( p->env != (kArray*)KNH_NULVAL(TY_Array) ) {
//			knh_Array_clear( ctx, p->env, 0 );
//		}
//		p->env = new_Array(ctx, TY_String, msize);
//		for (i = 0; i < msize; i++) {
//			kString *key = (kString *)knh_DictMap_keyAt(env, i);
//			kString *val = (kString *)knh_DictMap_valueAt(env, i);
//			char buf[key->str.len + val->str.len + 2];
//			snprintf(buf, sizeof(buf), "%s=%s", key->str.buf, val->str.buf);
//			knh_Array_add( ctx, p->env, new_String(ctx, buf) );
//		}
//	}
//	RETURNb_( ret );
//}

//## boolean Subproc.setCwd(String cwd);
KMETHOD Subproc_setCwd(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	int ret = PREEXEC(p);
	if(ret) {
		p->cwd = KLIB new_kString(kctx, S_text(sfp[1].asString), S_size(sfp[1].asString), 0);
	}
	RETURNb_( ret );
}

//## boolean Subproc.setBufsize(int size);
KMETHOD Subproc_setBufsize(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	int ret = PREEXEC(p);
	if(ret) {
		p->bufferSize = WORD2INT(sfp[1].intValue);
	}
	RETURNb_( ret );
}

//## boolean Subproc.setFileIN(File in);
//KMETHOD Subproc_setFileIN(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kSubproc *sp = (kSubproc*)sfp[0].asObject;
//	subprocData_t *p = sp->spd;
//	int ret = PREEXEC(p);
//	if(ret) {
//		ret = (sfp[1].p->rawptr != NULL);
//		if(ret) {
//			setFd(ctx, &p->w, M_FILE, (FILE*)sfp[1].p->rawptr);
//		}
//	}
//	RETURNb_( ret );
//}

//## boolean Subproc.setFileOUT(File out);
//KMETHOD Subproc_setFileOUT(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kSubproc *sp = (kSubproc*)sfp[0].asObject;
//	subprocData_t *p = sp->spd;
//	int ret = PREEXEC(p);
//	if(ret) {
//		ret = (sfp[1].p->rawptr != NULL);
//		if ( ret ) {
//			setFd(ctx, &p->r, M_FILE, (FILE*)sfp[1].p->rawptr);
//		}
//	}
//	RETURNb_( ret );
//}

//## boolean Subproc.setFileERR(File err);
//KMETHOD Subproc_setFileERR(KonohaContext *kctx, KonohaStack *sfp)
//{
//	subprocData_t *p = (subprocData_t*)sfp[0].p->rawptr;
//	int ret = PREEXEC(p);
//	if(ret) {
//		ret = (sfp[1].p->rawptr != NULL);
//		if(ret) {
//			setFd(ctx, &p->e, M_FILE, (FILE*)sfp[1].p->rawptr);
//		}
//	}
//	RETURNb_( ret );
//}

//## boolean Subproc.setTimeout(int milisec);
//KMETHOD Subproc_setTimeout(KonohaContext *kctx, KonohaStack *sfp)
//{
//	subprocData_t *p = (subprocData_t*)sfp[0].p->rawptr;
//	int ret = PREEXEC(p);
//	if(ret) {
//		int time = WORD2INT(sfp[1].intValue);
//		p->timeout = ( time > 0 ) ? time : 0;
//	}
//	RETURNb_( ret );
//}

////## File Subproc.getIN();
//KMETHOD Subproc_getIn(KonohaContext *kctx, KonohaStack *sfp)
//{
//	subprocData_t *p = (subprocData_t*)sfp[0].p->rawptr;
//	kRawPtr *po = (kRawPtr*)KNH_NULVAL(TY_void);
//	if(ONEXEC(p)) {
//		if( p->w.mode == M_PIPE ) {
//			po = new_RawPtr(ctx, ClassTBL(khn_getFileClass(ctx)), p->w.fp);
//		}
//	}
//	RETURN_( po );
//}

//## File Subproc.getOUT();
//KMETHOD Subproc_getOut(KonohaContext *kctx, KonohaStack *sfp)
//{
//	subprocData_t *p = (subprocData_t*)sfp[0].p->rawptr;
//	kRawPtr *po = (kRawPtr*)KNH_NULVAL(TY_void);
//	if(ONEXEC(p)) {
//		if( p->r.mode == M_PIPE ) {
//			po = new_RawPtr(ctx, ClassTBL(khn_getFileClass(ctx)), p->r.fp);
//		}
//	}
//	RETURN_( po );
//}

//## File Subproc.getERR();
//KMETHOD Subproc_getErr(KonohaContext *kctx, KonohaStack *sfp)
//{
//	subprocData_t *p = (subprocData_t*)sfp[0].p->rawptr;
//	kRawPtr *po = KNULL(TY_void);
//	if(ONEXEC(p)) {
//		if( p->e.mode == M_PIPE ) {
//			po = new_RawPtr(ctx, ClassTBL(khn_getFileClass(ctx)), p->e.fp);
//		}
//	}
//	RETURN_( po );
//}

//## int Subproc.getPid();
KMETHOD Subproc_getPid(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	RETURNi_( (p!= NULL) ? p->cpid : -1 );
}

//## int Subproc.getTimeout();
KMETHOD Subproc_getTimeout(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	RETURNi_( (p!= NULL) ? p->timeout : -1 );
}

//## int Subproc.getReturncode();
KMETHOD Subproc_getReturncode(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	RETURNi_( (p!= NULL) ? p->status : -1 );
}

//## boolean Subproc.enablePipemodeIN(Boolean isPipemode);
KMETHOD Subproc_enablePipemodeIN(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	int ret = PREEXEC(p);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			setFd(kctx, &p->w, M_PIPE, NULL);
		} else {
			if(p->w.mode == M_PIPE) {
				initFd(&p->w);
			}
		}
	}
	RETURNb_( ret );
}

//## boolean Subproc.enablePipemodeOUT(Boolean isPipemode);
KMETHOD Subproc_enablePipemodeOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	int ret = PREEXEC(p);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			setFd(kctx, &p->r, M_PIPE, NULL);
		} else {
			if(p->r.mode == M_PIPE) {
				initFd(&p->r);
			}
		}
	}
	RETURNb_( ret );
}

//## boolean Subproc.enablePipemodeERR(Boolean isPipemode);
KMETHOD Subproc_enablePipemodeERR(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	int ret = PREEXEC(p);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			setFd(kctx, &p->e, M_PIPE, NULL);
		}
		else {
			if(p->e.mode == M_PIPE) {
				initFd(&p->e);
			}
		}
	}
	RETURNb_( ret );
}

//## boolean Subproc.enableStandardIN(Boolean isStandard);
KMETHOD Subproc_enableStandardIN(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	int ret = PREEXEC(p);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			setFd(kctx, &p->w, M_NREDIRECT, NULL);
		}
		else {
			if(p->w.mode == M_NREDIRECT) {
				initFd(&p->w);
			}
		}
	}
	RETURNb_( ret );
}

//## boolean Subproc.enableStandardOUT(Boolean isStandard);
KMETHOD Subproc_enableStandardOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	int ret = PREEXEC(p);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			setFd(kctx, &p->r, M_NREDIRECT, NULL);
		} else {
			if(p->r.mode == M_NREDIRECT) {
				initFd(&p->r);
			}
		}
	}
	RETURNb_( ret );
}

//## boolean Subproc.enableStandardERR(Boolean isStandard);
KMETHOD Subproc_enableStandardERR(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	int ret = PREEXEC(p);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			setFd(kctx, &p->e, M_NREDIRECT, NULL);
		} else {
			if(p->e.mode == M_NREDIRECT) {
				initFd(&p->e);
			}
		}
	}
	RETURNb_(ret);
}

//## boolean Subproc.enableERR2StdOUT(Boolean isStdout);
KMETHOD Subproc_enableERR2StdOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	int ret = PREEXEC(p);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			setFd(kctx, &p->e, M_STDOUT, NULL);
		} else {
			if(p->e.mode == M_STDOUT) {
				initFd(&p->e);
			}
		}
	}
	RETURNb_(ret);
}

//## boolean Subproc.isShellmode();
KMETHOD Subproc_isShellmode(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	RETURNb_((p != NULL) ? (p->shell == 1) : 0);
}

//## boolean Subproc.isPipemodeIN();
KMETHOD Subproc_isPipemodeIN(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	RETURNb_((p != NULL) ? (p->w.mode == M_PIPE) : 0);
}

//## boolean Subproc.isPipemodeOUT();
KMETHOD Subproc_isPipemodeOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	RETURNb_((p != NULL) ? (p->r.mode == M_PIPE) : 0);
}

//## boolean Subproc.isPipemodeERR();
KMETHOD Subproc_isPipemodeERR(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	RETURNb_((p != NULL) ? (p->e.mode == M_PIPE) : 0);
}

//## boolean Subproc.isStandardIN();
KMETHOD Subproc_isStandardIN(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	RETURNb_((p != NULL) ? (p->w.mode == M_NREDIRECT) : 0);
}

//## boolean Subproc.isStandardOUT();
KMETHOD Subproc_isStandardOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	RETURNb_((p != NULL) ? (p->r.mode == M_NREDIRECT) : 0);
}

//## boolean Subproc.isStandardERR();
KMETHOD Subproc_isStandardERR(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	RETURNb_((p != NULL) ? (p->e.mode == M_NREDIRECT) : 0);
}

//## boolean Subproc.isERR2StdOUT();
KMETHOD Subproc_isERR2StdOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc*)sfp[0].asObject;
	subprocData_t *p = sp->spd;
	RETURNb_((p != NULL) ? (p->e.mode == M_STDOUT) : 0);
}

/* ------------------------------------------------------------------------ */

static void kmodsubproc_setup(KonohaContext *kctx, struct KonohaModule *def, int newctx)
{
}

static void kmodsubproc_reftrace(KonohaContext *kctx, struct KonohaModule *baseh)
{
}

static void kmodsubproc_free(KonohaContext *kctx, struct KonohaModule *baseh)
{
	KFREE(baseh, sizeof(kmodsubproc_t));
}

/* ------------------------------------------------------------------------ */

static void Subproc_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct _kSubproc *proc = (struct _kSubproc*)o;
	if(conf != NULL) {
		proc->spd = (subprocData_t*)conf;
	}else {
		proc->spd = (subprocData_t *)KCALLOC(sizeof(subprocData_t), 1);
	}
}

static void Subproc_free(KonohaContext *kctx, kObject *o)
{
	struct _kSubproc *proc = (struct _kSubproc*)o;
	if(proc->spd != NULL) {
		KFREE(proc->spd, sizeof(subprocData_t));
		proc->spd = NULL;
	}
}

static void Subproc_p(KonohaContext *kctx, KonohaStack *sfp, int pos, KUtilsWriteBuffer *wb, int level)
{

}

/* ------------------------------------------------------------------------ */

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _Im kMethod_Immutable
#define _F(F)   (intptr_t)(F)

static kbool_t subproc_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	kmodsubproc_t *base = (kmodsubproc_t *)KCALLOC(sizeof(kmodsubproc_t), 1);
	base->h.name     = "subproc";
	base->h.setup    = kmodsubproc_setup;
	base->h.reftrace = kmodsubproc_reftrace;
	base->h.free     = kmodsubproc_free;
	KLIB Konoha_setModule(kctx, MOD_subproc, &base->h, pline);

	KDEFINE_CLASS defSubproc = {
		STRUCTNAME(Subproc),
		.cflag = kClass_Final,
		.init  = Subproc_init,
		.free  = Subproc_free,
		.p     = Subproc_p,
	};

	base->cSubproc= KLIB Konoha_defineClass(kctx, ns->packageId, ns->packageDomain, NULL, &defSubproc, pline);

	kparamtype_t ps = {TY_String, FN_("str")};
	KonohaClass *CT_StringArray2 = KLIB KonohaClass_Generics(kctx, CT_Array, TY_String, 1, &ps);
	ktype_t TY_StringArray = CT_StringArray2->typeId;

	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(Subproc_new), TY_Subproc, TY_Subproc,MN_("new"), 2, TY_String, FN_("path"), TY_Boolean, FN_("mode"),
		_Public|_Const|_Im, _F(Subproc_fg), TY_Int, TY_Subproc, MN_("fg"), 0,
		_Public|_Const|_Im, _F(Subproc_bg), TY_Boolean, TY_Subproc, MN_("bg"), 0,
		_Public|_Const|_Im, _F(Subproc_exec), TY_String, TY_Subproc, MN_("exec"), 1, TY_String, FN_("data"),
		_Public|_Const|_Im, _F(Subproc_communicate), TY_StringArray, TY_Subproc, MN_("communicate"), 1, TY_String, FN_("input"),
//		_Public|_Const|_Im, _F(Subproc_poll), TY_Int, TY_Subproc, MN_("poll"), 0,
//		_Public|_Const|_Im, _F(Subproc_wait), TY_Int, TY_Subproc, MN_("wait"), 0,
//		_Public|_Const|_Im, _F(Subproc_sendSignal), TY_Boolean, TY_Subproc, MN_("sendSignal"), 1 TY_Int, FN_("signal"),
//		_Public|_Const|_Im, _F(Subproc_terminate), TY_Boolean, TY_Subproc, MN_("terminate"), 0,
		_Public|_Const|_Im, _F(Subproc_getPid), TY_Int, TY_Subproc, MN_("getPid"), 0,
		_Public|_Const|_Im, _F(Subproc_enableShellmode), TY_Boolean, TY_Subproc, MN_("enableShellmode"), 1, TY_Boolean, FN_("isShellmode"),
		_Public|_Const|_Im, _F(Subproc_enablePipemodeIN), TY_Boolean, TY_Subproc, MN_("enablePipemodeIN"), 1, TY_Boolean, FN_("isPipemode"),
		_Public|_Const|_Im, _F(Subproc_enablePipemodeOUT), TY_Boolean, TY_Subproc, MN_("enablePipemodeOUT"), 1, TY_Boolean, FN_("isPipemode"),
		_Public|_Const|_Im, _F(Subproc_enablePipemodeERR), TY_Boolean, TY_Subproc, MN_("enablePipemodeERR"), 1, TY_Boolean, FN_("isPipemode"),
		_Public|_Const|_Im, _F(Subproc_enableStandardIN), TY_Boolean, TY_Subproc, MN_("enableStandardIN"), 1, TY_Boolean, FN_("isStandard"),
		_Public|_Const|_Im, _F(Subproc_enableStandardOUT), TY_Boolean, TY_Subproc, MN_("enableStandardOUT"), 1, TY_Boolean, FN_("isStandard"),
		_Public|_Const|_Im, _F(Subproc_enableStandardERR), TY_Boolean, TY_Subproc, MN_("enableStandardERR"), 1, TY_Boolean, FN_("isStandard"),
		_Public|_Const|_Im, _F(Subproc_isShellmode), TY_Boolean, TY_Subproc, MN_("isShellmode"), 0,
		_Public|_Const|_Im, _F(Subproc_isPipemodeIN), TY_Boolean, TY_Subproc, MN_("isPipemodeIN"), 0,
		_Public|_Const|_Im, _F(Subproc_isPipemodeOUT), TY_Boolean, TY_Subproc, MN_("isPipemodeOUT"), 0,
		_Public|_Const|_Im, _F(Subproc_isPipemodeERR), TY_Boolean, TY_Subproc, MN_("isPipemodeERR"), 0,
		_Public|_Const|_Im, _F(Subproc_isStandardIN), TY_Boolean, TY_Subproc, MN_("isStandardIN"), 0,
		_Public|_Const|_Im, _F(Subproc_isStandardOUT), TY_Boolean, TY_Subproc, MN_("isStandardOUT"), 0,
		_Public|_Const|_Im, _F(Subproc_isStandardERR), TY_Boolean, TY_Subproc, MN_("isStandardERR"), 0,
		_Public|_Const|_Im, _F(Subproc_isERR2StdOUT), TY_Boolean, TY_Subproc, MN_("isERR2StdOUT"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t subproc_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t subproc_initNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t subproc_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* subproc_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("subproc", "1.0"),
		.initPackage = subproc_initPackage,
		.setupPackage = subproc_setupPackage,
		.initNameSpace = subproc_initNameSpace,
		.setupNameSpace = subproc_setupNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
