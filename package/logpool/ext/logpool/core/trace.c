#include "logpool_internal.h"
#include "io.h"
#include "stream.h"
#include <assert.h>
#include <errno.h>
#include <event2/event.h>
#include <event2/dns.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int util_send_quit_msg(struct bufferevent *bev)
{
    char buf[16];
    int size = emit_message(buf, LOGPOOL_EVENT_QUIT, 1, 0, 0, NULL, NULL);
    if(bufferevent_write(bev, buf, size) != 0) {
        debug_print(0, "[util:quit] write error");
        return IO_FAILED;
    }
    return IO_OK;
}

static void tracer_cb_event(struct bufferevent *bev, short events, void *ctx)
{
    struct event_base *base = ctx;
    if(events & BEV_EVENT_CONNECTED) {
        debug_print(0, "Connect okay.");
    } else if(events & BEV_EVENT_TIMEOUT) {
        debug_print(1, "server timeout");
        bufferevent_free(bev);
        event_base_loopexit(base, NULL);
    } else if(events & (BEV_EVENT_ERROR|BEV_EVENT_EOF)) {
        if(events & BEV_EVENT_ERROR) {
            int err = bufferevent_socket_get_dns_error(bev);
            if(err)
                fprintf(stderr, "DNS error: %s\n", evutil_gai_strerror(err));
        }
        bufferevent_free(bev);
        event_base_loopexit(base, NULL);
    }
}

static void tracer_cb_read(struct bufferevent *bev, void *ctx)
{
    //debug_print(0, "read_cb");
}

static void tracer_cb_write(struct bufferevent *bev, void *ctx)
{
    //debug_print(0, "write_cb");
}

static void trace_thread_start(struct io *io);
static int io_tracer_init(struct io *io, char *host, int port, int ev_mode)
{
    struct event_base *base = event_base_new();
    struct evdns_base *dns_base;
    struct bufferevent *bev;
    bev = bufferevent_socket_new(base, -1,
            BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);
    if(!bev) {
        fprintf(stderr, "Error constructing bufferevent\n");
        return IO_FAILED;
    }
    bufferevent_setcb(bev, tracer_cb_read, tracer_cb_write, tracer_cb_event, base);

    bufferevent_enable(bev, ev_mode);
    dns_base = evdns_base_new(base, 1);
    int ret = bufferevent_socket_connect_hostname(bev, dns_base, AF_INET, host, port);
    if(ret == -1) {
        bufferevent_free(bev);
        io->bev = NULL;
        return IO_FAILED;
    }

    //struct timeval tv;
    //tv.tv_sec = 20;
    //tv.tv_usec = 0;
    //bufferevent_set_timeouts(bev, &tv, NULL);

    io->bev = bev;
    trace_thread_start(io);
    return IO_OK;
}

static void *trace_thread_main(void *args)
{
    struct io *io = (struct io *) args;
    fprintf(stderr, "%s start\n", __func__);
    io->flags |= IO_MODE_THREAD;
    mfence();
    assert(io && io->bev);
    pthread_mutex_unlock(&io->lock);
    event_base_dispatch(bufferevent_get_base(io->bev));
    io->flags ^= IO_MODE_THREAD;
    fprintf(stderr, "%s exit\n", __func__);
    mfence();
    return 0;
}

static void trace_thread_start(struct io *io)
{
    pthread_mutex_lock(&io->lock);
    pthread_create(&io->thread, NULL, trace_thread_main, io);
    while (1) {
        if(pthread_mutex_trylock(&io->lock) == 0) {
            pthread_mutex_unlock(&io->lock);
            break;
        }
    }
    fprintf(stderr, "thread started\n");
}

static int io_tracer_write(struct io *io, const void *data, uint32_t nbyte)
{
    //fprintf(stderr, "io=%p, bev=%p, data=%p, %d\n", io, io->bev, data, nbyte);
    if(bufferevent_write(io->bev, data, nbyte) != 0) {
        fprintf(stderr, "write error, v=('%p', %u)\n", data, nbyte);
        return IO_FAILED;
    }
    return IO_OK;
}

static int io_tracer_read(struct io *io, const void *data, uint32_t nbyte)
{
    int len = bufferevent_read(io->bev, (char*)data, nbyte);
    debug_print(1, "read: len=[%d] data=[%s]\n", len, (char*)data);
    return IO_OK;
}

static int io_tracer_close(struct io *io)
{
    if(io->flags & IO_MODE_THREAD) {
        util_send_quit_msg(io->bev);
        if(pthread_join(io->thread, NULL) != 0) {
            fprintf(stderr, "pthread join failure. %s\n", strerror(errno));
            abort();
            return IO_FAILED;
        }
    }
    return IO_OK;
}

struct io_api trace_api = {
    "tracer",
    io_tracer_init,
    io_tracer_read,
    io_tracer_write,
    io_tracer_close
};

#ifdef __cplusplus
}
#endif
