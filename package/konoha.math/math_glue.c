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

#include <math.h>

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/float.h>
#include "mt19937ar.h"

#define Int_to(T, a)               ((T)a.intValue)
#define Float_to(T, a)             ((T)a.floatValue)

static KMETHOD Math_abs(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(abs(Int_to(int, sfp[1])));
}

static KMETHOD Math_fabs(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(fabs(Float_to(double, sfp[1])));
}

static KMETHOD Math_pow(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(pow(Float_to(double, sfp[1]),Float_to(double, sfp[2])));
}

static KMETHOD Math_ldexp(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(ldexp(Float_to(double, sfp[1]), Int_to(int, sfp[2])));
}

static KMETHOD Math_modf(KonohaContext *kctx, KonohaStack *sfp)
{
	double iprt = Float_to(double, sfp[2]);
	RETURNf_(modf(Float_to(double, sfp[1]), &iprt));
}

static KMETHOD Math_frexp(KonohaContext *kctx, KonohaStack *sfp)
{
	int exp = Int_to(int, sfp[2]);
	RETURNf_(frexp(Float_to(double, sfp[1]), &exp));
}

static KMETHOD Math_fmod(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(fmod(Float_to(double, sfp[1]),Float_to(double, sfp[2])));
}

static KMETHOD Math_ceil(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(ceil(Float_to(double, sfp[1])));
}

#ifdef K_USING_WIN32_
static KMETHOD Math_round(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(round(Float_to(double, sfp[1])));
}

static KMETHOD Math_nearByInt(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(nearbyint(Float_to(double, sfp[1])));
}
#endif

static KMETHOD Math_floor(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(floor(Float_to(double, sfp[1])));
}

static KMETHOD Math_sqrt(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(sqrt(Float_to(double, sfp[1])));
}

static KMETHOD Math_exp(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(exp(Float_to(double, sfp[1])));
}

static KMETHOD Math_log10(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(log10(Float_to(double, sfp[1])));
}

static KMETHOD Math_log(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(log(Float_to(double, sfp[1])));
}

static KMETHOD Math_sin(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(sin(Float_to(double, sfp[1])));
}

static KMETHOD Math_cos(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(cos(Float_to(double, sfp[1])));
}

static KMETHOD Math_tan(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(tan(Float_to(double, sfp[1])));
}

static KMETHOD Math_asin(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(asin(Float_to(double, sfp[1])));
}

static KMETHOD Math_acos(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(acos(Float_to(double, sfp[1])));
}

static KMETHOD Math_atan(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(atan(Float_to(double, sfp[1])));
}

static KMETHOD Math_atan2(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(atan2(Float_to(double, sfp[1]),Float_to(double, sfp[2])));
}

static KMETHOD Math_sinh(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(sinh(Float_to(double, sfp[1])));
}

static KMETHOD Math_cosh(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(cosh(Float_to(double, sfp[1])));
}

static KMETHOD Math_tanh(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(tanh(Float_to(double, sfp[1])));
}

#if defined(K_USING_WIN32_)
static KMETHOD Math_asinh(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(asinh(Float_to(double, sfp[1])));
}

static KMETHOD Math_acosh(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(acosh(Float_to(double, sfp[1])));
}

static KMETHOD Math_atanh(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNf_(atanh(Float_to(double, sfp[1])));
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
	RETURNf_(ret);
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)
#define _KVf(T) "MATH_" #T, TY_Float, M_##T
#define TY_Math  (cMath->typeId)

