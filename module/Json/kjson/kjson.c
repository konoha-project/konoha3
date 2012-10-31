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

#include "kjson.h"
#include "kstream.h"
#include "kstring_builder.h"

#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#ifdef __SSE2__
#include <emmintrin.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

static JSON JSONString_new2(JSONMemoryPool *jm, string_builder *builder)
{
    size_t len;
    char *s = string_builder_tostring(builder, &len, 1);
    len -= 1;
    bool malloced;
    JSONString *o = (JSONString *) JSONMemoryPool_Alloc(jm, sizeof(*o), &malloced);
    o->length = len;
    o->str = (const char *) malloc(len);
    memcpy((char *) o->str, s, len);
    JSONString_InitHashCode(o);
    KJSON_FREE(s);
    return toJSON(ValueU(o));
}

static void _JSON_free(JSON o);
static void JSONNOP_free(JSON o) {}
KJSON_API void JSON_free(JSON o)
{
    _JSON_free(o);
}

static void JSONObject_free(JSON json)
{
    JSONObject *o = toObj(json.val);
    kmap_dispose(&o->child);
}

static void _JSONString_free(JSONString *obj)
{
    free((char *)obj->str);
}

static void JSONString_free(JSON json)
{
    JSONString *o = toStr(json.val);
    _JSONString_free(o);
}

static void JSONArray_free(JSON json)
{
    JSONArray *a;
    JSON *s, *e;
    JSON_ARRAY_EACH_(json, a, s, e, 0) {
        _JSON_free(*s);
    }

    free(a->list);
}

#define JSON_OP(OP)\
    OP(/* 00 */JSONDouble)\
    OP(/* 01 */JSONString)\
    OP(/* 02 */JSONInt32)\
    OP(/* 03 */JSONObject)\
    OP(/* 04 */JSONBool)\
    OP(/* 05 */JSONArray)\
    OP(/* 06 */JSONNull)\
    OP(/* 07 */JSONDouble)\
    OP(/* 08 */JSONNOP)\
    OP(/* 09 */JSONUString)\
    OP(/* 10 */JSONNOP)\
    OP(/* 11 */JSONInt64)\
    OP(/* 12 */JSONNOP)\
    OP(/* 13 */JSONNOP)\
    OP(/* 14 */JSONNOP)\
    OP(/* 15 */JSONNOP)

#define JSONInt64_free   JSONNOP_free
#define JSONDouble_free  JSONNOP_free
#define JSONInt32_free   JSONNOP_free
#define JSONBool_free    JSONNOP_free
#define JSONNull_free    JSONNOP_free
#define JSONUString_free JSONString_free

static void _JSON_free(JSON o)
{
    kjson_type type = JSON_type(o);
    typedef void (*freeJSON)(JSON);
    static const freeJSON dispatch_free[] = {
#define JSON_FREE(T) T##_free,
        JSON_OP(JSON_FREE)
#undef JSON_FREE
    };
    dispatch_free[type](o);
}

static void _JSONArray_append(JSONArray *a, JSON o)
{
    if (a->length + 1 >= a->capacity) {
        uint32_t newsize = 1 << LOG2(a->capacity * 2 + 1);
        a->list = (JSON*) realloc(a->list, newsize * sizeof(JSON));
        a->capacity = newsize;
    }
    a->list[a->length++] = o;
}

KJSON_API void JSONArray_append(JSONMemoryPool *jm, JSON json, JSON o)
{
    JSONArray *a = toAry(json.val);
    _JSONArray_append(a, o);
}

static void _JSONObject_set(JSONObject *o, JSONString *key, JSON value)
{
    assert(JSON_type(value) < 16);
    kmap_set(&o->child, key, value.bits);
}

KJSON_API void JSONObject_set(JSONMemoryPool *jm, JSON json, JSON key, JSON value)
{
    assert(JSON_TYPE_CHECK(Object, json));
    assert(JSON_TYPE_CHECK(String, key));
    JSONObject *o = toObj(json.val);
    _JSONObject_set(o, toStr(key.val), value);
}

