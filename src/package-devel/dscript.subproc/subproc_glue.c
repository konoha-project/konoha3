/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
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
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>

#define USE_FILE 1
#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/konoha_common.h>
#include <konoha3/import/methoddecl.h>

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

typedef struct {
	int mode;                              // the kind of identifier
	FILE* fp;                              // file stream pointer
} pfd_t;                                   // pipe fd structure

/*
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
*/

#define SUBPROC_CLOSEFDS         ((kshortflag_t)(1<<0))
#define SUBPROC_BACKGROUND       ((kshortflag_t)(1<<1))
#define SUBPROC_SHELL            ((kshortflag_t)(1<<2))

#define SUBPROC_IsCloseFds(P)    (FLAG_is((P)->flag, SUBPROC_CLOSEFDS))
#define SUBPROC_IsBackground(P)  (FLAG_is((P)->flag, SUBPROC_BACKGROUND))
#define SUBPROC_IsShell(P)       (FLAG_is((P)->flag, SUBPROC_SHELL))

#define SUBPROC_SetCloseFds(P)   (FLAG_Set((P)->flag, SUBPROC_CLOSEFDS))
#define SUBPROC_SetBackground(P) (FLAG_Set((P)->flag, SUBPROC_BACKGROUND))
#define SUBPROC_SetShell(P)      (FLAG_Set((P)->flag, SUBPROC_SHELL))

//#define SUBPROC_unsetCloseFds(P) (FLAG_unset((P)->flag, SUBPROC_CLOSEFDS))
#define SUBPROC_unsetBackground(P) (FLAG_unset((P)->flag, SUBPROC_BACKGROUND))
//#define SUBPROC_unsetShell(P) (FLAG_unset((P)->flag, SUBPROC_SHELL))

typedef struct kSubprocVar kSubproc;

struct kSubprocVar {
	kObjectHeader h;
	kshortflag_t   flag;
	//	kbool_t shell;                     // shell mode [true/false]
	//	kbool_t closefds;                  // closefds   [true/false]
	//	kbool_t bg;                        // bg mode    [true/false]
	kArray  *env;                           // child process environment
	kString *command;                      // child process command
	kString *cwd;                          // child process current working directory
	int   rmode;
	FILE *rfp;                             // child process output identifier
	int   wmode;
	FILE  *wfp;                            // child process input identifier
	int   emode;
	FILE  *efp;                            // child process error identifier
	int cpid;                              // child process ID
	int bufferSize;                        // buffer size (unused)
	int timeout;                           // child process timeout value
	int status;                            // waitpid status
	int timeoutKill;                       // child process Timeout ending flag [true/false]
	//SUBPROC_RESOURCEMON_INSTANCE;
};

// TRACE



// SubProc is new one

#define R     0
#define W     1

//#define kSubProc_is(P, S)            (KFlag_Is(uintptr_t, (S)->h.magicflag, kSubProcFlag_##P))
#define kSubProc_Set(P, S, T)         KFlag_Set(uintptr_t,(S)->h.magicflag, kSubProcFlag_##P, T)

//#define kSubProcFlag_CLOSEFDS         ((kshortflag_t)(1<<0))
#define kSubProcFlag_RunningBackground       ((kshortflag_t)(1<<1))
//#define kSubProcFlag_SHELL            ((kshortflag_t)(1<<2))

typedef struct kSubProcVar kSubProc;

struct kSubProcVar {
	kObjectHeader h;
	kshortflag_t   flag;
	kString *Command;
	kArray  *ArgumentList;
	kFile   *InNULL;
	kFile   *OutNULL;
	kFile   *ErrNULL;
	int childProcessId;
	int status;
};

static void kSubProc_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kSubProc *sbp = (kSubProc *)o;
	sbp->flag        = 0;
	KFieldInit(sbp, sbp->Command, KNULL(String));
	KFieldInit(sbp, sbp->ArgumentList, K_EMPTYARRAY);
	sbp->InNULL  = NULL;
	sbp->OutNULL = NULL;
	sbp->ErrNULL = NULL;
}

static void kSubProc_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kSubProc *sbp = (kSubProc *)o;
	KRefTrace(sbp->Command);
	KRefTrace(sbp->ArgumentList);
	KRefTraceNullable(sbp->InNULL);
	KRefTraceNullable(sbp->OutNULL);
	KRefTraceNullable(sbp->ErrNULL);
}

static void cleanUp_fds(int *fds)
{
	if(fds[0] != -1) close(fds[0]);
	if(fds[1] != -1) close(fds[1]);
}

static int TRACE_pipe2(KonohaContext *kctx, int *fds, int flags, KTraceInfo *trace)
{
	//int val;
	int p[2];
	if( pipe(p) == -1 ) {
		KTraceErrorPoint(trace, SystemFault, "pipe", LogErrno);
		return -1;
	}
//	fprintf(stderr, "??? pid=%d, dfs[0]=%d, fds[1]=%d\n", getpid(), p[0], p[1]);
//	if( (val=fcntl(p[0], F_GETFL, 0)) == -1 ) {
//		goto L_ERR;
//	}
//	if( fcntl(p[0], F_SETFL, val | flags) == -1 ) {
//		goto L_ERR;
//	}
//	if( (val=fcntl(p[1], F_GETFL, 0)) == -1 ) {
//		goto L_ERR;
//	}
//	if( fcntl(p[1], F_SETFL, val | flags) == -1 ) {
//		goto L_ERR;
//	}
	fds[0] = p[0];
	fds[1] = p[1];
	return 0;
//L_ERR:;
//	{
//		KTraceErrorPoint(trace, SystemFault, "fcntl", LogErrno);
//		cleanUp_fds(p);
//		return -1;
//	}
}

static void dup2_or_exit(int fd, int fd2)
{
	//fprintf(stderr, ">>>>> pid=%d, fd=%d, fd2=%d\n", getpid(), fd, fd2);
	if(fd != fd2) {
		close(fd2);
		if(dup2(fd, fd2) == -1) {
			_exit(1);  // terminate calling process
		}
	}
}

static kFile *new_PipeFile(KonohaContext *kctx, kArray *gcstack, int fd, const char *mode, kString *command, KTraceInfo *trace)
{
	FILE *fp = fdopen(fd, mode);
	if(fp == NULL) {
		KTraceErrorPoint(trace, SystemFault, "fdopen", LogUint("fildes", fd), LogText("mode", mode), LogErrno);
	}
	struct kFileVar *file = (struct kFileVar *)KLIB new_kObject(kctx, gcstack, KClass_File, (uintptr_t)fp);
	KFieldInit(file, file->PathInfoNULL, command);
	if(!PLATAPI I18NModule.isSystemCharsetUTF8(kctx)) {
		if(mode[0] == 'r') {
			file->readerIconv = PLATAPI I18NModule.iconvSystemCharsetToUTF8(kctx, trace);
		}
		else {
			file->writerIconv = PLATAPI I18NModule.iconvUTF8ToSystemCharset(kctx, trace);
		}
	}
	kFile_Set(ChangeLessStream, file, true);
	return file;
}

static void kSubProc_execOnChild(KonohaContext *kctx, kSubProc *sbp, KTraceInfo *trace)
{
	size_t i;
	char *args[kArray_size(sbp->ArgumentList) + 2];
	args[0] = (char *)kString_text(sbp->Command);
	args[kArray_size(sbp->ArgumentList) + 1] = NULL;
	for(i = 0; i < kArray_size(sbp->ArgumentList); i++) {
		args[i+1] = (char *)kString_text(sbp->ArgumentList->NodeItems[i]);
	}
	KTraceChangeSystemPoint(trace, "execvp", LogText("command", args[0]), LogTextArray("argv", args), LogUint("pid", getpid()));
	//	shell mode execlp("sh", "sh", "-c", kString_text(command), NULL);
	if(execvp(args[0], args) == -1) {
		int fault = KLIB DiagnosisFaultType(kctx, kString_GuessUserFault(sbp->Command)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "execvp", LogText("command", args[0]), LogTextArray("argv", args), LogErrno);
		_exit(1);
	}
}

static kbool_t ignoreSigchld(KonohaContext *kctx, KTraceInfo *trace)
{
	/* Ignore SIGCHLD signale to prevent zombie process. */
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = SA_NOCLDWAIT;
	if(sigaction(SIGCHLD, &sa, NULL) == -1) {
		KTraceApi(trace, SystemFault, "sigaction", LogErrno);
		return false;
	}
	return true;
}

