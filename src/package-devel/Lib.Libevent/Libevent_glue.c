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
	struct cevent_base *eb = (struct cevent_base *) o;
	eb->event_base = NULL;
}

static void cevent_base_Free(KonohaContext *kctx, kObject *o)
{
	struct cevent_base *eb = (struct cevent_base *) o;
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
 * cevent_base Class 1st stage callback from event_base_dispatch(), NEVER BE CALLED FROM OTHERS.
 */
static void cevent_CB_method_invoke(evutil_socket_t evd, short event, void *arg) {
	kcevent *ev = arg;
	KonohaContext *kctx = ev->kctx;

	BEGIN_UnusedStack(lsfp);
	KClass *returnType = kMethod_GetReturnType(ev->kcb->method);
	KUnsafeFieldSet(lsfp[0].asObject, K_NULL);
	lsfp[1].intValue = evd;
	lsfp[2].intValue = event;
	KUnsafeFieldSet(lsfp[3].asObject, ev->kcbArg);

	KStackSetFuncAll(lsfp, KLIB Knull(kctx, returnType), 0/*UL*/, ev->kcb, 3);
	KStackCall(lsfp);
	END_UnusedStack();
}


static void cevent_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct cevent *ev = (struct cevent *) o;
	ev->kctx = NULL;
	ev->event = NULL;
	KFieldInit(ev, ev->kcb, K_NULL);
	KFieldInit(ev, ev->kcbArg, K_NULL);
	KFieldInit(ev, ev->kctimeval, K_NULL);
}

static void cevent_Free(KonohaContext *kctx, kObject *o)
{
	struct cevent *ev = (struct cevent *) o;

	if(ev->event != NULL) {
		event_free(ev->event);
		ev->event = NULL;
	}
	ev->kctx = NULL;
}

static void cevent_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	struct cevent *ev = (struct cevent *) o;
	KRefTrace(ev->kcb);
	KRefTrace(ev->kcbArg);
	KRefTrace(ev->kctimeval);
}

//## cevent cevent.new(cevent_base event_base, int evd, int event, Func[void, int, int, Object] cb, Object cbArg);
static KMETHOD cevent_event_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct cevent *ev = (struct cevent *) sfp[0].asObject;
	ev->kctx = kctx;
	struct cevent_base *cEvent_base = (struct cevent_base *)sfp[1].asObject;
	evutil_socket_t evd = (evutil_socket_t)sfp[2].intValue;
	short event = (short)(sfp[3].intValue & 0xffff);
	KFieldSet(ev, ev->kcb, sfp[4].asFunc);
	KFieldSet(ev, ev->kcbArg, sfp[5].asObject);	//deliver to callback method

	ev->event = event_new(cEvent_base->event_base, evd, event, cevent_CB_method_invoke, ev);
	KReturn(ev);
}

//## cevent cevent.new(cevent_base event_base, int evd, Func[void, int, int, Object] cb, Object cbArg);
//## for signal event
static KMETHOD cevent_signal_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct cevent *ev = (struct cevent *) sfp[0].asObject;
	ev->kctx = kctx;
	struct cevent_base *cEvent_base = (struct cevent_base *)sfp[1].asObject;
	evutil_socket_t evd = (evutil_socket_t)sfp[2].intValue;
	KFieldSet(ev, ev->kcb, sfp[3].asFunc);
	KFieldSet(ev, ev->kcbArg, sfp[4].asObject);	//deliver to callback method

	ev->event = evsignal_new(cEvent_base->event_base, evd, cevent_CB_method_invoke, ev);
	KReturn(ev);
}

//## cevent cevent.new(cevent_base event_base, Func[void, int, int, Object] cb, Object cbArg);
//## for timer event
static KMETHOD cevent_timer_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct cevent *ev = (struct cevent *) sfp[0].asObject;
	ev->kctx = kctx;
	struct cevent_base *cEvent_base = (struct cevent_base *)sfp[1].asObject;
	KFieldSet(ev, ev->kcb, sfp[2].asFunc);
	KFieldSet(ev, ev->kcbArg, sfp[3].asObject);	//deliver to callback method

	ev->event = evtimer_new(cEvent_base->event_base, cevent_CB_method_invoke, ev);
	KReturn(ev);
}

