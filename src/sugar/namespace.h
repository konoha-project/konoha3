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


// ---------------------------------------------------------------------------
// Syntax Management

static void checkFuncArray(KonohaContext *kctx, kFunc **funcItems);
static void parseSyntaxRule(KonohaContext *kctx, const char *rule, kfileline_t pline, kArray *a);

static SugarSyntax* NameSpace_syn(KonohaContext *kctx, kNameSpace *ns0, ksymbol_t keyword, int isNew)
{
	kNameSpace *ns = ns0;
	uintptr_t hcode = keyword;
	SugarSyntax *parent = NULL;
	assert(ns0 != NULL);  /* scan-build: remove warning */
	while(ns != NULL) {
		if(ns->syntaxMapNN != NULL) {
			KUtilsHashMapEntry *e = KLIB Kmap_get(kctx, ns->syntaxMapNN, hcode);
			while(e != NULL) {
				if(e->hcode == hcode) {
					parent = (SugarSyntax*)e->uvalue;
					if(isNew && ns0 != ns) goto L_NEW;
					return parent;
				}
				e = e->next;
			}
		}
		ns = ns->parentNULL;
	}
	L_NEW:;
	if(isNew == 1) {
		if(ns0->syntaxMapNN == NULL) {
			((kNameSpaceVar*)ns0)->syntaxMapNN = KLIB Kmap_init(kctx, 0);
		}
		KUtilsHashMapEntry *e = KLIB Kmap_newEntry(kctx, ns0->syntaxMapNN, hcode);
		SugarSyntaxVar *syn = (SugarSyntaxVar*)KCALLOC(sizeof(SugarSyntax), 1);
		e->uvalue = (uintptr_t)syn;

		if(parent != NULL) {  // TODO: RCGC
			memcpy(syn, parent, sizeof(SugarSyntax));
			checkFuncArray(kctx, &(syn->PatternMatch));
			checkFuncArray(kctx, &(syn->ParseExpr));
			checkFuncArray(kctx, &(syn->TopStmtTyCheck));
			checkFuncArray(kctx, &(syn->StmtTyCheck));
			checkFuncArray(kctx, &(syn->ExprTyCheck));
		}
		else {
			syn->keyword  = keyword;
			syn->ty  = TY_unknown;
			syn->op1 = SYM_NONAME;
			syn->op2 = SYM_NONAME;
			KINITv(syn->PatternMatch, kmodsugar->UndefinedParseExpr);  // never called and avoid nullcheck
			KINITv(syn->ParseExpr, kmodsugar->UndefinedParseExpr);
			KINITv(syn->TopStmtTyCheck, kmodsugar->UndefinedStmtTyCheck);
			KINITv(syn->StmtTyCheck, kmodsugar->UndefinedStmtTyCheck);
			KINITv(syn->ExprTyCheck, kmodsugar->UndefinedExprTyCheck);
		}
		return syn;
	}
	return NULL;
}

static void checkFuncArray(KonohaContext *kctx, kFunc **funcItems)
{
	if(funcItems[0] != NULL && IS_Array(funcItems[0])) {
		size_t i;
		kArray *newa = new_(Array, 8), *a = (kArray*)funcItems[0];
		for(i = 0; i < kArray_size(a); i++) {
			KLIB kArray_add(kctx, newa, a->objectItems[i]);
		}
		KSETv(funcItems[0], (kFunc*)newa);
	}
}

static void kNameSpace_setSugarFunc(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, size_t idx, kFunc *fo)
{
	SugarSyntaxVar *syn = (SugarSyntaxVar *)NameSpace_syn(kctx, ns, keyword, 1/*new*/);
	kFunc **synp = &(syn->PatternMatch);
	DBG_ASSERT(idx <= SYNIDX_ExprTyCheck);
	KSETv(synp[idx], fo);
}

static void kNameSpace_addSugarFunc(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, size_t idx, kFunc *fo)
{
	SugarSyntaxVar *syn = (SugarSyntaxVar *)NameSpace_syn(kctx, ns, keyword, 1/*new*/);
	kFunc **synp = &(syn->PatternMatch);
	DBG_ASSERT(idx <= SYNIDX_ExprTyCheck);
	if(synp[idx] == kmodsugar->UndefinedParseExpr || synp[idx] == kmodsugar->UndefinedStmtTyCheck || synp[idx] == kmodsugar->UndefinedExprTyCheck) {
		KSETv(synp[idx], fo);
	}
	kArray *a = (kArray*)synp[idx];
	if(!IS_Array(a)) {
		PUSH_GCSTACK(fo);
		a = new_(Array, 0);
		KLIB kArray_add(kctx, a, synp[idx]);
		KSETv(synp[idx], (kFunc*)a);
	}
	KLIB kArray_add(kctx, a, fo);
}

