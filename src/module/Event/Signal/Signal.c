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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <signal.h>
#include <konoha3/konoha.h>
#define EVENTAPI PLATAPI EventModule.
// -------------------------------------------------------------------------
/* EventModule, EventContext */

struct JsonBuf {
	int64_t josn_i;  // it consumes 64bits (based on Ide's impl)
};

struct EventContext {
	volatile int      *safePointRef;
	struct LocalQueue *queue;
	int                isWaiting;  // see WaitEvent
	struct sigaction   sigdata[32];
	int                caughtSignal;
	siginfo_t         *siginfo[32];
};

/* LocalQueue */

typedef struct JsonBuf   RawEvent;
typedef struct LocalQueue LocalQueue;

struct LocalQueue {
	int front;
	int last;
	RawEvent   **list;
};

// LocalQueue
#define QUEUESIZE 64
#define next(index) (((index) + 1) % QUEUESIZE)
//#define HttpEventQueue   kmodevent->localQueues[HTTP_EVENT]
//#define SignalEventQueue kmodevent->localQueues[SIGNAL_EVENT]

static void LocalQueue_Init(KonohaContext *kctx, struct LocalQueue *queue)
{
	queue->front = 0;
	queue->last  = 0;
	queue->list  = (RawEvent**)PLATAPI malloc_i(sizeof(RawEvent) * QUEUESIZE);
}

static void LocalQueue_Free(KonohaContext *kctx, LocalQueue *queue)
{
	PLATAPI free_i(queue->list);
	PLATAPI free_i(queue);
}

// function of enqueueing raw event to local queue
static kbool_t enqueueRawEventToLocalQueue(LocalQueue *queue, RawEvent *rawEvent)
{
	int *front = &queue->front;
	int *last  = &queue->last;
	if(next(*front) == *last) return false;
	queue->list[*front] = rawEvent;
	*front = next(*front);
	return true;
}

// function of dequeueing raw event from local queue
static RawEvent* dequeueRawEventFromLocalQueue(LocalQueue *queue)
{
	int *front = &queue->front;
	int *last  = &queue->last;
	if(*front == *last) return NULL;
	RawEvent *rawEvent = queue->list[*last];
	*last = next(*last);
	return rawEvent;
}

//void knh_CheckSafePoint(CTX ctx, ksfp_t *sfp, const char *file, int line)
//{
//	int safepoint = ctx->safepoint;
//	WCTX(ctx)->safepoint = 0;
//	if(KFlag_Is(int, safepoint, SAFEPOINT_GC)) {
//		if(line != 0) {
//			GC_LOG("%s:%d safepoint=%d", file, line, safepoint);
//		}
//		invoke_gc(ctx);
//	}
//	if(KFlag_Is(int, safepoint, SAFEPOINT_SIGNAL)) {
//		if(ctx->sighandlers != NULL) {
//			KNH_ASSERT(ctx->signal < K_SIGNAL_MAX);
//			kFunc *handler_func = (kFunc *)ctx->sighandlers[ctx->signal];
//			if(handler_func != NULL) {
//				ksfp_t *lsfp = ctx->esp + 1; // for safety
//				lsfp[K_CALLDELTA + 1].ivalue = ctx->signal;
//				knh_Func_invoke(ctx, handler_func, lsfp, 1/* argc */);
//			}
//		}
//		WCTX(ctx)->signal = 0;
//	}
//	if(KFlag_Is(int, safepoint, SAFEPOINT_MONITOR)) {
//		//
//	}
//}

// -------------------------------------------------------------------------
/* Signal */

static KonohaContext* signalContext = NULL;

static void EnterSignalContext(KonohaContext *kctx)
{
	if(signalContext == NULL) {
		signalContext = kctx;
	}
}

static void ExitSignalContext(KonohaContext *kctx)
{
	signalContext = NULL;
}

