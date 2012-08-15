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

/* ************************************************************************ */

#define USE_STRINGLIB 1

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/klib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SPOL_sub(s)     (S_isASCII(s) ? SPOL_ASCII : 0)
#define S_msize(s)      text_mlen(S_text(s), S_size(s))
#define S_length(s)     (S_isASCII(s) ? S_size(s) : S_msize(s))
#define CT_StringArray0 CT_p0(kctx, CT_Array, TY_String)
#define S_index(s, n)   ((n < 0) ? S_length(s) + n : n)

/* FIXME: needs throw? */
#define S_range(s, n)   ((n < 0) ? 0 : ((n > S_length(s)) ? S_length(s) : n))

/* ------------------------------------------------------------------------ */

static size_t text_mlen(const char *s_text, size_t s_size)
{
#ifdef K_USING_UTF8
	size_t size = 0;
	const unsigned char *start = (const unsigned char*)s_text;
	const unsigned char *end = start + s_size;
	while (start < end) {
		size_t ulen = utf8len(start[0]);
		size++;
		start += ulen;
	}
	return size;
#else
	return s_size;
#endif
}

static kString *S_mget(KonohaContext *kctx, kString *s, size_t n)
{
	DBG_ASSERT(!S_isASCII(s));
	kString *ret = NULL;
	const unsigned char *text = (const unsigned char *)S_text(s);
	const unsigned char *start = text;
	size_t size = S_size(s);
	size_t i;

	if ((int)n < 0) {
		return ret;
	}
	for (i = 0; i < n; i++) {
		start += utf8len(start[0]);
	}
	if (start < text + size) {
		const unsigned char *end = start;
		end += utf8len(end[0]);
		ret = KLIB new_kString(kctx, (const char *)start, end - start, SPOL_POOL|SPOL_UTF8);
	}
	return ret;
}

static kint_t S_mcharCodeAt(KonohaContext *kctx, kString *s, size_t n)
{
	const unsigned char *text = (const unsigned char *)S_text(s);
	const unsigned char *start = text;
	size_t i;

	if((int)n < 0) {
		return -1;
	}
	for(i = 0; i < n; i++) {
		start += utf8len(start[0]);
	}
	if(start < text + S_size(s)) {
		const unsigned char *buf = start;
		unsigned short ret = 0;
		i = utf8len(buf[0]);
		switch(i) {
		case 4:
			ret = ((buf[0] & 7) << 18) | ((buf[1] & 63) << 12) |
				((buf[2] & 63) << 6) | (buf[3] & 63);
			break;
		case 3:
			ret = ((buf[0] & 63) << 12) | ((buf[1] & 63) << 6) |
				(buf[2] & 63);
			break;
		case 2:
			ret = ((buf[0] & 63) << 6) | (buf[1] & 63);
			break;
		case 1:
			ret = buf[0];
			break;
		default:
			assert(0);
		}
		return ret;
	}
	return -1;
}

static kString *S_msubstr(KonohaContext *kctx, kString *s, size_t moff, size_t mlen)
{
	DBG_ASSERT(!S_isASCII(s));
	const unsigned char *text = (const unsigned char *)S_text(s);
	const unsigned char *start = text;
	size_t size = S_size(s);
	kString *ret = NULL;
	size_t i;

	if ((int)moff < 0 || moff > size) {
		return ret;
	}
	for (i = 0; i < moff; i++) {
		start += utf8len(start[0]);
	}
	if (start < text + size) {
		const unsigned char *end = NULL;
		if ((int)mlen <= 0 || mlen > size) {
			end = text + size;
			ret = KLIB new_kString(kctx, (const char *)start, end - start, SPOL_POOL|SPOL_UTF8);
		}
		else {
			end = start;
			for (i = 0; i < mlen; i++) {
				end += utf8len(end[0]);
			}
			if (end < text + size) {
				ret = KLIB new_kString(kctx, (const char *)start, end - start, SPOL_POOL|SPOL_UTF8);
			}
			else {
				ret = KLIB new_kString(kctx, (const char *)start, text + size - start, SPOL_POOL|SPOL_UTF8);
			}
		}
	}
	return ret;
}

static kString* S_toupper(KonohaContext *kctx, kString *s0, size_t start)
{
	size_t i, size = S_size(s0);
	kString *s = KLIB new_kString(kctx, NULL, size, SPOL_sub(s0)|SPOL_NOCOPY);
	memcpy(s->buf, s0->buf, size);
	for(i = start; i < size; i++) {
		int ch = s->buf[i];
		if('a' <= ch && ch <= 'z') {
			s->buf[i] = toupper(ch);
		}
	}
	return s;
}

static kString* S_tolower(KonohaContext *kctx, kString *s0, size_t start)
{
	size_t i, size = S_size(s0);
	kString *s = KLIB new_kString(kctx, NULL, size, SPOL_sub(s0)|SPOL_NOCOPY);
	memcpy(s->buf, s0->buf, size);
	for(i = start; i < size; i++) {
		int ch = s->buf[i];
		if('A' <= ch && ch <= 'Z') {
			s->buf[i] = tolower(ch);
		}
	}
	return s;
}

static kString *S_substring(KonohaContext *kctx, KonohaStack *sfp, kString *s0, size_t start, size_t length)
{
	kString *ret = NULL;
	if (S_isASCII(s0)) {
		start = check_index(kctx, start, S_size(s0), sfp[K_RTNIDX].uline);
		const char *new_text = S_text(s0) + start;
		ret = KLIB new_kString(kctx, new_text, length, SPOL_ASCII|SPOL_POOL); // FIXME SPOL
	}
	else {
		ret = S_msubstr(kctx, s0, start, length);
		if (unlikely(ret == NULL)) {
			kreportf(CritTag, sfp[K_RTNIDX].uline, "Script!!: out of array index %ld", sfp[1].intValue);
			KLIB Kraise(kctx, EXPT_("OutOfStringBoundary"), sfp, sfp[K_RTNIDX].uline);
		}
	}
	return ret;
}

/* // The function below must not use for ASCII string (nakata) */
/* static kString *new_MultiByteSubString(KonohaContext *kctx, kString *s, size_t moff, size_t mlen) */
/* { */
/* 	DBG_ASSERT(!S_isASCII(s)); */
/* 	const unsigned char *start = (unsigned char *)S_text(s); */
/* 	const unsigned char *itr = start; */
/* 	size_t i; */
/* 	for(i = 0; i < moff; i++) { */
/* 		itr += utf8len(itr[0]); */
/* 	} */
/* 	start = itr; */
/* 	for(i = 0; i < mlen; i++) { */
/* 		itr += utf8len(itr[0]); */
/* 	} */
/* 	size_t len = itr - start; */
/* 	s = KLIB new_kString(kctx, (const char *)start, len, SPOL_UTF8); */
/* 	return s; */
/* } */


/* ------------------------------------------------------------------------ */
//## method @Const Int String.getSize();

static KMETHOD String_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	RETURNi_(S_length(s0));
}

/* ------------------------------------------------------------------------ */
//## @Const method Boolean String.startsWith(String s);

static KMETHOD String_startsWith(KonohaContext *kctx, KonohaStack *sfp)
{
	// @TEST "A".startsWith("ABC");
	RETURNb_(strncmp(S_text(sfp[0].asString), S_text(sfp[1].asString), S_size(sfp[1].asString)) == 0);
}

/* ------------------------------------------------------------------------ */
//## @Const method Boolean String.endsWith(String s);

static KMETHOD String_endsWith(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 =  sfp[1].asString;
	int ret;
	if (S_size(s0) < S_size(s1)) {
		ret = 0;
	}
	else {
		const char *p = S_text(s0) + (S_size(s0) - S_size(s1));
		ret = (strncmp(p, S_text(s1), S_size(s1)) == 0);
	}
	RETURNb_(ret);
}

/* ------------------------------------------------------------------------ */
//## @Const method Int String.indexOf(String searchvalue);

