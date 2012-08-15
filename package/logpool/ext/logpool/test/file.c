/* file api test */
#include "logpool.h"
static struct logpool_param_file FILE_API_PARAM = {
    8,
    1024,
    "LOG"
};
#define LOGAPI_PARAM cast(logpool_param_t *, &FILE_API_PARAM)
#define LOGAPI_INIT_FLAG (LOGPOOL_DEFAULT)
#define LOGAPI FILE_API
#include "test_main.c"