static	kbool_t math_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KREQUIRE_PACKAGE("konoha.float", pline);
	static KDEFINE_CLASS MathDef = {
			.structname = "Math"/*structname*/,
			.typeId = TY_newid/*cid*/,
	};
	KonohaClass *cMath = KLIB Konoha_defineClass(kctx, ns->packageId, ns->packageDomain, NULL, &MathDef, pline);
	int FN_x = FN_("x");
	int FN_y = FN_("y");
	KDEFINE_METHOD MethodData[] = {
			_Public|_Const, _F(Math_abs), TY_Int, TY_Math, MN_("abs"), 1, TY_Int, FN_x,
			_Public|_Const, _F(Math_fabs), TY_Float, TY_Math, MN_("fabs"), 1, TY_Float, FN_x,
			_Public|_Const, _F(Math_pow), TY_Float, TY_Math, MN_("pow"), 2, TY_Float, FN_x, TY_Float, FN_y,
			_Public|_Const, _F(Math_ldexp), TY_Float, TY_Math, MN_("ldexp"), 2, TY_Float, FN_x, TY_Int, FN_y,
			_Public|_Const, _F(Math_modf), TY_Float, TY_Math, MN_("modf"), 2, TY_Float, FN_x, TY_Float, FN_y,
			_Public|_Const, _F(Math_frexp), TY_Float, TY_Math, MN_("frexp"), 2, TY_Float, FN_x, TY_Int, FN_y,
			_Public|_Const, _F(Math_fmod), TY_Float, TY_Math, MN_("fmod"), 2, TY_Float, FN_x, TY_Float, FN_y,
			_Public|_Const, _F(Math_ceil), TY_Float, TY_Math, MN_("ceil"), 1, TY_Float, FN_x,
#ifdef K_USING_WIN32_
			_Public, _F(Math_round), TY_Float, TY_Math, MN_("round"), 1, TY_Float, FN_x,
			_Public, _F(Math_nearByInt), TY_Float, TY_Math, MN_("nearByInt"), 1, TY_Float, FN_x,
#endif
			_Public|_Const, _F(Math_floor), TY_Float, TY_Math, MN_("floor"), 1, TY_Float, FN_x,
			_Public|_Const, _F(Math_sqrt), TY_Float, TY_Math, MN_("sqrt"), 1, TY_Float, FN_x,
			_Public|_Const, _F(Math_exp), TY_Float, TY_Math, MN_("exp"), 1, TY_Float, FN_x,
			_Public|_Const, _F(Math_log10), TY_Float, TY_Math, MN_("log10"), 1, TY_Float, FN_x,
			_Public|_Const, _F(Math_log), TY_Float, TY_Math, MN_("log"), 1, TY_Float, FN_x,
			_Public|_Const, _F(Math_sin), TY_Float, TY_Math, MN_("sin"), 1, TY_Float, FN_x,
			_Public|_Const, _F(Math_cos), TY_Float, TY_Math, MN_("cos"), 1, TY_Float, FN_x,
			_Public|_Const, _F(Math_tan), TY_Float, TY_Math, MN_("tan"), 1, TY_Float, FN_x,
			_Public|_Const, _F(Math_asin), TY_Float, TY_Math, MN_("asin"), 1, TY_Float, FN_x,
			_Public|_Const, _F(Math_acos), TY_Float, TY_Math, MN_("acos"), 1, TY_Float, FN_x,
			_Public|_Const, _F(Math_atan), TY_Float, TY_Math, MN_("atan"), 1, TY_Float, FN_x,
			_Public|_Const, _F(Math_atan2), TY_Float, TY_Math, MN_("atan2"), 2, TY_Float, FN_x, TY_Float, FN_y,
			_Public|_Const, _F(Math_sinh), TY_Float, TY_Math, MN_("sinh"), 1, TY_Float, FN_x,
			_Public|_Const, _F(Math_cosh), TY_Float, TY_Math, MN_("cosh"), 1, TY_Float, FN_x,
			_Public|_Const, _F(Math_tanh), TY_Float, TY_Math, MN_("tanh"), 1, TY_Float, FN_x,
#if defined(K_USING_WIN32_)
			_Public, _F(Math_asinh), TY_Float, TY_Math, MN_("asinh"), 1, TY_Float, FN_x,
			_Public, _F(Math_acosh), TY_Float, TY_Math, MN_("acosh"), 1, TY_Float, FN_x,
			_Public, _F(Math_atanh), TY_Float, TY_Math, MN_("atanh"), 1, TY_Float, FN_x,
#endif
			_Public, _F(Math_random), TY_Float, TY_Math, MN_("random"), 0,
			DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);

	KDEFINE_FLOAT_CONST FloatData[] = {
			{_KVf(E)},
			{_KVf(LOG2E)},
			{_KVf(LOG10E)},
			{_KVf(LN2)},
			{_KVf(LN10)},
			{_KVf(PI)},
			{_KVf(SQRT2)},
			{}
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(FloatData), 0);
	return true;
}

static kbool_t math_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t math_initNameSpace(KonohaContext *kctx,  kNameSpace *ns, kfileline_t pline)
{
	KEXPORT_PACKAGE("konoha.float", ns, pline);
	return true;
}

static kbool_t math_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}
KDEFINE_PACKAGE* math_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("math", "1.0"),
		.initPackage = math_initPackage,
		.setupPackage = math_setupPackage,
		.initNameSpace = math_initNameSpace,
		.setupNameSpace = math_setupNameSpace,
	};
	return &d;
}
