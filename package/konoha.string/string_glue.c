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

#define S_FLAG_MASK_BASE (7)
#define S_FLAG_LINER     ((1UL << (0)))
#define S_FLAG_NOFREE    ((1UL << (1)))
#define S_FLAG_ROPE      ((1UL << (2)))
#define S_FLAG_INLINE    (S_FLAG_NOFREE)
#define S_FLAG_EXTERNAL  (S_FLAG_LINER | S_FLAG_NOFREE)

#define MASK_LINER    ((S_FLAG_LINER   ) << S_FLAG_MASK_BASE)
//#define MASK_NOFREE ((S_FLAG_NOFREE  ) << S_FLAG_MASK_BASE)
#define MASK_ROPE     ((S_FLAG_ROPE    ) << S_FLAG_MASK_BASE)
#define MASK_INLINE   ((S_FLAG_INLINE  ) << S_FLAG_MASK_BASE)
#define MASK_EXTERNAL ((S_FLAG_EXTERNAL) << S_FLAG_MASK_BASE)

#define StringBase_length(s) ((s)->length)
typedef struct kStringBase {
	KonohaObjectHeader h;
	size_t length;
} kStringBase;

typedef struct kLinerString {
	kStringBase base;
	char *text;
} kLinerString;

typedef struct kExternalString {
	struct kStringBase base;
	char *text;
} kExternalString;

typedef struct kRopeString {
	struct kStringBase base;
	struct kStringBase *left;
	struct kStringBase *right;
} kRopeString;

typedef struct kInlineString {
	struct kStringBase base;
	/* for binary compatibility with kString, we need 'text' pointer */
	char *text;
	char inline_text[SIZEOF_INLINETEXT];
} kInlineString;

static inline uint32_t kStringBase_flag(kStringBase *s)
{
	uint32_t flag = ((~0U) & (s)->h.magicflag) >> S_FLAG_MASK_BASE;
	DBG_ASSERT(flag <= S_FLAG_ROPE);
	return flag;
}

static inline int kStringBase_isRope(kStringBase *s)
{
	return kStringBase_flag(s) == S_FLAG_ROPE;
}

static inline int kStringBase_isLiner(kStringBase *s)
{
	return kStringBase_flag(s) == S_FLAG_LINER;
}

static void kStringBase_setFlag(kStringBase *s, uint32_t mask)
{
	s->h.magicflag |= (uintptr_t)mask;
}

static void kStringBase_unsetFlag(kStringBase *s, uint32_t mask)
{
	s->h.magicflag ^= (uintptr_t)mask;
}

static kStringBase *new_kStringBase(KonohaContext *kctx, kArray* gcstack, uint32_t mask)
{
	kStringBase *s = (kStringBase *)new_(String, 0, gcstack);
	kStringBase_unsetFlag(s, MASK_INLINE);
	kStringBase_setFlag(s, mask);
	return s;
}

static kStringBase *kStringBase_InitInline(KonohaContext *kctx, kStringBase *base, const char *text, size_t len)
{
	size_t i;
	kInlineString *s = (kInlineString *) base;
	kStringBase_setFlag(base, MASK_INLINE);
	s->base.length = len;
	DBG_ASSERT(len < SIZEOF_INLINETEXT);
	for (i = 0; i < len; ++i) {
		s->inline_text[i] = text[i];
	}
	s->inline_text[len] = '\0';
	s->text = s->inline_text;
	return base;
}

static kStringBase *kStringBase_InitExternal(KonohaContext *kctx, kStringBase *base, const char *text, size_t len)
{
	kExternalString *s = (kExternalString *) base;
	kStringBase_setFlag(base, MASK_EXTERNAL);
	s->base.length = len;
	s->text = (char *) text;
	return base;
}

static kStringBase *kStringBase_InitLiner(KonohaContext *kctx, kStringBase *base, const char *text, size_t len)
{
	kLinerString *s = (kLinerString *) base;
	kStringBase_setFlag(base, MASK_LINER);
	s->base.length = len;
	s->text = (char *) KMalloc_UNTRACE(len+1);
	memcpy(s->text, text, len);
	s->text[len] = '\0';
	return base;
}

static kStringBase *kStringBase_InitRope(KonohaContext *kctx, kArray *gcstack, kStringBase *left, kStringBase *right, size_t len)
{
	kRopeString *rope = (kRopeString *) new_kStringBase(kctx, gcstack, MASK_ROPE);
	rope->base.length = len;
	KUnsafeFieldInit(rope->left,  left);
	KUnsafeFieldInit(rope->right, right);
	KLIB Kwrite_barrier(kctx, (kObject *)rope);
	return (kStringBase *) rope;
}

static kLinerString *kRopeString_toLinerString(kRopeString *o, char *text, size_t len)
{
	kLinerString *s = (kLinerString *) o;
	kStringBase_unsetFlag((kStringBase *)s, MASK_ROPE);
	kStringBase_setFlag((kStringBase *)s, MASK_LINER);
	s->base.length = len;
	s->text = text;
	return s;
}

static void checkASCII(KonohaContext *kctx, kStringBase *s, const char *text, size_t length)
{
	unsigned char ch = 0;
	long len = length, n = (len + 3) / 4;
	const unsigned char * p = (const unsigned char *) text;
	switch(len % 4) { /* Duff's device written by ide */
		case 0: do{ ch |= *p++;
		case 3:     ch |= *p++;
		case 2:     ch |= *p++;
		case 1:     ch |= *p++;
		} while(--n>0);
	}
	kString_set(ASCII, (kStringVar *)s, (ch < 128));
}

static kString *new_kString(KonohaContext *kctx, kArray *gcstack, const char *text, size_t len, int policy)
{
	kStringBase *s = (kStringBase *) new_kStringBase(kctx, gcstack, 0);
	if(TFLAG_is(int, policy, StringPolicy_ASCII)) {
		kString_set(ASCII, s, 1);
	} else if(TFLAG_is(int, policy, StringPolicy_UTF8)) {
		kString_set(ASCII, s, 0);
	} else {
		checkASCII(kctx, s, text, len);
	}
	if(len < SIZEOF_INLINETEXT)
		return (kString *) kStringBase_InitInline(kctx, s, text, len);
	if(TFLAG_is(int, policy, StringPolicy_TEXT))
		return (kString *) kStringBase_InitExternal(kctx, s, text, len);
	return (kString *) kStringBase_InitLiner(kctx, s, text, len);
}

static void String2_free(KonohaContext *kctx, kObject *o)
{
	kStringBase *base = (kStringBase *) o;
	if((kStringBase_flag(base) & S_FLAG_EXTERNAL) == S_FLAG_LINER) {
		assert(((kLinerString *)base)->text == ((kString *)base)->buf);
		KFree(((kLinerString *)base)->text, StringBase_length(base)+1);
	}
}

static void Stack_init(KonohaContext *kctx, KGrowingArray *stack)
{
	KLIB Karray_init(kctx, stack, 4 * sizeof(kStringBase**));
}

static void Stack_push(KonohaContext *kctx, KGrowingArray *stack, kStringBase *str)
{
	size_t index = stack->bytesize / sizeof(kStringBase *);
	if(stack->bytesize == stack->bytemax) {
		KLIB Karray_expand(kctx, stack, stack->bytemax * 2);
	}
	stack->ObjectItems[index] = (kObject *) str;
	stack->bytesize += sizeof(kStringBase *);
}

static kStringBase *Stack_pop(KonohaContext *kctx, KGrowingArray *stack)
{
	size_t index = (stack->bytesize / sizeof(kStringBase *));
	if(index == 0) {
		return NULL;
	}
	kStringBase *str = (kStringBase *)stack->ObjectItems[index-1];
	stack->bytesize -= sizeof(kStringBase *);
	return str;
}

static void Stack_dispose(KonohaContext *kctx, KGrowingArray *stack)
{
	KLIB Karray_free(kctx, stack);
}

static void copyText(KonohaContext *kctx, KGrowingArray *stack, char *dest, size_t size)
{
	kStringBase *base;
	kRopeString *str;
	while((base = Stack_pop(kctx, stack)) != NULL) {
		switch (kStringBase_flag(base)) {
			case S_FLAG_LINER:
			case S_FLAG_EXTERNAL:
				memcpy(dest, ((kLinerString *) base)->text, base->length);
				dest += base->length;
				break;
			case S_FLAG_INLINE:
				assert(base->length < SIZEOF_INLINETEXT);
				memcpy(dest, ((kInlineString *) base)->text, base->length);
				dest += base->length;
				break;
			case S_FLAG_ROPE:
				str = (kRopeString *) base;
				Stack_push(kctx, stack, str->right);
				Stack_push(kctx, stack, str->left);
				break;
		}
	}
}

static kLinerString *kRopeString_flatten(KonohaContext *kctx, kRopeString *rope)
{
	size_t length = StringBase_length((kStringBase *) rope);
	char  *dest = (char *) KMalloc_UNTRACE(length+1);
	KGrowingArray stack;
	Stack_init(kctx, &stack);
	Stack_push(kctx, &stack, rope->right);
	Stack_push(kctx, &stack, rope->left);
	copyText(kctx, &stack, dest, length);
	Stack_dispose(kctx, &stack);
	return kRopeString_toLinerString(rope, dest, length);
}

static char *kStringBase_getTextReference(KonohaContext *kctx, kStringBase *s)
{
	uint32_t flag = kStringBase_flag(s);
	switch (flag) {
		case S_FLAG_LINER:
		case S_FLAG_EXTERNAL:
		case S_FLAG_INLINE:
			return ((kLinerString *)s)->text;
		case S_FLAG_ROPE:
			return kRopeString_flatten(kctx, (kRopeString *)s)->text;
		default:
			/*unreachable*/
			assert(0);
	}
	return NULL;
}

static void String2_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kStringBase *s = (kStringBase *) o;
	if(kStringBase_isRope(s)) {
		kRopeString *rope = (kRopeString *) s;
		KREFTRACEv(rope->left);
		KREFTRACEv(rope->right);
	}
}

static uintptr_t String2_unbox(KonohaContext *kctx, kObject *o)
{
	kStringBase *s = (kStringBase *)o;
	return (uintptr_t) kStringBase_getTextReference(kctx, s);
}

static kStringBase *kStringBase_concat(KonohaContext *kctx, kArray *gcstack, kString *s0, kString *s1)
{
	kStringBase *left = (kStringBase *) s0, *right = (kStringBase *) s1;
	size_t leftLen = StringBase_length(left);
	if(leftLen == 0) {
		return right;
	}
	size_t rightLen = StringBase_length(right);
	if(rightLen == 0) {
		return left;
	}
	size_t length = leftLen + rightLen;

	if(length + 1 < SIZEOF_INLINETEXT) {
		char *leftChar = kStringBase_getTextReference(kctx, left);
		char *rightChar = kStringBase_getTextReference(kctx, right);
		kInlineString *result = (kInlineString *) new_kStringBase(kctx, gcstack, MASK_INLINE);
		result->base.length = length;
		memcpy(result->inline_text, leftChar, leftLen);
		memcpy(result->inline_text + leftLen, rightChar, rightLen);
		result->inline_text[length] = '\0';
		result->text = result->inline_text;
		return (kStringBase *) result;
	}
	return kStringBase_InitRope(kctx, gcstack, left, right, length);
}