static void signal_handler(int signum, siginfo_t *siginfo, void *context)
{
	KonohaContext* kctx = signalContext;
	if(kctx != NULL) {
		struct EventContext *eventContext = EVENTAPI eventContext;
		eventContext->caughtSignal = 1;
		if(eventContext->siginfo[signum] != NULL) {
			// overriding unconsumed signal
		}
		eventContext->siginfo[signum] = siginfo;
		*(eventContext->safePointRef) |= SafePoint_Event;
	}
}

static void SetSigAction(KonohaContext *kctx, int signo, int flags, struct sigaction *act, struct sigaction *oact)
{
	act->sa_sigaction = signal_handler;
	act->sa_flags     = SA_SIGINFO | flags;
	if(sigaction(signo, act, oact) == -1) {

	}
}

static void ResetSigAction(KonohaContext *kctx, int signo, struct sigaction *act, struct sigaction *oact)
{
	if(sigaction(signo, act, oact) == -1) {

	}
}

#define KSetSignal(signum, flags) \
	bzero(&act, sizeof(struct sigaction));\
	SetSigAction(kctx, signum, flags, &act, &(eventContext->sigdata[signum-1]));\

#define KResetSignal(signum) \
	ResetSigAction(kctx, signum, &(eventContext->sigdata[signum-1]), &act);\

static void SetSignal(KonohaContext *kctx)
{
	struct EventContext *eventContext = EVENTAPI eventContext;
	struct sigaction act = {};

	KSetSignal(SIGHUP, 0);
//				KSetSignal(SIGINT, SA_NODEFER);
	KSetSignal(SIGQUIT, 0);
	KSetSignal(SIGILL, 0);
	KSetSignal(SIGTRAP, 0);
	KSetSignal(SIGABRT, 0);
//				KSetSignal(SIGEMT, 0)
	KSetSignal(SIGFPE, SA_NODEFER);
//				KSetSignal(SIGKILL, 0)
	KSetSignal(SIGBUS, 0);
	KSetSignal(SIGSEGV, 0);
	KSetSignal(SIGSYS, 0);
	KSetSignal(SIGPIPE, 0);
	KSetSignal(SIGALRM, 0);
//				KSetSignal(SIGTERM, 0)
	KSetSignal(SIGURG, 0);
	KSetSignal(SIGSTOP, 0);
	KSetSignal(SIGTSTP, 0);
	KSetSignal(SIGCONT, 0);
	KSetSignal(SIGCHLD, 0);
	KSetSignal(SIGTTIN, 0);
	KSetSignal(SIGTTOU, 0);
	KSetSignal(SIGIO, 0);
	KSetSignal(SIGXCPU, 0);
	KSetSignal(SIGXFSZ, 0);
	KSetSignal(SIGVTALRM, 0);
	KSetSignal(SIGPROF, 0);
	KSetSignal(SIGWINCH, 0);
//				KSetSignal(SIGINFO, 0)
//				CASE_USER_DEFINED_EVENT
	KSetSignal(SIGUSR1, 0);
	KSetSignal(SIGUSR2, 0);
}

static void ResetSignal(KonohaContext *kctx)
{
	struct EventContext *eventContext = EVENTAPI eventContext;
	struct sigaction act = {};

	KResetSignal(SIGHUP);
//				KResetSignal(SIGINT);
	KResetSignal(SIGQUIT);
	KResetSignal(SIGILL);
	KResetSignal(SIGTRAP);
	KResetSignal(SIGABRT);
//				KResetSignal(SIGEMT)
	KResetSignal(SIGFPE);
//				KResetSignal(SIGKILL, 0)
	KResetSignal(SIGBUS);
	KResetSignal(SIGSEGV);
	KResetSignal(SIGSYS);
	KResetSignal(SIGPIPE);
	KResetSignal(SIGALRM);
//				KResetSignal(SIGTERM, 0)
	KResetSignal(SIGURG);
	KResetSignal(SIGSTOP);
	KResetSignal(SIGTSTP);
	KResetSignal(SIGCONT);
	KResetSignal(SIGCHLD);
	KResetSignal(SIGTTIN);
	KResetSignal(SIGTTOU);
	KResetSignal(SIGIO);
	KResetSignal(SIGXCPU);
	KResetSignal(SIGXFSZ);
	KResetSignal(SIGVTALRM);
	KResetSignal(SIGPROF);
	KResetSignal(SIGWINCH);
//				KResetSignal(SIGINFO, 0)
//				CASE_USER_DEFINED_EVENT
	KResetSignal(SIGUSR1);
	KResetSignal(SIGUSR2);
}

