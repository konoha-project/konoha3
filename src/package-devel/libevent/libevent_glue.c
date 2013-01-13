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
} kCEvent_base;

static void CEvent_base_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct CEvent_base *ev = (struct CEvent_base *) o;
	ev->event_base = NULL;
}

static void CEvent_base_Free(KonohaContext *kctx, kObject *o)
{
	struct CEvent_base *ev = (struct CEvent_base *) o;

	if (ev->event_base != NULL) {
		event_base_free(ev->event_base);
		ev->event_base = NULL;
	}
}

//## CEvent_base CEvent_base.new();
static KMETHOD CEvent_base_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kCEvent_base *ev = (kCEvent_base *)sfp[0].asObject;
	ev->event_base = event_base_new();
	KReturn(ev);
}

//## CEvent_base CEvent_base.dispatch();
static KMETHOD CEvent_base_dispatch(KonohaContext *kctx, KonohaStack *sfp)
{
	kCEvent_base *ev = (kCEvent_base *)sfp[0].asObject;
	int ret = event_base_dispatch(ev->event_base);
	KReturnUnboxValue(ret);
}


/* ======================================================================== */
// CEvent class
typedef struct CEvent {
	kObjectHeader h;
	struct event *event;
} kCEvent;

static void CEvent_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct CEvent *ev = (struct CEvent *) o;
	ev->event = NULL;
}

static void CEvent_Free(KonohaContext *kctx, kObject *o)
{
	struct CEvent *ev = (struct CEvent *) o;

	if (ev->event != NULL) {
		event_free(ev->event);
		ev->event = NULL;
	}
}

//## CEvent CEvent.new(CEvent_base event_base, int fd, int what, );
static KMETHOD CEvent_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct CEvent *ev = (struct CEvent *) sfp[0].asObject;
	struct CEvent_base *libevent = (struct CEvent_base *)sfp[1].asObject;
	evutil_socket_t fd = (evutil_socket_t)sfp[2].intValue;
	kint_t what = sfp[3].intValue;
	event_callback_fn cb = (event_callback_fn)sfp[4].asFunc;	//TODO from eventListener package
	void *arg = (void *)sfp[5].asObject;
	ev->event = event_new(libevent->event_base, fd, (short)what, cb, arg);
	KReturn(ev);
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
	KDEFINE_CLASS defCEvent_base = {0};
	SETSTRUCTNAME(defCEvent_base, CEvent_base);
	//defCEvent_base.cflag     = KClassFlag_Final;
	defCEvent_base.init      = CEvent_base_Init;
	defCEvent_base.free      = CEvent_base_Free;
	KClass *CEvent_baseClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defCEvent_base, trace);

	KDEFINE_CLASS defCEvent = {0};
	SETSTRUCTNAME(defCEvent, CEvent);
	defCEvent.cflag     = KClassFlag_Final;
	defCEvent.init      = CEvent_Init;
	defCEvent.free      = CEvent_Free;
	KClass *CEventClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defCEvent, trace);


	/* You can define methods with the following procedures. */
	int KType_CEvent_base = CEvent_baseClass->typeId;
	int KType_CEvent = CEventClass->typeId;

	KDEFINE_METHOD MethodData[] = {
		_Public, _F(CEvent_base_new), KType_CEvent_base, KType_CEvent_base, KMethodName_("new"), 0,
		_Public, _F(CEvent_base_dispatch), KType_CEvent_base, KType_CEvent_base, KMethodName_("dispatch"), 0,
		_Public|_Static|_Const|_Im, _F(System_event_add), KType_int, KType_System, KMethodName_("event_add"), 2, KType_Object, KFieldName_("CEvent"), KType_Object, KFieldName_("timeval"),	//TODO: param type should be "KType_CEvent" "KType_Date"
		_Public|_Static|_Const|_Im, _F(System_event_del), KType_int, KType_System, KMethodName_("event_del"), 1, KType_Object, KFieldName_("CEvent"),	//TODO: param type should be "KType_CEvent"

		_Public, _F(CEvent_new), KType_CEvent, KType_CEvent, KMethodName_("new"), 0,


#ifdef	CUTCUT
		_Public, _F(Person_say), KType_String, KType_Person, KMethodName_("say"), 0,
#endif
		DEND, /* <= sentinel */
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

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
