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
#include "minikonoha/minikonoha.h"
#include "minikonoha/gc.h"

extern int verbose_debug;
#include <minikonoha/platform.h>

static int _sum_  = 0;
static int _sum2_ = 0;

static void reftrace(KonohaContext *kctx, KUtilsHashMapEntry *e, void *thunk)
{
    _sum_ += e->unboxValue;
}
static void reftrace2(KonohaContext *kctx, void *e)
{
    _sum2_ += (uintptr_t)e;
}

void test_Kmap(KonohaContext *kctx)
{
    int i;
    KUtilsHashMap* map = KLIB Kmap_init(kctx, 4);
    for (i = 0; i < 10; ++i) {
        KUtilsHashMapEntry *entry = KLIB Kmap_newEntry(kctx, map, i);
        assert(entry->hcode == i);
        entry->unboxKey = i*2;
        entry->unboxValue = i;
    }
    for (i = 0; i < 10; ++i) {
        KUtilsHashMapEntry *entry = KLIB Kmap_get(kctx, map, i);
        assert(entry != NULL);
        assert(entry->unboxValue == i);
    }
    KLIB Kmap_each(kctx, map, NULL, reftrace);
    fprintf(stderr, "%d\n", _sum_);
    assert(_sum_ == 45);

    for (i = 0; i < 10; i+=2) {
        KUtilsHashMapEntry *entry = KLIB Kmap_get(kctx, map, i);
        assert(entry != NULL);
        KLIB Kmap_remove(map, entry);
    }
    for (i = 0; i < 10; i+=2) {
        KUtilsHashMapEntry *entry = KLIB Kmap_get(kctx, map, i);
        assert(entry == NULL);
    }
    for (i = 0; i < 10; ++i) {
        KUtilsHashMapEntry *entry = KLIB Kmap_get(kctx, map, i);
        if (i % 2 == 0) {
            assert(entry == NULL);
        } else {
            assert(entry->unboxValue == i);
        }
    }
    KLIB Kmap_free(kctx, map, reftrace2);
    assert(_sum2_ == 25);
    fprintf(stderr, "%d\n", _sum2_);
    _sum_ = _sum2_ = 0;
}

int main(int argc, const char *argv[])
{
    KonohaContext* konoha = konoha_open(KonohaUtils_getDefaultPlatformApi());
    int i;
    for (i = 0; i < 100; ++i) {
        test_Kmap(konoha);
    }
    konoha_close(konoha);
    return 0;
}
