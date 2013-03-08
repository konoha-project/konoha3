/****************************************************************************
 * Copyright (c) 2013, the Konoha project authors. All rights reserved.
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

#include <konoha3/konoha.h>
#include <konoha3/konoha_common.h>
#include <konoha3/sugar.h>
#include <konoha3/import/methoddecl.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/http.h>

#ifdef __cplusplus
extern "C" {
#endif


/* ======================================================================== */
#include <sys/time.h>
typedef struct ctimeval {
	kObjectHeader h;
	struct timeval timeval;
} kctimeval;

typedef struct cevent_base {
	kObjectHeader h;
	struct event_base *event_base;
} kcevent_base;

typedef struct cevent {
	kObjectHeader h;
	KonohaContext *kctx;
	struct event *event;
	kFunc *kcb;		// konoha call back method
	kObject *kcbArg;
	kctimeval *kctimeval;
} kcevent;

typedef struct cbufferevent {
	kObjectHeader h;
	KonohaContext *kctx;
	struct bufferevent *bev;
	kFunc *readcb;
	kFunc *writecb;
	kFunc *eventcb;
	kObject *kcbArg;
} kcbufferevent;

typedef struct cevbuffer {
	kObjectHeader h;
	struct evbuffer *buf;
} kcevbuffer;

typedef struct cevhttp {
	kObjectHeader h;
	struct evhttp *evhttp;
	kArray *cbargArray;	//to keep evhttp_set_cb_arg reference
} kcevhttp;

typedef struct cevhttp_bound_socket {
	kObjectHeader h;
	struct evhttp_bound_socket *socket;
} kcevhttp_bound_socket;

typedef struct evhttp_set_cb_arg {
	kObjectHeader h;
	KonohaContext *kctx;
	kFunc *kcb;		// konoha call back method and argment
	kString *uri;	// correspond to kcb. If NULL object, will behave as evhttp_set_gencb()
} kevhttp_set_cb_arg;

typedef struct cevhttp_request {
	kObjectHeader h;
	KonohaContext *kctx;
	struct evhttp_request *req;
	kFunc *kcb;		// konoha call back method and argment
	kFunc *chunked_kcb;		// konoha call back method and argment
} kcevhttp_request;

typedef struct cevconnlistener {
	kObjectHeader h;
	struct evconnlistener *listener;
} kcevconnlistener;

typedef struct cevhttp_connection {
	kObjectHeader h;
	KonohaContext *kctx;
	struct evhttp_connection *evcon;
	kFunc *close_kcb;		// konoha call back method and argment
} kcevhttp_connection;

typedef struct connection_peer {
	kObjectHeader h;
	kString *address;
	int port;
} kconnection_peer;

typedef struct cevkeyvalq {
	kObjectHeader h;
	struct evkeyvalq *keyvalq;
} kcevkeyvalq;

typedef struct cevhttp_uri {
	kObjectHeader h;
	struct evhttp_uri *uri;
} kcevhttp_uri;

typedef struct cevdns_base {
	kObjectHeader h;
	struct evdns_base *base;
} kcevdns_base;

static KClass *KClass_cevent_base;
static KClass *KClass_cbufferevent;
static KClass *KClass_cevhttp_bound_socket;
static KClass *KClass_evhttp_set_cb_arg;
static KClass *KClass_cevbuffer;
static KClass *KClass_cevhttp_request;
static KClass *KClass_cevconnlistener;
static KClass *KClass_cevhttp_connection;
static KClass *KClass_connection_peer;
static KClass *KClass_cevhttp_uri;
static KClass *KClass_cevkeyvalq;

// TODO ----- should be implement in posix.socket package
#include <sys/socket.h>
typedef struct Sockaddr_in {
	kObjectHeader h;
	struct sockaddr_in sockaddr;
} kSockaddr_in;
// TODO should be implement in posix.socket package -----


/* ======================================================================== */
// cevent_base class
static void cevent_base_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kcevent_base *eb = (kcevent_base *) o;
	eb->event_base = NULL;
}

//static void cevent_base_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
//{
//	struct cevent_base *evbase = (struct cevent_base *) o;
//}

static void cevent_base_Free(KonohaContext *kctx, kObject *o)
{
	kcevent_base *eb = (kcevent_base *) o;
	if(eb->event_base != NULL) {
		event_base_free(eb->event_base);
		eb->event_base = NULL;
	}
}

//## cevent_base cevent_base.new();
static KMETHOD cevent_base_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevent_base *eb = (kcevent_base *)sfp[0].asObject;
	eb->event_base = event_base_new();
	KReturn(eb);
}

//## cevent_base cevent_base.event_dispatch();
static KMETHOD cevent_base_event_dispatch(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevent_base *eb = (kcevent_base *)sfp[0].asObject;
	int ret = event_base_dispatch(eb->event_base);
	KReturnUnboxValue(ret);
}

//## int cevent_base.event_loopbreak();
static KMETHOD cevent_base_event_loopbreak(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevent_base *ev = (kcevent_base *)sfp[0].asObject;
	int ret = event_base_loopbreak(ev->event_base);
	KReturnUnboxValue(ret);
}

//## int cevent_base.evutil_make_socket_nonblocking(int fd);
static KMETHOD cevent_base_evutil_make_socket_nonblocking(KonohaContext *kctx, KonohaStack* sfp)
{
	evutil_socket_t evd = (evutil_socket_t)sfp[1].intValue;
	int ret = evutil_make_socket_nonblocking(evd);
	KReturnUnboxValue(ret);
}


/* ======================================================================== */
// cevent class
/*
 * cevent_base Class 1st stage callback from event_base_dispatch(),
 * NEVER BE CALLED FROM OTHERS.
 */
static void cevent_CB_method_invoke(evutil_socket_t evd, short event, void *arg) {
	kcevent *ev = arg;
	KonohaContext *kctx = ev->kctx;

	BEGIN_UnusedStack(lsfp);
	KClass *returnType = kMethod_GetReturnType(ev->kcb->method);
	KStackSetObjectValue(lsfp[0].asObject, K_NULL);
	KStackSetUnboxValue(lsfp[1].intValue, evd);
	KStackSetUnboxValue(lsfp[2].intValue, event);
	KStackSetObjectValue(lsfp[3].asObject, ev->kcbArg);

	KStackSetFuncAll(lsfp, KLIB Knull(kctx, returnType), 0/*UL*/, ev->kcb, 3);
	KStackCall(lsfp);
	END_UnusedStack();
}


static void cevent_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kcevent *ev = (kcevent *) o;
	ev->kctx = NULL;
	ev->event = NULL;
	KFieldInit(ev, ev->kcb, K_NULL);
	KFieldInit(ev, ev->kcbArg, K_NULL);
	KFieldInit(ev, ev->kctimeval, K_NULL);
}

static void cevent_Free(KonohaContext *kctx, kObject *o)
{
	kcevent *ev = (kcevent *) o;

	if(ev->event != NULL) {
		event_free(ev->event);
		ev->event = NULL;
	}
	ev->kctx = NULL;
}

static void cevent_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kcevent *ev = (kcevent *) o;
	KRefTrace(ev->kcb);
	KRefTrace(ev->kcbArg);
	KRefTrace(ev->kctimeval);
}

//## cevent cevent.new(cevent_base event_base, int evd, int event, Func[void, int, int, Object] cb, Object cbArg);
static KMETHOD cevent_event_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevent *ev = (kcevent *) sfp[0].asObject;
	ev->kctx = kctx;
	kcevent_base *cEvent_base = (kcevent_base *)sfp[1].asObject;
	evutil_socket_t evd = (evutil_socket_t)sfp[2].intValue;
	short event = (short)(sfp[3].intValue & 0xffff);
	KFieldSet(ev, ev->kcb, sfp[4].asFunc);
	KFieldSet(ev, ev->kcbArg, sfp[5].asObject);	//deliver to callback method

	ev->event = event_new(cEvent_base->event_base, evd, event, cevent_CB_method_invoke, ev);
	KReturn(ev);
}

//## constructor for signal event
//## cevent cevent.new(cevent_base event_base, int evd, Func[void, int, int, Object] cb, Object cbArg);
static KMETHOD cevent_signal_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevent *ev = (kcevent *) sfp[0].asObject;
	ev->kctx = kctx;
	kcevent_base *cEvent_base = (kcevent_base *)sfp[1].asObject;
	evutil_socket_t evd = (evutil_socket_t)sfp[2].intValue;
	KFieldSet(ev, ev->kcb, sfp[3].asFunc);
	KFieldSet(ev, ev->kcbArg, sfp[4].asObject);	//deliver to callback method

	ev->event = evsignal_new(cEvent_base->event_base, evd, cevent_CB_method_invoke, ev);
	KReturn(ev);
}

//## constructor for timer event
//## cevent cevent.new(cevent_base event_base, Func[void, int, int, Object] cb, Object cbArg);
static KMETHOD cevent_timer_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevent *ev = (kcevent *) sfp[0].asObject;
	ev->kctx = kctx;
	kcevent_base *cEvent_base = (kcevent_base *)sfp[1].asObject;
	KFieldSet(ev, ev->kcb, sfp[2].asFunc);
	KFieldSet(ev, ev->kcbArg, sfp[3].asObject);	//deliver to callback method

	ev->event = evtimer_new(cEvent_base->event_base, cevent_CB_method_invoke, ev);
	KReturn(ev);
}

//## cevent cevent.event_assign(cevent_base event_base, int evd, int event, Func[void, int, int, Object] cb, Object cbArg);
static KMETHOD cevent_event_assign(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevent *ev = (kcevent *) sfp[0].asObject;
	kcevent_base *cEvent_base = (kcevent_base *)sfp[1].asObject;
	evutil_socket_t evd = (evutil_socket_t)sfp[2].intValue;
	short event = (short)(sfp[3].intValue & 0xffff);
	KFieldSet(ev, ev->kcb, sfp[4].asFunc);
	KFieldSet(ev, ev->kcbArg, sfp[5].asObject);	//deliver to callback method

	int ret = event_assign(ev->event, cEvent_base->event_base, evd, event, cevent_CB_method_invoke, ev);
	KReturnUnboxValue(ret);
}

//## cevent cevent.signal_assign(cevent_base event_base, int evd, Func[void, int, int, Object] cb, Object cbArg);
static KMETHOD cevent_signal_assign(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevent *ev = (kcevent *) sfp[0].asObject;
	kcevent_base *cEvent_base = (kcevent_base *)sfp[1].asObject;
	evutil_socket_t evd = (evutil_socket_t)sfp[2].intValue;
	KFieldSet(ev, ev->kcb, sfp[3].asFunc);
	KFieldSet(ev, ev->kcbArg, sfp[4].asObject);	//deliver to callback method

	int ret = evsignal_assign(ev->event, cEvent_base->event_base, evd, cevent_CB_method_invoke, ev);
	KReturnUnboxValue(ret);
}

//## cevent cevent.timer_assign(cevent_base event_base, Func[void, int, int, Object] cb, Object cbArg);
static KMETHOD cevent_timer_assign(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevent *ev = (kcevent *) sfp[0].asObject;
	kcevent_base *cEvent_base = (kcevent_base *)sfp[1].asObject;
	KFieldSet(ev, ev->kcb, sfp[2].asFunc);
	KFieldSet(ev, ev->kcbArg, sfp[3].asObject);	//deliver to callback method

	int ret = evtimer_assign(ev->event, cEvent_base->event_base, cevent_CB_method_invoke, ev);
	KReturnUnboxValue(ret);
}

#define tvIsNull(tv_p)	((tv_p)->timeval.tv_sec == 0 && (tv_p)->timeval.tv_usec == 0)
//## int cevent.event_add(cevent_base event, timeval tv);
static KMETHOD cevent_event_add(KonohaContext *kctx, KonohaStack* sfp)
{
	kcevent *kcev = (kcevent *)sfp[0].asObject;
	kctimeval *tv = (kctimeval *)sfp[1].asObject;
	KStackSetObjectValue(kcev->kctimeval, tv);
	int ret = event_add(kcev->event, tvIsNull(tv) ? NULL : &tv->timeval);
	KReturnUnboxValue(ret);
}

//## int cevent.event_del(cevent event);
static KMETHOD cevent_event_del(KonohaContext *kctx, KonohaStack* sfp)
{
	kcevent *kcev = (kcevent *)sfp[0].asObject;
	KFieldSet(kcev, kcev->kctimeval, (kctimeval *)K_NULL);	//delete reference
	int ret = event_del(kcev->event);
	KReturnUnboxValue(ret);
}

//## int cevent.event_pending(short events, timeval tv);
static KMETHOD cevent_event_pending(KonohaContext *kctx, KonohaStack* sfp)
{
	kcevent *kcev = (kcevent *)sfp[0].asObject;
	short events = (short)sfp[1].intValue;
	kctimeval *tv = (kctimeval *)sfp[2].asObject;
	int ret = event_pending(kcev->event, events, tvIsNull(tv) ? NULL : &tv->timeval);
	KReturnUnboxValue(ret);
}

