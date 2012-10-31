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

#include <stdint.h>
#include <stdbool.h>

#ifndef KJSON_NUMBOX_H_
#define KJSON_NUMBOX_H_

#define TagMask     (0xffff800000000000LL)
#define TagBitShift (47)
#define TagBaseMask (0x1fff0ULL)
#define TagBase     (TagBaseMask << TagBitShift)

enum NumBoxTag {
    TagDouble   = (TagBaseMask | 0x0ULL) << TagBitShift,
    TagDouble2  = (TagBaseMask | 0x7ULL) << TagBitShift,
    TagInt32    = (TagBaseMask | 0x2ULL) << TagBitShift,
    TagBoolean  = (TagBaseMask | 0x4ULL) << TagBitShift,
    TagNull     = (TagBaseMask | 0x6ULL) << TagBitShift,
    TagObject   = (TagBaseMask | 0x3ULL) << TagBitShift,
    TagString   = (TagBaseMask | 0x1ULL) << TagBitShift,
    TagUString  = (TagBaseMask | 0x9ULL) << TagBitShift,
    TagArray    = (TagBaseMask | 0x5ULL) << TagBitShift,
    TagInt64    = (TagBaseMask | 0xbULL) << TagBitShift
};

union JSONValue;
struct JSONObject;
struct JSONString;
struct JSONArray;
struct JSONInt64;

typedef union numbox {
    void    *pval;
    double   dval;
    int32_t  ival;
    bool     bval;
    uint64_t bits;
} Value;

static inline uint64_t toU64(long v) {
    return (uint64_t) v;
}

static inline Value ValueF(double d) {
    Value v; v.dval = d; return v;
}

static inline Value ValueI(int32_t ival) {
    uint64_t n = (uint64_t)ival;
    n = n & 0x00000000ffffffffLL;
    Value v; v.bits = n | TagInt32; return v;
}

static inline Value ValueIO(struct JSONInt64 *oval) {
    Value v; v.bits = toU64((long)oval) | TagInt64; return v;
}
static inline Value ValueB(bool bval) {
    Value v; v.bits = (uint64_t)bval | TagBoolean; return v;
}
static inline Value ValueO(struct JSONObject *oval) {
    Value v; v.bits = toU64((long)oval) | TagObject; return v;
}
static inline Value ValueS(struct JSONString *sval) {
    Value v; v.bits = toU64((long)sval) | TagString; return v;
}
static inline Value ValueU(struct JSONString *sval) {
    Value v; v.bits = toU64((long)sval) | TagUString; return v;
}
static inline Value ValueA(struct JSONArray *aval) {
    Value v; v.bits = toU64((long)aval) | TagArray; return v;
}
static inline Value ValueN() {
    Value v; v.bits = TagNull; return v;
}
static inline Value ValueP(uint64_t bits) {
    Value v; v.bits = bits; return v;
}
static inline uint64_t Tag(Value v) { return (v.bits &  TagMask); }
static inline uint64_t Val(Value v) { return (v.bits & ~TagMask); }
static inline double toDouble(Value v) {
    return v.dval;
}
static inline int32_t toInt32(Value v) {
    return (int32_t) Val(v);
}
static inline long toLong(Value v) {
    return (long) Val(v);
}
static inline bool toBool(Value v) {
    return (bool) Val(v);
}
static inline struct JSONObject *toObj(Value v) {
    return (struct JSONObject *) toLong(v);
}
static inline struct JSONString *toStr(Value v) {
    return (struct JSONString *) toLong(v);
}
static inline struct JSONString *toUStr(Value v) {
    return (struct JSONString *) toLong(v);
}
static inline struct JSONArray *toAry(Value v) {
    return (struct JSONArray *) toLong(v);
}
static inline struct JSONInt64 *toInt64(Value v) {
    return (struct JSONInt64 *) toLong(v);
}
static inline bool IsDouble(Value v) {
    return Tag(v) <= TagDouble;
}
static inline bool IsInt32(Value v) {
    return Tag(v) == TagInt32;
}
static inline bool IsBool(Value v) {
    return Tag(v) == TagBoolean;
}
static inline bool IsObj(Value v) {
    return Tag(v) == TagObject;
}
static inline bool IsStr(Value v) {
    return Tag(v) == TagString;
}
static inline bool IsAry(Value v) {
    return Tag(v) == TagArray;
}
static inline bool IsNull(Value v) {
    return Tag(v) == TagNull;
}
#endif /* end of include guard */