static void setSugarFunc(KonohaContext *kctx, MethodFunc f, kFunc **funcItems, MethodFunc *p, kFunc **mp)
{
	if(f != NULL) {
		if(f != p[0]) {
			p[0] = f;
			mp[0] = new_SugarFunc(f);
		}
		KINITv(funcItems[0], mp[0]);
	}
}

static void kNameSpace_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KDEFINE_SYNTAX *syndef)
{
	MethodFunc pPatternMatch = NULL, pParseExpr = NULL, pStmtTyCheck = NULL, pExprTyCheck = NULL;
	kFunc *mPatternMatch = NULL, *mParseExpr = NULL, *mStmtTyCheck = NULL, *mExprTyCheck = NULL;
	while(syndef->keyword != KW_END) {
		SugarSyntaxVar* syn = (SugarSyntaxVar*)NameSpace_syn(kctx, ns, syndef->keyword, 1/*isnew*/);
		syn->flag  |= ((kshortflag_t)syndef->flag);
		if(syndef->type != 0) {
			syn->ty = syndef->type;
		}
		if(syndef->op1 != NULL) {
			syn->op1 = ksymbolA(syndef->op1, strlen(syndef->op1), SYM_NEWID);
		}
		if(syndef->op2 != NULL) {
			syn->op2 = ksymbolA(syndef->op2, strlen(syndef->op2), SYM_NEWID);
		}
		if(syndef->priority_op2 > 0) {
			syn->priority = syndef->priority_op2;
		}
		if(syndef->rule != NULL) {
			KINITv(syn->syntaxRuleNULL, new_(TokenArray, 0));
			parseSyntaxRule(kctx, syndef->rule, 0, syn->syntaxRuleNULL);
		}
		setSugarFunc(kctx, syndef->PatternMatch, &(syn->PatternMatch), &pPatternMatch, &mPatternMatch);
		setSugarFunc(kctx, syndef->ParseExpr, &(syn->ParseExpr), &pParseExpr, &mParseExpr);
		setSugarFunc(kctx, syndef->TopStmtTyCheck, &(syn->TopStmtTyCheck), &pStmtTyCheck, &mStmtTyCheck);
		setSugarFunc(kctx, syndef->StmtTyCheck, &(syn->StmtTyCheck), &pStmtTyCheck, &mStmtTyCheck);
		setSugarFunc(kctx, syndef->ExprTyCheck, &(syn->ExprTyCheck), &pExprTyCheck, &mExprTyCheck);
		if(syn->ParseExpr == kmodsugar->UndefinedParseExpr) {
			if(FLAG_is(syn->flag, SYNFLAG_ExprOp)) {
				KSETv(syn->ParseExpr, kmodsugar->ParseExpr_Op);
			}
			else if(FLAG_is(syn->flag, SYNFLAG_ExprTerm)) {
				KSETv(syn->ParseExpr, kmodsugar->ParseExpr_Term);
			}
		}
		DBG_ASSERT(syn == SYN_(ns, syndef->keyword));
		syndef++;
	}
}

// --------------------------------------------------------------------------
// ConstTable

static int comprKeyVal(const void *a, const void *b)
{
	int akey = SYMKEY_unbox(((KUtilsKeyValue*)a)->key);
	int bkey = SYMKEY_unbox(((KUtilsKeyValue*)b)->key);
	return akey - bkey;
}

static KUtilsKeyValue* kNameSpace_getConstNULL(KonohaContext *kctx, kNameSpace *ns, ksymbol_t ukey)
{
	size_t min = 0, max = ns->constTable.bytesize / sizeof(KUtilsKeyValue);
	while(min < max) {
		size_t p = (max + min) / 2;
		ksymbol_t key = SYMKEY_unbox(ns->constTable.keyvalueItems[p].key);
		if(key == ukey) return ns->constTable.keyvalueItems + p;
		if(key < ukey) {
			min = p + 1;
		}
		else {
			max = p;
		}
	}
	return NULL;
}

static kbool_t checkConstConflict(KonohaContext *kctx, kNameSpace *ns, KUtilsKeyValue *kvs, kfileline_t pline)
{
	ksymbol_t ukey = kvs->key;
	KUtilsKeyValue* ksval = kNameSpace_getConstNULL(kctx, ns, ukey);
	if(ksval != NULL) {
		if(kvs->ty == ksval->ty && kvs->uval == ksval->uval) {
			return true;  // same value
		}
		kreportf(WarnTag, pline, "conflicted name: %s", SYM_t(SYMKEY_unbox(ukey)));
		return true;
	}
	return false;
}

