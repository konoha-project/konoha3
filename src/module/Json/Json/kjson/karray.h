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

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#ifndef KJSON_ARRAY_H_
#define KJSON_ARRAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LOG2
#define LOG2(N) ((unsigned)((sizeof(void *) * 8) - __builtin_clzl((N) - 1)))
#endif

/* ARRAY */
#define ARRAY(T) ARRAY_##T##_t
#define DEF_ARRAY_STRUCT0(T, SizeTy)\
struct ARRAY(T) {\
    T *list;\
    SizeTy size;\
    SizeTy capacity;\
}
#define DEF_ARRAY_STRUCT(T) DEF_ARRAY_STRUCT0(T, int)

#define DEF_ARRAY_T(T)\
struct ARRAY(T);\
typedef struct ARRAY(T) ARRAY(T)

#define DEF_ARRAY_OP__(T, ValueType)\
static inline ARRAY(T) *ARRAY_init_##T (ARRAY(T) *a, size_t initsize) {\
    a->list = (T *) KJSON_MALLOC(sizeof(T)*initsize);\
    a->capacity  = initsize;\
    a->size  = 0;\
    return a;\
}\
static inline void ARRAY_##T##_ensureSize(ARRAY(T) *a, size_t size) {\
    if(a->size + size <= a->capacity) {\
        return;\
    }\
    while(a->size + size > a->capacity) {\
        a->capacity *= 2;\
    }\
    a->list = (T *)realloc(a->list, sizeof(T) * a->capacity);\
}\
static inline void ARRAY_##T##_dispose(ARRAY(T) *a) {\
    KJSON_FREE(a->list);\
    a->size     = 0;\
    a->capacity = 0;\
    a->list     = NULL;\
}\
static inline void ARRAY_##T##_add(ARRAY(T) *a, ValueType v) {\
    if(a->size + 1 >= a->capacity) {\
        a->capacity = 1 << LOG2(a->capacity * 2 + 1);\
        a->list = (T *)realloc(a->list, sizeof(T) * a->capacity);\
    }\
    ARRAY_##T##_set(a, a->size++, v);\
}

#define DEF_ARRAY_OP(T)\
static inline T *ARRAY_##T##_get(ARRAY(T) *a, int idx) {\
    return a->list+idx;\
}\
static inline void ARRAY_##T##_set(ARRAY(T) *a, int idx, T *v) {\
    memcpy(a->list+idx, v, sizeof(T));\
}\
DEF_ARRAY_OP__(T, T *)

#define DEF_ARRAY_OP_NOPOINTER(T)\
static inline T ARRAY_##T##_get(ARRAY(T) *a, int idx) {\
    return a->list[idx];\
}\
static inline void ARRAY_##T##_set(ARRAY(T) *a, int idx, T v) {\
    a->list[idx] = v;\
}\
DEF_ARRAY_OP__(T, T)

#define DEF_ARRAY_T_OP(T) DEF_ARRAY_T(T);DEF_ARRAY_OP(T)

#define ARRAY_get(T, a, idx)    ARRAY_##T##_get(a, idx)
#define ARRAY_set(T, a, idx, v) ARRAY_##T##_set(a, idx, v)
#define ARRAY_add(T, a, v)      ARRAY_##T##_add(a, v)
#define ARRAY_dispose(T, a)     ARRAY_##T##_dispose(a)
#define ARRAY_init(T, a, s)     ARRAY_init_##T (a, s)
#define ARRAY_ensureSize(T, a, size) ARRAY_##T##_ensureSize(a, size)
#define ARRAY_n(a, n)  ((a).list+n)
#define ARRAY_size(a)  ((a).size)
#define ARRAY_last(a)  ARRAY_n(a, ((a).size-1))
#define ARRAY_init_1(T, a, e1) do {\
    ARRAY_init(T, a, 4);\
    ARRAY_add(T, a, e1);\
} while(0)

#define FOR_EACH_ARRAY_(a, x, i)\
    for(i=0, x = ARRAY_n(a, i); i < ARRAY_size(a); x = ARRAY_n(a,(++i)))

#define ARRAY_BEGIN(A) ARRAY_n(A, 0)
#define ARRAY_END(A)   ARRAY_n(A,ARRAY_size(A))

#define FOR_EACH_ARRAY(a, x, e)\
    for(x = ARRAY_BEGIN(a), e = ARRAY_END(a); x != e; ++x)


#ifdef __cplusplus
}
#endif
#endif /* end of include guard */
