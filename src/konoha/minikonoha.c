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

#include "klibexec.h"
#include "datatype.h"

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

static void KRUNTIME_init(KonohaContext *kctx, KonohaContextVar *ctx, size_t stacksize)
{
	size_t i;
	KonohaLocalRuntimeVar *base = (KonohaLocalRuntimeVar*)KCALLOC(sizeof(KonohaLocalRuntimeVar), 1);
	base->stacksize = stacksize;
	base->stack = (KonohaStack*)KCALLOC(sizeof(KonohaStack), stacksize);
	assert(stacksize>64);
	base->stack_uplimit = base->stack + (stacksize - 64);
	for(i = 0; i < stacksize; i++) {
		KINITv(base->stack[i].o, K_NULL);
	}
	KINITv(base->gcstack, new_(Array, K_PAGESIZE/sizeof(void*)));
	KARRAY_INIT(&base->cwb, K_PAGESIZE * 4);
	KARRAY_INIT(&base->ref, K_PAGESIZE);
	base->reftail = base->ref.refhead;
	ctx->esp = base->stack;
	ctx->stack = base;
}

static void KRUNTIME_reftrace(KonohaContext *kctx, KonohaContextVar *ctx)
{
	KonohaStack *sp = ctx->stack->stack;
	BEGIN_REFTRACE((kctx->esp - sp)+1);
	while(sp < ctx->esp) {
		KREFTRACEv(sp[0].o);
		sp++;
	}
	KREFTRACEv(ctx->stack->gcstack);
	END_REFTRACE();
}

static void KRUNTIME_free(KonohaContext *kctx, KonohaContextVar *ctx)
{
	if(kctx->stack->evaljmpbuf != NULL) {
		KFREE(kctx->stack->evaljmpbuf, sizeof(jmpbuf_i));
	}
	KARRAY_FREE(&kctx->stack->cwb);
	KARRAY_FREE(&kctx->stack->ref);
	KFREE(kctx->stack->stack, sizeof(KonohaStack) * ctx->stack->stacksize);
	KFREE(kctx->stack, sizeof(KonohaLocalRuntimeVar));
}

static kbool_t KRUNTIME_setModule(KonohaContext *kctx, int x, kmodshare_t *d, kfileline_t pline)
{
	if(kctx->modshare[x] == NULL) {
		kctx->modshare[x] = d;
		return 1;
	}
	else {
		kreportf(CRIT_, pline, "module already registered: %s", kctx->modshare[x]->name);
		return 0;
	}
}

/* ------------------------------------------------------------------------ */
/* [kcontext] */

static KonohaContextVar* new_context(KonohaContext *kctx, const PlatformApi *plat)
{
	KonohaContextVar *newctx;
	static volatile size_t ctxid_counter = 0;
	ctxid_counter++;
	if(kctx == NULL) {  // NULL means first one
		LibKonohaApiVar *klib2 = (LibKonohaApiVar*)calloc(sizeof(LibKonohaApi) + sizeof(KonohaContextVar), 1);
		klib2_init(klib2);
		newctx = (KonohaContextVar*)(klib2 + 1);
		newctx->lib2 = (LibKonohaApi*)klib2;
		newctx->plat = plat;
		kctx = (KonohaContext*)newctx;
		newctx->modshare = (kmodshare_t**)calloc(sizeof(kmodshare_t*), MOD_MAX);
		newctx->modlocal = (kmodlocal_t**)calloc(sizeof(kmodlocal_t*), MOD_MAX);

		MODLOGGER_init(kctx, newctx);
		MODGC_init(kctx, newctx);
		KCLASSTABLE_init(kctx, newctx);
	}
	else {   // others take ctx as its parent
		newctx = (KonohaContextVar*)KCALLOC(sizeof(KonohaContextVar), 1);
		newctx->lib2 = kctx->lib2;
		newctx->plat = kctx->plat;
		newctx->share = kctx->share;
		newctx->modshare = kctx->modshare;
		newctx->modlocal = (kmodlocal_t**)KCALLOC(sizeof(kmodlocal_t*), MOD_MAX);
		MODGC_init(kctx, newctx);
//		MODLOGGER_init(kctx, newctx);
	}
	KRUNTIME_init(kctx, newctx, plat->stacksize);
	if(IS_RootKonohaContext(newctx)) {
		MODCODE_init(kctx, newctx);
		MODSUGAR_init(kctx, newctx);
		KCLASSTABLE_loadMethod(kctx);
		MODSUGAR_loadMethod(kctx);
	}
	else {
//		for(i = 0; i < MOD_MAX; i++) {
//			if(newctx->modshare[i] != NULL && newctx->modshare[i]->new_local != NULL) {
//				newctx->mod[i] = newctx->modshare[i]->new_local((KonohaContext_t)newctx, newctx->modshare[i]);
//			}
//		}
	}
	return newctx;
}