//## cevent cevent.event_assign(cevent_base event_base, int evd, int event, Func[void, int, int, Object] cb, Object cbArg);
static KMETHOD cevent_event_assign(KonohaContext *kctx, KonohaStack *sfp)
{
	struct cevent *ev = (struct cevent *) sfp[0].asObject;
	struct cevent_base *cEvent_base = (struct cevent_base *)sfp[1].asObject;
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
	struct cevent *ev = (struct cevent *) sfp[0].asObject;
	struct cevent_base *cEvent_base = (struct cevent_base *)sfp[1].asObject;
	evutil_socket_t evd = (evutil_socket_t)sfp[2].intValue;
	KFieldSet(ev, ev->kcb, sfp[3].asFunc);
	KFieldSet(ev, ev->kcbArg, sfp[4].asObject);	//deliver to callback method

	int ret = evsignal_assign(ev->event, cEvent_base->event_base, evd, cevent_CB_method_invoke, ev);
	KReturnUnboxValue(ret);
}

//## cevent cevent.timer_assign(cevent_base event_base, Func[void, int, int, Object] cb, Object cbArg);
static KMETHOD cevent_timer_assign(KonohaContext *kctx, KonohaStack *sfp)
{
	struct cevent *ev = (struct cevent *) sfp[0].asObject;
	struct cevent_base *cEvent_base = (struct cevent_base *)sfp[1].asObject;
	KFieldSet(ev, ev->kcb, sfp[2].asFunc);
	KFieldSet(ev, ev->kcbArg, sfp[3].asObject);	//deliver to callback method

	int ret = evtimer_assign(ev->event, cEvent_base->event_base, cevent_CB_method_invoke, ev);
	KReturnUnboxValue(ret);
}

#define tvIsNull(tv_p)	((tv_p)->timeval.tv_sec == 0 && (tv_p)->timeval.tv_usec == 0)
//## int cevent.event_add(cevent_base event, ctimeval tv);
static KMETHOD cevent_event_add(KonohaContext *kctx, KonohaStack* sfp)
{
	kcevent *kcev = (kcevent *)sfp[0].asObject;
	kctimeval *tv = (kctimeval *)sfp[1].asObject;
	KUnsafeFieldSet(kcev->kctimeval, tv);
	int ret = event_add(kcev->event, tvIsNull(tv) ? NULL : &tv->timeval);
	KReturnUnboxValue(ret);
}

//## int cevent.event_del(cevent event);
static KMETHOD cevent_event_del(KonohaContext *kctx, KonohaStack* sfp)
{
	kcevent *kcev = (kcevent *)sfp[0].asObject;
	KUnsafeFieldInit(kcev->kctimeval, K_NULL);	//delete reference
	int ret = event_del(kcev->event);
	KReturnUnboxValue(ret);
}

//## int cevent.event_pending(short events, ctimeval tv);
static KMETHOD cevent_event_pending(KonohaContext *kctx, KonohaStack* sfp)
{
	kcevent *kcev = (kcevent *)sfp[0].asObject;
	short events = (short)sfp[1].intValue;
	kctimeval *tv = (kctimeval *)sfp[2].asObject;
	int ret = event_pending(kcev->event, events, tvIsNull(tv) ? NULL : &tv->timeval);
	KReturnUnboxValue(ret);
}

//## int cevent.event_initialized();
static KMETHOD cevent_event_initialized(KonohaContext *kctx, KonohaStack* sfp)
{
	kcevent *kcev = (kcevent *)sfp[0].asObject;
	int ret = event_initialized(kcev->event);
	KReturnUnboxValue(ret);
}

//## void cevent.event_free();
static KMETHOD cevent_event_free(KonohaContext *kctx, KonohaStack *sfp)
{
	struct cevent *ev = (struct cevent *) sfp[0].asObject;
	event_free(ev->event);
	KReturnVoid();
}

//## void cevent.event_active(int res, int ncalls);
static KMETHOD cevent_event_active(KonohaContext *kctx, KonohaStack *sfp)
{
	struct cevent *ev = (struct cevent *) sfp[0].asObject;
	int res = sfp[1].intValue;
	short ncalls = (short)sfp[2].intValue;
	event_active(ev->event, res, ncalls);
	KReturnVoid();
}


//## cevent cevent.getEvents();
// get event category field
static KMETHOD cevent_getEvents(KonohaContext *kctx, KonohaStack *sfp)
{
	struct cevent *ev = (struct cevent *) sfp[0].asObject;
	KReturnUnboxValue(ev->event->ev_events);
}


/* ======================================================================== */
// cbufferevent class

static void cbufferevent_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct cbufferevent *bev = (struct cbufferevent *) o;
	bev->kctx = NULL;
	bev->bev = NULL;
	KFieldInit(bev, bev->readcb, K_NULL);
	KFieldInit(bev, bev->writecb, K_NULL);
	KFieldInit(bev, bev->eventcb, K_NULL);
	KFieldInit(bev, bev->kcbArg, K_NULL);
}