//## int cevent.signal_pending(timeval tv);
static KMETHOD cevent_signal_pending(KonohaContext *kctx, KonohaStack* sfp)
{
	kcevent *kcev = (kcevent *)sfp[0].asObject;
	kctimeval *tv = (kctimeval *)sfp[1].asObject;
	int ret = evsignal_pending(kcev->event, tvIsNull(tv) ? NULL : &tv->timeval);
	KReturnUnboxValue(ret);
}

//## int cevent.timer_pending(timeval tv);
static KMETHOD cevent_timer_pending(KonohaContext *kctx, KonohaStack* sfp)
{
	kcevent *kcev = (kcevent *)sfp[0].asObject;
	kctimeval *tv = (kctimeval *)sfp[1].asObject;
	int ret = evtimer_pending(kcev->event, tvIsNull(tv) ? NULL : &tv->timeval);
	KReturnUnboxValue(ret);
}

//## int cevent.event_initialized();
static KMETHOD cevent_event_initialized(KonohaContext *kctx, KonohaStack* sfp)
{
	kcevent *kcev = (kcevent *)sfp[0].asObject;
	int ret = event_initialized(kcev->event);
	KReturnUnboxValue(ret);
}

//## void cevent.event_active(int res, int ncalls);
static KMETHOD cevent_event_active(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevent *ev = (kcevent *) sfp[0].asObject;
	int res = sfp[1].intValue;
	short ncalls = (short)sfp[2].intValue;
	event_active(ev->event, res, ncalls);
	KReturnVoid();
}


//## cevent cevent.getEvents();
// get event category field
static KMETHOD cevent_getEvents(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevent *ev = (kcevent *) sfp[0].asObject;
	KReturnUnboxValue(ev->event->ev_events);
}


/* ======================================================================== */
// bufferevent class

static void cbufferevent_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kcbufferevent *bev = (kcbufferevent *) o;
	bev->kctx = NULL;
	bev->bev = NULL;
	KFieldInit(bev, bev->readcb, K_NULL);
	KFieldInit(bev, bev->writecb, K_NULL);
	KFieldInit(bev, bev->eventcb, K_NULL);
	KFieldInit(bev, bev->kcbArg, K_NULL);
}

static void cbufferevent_Free(KonohaContext *kctx, kObject *o)
{
	kcbufferevent *bev = (kcbufferevent *) o;

	bev->kctx = NULL;
	if (bev->bev != NULL) {
		bufferevent_free(bev->bev);
		bev->bev = NULL;
	}
}

static void cbufferevent_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kcbufferevent *bev = (kcbufferevent *) o;
	KRefTrace(bev->readcb);
	KRefTrace(bev->writecb);
	KRefTrace(bev->eventcb);
	KRefTrace(bev->kcbArg);
}

//## bufferevent bufferevent.new(cevent_base event_base, int evd, int option);
static KMETHOD cbufferevent_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kcbufferevent *bev = (kcbufferevent *)sfp[0].asObject;
	bev->kctx = kctx;
	kcevent_base *cev_base = (kcevent_base *)sfp[1].asObject;
	evutil_socket_t evd = (evutil_socket_t)sfp[2].intValue;
	int options = sfp[3].intValue;

	bev->bev = bufferevent_socket_new(cev_base->event_base, evd, options);
	KReturn(bev);
}

static void Cbev_dataCB_dispatcher(kFunc *datacb, struct bufferevent *bev, void *arg)
{
	kcbufferevent *kcbev = arg;
	KonohaContext *kctx = kcbev->kctx;
#if 1	// this compile switch is just to check libevent behavior at coding. If stoped by assert(), please change compile switch to '0'.
	assert(bev == kcbev->bev);
#else
	kcbev->bev = bev;
#endif

	BEGIN_UnusedStack(lsfp);
	KClass *returnType = kMethod_GetReturnType(datacb->method);
	KStackSetObjectValue(lsfp[0].asObject, K_NULL/*(kObject *)kcbev*/);
	KStackSetObjectValue(lsfp[1].asObject, (kObject *)kcbev);
	KStackSetObjectValue(lsfp[2].asObject, kcbev->kcbArg);
	KStackSetFuncAll(lsfp, KLIB Knull(kctx, returnType), 0/*UL*/, datacb, 2);
	KStackCall(lsfp);
	END_UnusedStack();
}

/*
 * bufferevent Class (*buffer_data_cb)() 1st stage callback from event_base_dispatch(),
 * NEVER BE CALLED FROM OTHERS.
 */
static void cbev_readCB_method_invoke(struct bufferevent *bev, void *arg)
{
	kcbufferevent *kcbev = arg;
	Cbev_dataCB_dispatcher(kcbev->readcb, bev, arg);
}

/*
 * bufferevent Class (*buffer_data_cb)() 1st stage callback from event_base_dispatch(),
 * NEVER BE CALLED FROM OTHERS.
 */
static void cbev_writeCB_method_invoke(struct bufferevent *bev, void *arg)
{
	kcbufferevent *kcbev = arg;
	Cbev_dataCB_dispatcher(kcbev->writecb, bev, arg);
}

/*
 * bufferevent Class (*buffer_event_cb)() 1st stage callback from event_base_dispatch(),
 * NEVER BE CALLED FROM OTHERS.
 */
static void cbev_eventCB_method_invoke(struct bufferevent *bev, short what, void *arg)
{
	kcbufferevent *kcbev = arg;
	KonohaContext *kctx = kcbev->kctx;
#if 1	// this compile switch is just to check libevent behavior at coding. If stoped by assert(), please change compile switch to '0'.
	assert(bev == kcbev->bev);
#else
	kcbev->bev = bev;
#endif

	BEGIN_UnusedStack(lsfp);
	KClass *returnType = kMethod_GetReturnType(kcbev->eventcb->method);
	KStackSetObjectValue(lsfp[0].asObject, K_NULL);
	KStackSetObjectValue(lsfp[1].asObject, (kObject *)kcbev);
	KStackSetUnboxValue(lsfp[2].intValue, what);
	KStackSetObjectValue(lsfp[3].asObject, (kObject *)kcbev->kcbArg);
	KStackSetFuncAll(lsfp, KLIB Knull(kctx, returnType), 0/*UL*/, kcbev->eventcb, 3);
	KStackCall(lsfp);
	END_UnusedStack();
}

//## void bufferevent.setcb(
//##	Func[void, bufferevent, Object] readcb,
//##	Func[void, bufferevent, Object] writecb,
//##	Func[void, bufferevent, int, Object] eventcb,
//##	Object cbArg
//## );
static KMETHOD cbufferevent_setcb(KonohaContext *kctx, KonohaStack *sfp)
{
	kcbufferevent *kcbe = (kcbufferevent *)sfp[0].asObject;

	KFieldSet(kcbe, kcbe->readcb, sfp[1].asFunc);
	KFieldSet(kcbe, kcbe->writecb, sfp[2].asFunc);
	KFieldSet(kcbe, kcbe->eventcb, sfp[3].asFunc);
	KFieldSet(kcbe, kcbe->kcbArg, sfp[4].asObject);
	bufferevent_setcb(kcbe->bev, cbev_readCB_method_invoke, cbev_writeCB_method_invoke, cbev_eventCB_method_invoke, kcbe);
	KReturnVoid();
}

//## int bufferevent.socket_connect(Sockaddr_in sa);
static KMETHOD cbufferevent_socket_connect(KonohaContext *kctx, KonohaStack *sfp)
{
	kcbufferevent *bev = (kcbufferevent *)sfp[0].asObject;
	kSockaddr_in *sa = (kSockaddr_in *)sfp[1].asObject;
	int ret = bufferevent_socket_connect(bev->bev, (struct sockaddr *)&sa->sockaddr, sizeof sa->sockaddr);
	KReturnUnboxValue(ret);
}

//## int bufferevent.enable(int event);
static KMETHOD cbufferevent_enable(KonohaContext *kctx, KonohaStack *sfp)
{
	kcbufferevent *bev = (kcbufferevent *)sfp[0].asObject;
	short event = (short)sfp[1].intValue;

	int ret = bufferevent_enable(bev->bev, event);
	KReturnUnboxValue(ret);
}

//## int bufferevent.write(Bytes buf);
static KMETHOD cbufferevent_write(KonohaContext *kctx, KonohaStack *sfp)
{
	kcbufferevent *bev = (kcbufferevent *)sfp[0].asObject;
	kBytes *buf = sfp[1].asBytes;

	int ret = bufferevent_write(bev->bev, buf->byteptr, buf->bytesize);
	KReturnUnboxValue(ret);
}

//## int bufferevent.bufferevent_read(Bytes buf);
static KMETHOD cbufferevent_read(KonohaContext *kctx, KonohaStack *sfp)
{
	kcbufferevent *bev = (kcbufferevent *)sfp[0].asObject;
	kBytes *buf = sfp[1].asBytes;

	int ret = bufferevent_read(bev->bev, buf->buf, buf->bytesize);
	KReturnUnboxValue(ret);
}


/* ======================================================================== */
// evbuffer class
static void cevbuffer_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kcevbuffer *evbuf = (kcevbuffer *) o;
	evbuf->buf = NULL;
}

static void cevbuffer_Free(KonohaContext *kctx, kObject *o)
{
	kcevbuffer *evbuf = (kcevbuffer *) o;

	if (evbuf->buf != NULL) {
		evbuffer_free(evbuf->buf);
		evbuf->buf = NULL;
	}
}

/* ======================================================================== */
// evhttp class

static void cevhttp_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kcevhttp *http = (kcevhttp *) o;
	http->evhttp = NULL;
	KFieldInit(http, http->cbargArray, K_NULL);
}

static void cevhttp_Free(KonohaContext *kctx, kObject *o)
{
	kcevhttp *http = (kcevhttp *) o;
	if (http->evhttp != NULL) {
		evhttp_free(http->evhttp);
		http->evhttp = NULL;
	}
}

static void cevhttp_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kcevhttp *http = (kcevhttp *) o;
	KRefTrace(http->cbargArray);
}

//## evhttp evhttp.new();
static KMETHOD cevhttp_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp *http = (kcevhttp *)sfp[0].asObject;
	kcevent_base *ceb = (kcevent_base *)sfp[1].asObject;
	http->evhttp = evhttp_new(ceb->event_base);
	//TODO confirm to use 'OnField' for new object
	KFieldSet(http, http->cbargArray, new_(Array, 0, OnField));	//refered "KFieldInit(ns, ns->NameSpaceConstList, new_(Array, 0, OnField));"	in src/konoha/import/datatype.h
	KReturn(http);
}

//## int evhttp.bind_socket(String address, int port);
static KMETHOD cevhttp_bind_socket(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp *http = (kcevhttp *)sfp[0].asObject;
	kString *addr = sfp[1].asString;
	int port = sfp[2].intValue;
	int ret = evhttp_bind_socket(http->evhttp, addr->text, port);
	KReturnUnboxValue(ret);
}

//## evhttp_bound_socket evhttp.bind_socket_with_handle(String address, int port);
static KMETHOD cevhttp_bind_socket_with_handle(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp *http = (kcevhttp *)sfp[0].asObject;
	kString *addr = sfp[1].asString;
	int port = sfp[2].intValue;

	kcevhttp_bound_socket *ret = (kcevhttp_bound_socket *)KLIB new_kObject(kctx, OnStack, kObject_class(sfp[-K_CALLDELTA].asObject), 0);
	struct evhttp_bound_socket *socket = evhttp_bind_socket_with_handle(http->evhttp, addr->text, port);
	ret->socket = socket;
	KReturn(ret);
}

//## int evhttp.accept_socket(int fd);
static KMETHOD cevhttp_accept_socket(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp *http = (kcevhttp *)sfp[0].asObject;
	int fd = sfp[1].intValue;
	KReturnUnboxValue(evhttp_accept_socket(http->evhttp, fd));
}

//## evhttp_bound_socket evhttp.accept_socket_with_handle(int fd);
static KMETHOD cevhttp_accept_socket_with_handle(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp *http = (kcevhttp *)sfp[0].asObject;
	int fd = sfp[1].intValue;

	kcevhttp_bound_socket *ret = (kcevhttp_bound_socket *)KLIB new_kObject(kctx, OnStack, kObject_class(sfp[-K_CALLDELTA].asObject), 0);
	struct evhttp_bound_socket *socket = evhttp_accept_socket_with_handle(http->evhttp, fd);
	ret->socket = socket;
	KReturn(ret);
}

