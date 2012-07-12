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

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */

static GammaAllocaData *Gamma_push(KonohaContext *kctx, kGamma *gma, GammaAllocaData *newone)
{
	GammaAllocaData *oldone = gma->genv;
	gma->genv = newone;
	newone->lvarlst = ctxsugar->lvarlst;
	newone->lvarlst_top = kArray_size(ctxsugar->lvarlst);
	return oldone;
}

static GammaAllocaData *Gamma_pop(KonohaContext *kctx, kGamma *gma, GammaAllocaData *oldone, GammaAllocaData *checksum)
{
	GammaAllocaData *newone = gma->genv;
	assert(checksum == newone);
	gma->genv = oldone;
	kArray_clear(newone->lvarlst, newone->lvarlst_top);
	if(newone->l.allocsize > 0) {
		KFREE(newone->l.vars, newone->l.allocsize);
	}
	if(newone->f.allocsize > 0) {
		KFREE(newone->f.vars, newone->f.allocsize);
	}
	return newone;
}

#define GAMMA_PUSH(G,B) GammaAllocaData *oldbuf_ = Gamma_push(kctx, G, B)
#define GAMMA_POP(G,B)  Gamma_pop(kctx, G, oldbuf_, B)

// --------------------------------------------------------------------------

static KMETHOD UndefinedExprTyCheck(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	if(Expr_isTerm(expr)) {
		expr = kToken_p(stmt, expr->tk, ERR_, "undefined token type checker: '%s'", kToken_s(expr->tk));
	}
	else {
		expr = kStmt_p(stmt, ERR_, "undefined operator type checker: %s%s",  KW_t(expr->syn->keyword));
	}
	RETURN_(expr);
}

static kExpr *ExprTyCheckFunc(KonohaContext *kctx, kFunc *fo, kStmt *stmt, kExpr *expr, kGamma *gma, int reqty)
{
	INIT_GCSTACK();
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 5);
	KSETv(lsfp[K_CALLDELTA+0].o, fo->self);
	KSETv(lsfp[K_CALLDELTA+1].o, (kObject*)stmt);
	KSETv(lsfp[K_CALLDELTA+2].o, (kObject*)expr);
	KSETv(lsfp[K_CALLDELTA+3].o, (kObject*)gma);
	lsfp[K_CALLDELTA+4].ivalue = reqty;
	KCALL(lsfp, 0, fo->mtd, 5, K_NULLEXPR);
	END_LOCAL();
	RESET_GCSTACK();
	DBG_ASSERT(IS_Expr(lsfp[0].o));
	return (kExpr*)lsfp[0].o;
}

static kExpr *ExprTyCheck(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kGamma *gma, int reqty)
{
	kFunc *fo = expr->syn->ExprTyCheck;
	kExpr *texpr;
	if(IS_Array(fo)) {
		int i;
		kArray *a = (kArray*)fo;
		for(i = kArray_size(a) - 1; i > 0; i--) {
			texpr = ExprTyCheckFunc(kctx, a->funcs[i], stmt, expr, gma, reqty);
			if(kStmt_isERR(stmt)) return K_NULLEXPR;
			if(texpr->ty != TY_var) return texpr;
		}
		fo = a->funcs[0];
	}
	DBG_ASSERT(IS_Func(fo));
	texpr = ExprTyCheckFunc(kctx, fo, stmt, expr, gma, reqty);
	if(kStmt_isERR(stmt)) return K_NULLEXPR;
//	FIXME: CHECK ALL VAR_ExprTyCheck
//	if(texpr->ty == TY_var && texpr != K_NULLEXPR) {
//		texpr = kExpr_p(stmt, expr, ERR_, "typing error");
//	}
	return texpr;
}

static void Expr_putConstValue(KonohaContext *kctx, kExpr *expr, KonohaStack *sfp)
{
	if(expr->build == TEXPR_CONST) {
		KSETv(sfp[0].o, expr->data);
		sfp[0].ndata = O_unbox(expr->data);
	}else if(expr->build == TEXPR_NCONST) {
		sfp[0].ndata = expr->ndata;
	}else if(expr->build == TEXPR_NEW) {
		KSETv(sfp[0].o, new_kObject(CT_(expr->ty), expr->ndata /*FIXME*/));
	}else {
		assert(expr->build == TEXPR_NULL);
		KSETv(sfp[0].o, knull(CT_(expr->ty)));
		sfp[0].ndata = 0;
	}
}

static kExpr* ExprCall_toConstValue(KonohaContext *kctx, kExpr *expr, kArray *cons, ktype_t rtype)
{
	size_t i, size = kArray_size(cons), psize = size - 2;
	kMethod *mtd = cons->methodList[0];
	BEGIN_LOCAL(lsfp, K_CALLDELTA + psize);
	for(i = 1; i < size; i++) {
		Expr_putConstValue(kctx, cons->exprs[i], lsfp + K_CALLDELTA + i - 1);
	}
	KCALL(lsfp, 0, mtd, psize, knull(CT_(expr->ty)));
	END_LOCAL();
	if(TY_isUnbox(rtype) || rtype == TY_void) {
		return kExpr_setNConstValue(expr, rtype, lsfp[0].ndata);
	}
	return kExpr_setConstValue(expr, rtype, lsfp[0].o);
}

static kbool_t CT_isa(KonohaContext *kctx, ktype_t cid1, ktype_t cid2)
{
	DBG_ASSERT(cid1 != cid2); // should be checked
	KonohaClass *ct = CT_(cid1), *t = CT_(cid2);
	return ct->isSubType(kctx, ct, t);
}

static kExpr *new_BoxingExpr(KonohaContext *kctx, kExpr *expr, ktype_t reqty)
{
	if(expr->build == TEXPR_NCONST) {
		kExprVar *Wexpr = (kExprVar*)expr;
		Wexpr->build = TEXPR_CONST;
		KINITv(Wexpr->data, new_kObject(CT_(Wexpr->ty), Wexpr->ndata));
		Wexpr->ty = reqty;
		return expr;
	}
	else {
		kExprVar *texpr = new_Var(Expr, NULL);
		PUSH_GCSTACK(texpr);
		KINITv(texpr->single, expr);
		texpr->build = TEXPR_BOX;
		texpr->ty = reqty;
		return texpr;
	}
}

static kExpr *Expr_tyCheck(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kGamma *gma, ktype_t reqty, int pol)
{
	kExpr *texpr = expr;
	if(kStmt_isERR(stmt)) texpr = K_NULLEXPR;
	if(expr->ty == TY_var && expr != K_NULLEXPR) {
		if(!IS_Expr(expr)) {
			expr = new_ConstValue(O_cid(expr), expr);
			PUSH_GCSTACK(expr);
		}
		texpr = ExprTyCheck(kctx, stmt, expr, gma, reqty);
	}
	if(kStmt_isERR(stmt)) texpr = K_NULLEXPR;
	if(texpr != K_NULLEXPR) {
		//DBG_P("type=%s, reqty=%s", TY_t(expr->ty), TY_t(reqty));
		if(texpr->ty == TY_void) {
			if(!FLAG_is(pol, TPOL_ALLOWVOID)) {
				texpr = kExpr_p(stmt, expr, ERR_, "void is not acceptable");
			}
			return texpr;
		}
		if(reqty == TY_var || texpr->ty == reqty || FLAG_is(pol, TPOL_NOCHECK)) {
			return texpr;
		}
		if(CT_isa(kctx, texpr->ty, reqty)) {
			if(TY_isUnbox(texpr->ty) && !TY_isUnbox(reqty)) {
				return new_BoxingExpr(kctx, expr, reqty);
			}
			return texpr;
		}
		kMethod *mtd = kNameSpace_getCastMethodNULL(gma->genv->ns, texpr->ty, reqty);
		DBG_P("finding cast %s => %s: %p", TY_t(texpr->ty), TY_t(reqty), mtd);
		if(mtd != NULL && (kMethod_isCoercion(mtd) || FLAG_is(pol, TPOL_COERCION))) {
			return new_TypedMethodCall(kctx, stmt, reqty, mtd, gma, 1, texpr);
		}
		return kExpr_p(stmt, expr, ERR_, "%s is requested, but %s is given", TY_t(reqty), TY_t(texpr->ty));
	}
	return texpr;
}

