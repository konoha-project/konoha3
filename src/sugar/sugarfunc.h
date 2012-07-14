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
/* Expression TyCheck */

static KMETHOD ExprTyCheck_Text(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->termToken;
	RETURN_(kExpr_setConstValue(expr, TY_String, tk->text));
}

static KMETHOD ExprTyCheck_Type(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	DBG_ASSERT(TK_isType(expr->termToken));
	RETURN_(kExpr_setVariable(expr, NULL, expr->termToken->ty, 0, gma));
}

static KMETHOD ExprTyCheck_true(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	RETURN_(kExpr_setNConstValue(expr, TY_Boolean, (uintptr_t)1));
}

static KMETHOD ExprTyCheck_false(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	RETURN_(kExpr_setNConstValue(expr, TY_Boolean, (uintptr_t)0));
}

static KMETHOD ExprTyCheck_Int(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->termToken;
	long long n = strtoll(S_text(tk->text), NULL, 0);
	RETURN_(kExpr_setNConstValue(expr, TY_Int, (uintptr_t)n));
}

static KMETHOD ExprTyCheck_AND(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	if(kExpr_tyCheckAt(stmt, expr, 1, gma, TY_Boolean, 0) != K_NULLEXPR) {
		if(kExpr_tyCheckAt(stmt, expr, 2, gma, TY_Boolean, 0) != K_NULLEXPR) {
			RETURN_(kExpr_typed(expr, AND, TY_Boolean));
		}
	}
}

static KMETHOD ExprTyCheck_OR(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	if(kExpr_tyCheckAt(stmt, expr, 1, gma, TY_Boolean, 0) != K_NULLEXPR) {
		if(kExpr_tyCheckAt(stmt, expr, 2, gma, TY_Boolean, 0) != K_NULLEXPR) {
			RETURN_(kExpr_typed(expr, OR, TY_Boolean));
		}
	}
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

static KMETHOD ExprTyCheck_Block(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kExpr *texpr = K_NULLEXPR;
	kStmt *lastExpr = NULL;
	kfileline_t uline = stmt->uline;
	kBlock *bk = expr->block;
	DBG_ASSERT(IS_Block(bk));
	if(kArray_size(bk->stmtList) > 0) {
		kStmt *stmt = bk->stmtList->stmtItems[kArray_size(bk->stmtList)-1];
		if(stmt->syn->keyword == KW_ExprPattern) {
			lastExpr = stmt;
		}
		uline = stmt->uline;
	}
	if(lastExpr != NULL) {
		int lvarsize = gma->genv->localScope.varsize;
		//size_t i, atop = kArray_size(gma->genv->lvarlst);
		if(!Block_tyCheckAll(kctx, bk, gma)) {
			RETURN_(texpr);
		}
		kExpr *lvar = new_Variable(LOCAL, TY_var, addGammaStack(kctx, &gma->genv->localScope, TY_var, 0/*FN_*/), gma);
				kExpr *rexpr = kStmt_expr(lastExpr, KW_ExprPattern, NULL);
		DBG_ASSERT(rexpr != NULL);
		ktype_t ty = rexpr->ty;
		if(ty != TY_void) {
			kExpr *letexpr = new_TypedConsExpr(kctx, TEXPR_LET, TY_void, 3, K_NULL, lvar, rexpr);
			KLIB kObject_setObject(kctx, lastExpr, KW_ExprPattern, TY_Expr, letexpr);
			texpr = kExpr_setVariable(expr, BLOCK, ty, lvarsize, gma);
		}
		//         FIXME:
		//             for(i = atop; i < kArray_size(gma->genv->lvarlst); i++) {
			//                     kExprVar *v = gma->genv->lvarlst->exprVarItems[i];
		//                     if(v->build == TEXPR_LOCAL_ && v->index >= lvarsize) {
		//                             v->build = TEXPR_STACKTOP; v->index = v->index - lvarsize;
		//                             //DBG_P("v->index=%d", v->index);
		//                     }
		//             }
		if(lvarsize < gma->genv->localScope.varsize) {
			gma->genv->localScope.varsize = lvarsize;
		}
	}
	if(texpr == K_NULLEXPR) {
		kStmt_errline(stmt, uline);
		kStmt_p(stmt, ErrTag, "block has no value");
	}
	RETURN_(texpr);
}

static kMethod* NameSpace_getGetterMethodNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t cid, ksymbol_t fn)
{
	kMethod *mtd = KLIB kNameSpace_getMethodNULL(kctx, ns, cid, MN_toGETTER(fn));
	if(mtd == NULL) {
		mtd = KLIB kNameSpace_getMethodNULL(kctx, ns, cid, MN_toISBOOL(fn));
	}
	return mtd;
}

