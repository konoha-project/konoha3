/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <konoha3/konoha.h>

// -------------------------------------------------------------------------
/* EventModule, EventContext */

struct JsonBuf {
	int64_t josn_i;  // it consumes 64bits (based on Ide's impl)
};

// -------------------------------------------------------------------------


static void (*SignalStartEventHandler)(KonohaContext *kctx);
static void (*SignalStopEventHandler)(KonohaContext *kctx);
static void (*SignalEnterEventContext)(KonohaContext *kctx);
static void (*SignalExitEventContext)(KonohaContext *kctx);

static void StartEventHandler(KonohaContext *kctx)
{
	SignalStartEventHandler(kctx);
	// add here
}

static void StopEventHandler(KonohaContext *kctx)
{
	// add here
	SignalStopEventHandler(kctx);
}



static void EnterEventContext(KonohaContext *kctx)
{
	SignalEnterEventContext(kctx);
	// add here
}

static void ExitEventContext(KonohaContext *kctx)
{
	// add here
	SignalExitEventContext(kctx);
}

// -------------------------------------------------------------------------

kbool_t LoadSignalHttpEventModule(KonohaFactory *factory, ModuleType type)
{
	if(factory->LoadPlatformModule(factory, "Signal", type)) {
		static KModuleInfo ModuleInfo = {
			"SignalHttpEvent", "0.1", 0, "signal http",
		};
		factory->EventInfo            = &ModuleInfo;
		factory->eventContext = NULL;
		SignalStartEventHandler = factory->StartEventHandler;
		SignalStopEventHandler  = factory->StopEventHandler;
		SignalEnterEventContext = factory->EnterEventContext;
		SignalExitEventContext  = factory->ExitEventContext;
		factory->StartEventHandler = StartEventHandler;
		factory->StopEventHandler  = StopEventHandler;
		factory->EnterEventContext = EnterEventContext;
		factory->ExitEventContext  = ExitEventContext;
//		factory->EmitEvent         = EmitEvent;
//		factory->DispatchEvent     = DispatchEvent;
//		factory->WaitEvent         = WaitEvent;
		return true;
	}
	return false;
}


#ifdef __cplusplus
} /* extern "C" */
#endif
