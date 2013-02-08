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

#define USE_KEYWORD_LIST 1
#include <konoha3/konoha.h>
#include <konoha3/klib.h>
#include <konoha3/local.h>
#include <konoha3/sugar.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "import/klibexec.h"
#include "import/datatype.h"
#include "import/methods.h"

// -------------------------------------------------------------------------

static void InitKonoha(void)
{
	static int isInit = 0;
	if(isInit == 0) {
		isInit = 1;
	}
}

/* ------------------------------------------------------------------------ */
/* stack */

static void KRuntimeContext_Init(KonohaContext *kctx, KonohaContextVar *ctx, size_t stacksize)
{
	size_t i;
	KRuntimeContextVar *base = (KRuntimeContextVar *)KCalloc_UNTRACE(sizeof(KRuntimeContextVar), 1);
	base->stacksize = stacksize;
	base->stack = (KonohaStack *)KCalloc_UNTRACE(sizeof(KonohaStack), stacksize);
	assert(stacksize>64);
	base->stack_uplimit = base->stack + (stacksize - 64);
	for(i = 0; i < stacksize; i++) {
		KUnsafeFieldInit(base->stack[i].asObject, K_NULL);
	}
	KUnsafeFieldInit(base->ContextConstList, new_(Array, 8, OnField));
	KUnsafeFieldInit(base->ThrownException, (kException *)K_NULL);
	base->gcStack = new_(Array, K_PAGESIZE/sizeof(void *), base->ContextConstList);
	KLIB KArray_Init(kctx, &base->cwb, K_PAGESIZE * 4);
	ctx->esp = base->stack;
	ctx->stack = base;
}

static void KRuntimeContext_Reftrace(KonohaContext *kctx, KonohaContextVar *ctx, KObjectVisitor *visitor)
{
	KonohaStack *sp = ctx->stack->stack;
	while(sp < ctx->esp) {
		KRefTrace(sp[0].asObject);
		sp++;
	}
	KRefTrace(ctx->stack->ContextConstList);
	KRefTrace(ctx->stack->ThrownException);
}

static void KRuntimeContext_Free(KonohaContext *kctx, KonohaContextVar *ctx)
{
	if(ctx->stack->evaljmpbuf != NULL) {
		KFree(ctx->stack->evaljmpbuf, sizeof(jmpbuf_i));
	}
	KLIB KArray_Free(kctx, &ctx->stack->cwb);
	KFree(ctx->stack->stack, sizeof(KonohaStack) * ctx->stack->stacksize);
	KFree(ctx->stack, sizeof(KRuntimeContextVar));
}

static kbool_t KRuntime_SetModule(KonohaContext *kctx, int x, KRuntimeModule *d, KTraceInfo *trace)
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

static void KonohaContext_Free(KonohaContext *kctx, KonohaContextVar *ctx);
static void ReftraceAll(KonohaContext *kctx, KObjectVisitor *visitor);
KonohaContext *KonohaFactory_CreateKonoha(KonohaFactory *factory);
int            Konoha_Destroy(KonohaContext *kctx);

