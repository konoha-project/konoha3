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

#define USE_STRINGLIB 1

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/klib.h>
#include <minikonoha/float.h>
#include <jansson.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CT_Json          cJson
#define TY_Json          cJson->typeId
#define IS_Json(O)       ((O)->h.ct == CT_Json)
#define CT_JsonArray     CT_p0(kctx, CT_Array, TY_Json)

#define CT_StringArray0   CT_p0(kctx, CT_Array, TY_String)

typedef const struct _kJson kJson;
struct _kJson {
	KonohaObjectHeader h;
	json_t *obj;
};

/* ------------------------------------------------------------------------ */

static void Jansson_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct _kJson *json = (struct _kJson *)o;
	json->obj = NULL;
}

static void Jansson_free(KonohaContext *kctx, kObject *o)
{
	struct _kJson *json = (struct _kJson *)o;
	if(json->obj != NULL) {
		json_decref(json->obj);
		json->obj = NULL;
	}
}

static void Jansson_p(KonohaContext *kctx, KonohaValue *v, int pos, KUtilsWriteBuffer *wb)
{
	struct _kJson *json = (struct _kJson *)v[pos].asObject;
	char* data = json_dumps(json->obj, JSON_ENSURE_ASCII);
	KLIB Kwb_printf(kctx, wb, "%s", data);
	free(data);
}

/* ------------------------------------------------------------------------ */
/* [API methodList] */

#define CHECK_JSON(obj, ret_stmt) do {\
		if (!json_is_object(obj)) {\
			DBG_P("[ERROR]: Object is not Json object.");\
			/*KLIB KonohaRuntime_raise(kctx, 1, sfp, pline, msg);*/\
			ret_stmt;\
		}\
	} while(0);

//## Json Json.new();
static KMETHOD Json_new (KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kJson* json = (struct _kJson*)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].asObject), 0);
	json->obj = json_object();
	json_incref(json->obj);
	RETURN_(json);
}

//## @Static Json Json.parse(String str);
static KMETHOD Json_parse(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *buf = S_text(sfp[1].asString);
	json_t* obj;
	json_error_t err;
	obj = json_loads(buf, 0, &err);
	struct _kJson *ret = (struct _kJson*)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].asObject), 0);
	CHECK_JSON(obj, RETURN_((kJson*)KLIB Knull(kctx, O_ct(ret))));
	obj = json_incref(obj);
	ret->obj = obj;
	RETURN_(ret);
}

//## Json Json.getJson(String key);
static KMETHOD Json_getJson(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct _kJson*)sfp[0].asObject)->obj;
	CHECK_JSON(obj, RETURN_((kJson*)KLIB Knull(kctx, O_ct(sfp[0].asObject))));
	const char *key = S_text(sfp[1].asString);
	json_t* ret = json_object_get(obj, key);
	CHECK_JSON(ret, RETURN_((kJson*)KLIB Knull(kctx, O_ct(sfp[0].asObject))));
	ret = json_incref(ret);
	struct _kJson *json = (struct _kJson*)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].asObject), 0);
	json->obj = ret;
	RETURN_(json);
}

//## Array Json.getArray();
static KMETHOD Json_getArray(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct _kJson*)sfp[0].asObject)->obj;
	CHECK_JSON(obj, RETURN_(KNULL(Array)));
	const char *key = S_text(sfp[1].asString);
	json_t* ja;
	if (key == NULL) {
		ja = ((struct _kJson*)sfp[0].asObject)->obj;
		ja = json_incref(ja);
	}
	else {
		ja = json_object_get(obj, key);
		ja = json_incref(ja);
	}
	if (!json_is_array(ja)) {
		RETURN_(KNULL(Array));
	}
	kArrayVar* a = (kArrayVar*)KLIB new_kObject(kctx, CT_Array, 0);
	a->objectItems= (kObject**)ja;
	RETURN_(a);
}

//## Boolean Json.getBool(String key);
static KMETHOD Json_getBool(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct _kJson*)sfp[0].asObject)->obj;
	CHECK_JSON(obj, RETURNb_(false));
	const char *key = S_text(sfp[1].asString);
	json_t* json = json_object_get(obj, key);
	kbool_t ret = false;
	if (json_is_true(json)) {
		ret = true;
	}
	RETURNb_(ret);
}

//## float Json.getFloat(String key);
static KMETHOD Json_getFloat(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct _kJson*)sfp[0].asObject)->obj;
	CHECK_JSON(obj, RETURNf_(0.0));
	const char *key = S_text(sfp[1].asString);
	json_t* ret = json_object_get(obj, key);
	if (!json_is_real(ret)) {
		RETURNf_(0.0);
	}
	ret = json_incref(ret);
	double val = json_real_value(ret);
	RETURNf_(val);
}

//## int Json.getInt(String key);
static KMETHOD Json_getInt(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct _kJson*)sfp[0].asObject)->obj;
	CHECK_JSON(obj, RETURNi_(0));
	const char *key = S_text(sfp[1].asString);
	json_t* ret = json_object_get(obj, key);
	if (!json_is_integer(ret)) {
		RETURNi_(0);
	}
	json_int_t val = json_integer_value(ret);
	RETURNi_((kint_t)val);
}

//## String Json.getString(String key);
static KMETHOD Json_getString(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct _kJson*)sfp[0].asObject)->obj;
	CHECK_JSON(obj, RETURN_(KNULL(String)));
	const char *key = S_text(sfp[1].asString);
	json_t* ret = json_object_get(obj, key);
	if (!json_is_string(ret)) {
		RETURN_(KNULL(String));
	}
	ret = json_incref(ret);
	const char* str = json_string_value(ret);
	if (str == NULL) {
		RETURN_(KNULL(String));
	}
	RETURN_(KLIB new_kString(kctx, str, strlen(str), 0));
}

//## void Json.setJson(String key, Json value);
static KMETHOD Json_setJson(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct _kJson*)sfp[0].asObject)->obj;
	CHECK_JSON(obj, RETURN_DefaultObjectValue());
	const char *key = S_text(sfp[1].asString);
	json_t* val = ((struct _kJson*)sfp[2].asObject)->obj;
	CHECK_JSON(val, RETURN_DefaultObjectValue());
	int ret = json_object_set(obj, key, val);
	if (ret < 0) {
		DBG_P("[WARNING] Json set cannnot set target object");
		RETURN_DefaultObjectValue();
	}
	RETURNvoid_();
}

//## void Json.setArray(String key, Json[] a);
static KMETHOD Json_setArray(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct _kJson*)sfp[0].asObject)->obj;
	CHECK_JSON(obj, RETURN_DefaultObjectValue());
	const char *key = S_text(sfp[1].asString);
	kArrayVar* a = (kArrayVar*)sfp[2].asArray;
	json_t *ja = (json_t*)a->objectItems;
	json_object_set(obj, key, ja);
	RETURNvoid_();
}

//## void Json.setBool(String key, String value);
static KMETHOD Json_setBool(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct _kJson*)sfp[0].asObject)->obj;
	CHECK_JSON(obj, RETURN_DefaultObjectValue());
	const char *key = S_text(sfp[1].asString);
	kbool_t bval = sfp[2].boolValue;
	json_t* val;
	if (bval) {
		val = json_true();
	}
	else {
		val = json_false();
	}
	int ret = json_object_set(obj, key, val);
	if (ret < 0) {
		DBG_P("[WARNING] Json set cannnot set target object");
		RETURN_DefaultObjectValue();
	}
	RETURNvoid_();
}

//## void Json.setFloat(String key, String value);
static KMETHOD Json_setFloat(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct _kJson*)sfp[0].asObject)->obj;
	CHECK_JSON(obj, RETURN_DefaultObjectValue());
	const char *key = S_text(sfp[1].asString);
	float fval = sfp[2].floatValue;
	json_t* val = json_real(fval);
	if (!json_is_real(val)) {
		DBG_P("[ERROR]: Value is not Json object.");
		//KLIB KonohaRuntime_raise(kctx, 1, sfp, pline, msg);
		RETURN_DefaultObjectValue();
	}
	int ret = json_object_set(obj, key, val);
	if (ret < 0) {
		DBG_P("[WARNING] Json set cannnot set target object");
		RETURN_DefaultObjectValue();
	}
	RETURNvoid_();
}

