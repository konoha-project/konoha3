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
typedef struct Cevent_base {
	kObjectHeader h;
	struct event_base *event_base;
} kCevent_base;

typedef struct Cevent {
	kObjectHeader h;
	struct event *event;
} kCevent;

typedef struct Cbufferevent {
	kObjectHeader h;
	struct bufferevent *bev;
} kCbufferevent;

typedef struct EventCBArg {	//callback-method argument wrapper
	kObjectHeader h;
	KonohaContext *kctx;
	kFunc *kcb;		// konoha call back method
	kObject *arg;
} kEventCBArg;

enum e_buffereventCB {BEV_ReadCB, BEV_WriteCB, BEV_EventCB, NUM_BuffereventCB};
typedef struct BuffereventCBArg {	//callback-method argument wrapper
	kObjectHeader h;
	KonohaContext *kctx;
	kCbufferevent *cbev;	//'cbev' is set in Cbufferevent_setcb() function
	kFunc *kcb[NUM_BuffereventCB];		// konoha call back methods
	kObject *arg;
} kBuffereventCBArg;

#include <sys/time.h>
typedef struct TimeVal {
	kObjectHeader h;
	struct timeval timeval;
} kTimeVal;

// TODO ----- should be implement in posix.socket package
#include <sys/socket.h>
typedef struct Sockaddr_in {
	kObjectHeader h;
	struct sockaddr_in sockaddr;
} kSockaddr_in;
// TODO should be implement in posix.socket package -----


/* ======================================================================== */
// Cevent_base class
static void Cevent_base_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct Cevent_base *ev = (struct Cevent_base *) o;
	ev->event_base = NULL;
}

static void Cevent_base_Free(KonohaContext *kctx, kObject *o)
{
	struct Cevent_base *ev = (struct Cevent_base *) o;
	if(ev->event_base != NULL) {
		event_base_free(ev->event_base);
		ev->event_base = NULL;
	}
}

//## Cevent_base Cevent_base.new();
static KMETHOD Cevent_base_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kCevent_base *ev = (kCevent_base *)sfp[0].asObject;
	ev->event_base = event_base_new();
	KReturn(ev);
}

//## Cevent_base Cevent_base.event_dispatch();
static KMETHOD Cevent_base_event_dispatch(KonohaContext *kctx, KonohaStack *sfp)
{
	kCevent_base *ev = (kCevent_base *)sfp[0].asObject;
	int ret = event_base_dispatch(ev->event_base);
	KReturnUnboxValue(ret);
}

/*
 * Cevent_base Class 1st stage callback from event_base_dispatch(), NEVER BE CALLED FROM OTHERS.
 */
static void Cevent_callback_1st(evutil_socket_t evd, short event, void *arg) {
	kEventCBArg *cbArg = arg;
	KonohaContext *kctx = cbArg->kctx;

	BEGIN_UnusedStack(lsfp);
	KClass *returnType = kMethod_GetReturnType(cbArg->kcb->method);
	KUnsafeFieldSet(lsfp[0].asObject, K_NULL);
	KUnsafeFieldSet(lsfp[1].intValue, evd);
	KUnsafeFieldSet(lsfp[2].intValue, event);
	KUnsafeFieldSet(lsfp[3].asObject, (kObject *)cbArg->arg);

	KStackSetFuncAll(lsfp, KLIB Knull(kctx, returnType), 0/*UL*/, cbArg->kcb, 3);
	KStackCall(lsfp);
	END_UnusedStack();
}


/* ======================================================================== */
// Cevent class

static void Cevent_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct Cevent *ev = (struct Cevent *) o;
	ev->event = NULL;
}

static void Cevent_Free(KonohaContext *kctx, kObject *o)
{
	struct Cevent *ev = (struct Cevent *) o;

	if(ev->event != NULL) {
		event_free(ev->event);
		ev->event = NULL;
	}
}

//static void Cevent_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
//{
//	struct Cevent *ev = (struct Cevent *) o;
//}

//## Cevent Cevent.new(Cevent_base event_base, int evd, int event, EventCBArg cbArg);
static KMETHOD Cevent_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct Cevent *ev = (struct Cevent *) sfp[0].asObject;
	struct Cevent_base *cEvent_base = (struct Cevent_base *)sfp[1].asObject;
	evutil_socket_t evd = (evutil_socket_t)sfp[2].intValue;
	short event = (short)(sfp[3].intValue & 0xffff);
	kEventCBArg *cbArg = (kEventCBArg *)sfp[4].asObject;	//deliver callback method

	ev->event = event_new(cEvent_base->event_base, evd, event, Cevent_callback_1st, cbArg);
	KReturn(ev);
}

