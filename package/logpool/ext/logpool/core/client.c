#include "logpool_internal.h"
#include "io.h"
#include "stream.h"
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <event2/event.h>
#include <event2/dns.h>
#include <event2/buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

static void client_cb_event(struct bufferevent *bev, short events, void *ctx)
{
    struct io *io = (struct io *) ctx;
    struct event_base *base = io->base;
    if(events & BEV_EVENT_CONNECTED) {
        debug_print(0, "Connect okay.");
    } else if(events & BEV_EVENT_TIMEOUT) {
        debug_print(0, "server timeout");
        bufferevent_free(bev);
        event_base_loopexit(base, NULL);
        io->base = NULL;
        io->bev  = NULL;
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

static void client_cb_read(struct bufferevent *bev, void *ctx)
{
    debug_print(0, "read_cb");
}

static void client_cb_write(struct bufferevent *bev, void *ctx)
{
    debug_print(0, "write_cb");
}

static void client_thread_start(struct io *io);
static int io_client_init(struct io *io, char *host, int port, int ev_mode)
{
    struct event_base *base = event_base_new();
    struct evdns_base *dns_base;
    struct bufferevent *bev;
    bev = bufferevent_socket_new(base, -1,
            BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);
    io->bev  = bev;
    io->base = base;
    bufferevent_setcb(bev, client_cb_read,
            client_cb_write, client_cb_event, io);

    bufferevent_enable(bev, ev_mode);
    dns_base = evdns_base_new(base, 1);
    int ret = bufferevent_socket_connect_hostname(bev,
            dns_base, AF_INET, host, port);
    if(ret == -1) {
        bufferevent_free(bev);
        io->bev = NULL;
        return IO_FAILED;
    }

    //struct timeval tv;
    //tv.tv_sec = 10;
    //tv.tv_usec = 0;
    //bufferevent_set_timeouts(bev, NULL, &tv);

    client_thread_start(io);
    return IO_OK;
}

static void *client_thread_main(void *args)
{
    struct io *io = (struct io *) args;
    assert(io);
    //fprintf(stderr, "%s start\n", __func__);
    io->flags |= IO_MODE_THREAD;
    mfence();
    event_base_dispatch(bufferevent_get_base(io->bev));
    io->flags ^= IO_MODE_THREAD;
    //fprintf(stderr, "%s exit\n", __func__);
    mfence();
    return 0;
}

static void client_thread_start(struct io *io)
{
    pthread_create(&io->thread, NULL, client_thread_main, io);
    while (io->flags & IO_MODE_THREAD) {
        mfence();
    }
}

static int io_client_write(struct io *io, const void *data, uint32_t nbyte)
{
    if(io->bev == NULL || bufferevent_write(io->bev, data, nbyte) != 0) {
        fprintf(stderr, "write error, v=('%p', %u)\n", data, nbyte);
        return IO_FAILED;
    }
    return IO_OK;
}

static int io_client_read(struct io *io, const void *data, uint32_t nbyte)
{
    struct range_stream *cs;
    if(io->cs == NULL) {
        cs = range_stream_new(io, io->bev);
        io->cs = cs;
    } else {
        cs = io->cs;
    }
    if(io->bev) {
        int log_size;
        struct Log *log;
        //L_redo:;
        while (range_stream_empty(cs)) {
            if((io->flags & IO_MODE_THREAD) == 0) {
                break;
            }
            usleep(1);
        }
        log = range_stream_get(cs, &log_size);
        if(log == NULL) {
            fprintf(stderr, "log is null\n");
            goto L_failed;
        }
        if(log_data_process(log) == LOGPOOL_EVENT_QUIT) {
            bufferevent_free(io->bev);
            debug_print(1, "stream connection close");
            return IO_FAILED;
        }
        memcpy((void*)data, log, nbyte);
        return IO_OK;
    }
    debug_print(1, "stream was not connected");
    L_failed:;
    return IO_FAILED;
}

static int io_client_close(struct io *io)
{
    if(io->flags & IO_MODE_THREAD) {
        event_base_loopexit(io->base, NULL);
        if(pthread_join(io->thread, NULL) != 0) {
            fprintf(stderr, "pthread join failure. %s\n", strerror(errno));
            abort();
            return IO_FAILED;
        }
    }
    return IO_OK;
}

struct io_api client_api = {
    "client",
    io_client_init,
    io_client_read,
    io_client_write,
    io_client_close
};

char *Log_get(struct Log *log, char *key, int klen, int *vlen)
{
    uint16_t i;
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

#ifdef __cplusplus
}
#endif
