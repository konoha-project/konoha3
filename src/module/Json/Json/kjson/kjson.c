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

#include "kjson.h"
#include "kstream.h"
#include "kstring_builder.h"

#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#undef __SSE2__
#ifdef __SSE2__
#include <emmintrin.h>
#endif

#include "khash.h"

#ifdef __cplusplus
extern "C" {
#endif

#define THROW_IF(COND, EXCEPTION, MESSAGE) do {\
    if(unlikely(COND)) {\
        THROW(&(EXCEPTION), PARSER_EXCEPTION, MESSAGE);\
    }\
} while(0)

static inline bool JSON_CanFree(JSON json)
{
    JSONRC *rc = JSON_Reference(json);
    return rc->count <= 0;
}

static inline void JSON_dispose(JSON json)
{
    JSONRC *rc = JSON_Reference(json);
    rc->count = INT_MAX;
}

static inline JSON JSONError_new(JSONMemoryPool *jm, const char *emessage)
{
    bool malloced;
    JSONError *o = (JSONError *) JSONMemoryPool_Alloc(jm, sizeof(*o), &malloced);
    JSON json = toJSON(ValueE(o));
    JSON_Init(json);
    o->message = emessage;
    return json;
}

static JSON JSONUString_new(JSONMemoryPool *jm, string_builder *builder)
{
    size_t len;
    bool malloced;
    char *s = string_builder_tostring(builder, &len, 1);
    JSONString *o = (JSONString *) JSONMemoryPool_Alloc(jm, sizeof(*o), &malloced);
    if(len <= JSONSTRING_INLINE_SIZE) {
        memcpy(o->text, s, len-1);
        free(s);
        s = o->text;
    }
    JSONString_init(o, s, len-1);
    JSON json = toJSON(ValueU(o));
    JSON_Init(json);
    return json;
}

static unsigned JSONString_hashCode(JSONString *key)
{
    if(!key->hashcode)
        key->hashcode = HASH(key->str, key->length);
    return key->hashcode;
}

static int JSONString_equal(JSONString *k0, JSONString *k1)
{
    if(k0->length != k1->length)
        return 0;
    if(JSONString_hashCode(k0) != JSONString_hashCode(k1))
        return 0;
    if(k0->str[0] != k1->str[0])
        return 0;
    return strncmp(k0->str, k1->str, k0->length) == 0;
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
    JSON_Release(json);
    if (JSON_CanFree(json)) {
        kmap_dispose(&o->child);
        JSON_dispose(json);
        JSONMemoryPool_Free(0, o);
    }
}

static void _JSONString_free(JSONString *obj)
{
    JSON json = toJSON(ValueS(obj));
    JSON_Release(json);
    if (JSON_CanFree(json)) {
        if(obj->length > JSONSTRING_INLINE_SIZE) {
            free((char *)obj->str);
        }
        JSON_dispose(json);
        JSONMemoryPool_Free(0, obj);
    }
}

static void JSONString_free(JSON json)
{
    JSONString *o = toStr(json.val);
    _JSONString_free(o);
}

static void JSONArray_free(JSON json)
{
    JSONArray *a = toAry(json.val);
    JSON_Release(json);
    if (JSON_CanFree(json)) {
        JSON *s, *e;

        FOR_EACH_ARRAY(a->array, s, e) {
            _JSON_free(*s);
        }
        ARRAY_dispose(JSON, &a->array);
        JSON_dispose(json);
        JSONMemoryPool_Free(0, a);
    }
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
    ARRAY_add(JSON, &a->array, o);
}

KJSON_API void JSONArray_append(JSONMemoryPool *jm, JSON json, JSON o)
{
    JSONArray *a = toAry(json.val);
    _JSONArray_append(a, o);
}

static void _JSONObject_set(JSONObject *o, JSONString *key, JSON value)
{
    assert(JSON_type(value) < 16);
    JSONObject_Retain(value);
    kmap_set(&o->child, key, value.bits);
}

KJSON_API void JSONObject_setObject(JSONMemoryPool *jm, JSON json, JSON key, JSON value)
{
    assert(JSON_TYPE_CHECK(Object, json));
    assert(JSON_TYPE_CHECK(String, key));
    JSONObject *o = toObj(json.val);
    _JSONObject_set(o, toStr(key.val), value);
}

