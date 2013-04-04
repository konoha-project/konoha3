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

#ifndef KONOHA_INIT_H_
#define KONOHA_INIT_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef K_USE_PTHREAD
#if defined(__linux__) && !defined(__USE_UNIX98)
#define __USE_UNIX98 1
#endif
#endif

#if defined(HAVE_CONFIG_H) && !defined(HAVE_BZERO)
#define bzero(s, n) memset(s, 0, n)
#endif


#ifndef PLATAPIFORM_KERNEL
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#else
#include "platform_lkm.h"
#endif /* PLATAPIFORM_KERNEL */

#include <stddef.h>
#include <stdarg.h>

#ifndef __KERNEL__
#include <limits.h>
#include <float.h>
#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#else
#include "konoha3/stdbool.h"
#endif
#include <stdint.h>
#endif

#ifdef __GCC__
#define __PRINTFMT(idx1, idx2) __attribute__((format(printf, idx1, idx2)))
#else
#define __PRINTFMT(idx1, idx2)
#endif

#ifdef _MSC_VER
#pragma warning(disable:4013)
#pragma warning(disable:4018)
#pragma warning(disable:4033)
#pragma warning(disable:4100)
#pragma warning(disable:4101)
#pragma warning(disable:4114)
#pragma warning(disable:4127)
#pragma warning(disable:4201)
#pragma warning(disable:4204)
#pragma warning(disable:4431)
#pragma warning(disable:4820)

#define inline __inline
typedef long long ssize_t;
#define __func__ __FUNCTION__
#endif

/*
 * differ prototype definition in *BSD and linux.
 */

#if defined(__NetBSD__)
#define ICONV_INBUF_CONST const
#else
#define ICONV_INBUF_CONST
#endif



#endif /* KONOHA_INIT_H_ */
