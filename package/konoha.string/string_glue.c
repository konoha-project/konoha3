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
/*
 * Bit encoding for Rope String
 * ObjectHeader's magicflag space
 * [StringType]    | MSB ------------------------ LSB
 *                 | 10987654321098765432109876543210
 * (not-allowed)   | xxxxxxxxxxxxxxxxxxxxxx000xxxxxxx
 * LinerString     | xxxxxxxxxxxxxxxxxxxxxx001xxxxxxx
 * ExterenalString | xxxxxxxxxxxxxxxxxxxxxx011xxxxxxx
 * InlinedString   | xxxxxxxxxxxxxxxxxxxxxx010xxxxxxx
 * RopeString      | xxxxxxxxxxxxxxxxxxxxxx100xxxxxxx
 * ASCII-String    | xxxxxxxxxxxxxxxxxxxxxxxxx1xxxxxx
 * Pooled-String   | xxxxxxxxxxxxxxxxxxxxxxxxxx1xxxxx
 */
#define S_isRope(o)          (TFLAG_is(uintptr_t,(o)->h.magicflag,kObject_Local1))

#define S_FLAG_MASK_BASE (7)
#define S_FLAG_LINER     ((1UL << (0)))
#define S_FLAG_NOFREE    ((1UL << (1)))
#define S_FLAG_ROPE      ((1UL << (2)))
#define S_FLAG_INLINE    (S_FLAG_NOFREE)
#define S_FLAG_EXTERNAL  (S_FLAG_LINER | S_FLAG_NOFREE)

#define MASK_LINER    ((S_FLAG_LINER   ) << S_FLAG_MASK_BASE)
#define MASK_NOFREE   ((S_FLAG_NOFREE  ) << S_FLAG_MASK_BASE)
#define MASK_ROPE     ((S_FLAG_ROPE    ) << S_FLAG_MASK_BASE)
#define MASK_INLINE   ((S_FLAG_INLINE  ) << S_FLAG_MASK_BASE)
#define MASK_EXTERNAL ((S_FLAG_EXTERNAL) << S_FLAG_MASK_BASE)

#define S_len(s) ((s)->length)
typedef struct StringBase {
	KonohaObjectHeader h;
	size_t length;
} StringBase;

typedef struct LinerString {
	StringBase base;
	char *text;
} LinerString;

typedef struct ExternalString {
	struct StringBase base;
	char *text;
} ExternalString;

typedef struct RopeString {
	struct StringBase base;
	struct StringBase *left;
	struct StringBase *right;
} RopeString;

typedef struct InlineString {
	struct StringBase base;
	char *text;
	char inline_text[SIZEOF_INLINETEXT];
} InlineString;

static inline uint32_t S_flag(StringBase *s)
{
	uint32_t flag = ((~0U) & (s)->h.magicflag) >> S_FLAG_MASK_BASE;
	assert(flag <= S_FLAG_ROPE);
	return flag;
}

static inline int StringBase_isRope(StringBase *s)
{
	return S_flag(s) == S_FLAG_ROPE;
}

static inline int StringBase_isLiner(StringBase *s)
{
	return S_flag(s) == S_FLAG_LINER;
}

static void StringBase_setFlag(StringBase *s, uint32_t mask)
{
	s->h.magicflag |= mask;
}

static void StringBase_unsetFlag(StringBase *s, uint32_t mask)
{
	s->h.magicflag ^= mask;
}

static StringBase *new_StringBase(KonohaContext *kctx, uint32_t mask)
{
	StringBase *s = (StringBase *) new_(String, 0);
	StringBase_unsetFlag(s, MASK_INLINE);
	StringBase_setFlag(s, mask);
	return s;
}

static StringBase *InlineString_new(KonohaContext *kctx, StringBase *base, const char *text, size_t len)
{
	size_t i;
	InlineString *s = (InlineString *) base;
	StringBase_setFlag(base, MASK_INLINE);
	s->base.length = len;
	assert(len < SIZEOF_INLINETEXT);
	for (i = 0; i < len; ++i) {
		s->inline_text[i] = text[i];
	}
	s->inline_text[len] = '\0';
	s->text = s->inline_text;
	return base;
}

static StringBase *ExternalString_new(KonohaContext *kctx, StringBase *base,
		const char *text, size_t len)
{
	ExternalString *s = (ExternalString *) base;
	StringBase_setFlag(base, MASK_EXTERNAL);
	s->base.length = len;
	s->text = (char *) text;
	return base;
}