static void AddSignalEvent(KonohaContext *kctx, struct EventContext *eventContext, KTraceInfo *trace)
{
	size_t i;
	for(i = 0; i < 32; i++) {
		siginfo_t *siginfo = eventContext->siginfo[i];
		eventContext->siginfo[i] = NULL;
		if(siginfo != NULL) {
			char buf[BUFSIZ];
			struct JsonBuf jsonbuf = {};
			snprintf(buf, sizeof(buf), "{\"event\": \"signal\", \"signal\": %d}", (int)(i+1));
			if(PLATAPI JsonModule.ParseJson(kctx, &jsonbuf, buf, strlen(buf), trace)) {
				enqueueRawEventToLocalQueue(eventContext->queue, (RawEvent *)&jsonbuf);
			}
		}
	}
}

// ---------------------------------------------------------------------------

static void StartEventHandler(KonohaContext *kctx, void *args)
{
	KNH_ASSERT(EVENTAPI eventContext == NULL);
	struct EventContext *eventContext = (struct EventContext *)PLATAPI malloc_i(sizeof(struct EventContext));
	bzero(eventContext, sizeof(struct EventContext));
	((KonohaFactory *)kctx->platApi)->EventModule.eventContext = eventContext;
	eventContext->queue = (LocalQueue *)PLATAPI malloc_i(sizeof(LocalQueue));
	LocalQueue_Init(kctx, eventContext->queue);
	SetSignal(kctx);
}

static void StopEventHandler(KonohaContext *kctx, void *args)
{
	ResetSignal(kctx);
	struct EventContext *eventContext = ((KonohaFactory *)kctx->platApi)->EventModule.eventContext;
	LocalQueue_Free(kctx, eventContext->queue);
	PLATAPI free_i(eventContext);
}

static void EnterEventContext(KonohaContext *kctx, void *args)
{
	EnterSignalContext(kctx);
}

static void ExitEventContext(KonohaContext *kctx, void *args)
{
	ExitSignalContext(kctx);
}

static kbool_t EmitEvent(KonohaContext *kctx, struct JsonBuf *json, KTraceInfo *trace)
{
	struct EventContext *eventContext = EVENTAPI eventContext;
	kbool_t ret = false;
	if(eventContext != NULL) {
		ret = enqueueRawEventToLocalQueue(eventContext->queue, (RawEvent *)json);
		if(ret && eventContext->isWaiting) {
			raise(SIGCONT); // see WaitEvent();
		}
	}
	return ret;
}

static void DispatchEvent(KonohaContext *kctx, kbool_t (*consume)(KonohaContext *kctx, struct JsonBuf *, KTraceInfo *), KTraceInfo *trace)
{
	struct EventContext *eventContext = EVENTAPI eventContext;
	if(eventContext->caughtSignal != 0) {
		eventContext->caughtSignal = 0;
		AddSignalEvent(kctx, eventContext, trace);
	}
	RawEvent *rawEvent = dequeueRawEventFromLocalQueue(eventContext->queue);
	while(rawEvent != NULL) {
		consume(kctx, (struct JsonBuf *)rawEvent, trace);
		rawEvent = dequeueRawEventFromLocalQueue(eventContext->queue);
	}
}

static void WaitEvent(KonohaContext *kctx, kbool_t (*consume)(KonohaContext *kctx, struct JsonBuf *, KTraceInfo *), KTraceInfo *trace)
{
	int signo;
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss, SIGCONT);
	sigaddset(&ss, SIGINT);
