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
#include <minikonoha/konoha_common.h>
#include <jansson.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KType_Json          cJson->typeId
#define KClass_JsonArray     KClass_p0(kctx, KClass_Array, KType_Json)
#define KClass_StringArray0   KClass_p0(kctx, KClass_Array, KType_String)

typedef struct kJanssonVar kJson;
struct kJanssonVar {
	kObjectHeader h;
	json_t *obj;
};

/* ------------------------------------------------------------------------ */

static void Jansson_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct kJanssonVar *json = (struct kJanssonVar *)o;
	json->obj = NULL;
}

static void Jansson_Free(KonohaContext *kctx, kObject *o)
{
	struct kJanssonVar *json = (struct kJanssonVar *)o;
	if(json->obj != NULL) {
		json_decref(json->obj);
		json->obj = NULL;
	}
}

static void Jansson_p(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	struct kJanssonVar *json = (struct kJanssonVar *)v[pos].asObject;
	char* data = json_dumps(json->obj, JSON_ENSURE_ASCII);
	KLIB KBuffer_printf(kctx, wb, "%s", data);
	free(data);
}

/* ------------------------------------------------------------------------ */
/* [API methodList] */

#define CHECK_JSON(obj, ret_stmt) do {\
		if(!json_is_object(obj)) {\
			DBG_P("[ERROR]: Object is not Json object.");\
			/*KLIB KRuntime_raise(kctx, 1, sfp, pline, msg);*/\
			ret_stmt;\
		}\
	} while(0);

//## Json Json.new();
static KMETHOD Json_new (KonohaContext *kctx, KonohaStack *sfp)
{
	struct kJanssonVar* json = (struct kJanssonVar *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	json->obj = json_object();
	json_incref(json->obj);
	KReturn(json);
}

//## @Static Json Json.parse(String str);
static KMETHOD Json_Parse(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *buf = kString_text(sfp[1].asString);
	json_t* obj;
	json_error_t err;
	obj = json_loads(buf, 0, &err);
	struct kJanssonVar *ret = (struct kJanssonVar *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	CHECK_JSON(obj, KReturn((kJson *)KLIB Knull(kctx, kObject_class(ret))));
	obj = json_incref(obj);
	ret->obj = obj;
	KReturn(ret);
}

//## Json Json.getJson(String key);
static KMETHOD Json_getJson(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct kJanssonVar *)sfp[0].asObject)->obj;
	CHECK_JSON(obj, KReturn((kJson *)KLIB Knull(kctx, kObject_class(sfp[0].asObject))));
	const char *key = kString_text(sfp[1].asString);
	json_t* ret = json_object_get(obj, key);
	CHECK_JSON(ret, KReturn((kJson *)KLIB Knull(kctx, kObject_class(sfp[0].asObject))));
	ret = json_incref(ret);
	struct kJanssonVar *json = (struct kJanssonVar *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	json->obj = ret;
	KReturn(json);
}

//## Array Json.getArray();
static KMETHOD Json_getArray(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct kJanssonVar *)sfp[0].asObject)->obj;
	CHECK_JSON(obj, KReturn(KNULL(Array)));
	const char *key = kString_text(sfp[1].asString);
	json_t* ja;
	if(key == NULL) {
		ja = ((struct kJanssonVar *)sfp[0].asObject)->obj;
		ja = json_incref(ja);
	}
	else {
		ja = json_object_get(obj, key);
		ja = json_incref(ja);
	}
	if(!json_is_array(ja)) {
		KReturn(KNULL(Array));
	}
	kArrayVar* a = (kArrayVar *)KLIB new_kObject(kctx, OnStack, KClass_Array, 0);
	a->ObjectItems= (kObject**)ja;
	KReturn(a);
}

//## Boolean Json.getBool(String key);
static KMETHOD Json_getBool(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct kJanssonVar *)sfp[0].asObject)->obj;
	CHECK_JSON(obj, KReturnUnboxValue(false));
	const char *key = kString_text(sfp[1].asString);
	json_t* json = json_object_get(obj, key);
	kbool_t ret = false;
	if(json_is_true(json)) {
		ret = true;
	}
	KReturnUnboxValue(ret);
}

