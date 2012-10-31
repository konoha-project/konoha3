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

#ifdef __cplusplus
extern "C" {
#endif

#include "./kjson/kjson.c"
#include <minikonoha/minikonoha.h>

// -------------------------------------------------------------------------
/* JSON Parse/ToString API */
static KJson_t Json_parse(KJsonContext jctx, const char *text, size_t length)
{
	return parseJSON((JSONMemoryPool *) jctx, text, text + length).bits;
}

static const char *Json_toString(KJsonContext jctx, KJson_t json, size_t *lengthPtr)
{
	const char *text = JSON_toStringWithLength(toJSON(json), lengthPtr);
	*lengthPtr = *lengthPtr - 1;
	return text;
}

/* JSON New/Delete API */
static KJson_t JsonInt_new(KJsonContext jctx, int64_t val)
{
	return JSONInt_new((JSONMemoryPool *) jctx, val).bits;
}

static KJson_t JsonDouble_new(KJsonContext jctx, double val)
{
	return JSONDouble_new(val).bits;
}

static KJson_t JsonString_new(KJsonContext jctx, const char *text, size_t length)
{
	return JSONString_new((JSONMemoryPool *) jctx, text, length).bits;
}

static KJson_t JsonArray_new(KJsonContext jctx)
{
	return JSONArray_new((JSONMemoryPool *) jctx).bits;
}

static KJson_t JsonObject_new(KJsonContext jctx)
{
	return JSONObject_new((JSONMemoryPool *) jctx).bits;
}

static void Json_free(KJsonContext jctx, KJson_t json)
{
	JSON_free(toJSON(json));
}

static kbool_t JsonToObject(KJsonContext jctx, KonohaContext *kctx, KJson_t json, ktype_t RequestType, KonohaStack *sfp)
{
	if(RequestType == TY_int) {
		sfp[0].intValue = JSONInt_get(toJSON(json));
		return true;
	}
	//if(RequestType == TY_float) {
	//	sfp[0].floatValue =  JSONDouble_get(toJSON(json));
	//	return true;
	//}
	if(RequestType == TY_String) {
		const char *text = JSONString_get(toJSON(json));
		sfp[0].asString = KLIB new_kString(kctx, OnStack, text, JSON_length(toJSON(json)), 0);
		return true;
	}
	return false;
}

/* JSONObject API */
static void JsonObject_set(KJsonContext jctx, KJson_t json, const char *Key, size_t KeyLen, KJson_t val)
{
	JSON tmp = JSONString_new((JSONMemoryPool *) jctx, Key, KeyLen);
	JSONObject_set((JSONMemoryPool *) jctx, toJSON(json), tmp, toJSON(val));
}

static KJson_t JsonObject_get(KJsonContext jctx, KJson_t json, const char *Key, size_t KeyLen)
{
	return JSON_get(toJSON(json), Key, KeyLen).bits;
}

static bool JsonObject_getBool(KJsonContext jctx, KJson_t json, const char *Key, size_t KeyLen)
{
	return JSON_getBool(toJSON(json), Key, KeyLen);
}

static int64_t JsonObject_getInt(KJsonContext jctx, KJson_t json, const char *Key, size_t KeyLen)
{
	return JSON_getInt(toJSON(json), Key, KeyLen);
}

static double JsonObject_getDouble(KJsonContext jctx, KJson_t json, const char *Key, size_t KeyLen)
{
	return JSON_getDouble(toJSON(json), Key, KeyLen);
}

static const char *JsonObject_getString(KJsonContext jctx, KJson_t json, const char *Key, size_t KeyLen, size_t *ValLen)
{
	*ValLen = KeyLen;
	return JSON_getString(toJSON(json), Key, ValLen);
}

static KJson_t JsonObject_getArray(KJsonContext jctx, KJson_t json, const char *Key, size_t KeyLen)
{
	return JSON_get(toJSON(json), Key, KeyLen).bits;
}

static KJson_t JsonObject_getObject(KJsonContext jctx, KJson_t json, const char *Key, size_t KeyLen)
{
	return JSON_get(toJSON(json), Key, KeyLen).bits;
}

static void JsonObject_each(KJsonContext jctx, KJson_t json, void (*Func)(KJson_t, KJson_t, void *), void *thunk)
{
	JSON Key, Val;
	JSONObject_iterator Itr;
	JSON obj = toJSON(json);
	JSON_OBJECT_EACH(obj, Itr, Key, Val) {
		Func(Key.bits, Val.bits, thunk);
	}
}

/* JSONArray API */
static void JsonArray_append(KJsonContext jctx, KJson_t json, KJson_t val)
{
	JSONArray_append((JSONMemoryPool *) jctx, toJSON(json), toJSON(val));
}

static KJson_t JsonArray_get(KJsonContext jctx, KJson_t json, size_t idx)
{
	return JSONArray_get(toJSON(json), idx).bits;
}

static void JsonArray_set(KJsonContext jctx, KJson_t json, size_t idx, KJson_t value)
{
	//JSONArray_set(json, idx, value);
}

static unsigned Json_length(KJsonContext jctx, KJson_t json)
{
	return JSON_length(toJSON(json));
}

kbool_t LoadJsonModule(KonohaFactory *factory, ModuleType type)
{
	factory->Module_Json            = "KJson";
	factory->JsonContext            = (KJsonContext) (JSONMemoryPool *) malloc(sizeof(JSONMemoryPool));
	factory->Json_parse_i           = Json_parse;
	factory->Json_toString_i        = Json_toString;
	factory->JsonInt_new_i          = JsonInt_new;
	factory->JsonDouble_new_i       = JsonDouble_new;
	factory->JsonString_new_i       = JsonString_new;
	factory->JsonArray_new_i        = JsonArray_new;
	factory->JsonObject_new_i       = JsonObject_new;
	factory->Json_free_i            = Json_free;
	factory->JsonToObject_i         = JsonToObject;
	factory->JsonObject_set_i       = JsonObject_set;
	factory->JsonObject_get_i       = JsonObject_get;
	factory->JsonObject_getBool_i   = JsonObject_getBool;
	factory->JsonObject_getInt_i    = JsonObject_getInt;
	factory->JsonObject_getDouble_i = JsonObject_getDouble;
	factory->JsonObject_getString_i = JsonObject_getString;
	factory->JsonObject_getArray_i  = JsonObject_getArray;
	factory->JsonObject_getObject_i = JsonObject_getObject;
	factory->JsonObject_each_i      = JsonObject_each;
	factory->JsonArray_append_i     = JsonArray_append;
	factory->JsonArray_get_i        = JsonArray_get;
	factory->JsonArray_set_i        = JsonArray_set;
	factory->Json_length_i          = Json_length;
	JSONMemoryPool_Init((JSONMemoryPool *)factory->JsonContext);
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
