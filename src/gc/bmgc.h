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

#include <stdbool.h>
#include <stdio.h>

#define K_USING_POSIX_
#if defined(K_USING_POSIX_)
#include <time.h>
#include <sys/time.h>
#elif defined(K_USING_WINDOWS_)
#include <windows.h>
#include <time.h>
#elif defined(K_USING_BTRON)
#include <btron/datetime.h>
#include <btron/event.h>
#endif

#include "minikonoha/gc.h"
#include "minikonoha/local.h"

#ifndef BMGC_H_
#define BMGC_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(GCDEBUG) && !defined(GCSTAT)
#define GCSTAT 1
#endif

/* memory config */
#define GC_USE_DEFERREDSWEEP 1
#define SUBHEAP_DEFAULT_SEGPOOL_SIZE (128)/* 128 * SEGMENT_SIZE(128k) = 16MB*/
#define SUBHEAP_KLASS_MIN  5 /* 1 <<  5 == 32 */
#define SUBHEAP_KLASS_MAX 12 /* 1 << 12 == 4096 */
#define MIN_ALIGN (ONE << SUBHEAP_KLASS_MIN)

#define SEGMENT_SIZE (128 * KB_)
#define SEGMENT_LEVEL 3
#define ONE ((uintptr_t)1)
#define KlassBlockSize(klass) (ONE << klass)
#define SUBHEAP_KLASS_SIZE_MIN KlassBlockSize(SUBHEAP_KLASS_MIN)
#define SUBHEAP_KLASS_SIZE_MAX KlassBlockSize(SUBHEAP_KLASS_MAX)
#define BITMAP_FULL ((uintptr_t)(-1))
#define ALIGN(x,n)  (((x)+((n)-1))&(~((n)-1)))
#define CEIL(F)     (F-(int)(F) > 0 ? (int)(F+1) : (int)(F))
#define PTR_SIZE (sizeof(void*))
#define BITS (PTR_SIZE * 8)
#if _WIN64
#define FFS(n) __builtin_ffsll(n)
#define CLZ(n) __builtin_clzll(n)
#else
#define FFS(n) __builtin_ffsl(n)
#define CLZ(n) __builtin_clzl(n)
#endif
#define BSR(n) CLZ(n)
//#define BSR(n) (CLZ(n) & 0x3f)
#define BM_SET(m, mask)  (m |= mask)
#define BM_TEST(m, mask) (m  & mask)

#ifdef unlikely
#undef unlikely
#endif

#ifdef likely
#undef likely
#endif

#define unlikely(x)   __builtin_expect(!!(x), 0)
#define likely(x)     __builtin_expect(!!(x), 1)
#define prefetch_(addr, rw, locality) __builtin_prefetch(addr, rw, locality)

#define for_each_heap(H, I, HEAPS) \
	for (I = SUBHEAP_KLASS_MIN, H = (HEAPS)+I; I <= SUBHEAP_KLASS_MAX; ++H, ++I)

static inline void *do_malloc(size_t size);
static inline void *do_realloc(void *ptr, size_t oldSize, size_t newSize);
static inline void  do_free(void *ptr, size_t size);

/* ARRAY */
#define ARRAY(T) ARRAY_##T##_t
#define DEF_ARRAY_STRUCT(T) \
	struct ARRAY(T) {\
		T *list;\
		int size;  \
		int capacity;  \
	}

#define DEF_ARRAY_T(T)              \
struct ARRAY(T);                    \
typedef struct ARRAY(T) ARRAY(T);   \
DEF_ARRAY_STRUCT(T)

#define DEF_ARRAY_OP(T)\
static inline ARRAY(T) *ARRAY_init_##T (ARRAY(T) *a) {\
	a->list = (T*) do_malloc(4 * sizeof(T));\
	a->capacity  = 4;\
	a->size  = 0;\
	return a;\
}\
static inline void ARRAY_##T##_add(ARRAY(T) *a, T v) {\
	if (a->size + 1 >= a->capacity) {\
		size_t os = sizeof(T) * a->capacity;\
		a->capacity *= 2;\
		a->list = (T*)do_realloc(a->list, os, sizeof(T) * a->capacity);\
	}\
	a->list[a->size++] = v;\
}\
static inline T ARRAY_##T##_get(ARRAY(T) *a, int idx) {\
	return a->list[idx];\
}\
static inline void ARRAY_##T##_set(ARRAY(T) *a, int idx, T v){ \
	a->list[idx] = v;\
}\
static inline void ARRAY_##T##_dispose(ARRAY(T) *a) {\
	do_free(a->list, sizeof(T) * a->capacity);\
	a->size     = 0;\
	a->capacity = 0;\
	a->list     = NULL;\
}

#define DEF_ARRAY_T_OP(T) DEF_ARRAY_T(T);DEF_ARRAY_OP(T)

#define ARRAY_add(T, a, v)      ARRAY_##T##_add(a, v)
#define ARRAY_dispose(T, a)     ARRAY_##T##_dispose(a)
#define ARRAY_init(T, a)        ARRAY_init_##T (a)
#define ARRAY_n(a, n)  ((a).list[n])
#define ARRAY_size(a)  ((a).size)
#define ARRAY_init_1(T, a, e1) do {\
	ARRAY_init(T, a);\
	ARRAY_add(T, a, e1);\
} while(0)

#define FOR_EACH_ARRAY_(a, i)  for(i=0; i < ARRAY_size(a); ++i)
#define FOR_EACH_ARRAY(a, x, i) \
		for(i=0, x = ARRAY_n(a, i); i < ARRAY_size(a); x = ARRAY_n(a,(++i)))

/* struct gc */
#define HeapMng(kctx) ((memshare(kctx)->gcHeapMng))

struct SubHeap;
struct Segment;
union  AllocationBlock;
struct HeapManager;

typedef void BlkPtr;
typedef uintptr_t bitmap_t;
typedef struct SubHeap SubHeap;
typedef struct Segment Segment;
typedef struct HeapManager HeapManager;
typedef union  AllocationBlock AllocationBlock;

typedef struct BitPtr {
	uintptr_t idx;
	uintptr_t mask;
} BitPtr;

typedef struct AllocationPointer {
	BitPtr bitptrs[ SEGMENT_LEVEL];
	Segment *seg;
	BlkPtr *blkptr;
} AllocationPointer;

struct SubHeap {
	AllocationPointer p;
	int heap_klass;
	int isFull;
	Segment *freelist;
	Segment **seglist;
	int seglist_size;
	int seglist_max;
};

typedef Segment* SegmentPtr;
typedef void* VoidPtr;
DEF_ARRAY_T_OP(SegmentPtr);
DEF_ARRAY_T_OP(size_t);
DEF_ARRAY_T_OP(VoidPtr);

struct HeapManager {
	bitmap_t flags;
	SubHeap heaps[SUBHEAP_KLASS_MAX+1];
	Segment *segmentList;
	ARRAY(SegmentPtr) segment_pool_a;
	ARRAY(size_t)     segment_size_a;
	ARRAY(VoidPtr)    managed_heap_a;
	ARRAY(VoidPtr)    managed_heap_end_a;
	ARRAY(size_t)     heap_size_a;
};

struct Segment {
	bitmap_t *base[ SEGMENT_LEVEL];
	struct Segment *next;
	int live_count;
	int heap_klass;
	const AllocationBlock *blk;
	bitmap_t *bitmap;
	void *unused;
#if GCDEBUG
	void  *managed_heap;
	void  *managed_heap_end;
#endif
};

typedef struct BlockHeader {
	Segment *seg;
	int klass;
} BlockHeader;

typedef struct gc_stat {
	size_t total_object;
	size_t object_count[SUBHEAP_KLASS_MAX+1];
	size_t gc_count;
	size_t marked[SUBHEAP_KLASS_MAX+1];
	size_t collected[SUBHEAP_KLASS_MAX+1];
	size_t markingTime;
	size_t sweepingTime;
	size_t current_request_size;
	AllocationBlock *managed_heap;
	AllocationBlock *managed_heap_end;
	FILE *fp;
} bmgc_stat;

#ifdef GCSTAT
static bmgc_stat global_gc_stat = {};
#endif

#define DEF_BM(size)  struct bm##size { uintptr_t bm[size]; }
DEF_BM(  1);DEF_BM(  2);DEF_BM(  4);
DEF_BM(  8);DEF_BM( 16);DEF_BM( 32);
DEF_BM( 64);DEF_BM(128);DEF_BM(256);
#define BITMAP_L0_SIZE(N) (CEIL(((float)SEGMENT_SIZE)/KlassBlockSize(N)/BITS))
#define BITMAP_L1_SIZE(N) (CEIL(((float)SEGMENT_SIZE)/KlassBlockSize(N)/BITS/BITS))
#define BITMAP_L2_SIZE(N) (CEIL(((float)SEGMENT_SIZE)/KlassBlockSize(N)/BITS/BITS/BITS))

static const size_t SegmentBitMapCount[] = {
	0,0,0,0,0,
	BITMAP_L0_SIZE(5 ),
	BITMAP_L0_SIZE(6 ),
	BITMAP_L0_SIZE(7 ),
	BITMAP_L0_SIZE(8 ),
	BITMAP_L0_SIZE(9 ),
	BITMAP_L0_SIZE(10),
	BITMAP_L0_SIZE(11),
	BITMAP_L0_SIZE(12)
};

