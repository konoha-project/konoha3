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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>

#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/file.h>
#include <event.h>
#include <evhttp.h>
#include <jansson.h>

#ifdef __cplusplus
extern "C"{
#endif

#define KonohaContext_getEventContext(kctx)    ((EventContext *)kctx->modlocal[MOD_EVENT])
#define kmodevent ((KModuleEvent *)kctx->modshare[MOD_EVENT])
#define FLAG_EVENT (1 << 0)
#define CT_Event (kmodevent->cEvent)

typedef json_t*  RawEvent;
typedef RawEvent* RawEventList;

typedef struct {
	int front;
	int last;
	RawEventList list;
} LocalQueue;

#define HTTP_EVENT      0
#define SIGNAL_EVENT    1
#define EVENT_TYPE_MAX 2

typedef struct {
	KonohaModule h;
	int flag;
	KonohaClass *cEvent;
	LocalQueue *localQueues[EVENT_TYPE_MAX];
} KModuleEvent;

typedef struct {
	KonohaModuleContext h;
	kFunc *invokeFuncNULL;
	kFunc *enqFuncNULL;
} EventContext;

/* ------------------------------------------------------------------------ */
// RawEvent
static RawEvent createRawEvent(KonohaContext *kctx, unsigned char *str)
{
	json_error_t error;
	return (RawEvent)json_loads((const char *)str, 0, &error);
}

// LocalQueue
#define QUEUESIZE 64
#define next(index) (((index) + 1) % QUEUESIZE)
#define HttpEventQueue   kmodevent->localQueues[HTTP_EVENT]
#define SignalEventQueue kmodevent->localQueues[SIGNAL_EVENT]

static void LocalQueue_init(KonohaContext *kctx, LocalQueue *queue)
{
	queue->front = 0;
	queue->last  = 0;
	queue->list  = (RawEventList)PLATAPI malloc_i(sizeof(RawEvent) * QUEUESIZE);
}

static void LocalQueue_free(KonohaContext *kctx, LocalQueue *queue)
{
	PLATAPI free_i(queue->list);
	PLATAPI free_i(queue);
}

// function of enqueueing raw event to local queue
static kbool_t enqueueRawEventToLocalQueue(LocalQueue *queue, RawEvent rawEvent)
{
	int *front = &queue->front;
	int *last  = &queue->last;
	if(next(*front) == *last) return false;
	queue->list[*front] = rawEvent;
	*front = next(*front);
	return true;
}

// function of dequeueing raw event from local queue
static RawEvent dequeueRawEventFromLocalQueue(LocalQueue *queue)
{
	int *front = &queue->front;
	int *last  = &queue->last;
	if(*front == *last) return NULL;
	RawEvent rawEvent = queue->list[*last];
	*last = next(*last);
	return rawEvent;
}

/* ------------------------------------------------------------------------ */
// Event class
typedef struct {
	KonohaObjectHeader h;
	json_t *j;
} kEvent;

static void Event_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kEvent *event = (kEvent *)o;
	event->j = NULL;
}

static void Event_free(KonohaContext *kctx, kObject *o)
{
	kEvent *event = (kEvent *)o;
	if(event->j != NULL) {
		json_decref(event->j);
		event->j = NULL;
	}
}

static void Event_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
}

#define CHECK_JSON(obj, ret_stmt) do {\
		if (!json_is_object(obj)) {\
			DBG_P("[ERROR]: Object is not Json object.");\
			/*KLIB KonohaRuntime_raise(kctx, 1, sfp, pline, msg);*/\
			ret_stmt;\
		}\
	} while(0);

//## String Event.getProperty(String key);
static KMETHOD Event_getProperty(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((kEvent*)sfp[0].asObject)->j;
	CHECK_JSON(obj, KReturn(KNULL(String)));
	const char *key = S_text(sfp[1].asString);
	json_t* ret = json_object_get(obj, key);
	if (!json_is_string(ret)) {
		KReturn(KNULL(String));
	}
	ret = json_incref(ret);
	const char* str = json_string_value(ret);
	if (str == NULL) {
		KReturn(KNULL(String));
	}
	KReturn(KLIB new_kString(kctx, OnStack, str, strlen(str), 0));
}

