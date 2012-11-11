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

#include <stdio.h>
#include <complex.h>

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/konoha_common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Complex {
	KonohaObjectHeader h;
	double _Complex z;
} kComplex;

static void Complex_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kComplex *kc = (kComplex *) o;
	kc->z = 0;
}

static void Complex_free(KonohaContext *kctx, kObject *o)
{
}

//==========================================================<<< KMETHOD >>>=====================================================================

//## Complex Complex.new(z);
static KMETHOD Complex_new(KonohaContext *kctx, KonohaStack *sfp)
{
	/* You do not need to allocate object because
	 * object is allocated by Runtime. */
	kComplex *ret = (kComplex *) sfp[0].asObject;
	double real = (double)sfp[1].floatValue;
	double imaginary = (double)sfp[2].floatValue;
	ret->z = real + I * imaginary;
	KReturn(ret);
}

//## Complex Complex.csin();
static KMETHOD Complex_csin(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = csin(z);
	KReturnFloatValue(ret);
}

//## Complex Complex.csinf();
static KMETHOD Complex_csinf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	float _Complex zf = (float _Complex)kc->z;
	float ret = csinf(zf);
	KReturnFloatValue(ret);
}

//## Complex Complex.csinl();
static KMETHOD Complex_csinl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	long double _Complex zl = (long double _Complex)kc->z;
	long double ret = csinl(zl);
	KReturnFloatValue(ret);
}

//## Complex Complex.ccos();
static KMETHOD Complex_ccos(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = ccos(z);
	KReturnFloatValue(ret);
}

//## Complex Complex.ccosf();
static KMETHOD Complex_ccosf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	float _Complex zf = (float _Complex)kc->z;
	float  ret = ccosf(zf);
	KReturnFloatValue(ret);
}

//## Complex Complex.ccosl();
static KMETHOD Complex_ccosl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	long double _Complex zl = (long double _Complex)kc->z;
	long double ret = ccosl(zl);
	KReturnFloatValue(ret);
}

//## Complex Complex.ctan();
static KMETHOD Complex_ctan(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = ctan(z);
	KReturnFloatValue(ret);
}

//## Complex Complex.ctanf();
static KMETHOD Complex_ctanf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	float _Complex zf = (float _Complex)kc->z;
	float ret = ctanf(zf);
	KReturnFloatValue(ret);
}

//## Complex Complex.ctanl();
static KMETHOD Complex_ctanl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	long double _Complex zl = (long double _Complex)kc->z;
	long double ret = ctanl(zl);
	KReturnFloatValue(ret);
}

//## Complex Complex.casin();
static KMETHOD Complex_casin(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = casin(z);
	KReturnFloatValue(ret);
}

//## Complex Complex.casinf();
static KMETHOD Complex_casinf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	float _Complex zf = (float _Complex)kc->z;
	float ret = casinf(zf);
	KReturnFloatValue(ret);
}

//## Complex Complex.casinl();
static KMETHOD Complex_casinl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	long double _Complex zl = (long double _Complex)kc->z;
	long double ret = casinl(zl);
	KReturnFloatValue(ret);
}

//## Complex Complex.cacos();
static KMETHOD Complex_cacos(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = cacos(z);
	KReturnFloatValue(ret);
}

//## Complex Complex.cacos();
static KMETHOD Complex_cacosf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	float _Complex zf = (float _Complex)kc->z;
	float ret = cacosf(zf);
	KReturnFloatValue(ret);
}

//## Complex Complex.cacosl();
static KMETHOD Complex_cacosl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	long double _Complex zl = (long double _Complex)kc->z;
	long double ret = cacosl(zl);
	KReturnFloatValue(ret);
}

//## Complex Complex.catan();
static KMETHOD Complex_catan(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = catan(z);
	KReturnFloatValue(ret);
}

//## Complex Complex.catanf();
static KMETHOD Complex_catanf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	float _Complex zf = (float _Complex)kc->z;
	float ret = catanf(zf);
	KReturnFloatValue(ret);
}

//## Complex Complex.catanl();
static KMETHOD Complex_catanl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	long double _Complex zl = (long double _Complex)kc->z;
	long double ret = catanl(zl);
	KReturnFloatValue(ret);
}

//## Complex Complex.csinh();
static KMETHOD Complex_csinh(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = csinh(z);
	KReturnFloatValue(ret);
}

