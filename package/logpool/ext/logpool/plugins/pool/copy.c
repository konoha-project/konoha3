#include "pool_plugin.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

static bool copy_apply(struct pool_plugin *p, struct LogEntry *e, uint32_t state)
{
    struct LogEntry *newe = malloc(e->h.size);
    memcpy(newe, e, e->h.size);
    RefInit(newe);
    p->apply->Apply(p->apply, newe, 0);
    return true;
}

static bool copy_failed(struct pool_plugin *p, struct LogEntry *e, uint32_t state)
{
    //TODO
    return true;
}

static struct pool_plugin *pool_plugin_copy_create(struct pool_plugin *_p)
{
    struct pool_plugin *p = (struct pool_plugin *) _p;
    p->apply  = pool_plugin_init(_p->apply);
    p->failed = pool_plugin_init(_p->failed);
    p->Apply  = copy_apply;
    p->Failed = copy_failed;
    p->name = "copy";
    return p;
}

static void pool_plugin_copy_dispose(struct pool_plugin *p)
{
    CHECK_PLUGIN("copy", p);
    pool_plugin_dispose(p->apply);
    pool_plugin_dispose(p->failed);
    bzero(p, sizeof(struct pool_plugin_copy));
    free(p);
}

EXPORT_POOL_PLUGIN(pool_plugin_copy) = {
    {0, NULL, NULL, pool_plugin_copy_create, pool_plugin_copy_dispose, copy_apply, copy_failed, NULL}
};

#ifdef __cplusplus
}
#endif