static int kSubProc_exec(KonohaContext *kctx, kSubProc *sbp, KTraceInfo *trace)
{
	int c2p[2] = {-1, -1}, p2c[2] = {-1, -1}, errPipe[2] = {-1, -1};
	if(sbp->InNULL == NULL) {
		if(TRACE_pipe2(kctx, p2c, O_NONBLOCK, trace) != 0) {
			return -1;
		}
	}
	if(sbp->OutNULL == NULL) {
		if(TRACE_pipe2(kctx, c2p, O_NONBLOCK, trace) != 0) {
			cleanUp_fds(p2c);
			return -1;
		}
	}
	if(sbp->ErrNULL == NULL) {
		if(TRACE_pipe2(kctx, errPipe, O_NONBLOCK, trace) != 0) {
			cleanUp_fds(c2p);
			cleanUp_fds(p2c);
			return -1;
		}
	}
	pid_t pid = fork();
	switch(pid) {
	case -1: { // parent process illegal route
		KTraceErrorPoint(trace, SystemFault, "fork", LogErrno);
		cleanUp_fds(c2p);
		cleanUp_fds(p2c);
		cleanUp_fds(errPipe);
	}
	break;
	case 0: // child process normal route
		if(sbp->InNULL != NULL) {
			dup2_or_exit(fileno(sbp->InNULL->fp), 0);
		}
		else {
			dup2_or_exit(p2c[R], 0);
			cleanUp_fds(p2c);
		}
		if(sbp->OutNULL != NULL) {
			dup2_or_exit(fileno(sbp->OutNULL->fp), 1);
		}
		else {
			dup2_or_exit(c2p[W], 1);
			cleanUp_fds(c2p);
		}
		if(sbp->ErrNULL != NULL) {
			dup2_or_exit(fileno(sbp->ErrNULL->fp), 2);
		}
		else {
			dup2_or_exit(errPipe[W], 2);
			cleanUp_fds(errPipe);
		}
		{
			int cfd, maxfd = sysconf(_SC_OPEN_MAX);
			for(cfd = 3; cfd < maxfd; cfd++) {
				close(cfd);  // close fildes except for 0, 1, and 2
			}
		}
		if(!KonohaContext_Is(Interactive, kctx)) {
			setsid(); // separate from tty
			// prevent child process from getting tty again
			// FIXME: if fork() is enabiled, cannot get exit status correctly.
			//if(fork()) {
			//	exit(0);
			//}
			//TODO: if ignoreSigchld is disabled, zombie process may be created
			//ignoreSigchld(kctx, trace);
		}
//		if(!IS_NULL(sp->cwd)) { // TODO!!
//			if(chdir(kString_text((sp->cwd))) != 0) {
//				//TODO: trace
//				_exit(1);
//			}
//		}
		kSubProc_execOnChild(kctx, sbp, trace);
		break;
	default:
//		fprintf(stderr, "??? pid=%d, p2c[0]=%d, p2c[1]=%d\n", getpid(), p2c[0], p2c[1]);
//		fprintf(stderr, "??? pid=%d, c2p[0]=%d, c2p[1]=%d\n", getpid(), c2p[0], c2p[1]);
//		fprintf(stderr, "??? pid=%d, err[0]=%d, err[1]=%d\n", getpid(), errPipe[0], errPipe[1]);
		if(sbp->InNULL == NULL) {
			KFieldInit(sbp, sbp->InNULL, new_PipeFile(kctx, OnField, p2c[W], "w", sbp->Command, trace));
			close(p2c[R]);
		}
		if(sbp->OutNULL == NULL) {
			KFieldInit(sbp, sbp->OutNULL, new_PipeFile(kctx, OnField, c2p[R], "r", sbp->Command, trace));
			close(c2p[W]);
		}
		if(sbp->ErrNULL == NULL) {
			KFieldInit(sbp, sbp->ErrNULL, new_PipeFile(kctx, OnField, errPipe[R], "r", sbp->Command, trace));
			close(errPipe[W]);
		}
	}
	return pid;
}

static void kSubProc_wait(KonohaContext *kctx, kSubProc *sbp, int pid, KTraceInfo *trace)
{
	pid_t childid;
	KTraceResponseCheckPoint(trace, 0, "waitpid",
		childid = waitpid(pid, &sbp->status, WUNTRACED) // is it OK?
	);
	if(childid == -1) {
		KTraceErrorPoint(trace, SystemFault, "waitpid", LogErrno);
	}
}

static kString *kFILE_readAll(KonohaContext *kctx, kArray *gcstack, kFile *file, KTraceInfo *trace)
{
	char buf[K_PAGESIZE];
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	kString *ret = KNULL(String);
	while(1) {
		size_t size = fread(buf, 1, sizeof(buf), file->fp);
		if(size > 0) {
			KLIB KBuffer_Write(kctx, &wb, buf, size);
		}
		else {
			break;
		}
	}
	if(ferror(file->fp)) {
		// We should not use LogErrno here
		KTraceErrorPoint(trace, SystemFault, "fread");
		clearerr(file->fp);
		return ret;
	}
	return KLIB KBuffer_Stringfy(kctx, &wb, gcstack, StringPolicy_FreeKBuffer);
}

static kbool_t checkExecutablePath(KonohaContext *kctx, const char *path, const char *cmd)
{
	char buf[PATH_MAX];
	struct stat sb;
	const char *fullpath;
	if(path != NULL) {
		snprintf(buf, PATH_MAX, "%s/%s", path, cmd);
		fullpath = buf;
	}
	else {
		fullpath = cmd;
	}
	DBG_P("path='%s'", fullpath);
	if(lstat(fullpath, &sb) == -1) {
		return false;
	}
	return (sb.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH));
}

//static int kSubProc_waitWithTimer(KonohaContext *kctx, kSubProc *sbp, KTraceInfo *trace)
//{
//#ifndef __APPLE__
//	__sighandler_t alarm_oldset  = SIG_ERR;
//	__sighandler_t keyInt_oldset = SIG_ERR;
//	__sighandler_t ret = SIG_ERR;
//#else
//	sig_t alarm_oldset  = SIG_ERR;
//	sig_t keyInt_oldset = SIG_ERR;
//	sig_t ret = SIG_ERR;
//#endif
//	if(timeout > 0) {
//		if(sigsetjmp(env, 1)) {
//			// wait timeout return route
//			setitimer(ITIMER_REAL, NULL, NULL);
//			if(alarm_oldset != SIG_ERR) {
//				ret = signal(SIGALRM, alarm_oldset);
//			}
//			else {
//				ret = signal(SIGALRM, SIG_DFL);
//			}
//			if(ret == SIG_ERR) {
//				//todo : tracesetitimer
//			}
//			return S_TIMEOUT;
//		}
//	}
//	if(!FLAG_is(flag, SUBPROC_BACKGROUND)/*bg != 1*/) {
//		// SIGINT registration
//		fgPid = pid;
//		keyInt_oldset = signal(SIGINT, keyIntHandler);
//	}
//	if(timeout > 0) {
//		// SIGALRM registration
//		struct itimerval its;
//		its.it_value.tv_sec  = timeout / 1000;
//		its.it_value.tv_usec = (timeout % 1000) * 1000;
//		its.it_interval.tv_sec  = 0;
//		its.it_interval.tv_usec = 0;
//		setitimer(ITIMER_REAL, &its, NULL);
//		alarm_oldset = signal(SIGALRM, alarmHandler);
//	}
//	int stat;
//	waitpid(pid, &stat, WUNTRACED);
//	if(timeout > 0) {
//		// SIGALRM release
//		struct itimerval its;
//		its.it_value.tv_sec = 0;
//		its.it_value.tv_usec = 0;
//		its.it_interval.tv_sec = 0;
//		its.it_interval.tv_usec = 0;
//		setitimer(ITIMER_REAL, &its, NULL);
//		if(alarm_oldset != SIG_ERR) {
//			signal(SIGALRM, alarm_oldset);
//		}
//		else {
//			signal(SIGALRM, SIG_DFL);
//		}
//	}
//	if(FLAG_is(flag, SUBPROC_BACKGROUND)/*bg != 1*/) {
//		// SIGINT release
//		if(keyInt_oldset != SIG_ERR) {
//			ret = signal(SIGINT, keyInt_oldset);
//		}
//		else {
//			ret = signal(SIGINT, SIG_DFL);
//		}
//		if(ret == SIG_ERR) {
//			// todo: error
//		}
//	}
//	if(status != NULL) {
//		*status = stat;
//	}
//	// return value creation
//	if(WIFSIGNALED(stat)) {
//		return WTERMSIG(stat) * -1;
//	} else if( WIFSTOPPED(stat) ) {
//		return WSTOPSIG(stat) * -1;
//	} else {
//		return S_EXIT;
//	}
//}

