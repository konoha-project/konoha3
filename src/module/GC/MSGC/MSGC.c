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

/* ************************************************************************ */

#include <stdio.h>

//#define GCDEBUG 1

#include "konoha3/konoha.h"
#include "konoha3/local.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(GCDEBUG) && !defined(GCSTAT)
#define GCSTAT 1
#endif

/* memory config */

#ifndef unlikely
#define unlikely(x)   __builtin_expect(!!(x), 0)
#endif

#ifndef likely
#define likely(x)     __builtin_expect(!!(x), 1)
#endif

#ifdef __GNUC__
#define prefetch_(addr, rw, locality) __builtin_prefetch(addr, rw, locality)
#else
#define prefetch_(addr, rw, locality)
#endif

/* ------------------------------------------------------------------------ */
#define ShiftPointer(p, size)   ((char *)p + (size))
#define MemorySize(p, p2)      ((size_t)(((char *)p) - ((char *)p2)))

#define PageObjectSize(i) (K_PAGESIZE / sizeof(kGCObject##i))

#define ArenaTable_InitSize     32

#define Object_unsetMark(o) KFlag_Set0(uintptr_t,(o)->h.magicflag,kObjectFlag_GCFlag)
#define Object_SetMark(o)   KFlag_Set1(uintptr_t,(o)->h.magicflag,kObjectFlag_GCFlag)
#define Object_isMark(o)   (KFlag_Is(uintptr_t,(o)->h.magicflag, kObjectFlag_GCFlag))
//static int verbose_gc = 0;
static inline void *do_malloc(size_t size);
static inline void *do_calloc(size_t count, size_t size);
static inline void *do_realloc(void *ptr, size_t oldSize, size_t newSize);
static inline void  do_Free(void *ptr, size_t size);
static inline void  do_bzero(void *ptr, size_t size);

/* FIXME ARRAY template */

/* [MSGC Deta Structure] */

typedef struct kGCObject0 {
	kObjectHeader h;
	struct kGCObject0 *ref;
	void *ref2_unused;
	void *ref3_unused;
	void *ref4_unused;
	struct kGCObject0 *ref5_tail;
} kGCObject0;

typedef struct kGCObject1 {
	kObjectHeader h;
	struct kGCObject1 *ref;
	void *ref2_unused;
	void *ref3_unused;
	void *ref4_unused;
	struct kGCObject1 *ref5_tail;
	uint8_t unused[sizeof(kGCObject0)*2-sizeof(kGCObject0)];
} kGCObject1;

