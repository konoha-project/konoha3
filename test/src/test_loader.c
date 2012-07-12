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
#include "konoha2/konoha2.h"
#include "konoha2/gc.h"
#include "loader/mod_sample.h"
#include "../test_konoha.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef PACKAGE_BUILD_DIR
#define BUILD_DIR ""
#else
#define BUILD_DIR PACKAGE_BUILD_DIR "/"
#endif

void test_module_load(CTX)
{
    module_load(_ctx, (kcontext_t*)_ctx, BUILD_DIR "mod_sample.dylib", "ksample");
    ksampleshare_t *sharemod = (ksampleshare_t*) _ctx->modshare[MOD_SAMPLE];
    assert(sharemod);

    sharemod->h.setup(_ctx, (kmodshare_t*)sharemod);
    ksamplemod_t *mod = (ksamplemod_t*) _ctx->mod[MOD_SAMPLE];

    assert(mod);
    assert(IS_NULL(mod->array) == 0);
    MODGC_gc_invoke(_ctx, 0);
    assert(IS_NULL(mod->array) == 0);
}

static kcid_t sample_cid;
void test_classdef_load(CTX)
{
    kclass_t *ct;
    ct = class_load(_ctx, BUILD_DIR "classdef_sample.dylib", "Sample");
    assert(ct);
    assert(strcmp(S_totext(ct->name), "Sample") == 0);
    assert(ct->cflag == 2);
    assert(ct->cstruct_size == 64/*sizeof(struct Sample)*/);
    sample_cid = ct->cid;
}

void test_mtd_load(CTX)
{
    kParam  *pa;
    kMethod *mtd;
    knh_Fmethod fmtd;
    kmethodn_t mn;
    kparam_t p[] = {{TY_Int, 0}, {TY_Int, 0}};
    p[0].fn = _ctx->lib2->Ksymbol(_ctx, "arg0", 4, SYM_NEWID, 0);
    p[1].fn = _ctx->lib2->Ksymbol(_ctx, "arg1", 4, SYM_NEWID, 0);
    mn = _ctx->lib2->Ksymbol(_ctx, "f", 1, SYM_NEWID, 0);

    pa   = _ctx->lib2->Knew_Param(_ctx, TY_Int, 2, p);
    mtd  = _ctx->lib2->Knew_Method(_ctx, 0, sample_cid, mn, pa, 0);
    fmtd = method_load(_ctx, BUILD_DIR "mtd_sample.dylib", mtd);
    assert(fmtd != NULL);
}

int main(int argc, const char *argv[])
{
    static kplatform_t plat = {
    	"test", 4096,
    };
    konoha_t konoha = konoha_open((const kplatform_t*)&plat);
    test_module_load(konoha);
    test_classdef_load(konoha);
    test_mtd_load(konoha);
    konoha_close(konoha);
    MODGC_check_malloced_size();
    return 0;
}
