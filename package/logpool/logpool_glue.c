/****************************************************************************
 * Copyright (c) 2012, the Konoha project authors. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <libmemcached/memcached.h>
#include <logpool/logpool.h>
#include <logpool/protocol.h>
#include <logpool/pool_plugin.h>
#include <logpool/io.h>
#include <logpool/message.idl.data.h>

typedef struct kRawPtr {
	KonohaObjectHeader h;
	void *rawptr;
} kRawPtr;

static void RawPtr_init(KonohaContext *kctx, kObject *po, void *conf)
{
	kRawPtr *o = (kRawPtr*)(po);
	o->rawptr = conf;
}
static void RawPtr_free(KonohaContext *kctx, kObject *po)
{
	kRawPtr *o = (kRawPtr*)(po);
	o->rawptr = NULL;
}
static void Logpool_free(KonohaContext *kctx, kObject *po)
{
	kRawPtr *o = (kRawPtr*)(po);
	if (o->rawptr) {
		logpool_close(o->rawptr);
		o->rawptr = NULL;
	}
}
static void Log_free(KonohaContext *kctx, kObject *po)
{
	kRawPtr *o = (kRawPtr*)(po);
	free(o->rawptr);
	o->rawptr = NULL;
}

static void Log_p(KonohaContext *kctx, KonohaStack *sfp, int pos, KUtilsWriteBuffer *wb)
{
	kRawPtr *self = (kRawPtr *) sfp[0].asObject;
	struct Log *log = (struct Log *) self->rawptr;
	int i;
	char *data = log_get_data(log);
	uint16_t klen, vlen;
	KLIB Kwb_printf(kctx, wb, "{");
	for (i = 0; i < log->logsize; ++i) {
		char kbuf[64] = {};
		char vbuf[64] = {};
		char *next = log_iterator(log, data, i);
		klen = log_get_length(log, i*2+0);
		vlen = log_get_length(log, i*2+1);
		memcpy(kbuf, data,klen);
		memcpy(vbuf, data+klen, vlen);
		KLIB Kwb_printf(kctx, wb, "'%s': '%s' ", kbuf, vbuf);
		data = next;
	}
	KLIB Kwb_printf(kctx, wb, "}");
}

//static kObject *new_RawPtr(KonohaContext *kctx, const KonohaClass *ct, void *ptr)
//{
//	kObject *ret = KLIB new_kObject(kctx, ct, ptr);
//	RawPtr_init(kctx, ret, ptr);
//	return ret;
//}

//## LogPool LogPool.new(String host, int port)
static KMETHOD LogPool_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kRawPtr *ret = (kRawPtr *) sfp[0].asObject;
	char *host = (char *) S_text(sfp[1].asString);
	int   port = sfp[2].intValue;
	RawPtr_init(kctx, sfp[0].asObject, logpool_open_client(NULL, host, port));
	RETURN_(ret);
}

//## Log LogPool.get()
static KMETHOD LogPool_get(KonohaContext *kctx, KonohaStack *sfp)
{
	logpool_t *lp = (logpool_t *) ((kRawPtr *) sfp[0].asObject)->rawptr;
	char *buf = malloc(256);
	char *ret = logpool_client_get(lp, buf, 256);
	kObject *log = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
	if (ret == NULL) {
		kObject_setNullObject(log, 1);
		free(buf);
	}
	RawPtr_init(kctx, log, buf);
	RETURN_(log);
}

//## String Log.get(String key)
static KMETHOD Log_get_(KonohaContext *kctx, KonohaStack *sfp)
{
	kRawPtr *self = (kRawPtr *) sfp[0].asObject;
	struct Log *log = (struct Log *) self->rawptr;
	char *key  = (char *) S_text(sfp[1].asString);
	int   klen = S_size(sfp[1].asString);
	int   vlen;
	char *data = Log_get(log, key, klen, &vlen);
	RETURN_(KLIB new_kString(kctx, data, vlen, SPOL_ASCII|SPOL_POOL));
}

// --------------------------------------------------------------------------
struct konoha_context {
	uintptr_t id;
	KonohaContext* konoha;
	kFunc *func;
	kFunc *finit;
	kFunc *fexit;
};

static uintptr_t p_init(uintptr_t context)
{
	struct konoha_context *c = malloc(sizeof(struct konoha_context));
	memcpy(c, (struct konoha_context*) context, sizeof(*c));
	KonohaContext *kctx = c->konoha;
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 5);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+0].o, c->finit->self, GC_NO_WRITE_BARRIER);
	KCALL(lsfp, 0, c->finit->mtd, 0, K_NULL);
	END_LOCAL();
	return (uintptr_t) c;
}

static uintptr_t p_exit(uintptr_t context)
{
	struct konoha_context *c = malloc(sizeof(struct konoha_context));
	KonohaContext *kctx = c->konoha;
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 5);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+0].o, c->fexit->self, GC_NO_WRITE_BARRIER);
	KCALL(lsfp, 0, c->fexit->mtd, 0, K_NULL);
	END_LOCAL();
	bzero(c, sizeof(*c));
	free(c);
	return lsfp[0].intValue;
}

static kRawPtr *Log_new(KonohaContext *kctx, struct Log *e)
{
	return NULL;
}

static uintptr_t p_func(uintptr_t context, struct LogEntry *e)
{
	struct konoha_context *c = malloc(sizeof(struct konoha_context));
	KonohaContext *kctx = c->konoha;
	kObject *log = (kObject *) Log_new(kctx, (struct Log *) &e->data);
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 5);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+0].o, c->func->self, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+1].o, log, GC_NO_WRITE_BARRIER);
	KCALL(lsfp, 0, c->func->mtd, 0, K_NULL);
	END_LOCAL();
	return context;
}

static bool val_eq(void *v0, void *v1, uint16_t l0, uint16_t l1)
{
	return l0 == l1 && memcmp(v0, v1, l0) == 0;
}

static char *copy_string(KonohaContext *kctx, kString *s)
{
	char *str = (char *) calloc(1, S_size(s));
	memcpy(str, S_text(s), S_size(s));
	return str;
}

// PoolPlugin Printer.create();
static KMETHOD Printer_create(KonohaContext *kctx, KonohaStack *sfp)
{
	struct pool_plugin_print *p = POOL_PLUGIN_CLONE(pool_plugin_print);
	kRawPtr *ret = (kRawPtr *) KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), (uintptr_t)p);
	RETURN_(ret);
}

// PoolPlugin ValFilter.create(String key, String val, String op);
static KMETHOD ValFilter_create(KonohaContext *kctx, KonohaStack *sfp)
{
	struct pool_plugin_val_filter *p = POOL_PLUGIN_CLONE(pool_plugin_val_filter);
	kRawPtr *ret = (kRawPtr *) KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), (uintptr_t)p);
	p->klen = S_size(sfp[1].asString);
	p->key  = copy_string(kctx, sfp[1].asString);
	p->vlen = S_size(sfp[2].s);
	p->val  = copy_string(kctx, sfp[2].s);
	if (strncmp(S_text(sfp[3].s), "eq", 2) == 0) {
		p->val_cmp = val_eq;
	} else {
		assert(0 && "TODO");
	}
	RETURN_(ret);
}

// PoolPlugin KeyFilter.create(String key);
static KMETHOD KeyFilter_create(KonohaContext *kctx, KonohaStack *sfp)
{
	struct pool_plugin_key_filter *p = POOL_PLUGIN_CLONE(pool_plugin_key_filter);
	kRawPtr *ret = (kRawPtr *) KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), (uintptr_t)p);
	p->klen = S_size(sfp[1].asString);
	p->key  = copy_string(kctx, sfp[2].s);
	RETURN_(ret);
}

// PoolPlugin React.create(String traceName, String key);
static KMETHOD React_create(KonohaContext *kctx, KonohaStack *sfp)
{
	struct pool_plugin_react *p = POOL_PLUGIN_CLONE(pool_plugin_react);
	kRawPtr *ret = (kRawPtr *) KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), (uintptr_t)p);
	p->conf.traceName = copy_string(kctx, sfp[1].asString);
	p->conf.key       = copy_string(kctx, sfp[2].s);
	RETURN_(ret);
}

// PoolPlugin Timer.create(int timer, int startFlat, int contFlat, int finFlag);
static KMETHOD Timer_create(KonohaContext *kctx, KonohaStack *sfp)
{
	struct pool_plugin_timer *p = POOL_PLUGIN_CLONE(pool_plugin_timer);
	kRawPtr *ret = (kRawPtr *) KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), (uintptr_t)p);
	p->timer = sfp[1].intValue;
	p->flag_start  = sfp[2].intValue;
	p->flag_cont   = sfp[3].intValue;
	p->flag_finish = sfp[4].intValue;
	RETURN_(ret);
}

// PoolPlugin Copy.create();
static KMETHOD Copy_create(KonohaContext *kctx, KonohaStack *sfp)
{
	struct pool_plugin_copy *p = POOL_PLUGIN_CLONE(pool_plugin_copy);
	kRawPtr *ret = (kRawPtr *) KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), (uintptr_t)p);
	RETURN_(ret);
}

static void *statics_init(KonohaContext *kctx, kFunc *initFo, kFunc *exitFo, kFunc *funcFo)
{
	struct konoha_context *c = malloc(sizeof(struct konoha_context));
	c->konoha = kctx;
	KSETv_AND_WRITE_BARRIER(NULL, c->finit, initFo, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, c->fexit, exitFo, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, c->func,  funcFo, GC_NO_WRITE_BARRIER);
	return (void*) c;
}

// PoolPlugin Statics.create(Func initFo, Func exitFo, Func func);
static KMETHOD Statics_create(KonohaContext *kctx, KonohaStack *sfp)
{
	struct pool_plugin_statics *p = POOL_PLUGIN_CLONE(pool_plugin_statics);
	kRawPtr *ret = (kRawPtr *) KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), (uintptr_t)p);
	p->context = (uintptr_t)statics_init(kctx, sfp[1].asFunc, sfp[2].asFunc, sfp[3].asFunc);
	p->finit = p_init;
	p->fexit = p_exit;
	p->function = p_func;
	RETURN_(ret);
}

// PoolPlugin Response.create(Event ev);
static KMETHOD Response_create(KonohaContext *kctx, KonohaStack *sfp)
{
	struct pool_plugin_response *p = POOL_PLUGIN_CLONE(pool_plugin_response);
	kRawPtr *ret = (kRawPtr *) KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), (uintptr_t)p);
	kRawPtr *ev = (kRawPtr *) sfp[1].asObject;
	p->bev = ev->rawptr;
	RETURN_(ret);
}

// void PoolPlugin.apply(PoolPlugin p);
static KMETHOD PoolPlugin_apply(KonohaContext *kctx, KonohaStack *sfp)
{
	kRawPtr *self = (kRawPtr *) sfp[0].asObject;
	kRawPtr *plug = (kRawPtr *) sfp[1].asObject;
	((struct pool_plugin *) self->rawptr)->apply = ((struct pool_plugin *) plug->rawptr);
}

// void PoolPlugin.failed(PoolPlugin p);
static KMETHOD PoolPlugin_failed(KonohaContext *kctx, KonohaStack *sfp)
{
	kRawPtr *self = (kRawPtr *) sfp[0].asObject;
	kRawPtr *plug = (kRawPtr *) sfp[1].asObject;
	((struct pool_plugin *) self->rawptr)->failed = ((struct pool_plugin *) plug->rawptr);
}

static char *loadFile(const char *file, size_t *plen)
{
	char *script = malloc(1024), *p = script;
	FILE *f;
	if ((f = fopen(file, "r")) != NULL) {
		char buf[1024];
		size_t len = 0;
		while ((len = fread(buf, 1, sizeof(buf), f)) > 0) {
			assert(p - script < 1024);
			memcpy(p, buf, len);
			p += len;
		}
		fclose(f);
	}
	*plen = p - script;
	return script;
}

// LogPool LogPool.loadFile(String key, String file);
static KMETHOD LogPool_loadFile(KonohaContext *kctx, KonohaStack *sfp)
{
	logpool_t *lp = (logpool_t *) ((kRawPtr *) sfp[0].asObject)->rawptr;
	kString *key  = sfp[1].asString;
	kString *file = sfp[2].s;
	memcached_st *mc = memcached_create(NULL);
	memcached_server_list_st servers;
	memcached_return_t rc;
	servers = memcached_server_list_append(NULL, "127.0.0.1", 11211, &rc);
	if (rc != MEMCACHED_SUCCESS) {
		fprintf(stderr, "memcached_server_list_append failed\n");
		RETURNvoid_();
	}

	rc = memcached_server_push(mc, servers);
	memcached_server_list_free(servers);

	size_t len;
	char *script = loadFile(S_text(file), &len);
	memcached_set(mc, S_text(key), S_size(key), script, len, 0, 0);
	logpool_procedure(lp, (char*)S_text(key), S_size(key));
	free(script);
	RETURNvoid_();
}

// --------------------------------------------------------------------------
#define _P kMethod_Public
#define _C kMethod_Const
#define _S kMethod_Static
#define _F(F)   (intptr_t)(F)
#define TY_Logpool  (ct0->typeId)
#define TY_Log      (ct1->typeId)

static kbool_t logpool_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	int i;
	static KDEFINE_CLASS Def0 = {
		.structname = "LogPool"/*structname*/,
		.typeId = TY_newid/*cid*/,
		.init = RawPtr_init,
		.free = Logpool_free,
	};
	KonohaClass *ct0 = KLIB kNameSpace_defineClass(kctx, ns, NULL, &Def0, pline);

	static KDEFINE_CLASS Def1 = {
		.structname = "Log"/*structname*/,
		.typeId = TY_newid/*cid*/,
		.init = RawPtr_init,
		.free = Log_free,
		.p    = Log_p,
	};
	KonohaClass *ct1 = KLIB kNameSpace_defineClass(kctx, ns, NULL, &Def1, pline);

	static KDEFINE_CLASS Def2 = {
		.structname = "PoolPlugin",
		.typeId = TY_newid,
		.init = RawPtr_init,
		.free = RawPtr_free,
	};
	KonohaClass *ct2 = KLIB kNameSpace_defineClass(kctx, ns, NULL, &Def2, pline);
