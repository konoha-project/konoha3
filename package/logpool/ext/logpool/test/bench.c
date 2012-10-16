#define N 10000000
#include <konoha1.h>
#include <konoha1/inlinelibs.h>
#include <errno.h>
#define EBUFSIZ 4096

typedef struct {
    union {
        kintptr_t    type;
        const char     *key;
        kintptr_t   ivalue;
        kuintptr_t  uvalue;
        kfloatptr_t fvalue;
        const char    *svalue;
        void          *ptr;
        Object        *ovalue;
    };
} knh_ldata2_t;

static char *write_b(char *p, char *ebuf, const char *text, size_t len)
{
    if(ebuf - p > len) {
        knh_memcpy(p, text, len);
        return p+len;
    }
    else {
        return NULL;
    }
}

static char *write_s(char *p, char *ebuf, const knh_ldata2_t *d)
{
    const char *s = d->svalue;
    if(ebuf - p > strlen(s) * 2  + 1) {
        p[0] = '"'; p++;
        while(*s != 0) {
            if(*s == '"') {
                p[0] = '\"'; p++;
            }
            p[0] = s[0]; p++; s++;
        }
        p[0] = '"'; p++;
        return p;
    }
    else {
        if(p < ebuf) { p[0] = '"'; p++; }
        while(*s != 0) {
            if(*s == '"' && p < ebuf) {
                p[0] = '\"'; p++;
            }
            if(p < ebuf) {p[0] = s[0]; p++;}
            s++;
        }
        if(p < ebuf) { p[0] = '"'; p++; }
        else return NULL;
        return p;
    }
}

static char *write_key(char *p, char *ebuf, const char *key)
{
    if(ebuf - p < 32) return NULL;
    p[0] = '"'; p++;
    p = write_b(p, ebuf, key, strlen(key));
    if(p != NULL) {
        if(ebuf - p < 32) return NULL;
        p[0] = '"'; p++;
        p[0] = ':'; p++;
        p[0] = ' '; p++;
    }
    return p;
}

static void reverse(char *const start, char *const end, const int len)
{
    int i, l = len / 2;
    register char *s = start;
    register char *e = end - 1;
    for (i = 0; i < l; i++) {
        char tmp = *s;
        tmp  = *s;
        *s++ = *e;
        *e-- = tmp;
    }
}

static char *write_d(char *const p, const char *const end, kuintptr_t uvalue)
{
    int i = 0;
    while (p + i < end) {
        int tmp = uvalue % 10;
        uvalue /= 10;
        p[i] = '0' + tmp;
        ++i;
        if(uvalue == 0)
            break;
    }
    reverse(p, p + i, i);
    return p + i;
}

static char *write_i(char *p, char *ebuf, const knh_ldata2_t *d)
{
    if(ebuf - p < 32) return NULL;
    kuintptr_t uvalue = d->uvalue;
    if(d->ivalue < 0) {
        p[0] = '-'; p++;
        uvalue = -(d->ivalue);
    }
    kuintptr_t u = uvalue / 10, r = uvalue % 10;
    if(u != 0) {
        p = write_d(p, ebuf, u);
    }
    p[0] = ('0' + r);
    return p + 1;
}

static char *write_u(char *p, char *ebuf, const knh_ldata2_t *d)
{
    if(ebuf - p < 32) return NULL;
    kuintptr_t u = d->uvalue / 10, r = d->uvalue % 10;
    if(u != 0) {
        p = write_d(p, ebuf, u);
    }
    p[0] = ('0' + r);
    return p + 1;
}

static char *write_f(char *p, char *ebuf, const knh_ldata2_t *d)
{
    if(ebuf - p < 32) return NULL;
    kuintptr_t uvalue = (kuintptr_t)d->ivalue;
    if(d->ivalue < 0) {
        p[0] = '-'; p++;
    }
    kuintptr_t u = uvalue / 1000, r = uvalue % 1000;
    if(u != 0) {
        p = write_d(p, ebuf, u);
    }
    else {
        p[0] = '0'; p++;
    }
    p[0] = '.'; p++;
    u = r / 100;
    p[0] = ('0' + (u)); p++;
    p[0] = ('0' + (u / 10)); p++;
    p[0] = ('0' + (u % 10));
    return p + 1;
}

