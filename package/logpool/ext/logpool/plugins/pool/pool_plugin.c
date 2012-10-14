#include "pool_plugin.h"
#include "array.h"
#include "protocol.h"
#include "minikonoha/minikonoha.h"
#include "minikonoha/sugar.h"
#if 1
#include "llcache.h"
#endif
#include "libmemcached/memcached.h"
#include <string.h>
#include <time.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

static void pool_plugin_nop_dispose(struct pool_plugin *p);
static struct pool_plugin *pool_plugin_nop_create(struct pool_plugin *_p);
static bool nop_apply(struct pool_plugin *p, struct LogEntry *e, uint32_t state);
static bool nop_failed(struct pool_plugin *p, struct LogEntry *e, uint32_t state);

struct pool_plugin_nop {
    struct pool_plugin base;
};

struct pool_plugin_nop pool_nop_plugin = {
    {0, NULL, NULL, pool_plugin_nop_create, pool_plugin_nop_dispose, nop_apply, nop_failed, "nop"}
};

static bool nop_apply(struct pool_plugin *p, struct LogEntry *e, uint32_t state)
{
    return true;
}

static bool nop_failed(struct pool_plugin *p, struct LogEntry *e, uint32_t state)
{
    return true;
}

static struct pool_plugin *pool_plugin_nop_create(struct pool_plugin *_p)
{
    return (struct pool_plugin *) &pool_nop_plugin;
}

static void pool_plugin_nop_dispose(struct pool_plugin *p)
{
    CHECK_PLUGIN("nop", p);
}

struct pool_plugin *pool_plugin_init(struct pool_plugin *p)
{
    if(!p)
        return pool_plugin_nop_create(NULL);
    if(p->flags) {
        /* already inited */
        return p;
    }
    struct pool_plugin *_p = p->Create(p);
    _p->flags |= 1;
    return _p;
}

struct pool_plugin *pool_plugin_clone(struct pool_plugin *p, uint32_t size)
{
    struct pool_plugin *newp = (struct pool_plugin *) malloc(size);
    memcpy(newp, p, size);
    return newp;
}

void pool_process_log(struct pool_plugin *p, struct LogEntry *e)
{
    p->Apply(p, e, 0);
}

void pool_plugin_dispose(struct pool_plugin *p)
{
    if(p->Dispose)
        p->Dispose(p);
}

char *LogEntry_get(struct LogEntry *e, char *key, int klen, int *vlen)
{
    uint16_t i;
    struct Log *log = (struct Log*)&e->data;
    char *data = log_get_data(log);
    for (i = 0; i < log->logsize; ++i) {
        char *next = log_iterator(log, data, i);
        uint16_t len0 = log_get_length(log, i*2+0);
        uint16_t len1 = log_get_length(log, i*2+1);
        if(klen == len0 && strncmp(key, data, klen) == 0) {
            *vlen = len1;
            return data+klen;
        }
        data = next;
    }
    return NULL;
}

typedef struct pool_plugin pool_plugin_t;
typedef struct connection {
    pool_plugin_t *p;
    struct bufferevent *bev;
} conn_t;

DEF_ARRAY_STRUCT0(conn_t, uint32_t);
DEF_ARRAY_T(conn_t);
DEF_ARRAY_OP(conn_t);

//#define USE_LLVM
#define USE_KONOHA 1
struct pool_list {
#if USE_KONOHA
    KonohaContext *konoha;
    memcached_st *mc;
#else
    llcache_t *mc;
#endif
    ARRAY(conn_t) list;
};

static inline uint64_t TimeMilliSecond(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec;
}

void pool_exec(struct Log *log, int logsize, struct pool_list *plist)
{
    uint32_t size = sizeof(struct LogHead) + logsize;
    char buffer[size];
    struct LogEntry *newe = (struct LogEntry *) buffer;
    newe->h.next = NULL;
    newe->h.size = size;
    newe->h.time = TimeMilliSecond();
    RefInit(newe);
    memcpy(&newe->data, log, logsize);
    conn_t *c, *ce;
    FOR_EACH_ARRAY(plist->list, c, ce) {
        struct pool_plugin *p = c->p;
        if(p) {
            p->Apply(p, newe, 0);
        }
    }
}

typedef pool_plugin_t *(*fpool_plugin_init)(struct bufferevent *bev);
void konoha_plugin_init(KonohaContextVar **kctxp, memcached_st **mcp);
pool_plugin_t *konoha_plugin_get(KonohaContext *kctx, memcached_st *mc, char *buf, size_t len, void *req);

void pool_add(struct Procedure *q, struct bufferevent *bev, struct pool_list *l)
{
    char buf[128] = {};
    memcpy(buf, q->data, q->vlen);
    fprintf(stderr, "procedure: '%s':%d\n", buf, q->vlen);
#if USE_LLVM
    memcpy(buf+q->vlen, "_init", 6);
#endif
#if USE_KONOHA
    pool_plugin_t *plugin = konoha_plugin_get(l->konoha, l->mc, buf, strlen(buf), bev);
    if(plugin == NULL)
        fprintf(stderr, "%s was not loaded.\n", buf);
#else
    fpool_plugin_init finit = (fpool_plugin_init) llcache_get(l->mc, buf);
    pool_plugin_t *plugin = finit(bev);
#endif
    conn_t conn;
    conn.p = plugin;
    conn.bev = bev;
    ARRAY_add(conn_t, &l->list, &conn);
}

void pool_delete_connection(struct pool_list *l, struct bufferevent *bev)
{
    conn_t *c, *ce;
    FOR_EACH_ARRAY(l->list, c, ce) {
        if(c->bev == bev) {
            fprintf(stderr, "connection dispose; %p, %p\n", c->bev, c->p);
            c->bev = NULL;
            c->p = NULL;
        }
    }
}

extern void konoha_plugin_init(KonohaContextVar **konohap, memcached_st **mcp);
struct pool_list * pool_new(void)
{
    struct pool_list *l = malloc(sizeof(struct pool_list));
#if USE_KONOHA
    konoha_plugin_init((KonohaContextVar**)&l->konoha, &l->mc);
#else
    l->mc = llcache_new("127.0.0.1", 11211);
#endif
    ARRAY_init(conn_t, &l->list, 4);
    return l;
}

void pool_delete(struct pool_list *l)
{
#if USE_KONOHA
    konoha_close(l->konoha);
#else
    llcache_delete(l->mc);
#endif
    ARRAY_dispose(conn_t, &l->list);
    free(l);
}

#ifdef __cplusplus
}
#endif
