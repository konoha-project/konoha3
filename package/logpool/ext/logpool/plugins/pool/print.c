#include "pool_plugin.h"
#include "protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

static bool print_apply(struct pool_plugin *p, struct LogEntry *e, uint32_t state)
{
    log_dump(stderr, "", (struct Log *) &e->data, "\n", 1);
    p->apply->Apply(p->apply, e, state);
    return true;
}

static bool print_failed(struct pool_plugin *p, struct LogEntry *e, uint32_t state)
{
    //TODO
    return true;
}

static struct pool_plugin *pool_plugin_print_create(struct pool_plugin *_p)
{
    struct pool_plugin *p = (struct pool_plugin *) _p;
    p->apply  = pool_plugin_init(_p->apply);
    p->failed = pool_plugin_init(_p->failed);
    p->Apply  = print_apply;
    p->Failed = print_failed;
    p->name = "print";
    return p;
}

static void pool_plugin_print_dispose(struct pool_plugin *p)
{
    CHECK_PLUGIN("print", p);
    pool_plugin_dispose(p->apply);
    pool_plugin_dispose(p->failed);
    bzero(p, sizeof(struct pool_plugin_print));
    free(p);
}

EXPORT_POOL_PLUGIN(pool_plugin_print) = {
    {0, NULL, NULL, pool_plugin_print_create, pool_plugin_print_dispose, print_apply, print_failed, NULL}
};

#ifdef __cplusplus
}
#endif
