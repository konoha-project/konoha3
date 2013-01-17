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

//## CEvent_base CEvent_base.dispatch();
static KMETHOD CEvent_base_dispatch(KonohaContext *kctx, KonohaStack *sfp)
{
	kCEvent_base *ev = (kCEvent_base *)sfp[0].asObject;
	int ret = event_base_dispatch(ev->event_base);
	KReturnUnboxValue(ret);
}

/*
 * 1st stage callback from event_base_dispatch(), NEVER BE CALLED FROM OTHERS.
 */
static bool callback_1st(evutil_socket_t evd, short event, void *arg) {
	CEvent *cbArg = arg;

	ev_base.eventMap.get(


	//CEvent ev = event_Map.get(key) as CEvent;


	BEGIN_UnusedStack(lsfp);
	KUnsafeFieldSet(lsfp[0].asObject, K_NULL);
	KUnsafeFieldSet(lsfp[1].asObject, (kObject *)ev);
	KStackSetFuncAll(lsfp, KLIB Knull(kctx, returnType), 0/*UL*/, KonohaContext_getEventContext(kctx)->enqFuncNULL, 1);
	KStackCall(lsfp);
	END_UnusedStack();






	--- copy from src/parser/import/eval.h ---

	BEGIN_UnusedStack(lsfp);
	KRuntimeContextVar *runtime = kctx->stack;
	if(runtime->evalty != KType_void) {
		KUnsafeFieldSet(lsfp[1].asObject, runtime->stack[runtime->evalidx].asObject);
		lsfp[1].intValue = runtime->stack[runtime->evalidx].intValue;
	}
	KStackSetMethodAll(lsfp, KLIB Knull(kctx, KClass_(rtype)), uline, mtd, 1);
	kstatus_t result = K_CONTINUE;
	if(KLIB KRuntime_tryCallMethod(kctx, lsfp)) {
		runtime->evalidx = ((lsfp + K_RTNIDX) - kctx->stack->stack);
		runtime->evalty = rtype;
	}
	else {
		runtime->evalty = KType_void;  // no value
		result = K_BREAK;        // message must be reported;
	}
	END_UnusedStack();
	return result;

	--- end of copy ---



}


/* ======================================================================== */
// CEvent class

typedef struct CEvent {
	kObjectHeader h;
	struct event *event;
	kEvent_base *evBase;
	kFunc *kcb;		// konoha call back method	
	kObject *arg;	// arg to be passed konoha callback-method
} kCEvent;

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

//## CEvent CEvent.new(CEvent_base event_base, String key, int evd, int event, Func[void, Object cb, TimeVal timout);
static KMETHOD CEvent_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct CEvent *ev = (struct CEvent *) sfp[0].asObject;
	struct CEvent_base *cEvent_base = (struct CEvent_base *)sfp[1].asObject;
	kString *key = (kString *) sfp[2].asString;
	evutil_socket_t evd = (evutil_socket_t)sfp[3].intValue;
	short event = (short)sfp[4].intValue;
	kFunc *konoha_cb = (kFunc *)sfp[5].asFunc;
	struct TimeVal timout = (struct TimeVal)sft[6].asObject;

	ev->event = event_new(cEvent_base->event_base, evd, event, callback_1st, this);
	ev->evBase = cEvent_base;
	ev->key = key;
	ev->arg = arg;
	//event_add() will call in EventBase.event_add() after return this method
	KReturn(ev);
}

/* ======================================================================== */
// TimeVal class

#include <sys/time.h>

typedef struct TimeVal {
	kObjectHeader h;
	struct timeval timeval;
} kTimeVal;

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
KMETHOD System_event_add(KonohaContext *kctx, KonohaStack* sfp)
{
#define Declare_kDate
#ifdef Declare_kDate	//TODO:It should be declared in headerfile in js4.date package.
	typedef struct kDateVar {
		kObjectHeader h;
		struct timeval tv;
	} kDate;
#endif	//Declare_kDate
#undef Declare_kDate

	kCEvent *ev = (kCEvent *)sfp[1].asObject;
	kDate *date = (kDate *)sfp[2].asObject;
	int ret = event_add(ev->event, &date->tv);
	KReturnUnboxValue(ret);
}

//## int System.event_del(CEvent event);
KMETHOD System_event_del(KonohaContext *kctx, KonohaStack* sfp)
{
	kCEvent *ev = (kCEvent *)sfp[1].asObject;
	int ret = event_del(ev->event);
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
		_Public, _F(CEvent_base_dispatch), KType_CEvent_base, KType_CEvent_base, KMethodName_("dispatch"), 0,

		// CEvent
		_Public, _F(CEvent_new), KType_CEvent, KType_CEvent, KMethodName_("new"), 5, KType_CEvent_base, KFieldName_("Event_base"), KType_int, KFieldName_("fd"), KType_int, KFieldName_("event"), KType_Func, KFieldName_("cb"), KType_int /*TODO: (void *) */, KFieldName_("arg"),

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