//## void Json.setInt(String key, int value);
static KMETHOD Json_setInt(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct _kJson*)sfp[0].asObject)->obj;
	CHECK_JSON(obj, RETURN_DefaultObjectValue());
	const char *key = S_text(sfp[1].asString);
	kint_t ival = sfp[2].intValue;
	json_t* val = json_integer((json_int_t)ival);
	if (!json_is_integer(val)) {
		DBG_P("[ERROR]: Value is not Json object.");
		//KLIB KonohaRuntime_raise(kctx, 1, sfp, pline, msg);
		RETURN_DefaultObjectValue();
	}
	int ret = json_object_set(obj, key, val);
	if (ret < 0) {
		DBG_P("[WARNING] Json set cannnot set target object");
		RETURN_DefaultObjectValue();
	}
	RETURNvoid_();
}

//## void Json.setString(String key, String value);
static KMETHOD Json_setString(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct _kJson*)sfp[0].asObject)->obj;
	CHECK_JSON(obj, RETURN_DefaultObjectValue());
	const char *key = S_text(sfp[1].asString);
	const char *stringValue = S_text(sfp[2].s);
	json_t* val = json_string(stringValue);
	if (!json_is_string(val)) {
		DBG_P("[ERROR]: Value is not Json object.");
		//KLIB KonohaRuntime_raise(kctx, 1, sfp, pline, msg);
		RETURN_DefaultObjectValue();
	}
	int ret = json_object_set(obj, key, val);
	if (ret < 0) {
		DBG_P("[WARNING] Json set cannnot set target object");
		RETURN_DefaultObjectValue();
	}
	RETURNvoid_();
}

//## String[] Json.getKeys();
static KMETHOD Json_getKeys(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct _kJson*)sfp[0].asObject)->obj;
	kArray *a = (kArray*)KLIB new_kObject(kctx, CT_StringArray0, 0);
	CHECK_JSON(obj, RETURN_(KNULL(Array)));
	const char* key;
	void* iter = json_object_iter(obj);
	while (iter) {
		key = json_object_iter_key(iter);
		iter = json_object_iter_next(obj, iter);
		KLIB kArray_add(kctx, a, KLIB new_kString(kctx, key, strlen(key), SPOL_POOL|SPOL_ASCII));
	}
	RETURN_(a);
}

//## String Json.dump();
static KMETHOD Json_dump(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct _kJson*)sfp[0].asObject)->obj;
	CHECK_JSON(obj, RETURN_DefaultObjectValue());
	char* data = json_dumps(obj, JSON_ENSURE_ASCII);
	if (data == NULL) {
		RETURN_(KNULL(String));
	}
	RETURN_(KLIB new_kString(kctx, data, strlen(data), 0));
}

/* ------------------------------------------------------------------------ */

static KMETHOD JsonArray_newArray(KonohaContext *kctx, KonohaStack *sfp)
{
	kArrayVar *a = (kArrayVar *)sfp[0].asObject;
	size_t asize = (size_t)sfp[1].intValue;
	a->bytemax = asize * sizeof(void*);
	kArray_setsize((kArray*)a, asize);
	//a->list = (kObject**)KCALLOC(a->bytemax, 1);
	a->objectItems = (kObject**)json_array();
	RETURN_(a);
}

//## void Json[].add(Json json);
static KMETHOD JsonArray_add(KonohaContext *kctx, KonohaStack *sfp)
{
	kArrayVar *a = (kArrayVar *)sfp[0].asObject;
	json_t* ja = (json_t*)a->objectItems;
	if (!json_is_array(ja)) {
		DBG_P("[ERROR]: Object is not Json Array.");
		//KLIB KonohaRuntime_raise(kctx, 1, sfp, pline, msg);
		RETURN_DefaultObjectValue();
	}
	struct _kJson *json = (struct _kJson*)sfp[1].asObject;
	json_array_append(ja, json->obj);
	json_incref(json->obj);
	RETURNvoid_();
}

//## int Json[].getSize();
static KMETHOD JsonArray_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	const json_t *ja = (json_t*)a->objectItems;
	RETURNi_(json_array_size(ja));
}

//## Json Json[].get(int idx);
static KMETHOD JsonArray_get(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	json_t *ja = (json_t*)a->objectItems;
	struct _kJson *json = (struct _kJson*)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].asObject), 0);
	json->obj = json_array_get(ja, sfp[1].intValue);
	RETURN_(json);
}

////## void Json[].set(Json json);
//static KMETHOD Json_set(KonohaContext *kctx, KonohaStack *sfp)
//{
//}

//## void Json[].insert(Json json);
//static KMETHOD JsonArray_insert(KonohaContext *kctx, KonohaStack *sfp)
//{
//}

//## void Json[].delete(Json json);
//static KMETHOD JsonArray_delete(KonohaContext *kctx, KonohaStack *sfp)
//{
//}

/* ------------------------------------------------------------------------ */

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _Static   kMethod_Static
#define _Im kMethod_Immutable
#define _F(F)   (intptr_t)(F)

#define _KVi(T)  #T, TY_int, T

static kbool_t jansson_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KRequirePackage("konoha.float", pline);
	KDEFINE_CLASS JsonDef = {
		.structname = "Json",
		.typeId = TY_newid,
		.cflag = kClass_Final,
		.init = Jansson_init,
		.free = Jansson_free,
		.p    = Jansson_p,
	};
	KonohaClass *cJson = KLIB kNameSpace_defineClass(kctx, ns, NULL, &JsonDef, pline);
	ktype_t TY_JsonArray = CT_JsonArray->typeId;

	ktype_t TY_StringArray0 = CT_StringArray0->typeId;

	int FN_k = FN_("key");
	int FN_v = FN_("value");

	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(Json_dump),      TY_String,    TY_Json, MN_("dump"),      0,
		_Public|_Const|_Im, _F(Json_getJson),   TY_Json,      TY_Json, MN_("getJson"),   1, TY_String, FN_k,
		_Public|_Const|_Im, _F(Json_getArray),  TY_JsonArray, TY_Json, MN_("getArray"),  1, TY_String, FN_k,
		_Public|_Const|_Im, _F(Json_getBool),   TY_boolean,   TY_Json, MN_("getBool"),   1, TY_String, FN_k,
		_Public|_Const|_Im, _F(Json_getFloat),  TY_float,     TY_Json, MN_("getFloat"),  1, TY_String, FN_k,
		_Public|_Const|_Im, _F(Json_getInt),    TY_int,       TY_Json, MN_("getInt"),    1, TY_String, FN_k,
		_Public|_Const|_Im, _F(Json_getString), TY_String,    TY_Json, MN_("getString"), 1, TY_String, FN_k,
		_Public,            _F(Json_new),       TY_Json,      TY_Json, MN_("new"),       0,
		_Public|_Static|_Const|_Im, _F(Json_parse), TY_Json,  TY_Json, MN_("parse"),     1, TY_String, FN_v,
		_Public,            _F(Json_setJson),   TY_void,      TY_Json, MN_("setJson"),   2, TY_String, FN_k, TY_Json, FN_v,
		_Public,            _F(Json_setArray),  TY_void,      TY_Json, MN_("setArray"),  2, TY_String, FN_k, TY_JsonArray, FN_v,
		_Public,            _F(Json_setBool),   TY_void,      TY_Json, MN_("setBool"),   2, TY_String, FN_k, TY_boolean, FN_v,
		_Public,            _F(Json_setFloat),  TY_void,      TY_Json, MN_("setFloat"),  2, TY_String, FN_k, TY_float, FN_v,
		_Public,            _F(Json_setInt),    TY_void,      TY_Json, MN_("setInt"),    2, TY_String, FN_k, TY_int, FN_v,
		_Public,            _F(Json_setString), TY_void,      TY_Json, MN_("setString"), 2, TY_String, FN_k, TY_String, FN_v,
		_Public|_Const|_Im, _F(Json_getKeys),   TY_StringArray0, TY_Json, MN_("getKeys"), 0,

		_Public|_Const|_Im, _F(Json_dump),      TY_String,    TY_JsonArray, MN_("dump"), 0,
		_Public,            _F(JsonArray_newArray), TY_JsonArray,      TY_JsonArray, MN_("newArray"), 1, TY_int, FN_("size"),
		_Public|_Const|_Im, _F(JsonArray_get),  TY_Json,      TY_JsonArray, MN_("get"),  1, TY_int, FN_("idx"),
		_Public,            _F(JsonArray_add),  TY_void,      TY_JsonArray, MN_("add"),  1, TY_Json, FN_v,
		_Public|_Const|_Im, _F(JsonArray_getSize), TY_int,    TY_JsonArray, MN_("getSize"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t jansson_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t jansson_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t jansson_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* jansson_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("jansson", "1.0"),
		.initPackage    = jansson_initPackage,
		.setupPackage   = jansson_setupPackage,
		.initNameSpace  = jansson_initNameSpace,
		.setupNameSpace = jansson_setupNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
