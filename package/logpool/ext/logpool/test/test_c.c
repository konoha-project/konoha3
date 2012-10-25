#include "io.h"
#include "protocol.h"
#include "message.idl.data.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>

static void emit_procedure(struct io *io, char *q)
{
    char buf[128] = {};
    size_t len = emit_message(buf, LOGPOOL_EVENT_READ, 1,
            0, strlen(q), NULL, q);
    assert(io_write(io, buf, len) == IO_OK);
}

static void read_log(struct io *io, struct Log *tmp)
{
    io_read(io, (char *) tmp, 128);
}

int main(int argc, char **argv)
{
    extern struct io_api client_api;
    struct io *io = io_open("0.0.0.0", 14801,
            IO_MODE_READ|IO_MODE_WRITE, &client_api);
    emit_procedure(io, "match tid tid0");
    struct Log *log = alloca(sizeof(struct Log) + 256);
    while(1) {
        char kbuf[16] = {};
        char vbuf[128] = {};
        read_log(io, log);
        char *data = log_get_data(log);
        memcpy(kbuf, data,log->klen);
        memcpy(vbuf, data+log->klen, log->vlen);
        fprintf(stderr, "log=(%d, %d, '%s': '%s')\n",
                log->klen, log->vlen, kbuf, vbuf);
        usleep(100);
    }
    io_close(io);
    return 0;
}
