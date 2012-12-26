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
#include <minikonoha/klib.h>
#include <minikonoha/konoha_common.h>
#include <minikonoha/import/methoddecl.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define KType_Json          cJson->typeId
//#define KClass_JsonArray     KClass_p0(kctx, KClass_Array, KType_Json)
//#define KClass_StringArray0   KClass_p0(kctx, KClass_Array, KType_String)
struct JsonBuf {
	uint64_t json_i;
};

struct kJsonVar {
	kObjectHeader h;
	struct JsonBuf jsonbuf;
};

typedef struct kJsonVar kJson;
/* ------------------------------------------------------------------------ */

static void kJson_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kJson *jo = (kJson *)o;
	PLATAPI CreateJson(kctx, &jo->jsonbuf, KJSON_OBJECT);
}

static void kJson_Free(KonohaContext *kctx, kObject *o)
{
	kJson *jo = (kJson *)o;
	PLATAPI FreeJson(kctx, &jo->jsonbuf);
}


static void kJson_p(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
//	kJson *jo = (kJson *)v[pos].asObject;
//	if(PLATAPI IsJsonType(&jo->jsonbuf, KJSON_STRING)) {
//		const char *text = PLATAPI GetJsonText(kctx, &jo->jsonbuf, NULL, 0, NULL);
//		DBG_ASSERT(text != NULL);
//		KReturn(KLIB new_kString(kctx, OnStack, text, strlen(text), 0));
//	}
//	else if(PLATAPI IsJsonType(&jo->jsonbuf, KJSON_NULL)) {
//		KReturn(KNULL(String));
//	}
//	char* data = json_dumps(json->obj, JSON_ENSURE_ASCII);
//	KLIB KBuffer_printf(kctx, wb, "%s", data);
//	free(data);
}

/* ------------------------------------------------------------------------ */
/* [API methodList] */

