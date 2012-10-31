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
#include "minikonoha/gc.h"

#ifdef K_USING_LOGPOOL
#include <logpool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "minikonoha/local.h"

#include "protomap.h"
#include "klibexec.h"
#include "datatype.h"
#include "methods.h"

// -------------------------------------------------------------------------
// util stack

static void konoha_init(void)
{
	static int isInit = 0;
	if(isInit == 0) {
		isInit = 1;
	}
}

static void knh_beginContext(KonohaContext *kctx, void **bottom)
{
	kctx->stack->cstack_bottom = bottom;
}

static void knh_endContext(KonohaContext *kctx)
{
	kctx->stack->cstack_bottom = NULL;
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
	base->visitor = kVisitor_KonohaVM;
	ctx->esp = base->stack;
	ctx->stack = base;
}

static void KonohaStackRuntime_reftrace(KonohaContext *kctx, KonohaContextVar *ctx, KObjectVisitor *visitor)
{
	KonohaStack *sp = ctx->stack->stack;
	BEGIN_REFTRACE((kctx->esp - sp) + 2);
	while(sp < ctx->esp) {
		KREFTRACEv(sp[0].asObject);
		sp++;
	}
	KREFTRACEv(ctx->stack->ContextConstList);
	KREFTRACEv(ctx->stack->OptionalErrorInfo);
	END_REFTRACE();
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
		KLIB ReportRuntimeMessage(kctx, trace, ErrTag, "module %s already registered", kctx->modshare[x]->name);
		return false;
	}
	kctx->modshare[x] = d;
	return true;
}

/* ------------------------------------------------------------------------ */
/* [kcontext] */

static void KonohaContext_free(KonohaContext *kctx, KonohaContextVar *ctx);

static KonohaContextVar* new_KonohaContext(KonohaContext *kctx, const PlatformApi *platApi)
{
	KonohaContextVar *newctx;
	static volatile size_t ctxid_counter = 0;
	ctxid_counter++;
	if(kctx == NULL) {  // NULL means first one
		KonohaLibVar *klib = (KonohaLibVar *)calloc(sizeof(KonohaLib) + sizeof(KonohaContextVar), 1);
		klib_init(klib);
		klib->KonohaRuntime_setModule    = KonohaRuntime_setModule;
		newctx = (KonohaContextVar *)(klib + 1);
		newctx->klib = (KonohaLib *)klib;
		newctx->platApi = platApi;
		((KonohaLibVar *)newctx->klib)->KonohaContext_init = new_KonohaContext;
		((KonohaLibVar *)newctx->klib)->KonohaContext_free = KonohaContext_free;
		kctx = (KonohaContext *)newctx;
		newctx->modshare = (KonohaModule**)calloc(sizeof(KonohaModule *), KonohaModule_MAXSIZE);
		newctx->modlocal = (KonohaModuleContext**)calloc(sizeof(KonohaModuleContext *), KonohaModule_MAXSIZE);

		MODGC_init(kctx, newctx);
		KonohaRuntime_init(kctx, newctx);
	}
	else {   // others take ctx as its parent
		newctx = (KonohaContextVar *)KCalloc_UNTRACE(sizeof(KonohaContextVar), 1);
		newctx->klib = kctx->klib;
		newctx->platApi = kctx->platApi;
		newctx->share = kctx->share;
		newctx->modshare = kctx->modshare;
		newctx->modlocal = (KonohaModuleContext**)KCalloc_UNTRACE(sizeof(KonohaModuleContext *), KonohaModule_MAXSIZE);
		MODGC_init(kctx, newctx);
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

void KonohaContext_reftraceAll(KonohaContext *kctx, KObjectVisitor *visitor)
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
		KLIB KdeleteGcContext(ctx->gcContext);
		KonohaRuntime_free(kctx, ctx);
		MODGC_check_malloced_size(kctx);
		free(kctx->modlocal);
		free(kctx->modshare);
		free(kklib/*, sizeof(KonohaLib) + sizeof(KonohaContextVar)*/);
	}
	else {
		KLIB KdeleteGcContext(ctx->gcContext);
		KFree(ctx->modlocal, sizeof(KonohaModuleContext *) * KonohaModule_MAXSIZE);
		KFree(ctx, sizeof(KonohaContextVar));
	}
}

/* ------------------------------------------------------------------------ */
/* konoha api */

#define BEGIN_(kctx) knh_beginContext(kctx, (void**)&kctx)
#define END_(kctx)   knh_endContext(kctx)

KonohaContext* konoha_open(const PlatformApi *platform)
{
	konoha_init();
	return (KonohaContext *)new_KonohaContext(NULL, platform);
}

void konoha_close(KonohaContext* konoha)
{
	KonohaContext_free(konoha, (KonohaContextVar *)konoha);
}

kbool_t konoha_load(KonohaContext* kctx, const char *scriptname)
{
	PLATAPI BEFORE_LoadScript(kctx, scriptname);
	BEGIN_(kctx);
	kbool_t res = (MODSUGAR_loadScript(kctx, scriptname, strlen(scriptname), 0) == K_CONTINUE);
	END_(kctx);
	PLATAPI AFTER_LoadScript(kctx, scriptname);
	return res;
}

kbool_t konoha_eval(KonohaContext* konoha, const char *script, kfileline_t uline)
{
	BEGIN_(konoha);
	kbool_t res = (MODSUGAR_eval(konoha, script, uline) == K_CONTINUE);
	END_(konoha);
	return res;
}

// -------------------------------------------------------------------------
/* Factory */

void KonohaFactory_LoadRuntimeModule(KonohaFactory *factory, const char *name, ModuleType option)
{
	if(!factory->LoadRuntimeModule(factory, name, option)) {
		factory->printf_i("failed to load module: %s\n", name);
		factory->exit_i(1);
	}
}

void KonohaFactory_SetDefaultFactory(KonohaFactory *factory, void (*SetPlatformApi)(KonohaFactory *), int argc, const char **argv)
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
			KonohaFactory_LoadRuntimeModule(factory, moduleName, ReleaseModule);
		}
	}
}

KonohaContext* KonohaFactory_CreateKonoha(KonohaFactory *factory)
{
	KonohaFactory *platapi = (KonohaFactory *)factory->malloc_i(sizeof(KonohaFactory));
	memcpy(platapi, factory, sizeof(KonohaFactory));
	konoha_init();
	return (KonohaContext *)new_KonohaContext(NULL, (PlatformApi *)platapi);
}

void Konoha_Destroy(KonohaContext *kctx)
{
	KonohaFactory *platapi = (KonohaFactory*)kctx->platApi;
	KonohaContext_free(kctx, (KonohaContextVar *)kctx);
	platapi->free_i(platapi);
}


#ifdef __cplusplus
}
#endif
