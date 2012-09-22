#include "logpool.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

static struct logpool_param_stream TRACE_API_PARAM = {
    8,
    1024,
    "0.0.0.0", 14801
};
#define LOGAPI_PARAM cast(logpool_param_t *, &TRACE_API_PARAM)
#define LOGAPI_INIT_FLAG (LOGPOOL_TRACE)
#define LOGPOOL_TEST_COUNT(argc, argv) get_count(argc, argv)
#define LOGAPI STREAM_API
static int get_count(int argc, const char **argv)
{
    char *env = PLATAPI getenv_i("LOGPOOL_TESTCASE_SIZE");
    if (!env && argc > 1) {
        env = (char *) argv[1];
    }
    env = (env) ? env : "100";
    fprintf(stderr, "%s:%d test_size=%s\n", __FILE__, __LINE__, env);
    return strtol(env, NULL, 10);
}

extern logapi_t LOGAPI;
#define LOG_END 0
#define LOG_s   1
#define LOG_u   2

#define LogUint(K,V)    LOG_u, (K), ((uintptr_t)V)
#define LogText(K,V)    LOG_s, (K), (V)

int n = 0;
static void logpool_test_write(logpool_t *logpool)
{
    long   i = n;
    const char *s = "Good Bye";
    void *args;
    logpool_record(logpool, &args, LOG_NOTICE, "check",
            LogText("string", s),
            LogUint("tid",   i%10),
            LOG_END
            );
    n++;
}

int main(int argc, char const* argv[])
{
    logpool_global_init(LOGAPI_INIT_FLAG);
    logpool_t *logpool = logpool_open(NULL, &LOGAPI, LOGAPI_PARAM);
    int i, size = LOGPOOL_TEST_COUNT(argc, argv);
    for (i = 0; i < size; ++i) {
        logpool_test_write(logpool);
        if (i % 2) {
            usleep(1);
        }
    }
    logpool_close(logpool);
    logpool_global_exit();
    return 0;
}
