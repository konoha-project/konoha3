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


#define PACKSUGAR    .packageId = 1, .packageDomain = 1

/* --------------- */
/* NameSpace */

static void NameSpace_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kNameSpaceVar *ns = (kNameSpaceVar*)o;
	bzero(&ns->parentNULL, sizeof(kNameSpace) - sizeof(KonohaObjectHeader));
	ns->parentNULL = conf;
	KINITv(ns->methodList, K_EMPTYARRAY);
	KINITv(ns->scriptObject, knull(CT_System));
}

static void syntax_reftrace(KonohaContext *kctx, KUtilsHashMapEntry *p)
{
	SugarSyntax *syn = (SugarSyntax*)p->uvalue;
	BEGIN_REFTRACE(6);
	KREFTRACEn(syn->syntaxRuleNULL);
	KREFTRACEv(syn->PatternMatch);
	KREFTRACEv(syn->ParseExpr);
	KREFTRACEv(syn->TopStmtTyCheck);
	KREFTRACEv(syn->StmtTyCheck);
	KREFTRACEv(syn->ExprTyCheck);
	END_REFTRACE();
}

static void NameSpace_reftrace(KonohaContext *kctx, kObject *o)
{
	kNameSpace *ns = (kNameSpace*)o;
	if(ns->syntaxMapNN != NULL) {
		KLIB Kmap_reftrace(kctx, ns->syntaxMapNN, syntax_reftrace);
	}
	size_t i, size = KARRAYSIZE(ns->constTable.bytesize, kvs);
	BEGIN_REFTRACE(size);
	for(i = 0; i < size; i++) {
		if(SYMKEY_isBOXED(ns->constTable.kvs[i].key)) {
			KREFTRACEv(ns->constTable.kvs[i].oval);
		}
	}
	KREFTRACEn(ns->parentNULL);
	KREFTRACEv(ns->scriptObject);
	KREFTRACEv(ns->methodList);
	END_REFTRACE();
}

static void syntax_free(KonohaContext *kctx, void *p)
{
	KFREE(p, sizeof(SugarSyntax));
}

static void NameSpace_free(KonohaContext *kctx, kObject *o)
{
	kNameSpaceVar *ns = (kNameSpaceVar*)o;
	if(ns->syntaxMapNN != NULL) {
		KLIB Kmap_free(kctx, ns->syntaxMapNN, syntax_free);
	}
	if(ns->tokenMatrix != NULL) {
		KFREE((void*)ns->tokenMatrix, SIZEOF_TOKENMATRIX);
	}
	KLIB Karray_free(kctx, &ns->constTable);
}

// syntax
static void checkFuncArray(KonohaContext *kctx, kFunc **synp);
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
		KUtilsHashMapEntry *e = KLIB Kmap_newentry(kctx, ns0->syntaxMapNN, hcode);
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

static void checkFuncArray(KonohaContext *kctx, kFunc **synp)
{
	if(synp[0] != NULL && IS_Array(synp[0])) {
		size_t i;
		kArray *newa = new_(Array, 8), *a = (kArray*)synp[0];
		for(i = 0; i < kArray_size(a); i++) {
			kArray_add(newa, a->list[i]);
		}
		KSETv(synp[0], (kFunc*)newa);
	}
}

static void SYN_setSugarFunc(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, size_t idx, kFunc *fo)
{
	SugarSyntaxVar *syn = (SugarSyntaxVar *)NameSpace_syn(kctx, ns, keyword, 1/*new*/);
	kFunc **synp = &(syn->PatternMatch);
	DBG_ASSERT(idx <= SYNIDX_ExprTyCheck);
	KSETv(synp[idx], fo);
}

static void SYN_addSugarFunc(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, size_t idx, kFunc *fo)
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
		kArray_add(a, synp[idx]);
		KSETv(synp[idx], (kFunc*)a);
	}
	kArray_add(a, fo);
}

static void setSugarFunc(KonohaContext *kctx, MethodFunc f, kFunc **synp, MethodFunc *p, kFunc **mp)
{
	if(f != NULL) {
		if(f != p[0]) {
			p[0] = f;
			mp[0] = new_SugarFunc(f);
		}
		KINITv(synp[0], mp[0]);
	}
}

static void NameSpace_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KDEFINE_SYNTAX *syndef)
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
	//DBG_P("syntax size=%d, hmax=%d", ks->syntaxMapNN->size, ks->syntaxMapNN->hmax);
}

#define T_statement(kw)  KW_tSTMT_(kctx, kw), KW_tSTMTPOST(kw)

static const char* KW_tSTMT_(KonohaContext *kctx, ksymbol_t keyword)
{
	const char *statement = SYM_t(keyword);
	if(keyword == KW_ExprPattern) statement = "expression";
	else if(keyword == KW_StmtTypeDecl) statement = "variable";
	else if(keyword == KW_StmtMethodDecl) statement =  "function";
	return statement;
}

static const char* KW_tSTMTPOST(ksymbol_t keyword)
{
	const char *postfix = " statement";
	if(keyword == KW_ExprPattern) postfix = "";
	else if(keyword == KW_StmtTypeDecl || keyword == KW_StmtMethodDecl) postfix = " declaration";
	return postfix;
}

// USymbolTable
static int comprKeyVal(const void *a, const void *b)
{
	int akey = SYMKEY_unbox(((KUtilsKeyValue*)a)->key);
	int bkey = SYMKEY_unbox(((KUtilsKeyValue*)b)->key);
	return akey - bkey;
}

static KUtilsKeyValue* NameSpace_getConstNULL(KonohaContext *kctx, kNameSpace *ns, ksymbol_t ukey)
{
	size_t min = 0, max = KARRAYSIZE(ns->constTable.bytesize, kvs);
	while(min < max) {
		size_t p = (max + min) / 2;
		ksymbol_t key = SYMKEY_unbox(ns->constTable.kvs[p].key);
		if(key == ukey) return ns->constTable.kvs + p;
		if(key < ukey) {
			min = p + 1;
		}
		else {
			max = p;
		}
	}
	return NULL;
}

static kbool_t checkConflictedConst(KonohaContext *kctx, kNameSpace *ns, KUtilsKeyValue *kvs, kfileline_t pline)
{
	ksymbol_t ukey = kvs->key;
	KUtilsKeyValue* ksval = NameSpace_getConstNULL(kctx, ns, ukey);
	if(ksval != NULL) {
		if(kvs->ty == ksval->ty && kvs->uval == ksval->uval) {
			return true;  // same value
		}
		kreportf(WARN_, pline, "conflicted name: %s", SYM_t(SYMKEY_unbox(ukey)));
		return true;
	}
	return false;
}

static void NameSpace_mergeConstData(KonohaContext *kctx, kNameSpaceVar *ns, KUtilsKeyValue *kvs, size_t nitems, kfileline_t pline)
{
	size_t i, s = KARRAYSIZE(ns->constTable.bytesize, kvs);
	if(s == 0) {
		KLIB Karray_init(kctx, &ns->constTable, (nitems + 8) * sizeof(KUtilsKeyValue));
		memcpy(ns->constTable.kvs, kvs, nitems * sizeof(KUtilsKeyValue));
	}
	else {
		KUtilsWriteBuffer wb;
		KLIB Kwb_init(&(ctxsugar->errorMessageBuffer), &wb);
		for(i = 0; i < nitems; i++) {
			if(checkConflictedConst(kctx, ns, kvs+i, pline)) continue;
			KLIB Kwb_write(kctx, &wb, (const char*)(kvs+i), sizeof(KUtilsKeyValue));
		}
		kvs = (KUtilsKeyValue*)KLIB Kwb_top(kctx, &wb, 0);
		nitems = Kwb_bytesize(&wb)/sizeof(KUtilsKeyValue);
		if(nitems > 0) {
			KLIB Karray_resize(kctx, &ns->constTable, (s + nitems + 8) * sizeof(KUtilsKeyValue));
			memcpy(ns->constTable.kvs + s, kvs, nitems * sizeof(KUtilsKeyValue));
		}
		KLIB Kwb_free(&wb);
	}
	ns->constTable.bytesize = (s + nitems) * sizeof(KUtilsKeyValue);
	PLAT qsort_i(ns->constTable.kvs, s + nitems, sizeof(KUtilsKeyValue), comprKeyVal);
}

static size_t strlen_alnum(const char *p)
{
	size_t len = 0;
	while(isalnum(p[len]) || p[len] == '_') len++;
	return len;
}

static void NameSpace_loadConstData(KonohaContext *kctx, kNameSpace *ns, const char **d, kfileline_t pline)
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
			kv.sval = new_kString(d[2], strlen(d[2]), SPOL_TEXT);
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
		NameSpace_mergeConstData(kctx, (kNameSpaceVar*)ns, (KUtilsKeyValue*)KLIB Kwb_top(kctx, &wb, 0), nitems, pline);
	}
	KLIB Kwb_free(&wb);
	RESET_GCSTACK();
}

static void NameSpace_importClassName(KonohaContext *kctx, kNameSpace *ns, kpackage_t packageId, kfileline_t pline)
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
		NameSpace_mergeConstData(kctx, (kNameSpaceVar*)ns, (KUtilsKeyValue*)KLIB Kwb_top(kctx, &wb, 0), nitems, pline);
	}
	KLIB Kwb_free(&wb);
}

// NameSpace

static KonohaClass *NameSpace_getCT(KonohaContext *kctx, kNameSpace *ns, KonohaClass *thisct/*NULL*/, const char *name, size_t len, ktype_t def)
{
	KonohaClass *ct = NULL;
	ksymbol_t un = ksymbolA(name, len, SYM_NONAME);
	if(un != SYM_NONAME) {
		uintptr_t hcode = longid(PN_konoha, un);
		ct = (KonohaClass*)map_getu(kctx, kctx->share->longClassNameMapNN, hcode, 0);
		if(ct == NULL) {
			KUtilsKeyValue *kvs = NameSpace_getConstNULL(kctx, ns, un);
			DBG_P("kvs=%s, %p", name, kvs);
			if(kvs != NULL && kvs->ty == TY_TYPE) {
				return (KonohaClass*)kvs->uval;
			}
		}
	}
	return (ct != NULL) ? ct : ((def >= 0) ? NULL : CT_(def));
}

static void CT_addMethod(KonohaContext *kctx, KonohaClass *ct, kMethod *mtd)
{
	if(unlikely(ct->methodList == K_EMPTYARRAY)) {
		KINITv(((KonohaClassVar*)ct)->methodList, new_(MethodArray, 8));
	}
	kArray_add(ct->methodList, mtd);
}

static void NameSpace_addMethod(KonohaContext *kctx, kNameSpace *ns, kMethod *mtd)
{
	if(ns->methodList == K_EMPTYARRAY) {
		KINITv(((kNameSpaceVar*)ns)->methodList, new_(MethodArray, 8));
	}
	kArray_add(ns->methodList, mtd);
}

/* NameSpace/Class/Method */
static kMethod* CT_findMethodNULL(KonohaContext *kctx, KonohaClass *ct, kmethodn_t mn)
{
	while(ct != NULL) {
		size_t i;
		kArray *a = ct->methodList;
		for(i = 0; i < kArray_size(a); i++) {
			kMethod *mtd = a->methodList[i];
			if((mtd)->mn == mn) {
				return mtd;
			}
		}
		ct = ct->searchSuperMethodClassNULL;
	}
	return NULL;
}

#define kNameSpace_getStaticMethodNULL(ns, mn)   NameSpace_getStaticMethodNULL(kctx, ns, mn)

static kMethod* NameSpace_getMethodNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t cid, kmethodn_t mn)
{
	while(ns != NULL) {
		size_t i;
		kArray *a = ns->methodList;
		for(i = 0; i < kArray_size(a); i++) {
			kMethod *mtd = a->methodList[i];
			if(mtd->cid == cid && mtd->mn == mn) {
				return mtd;
			}
		}
		ns = ns->parentNULL;
	}
	return CT_findMethodNULL(kctx, CT_(cid), mn);
}

