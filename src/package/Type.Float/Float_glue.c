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

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/konoha_common.h>
#include <konoha3/import/methoddecl.h>

#ifdef _MSC_VER
#define INFINITY (DBL_MAX+DBL_MAX)
#define NAN (INFINITY-INFINITY)
#else
#include <math.h> /* for INFINATE, NAN */

#ifndef INFINITY
#ifdef __GNUC__
#define INFINITY (__builtin_inff())
#elif defined(HUGE_VAL)
#endif
#endif /* !defined(INFINITY) */

#ifndef NAN
#ifdef __GNUC__
#define NAN (__builtin_nanf(""))
#else
#define NAN (0.0/0)
#endif
#endif
#endif /* !defined(NAN) */

#ifdef __cplusplus
extern "C" {
#endif

// --------------------------------------------------------------------------

static void THROW_ZeroDividedException(KonohaContext *kctx, KonohaStack *sfp)
{
	KLIB KRuntime_raise(kctx, KException_("ZeroDivided"), SoftwareFault, NULL, sfp);
}

// --------------------------------------------------------------------------

static void kFloat_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kNumberVar *n = (kNumberVar *)o;  // kFloat has the same structure
	n->unboxValue = (uintptr_t)conf;  // conf is unboxed data
}

static void kFloat_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	KLIB KBuffer_printf(kctx, wb, KFLOAT_FMT, v[pos].floatValue);
}


// --------------------------------------------------------------------------

static KMETHOD Float_opPlus(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue((sfp[0].floatValue));
}

/* float + float */
static KMETHOD Float_opADD(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(sfp[0].floatValue + sfp[1].floatValue);
}

static KMETHOD Int_opADD(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(sfp[0].intValue + sfp[1].floatValue);
}

/* float - float */
static KMETHOD Float_opSUB(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(sfp[0].floatValue - sfp[1].floatValue);
}

static KMETHOD Int_opSUB(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(sfp[0].intValue - sfp[1].floatValue);
}

/* float * float */
static KMETHOD Float_opMUL(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(sfp[0].floatValue * sfp[1].floatValue);
}

/* float / float */
static KMETHOD Float_opDIV(KonohaContext *kctx, KonohaStack *sfp)
{
	kfloat_t n = sfp[1].floatValue;
	if(unlikely(n == 0.0)) {
		THROW_ZeroDividedException(kctx, sfp);
	}
	KReturnFloatValue(sfp[0].floatValue / n);
}

static KMETHOD Int_opMUL(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(sfp[0].intValue * sfp[1].floatValue);
}

/* float / float */
static KMETHOD Int_opDIV(KonohaContext *kctx, KonohaStack *sfp)
{
	kfloat_t n = sfp[1].floatValue;
	if(unlikely(n == 0.0)) {
		THROW_ZeroDividedException(kctx, sfp);
	}
	KReturnFloatValue(sfp[0].intValue / n);
}

/* float == float */
static KMETHOD Float_opEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].floatValue == sfp[1].floatValue);
}

/* float != float */
static KMETHOD Float_opNEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].floatValue != sfp[1].floatValue);
}

/* float < float */
static KMETHOD Float_opLT(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].floatValue < sfp[1].floatValue);
}

/* float <= float */
static KMETHOD Float_opLTE(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].floatValue <= sfp[1].floatValue);
}

/* float > float */
static KMETHOD Float_opGT(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].floatValue > sfp[1].floatValue);
}

/* float >= float */
static KMETHOD Float_opGTE(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].floatValue >= sfp[1].floatValue);
}

//////

static KMETHOD Int_opEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue == sfp[1].floatValue);
}

/* float != float */
static KMETHOD Int_opNEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue != sfp[1].floatValue);
}


/* float < float */
static KMETHOD Int_opLT(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue < sfp[1].floatValue);
}

/* float <= float */
static KMETHOD Int_opLTE(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue <= sfp[1].floatValue);
}

/* float > float */
static KMETHOD Int_opGT(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue > sfp[1].floatValue);
}

/* float >= float */
static KMETHOD Int_opGTE(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].intValue >= sfp[1].floatValue);
}


/* float to int */
static KMETHOD Float_toInt(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue((kint_t)sfp[0].floatValue);
}

/* float >= float */
static KMETHOD Int_toFloat(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue((kfloat_t)sfp[0].intValue);
}

/* float to String */
static KMETHOD Float_toString(KonohaContext *kctx, KonohaStack *sfp)
{
	char buf[40];
	PLATAPI snprintf_i(buf, sizeof(buf), KFLOAT_FMT, sfp[0].floatValue);
	KReturn(KLIB new_kString(kctx, OnStack, buf, strlen(buf), StringPolicy_ASCII));
}

/* String to float */
static KMETHOD String_toFloat(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue((kfloat_t)strtod(kString_text(sfp[0].asString), NULL));
}

//## @Const method Int Int.opMINUS();
static KMETHOD Float_opMINUS(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(-(sfp[0].floatValue));
}

/* ------------------------------------------------------------------------ */

