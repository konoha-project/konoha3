#include "pool_plugin.h"
#include <event2/bufferevent.h>

#ifdef __cplusplus
extern "C" {
#endif

static bool response_apply(struct pool_plugin *_p, struct LogEntry *e, uint32_t state)
{
    struct pool_plugin_response *p = (struct pool_plugin_response *) _p;
    bufferevent_write(p->bev, &e->data, e->h.size-sizeof(struct LogHead));
    _p->apply->Apply(_p->apply, e, state);
    return true;
}

static bool response_failed(struct pool_plugin *_p, struct LogEntry *e, uint32_t state)
{
    //TODO
    _p->failed->Apply(_p->failed, e, state);
    return true;
}

static struct pool_plugin *pool_plugin_response_create(struct pool_plugin *_p)
{
    struct pool_plugin_response *p = (struct pool_plugin_response *) _p;
    p->base.apply  = pool_plugin_init(_p->apply);
    p->base.failed = pool_plugin_init(_p->failed);
    p->base.Apply  = response_apply;
    p->base.Failed = response_failed;
    p->base.name = "response";
    assert(p->bev);
    return _p;
}

static void pool_plugin_response_dispose(struct pool_plugin *p)
{
    CHECK_PLUGIN("response", p);
    pool_plugin_dispose(p->apply);
    pool_plugin_dispose(p->failed);
    bzero(p, sizeof(struct pool_plugin_response));
    free(p);
}

EXPORT_POOL_PLUGIN(pool_plugin_response) = {
    {0, NULL, NULL, pool_plugin_response_create, pool_plugin_response_dispose, response_apply, response_failed, NULL},
    NULL
};

#ifdef __cplusplus
}
#endif
