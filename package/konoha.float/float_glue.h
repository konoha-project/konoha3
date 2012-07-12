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

#ifndef FLOAT_GLUE_H_
#define FLOAT_GLUE_H_

#include<float.h>
#include "ext/mt19937ar.h"

// Int
static void Float_init(CTX, kObject *o, void *conf)
{
	struct _kNumber *n = (struct _kNumber*)o;  // kFloat has the same structure
	n->ndata = (uintptr_t)conf;  // conf is unboxed data
}

static void Float_p(CTX, ksfp_t *sfp, int pos, kwb_t *wb, int level)
{
	kwb_printf(wb, KFLOAT_FMT, sfp[pos].fvalue);
}

static void kmodfloat_setup(CTX, struct kmodshare_t *def, int newctx)
{
}

static void kmodfloat_reftrace(CTX, struct kmodshare_t *baseh)
{
}

static void kmodfloat_free(CTX, struct kmodshare_t *baseh)
{
	KFREE(baseh, sizeof(kmodfloat_t));
}

// --------------------------------------------------------------------------

/* float + float */
static KMETHOD Float_opADD(CTX, ksfp_t *sfp _RIX)
{
	RETURNf_(sfp[0].fvalue + sfp[1].fvalue);
}

/* float - float */
static KMETHOD Float_opSUB(CTX, ksfp_t *sfp _RIX)
{
	RETURNf_(sfp[0].fvalue - sfp[1].fvalue);
}

/* float * float */
static KMETHOD Float_opMUL(CTX, ksfp_t *sfp _RIX)
{
	RETURNf_(sfp[0].fvalue * sfp[1].fvalue);
}

/* float / float */
static KMETHOD Float_opDIV(CTX, ksfp_t *sfp _RIX)
{
	kfloat_t n = sfp[1].fvalue;
	if(unlikely(n == 0.0)) {
		kreportf(CRIT_, sfp[K_RTNIDX].uline, "Script!!: zero divided");
	}
	RETURNf_(sfp[0].fvalue / n);
}

/* float == float */
static KMETHOD Float_opEQ(CTX, ksfp_t *sfp _RIX)
{
	RETURNb_(sfp[0].fvalue == sfp[1].fvalue);
}

/* float != float */
static KMETHOD Float_opNEQ(CTX, ksfp_t *sfp _RIX)
{
	RETURNb_(sfp[0].fvalue != sfp[1].fvalue);
}

/* float < float */
static KMETHOD Float_opLT(CTX, ksfp_t *sfp _RIX)
{
	RETURNb_(sfp[0].fvalue < sfp[1].fvalue);
}

/* float <= float */
static KMETHOD Float_opLTE(CTX, ksfp_t *sfp _RIX)
{
	RETURNb_(sfp[0].fvalue <= sfp[1].fvalue);
}

/* float > float */
static KMETHOD Float_opGT(CTX, ksfp_t *sfp _RIX)
{
	RETURNb_(sfp[0].fvalue > sfp[1].fvalue);
}

/* float >= float */
static KMETHOD Float_opGTE(CTX, ksfp_t *sfp _RIX)
{
	RETURNb_(sfp[0].fvalue >= sfp[1].fvalue);
}

/* float to int */
static KMETHOD Float_toInt(CTX, ksfp_t *sfp _RIX)
{
	RETURNi_((kint_t)sfp[0].fvalue);
}

/* float >= float */
static KMETHOD Int_toFloat(CTX, ksfp_t *sfp _RIX)
{
	RETURNf_((kfloat_t)sfp[0].ivalue);
}

/* float to String */
static KMETHOD Float_toString(CTX, ksfp_t *sfp _RIX)
{
	char buf[40];
	PLAT snprintf_i(buf, sizeof(buf), KFLOAT_FMT, sfp[0].fvalue);
	RETURN_(new_kString(buf, strlen(buf), SPOL_ASCII));
}

/* String to float */
static KMETHOD String_toFloat(CTX, ksfp_t *sfp _RIX)
{
	RETURNf_((kfloat_t)strtod(S_text(sfp[0].s), NULL));
}

