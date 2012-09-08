#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ARRAY */
#define ARRAY(T) ARRAY_##T##_t
#define DEF_ARRAY_STRUCT0(T, SizeTy) \
struct ARRAY(T) {\
	T *list;\
	SizeTy size;  \
	SizeTy capacity;  \
}
#define DEF_ARRAY_STRUCT(T) DEF_ARRAY_STRUCT0(T, int)

#define DEF_ARRAY_T(T)              \
struct ARRAY(T);                    \
typedef struct ARRAY(T) ARRAY(T)

#define DEF_ARRAY_OP(T)\
static inline ARRAY(T) *ARRAY_init_##T (ARRAY(T) *a, size_t initsize) {\
	a->list = (T*) malloc(sizeof(T)*initsize);\
	a->capacity  = initsize;\
	a->size  = 0;\
	return a;\
}\
static inline T *ARRAY_##T##_get(ARRAY(T) *a, int idx) {\
	return a->list+idx;\
}\
static inline void ARRAY_##T##_set(ARRAY(T) *a, int idx, T *v){ \
	memcpy(a->list+idx, v, sizeof(T));\
}\
static inline void ARRAY_##T##_add(ARRAY(T) *a, T *v) {\
	if (a->size + 1 >= a->capacity) {\
		a->capacity *= 2;\
		a->list = (T*)realloc(a->list, sizeof(T) * a->capacity);\
	}\
	ARRAY_##T##_set(a, a->size++, v);\
}\
static inline void ARRAY_##T##_dispose(ARRAY(T) *a) {\
	free(a->list);\
	a->size     = 0;\
	a->capacity = 0;\
	a->list     = NULL;\
}

#define DEF_ARRAY_T_OP(T) DEF_ARRAY_T(T);DEF_ARRAY_OP(T)

#define ARRAY_get(T, a, idx)    ARRAY_##T##_get(a, idx)
#define ARRAY_set(T, a, idx, v) ARRAY_##T##_set(a, idx, v)
#define ARRAY_add(T, a, v)      ARRAY_##T##_add(a, v)
#define ARRAY_dispose(T, a)     ARRAY_##T##_dispose(a)
#define ARRAY_init(T, a, s)     ARRAY_init_##T (a, s)
#define ARRAY_n(a, n)  ((a).list+n)
#define ARRAY_size(a)  ((a).size)
#define ARRAY_last(a)  ARRAY_n(a, ((a).size-1))
#define ARRAY_init_1(T, a, e1) do {\
	ARRAY_init(T, a);\
	ARRAY_add(T, a, e1);\
} while (0)

#define FOR_EACH_ARRAY_(a, x, i) \
		for(i=0, x = ARRAY_n(a, i); i < ARRAY_size(a); x = ARRAY_n(a,(++i)))

#define FOR_EACH_ARRAY(a, x, e) \
		for(x = ARRAY_n(a, 0), e = ARRAY_n(a,ARRAY_size(a)); x != e; ++x)

#ifdef __cplusplus
}
#endif
