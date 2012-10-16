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

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#define strtoll _strtoi64
#endif

/* String */
//## String Object.toString();
static KMETHOD Object_toString(KonohaContext *kctx, KonohaStack *sfp)
{
	KGrowingBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	if(TY_isUnbox(O_typeId(sfp[0].asObject))) {
		//sfp[0].unboxValue = (sfp[0].asNumber)->unboxValue;
		O_ct(sfp[0].asObject)->p(kctx, sfp, 0, &wb);
	}
	else {
		kObject_writeToBuffer(kctx, sfp[0].asObject, false/*delim*/, &wb, sfp, 0);
	}
	kString* returnValue = KLIB new_kString(kctx, OnStack, KLIB Kwb_top(kctx, &wb, 1), Kwb_bytesize(&wb), 0);
	KLIB Kwb_free(&wb);
	KReturn(returnValue);
}

//## @Const method Boolean Boolean.opNOT();
static KMETHOD Boolean_opNOT(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(!sfp[0].boolValue);
}

//## @Const method Boolean Boolean.opEQ(Boolean x);
static KMETHOD Boolean_opEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].boolValue == sfp[1].boolValue);
}

//## @Const method Int Int.opMINUS();
static KMETHOD Int_opMINUS(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(-(sfp[0].intValue));
}

//## @Const method Int Int.opADD(Int x);
static KMETHOD Int_opADD(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue + sfp[1].intValue);
}

//## @Const method Int Int.opSUB(Int x);
static KMETHOD Int_opSUB(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue - sfp[1].intValue);
}

//## @Const method Int Int.opMUL(Int x);
static KMETHOD Int_opMUL(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue * sfp[1].intValue);
}

//## @Const method Int Int.opDIV(Int x);
static KMETHOD Int_opDIV(KonohaContext *kctx, KonohaStack *sfp)
{
	kint_t n = sfp[1].intValue;
	if(unlikely(n == 0)) {
		KMakeTrace(trace, sfp);
		KLIB KonohaRuntime_raise(kctx, EXPT_("ZeroDivided"), NULL, trace);
	}
	KReturnUnboxValue(sfp[0].intValue / n);
}

//## @Const method Int Int.opMOD(Int x);
static KMETHOD Int_opMOD(KonohaContext *kctx, KonohaStack *sfp)
{
	kint_t n = sfp[1].intValue;
	if(unlikely(n == 0)) {
		KMakeTrace(trace, sfp);
		KLIB KonohaRuntime_raise(kctx, EXPT_("ZeroDivided"), NULL, trace);
	}
	KReturnUnboxValue(sfp[0].intValue % n);
}

//## @Const method Boolean Int.opEQ(Int x);
static KMETHOD Int_opEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue == sfp[1].intValue);
}

//## @Const method Boolean Int.opNEQ(Int x);
static KMETHOD Int_opNEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue != sfp[1].intValue);
}

//## @Const method Boolean Int.opLT(Int x);
static KMETHOD Int_opLT(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue < sfp[1].intValue);
}

//## @Const method Boolean Int.opLTE(Int x);
static KMETHOD Int_opLTE(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue <= sfp[1].intValue);
}

//## @Const method Boolean Int.opGT(Int x);
static KMETHOD Int_opGT(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue > sfp[1].intValue);
}

//## @Const method Boolean Int.opGTE(Int x);
static KMETHOD Int_opGTE(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue >= sfp[1].intValue);
}

//## @Const method String Int.asString();
static KMETHOD Int_toString(KonohaContext *kctx, KonohaStack *sfp)
{
	char buf[40];
	PLATAPI snprintf_i(buf, sizeof(buf), "%ld", (intptr_t)sfp[0].intValue);
	KReturn(KLIB new_kString(kctx, OnStack, buf, strlen(buf), StringPolicy_ASCII));
}

//## @Const method Object Boolean.box();
static KMETHOD Boolean_box(KonohaContext *kctx, KonohaStack *sfp)
{
	kBoolean *o = !!(sfp[0].unboxValue) ? K_TRUE : K_FALSE;
	sfp[K_RTNIDX].unboxValue = sfp[0].unboxValue;
	KReturn(o);
}

