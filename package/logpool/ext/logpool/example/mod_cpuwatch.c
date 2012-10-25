#include "plugins/pool/pool_plugin.h"
#include <stdio.h>

struct cpu_average {
    int sum;
    int size;
};

static uintptr_t p5_init(uintptr_t context)
{
    struct cpu_average *v = malloc(sizeof(struct cpu_average));
    v->sum = v->size = 0;
    return (uintptr_t) v;
}

static uintptr_t p5_exit(uintptr_t context)
{
    struct cpu_average *average = (struct cpu_average *) context;
    uintptr_t data = 0;
    if(average->size) {
        data = average->sum/ average->size;
    }
    free(average);
    return data > 90;
}
static uintptr_t p5_func(uintptr_t context, struct LogEntry *e)
{
    struct cpu_average *average = (struct cpu_average *) context;
    int vlen;
    char *val = LogEntry_get(e, "cpu", strlen("cpu"), &vlen);
    char *end = val + vlen;
    if(val) {
        uintptr_t cpu = strtol(val, &end, 10);
        average->sum  += cpu;
        average->size += 1;
    }
    return context;
}

static bool val_eq(void *v0, void *v1, uint16_t l0, uint16_t l1)
{
    return l0 == l1 && memcmp(v0, v1, l0) == 0;
}

struct pool_plugin *cpu_usage_init(struct bufferevent *bev)
{
    struct pool_plugin_print *p0 = POOL_PLUGIN_CLONE(pool_plugin_print);
    struct pool_plugin_val_filter *p1 = POOL_PLUGIN_CLONE(pool_plugin_val_filter);
    struct pool_plugin_copy    *p2 = POOL_PLUGIN_CLONE(pool_plugin_copy);
    struct pool_plugin_react   *p3 = POOL_PLUGIN_CLONE(pool_plugin_react);
    struct pool_plugin_timer   *p4 = POOL_PLUGIN_CLONE(pool_plugin_timer);
    struct pool_plugin_statics *p5 = POOL_PLUGIN_CLONE(pool_plugin_statics);
    struct pool_plugin_response *p6 = POOL_PLUGIN_CLONE(pool_plugin_response);
    p0->base.apply  = &p1->base;
    p1->base.apply  = &p2->base;
    p2->base.apply  = &p3->base;
    p3->base.apply  = &p4->base;
    p3->base.failed = &p4->base;
    p4->base.apply  = &p5->base;
    p5->base.apply  = &p6->base;
    {
        p1->key = "TraceID";
        p1->klen = strlen("TraceID");
        p1->val = "cpuevent";
        p1->vlen = strlen("cpuevent");
        p1->val_cmp = val_eq;
    }
    {
        p3->conf.traceName = "cpuevent";
        p3->conf.key = "cpu";
    }
    {
        p4->timer = 500;
        p4->flag_start = 0;
        p4->flag_cont = 1;
        p4->flag_finish = 2;
    }
    {
        p5->finit = p5_init;
        p5->fexit = p5_exit;
        p5->function = p5_func;
    }
    {
        p6->bev = bev;
    }
    return pool_plugin_init((struct pool_plugin *) p0);
}
