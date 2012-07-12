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

#ifndef GC_COMMONS_H_
#define GC_COMMONS_H_
#ifndef K_INTERNAL
#define K_INTERNAL  1
#endif
#include <minikonoha/minikonoha.h>

#ifdef K_USING_POSIX_
#include <sys/mman.h>
#define knh_mlock(p, size)   mlock(p, size)
#define knh_unmlock(p)       unmlock(p)
#endif

#ifndef knh_mlock
#define knh_mlock(p, size)
#define knh_unmlock(p)
#endif

#define KB_   (1024)
#define MB_   (KB_*1024)
#define GB_   (MB_*1024)

//#define K_USING_MEMSTAT  1
//#define K_USING_MEMLOG   1

#ifdef K_USING_MEMLOG
//static uint64_t memlog_start = 0;
//#define MEMLOG_INIT()  memlog_start = knh_getTimeMilliSecond()

#define MEMLOG(kctx, action, pe, ...) KNH_NTRACE2(kctx, action, pe, ## __VA_ARGS__)

#else
#define MEMLOG(kctx, action, pe, ...)
#endif

#define KNH_ATOMIC_ADD(a, b) __sync_add_and_fetch(&(a), b)
#define KNH_ATOMIC_SUB(a, b) __sync_sub_and_fetch(&(a), b)

#ifdef K_USING_MEMSTAT
#define STAT_(stmt) stmt
#define STAT_mem(kctx, SIZE) do { \
	kstatinfo_t *stat = kctx->stat;\
	KNH_ATOMIC_ADD(stat->usedMemorySize, (SIZE));\
	if(stat->usedMemorySize > stat->maxMemoryUsage) stat->maxMemoryUsage = stat->usedMemorySize;\
} while (0)

#define STAT_dmem(kctx, SIZE)  KNH_ATOMIC_SUB((kctx->stat)->usedMemorySize, (SIZE))

#define STAT_Object(kctx, ct) do { \
	((KonohaClassVar*)ct)->count += 1; \
	((KonohaClassVar*)ct)->total += 1; \
} while (0)

#define STAT_dObject(kctx, ct) ((KonohaClassVar*)ct)->count -= 1

#else
#define STAT_(stmt)
#define STAT_mem(kctx, SIZE)
#define STAT_dmem(kctx, SIZE)
#define STAT_Object(kctx, ct)
#define STAT_dObject(kctx, ct)

#endif

int verbose_gc = 0;

#endif /* GC_COMMONS_H_ */