static kbool_t float_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KRequireKonohaCommonModule(trace);
	if(KClass_Float == NULL) {
		KDEFINE_CLASS defFloat = {0};
		SETUNBOXNAME(defFloat, float);
		defFloat.cstruct_size = sizeof(kFloat);
		defFloat.cflag = KClassFlag_int;
		defFloat.init = kFloat_Init;
		defFloat.format     = kFloat_format;
		KClass_Float = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defFloat, trace);
	}
	int FN_x = KFieldName_("x");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(Float_opPlus), KType_float, KType_float, KMethodName_("+"), 0,
		_Public|_Const|_Im, _F(Float_opADD), KType_float, KType_float, KMethodName_("+"), 1, KType_float, FN_x,
		_Public|_Const|_Im, _F(Int_opADD), KType_float, KType_Int, KMethodName_("+"), 1, KType_float, FN_x,
		_Public|_Const|_Im, _F(Float_opSUB), KType_float, KType_float, KMethodName_("-"), 1, KType_float, FN_x,
		_Public|_Const|_Im, _F(Int_opSUB), KType_float, KType_Int, KMethodName_("-"), 1, KType_float, FN_x,
		_Public|_Const|_Im, _F(Float_opMUL), KType_float, KType_float, KMethodName_("*"), 1, KType_float, FN_x,
		_Public|_Im, _F(Float_opDIV), KType_float, KType_float, KMethodName_("/"), 1, KType_float, FN_x,
		_Public|_Const|_Im, _F(Float_opEQ),  KType_Boolean, KType_float, KMethodName_("=="), 1, KType_float, FN_x,
		_Public|_Const|_Im, _F(Float_opNEQ), KType_Boolean, KType_float, KMethodName_("!="), 1, KType_float, FN_x,
		_Public|_Const|_Im, _F(Float_opLT),  KType_Boolean, KType_float, KMethodName_("<"), 1, KType_float, FN_x,
		_Public|_Const|_Im, _F(Float_opLTE), KType_Boolean, KType_float, KMethodName_("<="), 1, KType_float, FN_x,
		_Public|_Const|_Im, _F(Float_opGT),  KType_Boolean, KType_float, KMethodName_(">"), 1, KType_float, FN_x,
		_Public|_Const|_Im, _F(Float_opGTE), KType_Boolean, KType_float, KMethodName_(">="), 1, KType_float, FN_x,
		_Public|_Const|_Im, _F(Int_opMUL), KType_float, KType_Int, KMethodName_("*"), 1, KType_float, FN_x,
		_Public|_Im, _F(Int_opDIV), KType_float, KType_Int, KMethodName_("/"), 1, KType_float, FN_x,
		_Public|_Const|_Im, _F(Int_opEQ),  KType_Boolean, KType_Int, KMethodName_("=="), 1, KType_float, FN_x,
		_Public|_Const|_Im, _F(Int_opNEQ), KType_Boolean, KType_Int, KMethodName_("!="), 1, KType_float, FN_x,
		_Public|_Const|_Im, _F(Int_opLT),  KType_Boolean, KType_Int, KMethodName_("<"), 1, KType_float, FN_x,
		_Public|_Const|_Im, _F(Int_opLTE), KType_Boolean, KType_Int, KMethodName_("<="), 1, KType_float, FN_x,
		_Public|_Const|_Im, _F(Int_opGT),  KType_Boolean, KType_Int, KMethodName_(">"), 1, KType_float, FN_x,
		_Public|_Const|_Im, _F(Int_opGTE), KType_Boolean, KType_Int, KMethodName_(">="), 1, KType_float, FN_x,

		_Public|_Const|_Im|_Coercion, _F(Float_toInt), KType_Int, KType_float, KMethodName_To(KType_Int), 0,
		_Public|_Const|_Im|_Coercion, _F(Int_toFloat), KType_float, KType_Int, KMethodName_To(KType_float), 0,
		_Public|_Const|_Im, _F(Float_toString), KType_String, KType_float, KMethodName_To(KType_String), 0,
		_Public|_Const|_Im, _F(String_toFloat), KType_float, KType_String, KMethodName_To(KType_float), 0,
		_Public|_Const|_Im, _F(Float_opMINUS), KType_float, KType_float, KMethodName_("-"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	KDEFINE_FLOAT_CONST FloatData[] = {
		{"FLOAT_EPSILON", KType_float, DBL_EPSILON},
		{"Infinity", KType_float, INFINITY},
		{"NaN", KType_float, NAN},
		{NULL} /* sentinel */
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(FloatData), trace);
	return true;
}

//----------------------------------------------------------------------------

static KMETHOD TypeCheck_Float(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck2(stmt, expr, ns, reqc);
	kToken *tk = expr->TermToken;
	sfp[4].floatValue = strtod(kString_text(tk->text), NULL);   // just using tramsformation float
	KReturn(SUGAR kNode_SetUnboxConst(kctx, expr, KType_float, sfp[4].unboxValue));
}

static kbool_t float_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	/* Use Syntax.Number package's Parser to parsing FloatLiteral */
	KImportPackage(ns, "Syntax.Number", trace);
	kSyntax *numberSyntax = kSyntax_(ns, KSymbol_NumberPattern);
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("$Float"), SYNFLAG_CTypeFunc, 0, 0, {numberSyntax->ParseFuncNULL}, {SUGARFUNC TypeCheck_Float}, KonohaChar_Digit, {numberSyntax->TokenFuncNULL}},
		{ KSymbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}

//---

static kbool_t float_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	float_defineMethod(kctx, ns, trace);
	float_defineSyntax(kctx, ns, trace);
	return true;
}

static kbool_t float_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	if(sizeof(kfloat_t) == sizeof(double)) {
		KDEFINE_INT_CONST ClassData[] = {   // add Array as available
			{"double", VirtualType_KClass, (uintptr_t)KClass_Float},
			{NULL},
		};
		KLIB kNameSpace_LoadConstData(kctx, exportNS, KConst_(ClassData), trace);
	}
	return true;
}


KDEFINE_PACKAGE *Float_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", K_VERSION);
	d.PackupNameSpace    = float_PackupNameSpace;
	d.ExportNameSpace   = float_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