static StringBase *LinerString_new(KonohaContext *kctx, StringBase *base,
		const char *text, size_t len)
{
	LinerString *s = (LinerString *) base;
	StringBase_setFlag(base, MASK_LINER);
	s->base.length = len;
	s->text = (char *) KMALLOC(len+1);
	memcpy(s->text, text, len);
	s->text[len] = '\0';
	return base;
}

static StringBase *RopeString_new(KonohaContext *kctx, StringBase *left, StringBase *right, size_t len)
{
	RopeString *s = (RopeString *) new_StringBase(kctx, MASK_ROPE);
	s->base.length = len;
	s->left  = left;
	s->right = right;
	return (StringBase *) s;
}

static LinerString *RopeString_toLinerString(RopeString *o, char *text, size_t len)
{
	LinerString *s = (LinerString *) o;
	StringBase_unsetFlag((StringBase*)s, MASK_ROPE);
	StringBase_setFlag((StringBase*)s, MASK_LINER);
	s->base.length = len;
	s->text = text;
	return s;
}

static void checkASCII(KonohaContext *kctx, StringBase *s, const char *text, size_t length)
{
	unsigned char ch = 0;
	long len = length, n = (len + 3) / 4;
	const unsigned char*p = (const unsigned char *) text;
	switch(len % 4) { /* Duff's device written by ide */
		case 0: do{ ch |= *p++;
		case 3:     ch |= *p++;
		case 2:     ch |= *p++;
		case 1:     ch |= *p++;
		} while(--n>0);
	}
	S_setASCII((kStringVar*)s, (ch < 128));
}

static kString *new_kString(KonohaContext *kctx, const char *text, size_t len, int policy)
{
	StringBase *s = (StringBase *) new_StringBase(kctx, 0);
	if(TFLAG_is(int, policy, SPOL_ASCII)) {
		S_setASCII(s, 1);
	} else if(TFLAG_is(int, policy, SPOL_UTF8)) {
		S_setASCII(s, 0);
	} else {
		checkASCII(kctx, s, text, len);
	}
	if (len < SIZEOF_INLINETEXT)
		return (kString*) InlineString_new(kctx, s, text, len);
	if(TFLAG_is(int, policy, SPOL_TEXT))
		return (kString*) ExternalString_new(kctx, s, text, len);
	return (kString*) LinerString_new(kctx, s, text, len);
}

static void String2_free(KonohaContext *kctx, kObject *o)
{
	StringBase *base = (StringBase*) o;
	if ((S_flag(base) & S_FLAG_EXTERNAL) == S_FLAG_LINER) {
		assert(((LinerString *)base)->text == ((kString*)base)->buf);
		KFREE(((LinerString *)base)->text, S_len(base)+1);
	}
}

static void write_text(StringBase *base, char *dest, int size)
{
	RopeString *str;
	size_t len;
	while (1) {
		switch (S_flag(base)) {
			case S_FLAG_LINER:
			case S_FLAG_EXTERNAL:
				memcpy(dest, ((LinerString *) base)->text, size);
				return;
			case S_FLAG_INLINE:
				assert(base->length < SIZEOF_INLINETEXT);
				memcpy(dest, ((InlineString *) base)->inline_text, size);
				return;
			case S_FLAG_ROPE:
				str = (RopeString *) base;
				assert(str->left->length + str->right->length == size);
				len = S_len(str->left);
				write_text(str->left, dest, len);
				base = str->right;
				dest += len;
				size -= len;
				break;
		}
	}
}

static LinerString *RopeString_flatten(KonohaContext *kctx, RopeString *rope)
{
	size_t length = S_len((StringBase *) rope);
	char *dest = (char *) KMALLOC(length+1);
	size_t llen = S_len(rope->left);
	write_text(rope->left,  dest,llen);
	write_text(rope->right, dest+llen, length - llen);
	return RopeString_toLinerString(rope, dest, length);
}

static char *String_getReference(KonohaContext *kctx, StringBase *s)
{
	uint32_t flag = S_flag(s);
	switch (flag) {
		case S_FLAG_LINER:
		case S_FLAG_EXTERNAL:
		case S_FLAG_INLINE:
			return ((LinerString*)s)->text;
		case S_FLAG_ROPE:
			return RopeString_flatten(kctx, (RopeString*)s)->text;
		default:
			/*unreachable*/
			assert(0);
	}
	return NULL;
}

