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

#ifndef MODICONV_H_
#define MODICONV_H_

#ifndef MINIOKNOHA_H_
#error Do not include bytes.h without minikonoha.h.
#endif

#include <string.h>
#ifdef HAVE_ICONV_H
#include <iconv.h>
#endif /* HAVE_ICONV_H */

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */
/* [class defs] */

#define ctxiconv         ((ctxiconv_t*)kctx->mod[MOD_iconv])
#define kmodiconv        ((kmodiconv_t*)kctx->modshare[MOD_iconv])
#define IS_defineBytes() (kctx->modshare[MOD_iconv] != NULL)
#define CT_Bytes         kmodiconv->cBytes
#define TY_Bytes         kmodiconv->cBytes->typeId

#define IS_Bytes(O)      ((O)->h.ct == CT_Bytes)

#ifdef HAVE_ICONV_H
typedef iconv_t kiconv_t;
#else
typedef long    kiconv_t;
#endif /* HAVE_ICONV_H */

typedef struct {
	KonohaModule h;
	KonohaClass     *cBytes;
	//kbool_t      (*encode)(const char* from, const char* to, const char* text, size_t len, KUtilsWriteBuffer* wb);
	//const char*  fmt;
	//const char*  locale;
} kmodiconv_t;

typedef struct {
	KonohaModuleContext h;
} ctxiconv_t;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MODICONV_H_ */
