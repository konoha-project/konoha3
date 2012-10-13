#include "logpool.h"
#include "llcache.h"

#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "io.h"
#include "protocol.h"
#include "message.idl.data.h"

int main(int argc, char **argv)
{
    if(argc < 2) {
        fprintf(stderr, "usage: %s 'compiled-llvm-bitcode'\n", argv[0]);
        return 1;
    }

    llcache_t *llmc = llcache_new("0.0.0.0", 11211);
    logpool_t *logpool = logpool_open_client(NULL, "0.0.0.0", 14801);
    llcache_set(llmc, "tid_usage_init", argv[1]);
    logpool_procedure(logpool, "tid_usage", strlen("tid_usage"));
    struct Log *logbuf = alloca(sizeof(struct Log) + 256);
    while (1) {
        if(logpool_client_get(logpool, logbuf, 256) == NULL) {
            break;
        }
        log_dump(stderr, "log=(", logbuf, ")\n", 1);
        usleep(1);
    }
    logpool_close(logpool);
    llcache_delete(llmc);
    return 0;
}
