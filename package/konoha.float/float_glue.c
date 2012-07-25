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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/float.h>
#include "ext/mt19937ar.h"

#ifdef __cplusplus
extern "C" {
#endif

// --------------------------------------------------------------------------

// Int
static void Float_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kNumberVar *n = (kNumberVar*)o;  // kFloat has the same structure
	n->unboxValue = (uintptr_t)conf;  // conf is unboxed data
}

static void Float_p(KonohaContext *kctx, KonohaStack *sfp, int pos, KUtilsWriteBuffer *wb, int level)
{
	KLIB Kwb_printf(kctx, wb, KFLOAT_FMT, sfp[pos].floatValue);
}

static void kmodfloat_setup(KonohaContext *kctx, struct KonohaModule *def, int newctx)
{
}

static void kmodfloat_reftrace(KonohaContext *kctx, struct KonohaModule *baseh)
{
}

static void kmodfloat_free(KonohaContext *kctx, struct KonohaModule *baseh)
{
	KFREE(baseh, sizeof(kmodfloat_t));
}

// --------------------------------------------------------------------------

/* float + float */
static KMETHOD Float_opADD(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(sfp[0].floatValue + sfp[1].floatValue);
}

static KMETHOD Int_opADD(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(sfp[0].intValue + sfp[1].floatValue);
}

/* float - float */
static KMETHOD Float_opSUB(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(sfp[0].floatValue - sfp[1].floatValue);
}

static KMETHOD Int_opSUB(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(sfp[0].intValue - sfp[1].floatValue);
}

/* float * float */
static KMETHOD Float_opMUL(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(sfp[0].floatValue * sfp[1].floatValue);
}

/* float / float */
static KMETHOD Float_opDIV(KonohaContext *kctx, KonohaStack *sfp)
{
	kfloat_t n = sfp[1].floatValue;
	if(unlikely(n == 0.0)) {
		kreportf(CritTag, sfp[K_RTNIDX].uline, "Script!!: zero divided");
	}
	RETURNf_(sfp[0].floatValue / n);
}

static KMETHOD Int_opMUL(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(sfp[0].intValue * sfp[1].floatValue);
}

/* float / float */
static KMETHOD Int_opDIV(KonohaContext *kctx, KonohaStack *sfp)
{
	kfloat_t n = sfp[1].floatValue;
	if(unlikely(n == 0.0)) {
		kreportf(CritTag, sfp[K_RTNIDX].uline, "Script!!: zero divided");
	}
	RETURNf_(sfp[0].intValue / n);
}

/* float == float */
static KMETHOD Float_opEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(sfp[0].floatValue == sfp[1].floatValue);
}

/* float != float */
static KMETHOD Float_opNEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(sfp[0].floatValue != sfp[1].floatValue);
}

/* float < float */
static KMETHOD Float_opLT(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(sfp[0].floatValue < sfp[1].floatValue);
}

/* float <= float */
static KMETHOD Float_opLTE(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(sfp[0].floatValue <= sfp[1].floatValue);
}

/* float > float */
static KMETHOD Float_opGT(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(sfp[0].floatValue > sfp[1].floatValue);
}

/* float >= float */
static KMETHOD Float_opGTE(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(sfp[0].floatValue >= sfp[1].floatValue);
}

//////

static KMETHOD Int_opEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(sfp[0].intValue == sfp[1].floatValue);
}

/* float != float */
static KMETHOD Int_opNEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(sfp[0].intValue != sfp[1].floatValue);
}


/* float < float */
static KMETHOD Int_opLT(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(sfp[0].intValue < sfp[1].floatValue);
}

/* float <= float */
static KMETHOD Int_opLTE(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(sfp[0].intValue <= sfp[1].floatValue);
}

/* float > float */
static KMETHOD Int_opGT(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(sfp[0].intValue > sfp[1].floatValue);
}

/* float >= float */
static KMETHOD Int_opGTE(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(sfp[0].intValue >= sfp[1].floatValue);
}


/* float to int */
static KMETHOD Float_toInt(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_((kint_t)sfp[0].floatValue);
}

/* float >= float */
static KMETHOD Int_toFloat(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_((kfloat_t)sfp[0].intValue);
}

/* float to String */
static KMETHOD Float_toString(KonohaContext *kctx, KonohaStack *sfp)
{
	char buf[40];
	PLATAPI snprintf_i(buf, sizeof(buf), KFLOAT_FMT, sfp[0].floatValue);
	RETURN_(KLIB new_kString(kctx, buf, strlen(buf), SPOL_ASCII));
}

/* String to float */
static KMETHOD String_toFloat(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_((kfloat_t)strtod(S_text(sfp[0].s), NULL));
}

//## @Const method Int Int.opMINUS();
static KMETHOD Float_opMINUS(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(-(sfp[0].floatValue));
}

//double genrand64_real1(void);