static kExpr* Expr_tyCheckAt(KonohaContext *kctx, kStmt *stmt, kExpr *exprP, size_t pos, kGamma *gma, ktype_t reqty, int pol)
{
	if(!Expr_isTerm(exprP) && pos < kArray_size(exprP->cons)) {
		kExpr *expr = exprP->cons->exprs[pos];
		expr = Expr_tyCheck(kctx, stmt, expr, gma, reqty, pol);
		KSETv(exprP->cons->exprs[pos], expr);
		return expr;
	}
	return K_NULLEXPR;
}

static kbool_t Stmt_tyCheckExpr(KonohaContext *kctx, kStmt *stmt, ksymbol_t nameid, kGamma *gma, ktype_t reqty, int pol)
{
	kExpr *expr = (kExpr*)kObject_getObjectNULL(stmt, nameid);
	if(expr != NULL && IS_Expr(expr)) {
		kExpr *texpr = Expr_tyCheck(kctx, stmt, expr, gma, reqty, pol);
//		DBG_P("reqty=%s, texpr->ty=%s isnull=%d", TY_t(reqty), TY_t(texpr->ty), (texpr == K_NULLEXPR));
		if(texpr != K_NULLEXPR) {
			if(texpr != expr) {
				kObject_setObject(stmt, nameid, texpr);
			}
			return 1;
		}
	}
	return 0;
}

/* ------------------------------------------------------------------------ */

static KMETHOD ExprTyCheck_Text(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->tk;
	RETURN_(kExpr_setConstValue(expr, TY_String, tk->text));
}

static KMETHOD ExprTyCheck_Type(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	DBG_ASSERT(TK_isType(expr->tk));
	RETURN_(kExpr_setVariable(expr, NULL, expr->tk->ty, 0, gma));
}

static KMETHOD ExprTyCheck_true(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	RETURN_(kExpr_setNConstValue(expr, TY_Boolean, (uintptr_t)1));
}

static KMETHOD ExprTyCheck_false(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	RETURN_(kExpr_setNConstValue(expr, TY_Boolean, (uintptr_t)0));
}

static KMETHOD ExprTyCheck_Int(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->tk;
	long long n = strtoll(S_text(tk->text), NULL, 0);
	RETURN_(kExpr_setNConstValue(expr, TY_Int, (uintptr_t)n));
}


static kMethod* NameSpace_getGetterMethodNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t cid, ksymbol_t fn)
{
	kMethod *mtd = kNameSpace_getMethodNULL(ns, cid, MN_toGETTER(fn));
	if(mtd == NULL) {
		mtd = kNameSpace_getMethodNULL(ns, cid, MN_toISBOOL(fn));
	}
	return mtd;
}

static kExpr* new_GetterExpr(KonohaContext *kctx, kToken *tkU, kMethod *mtd, kExpr *expr)
{
	kExprVar *expr1 = (kExprVar *)new_TypedConsExpr(kctx, TEXPR_CALL, kMethod_rtype(mtd), 2, mtd, expr);
	KSETv(expr1->tk, tkU); // for uline
	return (kExpr*)expr1;
}

static kObject *NameSpace_getSymbolValueNULL(KonohaContext *kctx, kNameSpace *ns, const char *key, size_t klen)
{
	if(key[0] == 'K' && (key[1] == 0 || strcmp("Konoha", key) == 0)) {
		return (kObject*)ns;
	}
	return NULL;
}

static kExpr* Expr_tyCheckVariable2(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kGamma *gma, ktype_t reqty)
{
	DBG_ASSERT(expr->ty == TY_var);
	kToken *tk = expr->tk;
	ksymbol_t fn = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NONAME);
	int i;
	GammaAllocaData *genv = gma->genv;
	for(i = genv->l.varsize - 1; i >= 0; i--) {
		if(genv->l.vars[i].fn == fn) {
			return kExpr_setVariable(expr, LOCAL_, genv->l.vars[i].ty, i, gma);
		}
	}
	for(i = genv->f.varsize - 1; i >= 0; i--) {
		if(genv->f.vars[i].fn == fn) {
			return kExpr_setVariable(expr, LOCAL, genv->f.vars[i].ty, i, gma);
		}
	}
	if(genv->f.vars[0].ty != TY_void) {
		DBG_ASSERT(genv->this_cid == genv->f.vars[0].ty);
		KonohaClass *ct = CT_(genv->this_cid);
		for(i = ct->fsize; i >= 0; i--) {
			if(ct->fields[i].fn == fn && ct->fields[i].ty != TY_void) {
				return kExpr_setVariable(expr, FIELD, ct->fields[i].ty, longid((kshort_t)i, 0), gma);
			}
		}
		kMethod *mtd = NameSpace_getGetterMethodNULL(kctx, genv->ns, genv->this_cid, fn);
		if(mtd != NULL) {
			return new_GetterExpr(kctx, tk, mtd, new_Variable(LOCAL, genv->this_cid, 0, gma));
		}
//		mtd = kNameSpace_getMethodNULL(genv->ns, genv->this_cid, fn);
//		if(mtd != NULL) {
//			return new_FuncValue(kctx, mtd, 0);
//		}
	}
	{
		ktype_t cid = O_cid(genv->ns->scriptObject);
		kMethod *mtd = NameSpace_getGetterMethodNULL(kctx, genv->ns, cid, fn);
		if(mtd != NULL && cid != TY_System) {
			return new_GetterExpr(kctx, tk, mtd, new_ConstValue(cid, genv->ns->scriptObject));
		}
		mtd = kNameSpace_getMethodNULL(genv->ns, cid, fn);
		if(mtd != NULL) {
			kParam *pa = kMethod_param(mtd);
			KonohaClass *ct = kClassTable_Generics(CT_Func, pa->rtype, pa->psize, (kparam_t*)pa->p);
			kFuncVar *fo = (kFuncVar*)new_kObject(ct, mtd);
			PUSH_GCSTACK(fo);
			KSETv(fo->self, genv->ns->scriptObject);
			return new_ConstValue(ct->cid, fo);
		}
	}
	if(fn != SYM_NONAME) {
		kvs_t *kv = NameSpace_getConstNULL(kctx, gma->genv->ns, fn);
		if(kv != NULL) {
			if(SYMKEY_isBOXED(kv->key)) {
				kExpr_setConstValue(expr, kv->ty, kv->oval);
			}
			else {
				kExpr_setNConstValue(expr, kv->ty, kv->uval);
			}
			return expr;
		}
	}
	kObject *v = NameSpace_getSymbolValueNULL(kctx, gma->genv->ns, S_text(tk->text), S_size(tk->text));
	kExpr *texpr = (v == NULL) ? kToken_p(stmt, tk, ERR_, "undefined name: %s", kToken_s(tk)) : kExpr_setConstValue(expr, O_cid(v), v);
	return texpr;
}

static KMETHOD ExprTyCheck_Symbol(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	RETURN_(Expr_tyCheckVariable2(kctx, stmt, expr, gma, reqty));
}

static KMETHOD ExprTyCheck_Usymbol(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->tk;
	ksymbol_t ukey = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NONAME);
	if(ukey != SYM_NONAME) {
		kvs_t *kv = NameSpace_getConstNULL(kctx, gma->genv->ns, ukey);
		if(kv != NULL) {
			if(SYMKEY_isBOXED(kv->key)) {
				kExpr_setConstValue(expr, kv->ty, kv->oval);
			}
			else {
				kExpr_setNConstValue(expr, kv->ty, kv->uval);
			}
			RETURN_(expr);
		}
	}
	kObject *v = NameSpace_getSymbolValueNULL(kctx, gma->genv->ns, S_text(tk->text), S_size(tk->text));
	kExpr *texpr = (v == NULL) ? kToken_p(stmt, tk, ERR_, "undefined name: %s", kToken_s(tk)) : kExpr_setConstValue(expr, O_cid(v), v);
	RETURN_(texpr);
}

