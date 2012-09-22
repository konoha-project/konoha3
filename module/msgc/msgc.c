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
#include "minikonoha/gc.h"
#include "minikonoha/local.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void *do_malloc(size_t size);
static inline void *do_realloc(void *ptr, size_t oldSize, size_t newSize);
static inline void  do_free(void *ptr, size_t size);
static inline void  do_bzero(void *ptr, size_t size);

int verbose_gc = 0;

/* ------------------------------------------------------------------------ */
#define K_SHIFTPTR(p, size)   ((char*)p + (size))
#define K_MEMSIZE(p, p2)      (((char*)p) - ((char*)p2))

#define K_PAGEOBJECTSIZE(i) (K_PAGESIZE / sizeof(kGCObject##i))

#define K_ARENATBL_INITSIZE     32

#define Object_unsetMark(o) TFLAG_set0(uintptr_t,(o)->h.magicflag,kObject_GCFlag)
#define Object_setMark(o)   TFLAG_set1(uintptr_t,(o)->h.magicflag,kObject_GCFlag)
#define Object_isMark(o)   (TFLAG_is(uintptr_t,(o)->h.magicflag, kObject_GCFlag))
#define prefetch_(addr, rw, locality) __builtin_prefetch(addr, rw, locality)


typedef struct kGCObject0 {
	KonohaObjectHeader h;
	struct kGCObject0 *ref;
	void *ref2_unused;
	void *ref3_unused;
	void *ref4_unused;
	struct kGCObject0 *ref5_tail;
} kGCObject0;

typedef struct kGCObject1 {
	KonohaObjectHeader h;
	struct kGCObject1 *ref;
	void *ref2_unused;
	void *ref3_unused;
	void *ref4_unused;
	struct kGCObject1 *ref5_tail;
	uint8_t unused[sizeof(kGCObject0)*2-sizeof(kGCObject0)];
} kGCObject1;

typedef struct kGCObject2 {
	KonohaObjectHeader h;
	struct kGCObject2 *ref;
	void *ref2_unused;
	void *ref3_unused;
	void *ref4_unused;
	struct kGCObject2 *ref5_tail;
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

typedef struct MarkStack {
	kObject **stack;
	size_t tail;
	size_t capacity;
	size_t capacity_log2;
} MarkStack;

typedef struct kmemlocal_t {
	KonohaModuleContext     h;
	objpageTBL_t  *ObjectArenaTBL[K_ARENA_COUNT];
	size_t         sizeObjectArenaTBL[K_ARENA_COUNT];
	size_t         capacityObjectArenaTBL[K_ARENA_COUNT];

	kGCObject     *freeObjectList[K_ARENA_COUNT];
	kGCObject     *freeObjectTail[K_ARENA_COUNT];
	size_t         freeObjectListSize[K_ARENA_COUNT];
	size_t         gc_threshold[K_ARENA_COUNT];

	MarkStack mstack;
} kmemlocal_t;

typedef struct kmemshare_t {
	KonohaModule     h;
} kmemshare_t;

#define memlocal(kctx) ((kmemlocal_t*)((kctx)->modlocal[MOD_gc]))
#define memshare(kctx) ((kmemshare_t*)((kctx)->modshare[MOD_gc]))

#define INIT_THRESHOLD_SIZE(i) do {\
	memlocal(kctx)->gc_threshold[i] = memlocal(kctx)->freeObjectListSize[i] / 8;\
} while(0);

static void Arena_init(KonohaContext *kctx, kmemlocal_t *memlocal)
{
	size_t i;
	for(i = 0;i<K_ARENA_COUNT;++i) {
		memlocal->ObjectArenaTBL[i] = (objpageTBL_t *)do_malloc(K_ARENATBL_INITSIZE * sizeof(objpageTBL_t));
		memlocal->sizeObjectArenaTBL[i] = 0;
		memlocal->capacityObjectArenaTBL[i] = K_ARENATBL_INITSIZE;
	}
}

