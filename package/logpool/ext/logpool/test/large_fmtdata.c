/* large size format test */
#include "logpool.h"
static struct logpool_param_string STRING_API_PARAM = {
#define LOGFMT_MAX_SIZE 32
    LOGFMT_MAX_SIZE,
    1024
};
#define LOGAPI_PARAM cast(logpool_param_t *, &STRING_API_PARAM)
#define LOGAPI STRING_API

#include "logpool.h"
#include <stdbool.h>
extern logapi_t LOGAPI;
#define LOG_END 0
#define LOG_s   1
#define LOG_u   2
#define LOG_i   2
#define LOG_f   4

#define KeyValue_u(K,V)    LOG_u, (K), ((uintptr_t)V)
#define KEYVALUE_i(K,V)    LOG_i, (K), ((uintptr_t)V)
#define KEYVALUE_f(K,V)    LOG_f, (K), (f2u(V))
#define KeyValue_s(K,V)    LOG_s, (K), (V)


static void logpool_test_write(logpool_t *logpool)
{
    double f = 3.14;
    long   i = 128;
    const char *s = "hello world";
    logpool_record(logpool, NULL, LOG_NOTICE, "event",
            KEYVALUE_f("0:float", f),
            KEYVALUE_i("0:int",   i),
            KeyValue_s("0:string", s),
            KEYVALUE_f("1:float", f),
            KEYVALUE_i("1:int",   i),
            KeyValue_s("1:string", s),
            KEYVALUE_f("2:float", f),
            KEYVALUE_i("2:int",   i),
            KeyValue_s("2:string", s),
            KEYVALUE_f("3:float", f),
            KEYVALUE_i("3:int",   i),
            KeyValue_s("3:string", s),
            KEYVALUE_f("4:float", f),
            KEYVALUE_i("4:int",   i),
            KeyValue_s("4:string", s),
            KEYVALUE_f("5:float", f),
            KEYVALUE_i("5:int",   i),
            KeyValue_s("5:string", s),
            KEYVALUE_f("6:float", f),
            KEYVALUE_i("6:int",   i),
            KeyValue_s("6:string", s),
            KEYVALUE_f("7:float", f),
            KEYVALUE_i("7:int",   i),
            KeyValue_s("7:string", s),
            KEYVALUE_f("8:float", f),
            KEYVALUE_i("8:int",   i),
            KeyValue_s("8:string", s),
            KEYVALUE_f("9:float", f),
            KEYVALUE_i("9:int",   i),
            KeyValue_s("9:string", s),
            LOG_END
            );
}
int main()
{
    logpool_global_init(LOGPOOL_DEFAULT);
    {
        logpool_t *logpool = logpool_open(NULL, &LOGAPI, LOGAPI_PARAM);
        logpool_test_write(logpool);
        logpool_close(logpool);
    }
    return 0;
}
