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

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>

#define USE_FILE 1
#include <minikonoha/minikonoha.h>
#include <minikonoha/klib.h>
#include <minikonoha/konoha_common.h>

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

/* ------------------------------------------------------------------------ */

// child process IO mode
enum {
	M_DEFAULT   = -2, // initialization state
	M_NREDIRECT = -1, // parent process succession
	M_PIPE      =  0, // pipe
	M_STDOUT    =  1, // stdout
	M_FILE      =  2  // file
};

// child process status code
enum {
	S_RUNNING      = -300, // running
	S_PREEXECUTION = -400, // preexecution
	S_TIMEOUT      = -500, // tiomeout
	S_EXIT         =    0  // terminate
};

// subproc macro
#define MAXARGS            128				// the number maximum of parameters for spSplit
#define DELAY              1000				// the adjustment value at the time of signal transmission
#define DEF_TIMEOUT        10 * 1000		// default timeout valx
//#define DEF_TIMEOUT -1


/* ------------------------------------------------------------------------ */

#define kSubprocFlag_CloseFds      kObject_Local1
#define kSubprocFlag_Background    kObject_Local2
#define kSubprocFlag_Shell         kObject_Local3
#define kSubprocFlag_TimeoutKill   kObject_Local4
#define kSubprocFlag_OnExec        kObject_Local4

#define kSubproc_is(P, o)      (TFLAG_is(uintptr_t,(o)->h.magicflag, kSubprocFlag_##P))
#define kSubproc_set(P,o,B)    TFLAG_set(uintptr_t,(o)->h.magicflag, kSubprocFlag_##P, B)

typedef struct kSubprocVar kSubproc;
struct kSubprocVar {
	KonohaObjectHeader h;
	kArray  *env;     // child process environment
	kString *command; // child process command
	kString *cwd;     // child process current working directory
	kFile   *rfp;     // input stream
	kFile   *wfp;     // output stream
	kFile   *efp;     // err stream
	int      rmode;   // input stream mode
	int      wmode;   // output stream mode
	int      emode;   // err stream mode
	int      cpid;    // child process ID
	int      timeout; // child process timeout value
	int      status;  // waitpid status
	//kSubproc_RESOURCEMON_INSTANCE;
};

static void kSubproc_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kSubproc *proc = (kSubproc *)o;
	proc->env      = KNULL(Array);
	proc->command  = KNULL(String);
	proc->cwd      = KNULL(String);
	proc->rfp      = KNULL(File);
	proc->wfp      = KNULL(File);
	proc->efp      = KNULL(File);
	proc->rmode    = M_DEFAULT;
	proc->wmode    = M_DEFAULT;
	proc->emode    = M_DEFAULT;
	proc->cpid     = -1;
	proc->timeout  = DEF_TIMEOUT;
	INIT_RESOURCE_MONITOR(proc);
}

static void kSubproc_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kSubproc *proc = (kSubproc *)o;
	KREFTRACEv(proc->env);
	KREFTRACEv(proc->command);
	KREFTRACEv(proc->cwd);
	KREFTRACEv(proc->rfp);
	KREFTRACEv(proc->wfp);
	KREFTRACEv(proc->efp);
}

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