static void cbufferevent_Free(KonohaContext *kctx, kObject *o)
{
	struct cbufferevent *bev = (struct cbufferevent *) o;

	bev->kctx = NULL;
	if (bev->bev != NULL) {
		bufferevent_free(bev->bev);
		bev->bev = NULL;
	}
}

static void cbufferevent_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	struct cbufferevent *bev = (struct cbufferevent *) o;
	KRefTrace(bev->readcb);
	KRefTrace(bev->writecb);
	KRefTrace(bev->eventcb);
	KRefTrace(bev->kcbArg);
}

//## cbufferevent cbufferevent.new(cevent_base event_base, int evd, int option);
static KMETHOD cbufferevent_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct cbufferevent *bev = (struct cbufferevent *)sfp[0].asObject;
	bev->kctx = kctx;
	struct cevent_base *cev_base = (struct cevent_base *)sfp[1].asObject;
	evutil_socket_t evd = (evutil_socket_t)sfp[2].intValue;
	int options = sfp[3].intValue;

	bev->bev = bufferevent_socket_new(cev_base->event_base, evd, options);
	KReturn(bev);
}

static void Cbev_dataCB_dispatcher(kFunc *datacb, struct bufferevent *bev, void *arg)
{
	kcbufferevent *kcbev = arg;
	KonohaContext *kctx = kcbev->kctx;
	assert(bev == kcbev->bev);

	BEGIN_UnusedStack(lsfp);
	KClass *returnType = kMethod_GetReturnType(datacb->method);
	KUnsafeFieldSet(lsfp[0].asObject, K_NULL/*(kObject *)kcbev*/);
	KUnsafeFieldSet(lsfp[1].asObject, (kObject *)kcbev);
	KUnsafeFieldSet(lsfp[2].asObject, kcbev->kcbArg);
	KStackSetFuncAll(lsfp, KLIB Knull(kctx, returnType), 0/*UL*/, datacb, 2);
	KStackCall(lsfp);
	END_UnusedStack();
}

/*
 * cbufferevent Class (*buffer_data_cb)() 1st stage callback from event_base_dispatch(), NEVER BE CALLED FROM OTHERS.
 */
static void cbev_readCB_method_invoke(struct bufferevent *bev, void *arg)
{
	kcbufferevent *kcbev = arg;
	Cbev_dataCB_dispatcher(kcbev->readcb, bev, arg);
}

/*
 * cbufferevent Class (*buffer_data_cb)() 1st stage callback from event_base_dispatch(), NEVER BE CALLED FROM OTHERS.
 */
static void cbev_writeCB_method_invoke(struct bufferevent *bev, void *arg)
{
	kcbufferevent *kcbev = arg;
	Cbev_dataCB_dispatcher(kcbev->writecb, bev, arg);
}

/*
 * cbufferevent Class (*buffer_event_cb)() 1st stage callback from event_base_dispatch(), NEVER BE CALLED FROM OTHERS.
 */
static void cbev_eventCB_method_invoke(struct bufferevent *bev, short what, void *arg)
{
	kcbufferevent *kcbev = arg;
	KonohaContext *kctx = kcbev->kctx;
	assert(bev == kcbev->bev);

	BEGIN_UnusedStack(lsfp);
	KClass *returnType = kMethod_GetReturnType(kcbev->eventcb->method);
	KUnsafeFieldSet(lsfp[0].asObject, K_NULL);
	KUnsafeFieldSet(lsfp[1].asObject, (kObject *)kcbev);
	lsfp[2].intValue = what;
	KUnsafeFieldSet(lsfp[3].asObject, (kObject *)kcbev->kcbArg);
	KStackSetFuncAll(lsfp, KLIB Knull(kctx, returnType), 0/*UL*/, kcbev->eventcb, 3);
	KStackCall(lsfp);
	END_UnusedStack();
}

//## void cbufferevent.setcb(
//##	Func[void, cbufferevent, Object] readcb,
//##	Func[void, cbufferevent, Object] writecb,
//##	Func[void, cbufferevent, int, Object] eventcb,
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

//## int cbufferevent.socket_connect(Sockaddr_in sa);
static KMETHOD cbufferevent_socket_connect(KonohaContext *kctx, KonohaStack *sfp)
{
	kcbufferevent *bev = (kcbufferevent *)sfp[0].asObject;
	kSockaddr_in *sa = (kSockaddr_in *)sfp[1].asObject;
	int ret = bufferevent_socket_connect(bev->bev, (struct sockaddr *)&sa->sockaddr, sizeof sa->sockaddr);
	KReturnUnboxValue(ret);
}