#if SIZEOF_VOIDP*8 == 64
struct BM5  { struct bm64 m0; struct bm1 S;struct bm1 m1;};
struct BM6  { struct bm32 m0; struct bm1 S;struct bm1 m1;};
struct BM7  { struct bm16 m0; struct bm1 S;struct bm1 m1;};
struct BM8  { struct bm8  m0; struct bm1 S;struct bm1 m1;};
struct BM9  { struct bm4  m0; struct bm1 S;struct bm1 m1;};
struct BM10 { struct bm2  m0; struct bm1 S;struct bm1 m1;};
struct BM11 { struct bm1  m0; struct bm1 S;};
struct BM12 { struct bm1  m0;};
#else
struct BM5  { struct bm128 m0; struct bm1 S;struct bm4 m1; struct bm1 m2;};
struct BM6  { struct bm64  m0; struct bm1 S;struct bm2 m1; struct bm1 m2;};
struct BM7  { struct bm32  m0; struct bm1 S;struct bm1 m1;};
struct BM8  { struct bm16  m0; struct bm1 S;struct bm1 m1;};
struct BM9  { struct bm8   m0; struct bm1 S;struct bm1 m1;};
struct BM10 { struct bm4   m0; struct bm1 S;struct bm1 m1;};
struct BM11 { struct bm2   m0; struct bm1 S;struct bm1 m1;};
struct BM12 { struct bm1   m0;};
#endif


#define _BLOCK_(size)  struct blk##size{uint8_t m[size];} \
	b##size [SEGMENT_SIZE/(sizeof(struct blk##size))]
union AllocationBlock {
	_BLOCK_(8   );_BLOCK_(16  );_BLOCK_(32  );_BLOCK_(64  );
	_BLOCK_(128 );_BLOCK_(256 );_BLOCK_(512 );_BLOCK_(1024);
	_BLOCK_(2048);_BLOCK_(4096);
};

#define SEGMENT_BLOCK_COUNT(n) ((n >= SUBHEAP_KLASS_MIN)?(SEGMENT_SIZE / KlassBlockSize(n ) - 1):0)
static const size_t SegmentBlockCount[] = {
	0, 0, 0,
	SEGMENT_BLOCK_COUNT(3 ), SEGMENT_BLOCK_COUNT(4 ),
	SEGMENT_BLOCK_COUNT(5 ), SEGMENT_BLOCK_COUNT(6 ),
	SEGMENT_BLOCK_COUNT(7 ), SEGMENT_BLOCK_COUNT(8 ),
	SEGMENT_BLOCK_COUNT(9 ), SEGMENT_BLOCK_COUNT(10),
	SEGMENT_BLOCK_COUNT(11), SEGMENT_BLOCK_COUNT(12),
};

#if SIZEOF_VOIDP*8 == 64
//#define BM_SENTINEL_L2_3  (BITMAP_FULL << (2))
//#define BM_SENTINEL_L2_4  (BITMAP_FULL << (1))
#define BM_SENTINEL_L2_5  (BITMAP_FULL)
#define BM_SENTINEL_L2_6  (BITMAP_FULL)
#define BM_SENTINEL_L2_7  (BITMAP_FULL)
#define BM_SENTINEL_L2_8  (BITMAP_FULL)
#define BM_SENTINEL_L2_9  (BITMAP_FULL)
#define BM_SENTINEL_L2_10 (BITMAP_FULL)
#define BM_SENTINEL_L2_11 (BITMAP_FULL)
#define BM_SENTINEL_L2_12 (BITMAP_FULL)

//#define BM_SENTINEL_L1_3  (BITMAP_FULL << (63))
//#define BM_SENTINEL_L1_4  (BITMAP_FULL << (63))
#define BM_SENTINEL_L1_5  (BITMAP_FULL << (63))
#define BM_SENTINEL_L1_6  (BITMAP_FULL << (32))
#define BM_SENTINEL_L1_7  (BITMAP_FULL << (16))
#define BM_SENTINEL_L1_8  (BITMAP_FULL << (8))
#define BM_SENTINEL_L1_9  (BITMAP_FULL << (2))
#define BM_SENTINEL_L1_10 (BITMAP_FULL << (1))
#define BM_SENTINEL_L1_11 (BITMAP_FULL)
#define BM_SENTINEL_L1_12 (BITMAP_FULL)

//#define BM_SENTINEL_L0_3  (BITMAP_FULL)
//#define BM_SENTINEL_L0_4  (BITMAP_FULL)
#define BM_SENTINEL_L0_5  (BITMAP_FULL)
#define BM_SENTINEL_L0_6  (BITMAP_FULL)
#define BM_SENTINEL_L0_7  (BITMAP_FULL)
#define BM_SENTINEL_L0_8  (BITMAP_FULL)
#define BM_SENTINEL_L0_9  (BITMAP_FULL)
#define BM_SENTINEL_L0_10 (BITMAP_FULL)
#define BM_SENTINEL_L0_11 (BITMAP_FULL)
#define BM_SENTINEL_L0_12 (BITMAP_FULL << (32))

#else

//#define BM_SENTINEL_L2_3  (BITMAP_FULL << (2))
//#define BM_SENTINEL_L2_4  (BITMAP_FULL << (1))
#define BM_SENTINEL_L2_5  (BITMAP_FULL << (1))
#define BM_SENTINEL_L2_6  (BITMAP_FULL << (1))
#define BM_SENTINEL_L2_7  (BITMAP_FULL)
#define BM_SENTINEL_L2_8  (BITMAP_FULL)
#define BM_SENTINEL_L2_9  (BITMAP_FULL)
#define BM_SENTINEL_L2_10 (BITMAP_FULL)
#define BM_SENTINEL_L2_11 (BITMAP_FULL)
#define BM_SENTINEL_L2_12 (BITMAP_FULL)

//#define BM_SENTINEL_L1_3  (BITMAP_FULL << (63))
//#define BM_SENTINEL_L1_4  (BITMAP_FULL << (63))
#define BM_SENTINEL_L1_5  (BITMAP_FULL << (31))
#define BM_SENTINEL_L1_6  (BITMAP_FULL << (16))
#define BM_SENTINEL_L1_7  (BITMAP_FULL << (8))
#define BM_SENTINEL_L1_8  (BITMAP_FULL << (4))
#define BM_SENTINEL_L1_9  (BITMAP_FULL << (2))
#define BM_SENTINEL_L1_10 (BITMAP_FULL << (1))
#define BM_SENTINEL_L1_11 (BITMAP_FULL)
#define BM_SENTINEL_L1_12 (BITMAP_FULL)

//#define BM_SENTINEL_L0_3  (BITMAP_FULL)
//#define BM_SENTINEL_L0_4  (BITMAP_FULL)
#define BM_SENTINEL_L0_5  (BITMAP_FULL)
#define BM_SENTINEL_L0_6  (BITMAP_FULL)
#define BM_SENTINEL_L0_7  (BITMAP_FULL)
#define BM_SENTINEL_L0_8  (BITMAP_FULL)
#define BM_SENTINEL_L0_9  (BITMAP_FULL)
#define BM_SENTINEL_L0_10 (BITMAP_FULL)
#define BM_SENTINEL_L0_11 (BITMAP_FULL)
#define BM_SENTINEL_L0_12 (BITMAP_FULL)
#endif

static bitmap_t bitmap_empty = BITMAP_FULL;
static bitmap_t *bitmap_dummy = &bitmap_empty;
static Segment segment_dummy = {};

#define DEF_BM_OP0(N, L0, L1, L2)\
static inline void BITPTRS_SET_BASE##N (bitmap_t **base, bitmap_t *bitmap)\
{\
	struct BM##N *bm = (struct BM##N *)bitmap;\
	base[0] = (bitmap_t*)(&bm->m0.bm);\
	base[1] = (bitmap_t*) bitmap_dummy;\
	base[2] = (bitmap_t*) bitmap_dummy;\
}\
static inline void BITMAP_SET_LIMIT##N (bitmap_t *const bitmap)\
{\
	bitmap[L0-1] = BM_SENTINEL_L0_##N;\
}

#define DEF_BM_OP1(N, L0, L1, L2)\
static inline void BITPTRS_SET_BASE##N (bitmap_t **base, bitmap_t *bitmap)\
{\
	struct BM##N *bm = (struct BM##N *)bitmap;\
	base[0] = (bitmap_t*)(&bm->m0.bm);\
	base[1] = (bitmap_t*)(&bm->m1.bm);\
	base[2] = (bitmap_t*)bitmap_dummy;\
}\
static inline void BITMAP_SET_LIMIT##N (bitmap_t *const bitmap)\
{\
	struct BM##N *bm = (struct BM##N *)bitmap;\
	bitmap[L0-1] = BM_SENTINEL_L0_##N;\
	bm->m1.bm[L1-1] = BM_SENTINEL_L1_##N;\
}

