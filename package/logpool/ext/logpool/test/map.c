#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void poolmap_dump(poolmap_t *m)
{
    poolmap_iterator itr = {0};
    pmap_record_t *r;
    int i = 0;
    while ((r = poolmap_next(m, &itr)) != NULL) {
        ++i;
#if 0
        fprintf(stderr, "{h=0x%08x, key=%s, v=%s}\n",
                r->hash, (char *)r->k, (char *)r->v);
#endif
    }
    fprintf(stderr, "poolmap.size:%d\n", poolmap_size(m));
    assert(poolmap_size(m) == 38201);
}

static void load(poolmap_t *m, const char *fname)
{
    char buffer[32];
    FILE *fp = fopen(fname, "r");
    while(fgets(buffer , sizeof(buffer), fp) != NULL) {
        int len = strlen(buffer);
        uint32_t size = len+sizeof(uint32_t);
        uint32_t *str = (uint32_t *) malloc(size);
        bzero(str, size);
        str[0] = len;
        memcpy(str+1, buffer, len-1);
        //fprintf(stderr, "key '%s' %d %p\n", (char *)(str+1), len, str);
        poolmap_set(m, (char *)(str+1), len-1, (char *)(str+1));
    }
    fclose(fp);
}

static int entry_key_eq(uintptr_t k0, uintptr_t k1)
{
    uint32_t *l0 = (uint32_t *)k0;
    uint32_t *l1 = (uint32_t *)k1;
    char *s0 = (char *) (l0);
    char *s1 = (char *) (l1);
    //fprintf(stderr, "cmp '%s' %d, '%s' %d %p %p\n", s0, l0[-1], s1, l1[-1], s0, s1);
    assert(l0[-1] > 0 && l1[-1] > 0);
    assert(*s0 > 0);
    return l0[-1] == l1[-1] && strncmp(s0, s1, l0[-1]) == 0;
}

static void entry_free(pmap_record_t *r)
{
    uint32_t *l = (uint32_t *)r->k;
    char *s0 = (char *) l;
    assert(l[-1] > 0);
    //fprintf(stderr, "fre '%s' %d %p\n", s0, l[-1], s0);
    memset(s0, 0xa, l[-1]);
    free(l-1);
}

static uintptr_t entry_keygen(char *key, uint32_t len)
{
    return (uintptr_t) key;
}

int main(int argc, char const *argv[])
{
    int i;
    pool_global_init();
    poolmap_t *map = poolmap_new(4, entry_keygen, entry_key_eq, entry_free);
    assert(argc == 2);
    for (i = 0; i < 100; ++i) {
        load(map, argv[1]);
    }
    poolmap_dump(map);
    poolmap_delete(map);
    pool_global_deinit();
    return 0;
}
