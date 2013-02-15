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
#include <gmp.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _kMpz {
	kObjectHeader h;
	mpz_t mpz; // mpz_t is pointer; you don't need put '*'.
} kMpz;

typedef struct _kMpf {
	kObjectHeader h;
	mpf_t mpf; // mpf_t is pointer
} kMpf;

static void THROW_ZeroDividedException(KonohaContext *kctx, KonohaStack *sfp)
{
	KLIB KRuntime_raise(kctx, KException_("ZeroDivided"), SoftwareFault, NULL, sfp);
}

/* ------------------------------------------------------------------------ */
/* [API methods] */
/* Mpz class  */

static void Mpz_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kMpz *mpz = (kMpz *)o;
	mpz_init(mpz->mpz);
}

static void Mpz_Free(KonohaContext *kctx, kObject *o)
{
	kMpz *mpz = (kMpz *)o;
	mpz_clear(mpz->mpz);
}

static void Mpz_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	/* This function is called when serializing the object. */
	kMpz *p = (kMpz *)v[pos].asObject;
	char *buf = mpz_get_str(NULL, 10, p->mpz);
	KLIB KBuffer_printf(kctx, wb, "%s", buf);
	free(buf);
}

static KMETHOD Mpz_new(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(sfp[0].asObject);
}

static KMETHOD Mpz_new_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *self = (kMpz *)sfp[0].asObject;
	kMpz *src  = (kMpz *)sfp[1].asObject;
	mpz_set(self->mpz, src->mpz);
	KReturn(self);
}

static KMETHOD Mpz_new_int(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *self = (kMpz *)sfp[0].asObject;
	mpz_set_si(self->mpz, sfp[1].intValue);
	KReturn(self);
}

static KMETHOD Mpz_new_str(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *self = (kMpz *)sfp[0].asObject;
	const char *src  = kString_text(sfp[1].asString);
	mpz_set_str(self->mpz, src, 10);
	KReturn(self);
}

static KMETHOD Mpz_toString(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *self = (kMpz *)sfp[0].asObject;
	char *buf = ALLOCA(char, mpz_sizeinbase(self->mpz, 10));
	mpz_get_str(buf, 10, self->mpz);
	KReturn(KLIB new_kString(kctx, OnStack, buf, strlen(buf), StringPolicy_ASCII));
}

static KMETHOD Mpz_toInt(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *self = (kMpz *)sfp[0].asObject;
	KReturnUnboxValue(mpz_get_si(self->mpz));
}

static KMETHOD Int_toMpz(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	mpz_set_si(ret->mpz, sfp[0].intValue);
	KReturn(ret);
}

static KMETHOD String_toMpz(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	const char *src  = kString_text(sfp[0].asString);
	mpz_set_str(ret->mpz, src, 10);
	KReturn(ret);
}

static KMETHOD Mpz_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *self = (kMpz *)sfp[0].asObject;
	char *buf = ALLOCA(char, mpz_sizeinbase(self->mpz, 10));
	mpz_get_str(buf, 10, self->mpz);
	int size = strlen(buf);
	if(mpz_cmp_si(self->mpz, 0) < 0){
		size -= 1;
	}
	KReturnUnboxValue(size);
}

static KMETHOD Mpz_isEven(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *self = (kMpz *)sfp[0].asObject;
	KReturnUnboxValue(mpz_even_p(self->mpz));
}

static KMETHOD Mpz_power(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *self = (kMpz *)sfp[0].asObject;
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	mpz_pow_ui(ret->mpz, self->mpz, sfp[1].intValue);
	KReturn(ret);
}

static KMETHOD Mpz_opMINUS(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz *)sfp[0].asObject;
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	mpz_neg(ret->mpz, lhs->mpz);
	KReturn(ret);
}

static KMETHOD Mpz_abs(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz *)sfp[0].asObject;
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	mpz_abs(ret->mpz, lhs->mpz);
	KReturn(ret);
}

static KMETHOD Mpz_opADD(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz *)sfp[0].asObject;
	kMpz *rhs = (kMpz *)sfp[1].asObject;
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	mpz_add(ret->mpz, lhs->mpz, rhs->mpz);
	KReturn(ret);
}

static KMETHOD Mpz_opADD_int(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz *)sfp[0].asObject;
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	mpz_set_si(ret->mpz, sfp[1].intValue);
	mpz_add(ret->mpz, ret->mpz, lhs->mpz);
	KReturn(ret);
}

static KMETHOD Int_opADD_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *rhs = (kMpz *)sfp[1].asObject;
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	mpz_set_si(ret->mpz, sfp[0].intValue);
	mpz_add(ret->mpz, ret->mpz, rhs->mpz);
	KReturn(ret);
}

static KMETHOD Mpz_opSUB(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz *)sfp[0].asObject;
	kMpz *rhs = (kMpz *)sfp[1].asObject;
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	mpz_sub(ret->mpz, lhs->mpz, rhs->mpz);
	KReturn(ret);
}

static KMETHOD Mpz_opSUB_int(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz *)sfp[0].asObject;
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	mpz_set_si(ret->mpz, sfp[1].intValue);
	mpz_sub(ret->mpz, lhs->mpz, ret->mpz);
	KReturn(ret);
}

