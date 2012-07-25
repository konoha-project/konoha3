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
#ifndef INT_GLUE_H_
#define INT_GLUE_H_

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/klib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */

//## @Const method Int Int.opPlus();
static KMETHOD Int_opPlus(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(+(sfp[0].intValue));
}

static KMETHOD Int_opCompl (KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(~sfp[0].intValue);
}

static KMETHOD Int_opLSHIFT (KonohaContext *kctx, KonohaStack *sfp)
{
	int lshift = sfp[1].intValue;
	RETURNi_(sfp[0].intValue << lshift);
}

static KMETHOD Int_opRSHIFT (KonohaContext *kctx, KonohaStack *sfp)
{
	int rshift = sfp[1].intValue;
	RETURNi_(sfp[0].intValue >> rshift);
}

static KMETHOD Int_opAND(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(sfp[0].intValue & sfp[1].intValue);
}

static KMETHOD Int_opOR(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(sfp[0].intValue | sfp[1].intValue);
}

static KMETHOD Int_opXOR(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(sfp[0].intValue ^ sfp[1].intValue);
}

/* ------------------------------------------------------------------------ */

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Im       kMethod_Immutable
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

static kbool_t int_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	int FN_x = FN_("x");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(Int_opPlus), TY_Int, TY_Int, MN_("+"), 0,
		_Public|_Const|_Im, _F(Int_opCompl), TY_Int, TY_Int, MN_("~"), 0,
		_Public|_Const|_Im, _F(Int_opLSHIFT), TY_Int, TY_Int, MN_("<<"), 1, TY_Int, FN_x,
		_Public|_Const|_Im, _F(Int_opRSHIFT), TY_Int, TY_Int, MN_(">>"), 1, TY_Int, FN_x,
		_Public|_Const|_Im, _F(Int_opAND), TY_Int, TY_Int, MN_("&"), 1, TY_Int, FN_x,
		_Public|_Const|_Im, _F(Int_opOR ), TY_Int, TY_Int, MN_("|"), 1, TY_Int, FN_x,
		_Public|_Const|_Im, _F(Int_opXOR), TY_Int, TY_Int, MN_("^"), 1, TY_Int, FN_x,
//		_Public|_Const|_Im, _F(Int_opINC), TY_Int, TY_Int, MN_("opINC"), 0,
//		_Public|_Const|_Im, _F(Int_opDEC), TY_Int, TY_Int, MN_("opDEC"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	KDEFINE_INT_CONST IntData[] = {
			{"INT_MAX", TY_Int, KINT_MAX},
			{"INT_MIN", TY_Int, KINT_MIN},
			{NULL},
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(IntData), pline);
	return true;
}

static kbool_t int_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t int_initNameSpace(KonohaContext *kctx,  kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
			{ .keyword = SYM_("~"), _OP, .precedence_op1  = 16,},
			{ .keyword = SYM_("<<"), _OP, .precedence_op2 = 128,},
			{ .keyword = SYM_(">>"), _OP, .precedence_op2 = 128,},
			{ .keyword = SYM_("&"), _OP,  .precedence_op2 = 640,},
			{ .keyword = SYM_("|"), _OP,  .precedence_op2 = 768,},
			{ .keyword = SYM_("^"), _OP,  .precedence_op2 = 896,},
//			{ TOKEN("++"), _OP, .op1 = "opINC", .precedence_op2 = 16, .flag = SYNFLAG_ExprPostfixOp2, },
//			{ TOKEN("--"), _OP, .op1 = "opDEC", .precedence_op2 = 16, .flag = SYNFLAG_ExprPostfixOp2,},
			{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX);
	SugarSyntaxVar *syn = (SugarSyntaxVar*)SUGAR kNameSpace_getSyntax(kctx, ns, SYM_("+"), 0);
	if(syn != NULL) {
		syn->precedence_op1  = 16;
	}
	return true;
}

static kbool_t int_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

#ifdef __cplusplus
}
#endif

#endif /* INT_GLUE_H_ */
