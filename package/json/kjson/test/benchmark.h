#include <sys/time.h>

#ifndef BENCHMARK_H_
#define BENCHMARK_H_

static struct timeval g_timer;
static void reset_timer()
{
    gettimeofday(&g_timer, NULL);
}

static inline void _show_timer(const char *s, size_t bufsz)
{
    struct timeval endtime;
    gettimeofday(&endtime, NULL);
    double sec = (endtime.tv_sec - g_timer.tv_sec)
        + (double)(endtime.tv_usec - g_timer.tv_usec) / 1000 / 1000;
    printf("%20s: %f sec\n", s, sec);
    printf("%20s: %f MB\n", s, ((double)bufsz)/1024/1024);
    printf("%20s: %f Mbps\n", s, ((double)bufsz)*8/sec/1000/1000);
}

static void show_timer(const char *s)
{
    struct timeval endtime;
    gettimeofday(&endtime, NULL);
    double sec = (endtime.tv_sec - g_timer.tv_sec)
        + (double)(endtime.tv_usec - g_timer.tv_usec) / 1000 / 1000;
    printf("%20s: %f sec\n", s, sec);
}

#endif /* end of include guard */
