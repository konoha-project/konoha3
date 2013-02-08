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

/* ************************************************************************ */

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <konoha3/import/methoddecl.h>

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

static kbool_t int_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	int FN_x = KFieldName_("x");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(Int_opPlus), KType_Int, KType_Int, KMethodName_("+"), 0,
		_Public|_Const|_Im, _F(Int_opCompl), KType_Int, KType_Int, KMethodName_("~"), 0,
		_Public|_Const|_Im, _F(Int_opLSHIFT), KType_Int, KType_Int, KMethodName_("<<"), 1, KType_Int, FN_x,
		_Public|_Const|_Im, _F(Int_opRSHIFT), KType_Int, KType_Int, KMethodName_(">>"), 1, KType_Int, FN_x,
		_Public|_Const|_Im, _F(Int_opAND), KType_Int, KType_Int, KMethodName_("&"), 1, KType_Int, FN_x,
		_Public|_Const|_Im, _F(Int_opOR ), KType_Int, KType_Int, KMethodName_("|"), 1, KType_Int, FN_x,
		_Public|_Const|_Im, _F(Int_opXOR), KType_Int, KType_Int, KMethodName_("^"), 1, KType_Int, FN_x,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	KDEFINE_INT_CONST IntData[] = {
		{"INT_MAX", KType_Int, KINT_MAX},
		{"INT_MIN", KType_Int, KINT_MIN},
		{NULL},
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(IntData), trace);
	return true;
}

static kbool_t int_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
//		{ KSymbol_NumberPattern, SYNFLAG_CTypeFunc, 0, 0, {SUGAR termParseFunc}, {SUGARFUNC TypeCheck_ExtendedIntLiteral}},
		{ KSymbol_("~"),  0, 0, Precedence_CStylePrefixOperator, {SUGAR opParseFunc}, {SUGAR methodTypeFunc} },
		{ KSymbol_("<<"), 0, Precedence_CStyleSHIFT,  0, {SUGAR opParseFunc}, {SUGAR methodTypeFunc}},
		{ KSymbol_(">>"), 0, Precedence_CStyleSHIFT,  0, {SUGAR opParseFunc}, {SUGAR methodTypeFunc}},
		{ KSymbol_("&"),  0, Precedence_CStyleBITAND, 0, {SUGAR opParseFunc}, {SUGAR methodTypeFunc}},
		{ KSymbol_("|"),  0, Precedence_CStyleBITOR,  0, {SUGAR opParseFunc}, {SUGAR methodTypeFunc}},
		{ KSymbol_("^"),  0, Precedence_CStyleBITXOR, 0, {SUGAR opParseFunc}, {SUGAR methodTypeFunc}},
		{ KSymbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
//	SUGAR kNameSpace_SetTokenFunc(kctx, ns, KSymbol_NumberPattern, KonohaChar_Digit, KSugarFunc(ns, TokenFunc_ExtendedIntLiteral));
	return true;
}

// --------------------------------------------------------------------------

static kbool_t CStyleBitwiseOperator_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	int_defineMethod(kctx, ns, trace);
	int_defineSyntax(kctx, ns, trace);
	return true;
}

static kbool_t CStyleBitwiseOperator_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	KDEFINE_INT_CONST ClassData[] = {   // long as alias
		{"long", VirtualType_KClass, (uintptr_t)KClass_Int},
		{NULL},
	};
	KLIB kNameSpace_LoadConstData(kctx, exportNS, KConst_(ClassData), trace);
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE *CStyleBitwiseOperator_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "CStyle", K_VERSION);
	d.PackupNameSpace   = CStyleBitwiseOperator_PackupNameSpace;
	d.ExportNameSpace   = CStyleBitwiseOperator_ExportNameSpace;
	return &d;
}

// --------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif
