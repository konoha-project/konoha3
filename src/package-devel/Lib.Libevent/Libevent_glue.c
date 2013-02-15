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

typedef struct Cbufferevent_socket {
	kObjectHeader h;
	struct bufferevent *bufev;
} kCbufferevent_socket;

typedef struct EventCallBackArg {	//callback-method argument wrapper
	kObjectHeader h;
	KonohaContext *kctx;
	kFunc *kcb;		// konoha call back method	
	kObject *arg;
} kEventCallBackArg;

#include <sys/time.h>
typedef struct TimeVal {
	kObjectHeader h;
	struct timeval timeval;
} kTimeVal;


/* ======================================================================== */
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
	/*
	TODO
	I don't know why, it is "ev->event_base != null" at here.
	Is it correct execution?
	*/
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
 * 1st stage callback from event_base_dispatch(), NEVER BE CALLED FROM OTHERS.
 */
static void callback_1st(evutil_socket_t evd, short event, void *arg) {
	kEventCallBackArg *cbArg = arg;
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

//## Cevent Cevent.new(Cevent_base event_base, int evd, int event, EventCallBackArg cbArg);
static KMETHOD Cevent_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct Cevent *ev = (struct Cevent *) sfp[0].asObject;
	struct Cevent_base *cEvent_base = (struct Cevent_base *)sfp[1].asObject;
	evutil_socket_t evd = (evutil_socket_t)sfp[2].intValue;
	short event = (short)(sfp[3].intValue & 0xffff);
	kEventCallBackArg *cbArg = (kEventCallBackArg *)sfp[4].asObject;	//deliver callback method

	ev->event = event_new(cEvent_base->event_base, evd, event, callback_1st, cbArg);
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

#ifdef CUTCUT
/* ======================================================================== */
// Cbufferevent_socket class

static void Cbufferevent_socket_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct Cbufferevent_socket *bev = (struct Cbufferevent_socket *) o;
	bev->bufev = NULL;
}

static void Cbufferevent_socket_Free(KonohaContext *kctx, kObject *o)
{
	struct Cbufferevent_socket *bev = (struct Cbufferevent_socket *) o;

	if(bev->bufev != NULL) {
		bufferevent_free(bev->bufev);
		bev->bufev = NULL;
	}
}

static void Cbufferevent_socket_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	struct Cbufferevent_socket *bev = (struct Cbufferevent_socket *) o;
	KRefTraceNullable(bev->key);
}

//## Cbufferevent_socket Cbufferevent_socket.new(Cevent_base event_base, String key, int evd, int event, Func[int, int, Object arg] cb, EventCallBackArg cbArg);
static KMETHOD Cbufferevent_socket_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct Cbufferevent_socket *bev = (struct Cbufferevent_socket *)sfp[0].asObject;
	struct Cevent_base *cEvent_base = (struct Cevent_base *)sfp[1].asObject;
	kString *key = (kString *)sfp[2].asString;
	evutil_socket_t evd = (evutil_socket_t)sfp[3].intValue;
	short event = (short)(sfp[4].intValue & 0xffff);
	kEventCallBackArg *cbArg = (kEventCallBackArg *)sfp[5].asObject;	//deliver callback method

	bev->event = event_new(cEvent_base->event_base, evd, event, callback_1st, cbArg);
	KFieldSet(bev, bev->key, key);
	KReturn(ev);
}
#endif

/* ======================================================================== */
// EventCallBackArg class

static void EventCallBackArg_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct EventCallBackArg *cbarg = (struct EventCallBackArg *) o;
	cbarg->kctx = NULL;
	KFieldInit(cbarg, cbarg->kcb, K_NULL);
	KFieldInit(cbarg, cbarg->arg, K_NULL);
}

static void EventCallBackArg_Free(KonohaContext *kctx, kObject *o)
{
	struct EventCallBackArg *cbarg = (struct EventCallBackArg *) o;

	cbarg->kctx = NULL;
	KFieldInit(cbarg, cbarg->kcb, K_NULL);
	KFieldInit(cbarg, cbarg->arg, K_NULL);
}

static void EventCallBackArg_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	struct EventCallBackArg *cba = (struct EventCallBackArg *) o;
	KRefTraceNullable(cba->kcb);
	KRefTraceNullable(cba->arg);
}

//## EventCallBackArg EventCallBackArg.new(Func[int, int, Object arg] cb, Object cbArg);
static KMETHOD EventCallBackArg_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct EventCallBackArg *cbarg = (struct EventCallBackArg *) sfp[0].asObject;
	kFunc *cb = sfp[1].asFunc;
	kObjectVar *cbArg = sfp[2].asObjectVar;	//deliver callback method

	cbarg->kctx = kctx;
	KFieldSet(cbarg, cbarg->kcb, cb);
	KFieldSet(cbarg, cbarg->arg, cbArg);
	KReturn(cbarg);
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

