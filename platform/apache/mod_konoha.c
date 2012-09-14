/* 
 * mod_konoha
 *
 * This is a konoha module for Apache HTTP Server.
 *
 * ## Settings
 * Add to /path/to/httpd.conf
 *
 * FIXME: Current version of mod_konoha are not able
 *        to set KonohaPackageDir.
 * If you want to specify package dir for konoha,
 * use 'KonohaPackageDir'.
 *   LoadModule konoha_module modules/mod_konoha.so
 *   AddHandler konoha-script .k
 *   KonohaPackageDir /path/to/konoha/package
 *
 * Then after restarting Apache via
 *   $ apachectl restart
 *
 * void Request.puts(String x);
 */

#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"

#include "apr_strings.h"
#include "http_log.h"
#ifndef K_USE_PTHREAD
#define K_USE_PTHREAD 1
#endif
#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
extern int verbose_debug;
#include <minikonoha/platform_posix.h>
#include "../../package/apache/apache_glue.h"

#ifndef K_PREFIX
#define K_PREFIX  "/usr/local"
#endif

typedef struct konoha_config {
	const char *package_dir;
} konoha_config_t;

module AP_MODULE_DECLARE_DATA konoha_module;
static const char *apache_package_path = NULL;

static const char *apache_formatPackagePath(char *buf, size_t bufsiz, const char *packageName, const char *ext)
{
	if (apache_package_path) {
		snprintf(buf, bufsiz, "%s/%s/%s%s", apache_package_path, packageName, packname(packageName), ext);
		FILE *fp = fopen(buf, "r");
		if(fp != NULL) {
			fclose(fp);
			return buf;
		}
	}
	return formatPackagePath(buf, bufsiz, packageName, ext);
}

static KonohaPackageHandler *apache_loadPackageHandler(const char *packageName)
{
	char pathbuf[256];
	apache_formatPackagePath(pathbuf, sizeof(pathbuf), packageName, "_glue" K_OSDLLEXT);
	void *gluehdr = dlopen(pathbuf, RTLD_LAZY);
	//fprintf(stderr, "pathbuf=%s, gluehdr=%p", pathbuf, gluehdr);
	if(gluehdr != NULL) {
		char funcbuf[80];
		snprintf(funcbuf, sizeof(funcbuf), "%s_init", packname(packageName));
		PackageLoadFunc f = (PackageLoadFunc)dlsym(gluehdr, funcbuf);
		if(f != NULL) {
			return f();
		}
	}
	return NULL;
}

static const char* apache_beginTag(kinfotag_t t) { (void)t; return ""; }
static const char* apache_endTag(kinfotag_t t) { (void)t; return ""; }

static PlatformApi *getApachePlatform()
{
	PlatformApiVar *apache_platformvar = (PlatformApiVar *)KonohaUtils_getDefaultPlatformApi();
	apache_platformvar->name               = "apache";
	apache_platformvar->stacksize          = K_PAGESIZE;
	apache_platformvar->formatPackagePath  = apache_formatPackagePath;
	apache_platformvar->loadPackageHandler = apache_loadPackageHandler;
	apache_platformvar->beginTag           = apache_beginTag;
	apache_platformvar->endTag             = apache_endTag;
	return (PlatformApi *)apache_platformvar;
}

// class methodList start ==============================================================================================
// ## void Request.puts(String s)
static KMETHOD Request_puts(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	kString *data = sfp[1].asString;
	ap_rputs(S_text(data), self->r);
	RETURNvoid_();
}

// ## String Request.getMethod()
static KMETHOD Request_getMethod(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	RETURN_(KLIB new_kString(kctx, self->r->method, strlen(self->r->method), 0));
}
// ## String Request.getArgs();
static KMETHOD Request_getArgs(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	if(self->r->args == NULL) {
		RETURN_(KNULL(String));
	}
	RETURN_(KLIB new_kString(kctx, self->r->args, strlen(self->r->args), 0));
}
// ## String Request.getUri();
static KMETHOD Request_getUri(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	RETURN_(KLIB new_kString(kctx, self->r->uri, strlen(self->r->uri), 0));
}
// ## String Request.getPathInfo();
static KMETHOD Request_getPathInfo(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	RETURN_(KLIB new_kString(kctx, self->r->path_info, strlen(self->r->path_info), 0));
}
// ## String Request.getHandler();
static KMETHOD Request_getHandler(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	RETURN_(KLIB new_kString(kctx, self->r->handler, strlen(self->r->handler), 0));
}
// ## void Request.setContentType(String type);
static KMETHOD Request_setContentType(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	kString *type = sfp[1].asString;
	self->r->content_type = apr_pstrdup(self->r->pool, S_text(type));
	RETURNvoid_();
}
// ##void Request.setContentEncoding(String enc);
static KMETHOD Request_setContentEncoding(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	kString *enc = sfp[1].asString;
	self->r->content_encoding = apr_pstrdup(self->r->pool, S_text(enc));
	RETURNvoid_();
}
// ## void Request.logRerror(int level, int status, String msg);
static KMETHOD Request_logError(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	int level = sfp[1].intValue;
	apr_status_t status = (apr_status_t)sfp[2].intValue;
	const char *msg = S_text(sfp[3].s);
	ap_log_rerror(APLOG_MARK, level, status, self->r, msg, NULL);
	RETURNvoid_();
}
// ## AprTable Request.getHeadersIn();
static KMETHOD Request_getHeadersIn(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	RETURN_(KLIB new_kObject(kctx, CT_AprTable, (uintptr_t)self->r->headers_in));
}
// ## AprTable Request.getHeadersOut();
static KMETHOD Request_getHeadersOut(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	RETURN_(KLIB new_kObject(kctx, CT_AprTable, (uintptr_t)self->r->headers_out));
}