#define TY_Plugin ct2->typeId
	static KDEFINE_CLASS Def3 = {
		.structname = "",
		.typeId = TY_newid,
		.init = RawPtr_init,
		.free = RawPtr_free,
	};
	Def3.superTypeId = ct2->typeId;
	static const char *names[] = {
		"Printer",
		"KeyFilter",
		"ValFilter",
		"React",
		"Timer",
		"Statics",
		"Copy",
		"Response",
	};
	KonohaClass *tbls[8];
#define TY_Printer   tbls[0]->typeId
#define TY_KeyFilter tbls[1]->typeId
#define TY_ValFilter tbls[2]->typeId
#define TY_React     tbls[3]->typeId
#define TY_Timer     tbls[4]->typeId
#define TY_Statics   tbls[5]->typeId
#define TY_Copy      tbls[6]->typeId
#define TY_Response  tbls[7]->typeId

	for (i = 0; i < 8; i++) {
		Def3.structname = names[i];
		tbls[i] = KLIB kNameSpace_defineClass(kctx, ns, NULL, &Def3, pline);
	}

	int FN_x = FN_("x");
	int FN_y = FN_("y");
	int FN_z = FN_("z");
	KDEFINE_METHOD MethodData[] = {
		_P|_C, _F(LogPool_new), TY_Logpool, TY_Logpool, MN_("new"), 2, TY_String, FN_x, TY_int, FN_y,
		_P|_C, _F(LogPool_get), TY_Log, TY_Logpool, MN_("get"), 0,
		_P|_C, _F(LogPool_loadFile), TY_void, TY_Logpool, MN_("loadFile"), 2, TY_String, FN_x, TY_String, FN_y,
		_P|_C, _F(Log_get_), TY_String, TY_Log, MN_("get"), 1, TY_String, FN_x,
		_P|_S, _F(Printer_create  ), TY_Plugin, TY_Printer  , MN_("create"), 0,
		_P|_S, _F(KeyFilter_create), TY_Plugin, TY_KeyFilter, MN_("create"), 1, TY_String, FN_x,
		_P|_S, _F(ValFilter_create), TY_Plugin, TY_ValFilter, MN_("create"), 3, TY_String, FN_x, TY_String, FN_y, TY_String, FN_z,

		_P|_S, _F(React_create    ), TY_Plugin, TY_React    , MN_("create"), 2, TY_String, FN_x, TY_String, FN_y,
		_P|_S, _F(Timer_create    ), TY_Plugin, TY_Timer    , MN_("create"), 3, TY_int, FN_x, TY_int, FN_y, TY_int, FN_z,
		_P|_S, _F(Statics_create  ), TY_Plugin, TY_Statics  , MN_("create"), 3, TY_Func, FN_x, TY_Func, FN_y, TY_Func, FN_z,
		_P|_S, _F(Copy_create     ), TY_Plugin, TY_Copy     , MN_("create"), 0,
		_P|_S, _F(Response_create ), TY_Plugin, TY_Response , MN_("create"), 1, TY_Object, FN_x,
		_P   , _F(PoolPlugin_apply ), TY_void,  TY_Plugin   , MN_("apply"), 1, TY_Plugin, FN_x,
		_P   , _F(PoolPlugin_failed), TY_void,  TY_Plugin   , MN_("failed"), 1, TY_Plugin, FN_x,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t logpool_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t logpool_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t logpool_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}
KDEFINE_PACKAGE* logpool_init(void)
{
	logpool_global_init(LOGPOOL_DEFAULT);
	static KDEFINE_PACKAGE d = {
		KPACKNAME("logpool", "1.0"),
		.initPackage = logpool_initPackage,
		.setupPackage = logpool_setupPackage,
		.initNameSpace = logpool_initNameSpace,
		.setupNameSpace = logpool_setupNameSpace,
	};
	return &d;
}

