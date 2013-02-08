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

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif

#include <math.h>

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/konoha_common.h>
#include "mt19937ar.h"
#include <konoha3/import/methoddecl.h>

#ifdef __cplusplus
extern "C"{
#endif

#define Int_to(T, a)               ((T)a.intValue)
#define Float_to(T, a)             ((T)a.floatValue)

static KMETHOD Math_abs(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(abs(Int_to(int, sfp[1])));
}

static KMETHOD Math_fabs(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(fabs(Float_to(double, sfp[1])));
}

static KMETHOD Math_pow(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(pow(Float_to(double, sfp[1]),Float_to(double, sfp[2])));
}

static KMETHOD Math_ldexp(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(ldexp(Float_to(double, sfp[1]), Int_to(int, sfp[2])));
}

static KMETHOD Math_modf(KonohaContext *kctx, KonohaStack *sfp)
{
	double iprt = Float_to(double, sfp[2]);
	KReturnFloatValue(modf(Float_to(double, sfp[1]), &iprt));
}

static KMETHOD Math_frexp(KonohaContext *kctx, KonohaStack *sfp)
{
	int exp = Int_to(int, sfp[2]);
	KReturnFloatValue(frexp(Float_to(double, sfp[1]), &exp));
}

static KMETHOD Math_fmod(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(fmod(Float_to(double, sfp[1]),Float_to(double, sfp[2])));
}

static KMETHOD Math_ceil(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(ceil(Float_to(double, sfp[1])));
}

#ifdef K_USING_WIN32_
static KMETHOD Math_round(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(round(Float_to(double, sfp[1])));
}

static KMETHOD Math_nearByInt(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(nearbyint(Float_to(double, sfp[1])));
}
#endif

static KMETHOD Math_floor(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(floor(Float_to(double, sfp[1])));
}

static KMETHOD Math_sqrt(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(sqrt(Float_to(double, sfp[1])));
}

static KMETHOD Math_exp(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(exp(Float_to(double, sfp[1])));
}

static KMETHOD Math_log10(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(log10(Float_to(double, sfp[1])));
}

static KMETHOD Math_log(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(log(Float_to(double, sfp[1])));
}

static KMETHOD Math_sin(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(sin(Float_to(double, sfp[1])));
}

static KMETHOD Math_cos(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(cos(Float_to(double, sfp[1])));
}

static KMETHOD Math_tan(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(tan(Float_to(double, sfp[1])));
}

static KMETHOD Math_asin(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(asin(Float_to(double, sfp[1])));
}

static KMETHOD Math_acos(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(acos(Float_to(double, sfp[1])));
}

static KMETHOD Math_atan(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(atan(Float_to(double, sfp[1])));
}

static KMETHOD Math_atan2(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(atan2(Float_to(double, sfp[1]),Float_to(double, sfp[2])));
}

static KMETHOD Math_sinh(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(sinh(Float_to(double, sfp[1])));
}

static KMETHOD Math_cosh(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(cosh(Float_to(double, sfp[1])));
}

static KMETHOD Math_tanh(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(tanh(Float_to(double, sfp[1])));
}

#if defined(K_USING_WIN32_)
static KMETHOD Math_asinh(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(asinh(Float_to(double, sfp[1])));
}

static KMETHOD Math_acosh(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(acosh(Float_to(double, sfp[1])));
}

static KMETHOD Math_atanh(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnFloatValue(atanh(Float_to(double, sfp[1])));
}
#endif

static KMETHOD Math_random(KonohaContext *kctx, KonohaStack *sfp)
{
	kfloat_t ret;
#if defined(K_USING_INT32)
	ret = genrand_real1();
#else
	ret =genrand64_real1();
#endif
	KReturnFloatValue(ret);
}

// --------------------------------------------------------------------------

#define MATH_(T) "MATH_" #T, KType_float, M_##T
#define KType_Math  (cMath->typeId)

static kbool_t math_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequirePackage("Type.Float", trace);
	static KDEFINE_CLASS MathDef = {0};
	MathDef.cflag = KClassFlag_Singleton|KClassFlag_Final;
	MathDef.structname = "Math"; /*structname*/
	MathDef.typeId = KTypeAttr_NewId; /*cid*/

	KClass *cMath = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &MathDef, trace);
	int FN_x = KFieldName_("x");
	int FN_y = KFieldName_("y");
	KDEFINE_METHOD MethodData[] = {
			_Public|_Const|_Static, _F(Math_abs), KType_Int, KType_Math, KMethodName_("abs"), 1, KType_Int, FN_x,
			_Public|_Const|_Static, _F(Math_fabs), KType_float, KType_Math, KMethodName_("fabs"), 1, KType_float, FN_x,
			_Public|_Const|_Static, _F(Math_pow), KType_float, KType_Math, KMethodName_("pow"), 2, KType_float, FN_x, KType_float, FN_y,
			_Public|_Const|_Static, _F(Math_ldexp), KType_float, KType_Math, KMethodName_("ldexp"), 2, KType_float, FN_x, KType_Int, FN_y,
			_Public|_Const|_Static, _F(Math_modf), KType_float, KType_Math, KMethodName_("modf"), 2, KType_float, FN_x, KType_float, FN_y,
			_Public|_Const|_Static, _F(Math_frexp), KType_float, KType_Math, KMethodName_("frexp"), 2, KType_float, FN_x, KType_Int, FN_y,
			_Public|_Const|_Static, _F(Math_fmod), KType_float, KType_Math, KMethodName_("fmod"), 2, KType_float, FN_x, KType_float, FN_y,
			_Public|_Const|_Static, _F(Math_ceil), KType_float, KType_Math, KMethodName_("ceil"), 1, KType_float, FN_x,
#ifdef K_USING_WIN32_
			_Public, _F(Math_round), KType_float, KType_Math, KMethodName_("round"), 1, KType_float, FN_x,
			_Public, _F(Math_nearByInt), KType_float, KType_Math, KMethodName_("nearByInt"), 1, KType_float, FN_x,
#endif
			_Public|_Const|_Static, _F(Math_floor), KType_float, KType_Math, KMethodName_("floor"), 1, KType_float, FN_x,
			_Public|_Const|_Static, _F(Math_sqrt), KType_float, KType_Math, KMethodName_("sqrt"), 1, KType_float, FN_x,
			_Public|_Const|_Static, _F(Math_exp), KType_float, KType_Math, KMethodName_("exp"), 1, KType_float, FN_x,
			_Public|_Const|_Static, _F(Math_log10), KType_float, KType_Math, KMethodName_("log10"), 1, KType_float, FN_x,
			_Public|_Const|_Static, _F(Math_log), KType_float, KType_Math, KMethodName_("log"), 1, KType_float, FN_x,
			_Public|_Const|_Static, _F(Math_sin), KType_float, KType_Math, KMethodName_("sin"), 1, KType_float, FN_x,
			_Public|_Const|_Static, _F(Math_cos), KType_float, KType_Math, KMethodName_("cos"), 1, KType_float, FN_x,
			_Public|_Const|_Static, _F(Math_tan), KType_float, KType_Math, KMethodName_("tan"), 1, KType_float, FN_x,
			_Public|_Const|_Static, _F(Math_asin), KType_float, KType_Math, KMethodName_("asin"), 1, KType_float, FN_x,
			_Public|_Const|_Static, _F(Math_acos), KType_float, KType_Math, KMethodName_("acos"), 1, KType_float, FN_x,
			_Public|_Const|_Static, _F(Math_atan), KType_float, KType_Math, KMethodName_("atan"), 1, KType_float, FN_x,
			_Public|_Const|_Static, _F(Math_atan2), KType_float, KType_Math, KMethodName_("atan2"), 2, KType_float, FN_x, KType_float, FN_y,
			_Public|_Const|_Static, _F(Math_sinh), KType_float, KType_Math, KMethodName_("sinh"), 1, KType_float, FN_x,
			_Public|_Const|_Static, _F(Math_cosh), KType_float, KType_Math, KMethodName_("cosh"), 1, KType_float, FN_x,
			_Public|_Const|_Static, _F(Math_tanh), KType_float, KType_Math, KMethodName_("tanh"), 1, KType_float, FN_x,
#if defined(K_USING_WIN32_)
			_Public|_Const|_Static, _F(Math_asinh), KType_float, KType_Math, KMethodName_("asinh"), 1, KType_float, FN_x,
			_Public|_Const|_Static, _F(Math_acosh), KType_float, KType_Math, KMethodName_("acosh"), 1, KType_float, FN_x,
			_Public|_Const|_Static, _F(Math_atanh), KType_float, KType_Math, KMethodName_("atanh"), 1, KType_float, FN_x,
#endif
			_Public, _F(Math_random), KType_float, KType_Math, KMethodName_("random"), 0,
			DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	KDEFINE_FLOAT_CONST FloatData[] = {
			{MATH_(E)},
			{MATH_(LOG2E)},
			{MATH_(LOG10E)},
			{MATH_(LN2)},
			{MATH_(LN10)},
			{MATH_(PI)},
			{MATH_(SQRT2)},
			{}
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(FloatData), trace);
	return true;
}

static kbool_t math_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *Math_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "JavaScript", "1.4");
	d.PackupNameSpace    = math_PackupNameSpace;
	d.ExportNameSpace   = math_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