KJSON_API void JSONObject_set(JSONMemoryPool *jm, JSON json, const char *keyword, size_t keylen, JSON value)
{
    JSONString *key = toStr(JSONString_new(jm, keyword, keylen).val);
    _JSONObject_set(toObj(json.val), key, value);
}

KJSON_API void JSONObject_removeObject(JSONMemoryPool *jm, JSON json, JSONString *key)
{

    JSONObject *o = toObj(json.val);
    kmap_remove(&o->child, key);
}

KJSON_API void JSONObject_remove(JSONMemoryPool *jm, JSON json, const char *keyword, size_t keylen)
{
    JSONString tmp;
    tmp.str = (char *)keyword;
    tmp.length = keylen;
    tmp.hashcode = 0;
    JSONObject_removeObject(jm, json, &tmp);
}

/* Parser functions */
#define NEXT(ins) string_input_stream_next(ins)
#define EOS(ins)  string_input_stream_eos(ins)
static JSON parseNull(JSONMemoryPool *jm, input_stream *ins, uint8_t c);
static JSON parseNumber(JSONMemoryPool *jm, input_stream *ins, uint8_t c);
static JSON parseBoolean(JSONMemoryPool *jm, input_stream *ins, uint8_t c);
static JSON parseObject(JSONMemoryPool *jm, input_stream *ins, uint8_t c);
static JSON parseArray(JSONMemoryPool *jm, input_stream *ins, uint8_t c);
static JSON parseString(JSONMemoryPool *jm, input_stream *ins, uint8_t c);

static JSON parseNOP(JSONMemoryPool *jm, input_stream *ins, uint8_t c)
{
    THROW(&ins->exception, PARSER_EXCEPTION, "invalid token");
    return JSON_NOP();
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

static uint8_t skip_space(input_stream *ins, uint8_t c)
{
#if 0 && defined(__SSE2__)
#define ffs(x) __builtin_ffsl(x)
    register uint8_t *str = (uint8_t *) (ins->pos - 1);
    const __m128i k0x00 = _mm_set1_epi8(0);
    const __m128i k0x09 = _mm_set1_epi8('\t');
    const __m128i k0x0a = _mm_set1_epi8('\n');
    const __m128i k0x0d = _mm_set1_epi8('\r');
    const __m128i k0x20 = _mm_set1_epi8(' ');
    size_t ip = (size_t) str;
    size_t n = ip & 15;
    assert(c != 0 && c == *str);
    if(n > 0) {
        ip &= ~15;
        __m128i x = _mm_loadu_si128((const __m128i *)ip);
        __m128i mask1 = _mm_or_si128(_mm_cmpeq_epi8(x, k0x09), _mm_cmpeq_epi8(x, k0x0a));
        __m128i mask2 = _mm_or_si128(_mm_cmpeq_epi8(x, k0x0d), _mm_cmpeq_epi8(x, k0x20));
        __m128i mask  = _mm_or_si128(_mm_cmpeq_epi8(x, k0x00), _mm_or_si128(mask1, mask2));
        unsigned short mask3 = _mm_movemask_epi8(mask);
        mask3 = ~(mask3 | ((1UL << n)-1));
        if(mask3) {
            uint8_t *tmp = (uint8_t *)ip + ffs(mask3) - 1;
            ins->pos = tmp + 1;
            return *(tmp);
        }
        str += 16 - n;
    }
    while(1) {
        __m128i x = _mm_loadu_si128((const __m128i *)(str));
        __m128i mask1 = _mm_or_si128(_mm_cmpeq_epi8(x, k0x09), _mm_cmpeq_epi8(x, k0x0a));
        __m128i mask2 = _mm_or_si128(_mm_cmpeq_epi8(x, k0x0d), _mm_cmpeq_epi8(x, k0x20));
        __m128i mask  = _mm_or_si128(_mm_cmpeq_epi8(x, k0x00), _mm_or_si128(mask1, mask2));
        unsigned short mask3 = ~(_mm_movemask_epi8(mask));
        if(mask3) {
            uint8_t *tmp = str + ffs(mask3) - 1;
            ins->pos = tmp + 1;
            return *(tmp);
        }
        str += 16;
    }
    ins->pos = ins->end;
    return 0;
#else
    int ch;
    for(ch = c; EOS(ins); ch = NEXT(ins)) {
        if(!(0x40 & string_table[ch])) {
            return (uint8_t) ch;
        }
    }
    return 0;
#endif
}

static uint8_t skipBSorDoubleQuote(input_stream *ins)
{
#ifdef __SSE2__
#define bsf(x) __builtin_ctzl(x)
    uint8_t *str = (uint8_t *) ins->pos;
    const __m128i k0x00 = _mm_set1_epi8(0);
    const __m128i k0x5c = _mm_set1_epi8('\\');
    const __m128i k0x22 = _mm_set1_epi8('"');
    size_t ip = (size_t) str;
    size_t n = ip & 15;
    if(n > 0) {
        __m128i mask;
        __m128i x = _mm_loadu_si128((const __m128i *)(ip & ~15));
        __m128i result1 = _mm_cmpeq_epi8(x, k0x5c);
        __m128i result2 = _mm_cmpeq_epi8(x, k0x22);
        __m128i result3 = _mm_cmpeq_epi8(x, k0x00);
        mask = _mm_or_si128(result1, result2);
        mask = _mm_or_si128(result3, mask);
        unsigned long mask2 = _mm_movemask_epi8(mask);
        mask2 &= 0xffffffffUL << n;
        if(mask2) {
            uint8_t *tmp = str + bsf(mask2) - n;
            ins->pos = tmp + 1;
            return *tmp;
        }
        str += 16 - n;
    }
    while(1) {
        __m128i x = _mm_loadu_si128((const __m128i *)str);
        __m128i result1 = _mm_cmpeq_epi8(x, k0x5c);
        __m128i result2 = _mm_cmpeq_epi8(x, k0x22);
        __m128i result3 = _mm_cmpeq_epi8(x, k0x00);
        __m128i mask    = _mm_or_si128(result3, _mm_or_si128(result1, result2));
        unsigned long mask2 = _mm_movemask_epi8(mask);
        if(mask2) {
            uint8_t *tmp = str + bsf(mask2);
            ins->pos = tmp + 1;
            return *tmp;
        }
        str += 16;
    }
    ins->pos = ins->end;
    return -1;
#else
    register unsigned ch = NEXT(ins);
    register uint8_t *str;
    register uint8_t *end;
    str = (uint8_t *) ins->pos;
    end = (uint8_t *) ins->end;
    for(; str != end; ch = *str++) {
        if(0x80 & string_table[ch]) {
            break;
        }
    }
    ins->pos = str;
    return ch;
#endif
}

static unsigned toHex(input_stream *ins, uint8_t c)
{
    if(c >= '0' && c <= '9') return c - '0';
    else if(c >= 'a' && c <= 'f') return c - 'a' + 10;
    else if(c >= 'A' && c <= 'F') return c - 'A' + 10;
    THROW(&ins->exception, PARSER_EXCEPTION, "invalid hex digit");
}

static void writeUnicode(unsigned data, string_builder *sb)
{
    if(data <= 0x7f) {
        string_builder_add(sb, (char)data);
    } else if(data <= 0x7ff) {
        string_builder_add(sb, (char)(0xc0 | (data >> 6)));
        string_builder_add(sb, (char)(0x80 | (0x3f & data)));
    } else if(data <= 0xffff) {
        string_builder_add(sb, (char)(0xe0 | (data >> 12)));
        string_builder_add(sb, (char)(0x80 | (0x3f & (data >> 6))));
        string_builder_add(sb, (char)(0x80 | (0x3f & data)));
    } else if(data <= 0x10FFFF) {
        string_builder_add(sb, (char)(0xf0 | (data >> 18)));
        string_builder_add(sb, (char)(0x80 | (0x3f & (data >> 12))));
        string_builder_add(sb, (char)(0x80 | (0x3f & (data >>  6))));
        string_builder_add(sb, (char)(0x80 | (0x3f & data)));
    }
}

static void parseUnicode(input_stream *ins, string_builder *sb)
{
    unsigned data = 0;
    data  = toHex(ins, NEXT(ins)) * 4096; assert(EOS(ins));
    data += toHex(ins, NEXT(ins)) *  256; assert(EOS(ins));
    data += toHex(ins, NEXT(ins)) *   16; assert(EOS(ins));
    data += toHex(ins, NEXT(ins)) *    1; assert(EOS(ins));
    writeUnicode(data, sb);
}

static void parseEscape(input_stream *ins, string_builder *sb, uint8_t c)
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
        default: {
            string_builder_dispose(sb);
            THROW(&ins->exception, PARSER_EXCEPTION, "Illegal escape token");
        }
    }
    string_builder_add(sb, c);
}