static void StringBase_reftrace(KonohaContext *kctx, StringBase *s)
{
	while (1) {
		if (unlikely(!StringBase_isRope(s)))
			break;
		RopeString *rope = (RopeString *) s;
		StringBase_reftrace(kctx, rope->left);
		BEGIN_REFTRACE(3);
		KREFTRACEv(rope);//FIXME reftracing rope is needed?
		KREFTRACEv(rope->left);
		KREFTRACEv(rope->right);
		END_REFTRACE();
		s = rope->right;
	}
}

static void String2_reftrace(KonohaContext *kctx, kObject *o)
{
	StringBase_reftrace(kctx, (StringBase *) o);
}

static uintptr_t String2_unbox(KonohaContext *kctx, kObject *o)
{
	StringBase *s = (StringBase*)o;
	return (uintptr_t) String_getReference(kctx, s);
}

static StringBase *StringBase_concat(KonohaContext *kctx, kString *s0, kString *s1)
{
	StringBase *left = (StringBase*) s0, *right = (StringBase*) s1;
	size_t llen, rlen, length;

	llen = S_len(left);
	if (llen == 0)
		return right;

	rlen = S_len(right);
	if (rlen == 0)
		return left;

	length = llen + rlen;
	if (length < SIZEOF_INLINETEXT) {
		InlineString *ret = (InlineString *) new_StringBase(kctx, MASK_INLINE);
		char *s0 = String_getReference(kctx, left);
		char *s1 = String_getReference(kctx, right);
		assert(length < SIZEOF_INLINETEXT);
		ret->base.length = length;
		memcpy(ret->inline_text, s0, llen);
		memcpy(ret->inline_text + llen, s1, rlen);
		ret->inline_text[length] = '\0';
		ret->text = ret->inline_text;
		return (StringBase *) ret;
	}
	return RopeString_new(kctx, left, right, length);
}

/* ------------------------------------------------------------------------ */
//## @Const method String String.concat(String rhs);
//## @Const method String String.opADD(String rhs);

static KMETHOD Rope_opADD(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *lhs = sfp[0].s, *rhs = sfp[1].asString;
	RETURN_(StringBase_concat(kctx, lhs, rhs));
}

/* ------------------------------------------------------------------------ */

#define SPOL_isASCII(s)       (S_isASCII(s) ? SPOL_ASCII : 0)
#define S_msize(s)        text_mlen(S_text(s), S_size(s))
#define S_length(s)       (S_isASCII(s) ? S_size(s) : S_msize(s))
#define CT_StringArray0   CT_p0(kctx, CT_Array, TY_String)
#define S_index(s, n)     ((n < 0) ? S_length(s) + n : n)
#define S_compare(s0, s1) strncmp(S_text(s0), S_text(s1), (S_size(s0) < S_size(s1)) ? S_size(s1) : S_size(s0))

/* FIXME: needs throw? */
#define S_range(s, n)     ((n < 0) ? 0 : ((n > S_length(s)) ? S_length(s) : n))

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

static size_t text_msize(const char *text, size_t size)
{
	const unsigned char *start = (const unsigned char *)text;
	size_t i, mindex = 0;
	for(i = 0; i < size; i++) {
		mindex += utf8len(*(start + mindex));
	}
	return mindex;
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
	kString *s = KLIB new_kString(kctx, S_text(s0), size, SPOL_isASCII(s0)|SPOL_NOCOPY);
	char *text = (char *) S_text(s);
	for(i = start; i < size; i++) {
		int ch = text[i];
		text[i] = ('a' <= ch && ch <= 'z') ? toupper(ch) : ch;
	}
	return s;
}

static kString* S_tolower(KonohaContext *kctx, kString *s0, size_t start)
{
	size_t i, size = S_size(s0);
	kString *s = KLIB new_kString(kctx, S_text(s0), size, SPOL_isASCII(s0)|SPOL_NOCOPY);
	char *text = (char *) S_text(s);
	for(i = start; i < size; i++) {
		int ch = text[i];
		text[i] = ('A' <= ch && ch <= 'Z') ? tolower(ch) : ch;
	}
	return s;
}

static kString *S_substring(KonohaContext *kctx, KonohaStack *sfp, kString *s0, size_t start, size_t length)
{
	kString *ret = NULL;
	if(length == 0) {
		return KNULL(String);
	}
	if (S_isASCII(s0)) {
		start = check_index(kctx, start, S_size(s0), sfp[K_RTNIDX].uline);
		const char *new_text = S_text(s0) + start;
		ret = KLIB new_kString(kctx, new_text, length, SPOL_ASCII|SPOL_POOL); // FIXME SPOL
	}
	else {
		ret = S_msubstr(kctx, s0, start, length);
		if (unlikely(ret == NULL)) {
			kreportf(CritTag, sfp[K_RTNIDX].uline, "Script!!: out of array index %ld", sfp[1].intValue);
			KLIB KonohaRuntime_raise(kctx, EXPT_("OutOfStringBoundary"), sfp, sfp[K_RTNIDX].uline, NULL);
		}
	}
	return ret;
}

