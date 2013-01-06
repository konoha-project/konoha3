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
#include <minikonoha/import/methoddecl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */
/* BitwiseOperator ( <<, >>, |, &, ^ )*/

static KMETHOD Int_opPlus(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(+(sfp[0].intValue));
}

static KMETHOD Int_opCompl (KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(~sfp[0].intValue);
}

static KMETHOD Int_opLSHIFT (KonohaContext *kctx, KonohaStack *sfp)
{
	int lshift = sfp[1].intValue;
	KReturnUnboxValue(sfp[0].intValue << lshift);
}

static KMETHOD Int_opRSHIFT (KonohaContext *kctx, KonohaStack *sfp)
{
	int rshift = sfp[1].intValue;
	KReturnUnboxValue(sfp[0].intValue >> rshift);
}

static KMETHOD Int_opAND(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue & sfp[1].intValue);
}

static KMETHOD Int_opOR(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue | sfp[1].intValue);
}

static KMETHOD Int_opXOR(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue ^ sfp[1].intValue);
}

/* ------------------------------------------------------------------------ */

static kbool_t BitwiseOperator_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	int FN_x = KFieldName_("x");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(Int_opPlus), KType_int, KType_int, KMethodName_("+"), 0,
		_Public|_Const|_Im, _F(Int_opCompl), KType_int, KType_int, KMethodName_("~"), 0,
		_Public|_Const|_Im, _F(Int_opLSHIFT), KType_int, KType_int, KMethodName_("<<"), 1, KType_int, FN_x,
		_Public|_Const|_Im, _F(Int_opRSHIFT), KType_int, KType_int, KMethodName_(">>"), 1, KType_int, FN_x,
		_Public|_Const|_Im, _F(Int_opAND), KType_int, KType_int, KMethodName_("&"), 1, KType_int, FN_x,
		_Public|_Const|_Im, _F(Int_opOR ), KType_int, KType_int, KMethodName_("|"), 1, KType_int, FN_x,
		_Public|_Const|_Im, _F(Int_opXOR), KType_int, KType_int, KMethodName_("^"), 1, KType_int, FN_x,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("~"),  0, NULL, 0,                   Precedence_CStylePREUNARY, NULL, NULL, NULL, NULL, NULL, },
		{ KSymbol_("<<"), 0, NULL, Precedence_CStyleSHIFT,  0,                     NULL, NULL, NULL, NULL, NULL, },
		{ KSymbol_(">>"), 0, NULL, Precedence_CStyleSHIFT,  0,                     NULL, NULL, NULL, NULL, NULL, },
		{ KSymbol_("&"),  0, NULL, Precedence_CStyleBITAND, 0,                     NULL, NULL, NULL, NULL, NULL, },
		{ KSymbol_("|"),  0, NULL, Precedence_CStyleBITOR,  0,                     NULL, NULL, NULL, NULL, NULL, },
		{ KSymbol_("^"),  0, NULL, Precedence_CStyleBITXOR, 0,                     NULL, NULL, NULL, NULL, NULL, },
		{ KSymbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);

	KSyntaxVar *syn = (KSyntaxVar *)SUGAR kNameSpace_GetSyntax(kctx, ns, KSymbol_("+"), 0);
	if(syn != NULL) {
		syn->precedence_op1  = Precedence_CStylePREUNARY;
	}

	return true;
}


// --------------------------------------------------------------------------

static kbool_t BitwiseOperator_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* CStyleBitwiseOperator_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "BitwiseOperator", "1.0");
	d.PackupNameSpace = BitwiseOperator_PackupNameSpace;
	d.ExportNameSpace = BitwiseOperator_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