static KMETHOD StmtTyCheck_ConstDecl(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_StmtTyCheck(stmt, gma);
	kbool_t r = false;
	kNameSpace *ns = gma->genv->ns;
	kToken *tk = kStmt_token(stmt, KW_UsymbolPattern, NULL);
	ksymbol_t ukey = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NEWID);
	kvs_t *kv = NameSpace_getConstNULL(kctx, ns, ukey);
	if(kv != NULL) {
		kStmt_p(stmt, ERR_, "already defined name: %s", kToken_s(tk));
	}
	else {
		r = Stmt_tyCheckExpr(kctx, stmt, KW_ExprPattern, gma, TY_var, TPOL_CONST);
		if(r) {
			kExpr *expr = kStmt_expr(stmt, KW_ExprPattern, NULL);
			kvs_t kv = { ukey, expr->ty};
			if(expr->build == TEXPR_NULL) {
				kv.ty = TY_TYPE;
				kv.uval = (uintptr_t)(CT_(expr->ty));
				expr = NULL;
			}
			else if(expr->build == TEXPR_CONST) {
				kv.key = ukey | SYMKEY_BOXED;
				kv.oval = expr->data;
				expr = NULL;
			}
			else if(expr->build == TEXPR_NCONST) {
				kv.uval = (uintptr_t)expr->ndata;
				expr = NULL;
			}
			if(expr == NULL) {
				NameSpace_mergeConstData(kctx, (kNameSpaceVar*)ns, &kv, 1, stmt->uline);
			}
			else {
				kStmt_p(stmt, ERR_, "constant value is expected");
			}
			kStmt_done(stmt);
		}
	}
	RETURNb_(r);
}

static ktype_t ktype_var(KonohaContext *kctx, ktype_t ty, KonohaClass *this_ct)
{
	KonohaClass *ct = CT_(ty);
	ct = ct->realtype(kctx, ct, this_ct);
	return ct->cid;
}

static int param_policy(ksymbol_t fn)
{
	int pol = 0;
	if(FN_isCOERCION(fn)) {
		pol = pol | TPOL_COERCION;
	}
	return pol;
}

///* @Overloading */
//static void NameSpace_lookupMethods(KonohaContext *kctx, kNameSpace *ns, ktype_t cid, kmethodn_t mn, kArray *abuf)
//{
//	while(ks != NULL) {
//		size_t i;
//		kArray *a = ks->methodList;
//		for(i = 0; i < kArray_size(a); i++) {
//			kMethod *mtd = a->methodList[i];
//			if(mtd->cid == cid && mtd->mn == mn) {
//				kArray_add(abuf, mtd);
//			}
//		}
//		ks = ks->parentNULL;
//	}
//	KonohaClass *ct = CT_(cid);
//	while(ct != NULL) {
//		size_t i;
//		kArray *a = ct->methodList;
//		for(i = 0; i < kArray_size(a); i++) {
//			kMethod *mtd = a->methodList[i];
//			if(mtd->mn == mn) {
//				kArray_add(abuf, mtd);
//			}
//		}
//		ct = ct->searchSuperMethodClassNULL;
//	}
//}
//
//static kbool_t kParam_equals(KonohaContext *kctx, kParam *pa, KonohaClass *thisct, int psize, kparam_t *p)
//{
//	int i;
//	for(i = 0; i < psize; i++) {
//		if(pa->p[i].ty == p[i].ty) continue;
//		if(ktype_var(kctx, pa->p[i].ty, thisct) != p[i].ty) return false;
//	}
//	return true;
//}
//
//static kbool_t kParam_match(KonohaContext *kctx, kParam *pa, KonohaClass *thisct, int psize, kparam_t *p)
//{
////	int i;
////	for(i = 0; i < psize; i++) {
////		if(pa->p[i].ty != p[i].ty) return false;
////	}
////	return true;
//	return false;
//}
//
///* NameSpace/Class/Method */
//static kMethod* Array_lookupMethod(KonohaContext *kctx, kArray *a, int s, int e, KonohaClass *thisct, int psize, kparam_t *p)
//{
//	int i;
//	for(i = s; i < e; i++) {
//		kParam *pa = kMethod_param(a->methodList[i]);
//		if(pa->psize == psize && kParam_equals(kctx, pa, thisct, psize, p)) {
//			return a->methodList[i];
//		}
//	}
//	for(i = s; i < e; i++) {
//		kParam *pa = kMethod_param(a->methodList[i]);
//		if(kParam_match(kctx, pa, thisct, psize, p)) {
//			return a->methodList[i];
//		}
//	}
//	return NULL;
//}
//
//static kMethod *kExpr_lookUpOverloadMethod(KonohaContext *kctx, kExpr *expr, kMethod *mtd, kGamma *gma, KonohaClass *thisct)
//{
//	kArray *abuf = kctx->stack->gcstack;
//	int i, psize = kArray_size(expr->cons) - 2, atop = kArray_size(abuf);
//	kparam_t p[psize];
//	for(i = 0; i < psize; i++) {
//		p[i].ty = expr->cons->exprs[i+2]->ty;
//	}
//	KonohaClass *ct = thisct;
//	while(1) {
//		NameSpace_lookupMethods(kctx, gma->genv->ns, ct->cid, mtd->mn, abuf);
//		kMethod *mtd2 = Array_lookupMethod(kctx, abuf, atop, kArray_size(abuf), thisct, psize, p);
//		kArray_clear(abuf, atop);
//		if(mtd2 != NULL) return mtd2;
//		if(ct->cid == TY_Object) break;
//		ct = CT_(ct->supcid);
//	}
//	return mtd;
//}

static kExpr* Expr_typedWithMethod(KonohaContext *kctx, kExpr *expr, kMethod *mtd, ktype_t reqty)
{
	kExpr *expr1 = kExpr_at(expr, 1);
	KSETv(expr->cons->methodList[0], mtd);
	if(expr1->build == TEXPR_NEW) {
		kExpr_typed(expr, CALL, expr1->ty);
	}
	else {
		kExpr_typed(expr, CALL, kMethod_isSmartReturn(mtd) ? reqty : ktype_var(kctx, kMethod_rtype(mtd), CT_(expr1->ty)));
	}
	return expr;
}

static kExpr *Expr_tyCheckCallParams(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kMethod *mtd, kGamma *gma, ktype_t reqty)
{
	kArray *cons = expr->cons;
	size_t i, size = kArray_size(cons);
	kExpr *expr1 = cons->exprs[1];
	KonohaClass *this_ct = CT_(expr1->ty);
	DBG_ASSERT(IS_Method(mtd));
	DBG_ASSERT(this_ct->cid != TY_var);
	if(!TY_isUnbox(mtd->cid) && CT_isUnbox(this_ct)) {
		expr1 = new_BoxingExpr(kctx, cons->exprs[1], this_ct->cid);
		KSETv(cons->exprs[1], expr1);
	}
	int isConst = (Expr_isCONST(expr1)) ? 1 : 0;
	//	if(rtype == TY_var && gma->genv->mtd == mtd) {
	//		return ERROR_Unsupported(kctx, "type inference of recursive calls", TY_unknown, NULL);
	//	}
	for(i = 2; i < size; i++) {
		kExpr *texpr = kExpr_tyCheckAt(stmt, expr, i, gma, TY_var, 0);
		if(texpr == K_NULLEXPR) {
			return texpr;
		}
	}
//	mtd = kExpr_lookUpOverloadMethod(kctx, expr, mtd, gma, this_ct);
	kParam *pa = kMethod_param(mtd);
	if(pa->psize + 2 != size) {
		return kExpr_p(stmt, expr, ERR_, "%s.%s%s takes %d parameter(s), but given %d parameter(s)", CT_t(this_ct), T_mn(mtd->mn), (int)pa->psize, (int)size-2);
	}
	for(i = 0; i < pa->psize; i++) {
		size_t n = i + 2;
		ktype_t ptype = ktype_var(kctx, pa->p[i].ty, this_ct);
		int pol = param_policy(pa->p[i].fn);
		kExpr *texpr = kExpr_tyCheckAt(stmt, expr, n, gma, ptype, pol);
		if(texpr == K_NULLEXPR) {
			return kExpr_p(stmt, expr, ERR_, "%s.%s%s accepts %s at the parameter %d", CT_t(this_ct), T_mn(mtd->mn), TY_t(ptype), (int)i+1);
		}
		if(!Expr_isCONST(expr)) isConst = 0;
	}
	expr = Expr_typedWithMethod(kctx, expr, mtd, reqty);
	if(isConst && kMethod_isConst(mtd)) {
		ktype_t rtype = ktype_var(kctx, pa->rtype, this_ct);
		return ExprCall_toConstValue(kctx, expr, cons, rtype);
	}
	return expr;
}

