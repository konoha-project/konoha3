#include "protocol.h"

#ifndef IO_STREAM_H
#define IO_STREAM_H

struct io;
struct bufferevent;
struct range_stream {
    struct io *io;
    struct bufferevent *bev;
    char *cur;
    char *buffer;
    int   len;
};

int range_stream_empty(struct range_stream *cs);
struct range_stream *range_stream_Init(struct range_stream *cs, struct io *io, struct bufferevent *bev);
void range_stream_deinit(struct range_stream *cs);
struct Log *range_stream_get(struct range_stream *cs, int *log_size);
struct range_stream *range_stream_new(struct io *io, struct bufferevent *bev);
void range_stream_delete(struct range_stream *cs);

#endif /* end of include guard */