static KMETHOD String_indexOf(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString, *s1 = sfp[1].asString;
	if (S_size(s1) > 0) {
		long loc = -1;
		const char *t0 = S_text(s0);
		const char *t1 = S_text(s1);
		char *p = strstr(t0, t1);
		if (p != NULL) {
			loc = p - t0;
			if (!S_isASCII(s0)) {
				loc = text_mlen(t0, (size_t)loc);
			}
		}
		RETURNi_(loc);
	}
	RETURNi_(0);
}

/* ------------------------------------------------------------------------ */
//## @Const method Int String.indexOf(String searchvalue, Int start);

static KMETHOD String_indexOfwithStart(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString, *s1 = sfp[1].asString;
	kint_t start = sfp[2].intValue;
	start = S_range(s0, start);
	if(S_size(s1) > 0) {
		long loc = -1;
		const char *t0 = S_text(s0);
		t0 += text_mlen(t0, (size_t)start);
		const char *t1 = S_text(s1);
		char *p = strstr(t0, t1);
		if (p != NULL) {
			loc = p - t0;
			if (!S_isASCII(s0)) {
				loc = text_mlen(t0, (size_t)loc);
			}
		}
		RETURNi_(loc + start);
	}
	RETURNi_(start);
}

/* ------------------------------------------------------------------------ */
//## @Const method Int String.lastIndexOf(String searchvalue);

static KMETHOD String_lastIndexOf(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	const char *t0 = S_text(s0);
	const char *t1 = S_text(s1);
	intptr_t loc = S_size(s0) - S_size(s1);
	int len = S_size(s1);
	if(S_size(s1) == 0) loc--;
	for(; loc >= 0; loc--) {
		if(t0[loc] == t1[0]) {
			if(strncmp(t0 + loc, t1, len) == 0) break;
		}
	}
	if (loc >= 0 && !S_isASCII(s0)) {
		loc = text_mlen(t0, (size_t)loc);
	}
	RETURNi_(loc);
}

/* ------------------------------------------------------------------------ */
//## @Const method Int String.lastIndexOf(String searchvalue, Int start);

static KMETHOD String_lastIndexOfwithStart(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	kint_t start = S_range(s0, sfp[2].intValue);
	const char *t0 = S_text(s0);
	const char *t1 = S_text(s1);
	intptr_t loc = text_mlen(t0, (size_t)start) - S_size(s1);
	int len = S_size(s1);
	if(S_size(s1) == 0) loc--;
	for(; loc >= 0; loc--) {
		if(t0[loc] == t1[0]) {
			if(strncmp(t0 + loc, t1, len) == 0) break;
		}
	}
	if (loc >= 0 && !S_isASCII(s0)) {
		loc = text_mlen(t0, (size_t)loc);
	}
	RETURNi_(loc);
}

/* ------------------------------------------------------------------------ */
//## @Const method String String.replace(String searchvalue, String newvalue);

static KMETHOD String_replace(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *searchvalue = sfp[1].asString;
	kString *newvalue = sfp[2].asString;
	const char *start = S_text(s0);
	const char *end = start + S_size(s0);
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	if(S_size(searchvalue) == 0) {
		KLIB Kwb_write(kctx, &wb, S_text(newvalue), S_size(newvalue));
	}
	else {
		while(start < end) {
			char *res = strstr(start, S_text(searchvalue));
			if(res == NULL) break;
			if(res - start > 0) {
				KLIB Kwb_write(kctx, &wb, start, res - start);
			}
			KLIB Kwb_write(kctx, &wb, S_text(newvalue), S_size(newvalue));
			start = res + S_size(searchvalue);
		}
	}
	KLIB Kwb_write(kctx, &wb, start, strlen(start)); // write out remaining string
	RETURN_(KLIB new_kString(kctx, KLIB Kwb_top(kctx, &wb, 0), Kwb_bytesize(&wb), SPOL_sub(s0)|SPOL_POOL));
}

/* ------------------------------------------------------------------------ */
//## @Const method String String.trim();