/* ------------------------------------------------------------------------ */
//## method @Const Boolean String.<(String s);

static KMETHOD String_opLT(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	int res = S_compare(s0, s1);
	if(res < 0) {
		RETURNb_(true);
	}
	RETURNb_(false);
}

/* ------------------------------------------------------------------------ */
//## method @Const Boolean String.<=(String s);

static KMETHOD String_opLTE(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	int res = S_compare(s0, s1);
	if(res <= 0) {
		RETURNb_(true);
	}
	RETURNb_(false);
}

/* ------------------------------------------------------------------------ */
//## method @Const Boolean String.>(String s);

static KMETHOD String_opGT(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	int res = S_compare(s0, s1);
	if(res > 0) {
		RETURNb_(true);
	}
	RETURNb_(false);
}

/* ------------------------------------------------------------------------ */
//## method @Const Boolean String.>=(String s);

static KMETHOD String_opGTE(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	int res = S_compare(s0, s1);
	if(res >= 0) {
		RETURNb_(true);
	}
	RETURNb_(false);
}

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
		const char *p = strstr(t0, t1);
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
	if(S_size(s1) == 0) {
		RETURNi_(start);
	}
	long loc = -1;
	const char *t0 = S_text(s0);
	if(start > 0) {
		t0 += text_msize(t0, start);
	}
	const char *t1 = S_text(s1);
	const char *p = strstr(t0, t1);
	if (p != NULL) {
		loc = p - t0;
		if (!S_isASCII(s0)) {
			loc = text_mlen(t0, (size_t)loc);
		}
		RETURNi_(loc + start);
	}
	RETURNi_(-1);
}

static kint_t S_lastIndexOf(KonohaContext *kctx, kString *s0, kString *s1, size_t start)
{
	const char *t0 = S_text(s0);
	const char *t1 = S_text(s1);
	int len = S_size(s1);
	if(len == 0) {
		return S_isASCII(s0) ? start : text_mlen(t0, start);
	}
	kint_t loc;
	for(loc = start - len; loc >= 0; loc--) {
		if(t0[loc] == t1[0]) {
			if(strncmp(t0 + loc, t1, len) == 0) break;
		}
	}
	if (loc >= 0 && !S_isASCII(s0)) {
		loc = text_mlen(t0, (size_t)loc);
	}
	return (loc < 0) ? -1 : loc;
}

/* ------------------------------------------------------------------------ */
//## @Const method Int String.lastIndexOf(String searchvalue);

static KMETHOD String_lastIndexOf(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	int start = S_size(s0);
	RETURNi_(S_lastIndexOf(kctx, s0, s1, start));
}

/* ------------------------------------------------------------------------ */
//## @Const method Int String.lastIndexOf(String searchvalue, Int start);

static KMETHOD String_lastIndexOfwithStart(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	const char *t0 = S_text(s0);
	kint_t start = S_range(s0, sfp[2].intValue);
	RETURNi_(S_lastIndexOf(kctx, s0, s1, text_msize(t0, start + 1)));
}

/* ------------------------------------------------------------------------ */
//## @Const method Int String.localeCompare(String that);
/* http://ecma-international.org/ecma-262/5.1/#sec-15.5.4.9 */

