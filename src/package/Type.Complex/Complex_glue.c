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

#include <stdio.h>
#include <complex.h>

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/konoha_common.h>
#include <konoha3/import/methoddecl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Complex {
	kObjectHeader h;
	double _Complex z;
} kComplex;

static void kComplex_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kComplex *kc = (kComplex *) o;
	kc->z = 0;
}

static void kComplex_Free(KonohaContext *kctx, kObject *o)
{
}

// METHOD =====================================================================

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

static kbool_t Complex_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequirePackage("Type.Float", trace);
	KDEFINE_CLASS defComplex = {0};
	SETSTRUCTNAME(defComplex, Complex);
	defComplex.cflag     = KClassFlag_Final;
	defComplex.init      = kComplex_Init;
	defComplex.free      = kComplex_Free;
	KClass *ComplexClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defComplex, trace);

	int KType_Complex = ComplexClass->typeId;
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Complex_new),     KType_Complex, KType_Complex, KMethodName_("new"),   2, KType_float, KFieldName_("real"), KType_float, KFieldName_("imaginary"),
		_Public, _F(Complex_csin),    KType_float, KType_Complex, KMethodName_("csin"),    0,
		_Public, _F(Complex_csinf),   KType_float, KType_Complex, KMethodName_("csinf"),   0,
		_Public, _F(Complex_csinl),   KType_float, KType_Complex, KMethodName_("csinl"),   0,
		_Public, _F(Complex_ccos),    KType_float, KType_Complex, KMethodName_("ccos"),    0,
		_Public, _F(Complex_ccosf),   KType_float, KType_Complex, KMethodName_("ccosf"),   0,
		_Public, _F(Complex_ccosl),   KType_float, KType_Complex, KMethodName_("ccosl"),   0,
		_Public, _F(Complex_ctan),    KType_float, KType_Complex, KMethodName_("ctan"),    0,
		_Public, _F(Complex_ctanf),   KType_float, KType_Complex, KMethodName_("ctanf"),   0,
		_Public, _F(Complex_ctanl),   KType_float, KType_Complex, KMethodName_("ctanl"),   0,
		_Public, _F(Complex_casin),   KType_float, KType_Complex, KMethodName_("casin"),   0,
		_Public, _F(Complex_casinf),  KType_float, KType_Complex, KMethodName_("casinf"),  0,
		_Public, _F(Complex_casinl),  KType_float, KType_Complex, KMethodName_("casinl"),  0,
		_Public, _F(Complex_cacos),   KType_float, KType_Complex, KMethodName_("cacos"),   0,
		_Public, _F(Complex_cacosf),  KType_float, KType_Complex, KMethodName_("cacosf"),  0,
		_Public, _F(Complex_cacosl),  KType_float, KType_Complex, KMethodName_("cacosl"),  0,
		_Public, _F(Complex_catan),   KType_float, KType_Complex, KMethodName_("catan"),   0,
		_Public, _F(Complex_catanf),  KType_float, KType_Complex, KMethodName_("catanf"),  0,
		_Public, _F(Complex_catanl),  KType_float, KType_Complex, KMethodName_("catanl"),  0,
		_Public, _F(Complex_csinh),   KType_float, KType_Complex, KMethodName_("csinh"),   0,
		_Public, _F(Complex_csinhf),  KType_float, KType_Complex, KMethodName_("csinhf"),  0,
		_Public, _F(Complex_csinhl),  KType_float, KType_Complex, KMethodName_("csinhl"),  0,
		_Public, _F(Complex_ccosh),   KType_float, KType_Complex, KMethodName_("ccosh"),   0,
		_Public, _F(Complex_ccoshf),  KType_float, KType_Complex, KMethodName_("ccoshf"),  0,
		_Public, _F(Complex_ccoshl),  KType_float, KType_Complex, KMethodName_("ccoshl"),  0,
		_Public, _F(Complex_ctanh),   KType_float, KType_Complex, KMethodName_("ctanh"),   0,
		_Public, _F(Complex_ctanhf),  KType_float, KType_Complex, KMethodName_("ctanhf"),  0,
		_Public, _F(Complex_ctanhl),  KType_float, KType_Complex, KMethodName_("ctanhl"),  0,
		_Public, _F(Complex_casinh),  KType_float, KType_Complex, KMethodName_("casinh"),  0,
		_Public, _F(Complex_casinhf), KType_float, KType_Complex, KMethodName_("casinhf"), 0,
		_Public, _F(Complex_casinhl), KType_float, KType_Complex, KMethodName_("casinhl"), 0,
		_Public, _F(Complex_cacosh),  KType_float, KType_Complex, KMethodName_("cacosh"),  0,
		_Public, _F(Complex_cacoshf), KType_float, KType_Complex, KMethodName_("cacoshf"), 0,
		_Public, _F(Complex_cacoshl), KType_float, KType_Complex, KMethodName_("cacoshl"), 0,
		_Public, _F(Complex_catanh),  KType_float, KType_Complex, KMethodName_("catanh"),  0,
		_Public, _F(Complex_catanhf), KType_float, KType_Complex, KMethodName_("catanhf"), 0,
		_Public, _F(Complex_catanhl), KType_float, KType_Complex, KMethodName_("catanhl"), 0,
		_Public, _F(Complex_cexp),    KType_float, KType_Complex, KMethodName_("cexp"),    0,
		_Public, _F(Complex_cexpf),   KType_float, KType_Complex, KMethodName_("cexpf"),   0,
		_Public, _F(Complex_cexpl),   KType_float, KType_Complex, KMethodName_("cexpl"),   0,
		_Public, _F(Complex_clog),    KType_float, KType_Complex, KMethodName_("clog"),    0,
		_Public, _F(Complex_clogf),   KType_float, KType_Complex, KMethodName_("clogf"),   0,
		_Public, _F(Complex_clogl),   KType_float, KType_Complex, KMethodName_("clogl"),   0,
		_Public, _F(Complex_cabs),    KType_float, KType_Complex, KMethodName_("cabs"),    0,
		_Public, _F(Complex_cabsf),   KType_float, KType_Complex, KMethodName_("cabsf"),   0,
		_Public, _F(Complex_cabsl),   KType_float, KType_Complex, KMethodName_("cabsl"),   0,
		_Public, _F(Complex_csqrt),   KType_float, KType_Complex, KMethodName_("csqrt"),   0,
		_Public, _F(Complex_csqrtf),  KType_float, KType_Complex, KMethodName_("csqrtf"),  0,
		_Public, _F(Complex_csqrtl),  KType_float, KType_Complex, KMethodName_("csqrtl"),  0,
		_Public, _F(Complex_cpow),    KType_float, KType_Complex, KMethodName_("cpow"),    2, KType_float,   KFieldName_("real"), KType_float, KFieldName_("imaginary"),
		_Public, _F(Complex_cpowf),   KType_float, KType_Complex, KMethodName_("cpowf"),   2, KType_float,   KFieldName_("real"), KType_float, KFieldName_("imaginary"),
		_Public, _F(Complex_cpowl),   KType_float, KType_Complex, KMethodName_("cpowl"),   2, KType_float,   KFieldName_("real"), KType_float, KFieldName_("imaginary"),
		_Public, _F(Complex_creal),   KType_float, KType_Complex, KMethodName_("creal"),   0,
		_Public, _F(Complex_crealf),  KType_float, KType_Complex, KMethodName_("crealf"),  0,
		_Public, _F(Complex_creall),  KType_float, KType_Complex, KMethodName_("creall"),  0,
		_Public, _F(Complex_cimag),   KType_float, KType_Complex, KMethodName_("cimag"),   0,
		_Public, _F(Complex_cimagf),  KType_float, KType_Complex, KMethodName_("cimagf"),  0,
		_Public, _F(Complex_cimagl),  KType_float, KType_Complex, KMethodName_("cimagl"),  0,
		_Public, _F(Complex_carg),    KType_float, KType_Complex, KMethodName_("carg"),    0,
		_Public, _F(Complex_cargf),   KType_float, KType_Complex, KMethodName_("cargf"),   0,
		_Public, _F(Complex_cargl),   KType_float, KType_Complex, KMethodName_("cargl"),   0,
		_Public, _F(Complex_conj),    KType_Complex, KType_Complex, KMethodName_("conj"),  0,
		_Public, _F(Complex_conjf),   KType_Complex, KType_Complex, KMethodName_("conjf"), 0,
		_Public, _F(Complex_conjl),   KType_Complex, KType_Complex, KMethodName_("conjl"), 0,
		DEND, /* <= sentinel */
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	return true;
}

static kbool_t Complex_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *Complex_Init(void)
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
