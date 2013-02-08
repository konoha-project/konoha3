/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
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

#include <jansson.h>
#include <konoha3/konoha.h>

// -------------------------------------------------------------------------

struct JsonBuf {
	json_t *jsonobj;
};

static void SetJsonBuf_NewRef(struct JsonBuf *jsonbuf, json_t *json)
{
	if(jsonbuf->jsonobj != NULL) {
		json_decref(jsonbuf->jsonobj);
	}
	jsonbuf->jsonobj = json;
}

static void SetJsonBuf(struct JsonBuf *jsonbuf, json_t *json)
{
	json_incref(json);
	if(jsonbuf->jsonobj != NULL) {
		json_decref(jsonbuf->jsonobj);
	}
	jsonbuf->jsonobj = json;
}

#if JANSSON_VERSION_HEX < 0x020400
#define json_boolean(val)      ((val) ? json_true() : json_false())
#endif /* JANSSON_VERSION_HEX < 0x020400 */

static json_t* NewJsonRef(KJSONTYPE type, va_list ap)
{
	json_t *newref = NULL;
	switch(type) {
		case KJSON_OBJECT:   newref = json_object(); break;
		case KJSON_ARRAY:    newref = json_array(); break;
		case KJSON_STRING:   newref = json_string_nocheck(va_arg(ap, const char *)); break;
		case KJSON_INT:      newref = json_integer(va_arg(ap, intptr_t)); break;
		case KJSON_DOUBLE:   newref = json_real(va_arg(ap, double)); break;
		case KJSON_BOOLEAN:  newref = json_boolean(va_arg(ap, int)); break;
		case KJSON_NULL:     newref = json_null();
		case KJSON_INT64:    newref = json_integer(va_arg(ap, int64_t));
		case KJSON_LONG:     newref = json_integer(va_arg(ap, long));
	}
	return newref;
}

static kbool_t IsJsonType(struct JsonBuf *jsonbuf, KJSONTYPE type)
{
	switch(type) {
		case KJSON_OBJECT:   return json_is_object(jsonbuf->jsonobj);
		case KJSON_ARRAY:    return json_is_array(jsonbuf->jsonobj);
		case KJSON_STRING:   return json_is_string(jsonbuf->jsonobj);
		case KJSON_INT:      return json_is_integer(jsonbuf->jsonobj);
		case KJSON_DOUBLE:   return json_is_real(jsonbuf->jsonobj);
		case KJSON_BOOLEAN:  return json_is_boolean(jsonbuf->jsonobj);
		case KJSON_NULL:     return json_is_null(jsonbuf->jsonobj);
		case KJSON_INT64:    return json_is_integer(jsonbuf->jsonobj);
		case KJSON_LONG:     return json_is_integer(jsonbuf->jsonobj);
	}
	return false;
}

static struct JsonBuf* CreateJson(KonohaContext *kctx, struct JsonBuf *jsonbuf, KJSONTYPE type, ...)
{
	va_list ap;
	va_start(ap, type);
	SetJsonBuf_NewRef(jsonbuf, NewJsonRef(type, ap));
	va_end(ap);
	return jsonbuf;
}

static kbool_t ParseJson(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *text, size_t length, KTraceInfo *trace)
{
	json_error_t err;
	json_t* obj = json_loadb(text, length, 0, &err);
	if(obj != NULL) {
		SetJsonBuf_NewRef(jsonbuf, obj);
		return true;
	}
	return false;
}

static void FreeJson(KonohaContext *kctx, struct JsonBuf *jsonbuf)
{
	if(jsonbuf->jsonobj != NULL) {
		json_decref((json_t *)jsonbuf->jsonobj);
		jsonbuf->jsonobj = NULL;
	}
}

static const char* JsonToNewText(KonohaContext *kctx, struct JsonBuf *jsonbuf)
{
	return (const char *) json_dumps(jsonbuf->jsonobj, 0);  // FIXME: if UTF-8 is supported
}

static size_t DoJsonEach(KonohaContext *kctx, struct JsonBuf *jsonbuf, void *thunk, void (*doEach)(KonohaContext *, const char *, size_t, struct JsonBuf *, void *))
{
	size_t count = 0;
	struct JsonBuf eachbuf = {};
	void *iter = json_object_iter(jsonbuf->jsonobj);
	while(iter != NULL) {
		const char *key = json_object_iter_key(iter);
		size_t len = strlen(key);
		json_t *value   = json_object_iter_value(iter);
		SetJsonBuf(&eachbuf, value);
		doEach(kctx, key, len, &eachbuf, thunk);
		count++;
		iter = json_object_iter_next(jsonbuf->jsonobj, iter);
	}
	FreeJson(kctx, &eachbuf);
	return count;
}

//static void WriteJsonToBuffer(KonohaContext *kctx, KBuffer *wb, struct JsonBuf *jsonbuf)
//{
//	char* data = json_dumps(jsonbuf->jsonobj, JSON_ENSURE_ASCII);  // FIXME: if UTF-8 is supported
//	KLIB KBuffer_Write(kctx, wb, data, strlen(data));
//	free(data);
//}

static kbool_t RetrieveJsonKeyValue(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, struct JsonBuf *newbuf)
{
	json_t *obj = json_object_get(jsonbuf->jsonobj, key);
	if(obj != NULL) {
		SetJsonBuf(newbuf, obj);
		return true;
	}
	return false;
}

static kbool_t SetJsonKeyValue(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, struct JsonBuf *otherbuf)
{
	return (json_object_set(jsonbuf->jsonobj, key, otherbuf->jsonobj) == 0);
}

static kbool_t SetJsonValue(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, KJSONTYPE type, ...)
{
	va_list ap;
	va_start(ap, type);
	json_t *newref = NewJsonRef(type, ap);
	kbool_t ret = true;
	if(key != NULL) {
		ret = (json_object_set_new(jsonbuf->jsonobj, key, newref) != -1);
	}
	else {
		SetJsonBuf_NewRef(jsonbuf, newref);
	}
	va_end(ap);
	return ret;
}

static kbool_t GetJsonBoolean(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, kbool_t defval)
{
	json_t* obj = (key != NULL) ? json_object_get(jsonbuf->jsonobj, key) : jsonbuf->jsonobj;
	if(json_is_boolean(obj)) {
		return (kbool_t)json_is_true(obj);
	}
	return defval;
}

static int64_t GetJsonInt(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, int64_t defval)
{
	json_t* obj = (key != NULL) ? json_object_get(jsonbuf->jsonobj, key) : jsonbuf->jsonobj;
	if(json_is_integer(obj)) {
		return (int64_t)json_integer_value(obj);
	}
	return defval;
}

static double GetJsonFloat(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, double defval)
{
	json_t* obj = (key != NULL) ? json_object_get(jsonbuf->jsonobj, key) : jsonbuf->jsonobj;
	if(json_is_real(obj)) {
		return json_real_value(obj);
	}
	return defval;
}

static const char* GetJsonText(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, const char *defval)
{
	json_t* obj = (key != NULL) ? json_object_get(jsonbuf->jsonobj, key) : jsonbuf->jsonobj;
	if(json_is_string(obj)) {
		return json_string_value(obj);
	}
	return defval;
}

static size_t GetJsonSize(KonohaContext *kctx, struct JsonBuf *jsonbuf)
{
	if(json_is_array(jsonbuf->jsonobj)) {
		return json_array_size(jsonbuf->jsonobj);
	}
	else if(json_is_object(jsonbuf->jsonobj)) {
		return json_object_size(jsonbuf->jsonobj);
	}
	return 1;
}

static kbool_t RetrieveJsonArrayAt(KonohaContext *kctx, struct JsonBuf *jsonbuf, size_t index, struct JsonBuf *otherbuf)
{
	json_t *obj = json_array_get(jsonbuf->jsonobj, index);
	if(obj != NULL) {
		SetJsonBuf(otherbuf, obj);
		return true;
	}
	return false;
}

static kbool_t SetJsonArrayAt(KonohaContext *kctx, struct JsonBuf *jsonbuf, size_t index, struct JsonBuf *otherbuf)
{
	return (json_array_set(jsonbuf->jsonobj, index, otherbuf->jsonobj) != -1);
}

static kbool_t AppendJsonArray(KonohaContext *kctx, struct JsonBuf *jsonbuf, struct JsonBuf *otherbuf)
{
	return (json_array_append(jsonbuf->jsonobj, otherbuf->jsonobj) != -1);
}

kbool_t LoadJanssonModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"Janson", "0.1", 0, "json jansson",
	};
	factory->JsonModule.JsonDataInfo            = &ModuleInfo;
	factory->JsonModule.IsJsonType             = IsJsonType;
	factory->JsonModule.CreateJson             = CreateJson;
	factory->JsonModule.ParseJson              = ParseJson;
	factory->JsonModule.FreeJson               = FreeJson;
	factory->JsonModule.JsonToNewText          = JsonToNewText;
	factory->JsonModule.DoJsonEach             = DoJsonEach;
	factory->JsonModule.RetrieveJsonKeyValue   = RetrieveJsonKeyValue;
	factory->JsonModule.SetJsonKeyValue        = SetJsonKeyValue;
	factory->JsonModule.SetJsonValue           = SetJsonValue;
	factory->JsonModule.GetJsonBoolean         = GetJsonBoolean;
	factory->JsonModule.GetJsonInt             = GetJsonInt;
	factory->JsonModule.GetJsonFloat           = GetJsonFloat;
	factory->JsonModule.GetJsonText            = GetJsonText;
	factory->JsonModule.GetJsonSize            = GetJsonSize;
	factory->JsonModule.RetrieveJsonArrayAt = RetrieveJsonArrayAt;
	factory->JsonModule.SetJsonArrayAt      = SetJsonArrayAt;
	factory->JsonModule.AppendJsonArray        = AppendJsonArray;
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