#define DEF_BM_OP2(N, L0, L1, L2)\
static inline void BITPTRS_SET_BASE##N (bitmap_t **base, bitmap_t *bitmap)\
{\
	struct BM##N *bm = (struct BM##N *)bitmap;\
	base[0] = (bitmap_t*)(&bm->m0.bm);\
	base[1] = (bitmap_t*)(&bm->m1.bm);\
	base[2] = (bitmap_t*)(&bm->m2.bm);\
}\
static inline void BITMAP_SET_LIMIT##N (bitmap_t *const bitmap)\
{\
	struct BM##N *bm = (struct BM##N *)bitmap;\
	bitmap[L0-1] = BM_SENTINEL_L0_##N;\
	bm->m1.bm[L1-1] = BM_SENTINEL_L1_##N;\
	bm->m2.bm[L2-1] = BM_SENTINEL_L2_##N;\
}

#define DEF_BM_OP0_(N, S) \
	DEF_BM_OP0(N, BITMAP_L0_SIZE(N)+S, BITMAP_L1_SIZE(N), BITMAP_L2_SIZE(N))
#define DEF_BM_OP1_(N, S) \
	DEF_BM_OP1(N, BITMAP_L0_SIZE(N)+S, BITMAP_L1_SIZE(N), BITMAP_L2_SIZE(N))
#define DEF_BM_OP2_(N, S) \
	DEF_BM_OP2(N, BITMAP_L0_SIZE(N)+S, BITMAP_L1_SIZE(N), BITMAP_L2_SIZE(N))
//DEF_BM_OP2( 3, 256+1/*sentinel*/, 4, 1);
//DEF_BM_OP2( 4, 128+1/*sentinel*/, 2, 1);
DEF_BM_OP1_( 5, 1/*sentinel*/);
DEF_BM_OP1_( 6, 1/*sentinel*/);
DEF_BM_OP1_( 7, 1/*sentinel*/);
DEF_BM_OP1_( 8, 1/*sentinel*/);
DEF_BM_OP1_( 9, 1/*sentinel*/);
DEF_BM_OP1_(10, 1/*sentinel*/);
DEF_BM_OP0_(11, 1/*sentinel*/);
DEF_BM_OP0_(12, 0);


#define COND(C, T, F) ((C) ? T : F)
#define _MASK_(N) {\
	COND(BITMAP_L0_SIZE(N) > 0, 1, 0),\
	COND(BITMAP_L1_SIZE(N) > 0, 1, 0),\
	COND(BITMAP_L2_SIZE(N) > 0, 1, 0)}
#define _MASK_NULL {}
static const bool BITMAP_DEFAULT_MASK[][SEGMENT_LEVEL] = {
	_MASK_NULL,
	_MASK_NULL,
	_MASK_NULL,
	_MASK_( 3), _MASK_( 4),
	_MASK_( 5), _MASK_( 6),
	_MASK_( 7), _MASK_( 8),
	_MASK_( 9), _MASK_(10),
	_MASK_(11), _MASK_(12),
};

typedef void (*fBITPTRS_SET_BASE)(bitmap_t *base[SEGMENT_LEVEL], bitmap_t *bm);
static void BITPTRS_SET_BASE_(bitmap_t *base[SEGMENT_LEVEL], bitmap_t *bm) {}
static const fBITPTRS_SET_BASE BITPTRS_SET_BASE[] = {
	BITPTRS_SET_BASE_, BITPTRS_SET_BASE_, BITPTRS_SET_BASE_,  BITPTRS_SET_BASE_,
	BITPTRS_SET_BASE_, BITPTRS_SET_BASE5, BITPTRS_SET_BASE6,  BITPTRS_SET_BASE7,
	BITPTRS_SET_BASE8, BITPTRS_SET_BASE9, BITPTRS_SET_BASE10, BITPTRS_SET_BASE11,
	BITPTRS_SET_BASE12
};

typedef void (*fBITMAP_SET_LIMIT)(bitmap_t *const bm);
static void BITMAP_SET_LIMIT_(bitmap_t *const bm) { (void)bm; }
static const fBITMAP_SET_LIMIT BITMAP_SET_LIMIT__[] = {
	BITMAP_SET_LIMIT_, BITMAP_SET_LIMIT_, BITMAP_SET_LIMIT_,  BITMAP_SET_LIMIT_,
	BITMAP_SET_LIMIT_, BITMAP_SET_LIMIT5, BITMAP_SET_LIMIT6,  BITMAP_SET_LIMIT7,
	BITMAP_SET_LIMIT8, BITMAP_SET_LIMIT9, BITMAP_SET_LIMIT10, BITMAP_SET_LIMIT11,
	BITMAP_SET_LIMIT12
};

static inline void BITMAP_SET_LIMIT(bitmap_t *const bitmap, size_t klass)
{
	BITMAP_SET_LIMIT__[klass](bitmap);
	BM_SET(bitmap[0], 1);
}

static inline void BITPTRS_INIT(BitPtr bitptrs[SEGMENT_LEVEL], Segment *seg, size_t klass)
{
	size_t i;
	BITPTRS_SET_BASE[klass](seg->base, seg->bitmap);
	for (i = 0; i < SEGMENT_LEVEL; ++i) {
		bitptrs[i].idx = 0;
		bitptrs[i].mask = BITMAP_DEFAULT_MASK[klass][i];
	}
}

static const size_t BM_SIZE[] = {
	0, 0, 0, 0, 0,
	sizeof(struct BM5),  sizeof(struct BM6),
	sizeof(struct BM7),  sizeof(struct BM8),
	sizeof(struct BM9),  sizeof(struct BM10),
	sizeof(struct BM11), sizeof(struct BM12),
};

#define AllocBitMap(n)      ((bitmap_t*)(do_malloc(BM_SIZE[n])))
#define DeleteBitMap(bm, n) ((do_free((void*)bm, BM_SIZE[n])))
#define ClearBitMap(bm, n)  (do_bzero((void*)bm, BM_SIZE[n]))

#if GCDEBUG
#define gc_info(fmt, ...)  fprintf(stderr, "(%s:%d) " fmt "\n" , __func__, __LINE__,  ## __VA_ARGS__)
#define gc_debug(fmt, ...) fprintf(stderr, "(%s:%d) " fmt "\n" , __func__, __LINE__, ## __VA_ARGS__)
#define gc_stat(fmt, ...)  gc_debug(fmt, ##__VA_ARGS__);\
	fprintf(global_gc_stat.fp, "(%s:%d) " fmt "\n" , __func__, __LINE__,  ## __VA_ARGS__)
#else
#define gc_info(fmt, ...)
#define gc_debug(fmt, ...)
#define gc_stat(fmt, ...)  fprintf(global_gc_stat.fp, "(%s:%d) " fmt "\n" , __func__, __LINE__,  ## __VA_ARGS__)
#endif

/* ------------------------------------------------------------------------ */
/* prototype declaration */

static kObject *bm_malloc_internal(KonohaContext *kctx, HeapManager *mng, size_t n);
void *bm_malloc(KonohaContext *kctx, size_t n);
void *bm_realloc(KonohaContext *kctx, void *ptr, size_t os, size_t ns);
void bm_free(KonohaContext *kctx, void *ptr, size_t n);
static void BMGC_dump(HeapManager *mng);
static void bitmapMarkingGC(KonohaContext *kctx, HeapManager *mng);
static void HeapManager_init(KonohaContext *kctx, HeapManager *mng, size_t heap_size);
static void HeapManager_delete(KonohaContext *kctx, HeapManager *mng);
static void HeapManager_final_free(KonohaContext *kctx, HeapManager *mng);
static inline void bmgc_Object_free(KonohaContext *kctx, kObject *o);
static bool findNextFreeBlock(AllocationPointer *p);
static HeapManager *BMGC_init(KonohaContext *kctx);
static void BMGC_exit(KonohaContext *kctx, HeapManager *mng);
#ifdef GCDEBUG
static kbool_t knh_isObject(KonohaContext *kctx, kObject *o);
#endif

typedef struct kmemlocal_t {
	kmodlocal_t   h;
	kObject     **queue;
	size_t        queue_capacity;
	size_t        queue_log2;
} kmemlocal_t;

typedef struct kmemshare_t {
	kmodshare_t h;
	HeapManager *gcHeapMng;
	size_t   gcObjectCount;
	size_t   usedMemorySize;
	size_t   markedObject;
	size_t   collectedObject;
	size_t   freeObjectCount;
	uint64_t latestGcTime;
	size_t   markingTime;
	size_t   gcCount;
	size_t   gcTime;
} kmemshare_t;

#define memlocal(kctx) ((kmemlocal_t*)((kctx)->modlocal[MOD_gc]))
#define memshare(kctx) ((kmemshare_t*)((kctx)->modshare[MOD_gc]))

/* ------------------------------------------------------------------------ */

static inline uint64_t knh_getTimeMilliSecond(void)
{
#if defined(K_USING_WINDOWS_)
	DWORD tickCount = GetTickCount();
	return (kint64_t)tickCount;
#elif defined(K_USING_POSIX_)
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#elif defined(K_USING_BTRON)
	/* FIXME: thread safety */
	static volatile int first = 1;
	static UW start = 0;
	UW current;
	if (first) {
		get_etm(&start);
		first = 0;
	}
	get_etm(&current);
	return (uint64_t)((current - start) & 0x7fffffff);
#else
	return 0;
#endif
}

/* ------------------------------------------------------------------------ */
/* malloc */

static void THROW_OutOfMemory(KonohaContext *kctx, size_t size)
{
	/* TODO */
	PLAT exit_i(EXIT_FAILURE);
}

static void *call_malloc_aligned(KonohaContext *kctx, size_t size, size_t align)
{
	void *block = NULL;
#if defined(HAVE_POSIX_MEMALIGN)
	int ret = posix_memalign(&block, align, size);
	if(ret != 0)
		goto L_OutOfMemory;
#elif defined(HAVE_MEMALIGN)
	block = memalign(align, size);
	if (unlikely(block == NULL))
		goto L_OutOfMemory;
#elif defined(K_USING_WINDOWS_)
	block = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (unlikely(block == NULL))
		goto L_OutOfMemory;
#else
	block = malloc(size + align);
	if (unlikely(block == NULL))
		goto L_OutOfMemory;
	if((uintptr_t)block % align != 0) {
		char *t2 = (char*)((((uintptr_t)block / align) + 1) * align);
		void **p = (void**)(t2 + size);
		DBG_ASSERT((char*)p < ((char*)block) + size + align);
		p[0] = block;
		block = (void*)t2;
		DBG_ASSERT((uintptr_t)block % align == 0);
	}
	else {
		void **p = (void**)((char*)block + size);
		p[0] = block;
	}
#endif
	return block;
	L_OutOfMemory:
	THROW_OutOfMemory(kctx, size);
	return NULL;
}
static void call_free_aligned(KonohaContext *kctx, void *block, size_t size)
{
#if defined(HAVE_POSIX_MEMALIGN) || defined(HAVE_MEMALIGN)
	free(block);
#elif defined(K_USING_WINDOWS_)
	VirtualFree(block, 0, MEM_RELEASE);
#else
	void **p = (void**)((char*)block + size);
	block = p[0];
	free(block);
#endif
}

#if GCDEBUG
static size_t malloced_size = 0;
#define DBG_CHECK_MALLOCED_SIZE()      DBG_ASSERT(malloced_size == 0)
#define DBG_CHECK_MALLOCED_INC_SIZE(n) (malloced_size += (n))
#define DBG_CHECK_MALLOCED_DEC_SIZE(n) (malloced_size -= (n))
#else
#define DBG_CHECK_MALLOCED_SIZE()
#define DBG_CHECK_MALLOCED_INC_SIZE(n)
#define DBG_CHECK_MALLOCED_DEC_SIZE(n)
#endif

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
	DBG_CHECK_MALLOCED_INC_SIZE(size);
	return ptr;
}

