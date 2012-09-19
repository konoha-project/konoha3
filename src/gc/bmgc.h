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
#include <sys/time.h>

#include "minikonoha/minikonoha.h"
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

//#define MEMORY_DEBUG 1

/* memory config */

#define GC_USE_DEFERREDSWEEP 1
#define USE_SAFEPOINT_POLICY 1
#define SUBHEAP_DEFAULT_SEGPOOL_SIZE (128)/* 128 * SEGMENT_SIZE(128k) = 16MB*/
#define SUBHEAP_KLASS_MIN  5 /* 1 <<  5 == 32 */
#define SUBHEAP_KLASS_MAX 12 /* 1 << 12 == 4096 */
#define SEGMENT_LEVEL 3
#define MIN_ALIGN (1UL << SUBHEAP_KLASS_MIN)

#ifdef USE_GENERATIONAL_GC
#define MINOR_COUNT 16
#endif

#define KB_   (1024)
#define MB_   (KB_*1024)

#define SEGMENT_SIZE (128 * KB_)
#define PTR_SIZE (sizeof(void*))
#define BITS     (PTR_SIZE * 8)
#define PowerOf2(N) (1UL << N)
#define ALIGN(X,N)  (((X)+((N)-1))&(~((N)-1)))
#define CEIL(F)     (F-(int)(F) > 0 ? (int)(F+1) : (int)(F))
#if _WIN64
#define FFS(n) __builtin_ffsll(n)
#define CLZ(n) __builtin_clzll(n)
#define CTZ(x) __builtin_ctzll(x)
#else
#define FFS(n) __builtin_ffsl(n)
#define CLZ(n) __builtin_clzl(n)
#define CTZ(x) __builtin_ctzl(x)
#endif

#define BITMAP_FULL ((uintptr_t)(-1))
#define SUBHEAP_KLASS_SIZE_MIN PowerOf2(SUBHEAP_KLASS_MIN)
#define SUBHEAP_KLASS_SIZE_MAX PowerOf2(SUBHEAP_KLASS_MAX)
#define BM_SET(m, mask)  (m |= mask)
#define BM_TEST(m, mask) (m  & mask)

#ifndef unlikely
#define unlikely(x)   __builtin_expect(!!(x), 0)
#endif

#ifndef likely
#define likely(x)     __builtin_expect(!!(x), 1)
#endif

#define prefetch_(addr, rw, locality) __builtin_prefetch(addr, rw, locality)

#if SIZEOF_VOIDP*8 == 64
#define PREFIX_x "lx"
#define PREFIX_d "ld"
#else
#define PREFIX_x "x"
#define PREFIX_d "d"
#endif

static inline void *do_malloc(size_t size);
static inline void *do_calloc(size_t count, size_t size);
static inline void *do_realloc(void *ptr, size_t oldSize, size_t newSize);
static inline void  do_free(void *ptr, size_t size);
static inline void  do_bzero(void *ptr, size_t size);

/* ARRAY template */

#define ARRAY(T) ARRAY_##T##_t
#define DEF_ARRAY_STRUCT_(T, SizeTy) \
struct ARRAY(T) {\
	T *list;\
	SizeTy size;\
	SizeTy capacity;\
}
#define DEF_ARRAY_STRUCT(T)  DEF_ARRAY_STRUCT_(T, int)

#define DEF_ARRAY_T(T)\
struct ARRAY(T);\
typedef struct ARRAY(T) ARRAY(T);\
DEF_ARRAY_STRUCT(T)

#define DEF_ARRAY_OP(T)\
static inline ARRAY(T) *ARRAY_init_##T (ARRAY(T) *a) {\
	a->list = (T*) do_malloc(4 * sizeof(T));\
	a->capacity  = 4;\
	a->size  = 0;\
	return a;\
}\
static inline T ARRAY_##T##_get(ARRAY(T) *a, int idx) {\
	return a->list[idx];\
}\
static inline void ARRAY_##T##_set(ARRAY(T) *a, int idx, T v){ \
	a->list[idx] = v;\
}\
static inline void ARRAY_##T##_add(ARRAY(T) *a, T v) {\
	if (a->size + 1 >= a->capacity) {\
		size_t os = sizeof(T) * a->capacity;\
		a->capacity *= 2;\
		a->list = (T*)do_realloc(a->list, os, sizeof(T) * a->capacity);\
	}\
	ARRAY_##T##_set(a, a->size++, v);\
}\
static inline void ARRAY_##T##_dispose(ARRAY(T) *a) {\
	do_free(a->list, sizeof(T) * a->capacity);\
	a->size     = 0;\
	a->capacity = 0;\
	a->list     = NULL;\
}\
static inline void ARRAY_##T##_clear(ARRAY(T) *a) {\
	do_bzero(a->list, sizeof(T) * a->size);\
	a->size = 0;\
}

#define DEF_ARRAY_T_OP(T) DEF_ARRAY_T(T);DEF_ARRAY_OP(T)

#define ARRAY_add(T, a, v)      ARRAY_##T##_add(a, v)
#define ARRAY_dispose(T, a)     ARRAY_##T##_dispose(a)
#define ARRAY_init(T, a)        ARRAY_init_##T (a)
#define ARRAY_clear(T, a)       ARRAY_##T##_clear(a)
#define ARRAY_n(a, n)  ((a).list[n])
#define ARRAY_size(a)  ((a).size)
#define ARRAY_init_1(T, a, e1) do {\
	ARRAY_init(T, a);\
	ARRAY_add(T, a, e1);\
} while (0)

#define FOR_EACH_ARRAY_(a, i)  for(i=0; i < ARRAY_size(a); ++i)
#define FOR_EACH_ARRAY(a, x, i) \
		for(i=0, x = ARRAY_n(a, i); i < ARRAY_size(a); x = ARRAY_n(a,(++i)))

/* [bitmap ops] */
typedef uintptr_t bitmap_t;

static inline void bitmap_reset(bitmap_t *bm, bitmap_t val)
{
	*bm = val;
}

static inline int bitmap_get(bitmap_t *bm, unsigned index)
{
	bitmap_t mask = 1UL << (index % BITS);
	return (*bm & mask) != 0;
}

static inline void bitmap_set(bitmap_t *bm, unsigned index, unsigned val)
{
	bitmap_t mask = (uintptr_t)val << (index % BITS);
	*bm |= mask;
}

static inline void bitmap_flip(bitmap_t *bm, unsigned index)
{
	bitmap_t mask = 1UL << (index % BITS);
	*bm ^= mask;
}

/* [BMGC Deta Structure] */

#define HeapManager(kctx) ((memlocal(kctx)->gcHeapManager))

struct SubHeap;
struct Segment;
union  AllocationBlock;
struct HeapManager;

typedef void BlockPtr;
typedef struct SubHeap SubHeap;
typedef struct Segment Segment;
typedef struct HeapManager HeapManager;
typedef union  AllocationBlock AllocationBlock;

typedef struct BitPtr {
	uintptr_t idx;
	uintptr_t mask;
} BitPtr;

typedef struct AllocationPointer {
	BitPtr bitptrs[SEGMENT_LEVEL];
	Segment *seg;
	BlockPtr *blockptr;
} AllocationPointer;

struct SubHeap {
	AllocationPointer p;
	int heap_klass;
#ifdef USE_GENERATIONAL_GC
	int minor_count;
#endif
	Segment *freelist;
	Segment **seglist;
	int seglist_size;
	int seglist_max;
};

#define for_each_heap(H, I, HEAPS) \
	for (I = SUBHEAP_KLASS_MIN, H = (HEAPS)+I; I <= SUBHEAP_KLASS_MAX; ++H, ++I)