/* Parser functions */
#define NEXT(ins) string_input_stream_next(ins)
#define EOS(ins)  string_input_stream_eos(ins)
static JSON parseNull(JSONMemoryPool *jm, input_stream *ins, unsigned char c);
static JSON parseNumber(JSONMemoryPool *jm, input_stream *ins, unsigned char c);
static JSON parseBoolean(JSONMemoryPool *jm, input_stream *ins, unsigned char c);
static JSON parseObject(JSONMemoryPool *jm, input_stream *ins, unsigned char c);
static JSON parseArray(JSONMemoryPool *jm, input_stream *ins, unsigned char c);
static JSON parseString(JSONMemoryPool *jm, input_stream *ins, unsigned char c);

static JSON parseNOP(JSONMemoryPool *jm, input_stream *ins, unsigned char c)
{
    JSON o; o.bits = 0;
    return o;
}

#define _N 0x40 |
#define _M 0x80 |
static const unsigned string_table[] = {
    0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,_N 0,_N 0,0   ,0   ,_N 0,0   ,0,
    0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0,
    _N 0,0   ,_M 2,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,4   ,0   ,0,
    4   ,4   ,4   ,4   ,4   ,4   ,4   ,4   ,4   ,4   ,0   ,0   ,0   ,0   ,0   ,0,
    0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0,
    0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,3   ,_M 0,0   ,0   ,0,
    0   ,0   ,0   ,0   ,0   ,0   ,5   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,6   ,0,
    0   ,0   ,0   ,0   ,5   ,0   ,0   ,0   ,0   ,0   ,0   ,1   ,0   ,0   ,0   ,0,
    0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0,
    0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0,
    0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0,
    0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0,
    0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0,
    0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0,
    0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0,
    0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0
};
#undef _M
#undef _N

static unsigned char skip_space(input_stream *ins, unsigned char c)
{
#if 0
//#ifdef __SSE2__
//#define ffs(x) __builtin_ffsl(x)
//    register unsigned char *str = (unsigned char *) (ins->d0.str - 1);
//    const __m128i m0x00 = _mm_set1_epi8(0);
//    const __m128i m0x09 = _mm_set1_epi8('\t');
//    const __m128i m0x0a = _mm_set1_epi8('\n');
//    const __m128i m0x0d = _mm_set1_epi8('\r');
//    const __m128i m0x20 = _mm_set1_epi8(' ');
//    size_t ip = (size_t) str;
//    size_t n = ip & 15;
//    assert(c != 0 && c == *str);
//    if (n > 0) {
//        ip &= ~15;
//        __m128i x = _mm_loadu_si128((const __m128i*)ip);
//        __m128i mask1 = _mm_or_si128(_mm_cmpeq_epi8(x, m0x09), _mm_cmpeq_epi8(x, m0x0a));
//        __m128i mask2 = _mm_or_si128(_mm_cmpeq_epi8(x, m0x0d), _mm_cmpeq_epi8(x, m0x20));
//        __m128i mask  = _mm_or_si128(_mm_cmpeq_epi8(x, m0x00), _mm_or_si128(mask1, mask2));
//        unsigned short mask3 = _mm_movemask_epi8(mask);
//        mask3 = ~(mask3 | ((1UL << n)-1));
//        if (mask3) {
//            unsigned char *tmp = (unsigned char *)ip + ffs(mask3) - 1;
//            ins->d0.str = tmp + 1;
//            return *(tmp);
//        }
//        str += 16 - n;
//    }
//    while (1) {
//        __m128i x = _mm_loadu_si128((const __m128i*)(str));
//        __m128i mask1 = _mm_or_si128(_mm_cmpeq_epi8(x, m0x09), _mm_cmpeq_epi8(x, m0x0a));
//        __m128i mask2 = _mm_or_si128(_mm_cmpeq_epi8(x, m0x0d), _mm_cmpeq_epi8(x, m0x20));
//        __m128i mask  = _mm_or_si128(_mm_cmpeq_epi8(x, m0x00), _mm_or_si128(mask1, mask2));
//        unsigned short mask3 = ~(_mm_movemask_epi8(mask));
//        if (mask3) {
//            unsigned char *tmp = str + ffs(mask3) - 1;
//            ins->d0.str = tmp + 1;
//            return *(tmp);
//        }
//        str += 16;
//    }
//    ins->d0.str = ins->d1.str;
//    return 0;
#else
    int ch;
    for (ch = c; EOS(ins); ch = NEXT(ins)) {
        assert(ch >= 0);
        if (!(0x40 & string_table[ch])) {
            return (unsigned char) ch;
        }
    }
    return 0;
#endif
}