#if defined (__APPLE__) || defined(__USE_LOCAL_PIPE2__)
// for fg & bg & exec & restart
static int pipe2( int *fd, int flags ) {
	int val;
	int fds[2];
	if(pipe(fds) < 0) {
		return -1;
	}
	if((val = fcntl(fds[0], F_GETFL, 0)) < 0) {
		goto L_ERR;
	}
	if(fcntl(fds[0], F_SETFL, val|flags) < 0) {
		goto L_ERR;
	}
	if((val = fcntl(fds[1], F_GETFL, 0)) < 0) {
		goto L_ERR;
	}
	if(fcntl(fds[1], F_SETFL, val|flags) < 0) {
		goto L_ERR;
	}
	fd[0] = fds[0];
	fd[1] = fds[1];
	return 0;
L_ERR:;
	{
		close(fds[0]);
		close(fds[1]);
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
// if(param > 0 ) {
//    printf("param:%d args[0]:%s args[1]:%s\n", param, args[0], args[1]);
// }
//	else {
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

	if((str == NULL) || (args == NULL)) {
		return -1;
	}
	int index;
	char *cp = str;
	for(index = 0; index <= MAXARGS; index++) {
		if(index == MAXARGS) {
			return -2;
		}
		else if((args[index] = strtok(cp, " ")) == NULL) {
			break;
		}
		else {
			cp = NULL;
		}
	}
	// number of parameter
	return index;
}

// for getIN & getOUT & getERR
/**
 * @return "konoha.posix.File" class id
 */
//static KonohaClass khn_getFileClass(KonohaContext *kctx) {
//	char *name = "konoha.posix.File";
//	kbytes_t lname;
//	lname.ubuf = (kchar_t *)name;
//	lname.len = knh_strlen(lname.text);
//	return knh_getcid(kctx, lname);
//}

// for fg & bg & exec & restart
/**
 * @return child process id
 *         -1 is Internal Error
 */

static int kSubproc_popen(KonohaContext *kctx, kSubproc *proc, int defaultMode, KTraceInfo *trace)
{
	kString *command = proc->command;
	if(IS_NULL(command) || S_size(command) == 0) {
		return -1;
	}
	pid_t pid  = -1;
	int rmode = (proc->rmode==M_DEFAULT) ? defaultMode : proc->rmode;
	int wmode = (proc->wmode==M_DEFAULT) ? defaultMode : proc->wmode;
	int emode = (proc->emode==M_DEFAULT) ? defaultMode : proc->emode;
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
			KTraceApi(trace, SystemFault, "pipe", LogErrno);
			close(c2p[0]); close(c2p[1]);
			return -1;
		}
	}

	if(emode == M_PIPE) {
		if(pipe2(err, O_NONBLOCK) != 0) {
			KTraceApi(trace, SystemFault, "pipe2", LogErrno);
			close(c2p[0]); close(c2p[1]);
			close(p2c[0]); close(p2c[1]);
			return -1;
		}
	}

	//	SETUP_RESOURCE_MONITOR(sp);

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
		//		CLEANUP_RESOURCE_MONITOR(sp);
		break;
	case 0:
		// child process normal route
		//	SETUP_RESOURCE_MONITOR_FOR_CHILD(sp);
		if(wmode == M_PIPE) {
			close(0);
			if(dup2(p2c[0], 0) == -1) {
				KTraceApi(trace, SystemFault, "dup2", LogErrno);
			}
			close(p2c[0]); close(p2c[1]);
		}
		else if(wmode == M_FILE) {
			close(0);
			if(dup2(fileno(proc->wfp->fp), 0) == -1) {
				KTraceApi(trace, SystemFault, "dup2", LogErrno);
			}
		}
		if(rmode == M_PIPE) {
			close(1);
			if(dup2(c2p[1], 1) == -1){
				KTraceApi(trace, SystemFault, "dup2", LogErrno);
			}
			close(c2p[0]); close(c2p[1]);
		}
		else if(rmode == M_FILE) {
			close(1);
			if(dup2(fileno(proc->rfp->fp), 1) == -1) {
				KTraceApi(trace, SystemFault, "dup2", LogErrno);
			}
		}
		if(emode == M_PIPE) {
			close(2);
			if(dup2(err[1], 2) == -1) {
				KTraceApi(trace, SystemFault, "dup2", LogErrno);
			}
			close(err[0]); close(err[1]);
		}
		else if(emode == M_FILE) {
			close(2);
			if(dup2(fileno(proc->efp->fp), 2) == -1) {
				KTraceApi(trace, SystemFault, "dup2", LogErrno);
			}
		}
		else if(emode == M_STDOUT) {
			close(2);
			if(dup2(1, 2) == -1) {
				KTraceApi(trace, SystemFault, "dup2", LogErrno);
			}
		}
		if(kSubproc_is(CloseFds, proc)) {
			// close other than 0, 1, and 2
			int cfd = 3;
			int maxFd = sysconf(_SC_OPEN_MAX);
			do {
				close(cfd);
			} while(++cfd < maxFd);
		}
		setsid(); // separate from tty
		if(IS_NOTNULL(proc->cwd)) {
			if(chdir(S_text((proc->cwd))) != 0) {
				KTraceApi(trace, SystemFault|UserFault, "chrdir", LogErrno);
				_exit(1);
			}
		}
		char *args[MAXARGS];
		if(!kSubproc_is(Shell, proc)) {
			// division of a commnad parameter
			if(spSplit((char*)S_text(command), args) < 0){
				//TODO: error
				args[0] = NULL;
			}
		}
		if(IS_NOTNULL(proc->env)) {
			// division of a environment parameter
			kArray *a = proc->env;
			int num = kArray_size(a);
			char *envs[num+1];
			int i;
			for(i = 0; i < num; i++) {
				envs[i] = (char *)S_text(a->stringItems[i]);
			}
			envs[num] = NULL;
			// exec load new process image if success.
			// if its not, they will return with -1.
			if(!kSubproc_is(Shell, proc)) {
				if(execve(args[0], args, envs) == -1) {
					KTraceApi(trace, SystemFault|UserFault, "execve", LogErrno);
				}
			}
			else {
				if(execle("/bin/sh", "sh", "-c", S_text(command), NULL, envs) == -1) {
					KTraceApi(trace, SystemFault|UserFault, "execle", LogErrno);
				}
			}
		}
		else {
			if(!kSubproc_is(Shell, proc)) {
				if(execvp(args[0], args) == -1) {
					KTraceApi(trace, SystemFault|UserFault, "execvp", LogErrno);
				}
			}
			else {
				if(execlp("sh", "sh", "-c", S_text(command), NULL) == -1) {
					KTraceApi(trace, SystemFault|UserFault, "execlp", LogErrno);
				}
			}
		}
		perror("kSubproc_popen :");
		_exit(1);
	default:
		// parent process normal route
