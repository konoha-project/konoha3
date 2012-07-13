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
/* String */
static KMETHOD Object_toString(KonohaContext *kctx, KonohaStack *sfp)
{
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	O_ct(sfp[0].toObject)->p(kctx, sfp, 0, &wb, 0);
	kString* s = KLIB new_kString(kctx, KLIB Kwb_top(kctx, &wb, 1), Kwb_bytesize(&wb), 0);
	KLIB Kwb_free(&wb);
	RETURN_(s);
}

//## @Const method Boolean Boolean.opNOT();
static KMETHOD Boolean_opNOT(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(!sfp[0].bvalue);
}

//## @Const method Int Int.opMINUS();
static KMETHOD Int_opMINUS(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(-(sfp[0].ivalue));
}

//## @Const method Int Int.opADD(Int x);
static KMETHOD Int_opADD(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(sfp[0].ivalue + sfp[1].ivalue);
}

//## @Const method Int Int.opSUB(Int x);
static KMETHOD Int_opSUB(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(sfp[0].ivalue - sfp[1].ivalue);
}

//## @Const method Int Int.opMUL(Int x);
static KMETHOD Int_opMUL(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(sfp[0].ivalue * sfp[1].ivalue);
}

//## @Const method Int Int.opDIV(Int x);
static KMETHOD Int_opDIV(KonohaContext *kctx, KonohaStack *sfp)
{
	kint_t n = sfp[1].ivalue;
	if(unlikely(n == 0)) {
		kreportf(CritTag, sfp[K_RTNIDX].uline, "Script!!: zero divided");
	}
	RETURNi_(sfp[0].ivalue / n);
}

//## @Const method Int Int.opMOD(Int x);
static KMETHOD Int_opMOD(KonohaContext *kctx, KonohaStack *sfp)
{
	kint_t n = sfp[1].ivalue;
	if(unlikely(n == 0)) {
		kreportf(CritTag, sfp[K_RTNIDX].uline, "Script!!: zero divided");
	}
	RETURNi_(sfp[0].ivalue % n);
}

//## @Const method Boolean Int.opEQ(Int x);
static KMETHOD Int_opEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(sfp[0].ivalue == sfp[1].ivalue);
}

//## @Const method Boolean Int.opNEQ(Int x);
static KMETHOD Int_opNEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(sfp[0].ivalue != sfp[1].ivalue);
}

//## @Const method Boolean Int.opLT(Int x);
static KMETHOD Int_opLT(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(sfp[0].ivalue < sfp[1].ivalue);
}

//## @Const method Boolean Int.opLTE(Int x);
static KMETHOD Int_opLTE(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(sfp[0].ivalue <= sfp[1].ivalue);
}

//## @Const method Boolean Int.opGT(Int x);
static KMETHOD Int_opGT(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(sfp[0].ivalue > sfp[1].ivalue);
}

//## @Const method Boolean Int.opGTE(Int x);
static KMETHOD Int_opGTE(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(sfp[0].ivalue >= sfp[1].ivalue);
}

//## @Const method String Int.toString();
static KMETHOD Int_toString(KonohaContext *kctx, KonohaStack *sfp)
{
	char buf[40];
	PLATAPI snprintf_i(buf, sizeof(buf), "%ld", (intptr_t)sfp[0].ivalue);
	RETURN_(KLIB new_kString(kctx, buf, strlen(buf), SPOL_ASCII));
}

//## @Const method String String.toInt();
static KMETHOD String_toInt(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_((kint_t)strtoll(S_text(sfp[0].s), NULL, 10));
}

//## @Const @Immutable method String String.opAdd(@Coercion String x);
static KMETHOD String_opADD(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *lhs = sfp[0].s, *rhs = sfp[1].toString;
	int spol = (S_isASCII(lhs) && S_isASCII(rhs)) ? SPOL_ASCII : SPOL_UTF8;
	kString *s = KLIB new_kString(kctx, NULL, S_size(lhs)+S_size(rhs), spol|SPOL_NOCOPY);
	memcpy(s->buf, S_text(lhs), S_size(lhs));
	memcpy(s->buf+S_size(lhs), S_text(rhs), S_size(rhs));
	RETURN_(s);
}

//## @Const method Boolean String.equals(String s);
//## @Const method Boolean String.opEQ(String s);
static KMETHOD String_opEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].s;
	kString *s1 = sfp[1].toString;
	if(S_size(s0) == S_size(s1)) {
		RETURNb_(strncmp(S_text(s0), S_text(s1), S_size(s0)) == 0);
	}
	RETURNb_(0);
}

static KMETHOD String_opNEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].s;
	kString *s1 = sfp[1].toString;
	if(S_size(s0) == S_size(s1)) {
		RETURNb_(strncmp(S_text(s0), S_text(s1), S_size(s0)) != 0);
	}
	RETURNb_(1);
}

//## This Func.new(Object self, Method mtd);
static KMETHOD Func_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kFuncVar *fo = (kFuncVar*)sfp[0].toFunc;
	KSETv(fo->self, sfp[1].toObject);
	KSETv(fo->mtd,  sfp[2].toMethod);
	RETURN_(fo);
}

//## @Hidden T0 Func.invoke();
static KMETHOD Func_invoke(KonohaContext *kctx, KonohaStack *sfp)
{
	kFunc* fo = sfp[0].toFunc;
	DBG_P("fo=%s", CT_t(O_ct(fo)));
	DBG_ASSERT(IS_Func(fo));
	DBG_ASSERT(IS_Method(fo->mtd));
	DBG_P("fo->mtd->invokeMethodFunc == %p", fo->mtd->invokeMethodFunc);
	KSETv(sfp[0].toObject, fo->self);
	KSELFCALL(sfp, fo->mtd);
}

int konoha_AssertResult = 0;

//## @Const @Static void System.assert(boolean x)
static KMETHOD System_assert(KonohaContext *kctx, KonohaStack *sfp)
{
	kbool_t cond = sfp[1].bvalue;
	if (cond == false) {
		kfileline_t pline  = sfp[K_RTNIDX].uline;
		konoha_AssertResult = 1;
		kreportf(CritTag, pline, "Assert!!");
	}
}

//## method void System.p(@Coercion String msg);
static KMETHOD System_p(KonohaContext *kctx, KonohaStack *sfp)
{
	kfileline_t uline = sfp[K_RTNIDX].uline;
	kreportf(NoneTag, uline, "%s", S_text(sfp[1].toString));
}

//## method void System.gc();
static KMETHOD System_gc(KonohaContext *kctx, KonohaStack *sfp)
{
	MODGC_gc_invoke(kctx, 1/* needsCStackTrace */);
}

#ifdef __cplusplus
}
#endif
