/****************************************************************************
 * Copyright (c) 2012, Masahiro Ide <ide@konohascript.org>
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

#ifndef KJSON_H_
#define KJSON_H_

#include "kmemory_pool.h"
#include "numbox.h"
#include "kmap.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef KJSON_HEADER_ONLY
#define KJSON_API static
#else
#define KJSON_API
#endif

typedef enum kjson_type {
    /** ($type & 1 == 0) means $type extends Number */
    JSON_Double   =  0, /* 0b00000 */
    JSON_String   =  1, /* 0b00001 */
    JSON_Int32    =  2, /* 0b00010 */
    JSON_Object   =  3, /* 0b00011 */
    JSON_Bool     =  4, /* 0b00100 */
    JSON_Array    =  5, /* 0b00101 */
    JSON_Null     =  6, /* 0b00110 */
    JSON_UString  =  9, /* 0b01001 */
    JSON_Int64    = 11, /* 0b01011 */
    JSON_Error    = 15, /* 0b01111 */
    JSON_reserved =  7  /* 0b00111 '7' is reserved by numbox */
} kjson_type;

union JSONValue;
typedef union JSONValue JSON;

typedef struct JSONString {
    unsigned length;
    unsigned hashcode;
    const char *str;
} JSONString;
typedef JSONString JSONUString;

typedef struct JSONArray {
    int  length;
    int  capacity;
    JSON *list;
} JSONArray;

typedef Value JSONNumber;

typedef JSONNumber JSONInt;
typedef JSONInt    JSONInt32;
typedef JSONNumber JSONDouble;
typedef JSONNumber JSONBool;

typedef struct JSONInt64 {
    kjson_type type;
    int64_t val;
} JSONInt64;

typedef struct JSONObject {
    kmap_t child;
} JSONObject;

union JSONValue {
    Value       val;
    JSONNumber  num;
    JSONString *str;
    JSONArray  *ary;
    JSONObject *obj;
    uint64_t bits;
};

typedef JSONNumber JSONNull;

/* [Getter API] */
KJSON_API JSON *JSON_getArray(JSON json, const char *key, size_t *len);
KJSON_API const char *JSON_getString(JSON json, const char *key, size_t *len);
KJSON_API double JSON_getDouble(JSON json, const char *key, size_t len);
KJSON_API bool JSON_getBool(JSON json, const char *key, size_t len);
KJSON_API int JSON_getInt(JSON json, const char *key, size_t len);
KJSON_API JSON JSON_get(JSON json, const char *key, size_t len);

/* [Other API] */
KJSON_API void JSONObject_set(JSONMemoryPool *jm, JSON obj, JSON key, JSON value);
KJSON_API void JSONArray_append(JSONMemoryPool *jm, JSON ary, JSON o);
KJSON_API void JSON_free(JSON o);

KJSON_API JSON parseJSON(JSONMemoryPool *jm, const char *s, const char *e);
KJSON_API char *JSON_toStringWithLength(JSON json, size_t *len);
static inline char *JSON_toString(JSON json)
{
    size_t len;
    char *s = JSON_toStringWithLength(json, &len);
    return s;
}

static inline bool JSON_isValid(JSON json)
{
    return json.bits != 0;
}

static inline kjson_type JSON_type(JSON json) {
    Value v; v.bits = (uint64_t)json.val.bits;
    uint64_t tag = Tag(v);
    return (IsDouble((v)))?
        JSON_Double : (kjson_type) ((tag >> TagBitShift) & 15);
}

#define JSON_TYPE_CHECK(T, O) (JSON_type(((JSON)O)) == JSON_##T)

#define JSON_ARRAY_EACH(json, A, I, E) JSON_ARRAY_EACH_(json, A, I, E, 0)
#define JSON_ARRAY_EACH_(json, A, I, E, N)\
    if (!JSON_type((json)) == JSON_Array) {} else\
        if (!(A = toAry((json).val)) != 0) {}\
        else\
        for (I = (A)->list + N,\
                E = (A)->list+(A)->length; I != E; ++I)

typedef struct JSONObject_iterator {
    long index;
    JSONObject *obj;
} JSONObject_iterator;

#define JSON_OBJECT_EACH(O, ITR, KEY, VAL)\
    if (!JSON_TYPE_CHECK(Object, O)) {} else\
    if (!(JSONObject_iterator_init(&ITR, O))) {}\
    else\
    for (KEY = JSONObject_iterator_next(&ITR, &VAL); KEY.bits;\
            KEY = JSONObject_iterator_next(&ITR, &VAL))

#ifdef __cplusplus
}
#endif

#ifndef LOG2
#define LOG2(N) ((uint32_t)((sizeof(void*) * 8) - __builtin_clzl(N - 1)))
#endif

#ifdef __cplusplus
static inline JSON toJSON(Value v) {
    JSON json;
    json.bits = v.bits;
    return json;
}
#else
#define toJSON(O) ((JSON) O)
#endif

#ifndef INT32_MAX
#define INT32_MAX        2147483647
#endif

#ifndef INT32_MIN
#define INT32_MIN        (-INT32_MAX-1)
#endif

static inline JSON JSON_parse(JSONMemoryPool *jm, const char *str)
{
    const char *end = str + strlen(str);
    return parseJSON(jm, str, end);
}

/* [Getter API] */
static inline const char *JSONString_get(JSON json)
{
    JSONString *s = toStr(json.val);
    return s->str;
}

static inline unsigned JSONString_length(JSON json)
{
    JSONString *s = toStr(json.val);
    return s->length;
}

static inline int32_t JSONInt_get(JSON json)
{
    return toInt32(json.val);
}

static inline double JSONDouble_get(JSON json)
{
    return toDouble(json.val);
}

static inline int JSONBool_get(JSON json)
{
    return toBool(json.val);
}

static void JSONString_InitHashCode(JSONString *key) { key->hashcode = 0; }

/* [New API] */
static inline JSON JSONString_new(JSONMemoryPool *jm, const char *s, size_t len)
{
    bool malloced;
    JSONString *o = (JSONString *) JSONMemoryPool_Alloc(jm, sizeof(*o), &malloced);
    o->str = (const char *) malloc(len+1);
    o->length = len;
    memcpy((char *)o->str, s, len);
    ((char*)o->str)[len] = 0;
    JSONString_InitHashCode(o);
    return toJSON(ValueS(o));
}

static inline JSON JSONNull_new()
{
    return toJSON(ValueN());
}

static inline JSON JSONObject_new(JSONMemoryPool *jm)
{
    bool malloced;
    JSONObject *o = (JSONObject *) JSONMemoryPool_Alloc(jm, sizeof(*o), &malloced);
    kmap_init(&(o->child), 0);
    return toJSON(ValueO(o));
}

static inline JSON JSONArray_new(JSONMemoryPool *jm)
{
    bool malloced;
    JSONArray *o = (JSONArray *) JSONMemoryPool_Alloc(jm, sizeof(*o), &malloced);
    o->length   = 0;
    o->capacity = 4;
    o->list   = (JSON *) malloc(sizeof(JSON)*4);
    return toJSON(ValueA(o));
}

static inline JSON JSONDouble_new(double val)
{
    return toJSON(ValueF(val));
}

static inline JSON JSONInt_new(JSONMemoryPool *jm, int64_t val)
{
    if (val > INT32_MAX || val < INT32_MIN) {
        bool malloced;
        JSONInt64 *i64 = (JSONInt64 *) JSONMemoryPool_Alloc(jm, sizeof(JSONInt64), &malloced);
        i64->val = val;
        return toJSON(ValueIO(i64));
    } else {
        return toJSON(ValueI(val));
    }
}

static inline JSON JSONBool_new(bool val)
{
    return toJSON(ValueB(val));
}

static inline unsigned JSON_length(JSON json)
{
    assert((JSON_type(json) & 0x3) == 0x1);
    JSONArray *a = toAry(json.val);
    return a->length;
}

static inline JSON JSONArray_get(JSON json, unsigned index)
{
    if (JSON_TYPE_CHECK(Array, json)) {
        JSONArray *a = toAry(json.val);
        return (a)->list[index];
    } else {
        return JSONNull_new();
    }
}

static inline int JSONObject_iterator_init(JSONObject_iterator *itr, JSON json)
{
    if (!JSON_type(json) ==  JSON_Object)
        return 0;
    itr->obj = toObj(json.val);
    itr->index = 0;
    return 1;
}

static inline JSON JSONObject_iterator_next(JSONObject_iterator *itr, JSON *val)
{
    JSONObject *o = itr->obj;
    map_record_t *r;
    while ((r = kmap_next(&o->child, (kmap_iterator*) itr)) != NULL) {
        *val = toJSON(ValueP(r->v));
        return toJSON(ValueS(r->k));
    }
    JSON obj; obj.bits = 0;
    *val = obj;
    return obj;
}

#endif /* end of include guard */