//## Cevent Cevent.getEvfd();
// get event file descriptor
static KMETHOD Cevent_getEvfd(KonohaContext *kctx, KonohaStack *sfp)
{
	struct Cevent *ev = (struct Cevent *) sfp[0].asObject;
	KReturnUnboxValue(ev->event->ev_fd);
}

//## Cevent Cevent.getEvents();
// get event category field
static KMETHOD Cevent_getEvents(KonohaContext *kctx, KonohaStack *sfp)
{
	struct Cevent *ev = (struct Cevent *) sfp[0].asObject;
	KReturnUnboxValue(ev->event->ev_events);
}


/* ======================================================================== */
// Cbufferevent class

static void Cbufferevent_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct Cbufferevent *bev = (struct Cbufferevent *) o;
	bev->bev = NULL;
}

static void Cbufferevent_Free(KonohaContext *kctx, kObject *o)
{
	struct Cbufferevent *bev = (struct Cbufferevent *) o;

	if (bev->bev != NULL) {
		bufferevent_free(bev->bev);
		bev->bev = NULL;
	}
}

//static void Cbufferevent_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
//{
//	struct Cbufferevent *bev = (struct Cbufferevent *) o;
//}

//## Cbufferevent Cbufferevent.new(Cevent_base event_base, int evd, int option);
static KMETHOD Cbufferevent_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct Cbufferevent *bev = (struct Cbufferevent *)sfp[0].asObject;
	struct Cevent_base *cev_base = (struct Cevent_base *)sfp[1].asObject;
	evutil_socket_t evd = (evutil_socket_t)sfp[2].intValue;
	int options = sfp[3].intValue;

	bev->bev = bufferevent_socket_new(cev_base->event_base, evd, options);
	KReturn(bev);
}

static void Cbev_dataCB_dispatcher(enum e_buffereventCB cat, struct bufferevent *bev, void *arg)
{
	kBuffereventCBArg *cbArg = arg;
	KonohaContext *kctx = cbArg->kctx;

	BEGIN_UnusedStack(lsfp);
	KClass *returnType = kMethod_GetReturnType(cbArg->kcb[cat]->method);
	KUnsafeFieldSet(lsfp[0].asObject, K_NULL);
	KUnsafeFieldSet(lsfp[1].asObject, (kObject *)cbArg->cbev);
	KUnsafeFieldSet(lsfp[2].asObject, (kObject *)cbArg->arg);

	KStackSetFuncAll(lsfp, KLIB Knull(kctx, returnType), 0/*UL*/, cbArg->kcb[cat], 2);
	KStackCall(lsfp);
	END_UnusedStack();
}

/*
 * Cbufferevent Class (*buffer_data_cb)() 1st stage callback from event_base_dispatch(), NEVER BE CALLED FROM OTHERS.
 */
static void Cbev_readCB_1st(struct bufferevent *bev, void *arg)
{ Cbev_dataCB_dispatcher(BEV_ReadCB, bev, arg); }

/*
 * Cbufferevent Class (*buffer_data_cb)() 1st stage callback from event_base_dispatch(), NEVER BE CALLED FROM OTHERS.
 */
static void Cbev_writeCB_1st(struct bufferevent *bev, void *arg)
{ Cbev_dataCB_dispatcher(BEV_WriteCB, bev, arg); }

/*
 * Cbufferevent Class (*buffer_event_cb)() 1st stage callback from event_base_dispatch(), NEVER BE CALLED FROM OTHERS.
 */
static void Cbev_eventCB_1st(struct bufferevent *bev, short what, void *arg)
{
	kBuffereventCBArg *cbArg = arg;
	KonohaContext *kctx = cbArg->kctx;

	BEGIN_UnusedStack(lsfp);
	KClass *returnType = kMethod_GetReturnType(cbArg->kcb[BEV_EventCB]->method);
	KUnsafeFieldSet(lsfp[0].asObject, K_NULL);
	KUnsafeFieldSet(lsfp[1].asObject, (kObject *)cbArg->cbev);
	KUnsafeFieldSet(lsfp[2].intValue, what);
	KUnsafeFieldSet(lsfp[3].asObject, (kObject *)cbArg->arg);
	KStackSetFuncAll(lsfp, KLIB Knull(kctx, returnType), 0/*UL*/, cbArg->kcb[BEV_EventCB], 3);
	KStackCall(lsfp);
	END_UnusedStack();
}