static inline void *do_realloc(void *ptr, size_t oldSize, size_t newSize)
{
	char *newptr = (char *) realloc(ptr, newSize);
	do_bzero(newptr+oldSize, newSize-oldSize);
	DBG_CHECK_MALLOCED_INC_SIZE(newSize - oldSize);
	return (void *) newptr;
}

static inline void do_free(void *ptr, size_t size)
{
#if GCDEBUG
	memset(ptr, 0xa, size);
#endif
	DBG_CHECK_MALLOCED_DEC_SIZE(size);
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
#if GCDEBUG
	ktrace(LOGPOL_DEBUG,
			KEYVALUE_s("@", "malloc"),
			KEYVALUE_p("from", p),
			KEYVALUE_p("to", ((char*)p)+s),
			KEYVALUE_u("size", s));
#endif
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
#if GCDEBUG
	ktrace(LOGPOL_DEBUG,
			KEYVALUE_s("@", "free"),
			KEYVALUE_p("from", p),
			KEYVALUE_p("to", ((char*)p)+s),
			KEYVALUE_u("size", s));
#endif
	kklib_malloced -= s;
}

void MODGC_check_malloced_size(void)
{
	if(verbose_gc) {
		fprintf(stdout, "\nklib:memory leaked=%ld\n", kklib_malloced);
#ifdef GCDEBUG
		DUMP_P("sys :memory leaked=%ld\n", malloced_size);
#endif
	}
}

/* ------------------------------------------------------------------------ */
static void kmodgc_local_reftrace(KonohaContext *kctx, struct kmodlocal_t *baseh) {}

static void kmodgc_local_free(KonohaContext *kctx, struct kmodlocal_t *baseh)
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

static void kmodgc_setup(KonohaContext *kctx, struct kmodshare_t *def, int newctx)
{
	if(memlocal(kctx) == NULL) {
		kmemlocal_t *base = do_malloc(sizeof(kmemlocal_t));
		do_bzero(base, sizeof(kmemlocal_t));
		base->h.reftrace = kmodgc_local_reftrace;
		base->h.free     = kmodgc_local_free;
		kctx->modlocal[MOD_gc] = (kmodlocal_t*)base;
	}
}

static void kmodgc_reftrace(KonohaContext *kctx, struct kmodshare_t *baseh) {}

static void kmodgc_free(KonohaContext *kctx, struct kmodshare_t *baseh)
{
	do_free(baseh, sizeof(kmemshare_t));
	kctx->modshare[MOD_gc] = NULL;
}

void MODGC_init(KonohaContext *kctx, KonohaContextVar *ctx)
{
	if(IS_RootKonohaContext(ctx)) {
		kmemshare_t *base = (kmemshare_t*) do_malloc(sizeof(kmemshare_t));
		base->h.name     = "bmgc";
		base->h.setup    = kmodgc_setup;
		base->h.reftrace = kmodgc_reftrace;
		// MODGC do not set 'free'
		base->h.free     = NULL;

		base->gcObjectCount = 0;
		base->latestGcTime  = knh_getTimeMilliSecond();
		base->gcHeapMng = BMGC_init(ctx);
		KSET_KLIB(malloc, 0);
		KSET_KLIB(zmalloc, 0);
		KSET_KLIB(free, 0);
		Konoha_setModule(MOD_gc, &base->h, 0);
	}
	kmodgc_setup(ctx, (kmodshare_t*) memshare(kctx), 1);
}

void MODGC_destoryAllObjects(KonohaContext *kctx, KonohaContextVar *ctx)
{
	BMGC_exit(ctx, memshare(ctx)->gcHeapMng);
	memshare((KonohaContextVar*)ctx)->gcHeapMng = NULL;
}

void MODGC_free(KonohaContext *kctx, KonohaContextVar *ctx)
{
	assert(memlocal(ctx) == NULL);
	if(IS_RootKonohaContext(ctx)) {
		kmodgc_free(kctx, (kmodshare_t*) memshare(kctx));
		Konoha_setModule(MOD_gc, NULL, 0);
	}
}

/* ------------------------------------------------------------------------ */
/* bmgc */
/* ------------------------------------------------------------------------ */

#define OBJECT_INIT(o) do {\
	o->h.magicflag = 0;\
	o->h.ct = NULL;\
	o->h.gcinfo = NULL;\
	o->fieldObjectItems[0] = NULL;\
} while(0)

kObject *MODGC_omalloc(KonohaContext *kctx, size_t size)
{
	kObjectVar *o = (kObjectVar*)bm_malloc_internal(kctx, HeapMng(kctx), size);
	OBJECT_INIT(o);
#if GCDEBUG
	ktrace(LOGPOL_DEBUG,
			KEYVALUE_s("@", "new"),
			KEYVALUE_p("ptr", o),
			KEYVALUE_u("size", size));
#endif
	return (kObject*)o;
}

/* ------------------------------------------------------------------------ */
/* [ostack] */

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
		size_t stacksize = sizeof(kObject*) * capacity;
		ostack->stack = (kObject**)do_realloc(ostack->stack, stacksize, stacksize * 2);
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
	if(likely(ostack->cur != ostack->tail)) {
		ostack->tail -=1;
		ref = ostack->stack[ostack->tail];
		prefetch_(ref, 0, 0);
	}
	return ref;
}

static void ostack_free(KonohaContext *kctx, knh_ostack_t *ostack)
{
	KonohaContextVar *wctx = (KonohaContextVar*) kctx;
	memlocal(wctx)->queue_capacity = ostack->capacity;
	memlocal(wctx)->queue = ostack->stack;
	memlocal(wctx)->queue_log2 = ostack->capacity_log2;
}

static HeapManager *BMGC_init(KonohaContext *kctx)
{
	HeapManager *mng = (HeapManager*) do_malloc(sizeof(*mng));
#ifdef GCSTAT
	global_gc_stat.fp = fopen("KONOHA_BMGC_INFO", "a");
#endif
	size_t default_size = SUBHEAP_DEFAULT_SEGPOOL_SIZE;
#ifdef GC_CONFIG
	char *poolsize = knh_getenv("KONOHA_DEFAULT_MEMPOOL_SIZE");
	if (poolsize) {
		kint_t tmp;
		if (knh_bytes_parseint(B(poolsize), &tmp))
			default_size = (size_t) tmp;
	}
#endif
	HeapManager_init(kctx, mng, default_size);
	return mng;
}

static void BMGC_exit(KonohaContext *kctx, HeapManager *mng)
{
	HeapManager_final_free(kctx, mng);
	HeapManager_delete(kctx, mng);
	do_free(mng, sizeof(*mng));
#ifdef GCSTAT
	fclose(global_gc_stat.fp);
#endif
}

static inline size_t SizeToKlass(size_t n) {
	size_t size = ALIGN(n, MIN_ALIGN);
	size_t size_w = size - 1;
	return (BITS - BSR(size_w));
}

