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
#include <gmp.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _kMpz {
	KonohaObjectHeader h;
	mpz_t mpz; // mpz_t is pointer
} kMpz;

/* ------------------------------------------------------------------------ */
/* [API methods] */
/* Mpz class  */

static void Mpz_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kMpz *mpz = (kMpz *)o;
	mpz_init(mpz->mpz);
}

static void Mpz_free(KonohaContext *kctx, kObject *o)
{
	kMpz *mpz = (kMpz *)o;
	mpz_clear(mpz->mpz);
}

static void Mpz_p(KonohaContext *kctx, KonohaValue *v, int pos, KUtilsWriteBuffer *wb)
{
	/* This function is called when serializing the object. */
	kMpz *p = (kMpz*)v[pos].o;
	char *buf = mpz_get_str(NULL, 10, p->mpz);
	KLIB Kwb_printf(kctx, wb, "%s", buf);
	free(buf);
}

static KMETHOD Mpz_new(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(sfp[0].asObject);
}

static KMETHOD Mpz_new_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *self = (kMpz*)sfp[0].asObject;
	kMpz *src  = (kMpz*)sfp[1].asObject;
	mpz_set(self->mpz, src->mpz);
	RETURN_(self);
}

static KMETHOD Mpz_new_int(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *self = (kMpz*)sfp[0].asObject;
	mpz_set_si(self->mpz, sfp[1].intValue);
	RETURN_(self);
}

static KMETHOD Mpz_new_str(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *self = (kMpz*)sfp[0].asObject;
	const char *src  = S_text(sfp[1].s);
	mpz_set_str(self->mpz, src, 10);
	RETURN_(self);
}

static KMETHOD Mpz_toString(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *self = (kMpz*)sfp[0].asObject;
	char *buf = ALLOCA(char, mpz_sizeinbase(self->mpz, 10));
	mpz_get_str(buf, 10, self->mpz);
	kObject * ret = (kObject*)KLIB new_kString(kctx, buf, strlen(buf), StringPolicy_ASCII);
	RETURN_(ret);
}

static KMETHOD Mpz_toInt(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *self = (kMpz*)sfp[0].asObject;
	RETURNi_(mpz_get_si(self->mpz));
}

static KMETHOD Int_toMpz(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
	mpz_set_si(ret->mpz, sfp[0].intValue);
	RETURN_(ret);
}

static KMETHOD String_toMpz(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
	const char *src  = S_text(sfp[0].s);
	mpz_set_str(ret->mpz, src, 10);
	RETURN_(ret);
}

static KMETHOD Mpz_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *self = (kMpz*)sfp[0].asObject;
	char *buf = ALLOCA(char, mpz_sizeinbase(self->mpz, 10));
	mpz_get_str(buf, 10, self->mpz);
	int size = strlen(buf);
	if(mpz_cmp_si(self->mpz, 0) < 0){
		size -= 1;
	}
	RETURNi_(size);
}

static KMETHOD Mpz_isEven(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *self = (kMpz*)sfp[0].asObject;
	RETURNb_(mpz_even_p(self->mpz));
}

static KMETHOD Mpz_power(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *self = (kMpz*)sfp[0].asObject;
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
	mpz_pow_ui(ret->mpz, self->mpz, sfp[1].intValue);
	RETURN_(ret);
}

static KMETHOD Mpz_opMINUS(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz*)sfp[0].asObject;
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(lhs), 0);
	mpz_neg(ret->mpz, lhs->mpz);
	RETURN_(ret);
}

static KMETHOD Mpz_abs(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz*)sfp[0].asObject;
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(lhs), 0);
	mpz_abs(ret->mpz, lhs->mpz);
	RETURN_(ret);
}

static KMETHOD Mpz_opADD(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz*)sfp[0].asObject;
	kMpz *rhs = (kMpz*)sfp[1].asObject;
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(lhs), 0);
	mpz_add(ret->mpz, lhs->mpz, rhs->mpz);
	RETURN_(ret);
}

static KMETHOD Mpz_opADD_int(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz*)sfp[0].asObject;
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(lhs), 0);
	mpz_add_ui(ret->mpz, lhs->mpz, sfp[1].intValue);
	RETURN_(ret);
}

static KMETHOD Int_opADD_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *rhs = (kMpz*)sfp[1].asObject;
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
	mpz_add_ui(ret->mpz, rhs->mpz, sfp[0].intValue);
	RETURN_(ret);
}

static KMETHOD Mpz_opSUB(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz*)sfp[0].asObject;
	kMpz *rhs = (kMpz*)sfp[1].asObject;
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(lhs), 0);
	mpz_sub(ret->mpz, lhs->mpz, rhs->mpz);
	RETURN_(ret);
}

