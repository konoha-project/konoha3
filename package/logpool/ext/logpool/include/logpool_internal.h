#include <assert.h>
#include <stdint.h>
#ifndef LOGPOOL_INTERNAL_H
#define LOGPOOL_INTERNAL_H

#ifndef unlikely
#define unlikely(x)   __builtin_expect(!!(x), 0)
#endif

#ifndef likely
#define likely(x)     __builtin_expect(!!(x), 1)
#endif

#ifdef __cplusplus
extern "C" {
#endif

static inline double u2f(uint64_t u)
{
    union {uint64_t u; double f;} v;
    v.u = u;
    return v.f;
}

struct logpool_plugin_header {
    void *param0;
    void *param1;
};

typedef void logplugin_t;

#define __UNUSED__ __attribute__((unused))

struct logpool;
__UNUSED__ static int logpool_default_priority(struct logpool *ctx __UNUSED__, int priority __UNUSED__)
{
    return 1;
}

#ifdef LOGPOOL_H_
static inline void logpool_context_switch(struct logpool *ctx, void *conn)
{
    ctx->connection = conn;
}
#endif

void logpool_format_flush(struct logpool *ctx);

#define LOGPOOL_DEBUG 1
#define LOGPOOL_DEBUG_LEVEL 1
#define debug_print(level, ...) do {\
    if(level >= LOGPOOL_DEBUG_LEVEL) {\
        if(LOGPOOL_DEBUG) {\
            fprintf(stderr, "[%s:%d] ", __func__, __LINE__);\
            fprintf(stderr, ## __VA_ARGS__);\
            fprintf(stderr, "\n");\
            fflush(stderr);\
        }\
    }\
} while (0)

#ifdef __cplusplus
}
#endif
#endif /* end of include guard */