/* ------------------------------------------------------------------------ */
// HttpEventListener class
static void httpEventHandler(struct evhttp_request *req, void *args)
{
	KonohaContext *kctx = (KonohaContext *)args;
	struct evbuffer *body = evhttp_request_get_input_buffer(req);
	size_t len = evbuffer_get_length(body);
	unsigned char *requestLine;
	RawEvent rawEvent;
	struct evbuffer *buf = evbuffer_new();

	switch(req->type) {
		case EVHTTP_REQ_POST:
			requestLine = evbuffer_pullup(body, -1);
			requestLine[len] = '\0';
			rawEvent = createRawEvent(kctx, requestLine);
			if(enqueueRawEventToLocalQueue(HttpEventQueue, rawEvent) == true) {
				evhttp_send_reply(req, HTTP_OK, "OK", buf);
			}
			else {
				evhttp_send_error(req, HTTP_SERVUNAVAIL, "Event queue is full");
			}
			break;
		default:
			evhttp_send_error(req, HTTP_BADREQUEST, "Available POST only");
			break;
	}
	evbuffer_free(buf);
}

typedef struct {
	KonohaContext *kctx;
	const char *host;
	int port;
} targs_t;

static void *httpEventListener(void *args)
{
	targs_t *targs = (targs_t *)args;
	KonohaContext *kctx = targs->kctx;
	const char *host = targs->host;
	int port = targs->port;
	struct event_base *base = event_base_new();
	struct evhttp *httpd = evhttp_new(base);
	if(evhttp_bind_socket(httpd, host, port) < 0) {
		pthread_exit(NULL);
	}
	evhttp_set_gencb(httpd, httpEventHandler, (void *)kctx);
	event_base_dispatch(base);
	evhttp_free(httpd);
	event_base_free(base);
	return NULL;
}

//## void HttpEventListener.start(String host, int port);
static KMETHOD HttpEventListener_start(KonohaContext *kctx, KonohaStack *sfp)
{
	HttpEventQueue = (LocalQueue *)PLATAPI malloc_i(sizeof(LocalQueue));
	LocalQueue_init(kctx, HttpEventQueue);

	const char *host = S_text(sfp[1].asString);
	int port = sfp[2].intValue;
	pthread_t t;
	static targs_t targs = {};
	targs.kctx = kctx;
	targs.host = host;
	targs.port = port;
	pthread_create(&t, NULL, httpEventListener, (void *)&targs);
}

/* ------------------------------------------------------------------------ */
// SignalEventListener class

#define BUFSIZE 4096

static void initMailbox(char *path)
{
	FILE *mailbox;
	if(!(mailbox = fopen(path, "w"))) {
		fprintf(stderr, "cannot open mailbox file!\n");
		pthread_exit(NULL);
	}
	fclose(mailbox);
}

static void signalEventHandler(KonohaContext *kctx)
{
	FILE *mailbox;
	char mailboxPath[] = "/usr/local/minikonoha/dse/mailbox";
	char buf[BUFSIZE];
	RawEvent rawEvent;

	if(!(mailbox = fopen(mailboxPath, "r"))) {
		fprintf(stderr, "cannot open mailbox file!\n");
		pthread_exit(NULL);
	}

	flock(fileno(mailbox), LOCK_EX);
	while(true) {
		if(fgets(buf, BUFSIZE, mailbox) == NULL) break;
		rawEvent = createRawEvent(kctx, (unsigned char *)buf);
		if(enqueueRawEventToLocalQueue(SignalEventQueue, rawEvent) == false) {
			fprintf(stderr, "Event queue is full");
		}
	}
	initMailbox(mailboxPath);
	flock(fileno(mailbox), LOCK_UN);
	fclose(mailbox);
}

static void *signalEventListener(void *args)
{
	KonohaContext *kctx = (KonohaContext *)args;
	int signo;
	sigset_t ss;
	sigemptyset(&ss);
	if(sigaddset(&ss, SIGUSR1)) {
		fprintf(stderr, "sigaddset error!\n");
		pthread_exit(NULL);
	}
	if(sigprocmask(SIG_BLOCK, &ss, NULL)) {
		fprintf(stderr, "sigprocmask error!\n");
		pthread_exit(NULL);
	}

	while(1) {
		if(sigwait(&ss, &signo) == 0) {
			switch(signo) {
				case SIGUSR1:
					signalEventHandler(kctx);
					break;
				default:
					break;
			}
		}
	}
}

