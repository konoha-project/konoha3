#include <stdlib.h>
#include "logpool.h"
#include "logpool_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct filter {
    int   priority;
    int   emitLog;
    void *connection;
    logapi_t *api;
    keyFn     fn_key;
} filter_t;

static char *logpool_Filter_fn_key(logpool_t *ctx, uint64_t v, uint64_t seq, short klen)
{
    char *ret;
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logpool *, ctx)->connection = filter->connection;
    ret = filter->fn_key(ctx, v, seq, klen);
    cast(struct logpool *, ctx)->connection = filter;
    return ret;
}

static void *logpool_Filter_init(logpool_t *ctx, logpool_param_t *p)
{
    struct logpool_param_filter *args = cast(struct logpool_param_filter *, p);
    filter_t *filter = cast(filter_t *, malloc(sizeof(*filter)));
    filter->api = args->api;
    filter->emitLog    = 1;
    filter->priority   = args->priority;
    filter->connection = args->api->fn_init(ctx, args->param);
    filter->fn_key = ctx->fn_key;
    cast(struct logpool *, ctx)->fn_key = logpool_Filter_fn_key;
    return cast(void *, filter);
}

static void logpool_Filter_close(logpool_t *ctx)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logpool *, ctx)->connection = filter->connection;
    filter->api->fn_close(ctx);
    free(filter);
}

static void logpool_Filter_flush(logpool_t *ctx, void **args)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    if(filter->emitLog) {
        logpool_format_flush(ctx);
        cast(struct logpool *, ctx)->connection = filter->connection;
        filter->api->fn_flush(ctx, args);
        cast(struct logpool *, ctx)->connection = filter;
    }
}

static int logpool_Filter_priority(logpool_t *ctx, int priority)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    filter->emitLog  = (priority <= filter->priority);
    return filter->emitLog;
}

static void logpool_Filter_null(logpool_t *ctx, const char *key, uint64_t v, short klen, short vlen)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logpool *, ctx)->connection = filter->connection;
    filter->api->fn_null(ctx, key, v, klen, vlen);
    cast(struct logpool *, ctx)->connection = filter;
}

static void logpool_Filter_bool(logpool_t *ctx, const char *key, uint64_t v, short klen, short vlen)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    logpool_context_switch(ctx, filter->connection);
    filter->api->fn_bool(ctx, key, v, klen, vlen);
    logpool_context_switch(ctx, filter);
}

static void logpool_Filter_int(logpool_t *ctx, const char *key, uint64_t v, short klen, short vlen)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logpool *, ctx)->connection = filter->connection;
    filter->api->fn_int(ctx, key, v, klen, vlen);
    cast(struct logpool *, ctx)->connection = filter;
}

static void logpool_Filter_hex(logpool_t *ctx, const char *key, uint64_t v, short klen, short vlen)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logpool *, ctx)->connection = filter->connection;
    filter->api->fn_hex(ctx, key, v, klen, vlen);
    cast(struct logpool *, ctx)->connection = filter;
}

static void logpool_Filter_float(logpool_t *ctx, const char *key, uint64_t v, short klen, short vlen)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logpool *, ctx)->connection = filter->connection;
    filter->api->fn_float(ctx, key, v, klen, vlen);
    cast(struct logpool *, ctx)->connection = filter;
}

static void logpool_Filter_char(logpool_t *ctx, const char *key, uint64_t v, short klen, short vlen)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logpool *, ctx)->connection = filter->connection;
    filter->api->fn_char(ctx, key, v, klen, vlen);
    cast(struct logpool *, ctx)->connection = filter;
}

static void logpool_Filter_string(logpool_t *ctx, const char *key, uint64_t v, short klen, short vlen)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logpool *, ctx)->connection = filter->connection;
    filter->api->fn_string(ctx, key, v, klen, vlen);
    cast(struct logpool *, ctx)->connection = filter;
}

static void logpool_Filter_raw(logpool_t *ctx, const char *key, uint64_t v, short klen, short vlen)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logpool *, ctx)->connection = filter->connection;
    filter->api->fn_raw(ctx, key, v, klen, vlen);
    cast(struct logpool *, ctx)->connection = filter;
}

static void logpool_Filter_delim(logpool_t *ctx)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logpool *, ctx)->connection = filter->connection;
    filter->api->fn_delim(ctx);
    cast(struct logpool *, ctx)->connection = filter;
}

struct logapi FILTER_API = {
    logpool_Filter_null,
    logpool_Filter_bool,
    logpool_Filter_int,
    logpool_Filter_hex,
    logpool_Filter_float,
    logpool_Filter_char,
    logpool_Filter_string,
    logpool_Filter_raw,
    logpool_Filter_delim,
    logpool_Filter_flush,
    logpool_Filter_init,
    logpool_Filter_close,
    logpool_Filter_priority
};

#ifdef __cplusplus
}
#endif