#if defined(SUBPROC_ENABLE_RESOURCEMONITOR)
/		ATTACH_RESOURCE_MONITOR_FOR_CHILD(proc, pid);
//		size_t mem = FETCH_MEM_FROM_RESOURCE_MONITOR(sp);
//		fprintf(stderr, "menusage:%.1fM\n", (double)mem / (1024.0 * 1024.0));
		CLEANUP_RESOURCE_MONITOR(proc);
#endif
		if(rmode == M_PIPE) {
			DBG_P("rfp is set");
			KFieldSet(proc, proc->rfp, (kFile *)KLIB new_kObject(kctx, OnField, CT_FILE, (intptr_t)fdopen(c2p[0], "r")));
			close(c2p[1]);
		}
		if(wmode == M_PIPE) {
			DBG_P("wfp is set");
			KFieldSet(proc, proc->wfp, (kFile *)KLIB new_kObject(kctx, OnField, CT_FILE, (intptr_t)fdopen(p2c[1], "w")));
			close(p2c[0]);
		}
		if(emode == M_PIPE) {
			DBG_P("efp is set");
			KFieldSet(proc, proc->efp, (kFile *)KLIB new_kObject(kctx, OnField, CT_FILE, (intptr_t)fdopen(err[0], "r")));
			close(err[1]);
		}
	}
	return pid;
}


// for wait & fg & exec & communicate
/**
 * @return termination status of a child process
 */
static int kSubproc_wait(KonohaContext *kctx, kSubproc *proc, KTraceInfo *trace)
{
	pid_t pid   = proc->cpid;
	int timeout = proc->timeout;
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
				KTraceApi(trace, SystemFault, "setitimer", LogErrno);
			}
			return S_TIMEOUT;
		}
	}
	if(!kSubproc_is(Background, proc)/*bg != 1*/) {
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
	if(!kSubproc_is(Background, proc)/*bg != 1*/) {
		// SIGINT release
		if(keyInt_oldset != SIG_ERR) {
			ret = signal(SIGINT, keyInt_oldset);
		}
		else {
			ret = signal(SIGINT, SIG_DFL);
		}
		if(ret == SIG_ERR) {
			// todo: error
		}
	}
	proc->status = stat;
	// return value creation
	if(WIFSIGNALED(stat)) {
		return WTERMSIG(stat) * -1;
	}
	else if(WIFSTOPPED(stat) ) {
		return WSTOPSIG(stat) * -1;
	}
	else {
		return S_EXIT;
	}
}