static KMETHOD SignalEventListener_start(KonohaContext *kctx, KonohaStack *sfp)
{
	SignalEventQueue = (LocalQueue *)PLATAPI malloc_i(sizeof(LocalQueue));
	LocalQueue_init(kctx, SignalEventQueue);

	FILE *pid;
	pthread_t t;
	if(!(pid = fopen("/usr/local/minikonoha/dse/pid", "w"))) {
		fprintf(stderr, "cannot open pid file\n");
		exit(1);
	}
	fprintf(pid, "%d", getpid());
	fflush(pid);
	fclose(pid);
	pthread_create(&t, NULL, signalEventListener, (void *)kctx);
}

/* ------------------------------------------------------------------------ */
//## void System.setSafepoint();
static KMETHOD System_setSafepoint(KonohaContext *kctx, KonohaStack *sfp)
{
	kmodevent->flag |= FLAG_EVENT;
	((KonohaContextVar *)kctx)->safepoint = 1;
	KReturnVoid();
}

//## void System.setEventInvokeFunc(Func f);
static KMETHOD System_setEventInvokeFunc(KonohaContext *kctx, KonohaStack *sfp)
{
	KonohaContext_getEventContext(kctx)->invokeFuncNULL = sfp[1].asFunc;
	KReturnVoid();
}

//## void System.setEnqFunc(Func f);
static KMETHOD System_setEnqFunc(KonohaContext *kctx, KonohaStack *sfp)
{
	KonohaContext_getEventContext(kctx)->enqFuncNULL = sfp[1].asFunc;
	KReturnVoid();
}

/* ------------------------------------------------------------------------ */
// function of enequeueing global queue
static void enqueueEventToGlobalQueue(KonohaContext *kctx, RawEvent rawEvent)
{
	kEvent *ev = (kEvent *)KLIB new_kObject(kctx, OnStack, CT_Event, 0);
	ev->j = (json_t *)rawEvent;
	ktype_t resolve_type = Method_returnType(KonohaContext_getEventContext(kctx)->enqFuncNULL->mtd);
	BEGIN_LOCAL(lsfp, K_CALLDELTA+1);
	KUnsafeFieldSet(lsfp[K_CALLDELTA+0].asObject, K_NULL);
	KUnsafeFieldSet(lsfp[K_CALLDELTA+1].asObject, (kObject *)ev);
	{
		KonohaStack *sfp = lsfp + K_CALLDELTA;
		KSetMethodCallStack(sfp, 0/*UL*/, KonohaContext_getEventContext(kctx)->enqFuncNULL->mtd, 1, KLIB Knull(kctx, CT_(resolve_type)));
		KonohaRuntime_callMethod(kctx, sfp);
	}
	END_LOCAL();
}

// function of absorbing event from local queues
static void GlobalQueue_absorbRawEventFromLocalQueues(KonohaContext *kctx, LocalQueue **queues)
{
	int i = 0;
	RawEvent rawEvent;
	for(; i < EVENT_TYPE_MAX; i++) {
		if(queues[i] == NULL) continue;
		while(true) {
			rawEvent = dequeueRawEventFromLocalQueue(queues[i]);
			if(rawEvent == NULL) break;
			enqueueEventToGlobalQueue(kctx, rawEvent);
		}
	}
}

/* ------------------------------------------------------------------------ */
// MODEVENT
static void KscheduleEvent(KonohaContext *kctx) {
	// collect events
	GlobalQueue_absorbRawEventFromLocalQueues(kctx, kmodevent->localQueues);

	// dispatch events
	if(kmodevent->flag & FLAG_EVENT) {
		kmodevent->flag ^= FLAG_EVENT;
		BEGIN_LOCAL(lsfp, K_CALLDELTA);
		{
			KonohaStack *sfp = lsfp + K_CALLDELTA;
			KSetMethodCallStack(sfp, 0/*UL*/, KonohaContext_getEventContext(kctx)->invokeFuncNULL->mtd, 0, K_NULL);
			KonohaRuntime_callMethod(kctx, sfp);
		}
		END_LOCAL();
	}
}

static void EventContext_reftrace(KonohaContext *kctx, struct KonohaModuleContext *baseh, KObjectVisitor *visitor)
{
	EventContext *base = (EventContext *)baseh;
//	BEGIN_REFTRACE(2);
	KREFTRACEn(base->invokeFuncNULL);
	KREFTRACEn(base->enqFuncNULL);
//	END_REFTRACE();
}

static void EventContext_free(KonohaContext *kctx, struct KonohaModuleContext *baseh)
{
	EventContext *base = (EventContext *)baseh;
	KFree(base, sizeof(EventContext));
}