#define SEGMENTLIST_NEXT(seg, list) do {\
	seg  = (list);\
	DBG_ASSERT(seg != NULL);\
	(list) = seg->next;\
	seg->next = NULL;\
} while(0)

#define BM_IS_FULL(BM) (~(BM) == 0)
#define SEG_BITMAP_N(seg, n, idx) ((bitmap_t*)((seg->base[n])+idx))
#define AP_BITMAP_N(ap, n, idx) ((bitmap_t*)((ap->seg->base[n])+idx))

static Segment *allocSegment(HeapManager *mng, int klass)
{
	Segment *seg = NULL;
	if (mng->segmentList) {
		SEGMENTLIST_NEXT(seg, mng->segmentList);
		gc_debug("klass=%d seg=%p mng->seg=%p", klass, seg, mng->segmentList);
	}
	return seg;
}
static void findBlockOfLastSegment(Segment *seg, SubHeap *h, size_t size)
{
	const AllocationBlock *blk = seg->blk;
	BlockHeader *head = (BlockHeader *) blk;
	head->seg   = seg;
	head->klass = seg->heap_klass;
	gc_info("seg=%p, blk=(%p,%p)", seg, blk, blk+1);
	h->p.blkptr = (AllocationBlock*)((char*)blk+(size));
}

static bool newSegment(HeapManager *mng, SubHeap *h)
{
	size_t klass = h->heap_klass;
	Segment *seg = allocSegment(mng, klass);
	DBG_ASSERT(h->freelist == NULL);

	if (!seg) return false;
	DBG_ASSERT(seg->live_count == 0);
	if (h->seglist_size == h->seglist_max) {
		size_t newSize, oldSize;
		oldSize = sizeof(Segment**)*h->seglist_max;
		newSize = sizeof(Segment**)*h->seglist_max * 2;
		h->seglist_max *= 2;
		h->seglist = (Segment**)(do_realloc(h->seglist, oldSize, newSize));
	}
	seg->bitmap = AllocBitMap(klass);
	seg->heap_klass = klass;
	h->seglist[h->seglist_size++] = seg;

	h->p.seg = seg;
	findBlockOfLastSegment(seg, h, KlassBlockSize(klass));
	BITPTRS_INIT(h->p.bitptrs, seg, klass);
	BITMAP_SET_LIMIT(seg->bitmap, klass);

	return true;
}

static inline Segment *freelist_pop(SubHeap *h)
{
	Segment *seg = h->freelist;
	h->freelist = seg->next;
	return seg;
}

static inline bool freelist_isEmpty(SubHeap *h)
{
	return (h->freelist == NULL);
}

static bool fetchSegment(SubHeap *h, size_t klass)
{
	Segment *seg;
	if (freelist_isEmpty(h))
		return false;
	seg = freelist_pop(h);
	h->p.seg = seg;
	BITPTRS_INIT(h->p.bitptrs, seg, klass);
	return true;
}

static bool nextSegment(HeapManager *mng, SubHeap *h, AllocationPointer *p)
{
	Segment *seg;
	while (h->freelist != NULL) {
		seg = freelist_pop(h);
		DBG_ASSERT(seg->live_count < SegmentBlockCount[h->heap_klass]);
		p->seg = seg;
		BITPTRS_INIT(h->p.bitptrs, seg, h->heap_klass);
		if (findNextFreeBlock(p)) {
			gc_info("h[%d], seg=%p", h->heap_klass, seg);
			return true;
		}
	}

	if (newSegment(mng, h)) {
		findNextFreeBlock(p);
		return true;
	}

	return false;
}

#define BP(p, n) ((p)->bitptrs[n])
static void BitPtr0_inc(AllocationPointer *p)
{
	uintptr_t bpidx  = BP(p, 0).idx;
	uintptr_t bpmask = BP(p, 0).mask;
	bitmap_t *bm = AP_BITMAP_N(p, 0, bpidx);
	BM_SET(*bm, bpmask);
	uintptr_t rot = bpmask >> (BITS - 1);

	BP(p, 0).idx  = bpidx + rot;
	BP(p, 0).mask = (bpmask << 1) | rot;
}

static void inc(AllocationPointer *p, SubHeap *h)
{
	int size = KlassBlockSize(h->heap_klass);
	p->blkptr = (AllocationBlock*)((char*)p->blkptr+size);
	BitPtr0_inc(p);
	p->seg->live_count++;
}

static bool isMarked(AllocationPointer *p)
{
	uintptr_t idx  = BP(p, 0).idx;
	uintptr_t mask = BP(p, 0).mask;
	bitmap_t *bm = AP_BITMAP_N(p, 0, idx);
	return (BM_TEST(*bm, mask));
}

#define BITPTR_INIT_(bpidx, bpmask, idx) do {\
	bpidx  = idx / BITS;\
	bpmask = ONE << (idx % BITS);\
} while(0)

static BitPtr *BitPtr_init(BitPtr *bp, uintptr_t idx)
{
	BITPTR_INIT_(bp->idx, bp->mask, idx);
	return bp;
}

static uintptr_t bitptrToIndex(uintptr_t bpidx, uintptr_t bpmask)
{
	return bpidx * BITS + FFS(bpmask) - 1;
}

static BlkPtr *blockAddress(Segment *s, uintptr_t idx, uintptr_t mask)
{
	size_t size = s->heap_klass;
	size_t offset = bitptrToIndex(idx, mask) << size;
	const BlkPtr *ptr = s->blk;
	return (AllocationBlock*)((char*)ptr+offset);
}

#define BP_NEXT_MASK(ap, bpidx, bpmask, j) do {\
	bitmap_t *bm   = AP_BITMAP_N(p, j, bpidx);\
	uintptr_t temp = *bm | (bpmask - ONE);\
	uintptr_t mask = (temp + ONE) & ~temp;\
	bpmask = mask;\
} while(0)

#if GCDEBUG
#define DBG_ALLOCATION_POINTER(p) do {\
	kObject *o = blockAddress(p->seg, BP(p, 0).idx, BP(p, 0).mask);\
	if (o->h.meta == NULL) {\
		fprintf(stderr, "o=%p, seg=%p\n", o, p->seg);\
		assert(o->h.meta == NULL);\
	}\
} while(0)
#else
#define DBG_ALLOCATION_POINTER(p)
#endif

static bool findNextFreeBlock(AllocationPointer *p)
{
	uintptr_t i, idx = BP(p, 0).idx;
	BP_NEXT_MASK(p, BP(p, 0).idx, BP(p, 0).mask, 0);
	if (BP(p, 0).mask == 0) {
		BitPtr *bp;
#if GCDEBUG
		gc_info("klass=%d, idx=%ld mask=%lx, seg=%p",
				(p->seg)?p->seg->heap_klass:-1,
				BP(p, 0).idx, BP(p, 0).mask, p->seg);
#endif
		for (i = 1; i < SEGMENT_LEVEL; ++i) {
			bp = BitPtr_init(&BP(p, i), idx);
			bitmap_t *bm = AP_BITMAP_N(p, i, bp->idx);
			BM_SET(*bm, bp->mask);
			BP_NEXT_MASK(p, bp->idx, bp->mask, i);
			if (bp->mask != 0)
				break;
			idx /= BITS;
		}
		if (i == SEGMENT_LEVEL)
			return false;
		do {
			--i;
			BP(p, i).idx  = bitptrToIndex(bp->idx, bp->mask);
			BP(p, i).mask = 1;
			BP_NEXT_MASK(p, BP(p, i).idx, BP(p,i).mask, i);
			gc_info("klass=%d, level=%lu idx=%ld mask=%lx",
					p->seg->heap_klass, i, BP(p, i).idx, BP(p, i).mask);
			DBG_ASSERT(BP(p, i).mask != 0);
			DBG_ALLOCATION_POINTER(p);
		} while(i > 0);
	}
	p->blkptr = blockAddress(p->seg, BP(p, 0).idx, BP(p, 0).mask);
	return true;
}

static void *tryAlloc(HeapManager *mng, SubHeap *h)
{
	AllocationPointer *p = &h->p;
	void *temp;
	if (isMarked(p)) {
		if (findNextFreeBlock(p) == false) {
			if (nextSegment(mng, h, p) == false) {
				return NULL;
			}
		}
	}
	temp = p->blkptr;
	prefetch_(temp, 0, 0);
	inc(p, h);
#define GC_SAFEPOINT(ctx) do {\
	ctx->stat->gcObjectCount -=1;\
	if(ctx->stat->gcObjectCount == 0) {\
		SAFEPOINT_SETGC(ctx);\
	}\
} while(0)

	return temp;
}

#define HEAP_SEGMENTLIST_INIT_SIZE 16
static bool Heap_init(HeapManager *mng, SubHeap *h, int klass)
{
	size_t i;

	h->heap_klass = klass;
	h->seglist_size = 0;
	h->seglist_max  = HEAP_SEGMENTLIST_INIT_SIZE;
	h->seglist  = (Segment**)(do_malloc(sizeof(Segment**)*h->seglist_max));
	h->freelist = NULL;
	h->p.bitptrs[0].mask = 1;
	h->p.seg = &segment_dummy;
	for (i = 0; i < SEGMENT_LEVEL; ++i) {
		h->p.seg->base[i] = (bitmap_t*)bitmap_dummy;
	}
	return true;
}