#ifdef CUTCUT
	// Cbufferevent_socket
	KDEFINE_CLASS defCbufferevent_socket = {0};
	SETSTRUCTNAME(defCbufferevent_socket, Cbufferevent_socket);
	defCbufferevent_socket.cflag     = KClassFlag_Final;
	defCbufferevent_socket.init      = Cbufferevent_socket_Init;
	defCbufferevent_socket.reftrace  = Cbufferevent_socket_Reftrace;
	defCbufferevent_socket.free      = Cbufferevent_socket_Free;
	KClass *Cbufferevent_socketClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defCbufferevent_socket, trace);
#endif

	// EventCallBackArg
	KDEFINE_CLASS defEventCallBackArg = {0};
	SETSTRUCTNAME(defEventCallBackArg, EventCallBackArg);
	defEventCallBackArg.cflag     = KClassFlag_Final;
	defEventCallBackArg.init      = EventCallBackArg_Init;
	defEventCallBackArg.reftrace  = EventCallBackArg_Reftrace;
	defEventCallBackArg.free      = EventCallBackArg_Free;
	KClass *EventCallBackArgClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defEventCallBackArg, trace);

	// TimeVal
	KDEFINE_CLASS defTimeVal = {0};
	SETSTRUCTNAME(defTimeVal, TimeVal);
	defTimeVal.cflag     = KClassFlag_Final;
	defTimeVal.init      = TimeVal_Init;
	KClass *TimeValClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defTimeVal, trace);


	/* You can define methods with the following procedures. */
	int KType_Cevent_base = Cevent_baseClass->typeId;
	int KType_Cevent = CeventClass->typeId;
#ifdef CUTCUT
	int KType_Cbufferevent_socket = Cbufferevent_socketClass->typeId;
#endif
	int KType_EventCallBackArg = EventCallBackArgClass->typeId;
	int KType_TimeVal = TimeValClass->typeId;

	/* define Generics parameter for callback method */
	kparamtype_t p[] = {{KType_Int}, {KType_Int}, {KType_Object}};
	KClass *CeventCBfunc = KLIB KClass_Generics(kctx, KClass_Func, KType_void, 3, p);
	int KType_CeventCBfunc = CeventCBfunc->typeId;

	KDEFINE_METHOD MethodData[] = {
		// System class
		_Public|_Static|_Const|_Im, _F(System_evutil_make_socket_nonblocking), KType_Int, KType_System, KMethodName_("evutil_make_socket_nonblocking"), 1, KType_Int, KFieldName_("fd"),
		_Public|_Static|_Const|_Im, _F(System_event_add), KType_Int, KType_System, KMethodName_("event_add"), 2, KType_Cevent, KFieldName_("Cevent"), KType_TimeVal, KFieldName_("timeval"),
		_Public|_Static|_Const|_Im, _F(System_event_del), KType_Int, KType_System, KMethodName_("event_del"), 1, KType_Cevent, KFieldName_("Cevent"),

		// Cevent_base
		_Public, _F(Cevent_base_new), KType_Cevent_base, KType_Cevent_base, KMethodName_("new"), 0,
		_Public, _F(Cevent_base_event_dispatch), KType_Int, KType_Cevent_base, KMethodName_("event_dispatch"), 0,

		// Cevent
		_Public, _F(Cevent_new), KType_Cevent, KType_Cevent, KMethodName_("new"), 4, KType_Cevent_base, KFieldName_("Event_base"), KType_Int, KFieldName_("evd"), KType_Int, KFieldName_("event"), KType_EventCallBackArg, KFieldName_("CBarg"),
		_Public, _F(Cevent_getEvfd), KType_Int, KType_Cevent, KMethodName_("getEvfd"), 0, 
		_Public, _F(Cevent_getEvents), KType_Int, KType_Cevent, KMethodName_("getEvents"), 0, 

#ifdef CUTCUT
		// Cbufferevent_socket
		_Public, _F(Cbufferevent_socket_new), KType_Cbufferevent_socket, KType_Cbufferevent_socket, KMethodName_("new"), 5, KType_Cbufferevent_socket_base, KFieldName_("Event_base"), KType_String, KFieldName_("key"), KType_Int, KFieldName_("evd"), KType_Int, KFieldName_("event"), KType_EventCallBackArg, KFieldName_("CBarg"),
#endif

		// EventCallBackArg
		_Public, _F(EventCallBackArg_new), KType_EventCallBackArg, KType_EventCallBackArg, KMethodName_("new"), 2, KType_CeventCBfunc, KFieldName_("konoha_CB"), KType_Object, KFieldName_("CBarg"),

		// TimeVal
		_Public, _F(TimeVal_new), KType_TimeVal, KType_TimeVal, KMethodName_("new"), 2, KType_Int, KFieldName_("tvSec"), KType_Int, KFieldName_("tvUsec"),

		DEND, /* <= sentinel */
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);


	KDEFINE_INT_CONST IntData[] = {
		//for event_new()
		{KDefineConstInt(EV_TIMEOUT)},
		{KDefineConstInt(EV_READ)},
		{KDefineConstInt(EV_WRITE)},
		{KDefineConstInt(EV_SIGNAL)},
		{KDefineConstInt(EV_PERSIST)},
		{KDefineConstInt(EV_ET)},

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
