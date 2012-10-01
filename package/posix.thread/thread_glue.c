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
typedef struct {
	KonohaObjectHeader h;
	KonohaContext *kctx;
	KonohaContext *rootCtx;
	pthread_t thread;
	kFunc *func;
	//kArray *args;
} kThread;

typedef struct {
	KonohaObjectHeader h;
	pthread_mutex_t mutex;
} kMutex;

typedef struct {
	KonohaObjectHeader h;
	pthread_cond_t cond;
} kCond;

static void *spawn_start(void *v)
{
	kThread *t = (kThread *)v;
	KonohaContext *kctx = t->kctx;

	// TODO Exception handling
	// TODO push func arguments

	BEGIN_LOCAL(lsfp, K_CALLDELTA+0);
	KCALL(lsfp, 0, t->func->mtd, 0, K_NULL);
	END_LOCAL();

	KLIB KonohaContext_free(t->rootCtx, (KonohaContextVar *)kctx);
	t->kctx = NULL;
	// TODO cond_signal gc
	return NULL;
}

static void kThread_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kThread *t = (kThread *)o;
	KINITv(t->func, K_NULL);
	//KINITv(t->args, K_NULL);
}

static void kThread_free(KonohaContext *kctx, kObject *o)
{
	//kThread *t = (kThread *)o;
	//TODO
}

static int kThread_compareTo(kObject *o1, kObject *o2)
{
	kThread *t1 = (kThread *)o1;
	kThread *t2 = (kThread *)o2;
	return pthread_equal(t1->thread, t2->thread) != 0 ? 0 : 1;
}

static void kThread_reftrace(KonohaContext *kctx, kObject *o)
{
	kThread *t = (kThread *)o;
	BEGIN_REFTRACE(1);
	KREFTRACEv(t->func);
	//KREFTRACEv(t->args);
	END_REFTRACE();
}

static void kMutex_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kMutex *m = (kMutex *)o;
	pthread_mutex_init(&m->mutex, NULL);
}

static void kMutex_free(KonohaContext *kctx, kObject *o)
{
	kMutex *m = (kMutex *)o;
	pthread_mutex_destroy(&m->mutex);
}

static void kCond_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kCond *c = (kCond *)o;
	pthread_cond_init(&c->cond, NULL);
}

static void kCond_free(KonohaContext *kctx, kObject *o)
{
	kCond *c = (kCond *)o;
	pthread_cond_destroy(&c->cond);
}

/* ------------------------------------------------------------------------ */
//## @Native Thread Thread.create(Func f)
static KMETHOD Thread_create(KonohaContext *kctx, KonohaStack *sfp)
{
	kFunc *f = sfp[1].asFunc;
	KLIB kNameSpace_compileAllDefinedMethods(kctx);
	//kArray *args = sfp[2].a;
	kThread *thread = (kThread *)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].asObject), 0);
	thread->rootCtx = kctx; //TODO getRootContext
	thread->kctx = KLIB KonohaContext_init(kctx, kctx->platApi);
	KSETv(thread, thread->func, f);
	//KSETv(t, t->args, args);
	pthread_create(&(thread->thread), NULL, spawn_start, thread);
	RETURN_(thread);
}

//## @Native void Thread.join();
static KMETHOD Thread_join(KonohaContext *kctx, KonohaStack *sfp)
{
	kThread *t = (kThread *)sfp[0].o;
	void *v;
	// TODO inc gcCounter
	pthread_join(t->thread, &v);
	// TODO dec gcCounter
	RETURNvoid_();
}

//## @Native void Thread.exit();
static KMETHOD Thread_exit(KonohaContext *kctx, KonohaStack *sfp)
{
	pthread_exit(NULL/*FIXME*/);
	RETURNvoid_();
}
	
//## @Native void Thread.cancel();
static KMETHOD Thread_cancel(KonohaContext *kctx, KonohaStack *sfp)
{
	kThread *t = (kThread *)sfp[0].o;
	pthread_cancel(t->thread);
	RETURNvoid_();
}

//## @Native void Thread.detach();
static KMETHOD Thread_detach(KonohaContext *kctx, KonohaStack *sfp)
{
	kThread *t = (kThread *)sfp[0].o;
	pthread_detach(t->thread);
	RETURNvoid_();
}

//## @Native @Static Thread Thread.self();
static KMETHOD Thread_self(KonohaContext *kctx, KonohaStack *sfp)
{
	kThread *t = (kThread *)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].asObject), 0);
	t->kctx = kctx;//FIXME
	t->thread = pthread_self();
	RETURN_(t);
}

//## @Native boolean Thread.equal(Thread t);
static KMETHOD Thread_equal(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(kThread_compareTo(sfp[0].o, sfp[1].o) == 0);
}

/* ------------------------------------------------------------------------ */
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
	// TODO inc gcCounter
	pthread_mutex_lock(&m->mutex);
	// TODO dec gcCounter
	RETURNvoid_();
}