//## @Const method Object Int.box();
static KMETHOD Int_box(KonohaContext *kctx, KonohaStack *sfp)
{
	KonohaClass *c = KGetReturnType(sfp);
	DBG_P("reqt=%s", CT_t(c));
	DBG_ASSERT(CT_isUnbox(c));
	sfp[K_RTNIDX].unboxValue = sfp[0].unboxValue;
	KReturn(KLIB new_kObject(kctx, OnStack, c, sfp[0].unboxValue));
}

//## @Const method String String.toInt();
static KMETHOD String_toInt(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue((kint_t)strtoll(S_text(sfp[0].asString), NULL, 10));
}

//## @Const @Immutable method String String.opAdd(@Coercion String x);
static KMETHOD String_opADD(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *leftHandString = sfp[0].asString, *rightHandString = sfp[1].asString;
	int spol = (kString_is(ASCII, leftHandString) && kString_is(ASCII, rightHandString)) ? StringPolicy_ASCII : StringPolicy_UTF8;
	kString *s = KLIB new_kString(kctx, OnStack, NULL, S_size(leftHandString)+S_size(rightHandString), spol|StringPolicy_NOCOPY);
	memcpy(s->buf,  S_text(leftHandString), S_size(leftHandString));
	memcpy(s->buf + S_size(leftHandString), S_text(rightHandString), S_size(rightHandString));
	KReturn(s);
}

//## @Const method Boolean String.equals(String s);
//## @Const method Boolean String.opEQ(String s);
static KMETHOD String_opEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	if(S_size(s0) == S_size(s1)) {
		KReturnUnboxValue(strncmp(S_text(s0), S_text(s1), S_size(s0)) == 0);
	}
	KReturnUnboxValue(0);
}

static KMETHOD String_opNEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kString *s1 = sfp[1].asString;
	if(S_size(s0) == S_size(s1)) {
		KReturnUnboxValue(strncmp(S_text(s0), S_text(s1), S_size(s0)) != 0);
	}
	KReturnUnboxValue(1);
}

//## This Func.new(Object self, Method mtd);
static KMETHOD Func_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kFuncVar *fo = (kFuncVar*)sfp[0].asFunc;
	KFieldSet(fo, fo->self, sfp[1].asObject);
	KFieldSet(fo, fo->mtd,  sfp[2].asMethod);
	KReturn(fo);
}

//## @Hidden T0 Func.invoke();
static KMETHOD Func_invoke(KonohaContext *kctx, KonohaStack *sfp)
{
	kFunc* fo = sfp[0].asFunc;
	DBG_ASSERT(IS_Func(fo));
	DBG_ASSERT(IS_Method(fo->mtd));
	KUnsafeFieldSet(sfp[0].asObject, fo->self);
	KSELFCALL(sfp, fo->mtd);
}

int konoha_detectFailedAssert = false;

//## @Const @Static void System.assert(boolean x)
static KMETHOD System_assert(KonohaContext *kctx, KonohaStack *sfp)
{
	kbool_t cond = sfp[1].boolValue;
//	konoha_detectFailedAssert = false;
	if(cond == false) {
		KMakeTrace(trace, sfp);
		konoha_detectFailedAssert = true;
		KLIB KonohaRuntime_raise(kctx, EXPT_("Assertion"), NULL, trace);
	}
}

//## method void System.p(@Coercion String msg);
static KMETHOD System_p(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *text = (IS_NULL(sfp[1].asString)) ? K_NULLTEXT : S_text(sfp[1].asString);
	KMakeTrace(trace, sfp);
	kreportf(NoneTag, trace, "%s", text);
}

//## method void System.gc();
static KMETHOD System_gc(KonohaContext *kctx, KonohaStack *sfp)
{
	KLIB KscheduleGC(kctx->gcContext);
}

// --------------------------------------------------------------------------
#define _Public    kMethod_Public
#define _Const     kMethod_Const
#define _Static    kMethod_Static
#define _Immutable kMethod_Immutable
#define _Coercion  kMethod_Coercion
#define _Hidden    kMethod_Hidden
#define _Override  kMethod_Override
#define _F(F)      (intptr_t)(F)