//## Complex Complex.csinhf();
static KMETHOD Complex_csinhf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	float _Complex zf = (float _Complex)kc->z;
	float ret = csinh(zf);
	KReturnFloatValue(ret);
}

//## Complex Complex.csinhl();
static KMETHOD Complex_csinhl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	long double _Complex zl = (long double _Complex)kc->z;
	long double ret = csinh(zl);
	KReturnFloatValue(ret);
}

//## Complex Complex.ccosh();
static KMETHOD Complex_ccosh(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = ccosh(z);
	KReturnFloatValue(ret);
}

//## Complex Complex.ccoshf();
static KMETHOD Complex_ccoshf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	float _Complex zf = (float _Complex)kc->z;
	float ret = ccosh(zf);
	KReturnFloatValue(ret);
}

//## Complex Complex.ccoshl();
static KMETHOD Complex_ccoshl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	long double _Complex zl = (long double _Complex)kc->z;
	long double ret = ccosh(zl);
	KReturnFloatValue(ret);
}

//## Complex Complex.ctanh();
static KMETHOD Complex_ctanh(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = ctanh(z);
	KReturnFloatValue(ret);
}

//## Complex Complex.ctanhf();
static KMETHOD Complex_ctanhf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	float _Complex zf = (float _Complex)kc->z;
	float ret = ctanh(zf);
	KReturnFloatValue(ret);
}

//## Complex Complex.ctanhl();
static KMETHOD Complex_ctanhl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	long double _Complex zl = (long double _Complex)kc->z;
	long double ret = ctanh(zl);
	KReturnFloatValue(ret);
}

//## Complex Complex.casinh();
static KMETHOD Complex_casinh(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = casinh(z);
	KReturnFloatValue(ret);
}

//## Complex Complex.casinhf();
static KMETHOD Complex_casinhf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	float _Complex zf = (float _Complex)kc->z;
	float ret = casinh(zf);
	KReturnFloatValue(ret);
}

//## Complex Complex.casinhl();
static KMETHOD Complex_casinhl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	long double _Complex zl = (long double _Complex)kc->z;
	long double ret = casinh(zl);
	KReturnFloatValue(ret);
}

//## Complex Complex.cacosh();
static KMETHOD Complex_cacosh(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = cacosh(z);
	KReturnFloatValue(ret);
}

//## Complex Complex.cacoshf();
static KMETHOD Complex_cacoshf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	float _Complex zf = (float _Complex)kc->z;
	float ret = cacosh(zf);
	KReturnFloatValue(ret);
}

//## Complex Complex.cacoshl();
static KMETHOD Complex_cacoshl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	long double _Complex zl = (long double _Complex)kc->z;
	long double ret = cacosh(zl);
	KReturnFloatValue(ret);
}
//## Complex Complex.catanh();
static KMETHOD Complex_catanh(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = catanh(z);
	KReturnFloatValue(ret);
}

//## Complex Complex.catanhf();
static KMETHOD Complex_catanhf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	float _Complex zf = (float _Complex)kc->z;
	float ret = catanh(zf);
	KReturnFloatValue(ret);
}

//## Complex Complex.catanhl();
static KMETHOD Complex_catanhl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	long double _Complex zl = (long double _Complex)kc->z;
	long double ret = catanh(zl);
	KReturnFloatValue(ret);
}

//## Complex Complex.cexp();
static KMETHOD Complex_cexp(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = cexp(z);
	KReturnFloatValue(ret);
}

//## Complex Complex.cexpf();
static KMETHOD Complex_cexpf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	float _Complex zf = (float _Complex)kc->z;
	float ret = cexp(zf);
	KReturnFloatValue(ret);
}

//## Complex Complex.cexpl();
static KMETHOD Complex_cexpl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	long double _Complex zl = (long double _Complex)kc->z;
	long double ret = cexp(zl);
	KReturnFloatValue(ret);
}

//## Complex Complex.clog();
static KMETHOD Complex_clog(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = clog(z);
	KReturnFloatValue(ret);
}

//## Complex Complex.clogf();
static KMETHOD Complex_clogf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	float _Complex zf = (float _Complex)kc->z;
	float ret = clog(zf);
	KReturnFloatValue(ret);
}

//## Complex Complex.clogl();
static KMETHOD Complex_clogl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	long double _Complex zl = (long double _Complex)kc->z;
	long double ret = clog(zl);
	KReturnFloatValue(ret);
}

