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

#include "../../src/gc/commons.h"
#include "minikonoha/gc.h"
#include "minikonoha/local.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void *do_malloc(size_t size);
static inline void *do_realloc(void *ptr, size_t oldSize, size_t newSize);
static inline void  do_free(void *ptr, size_t size);
static inline void  do_bzero(void *ptr, size_t size);

/* ------------------------------------------------------------------------ */
#define K_SHIFTPTR(p, size)   ((char*)p + (size))
#define K_MEMSIZE(p, p2)      (((char*)p) - ((char*)p2))

#define K_PAGEOBJECTSIZE(i) (K_PAGESIZE / sizeof(kGCObject##i))

#define K_ARENATBL_INITSIZE     32

typedef struct kGCObject0 {
	KonohaObjectHeader h;
	struct kGCObject0 *ref;
	void *ref2_unused;
	void *ref3_unused;
	struct kGCObject0 *ref4_tail;
} kGCObject0;

typedef struct kGCObject1 {
	KonohaObjectHeader h;
	struct kGCObject1 *ref;
	void *ref2_unused;
	void *ref3_unused;
	struct kGCObject1 *ref4_tail;
	uint8_t unused[sizeof(kGCObject0)*2-sizeof(kGCObject0)];
} kGCObject1;

typedef struct kGCObject2 {
	KonohaObjectHeader h;
	struct kGCObject2 *ref;
	void *ref2_unused;
	void *ref3_unused;
	struct kGCObject2 *ref4_tail;
	uint8_t unused[sizeof(kGCObject0)*4-sizeof(kGCObject0)];
} kGCObject2;

typedef union kGCObject {
	kGCObject0 o0;
	kGCObject1 o1;
	kGCObject2 o2;
} kGCObject;

typedef struct objpage0_t {
	kGCObject0  slots[K_PAGEOBJECTSIZE(0)];
} objpage0_t;

typedef struct objpage1_t {
	kGCObject1  slots[K_PAGEOBJECTSIZE(1)];
} objpage1_t;

typedef struct objpage2_t {
	kGCObject2  slots[K_PAGEOBJECTSIZE(2)];
} objpage2_t;

typedef struct objpageTBL_t {
	union {
		objpage0_t      *head0;
		objpage1_t      *head1;
		objpage2_t      *head2;
	};
	union {
		objpage0_t      *bottom0;
		objpage1_t      *bottom1;
		objpage2_t      *bottom2;
	};
	size_t          arenasize;
} objpageTBL_t;

#define ARENA_COUNT_SIZE(size,c) (size) >> (c)
#define K_ARENA_COUNT 3

typedef struct kmemlocal_t {
	KonohaModuleContext     h;
	objpageTBL_t  *ObjectArenaTBL[K_ARENA_COUNT];
	kGCObject     *freeObjectList[K_ARENA_COUNT];
	kGCObject     *freeObjectTail[K_ARENA_COUNT];
	size_t         freeObjectListSize[K_ARENA_COUNT];

	kObject     **queue;
	size_t        queue_capacity;
	size_t        queue_log2;
} kmemlocal_t;

typedef struct kmemshare_t {
	KonohaModule     h;
	objpageTBL_t   *ObjectArenaTBL[K_ARENA_COUNT];
	size_t          sizeObjectArenaTBL[K_ARENA_COUNT];
	size_t          capacityObjectArenaTBL[K_ARENA_COUNT];
} kmemshare_t;

#define memlocal(kctx) ((kmemlocal_t*)((kctx)->modlocal[MOD_gc]))
#define memshare(kctx) ((kmemshare_t*)((kctx)->modshare[MOD_gc]))

static void Arena_init(KonohaContext *kctx, kmemshare_t *memshare)
{
	size_t i;
	for(i = 0;i<K_ARENA_COUNT;++i) {
		memshare->ObjectArenaTBL[i] = (objpageTBL_t *)do_malloc(K_ARENATBL_INITSIZE * sizeof(objpageTBL_t));
		memshare->sizeObjectArenaTBL[i] = 0;
		memshare->capacityObjectArenaTBL[i] = K_ARENATBL_INITSIZE;
	}
}

#define ARENA_FREE(j) do { \
	size_t i;\
	DBG_ASSERT(memshare->ObjectArenaTBL[j] != NULL);\
	for(i = 0; i < memshare->sizeObjectArenaTBL[j]; i++) {\
		objpageTBL_t *oat = memshare->ObjectArenaTBL[j] + i;\
		DBG_ASSERT(K_MEMSIZE(oat->bottom##j, oat->head##j) == oat->arenasize);\
		do_free(oat->head##j, oat->arenasize);\
	}\
	do_free(memshare->ObjectArenaTBL[j], memshare->capacityObjectArenaTBL[j] * sizeof(objpageTBL_t));\
	memshare->ObjectArenaTBL[j] = NULL;\
}while(0)

static void Arena_free(KonohaContext *kctx, kmemshare_t *memshare)
{
	ARENA_FREE(0);
	ARENA_FREE(1);
	ARENA_FREE(2);
}

/* ------------------------------------------------------------------------ */

static inline void do_bzero(void *ptr, size_t size)
{
	memset(ptr, 0, size);
}

static inline void *do_malloc(size_t size)
{
	void *ptr = malloc(size);
	do_bzero(ptr, size);
	return ptr;
}

static inline void *do_realloc(void *ptr, size_t oldSize, size_t newSize)
{
	char *newptr = (char *) realloc(ptr, newSize);
	do_bzero(newptr+oldSize, newSize-oldSize);
	return (void *) newptr;
}

static inline void do_free(void *ptr, size_t size)
{
	free(ptr);
}

static ssize_t kklib_malloced = 0;

static void* Kmalloc(KonohaContext *kctx, size_t s)
{
	size_t *p = (size_t*)do_malloc(s + sizeof(size_t));
	if(unlikely(p == NULL)) {
		ktrace(_ScriptFault|_SystemFault,
				KEYVALUE_s("!",  "OutOfMemory"),
				KEYVALUE_s("at", "malloc"),
				KEYVALUE_u("size", s),
				KEYVALUE_u("malloced_size", kklib_malloced)
		  );
	}
	p[0] = s;
	kklib_malloced += s;
	return (void*)(p+1);
}

static void* Kzmalloc(KonohaContext *kctx, size_t s)
{
	kklib_malloced += s;
	size_t *p = (size_t*)do_malloc(s + sizeof(size_t));
	p[0] = s;
	do_bzero(p+1, s);
	return (void*)(p+1);
}

static void Kfree(KonohaContext *kctx, void *p, size_t s)
{
	size_t *pp = (size_t*)p;
	DBG_ASSERT(pp[-1] == s);
	do_free(pp - 1, s + sizeof(size_t));
	kklib_malloced -= s;
}

/* ------------------------------------------------------- */
#define FREELIST_POP(o,i) do {\
	if(memlocal(kctx)->freeObjectList[i] == NULL) {\
		MODGC_gc_invoke(kctx,0);\
	}\
	DBG_ASSERT(memlocal(kctx)->freeObjectList[i] != NULL);\
	o = memlocal(kctx)->freeObjectList[i];\
	memlocal(kctx)->freeObjectList[i] = (kGCObject *)((kGCObject0 *)o)->ref;\
	((kGCObject0 *)o)->ref = NULL;\
} while(0)

#define FREELIST_PUSH(o,i) do {\
	kGCObject0 *tmp0 = (kGCObject0 *) o;\
	tmp0->ref = (kGCObject0 *) memlocal(kctx)->freeObjectList[i];\
	memlocal(kctx)->freeObjectList[i] = (kGCObject *)o;\
} while(0)

#define OBJECT_REUSE(used,i) do {\
	(used)->h.ct = NULL;\
	(used)->h.refc = 0;\
	FREELIST_PUSH(used,i);\
} while(0)

