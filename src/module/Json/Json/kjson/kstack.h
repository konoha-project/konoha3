/****************************************************************************
 * Copyright (c) 2012-2013, Masahiro Ide <ide@konohascript.org>
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

#ifndef KJSON_H
#include "kjson.h"
#endif

#ifndef KSTACK_H
#define KSTACK_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef KJSON_MALLOC
#define KJSON_MALLOC(N) malloc(N)
#define KJSON_FREE(PTR) free(PTR)
#endif

typedef ARRAY(JSON) kstack_t;

static inline unsigned kstack_size(kstack_t *stack)
{
    return stack->size;
}

static inline void kstack_init(kstack_t *stack)
{
    ARRAY_init(JSON, stack, 8);
}

static inline void kstack_deinit(kstack_t *stack, int check_stack)
{
    if(check_stack) {
        assert(kstack_size(stack) == 0);
    }
    ARRAY_dispose(JSON, stack);
}

static inline void kstack_push(kstack_t *stack, JSON v)
{
    JSONObject_Retain(v);
    ARRAY_add(JSON, stack, v);
}

static inline JSON kstack_pop(kstack_t *stack)
{
    unsigned size = --stack->size;
    assert(size >= 0);
    return ARRAY_get(JSON, stack, size);
}

static inline void kstack_move(kstack_t *stack, JSON *dst, unsigned beginIdx, unsigned length)
{
    JSON *src = stack->list + beginIdx, *end = src + length;
    while (src < end) {
        *dst++ = *src++;
    }
    stack->size -= length;
}

#undef KJSON_MALLOC
#undef KJSON_FREE

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* end of include guard */