//## Complex Complex.cabs();
static KMETHOD Complex_cabs(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = cabs(z);
	KReturnFloatValue(ret);
}

//## Complex Complex.cabsf();
static KMETHOD Complex_cabsf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	float _Complex zf = (float _Complex)kc->z;
	float ret = cabs(zf);
	KReturnFloatValue(ret);
}

//## Complex Complex.cabsl();
static KMETHOD Complex_cabsl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	long double _Complex zl = (long double _Complex)kc->z;
	long double ret = cabs(zl);
	KReturnFloatValue(ret);
}

//## Complex Complex.csqrt();
static KMETHOD Complex_csqrt(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = csqrt(z);
	KReturnFloatValue(ret);
}

//## Complex Complex.csqrtf();
static KMETHOD Complex_csqrtf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	float _Complex zf = (float _Complex)kc->z;
	float ret = csqrt(zf);
	KReturnFloatValue(ret);
}

//## Complex Complex.csqrtl();
static KMETHOD Complex_csqrtl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	long double _Complex zl = (long double _Complex)kc->z;
	long double ret = csqrt(zl);
	KReturnFloatValue(ret);
}

//## Complex Complex.cpow();
static KMETHOD Complex_cpow(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kx = (kComplex *) sfp[0].asObject;
	double _Complex x = kx->z;
	double real = (double)sfp[1].floatValue;
	double imaginary = (double)sfp[2].floatValue;
	double _Complex y = real + I * imaginary;
	double ret = cpow(x, y);
	KReturnFloatValue(ret);
}

//## Complex Complex.cpowf();
static KMETHOD Complex_cpowf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kx = (kComplex *) sfp[0].asObject;
	float _Complex x = (float _Complex)kx->z;
	float real = (float)sfp[1].floatValue;
	float imaginary = (float)sfp[2].floatValue;
	float _Complex y = real + I * imaginary;
	float ret = cpowf(x, y);
	KReturnFloatValue(ret);
}

//## Complex Complex.cpowl();
static KMETHOD Complex_cpowl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kx = (kComplex *) sfp[0].asObject;
	long double _Complex x = (long double _Complex)kx->z;
	long double real = (long double)sfp[1].floatValue;
	long double imaginary = (long double)sfp[2].floatValue;
	long double _Complex y = real + I * imaginary;
	long double ret = cpowl(x, y);
	KReturnFloatValue(ret);
}

//## Complex Complex.creal();
static KMETHOD Complex_creal(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = creal(z);
	KReturnFloatValue(ret);
}

//## Complex Complex.crealf();
static KMETHOD Complex_crealf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	float _Complex zf = (float _Complex)kc->z;
	float ret = creal(zf);
	KReturnFloatValue(ret);
}

//## Complex Complex.creall();
static KMETHOD Complex_creall(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	long double _Complex zl = (long double _Complex)kc->z;
	long double ret = creal(zl);
	KReturnFloatValue(ret);
}

//## Complex Complex.cimag();
static KMETHOD Complex_cimag(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = cimag(z);
	KReturnFloatValue(ret);
}

//## Complex Complex.cimagf();
static KMETHOD Complex_cimagf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	float _Complex zf = (float _Complex)kc->z;
	float ret = cimag(zf);
	KReturnFloatValue(ret);
}

//## Complex Complex.cimagl();
static KMETHOD Complex_cimagl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	long double _Complex zl = (long double _Complex)kc->z;
	long double ret = cimag(zl);
	KReturnFloatValue(ret);
}

//## Complex Complex.carg();
static KMETHOD Complex_carg(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = carg(z);
	KReturnFloatValue(ret);
}

//## Complex Complex.cargf();
static KMETHOD Complex_cargf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	float _Complex zf = (float _Complex)kc->z;
	float ret = carg(zf);
	KReturnFloatValue(ret);
}

//## Complex Complex.cargl();
static KMETHOD Complex_cargl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	long double _Complex zl = (long double _Complex)kc->z;
	long double ret = carg(zl);
	KReturnFloatValue(ret);
}

