#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <event2/bufferevent.h>
#include "message.idl.data.h"
#ifndef PROTOCOL_H
#define PROTOCOL_H

enum LOGPOOL_EVENT_PROTOCOL {
    LOGPOOL_EVENT_NULL,
    LOGPOOL_EVENT_WRITE,
    LOGPOOL_EVENT_READ,
    LOGPOOL_EVENT_QUIT
};

//struct log_data {
//    uint32_t crc32;
//    uint16_t protocol, logsize, klen, vlen;
//    char data[128];
//};
#define LOG_PROTOCOL_FIELDS 4
#define LOG_PROTOCOL_SIZE (sizeof(struct Message))

static inline enum LOGPOOL_EVENT_PROTOCOL log_data_protocol(struct Log *data)
{
    return (enum LOGPOOL_EVENT_PROTOCOL) data->protocol;
}

static inline uint16_t log_data_process(struct Log *data)
{
    uint16_t protocol = log_data_protocol(data);
    switch (protocol) {
        case LOGPOOL_EVENT_NULL:
        case LOGPOOL_EVENT_READ:
            break;
        case LOGPOOL_EVENT_WRITE:
#ifdef LOGPOOL_DEBUG
            debug_print(0, "%d, %d, '%s'\n", data->klen, data->vlen, data->data);
#endif
            break;
        case LOGPOOL_EVENT_QUIT:
#ifdef LOGPOOL_DEBUG
            debug_print(0, "quit, %d, %d\n", data->klen, data->vlen);
#endif
            break;
        default:
            /*TODO*/abort();
            break;
    }
    return protocol;
}

static inline char *log_get_data(struct Log *log)
{
    int offset = LOG_PROTOCOL_FIELDS+(log->logsize)*2;
    uint16_t *p = (uint16_t*)log;
    return (char *)(p + offset);
}

static inline uint16_t log_get_length(struct Log *log, uint16_t idx)
{
    int offset = LOG_PROTOCOL_FIELDS+idx;
    return ((uint16_t*)log)[offset];
}

static inline char *log_iterator(struct Log *log, char *cur, uint16_t idx)
{
    uint16_t klen = log_get_length(log, idx*2+0);
    uint16_t vlen = log_get_length(log, idx*2+1);
    return cur + klen + vlen;
}

static inline char *log_get_trace(struct Log *log)
{
    char *traceID = log_get_data(log) + log_get_length(log, 1);
    return traceID;
}

static inline void log_dump(FILE *fp, char *prefix, struct Log *log, char *suffix, int force)
{
    if(
#ifdef LOGPOOL_DEBUG
            LOGPOOL_DEBUG || 
#endif
            force) {
        int i;
        char *data = log_get_data(log);
        uint16_t klen, vlen;
        fprintf(fp, "%s", prefix);
        for (i = 0; i < log->logsize; ++i) {
            char kbuf[64] = {};
            char vbuf[64] = {};
            char *next = log_iterator(log, data, i);
            klen = log_get_length(log, i*2+0);
            vlen = log_get_length(log, i*2+1);
            memcpy(kbuf, data,klen);
            memcpy(vbuf, data+klen, vlen);
            fprintf(fp, "%d, %d, '%s': '%s' ",
                    klen, vlen, kbuf, vbuf);
            data = next;
        }
        fprintf(fp, "%s", suffix);
    }
}

static inline int emit_message(char *buf, uint16_t protocol, uint16_t logsize, ...)
{
    va_list ap;
    char *key, *val;
    uint16_t i, klen, vlen, total_size = 0;
    uint16_t *loginfo;
    struct Log *tmp = (struct Log *) buf;

    va_start(ap, logsize);
    tmp->protocol = protocol;
    tmp->logsize  = logsize;
    loginfo = ((uint16_t*)buf) + LOG_PROTOCOL_FIELDS;
    buf = (char *) (loginfo + logsize * 2);
    for (i = 0; i < logsize; ++i) {
        klen = (uint16_t) va_arg(ap, unsigned long);
        vlen = (uint16_t) va_arg(ap, unsigned long);
        key  = va_arg(ap, char *);
        val  = va_arg(ap, char *);
        loginfo[i*2+0] = klen;
        loginfo[i*2+1] = vlen;
        if(klen) {
            memcpy(buf, key, klen);
            buf = buf + klen;
        }
        if(vlen) {
            memcpy(buf, val, vlen);
            buf = buf + vlen;
        }
        total_size += klen + vlen;
    }
    return LOG_PROTOCOL_SIZE + sizeof(uint16_t) * logsize * 2 + total_size;
}

char *Log_get(struct Log *log, char *key, int klen, int *vlen);

#endif /* end of include guard */
