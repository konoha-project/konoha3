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
#include <minikonoha/import/methoddecl.h>
#include <event2/event.h>
#include "libevent_glue.h"

#ifdef __cplusplus
extern "C" {
#endif

static void Libevent_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct Libevent *ev = (struct Libevent *) o;
	ev->event_base = NULL;
}

static void Libevent_Free(KonohaContext *kctx, kObject *o)
{
	struct Libevent *ev = (struct Libevent *) o;

	if (ev->event_base != NULL) {
		event_base_free(ev->event_base);
		ev->event_base = NULL;
	}
}

//## Libevent Libevent.new();
static KMETHOD Libevent_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kLibevent *ev = (kLibevent *)sfp[0].asObject;
	ev->event_base = event_base_new();
	KReturn(ev);
}

/* ======================================================================== */
//## int System.event_add(Libevent_event event, Date tv);
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

	kLibevent_event *ev = (kLibevent_event *)sfp[1].asObject;
	kDate *date = (kDate *)sfp[2].asObject;
	int ret = event_add(ev->event, &date->tv);
	KReturnUnboxValue(ret);
}

static kbool_t Libevent_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	/* Class Definition */
	/* If you want to create Generic class like Array<T>, see konoha.map package */
	KDEFINE_CLASS defLibevent = {0};
	SETSTRUCTNAME(defLibevent, Libevent);
	defLibevent.cflag     = KClassFlag_Final;
	defLibevent.init      = Libevent_Init;
	defLibevent.free      = Libevent_Free;
	KClass *LibeventClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defLibevent, trace);

	/* You can define methods with the following procedures. */
	int KType_Libevent = LibeventClass->typeId;

	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Libevent_new), KType_Libevent, KType_Libevent, KKMethodName_("new"), 0,
		_Public|_Static|_Const|_Im, _F(System_event_add), KType_int, KType_System, KKMethodName_("event_add"), 2, KType_Object, KFieldName_("Libevent_event"), KType_Object, KFieldName_("timeval"),	//TODO: param type should be "KType_Libevent_event" "KType_Date"



#ifdef	CUTCUT
		_Public, _F(Person_say), KType_String, KType_Person, KKMethodName_("say"), 0,
#endif
		DEND, /* <= sentinel */
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

#ifdef	CUTCUT
	/* You can define constant variable with the following procedures. */
	KDEFINE_INT_CONST IntData[] = {
		{"NARUTO_AGE", KType_int, 18},
		{} /* <= sentinel */
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(IntData), false/*isOverride*/, trace);
#endif
	return true;
}

static kbool_t Libevent_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *libevent_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "libevent2", "0.1");
	d.PackupNameSpace    = Libevent_PackupNameSpace;
	d.ExportNameSpace   = Libevent_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif



#if CUTCUT
//## String Person.say();
static KMETHOD Libevent_event_add(KonohaContext *kctx, KonohaStack *sfp)
{
	struct Libevent *ev = (struct Libevent *) sfp[0].asObject;

	kString *name = p->name;
	/* When you want to operate with a raw string, please use kString_text() macro
	 * to acquire the pointer of a raw string. */
	const char *text = kString_text(name);
	char *buf = (char *)alloca(16 + kString_size(name));
	sprintf(buf, "hello , %s!", text);
	KReturn(KLIB new_kString(kctx, OnStack, buf, strlen(buf), StringPolicy_TEXT));
}
#endif
