#include "io.h"
#include "stream.h"
#include <assert.h>
#include <event2/event.h>
#include <event2/thread.h>

#ifdef __cplusplus
extern "C" {
#endif

static volatile int once = 1;
struct io *io_open(char *host, int port, int mode, struct io_api *api)
{
    struct io *io;
    short ev_mode = 0;
    if(once) {
        once = 0;
        if(evthread_use_pthreads() != 0) {
            fprintf(stderr, "thread init error\n");
            return NULL;
        }
    }

    if(mode & IO_MODE_READ)
        ev_mode |= EV_READ;
    if(mode & IO_MODE_WRITE)
        ev_mode |= EV_WRITE;

    io = calloc(1, sizeof(*io));
    io->flags = mode & 0x3;
    io->api  = api;
    pthread_mutex_init(&io->lock, NULL);
    api->f_init(io, host, port, ev_mode);
    return io;
}

int io_close(struct io *io)
{
    io->api->f_close(io);
    pthread_mutex_destroy(&io->lock);
    bzero(io, sizeof(*io));
    free(io);
    return IO_OK;
}

int io_write(struct io *io, const void *data, uint32_t nbyte)
{
    return io->api->f_write(io, data, nbyte);
}

int io_read(struct io *io, void *data, uint32_t nbyte)
{
    return io->api->f_read(io, data, nbyte);
}

int io_sync(struct io *io)
{
    (void)io;
    return IO_OK;
}

int io_dispatch(struct io *io)
{
    assert(io->base);
    fprintf(stderr, "dispach start\n");
    event_base_dispatch(io->base);
    io_close(io);
    return 0;
}

extern struct io_api trace_api;
struct io *io_open_trace(char *host, int port)
{
    struct io *io = io_open(host, port,
            IO_MODE_READ|IO_MODE_WRITE, &trace_api);
    return io;
}

#ifdef __cplusplus
}
#endif