static kExpr* Expr_tyCheckDynamicCallParams(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kMethod *mtd, kGamma *gma, kString *name, kmethodn_t mn, ktype_t reqty)
{
	int i;
	kParam *pa = kMethod_param(mtd);
	ktype_t ptype = (pa->psize == 0) ? TY_Object : pa->p[0].ty;
	for(i = 2; i < kArray_size(expr->cons); i++) {
		kExpr *texpr = kExpr_tyCheckAt(stmt, expr, i, gma, ptype, 0);
		if(texpr == K_NULLEXPR) return texpr;
	}
	Expr_add(kctx, expr, new_ConstValue(TY_String, name));
	return Expr_typedWithMethod(kctx, expr, mtd, reqty);
}

static const char* MethodType_t(KonohaContext *kctx, kmethodn_t mn, size_t psize)
{
//	static const char *mnname[3] = {"method", "unary operator", "binary operator"};
//	DBG_ASSERT(mn_type <= (size_t)MNTYPE_binary);
//	return mnname[mn_type];
	return "method";
}

static kExpr *Expr_lookupMethod(KonohaContext *kctx, kStmt *stmt, kExpr *expr, ktype_t this_cid, kGamma *gma, ktype_t reqty)
{
	kNameSpace *ns = gma->genv->ns;
	kTokenVar *tkMN = expr->cons->Wtoks[0];
	DBG_ASSERT(IS_Token(tkMN));
	if(tkMN->keyword == TK_SYMBOL) {
		tkMN->keyword = ksymbolA(S_text(tkMN->text), S_size(tkMN->text), SYM_NEWID);
	}
	kMethod *mtd = kNameSpace_getMethodNULL(ns, this_cid, tkMN->keyword);
	if(mtd == NULL) {
		if(tkMN->text != TS_EMPTY) {  // Dynamic Call
			mtd = kNameSpace_getMethodNULL(ns, this_cid, 0);
			if(mtd != NULL) {
				return Expr_tyCheckDynamicCallParams(kctx, stmt, expr, mtd, gma, tkMN->text, tkMN->keyword, reqty);
			}
		}
		size_t psize = kArray_size(expr->cons) - 2;
		if(tkMN->keyword == MN_new && psize == 0 && CT_(kExpr_at(expr, 1)->ty)->bcid == TY_Object) {
			//DBG_P("bcid=%s", TY_t(CT_(kExpr_at(expr, 1)->ty)->bcid));
			DBG_ASSERT(kExpr_at(expr, 1)->ty != TY_var);
			return kExpr_at(expr, 1);  // new Person(); // default constructor
		}
		kToken_p(stmt, tkMN, ERR_, "undefined %s: %s.%s%s", MethodType_t(kctx, tkMN->keyword, psize), TY_t(this_cid), KW_t(tkMN->keyword));
	}
	if(mtd != NULL) {
		return Expr_tyCheckCallParams(kctx, stmt, expr, mtd, gma, reqty);
	}
	return K_NULLEXPR;
}

static KMETHOD ExprTyCheck_MethodCall(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kExpr *texpr = kExpr_tyCheckAt(stmt, expr, 1, gma, TY_var, 0);
	if(texpr != K_NULLEXPR) {
		ktype_t this_cid = texpr->ty;
		RETURN_(Expr_lookupMethod(kctx, stmt, expr, this_cid, gma, reqty));
	}
}

static kExpr *Expr_tyCheckFuncParams(KonohaContext *kctx, kStmt *stmt, kExpr *expr, KonohaClass *ct, kGamma *gma);

static kbool_t Expr_isSymbol(kExpr *expr)
{
	return (Expr_isTerm(expr) && (expr->tk->keyword == TK_SYMBOL));
}

static kMethod* Expr_lookUpFuncOrMethod(KonohaContext *kctx, kExpr *exprN, kGamma *gma, ktype_t reqty)
{
	kExpr *expr = kExpr_at(exprN, 0);
	kToken *tk = expr->tk;
	ksymbol_t fn = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NONAME);
	int i;
	GammaAllocaData *genv = gma->genv;
	for(i = genv->l.varsize - 1; i >= 0; i--) {
		if(genv->l.vars[i].fn == fn && TY_isFunc(genv->l.vars[i].ty)) {
			kExpr_setVariable(expr, LOCAL_, genv->l.vars[i].ty, i, gma);
			return NULL;
		}
	}
	for(i = genv->f.varsize - 1; i >= 0; i--) {
		if(genv->f.vars[i].fn == fn && TY_isFunc(genv->l.vars[i].ty)) {
			kExpr_setVariable(expr, LOCAL, genv->f.vars[i].ty, i, gma);
			return NULL;
		}
	}
	if(genv->f.vars[0].ty != TY_void) {
		DBG_ASSERT(genv->this_cid == genv->f.vars[0].ty);
		kMethod *mtd = kNameSpace_getMethodNULL(genv->ns, genv->this_cid, fn);
		if(mtd != NULL) {
			KSETv(exprN->cons->exprs[1], new_Variable(LOCAL, gma->genv->this_cid, 0, gma));
			return mtd;
		}
		KonohaClass *ct = CT_(genv->this_cid);
		if (ct->fsize) {
			for(i = ct->fsize; i >= 0; i--) {
				if(ct->fields[i].fn == fn && TY_isFunc(ct->fields[i].ty)) {
					kExpr_setVariable(expr, FIELD, ct->fields[i].ty, longid((kshort_t)i, 0), gma);
					return NULL;
				}
			}
		}
		mtd = NameSpace_getGetterMethodNULL(kctx, genv->ns, genv->this_cid, fn);
		if(mtd != NULL && TY_isFunc(kMethod_rtype(mtd))) {
			KSETv(exprN->cons->exprs[0], new_GetterExpr(kctx, tk, mtd, new_Variable(LOCAL, genv->this_cid, 0, gma)));
			return NULL;
		}
	}
	{
		ktype_t cid = O_cid(genv->ns->scriptObject);
		kMethod *mtd = kNameSpace_getMethodNULL(genv->ns, cid, fn);
		if(mtd != NULL) {
			KSETv(exprN->cons->exprs[1], new_ConstValue(cid, genv->ns->scriptObject));
			return mtd;
		}
		mtd = NameSpace_getGetterMethodNULL(kctx, genv->ns, cid, fn);
		if(mtd != NULL && TY_isFunc(kMethod_rtype(mtd))) {
			KSETv(exprN->cons->exprs[0], new_GetterExpr(kctx, tk, mtd, new_ConstValue(cid, genv->ns->scriptObject)));
			return NULL;
		}
		mtd = kNameSpace_getMethodNULL(genv->ns, TY_System, fn);
		if(mtd != NULL) {
			KSETv(exprN->cons->exprs[1], new_Variable(NULL, TY_System, 0, gma));
		}
		return mtd;
	}
}

