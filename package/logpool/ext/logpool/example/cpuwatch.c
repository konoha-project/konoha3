#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "libmemcached/memcached.h"
#include "logpool/logpool.h"
#include "logpool/io.h"
#include "logpool/protocol.h"
#include "logpool/message.idl.data.h"

#if 1
static const char query_script[] = "K.import(\"logpool\");\n"
"K.import(\"konoha.assignment\");\n"
"\n"
"PoolPlugin initPlugin(Object ev) {\n"
"    PoolPlugin p0 = Response.create(ev);\n"
"    return p0;\n"
"}\n";
#endif

static char *loadFile(char *file, size_t *plen)
{
    *plen = strlen(query_script);
    return (char *) query_script;
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
    memcached_set(mc, "cpu_watch", strlen("cpu_watch"), script, script_len, 0, 0);
    logpool_procedure(logpool, "cpu_watch", strlen("cpu_watch"));
    struct Log *logbuf = alloca(sizeof(struct Log) + 256);
    while (1) {
        if(logpool_client_get(logpool, logbuf, 256) == NULL) {
            break;
        }
        log_dump(stderr, "log=(", logbuf, ")\n", 1);
        usleep(1);
    }
    logpool_close(logpool);
    return 0;
}