//## @Const method Int Int.opMINUS();
static KMETHOD Float_opMINUS(CTX, ksfp_t *sfp _RIX)
{
	RETURNf_(-(sfp[0].fvalue));
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

static KMETHOD Float_random(CTX, ksfp_t *sfp _RIX)
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

static	kbool_t float_initPackage(CTX, kNameSpace *ks, int argc, const char**args, kline_t pline)
{
	kmodfloat_t *base = (kmodfloat_t*)KCALLOC(sizeof(kmodfloat_t), 1);
	base->h.name     = "float";
	base->h.setup    = kmodfloat_setup;
	base->h.reftrace = kmodfloat_reftrace;
	base->h.free     = kmodfloat_free;
	Konoha_setModule(MOD_float, &base->h, pline);

	KDEFINE_CLASS defFloat = {
		STRUCTNAME(Float),
		.cflag = CFLAG_Int,
		.init = Float_init,
		.p     = Float_p,
	};
	base->cFloat = Konoha_addClassDef(ks->packid, PN_konoha, NULL, &defFloat, pline);
	int FN_x = FN_("x");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(Float_opADD), TY_Float, TY_Float, MN_("opADD"), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Float_opSUB), TY_Float, TY_Float, MN_("opSUB"), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Float_opMUL), TY_Float, TY_Float, MN_("opMUL"), 1, TY_Float, FN_x,
		_Public|_Im, _F(Float_opDIV), TY_Float, TY_Float, MN_("opDIV"), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Float_opEQ),  TY_Boolean, TY_Float, MN_("opEQ"), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Float_opNEQ), TY_Boolean, TY_Float, MN_("opNEQ"), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Float_opLT),  TY_Boolean, TY_Float, MN_("opLT"), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Float_opLTE), TY_Boolean, TY_Float, MN_("opLTE"), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Float_opGT),  TY_Boolean, TY_Float, MN_("opGT"), 1, TY_Float, FN_x,
		_Public|_Const|_Im, _F(Float_opGTE), TY_Boolean, TY_Float, MN_("opGTE"), 1, TY_Float, FN_x,
		_Public|_Const|_Im|_Coercion, _F(Float_toInt), TY_Int, TY_Float, MN_to(TY_Int), 0,
		_Public|_Const|_Im|_Coercion, _F(Int_toFloat), TY_Float, TY_Int, MN_to(TY_Float), 0,
		_Public|_Const|_Im, _F(Float_toString), TY_String, TY_Float, MN_to(TY_String), 0,
		_Public|_Const|_Im, _F(String_toFloat), TY_Float, TY_String, MN_to(TY_Float), 0,
		_Public|_Const|_Im, _F(Float_opMINUS), TY_Float, TY_Float, MN_("opMINUS"), 0,
		_Public|_Static|_Im, _F(Float_random), TY_Float, TY_Float, MN_("random"), 0,
		DEND,
	};
	kNameSpace_loadMethodData(ks, MethodData);
	KDEFINE_FLOAT_CONST FloatData[] = {
		{"FLOAT_EPSILON", TY_Float, DBL_EPSILON},
		{}
	};
	kNameSpace_loadConstData(ks, FloatData, pline);
	return true;
}

static kbool_t float_setupPackage(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

//----------------------------------------------------------------------------

static KMETHOD ExprTyCheck_Float(CTX, ksfp_t *sfp _RIX)
{
	USING_SUGAR;
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->tk;
	sfp[4].fvalue = strtod(S_text(tk->text), NULL);
	RETURN_(kExpr_setNConstValue(expr, TY_Float, sfp[4].ndata));
}

static kbool_t float_initNameSpace(CTX,  kNameSpace *ks, kline_t pline)
{
	USING_SUGAR;
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .kw = SYM_("float"), .type = TY_Float, },
		{ .kw = SYM_("double"), .type = TY_Float, },
		{ .kw = SYM_("$FLOAT"), ExprTyCheck_(Float), },
		{ .kw = KW_END, },
	};
	SUGAR NameSpace_defineSyntax(_ctx, ks, SYNTAX);
	return true;
}

static kbool_t float_setupNameSpace(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

#endif /* FLOAT_GLUE_H_ */