static void ObjectPage_init0(objpage0_t *opage)
{
	size_t i = 0;
	kGCObject0 *o = opage->slots;
	size_t t = K_PAGEOBJECTSIZE(0) - 1;
	for(i = 0; i < t; ++i) {
		o[i].h.ct = NULL;
		o[i].ref = &(o[i+1]);
	}
	opage->slots[t].h.ct = NULL;
	opage->slots[t].ref = opage[1].slots;
}

static void ObjectPage_init1(objpage1_t *opage)
{
	size_t i = 0;
	kGCObject1 *o = opage->slots;
	size_t t = K_PAGEOBJECTSIZE(1) - 1;
	for(i = 0; i < t; ++i) {
		o[i].h.ct = NULL;
		o[i].ref = &(o[i+1]);
	}
	opage->slots[t].h.ct = NULL;
	opage->slots[t].ref = opage[1].slots;
}

static void ObjectPage_init2(objpage2_t *opage)
{
	size_t i = 0;
	kGCObject2 *o = opage->slots;
	size_t t = K_PAGEOBJECTSIZE(2) - 1;
	for(i = 0; i < t; ++i) {
		o[i].h.ct = NULL;
		o[i].ref = &(o[i+1]);
	}
	opage->slots[t].h.ct = NULL;
	opage->slots[t].ref = opage[1].slots;
}

static void ObjectArenaTBL_init0(KonohaContext *kctx, objpageTBL_t *oat, size_t arenasize)
{
	objpage0_t *opage = (objpage0_t *)do_malloc(arenasize);
	oat->head0 =   opage;
	oat->bottom0 = (objpage0_t *)K_SHIFTPTR(opage, arenasize);
	oat->arenasize = arenasize;
	for(; opage < oat->bottom0; opage++) {
		ObjectPage_init0(opage);
		memlocal(kctx)->freeObjectListSize[0] += K_PAGEOBJECTSIZE(0);
	}
	(opage-1)->slots[K_PAGEOBJECTSIZE(0) - 1].ref = NULL;
}

static void ObjectArenaTBL_init1(KonohaContext *kctx, objpageTBL_t *oat, size_t arenasize)
{
	objpage1_t *opage = (objpage1_t *)do_malloc(arenasize);
	oat->head1 =   opage;
	oat->bottom1 = (objpage1_t *)K_SHIFTPTR(opage, arenasize);
	oat->arenasize = arenasize;
	for(; opage < oat->bottom1; opage++) {
		ObjectPage_init1(opage);
		memlocal(kctx)->freeObjectListSize[1] += K_PAGEOBJECTSIZE(1);
	}
	(opage-1)->slots[K_PAGEOBJECTSIZE(1) - 1].ref = NULL;
}

static void ObjectArenaTBL_init2(KonohaContext *kctx, objpageTBL_t *oat, size_t arenasize)
{
	objpage2_t *opage = (objpage2_t *)do_malloc(arenasize);
	oat->head2 =   opage;
	oat->bottom2 = (objpage2_t *)K_SHIFTPTR(opage, arenasize);
	oat->arenasize = arenasize;
	for(; opage < oat->bottom2; opage++) {
		ObjectPage_init2(opage);
		memlocal(kctx)->freeObjectListSize[2] += K_PAGEOBJECTSIZE(2);
	}
	(opage-1)->slots[K_PAGEOBJECTSIZE(2) - 1].ref = NULL;
}

static kGCObject0 *new_ObjectArena0(KonohaContext *kctx, size_t arenasize)
{
	objpageTBL_t *oat;
	kmemshare_t *memshare = memshare(kctx);
	size_t pageindex = memshare->sizeObjectArenaTBL[0];
	if(unlikely(!(pageindex < memshare->capacityObjectArenaTBL[0]))) {
		size_t oldsize = memshare->capacityObjectArenaTBL[0];
		size_t newsize = oldsize * 2;
		memshare->ObjectArenaTBL[0] = (objpageTBL_t*)do_realloc(memshare->ObjectArenaTBL[0], oldsize * sizeof(objpageTBL_t), newsize * sizeof(objpageTBL_t));
		memshare->capacityObjectArenaTBL[0] = newsize;
	}
	memshare->sizeObjectArenaTBL[0] += 1;
	DBG_ASSERT(sizeof(objpage0_t) == K_PAGESIZE);
	oat = &memshare->ObjectArenaTBL[0][pageindex];
	ObjectArenaTBL_init0(kctx, oat, arenasize);
	kGCObject0 *p = oat->head0->slots;
	p->ref4_tail = (kGCObject0 *)&(oat->bottom0[-1]);
	int i = 0;
	kGCObject0 *tmp = p;
	while (tmp != &oat->head0->slots[K_PAGEOBJECTSIZE(0)]) {
		tmp = tmp->ref;
		i++;
	}
	assert(i == K_PAGEOBJECTSIZE(0));
	return p;
}

static kGCObject1 *new_ObjectArena1(KonohaContext *kctx, size_t arenasize)
{
	objpageTBL_t *oat;
	kmemshare_t *memshare = memshare(kctx);
	size_t pageindex = memshare->sizeObjectArenaTBL[1];
	if(unlikely(!(pageindex < memshare->capacityObjectArenaTBL[1]))) {
		size_t oldsize = memshare->capacityObjectArenaTBL[1];
		size_t newsize = oldsize * 2;
		memshare->ObjectArenaTBL[1] = (objpageTBL_t*)do_realloc(memshare->ObjectArenaTBL[1], oldsize * sizeof(objpageTBL_t), newsize * sizeof(objpageTBL_t));
		memshare->capacityObjectArenaTBL[1] = newsize;
	}
	memshare->sizeObjectArenaTBL[1] += 1;
	DBG_ASSERT(sizeof(objpage1_t) == K_PAGESIZE);
	oat = &memshare->ObjectArenaTBL[1][pageindex];
	ObjectArenaTBL_init1(kctx, oat, arenasize);
	kGCObject1 *p = oat->head1->slots;
	p->ref4_tail = (kGCObject1 *)&(oat->bottom1[-1]);

	int i = 0;
	kGCObject1 *tmp = p;
	while (tmp != &oat->head1->slots[K_PAGEOBJECTSIZE(1)]) {
		tmp = tmp->ref;
		i++;
	}
	assert(i == K_PAGEOBJECTSIZE(1));
	return p;
}

static kGCObject2 *new_ObjectArena2(KonohaContext *kctx, size_t arenasize)
{
	objpageTBL_t *oat;
	kmemshare_t *memshare = memshare(kctx);
	size_t pageindex = memshare->sizeObjectArenaTBL[2];
	if(unlikely(!(pageindex < memshare->capacityObjectArenaTBL[2]))) {
		size_t oldsize = memshare->capacityObjectArenaTBL[2];
		size_t newsize = oldsize * 2;
		memshare->ObjectArenaTBL[2] = (objpageTBL_t*)do_realloc(memshare->ObjectArenaTBL[2], oldsize * sizeof(objpageTBL_t), newsize * sizeof(objpageTBL_t));
		memshare->capacityObjectArenaTBL[2] = newsize;
	}
	memshare->sizeObjectArenaTBL[2] += 1;
	DBG_ASSERT(sizeof(objpage2_t) == K_PAGESIZE);
	oat = &memshare->ObjectArenaTBL[2][pageindex];
	ObjectArenaTBL_init2(kctx, oat, arenasize);
	kGCObject2 *p = oat->head2->slots;
	p->ref4_tail = (kGCObject2 *) &(oat->bottom2[-1]);

	int i = 0;
	kGCObject2 *tmp = p;
	while (tmp != &oat->head2->slots[K_PAGEOBJECTSIZE(2)]) {
		tmp = tmp->ref;
		i++;
	}
	assert(i == K_PAGEOBJECTSIZE(2));
	return p;
}

static void knh_ObjectObjectArenaTBL_free0(KonohaContext *kctx, const objpageTBL_t *oat)
{
	objpage0_t *opage = oat->head0;
	while(opage < oat->bottom0) {
		size_t i;
		for(i = 0; i < K_PAGEOBJECTSIZE(0) - 1; ++i) {
			kGCObject0 *o = &opage->slots[i];
			if(o->h.ct == NULL) continue;
			KONOHA_freeObjectField(kctx, (kObjectVar*)o);
		}
		opage++;
	}
}

static void knh_ObjectObjectArenaTBL_free1(KonohaContext *kctx, const objpageTBL_t *oat)
{
	objpage1_t *opage = oat->head1;
	while(opage < oat->bottom1) {
		size_t i;
		for(i = 0; i < K_PAGEOBJECTSIZE(1) - 1; ++i) {
			kGCObject1 *o = &opage->slots[i];
			if(o->h.ct == NULL) continue;
			KONOHA_freeObjectField(kctx, (kObjectVar*)o);
		}
		opage++;
	}
}

static void knh_ObjectObjectArenaTBL_free2(KonohaContext *kctx, const objpageTBL_t *oat)
{
	objpage2_t *opage = oat->head2;
	while(opage < oat->bottom2) {
		size_t i;
		for(i = 0; i < K_PAGEOBJECTSIZE(2) - 1; ++i) {
			kGCObject2 *o = &opage->slots[i];
			if(o->h.ct == NULL) continue;
			KONOHA_freeObjectField(kctx, (kObjectVar*)o);
		}
		opage++;
	}
}

#define KNH_OBJECTARENA_FINALFREE(j) do {\
	size_t i;\
	DBG_ASSERT(oat != NULL);\
	for(i = 0; i < oatSize; i++) {\
		objpageTBL_t *t = oat + i;\
		knh_ObjectObjectArenaTBL_free##j(kctx, t);\
	}\
} while(0)

static void knh_ObjectArena_finalfree0(KonohaContext *kctx, objpageTBL_t *oat, size_t oatSize)
{
	KNH_OBJECTARENA_FINALFREE(0);
}
static void knh_ObjectArena_finalfree1(KonohaContext *kctx, objpageTBL_t *oat, size_t oatSize)
{
	KNH_OBJECTARENA_FINALFREE(1);
}
static void knh_ObjectArena_finalfree2(KonohaContext *kctx, objpageTBL_t *oat, size_t oatSize)
{
	KNH_OBJECTARENA_FINALFREE(2);
}

void MODGC_destoryAllObjects(KonohaContext *kctx, KonohaContextVar *ctx)
{
	if(IS_RootKonohaContext(ctx)) {
		knh_ObjectArena_finalfree0(ctx, memshare(kctx)->ObjectArenaTBL[0], memshare(kctx)->sizeObjectArenaTBL[0]);
		knh_ObjectArena_finalfree1(ctx, memshare(kctx)->ObjectArenaTBL[1], memshare(kctx)->sizeObjectArenaTBL[1]);
		knh_ObjectArena_finalfree2(ctx, memshare(kctx)->ObjectArenaTBL[2], memshare(kctx)->sizeObjectArenaTBL[2]);
		Arena_free(kctx, memshare(kctx));
	}
}

void kmemlocal_free(KonohaContext *kctx)
{
	if(kctx->memlocal != NULL) {
		if(kctx->memlocal->queue_capacity > 0) {
			do_free(kctx->memlocal->queue,  (kctx->memlocal->queue_capacity + 1) * sizeof(kObject*));
			((KonohaContextVar *)kctx)->memlocal->queue = NULL;
			kctx->memlocal->queue_capacity = 0;
		}
		do_free(kctx->memlocal, sizeof(kmemlocal_t));
	}
}

#define K_ARENASIZE             ((sizeof(kGCObject0) * K_PAGESIZE) * 16) /*4MB*/

