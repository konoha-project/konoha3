#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "kjson.h"

static struct timeval g_timer;

static void reset_timer()
{
    gettimeofday(&g_timer, NULL);
}

static void show_timer(const char *s)
{
    struct timeval endtime;
    gettimeofday(&endtime, NULL);
    double sec = (endtime.tv_sec - g_timer.tv_sec)
        + (double)(endtime.tv_usec - g_timer.tv_usec) / 1000 / 1000;
    printf("%20s: %f sec\n", s, sec);
}

//static const unsigned int TASK_INT_NUM = 1<<24;
//static const unsigned int TASK_STR_LEN = 1<<15;
static const unsigned int TASK_INT_NUM = 1<<20;
static const unsigned int TASK_STR_LEN = 1<<11;
static const char* TASK_STR_PTR;

static int loop_count = 2;
void bench_kjson(void)
{
    puts("== KJSON ==");

    JSON o;
    char *buf;
    size_t len;
    reset_timer();
    {
        unsigned int i;
        o = JSONArray_new();
        for(i=0; i < TASK_STR_LEN; ++i) {
            JSON v = JSONString_new((char*)TASK_STR_PTR, i);
            JSONArray_append(o, v);
        }
    }
    show_timer("generate string");
    buf = JSON_toStringWithLength(o, &len);
    JSON_free(o);

    int i;
    for (i=0; i<loop_count; i++) {
        reset_timer();
        {
            o = parseJSON(buf, buf + len);
            if (o.bits == 0) {
                fprintf(stderr, "Errro\n");
            }
        }
        show_timer("parse string");
        JSON_free(o);
    }
    free(buf);
}

int main(int argc, char* argv[])
{
    char* str = malloc(TASK_STR_LEN);
    memset(str, 'a', TASK_STR_LEN);
    TASK_STR_PTR = str;
    if (argc > 1 && strncmp(argv[1], "-t", 2) == 0) {
        loop_count = atoi(argv[1]+2);
    }
    bench_kjson();
    free(str);
    return 0;
}