static char *write_o(char *p, char *ebuf, const knh_ldata2_t *d)
{
    return NULL; // TODO
}

typedef char* (*writedata)(char *, char *, const knh_ldata2_t *);

static writedata writeldata[] = {
    NULL,
    write_s, /*LOGT_s*/
    write_i, /*LOGT_i*/
    write_u, /*LOGT_u*/
    write_f, /*LOGT_f*/
    write_u, /*LOGT_p*/
    write_o, /*LOGT_o*/
};

static char *write_comma(char *p, char *ebuf, const knh_ldata2_t *d)
{
    if(d->type != 0) {
        p[0] = ','; p++;
        p[0] = ' '; p++;
    }
    return p;
}

static FILE *devnull;
static void ntrace(CTX ctx, const char *event, int pe, const knh_ldata2_t *d)
{
    char buf[EBUFSIZ], *p = buf, *ebuf =  p + (EBUFSIZ - 4);
    int logtype = LOG_NOTICE;
    p = write_b(p, ebuf, ctx->trace, strlen(ctx->trace));
    p[0] = '+'; p++;
    p = write_d(p, ebuf, ctx->seq);
    ((kcontext_t*)ctx)->seq += 1;
    p[0] = ' '; p++;
    p = write_b(p, ebuf, event, strlen(event));
    if(pe % 2 == 1) {
        p = write_b(p, ebuf, "*FAILED* ", strlen("*FAILED* "));
        logtype = LOG_ERR;
    }
    else {
        p[0] = ' '; p++;
    }
    p[0] = '{'; p++;
    if(pe % 2 == 1 && ((pe & FLAG_TRACE_ERRNO) == FLAG_TRACE_ERRNO)) {
        int errno_ = errno;
        if(errno_ > 0) {
            p = write_key(p, ebuf, "errno");
            p = write_d(p, ebuf, errno_);
            p[0] = ','; p++; p[0] = ' '; p++;
            knh_ldata2_t d[1];
            d[0].svalue = strerror(errno_);
            if(d[0].svalue != NULL) {
                p = write_key(p, ebuf, "msg");
                p = write_s(p, ebuf, d);
                p[0] = ','; p++; p[0] = ' '; p++;
            }
        }
    }
    while(d->type != 0) {
        char *p2 = write_key(p, ebuf, d[1].key);
        if(p2 != NULL) {
            if(d->type == LOGT_sfp) {
                // TODO
                d+=2; continue;
            }
            DBG_ASSERT(d->type <= LOGT_o);
            p2 = writeldata[d->type](p2, ebuf, d+2);
            if(p2 != NULL) {
                p = write_comma(p2, ebuf, d+3);
            }
        }
        d += 3;
    }
    p[0] = '}'; p++;
    p[0] = 0;
    fwrite(buf, p - buf + 1, 1, devnull);
}
#define NTRACE(ctx, event, pe, L) do {\
    knh_ldata_t ldata[] = L;\
    ntrace(ctx, event, pe, (const knh_ldata2_t *) ldata);\
} while (0)

#define N 10000000

uint64_t konoha_ntrace0(CTX ctx)
{
    int i;
    kuint64_t s = knh_getTimeMilliSecond();
    for (i = 0; i < N; ++i) {
        NTRACE(ctx, "setpgid", K_OK, KNH_LDATA0);
    }
    kuint64_t e = knh_getTimeMilliSecond();
    return e - s;
}

uint64_t konoha_ntrace1(CTX ctx)
{
    int pid  = 0x10;
    int pgid = 0x20;
    void *p  = (void*) 0xdeadbeaf;
    int i;
    kuint64_t s = knh_getTimeMilliSecond();
    for (i = 0; i < N; ++i) {
        NTRACE(ctx, "setpgid", K_OK, KNH_LDATA(LOG_i("pid", pid), LOG_s("send", "foo"), LOG_i("pgid", pgid), LOG_p("ptr", p)));
    }
    kuint64_t e = knh_getTimeMilliSecond();
    return e - s;
}