/* ======================================================================== */
// EventCBArg class

static void EventCBArg_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct EventCBArg *cbarg = (struct EventCBArg *) o;
	cbarg->kctx = NULL;
	KFieldInit(cbarg, cbarg->kcb, K_NULL);
	KFieldInit(cbarg, cbarg->arg, K_NULL);
}

static void EventCBArg_Free(KonohaContext *kctx, kObject *o)
{
	struct EventCBArg *cbarg = (struct EventCBArg *) o;

	cbarg->kctx = NULL;
	KFieldInit(cbarg, cbarg->kcb, K_NULL);
	KFieldInit(cbarg, cbarg->arg, K_NULL);
}

static void EventCBArg_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	struct EventCBArg *cba = (struct EventCBArg *) o;
	KRefTraceNullable(cba->kcb);
	KRefTraceNullable(cba->arg);
}

//## EventCBArg EventCBArg.new(Func[void, int, Object arg] cb, Object cbArg);
static KMETHOD EventCBArg_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct EventCBArg *cbarg = (struct EventCBArg *) sfp[0].asObject;
	kFunc *cb = sfp[1].asFunc;
	kObjectVar *cbArg = sfp[2].asObjectVar;	//deliver callback method

	cbarg->kctx = kctx;
	KFieldSet(cbarg, cbarg->kcb, cb);
	KFieldSet(cbarg, cbarg->arg, cbArg);
	KReturn(cbarg);
}


/* ======================================================================== */
// BuffereventCBArg class

static void BuffereventCBArg_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct BuffereventCBArg *bcbarg = (struct BuffereventCBArg *) o;
	bcbarg->kctx = NULL;
	enum e_buffereventCB i;
	for (i = BEV_ReadCB; i < NUM_BuffereventCB; i++){
		KFieldInit(bcbarg, bcbarg->kcb[i], K_NULL);
	}
	KFieldInit(bcbarg, bcbarg->cbev, K_NULL);
	KFieldInit(bcbarg, bcbarg->arg, K_NULL);
}

static void BuffereventCBArg_Free(KonohaContext *kctx, kObject *o)
{
	struct BuffereventCBArg *bcbarg = (struct BuffereventCBArg *) o;

	bcbarg->kctx = NULL;
	enum e_buffereventCB i;
	for (i = BEV_ReadCB; i < NUM_BuffereventCB; i++){
		KFieldInit(bcbarg, bcbarg->kcb[i], K_NULL);
	}
	KFieldInit(bcbarg, bcbarg->cbev, K_NULL);
	KFieldInit(bcbarg, bcbarg->arg, K_NULL);
}

static void BuffereventCBArg_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	struct BuffereventCBArg *bcbarg = (struct BuffereventCBArg *) o;
	enum e_buffereventCB i;
	for (i = BEV_ReadCB; i < NUM_BuffereventCB; i++){
		KRefTraceNullable(bcbarg->kcb[i]);
	}
	KRefTraceNullable(bcbarg->cbev);
	KRefTraceNullable(bcbarg->arg);
}

//## BuffereventCBArg BuffereventCBArg.new(Func[void, int, Object arg] cb, Object cbArg);
static KMETHOD BuffereventCBArg_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct BuffereventCBArg *bcbarg = (struct BuffereventCBArg *) sfp[0].asObject;
	kObjectVar *cbArg = sfp[4].asObjectVar;	//deliver callback method

	bcbarg->kctx = kctx;
	enum e_buffereventCB i;
	for (i = BEV_ReadCB; i < NUM_BuffereventCB; i++){
		kFunc *cb = sfp[i + 1].asFunc;
		KFieldSet(bcbarg, bcbarg->kcb[i], cb);
	}
	/*
	!!ATTENTION!!
	'bcbarg->cbev' will be set in Cbufferevent_setcb() function
	*/
	KFieldSet(bcbarg, bcbarg->arg, cbArg);
	KReturn(bcbarg);
}


/* ======================================================================== */
// TimeVal class