#define ARENA_FREE(j) do {\
	size_t i;\
	DBG_ASSERT(memlocal->ObjectArenaTBL[j] != NULL);\
	for(i = 0; i < memlocal->sizeObjectArenaTBL[j]; i++) {\
		objpageTBL_t *oat = memlocal->ObjectArenaTBL[j] + i;\
		DBG_ASSERT(K_MEMSIZE(oat->bottom##j, oat->head##j) == oat->arenasize);\
		do_free(oat->head##j, oat->arenasize);\
	}\
	do_free(memlocal->ObjectArenaTBL[j], memlocal->capacityObjectArenaTBL[j] * sizeof(objpageTBL_t));\
	memlocal->ObjectArenaTBL[j] = NULL;\
} while (0)

static void Arena_free(KonohaContext *kctx, kmemlocal_t *memlocal)
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
		OLDTRACE_SWITCH_TO_KTrace(_ScriptFault|_SystemFault,
				LogText("!",  "OutOfMemory"),
				LogText("at", "malloc"),
				LogUint("size", s),
				LogUint("malloced_size", kklib_malloced)
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
	if(memlocal(kctx)->freeObjectListSize[i] <= memlocal(kctx)->gc_threshold[i]) {\
		((KonohaContextVar*)kctx)->safepoint = 1;\
	}\
	DBG_ASSERT(memlocal(kctx)->freeObjectList[i] != NULL);\
	o = memlocal(kctx)->freeObjectList[i];\
	memlocal(kctx)->freeObjectList[i] = (kGCObject *)((kGCObject0 *)o)->ref;\
	((kGCObject0 *)o)->ref = NULL;\
} while (0)

#define FREELIST_PUSH(o,i) do {\
	kGCObject0 *tmp0 = (kGCObject0 *) o;\
	tmp0->ref = (kGCObject0 *) memlocal(kctx)->freeObjectList[i];\
	memlocal(kctx)->freeObjectList[i] = (kGCObject *)o;\
} while (0)

#define OBJECT_REUSE(used,i) do {\
	(used)->h.ct = NULL;\
	(used)->h.magicflag = 0;\
	FREELIST_PUSH(used,i);\
} while (0)

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
	INIT_THRESHOLD_SIZE(0);
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
	INIT_THRESHOLD_SIZE(1);
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
	INIT_THRESHOLD_SIZE(2);
	(opage-1)->slots[K_PAGEOBJECTSIZE(2) - 1].ref = NULL;
}

static kGCObject0 *new_ObjectArena0(KonohaContext *kctx, size_t arenasize)
{
	objpageTBL_t *oat;
	kmemlocal_t *memlocal = memlocal(kctx);
	size_t pageindex = memlocal->sizeObjectArenaTBL[0];
	if(unlikely(!(pageindex < memlocal->capacityObjectArenaTBL[0]))) {
		size_t oldsize = memlocal->capacityObjectArenaTBL[0];
		size_t newsize = oldsize * 2;
		memlocal->ObjectArenaTBL[0] = (objpageTBL_t*)do_realloc(memlocal->ObjectArenaTBL[0], oldsize * sizeof(objpageTBL_t), newsize * sizeof(objpageTBL_t));
		memlocal->capacityObjectArenaTBL[0] = newsize;
	}
	memlocal->sizeObjectArenaTBL[0] += 1;
	DBG_ASSERT(sizeof(objpage0_t) == K_PAGESIZE);
	oat = &memlocal->ObjectArenaTBL[0][pageindex];
	ObjectArenaTBL_init0(kctx, oat, arenasize);
	kGCObject0 *p = oat->head0->slots;
	p->ref5_tail = (kGCObject0 *)&(oat->bottom0[-1]);
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
	kmemlocal_t *memlocal = memlocal(kctx);
	size_t pageindex = memlocal->sizeObjectArenaTBL[1];
	if(unlikely(!(pageindex < memlocal->capacityObjectArenaTBL[1]))) {
		size_t oldsize = memlocal->capacityObjectArenaTBL[1];
		size_t newsize = oldsize * 2;
		memlocal->ObjectArenaTBL[1] = (objpageTBL_t*)do_realloc(memlocal->ObjectArenaTBL[1], oldsize * sizeof(objpageTBL_t), newsize * sizeof(objpageTBL_t));
		memlocal->capacityObjectArenaTBL[1] = newsize;
	}
	memlocal->sizeObjectArenaTBL[1] += 1;
	DBG_ASSERT(sizeof(objpage1_t) == K_PAGESIZE);
	oat = &memlocal->ObjectArenaTBL[1][pageindex];
	ObjectArenaTBL_init1(kctx, oat, arenasize);
	kGCObject1 *p = oat->head1->slots;
	p->ref5_tail = (kGCObject1 *)&(oat->bottom1[-1]);

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
	kmemlocal_t *memlocal = memlocal(kctx);
	size_t pageindex = memlocal->sizeObjectArenaTBL[2];
	if(unlikely(!(pageindex < memlocal->capacityObjectArenaTBL[2]))) {
		size_t oldsize = memlocal->capacityObjectArenaTBL[2];
		size_t newsize = oldsize * 2;
		memlocal->ObjectArenaTBL[2] = (objpageTBL_t*)do_realloc(memlocal->ObjectArenaTBL[2], oldsize * sizeof(objpageTBL_t), newsize * sizeof(objpageTBL_t));
		memlocal->capacityObjectArenaTBL[2] = newsize;
	}
	memlocal->sizeObjectArenaTBL[2] += 1;
	DBG_ASSERT(sizeof(objpage2_t) == K_PAGESIZE);
	oat = &memlocal->ObjectArenaTBL[2][pageindex];
	ObjectArenaTBL_init2(kctx, oat, arenasize);
	kGCObject2 *p = oat->head2->slots;
	p->ref5_tail = (kGCObject2 *) &(oat->bottom2[-1]);

	int i = 0;
	kGCObject2 *tmp = p;
	while (tmp != &oat->head2->slots[K_PAGEOBJECTSIZE(2)]) {
		tmp = tmp->ref;
		i++;
	}
	assert(i == K_PAGEOBJECTSIZE(2));
	return p;
}

