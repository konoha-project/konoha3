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
#include "test_konoha.h"

void test_Kwb(CTX)
{
    karray_t a;
    kwb_t wb;
    /* if we use karray as kwb, struct_size should be sizeof(char) */
    _ctx->lib2->Karray_init(_ctx, &a, 4);
    _ctx->lib2->Kwb_init(&a, &wb);
    _ctx->lib2->Kwb_write(_ctx, &wb, "abcd", 4);
    _ctx->lib2->Kwb_write(_ctx, &wb, "abcd", 4);
    const char *data = _ctx->lib2->Kwb_top(_ctx, &wb, 1);
    assert(strcmp(data, "abcdabcd") == 0);
    _ctx->lib2->Kwb_free(&wb);
    _ctx->lib2->Karray_free(_ctx, &a);
}

int main(int argc, const char *argv[])
{
    konoha_t konoha = konoha_open((const kplatform_t*)&plat);
    int i;
    for (i = 0; i < 100; ++i) {
        test_Kwb(konoha);
    }
    konoha_close(konoha);
    MODGC_check_malloced_size();
    return 0;
}