//	sigprocmask(SIG_BLOCK, &ss, NULL); // check this out

	while(EVENTAPI eventContext != NULL) {
		int safePoint = *(EVENTAPI eventContext->safePointRef);
		*(EVENTAPI eventContext->safePointRef) ^= SafePoint_Event;  // FIXME
		if((safePoint & SafePoint_Event) == SafePoint_Event) {
			DispatchEvent(kctx, consume, trace);
		}
		EVENTAPI eventContext->isWaiting = true;
		sigwait(&ss, &signo);
		if(signo == SIGINT) break;
	}
	if(EVENTAPI eventContext != NULL) {
		EVENTAPI eventContext->isWaiting = false;
	}
}


// -------------------------------------------------------------------------

kbool_t LoadSignalModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"Signal", "0.1", 0, "signal",
	};
	factory->EventModule.EventInfo            = &ModuleInfo;
	factory->EventModule.eventContext = NULL;
	factory->EventModule.StartEventHandler = StartEventHandler;
	factory->EventModule.StopEventHandler  = StopEventHandler;
	factory->EventModule.EnterEventContext = EnterEventContext;
	factory->EventModule.ExitEventContext  = ExitEventContext;
	factory->EventModule.EmitEvent         = EmitEvent;
	factory->EventModule.DispatchEvent     = DispatchEvent;
	factory->EventModule.WaitEvent         = WaitEvent;
	return true;
}