//## float Json.getFloat(String key);
static KMETHOD Json_getFloat(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct kJanssonVar *)sfp[0].asObject)->obj;
	CHECK_JSON(obj, KReturnFloatValue(0.0));
	const char *key = kString_text(sfp[1].asString);
	json_t* ret = json_object_get(obj, key);
	if(!json_is_real(ret)) {
		KReturnFloatValue(0.0);
	}
	ret = json_incref(ret);
	double val = json_real_value(ret);
	KReturnFloatValue(val);
}

//## int Json.getInt(String key);
static KMETHOD Json_getInt(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct kJanssonVar *)sfp[0].asObject)->obj;
	CHECK_JSON(obj, KReturnUnboxValue(0));
	const char *key = kString_text(sfp[1].asString);
	json_t* ret = json_object_get(obj, key);
	if(!json_is_integer(ret)) {
		KReturnUnboxValue(0);
	}
	json_int_t val = json_integer_value(ret);
	KReturnUnboxValue((kint_t)val);
}

//## String Json.getString(String key);
static KMETHOD Json_getString(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct kJanssonVar *)sfp[0].asObject)->obj;
	CHECK_JSON(obj, KReturn(KNULL(String)));
	const char *key = kString_text(sfp[1].asString);
	json_t* ret = json_object_get(obj, key);
	if(!json_is_string(ret)) {
		KReturn(KNULL(String));
	}
	ret = json_incref(ret);
	const char* str = json_string_value(ret);
	if(str == NULL) {
		KReturn(KNULL(String));
	}
	KReturn(KLIB new_kString(kctx, GcUnsafe, str, strlen(str), 0));
}

//## void Json.setJson(String key, Json value);
static KMETHOD Json_setJson(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct kJanssonVar *)sfp[0].asObject)->obj;
	CHECK_JSON(obj, KReturnDefaultValue());
	const char *key = kString_text(sfp[1].asString);
	json_t* val = ((struct kJanssonVar *)sfp[2].asObject)->obj;
	CHECK_JSON(val, KReturnDefaultValue());
	int ret = json_object_set(obj, key, val);
	if(ret < 0) {
		DBG_P("[WARNING] Json set cannnot set target object");
		KReturnDefaultValue();
	}
	KReturnVoid();
}

//## void Json.setArray(String key, Json[] a);
static KMETHOD Json_setArray(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct kJanssonVar *)sfp[0].asObject)->obj;
	CHECK_JSON(obj, KReturnDefaultValue());
	const char *key = kString_text(sfp[1].asString);
	kArrayVar* a = (kArrayVar *)sfp[2].asArray;
	json_t *ja = (json_t *)a->ObjectItems;
	json_object_set(obj, key, ja);
	KReturnVoid();
}

//## void Json.setBool(String key, String value);
static KMETHOD Json_setBool(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct kJanssonVar *)sfp[0].asObject)->obj;
	CHECK_JSON(obj, KReturnDefaultValue());
	const char *key = kString_text(sfp[1].asString);
	kbool_t bval = sfp[2].boolValue;
	json_t* val;
	if(bval) {
		val = json_true();
	}
	else {
		val = json_false();
	}
	int ret = json_object_set(obj, key, val);
	if(ret < 0) {
		DBG_P("[WARNING] Json set cannnot set target object");
		KReturnDefaultValue();
	}
	KReturnVoid();
}

//## void Json.setFloat(String key, String value);
static KMETHOD Json_setFloat(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct kJanssonVar *)sfp[0].asObject)->obj;
	CHECK_JSON(obj, KReturnDefaultValue());
	const char *key = kString_text(sfp[1].asString);
	float fval = sfp[2].floatValue;
	json_t* val = json_real(fval);
	if(!json_is_real(val)) {
		DBG_P("[ERROR]: Value is not Json object.");
		//KLIB KRuntime_raise(kctx, 1, sfp, pline, msg);
		KReturnDefaultValue();
	}
	int ret = json_object_set(obj, key, val);
	if(ret < 0) {
		DBG_P("[WARNING] Json set cannnot set target object");
		KReturnDefaultValue();
	}
	KReturnVoid();
}