static KMETHOD String_trim(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *s = S_text(sfp[0].asString);
	int len = S_size(sfp[0].asString);
	kString *ret = NULL;
	while(isspace(s[0])) {
		s++;
		len--;
	}
	if(len != 0) {
		while(isspace(s[len-1])) {
			len--;
			if(len == 0) break;
		}
	}
	if(S_size(sfp[0].asString) > len) {
		ret = KLIB new_kString(kctx, s, len, SPOL_sub(sfp[0].asString));
	}
	else {
		ret = sfp[0].asString;
	}
	RETURN_(ret);
}

/* ------------------------------------------------------------------------ */
//## @Const method Boolean String.opHAS(String s);

static KMETHOD String_opHAS(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(strstr(S_text(sfp[0].asString), S_text(sfp[1].asString)) != NULL);
}

/* ------------------------------------------------------------------------ */
//## @Const method String String.get(Int n);

static KMETHOD String_get(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s = sfp[0].asString;
	size_t n = (size_t)sfp[1].intValue;
	if (S_isASCII(s)) {
		n = check_index(kctx, sfp[1].intValue, S_size(s), sfp[K_RTNIDX].uline);
		s = KLIB new_kString(kctx, S_text(s) + n, 1, SPOL_POOL|SPOL_ASCII);
	}
	else {
		s = S_mget(kctx, s, n);
		if (unlikely(s == NULL)) {
			kreportf(CritTag, sfp[K_RTNIDX].uline, "Script!!: out of array index %ld", (int)n);
			KLIB Kraise(kctx, EXPT_("OutOfStringBoundary"), sfp, sfp[K_RTNIDX].uline);
		}
	}
	RETURN_(s);
}

/* ------------------------------------------------------------------------ */
//## @Const method Int String.charCodeAt(Int n);

static KMETHOD String_charCodeAt(KonohaContext *kctx, KonohaStack *sfp)
{
	size_t n = (size_t)sfp[1].intValue;
	kint_t ccode = S_mcharCodeAt(kctx, sfp[0].asString, n);
	if (unlikely(ccode == -1)) {
		kreportf(CritTag, sfp[K_RTNIDX].uline, "Script!!: out of array index %ld", (int)n);
		KLIB Kraise(kctx, EXPT_("OutOfStringBoundary"), sfp, sfp[K_RTNIDX].uline);
	}
	RETURNi_(ccode);
}

/* ------------------------------------------------------------------------ */
//## @Const method String String.slice(Int start);
//## @Const method String String.substr(Int start);
/* Microsoft's JScript does not support negative values for the start index. */

static KMETHOD String_substr(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	size_t start = S_index(s0, sfp[1].intValue);
	kint_t length = S_length(s0);
	RETURN_(S_substring(kctx, sfp, s0, start, length));
}

/* ------------------------------------------------------------------------ */
//## @Const method String String.substr(Int start, Int length);
/* Microsoft's JScript does not support negative values for the start index. */

static KMETHOD String_substrwithLength(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	size_t start = S_index(s0, sfp[1].intValue);
	kint_t length = S_range(s0, sfp[2].intValue);
	if(length == 0) {
		RETURN_(KNULL(String));
	}
	RETURN_(S_substring(kctx, sfp, s0, start, length));
}

/* ------------------------------------------------------------------------ */
//## @Const method String String.toUpper();

static KMETHOD String_toUpper(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	size_t i, size = S_size(s0);
	for(i = 0; i < size; i++) {
		int ch = s0->buf[i];
		if('a' <= ch && ch <= 'z') {
			RETURN_(S_toupper(kctx, s0, i));
		}
	}
	RETURN_(s0);
}

/* ------------------------------------------------------------------------ */
//## @Const method String String.toLower();

static KMETHOD String_toLower(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	size_t i, size = S_size(s0);
	for(i = 0; i < size; i++) {
		int ch = s0->buf[i];
		if('A' <= ch && ch <= 'Z') {
			RETURN_(S_tolower(kctx, s0, i));
		}
	}
	RETURN_(s0);
}

/* ------------------------------------------------------------------------ */
//## @Const method String String.fromCharCode(Int n);

