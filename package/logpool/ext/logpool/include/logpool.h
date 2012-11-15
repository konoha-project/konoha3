#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <syslog.h> /* LOG_EMERG etc.. */

#ifndef LOGPOOL_H_
#define LOGPOOL_H_

#ifdef __cplusplus
extern "C" {
#endif

//#define LOGPOOL_USE_SEQUENCE 1
struct logpool;
typedef struct logpool logpool_t;
typedef struct logapi logapi_t;
typedef struct logfmt logfmt_t;

#ifndef LOG_EMERG
#define LOG_EMERG   0 /* system is unusable */
#define LOG_ALERT   1 /* action must be taken immediately */
#define LOG_CRIT    2 /* critical conditions */
#define LOG_ERR     3 /* error conditions */
#define LOG_WARNING 4 /* warning conditions */
#define LOG_NOTICE  5 /* normal but significant condition */
#define LOG_INFO    6 /* informational */
#define LOG_DEBUG   7 /* debug-level messages */
#endif

/* param format */
typedef struct logpool_param {
    int logfmt_capacity;
} logpool_param_t;

struct logpool_param_string {
    int logfmt_capacity;
    uintptr_t buffer_size;
};

struct logpool_param_stream {
    int logfmt_capacity;
    uintptr_t buffer_size;
    const char *host;
    long port;
};

struct logpool_param_memcache {
    int logfmt_capacity;
    uintptr_t buffer_size;
    const char *host;
    long port;
};

struct logpool_param_syslog {
    int logfmt_capacity;
    uintptr_t buffer_size;
};

struct logpool_param_file {
    int logfmt_capacity;
    uintptr_t buffer_size;
    const char *fname;
};

struct logpool_param_filter {
    int logfmt_capacity;
    int priority;
    logapi_t *api;
    struct logpool_param *param;
};

#define LOGPOOL_MULTIPLEXER_MAX 4
struct logpool_param_multiplexer {
    int logfmt_capacity;
    int argc;
    struct plugin_param {
        logapi_t *api;
        struct logpool_param *param;
    } args[LOGPOOL_MULTIPLEXER_MAX];
};

/* log formatter API */
typedef void  (*logFn)(logpool_t *, const char *k, uint64_t v, short klen, short vlen);
typedef char *(*keyFn)(logpool_t *, uint64_t v, uint64_t seq, short len);

struct keyapi {
    keyFn hex;
    keyFn str;
};

struct logfmt {
    logFn fn;
    union key {
        uint64_t seq;
        const char *key;
    } k;
    union logval {
        uint64_t u;
        double f;
        char *s;
    } v;
    short klen;
    short vlen;
};

struct logapi {
    logFn fn_null;
    logFn fn_bool;
    logFn fn_int;
    logFn fn_hex;
    logFn fn_float;
    logFn fn_char;
    logFn fn_string;
    logFn fn_raw;
    void  (*fn_delim)(logpool_t *);
    void  (*fn_flush)(logpool_t *, void**);
    void *(*fn_Init) (logpool_t *, logpool_param_t *);
    void  (*fn_close)(logpool_t *);
    int   (*fn_priority)(logpool_t *, int);
};

enum LOGPOOL_EXEC_MODE {
    LOGPOOL_DEFAULT = 1,
    LOGPOOL_JIT     = 2,
    LOGPOOL_TRACE   = 4
};

void logpool_global_Init(int mode);
void logpool_global_exit(void);

struct logpool {
    void *connection;
    keyFn     fn_key;
    logapi_t *formatter;
    logfmt_t logkey;
    logfmt_t *fmt;
    long logfmt_size;
    long logfmt_capacity;
    uintptr_t is_flushed;
    logpool_t *parent;
};

logpool_t *logpool_open(logpool_t *parent, struct logapi *api, logpool_param_t *);
logpool_t *logpool_open_trace(logpool_t *parent, char *host, int port);
void logpool_close(logpool_t *p);

void logpool_record(logpool_t *logpool, void *args, int priority, char *trace_id, ...);
void logpool_record_ap(logpool_t *ctx, void *args, int priority, char *trace_id, va_list ap);
int  logpool_Check_priority(logpool_t *logpool, int priority);

struct logdata {
    long  type;
    short klen;
    short vlen;
    char *key;
    char *val;
};

void logpool_record_list(logpool_t *ctx, void *args, int priority, char *trace_id, int logsize, struct logdata *logs);

#define cast(T, V) ((T)(V))

static inline uint64_t f2u(double f)
{
    union {uint64_t u; double f;} v;
    v.f = f;
    return v.u;
}

/* Logpool Client API */
logpool_t *logpool_open_client(logpool_t *parent, char *host, int port);
void logpool_procedure(logpool_t *logpool, char *q, int qlen);
void logpool_query(logpool_t *logpool, char *q, int qlen);
void *logpool_client_get(logpool_t *logpool, void *buff, size_t bufsize);

/* Logpool Daemon API */
int logpoold_start(char *host, int port);

#ifdef __cplusplus
}
#endif

#endif /* end of include guard */