//## void Json.setInt(String key, int value);
static KMETHOD Json_setInt(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct kJanssonVar *)sfp[0].asObject)->obj;
	CHECK_JSON(obj, KReturnDefaultValue());
	const char *key = kString_text(sfp[1].asString);
	kint_t ival = sfp[2].intValue;
	json_t* val = json_integer((json_int_t)ival);
	if(!json_is_integer(val)) {
		DBG_P("[ERROR]: Value is not Json object.");
		//KLIB KRuntime_raise(kctx, 1, sfp, pline, msg);
		KReturnDefaultValue();
	}
	int ret = json_object_set(obj, key, val);
	if(ret < 0) {
		DBG_P("[WARNING] Json set cannnot set target object");
		KReturnDefaultValue();
	}
	KReturnVoid();
}

//## void Json.setString(String key, String value);
static KMETHOD Json_setString(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct kJanssonVar *)sfp[0].asObject)->obj;
	CHECK_JSON(obj, KReturnDefaultValue());
	const char *key = kString_text(sfp[1].asString);
	const char *stringValue = kString_text(sfp[2].asString);
	json_t* val = json_string(stringValue);
	if(!json_is_string(val)) {
		DBG_P("[ERROR]: Value is not Json object.");
		//KLIB KRuntime_raise(kctx, 1, sfp, pline, msg);
		KReturnDefaultValue();
	}
	int ret = json_object_set(obj, key, val);
	if(ret < 0) {
		DBG_P("[WARNING] Json set cannnot set target object");
		KReturnDefaultValue();
	}
	KReturnVoid();
}

//## String[] Json.getKeys();
static KMETHOD Json_getKeys(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct kJanssonVar *)sfp[0].asObject)->obj;
	kArray *a = (kArray *)KLIB new_kObject(kctx, OnStack, KClass_StringArray0, 0);
	CHECK_JSON(obj, KReturn(KNULL(Array)));
	const char* key;
	void* iter = json_object_iter(obj);
	while(iter) {
		key = json_object_iter_key(iter);
		iter = json_object_iter_next(obj, iter);
		KLIB kArray_Add(kctx, a, KLIB new_kString(kctx, GcUnsafe, key, strlen(key), 0));
	}
	KReturn(a);
}

//## String Json.dump();
static KMETHOD Json_dump(KonohaContext *kctx, KonohaStack *sfp)
{
	json_t* obj = ((struct kJanssonVar *)sfp[0].asObject)->obj;
	CHECK_JSON(obj, KReturnDefaultValue());
	char* data = json_dumps(obj, JSON_ENSURE_ASCII);
	if(data == NULL) {
		KReturn(KNULL(String));
	}
	KReturn(KLIB new_kString(kctx, GcUnsafe, data, strlen(data), 0));
}

/* ------------------------------------------------------------------------ */

static KMETHOD JsonArray_newArray(KonohaContext *kctx, KonohaStack *sfp)
{
	kArrayVar *a = (kArrayVar *)sfp[0].asObject;
	size_t asize = (size_t)sfp[1].intValue;
	a->bytemax = asize * sizeof(void *);
	kArray_SetSize((kArray *)a, asize);
	//a->list = (kObject**)KCalloc_UNTRACE(a->bytemax, 1);
	a->ObjectItems = (kObject**)json_array();
	KReturn(a);
}

//## void Json[].add(Json json);
static KMETHOD JsonArray_Add(KonohaContext *kctx, KonohaStack *sfp)
{
	kArrayVar *a = (kArrayVar *)sfp[0].asObject;
	json_t* ja = (json_t *)a->ObjectItems;
	if(!json_is_array(ja)) {
		DBG_P("[ERROR]: Object is not Json Array.");
		//KLIB KRuntime_raise(kctx, 1, sfp, pline, msg);
		KReturnDefaultValue();
	}
	struct kJanssonVar *json = (struct kJanssonVar *)sfp[1].asObject;
	json_array_append(ja, json->obj);
	json_incref(json->obj);
	KReturnVoid();
}

//## int Json[].getSize();
static KMETHOD JsonArray_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	const json_t *ja = (json_t *)a->ObjectItems;
	KReturnUnboxValue(json_array_size(ja));
}

