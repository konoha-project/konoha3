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

//	BEGIN_LOCAL(lsfp, K_CALLDELTA+0);
//	KCALL(lsfp, 0, t->func->mtd, 0, K_NULL);
//	END_LOCAL();

	KLIB KonohaContext_Free(t->rootCtx, (KonohaContextVar *)kctx);
	t->kctx = NULL;
	// TODO cond_signal gc
	return NULL;
}

static void kThread_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kThread *t = (kThread *)o;
	KFieldInit(t, t->func, K_NULL);
	//KFieldInit(t, t->args, K_NULL);
}

static void kThread_Free(KonohaContext *kctx, kObject *o)
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

static void kThread_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kThread *t = (kThread *)o;
	KRefTrace(t->func);
	//KRefTrace(t->args);
}

static void kMutex_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kMutex *m = (kMutex *)o;
	pthread_mutex_init(&m->mutex, NULL);
}

static void kMutex_Free(KonohaContext *kctx, kObject *o)
{
	kMutex *m = (kMutex *)o;
	pthread_mutex_destroy(&m->mutex);
}

static void kCond_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kCond *c = (kCond *)o;
	pthread_cond_init(&c->cond, NULL);
}

static void kCond_Free(KonohaContext *kctx, kObject *o)
{
	kCond *c = (kCond *)o;
	pthread_cond_destroy(&c->cond);
}

/* ------------------------------------------------------------------------ */
//## @Native Thread Thread.create(Func f)
static KMETHOD Thread_create(KonohaContext *kctx, KonohaStack *sfp)
{
	INIT_GCSTACK();
	kFunc *f = sfp[1].asFunc;
	KLIB kMethod_DoLazyCompilation(kctx, (f)->mtd, NULL, HatedLazyCompile);
	kThread *thread = (kThread *)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), 0);
	thread->rootCtx = kctx; //TODO getRootContext
	thread->kctx = KLIB KonohaContext_Init(kctx, kctx->platApi);
	KFieldSet(thread, thread->func, f);
	pthread_create(&(thread->thread), NULL, spawn_start, thread);
	RESET_GCSTACK(); // FIXME?? Not sure this is okay??
	KReturn(thread);
}

//## @Native void Thread.join();
static KMETHOD Thread_join(KonohaContext *kctx, KonohaStack *sfp)
{
	kThread *t = (kThread *)sfp[0].asObject;
	void *v;
	// TODO inc gcCounter
	pthread_join(t->thread, &v);
	// TODO dec gcCounter
	KReturnVoid();
}

//## @Native void Thread.exit();
static KMETHOD Thread_exit(KonohaContext *kctx, KonohaStack *sfp)
{
	pthread_exit(NULL/*FIXME*/);
	KReturnVoid();
}

//## @Native void Thread.cancel();
static KMETHOD Thread_cancel(KonohaContext *kctx, KonohaStack *sfp)
{
	kThread *t = (kThread *)sfp[0].asObject;
	pthread_cancel(t->thread);
	KReturnVoid();
}

//## @Native void Thread.detach();
static KMETHOD Thread_detach(KonohaContext *kctx, KonohaStack *sfp)
{
	kThread *t = (kThread *)sfp[0].asObject;
	pthread_detach(t->thread);
	KReturnVoid();
}

//## @Native @Static Thread Thread.self();
static KMETHOD Thread_self(KonohaContext *kctx, KonohaStack *sfp)
{
	kThread *t = (kThread *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	t->kctx = kctx;//FIXME
	t->thread = pthread_self();
	KReturn(t);
}

//## @Native boolean Thread.equal(Thread t);
static KMETHOD Thread_equal(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kThread_compareTo(sfp[0].asObject, sfp[1].asObject) == 0);
}

/* ------------------------------------------------------------------------ */
//## @Native Mutex Mutex.new()
static KMETHOD Mutex_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kMutex *m = (kMutex *)sfp[0].asObject;
	KReturn(m);
}

//## @Native void Mutex.lock()
static KMETHOD Mutex_lock(KonohaContext *kctx, KonohaStack *sfp)
{
	kMutex *m = (kMutex *)sfp[0].asObject;
	// TODO inc gcCounter
	pthread_mutex_lock(&m->mutex);
	// TODO dec gcCounter
	KReturnVoid();
}

//## @Native boolean Mutex.trylock()
static KMETHOD Mutex_trylock(KonohaContext *kctx, KonohaStack *sfp)
{
	// lock success: return true
	kMutex *m = (kMutex *)sfp[0].asObject;
	KReturnUnboxValue(pthread_mutex_trylock(&m->mutex) == 0);
}

//## @Native void Mutex.unlock()
static KMETHOD Mutex_unlock(KonohaContext *kctx, KonohaStack *sfp)
{
	kMutex *m = (kMutex *)sfp[0].asObject;
	pthread_mutex_unlock(&m->mutex);
	KReturnVoid();
}

/* ------------------------------------------------------------------------ */
//## @Native Cond Cond.new()
static KMETHOD Cond_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kCond *c = (kCond *)sfp[0].asObject;
	KReturn(c);
}

//## @Native void Cond.wait(Mutex m)
static KMETHOD Cond_wait(KonohaContext *kctx, KonohaStack *sfp)
{
	kCond *c = (kCond *)sfp[0].asObject;
	kMutex *m = (kMutex *)sfp[1].asObject;
	// TODO inc gcCounter
	pthread_cond_wait(&c->cond, &m->mutex);
	// TODO dec gcCounter
	KReturnVoid();
}

//## @Native void Cond.signal()
static KMETHOD Cond_signal(KonohaContext *kctx, KonohaStack *sfp)
{
	kCond *c = (kCond *)sfp[0].asObject;
	pthread_cond_signal(&c->cond);
	KReturnVoid();
}

//## @Native void Cond.broadcast()
static KMETHOD Cond_broadcast(KonohaContext *kctx, KonohaStack *sfp)
{
	kCond *c = (kCond *)sfp[0].asObject;
	pthread_cond_broadcast(&c->cond);
	KReturnVoid();
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Static   kMethod_Static
#define _F(F)   (intptr_t)(F)

#define TY_Thread cThread->typeId
#define TY_Mutex  cMutex->typeId
#define TY_Cond   cCond->typeId

static kbool_t thread_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KDEFINE_CLASS defThread = {
		STRUCTNAME(Thread),
		.cflag    = kClass_Final,
		.init     = kThread_Init,
		.reftrace = kThread_Reftrace,
		.free     = kThread_Free,
		.compareObject = kThread_compareTo,
	};
	KDEFINE_CLASS defMutex = {
		STRUCTNAME(Mutex),
		.cflag = kClass_Final,
		.init  = kMutex_Init,
		.free  = kMutex_Free,
	};
	KDEFINE_CLASS defCond = {
		STRUCTNAME(Cond),
		.cflag = kClass_Final,
		.init  = kCond_Init,
		.free  = kCond_Free,
	};
	KonohaClass *cThread = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defThread, trace);
	KonohaClass *cMutex  = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defMutex, trace);
	KonohaClass *cCond   = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defCond, trace);

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
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

static kbool_t thread_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* thread_Init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("thread", "1.0"),
		.PackupNameSpace    = thread_PackupNameSpace,
		.ExportNameSpace   = thread_ExportNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif