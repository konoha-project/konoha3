#include "pool_plugin.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

static struct LogEntry *LogEntry_new(uint32_t size, uint64_t time)
{
    struct LogEntry *e = malloc(size);
    e->h.next = NULL;
    e->h.size = size;
    e->h.time = time;
    RefInit(e);
    return e;
}

static void LogList_init(struct LogList *list)
{
    struct LogEntry *head = LogEntry_new(sizeof(struct LogEntry), UINT64_MAX);
    list->head = head;
    list->tail = head;
}

static void LogList_check_timer(struct LogList *list, uint64_t current, uint64_t interval)
{
    struct LogEntry *head;
    head = list->head->h.next;
    while (head) {
        if(current - head->h.time < interval)
            break;
        DecRC(head);
        head = head->h.next;
    }
}

static void LogList_check_interval(struct LogList *list)
{
    struct LogEntry *e, *next, *prev;
    e    = list->head->h.next;
    prev = list->head;
    if(RC0(list->tail)) {
#ifdef DEBUG
        struct LogEntry *head = e;
        while (head) {
            assert(RC0(head));
            head = head->h.next;
        }
#endif
        list->tail = list->head;
    }
    while (e) {
        next = e->h.next;
        if(!RC0(e))
            break;
        free(e);
        e = next;
    }
    prev->h.next = e;
}

static void LogList_append(struct LogList *list, struct LogEntry *log, uint64_t interval)
{
    struct LogEntry *e;
    LogList_check_timer(list, log->h.time, interval);
    LogList_check_interval(list);
    e = LogEntry_new(log->h.size, log->h.time);
    memcpy(e, log, log->h.size);
    list->tail->h.next = e;
    list->tail = e;
}

//static int LogList_size(struct LogList *list)
//{
//    struct LogEntry *e = list->head;
//    int i = 0;
//    while (e) {
//        e = e->h.next;
//        ++i;
//    }
//    return i;
//}

static void LogList_dispose(struct LogList *list)
{
    struct LogEntry *e = list->head, *next;
    while (e) {
        next = e->h.next;
        free(e);
        e = next;
    }
}

static bool timer_apply(struct pool_plugin *_p, struct LogEntry *e, uint32_t state)
{
    struct pool_plugin_timer *p = (struct pool_plugin_timer *) _p;
    struct LogEntry *head, *next;
    LogList_append(&p->list, e, p->timer);
    head = p->list.head->h.next;
    //fprintf(stderr, "emit %d %d\n", LogList_size(&p->list), p->timer);
    p->base.apply->Apply(_p->apply, e, p->flag_start);
    while (head) {
        next = head->h.next;
        p->base.apply->Apply(_p->apply, head, p->flag_cont);
        head = next;
    }
    p->base.apply->Apply(_p->apply, e, p->flag_finish);
    return true;
}

static bool timer_failed(struct pool_plugin *_p, struct LogEntry *e, uint32_t state)
{
    struct pool_plugin_timer *p = (struct pool_plugin_timer *) _p;
    LogList_append(&p->list, e, p->timer);
    //fprintf(stderr, "buffered %d %d\n", LogList_size(&p->list), p->timer);
    return true;
}

static struct pool_plugin *pool_plugin_timer_create(struct pool_plugin *_p)
{
    struct pool_plugin_timer *p = (struct pool_plugin_timer *) _p;
    p->base.apply  = pool_plugin_init(_p->apply);
    p->base.failed = pool_plugin_init(_p->failed);
    p->base.Apply  = timer_apply;
    p->base.Failed = timer_failed;
    p->base.name = "timer";
    assert(p->timer > 0);
    LogList_init(&p->list);
    return _p;
}

static void pool_plugin_timer_dispose(struct pool_plugin *_p)
{
    struct pool_plugin_timer *p = (struct pool_plugin_timer *) _p;
    CHECK_PLUGIN("timer", _p);
    pool_plugin_dispose(p->base.apply);
    pool_plugin_dispose(p->base.failed);
    LogList_dispose(&p->list);
    bzero(p, sizeof(struct pool_plugin_timer));
    free(p);
}

EXPORT_POOL_PLUGIN(pool_plugin_timer) = {
    {0, NULL, NULL, pool_plugin_timer_create, pool_plugin_timer_dispose, timer_apply, timer_failed, NULL}
};

#ifdef __cplusplus
}
#endif
