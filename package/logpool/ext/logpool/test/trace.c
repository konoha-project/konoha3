/*trace api test*/
#include "logpool.h"
#include <stdlib.h>
#include <stdio.h>
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
    if(!env && argc > 1) {
        env = (char *) argv[1];
    }
    env = (env) ? env : "100";
    fprintf(stderr, "%s:%d test_size=%s\n", __FILE__, __LINE__, env);
    return strtol(env, NULL, 10);
}
#include "test_main.c"