static kfloat_t kfloat_rand(void)
{
#if defined(K_USING_NOFLOAT)
	return (kfloat_t)knh_rand();
#elif defined(K_USING_INT32)
	return (kfloat_t)genrand_real1();
#else
	return (kfloat_t)genrand64_real1();
#endif
}

static KMETHOD Float_random(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(kfloat_rand());
}

/* ------------------------------------------------------------------------ */

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Im       kMethod_Immutable
#define _Coercion kMethod_Coercion
#define _Static   kMethod_Static
#define _F(F)   (intptr_t)(F)

static kbool_t float_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	kmodfloat_t *base = (kmodfloat_t*)KCALLOC(sizeof(kmodfloat_t), 1);
	base->h.name     = "float";
	base->h.setup    = kmodfloat_setup;
	base->h.reftrace = kmodfloat_reftrace;
	base->h.free     = kmodfloat_free;
	KLIB Konoha_setModule(kctx, MOD_float, &base->h, pline);

	KDEFINE_CLASS defFloat = {
		STRUCTNAME(Float),
		.cflag = CFLAG_Int,
		.init = Float_init,
		.p     = Float_p,
	};
	base->cFloat = KLIB Konoha_defineClass(kctx, ns->packageId, PN_konoha, NULL, &defFloat, pline);
	int FN_x = FN_("x");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(Float_opADD), TY_Float, TY_Float, MN_("+"), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Int_opADD), TY_Float, TY_Int, MN_("+"), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Float_opSUB), TY_Float, TY_Float, MN_("-"), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Int_opSUB), TY_Float, TY_Int, MN_("-"), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Float_opMUL), TY_Float, TY_Float, MN_("*"), 1, TY_Float, FN_x,
		_Public|_Im, _F(Float_opDIV), TY_Float, TY_Float, MN_("/"), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Float_opEQ),  TY_Boolean, TY_Float, MN_("=="), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Float_opNEQ), TY_Boolean, TY_Float, MN_("!="), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Float_opLT),  TY_Boolean, TY_Float, MN_("<"), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Float_opLTE), TY_Boolean, TY_Float, MN_("<="), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Float_opGT),  TY_Boolean, TY_Float, MN_(">"), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Float_opGTE), TY_Boolean, TY_Float, MN_(">="), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Int_opMUL), TY_Float, TY_Int, MN_("*"), 1, TY_Float, FN_x,
		_Public|_Im, _F(Int_opDIV), TY_Float, TY_Int, MN_("/"), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Int_opEQ),  TY_Boolean, TY_Int, MN_("=="), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Int_opNEQ), TY_Boolean, TY_Int, MN_("!="), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Int_opLT),  TY_Boolean, TY_Int, MN_("<"), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Int_opLTE), TY_Boolean, TY_Int, MN_("<="), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Int_opGT),  TY_Boolean, TY_Int, MN_(">"), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Int_opGTE), TY_Boolean, TY_Int, MN_(">="), 1, TY_Float, FN_x,

		_Public|_Const|_Im|_Coercion, _F(Float_toInt), TY_Int, TY_Float, MN_to(TY_Int), 0,
		_Public|_Const|_Im|_Coercion, _F(Int_toFloat), TY_Float, TY_Int, MN_to(TY_Float), 0,
		_Public|_Const|_Im, _F(Float_toString), TY_String, TY_Float, MN_to(TY_String), 0,
		_Public|_Const|_Im, _F(String_toFloat), TY_Float, TY_String, MN_to(TY_Float), 0,
		_Public|_Const|_Im, _F(Float_opMINUS), TY_Float, TY_Float, MN_("-"), 0,
		_Public|_Static|_Im, _F(Float_random), TY_Float, TY_Float, MN_("random"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	KDEFINE_FLOAT_CONST FloatData[] = {
		{"FLOAT_EPSILON", TY_Float, DBL_EPSILON},
		{}
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(FloatData), pline);
	return true;
}

static kbool_t float_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

//----------------------------------------------------------------------------

static KMETHOD ExprTyCheck_Float(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->termToken;
	sfp[4].floatValue = strtod(S_text(tk->text), NULL);   // just using tramsformation float
	RETURN_(SUGAR kExpr_setUnboxConstValue(kctx, expr, TY_Float, sfp[4].unboxValue));
}

static kbool_t float_initNameSpace(KonohaContext *kctx,  kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("float"), .type = TY_Float, },
		{ .keyword = SYM_("double"), .type = TY_Float, },
		{ .keyword = SYM_("$FLOAT"), ExprTyCheck_(Float), },
		{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX);
	return true;
}

static kbool_t float_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* float_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("float", "1.0"),
		.initPackage = float_initPackage,
		.setupPackage = float_setupPackage,
		.initNameSpace = float_initNameSpace,
		.setupNameSpace = float_setupNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