static KMETHOD Mpz_opSUB_int(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz*)sfp[0].asObject;
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(lhs), 0);
	mpz_sub_ui(ret->mpz, lhs->mpz, sfp[1].intValue);
	RETURN_(ret);
}

static KMETHOD Int_opSUB_mpz(KonohaContext *kctx, KonohaStack *sfp)
{	
	kMpz *rhs = (kMpz*)sfp[1].asObject;
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
	mpz_set_si(ret->mpz, sfp[0].intValue);
	mpz_sub(ret->mpz, ret->mpz, rhs->mpz);
	RETURN_(ret);
}

static KMETHOD Mpz_opMUL(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz*)sfp[0].asObject;
	kMpz *rhs = (kMpz*)sfp[1].asObject;
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(lhs), 0);
	mpz_mul(ret->mpz, lhs->mpz, rhs->mpz);
	RETURN_(ret);
}

static KMETHOD Mpz_opMUL_int(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz*)sfp[0].asObject;
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(lhs), 0);
	mpz_mul_ui(ret->mpz, lhs->mpz, sfp[1].intValue);
	RETURN_(ret);
}

static KMETHOD Int_opMUL_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *rhs = (kMpz*)sfp[1].asObject;
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
	mpz_mul_ui(ret->mpz, rhs->mpz, sfp[0].intValue);
	RETURN_(ret);
}

static KMETHOD Mpz_opMOD(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz*)sfp[0].asObject;
	kMpz *rhs = (kMpz*)sfp[1].asObject;
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(lhs), 0);
	mpz_mod(ret->mpz, lhs->mpz, rhs->mpz);
	RETURN_(ret);
}

static KMETHOD Mpz_opMOD_int(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz*)sfp[0].asObject;
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(lhs), 0);
	mpz_mod_ui(ret->mpz, lhs->mpz, sfp[1].intValue);
	RETURN_(ret);
}

static KMETHOD Int_opMOD_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *rhs = (kMpz*)sfp[1].asObject;
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0);
	mpz_set_si(ret->mpz, sfp[0].intValue);
	mpz_mod(ret->mpz, ret->mpz, rhs->mpz);
	RETURN_(ret);
}

static void THROW_ZeroDividedException(KonohaContext *kctx, KonohaStack *sfp)
{
	KLIB KonohaRuntime_raise(kctx, EXPT_("ZeroDivided"), sfp, sfp[K_RTNIDX].uline, NULL);
}

static KMETHOD Mpz_opDIV(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz*)sfp[0].asObject;
	kMpz *rhs = (kMpz*)sfp[1].asObject;
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(lhs), 0);
	if(unlikely(mpz_sgn(rhs->mpz) == 0)){
		THROW_ZeroDividedException(kctx, sfp);
	}else{
		mpz_tdiv_q(ret->mpz, lhs->mpz, rhs->mpz);
	}
	RETURN_(ret);
}

static KMETHOD Mpz_opDIV_int(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz  *lhs = (kMpz*)sfp[0].asObject;
	kint_t rhs = sfp[1].intValue;
	kMpz  *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(lhs), 0);
	if(unlikely(rhs == 0)){
		THROW_ZeroDividedException(kctx, sfp);
	}else{
		mpz_tdiv_q_ui(ret->mpz, lhs->mpz, rhs);
	}
	RETURN_(ret);
}

static KMETHOD Int_opDIV_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *rhs = (kMpz*)sfp[1].asObject;
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(rhs), 0);
	if(unlikely(mpz_sgn(rhs->mpz) == 0)){
		THROW_ZeroDividedException(kctx, sfp);
	}else{
		mpz_set_si(ret->mpz, sfp[0].intValue);
		mpz_tdiv_q(ret->mpz, ret->mpz, rhs->mpz);
	}
	RETURN_(ret);
}

static KMETHOD Mpz_opAND(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz*)sfp[0].asObject;
	kMpz *rhs = (kMpz*)sfp[1].asObject;
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(lhs), 0);
	mpz_and(ret->mpz, lhs->mpz, rhs->mpz);
	RETURN_(ret);
}

static KMETHOD Mpz_opOR(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz*)sfp[0].asObject;
	kMpz *rhs = (kMpz*)sfp[1].asObject;
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(lhs), 0);
	mpz_ior(ret->mpz, lhs->mpz, rhs->mpz);
	RETURN_(ret);
}

static KMETHOD Mpz_opXOR(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz*)sfp[0].asObject;
	kMpz *rhs = (kMpz*)sfp[1].asObject;
	kMpz *ret = (kMpz*)KLIB new_kObject(kctx, O_ct(lhs), 0);
	mpz_xor(ret->mpz, lhs->mpz, rhs->mpz);
	RETURN_(ret);
}

static int kMpz_cmp(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz*)sfp[0].asObject;
	kMpz *rhs = (kMpz*)sfp[1].asObject;
	return mpz_cmp(lhs->mpz, rhs->mpz);
}