static KMETHOD String_fromCharCode(KonohaContext *kctx, KonohaStack *sfp)
{
	kint_t c = sfp[1].intValue;
	if(c < 0x0000 || c > 0x10FFFF) { /* FIXME: out of unicode range */
		RETURN_(KNULL(String));
	}
	char buf[5] = {0};
	size_t length = 0;
	int policy = 0;
	if(c <= 0x007F) { /* 1 byte */
		buf[0] = (char)c;
		length = 1;
		policy |= SPOL_ASCII;
	}
	else if(c <= 0x07FF) { /* 2 bytes */
		buf[0] = (char)(0xC0 | (c >> 6));
		buf[1] = (char)(0x80 | (c & 0x3F));
		length = 2;
		policy |= SPOL_UTF8;
	}
	else if(c <= 0xFFFF) { /* 3 bytes */
		buf[0] = (char)(0xE0 | (c >> 12));
		buf[1] = (char)(0x80 | ((c >> 6) & 0x3F));
		buf[2] = (char)(0x80 | (c & 0x3F));
		length = 3;
		policy |= SPOL_UTF8;
	}
	else { /* 4 bytes */
		buf[0] = (char)(0xF0 | (c >> 18));
		buf[1] = (char)(0x80 | ((c >> 12) & 0x3F));
		buf[2] = (char)(0x80 | ((c >> 6) & 0x3F));
		buf[3] = (char)(0x80 | (c & 0x3F));
		length = 4;
		policy |= SPOL_UTF8;
	}
	RETURN_(KLIB new_kString(kctx, (const char *)buf, length, policy));
}

/* ------------------------------------------------------------------------ */
//## @Const method String String.slice(Int start, Int end);

static KMETHOD String_slice(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kint_t start = S_index(s0, sfp[1].intValue);
	kint_t end = S_index(s0, sfp[2].intValue);
	end = check_index(kctx, end, S_length(s0), sfp[K_RTNIDX].uline);
	size_t length = end - start;
	RETURN_(S_substring(kctx, sfp, s0, start, length));
}

/* ------------------------------------------------------------------------ */
//## @Const method String[] String.split();

static KMETHOD String_split(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = (kArray*)KLIB new_kObject(kctx, CT_StringArray0, 0);
	KLIB kArray_add(kctx, a, sfp[0].asString);
	RETURN_(a);
}

/* ------------------------------------------------------------------------ */
//## @Const method String[] String.split(String separator);

static KMETHOD String_splitwithSeparator(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *separator = sfp[1].asString;
	kArray *a = (kArray*)KLIB new_kObject(kctx, CT_StringArray0, 0);
	const char *start = S_text(s0);
	const char *end = start + S_size(s0);
	if(S_size(separator) == 0) {
		size_t i;
		for(i = 0; i < S_size(s0); i++) {
			if(S_isASCII(s0)) {
				KLIB kArray_add(kctx, a, KLIB new_kString(kctx, S_text(s0) + i, 1, SPOL_POOL|SPOL_ASCII));
			}
			else {
				KLIB kArray_add(kctx, a, S_mget(kctx, s0, i));
			}
		}
	}
	else {
		while(start < end) {
			char *res = strstr(start, S_text(separator));
			if(res == NULL) break;
			if(res - start > 0) {
				KLIB kArray_add(kctx, a, KLIB new_kString(kctx, start, res - start, SPOL_sub(s0)|SPOL_POOL));
			}
			start = res + S_size(separator);
		}
		KLIB kArray_add(kctx, a, KLIB new_kString(kctx, start, strlen(start), SPOL_sub(s0)|SPOL_ASCII));
	}
	RETURN_(a);
}

/* ------------------------------------------------------------------------ */
//## @Const method String[] String.split(String separator, Int limit);