static KMETHOD ExprTyCheck_FuncStyleCall(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	DBG_ASSERT(IS_Expr(kExpr_at(expr, 0)));
	DBG_ASSERT(expr->cons->list[1] == K_NULL);
	if(Expr_isSymbol(kExpr_at(expr, 0))) {
		kMethod *mtd = Expr_lookUpFuncOrMethod(kctx, expr, gma, reqty);
		if(mtd != NULL) {
			RETURN_(Expr_tyCheckCallParams(kctx, stmt, expr, mtd, gma, reqty));
		}
		if(!TY_isFunc(kExpr_at(expr, 0)->ty)) {
			kToken *tk = kExpr_at(expr, 0)->tk;
			RETURN_(kToken_p(stmt, tk, ERR_, "undefined function: %s", kToken_s(tk)));
		}
	}
	else {
		if(Expr_tyCheckAt(kctx, stmt, expr, 0, gma, TY_var, 0) != K_NULLEXPR) {
			if(!TY_isFunc(expr->cons->exprs[0]->ty)) {
				RETURN_(kExpr_p(stmt, expr, ERR_, "function is expected"));
			}
		}
	}
	RETURN_(Expr_tyCheckFuncParams(kctx, stmt, expr, CT_(kExpr_at(expr, 0)->ty), gma));
}

static kExpr *Expr_tyCheckFuncParams(KonohaContext *kctx, kStmt *stmt, kExpr *expr, KonohaClass *ct, kGamma *gma)
{
	ktype_t rtype = ct->p0;
	kParam *pa = CT_cparam(ct);
	size_t i, size = kArray_size(expr->cons);
	if(pa->psize + 2 != size) {
		return kExpr_p(stmt, expr, ERR_, "function %s takes %d parameter(s), but given %d parameter(s)", CT_t(ct), (int)pa->psize, (int)size-2);
	}
	for(i = 0; i < pa->psize; i++) {
		size_t n = i + 2;
		kExpr *texpr = kExpr_tyCheckAt(stmt, expr, n, gma, pa->p[i].ty, 0);
		if(texpr == K_NULLEXPR) {
			return texpr;
		}
	}
	kMethod *mtd = kNameSpace_getMethodNULL(gma->genv->ns, CLASS_Func, MN_("invoke"));
	DBG_ASSERT(mtd != NULL);
	KSETv(expr->cons->exprs[1], expr->cons->exprs[0]);
	return Expr_typedWithMethod(kctx, expr, mtd, rtype);
}


static KMETHOD ExprTyCheck_AND(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	if(kExpr_tyCheckAt(stmt, expr, 1, gma, TY_Boolean, 0) != K_NULLEXPR) {
		if(kExpr_tyCheckAt(stmt, expr, 2, gma, TY_Boolean, 0) != K_NULLEXPR) {
			RETURN_(kExpr_typed(expr, AND, TY_Boolean));
		}
	}
}

static KMETHOD ExprTyCheck_OR(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	if(kExpr_tyCheckAt(stmt, expr, 1, gma, TY_Boolean, 0) != K_NULLEXPR) {
		if(kExpr_tyCheckAt(stmt, expr, 2, gma, TY_Boolean, 0) != K_NULLEXPR) {
			RETURN_(kExpr_typed(expr, OR, TY_Boolean));
		}
	}
}

static KMETHOD StmtTyCheck_Expr(KonohaContext *kctx, KonohaStack *sfp _RIX)  // $expr
{
	VAR_StmtTyCheck(stmt, gma);
	kbool_t r = Stmt_tyCheckExpr(kctx, stmt, KW_ExprPattern, gma, TY_var, TPOL_ALLOWVOID);
	kStmt_typed(stmt, EXPR);
	RETURNb_(r);
}

static int addGammaStack(KonohaContext *kctx, GammaStack *s, ktype_t ty, ksymbol_t fn)
{
	int index = s->varsize;
	if(!(s->varsize < s->capacity)) {
		s->capacity *= 2;
		size_t asize = sizeof(GammaStackDecl) * s->capacity;
		GammaStackDecl *v = (GammaStackDecl*)KMALLOC(asize);
		memcpy(v, s->vars, asize/2);
		if(s->allocsize > 0) {
			KFREE(s->vars, s->allocsize);
		}
		s->vars = v;
		s->allocsize = asize;
	}
	DBG_P("index=%d, ty=%s fn=%s", index, TY_t(ty), SYM_t(fn));
	s->vars[index].ty = ty;
	s->vars[index].fn = fn;
	s->varsize += 1;
	return index;
}

static KMETHOD UndefinedStmtTyCheck(KonohaContext *kctx, KonohaStack *sfp _RIX)  // $expr
{
	VAR_StmtTyCheck(stmt, gma);
	const char *location = kGamma_isTOPLEVEL(gma) ? "at the top level" : "inside the function";
	kStmt_p(stmt, ERR_, "%s%s is not available %s", T_statement(stmt->syn->keyword), location);
	RETURNb_(false);
}

static kbool_t Stmt_TyCheckFunc(KonohaContext *kctx, kFunc *fo, kStmt *stmt, kGamma *gma)
{
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 3);
	KSETv(lsfp[K_CALLDELTA+0].o, (kObject*)fo->self);
	KSETv(lsfp[K_CALLDELTA+1].o, (kObject*)stmt);
	KSETv(lsfp[K_CALLDELTA+2].o, (kObject*)gma);
	KCALL(lsfp, 0, fo->mtd, 3, K_FALSE);
	END_LOCAL();
	return lsfp[0].bvalue;
}

static kbool_t Stmt_TyCheck(KonohaContext *kctx, SugarSyntax *syn, kStmt *stmt, kGamma *gma)
{
	kFunc *fo = kGamma_isTOPLEVEL(gma) ? syn->TopStmtTyCheck : syn->StmtTyCheck;
	kbool_t result;
	if(IS_Array(fo)) { // @Future
		int i;
		kArray *a = (kArray*)fo;
		for(i = kArray_size(a) - 1; i > 0; i--) {
			result = Stmt_TyCheckFunc(kctx, a->funcs[i], stmt, gma);
			if(stmt->syn == NULL) return true;
			if(stmt->build != TSTMT_UNDEFINED) return result;
		}
		fo = a->funcs[0];
	}
	DBG_ASSERT(IS_Func(fo));
	result = Stmt_TyCheckFunc(kctx, fo, stmt, gma);
	if(stmt->syn == NULL) return true; // this means done;
	if(result == false && stmt->build == TSTMT_UNDEFINED) {
		kStmt_p(stmt, ERR_, "statement typecheck error: %s%s", T_statement(syn->keyword));
	}
	return result;
}

static kbool_t Block_tyCheckAll(KonohaContext *kctx, kBlock *bk, kGamma *gma)
{
	int i, result = true, lvarsize = gma->genv->l.varsize;
	for(i = 0; i < kArray_size(bk->stmtList); i++) {
		kStmt *stmt = (kStmt*)bk->stmtList->list[i];
		SugarSyntax *syn = stmt->syn;
		dumpStmt(kctx, stmt);
		if(syn == NULL) continue; /* This means 'done' */
		if(kStmt_isERR(stmt) || !Stmt_TyCheck(kctx, syn, stmt, gma)) {
			DBG_ASSERT(kStmt_isERR(stmt));
			kGamma_setERROR(gma, 1);
			result = false;
			break;
		}
	}
	if(bk != K_NULLBLOCK) {
		kExpr_setVariable(bk->esp, LOCAL_, TY_void, gma->genv->l.varsize, gma);
	}
	if(lvarsize < gma->genv->l.varsize) {
		gma->genv->l.varsize = lvarsize;
	}
	return result;
}

