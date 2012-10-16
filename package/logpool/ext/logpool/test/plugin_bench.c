#define N 100000
#define M 2
#include <stdio.h>
#include "logpool.h"

#include <time.h>
#include <sys/time.h>
static inline uint64_t getTimeMilliSecond(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


uint64_t ntrace0(ltrace_t *ltrace)
{
    int i;
    uint64_t s = getTimeMilliSecond();
    for (i = 0; i < N; ++i) {
        ltrace_record(ltrace, "setpgid", LOG_END);
    }
    uint64_t e = getTimeMilliSecond();
    return e - s;
}

uint64_t ntrace1(ltrace_t *ltrace)
{
    int pid  = 0x10;
    int pgid = 0x20;
    void *p  = (void*) 0xdeadbeaf;
    int i;
    uint64_t s = getTimeMilliSecond();
    for (i = 0; i < N; ++i) {
        ltrace_record(ltrace, LOG_NOTICE, "setpgid", LOG_i("pid", pid), LOG_s("send", "foo"), LOG_i("pgid", pgid), LOG_p("ptr", p), LOG_END);
    }
    uint64_t e = getTimeMilliSecond();
    return e - s;
}
static void *MEMCACHE_API_PARAM[] = {
    (void*) 1024,
    (void*) "0.0.0.0",
    (void*) 11211L
};
extern logapi_t MEMCACHE_API;

static struct logpool_param_string STRING_API_PARAM = {8, 1024};
extern logapi_t STRING_API;

static struct logpool_param_file FILE_API_PARAM =  {8, 1024, "LOG"};
extern logapi_t FILE2_API;

static struct logpool_param_syslog SYSLOG_API_PARAM = {8, 1024};
extern logapi_t SYSLOG_API;

static void **ARGS[] = {
    //STRING_API_PARAM,
    FILE_API_PARAM,
    SYSLOG_API_PARAM,
    MEMCACHE_API_PARAM,
};

static logapi_t *APIs[] = {
    //&STRING_API,
    &FILE2_API,
    &SYSLOG_API,
    &MEMCACHE_API
};
static const char *NAMEs[] = {
    //"string",
    "file",
    "syslog",
    "memcache",
};

int main(int argc, const char *argv[])
{
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
    logpool_global_init(LOGPOOL_DEFAULT);
    ltrace_t *ltrace;
    int i, j;
    for (j = 0; j < ARRAY_SIZE(APIs); ++j) {
        const char *name = NAMEs[j];
        ltrace = ltrace_open(NULL, APIs[j], ARGS[j]);
        for (i = 0; i < M; ++i) {
            uint64_t t1 = ntrace0(ltrace);
            uint64_t t2 = ntrace1(ltrace);
            fprintf(stderr, "%d:%s:%lld\n", i, name, t1);
            fprintf(stderr, "%d:%s:%lld\n", i, name, t2);
        }
        ltrace_close(ltrace);
    }
    return 0;
}

