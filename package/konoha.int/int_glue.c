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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/klib.h>

#ifdef __cplusplus
extern "C" {
#endif

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
#define _F(F)   (intptr_t)(F)

static kbool_t int_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	int FN_x = FN_("x");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(Int_opPlus), TY_int, TY_int, MN_("+"), 0,
		_Public|_Const|_Im, _F(Int_opCompl), TY_int, TY_int, MN_("~"), 0,
		_Public|_Const|_Im, _F(Int_opLSHIFT), TY_int, TY_int, MN_("<<"), 1, TY_int, FN_x,
		_Public|_Const|_Im, _F(Int_opRSHIFT), TY_int, TY_int, MN_(">>"), 1, TY_int, FN_x,
		_Public|_Const|_Im, _F(Int_opAND), TY_int, TY_int, MN_("&"), 1, TY_int, FN_x,
		_Public|_Const|_Im, _F(Int_opOR ), TY_int, TY_int, MN_("|"), 1, TY_int, FN_x,
		_Public|_Const|_Im, _F(Int_opXOR), TY_int, TY_int, MN_("^"), 1, TY_int, FN_x,
		//_Public|_Const|_Im, _F(Int_opINC), TY_int, TY_int, MN_("opINC"), 0,
		//_Public|_Const|_Im, _F(Int_opDEC), TY_int, TY_int, MN_("opDEC"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	KDEFINE_INT_CONST IntData[] = {
		{"INT_MAX", TY_int, KINT_MAX},
		{"INT_MIN", TY_int, KINT_MIN},
		{NULL},
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(IntData), pline);
	return true;
}

static kbool_t int_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

//static KMETHOD parseNonDecimalNumber(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kTokenVar *tk = (kTokenVar *)sfp[1].o;
//	const char *source = S_text(sfp[2].asString);
//	const char *start = source, *end;
//	int c = *source++;
//	if (c != '0') {
//		/* It do not seem as NonDecimalNumber */
//		RETURNi_(0);
//	}
//	/*
//	 * DIGIT  = 0-9
//	 * DIGITS = DIGIT | DIGIT DIGITS
//	 * TAG    = "0x"  | "0b"
//	 * INT_NON_DECIMAL = TAG DIGITS
//	 */
//	int base = 0;
//	kint_t num = 0;
//	c = *source++;
//	switch (c) {
//		case 'b':
//			base = 2;  break;
//		case 'x':
//			base = 16; break;
//		default:
//			RETURNi_(0);
//	}
//	for (c = *source++; '0' <= c && c <= '9' && c != 0; c = *source++) {
//		if (c == '_') continue;
//		num = num * base + (c - '0');
//	}
//	end = source;
//	if (IS_NOTNULL(tk)) {
//		/* skip unit */
//		while (isalpha(*source) && *source != 0)
//			source++;
//		KSETv(tk, tk->text, KLIB new_kString(kctx, start, end - start - 1, SPOL_ASCII));
//		tk->unresolvedTokenType = TokenType_INT;
//	}
//	RETURNi_(source - start - 1);
//}

static kbool_t int_initNameSpace(KonohaContext *kctx,  kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("~"), .precedence_op1 = C_PRECEDENCE_PREUNARY,},
		{ .keyword = SYM_("<<"),  .precedence_op2 = C_PRECEDENCE_SHIFT,},
		{ .keyword = SYM_(">>"),  .precedence_op2 = C_PRECEDENCE_SHIFT,},
		{ .keyword = SYM_("&"),   .precedence_op2 = C_PRECEDENCE_BITAND,},
		{ .keyword = SYM_("|"),   .precedence_op2 = C_PRECEDENCE_BITOR,},
		{ .keyword = SYM_("^"),   .precedence_op2 = C_PRECEDENCE_BITXOR,},
		//{ TOKEN("++"),  .op1 = "opINC", .precedence_op2 = C_PRECEDENCE_PREUNARY, .flag = SYNFLAG_ExprPostfixOp2, },
		//{ TOKEN("--"),  .op1 = "opDEC", .precedence_op2 = C_PRECEDENCE_PREUNARY, .flag = SYNFLAG_ExprPostfixOp2,},
		{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX);
	//kMethod *mtd = KLIB new_kMethod(kctx, 0, 0, 0, parseNonDecimalNumber);
	//kFunc *fo = GCSAFE_new(Func, (uintptr_t) mtd);
	//SUGAR kNameSpace_setTokenizeFunc(kctx, ns, '0', NULL, fo, 0);

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

KDEFINE_PACKAGE* int_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("int", "1.0"),
		.initPackage =int_initPackage,
		.setupPackage = int_setupPackage,
		.initNameSpace = int_initNameSpace,
		.setupNameSpace = int_setupNameSpace,
	};
	return &d;
}
#ifdef __cplusplus
}
#endif