/* ------------------------------------------------------------------------ */

//## SubProc SubProc.new(String cmd);
static KMETHOD SubProc_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubProc *sbp = (kSubProc *)sfp[0].asObject;
	KFieldSet(sbp, sbp->Command, sfp[1].asString);
	KReturn(sbp);
}

//## void SubProc.setArgumentList(String[] a);
static KMETHOD SubProc_SetArgumentList(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubProc *sbp = (kSubProc *)sfp[0].asObject;
	KFieldSet(sbp, sbp->ArgumentList, sfp[1].asArray);
	KReturnVoid();
}

//## void SubProc.setInputStream(File f);
static KMETHOD SubProc_SetInputStream(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubProc *sbp = (kSubProc *)sfp[0].asObject;
	kFile *file   = sfp[1].asFile;
	KSafeFieldSet(sbp, sbp->InNULL, file);
	KReturnVoid();
}

//## void SubProc.setOutputStream(File f);
static KMETHOD SubProc_SetOutputStream(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubProc *sbp = (kSubProc *)sfp[0].asObject;
	kFile *file   = sfp[1].asFile;
	KSafeFieldSet(sbp, sbp->OutNULL, file);
	KReturnVoid();
}

//## void SubProc.setErrorStream(File f);
static KMETHOD SubProc_SetErrorStream(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubProc *sbp = (kSubProc *)sfp[0].asObject;
	kFile *file   = sfp[1].asFile;
	KSafeFieldSet(sbp, sbp->ErrNULL, file);
	KReturnVoid();
}

//## FILE SubProc.getInputStream();
static KMETHOD SubProc_getInputStream(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubProc *sbp = (kSubProc *)sfp[0].asObject;
	if(sbp->InNULL == NULL) {
		KReturn(KNULL(File));
	}
	KReturn(sbp->InNULL);
}

//## FILE SubProc.getOutputStream();
static KMETHOD SubProc_getOutputStream(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubProc *sbp = (kSubProc *)sfp[0].asObject;
	if(sbp->OutNULL == NULL) {
		KReturn(KNULL(File));
	}
	KReturn(sbp->OutNULL);
}

//## FILE SubProc.getErrorStream();
static KMETHOD SubProc_getErrorStream(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubProc *sbp = (kSubProc *)sfp[0].asObject;
	if(sbp->ErrNULL == NULL) {
		KReturn(KNULL(File));
	}
	KReturn(sbp->ErrNULL);
}

//## int SubProc.bg();
static KMETHOD SubProc_bg(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubProc *sbp = (kSubProc *)sfp[0].asObject;
	kSubProc_Set(RunningBackground, sbp, true);
	KMakeTrace(trace, sfp);
	int pid = kSubProc_exec(kctx, sbp, trace);
	sbp->childProcessId = pid;
	KReturnUnboxValue(pid);
}

//## void SubProc.fg();
static KMETHOD SubProc_fg(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubProc *sbp = (kSubProc *)sfp[0].asObject;
	KMakeTrace(trace, sfp);
	int pid = kSubProc_exec(kctx, sbp, trace);
	if(pid > 0) {
		kSubProc_wait(kctx, sbp, pid, trace);
	}
	sbp->childProcessId = pid;
	KReturnUnboxValue(pid);
}

//## void SubProc.pipe(SubProc next, boolean WithError);
static KMETHOD SubProc_pipe(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubProc *sbp  = (kSubProc *)sfp[0].asObject;
	kSubProc *sbp2 = (kSubProc *)sfp[1].asObject;
//	if(sbp->InNULL != NULL) {
//		KSafeFieldSet(sbp, sbp2->OutNULL, sbp->InNULL);
//	}
	if(sbp->OutNULL != NULL) {
		KSafeFieldSet(sbp, sbp2->InNULL, sbp->OutNULL);
	}
	if(sfp[2].boolValue && sbp->ErrNULL != NULL) {
		KSafeFieldSet(sbp, sbp2->InNULL, sbp->ErrNULL);
	}
	KReturnVoid();
}

//## String[] SubProc.communicate(String input);
static KMETHOD SubProc_communicate(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubProc *sbp  = (kSubProc *)sfp[0].asObject;
	kString *input = sfp[1].asString;
	KMakeTrace(trace, sfp);
	if(fwrite(kString_text(input), sizeof(char), kString_size(input), sbp->InNULL->fp) > 0) {
		fclose(sbp->InNULL->fp);
		sbp->InNULL->fp = NULL;
	} else {
		KTraceErrorPoint(trace, SystemFault, "fwrite", LogErrno);
	}
	kSubProc_wait(kctx, sbp, sbp->childProcessId, trace);
	INIT_GCSTACK();
	kArray *resultArray = (kArray *)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), 0);
	KLIB kArray_Add(kctx, resultArray, kFILE_readAll(kctx, OnField, sbp->OutNULL, trace));
	KLIB kArray_Add(kctx, resultArray, kFILE_readAll(kctx, OnField, sbp->ErrNULL, trace));
	KReturnWith(resultArray, RESET_GCSTACK());
}

//## int SubProc.getPid();
static KMETHOD SubProc_getPid(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubProc *sbp = (kSubProc *)sfp[0].asObject;
	KReturnUnboxValue(sbp->childProcessId);
}

//## int SubProc.getStatus();
static KMETHOD SubProc_getStatus(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubProc *sbp = (kSubProc *)sfp[0].asObject;
	int status = sbp->status;
	int ret = sbp->status;
	if(WIFEXITED(status)) {
		ret = WEXITSTATUS(status);
	}
	else if(WIFSIGNALED(status)) {
		ret = WTERMSIG(status);
	}
	else if(WIFSTOPPED(status)) {
		ret = WSTOPSIG(status);
	}
	KReturnUnboxValue(ret);
}

//## boolean SubProc.isCommand(String command);
static KMETHOD SubProc_isCommand(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *cmd = kString_text(sfp[1].asString);
	size_t bufsize = confstr(_CS_PATH, NULL, 0);
	char buf[bufsize];
	confstr(_CS_PATH, buf, bufsize);
	char *pos, *p = buf;
	if(cmd[0] == '/' && checkExecutablePath(kctx, NULL, cmd)) {
		KReturnUnboxValue(true);
	}
	while(p < buf + bufsize) {
		if((pos = strchr(p, ':')) == NULL) {
			if(checkExecutablePath(kctx, p, cmd)) {
				KReturnUnboxValue(true);
			}
			break;
		}
		p[pos - p] = '\0';
		if(checkExecutablePath(kctx, p, cmd)) {
			KReturnUnboxValue(true);
		}
		p = pos + 1;
	}
	KReturnUnboxValue(false);
}

