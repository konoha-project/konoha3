#include "message.idl.data.h"

#ifndef LOGPOOL_LOG_H
#define LOGPOOL_LOG_H

#ifdef __cplusplus
extern "C" {
#endif


struct Log;
struct LogEntry {
    struct LogHead {
        struct LogEntry *next;
        uint64_t time;
        uint16_t size;
        int16_t  refc;
    } h;
    struct Message data;
};

char *LogEntry_get(struct LogEntry *e, char *key, int klen, int *vlen);
#define RefInit(e)  ((e)->h.refc =  1)
#define IncRC(e, N) ((e)->h.refc += N)
#define DecRC(e)    ((e)->h.refc -= 1)
#define RC0(e)      ((e)->h.refc <  0)


#ifdef __cplusplus
}
#endif
#endif /* end of include guard */
