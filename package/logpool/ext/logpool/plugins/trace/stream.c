#include <stdlib.h>
#include <stdio.h>
#include "logpool.h"
#include "lpstring.h"
#include "logpool_internal.h"
#include "io.h"
#include "protocol.h"

enum {
    LOGPOOL_FAILURE = -1,
    LOGPOOL_SUCCESS = 0
};

#define DEFAULT_SERVER "0.0.0.0"
#define DEFAULT_PORT   14801

#ifdef __cplusplus
extern "C" {
#endif

struct io_plugin {
    char *buf;
    struct io *io;
    char base[1];
};
static int get_server_info(struct logpool_param_stream *pa, char *hostbuf)
{
    if(pa) {
        memcpy(hostbuf, pa->host, strlen(pa->host));
        return pa->port;
    }
    char *serverinfo = PLATAPI getenv_i("LOGPOOL_SERVER");
    int  port = DEFAULT_PORT;
    if(serverinfo) {
        char *pos;
        if((pos = strchr(serverinfo, ':')) != NULL) {
            port = strtol(pos+1, NULL, 10);
            memcpy(hostbuf, serverinfo, pos - serverinfo);
        } else {
            memcpy(hostbuf, DEFAULT_SERVER, strlen(DEFAULT_SERVER));
        }
    } else {
        memcpy(hostbuf, DEFAULT_SERVER, strlen(DEFAULT_SERVER));
    }
    return port;
}

static uint16_t *emit_header(char *buf, int protocol, int logsize)
{
    struct Message *msg = ((struct Message*)buf);
    msg->crc32 = 0;
    msg->protocol = LOGPOOL_EVENT_WRITE;
    msg->logsize  = logsize;
    buf += LOG_PROTOCOL_SIZE;
    return (uint16_t *) buf;
}

static void *logpool_io_init(logpool_t *logpool, logpool_param_t *p)
{
    struct io_plugin *lp;
    lp = cast(struct io_plugin *, logpool_string_init(logpool, p));
    char host[128] = {};
    int port = get_server_info((struct logpool_param_stream*) p, host);
    lp->io = io_open_trace(host, port);
    return cast(void *, lp);
}

static void logpool_io_close(logpool_t *logpool)
{
    struct io_plugin *lp = cast(struct io_plugin *, logpool->connection);
    io_close(lp->io);
    logpool_string_close(logpool);
}

static void logpool_io_flush(logpool_t *logpool, void **args __UNUSED__)
{
    struct io_plugin *lp = cast(struct io_plugin *, logpool->connection);
    char *buf_orig = lp->buf;
    size_t size, bufsize;
    struct logfmt *fmt, *fmte;
    int ret;
    uint16_t *loginfo;

    size = logpool->logfmt_size + 1;
    fmt = cast(struct logpool *, logpool)->fmt;
    fmte = fmt + size - 1;
    loginfo = emit_header(lp->buf, LOGPOOL_EVENT_WRITE, size);
    lp->buf = (char *)(loginfo + size * 2);
    char *p = lp->buf;
    logpool->fn_key(logpool, logpool->logkey.v.u,
            logpool->logkey.k.seq, logpool->logkey.klen);
    *loginfo++ = strlen("TraceID");
    *loginfo++ = lp->buf - p - strlen("TraceID");
    assert(size > 0);

    while (fmt < fmte) {
        char *_buf = lp->buf;
        fmt->fn(logpool, fmt->k.key, fmt->v.u, fmt->klen, fmt->vlen);
        *loginfo++ = fmt->klen;
        *loginfo++ = (lp->buf - _buf) - fmt->klen;
        ++fmt;
    }
    cast(struct logpool *, logpool)->logfmt_size = 0;
    bufsize = (char*) lp->buf - buf_orig;
    ret = io_write(lp->io, buf_orig, bufsize);
    if(ret != LOGPOOL_SUCCESS) {
        /* TODO Error */
        fprintf(stderr, "Error!!\n");
        abort();
    }
    logpool_string_reset(logpool);
    ++(cast(struct logpool *, logpool)->logkey.k.seq);
}

static void logpool_io_delim(logpool_t *ctx) {}
static void logpool_io_string(logpool_t *ctx, const char *key, uint64_t v, short klen, short vlen)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    char *s = cast(char *, v);
    buf_put_string(buf, key, klen);
    buf_put_string(buf, s, vlen);
}

struct logapi STREAM_API = {
    logpool_string_null,
    logpool_string_bool,
    logpool_string_int,
    logpool_string_hex,
    logpool_string_float,
    logpool_string_char,
    logpool_io_string,
    logpool_string_raw,
    logpool_io_delim,
    logpool_io_flush,
    logpool_io_init,
    logpool_io_close,
    logpool_default_priority
};

extern struct keyapi *logpool_string_api_init(void);

struct keyapi *logpool_trace_api_init(void)
{
#if LOGPOOL_DEBUG
    char host[128] = {0};
    int port = get_server_info(NULL, host);
    fprintf(stderr,"default config [%s:%u]\n", host, port);
#endif
    return logpool_string_api_init();
}

void logpool_trace_api_deinit(void)
{
}

int logpoold_start(char *host, int port)
{
    extern struct io_api server_api;
    struct io *io = io_open(host, port,
            IO_MODE_READ|IO_MODE_WRITE, &server_api);
    return io_dispatch(io);
}


static void *logpool_io_client_init(logpool_t *logpool, logpool_param_t *p)
{
    extern struct io_api client_api;
    struct io_plugin *lp;
    struct logpool_param_stream *args = cast(struct logpool_param_stream *, p);
    char *host = (char*) args->host;
    long port = args->port;

    lp = cast(struct io_plugin *, logpool_string_init(logpool, p));
    lp->io = io_open(host, port, IO_MODE_READ|IO_MODE_WRITE, &client_api);
    return cast(void *, lp);
}
static void logpool_io_client_close(logpool_t *ctx)
{
    struct io_plugin *lp = cast(struct io_plugin *, ctx->connection);
    logpool_string_close(ctx);
    io_close(lp->io);
}

static struct logapi CLIENT_API = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    logpool_io_client_init,
    logpool_io_client_close,
    NULL
};

logpool_t *logpool_open_client(logpool_t *parent, char *host, int port)
{
    struct logpool_param_stream param = {8, 1024};
    logpool_global_init(LOGPOOL_DEFAULT);
    param.host = host;
    param.port = port;
    return logpool_open(parent, &CLIENT_API, (struct logpool_param*) &param);
}

void logpool_procedure(logpool_t *logpool, char *q, int qlen)
{
    struct io_plugin *lp = cast(struct io_plugin *, logpool->connection);
    char buf[128] = {};
    size_t len = emit_message(buf, LOGPOOL_EVENT_READ, 1,
            0, qlen, NULL, q);
    assert(io_write(lp->io, buf, len) == IO_OK);
}

void *logpool_client_get(logpool_t *logpool, void *logbuf, size_t bufsize)
{
    struct io_plugin *lp = cast(struct io_plugin *, logpool->connection);
    if(io_read(lp->io, (char*) logbuf, bufsize) == IO_FAILED)
        return NULL;
    return (void*) logbuf;
}

#ifdef __cplusplus
}
#endif
