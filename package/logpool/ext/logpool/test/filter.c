/*filter api test*/
#include "logpool.h"
extern logapi_t STRING_API;
static struct logpool_param_string STRING_API_PARAM = {8, 1024};
static struct logpool_param_filter FILTERED_STRING_API_PARAM = {
    8, LOG_NOTICE, &STRING_API, (struct logpool_param *) &STRING_API_PARAM
};
#define LOGAPI_PARAM cast(logpool_param_t *, &FILTERED_STRING_API_PARAM)
#define LOGAPI FILTER_API

#include <stdbool.h>

#ifndef LOGAPI
#error define LOGAPI && LOGAPI_PARAM
#endif

#define LOG_END 0
#define LOG_s   1
#define LOG_u   2
#define LOG_i   2
#define LOG_f   4

#define LogUint(K,V)    LOG_u, (K), ((uintptr_t)V)
#define KEYVALUE_i(K,V)    LOG_i, (K), ((uintptr_t)V)
#define KEYVALUE_f(K,V)    LOG_f, (K), (f2u(V))
#define LogText(K,V)    LOG_s, (K), (V)

extern logapi_t LOGAPI;

static void logpool_test_write0(logpool_t *logpool)
{
    double f = 3.14;
    long   i = 128;
    const char *s = "hello world";
    logpool_record(logpool, NULL, LOG_INFO, "event",
            KEYVALUE_f("float", f),
            KEYVALUE_i("int",   i),
            LogText("string", s),
            LOG_END
            );
}

static void logpool_test_write1(logpool_t *logpool)
{
    double f = 3.14;
    long   i = 128;
    const char *s = "hello world";
    logpool_record(logpool, NULL, LOG_NOTICE, "event",
            KEYVALUE_f("float", f),
            KEYVALUE_i("int",   i),
            LogText("string", s),
            LOG_END
            );
}


int main(void)
{
    logpool_global_init(LOGPOOL_DEFAULT);
    {
        logpool_t *logpool = logpool_open(NULL, &LOGAPI, LOGAPI_PARAM);
        int i;
        for (i = 0; i < 5; ++i) {
            logpool_test_write0(logpool);
            logpool_test_write1(logpool);
        }
        logpool_close(logpool);
    }
    return 0;
}