// ## void AprTable.add(String key, String val)
static KMETHOD AprTable_add(KonohaContext *kctx, KonohaStack *sfp)
{
	kAprTable *self = (kAprTable *) sfp[0].asObject;
	const char *key = S_text(sfp[1].asString);
	const char *val = S_text(sfp[2].s);
	apr_table_add(self->tbl, key, val);
	RETURNvoid_();
}
// ## void AprTable.set(String key, String val)
static KMETHOD AprTable_set(KonohaContext *kctx, KonohaStack *sfp)
{
	kAprTable *self = (kAprTable *) sfp[0].asObject;
	const char *key = S_text(sfp[1].asString);
	const char *val = S_text(sfp[2].s);
	apr_table_set(self->tbl, key, val);
	RETURNvoid_();
}
// ## Array[AprTableEntry] AprTable.getElts()
static KMETHOD AprTable_getElts(KonohaContext *kctx, KonohaStack *sfp)
{
	kAprTable *self = (kAprTable *) sfp[0].asObject;
	kArray *arr = (kArray*)KLIB new_kObject(kctx, CT_Array, 0);
	const apr_array_header_t *apr_arr = apr_table_elts(self->tbl);
	const apr_table_entry_t *entries = (apr_table_entry_t *)apr_arr->elts;
	int i=0;
	for (i=0; i<apr_arr->nelts; i++) {
		KLIB kArray_add(kctx, arr, (kAprTableEntry *)KLIB new_kObject(kctx, CT_AprTableEntry, (uintptr_t)entries));
		entries++;
	}
	RETURN_(arr);
}
// ## void AprTableEntry.getKey()
static KMETHOD AprTableEntry_getKey(KonohaContext *kctx, KonohaStack *sfp)
{
	kAprTableEntry *self = (kAprTableEntry *) sfp[0].asObject;
	RETURN_(KLIB new_kString(kctx, self->entry->key, strlen(self->entry->key), 0));
}
// ## void AprTableEntry.getVal()
static KMETHOD AprTableEntry_getVal(KonohaContext *kctx, KonohaStack *sfp)
{
	kAprTableEntry *self = (kAprTableEntry *) sfp[0].asObject;
	RETURN_(KLIB new_kString(kctx, self->entry->val, strlen(self->entry->val), 0));
}
// class methodList end ==============================================================================================