typedef struct kGCObject2 {
	kObjectHeader h;
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

typedef struct ObjectPage0_t {
	kGCObject0  slots[PageObjectSize(0)];
} ObjectPage0_t;

typedef struct ObjectPage1_t {
	kGCObject1  slots[PageObjectSize(1)];
} ObjectPage1_t;

typedef struct ObjectPage2_t {
	kGCObject2  slots[PageObjectSize(2)];
} ObjectPage2_t;

typedef struct ObjectPageTable_t {
	union {
		ObjectPage0_t      *head0;
		ObjectPage1_t      *head1;
		ObjectPage2_t      *head2;
	};
	union {
		ObjectPage0_t      *bottom0;
		ObjectPage1_t      *bottom1;
		ObjectPage2_t      *bottom2;
	};
	size_t          arena_size;
} ObjectPageTable_t;

#define ARENA_COUNT 3

typedef struct MarkStack {
	kObject **stack;
	size_t tail;
	size_t capacity;
	size_t capacity_log2;
} MarkStack;

#define GcManager GcContext
typedef struct ArenaTable {
	ObjectPageTable_t  *table;
	size_t              size;
	size_t              capacity;
} ArenaTable;

typedef struct FreeObjectList {
	kGCObject *list;
	size_t     size;
} FreeObjectList;

typedef struct MSGCManager {
	ArenaTable     arena_table;
	FreeObjectList freelist;
	size_t         gc_threshold;
} MSGCManager;

struct GcManager {
	uintptr_t flags;
	KonohaContext *kctx;
	MSGCManager msgc[ARENA_COUNT]; //FIXME
	MarkStack mstack;
};

typedef struct gc_stat {
	size_t total_object;
	//size_t object_count[];
	size_t gc_count;
	//size_t marked[];
	//size_t collected[];
	size_t markingTime;
	size_t sweepingTime;
	size_t current_request_size;
	//AllocationNode *managed_heap;
	//AllocationNode *managed_heap_end;
	FILE *fp;
} msgc_stat;

#ifdef GCSTAT
static msgc_stat global_gc_stat = {};
#endif

/* ------------------------------------------------------------------------ */
/* [MSGC API declaration] */
/* ------------------------------------------------------------------------ */
static void mark_mstack(GcManager *kctx, kObject *ref, MarkStack *mstack);

#define MSGC(i) mng->msgc[(i)]
static GcManager *Arena_Init(KonohaContext *kctx)
{
	GcManager *mng = (GcManager *)do_malloc(sizeof(GcManager));
	size_t i = 0;
	for(; i < ARENA_COUNT; ++i) {
		MSGC(i).arena_table.table = (ObjectPageTable_t *)do_malloc(ArenaTable_InitSize * sizeof(ObjectPageTable_t));
		MSGC(i).arena_table.size = 0;
		MSGC(i).arena_table.capacity = ArenaTable_InitSize;
	}
	return mng;
}

#define ARENA_FREE(j) do {\
	size_t i;\
	DBG_ASSERT(MSGC(j).arena_table.table != NULL);\
	for(i = 0; i < MSGC(j).arena_table.size; i++) {\
		ObjectPageTable_t *oat = MSGC(j).arena_table.table + i;\
		DBG_ASSERT(MemorySize(oat->bottom##j, oat->head##j) == oat->arena_size);\
		do_Free(oat->head##j, oat->arena_size);\
	}\
	do_Free(MSGC(j).arena_table.table, MSGC(j).arena_table.capacity * sizeof(ObjectPageTable_t));\
	/*memlocal->ObjectArenaTBL[j] = NULL;*/\
} while(0)

static void Arena_Free(GcManager *mng)
{
	ARENA_FREE(0);
	ARENA_FREE(1);
	ARENA_FREE(2);
}

/* ------------------------------------------------------------------------ */
/* malloc */

static void THROW_OutOfMemory(KonohaContext *kctx, size_t size)
{
	KExit(EXIT_FAILURE);
}

static inline void do_bzero(void *ptr, size_t size)
{
	memset(ptr, 0, size);
}

static inline void do_memcpy(void *s1, void *s2, size_t size)
{
	memcpy(s1, s2, size);
}

static inline void *do_malloc(size_t size)
{
	void *ptr = malloc(size);
	do_bzero(ptr, size);
	//DBG_CHECK_MALLOCED_INC_SIZE(size);
	return ptr;
}

static inline void *do_calloc(size_t count, size_t size)
{
	void *ptr = calloc(count, size);
	//DBG_CHECK_MALLOCED_INC_SIZE(size);
	return ptr;
}

static inline void *do_realloc(void *ptr, size_t oldSize, size_t newSize)
{
	char *newptr = (char *) realloc(ptr, newSize);
	do_bzero(newptr+oldSize, newSize-oldSize);
	//DBG_CHECK_MALLOCED_INC_SIZE(newSize - oldSize);
	return (void *) newptr;
}

static inline void do_Free(void *ptr, size_t size)
{
#if GCDEBUG
	memset(ptr, 0xa, size);
#endif
	//DBG_CHECK_MALLOCED_DEC_SIZE(size);
	free(ptr);
}

static ssize_t klib_malloced = 0;

static void *Kmalloc(KonohaContext *kctx, size_t s, KTraceInfo *trace)
{
	size_t *p = (size_t *)do_malloc(s
#ifdef MEMORY_DEBUG
			+ sizeof(size_t)
#endif
			);
	if(unlikely(p == NULL)) {
		KTraceApi(trace, SystemFault|UserFault, "malloc",
			LogUint("size", s), LogUint("UsedMemorySize", klib_malloced));
		THROW_OutOfMemory(kctx, s);
	}
#if GCDEBUG
	OLDTRACE_SWITCH_TO_KTrace(LOGPOL_DEBUG,
			LogText("@", "malloc"),
			KeyValue_p("from", p),
			KeyValue_p("to", ((char *)p)+s),
			LogUint("size", s));
#endif
	klib_malloced += s;
#ifdef MEMORY_DEBUG
	p[0] = s;
	p += 1;
#endif
	return (void *)p;
}

static void *Kzmalloc(KonohaContext *kctx, size_t s, KTraceInfo *trace)
{
	size_t *p = (size_t *)do_calloc(1, s
#ifdef MEMORY_DEBUG
			+ sizeof(size_t)
#endif
			);
	if(unlikely(p == NULL)) {
		THROW_OutOfMemory(kctx, s);
	}
	klib_malloced += s;
#ifdef MEMORY_DEBUG
	p[0] = s;
	p += 1;
#endif
	return (void *)(p);
}

static void Kfree(KonohaContext *kctx, void *p, size_t s)
{
	size_t *pp = (size_t *)p;
	klib_malloced -= s;
#ifdef MEMORY_DEBUG
	DBG_ASSERT(pp[-1] == s);
	pp -= 1;
#endif
#if GCDEBUG
	OLDTRACE_SWITCH_TO_KTrace(LOGPOL_DEBUG,
			LogText("@", "free"),
			KeyValue_p("from", p),
			KeyValue_p("to", ((char *)p)+s),
			LogUint("size", s));
#endif
	do_Free(pp, s
#ifdef MEMORY_DEBUG
			+ sizeof(size_t)
#endif
			);
}

/* ------------------------------------------------------- */
#define FREELIST_POP(o,i) do {\
	if(MSGC(i).freelist.size <= MSGC(i).gc_threshold) {\
		mng->flags = 1;\
	}\
	DBG_ASSERT(MSGC(i).freelist.list != NULL);\
	o = MSGC(i).freelist.list;\
	MSGC(i).freelist.list = (kGCObject *)((kGCObject0 *)o)->ref;\
	((kGCObject0 *)o)->ref = NULL;\
} while(0)

#define FREELIST_PUSH(o,i) do {\
	kGCObject0 *tmp0 = (kGCObject0 *) o;\
	tmp0->ref = (kGCObject0 *) MSGC(i).freelist.list;\
	MSGC(i).freelist.list = (kGCObject *)o;\
} while(0)

#define OBJECT_REUSE(used,i) do {\
	(used)->h.ct = NULL;\
	(used)->h.magicflag = 0;\
	FREELIST_PUSH(used,i);\
} while(0)