#undef LOG_s
#undef LOG_i
#undef LOG_p
#undef LOG_u
#undef LOG_f
#undef LOG_END
#include "logpool.h"

uint64_t logpool_ntrace1(ltrace_t *ltrace)
{
    int pid  = 0x10;
    int pgid = 0x20;
    void *p  = (void*) 0xdeadbeaf;
    int i;
    uint64_t s = knh_getTimeMilliSecond();
    for (i = 0; i < N; ++i) {
        ltrace_record(ltrace, LOG_NOTICE, "setpgid", LOG_i("pid", pid), LOG_s("send", "foo"), LOG_i("pgid", pgid), LOG_p("ptr", p), LOG_END);
    }
    uint64_t e = knh_getTimeMilliSecond();
    return e - s;
}
uint64_t logpool_ntrace0(ltrace_t *ltrace)
{
    int i;
    uint64_t s = knh_getTimeMilliSecond();
    for (i = 0; i < N; ++i) {
        ltrace_record(ltrace, "setpgid", LOG_END);
    }
    uint64_t e = knh_getTimeMilliSecond();
    return e - s;
}
uint64_t logpool_ntrace_jit0(ltrace_t *ltrace)
{
    int i;
    uint64_t s = knh_getTimeMilliSecond();
    for (i = 0; i < N; ++i) {
        ltrace_record(ltrace, "setpgid", LOG_END);
    }
    uint64_t e = knh_getTimeMilliSecond();
    return e - s;
}

uint64_t logpool_ntrace_jit1(ltrace_t *ltrace)
{
    int pid  = 0x10;
    int pgid = 0x20;
    void *p  = (void*) 0xdeadbeaf;
    int i;
    uint64_t s = knh_getTimeMilliSecond();
    for (i = 0; i < N; ++i) {
        ltrace_record(ltrace, LOG_NOTICE, "setpgid", LOG_i("pid", pid), LOG_s("send", "foo"), LOG_i("pgid", pgid), LOG_p("ptr", p), LOG_END);
    }
    uint64_t e = knh_getTimeMilliSecond();
    return e - s;
}

extern logapi_t FILE2_API;
extern logapi_t LLVM_FILE2_API;
void logpool_global_init(int mode);
int main(int argc, const char *argv[])
{
    static struct logpool_param_file ARGS = {
        1024,
        "/dev/null"
    };
    konoha_ginit(argc, argv);
    konoha_t konoha = konoha_open();
    logpool_global_init(LOGPOOL_DEFAULT);
    ltrace_t *ltrace0, *ltrace1;
    ltrace0 = ltrace_open(NULL, &FILE2_API, ARGS);
    logpool_global_init(LOGPOOL_JIT);
    ltrace1 = ltrace_open(NULL, &LLVM_FILE2_API, ARGS);
    devnull = fopen("/dev/null", "w");
    CTX ctx = konoha;
    int i;
    for (i = 0; i < 4; ++i) {
        uint64_t t1 = konoha_ntrace0(ctx);
        uint64_t t2 = konoha_ntrace1(ctx);
        uint64_t t3 = logpool_ntrace0(ltrace0);
        uint64_t t4 = logpool_ntrace1(ltrace0);
        uint64_t t5 = logpool_ntrace_jit0(ltrace1);
        uint64_t t6 = logpool_ntrace_jit1(ltrace1);
        fprintf(stderr, "%d:ntrace0:%lld\n", i, t1);
        fprintf(stderr, "%d:ntrace1:%lld\n", i, t2);
        fprintf(stderr, "%d:logvm0 :%lld\n", i, t3);
        fprintf(stderr, "%d:logvm1 :%lld\n", i, t4);
        fprintf(stderr, "%d:logjit0:%lld\n", i, t5);
        fprintf(stderr, "%d:logjit1:%lld\n", i, t6);

    }
    ltrace_close(ltrace0);
    ltrace_close(ltrace1);
    fclose(devnull);
    konoha_close(konoha);
    return 0;
}

