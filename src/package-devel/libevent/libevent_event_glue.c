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
#include "libevent_glue.h"

#ifdef __cplusplus
extern "C" {
#endif

static void Libevent_event_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct Libevent_event *ev = (struct Libevent_event *) o;
	ev->event = NULL;
}

static void Libevent_event_Free(KonohaContext *kctx, kObject *o)
{
	struct Libevent_event *ev = (struct Libevent_event *) o;

	if (ev->event != NULL) {
		event_free(ev->event);
		ev->event = NULL;
	}
}

//## Libevent_event Libevent_event.new();
static KMETHOD Libevent_event_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct Libevent_event *ev = (struct Libevent_event *) sfp[0].asObject;
	struct Libevent *libevent = (struct Libevent *)sfp[1].asObject;
	evutil_socket_t fd = (evutil_socket_t)sfp[2].intValue;
	kint_t what = sfp[3].intValue;
	event_callback_fn cb = (event_callback_fn)sfp[4].asFunc;
	void *arg = (void *)sfp[5].asObject;
	ev->event = event_new(libevent->event_base, fd, (short)what, cb, arg);
	KReturn(ev);
}
static kbool_t Libevent_event_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	/* Class Definition */
	/* If you want to create Generic class like Array<T>, see konoha.map package */
	KDEFINE_CLASS defLibevent_event = {0};
	SETSTRUCTNAME(defLibevent_event, Libevent_event);
	defLibevent_event.cflag     = KClassFlag_Final;
	defLibevent_event.init      = Libevent_event_Init;
	defLibevent_event.free      = Libevent_event_Free;
	KClass *Libevent_eventClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defLibevent_event, trace);

	/* You can define methods with the following procedures. */
	int KType_Libevent_event = Libevent_eventClass->typeId;

	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Libevent_event_new), KType_Libevent_event, KType_Libevent_event, KKMethodName_("new"), 0,
		DEND, /* <= sentinel */
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	KDEFINE_INT_CONST IntData[] = {
		{KDefineConstInt(EV_TIMEOUT)},
		{KDefineConstInt(EV_READ)},
		{KDefineConstInt(EV_WRITE)},
		{KDefineConstInt(EV_SIGNAL)},
		{KDefineConstInt(EV_PERSIST)},
		{KDefineConstInt(EV_ET)},
		{} /* <= sentinel */
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(IntData), false/*isOverride*/, trace);
	return true;
}

static kbool_t Libevent_event_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *libevent_event_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "libevent2", "0.1");
	d.PackupNameSpace    = Libevent_event_PackupNameSpace;
	d.ExportNameSpace   = Libevent_event_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