static void Heap_dispose(SubHeap *h)
{
	if (h->seglist) {
		do_free(h->seglist, sizeof(Segment**)*h->seglist_max);
	}
	do_bzero(h, sizeof(*h));
}

static Segment *SegmentPool_init(size_t size, AllocationBlock *blk)
{
	size_t i;
	Segment *pool = (Segment*)(do_malloc(sizeof(Segment) * size));
	Segment *seg  = pool;
	Segment *next = seg+1, *tail = pool + size - 1;
	for (i = 0; i < size; ++i, ++blk, ++next, ++seg) {
		seg->blk  = blk;
		seg->next = next;
#if GCDEBUG
		seg->managed_heap = blk;
		seg->managed_heap_end = blk+1;
#endif
		//gc_info("[%d] seg=%p, next=%p, blk=%p, %p", i, seg, next,
		//        seg->managed_heap, seg->managed_heap_end);
	}
	tail->next = NULL;
	return pool;
}

static void SegmentPool_dispose(Segment *pool, size_t size)
{
	Segment *seg = NULL;
	size_t i;

	for (i = 0; i < size; i++) {
		seg = pool + i;
		if (seg->bitmap) {
			DeleteBitMap(seg->bitmap, seg->heap_klass);
		}
	}
	do_free(pool, sizeof(Segment) * size);
}

static void HeapManager_expandHeap(KonohaContext *kctx, HeapManager *mng, size_t list_size)
{
	Segment *segment_pool;

	size_t heap_size = list_size * SEGMENT_SIZE;
	void *managed_heap = call_malloc_aligned(kctx, heap_size, SEGMENT_SIZE);
	void *managed_heap_end = (char*)managed_heap + heap_size;
	do_bzero(managed_heap, heap_size);
#if defined(GCDEBUG) && defined(GCSTAT)
	global_gc_stat.managed_heap = (AllocationBlock*) managed_heap;
	global_gc_stat.managed_heap_end = (AllocationBlock*) managed_heap_end;
#endif

	segment_pool = SegmentPool_init(list_size, (AllocationBlock*) managed_heap);
	mng->segmentList  = segment_pool;

	ARRAY_add(size_t,  &mng->heap_size_a, heap_size);
	ARRAY_add(VoidPtr, &mng->managed_heap_a    , (void*)managed_heap);
	ARRAY_add(VoidPtr, &mng->managed_heap_end_a, (void*)managed_heap_end);

	ARRAY_add(SegmentPtr, &mng->segment_pool_a, segment_pool);
	ARRAY_add(size_t, &mng->segment_size_a, list_size);
#ifdef K_USING_DEBUG
	memshare(kctx)->usedMemorySize += heap_size;
#endif
#ifdef GCSTAT
	gc_stat("Expand Heap(%luMB)[%p, %p]", heap_size/MB_, managed_heap, managed_heap_end);
#endif
}

static void HeapManager_init(KonohaContext *kctx, HeapManager *mng, size_t list_size)
{
	size_t i;
	SubHeap *h;
	ARRAY_init(size_t,  &mng->heap_size_a);
	ARRAY_init(VoidPtr, &mng->managed_heap_a);
	ARRAY_init(VoidPtr, &mng->managed_heap_end_a);
	ARRAY_init(SegmentPtr, &mng->segment_pool_a);
	ARRAY_init(size_t, &mng->segment_size_a);

	HeapManager_expandHeap(kctx, mng, list_size);
	for_each_heap(h, i, mng->heaps) {
		Heap_init(mng, (mng->heaps+i), i);
	}
}

static void HeapManager_delete(KonohaContext *kctx, HeapManager *mng)
{
	size_t i;
	Segment *x;
	void *p;
	SubHeap *h;
	for_each_heap(h, i, mng->heaps) {
		Heap_dispose(mng->heaps+i);
	}

	FOR_EACH_ARRAY(mng->segment_pool_a, x, i) {
		size_t size = ARRAY_n(mng->segment_size_a, i);
		SegmentPool_dispose(x, size);
	}
	ARRAY_dispose(SegmentPtr, &mng->segment_pool_a);
	ARRAY_dispose(size_t,     &mng->segment_size_a);

	FOR_EACH_ARRAY(mng->managed_heap_a, p, i) {
		size_t size = ARRAY_n(mng->heap_size_a, i);
		call_free_aligned(kctx, p, size);
#ifdef K_USING_DEBUG
		memshare(kctx)->usedMemorySize += size;
#endif
	}
	ARRAY_dispose(size_t,  &mng->heap_size_a);
	ARRAY_dispose(VoidPtr, &mng->managed_heap_a);
	ARRAY_dispose(VoidPtr, &mng->managed_heap_end_a);
}

static SubHeap *findSubHeapBySize(HeapManager *mng, size_t n)
{
	size_t klass = SizeToKlass(n);
	DBG_ASSERT(n <= SUBHEAP_KLASS_SIZE_MAX);
	DBG_ASSERT(n != 0);
	return &(mng->heaps)[klass];
}


#ifdef GCDEBUG
#define CLEAR_GCINFO(o) (o)->h.gcinfo = NULL
#else
#define CLEAR_GCINFO(o)
#endif

#if GCDEBUG
static bool CHECK_OBJECT(SubHeap *h, kObject *o, size_t request_size)
{
	short *gcinfo = (short*) &o->h.gcinfo;
	if (gcinfo[0] > 0) {
		short x = gcinfo[0];
		short y = gcinfo[1];
		fprintf(stderr, "size=%lu temp=%p, [0]=%d, [1]=%d\n",
				KlassBlockSize(h->heap_klass), gcinfo, x, y);
		return true;
	}
	return false;
}
#else
#define CHECK_OBJECT(h, o, request_size)
#endif

#if GCDEBUG
static void DBG_CHECK_OBJECT(SubHeap *h, kObject *o, size_t request_size, bool write)
{
	short *gcinfo = (short*) &o->h.gcinfo;
	if (CHECK_OBJECT(h, o, request_size)) {
		short x = gcinfo[0];
		short y = gcinfo[1];
		assert(x==0&&y==0);
	}
	if (write) {
		gcinfo[0] = global_gc_stat.object_count[h->heap_klass];
		gcinfo[1] = request_size;
	}
}
#else
#define DBG_CHECK_OBJECT(h, o, request_size, write)
#endif

#if GCDEBUG
static bool DBG_CHECK_OBJECT_IN_SEGMENT(kObject *o, Segment *seg)
{
	kObject *s = (kObject *) seg->managed_heap;
	kObject *e = (kObject *) seg->managed_heap_end;
	return (s < o && o < e);
}

static bool DBG_CHECK_OBJECT_IN_HEAP(kObject *o, SubHeap *h)
{
	Segment *seg = h->p.seg;
	if (DBG_CHECK_OBJECT_IN_SEGMENT(o, seg))
		return true;
	/* TODO */
	// search from seglist;
	return false;
}
#else
#define DBG_CHECK_OBJECT_IN_SEGMENT(o, seg) true
#define DBG_CHECK_OBJECT_IN_HEAP(o, h) true
#endif

static void deferred_sweep(KonohaContext *kctx, kObject *o)
{
#ifdef GC_USE_DEFERREDSWEEP
	memshare(kctx)->collectedObject++;
	bmgc_Object_free(kctx, o);
	CLEAR_GCINFO(o);
#else
	assert(O_ct(o) == NULL);
#endif
}

static kObject *bm_malloc_internal(KonohaContext *kctx, HeapManager *mng, size_t n)
{
	kObject *temp = NULL;
	SubHeap *h;

	DBG_ASSERT(n != 0);
#if GCDEBUG
	global_gc_stat.current_request_size = n;
#endif
	if (n > SUBHEAP_KLASS_SIZE_MAX)
		return do_malloc(n);
	h = findSubHeapBySize(mng, n);
	temp = tryAlloc(mng, h);

	if (unlikely(temp == NULL)) {
		bitmapMarkingGC(kctx, mng);
		temp = tryAlloc(mng, h);
		if (unlikely(temp == NULL)) {
			THROW_OutOfMemory(kctx, n);
		}
	}

	DBG_ASSERT(DBG_CHECK_OBJECT_IN_HEAP(temp, h));
#if GCDEBUG
	global_gc_stat.total_object++;
	global_gc_stat.object_count[h->heap_klass]++;
#endif
	deferred_sweep(kctx, temp);
	DBG_CHECK_OBJECT(h, temp, n, true);
	return temp;
}

static void clearAllBitMapsAndCount(HeapManager *mng, SubHeap *h)
{
	size_t i;

	for (i = 0; i < h->seglist_size; i++) {
		Segment *s = h->seglist[i];
		ClearBitMap(s->bitmap, h->heap_klass);
		BITMAP_SET_LIMIT(s->bitmap, h->heap_klass);
		gc_info("klass=%d, seg[%lu]=%p count=%d",
				s->heap_klass, i, s, s->live_count);
		s->live_count = 0;
	}
}

#define NEXT_MASK(bm, mask) do {\
	uintptr_t temp;\
	temp = bm | (mask - ONE);\
	mask = (temp + ONE) & ~temp;\
	BM_SET(bm, mask);\
} while(0)