static void TimeVal_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct TimeVal *tv = (struct TimeVal *) o;
	tv->timeval.tv_sec = 0;
	tv->timeval.tv_usec = 0;
}

//## TimeVal TimeVal.new(int tv_sec, int tv_usec);
static KMETHOD TimeVal_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct TimeVal *tv = (struct TimeVal *) sfp[0].asObject;
	time_t sec = (time_t)sfp[1].intValue;
	suseconds_t usec = (suseconds_t)sfp[2].intValue;
	tv->timeval.tv_sec = sec;
	tv->timeval.tv_usec = usec;
	KReturn(tv);
}


// TODO ----- should be implement in posix.socket package
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
// System class
//## int System.evutil_make_socket_nonblocking(int fd);
static KMETHOD System_evutil_make_socket_nonblocking(KonohaContext *kctx, KonohaStack* sfp)
{
	evutil_socket_t evd = (evutil_socket_t)sfp[1].intValue;
	int ret = evutil_make_socket_nonblocking(evd);
	KReturnUnboxValue(ret);
}

//## int System.event_add(Cevent_base event, TimeVal tv);
static KMETHOD System_event_add(KonohaContext *kctx, KonohaStack* sfp)
{
	kCevent *kcev = (kCevent *)sfp[1].asObject;
	kTimeVal *tv = (kTimeVal *)sfp[2].asObject;
	int ret = event_add(kcev->event, (tv->timeval.tv_sec == 0 && tv->timeval.tv_usec == 0) ? NULL : &tv->timeval);
	KReturnUnboxValue(ret);
}

//## int System.event_del(Cevent event);
static KMETHOD System_event_del(KonohaContext *kctx, KonohaStack* sfp)
{
	kCevent *kcev = (kCevent *)sfp[1].asObject;
	int ret = event_del(kcev->event);
	KReturnUnboxValue(ret);
}

//## void System.bufferevent_setcb(Cbufferevent bev, BuffereventCBArg cbarg);
static KMETHOD System_bufferevent_setcb(KonohaContext *kctx, KonohaStack *sfp)
{
	kCbufferevent *bev = (kCbufferevent *)sfp[1].asObject;
	kBuffereventCBArg *cbArg = (kBuffereventCBArg *)sfp[2].asObject;

	KFieldSet(cbArg, cbArg->cbev, bev);
	bufferevent_setcb(bev->bev, Cbev_readCB_1st, Cbev_writeCB_1st, Cbev_eventCB_1st, cbArg);
	KReturnVoid();
}

//## int System.bufferevent_socket_connect(Cbufferevent bev, Sockaddr_in sa);
static KMETHOD System_bufferevent_socket_connect(KonohaContext *kctx, KonohaStack *sfp)
{
	kCbufferevent *bev = (kCbufferevent *)sfp[1].asObject;
	kSockaddr_in *sa = (kSockaddr_in *)sfp[2].asObject;
	int ret = bufferevent_socket_connect(bev->bev, (struct sockaddr *)&sa->sockaddr, sizeof sa->sockaddr);
	KReturnUnboxValue(ret);
}

//## int System.bufferevent_enable(Cbufferevent bev, int event);
static KMETHOD System_bufferevent_enable(KonohaContext *kctx, KonohaStack *sfp)
{
	kCbufferevent *bev = (kCbufferevent *)sfp[1].asObject;
	short event = (short)sfp[2].intValue;

	int ret = bufferevent_enable(bev->bev, event);
	KReturnUnboxValue(ret);
}
//## int System.evbuffer_add_printf(Cbufferevent bev, String msg);
static KMETHOD System_evbuffer_add_printf(KonohaContext *kctx, KonohaStack *sfp)
{
	kCbufferevent *bev = (kCbufferevent *)sfp[1].asObject;
	kString *msg = sfp[2].asString;

	int ret = evbuffer_add_printf(bufferevent_get_output(bev->bev), kString_text(msg));
	KReturnUnboxValue(ret);
}


/* ======================================================================== */

static kbool_t Cevent_base_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	/* Class Definition */
	/* If you want to create Generic class like Array<T>, see konoha.map package */
	// Cevent_base
	KDEFINE_CLASS defCevent_base = {0};
	SETSTRUCTNAME(defCevent_base, Cevent_base);
	defCevent_base.cflag     = KClassFlag_Final;	//must be final in C
	defCevent_base.init      = Cevent_base_Init;
	defCevent_base.free      = Cevent_base_Free;
	KClass *Cevent_baseClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defCevent_base, trace);

	// Cevent
	KDEFINE_CLASS defCevent = {0};
	SETSTRUCTNAME(defCevent, Cevent);
	defCevent.cflag     = KClassFlag_Final;
	defCevent.init      = Cevent_Init;