static unsigned char skipBSorDoubleQuote(input_stream *ins)
{
#ifdef __SSE2__
#define bsf(x) __builtin_ctzl(x)
    unsigned char *str = (unsigned char *) ins->d0.str;
    const __m128i m0x00 = _mm_set1_epi8(0);
    const __m128i m0x5c = _mm_set1_epi8('\\');
    const __m128i m0x22 = _mm_set1_epi8('"');
    size_t ip = (size_t) str;
    size_t n = ip & 15;
    if (n > 0) {
        __m128i mask;
        __m128i x = _mm_loadu_si128((const __m128i*)(ip & ~15));
        __m128i result1 = _mm_cmpeq_epi8(x, m0x5c);
        __m128i result2 = _mm_cmpeq_epi8(x, m0x22);
        __m128i result3 = _mm_cmpeq_epi8(x, m0x00);
        mask = _mm_or_si128(result1, result2);
        mask = _mm_or_si128(result3, mask);
        unsigned long mask2 = _mm_movemask_epi8(mask);
        mask2 &= 0xffffffffUL << n;
        if (mask2) {
            unsigned char *tmp = str + bsf(mask2) - n;
            ins->d0.str = tmp + 1;
            return *tmp;
        }
        str += 16 - n;
    }
    while (1) {
        __m128i x = _mm_loadu_si128((const __m128i*)str);
        __m128i result1 = _mm_cmpeq_epi8(x, m0x5c);
        __m128i result2 = _mm_cmpeq_epi8(x, m0x22);
        __m128i result3 = _mm_cmpeq_epi8(x, m0x00);
        __m128i mask    = _mm_or_si128(result3, _mm_or_si128(result1, result2));
        unsigned long mask2 = _mm_movemask_epi8(mask);
        if (mask2) {
            unsigned char *tmp = str + bsf(mask2);
            ins->d0.str = tmp + 1;
            return *tmp;
        }
        str += 16;
    }
    ins->d0.str = ins->d1.str;
    return -1;
#else
    register unsigned ch = NEXT(ins);
    register unsigned char *str;
    register unsigned char *end;
    str = (unsigned char *) ins->d0.str;
    end = (unsigned char *) ins->d1.str;
    for(; str != end; ch = *str++) {
        if (0x80 & string_table[ch]) {
            break;
        }
    }
    ins->d0.str = str;
    return ch;
#endif
}

static unsigned int toHex(unsigned char c)
{
    return (c >= '0' && c <= '9') ? c - '0' :
        (c >= 'a' && c <= 'f') ? c - 'a' + 10:
        (c >= 'A' && c <= 'F') ? c - 'A' + 10:
        (assert(0 && "invalid hex digit"), 0);
}

static void writeUnicode(unsigned int data, string_builder *sb)
{
    if (data <= 0x7f) {
        string_builder_add(sb, (char)data);
    } else if (data <= 0x7ff) {
        string_builder_add(sb, (char)(0xc0 | (data >> 6)));
        string_builder_add(sb, (char)(0x80 | (0x3f & data)));
    } else if (data <= 0xffff) {
        string_builder_add(sb, (char)(0xe0 | (data >> 12)));
        string_builder_add(sb, (char)(0x80 | (0x3f & (data >> 6))));
        string_builder_add(sb, (char)(0x80 | (0x3f & data)));
    } else if (data <= 0x10FFFF) {
        string_builder_add(sb, (char)(0xf0 | (data >> 18)));
        string_builder_add(sb, (char)(0x80 | (0x3f & (data >> 12))));
        string_builder_add(sb, (char)(0x80 | (0x3f & (data >>  6))));
        string_builder_add(sb, (char)(0x80 | (0x3f & data)));
    }
}

