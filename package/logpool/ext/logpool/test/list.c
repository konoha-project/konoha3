#include "reactive.h"
#include "io.h"
#include "protocol.h"

#include <stdio.h>
/* test */
#define TEST_ENTRY   10
#define TEST_WATCHER 100
#define TEST_LOG     100000000
#define TEST_WATCHER_PER_ENTRY 10

static void emit_log(struct LogList *list, int i)
{
    char buf[1024];
    char data[128];
    snprintf(data, 128, "%d", i);
    int logSize = emit_message(buf, LOGPOOL_EVENT_WRITE, 3,
            strlen("TraceID"), strlen(data), "TraceID", data,
            strlen("key0"), strlen("val0"), "key0", "val0",
            strlen("key1"), strlen("val1"), "key1", "val1");
    LogList_append(list, (struct Log *) buf, logSize);
}

int main(int argc, char const *argv[])
{
    struct LogList list;
    LogList_init(&list);
    int i;
    for (i = 0; i < 100; ++i) {
        emit_log(&list, i);
    }
    LogList_dispose(&list);
    return 0;
}
