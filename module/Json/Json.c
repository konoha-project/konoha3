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

struct JsonBuf {
	uint64_t json_i;
};

/* please export below */

//typedef enum kjson_type {
//    /** ($type & 1 == 0) means $type extends Number */
//    JSON_Double   =  0, /* 0b00000 */
//    JSON_String   =  1, /* 0b00001 */
//    JSON_Int32    =  2, /* 0b00010 */
//    JSON_Object   =  3, /* 0b00011 */
//    JSON_Bool     =  4, /* 0b00100 */
//    JSON_Array    =  5, /* 0b00101 */
//    JSON_Null     =  6, /* 0b00110 */
//    JSON_UString  =  9, /* 0b01001 */
//    JSON_Int64    = 11, /* 0b01011 */
//    JSON_Error    = 15, /* 0b01111 */
//    JSON_reserved =  7  /* 0b00111 '7' is reserved by numbox */
//} kjson_type;
//
//static inline kjson_type JSON_type(JSON json) {
//    Value v; v.bits = (uint64_t)json.val.bits;
//    uint64_t tag = Tag(v);
//    return (IsDouble((v)))?
//        JSON_Double : (kjson_type) ((tag >> TagBitShift) & 15);
//}
//
//#define JSON_TYPE_CHECK(T, O) (JSON_type(((JSON)O)) == JSON_##T)

static kbool_t IsJsonType(struct JsonBuf *jsonbuf, KJSONTYPE type)
{
//	switch(type) {
//		case KJSON_OBJECT:   return JSON_TYPE_CHECK(toJSON(jsonbuf->json_i), Object);
//		case KJSON_ARRAY:    return JSON_TYPE_CHECK(toJSON(jsonbuf->json_i), Array);
//		case KJSON_STRING:   return JSON_TYPE_CHECK(toJSON(jsonbuf->json_i), String);
//		case KJSON_INT:      return JSON_TYPE_CHECK(toJSON(jsonbuf->json_i), Int32);
//		case KJSON_DOUBLE:   return JSON_TYPE_CHECK(toJSON(jsonbuf->json_i), Double);
//		case KJSON_BOOLEAN:  return JSON_TYPE_CHECK(toJSON(jsonbuf->json_i), Bool);
//		case KJSON_NULL:     return JSON_TYPE_CHECK(toJSON(jsonbuf->json_i), Null);
//		case KJSON_INT64:    return JSON_TYPE_CHECK(toJSON(jsonbuf->json_i), Int64); /* Int64 */
//		case KJSON_LONG:     return JSON_TYPE_CHECK(toJSON(jsonbuf->json_i), Int32);
//	}
	return false; /* imf */
}

static uint64_t NewJsonI(JSONMemoryPool *pool, KJSONTYPE type, va_list ap)
{
	switch(type) {
		case KJSON_OBJECT:   return JSONObject_new(pool).bits;
		case KJSON_ARRAY:    return JSONArray_new(pool).bits;
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

static struct JsonBuf* CreateJson(KonohaContext *kctx, struct JsonBuf *jsonbuf, KJSONTYPE type, ...)
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
	return true;  // what happens when the text is broken ?
}

static void FreeJson(KonohaContext *kctx, struct JsonBuf *jsonbuf)
{
	JSON_free(toJSON(jsonbuf->json_i));
}

static const char* JsonToNewText(KonohaContext *kctx, struct JsonBuf *jsonbuf)
{
	/* FIXME: I don't understand how to use this api at all (by kimio) */
	const char *text = JSON_toStringWithLength(toJSON(jsonbuf->json_i), NULL);
//	*lengthPtr = *lengthPtr - 1;
	return text;  // is it okay to free?
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
	return true; /* always ? */
}

static kbool_t SetJsonKeyValue(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, struct JsonBuf *otherbuf)
{
	JSON tmp = JSONString_new((JSONMemoryPool *)(PLATAPI JsonHandler), key, KeyLen(key, keylen_or_zero));
	JSONObject_set((JSONMemoryPool *)(PLATAPI JsonHandler), toJSON(jsonbuf->json_i), tmp, toJSON(otherbuf->json_i));
	return true; /* always ? */
}

static kbool_t SetJsonValue(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, KJSONTYPE type, ...)
{
	va_list ap;
	va_start(ap, type);
	JSON val = toJSON(NewJsonI((JSONMemoryPool *)(PLATAPI JsonHandler), type, ap));
	kbool_t ret = true;
	if(key != NULL) {
		JSON tmp = JSONString_new((JSONMemoryPool *)(PLATAPI JsonHandler), key, KeyLen(key, keylen_or_zero));
		JSONObject_set((JSONMemoryPool *)(PLATAPI JsonHandler), toJSON(jsonbuf->json_i), tmp, val);
	}
	else {
		jsonbuf->json_i = val.bits;
	}
	va_end(ap);
	return ret;
}

static kbool_t GetJsonBoolean(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, kbool_t defval)
{
	//return (key != NULL) ? JSON_getBool(toJSON(jsonbuf->json_i), key, KeyLen(key, keylen_or_zero)) : toBool(toJSON(jsonbuf->json_i));
	return false; /* imf */
}

static int64_t GetJsonInt(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, int64_t defval)
{
	//return (key != NULL) ? JSON_getInt(toJSON(jsonbuf->json_i), key, KeyLen(key, keylen_or_zero)) : toInt(jsonbuf->json_i);
	return false; /* imf */
}

static double GetJsonFloat(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, double defval)
{
	//return (key != NULL) ? JSON_getDouble(toJSON(jsonbuf->json_i), key, KeyLen(key, keylen_or_zero)) : toDouble(jsonbuf->json_i);
	return false; /* imf */
}

static const char* GetJsonText(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, const char *defval)
{
	size_t ValLen ; /* What is this ? */
	//return (key != NULL) ? JSON_getString(toJSON(jsonbuf->json_i), key, KeyLen(key, keylen_or_zero), &ValLen) : toString(jsonbuf->json_i);
	return false; /* imf */
}

static size_t GetJsonSize(KonohaContext *kctx, struct JsonBuf *jsonbuf)
{
	return JSON_length(toJSON(jsonbuf->json_i));
}

static kbool_t RetrieveJsonArrayIndex(KonohaContext *kctx, struct JsonBuf *jsonbuf, size_t index, struct JsonBuf *otherbuf)
{
//	JSONArray_get(jsonbuf->json_i, )
//	json_t *obj = json_array_get(jsonbuf->jsonobj, index);
//	if(obj != NULL) {
//		SetJsonBuf(otherbuf, obj);
//		return true;
//	}
	return true; /* always? */
}

static kbool_t SetJsonArrayIndex(KonohaContext *kctx, struct JsonBuf *jsonbuf, size_t index, struct JsonBuf *otherbuf)
{
	//JSONArray_set(json, idx, value);
	return false;
}

static kbool_t AppendJsonArray(KonohaContext *kctx, struct JsonBuf *jsonbuf, struct JsonBuf *otherbuf)
{
	JSONArray_append((JSONMemoryPool *)(PLATAPI JsonHandler), toJSON(jsonbuf->json_i), toJSON(otherbuf->json_i));
	return true; /* always? */
}

// -------------------------------------------------------------------------

kbool_t LoadJsonModule(KonohaFactory *factory, ModuleType type)
{
	factory->Module_Json            = "Json (i version)";
	factory->Module_Json            = "jansson";
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
	factory->RetrieveJsonArrayIndex = RetrieveJsonArrayIndex;
	factory->SetJsonArrayIndex      = SetJsonArrayIndex;
	factory->AppendJsonArray        = AppendJsonArray;

	factory->JsonHandler            = (JSONMemoryPool *) malloc(sizeof(JSONMemoryPool));
	JSONMemoryPool_Init((JSONMemoryPool *)factory->JsonHandler);
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