#define knh_ObjectObjectArenaTBL_FREE(n) do {\
	objpage##n##_t *opage = oat->head##n;\
	while(opage < oat->bottom##n) {\
		size_t i;\
		for(i = 0; i < K_PAGEOBJECTSIZE(n); ++i) {\
			kGCObject##n *o = &opage->slots[i];\
			if(o->h.ct == NULL) continue;\
			KONOHA_freeObjectField(kctx, (kObjectVar*)o);\
		}\
		opage++;\
	}\
} while (0)

static void knh_ObjectObjectArenaTBL_free0(KonohaContext *kctx, const objpageTBL_t *oat)
{
	knh_ObjectObjectArenaTBL_FREE(0);
}

static void knh_ObjectObjectArenaTBL_free1(KonohaContext *kctx, const objpageTBL_t *oat)
{
	knh_ObjectObjectArenaTBL_FREE(1);
}

static void knh_ObjectObjectArenaTBL_free2(KonohaContext *kctx, const objpageTBL_t *oat)
{
	knh_ObjectObjectArenaTBL_FREE(2);
}

#define KNH_OBJECTARENA_FINALFREE(j) do {\
	size_t i;\
	DBG_ASSERT(oat != NULL);\
	for(i = 0; i < oatSize; i++) {\
		objpageTBL_t *t = oat + i;\
		knh_ObjectObjectArenaTBL_free##j(kctx, t);\
	}\
} while (0)

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
}

#define K_ARENASIZE             ((sizeof(kGCObject0) * K_PAGESIZE) * 16) /*4MB*/