typedef Segment *SegmentPtr;
typedef void *VoidPtr;
typedef kObject *ObjectPtr;
typedef bitmap_t *BitMapPtr;
DEF_ARRAY_T_OP(SegmentPtr);
DEF_ARRAY_T_OP(size_t);
DEF_ARRAY_T_OP(VoidPtr);
DEF_ARRAY_T_OP(ObjectPtr);
DEF_ARRAY_T_OP(BitMapPtr);

struct HeapManager {
	long flags;
	SubHeap heaps[SUBHEAP_KLASS_MAX+1];
#ifdef USE_GENERATIONAL_GC
	ARRAY(BitMapPtr)  remember_sets;
#endif
	Segment *segmentList;
	ARRAY(SegmentPtr) segment_pool_a;
	ARRAY(size_t)     segment_size_a;
	ARRAY(VoidPtr)    managed_heap_a;
	ARRAY(VoidPtr)    managed_heap_end_a;
	ARRAY(size_t)     heap_size_a;
#if defined(USE_GENERATIONAL_GC) && defined(GCDEBUG)
	ARRAY(ObjectPtr)  remember_set_debug;
#endif
};

struct Segment {
	bitmap_t *base[SEGMENT_LEVEL];
	const AllocationBlock *block;
	int heap_klass;
	int live_count;
	struct Segment *next;
#ifdef USE_GENERATIONAL_GC
	bitmap_t *snapshots[SEGMENT_LEVEL];
	int tenure_live_count;
	bitmap_t *remember_set; /* for debug */
#else
	void *unused;
#endif
#if GCDEBUG
	void  *managed_heap;
	void  *managed_heap_end;
#endif
};

