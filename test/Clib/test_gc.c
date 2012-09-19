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

#ifdef __cplusplus
extern "C" {
#endif

extern int verbose_debug;
#include <minikonoha/platform.h>

typedef struct Dummy {
    int x;
} kDummy;


static int __init__  = 0;
static int __trace__ = -1;
static int __free__  = 0;

static void Dummy_init(KonohaContext *kctx, kObject *o, void *conf)
{
    assert((uintptr_t)conf == 0xdeadbeaf);
    ((kDummy*)o)->x = __init__++;
}

static void Dummy_reftrace(KonohaContext *kctx, kObject *o)
{
    __trace__++;
}

static void Dummy_free(KonohaContext *kctx, kObject *o)
{
    __free__++;
}

static KDEFINE_CLASS DummyDef = {
    .structname   = "Dummy",
    .typeId          = 100,
    .cflag        = 0,
    .cstruct_size = sizeof(struct Dummy),
    .init     = Dummy_init,
    .reftrace = Dummy_reftrace,
    .free     = Dummy_free
};

void test_gc(KonohaContext *kctx)
{
#define CT_Dummy ct
    int i, j;
    KonohaClass *ct = KLIB KonohaClass_define(kctx, 0, NULL, &DummyDef, 0);
    /* small size */
    for (i = 0; i < 10; ++i) {
        for (j = 0; j < 100; ++j) {
            kDummy *dummy = new_(Dummy, 0xdeadbeaf);
            assert(__init__ == dummy->x+1);
        }
        assert(__init__ == (i+1) * 100);
        assert(__trace__ == -1);
        KLIB Kgc_invoke(kctx, 0);
    }

    int small_object_count = __init__;
    /* middle size */
    for (i = 0; i < 100; ++i) {
        for (j = 0; j < 1000; ++j) {
            kDummy *dummy = new_(Dummy, 0xdeadbeaf);
            assert(__init__ == dummy->x+1);
        }
        assert(__init__ == (i+1) * 1000 + small_object_count);
        assert(__trace__ == -1);
        KLIB Kgc_invoke(kctx, 0);
    }
}

int main(int argc, const char *argv[])
{
    int ret = 0;
    KonohaContext* konoha = konoha_open(KonohaUtils_getDefaultPlatformApi());
    test_gc(konoha);
    konoha_close(konoha);
    assert(__free__ == __init__);
    fprintf(stderr, "alloced_object_count = %d, freed_object_count=%d\n", __init__, __free__);
    return ret;
}

#ifdef __cplusplus
}
#endif
