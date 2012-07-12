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

#include <konoha2/konoha2.h>
#include <konoha2/sugar.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MASK_LINER    ((S_FLAG_LINER   ) << S_FLAG_MASK_BASE)
#define MASK_NOFREE   ((S_FLAG_NOFREE  ) << S_FLAG_MASK_BASE)
#define MASK_ROPE     ((S_FLAG_ROPE    ) << S_FLAG_MASK_BASE)
#define MASK_INLINE   ((S_FLAG_INLINE  ) << S_FLAG_MASK_BASE)
#define MASK_EXTERNAL ((S_FLAG_EXTERNAL) << S_FLAG_MASK_BASE)

#define S_len(s) ((s)->length)
typedef struct StringBase {
	kObjectHeader h;
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

static StringBase *new_StringBase(CTX, uint32_t mask)
{
	StringBase *s = (StringBase *) new_(String, 0);
	StringBase_unsetFlag(s, MASK_INLINE);
	StringBase_setFlag(s, mask);
	return s;
}

static StringBase *InlineString_new(CTX, StringBase *base, const char *text, size_t len)
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

static StringBase *ExternalString_new(CTX, StringBase *base,
		const char *text, size_t len)
{
	ExternalString *s = (ExternalString *) base;
	StringBase_setFlag(base, MASK_EXTERNAL);
	s->base.length = len;
	s->text = (char *) text;
	return base;
}

static StringBase *LinerString_new(CTX, StringBase *base,
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

static StringBase *RopeString_new(CTX, StringBase *left, StringBase *right, size_t len)
{
	RopeString *s = (RopeString *) new_StringBase(_ctx, MASK_ROPE);
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

static kString *Knew_String(CTX, const char *text, size_t len, int policy)
{
	StringBase *s = (StringBase *) new_StringBase(_ctx, 0);
	if (len < SIZEOF_INLINETEXT)
		return (kString*) InlineString_new(_ctx, s, text, len);
	if(TFLAG_is(int, policy, SPOL_TEXT))
		return (kString*) ExternalString_new(_ctx, s, text, len);
	return (kString*) LinerString_new(_ctx, s, text, len);
}

static void String2_free(CTX, kObject *o)
{
	StringBase *base = (StringBase*) o;
	if (S_isMallocText(base)) {
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

static LinerString *RopeString_flatten(CTX, RopeString *rope)
{
	size_t length = S_len((StringBase *) rope);
	char *dest = (char *) KMALLOC(length+1);
	size_t llen = S_len(rope->left);
	write_text(rope->left,  dest,llen);
	write_text(rope->right, dest+llen, length - llen);
	return RopeString_toLinerString(rope, dest, length);
}

static char *String_getReference(CTX, StringBase *s)
{
	uint32_t flag = S_flag(s);
	switch (flag) {
		case S_FLAG_LINER:
		case S_FLAG_EXTERNAL:
		case S_FLAG_INLINE:
			return ((LinerString*)s)->text;
		case S_FLAG_ROPE:
			return RopeString_flatten(_ctx, (RopeString*)s)->text;
		default:
			/*unreachable*/
			assert(0);
	}
	return NULL;
}

extern struct _kObject** KONOHA_reftail(CTX, size_t size);

static void StringBase_reftrace(CTX, StringBase *s)
{
	while (1) {
		if (unlikely(!StringBase_isRope(s)))
			break;
		RopeString *rope = (RopeString *) s;
		StringBase_reftrace(_ctx, rope->left);
		BEGIN_REFTRACE(3);
		KREFTRACEv(rope);//FIXME reftracing rope is needed?
		KREFTRACEv(rope->left);
		KREFTRACEv(rope->right);
		END_REFTRACE();
		s = rope->right;
	}
}

static void String2_reftrace(CTX, kObject *o)
{
	StringBase_reftrace(_ctx, (StringBase *) o);
}

static uintptr_t String2_unbox(CTX, kObject *o)
{
	StringBase *s = (StringBase*)o;
	return (uintptr_t) String_getReference(_ctx, s);
}

static StringBase *StringBase_concat(CTX, kString *s0, kString *s1)
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
		InlineString *ret = (InlineString *) new_StringBase(_ctx, MASK_INLINE);
		char *s0 = String_getReference(_ctx, left);
		char *s1 = String_getReference(_ctx, right);
		assert(length < SIZEOF_INLINETEXT);
		ret->base.length = length;
		memcpy(ret->inline_text, s0, llen);
		memcpy(ret->inline_text + llen, s1, rlen);
		ret->inline_text[length] = '\0';
		ret->text = ret->inline_text;
		return (StringBase *) ret;
	}
	return RopeString_new(_ctx, left, right, length);
}

/* ------------------------------------------------------------------------ */
//## @Const method String String.concat(String rhs);
//## @Const method String String.opADD(String rhs);

static KMETHOD Rope_opADD(CTX, ksfp_t *sfp _RIX)
{
	kString *lhs = sfp[0].s, *rhs = sfp[1].s;
	RETURN_(StringBase_concat(_ctx, lhs, rhs));
}

/* ------------------------------------------------------------------------ */

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

static kbool_t rope_initPackage(CTX, kNameSpace *ks, int argc, const char**args, kline_t pline)
{
	kMethod *mtd = kNameSpace_getMethodNULL(ks, TY_String, MN_("opADD"));
	if (mtd) {
		kMethod_setFunc(mtd, Rope_opADD);
	} else {
		int FN_x = FN_("x");
		KDEFINE_METHOD MethodData[] = {
			_Public|_Const, _F(Rope_opADD), TY_String, TY_String, MN_("opADD"), 1, TY_String, FN_x,
			DEND
		};
		kNameSpace_loadMethodData(ks, MethodData);
	}
	KSET_CLASSFUNC(CT_String, unbox, String2, pline);
	KSET_CLASSFUNC(CT_String, free, String2, pline);
	KSET_CLASSFUNC(CT_String, reftrace, String2, pline);
	KSET_KLIB(new_String, pline);
	return true;
}

static kbool_t rope_setupPackage(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

static kbool_t rope_initNameSpace(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

static kbool_t rope_setupNameSpace(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* rope_init(void)
{
	static const KDEFINE_PACKAGE d = {
		KPACKNAME("Rope", "1.0"),
		.initPackage = rope_initPackage,
		.setupPackage = rope_setupPackage,
		.initNameSpace = rope_initNameSpace,
		.setupNameSpace = rope_setupNameSpace,
	};
	assert(sizeof(StringBase) <= sizeof(kString));
	assert(sizeof(ExternalString) <= sizeof(kString));
	assert(sizeof(InlineString) <= sizeof(kString));
	assert(sizeof(LinerString) <= sizeof(kString));
	assert(sizeof(RopeString) <= sizeof(kString));
	return &d;
}

#ifdef __cplusplus
}
#endif