//static kMethod* NameSpace_getStaticMethodNULL(KonohaContext *kctx, kNameSpace *ns, kmethodn_t mn)
//{
//	while(ks != NULL) {
//		kMethod *mtd = kNameSpace_getMethodNULL(ns, O_cid(ks->scriptObject), mn);
//		if(mtd != NULL && kMethod_isStatic(mtd)) {
//			return mtd;
//		}
//		if(ks->static_cid != TY_unknown) {
//			kMethod *mtd = kNameSpace_getMethodNULL(ns, ks->static_cid, mn);
//			if(mtd != NULL && kMethod_isStatic(mtd)) {
//				return mtd;
//			}
//		}
//		ks = ks->parentNULL;
//	}
//	return NULL;
//}

#define kNameSpace_getCastMethodNULL(ns, cid, tcid)     NameSpace_getCastMethodNULL(kctx, ns, cid, tcid)
static kMethod* NameSpace_getCastMethodNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t cid, ktype_t tcid)
{
	kMethod *mtd = NameSpace_getMethodNULL(kctx, ns, cid, MN_to(tcid));
	if(mtd == NULL) {
		mtd = NameSpace_getMethodNULL(kctx, ns, cid, MN_as(tcid));
	}
	return mtd;
}

#define kNameSpace_defineMethod(NS,MTD,UL)  NameSpace_defineMethod(kctx, NS, MTD, UL)

static kbool_t NameSpace_defineMethod(KonohaContext *kctx, kNameSpace *ns, kMethod *mtd, kfileline_t pline)
{
	//if(pline != 0) {
	//	kMethod *mtdOLD = NameSpace_getMethodNULL(kctx, ks, mtd->cid, mtd->mn);
	//	if(mtdOLD != NULL) {
	//		char mbuf[128];
	//		kreportf(ERR_, pline, "method %s.%s is already defined", TY_t(mtd->cid), T_mn(mbuf, mtd->mn));
	//		return 0;
	//	}
	//}
	if(mtd->packageId == 0) {
		((kMethodVar*)mtd)->packageId = ns->packageId;
	}
	KonohaClass *ct = CT_(mtd->cid);
	if(ct->packageDomain == ns->packageDomain && kMethod_isPublic(mtd)) {
		CT_addMethod(kctx, ct, mtd);
	}
	else {
		NameSpace_addMethod(kctx, ns, mtd);
	}
	return 1;
}

static void NameSpace_loadMethodData(KonohaContext *kctx, kNameSpace *ns, intptr_t *data)
{
	intptr_t *d = data;
	while(d[0] != -1) {
		uintptr_t flag = (uintptr_t)d[0];
		MethodFunc f = (MethodFunc)d[1];
		ktype_t rtype = (ktype_t)d[2];
		ktype_t cid  = (ktype_t)d[3];
		kmethodn_t mn = (kmethodn_t)d[4];
		size_t i, psize = (size_t)d[5];
		kparam_t p[psize+1];
		d = d + 6;
		for(i = 0; i < psize; i++) {
			p[i].ty = (ktype_t)d[0];
			p[i].fn = (ksymbol_t)d[1];
			d += 2;
		}
		kMethod *mtd = new_kMethod(flag, cid, mn, f);
		kMethod_setParam(mtd, rtype, psize, p);
		if(ns == NULL || kMethod_isPublic(mtd)) {
			CT_addMethod(kctx, CT_(cid), mtd);
		} else {
			NameSpace_addMethod(kctx, ns, mtd);
		}
	}
}

//#define kNameSpace_loadGlueFunc(NS, F, OPT, UL)  NameSpace_loadGlueFunc(kctx, NS, F, OPT, UL)
//
//static MethodFunc NameSpace_loadGlueFunc(KonohaContext *kctx, kNameSpace *ns, const char *funcname, int DOPTION, kfileline_t pline)
//{
//	void *f = NULL;
//	if(ks->gluehdr != NULL) {
//		char namebuf[128];
//		snprintf(namebuf, sizeof(namebuf), "D%s", funcname);
//		if(DOPTION) {
//			f = dlsym(ks->gluehdr, (const char*)namebuf);
//		}
//		if(f == NULL) {
//			f = dlsym(ks->gluehdr, (const char*)namebuf+1);
//		}
//		kreportf(WARN_, pline, "glue method function is not found: %s", namebuf + 1);
//	}
//	return f;
//}

