#include <stdint.h>
#include <pthread.h>

#ifndef IO_H
#define IO_H

#ifdef __cplusplus
extern "C" {
#endif

enum io_status {
    IO_OK = 0,
    IO_FAILED = -1
};

enum IO_MODE {
    IO_MODE_READ   = (1 << 0),
    IO_MODE_WRITE  = (1 << 1),
    IO_MODE_CLIENT = (1 << 2),
    IO_MODE_THREAD = (1 << 3)
};

#define IO_BUFFER_SIZE 128
struct io;
typedef int (*io_cb)(struct io *io, const void *data, uint32_t nbyte);

struct pool_list;
struct range_stream;
struct bufferevent;

struct io_api;
struct io {
    const struct io_api *api;
    uint32_t flags;
    uint32_t host;
    struct bufferevent *bev;
    struct pool_list *pool;
    struct event_base *base;
    struct range_stream *cs; // use this at client
    pthread_t thread;
    pthread_mutex_t lock;
};

struct io_api {
    const char *name;
    int (*f_init)(struct io *io, char *host, int port, int ev_mode);
    io_cb f_read;
    io_cb f_write;
    int (*f_close)(struct io *io);
};

extern struct io *io_open(char *host, int port, int mode, struct io_api *api);
extern struct io *io_open_trace(char *host, int port);
extern int io_close(struct io *io);
extern int io_write(struct io *io, const void *data, uint32_t nbyte);
extern int io_read(struct io *io, void *data, uint32_t nbyte);
extern int io_sync(struct io *io);
extern int io_dispatch(struct io *io);

#define mfence() asm volatile ("" ::: "memory")

#ifdef __cplusplus
}
#endif
#endif /* end of include guard */