static int kMpz_cmp_int(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz*)sfp[0].asObject;
	return mpz_cmp_si(lhs->mpz, sfp[1].intValue);
}

static int kInt_cmp_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *rhs = (kMpz*)sfp[1].asObject;
	return -mpz_cmp_si(rhs->mpz, sfp[0].intValue);
}

static KMETHOD Mpz_opLT(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(kMpz_cmp(kctx, sfp) < 0);
}

static KMETHOD Mpz_opGT(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(kMpz_cmp(kctx, sfp) > 0);
}

static KMETHOD Mpz_opEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(kMpz_cmp(kctx, sfp) == 0);
}

static KMETHOD Mpz_opLTEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(kMpz_cmp(kctx, sfp) <= 0);
}

static KMETHOD Mpz_opGTEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(kMpz_cmp(kctx, sfp) >= 0);
}

static KMETHOD Mpz_opNEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(kMpz_cmp(kctx, sfp) != 0);
}

static KMETHOD Mpz_opLT_int(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(kMpz_cmp_int(kctx, sfp) < 0);
}

static KMETHOD Mpz_opGT_int(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(kMpz_cmp_int(kctx, sfp) > 0);
}

static KMETHOD Mpz_opEQ_int(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(kMpz_cmp_int(kctx, sfp) == 0);
}

static KMETHOD Mpz_opLTEQ_int(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(kMpz_cmp_int(kctx, sfp) <= 0);
}

static KMETHOD Mpz_opGTEQ_int(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(kMpz_cmp_int(kctx, sfp) >= 0);
}

static KMETHOD Mpz_opNEQ_int(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(kMpz_cmp_int(kctx, sfp) != 0);
}

static KMETHOD Int_opLT_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(kInt_cmp_mpz(kctx, sfp) < 0);
}

static KMETHOD Int_opGT_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(kInt_cmp_mpz(kctx, sfp) > 0);
}

static KMETHOD Int_opEQ_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(kInt_cmp_mpz(kctx, sfp) == 0);
}

static KMETHOD Int_opLTEQ_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(kInt_cmp_mpz(kctx, sfp) <= 0);
}

static KMETHOD Int_opGTEQ_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(kInt_cmp_mpz(kctx, sfp) >= 0);
}

static KMETHOD Int_opNEQ_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(kInt_cmp_mpz(kctx, sfp) != 0);
}


/* ------------------------------------------------------------------------ */

#define _Public   kMethod_Public
#define _Im       kMethod_Immutable
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

#define TY_Mpz     cMpz->typeId


