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

#ifndef MODPOSIX_H_
#define MODPOSIX_H_

#ifndef MINIOKNOHA_H_
#error Do not include posix.h without minikonoha.h.
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */
/* [class defs] */

#define ctxposix         ((ctxposix_t*)kctx->mod[MOD_posix])
#define kmodposix        ((kmodposix_t*)kctx->modshare[MOD_posix])
#define IS_definePosix() (kctx->modshare[MOD_posix] != NULL)
#define CT_Posix         kmodposix->cPosix
#define TY_Posix         kmodposix->cPosix->typeId

#define IS_Posix(O)      ((O)->h.ct == CT_Posix);

typedef struct {
	KonohaModule h;
	KonohaClass     *cPosix;
} kmodposix_t;

typedef struct {
	KonohaModuleContext h;
} ctxposix_t;

typedef const struct kFILEVar    kFILE;
typedef struct kFILEVar          kFILEVar;

struct kFILEVar {
	KonohaObjectHeader h;
	FILE *fp;
	const char *realpath;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MODPOSIX_H_ */
