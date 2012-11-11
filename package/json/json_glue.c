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
#include <minikonoha/konoha_common.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kJson {
	KonohaObjectHeader h;
	KJson_t json;
} kJson;

/* ------------------------------------------------------------------------ */

static void kJson_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kJson *json = (kJson *)o;
	if(conf != NULL) {
		json->json = (KJson_t) conf;
	} else {
		json->json = 0;
	}
}

static void kJson_free(KonohaContext *kctx, kObject *o)
{
	kJson *json = (kJson *)o;
	if(json->json != 0) {
		PLATAPI DeleteJson(PLATAPI JsonContext, json->json);
	}
}

static void kJson_p(KonohaContext *kctx, KonohaValue *v, int pos, KGrowingBuffer *wb)
{
	kJson *json = (kJson *)v[pos].asObject;
	size_t length;
	const char *data = PLATAPI Json_toString_i(PLATAPI JsonContext, json->json, &length);
	KLIB Kwb_printf(kctx, wb, "%s", data);
	free((char *)data);
}

/* ------------------------------------------------------------------------ */
/* [API methodList] */

//## JSON JSON.new();
static KMETHOD kJson_new(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(sfp[0].asObject);
}

static kJson *NewJsonObject(KonohaContext *kctx, KonohaStack *sfp, KJson_t val)
{
	kJson *json = (kJson *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	json->json = val;
	return json;
}

//## @Static JSON JSON.parse(String str);
static KMETHOD kJson_parse(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *text = S_text(sfp[1].asString);
	KJson_t json = PLATAPI ParseJson(PLATAPI JsonContext, text, S_size(sfp[1].asString));
	if(json == 0) {
		KReturnDefaultValue();
	}
	KReturn(NewJsonObject(kctx, sfp, json));
}

//## JSON      JSON.get(String key);
//## JSONArray JSON.getArray(String key);
static KMETHOD kJson_get(KonohaContext *kctx, KonohaStack *sfp)
{
	KJson_t obj = ((kJson *)sfp[0].asObject)->json;
	kString *Key = sfp[1].asString;
	KJson_t json = PLATAPI JsonObject_get_i(PLATAPI JsonContext, obj, S_text(Key), S_size(Key));
	KReturn(NewJsonObject(kctx, sfp, json));
}

//## JSONBool JSON.getBool(String key);
static KMETHOD kJson_getBool(KonohaContext *kctx, KonohaStack *sfp)
{
	KJson_t obj = ((kJson *)sfp[0].asObject)->json;
	kString *Key = sfp[1].asString;
	kbool_t json = PLATAPI JsonObject_getBool_i(PLATAPI JsonContext, obj, S_text(Key), S_size(Key));
	KReturnUnboxValue(json);
}

//## JSONFloat JSON.getFloat(String key);
static KMETHOD kJson_getFloat(KonohaContext *kctx, KonohaStack *sfp)
{
	KJson_t obj = ((kJson *)sfp[0].asObject)->json;
	kString *Key = (sfp[1].asString);
	double json = PLATAPI JsonObject_getDouble_i(PLATAPI JsonContext, obj, S_text(Key), S_size(Key));
	KReturnFloatValue(json);
}

//## String JSON.getInt(String key);
static KMETHOD kJson_getInt(KonohaContext *kctx, KonohaStack *sfp)
{
	KJson_t obj = ((kJson *)sfp[0].asObject)->json;
	kString *Key = (sfp[1].asString);
	int32_t json = PLATAPI JsonObject_getInt_i(PLATAPI JsonContext, obj, S_text(Key), S_size(Key));
	KReturnUnboxValue(json);
}

//## String JSON.getString(String key);
static KMETHOD kJson_getString(KonohaContext *kctx, KonohaStack *sfp)
{
	KJson_t obj = ((kJson *)sfp[0].asObject)->json;
	kString *Key = (sfp[1].asString);
	size_t len = 0;
	const char *text = PLATAPI JsonObject_getString_i(PLATAPI JsonContext, obj, S_text(Key), S_size(Key), &len);
	KReturn(KLIB new_kString(kctx, OnStack, text, len, 0));
}

//## void JSON.set(String key, JSON value);
static KMETHOD kJson_set(KonohaContext *kctx, KonohaStack *sfp)
{
	KJson_t obj, val;
	kString *Key = (sfp[1].asString);
	obj = ((kJson *)sfp[0].asObject)->json;
	val = ((kJson *)sfp[2].asObject)->json;
	PLATAPI JsonObject_set_i(PLATAPI JsonContext, obj, S_text(Key), S_size(Key), val);
	KReturnVoid();
}

struct KonohaThunk {
	KonohaContext *kctx;
	kArray *ret;
};

static void Func(KJson_t Key, KJson_t Val, void *thunk)
{
	struct KonohaThunk *th = (struct KonohaThunk *) thunk;
	KonohaStack sfp[1];
	KonohaContext *kctx = th->kctx;
	kbool_t success = PLATAPI JsonToObject_i(PLATAPI JsonContext, kctx, Key, TY_String, sfp);
	if(success == true) {
		KLIB kArray_add(kctx, th->ret, sfp[0].asString);
	}
}

//## String[] JSON.getKeys();
static KMETHOD kJson_getKeys(KonohaContext *kctx, KonohaStack *sfp)
{
	KJson_t obj = ((kJson *)sfp[0].asObject)->json;
	kArray *ret = (kArray *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	struct KonohaThunk thunk = { kctx, ret };
	KUnsafeFieldSet(sfp[K_RTNIDX].asArray, ret);
	PLATAPI JsonObject_each_i(PLATAPI JsonContext, obj, Func, &thunk);
	KReturn(ret);
}

/* ------------------------------------------------------------------------ */

//## void JSONArray.append(JSON json);
static KMETHOD kJsonArray_append(KonohaContext *kctx, KonohaStack *sfp)
{
	KJson_t obj = ((kJson *)sfp[0].asObject)->json;
	KJson_t val = ((kJson *)sfp[1].asObject)->json;
	PLATAPI JsonArray_append_i(PLATAPI JsonContext, obj, val);
	KReturnVoid();
}

//## int JSON.getSize();
static KMETHOD kJsonArray_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	KJson_t obj = ((kJson *)sfp[0].asObject)->json;
	KReturnUnboxValue(PLATAPI Json_length_i(PLATAPI JsonContext, obj));
}

//## JSON JSONArray.get(int index);
static KMETHOD kJsonArray_get(KonohaContext *kctx, KonohaStack *sfp)
{
	KJson_t obj  = ((kJson *)sfp[0].asObject)->json;
	KJson_t json = PLATAPI JsonArray_get_i(PLATAPI JsonContext, obj, sfp[1].intValue);
	KReturn(NewJsonObject(kctx, sfp, json));
}

//## JSON JSON.toInt();
static KMETHOD Int_toJson(KonohaContext *kctx, KonohaStack *sfp)
{
	KJson_t json = PLATAPI JsonInt_new_i(PLATAPI JsonContext, sfp[0].intValue);
	KReturn(NewJsonObject(kctx, sfp, json));
}

//## JSON Float.toJSON();
static KMETHOD Float_toJson(KonohaContext *kctx, KonohaStack *sfp)
{
	KJson_t json = PLATAPI JsonDouble_new_i(PLATAPI JsonContext, sfp[0].floatValue);
	KReturn(NewJsonObject(kctx, sfp, json));
}

//## JSON String.toJSON();
static KMETHOD String_toJson(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *Val = sfp[0].asString;
	KJson_t json = PLATAPI ParseJson(PLATAPI JsonContext, S_text(Val), S_size(Val));
	if(json != 0) {
		KReturn(NewJsonObject(kctx, sfp, json));
	}
}

//## JSON Array.toJSON();
static KMETHOD Array_toJson(KonohaContext *kctx, KonohaStack *sfp)
{
	KGrowingBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	O_ct(sfp[0].asObject)->p(kctx, sfp, 0, &wb);
	const char *text = KLIB Kwb_top(kctx, &wb, 1);
	KJson_t json = PLATAPI ParseJson(PLATAPI JsonContext, text, Kwb_bytesize(&wb));
	if(json != 0) {
		KReturn(NewJsonObject(kctx, sfp, json));
	}
	KLIB Kwb_free(&wb);
}

//## JSON JSON.toString();
static KMETHOD Json_toString(KonohaContext *kctx, KonohaStack *sfp)
{
	KJson_t json = ((kJson *)sfp[0].asObject)->json;
	size_t length;
	const char *data = PLATAPI Json_toString_i(PLATAPI JsonContext, json, &length);
	kString *ret = KLIB new_kString(kctx, OnStack, data, length, 0);
	free((char *)data);
	KReturn(ret);
}


/* ------------------------------------------------------------------------ */
#define _Public  kMethod_Public
#define _Static  kMethod_Static
#define _Const   kMethod_Const
#define _Im      kMethod_Immutable
#define _F(F)   (intptr_t)(F)

#define TY_Json     cJson->typeId

static kbool_t JSON_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequireKonohaCommonModule(trace);
	KImportPackage(ns, "konoha.float", trace);
	KDEFINE_CLASS JsonDef = {
		.structname = "Json",
		.typeId = TY_newid,
		.cflag = kClass_Final,
		.init = kJson_init,
		.free = kJson_free,
		.p    = kJson_p,
	};
	KonohaClass *cJson = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &JsonDef, trace);
	KonohaClass *CT_JSONArray    = CT_p0(kctx, CT_Array, TY_Json);
	KonohaClass *CT_StringArray2 = CT_p0(kctx, CT_Array, TY_String);
	ktype_t TY_JsonArray = CT_JSONArray->typeId;
	ktype_t TY_StringArray = CT_StringArray2->typeId;

	int FN_key = FN_("key");
	int FN_v   = FN_("v");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Static, _F(kJson_parse), TY_Json,      TY_Json, MN_("parse"),     1, TY_String, FN_("data"),
		_Public|_Const|_Im, _F(kJson_get),       TY_Json,      TY_Json, MN_("get"),       1, TY_String, FN_key,
		_Public|_Const|_Im, _F(kJson_get),       TY_JsonArray, TY_Json, MN_("getArray"),  1, TY_String, FN_key,
		_Public|_Const|_Im, _F(kJson_getBool),   TY_boolean,   TY_Json, MN_("getBool"),   1, TY_String, FN_key,
		_Public|_Const|_Im, _F(kJson_getFloat),  TY_float,     TY_Json, MN_("getFloat"),  1, TY_String, FN_key,
		_Public|_Const|_Im, _F(kJson_getInt),    TY_int,       TY_Json, MN_("getInt"),    1, TY_String, FN_key,
		_Public|_Const|_Im, _F(kJson_getString), TY_String,    TY_Json, MN_("getString"), 1, TY_String, FN_key,
		_Public,            _F(kJson_new),       TY_Json,      TY_Json, MN_("new"), 0,
		_Public,            _F(kJson_set),       TY_void,      TY_Json, MN_("set"), 2, TY_String, FN_key, TY_Json, FN_("value"),
		_Public|_Const|_Im, _F(kJsonArray_get),    TY_Json,  TY_JsonArray, MN_("get"),    1, TY_int,  FN_("index"),
		_Public,            _F(kJsonArray_append), TY_void,  TY_JsonArray, MN_("add"),    1, TY_Json, FN_v,
		_Public,            _F(kJsonArray_append), TY_void,  TY_JsonArray, MN_("append"), 1, TY_Json, FN_v,
		_Public|_Const|_Im, _F(kJsonArray_getSize),TY_int,   TY_JsonArray, MN_("getSize"),0,
		_Public|_Const|_Im, _F(kJson_getKeys),    TY_StringArray, TY_Json, MN_("getKeys"), 0,
		_Public|_Const|_Im, _F(Int_toJson),   TY_Json, TY_int,    MN_to(TY_Json), 0,
		_Public|_Const|_Im, _F(Float_toJson), TY_Json, TY_float,  MN_to(TY_Json), 0,
		_Public|_Const|_Im, _F(String_toJson),TY_Json, TY_String, MN_to(TY_Json), 0,
		_Public|_Const|_Im, _F(Array_toJson), TY_Json, TY_Array, MN_to(TY_Json), 0,
		_Public|_Const|_Im, _F(Json_toString),TY_String, TY_Json, MN_to(TY_String), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

static kbool_t JSON_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* json_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("JSON", "1.0"),
		.PackupNameSpace    = JSON_PackupNameSpace,
		.ExportNameSpace   = JSON_ExportNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
