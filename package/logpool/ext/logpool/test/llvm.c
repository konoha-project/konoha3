#include "logpool.h"
#include <stdbool.h>

static struct logpool_param_string LLVM_STRING_API_PARAM = {8, 1024};
extern logapi_t LLVM_STRING_API;

void ltrace_test(void) {
    ltrace_t *ltrace;
    const char *s = "hello world";
    logpool_global_init(LOGPOOL_JIT);
    ltrace = ltrace_open(NULL, &LLVM_STRING_API,
            (logpool_param_t *) &LLVM_STRING_API_PARAM);
    ltrace_record(ltrace, LOG_NOTICE, "test",
            LOG_s("string", s),
            LOG_END
            );
    ltrace_close(ltrace);
}

int main(void)
{
    ltrace_test();
    return 0;
}