#define ObjectPage_Init(j) do {\
	size_t i = 0;\
	kGCObject##j *o = opage->slots;\
	size_t t = PageObjectSize(j) - 1;\
	for(i = 0; i < t; ++i) {\
		o[i].h.ct = NULL;\
		o[i].ref = &(o[i+1]);\
	}\
	opage->slots[t].h.ct = NULL;\
	opage->slots[t].ref = opage[1].slots;\
} while(0)

static void ObjectPage_Init0(ObjectPage0_t *opage)
{
	ObjectPage_Init(0);
}

static void ObjectPage_Init1(ObjectPage1_t *opage)
{
	ObjectPage_Init(1);
}

static void ObjectPage_Init2(ObjectPage2_t *opage)
{
	ObjectPage_Init(2);
}

#define INIT_THRESHOLD_SIZE(i) do {\
	MSGC(i).gc_threshold = MSGC(i).freelist.size / 8;\
} while(0);

#define ObjectArenaTable_Init(j) do {\
	ObjectPage##j##_t *opage = (ObjectPage##j##_t *)do_malloc(arenasize);\
	oat->head##j   = opage;\
	oat->bottom##j = (ObjectPage##j##_t *)ShiftPointer(opage, arenasize);\
	oat->arena_size = arenasize;\
	for(; opage < oat->bottom##j; opage++) {\
		ObjectPage_Init##j(opage);\
		MSGC(j).freelist.size += PageObjectSize(j);\
	}\
	INIT_THRESHOLD_SIZE(j);\
	(opage-1)->slots[PageObjectSize(j) - 1].ref = NULL;\
} while(0)

static void ObjectArenaTable_Init0(GcManager *mng, ObjectPageTable_t *oat, size_t arenasize)
{
	ObjectArenaTable_Init(0);
}

static void ObjectArenaTable_Init1(GcManager *mng, ObjectPageTable_t *oat, size_t arenasize)
{
	ObjectArenaTable_Init(1);
}

static void ObjectArenaTable_Init2(GcManager *mng, ObjectPageTable_t *oat, size_t arenasize)
{
	ObjectArenaTable_Init(2);
}

