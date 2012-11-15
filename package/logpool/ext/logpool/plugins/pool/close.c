#include "pool_plugin.h"
#include <event2/bufferevent.h>

#ifdef __cplusplus
extern "C" {
#endif

static bool close_Apply(struct pool_plugin *_p, struct LogEntry *e, uint32_t state)
{
    struct pool_plugin_close *p = (struct pool_plugin_close *) _p;
    //fprintf(stderr, "%s:%d close %p\n", __FILE__, __LINE__, p->bev);
    bufferevent_Free(p->bev);
    return false;
}

static bool close_failed(struct pool_plugin *p, struct LogEntry *e, uint32_t state)
{
    return false;
}

static struct pool_plugin *pool_plugin_close_create(struct pool_plugin *_p)
{
    struct pool_plugin_close *p = (struct pool_plugin_close *) _p;
    p->base.apply  = pool_plugin_Init(_p->apply);
    p->base.failed = pool_plugin_Init(_p->failed);
    p->base.Apply  = close_Apply;
    p->base.Failed = close_failed;
    p->base.name = "close";
    assert(p->bev);
    return _p;
}

static void pool_plugin_close_dispose(struct pool_plugin *p)
{
    CHECK_PLUGIN("close", p);
    pool_plugin_dispose(p->apply);
    pool_plugin_dispose(p->failed);
    bzero(p, sizeof(struct pool_plugin_close));
    free(p);
}

EXPORT_POOL_PLUGIN(pool_plugin_close) = {
    {0, NULL, NULL, pool_plugin_close_create, pool_plugin_close_dispose, close_Apply, close_failed, NULL},
    NULL
};

#ifdef __cplusplus
}
#endif
