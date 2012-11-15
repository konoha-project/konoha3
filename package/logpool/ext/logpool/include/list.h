#ifndef LIST_H
#define LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#define LIST(T) LIST_##T##_t
#define ELEM(T) ELEM_##T##_t
#define DEF_LIST_STRUCT(T) struct LIST(T) { struct ELEM(T) *head, *tail;}
#define DEF_LIST_ELEMENT_STRUCT(T) struct ELEM(T) { struct ELEM(T) *next; T v; }

#define ELEM_OP(T, OP) ELEM_##T##_##OP
#define LIST_OP(T, OP) LIST_##T##_##OP

#define DEF_LIST_T(T)\
struct ELEM(T);\
struct LIST(T);\
DEF_LIST_ELEMENT_STRUCT(T);\
DEF_LIST_STRUCT(T);\
typedef struct ELEM(T) ELEM(T);\
typedef struct LIST(T) LIST(T)

#ifndef LIST_ASSIGNMENT
#define LIST_ASSIGNMENT(ELM, VAL) (ELM)->v = VAL
#endif

#define DEF_LIST_OP(T)\
static inline ELEM(T) *ELEM_OP(T, create) () {\
    ELEM(T) *e = malloc(sizeof(ELEM(T)));\
    bzero(e, sizeof(struct ELEM(T)));\
    return e;\
}\
static inline void LIST_OP(T, append) (LIST(T) *list, T v) {\
    ELEM(T) *e;\
    e = ELEM_OP(T,create)();\
    LIST_ASSIGNMENT(e, v);\
    list->tail->next = e;\
    list->tail = e;\
}\
static inline void LIST_OP(T, each) (LIST(T) *list, void (*f)(ELEM(T) *e, void *arg), void *arg) {\
    ELEM(T) *e = list->head->next, *next;\
    while(e) {\
        next = e->next;\
        f(e, arg);\
        e = next;\
    }\
}\
static void LIST_OP(T, init) (LIST(T) *list) {\
    ELEM(T) *head = ELEM_OP(T,create)();\
    list->head = head;\
    list->tail = head;\
}\
static inline LIST(T) *LIST_OP(T, new)() {\
    LIST(T) *list = malloc(sizeof(LIST(T)));\
    LIST_OP(T, init)(list);\
    return list;\
}\
static void LIST_OP(T, delete_helper)(ELEM(T) *elem, void *arg) {\
    free(elem/*, sizeof(ELEM(T))*/);\
}\
static inline void LIST_OP(T, dispose)(LIST(T) *list) {\
    LIST_OP(T, each)(list, LIST_OP(T, delete_helper), NULL);\
}\
static inline void LIST_OP(T, delete)(LIST(T) *list) {\
    LIST_OP(T, dispose)(list);\
    free(list/*, sizeof(LIST(T))*/);\
}

#define LIST_new(T)           LIST_OP(T, new)()
#define LIST_Init(T, L)       LIST_OP(T, init)(L)
#define LIST_dispose(T, L)    LIST_OP(T, dispose)(L)
#define LIST_delete(T, L)     LIST_OP(T, delete)(L)
#define LIST_append(T, L, E)  LIST_OP(T, append)(L, E)
#define LIST_each(T, L, F, A) LIST_OP(T, each)(L, F, A)
#define LIST_head(L)       ((L)->head)
#define LIST_tail(L)       ((L)->tail)
#define LIST_next(L)       ((L)->next)

#define LIST_FOR_EACH(L, X) for (X = LIST_next(LIST_head(L)); X; X = LIST_next(X))

#ifdef __cplusplus
}
#endif
#endif /* end of include guard */