static KMETHOD ExprTyCheck_Block(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kExpr *texpr = K_NULLEXPR;
	kStmt *lastExpr = NULL;
	kfileline_t uline = expr->tk->uline;
	kBlock *bk = expr->block;
	DBG_ASSERT(IS_Block(bk));
	if(kArray_size(bk->stmtList) > 0) {
		kStmt *stmt = bk->stmtList->stmts[kArray_size(bk->stmtList)-1];
		if(stmt->syn->keyword == KW_ExprPattern) {
			lastExpr = stmt;
		}
		uline = stmt->uline;
	}
	if(lastExpr != NULL) {
		int lvarsize = gma->genv->l.varsize;
		size_t i, atop = kArray_size(gma->genv->lvarlst);
		kExpr *lvar = new_Variable(LOCAL_, TY_var, addGammaStack(kctx, &gma->genv->l, TY_var, 0/*FN_*/), gma);
		if(!Block_tyCheckAll(kctx, bk, gma)) {
			RETURN_(texpr);
		}
		kExpr *rexpr = kStmt_expr(lastExpr, KW_ExprPattern, NULL);
		DBG_ASSERT(rexpr != NULL);
		ktype_t ty = rexpr->ty;
		if(ty != TY_void) {
			kExpr *letexpr = new_TypedConsExpr(kctx, TEXPR_LET, TY_void, 3, K_NULL, lvar, rexpr);
			kObject_setObject(lastExpr, KW_ExprPattern, letexpr);
			texpr = kExpr_setVariable(expr, BLOCK_, ty, lvarsize, gma);
		}
		for(i = atop; i < kArray_size(gma->genv->lvarlst); i++) {
			kExprVar *v = gma->genv->lvarlst->Wexprs[i];
			if(v->build == TEXPR_LOCAL_ && v->index >= lvarsize) {
				v->build = TEXPR_STACKTOP; v->index = v->index - lvarsize;
				//DBG_P("v->index=%d", v->index);
			}
		}
		if(lvarsize < gma->genv->l.varsize) {
			gma->genv->l.varsize = lvarsize;
		}
	}
	if(texpr == K_NULLEXPR) {
		kStmt_errline(stmt, uline);
		kStmt_p(stmt, ERR_, "block has no value");
	}
	RETURN_(texpr);
}

static KMETHOD StmtTyCheck_if(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	kbool_t r = 1;
	VAR_StmtTyCheck(stmt, gma);
	if((r = Stmt_tyCheckExpr(kctx, stmt, KW_ExprPattern, gma, TY_Boolean, 0))) {
		kBlock *bkThen = kStmt_block(stmt, KW_BlockPattern, K_NULLBLOCK);
		kBlock *bkElse = kStmt_block(stmt, KW_else, K_NULLBLOCK);
		r = Block_tyCheckAll(kctx, bkThen, gma);
		r = r & Block_tyCheckAll(kctx, bkElse, gma);
		kStmt_typed(stmt, IF);
	}
	RETURNb_(r);
}

static kStmt* Stmt_lookupIfStmtWithoutElse(KonohaContext *kctx, kStmt *stmt)
{
	kBlock *bkElse = kStmt_block(stmt, KW_else, NULL);
	if(bkElse != NULL) {
		if(kArray_size(bkElse->stmtList) == 1) {
			kStmt *stmtIf = bkElse->stmtList->stmts[0];
			if(stmtIf->syn->keyword == KW_if) {
				return Stmt_lookupIfStmtWithoutElse(kctx, stmtIf);
			}
		}
		return NULL;
	}
	return stmt;
}

static kStmt* Stmt_lookupIfStmtNULL(KonohaContext *kctx, kStmt *stmt)
{
	int i;
	kArray *bka = stmt->parentBlockNULL->stmtList;
	kStmt *prevIfStmt = NULL;
	for(i = 0; kArray_size(bka); i++) {
		kStmt *s = bka->stmts[i];
		if(s == stmt) {
			if(prevIfStmt != NULL) {
				return Stmt_lookupIfStmtWithoutElse(kctx, prevIfStmt);
			}
			return NULL;
		}
		if(s->syn == NULL) continue;  // this is done
		prevIfStmt = (s->syn->keyword == KW_if) ? s : NULL;
	}
	return NULL;
}

static KMETHOD StmtTyCheck_else(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	kbool_t r = 1;
	VAR_StmtTyCheck(stmt, gma);
	kStmt *stmtIf = Stmt_lookupIfStmtNULL(kctx, stmt);
	if(stmtIf != NULL) {
		kBlock *bkElse = kStmt_block(stmt, KW_BlockPattern, K_NULLBLOCK);
		kObject_setObject(stmtIf, KW_else, bkElse);
		kStmt_done(stmt);
		r = Block_tyCheckAll(kctx, bkElse, gma);
	}
	else {
		kStmt_p(stmt, ERR_, "else is not statement");
		r = 0;
	}
	RETURNb_(r);
}

static KMETHOD StmtTyCheck_return(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_StmtTyCheck(stmt, gma);
	kbool_t r = 1;
	ktype_t rtype = kMethod_rtype(gma->genv->mtd);
	kStmt_typed(stmt, RETURN);
	if(rtype != TY_void) {
		r = Stmt_tyCheckExpr(kctx, stmt, KW_ExprPattern, gma, rtype, 0);
	} else {
		kExpr *expr = (kExpr*)kObject_getObjectNULL(stmt, KW_ExprPattern);
		if (expr != NULL) {
			kStmt_p(stmt, WARN_, "ignored return value");
			r = Stmt_tyCheckExpr(kctx, stmt, KW_ExprPattern, gma, TY_var, 0);
			kObject_removeKey(stmt, 1);
		}
	}
	RETURNb_(r);
}

///* ------------------------------------------------------------------------ */

static kbool_t ExprTerm_toVariable(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kGamma *gma, ktype_t ty)
{
	if(Expr_isTerm(expr) && expr->tk->keyword == TK_SYMBOL) {
		kToken *tk = expr->tk;
		if(tk->keyword != KW_SymbolPattern) {
			kToken_p(stmt, tk, ERR_, "%s is keyword", S_text(tk->text));
			return false;
		}
		ksymbol_t fn = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NEWID);
		int index = addGammaStack(kctx, &gma->genv->l, ty, fn);
		kExpr_setVariable(expr, LOCAL_, ty, index, gma);
		return true;
	}
	return false;
}

static kbool_t appendAssignmentStmt(KonohaContext *kctx, kExpr *expr, kStmt **lastStmtRef)
{
	kStmt *lastStmt = lastStmtRef[0];
	kStmt *newstmt = new_(Stmt, lastStmt->uline);
	Block_insertAfter(kctx, lastStmt->parentBlockNULL, lastStmt, newstmt);
	kStmt_setsyn(newstmt, SYN_(kStmt_nameSpace(newstmt), KW_ExprPattern));
	kExpr_typed(expr, LET, TY_void);
	kObject_setObject(newstmt, KW_ExprPattern, expr);
	lastStmtRef[0] = newstmt;
	return true;
}

static kbool_t Expr_declType(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kGamma *gma, ktype_t ty, kStmt **lastStmtRef)
{
	DBG_ASSERT(IS_Expr(expr));
	if(Expr_isTerm(expr)) {
		if(ExprTerm_toVariable(kctx, stmt, expr, gma, ty)) {
			kExpr *vexpr = new_Variable(NULL, ty, 0, gma);
			expr = new_TypedConsExpr(kctx, TEXPR_LET, TY_void, 3, K_NULL, expr, vexpr);
			return appendAssignmentStmt(kctx, expr, lastStmtRef);
		}
	}
	else if(expr->syn->keyword == KW_LET) {
		kExpr *lexpr = kExpr_at(expr, 1);
		if(kExpr_tyCheckAt(stmt, expr, 2, gma, TY_var, 0) == K_NULLEXPR) {
			// this is neccesarry to avoid 'int a = a + 1;';
			return false;
		}
		if(ExprTerm_toVariable(kctx, stmt, lexpr, gma, ty)) {
			if(kExpr_tyCheckAt(stmt, expr, 2, gma, ty, 0) != K_NULLEXPR) {
				return appendAssignmentStmt(kctx, expr, lastStmtRef);
			}
			return false;
		}
	} else if(expr->syn->keyword == KW_COMMA) {
		size_t i;
		for(i = 1; i < kArray_size(expr->cons); i++) {
			if(!Expr_declType(kctx, stmt, kExpr_at(expr, i), gma, ty, lastStmtRef)) return false;
		}
		return true;
	}
	kStmt_p(stmt, ERR_, "variable name is expected");
	return false;
}

