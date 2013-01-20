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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/import/methoddecl.h>
#include <event2/event.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ======================================================================== */
// Event_base class
typedef struct CEvent_base {
	kObjectHeader h;
	struct event_base *event_base;
	KonohaContext *kctx;
} kCEvent_base;

typedef struct CEvent {
	kObjectHeader h;
	kString *key;
	struct event *event;
	kCEvent_base *evBase;
	kFunc *kcb;		// konoha call back method	
	kObject *arg;	// arg to be passed konoha callback-method
} kCEvent;

#include <sys/time.h>
typedef struct TimeVal {
	kObjectHeader h;
	struct timeval timeval;
} kTimeVal;

static void CEvent_base_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct CEvent_base *ev = (struct CEvent_base *) o;
	ev->event_base = NULL;
	ev->kctx = NULL;
}

static void CEvent_base_Free(KonohaContext *kctx, kObject *o)
{
	struct CEvent_base *ev = (struct CEvent_base *) o;
	if (ev->event_base != NULL) {
		event_base_free(ev->event_base);
		ev->event_base = NULL;
		ev->kctx = NULL;
	}
}

//## CEvent_base CEvent_base.new();
static KMETHOD CEvent_base_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kCEvent_base *ev = (kCEvent_base *)sfp[0].asObject;
	if (ev->event_base == NULL) {
		ev->event_base = event_base_new();
		ev->kctx = kctx;
	} else {
		// TODO: throw exception ?
	}
	KReturn(ev);
}

//## CEvent_base CEvent_base.event_dispatch();
static KMETHOD CEvent_base_event_dispatch(KonohaContext *kctx, KonohaStack *sfp)
{
	kCEvent_base *ev = (kCEvent_base *)sfp[0].asObject;
	int ret = event_base_dispatch(ev->event_base);
	KReturnUnboxValue(ret);
}

/*
 * 1st stage callback from event_base_dispatch(), NEVER BE CALLED FROM OTHERS.
 */
static void callback_1st(evutil_socket_t evd, short event, void *arg) {
	kCEvent *cbArg = arg;
	KonohaContext *kctx = cbArg->evBase->kctx;

	BEGIN_UnusedStack(lsfp);
	KClass *returnType = kMethod_GetReturnType(cbArg->kcb->method);
	KUnsafeFieldSet(lsfp[0].asObject, K_NULL);
	KUnsafeFieldSet(lsfp[1].asObject, cbArg->arg);

	KStackSetFuncAll(lsfp, KLIB Knull(kctx, returnType), 0/*UL*/, cbArg->kcb, 1);
	KStackCall(lsfp);
	END_UnusedStack();
}


/* ======================================================================== */
// CEvent class

static void CEvent_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct CEvent *ev = (struct CEvent *) o;
	ev->event = NULL;
	ev->evBase = NULL;
	ev->kcb = NULL;
	ev->arg = NULL;
}

static void CEvent_Free(KonohaContext *kctx, kObject *o)
{
	struct CEvent *ev = (struct CEvent *) o;

	if (ev->event != NULL) {
		event_free(ev->event);
		ev->event = NULL;
		ev->evBase = NULL;
		ev->kcb = NULL;
		ev->arg = NULL;
	}
}

//## CEvent CEvent.new(CEvent_base event_base, String key, int evd, int event, Func[void, Object cb);
static KMETHOD CEvent_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct CEvent *ev = (struct CEvent *) sfp[0].asObject;
	struct CEvent_base *cEvent_base = (struct CEvent_base *)sfp[1].asObject;
	kString *key = (kString *) sfp[2].asString;
	evutil_socket_t evd = (evutil_socket_t)sfp[3].intValue;
	short event = (short)(sfp[4].intValue & 0xffff);
	kFunc *konoha_cb = sfp[5].asFunc;

	printf("cEvent_base->event_base:%p, evd:%d, event:%#x, callback_1st:%p, ev:%p\n", cEvent_base->event_base, evd, event, callback_1st, ev);
	ev->event = event_new(cEvent_base->event_base, evd, event, callback_1st, ev);
	printf("ev->event = %p in CEvent_new\n", ev->event);
	ev->evBase = cEvent_base;
	ev->key = key;
	ev->kcb = konoha_cb;
	ev->arg = (kObject *)ev;
	//event_add() will call in EventBase.event_add() after return this method
	KReturn(ev);
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
//## int System.event_add(CEvent_base event, Date tv);
static KMETHOD System_event_add(KonohaContext *kctx, KonohaStack* sfp)
{
	kCEvent *kcev = (kCEvent *)sfp[1].asObject;
	kTimeVal *tv = (kTimeVal *)sfp[2].asObject;
	printf("kcev->event = %p in System_event_add\n", kcev->event);
	int ret = event_add(kcev->event, &tv->timeval);
	printf("System_event_add 2\n");
	KReturnUnboxValue(ret);
}