static kExpr* new_GetterExpr(KonohaContext *kctx, kToken *tkU, kMethod *mtd, kExpr *expr)
{
	kExprVar *expr1 = (kExprVar *)new_TypedConsExpr(kctx, TEXPR_CALL, kMethod_rtype(mtd), 2, mtd, expr);
	//KSETv(expr1->tk, tkU); // for uline
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
	kToken *tk = expr->termToken;
	ksymbol_t fn = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NONAME);
	int i;
	GammaAllocaData *genv = gma->genv;
	for(i = genv->localScope.varsize - 1; i >= 0; i--) {
		if(genv->localScope.vars[i].fn == fn) {
			return kExpr_setVariable(expr, LOCAL, genv->localScope.vars[i].ty, i, gma);
		}
	}
	if(genv->localScope.vars[0].ty != TY_void) {
		DBG_ASSERT(genv->this_cid == genv->localScope.vars[0].ty);
		KonohaClass *ct = CT_(genv->this_cid);
		for(i = ct->fsize; i >= 0; i--) {
			if(ct->fieldItems[i].fn == fn && ct->fieldItems[i].ty != TY_void) {
				return kExpr_setVariable(expr, FIELD, ct->fieldItems[i].ty, longid((kshort_t)i, 0), gma);
			}
		}
		kMethod *mtd = NameSpace_getGetterMethodNULL(kctx, genv->ns, genv->this_cid, fn);
		if(mtd != NULL) {
			return new_GetterExpr(kctx, tk, mtd, new_Variable(LOCAL, genv->this_cid, 0, gma));
		}
//		mtd = KLIB kNameSpace_getMethodNULL(kctx, genv->ns, genv->this_cid, fn);
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
		mtd = KLIB kNameSpace_getMethodNULL(kctx, genv->ns, cid, fn);
		if(mtd != NULL) {
			kParam *pa = kMethod_param(mtd);
			KonohaClass *ct = KLIB KonohaClass_Generics(kctx, CT_Func, pa->rtype, pa->psize, (kparam_t*)pa->p);
			kFuncVar *fo = (kFuncVar*)KLIB new_kObjectOnGCSTACK(kctx, ct, (uintptr_t)mtd);
			KSETv(fo->self, genv->ns->scriptObject);
			return new_ConstValue(ct->cid, fo);
		}
	}
	if(fn != SYM_NONAME) {
		KUtilsKeyValue *kv = NameSpace_getConstNULL(kctx, gma->genv->ns, fn);
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
	kExpr *texpr = (v == NULL) ? kToken_p(stmt, tk, ErrTag, "undefined name: %s", kToken_s(tk)) : kExpr_setConstValue(expr, O_cid(v), v);
	return texpr;
}

static KMETHOD ExprTyCheck_Symbol(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	RETURN_(Expr_tyCheckVariable2(kctx, stmt, expr, gma, reqty));
}

static KMETHOD ExprTyCheck_Usymbol(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->termToken;
	ksymbol_t ukey = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NONAME);
	if(ukey != SYM_NONAME) {
		KUtilsKeyValue *kv = NameSpace_getConstNULL(kctx, gma->genv->ns, ukey);
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
	kExpr *texpr = (v == NULL) ? kToken_p(stmt, tk, ErrTag, "undefined name: %s", kToken_s(tk)) : kExpr_setConstValue(expr, O_cid(v), v);
	RETURN_(texpr);
}