static KonohaContextVar* new_KonohaContext(KonohaContext *kctx, const PlatformApi *platApi)
{
	KonohaContextVar *newctx;
	static volatile size_t ctxid_counter = 0;
	ctxid_counter++;
	if(kctx == NULL) {  // NULL means first one
		KonohaLibVar *klib = (KonohaLibVar *)calloc(sizeof(KonohaLib) + sizeof(KonohaContextVar), 1);
		klib_Init(klib);
		klib->KRuntime_SetModule  = KRuntime_SetModule;
		klib->KonohaContext_Init = new_KonohaContext;
		klib->KonohaContext_Free = KonohaContext_Free;
		klib->ReftraceAll = ReftraceAll;
		klib->KonohaFactory_CreateKonoha = KonohaFactory_CreateKonoha;
		klib->Konoha_Destroy = Konoha_Destroy;
		newctx = (KonohaContextVar *)(klib + 1);
		newctx->klib = (KonohaLib *)klib;
		newctx->platApi = platApi;
		kctx = (KonohaContext *)newctx;
		newctx->modshare = (KRuntimeModule**)calloc(sizeof(KRuntimeModule *), KRuntimeModule_MAXSIZE);
		newctx->modlocal = (KContextModule**)calloc(sizeof(KContextModule *), KRuntimeModule_MAXSIZE);
		DBG_ASSERT(PLATAPI GCModule.InitGcContext != NULL);
		PLATAPI GCModule.InitGcContext(newctx);
		PLATAPI JsonModule.InitJsonContext(newctx);
		KRuntime_Init(kctx, newctx);
	}
	else {   // others take ctx as its parent
		newctx = (KonohaContextVar *)KCalloc_UNTRACE(sizeof(KonohaContextVar), 1);
		newctx->klib = kctx->klib;
		newctx->platApi = kctx->platApi;
		newctx->share = kctx->share;
		newctx->modshare = kctx->modshare;
		newctx->modlocal = (KContextModule**)KCalloc_UNTRACE(sizeof(KContextModule *), KRuntimeModule_MAXSIZE);
		PLATAPI GCModule.InitGcContext(kctx);
		PLATAPI JsonModule.InitJsonContext(newctx);
	}
	KRuntimeContext_Init(kctx, newctx, platApi->stacksize);
	if(IS_RootKonohaContext(newctx)) {
		MODSUGAR_Init(kctx, newctx);
		LoadDefaultMethod(kctx, KNULL(NameSpace));
		LoadDefaultSugarMethod(kctx, KNULL(NameSpace));
	}
	else {
		int i;
		for(i = 0; i < KRuntimeModule_MAXSIZE; i++) {
			KRuntimeModule *mod = newctx->modshare[i];
			if(mod == NULL) continue;
			if(mod->setupModuleContext != NULL) {
				mod->setupModuleContext((KonohaContext *)newctx, mod, true);
			}
		}
	}
	return newctx;
}

static void KonohaContext_Reftrace(KonohaContext *kctx, KonohaContextVar *ctx, KObjectVisitor *visitor)
{
	size_t i;
	if(IS_RootKonohaContext(kctx)) {
		KRuntime_Reftrace(kctx, ctx, visitor);
	}
	KRuntimeContext_Reftrace(kctx, ctx, visitor);
	for(i = 0; i < KRuntimeModule_MAXSIZE; i++) {
		KContextModule *p = ctx->modlocal[i];
		if(p != NULL && p->reftrace != NULL) {
			p->reftrace(kctx, p, visitor);
		}
	}
}

static void ReftraceAll(KonohaContext *kctx, KObjectVisitor *visitor)
{
	KonohaContext_Reftrace(kctx, (KonohaContextVar *)kctx, visitor);
}

static void KonohaContext_Free(KonohaContext *kctx, KonohaContextVar *ctx)
{
	size_t i;
	for(i = 1; i < KRuntimeModule_MAXSIZE; i++) {   // 0 is LOGGER, free lately
		KContextModule *p = ctx->modlocal[i];
		if(p != NULL && p->free != NULL) {
			p->free(ctx, p);
		}
	}
	KRuntimeContext_Free(kctx, ctx);
	if(IS_RootKonohaContext(ctx)){  // share
		PLATAPI ExecutionEngineModule.DeleteExecutionEngine(ctx);
		KonohaLibVar *kklib = (KonohaLibVar *)ctx - 1;
		for(i = 0; i < KRuntimeModule_MAXSIZE; i++) {
			KRuntimeModule *p = ctx->modshare[i];
			if(p == NULL) continue;
			if(p->allocSize > 0) {
				KFree(p, p->allocSize);
			}
			else if(p->freeModule != NULL) {
				p->freeModule(kctx, p);
			}
		}
		PLATAPI GCModule.DeleteGcContext(ctx);
		PLATAPI JsonModule.DeleteJsonContext(ctx);
		KRuntime_Free(kctx, ctx);
		free(kctx->modlocal);
		free(kctx->modshare);
		free(kklib/*, sizeof(KonohaLib) + sizeof(KonohaContextVar)*/);
	}
	else {
		PLATAPI GCModule.DeleteGcContext(ctx);
		PLATAPI JsonModule.DeleteJsonContext(ctx);
		KFree(ctx->modlocal, sizeof(KContextModule *) * KRuntimeModule_MAXSIZE);
		KFree(ctx, sizeof(KonohaContextVar));
	}
}

/* ------------------------------------------------------------------------ */
/* konoha api */

KonohaContext* konoha_open(const PlatformApi *platform)
{
	assert(0);  // obsolate
	InitKonoha();
	return (KonohaContext *)new_KonohaContext(NULL, platform);
}

void konoha_close(KonohaContext* konoha)
{
	assert(0);
	KonohaContext_Free(konoha, (KonohaContextVar *)konoha);
}