//## Json Json.new();
static KMETHOD Json_new (KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo = (kJson *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	PLATAPI CreateJson(kctx, &jo->jsonbuf, KJSON_OBJECT);
	KReturn(jo);
}

//## @Static Json Json.parse(String str);
static KMETHOD Json_Parse(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo = (kJson *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	KMakeTrace(trace, sfp);
	if(PLATAPI ParseJson(kctx, &jo->jsonbuf, kString_text(sfp[1].asString), kString_size(sfp[1].asString), trace)) {
		KReturn(jo);
	}
}

//## String Json.toInt();
static KMETHOD Json_toInt(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo = (kJson *)sfp[0].asObject;
	if(PLATAPI IsJsonType(&jo->jsonbuf, KJSON_INT)) {
		int64_t n = PLATAPI GetJsonInt(kctx, &jo->jsonbuf, NULL, 0, 0);
		KReturnUnboxValue(n);
	}
	else if(PLATAPI IsJsonType(&jo->jsonbuf, KJSON_DOUBLE)) {
		double n = PLATAPI GetJsonInt(kctx, &jo->jsonbuf, NULL, 0, 0);
		KReturnUnboxValue((intptr_t)n);
	}
	else if(PLATAPI IsJsonType(&jo->jsonbuf, KJSON_STRING)) {
		const char *text = PLATAPI GetJsonText(kctx, &jo->jsonbuf, NULL, 0, "0");
		int64_t n = strtoll(text, NULL, 10);
		KReturnUnboxValue((intptr_t)n);
	}
	else if(PLATAPI IsJsonType(&jo->jsonbuf, KJSON_BOOLEAN)) {
		int n = PLATAPI GetJsonBoolean(kctx, &jo->jsonbuf, NULL, 0, 0);
		KReturnUnboxValue((intptr_t)n);
	}
	KReturnUnboxValue(0);
}

//## Json Int.toJson();
static KMETHOD Int_toJson(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo = (kJson *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	PLATAPI CreateJson(kctx, &jo->jsonbuf, KJSON_INT64, (int64_t)sfp[0].intValue);
	KReturn(jo);
}

//## String Json.toString();
static KMETHOD Json_toString(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo = (kJson *)sfp[0].asObject;
	if(PLATAPI IsJsonType(&jo->jsonbuf, KJSON_NULL)) {
		KReturn(KNULL(String));
	}
	else {
		const char *text = PLATAPI JsonToNewText(kctx, &jo->jsonbuf);
		DBG_ASSERT(text != NULL);
		kString *ret = KLIB new_kString(kctx, OnStack, text, strlen(text), 0);
		free((char *) text);
		KReturn(ret);
	}
}

//## Json String.toJson();
static KMETHOD String_toJson(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo = (kJson *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	KMakeTrace(trace, sfp);
	PLATAPI CreateJson(kctx, &jo->jsonbuf, KJSON_STRING, kString_text(sfp[0].asString));
	KReturn(jo);
}

//## float Json.toFloat();
static KMETHOD Json_toFloat(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo = (kJson *)sfp[0].asObject;
	double ret = PLATAPI GetJsonFloat(kctx, &jo->jsonbuf, NULL, 0, 0.0);
	KReturnFloatValue(ret);
}

//## Boolean Json.toBoolean();
static KMETHOD Json_toBoolean(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo = (kJson *)sfp[0].asObject;
	kbool_t ret = PLATAPI GetJsonBoolean(kctx, &jo->jsonbuf, NULL, 0, false);
	KReturnUnboxValue(ret);
}

//## boolean Json.hasKey(String key);
static KMETHOD Json_hasKey(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo = (kJson *)sfp[0].asObject;
	struct JsonBuf jsonbuf = {};
	kbool_t ret = PLATAPI RetrieveJsonKeyValue(kctx, &jo->jsonbuf, kString_text(sfp[1].asString), kString_size(sfp[1].asString), &jsonbuf);
	PLATAPI FreeJson(kctx, &jsonbuf);
	KReturnUnboxValue(ret);
}

//## int Json.getSize();
static KMETHOD Json_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo = (kJson *)sfp[0].asObject;
	int64_t ret = PLATAPI GetJsonSize(kctx, &jo->jsonbuf);
	KReturnUnboxValue(ret);
}

//## Json Json.get(String key);
static KMETHOD Json_getJson(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo = (kJson *)sfp[0].asObject;
	struct JsonBuf jsonbuf = {};
	kJson *jnew = (kJson *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	if(PLATAPI RetrieveJsonKeyValue(kctx, &jo->jsonbuf, kString_text(sfp[1].asString), kString_size(sfp[1].asString), &jsonbuf)) {
		memcpy(&jnew->jsonbuf, &jsonbuf, sizeof(struct JsonBuf));
	}
	KReturn(jnew);
}

//## Json Json.get(int index);
static KMETHOD Json_getJson_index(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo = (kJson *)sfp[0].asObject;
	struct JsonBuf jsonbuf = {};
	kJson *jnew = (kJson *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	if(PLATAPI RetrieveJsonArrayAt(kctx, &jo->jsonbuf, sfp[1].unboxValue, &jsonbuf)) {
		memcpy(&jnew->jsonbuf, &jsonbuf, sizeof(struct JsonBuf));
	}
	KReturn(jnew);
}


//## Boolean Json.getBoolean(String key);
static KMETHOD Json_getBoolean(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo = (kJson *)sfp[0].asObject;
	kbool_t ret = PLATAPI GetJsonBoolean(kctx, &jo->jsonbuf, kString_text(sfp[1].asString), kString_size(sfp[1].asString), false);
	KReturnUnboxValue(ret);
}

//## float Json.getFloat(String key);
static KMETHOD Json_getFloat(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo = (kJson *)sfp[0].asObject;
	double ret = PLATAPI GetJsonFloat(kctx, &jo->jsonbuf, kString_text(sfp[1].asString), kString_size(sfp[1].asString), 0.0);
	KReturnFloatValue(ret);
}

//## int Json.getInt(String key);
static KMETHOD Json_getInt(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo = (kJson *)sfp[0].asObject;
	int64_t ret = PLATAPI GetJsonInt(kctx, &jo->jsonbuf, kString_text(sfp[1].asString), kString_size(sfp[1].asString), 0);
	KReturnUnboxValue(ret);
}

//## String Json.getString(String key, String defval);
static KMETHOD Json_getString(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo = (kJson *)sfp[0].asObject;
	const char *text = PLATAPI GetJsonText(kctx, &jo->jsonbuf, kString_text(sfp[1].asString), kString_size(sfp[1].asString), 0);
	if(text != NULL){
		KReturn(KLIB new_kString(kctx, OnStack, text, strlen(text), 0));
	}
	KReturn(KNULL(String));
}

//## void Json.setString(String key, String val);
static KMETHOD Json_setString(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo = (kJson *)sfp[0].asObject;
	if(!PLATAPI SetJsonValue(kctx, &jo->jsonbuf, kString_text(sfp[1].asString), kString_size(sfp[1].asString), KJSON_STRING, kString_text(sfp[2].asString))) {
		DBG_P("[WARNING] Json cannot set target object");
	}
	KReturnVoid();
}

//## void Json.setJson(String key, Json value);
static KMETHOD Json_setJson(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo  = (kJson *)sfp[0].asObject;
	kJson *val = (kJson *)sfp[2].asObject;
	if(!PLATAPI SetJsonKeyValue(kctx, &jo->jsonbuf, kString_text(sfp[1].asString), kString_size(sfp[1].asString), &val->jsonbuf)) {
		DBG_P("[WARNING] Json cannot set target object");
	}
	KReturnVoid();
}

//## void Json.setJson(int key, Json value);
static KMETHOD Json_setJson_index(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo  = (kJson *)sfp[0].asObject;
	kJson *val = (kJson *)sfp[2].asObject;
	if(!PLATAPI SetJsonArrayAt(kctx, &jo->jsonbuf, sfp[1].unboxValue, &val->jsonbuf)) {
		DBG_P("[WARNING] Json cannot set target object");
	}
	KReturnVoid();
}

//## void Json.add(Json value);
static KMETHOD Json_AddJson(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo  = (kJson *)sfp[0].asObject;
	kJson *val = (kJson *)sfp[1].asObject;
	if(!PLATAPI AppendJsonArray(kctx, &jo->jsonbuf, &val->jsonbuf)) {
		DBG_P("[WARNING] Json cannot set target object");
	}
	KReturnVoid();
}

//## void Json.setBoolean(String key, boolean value);
static KMETHOD Json_setBoolean(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo = (kJson *)sfp[0].asObject;
	if(!PLATAPI SetJsonValue(kctx, &jo->jsonbuf, kString_text(sfp[1].asString), kString_size(sfp[1].asString), KJSON_BOOLEAN, sfp[2].unboxValue)) {
		DBG_P("[WARNING] Json cannot set target object");
	}
	KReturnVoid();
}

//## void Json.setFloat(String key, Float value);
static KMETHOD Json_setFloat(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo = (kJson *)sfp[0].asObject;
	if(!PLATAPI SetJsonValue(kctx, &jo->jsonbuf, kString_text(sfp[1].asString), kString_size(sfp[1].asString), KJSON_DOUBLE, sfp[2].floatValue)) {
		DBG_P("[WARNING] Json cannot set target object");
	}
	KReturnVoid();
}

//## void Json.setInt(String key, int value);
static KMETHOD Json_setInt(KonohaContext *kctx, KonohaStack *sfp)
{
	kJson *jo = (kJson *)sfp[0].asObject;
	if(!PLATAPI SetJsonValue(kctx, &jo->jsonbuf, kString_text(sfp[1].asString), kString_size(sfp[1].asString), KJSON_INT, sfp[2].unboxValue)) {
		DBG_P("[WARNING] Json cannot set target object");
	}
	KReturnVoid();
}

/* ------------------------------------------------------------------------ */

#define KType_Json   (cJson->typeId)

static kbool_t json_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequirePackage("konoha.float", trace);
	KDEFINE_CLASS JsonDef = {
		.structname = "Json",
		.typeId = KTypeAttr_NewId,
		.cflag = KClassFlag_Final,
		.init = kJson_Init,
		.free = kJson_Free,
		.p    = kJson_p,
	};
	KClass *cJson = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &JsonDef, trace);

	int FN_k = KFieldName_("key");
	int FN_v = KFieldName_("value");

	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(Int_toJson),     KType_Json,       KType_int , KMethodName_To(KType_Json),    0,
		_Public|_Const|_Im, _F(String_toJson),  KType_Json,     KType_String, KMethodName_To(KType_Json),    0,
		_Public|_Const|_Im, _F(Json_toInt),     KType_int,        KType_Json, KMethodName_To(KType_int),     0,
		_Public|_Const|_Im, _F(Json_toString),  KType_String,     KType_Json, KMethodName_To(KType_String),  0,
		_Public|_Const|_Im, _F(Json_toFloat),   KType_String,     KType_Json, KMethodName_To(KType_float),   0,
		_Public|_Const|_Im, _F(Json_toBoolean), KType_String,     KType_Json, KMethodName_To(KType_boolean), 0,
		_Public|_Const|_Im, _F(Json_getSize),   KType_int,        KType_Json, KKMethodName_("getSize"),    0,
		_Public|_Const|_Im, _F(Json_hasKey),    KType_boolean,    KType_Json, KKMethodName_("hasKey"),     1, KType_String, FN_k,
		_Public|_Const|_Im, _F(Json_getJson),   KType_Json,       KType_Json, KKMethodName_("get"),        1, KType_String, FN_k,
		_Public|_Const|_Im, _F(Json_getJson_index),KType_Json,    KType_Json, KKMethodName_("get"),        1, KType_int, FN_k,
		_Public|_Const|_Im, _F(Json_getBoolean),   KType_boolean, KType_Json, KKMethodName_("getBoolean"), 1, KType_String, FN_k,
		_Public|_Const|_Im, _F(Json_getFloat),  KType_float,      KType_Json, KKMethodName_("getFloat"),   1, KType_String, FN_k,
		_Public|_Const|_Im, _F(Json_getInt),    KType_int,        KType_Json, KKMethodName_("getInt"),     1, KType_String, FN_k,
		_Public|_Const|_Im, _F(Json_getString), KType_String,     KType_Json, KKMethodName_("getString"),  1, KType_String, FN_k,
		_Public,            _F(Json_new),       KType_Json,       KType_Json, KKMethodName_("new"),        0,
		_Public|_Static|_Im, _F(Json_Parse), KType_Json,   KType_Json, KKMethodName_("parse"),      1, KType_String, FN_v,
		_Public,            _F(Json_setJson),   KType_void,       KType_Json, KKMethodName_("set"),        2, KType_String, FN_k, KType_Json, FN_v,
		_Public,            _F(Json_setJson_index),   KType_void,       KType_Json, KKMethodName_("set"),        2, KType_int,    FN_k, KType_Json, FN_v,
		_Public,            _F(Json_AddJson),   KType_void,       KType_Json, KKMethodName_("add"),        1, KType_Json,   FN_v,
		_Public,            _F(Json_setBoolean),   KType_void,       KType_Json, KKMethodName_("setBoolean"),    2, KType_String, FN_k, KType_boolean, FN_v,
		_Public,            _F(Json_setFloat),  KType_void,       KType_Json, KKMethodName_("setFloat"),   2, KType_String, FN_k, KType_float, FN_v,
		_Public,            _F(Json_setInt),    KType_void,       KType_Json, KKMethodName_("setInt"),     2, KType_String, FN_k, KType_int, FN_v,
		_Public,            _F(Json_setString), KType_void,       KType_Json, KKMethodName_("setString"),  2, KType_String, FN_k, KType_String, FN_v,

		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

static kbool_t json_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *json_Init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("json", "1.0"),
		.PackupNameSpace    = json_PackupNameSpace,
		.ExportNameSpace   = json_ExportNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