typedef struct BlockHeader {
	Segment *seg;
	long klass;
#ifdef USE_GENERATIONAL_GC
	bitmap_t *remember_set;
#endif
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
#define BITMAP_L0_SIZE(N) (CEIL(((float)SEGMENT_SIZE)/PowerOf2(N)/BITS))
#define BITMAP_L1_SIZE(N) (CEIL(((float)SEGMENT_SIZE)/PowerOf2(N)/BITS/BITS))
#define BITMAP_L2_SIZE(N) (CEIL(((float)SEGMENT_SIZE)/PowerOf2(N)/BITS/BITS/BITS))

static const unsigned SegmentBitMapCount[] = {
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
struct BM5  { struct bm128 m0; struct bm1 S;struct bm4 m1; struct bm1 S2; struct bm1 m2;};
struct BM6  { struct bm64  m0; struct bm1 S;struct bm2 m1; struct bm1 S2; struct bm1 m2;};
struct BM7  { struct bm32  m0; struct bm1 S;struct bm1 m1; struct bm1 S2;};
struct BM8  { struct bm16  m0; struct bm1 S;struct bm1 m1;};
struct BM9  { struct bm8   m0; struct bm1 S;struct bm1 m1;};
struct BM10 { struct bm4   m0; struct bm1 S;struct bm1 m1;};
struct BM11 { struct bm2   m0; struct bm1 S;struct bm1 m1;};
struct BM12 { struct bm1   m0;};
#endif

#define _BLOCK_(size)  struct block##size{uint8_t m[size];} \
	b##size [SEGMENT_SIZE/(sizeof(struct block##size))]
union AllocationBlock {
	_BLOCK_(8   );_BLOCK_(16  );_BLOCK_(32  );_BLOCK_(64  );
	_BLOCK_(128 );_BLOCK_(256 );_BLOCK_(512 );_BLOCK_(1024);
	_BLOCK_(2048);_BLOCK_(4096);
};

#define SEGMENT_BLOCK_COUNT(n) ((n >= SUBHEAP_KLASS_MIN)?(SEGMENT_SIZE / PowerOf2(n ) - 1):0)
static const unsigned SegmentBlockCount[] = {
	0, 0, 0,
	SEGMENT_BLOCK_COUNT(3 ), SEGMENT_BLOCK_COUNT(4 ),
	SEGMENT_BLOCK_COUNT(5 ), SEGMENT_BLOCK_COUNT(6 ),
	SEGMENT_BLOCK_COUNT(7 ), SEGMENT_BLOCK_COUNT(8 ),
	SEGMENT_BLOCK_COUNT(9 ), SEGMENT_BLOCK_COUNT(10),
	SEGMENT_BLOCK_COUNT(11), SEGMENT_BLOCK_COUNT(12),
};

#define MARGINE 0.975
static const unsigned int SegmentBlockCount_GC_MARGIN[] = {
	0, 0, 0,
	CEIL(SEGMENT_BLOCK_COUNT(3 )*MARGINE), CEIL(SEGMENT_BLOCK_COUNT(4 )*MARGINE),
	CEIL(SEGMENT_BLOCK_COUNT(5 )*MARGINE), CEIL(SEGMENT_BLOCK_COUNT(6 )*MARGINE),
	CEIL(SEGMENT_BLOCK_COUNT(7 )*MARGINE), CEIL(SEGMENT_BLOCK_COUNT(8 )*MARGINE),
	CEIL(SEGMENT_BLOCK_COUNT(9 )*MARGINE), CEIL(SEGMENT_BLOCK_COUNT(10)*MARGINE),
	CEIL(SEGMENT_BLOCK_COUNT(11)*MARGINE), CEIL(SEGMENT_BLOCK_COUNT(12)*MARGINE),
};

static bitmap_t bitmap_empty = BITMAP_FULL;
static bitmap_t *bitmap_dummy = &bitmap_empty;
static Segment segment_dummy = {};

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
//#define BM_SENTINEL_L2_3  (BITMAP_FULL << (16))
//#define BM_SENTINEL_L2_4  (BITMAP_FULL << (8))
#define BM_SENTINEL_L2_5  (BITMAP_FULL << (4))
#define BM_SENTINEL_L2_6  (BITMAP_FULL << (2))
#define BM_SENTINEL_L2_7  (BITMAP_FULL << (1))
#define BM_SENTINEL_L2_8  (BITMAP_FULL)
#define BM_SENTINEL_L2_9  (BITMAP_FULL)
#define BM_SENTINEL_L2_10 (BITMAP_FULL)
#define BM_SENTINEL_L2_11 (BITMAP_FULL)
#define BM_SENTINEL_L2_12 (BITMAP_FULL)

//#define BM_SENTINEL_L1_3  (BITMAP_FULL)
//#define BM_SENTINEL_L1_4  (BITMAP_FULL)
#define BM_SENTINEL_L1_5  (BITMAP_FULL)
#define BM_SENTINEL_L1_6  (BITMAP_FULL)
#define BM_SENTINEL_L1_7  (BITMAP_FULL)
#define BM_SENTINEL_L1_8  (BITMAP_FULL << (16))
#define BM_SENTINEL_L1_9  (BITMAP_FULL << (8))
#define BM_SENTINEL_L1_10 (BITMAP_FULL << (2))
#define BM_SENTINEL_L1_11 (BITMAP_FULL << (1))
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

#define DEF_BM_OP0(N, L0, L1, L2)\
static inline void BITPTRS_SET_BASE##N (bitmap_t **base)\
{\
	base[1] = (bitmap_t*) bitmap_dummy;\
	base[2] = (bitmap_t*) bitmap_dummy;\
}\
static inline void BITMAP_SET_LIMIT##N (bitmap_t *const bitmap)\
{\
	bitmap[L0-1] = BM_SENTINEL_L0_##N;\
}\
static inline void BITMAP_SET_LIMIT_AND_COPY_BM##N (bitmap_t *const bitmap, bitmap_t *const snapshot)\
{\
	bitmap[L0-1] = BM_SENTINEL_L0_##N;\
}

#define DEF_BM_OP1(N, L0, L1, L2)\
static inline void BITPTRS_SET_BASE##N (bitmap_t **base)\
{\
	struct BM##N *bm = (struct BM##N *)base[0];\
	base[1] = (bitmap_t*)(&bm->m1.bm);\
	base[2] = (bitmap_t*)bitmap_dummy;\
}\
static inline void BITMAP_SET_LIMIT##N (bitmap_t *const bitmap)\
{\
	struct BM##N *bm = (struct BM##N *)bitmap;\
	bitmap[L0-1] = BM_SENTINEL_L0_##N;\
	bm->m1.bm[L1-1] = BM_SENTINEL_L1_##N;\
}\
static inline void BITMAP_SET_LIMIT_AND_COPY_BM##N (bitmap_t *const bitmap, bitmap_t *const snapshot)\
{\
	struct BM##N *bm = (struct BM##N *)bitmap;\
	struct BM##N *ss = (struct BM##N *)snapshot;\
	bitmap[L0-1] = BM_SENTINEL_L0_##N;\
	bm->m1.bm[L1-1] = BM_SENTINEL_L1_##N | ss->m1.bm[L1-1];\
}

#define DEF_BM_OP2(N, L0, L1, L2)\
static inline void BITPTRS_SET_BASE##N (bitmap_t **base)\
{\
	struct BM##N *bm = (struct BM##N *)base[0];\
	base[1] = (bitmap_t*)(&bm->m1.bm);\
	base[2] = (bitmap_t*)(&bm->m2.bm);\
}\
static inline void BITMAP_SET_LIMIT##N (bitmap_t *const bitmap)\
{\
	struct BM##N *bm = (struct BM##N *)bitmap;\
	bitmap[L0-1] = BM_SENTINEL_L0_##N;\
	bm->m1.bm[L1-1] = BM_SENTINEL_L1_##N;\
	bm->m2.bm[L2-1] = BM_SENTINEL_L2_##N;\
}\
static inline void BITMAP_SET_LIMIT_AND_COPY_BM##N (bitmap_t *const bitmap, bitmap_t *const snapshot)\
{\
	struct BM##N *bm = (struct BM##N *)bitmap;\
	struct BM##N *ss = (struct BM##N *)snapshot;\
	bitmap[L0-1] = BM_SENTINEL_L0_##N;\
	bm->m1.bm[L1-1] = BM_SENTINEL_L1_##N | ss->m1.bm[L1-1];\
	bm->m2.bm[L2-1] = BM_SENTINEL_L2_##N | ss->m2.bm[L2-1];\
}

#define DEF_BM_OP0_(N, S1, S2) \
	DEF_BM_OP0(N, BITMAP_L0_SIZE(N)+S1, BITMAP_L1_SIZE(N)+S2, BITMAP_L2_SIZE(N))
#define DEF_BM_OP1_(N, S1, S2) \
	DEF_BM_OP1(N, BITMAP_L0_SIZE(N)+S1, BITMAP_L1_SIZE(N)+S2, BITMAP_L2_SIZE(N))
#define DEF_BM_OP2_(N, S1, S2) \
	DEF_BM_OP2(N, BITMAP_L0_SIZE(N)+S1, BITMAP_L1_SIZE(N)+S2, BITMAP_L2_SIZE(N))
//DEF_BM_OP2( 3, 256+1/*sentinel*/, 4, 1);
//DEF_BM_OP2( 4, 128+1/*sentinel*/, 2, 1);
#if SIZEOF_VOIDP*8 == 64
DEF_BM_OP1_( 5, 1/*sentinel0*/, 0/*sentinel1*/);
DEF_BM_OP1_( 6, 1/*sentinel0*/, 0/*sentinel1*/);
#else
DEF_BM_OP2_( 5, 1/*sentinel0*/, 1/*sentinel1*/);
DEF_BM_OP2_( 6, 1/*sentinel0*/, 1/*sentinel1*/);
#endif
DEF_BM_OP1_( 7, 1/*sentinel0*/, 0/*sentinel1*/);
DEF_BM_OP1_( 8, 1/*sentinel0*/, 0/*sentinel1*/);
DEF_BM_OP1_( 9, 1/*sentinel0*/, 0/*sentinel1*/);
DEF_BM_OP1_(10, 1/*sentinel0*/, 0/*sentinel1*/);
DEF_BM_OP0_(11, 1/*sentinel0*/, 0/*sentinel1*/);
DEF_BM_OP0_(12, 0, 0);


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

typedef void (*fBITPTRS_SET_BASE)(bitmap_t *base[SEGMENT_LEVEL]);
static void BITPTRS_SET_BASE_(bitmap_t *base[SEGMENT_LEVEL]) {}
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

static inline void BITMAP_SET_LIMIT(bitmap_t *const bitmap, unsigned klass)
{
	BITMAP_SET_LIMIT__[klass](bitmap);
	BM_SET(bitmap[0], 1);
}

static inline void BITPTRS_INIT(BitPtr bitptrs[SEGMENT_LEVEL], Segment *seg, unsigned klass)
{
	unsigned i;
	BITPTRS_SET_BASE[klass](seg->base);
	for (i = 0; i < SEGMENT_LEVEL; ++i) {
		bitptrs[i].idx = 0;
		bitptrs[i].mask = BITMAP_DEFAULT_MASK[klass][i];
	}
}

#ifdef USE_GENERATIONAL_GC
typedef void (*fBITMAP_SET_LIMIT_AND_COPY_BM)(bitmap_t *const bm, bitmap_t *const ss);
static void BITMAP_SET_LIMIT_AND_COPY_BM_(bitmap_t *const bm, bitmap_t *const ss) { (void)bm; }
static const fBITMAP_SET_LIMIT_AND_COPY_BM BITMAP_SET_LIMIT_AND_COPY_BM__[] = {
	BITMAP_SET_LIMIT_AND_COPY_BM_, BITMAP_SET_LIMIT_AND_COPY_BM_, BITMAP_SET_LIMIT_AND_COPY_BM_,
	BITMAP_SET_LIMIT_AND_COPY_BM_, BITMAP_SET_LIMIT_AND_COPY_BM_, BITMAP_SET_LIMIT_AND_COPY_BM5,
	BITMAP_SET_LIMIT_AND_COPY_BM6, BITMAP_SET_LIMIT_AND_COPY_BM7, BITMAP_SET_LIMIT_AND_COPY_BM8,
	BITMAP_SET_LIMIT_AND_COPY_BM9, BITMAP_SET_LIMIT_AND_COPY_BM10, BITMAP_SET_LIMIT_AND_COPY_BM11,
	BITMAP_SET_LIMIT_AND_COPY_BM12
};
static inline void BITMAP_SET_LIMIT_AND_COPY_BM(bitmap_t *const bitmap, bitmap_t *const snapshot, unsigned klass)
{
	BITMAP_SET_LIMIT_AND_COPY_BM__[klass](bitmap, snapshot);
	BM_SET(bitmap[0], 1);
	BM_SET(snapshot[0], 1);
}

static inline void SNAPSHOT_INIT(Segment *seg, unsigned klass)
{
	BITPTRS_SET_BASE[klass](seg->snapshots);
}
#endif

static const unsigned BM_SIZE[] = {
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

#define Object_setTenure(o) TFLAG_set(uintptr_t,(o)->h.magicflag,kObject_GCFlag,1)
#define Object_setYoung(o)  TFLAG_set(uintptr_t,(o)->h.magicflag,kObject_GCFlag,0)
#define Object_isTenure(o) (TFLAG_is(uintptr_t,(o)->h.magicflag, kObject_GCFlag))
#define Object_isYoung(o)  (!Object_isTenure(o))

enum gc_mode {
#define GC_MINOR_FLAG 0
#define GC_MAJOR_FLAG 1
	GC_NOP   = 0,
	GC_MINOR = 1 << GC_MINOR_FLAG,
	GC_MAJOR = 1 << GC_MAJOR_FLAG,
	GC_MAJOR_AND_MINOR = GC_MINOR|GC_MAJOR
};

/* ------------------------------------------------------------------------ */
/* [BMGC API declaration] */
/* ------------------------------------------------------------------------ */

static kObject *bm_malloc_internal(KonohaContext *kctx, HeapManager *mng, size_t n);
//void *bm_malloc(KonohaContext *kctx, size_t n);
//void *bm_realloc(KonohaContext *kctx, void *ptr, size_t os, size_t ns);
//void bm_free(KonohaContext *kctx, void *ptr, size_t n);
static void BMGC_dump(HeapManager *mng);
static void bitmapMarkingGC(KonohaContext *kctx, HeapManager *mng, enum gc_mode mode);
static void HeapManager_init(KonohaContext *kctx, HeapManager *mng, size_t heap_size);
static void HeapManager_delete(KonohaContext *kctx, HeapManager *mng);
static void HeapManager_final_free(KonohaContext *kctx, HeapManager *mng);
static inline void bmgc_Object_free(KonohaContext *kctx, kObject *o);
static bool findNextFreeBlock(AllocationPointer *p);
static HeapManager *BMGC_init(KonohaContext *kctx);
static void BMGC_exit(KonohaContext *kctx, HeapManager *mng);

typedef struct MarkStack {
	kObject **stack;
	size_t tail;
	size_t capacity;
	size_t capacity_log2;
} MarkStack;

typedef struct kmemlocal_t {
	KonohaModuleContext   h;
	HeapManager *gcHeapManager;
	MarkStack mstack;
} kmemlocal_t;

typedef struct kmemshare_t {
	KonohaModule h;
} kmemshare_t;

#define memlocal(kctx) ((kmemlocal_t*)((kctx)->modlocal[MOD_gc]))
#define memshare(kctx) ((kmemshare_t*)((kctx)->modshare[MOD_gc]))

/* ------------------------------------------------------------------------ */

static inline uint64_t getTimeMilliSecond(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

/* ------------------------------------------------------------------------ */
/* malloc */

static void THROW_OutOfMemory(KonohaContext *kctx, size_t size)
{
	/* TODO */
	PLATAPI exit_i(EXIT_FAILURE);
}

static void *call_malloc_aligned(KonohaContext *kctx, size_t size, size_t align)
{
	void *block = NULL;
#if defined(HAVE_POSIX_MEMALIGN)
	int ret = posix_memalign(&block, align, size);
	if (ret != 0)
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
	if ((uintptr_t)block % align != 0) {
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
	PLATAPI exit_i(EXIT_FAILURE);
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

static inline void *do_calloc(size_t count, size_t size)
{
	void *ptr = calloc(count, size);
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

static ssize_t klib_malloced = 0;

static void* Kmalloc(KonohaContext *kctx, size_t s)
{
	size_t *p = (size_t*)do_malloc(s
#ifdef MEMORY_DEBUG
			+ sizeof(size_t)
#endif
			);
	if (unlikely(p == NULL)) {
		ktrace(_ScriptFault|_SystemFault,
			KeyValue_s("!",  "OutOfMemory"),
			KeyValue_s("at", "malloc"),
			KeyValue_u("size", s),
			KeyValue_u("malloced_size", klib_malloced)
		);
	}
#if GCDEBUG
	ktrace(LOGPOL_DEBUG,
			KeyValue_s("@", "malloc"),
			KeyValue_p("from", p),
			KeyValue_p("to", ((char*)p)+s),
			KeyValue_u("size", s));
#endif
	klib_malloced += s;
#ifdef MEMORY_DEBUG
	p[0] = s;
	p += 1;
#endif
	return (void*)p;
}

static void* Kzmalloc(KonohaContext *kctx, size_t s)
{
	size_t *p = (size_t*)do_calloc(1, s
#ifdef MEMORY_DEBUG
			+ sizeof(size_t)
#endif
			);
	klib_malloced += s;
#ifdef MEMORY_DEBUG
	p[0] = s;
	p += 1;
#endif
	return (void*)(p);
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
	ktrace(LOGPOL_DEBUG,
			KeyValue_s("@", "free"),
			KeyValue_p("from", p),
			KeyValue_p("to", ((char*)p)+s),
			KeyValue_u("size", s));
#endif
	do_free(pp, s
#ifdef MEMORY_DEBUG
			+ sizeof(size_t)
#endif
			);
}

/* ------------------------------------------------------------------------ */
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

static HeapManager *BMGC_init(KonohaContext *kctx)
{
	HeapManager *mng = (HeapManager*) do_malloc(sizeof(*mng));
#ifdef GCSTAT
	global_gc_stat.fp = fopen("KONOHA_BMGC_INFO", "a");
#endif
	size_t default_size = SUBHEAP_DEFAULT_SEGPOOL_SIZE;
#ifdef GC_CONFIG
	char *poolsize = knh_PLATAPI getenv_i("KONOHA_DEFAULT_MEMPOOL_SIZE");
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
	return (BITS - CLZ(size_w));
}

#define SEGMENTLIST_NEXT(seg, list) do {\
	seg  = (list);\
	DBG_ASSERT(seg != NULL);\
	(list) = seg->next;\
	seg->next = NULL;\
} while (0)

#define BM_IS_FULL(BM) (~(BM) == 0)
#define SEG_BITMAP_N(seg, n, idx) ((bitmap_t*)((seg->base[n])+idx))
#define AP_BITMAP_N(ap, n, idx)   SEG_BITMAP_N(ap->seg, n, idx)

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
	const AllocationBlock *block = seg->block;
	BlockHeader *head = (BlockHeader *) block;
	head->seg   = seg;
	head->klass = seg->heap_klass;
	gc_info("seg=%p, block=(%p,%p)", seg, block, block+1);
	h->p.blockptr = (AllocationBlock*)((char*)block+(size));
}

static bool newSegment(HeapManager *mng, SubHeap *h)
{
	unsigned klass = h->heap_klass;
	Segment *seg = allocSegment(mng, klass);
	DBG_ASSERT(h->freelist == NULL);

	if (!seg) return false;
	DBG_ASSERT(seg->live_count == 0);
	if (h->seglist_size == h->seglist_max) {
		size_t newSize, oldSize;
		oldSize = sizeof(Segment*)*h->seglist_max;
		newSize = sizeof(Segment*)*h->seglist_max * 2;
		h->seglist_max *= 2;
		h->seglist = (Segment**)(do_realloc(h->seglist, oldSize, newSize));
	}
	seg->base[0] = AllocBitMap(klass);
	seg->heap_klass = klass;
	h->seglist[h->seglist_size++] = seg;

	h->p.seg = seg;
	findBlockOfLastSegment(seg, h, PowerOf2(klass));
	BITPTRS_INIT(h->p.bitptrs, seg, klass);
	BITMAP_SET_LIMIT(seg->base[0], klass);
#ifdef USE_GENERATIONAL_GC
	seg->snapshots[0] = AllocBitMap(klass);
	SNAPSHOT_INIT(seg, klass);
	BITMAP_SET_LIMIT(seg->snapshots[0], klass);
#endif

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

static bool fetchSegment(SubHeap *h, unsigned klass)
{
	Segment *seg;
	if (freelist_isEmpty(h))
		return false;
	seg = freelist_pop(h);
	h->p.seg = seg;
	BITPTRS_INIT(h->p.bitptrs, seg, klass);
	return true;
}

static bool nextSegment(HeapManager *mng, SubHeap *h, AllocationPointer *p, KonohaContext *kctx)
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
#ifdef USE_GENERATIONAL_GC
	bitmap_set(&((KonohaContextVar*)kctx)->safepoint, GC_MINOR_FLAG,
			(uintptr_t)((--h->minor_count) & (MINOR_COUNT-1)) == 0);
#endif

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

static bool inc(AllocationPointer *p, SubHeap *h)
{
	int size = PowerOf2(h->heap_klass);
	p->blockptr = (AllocationBlock*)((char*)p->blockptr+size);
	BitPtr0_inc(p);
	return ++p->seg->live_count > SegmentBlockCount_GC_MARGIN[h->heap_klass];
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
	bpmask = 1UL << (idx % BITS);\
} while (0)

static BitPtr *BitPtr_init(BitPtr *bp, uintptr_t idx)
{
	BITPTR_INIT_(bp->idx, bp->mask, idx);
	return bp;
}

static uintptr_t bitptrToIndex(uintptr_t bpidx, uintptr_t bpmask)
{
	return bpidx * BITS + FFS(bpmask) - 1;
}

static BlockPtr *blockAddress(Segment *seg, uintptr_t idx, uintptr_t mask)
{
	size_t size = seg->heap_klass;
	size_t offset = bitptrToIndex(idx, mask) << size;
	const BlockPtr *ptr = seg->block;
	return (AllocationBlock*)((char*)ptr+offset);
}

#define BP_NEXT_MASK(ap, bpidx, bpmask, j) do {\
	bitmap_t *bm   = AP_BITMAP_N(p, j, bpidx);\
	uintptr_t temp = *bm | (bpmask - 1UL);\
	uintptr_t mask = (temp + 1UL) & ~temp;\
	bpmask = mask;\
} while (0)

static bool findNextFreeBlock(AllocationPointer *p)
{
	uintptr_t idx = BP(p, 0).idx;
	BP_NEXT_MASK(p, BP(p, 0).idx, BP(p, 0).mask, 0);
	if (BP(p, 0).mask == 0) {
		BitPtr *bp;
		unsigned i;
#if GCDEBUG
		gc_info("klass=%" PREFIX_d ", idx=%" PREFIX_d " mask=%" PREFIX_x", seg=%p",
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
			gc_info("klass=%" PREFIX_d ", level=%" PREFIX_d " idx=%" PREFIX_d ", mask=%" PREFIX_x,
					p->seg->heap_klass, i, BP(p, i).idx, BP(p, i).mask);
			DBG_ASSERT(BP(p, i).mask != 0);
		} while (i > 0);
	}
	p->blockptr = blockAddress(p->seg, BP(p, 0).idx, BP(p, 0).mask);
	return true;
}

static void *tryAlloc(KonohaContext *kctx, HeapManager *mng, SubHeap *h)
{
	AllocationPointer *p = &h->p;
	void *temp;
	if (isMarked(p)) {
		if (findNextFreeBlock(p) == false) {
			if (nextSegment(mng, h, p, kctx) == false) {
				return NULL;
			}
		}
	}
	temp = p->blockptr;
	prefetch_(temp, 0, 0);
	bool isEmpty = inc(p, h);

	bitmap_set(&((KonohaContextVar*)kctx)->safepoint, GC_MAJOR_FLAG,
			(mng->segmentList == NULL && h->freelist == NULL && isEmpty));
	return temp;
}

#define HEAP_SEGMENTLIST_INIT_SIZE 16
static bool Heap_init(HeapManager *mng, SubHeap *h, int klass)
{
	size_t i;

#ifdef USE_GENERATIONAL_GC
	h->minor_count = MINOR_COUNT;
#endif
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

static Segment *SegmentPool_init(size_t size, AllocationBlock *block)
{
	size_t i;
	Segment *pool = (Segment*)(do_malloc(sizeof(Segment) * size));
	Segment *seg  = pool;
	Segment *next = seg+1, *tail = pool + size - 1;
	for (i = 0; i < size; ++i, ++block, ++next, ++seg) {
		seg->block = block;
		seg->next  = next;
#if GCDEBUG
		seg->managed_heap = block;
		seg->managed_heap_end = block+1;
#endif
	}
	tail->next = NULL;
	return pool;
}

#ifdef USE_GENERATIONAL_GC
static void dispatchRememberSet(HeapManager *mng, size_t heap_size, AllocationBlock *block)
{
	BlockHeader *head;
	Segment *seg = mng->segmentList;
	bitmap_t *map = do_malloc(heap_size / (MIN_ALIGN) / sizeof(bitmap_t));
	ARRAY_add(BitMapPtr,  &mng->remember_sets, map);
	while (seg) {
		head = (BlockHeader *) block;
		head->remember_set = map;
		seg->remember_set  = map;
		seg = seg->next;
		map += SEGMENT_SIZE / MIN_ALIGN / BITS;
		block++;
	}
}
#endif

static void SegmentPool_dispose(Segment *pool, size_t size)
{
	Segment *seg = NULL;
	size_t i;

	for (i = 0; i < size; i++) {
		seg = pool + i;
		if (seg->base[0]) {
			DeleteBitMap(seg->base[0], seg->heap_klass);
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

#ifdef USE_GENERATIONAL_GC
	dispatchRememberSet(mng, heap_size, (AllocationBlock*) managed_heap);
#endif

	ARRAY_add(size_t,  &mng->heap_size_a, heap_size);
	ARRAY_add(VoidPtr, &mng->managed_heap_a    , managed_heap);
	ARRAY_add(VoidPtr, &mng->managed_heap_end_a, managed_heap_end);

	ARRAY_add(SegmentPtr, &mng->segment_pool_a, segment_pool);
	ARRAY_add(size_t, &mng->segment_size_a, list_size);
#ifdef K_USING_DEBUG
	memshare(kctx)->usedMemorySize += heap_size;
#endif
#ifdef GCSTAT
	gc_stat("Expand Heap(%uMB)[%p, %p]", (int)heap_size/MB_,
			managed_heap, managed_heap_end);
#endif
}

static void HeapManager_init(KonohaContext *kctx, HeapManager *mng, size_t list_size)
{
	size_t i;
	SubHeap *h;
	mng->flags = 0;
	ARRAY_init(size_t,  &mng->heap_size_a);
	ARRAY_init(VoidPtr, &mng->managed_heap_a);
	ARRAY_init(VoidPtr, &mng->managed_heap_end_a);
	ARRAY_init(SegmentPtr, &mng->segment_pool_a);
	ARRAY_init(size_t, &mng->segment_size_a);
#ifdef USE_GENERATIONAL_GC
#ifdef GCDEBUG
	ARRAY_init(ObjectPtr, &mng->remember_set_debug);
#endif
	ARRAY_init(BitMapPtr, &mng->remember_sets);
#endif

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
	unsigned klass = SizeToKlass(n);
	DBG_ASSERT(n <= SUBHEAP_KLASS_SIZE_MAX);
	DBG_ASSERT(n != 0);
	return &(mng->heaps)[klass];
}

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
	return (DBG_CHECK_OBJECT_IN_SEGMENT(o, seg));
}
#else
#define DBG_CHECK_OBJECT_IN_SEGMENT(o, seg) true
#define DBG_CHECK_OBJECT_IN_HEAP(o, h) true
#endif

static void deferred_sweep(KonohaContext *kctx, kObject *o)
{
#ifdef GC_USE_DEFERREDSWEEP
#if GCSTAT
	BlockHeader *head = (BlockHeader*) (((uintptr_t)o) & ~(SEGMENT_SIZE - 1UL));
	global_gc_stat.collected[head->klass] += 1;
#endif
	bmgc_Object_free(kctx, o);
#else
	assert(O_ct(o) == NULL);
#endif
}

#define minorGC(kctx, mng) bitmapMarkingGC(kctx, mng, GC_MAJOR)
#define majorGC(kctx, mng) bitmapMarkingGC(kctx, mng, GC_MINOR)

static kObject *bm_malloc_internal(KonohaContext *kctx, HeapManager *mng, size_t n)
{
	kObject *temp = NULL;
	SubHeap *h;
	DBG_ASSERT(n != 0);
#if GCDEBUG
	global_gc_stat.current_request_size = n;
#endif
	if (n > SUBHEAP_KLASS_SIZE_MAX) {
		// is it really okay? (kimio)
		char *ptr = (char *) do_malloc(n+sizeof(BlockHeader));
		return (kObject *) (ptr + sizeof(BlockHeader));
	}
	h = findSubHeapBySize(mng, n);
	temp = tryAlloc(kctx, mng, h);

	if (temp != NULL)
		goto L_finaly;
#ifdef USE_SAFEPOINT_POLICY
	HeapManager_expandHeap(kctx, mng, SUBHEAP_DEFAULT_SEGPOOL_SIZE*2);
	newSegment(mng, h);
#else
#ifdef USE_GENERATIONAL_GC
	minorGC(kctx, mng);
	temp = tryAlloc(kctx, mng, h);
#endif
	if (temp != NULL)
		goto L_finaly;
	majorGC(kctx, mng);
#endif /* defined(USE_SAFEPOINT_POLICY) */
	temp = tryAlloc(kctx, mng, h);
	if (temp == NULL) {
		THROW_OutOfMemory(kctx, n);
		assert(0);
	}
	L_finaly:;
	DBG_ASSERT(DBG_CHECK_OBJECT_IN_HEAP(temp, h));
#if GCDEBUG
	global_gc_stat.total_object++;
	global_gc_stat.object_count[h->heap_klass]++;
#endif
	deferred_sweep(kctx, temp);
	return temp;
}

#ifdef GCDEBUG
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
	fprintf(stderr, "%16" PREFIX_x , bm);
	for (i = BITS-1; i >= 0; i--) {
		fprintf(stderr, "%d", i % 10);
	}
	fprintf(stderr, "\n                 ");
	for (mask = 1UL << (BITS-1); mask; mask >>= 1) {
		fprintf(stderr, "%d", (bm & mask)?1:0);
	}
	fprintf(stderr, "\n");
}
#endif

static void clearAllBitMapsAndCount(HeapManager *mng, SubHeap *h)
{
	size_t i;
	for (i = 0; i < h->seglist_size; i++) {
		Segment *seg = h->seglist[i];
		ClearBitMap(seg->base[0], h->heap_klass);
		BITMAP_SET_LIMIT(seg->base[0], h->heap_klass);
		gc_info("klass=%d, seg[%" PREFIX_d "]=%p count=%d",
				seg->heap_klass, i, seg, seg->live_count);
		seg->live_count = 0;
	}
}

#ifdef USE_GENERATIONAL_GC
#define LOAD_SNAPSHOT(seg)\
	do_memcpy(seg->base[0], seg->snapshots[0], BM_SIZE[seg->heap_klass])

#define SAVE_SNAPSHOT(seg)\
	do_memcpy(seg->snapshots[0], seg->base[0], BM_SIZE[seg->heap_klass])

#define LOAD_LIVECOUNT(seg) seg->live_count = seg->tenure_live_count
#define SAVE_LIVECOUNT(seg) seg->tenure_live_count = seg->live_count

static void setTenureBitMapsAndCount(HeapManager *mng, SubHeap *h)
{
	size_t i;
	for (i = 0; i < h->seglist_size; i++) {
		Segment *seg = h->seglist[i];
		ClearBitMap(seg->base[0], h->heap_klass);
		LOAD_SNAPSHOT(seg);
		LOAD_LIVECOUNT(seg);
		BITMAP_SET_LIMIT_AND_COPY_BM(seg->base[0], seg->snapshots[0], h->heap_klass);
		gc_info("klass=%d, seg[%lu]=%p count=%d",
				seg->heap_klass, i, seg, seg->live_count);
	}
}
#endif

#define NEXT_MASK(bm, mask) do {\
	uintptr_t temp;\
	temp = bm | (mask - 1UL);\
	mask = (temp + 1UL) & ~temp;\
	BM_SET(bm, mask);\
} while (0)

static kObject *indexToAddr(Segment *seg, uintptr_t idx, uintptr_t mask)
{
	const BlockPtr *ptr = seg->block;
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
			Segment *seg = h->seglist[i];
			bitmap_t *bm0;
			bitmap_t *b0 = (bitmap_t *) seg->base[0];
			bitmap_t *l0 = b0 + SegmentBitMapCount[j];
			for (bm0 = b0; bm0 < l0; ++bm0) {
				b0_final_sweep(kctx, *bm0, bm0 - b0, seg);
			}
		}
		gc_info("heap[%d] collected %" PREFIX_d,
				h->heap_klass,
				global_gc_stat.collected[h->heap_klass]);
	}
}

#if GCDEBUG
static void Heap_dump(const SubHeap *h)
{
	gc_info("klass[%2d] object_count=%u segment_list=(%" PREFIX_d ") ",
			h->heap_klass, global_gc_stat.object_count[h->heap_klass],
			h->seglist_size);
}

#endif /* GCDEBUG */

#ifdef GCDEBUG
static void BMGC_dump(HeapManager *mng)
{
	size_t i;
	gc_info("********************************");
	gc_info("* Heap Information");
	gc_info("********************************");
	gc_info("total allocated object count=%" PREFIX_d, global_gc_stat.total_object);
	for (i = SUBHEAP_KLASS_MIN; i <= SUBHEAP_KLASS_MAX; i++) {
		SubHeap *h = mng->heaps + i;
		Heap_dump(h);
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

static void bmgc_gc_init(KonohaContext *kctx, HeapManager *mng, enum gc_mode mode)
{
	size_t i;
	SubHeap *h;
#ifdef USE_GENERATIONAL_GC
	void (*finit_bitmap) (HeapManager *mng, SubHeap *h);
	finit_bitmap = (mode & GC_MAJOR) ? clearAllBitMapsAndCount : setTenureBitMapsAndCount;
#endif
	BMGC_dump(mng);
	for_each_heap(h, i, mng->heaps) {
#ifdef USE_GENERATIONAL_GC
		finit_bitmap(mng, h);
#else
		clearAllBitMapsAndCount(mng, h);
#endif
	}
}

#define OBJECT_LOAD_BLOCK_INFO(o, seg, index, klass) do {\
	uintptr_t addr, offset;\
	addr   = ((uintptr_t)o) & ~(SEGMENT_SIZE - 1UL);\
	offset = ((uintptr_t)o) &  (SEGMENT_SIZE - 1UL);\
	BlockHeader *head = (BlockHeader*) addr;\
	seg   = head->seg;\
	klass = head->klass;\
	index = offset >> klass;\
} while (0)

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

static void mark_mstack(KonohaContext *kctx, HeapManager *mng, kObject *o, MarkStack *mstack)
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
#ifdef USE_GENERATIONAL_GC
		Object_setTenure(((kObjectVar*)o));
#endif
		bitmap_mark(*bm, seg, bpidx, bpmask);
		++(seg->live_count);
		mstack_push(kctx, mstack, o);
#ifdef GCSTAT
		global_gc_stat.marked[klass]++;
#endif
	}
}

#ifdef USE_GENERATIONAL_GC
static void RememberSet_add(KonohaContext *kctx, kObject *o)
{
#ifdef GCDEBUG
	HeapManager* mng = HeapManager(kctx);
	size_t i;
	for (i = 0; i < ARRAY_size(mng->remember_set_debug); i++) {
		kObject *ptr = ARRAY_n(mng->remember_set_debug, i);
		if (ptr == o) {
			break;
		}
	}
	ARRAY_add(ObjectPtr, &mng->remember_set_debug, o);
#endif
	uintptr_t addr   = ((uintptr_t)o & ~(SEGMENT_SIZE - 1UL));
	uintptr_t offset = ((uintptr_t)o &  (SEGMENT_SIZE - 1UL)) >> SUBHEAP_KLASS_MIN;
	BlockHeader *head = (BlockHeader*) addr;
	bitmap_t *map = head->remember_set;
	bitmap_set(map+(offset/BITS), offset%BITS, Object_isTenure(o));
}

static void RememberSet_reftrace(KonohaContext *kctx, HeapManager *mng)
{
	size_t i;
	FOR_EACH_ARRAY_(mng->remember_sets, i) {
		uintptr_t base_address = (uintptr_t) ARRAY_n(mng->managed_heap_a, i);
		size_t bitmap_size = ARRAY_n(mng->heap_size_a, i) / (MIN_ALIGN * BITS);
		bitmap_t *base = ARRAY_n(mng->remember_sets, i);
		bitmap_t *m = base;
		bitmap_t *e = m + bitmap_size;
		for (; m != e; base+=BITS, base_address += SEGMENT_SIZE) {
			for (; m < base + BITS; ++m) {
				bitmap_t b = (*m);
				while (b != 0) {
					unsigned index, offset;
					index = CTZ(b);
					b ^= 1UL << index;
					offset = (m - base) * BITS + index;
					kObject *o = (kObject*)(base_address + (offset << SUBHEAP_KLASS_MIN));
					KONOHA_reftraceObject(kctx, o);
#ifdef GCDEBUG
					{
						size_t j;
						int marked = 0;
						FOR_EACH_ARRAY_(mng->remember_set_debug, j) {
							kObject* v =  (kObject *)ARRAY_n(mng->remember_set_debug, j);
							if (o == v)
								marked = 1;
						}
						assert(marked == 1);
					}
#endif
				}
				bitmap_reset(m, 0);
			}
		}
	}
}

static void RememberSet_clear(HeapManager *mng)
{
#ifdef GCDEBUG
	ARRAY_clear(ObjectPtr, &mng->remember_set_debug);
#endif
}

#endif

static void Kwrite_barrier(KonohaContext *kctx, kObject *parent)
{
#ifdef USE_GENERATIONAL_GC
	RememberSet_add(kctx, parent);
#endif
}

#define context_reset_refs(kctx) kctx->stack->reftail = kctx->stack->ref.refhead

static void bmgc_gc_mark(KonohaContext *kctx, HeapManager *mng, KonohaStack *esp, enum gc_mode mode)
{
	long i;
	MarkStack *mstack = mstack_init(kctx, &memlocal(kctx)->mstack);
	KonohaStackRuntimeVar *stack = kctx->stack;
	kObject *ref = NULL;

	context_reset_refs(kctx);
	KonohaContext_reftraceAll(kctx);
#ifdef USE_GENERATIONAL_GC
	if (mode & GC_MINOR) {
		RememberSet_reftrace(kctx, mng);
	}
#endif
	size_t ref_size = stack->reftail - stack->ref.refhead;
	goto L_INLOOP;
	while ((ref = mstack_next(mstack)) != NULL) {
		context_reset_refs(kctx);
		KONOHA_reftraceObject(kctx, ref);
		ref_size = stack->reftail - stack->ref.refhead;
		if (ref_size > 0) {
			L_INLOOP:;
			for (i = ref_size-1; i >= 0; --i) {
				mark_mstack(kctx, mng, stack->ref.refhead[i], mstack);
			}
		}
	}
#ifdef USE_GENERATIONAL_GC
	RememberSet_clear(mng);
#endif
}

#if 0
void *bm_malloc(KonohaContext *kctx, size_t n)
{
	HeapManager *mng = HeapManager(kctx);
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
	} else {
		do_free(ptr, n);
	}
}

void *bm_realloc(KonohaContext *kctx, void *ptr, size_t os, size_t ns)
{
	HeapManager *mng = HeapManager(kctx);
	if (os <= K_FASTMALLOC_SIZE) {
		void *newptr = (void *) bm_malloc_internal(kctx, mng, ns);
		if (os > 0) {
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
#endif

#define LIST_PUSH(tail, e) do {\
	*tail = e;\
	tail  = &e->next;\
} while (0)

static void rearrangeSegList(SubHeap *h, unsigned klass, bitmap_t *checkFull)
{
	size_t i, count_dead = 0;
	Segment *unfilled = NULL, **unfilled_tail = &unfilled;

	if (h->seglist_size < 1)
		return;
	for (i = 0; i < h->seglist_size; i++) {
		Segment *seg = h->seglist[i];
		size_t dead = SegmentBlockCount[klass] - seg->live_count;
		count_dead += dead;
		if (dead > 0)
			LIST_PUSH(unfilled_tail, seg);
#ifdef USE_GENERATIONAL_GC
		SAVE_SNAPSHOT(seg);
		SAVE_LIVECOUNT(seg);
#endif
	}
	*unfilled_tail = NULL;
	h->freelist = unfilled;
	fetchSegment(h, klass);
	bitmap_set(checkFull, klass,
			(count_dead < SegmentBlockCount[klass] && h->freelist == NULL));
}

static void bmgc_gc_sweep(KonohaContext *kctx, HeapManager *mng)
{
	bitmap_t checkFull = 0;
	size_t i, j;
	SubHeap *h;

	for_each_heap(h, j, mng->heaps) {
#ifndef GC_USE_DEFERREDSWEEP
		for (i = 0; i < h->seglist_size; i++) {
			Segment *seg = h->seglist[i];
			bitmap_t *bm0;
			bitmap_t *b0 = (bitmap_t *) seg->base[0];
			bitmap_t *l0 = b0 + SegmentBitMapCount[j];
			for (bm0 = b0; bm0 < l0; ++bm0) {
				b0_final_sweep(kctx, *bm0, bm0 - b0, seg);
			}
		}
		gc_info("heap[%d] collected %lu",
				h->heap_klass,
				global_gc_stat.collected[h->heap_klass]);
#endif
		rearrangeSegList(h, j, &checkFull);
	}

	if (checkFull) {
		HeapManager_expandHeap(kctx, mng, SUBHEAP_DEFAULT_SEGPOOL_SIZE*2);
		for_each_heap(h, i, mng->heaps) {
			if (bitmap_get(&checkFull, i))
				newSegment(mng, h);
		}
	}
}

static void bitmapMarkingGC(KonohaContext *kctx, HeapManager *mng, enum gc_mode mode)
{
	DBG_P("GC starting");
	bmgc_gc_init(kctx, mng, mode);
#ifdef GCSTAT
	size_t i = 0, marked = 0, collected = 0, heap_size = 0;
	FOR_EACH_ARRAY_(mng->heap_size_a, i) {
		heap_size += ARRAY_n(mng->heap_size_a, i);
	}
#endif
	bmgc_gc_mark(kctx, mng, kctx->esp, mode);

	bmgc_gc_sweep(kctx, HeapManager(kctx));

#ifdef GCSTAT
	SubHeap *h;
	for_each_heap(h, i, mng->heaps) {
		marked    += global_gc_stat.marked[i];
		collected += global_gc_stat.collected[i];
		global_gc_stat.marked[i]    = 0;
		global_gc_stat.collected[i] = 0;
	}
	global_gc_stat.gc_count += 1;
	gc_stat("%sGC(%" PREFIX_d ") HeapSize=%" PREFIX_d" MB, last_collected=%" PREFIX_d", marked=%"PREFIX_d,
			(mode & GC_MAJOR)?"major":"minor",
			global_gc_stat.gc_count, (heap_size/MB_), collected, marked);
#endif
	bitmap_reset(&((KonohaContextVar*)kctx)->safepoint, 0);
}

/* ------------------------------------------------------------------------ */

static inline void bmgc_Object_free(KonohaContext *kctx, kObject *o)
{
	KonohaClass *ct = O_ct(o);
	if (ct) {
#if GCDEBUG
		ktrace(LOGPOL_DEBUG,
				KeyValue_s("@", "delete"),
				KeyValue_p("ptr", o),
				KeyValue_u("size", ct->cstruct_size),
				KeyValue_u("cid", ct->typeId));
#endif
		gc_info("~Object ptr=%p, cid=%d", o, ct->typeId);
		KONOHA_freeObjectField(kctx, (kObjectVar*)o);
		//((kObjectVar*)o)->h.ct = NULL;
		do_bzero((void *)o, ct->cstruct_size);
#if GCDEBUG
		//memset((void*)o, 0xa, ct->cstruct_size);
		//((kObjectVar*)o)->h.magicflag = 5;
#endif
#ifdef GCSTAT
		Segment *seg;
		uintptr_t klass, index;
		OBJECT_LOAD_BLOCK_INFO(o, seg, index, klass);
		global_gc_stat.collected[seg->heap_klass] += 1;
#endif
	}
}

/* [MODGC API] */
void MODGC_check_malloced_size(KonohaContext *kctx)
{
	if (verbose_gc) {
		PLATAPI printf_i("\nklib:memory leaked=%ld\n", (long)klib_malloced);
	}
}

#define OBJECT_INIT(o) do {\
	o->h.magicflag = 0;\
	o->h.ct = NULL;\
	o->fieldObjectItems[0] = NULL;\
} while (0)

kObject *MODGC_omalloc(KonohaContext *kctx, size_t size)
{
	kObjectVar *o = (kObjectVar*)bm_malloc_internal(kctx, HeapManager(kctx), size);
	OBJECT_INIT(o);
#if GCDEBUG
	ktrace(LOGPOL_DEBUG,
			KeyValue_s("@", "new"),
			KeyValue_p("ptr", o),
			KeyValue_u("size", size));
#endif
	return (kObject*)o;
}

kbool_t MODGC_kObject_isManaged(KonohaContext *kctx, void *ptr)
{
	kObject *o = (kObject *) ptr;

	if ((uintptr_t) o % PowerOf2(SUBHEAP_KLASS_MIN) != 0)
		return false;

	size_t i;
	HeapManager *mng = (HeapManager*) HeapManager(kctx);
	FOR_EACH_ARRAY_(mng->managed_heap_a, i) {
		kObject *s = (kObject *) ARRAY_n(mng->managed_heap_a, i);
		kObject *e = (kObject *) ARRAY_n(mng->managed_heap_end_a, i);
		if (s < o && o < e) {
			Segment *seg;
			uintptr_t klass, index;
			OBJECT_LOAD_BLOCK_INFO(o, seg, index, klass);
			DBG_ASSERT((uintptr_t) o % PowerOf2(klass) == 0);
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

/* ------------------------------------------------------------------------ */

static void kmodgc_local_reftrace(KonohaContext *kctx, struct KonohaModuleContext *baseh) {}

static void kmodgc_local_free(KonohaContext *kctx, struct KonohaModuleContext *baseh)
{
	kmemlocal_t *local = (kmemlocal_t *) baseh;
	if (local->mstack.capacity > 0) {
		do_free(local->mstack.stack,  (local->mstack.capacity + 1) * sizeof(kObject*));
		local->mstack.stack = NULL;
		local->mstack.capacity = 0;
	}
	BMGC_exit(kctx, local->gcHeapManager);
	do_free(local, sizeof(kmemlocal_t));
	kctx->modlocal[MOD_gc] = NULL;
}

static void kmodgc_setup(KonohaContext *kctx, struct KonohaModule *def, int newctx)
{
	if (memlocal(kctx) == NULL) {
		kmemlocal_t *base = do_malloc(sizeof(kmemlocal_t));
		do_bzero(base, sizeof(kmemlocal_t));
		base->h.reftrace = kmodgc_local_reftrace;
		base->h.free     = kmodgc_local_free;
		base->gcHeapManager = BMGC_init(kctx);
		kctx->modlocal[MOD_gc] = (KonohaModuleContext*)base;
	}
}

static void kmodgc_reftrace(KonohaContext *kctx, struct KonohaModule *baseh) {}

static void kmodgc_free(KonohaContext *kctx, struct KonohaModule *baseh)
{
	do_free(baseh, sizeof(kmemshare_t));
	kctx->modshare[MOD_gc] = NULL;
}

static void Kgc_invoke(KonohaContext *kctx, KonohaStack *esp)
{
	enum gc_mode mode = kctx->safepoint & 0x3;
	mode = (mode == GC_NOP) ? mode : GC_MINOR;
	bitmapMarkingGC(kctx, HeapManager(kctx), mode);
}

void MODGC_init(KonohaContext *kctx, KonohaContextVar *ctx)
{
	if (IS_RootKonohaContext(ctx)) {
		kmemshare_t *base = (kmemshare_t*) do_malloc(sizeof(kmemshare_t));
		base->h.name     = "bmgc";
		base->h.setup    = kmodgc_setup;
		base->h.reftrace = kmodgc_reftrace;
		base->h.free     = kmodgc_free;
		KSET_KLIB(Kmalloc, 0);
		KSET_KLIB(Kzmalloc, 0);
		KSET_KLIB(Kfree, 0);
		KSET_KLIB(Kwrite_barrier, 0);
		KSET_KLIB(Kgc_invoke, 0);
		KLIB KonohaRuntime_setModule(kctx, MOD_gc, &base->h, 0);
		assert(sizeof(BlockHeader) <= MIN_ALIGN
				&& "Minimum size of Object may lager than sizeof BlockHeader");
	}
	kmodgc_setup(ctx, (KonohaModule*) memshare(kctx), 1);
}

void MODGC_destoryAllObjects(KonohaContext *kctx, KonohaContextVar *ctx)
{
}

void MODGC_free(KonohaContext *kctx, KonohaContextVar *ctx)
{
	assert(memlocal(ctx) == NULL);
	if (IS_RootKonohaContext(ctx)) {
		KLIB KonohaRuntime_setModule(kctx, MOD_gc, NULL, 0);
	}
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif

#endif /* BMGC_H_ */
