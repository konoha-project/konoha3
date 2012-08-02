#include "lio.h"
#include "protocol.h"
#include "message.idl.data.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>

static void emit_log(struct lio *lio, char *key, char *val, char *key1, char *val1, char *key2, char *val2)
{
    char buf[128] = {};
    size_t len;
    len = emit_message(buf, LOGPOOL_EVENT_WRITE, 3,
            strlen(key), strlen(val), key, val,
            strlen(key1), strlen(val1), key1, val1,
            strlen(key2), strlen(val2), key2, val2,
            NULL);
    fprintf(stderr, "len=%zd, buf=%s\n", len, buf+LOG_PROTOCOL_SIZE*4);
    assert(lio_write(lio, buf, len) == LIO_OK);
}

static void emit_int(struct lio *lio, char *key, int n)
{
    char buf0[64] = {};
    sprintf(buf0, "%d", n);
    emit_log(lio, "TraceID", buf0, "tid", key, "File", __FILE__);
}

static void emit(struct lio *lio, int n)
{
    int i;
    for (i = 0; i < 10; ++i) {
        char buf[128] = {};
        sprintf(buf, "tid%d", i);
        emit_int(lio, buf, n * 10 + i);
    }
}

int main(int argc, char **argv)
{
    int i = 0;
    extern struct lio_api trace_api;
    struct lio *lio = lio_open("0.0.0.0", 14801,
            LIO_MODE_READ|LIO_MODE_WRITE, &trace_api);
    while (i < 1000) {
        fprintf(stderr, "start\n");
        emit(lio, i++);
        fprintf(stderr, "end\n");
        usleep(10);
    }
    lio_close(lio);
    return 0;
}