KonohaContext* konoha_create(KonohaClass **cRequest)
{
	PlatformApi *apache_platform = getApachePlatform();
	KonohaContext* kctx = konoha_open(apache_platform);
	kNameSpace *ns = KNULL(NameSpace);
	KRequirePackage("apache", 0);
	*cRequest = CT_Request;
#define _P    kMethod_Public
#define _F(F) (intptr_t)(F)
#define TY_Req  (CT_Request->typeId)
#define TY_Tbl  (CT_AprTable->typeId)
#define TY_TblEntry  (CT_AprTableEntry->typeId)

	KonohaClass *CT_TblEntryArray = CT_p0(kctx, CT_Array, TY_TblEntry);
	ktype_t TY_TblEntryArray = CT_TblEntryArray->typeId;

	int FN_x = FN_("x");
	KDEFINE_METHOD MethodData[] = {
		_P, _F(Request_puts), TY_void, TY_Req, MN_("puts"), 1, TY_String, FN_x,
		_P, _F(Request_getMethod), TY_String, TY_Req, MN_("getMethod"), 0,
		_P, _F(Request_getArgs), TY_String, TY_Req, MN_("getArgs"), 0,
		_P, _F(Request_getUri), TY_String, TY_Req, MN_("getUri"), 0,
		_P, _F(Request_getPathInfo), TY_String, TY_Req, MN_("getPathInfo"), 0,
		_P, _F(Request_getHandler), TY_String, TY_Req, MN_("getHandler"), 0,
		_P, _F(Request_setContentType), TY_void, TY_Req, MN_("setContentType"), 1, TY_String, FN_("type"),
		_P, _F(Request_setContentEncoding), TY_void, TY_Req, MN_("setContentEncoding"), 1, TY_String, FN_("enc"),
		_P, _F(Request_logError), TY_void, TY_Req, MN_("logError"), 3, TY_int, FN_("level"), TY_int, FN_("status"), TY_String, FN_("msg"),
		_P, _F(Request_getHeadersIn), TY_Tbl, TY_Req, MN_("getHeadersIn"), 0,
		_P, _F(Request_getHeadersOut), TY_Tbl, TY_Req, MN_("getHeadersOut"), 0,
		_P, _F(AprTable_add), TY_void, TY_Tbl, MN_("add"), 2, TY_String, FN_("key"), TY_String, FN_("val"),
		_P, _F(AprTable_set), TY_void, TY_Tbl, MN_("set"), 2, TY_String, FN_("key"), TY_String, FN_("val"),
		_P, _F(AprTable_getElts), TY_TblEntryArray, TY_Tbl, MN_("getElts"), 0,
		_P, _F(AprTableEntry_getKey), TY_String, TY_TblEntry, MN_("getKey"), 0,
		_P, _F(AprTableEntry_getVal), TY_String, TY_TblEntry, MN_("getVal"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return kctx;
}

static int konoha_handler(request_rec *r)
{
	//konoha_config_t *conf = ap_get_module_config(
	//		r->server->module_config, &konoha_module);
	if (strcmp(r->handler, "konoha-script")) {
		return DECLINED;
	}
	// if (r->method_number != M_GET) {
	// 	 TODO 
	// 	return HTTP_METHOD_NOT_ALLOWED;
	// }
	KonohaClass *cRequest;
	verbose_debug = 1;
	KonohaContext* konoha = konoha_create(&cRequest);
	//assert(cRequest != NULL);
	r->content_encoding = "utf-8";
	ap_log_rerror(APLOG_MARK, APLOG_CRIT, 0, r, "filename=%s", r->filename);
	if (konoha_load(konoha, r->filename)) {
		return DECLINED;
	}

	KonohaContext *kctx = konoha;
	kNameSpace *ns = KNULL(NameSpace);
	kMethod *mtd = KLIB kNameSpace_getMethodByParamSizeNULL(kctx, ns, TY_System, MN_("handler"), -1);  // fixme
	if (mtd == NULL) {
		ap_log_rerror(APLOG_MARK, APLOG_CRIT, 0, r, "Apache.handler() not found");
		return -1;
	}

	/* XXX: We assume Request Object may not be freed by GC */
	kObject *req_obj = KLIB new_kObject(kctx, cRequest, (uintptr_t)r);
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 1);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+0].o, K_NULL, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+1].o, req_obj, GC_NO_WRITE_BARRIER);
	KCALL(lsfp, 0, mtd, 1, KLIB Knull(kctx, CT_Int));
	END_LOCAL();
	return lsfp[0].intValue;
}

static int mod_konoha_init(apr_pool_t *p, apr_pool_t *plog, apr_pool_t *ptemp, server_rec *s)
{
	/* TODO: Create Global Instance to share constants objects */
	(void)p;(void)plog;(void)ptemp;(void)s;
	return 0;
}

static void mod_konoha_child_init(apr_pool_t *pool, server_rec *server)
{
	/* TODO: Create VM Instance per child process */
	(void)pool;(void)server;
	//konoha_config_t *conf = (konoha_config_t *) ap_get_module_config(
	//		server->module_config, &konoha_module);
	//KonohaContext* konoha = konoha_open(&apache_platform);
}

static const char *set_package_dir(cmd_parms *cmd, void *vp, const char *arg)
{
	(void)vp;
	const char *err = ap_check_cmd_context(cmd, NOT_IN_FILES | NOT_IN_LIMIT);
	konoha_config_t *conf = (konoha_config_t *)
		ap_get_module_config(cmd->server->module_config, &konoha_module);
	if (err != NULL) {
		return err;
	}
	if (arg) {
		conf->package_dir = apr_pstrdup(cmd->pool, arg);
	}
	return NULL;
}

/* konoha commands */
static const command_rec konoha_cmds[] = {
	AP_INIT_TAKE1("KonohaPackageDir",
			set_package_dir,
			NULL,
			OR_ALL,
			"set konoha package path"),
	{ NULL, {NULL}, NULL, 0, RAW_ARGS, NULL }
};

/* konoha register hooks */
static void konoha_register_hooks(apr_pool_t *p)
{
	(void)p;
	ap_hook_post_config(mod_konoha_init, NULL, NULL, APR_HOOK_MIDDLE);
	ap_hook_child_init(mod_konoha_child_init, NULL, NULL, APR_HOOK_MIDDLE);
	ap_hook_handler(konoha_handler, NULL, NULL, APR_HOOK_FIRST);
}

/* Dispatch list for API hooks */
module AP_MODULE_DECLARE_DATA konoha_module = {
	STANDARD20_MODULE_STUFF,
	NULL,       /* create per-dir    config structures */
	NULL,       /* merge  per-dir    config structures */
	NULL,       /* create per-server config structures */
	NULL,       /* merge  per-server config structures */
	konoha_cmds,/* table of config file commands       */
	konoha_register_hooks  /* register hooks           */
};

