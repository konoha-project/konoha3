#include "logpool_internal.h"
#include "io.h"
#include "stream.h"
#include "plugins/pool/pool_plugin.h"
#include <assert.h>
#include <arpa/inet.h>
#include <event2/listener.h>

#ifdef __cplusplus
extern "C" {
#endif

static void server_event_callback(struct bufferevent *bev, short events, void *ctx)
{
    struct range_stream *cs = (struct range_stream *) ctx;
    struct io *io = cs->io;
    if(events & BEV_EVENT_EOF) {
        debug_print(1, "client disconnect");
        range_stream_delete(cs);
        pool_delete_connection(io->pool, bev);
        bufferevent_free(bev);
    } else if(events & BEV_EVENT_TIMEOUT) {
        debug_print(1, "client timeout e=%p, events=%x", bev, events);
        pool_delete_connection(io->pool, bev);
        bufferevent_free(bev);
    } else {
        /* Other case, maybe error occur */
        pool_delete_connection(io->pool, bev);
        bufferevent_free(bev);
    }
}

static void server_read_callback(struct bufferevent *bev, void *ctx)
{
    struct range_stream *cs = (struct range_stream *) ctx;
    struct io *io = cs->io;
    debug_print(0, "read_cb bev=%p", bev);
    while (!range_stream_empty(cs)) {
        int log_size;
        struct Log *log = range_stream_get(cs, &log_size);
        if(log == NULL) {
            break;
        }
        char *data = log_get_data((struct Log *) log);
        debug_print(0, "%d %d %s", log->protocol, log->logsize, data);
        switch (log_data_protocol(log)) {
        case LOGPOOL_EVENT_READ:
            debug_print(1, "R %d %d, '%s'", log->klen, log->vlen, data);
            pool_add((struct Procedure*) log, bev, io->pool);
            break;
        case LOGPOOL_EVENT_WRITE:
#if LOGPOOL_DEBUG >= 1
            log_dump(stderr, "W ", (struct Log *) log, "\n", 0);
#endif
            pool_exec((struct Log *) log, log_size, io->pool);
            break;
        case LOGPOOL_EVENT_QUIT:
            debug_print(1, "Q %d, %d\n", log->klen, log->vlen);
            pool_delete_connection(io->pool, bev);
            bufferevent_free(bev);
            goto L_exit;
        case LOGPOOL_EVENT_NULL:
        default:
            /*TODO*/abort();
            break;
        }
    }
    L_exit:;
    return;
}

static void server_write_callback(struct bufferevent *bev, void *ctx)
{
    //debug_print(0, "write_cb bev=%p", bev);
}

static void server_accept_callback(struct evconnlistener *lev, evutil_socket_t fd,
        struct sockaddr *sa, int socklen, void *ctx)
{
    (void)socklen;
    struct io *io = (struct io *) ctx;
    struct event_base *base = evconnlistener_get_base(lev);
    struct bufferevent *bev;

    debug_print(1, "client connect from [%s:%u] over fd [%d]",
            inet_ntoa(((struct sockaddr_in *) sa)->sin_addr),
            (unsigned short) ntohs(((struct sockaddr_in *) sa)->sin_port), fd);

    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if(bev == NULL) {
        debug_print(0, "bufferevent_socket_new() failed");
        evutil_closesocket(fd);
        return;
    }

    struct range_stream *cs = range_stream_new(io, bev);
    bufferevent_setcb(bev, server_read_callback,
            server_write_callback, server_event_callback, cs);

    bufferevent_enable(bev, EV_READ|EV_WRITE);

    //FIXME
    //struct timeval tv;
    //tv.tv_sec = 10;
    //tv.tv_usec = 0;
    //bufferevent_set_timeouts(bev, &tv, &tv);
}

static int io_server_init(struct io *io, char *host, int port, int ev_mode)
{
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(host);
    sin.sin_port = htons(port);

    struct evconnlistener *lev;
    struct event_base *base = event_base_new();
    lev = evconnlistener_new_bind(base, server_accept_callback, io,
            LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
            (struct sockaddr *) &sin, sizeof(sin));
    debug_print(9, "host=%s, port=%d", host, port);
    if(lev == NULL) {
        debug_print(9, "bind() failed");
        return IO_FAILED;
    }
    io->base = base;
    io->pool = pool_new();
    return IO_OK;
}

static int io_server_write(struct io *io, const void *data, uint32_t nbyte)
{
    if(bufferevent_write(io->bev, data, nbyte) != 0) {
        fprintf(stderr, "write error, v=('%p', %u)\n", data, nbyte);
        return IO_FAILED;
    }
    return IO_OK;
}

static int io_server_read(struct io *io, const void *data, uint32_t nbyte)
{
    (void)io;(void)data;(void)nbyte;
    debug_print(0, "read");
    return IO_FAILED;
}

static int io_server_close(struct io *io)
{
    pool_delete(io->pool);
    return IO_OK;
}

struct io_api server_api = {
    "server",
    io_server_init,
    io_server_read,
    io_server_write,
    io_server_close
};

#ifdef __cplusplus
}
#endif
