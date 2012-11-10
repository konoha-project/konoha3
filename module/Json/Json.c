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

static kbool_t IsJsonType(struct JsonBuf *jsonbuf, KJSONTYPE type)
{
	switch(type) {
	case KJSON_OBJECT:  return JSON_TYPE_CHECK(Object, toJSON(jsonbuf->json_i));
	case KJSON_ARRAY:   return JSON_TYPE_CHECK(Array , toJSON(jsonbuf->json_i));
	case KJSON_STRING:  return JSON_TYPE_CHECK(String, toJSON(jsonbuf->json_i));
	case KJSON_INT:     return JSON_TYPE_CHECK(Int32 , toJSON(jsonbuf->json_i));
	case KJSON_DOUBLE:  return JSON_TYPE_CHECK(Double, toJSON(jsonbuf->json_i));
	case KJSON_BOOLEAN: return JSON_TYPE_CHECK(Bool  , toJSON(jsonbuf->json_i));
	case KJSON_NULL:    return JSON_TYPE_CHECK(Null  , toJSON(jsonbuf->json_i));
	case KJSON_INT64:   return JSON_TYPE_CHECK(Int64 , toJSON(jsonbuf->json_i));
	case KJSON_LONG:    return JSON_TYPE_CHECK(Int32 , toJSON(jsonbuf->json_i));
	}
	return false; /* imf */
}

static uint64_t NewJsonI(JSONMemoryPool *pool, KJSONTYPE type, va_list ap)
{
	switch(type) {
		case KJSON_OBJECT:   return JSONObject_new(pool, 0).bits;
		case KJSON_ARRAY:    return JSONArray_new(pool, 0).bits;
		case KJSON_STRING:   {
			const char *s = va_arg(ap, const char*);
			return JSONString_new(pool, s, strlen(s)).bits;
		}
		case KJSON_INT:      return JSONInt_new(pool, va_arg(ap, int)).bits;
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
	jsonbuf->json_i = parseJSON((JSONMemoryPool *)(PLATAPI JsonHandler), text, text + length).bits;
	return jsonbuf->json_i != 0;
}

static void FreeJson(KonohaContext *kctx, struct JsonBuf *jsonbuf)
{
	JSON_free(toJSON(jsonbuf->json_i));
}

static const char *JsonToNewText(KonohaContext *kctx, struct JsonBuf *jsonbuf)
{
	size_t length;
	const char *text = JSON_toStringWithLength(toJSON(jsonbuf->json_i), &length);
	return text; /* need free */
}

static size_t DoJsonEach(KonohaContext *kctx, struct JsonBuf *jsonbuf, void *thunk, void (*doEach)(KonohaContext *, const char *, struct JsonBuf *, void *))
{
	size_t count = 0;
	struct JsonBuf eachbuf = {};
	{
		JSON Key, Val;
		JSONObject_iterator Itr;
		JSON obj = toJSON(jsonbuf->json_i);
		JSON_OBJECT_EACH(obj, Itr, Key, Val) {
			const char *key = Key.str->str;
			eachbuf.json_i = Val.bits;
			doEach(kctx, key, &eachbuf, thunk);
			count++;
		}
	}
	return count;
}

#define KeyLen(key, keylen_or_zero)  ((keylen_or_zero == 0) ? strlen(key) : keylen_or_zero)

static kbool_t RetrieveJsonKeyValue(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, struct JsonBuf *newbuf)
{
	newbuf->json_i = JSON_get(toJSON(jsonbuf->json_i), key, KeyLen(key, keylen_or_zero)).bits;
	return newbuf->json_i != 0;
}

static kbool_t SetJsonKeyValue(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, struct JsonBuf *otherbuf)
{
	if(!JSON_TYPE_CHECK(Object, toJSON(jsonbuf->json_i))) {
		return false;
	}
	size_t keylen = KeyLen(key, keylen_or_zero);
	JSONObject_set((JSONMemoryPool *)(PLATAPI JsonHandler), toJSON(jsonbuf->json_i),
			key, keylen, toJSON(otherbuf->json_i));
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
		JSONObject_set((JSONMemoryPool *)(PLATAPI JsonHandler),
				toJSON(jsonbuf->json_i), key, keylen, val);
	}
	else {
		jsonbuf->json_i = val.bits;
	}
	va_end(ap);
	return ret;
}