static KMETHOD String_localeCompare(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	kint_t ret = 0;
	int res = S_compare(s0, s1);
	if(res < 0) {
		ret = -1;
	}
	else if(res > 0) {
		ret = 1;
	}
	RETURNi_(ret);
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
			const char *res = strstr(start, S_text(searchvalue));
			if(res == NULL) break;
			if(res - start > 0) {
				KLIB Kwb_write(kctx, &wb, start, res - start);
			}
			KLIB Kwb_write(kctx, &wb, S_text(newvalue), S_size(newvalue));
			start = res + S_size(searchvalue);
		}
	}
	KLIB Kwb_write(kctx, &wb, start, strlen(start)); // write out remaining string
	RETURN_(KLIB new_kString(kctx, KLIB Kwb_top(kctx, &wb, 0), Kwb_bytesize(&wb), SPOL_isASCII(s0)|SPOL_POOL));
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
		ret = KLIB new_kString(kctx, s, len, SPOL_isASCII(sfp[0].asString));
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
			KLIB KonohaRuntime_raise(kctx, EXPT_("OutOfStringBoundary"), sfp, sfp[K_RTNIDX].uline, NULL);
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
		KLIB KonohaRuntime_raise(kctx, EXPT_("OutOfStringBoundary"), sfp, sfp[K_RTNIDX].uline, NULL);
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
	start = (((kint_t)S_length(s0)) < start) ? S_length(s0) : start;
	kint_t length = S_length(s0) - start;
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
	const char *text = S_text(s0);
	for(i = 0; i < size; i++) {
		int ch = text[i];
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
	const char *text = S_text(s0);
	for(i = 0; i < size; i++) {
		int ch = text[i];
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
	start = (((kint_t)S_length(s0)) < start) ? S_length(s0) : start;
	kint_t end = S_index(s0, sfp[2].intValue);
	end = (((kint_t)S_length(s0)) < end) ? S_length(s0) : end;
	size_t length = (end < start) ? 0 : end - start;
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
			const char *res = strstr(start, S_text(separator));
			if(res == NULL) break;
			if(res - start > 0) {
				KLIB kArray_add(kctx, a, KLIB new_kString(kctx, start, res - start, SPOL_isASCII(s0)|SPOL_POOL));
			}
			start = res + S_size(separator);
		}
		KLIB kArray_add(kctx, a, KLIB new_kString(kctx, start, strlen(start), SPOL_isASCII(s0)|SPOL_ASCII));
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
	if(limit < 0) {
		/* ignore limit */
		limit = length;
	}
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
			const char *res = strstr(start, S_text(separator));
			if(res == NULL) break;
			if(res - start > 0) {
				KLIB kArray_add(kctx, a, KLIB new_kString(kctx, start, res - start, SPOL_isASCII(s0)|SPOL_POOL));
				length++;
			}
			start = res + S_size(separator);
		}
		if(length < limit - 1) {
			KLIB kArray_add(kctx, a, KLIB new_kString(kctx, start, strlen(start), SPOL_isASCII(s0)|SPOL_ASCII));
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

	kMethod *concat = KLIB kNameSpace_getMethodByParamSizeNULL(kctx, ns, TY_String, MN_("+"), 1);
	if (concat != NULL) {
		KLIB kMethod_setFunc(kctx, concat, Rope_opADD);
	} else {
		KDEFINE_METHOD MethodData[] = {
			_Public|_Const|_Im, _F(Rope_opADD), TY_String, TY_String, MN_("+"), 1, TY_String, FN_s,
			DEND
		};
		KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	}

	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(Rope_opADD), TY_String, TY_String, MN_("concat"), 1, TY_String, FN_s,
		/*JS*/_Public|_Const|_Im, _F(String_opLT),  TY_boolean, TY_String, MN_("<"),  1, TY_String, FN_s,
		/*JS*/_Public|_Const|_Im, _F(String_opLTE),  TY_boolean, TY_String, MN_("<="),  1, TY_String, FN_s,
		/*JS*/_Public|_Const|_Im, _F(String_opGT),  TY_boolean, TY_String, MN_(">"),  1, TY_String, FN_s,
		/*JS*/_Public|_Const|_Im, _F(String_opGTE),  TY_boolean, TY_String, MN_(">="),  1, TY_String, FN_s,
		/*JS*/_Public|_Const|_Im, _F(String_getSize), TY_int, TY_String, MN_("getlength"), 0,
		/*JS*/_Public|_Const|_Im, _F(String_get), TY_String, TY_String, MN_("charAt"), 1, TY_int, FN_("index"),
		/*JS*/_Public|_Const|_Im, _F(String_charCodeAt), TY_int, TY_String, MN_("charCodeAt"), 1, TY_int, FN_("index"),
		/*JS*/_Public|_Static|_Const|_Im, _F(String_fromCharCode), TY_String, TY_String, MN_("fromCharCode"), 1, TY_int, FN_n,
		/*JS*/_Public|_Const|_Im, _F(String_indexOf), TY_int, TY_String, MN_("indexOf"), 1, TY_String, FN_("searchvalue"),
		/*JS*/_Public|_Const|_Im, _F(String_indexOfwithStart), TY_int, TY_String, MN_("indexOf"), 2, TY_String, FN_("searchvalue"), TY_int, FN_("start"),
		/*JS*/_Public|_Const|_Im, _F(String_lastIndexOf), TY_int, TY_String, MN_("lastIndexOf"), 1, TY_String, FN_("searchvalue"),
		/*JS*/_Public|_Const|_Im, _F(String_lastIndexOfwithStart), TY_int, TY_String, MN_("lastIndexOf"), 2, TY_String, FN_("searchvalue"), TY_int, FN_("start"),
		/*JS*/_Public|_Const|_Im, _F(String_localeCompare), TY_int, TY_String, MN_("localeCompare"), 1, TY_String, FN_("that"),
		/*JS*/_Public|_Const|_Im, _F(String_replace), TY_String, TY_String, MN_("replace"), 2, TY_String, FN_("searchvalue"), TY_String, FN_("newvalue"),
		/*JS*/_Public|_Const|_Im, _F(String_indexOf), TY_int, TY_String, MN_("search"), 1, TY_String, FN_("searchvalue"),
		/*JS*/_Public|_Const|_Im, _F(String_substr), TY_String, TY_String, MN_("slice"), 1, TY_int, FN_("start"),
		/*JS*/_Public|_Const|_Im, _F(String_slice), TY_String, TY_String, MN_("slice"), 2, TY_int, FN_("start"), TY_int, FN_("end"),
		/*JS*/_Public|_Im, _F(String_split), TY_StringArray0, TY_String, MN_("split"), 0,
		/*JS*/_Public|_Im, _F(String_splitwithSeparator), TY_StringArray0, TY_String, MN_("split"), 1, TY_String, FN_("separator"),
		/*JS*/_Public|_Im, _F(String_splitwithSeparatorLimit), TY_StringArray0, TY_String, MN_("split"), 2, TY_String, FN_("separator"), TY_int, FN_("limit"),
		/*JS*/_Public|_Const|_Im, _F(String_substr), TY_String, TY_String, MN_("substr"), 1, TY_int, FN_("start"),
		/*JS*/_Public|_Const|_Im, _F(String_substrwithLength), TY_String, TY_String, MN_("substr"), 2, TY_int, FN_("start"), TY_int, FN_("length"),
		/*JS*/_Public|_Const|_Im, _F(String_substring), TY_String, TY_String, MN_("substring"), 1, TY_int, FN_("indexA"),
		/*JS*/_Public|_Const|_Im, _F(String_substringwithIndexB), TY_String, TY_String, MN_("substring"), 2, TY_int, FN_("indexA"), TY_int, FN_("indexB"),
		/*JS*/_Public|_Const|_Im, _F(String_toLower), TY_String, TY_String, MN_("toLowerCase"), 0,
		/*JS*/_Public|_Const|_Im, _F(String_toUpper), TY_String, TY_String, MN_("toUpperCase"), 0,
		/*JS*/_Public|_Const|_Im, _F(String_valueOf), TY_String, TY_String, MN_("valueOf"), 0,
		_Public|_Const|_Im, _F(String_opHAS),      TY_boolean, TY_String, MN_("opHAS"), 1, TY_String, FN_s,
		_Public|_Const|_Im, _F(String_trim),       TY_String, TY_String, MN_("trim"), 0,
		_Public|_Const|_Im, _F(String_get),        TY_String, TY_String, MN_("get"), 1, TY_int, FN_n,
		_Public|_Const|_Im, _F(String_startsWith), TY_boolean, TY_String, MN_("startsWith"), 1, TY_String, FN_s,
		_Public|_Const|_Im, _F(String_endsWith),   TY_boolean, TY_String, MN_("endsWith"),   1, TY_String, FN_s,
		_Public|_Const|_Im, _F(String_getSize),    TY_int, TY_String, MN_("getSize"), 0,
		_Public|_Const|_Im, _F(String_toUpper),    TY_String, TY_String, MN_("toUpper"), 0,
		_Public|_Const|_Im, _F(String_toLower),    TY_String, TY_String, MN_("toLower"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);

	KSET_TYFUNC(CT_String, unbox, String2, pline);
	KSET_TYFUNC(CT_String, free, String2, pline);
	KSET_TYFUNC(CT_String, reftrace, String2, pline);
	KSET_KLIB(new_kString, pline);

	return true;
}

static kbool_t string_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t string_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t string_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* string_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSETPACKNAME(d, "String", "1.0");
	d.initPackage    = string_initPackage;
	d.setupPackage   = string_setupPackage;
	d.initNameSpace  = string_initNameSpace;
	d.setupNameSpace = string_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