#define gc_extendObjectArena(N) do {\
	size_t i = 0;\
	size_t size = memlocal(kctx)->sizeObjectArenaTBL[N];\
	for(;i<size;++i) {\
		kGCObject##N *block = new_ObjectArena##N(kctx, K_ARENASIZE);\
		kGCObject##N *tail  = block->ref5_tail;\
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
/* [mstack] */

static MarkStack *mstack_init(KonohaContext *kctx, MarkStack *mstack)
{
	if (mstack->capacity == 0) {
		mstack->capacity_log2 = 12;
		mstack->capacity = (1 << mstack->capacity_log2) - 1;
		DBG_ASSERT(K_PAGESIZE == 1 << 12);
		mstack->stack = (kObject**)do_malloc(sizeof(kObject*)*(mstack->capacity + 1));
	}
	mstack->tail = 0;
	return mstack;
}

static void mstack_push(KonohaContext *kctx, MarkStack *mstack, kObject *ref)
{
	size_t ntail = (mstack->tail + 1) & mstack->capacity;
	if (unlikely(ntail == 0)) {
		size_t capacity = 1 << mstack->capacity_log2;
		size_t stacksize = sizeof(kObject*) * capacity;
		mstack->stack = (kObject**)do_realloc(mstack->stack, stacksize, stacksize * 2);
		mstack->capacity_log2 += 1;
		mstack->capacity = (1 << mstack->capacity_log2) - 1;
		ntail = (mstack->tail + 1) & mstack->capacity;
	}
	mstack->stack[mstack->tail] = ref;
	mstack->tail = ntail;
}

static kObject *mstack_next(MarkStack *mstack)
{
	kObject *ref = NULL;
	if (likely(mstack->tail != 0)) {
		mstack->tail -=1;
		ref = mstack->stack[mstack->tail];
		prefetch_(ref, 0, 0);
	}
	return ref;
}
/* --------------------------------------------------------------- */

static int marked = 0;
static void mark_mstack(KonohaContext *kctx, kObject *ref, MarkStack *mstack)
{
	if(!Object_isMark(ref)) {
		Object_setMark((kObjectVar *)ref);
		++marked;
		mstack_push(kctx, mstack, ref);
	}
}

static void gc_init(KonohaContext *kctx)
{
}

#define context_reset_refs(kctx) kctx->stack->reftail = kctx->stack->ref.refhead

static void gc_mark(KonohaContext *kctx)
{
	long i;
	MarkStack *mstack = mstack_init(kctx, &memlocal(kctx)->mstack);
	KonohaStackRuntimeVar *stack = kctx->stack;
	kObject *ref = NULL;
	marked = 0;

	context_reset_refs(kctx);
	KonohaContext_reftraceAll(kctx);
	size_t ref_size = stack->reftail - stack->ref.refhead;
	goto L_INLOOP;
	while((ref = mstack_next(mstack)) != NULL) {
		context_reset_refs(kctx);
		KONOHA_reftraceObject(kctx, ref);
		ref_size = stack->reftail - stack->ref.refhead;
		if(ref_size > 0) {
			L_INLOOP:;
			for (i = ref_size-1; i >= 0; --i) {
				mark_mstack(kctx, stack->ref.refhead[i], mstack);
			}
		}
	}
}

#define CHECK_EXPAND(listSize,n) do {\
	if(memlocal(kctx)->freeObjectListSize[n] <= memlocal(kctx)->gc_threshold[n]) {\
		gc_extendObjectArena##n(kctx);\
	}\
} while (0)

