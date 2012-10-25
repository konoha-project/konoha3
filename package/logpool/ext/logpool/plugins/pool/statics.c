#include "pool_plugin.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

static bool statics_apply(struct pool_plugin *_p, struct LogEntry *e, uint32_t state)
{
    struct pool_plugin_statics *p = (struct pool_plugin_statics *) _p;
    uintptr_t data = 0;
    switch (state) {
        case 0:
            p->context = p->finit(p->context);
            return true;
        case 1:
            p->context = p->function(p->context, e);
            return true;
        case 2:
            data = p->fexit(p->context);
            break;
        default:
            fprintf(stderr, "%s:%d Error\n", __func__, __LINE__);
    }
    if(data) {
        p->base.apply->Apply(p->base.apply, e, (uint32_t) data);
    } else {
        p->base.failed->Apply(p->base.failed, e, (uint32_t) data);
    }
    return true;
}

static bool statics_failed(struct pool_plugin *p, struct LogEntry *e, uint32_t state)
{
    return true;
}

static struct pool_plugin *pool_plugin_statics_create(struct pool_plugin *_p)
{
    struct pool_plugin_statics *p = (struct pool_plugin_statics *) _p;
    p->base.apply  = pool_plugin_init(_p->apply);
    p->base.failed = pool_plugin_init(_p->failed);
    p->base.Apply  = statics_apply;
    p->base.Failed = statics_failed;
    p->base.name = "statics";
    return _p;
}

static void pool_plugin_statics_dispose(struct pool_plugin *p)
{
    CHECK_PLUGIN("statics", p);
    pool_plugin_dispose(p->apply);
    pool_plugin_dispose(p->failed);
    bzero(p, sizeof(struct pool_plugin_statics));
    free(p);
}

static uintptr_t static_nop_init(uintptr_t context)
{
    return 0;
}

static uintptr_t static_nop_exit(uintptr_t context)
{
    return 0;
}

static uintptr_t static_nop_function(uintptr_t context, struct LogEntry *e)
{
    return 0;
}

EXPORT_POOL_PLUGIN(pool_plugin_statics) = {
    {0, NULL, NULL, pool_plugin_statics_create, pool_plugin_statics_dispose, statics_apply, statics_failed, NULL},
    0,
    static_nop_init,
    static_nop_exit,
    static_nop_function
};

#ifdef __cplusplus
}
#endif
