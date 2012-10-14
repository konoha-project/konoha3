#include "pool_plugin.h"
#include "protocol.h"
#include <event2/bufferevent.h>

#ifdef __cplusplus
extern "C" {
#endif

static char *emit_message_header(char *buf, uint16_t protocol, uint16_t logsize, uint16_t lengths[])
{
    uint16_t i, *loginfo;
    struct Log *tmp = (struct Log *) buf;

    tmp->protocol = protocol;
    tmp->logsize  = logsize;
    loginfo = ((uint16_t*)buf) + LOG_PROTOCOL_FIELDS;
    buf = (char *) (loginfo + logsize * 2);
    for (i = 0; i < logsize; ++i) {
        loginfo[i*2+0] = (uint16_t) lengths[i*2+0];
        loginfo[i*2+1] = (uint16_t) lengths[i*2+1];
    }
    return buf;
}

static bool create_apply(struct pool_plugin *_p, struct LogEntry *e, uint32_t state)
{
    struct pool_plugin_create *p = (struct pool_plugin_create *) _p;
    uint16_t i, logsize, datasize = 0;
    uint16_t lengths[32];
    p->context = p->finit(p->context, state);
    logsize  = p->write_size(p->context, state, lengths);
    for (i = 0; i < logsize; ++i) {
        datasize += lengths[i*2+0];
        datasize += lengths[i*2+1];
    }
    uint16_t entry_size = sizeof(struct LogEntry)+sizeof(uint16_t)*2*logsize+datasize;
    struct LogEntry *newe = malloc(entry_size);
    char *buf = emit_message_header((char*)&newe->data, LOGPOOL_EVENT_WRITE, logsize, lengths);
    newe->h.size = entry_size;
    newe->h.next = NULL;
    newe->h.time = e->h.time;
    p->write_data(p->context, e, buf);
    RefInit(newe);
    p->context = p->fexit(p->context);
    p->base.apply->Apply(p->base.apply, newe, 0);
    return true;
}

static bool create_failed(struct pool_plugin *p, struct LogEntry *e, uint32_t state)
{
    //TODO
    return true;
}

static struct pool_plugin *pool_plugin_create_create(struct pool_plugin *_p)
{
    struct pool_plugin_create *p = (struct pool_plugin_create *) _p;
    p->base.apply  = pool_plugin_init(_p->apply);
    p->base.failed = pool_plugin_init(_p->failed);
    p->base.Apply  = create_apply;
    p->base.Failed = create_failed;
    p->base.name = "create";
    return _p;
}

static void pool_plugin_create_dispose(struct pool_plugin *p)
{
    CHECK_PLUGIN("create", p);
    pool_plugin_dispose(p->apply);
    pool_plugin_dispose(p->failed);
    bzero(p, sizeof(struct pool_plugin_create));
    free(p);
}

static uint16_t nop_write_size(uintptr_t context, uint32_t state, uint16_t lengths[])
{
    return 0;
}

static void nop_write_data(uintptr_t context, struct LogEntry *e, char *buf)
{
    //for (i = 0; i < logsize; ++i) {
    //    if(klen) {
    //        memcpy(buf, key, klen);
    //        buf = buf + klen;
    //    }
    //    if(vlen) {
    //        memcpy(buf, val, vlen);
    //        buf = buf + vlen;
    //    }
    //}
}

static uintptr_t nop_init(uintptr_t context, uint32_t t) { return 0; }
static uintptr_t nop_exit(uintptr_t context) { return 0; }

EXPORT_POOL_PLUGIN(pool_plugin_create) = {
    {0, NULL, NULL, pool_plugin_create_create, pool_plugin_create_dispose, create_apply, create_failed, NULL},
    0, nop_init, nop_exit, nop_write_size, nop_write_data
};

#ifdef __cplusplus
}
#endif
