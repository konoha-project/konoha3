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

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif

#include <math.h>

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/konoha_common.h>
#include "mt19937ar.h"

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

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Static   kMethod_Static
#define _F(F)   (intptr_t)(F)
#define MATH_(T) "MATH_" #T, TY_float, M_##T
#define TY_Math  (cMath->typeId)

static kbool_t math_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequirePackage("konoha.float", trace);
	static KDEFINE_CLASS MathDef = {0};
	MathDef.cflag = kClass_Singleton|kClass_Final;
	MathDef.structname = "Math"; /*structname*/
	MathDef.typeId = TY_newid; /*cid*/

	KonohaClass *cMath = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &MathDef, trace);
	int FN_x = FN_("x");
	int FN_y = FN_("y");
	KDEFINE_METHOD MethodData[] = {
			_Public|_Const|_Static, _F(Math_abs), TY_int, TY_Math, MN_("abs"), 1, TY_int, FN_x,
			_Public|_Const|_Static, _F(Math_fabs), TY_float, TY_Math, MN_("fabs"), 1, TY_float, FN_x,
			_Public|_Const|_Static, _F(Math_pow), TY_float, TY_Math, MN_("pow"), 2, TY_float, FN_x, TY_float, FN_y,
			_Public|_Const|_Static, _F(Math_ldexp), TY_float, TY_Math, MN_("ldexp"), 2, TY_float, FN_x, TY_int, FN_y,
			_Public|_Const|_Static, _F(Math_modf), TY_float, TY_Math, MN_("modf"), 2, TY_float, FN_x, TY_float, FN_y,
			_Public|_Const|_Static, _F(Math_frexp), TY_float, TY_Math, MN_("frexp"), 2, TY_float, FN_x, TY_int, FN_y,
			_Public|_Const|_Static, _F(Math_fmod), TY_float, TY_Math, MN_("fmod"), 2, TY_float, FN_x, TY_float, FN_y,
			_Public|_Const|_Static, _F(Math_ceil), TY_float, TY_Math, MN_("ceil"), 1, TY_float, FN_x,
#ifdef K_USING_WIN32_
			_Public, _F(Math_round), TY_float, TY_Math, MN_("round"), 1, TY_float, FN_x,
			_Public, _F(Math_nearByInt), TY_float, TY_Math, MN_("nearByInt"), 1, TY_float, FN_x,
#endif
			_Public|_Const|_Static, _F(Math_floor), TY_float, TY_Math, MN_("floor"), 1, TY_float, FN_x,
			_Public|_Const|_Static, _F(Math_sqrt), TY_float, TY_Math, MN_("sqrt"), 1, TY_float, FN_x,
			_Public|_Const|_Static, _F(Math_exp), TY_float, TY_Math, MN_("exp"), 1, TY_float, FN_x,
			_Public|_Const|_Static, _F(Math_log10), TY_float, TY_Math, MN_("log10"), 1, TY_float, FN_x,
			_Public|_Const|_Static, _F(Math_log), TY_float, TY_Math, MN_("log"), 1, TY_float, FN_x,
			_Public|_Const|_Static, _F(Math_sin), TY_float, TY_Math, MN_("sin"), 1, TY_float, FN_x,
			_Public|_Const|_Static, _F(Math_cos), TY_float, TY_Math, MN_("cos"), 1, TY_float, FN_x,
			_Public|_Const|_Static, _F(Math_tan), TY_float, TY_Math, MN_("tan"), 1, TY_float, FN_x,
			_Public|_Const|_Static, _F(Math_asin), TY_float, TY_Math, MN_("asin"), 1, TY_float, FN_x,
			_Public|_Const|_Static, _F(Math_acos), TY_float, TY_Math, MN_("acos"), 1, TY_float, FN_x,
			_Public|_Const|_Static, _F(Math_atan), TY_float, TY_Math, MN_("atan"), 1, TY_float, FN_x,
			_Public|_Const|_Static, _F(Math_atan2), TY_float, TY_Math, MN_("atan2"), 2, TY_float, FN_x, TY_float, FN_y,
			_Public|_Const|_Static, _F(Math_sinh), TY_float, TY_Math, MN_("sinh"), 1, TY_float, FN_x,
			_Public|_Const|_Static, _F(Math_cosh), TY_float, TY_Math, MN_("cosh"), 1, TY_float, FN_x,
			_Public|_Const|_Static, _F(Math_tanh), TY_float, TY_Math, MN_("tanh"), 1, TY_float, FN_x,
#if defined(K_USING_WIN32_)
			_Public|_Const|_Static, _F(Math_asinh), TY_float, TY_Math, MN_("asinh"), 1, TY_float, FN_x,
			_Public|_Const|_Static, _F(Math_acosh), TY_float, TY_Math, MN_("acosh"), 1, TY_float, FN_x,
			_Public|_Const|_Static, _F(Math_atanh), TY_float, TY_Math, MN_("atanh"), 1, TY_float, FN_x,
#endif
			_Public, _F(Math_random), TY_float, TY_Math, MN_("random"), 0,
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
	KLIB kNameSpace_LoadConstData(kctx, ns, KonohaConst_(FloatData), 0);
	return true;
}

static kbool_t math_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *math_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", K_VERSION);
	d.PackupNameSpace    = math_PackupNameSpace;
	d.ExportNameSpace   = math_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
