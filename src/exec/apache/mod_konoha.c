/*
 * mod_konoha
 *
 * This is a konoha module for Apache HTTP Server.
 *
 * ## Settings
 * Add to /path/to/httpd.conf
 *
 * FIXME: Current version of mod_konoha are not able
 *        to set KPackageDir.
 * If you want to specify package dir for konoha,
 * use 'KPackageDir'.
 *   LoadModule konoha_module modules/mod_konoha.so
 *   AddHandler konoha-script .k
 *   KPackageDir /path/to/konoha/package
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
#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/klib.h>
#include <konoha3/platform.h>
#include <konoha3/import/methoddecl.h>
#include "../../package-devel/apache/apache_glue.h"

#ifndef K_PREFIX
#define K_PREFIX  "/usr/local"
#endif

typedef struct konoha_config {
	const char *package_dir;
} konoha_config_t;

module AP_MODULE_DECLARE_DATA konoha_module;
static const char *apache_package_path = NULL;

static const char *apache_FormatPackagePath(KonohaContext *kctx, char *buf, size_t bufsiz, const char *packageName, const char *ext)
{
	if(apache_package_path) {
		snprintf(buf, bufsiz, "%s/%s/%s%s", apache_package_path, packageName, ShortPackageName(packageName), ext);
		FILE *fp = fopen(buf, "r");
		if(fp != NULL) {
			fclose(fp);
			return buf;
		}
	}
	return FormatPackagePath(kctx, buf, bufsiz, packageName, ext);
}

static KPackageHandler *apache_LoadPackageHandler(KonohaContext *kctx, const char *packageName)
{
	char pathbuf[256];
	apache_FormatPackagePath(kctx, pathbuf, sizeof(pathbuf), packageName, "_glue" K_OSDLLEXT);
	void *gluehdr = dlopen(pathbuf, RTLD_LAZY);
	//fprintf(stderr, "pathbuf=%s, gluehdr=%p", pathbuf, gluehdr);
	if(gluehdr != NULL) {
		char funcbuf[80];
		snprintf(funcbuf, sizeof(funcbuf), "%s_Init", ShortPackageName(packageName));
		PackageLoadFunc f = (PackageLoadFunc)dlsym(gluehdr, funcbuf);
		if(f != NULL) {
			return f();
		}
	}
	return NULL;
}

static void ApacheFactory(KonohaFactory *factory)
{
	factory->name               = "apache";
	factory->stacksize          = K_PAGESIZE;
	factory->FormatPackagePath  = apache_FormatPackagePath;
	factory->LoadPackageHandler = apache_LoadPackageHandler;
}

// class methodList start
// ==============================================================================================

// ## void Request.puts(String s)
static KMETHOD Request_puts(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	kString *data = sfp[1].asString;
	ap_rputs(kString_text(data), self->r);
	KReturnVoid();
}

// ## String Request.getMethod()
static KMETHOD Request_getMethod(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	KReturn(KLIB new_kString(kctx, OnStack, self->r->method, strlen(self->r->method), 0));
}
// ## String Request.getArgs();
static KMETHOD Request_getArgs(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	if(self->r->args == NULL) {
		KReturn(KNULL(String));
	}
	KReturn(KLIB new_kString(kctx, OnStack, self->r->args, strlen(self->r->args), 0));
}
// ## String Request.getUri();
static KMETHOD Request_getUri(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	KReturn(KLIB new_kString(kctx, OnStack, self->r->uri, strlen(self->r->uri), 0));
}
// ## String Request.getPathInfo();
static KMETHOD Request_getPathInfo(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	KReturn(KLIB new_kString(kctx, OnStack, self->r->path_info, strlen(self->r->path_info), 0));
}
// ## String Request.getHandler();
static KMETHOD Request_getHandler(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	KReturn(KLIB new_kString(kctx, OnStack, self->r->handler, strlen(self->r->handler), 0));
}
// ## void Request.setContentType(String type);
static KMETHOD Request_SetContentType(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	kString *type = sfp[1].asString;
	self->r->content_type = apr_pstrdup(self->r->pool, kString_text(type));
	KReturnVoid();
}
// ##void Request.setContentEncoding(String enc);
static KMETHOD Request_SetContentEncoding(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	kString *enc = sfp[1].asString;
	self->r->content_encoding = apr_pstrdup(self->r->pool, kString_text(enc));
	KReturnVoid();
}
// ## void Request.logRerror(int level, int status, String msg);
static KMETHOD Request_logError(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	int level = sfp[1].intValue;
	apr_status_t status = (apr_status_t)sfp[2].intValue;
	const char *msg = kString_text(sfp[3].asString);
	ap_log_rerror(APLOG_MARK, level, status, self->r, msg, NULL);
	KReturnVoid();
}
// ## AprTable Request.getHeadersIn();
static KMETHOD Request_getHeadersIn(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	KReturn(KLIB new_kObject(kctx, OnStack, KClass_AprTable, (uintptr_t)self->r->headers_in));
}
// ## AprTable Request.getHeadersOut();
static KMETHOD Request_getHeadersOut(KonohaContext *kctx, KonohaStack *sfp)
{
	kRequest *self = (kRequest *) sfp[0].asObject;
	KReturn(KLIB new_kObject(kctx, OnStack,KClass_AprTable, (uintptr_t)self->r->headers_out));
}

// ## void AprTable.add(String key, String val)
static KMETHOD AprTable_Add(KonohaContext *kctx, KonohaStack *sfp)
{
	kAprTable *self = (kAprTable *) sfp[0].asObject;
	const char *key = kString_text(sfp[1].asString);
	const char *val = kString_text(sfp[2].asString);
	apr_table_add(self->tbl, key, val);
	KReturnVoid();
}
// ## void AprTable.set(String key, String val)
static KMETHOD AprTable_Set(KonohaContext *kctx, KonohaStack *sfp)
{
	kAprTable *self = (kAprTable *) sfp[0].asObject;
	const char *key = kString_text(sfp[1].asString);
	const char *val = kString_text(sfp[2].asString);
	apr_table_set(self->tbl, key, val);
	KReturnVoid();
}
// ## Array[AprTableEntry] AprTable.getElts()
static KMETHOD AprTable_getElts(KonohaContext *kctx, KonohaStack *sfp)
{
	kAprTable *self = (kAprTable *) sfp[0].asObject;
	kArray *arr = (kArray *)KLIB new_kObject(kctx, OnStack, KClass_Array, 0);
	const apr_array_header_t *apr_arr = apr_table_elts(self->tbl);
	const apr_table_entry_t *entries = (apr_table_entry_t *)apr_arr->elts;
	int i=0;
	for (i=0; i<apr_arr->nelts; i++) {
		KLIB kArray_Add(kctx, arr, (kAprTableEntry *)KLIB new_kObject(kctx, OnStack, KClass_AprTableEntry, (uintptr_t)entries));
		entries++;
	}
	KReturn(arr);
}
// ## void AprTableEntry.getKey()
static KMETHOD AprTableEntry_getKey(KonohaContext *kctx, KonohaStack *sfp)
{
	kAprTableEntry *self = (kAprTableEntry *) sfp[0].asObject;
	KReturn(KLIB new_kString(kctx, OnStack, self->entry->key, strlen(self->entry->key), 0));
}
// ## void AprTableEntry.getVal()
static KMETHOD AprTableEntry_getVal(KonohaContext *kctx, KonohaStack *sfp)
{
	kAprTableEntry *self = (kAprTableEntry *) sfp[0].asObject;
	KReturn(KLIB new_kString(kctx, OnStack, self->entry->val, strlen(self->entry->val), 0));
}
// class methodList end ==============================================================================================

void KonohaFactory_SetDefaultFactory(KonohaFactory *factory, void (*SetPlatformApi)(KonohaFactory *), int argc, char **argv);
KonohaContext* KonohaFactory_CreateKonoha(KonohaFactory *factory);
int Konoha_Destroy(KonohaContext *kctx);

KonohaContext* konoha_create(KClass **cRequest)
{
	struct KonohaFactory factory = {};
	int argc = 0;
	char *argv[1] = {0};
	KonohaFactory_SetDefaultFactory(&factory, PosixFactory, argc, argv);
	ApacheFactory(&factory);
	KonohaContext* kctx = KonohaFactory_CreateKonoha(&factory);
	KBaseTrace(trace);
	kNameSpace *ns = KNULL(NameSpace);
	KImportPackage(ns, "Lib.Apache", trace);
	*cRequest = KClass_Request;
#define KType_Req  (KClass_Request->typeId)
#define KType_Tbl  (KClass_AprTable->typeId)
#define KType_TblEntry  (KClass_AprTableEntry->typeId)

	KClass *KClass_TblEntryArray = KClass_p0(kctx, KClass_Array, KType_TblEntry);
	ktypeattr_t KType_TblEntryArray = KClass_TblEntryArray->typeId;

	int FN_x = KFieldName_("x");
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Request_puts), KType_void, KType_Req, KMethodName_("puts"), 1, KType_String, FN_x,
		_Public, _F(Request_getMethod), KType_String, KType_Req, KMethodName_("getMethod"), 0,
		_Public, _F(Request_getArgs), KType_String, KType_Req, KMethodName_("getArgs"), 0,
		_Public, _F(Request_getUri), KType_String, KType_Req, KMethodName_("getUri"), 0,
		_Public, _F(Request_getPathInfo), KType_String, KType_Req, KMethodName_("getPathInfo"), 0,
		_Public, _F(Request_getHandler), KType_String, KType_Req, KMethodName_("getHandler"), 0,
		_Public, _F(Request_SetContentType), KType_void, KType_Req, KMethodName_("setContentType"), 1, KType_String, KFieldName_("type"),
		_Public, _F(Request_SetContentEncoding), KType_void, KType_Req, KMethodName_("setContentEncoding"), 1, KType_String, KFieldName_("enc"),
		_Public, _F(Request_logError), KType_void, KType_Req, KMethodName_("logError"), 3, KType_Int, KFieldName_("level"), KType_Int, KFieldName_("status"), KType_String, KFieldName_("msg"),
		_Public, _F(Request_getHeadersIn), KType_Tbl, KType_Req, KMethodName_("getHeadersIn"), 0,
		_Public, _F(Request_getHeadersOut), KType_Tbl, KType_Req, KMethodName_("getHeadersOut"), 0,
		_Public, _F(AprTable_Add), KType_void, KType_Tbl, KMethodName_("add"), 2, KType_String, KFieldName_("key"), KType_String, KFieldName_("val"),
		_Public, _F(AprTable_Set), KType_void, KType_Tbl, KMethodName_("set"), 2, KType_String, KFieldName_("key"), KType_String, KFieldName_("val"),
		_Public, _F(AprTable_getElts), KType_TblEntryArray, KType_Tbl, KMethodName_("getElts"), 0,
		_Public, _F(AprTableEntry_getKey), KType_String, KType_TblEntry, KMethodName_("getKey"), 0,
		_Public, _F(AprTableEntry_getVal), KType_String, KType_TblEntry, KMethodName_("getVal"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return kctx;
}

static int konoha_handler(request_rec *r)
{
	//konoha_config_t *conf = ap_get_module_config(
	//		r->server->module_config, &konoha_module);
	if(strcmp(r->handler, "konoha-script")) {
		return DECLINED;
	}
	//if(r->method_number != M_GET) {
	//	 TODO
	//	return HTTP_METHOD_NOT_ALLOWED;
	//}
	KClass *cRequest;
	verbose_debug = 1;
	KonohaContext* konoha = konoha_create(&cRequest);
	//assert(cRequest != NULL);
	r->content_encoding = "utf-8";
	ap_log_rerror(APLOG_MARK, APLOG_CRIT, 0, r, "filename=%s", r->filename);
	if(Konoha_LoadScript(konoha, r->filename)) {
		return DECLINED;
	}

	KonohaContext *kctx = konoha;
	kNameSpace *ns = KNULL(NameSpace);
	kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, KClass_System, KMethodName_("handler"), -1, KMethodMatch_NoOption);  // fixme
	if(mtd == NULL) {
		ap_log_rerror(APLOG_MARK, APLOG_CRIT, 0, r, "System.handler() not found");
		return -1;
	}

	/* XXX: We assume Request Object may not be freed by GC */
	kObject *req_obj = KLIB new_kObject(kctx, OnStack, cRequest, (uintptr_t)r);
	BEGIN_UnusedStack(lsfp);
	KUnsafeFieldSet(lsfp[K_CALLDELTA+0].asObject, K_NULL);
	KUnsafeFieldSet(lsfp[K_CALLDELTA+1].asObject, req_obj);
	{
		KonohaStack *sfp = lsfp + K_CALLDELTA;
		KStackSetMethodAll(sfp, KLIB Knull(kctx, KClass_Int), 0/*UL*/, mtd, 1);
		KStackCall(sfp);
	}
	END_UnusedStack();
	int ret = lsfp[0].intValue;
	Konoha_Destroy(konoha);
	return ret;
}

static int mod_konoha_Init(apr_pool_t *p, apr_pool_t *plog, apr_pool_t *ptemp, server_rec *s)
{
	/* TODO: Create Global Instance to share constants objects */
	(void)p;(void)plog;(void)ptemp;(void)s;
	return 0;
}

static void mod_konoha_child_Init(apr_pool_t *pool, server_rec *server)
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
	if(err != NULL) {
		return err;
	}
	if(arg) {
		conf->package_dir = apr_pstrdup(cmd->pool, arg);
	}
	return NULL;
}

/* konoha commands */
static const command_rec konoha_cmds[] = {
	AP_INIT_TAKE1("KPackageDir",
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
	ap_hook_post_config(mod_konoha_Init, NULL, NULL, APR_HOOK_MIDDLE);
	ap_hook_child_init(mod_konoha_child_Init, NULL, NULL, APR_HOOK_MIDDLE);
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

