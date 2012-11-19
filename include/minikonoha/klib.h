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

#ifndef MINIOKNOHA_INLINELIBS_H_
#define MINIOKNOHA_INLINELIBS_H_

//	CASE(NOP) {
//		OPNOP *op = (OPNOP *)pc;
//		OPEXEC_NOP();  pc++;
//		GOTO_NEXT();
//	}
//	CASE(THCODE) {
//		OPTHCODE *op = (OPTHCODE *)pc;
//		OPEXEC_THCODE(op->codesize, op->threadCode); pc++;
//		GOTO_NEXT();
//	}
//	CASE(ENTER) {
//		OPENTER *op = (OPENTER *)pc;
//		OPEXEC_ENTER(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(EXIT) {
//		OPEXIT *op = (OPEXIT *)pc;
//		OPEXEC_EXIT(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(NSET) {
//		OPNSET *op = (OPNSET *)pc;
//		OPEXEC_NSET(op->a, op->n, op->ty); pc++;
//		GOTO_NEXT();
//	}
//	CASE(NMOV) {
//		OPNMOV *op = (OPNMOV *)pc;
//		OPEXEC_NMOV(op->a, op->b, op->ty); pc++;
//		GOTO_NEXT();
//	}
//	CASE(NMOVx) {
//		OPNMOVx *op = (OPNMOVx *)pc;
//		OPEXEC_NMOVx(op->a, op->b, op->bx, op->ty); pc++;
//		GOTO_NEXT();
//	}
//	CASE(XNMOV) {
//		OPXNMOV *op = (OPXNMOV *)pc;
//		OPEXEC_XNMOV(op->a, op->ax, op->b, op->ty); pc++;
//		GOTO_NEXT();
//	}
//	CASE(NEW) {
//		OPNEW *op = (OPNEW *)pc;
//		OPEXEC_NEW(op->a, op->p, op->ty); pc++;
//		GOTO_NEXT();
//	}
//	CASE(NULL) {
//		OPNULL *op = (OPNULL *)pc;
//		OPEXEC_NULL(op->a, op->ty); pc++;
//		GOTO_NEXT();
//	}
//	CASE(LOOKUP) {
//		OPLOOKUP *op = (OPLOOKUP *)pc;
//		OPEXEC_LOOKUP(op->thisidx, op->ns, op->mtd); pc++;
//		GOTO_NEXT();
//	}
//	CASE(CALL) {
//		OPCALL *op = (OPCALL *)pc;
//		OPEXEC_CALL(op->uline, op->thisidx, op->espshift, op->tyo); pc++;
//		GOTO_NEXT();
//	}
//	CASE(RET) {
//		OPRET *op = (OPRET *)pc;
//		OPEXEC_RET(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(NCALL) {
//		OPNCALL *op = (OPNCALL *)pc;
//		OPEXEC_NCALL(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(BNOT) {
//		OPBNOT *op = (OPBNOT *)pc;
//		OPEXEC_BNOT(op->c, op->a); pc++;
//		GOTO_NEXT();
//	}
//	CASE(JMP) {
//		OPJMP *op = (OPJMP *)pc;
//		OPEXEC_JMP(pc = op->jumppc, JUMP); pc++;
//		GOTO_NEXT();
//	}
//	CASE(JMPF) {
//		OPJMPF *op = (OPJMPF *)pc;
//		OPEXEC_JMPF(pc = op->jumppc, JUMP, op->a);pc++;
//		GOTO_NEXT();
//	}
//	CASE(TRYJMP) {
//		OPTRYJMP *op = (OPTRYJMP *)pc;
//		OPEXEC_TRYJMP(pc = op->jumppc, JUMP); pc++;
//		GOTO_NEXT();
//	}
//	CASE(YIELD) {
//		OPYIELD *op = (OPYIELD *)pc;
//		OPEXEC_YIELD(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(ERROR) {
//		OPERROR *op = (OPERROR *)pc;
//		OPEXEC_ERROR(op->uline, op->msg, op->esp); pc++;
//		GOTO_NEXT();
//	}
//	CASE(SAFEPOINT) {
//		OPSAFEPOINT *op = (OPSAFEPOINT *)pc;
//		OPEXEC_SAFEPOINT(op->uline, op->esp); pc++;
//		GOTO_NEXT();
//	}
//	CASE(CHKSTACK) {
//		OPCHKSTACK *op = (OPCHKSTACK *)pc;
//		OPEXEC_CHKSTACK(op->uline); pc++;
//		GOTO_NEXT();
//	}
//	CASE(TRACE) {
//		OPTRACE *op = (OPTRACE *)pc;
//		OPEXEC_TRACE(op->uline, op->thisidx, op->trace); pc++;
//		GOTO_NEXT();
//	}

#ifndef MINIOKNOHA_H_
#error Do not include klib.h without minikonoha.h.
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#define kinline __attribute__((unused))
#else
#define kinline
#endif