static void parseUnicode(input_stream *ins, string_builder *sb)
{
    unsigned int data = 0;
    data  = toHex(NEXT(ins)) * 4096; assert(EOS(ins));
    data += toHex(NEXT(ins)) *  256; assert(EOS(ins));
    data += toHex(NEXT(ins)) *   16; assert(EOS(ins));
    data += toHex(NEXT(ins)) *    1; assert(EOS(ins));
    writeUnicode(data, sb);
}

static void parseEscape(input_stream *ins, string_builder *sb, unsigned char c)
{
    switch (c) {
        case '"':  c = '"';  break;
        case '\\': c = '\\'; break;
        case '/': c = '/';  break;
        case 'b': c = '\b';  break;
        case 'f': c = '\f';  break;
        case 'n': c = '\n';  break;
        case 'r': c = '\r';  break;
        case 't': c = '\t';  break;
        case 'u': parseUnicode(ins, sb); return;
        default: assert(0 && "Unknown espace");
    }
    string_builder_add(sb, c);
}

static JSON parseString(JSONMemoryPool *jm, input_stream *ins, unsigned char c)
{
    union io_data state, state2;
    assert(c == '"' && "Missing open quote at start of JSONString");
    state = _input_stream_save(ins);
    c = skipBSorDoubleQuote(ins);
    state2 = _input_stream_save(ins);
    if (c == '"') {/* fast path */
        return JSONString_new(jm, (char *)state.str, state2.str - state.str - 1);
    }
    string_builder sb; string_builder_init(&sb);
    if (state2.str - state.str - 1 > 0) {
        string_builder_add_string(&sb, (const char *) state.str,
                state2.str - state.str - 1);
    }
    assert(c == '\\');
    goto L_escape;
    for(; EOS(ins); c = NEXT(ins)) {
        switch (c) {
            case '\\':
            L_escape:;
                parseEscape(ins, &sb, NEXT(ins));
                continue;
            case '"':
                goto L_end;
            default:
                break;
        }
        string_builder_add(&sb, c);
    }
    L_end:;
    return JSONString_new2(jm, &sb);
}

static JSON parseChild(JSONMemoryPool *jm, input_stream *ins, unsigned char c)
{
    c = skip_space(ins, c);
    typedef JSON (*parseJSON)(JSONMemoryPool *jm, input_stream *ins, unsigned char c);
    static const parseJSON dispatch_func[] = {
        parseNOP,
        parseObject,
        parseString,
        parseArray,
        parseNumber,
        parseBoolean,
        parseNull};
    return dispatch_func[0x7 & string_table[(int)c]](jm, ins, c);
}

static JSON parseObject(JSONMemoryPool *jm, input_stream *ins, unsigned char c)
{
    assert(c == '{' && "Missing open brace '{' at start of json object");
    JSON json = JSONObject_new(jm);
    JSONObject *obj = toObj(json.val);
    for (c = skip_space(ins, NEXT(ins)); EOS(ins); c = skip_space(ins, NEXT(ins))) {
        JSONString *key = NULL;
        JSON val;
        if (c == '}') {
            break;
        }
        assert(c == '"' && "Missing open quote for element key");
        key = toStr(parseString(jm, ins, c).val);
        c = skip_space(ins, NEXT(ins));
        assert(c == ':' && "Missing ':' after key in object");
        val = parseChild(jm, ins, NEXT(ins));
        _JSONObject_set(obj, key, val);
        c = skip_space(ins, NEXT(ins));
        if (c == '}') {
            break;
        }
        assert(c == ',' && "Missing comma or end of JSON Object '}'");
    }
    return json;
}

