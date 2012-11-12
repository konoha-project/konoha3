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

#include "minikonoha/minikonoha.h"
#include "minikonoha/klib.h"
#include "minikonoha/local.h"
#include "minikonoha/sugar.h"

#include "protomap.h"
#include "klibexec.h"
#include "datatype.h"
#include "methods.h"

#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------------------------------------
// util stack

static void konoha_init(void)
{
	static int isInit = 0;
	if(isInit == 0) {
		isInit = 1;
	}
}

/* ------------------------------------------------------------------------ */
/* stack */

static void KonohaStackRuntime_init(KonohaContext *kctx, KonohaContextVar *ctx, size_t stacksize)
{
	size_t i;
	KonohaStackRuntimeVar *base = (KonohaStackRuntimeVar *)KCalloc_UNTRACE(sizeof(KonohaStackRuntimeVar), 1);
	base->stacksize = stacksize;
	base->stack = (KonohaStack *)KCalloc_UNTRACE(sizeof(KonohaStack), stacksize);
	assert(stacksize>64);
	base->stack_uplimit = base->stack + (stacksize - 64);
	for(i = 0; i < stacksize; i++) {
		KUnsafeFieldInit(base->stack[i].asObject, K_NULL);
	}
	KUnsafeFieldInit(base->ContextConstList, new_(Array, 8, OnField));
	KUnsafeFieldInit(base->OptionalErrorInfo, TS_EMPTY);
	base->gcstack_OnContextConstList = new_(Array, K_PAGESIZE/sizeof(void *), base->ContextConstList);
	KLIB Karray_init(kctx, &base->cwb, K_PAGESIZE * 4);
	ctx->esp = base->stack;
	ctx->stack = base;
}

static void KonohaStackRuntime_reftrace(KonohaContext *kctx, KonohaContextVar *ctx, KObjectVisitor *visitor)
{
	KonohaStack *sp = ctx->stack->stack;
	while(sp < ctx->esp) {
		KREFTRACEv(sp[0].asObject);
		sp++;
	}
	KREFTRACEv(ctx->stack->ContextConstList);
	KREFTRACEv(ctx->stack->OptionalErrorInfo);
}

static void KonohaStackRuntime_free(KonohaContext *kctx, KonohaContextVar *ctx)
{
	if(ctx->stack->evaljmpbuf != NULL) {
		KFree(ctx->stack->evaljmpbuf, sizeof(jmpbuf_i));
	}
	KLIB Karray_free(kctx, &ctx->stack->cwb);
	KFree(ctx->stack->stack, sizeof(KonohaStack) * ctx->stack->stacksize);
	KFree(ctx->stack, sizeof(KonohaStackRuntimeVar));
}

static kbool_t KonohaRuntime_setModule(KonohaContext *kctx, int x, KonohaModule *d, KTraceInfo *trace)
{
	if(kctx->modshare[x] != NULL) {
		KLIB ReportScriptMessage(kctx, trace, ErrTag, "module %s already registered", kctx->modshare[x]->name);
		return false;
	}
	kctx->modshare[x] = d;
	return true;
}

/* ------------------------------------------------------------------------ */
/* [kcontext] */

static void KonohaContext_free(KonohaContext *kctx, KonohaContextVar *ctx);
static void ReftraceAll(KonohaContext *kctx, KObjectVisitor *visitor);
KonohaContext*    KonohaFactory_CreateKonoha(KonohaFactory *factory);
int               Konoha_Destroy(KonohaContext *kctx);

static KonohaContextVar* new_KonohaContext(KonohaContext *kctx, const PlatformApi *platApi)
{
	KonohaContextVar *newctx;
	static volatile size_t ctxid_counter = 0;
	ctxid_counter++;
	if(kctx == NULL) {  // NULL means first one
		KonohaLibVar *klib = (KonohaLibVar *)calloc(sizeof(KonohaLib) + sizeof(KonohaContextVar), 1);
		klib_init(klib);
		klib->KonohaRuntime_setModule    = KonohaRuntime_setModule;
		klib->KonohaContext_init = new_KonohaContext;
		klib->KonohaContext_free = KonohaContext_free;
		klib->ReftraceAll = ReftraceAll;
		klib->KonohaFactory_CreateKonoha = KonohaFactory_CreateKonoha;
		klib->Konoha_Destroy = Konoha_Destroy;
		newctx = (KonohaContextVar *)(klib + 1);
		newctx->klib = (KonohaLib *)klib;
		newctx->platApi = platApi;
		kctx = (KonohaContext *)newctx;
		newctx->modshare = (KonohaModule**)calloc(sizeof(KonohaModule *), KonohaModule_MAXSIZE);
		newctx->modlocal = (KonohaModuleContext**)calloc(sizeof(KonohaModuleContext *), KonohaModule_MAXSIZE);
		DBG_ASSERT(PLATAPI InitGcContext != NULL);
		PLATAPI InitGcContext(newctx);
		KonohaRuntime_init(kctx, newctx);
	}
	else {   // others take ctx as its parent
		newctx = (KonohaContextVar *)KCalloc_UNTRACE(sizeof(KonohaContextVar), 1);
		newctx->klib = kctx->klib;
		newctx->platApi = kctx->platApi;
		newctx->share = kctx->share;
		newctx->modshare = kctx->modshare;
		newctx->modlocal = (KonohaModuleContext**)KCalloc_UNTRACE(sizeof(KonohaModuleContext *), KonohaModule_MAXSIZE);
		PLATAPI InitGcContext(kctx);
	}
	KonohaStackRuntime_init(kctx, newctx, platApi->stacksize);
	if(IS_RootKonohaContext(newctx)) {
		MODCODE_init(kctx, newctx);
		MODSUGAR_init(kctx, newctx);
		LoadDefaultMethod(kctx, KNULL(NameSpace));
		LoadDefaultSugarMethod(kctx, KNULL(NameSpace));
	}
	else {
		int i;
		for(i = 0; i < KonohaModule_MAXSIZE; i++) {
			KonohaModule *mod = newctx->modshare[i];
			if(mod == NULL) continue;
			if(mod->setupModuleContext != NULL) {
				mod->setupModuleContext((KonohaContext *)newctx, mod, true);
			}
		}
	}
	return newctx;
}

