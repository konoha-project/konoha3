#include "plugins/pool/pool_plugin.h"
#include "io.h"

struct cpu_average {
    int sum;
    int size;
};

uintptr_t p4_init(uintptr_t context)
{
    struct cpu_average *v = malloc(sizeof(struct cpu_average));
    return (uintptr_t) v;
}
uintptr_t p4_exit(uintptr_t context)
{
    struct cpu_average *average = (struct cpu_average*) context;
    free(average);
    return 0;
}
uintptr_t p4_func(uintptr_t context, struct LogEntry *e)
{
    struct cpu_average *average = (struct cpu_average*) context;
    int vlen;
    char *val = LogEntry_get(e, "cpu", strlen("cpu"), &vlen);
    char *end = val + vlen;
    if(val) {
        average->sum  += strtol(val, &end, 10);
        average->size += 1;
    }
    return context;
}

int main(int argc, char const* argv[])
{
    struct pool_plugin_print *p = POOL_PLUGIN_CLONE(pool_plugin_print);
    struct pool_plugin_val_filter *p0 = POOL_PLUGIN_CLONE(pool_plugin_val_filter);
    struct pool_plugin_copy    *p1 = POOL_PLUGIN_CLONE(pool_plugin_copy);
    struct pool_plugin_react   *p2 = POOL_PLUGIN_CLONE(pool_plugin_react);
    struct pool_plugin_timer   *p3 = POOL_PLUGIN_CLONE(pool_plugin_timer);
    struct pool_plugin_statics *p4 = POOL_PLUGIN_CLONE(pool_plugin_statics);
    struct pool_plugin_create  *p5 = POOL_PLUGIN_CLONE(pool_plugin_create);
    struct pool_plugin_stream  *p6 = POOL_PLUGIN_CLONE(pool_plugin_stream);
    p->base.apply   = &p0->base;
    p0->base.apply  = &p1->base;
    p1->base.apply  = &p2->base;
    p2->base.apply  = &p3->base;
    p2->base.failed = &p3->base;
    p3->base.apply  = &p4->base;
    p4->base.apply  = &p5->base;
    p5->base.apply  = &p6->base;
    {
        p0->key = "TraceID";
        p0->klen = strlen("TraceID");
        p0->val = "tid0";
        p0->vlen = strlen("tid0");
    }
    {
        p2->conf.traceName = "tid0";
        p2->conf.key = "cpu_usage";
    }
    {
        p3->timer = 100;
        p3->flag_start = 0;
        p3->flag_cont = 1;
        p3->flag_finish = 2;
    }
    {
        p4->finit = p4_init;
        p4->fexit = p4_exit;
        p4->function = p4_func;
    }
    {
        p6->io = io_open_trace("0.0.0.0", 148001);
    }
    p = (struct pool_plugin_print *) pool_plugin_init((struct pool_plugin *) p);
    struct LogEntry e = {};
    e.h.size = sizeof(struct LogEntry);
    pool_process_log((struct pool_plugin *) p, &e);
    pool_plugin_dispose((struct pool_plugin *) p);
    return 0;
}
