#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "list.h"

DEF_LIST_T(int);
DEF_LIST_OP(int);

static void LIST_dump(ELEM(int) *e, void *arg)
{
    fprintf(stderr, "%d\n", e->v);
}

int main(int argc, char const* argv[])
{
    LIST(int) *l = LIST_new(int);
    unsigned i;
    for (i = 0; i < 10; ++i) {
        LIST_append(int, l, i);
    }
    LIST_each(int, l, LIST_dump, NULL);
    for (i = 10; i < 20; ++i) {
        LIST_append(int, l, i);
    }
    LIST_each(int, l, LIST_dump, NULL);

    ELEM(int) *e;
    LIST_FOR_EACH(l, e) {
        fprintf(stderr, "%d\n", e->v);
    }
    LIST_delete(int, l);
    return 0;
}