static JSON parseArray(JSONMemoryPool *jm, input_stream *ins, unsigned char c)
{
    JSON json = JSONArray_new(jm);
    JSONArray *a = toAry(json.val);
    assert(c == '[' && "Missing open brace '[' at start of json array");
    c = skip_space(ins, NEXT(ins));
    if (c == ']') {
        /* array with no elements "[]" */
        return json;
    }
    for (; EOS(ins); c = skip_space(ins, NEXT(ins))) {
        JSON val = parseChild(jm, ins, c);
        _JSONArray_append(a, val);
        c = skip_space(ins, NEXT(ins));
        if (c == ']') {
            break;
        }
        assert(c == ',' && "Missing comma or end of JSON Array ']'");
    }
    return json;
}

static JSON parseNumber(JSONMemoryPool *jm, input_stream *ins, unsigned char c)
{
    assert((c == '-' || ('0' <= c && c <= '9')) && "It do not seem as Number");
    kjson_type type = JSON_Int32;
    union io_data state, state2;
    state = _input_stream_save(ins);
    bool negative = false;
    int64_t val = 0;
    JSON n;
    if (c == '-') { negative = true; c = NEXT(ins); }
    if (c == '0') { c = NEXT(ins); }
    else if ('1' <= c && c <= '9') {
        for (; '0' <= c && c <= '9' && EOS(ins); c = NEXT(ins)) {
            val = val * 10 + (c - '0');
        }
    }
    if (c != '.' && c != 'e' && c != 'E') {
        goto L_emit;
    }
    if (c == '.') {
        type = JSON_Double;
        for (c = NEXT(ins); '0' <= c && c <= '9' &&
                EOS(ins); c = NEXT(ins)) {}
    }
    if (c == 'e' || c == 'E') {
        type = JSON_Double;
        c = NEXT(ins);
        if (c == '+' || c == '-') {
            c = NEXT(ins);
        }
        for (; '0' <= c && c <= '9' && EOS(ins); c = NEXT(ins)) {}
    }
    L_emit:;
    state2 = _input_stream_save(ins);
    state2.str -= 1;
    _input_stream_resume(ins, state2);
    if (type != JSON_Double) {
        val = (negative)? -val : val;
        n = JSONInt_new(jm, val);
    } else {
        char *s = (char *)state.str-1;
        char *e = (char *)state2.str;
        double d = strtod(s, &e);
        n = JSONDouble_new(d);
    }
    return n;
}

static inline uint32_t encode4(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    return d << 24 | c << 16 | b << 8 | a;
}

static inline int check3(input_stream *ins, uint8_t a, uint8_t b, uint8_t c)
{
    uint32_t d = *(uint32_t *) (ins->d0.str);
    return (((1 << 24)-1) & d) == encode4(a, b, c, 0);
}

static int checkNull(input_stream *ins)
{
    return check3(ins, 'u', 'l', 'l');
}

static int checkTrue(input_stream *ins)
{
    return check3(ins, 'r', 'u', 'e');
}

static int checkFalse(input_stream *ins)
{
    uint32_t d = *(uint32_t *) (ins->d0.str);
    return d == encode4('a', 'l', 's', 'e');
}

static JSON parseBoolean(JSONMemoryPool *jm, input_stream *ins, unsigned char c)
{
    int val = 0;
    if (ins->d1.str - ins->d0.str >= 3 && checkTrue(ins)) {
        val = 1;
        ins->d0.str += 3;
    }
    else if (ins->d1.str - ins->d0.str >= 4 && checkFalse(ins)) {
        ins->d0.str += 4;
    }
    else {
        assert(0 && "Cannot parse JSON bool variable");
    }
    return JSONBool_new(val);
}

static JSON parseNull(JSONMemoryPool *jm, input_stream *ins, unsigned char c)
{
    if (ins->d1.str - ins->d0.str >= 3 && checkNull(ins)) {
        ins->d0.str += 3;
        return JSONNull_new();
    }
    assert(0 && "Cannot parse JSON null variable");
    JSON o; o.bits = 0;
    return o;
}

static JSON parse(JSONMemoryPool *jm, input_stream *ins)
{
    unsigned char c = 0;
    for_each_istream(ins, c) {
        JSON json;
        if ((c = skip_space(ins, c)) == 0) {
            break;
        }
        json = parseChild(jm, ins, c);
        if (json.obj != NULL)
            return json;
    }
    JSON o; o.bits = 0;
    return o;
}