//## Json Json[].get(int idx);
static KMETHOD JsonArray_get(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	json_t *ja = (json_t *)a->ObjectItems;
	struct kJanssonVar *json = (struct kJanssonVar *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	json->obj = json_array_get(ja, sfp[1].intValue);
	KReturn(json);
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
#define _Static   kMethod_Static
#define _Im kMethod_Immutable
#define _F(F)   (intptr_t)(F)

static kbool_t jansson_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequirePackage("konoha.float", trace);
	KDEFINE_CLASS JsonDef = {
		.structname = "Json",
		.typeId = TypeAttr_NewId,
		.cflag = KClassFlag_Final,
		.init = Jansson_Init,
		.free = Jansson_Free,
		.p    = Jansson_p,
	};
	KClass *cJson = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &JsonDef, trace);
	ktypeattr_t KType_JsonArray = KClass_JsonArray->typeId;
	ktypeattr_t KType_StringArray0 = KClass_StringArray0->typeId;

	int FN_k = KFieldName_("key");
	int FN_v = KFieldName_("value");

	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(Json_dump),      KType_String,    KType_Json, KMethodName_("dump"),      0,
		_Public|_Const|_Im, _F(Json_getJson),   KType_Json,      KType_Json, KMethodName_("getJson"),   1, KType_String, FN_k,
		_Public|_Const|_Im, _F(Json_getArray),  KType_JsonArray, KType_Json, KMethodName_("getArray"),  1, KType_String, FN_k,
		_Public|_Const|_Im, _F(Json_getBool),   KType_boolean,   KType_Json, KMethodName_("getBool"),   1, KType_String, FN_k,
		_Public|_Const|_Im, _F(Json_getFloat),  KType_float,     KType_Json, KMethodName_("getFloat"),  1, KType_String, FN_k,
		_Public|_Const|_Im, _F(Json_getInt),    KType_int,       KType_Json, KMethodName_("getInt"),    1, KType_String, FN_k,
		_Public|_Const|_Im, _F(Json_getString), KType_String,    KType_Json, KMethodName_("getString"), 1, KType_String, FN_k,
		_Public,            _F(Json_new),       KType_Json,      KType_Json, KMethodName_("new"),       0,
		_Public|_Static|_Const|_Im, _F(Json_Parse), KType_Json,  KType_Json, KMethodName_("parse"),     1, KType_String, FN_v,
		_Public,            _F(Json_setJson),   KType_void,      KType_Json, KMethodName_("setJson"),   2, KType_String, FN_k, KType_Json, FN_v,
		_Public,            _F(Json_setArray),  KType_void,      KType_Json, KMethodName_("setArray"),  2, KType_String, FN_k, KType_JsonArray, FN_v,
		_Public,            _F(Json_setBool),   KType_void,      KType_Json, KMethodName_("setBool"),   2, KType_String, FN_k, KType_boolean, FN_v,
		_Public,            _F(Json_setFloat),  KType_void,      KType_Json, KMethodName_("setFloat"),  2, KType_String, FN_k, KType_float, FN_v,
		_Public,            _F(Json_setInt),    KType_void,      KType_Json, KMethodName_("setInt"),    2, KType_String, FN_k, KType_int, FN_v,
		_Public,            _F(Json_setString), KType_void,      KType_Json, KMethodName_("setString"), 2, KType_String, FN_k, KType_String, FN_v,
		_Public|_Const|_Im, _F(Json_getKeys),   KType_StringArray0, KType_Json, KMethodName_("getKeys"), 0,

		_Public|_Const|_Im, _F(Json_dump),      KType_String,    KType_JsonArray, KMethodName_("dump"), 0,
		_Public,            _F(JsonArray_newArray), KType_JsonArray,      KType_JsonArray, KMethodName_("newArray"), 1, KType_int, KFieldName_("size"),
		_Public|_Const|_Im, _F(JsonArray_get),  KType_Json,      KType_JsonArray, KMethodName_("get"),  1, KType_int, KFieldName_("idx"),
		_Public,            _F(JsonArray_Add),  KType_void,      KType_JsonArray, KMethodName_("add"),  1, KType_Json, FN_v,
		_Public|_Const|_Im, _F(JsonArray_getSize), KType_int,    KType_JsonArray, KMethodName_("getSize"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

static kbool_t jansson_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* jansson_Init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("jansson", "1.0"),
		.PackupNameSpace    = jansson_PackupNameSpace,
		.ExportNameSpace   = jansson_ExportNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