//## Complex Complex.conj();
static KMETHOD Complex_conj(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = conj(z);
	kComplex *ret = (kComplex *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	ret->z = answer;
	KReturn(ret);
}

//## Complex Complex.conjf();
static KMETHOD Complex_conjf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	float _Complex zf = (float _Complex)kc->z;
	float _Complex answer = conj(zf);
	kComplex *ret = (kComplex *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	ret->z = answer;
	KReturn(ret);
}

//## Complex Complex.conjl();
static KMETHOD Complex_conjl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	long double _Complex zl = (long double _Complex)kc->z;
	long double answer = conj(zl);
	kComplex *ret = (kComplex *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	ret->z = answer;
	KReturn(ret);
}

/* You can attach the following annotations to each methods. */
#define _Public   kMethod_Public
#define _F(F)     (intptr_t)(F)

static kbool_t Complex_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequirePackage("konoha.float", trace);
	KDEFINE_CLASS defComplex = {0};
	SETSTRUCTNAME(defComplex, Complex);
	defComplex.cflag     = kClass_Final;
	defComplex.init      = Complex_init;
	defComplex.free      = Complex_free;
	KonohaClass *ComplexClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defComplex, trace);

	/* You can define methods with the following procedures. */
	int TY_Complex = ComplexClass->typeId;
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Complex_new),     TY_Complex, TY_Complex, MN_("new"),   2, TY_float, FN_("real"), TY_float, FN_("imaginary"),
		_Public, _F(Complex_csin),    TY_float, TY_Complex, MN_("csin"),    0,
		_Public, _F(Complex_csinf),   TY_float, TY_Complex, MN_("csinf"),   0,
		_Public, _F(Complex_csinl),   TY_float, TY_Complex, MN_("csinl"),   0,
		_Public, _F(Complex_ccos),    TY_float, TY_Complex, MN_("ccos"),    0,
		_Public, _F(Complex_ccosf),   TY_float, TY_Complex, MN_("ccosf"),   0,
		_Public, _F(Complex_ccosl),   TY_float, TY_Complex, MN_("ccosl"),   0,
		_Public, _F(Complex_ctan),    TY_float, TY_Complex, MN_("ctan"),    0,
		_Public, _F(Complex_ctanf),   TY_float, TY_Complex, MN_("ctanf"),   0,
		_Public, _F(Complex_ctanl),   TY_float, TY_Complex, MN_("ctanl"),   0,
		_Public, _F(Complex_casin),   TY_float, TY_Complex, MN_("casin"),   0,
		_Public, _F(Complex_casinf),  TY_float, TY_Complex, MN_("casinf"),  0,
		_Public, _F(Complex_casinl),  TY_float, TY_Complex, MN_("casinl"),  0,
		_Public, _F(Complex_cacos),   TY_float, TY_Complex, MN_("cacos"),   0,
		_Public, _F(Complex_cacosf),  TY_float, TY_Complex, MN_("cacosf"),  0,
		_Public, _F(Complex_cacosl),  TY_float, TY_Complex, MN_("cacosl"),  0,
		_Public, _F(Complex_catan),   TY_float, TY_Complex, MN_("catan"),   0,
		_Public, _F(Complex_catanf),  TY_float, TY_Complex, MN_("catanf"),  0,
		_Public, _F(Complex_catanl),  TY_float, TY_Complex, MN_("catanl"),  0,
		_Public, _F(Complex_csinh),   TY_float, TY_Complex, MN_("csinh"),   0,
		_Public, _F(Complex_csinhf),  TY_float, TY_Complex, MN_("csinhf"),  0,
		_Public, _F(Complex_csinhl),  TY_float, TY_Complex, MN_("csinhl"),  0,
		_Public, _F(Complex_ccosh),   TY_float, TY_Complex, MN_("ccosh"),   0,
		_Public, _F(Complex_ccoshf),  TY_float, TY_Complex, MN_("ccoshf"),  0,
		_Public, _F(Complex_ccoshl),  TY_float, TY_Complex, MN_("ccoshl"),  0,
		_Public, _F(Complex_ctanh),   TY_float, TY_Complex, MN_("ctanh"),   0,
		_Public, _F(Complex_ctanhf),  TY_float, TY_Complex, MN_("ctanhf"),  0,
		_Public, _F(Complex_ctanhl),  TY_float, TY_Complex, MN_("ctanhl"),  0,
		_Public, _F(Complex_casinh),  TY_float, TY_Complex, MN_("casinh"),  0,
		_Public, _F(Complex_casinhf), TY_float, TY_Complex, MN_("casinhf"), 0,
		_Public, _F(Complex_casinhl), TY_float, TY_Complex, MN_("casinhl"), 0,
		_Public, _F(Complex_cacosh),  TY_float, TY_Complex, MN_("cacosh"),  0,
		_Public, _F(Complex_cacoshf), TY_float, TY_Complex, MN_("cacoshf"), 0,
		_Public, _F(Complex_cacoshl), TY_float, TY_Complex, MN_("cacoshl"), 0,
		_Public, _F(Complex_catanh),  TY_float, TY_Complex, MN_("catanh"),  0,
		_Public, _F(Complex_catanhf), TY_float, TY_Complex, MN_("catanhf"), 0,
		_Public, _F(Complex_catanhl), TY_float, TY_Complex, MN_("catanhl"), 0,
		_Public, _F(Complex_cexp),    TY_float, TY_Complex, MN_("cexp"),    0,
		_Public, _F(Complex_cexpf),   TY_float, TY_Complex, MN_("cexpf"),   0,
		_Public, _F(Complex_cexpl),   TY_float, TY_Complex, MN_("cexpl"),   0,
		_Public, _F(Complex_clog),    TY_float, TY_Complex, MN_("clog"),    0,
		_Public, _F(Complex_clogf),   TY_float, TY_Complex, MN_("clogf"),   0,
		_Public, _F(Complex_clogl),   TY_float, TY_Complex, MN_("clogl"),   0,
		_Public, _F(Complex_cabs),    TY_float, TY_Complex, MN_("cabs"),    0,
		_Public, _F(Complex_cabsf),   TY_float, TY_Complex, MN_("cabsf"),   0,
		_Public, _F(Complex_cabsl),   TY_float, TY_Complex, MN_("cabsl"),   0,
		_Public, _F(Complex_csqrt),   TY_float, TY_Complex, MN_("csqrt"),   0,
		_Public, _F(Complex_csqrtf),  TY_float, TY_Complex, MN_("csqrtf"),  0,
		_Public, _F(Complex_csqrtl),  TY_float, TY_Complex, MN_("csqrtl"),  0,
		_Public, _F(Complex_cpow),    TY_float, TY_Complex, MN_("cpow"),    2, TY_float,   FN_("real"), TY_float, FN_("imaginary"),
		_Public, _F(Complex_cpowf),   TY_float, TY_Complex, MN_("cpowf"),   2, TY_float,   FN_("real"), TY_float, FN_("imaginary"),
		_Public, _F(Complex_cpowl),   TY_float, TY_Complex, MN_("cpowl"),   2, TY_float,   FN_("real"), TY_float, FN_("imaginary"),
		_Public, _F(Complex_creal),   TY_float, TY_Complex, MN_("creal"),   0,
		_Public, _F(Complex_crealf),  TY_float, TY_Complex, MN_("crealf"),  0,
		_Public, _F(Complex_creall),  TY_float, TY_Complex, MN_("creall"),  0,
		_Public, _F(Complex_cimag),   TY_float, TY_Complex, MN_("cimag"),   0,
		_Public, _F(Complex_cimagf),  TY_float, TY_Complex, MN_("cimagf"),  0,
		_Public, _F(Complex_cimagl),  TY_float, TY_Complex, MN_("cimagl"),  0,
		_Public, _F(Complex_carg),    TY_float, TY_Complex, MN_("carg"),    0,
		_Public, _F(Complex_cargf),   TY_float, TY_Complex, MN_("cargf"),   0,
		_Public, _F(Complex_cargl),   TY_float, TY_Complex, MN_("cargl"),   0,
		_Public, _F(Complex_conj),    TY_Complex, TY_Complex, MN_("conj"),  0,
		_Public, _F(Complex_conjf),   TY_Complex, TY_Complex, MN_("conjf"), 0,
		_Public, _F(Complex_conjl),   TY_Complex, TY_Complex, MN_("conjl"), 0,
		DEND, /* <= sentinel */
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	/* You can define constant variable with the following procedures. */
	KDEFINE_FLOAT_CONST FloatData[] = {
		{"FLOAT_EPSILON", TY_float, DBL_EPSILON},
		{} /* <= sentinel */
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KonohaConst_(FloatData), trace);
	return true;
}

static kbool_t Complex_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* complex_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", "1.0");
	d.PackupNameSpace    = Complex_PackupNameSpace;
	d.ExportNameSpace   = Complex_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