static size_t sweep0(KonohaContext *kctx, void *p, int n, size_t sizeOfObject)
{
	unsigned i;
	size_t collected = 0;
	size_t pageSize = K_PAGESIZE/sizeOfObject;
	for(i = 0; i < pageSize; ++i) {
		kGCObject0 *o = (kGCObject0 *) K_SHIFTPTR(p,sizeOfObject*i);
		if(!Object_isMark((kObject*)o)) {
			if( O_ct(o)) {
				DBG_P("~Object%d %s", n, O_ct(o)->DBG_NAME);
				KONOHA_freeObjectField(kctx, (kObjectVar*)o);
				assert(O_ct(o)->cstruct_size == sizeOfObject);
				++collected;
				OBJECT_REUSE(o, n);
				memlocal(kctx)->freeObjectListSize[n] += 1;
			}
		}
		Object_unsetMark(((kObjectVar*)o));
	}
	return collected;
}
#define GC_SWEEP(n) do {\
	size_t collected     = 0;\
	objpageTBL_t *oat    = memlocal(kctx)->ObjectArenaTBL[n];\
	size_t atindex, size = memlocal(kctx)->sizeObjectArenaTBL[n];\
	size_t listSize      = 0;\
	for(atindex = 0; atindex < size; atindex++) {\
		objpage##n##_t *opage = oat[atindex].head##n;\
		for(;opage < oat[atindex].bottom##n; opage++) {\
			collected += sweep0(kctx, opage->slots, n, sizeof(kGCObject##n));\
			listSize  += K_PAGEOBJECTSIZE(n);\
		}\
	}\
	CHECK_EXPAND(listSize,n);\
	return collected;\
} while (0)

static size_t gc_sweep0(KonohaContext *kctx)
{
	GC_SWEEP(0);
}

static size_t gc_sweep1(KonohaContext *kctx)
{
	GC_SWEEP(1);
}

static size_t gc_sweep2(KonohaContext *kctx)
{
	GC_SWEEP(2);
}

static void gc_sweep(KonohaContext *kctx)
{
	gc_sweep0(kctx);
	gc_sweep1(kctx);
	gc_sweep2(kctx);
	((KonohaContextVar *)kctx)->safepoint = 0;
}

/* --------------------------------------------------------------- */
static void Kwrite_barrier(KonohaContext *kctx, kObject *parent)
{
	(void)kctx;(void)parent;
}

static void Kgc_invoke(KonohaContext *kctx, KonohaStack *esp)
{
	//TODO : stop the world
	gc_init(kctx);
	gc_mark(kctx);
	gc_sweep(kctx);
}
static void MSGC_local_reftrace(KonohaContext *kctx, struct KonohaModuleContext *baseh) {}

#define ARENA_FINAL_FREE(n) knh_ObjectArena_finalfree##n(kctx, memlocal(kctx)->ObjectArenaTBL[n], memlocal(kctx)->sizeObjectArenaTBL[n]);
static void MSGC_local_free(KonohaContext *kctx, struct KonohaModuleContext *baseh)
{
	kmemlocal_t *local = (kmemlocal_t *) baseh;
	ARENA_FINAL_FREE(0);
	ARENA_FINAL_FREE(1);
	ARENA_FINAL_FREE(2);
	Arena_free(kctx, local);
	if(local->mstack.capacity > 0) {
		do_free(local->mstack.stack,  (local->mstack.capacity + 1) * sizeof(kObject*));
		local->mstack.stack = NULL;
		local->mstack.capacity = 0;
	}
	do_free(local, sizeof(kmemlocal_t));
	kctx->modlocal[MOD_gc] = NULL;
}

#define MSGC_SETUP(i) do {\
	kGCObject##i *p = new_ObjectArena##i(kctx, K_ARENASIZE);\
	base->freeObjectList[i] = (kGCObject *)p;\
	base->freeObjectTail[i] = (kGCObject *)p->ref5_tail;\
	/*base->freeObjectListSize[i] = K_ARENASIZE/K_PAGEOBJECTSIZE(i);*/\
} while (0)

static void MSGC_setup(KonohaContext *kctx, struct KonohaModule *def, int newctx)
{
	if(memlocal(kctx) == NULL) {
		kmemlocal_t *base = do_malloc(sizeof(kmemlocal_t));
		base->h.reftrace = MSGC_local_reftrace;
		base->h.free     = MSGC_local_free;
		kctx->modlocal[MOD_gc] = (KonohaModuleContext*)base;
		Arena_init(kctx,base);
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
		KSET_KLIB(Kmalloc, 0);
		KSET_KLIB(Kzmalloc, 0);
		KSET_KLIB(Kfree, 0);
		KSET_KLIB(Kwrite_barrier, 0);
		KSET_KLIB(Kgc_invoke, 0);
		KLIB KonohaRuntime_setModule(kctx, MOD_gc, &base->h, 0);
	}
	MSGC_setup(ctx, (KonohaModule*) memlocal(kctx), 1);
}

void MODGC_free(KonohaContext *kctx, KonohaContextVar *ctx)
{
	assert(memlocal(ctx) == NULL);
	if(IS_RootKonohaContext(ctx)) {
		MSGC_free(kctx, (KonohaModule*) memshare(kctx));
		KLIB KonohaRuntime_setModule(kctx, MOD_gc, NULL, 0);
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


void MODGC_check_malloced_size(KonohaContext *kctx)
{
}

#define IS_Managed(n) do {\
	objpageTBL_t *oat = memlocal(kctx)->ObjectArenaTBL[n];\
	size_t atindex, size = memlocal(kctx)->sizeObjectArenaTBL[n];\
	for(atindex = 0; atindex < size; atindex++) {\
		uintptr_t start = (uintptr_t)oat[atindex].head##n;\
		uintptr_t end   = (uintptr_t)oat[atindex].bottom##n;\
		if (start < o && o < end) {\
			return true;\
		}\
	}\
} while (0)

kbool_t MODGC_kObject_isManaged(KonohaContext *kctx, void *ptr)
{
	uintptr_t o = (uintptr_t)ptr;
	IS_Managed(0);
	IS_Managed(1);
	IS_Managed(2);
	return false;
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
