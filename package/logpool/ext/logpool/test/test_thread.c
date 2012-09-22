#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <logpool/logpool.h>

static void *thread_main(void *args)
{
    logpool_t *logpool = logpool_open_trace(NULL, "0.0.0.0", 14801);
    void *lp_args;
#define LOG_END 0
#define LOG_u   2
#define LogUint(K,V)    LOG_u, (K), ((uintptr_t)V)

    int i;
    for (i = 0; i < 1024; ++i) {
        logpool_record(logpool, &lp_args, LOG_NOTICE, "thread",
                LogUint("ID", (uintptr_t)args),
                LogUint("V", i),
                LOG_END);
    }
    logpool_close(logpool);
    usleep(100);
    return NULL;
}

int main(int argc, char const* argv[])
{
    logpool_global_init(LOGPOOL_TRACE);
#define N 16
    pthread_t threads[N];
    long i;
    for (i = 0; i < N; i++) {
        pthread_create(&threads[i], NULL, thread_main, (void*)i);
    }
    for (i = 0; i < N; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "join error\n");
        }
    }
    logpool_global_exit();
    return 0;
}
