/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
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

#ifndef ROPE_STRING_H
#define ROPE_STRING_H

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
	kObjectHeader h;
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
	PLATAPI GCModule.WriteBarrier(kctx, (kObject *)rope);
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
	kString_Set(ASCII, (kStringVar *)s, (ch < 128));
}

static kString *new_kString(KonohaContext *kctx, kArray *gcstack, const char *text, size_t len, int policy)
{
	kStringBase *s = (kStringBase *) new_kStringBase(kctx, gcstack, 0);
	if(KFlag_Is(int, policy, StringPolicy_ASCII)) {
		kString_Set(ASCII, s, 1);
	} else if(KFlag_Is(int, policy, StringPolicy_UTF8)) {
		kString_Set(ASCII, s, 0);
	} else {
		checkASCII(kctx, s, text, len);
	}

	if(len < SIZEOF_INLINETEXT)
		return (kString *) kStringBase_InitInline(kctx, s, text, len);
	if(KFlag_Is(int, policy, StringPolicy_TEXT))
		return (kString *) kStringBase_InitExternal(kctx, s, text, len);
	return (kString *) kStringBase_InitLiner(kctx, s, text, len);
}

static void String2_Free(KonohaContext *kctx, kObject *o)
{
	kStringBase *base = (kStringBase *) o;
	if((kStringBase_flag(base) & S_FLAG_EXTERNAL) == S_FLAG_LINER) {
		assert(((kLinerString *)base)->text == ((kString *)base)->buf);
		KFree(((kLinerString *)base)->text, StringBase_length(base)+1);
	}
}

static void Stack_Init(KonohaContext *kctx, KGrowingArray *stack)
{
	KLIB KArray_Init(kctx, stack, 4 * sizeof(kStringBase**));
}

static void Stack_Push(KonohaContext *kctx, KGrowingArray *stack, kStringBase *str)
{
	size_t index = stack->bytesize / sizeof(kStringBase *);
	if(stack->bytesize == stack->bytemax) {
		KLIB KArray_Expand(kctx, stack, stack->bytemax * 2);
	}
	stack->ObjectItems[index] = (kObject *) str;
	stack->bytesize += sizeof(kStringBase *);
}

static kStringBase *Stack_Pop(KonohaContext *kctx, KGrowingArray *stack)
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
	KLIB KArray_Free(kctx, stack);
}

static void copyText(KonohaContext *kctx, KGrowingArray *stack, char *dest, size_t size)
{
	kStringBase *base;
	kRopeString *str;
	while((base = Stack_Pop(kctx, stack)) != NULL) {
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
				Stack_Push(kctx, stack, str->right);
				Stack_Push(kctx, stack, str->left);
				break;
		}
	}
}

static kLinerString *kRopeString_flatten(KonohaContext *kctx, kRopeString *rope)
{
	size_t length = StringBase_length((kStringBase *) rope);
	char  *dest = (char *) KMalloc_UNTRACE(length+1);
	KGrowingArray stack;
	Stack_Init(kctx, &stack);
	Stack_Push(kctx, &stack, rope->right);
	Stack_Push(kctx, &stack, rope->left);
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

static void String2_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kStringBase *s = (kStringBase *) o;
	if(kStringBase_isRope(s)) {
		kRopeString *rope = (kRopeString *) s;
		KRefTrace(rope->left);
		KRefTrace(rope->right);
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

	bool isASCII = kString_Is(ASCII, left) && kString_Is(ASCII, right);
	kStringBase *result = NULL;

	if(length + 1 < SIZEOF_INLINETEXT) {
		char *leftChar = kStringBase_getTextReference(kctx, left);
		char *rightChar = kStringBase_getTextReference(kctx, right);
		kInlineString *inlined = (kInlineString *) new_kStringBase(kctx, gcstack, MASK_INLINE);
		inlined->base.length = length;
		memcpy(inlined->inline_text, leftChar, leftLen);
		memcpy(inlined->inline_text + leftLen, rightChar, rightLen);
		inlined->inline_text[length] = '\0';
		inlined->text = inlined->inline_text;

		result = (kStringBase *) inlined;
	} else {
		result = kStringBase_InitRope(kctx, gcstack, left, right, length);
	}
	kString_Set(ASCII, (kStringVar *)result, isASCII);
	return result;
}

/* ------------------------------------------------------------------------ */

//## @Static String String.concat(String lhs, String rhs);

static KMETHOD StringUtil_concat(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *lhs = sfp[1].asString, *rhs = sfp[2].asString;
	KReturn(kStringBase_concat(kctx, OnStack, lhs, rhs));
}

static KMETHOD Rope_opADD(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *lhs = sfp[0].asString, *rhs = sfp[1].asString;
	KReturn(kStringBase_concat(kctx, OnStack, lhs, rhs));
}

static kbool_t LoadRopeMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace, int KType_StringUtil)
{
	int FN_s = KFieldName_("s");
	int FN_t = KFieldName_("t");
	kMethod *concat = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, KClass_String, KMethodName_("+"), 1, KMethodMatch_NoOption);
	if(concat != NULL) {
		KLIB kMethod_SetFunc(kctx, concat, Rope_opADD);
	} else {
		KDEFINE_METHOD MethodData[] = {
			_Public|_Const|_Im, _F(Rope_opADD), KType_String, KType_String, KMethodName_("+"), 1, KType_String, FN_s,
			DEND
		};
		KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	}
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(StringUtil_concat), KType_String, KType_StringUtil, KMethodName_("concat"), 2, KType_String, FN_t, KType_String, FN_s,
		DEND
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* end of include guard */
