#include <libmemcached/memcached.h>
#include <stdio.h>
#include "logpool.h"
#include "lpstring.h"
#include "logpool_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mc {
    char *buf;
    memcached_st *st;
    char base[1];
} mc_t;

#define USE_BUFFER_REQ 1
void *logpool_memcache_init(logpool_t *ctx, logpool_param_t *p)
{
    struct logpool_param_memcache *args = cast(struct logpool_param_memcache *, p);
    const char *host = args->host;
    long port = args->port;
    mc_t *mc = cast(mc_t *, logpool_string_init(ctx, p));
    memcached_return_t rc;
    memcached_server_list_st servers;

    mc->st = memcached_create(NULL);
#if 0
    memcached_behavior_set (mc->st, MEMCACHED_BEHAVIOR_NO_BLOCK, 1);
#endif
#ifdef USE_BUFFER_REQ
    memcached_behavior_set (mc->st, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS, 1);
#endif
    if(unlikely(mc->st == NULL)) {
        /* TODO Error */
        abort();
    }
    servers = memcached_server_list_append(NULL, host, port, &rc);
    if(unlikely(rc != MEMCACHED_SUCCESS)) {
        /* TODO Error */
        fprintf(stderr, "Error!! '%s'\n", memcached_strerror(mc->st, rc));
        abort();
    }
    rc = memcached_server_push(mc->st, servers);
    if(unlikely(rc != MEMCACHED_SUCCESS)) {
        /* TODO Error */
        fprintf(stderr, "Error!! '%s'\n", memcached_strerror(mc->st, rc));
        abort();
    }
    memcached_server_list_free(servers);
    return cast(void *, mc);
}

void logpool_memcache_close(logpool_t *ctx)
{
    mc_t *mc = cast(mc_t *, ctx->connection);
    memcached_st *st = mc->st;
#ifdef USE_BUFFER_REQ
    memcached_return_t rc = memcached_flush_buffers(st);
    if(unlikely(rc != MEMCACHED_SUCCESS)) {
        /* TODO Error */
        fprintf(stderr, "Error!! '%s'\n", memcached_strerror(mc->st, rc));
        abort();
    }
#endif
    memcached_free(st);
    logpool_string_close(ctx);
}

static void logpool_memcache_flush(logpool_t *ctx, void **args __UNUSED__)
{
    mc_t *mc = cast(mc_t *, ctx->connection);
    char key[128] = {0}, *buf_orig = mc->buf, *p;
    const char *value = mc->base;
    uint32_t flags = 0;
    size_t klen, vlen;
    memcached_return_t rc;
    struct logfmt *fmt = cast(struct logpool *, ctx)->fmt;
    size_t i, size = ctx->logfmt_size;

    mc->buf = key;
    p = ctx->fn_key(ctx, ctx->logkey.v.u, ctx->logkey.k.seq, ctx->logkey.klen);
    klen = p - (char*) key;
    mc->buf = buf_orig;

    if(size) {
        void (*fn_delim)(logpool_t *) = ctx->formatter->fn_delim;
        fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->klen, fmt->vlen);
        ++fmt;
        for (i = 1; i < size; ++i, ++fmt) {
            fn_delim(ctx);
            fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->klen, fmt->vlen);
        }
        cast(struct logpool *, ctx)->logfmt_size = 0;
    }

    vlen = (char*) mc->buf - value;
    rc = memcached_set(mc->st, key, klen, value, vlen, 0, flags);
#ifdef USE_BUFFER_REQ
    if(unlikely(rc == MEMCACHED_BUFFERED)) {
    }
    else
#endif
    if(unlikely(rc != MEMCACHED_SUCCESS)) {
        /* TODO Error */
        fprintf(stderr, "Error!! '%s'\n", memcached_strerror(mc->st, rc));
        abort();
    }
    logpool_string_reset(ctx);
    ++(cast(struct logpool *, ctx)->logkey.k.seq);
}

struct logapi MEMCACHE_API = {
    logpool_string_null,
    logpool_string_bool,
    logpool_string_int,
    logpool_string_hex,
    logpool_string_float,
    logpool_string_char,
    logpool_string_string,
    logpool_string_raw,
    logpool_string_delim,
    logpool_memcache_flush,
    logpool_memcache_init,
    logpool_memcache_close,
    logpool_default_priority
};

#ifdef __cplusplus
}
#endif