/* --------------- */
/* Token */

static void Token_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kTokenVar *tk = (kTokenVar*)o;
	tk->uline     =   0;
	tk->keyword        =   (ksymbol_t)(intptr_t)conf;
	KINITv(tk->text, TS_EMPTY);
}

static void Token_reftrace(KonohaContext *kctx, kObject *o)
{
	kToken *tk = (kToken*)o;
	BEGIN_REFTRACE(1);
	KREFTRACEv(tk->text);
	END_REFTRACE();
}

//static const char *T_tt(ksymbol_t t)
//{
//	int tt = SYM_UNMASK(t);
//	static const char* symTKDATA[] = {
//		"TK_NONE",
//		"TK_INDENT",
//		"TK_SYMBOL",
//		"TK_USYMBOL",
//		"TK_TEXT",
//		"TK_INT",
//		"TK_FLOAT",
//		"TK_TYPE",
//		"AST_()",
//		"AST_[]",
//		"AST_{}",
//
//		"TK_OPERATOR",
//		"TK_MSYMBOL",
//		"TK_ERR",
//		"TK_CODE",
//		"TK_WHITESPACE",
//		"TK_METANAME",
//		"TK_MN",
//		"AST_OPTIONAL[]",
//	};
//	if(tt <= AST_OPTIONAL) {
//		return symTKDATA[tt];
//	}
//	return "TK_UNKNOWN";
//}

static void dumpToken(KonohaContext *kctx, kToken *tk)
{
	if(verbose_sugar) {
		DUMP_P("%s%s %d: kw=%s%s '%s'\n", KW_t(tk->keyword), (short)tk->uline, KW_t(tk->keyword), kToken_s(tk));
	}
}

static void dumpIndent(KonohaContext *kctx, int nest)
{
	int i;
	for(i = 0; i < nest; i++) {
		DUMP_P("  ");
	}
}

static int kTokenList_beginChar(kToken *tk)
{
	switch(tk->keyword) {
	case AST_PARENTHESIS: return '(';
	case AST_BRACE: return '{';
	case AST_BRACKET: return '[';
	}
	return '<';
}

static int kTokenList_endChar(kToken *tk)
{
	switch(tk->keyword) {
	case AST_PARENTHESIS: return ')';
	case AST_BRACE: return '}';
	case AST_BRACKET: return ']';
	}
	return '>';
}

static void dumpTokenArray(KonohaContext *kctx, int nest, kArray *a, int s, int e)
{
	if(verbose_sugar) {
		if(nest == 0) DUMP_P("\n");
		while(s < e) {
			kToken *tk = a->toks[s];
			dumpIndent(kctx, nest);
			if(IS_Array(tk->sub)) {
				DUMP_P("%c\n", kTokenList_beginChar(tk));
				dumpTokenArray(kctx, nest+1, tk->sub, 0, kArray_size(tk->sub));
				dumpIndent(kctx, nest);
				DUMP_P("%c\n", kTokenList_endChar(tk));
			}
			else {
				DUMP_P("TK(%d) ", s);
				dumpToken(kctx, tk);
			}
			s++;
		}
		if(nest == 0) DUMP_P("====\n");
	}
}

/* --------------- */
/* Expr */

static void Expr_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kExprVar *expr      =   (kExprVar*)o;
	expr->build      =   TEXPR_UNTYPED;
	expr->ty         =   TY_var;
	KINITv(expr->tk, K_NULLTOKEN);
	KINITv(expr->data, K_NULL);
	expr->syn = (SugarSyntax*)conf;
}

static void Expr_reftrace(KonohaContext *kctx, kObject *o)
{
	kExpr *expr = (kExpr*)o;
	BEGIN_REFTRACE(2);
	KREFTRACEv(expr->tk);
	KREFTRACEv(expr->data);
	END_REFTRACE();
}

static kExprVar* Expr_vadd(KonohaContext *kctx, kExprVar *expr, int n, va_list ap)
{
	int i;
	if(!IS_Array(expr->cons)) {
		KSETv(expr->cons, new_(Array, 8));
	}
	for(i = 0; i < n; i++) {
		kObject *v =  (kObject*)va_arg(ap, kObject*);
		if(v == NULL || v == (kObject*)K_NULLEXPR) {
			return (kExprVar*)K_NULLEXPR;
		}
		kArray_add(expr->cons, v);
	}
	return expr;
}

static kExpr* new_ConsExpr(KonohaContext *kctx, SugarSyntax *syn, int n, ...)
{
	va_list ap;
	va_start(ap, n);
	DBG_ASSERT(syn != NULL);
	kExprVar *expr = new_Var(Expr, syn);
	PUSH_GCSTACK(expr);
	expr = Expr_vadd(kctx, expr, n, ap);
	va_end(ap);
	return (kExpr*)expr;
}

static kExpr* new_TypedConsExpr(KonohaContext *kctx, int build, ktype_t ty, int n, ...)
{
	va_list ap;
	va_start(ap, n);
	kExprVar *expr = new_Var(Expr, NULL);
	PUSH_GCSTACK(expr);
	expr = Expr_vadd(kctx, expr, n, ap);
	va_end(ap);
	expr->build = build;
	expr->ty = ty;
	return (kExpr*)expr;
}

static kExpr *Expr_tyCheckCallParams(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kMethod *mtd, kGamma *gma, ktype_t reqty);

static kExpr* new_TypedMethodCall(KonohaContext *kctx, kStmt *stmt, ktype_t ty, kMethod *mtd, kGamma *gma, int n, ...)
{
	va_list ap;
	va_start(ap, n);
	kExprVar *expr = new_Var(Expr, NULL);
	PUSH_GCSTACK(expr);
	KSETv(expr->cons, new_(Array, 8));
	kArray_add(expr->cons, mtd);
	expr = Expr_vadd(kctx, expr, n, ap);
	va_end(ap);
	expr->build = TEXPR_CALL;
	expr->ty = ty;
	return Expr_tyCheckCallParams(kctx, stmt, (kExpr*)expr, mtd, gma, ty);
}


static kExpr* Expr_add(KonohaContext *kctx, kExpr *expr, kExpr *e)
{
	DBG_ASSERT(IS_Array(expr->cons));
	if(expr != K_NULLEXPR && e != NULL && e != K_NULLEXPR) {
		kArray_add(expr->cons, e);
		return expr;
	}
	return K_NULLEXPR;
}

static void dumpExpr(KonohaContext *kctx, int n, int nest, kExpr *expr)
{
	if(verbose_sugar) {
		if(nest == 0) DUMP_P("\n");
		dumpIndent(kctx, nest);
		if(expr == K_NULLEXPR) {
			DUMP_P("[%d] ExprTerm: null", n);
		}
		else if(Expr_isTerm(expr)) {
			DUMP_P("[%d] ExprTerm: kw='%s%s' %s", n, KW_t(expr->tk->keyword), kToken_s(expr->tk));
			if(expr->ty != TY_var) {

			}
			DUMP_P("\n");
		}
		else {
			int i;
			if(expr->syn == NULL) {
				DUMP_P("[%d] Cons: kw=NULL, size=%ld", n, kArray_size(expr->cons));
			}
			else {
				DUMP_P("[%d] Cons: kw='%s%s', size=%ld", n, KW_t(expr->syn->keyword), kArray_size(expr->cons));
			}
			if(expr->ty != TY_var) {

			}
			DUMP_P("\n");
			for(i=0; i < kArray_size(expr->cons); i++) {
				kObject *o = expr->cons->list[i];
				if(O_ct(o) == CT_Expr) {
					dumpExpr(kctx, i, nest+1, (kExpr*)o);
				}
				else {
					dumpIndent(kctx, nest+1);
					if(O_ct(o) == CT_Token) {
						kToken *tk = (kToken*)o;
						DUMP_P("[%d] O: %s ", i, CT_t(o->h.ct));
						dumpToken(kctx, tk);
					}
					else if(o == K_NULL) {
						DUMP_P("[%d] O: null\n", i);
					}
					else {
						DUMP_P("[%d] O: %s\n", i, CT_t(o->h.ct));
					}
				}
			}
		}
	}
}

