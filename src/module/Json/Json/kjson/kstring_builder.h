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

#include <string.h>
#include <stdint.h>

#ifndef KJSON_MALLOC
#define KJSON_MALLOC(N) malloc(N)
#define KJSON_FREE(PTR) free(PTR)
#endif

#include "karray.h"

#ifndef KJSON_STRING_BUILDER_H_
#define KJSON_STRING_BUILDER_H_

DEF_ARRAY_STRUCT0(char, unsigned);
DEF_ARRAY_T(char);
DEF_ARRAY_OP_NOPOINTER(char);

typedef struct string_builder {
    ARRAY(char) buf;
} string_builder;

static inline void string_builder_init(string_builder *sb)
{
    ARRAY_init(char, &sb->buf, 4);
}

static inline void string_builder_add_no_check(string_builder *sb, char c)
{
    char *p = sb->buf.list + ARRAY_size(sb->buf);
    *p = c;
    sb->buf.size += 1;
}

static inline void string_builder_add(string_builder *sb, char c)
{
    ARRAY_add(char, &sb->buf, c);
}

static void reverse(char *const start, char *const end)
{
    char *m = start + (end - start) / 2;
    char tmp, *s = start, *e = end - 1;
    while(s < m) {
        tmp  = *s;
        *s++ = *e;
        *e-- = tmp;
    }
}

static inline char toHexChar(uint8_t c)
{
    return c < 10 ? c + '0': c - 10 + 'a';
}

static inline char *put_x(char *p, uint64_t v)
{
    char *base = p;
    do {
        *p++ = toHexChar(v % 16);
    } while((v /= 16) != 0);
    reverse(base, p);
    return p;
}

static inline char *put_d(char *p, uint64_t v)
{
    char *base = p;
    do {
        *p++ = '0' + (v % 10);
    } while((v /= 10) != 0);
    reverse(base, p);
    return p;
}

static inline char *put_i(char *p, int64_t value)
{
    if(value < 0) {
        p[0] = '-'; p++;
        value = -value;
    }
    return put_d(p, (uint64_t)value);
}

static inline void string_builder_add_hex_no_check(string_builder *sb, uint32_t i)
{
    ARRAY_ensureSize(char, &sb->buf, 4/* = sizeof("abcd") */);
    char *p = sb->buf.list + ARRAY_size(sb->buf);
    char *e = put_x(p, i);
    sb->buf.size += e - p;
}

static inline void string_builder_add_int(string_builder *sb, int32_t i)
{
    ARRAY_ensureSize(char, &sb->buf, 12/* = sizeof("-2147483648") */);
    char *p = sb->buf.list + ARRAY_size(sb->buf);
    char *e = put_i(p, i);
    sb->buf.size += e - p;
}

static inline void string_builder_add_int64(string_builder *sb, int64_t i)
{
    ARRAY_ensureSize(char, &sb->buf, 20/* = sizeof("-9223372036854775807") */);
    char *p = sb->buf.list + ARRAY_size(sb->buf);
    char *e = put_i(p, i);
    sb->buf.size += e - p;
}

static inline void string_builder_ensure_size(string_builder *sb, size_t len)
{
    ARRAY_ensureSize(char, &sb->buf, len);
}

static inline void string_builder_add_string_no_check(string_builder *sb, const char *s, size_t len)
{
    char *p = sb->buf.list + ARRAY_size(sb->buf);
#ifdef USE_MEMCPY
    memcpy(p, s, len);
#else
    const char *const e = s + len;
    while(s < e) {
        *p++ = *s++;
    }
#endif
    sb->buf.size += len;
}

static inline void string_builder_add_string(string_builder *sb, const char *s, size_t len)
{
    string_builder_ensure_size(sb, len);
    string_builder_add_string_no_check(sb, s, len);
}

static inline void string_builder_dispose(string_builder *sb)
{
    ARRAY_dispose(char, &sb->buf);
}

static inline char *string_builder_tostring(string_builder *sb,
        size_t *len, int ensureZero)
{
    if(ensureZero) {
        ARRAY_add(char, &sb->buf, '\0');
    }
    char *list = sb->buf.list;
    *len = (size_t) sb->buf.size;
    sb->buf.list     = NULL;
    sb->buf.size     = 0;
    sb->buf.capacity = 0;
    return list;
}

#endif /* end of include guard */
