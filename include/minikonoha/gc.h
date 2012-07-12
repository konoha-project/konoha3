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

//#define kgcmod        ((kgcmod_t*)_ctx->mod[MOD_GC])
//#define kgcshare      ((kgcshare_t*)_ctx->modshare[MOD_GC])
//
//typedef struct {
//	kmodshare_t h;
//} kgcshare_t;
//
//typedef struct {
//	kmodlocal_t h;
//} kgcmod_t;

extern void MODGC_init(CTX, kcontext_t *ctx);
extern void MODGCSHARE_free(CTX, kcontext_t *ctx);
extern void MODGC_destoryAllObjects(CTX, kcontext_t *ctx);

extern void MODGC_init(CTX, kcontext_t *ctx);
extern void MODGC_free(CTX, kcontext_t *ctx);
extern kObject *MODGC_omalloc(CTX, size_t size);

/* root reftrace */
extern void KRUNTIME_reftraceAll(CTX);

extern void MODGC_gc_invoke(CTX, int needsCStackTrace);
extern void MODGC_check_malloced_size(void);

#endif /* MODGC_H_ */
