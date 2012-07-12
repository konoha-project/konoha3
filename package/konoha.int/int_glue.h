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

#include <konoha2/konoha2.h>
#include <konoha2/sugar.h>
#include <konoha2/klib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */

static KMETHOD Int_opLSHIFT (CTX, ksfp_t *sfp _RIX)
{
	int lshift = sfp[1].ivalue;
	RETURNi_(sfp[0].ivalue << lshift);
}

static KMETHOD Int_opRSHIFT (CTX, ksfp_t *sfp _RIX)
{
	int rshift = sfp[1].ivalue;
	RETURNi_(sfp[0].ivalue >> rshift);
}

/* ------------------------------------------------------------------------ */

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Im       kMethod_Immutable
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

static kbool_t int_initPackage(CTX, kNameSpace *ns, int argc, const char**args, kline_t pline)
{
	int FN_x = FN_("x");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(Int_opLSHIFT), TY_Int, TY_Int, MN_("opLSHIFT"), 1, TY_Int, FN_x,
		_Public|_Const|_Im, _F(Int_opRSHIFT), TY_Int, TY_Int, MN_("opRSHIFT"), 1, TY_Int, FN_x,
//		_Public|_Const|_Im, _F(Int_opINC), TY_Int, TY_Int, MN_("opINC"), 0,
//		_Public|_Const|_Im, _F(Int_opDEC), TY_Int, TY_Int, MN_‘("opDEC"), 0,
		DEND,
	};
	kNameSpace_loadMethodData(ns, MethodData);
	KDEFINE_INT_CONST IntData[] = {
			{"INT_MAX", TY_Int, KINT_MAX},
			{"INT_MIN", TY_Int, KINT_MIN},
			{NULL},
	};
	kNameSpace_loadConstData(ns, IntData, pline);
	return true;
}

static kbool_t int_setupPackage(CTX, kNameSpace *ns, kline_t pline)
{
	return true;
}

static kbool_t int_initNameSpace(CTX,  kNameSpace *ns, kline_t pline)
{
	USING_SUGAR;
	KDEFINE_SYNTAX SYNTAX[] = {
			{ .kw = SYM_("<<"), _OP, .op2 = "opLSHIFT", .priority_op2 = 128,},
			{ .kw = SYM_(">>"), _OP, .op2 = "opRSHIFT", .priority_op2 = 128,},
//			{ TOKEN("++"), _OP, .op1 = "opINC", .priority_op2 = 16, .flag = SYNFLAG_ExprPostfixOp2, },
//			{ TOKEN("--"), _OP, .op1 = "opDEC", .priority_op2 = 16, .flag = SYNFLAG_ExprPostfixOp2,},
			{ .kw = KW_END, },
	};
	SUGAR NameSpace_defineSyntax(_ctx, ns, SYNTAX);
	return true;
}

static kbool_t int_setupNameSpace(CTX, kNameSpace *ns, kline_t pline)
{
	return true;
}

#ifdef __cplusplus
}
#endif

#endif /* INT_GLUE_H_ */
