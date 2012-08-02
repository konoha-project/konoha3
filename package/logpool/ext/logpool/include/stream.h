#include "protocol.h"

#ifndef IO_STREAM_H
#define IO_STREAM_H

struct io;
struct bufferevent;
struct chunk_stream {
    struct io *io;
    struct bufferevent *bev;
    char *cur;
    char *buffer;
    int   len;
};

int chunk_stream_empty(struct chunk_stream *cs);
struct chunk_stream *chunk_stream_init(struct chunk_stream *cs, struct io *io, struct bufferevent *bev);
void chunk_stream_deinit(struct chunk_stream *cs);
struct Log *chunk_stream_get(struct chunk_stream *cs, int *log_size);
struct chunk_stream *chunk_stream_new(struct io *io, struct bufferevent *bev);
void chunk_stream_delete(struct chunk_stream *cs);

#endif /* end of include guard */
