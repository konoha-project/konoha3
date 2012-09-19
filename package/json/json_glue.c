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
#include <minikonoha/float.h>
#include <stdbool.h>

#include "kjson/kjson.c"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kJSON {
	KonohaObjectHeader h;
	JSON json;
} kJSON;

/* ------------------------------------------------------------------------ */

static void kJSON_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kJSON *json = (kJSON *)o;
	json->json  = JSONNull_new();
}

static void kJSON_free(KonohaContext *kctx, kObject *o)
{
	kJSON *json = (kJSON *)o;
	if(json->json.bits) {
		JSON_free(json->json);
	}
}

static void kJSON_p(KonohaContext *kctx, KonohaStack *sfp, int pos, KUtilsWriteBuffer *wb, int level)
{
	kJSON *json = (kJSON *)sfp[pos].asObject;
	char *data = JSON_toString(json->json);
	KLIB Kwb_printf(kctx, wb, "%s", data);
	free(data);
}

/* ------------------------------------------------------------------------ */
/* [API methodList] */

//## JSON JSON.new();
static KMETHOD kJSON_new(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(sfp[0].o);
}

static kJSON *NewJsonObject(KonohaContext *kctx, KonohaStack *sfp, JSON val)
{
	kJSON *json = (kJSON*)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].asObject), 0);
	json->json = val;
	return json;
}

//## @Static JSON JSON.parse(String str);
static KMETHOD kJSON_parse(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *text = S_text(sfp[1].asString);
	JSON json = parseJSON(text, text+S_size(sfp[1].asString));
	if (!json.bits) {
		/* FIXME Error Handling */
		RETURN_DefaultObjectValue();
	}
	RETURN_(NewJsonObject(kctx, sfp, json));
}

//## JSON JSON.get(String key);
static KMETHOD kJSON_get(KonohaContext *kctx, KonohaStack *sfp)
{
	JSON obj = ((kJSON*)sfp[0].asObject)->json;
	const char *key = S_text(sfp[1].asString);
	JSON json = JSON_get(obj, key);
	RETURN_(NewJsonObject(kctx, sfp, json));
}

//## JSONArray JSON.getArray();
static KMETHOD kJSON_getArray(KonohaContext *kctx, KonohaStack *sfp)
{
	JSON obj = ((kJSON*)sfp[0].asObject)->json;
	const char *key = S_text(sfp[1].asString);
	JSON json = JSON_get(obj, key);
	RETURN_(NewJsonObject(kctx, sfp, json));
}

//## JSONBool JSON.getBool(String key);
static KMETHOD kJSON_getBool(KonohaContext *kctx, KonohaStack *sfp)
{
	JSON obj = ((kJSON*)sfp[0].asObject)->json;
	const char *key = S_text(sfp[1].asString);
	bool json = JSON_getBool(obj, key);
	RETURNb_(json);
}

//## JSONFloat JSON.getFloat(String key);
static KMETHOD kJSON_getFloat(KonohaContext *kctx, KonohaStack *sfp)
{
	JSON obj = ((kJSON*)sfp[0].asObject)->json;
	const char *key = S_text(sfp[1].asString);
	double json = JSON_getDouble(obj, key);
	RETURNf_(json);
}

//## String JSON.getInt(String key);
static KMETHOD kJSON_getInt(KonohaContext *kctx, KonohaStack *sfp)
{
	JSON obj = ((kJSON*)sfp[0].asObject)->json;
	const char *key = S_text(sfp[1].asString);
	int32_t json = JSON_getInt(obj, key);
	RETURNi_(json);
}

//## String JSON.getString(String key);
static KMETHOD kJSON_getString(KonohaContext *kctx, KonohaStack *sfp)
{
	JSON obj = ((kJSON*)sfp[0].asObject)->json;
	const char *key = S_text(sfp[1].asString);
	size_t len;
	const char *text = JSON_getString(obj, key, &len);
	RETURN_(KLIB new_kString(kctx, text, len, 0));
}

//## void JSON.set(String key, JSON value);
static KMETHOD kJSON_set(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s = sfp[1].asString;
	JSON obj, key, val;
	obj = ((kJSON*)sfp[0].asObject)->json;
	val = ((kJSON*)sfp[2].asObject)->json;
	key = JSONString_new(S_text(s), S_size(s));
	JSONObject_set(obj, key, val);
	RETURNvoid_();
}

