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
#include <pthread.h>
#include <event.h>
#include <evhttp.h>
#include <konoha3/konoha.h>

#define EVENTAPI PLATAPI EventModule.
// -------------------------------------------------------------------------
/* EventModule, EventContext */

struct JsonBuf {
	int64_t json_i;  // it consumes 64bits (based on Ide's impl)
};

struct EventContext {
	volatile int      *safePointRef;
	struct LocalQueue *queue;
	volatile int       isWaiting;  // see WaitEvent
	KonohaContext     *httpContext;
	pthread_mutex_t    lock;
	pthread_cond_t     cond;
	struct event_base *base;
	struct evhttp     *httpd;
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
/* Http */

static KonohaContext* httpContext = NULL;
//static KonohaContext* signalContext = NULL;

static void EnterHttpContext(KonohaContext *kctx)
{
	if(httpContext == NULL) {
		httpContext = kctx;
	}
}

static void ExitHttpContext(KonohaContext *kctx)
{
	httpContext = NULL;
}

// ---------------------------------------------------------------------------

static void http_handler(struct evhttp_request *req, void *args)
{
	struct EventContext *eventContext = (struct EventContext *)args;
	KonohaContext *kctx = eventContext->httpContext;
	struct evbuffer *body = evhttp_request_get_input_buffer(req);
	size_t len = evbuffer_get_length(body);
	unsigned char *requestLine;
	struct evbuffer *buf = evbuffer_new();

	switch(req->type) {
		case EVHTTP_REQ_POST:
			requestLine = evbuffer_pullup(body, -1);
			requestLine[len] = '\0';
			struct JsonBuf jsonbuf = {};
			if(PLATAPI JsonModule.ParseJson(kctx, &jsonbuf, (char *)requestLine, strlen((char *)requestLine), NULL)) {
				pthread_mutex_lock(&eventContext->lock);
				if(enqueueRawEventToLocalQueue(eventContext->queue, (RawEvent *)&jsonbuf) == true) {
					pthread_mutex_unlock(&eventContext->lock);
					evhttp_send_reply(req, HTTP_OK, "OK", buf);
				}
				*(eventContext->safePointRef) |= SafePoint_Event;
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

static void *HttpEventListener(void *args)
{
	struct EventContext *eventContext = (struct EventContext *)args;
	event_base_dispatch(eventContext->base);
	return NULL;
}

typedef struct {
	const char *host;
	int port;
} Ip;

static void StartEventHandler(KonohaContext *kctx, void *args)
{
	KNH_ASSERT(EVENTAPI eventContext == NULL);
	struct EventContext *eventContext = (struct EventContext *)PLATAPI malloc_i(sizeof(struct EventContext));
	bzero(eventContext, sizeof(struct EventContext));
	((KonohaFactory *)kctx->platApi)->EventModule.eventContext = eventContext;

	eventContext->safePointRef = (int *)&kctx->safepoint;

	eventContext->queue = (LocalQueue *)PLATAPI malloc_i(sizeof(LocalQueue));
	LocalQueue_Init(kctx, eventContext->queue);

	eventContext->httpContext = httpContext;

	pthread_mutex_init(&eventContext->lock, NULL);
	pthread_cond_init(&eventContext->cond, NULL);

	KNH_ASSERT(args != NULL);
	Ip *ip = (Ip *)args;
	struct event_base *base = event_base_new();
	struct evhttp *httpd = evhttp_new(base);
	KNH_ASSERT(evhttp_bind_socket(httpd, ip->host, ip->port) >= 0);
	evhttp_set_gencb(httpd, http_handler, (void *)eventContext);
	eventContext->base = base;
	eventContext->httpd = httpd;

	pthread_t t;
	pthread_create(&t, NULL, HttpEventListener, (void *)eventContext);
}

static void StopEventHandler(KonohaContext *kctx, void *args)
{
	KonohaFactory *factory = (KonohaFactory *)kctx->platApi;
	struct EventContext *eventContext = factory->EventModule.eventContext;
	if(eventContext != NULL) {
		eventContext->isWaiting = false;
		pthread_cond_signal(&eventContext->cond);
		pthread_mutex_destroy(&eventContext->lock);
		pthread_cond_destroy(&eventContext->cond);
		LocalQueue_Free(kctx, eventContext->queue);
		evhttp_free(eventContext->httpd);
		event_base_free(eventContext->base);
		PLATAPI free_i(eventContext);
		factory->EventModule.eventContext = NULL;
	}
}

static void EnterEventContext(KonohaContext *kctx, void *args)
{
	EnterHttpContext(kctx);
}

static void ExitEventContext(KonohaContext *kctx, void *args)
{
	ExitHttpContext(kctx);
}

static kbool_t EmitEvent(KonohaContext *kctx, struct JsonBuf *json, KTraceInfo *trace)
{
	struct EventContext *eventContext = EVENTAPI eventContext;
	kbool_t ret = false;
	if(eventContext != NULL) {
		pthread_mutex_lock(&eventContext->lock);
		ret = enqueueRawEventToLocalQueue(eventContext->queue, (RawEvent *)json);
		pthread_mutex_unlock(&eventContext->lock);
		if(ret && eventContext->isWaiting) {
			pthread_cond_signal(&eventContext->cond); // see WaitEvent();
		}
	}
	return ret;
}

static void DispatchEvent(KonohaContext *kctx, kbool_t (*consume)(KonohaContext *kctx, struct JsonBuf *, KTraceInfo *), KTraceInfo *trace)
{
	struct EventContext *eventContext = EVENTAPI eventContext;
	pthread_mutex_lock(&eventContext->lock);
	RawEvent *rawEvent = dequeueRawEventFromLocalQueue(eventContext->queue);
	pthread_mutex_unlock(&eventContext->lock);
	while(rawEvent != NULL) {
		consume(kctx, (struct JsonBuf *)rawEvent, trace);
		pthread_mutex_lock(&eventContext->lock);
		rawEvent = dequeueRawEventFromLocalQueue(eventContext->queue);
		pthread_mutex_unlock(&eventContext->lock);
	}
}

static void WaitEvent(KonohaContext *kctx, kbool_t (*consume)(KonohaContext *kctx, struct JsonBuf *, KTraceInfo *), KTraceInfo *trace)
{
	while(EVENTAPI eventContext != NULL) { // check this out
		int safePoint = *(EVENTAPI eventContext->safePointRef);
		*(EVENTAPI eventContext->safePointRef) ^= SafePoint_Event;  // FIXME
		if((safePoint & SafePoint_Event) == SafePoint_Event) {
			DispatchEvent(kctx, consume, trace);
		}
		EVENTAPI eventContext->isWaiting = true;
		pthread_cond_wait(&(EVENTAPI eventContext->cond), &(EVENTAPI eventContext->lock));
		if(EVENTAPI eventContext->isWaiting == false) break;
	}
}

// -------------------------------------------------------------------------

kbool_t LoadHttpModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"Http", "0.1", 0, "http",
	};
	factory->EventModule.EventInfo         = &ModuleInfo;
	factory->EventModule.eventContext      = NULL;
	factory->EventModule.StartEventHandler = StartEventHandler;
	factory->EventModule.StopEventHandler  = StopEventHandler;
	factory->EventModule.EnterEventContext = EnterEventContext;
	factory->EventModule.ExitEventContext  = ExitEventContext;
	factory->EventModule.EmitEvent         = EmitEvent;
	factory->EventModule.DispatchEvent     = DispatchEvent;
	factory->EventModule.WaitEvent         = WaitEvent;
	return true;
}


#ifdef __cplusplus
} /* extern "C" */
#endif
