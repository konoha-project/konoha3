/****************************************************************************
 * Copyright (c) 2012-2013, Masahiro Ide <ide@konohascript.org>
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
#include "knumbox.h"
#include "kmap.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef KJSON_HEADER_ONLY
#define KJSON_API static
#else
#define KJSON_API
#endif

#ifndef unlikely
#define unlikely(x)   __builtin_expect(!!(x), 0)
#endif

#ifndef likely
#define likely(x)     __builtin_expect(!!(x), 1)
#endif

typedef enum kjson_type {
    /** ($type & 1 == 0) means $type extends Number */
    JSON_Double   =  0, /* 0b0000 */
    JSON_String   =  1, /* 0b0001 */
    JSON_Int32    =  2, /* 0b0010 */
    JSON_Object   =  3, /* 0b0011 */
    JSON_Bool     =  4, /* 0b0100 */
    JSON_Array    =  5, /* 0b0101 */
    JSON_Null     =  6, /* 0b0110 */
    JSON_UString  =  9, /* 0b1001 */
    JSON_Int64    = 11, /* 0b1011 */
    JSON_Error    = 15, /* 0b1111 */
    JSON_reserved =  7  /* 0b0111 '7' is reserved by numbox */
} kjson_type;

union JSONValue;
typedef union JSONValue JSON;

typedef struct JSONRC {
    long count;
} JSONRC;

#define JSONSTRING_INLINE_SIZE (sizeof(void *)*2)
typedef struct JSONString {
    JSONRC rc;
    const char *str;
    unsigned length;
    unsigned hashcode;
    char text[JSONSTRING_INLINE_SIZE];
} JSONString;
typedef JSONString JSONUString;

DEF_ARRAY_STRUCT0(JSON, unsigned);
DEF_ARRAY_T(JSON);

typedef struct JSONArray {
    JSONRC rc;
    ARRAY(JSON) array;
} JSONArray;

typedef Value JSONNumber;

typedef JSONNumber JSONInt;
typedef JSONInt    JSONInt32;
typedef JSONNumber JSONDouble;
typedef JSONNumber JSONBool;

typedef struct JSONInt64 {
    JSONRC rc;
    int64_t val;
} JSONInt64;

typedef struct JSONObject {
    JSONRC rc;
    kmap_t child;
} JSONObject;

typedef struct JSONError {
    JSONRC rc;
    const char *message;
} JSONError;

union JSONValue {
    Value       val;
    JSONNumber  num;
    JSONInt64  *box;
    JSONString *str;
    JSONArray  *ary;
    JSONObject *obj;
    JSONError  *err;
    uint64_t bits;
};

#ifndef KJSON_MALLOC
#define KJSON_MALLOC(N) malloc(N)
#define KJSON_FREE(PTR) free(PTR)
#endif
DEF_ARRAY_OP_NOPOINTER(JSON);
#undef KJSON_MALLOC
#undef KJSON_FREE

typedef JSONNumber JSONNull;

static inline JSON JSON_NOP(void)
{
    JSON def = {{0}};
    return def;
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

/* JSON Reference Count API */
static inline JSONRC *JSON_Reference(JSON json)
{
    JSONObject *o = toObj(json.val);
    return &o->rc;
}

static inline void JSON_Init(JSON json)
{
    JSONRC *rc = JSON_Reference(json);
    rc->count = 0;
}

static inline void JSON_Retain(JSON json)
{
    JSONRC *rc = JSON_Reference(json);
    rc->count += 1;
}

static inline void JSONObject_Retain(JSON json)
{
    if ((JSON_type(json) & 1) == 1) {
        JSON_Retain(json);
    }
}

static inline void JSON_Release(JSON json)
{
    JSONRC *rc = JSON_Reference(json);
    rc->count -= 1;
}

/* [Getter API] */
KJSON_API JSON JSON_get(JSON json, const char *key, size_t len);
static inline int64_t JSON_getInt(JSON json, const char *key, size_t len)
{
    JSON v = JSON_get(json, key, len);
    return JSON_isValid(v) ?
        (JSON_type(v) == JSON_Int32) ?
            toInt32(v.val) : toInt64(v.val)->val : 0;
}

static inline bool JSON_getBool(JSON json, const char *key, size_t len)
{
    JSON v = JSON_get(json, key, len);
    return JSON_isValid(v) ? toBool(v.val) : 0; }

static inline double JSON_getDouble(JSON json, const char *key, size_t len)
{
    JSON v = JSON_get(json, key, len);
    return JSON_isValid(v) ? toDouble(v.val) : 0;
}

static inline const char *JSON_getString(JSON json, const char *key, size_t *len)
{
    JSON obj = JSON_get(json, key, *len);
    if(!JSON_isValid(obj))
        return NULL;
    JSONString *s = toStr(obj.val);
    *len = s->length;
    return s->str;
}

static inline JSON *JSON_getArray(JSON json, const char *key, size_t *len)
{
    JSON obj = JSON_get(json, key, *len);
    if(!JSON_isValid(obj))
        return NULL;
    JSONArray *a = toAry(obj.val);
    *len = a->array.size;
    return a->array.list;
}

/* [Other API] */
KJSON_API void JSONObject_setObject(JSONMemoryPool *jm, JSON obj, JSON key, JSON value);
KJSON_API void JSONObject_set(JSONMemoryPool *jm, JSON obj, const char *key, size_t len, JSON value);

KJSON_API void JSONObject_removeObject(JSONMemoryPool *jm, JSON json, JSONString *key);
KJSON_API void JSONObject_remove(JSONMemoryPool *jm, JSON json, const char *keyword, size_t keylen);

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

#define JSON_TYPE_CHECK(T, O) (JSON_type(((JSON)O)) == JSON_##T)

typedef struct JSONArray_iterator {
    JSON      *Itr, *End;
    JSONArray *Array;
} JSONArray_iterator;

#define JSON_ARRAY_EACH(json, A, I, E)\
    if(!JSON_type((json)) == JSON_Array) {} else\
        if(!(A = toAry((json).val)) != 0) {}\
        else FOR_EACH_ARRAY(A->array, I, E)

typedef struct JSONObject_iterator {
    long index;
    JSONObject *obj;
} JSONObject_iterator;

#define JSON_OBJECT_EACH(O, ITR, KEY, VAL)\
    if(!JSON_TYPE_CHECK(Object, O)) {} else\
    if(!(JSONObject_iterator_init(&ITR, O))) {}\
    else\
    for(KEY = JSONObject_iterator_next(&ITR, &VAL); KEY.bits;\
            KEY = JSONObject_iterator_next(&ITR, &VAL))

#ifdef __cplusplus
}
#endif

#ifndef LOG2
#define LOG2(N) ((uint32_t)((sizeof(void *) * 8) - __builtin_clzl(N - 1)))
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

static inline int64_t JSONInt64_get(JSON json)
{
    JSONInt64 *i = toInt64(json.val);
    return i->val;
}

static inline double JSONDouble_get(JSON json)
{
    return toDouble(json.val);
}

static inline int JSONBool_get(JSON json)
{
    return toBool(json.val);
}

static inline JSONString *JSONString_init(JSONString *buffer, const char *str, size_t length)
{
    buffer->str      = str;
    buffer->length   = length;
    buffer->hashcode = 0;
    return buffer;
}

/* [New API] */
static inline JSON JSONString_new(JSONMemoryPool *jm, const char *s, size_t len)
{
    bool malloced;
    JSONString *o = (JSONString *) JSONMemoryPool_Alloc(jm, sizeof(*o), &malloced);
    JSON json = toJSON(ValueS(o));
    JSON_Init(json);
    assert(len >= 0);
    char *str = (len > JSONSTRING_INLINE_SIZE) ? (char *) malloc(len) : o->text;
    memcpy(str, s, len);
    JSONString_init(o, (const char *)str, len);
    return json;
}

static inline JSON JSONNull_new()
{
    return toJSON(ValueN());
}

static inline JSON JSONObject_new(JSONMemoryPool *jm, unsigned map_size)
{
    bool malloced;
    JSONObject *o = (JSONObject *) JSONMemoryPool_Alloc(jm, sizeof(*o), &malloced);
    JSON json = toJSON(ValueO(o));
    JSON_Init(json);
    kmap_init(&(o->child), map_size);
    return json;
}

static inline JSON JSONArray_new(JSONMemoryPool *jm, unsigned elm_size)
{
    bool malloced;
    JSONArray *o = (JSONArray *) JSONMemoryPool_Alloc(jm, sizeof(*o), &malloced);
    JSON json = toJSON(ValueA(o));
    JSON_Init(json);
    ARRAY_init(JSON, &o->array, elm_size);
    return json;
}

static inline JSON JSONDouble_new(double val)
{
    return toJSON(ValueF(val));
}

static inline JSON JSONInt32_new(int64_t val)
{
    assert(INT32_MIN < val && val < INT32_MAX);
    return toJSON(ValueI(val));
}

static inline JSON JSONInt_new(JSONMemoryPool *jm, int64_t val)
{
    if(val > INT32_MAX || val < INT32_MIN) {
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
    assert((JSON_type(json) & 0x1) == 0x1);
    return JSON_type(json) == JSON_Object ?
        kmap_size(&toObj(json.val)->child):
        ARRAY_size(toAry(json.val)->array);
}

static inline const char *JSONError_get(JSON json)
{
    return toError(json.val)->message;
}

static inline JSON JSONArray_get(JSON json, unsigned index)
{
    JSONArray *a = toAry(json.val);
    if(JSON_TYPE_CHECK(Array, json) && index < a->array.size) {
        return ARRAY_get(JSON, &a->array, index);
    } else {
        return JSON_NOP();
    }
}
static inline bool JSONArray_set(JSON json, unsigned index, JSON val)
{
    JSONArray *a = toAry(json.val);
    if(JSON_TYPE_CHECK(Array, json) && index < a->array.size) {
        ARRAY_set(JSON, &a->array, index, val);
        return true;
    }
    return false;
}

static inline int JSONObject_iterator_init(JSONObject_iterator *itr, JSON json)
{
    if(!JSON_TYPE_CHECK(Object, json))
        return 0;
    itr->obj = toObj(json.val);
    itr->index = 0;
    return 1;
}

static inline JSON JSONObject_iterator_next(JSONObject_iterator *itr, JSON *val)
{
    JSONObject *o = itr->obj;
    map_record_t *r;
    while((r = kmap_next(&o->child, (kmap_iterator *) itr)) != NULL) {
        JSONString *key = r->k;
        *val = toJSON(ValueP(r->v));
        return toJSON(ValueS(key));
    }
    JSON obj; obj.bits = 0;
    *val = obj;
    return obj;
}

#endif /* end of include guard */