static KMETHOD String_splitwithSeparatorLimit(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *separator = sfp[1].asString;
	kint_t limit = sfp[2].intValue;
	size_t length = S_length(s0);
	kArray *a = (kArray*)KLIB new_kObject(kctx, CT_StringArray0, 0);
	const char *start = S_text(s0);
	const char *end = start + S_size(s0);
	if(S_size(separator) == 0) {
		size_t i;
		if(limit > length) {
			limit = length;
		}
		for(i = 0; i < limit; i++) {
			if(S_isASCII(s0)) {
				KLIB kArray_add(kctx, a, KLIB new_kString(kctx, S_text(s0) + i, 1, SPOL_POOL|SPOL_ASCII));
			}
			else {
				KLIB kArray_add(kctx, a, S_mget(kctx, s0, i));
			}
		}
	}
	else {
		length = 0;
		while(start < end && length < limit) {
			char *res = strstr(start, S_text(separator));
			if(res == NULL) break;
			if(res - start > 0) {
				KLIB kArray_add(kctx, a, KLIB new_kString(kctx, start, res - start, SPOL_sub(s0)|SPOL_POOL));
				length++;
			}
			start = res + S_size(separator);
		}
		if(length < limit - 1) {
			KLIB kArray_add(kctx, a, KLIB new_kString(kctx, start, strlen(start), SPOL_sub(s0)|SPOL_ASCII));
		}
	}
	RETURN_(a);
}

/* ------------------------------------------------------------------------ */
//## @Const method String String.substring(Int indexA);

static KMETHOD String_substring(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	size_t indexA = S_range(s0, sfp[1].intValue);
	size_t length = S_length(s0);
	RETURN_(S_substring(kctx, sfp, s0, indexA, length));
}

/* ------------------------------------------------------------------------ */
//## @Const method String String.substring(Int indexA, Int indexB);

static KMETHOD String_substringwithIndexB(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	size_t indexA = S_range(s0, sfp[1].intValue);
	size_t indexB = S_range(s0, sfp[2].intValue);
	if(indexA > indexB) {
		/* swap */
		size_t tmp = indexA;
		indexA = indexB;
		indexB = tmp;
	}
	size_t length = indexB - indexA;
	RETURN_(S_substring(kctx, sfp, s0, indexA, length));
}

/* ------------------------------------------------------------------------ */
//## @Const method String String.valueOf();