static void KonohaContext_reftrace(KonohaContext *kctx, KonohaContextVar *ctx, KObjectVisitor *visitor)
{
	size_t i;
	if(IS_RootKonohaContext(kctx)) {
		KonohaRuntime_reftrace(kctx, ctx, visitor);
	}
	KonohaStackRuntime_reftrace(kctx, ctx, visitor);
	for(i = 0; i < KonohaModule_MAXSIZE; i++) {
		KonohaModuleContext *p = ctx->modlocal[i];
		if(p != NULL && p->reftrace != NULL) {
			p->reftrace(kctx, p, visitor);
		}
	}
}

static void ReftraceAll(KonohaContext *kctx, KObjectVisitor *visitor)
{
	KonohaContext_reftrace(kctx, (KonohaContextVar *)kctx, visitor);
}

static void KonohaContext_free(KonohaContext *kctx, KonohaContextVar *ctx)
{
	size_t i;
	for(i = 1; i < KonohaModule_MAXSIZE; i++) {   // 0 is LOGGER, free lately
		KonohaModuleContext *p = ctx->modlocal[i];
		if(p != NULL && p->free != NULL) {
			p->free(ctx, p);
		}
	}
	KonohaStackRuntime_free(kctx, ctx);
	if(IS_RootKonohaContext(ctx)){  // share
		PLATAPI DeleteVirtualMachine(ctx);
		KonohaLibVar *kklib = (KonohaLibVar *)ctx - 1;
		for(i = 0; i < KonohaModule_MAXSIZE; i++) {
			KonohaModule *p = ctx->modshare[i];
			if(p == NULL) continue;
			if(p->allocSize > 0) {
				KFree(p, p->allocSize);
			}
			else if(p->freeModule != NULL) {
				p->freeModule(kctx, p);
			}
		}
		PLATAPI DeleteGcContext(ctx);
		KonohaRuntime_free(kctx, ctx);
		//MODGC_check_malloced_size(kctx);
		free(kctx->modlocal);
		free(kctx->modshare);
		free(kklib/*, sizeof(KonohaLib) + sizeof(KonohaContextVar)*/);
	}
	else {
		PLATAPI DeleteGcContext(ctx);
		KFree(ctx->modlocal, sizeof(KonohaModuleContext *) * KonohaModule_MAXSIZE);
		KFree(ctx, sizeof(KonohaContextVar));
	}
}

/* ------------------------------------------------------------------------ */
/* konoha api */

KonohaContext* konoha_open(const PlatformApi *platform)
{
	assert(0);  // obsolate
	konoha_init();
	return (KonohaContext *)new_KonohaContext(NULL, platform);
}

void konoha_close(KonohaContext* konoha)
{
	assert(0);
	KonohaContext_free(konoha, (KonohaContextVar *)konoha);
}

// -------------------------------------------------------------------------
// Default Platform Module API

static void DefaultEventHandler(KonohaContext *kctx)
{
}
static kbool_t DefaultEmitEvent(KonohaContext *kctx, struct JsonBuf *json, KTraceInfo *trace)
{
	return false;
}
static void DefaultDispatchEvent(KonohaContext *kctx, kbool_t (*consume)(KonohaContext *kctx, struct JsonBuf *, KTraceInfo *), KTraceInfo *trace)
{
}

static void DefaultTraceLog(KonohaContext *kctx, KTraceInfo *trace, int logkey, logconf_t *logconf, ...)
{

}

// -------------------------------------------------------------------------
/* Diagnosis */

static kbool_t CheckStaticRisk(KonohaContext *kctx, const char *keyword, size_t keylen, kfileline_t uline)
{
	return true; // OK
}

static void CheckDynamicRisk(KonohaContext *kctx, const char *keyword, size_t keylen, KTraceInfo *trace)
{
}

static int DiagnosisSoftwareProcess(KonohaContext *kctx, kfileline_t uline, KTraceInfo *trace)
{
	return 0;
}