#undef EOS
#undef NEXT

static const JSON JSON_default = {{0}};

static JSON parseJSON_stream(JSONMemoryPool *jm, input_stream *ins)
{
    return parse(jm, ins);
}

KJSON_API JSON parseJSON(JSONMemoryPool *jm, const char *s, const char *e)
{
    input_stream insbuf;
    input_stream *ins = new_string_input_stream(&insbuf, s, e - s);
    JSON json = parseJSON_stream(jm, ins);
    input_stream_delete(ins);
    return json;
}

static JSON _JSON_get(JSON json, const char *key, size_t len)
{
    JSONObject *o = toObj(json.val);

    struct JSONString tmp;
    tmp.str = (char *)key;
    tmp.length = len;
    JSONString_InitHashCode(&tmp);
    map_record_t *r = kmap_get(&o->child, &tmp);
    return (r) ? toJSON(ValueP(r->v)) : JSON_default;
}

KJSON_API JSON JSON_get(JSON json, const char *key, size_t len)
{
    return _JSON_get(json, key, len);
}

KJSON_API int JSON_getInt(JSON json, const char *key, size_t len)
{
    JSON v = _JSON_get(json, key, len);
    return toInt32(v.val);
}

KJSON_API bool JSON_getBool(JSON json, const char *key, size_t len)
{
    JSON v = _JSON_get(json, key, len);
    return toBool(v.val);
}

KJSON_API double JSON_getDouble(JSON json, const char *key, size_t len)
{
    JSON v = _JSON_get(json, key, len);
    return toDouble(v.val);
}

KJSON_API const char *JSON_getString(JSON json, const char *key, size_t *len)
{
    JSON obj = _JSON_get(json, key, *len);
    JSONString *s = toStr(obj.val);
    *len = s->length;
    return s->str;
}

KJSON_API JSON *JSON_getArray(JSON json, const char *key, size_t *len)
{
    JSON obj = _JSON_get(json, key, *len);
    JSONArray *a = toAry(obj.val);
    *len = a->length;
    return a->list;
}

static void _JSONString_toString(string_builder *sb, JSONString *o)
{
    string_builder_ensure_size(sb, o->length+2);
    string_builder_add_no_check(sb, '"');
    string_builder_add_string_no_check(sb, o->str, o->length);
    string_builder_add_no_check(sb, '"');
}

static void _JSON_toString(string_builder *sb, JSON json);

static void JSONObject_toString(string_builder *sb, JSON json)
{
    map_record_t *r;
    kmap_iterator itr = {0};
    string_builder_add(sb, '{');
    JSONObject *o = toObj(json.val);
    if ((r = kmap_next(&o->child, &itr)) != NULL) {
        goto L_internal;
        while ((r = kmap_next(&o->child, &itr)) != NULL) {
            string_builder_add(sb, ',');
            L_internal:
            _JSONString_toString(sb, r->k);
            string_builder_add(sb, ':');
            _JSON_toString(sb, toJSON(ValueP(r->v)));
        }
    }
    string_builder_add(sb, '}');
}

static void JSONArray_toString(string_builder *sb, JSON json)
{
    JSON *s, *e;
    JSONArray *a = toAry(json.val);
    string_builder_add(sb, '[');
    s = (a)->list;
    e = (a)->list+(a)->length;
    if (s < e)
        goto L_internal;
    for (; s < e; ++s) {
        string_builder_add(sb, ',');
        L_internal:
        _JSON_toString(sb, *s);
    }
    string_builder_add(sb, ']');
}

static void JSONString_toString(string_builder *sb, JSON json)
{
    JSONString *o = toStr(json.val);
    _JSONString_toString(sb, o);
}

static int utf8_check_size(unsigned char s)
{
    uint8_t u = (uint8_t) s;
    assert (u >= 0x80);
    if (0xc2 <= u && u <= 0xdf)
        return 2;
    else if (0xe0 <= u && u <= 0xef)
        return 3;
    else if (0xf0 <= u && u <= 0xf4)
        return 4;
    //assert(0 && "Invalid encoding");
    return 0;
}