static kExpr* Expr_setConstValue(KonohaContext *kctx, kExpr *expr, ktype_t ty, kObject *o)
{
	if(expr == NULL) {
		expr = new_(Expr, 0);
		PUSH_GCSTACK(expr);
	}
	kExprVar *Wexpr = (kExprVar*)expr;
	Wexpr->ty = ty;
	if(TY_isUnbox(ty)) {
		Wexpr->build = TEXPR_NCONST;
		Wexpr->ndata = N_toint(o);
		KSETv(Wexpr->data, K_NULL);
	}
	else {
		Wexpr->build = TEXPR_CONST;
		KINITv(Wexpr->data, o);
	}
	return expr;
}

static kExpr* Expr_setNConstValue(KonohaContext *kctx, kExpr *expr, ktype_t ty, uintptr_t ndata)
{
	if(expr == NULL) {
		expr = new_(Expr, 0);
		PUSH_GCSTACK(expr);
	}
	kExprVar *Wexpr = (kExprVar*)expr;
	Wexpr->build = TEXPR_NCONST;
	Wexpr->ndata = ndata;
	KSETv(Wexpr->data, K_NULL);
	Wexpr->ty = ty;
	return expr;
}

static kExpr *Expr_setVariable(KonohaContext *kctx, kExpr *expr, int build, ktype_t ty, intptr_t index, kGamma *gma)
{
	if(expr == NULL) {
		expr = new_(Expr, 0);
		PUSH_GCSTACK(expr);
	}
	kExprVar *Wexpr = (kExprVar*)expr;
	Wexpr->build = build;
	Wexpr->ty = ty;
	Wexpr->index = index;
	// KSETv(Wexpr->data, K_NULL);  block need unclear
	if(build < TEXPR_UNTYPED) {
		kArray_add(gma->genv->lvarlst, Wexpr);
	}
	return expr;
}

/* --------------- */
/* Stmt */

static void Stmt_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kStmtVar *stmt = (kStmtVar*)o;
	stmt->uline    = (kfileline_t)conf;
	stmt->syn      = NULL;
	stmt->parentBlockNULL = NULL;
}

static void Stmt_reftrace(KonohaContext *kctx, kObject *o)
{
	kStmt *stmt = (kStmt*)o;
	BEGIN_REFTRACE(1);
	KREFTRACEn(stmt->parentBlockNULL);
	END_REFTRACE();
}

static void _dumpToken(KonohaContext *kctx, void *arg, KUtilsKeyValue *d)
{
	if((d->key & SYMKEY_BOXED) == SYMKEY_BOXED) {
		ksymbol_t key = ~SYMKEY_BOXED & d->key;
		DUMP_P("key='%s%s': ", KW_t(key));
		if(IS_Token(d->oval)) {
			dumpToken(kctx, (kToken*)d->oval);
		} else if (IS_Expr(d->oval)) {
			dumpExpr(kctx, 0, 0, (kExpr *) d->oval);
		}
	}
}

static void dumpStmt(KonohaContext *kctx, kStmt *stmt)
{
	if(verbose_sugar) {
		if(stmt->syn == NULL) {
			DUMP_P("STMT (DONE)\n");
		}
		else {
			DUMP_P("STMT %s%s {\n", T_statement(stmt->syn->keyword));
			kObject_protoEach(stmt, NULL, _dumpToken);
			DUMP_P("\n}\n");
		}
	}
}

#define AKEY(T)   T, (sizeof(T)-1)

typedef struct flagop_t {
	const char *key;
	size_t keysize;
	uintptr_t flag;
} flagop_t ;

static uintptr_t Stmt_flag(KonohaContext *kctx, kStmt *stmt, flagop_t *fop, uintptr_t flag)
{
	while(fop->key != NULL) {
		ksymbol_t kw = ksymbolA(fop->key, fop->keysize, SYM_NONAME);
		if(kw != SYM_NONAME) {
			kObject *op = kObject_getObjectNULL(stmt, kw);
			if(op != NULL) {
				flag |= fop->flag;
			}
		}
		fop++;
	}
	return flag;
}