//## @Native boolean Mutex.trylock()
static KMETHOD Mutex_trylock(KonohaContext *kctx, KonohaStack *sfp)
{
	// lock success: return true
	kMutex *m = (kMutex *)sfp[0].o;
	RETURNb_(pthread_mutex_trylock(&m->mutex) == 0);
}

//## @Native void Mutex.unlock()
static KMETHOD Mutex_unlock(KonohaContext *kctx, KonohaStack *sfp)
{
	kMutex *m = (kMutex *)sfp[0].o;
	pthread_mutex_unlock(&m->mutex);
	RETURNvoid_();
}

/* ------------------------------------------------------------------------ */
//## @Native Cond Cond.new()
static KMETHOD Cond_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kCond *c = (kCond *)sfp[0].o;
	RETURN_(c);
}

//## @Native void Cond.wait(Mutex m)
static KMETHOD Cond_wait(KonohaContext *kctx, KonohaStack *sfp)
{
	kCond *c = (kCond *)sfp[0].o;
	kMutex *m = (kMutex *)sfp[1].o;
	// TODO inc gcCounter
	pthread_cond_wait(&c->cond, &m->mutex);
	// TODO dec gcCounter
	RETURNvoid_();
}

//## @Native void Cond.signal()
static KMETHOD Cond_signal(KonohaContext *kctx, KonohaStack *sfp)
{
	kCond *c = (kCond *)sfp[0].o;
	pthread_cond_signal(&c->cond);
	RETURNvoid_();
}

//## @Native void Cond.broadcast()
static KMETHOD Cond_broadcast(KonohaContext *kctx, KonohaStack *sfp)
{
	kCond *c = (kCond *)sfp[0].o;
	pthread_cond_broadcast(&c->cond);
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
#define TY_Cond   cCond->typeId

static kbool_t thread_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KDEFINE_CLASS defThread = {
		STRUCTNAME(Thread),
		.cflag    = kClass_Final,
		.init     = kThread_init,
		.reftrace = kThread_reftrace,
		.free     = kThread_free,
		.compareObject = kThread_compareTo,
	};
	KDEFINE_CLASS defMutex = {
		STRUCTNAME(Mutex),
		.cflag = kClass_Final,
		.init  = kMutex_init,
		.free  = kMutex_free,
	};
	KDEFINE_CLASS defCond = {
		STRUCTNAME(Cond),
		.cflag = kClass_Final,
		.init  = kCond_init,
		.free  = kCond_free,
	};
	KonohaClass *cThread = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defThread, pline);
	KonohaClass *cMutex  = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defMutex, pline);
	KonohaClass *cCond   = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defCond, pline);
	
	kparamtype_t P_Func[] = {{}};
	int TY_FUNC = (KLIB KonohaClass_Generics(kctx, CT_Func, TY_void, 0, P_Func))->typeId;

	int FN_func = FN_("func");
	int FN_x = FN_("x");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Static, _F(Thread_create), TY_Thread, TY_Thread, MN_("create"), 1, TY_FUNC, FN_func,
		_Public, _F(Thread_join)   , TY_void, TY_Thread, MN_("join"), 0,
		_Public, _F(Thread_exit)   , TY_void, TY_Thread, MN_("exit"), 0,
		_Public, _F(Thread_cancel) , TY_void, TY_Thread, MN_("cancel"), 0,
		_Public, _F(Thread_detach) , TY_void, TY_Thread, MN_("detach"), 0,
		_Public|_Static, _F(Thread_self)   , TY_Thread , TY_Thread, MN_("self"), 0,
		_Public, _F(Thread_equal)  , TY_boolean, TY_Thread, MN_("equal"), 1, TY_Thread, FN_x,
		_Public, _F(Mutex_new)     , TY_Mutex, TY_Mutex, MN_("new"), 0,
		_Public, _F(Mutex_lock)    , TY_void, TY_Mutex, MN_("lock"), 0,
		_Public, _F(Mutex_trylock) , TY_boolean, TY_Mutex, MN_("trylock"), 0,
		_Public, _F(Mutex_unlock)  , TY_void, TY_Mutex, MN_("unlock"), 0,
		_Public, _F(Cond_new)      , TY_Cond, TY_Cond, MN_("new"), 0,
		_Public, _F(Cond_wait)     , TY_void, TY_Cond, MN_("wait"), 1, TY_Mutex, FN_x,
		_Public, _F(Cond_signal)   , TY_void, TY_Cond, MN_("signal"), 0,
		_Public, _F(Cond_broadcast), TY_void, TY_Cond, MN_("broadcast"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t thread_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t thread_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t thread_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* thread_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("thread", "1.0"),
		.initPackage    = thread_initPackage,
		.setupPackage   = thread_setupPackage,
		.initNameSpace  = thread_initNameSpace,
		.setupNameSpace = thread_setupNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