static kbool_t subproc_InitSubProc(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_CLASS defSubProc = {
		STRUCTNAME(SubProc),
		.cflag    = KClassFlag_Final,
		.init     = kSubProc_Init,
		.reftrace = kSubProc_Reftrace,
	};
	KClass *cSubproc = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defSubProc, trace);

	kparamtype_t ps = {KType_String, KFieldName_("str")};
	KClass *KClass_StringArray2 = KLIB KClass_Generics(kctx, KClass_Array, KType_String, 1, &ps);
	ktypeattr_t KType_StringArray = KClass_StringArray2->typeId, KType_SubProc = cSubproc->typeId;

	KDEFINE_METHOD MethodData[] = {
		_Public,     _F(SubProc_new),             KType_SubProc, KType_SubProc, KMethodName_("new"), 1, KType_String, KFieldName_("command"),
		_Public,     _F(SubProc_SetArgumentList), KType_void, KType_SubProc, KMethodName_("setArgumentList"), 1, KType_StringArray, KFieldName_("arguments"),
		_Public,     _F(SubProc_SetInputStream),  KType_void, KType_SubProc, KMethodName_("setInputStream"), 1, KType_File, KFieldName_("stream"),
		_Public,     _F(SubProc_SetOutputStream), KType_void, KType_SubProc, KMethodName_("setOutputStream"), 1, KType_File, KFieldName_("stream"),
		_Public,     _F(SubProc_SetErrorStream),  KType_void, KType_SubProc, KMethodName_("setErrorStream"), 1, KType_File, KFieldName_("stream"),
		_Public|_Im, _F(SubProc_getInputStream),  KType_File, KType_SubProc, KMethodName_("getInputStream"), 0,
		_Public|_Im, _F(SubProc_getOutputStream), KType_File, KType_SubProc, KMethodName_("getOutputStream"), 0,
		_Public|_Im, _F(SubProc_getErrorStream),  KType_File, KType_SubProc, KMethodName_("getErrorStream"), 0,
		_Public,     _F(SubProc_pipe),            KType_void, KType_SubProc, KMethodName_("pipe"), 2, KType_SubProc, KFieldName_("next"), KType_Boolean, KFieldName_("error"),
		_Public,     _F(SubProc_fg),              KType_Int, KType_SubProc, KMethodName_("fg"), 0,
		_Public,     _F(SubProc_bg),              KType_void, KType_SubProc, KMethodName_("bg"), 0,
		_Public,     _F(SubProc_communicate),     KType_StringArray, KType_SubProc, KMethodName_("communicate"), 1, KType_String, KFieldName_("input"),
		_Public|_Im, _F(SubProc_getPid),          KType_Int, KType_SubProc, KMethodName_("getPid"), 0,
		_Public|_Im, _F(SubProc_getStatus),       KType_Int, KType_SubProc, KMethodName_("getStatus"), 0,
		_Public|_Static|_Im, _F(SubProc_isCommand), KType_Boolean, KType_SubProc, KMethodName_("isCommand"), 1, KType_String, KFieldName_("command"),

//		_Public|_Im, _F(Subproc_exec), KType_String, KType_Subproc, KMethodName_("exec"), 1, KType_String, KFieldName_("data"),
//		_Public|_Im, _F(Subproc_communicate), KType_StringArray, KType_Subproc, KMethodName_("communicate"), 1, KType_String, KFieldName_("input"),
//		_Public|_Im, _F(Subproc_poll), KType_Int, KType_Subproc, KMethodName_("poll"), 0,
//		_Public|_Im, _F(Subproc_wait), KType_Int, KType_Subproc, KMethodName_("wait"), 0,
//		_Public|_Im, _F(Subproc_sendSignal), KType_Boolean, KType_Subproc, KMethodName_("sendSignal"), 1 KType_Int, KFieldName_("signal"),
//		_Public|_Im, _F(Subproc_terminate), KType_Boolean, KType_Subproc, KMethodName_("terminate"), 0,
//		_Public|_Im, _F(Subproc_getPid), KType_Int, KType_Subproc, KMethodName_("getPid"), 0,
//		_Public|_Im, _F(Subproc_enableShellmode), KType_Boolean, KType_Subproc, KMethodName_("enableShellmode"), 1, KType_Boolean, KFieldName_("isShellmode"),
//		_Public|_Im, _F(Subproc_enablePipemodeIN), KType_Boolean, KType_Subproc, KMethodName_("enablePipemodeIN"), 1, KType_Boolean, KFieldName_("isPipemode"),
//		_Public|_Im, _F(Subproc_enablePipemodeOUT), KType_Boolean, KType_Subproc, KMethodName_("enablePipemodeOUT"), 1, KType_Boolean, KFieldName_("isPipemode"),
//		_Public|_Im, _F(Subproc_enablePipemodeERR), KType_Boolean, KType_Subproc, KMethodName_("enablePipemodeERR"), 1, KType_Boolean, KFieldName_("isPipemode"),
//		_Public|_Im, _F(Subproc_enableStandardIN), KType_Boolean, KType_Subproc, KMethodName_("enableStandardIN"), 1, KType_Boolean, KFieldName_("isStandard"),
//		_Public|_Im, _F(Subproc_enableStandardOUT), KType_Boolean, KType_Subproc, KMethodName_("enableStandardOUT"), 1, KType_Boolean, KFieldName_("isStandard"),
//		_Public|_Im, _F(Subproc_enableStandardERR), KType_Boolean, KType_Subproc, KMethodName_("enableStandardERR"), 1, KType_Boolean, KFieldName_("isStandard"),
//		_Public|_Im, _F(Subproc_isShellmode), KType_Boolean, KType_Subproc, KMethodName_("isShellmode"), 0,
//		_Public|_Im, _F(Subproc_isPipemodeIN), KType_Boolean, KType_Subproc, KMethodName_("isPipemodeIN"), 0,
//		_Public|_Im, _F(Subproc_isPipemodeOUT), KType_Boolean, KType_Subproc, KMethodName_("isPipemodeOUT"), 0,
//		_Public|_Im, _F(Subproc_isPipemodeERR), KType_Boolean, KType_Subproc, KMethodName_("isPipemodeERR"), 0,
//		_Public|_Im, _F(Subproc_isStandardIN), KType_Boolean, KType_Subproc, KMethodName_("isStandardIN"), 0,
//		_Public|_Im, _F(Subproc_isStandardOUT), KType_Boolean, KType_Subproc, KMethodName_("isStandardOUT"), 0,
//		_Public|_Im, _F(Subproc_isStandardERR), KType_Boolean, KType_Subproc, KMethodName_("isStandardERR"), 0,
//		_Public|_Im, _F(Subproc_isERR2StdOUT), KType_Boolean, KType_Subproc, KMethodName_("isERR2StdOUT"), 0,
//		_Public|_Static|_Im, _F(Subproc_call), KType_Int, KType_Subproc, KMethodName_("call"), 1, KType_String, KFieldName_("args"),
//		_Public|_Static|_Im, _F(Subproc_CheckOutput), KType_String, KType_Subproc, KMethodName_("checkOutput"), 1, KType_String, KFieldName_("data"),
//		_Public|_Im, _F(Subproc_SetCwd), KType_Boolean, KType_Subproc, KMethodName_("setCwd"), 1, KType_String, KFieldName_("cwd"),
//		_Public|_Im, _F(Subproc_SetBufsize), KType_Boolean, KType_Subproc, KMethodName_("setBufsize"), 1, KType_Int, KFieldName_("bufsize"),
//		_Public|_Im, _F(Subproc_SetFileIN), KType_Boolean, KType_Subproc, KMethodName_("setFileIN"), 1, KType_FILE, KFieldName_("file"),
//		_Public|_Im, _F(Subproc_SetFileOUT), KType_Boolean, KType_Subproc, KMethodName_("setFileOUT"), 1, KType_FILE, KFieldName_("file"),
//		_Public|_Im, _F(Subproc_SetFileERR), KType_Boolean, KType_Subproc, KMethodName_("setFileERR"), 1, KType_FILE, KFieldName_("file"),
//		_Public|_Im, _F(Subproc_getTimeout), KType_Int, KType_Subproc, KMethodName_("getTimeout"), 0,
//		_Public|_Im, _F(Subproc_getReturncode), KType_Int, KType_Subproc, KMethodName_("getReturncode"), 0,
//		_Public|_Im, _F(Subproc_enableERR2StdOUT), KType_Boolean, KType_Subproc, KMethodName_("enableERR2StdOUT"), 1, KType_Boolean, KFieldName_("isStdout"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

/* ------------------------------------------------------------------------ */
/* [class defs] */

//#define KClass_Subproc         cSubproc
#define KType_Subproc         cSubproc->typeId
//#define IS_Subproc(O)      (kObject_class(O) == KClass_Subproc)

/* ------------------------------------------------------------------------ */
/* [global variables] */

static jmp_buf env;

static void alarmHandler(int sig) {
	siglongjmp(env, 1);
}

static int fgPid;
static void keyIntHandler(int sig) { kill(fgPid, SIGINT); }

/* ------------------------------------------------------------------------ */

// child process IO mode
//#define M_DEFAULT          -2			// initialization state
//#define M_NREDIRECT        -1			// parent process succession
//#define M_PIPE             0			// pipe
//#define M_STDOUT           1			// standard OUT
//#define M_FILE             2			// file

enum {
	M_DEFAULT   = -2,
	M_NREDIRECT = -1,
	M_PIPE      =  0,
	M_STDOUT    =  1,
	M_FILE      =  2
};

// child process status code
//#define S_RUNNING          -300			// running
#define S_PREEXECUTION     -400			// preexecution
#define S_TIMEOUT          -500			// tiomeout
#define S_EXIT             0			// terminate

// subproc macro
#define MAXARGS            128				// the number maximum of parameters for spSplit
#define DELAY              1000				// the adjustment value at the time of signal transmission
#define DEF_TIMEOUT        10 * 1000		// default timeout valx
//#define DEF_TIMEOUT -1
#define ONEXEC(p)          (p->cpid > 0) ? 1 : 0
#define PREEXEC(p)         (p->cpid == -1) ? 1 : 0

/* ------------------------------------------------------------------------ */

#if defined (__APPLE__) || defined(__USE_LOCAL_PIPE2__)
// for fg & bg & exec & restart
static int pipe2( int *fd, int flags ) {
	int val;
	int p[2];
	if( pipe(p) < 0 ) {
		return -1;
	}
	if( (val=fcntl(p[0], F_GETFL, 0)) < 0 ) {
		goto L_ERR;
	}
	if( fcntl(p[0], F_SETFL, val | flags) < 0 ) {
		goto L_ERR;
	}
	if( (val=fcntl(p[1], F_GETFL, 0)) < 0 ) {
		goto L_ERR;
	}
	if( fcntl(p[1], F_SETFL, val | flags) < 0 ) {
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
// if( param > 0 ) {
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

static int spSplit(char* str, char* args[])
{

	if( (str == NULL) || (args == NULL) ) {
		return -1;
	}
	int indx;
	char *cp = str;
	for (indx = 0; indx <= MAXARGS; indx++) {
		if( indx == MAXARGS ) {
			return -2;
		} else if( (args[indx] = strtok(cp, " ")) == NULL ) {
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
//static KClass khn_getFileClass(KonohaContext *kctx) {
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

static int subproc_Popen(KonohaContext *kctx, kString* command, kSubproc *p, int defaultMode)
{
	struct kSubprocVar *sp = (struct kSubprocVar *)p;
	if(IS_NULL(command) || kString_size(command) == 0) {
		return -1;
	}
	pid_t pid  = -1;
	int rmode = (sp->rmode==M_DEFAULT) ? defaultMode : sp->rmode;
	int wmode = (sp->wmode==M_DEFAULT) ? defaultMode : sp->wmode;
	int emode = (sp->emode==M_DEFAULT) ? defaultMode : sp->emode;
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
			// TODO: trace
			close(c2p[0]); close(c2p[1]);
			return -1;
		}
	}

	if(emode == M_PIPE) {
		if(pipe2(err, O_NONBLOCK) != 0) {
			// TODO: trace
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
		if(wmode == M_PIPE){
			close(0);
			if(dup2(p2c[0], 0) == -1) {
				//TODO: trace
			}
			close(p2c[0]); close(p2c[1]);
		}
		else if(wmode == M_FILE) {
			close(0);
			if(dup2(fileno(sp->wfp), 0) == -1) {
				// TODO: trace
			}
		}
		if(rmode == M_PIPE) {
			close(1);
			if(dup2(c2p[1], 1) == -1){
				// TODO: trace dup2
			}
			close(c2p[0]); close(c2p[1]);
		}
		else if(rmode == M_FILE) {
			close(1);
			if(dup2(fileno(sp->rfp), 1) == -1) {
				// TODO: trace
			}
		}
		if(emode == M_PIPE) {
			close(2);
			dup2(err[1], 2);
			close(err[0]); close(err[1]);
		}
		else if(emode == M_FILE) {
			close(2);
			dup2(fileno(sp->efp), 2);
		}
		else if(emode == M_STDOUT) {
			close(2);
			dup2(1, 2);
		}
		if(SUBPROC_IsCloseFds(sp)) {
			// close other than 0, 1, and 2
			int cfd = 3;
			int maxFd = sysconf(_SC_OPEN_MAX);
			do {
				close(cfd);
			} while(++cfd < maxFd);
		}
		setsid(); // separate from tty
		if(!IS_NULL(sp->cwd)) { // TODO!!
			if(chdir(kString_text((sp->cwd))) != 0) {
				//TODO: trace
				_exit(1);
			}
		}
		char *args[MAXARGS];
		//		if(sp->shell == 0) {
		if(!SUBPROC_IsShell(sp)) {
			// division of a commnad parameter
			if(spSplit((char *)kString_text(command), args) < 0){
				//TODO: error
				args[0] = NULL;
			}
		}
		if(!IS_NULL(sp->env)) {
			// division of a environment parameter
			kArray *a = sp->env;
			int num = kArray_size(a);
			char *envs[num+1];
			int i;
			for(i = 0; i < num; i++) {
				envs[i] = (char *)kString_text(a->stringItems[i]);
			}
			envs[num] = NULL;
			// exec load new process image if success.
			// if its not, they will return with -1.
			//			if(sp->shell == 0) {
			if(!SUBPROC_IsShell(sp)) {
				if(execve(args[0], args, envs) == -1) {
					//todo: trace
				}
			}
			else {
				if(execle("/bin/sh", "sh", "-c", kString_text(command), NULL, envs) == -1) {
					//todo: trace
				}
			}
		} else {
			//			if(sp->shell == 0) {
			if(!SUBPROC_IsShell(sp)) {
				if(execvp(args[0], args) == -1) {
					//todo trace
				}
			}
			else {
				if(execlp("sh", "sh", "-c", kString_text(command), NULL) == -1) {
					//todo trace
				}
			}
		}
		perror("subproc_Popen :");
		_exit(1);
	default:
		// parent process normal route
#if defined(SUBPROC_ENABLE_RESOURCEMONITOR)
/		ATTACH_RESOURCE_MONITOR_FOR_CHILD(sp, pid);
//		size_t mem = FETCH_MEM_FROM_RESOURCE_MONITOR(sp);
//		fprintf(stderr, "menusage:%.1fM\n", (double)mem / (1024.0 * 1024.0));
		CLEANUP_RESOURCE_MONITOR(sp);
#endif
		if(rmode == M_PIPE) {
			sp->rfp = fdopen(c2p[0], "r");
			close(c2p[1]);
		}
		if(wmode == M_PIPE) {
			sp->wfp = fdopen(p2c[1], "w");
			close(p2c[0]);
		}
		if(emode == M_PIPE) {
			sp->efp = fdopen(err[0], "r");
			close(err[1]);
		}
	}
	return pid;
}


// for wait & fg & exec & communicate
/**
 * @return termination status of a child process
 */

static int subproc_wait(KonohaContext *kctx, int pid, kshortflag_t flag, int timeout, int *status ) {

#if defined(__linux__)
	__sighandler_t alarm_oldset  = SIG_ERR;
	__sighandler_t keyInt_oldset = SIG_ERR;
	__sighandler_t ret = SIG_ERR;
#elif defined(__APPLE__) || defined(__NetBSD__)
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
				//todo : tracesetitimer
			}
			return S_TIMEOUT;
		}
	}
	if(!FLAG_is(flag, SUBPROC_BACKGROUND)/*bg != 1*/) {
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
	if(FLAG_is(flag, SUBPROC_BACKGROUND)/*bg != 1*/) {
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
	if(status != NULL) {
		*status = stat;
	}
	// return value creation
	if(WIFSIGNALED(stat)) {
		return WTERMSIG(stat) * -1;
	} else if( WIFSTOPPED(stat) ) {
		return WSTOPSIG(stat) * -1;
	} else {
		return S_EXIT;
	}
}

// for fg & bg & restart
/**
 * @return in the case of foreground, it is start status of a child process
 *         in the case of background, it is termination status of a child process
 */

static int proc_start(KonohaContext *kctx, struct kSubprocVar *sp)
{
	int ret = S_PREEXECUTION;
	int pid = subproc_Popen(kctx, sp->command, sp, M_NREDIRECT );
	if(pid > 0) {
		sp->cpid  = pid;
		//		if(sp->bg != 1) {
		if(!SUBPROC_IsBackground(sp)) {
			ret = subproc_wait(kctx, sp->cpid, sp->flag, sp->timeout, &sp->status );
		} else {
			// nomal end status for bg
			ret = 0;
		}
	} else {
		DBG_P("failed");
	}
	return ret;
}

// for new & enablePipemodeXXX(false) & enableStandardXXX(false) & enableERR2StdOUT(false)
static inline void initFd(pfd_t *p) {
	p->mode = M_DEFAULT;
	p->fp   = NULL;
}

// for setFileXXX & PipemodeXXX(true) & enableStandardXXX(true) & enableERR2StdOUT(true)

static void setFd(KonohaContext *kctx, pfd_t *p, int changeMode, FILE* ptr) {
	if(((p->mode == M_PIPE) || (p->mode == M_FILE)) && !(p->mode == changeMode)) {
		// warning of the pipe or file mode overwrite
		//char *msg = (p->mode == M_PIPE) ? "pipe has already set, but we overwrite it." :
		//"file has already set, but we overwrite it.";
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

// for poll
static int getPidStatus(int pid, int *status) {
#if defined(__NetBSD__)
#define WCONTINUED	0
#endif
	return waitpid(pid, status, WNOHANG | WUNTRACED | WCONTINUED);
#if defined(__NetBSD__)
#undef WCONTINUED
#endif
}

// for Subproc_Free & fg & exec & communicate & restart
static void killWait(int pid) {
	int status;
	kill(pid, SIGKILL);
	usleep(DELAY);
	getPidStatus(pid, &status); // this wait is in order not to leave a zombie process.
}

static kbool_t kPipeReadArray(KonohaContext *kctx, kArray *a, FILE *fp)
{
	char buf[K_PAGESIZE];
	while(1) {
		size_t size = fread(buf, 1, sizeof(buf), fp);
		if(size > 0) {
			KLIB kArray_Add(kctx, a, KLIB new_kString(kctx, GcUnsafe, buf, size, 0));
		}
		else {
			break;
		}
	}
	if(ferror(fp) == 0) {
		KLIB kArray_Add(kctx, a, KNULL(String));
		return false;
	}
	return true;
}

static kString *kPipeReadStringNULL(KonohaContext *kctx, FILE *fp)
{
	char buf[K_PAGESIZE];
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	while(1) {
		size_t size = fread(buf, 1, sizeof(buf), fp);
		if(size > 0) {
			KLIB KBuffer_Write(kctx, &wb, buf, size);
		}
		else {
			break;
		}
	}
	if(ferror(fp)) {
		return NULL;
	}
	return KLIB KBuffer_Stringfy(kctx, &wb, OnGcStack, StringPolicy_FreeKBuffer);
}

static kString *kSubproc_CheckOutput(KonohaContext *kctx, kSubproc *p, kString *command)
{
	//	subprocData_t *p = sp->spd;
	struct kSubprocVar *sp = (struct kSubprocVar *)p;
	kString *ret_s = KNULL(String);
	if(PREEXEC(sp)) {
		sp->timeoutKill = 0;
		int pid = subproc_Popen(kctx, command, sp, M_PIPE );
		if(pid > 0 ) {
			if(subproc_wait(kctx, pid, 0, sp->timeout, NULL ) == S_TIMEOUT ) {
				sp->timeoutKill = 1;
				killWait(pid);
				clearFd((pfd_t *)(&sp->rmode));
				clearFd((pfd_t *)&sp->wmode);
				clearFd((pfd_t *)&sp->emode);
				//todo: error
			}
			else if( (sp->rmode == M_PIPE) || (sp->rmode == M_DEFAULT) ) {
				ret_s = kPipeReadStringNULL(kctx, sp->rfp);
				if(ret_s == NULL) {
					// todo: error
					ret_s = KNULL(String);
				}
				clearFd((pfd_t *)&sp->rmode);
				clearFd((pfd_t *)&sp->wmode);
				clearFd((pfd_t *)&sp->emode);
			} else if(sp->rmode == M_FILE) {
				char *msg = " will be ignored.";
				char *cmd = (char *)command;
				char mbuf[strlen(msg)+strlen(cmd)+1];
				snprintf(mbuf, sizeof(mbuf), "'%s'%s", cmd, msg);
			}
		} else {
			//toro popen error
		}
		// remove alarm
		struct itimerval val;
		getitimer(ITIMER_REAL, &val);
		val.it_value.tv_sec = 0;
		setitimer(ITIMER_REAL, &val, NULL);
	}
	return ret_s;
}

/* ------------------------------------------------------------------------ */

//## Subproc Subproc.new(String cmd, boolean closefds);
static KMETHOD Subproc_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	KFieldSet(sp, sp->command, sfp[1].asString);
	if(sfp[2].boolValue) SUBPROC_SetCloseFds(sp);
	KReturn(sp);
}

//## boolean Subproc.bg();
static KMETHOD Subproc_bg(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	int ret = -1;
	if(PREEXEC(sp)) {
		sp->timeoutKill = 0;
		//sp->bg = 1;
		SUBPROC_SetBackground(sp);
		if( (ret = proc_start(kctx, sp)) != 0 ) {
			// todo : proc_strat error
		}
	}
	KReturnUnboxValue(ret == 0);
}

//## int Subproc.fg();
static KMETHOD Subproc_fg(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	int ret = S_PREEXECUTION;
	if(PREEXEC(sp)) {
		sp->timeoutKill = 0;
		//sp->bg = 0;
		SUBPROC_unsetBackground(sp);
		if( (ret = proc_start(kctx, sp)) == S_TIMEOUT ) {
			sp->timeoutKill = 1;
			killWait(sp->cpid);
		}
	}
	KReturnUnboxValue(ret);
}

//## @Throwable String Subproc.exec(String data);
static KMETHOD Subproc_exec(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(kSubproc_CheckOutput(kctx, (kSubproc *)sfp[0].asObject, sfp[1].asString));
}

//## @Throwable String[] Subproc.communicate(String input);
static KMETHOD Subproc_communicate(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *ret_a = KNULL(Array);
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	KMakeTrace(trace, sfp);
	if(ONEXEC(sp)) {
		if((sp->wmode == M_PIPE) && (kString_size(sfp[1].asString) > 0)) {
			kString *s = sfp[1].asString;
			// The measure against panic,
			// if "Broken Pipe" is detected at the time of writing.
#if defined(__linux__)
			__sighandler_t oldset = signal(SIGPIPE, SIG_IGN);
#elif defined(__APPLE__) || defined(__NetBSD__)
			sig_t oldset = signal(SIGPIPE, SIG_IGN);
#endif
			if(fwrite(kString_text(s), sizeof(char), kString_size(s), sp->wfp) > 0) {
//				fputc('\n', p->w.fp);
//				fflush(p->w.fp);
//				fsync(fileno(p->w.fp));
				fclose(sp->wfp);
			} else {
				KTraceApi(trace, SystemFault, "fwrite", LogErrno);
			}
			if(oldset != SIG_ERR) {
				signal(SIGPIPE, oldset);
			}
		}
		if(subproc_wait(kctx, sp->cpid, sp->flag, sp->timeout, &sp->status ) == S_TIMEOUT) {
			sp->timeoutKill = 1;
			killWait(sp->cpid);
			//TODO: raise error
		} else {
			ret_a = (kArray *)KLIB new_kObject(kctx, GcUnsafe, KClass_Array, 0);
			if(sp->rmode == M_PIPE) {
				if(!kPipeReadArray(kctx, ret_a, sp->rfp)) {
					//TODO;  raise error
				}
			}
			else {
				KLIB kArray_Add(kctx, ret_a, KNULL(String));
			}
			if(sp->emode == M_PIPE) {
				if(!kPipeReadArray(kctx, ret_a, sp->efp)) {
					// TODO: raise error
				}
			}
			else {
				KLIB kArray_Add(kctx, ret_a, KNULL(String));
			}
		}
	}
	KReturn(ret_a);
}

//## @Restricted boolean Subproc.enableShellmode(boolean isShellmode);
static KMETHOD Subproc_enableShellmode(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	int ret = PREEXEC(sp);
	if(ret) {
		//		sp->shell = sfp[1].boolValue;
		if(sfp[1].boolValue) SUBPROC_SetShell(sp);
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.setEnv(Map env);
//KMETHOD Subproc_SetEnv(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kSubproc *sp = (kSubproc *)sfp[0].asObject;
//	subprocData_t *p = sp->spd;
//	int ret = PREEXEC(p);
//	if( ret ) {
//		kDictMap *env = (kDictMap *)sfp[1].asObject;
//		int i;
//		size_t msize = env->spi->size(ctx, env->mapptr);
//		if( p->env != (kArray *)KNH_NULVAL(KType_Array) ) {
//			knh_Array_clear( ctx, p->env, 0 );
//		}
//		p->env = new_Array(ctx, KType_String, msize);
//		for (i = 0; i < msize; i++) {
//			kString *key = (kString *)knh_DictMap_keyAt(env, i);
//			kString *val = (kString *)knh_DictMap_valueAt(env, i);
//			char buf[key->str.len + val->str.len + 2];
//			snprintf(buf, sizeof(buf), "%s=%s", key->str.buf, val->str.buf);
//			knh_Array_Add( ctx, p->env, new_String(ctx, buf) );
//		}
//	}
//	KReturnUnboxValue( ret );
//}

//## boolean Subproc.setCwd(String cwd);
static KMETHOD Subproc_SetCwd(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	int ret = PREEXEC(sp);
	if(ret) {
		sp->cwd = KLIB new_kString(kctx, GcUnsafe, kString_text(sfp[1].asString), kString_size(sfp[1].asString), 0);
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.setBufsize(int size);
static KMETHOD Subproc_SetBufsize(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	int ret = PREEXEC(sp);
	if(ret) {
		sp->bufferSize = (int) (sfp[1].intValue);
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.setFileIN(File in);
static KMETHOD Subproc_SetFileIN(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	int ret = PREEXEC(sp);
	if(ret) {
		kFile *kfile = (kFile *)sfp[1].asObject;
		setFd(kctx,(pfd_t *)&sp->wmode, M_FILE, kfile->fp);
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.setFileOUT(File out);
KMETHOD Subproc_SetFileOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	int ret = PREEXEC(sp);
	if(ret) {
		kFile *kfile = (kFile *)sfp[1].asObject;
		setFd(kctx, (pfd_t *)&sp->rmode, M_FILE, kfile->fp);
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.setFileERR(File err);
KMETHOD Subproc_SetFileERR(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	int ret = PREEXEC(sp);
	if(ret) {
			kFile *kfile = (kFile *)sfp[1].asObject;
			setFd(kctx, (pfd_t *)&sp->emode, M_FILE, kfile->fp);
	}
	KReturnUnboxValue(ret);
}

//## int Subproc.getPid();
static KMETHOD Subproc_getPid(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(ONEXEC(sp) == 1 ? sp->cpid : -1 );
}

//## int Subproc.getTimeout();
static KMETHOD Subproc_getTimeout(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(sp->timeout);
}

//## int Subproc.getReturncode();
static KMETHOD Subproc_getReturncode(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(sp->status);
}

//## boolean Subproc.enablePipemodeIN(Boolean isPipemode);
static KMETHOD Subproc_enablePipemodeIN(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	int ret = PREEXEC(sp);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			setFd(kctx, (pfd_t *)&sp->wmode, M_PIPE, NULL);
		} else {
			if(sp->wmode == M_PIPE) {
				initFd((pfd_t *)(&sp->wmode));
			}
		}
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.enablePipemodeOUT(Boolean isPipemode);
static KMETHOD Subproc_enablePipemodeOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	int ret = PREEXEC(sp);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			setFd(kctx, (pfd_t *)&sp->rmode, M_PIPE, NULL);
		} else {
			if(sp->rmode == M_PIPE) {
				initFd((pfd_t *)(&sp->rmode));
			}
		}
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.enablePipemodeERR(Boolean isPipemode);
static KMETHOD Subproc_enablePipemodeERR(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	int ret = PREEXEC(sp);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			setFd(kctx, (pfd_t *)&sp->emode, M_PIPE, NULL);
		}
		else {
			if(sp->emode == M_PIPE) {
				initFd((pfd_t *)&sp->emode);
			}
		}
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.enableStandardIN(Boolean isStandard);
static KMETHOD Subproc_enableStandardIN(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	int ret = PREEXEC(sp);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			setFd(kctx, (pfd_t *)&sp->wmode, M_NREDIRECT, NULL);
		}
		else {
			if(sp->wmode == M_NREDIRECT) {
				initFd((pfd_t *)&sp->wmode);
			}
		}
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.enableStandardOUT(Boolean isStandard);
static KMETHOD Subproc_enableStandardOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	int ret = PREEXEC(sp);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			setFd(kctx, (pfd_t *)&sp->rmode, M_NREDIRECT, NULL);
		} else {
			if(sp->rmode == M_NREDIRECT) {
				initFd((pfd_t *)&sp->rmode);
			}
		}
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.enableStandardERR(Boolean isStandard);
static KMETHOD Subproc_enableStandardERR(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	int ret = PREEXEC(sp);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			setFd(kctx, (pfd_t *)&sp->emode, M_NREDIRECT, NULL);
		} else {
			if(sp->emode == M_NREDIRECT) {
				initFd((pfd_t *)&sp->emode);
			}
		}
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.enableERR2StdOUT(Boolean isStdout);
static KMETHOD Subproc_enableERR2StdOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	int ret = PREEXEC(sp);
	if(ret) {
		if(sfp[1].boolValue == 1) {
			setFd(kctx, (pfd_t *)&sp->emode, M_STDOUT, NULL);
		} else {
			if(sp->emode == M_STDOUT) {
				initFd((pfd_t *)&sp->emode);
			}
		}
	}
	KReturnUnboxValue(ret);
}

//## boolean Subproc.isShellmode();
static KMETHOD Subproc_isShellmode(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	//	KReturnUnboxValue(sp->shell == 1);
	KReturnUnboxValue(SUBPROC_IsShell(sp));
}

//## boolean Subproc.isPipemodeIN();
static KMETHOD Subproc_isPipemodeIN(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(sp->wmode == M_PIPE);
}

//## boolean Subproc.isPipemodeOUT();
static KMETHOD Subproc_isPipemodeOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(sp->rmode == M_PIPE);
}

//## boolean Subproc.isPipemodeERR();
static KMETHOD Subproc_isPipemodeERR(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(sp->emode == M_PIPE);
}

//## boolean Subproc.isStandardIN();
static KMETHOD Subproc_isStandardIN(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(sp->wmode == M_NREDIRECT);
}

//## boolean Subproc.isStandardOUT();
static KMETHOD Subproc_isStandardOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(sp->rmode == M_NREDIRECT);
}

//## boolean Subproc.isStandardERR();
static KMETHOD Subproc_isStandardERR(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(sp->emode == M_NREDIRECT);
}

//## boolean Subproc.isERR2StdOUT();
static KMETHOD Subproc_isERR2StdOUT(KonohaContext *kctx, KonohaStack *sfp)
{
	kSubproc *sp = (kSubproc *)sfp[0].asObject;
	KReturnUnboxValue(sp->emode == M_STDOUT);
}

//## @Static int Subproc.call(String args);
//static KMETHOD Subproc_call(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kSubproc *sp = /*G*/new_(Subproc, NULL);
//	subprocData_t *p = sp->spd;
//	int ret = S_PREEXECUTION;
//	KFieldSet(sp, p->command, sfp[1].asString);
//	if( (ret = proc_start(kctx, p)) == S_TIMEOUT ) {
//		p->timeoutKill = 1;
//		killWait(p->cpid);
////		KNH_NTHROW2(kctx, sfp, "Script!!", "subproc.fg :: timeout", K_FAILED, KNH_LDATA0);
//	}
//	int status = p->status;
//	if(WIFEXITED(status)) {
//		ret = WEXITSTATUS(status);
//	} else if(WIFSIGNALED(status)) {
//		ret = WTERMSIG(status) * -1;
//	} else if(WIFSTOPPED(status)) {
//		ret = WSTOPSIG(status) * -1;
//	}
//	KReturnUnboxValue( ret );
//}

//## @Static String Subproc.checkOutput(String args);
//static KMETHOD Subproc_CheckOutput(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KReturn(kSubproc_CheckOutput(kctx, /*G*/new_(Subproc, NULL), sfp[1].asString));
//}

/* ------------------------------------------------------------------------ */

static void kSubproc_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	//	struct kSubprocVar *proc = (struct kSubprocVar *)o;
	struct kSubprocVar *p = (struct kSubprocVar *)o;
	/*	if(conf != NULL) {
		proc->spd = (subprocData_t *)conf;
	} else {
	*/
	//	subprocData_t *p = (subprocData_t *)KCalloc_UNTRACE(sizeof(subprocData_t), 1);
	p->command     = KNULL(String);
	p->cwd         = KNULL(String);
	p->env         = KNULL(Array);
	p->cpid        = -1;
	//	p->closefds    = 0;
	//	p->bg          = 0;
	//	p->shell       = 0;
	p->flag        = 0;
	p->timeout     = DEF_TIMEOUT;
	p->bufferSize  = 0;
	p->timeoutKill = 0;
	initFd((pfd_t *)&p->rmode);
	initFd((pfd_t *)&p->wmode);
	initFd((pfd_t *)&p->emode);
	INIT_RESOURCE_MONITOR(p);
}

static void kSubproc_Free(KonohaContext *kctx, kObject *o)
{
	/*	struct kSubprocVar *proc = (struct _kSubproc *)o;
		if(proc->spd != NULL) {
		KFree(proc->spd, sizeof(subprocData_t));
		proc->spd = NULL;
	}
	*/
}

static void kSubproc_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	struct kSubprocVar *proc = (struct kSubprocVar *)o;
	KRefTrace(proc->env);
	KRefTrace(proc->command);
	KRefTrace(proc->cwd);
}

/* ------------------------------------------------------------------------ */

#define _Public   kMethod_Public
//#define _Static   kMethod_Static
#define _Im kMethod_Immutable
#define _F(F)   (intptr_t)(F)

static kbool_t subproc_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequireKonohaCommonModule(trace);
	KRequirePackage("Type.File", trace);
	subproc_InitSubProc(kctx, ns, trace);

	// old subproc ..
	KDEFINE_CLASS defSubproc = {
		STRUCTNAME(Subproc),
		.cflag    = KClassFlag_Final,
		.init     = kSubproc_Init,
		.free     = kSubproc_Free,
		.reftrace = kSubproc_Reftrace,
	};

	KClass *cSubproc = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defSubproc, trace);

	kparamtype_t ps = {KType_String, KFieldName_("str")};
	KClass *KClass_StringArray2 = KLIB KClass_Generics(kctx, KClass_Array, KType_String, 1, &ps);
	ktypeattr_t KType_StringArray = KClass_StringArray2->typeId;

	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Subproc_new), KType_Subproc, KType_Subproc,KMethodName_("new"), 2, KType_String, KFieldName_("path"), KType_Boolean, KFieldName_("mode"),
		_Public|_Im, _F(Subproc_fg), KType_Int, KType_Subproc, KMethodName_("fg"), 0,
		_Public|_Im, _F(Subproc_bg), KType_Boolean, KType_Subproc, KMethodName_("bg"), 0,
		_Public|_Im, _F(Subproc_exec), KType_String, KType_Subproc, KMethodName_("exec"), 1, KType_String, KFieldName_("data"),
		_Public|_Im, _F(Subproc_communicate), KType_StringArray, KType_Subproc, KMethodName_("communicate"), 1, KType_String, KFieldName_("input"),
//		_Public|_Im, _F(Subproc_poll), KType_Int, KType_Subproc, KMethodName_("poll"), 0,
//		_Public|_Im, _F(Subproc_wait), KType_Int, KType_Subproc, KMethodName_("wait"), 0,
//		_Public|_Im, _F(Subproc_sendSignal), KType_Boolean, KType_Subproc, KMethodName_("sendSignal"), 1 KType_Int, KFieldName_("signal"),
//		_Public|_Im, _F(Subproc_terminate), KType_Boolean, KType_Subproc, KMethodName_("terminate"), 0,
		_Public|_Im, _F(Subproc_getPid), KType_Int, KType_Subproc, KMethodName_("getPid"), 0,
		_Public|_Im, _F(Subproc_enableShellmode), KType_Boolean, KType_Subproc, KMethodName_("enableShellmode"), 1, KType_Boolean, KFieldName_("isShellmode"),
		_Public|_Im, _F(Subproc_enablePipemodeIN), KType_Boolean, KType_Subproc, KMethodName_("enablePipemodeIN"), 1, KType_Boolean, KFieldName_("isPipemode"),
		_Public|_Im, _F(Subproc_enablePipemodeOUT), KType_Boolean, KType_Subproc, KMethodName_("enablePipemodeOUT"), 1, KType_Boolean, KFieldName_("isPipemode"),
		_Public|_Im, _F(Subproc_enablePipemodeERR), KType_Boolean, KType_Subproc, KMethodName_("enablePipemodeERR"), 1, KType_Boolean, KFieldName_("isPipemode"),
		_Public|_Im, _F(Subproc_enableStandardIN), KType_Boolean, KType_Subproc, KMethodName_("enableStandardIN"), 1, KType_Boolean, KFieldName_("isStandard"),
		_Public|_Im, _F(Subproc_enableStandardOUT), KType_Boolean, KType_Subproc, KMethodName_("enableStandardOUT"), 1, KType_Boolean, KFieldName_("isStandard"),
		_Public|_Im, _F(Subproc_enableStandardERR), KType_Boolean, KType_Subproc, KMethodName_("enableStandardERR"), 1, KType_Boolean, KFieldName_("isStandard"),
		_Public|_Im, _F(Subproc_isShellmode), KType_Boolean, KType_Subproc, KMethodName_("isShellmode"), 0,
		_Public|_Im, _F(Subproc_isPipemodeIN), KType_Boolean, KType_Subproc, KMethodName_("isPipemodeIN"), 0,
		_Public|_Im, _F(Subproc_isPipemodeOUT), KType_Boolean, KType_Subproc, KMethodName_("isPipemodeOUT"), 0,
		_Public|_Im, _F(Subproc_isPipemodeERR), KType_Boolean, KType_Subproc, KMethodName_("isPipemodeERR"), 0,
		_Public|_Im, _F(Subproc_isStandardIN), KType_Boolean, KType_Subproc, KMethodName_("isStandardIN"), 0,
		_Public|_Im, _F(Subproc_isStandardOUT), KType_Boolean, KType_Subproc, KMethodName_("isStandardOUT"), 0,
		_Public|_Im, _F(Subproc_isStandardERR), KType_Boolean, KType_Subproc, KMethodName_("isStandardERR"), 0,
		_Public|_Im, _F(Subproc_isERR2StdOUT), KType_Boolean, KType_Subproc, KMethodName_("isERR2StdOUT"), 0,
//		_Public|_Static|_Im, _F(Subproc_call), KType_Int, KType_Subproc, KMethodName_("call"), 1, KType_String, KFieldName_("args"),
//		_Public|_Static|_Im, _F(Subproc_CheckOutput), KType_String, KType_Subproc, KMethodName_("checkOutput"), 1, KType_String, KFieldName_("data"),
		_Public|_Im, _F(Subproc_SetCwd), KType_Boolean, KType_Subproc, KMethodName_("setCwd"), 1, KType_String, KFieldName_("cwd"),
		_Public|_Im, _F(Subproc_SetBufsize), KType_Boolean, KType_Subproc, KMethodName_("setBufsize"), 1, KType_Int, KFieldName_("bufsize"),
		_Public|_Im, _F(Subproc_SetFileIN), KType_Boolean, KType_Subproc, KMethodName_("setFileIN"), 1, KType_FILE, KFieldName_("file"),
		_Public|_Im, _F(Subproc_SetFileOUT), KType_Boolean, KType_Subproc, KMethodName_("setFileOUT"), 1, KType_FILE, KFieldName_("file"),
		_Public|_Im, _F(Subproc_SetFileERR), KType_Boolean, KType_Subproc, KMethodName_("setFileERR"), 1, KType_FILE, KFieldName_("file"),
		_Public|_Im, _F(Subproc_getTimeout), KType_Int, KType_Subproc, KMethodName_("getTimeout"), 0,
		_Public|_Im, _F(Subproc_getReturncode), KType_Int, KType_Subproc, KMethodName_("getReturncode"), 0,
		_Public|_Im, _F(Subproc_enableERR2StdOUT), KType_Boolean, KType_Subproc, KMethodName_("enableERR2StdOUT"), 1, KType_Boolean, KFieldName_("isStdout"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

static kbool_t subproc_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE *subproc_Init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("subproc", "1.0"),
		.PackupNameSpace    = subproc_PackupNameSpace,
		.ExportNameSpace   = subproc_ExportNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
