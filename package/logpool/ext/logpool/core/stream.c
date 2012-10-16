#include "logpool_internal.h"
#include "io.h"
#include "stream.h"
#include <assert.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#ifdef __cplusplus
extern "C" {
#endif

struct range_stream *range_stream_new(struct io *io, struct bufferevent *bev)
{
    struct range_stream *cs = malloc(sizeof(*cs));
    cs->io = io;
    cs->bev = bev;
    cs->len = 0;
    cs->buffer = malloc(IO_BUFFER_SIZE);
    cs->cur = cs->buffer;
    debug_print(0, "*New* len=%d, buffer=%p", cs->len, cs->buffer);
    return cs;
}

void range_stream_delete(struct range_stream *cs)
{
    debug_print(0, "*Del* len=%d, buffer=%p", cs->len, cs->buffer);
    free(cs->buffer);
    bzero(cs, sizeof(*cs));
}

static int range_stream_size(struct range_stream *cs)
{
    return evbuffer_get_length(bufferevent_get_input(cs->bev));
}

int range_stream_empty(struct range_stream *cs)
{
    if(range_stream_size(cs) > 0) {
        debug_print(0, "empty len=%d, stream_size=%d, %d",
                cs->len, range_stream_size(cs), cs->len >= 0);
    }
    assert(cs->len >= 0);
    return cs->len == 0 && range_stream_size(cs) == 0;
}

//static int range_stream_reset(struct range_stream *cs, int request_size)
//{
//    int old_len = cs->len;
//    if(cs->len) {
//        memmove(cs->io->buffer, cs->cur, cs->len);
//    }
//    cs->len += bufferevent_read(cs->bev,
//            cs->io->buffer + cs->len, IO_BUFFER_SIZE - cs->len);
//    cs->cur  = cs->io->buffer;
//    debug_print(0, "reset %d=>%d", old_len, cs->len);
//    return cs->len >= request_size;
//}

static int range_stream_reset(struct range_stream *cs, int request_size)
{
    int old_len = cs->len;
    if(cs->len) {
        memmove(cs->buffer, cs->cur, cs->len);
    }
    cs->len += bufferevent_read(cs->bev,
            cs->buffer + cs->len, IO_BUFFER_SIZE - cs->len);
    cs->cur  = cs->buffer;
    debug_print(0, "reset %d=>%d", old_len, cs->len);
    return cs->len >= request_size;
}

static char *range_stream_next(struct range_stream *cs, size_t offset)
{
    char *d = cs->cur;
    debug_print(0, "next offset=%lu, old_len=%d", offset, cs->len);
    cs->len -= offset;
    assert(cs->len >= 0);
    cs->cur += offset;
    return d;
}

static int range_stream_check_size(struct range_stream *cs, int reqsize)
{
    if(cs->len < reqsize) {
        if(range_stream_size(cs) <= 0) {
            return 0;
        }
        if(!range_stream_reset(cs, reqsize)) {
            return 0;
        }
    }
    return 1;
}

struct Log *range_stream_get(struct range_stream *cs, int *log_size)
{
    if(!range_stream_check_size(cs, LOG_PROTOCOL_SIZE)) {
        return NULL;
    }
    debug_print(0, "len=%d", cs->len);
    assert(cs->len >= LOG_PROTOCOL_SIZE);
    uint16_t klen = 0, vlen = 0, i, logsize, *logp = NULL;
    struct Log *d = (struct Log *) cs->cur;
    int reqsize = 0;
    logsize = d->logsize;
    if(!range_stream_check_size(cs, LOG_PROTOCOL_SIZE + sizeof(uint16_t) * logsize * 2)) {
        return NULL;
    }
    /**
     * If we call check_size(), cs->cur may be changed.
     * So, 'd' and 'logp' must be reassigned
     */
    d = (struct Log *) cs->cur;
    logp = ((uint16_t *) d) + LOG_PROTOCOL_FIELDS;
    for (i = 0; i < logsize; ++i) {
        klen += logp[0];
        vlen += logp[1];
        logp += 2;
    }
    reqsize = LOG_PROTOCOL_SIZE + sizeof(uint16_t) * logsize * 2 + klen + vlen;
    debug_print(0, "%d, reqsize=%d, logsize=%d", cs->len, reqsize, logsize);

    if(!range_stream_check_size(cs, reqsize)) {
        return NULL;
    }
    *log_size = reqsize;
    return (struct Log *) range_stream_next(cs, reqsize);
}

#ifdef __cplusplus
}
#endif