static void kcontext_reftrace(KonohaContext *kctx, KonohaContextVar *ctx)
{
	size_t i;
	if(IS_RootKonohaContext(kctx)) {
		kshare_reftrace(kctx, ctx);
		for(i = 0; i < MOD_MAX; i++) {
			kmodshare_t *p = ctx->modshare[i];
			if(p != NULL && p->reftrace != NULL) {
				p->reftrace(kctx, p);
			}
		}
	}
	KRUNTIME_reftrace(kctx, ctx);
	for(i = 0; i < MOD_MAX; i++) {
		kmodlocal_t *p = ctx->modlocal[i];
		if(p != NULL && p->reftrace != NULL) {
			p->reftrace(kctx, p);
		}
	}
}

void KRUNTIME_reftraceAll(KonohaContext *kctx)
{
	kcontext_reftrace(kctx, (KonohaContextVar*)kctx);
}

static void kcontext_free(KonohaContext *kctx, KonohaContextVar *ctx)
{
	size_t i;
	for(i = 1; i < MOD_MAX; i++) {   // 0 is LOGGER, free lately
		kmodlocal_t *p = ctx->modlocal[i];
		if(p != NULL && p->free != NULL) {
			p->free(kctx, p);
		}
	}
	KRUNTIME_free(kctx, ctx);
	if(IS_RootKonohaContext(kctx)){  // share
		LibKonohaApiVar *klib2 = (LibKonohaApiVar*)ctx - 1;
		for(i = 0; i < MOD_MAX; i++) {
			kmodshare_t *p = ctx->modshare[i];
			if(p != NULL && p->free != NULL) {
				p->free(kctx, p);
			}
		}
		MODGC_destoryAllObjects(kctx, ctx);
		CLASSTABLE_free(kctx, ctx);
		MODGC_free(kctx, ctx);
		MODLOGGER_free(kctx, ctx);
		free(kctx->modlocal);
		free(kctx->modshare);
		free(klib2/*, sizeof(LibKonohaApi) + sizeof(KonohaContextVar)*/);
	}
	else {
		MODGC_free(kctx, ctx);
		MODLOGGER_free(kctx, ctx);
		KFREE(kctx->modlocal, sizeof(kmodlocal_t*) * MOD_MAX);
		KFREE(ctx, sizeof(KonohaContextVar));
	}
}

/* ------------------------------------------------------------------------ */

// Don't export KONOHA_reftail to packages
// Don't include KONOHA_reftail in shared header files  (kimio)

kObjectVar** KONOHA_reftail(KonohaContext *kctx, size_t size)
{
	KonohaLocalRuntimeVar *stack = kctx->stack;
	size_t ref_size = stack->reftail - stack->ref.refhead;
	if(stack->ref.bytemax/sizeof(void*) < size + ref_size) {
		KARRAY_EXPAND(&stack->ref, (size + ref_size) * sizeof(kObject*));
		stack->reftail = stack->ref.refhead + ref_size;
	}
	kObjectVar **reftail = stack->reftail;
	stack->reftail = NULL;
	return reftail;
}

/* ------------------------------------------------------------------------ */
/* konoha api */

#define BEGIN_(kctx) knh_beginContext(kctx, (void**)&kctx)
#define END_(kctx)   knh_endContext(kctx)

KonohaContext* konoha_open(const PlatformApi *platform)
{
	konoha_init();
	return (KonohaContext*)new_context(NULL, platform);
}

void konoha_close(KonohaContext* konoha)
{
	kcontext_free(konoha, (KonohaContextVar*)konoha);
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

#ifdef USE_BUILTINTEST
#include"testkonoha.h"
#endif


#ifdef __cplusplus
}
#endif