static kGCObject0 *new_ObjectArena0(GcManager *mng, size_t arenasize)
{
	ObjectPageTable_t *oat;
	//KonohaContext *kctx = mng->kctx;
	size_t pageindex = MSGC(0).arena_table.size;
	if(unlikely(!(pageindex < MSGC(0).arena_table.capacity))) {
		size_t oldsize = MSGC(0).arena_table.capacity;
		size_t newsize = oldsize * 2;
		MSGC(0).arena_table.table = (ObjectPageTable_t *)do_realloc(MSGC(0).arena_table.table, oldsize * sizeof(ObjectPageTable_t), newsize * sizeof(ObjectPageTable_t));
		MSGC(0).arena_table.capacity = newsize;
	}
	MSGC(0).arena_table.size += 1;
	DBG_ASSERT(sizeof(ObjectPage1_t) == K_PAGESIZE);
	oat = &MSGC(0).arena_table.table[pageindex];
	ObjectArenaTable_Init0(mng, oat, arenasize);
	kGCObject0 *p = oat->head0->slots;
	p->ref5_tail = (kGCObject0 *)&(oat->bottom0[-1]);

	int i = 0;
	kGCObject0 *tmp = p;
	while(tmp != &oat->head0->slots[PageObjectSize(0)]) {
		tmp = tmp->ref;
		i++;
	}
	assert(i == PageObjectSize(0));
	return p;
}

static kGCObject1 *new_ObjectArena1(GcManager *mng, size_t arenasize)
{
	ObjectPageTable_t *oat;
	//KonohaContext *kctx = mng->kctx;
	size_t pageindex = MSGC(1).arena_table.size;
	if(unlikely(!(pageindex < MSGC(1).arena_table.capacity))) {
		size_t oldsize = MSGC(1).arena_table.capacity;
		size_t newsize = oldsize * 2;
		MSGC(1).arena_table.table = (ObjectPageTable_t *)do_realloc(MSGC(1).arena_table.table, oldsize * sizeof(ObjectPageTable_t), newsize * sizeof(ObjectPageTable_t));
		MSGC(1).arena_table.capacity = newsize;
	}
	MSGC(1).arena_table.size += 1;
	DBG_ASSERT(sizeof(ObjectPage1_t) == K_PAGESIZE);
	oat = &MSGC(1).arena_table.table[pageindex];
	ObjectArenaTable_Init1(mng, oat, arenasize);
	kGCObject1 *p = oat->head1->slots;
	p->ref5_tail = (kGCObject1 *)&(oat->bottom1[-1]);

	int i = 0;
	kGCObject1 *tmp = p;
	while(tmp != &oat->head1->slots[PageObjectSize(1)]) {
		tmp = tmp->ref;
		i++;
	}
	assert(i == PageObjectSize(1));
	return p;
}

static kGCObject2 *new_ObjectArena2(GcManager *mng, size_t arenasize)
{
	ObjectPageTable_t *oat;
	//KonohaContext *kctx = mng->kctx;
	size_t pageindex = MSGC(2).arena_table.size;
	if(unlikely(!(pageindex < MSGC(2).arena_table.capacity))) {
		size_t oldsize = MSGC(2).arena_table.capacity;
		size_t newsize = oldsize * 2;
		MSGC(2).arena_table.table = (ObjectPageTable_t *)do_realloc(MSGC(2).arena_table.table, oldsize * sizeof(ObjectPageTable_t), newsize * sizeof(ObjectPageTable_t));
		MSGC(2).arena_table.capacity = newsize;
	}
	MSGC(2).arena_table.size += 1;
	DBG_ASSERT(sizeof(ObjectPage2_t) == K_PAGESIZE);
	oat = &MSGC(2).arena_table.table[pageindex];
	ObjectArenaTable_Init2(mng, oat, arenasize);
	kGCObject2 *p = oat->head2->slots;
	p->ref5_tail = (kGCObject2 *)&(oat->bottom2[-1]);

	int i = 0;
	kGCObject2 *tmp = p;
	while(tmp != &oat->head2->slots[PageObjectSize(2)]) {
		tmp = tmp->ref;
		i++;
	}
	assert(i == PageObjectSize(2));
	return p;
}

