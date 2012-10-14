#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "libmemcached/memcached.h"
#include "logpool/logpool.h"
#include "logpool/io.h"
#include "logpool/protocol.h"
#include "logpool/message.idl.data.h"

static char *loadFile(char *file, size_t *plen)
{
    char *script = malloc(1024), *p = script;
    FILE *f;
    if((f = fopen(file, "r")) != NULL) {
        char buf[1024];
        size_t len = 0;
        while ((len = fread(buf, 1, sizeof(buf), f)) > 0) {
            assert(p - script < 1024);
            memcpy(p, buf, len);
            p += len;
        }
        fclose(f);
    }
    *plen = p - script;
    return script;
}

int main(int argc, char **argv)
{
    if(argc < 2) {
        fprintf(stderr, "usage: %s 'script.k'\n", argv[0]);
        return 1;
    }
    memcached_st *mc = memcached_create(NULL);
    memcached_server_list_st servers;
    memcached_return_t rc;
    servers = memcached_server_list_append(NULL, "127.0.0.1", 11211, &rc);
    if(rc != MEMCACHED_SUCCESS) {
        fprintf(stderr, "memcached_server_list_append failed\n");
    }
    rc = memcached_server_push(mc, servers);
    memcached_server_list_free(servers);

    logpool_t *logpool = logpool_open_client(NULL, "0.0.0.0", 14801);

    size_t script_len;
    char *script = loadFile(argv[1], &script_len);
    memcached_set(mc, "dump_init", strlen("dump_init"), script, script_len, 0, 0);
    logpool_procedure(logpool, "dump", strlen("dump"));
    struct Log *logbuf = alloca(sizeof(struct Log) + 256);
    while (1) {
        if(logpool_client_get(logpool, logbuf, 256) == NULL) {
            break;
        }
        log_dump(stderr, "log=(", logbuf, ")\n", 1);
        usleep(1);
    }
    logpool_close(logpool);
    free(script);
    return 0;
}