static kObject *indexToAddr(Segment *seg, uintptr_t idx, uintptr_t mask)
{
	const BlkPtr *ptr = seg->blk;
	size_t size = seg->heap_klass;
	size_t n = idx * BITS + FFS(mask) - 1;
	size_t offset = n << size;
	return (kObject*)((char*)ptr+offset);
}

static void b0_final_sweep(KonohaContext *kctx, bitmap_t bm, size_t idx, Segment *seg)
{
	kObject *o;
	bitmap_t mask = 1;
	NEXT_MASK(bm, mask);
	while (mask) {
		o = indexToAddr(seg, idx, mask);
#if GCDEBUG
		if (O_ct(o)) {
			global_gc_stat.collected[seg->heap_klass] += 1;
		}
#endif
		bmgc_Object_free(kctx, o);
		NEXT_MASK(bm, mask);
	}
}

static void HeapManager_final_free(KonohaContext *kctx, HeapManager *mng)
{
	size_t i, j;
	SubHeap *h;
	for_each_heap(h, j, mng->heaps) {
		clearAllBitMapsAndCount(mng, h);
		for (i = 0; i < h->seglist_size; i++) {
			Segment *s = h->seglist[i];
			bitmap_t *bm0;
			bitmap_t *b0 = (bitmap_t *) s->base[0];
			bitmap_t *l0 = b0 + SegmentBitMapCount[j];
			for (bm0 = b0; bm0 < l0; ++bm0) {
				b0_final_sweep(kctx, *bm0, bm0 - b0, s);
			}
		}
		gc_info("heap[%d] collected %lu",
				h->heap_klass,
				global_gc_stat.collected[h->heap_klass]);
	}
}

#if GCDEBUG
static void dumpBM(uintptr_t bm)
{
	int i;
	uintptr_t mask;
	fprintf(stderr, "                 ");
	for (i = BITS-1; i >= 0; i--) {
		if (i %10 == 0) {
			fprintf(stderr, "%d", i / 10);
		} else {
			fprintf(stderr, " ");
		}
	}
	fprintf(stderr, "\n");
	fprintf(stderr, "%16lx ", bm);
	for (i = BITS-1; i >= 0; i--) {
		fprintf(stderr, "%d", i % 10);
	}
	fprintf(stderr, "\n                 ");
	for (mask = ONE << (BITS-1); mask; mask >>= 1) {
		fprintf(stderr, "%d", (bm & mask)?1:0);
	}
	fprintf(stderr, "\n");
}

enum heap_dump_mode {
	HEAP_DUMP_INFO,
	HEAP_DUMP_VERBOSE
};

static void Heap_dump(const SubHeap *h, enum heap_dump_mode mode)
{
	gc_info("klass[%2d] object_count=%lu segment_list=(%d) ",
			h->heap_klass, global_gc_stat.object_count[h->heap_klass],
			h->seglist_size);
	//for (i = 0; i < h->seglist_size; ++i) {
	//    fprintf(stderr, "seg[%d]=%p ", i, h->seglist[i]);
	//}
	//fprintf(stderr, "\n");
	//for (i = 0; i < SEGMENT_LEVEL; i++) {
	//    BitPtr p = h->p.bitptrs[i];
	//    fprintf(stderr, "bit_ptr[%lu]\nidx=%lu\n", i, p.idx);
	//    dumpBM(p.mask);
	//}
	//if (mode == HEAP_DUMP_VERBOSE) {
	//    fprintf(stderr, "current bitmap status\n");
	//    for (i = 0; i < SEGMENT_LEVEL; i++) {
	//        fprintf(stderr, "bitmap[%d]\n", i);
	//        uintptr_t idx = h->p.bitptrs[i].idx;
	//        bitmap_t *bm  = h->p.base[0]+idx;
	//        dumpBM(*bm);
	//    }
	//}
}

#endif /* GCDEBUG */

#ifdef GCDEBUG
static void BMGC_dump(HeapManager *mng)
{
	size_t i;
	gc_info("********************************");
	gc_info("* Heap Information");
	gc_info("********************************");
	gc_info("total allocated object count=%lu",
			global_gc_stat.total_object);
	for (i = SUBHEAP_KLASS_MIN; i <= SUBHEAP_KLASS_MAX; i++) {
	}
	//gc_info("HeapManager=%p segment_pool_size=%lu",
	//        mng, mng->segment_pool_size);
	for (i = SUBHEAP_KLASS_MIN; i <= SUBHEAP_KLASS_MAX; i++) {
		SubHeap *h = mng->heaps + i;
		Heap_dump(h, HEAP_DUMP_INFO);
	}
	gc_info("\n");
}

static bool DBG_CHECK_BITMAP(Segment *seg, bitmap_t *bm)
{
	bitmap_t *b0 = (bitmap_t *)seg->base[0];
	bitmap_t *l0 = (bitmap_t *)seg->base[0] + SegmentBitMapCount[seg->heap_klass];
	return (b0 <= bm && bm <= l0);
}
#else
static void BMGC_dump(HeapManager *info) {}
#define DBG_CHECK_BITMAP(seg, bm) true
#endif

static void bmgc_gc_init(KonohaContext *kctx, HeapManager *mng)
{
	size_t i;
	SubHeap *h;

	STAT_(memshare(kctx)->markedObject = 0;)
	BMGC_dump(mng);
	for_each_heap(h, i, mng->heaps) {
		clearAllBitMapsAndCount(mng, h);
	}
}

#define OBJECT_LOAD_BLOCK_INFO(o, seg, index, klass) do {\
	uintptr_t addr, offset;\
	addr   = ((uintptr_t)o) & ~(SEGMENT_SIZE - ONE);\
	offset = ((uintptr_t)o) &  (SEGMENT_SIZE - ONE);\
	BlockHeader *head = (BlockHeader*) addr;\
	seg   = head->seg;\
	klass = head->klass;\
	index = offset >> klass;\
} while(0)

static void bitmap_mark(bitmap_t bm, Segment *seg, uintptr_t idx, uintptr_t mask)
{
	if (BM_IS_FULL(bm)) {
		size_t i;
		for (i = 1; i < SEGMENT_LEVEL-1; ++i) {
			uintptr_t bpidx, bpmask;
			BITPTR_INIT_(bpidx, bpmask, idx);
			bitmap_t *bm1 = SEG_BITMAP_N(seg, i, bpidx);
			BM_SET(*bm1, bpmask);
			if (!BM_IS_FULL(*bm1))
				break;
			idx /= BITS;
		}
	}
}

static void mark_ostack(KonohaContext *kctx, HeapManager *mng, kObject *o, knh_ostack_t *ostack)
{
	Segment *seg;
	int index, klass;
	uintptr_t bpidx, bpmask;
	OBJECT_LOAD_BLOCK_INFO(o, seg, index, klass);
	BITPTR_INIT_(bpidx, bpmask, index);
	bitmap_t *bm  = SEG_BITMAP_N(seg, 0, bpidx);
	prefetch_(SEG_BITMAP_N(seg, 1, 0), 1, 1);

	DBG_ASSERT(DBG_CHECK_OBJECT_IN_SEGMENT(o, seg));
	DBG_ASSERT(DBG_CHECK_BITMAP(seg, bm));
	if (!BM_TEST(*bm, bpmask)) {
		BM_SET(*bm, bpmask);
		bitmap_mark(*bm, seg, bpidx, bpmask);
		++(seg->live_count);
		ostack_push(kctx, ostack, o);
		STAT_(++(memshare(kctx)->markedObject););
#ifdef GCSTAT
		global_gc_stat.marked[klass]++;
#endif
#if GCDEBUG
		o->h.meta = (void*)((uintptr_t)o->h.meta + 0x1);
#endif
	}
}

#define context_reset_refs(kctx) kctx->stack->reftail = kctx->stack->ref.refhead

static void bmgc_gc_mark(KonohaContext *kctx, HeapManager *mng)
{
	long i;
	knh_ostack_t ostackbuf, *ostack = ostack_init(kctx, &ostackbuf);
	KonohaLocalRuntimeVar *stack = kctx->stack;
	kObject *ref = NULL;

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
				mark_ostack(kctx, mng, stack->ref.refhead[i], ostack);
			}
		}
	}
	ostack_free(kctx, ostack);
}

void *bm_malloc(KonohaContext *kctx, size_t n)
{
	HeapManager *mng = HeapMng(kctx);
	return (void *) bm_malloc_internal(kctx, mng, n);
}

void bm_free(KonohaContext *kctx, void *ptr, size_t n)
{
	if (n <= SUBHEAP_KLASS_SIZE_MAX) {
		kObject *o = (kObject *) ptr;
		Segment *seg;
		uintptr_t bpidx, bpmask, index, klass;
		OBJECT_LOAD_BLOCK_INFO(o, seg, index, klass);
		BITPTR_INIT_(bpidx, bpmask, index);
		bitmap_t *bm = SEG_BITMAP_N(seg, 0, bpidx);
		DBG_ASSERT(DBG_CHECK_OBJECT_IN_SEGMENT(o, seg));
		DBG_ASSERT(DBG_CHECK_BITMAP(seg, bm));
		DBG_ASSERT(BM_TEST(*bm, bpmask));
		seg->live_count -= 1;
		// TODO(); FIXME
		bitmap_mark(*bm, seg, bpidx, bpmask);
		CLEAR_GCINFO((kObject*)ptr);
	} else {
		do_free(ptr, n);
	}
}