static KMETHOD StmtTyCheck_TypeDecl(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_StmtTyCheck(stmt, gma);
	kToken *tk  = kStmt_token(stmt, KW_TypePattern, NULL);
	kExpr  *expr = kStmt_expr(stmt, KW_ExprPattern, NULL);
	if(tk == NULL || !TK_isType(tk) || expr == NULL) {
		RETURNb_(false);
	}
	kStmt_done(stmt);
	RETURNb_(Expr_declType(kctx, stmt, expr, gma, TK_type(tk), &stmt));
}

///* ------------------------------------------------------------------------ */
///* [MethodDecl] */

static flagop_t MethodDeclFlag[] = {
	{AKEY("@Virtual"),    kMethod_Virtual},
	{AKEY("@Public"),     kMethod_Public},
	{AKEY("@Const"),      kMethod_Const},
	{AKEY("@Static"),     kMethod_Static},
	{AKEY("@Restricted"), kMethod_Restricted},
	{NULL},
};

static ktype_t Stmt_getcid(KonohaContext *kctx, kStmt *stmt, kNameSpace *ns, ksymbol_t kw, ktype_t defcid)
{
	kToken *tk = (kToken*)kObject_getObjectNULL(stmt, kw);
	if(tk == NULL || !IS_Token(tk)) {
		return defcid;
	}
	else {
		assert(TK_isType(tk));
		return TK_type(tk);
	}
}

static ktype_t Stmt_getmn(KonohaContext *kctx, kStmt *stmt, kNameSpace *ns, ksymbol_t kw, kmethodn_t defmn)
{
	kToken *tk = (kToken*)kObject_getObjectNULL(stmt, kw);
	if(tk == NULL || !IS_Token(tk) || !IS_String(tk->text)) {
		return defmn;
	}
	else {
		DBG_ASSERT(IS_String(tk->text));
		return ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NEWID);
	}
}

static kParam *Stmt_newMethodParamNULL(KonohaContext *kctx, kStmt *stmt, kGamma* gma)
{
	kParam *pa = (kParam*)kObject_getObjectNULL(stmt, KW_ParamsPattern);
	if(pa == NULL || !IS_Param(pa)) {
		SugarSyntax *syn = SYN_(kStmt_nameSpace(stmt), KW_ParamsPattern);
		if(!Stmt_TyCheck(kctx, syn, stmt, gma)) {
			return NULL;
		}
	}
	pa = (kParam*)kObject_getObjectNULL(stmt, KW_ParamsPattern);
	DBG_ASSERT(IS_Param(pa));
	return pa;
}

static kbool_t Method_compile(KonohaContext *kctx, kMethod *mtd, kString *text, kfileline_t uline, kNameSpace *ns);

static KMETHOD Fmethod_lazyCompilation(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	KonohaStack *esp = kctx->esp;
	kMethod *mtd = sfp[K_MTDIDX].mtdNC;
	kString *text = mtd->tcode->text;
	kfileline_t uline = mtd->tcode->uline;
	kNameSpace *ns = mtd->lazyns;
	Method_compile(kctx, mtd, text, uline, ns);
	((KonohaContextVar*)kctx)->esp = esp;
	mtd->fcall_1(kctx, sfp K_RIXPARAM); // call again;
}

static void NameSpace_syncMethods(KonohaContext *kctx)
{
	size_t i, size = kArray_size(ctxsugar->definedMethods);
	for (i = 0; i < size; ++i) {
		kMethod *mtd = ctxsugar->definedMethods->methodList[i];
		if (mtd->fcall_1 == Fmethod_lazyCompilation) {
			kString *text = mtd->tcode->text;
			kfileline_t uline = mtd->tcode->uline;
			kNameSpace *ns = mtd->lazyns;
			Method_compile(kctx, mtd, text, uline, ns);
			assert(mtd->fcall_1 != Fmethod_lazyCompilation);
		}
	}
	kArray_clear(ctxsugar->definedMethods, 0);
}

static void Stmt_setMethodFunc(KonohaContext *kctx, kStmt *stmt, kNameSpace *ns, kMethod *mtd)
{
	kToken *tcode = kStmt_token(stmt, KW_BlockPattern, NULL);
	if(tcode != NULL && tcode->keyword == TK_CODE) {
		KSETv(((kMethodVar*)mtd)->tcode, tcode);  //FIXME
		KSETv(((kMethodVar*)mtd)->lazyns, ns);
		kMethod_setFunc(mtd, Fmethod_lazyCompilation);
		kArray_add(ctxsugar->definedMethods, mtd);
	}
}

static KMETHOD StmtTyCheck_MethodDecl(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_StmtTyCheck(stmt, gma);
	kbool_t r = false;
	kNameSpace *ns = gma->genv->ns;
	uintptr_t flag   =  Stmt_flag(kctx, stmt, MethodDeclFlag, 0);
	ktype_t cid       =  Stmt_getcid(kctx, stmt, ns, KW_UsymbolPattern, O_cid(ns->scriptObject));
	kmethodn_t mn    = Stmt_getmn(kctx, stmt, ns, KW_SymbolPattern, MN_new);
	kParam *pa       = Stmt_newMethodParamNULL(kctx, stmt, gma);
	if(TY_isSingleton(cid)) flag |= kMethod_Static;
	if(pa != NULL) {
		INIT_GCSTACK();
		kMethod *mtd = new_kMethod(flag, cid, mn, NULL);
		PUSH_GCSTACK(mtd);
		kMethod_setParam(mtd, pa->rtype, pa->psize, (kparam_t*)pa->p);
		if(kNameSpace_defineMethod(ns, mtd, stmt->uline)) {
			r = true;
			Stmt_setMethodFunc(kctx, stmt, ns, mtd);
			kStmt_done(stmt);
		}
		RESET_GCSTACK();
	}
	RETURNb_(r);
}

static kbool_t StmtTypeDecl_setParam(KonohaContext *kctx, kStmt *stmt, int n, kparam_t *p)
{
	kToken *tkT  = kStmt_token(stmt, KW_TypePattern, NULL);
	kExpr  *expr = kStmt_expr(stmt, KW_ExprPattern, NULL);
	DBG_ASSERT(tkT != NULL);
	DBG_ASSERT(expr != NULL);
	if(Expr_isTerm(expr) && expr->tk->keyword == TK_SYMBOL) {
		kToken *tkN = expr->tk;
		ksymbol_t fn = ksymbolA(S_text(tkN->text), S_size(tkN->text), SYM_NEWID);
		p[n].fn = fn;
		p[n].ty = TK_type(tkT);
		return true;
	}
	return false;
}

static KMETHOD StmtTyCheck_ParamsDecl(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_StmtTyCheck(stmt, gma);
	kToken *tkT = kStmt_token(stmt, KW_TypePattern, NULL); // type
	ktype_t rtype =  tkT == NULL ? TY_void : TK_type(tkT);
	kParam *pa = NULL;
	kBlock *params = (kBlock*)kObject_getObjectNULL(stmt, KW_ParamsPattern);
	if(params == NULL) {
		pa = new_kParam2(rtype, 0, NULL);
	}
	else if(IS_Block(params)) {
		size_t i, psize = kArray_size(params->stmtList);
		kparam_t p[psize];
		for(i = 0; i < psize; i++) {
			kStmt *stmt = params->stmtList->stmts[i];
			if(stmt->syn->keyword != KW_StmtTypeDecl || !StmtTypeDecl_setParam(kctx, stmt, i, p)) {
				kStmt_p(stmt, ERR_, "parameter declaration must be a $type $name form");
				RETURNb_(false);
			}
		}
		pa = new_kParam2(rtype, psize, p);
	}
	if(IS_Param(pa)) {
		kObject_setObject(stmt, KW_ParamsPattern, pa);
		RETURNb_(true);
	}
	RETURNb_(false);
}

static kBlock* Method_newBlock(KonohaContext *kctx, kMethod *mtd, kString *source, kfileline_t uline)
{
	const char *script = S_text(source);
	if(IS_NULL(source) || script[0] == 0) {
		DBG_ASSERT(IS_Token(mtd->tcode));
		script = S_text(mtd->tcode->text);
		uline = mtd->tcode->uline;
	}
	kArray *tls = ctxsugar->tokens;
	size_t pos = kArray_size(tls);
	NameSpace_tokenize(kctx, KNULL(NameSpace), script, uline, tls); //FIXME: ks
	kBlock *bk = new_Block(kctx, KNULL(NameSpace), NULL, tls, pos, kArray_size(tls), ';');
	kArray_clear(tls, pos);
	return bk;
}

