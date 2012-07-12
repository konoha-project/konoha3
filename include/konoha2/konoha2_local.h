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

#ifndef KONOHA2_LOCAL_H_
#define KONOHA2_LOCAL_H_

#define IS_ROOTCTX(o)  (_ctx == (CTX_t)o)

// These functions are local functions in konoha2 binary.
// Don't call from packages directly   (kimio)

struct _kObject** KONOHA_reftail(CTX, size_t size);
void KONOHA_reftraceObject(CTX, kObject *o);  // called from MODGC
void KONOHA_freeObjectField(CTX, struct _kObject *o);       // callled from MODGC

void MODCODE_init(CTX, kcontext_t *ctx);
//void MODCODE_genCode(CTX, kMethod *mtd, const struct _kBlock *bk);

void MODSUGAR_init(CTX, kcontext_t *ctx);
kstatus_t MODSUGAR_loadscript(CTX, const char *path, size_t len, kline_t pline);
kstatus_t MODSUGAR_eval(CTX, const char *script, kline_t uline);

void MODLOGGER_init(CTX, kcontext_t *ctx);
void MODLOGGER_free(CTX, kcontext_t *ctx);
void MODSUGAR_loadMethod(CTX);




#endif /* KONOHA2_LOCAL_H_ */
