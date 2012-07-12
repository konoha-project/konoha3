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


#define PACKSUGAR    .packid = 1, .packdom = 1

/* --------------- */
/* NameSpace */

static void NameSpace_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct _kNameSpace *ks = (struct _kNameSpace*)o;
	bzero(&ks->parentNULL, sizeof(kNameSpace) - sizeof(KonohaObjectHeader));
	ks->parentNULL = conf;
	KINITv(ks->methods, K_EMPTYARRAY);
	KINITv(ks->scrobj, knull(CT_System));
}

static void syntax_reftrace(KonohaContext *kctx, kmape_t *p)
{
	ksyntax_t *syn = (ksyntax_t*)p->uvalue;
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
	kNameSpace *ks = (kNameSpace*)o;
	if(ks->syntaxMapNN != NULL) {
		kmap_reftrace(ks->syntaxMapNN, syntax_reftrace);
	}
	size_t i, size = KARRAYSIZE(ks->cl.bytesize, kvs);
	BEGIN_REFTRACE(size);
	for(i = 0; i < size; i++) {
		if(SYMKEY_isBOXED(ks->cl.kvs[i].key)) {
			KREFTRACEv(ks->cl.kvs[i].oval);
		}
	}
	KREFTRACEn(ks->parentNULL);
	KREFTRACEv(ks->scrobj);
	KREFTRACEv(ks->methods);
	END_REFTRACE();
}

static void syntax_free(KonohaContext *kctx, void *p)
{
	KFREE(p, sizeof(ksyntax_t));
}

static void NameSpace_free(KonohaContext *kctx, kObject *o)
{
	struct _kNameSpace *ks = (struct _kNameSpace*)o;
	if(ks->syntaxMapNN != NULL) {
		kmap_free(ks->syntaxMapNN, syntax_free);
	}
	if(ks->tokenMatrix != NULL) {
		KFREE((void*)ks->tokenMatrix, SIZEOF_TOKENMATRIX);
	}
	KARRAY_FREE(&ks->cl);
}

// syntax
static void checkFuncArray(KonohaContext *kctx, kFunc **synp);
static void parseSyntaxRule(KonohaContext *kctx, const char *rule, kline_t pline, kArray *a);