static JSON parseString(JSONMemoryPool *jm, input_stream *ins, uint8_t c)
{
    const uint8_t *state1, *state2;
    THROW_IF(c != '"', ins->exception, "Missing open quote at start of JSONString");
    state1 = _input_stream_save(ins);
    c = skipBSorDoubleQuote(ins);
    state2 = _input_stream_save(ins);
    unsigned length = state2 - state1 - 1;
    if(c == '"') {/* fast path */
        return JSONString_new(jm, (char *)state1, length);
    }
    string_builder sb; string_builder_init(&sb);
    if(length > 0) {
        string_builder_add_string(&sb, (const char *) state1, length);
    }
    THROW_IF(c != '\\', ins->exception, "Unexpected Token at middle of JSONString");
    parseEscape(ins, &sb, NEXT(ins));
    c = NEXT(ins);
    for(; EOS(ins); c = NEXT(ins)) {
        switch (c) {
            case '\\':
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
    return JSONUString_new(jm, &sb);
}

static JSON parseChild(JSONMemoryPool *jm, input_stream *ins, uint8_t c)
{
    c = skip_space(ins, c);
    typedef JSON (*parseJSON)(JSONMemoryPool *jm, input_stream *ins, uint8_t c);
    static const parseJSON dispatch_func[] = {
        parseNOP,
        parseObject,
        parseString,
        parseArray,
        parseNumber,
        parseBoolean,
        parseNull
    };
    return dispatch_func[0x7 & string_table[(int)c]](jm, ins, c);
}

static JSON parseObject(JSONMemoryPool *jm, input_stream *ins, uint8_t c)
{
    THROW_IF(c != '{', ins->exception, "Missing open brace '{' at start of json object");
    unsigned stack_top = kstack_size(&ins->stack);
    for(c = skip_space(ins, NEXT(ins)); EOS(ins); c = skip_space(ins, NEXT(ins))) {
        if(c == '}') {
            break;
        }
        THROW_IF(c != '"', ins->exception, "Missing open quote for element key");

        JSON key = parseString(jm, ins, c);
        THROW_IF(key.bits == 0, ins->exception, "JSONObject with extra comma");
        JSONString_hashCode(toStr(key.val));
        kstack_push(&ins->stack, key);
        c = skip_space(ins, NEXT(ins));
        THROW_IF(c != ':', ins->exception, "Missing ':' after key in object");

        JSON val = parseChild(jm, ins, NEXT(ins));
        kstack_push(&ins->stack, val);
        c = skip_space(ins, NEXT(ins));
        if(c == '}') {
            break;
        }
        THROW_IF(c != ',', ins->exception, "Missing comma or end of JSON Object '}'");
    }
    unsigned field_size = (kstack_size(&ins->stack) - stack_top) / 2;
    JSON json = JSONObject_new(jm, field_size);
    JSONObject *obj = toObj(json.val);
    while(field_size-- > 0) {
        JSONString *key;
        JSON val;
        val = kstack_pop(&ins->stack);
        key = toStr(kstack_pop(&ins->stack).val);
        assert(val.bits != 0 && key);
        _JSONObject_set(obj, key, val);
    }
    assert(kstack_size(&ins->stack) == stack_top);
    return json;
}

static JSON parseArray(JSONMemoryPool *jm, input_stream *ins, uint8_t c)
{
    THROW_IF(c != '[', ins->exception, "Missing open brace '[' at start of json array");
    unsigned stack_top = kstack_size(&ins->stack);
    c = skip_space(ins, NEXT(ins));
    if(c == ']') {
        /* array with no elements "[]" */
        return JSONArray_new(jm, 0);
    }
    for(; EOS(ins); c = skip_space(ins, NEXT(ins))) {
        JSON val = parseChild(jm, ins, c);
        THROW_IF(val.bits == 0, ins->exception, "JSONArray with extra comma");
        kstack_push(&ins->stack, val);
        c = skip_space(ins, NEXT(ins));
        if(c == ']') {
            break;
        }
        THROW_IF(c != ',', ins->exception, "Missing comma or end of JSON Array ']'");
    }

    unsigned element_size = kstack_size(&ins->stack) - stack_top;
    JSON json = JSONArray_new(jm, element_size);
    JSONArray *a = toAry(json.val);
    kstack_move(&ins->stack, a->array.list, stack_top, element_size);
    assert(kstack_size(&ins->stack) == stack_top);
    a->array.size += element_size;
    return json;
}

static JSON parseNumber(JSONMemoryPool *jm, input_stream *ins, uint8_t c)
{
    assert((c == '-' || ('0' <= c && c <= '9')) && "It do not seem as Number");
    kjson_type type = JSON_Int32;
    const uint8_t *state1, *state2;
    state1 = _input_stream_save(ins);
    bool negative = false;
    int64_t val = 0;
    JSON n;
    if(c == '-') { negative = true; c = NEXT(ins); }
    if(c == '0') { c = NEXT(ins); }
    else if('1' <= c && c <= '9') {
        for(; '0' <= c && c <= '9' && EOS(ins); c = NEXT(ins)) {
            val = val * 10 + (c - '0');
        }
    }
    if(c != '.' && c != 'e' && c != 'E') {
        goto L_emit;
    }
    if(c == '.') {
        type = JSON_Double;
        for(c = NEXT(ins); '0' <= c && c <= '9' &&
                EOS(ins); c = NEXT(ins)) {}
    }
    if(c == 'e' || c == 'E') {
        type = JSON_Double;
        c = NEXT(ins);
        if(c == '+' || c == '-') {
            c = NEXT(ins);
        }
        for(; '0' <= c && c <= '9' && EOS(ins); c = NEXT(ins)) {}
    }
    L_emit:;
    state2 = _input_stream_save(ins) - 1;
    _input_stream_resume(ins, state2);
    if(type != JSON_Double) {
        val = (negative)? -val : val;
        n = JSONInt_new(jm, val);
    } else {
        char *s = (char *)state1-1;
        char *e = (char *)state2;
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
    uint32_t d = *(uint32_t *) (ins->pos);
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
    uint32_t d = *(uint32_t *) (ins->pos);
    return d == encode4('a', 'l', 's', 'e');
}

static JSON parseBoolean(JSONMemoryPool *jm, input_stream *ins, uint8_t c)
{
    int val = 0;
    if(ins->end - ins->pos >= 3 && checkTrue(ins)) {
        val = 1;
        ins->pos += 3;
    }
    else if(ins->end - ins->pos >= 4 && checkFalse(ins)) {
        ins->pos += 4;
    }
    else {
        THROW(&ins->exception, PARSER_EXCEPTION, "Cannot parse JSON bool variable");
    }
    return JSONBool_new(val);
}

static JSON parseNull(JSONMemoryPool *jm, input_stream *ins, uint8_t c)
{
    if(ins->end - ins->pos >= 3 && checkNull(ins)) {
        ins->pos += 3;
        return JSONNull_new();
    }
    THROW(&ins->exception, PARSER_EXCEPTION, "Cannot parse JSON null variable");
    return JSON_NOP();
}

static JSON parse(JSONMemoryPool *jm, input_stream *ins)
{
    uint8_t c = string_input_stream_next(ins);
    JSON json;
    if((c = skip_space(ins, c)) == 0) {
        return JSONNull_new();
    }
    json = parseChild(jm, ins, c);
    assert(json.bits != 0);
    THROW_IF(IsError(json.val), ins->exception, "Parse Error");
#if 0
    if(EOS(ins)) {
        c = skip_space(ins, NEXT(ins));
        THROW_IF(c != 0, ins->exception, "Rest of token are not parsed");
    }
#endif
    return json;
}

#undef EOS
#undef NEXT

static JSON parseJSON_stream(JSONMemoryPool *jm, input_stream *ins)
{
    return parse(jm, ins);
}

KJSON_API JSON parseJSON(JSONMemoryPool *jm, const char *s, const char *e)
{
    input_stream insbuf;
    input_stream *ins = new_string_input_stream(&insbuf, s, e - s);
    kexception_handler_init(&ins->exception);
    JSON json;
    TRY(ins->exception) {
        json = parseJSON_stream(jm, ins);
    }
    CATCH(PARSER_EXCEPTION) {
        const char *emessage = ins->exception.error_message;
        json = JSONError_new(jm, emessage);
        ins->exception.has_error = 1;
        goto L_finally;
    }
    FINALLY() {
        input_stream_delete(ins);
    }
    return json;
}

static inline JSON JSONObject_get(JSON json, JSONString *key)
{
    JSONObject *o = toObj(json.val);
    map_record_t *r = kmap_get(&o->child, key);
    return (r) ? toJSON(ValueP(r->v)) : JSON_NOP();
}

KJSON_API JSON JSON_get(JSON json, const char *key, size_t len)
{
    JSONString tmp;
    tmp.str = (char *)key;
    tmp.length = len;
    tmp.hashcode = 0;
    return JSONObject_get(json, &tmp);
}

static void _JSONString_toString(string_builder *sb, JSONString *o)
{
    string_builder_ensure_size(sb, o->length+2);
    string_builder_add_no_check(sb, '"');
    string_builder_add_string_no_check(sb, o->str, o->length);
    string_builder_add_no_check(sb, '"');
}

static void _JSON_toString(string_builder *sb, JSON json);

static void JSONObjectElement_toString(string_builder *sb, map_record_t *r)
{
    _JSONString_toString(sb, r->k);
    string_builder_add(sb, ':');
    _JSON_toString(sb, toJSON(ValueP(r->v)));
}

static void JSONObject_toString(string_builder *sb, JSON json)
{
    map_record_t *r;
    kmap_iterator itr = {0};
    string_builder_add(sb, '{');
    JSONObject *o = toObj(json.val);
    if((r = kmap_next(&o->child, &itr)) != NULL) {
        JSONObjectElement_toString(sb, r);
        while((r = kmap_next(&o->child, &itr)) != NULL) {
            string_builder_add(sb, ',');
            JSONObjectElement_toString(sb, r);
        }
    }
    string_builder_add(sb, '}');
}

static void JSONArray_toString(string_builder *sb, JSON json)
{
    JSON *s, *e;
    JSONArray *a = toAry(json.val);
    string_builder_add(sb, '[');
    s = ARRAY_BEGIN(a->array);
    e = ARRAY_END(a->array);
    if(s < e) {
        _JSON_toString(sb, *s++);
        for(; s != e; ++s) {
            string_builder_add(sb, ',');
            _JSON_toString(sb, *s);
        }
    }
    string_builder_add(sb, ']');
}

static void JSONString_toString(string_builder *sb, JSON json)
{
    JSONString *o = toStr(json.val);
    _JSONString_toString(sb, o);
}

static int utf8_check_size(uint8_t s)
{
    uint8_t u = (uint8_t) s;
    assert (u >= 0x80);
    if(0xc2 <= u && u <= 0xdf)
        return 2;
    else if(0xe0 <= u && u <= 0xef)
        return 3;
    else if(0xf0 <= u && u <= 0xf4)
        return 4;
    //assert(0 && "Invalid encoding");
    return 0;
}

static const char *toUTF8(string_builder *sb, const char *s, const char *e)
{
    uint32_t v = 0;
    int i, length = utf8_check_size((uint8_t) (*s));
    if(length == 2) v = *s++ & 0x1f;
    else if(length == 3) v = *s++ & 0xf;
    else if(length == 4) v = *s++ & 0x7;
    for(i = 1; i < length && s < e; ++i) {
        uint8_t tmp = (uint8_t) *s++;
        if(tmp < 0x80 || tmp > 0xbf) {
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
    string_builder_add_no_check(sb, '"');
    while(s < e) {
        uint8_t c = *s;
        string_builder_ensure_size(sb, 8);
        if(c & 0x80) {
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
    string_builder_add_int64(sb, o->val);
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
    if(len) {
        *len = length;
    }
    return str;
}

#ifdef __cplusplus
}
#endif

#include "kmap.c"