//## evhttp_bound_socket evhttp.bind_listener(evconnlistener listener);
static KMETHOD cevhttp_bind_listener(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp *http = (kcevhttp *)sfp[0].asObject;
	kcevconnlistener *listener = (kcevconnlistener *)sfp[1].asObject;

	kcevhttp_bound_socket *ret = (kcevhttp_bound_socket *)KLIB new_kObject(kctx, OnStack, kObject_class(sfp[-K_CALLDELTA].asObject), 0);
	struct evhttp_bound_socket *socket = evhttp_bind_listener(http->evhttp, listener->listener);
	ret->socket = socket;
	KReturn(ret);
}

//## void evhttp.del_accept_socket(evhttp_bound_socket bound_socket);
static KMETHOD cevhttp_del_accept_socket(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp *http = (kcevhttp *)sfp[0].asObject;
	kcevhttp_bound_socket *socket = (kcevhttp_bound_socket *)sfp[1].asObject;
	evhttp_del_accept_socket(http->evhttp, socket->socket);
	KReturnVoid();
}

//## void evhttp.set_max_headers_size(int max_headers_size);
static KMETHOD cevhttp_set_max_headers_size(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp *http = (kcevhttp *)sfp[0].asObject;
	int size = sfp[1].intValue;
	assert(size >= 0);
	evhttp_set_max_headers_size(http->evhttp, (ev_ssize_t)size);
	KReturnVoid();
}

//## void evhttp.set_max_body_size(int max_body_size);
static KMETHOD cevhttp_set_max_body_size(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp *http = (kcevhttp *)sfp[0].asObject;
	int size = sfp[1].intValue;
	assert(size >= 0);
	evhttp_set_max_body_size(http->evhttp, (ev_ssize_t)size);
	KReturnVoid();
}

//## void evhttp.set_allowed_methos(int methods);
static KMETHOD cevhttp_set_allowed_methods(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp *http = (kcevhttp *)sfp[0].asObject;
	ev_uint16_t methods = (ev_uint16_t)sfp[1].intValue;
	evhttp_set_allowed_methods(http->evhttp, methods);
	KReturnVoid();
}

/*
 * cevhttp Class 1st stage callback from event_base_dispatch(),
 * NEVER BE CALLED FROM OTHERS.
 */
static void cevhttp_CB_method_invoke(struct evhttp_request *req, void * arg)
{
	kevhttp_set_cb_arg *cbarg = arg;
	KonohaContext *kctx = cbarg->kctx;

	kcevhttp_request *kreq = (kcevhttp_request *)(new_(cevhttp_request, 0, OnStack));
	kreq->req = req;

	BEGIN_UnusedStack(lsfp);
	KClass *returnType = kMethod_GetReturnType(cbarg->kcb->method);
	KStackSetObjectValue(lsfp[0].asObject, K_NULL);
	KStackSetObjectValue(lsfp[1].asObject, (kObject *)kreq);
	KStackSetObjectValue(lsfp[2].asObject, cbarg->kcb->env);

	KStackSetFuncAll(lsfp, KLIB Knull(kctx, returnType), 0/*UL*/, cbarg->kcb, 2);
	KStackCall(lsfp);
	END_UnusedStack();
}

//
static int cevhttp_set_cb_common(bool isGencb, KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp *http = (kcevhttp *)sfp[0].asObject;
	kString *uri = NULL;
	kFunc *cb = NULL;
	kObject *cbarg = NULL;

	if (isGencb) {
		cb = (kFunc *)sfp[1].asFunc;
		cbarg = sfp[2].asObject;
	} else {
		uri = sfp[1].asString;
		cb = (kFunc *)sfp[2].asFunc;
		cbarg = sfp[3].asObject;
	}

	kevhttp_set_cb_arg *set_cb_cbarg = (kevhttp_set_cb_arg *)(new_(evhttp_set_cb_arg, 0, OnField));
	int ret;

	set_cb_cbarg->kctx = kctx;
	kFuncVar *fo;
	{
		// copy kFunc Object to use 'env' as CBarg	TODO check
		kMethod *cbmtd = cb->method;
		kParam *pa = kMethod_GetParam(cbmtd);
		KClass *ct = KLIB KClass_Generics(kctx, KClass_Func, pa->rtype, pa->psize, (kparamtype_t *)pa->paramtypeItems);
		fo = (kFuncVar *)KLIB new_kObject(kctx, OnGcStack, ct, (uintptr_t)cbmtd);
		KFieldSet(fo, fo->method, cb->method);
		KFieldSet(fo, fo->env, cbarg);
	}
	KFieldSet(set_cb_cbarg, set_cb_cbarg->kcb, fo);
	KFieldSet(set_cb_cbarg, set_cb_cbarg->uri, uri);

	if (isGencb) {
		evhttp_set_gencb(http->evhttp, cevhttp_CB_method_invoke, set_cb_cbarg);
		ret = 0;
	} else {
		if ((ret = evhttp_set_cb(http->evhttp, uri->text, cevhttp_CB_method_invoke, set_cb_cbarg)) == 0) {
			KLIB kArray_Add(kctx, http->cbargArray, fo); //set the reference to kArray in evhttp class
		}
	}
	return ret;
}

//## int evhttp.set_cb(String uri, Func[void, evhttp_request, Object] cb, Object cbarg);
static KMETHOD cevhttp_set_cb(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(cevhttp_set_cb_common(false, kctx, sfp));
}

//## int evhttp.del_cb(String uri);
static KMETHOD cevhttp_del_cb(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp *http = (kcevhttp *)sfp[0].asObject;
	kString *uri = sfp[1].asString;
	int ret = evhttp_del_cb(http->evhttp, uri->text);

	//TODO delete member from cbargArray

	KReturnUnboxValue(ret);
}

//## void evhttp.set_gencb(Func[void, evhttp_request, Object] cb, Object cbarg);
static KMETHOD cevhttp_set_gencb(KonohaContext *kctx, KonohaStack *sfp)
{
	cevhttp_set_cb_common(true, kctx, sfp);
	KReturnVoid();
}

//## int evhttp.add_virtual_host(String pattern, evhttp vhost);
static KMETHOD cevhttp_add_virtual_host(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp *http = (kcevhttp *)sfp[0].asObject;
	kString *pattern = sfp[1].asString;
	kcevhttp *vhost = (kcevhttp *)sfp[2].asObject;
	int ret = evhttp_add_virtual_host(http->evhttp, pattern->text, vhost->evhttp);
	KReturnUnboxValue(ret);
}

//## int evhttp.remove_virtual_host(evhttp vhost);
static KMETHOD cevhttp_remove_virtual_host(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp *http = (kcevhttp *)sfp[0].asObject;
	kcevhttp *vhost = (kcevhttp *)sfp[1].asObject;
	int ret = evhttp_remove_virtual_host(http->evhttp, vhost->evhttp);
	KReturnUnboxValue(ret);
}

//## int evhttp.add_server_alias(String alias);
static KMETHOD cevhttp_add_server_alias(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp *http = (kcevhttp *)sfp[0].asObject;
	kString *alias = sfp[1].asString;
	int ret = evhttp_add_server_alias(http->evhttp, alias->text);
	KReturnUnboxValue(ret);
}

//## int evhttp.remove_server_alias(String alias);
static KMETHOD cevhttp_remove_server_alias(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp *http = (kcevhttp *)sfp[0].asObject;
	kString *alias = sfp[1].asString;
	int ret = evhttp_remove_server_alias(http->evhttp, alias->text);
	KReturnUnboxValue(ret);
}

//## void evhttp.set_timeout(int timeout_in_secs);
static KMETHOD cevhttp_set_timeout(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp *http = (kcevhttp *)sfp[0].asObject;
	int secs = sfp[1].intValue;
	evhttp_set_timeout(http->evhttp, secs);
	KReturnVoid();
}

/* ======================================================================== */
// evhttp_bound_socket class
/*
 * "evhttp_bound_socket" has no "new()" method,
 * because it will be executed by new_() macro
 * in evhttp.bind_socket_with_handle() or others.
 */

static void cevhttp_bound_socket_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kcevhttp_bound_socket *req = (kcevhttp_bound_socket *) o;
	req->socket = NULL;
}

//## evconnlistener evhttp_bound_socket.get_listener();
static KMETHOD cevhttp_bound_socket_get_listener(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_bound_socket *bound = (kcevhttp_bound_socket *)sfp[0].asObject;
	kcevconnlistener *ret = (kcevconnlistener *)KLIB new_kObject(kctx, OnStack, kObject_class(sfp[-K_CALLDELTA].asObject), 0);
	struct evconnlistener *listener = evhttp_bound_socket_get_listener(bound->socket);
	ret->listener = listener;
	KReturn(ret);
}

//## int evhttp_bound_socket.get_fd();
static KMETHOD cevhttp_bound_socket_get_fd(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_bound_socket *bound = (kcevhttp_bound_socket *)sfp[0].asObject;
	evutil_socket_t ret = evhttp_bound_socket_get_fd(bound->socket);
	KReturnUnboxValue(ret);
}

/* ======================================================================== */
// evhttp_request class
static void cevhttp_request_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kcevhttp_request *req = (kcevhttp_request *) o;
	req->kctx = NULL;
	req->req = NULL;
	KFieldInit(req, req->kcb, K_NULL);
	KFieldInit(req, req->chunked_kcb, K_NULL);
}

static void cevhttp_request_Free(KonohaContext *kctx, kObject *o)
{
	kcevhttp_request *req = (kcevhttp_request *) o;
	if (req->req != NULL) {
		evhttp_request_free(req->req);
		req->req = NULL;
	}
}

static void cevhttp_request_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kcevhttp_request *req = (kcevhttp_request *) o;
	KRefTrace(req->kcb);
	KRefTrace(req->chunked_kcb);
}

/*
 * cevhttp_request Class 1st stage callback by libevent framework,
 * NEVER BE CALLED FROM OTHERS.
 */
static void cevhttp_request_CB_method_invoke(struct evhttp_request *req, void * arg)
{
	kcevhttp_request *cbarg = arg;
	KonohaContext *kctx = cbarg->kctx;

#if 1	// this compile switch is just to check libevent behavior at coding. If stoped by assert(), please change compile switch to '0'.
	assert(cbarg->req == req);
#else
	cbarg->req = req;
#endif

	BEGIN_UnusedStack(lsfp);
	KClass *returnType = kMethod_GetReturnType(cbarg->kcb->method);
	KStackSetObjectValue(lsfp[0].asObject, K_NULL);
	KStackSetObjectValue(lsfp[1].asObject, (kObject *)cbarg);
	KStackSetObjectValue(lsfp[2].asObject, (kObject *)cbarg->kcb->env);
	KStackSetFuncAll(lsfp, KLIB Knull(kctx, returnType), 0/*UL*/, cbarg->kcb, 2);
	KStackCall(lsfp);
	END_UnusedStack();
}

//## evhttp evhttp_request.new(Func[void, evhttp_request, Object] cb, Object arg);
static KMETHOD cevhttp_request_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_request *req = (kcevhttp_request *)sfp[0].asObject;
	kFunc *cb = sfp[1].asFunc;
	kObject *arg = sfp[2].asObject;
	req->req = evhttp_request_new(cevhttp_request_CB_method_invoke, req);
	KFieldSet(req, req->kcb, cb);
	KFieldSet(req->kcb, req->kcb->env, arg);
	KReturn(req);
}

//## void evhttp_request.send_error(int error, String reason);
static KMETHOD cevhttp_request_send_error(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_request *req = (kcevhttp_request *)sfp[0].asObject;
	int error = sfp[1].intValue;
	kString *reason = sfp[2].asString;
	evhttp_send_error(req->req, error, reason->text);
	KReturnVoid();
}

//## void evhttp_request.send_reply(int code, String reason, evbuffer databuf);
static KMETHOD cevhttp_request_send_reply(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_request *req = (kcevhttp_request *)sfp[0].asObject;
	int code = sfp[1].intValue;
	kString *reason = sfp[2].asString;
	kcevbuffer *databuf = (kcevbuffer *)sfp[3].asObject;
	evhttp_send_reply(req->req, code, reason->text, databuf->buf);
	KReturnVoid();
}

//## void evhttp_request.send_reply_start(int code, String reason);
static KMETHOD cevhttp_request_send_reply_start(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_request *req = (kcevhttp_request *)sfp[0].asObject;
	int code = sfp[1].intValue;
	kString *reason = sfp[2].asString;
	evhttp_send_reply_start(req->req, code, reason->text);
	KReturnVoid();
}

//## void evhttp_request.reply_chunk(evbuffer databuf);
static KMETHOD cevhttp_request_reply_chunk(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_request *req = (kcevhttp_request *)sfp[0].asObject;
	kcevbuffer *databuf = (kcevbuffer *)sfp[1].asObject;
	evhttp_send_reply_chunk(req->req, databuf->buf);
	KReturnVoid();
}

