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
#include "konoha2/sugar.h"
#include "konoha2/gc.h"
#include <assert.h>

extern kMethod* NameSpace_getMethodNULL(CTX, kNameSpace *ns, kcid_t cid, kmethodn_t mn);
/* from ../src/minivm/konoha2.h */
#ifndef DEND
#define PTRSIZE  (sizeof(void*))
#define DEND     ((const char*)-1)
#define DT_(X)   ((const char*)((TY_##X)    | (1UL<<(PTRSIZE*8-1))))
#define DC_(X)   ((const char*)((CLASS_##X) | (1UL<<(PTRSIZE*8-1))))
#define DS_(X)   ((const char*)((FN_##X)    | (1UL<<(PTRSIZE*8-1))))
#define DM_(X)   ((const char*)((MN_##X)    | (1UL<<(PTRSIZE*8-1))))
#define DF_(X)   ((const char*)(X))
#define FF_(X)   (kMethod_##X)
#endif
static const char *KW_t_(CTX, ksymbol_t kw) { return NULL; }
static kString *Skw_(CTX, ksymbol_t kw) { return NULL; }

static KMETHOD DummyBase_f(CTX, ksfp_t *sfp _RIX) {}
static KMETHOD DummyBase_g(CTX, ksfp_t *sfp _RIX) {}
static KMETHOD Dummy_f(CTX, ksfp_t *sfp _RIX) {}

struct DummyBase {
    int x;
};
static KDEFINE_CLASS DummyBaseDef = {
    .structname   = "DummyBase",
    .cid          = 100,
    .cflag        = 0,
    .cstruct_size = sizeof(struct DummyBase),
};

static KDEFINE_CLASS DummyDef = {
    .structname   = "Dummy",
    .cid          = 0,
    .cflag        = 0,
    .cstruct_size = sizeof(struct DummyBase),
};

#define CT_Dummy ct

kclass_t *ct_dummybase;
kclass_t *ct_dummy;

void init_test(CTX)
{
    ct_dummybase = (kclass_t *) Konoha_addClassDef(&DummyBaseDef);
    DummyDef.supcid = ct_dummybase->cid;
    ct_dummy = (kclass_t *) Konoha_addClassDef(&DummyDef);
    const char *MethodData[] = {
        DF_(0L), DF_(DummyBase_f), DT_(Int), DF_("DummyBase"), "f", DF_(0),
        DF_(0), DF_(DummyBase_g), DT_(Int), DF_("DummyBase"), "g", DF_(0),
        DF_(0), DF_(Dummy_f),     DT_(Int), DF_("Dummy"), "f", DF_(0),
        DEND
    };
    kmodsugar->h.setup(_ctx, (kmodshare_t*)kmodsugar);
    _ctx->lib2->KloadMethodData(_ctx, kmodsugar->rootns,
            (const char **)MethodData);
}

void test_getMethod(CTX)
{
    kMethod *mtd;
    kmethodn_t mn_f, mn_g, mn_h;
    mn_f = ksymbol("f", 127, SYM_NEWID, SYMPOL_METHOD);
    mn_g = ksymbol("g", 127, SYM_NEWID, SYMPOL_METHOD);
    mn_h = ksymbol("h", 127, SYM_NEWID, SYMPOL_METHOD);

    mtd = NameSpace_getMethodNULL(_ctx, kmodsugar->rootns,
            ct_dummy->cid, mn_f);
    fprintf(stderr, "mtd=%p, mtd->cid=%d, mtd->mn=%d\n",
            mtd, mtd->cid, mtd->mn);
    assert(mtd != NULL && mtd->cid == ct_dummy->cid && mtd->mn == mn_f);

    mtd = NameSpace_getMethodNULL(_ctx, kmodsugar->rootns,
            ct_dummybase->cid, mn_g);
    fprintf(stderr, "mtd=%p, mtd->cid=%d, mtd->mn=%d\n",
            mtd, mtd->cid, mtd->mn);
    assert(mtd != NULL && mtd->cid == ct_dummybase->cid && mtd->mn == mn_g);

    mtd = NameSpace_getMethodNULL(_ctx, kmodsugar->rootns,
            ct_dummybase->cid, mn_h);
    fprintf(stderr, "mtd=%p\n", mtd);
    assert(mtd == NULL);
}

int main(int argc, const char *argv[])
{
	static kplatform_t plat = {
    	"test", 4096,
    };
    konoha_t konoha = konoha_open((const kplatform_t*)&plat);
    init_test(konoha);
    test_getMethod(konoha);
    konoha_close(konoha);
    MODGC_check_malloced_size();
    return 0;
}