//// http://www.ibm.com/developerworks/jp/linux/library/l-sigdebug/index.html
//#if defined(K_USING_MINGW_)
//#define RECDATA
//#define RECARG
//#else
//#define RECDATA , si, sc
//#define RECARG , siginfo_t* si, void *sc
//#endif
//
//static void record_signal(CTX ctx, int sn RECARG)
//{
//#if defined(K_USING_MINGW_)
//	fprintf(stderr, "signal number = %d", sn);
//#else
//	fprintf(stderr, "signal number = %d, signal errno = %d, signal code = %d", si->si_signo,si->si_errno, si->si_code);
//	fprintf(stderr, "senders' pid = %x, sender's uid = %d\n", si->si_pid, si->si_uid);
//#endif /* defined(K_USING_MINGW_) */
//}
//
//static void trapSIGINT(int sig RECARG)
//{
//	CTX ctx = knh_getCurrentContext();
////	record_signal(ctx, sig RECDATA);
//	if(ctx != NULL) {
////#if defined(K_USING_MINGW_)
////		knh_ldata_t ldata[] = {LOG_END};
////#else
////		knh_ldata_t ldata[] = {LOG_i("sender_pid", si->si_pid), LOG_i("sender_uid", si->si_uid), LOG_END};
////#endif /* defined(K_USING_MINGW_) */
////		KNH_NTRACE(ctx, "konoha:signal", K_NOTICE, ldata);
//		KNH_NTRACE2(ctx, "konoha:signal", K_NOTICE,
//#if defined(K_USING_MINGW_)
//				KNH_LDATA0
//#else
//				KNH_LDATA(LOG_i("sender_pid", si->si_pid), LOG_i("sender_uid", si->si_uid))
//#endif
//		);
//	}
//	_Exit(0);
//}
//
//static void trapSIGFPE(int sig RECARG)
//{
//	static const char *emsg[] = {
//			/* FPE_NOOP	  0*/ "SIGFPE",
//			/* FPE_FLTDIV 1*/ "floating point divide by zero",
//			/* FPE_FLTOVF 2*/ "floating point overflow",
//			/* FPE_FLTUND 3*/ "floating point underflow",
//			/* FPE_FLTRES 4*/ "floating point inexact result",
//			/* FPE_FLTINV	5	*/ "invalid floating point operation",
//			/* FPE_FLTSUB	6	*/ "subscript out of range",
//			/* FPE_INTDIV	7	*/ "integer divide by zero",
//			/* FPE_INTOVF	8	*/ "integer overflow"};
//	CTX ctx = knh_getCurrentContext();
//	record_signal(ctx, sig RECDATA);
//	if(ctx != NULL) {
//#if defined(K_USING_MINGW_)
//		int si_code = 0;
//#else
//		int si_code = (si->si_code < 9) ? si->si_code : 0;
//#endif /* defined(K_USING_MINGW_) */
//		THROW_Arithmetic(ctx, NULL, emsg[si_code]);
//	}
//}
//
//#ifndef K_USING_DEBUG
//static void trapSEGV(int sig RECARG)
//{
//	CTX ctx = knh_getCurrentContext();
//	record_signal(ctx, sig RECDATA);
//#if !defined(K_USING_MINGW_)
//	if(si->si_code == SEGV_ACCERR) {
//		void* address = (void *)si->si_Addr;
//		fprintf(stderr, "address=%p\n", address);
//	}
//#endif /* defined(K_USING_MINGW_) */
//	if(ctx != NULL) {
//		WCTX(ctx)->signal = sig;
//		THROW_Halt(ctx, NULL, "segmentation fault");
//	}
//	_Exit(EX_SOFTWARE);
//}
//
//static void trapILL(int sig RECARG)
//{
//	static const char *emsg[] = {
//			/* FPE_NOOP	  0*/ "SIGILL",
//			/* ILL_ILLOPC 1*/ "illegal opcode",
//			/* ILL_ILLTRP 2*/ "illegal trap",
//			/* ILL_PRVOPC 3*/ "privileged opcode",
//			/* ILL_ILLOPN 4*/ "illegal operand",
//			/* 	5	*/ "illegal addressing mode",
//			/* 	6	*/ "privileged register",
//			/* 	7	*/ "coprocessor error",
//			/* 	8	*/ "internal stack error"};
//	CTX ctx = knh_getCurrentContext();
//	record_signal(ctx, sig RECDATA);
//	if(ctx != NULL) {
//#if defined(K_USING_MINGW_)
//		int si_code = 0;
//#else
//		int si_code = (si->si_code < 9) ? si->si_code : 0;
//#endif /* defined(K_USING_MINGW_) */
//		WCTX(ctx)->signal = sig;
//		THROW_Halt(ctx, NULL, emsg[si_code]);
//	}
//	_Exit(EX_SOFTWARE);
//}
//
//#if !defined(K_USING_MINGW_)
//static void trapBUS(int sig RECARG)
//{
//	static const char *emsg[] = {
//			/* BUS_NOOP	  0*/ "BUS_NOOP",
//			/* BUS_ADRALN 1*/ "invalid address alignment",
//			/* BUS_ADRERR 2*/ "nonexistent physical address",
//			/* BUS_OBJERR 3*/ "object-specific HW error"};
//	CTX ctx = knh_getCurrentContext();
//	record_signal(ctx, sig RECDATA);
//	if(ctx != NULL) {
//		int si_code = (si->si_code < 4) ? si->si_code : 1;
//		WCTX(ctx)->signal = sig;
//		THROW_Halt(ctx, NULL, emsg[si_code]);
//	}
//	_Exit(EX_SOFTWARE);
//}
//#endif /* !defined(K_USING_MINGW_) */
//
//#endif
//
#if 0
#define KNH_SIGACTION(T, sa, sa_orig, n) do {                \
	if(T < n  && sigaction(T, sa, sa_orig + T) != 0 ) {        \
		KNH_NTRACE2(ctx, "sigaction", K_PERROR, \
				KNH_LDATA(LOG_i("signal", T)));        \
	}                                                          \
	knh_bzero(sa, sizeof(struct sigaction));                   \
} while(0)
#endif

//#endif /* defined(K_USING_MINGW_) */

#ifdef __cplusplus
} /* extern "C" */
#endif