static int DiagnosisSystemResource(KonohaContext *kctx, KTraceInfo *trace)
{
	return 0;
}

static int DiagnosisFileSystem(KonohaContext *kctx, const char *path, size_t pathlen, KTraceInfo *trace)
{
	return 0; // unknown
}

static int DiagnosisNetworking(KonohaContext *kctx, const char *path, size_t pathlen, int port, KTraceInfo *trace)
{
	return 0; // unknown
}

static int DiagnosisSystemError(KonohaContext *kctx, int userFault)
{
	return userFault | SoftwareFault | SystemFault;
}

static kbool_t DiagnosisCheckSoftwareTestIsPass(KonohaContext *kctx, const char *filename, int line)
{
	return false;
}
// -------------------------------------------------------------------------
/* Konoha C API */

kbool_t KonohaFactory_LoadPlatformModule(KonohaFactory *factory, const char *name, ModuleType option)
{
	if(!factory->LoadPlatformModule(factory, name, option)) {
		factory->syslog_i(ErrTag, "failed to load platform module: %s\n", name);
		factory->printf_i("failed to load platform module: %s\n", name);
		return true;
	}
	return false;
}

void KonohaFactory_SetDefaultFactory(KonohaFactory *factory, void (*SetPlatformApi)(KonohaFactory *), int argc, char **argv)
{
	int i;
	SetPlatformApi(factory);
	for(i = 0; i < argc; i++) {
		const char *t = argv[i];
		if(t[0] == '-' && t[1] == 'M') {   /* -MName */
			const char *moduleName = t + 2;
			if(moduleName[0] == 0 && i+1 < argc) {  /* -M Name */
				i++;
				moduleName = argv[i];
			}
			KonohaFactory_LoadPlatformModule(factory, moduleName, ReleaseModule);
		}
	}
}

void KonohaFactory_CheckVirtualMachine(KonohaFactory *factory);  // For compatibility

static void KonohaFactory_Check(KonohaFactory *factory)
{
	if(factory->LoggerInfo == NULL) {
		factory->TraceDataLog = DefaultTraceLog;  // for safety
	}
	if(factory->VirtualMachineInfo == NULL) {
		const char *mod = factory->getenv_i("KONOHA_VM");
		if(mod == NULL) mod = "MiniVM";
		KonohaFactory_LoadPlatformModule(factory, mod, ReleaseModule);
	}
	if(factory->GCInfo == NULL) {
		const char *mod = factory->getenv_i("KONOHA_GC");
		if(mod == NULL) mod = "BitmapGenGC";  // default
		KonohaFactory_LoadPlatformModule(factory, mod, ReleaseModule);
	}
	if(factory->I18NInfo == NULL) {
		const char *mod = factory->getenv_i("KONOHA_I18N");
		if(mod == NULL) mod = "IConv";        // default
		KonohaFactory_LoadPlatformModule(factory, mod, ReleaseModule);
	}
	if(factory->JsonDataInfo == NULL) {
		const char *mod = factory->getenv_i("KONOHA_JSON");
		if(mod == NULL) mod = "Json";
		KonohaFactory_LoadPlatformModule(factory, mod, ReleaseModule);
	}
	if(factory->EventInfo == NULL) {
		factory->StartEventHandler = DefaultEventHandler;
		factory->StopEventHandler  = DefaultEventHandler;
		factory->EnterEventContext = DefaultEventHandler;
		factory->ExitEventContext  = DefaultEventHandler;
		factory->EmitEvent         = DefaultEmitEvent;
		factory->DispatchEvent     = DefaultDispatchEvent;
		factory->WaitEvent         = NULL;  // check NULL
	}
	if(factory->DiagnosisInfo == NULL) {
		factory->CheckStaticRisk          = CheckStaticRisk;
		factory->CheckDynamicRisk         = CheckDynamicRisk;
		factory->DiagnosisSystemError     = DiagnosisSystemError;
		factory->DiagnosisSoftwareProcess = DiagnosisSoftwareProcess;
		factory->DiagnosisSystemResource  = DiagnosisSystemResource;
		factory->DiagnosisFileSystem      = DiagnosisFileSystem;
		factory->DiagnosisNetworking      = DiagnosisNetworking;
		factory->DiagnosisCheckSoftwareTestIsPass = DiagnosisCheckSoftwareTestIsPass;
	}
}

KonohaContext* KonohaFactory_CreateKonoha(KonohaFactory *factory)
{
	KonohaFactory *platapi = (KonohaFactory *)factory->malloc_i(sizeof(KonohaFactory));
	KonohaFactory_Check(factory);
	memcpy(platapi, factory, sizeof(KonohaFactory));
	konoha_init();
	return (KonohaContext *)new_KonohaContext(NULL, (PlatformApi *)platapi);
}

int Konoha_Destroy(KonohaContext *kctx)
{
	KonohaFactory *platapi = (KonohaFactory *)kctx->platApi;
	int exitStatus = platapi->exitStatus;
	KonohaContext_free(kctx, (KonohaContextVar *)kctx);
	platapi->free_i(platapi);
	return exitStatus;
}

#ifdef __cplusplus
}
#endif