static kinline size_t size64(size_t s)
{
	size_t base = sizeof(kObjectVar);
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

#define FileId_s(X)  FileId_s_(kctx, X)
#define FileId_t(X)  S_text(FileId_s_(kctx, X))
static kinline kString* FileId_s_(KonohaContext *kctx, kfileline_t fileid)
{
	kfileline_t n = (fileid >> (sizeof(kshort_t) * 8));
	DBG_ASSERT(n < kArray_size(kctx->share->fileIdList_OnGlobalConstList));
	return kctx->share->fileIdList_OnGlobalConstList->stringItems[n];
}

#define PackageId_s(X)    PackageId_s_(kctx, X)
#define PackageId_t(X)    S_text(PackageId_s_(kctx, X))
static kinline kString* PackageId_s_(KonohaContext *kctx, kpackageId_t packageId)
{
	DBG_ASSERT(packageId < kArray_size(kctx->share->packageIdList_OnGlobalConstList));
	return kctx->share->packageIdList_OnGlobalConstList->stringItems[packageId];
}

#define CT_s(X)   CT_s_(kctx, X)
#define CT_t(X)   S_text(CT_s_(kctx, X))
static kinline kString* CT_s_(KonohaContext *kctx, KonohaClass *ct)
{
	return kctx->klib->KonohaClass_shortName(kctx, ct);
}

#define TY_s(X)   TY_s_(kctx, X)
#define TY_t(X)   S_text(TY_s(X))
static kinline kString* TY_s_(KonohaContext *kctx, ktype_t ty)
{
	DBG_ASSERT(ty < KARRAYSIZE(kctx->share->classTable.bytemax, intptr));
	return CT_s_(kctx, CT_(ty));
}

#define SYM_s(sym)   SYM_s_(kctx, sym)
#define SYM_t(sym)   S_text(SYM_s_(kctx, sym))
static kinline kString* SYM_s_(KonohaContext *kctx, ksymbol_t sym)
{
	size_t index = (size_t) SYM_UNMASK(sym);
//	if(!(index < kArray_size(kctx->share->symbolList_OnGlobalConstList))) {
//		DBG_P("index=%d, size=%d", index, kArray_size(kctx->share->symbolList_OnGlobalConstList));
//	}
	DBG_ASSERT(index < kArray_size(kctx->share->symbolList_OnGlobalConstList));
	return kctx->share->symbolList_OnGlobalConstList->stringItems[index];
}

#define PSYM_t(sym)   SYM_PRE(sym),S_text(SYM_s_(kctx, sym))
static kinline const char* SYM_PRE(ksymbol_t sym)
{
	size_t mask = ((size_t)(SYM_HEAD(sym)) >> ((sizeof(ksymbol_t) * 8)-3));
	DBG_ASSERT(mask < 8);
	static const char* prefixes[] = {
		/*000*/ "",   /*001*/ "set", /*010*/ "get", /*011*/ "@",
		/*100*/ "",   /*101*/ "as",  /*110*/ "to",  /*111*/ "$",
	};
	return prefixes[mask];
}

#define SYM_equals(S1, S2)     sym_equals(kctx, S1, S2)
static kinline kbool_t sym_equals(KonohaContext *kctx, ksymbol_t s1, ksymbol_t s2)
{
	if(SYM_HEAD(s1) == SYM_HEAD(s2)) {
		const char *t1 = S_text(kctx->share->symbolList_OnGlobalConstList->stringItems[SYM_UNMASK(s1)]);
		const char *t2 = S_text(kctx->share->symbolList_OnGlobalConstList->stringItems[SYM_UNMASK(s2)]);
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

static kinline uintptr_t longid(kushort_t packageDomain, kushort_t un)
{
	uintptr_t hcode = packageDomain;
	return (hcode << (sizeof(kshort_t)*8)) | un;
}

static kinline KonohaClass *CT_p0(KonohaContext *kctx, KonohaClass *ct, ktype_t ty)
{
	kparamtype_t p = {ty, 0};
	return KLIB KonohaClass_Generics(kctx, ct, TY_void, 1, &p);
}

#define uNULL   ((uintptr_t)NULL)
static kinline void map_Addu(KonohaContext *kctx, KHashMap *kmp, uintptr_t hcode, uintptr_t unboxValue)
{
	KHashMapEntry *e = KLIB Kmap_newEntry(kctx, kmp, hcode);
	e->unboxValue = unboxValue;
}

static kinline uintptr_t map_getu(KonohaContext *kctx, KHashMap *kmp, uintptr_t hcode, uintptr_t def)
{
	KHashMapEntry *e = KLIB Kmap_get(kctx, kmp, hcode);
	while(e != NULL) {
		if(e->hcode == hcode) return e->unboxValue;
		e = e->next;
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

#define KCheckIndex(N, MAX) \
	if(unlikely (!(((size_t)N) < ((size_t)MAX)))) {\
		KLIB KonohaRuntime_raise(kctx, EXPT_("OutOfBoundary"), SoftwareFault, NULL, sfp);\
	}\


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

//static kinline void Method_setProceedMethod(KonohaContext *kctx, kMethod *mtd, kMethod *mtd2)
//{
//	DBG_ASSERT(mtd != mtd2);
//	DBG_ASSERT(mtd->proceedNUL == NULL);
//	KFieldInit(mtd, ((kMethodVar *)mtd)->proceedNUL, mtd2);
//}

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* MINIOKNOHA_INLINELIBS_H_ */