//## void evhttp_request.reply_end();
static KMETHOD cevhttp_request_reply_end(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_request *req = (kcevhttp_request *)sfp[0].asObject;
	evhttp_send_reply_end(req->req);
	KReturnVoid();
}

/*
 * cevhttp_request Class 1st stage callback by libevent framework,
 * NEVER BE CALLED FROM OTHERS.
 */
static void cevhttp_request_chunked_CB_method_invoke(struct evhttp_request *req, void * arg)
{
	kcevhttp_request *cbarg = arg;
	KonohaContext *kctx = cbarg->kctx;

#if 1	// this compile switch is just to check libevent behavior at coding. If stoped by assert(), please change compile switch to '0'.
	assert(cbarg->req == req);
#else
	arg->req = req;
#endif

	BEGIN_UnusedStack(lsfp);
	KClass *returnType = kMethod_GetReturnType(cbarg->chunked_kcb->method);
	KStackSetObjectValue(lsfp[0].asObject, K_NULL);
	KStackSetObjectValue(lsfp[1].asObject, (kObject *)cbarg);
	KStackSetObjectValue(lsfp[2].asObject, (kObject *)cbarg->chunked_kcb->env);
	KStackSetFuncAll(lsfp, KLIB Knull(kctx, returnType), 0/*UL*/, cbarg->chunked_kcb, 2);
	KStackCall(lsfp);
	END_UnusedStack();
}

//## void evhttp_request.set_chunked_cb(Func[void, evhttp_request, Object] cb, Object arg);
static KMETHOD cevhttp_request_set_chunked_cb(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_request *req = (kcevhttp_request *)sfp[0].asObject;
	kFunc *cb = sfp[1].asFunc;
	kObject *arg = sfp[2].asObject;
	evhttp_request_set_chunked_cb(req->req, cevhttp_request_chunked_CB_method_invoke);
	KFieldSet(req, req->chunked_kcb, cb);
	KFieldSet(req->chunked_kcb, req->chunked_kcb->env, arg);
	KReturnVoid();
}

//## void evhttp_request.own();
static KMETHOD cevhttp_request_own(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_request *req = (kcevhttp_request *)sfp[0].asObject;
	evhttp_request_own(req->req);
	KReturnVoid();
}

//## int evhttp_request.is_owned();
static KMETHOD cevhttp_request_is_owned(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_request *req = (kcevhttp_request *)sfp[0].asObject;
	int ret = evhttp_request_is_owned(req->req);
	KReturnUnboxValue(ret);
}

//## evhttp_connection evhttp_request.get_connection();
static KMETHOD cevhttp_request_get_connection(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_request *req = (kcevhttp_request *)sfp[0].asObject;
	kcevhttp_connection *ret = (kcevhttp_connection *)KLIB new_kObject(kctx, OnStack, kObject_class(sfp[-K_CALLDELTA].asObject), 0);
	struct evhttp_connection *evcon = evhttp_request_get_connection(req->req);
	ret->evcon = evcon;
	KReturn(ret);
}

//## void evhttp_request.cancel();
static KMETHOD cevhttp_request_cancel(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_request *req = (kcevhttp_request *)sfp[0].asObject;
	evhttp_cancel_request(req->req);
	KReturnVoid();
}

//## String evhttp_request.get_uri();
static KMETHOD cevhttp_request_get_uri(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_request *req = (kcevhttp_request *)sfp[0].asObject;
	const char *uri = evhttp_request_get_uri(req->req);
	KReturn(KLIB new_kString(kctx, OnStack, uri, strlen(uri), StringPolicy_UTF8));
}

//## evhttp_uri evhttp_request.get_evhttp_uri();
static KMETHOD cevhttp_request_get_evhttp_uri(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_request *req = (kcevhttp_request *)sfp[0].asObject;
	kcevhttp_uri *ret = (kcevhttp_uri *)KLIB new_kObject(kctx, OnStack, kObject_class(sfp[-K_CALLDELTA].asObject), 0);
	ret->uri = (struct evhttp_uri *)evhttp_request_get_evhttp_uri(req->req);
	KReturn(ret);
}

//## int evhttp_request.get_command();
static KMETHOD cevhttp_request_get_command(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_request *req = (kcevhttp_request *)sfp[0].asObject;
	int ret = evhttp_request_get_command(req->req);
	KReturnUnboxValue(ret);
}

//## int evhttp_request.get_response_code();
static KMETHOD cevhttp_request_get_response_code(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_request *req = (kcevhttp_request *)sfp[0].asObject;
	int ret = evhttp_request_get_response_code(req->req);
	KReturnUnboxValue(ret);
}

//## evkeyvalq evhttp_request.get_input_headers();
static KMETHOD cevhttp_request_get_input_headers(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_request *req = (kcevhttp_request *)sfp[0].asObject;
	kcevkeyvalq *ret = (kcevkeyvalq *)KLIB new_kObject(kctx, OnStack, kObject_class(sfp[-K_CALLDELTA].asObject), 0);
	ret->keyvalq = evhttp_request_get_input_headers(req->req);
	KReturn(ret);
}

//## evkeyvalq evhttp_request.get_output_headers();
static KMETHOD cevhttp_request_get_output_headers(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_request *req = (kcevhttp_request *)sfp[0].asObject;
	kcevkeyvalq *ret = (kcevkeyvalq *)KLIB new_kObject(kctx, OnStack, kObject_class(sfp[-K_CALLDELTA].asObject), 0);
	ret->keyvalq = evhttp_request_get_output_headers(req->req);
	KReturn(ret);
}

//## evbuffer evhttp_request.get_input_buffer();
static KMETHOD cevhttp_request_get_input_buffer(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_request *req = (kcevhttp_request *)sfp[0].asObject;
	kcevbuffer *ret = (kcevbuffer *)KLIB new_kObject(kctx, OnStack, kObject_class(sfp[-K_CALLDELTA].asObject), 0);
	ret->buf = evhttp_request_get_input_buffer(req->req);
	KReturn(ret);
}

//## evbuffer evhttp_request.get_output_buffer();
static KMETHOD cevhttp_request_get_output_buffer(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_request *req = (kcevhttp_request *)sfp[0].asObject;
	kcevbuffer *ret = (kcevbuffer *)KLIB new_kObject(kctx, OnStack, kObject_class(sfp[-K_CALLDELTA].asObject), 0);
	ret->buf = evhttp_request_get_output_buffer(req->req);
	KReturn(ret);
}

//## String evhttp_request.get_host();
static KMETHOD cevhttp_request_get_host(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_request *req = (kcevhttp_request *)sfp[0].asObject;
	const char *host = evhttp_request_get_host(req->req);
	KReturn(KLIB new_kString(kctx, OnStack, host, strlen(host), StringPolicy_UTF8));
}


/* ======================================================================== */
// evhttp_connection class
static void cevhttp_connection_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kcevhttp_connection *con = (struct cevhttp_connection *) o;
	con->kctx = NULL;
	con->evcon = NULL;
	KFieldInit(con, con->close_kcb, K_NULL);
}

static void cevhttp_connection_Free(KonohaContext *kctx, kObject *o)
{
	kcevhttp_connection *con = (kcevhttp_connection *) o;
	if (con->evcon != NULL) {
		evhttp_connection_free(con->evcon);
		con->evcon = NULL;
	}
}

static void cevhttp_connection_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kcevhttp_connection *con = (kcevhttp_connection *) o;
	KRefTrace(con->close_kcb);
}

//## evhttp evhttp_connection.new(event_base base, evdns_base dnsbase, String address, int port);
static KMETHOD cevhttp_connection_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_connection *con = (kcevhttp_connection *)sfp[0].asObject;
	kcevent_base *base = (kcevent_base *)sfp[1].asObject;
	kcevdns_base *dnsbase = (kcevdns_base *)sfp[2].asObject;
	kString *addr = sfp[3].asString;
	int port = sfp[4].intValue;
	assert(port >= 0);
	con->evcon = evhttp_connection_base_new(base->event_base, dnsbase->base, addr->text, (unsigned short)port);
	KReturn(con);
}

//## bufferevent evhttp_connection.get_bufferevent();
static KMETHOD cevhttp_connection_get_bufferevent(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_connection *con = (kcevhttp_connection *)sfp[0].asObject;
	kcbufferevent *ret = (kcbufferevent *)KLIB new_kObject(kctx, OnStack, kObject_class(sfp[-K_CALLDELTA].asObject), 0);
	ret->bev = evhttp_connection_get_bufferevent(con->evcon);
	KReturn(ret);
}

//## event_base evhttp_connection.get_base();
static KMETHOD cevhttp_connection_get_base(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_connection *con = (kcevhttp_connection *)sfp[0].asObject;
	kcevent_base *ret = (kcevent_base *)KLIB new_kObject(kctx, OnStack, kObject_class(sfp[-K_CALLDELTA].asObject), 0);
	ret->event_base = evhttp_connection_get_base(con->evcon);
	KReturn(ret);
}

//## void evhttp_connection.set_max_headers_size(int max_headers_size);
static KMETHOD cevhttp_connection_set_max_headers_size(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_connection *con = (kcevhttp_connection *)sfp[0].asObject;
	ev_ssize_t siz = (ev_ssize_t)sfp[1].intValue;
	evhttp_connection_set_max_headers_size(con->evcon, siz);
	KReturnVoid();
}

//## void evhttp_connection.set_max_body_size(int max_body_size);
static KMETHOD cevhttp_connection_set_max_body_size(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_connection *con = (kcevhttp_connection *)sfp[0].asObject;
	ev_ssize_t siz = (ev_ssize_t)sfp[1].intValue;
	evhttp_connection_set_max_body_size(con->evcon, siz);
	KReturnVoid();
}

//## void evhttp_connection.set_local_address(String address);
static KMETHOD cevhttp_connection_set_local_address(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_connection *con = (kcevhttp_connection *)sfp[0].asObject;
	kString *addr = sfp[1].asString;
	evhttp_connection_set_local_address(con->evcon, addr->text);
	KReturnVoid();
}

//## void evhttp_connection.set_local_port(int port);
static KMETHOD cevhttp_connection_set_local_port(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_connection *con = (kcevhttp_connection *)sfp[0].asObject;
	ev_uint16_t port = (ev_uint16_t)sfp[1].intValue;
	evhttp_connection_set_local_port(con->evcon, port);
	KReturnVoid();
}

//## void evhttp_connection.set_timeout(int timeout_in_secs);
static KMETHOD cevhttp_connection_set_timeout(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_connection *con = (kcevhttp_connection *)sfp[0].asObject;
	int secs = sfp[1].intValue;
	evhttp_connection_set_timeout(con->evcon, secs);
	KReturnVoid();
}

//## void evhttp_connection.set_retries(int retry_max);
static KMETHOD cevhttp_connection_set_retries(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_connection *con = (kcevhttp_connection *)sfp[0].asObject;
	int retry_max = sfp[1].intValue;
	evhttp_connection_set_retries(con->evcon, retry_max);
	KReturnVoid();
}

/*
 * cevhttp_connection Class 1st stage callback by libevent framework,
 * NEVER BE CALLED FROM OTHERS.
 */
static void cevhttp_connection_closeCB_method_invoke(struct evhttp_connection *con, void * arg)
{
	kcevhttp_connection *cbarg = arg;
	KonohaContext *kctx = cbarg->kctx;

#if 1	// this compile switch is just to check libevent behavior at coding. If stoped by assert(), please change compile switch to '0'.
	assert(cbarg->evcon == con);
#else
	cbarg->evcon = con;
#endif

	BEGIN_UnusedStack(lsfp);
	KClass *returnType = kMethod_GetReturnType(cbarg->close_kcb->method);
	KStackSetObjectValue(lsfp[0].asObject, K_NULL);
	KStackSetObjectValue(lsfp[1].asObject, (kObject *)cbarg);
	KStackSetObjectValue(lsfp[2].asObject, (kObject *)cbarg->close_kcb->env);
	KStackSetFuncAll(lsfp, KLIB Knull(kctx, returnType), 0/*UL*/, cbarg->close_kcb, 2);
	KStackCall(lsfp);
	END_UnusedStack();
}

//## void evhttp_connection.set_closecb(Func[void, evhttp_connection, Object] cb, Object arg);
static KMETHOD cevhttp_connection_set_closecb(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_connection *con = (kcevhttp_connection *)sfp[0].asObject;
	kFunc *cb = sfp[1].asFunc;
	kObject *arg = sfp[2].asObject;
	KFieldSet(con->close_kcb, con->close_kcb->env, arg);
	KFieldSet(con, con->close_kcb, cb);
	evhttp_connection_set_closecb(con->evcon, cevhttp_connection_closeCB_method_invoke, con);
	KReturnVoid();
}