static kbool_t GetJsonBoolean(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, kbool_t defval)
{
	if(key == NULL)
		return JSONBool_get(toJSON(jsonbuf->json_i));
	return JSON_getBool(toJSON(jsonbuf->json_i), key, KeyLen(key, keylen_or_zero));
}

static int64_t GetJsonInt(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, int64_t defval)
{
	if(key == NULL) {
		if(JSON_TYPE_CHECK(Int32, toJSON(jsonbuf->json_i))) {
			return JSONInt_get(toJSON(jsonbuf->json_i));
		}
	}
	return JSON_getInt(toJSON(jsonbuf->json_i), key, KeyLen(key, keylen_or_zero));
}

static double GetJsonFloat(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, double defval)
{
	if(key == NULL)
		JSONDouble_get(toJSON(jsonbuf->json_i));
	return JSON_getDouble(toJSON(jsonbuf->json_i), key, KeyLen(key, keylen_or_zero));
}

static const char *GetJsonText(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, const char *defval)
{
	if(key == NULL)
		JSONString_get(toJSON(jsonbuf->json_i));
	size_t length = KeyLen(key, keylen_or_zero);
	return JSON_getString(toJSON(jsonbuf->json_i), key, &length);
}

static size_t GetJsonSize(KonohaContext *kctx, struct JsonBuf *jsonbuf)
{
	return JSON_length(toJSON(jsonbuf->json_i));
}

static kbool_t RetrieveJsonArrayAt(KonohaContext *kctx, struct JsonBuf *jsonbuf, size_t index, struct JsonBuf *otherbuf)
{
	if(JSON_TYPE_CHECK(Array, toJSON(jsonbuf->json_i))) {
		otherbuf->json_i = JSONArray_get(toJSON(jsonbuf->json_i), index).bits;
		return otherbuf->json_i == 0;
	}
	return false;
}

static kbool_t SetJsonArrayAt(KonohaContext *kctx, struct JsonBuf *jsonbuf, size_t index, struct JsonBuf *otherbuf)
{
	if(JSON_TYPE_CHECK(Array, toJSON(jsonbuf->json_i))) {
		JSONArray_set(toJSON(jsonbuf->json_i), index, toJSON(otherbuf->json_i));
		return true;
	}
	return false;
}

static kbool_t AppendJsonArray(KonohaContext *kctx, struct JsonBuf *jsonbuf, struct JsonBuf *otherbuf)
{
	if(JSON_TYPE_CHECK(Array, toJSON(jsonbuf->json_i))) {
		JSONArray_append((JSONMemoryPool *)(PLATAPI JsonHandler),
				toJSON(jsonbuf->json_i), toJSON(otherbuf->json_i));
		return true;
	}
	return false;
}

// -------------------------------------------------------------------------

kbool_t LoadJsonModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"Json", "0.1", 0, "json",
	};
	factory->JsonDataInfo            = &ModuleInfo;
	factory->IsJsonType             = IsJsonType;
	factory->CreateJson             = CreateJson;
	factory->ParseJson              = ParseJson;
	factory->FreeJson               = FreeJson;
	factory->JsonToNewText          = JsonToNewText;
	factory->DoJsonEach             = DoJsonEach;
	factory->RetrieveJsonKeyValue   = RetrieveJsonKeyValue;
	factory->SetJsonKeyValue        = SetJsonKeyValue;
	factory->SetJsonValue           = SetJsonValue;
	factory->GetJsonBoolean         = GetJsonBoolean;
	factory->GetJsonInt             = GetJsonInt;
	factory->GetJsonFloat           = GetJsonFloat;
	factory->GetJsonText            = GetJsonText;
	factory->GetJsonSize            = GetJsonSize;
	factory->RetrieveJsonArrayAt = RetrieveJsonArrayAt;
	factory->SetJsonArrayAt      = SetJsonArrayAt;
	factory->AppendJsonArray        = AppendJsonArray;

	factory->JsonHandler            = (JSONMemoryPool *) malloc(sizeof(JSONMemoryPool));
	JSONMemoryPool_Init((JSONMemoryPool *)factory->JsonHandler);
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
