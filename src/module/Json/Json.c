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

#include "kjson/kjson.c"
#include <minikonoha/minikonoha.h>

// -------------------------------------------------------------------------
/* JSON Parse/ToString API */

struct JsonBuf {
	uint64_t json_i;
};

#define AsJSON(JSONBUF) (toJSON(JSONBUF->json_i))

static kbool_t IsJsonType(struct JsonBuf *jsonbuf, KJSONTYPE type)
{
	switch(type) {
	case KJSON_OBJECT:  return JSON_TYPE_CHECK(Object, AsJSON(jsonbuf));
	case KJSON_ARRAY:   return JSON_TYPE_CHECK(Array , AsJSON(jsonbuf));
	case KJSON_STRING:  return JSON_TYPE_CHECK(String, AsJSON(jsonbuf));
	case KJSON_INT:     return JSON_TYPE_CHECK(Int32 , AsJSON(jsonbuf));
	case KJSON_DOUBLE:  return JSON_TYPE_CHECK(Double, AsJSON(jsonbuf));
	case KJSON_BOOLEAN: return JSON_TYPE_CHECK(Bool  , AsJSON(jsonbuf));
	case KJSON_NULL:    return JSON_TYPE_CHECK(Null  , AsJSON(jsonbuf));
	case KJSON_INT64:   return JSON_TYPE_CHECK(Int64 , AsJSON(jsonbuf));
	case KJSON_LONG:    return JSON_TYPE_CHECK(Int32 , AsJSON(jsonbuf));
	}
	return false;
}

static uint64_t NewJsonI(JSONMemoryPool *pool, KJSONTYPE type, va_list ap)
{
	switch(type) {
		case KJSON_OBJECT:   return JSONObject_new(pool, 0).bits;
		case KJSON_ARRAY:    return JSONArray_new(pool, 0).bits;
		case KJSON_STRING:   {
			const char *s = va_arg(ap, const char *);
			return JSONString_new(pool, s, strlen(s)).bits;
		}
		case KJSON_INT:      return JSONInt_new(pool, va_arg(ap, intptr_t)).bits;
		case KJSON_DOUBLE:   return JSONDouble_new(va_arg(ap, double)).bits;
		case KJSON_BOOLEAN:  return JSONBool_new(va_arg(ap, int)).bits;
		case KJSON_NULL:     return JSONNull_new().bits;
		case KJSON_INT64:    return JSONInt_new(pool, va_arg(ap, int64_t)).bits;
		case KJSON_LONG:     return JSONInt_new(pool, va_arg(ap, long)).bits;
	}
	return 0;
}

static struct JsonBuf *CreateJson(KonohaContext *kctx, struct JsonBuf *jsonbuf, KJSONTYPE type, ...)
{
	va_list ap;
	va_start(ap, type);
	jsonbuf->json_i = NewJsonI((JSONMemoryPool *)(PLATAPI JsonHandler), type, ap);
	va_end(ap);
	return jsonbuf;
}

static kbool_t ParseJson(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *text, size_t length, KTraceInfo *trace)
{
	JSON json = parseJSON((JSONMemoryPool *)(PLATAPI JsonHandler), text, text + length);
	if(IsError(json.val)) {
		KLIB KRuntime_raise(kctx, KException_("InvalidJsonText"), SoftwareFault, NULL, trace->baseStack);
	}
	jsonbuf->json_i = json.bits;
	return jsonbuf->json_i != 0;
}

static void FreeJson(KonohaContext *kctx, struct JsonBuf *jsonbuf)
{
	JSON_free(AsJSON(jsonbuf));
}

static const char *JsonToNewText(KonohaContext *kctx, struct JsonBuf *jsonbuf)
{
	size_t length;
	const char *text = JSON_toStringWithLength(AsJSON(jsonbuf), &length);
	return text; /* need free */
}

static size_t DoJsonEach(KonohaContext *kctx, struct JsonBuf *jsonbuf, void *thunk, void (*doEach)(KonohaContext *, const char *, struct JsonBuf *, void *))
{
	size_t count = 0;
	struct JsonBuf eachbuf;
	JSON Key, Val;
	JSONObject_iterator Itr;
	JSON obj = AsJSON(jsonbuf);
	JSON_OBJECT_EACH(obj, Itr, Key, Val) {
		const char *key = JSONString_get(Key);
		eachbuf.json_i = Val.bits;
		doEach(kctx, key, &eachbuf, thunk);
		count++;
	}
	return count;
}

#define KeyLen(key, keylen_or_zero)  ((keylen_or_zero == 0) ? strlen(key) : keylen_or_zero)

static kbool_t RetrieveJsonKeyValue(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, struct JsonBuf *newbuf)
{
	JSON json = JSON_get(AsJSON(jsonbuf), key, KeyLen(key, keylen_or_zero));
	JSONObject_Retain(json);
	newbuf->json_i = json.bits;
	return newbuf->json_i != 0;
}

static kbool_t SetJsonKeyValue(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, struct JsonBuf *otherbuf)
{
	JSON json = AsJSON(jsonbuf);
	if(!JSON_TYPE_CHECK(Object, json)) {
		return false;
	}
	size_t keylen = KeyLen(key, keylen_or_zero);
	JSONObject_set((JSONMemoryPool *)(PLATAPI JsonHandler), json, key, keylen, AsJSON(otherbuf));
	return true;
}

static kbool_t SetJsonValue(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, KJSONTYPE type, ...)
{
	va_list ap;
	va_start(ap, type);
	JSON val = toJSON(NewJsonI((JSONMemoryPool *)(PLATAPI JsonHandler), type, ap));
	kbool_t ret = true;
	if(key != NULL) {
		size_t keylen = KeyLen(key, keylen_or_zero);
		JSONObject_set((JSONMemoryPool *)(PLATAPI JsonHandler), AsJSON(jsonbuf), key, keylen, val);
	}
	else {
		jsonbuf->json_i = val.bits;
	}
	va_end(ap);
	return ret;
}

static kbool_t GetJsonBoolean(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, kbool_t defval)
{
	JSON json = AsJSON(jsonbuf);
	if(key == NULL)
		return JSONBool_get(json);
	return JSON_getBool(json, key, KeyLen(key, keylen_or_zero));
}

static int64_t GetJsonInt(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, int64_t defval)
{
	JSON json = AsJSON(jsonbuf);
	if(key == NULL) {
		if(JSON_TYPE_CHECK(Int32, json)) {
			return JSONInt_get(json);
		}
	}
	return JSON_getInt(json, key, KeyLen(key, keylen_or_zero));
}

static double GetJsonFloat(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, double defval)
{
	JSON json = AsJSON(jsonbuf);
	if(key == NULL)
		JSONDouble_get(json);
	return JSON_getDouble(json, key, KeyLen(key, keylen_or_zero));
}

static const char *GetJsonText(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, const char *defval)
{
	JSON json = AsJSON(jsonbuf);
	if(key == NULL)
		JSONString_get(json);
	size_t length = KeyLen(key, keylen_or_zero);
	return JSON_getString(json, key, &length);
}

static size_t GetJsonSize(KonohaContext *kctx, struct JsonBuf *jsonbuf)
{
	return JSON_length(AsJSON(jsonbuf));
}

static kbool_t RetrieveJsonArrayAt(KonohaContext *kctx, struct JsonBuf *jsonbuf, size_t index, struct JsonBuf *otherbuf)
{
	JSON json = AsJSON(jsonbuf);
	if(JSON_TYPE_CHECK(Array, json)) {
		JSON ret = JSONArray_get(json, index);
		JSONObject_Retain(ret);
		otherbuf->json_i = ret.bits;
		return true;
	}
	return false;
}

static kbool_t SetJsonArrayAt(KonohaContext *kctx, struct JsonBuf *jsonbuf, size_t index, struct JsonBuf *otherbuf)
{
	JSON json = AsJSON(jsonbuf);
	if(JSON_TYPE_CHECK(Array, json)) {
		JSONArray_set(json, index, toJSON(otherbuf->json_i));
		return true;
	}
	return false;
}

static kbool_t AppendJsonArray(KonohaContext *kctx, struct JsonBuf *jsonbuf, struct JsonBuf *otherbuf)
{
	JSON json = AsJSON(jsonbuf);
	if(JSON_TYPE_CHECK(Array, json)) {
		JSONArray_append((JSONMemoryPool *)(PLATAPI JsonHandler), json, toJSON(otherbuf->json_i));
		return true;
	}
	return false;
}

// -------------------------------------------------------------------------
static void InitJsonContext(KonohaContext *kctx)
{
	JSONMemoryPool *mp = (JSONMemoryPool *) malloc(sizeof(JSONMemoryPool));
	JSONMemoryPool_Init(mp);
	KonohaFactory *factory = (KonohaFactory *) kctx->platApi;
	factory->JsonHandler = (void *) mp;
}

static void DeleteJsonContext(KonohaContext *kctx)
{
	JSONMemoryPool *mp = (JSONMemoryPool *)(PLATAPI JsonHandler);
	JSONMemoryPool_Delete(mp);
	free(mp);
}

kbool_t LoadJsonModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"Json", "0.1", 0, "json",
	};
	factory->JsonDataInfo         = &ModuleInfo;
	factory->IsJsonType           = IsJsonType;
	factory->InitJsonContext      = InitJsonContext;
	factory->DeleteJsonContext    = DeleteJsonContext;
	factory->CreateJson           = CreateJson;
	factory->ParseJson            = ParseJson;
	factory->FreeJson             = FreeJson;
	factory->JsonToNewText        = JsonToNewText;
	factory->DoJsonEach           = DoJsonEach;
	factory->RetrieveJsonKeyValue = RetrieveJsonKeyValue;
	factory->SetJsonKeyValue      = SetJsonKeyValue;
	factory->SetJsonValue         = SetJsonValue;
	factory->GetJsonBoolean       = GetJsonBoolean;
	factory->GetJsonInt           = GetJsonInt;
	factory->GetJsonFloat         = GetJsonFloat;
	factory->GetJsonText          = GetJsonText;
	factory->GetJsonSize          = GetJsonSize;
	factory->RetrieveJsonArrayAt  = RetrieveJsonArrayAt;
	factory->SetJsonArrayAt       = SetJsonArrayAt;
	factory->AppendJsonArray      = AppendJsonArray;
	factory->JsonHandler          = NULL;
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