static void Konoha_loadDefaultMethod(KonohaContext *kctx)
{
	int FN_x = FN_("x");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Immutable|_Const, _F(Object_toString), TY_String, TY_Object, MN_to(TY_String), 0,
		_Public|_Immutable|_Const, _F(Boolean_opNOT), TY_boolean, TY_boolean, MN_("!"), 0,
		_Public|_Immutable|_Const, _F(Boolean_opEQ), TY_boolean, TY_boolean, MN_("=="), 1, TY_boolean, FN_x,
		_Public|_Immutable|_Const, _F(Int_opNEQ), TY_boolean, TY_boolean, MN_("!="), 1, TY_boolean, FN_x,
		_Public|_Immutable|_Const, _F(Int_opMINUS), TY_int, TY_int, MN_("-"), 0,
		_Public|_Immutable|_Const, _F(Int_opADD), TY_int, TY_int, MN_("+"), 1, TY_int, FN_x,
		_Public|_Immutable|_Const, _F(Int_opSUB), TY_int, TY_int, MN_("-"), 1, TY_int, FN_x,
		_Public|_Immutable|_Const, _F(Int_opMUL), TY_int, TY_int, MN_("*"), 1, TY_int, FN_x,
		/* opDIV and opMOD raise zero divided exception. Don't set _Const */
		_Public|_Immutable, _F(Int_opDIV), TY_int, TY_int, MN_("/"), 1, TY_int, FN_x,
		_Public|_Immutable, _F(Int_opMOD), TY_int, TY_int, MN_("%"), 1, TY_int, FN_x,
		_Public|_Immutable|_Const, _F(Int_opEQ),  TY_boolean, TY_int, MN_("=="),  1, TY_int, FN_x,
		_Public|_Immutable|_Const, _F(Int_opNEQ), TY_boolean, TY_int, MN_("!="), 1, TY_int, FN_x,
		_Public|_Immutable|_Const, _F(Int_opLT),  TY_boolean, TY_int, MN_("<"),  1, TY_int, FN_x,
		_Public|_Immutable|_Const, _F(Int_opLTE), TY_boolean, TY_int, MN_("<="), 1, TY_int, FN_x,
		_Public|_Immutable|_Const, _F(Int_opGT),  TY_boolean, TY_int, MN_(">"),  1, TY_int, FN_x,
		_Public|_Immutable|_Const, _F(Int_opGTE), TY_boolean, TY_int, MN_(">="), 1, TY_int, FN_x,
		_Public|_Immutable|_Const,  _F(Int_toString), TY_String, TY_int, MN_to(TY_String), 0,
		_Public|_Immutable|_Const|kMethod_SmartReturn|kMethod_Hidden, _F(Boolean_box), TY_Object, TY_boolean, MN_box, 0,
		_Public|_Immutable|_Const|kMethod_SmartReturn|kMethod_Hidden, _F(Int_box), TY_Object, TY_int, MN_box, 0,
		_Public|_Immutable|_Const, _F(String_opEQ),  TY_boolean, TY_String, MN_("=="),  1, TY_String, FN_x ,
		_Public|_Immutable|_Const, _F(String_opNEQ), TY_boolean, TY_String, MN_("!="), 1, TY_String, FN_x ,
		_Public|_Immutable|_Const, _F(String_toInt), TY_int, TY_String, MN_to(TY_int), 0,
		_Public|_Immutable|_Const, _F(String_opADD), TY_String, TY_String, MN_("+"), 1, TY_String, FN_x | FN_COERCION,
		_Public|_Const|_Hidden, _F(Func_new), TY_Func, TY_Func, MN_new, 2, TY_Object, FN_x, TY_Method, FN_x,
		_Public|kMethod_SmartReturn|_Hidden, _F(Func_invoke), TY_Object, TY_Func, MN_("invoke"), 0,
		_Static|_Public|_Immutable, _F(System_assert), TY_void, TY_NameSpace, MN_("assert"), 1, TY_boolean, FN_x,
		_Static|_Public|_Immutable, _F(System_p), TY_void, TY_System, MN_("p"), 1, TY_String, FN_("s") | FN_COERCION,
		_Static|_Public|_Immutable, _F(System_gc), TY_void, TY_System, MN_("gc"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, NULL, MethodData);
}

#ifdef __cplusplus
}
#endif