#define ObjectArenaTable_Free(j) do {\
	ObjectPage##j##_t *opage = oat->head##j;\
	while(opage < oat->bottom##j) {\
		size_t i;\
		for(i = 0; i < PageObjectSize(j); ++i) {\
			kGCObject##j *o = &opage->slots[i];\
			if(o->h.ct == NULL) continue;\
			KLIB kObjectProto_Free(kctx, (kObjectVar *)o);\
		}\
		opage++;\
	}\
} while(0)

static void ObjectArenaTable_Free0(KonohaContext *kctx, const ObjectPageTable_t *oat)
{
	ObjectArenaTable_Free(0);
}

static void ObjectArenaTable_Free1(KonohaContext *kctx, const ObjectPageTable_t *oat)
{
	ObjectArenaTable_Free(1);
}

static void ObjectArenaTable_Free2(KonohaContext *kctx, const ObjectPageTable_t *oat)
{
	ObjectArenaTable_Free(2);
}

#define ObjectArena_FinalFree(j) do {\
	size_t i;\
	DBG_ASSERT(oat != NULL);\
	for(i = 0; i < oatSize; i++) {\
		ObjectPageTable_t *t = oat + i;\
		ObjectArenaTable_Free##j(kctx, t);\
	}\
} while(0)

static void ObjectArena_FinalFree0(KonohaContext *kctx, ObjectPageTable_t *oat, size_t oatSize)
{
	ObjectArena_FinalFree(0);
}

static void ObjectArena_FinalFree1(KonohaContext *kctx, ObjectPageTable_t *oat, size_t oatSize)
{
	ObjectArena_FinalFree(1);
}

static void ObjectArena_FinalFree2(KonohaContext *kctx, ObjectPageTable_t *oat, size_t oatSize)
{
	ObjectArena_FinalFree(2);
}

#define K_ARENASIZE  ((sizeof(kGCObject0) * K_PAGESIZE) * 16) /*4MB*/

#define gc_extendObjectArena(N) do {\
	size_t i = 0;\
	size_t size = MSGC(N).arena_table.size;\
	for(;i<size;++i) {\
		kGCObject##N *block = new_ObjectArena##N(mng, K_ARENASIZE);\
		kGCObject##N *tail  = block->ref5_tail;\
		tail->ref = (kGCObject##N *)MSGC(N).freelist.list;\
		MSGC(N).freelist.list = (kGCObject *)block;\
	}\
} while(0)

static void gc_extendObjectArena0(GcManager *mng)
{
	gc_extendObjectArena(0);
}

static void gc_extendObjectArena1(GcManager *mng)
{
	gc_extendObjectArena(1);
}

static void gc_extendObjectArena2(GcManager *mng)
{
	gc_extendObjectArena(2);
}

/* ------------------------------------------------------------------------ */
/* [mstack] */

static MarkStack *mstack_Init(MarkStack *mstack)
{
	if(mstack->capacity == 0) {
		mstack->capacity_log2 = 12;
		mstack->capacity = (1 << mstack->capacity_log2) - 1;
		DBG_ASSERT(K_PAGESIZE == 1 << 12);
		mstack->stack = (kObject**)do_malloc(sizeof(kObject *)*(mstack->capacity + 1));
	}
	mstack->tail = 0;
	return mstack;
}

static void mstack_Push(MarkStack *mstack, kObject *ref)
{
	size_t ntail = (mstack->tail + 1) & mstack->capacity;
	if(unlikely(ntail == 0)) {
		size_t capacity = 1 << mstack->capacity_log2;
		size_t stacksize = sizeof(kObject *) * capacity;
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
	if(likely(mstack->tail != 0)) {
		mstack->tail -=1;
		ref = mstack->stack[mstack->tail];
		prefetch_(ref, 0, 0);
	}
	return ref;
}

/* --------------------------------------------------------------- */

#define MSGC_SETUP(i) do {\
	kGCObject##i *p = new_ObjectArena##i(mng, K_ARENASIZE);\
	MSGC(i).freelist.list = (kGCObject *)p;\
} while(0)

static void KnewGcContext(KonohaContext *kctx)
{
	((KonohaContextVar *)kctx)->gcContext = Arena_Init(kctx);
	GcManager *mng = (GcManager *)kctx->gcContext;
	mng->kctx = kctx;
	MSGC_SETUP(0);
	MSGC_SETUP(1);
	MSGC_SETUP(2);

#ifdef GCSTAT
	global_gc_stat.fp = fopen("KONOHA_BMGC_INFO", "a");
#endif
	((KonohaContextVar *)kctx)->gcContext = mng;
}