static void kNameSpace_mergeConstData(KonohaContext *kctx, kNameSpaceVar *ns, KUtilsKeyValue *kvs, size_t nitems, kfileline_t pline)
{
	size_t i, s = ns->constTable.bytesize / sizeof(KUtilsKeyValue);
	if(s == 0) {
		KLIB Karray_init(kctx, &ns->constTable, (nitems + 8) * sizeof(KUtilsKeyValue));
		memcpy(ns->constTable.keyvalueItems, kvs, nitems * sizeof(KUtilsKeyValue));
	}
	else {
		KUtilsWriteBuffer wb;
		KLIB Kwb_init(&(ctxsugar->errorMessageBuffer), &wb);
		for(i = 0; i < nitems; i++) {
			if(checkConstConflict(kctx, ns, kvs+i, pline)) continue;
			KLIB Kwb_write(kctx, &wb, (const char*)(kvs+i), sizeof(KUtilsKeyValue));
		}
		kvs = (KUtilsKeyValue*)KLIB Kwb_top(kctx, &wb, 0);
		nitems = Kwb_bytesize(&wb)/sizeof(KUtilsKeyValue);
		if(nitems > 0) {
			KLIB Karray_resize(kctx, &ns->constTable, (s + nitems + 8) * sizeof(KUtilsKeyValue));
			memcpy(ns->constTable.keyvalueItems + s, kvs, nitems * sizeof(KUtilsKeyValue));
		}
		KLIB Kwb_free(&wb);
	}
	ns->constTable.bytesize = (s + nitems) * sizeof(KUtilsKeyValue);
	PLATAPI qsort_i(ns->constTable.keyvalueItems, s + nitems, sizeof(KUtilsKeyValue), comprKeyVal);
}

static size_t strlen_alnum(const char *p)
{
	size_t len = 0;
	while(isalnum(p[len]) || p[len] == '_') len++;
	return len;
}

static void kNameSpace_loadConstData(KonohaContext *kctx, kNameSpace *ns, const char **d, kfileline_t pline)
{
	INIT_GCSTACK();
	KUtilsKeyValue kv;
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	while(d[0] != NULL) {
		//DBG_P("key='%s'", d[0]);
		kv.key = ksymbolSPOL(d[0], strlen_alnum(d[0]), SPOL_TEXT|SPOL_ASCII, _NEWID) | SYMKEY_BOXED;
		kv.ty  = (ktype_t)(uintptr_t)d[1];
		if(kv.ty == TY_TEXT) {
			kv.ty = TY_String;
			kv.sval = KLIB new_kString(kctx, d[2], strlen(d[2]), SPOL_TEXT);
			PUSH_GCSTACK(kv.oval);
		}
		else if(TY_isUnbox(kv.ty) || kv.ty == TY_TYPE) {
			kv.key = SYMKEY_unbox(kv.key);
			kv.uval = (uintptr_t)d[2];
		}
		else {
			kv.oval = (kObject*)d[2];
		}
		KLIB Kwb_write(kctx, &wb, (const char*)(&kv), sizeof(KUtilsKeyValue));
		d += 3;
	}
	size_t nitems = Kwb_bytesize(&wb) / sizeof(KUtilsKeyValue);
	if(nitems > 0) {
		kNameSpace_mergeConstData(kctx, (kNameSpaceVar*)ns, (KUtilsKeyValue*)KLIB Kwb_top(kctx, &wb, 0), nitems, pline);
	}
	KLIB Kwb_free(&wb);
	RESET_GCSTACK();
}

static void kNameSpace_importClassName(KonohaContext *kctx, kNameSpace *ns, kpackage_t packageId, kfileline_t pline)
{
	KUtilsKeyValue kv;
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	size_t i, size = KARRAYSIZE(kctx->share->classTable.bytesize, uintptr);
	for(i = 0; i < size; i++) {
		KonohaClass *ct = CT_(i);
		if(CT_isPrivate(ct)) continue;
		if(ct->packageId == packageId) {
			DBG_P("importing packageId=%s.%s, %s..", PN_t(ct->packageId), SYM_t(ct->nameid), PN_t(packageId));
			kv.key = ct->nameid;
			kv.ty  = TY_TYPE;
			kv.uval = (uintptr_t)ct;
			KLIB Kwb_write(kctx, &wb, (const char*)(&kv), sizeof(KUtilsKeyValue));
		}
	}
	size_t nitems = Kwb_bytesize(&wb) / sizeof(KUtilsKeyValue);
	if(nitems > 0) {
		kNameSpace_mergeConstData(kctx, (kNameSpaceVar*)ns, (KUtilsKeyValue*)KLIB Kwb_top(kctx, &wb, 0), nitems, pline);
	}
	KLIB Kwb_free(&wb);
}