#define kStmt_is(STMT, KW) Stmt_is(kctx, STMT, KW)

static inline kbool_t Stmt_is(KonohaContext *kctx, kStmt *stmt, ksymbol_t kw)
{
	return (kObject_getObjectNULL(stmt, kw) != NULL);
}

static kToken* Stmt_token(KonohaContext *kctx, kStmt *stmt, ksymbol_t kw, kToken *def)
{
	kToken *tk = (kToken*)kObject_getObjectNULL(stmt, kw);
	if(tk != NULL && IS_Token(tk)) {
		return tk;
	}
	return def;
}

static kExpr* Stmt_expr(KonohaContext *kctx, kStmt *stmt, ksymbol_t kw, kExpr *def)
{
	kExpr *expr = (kExpr*)kObject_getObjectNULL(stmt, kw);
	if(expr != NULL && IS_Expr(expr)) {
		return expr;
	}
	return def;
}

static const char* Stmt_text(KonohaContext *kctx, kStmt *stmt, ksymbol_t kw, const char *def)
{
	kExpr *expr = (kExpr*)kObject_getObjectNULL(stmt, kw);
	if(expr != NULL) {
		if(IS_Expr(expr) && Expr_isTerm(expr)) {
			return S_text(expr->tk->text);
		}
		else if(IS_Token(expr)) {
			kToken *tk = (kToken*)expr;
			if(IS_String(tk->text)) return S_text(tk->text);
		}
	}
	return def;
}

static kbool_t Token_toBRACE(KonohaContext *kctx, kTokenVar *tk, kNameSpace *ns);
static kBlock *new_Block(KonohaContext *kctx, kNameSpace* ns, kStmt *stmt, kArray *tls, int s, int e, int delim);
static kBlock* Stmt_block(KonohaContext *kctx, kStmt *stmt, ksymbol_t kw, kBlock *def)
{
	kBlock *bk = (kBlock*)kObject_getObjectNULL(stmt, kw);
	if(bk != NULL) {
		if(IS_Token(bk)) {
			kToken *tk = (kToken*)bk;
			if (tk->keyword == TK_CODE) {
				Token_toBRACE(kctx, (kTokenVar*)tk, kStmt_nameSpace(stmt));
			}
			if (tk->keyword == AST_BRACE) {
				bk = new_Block(kctx, kStmt_nameSpace(stmt), stmt, tk->sub, 0, kArray_size(tk->sub), ';');
				kObject_setObject(stmt, kw, bk);
			}
		}
		if(IS_Block(bk)) return bk;
	}
	return def;
}

/* --------------- */
/* Block */

static void Block_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kBlockVar *bk = (kBlockVar*)o;
	kNameSpace *ns = (conf != NULL) ? (kNameSpace*)conf : KNULL(NameSpace);
	bk->parentStmtNULL = NULL;
	KINITv(bk->blockNameSpace, ns);
	KINITv(bk->stmtList, new_(StmtArray, 0));
	KINITv(bk->esp, new_(Expr, 0));
}

static void Block_reftrace(KonohaContext *kctx, kObject *o)
{
	kBlock *bk = (kBlock*)o;
	BEGIN_REFTRACE(4);
	KREFTRACEv(bk->blockNameSpace);
	KREFTRACEv(bk->stmtList);
	KREFTRACEv(bk->esp);
	KREFTRACEn(bk->parentStmtNULL);
	END_REFTRACE();
}

static void Block_insertAfter(KonohaContext *kctx, kBlock *bk, kStmt *target, kStmt *stmt)
{
	//DBG_ASSERT(stmt->parentNULL == NULL);
	KSETv(((kStmtVar*)stmt)->parentBlockNULL, bk);
	size_t i;
	for(i = 0; i < kArray_size(bk->stmtList); i++) {
		if(bk->stmtList->stmts[i] == target) {
			kArray_insert(bk->stmtList, i+1, stmt);
			return;
		}
	}
	DBG_ABORT("target was not found!!");
}

/* --------------- */
/* Block */

static void Gamma_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kGammaVar *gma = (kGammaVar*)o;
	gma->genv = NULL;
}