static void EventModule_setup(KonohaContext *kctx, struct KonohaModule *def, int newctx)
{
	if(!newctx && kctx->modlocal[MOD_EVENT] == NULL) {
		EventContext *base = (EventContext *)KCalloc_UNTRACE(sizeof(EventContext), 1);
		base->h.reftrace = EventContext_reftrace;
		base->h.free     = EventContext_free;
		KUnsafeFieldInit(base->invokeFuncNULL, K_NULL);
		KUnsafeFieldInit(base->enqFuncNULL, K_NULL);
		kctx->modlocal[MOD_EVENT] = (KonohaModuleContext *)base;
	}
}

static void EventModule_free(KonohaContext *kctx, struct KonohaModule *def)
{
	KModuleEvent *mod = (KModuleEvent *)def;
	int i = 0;
	for(; i < EVENT_TYPE_MAX; i++) {
		if(mod->localQueues[i] != NULL) {
			LocalQueue_free(kctx, mod->localQueues[i]);
		}
	}
	KFree(mod, sizeof(KModuleEvent));
}

static void MODEVENT_init(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KModuleEvent *mod = (KModuleEvent*)KCalloc_UNTRACE(sizeof(KModuleEvent), 1);
	mod->h.name     = "event";
	mod->h.allocSize = sizeof(KModuleEvent);
	mod->h.setupModuleContext    = EventModule_setup;
	mod->h.freeModule = EventModule_free;
	KLIB KonohaRuntime_setModule(kctx, MOD_EVENT, (KonohaModule *)mod, 0);

	KSetKLibFunc(ns->packageId, KscheduleEvent, KscheduleEvent, trace);

	KDEFINE_CLASS defEvent = {
		STRUCTNAME(Event),
		.cflag = kClass_Final,
		.init = Event_init,
		.reftrace = Event_reftrace,
		.free = Event_free,
	};

	KonohaClass *cEvent = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defEvent, trace);
	mod->cEvent = cEvent;
	mod->flag = 0;

	int i = 0;
	for(; i < EVENT_TYPE_MAX; i++) {
		mod->localQueues[i]   = NULL;
	}

	EventModule_setup(kctx, &mod->h, 0);
}

/* ------------------------------------------------------------------------ */

#define _Public   kMethod_Public
#define _Static   kMethod_Static
#define _Const    kMethod_Const
#define _Im       kMethod_Immutable
#define _F(F)   (intptr_t)(F)

#define TY_Event cEvent->typeId
#define TY_HttpEventListener cHttpEventListener->typeId
#define TY_SignalEventListener cSignalEventListener->typeId

static kbool_t eventlistener_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	MODEVENT_init(kctx, ns, trace);

	KDEFINE_CLASS defHttpEventListener = {
		.structname = "HttpEventListener",
		.typeId = TY_newid,
	};
	KDEFINE_CLASS defSignalEventListener = {
		.structname = "SignalEventListener",
		.typeId = TY_newid,
	};
	KonohaClass *cHttpEventListener = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defHttpEventListener, trace);
	KonohaClass *cSignalEventListener = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defSignalEventListener, trace);
	KonohaClass *cEvent = kmodevent->cEvent;

	kparamtype_t P_Func[] = {{TY_Event}};
	int TY_EnqFunc = (KLIB KonohaClass_Generics(kctx, CT_Func, TY_void, 1, P_Func))->typeId;

	KDEFINE_METHOD MethodData[] = {
		/* event gen */
		_Public|_Static, _F(HttpEventListener_start), TY_void, TY_HttpEventListener, MN_("start"), 2, TY_String, FN_("host"), TY_int, FN_("port"),
		_Public|_Static, _F(SignalEventListener_start), TY_void, TY_SignalEventListener, MN_("start"), 0,
		/* event */
		_Public|_Const|_Im, _F(Event_getProperty), TY_String,    TY_Event, MN_("getProperty"), 1, TY_String, FN_("key"),

		/* dispatch */
		_Public|_Static, _F(System_setSafepoint), TY_void, TY_System, MN_("setSafepoint"), 0,
		_Public|_Static, _F(System_setEventInvokeFunc), TY_void, TY_System, MN_("setEventInvokeFunc"), 1, TY_Func, FN_("f"),
		_Public|_Static, _F(System_setEnqFunc), TY_void, TY_System, MN_("setEnqFunc"), 1, TY_EnqFunc, FN_("f"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	return true;
}

static kbool_t eventlistener_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* eventlistener_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("event", "1.0"),
		.initPackage    = eventlistener_initPackage,
		.setupPackage   = eventlistener_setupPackage,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
