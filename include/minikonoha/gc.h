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


#ifndef MODGC_H_
#define MODGC_H_

#ifndef MINIOKNOHA_H_
#error Do not include gc.h without minikonoha.h.
#endif

#ifdef __cplusplus
extern "C" {
#endif

//#define kgcmod        ((kgcmod_t*)kctx->mod[MOD_GC])
//#define kgcshare      ((kgcshare_t*)kctx->modshare[MOD_GC])
//
//typedef struct {
//	KonohaModule h;
//} kgcshare_t;
//
//typedef struct {
//	KonohaModuleContext h;
//} kgcmod_t;

extern void MODGC_init(KonohaContext *kctx, KonohaContextVar *ctx);
extern void MODGCSHARE_free(KonohaContext *kctx, KonohaContextVar *ctx);
extern void MODGC_destoryAllObjects(KonohaContext *kctx, KonohaContextVar *ctx);

extern void MODGC_init(KonohaContext *kctx, KonohaContextVar *ctx);
extern void MODGC_free(KonohaContext *kctx, KonohaContextVar *ctx);
extern kObject *MODGC_omalloc(KonohaContext *kctx, size_t size);

/* root reftrace */
extern void KRUNTIME_reftraceAll(KonohaContext *kctx);

extern void MODGC_gc_invoke(KonohaContext *kctx, KonohaStack *esp);
extern void MODGC_check_malloced_size(void);
extern kbool_t MODGC_kObject_isManaged(KonohaContext *kctx, void *ptr);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* MODGC_H_ */
