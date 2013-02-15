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

#include <stdio.h>
#include "konoha3/konoha.h"
#include "test_konoha.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dummy {
	int x;
} kDummy;


static int __Init__  = 0;
static int __trace__ = -1;
static int __Free__  = 0;

static void Dummy_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	assert((uintptr_t)conf == 0xdeadbeaf);
	((kDummy *)o)->x = __Init__++;
}

static void Dummy_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	__trace__++;
}

static void Dummy_Free(KonohaContext *kctx, kObject *o)
{
	__Free__++;
}

static KDEFINE_CLASS DummyDef = {
	/*.structname   = */ "Dummy",
	/*.typeId       = */ 100,
	/*.cflag        = */ 0,
	0,
	0,
	0,
	0,
	0,
	/*.cstruct_size = */ sizeof(struct Dummy),
	0,
	0,
	0,
	/*.init         = */ Dummy_Init,
	/*.reftrace     = */ Dummy_Reftrace,
	/*.free         = */ Dummy_Free,
		0/*fnull*/,
		0/*p*/,
		0/*unbox*/,
		0/*compareObject*/,
		0/*compareUnboxValue*/,
		0/*initdef*/,
		0/*isSubType*/,
		0/*realtype*/
};

void test_gc(KonohaContext *kctx)
{
#define KClass_Dummy ct
	int i, j;
	KClass *ct = KLIB KClass_define(kctx, 0, NULL, &DummyDef, 0);
	/* small size */
	for (i = 0; i < 10; ++i) {
		for (j = 0; j < 100; ++j) {
			kDummy *dummy = new_(Dummy, 0xdeadbeaf, NULL);
			assert(__Init__ == dummy->x+1);
		}
		assert(__Init__ == (i+1) * 100);
		assert(__trace__ == -1);
		PLATAPI GCModule.ScheduleGC(kctx, NULL);
	}

	int small_object_count = __Init__;
	/* middle size */
	for (i = 0; i < 100; ++i) {
		for (j = 0; j < 1000; ++j) {
			kDummy *dummy = new_(Dummy, 0xdeadbeaf, NULL);
			assert(__Init__ == dummy->x+1);
		}
		assert(__Init__ == (i+1) * 1000 + small_object_count);
		assert(__trace__ == -1);
		PLATAPI GCModule.ScheduleGC(kctx, NULL);
	}
}

#ifdef _WIN64
#ifdef _MSC_VER
#include <intrin.h>
static uint32_t CLZ(uint32_t x)
{
	unsigned long r = 0;
	_BitScanReverse(&r, x);
	return 63 - r;
}
static uint32_t FFS(uint32_t x)
{
	if(x == 0) return 0;
	return CTZ(x) + 1;
}
#else /* defined(_MSC_VER) */
#define FFS(n) __builtin_ffsll(n)
#define CLZ(n) __builtin_clzll(n)
#endif /* defined(_MSC_VER) */
#else /* defined(_WIN64) */
#define FFS(n) __builtin_ffsl(n)
#define CLZ(n) __builtin_clzl(n)
#endif

static uintptr_t myffs(uintptr_t val)
{
	uintptr_t bit;

	if(val == 0)
		return 0;

	for (bit = 1; !(val & 1); bit++)
		val >>= 1;

	return bit;
}

static void test_bitops()
{
	static const uintptr_t test_data[] = {
		1, 2, 3, 4, 5, 7, 13,
		100, 108, 120, 128, 129, 219, 250, 256,
		257, 300, 420, 510, 512, 513, 1000, 1023,
		1024, 2040, 2048, 2049, 4095, 4096, 4097, 8190,
		8192, 8193, 16383, 16384, 16385, 32767, 32768, 32769
	};

	static const uintptr_t clz_test[] = {
		63, 62, 62, 61, 61, 61, 60,
		57, 57, 57, 56, 56, 56, 56, 55,
		55, 55, 55, 55, 54, 54, 54, 54,
		53, 53, 52, 52, 52, 51, 51, 51,
		50, 50, 50, 49, 49, 49, 48, 48
	};
	uintptr_t i;
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
	for (i = 0; i < ARRAY_SIZE(test_data); i++) {
		int test   = CLZ(test_data[i]);
		int answer = (clz_test[i]) - ((sizeof(void *)==8)?0:32);
		assert(test == answer);
		assert(FFS(test_data[i]) == myffs(test_data[i]));
	}
}

int main(int argc, const char *argv[])
{
	int ret = 0;
	KonohaContext* konoha = CreateContext();
	test_gc(konoha);
	DeleteContext(konoha);
	assert(__Free__ == __Init__);
	fprintf(stderr, "alloced_object_count = %d, freed_object_count=%d\n", __Init__, __Free__);
	test_bitops();
	return ret;
}

#ifdef __cplusplus
}
#endif
