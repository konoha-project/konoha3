#include <stdbool.h>
#include <unistd.h>

#ifndef LOGAPI
#error define LOGAPI && LOGAPI_PARAM
#endif
#ifndef LOGPOOL_TEST_COUNT
#define LOGPOOL_TEST_COUNT(argc, argv) 5
#endif
extern logapi_t LOGAPI;
#define LOG_END 0
#define LOG_s   1
#define LOG_u   2
#define LOG_i   2
#define LOG_f   4

#define LogUint(K,V)    LOG_u, (K), ((uintptr_t)V)
#define LogText(K,V)    LOG_s, (K), (V)


int n = 0;
static void logpool_test_write(logpool_t *logpool)
{
    long   i = n;
    const char *s = "hello world";
    void *args;
    logpool_record(logpool, &args, LOG_NOTICE, "event",
            LogUint("uint",   i),
            LogUint("tid",   i/10),
            LogText("string", s),
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