static KMETHOD Int_opSUB_mpz(KonohaContext *kctx, KonohaStack *sfp)
{	
	kMpz *rhs = (kMpz *)sfp[1].asObject;
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	mpz_set_si(ret->mpz, sfp[0].intValue);
	mpz_sub(ret->mpz, ret->mpz, rhs->mpz);
	KReturn(ret);
}

static KMETHOD Mpz_opMUL(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz *)sfp[0].asObject;
	kMpz *rhs = (kMpz *)sfp[1].asObject;
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	mpz_mul(ret->mpz, lhs->mpz, rhs->mpz);
	KReturn(ret);
}

static KMETHOD Mpz_opMUL_int(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz *)sfp[0].asObject;
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	mpz_mul_si(ret->mpz, lhs->mpz, sfp[1].intValue);
	KReturn(ret);
}

static KMETHOD Int_opMUL_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *rhs = (kMpz *)sfp[1].asObject;
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	mpz_mul_si(ret->mpz, rhs->mpz, sfp[0].intValue);
	KReturn(ret);
}

static KMETHOD Mpz_opMOD(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz *)sfp[0].asObject;
	kMpz *rhs = (kMpz *)sfp[1].asObject;
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	mpz_mod(ret->mpz, lhs->mpz, rhs->mpz);
	KReturn(ret);
}

static KMETHOD Mpz_opMOD_int(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz *)sfp[0].asObject;
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	mpz_set_si(ret->mpz, sfp[1].intValue);
	mpz_mod(ret->mpz, lhs->mpz, ret->mpz);
	KReturn(ret);
}

static KMETHOD Int_opMOD_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *rhs = (kMpz *)sfp[1].asObject;
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	mpz_set_si(ret->mpz, sfp[0].intValue);
	mpz_mod(ret->mpz, ret->mpz, rhs->mpz);
	KReturn(ret);
}

static KMETHOD Mpz_opDIV(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz *)sfp[0].asObject;
	kMpz *rhs = (kMpz *)sfp[1].asObject;
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	if(unlikely(mpz_sgn(rhs->mpz) == 0)){
		THROW_ZeroDividedException(kctx, sfp);
	}else{
		mpz_tdiv_q(ret->mpz, lhs->mpz, rhs->mpz);
	}
	KReturn(ret);
}

static KMETHOD Mpz_opDIV_int(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz  *lhs = (kMpz *)sfp[0].asObject;
	kint_t rhs = sfp[1].intValue;
	kMpz  *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	if(unlikely(rhs == 0)){
		THROW_ZeroDividedException(kctx, sfp);
	}else{
		mpz_set_si(ret->mpz, rhs);
		mpz_tdiv_q(ret->mpz, lhs->mpz, ret->mpz);
	}
	KReturn(ret);
}

static KMETHOD Int_opDIV_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *rhs = (kMpz *)sfp[1].asObject;
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, kObject_class(rhs), 0);
	if(unlikely(mpz_sgn(rhs->mpz) == 0)){
		THROW_ZeroDividedException(kctx, sfp);
	}else{
		mpz_set_si(ret->mpz, sfp[0].intValue);
		mpz_tdiv_q(ret->mpz, ret->mpz, rhs->mpz);
	}
	KReturn(ret);
}

static KMETHOD Mpz_opAND(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz *)sfp[0].asObject;
	kMpz *rhs = (kMpz *)sfp[1].asObject;
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	mpz_and(ret->mpz, lhs->mpz, rhs->mpz);
	KReturn(ret);
}

static KMETHOD Mpz_opOR(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz *)sfp[0].asObject;
	kMpz *rhs = (kMpz *)sfp[1].asObject;
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	mpz_ior(ret->mpz, lhs->mpz, rhs->mpz);
	KReturn(ret);
}

static KMETHOD Mpz_opXOR(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz *)sfp[0].asObject;
	kMpz *rhs = (kMpz *)sfp[1].asObject;
	kMpz *ret = (kMpz *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	mpz_xor(ret->mpz, lhs->mpz, rhs->mpz);
	KReturn(ret);
}

static int kMpz_cmp(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz *)sfp[0].asObject;
	kMpz *rhs = (kMpz *)sfp[1].asObject;
	return mpz_cmp(lhs->mpz, rhs->mpz);
}

static int kMpz_cmp_int(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *lhs = (kMpz *)sfp[0].asObject;
	return mpz_cmp_si(lhs->mpz, sfp[1].intValue);
}

static int kInt_cmp_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *rhs = (kMpz *)sfp[1].asObject;
	return -mpz_cmp_si(rhs->mpz, sfp[0].intValue);
}

static KMETHOD Mpz_opLT(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpz_cmp(kctx, sfp) < 0);
}

static KMETHOD Mpz_opGT(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpz_cmp(kctx, sfp) > 0);
}

static KMETHOD Mpz_opEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpz_cmp(kctx, sfp) == 0);
}

static KMETHOD Mpz_opLTEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpz_cmp(kctx, sfp) <= 0);
}

static KMETHOD Mpz_opGTEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpz_cmp(kctx, sfp) >= 0);
}

static KMETHOD Mpz_opNEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpz_cmp(kctx, sfp) != 0);
}

static KMETHOD Mpz_opLT_int(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpz_cmp_int(kctx, sfp) < 0);
}

static KMETHOD Mpz_opGT_int(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpz_cmp_int(kctx, sfp) > 0);
}

static KMETHOD Mpz_opEQ_int(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpz_cmp_int(kctx, sfp) == 0);
}

static KMETHOD Mpz_opLTEQ_int(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpz_cmp_int(kctx, sfp) <= 0);
}

static KMETHOD Mpz_opGTEQ_int(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpz_cmp_int(kctx, sfp) >= 0);
}

static KMETHOD Mpz_opNEQ_int(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpz_cmp_int(kctx, sfp) != 0);
}

static KMETHOD Int_opLT_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kInt_cmp_mpz(kctx, sfp) < 0);
}

static KMETHOD Int_opGT_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kInt_cmp_mpz(kctx, sfp) > 0);
}

static KMETHOD Int_opEQ_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kInt_cmp_mpz(kctx, sfp) == 0);
}

static KMETHOD Int_opLTEQ_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kInt_cmp_mpz(kctx, sfp) <= 0);
}

static KMETHOD Int_opGTEQ_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kInt_cmp_mpz(kctx, sfp) >= 0);
}

static KMETHOD Int_opNEQ_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kInt_cmp_mpz(kctx, sfp) != 0);
}

/* ------------------------------------------------------------------------ */
/* [API methods] */
/* Mpf class  */

static void Mpf_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kMpf *mpf = (kMpf *)o;
	mpf_init(mpf->mpf);
}

static void Mpf_Free(KonohaContext *kctx, kObject *o)
{
	kMpf *mpf = (kMpf *)o;
	mpf_clear(mpf->mpf);
}

static void Mpf_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	/* This function is called when serializing the object. */
	kMpf *p = (kMpf *)v[pos].asObject;
	char buf[1024];
	gmp_snprintf(buf, 1024, "%.Fg", p->mpf);
	KLIB KBuffer_printf(kctx, wb, "%s", buf);
}

static KMETHOD Mpf_new(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(sfp[0].asObject);
}

static KMETHOD Mpf_new_mpf(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *self = (kMpf *)sfp[0].asObject;
	kMpf *src  = (kMpf *)sfp[1].asObject;
	mpf_set(self->mpf, src->mpf);
	KReturn(self);
}

static KMETHOD Mpf_new_int(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *self = (kMpf *)sfp[0].asObject;
	mpf_set_si(self->mpf, sfp[1].intValue);
	KReturn(self);
}

static KMETHOD Mpf_new_mpz(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *self = (kMpf *)sfp[0].asObject;
	kMpz *src  = (kMpz *)sfp[1].asObject;
	mpf_set_z(self->mpf, src->mpz);
	KReturn(self);
}

static KMETHOD Mpf_new_float(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *self = (kMpf *)sfp[0].asObject;
	mpf_set_d(self->mpf, sfp[1].floatValue);
	KReturn(self);
}

static KMETHOD Mpf_new_str(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *self = (kMpf *)sfp[0].asObject;
	const char *src  = kString_text(sfp[1].asString);
	mpf_set_str(self->mpf, src, 10);
	KReturn(self);
}

static KMETHOD Mpf_getprec(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *self = (kMpf *)sfp[0].asObject;
	KReturnUnboxValue(mpf_get_prec(self->mpf));
}

static KMETHOD Mpf_Setprec(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *self = (kMpf *)sfp[0].asObject;
	mpf_set_prec(self->mpf, sfp[1].intValue);
}

static KMETHOD Mpf_toString(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *self = (kMpf *)sfp[0].asObject;
	char buf[1024]; // FIXME: find better way to get length of converted string.
	gmp_snprintf(buf, 1024, "%.Fg", self->mpf);
	KReturn(KLIB new_kString(kctx, OnStack, buf, strlen(buf), StringPolicy_ASCII));
}

static KMETHOD Mpf_toInt(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *self = (kMpf *)sfp[0].asObject;
	KReturnUnboxValue(mpf_get_si(self->mpf));
}

static KMETHOD Mpf_toFloat(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *self = (kMpf *)sfp[0].asObject;
	KReturnUnboxValue(mpf_get_d(self->mpf));
}

static KMETHOD Int_toMpf(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *ret = (kMpf *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	mpf_set_si(ret->mpf, sfp[0].intValue);
	KReturn(ret);
}

static KMETHOD Float_toMpf(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *ret = (kMpf *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	mpf_set_d(ret->mpf, sfp[0].floatValue);
	KReturn(ret);
}

static KMETHOD String_toMpf(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *ret = (kMpf *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	const char *src  = kString_text(sfp[0].asString);
	mpf_set_str(ret->mpf, src, 10);
	KReturn(ret);
}

static KMETHOD Mpz_toMpf(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpz *self = (kMpz *)sfp[0].asObject;
	kMpf *ret  = (kMpf *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	mpf_set_z(ret->mpf, self->mpz);
	KReturn(ret);
}

static KMETHOD Mpf_toMpz(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *self = (kMpf *)sfp[0].asObject;
	kMpz *ret  = (kMpz *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	mpz_set_f(ret->mpz, self->mpf);
	KReturn(ret);
}

static KMETHOD Mpf_power(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *self = (kMpf *)sfp[0].asObject;
	kMpf *ret = (kMpf *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	mpf_pow_ui(ret->mpf, self->mpf, sfp[1].intValue);
	KReturn(ret);
}

static KMETHOD Mpf_opMINUS(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *lhs = (kMpf *)sfp[0].asObject;
	kMpf *ret = (kMpf *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	mpf_neg(ret->mpf, lhs->mpf);
	KReturn(ret);
}

static KMETHOD Mpf_sqrt(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *self = (kMpf *)sfp[0].asObject;
	kMpf *ret = (kMpf *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	mpf_sqrt(ret->mpf, self->mpf);
	KReturn(ret);
}

static KMETHOD Mpf_abs(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *lhs = (kMpf *)sfp[0].asObject;
	kMpf *ret = (kMpf *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	mpf_abs(ret->mpf, lhs->mpf);
	KReturn(ret);
}

static KMETHOD Mpf_opADD(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *lhs = (kMpf *)sfp[0].asObject;
	kMpf *rhs = (kMpf *)sfp[1].asObject;
	kMpf *ret = (kMpf *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	mpf_add(ret->mpf, lhs->mpf, rhs->mpf);
	KReturn(ret);
}

static KMETHOD Mpf_opADD_int(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *lhs = (kMpf *)sfp[0].asObject;
	kMpf *ret = (kMpf *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	mpf_set_si(ret->mpf, sfp[1].intValue);
	mpf_add(ret->mpf, lhs->mpf, ret->mpf);
	KReturn(ret);
}

static KMETHOD Int_opADD_mpf(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *rhs = (kMpf *)sfp[1].asObject;
	kMpf *ret = (kMpf *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	mpf_set_si(ret->mpf, sfp[0].intValue);
	mpf_add(ret->mpf, rhs->mpf, ret->mpf);
	KReturn(ret);
}

static KMETHOD Mpf_opSUB(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *lhs = (kMpf *)sfp[0].asObject;
	kMpf *rhs = (kMpf *)sfp[1].asObject;
	kMpf *ret = (kMpf *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	mpf_sub(ret->mpf, lhs->mpf, rhs->mpf);
	KReturn(ret);
}

static KMETHOD Mpf_opSUB_int(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *lhs = (kMpf *)sfp[0].asObject;
	kMpf *ret = (kMpf *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	mpf_set_si(ret->mpf, sfp[1].intValue);
	mpf_sub(ret->mpf, lhs->mpf, ret->mpf);
	KReturn(ret);
}

static KMETHOD Int_opSUB_mpf(KonohaContext *kctx, KonohaStack *sfp)
{	
	kMpf *rhs = (kMpf *)sfp[1].asObject;
	kMpf *ret = (kMpf *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	mpf_set_si(ret->mpf, sfp[0].intValue);
	mpf_sub(ret->mpf, ret->mpf, rhs->mpf);
	KReturn(ret);
}

static KMETHOD Mpf_opMUL(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *lhs = (kMpf *)sfp[0].asObject;
	kMpf *rhs = (kMpf *)sfp[1].asObject;
	kMpf *ret = (kMpf *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	mpf_mul(ret->mpf, lhs->mpf, rhs->mpf);
	KReturn(ret);
}

static KMETHOD Mpf_opMUL_int(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *lhs = (kMpf *)sfp[0].asObject;
	kMpf *ret = (kMpf *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	mpf_set_si(ret->mpf, sfp[1].intValue);
	mpf_mul(ret->mpf, lhs->mpf, ret->mpf);
	KReturn(ret);
}

static KMETHOD Int_opMUL_mpf(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *rhs = (kMpf *)sfp[1].asObject;
	kMpf *ret = (kMpf *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	mpf_set_si(ret->mpf, sfp[0].intValue);
	mpf_mul(ret->mpf, rhs->mpf, ret->mpf);
	KReturn(ret);
}

static KMETHOD Mpf_opDIV(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *lhs = (kMpf *)sfp[0].asObject;
	kMpf *rhs = (kMpf *)sfp[1].asObject;
	kMpf *ret = (kMpf *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	if(unlikely(mpf_sgn(rhs->mpf) == 0)){
		THROW_ZeroDividedException(kctx, sfp);
	}else{
		mpf_div(ret->mpf, lhs->mpf, rhs->mpf);
	}
	KReturn(ret);
}

static KMETHOD Mpf_opDIV_int(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf  *lhs = (kMpf *)sfp[0].asObject;
	kint_t rhs = sfp[1].intValue;
	kMpf  *ret = (kMpf *)KLIB new_kObject(kctx, OnStack, kObject_class(lhs), 0);
	if(unlikely(rhs == 0)){
		THROW_ZeroDividedException(kctx, sfp);
	}else{
		mpf_set_si(ret->mpf, rhs);
		mpf_div(ret->mpf, lhs->mpf, ret->mpf);
	}
	KReturn(ret);
}

static KMETHOD Int_opDIV_mpf(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *rhs = (kMpf *)sfp[1].asObject;
	kMpf *ret = (kMpf *)KLIB new_kObject(kctx, OnStack, kObject_class(rhs), 0);
	if(unlikely(mpf_sgn(rhs->mpf) == 0)){
		THROW_ZeroDividedException(kctx, sfp);
	}else{
		mpf_set_si(ret->mpf, sfp[0].intValue);
		mpf_div(ret->mpf, ret->mpf, rhs->mpf);
	}
	KReturn(ret);
}

static int kMpf_cmp(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *lhs = (kMpf *)sfp[0].asObject;
	kMpf *rhs = (kMpf *)sfp[1].asObject;
	return mpf_cmp(lhs->mpf, rhs->mpf);
}

static int kMpf_cmp_int(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *lhs = (kMpf *)sfp[0].asObject;
	return mpf_cmp_si(lhs->mpf, sfp[1].intValue);
}

static int kInt_cmp_mpf(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *rhs = (kMpf *)sfp[1].asObject;
	return -mpf_cmp_si(rhs->mpf, sfp[0].intValue);
}

static int kMpf_cmp_float(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *lhs = (kMpf *)sfp[0].asObject;
	return mpf_cmp_d(lhs->mpf, sfp[1].floatValue);
}

static int kFloat_cmp_mpf(KonohaContext *kctx, KonohaStack *sfp)
{
	kMpf *rhs = (kMpf *)sfp[1].asObject;
	return -mpf_cmp_d(rhs->mpf, sfp[0].floatValue);
}

static KMETHOD Mpf_opLT(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpf_cmp(kctx, sfp) < 0);
}

static KMETHOD Mpf_opGT(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpf_cmp(kctx, sfp) > 0);
}

static KMETHOD Mpf_opEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpf_cmp(kctx, sfp) == 0);
}

static KMETHOD Mpf_opLTEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpf_cmp(kctx, sfp) <= 0);
}

static KMETHOD Mpf_opGTEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpf_cmp(kctx, sfp) >= 0);
}

static KMETHOD Mpf_opNEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpf_cmp(kctx, sfp) != 0);
}

static KMETHOD Mpf_opLT_int(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpf_cmp_int(kctx, sfp) < 0);
}

static KMETHOD Mpf_opGT_int(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpf_cmp_int(kctx, sfp) > 0);
}

static KMETHOD Mpf_opEQ_int(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpf_cmp_int(kctx, sfp) == 0);
}

static KMETHOD Mpf_opLTEQ_int(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpf_cmp_int(kctx, sfp) <= 0);
}

static KMETHOD Mpf_opGTEQ_int(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpf_cmp_int(kctx, sfp) >= 0);
}

static KMETHOD Mpf_opNEQ_int(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpf_cmp_int(kctx, sfp) != 0);
}

static KMETHOD Int_opLT_mpf(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kInt_cmp_mpf(kctx, sfp) < 0);
}

static KMETHOD Int_opGT_mpf(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kInt_cmp_mpf(kctx, sfp) > 0);
}

static KMETHOD Int_opEQ_mpf(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kInt_cmp_mpf(kctx, sfp) == 0);
}

static KMETHOD Int_opLTEQ_mpf(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kInt_cmp_mpf(kctx, sfp) <= 0);
}

static KMETHOD Int_opGTEQ_mpf(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kInt_cmp_mpf(kctx, sfp) >= 0);
}

static KMETHOD Int_opNEQ_mpf(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kInt_cmp_mpf(kctx, sfp) != 0);
}

static KMETHOD Mpf_opLT_float(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpf_cmp_float(kctx, sfp) < 0);
}

static KMETHOD Mpf_opGT_float(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpf_cmp_float(kctx, sfp) > 0);
}

static KMETHOD Mpf_opEQ_float(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpf_cmp_float(kctx, sfp) == 0);
}

static KMETHOD Mpf_opLTEQ_float(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpf_cmp_float(kctx, sfp) <= 0);
}

static KMETHOD Mpf_opGTEQ_float(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpf_cmp_float(kctx, sfp) >= 0);
}

static KMETHOD Mpf_opNEQ_float(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kMpf_cmp_float(kctx, sfp) != 0);
}

static KMETHOD Float_opLT_mpf(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kFloat_cmp_mpf(kctx, sfp) < 0);
}

static KMETHOD Float_opGT_mpf(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kFloat_cmp_mpf(kctx, sfp) > 0);
}

static KMETHOD Float_opEQ_mpf(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kFloat_cmp_mpf(kctx, sfp) == 0);
}

static KMETHOD Float_opLTEQ_mpf(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kFloat_cmp_mpf(kctx, sfp) <= 0);
}

static KMETHOD Float_opGTEQ_mpf(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kFloat_cmp_mpf(kctx, sfp) >= 0);
}

static KMETHOD Float_opNEQ_mpf(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kFloat_cmp_mpf(kctx, sfp) != 0);
}

/* ------------------------------------------------------------------------ */

#define KType_Mpz     cMpz->typeId
#define KType_Mpf     cMpf->typeId

static kbool_t gmp_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequirePackage("Type.Float", trace);

	static KDEFINE_CLASS MpzDef = {0};
	SETSTRUCTNAME(MpzDef, Mpz);
	MpzDef.cflag = KClassFlag_Final;
	MpzDef.init  = Mpz_Init;
	MpzDef.free  = Mpz_Free;
	MpzDef.format     = Mpz_format;
	static KDEFINE_CLASS MpfDef = {0};
	SETSTRUCTNAME(MpfDef, Mpf);
	MpfDef.cflag = KClassFlag_Final;
	MpfDef.init  = Mpf_Init;
	MpfDef.free  = Mpf_Free;
	MpfDef.format     = Mpf_format;

	KClass *cMpz = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &MpzDef, trace);
	KClass *cMpf = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &MpfDef, trace);

	int FN_x = KFieldName_("x");
	KDEFINE_METHOD MethodData[] = {
		// Mpz
		_Public|_Const,     _F(Mpz_new),        KType_Mpz,     KType_Mpz, KMethodName_("new"), 0,
		_Public|_Const,     _F(Mpz_new_mpz),    KType_Mpz,     KType_Mpz, KMethodName_("new"), 1, KType_Mpz, FN_x,
		_Public|_Const,     _F(Mpz_new_int),    KType_Mpz,     KType_Mpz, KMethodName_("new"), 1, KType_Int, FN_x,
		_Public|_Const,     _F(Mpz_new_str),    KType_Mpz,     KType_Mpz, KMethodName_("new"), 1, KType_String, FN_x,
		_Public|_Im|_Const          , _F(Mpz_toString),   KType_String,  KType_Mpz, KMethodName_To(KType_String),   0,
		_Public|_Im|_Const|_Coercion, _F(Mpz_toInt),      KType_Int,     KType_Mpz, KMethodName_To(KType_Int),   0,
		_Public|_Im|_Const|_Coercion, _F(Int_toMpz),      KType_Mpz,     KType_Int, KMethodName_To(KType_Mpz),   0,
		_Public|_Im|_Const, _F(String_toMpz),   KType_Mpz,     KType_String, KMethodName_To(KType_Mpz),   0,
		_Public|_Im|_Const, _F(Mpz_getSize),    KType_Int,     KType_Mpz, KMethodName_("getsize"), 0,
		_Public|_Im|_Const, _F(Mpz_isEven),     KType_Boolean, KType_Mpz, KMethodName_("isEven"), 0,
		_Public|_Im|_Const, _F(Mpz_power),      KType_Mpz,     KType_Mpz, KMethodName_("power"), 1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Mpz_opMINUS),    KType_Mpz,     KType_Mpz, KMethodName_("-"),   0,
		_Public|_Im|_Const, _F(Mpz_abs),        KType_Mpz,     KType_Mpz, KMethodName_("abs"), 0,
		_Public|_Im|_Const, _F(Mpz_opADD),      KType_Mpz,     KType_Mpz, KMethodName_("+"),   1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opADD_int),  KType_Mpz,     KType_Mpz, KMethodName_("+"),   1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Int_opADD_mpz),  KType_Mpz,     KType_Int, KMethodName_("+"),   1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opSUB),      KType_Mpz,     KType_Mpz, KMethodName_("-"),   1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opSUB_int),  KType_Mpz,     KType_Mpz, KMethodName_("-"),   1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Int_opSUB_mpz),  KType_Mpz,     KType_Int, KMethodName_("-"),   1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opMUL),      KType_Mpz,     KType_Mpz, KMethodName_("*"),   1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opMUL_int),  KType_Mpz,     KType_Mpz, KMethodName_("*"),   1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Int_opMUL_mpz),  KType_Mpz,     KType_Int, KMethodName_("*"),   1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opMOD),      KType_Mpz,     KType_Mpz, KMethodName_("%"),   1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opMOD_int),  KType_Mpz,     KType_Mpz, KMethodName_("%"),   1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Int_opMOD_mpz),  KType_Mpz,     KType_Int, KMethodName_("%"),   1, KType_Mpz, FN_x,
		_Public|_Im       , _F(Mpz_opDIV),      KType_Mpz,     KType_Mpz, KMethodName_("/"),   1, KType_Mpz, FN_x,
		_Public|_Im       , _F(Mpz_opDIV_int),  KType_Mpz,     KType_Mpz, KMethodName_("/"),   1, KType_Int, FN_x,
		_Public|_Im       , _F(Int_opDIV_mpz),  KType_Mpz,     KType_Int, KMethodName_("/"),   1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opAND),      KType_Mpz,     KType_Mpz, KMethodName_("&"),   1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opOR),       KType_Mpz,     KType_Mpz, KMethodName_("|"),   1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opXOR),      KType_Mpz,     KType_Mpz, KMethodName_("^"),   1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opLT),       KType_Boolean, KType_Mpz, KMethodName_("<"),   1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opGT),       KType_Boolean, KType_Mpz, KMethodName_(">"),   1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opEQ),       KType_Boolean, KType_Mpz, KMethodName_("=="),  1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opLTEQ),     KType_Boolean, KType_Mpz, KMethodName_("<="),  1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opGTEQ),     KType_Boolean, KType_Mpz, KMethodName_(">="),  1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opNEQ),      KType_Boolean, KType_Mpz, KMethodName_("!="),  1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Mpz_opLT_int),   KType_Boolean, KType_Mpz, KMethodName_("<"),   1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Mpz_opGT_int),   KType_Boolean, KType_Mpz, KMethodName_(">"),   1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Mpz_opEQ_int),   KType_Boolean, KType_Mpz, KMethodName_("=="),  1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Mpz_opLTEQ_int), KType_Boolean, KType_Mpz, KMethodName_("<="),  1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Mpz_opGTEQ_int), KType_Boolean, KType_Mpz, KMethodName_(">="),  1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Mpz_opNEQ_int),  KType_Boolean, KType_Mpz, KMethodName_("!="),  1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Int_opLT_mpz),   KType_Boolean, KType_Int, KMethodName_("<"),   1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Int_opGT_mpz),   KType_Boolean, KType_Int, KMethodName_(">"),   1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Int_opEQ_mpz),   KType_Boolean, KType_Int, KMethodName_("=="),  1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Int_opLTEQ_mpz), KType_Boolean, KType_Int, KMethodName_("<="),  1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Int_opGTEQ_mpz), KType_Boolean, KType_Int, KMethodName_(">="),  1, KType_Mpz, FN_x,
		_Public|_Im|_Const, _F(Int_opNEQ_mpz),  KType_Boolean, KType_Int, KMethodName_("!="),  1, KType_Mpz, FN_x,
		// Mpf
		_Public|_Const,     _F(Mpf_new),        KType_Mpf,     KType_Mpf, KMethodName_("new"), 0,
		_Public|_Const,     _F(Mpf_new_float),  KType_Mpf,     KType_Mpf, KMethodName_("new"), 1, KType_float, FN_x,
		_Public|_Const,     _F(Mpf_new_mpf),    KType_Mpf,     KType_Mpf, KMethodName_("new"), 1, KType_Mpf, FN_x,
		_Public|_Const,     _F(Mpf_new_int),    KType_Mpf,     KType_Mpf, KMethodName_("new"), 1, KType_Int, FN_x,
		_Public|_Const,     _F(Mpf_new_mpz),    KType_Mpf,     KType_Mpf, KMethodName_("new"), 1, KType_Mpz, FN_x,
		_Public|_Const,     _F(Mpf_new_str),    KType_Mpf,     KType_Mpf, KMethodName_("new"), 1, KType_String, FN_x,
		_Public|_Im,        _F(Mpf_getprec),    KType_Mpf,     KType_Int, KMethodName_("getprec"), 0,
		_Public,            _F(Mpf_Setprec),    KType_Mpf,     KType_void,KMethodName_("setprec"), 1, KType_Int, FN_x,
		_Public|_Im|_Const          , _F(Mpf_toString),   KType_String,  KType_Mpf, KMethodName_To(KType_String),   0,
		_Public|_Im|_Const|_Coercion, _F(Mpf_toInt),      KType_Int,     KType_Mpf, KMethodName_To(KType_Int),   0,
		_Public|_Im|_Const|_Coercion, _F(Mpf_toFloat),    KType_float,   KType_Mpf, KMethodName_To(KType_float), 0,
		_Public|_Im|_Const|_Coercion, _F(Int_toMpf),      KType_Mpf,     KType_Int, KMethodName_To(KType_Mpf),   0,
		_Public|_Im|_Const|_Coercion, _F(Float_toMpf),    KType_Mpf,     KType_float, KMethodName_To(KType_Mpf),   0,
		_Public|_Im|_Const, _F(Mpz_toMpf),      KType_Mpf,     KType_Mpz, KMethodName_To(KType_Mpf),   0,
		_Public|_Im|_Const, _F(Mpf_toMpz),      KType_Mpz,     KType_Mpf, KMethodName_To(KType_Mpz),   0,
		_Public|_Im|_Const, _F(String_toMpf),   KType_Mpf,     KType_String, KMethodName_To(KType_Mpf),   0,
		_Public|_Im|_Const, _F(Mpf_power),      KType_Mpf,     KType_Mpf, KMethodName_("power"), 1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Mpf_opMINUS),    KType_Mpf,     KType_Mpf, KMethodName_("-"),   0,
		_Public|_Im|_Const, _F(Mpf_abs),        KType_Mpf,     KType_Mpf, KMethodName_("abs"), 0,
		_Public|_Im|_Const, _F(Mpf_sqrt),       KType_Mpf,     KType_Mpf, KMethodName_("sqrt"), 0,
		_Public|_Im|_Const, _F(Mpf_opADD),      KType_Mpf,     KType_Mpf, KMethodName_("+"),   1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Mpf_opADD_int),  KType_Mpf,     KType_Mpf, KMethodName_("+"),   1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Int_opADD_mpf),  KType_Mpf,     KType_Int, KMethodName_("+"),   1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Mpf_opSUB),      KType_Mpf,     KType_Mpf, KMethodName_("-"),   1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Mpf_opSUB_int),  KType_Mpf,     KType_Mpf, KMethodName_("-"),   1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Int_opSUB_mpf),  KType_Mpf,     KType_Int, KMethodName_("-"),   1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Mpf_opMUL),      KType_Mpf,     KType_Mpf, KMethodName_("*"),   1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Mpf_opMUL_int),  KType_Mpf,     KType_Mpf, KMethodName_("*"),   1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Int_opMUL_mpf),  KType_Mpf,     KType_Int, KMethodName_("*"),   1, KType_Mpf, FN_x,
		_Public|_Im       , _F(Mpf_opDIV),      KType_Mpf,     KType_Mpf, KMethodName_("/"),   1, KType_Mpf, FN_x,
		_Public|_Im       , _F(Mpf_opDIV_int),  KType_Mpf,     KType_Mpf, KMethodName_("/"),   1, KType_Int, FN_x,
		_Public|_Im       , _F(Int_opDIV_mpf),  KType_Mpf,     KType_Int, KMethodName_("/"),   1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Mpf_opLT),       KType_Boolean, KType_Mpf, KMethodName_("<"),   1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Mpf_opGT),       KType_Boolean, KType_Mpf, KMethodName_(">"),   1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Mpf_opEQ),       KType_Boolean, KType_Mpf, KMethodName_("=="),  1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Mpf_opLTEQ),     KType_Boolean, KType_Mpf, KMethodName_("<="),  1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Mpf_opGTEQ),     KType_Boolean, KType_Mpf, KMethodName_(">="),  1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Mpf_opNEQ),      KType_Boolean, KType_Mpf, KMethodName_("!="),  1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Mpf_opLT_int),   KType_Boolean, KType_Mpf, KMethodName_("<"),   1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Mpf_opGT_int),   KType_Boolean, KType_Mpf, KMethodName_(">"),   1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Mpf_opEQ_int),   KType_Boolean, KType_Mpf, KMethodName_("=="),  1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Mpf_opLTEQ_int), KType_Boolean, KType_Mpf, KMethodName_("<="),  1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Mpf_opGTEQ_int), KType_Boolean, KType_Mpf, KMethodName_(">="),  1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Mpf_opNEQ_int),  KType_Boolean, KType_Mpf, KMethodName_("!="),  1, KType_Int, FN_x,
		_Public|_Im|_Const, _F(Int_opLT_mpf),   KType_Boolean, KType_Int, KMethodName_("<"),   1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Int_opGT_mpf),   KType_Boolean, KType_Int, KMethodName_(">"),   1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Int_opEQ_mpf),   KType_Boolean, KType_Int, KMethodName_("=="),  1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Int_opLTEQ_mpf), KType_Boolean, KType_Int, KMethodName_("<="),  1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Int_opGTEQ_mpf), KType_Boolean, KType_Int, KMethodName_(">="),  1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Int_opNEQ_mpf),  KType_Boolean, KType_Int, KMethodName_("!="),  1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Mpf_opLT_float),   KType_Boolean, KType_Mpf, KMethodName_("<"),   1, KType_float, FN_x,
		_Public|_Im|_Const, _F(Mpf_opGT_float),   KType_Boolean, KType_Mpf, KMethodName_(">"),   1, KType_float, FN_x,
		_Public|_Im|_Const, _F(Mpf_opEQ_float),   KType_Boolean, KType_Mpf, KMethodName_("=="),  1, KType_float, FN_x,
		_Public|_Im|_Const, _F(Mpf_opLTEQ_float), KType_Boolean, KType_Mpf, KMethodName_("<="),  1, KType_float, FN_x,
		_Public|_Im|_Const, _F(Mpf_opGTEQ_float), KType_Boolean, KType_Mpf, KMethodName_(">="),  1, KType_float, FN_x,
		_Public|_Im|_Const, _F(Mpf_opNEQ_float),  KType_Boolean, KType_Mpf, KMethodName_("!="),  1, KType_float, FN_x,
		_Public|_Im|_Const, _F(Float_opLT_mpf),   KType_Boolean, KType_float, KMethodName_("<"),   1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Float_opGT_mpf),   KType_Boolean, KType_float, KMethodName_(">"),   1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Float_opEQ_mpf),   KType_Boolean, KType_float, KMethodName_("=="),  1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Float_opLTEQ_mpf), KType_Boolean, KType_float, KMethodName_("<="),  1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Float_opGTEQ_mpf), KType_Boolean, KType_float, KMethodName_(">="),  1, KType_Mpf, FN_x,
		_Public|_Im|_Const, _F(Float_opNEQ_mpf),  KType_Boolean, KType_float, KMethodName_("!="),  1, KType_Mpf, FN_x,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	return true;
}

static kbool_t gmp_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

/* ======================================================================== */

KDEFINE_PACKAGE *Gmp_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "gmp", "1.0");
	d.PackupNameSpace    = gmp_PackupNameSpace;
	d.ExportNameSpace    = gmp_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