//## connection_peer evhttp_connection.get_peer();
static KMETHOD cevhttp_connection_get_peer(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_connection *con = (kcevhttp_connection *)sfp[0].asObject;
	kconnection_peer *ret = (kconnection_peer *)KLIB new_kObject(kctx, OnStack, kObject_class(sfp[-K_CALLDELTA].asObject), 0);
	char *addr;
	ev_uint16_t port;
	evhttp_connection_get_peer(con->evcon, &addr, &port);
	kString *addrStr = KLIB new_kString(kctx, OnStack, addr, strlen(addr), StringPolicy_UTF8);
	KFieldSet(ret, ret->address, addrStr);
	ret->port = port;
	KReturn(ret);
}

//## int evhttp_connection.make_request(evhttp_request req, int evhttp_cmd_type, String uri);
static KMETHOD cevhttp_connection_make_request(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_connection *con = (kcevhttp_connection *)sfp[0].asObject;
	kcevhttp_request *req = (kcevhttp_request *)sfp[1].asObject;
	int type = sfp[2].intValue;
	kString *uri = sfp[3].asString;
	int ret = evhttp_make_request(con->evcon, req->req, type, uri->text);
	KReturnUnboxValue(ret);
}


/* ======================================================================== */
// connection_peer class
static void connection_peer_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kconnection_peer *peer = (kconnection_peer *) o;
	KFieldInit(peer, peer->address, K_NULL);
	peer->port = 0;
}

static void connection_peer_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kconnection_peer *peer = (kconnection_peer *) o;
	KRefTrace(peer->address);
}


/* ======================================================================== */
// evkeyvalq class
static void cevkeyvalq_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kcevkeyvalq *queue = (kcevkeyvalq *) o;
	queue->keyvalq = NULL;
}

//## String evkeyvalq.find_header(String key);
static KMETHOD cevkeyvalq_find_header(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevkeyvalq *headers = (kcevkeyvalq *)sfp[0].asObject;
	kString *key = sfp[1].asString;
	const char *ret = evhttp_find_header(headers->keyvalq, key->text);
	KReturn(KLIB new_kString(kctx, OnStack, ret, strlen(ret), StringPolicy_UTF8));
}

//## int evkeyvalq.remove_header(String key);
static KMETHOD cevkeyvalq_remove_header(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevkeyvalq *headers = (kcevkeyvalq *)sfp[0].asObject;
	kString *key = sfp[1].asString;
	KReturnUnboxValue(evhttp_remove_header(headers->keyvalq, key->text));
}

//## int evkeyvalq.add_header(String key, String value);
static KMETHOD cevkeyvalq_add_header(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevkeyvalq *headers = (kcevkeyvalq *)sfp[0].asObject;
	kString *key = sfp[1].asString;
	kString *value = sfp[2].asString;
	KReturnUnboxValue(evhttp_add_header(headers->keyvalq, key->text, value->text));
}

//## void evkeyvalq.clear_header();;
static KMETHOD cevkeyvalq_clear_header(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevkeyvalq *headers = (kcevkeyvalq *)sfp[0].asObject;
	evhttp_clear_headers(headers->keyvalq);
	KReturnVoid();
}


/* ======================================================================== */
// evhttp_uri class
static void cevhttp_uri_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kcevhttp_uri *uri = (kcevhttp_uri *) o;
	uri->uri = NULL;
}

static void cevhttp_uri_Free(KonohaContext *kctx, kObject *o)
{
	kcevhttp_uri *uri = (kcevhttp_uri *) o;
	if (uri->uri != NULL) {
		evhttp_uri_free(uri->uri);
		uri->uri = NULL;
	}
}

//## evhttp_uri evhttp_uri.new();
static KMETHOD cevhttp_uri_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_uri *uri = (kcevhttp_uri *)sfp[0].asObject;
	uri->uri =  evhttp_uri_new();
	KReturn(uri);
}

//## void evhttp_uri.set_flags(int flags);
static KMETHOD cevhttp_uri_set_flags(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_uri *uri = (kcevhttp_uri *)sfp[0].asObject;
	unsigned int flags = (unsigned int)sfp[1].intValue;
	evhttp_uri_set_flags(uri->uri, flags);
	KReturnVoid();
}

//## String evhttp_uri.get_scheme();
static KMETHOD cevhttp_uri_get_scheme(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_uri *uri = (kcevhttp_uri *)sfp[0].asObject;
	const char *ret = evhttp_uri_get_scheme(uri->uri);
	KReturn(KLIB new_kString(kctx, OnStack, ret, strlen(ret), StringPolicy_ASCII));
}

//## String evhttp_uri.get_userinfo();
static KMETHOD cevhttp_uri_get_userinfo(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_uri *uri = (kcevhttp_uri *)sfp[0].asObject;
	const char *ret = evhttp_uri_get_userinfo(uri->uri);
	KReturn(KLIB new_kString(kctx, OnStack, ret, strlen(ret), StringPolicy_ASCII));
}

//## String evhttp_uri.get_host();
static KMETHOD cevhttp_uri_get_host(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_uri *uri = (kcevhttp_uri *)sfp[0].asObject;
	const char *ret = evhttp_uri_get_host(uri->uri);
	KReturn(KLIB new_kString(kctx, OnStack, ret, strlen(ret), StringPolicy_ASCII));
}

//## String evhttp_uri.get_port();
static KMETHOD cevhttp_uri_get_port(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_uri *uri = (kcevhttp_uri *)sfp[0].asObject;
	KReturnUnboxValue(evhttp_uri_get_port(uri->uri));
}

//## String evhttp_uri.get_path();
static KMETHOD cevhttp_uri_get_path(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_uri *uri = (kcevhttp_uri *)sfp[0].asObject;
	const char *ret = evhttp_uri_get_path(uri->uri);
	KReturn(KLIB new_kString(kctx, OnStack, ret, strlen(ret), StringPolicy_ASCII));
}

//## String evhttp_uri.get_query();
static KMETHOD cevhttp_uri_get_query(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_uri *uri = (kcevhttp_uri *)sfp[0].asObject;
	const char *ret = evhttp_uri_get_query(uri->uri);
	KReturn(KLIB new_kString(kctx, OnStack, ret, strlen(ret), StringPolicy_ASCII));
}

//## String evhttp_uri.get_fragment();
static KMETHOD cevhttp_uri_get_fragment(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_uri *uri = (kcevhttp_uri *)sfp[0].asObject;
	const char *ret = evhttp_uri_get_fragment(uri->uri);
	KReturn(KLIB new_kString(kctx, OnStack, ret, strlen(ret), StringPolicy_ASCII));
}

//## int evhttp_uri.set_scheme(String scheme);
static KMETHOD cevhttp_uri_set_scheme(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_uri *uri = (kcevhttp_uri *)sfp[0].asObject;
	kString *scheme = sfp[1].asString;
	KReturnUnboxValue(evhttp_uri_set_scheme(uri->uri, scheme->text));
}

//## int evhttp_uri.set_userinfo(String userinfo);
static KMETHOD cevhttp_uri_set_userinfo(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_uri *uri = (kcevhttp_uri *)sfp[0].asObject;
	kString *userinfo = sfp[1].asString;
	KReturnUnboxValue(evhttp_uri_set_userinfo(uri->uri, userinfo->text));
}

//## int evhttp_uri.set_host(String host);
static KMETHOD cevhttp_uri_set_host(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_uri *uri = (kcevhttp_uri *)sfp[0].asObject;
	kString *host = sfp[1].asString;
	KReturnUnboxValue(evhttp_uri_set_host(uri->uri, host->text));
}

//## int evhttp_uri.set_port(int port);
static KMETHOD cevhttp_uri_set_port(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_uri *uri = (kcevhttp_uri *)sfp[0].asObject;
	int port = sfp[1].intValue;
	KReturnUnboxValue(evhttp_uri_set_port(uri->uri, port));
}

//## int evhttp_uri.set_path(String path);
static KMETHOD cevhttp_uri_set_path(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_uri *uri = (kcevhttp_uri *)sfp[0].asObject;
	kString *path = sfp[1].asString;
	KReturnUnboxValue(evhttp_uri_set_path(uri->uri, path->text));
}

//## int evhttp_uri.set_query(String query);
static KMETHOD cevhttp_uri_set_query(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_uri *uri = (kcevhttp_uri *)sfp[0].asObject;
	kString *query = sfp[1].asString;
	KReturnUnboxValue(evhttp_uri_set_query(uri->uri, query->text));
}

//## int evhttp_uri.set_fragment(String fragment);
static KMETHOD cevhttp_uri_set_fragment(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_uri *uri = (kcevhttp_uri *)sfp[0].asObject;
	kString *fragment = sfp[1].asString;
	KReturnUnboxValue(evhttp_uri_set_fragment(uri->uri, fragment->text));
}

//## evhttp_uri evhttp_uri.parse_with_flags(String source_uri, int flags);
static KMETHOD cevhttp_uri_parse_with_flags(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_uri *uri = (kcevhttp_uri *)sfp[0].asObject;
	kString *source_uri = sfp[1].asString;
	unsigned int flags = sfp[2].intValue;
	uri->uri = evhttp_uri_parse_with_flags(source_uri->text, flags);
	KReturn(uri);
}

//## evhttp_uri evhttp_uri.parse(String source_uri);
static KMETHOD cevhttp_uri_parse(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_uri *uri = (kcevhttp_uri *)sfp[0].asObject;
	kString *source_uri = sfp[1].asString;
	uri->uri = evhttp_uri_parse(source_uri->text);
	KReturn(uri);
}

//## String evhttp_uri.join(Bytes buf);
static KMETHOD cevhttp_uri_join(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevhttp_uri *uri = (kcevhttp_uri *)sfp[0].asObject;
	kBytes *buf = sfp[1].asBytes;
	char *ret = evhttp_uri_join(uri->uri, buf->buf, buf->bytesize);
	KReturn(KLIB new_kString(kctx, OnStack, ret, strlen(ret), StringPolicy_ASCII));
}

//## @Static String evhttp_uri.encode_uri(String str);
static KMETHOD cevhttp_uri_encode_uri(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *str = sfp[1].asString;
	char *encoded = evhttp_encode_uri(str->text);
	kString *retStr = KLIB new_kString(kctx, OnStack, encoded, strlen(encoded), StringPolicy_ASCII);
	free(encoded);
	KReturn(retStr);
}

//## @Static String evhttp_uri.uriencode(String str, int size, int space_to_plus);
static KMETHOD cevhttp_uri_uriencode(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *str = sfp[1].asString;
	int size = sfp[2].intValue;
	int space_to_plus = sfp[3].intValue;
	char *encoded = evhttp_uriencode(str->text, size, space_to_plus);
	kString *retStr = KLIB new_kString(kctx, OnStack, encoded, strlen(encoded), StringPolicy_ASCII);
	free(encoded);
	KReturn(retStr);
}

//## @Static String evhttp_uri.decode_uri(String uri);
static KMETHOD cevhttp_uri_decode_uri(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *uri = sfp[1].asString;
	char *decoded = evhttp_decode_uri(uri->text);
	kString *retStr = KLIB new_kString(kctx, OnStack, decoded, strlen(decoded), StringPolicy_UTF8);
	free(decoded);
	KReturn(retStr);
}

//## @Static String evhttp_uri.uridecode(String uri, int decode_plus);
static KMETHOD cevhttp_uri_uridecode(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *uri = sfp[1].asString;
	int decode_plus = sfp[2].intValue;
	size_t size_out;
	char *decoded = evhttp_uridecode(uri->text, decode_plus, &size_out);
	kString *retStr = KLIB new_kString(kctx, OnStack, decoded, size_out, StringPolicy_UTF8);
	free(decoded);
	KReturn(retStr);
}

//## @Static int evhttp_uri.parse_query_str(String uri, evkeyvalq headers);
static KMETHOD cevhttp_uri_parse_query_str(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *uri = sfp[1].asString;
	kcevkeyvalq *headers = (kcevkeyvalq *)sfp[2].asObject;
	KReturnUnboxValue(evhttp_parse_query_str(uri->text, headers->keyvalq));
}

//## @Static String evhttp_uri.htmlescape(String html);
static KMETHOD cevhttp_uri_htmlescape(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *html = sfp[1].asString;
	char *escaped = evhttp_htmlescape(html->text);
	kString *retStr = KLIB new_kString(kctx, OnStack, escaped, strlen(escaped), StringPolicy_ASCII);
	free(escaped);
	KReturn(retStr);
}


/* ======================================================================== */
// evconnlistener class
/*
 * "evconnlistener" has no "new()" method,
 * because it will be executed by new_() macro
 * in evhttp_bound_socket.get_listener()
 */
static void cevconnlistener_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kcevconnlistener *evclistener = (kcevconnlistener *) o;
	evclistener->listener = NULL;
}


/* ======================================================================== */
// evhttp_set_cb_arg class
/*
 * "evhttp_set_cb_arg" has no "new()" method,
 * because it will be executed by new_() macro
 */
