#include "pool_plugin.h"
#include "io.h"

#ifdef __cplusplus
extern "C" {
#endif

static bool stream_apply(struct pool_plugin *_p, struct LogEntry *e, uint32_t state)
{
    struct pool_plugin_stream *p = (struct pool_plugin_stream *) _p;
    io_write(p->io, e, e->h.size);
    return true;
}

static bool stream_failed(struct pool_plugin *p, struct LogEntry *e, uint32_t state)
{
    return true;
}

static struct pool_plugin *pool_plugin_stream_create(struct pool_plugin *_p)
{
    struct pool_plugin_stream *p = (struct pool_plugin_stream *) _p;
    p->base.apply  = pool_plugin_init(_p->apply);
    p->base.failed = pool_plugin_init(_p->failed);
    p->base.Apply  = stream_apply;
    p->base.Failed = stream_failed;
    p->base.name = "stream";
    assert(p->io);
    return _p;
}

static void pool_plugin_stream_dispose(struct pool_plugin *_p)
{
    struct pool_plugin_stream *p = (struct pool_plugin_stream *) _p;
    CHECK_PLUGIN("stream", _p);
    pool_plugin_dispose(p->base.apply);
    pool_plugin_dispose(p->base.failed);
    io_close(p->io);
    bzero(p, sizeof(struct pool_plugin_stream));
    free(p);
}

EXPORT_POOL_PLUGIN(pool_plugin_stream) = {
    {0, NULL, NULL, pool_plugin_stream_create, pool_plugin_stream_dispose, stream_apply, stream_failed, NULL},
    NULL
};

#ifdef __cplusplus
}
#endif