void *bm_realloc(KonohaContext *kctx, void *ptr, size_t os, size_t ns)
{
	HeapManager *mng = HeapMng(kctx);
	if(os <= K_FASTMALLOC_SIZE) {
		void *newptr = (void *) bm_malloc_internal(kctx, mng, ns);
		if(os > 0) {
			do_memcpy(newptr, ptr, os);
			do_bzero((char*)newptr + os, ns - os);
			bm_free(kctx, ptr, os);
		}
		else {
			DBG_ASSERT(ptr == NULL);
			do_bzero(newptr, ns);
		}
		return newptr;
	}
	else {
		void *newptr = realloc(ptr, ns);
		DBG_ASSERT(ns > os);
		if (unlikely(newptr == NULL)) {
			THROW_OutOfMemory(kctx, ns);
		}
		do_bzero((char*)newptr + os, (ns - os));
		return newptr;
	}
}

#define LIST_PUSH(tail, e) do {\
	*tail = e;\
	tail  = &e->next;\
} while (0)

static bool rearrangeSegList(KonohaContext *kctx, SubHeap *h, size_t klass)
{
	size_t i, count_dead = 0;
	Segment *unfilled = NULL, **unfilled_tail = &unfilled;

	if (h->seglist_size < 1)
		return false;
	for (i = 0; i < h->seglist_size; i++) {
		Segment *s = h->seglist[i];
		size_t dead = SegmentBlockCount[klass] - s->live_count;
		count_dead += dead;
		if (dead > 0)
			LIST_PUSH(unfilled_tail, s);
	}
	*unfilled_tail = NULL;
	h->freelist = unfilled;
	fetchSegment(h, klass);
	h->isFull = (count_dead < SegmentBlockCount[klass] && h->freelist == NULL);
	return h->isFull;
}

static void bmgc_gc_sweep(KonohaContext *kctx, HeapManager *mng)
{
	bool isFull = false;
	size_t i, j;
	SubHeap *h;

	for_each_heap(h, j, mng->heaps) {
#ifndef GC_USE_DEFERREDSWEEP
		for (i = 0; i < h->seglist_size; i++) {
			Segment *s = h->seglist[i];
			bitmap_t *bm0;
			bitmap_t *b0 = (bitmap_t *) s->base[0];
			bitmap_t *l0 = b0 + SegmentBitMapCount[j];
			for (bm0 = b0; bm0 < l0; ++bm0) {
				b0_final_sweep(kctx, *bm0, bm0 - b0, s);
			}
		}
		gc_info("heap[%d] collected %lu",
				h->heap_klass,
				global_gc_stat.collected[h->heap_klass]);
#endif
		isFull |= rearrangeSegList(kctx, h, j);
	}

	if (isFull) {
		HeapManager_expandHeap(kctx, mng, SUBHEAP_DEFAULT_SEGPOOL_SIZE*2);
		for_each_heap(h, i, mng->heaps) {
			if (h->isFull)
				newSegment(mng, h);
		}
	}
}

static void bitmapMarkingGC(KonohaContext *kctx, HeapManager *mng)
{
	DBG_P("GC starting");
	bmgc_gc_init(kctx, mng);
#ifdef GCSTAT
	size_t i = 0, marked = 0, collected = 0, heap_size = 0;
	FOR_EACH_ARRAY_(mng->heap_size_a, i) {
		heap_size += ARRAY_n(mng->heap_size_a, i);
	}
#endif
	STAT_(
		uint64_t start_time = knh_getTimeMilliSecond();
		uint64_t mark_time = 0;
	);

	bmgc_gc_mark(kctx, mng);

	STAT_(mark_time = knh_getTimeMilliSecond());

	bmgc_gc_sweep(kctx, HeapMng(kctx));

	STAT_(
		memshare(kctx)->gcCount++;
		memshare(kctx)->markingTime += (mark_time-start_time);
		memshare(kctx)->latestGcTime = knh_getTimeMilliSecond();
		memshare(kctx)->gcTime += (memshare(kctx)->latestGcTime - start_time);
		GC_LOG("GC(%dMb): marked:%d, collected:%d marking_time=%dms",
			memshare(kctx)->usedMemorySize/ MB_,
			memshare(kctx)->markedObject, memshare(kctx)->collectedObject, 
			(int)(mark_time - start_time));
		memshare(kctx)->collectedObject = 0;
		);

#ifdef GCSTAT
	SubHeap *h;
	for_each_heap(h, i, mng->heaps) {
		marked    += global_gc_stat.marked[i];
		collected += global_gc_stat.collected[i];
		global_gc_stat.marked[i]    = 0;
		global_gc_stat.collected[i] = 0;
	}
	global_gc_stat.gc_count += 1;
	gc_stat("GC(%lu) HeapSize=%luMB, last_collected=%lu, marked=%lu",
			global_gc_stat.gc_count, (heap_size/MB_), collected, marked);
#endif
}

/* ------------------------------------------------------------------------ */
/* [Object] */

#ifdef GCDEBUG
static kbool_t knh_isObject(KonohaContext *kctx, kObject *o)
{
	HeapManager *mng = (HeapManager*) HeapMng(kctx);

	size_t i;
	if ((uintptr_t) o % KlassBlockSize(SUBHEAP_KLASS_MIN) != 0)
		return false;
	FOR_EACH_ARRAY_(mng->managed_heap_a, i) {
		kObject *s = (kObject *) ARRAY_n(mng->managed_heap_a, i);
		kObject *e = (kObject *) ARRAY_n(mng->managed_heap_end_a, i);
		if (s < o && o < e) {
			Segment *seg;
			uintptr_t klass, index;
			OBJECT_LOAD_BLOCK_INFO(o, seg, index, klass);
			DBG_ASSERT((uintptr_t) o % KlassBlockSize(klass) == 0);
			uintptr_t bpidx, bpmask;
			BITPTR_INIT_(bpidx, bpmask, index);
			bitmap_t *bm = SEG_BITMAP_N(seg, 0, bpidx);
			if (BM_TEST(*bm, bpmask)) {
				return true;
			}
		}
	}
	return false;
}
#endif

/* ------------------------------------------------------------------------ */

#define K_OZERO(o) ((kObjectVar*)o)->h.ct = NULL
static inline void bmgc_Object_free(KonohaContext *kctx, kObject *o)
{
	KonohaClass *ct = O_ct(o);
	if (ct) {
#if GCDEBUG
		ktrace(LOGPOL_DEBUG,
				KEYVALUE_s("@", "delete"),
				KEYVALUE_p("ptr", o),
				KEYVALUE_u("size", ct->cstruct_size),
				KEYVALUE_u("cid", ct->cid));
		MEMLOG(ctx, "~Object", K_NOTICE, KNH_LDATA(LOG_p("ptr", o), LOG_i("cid", ct->cid)));
#endif
		//gc_info("~Object ptr=%p, cid=%d, o->h.meta=%p", o, ct->cid, o->h.meta);
		KONOHA_freeObjectField(kctx, (kObjectVar*)o);
		//ctx->stat->gcObjectCount += 1;
		K_OZERO(o);
		STAT_dObject(ctx, ct);
#ifdef GCSTAT
		Segment *seg;
		uintptr_t klass, index;
		OBJECT_LOAD_BLOCK_INFO(o, seg, index, klass);
		global_gc_stat.collected[seg->heap_klass] += 1;
#endif
	}
}

static bool stop_the_world(KonohaContext *kctx)
{
	return true;
}

static bool start_the_world(KonohaContext *kctx)
{
	return true;
}

/* ------------------------------------------------------------------------ */

void MODGC_gc_invoke(KonohaContext *kctx, int needsCStackTrace)
{
	uint64_t start_time = knh_getTimeMilliSecond(), mark_time = 0, intval;
	if(stop_the_world(kctx)) {
#if GCDEBUG
		ktrace(LOGPOL_DEBUG, KEYVALUE_s("@", "gc_start"));
#endif
		bmgc_gc_init(kctx, HeapMng(kctx));
		bmgc_gc_mark(kctx, HeapMng(kctx));
		mark_time = knh_getTimeMilliSecond();
		start_the_world(kctx);
	}
	bmgc_gc_sweep(kctx, HeapMng(kctx));
	intval = start_time - memshare(kctx)->latestGcTime;
	memshare(kctx)->gcCount++;
	memshare(kctx)->markingTime += (mark_time-start_time);
	memshare(kctx)->latestGcTime = knh_getTimeMilliSecond();
	memshare(kctx)->gcTime += (memshare(kctx)->latestGcTime - start_time);
	memshare(kctx)->collectedObject = 0;
#if GCDEBUG
	ktrace(LOGPOL_DEBUG,
			KEYVALUE_s("@", "gc_finish"),
			KEYVALUE_u("markingTime", (mark_time-start_time)),
			KEYVALUE_u("gcTime", (memshare(kctx)->latestGcTime - start_time)));
#endif
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
#endif /* BMGC_H_ */
