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
static KMETHOD Object_toString(CTX, ksfp_t *sfp _RIX)
{
	kwb_t wb;
	kwb_init(&(_ctx->stack->cwb), &wb);
	O_ct(sfp[0].o)->p(_ctx, sfp, 0, &wb, 0);
	kString* s = new_kString(kwb_top(&wb, 1), kwb_bytesize(&wb), 0);
	kwb_free(&wb);
	RETURN_(s);
}

//## @Const method Boolean Boolean.opNOT();
static KMETHOD Boolean_opNOT(CTX, ksfp_t *sfp _RIX)
{
	RETURNb_(!sfp[0].bvalue);
}

//## @Const method Int Int.opMINUS();
static KMETHOD Int_opMINUS(CTX, ksfp_t *sfp _RIX)
{
	RETURNi_(-(sfp[0].ivalue));
}

//## @Const method Int Int.opADD(Int x);
static KMETHOD Int_opADD(CTX, ksfp_t *sfp _RIX)
{
	RETURNi_(sfp[0].ivalue + sfp[1].ivalue);
}

//## @Const method Int Int.opSUB(Int x);
static KMETHOD Int_opSUB(CTX, ksfp_t *sfp _RIX)
{
	RETURNi_(sfp[0].ivalue - sfp[1].ivalue);
}

//## @Const method Int Int.opMUL(Int x);
static KMETHOD Int_opMUL(CTX, ksfp_t *sfp _RIX)
{
	RETURNi_(sfp[0].ivalue * sfp[1].ivalue);
}

//## @Const method Int Int.opDIV(Int x);
static KMETHOD Int_opDIV(CTX, ksfp_t *sfp _RIX)
{
	kint_t n = sfp[1].ivalue;
	if(unlikely(n == 0)) {
		kreportf(CRIT_, sfp[K_RTNIDX].uline, "Script!!: zero divided");
	}
	RETURNi_(sfp[0].ivalue / n);
}

//## @Const method Int Int.opMOD(Int x);
static KMETHOD Int_opMOD(CTX, ksfp_t *sfp _RIX)
{
	kint_t n = sfp[1].ivalue;
	if(unlikely(n == 0)) {
		kreportf(CRIT_, sfp[K_RTNIDX].uline, "Script!!: zero divided");
	}
	RETURNi_(sfp[0].ivalue % n);
}

//## @Const method Boolean Int.opEQ(Int x);
static KMETHOD Int_opEQ(CTX, ksfp_t *sfp _RIX)
{
	RETURNb_(sfp[0].ivalue == sfp[1].ivalue);
}

//## @Const method Boolean Int.opNEQ(Int x);
static KMETHOD Int_opNEQ(CTX, ksfp_t *sfp _RIX)
{
	RETURNb_(sfp[0].ivalue != sfp[1].ivalue);
}

//## @Const method Boolean Int.opLT(Int x);
static KMETHOD Int_opLT(CTX, ksfp_t *sfp _RIX)
{
	RETURNb_(sfp[0].ivalue < sfp[1].ivalue);
}

//## @Const method Boolean Int.opLTE(Int x);
static KMETHOD Int_opLTE(CTX, ksfp_t *sfp _RIX)
{
	RETURNb_(sfp[0].ivalue <= sfp[1].ivalue);
}

//## @Const method Boolean Int.opGT(Int x);
static KMETHOD Int_opGT(CTX, ksfp_t *sfp _RIX)
{
	RETURNb_(sfp[0].ivalue > sfp[1].ivalue);
}

//## @Const method Boolean Int.opGTE(Int x);
static KMETHOD Int_opGTE(CTX, ksfp_t *sfp _RIX)
{
	RETURNb_(sfp[0].ivalue >= sfp[1].ivalue);
}

//## @Const method String Int.toString();
static KMETHOD Int_toString(CTX, ksfp_t *sfp _RIX)
{
	char buf[40];
	PLAT snprintf_i(buf, sizeof(buf), "%ld", (intptr_t)sfp[0].ivalue);
	RETURN_(new_kString(buf, strlen(buf), SPOL_ASCII));
}

//## @Const method String String.toInt();
static KMETHOD String_toInt(CTX, ksfp_t *sfp _RIX)
{
	RETURNi_((kint_t)strtoll(S_text(sfp[0].s), NULL, 10));
}

//## @Const @Immutable method String String.opAdd(@Coercion String x);
static KMETHOD String_opADD(CTX, ksfp_t *sfp _RIX)
{
	kString *lhs = sfp[0].s, *rhs = sfp[1].s;
	int spol = (S_isASCII(lhs) && S_isASCII(rhs)) ? SPOL_ASCII : SPOL_UTF8;
	kString *s = new_kString(NULL, S_size(lhs)+S_size(rhs), spol|SPOL_NOCOPY);
	memcpy(s->buf, S_text(lhs), S_size(lhs));
	memcpy(s->buf+S_size(lhs), S_text(rhs), S_size(rhs));
	RETURN_(s);
}

//## @Const method Boolean String.equals(String s);
//## @Const method Boolean String.opEQ(String s);
static KMETHOD String_opEQ(CTX, ksfp_t *sfp _RIX)
{
	kString *s0 = sfp[0].s;
	kString *s1 = sfp[1].s;
	if(S_size(s0) == S_size(s1)) {
		RETURNb_(strncmp(S_text(s0), S_text(s1), S_size(s0)) == 0);
	}
	RETURNb_(0);
}

static KMETHOD String_opNEQ(CTX, ksfp_t *sfp _RIX)
{
	kString *s0 = sfp[0].s;
	kString *s1 = sfp[1].s;
	if(S_size(s0) == S_size(s1)) {
		RETURNb_(strncmp(S_text(s0), S_text(s1), S_size(s0)) != 0);
	}
	RETURNb_(1);
}

//## This Func.new(Object self, Method mtd);
static KMETHOD Func_new(CTX, ksfp_t *sfp _RIX)
{
	struct _kFunc *fo = (struct _kFunc*)sfp[0].fo;
	KSETv(fo->self, sfp[1].o);
	KSETv(fo->mtd,  sfp[2].mtd);
	RETURN_(fo);
}

//## @Hidden T0 Func.invoke();
static KMETHOD Func_invoke(CTX, ksfp_t *sfp _RIX)
{
	kFunc* fo = sfp[0].fo;
	DBG_P("fo=%s", CT_t(O_ct(fo)));
	DBG_ASSERT(IS_Func(fo));
	DBG_ASSERT(IS_Method(fo->mtd));
	DBG_P("fo->mtd->fcall_1 == %p", fo->mtd->fcall_1);
	KSETv(sfp[0].o, fo->self);
	KSELFCALL(sfp, fo->mtd);
}

int konoha_AssertResult = 0;

//## @Const @Static void System.assert(boolean x)
static KMETHOD System_assert(CTX, ksfp_t *sfp _RIX)
{
	kbool_t cond = sfp[1].bvalue;
	if (cond == false) {
		kline_t pline  = sfp[K_RTNIDX].uline;
		konoha_AssertResult = 1;
		kreportf(CRIT_, pline, "Assert!!");
	}
}

//## method void System.p(@Coercion String msg);
static KMETHOD System_p(CTX, ksfp_t *sfp _RIX)
{
	kline_t uline = sfp[K_RTNIDX].uline;
	kreportf(PRINT_, uline, "%s", S_text(sfp[1].s));
}

//## method void System.gc();
static KMETHOD System_gc(CTX, ksfp_t *sfp _RIX)
{
	MODGC_gc_invoke(_ctx, 1/* needsCStackTrace */);
}

#ifdef __cplusplus
}
#endif
