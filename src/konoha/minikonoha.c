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

static void knh_beginContext(CTX, void **bottom)
{
	_ctx->stack->cstack_bottom = bottom;
}

static void knh_endContext(CTX)
{
	_ctx->stack->cstack_bottom = NULL;
}

/* ------------------------------------------------------------------------ */
/* stack */

static void KRUNTIME_init(CTX, kcontext_t *ctx, size_t stacksize)
{
	size_t i;
	kstack_t *base = (kstack_t*)KCALLOC(sizeof(kstack_t), 1);
	base->stacksize = stacksize;
	base->stack = (ksfp_t*)KCALLOC(sizeof(ksfp_t), stacksize);
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

static void KRUNTIME_reftrace(CTX, kcontext_t *ctx)
{
	ksfp_t *sp = ctx->stack->stack;
	BEGIN_REFTRACE((_ctx->esp - sp)+1);
	while(sp < ctx->esp) {
		KREFTRACEv(sp[0].o);
		sp++;
	}
	KREFTRACEv(ctx->stack->gcstack);
	END_REFTRACE();
}

static void KRUNTIME_free(CTX, kcontext_t *ctx)
{
	if(_ctx->stack->evaljmpbuf != NULL) {
		KFREE(_ctx->stack->evaljmpbuf, sizeof(jmpbuf_i));
	}
	KARRAY_FREE(&_ctx->stack->cwb);
	KARRAY_FREE(&_ctx->stack->ref);
	KFREE(_ctx->stack->stack, sizeof(ksfp_t) * ctx->stack->stacksize);
	KFREE(_ctx->stack, sizeof(kstack_t));
}

static kbool_t KRUNTIME_setModule(CTX, int x, kmodshare_t *d, kline_t pline)
{
	if(_ctx->modshare[x] == NULL) {
		_ctx->modshare[x] = d;
		return 1;
	}
	else {
		kreportf(CRIT_, pline, "module already registered: %s", _ctx->modshare[x]->name);
		return 0;
	}
}

/* ------------------------------------------------------------------------ */
/* [kcontext] */

static kcontext_t* new_context(const kcontext_t *_ctx, const kplatform_t *plat)
{
	kcontext_t *newctx;
	static volatile size_t ctxid_counter = 0;
	ctxid_counter++;
	if(_ctx == NULL) {  // NULL means first one
		struct _klib2 *klib2 = (struct _klib2*)calloc(sizeof(klib2_t) + sizeof(kcontext_t), 1);
		klib2_init(klib2);
		newctx = (kcontext_t*)(klib2 + 1);
		newctx->lib2 = (klib2_t*)klib2;
		newctx->plat = plat;
		_ctx = (CTX_t)newctx;
		newctx->modshare = (kmodshare_t**)calloc(sizeof(kmodshare_t*), MOD_MAX);
		newctx->modlocal = (kmodlocal_t**)calloc(sizeof(kmodlocal_t*), MOD_MAX);

		MODLOGGER_init(_ctx, newctx);
		MODGC_init(_ctx, newctx);
		KCLASSTABLE_init(_ctx, newctx);
	}
	else {   // others take ctx as its parent
		newctx = (kcontext_t*)KCALLOC(sizeof(kcontext_t), 1);
		newctx->lib2 = _ctx->lib2;
		newctx->plat = _ctx->plat;
		newctx->share = _ctx->share;
		newctx->modshare = _ctx->modshare;
		newctx->modlocal = (kmodlocal_t**)KCALLOC(sizeof(kmodlocal_t*), MOD_MAX);
		MODGC_init(_ctx, newctx);
//		MODLOGGER_init(_ctx, newctx);
	}
	KRUNTIME_init(_ctx, newctx, plat->stacksize);
	if(IS_ROOTCTX(newctx)) {
		MODCODE_init(_ctx, newctx);
		MODSUGAR_init(_ctx, newctx);
		KCLASSTABLE_loadMethod(_ctx);
		MODSUGAR_loadMethod(_ctx);
	}
	else {
//		for(i = 0; i < MOD_MAX; i++) {
//			if(newctx->modshare[i] != NULL && newctx->modshare[i]->new_local != NULL) {
//				newctx->mod[i] = newctx->modshare[i]->new_local((CTX_t)newctx, newctx->modshare[i]);
//			}
//		}
	}
	return newctx;
}

static void kcontext_reftrace(CTX, kcontext_t *ctx)
{
	size_t i;
	if(IS_ROOTCTX(_ctx)) {
		kshare_reftrace(_ctx, ctx);
		for(i = 0; i < MOD_MAX; i++) {
			kmodshare_t *p = ctx->modshare[i];
			if(p != NULL && p->reftrace != NULL) {
				p->reftrace(_ctx, p);
			}
		}
	}
	KRUNTIME_reftrace(_ctx, ctx);
	for(i = 0; i < MOD_MAX; i++) {
		kmodlocal_t *p = ctx->modlocal[i];
		if(p != NULL && p->reftrace != NULL) {
			p->reftrace(_ctx, p);
		}
	}
}

void KRUNTIME_reftraceAll(CTX)
{
	kcontext_reftrace(_ctx, (kcontext_t*)_ctx);
}

static void kcontext_free(CTX, kcontext_t *ctx)
{
	size_t i;
	for(i = 1; i < MOD_MAX; i++) {   // 0 is LOGGER, free lately
		kmodlocal_t *p = ctx->modlocal[i];
		if(p != NULL && p->free != NULL) {
			p->free(_ctx, p);
		}
	}
	KRUNTIME_free(_ctx, ctx);
	if(IS_ROOTCTX(_ctx)){  // share
		struct _klib2 *klib2 = (struct _klib2*)ctx - 1;
		for(i = 0; i < MOD_MAX; i++) {
			kmodshare_t *p = ctx->modshare[i];
			if(p != NULL && p->free != NULL) {
				p->free(_ctx, p);
			}
		}
		MODGC_destoryAllObjects(_ctx, ctx);
		CLASSTABLE_free(_ctx, ctx);
		MODGC_free(_ctx, ctx);
		MODLOGGER_free(_ctx, ctx);
		free(_ctx->modlocal);
		free(_ctx->modshare);
		free(klib2/*, sizeof(klib2_t) + sizeof(kcontext_t)*/);
	}
	else {
		MODGC_free(_ctx, ctx);
		MODLOGGER_free(_ctx, ctx);
		KFREE(_ctx->modlocal, sizeof(kmodlocal_t*) * MOD_MAX);
		KFREE(ctx, sizeof(kcontext_t));
	}
}

/* ------------------------------------------------------------------------ */

// Don't export KONOHA_reftail to packages
// Don't include KONOHA_reftail in shared header files  (kimio)

struct _kObject** KONOHA_reftail(CTX, size_t size)
{
	kstack_t *stack = _ctx->stack;
	size_t ref_size = stack->reftail - stack->ref.refhead;
	if(stack->ref.bytemax/sizeof(void*) < size + ref_size) {
		KARRAY_EXPAND(&stack->ref, (size + ref_size) * sizeof(kObject*));
		stack->reftail = stack->ref.refhead + ref_size;
	}
	struct _kObject **reftail = stack->reftail;
	stack->reftail = NULL;
	return reftail;
}

/* ------------------------------------------------------------------------ */
/* konoha api */

#define BEGIN_(_ctx) knh_beginContext(_ctx, (void**)&_ctx)
#define END_(_ctx)   knh_endContext(_ctx)

konoha_t konoha_open(const kplatform_t *platform)
{
	konoha_init();
	return (konoha_t)new_context(NULL, platform);
}

void konoha_close(konoha_t konoha)
{
	kcontext_free((CTX_t)konoha, (kcontext_t*)konoha);
}

kbool_t konoha_load(konoha_t konoha, const char *scriptname)
{
	BEGIN_(konoha);
	kbool_t res = (MODSUGAR_loadscript((CTX_t)konoha, scriptname, strlen(scriptname), 0) == K_CONTINUE);
	END_(konoha);
	return res;
}

kbool_t konoha_eval(konoha_t konoha, const char *script, kline_t uline)
{
	BEGIN_(konoha);
	kbool_t res = (MODSUGAR_eval((CTX_t)konoha, script, uline) == K_CONTINUE);
	END_(konoha);
	return res;
}

#ifdef USE_BUILTINTEST
#include"testkonoha.h"
#endif


#ifdef __cplusplus
}
#endif
