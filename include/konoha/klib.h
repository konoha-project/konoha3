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

#ifndef KLIB_H_
#define KLIB_H_

#ifndef MINIOKNOHA_H_
#error Do not include klib.h without konoha.h.
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

#define KFileLine_textFileName(X)  kString_text(KFileLine_GetFileName(kctx, X))
static kinline kString* KFileLine_GetFileName(KonohaContext *kctx, kfileline_t fileid)
{
	kfileline_t n = (fileid >> (sizeof(kshort_t) * 8));
	DBG_ASSERT(n < kArray_size(kctx->share->fileIdList));
	return kctx->share->fileIdList->stringItems[n];
}

#define KPackage_text(X)    kString_text(KPackage_GetName(kctx, X))
static kinline kString* KPackage_GetName(KonohaContext *kctx, kpackageId_t packageId)
{
	DBG_ASSERT(packageId < kArray_size(kctx->share->packageIdList));
	return kctx->share->packageIdList->stringItems[packageId];
}

#define KClass_text(X)   kString_text(KClass_GetName(kctx, X))
static kinline kString* KClass_GetName(KonohaContext *kctx, KClass *ct)
{
	return kctx->klib->KClass_shortName(kctx, ct);
}

#define KType_text(X)   kString_text(KType_GetString(kctx,KTypeAttr_Unmask(X)))
static kinline kString* KType_GetString(KonohaContext *kctx, ktypeattr_t ty)
{
	DBG_ASSERT(ty < KARRAYSIZE(kctx->share->classTable.bytemax, intptr));
	return KClass_GetName(kctx, KClass_(ty));
}

#define KSymbol_text(sym)   kString_text(KSymbol_GetString(kctx, sym))
static kinline kString* KSymbol_GetString(KonohaContext *kctx, ksymbol_t sym)
{
	size_t index = (size_t) KSymbol_Unmask(sym);
	DBG_ASSERT(index < kArray_size(kctx->share->symbolList));
	return kctx->share->symbolList->stringItems[index];
}

#define KSymbol_Fmt2(sym)   KSymbol_prefixText(sym), kString_text(KSymbol_GetString(kctx, sym))
static kinline const char* KSymbol_prefixText(ksymbol_t sym)
{
	size_t mask = ((size_t)(KSymbol_Attr(sym)) >> ((sizeof(ksymbol_t) * 8)-3));
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
	if(KSymbol_Attr(s1) == KSymbol_Attr(s2)) {
		const char *t1 = kString_text(kctx->share->symbolList->stringItems[KSymbol_Unmask(s1)]);
		const char *t2 = kString_text(kctx->share->symbolList->stringItems[KSymbol_Unmask(s2)]);
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

static kinline KClass *KClass_p0(KonohaContext *kctx, KClass *ct, ktypeattr_t ty)
{
	kparamtype_t p = {ty, 0};
	return KLIB KClass_Generics(kctx, ct, KType_void, 1, &p);
}

static kinline void map_Addu(KonohaContext *kctx, KHashMap *kmp, uintptr_t hcode, uintptr_t unboxValue)
{
	KHashMapEntry *e = KLIB KHashMap_newEntry(kctx, kmp, hcode);
	e->unboxValue = unboxValue;
}

static kinline uintptr_t map_getu(KonohaContext *kctx, KHashMap *kmp, uintptr_t hcode, uintptr_t def)
{
	KHashMapEntry *e = KLIB KHashMap_get(kctx, kmp, hcode);
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

#define KCheckIndex(N, MAX) do {\
	if(unlikely (!(((size_t)N) < ((size_t)MAX)))) {\
		KLIB KRuntime_raise(kctx, KException_("OutOfBoundary"), SoftwareFault, NULL, sfp);\
	}\
} while(0)


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

//static kinline void Method_SetProceedMethod(KonohaContext *kctx, kMethod *mtd, kMethod *mtd2)
//{
//	DBG_ASSERT(mtd != mtd2);
//	DBG_ASSERT(mtd->proceedNUL == NULL);
//	KFieldInit(mtd, ((kMethodVar *)mtd)->proceedNUL, mtd2);
//}

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* KLIB_H_ */