// -------------------------------------------------------------------------
// Default Platform Module API

static void DefaultEventHandler(KonohaContext *kctx, void *args)
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
		factory->LoggerModule.syslog_i(ErrTag, "failed to load platform module: %s\n", name);
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

static void KonohaFactory_syslog_i(int priority, const char *message, ...)
{
	/*FIXME(ide)
	 * If we cannot load default modules at KonohaFactory_LoadPlatformModule and
	 * we emit log info with facotry->syslog_i. */
	//abort();
}

static void KonohaFactory_Check(KonohaFactory *factory)
{
	if(factory->LoggerModule.LoggerInfo == NULL) {
		factory->LoggerModule.TraceDataLog = DefaultTraceLog;  // for safety
		factory->LoggerModule.syslog_i     = KonohaFactory_syslog_i;
	}
	if(factory->ExecutionEngineModule.ExecutionEngineInfo == NULL) {
		const char *mod = factory->getenv_i("KONOHA_VM");
		if(mod == NULL) mod = "MiniVM";
		KonohaFactory_LoadPlatformModule(factory, mod, ReleaseModule);
	}
	if(factory->GCModule.GCInfo == NULL) {
		const char *mod = factory->getenv_i("KONOHA_GC");
		if(mod == NULL) mod = "BitmapGenGC";  // default
		KonohaFactory_LoadPlatformModule(factory, mod, ReleaseModule);
	}
	if(factory->I18NModule.I18NInfo == NULL) {
		const char *mod = factory->getenv_i("KONOHA_I18N");
		if(mod == NULL) mod = "IConv";        // default
		KonohaFactory_LoadPlatformModule(factory, mod, ReleaseModule);
	}
	if(factory->JsonModule.JsonDataInfo == NULL) {
		const char *mod = factory->getenv_i("KONOHA_JSON");
		if(mod == NULL) mod = "Json";
		KonohaFactory_LoadPlatformModule(factory, mod, ReleaseModule);
	}
	if(factory->EventModule.EventInfo == NULL) {
		factory->EventModule.StartEventHandler = DefaultEventHandler;
		factory->EventModule.StopEventHandler  = DefaultEventHandler;
		factory->EventModule.EnterEventContext = DefaultEventHandler;
		factory->EventModule.ExitEventContext  = DefaultEventHandler;
		factory->EventModule.EmitEvent         = DefaultEmitEvent;
		factory->EventModule.DispatchEvent     = DefaultDispatchEvent;
		factory->EventModule.WaitEvent         = NULL;  // check NULL
	}
	if(factory->DiagnosisModule.DiagnosisInfo == NULL) {
		factory->DiagnosisModule.CheckStaticRisk          = CheckStaticRisk;
		factory->DiagnosisModule.CheckDynamicRisk         = CheckDynamicRisk;
		factory->DiagnosisModule.DiagnosisSystemError     = DiagnosisSystemError;
		factory->DiagnosisModule.DiagnosisSoftwareProcess = DiagnosisSoftwareProcess;
		factory->DiagnosisModule.DiagnosisSystemResource  = DiagnosisSystemResource;
		factory->DiagnosisModule.DiagnosisFileSystem      = DiagnosisFileSystem;
		factory->DiagnosisModule.DiagnosisNetworking      = DiagnosisNetworking;
		factory->DiagnosisModule.DiagnosisCheckSoftwareTestIsPass = DiagnosisCheckSoftwareTestIsPass;
	}
	if(factory->LoggerModule.syslog_i == KonohaFactory_syslog_i) {
		factory->LoggerModule.syslog_i = NULL;
	}
}

KonohaContext* KonohaFactory_CreateKonoha(KonohaFactory *factory)
{
	KonohaFactory *platapi = (KonohaFactory *)factory->malloc_i(sizeof(KonohaFactory));
	KonohaFactory_Check(factory);
	memcpy(platapi, factory, sizeof(KonohaFactory));
	InitKonoha();
	return (KonohaContext *)new_KonohaContext(NULL, (PlatformApi *)platapi);
}

int Konoha_Destroy(KonohaContext *kctx)
{
	KonohaFactory *platapi = (KonohaFactory *)kctx->platApi;
	int exitStatus = platapi->exitStatus;
	KonohaContext_Free(kctx, (KonohaContextVar *)kctx);
	platapi->free_i(platapi);
	return exitStatus;
}

#ifdef __cplusplus
}
#endif
