#include <syslog.h>
#include "logpool.h"
#include "logpool_internal.h"
#include "lpstring.h"

#ifdef __cplusplus
extern "C" {
#endif

static void logpool_syslog_flush(logpool_t *ctx, void **args __UNUSED__)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    logpool_format_flush(ctx);
    logpool_string_flush(ctx);
#if 0
    syslog(LOG_NOTICE, buf->base);
#endif
    syslog(LOG_NOTICE, "%s", buf->base);
    logpool_string_reset(ctx);
}

struct logapi SYSLOG_API = {
    logpool_string_null,
    logpool_string_bool,
    logpool_string_int,
    logpool_string_hex,
    logpool_string_float,
    logpool_string_char,
    logpool_string_string,
    logpool_string_raw,
    logpool_string_delim,
    logpool_syslog_flush,
    logpool_string_init,
    logpool_string_close,
    logpool_default_priority
};

#ifdef __cplusplus
}
#endif
