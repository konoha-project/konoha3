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

#ifndef MODIO_H_
#define MODIO_H_

#ifndef MINIOKNOHA_H_
#error Do not include io.h without minikonoha.h.
#endif

#include <stdio.h>
#ifdef _MSC_VER
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */
/* [class defs] */

typedef void FILE_i;

typedef struct StreamApi {
	size_t (*read_i)(KonohaContext *kctx, FILE_i *fp, char *buf, size_t);
	size_t (*write_i)(KonohaContext *kctx, FILE_i *fp, const char *buf, size_t);
	kbool_t (*close_i)(KonohaContext *kctx, FILE_i *fp);
	kbool_t (*isEndOfStream)(KonohaContext *kctx, FILE_i *fp);
} StreamApi;

#define ICONV_NULL       ((uintptr_t)-1)
#define HAS_ICONV(I)     (I != ICONV_NULL)

#ifdef _MSC_VER
#define kInputStream struct kInputStreamVar
#else
typedef struct kInputStreamVar kInputStream;
#endif

struct kInputStreamVar {
	KonohaObjectHeader h;
	FILE_i *fp;
	StreamApi *streamApi;
	uintptr_t iconv;
	KUtilsGrowingArray buffer;
	size_t top; size_t tail;
};

#ifdef _MSC_VER
#define kOutputStream struct kOutputStreamVar
#else
typedef struct kOutputStreamVar kOutputStream;
#endif

struct kOutputStreamVar {
	KonohaObjectHeader h;
	FILE *fp;
	StreamApi *streamApi;
	uintptr_t iconv;
	KUtilsGrowingArray buffer;
};

#define kioshare ((kioshare_t *)kctx->modshare[MOD_IO])

typedef struct {
	KonohaModule h;
	kInputStream  *kstdin;
	kOutputStream *kstdout;
	kOutputStream *kstderr;
} kioshare_t;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MODIO_H_ */