static KonohaClass *kNameSpace_getClass(KonohaContext *kctx, kNameSpace *ns, KonohaClass *thisct/*NULL*/, const char *name, size_t len, ktype_t def)
{
	KonohaClass *ct = NULL;
	ksymbol_t un = ksymbolA(name, len, SYM_NONAME);
	if(un != SYM_NONAME) {
		uintptr_t hcode = longid(PN_konoha, un);
		ct = (KonohaClass*)map_getu(kctx, kctx->share->longClassNameMapNN, hcode, 0);
		if(ct == NULL) {
			KUtilsKeyValue *kvs = kNameSpace_getConstNULL(kctx, ns, un);
			DBG_P("kvs=%s, %p", name, kvs);
			if(kvs != NULL && kvs->ty == TY_TYPE) {
				return (KonohaClass*)kvs->uval;
			}
		}
	}
	return (ct != NULL) ? ct : ((def >= 0) ? NULL : CT_(def));
}

// --------------------------------------------------------------------------
// Method Management

static kMethod* kMethodList_getMethodNULL(KonohaContext *kctx, kArray *methodList, size_t beginIdx, ktype_t classId, ksymbol_t mn, int option, int policy)
{
	size_t i;
	kMethod *foundMethod = NULL;
	for(i = beginIdx; i < kArray_size(methodList); i++) {
		kMethod *mtd = methodList->methodItems[i];
		if(mtd->mn != mn) continue;
		if(classId != TY_var && mtd->classId != classId) continue;
		if(TFLAG_is(int, policy, MPOL_PARAMSIZE)) {
			kParam *param = Method_param(mtd);
			if(param->psize != option) continue;
		}
		if(TFLAG_is(int, policy, MPOL_SIGNATURE)) {
			if(mtd->paramdom != option) continue;
		}
		if(TFLAG_is(int, policy, MPOL_SETTER)) {
			kParam *param = Method_param(mtd);
			if(param->psize != 1 && param->paramtypeItems[0].ty != (ktype_t)option) continue;
		}
		if(TFLAG_is(int, policy, MPOL_LATEST)) {
			foundMethod = mtd;
			continue;
		}
		return mtd;  // first one;
	}
	return foundMethod;
}

static void kMethodList_findMethodList(KonohaContext *kctx, kArray *methodList, ktype_t classId, ksymbol_t mn, kArray *resultList, int beginIdx)
{
	size_t i;
	for(i = 0; i < kArray_size(methodList); i++) {
		kMethod *mtd = methodList->methodItems[i];
		if(mtd->mn != mn) continue;
		if(classId != TY_var && mtd->classId != classId) continue;
		kMethod *foundMethod = kMethodList_getMethodNULL(kctx, resultList, beginIdx, classId, mn, 0, MPOL_FIRST);
		if(foundMethod == NULL) {
			KLIB kArray_add(kctx, resultList, mtd);
		}
	}
}

static void kNameSpace_findMethodList(KonohaContext *kctx, kNameSpace *ns, ktype_t classId, ksymbol_t mn, kArray *resultList, int beginIdx)
{
	KonohaClass *ct = CT_(classId);
	while(ns != NULL) {
		kMethodList_findMethodList(kctx, ns->methodList, classId, mn, resultList, beginIdx);
		ns = ns->parentNULL;
	}
	while(ct != NULL) {
		kMethodList_findMethodList(kctx, ct->methodList, TY_var, mn, resultList, beginIdx);
		ct = ct->searchSuperMethodClassNULL;
	}
}

static kMethod* KonohaClass_getMethodNULL(KonohaContext *kctx, KonohaClass *ct, kmethodn_t mn, int option, int policy)
{
	while(ct != NULL) {
		kMethod *mtd = kMethodList_getMethodNULL(kctx, ct->methodList, 0, TY_var, mn, option, policy);
		if(mtd != NULL) return mtd;
		ct = ct->searchSuperMethodClassNULL;
	}
	return NULL;
}

