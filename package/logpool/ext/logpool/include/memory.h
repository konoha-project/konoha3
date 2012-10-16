#include <stdlib.h>
#include <string.h>

#ifndef LOGPOOL_MEMORY_H
#define LOGPOOL_MEMORY_H

#define cast(T, V) ((T)(V))
#define CLZ(n) __builtin_clzl(n)
#define BITS (sizeof(void *) * 8)
#define SizeToKlass(N) ((uint32_t)(BITS - CLZ(N - 1)))

#ifdef USE_NO_CHECK_MALLOC
#define CHECK_MALLOC(PREFIX)
#define CHECK_MALLOCED_SIZE(PREFIX)
#define CHECK_MALLOCED_INC_SIZE(PREFIX, n)
#define CHECK_MALLOCED_DEC_SIZE(PREFIX, n)
#else /*defined(USE_CHECK_MALLOC)*/
#define CONCAT(A, B) A##_##B
#define CONCAT2(A, B) CONCAT(A, B)
#define CHECK_MALLOC(PREFIX) static size_t CONCAT(PREFIX,malloced_size) = 0
#define CHECK_MALLOCED_SIZE(PREFIX)      assert(CONCAT(PREFIX,malloced_size) == 0)
#define CHECK_MALLOCED_INC_SIZE(PREFIX, n) (CONCAT(PREFIX,malloced_size) += (n))
#define CHECK_MALLOCED_DEC_SIZE(PREFIX, n) (CONCAT(PREFIX,malloced_size) -= (n))
CHECK_MALLOC(MEMORY_PREFIX);
#endif /*defined(USE_CHECK_MALLOC)*/

#define DO_BZERO  CONCAT2(MEMORY_PREFIX, do_bzero)
#define DO_MALLOC CONCAT2(MEMORY_PREFIX, do_malloc)
#define DO_FREE   CONCAT2(MEMORY_PREFIX, do_free)
static inline void DO_BZERO (void *ptr, size_t size)
{
    memset(ptr, 0, size);
}

static inline void *DO_MALLOC (size_t size)
{
    void *ptr = malloc(size);
    DO_BZERO (ptr, size);
    CHECK_MALLOCED_INC_SIZE(MEMORY_PREFIX, size);
    return ptr;
}

static inline void DO_FREE (void *ptr, size_t size)
{
    DO_BZERO (ptr, size);
    CHECK_MALLOCED_DEC_SIZE(MEMORY_PREFIX, size);
    free(ptr);
}

#endif /* end of include guard */
