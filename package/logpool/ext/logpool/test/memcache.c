/*memcache api test*/
#include "logpool.h"
static struct logpool_param_memcache MEMCACHE_API_PARAM = {
    8,
    1024,
    "0.0.0.0",
    11211L
};
#define LOGAPI_PARAM cast(logpool_param_t *, &MEMCACHE_API_PARAM)
#define LOGAPI_INIT_FLAG (LOGPOOL_DEFAULT)
#define LOGAPI MEMCACHE_API
#include "test_main.c"
