#include "qengine.h"

#ifdef __cplusplus
extern "C" {
#endif

int main(int argc, char const* argv[])
{
    struct qengine *e = qengine_init();
    struct qcode *c;
    struct qcode_list qlist_, *qlist = qcode_list_init(&qlist_);
    qcode_list_deinit(qlist);
    char *q = "match TraceID abcd1";
    c = qengine_compile(e, q);
    qcode_dump(c, stderr);
    c = qengine_compile(e, "gt cpu 30");
    qcode_dump(c, stderr);
    qengine_exit(e);
    return 0;
}

#ifdef __cplusplus
}
#endif
