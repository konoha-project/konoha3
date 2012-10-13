#include "logpool.h"

#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "io.h"
#include "protocol.h"
#include "message.idl.data.h"

int main(int argc, char **argv)
{
    logpool_t *logpool;
    logpool = logpool_open_client(NULL, "0.0.0.0", 14801);
    if(argc < 2) {
        fprintf(stderr, "usage: %s key value key value ...\n", argv[0]);
        goto L_error;
    }

    logpool_procedure(logpool, argv[1], strlen(argv[1]));
    struct Log *logbuf = alloca(sizeof(struct Log) + 256);
    while (1) {
        logpool_client_get(logpool, logbuf, 256);
        log_dump(stdout, "log=(", logbuf, ")\n", 1);
        usleep(1);
    }
    logpool_close(logpool);
    return 0;

    L_error:;
    logpool_close(logpool);
    return 1;
}