//	defCevent.reftrace  = Cevent_Reftrace;
	defCevent.free      = Cevent_Free;
	KClass *CeventClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defCevent, trace);

	// Cbufferevent
	KDEFINE_CLASS defCbufferevent = {0};
	SETSTRUCTNAME(defCbufferevent, Cbufferevent);
	defCbufferevent.cflag     = KClassFlag_Final;
	defCbufferevent.init      = Cbufferevent_Init;
//	defCbufferevent.reftrace  = Cbufferevent_Reftrace;
	defCbufferevent.free      = Cbufferevent_Free;
	KClass *CbuffereventClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defCbufferevent, trace);

	// EventCBArg
	KDEFINE_CLASS defEventCBArg = {0};
	SETSTRUCTNAME(defEventCBArg, EventCBArg);
	defEventCBArg.cflag     = KClassFlag_Final;
	defEventCBArg.init      = EventCBArg_Init;
	defEventCBArg.reftrace  = EventCBArg_Reftrace;
	defEventCBArg.free      = EventCBArg_Free;
	KClass *EventCBArgClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defEventCBArg, trace);

	// BuffereventCBArg
	KDEFINE_CLASS defBuffereventCBArg = {0};
	SETSTRUCTNAME(defBuffereventCBArg, BuffereventCBArg);
	defBuffereventCBArg.cflag     = KClassFlag_Final;
	defBuffereventCBArg.init      = BuffereventCBArg_Init;
	defBuffereventCBArg.reftrace  = BuffereventCBArg_Reftrace;
	defBuffereventCBArg.free      = BuffereventCBArg_Free;
	KClass *BuffereventCBArgClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defBuffereventCBArg, trace);

	// TimeVal
	KDEFINE_CLASS defTimeVal = {0};
	SETSTRUCTNAME(defTimeVal, TimeVal);
	defTimeVal.cflag     = KClassFlag_Final;
	defTimeVal.init      = TimeVal_Init;
	KClass *TimeValClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defTimeVal, trace);

	// Sockaddr_in
	KDEFINE_CLASS defSockaddr_in = {0};
	SETSTRUCTNAME(defSockaddr_in, Sockaddr_in);
	defSockaddr_in.cflag     = KClassFlag_Final;
	defSockaddr_in.init      = Sockaddr_in_Init;
	KClass *Sockaddr_inClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defSockaddr_in, trace);


	/* You can define methods with the following procedures. */
	int KType_Cevent_base = Cevent_baseClass->typeId;
	int KType_Cevent = CeventClass->typeId;
	int KType_Cbufferevent = CbuffereventClass->typeId;
	int KType_EventCBArg = EventCBArgClass->typeId;
	int KType_BuffereventCBArg = BuffereventCBArgClass->typeId;
	int KType_TimeVal = TimeValClass->typeId;
	int KType_Sockaddr_in = Sockaddr_inClass->typeId;

	/* define Generics parameter for callback method */
	//eventCB_p
	kparamtype_t eventCB_p[] = {{KType_Int, 0}, {KType_Int, 0}, {KType_Object, 0}};
	KClass *CeventCBfunc = KLIB KClass_Generics(kctx, KClass_Func, KType_void, 3, eventCB_p);
	int KType_CeventCBfunc = CeventCBfunc->typeId;
	//bev_dataCB_p
	kparamtype_t bev_dataCB_p[] = {{KType_Cbufferevent, 0}, {KType_Object, 0}};
	KClass *Cbev_dataCBfunc = KLIB KClass_Generics(kctx, KClass_Func, KType_void, 2, bev_dataCB_p);
	int KType_Cbev_dataCBfunc = Cbev_dataCBfunc->typeId;
	//bev_eventCB_p
	kparamtype_t bev_eventCB_p[] = {{KType_Cbufferevent, 0}, {KType_Int, 0}, {KType_Object, 0}};
	KClass *Cbev_eventCBfunc = KLIB KClass_Generics(kctx, KClass_Func, KType_void, 3, bev_eventCB_p);
	int KType_Cbev_eventCBfunc = Cbev_eventCBfunc->typeId;

	KDEFINE_METHOD MethodData[] = {
		// System class
		_Public|_Static|_Const|_Im, _F(System_evutil_make_socket_nonblocking), KType_Int, KType_System, KMethodName_("evutil_make_socket_nonblocking"), 1, KType_Int, KFieldName_("fd"),
		_Public|_Static|_Const|_Im, _F(System_event_add), KType_Int, KType_System, KMethodName_("event_add"), 2, KType_Cevent, KFieldName_("Cevent"), KType_TimeVal, KFieldName_("timeval"),
		_Public|_Static|_Const|_Im, _F(System_event_del), KType_Int, KType_System, KMethodName_("event_del"), 1, KType_Cevent, KFieldName_("Cevent"),
		_Public|_Static|_Const|_Im, _F(System_bufferevent_setcb), KType_void, KType_System, KMethodName_("bufferevent_setcb"), 2, KType_Cbufferevent, KFieldName_("cbev"), KType_BuffereventCBArg, KFieldName_("BuffereventCBArg"),
		_Public|_Static|_Const|_Im, _F(System_bufferevent_socket_connect), KType_Int, KType_System, KMethodName_("bufferevent_socket_connect"), 2, KType_Cbufferevent, KFieldName_("cbev"), KType_Sockaddr_in, KFieldName_("sockaddr"),
		_Public|_Static|_Const|_Im, _F(System_bufferevent_enable), KType_Int, KType_System, KMethodName_("bufferevent_enable"), 2, KType_Cbufferevent, KFieldName_("cbev"), KType_Int, KFieldName_("event"),
		_Public|_Static|_Const|_Im, _F(System_evbuffer_add_printf), KType_Int, KType_System, KMethodName_("evbuffer_add_printf"), 2, KType_Cbufferevent, KFieldName_("cbev"), KType_String, KFieldName_("message"),

		// Cevent_base
		_Public, _F(Cevent_base_new), KType_Cevent_base, KType_Cevent_base, KMethodName_("new"), 0,
		_Public, _F(Cevent_base_event_dispatch), KType_Int, KType_Cevent_base, KMethodName_("event_dispatch"), 0,

		// Cevent
		_Public, _F(Cevent_new), KType_Cevent, KType_Cevent, KMethodName_("new"), 4, KType_Cevent_base, KFieldName_("Cevent_base"), KType_Int, KFieldName_("evd"), KType_Int, KFieldName_("event"), KType_EventCBArg, KFieldName_("CBarg"),
		_Public, _F(Cevent_getEvfd), KType_Int, KType_Cevent, KMethodName_("getEvfd"), 0, 
		_Public, _F(Cevent_getEvents), KType_Int, KType_Cevent, KMethodName_("getEvents"), 0, 

		// Cbufferevent
		_Public, _F(Cbufferevent_new), KType_Cbufferevent, KType_Cbufferevent, KMethodName_("new"), 3, KType_Cevent_base, KFieldName_("Cevent_base"), KType_Int, KFieldName_("evd"), KType_Int, KFieldName_("options"),

		// EventCBArg
		_Public, _F(EventCBArg_new), KType_EventCBArg, KType_EventCBArg, KMethodName_("new"), 2, KType_CeventCBfunc, KFieldName_("konoha_CB"), KType_Object, KFieldName_("CBarg"),

		// BuffereventCBArg
		_Public, _F(BuffereventCBArg_new), KType_BuffereventCBArg, KType_BuffereventCBArg, KMethodName_("new"), 4, KType_Cbev_dataCBfunc, KFieldName_("readCB"), KType_Cbev_dataCBfunc, KFieldName_("writeCB"), KType_Cbev_eventCBfunc, KFieldName_("eventCB"), KType_Object, KFieldName_("CBarg"),

		// TimeVal
		_Public, _F(TimeVal_new), KType_TimeVal, KType_TimeVal, KMethodName_("new"), 2, KType_Int, KFieldName_("tv_sec"), KType_Int, KFieldName_("tv_usec"),

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

		//TODO add other constants

		{} /* <= sentinel */
	};

	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(IntData), trace);

	return true;
}

static kbool_t Cevent_base_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *Libevent_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "libevent2", "0.1");
	d.PackupNameSpace	= Cevent_base_PackupNameSpace;
	d.ExportNameSpace	= Cevent_base_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