#define gc_extendObjectArena(N) do {\
	size_t i = 0;\
	size_t size = memshare(kctx)->sizeObjectArenaTBL[N];\
	for(;i<size;++i) {\
		kGCObject##N *block = new_ObjectArena##N(kctx, K_ARENASIZE);\
		kGCObject##N *tail  = block->ref4_tail;\
		tail->ref = (kGCObject##N *)memlocal(kctx)->freeObjectList[N];\
		memlocal(kctx)->freeObjectList[N] = (kGCObject *)block;\
	}\
} while (0)

static void gc_extendObjectArena0(KonohaContext *kctx)
{
	gc_extendObjectArena(0);
}

static void gc_extendObjectArena1(KonohaContext *kctx)
{
	gc_extendObjectArena(1);
}

static void gc_extendObjectArena2(KonohaContext *kctx)
{
	gc_extendObjectArena(2);
}

/* ------------------------------------------------------- */

typedef struct knh_ostack_t {
	kObject **stack;
	size_t cur;
	size_t tail;
	size_t capacity;
	size_t capacity_log2;
} knh_ostack_t;

static knh_ostack_t *ostack_init(KonohaContext *kctx, knh_ostack_t *ostack)
{
	ostack->capacity = memlocal(kctx)->queue_capacity;
	ostack->stack = memlocal(kctx)->queue;
	ostack->capacity_log2  = memlocal(kctx)->queue_log2;
	if(ostack->capacity == 0) {
		ostack->capacity_log2 = 12;
		ostack->capacity = (1 << ostack->capacity_log2) - 1;
		DBG_ASSERT(K_PAGESIZE == 1 << 12);
		ostack->stack = (kObject**)do_malloc(sizeof(kObject*) * (ostack->capacity + 1));
	}
	ostack->cur  = 0;
	ostack->tail = 0;
	return ostack;
}

static void ostack_push(KonohaContext *kctx, knh_ostack_t *ostack, kObject *ref)
{
	size_t ntail = (ostack->tail + 1 ) & ostack->capacity;
	if(unlikely(ntail == ostack->cur)) {
		size_t capacity = 1 << ostack->capacity_log2;
		ostack->stack = (kObject**)do_realloc(ostack->stack, capacity * sizeof(kObject*), capacity * 2 * sizeof(kObject*));
		ostack->capacity_log2 += 1;
		ostack->capacity = (1 << ostack->capacity_log2) - 1;
		ntail = (ostack->tail + 1) & ostack->capacity;
	}
	ostack->stack[ostack->tail] = ref;
	ostack->tail = ntail;
}

static kObject *ostack_next(knh_ostack_t *ostack)
{
	kObject *ref = NULL;
	if(ostack->cur != ostack->tail) {
		ostack->tail -=1;
		ref = ostack->stack[ostack->tail];
	}
	return ref;
}

static void ostack_free(KonohaContext *kctx, knh_ostack_t *ostack)
{
	memlocal(kctx)->queue_capacity = ostack->capacity;
	memlocal(kctx)->queue = ostack->stack;
	memlocal(kctx)->queue_log2 = ostack->capacity_log2;
}
/* --------------------------------------------------------------- */

static int marked = 0;
static void mark_ostack(KonohaContext *kctx, kObject *ref, knh_ostack_t *ostack,int i)
{
	if(ref->h.refc != 1) {
		((kObjectVar *)ref)->h.refc = 1;
		++marked;
		ostack_push(kctx, ostack, ref);
	}
}

static void gc_init(KonohaContext *kctx)
{
}

#define context_reset_refs(kctx) kctx->stack->reftail = kctx->stack->ref.refhead

