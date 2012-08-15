#include "log.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#ifndef POOL_PLUGIN_H
#define POOL_PLUGIN_H

struct pool_plugin {
    uintptr_t flags;
    struct pool_plugin *apply;
    struct pool_plugin *failed;
    struct pool_plugin *(*Create)(struct pool_plugin *);
    void (*Dispose)(struct pool_plugin *);
    bool (*Apply)(struct pool_plugin *p, struct LogEntry *, uint32_t state);
    bool (*Failed)(struct pool_plugin *p, struct LogEntry *, uint32_t state);
    const char *name;
};

struct pool_plugin_copy {
    struct pool_plugin base;
};

struct pool_plugin_close {
    struct pool_plugin base;
    struct bufferevent *bev;
};

struct pool_plugin_print {
    struct pool_plugin base;
};

struct pool_plugin_key_filter {
    struct pool_plugin base;
    char *key;
    uint16_t klen;
};

struct pool_plugin_val_filter {
    struct pool_plugin base;
    bool (*val_cmp)(void *v0, void *v1, uint16_t l0, uint16_t l1);
    char *key, *val;
    uint16_t klen, vlen;
};

struct pool_plugin_react {
    struct pool_plugin base;
    struct react {
        uint32_t traceID0;
        uint32_t traceID1;
        uint32_t keyID0;
        uint32_t keyID1;
        struct poolmap_t *map;
    } r;
    struct react_conf {
        char *traceName;
        char *key;
    } conf;
};

struct pool_plugin_timer {
    struct pool_plugin base;
    uint64_t timer;
    uint32_t flag_start;
    uint32_t flag_cont;
    uint32_t flag_finish;
    struct LogList {
        struct LogEntry *head;
        struct LogEntry *tail;
    } list;
};

struct pool_plugin_statics {
    struct pool_plugin base;
    uintptr_t context;
    uintptr_t (*finit)(uintptr_t context);
    uintptr_t (*fexit)(uintptr_t context);
    uintptr_t (*function)(uintptr_t context, struct LogEntry *e);
};

struct bufferevent;
struct pool_plugin_response {
    struct pool_plugin base;
    struct bufferevent *bev;
};

struct io;
struct pool_plugin_stream {
    struct pool_plugin base;
    struct io *io;
};

struct pool_plugin_create {
    struct pool_plugin base;
    uintptr_t context;
    uintptr_t (*finit)(uintptr_t context, uint32_t state);
    uintptr_t (*fexit)(uintptr_t context);
    uint16_t (*write_size)(uintptr_t context, uint32_t state, uint16_t *lengths);
    void (*write_data)(uintptr_t context, struct LogEntry *e, char *buf);
};

void pool_process_log(struct pool_plugin *p, struct LogEntry *e);
struct pool_plugin *pool_plugin_init(struct pool_plugin *p);
struct pool_plugin *pool_plugin_clone(struct pool_plugin *p, uint32_t size);
void pool_plugin_dispose(struct pool_plugin *p);
#define POOL_PLUGIN_CLONE(T) ({\
    extern struct T T##_global;\
    struct T *_p = (struct T *)pool_plugin_clone((struct pool_plugin *) &T##_global, sizeof(struct T));\
    _p;})

#define CHECK_PLUGIN(PLUGIN, p) assert(strcmp(p->name, PLUGIN) == 0)
#define EXPORT_POOL_PLUGIN(P) struct P P##_global

struct pool_list;
struct pool_list * pool_new(void);
void pool_exec(struct Log *log, int logsize, struct pool_list *plist);
void pool_add(struct Procedure *q, struct bufferevent *bev, struct pool_list *l);
void pool_delete_connection(struct pool_list *l, struct bufferevent *bev);
void pool_delete(struct pool_list *l);

#endif /* end of include guard */
