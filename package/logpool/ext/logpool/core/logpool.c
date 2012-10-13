#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "logpool.h"
#include "logpool_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

static struct keyapi *KeyAPI = NULL;

static void logpool_new(logpool_t *ctx, struct logapi *api, logpool_param_t *param)
{
    struct logpool *lctx = cast(struct logpool *, ctx);
    lctx->logfmt_capacity = param->logfmt_capacity;
    assert(lctx->logfmt_capacity > 0);
    lctx->fmt = cast(logfmt_t *, malloc(sizeof(logfmt_t) * lctx->logfmt_capacity));
    lctx->formatter   = api;
    lctx->connection  = api->fn_init(ctx, param);
    lctx->logkey.k.seq = 0;
    lctx->logfmt_size  = 0;
    lctx->is_flushed   = 0;
}

static void append_fmtdata(logpool_t *ctx, const char *key, uint64_t v, logFn f, short klen, short vlen)
{
    struct logpool *lctx = cast(struct logpool *, ctx);
    assert(lctx->logfmt_size < lctx->logfmt_capacity);
    lctx->fmt[lctx->logfmt_size].fn    = f;
    lctx->fmt[lctx->logfmt_size].k.key = key;
    lctx->fmt[lctx->logfmt_size].v.u   = v;
    lctx->fmt[lctx->logfmt_size].klen  = klen;
    lctx->fmt[lctx->logfmt_size].vlen  = vlen;
    ++lctx->logfmt_size;
}

static void logpool_flush(logpool_t *ctx, void *args)
{
    cast(struct logpool *, ctx)->is_flushed = 0;
    ctx->formatter->fn_flush(ctx, args);
}

static int logpool_init_logkey(logpool_t *ctx, int priority, uint64_t v, short klen)
{
    int emitLog = 1;//TODO ctx->formatter->fn_priority(ctx, priority);
    if(emitLog) {
        struct logpool *lctx = cast(struct logpool *, ctx);
        lctx->logkey.v.u = v;
        lctx->logkey.klen = klen;
        lctx->logkey.vlen = 0;
        lctx->logfmt_size = 0;
    }
    return emitLog;
}

/* @see konoha2/logger.h */
static const int logfn_index[] = {
    /* LOG_END */ -1,
    /* LOG_s */    6,
    /* LOG_u */    2,
    /* LOG_i */    2,
    /* LOG_b */    1,
    /* LOG_f */    4,
    /* LOG_x */    3,
    /* LOG_n */    0,
    /* LOG_r */    7,
};

void logpool_record_list(logpool_t *ctx, void *args, int priority, char *trace_id, int logsize, struct logdata *logs)
{
    int i;
    logFn f;

    assert(logsize < ctx->logfmt_capacity);
    logpool_init_logkey(ctx, priority, (uintptr_t) trace_id, strlen(trace_id));
    for (i = 0; i < logsize; ++i) {
        f = ((logFn*)ctx->formatter)[logfn_index[logs->type]];
        append_fmtdata(ctx, logs->key, (uint64_t)logs->val, f, logs->klen, logs->vlen);
        ++logs;
    }
    logpool_flush(ctx, args);
}

void logpool_record_ap(logpool_t *ctx, void *args, int priority, char *trace_id, va_list ap)
{
    int i;
    struct logdata logs[ctx->logfmt_capacity];
    for (i = 0; i < ctx->logfmt_capacity; ++i) {
        logs[i].type = va_arg(ap, int);
        if(logs[i].type == 0)
            break;
        logs[i].key  = va_arg(ap, char *);
        logs[i].val  = va_arg(ap, char *);
        logs[i].klen = strlen(logs[i].key);
        logs[i].vlen = (logs[i].type == 1/*LOG_s*/)? strlen(logs[i].val):0;
    }
    logpool_record_list(ctx, args, priority, trace_id, i, logs);
}

void logpool_record(logpool_t *ctx, void *args, int priority, char *trace_id, ...)
{
    va_list ap;
    va_start(ap, trace_id);
    logpool_record_ap(ctx, args, priority, trace_id, ap);
    va_end(ap);
}

void logpool_format_flush(logpool_t *ctx)
{
    struct logfmt *fmt = cast(struct logpool *, ctx)->fmt;
    size_t size = ctx->logfmt_size;
    if(ctx->is_flushed)
        return;
    ctx->fn_key(ctx, ctx->logkey.v.u, ctx->logkey.k.seq, ctx->logkey.klen);
    if(size) {
        size_t i;
        void (*fn_delim)(logpool_t *) = ctx->formatter->fn_delim;
        /* unroled */
        fn_delim(ctx);
        fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->klen, fmt->vlen);
        ++fmt;
        for (i = 1; i < size; ++i, ++fmt) {
            fn_delim(ctx);
            fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->klen, fmt->vlen);
        }
        cast(struct logpool *, ctx)->logfmt_size = 0;
    }
    ++(cast(struct logpool *, ctx)->logkey.k.seq);
    cast(struct logpool *, ctx)->is_flushed = 1;
}

logpool_t *logpool_open(logpool_t *parent, struct logapi *api, logpool_param_t *p)
{
    struct logpool *l = cast(struct logpool *, malloc(sizeof(*l)));
    l->parent = parent;
    l->fn_key = KeyAPI->str;
    logpool_new(cast(logpool_t *, l), api, p);
    return l;
}

logpool_t *logpool_open_trace(logpool_t *parent, char *host, int port)
{
    struct logpool_param_stream param = {8, 1024};
    param.host = host;
    param.port = port;
    extern struct logapi STREAM_API;
    return logpool_open(parent, &STREAM_API, (struct logpool_param*) &param);
}

void logpool_close(logpool_t *p)
{
    struct logpool *l = cast(struct logpool *, p);
    cast(logpool_t *, l)->formatter->fn_close(cast(logpool_t *, l));
    free(l->fmt);
    l->fmt = NULL;
    free(l);
}

#ifdef LOGPOOL_USE_LLVM
extern struct keyapi *logpool_llvm_api_init(void);
#endif
extern struct keyapi *logpool_string_api_init(void);
extern struct keyapi *logpool_trace_api_init(void);
static int exec_mode = 0;

void logpool_global_init(int mode)
{
    //assert(exec_mode == 0);
    exec_mode = mode;
    if(mode == LOGPOOL_JIT) {
#ifdef LOGPOOL_USE_LLVM
        KeyAPI = logpool_llvm_api_init();
#else
        assert(0 && "please enable USE_LLVM flag");
#endif
    } else if(mode == LOGPOOL_TRACE) {
        KeyAPI = logpool_trace_api_init();
    } else {
        KeyAPI = logpool_string_api_init();
    }
    KeyAPI = logpool_string_api_init();
}

extern void logpool_trace_api_deinit(void);
void logpool_global_exit(void)
{
    //assert(exec_mode != 0);
    if(exec_mode == LOGPOOL_TRACE) {
        logpool_trace_api_deinit();
    }
    exec_mode = 0;
}

#ifdef __cplusplus
}
#endif