static void gc_mark(KonohaContext *kctx)
{
	long i;
	knh_ostack_t ostackbuf, *ostack = ostack_init(kctx, &ostackbuf);
	KonohaContextRuntimeVar *stack = kctx->stack;
	kObject *ref = NULL;
	marked = 0;

	context_reset_refs(kctx);
	KRUNTIME_reftraceAll(kctx);
	size_t ref_size = stack->reftail - stack->ref.refhead;
	goto L_INLOOP;
	while((ref = ostack_next(ostack)) != NULL) {
		context_reset_refs(kctx);
		KONOHA_reftraceObject(kctx, ref);
		ref_size = stack->reftail - stack->ref.refhead;
		if(ref_size > 0) {
			L_INLOOP:;
			for (i = ref_size-1; i >= 0; --i) {
				mark_ostack(kctx, stack->ref.refhead[i], ostack,1);
			}
		}
	}
	ostack_free(kctx, ostack);
}

#define CHECK_EXPAND(listSize,n) do {\
	if(memlocal(kctx)->freeObjectListSize[n] < listSize / 10) {/* 90% */\
		gc_extendObjectArena##n(kctx);\
	}\
}while(0)

static size_t sweep0(KonohaContext *kctx, void *p, int n, size_t sizeOfObject)
{
	unsigned i;
	size_t collected = 0;
	size_t pageSize = K_PAGESIZE/sizeOfObject;
	for(i = 0; i < pageSize; ++i) {
		kGCObject0 *o = (kGCObject0 *) K_SHIFTPTR(p,sizeOfObject*i);
		if(o->h.refc != 1) {
			if( O_ct(o)) {
				//DBG_ASSERT(!IS_Method(o));
				DBG_P("~Object%d %s", n, O_ct(o)->DBG_NAME);
				KONOHA_freeObjectField(kctx, (kObjectVar*)o);
				assert(O_ct(o)->cstruct_size == sizeOfObject);
				++collected;
				OBJECT_REUSE(o, n);
				memlocal(kctx)->freeObjectListSize[n] += 1;
			}
		}
		o->h.refc = 0;
	}
	return collected;
}

static size_t gc_sweep0(KonohaContext *kctx)
{
	size_t collected = 0;
	objpageTBL_t *oat = memshare(kctx)->ObjectArenaTBL[0];
	size_t atindex, size = memshare(kctx)->sizeObjectArenaTBL[0];
	size_t listSize = 0;
	for(atindex = 0; atindex < size; atindex++) {
		objpage0_t *opage = oat[atindex].head0;
		for(;opage < oat[atindex].bottom0; opage++) {
			collected += sweep0(kctx, opage->slots, 0, sizeof(kGCObject0));
			listSize += K_PAGEOBJECTSIZE(0);
		}
	}
	CHECK_EXPAND(listSize,0);
	return collected;
}

static size_t gc_sweep1(KonohaContext *kctx)
{
	size_t collected = 0;
	objpageTBL_t *oat = memshare(kctx)->ObjectArenaTBL[1];
	size_t atindex, size = memshare(kctx)->sizeObjectArenaTBL[1];
	size_t listSize = 0;
	for(atindex = 0; atindex < size; atindex++) {
		objpage1_t *opage = oat[atindex].head1;
		for(;opage < oat[atindex].bottom1; opage++) {
			collected += sweep0(kctx, opage->slots, 1, sizeof(kGCObject1));
			listSize += K_PAGEOBJECTSIZE(1);
		}
	}
	CHECK_EXPAND(listSize,1);
	return collected;
}

static size_t gc_sweep2(KonohaContext *kctx)
{
	size_t collected = 0;
	objpageTBL_t *oat = memshare(kctx)->ObjectArenaTBL[2];
	size_t atindex, size = memshare(kctx)->sizeObjectArenaTBL[2];
	size_t listSize = 0;
	for(atindex = 0; atindex < size; atindex++) {
		objpage2_t *opage = oat[atindex].head2;
		for(;opage < oat[atindex].bottom2; opage++) {
			collected += sweep0(kctx, opage->slots, 2, sizeof(kGCObject2));
			listSize += K_PAGEOBJECTSIZE(2);
		}
	}
	CHECK_EXPAND(listSize,2);
	return collected;
}

