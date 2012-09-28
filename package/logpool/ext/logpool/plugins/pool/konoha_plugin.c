#include <libmemcached/memcached.h>
#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/platform_posix.h>
#include "pool_plugin.h"
#include <stdio.h>
#include <syslog.h>

#ifndef K_PREFIX
#define K_PREFIX  "/usr/local"
#endif

//static const char* l_packname(const char *str)
//{
//	char *p = strrchr(str, '.');
//	return (p == NULL) ? str : (const char*)p+1;
//}
//
//static const char* l_packagepath(char *buf, size_t bufsiz, const char *fname)
//{
//	char *path = PLATAPI getenv_i("KONOHA_PACKAGEPATH"), *local = "";
//	if(path == NULL) {
//		path = PLATAPI getenv_i("KONOHA_HOME");
//		local = "/package";
//	}
//	if(path == NULL) {
//		path = PLATAPI getenv_i("HOME");
//		local = "/.konoha2/package";
//	}
//	snprintf(buf, bufsiz, "%s%s/%s/%s_glue.k", path, local, fname, l_packname(fname));
//#ifdef K_PREFIX
//	FILE *fp = fopen(buf, "r");
//	if(fp != NULL) {
//		fclose(fp);
//	}
//	else {
//		snprintf(buf, bufsiz, K_PREFIX "/konoha2/package" "/%s/%s_glue.k", fname, l_packname(fname));
//	}
//#endif
//	return (const char*)buf;
//}
//
//static const char* l_exportpath(char *pathbuf, size_t bufsiz, const char *pname)
//{
//	char *p = strrchr(pathbuf, '/');
//	snprintf(p, bufsiz - (p  - pathbuf), "/%s_exports.k", l_packname(pname));
//	FILE *fp = fopen(pathbuf, "r");
//	if(fp != NULL) {
//		fclose(fp);
//		return (const char*)pathbuf;
//	}
//	return NULL;
//}

static void dbg_p(const char *file, const char *func, int L, const char *fmt, ...)
{
    va_list ap;
    va_start(ap , fmt);
#if 0
    fflush(stdout);
    fprintf(stderr, "DEBUG(%s:%s:%d) ", file, func, L);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
#endif
    va_end(ap);
}

static const PlatformApiVar logpool_platform = {
    .name      = "shell",
    .stacksize = K_PAGESIZE * 4,
    .malloc_i  = malloc,
    .free_i    = free,
    .setjmp_i  = ksetjmp,
    .longjmp_i = klongjmp,
    .syslog_i        = syslog,
    .vsyslog_i       = vsyslog,
    .printf_i        = printf,
    .vprintf_i       = vprintf,
    .snprintf_i      = snprintf,
    .vsnprintf_i     = vsnprintf,
    .qsort_i         = qsort,
    .exit_i          = exit,
    .formatPackagePath  = formatPackagePath,
    .formatTransparentPath = formatTransparentPath,
    .loadPackageHandler = loadPackageHandler,
    .loadScript         = loadScript,
    .beginTag           = beginTag,
    .endTag             = endTag,
    .debugPrintf        = dbg_p
};

struct kRawPtr {
    KonohaObjectHeader h;
    void *rawptr;
};

extern kstatus_t MODSUGAR_eval(KonohaContext *kctx, const char *script, kfileline_t uline);

void konoha_plugin_init(KonohaContextVar **konohap, memcached_st **mcp)
{
    *konohap = (KonohaContextVar*)konoha_open(&logpool_platform);
    *mcp = memcached_create(NULL);
    KonohaContext *kctx = *konohap;
    kNameSpace *ns = KNULL(NameSpace);
    KImportPackage("sugar", ns, 0);
    KImportPackage("logpool", ns, 0);
    memcached_server_list_st servers;
    memcached_return_t rc;
    servers = memcached_server_list_append(NULL, "127.0.0.1", 11211, &rc);
    if (rc != MEMCACHED_SUCCESS) {
        fprintf(stderr, "memcached_server_list_append failed\n");
    }
    rc = memcached_server_push(*mcp, servers);
    memcached_server_list_free(servers);
}

struct pool_plugin *konoha_plugin_get(KonohaContext *kctx, memcached_st *mc, char *buf, size_t len, void *req)
{
    size_t vlen;
    uint32_t flags;
    memcached_return_t rc;
    char *script = memcached_get(mc, buf, strlen(buf), &vlen, &flags, &rc);
    kObject *ev = KLIB new_kObject(kctx, CT_Int/*Dummy*/, (uintptr_t)req);
    MODSUGAR_eval(kctx, script, 0);
    kNameSpace *ns = KNULL(NameSpace);
    kMethod *mtd = KLIB kNameSpace_getMethodByParamSizeNULL(kctx, ns, TY_System, MN_("initPlugin"), 1, MPOL_PARAMSIZE|MPOL_FIRST);
    if (mtd) {
        BEGIN_LOCAL(lsfp, K_CALLDELTA + 2);
        KSETv_AND_WRITE_BARRIER(0, lsfp[K_CALLDELTA+0].o, K_NULL, GC_NO_WRITE_BARRIER);
        KSETv_AND_WRITE_BARRIER(0, lsfp[K_CALLDELTA+1].o, ev, GC_NO_WRITE_BARRIER);
        KCALL(lsfp, 0, mtd, 2, K_NULL);
        END_LOCAL();
        kObject *ret = lsfp[0].o;
        struct pool_plugin *plugin = (struct pool_plugin *) ((struct kRawPtr*) ret)->rawptr;
        plugin = pool_plugin_init(plugin);
        return plugin;
    }
    return NULL;
}