// for fg & bg & restart
/**
 * @return if foreground, returns start status of the child
 *         if background, returns termination status of a child
 */
static int kSubproc_start(KonohaContext *kctx, kSubproc *proc, KTraceInfo *trace) {
	int ret = S_PREEXECUTION;
	int pid = kSubproc_popen(kctx, proc, M_NREDIRECT, trace);
	if(pid > 0) {
		proc->cpid  = pid;
		kSubproc_set(OnExec, proc, true);
		if(!kSubproc_is(Background, proc)) {
			// foreground
			ret = kSubproc_wait(kctx, proc, trace);
		}
		else {
			// background
			// nomal end status for bg
			ret = 0;
		}
	}
	else {
		perror("popen failed");
	}
	return ret;
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

static kString *kFile_readAll(KonohaContext *kctx, kFile *file, KTraceInfo *trace)
{
	FILE *fp = file->fp;
	char buf[K_PAGESIZE];
	KGrowingBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	while(1) {
		size_t size = fread(buf, 1, sizeof(buf), fp);
		if(size > 0) {
			KLIB Kwb_write(kctx, &wb, buf, size);
		}
		else {
			break;
		}
	}
	if(ferror(fp)) {
		KTraceApi(trace, SoftwareFault|UserFault, "fread");
		clearerr(fp);
		fclose(fp);
		file->fp = NULL;
		return KNULL(String);
	}
	kString *ret = KLIB new_kString(kctx, GcUnsafe, KLIB Kwb_top(kctx, &wb, 0), Kwb_bytesize(&wb), 0);
	KLIB Kwb_free(&wb);
	fclose(fp);
	file->fp = NULL;
	return ret;
}

static kString *kSubproc_checkOutput(KonohaContext *kctx, kSubproc *p, kString *command, KTraceInfo *trace)
{
	kSubproc *proc = (kSubproc *)p;
	kString *ret_s = KNULL(String);
	if(kSubproc_is(OnExec, proc)) {
		return ret_s;
	}
	kSubproc_set(TimeoutKill, proc, false);
	KFieldSet(proc, proc->command, command);
	int pid = kSubproc_popen(kctx, proc, M_PIPE, trace);
	if(pid > 0) {
		proc->cpid = pid;
		if(kSubproc_wait(kctx, proc, trace) == S_TIMEOUT) {
			kSubproc_set(TimeoutKill, proc, true);
			killWait(pid);
			//todo: error
		}
		else if((proc->rmode == M_PIPE) || (proc->rmode == M_DEFAULT) ) {
			ret_s = kFile_readAll(kctx, proc->rfp, trace);
			if(IS_NULL(ret_s)) {
				// todo: error
			}
		}
		else if(proc->rmode == M_FILE) {
			char *msg = " will be ignored.";
			char *cmd = (char *)command;
			char mbuf[strlen(msg)+strlen(cmd)+1];
			snprintf(mbuf, sizeof(mbuf), "'%s'%s", cmd, msg);
		}
	}
	else {
		//toro popen error
	}
	// remove alarm
	struct itimerval val;
	getitimer(ITIMER_REAL, &val);
	val.it_value.tv_sec = 0;
	setitimer(ITIMER_REAL, &val, NULL);
	return ret_s;
}

static kArray *kSubproc_communicate(KonohaContext *kctx, kSubproc *proc, kString *input, KTraceInfo *trace)
{
	kArray *ret_a = (kArray *)KLIB new_kObject(kctx, OnStack, CT_p0(kctx, CT_Array, TY_String), 0);
	if(kSubproc_is(OnExec, proc)) {
		if(proc->wmode == M_PIPE) {
			// The measure against panic,
			// if "Broken Pipe" is detected at the time of writing.
#ifndef __APPLE__
			__sighandler_t oldset = signal(SIGPIPE, SIG_IGN);
#else
			sig_t oldset = signal(SIGPIPE, SIG_IGN);
#endif
			if(S_size(input) > 0 && !fwrite(S_text(input), sizeof(char), S_size(input), proc->wfp->fp) > 0) {
				KTraceApi(trace, SystemFault, "fwrite", LogErrno);
			}
			fclose(proc->wfp->fp);
			proc->wfp->fp = NULL;
			if(oldset != SIG_ERR) {
				signal(SIGPIPE, oldset);
			}
		}
		if(kSubproc_wait(kctx, proc, trace) == S_TIMEOUT) {
			kSubproc_set(TimeoutKill, proc, true);
			killWait(proc->cpid);
			//TODO: raise error
		}
		else {
			if(proc->rmode == M_PIPE) {
				kString *readstr = kFile_readAll(kctx, proc->rfp, trace);
				if(IS_NULL(readstr)) {
					//TODO;  raise error
				}
				KLIB kArray_add(kctx, ret_a, readstr);
			}
			else {
				KLIB kArray_add(kctx, ret_a, KNULL(String));
			}
			if(proc->emode == M_PIPE) {
				kString *readstr = kFile_readAll(kctx, proc->efp, trace);
				if(IS_NULL(readstr)) {
					// TODO: raise error
				}
				KLIB kArray_add(kctx, ret_a, readstr);
			}
			else {
				KLIB kArray_add(kctx, ret_a, KNULL(String));
			}
		}
	}
	return ret_a;
}

/* ------------------------------------------------------------------------ */

//## Subproc Subproc.new(String command);
static KMETHOD Subproc_new1(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	KFieldSet(proc, proc->command, sfp[1].asString);
	// By default, closefds is set to false
	// (file descriptor will not be closed automatically).
	kSubproc_set(CloseFds, proc, false);
	KReturn(proc);
}

//## Subproc Subproc.new(String command, boolean closefds);
static KMETHOD Subproc_new2(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	KFieldSet(proc, proc->command, sfp[1].asString);
	if(sfp[2].boolValue) {
		kSubproc_set(CloseFds, proc, true);
	}
	KReturn(proc);
}

//## boolean Subproc.bg();
static KMETHOD Subproc_bg(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	int ret = -1;
	if(!kSubproc_is(OnExec, proc)) {
		KMakeTrace(trace, sfp);
		kSubproc_set(TimeoutKill, proc, false);
		kSubproc_set(Background, proc, true);
		if((ret = kSubproc_start(kctx, proc, trace)) != 0 ) {
			// todo : proc_strat error
		}
	}
	KReturnUnboxValue(ret == 0);
}

//## int Subproc.fg();
static KMETHOD Subproc_fg(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	int ret = S_PREEXECUTION;
	if(!kSubproc_is(OnExec, proc)) {
		KMakeTrace(trace, sfp);
		kSubproc_set(TimeoutKill, proc, false);
		kSubproc_set(Background, proc, false);
		if((ret = kSubproc_start(kctx, proc, trace)) == S_TIMEOUT ) {
			kSubproc_set(TimeoutKill, proc, true);
			killWait(proc->cpid);
		}
	}
	KReturnUnboxValue(ret);
}

//## String Subproc.exec(String data);
static KMETHOD Subproc_exec(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	KReturn(kSubproc_checkOutput(kctx, (kSubproc *)sfp[0].asObject, sfp[1].asString, trace));
}

//## String[] Subproc.communicate();
static KMETHOD Subproc_communicate0(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	kString *input = KNULL(String);
	KMakeTrace(trace, sfp);
	KReturn(kSubproc_communicate(kctx, proc, input, trace));
}

//## String[] Subproc.communicate(String input);
static KMETHOD Subproc_communicate1(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	kString *input = sfp[1].asString;
	KMakeTrace(trace, sfp);
	KReturn(kSubproc_communicate(kctx, proc, input, trace));
}

//## @Restricted boolean Subproc.enableShellmode(boolean isShellmode);
static KMETHOD Subproc_enableShellmode(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	int ret = !kSubproc_is(OnExec, proc);
	if(ret) {
		if(sfp[1].boolValue) {
			kSubproc_set(Shell, proc, true);
		}
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.setEnv(Map env);
//KMETHOD Subproc_setEnv(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kSubproc *sp = (kSubproc *)sfp[0].asObject;
//	subprocData_t *p = sp->spd;
//	int ret = !kSubproc_is(OnExec, p);
//	if(ret ) {
//		kDictMap *env = (kDictMap *)sfp[1].asObject;
//		int i;
//		size_t msize = env->spi->size(ctx, env->mapptr);
//		if(p->env != (kArray *)KNH_NULVAL(TY_Array) ) {
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
//	KReturnUnboxValue( ret );
//}

//## boolean Subproc.setCwd(String cwd);
static KMETHOD Subproc_setCwd(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	int ret = !kSubproc_is(OnExec, proc);
	if(ret) {
		KFieldSet(proc, proc->cwd, KLIB new_kString(kctx, OnField, S_text(sfp[1].asString), S_size(sfp[1].asString), 0));
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.setFileIN(File in);
static KMETHOD Subproc_setFileIN(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	int ret = !kSubproc_is(OnExec, proc);
	if(ret) {
		kFile *kfile = (kFile *)sfp[1].asObject;
		proc->wmode = M_FILE;
		KFieldSet(proc, proc->wfp, kfile);
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.setFileOUT(File out);
KMETHOD Subproc_setFileOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	int ret = !kSubproc_is(OnExec, proc);
	if(ret) {
		kFile *kfile = (kFile *)sfp[1].asObject;
		proc->rmode = M_FILE;
		KFieldSet(proc, proc->rfp, kfile);
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.setFileERR(File err);
KMETHOD Subproc_setFileERR(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	int ret = !kSubproc_is(OnExec, proc);
	if(ret) {
		kFile *kfile = (kFile *)sfp[1].asObject;
		proc->emode = M_FILE;
		KFieldSet(proc, proc->efp, kfile);
	}
	KReturnUnboxValue(ret);
}

//## FILE Subproc.getFileIN();
KMETHOD Subproc_getFileIN(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	KReturn(proc->wfp);
}

//## FILE Subproc.getFileOUT();
KMETHOD Subproc_getFileOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	KReturn(proc->rfp);
}

//## FILE Subproc.getFileERR();
KMETHOD Subproc_getFileERR(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	KReturn(proc->efp);
}

//## int Subproc.getPid();
static KMETHOD Subproc_getPid(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(proc->cpid);
}

//## int Subproc.getTimeout();
static KMETHOD Subproc_getTimeout(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(proc->timeout);
}

//## int Subproc.getReturncode();
static KMETHOD Subproc_getReturncode(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(proc->status);
}

//## boolean Subproc.enablePipemodeIN(boolean isPipemode);
static KMETHOD Subproc_enablePipemodeIN(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	int ret = !kSubproc_is(OnExec, proc);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			proc->wmode = M_PIPE;
		}
		else if(proc->wmode == M_PIPE) {
			proc->wmode = M_DEFAULT;
		}
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.enablePipemodeOUT(boolean isPipemode);
static KMETHOD Subproc_enablePipemodeOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	int ret = !kSubproc_is(OnExec, proc);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			proc->rmode = M_PIPE;
		}
		else if(proc->rmode == M_PIPE) {
			proc->rmode = M_DEFAULT;
		}
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.enablePipemodeERR(boolean isPipemode);
static KMETHOD Subproc_enablePipemodeERR(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	int ret = !kSubproc_is(OnExec, proc);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			proc->emode = M_PIPE;
		}
		else if(proc->emode == M_PIPE) {
			proc->emode = M_DEFAULT;
		}
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.enableStandardIN(boolean isStandard);
static KMETHOD Subproc_enableStandardIN(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	int ret = !kSubproc_is(OnExec, proc);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			proc->wmode = M_NREDIRECT;
		}
		else if(proc->wmode == M_NREDIRECT) {
			proc->wmode = M_DEFAULT;
		}
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.enableStandardOUT(boolean isStandard);
static KMETHOD Subproc_enableStandardOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	int ret = !kSubproc_is(OnExec, proc);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			proc->rmode = M_NREDIRECT;
		}
		else if(proc->rmode == M_NREDIRECT) {
			proc->rmode = M_DEFAULT;
		}
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.enableStandardERR(boolean isStandard);
static KMETHOD Subproc_enableStandardERR(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	int ret = !kSubproc_is(OnExec, proc);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			proc->emode = M_NREDIRECT;
		}
		else if(proc->emode == M_NREDIRECT) {
			proc->emode = M_DEFAULT;
		}
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.enableERR2StdOUT(boolean isStdout);
static KMETHOD Subproc_enableERR2StdOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	int ret = !kSubproc_is(OnExec, proc);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			proc->emode = M_STDOUT;
		}
		else if(proc->emode == M_STDOUT) {
			proc->emode = M_DEFAULT;
		}
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.isShellmode();
static KMETHOD Subproc_isShellmode(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(kSubproc_is(Shell, proc));
}

//## boolean Subproc.isPipemodeIN();
static KMETHOD Subproc_isPipemodeIN(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(proc->wmode == M_PIPE);
}

//## boolean Subproc.isPipemodeOUT();
static KMETHOD Subproc_isPipemodeOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(proc->rmode == M_PIPE);
}

//## boolean Subproc.isPipemodeERR();
static KMETHOD Subproc_isPipemodeERR(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(proc->emode == M_PIPE);
}

//## boolean Subproc.isStandardIN();
static KMETHOD Subproc_isStandardIN(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(proc->wmode == M_NREDIRECT);
}

//## boolean Subproc.isStandardOUT();
static KMETHOD Subproc_isStandardOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(proc->rmode == M_NREDIRECT);
}

//## boolean Subproc.isStandardERR();
static KMETHOD Subproc_isStandardERR(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(proc->emode == M_NREDIRECT);
}

//## boolean Subproc.isERR2StdOUT();
static KMETHOD Subproc_isERR2StdOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *proc = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(proc->emode == M_STDOUT);
}

/* ------------------------------------------------------------------------ */

#define _Public    kMethod_Public
#define _Im        kMethod_Immutable
#define _F(F)      (intptr_t)(F)

#define TY_Subproc cSubproc->typeId

static kbool_t subproc_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	KRequireKonohaCommonModule(trace);
	KRequirePackage("konoha.file", trace);

	KDEFINE_CLASS defSubproc = {
		STRUCTNAME(Subproc),
		.cflag    = kClass_Final,
		.init     = kSubproc_init,
		.reftrace = kSubproc_reftrace,
	};

	KonohaClass *cSubproc = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defSubproc, trace);
	KonohaClass *CT_StringArray0 = CT_p0(kctx, CT_Array, TY_String);
	ktype_t TY_StringArray = CT_StringArray0->typeId;

	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Subproc_new1), TY_Subproc, TY_Subproc, MN_("new"), 1, TY_String, FN_("command"),
		_Public, _F(Subproc_new2), TY_Subproc, TY_Subproc, MN_("new"), 2, TY_String, FN_("command"), TY_boolean, FN_("closefds"),
		_Public, _F(Subproc_fg), TY_int, TY_Subproc, MN_("fg"), 0,
		_Public, _F(Subproc_bg), TY_boolean, TY_Subproc, MN_("bg"), 0,
		_Public, _F(Subproc_exec), TY_String, TY_Subproc, MN_("exec"), 1, TY_String, FN_("command"),
		_Public, _F(Subproc_communicate0), TY_StringArray, TY_Subproc, MN_("communicate"), 0,
		_Public, _F(Subproc_communicate1), TY_StringArray, TY_Subproc, MN_("communicate"), 1, TY_String, FN_("input"),
//		_Public, _F(Subproc_poll), TY_int, TY_Subproc, MN_("poll"), 0,
//		_Public, _F(Subproc_wait), TY_int, TY_Subproc, MN_("wait"), 0,
//		_Public, _F(Subproc_sendSignal), TY_boolean, TY_Subproc, MN_("sendSignal"), 1 TY_int, FN_("signal"),
//		_Public, _F(Subproc_terminate), TY_boolean, TY_Subproc, MN_("terminate"), 0,
		_Public|_Im, _F(Subproc_getPid), TY_int, TY_Subproc, MN_("getPid"), 0,
		_Public, _F(Subproc_enableShellmode), TY_boolean, TY_Subproc, MN_("enableShellmode"), 1, TY_boolean, FN_("isShellmode"),
		_Public, _F(Subproc_enablePipemodeIN), TY_boolean, TY_Subproc, MN_("enablePipemodeIN"), 1, TY_boolean, FN_("isPipemode"),
		_Public, _F(Subproc_enablePipemodeOUT), TY_boolean, TY_Subproc, MN_("enablePipemodeOUT"), 1, TY_boolean, FN_("isPipemode"),
		_Public, _F(Subproc_enablePipemodeERR), TY_boolean, TY_Subproc, MN_("enablePipemodeERR"), 1, TY_boolean, FN_("isPipemode"),
		_Public, _F(Subproc_enableStandardIN), TY_boolean, TY_Subproc, MN_("enableStandardIN"), 1, TY_boolean, FN_("isStandard"),
		_Public, _F(Subproc_enableStandardOUT), TY_boolean, TY_Subproc, MN_("enableStandardOUT"), 1, TY_boolean, FN_("isStandard"),
		_Public, _F(Subproc_enableStandardERR), TY_boolean, TY_Subproc, MN_("enableStandardERR"), 1, TY_boolean, FN_("isStandard"),
		_Public, _F(Subproc_enableERR2StdOUT), TY_boolean, TY_Subproc, MN_("enableERR2StdOUT"), 1, TY_boolean, FN_("isStdout"),
		_Public|_Im, _F(Subproc_isShellmode), TY_boolean, TY_Subproc, MN_("isShellmode"), 0,
		_Public|_Im, _F(Subproc_isPipemodeIN), TY_boolean, TY_Subproc, MN_("isPipemodeIN"), 0,
		_Public|_Im, _F(Subproc_isPipemodeOUT), TY_boolean, TY_Subproc, MN_("isPipemodeOUT"), 0,
		_Public|_Im, _F(Subproc_isPipemodeERR), TY_boolean, TY_Subproc, MN_("isPipemodeERR"), 0,
		_Public|_Im, _F(Subproc_isStandardIN), TY_boolean, TY_Subproc, MN_("isStandardIN"), 0,
		_Public|_Im, _F(Subproc_isStandardOUT), TY_boolean, TY_Subproc, MN_("isStandardOUT"), 0,
		_Public|_Im, _F(Subproc_isStandardERR), TY_boolean, TY_Subproc, MN_("isStandardERR"), 0,
		_Public|_Im, _F(Subproc_isERR2StdOUT), TY_boolean, TY_Subproc, MN_("isERR2StdOUT"), 0,
		_Public, _F(Subproc_setCwd), TY_boolean, TY_Subproc, MN_("setCwd"), 1, TY_String, FN_("cwd"),
		_Public, _F(Subproc_setFileIN), TY_boolean, TY_Subproc, MN_("setFileIN"), 1, TY_FILE, FN_("file"),
		_Public, _F(Subproc_setFileOUT), TY_boolean, TY_Subproc, MN_("setFileOUT"), 1, TY_FILE, FN_("file"),
		_Public, _F(Subproc_setFileERR), TY_boolean, TY_Subproc, MN_("setFileERR"), 1, TY_FILE, FN_("file"),
		_Public, _F(Subproc_getFileIN), TY_FILE, TY_Subproc, MN_("getFileIN"), 0,
		_Public, _F(Subproc_getFileOUT), TY_FILE, TY_Subproc, MN_("getFileOUT"), 0,
		_Public, _F(Subproc_getFileERR), TY_FILE, TY_Subproc, MN_("getFileERR"), 0,
		_Public|_Im, _F(Subproc_getTimeout), TY_int, TY_Subproc, MN_("getTimeout"), 0,
		_Public|_Im, _F(Subproc_getReturncode), TY_int, TY_Subproc, MN_("getReturncode"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t subproc_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* subproc_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("subproc", "1.0"),
		.initPackage    = subproc_initPackage,
		.setupPackage   = subproc_setupPackage,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
