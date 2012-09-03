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

/* ************************************************************************ */

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */
// [private functions]

typedef struct {
	KonohaObjectHeader h;
	KonohaContext *ctx;
	pthread_t thread;
	kFunc *func;
	//kArray *args;
} kThread;

typedef struct {
	KonohaObjectHeader h;
	pthread_mutex_t mutex;
} kMutex;

static void *spawn_start(void *v)
{
	//kThread *t = (kThread *)v;
	//KonohaContext *ctx = t->ctx;

	//KONOHA_BEGIN(ctx);
	// set ExceptionHandler
	//kExceptionHandler* hdr = new_(ExceptionHandler);
	//ctx->esp[0].hdr = hdr;
	//ctx->esp++;
	// set args
	//ksfp_t *sfp = ctx->esp;
	//int i, argc = knh_Array_size(t->args);
	//for(i=0; i<argc; i++) {
	//	kObject *o = knh_Array_n(t->args, i);
	//	switch(O_cid(o)) {
	//	case CLASS_Int:
	//		sfp[K_CALLDELTA + i + 1].ivalue = N_toint(o);
	//		break;
	//	case CLASS_Float:
	//		sfp[K_CALLDELTA + i + 1].fvalue = N_tofloat(o);
	//		break;
	//	case CLASS_Boolean:
	//		sfp[K_CALLDELTA + i + 1].bvalue = N_tobool(o);
	//		break;
	//	default:
	//		KNH_SETv(ctx, sfp[K_CALLDELTA + i + 1].o, o);
	//	}
	//}
	//knh_Func_invoke(ctx, t->func, sfp, argc);

	//int jump = knh_setjmp(DP(hdr)->jmpbuf);
	//if(jump == 0) {
	//	hdr->espidx = (ctx->esp - ctx->stack);
	//	hdr->parentNC = ctx->ehdrNC;
	//	((kcontext_t*)ctx)->ehdrNC = hdr;
	//	knh_Func_invoke(ctx, t->func, sfp, argc);
	//} else {
	//	/* catch exception */
	//	hdr = ctx->ehdrNC;
	//	((kcontext_t*)ctx)->ehdrNC = hdr->parentNC;
	//}
	//kthread_detach(ctx, t->thread);
	//KNH_FREE(ctx, t, sizeof(kThread));

	//KNH_SYSLOCK(ctx);
	//ctx->ctxobjNC = NULL;
	//ctx->wshare->threadCounter--;
	//KONOHA_END(ctx);
	//if(ctx->share->gcStopCounter != 0) {
	//	kthread_cond_signal(ctx->share->start_cond);
	//}else if(ctx->share->threadCounter == 1) {
	//	kthread_cond_signal(ctx->share->close_cond);
	//}
	//KNH_SYSUNLOCK(ctx);
	return NULL;
}

static void Thread_init(KonohaContext *kctx, kObject *o, void *conf)
{
	//kThread *t = (kThread *)o;
	//TODO
}

static void Thread_free(KonohaContext *kctx, kObject *o)
{
	//kThread *t = (kThread *)o;
	//TODO
}

static void Thread_reftrace(KonohaContext *kctx, kObject *o)
{
	kThread *t = (kThread *)o;
	BEGIN_REFTRACE(1);
	KREFTRACEv(t->func);
	END_REFTRACE();
}

static void Mutex_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kMutex *m = (kMutex *)o;
	pthread_mutex_init(&m->mutex, NULL);
}

static void Mutex_free(KonohaContext *kctx, kObject *o)
{
	kMutex *m = (kMutex *)o;
	pthread_mutex_destroy(&m->mutex);
}

/* ------------------------------------------------------------------------ */
//## @Native Thread Thread.create(Func f, dynamic[] args)
static KMETHOD Thread_create(KonohaContext *kctx, KonohaStack *sfp)
{
	kFunc *f = sfp[1].asFunc;
	//kArray *args = sfp[2].a;
	if(IS_NOTNULL(f)) {
		kThread *t = (kThread *)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].asObject), 0);
		//kcontext_t *newCtx = new_ThreadContext(WCTX(ctx));
		//t->ctx = newCtx;
		t->func = f;
		//t->args = args;
		pthread_create(&(t->thread), NULL, spawn_start, t);
		RETURN_(t);
	}
}

