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
	KonohaStackRuntimeVar *base = (KonohaStackRuntimeVar*)KCALLOC(sizeof(KonohaStackRuntimeVar), 1);
	base->stacksize = stacksize;
	base->stack = (KonohaStack*)KCALLOC(sizeof(KonohaStack), stacksize);
	assert(stacksize>64);
	base->stack_uplimit = base->stack + (stacksize - 64);
	for(i = 0; i < stacksize; i++) {
		KINITv(base->stack[i].o, K_NULL);
	}
	KINITv(base->gcstack, new_(Array, K_PAGESIZE/sizeof(void*)));
	KLIB Karray_init(kctx, &base->cwb, K_PAGESIZE * 4);
	KLIB Karray_init(kctx, &base->ref, K_PAGESIZE);
	KINITv(base->optionalErrorMessage, TS_EMPTY);
	base->reftail = base->ref.refhead;
	ctx->esp = base->stack;
	ctx->stack = base;
}

static void KonohaStackRuntime_reftrace(KonohaContext *kctx, KonohaContextVar *ctx)
{
	KonohaStack *sp = ctx->stack->stack;
	BEGIN_REFTRACE((kctx->esp - sp) + 2);
	while(sp < ctx->esp) {
		KREFTRACEv(sp[0].o);
		sp++;
	}
	KREFTRACEv(ctx->stack->gcstack);
	KREFTRACEv(ctx->stack->optionalErrorMessage);
	END_REFTRACE();
}

static void KonohaStackRuntime_free(KonohaContext *kctx, KonohaContextVar *ctx)
{
	if(ctx->stack->evaljmpbuf != NULL) {
		KFREE(ctx->stack->evaljmpbuf, sizeof(jmpbuf_i));
	}
	KLIB Karray_free(kctx, &ctx->stack->cwb);
	KLIB Karray_free(kctx, &ctx->stack->ref);
	KFREE(ctx->stack->stack, sizeof(KonohaStack) * ctx->stack->stacksize);
	KFREE(ctx->stack, sizeof(KonohaStackRuntimeVar));
}

static kbool_t KonohaRuntime_setModule(KonohaContext *kctx, int x, KonohaModule *d, kfileline_t pline)
{
	if(kctx->modshare[x] != NULL) {
		kreportf(ErrTag, pline, "module already registered: %s", kctx->modshare[x]->name);
		KLIB KonohaRuntime_raise(kctx, EXPT_("PackageLoader"), NULL, pline, NULL);
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
		KonohaLibVar *klib = (KonohaLibVar*)calloc(sizeof(KonohaLib) + sizeof(KonohaContextVar), 1);
		klib_init(klib);
		klib->KonohaRuntime_setModule    = KonohaRuntime_setModule;
		newctx = (KonohaContextVar*)(klib + 1);
		newctx->klib = (KonohaLib*)klib;
		newctx->platApi = platApi;
		((KonohaLibVar*)newctx->klib)->KonohaContext_init = new_KonohaContext;
		((KonohaLibVar*)newctx->klib)->KonohaContext_free = KonohaContext_free;
		kctx = (KonohaContext*)newctx;
		newctx->modshare = (KonohaModule**)calloc(sizeof(KonohaModule*), KonohaModule_MAXSIZE);
		newctx->modlocal = (KonohaModuleContext**)calloc(sizeof(KonohaModuleContext*), KonohaModule_MAXSIZE);

		MODGC_init(kctx, newctx);
		KonohaRuntime_init(kctx, newctx);
	}
	else {   // others take ctx as its parent
		newctx = (KonohaContextVar*)KCALLOC(sizeof(KonohaContextVar), 1);
		newctx->klib = kctx->klib;
		newctx->platApi = kctx->platApi;
		newctx->share = kctx->share;
		newctx->modshare = kctx->modshare;
		newctx->modlocal = (KonohaModuleContext**)KCALLOC(sizeof(KonohaModuleContext*), KonohaModule_MAXSIZE);
		MODGC_init(kctx, newctx);
	}
	KonohaStackRuntime_init(kctx, newctx, platApi->stacksize);
	if(IS_RootKonohaContext(newctx)) {
		MODCODE_init(kctx, newctx);
		MODSUGAR_init(kctx, newctx);
		Konoha_loadDefaultMethod(kctx);
		MODSUGAR_loadMethod(kctx);
	}
	else {
		int i;
		for(i = 0; i < KonohaModule_MAXSIZE; i++) {
			if(newctx->modshare[i] != NULL && newctx->modshare[i]->setup != NULL) {
				newctx->modshare[i]->setup((KonohaContext *)newctx, newctx->modshare[i], true);
			}
		}
	}
	return newctx;
}

static void KonohaContext_reftrace(KonohaContext *kctx, KonohaContextVar *ctx)
{
	size_t i;
	if(IS_RootKonohaContext(kctx)) {
		KonohaRuntime_reftrace(kctx, ctx);
		for(i = 0; i < KonohaModule_MAXSIZE; i++) {
			KonohaModule *p = ctx->modshare[i];
			if(p != NULL && p->reftrace != NULL) {
				p->reftrace(kctx, p);
			}
		}
	}
	KonohaStackRuntime_reftrace(kctx, ctx);
	for(i = 0; i < KonohaModule_MAXSIZE; i++) {
		KonohaModuleContext *p = ctx->modlocal[i];
		if(p != NULL && p->reftrace != NULL) {
			p->reftrace(kctx, p);
		}
	}
}

void KonohaContext_reftraceAll(KonohaContext *kctx)
{
	KonohaContext_reftrace(kctx, (KonohaContextVar*)kctx);
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
		KonohaLibVar *kklib = (KonohaLibVar*)ctx - 1;
		for(i = 0; i < KonohaModule_MAXSIZE; i++) {
			KonohaModule *p = ctx->modshare[i];
			if(p != NULL && p->free != NULL) {
				p->free(kctx, p);
			}
		}
		KonohaRuntime_free(kctx, ctx);
		MODGC_free(kctx, ctx);
		MODGC_check_malloced_size(kctx);
		free(kctx->modlocal);
		free(kctx->modshare);
		free(kklib/*, sizeof(KonohaLib) + sizeof(KonohaContextVar)*/);
	}
	else {
		MODGC_free(kctx, ctx);
		KFREE(ctx->modlocal, sizeof(KonohaModuleContext*) * KonohaModule_MAXSIZE);
		KFREE(ctx, sizeof(KonohaContextVar));
	}
}

/* ------------------------------------------------------------------------ */
/* konoha api */

#define BEGIN_(kctx) knh_beginContext(kctx, (void**)&kctx)
#define END_(kctx)   knh_endContext(kctx)

KonohaContext* konoha_open(const PlatformApi *platform)
{
	konoha_init();
	return (KonohaContext*)new_KonohaContext(NULL, platform);
}

void konoha_close(KonohaContext* konoha)
{
	KonohaContext_free(konoha, (KonohaContextVar*)konoha);
}

kbool_t konoha_load(KonohaContext* konoha, const char *scriptname)
{
	BEGIN_(konoha);
	kbool_t res = (MODSUGAR_loadScript(konoha, scriptname, strlen(scriptname), 0) == K_CONTINUE);
	END_(konoha);
	return res;
}

kbool_t konoha_eval(KonohaContext* konoha, const char *script, kfileline_t uline)
{
	BEGIN_(konoha);
	kbool_t res = (MODSUGAR_eval(konoha, script, uline) == K_CONTINUE);
	END_(konoha);
	return res;
}

#ifdef __cplusplus
}
#endif
