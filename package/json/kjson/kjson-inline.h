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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#ifndef KJSON_INLINE_H_
#define KJSON_INLINE_H_

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

/* [Getter API] */
static inline const char *JSONString_get(JSON json)
{
    JSONString *s = toStr(json.val);
    return s->str;
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

/* [New API] */
static inline JSON JSONString_new(const char *s, size_t len)
{
    JSONString *o = (JSONString *) KJSON_CALLOC(1, sizeof(*o)+len+1);
    o->str = (const char *) (o+1);
    memcpy((char *)o->str, s, len);
    o->hashcode = 0;
    o->length = len;
    return toJSON(ValueS(o));
}

static inline JSON JSONNull_new()
{
    return toJSON(ValueN());
}

static inline JSON JSONObject_new()
{
    JSONObject *o = (JSONObject *) KJSON_MALLOC(sizeof(*o));
    kmap_init(&(o->child), 0);
    return toJSON(ValueO(o));
}

static inline JSON JSONArray_new()
{
    JSONArray *o = (JSONArray *) KJSON_MALLOC(sizeof(*o));
    o->length   = 0;
    o->capacity = 0;
    o->list   = NULL;
    return toJSON(ValueA(o));
}

static inline JSON JSONDouble_new(double val)
{
    return toJSON(ValueF(val));
}

static inline JSON JSONInt_new(int64_t val)
{
    if (val > (int64_t)INT32_MAX || val < (int64_t)INT32_MIN) {
        JSONInt64 *i64 = (JSONInt64 *) KJSON_MALLOC(sizeof(JSONInt64));
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