static KMETHOD String_valueOf(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(sfp[0].asString);
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Static   kMethod_Static
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _Im kMethod_Immutable
#define _F(F)   (intptr_t)(F)

static kbool_t string_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	int FN_s = FN_("s");
	int FN_n = FN_("n");
	ktype_t TY_StringArray0 = CT_StringArray0->typeId;
	kMethod* concat = KLIB kNameSpace_getMethodNULL(kctx, ns, TY_String, MN_("+"), 0, MPOL_FIRST);
	KDEFINE_METHOD MethodData[] = {
		/*JS*/_Public|_Const|_Im, _F(String_getSize), TY_Int, TY_String, MN_("getLength"), 0,
		/*JS*/_Public|_Const|_Im, _F(String_get), TY_String, TY_String, MN_("charAt"), 1, TY_Int, FN_("index"),
		/*JS*/_Public|_Const|_Im, _F(String_charCodeAt), TY_Int, TY_String, MN_("charCodeAt"), 1, TY_Int, FN_("index"),
		/*JS*/_Public|_Const|_Im, _F(concat->invokeMethodFunc), TY_String, TY_String, MN_("concat"), 1, TY_String, FN_s,
		/*JS*/_Public|_Static|_Const|_Im, _F(String_fromCharCode), TY_String, TY_String, MN_("fromCharCode"), 1, TY_Int, FN_n,
		/*JS*/_Public|_Const|_Im, _F(String_indexOf), TY_Int, TY_String, MN_("indexOf"), 1, TY_String, FN_("searchvalue"),
		/*JS*/_Public|_Const|_Im, _F(String_indexOfwithStart), TY_Int, TY_String, MN_("indexOf"), 2, TY_String, FN_("searchvalue"), TY_Int, FN_("start"),
		/*JS*/_Public|_Const|_Im, _F(String_lastIndexOf), TY_Int, TY_String, MN_("lastIndexOf"), 1, TY_String, FN_("searchvalue"),
		/*JS*/_Public|_Const|_Im, _F(String_lastIndexOfwithStart), TY_Int, TY_String, MN_("lastIndexOf"), 2, TY_String, FN_("searchvalue"), TY_Int, FN_("start"),
		/*JS*/_Public|_Const|_Im, _F(String_replace), TY_String, TY_String, MN_("replace"), 2, TY_String, FN_("searchvalue"), TY_String, FN_("newvalue"),
		/*JS*/_Public|_Const|_Im, _F(String_indexOf), TY_Int, TY_String, MN_("search"), 1, TY_String, FN_("searchvalue"),
		/*JS*/_Public|_Const|_Im, _F(String_substr), TY_String, TY_String, MN_("slice"), 1, TY_Int, FN_("start"),
		/*JS*/_Public|_Const|_Im, _F(String_slice), TY_String, TY_String, MN_("slice"), 2, TY_Int, FN_("start"), TY_Int, FN_("end"),
		/*JS*/_Public|_Im, _F(String_split), TY_StringArray0, TY_String, MN_("split"), 0,
		/*JS*/_Public|_Im, _F(String_splitwithSeparator), TY_StringArray0, TY_String, MN_("split"), 1, TY_String, FN_("separator"),
		/*JS*/_Public|_Im, _F(String_splitwithSeparatorLimit), TY_StringArray0, TY_String, MN_("split"), 2, TY_String, FN_("separator"), TY_Int, FN_("limit"),
		/*JS*/_Public|_Const|_Im, _F(String_substr), TY_String, TY_String, MN_("substr"), 1, TY_Int, FN_("start"),
		/*JS*/_Public|_Const|_Im, _F(String_substrwithLength), TY_String, TY_String, MN_("substr"), 2, TY_Int, FN_("start"), TY_Int, FN_("length"),
		/*JS*/_Public|_Const|_Im, _F(String_substring), TY_String, TY_String, MN_("substring"), 1, TY_Int, FN_("indexA"),
		/*JS*/_Public|_Const|_Im, _F(String_substringwithIndexB), TY_String, TY_String, MN_("substring"), 2, TY_Int, FN_("indexA"), TY_Int, FN_("indexB"),
		/*JS*/_Public|_Const|_Im, _F(String_toLower), TY_String, TY_String, MN_("toLowerCase"), 0,
		/*JS*/_Public|_Const|_Im, _F(String_toUpper), TY_String, TY_String, MN_("toUpperCase"), 0,
		/*JS*/_Public|_Const|_Im, _F(String_valueOf), TY_String, TY_String, MN_("valueOf"), 0,
		_Public|_Const|_Im, _F(String_opHAS),      TY_Boolean, TY_String, MN_("opHAS"), 1, TY_String, FN_s,
		_Public|_Const|_Im, _F(String_trim),       TY_String, TY_String, MN_("trim"), 0,
		_Public|_Const|_Im, _F(String_get),        TY_String, TY_String, MN_("get"), 1, TY_Int, FN_n,
		_Public|_Const|_Im, _F(String_startsWith), TY_Boolean, TY_String, MN_("startsWith"), 1, TY_String, FN_s,
		_Public|_Const|_Im, _F(String_endsWith),   TY_Boolean, TY_String, MN_("endsWith"),   1, TY_String, FN_s,
		_Public|_Const|_Im, _F(String_getSize),    TY_Int, TY_String, MN_("getSize"), 0,
		_Public|_Const|_Im, _F(String_toUpper),    TY_String, TY_String, MN_("toUpper"), 0,
		_Public|_Const|_Im, _F(String_toLower),    TY_String, TY_String, MN_("toLower"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t string_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t string_initNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t string_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* string_init(void)
{
	static const KDEFINE_PACKAGE d = {
		KPACKNAME("String", "1.0"),
		.initPackage = string_initPackage,
		.setupPackage = string_setupPackage,
		.initNameSpace = string_initNameSpace,
		.setupNameSpace = string_setupNameSpace,
	};
	return &d;
}
#ifdef __cplusplus
}
#endif