//## int System.event_del(CEvent event);
static KMETHOD System_event_del(KonohaContext *kctx, KonohaStack* sfp)
{
	kCEvent *kcev = (kCEvent *)sfp[1].asObject;
	int ret = event_del(kcev->event);
	KReturnUnboxValue(ret);
}


/* ======================================================================== */

static kbool_t CEvent_base_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	/* Class Definition */
	/* If you want to create Generic class like Array<T>, see konoha.map package */
	// CEvent_base
	KDEFINE_CLASS defCEvent_base = {0};
	SETSTRUCTNAME(defCEvent_base, CEvent_base);
	//defCEvent_base.cflag     = KClassFlag_Final;
	defCEvent_base.init      = CEvent_base_Init;
	defCEvent_base.free      = CEvent_base_Free;
	KClass *CEvent_baseClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defCEvent_base, trace);

	// CEvent
	KDEFINE_CLASS defCEvent = {0};
	SETSTRUCTNAME(defCEvent, CEvent);
	defCEvent.cflag     = KClassFlag_Final;
	defCEvent.init      = CEvent_Init;
	defCEvent.free      = CEvent_Free;
	KClass *CEventClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defCEvent, trace);

	// TimeVal
	KDEFINE_CLASS defTimeVal = {0};
	SETSTRUCTNAME(defTimeVal, TimeVal);
	defTimeVal.cflag     = KClassFlag_Final;
	defTimeVal.init      = TimeVal_Init;
	KClass *TimeValClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defTimeVal, trace);


	/* You can define methods with the following procedures. */
	int KType_CEvent_base = CEvent_baseClass->typeId;
	int KType_CEvent = CEventClass->typeId;
	int KType_TimeVal = TimeValClass->typeId;

	KDEFINE_METHOD MethodData[] = {
		// System class
		_Public|_Static|_Const|_Im, _F(System_event_add), KType_int, KType_System, KMethodName_("event_add"), 2, KType_Object, KFieldName_("CEvent"), KType_Object, KFieldName_("timeval"),	//TODO: param type should be "KType_CEvent" "KType_Date"
		_Public|_Static|_Const|_Im, _F(System_event_del), KType_int, KType_System, KMethodName_("event_del"), 1, KType_Object, KFieldName_("CEvent"),	//TODO: param type should be "KType_CEvent"

		// CEvent_base
		_Public, _F(CEvent_base_new), KType_CEvent_base, KType_CEvent_base, KMethodName_("new"), 0,
		_Public, _F(CEvent_base_event_dispatch), KType_CEvent_base, KType_CEvent_base, KMethodName_("event_dispatch"), 0,

		// CEvent
		_Public, _F(CEvent_new), KType_CEvent, KType_CEvent, KMethodName_("new"), 5, KType_CEvent_base, KFieldName_("Event_base"), KType_String, KFieldName_("key"), KType_int, KFieldName_("evd"), KType_int, KFieldName_("event"), KType_Func, KFieldName_("konoha_CB"),

		// TimeVal
		_Public, _F(TimeVal_new), KType_TimeVal, KType_TimeVal, KMethodName_("new"), 2, KType_int, KFieldName_("tvSec"), KType_int, KFieldName_("tvUsec"),

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

	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(IntData), false/*isOverride*/, trace);

	return true;
}

static kbool_t CEvent_base_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *libevent_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "libevent2", "0.1");
	d.PackupNameSpace    = CEvent_base_PackupNameSpace;
	d.ExportNameSpace   = CEvent_base_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