static void evhttp_set_cb_arg_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kevhttp_set_cb_arg *cbarg = (kevhttp_set_cb_arg *) o;

	cbarg->kctx = NULL;
	KFieldInit(cbarg, cbarg->kcb, K_NULL);
	KFieldInit(cbarg, cbarg->uri, K_NULL);
}

static void evhttp_set_cb_arg_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kevhttp_set_cb_arg *arg = (kevhttp_set_cb_arg *) o;
	KRefTrace(arg->kcb);
	KRefTrace(arg->uri);
}


/* ======================================================================== */
// evdns_base class
static void cevdns_base_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kcevdns_base *dnsbase = (kcevdns_base *) o;
	dnsbase->base = NULL;
}


/* ======================================================================== */
// timeval class

static void ctimeval_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kctimeval *tv = (kctimeval *) o;
	tv->timeval.tv_sec = 0;
	tv->timeval.tv_usec = 0;
}

//## timeval timeval.new(int tv_sec, int tv_usec);
static KMETHOD ctimeval_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kctimeval *tv = (kctimeval *) sfp[0].asObject;
	time_t sec = (time_t)sfp[1].intValue;
	suseconds_t usec = (suseconds_t)sfp[2].intValue;
	tv->timeval.tv_sec = sec;
	tv->timeval.tv_usec = usec;
	KReturn(tv);
}


// TODO should be implement in posix.socket package -----
/* ======================================================================== */
// Sockaddr_in class

static void Sockaddr_in_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kSockaddr_in *sa = (kSockaddr_in *) o;
	memset(&sa->sockaddr, 0, sizeof (struct sockaddr));
}

//## Sockaddr_in Sockaddr_in.new(int family, int addr, int port);
static KMETHOD Sockaddr_in_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kSockaddr_in *sa = (kSockaddr_in *) sfp[0].asObject;
	sa_family_t family	= (sa_family_t)sfp[1].intValue;
	in_addr_t addr		= (in_addr_t)sfp[2].intValue;
	in_port_t port		= (in_port_t)sfp[3].intValue;

	sa->sockaddr.sin_family = family;
	sa->sockaddr.sin_addr.s_addr = htonl(addr);
	sa->sockaddr.sin_port = htons(port);
	KReturn(sa);
}
// TODO should be implement in posix.socket package -----


/* ======================================================================== */

static kbool_t Libevent_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequirePackage("Type.Bytes", trace);
	/* Class Definition */
	/* If you want to create Generic class like Array<T>, see konoha.map package */
	// cevent_base
	KDEFINE_CLASS defcevent_base = {0};
	defcevent_base.structname	= "event_base";
	defcevent_base.typeId		= KTypeAttr_NewId;
	defcevent_base.cstruct_size	= sizeof(kcevent_base);
	defcevent_base.cflag		= KClassFlag_Final;	//must be final in C
	defcevent_base.init			= cevent_base_Init;
	defcevent_base.free			= cevent_base_Free;
	KClass_cevent_base = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defcevent_base, trace);

	// cevent
	KDEFINE_CLASS defcevent = {0};
	defcevent.structname	= "event";
	defcevent.typeId		= KTypeAttr_NewId;
	defcevent.cstruct_size	= sizeof(kcevent);
	defcevent.cflag			= KClassFlag_Final;
	defcevent.init			= cevent_Init;
	defcevent.reftrace		= cevent_Reftrace;
	defcevent.free			= cevent_Free;
	KClass *ceventClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defcevent, trace);

	// cbufferevent
	KDEFINE_CLASS defcbufferevent = {0};
	defcbufferevent.structname	= "bufferevent";
	defcbufferevent.typeId		= KTypeAttr_NewId;
	defcbufferevent.cstruct_size = sizeof(kcbufferevent);
	defcbufferevent.cflag		= KClassFlag_Final;
	defcbufferevent.init		= cbufferevent_Init;
	defcbufferevent.reftrace	= cbufferevent_Reftrace;
	defcbufferevent.free		= cbufferevent_Free;
	KClass_cbufferevent = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defcbufferevent, trace);

	// cevbuffer
	KDEFINE_CLASS defcevbuffer = {0};
	defcevbuffer.structname	= "evbuffer";
	defcevbuffer.typeId		= KTypeAttr_NewId;
	defcevbuffer.cstruct_size = sizeof(kcevbuffer);
	defcevbuffer.cflag     = KClassFlag_Final;
	defcevbuffer.init      = cevbuffer_Init;
	defcevbuffer.free      = cevbuffer_Free;
	KClass_cevbuffer = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defcevbuffer, trace);

	// cevhttp
	KDEFINE_CLASS defcevhttp = {0};
	defcevhttp.structname	= "evhttp";
	defcevhttp.typeId		= KTypeAttr_NewId;
	defcevhttp.cstruct_size = sizeof(kcevhttp);
	defcevhttp.cflag     = KClassFlag_Final;
	defcevhttp.init      = cevhttp_Init;
	defcevhttp.reftrace  = cevhttp_Reftrace;
	defcevhttp.free      = cevhttp_Free;
	KClass *cevhttpClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defcevhttp, trace);

	// evhttp_set_cb_arg
	KDEFINE_CLASS defevhttp_set_cb_arg = {0};
	SETSTRUCTNAME(defevhttp_set_cb_arg, evhttp_set_cb_arg);
	defevhttp_set_cb_arg.cflag     = KClassFlag_Final;
	defevhttp_set_cb_arg.init      = evhttp_set_cb_arg_Init;
	defevhttp_set_cb_arg.reftrace  = evhttp_set_cb_arg_Reftrace;
	KClass_evhttp_set_cb_arg = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defevhttp_set_cb_arg, trace);

	// cevhttp_bound_socket
	KDEFINE_CLASS defcevhttp_bound_socket = {0};
	defcevhttp_bound_socket.structname	= "evhttp_bound_socket";
	defcevhttp_bound_socket.typeId		= KTypeAttr_NewId;
	defcevhttp_bound_socket.cstruct_size= sizeof(kcevhttp_bound_socket);
	defcevhttp_bound_socket.cflag		= KClassFlag_Final;
	defcevhttp_bound_socket.init		= cevhttp_bound_socket_Init;
	KClass_cevhttp_bound_socket = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defcevhttp_bound_socket, trace);

	// cevhttp_request
	KDEFINE_CLASS defcevhttp_request = {0};
	defcevhttp_request.structname	= "evhttp_request";
	defcevhttp_request.typeId		= KTypeAttr_NewId;
	defcevhttp_request.cstruct_size	= sizeof(kcevhttp_request);
	defcevhttp_request.cflag		= KClassFlag_Final;
	defcevhttp_request.init			= cevhttp_request_Init;
	defcevhttp_request.reftrace		= cevhttp_request_Reftrace;
	defcevhttp_request.free			= cevhttp_request_Free;
	KClass_cevhttp_request = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defcevhttp_request, trace);

	// cevconnlistener
	KDEFINE_CLASS defcevconnlistener = {0};
	defcevconnlistener.structname	= "evconnlistener";
	defcevconnlistener.typeId		= KTypeAttr_NewId;
	defcevconnlistener.cstruct_size= sizeof(kcevconnlistener);
	defcevconnlistener.cflag		= KClassFlag_Final;
	defcevconnlistener.init		= cevconnlistener_Init;
	KClass_cevconnlistener = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defcevconnlistener, trace);

	// cevhttp_connection
	KDEFINE_CLASS defcevhttp_connection = {0};
	defcevhttp_connection.structname	= "evhttp_connection";
	defcevhttp_connection.typeId		= KTypeAttr_NewId;
	defcevhttp_connection.cstruct_size	= sizeof(kcevhttp_connection);
	defcevhttp_connection.cflag			= KClassFlag_Final;
	defcevhttp_connection.init			= cevhttp_connection_Init;
	defcevhttp_connection.reftrace		= cevhttp_connection_Reftrace;
	defcevhttp_connection.free			= cevhttp_connection_Free;
	KClass_cevhttp_connection			= KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defcevhttp_connection, trace);

	// connection_peer
	KDEFINE_CLASS defconnection_peer = {0};
	defconnection_peer.structname	= "connection_peer";
	defconnection_peer.typeId		= KTypeAttr_NewId;
	defconnection_peer.cstruct_size	= sizeof(kconnection_peer);
	defconnection_peer.cflag		= KClassFlag_Final;
	defconnection_peer.init			= connection_peer_Init;
	defconnection_peer.reftrace		= connection_peer_Reftrace;
	KClass_connection_peer = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defconnection_peer, trace);

	// cevhttp_uri
	KDEFINE_CLASS defcevhttp_uri = {0};
	defcevhttp_uri.structname	= "evhttp_uri";
	defcevhttp_uri.typeId		= KTypeAttr_NewId;
	defcevhttp_uri.cstruct_size	= sizeof(kcevhttp_uri);
	defcevhttp_uri.cflag			= KClassFlag_Final;
	defcevhttp_uri.init			= cevhttp_uri_Init;
	defcevhttp_uri.free			= cevhttp_uri_Free;
	KClass_cevhttp_uri			= KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defcevhttp_uri, trace);

	// cevkeyvalq
	KDEFINE_CLASS defcevkeyvalq = {0};
	defcevkeyvalq.structname	= "evkeyvalq";
	defcevkeyvalq.typeId		= KTypeAttr_NewId;
	defcevkeyvalq.cstruct_size	= sizeof(kcevkeyvalq);
	defcevkeyvalq.cflag			= KClassFlag_Final;
	defcevkeyvalq.init			= cevkeyvalq_Init;
	KClass_cevkeyvalq			= KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defcevkeyvalq, trace);

	// cevdns_base
	KDEFINE_CLASS defcevdns_base = {0};
	defcevdns_base.structname	= "evdns_base";
	defcevdns_base.typeId		= KTypeAttr_NewId;
	defcevdns_base.cstruct_size	= sizeof(kcevdns_base);
	defcevdns_base.cflag			= KClassFlag_Final;
	defcevdns_base.init			= cevdns_base_Init;
	KClass *cevdns_baseClass	= KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defcevdns_base, trace);

	// ctimeval
	KDEFINE_CLASS defctimeval = {0};
	defctimeval.structname	= "timeval";
	defctimeval.typeId		= KTypeAttr_NewId;
	defctimeval.cstruct_size = sizeof(kctimeval);
	defctimeval.cflag		= KClassFlag_Final;
	defctimeval.init		= ctimeval_Init;
	KClass *ctimevalClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defctimeval, trace);

	// Sockaddr_in
	KDEFINE_CLASS defSockaddr_in = {0};
	defSockaddr_in.structname	= "sockaddr_in";
	defSockaddr_in.typeId		= KTypeAttr_NewId;
	defSockaddr_in.cstruct_size	= sizeof(kSockaddr_in);
	defSockaddr_in.cflag		= KClassFlag_Final;
	defSockaddr_in.init			= Sockaddr_in_Init;
	KClass *Sockaddr_inClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defSockaddr_in, trace);


	/* You can define methods with the following procedures. */
	int KType_cevent_base = KClass_cevent_base->typeId;
	int KType_cevent = ceventClass->typeId;
	int KType_cbufferevent = KClass_cbufferevent->typeId;
	int KType_cevbuffer = KClass_cevbuffer->typeId;
	int KType_cevhttp = cevhttpClass->typeId;
	int KType_cevhttp_bound_socket = KClass_cevhttp_bound_socket->typeId;