static KMETHOD StmtTyCheck_ConstDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_StmtTyCheck(stmt, gma);
	kbool_t r = false;
	kNameSpace *ns = gma->genv->ns;
	kToken *tk = kStmt_token(stmt, KW_UsymbolPattern, NULL);
	ksymbol_t ukey = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NEWID);
	KUtilsKeyValue *kv = NameSpace_getConstNULL(kctx, ns, ukey);
	if(kv != NULL) {
		kStmt_p(stmt, ErrTag, "already defined name: %s", kToken_s(tk));
	}
	else {
		r = Stmt_tyCheckExpr(kctx, stmt, KW_ExprPattern, gma, TY_var, TPOL_CONST);
		if(r) {
			kExpr *expr = kStmt_expr(stmt, KW_ExprPattern, NULL);
			KUtilsKeyValue kv = { ukey, expr->ty};
			if(expr->build == TEXPR_NULL) {
				kv.ty = TY_TYPE;
				kv.uval = (uintptr_t)(CT_(expr->ty));
				expr = NULL;
			}
			else if(expr->build == TEXPR_CONST) {
				kv.key = ukey | SYMKEY_BOXED;
				kv.oval = expr->objectConstValue;
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
				kStmt_p(stmt, ErrTag, "constant value is expected");
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
//			kMethod *mtd = a->methodItems[i];
//			if(mtd->cid == cid && mtd->mn == mn) {
//				KLIB kArray_add(kctx, abuf, mtd);
//			}
//		}
//		ks = ks->parentNULL;
//	}
//	KonohaClass *ct = CT_(cid);
//	while(ct != NULL) {
//		size_t i;
//		kArray *a = ct->methodList;
//		for(i = 0; i < kArray_size(a); i++) {
//			kMethod *mtd = a->methodItems[i];
//			if(mtd->mn == mn) {
//				KLIB kArray_add(kctx, abuf, mtd);
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
//		kParam *pa = kMethod_param(a->methodItems[i]);
//		if(pa->psize == psize && kParam_equals(kctx, pa, thisct, psize, p)) {
//			return a->methodItems[i];
//		}
//	}
//	for(i = s; i < e; i++) {
//		kParam *pa = kMethod_param(a->methodItems[i]);
//		if(kParam_match(kctx, pa, thisct, psize, p)) {
//			return a->methodItems[i];
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
//		p[i].ty = expr->cons->exprItems[i+2]->ty;
//	}
//	KonohaClass *ct = thisct;
//	while(1) {
//		NameSpace_lookupMethods(kctx, gma->genv->ns, ct->cid, mtd->mn, abuf);
//		kMethod *mtd2 = Array_lookupMethod(kctx, abuf, atop, kArray_size(abuf), thisct, psize, p);
//		KLIB kArray_clear(kctx, abuf, atop);
//		if(mtd2 != NULL) return mtd2;
//		if(ct->cid == TY_Object) break;
//		ct = CT_(ct->supcid);
//	}
//	return mtd;
//}

static kExpr* Expr_typedWithMethod(KonohaContext *kctx, kExpr *expr, kMethod *mtd, ktype_t reqty)
{
	kExpr *expr1 = kExpr_at(expr, 1);
	KSETv(expr->cons->methodItems[0], mtd);
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
	kExpr *expr1 = cons->exprItems[1];
	KonohaClass *this_ct = CT_(expr1->ty);
	DBG_ASSERT(IS_Method(mtd));
	DBG_ASSERT(this_ct->cid != TY_var);
	if(!TY_isUnbox(mtd->cid) && CT_isUnbox(this_ct)) {
		expr1 = new_BoxingExpr(kctx, cons->exprItems[1], this_ct->cid);
		KSETv(cons->exprItems[1], expr1);
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
		return kExpr_p(stmt, expr, ErrTag, "%s.%s%s takes %d parameter(s), but given %d parameter(s)", CT_t(this_ct), T_mn(mtd->mn), (int)pa->psize, (int)size-2);
	}
	for(i = 0; i < pa->psize; i++) {
		size_t n = i + 2;
		ktype_t ptype = ktype_var(kctx, pa->p[i].ty, this_ct);
		int pol = param_policy(pa->p[i].fn);
		kExpr *texpr = kExpr_tyCheckAt(stmt, expr, n, gma, ptype, pol);
		if(texpr == K_NULLEXPR) {
			return kExpr_p(stmt, expr, ErrTag, "%s.%s%s accepts %s at the parameter %d", CT_t(this_ct), T_mn(mtd->mn), TY_t(ptype), (int)i+1);
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
	kTokenVar *tkMN = expr->cons->tokenVarItems[0];
	DBG_ASSERT(IS_Token(tkMN));
	if(tkMN->keyword == TK_SYMBOL) {
		tkMN->keyword = ksymbolA(S_text(tkMN->text), S_size(tkMN->text), SYM_NEWID);
	}
	kMethod *mtd = KLIB kNameSpace_getMethodNULL(kctx, ns, this_cid, tkMN->keyword);
	if(mtd == NULL) {
		if(tkMN->text != TS_EMPTY) {  // Dynamic Call
			mtd = KLIB kNameSpace_getMethodNULL(kctx, ns, this_cid, 0);
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
		kToken_p(stmt, tkMN, ErrTag, "undefined %s: %s.%s%s", MethodType_t(kctx, tkMN->keyword, psize), TY_t(this_cid), KW_t(tkMN->keyword));
	}
	if(mtd != NULL) {
		return Expr_tyCheckCallParams(kctx, stmt, expr, mtd, gma, reqty);
	}
	return K_NULLEXPR;
}

static KMETHOD ExprTyCheck_MethodCall(KonohaContext *kctx, KonohaStack *sfp)
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
	return (Expr_isTerm(expr) && (expr->termToken->keyword == TK_SYMBOL));
}

static kMethod* Expr_lookUpFuncOrMethod(KonohaContext *kctx, kExpr *exprN, kGamma *gma, ktype_t reqty)
{
	kExpr *expr = kExpr_at(exprN, 0);
	kToken *tk = expr->termToken;
	ksymbol_t fn = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NONAME);
	int i;
	GammaAllocaData *genv = gma->genv;
	for(i = genv->localScope.varsize - 1; i >= 0; i--) {
		if(genv->localScope.vars[i].fn == fn && TY_isFunc(genv->localScope.vars[i].ty)) {
			kExpr_setVariable(expr, LOCAL, genv->localScope.vars[i].ty, i, gma);
			return NULL;
		}
	}
	if(genv->localScope.vars[0].ty != TY_void) {
		DBG_ASSERT(genv->this_cid == genv->localScope.vars[0].ty);
		kMethod *mtd = KLIB kNameSpace_getMethodNULL(kctx, genv->ns, genv->this_cid, fn);
		if(mtd != NULL) {
			KSETv(exprN->cons->exprItems[1], new_Variable(LOCAL, gma->genv->this_cid, 0, gma));
			return mtd;
		}
		KonohaClass *ct = CT_(genv->this_cid);
		if (ct->fsize) {
			for(i = ct->fsize; i >= 0; i--) {
				if(ct->fieldItems[i].fn == fn && TY_isFunc(ct->fieldItems[i].ty)) {
					kExpr_setVariable(expr, FIELD, ct->fieldItems[i].ty, longid((kshort_t)i, 0), gma);
					return NULL;
				}
			}
		}
		mtd = NameSpace_getGetterMethodNULL(kctx, genv->ns, genv->this_cid, fn);
		if(mtd != NULL && TY_isFunc(kMethod_rtype(mtd))) {
			KSETv(exprN->cons->exprItems[0], new_GetterExpr(kctx, tk, mtd, new_Variable(LOCAL, genv->this_cid, 0, gma)));
			return NULL;
		}
	}
	{
		ktype_t cid = O_cid(genv->ns->scriptObject);
		kMethod *mtd = KLIB kNameSpace_getMethodNULL(kctx, genv->ns, cid, fn);
		if(mtd != NULL) {
			KSETv(exprN->cons->exprItems[1], new_ConstValue(cid, genv->ns->scriptObject));
			return mtd;
		}
		mtd = NameSpace_getGetterMethodNULL(kctx, genv->ns, cid, fn);
		if(mtd != NULL && TY_isFunc(kMethod_rtype(mtd))) {
			KSETv(exprN->cons->exprItems[0], new_GetterExpr(kctx, tk, mtd, new_ConstValue(cid, genv->ns->scriptObject)));
			return NULL;
		}
		mtd = KLIB kNameSpace_getMethodNULL(kctx, genv->ns, TY_System, fn);
		if(mtd != NULL) {
			KSETv(exprN->cons->exprItems[1], new_Variable(NULL, TY_System, 0, gma));
		}
		return mtd;
	}
}

static KMETHOD ExprTyCheck_FuncStyleCall(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	DBG_ASSERT(IS_Expr(kExpr_at(expr, 0)));
	DBG_ASSERT(expr->cons->objectItems[1] == K_NULL);
	if(Expr_isSymbol(kExpr_at(expr, 0))) {
		kMethod *mtd = Expr_lookUpFuncOrMethod(kctx, expr, gma, reqty);
		if(mtd != NULL) {
			RETURN_(Expr_tyCheckCallParams(kctx, stmt, expr, mtd, gma, reqty));
		}
		if(!TY_isFunc(kExpr_at(expr, 0)->ty)) {
			kToken *tk = kExpr_at(expr, 0)->termToken;
			DBG_ASSERT(IS_Token(tk));  // TODO: make error message in case of not Token
			RETURN_(kToken_p(stmt, tk, ErrTag, "undefined function: %s", kToken_s(tk)));
		}
	}
	else {
		if(Expr_tyCheckAt(kctx, stmt, expr, 0, gma, TY_var, 0) != K_NULLEXPR) {
			if(!TY_isFunc(expr->cons->exprItems[0]->ty)) {
				RETURN_(kExpr_p(stmt, expr, ErrTag, "function is expected"));
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
		return kExpr_p(stmt, expr, ErrTag, "function %s takes %d parameter(s), but given %d parameter(s)", CT_t(ct), (int)pa->psize, (int)size-2);
	}
	for(i = 0; i < pa->psize; i++) {
		size_t n = i + 2;
		kExpr *texpr = kExpr_tyCheckAt(stmt, expr, n, gma, pa->p[i].ty, 0);
		if(texpr == K_NULLEXPR) {
			return texpr;
		}
	}
	kMethod *mtd = KLIB kNameSpace_getMethodNULL(kctx, gma->genv->ns, TY_Func, MN_("invoke"));
	DBG_ASSERT(mtd != NULL);
	KSETv(expr->cons->exprItems[1], expr->cons->exprItems[0]);
	return Expr_typedWithMethod(kctx, expr, mtd, rtype);
}

// ---------------------------------------------------------------------------
// Statement Expr

static KMETHOD StmtTyCheck_Expr(KonohaContext *kctx, KonohaStack *sfp)  // $expr
{
	VAR_StmtTyCheck(stmt, gma);
	kbool_t r = Stmt_tyCheckExpr(kctx, stmt, KW_ExprPattern, gma, TY_var, TPOL_ALLOWVOID);
	kStmt_typed(stmt, EXPR);
	RETURNb_(r);
}

static KMETHOD StmtTyCheck_if(KonohaContext *kctx, KonohaStack *sfp)
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
			kStmt *stmtIf = bkElse->stmtList->stmtItems[0];
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
		kStmt *s = bka->stmtItems[i];
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

static KMETHOD StmtTyCheck_else(KonohaContext *kctx, KonohaStack *sfp)
{
	kbool_t r = 1;
	VAR_StmtTyCheck(stmt, gma);
	kStmt *stmtIf = Stmt_lookupIfStmtNULL(kctx, stmt);
	if(stmtIf != NULL) {
		kBlock *bkElse = kStmt_block(stmt, KW_BlockPattern, K_NULLBLOCK);
		KLIB kObject_setObject(kctx, stmtIf, KW_else, TY_Block, bkElse);
		kStmt_done(stmt);
		r = Block_tyCheckAll(kctx, bkElse, gma);
	}
	else {
		kStmt_p(stmt, ErrTag, "else is not statement");
		r = 0;
	}
	RETURNb_(r);
}

static KMETHOD StmtTyCheck_return(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_StmtTyCheck(stmt, gma);
	kbool_t r = 1;
	ktype_t rtype = kMethod_rtype(gma->genv->mtd);
	kStmt_typed(stmt, RETURN);
	if(rtype != TY_void) {
		r = Stmt_tyCheckExpr(kctx, stmt, KW_ExprPattern, gma, rtype, 0);
	} else {
		kExpr *expr = (kExpr*)kStmt_getObjectNULL(kctx, stmt, KW_ExprPattern);
		if (expr != NULL) {
			kStmt_p(stmt, WarnTag, "ignored return value");
			r = Stmt_tyCheckExpr(kctx, stmt, KW_ExprPattern, gma, TY_var, 0);
			KLIB kObject_removeKey(kctx, stmt, 1);
		}
	}
	RETURNb_(r);
}

///* ------------------------------------------------------------------------ */

static kbool_t ExprTerm_toVariable(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kGamma *gma, ktype_t ty)
{
	if(Expr_isTerm(expr) && expr->termToken->keyword == TK_SYMBOL) {
		kToken *tk = expr->termToken;
		if(tk->keyword != KW_SymbolPattern) {
			kToken_p(stmt, tk, ErrTag, "%s is keyword", S_text(tk->text));
			return false;
		}
		ksymbol_t fn = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NEWID);
		int index = addGammaStack(kctx, &gma->genv->localScope, ty, fn);
		kExpr_setVariable(expr, LOCAL, ty, index, gma);
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
	KLIB kObject_setObject(kctx, newstmt, KW_ExprPattern, TY_Expr, expr);
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
	kStmt_p(stmt, ErrTag, "variable name is expected");
	return false;
}

static KMETHOD StmtTyCheck_TypeDecl(KonohaContext *kctx, KonohaStack *sfp)
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
	kToken *tk = (kToken*)kStmt_getObjectNULL(kctx, stmt, kw);
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
	kToken *tk = (kToken*)kStmt_getObjectNULL(kctx, stmt, kw);
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
	kParam *pa = (kParam*)kStmt_getObjectNULL(kctx, stmt, KW_ParamsPattern);
	if(pa == NULL || !IS_Param(pa)) {
		SugarSyntax *syn = SYN_(kStmt_nameSpace(stmt), KW_ParamsPattern);
		if(!Stmt_TyCheck(kctx, syn, stmt, gma)) {
			return NULL;
		}
	}
	pa = (kParam*)kStmt_getObjectNULL(kctx, stmt, KW_ParamsPattern);
	DBG_ASSERT(IS_Param(pa));
	return pa;
}

static kbool_t Method_compile(KonohaContext *kctx, kMethod *mtd, kString *text, kfileline_t uline, kNameSpace *ns);

static KMETHOD MethodFunc_lazyCompilation(KonohaContext *kctx, KonohaStack *sfp)
{
	KonohaStack *esp = kctx->esp;
	kMethod *mtd = sfp[K_MTDIDX].mtdNC;
	kString *text = mtd->tcode->text;
	kfileline_t uline = mtd->tcode->uline;
	kNameSpace *ns = mtd->lazyns;
	Method_compile(kctx, mtd, text, uline, ns);
	((KonohaContextVar*)kctx)->esp = esp;
	mtd->invokeMethodFunc(kctx, sfp); // call again;
}

static void kNameSpace_syncMethods(KonohaContext *kctx)
{
	size_t i, size = kArray_size(ctxsugar->definedMethodList);
	for (i = 0; i < size; ++i) {
		kMethod *mtd = ctxsugar->definedMethodList->methodItems[i];
		if (mtd->invokeMethodFunc == MethodFunc_lazyCompilation) {
			kString *text = mtd->tcode->text;
			kfileline_t uline = mtd->tcode->uline;
			kNameSpace *ns = mtd->lazyns;
			Method_compile(kctx, mtd, text, uline, ns);
			assert(mtd->invokeMethodFunc != MethodFunc_lazyCompilation);
		}
	}
	KLIB kArray_clear(kctx, ctxsugar->definedMethodList, 0);
}

static void Stmt_setMethodFunc(KonohaContext *kctx, kStmt *stmt, kNameSpace *ns, kMethod *mtd)
{
	kToken *tcode = kStmt_token(stmt, KW_BlockPattern, NULL);
	if(tcode != NULL && tcode->keyword == TK_CODE) {
		KSETv(((kMethodVar*)mtd)->tcode, tcode);  //FIXME
		KSETv(((kMethodVar*)mtd)->lazyns, ns);
		KLIB kMethod_setFunc(kctx, mtd, MethodFunc_lazyCompilation);
		KLIB kArray_add(kctx, ctxsugar->definedMethodList, mtd);
	}
}

static KMETHOD StmtTyCheck_MethodDecl(KonohaContext *kctx, KonohaStack *sfp)
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
		kMethod *mtd = KLIB new_kMethod(kctx, flag, cid, mn, NULL);
		PUSH_GCSTACK(mtd);
		KLIB kMethod_setParam(kctx, mtd, pa->rtype, pa->psize, (kparam_t*)pa->p);
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
	if(Expr_isTerm(expr) && expr->termToken->keyword == TK_SYMBOL) {
		kToken *tkN = expr->termToken;
		ksymbol_t fn = ksymbolA(S_text(tkN->text), S_size(tkN->text), SYM_NEWID);
		p[n].fn = fn;
		p[n].ty = TK_type(tkT);
		return true;
	}
	return false;
}

static KMETHOD StmtTyCheck_ParamsDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_StmtTyCheck(stmt, gma);
	kToken *tkT = kStmt_token(stmt, KW_TypePattern, NULL); // type
	ktype_t rtype =  tkT == NULL ? TY_void : TK_type(tkT);
	kParam *pa = NULL;
	kBlock *params = (kBlock*)kStmt_getObjectNULL(kctx, stmt, KW_ParamsPattern);
	if(params == NULL) {
		pa = new_kParam(kctx, rtype, 0, NULL);
	}
	else if(IS_Block(params)) {
		size_t i, psize = kArray_size(params->stmtList);
		kparam_t p[psize];
		for(i = 0; i < psize; i++) {
			kStmt *stmt = params->stmtList->stmtItems[i];
			if(stmt->syn->keyword != KW_StmtTypeDecl || !StmtTypeDecl_setParam(kctx, stmt, i, p)) {
				kStmt_p(stmt, ErrTag, "parameter declaration must be a $type $name form");
				RETURNb_(false);
			}
		}
		pa = new_kParam(kctx, rtype, psize, p);
	}
	if(IS_Param(pa)) {
		KLIB kObject_setObject(kctx, stmt, KW_ParamsPattern, TY_Param, pa);
		RETURNb_(true);
	}
	RETURNb_(false);
}

/* ------------------------------------------------------------------------ */

#define PATTERN(T)  .keyword = KW_##T##Pattern
#define TOKEN(T)    .keyword = KW_##T

static void defineDefaultSyntax(KonohaContext *kctx, kNameSpace *ns)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ TOKEN(ERR), .flag = SYNFLAG_StmtBreakExec, },
		{ PATTERN(Expr), .rule ="$expr", PatternMatch_(Expr), TopStmtTyCheck_(Expr), StmtTyCheck_(Expr),  },
		{ PATTERN(Symbol),  _TERM, PatternMatch_(Symbol),  ExprTyCheck_(Symbol),},
		{ PATTERN(Usymbol), _TERM, PatternMatch_(Usymbol), /* .rule = "$USYMBOL \"=\" $expr",*/ TopStmtTyCheck_(ConstDecl), ExprTyCheck_(Usymbol),},
		{ PATTERN(Text), _TERM, ExprTyCheck_(Text),},
		{ PATTERN(Int), _TERM, ExprTyCheck_(Int),},
		{ PATTERN(Float), _TERM, },
		{ PATTERN(Type), _TERM, PatternMatch_(Type), .rule = "$type $expr", StmtTyCheck_(TypeDecl), ExprTyCheck_(Type), },
		{ PATTERN(Parenthesis), .flag = SYNFLAG_ExprPostfixOp2, ParseExpr_(Parenthesis), .priority_op2 = 16, ExprTyCheck_(FuncStyleCall),}, //AST_PARENTHESIS
		{ PATTERN(Bracket),  },  //AST_BRACKET
		{ PATTERN(Brace),  }, // AST_BRACE
		{ PATTERN(Block), PatternMatch_(Block), ExprTyCheck_(Block), },
		{ PATTERN(Params), PatternMatch_(Params), TopStmtTyCheck_(ParamsDecl), ExprTyCheck_(MethodCall),},
		{ PATTERN(Toks), PatternMatch_(Toks), },
		{ TOKEN(DOT), ParseExpr_(DOT), .priority_op2 = 16, },
		{ TOKEN(DIV), _OP, .op2 = "opDIV", .priority_op2 = 32, },
		{ TOKEN(MOD), _OP, .op2 = "opMOD", .priority_op2 = 32, },
		{ TOKEN(MUL), _OP, .op2 = "opMUL", .priority_op2 = 32, },
		{ TOKEN(ADD), _OP, .op1 = "opPLUS", .op2 = "opADD", .priority_op2 = 64, },
		{ TOKEN(SUB), _OP, .op1 = "opMINUS", .op2 = "opSUB", .priority_op2 = 64, },
		{ TOKEN(LT), _OP, .op2 = "opLT", .priority_op2 = 256, },
		{ TOKEN(LTE), _OP, .op2 = "opLTE", .priority_op2 = 256, },
		{ TOKEN(GT), _OP, .op2 = "opGT", .priority_op2 = 256, },
		{ TOKEN(GTE), _OP, .op2 = "opGTE", .priority_op2 = 256, },
		{ TOKEN(EQ), _OP, .op2 = "opEQ", .priority_op2 = 512, },
		{ TOKEN(NEQ), _OP, .op2 = "opNEQ", .priority_op2 = 512, },
		{ TOKEN(AND), _OP, /*.op2 = ""unused*/ .priority_op2 = 1024, ExprTyCheck_(AND)},
		{ TOKEN(OR), _OP, /*.op2 = ""unused*/ .priority_op2 = 2048, ExprTyCheck_(OR)},
		{ TOKEN(NOT), _OP, .op1 = "opNOT", },
//		{ TOKEN(":"),  _OP,  .priority_op2 = 3072,},
		{ TOKEN(LET),  _OPLeft, /*.op2 = "*"*/ .priority_op2 = 4096, },
		{ TOKEN(COMMA), ParseExpr_(COMMA), .op2 = "*", .priority_op2 = 8192, /*.flag = SYNFLAG_ExprLeftJoinOP2,*/ },
		{ TOKEN(DOLLAR), ParseExpr_(DOLLAR), },
		{ TOKEN(void), .type = TY_void, .rule ="$type [$type \".\"] $SYMBOL $params [$block]", TopStmtTyCheck_(MethodDecl)},
		{ TOKEN(boolean), .type = TY_Boolean, },
		{ TOKEN(int),     .type = TY_Int, },
		{ TOKEN(true),  _TERM, ExprTyCheck_(true),},
		{ TOKEN(false),  _TERM, ExprTyCheck_(false),},
		{ TOKEN(if), .rule ="\"if\" \"(\" $expr \")\" $block [\"else\" else: $block]", TopStmtTyCheck_(if), StmtTyCheck_(if), },
		{ TOKEN(else), .rule = "\"else\" $block", TopStmtTyCheck_(else), StmtTyCheck_(else), },
		{ TOKEN(return), .rule ="\"return\" [$expr]", .flag = SYNFLAG_StmtBreakExec, StmtTyCheck_(return), },
		{ .keyword = KW_END, },
	};
	NameSpace_defineSyntax(kctx, ns, SYNTAX);
	SugarSyntaxVar *syn = (SugarSyntaxVar*)SYN_(ns, KW_void);
	syn->ty = TY_void; // it's not cool, but necessary
	syn = (SugarSyntaxVar*)SYN_(ns, KW_UsymbolPattern);
	KINITv(syn->syntaxRuleNULL, new_(TokenArray, 0));
	parseSyntaxRule(kctx, "$USYMBOL \"=\" $expr", 0, syn->syntaxRuleNULL);
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