static void Gamma_initParam(KonohaContext *kctx, GammaAllocaData *genv, kParam *pa)
{
	int i, psize = (pa->psize + 1 < genv->f.capacity) ? pa->psize : genv->f.capacity - 1;
	for(i = 0; i < psize; i++) {
		genv->f.vars[i+1].fn = pa->p[i].fn;
		genv->f.vars[i+1].ty = pa->p[i].ty;
	}
	if(!kMethod_isStatic(genv->mtd)) {
		genv->f.vars[0].fn = FN_this;
		genv->f.vars[0].ty = genv->this_cid;
	}
	genv->f.varsize = psize+1;
}

static void Gamma_shiftBlockIndex(KonohaContext *kctx, GammaAllocaData *genv)
{
	kArray *a = genv->lvarlst;
	size_t i, size = kArray_size(a);
	int shift = genv->f.varsize;
	for(i = genv->lvarlst_top; i < size; i++) {
		kExprVar *expr = a->Wexprs[i];
		if(expr->build == TEXPR_STACKTOP) continue;
		//DBG_ASSERT(expr->build < TEXPR_UNTYPED);
		if(expr->build < TEXPR_UNTYPED) {
			expr->index += shift;
			expr->build += TEXPR_shift;
		}
	}
}

static kbool_t Method_compile(KonohaContext *kctx, kMethod *mtd, kString *text, kfileline_t uline, kNameSpace *ns)
{
	INIT_GCSTACK();
	kGamma *gma = ctxsugar->gma;
	kBlock *bk = Method_newBlock(kctx, mtd, text, uline);
	GammaStackDecl fvars[32] = {}, lvars[32] = {};
	GammaAllocaData newgma = {
		.mtd = mtd,
		.ns = ns,
		.this_cid = (mtd)->cid,
		.f.vars = fvars, .f.capacity = 32, .f.varsize = 0, .f.allocsize = 0,
		.l.vars = lvars, .l.capacity = 32, .l.varsize = 0, .l.allocsize = 0,
	};
	GAMMA_PUSH(gma, &newgma);
	Gamma_initParam(kctx, &newgma, kMethod_param(mtd));
	Block_tyCheckAll(kctx, bk, gma);
	Gamma_shiftBlockIndex(kctx, &newgma);
	kMethod_genCode(mtd, bk);
	GAMMA_POP(gma, &newgma);
	RESET_GCSTACK();
	return 1;
}

/* ------------------------------------------------------------------------ */
// eval

static void Gamma_initIt(KonohaContext *kctx, GammaAllocaData *genv, kParam *pa)
{
	KonohaLocalRuntimeVar *base = kctx->stack;
	genv->f.varsize = 0;
	if(base->evalty != TY_void) {
		genv->f.vars[1].fn = FN_("it");
		genv->f.vars[1].ty = base->evalty;
		genv->f.varsize = 1;
	}
}

static kstatus_t Method_runEval(KonohaContext *kctx, kMethod *mtd, ktype_t rtype)
{
	BEGIN_LOCAL(lsfp, K_CALLDELTA);
	KonohaLocalRuntimeVar *base = kctx->stack;
	kstatus_t result = K_CONTINUE;
	//DBG_P("TY=%s, running EVAL..", TY_t(rtype));
	if(base->evalty != TY_void) {
		KSETv(lsfp[K_CALLDELTA+1].o, base->stack[base->evalidx].o);
		lsfp[K_CALLDELTA+1].ivalue = base->stack[base->evalidx].ivalue;
	}
	KCALL(lsfp, 0, mtd, 0, knull(CT_(rtype)));
	base->evalty = rtype;
	base->evalidx = (lsfp - kctx->stack->stack);
	END_LOCAL();
	return result;
}

static ktype_t Stmt_checkReturnType(KonohaContext *kctx, kStmt *stmt)
{
	if(stmt->syn->keyword == KW_ExprPattern) {
		kExpr *expr = (kExpr*)kObject_getObjectNULL(stmt, KW_ExprPattern);
		DBG_ASSERT(expr != NULL);
		if(expr->ty != TY_void) {
			kStmt_setsyn(stmt, SYN_(kStmt_nameSpace(stmt), KW_return));
			kStmt_typed(stmt, RETURN);
			return expr->ty;
		}
	}
	return TY_void;
}

static ktype_t Gamma_evalMethod(KonohaContext *kctx, kGamma *gma, kBlock *bk, kMethod *mtd)
{
	kStmt *stmt = bk->stmtList->stmts[0];
	if(stmt->syn == NULL) {
		kctx->stack->evalty = TY_void;
		return K_CONTINUE;
	}
	if(stmt->syn->keyword == KW_ERR) return K_FAILED;
	ktype_t rtype = Stmt_checkReturnType(kctx, stmt);
	kMethod_genCode(mtd, bk);
	return Method_runEval(kctx, mtd, rtype);
}

static kstatus_t SingleBlock_eval(KonohaContext *kctx, kBlock *bk, kMethod *mtd, kNameSpace *ns)
{
	kstatus_t result;
	kGamma *gma = ctxsugar->gma;
	GammaStackDecl fvars[32] = {}, lvars[32] = {};
	GammaAllocaData newgma = {
		.flag = kGamma_TOPLEVEL,
		.mtd = mtd,
		.ns = ns,
		.this_cid     = TY_System,
		.f.vars = fvars, .f.capacity = 32, .f.varsize = 0, .f.allocsize = 0,
		.l.vars = lvars, .l.capacity = 32, .l.varsize = 0, .l.allocsize = 0,
	};
	GAMMA_PUSH(gma, &newgma);
	Gamma_initIt(kctx, &newgma, kMethod_param(mtd));
	Block_tyCheckAll(kctx, bk, gma);
	if(kGamma_isERROR(gma)) {
		result = K_BREAK;
		kctx->stack->evalty = TY_void;
	}
	else {
		Gamma_shiftBlockIndex(kctx, &newgma);
		result = Gamma_evalMethod(kctx, gma, bk, mtd);
	}
	GAMMA_POP(gma, &newgma);
	return result;
}

static kstatus_t Block_eval(KonohaContext *kctx, kBlock *bk)
{
	INIT_GCSTACK();
	BEGIN_LOCAL(lsfp, 0);
	kBlock *bk1 = ctxsugar->singleBlock;
	kMethod *mtd = new_kMethod(kMethod_Static, 0, 0, NULL);
	PUSH_GCSTACK(mtd);
	kMethod_setParam(mtd, TY_Object, 0, NULL);
	int i, jmpresult;
	kstatus_t result = K_CONTINUE;
	KonohaLocalRuntimeVar *base = kctx->stack;
	jmpbuf_i lbuf = {};
	if(base->evaljmpbuf == NULL) {
		base->evaljmpbuf = (jmpbuf_i*)KCALLOC(sizeof(jmpbuf_i), 1);
	}
	memcpy(&lbuf, base->evaljmpbuf, sizeof(jmpbuf_i));
	if((jmpresult = PLAT setjmp_i(*base->evaljmpbuf)) == 0) {
		for(i = 0; i < kArray_size(bk->stmtList); i++) {
			KSETv(bk1->stmtList->list[0], bk->stmtList->list[i]);
			KSETv(((kBlockVar*)bk1)->blockNameSpace, bk->blockNameSpace);
			kArray_clear(bk1->stmtList, 1);
			result = SingleBlock_eval(kctx, bk1, mtd, bk->blockNameSpace);
			if(result == K_FAILED) break;
		}
	}
	else {
		DBG_P("Catch eval exception jmpresult=%d", jmpresult);
		base->evalty = TY_void;
		result = K_FAILED;
	}
	memcpy(base->evaljmpbuf, &lbuf, sizeof(jmpbuf_i));
	END_LOCAL();
	RESET_GCSTACK();
	return result;
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