//## int cbufferevent.enable(int event);
static KMETHOD cbufferevent_enable(KonohaContext *kctx, KonohaStack *sfp)
{
	kcbufferevent *bev = (kcbufferevent *)sfp[0].asObject;
	short event = (short)sfp[1].intValue;

	int ret = bufferevent_enable(bev->bev, event);
	KReturnUnboxValue(ret);
}

//## int cbufferevent.write(Bytes buf);
static KMETHOD cbufferevent_write(KonohaContext *kctx, KonohaStack *sfp)
{
	kcbufferevent *bev = (kcbufferevent *)sfp[0].asObject;
	kBytes *buf = sfp[1].asBytes;

	int ret = bufferevent_write(bev->bev, buf->byteptr, buf->bytesize);
	KReturnUnboxValue(ret);
}

//## int cbufferevent.bufferevent_read(Bytes buf);
static KMETHOD cbufferevent_read(KonohaContext *kctx, KonohaStack *sfp)
{
	kcbufferevent *bev = (kcbufferevent *)sfp[0].asObject;
	kBytes *buf = sfp[1].asBytes;

	int ret = bufferevent_read(bev->bev, buf->buf, buf->bytesize);
	KReturnUnboxValue(ret);
}


/* ======================================================================== */
// ctimeval class

static void ctimeval_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct ctimeval *tv = (struct ctimeval *) o;
	tv->timeval.tv_sec = 0;
	tv->timeval.tv_usec = 0;
}

//## ctimeval ctimeval.new(int tv_sec, int tv_usec);
static KMETHOD ctimeval_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct ctimeval *tv = (struct ctimeval *) sfp[0].asObject;
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
	struct Sockaddr_in *sa = (struct Sockaddr_in *) o;
	memset(&sa->sockaddr, 0, sizeof (struct sockaddr));
}

//## Sockaddr_in Sockaddr_in.new(int family, int addr, int port);
static KMETHOD Sockaddr_in_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct Sockaddr_in *sa = (struct Sockaddr_in *) sfp[0].asObject;
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
	KClass *cevent_baseClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defcevent_base, trace);

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
	KClass *cbuffereventClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defcbufferevent, trace);

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
	int KType_cevent_base = cevent_baseClass->typeId;
	int KType_cevent = ceventClass->typeId;
	int KType_cbufferevent = cbuffereventClass->typeId;
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
		_Public, _F(cevent_event_pending), KType_Int, KType_cevent, KMethodName_("event_pending"), 2, KType_Int, KFieldName_("events"), KType_Int, KFieldName_("ctimeval"),
		_Public, _F(cevent_event_initialized), KType_Int, KType_cevent, KMethodName_("event_initialized"), 0,
		_Public, _F(cevent_event_free), KType_void, KType_cevent, KMethodName_("event_free"), 0,
		_Public, _F(cevent_event_active), KType_void, KType_cevent, KMethodName_("event_active"), 2, KType_Int, KFieldName_("res"), KType_Int, KFieldName_("ncalls"),
		_Public, _F(cevent_getEvents), KType_Int, KType_cevent, KMethodName_("getEvents"), 0, 

		// cbufferevent
		_Public, _F(cbufferevent_new), KType_cbufferevent, KType_cbufferevent, KMethodName_("new"), 3, KType_cevent_base, KFieldName_("cevent_base"), KType_Int, KFieldName_("evd"), KType_Int, KFieldName_("options"),
		_Public, _F(cbufferevent_setcb), KType_void, KType_cbufferevent, KMethodName_("setcb"), 4, KType_Cbev_dataCBfunc, KFieldName_("readCB"), KType_Cbev_dataCBfunc, KFieldName_("writeCB"), KType_Cbev_eventCBfunc, KFieldName_("eventCB"), KType_Object, KFieldName_("CBarg"),
		_Public, _F(cbufferevent_socket_connect), KType_Int, KType_cbufferevent, KMethodName_("socket_connect"), 1, KType_Sockaddr_in, KFieldName_("sockaddr"),
		_Public, _F(cbufferevent_enable), KType_Int, KType_cbufferevent, KMethodName_("enable"), 1, KType_Int, KFieldName_("event"),
		_Public, _F(cbufferevent_write), KType_Int, KType_cbufferevent, KMethodName_("write"), 1, KType_Bytes, KFieldName_("writebuffer"),
		_Public, _F(cbufferevent_read), KType_Int, KType_cbufferevent, KMethodName_("read"), 1, KType_Bytes, KFieldName_("readbuffer"),

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