#define  FinalFree(n) ObjectArena_FinalFree##n(mng->kctx, MSGC(n).arena_table.table, MSGC(n).arena_table.size);
static void KdeleteGcContext(KonohaContext *kctx)
{
	GcManager *mng = (GcManager *)kctx->gcContext;
	FinalFree(0);
	FinalFree(1);
	FinalFree(2);
	Arena_Free(mng);
	if(mng->mstack.capacity > 0) {
		do_Free(mng->mstack.stack,  (mng->mstack.capacity + 1) * sizeof(kObject *));
		mng->mstack.capacity = 0;
	}
#ifdef GCSTAT
	fclose(global_gc_stat.fp);
#endif
}

/* ------------------------------------------------------------------------ */

typedef struct ObjectGraphTracer {
	KObjectVisitor base;
	GcManager  *mng;
	MarkStack  *mstack;
} ObjectGraphTracer;

static void ObjectGraphTracer_visit(KObjectVisitor *visitor, kObject *object)
{
	ObjectGraphTracer *tracer = (ObjectGraphTracer *) visitor;
	mark_mstack(tracer->mng, object, tracer->mstack);
}

static void ObjectGraphTracer_visitRange(KObjectVisitor *visitor, kObject **begin, kObject **end)
{
	ObjectGraphTracer *tracer = (ObjectGraphTracer *) visitor;
	kObject **itr;
	for (itr = begin; itr != end; ++itr) {
		mark_mstack(tracer->mng, *itr, tracer->mstack);
	}
}


/* ------------------------------------------------------------------------ */

static void Kwrite_barrier(KonohaContext *kctx, kObject *parent)
{
	(void)kctx;(void)parent;
}

static void KupdateObjectField(kObject *parent, kObject *oldValPtr, kObject *newVal)
{
	(void)parent;(void)oldValPtr;(void)newVal;
}


static int marked = 0;
static void mark_mstack(GcManager *mng, kObject *ref, MarkStack *mstack)
{
	if(!Object_isMark(ref)) {
		Object_SetMark((kObjectVar *)ref);
		++marked;
		mstack_Push(mstack, ref);
	}
}

static void msgc_gc_Init(GcManager *mng)
{
}

static void msgc_gc_mark(GcManager *mng)
{
	KonohaContext *kctx = mng->kctx;
	MarkStack *mstack = mstack_Init(&mng->mstack);
	kObject *ref = NULL;
	ObjectGraphTracer tracer = {};
	tracer.base.fn_visit      = ObjectGraphTracer_visit;
	tracer.base.fn_visitRange = ObjectGraphTracer_visitRange;
	tracer.mng    = mng;
	tracer.mstack = mstack;

	KLIB ReftraceAll(kctx, &tracer.base);

	ref = mstack_next(mstack);
	if(unlikely(ref == 0))
		return;
	do {
		KLIB kObjectProto_Reftrace(kctx, ref, &tracer.base);
	} while((ref = mstack_next(mstack)) != NULL);
}

#define CHECK_EXPAND(listSize,n) do {\
	if(MSGC(n).freelist.size <= MSGC(n).gc_threshold) {\
		gc_extendObjectArena##n(mng);\
	}\
} while(0)

static size_t sweep0(GcContext *mng, void *p, int n, size_t sizeOfObject)
{
	KonohaContext *kctx = mng->kctx;
	unsigned i;
	size_t collected = 0;
	size_t pageSize = K_PAGESIZE/sizeOfObject;
	for(i = 0; i < pageSize; ++i) {
		kGCObject0 *o = (kGCObject0 *)ShiftPointer(p,sizeOfObject*i);
		if(!Object_isMark((kObject *)o)) {
			if( kObject_class(o)) {
				DBG_P("~Object%d %s", n, kObject_class(o)->DBG_NAME);
				KLIB kObjectProto_Free(kctx, (kObjectVar *)o);
				assert(kObject_class(o)->cstruct_size == sizeOfObject);
				++collected;
				OBJECT_REUSE(o, n);
				MSGC(n).freelist.size += 1;
			}
		}
		Object_unsetMark(((kObjectVar *)o));
	}
	return collected;
}

#define GC_SWEEP(n) do {\
	size_t collected       = 0;\
	ObjectPageTable_t *oat = MSGC(n).arena_table.table;\
	size_t atindex, size   = MSGC(n).arena_table.size;\
	size_t listSize        = 0;\
	for(atindex = 0; atindex < size; atindex++) {\
		ObjectPage##n##_t *opage = oat[atindex].head##n;\
		for(;opage < oat[atindex].bottom##n; opage++) {\
			collected += sweep0(mng, opage->slots, n, sizeof(kGCObject##n));\
			listSize  += PageObjectSize(n);\
		}\
	}\
	CHECK_EXPAND(listSize,n);\
	return collected;\
} while(0)

static size_t gc_sweep0(GcManager *mng)
{
	GC_SWEEP(0);
}

static size_t gc_sweep1(GcManager *mng)
{
	GC_SWEEP(1);
}

static size_t gc_sweep2(GcManager *mng)
{
	GC_SWEEP(2);
}

static void msgc_gc_sweep(GcManager *mng)
{
	gc_sweep0(mng);
	gc_sweep1(mng);
	gc_sweep2(mng);
}

static void MarkAndSweepGC(GcManager *mng)
{
	msgc_gc_Init(mng);
	msgc_gc_mark(mng);
	msgc_gc_sweep(mng);
	mng->flags = 0;
}

/* ------------------------------------------------------------------------ */
/* [MODGC API] */

//void MODGC_Check_malloced_size(KonohaContext *kctx)
//{
//	if(verbose_gc) {
//		PLATAPI printf_i("\nklib:memory leaked=%ld\n", (long)klib_malloced);
//	}
//}

static kObjectVar *KallocObject(KonohaContext *kctx, size_t size, KTraceInfo *trace)
{
	GcManager *mng = (GcManager *)kctx->gcContext;
	int page_size = (size / sizeof(kGCObject0)) >> 1;
	DBG_ASSERT(page_size <= 4);
	kGCObject *o = NULL;
	FREELIST_POP(o,page_size);
	MSGC(page_size).freelist.size -= 1;
	do_bzero((void *)o, size);
#if GCDEBUG
	OLDTRACE_SWITCH_TO_KTrace(LOGPOL_DEBUG,
			LogText("@", "new"),
			KeyValue_p("ptr", o),
			LogUint("size", size));
#endif
	return (kObjectVar *)o;
}

#define IS_Managed(n) do {\
	ObjectPageTable_t *oat = MSGC(n).arena_table.table;\
	size_t atindex, size = MSGC(n).arena_table.size;\
	for(atindex = 0; atindex < size; atindex++) {\
		uintptr_t start = (uintptr_t)oat[atindex].head##n;\
		uintptr_t end   = (uintptr_t)oat[atindex].bottom##n;\
		if(start < o && o < end) {\
			return true;\
		}\
	}\
} while(0)

static kbool_t KisObject(KonohaContext *kctx, void *ptr)
{
	GcManager *mng = (GcManager *)kctx->gcContext;
	uintptr_t o = (uintptr_t)ptr;
	IS_Managed(0);
	IS_Managed(1);
	IS_Managed(2);
	return false;
}

static void KscheduleGC(KonohaContext *kctx, KTraceInfo *trace)
{
	GcManager *mng = (GcManager *)kctx->gcContext;
	if(mng->flags) {
		//gc_info("scheduleGC mode=%d", mode);
		MarkAndSweepGC(mng);
	}
}
/* ------------------------------------------------------------------------ */

kbool_t LoadMSGCModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"MSGC", "0.1", 0, "msgc",
	};
	factory->GCModule.GCInfo            = &ModuleInfo;
	factory->GCModule.Kmalloc           = Kmalloc;
	factory->GCModule.Kzmalloc          = Kzmalloc;
	factory->GCModule.Kfree             = Kfree;
	factory->GCModule.InitGcContext     = KnewGcContext;
	factory->GCModule.DeleteGcContext   = KdeleteGcContext;
	factory->GCModule.ScheduleGC        = KscheduleGC;
	factory->GCModule.AllocObject       = KallocObject;
	factory->GCModule.WriteBarrier      = Kwrite_barrier;   // check this
	factory->GCModule.UpdateObjectField = KupdateObjectField;  // check this
	factory->GCModule.IsKonohaObject    = KisObject;
	return true;
}

/* ------------------------------------------------------------------------ */
#ifdef __cplusplus
}
#endif
