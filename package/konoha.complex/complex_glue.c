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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <stdio.h>
#include <complex.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Complex {
	KonohaObjectHeader h;
	double _Complex z;
	float _Complex zf;
	long double _Complex zl;
} kComplex;

static void Complex_init(KonohaContext *kctx, kObject *o, void *conf)
{
	/* This function is called when generating the instance of this class.
	 * Moreover, it is called when generating the Null object of this class. */
	kComplex *kc = (kComplex *) o;
	kc->z = 0;
	kc->zf = 0;
	kc->zl = 0;
}

static void Complex_free(KonohaContext *kctx, kObject *o)
{
	/* This function is called at the time of object destruction. 
	 * It is not necessary to destruct the field of the object which GC has managed. */
	/* Do something
	 * struct Person *p = (struct Person *) o;
	 */
}

//## Complex Complex.new(String name, int age);
static KMETHOD Complex_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
//	kString *name = sfp[1].asString;
//	kint_t   age  = sfp[2].intValue;
//	KSETv(p, p->name, name);
//	p->age = age;
	RETURN_(kc);
}


//==========================================================<<< KMETHOD >>>=====================================================================


//## Complex Complex.csin();
static KMETHOD Complex_csin(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = csin(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}

//## Complex Complex.csinf();
static KMETHOD Complex_csinf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = csinf(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}

//## Complex Complex.csinl();
static KMETHOD Complex_csinl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = csinl(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}


//## Complex Complex.ccos();
static KMETHOD Complex_ccos(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = ccos(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}
//## Complex Complex.ccosf();
static KMETHOD Complex_ccosf(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = ccosf(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}
//## Complex Complex.ccosl();
static KMETHOD Complex_ccosl(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = ccosl(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}

//## Complex Complex.ctan();
static KMETHOD Complex_ctan(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = ctan(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}

//## Complex Complex.ctan();
static KMETHOD Complex_ctan(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = ctan(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}

//## Complex Complex.ctan();
static KMETHOD Complex_ctan(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = ctan(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}

//## Complex Complex.casin();
static KMETHOD Complex_casin(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = casin(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}

//## Complex Complex.cacos();
static KMETHOD Complex_cacos(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = cacos(c);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}

//## Complex Complex.catan();
static KMETHOD Complex_catan(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = catan(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}

//## Complex Complex.csinh();
static KMETHOD Complex_csinh(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = csinh(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}

//## Complex Complex.ccosh();
static KMETHOD Complex_ccosh(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = ccosh(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}

//## Complex Complex.ctanh();
static KMETHOD Complex_ctanh(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = ctanh(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}

//## Complex Complex.casinh();
static KMETHOD Complex_casinh(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = casinh(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}

//## Complex Complex.cacosh();
static KMETHOD Complex_cacosh(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = cacosh(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}

//## Complex Complex.cexp();
static KMETHOD Complex_cexp(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = cexp(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}

//## Complex Complex.clog();
static KMETHOD Complex_clog(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = clog(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}

//## Complex Complex.cabs();
static KMETHOD Complex_cabs(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = cabs(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}

//## Complex Complex.csqrt();
static KMETHOD Complex_csqrt(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = csqrt(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}

//## Complex Complex.cpow();
static KMETHOD Complex_cpow(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double _Complex answer = cpow(z);
	kComplex *ret = KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX]), 0);
	ret->z = answer;
	RETURN_(ret);
}

//## Complex Complex.creal();
static KMETHOD Complex_creal(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = creal(z);
	RETURNf_(ret);
}

//## Complex Complex.creal();
static KMETHOD Complex_cimag(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = cimag(z);
	RETURNf_(ret);
}

//## Complex Complex.carg();
static KMETHOD Complex_carg(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = carg(z);
	RETURNf_(ret);
}

//## Complex Complex.carg();
static KMETHOD Complex_conj(KonohaContext *kctx, KonohaStack *sfp)
{
	kComplex *kc = (kComplex *) sfp[0].asObject;
	double _Complex z = kc->z;
	double ret = conj(z);
	RETURNf_(ret);
}

/* You can attach the following annotations to each methods. */
#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Im       kMethod_Immutable
#define _Coercion kMethod_Coercion
#define _Static   kMethod_Static

#define _F(F)     (intptr_t)(F)

static kbool_t Complex_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	/* Class Definition */
	/* If you want to create Generic class like Array<T>, see konoha.map package */
	KDEFINE_CLASS defComplex = {0};
	SETSTRUCTNAME(defComplex, Complex);
	defComplex.cflag     = kClass_Final;
	defComplex.init      = Complex_init;
	defComplex.p         = Complex_p;
	defComplex.reftrace  = Complex_reftrace;
	defComplex.free      = Complex_free;
	KonohaClass *ComplexClass = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defComplex, pline);

	/* You can define methods with the following procedures. */
	int TY_Complex = ComplexClass->typeId;
	int FN_x = FN_("x");
	int FN_y = FN_("y");
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Complex_new), TY_Complex, TY_Complex, MN_("new"), 2, TY_String, FN_x, TY_int, FN_y,
		_Public, _F(Complex_say), TY_String, TY_Complex, MN_("say"), 0,
		DEND, /* <= sentinel */
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);

	/* You can define constant variable with the following procedures. */
	KDEFINE_INT_CONST IntData[] = {
		{"NARUTO_AGE", TY_int, 18},
		{} /* <= sentinel */
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(IntData), pline);
	return true;
}

static kbool_t HelloWorld_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t HelloWorld_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t HelloWorld_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* hello_world_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSETPACKNAME(d, "hello_world", "1.0");
	d.initPackage    = HelloWorld_initPackage;
	d.setupPackage   = HelloWorld_setupPackage;
	d.initNameSpace  = HelloWorld_initNameSpace;
	d.setupNameSpace = HelloWorld_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
