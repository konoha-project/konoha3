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

#ifndef MODFLOAT_H_
#define MODFLOAT_H_

#define KFLOAT_FMT             "%.6f"

#define kfloatmod        ((kfloatmod_t*)kctx->mod[MOD_float])
#define kmodfloat        ((kmodfloat_t*)kctx->modshare[MOD_float])
#define IS_defineFloat()    (kctx->modshare[MOD_float] != NULL)
#define CT_Float         kmodfloat->cFloat
#define TY_Float         kmodfloat->cFloat->classId

#define IS_Float(O)      ((O)->h.ct == CT_Float)

typedef struct {
	KonohaModule h;
	KonohaClass *cFloat;
} kmodfloat_t;

typedef struct {
	KonohaContextModule h;
} kfloatmod_t;

typedef const struct _kFloat kFloat;
struct _kFloat {
	KonohaObjectHeader h;
	kfloat_t floatValue;
};

#endif /* MODFLOAT_H_ */