//	int KType_evhttp_set_cb_arg = KClass_evhttp_set_cb_arg->typeId;
	int KType_cevhttp_request = KClass_cevhttp_request->typeId;
	int KType_cevconnlistener = KClass_cevconnlistener->typeId; 
	int KType_cevhttp_connection = KClass_cevhttp_connection->typeId;
	int KType_connection_peer = KClass_connection_peer->typeId;
	int KType_cevhttp_uri = KClass_cevhttp_uri->typeId;
	int KType_cevkeyvalq = KClass_cevkeyvalq->typeId;
	int KType_cevdns_base = cevdns_baseClass->typeId;
	int KType_ctimeval = ctimevalClass->typeId;
	int KType_Sockaddr_in = Sockaddr_inClass->typeId;

	/* define Generics parameter for callback method */
	//eventCB_p
	kparamtype_t eventCB_p[] = {{KType_Int, 0}, {KType_Int, 0}, {KType_Object, 0}};
	KClass *ceventCBfunc = KLIB KClass_Generics(kctx, KClass_Func, KType_void, 3, eventCB_p);
	int KType_ceventCBfunc = ceventCBfunc->typeId;
	//bev_dataCB_p
	kparamtype_t bev_dataCB_p[] = {{KType_cbufferevent, 0}, {KType_Object, 0}};
	KClass *Cbev_dataCBfunc = KLIB KClass_Generics(kctx, KClass_Func, KType_void, 2, bev_dataCB_p);
	int KType_Cbev_dataCBfunc = Cbev_dataCBfunc->typeId;
	//bev_eventCB_p
	kparamtype_t bev_eventCB_p[] = {{KType_cbufferevent, 0}, {KType_Int, 0}, {KType_Object, 0}};
	KClass *Cbev_eventCBfunc = KLIB KClass_Generics(kctx, KClass_Func, KType_void, 3, bev_eventCB_p);
	int KType_Cbev_eventCBfunc = Cbev_eventCBfunc->typeId;

	//cevhttp_requestCB_p
	kparamtype_t cevhttp_requestCB_p[] = {{KType_cevhttp_request, 0}, {KType_Object, 0}};
	KClass *cevhttp_requestCBfunc = KLIB KClass_Generics(kctx, KClass_Func, KType_void, 2, cevhttp_requestCB_p);
	int KType_cevhttp_requestCBfunc = cevhttp_requestCBfunc->typeId;

	//cevhttp_connectionCB_p
	kparamtype_t cevhttp_connectionCB_p[] = {{KType_cevhttp_connection, 0}, {KType_Object, 0}};
	KClass *cevhttp_connectionCBfunc = KLIB KClass_Generics(kctx, KClass_Func, KType_void, 2, cevhttp_connectionCB_p);
	int KType_cevhttp_connectionCBfunc = cevhttp_connectionCBfunc->typeId;

	KDEFINE_METHOD MethodData[] = {

		// cevent_base
		_Public|_Static, _F(cevent_base_evutil_make_socket_nonblocking), KType_Int, KType_cevent_base, KMethodName_("evutil_make_socket_nonblocking"), 1, KType_Int, KFieldName_("fd"),
		_Public, _F(cevent_base_new), KType_cevent_base, KType_cevent_base, KMethodName_("new"), 0,
		_Public, _F(cevent_base_event_dispatch), KType_Int, KType_cevent_base, KMethodName_("event_dispatch"), 0,
		_Public, _F(cevent_base_event_loopbreak), KType_Int, KType_cevent_base, KMethodName_("event_loopbreak"), 0,

		// cevent
		_Public, _F(cevent_event_new), KType_cevent, KType_cevent, KMethodName_("new"), 5, KType_cevent_base, KFieldName_("cevent_base"), KType_Int, KFieldName_("evd"), KType_Int, KFieldName_("event"), KType_ceventCBfunc, KFieldName_("konoha_CB"), KType_Object, KFieldName_("CBarg"),
		_Public, _F(cevent_signal_new), KType_cevent, KType_cevent, KMethodName_("new"), 4, KType_cevent_base, KFieldName_("cevent_base"), KType_Int, KFieldName_("signo"), KType_ceventCBfunc, KFieldName_("konoha_CB"), KType_Object, KFieldName_("CBarg"),
		_Public, _F(cevent_timer_new), KType_cevent, KType_cevent, KMethodName_("new"), 3, KType_cevent_base, KFieldName_("cevent_base"), KType_ceventCBfunc, KFieldName_("konoha_CB"), KType_Object, KFieldName_("CBarg"),
		_Public, _F(cevent_event_assign), KType_Int, KType_cevent, KMethodName_("event_assign"), 5, KType_cevent_base, KFieldName_("cevent_base"), KType_Int, KFieldName_("evd"), KType_Int, KFieldName_("event"), KType_ceventCBfunc, KFieldName_("konoha_CB"), KType_Object, KFieldName_("CBarg"),
		_Public, _F(cevent_signal_assign), KType_Int, KType_cevent, KMethodName_("signal_assign"), 4, KType_cevent_base, KFieldName_("cevent_base"), KType_Int, KFieldName_("evd"), KType_ceventCBfunc, KFieldName_("konoha_CB"), KType_Object, KFieldName_("CBarg"),
		_Public, _F(cevent_timer_assign), KType_Int, KType_cevent, KMethodName_("timer_assign"), 3, KType_cevent_base, KFieldName_("cevent_base"), KType_ceventCBfunc, KFieldName_("konoha_CB"), KType_Object, KFieldName_("CBarg"),
		_Public, _F(cevent_event_add), KType_Int, KType_cevent, KMethodName_("event_add"), 1, KType_ctimeval, KFieldName_("timeval"),
		_Public, _F(cevent_event_add), KType_Int, KType_cevent, KMethodName_("signal_add"), 1, KType_ctimeval, KFieldName_("timeval"),
		_Public, _F(cevent_event_add), KType_Int, KType_cevent, KMethodName_("timer_add"), 1, KType_ctimeval, KFieldName_("timeval"),
		_Public, _F(cevent_event_del), KType_Int, KType_cevent, KMethodName_("event_del"), 0,
		_Public, _F(cevent_event_del), KType_Int, KType_cevent, KMethodName_("signal_del"), 0,
		_Public, _F(cevent_event_del), KType_Int, KType_cevent, KMethodName_("timer_del"), 0,
		_Public, _F(cevent_event_pending), KType_Int, KType_cevent, KMethodName_("event_pending"), 2, KType_Int, KFieldName_("events"), KType_Int, KFieldName_("timeval"),
		_Public, _F(cevent_signal_pending), KType_Int, KType_cevent, KMethodName_("signal_pending"), 1, KType_Int, KFieldName_("timeval"),
		_Public, _F(cevent_timer_pending), KType_Int, KType_cevent, KMethodName_("timer_pending"), 1, KType_Int, KFieldName_("timeval"),
		_Public, _F(cevent_event_initialized), KType_Int, KType_cevent, KMethodName_("event_initialized"), 0,
		_Public, _F(cevent_event_initialized), KType_Int, KType_cevent, KMethodName_("signal_initialized"), 0,
		_Public, _F(cevent_event_initialized), KType_Int, KType_cevent, KMethodName_("timer_initialized"), 0,
		_Public, _F(cevent_event_active), KType_void, KType_cevent, KMethodName_("event_active"), 2, KType_Int, KFieldName_("res"), KType_Int, KFieldName_("ncalls"),
		_Public, _F(cevent_getEvents), KType_Int, KType_cevent, KMethodName_("getEvents"), 0, 

		// cbufferevent
		_Public, _F(cbufferevent_new), KType_cbufferevent, KType_cbufferevent, KMethodName_("new"), 3, KType_cevent_base, KFieldName_("cevent_base"), KType_Int, KFieldName_("evd"), KType_Int, KFieldName_("options"),
		_Public, _F(cbufferevent_setcb), KType_void, KType_cbufferevent, KMethodName_("setcb"), 4, KType_Cbev_dataCBfunc, KFieldName_("readCB"), KType_Cbev_dataCBfunc, KFieldName_("writeCB"), KType_Cbev_eventCBfunc, KFieldName_("eventCB"), KType_Object, KFieldName_("CBarg"),
		_Public, _F(cbufferevent_socket_connect), KType_Int, KType_cbufferevent, KMethodName_("socket_connect"), 1, KType_Sockaddr_in, KFieldName_("sockaddr"),
		_Public, _F(cbufferevent_enable), KType_Int, KType_cbufferevent, KMethodName_("enable"), 1, KType_Int, KFieldName_("event"),
		_Public, _F(cbufferevent_write), KType_Int, KType_cbufferevent, KMethodName_("write"), 1, KType_Bytes, KFieldName_("writebuffer"),
		_Public, _F(cbufferevent_read), KType_Int, KType_cbufferevent, KMethodName_("read"), 1, KType_Bytes, KFieldName_("readbuffer"),

		// cevhttp
		_Public, _F(cevhttp_new), KType_cevhttp, KType_cevhttp, KMethodName_("new"), 1, KType_cevent_base, KFieldName_("cevent_base"),
		_Public, _F(cevhttp_bind_socket), KType_Int, KType_cevhttp, KMethodName_("bind_socket"), 2, KType_String, KFieldName_("address"), KType_Int, KFieldName_("port"),
		_Public, _F(cevhttp_bind_socket_with_handle), KType_cevhttp_bound_socket, KType_cevhttp, KMethodName_("bind_socket_with_handle"), 2, KType_String, KFieldName_("address"), KType_Int, KFieldName_("port"),
		_Public, _F(cevhttp_accept_socket), KType_Int, KType_cevhttp, KMethodName_("accept_socket"), 1, KType_Int, KFieldName_("fd"),
		_Public, _F(cevhttp_accept_socket_with_handle), KType_cevhttp_bound_socket, KType_cevhttp, KMethodName_("accept_socket_with_handle"), 1, KType_Int, KFieldName_("fd"),
		_Public, _F(cevhttp_bind_listener), KType_cevhttp_bound_socket, KType_cevhttp, KMethodName_("bind_listener"), 1, KType_cevconnlistener, KFieldName_("listener"),
		_Public, _F(cevhttp_del_accept_socket), KType_void, KType_cevhttp, KMethodName_("del_accept_socket"), 1, KType_cevhttp_bound_socket, KFieldName_("bound_socket"),
		_Public, _F(cevhttp_set_max_headers_size), KType_void, KType_cevhttp, KMethodName_("set_max_headers_size"), 1, KType_Int, KFieldName_("max_headers_size"),
		_Public, _F(cevhttp_set_max_body_size), KType_void, KType_cevhttp, KMethodName_("set_max_body_size"), 1, KType_Int, KFieldName_("max_body_size"),
		_Public, _F(cevhttp_set_allowed_methods), KType_void, KType_cevhttp, KMethodName_("set_allowed_methods"), 1, KType_Int, KFieldName_("methods"),
		_Public, _F(cevhttp_set_cb), KType_Int, KType_cevhttp, KMethodName_("set_cb"), 3, KType_String, KFieldName_("uri"), KType_cevhttp_requestCBfunc, KFieldName_("evhttpCB"), KType_Object, KFieldName_("CBarg"),
		_Public, _F(cevhttp_del_cb), KType_Int, KType_cevhttp, KMethodName_("del_cb"), 1, KType_String, KFieldName_("uri"),
		_Public, _F(cevhttp_set_gencb), KType_void, KType_cevhttp, KMethodName_("set_gencb"), 2, KType_cevhttp_requestCBfunc, KFieldName_("evhttpCB"), KType_Object, KFieldName_("CBarg"),
		_Public, _F(cevhttp_add_virtual_host), KType_Int, KType_cevhttp, KMethodName_("add_virtual_host"), 2, KType_String, KFieldName_("pattern"), KType_cevhttp, KFieldName_("vhost"),
		_Public, _F(cevhttp_remove_virtual_host), KType_Int, KType_cevhttp, KMethodName_("remove_virtual_host"), 1, KType_cevhttp, KFieldName_("vhost"),
		_Public, _F(cevhttp_add_server_alias), KType_Int, KType_cevhttp, KMethodName_("add_server_alias"), 1, KType_String, KFieldName_("alias"),
		_Public, _F(cevhttp_remove_server_alias), KType_Int, KType_cevhttp, KMethodName_("remove_server_alias"), 1, KType_String, KFieldName_("alias"),
		_Public, _F(cevhttp_set_timeout), KType_void, KType_cevhttp, KMethodName_("set_timeout"), 1, KType_Int, KFieldName_("timeout_in_secs"),

		// cevhttp_bound_socket
		_Public, _F(cevhttp_bound_socket_get_listener), KType_cevconnlistener, KType_cevhttp_bound_socket, KMethodName_("get_listener"), 0,
		_Public, _F(cevhttp_bound_socket_get_fd), KType_Int, KType_cevhttp_bound_socket, KMethodName_("get_fd"), 0,

		// cevhttp_request
		_Public, _F(cevhttp_request_new), KType_cevhttp_request, KType_cevhttp_request, KMethodName_("new"), 2, KType_cevhttp_requestCBfunc, KFieldName_("cb"), KType_Object, KFieldName_("arg"),
		_Public, _F(cevhttp_request_send_error), KType_void, KType_cevhttp_request, KMethodName_("send_error"), 2, KType_Int, KFieldName_("error"), KType_String, KFieldName_("reason"),
		_Public, _F(cevhttp_request_send_reply), KType_void, KType_cevhttp_request, KMethodName_("send_reply"), 3, KType_Int, KFieldName_("code"), KType_String, KFieldName_("reason"), KType_cevbuffer, KFieldName_("databuf"),
		_Public, _F(cevhttp_request_send_reply_start), KType_void, KType_cevhttp_request, KMethodName_("send_reply_start"), 2, KType_Int, KFieldName_("code"), KType_String, KFieldName_("reason"),
		_Public, _F(cevhttp_request_reply_chunk), KType_void, KType_cevhttp_request, KMethodName_("reply_chunk"), 1, KType_cevbuffer, KFieldName_("databuf"),
		_Public, _F(cevhttp_request_reply_end), KType_void, KType_cevhttp_request, KMethodName_("reply_send"), 0,
		_Public, _F(cevhttp_request_set_chunked_cb), KType_void, KType_cevhttp_request, KMethodName_("set_chunked_cb"), 2, KType_cevhttp_requestCBfunc, KFieldName_("cb"), KType_Object, KFieldName_("arg"),
		_Public, _F(cevhttp_request_own), KType_void, KType_cevhttp_request, KMethodName_("own"), 0,
		_Public, _F(cevhttp_request_is_owned), KType_Int, KType_cevhttp_request, KMethodName_("is_owned"), 0,
		_Public, _F(cevhttp_request_get_connection), KType_cevhttp_connection, KType_cevhttp_request, KMethodName_("get_connection"), 0,
		_Public, _F(cevhttp_request_cancel), KType_void, KType_cevhttp_request, KMethodName_("cancel"), 0,
		_Public, _F(cevhttp_request_get_uri), KType_String, KType_cevhttp_request, KMethodName_("get_uri"), 0,
		_Public, _F(cevhttp_request_get_evhttp_uri), KType_cevhttp_uri, KType_cevhttp_request, KMethodName_("get_evhttp_uri"), 0,
		_Public, _F(cevhttp_request_get_command), KType_Int, KType_cevhttp_request, KMethodName_("get_command"), 0,
		_Public, _F(cevhttp_request_get_response_code), KType_Int, KType_cevhttp_request, KMethodName_("get_response_code"), 0,
		_Public, _F(cevhttp_request_get_input_headers), KType_cevkeyvalq, KType_cevhttp_request, KMethodName_("get_input_headers"), 0,
		_Public, _F(cevhttp_request_get_output_headers), KType_cevkeyvalq, KType_cevhttp_request, KMethodName_("get_output_headers"), 0,
		_Public, _F(cevhttp_request_get_input_buffer), KType_cevbuffer, KType_cevhttp_request, KMethodName_("get_input_buffer"), 0,
		_Public, _F(cevhttp_request_get_output_buffer), KType_cevbuffer, KType_cevhttp_request, KMethodName_("get_output_buffer"), 0,
		_Public, _F(cevhttp_request_get_host), KType_String, KType_cevhttp_request, KMethodName_("get_host"), 0,

		// cevhttp_connection
		_Public, _F(cevhttp_connection_new), KType_cevhttp_connection, KType_cevhttp_connection, KMethodName_("new"), 4, KType_cevent_base, KFieldName_("event_base"), KType_cevdns_base, KFieldName_("dnsbase"), KType_String, KFieldName_("address"), KType_Int, KFieldName_("port"),
		_Public, _F(cevhttp_connection_get_bufferevent), KType_cevbuffer, KType_cevhttp_connection, KMethodName_("get_bufferevent"), 0,
		_Public, _F(cevhttp_connection_get_base), KType_cevent_base, KType_cevhttp_connection, KMethodName_("get_base"), 0,
		_Public, _F(cevhttp_connection_set_max_headers_size), KType_void, KType_cevhttp_connection, KMethodName_("set_max_headers_size"), 1, KType_Int, KFieldName_("new_max_headers_size"),
		_Public, _F(cevhttp_connection_set_max_body_size), KType_void, KType_cevhttp_connection, KMethodName_("set_max_body_size"), 1, KType_Int, KFieldName_("new_max_body_size"),
		_Public, _F(cevhttp_connection_set_local_address), KType_void, KType_cevhttp_connection, KMethodName_("set_local_address"), 1, KType_String, KFieldName_("address"),
		_Public, _F(cevhttp_connection_set_local_port), KType_void, KType_cevhttp_connection, KMethodName_("set_local_port"), 1, KType_Int, KFieldName_("port"),
		_Public, _F(cevhttp_connection_set_timeout), KType_void, KType_cevhttp_connection, KMethodName_("set_timeout"), 1, KType_Int, KFieldName_("timeout_in_secs"),
		_Public, _F(cevhttp_connection_set_retries), KType_void, KType_cevhttp_connection, KMethodName_("set_retries"), 1, KType_Int, KFieldName_("retry_max"),
		_Public, _F(cevhttp_connection_set_closecb), KType_void, KType_cevhttp_connection, KMethodName_("set_closecb"), 2, KType_cevhttp_connectionCBfunc, KFieldName_("closeCB"), KType_Object, KFieldName_("arg"),
		_Public, _F(cevhttp_connection_get_peer), KType_connection_peer, KType_cevhttp_connection, KMethodName_("get_peer"), 0,
		_Public, _F(cevhttp_connection_make_request), KType_Int, KType_cevhttp_connection, KMethodName_("make_request"), 3, KType_cevhttp_request, KFieldName_("req"), KType_Int, KFieldName_("evhttp_cmd_type"), KType_String, KFieldName_("uri"),

		// cevkeyvalq
		_Public, _F(cevkeyvalq_find_header), KType_String, KType_cevkeyvalq, KMethodName_("find_header"), 1, KType_String, KFieldName_("key"),
		_Public, _F(cevkeyvalq_remove_header), KType_Int, KType_cevkeyvalq, KMethodName_("remove_header"), 1, KType_String, KFieldName_("key"),
		_Public, _F(cevkeyvalq_add_header), KType_Int, KType_cevkeyvalq, KMethodName_("add_header"), 2, KType_String, KFieldName_("key"), KType_String, KFieldName_("value"),
		_Public, _F(cevkeyvalq_clear_header), KType_Int, KType_cevkeyvalq, KMethodName_("clear_header"), 0,

		// cevhttp_uri
		_Public, _F(cevhttp_uri_new), KType_cevhttp_uri, KType_cevhttp_uri, KMethodName_("new"), 0,
		_Public, _F(cevhttp_uri_set_flags), KType_void, KType_cevhttp_uri, KMethodName_("set_flags"), 1, KType_Int, KFieldName_("flags"),
		_Public|_Im, _F(cevhttp_uri_get_scheme), KType_String, KType_cevhttp_uri, KMethodName_("get_scheme"), 0,
		_Public|_Im, _F(cevhttp_uri_get_userinfo), KType_String, KType_cevhttp_uri, KMethodName_("get_userinfo"), 0,
		_Public|_Im, _F(cevhttp_uri_get_host), KType_String, KType_cevhttp_uri, KMethodName_("get_host"), 0,
		_Public|_Im, _F(cevhttp_uri_get_port), KType_Int, KType_cevhttp_uri, KMethodName_("get_port"), 0,
		_Public|_Im, _F(cevhttp_uri_get_path), KType_String, KType_cevhttp_uri, KMethodName_("get_path"), 0,
		_Public|_Im, _F(cevhttp_uri_get_query), KType_String, KType_cevhttp_uri, KMethodName_("get_query"), 0,
		_Public|_Im, _F(cevhttp_uri_get_fragment), KType_String, KType_cevhttp_uri, KMethodName_("get_fragment"), 0,
		_Public, _F(cevhttp_uri_set_scheme), KType_Int, KType_cevhttp_uri, KMethodName_("set_scheme"), 1, KType_String, KFieldName_("scheme"),
		_Public, _F(cevhttp_uri_set_userinfo), KType_Int, KType_cevhttp_uri, KMethodName_("set_userinfo"), 1, KType_String, KFieldName_("userinfo"),
		_Public, _F(cevhttp_uri_set_host), KType_Int, KType_cevhttp_uri, KMethodName_("set_host"), 1, KType_String, KFieldName_("host"),
		_Public, _F(cevhttp_uri_set_port), KType_Int, KType_cevhttp_uri, KMethodName_("set_port"), 1, KType_Int, KFieldName_("port"),
		_Public, _F(cevhttp_uri_set_path), KType_Int, KType_cevhttp_uri, KMethodName_("set_path"), 1, KType_String, KFieldName_("path"),
		_Public, _F(cevhttp_uri_set_query), KType_Int, KType_cevhttp_uri, KMethodName_("set_query"), 1, KType_String, KFieldName_("query"),
		_Public, _F(cevhttp_uri_set_fragment), KType_Int, KType_cevhttp_uri, KMethodName_("set_fragment"), 1, KType_String, KFieldName_("fragment"),
		_Public, _F(cevhttp_uri_parse_with_flags), KType_cevhttp_uri, KType_cevhttp_uri, KMethodName_("parse_with_flags"), 2, KType_String, KFieldName_("source_uri"), KType_Int, KFieldName_("flags"),
		_Public, _F(cevhttp_uri_parse), KType_cevhttp_uri, KType_cevhttp_uri, KMethodName_("parse"), 1, KType_String, KFieldName_("source_uri"),
		_Public, _F(cevhttp_uri_join), KType_cevhttp_uri, KType_cevhttp_uri, KMethodName_("join"), 1, KType_Bytes, KFieldName_("buf"),

		_Static|_Public, _F(cevhttp_uri_encode_uri), KType_String, KType_cevhttp_uri, KMethodName_("encode_uri"), 1, KType_String, KFieldName_("str"),
		_Static|_Public, _F(cevhttp_uri_uriencode), KType_String, KType_cevhttp_uri, KMethodName_("uriencode"), 3, KType_String, KFieldName_("str"), KType_Int, KFieldName_("size"), KType_Int, KFieldName_("space_to_plus"),
		_Static|_Public, _F(cevhttp_uri_decode_uri), KType_String, KType_cevhttp_uri, KMethodName_("decode_uri"), 1, KType_String, KFieldName_("uri"),
		_Static|_Public, _F(cevhttp_uri_uridecode), KType_String, KType_cevhttp_uri, KMethodName_("uridecode"), 2, KType_String, KFieldName_("uri"), KType_Int, KFieldName_("decode_plus"),
		_Static|_Public, _F(cevhttp_uri_parse_query_str), KType_Int, KType_cevhttp_uri, KMethodName_("parse_query_str"), 2, KType_String, KFieldName_("uri"), KType_cevkeyvalq, KFieldName_("headers"),
		_Static|_Public, _F(cevhttp_uri_htmlescape), KType_String, KType_cevhttp_uri, KMethodName_("htmlescape"), 1, KType_String, KFieldName_("html"),

		// ctimeval
		_Public, _F(ctimeval_new), KType_ctimeval, KType_ctimeval, KMethodName_("new"), 2, KType_Int, KFieldName_("tv_sec"), KType_Int, KFieldName_("tv_usec"),

		// Sockaddr_in
		_Public, _F(Sockaddr_in_new), KType_Sockaddr_in, KType_Sockaddr_in, KMethodName_("new"), 3, KType_Int, KFieldName_("family"), KType_Int, KFieldName_("addr"), KType_Int, KFieldName_("port"),

		DEND, /* <= sentinel */
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);


	KDEFINE_INT_CONST IntData[] = {
		// === for event_new() ===
		{KDefineConstInt(EV_TIMEOUT)},
		{KDefineConstInt(EV_READ)},
		{KDefineConstInt(EV_WRITE)},
		{KDefineConstInt(EV_SIGNAL)},
		{KDefineConstInt(EV_PERSIST)},
		{KDefineConstInt(EV_ET)},

		// === for bufferevent ===
		// bufferevent.h
		{KDefineConstInt(BEV_EVENT_READING)},
		{KDefineConstInt(BEV_EVENT_WRITING)},
		{KDefineConstInt(BEV_EVENT_EOF)},
		{KDefineConstInt(BEV_EVENT_ERROR)},
		{KDefineConstInt(BEV_EVENT_TIMEOUT)},
		{KDefineConstInt(BEV_EVENT_CONNECTED)},

		// bufferevent.h: enum bufferevent_options
		{KDefineConstInt(BEV_OPT_CLOSE_ON_FREE)},
		{KDefineConstInt(BEV_OPT_THREADSAFE)},
		{KDefineConstInt(BEV_OPT_DEFER_CALLBACKS)},
		{KDefineConstInt(BEV_OPT_UNLOCK_CALLBACKS)},

		// http.h: enum evhttp_cmd_type
		{KDefineConstInt(EVHTTP_REQ_GET)},
		{KDefineConstInt(EVHTTP_REQ_POST)},
		{KDefineConstInt(EVHTTP_REQ_HEAD)},
		{KDefineConstInt(EVHTTP_REQ_PUT)},
		{KDefineConstInt(EVHTTP_REQ_DELETE)},
		{KDefineConstInt(EVHTTP_REQ_OPTIONS)},
		{KDefineConstInt(EVHTTP_REQ_TRACE)},
		{KDefineConstInt(EVHTTP_REQ_CONNECT)},
		{KDefineConstInt(EVHTTP_REQ_PATCH)},

		{KDefineConstInt(AF_INET)},// TODO should be implement in posix.socket package

		{} /* <= sentinel */
	};

	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(IntData), trace);

	return true;
}

static kbool_t Libevent_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *Libevent_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "libevent2.0.19", "0.1");
	d.PackupNameSpace	= Libevent_PackupNameSpace;
	d.ExportNameSpace	= Libevent_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