static kMethod* kNameSpace_getFirstMethodNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t classId, kmethodn_t mn, int option, int policy)
{
	if(ns != NULL) {
		kMethod *mtd = kNameSpace_getFirstMethodNULL(kctx, ns->parentNULL, classId, mn, option, policy);
		if(mtd != NULL) return mtd;
		mtd = kMethodList_getMethodNULL(kctx, ns->methodList, 0, classId, mn, option, policy);
		return mtd;
	}
	return NULL;
}

static kMethod* kNameSpace_getMethodNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t classId, kmethodn_t mn, int option, int policy)
{
	if(TFLAG_is(int, policy, MPOL_LATEST)) {
		while(ns != NULL) {
			kMethod *mtd = kMethodList_getMethodNULL(kctx, ns->methodList, 0, classId, mn, option, policy);
			if(mtd != NULL) return mtd;
			ns = ns->parentNULL;
		}
		return KonohaClass_getMethodNULL(kctx, CT_(classId), mn, option, policy);
	}
	else {
		kMethod *mtd = KonohaClass_getMethodNULL(kctx, CT_(classId), mn, option, policy);
		if(mtd != NULL) return mtd;
		return kNameSpace_getFirstMethodNULL(kctx, ns, classId, mn, option, policy);
	}
}

static kMethod* kNameSpace_addMethod(KonohaContext *kctx, kNameSpace *ns, kMethod *mtd)
{
	KonohaClass *ct = CT_(mtd->classId);
	if(mtd->packageId == 0 && ns != NULL) {
		((kMethodVar*)mtd)->packageId = ns->packageId;
	}
	DBG_P("loading method %s.%s%s: @Public=%d", Method_t(mtd), Method_isPublic(mtd), mtd->flag);
	if(Method_isPublic(mtd) /* && ct->packageDomain == ns->packageDomain*/) {
		kMethod *foundMethod = KonohaClass_getMethodNULL(kctx, ct, mtd->mn, mtd->paramdom, MPOL_FIRST|MPOL_SIGNATURE);
		if(foundMethod != NULL) {  // same signature
			if(foundMethod->classId == mtd->classId) {
				DBG_P("duplicated method %s.%s%s", Method_t(foundMethod));
				PUSH_GCSTACK(mtd);  // avoid memory leaking
				return foundMethod;
			}
		}
		else {
			foundMethod = KonohaClass_getMethodNULL(kctx, ct, mtd->mn, mtd->paramdom, MPOL_FIRST);
			if(foundMethod != NULL && Method_paramsize(mtd) == Method_paramsize(foundMethod)) {
				DBG_P("set overloading method %s.%s%s", Method_t(foundMethod));
				Method_setOverloaded(foundMethod, true);
				Method_setOverloaded(mtd, true);
			}
		}
		if(unlikely(ct->methodList == K_EMPTYARRAY)) {
			KINITv(((KonohaClassVar*)ct)->methodList, new_(MethodArray, 8));
		}
		KLIB kArray_add(kctx, ct->methodList, mtd);
	}
	else {
		if(ns->methodList == K_EMPTYARRAY) {
			KINITv(((kNameSpaceVar*)ns)->methodList, new_(MethodArray, 8));
		}
		KLIB kArray_add(kctx, ns->methodList, mtd);
	}
	return NULL;
}

static void kNameSpace_loadMethodData(KonohaContext *kctx, kNameSpace *ns, intptr_t *data)
{
	intptr_t *d = data;
	while(d[0] != -1) {
		uintptr_t flag = (uintptr_t)d[0];
		MethodFunc f = (MethodFunc)d[1];
		ktype_t rtype = (ktype_t)d[2];
		ktype_t cid  = (ktype_t)d[3];
		kmethodn_t mn = (kmethodn_t)d[4];
		size_t i, psize = (size_t)d[5];
		kparamtype_t p[psize+1];
		d = d + 6;
		for(i = 0; i < psize; i++) {
			p[i].ty = (ktype_t)d[0];
			p[i].fn = (ksymbol_t)d[1];
			d += 2;
		}
		kMethod *mtd = KLIB new_kMethod(kctx, flag, cid, mn, f);
		KLIB Method_setParam(kctx, mtd, rtype, psize, p);
		kNameSpace_addMethod(kctx, ns, mtd);
	}
}

static kMethod* kNameSpace_getCastMethodNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t cid, ktype_t tcid)
{
	kMethod *mtd = kNameSpace_getMethodNULL(kctx, ns, cid, MN_to(tcid), 0, MPOL_PARAMSIZE|MPOL_FIRST);
	if(mtd == NULL) {
		mtd = kNameSpace_getMethodNULL(kctx, ns, cid, MN_as(tcid), 0, MPOL_PARAMSIZE|MPOL_FIRST);
	}
	return mtd;
}