static ksyntax_t* NameSpace_syn(KonohaContext *kctx, kNameSpace *ks0, ksymbol_t kw, int isnew)
{
	kNameSpace *ks = ks0;
	uintptr_t hcode = kw;
	ksyntax_t *parent = NULL;
	assert(ks0 != NULL);  /* scan-build: remove warning */
	while(ks != NULL) {
		if(ks->syntaxMapNN != NULL) {
			kmape_t *e = kmap_get(ks->syntaxMapNN, hcode);
			while(e != NULL) {
				if(e->hcode == hcode) {
					parent = (ksyntax_t*)e->uvalue;
					if(isnew && ks0 != ks) goto L_NEW;
					return parent;
				}
				e = e->next;
			}
		}
		ks = ks->parentNULL;
	}
	L_NEW:;
	if(isnew == 1) {
		if(ks0->syntaxMapNN == NULL) {
			((struct _kNameSpace*)ks0)->syntaxMapNN = kmap_init(0);
		}
		kmape_t *e = kmap_newentry(ks0->syntaxMapNN, hcode);
		struct _ksyntax *syn = (struct _ksyntax*)KCALLOC(sizeof(ksyntax_t), 1);
		e->uvalue = (uintptr_t)syn;

		if(parent != NULL) {  // TODO: RCGC
			memcpy(syn, parent, sizeof(ksyntax_t));
			checkFuncArray(kctx, &(syn->PatternMatch));
			checkFuncArray(kctx, &(syn->ParseExpr));
			checkFuncArray(kctx, &(syn->TopStmtTyCheck));
			checkFuncArray(kctx, &(syn->StmtTyCheck));
			checkFuncArray(kctx, &(syn->ExprTyCheck));
		}
		else {
			syn->kw  = kw;
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

static void SYN_setSugarFunc(KonohaContext *kctx, kNameSpace *ks, ksymbol_t kw, size_t idx, kFunc *fo)
{
	struct _ksyntax *syn = (struct _ksyntax *)NameSpace_syn(kctx, ks, kw, 1/*new*/);
	kFunc **synp = &(syn->PatternMatch);
	DBG_ASSERT(idx <= SYNIDX_ExprTyCheck);
	KSETv(synp[idx], fo);
}

static void SYN_addSugarFunc(KonohaContext *kctx, kNameSpace *ks, ksymbol_t kw, size_t idx, kFunc *fo)
{
	struct _ksyntax *syn = (struct _ksyntax *)NameSpace_syn(kctx, ks, kw, 1/*new*/);
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

static void NameSpace_defineSyntax(KonohaContext *kctx, kNameSpace *ks, KDEFINE_SYNTAX *syndef)
{
	MethodFunc pPatternMatch = NULL, pParseExpr = NULL, pStmtTyCheck = NULL, pExprTyCheck = NULL;
	kFunc *mPatternMatch = NULL, *mParseExpr = NULL, *mStmtTyCheck = NULL, *mExprTyCheck = NULL;
	while(syndef->kw != KW_END) {
		struct _ksyntax* syn = (struct _ksyntax*)NameSpace_syn(kctx, ks, syndef->kw, 1/*isnew*/);
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
		DBG_ASSERT(syn == SYN_(ks, syndef->kw));
		syndef++;
	}
	//DBG_P("syntax size=%d, hmax=%d", ks->syntaxMapNN->size, ks->syntaxMapNN->hmax);
}

#define T_statement(kw)  KW_tSTMT_(kctx, kw), KW_tSTMTPOST(kw)

static const char* KW_tSTMT_(KonohaContext *kctx, ksymbol_t kw)
{
	const char *statement = SYM_t(kw);
	if(kw == KW_ExprPattern) statement = "expression";
	else if(kw == KW_StmtTypeDecl) statement = "variable";
	else if(kw == KW_StmtMethodDecl) statement =  "function";
	return statement;
}

static const char* KW_tSTMTPOST(ksymbol_t kw)
{
	const char *postfix = " statement";
	if(kw == KW_ExprPattern) postfix = "";
	else if(kw == KW_StmtTypeDecl || kw == KW_StmtMethodDecl) postfix = " declaration";
	return postfix;
}

// USymbolTable
static int comprKeyVal(const void *a, const void *b)
{
	int akey = SYMKEY_unbox(((kvs_t*)a)->key);
	int bkey = SYMKEY_unbox(((kvs_t*)b)->key);
	return akey - bkey;
}

static kvs_t* NameSpace_getConstNULL(KonohaContext *kctx, kNameSpace *ks, ksymbol_t ukey)
{
	size_t min = 0, max = KARRAYSIZE(ks->cl.bytesize, kvs);
	while(min < max) {
		size_t p = (max + min) / 2;
		ksymbol_t key = SYMKEY_unbox(ks->cl.kvs[p].key);
		if(key == ukey) return ks->cl.kvs + p;
		if(key < ukey) {
			min = p + 1;
		}
		else {
			max = p;
		}
	}
	return NULL;
}

static kbool_t checkConflictedConst(KonohaContext *kctx, kNameSpace *ks, kvs_t *kvs, kline_t pline)
{
	ksymbol_t ukey = kvs->key;
	kvs_t* ksval = NameSpace_getConstNULL(kctx, ks, ukey);
	if(ksval != NULL) {
		if(kvs->ty == ksval->ty && kvs->uval == ksval->uval) {
			return true;  // same value
		}
		kreportf(WARN_, pline, "conflicted name: %s", SYM_t(SYMKEY_unbox(ukey)));
		return true;
	}
	return false;
}

static void NameSpace_mergeConstData(KonohaContext *kctx, struct _kNameSpace *ks, kvs_t *kvs, size_t nitems, kline_t pline)
{
	size_t i, s = KARRAYSIZE(ks->cl.bytesize, kvs);
	if(s == 0) {
		KARRAY_INIT(&ks->cl, (nitems + 8) * sizeof(kvs_t));
		memcpy(ks->cl.kvs, kvs, nitems * sizeof(kvs_t));
	}
	else {
		kwb_t wb;
		kwb_init(&(ctxsugar->cwb), &wb);
		for(i = 0; i < nitems; i++) {
			if(checkConflictedConst(kctx, ks, kvs+i, pline)) continue;
			kwb_write(&wb, (const char*)(kvs+i), sizeof(kvs_t));
		}
		kvs = (kvs_t*)kwb_top(&wb, 0);
		nitems = kwb_bytesize(&wb)/sizeof(kvs_t);
		if(nitems > 0) {
			KARRAY_RESIZE(&ks->cl, (s + nitems + 8) * sizeof(kvs_t));
			memcpy(ks->cl.kvs + s, kvs, nitems * sizeof(kvs_t));
		}
		kwb_free(&wb);
	}
	ks->cl.bytesize = (s + nitems) * sizeof(kvs_t);
	PLAT qsort_i(ks->cl.kvs, s + nitems, sizeof(kvs_t), comprKeyVal);
}

static size_t strlen_alnum(const char *p)
{
	size_t len = 0;
	while(isalnum(p[len]) || p[len] == '_') len++;
	return len;
}

static void NameSpace_loadConstData(KonohaContext *kctx, kNameSpace *ks, const char **d, kline_t pline)
{
	INIT_GCSTACK();
	kvs_t kv;
	kwb_t wb;
	kwb_init(&(kctx->stack->cwb), &wb);
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
		kwb_write(&wb, (const char*)(&kv), sizeof(kvs_t));
		d += 3;
	}
	size_t nitems = kwb_bytesize(&wb) / sizeof(kvs_t);
	if(nitems > 0) {
		NameSpace_mergeConstData(kctx, (struct _kNameSpace*)ks, (kvs_t*)kwb_top(&wb, 0), nitems, pline);
	}
	kwb_free(&wb);
	RESET_GCSTACK();
}

static void NameSpace_importClassName(KonohaContext *kctx, kNameSpace *ks, kpack_t packid, kline_t pline)
{
	kvs_t kv;
	kwb_t wb;
	kwb_init(&(kctx->stack->cwb), &wb);
	size_t i, size = KARRAYSIZE(kctx->share->ca.bytesize, uintptr);
	for(i = 0; i < size; i++) {
		KonohaClass *ct = CT_(i);
		if(CT_isPrivate(ct)) continue;
		if(ct->packid == packid) {
			DBG_P("importing packid=%s.%s, %s..", PN_t(ct->packid), SYM_t(ct->nameid), PN_t(packid));
			kv.key = ct->nameid;
			kv.ty  = TY_TYPE;
			kv.uval = (uintptr_t)ct;
			kwb_write(&wb, (const char*)(&kv), sizeof(kvs_t));
		}
	}
	size_t nitems = kwb_bytesize(&wb) / sizeof(kvs_t);
	if(nitems > 0) {
		NameSpace_mergeConstData(kctx, (struct _kNameSpace*)ks, (kvs_t*)kwb_top(&wb, 0), nitems, pline);
	}
	kwb_free(&wb);
}

// NameSpace

static KonohaClass *NameSpace_getCT(KonohaContext *kctx, kNameSpace *ks, KonohaClass *thisct/*NULL*/, const char *name, size_t len, ktype_t def)
{
	KonohaClass *ct = NULL;
	ksymbol_t un = ksymbolA(name, len, SYM_NONAME);
	if(un != SYM_NONAME) {
		uintptr_t hcode = longid(PN_konoha, un);
		ct = (KonohaClass*)map_getu(kctx, kctx->share->lcnameMapNN, hcode, 0);
		if(ct == NULL) {
			kvs_t *kvs = NameSpace_getConstNULL(kctx, ks, un);
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
	if(unlikely(ct->methods == K_EMPTYARRAY)) {
		KINITv(((KonohaClassVar*)ct)->methods, new_(MethodArray, 8));
	}
	kArray_add(ct->methods, mtd);
}

static void NameSpace_addMethod(KonohaContext *kctx, kNameSpace *ks, kMethod *mtd)
{
	if(ks->methods == K_EMPTYARRAY) {
		KINITv(((struct _kNameSpace*)ks)->methods, new_(MethodArray, 8));
	}
	kArray_add(ks->methods, mtd);
}

/* NameSpace/Class/Method */
static kMethod* CT_findMethodNULL(KonohaContext *kctx, KonohaClass *ct, kmethodn_t mn)
{
	while(ct != NULL) {
		size_t i;
		kArray *a = ct->methods;
		for(i = 0; i < kArray_size(a); i++) {
			kMethod *mtd = a->methods[i];
			if((mtd)->mn == mn) {
				return mtd;
			}
		}
		ct = ct->searchSuperMethodClassNULL;
	}
	return NULL;
}

#define kNameSpace_getStaticMethodNULL(ns, mn)   NameSpace_getStaticMethodNULL(kctx, ns, mn)

static kMethod* NameSpace_getMethodNULL(KonohaContext *kctx, kNameSpace *ks, ktype_t cid, kmethodn_t mn)
{
	while(ks != NULL) {
		size_t i;
		kArray *a = ks->methods;
		for(i = 0; i < kArray_size(a); i++) {
			kMethod *mtd = a->methods[i];
			if(mtd->cid == cid && mtd->mn == mn) {
				return mtd;
			}
		}
		ks = ks->parentNULL;
	}
	return CT_findMethodNULL(kctx, CT_(cid), mn);
}

//static kMethod* NameSpace_getStaticMethodNULL(KonohaContext *kctx, kNameSpace *ks, kmethodn_t mn)
//{
//	while(ks != NULL) {
//		kMethod *mtd = kNameSpace_getMethodNULL(ks, O_cid(ks->scrobj), mn);
//		if(mtd != NULL && kMethod_isStatic(mtd)) {
//			return mtd;
//		}
//		if(ks->static_cid != TY_unknown) {
//			kMethod *mtd = kNameSpace_getMethodNULL(ks, ks->static_cid, mn);
//			if(mtd != NULL && kMethod_isStatic(mtd)) {
//				return mtd;
//			}
//		}
//		ks = ks->parentNULL;
//	}
//	return NULL;
//}

#define kNameSpace_getCastMethodNULL(ns, cid, tcid)     NameSpace_getCastMethodNULL(kctx, ns, cid, tcid)
static kMethod* NameSpace_getCastMethodNULL(KonohaContext *kctx, kNameSpace *ks, ktype_t cid, ktype_t tcid)
{
	kMethod *mtd = NameSpace_getMethodNULL(kctx, ks, cid, MN_to(tcid));
	if(mtd == NULL) {
		mtd = NameSpace_getMethodNULL(kctx, ks, cid, MN_as(tcid));
	}
	return mtd;
}

#define kNameSpace_defineMethod(NS,MTD,UL)  NameSpace_defineMethod(kctx, NS, MTD, UL)

static kbool_t NameSpace_defineMethod(KonohaContext *kctx, kNameSpace *ks, kMethod *mtd, kline_t pline)
{
	//if(pline != 0) {
	//	kMethod *mtdOLD = NameSpace_getMethodNULL(kctx, ks, mtd->cid, mtd->mn);
	//	if(mtdOLD != NULL) {
	//		char mbuf[128];
	//		kreportf(ERR_, pline, "method %s.%s is already defined", TY_t(mtd->cid), T_mn(mbuf, mtd->mn));
	//		return 0;
	//	}
	//}
	if(mtd->packid == 0) {
		((kMethodVar*)mtd)->packid = ks->packid;
	}
	KonohaClass *ct = CT_(mtd->cid);
	if(ct->packdom == ks->packdom && kMethod_isPublic(mtd)) {
		CT_addMethod(kctx, ct, mtd);
	}
	else {
		NameSpace_addMethod(kctx, ks, mtd);
	}
	return 1;
}

static void NameSpace_loadMethodData(KonohaContext *kctx, kNameSpace *ks, intptr_t *data)
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
		if(ks == NULL || kMethod_isPublic(mtd)) {
			CT_addMethod(kctx, CT_(cid), mtd);
		} else {
			NameSpace_addMethod(kctx, ks, mtd);
		}
	}
}

//#define kNameSpace_loadGlueFunc(NS, F, OPT, UL)  NameSpace_loadGlueFunc(kctx, NS, F, OPT, UL)
//
//static MethodFunc NameSpace_loadGlueFunc(KonohaContext *kctx, kNameSpace *ks, const char *funcname, int DOPTION, kline_t pline)
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
	struct _kToken *tk = (struct _kToken*)o;
	tk->uline     =   0;
	tk->kw        =   (ksymbol_t)(intptr_t)conf;
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
		DUMP_P("%s%s %d: kw=%s%s '%s'\n", KW_t(tk->kw), (short)tk->uline, KW_t(tk->kw), kToken_s(tk));
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
	switch(tk->kw) {
	case AST_PARENTHESIS: return '(';
	case AST_BRACE: return '{';
	case AST_BRACKET: return '[';
	}
	return '<';
}

static int kTokenList_endChar(kToken *tk)
{
	switch(tk->kw) {
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
	struct _kExpr *expr      =   (struct _kExpr*)o;
	expr->build      =   TEXPR_UNTYPED;
	expr->ty         =   TY_var;
	KINITv(expr->tk, K_NULLTOKEN);
	KINITv(expr->data, K_NULL);
	expr->syn = (ksyntax_t*)conf;
}

static void Expr_reftrace(KonohaContext *kctx, kObject *o)
{
	kExpr *expr = (kExpr*)o;
	BEGIN_REFTRACE(2);
	KREFTRACEv(expr->tk);
	KREFTRACEv(expr->data);
	END_REFTRACE();
}

static struct _kExpr* Expr_vadd(KonohaContext *kctx, struct _kExpr *expr, int n, va_list ap)
{
	int i;
	if(!IS_Array(expr->cons)) {
		KSETv(expr->cons, new_(Array, 8));
	}
	for(i = 0; i < n; i++) {
		kObject *v =  (kObject*)va_arg(ap, kObject*);
		if(v == NULL || v == (kObject*)K_NULLEXPR) {
			return (struct _kExpr*)K_NULLEXPR;
		}
		kArray_add(expr->cons, v);
	}
	return expr;
}

static kExpr* new_ConsExpr(KonohaContext *kctx, ksyntax_t *syn, int n, ...)
{
	va_list ap;
	va_start(ap, n);
	DBG_ASSERT(syn != NULL);
	struct _kExpr *expr = new_W(Expr, syn);
	PUSH_GCSTACK(expr);
	expr = Expr_vadd(kctx, expr, n, ap);
	va_end(ap);
	return (kExpr*)expr;
}

static kExpr* new_TypedConsExpr(KonohaContext *kctx, int build, ktype_t ty, int n, ...)
{
	va_list ap;
	va_start(ap, n);
	struct _kExpr *expr = new_W(Expr, NULL);
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
	struct _kExpr *expr = new_W(Expr, NULL);
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
			DUMP_P("[%d] ExprTerm: kw='%s%s' %s", n, KW_t(expr->tk->kw), kToken_s(expr->tk));
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
				DUMP_P("[%d] Cons: kw='%s%s', size=%ld", n, KW_t(expr->syn->kw), kArray_size(expr->cons));
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
	W(kExpr, expr);
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
	WASSERT(expr);
	return expr;
}

static kExpr* Expr_setNConstValue(KonohaContext *kctx, kExpr *expr, ktype_t ty, uintptr_t ndata)
{
	if(expr == NULL) {
		expr = new_(Expr, 0);
		PUSH_GCSTACK(expr);
	}
	W(kExpr, expr);
	Wexpr->build = TEXPR_NCONST;
	Wexpr->ndata = ndata;
	KSETv(Wexpr->data, K_NULL);
	Wexpr->ty = ty;
	WASSERT(expr);
	return expr;
}

static kExpr *Expr_setVariable(KonohaContext *kctx, kExpr *expr, int build, ktype_t ty, intptr_t index, kGamma *gma)
{
	if(expr == NULL) {
		expr = new_W(Expr, 0);
		PUSH_GCSTACK(expr);
	}
	W(kExpr, expr);
	Wexpr->build = build;
	Wexpr->ty = ty;
	Wexpr->index = index;
	// KSETv(Wexpr->data, K_NULL);  block need unclear
	if(build < TEXPR_UNTYPED) {
		kArray_add(gma->genv->lvarlst, Wexpr);
	}
	WASSERT(expr);
	return expr;
}

/* --------------- */
/* Stmt */

static void Stmt_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct _kStmt *stmt = (struct _kStmt*)o;
	stmt->uline      =   (kline_t)conf;
	stmt->syn = NULL;
	stmt->parentNULL = NULL;
}

static void Stmt_reftrace(KonohaContext *kctx, kObject *o)
{
	kStmt *stmt = (kStmt*)o;
	BEGIN_REFTRACE(1);
	KREFTRACEn(stmt->parentNULL);
	END_REFTRACE();
}

static void _dumpToken(KonohaContext *kctx, void *arg, kvs_t *d)
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
			DUMP_P("STMT %s%s {\n", T_statement(stmt->syn->kw));
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

static kbool_t Token_toBRACE(KonohaContext *kctx, struct _kToken *tk, kNameSpace *ks);
static kBlock *new_Block(KonohaContext *kctx, kNameSpace* ks, kStmt *stmt, kArray *tls, int s, int e, int delim);
static kBlock* Stmt_block(KonohaContext *kctx, kStmt *stmt, ksymbol_t kw, kBlock *def)
{
	kBlock *bk = (kBlock*)kObject_getObjectNULL(stmt, kw);
	if(bk != NULL) {
		if(IS_Token(bk)) {
			kToken *tk = (kToken*)bk;
			if (tk->kw == TK_CODE) {
				Token_toBRACE(kctx, (struct _kToken*)tk, kStmt_ks(stmt));
			}
			if (tk->kw == AST_BRACE) {
				bk = new_Block(kctx, kStmt_ks(stmt), stmt, tk->sub, 0, kArray_size(tk->sub), ';');
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
	struct _kBlock *bk = (struct _kBlock*)o;
	kNameSpace *ks = (conf != NULL) ? (kNameSpace*)conf : KNULL(NameSpace);
	bk->parentNULL = NULL;
	KINITv(bk->ks, ks);
	KINITv(bk->blocks, new_(StmtArray, 0));
	KINITv(bk->esp, new_(Expr, 0));
}

static void Block_reftrace(KonohaContext *kctx, kObject *o)
{
	kBlock *bk = (kBlock*)o;
	BEGIN_REFTRACE(4);
	KREFTRACEv(bk->ks);
	KREFTRACEv(bk->blocks);
	KREFTRACEv(bk->esp);
	KREFTRACEn(bk->parentNULL);
	END_REFTRACE();
}

static void Block_insertAfter(KonohaContext *kctx, kBlock *bk, kStmt *target, kStmt *stmt)
{
	//DBG_ASSERT(stmt->parentNULL == NULL);
	KSETv(((struct _kStmt*)stmt)->parentNULL, bk);
	size_t i;
	for(i = 0; i < kArray_size(bk->blocks); i++) {
		if(bk->blocks->stmts[i] == target) {
			kArray_insert(bk->blocks, i+1, stmt);
			return;
		}
	}
	DBG_ABORT("target was not found!!");
}

/* --------------- */
/* Block */

static void Gamma_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct _kGamma *gma = (struct _kGamma*)o;
	gma->genv = NULL;
}