/* ------------------------------------------------------------------------ */

//## void JSONArray.append(JSON json);
static KMETHOD kJSONArray_append(KonohaContext *kctx, KonohaStack *sfp)
{
	JSON obj = ((kJSON*)sfp[0].asObject)->json;
	JSON val = ((kJSON*)sfp[1].asObject)->json;
	JSONArray_append(obj, val);
	RETURNvoid_();
}

//## int JSON.getSize();
static KMETHOD kJSONArray_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	JSON obj = ((kJSON*)sfp[0].asObject)->json;
	RETURNi_(JSON_length(obj));
}

//## JSON JSONArray.get(int index);
static KMETHOD kJSONArray_get(KonohaContext *kctx, KonohaStack *sfp)
{
	JSON obj = ((kJSON*)sfp[0].asObject)->json;
	JSON json = JSONArray_get(obj, sfp[1].intValue);
	RETURN_(NewJsonObject(kctx, sfp, json));
}

/* ------------------------------------------------------------------------ */
#define _Public  kMethod_Public
#define _Const   kMethod_Const
#define _Im      kMethod_Immutable
#define _Static  kMethod_Static
#define _F(F)   (intptr_t)(F)

#define CT_JSON     cJSON
#define TY_JSON     cJSON->typeId

static kbool_t JSON_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KRequirePackage("konoha.float", pline);
	KDEFINE_CLASS JSONDef = {
		STRUCTNAME(JSON),
		.cflag = kClass_Final,
		.init = kJSON_init,
		.free = kJSON_free,
		.p    = kJSON_p,
	};
	KonohaClass *cJSON = KLIB kNameSpace_defineClass(kctx, ns, NULL, &JSONDef, pline);
	kparamtype_t ps = {TY_JSON, FN_("json")};
	KonohaClass *CT_JSONArray = KLIB KonohaClass_Generics(kctx, CT_Array, TY_JSON, 1, &ps);
	ktype_t TY_JSONArray = CT_JSONArray->typeId;

	int FN_key = FN_("key");
	int FN_v   = FN_("v");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Static, _F(kJSON_parse), TY_JSON,      TY_JSON, MN_("parse"),     1, TY_String, FN_("data"),
		_Public|_Const|_Im, _F(kJSON_get),       TY_JSON,      TY_JSON, MN_("get"),       1, TY_String, FN_key,
		_Public|_Const|_Im, _F(kJSON_getArray),  TY_JSONArray, TY_JSON, MN_("getArray"),  1, TY_String, FN_key,
		_Public|_Const|_Im, _F(kJSON_getBool),   TY_boolean,   TY_JSON, MN_("getBool"),   1, TY_String, FN_key,
		_Public|_Const|_Im, _F(kJSON_getFloat),  TY_float,     TY_JSON, MN_("getFloat"),  1, TY_String, FN_key,
		_Public|_Const|_Im, _F(kJSON_getInt),    TY_int,       TY_JSON, MN_("getInt"),    1, TY_String, FN_key,
		_Public|_Const|_Im, _F(kJSON_getString), TY_String,    TY_JSON, MN_("getString"), 1, TY_String, FN_key,
		_Public, _F(kJSON_new),  TY_JSON,      TY_JSON, MN_("new"), 0,
		_Public, _F(kJSON_set),  TY_void,      TY_JSON, MN_("set"), 2, TY_String, FN_key, TY_JSON, FN_("value"),

		_Public|_Const|_Im, _F(kJSONArray_get),    TY_JSON,  TY_JSONArray, MN_("get"),    1, TY_int,  FN_("index"),
		_Public, _F(kJSONArray_append),            TY_void,  TY_JSONArray, MN_("add"),    1, TY_JSON, FN_v,
		_Public, _F(kJSONArray_append),            TY_void,  TY_JSONArray, MN_("append"), 1, TY_JSON, FN_v,
		_Public|_Const|_Im, _F(kJSONArray_getSize),TY_int,   TY_JSONArray, MN_("getSize"),0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t JSON_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t JSON_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	//KImportPackage("konoha.string", ns,  pline);
	//KImportPackage("konoha.float", ns, pline);
	return true;
}

static kbool_t JSON_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* JSON_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("JSON", "1.0"),
		.initPackage = JSON_initPackage,
		.setupPackage = JSON_setupPackage,
		.initNameSpace = JSON_initNameSpace,
		.setupNameSpace = JSON_setupNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif

