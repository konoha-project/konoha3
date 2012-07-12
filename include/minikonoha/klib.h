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

#ifndef KONOHA2_INLINELIBS_H_
#define KONOHA2_INLINELIBS_H_

#define kinline __attribute__((unused))

static kinline size_t size64(size_t s)
{
	size_t base = sizeof(struct _kObject);
	while(base < s) {
		base *= 2;
	}
	return base;
}

static kinline uintptr_t strhash(const char *name, size_t len)
{
	uintptr_t i, hcode = 0;
	for(i = 0; i < len; i++) {
		hcode = name[i] + (31 * hcode);
	}
	return hcode;
}

static kinline const char* shortfilename(const char *str)
{
	/*XXX g++ 4.4.5 need char* cast to compile it. */
	char *p = (char *) strrchr(str, '/');
	return (p == NULL) ? str : (const char*)p+1;
}

#define SS_s(X)  SS_s_(kctx, X)
#define SS_t(X)  S_text(SS_s_(kctx, X))

static kinline kString* SS_s_(KonohaContext *kctx, kline_t fileid)
{
	kline_t n = (fileid >> (sizeof(kshort_t) * 8));
	DBG_ASSERT(n < kArray_size(kctx->share->fileidList));
	return kctx->share->fileidList->strings[n];
}

#define PN_s(X)    PN_s_(kctx, X)
#define PN_t(X)    S_text(PN_s_(kctx, X))
static kinline kString* PN_s_(KonohaContext *kctx, kpack_t packid)
{
	DBG_ASSERT(packid < kArray_size(kctx->share->packList));
	return kctx->share->packList->strings[packid];
}

#define CT_s(X)   CT_s_(kctx, X)
#define CT_t(X)   S_text(CT_s_(kctx, X))
static kinline kString* CT_s_(KonohaContext *kctx, kclass_t *ct)
{
	return kctx->lib2->KCT_shortName(kctx, ct);
}

#define TY_s(X)   TY_s_(kctx, X)
#define TY_t(X)   S_text(TY_s(X))
static kinline kString* TY_s_(KonohaContext *kctx, ktype_t ty)
{
	DBG_ASSERT(ty < KARRAYSIZE(kctx->share->ca.bytemax, intptr));
	return CT_s_(kctx, CT_(ty));
}

#define SYM_s(fn)   SYM_s_(kctx, fn)
#define SYM_t(fn)   S_text(SYM_s_(kctx, fn))
static kinline kString* SYM_s_(KonohaContext *kctx, ksymbol_t sym)
{
	size_t index = (size_t) SYM_UNMASK(sym);
//	if(!(index < kArray_size(kctx->share->symbolList))) {
//		DBG_P("index=%d, size=%d", index, kArray_size(kctx->share->symbolList));
//	}
	DBG_ASSERT(index < kArray_size(kctx->share->symbolList));
	return kctx->share->symbolList->strings[index];
}

static kinline const char* SYM_PRE(ksymbol_t sym)
{
	size_t mask = ((size_t)(SYM_HEAD(sym)) >> ((sizeof(ksymbol_t) * 8)-3));
	DBG_ASSERT(mask < 8);
	static const char* prefixes[] = {
		/*000*/ "",   /*001*/ "get", /*010*/ "set", /*011*/ "@",
		/*100*/ "is", /*101*/ "",    /*110*/ "to",  /*111*/ "$",
	};
	return prefixes[mask];
}

#define SYM_equals(S1, S2)     sym_equals(kctx, S1, S2)
static kinline kbool_t sym_equals(KonohaContext *kctx, ksymbol_t s1, ksymbol_t s2)
{
	if(SYM_HEAD(s1) == SYM_HEAD(s2)) {
		const char *t1 = S_text(kctx->share->symbolList->strings[SYM_UNMASK(s1)]);
		const char *t2 = S_text(kctx->share->symbolList->strings[SYM_UNMASK(s2)]);
		while(1) {
			if(t1[0] != t2[0]) {
				if(t1[0] == '_') { t1++; continue; }
				if(t2[0] == '_') { t2++; continue; }
				if(toupper(t1[0]) != toupper(t2[0])) break;
			}
			if(t1[0] == 0) return true;
			t1++; t2++;
		}
	}
	return false;
}


static kinline uintptr_t longid(kushort_t packdom, kushort_t un)
{
	uintptr_t hcode = packdom;
	return (hcode << (sizeof(kshort_t)*8)) | un;
}

static kinline kclass_t *CT_p0(KonohaContext *kctx, kclass_t *ct, ktype_t ty)
{
	kparam_t p = {ty, 0};
	return kClassTable_Generics(ct, TY_void, 1, &p);
}

#define uNULL   ((uintptr_t)NULL)
static kinline void map_addu(KonohaContext *kctx, kmap_t *kmp, uintptr_t hcode, uintptr_t uvalue)
{
	kmape_t *e = kmap_newentry(kmp, hcode);
	e->uvalue = uvalue;
}

static kinline uintptr_t map_getu(KonohaContext *kctx, kmap_t *kmp, uintptr_t hcode, uintptr_t def)
{
	kmape_t *e = kmap_get(kmp, hcode);
	while(e != NULL) {
		if(e->hcode == hcode) return e->uvalue;
	}
	return def;
}

static kinline const char* TAG_t(kinfotag_t t)
{
	DBG_ASSERT(t <= NoneTag);
	static const char* tags[] = {
		"(error) ", /*CritTag*/
		"(error) ", /*ErrTag*/
		"(warning) ", /*WarnTag*/
		"(notice) ", /*NoticeTag*/
		"(info) ", /*InfoTag*/
		"(debug) ", /*DebugTag*/
		"", /* NoneTag*/
	};
	return tags[(int)t];
}

static kinline size_t check_index(KonohaContext *kctx, kint_t n, size_t max, kline_t pline)
{
	size_t n1 = (size_t)n;
	if(unlikely(!(n1 < max))) {
		kreportf(CRIT_, pline, "Script!!: out of array index %ld < %lu", n, max);
	}
	return n1;
}

#ifdef USE_STRINGLIB

#define utf8len(c)    _utf8len[(int)c]

static const char _utf8len[] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 0, 0,
};
#endif

static kinline void Method_setProceedMethod(KonohaContext *kctx, kMethod *mtd, kMethod *mtd2)
{
	DBG_ASSERT(mtd != mtd2);
	DBG_ASSERT(mtd->proceedNUL == NULL);
	KINITv(((struct _kMethod*)mtd)->proceedNUL, mtd2);
}

#endif /* KONOHA2_INLINELIBS_H_ */
