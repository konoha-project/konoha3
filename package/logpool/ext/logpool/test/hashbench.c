#include <stdio.h>
#include <stdint.h>
#include <string.h>
uint32_t djbhash0(const char *p, uint32_t len)
{
    uint32_t hash = 5381;
    uint32_t i;
    for(i = 0; i < len; ++i) {
        hash = ((hash << 5) + hash) + *p++;
    }
    return (hash & 0x7fffffff);
}

uint32_t djbhash1(const char *p, uint32_t len)
{
    uint32_t hash = 5381;
    uint32_t n = (len + 3) / 4;
    /* Duff's device */
    switch(len%4){
    case 0: do{ hash = ((hash << 5) + hash) + *p++;
    case 3:     hash = ((hash << 5) + hash) + *p++;
    case 2:     hash = ((hash << 5) + hash) + *p++;
    case 1:     hash = ((hash << 5) + hash) + *p++;
            } while(--n>0);
    }
    return (hash & 0x7fffffff);
}

#include <time.h>
#include <sys/time.h>
static inline uint64_t getTime(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int main(int argc, char const* argv[])
{
    int i, j;
    uint32_t h = 0;
    uint64_t s, e;
    char *str = argv[1];
    int len = strlen(str);
    for (j = 0; j < 10; ++j) {
#define N 10000000
        h = 0;
        s = getTime();
        for (i = 0; i < N; ++i) {
            h += djbhash0(str, len);
        }
        e = getTime();
        fprintf(stderr, "0:%x, time=%lld\n", h, e - s);
        h = 0;
        s = getTime();
        for (i = 0; i < N; ++i) {
            h += djbhash1(str, len);
        }
        e = getTime();
        fprintf(stderr, "1:%x, time=%lld\n", h, e - s);
    }

    return 0;
}