static kbool_t gmp_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char **args, kfileline_t pline)
{
	static KDEFINE_CLASS MpzDef = {0};
	SETSTRUCTNAME(MpzDef, Mpz);
	MpzDef.cflag = kClass_Final;
	MpzDef.init  = Mpz_init;
	MpzDef.free  = Mpz_free;
	MpzDef.p     = Mpz_p;

	KonohaClass *cMpz = KLIB kNameSpace_defineClass(kctx, ns, NULL, &MpzDef, pline);

	int FN_x = FN_("x");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const,     _F(Mpz_new),        TY_Mpz,	    TY_Mpz, MN_("new"), 0,
		_Public|_Const,     _F(Mpz_new_mpz),    TY_Mpz,     TY_Mpz, MN_("new"), 1, TY_Mpz, FN_x,
		_Public|_Const,     _F(Mpz_new_int),    TY_Mpz,     TY_Mpz, MN_("new"), 1, TY_int, FN_x,
		_Public|_Const,     _F(Mpz_new_str),    TY_Mpz,     TY_Mpz, MN_("new"), 1, TY_String, FN_x,
		_Public|_Im|_Const|_Coercion, _F(Mpz_toString),   TY_String,  TY_Mpz, MN_to(TY_String),   0,
		_Public|_Im|_Const|_Coercion, _F(Mpz_toInt),      TY_int,     TY_Mpz, MN_to(TY_int),   0,
		_Public|_Im|_Const|_Coercion, _F(Int_toMpz),      TY_Mpz,     TY_int, MN_to(TY_Mpz),   0,
		_Public|_Im|_Const|_Coercion, _F(String_toMpz),   TY_Mpz,     TY_String, MN_to(TY_Mpz),   0,
		_Public|_Im|_Const, _F(Mpz_getSize),    TY_int,     TY_Mpz, MN_("getsize"), 0,
		_Public|_Im|_Const, _F(Mpz_isEven),     TY_boolean, TY_Mpz, MN_("isEven"), 0,
		_Public|_Im|_Const, _F(Mpz_power),      TY_Mpz,     TY_Mpz, MN_("power"), 1, TY_int, FN_x,
		_Public|_Im|_Const, _F(Mpz_opMINUS),    TY_Mpz,     TY_Mpz, MN_("-"),   0,
		_Public|_Im|_Const, _F(Mpz_abs),        TY_Mpz,     TY_Mpz, MN_("abs"), 0,
		_Public|_Im|_Const, _F(Mpz_opADD),      TY_Mpz,     TY_Mpz, MN_("+"),   1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opADD_int),  TY_Mpz,     TY_Mpz, MN_("+"),   1, TY_int, FN_x,
		_Public|_Im|_Const, _F(Int_opADD_mpz),  TY_Mpz,     TY_int, MN_("+"),   1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opSUB),      TY_Mpz,     TY_Mpz, MN_("-"),   1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opSUB_int),  TY_Mpz,     TY_Mpz, MN_("-"),   1, TY_int, FN_x,
		_Public|_Im|_Const, _F(Int_opSUB_mpz),  TY_Mpz,     TY_int, MN_("-"),   1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opMUL),      TY_Mpz,     TY_Mpz, MN_("*"),   1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opMUL_int),  TY_Mpz,     TY_Mpz, MN_("*"),   1, TY_int, FN_x,
		_Public|_Im|_Const, _F(Int_opMUL_mpz),  TY_Mpz,     TY_int, MN_("*"),   1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opMOD),      TY_Mpz,     TY_Mpz, MN_("%"),   1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opMOD_int),  TY_Mpz,     TY_Mpz, MN_("%"),   1, TY_int, FN_x,
		_Public|_Im|_Const, _F(Int_opMOD_mpz),  TY_Mpz,     TY_int, MN_("%"),   1, TY_Mpz, FN_x,
		_Public|_Im       , _F(Mpz_opDIV),      TY_Mpz,     TY_Mpz, MN_("/"),   1, TY_Mpz, FN_x,
		_Public|_Im       , _F(Mpz_opDIV_int),  TY_Mpz,     TY_Mpz, MN_("/"),   1, TY_int, FN_x,
		_Public|_Im       , _F(Int_opDIV_mpz),  TY_Mpz,     TY_int, MN_("/"),   1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opAND),      TY_Mpz,     TY_Mpz, MN_("&"),   1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opOR),       TY_Mpz,     TY_Mpz, MN_("|"),   1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opXOR),      TY_Mpz,     TY_Mpz, MN_("^"),   1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opLT),       TY_boolean, TY_Mpz, MN_("<"),   1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opGT),       TY_boolean, TY_Mpz, MN_(">"),   1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opEQ),       TY_boolean, TY_Mpz, MN_("=="),  1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opLTEQ),     TY_boolean, TY_Mpz, MN_("<="),  1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opGTEQ),     TY_boolean, TY_Mpz, MN_(">="),  1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opNEQ),      TY_boolean, TY_Mpz, MN_("!="),  1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opLT_int),   TY_boolean, TY_Mpz, MN_("<"),   1, TY_int, FN_x,
		_Public|_Im|_Const, _F(Mpz_opGT_int),   TY_boolean, TY_Mpz, MN_(">"),   1, TY_int, FN_x,
		_Public|_Im|_Const, _F(Mpz_opEQ_int),   TY_boolean, TY_Mpz, MN_("=="),  1, TY_int, FN_x,
		_Public|_Im|_Const, _F(Mpz_opLTEQ_int), TY_boolean, TY_Mpz, MN_("<="),  1, TY_int, FN_x,
		_Public|_Im|_Const, _F(Mpz_opGTEQ_int), TY_boolean, TY_Mpz, MN_(">="),  1, TY_int, FN_x,
		_Public|_Im|_Const, _F(Mpz_opNEQ_int),  TY_boolean, TY_Mpz, MN_("!="),  1, TY_int, FN_x,
		_Public|_Im|_Const, _F(Int_opLT_mpz),   TY_boolean, TY_int, MN_("<"),   1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Int_opGT_mpz),   TY_boolean, TY_int, MN_(">"),   1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Int_opEQ_mpz),   TY_boolean, TY_int, MN_("=="),  1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Int_opLTEQ_mpz), TY_boolean, TY_int, MN_("<="),  1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Int_opGTEQ_mpz), TY_boolean, TY_int, MN_(">="),  1, TY_Mpz, FN_x,
		_Public|_Im|_Const, _F(Int_opNEQ_mpz),  TY_boolean, TY_int, MN_("!="),  1, TY_Mpz, FN_x,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);

	return true;
}

static kbool_t gmp_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t gmp_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t gmp_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

/* ======================================================================== */

KDEFINE_PACKAGE* gmp_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSETPACKNAME(d, "gmp", "1.0");
	d.initPackage    = gmp_initPackage;
	d.setupPackage   = gmp_setupPackage;
	d.initNameSpace  = gmp_initNameSpace;
	d.setupNameSpace = gmp_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
