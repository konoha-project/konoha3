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
typedef struct Event_base {
	kObjectHeader h;
	struct event_base *event_base;
} kEvent_base;

static void Event_base_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct Event_base *ev = (struct Event_base *) o;
	ev->event_base = NULL;
}

static void Event_base_Free(KonohaContext *kctx, kObject *o)
{
	struct Event_base *ev = (struct Event_base *) o;

	if (ev->event_base != NULL) {
		event_base_free(ev->event_base);
		ev->event_base = NULL;
	}
}

//## Event_base Event_base.new();
static KMETHOD Event_base_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kEvent_base *ev = (kEvent_base *)sfp[0].asObject;
	ev->event_base = event_base_new();
	KReturn(ev);
}

//## Event_base Event_base.dispatch();
static KMETHOD Event_base_dispatch(KonohaContext *kctx, KonohaStack *sfp)
{
	kEvent_base *ev = (kEvent_base *)sfp[0].asObject;
	int ret = event_base_dispatch(ev->event_base);
	KReturnUnboxValue(ret);
}


/* ======================================================================== */
// Event class
typedef struct Event {
	kObjectHeader h;
	struct event *event;
} kEvent;

static void Event_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct Event *ev = (struct Event *) o;
	ev->event = NULL;
}

static void Event_Free(KonohaContext *kctx, kObject *o)
{
	struct Event *ev = (struct Event *) o;

	if (ev->event != NULL) {
		event_free(ev->event);
		ev->event = NULL;
	}
}

//## Event Event.new();
static KMETHOD Event_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct Event *ev = (struct Event *) sfp[0].asObject;
	struct Event_base *libevent = (struct Event_base *)sfp[1].asObject;
	evutil_socket_t fd = (evutil_socket_t)sfp[2].intValue;
	kint_t what = sfp[3].intValue;
	event_callback_fn cb = (event_callback_fn)sfp[4].asFunc;
	void *arg = (void *)sfp[5].asObject;
	ev->event = event_new(libevent->event_base, fd, (short)what, cb, arg);
	KReturn(ev);
}


/* ======================================================================== */
// System class
//## int System.event_add(Event_base_event event, Date tv);
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

	kEvent *ev = (kEvent *)sfp[1].asObject;
	kDate *date = (kDate *)sfp[2].asObject;
	int ret = event_add(ev->event, &date->tv);
	KReturnUnboxValue(ret);
}

//## int System.event_del(Event event);
KMETHOD System_event_del(KonohaContext *kctx, KonohaStack* sfp)
{
	kEvent *ev = (kEvent *)sfp[1].asObject;
	int ret = event_del(ev->event);
	KReturnUnboxValue(ret);
}


/* ======================================================================== */

static kbool_t Event_base_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	/* Class Definition */
	/* If you want to create Generic class like Array<T>, see konoha.map package */
	KDEFINE_CLASS defEvent_base = {0};
	SETSTRUCTNAME(defEvent_base, Event_base);
	//defEvent_base.cflag     = KClassFlag_Final;
	defEvent_base.init      = Event_base_Init;
	defEvent_base.free      = Event_base_Free;
	KClass *Event_baseClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defEvent_base, trace);

	KDEFINE_CLASS defEvent = {0};
	SETSTRUCTNAME(defEvent, Event);
	defEvent.cflag     = KClassFlag_Final;
	defEvent.init      = Event_Init;
	defEvent.free      = Event_Free;
	KClass *EventClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defEvent, trace);


	/* You can define methods with the following procedures. */
	int KType_Event_base = Event_baseClass->typeId;
	int KType_Event = EventClass->typeId;

	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Event_base_new), KType_Event_base, KType_Event_base, KMethodName_("new"), 0,
		_Public, _F(Event_base_dispatch), KType_Event_base, KType_Event_base, KMethodName_("dispatch"), 0,
		_Public|_Static|_Const|_Im, _F(System_event_add), KType_int, KType_System, KMethodName_("event_add"), 2, KType_Object, KFieldName_("Event"), KType_Object, KFieldName_("timeval"),	//TODO: param type should be "KType_Event" "KType_Date"
		_Public|_Static|_Const|_Im, _F(System_event_del), KType_int, KType_System, KMethodName_("event_del"), 1, KType_Object, KFieldName_("Event"),	//TODO: param type should be "KType_Event"

		_Public, _F(Event_new), KType_Event, KType_Event, KMethodName_("new"), 0,


#ifdef	CUTCUT
		_Public, _F(Person_say), KType_String, KType_Person, KMethodName_("say"), 0,
#endif
		DEND, /* <= sentinel */
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	return true;
}

static kbool_t Event_base_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *libevent_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "libevent2", "0.1");
	d.PackupNameSpace    = Event_base_PackupNameSpace;
	d.ExportNameSpace   = Event_base_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
