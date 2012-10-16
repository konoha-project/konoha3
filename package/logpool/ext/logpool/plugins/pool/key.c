#include "pool_plugin.h"
#include "protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

static bool key_apply(struct pool_plugin *_p, struct LogEntry *e, uint32_t state)
{
    struct pool_plugin_key_filter *p = (struct pool_plugin_key_filter *) _p;
    struct Log *log = (struct Log*)&e->data;
    uint16_t i;
    char *data = log_get_data(log);
    for (i = 0; i < log->logsize; ++i) {
        char *next = log_iterator(log, data, i);
        uint16_t klen = log_get_length(log, i*2+0);
        if(klen == p->klen && strncmp(p->key, data, klen) == 0) {
            _p->apply->Apply(_p->apply, e, 0);
            return true;
        }
        data = next;
    }
    return false;
}

static bool key_failed(struct pool_plugin *p, struct LogEntry *e, uint32_t state)
{
    return true;
}

static struct pool_plugin *pool_plugin_key_create(struct pool_plugin *_p)
{
    struct pool_plugin_key_filter *p = (struct pool_plugin_key_filter *) _p;
    p->base.apply  = pool_plugin_init(_p->apply);
    p->base.failed = pool_plugin_init(_p->failed);
    p->base.Apply  = key_apply;
    p->base.Failed = key_failed;
    p->base.name = "key";
    return _p;
}

static void pool_plugin_key_dispose(struct pool_plugin *p)
{
    CHECK_PLUGIN("key", p);
    pool_plugin_dispose(p->apply);
    pool_plugin_dispose(p->failed);
    bzero(p, sizeof(struct pool_plugin_key_filter));
    free(p);
}

EXPORT_POOL_PLUGIN(pool_plugin_key_filter) = {
    {0, NULL, NULL, pool_plugin_key_create, pool_plugin_key_dispose, key_apply, key_failed, NULL},
    NULL, 0
};

#ifdef __cplusplus
}
#endif