//## @Native void Thread.join();
static KMETHOD Thread_join(KonohaContext *kctx, KonohaStack *sfp)
{
	kThread *t = (kThread *)sfp[0].o;
	void *v;

	//KNH_SYSLOCK(ctx);
	//ctx->wshare->stopCounter++;
	//if(ctx->share->gcStopCounter != 0) {
	//	kthread_cond_signal(ctx->share->start_cond);
	//}
	//KNH_SYSUNLOCK(ctx);

	pthread_join(t->thread, &v);

	//KNH_SYSLOCK(ctx);
	//ctx->wshare->stopCounter--;
	//KNH_SYSUNLOCK(ctx);

	RETURNvoid_();
}

//## @Native Mutex Mutex.new()
static KMETHOD Mutex_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kMutex *m = (kMutex *)sfp[0].o;
	RETURN_(m);
}

//## @Native void Mutex.lock()
static KMETHOD Mutex_lock(KonohaContext *kctx, KonohaStack *sfp)
{
	kMutex *m = (kMutex *)sfp[0].o;

	//KNH_SYSLOCK(ctx);
	//ctx->wshare->stopCounter++;
	//if(ctx->share->gcStopCounter != 0) {
	//	kthread_cond_signal(ctx->share->start_cond);
	//}
	//KNH_SYSUNLOCK(ctx);

	pthread_mutex_lock(&m->mutex);

	//KNH_SYSLOCK(ctx);
	//ctx->wshare->stopCounter--;
	//KNH_SYSUNLOCK(ctx);

	RETURNvoid_();
}

//## @Native void Mutex.unlock()
static KMETHOD Mutex_unlock(KonohaContext *kctx, KonohaStack *sfp)
{
	kMutex *m = (kMutex *)sfp[0].o;
	pthread_mutex_unlock(&m->mutex);
	RETURNvoid_();
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Static   kMethod_Static
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _Im kMethod_Immutable
#define _F(F)   (intptr_t)(F)

#define TY_Thread cThread->typeId
#define TY_Mutex  cMutex->typeId

static kbool_t thread_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KDEFINE_CLASS defThread = {
		STRUCTNAME(Thread),
		.cflag    = kClass_Final,
		.init     = Thread_init,
		.reftrace = Thread_reftrace,
		.free     = Thread_free,
	};
	KDEFINE_CLASS defMutex = {
		STRUCTNAME(Mutex),
		.cflag = kClass_Final,
		.init  = Mutex_init,
		.free  = Mutex_free,
	};
	KonohaClass *cThread = KLIB Konoha_defineClass(kctx, ns->packageId, ns->packageDomain, NULL, &defThread, pline);
	KonohaClass *cMutex  = KLIB Konoha_defineClass(kctx, ns->packageId, ns->packageDomain, NULL, &defMutex, pline);
	
	kparamtype_t P_Func[] = {{}};
	int TY_FUNC = (KLIB KonohaClass_Generics(kctx, CT_Func, TY_void, 0, P_Func))->typeId;

	int FN_x = FN_("x");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Static, _F(Thread_create), TY_Thread, TY_Thread, MN_("create"), 1, TY_FUNC, FN_x,
		_Public, _F(Thread_join)  , TY_void, TY_Thread, MN_("join"), 0,
		_Public, _F(Mutex_new)    , TY_void, TY_Mutex, MN_("new"), 0,
		_Public, _F(Mutex_lock)   , TY_void, TY_Mutex, MN_("lock"), 0,
		_Public, _F(Mutex_unlock) , TY_void, TY_Mutex, MN_("unlock"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t thread_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t thread_initNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t thread_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* thread_init(void)
{
	static const KDEFINE_PACKAGE d = {
		KPACKNAME("thread", "1.0"),
		.initPackage = thread_initPackage,
		.setupPackage = thread_setupPackage,
		.initNameSpace = thread_initNameSpace,
		.setupNameSpace = thread_setupNameSpace,
	};
	return &d;
}
#ifdef __cplusplus
}
#endif