/* ------------------------------------------------------------------------ */

//## String String.concat(String rhs);
//## String String.opADD(String rhs);

static KMETHOD Rope_opADD(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *lhs = sfp[0].asString, *rhs = sfp[1].asString;
	KReturn(kStringBase_concat(kctx, OnStack, lhs, rhs));
}

/* ------------------------------------------------------------------------ */

#define StringPolicy_maskASCII(s)      (kString_is(ASCII, s) ? StringPolicy_ASCII : 0)

/*
 * Get the number of multibyte characters from text and bytesize.
 * e.g.) utf8_getMultibyteTextSize("あいうえお", 9) => 3
 */
static size_t utf8_getMultibyteTextSize(const char *s_text, size_t s_size)
{
	size_t size = 0;
	const unsigned char *start = (const unsigned char *)s_text;
	const unsigned char *end = start + s_size;
	while(start < end) {
		size_t ulen = utf8len(start[0]);
		size++;
		start += ulen;
	}
	return size;
}

/*
 * Get the bytesize from text and number of multibyte characters.
 * e.g.) utf8_getMultibyteByteSize("あいうえお", 3) => 9
 */
static size_t utf8_getMultibyteByteSize(const char *text, size_t size)
{
	const unsigned char *start = (const unsigned char *)text;
	size_t i, mindex = 0;
	for(i = 0; i < size; i++) {
		mindex += utf8len(*(start + mindex));
	}
	return mindex;
}

static void THROW_OutOfStringBoundary(KonohaContext *kctx, KonohaStack *sfp, kint_t index)
{
	KLIB KonohaRuntime_raise(kctx, EXPT_("OutOfStringBoundary"), SoftwareFault, NULL, sfp);
}

static size_t kString_getMultibyteSize(KonohaContext *kctx, kString *this)
{
	if(kString_is(ASCII, this)) {
		return S_size(this);
	}
	return utf8_getMultibyteTextSize(S_text(this), S_size(this));
}

static size_t kString_getIndex(KonohaContext *kctx, KonohaStack *sfp, kString *this, kint_t index)
{
	size_t multibyteSize = kString_getMultibyteSize(kctx, this);
	if(index < 0 && (size_t)-index < multibyteSize) {
		return multibyteSize + index;
	}
	if((size_t)index < multibyteSize) {
		return index;
	}
	THROW_OutOfStringBoundary(kctx, sfp, index);
	return 0;
}

static size_t kString_checkIndex(KonohaContext *kctx, KonohaStack *sfp, kString *this, kint_t index)
{
	if(index < 0 || (size_t)index > kString_getMultibyteSize(kctx, this)) {
		THROW_OutOfStringBoundary(kctx, sfp, index);
	}
	return (size_t)index;
}

static kint_t kString_compare(KonohaContext *kctx, kString *s0, kString *s1)
{
	size_t n = (S_size(s0) < S_size(s1)) ? S_size(s1) : S_size(s0);
	return strncmp(S_text(s0), S_text(s1), n);
}

static kint_t kStringMulti_charAt(KonohaContext *kctx, kString *s, size_t n)
{
	const unsigned char *text = (const unsigned char *)S_text(s);
	const unsigned char *start = utf8_getMultibyteByteSize((const char *)text, n) + text;

	if((int)n < 0) {
		return -1;
	}
	if(start < text + S_size(s)) {
		const unsigned char *buf = start;
		unsigned short ret = 0;
		size_t utfsize = utf8len(buf[0]);
		switch(utfsize) {
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
			DBG_ASSERT(false);
			break;
		}
		return ret;
	}
	return -1;
}

static kString *new_SubStringMultiNULL(KonohaContext *kctx, kArray *gcstack, kString *s, size_t moff, size_t mlen)
{
	const unsigned char *text = (const unsigned char *)S_text(s);
	const unsigned char *start = utf8_getMultibyteByteSize((const char *)text, moff) + text;
	size_t size = S_size(s);
	kString *ret = NULL;

	if((int)moff < 0 || moff > size) {
		return ret;
	}
	if(start < text + size) {
		const unsigned char *end = NULL;
		if((int)mlen <= 0 || mlen > size) {
			end = text + size;
			ret = KLIB new_kString(kctx, gcstack, (const char *)start, end - start, StringPolicy_UTF8);
		}
		else {
			end = utf8_getMultibyteByteSize((const char *)start, mlen) + start;
			if(end < text + size) {
				ret = KLIB new_kString(kctx, gcstack, (const char *)start, end - start, StringPolicy_UTF8);
			}
			else {
				ret = KLIB new_kString(kctx, gcstack, (const char *)start, text + size - start, StringPolicy_UTF8);
			}
		}
	}
	return ret;
}

static kString *new_SubString(KonohaContext *kctx, kArray *gcstack, kString *baseString, size_t start, size_t length, KonohaStack *sfp)
{
	kString *ret = NULL;
	if(length == 0) {
		return KNULL(String);
	}
	if(kString_is(ASCII, baseString)) {
		kString_checkIndex(kctx, sfp, baseString, start);
		const char *new_text = S_text(baseString) + start;
		ret = KLIB new_kString(kctx, gcstack, new_text, length, StringPolicy_ASCII); // FIXME SPOL
	}
	else {
		ret = new_SubStringMultiNULL(kctx, gcstack, baseString, start, length);
		if(unlikely(ret == NULL)) {
//			KMakeTrace(trace, sfp);
//			kreportf(CritTag, trace, "Script!!: out of array index %ld", sfp[1].intValue);
			KLIB KonohaRuntime_raise(kctx, EXPT_("OutOfStringBoundary"), SoftwareFault, NULL, sfp);
		}
	}
	return ret;
}

static kint_t kString_lastIndexOf(KonohaContext *kctx, kString *s0, kString *s1, size_t start)
{
	const char *t0 = S_text(s0);
	const char *t1 = S_text(s1);
	int len = S_size(s1);
	if(len == 0) {
		return kString_is(ASCII, s0) ? start : utf8_getMultibyteTextSize(t0, start);
	}
	kint_t loc;
	for(loc = start - len; loc >= 0; loc--) {
		if(t0[loc] == t1[0]) {
			if(strncmp(t0 + loc, t1, len) == 0) break;
		}
	}
	if(loc >= 0 && !kString_is(ASCII, s0)) {
		loc = utf8_getMultibyteTextSize(t0, (size_t)loc);
	}
	return (loc < 0) ? -1 : loc;
}

static kString* kString_toupper(KonohaContext *kctx, kArray *gcstack, kString *thisString, size_t start)
{
	size_t i, size = S_size(thisString);
	kString *s = KLIB new_kString(kctx, gcstack, S_text(thisString), size, StringPolicy_maskASCII(thisString)|StringPolicy_NOCOPY);
	char *text = (char *) S_text(s);
	for(i = start; i < size; i++) {
		int ch = text[i];
		text[i] = ('a' <= ch && ch <= 'z') ? toupper(ch) : ch;
	}
	return s;
}

static kString* kString_tolower(KonohaContext *kctx, kArray *gcstack, kString *thisString, size_t start)
{
	size_t i, size = S_size(thisString);
	kString *s = KLIB new_kString(kctx, gcstack, S_text(thisString), size, StringPolicy_maskASCII(thisString)|StringPolicy_NOCOPY);
	char *text = (char *) S_text(s);
	for(i = start; i < size; i++) {
		int ch = text[i];
		text[i] = ('A' <= ch && ch <= 'Z') ? tolower(ch) : ch;
	}
	return s;
}

/* ------------------------------------------------------------------------ */

//## boolean String.<(String s);
static KMETHOD String_opLT(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	int res = kString_compare(kctx, s0, s1);
	if(res < 0) {
		KReturnUnboxValue(true);
	}
	KReturnUnboxValue(false);
}

//## boolean String.<=(String s);
static KMETHOD String_opLTE(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	int res = kString_compare(kctx, s0, s1);
	if(res <= 0) {
		KReturnUnboxValue(true);
	}
	KReturnUnboxValue(false);
}

//## boolean String.>(String s);
static KMETHOD String_opGT(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	int res = kString_compare(kctx, s0, s1);
	if(res > 0) {
		KReturnUnboxValue(true);
	}
	KReturnUnboxValue(false);
}

//## boolean String.>=(String s);
static KMETHOD String_opGTE(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	int res = kString_compare(kctx, s0, s1);
	if(res >= 0) {
		KReturnUnboxValue(true);
	}
	KReturnUnboxValue(false);
}

//## int String.getSize();
static KMETHOD String_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *thisString = sfp[0].asString;
	KReturnUnboxValue(kString_getMultibyteSize(kctx, thisString));
}

//## boolean String.startsWith(String s);
static KMETHOD String_startsWith(KonohaContext *kctx, KonohaStack *sfp)
{
	// @TEST "A".startsWith("ABC");
	KReturnUnboxValue(strncmp(S_text(sfp[0].asString), S_text(sfp[1].asString), S_size(sfp[1].asString)) == 0);
}

//## boolean String.endsWith(String s);
static KMETHOD String_endsWith(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	int ret;
	if(S_size(s0) < S_size(s1)) {
		ret = 0;
	}
	else {
		const char *p = S_text(s0) + (S_size(s0) - S_size(s1));
		ret = (strncmp(p, S_text(s1), S_size(s1)) == 0);
	}
	KReturnUnboxValue(ret);
}

//## int String.indexOf(String searchString);
static KMETHOD String_indexOf(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *searchString = sfp[1].asString;
	if(S_size(searchString) > 0) {
		long loc = -1;
		const char *t0 = S_text(s0);
		const char *t1 = S_text(searchString);
		const char *p = strstr(t0, t1);
		if(p != NULL) {
			loc = p - t0;
			if(!kString_is(ASCII, s0)) {
				loc = utf8_getMultibyteTextSize(t0, (size_t)loc);
			}
		}
		KReturnUnboxValue(loc);
	}
	KReturnUnboxValue(0);
}

//## int String.indexOf(String searchvalue, int start);
static KMETHOD String_indexOfwithStart(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	kint_t start = kString_checkIndex(kctx, sfp, s0, sfp[2].intValue);
	if(S_size(s1) == 0) {
		KReturnUnboxValue(start);
	}
	long loc = -1;
	const char *t0 = S_text(s0);
	if(start > 0) {
		t0 += utf8_getMultibyteByteSize(t0, start);
	}
	const char *t1 = S_text(s1);
	const char *p = strstr(t0, t1);
	if(p != NULL) {
		loc = p - t0;
		if(!kString_is(ASCII, s0)) {
			loc = utf8_getMultibyteTextSize(t0, (size_t)loc);
		}
		KReturnUnboxValue(loc + start);
	}
	KReturnUnboxValue(-1);
}

static KMETHOD String_lastIndexOf(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	int start = S_size(s0);
	KReturnUnboxValue(kString_lastIndexOf(kctx, s0, s1, start));
}

//## int String.lastIndexOf(String searchvalue, int start);
static KMETHOD String_lastIndexOfwithStart(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	const char *t0 = S_text(s0);
	kint_t start = kString_checkIndex(kctx, sfp, s0, sfp[2].intValue);
	KReturnUnboxValue(kString_lastIndexOf(kctx, s0, s1, utf8_getMultibyteByteSize(t0, start + 1)));
}

/* http://ecma-international.org/ecma-262/5.1/#sec-15.5.4.9 */
//## int String.localeCompare(String that);
static KMETHOD String_localeCompare(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	kint_t ret = 0;
	int res = kString_compare(kctx, s0, s1);
	if(res < 0) {
		ret = -1;
	}
	else if(res > 0) {
		ret = 1;
	}
	KReturnUnboxValue(ret);
}

//## String String.replace(String searchvalue, String newvalue);
static KMETHOD String_replace(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *searchvalue = sfp[1].asString;
	kString *newvalue = sfp[2].asString;
	const char *start = S_text(s0);
	const char *end = start + S_size(s0);
	KGrowingBuffer wb;
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
	KReturn(KLIB new_kString(kctx, OnStack, KLIB Kwb_top(kctx, &wb, 0), Kwb_bytesize(&wb), StringPolicy_maskASCII(s0)));
}

//## String String.trim();
static KMETHOD String_trim(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *s = S_text(sfp[0].asString);
	size_t len = S_size(sfp[0].asString);
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
		ret = KLIB new_kString(kctx, OnStack, s, len, StringPolicy_maskASCII(sfp[0].asString));
	}
	else {
		ret = sfp[0].asString;
	}
	KReturn(ret);
}

//## boolean String.opHAS(String s);
static KMETHOD String_opHAS(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(strstr(S_text(sfp[0].asString), S_text(sfp[1].asString)) != NULL);
}

//## String String.get(int index);
static KMETHOD String_get(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s = sfp[0].asString;
	size_t index = kString_checkIndex(kctx, sfp, s, sfp[1].intValue);
	if(kString_is(ASCII, s)) {
		s = KLIB new_kString(kctx, OnStack, S_text(s) + index, 1, StringPolicy_ASCII);
	}
	else {
		s = new_SubString(kctx, OnStack, s, index, 1, sfp);
	}
	KReturn(s);
}

//## int String.charCodeAt(int index);
static KMETHOD String_charCodeAt(KonohaContext *kctx, KonohaStack *sfp)
{
	size_t n = (size_t)sfp[1].intValue;
	kint_t ccode = kStringMulti_charAt(kctx, sfp[0].asString, n);
	if(unlikely(ccode == -1)) {
		THROW_OutOfStringBoundary(kctx, sfp, n);
	}
	KReturnUnboxValue(ccode);
}

/* Microsoft's JScript does not support negative values for the start index. */
//## String String.slice(int start);
//## String String.substr(int start);
static KMETHOD String_substr(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *thisString = sfp[0].asString;
	size_t start = kString_getIndex(kctx, sfp, thisString, sfp[1].intValue);
	kint_t length = kString_getMultibyteSize(kctx, thisString) - start;
	KReturn(new_SubString(kctx, OnStack, thisString, start, length, sfp));
}

/* Microsoft's JScript does not support negative values for the start index. */
//## String String.substr(int start, int length);
static KMETHOD String_substrWithLength(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *thisString = sfp[0].asString;
	size_t start = kString_getIndex(kctx, sfp, thisString, sfp[1].intValue);  // FIXME
	kint_t length = kString_checkIndex(kctx, sfp, thisString, sfp[2].intValue);
	if(length == 0) {
		KReturn(KNULL(String));
	}
	KReturn(new_SubString(kctx, OnStack,  thisString, start, length, sfp));
}

//## String String.toUpper();
static KMETHOD String_toUpper(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *thisString = sfp[0].asString;
	size_t i, size = S_size(thisString);
	const char *text = S_text(thisString);
	for(i = 0; i < size; i++) {
		int ch = text[i];
		if('a' <= ch && ch <= 'z') {
			KReturn(kString_toupper(kctx, OnStack, thisString, i));
		}
	}
	KReturn(thisString);
}

//## String String.toLower();
static KMETHOD String_toLower(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *thisString = sfp[0].asString;
	size_t i, size = S_size(thisString);
	const char *text = S_text(thisString);
	for(i = 0; i < size; i++) {
		int ch = text[i];
		if('A' <= ch && ch <= 'Z') {
			KReturn(kString_tolower(kctx, OnStack, thisString, i));
		}
	}
	KReturn(thisString);
}

//## String String.fromCharCode(int charCode);
static KMETHOD String_fromCharCode(KonohaContext *kctx, KonohaStack *sfp)
{
	kint_t c = sfp[1].intValue;
	if(c < 0x0000 || c > 0x10FFFF) { /* FIXME: out of unicode range */
		KReturn(KNULL(String));
	}
	char buf[5] = {0};
	size_t length = 0;
	int policy = 0;
	if(c <= 0x007F) { /* 1 byte */
		buf[0] = (char)c;
		length = 1;
		policy |= StringPolicy_ASCII;
	}
	else if(c <= 0x07FF) { /* 2 bytes */
		buf[0] = (char)(0xC0 | (c >> 6));
		buf[1] = (char)(0x80 | (c & 0x3F));
		length = 2;
		policy |= StringPolicy_UTF8;
	}
	else if(c <= 0xFFFF) { /* 3 bytes */
		buf[0] = (char)(0xE0 | (c >> 12));
		buf[1] = (char)(0x80 | ((c >> 6) & 0x3F));
		buf[2] = (char)(0x80 | (c & 0x3F));
		length = 3;
		policy |= StringPolicy_UTF8;
	}
	else { /* 4 bytes */
		buf[0] = (char)(0xF0 | (c >> 18));
		buf[1] = (char)(0x80 | ((c >> 12) & 0x3F));
		buf[2] = (char)(0x80 | ((c >> 6) & 0x3F));
		buf[3] = (char)(0x80 | (c & 0x3F));
		length = 4;
		policy |= StringPolicy_UTF8;
	}
	KReturn(KLIB new_kString(kctx, OnStack, (const char *)buf, length, policy));
}

//## String String.slice(int start, int end);
static KMETHOD String_slice(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *thisString = sfp[0].asString;
	kint_t start = kString_getIndex(kctx, sfp, thisString, sfp[1].intValue);
	kint_t end = kString_getIndex(kctx, sfp, thisString, sfp[2].intValue);
	size_t length = (end < start) ? 0 : end - start;
	KReturn(new_SubString(kctx, OnStack, thisString, start, length, sfp));
}

//## String[] String.split();
static KMETHOD String_split(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = (kArray *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	KLIB kArray_add(kctx, a, sfp[0].asString);
	KReturn(a);
}

//## String[] String.split(String separator);
static KMETHOD String_splitWithSeparator(KonohaContext *kctx, KonohaStack *sfp)
{
	INIT_GCSTACK();
	kString *s0 = sfp[0].asString;
	kString *separator = sfp[1].asString;
	kArray *resultArray = (kArray *)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), 0);

	const char *start = S_text(s0);
	const char *end = start + S_size(s0);
	if(S_size(separator) == 0) {
		size_t i;
		for(i = 0; i < S_size(s0); i++) {
			if(kString_is(ASCII, s0)) {
				KLIB new_kString(kctx, resultArray, S_text(s0) + i, 1, StringPolicy_ASCII);
			}
			else {
				new_SubString(kctx, resultArray, s0, i, 1, sfp);
			}
		}
	}
	else {
		while(start < end) {
			const char *res = strstr(start, S_text(separator));
			if(res == NULL) break;
			if(res - start > 0) {
				KLIB new_kString(kctx, resultArray, start, res - start, StringPolicy_maskASCII(s0));
			}
			start = res + S_size(separator);
		}
		KLIB new_kString(kctx, resultArray, start, strlen(start), StringPolicy_maskASCII(s0));
	}
	KReturnWith(resultArray, RESET_GCSTACK());
}

//## String[] String.split(String separator, int limit);
static KMETHOD String_splitwithSeparatorLimit(KonohaContext *kctx, KonohaStack *sfp)
{
	INIT_GCSTACK();
	kString *thisString = sfp[0].asString;
	kString *separator = sfp[1].asString;
	kint_t limit = sfp[2].intValue;
	size_t length = kString_getMultibyteSize(kctx, thisString);
	if(limit < 0) {
		/* ignore limit */
		limit = length;
	}
	kArray *resultArray = (kArray *)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), 0);
	const char *start = S_text(thisString);
	const char *end = start + S_size(thisString);
	if(S_size(separator) == 0) {
		size_t i;
		if((size_t)limit > length) {
			limit = length;
		}
		for(i = 0; i < (size_t)limit; i++) {
			if(kString_is(ASCII, thisString)) {
				KLIB new_kString(kctx, resultArray, S_text(thisString) + i, 1, StringPolicy_ASCII);
			}
			else {
				new_SubString(kctx, resultArray, thisString, i, 1, sfp);
			}
		}
	}
	else {
		length = 0;
		while(start < end && length < (size_t)limit) {
			const char *res = strstr(start, S_text(separator));
			if(res == NULL) break;
			if(res - start > 0) {
				KLIB new_kString(kctx, resultArray, start, res - start, StringPolicy_maskASCII(thisString));
				length++;
			}
			start = res + S_size(separator);
		}
		if(length < (size_t)(limit - 1)) {
			KLIB new_kString(kctx, resultArray, start, strlen(start), StringPolicy_maskASCII(thisString));
		}
	}
	KReturnWith(resultArray, RESET_GCSTACK());
}

//## String String.substring(int indexA);
static KMETHOD String_substring(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *thisString = sfp[0].asString;
	kint_t indexA = kString_checkIndex(kctx, sfp, thisString, sfp[1].intValue);
	size_t length = kString_getMultibyteSize(kctx, thisString);
	KReturn(new_SubString(kctx, OnStack, thisString, indexA, length, sfp));
}

//## String String.substring(int indexA, int indexB);
static KMETHOD String_substringwithIndexB(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *thisString = sfp[0].asString;
	kint_t indexA = kString_checkIndex(kctx, sfp, thisString, sfp[1].intValue);
	kint_t indexB = kString_checkIndex(kctx, sfp, thisString, sfp[2].intValue);
	if(indexA > indexB) {
		/* swap */
		size_t tmp = indexA;
		indexA = indexB;
		indexB = tmp;
	}
	size_t length = indexB - indexA;
	KReturn(new_SubString(kctx, OnStack, thisString, indexA, length, sfp));
}

//## String String.valueOf();
static KMETHOD String_valueOf(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(sfp[0].asString);
}

// --------------------------------------------------------------------------
// String Interpolation

/* copied from src/sugar/sugarfunc.h */
static kString *kToken_resolvedEscapeSequence(KonohaContext *kctx, kToken *tk, size_t start)
{
	KGrowingBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	const char *text = S_text(tk->text) + start;
	const char *end  = S_text(tk->text) + S_size(tk->text);
	KLIB Kwb_write(kctx, &wb, S_text(tk->text), start);
	while(text < end) {
		int ch = *text;
		if(ch == '\\' && *(text+1) != '\0') {
			switch (*(text+1)) {
			/*
			 * compatible with ECMA-262
			 * http://ecma-international.org/ecma-262/5.1/#sec-7.8.4
			 */
			case 'b':  ch = '\b'; text++; break;
			case 't':  ch = '\t'; text++; break;
			case 'n':  ch = '\n'; text++; break;
			case 'v':  ch = '\v'; text++; break;
			case 'f':  ch = '\f'; text++; break;
			case 'r':  ch = '\r'; text++; break;
			case '"':  ch = '"';  text++; break;
			case '\'': ch = '\''; text++; break;
			case '\\': ch = '\\'; text++; break;
			default: return NULL;
			}
		}
		{
			char buf[1] = {ch};
			KLIB Kwb_write(kctx, &wb, (const char *)buf, 1);
		}
		text++;
	}
	kString *s = KLIB new_kString(kctx, OnGcStack, KLIB Kwb_top(kctx, &wb, 1), Kwb_bytesize(&wb), 0);
	KLIB Kwb_free(&wb);
	return s;
}

static kString *remove_escapes(KonohaContext *kctx, kToken *tk)
{
	kString *text = tk->text;
	if(kToken_is(RequiredReformat, tk)) {
		const char *escape = strchr(S_text(text), '\\');
		DBG_ASSERT(escape != NULL);
		text = kToken_resolvedEscapeSequence(kctx, tk, escape - S_text(text));
	}
	return text;
}

static KMETHOD TypeCheck_ExtendedTextLiteral(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kToken  *tk   = expr->termToken;
	INIT_GCSTACK();
	kString *text = remove_escapes(kctx, tk);
	if(text == NULL) {
		KReturnWith(K_NULLEXPR, RESET_GCSTACK());
	}

	const char *start = NULL, *end = NULL;
	const char *str = S_text(text);
	expr = SUGAR kExpr_setConstValue(kctx, expr, TY_String, UPCAST(text));
	kNameSpace *ns = Stmt_nameSpace(stmt);
	kMethod *concat = KLIB kNameSpace_getMethodByParamSizeNULL(kctx, ns, TY_String, MN_("+"), 1);

	expr = new_ConstValueExpr(kctx, TY_String, UPCAST(TS_EMPTY));
	while(true) {
		start = strstr(str, "${");
		if(start == NULL) {
			break;
		}
		if(start == strstr(str, "${}")) {
			str += 3;
			continue;
		}
		end = strchr(start, '}');
		if(end == NULL) {
			break;
		}

		KGrowingBuffer wb;
		KLIB Kwb_init(&(kctx->stack->cwb), &wb);
		KLIB Kwb_write(kctx, &wb, "(", 1);
		KLIB Kwb_write(kctx, &wb, start+2, end-(start+2));
		KLIB Kwb_write(kctx, &wb, ")", 1);

		TokenSequence range = {ns, KonohaContext_getSugarContext(kctx)->preparedTokenList};
		TokenSequence_push(kctx, range);
		const char *buf = KLIB Kwb_top(kctx, &wb, 1);
		SUGAR TokenSequence_tokenize(kctx, &range, buf, 0);

		{
			TokenSequence tokens = {ns, KonohaContext_getSugarContext(kctx)->preparedTokenList};
			TokenSequence_push(kctx, tokens);
			SUGAR TokenSequence_resolved(kctx, &tokens, NULL, &range, range.beginIdx);
			/* +1 means for skiping first indent token. */
			kExpr *newexpr = SUGAR kStmt_parseExpr(kctx, stmt, tokens.tokenList, tokens.beginIdx+1, tokens.endIdx, NULL);
			TokenSequence_pop(kctx, tokens);

			if(start - str > 0) {
				kExpr *first = new_ConstValueExpr(kctx, TY_String,
						UPCAST(KLIB new_kString(kctx, OnGcStack, str, (start - str), 0)));
				expr = SUGAR new_TypedCallExpr(kctx, stmt, gma, TY_String, concat, 2, expr, first);
			}
			expr = SUGAR new_TypedCallExpr(kctx, stmt, gma, TY_String, concat, 2, expr, newexpr);
		}
		TokenSequence_pop(kctx, range);
		KLIB Kwb_free(&wb);
		str = end + 1;
	}

	if((start == NULL) || (start != NULL && end == NULL)) {
		kExpr *rest = new_ConstValueExpr(kctx, TY_String,
				UPCAST(KLIB new_kString(kctx, OnGcStack, str, strlen(str), 0)));
		expr = SUGAR new_TypedCallExpr(kctx, stmt, gma, TY_String, concat, 2, expr, rest);
	}
	KReturnWith(expr, RESET_GCSTACK());
}


// --------------------------------------------------------------------------

#define _Public kMethod_Public
#define _Static kMethod_Static
#define _Const  kMethod_Const
#define _JS     kMethod_JSCompatible
#define _Im     kMethod_Immutable
#define _F(F)   (intptr_t)(F)

#define CT_StringArray0                CT_p0(kctx, CT_Array, TY_String)

static kbool_t string_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	int FN_s = FN_("s");
	int FN_n = FN_("n");
	int FN_index = FN_("index");
	int FN_start = FN_("start");
	ktype_t TY_StringArray0 = CT_StringArray0->typeId;

	KDEFINE_SYNTAX SYNTAX[] = {
		{ KW_TextPattern, 0,  NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_ExtendedTextLiteral, },
		{ KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX);

	kMethod *concat = KLIB kNameSpace_getMethodByParamSizeNULL(kctx, ns, TY_String, MN_("+"), 1);
	if(concat != NULL) {
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
		_JS|_Public|_Const|_Im, _F(String_opLT),  TY_boolean, TY_String, MN_("<"),  1, TY_String, FN_s,
		_JS|_Public|_Const|_Im, _F(String_opLTE),  TY_boolean, TY_String, MN_("<="),  1, TY_String, FN_s,
		_JS|_Public|_Const|_Im, _F(String_opGT),  TY_boolean, TY_String, MN_(">"),  1, TY_String, FN_s,
		_JS|_Public|_Const|_Im, _F(String_opGTE),  TY_boolean, TY_String, MN_(">="),  1, TY_String, FN_s,
		_JS|_Public|_Const|_Im, _F(String_getSize), TY_int, TY_String, MN_("getlength"), 0,
		_JS|_Public|_Const|_Im, _F(String_get), TY_String, TY_String, MN_("charAt"), 1, TY_int, FN_index,
		_JS|_Public|_Const|_Im, _F(String_charCodeAt), TY_int, TY_String, MN_("charCodeAt"), 1, TY_int, FN_index,
		_JS|_Public|_Static|_Const|_Im, _F(String_fromCharCode), TY_String, TY_String, MN_("fromCharCode"), 1, TY_int, FN_n,
		_JS|_Public|_Const|_Im, _F(String_indexOf), TY_int, TY_String, MN_("indexOf"), 1, TY_String, FN_("searchvalue"),
		_JS|_Public|_Const|_Im, _F(String_indexOfwithStart), TY_int, TY_String, MN_("indexOf"), 2, TY_String, FN_("searchvalue"), TY_int, FN_start,
		_JS|_Public|_Const|_Im, _F(String_lastIndexOf), TY_int, TY_String, MN_("lastIndexOf"), 1, TY_String, FN_("searchvalue"),
		_JS|_Public|_Const|_Im, _F(String_lastIndexOfwithStart), TY_int, TY_String, MN_("lastIndexOf"), 2, TY_String, FN_("searchvalue"), TY_int, FN_start,
		_JS|_Public|_Const|_Im, _F(String_localeCompare), TY_int, TY_String, MN_("localeCompare"), 1, TY_String, FN_("that"),
		_JS|_Public|_Const|_Im, _F(String_replace), TY_String, TY_String, MN_("replace"), 2, TY_String, FN_("searchvalue"), TY_String, FN_("newvalue"),
		_JS|_Public|_Const|_Im, _F(String_indexOf), TY_int, TY_String, MN_("search"), 1, TY_String, FN_("searchvalue"),
		_JS|_Public|_Const|_Im, _F(String_substr), TY_String, TY_String, MN_("slice"), 1, TY_int, FN_start,
		_JS|_Public|_Const|_Im, _F(String_slice), TY_String, TY_String, MN_("slice"), 2, TY_int, FN_start, TY_int, FN_("end"),
		_JS|_Public|_Im, _F(String_split), TY_StringArray0, TY_String, MN_("split"), 0,
		_JS|_Public|_Im, _F(String_splitWithSeparator), TY_StringArray0, TY_String, MN_("split"), 1, TY_String, FN_("separator"),
		_JS|_Public|_Im, _F(String_splitwithSeparatorLimit), TY_StringArray0, TY_String, MN_("split"), 2, TY_String, FN_("separator"), TY_int, FN_("limit"),
		_JS|_Public|_Const|_Im, _F(String_substr), TY_String, TY_String, MN_("substr"), 1, TY_int, FN_start,
		_JS|_Public|_Const|_Im, _F(String_substrWithLength), TY_String, TY_String, MN_("substr"), 2, TY_int, FN_start, TY_int, FN_("length"),
		_JS|_Public|_Const|_Im, _F(String_substring), TY_String, TY_String, MN_("substring"), 1, TY_int, FN_("indexA"),
		_JS|_Public|_Const|_Im, _F(String_substringwithIndexB), TY_String, TY_String, MN_("substring"), 2, TY_int, FN_("indexA"), TY_int, FN_("indexB"),
		_JS|_Public|_Const|_Im, _F(String_toLower), TY_String, TY_String, MN_("toLowerCase"), 0,
		_JS|_Public|_Const|_Im, _F(String_toUpper), TY_String, TY_String, MN_("toUpperCase"), 0,
		_JS|_Public|_Const|_Im, _F(String_valueOf), TY_String, TY_String, MN_("valueOf"), 0,
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

	KSET_TYFUNC(CT_String, unbox, String2, trace);
	KSET_TYFUNC(CT_String, free, String2, trace);
	KSET_TYFUNC(CT_String, reftrace, String2, trace);
	KSET_KLIB(new_kString, trace);

	return true;
}

static kbool_t string_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* string_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", "1.0");
	d.initPackage    = string_initPackage;
	d.setupPackage   = string_setupPackage;
	return &d;
}

#ifdef __cplusplus
}
#endif