static void gc_sweep(KonohaContext *kctx)
{
	gc_sweep0(kctx);
	gc_sweep1(kctx);
	gc_sweep2(kctx);
}

/* --------------------------------------------------------------- */
static void MSGC_local_reftrace(KonohaContext *kctx, struct KonohaModuleContext *baseh) {}

static void MSGC_local_free(KonohaContext *kctx, struct KonohaModuleContext *baseh)
{
	kmemlocal_t *local = (kmemlocal_t *) baseh;
	if(local->queue_capacity > 0) {
		do_free(local->queue,  (local->queue_capacity + 1) * sizeof(kObject*));
		local->queue = NULL;
		local->queue_capacity = 0;
	}
	do_free(local, sizeof(kmemlocal_t));
	kctx->modlocal[MOD_gc] = NULL;
}

#define MSGC_SETUP(i) do {\
	kGCObject##i *p = new_ObjectArena##i(kctx, K_ARENASIZE);\
	base->freeObjectList[i] = (kGCObject *)p;\
	base->freeObjectTail[i] = (kGCObject *)p->ref4_tail;\
	/*base->freeObjectListSize[i] = K_ARENASIZE/K_PAGEOBJECTSIZE(i);*/\
}while(0)

static void MSGC_setup(KonohaContext *kctx, struct KonohaModule *def, int newctx)
{
	if(memlocal(kctx) == NULL) {
		kmemlocal_t *base = do_malloc(sizeof(kmemlocal_t));
		//do_bzero(base, sizeof(kmemlocal_t));
		base->h.reftrace = MSGC_local_reftrace;
		base->h.free     = MSGC_local_free;
		kctx->modlocal[MOD_gc] = (KonohaModuleContext*)base;
		MSGC_SETUP(0);
		MSGC_SETUP(1);
		MSGC_SETUP(2);
	}
}

static void MSGC_reftrace(KonohaContext *kctx, struct KonohaModule *baseh) {}

static void MSGC_free(KonohaContext *kctx, struct KonohaModule *baseh)
{
	do_free(baseh, sizeof(kmemshare_t));
	kctx->modshare[MOD_gc] = NULL;
}

void MODGC_init(KonohaContext *kctx, KonohaContextVar *ctx)
{
	if(IS_RootKonohaContext(ctx)) {
		kmemshare_t *base = (kmemshare_t*) do_malloc(sizeof(kmemshare_t));
		base->h.name     = "msgc";
		base->h.setup    = MSGC_setup;
		base->h.reftrace = MSGC_reftrace;
		Arena_init(ctx,base);
		KSET_KLIB(malloc, 0);
		KSET_KLIB(zmalloc, 0);
		KSET_KLIB(free, 0);
		Konoha_setModule(MOD_gc, &base->h, 0);
	}
	MSGC_setup(ctx, (KonohaModule*) memshare(kctx), 1);
}

void MODGC_free(KonohaContext *kctx, KonohaContextVar *ctx)
{
	assert(memlocal(ctx) == NULL);
	if(IS_RootKonohaContext(ctx)) {
		MSGC_free(kctx, (KonohaModule*) memshare(kctx));
		Konoha_setModule(MOD_gc, NULL, 0);
	}
}

#define OBJECT_INIT(o, size) do_bzero(o, size)

kObject *MODGC_omalloc(KonohaContext *kctx, size_t size)
{
	int page_size = (size / sizeof(kGCObject0)) >> 1;
	DBG_ASSERT(page_size <= 4);
	kGCObject *o = NULL;
	FREELIST_POP(o,page_size);
	memlocal(kctx)->freeObjectListSize[page_size] -= 1;
	do_bzero((void*)o, size);
	return (kObject *)o;
}

void MODGC_gc_invoke(KonohaContext *kctx, int needsCStackTrace)
{
	//TODO : stop the world
	gc_init(kctx);
	gc_mark(kctx);
	gc_sweep(kctx);
	//P(memlocal(kctx)->freeObjectListSize[0]);
}

void MODGC_check_malloced_size(void)
{
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