static const char *toUTF8(string_builder *sb, const char *s, const char *e)
{
    uint32_t v = 0;
    int i, length = utf8_check_size((unsigned char) (*s));
    if (length == 2) v = *s++ & 0x1f;
    else if (length == 3) v = *s++ & 0xf;
    else if (length == 4) v = *s++ & 0x7;
    for (i = 1; i < length && s < e; ++i) {
        uint8_t tmp = (uint8_t) *s++;
        if (tmp < 0x80 || tmp > 0xbf) {
            return 0;
        }
        v = (v << 6) | (tmp & 0x3f);
    }
    string_builder_add_hex_no_check(sb, v);
    return s;
}

static void JSONUString_toString(string_builder *sb, JSON json)
{
    JSONString *o = toStr(json.val);
    const char *s = o->str;
    const char *e = o->str + o->length;
    string_builder_ensure_size(sb, o->length+2/* = strlen("\"\") */);
    string_builder_add(sb, '"');
    while (s < e) {
        unsigned char c = *s;
        string_builder_ensure_size(sb, 8);
        if (c & 0x80) {
            string_builder_add_string_no_check(sb, "\\u", 2);
            s = toUTF8(sb, s, e);
            continue;
        }
        c = *s++;
        switch (c) {
            case '"':  string_builder_add_string_no_check(sb, "\\\"", 2); break;
            case '\\': string_builder_add_string_no_check(sb, "\\\\", 2); break;
            case '/':  string_builder_add_string_no_check(sb, "\\/" , 2); break;
            case '\b': string_builder_add_string_no_check(sb, "\\b", 2); break;
            case '\f': string_builder_add_string_no_check(sb, "\\f", 2); break;
            case '\n': string_builder_add_string_no_check(sb, "\\n", 2); break;
            case '\r': string_builder_add_string_no_check(sb, "\\r", 2); break;
            case '\t': string_builder_add_string_no_check(sb, "\\t", 2); break;
            default:   string_builder_add_no_check(sb, c);
        }
    }
    string_builder_add_no_check(sb, '"');
}

static void JSONInt32_toString(string_builder *sb, JSON json)
{
    int32_t i = toInt32(json.val);
    string_builder_add_int(sb, i);
}
static void JSONInt64_toString(string_builder *sb, JSON json)
{
    JSONInt64 *o = toInt64(json.val);
    string_builder_add_int(sb, o->val);
}

static void JSONDouble_toString(string_builder *sb, JSON json)
{
    double d = toDouble(json.val);
    char buf[64];
    int len = snprintf(buf, 64, "%g", d);
    string_builder_add_string(sb, buf, len);
}

static void JSONBool_toString(string_builder *sb, JSON json)
{
    string_builder_ensure_size(sb, 5);
    if(toBool(json.val)) {
        string_builder_add_string_no_check(sb, "true", 4);
    } else {
        string_builder_add_string_no_check(sb, "false", 5);
    }
}

static void JSONNull_toString(string_builder *sb, JSON json)
{
    string_builder_add_string(sb, "null", 4);
}

static void JSONNOP_toString(string_builder *sb, JSON json)
{
    assert(0 && "Invalid type");
}

static void _JSON_toString(string_builder *sb, JSON json)
{
    kjson_type type = JSON_type(json);
    typedef void (*toString)(string_builder *sb, JSON);
    static const toString dispatch_toStr[] = {
#define TOSTRING(T) T##_toString,
        JSON_OP(TOSTRING)
#undef TOSTRING
    };
    dispatch_toStr[type](sb, json);
}

KJSON_API char *JSON_toStringWithLength(JSON json, size_t *len)
{
    char *str;
    size_t length;
    string_builder sb;

    string_builder_init(&sb);
    _JSON_toString(&sb, json);
    str = string_builder_tostring(&sb, &length, 1);
    if (len) {
        *len = length;
    }
    return str;
}

#ifdef __cplusplus
}
#endif

#include "kmap.c"
