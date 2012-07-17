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

static kString *resolveEscapeSequence(KonohaContext *kctx, kString *s, size_t start)
{
	const char *text = S_text(s) + start;
	const char *end  = S_text(s) + S_size(s);
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	KLIB Kwb_write(kctx, &wb, S_text(s), start);
	while (text < end) {
		int ch = *text;
		if(ch == '\\' && *(text+1) != '\0') {
			switch (*(text+1)) {
			case 'n':  ch = '\n'; text++; break;
			case 't':  ch = '\t'; text++; break;
			case 'r':  ch = '\r'; text++; break;
			case '\\': ch = '\\'; text++; break;
			case '"':  ch = '\"'; text++; break;
			}
		}
		kwb_putc(&wb, ch);
		text++;
	}
	s = KLIB new_kString(kctx, KLIB Kwb_top(kctx, &wb, 1), Kwb_bytesize(&wb), 0);
	KLIB Kwb_free(&wb);
	return s;
}


static KMETHOD ExprTyCheck_Text(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->termToken;
	kString *text = tk->text;
	size_t i, size = S_size(text);
	for(i = 0; i < size; i++) {
		int ch = text->buf[i];
		if(ch == '\\') {
			text = resolveEscapeSequence(kctx, text, i);
			break;
		}
	}
	RETURN_(SUGAR kExpr_setConstValue(kctx, expr, TY_String, UPCAST(text)));
}

static KMETHOD ExprTyCheck_Type(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	DBG_ASSERT(TK_isType(expr->termToken));
	RETURN_(SUGAR kExpr_setVariable(kctx, expr, gma, TEXPR_NULL, expr->termToken->ty, 0));
}

static KMETHOD ExprTyCheck_true(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	RETURN_(SUGAR kExpr_setUnboxConstValue(kctx, expr, TY_Boolean, (uintptr_t)1));
}

static KMETHOD ExprTyCheck_false(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	RETURN_(SUGAR kExpr_setUnboxConstValue(kctx, expr, TY_Boolean, (uintptr_t)0));
}

static KMETHOD ExprTyCheck_Int(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->termToken;
	long long n = strtoll(S_text(tk->text), NULL, 0);
	RETURN_(SUGAR kExpr_setUnboxConstValue(kctx, expr, TY_Int, (uintptr_t)n));
}

static KMETHOD ExprTyCheck_AND(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	if(SUGAR kkStmt_tyCheckByNameAt(kctx, stmt, expr, 1, gma, TY_Boolean, 0) != K_NULLEXPR) {
		if(SUGAR kkStmt_tyCheckByNameAt(kctx, stmt, expr, 2, gma, TY_Boolean, 0) != K_NULLEXPR) {
			RETURN_(kExpr_typed(expr, AND, TY_Boolean));
		}
	}
}

static KMETHOD ExprTyCheck_OR(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	if(SUGAR kkStmt_tyCheckByNameAt(kctx, stmt, expr, 1, gma, TY_Boolean, 0) != K_NULLEXPR) {
		if(SUGAR kkStmt_tyCheckByNameAt(kctx, stmt, expr, 2, gma, TY_Boolean, 0) != K_NULLEXPR) {
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
		memcpy(v, s->varItems, asize/2);
		if(s->allocsize > 0) {
			KFREE(s->varItems, s->allocsize);
		}
		s->varItems = v;
		s->allocsize = asize;
	}
	DBG_P("index=%d, ty=%s fn=%s", index, TY_t(ty), SYM_t(fn));
	s->varItems[index].ty = ty;
	s->varItems[index].fn = fn;
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
		int popBlockScopeShiftSize = gma->genv->blockScopeShiftSize;
		gma->genv->blockScopeShiftSize = lvarsize;
		if(!kBlock_tyCheckAll(kctx, bk, gma)) {
			RETURN_(texpr);
		}
		kExpr *lvar = new_VariableExpr(kctx, gma, TEXPR_LOCAL, TY_var, addGammaStack(kctx, &gma->genv->localScope, TY_var, 0/*FN_*/));
		kExpr *rexpr = SUGAR kStmt_getExpr(kctx, lastExpr, KW_ExprPattern, NULL);
		DBG_ASSERT(rexpr != NULL);
		ktype_t ty = rexpr->ty;
		if(ty != TY_void) {
			kExpr *letexpr = new_TypedConsExpr(kctx, TEXPR_LET, TY_void, 3, K_NULL, lvar, rexpr);
			KLIB kObject_setObject(kctx, lastExpr, KW_ExprPattern, TY_Expr, letexpr);
			texpr = SUGAR kExpr_setVariable(kctx, expr, gma, TEXPR_BLOCK, ty, lvarsize);
		}
		gma->genv->blockScopeShiftSize = popBlockScopeShiftSize;
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
	return KLIB kNameSpace_getMethodNULL(kctx, ns, cid, MN_toGETTER(fn), 0, MPOL_GETTER);
}

static kExpr* new_GetterExpr(KonohaContext *kctx, kToken *tkU, kMethod *mtd, kExpr *expr)
{
	kExprVar *expr1 = (kExprVar *)new_TypedConsExpr(kctx, TEXPR_CALL, Method_returnType(mtd), 2, mtd, expr);
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
	kNameSpace *ns = Stmt_nameSpace(stmt);
	for(i = genv->localScope.varsize - 1; i >= 0; i--) {
		if(genv->localScope.varItems[i].fn == fn) {
			return SUGAR kExpr_setVariable(kctx, expr, gma, TEXPR_LOCAL, genv->localScope.varItems[i].ty, i);
		}
	}
	if(genv->localScope.varItems[0].ty != TY_void) {
		DBG_ASSERT(genv->this_cid == genv->localScope.varItems[0].ty);
		KonohaClass *ct = CT_(genv->this_cid);
		for(i = ct->fieldsize; i >= 0; i--) {
			if(ct->fieldItems[i].fn == fn && ct->fieldItems[i].ty != TY_void) {
				return SUGAR kExpr_setVariable(kctx, expr, gma, TEXPR_FIELD, ct->fieldItems[i].ty, longid((kshort_t)i, 0));
			}
		}
		kMethod *mtd = NameSpace_getGetterMethodNULL(kctx, ns, genv->this_cid, fn);
		if(mtd != NULL) {
			return new_GetterExpr(kctx, tk, mtd, new_VariableExpr(kctx, gma, TEXPR_LOCAL, genv->this_cid, 0));
		}
//		mtd = KLIB kNameSpace_getMethodNULL(kctx, ns, genv->this_cid, fn);
//		if(mtd != NULL) {
//			return new_FuncValue(kctx, mtd, 0);
//		}
	}
	{
		ktype_t cid = O_classId(ns->scriptObject);
		kMethod *mtd = NameSpace_getGetterMethodNULL(kctx, ns, cid, fn);
		if(mtd != NULL && cid != TY_System) {
			return new_GetterExpr(kctx, tk, mtd, new_ConstValueExpr(kctx, cid, ns->scriptObject));
		}
		mtd = KLIB kNameSpace_getMethodNULL(kctx, ns, cid, fn, 0, MPOL_FIRST);  // finding function
		if(mtd != NULL) {
			kParam *pa = Method_param(mtd);
			KonohaClass *ct = KLIB KonohaClass_Generics(kctx, CT_Func, pa->rtype, pa->psize, (kparamtype_t*)pa->paramtypeItems);
			kFuncVar *fo = (kFuncVar*)KLIB new_kObjectOnGCSTACK(kctx, ct, (uintptr_t)mtd);
			KSETv(fo->self, ns->scriptObject);
			return new_ConstValueExpr(kctx, ct->classId, UPCAST(fo));
		}
	}
	if(fn != SYM_NONAME) {
		KUtilsKeyValue *kv = kNameSpace_getConstNULL(kctx, ns, fn);
		if(kv != NULL) {
			if(SYMKEY_isBOXED(kv->key)) {
				SUGAR kExpr_setConstValue(kctx, expr, kv->ty, kv->oval);
			}
			else {
				SUGAR kExpr_setUnboxConstValue(kctx, expr, kv->ty, kv->uval);
			}
			return expr;
		}
	}
	kObject *v = NameSpace_getSymbolValueNULL(kctx, ns, S_text(tk->text), S_size(tk->text));
	kExpr *texpr = (v == NULL) ? kToken_p(stmt, tk, ErrTag, "undefined name: %s", Token_text(tk)) : SUGAR kExpr_setConstValue(kctx, expr, O_classId(v), v);
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
	kNameSpace *ns = Stmt_nameSpace(stmt);
	if(ukey != SYM_NONAME) {
		KUtilsKeyValue *kv = kNameSpace_getConstNULL(kctx, ns, ukey);
		if(kv != NULL) {
			if(SYMKEY_isBOXED(kv->key)) {
				SUGAR kExpr_setConstValue(kctx, expr, kv->ty, kv->oval);
			}
			else {
				SUGAR kExpr_setUnboxConstValue(kctx, expr, kv->ty, kv->uval);
			}
			RETURN_(expr);
		}
	}
	kObject *v = NameSpace_getSymbolValueNULL(kctx, ns, S_text(tk->text), S_size(tk->text));
	kExpr *texpr = (v == NULL) ? kToken_p(stmt, tk, ErrTag, "undefined name: %s", Token_text(tk)) : SUGAR kExpr_setConstValue(kctx, expr, O_classId(v), v);
	RETURN_(texpr);
}

static KMETHOD StmtTyCheck_ConstDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_StmtTyCheck(stmt, gma);
	kbool_t r = false;
	kNameSpace *ns = Stmt_nameSpace(stmt);
	kToken *tk = SUGAR kStmt_getToken(kctx, stmt, KW_UsymbolPattern, NULL);
	ksymbol_t ukey = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NEWID);
	KUtilsKeyValue *kv = kNameSpace_getConstNULL(kctx, ns, ukey);
	if(kv != NULL) {
		kStmt_p(stmt, ErrTag, "already defined name: %s", Token_text(tk));
	}
	else {
		r = kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_var, TPOL_CONST);
		if(r) {
			kExpr *expr = SUGAR kStmt_getExpr(kctx, stmt, KW_ExprPattern, NULL);
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
				kv.uval = expr->unboxConstValue;
				expr = NULL;
			}
			if(expr == NULL) {
				kNameSpace_mergeConstData(kctx, (kNameSpaceVar*)ns, &kv, 1, stmt->uline);
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
	return ct->classId;
}

static int param_policy(ksymbol_t fn)
{
	int pol = 0;
	if(FN_isCOERCION(fn)) {
		pol = pol | TPOL_COERCION;
	}
	return pol;
}

static kMethod *lookupOverloadedMethod(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kMethod *mtd, kGamma *gma)
{
	KonohaClass *thisClass = CT_(expr->cons->exprItems[1]->ty);
	size_t i, psize = kArray_size(expr->cons) - 2;
	kparamtype_t p[psize];
	kParam *pa = Method_param(mtd);
	for(i = 0; i < psize; i++) {
		size_t n = i + 2;
		ktype_t paramType = (i < pa->psize) ? ktype_var(kctx, pa->paramtypeItems[i].ty, thisClass) : TY_var;
		kExpr *texpr = SUGAR kkStmt_tyCheckByNameAt(kctx, stmt, expr, n, gma, paramType, TPOL_NOCHECK);
		if(texpr == K_NULLEXPR) {
			return NULL;
		}
		p[i].ty = expr->cons->exprItems[i+2]->ty;
	}
	kNameSpace *ns = Stmt_nameSpace(stmt);
	kparamid_t paramdom = KLIB Kparamdom(kctx, psize, p);
	kMethod *foundMethod = kNameSpace_getMethodNULL(kctx, ns, thisClass->classId, mtd->mn, paramdom, MPOL_SIGNATURE|MPOL_LATEST);
	DBG_P("paradom=%d, foundMethod=%p", paramdom, foundMethod);
	if(foundMethod == NULL) {
		kArray *abuf = kctx->stack->gcstack;
		size_t atop = kArray_size(abuf);
		kNameSpace_findMethodList(kctx, ns, thisClass->classId, mtd->mn, abuf, atop);
		for(i = atop; kArray_size(abuf); i++) {
			foundMethod = abuf->methodItems[i];
			kParam *pa = Method_param(foundMethod);
			if(pa->psize != psize) continue;
			// TODO; check parameter;
		}
		KLIB kArray_clear(kctx, abuf, atop);
		if(foundMethod == NULL) {
			foundMethod = mtd;
		}
	}
	return foundMethod;
}

static kExpr* Expr_typedWithMethod(KonohaContext *kctx, kExpr *expr, kMethod *mtd, ktype_t reqty)
{
	kExpr *thisExpr = kExpr_at(expr, 1);
	KSETv(expr->cons->methodItems[0], mtd);
	if(thisExpr->build == TEXPR_NEW) {
		kExpr_typed(expr, CALL, thisExpr->ty);
	}
	else {
		kExpr_typed(expr, CALL, Method_isSmartReturn(mtd) ? reqty : ktype_var(kctx, Method_returnType(mtd), CT_(thisExpr->ty)));
	}
	return expr;
}

static kExpr* getBoxedThisExpr(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kMethod *mtd)
{
	kExpr *thisExpr = expr->cons->exprItems[1];
	KonohaClass *thisClass = CT_(thisExpr->ty);
	DBG_ASSERT(IS_Method(mtd));
	DBG_ASSERT(thisClass->classId != TY_var);
	if(!TY_isUnbox(mtd->classId) && CT_isUnbox(thisClass)) {
		thisExpr = new_BoxingExpr(kctx, expr->cons->exprItems[1], thisClass->classId);
		KSETv(expr->cons->exprItems[1], thisExpr);
	}
	return thisExpr;
}

static kExpr *kStmt_tyCheckCallParamExpr(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kMethod *mtd, kGamma *gma, ktype_t reqty)
{
	kExpr *thisExpr = getBoxedThisExpr(kctx, stmt, expr, mtd);
	KonohaClass *thisClass = CT_(thisExpr->ty);
	int isConst = (Expr_isCONST(thisExpr)) ? 1 : 0;
	kParam *pa = Method_param(mtd);
	size_t i;
	DBG_ASSERT(pa->psize +2 == kArray_size(expr->cons));
	for(i = 0; i < pa->psize; i++) {
		size_t n = i + 2;
		ktype_t paramType = ktype_var(kctx, pa->paramtypeItems[i].ty, thisClass);
		int tycheckPolicy = param_policy(pa->paramtypeItems[i].fn);
		kExpr *texpr = SUGAR kkStmt_tyCheckByNameAt(kctx, stmt, expr, n, gma, paramType, tycheckPolicy);
		if(texpr == K_NULLEXPR) {
			return kExpr_p(stmt, expr, ErrTag, "%s.%s%s accepts %s at the parameter %d", Method_t(mtd), TY_t(paramType), (int)i+1);
		}
		if(!Expr_isCONST(texpr)) isConst = 0;
	}
	expr = Expr_typedWithMethod(kctx, expr, mtd, reqty);
	if(isConst && Method_isConst(mtd)) {
		ktype_t rtype = ktype_var(kctx, pa->rtype, thisClass);
		return ExprCall_toConstValue(kctx, expr, expr->cons, rtype);
	}
	return expr;
}

static kExpr* tyCheckDynamicCallParams(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kMethod *mtd, kGamma *gma, kString *name, kmethodn_t mn, ktype_t reqty)
{
	int i;
	kParam *pa = Method_param(mtd);
	ktype_t ptype = (pa->psize == 0) ? TY_Object : pa->paramtypeItems[0].ty;
	for(i = 2; i < kArray_size(expr->cons); i++) {
		kExpr *texpr = SUGAR kkStmt_tyCheckByNameAt(kctx, stmt, expr, i, gma, ptype, 0);
		if(texpr == K_NULLEXPR) return texpr;
	}
	Expr_add(kctx, expr, new_ConstValueExpr(kctx, TY_String, UPCAST(name)));
	return Expr_typedWithMethod(kctx, expr, mtd, reqty);
}

static const char* MethodType_t(KonohaContext *kctx, kmethodn_t mn, size_t psize)
{
	return "method";
}

static kExpr *Expr_lookupMethod(KonohaContext *kctx, kStmt *stmt, kExpr *expr, ktype_t this_cid, kGamma *gma, ktype_t reqty)
{
	kNameSpace *ns = Stmt_nameSpace(stmt);
	kTokenVar *tkMN = expr->cons->tokenVarItems[0];
	DBG_ASSERT(IS_Token(tkMN));
	if(tkMN->keyword == TK_SYMBOL) {
		tkMN->keyword = ksymbolA(S_text(tkMN->text), S_size(tkMN->text), SYM_NEWID);
	}
	size_t psize = kArray_size(expr->cons) - 2;
	kMethod *mtd = KLIB kNameSpace_getMethodNULL(kctx, ns, this_cid, tkMN->keyword, psize, MPOL_LATEST|MPOL_PARAMSIZE);
	if(mtd == NULL) {
		if(tkMN->text != TS_EMPTY) {  // find Dynamic Call ..
			mtd = KLIB kNameSpace_getMethodNULL(kctx, ns, this_cid, 0/*NONAME*/, 1, MPOL_FIRST|MPOL_PARAMSIZE);
			if(mtd != NULL) {
				return tyCheckDynamicCallParams(kctx, stmt, expr, mtd, gma, tkMN->text, tkMN->keyword, reqty);
			}
		}
		if(tkMN->keyword == MN_new && psize == 0 && CT_(kExpr_at(expr, 1)->ty)->baseclassId == TY_Object) {
			//DBG_P("baseclassId=%s", TY_t(CT_(kExpr_at(expr, 1)->ty)->baseclassId));
			DBG_ASSERT(kExpr_at(expr, 1)->ty != TY_var);
			return kExpr_at(expr, 1);  // new Person(); // default constructor
		}
		kToken_p(stmt, tkMN, ErrTag, "undefined %s: %s.%s%s", MethodType_t(kctx, tkMN->keyword, psize), TY_t(this_cid), KW_t(tkMN->keyword));
	}
	if(mtd != NULL) {
		if(Method_isOverloaded(mtd)) {
			DBG_P("found overloaded method %s.%s%s", Method_t(mtd));
			mtd = lookupOverloadedMethod(kctx, stmt, expr, mtd, gma);
		}
		DBG_P("found resolved method %s.%s%s isOverloaded=%d", Method_t(mtd), Method_isOverloaded(mtd));
		return kStmt_tyCheckCallParamExpr(kctx, stmt, expr, mtd, gma, reqty);
	}
	return K_NULLEXPR;
}

static KMETHOD ExprTyCheck_MethodCall(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kExpr *texpr = SUGAR kkStmt_tyCheckByNameAt(kctx, stmt, expr, 1, gma, TY_var, 0);
	if(texpr != K_NULLEXPR) {
		ktype_t this_cid = texpr->ty;
		RETURN_(Expr_lookupMethod(kctx, stmt, expr, this_cid, gma, reqty));
	}
}

// --------------------------------------------------------------------------
// FuncStyleCall

static kExpr *Expr_tyCheckFuncParams(KonohaContext *kctx, kStmt *stmt, kExpr *expr, KonohaClass *ct, kGamma *gma);

static kbool_t Expr_isSymbol(kExpr *expr)
{
	return (Expr_isTerm(expr) && (expr->termToken->keyword == TK_SYMBOL));
}

static kMethod* Expr_lookUpFuncOrMethod(KonohaContext *kctx, kNameSpace *ns, kExpr *exprN, kGamma *gma, ktype_t reqty)
{
	kExpr *firstExpr = kExpr_at(exprN, 0);
	kToken *termToken = firstExpr->termToken;
	ksymbol_t fn = ksymbolA(S_text(termToken->text), S_size(termToken->text), SYM_NONAME);
	GammaAllocaData *genv = gma->genv;
	int i;
	for(i = genv->localScope.varsize - 1; i >= 0; i--) {
		if(genv->localScope.varItems[i].fn == fn && TY_isFunc(genv->localScope.varItems[i].ty)) {
			SUGAR kExpr_setVariable(kctx, firstExpr, gma, TEXPR_LOCAL, genv->localScope.varItems[i].ty, i);
			return NULL;
		}
	}
	if(genv->localScope.varItems[0].ty != TY_void) {
		DBG_ASSERT(genv->this_cid == genv->localScope.varItems[0].ty);
		kMethod *mtd = KLIB kNameSpace_getMethodNULL(kctx, ns, genv->this_cid, fn, 0, MPOL_FIRST);
		if(mtd != NULL) {
			KSETv(exprN->cons->exprItems[1], new_VariableExpr(kctx, gma, TEXPR_LOCAL, gma->genv->this_cid, 0));
			return mtd;
		}
		KonohaClass *ct = CT_(genv->this_cid);
		if (ct->fieldsize) {
			for(i = ct->fieldsize; i >= 0; i--) {
				if(ct->fieldItems[i].fn == fn && TY_isFunc(ct->fieldItems[i].ty)) {
					SUGAR kExpr_setVariable(kctx, firstExpr, gma, TEXPR_FIELD, ct->fieldItems[i].ty, longid((kshort_t)i, 0));
					return NULL;
				}
			}
		}
		mtd = NameSpace_getGetterMethodNULL(kctx, ns, genv->this_cid, fn);
		if(mtd != NULL && TY_isFunc(Method_returnType(mtd))) {
			KSETv(exprN->cons->exprItems[0], new_GetterExpr(kctx, termToken, mtd, new_VariableExpr(kctx, gma, TEXPR_LOCAL, genv->this_cid, 0)));
			return NULL;
		}
	}
	{
		ktype_t cid = O_classId(ns->scriptObject);
		int paramsize = kArray_size(exprN->cons) - 2;
		kMethod *mtd = KLIB kNameSpace_getMethodNULL(kctx, ns, cid, fn, paramsize, MPOL_FIRST|MPOL_PARAMSIZE);
		if(mtd != NULL) {
			KSETv(exprN->cons->exprItems[1], new_ConstValueExpr(kctx, cid, ns->scriptObject));
			return mtd;
		}
		mtd = NameSpace_getGetterMethodNULL(kctx, ns, cid, fn);
		if(mtd != NULL && TY_isFunc(Method_returnType(mtd))) {
			KSETv(exprN->cons->exprItems[0], new_GetterExpr(kctx, termToken, mtd, new_ConstValueExpr(kctx, cid, ns->scriptObject)));
			return NULL;
		}
		mtd = KLIB kNameSpace_getMethodNULL(kctx, ns, TY_System, fn, paramsize, MPOL_FIRST|MPOL_PARAMSIZE);
		if(mtd != NULL) {
			KSETv(exprN->cons->exprItems[1], new_VariableExpr(kctx, gma, TEXPR_NULL, TY_System, 0));
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
		kMethod *mtd = Expr_lookUpFuncOrMethod(kctx, Stmt_nameSpace(stmt), expr, gma, reqty);
		if(mtd != NULL) {
			if(Method_isOverloaded(mtd)) {
				DBG_P("overloaded found %s.%s%s", Method_t(mtd));
				mtd = lookupOverloadedMethod(kctx, stmt, expr, mtd, gma);
			}
			RETURN_(kStmt_tyCheckCallParamExpr(kctx, stmt, expr, mtd, gma, reqty));
		}
		if(!TY_isFunc(kExpr_at(expr, 0)->ty)) {
			kToken *tk = kExpr_at(expr, 0)->termToken;
			DBG_ASSERT(IS_Token(tk));  // TODO: make error message in case of not Token
			RETURN_(kToken_p(stmt, tk, ErrTag, "undefined function: %s", Token_text(tk)));
		}
	}
	else {
		if(kkStmt_tyCheckByNameAt(kctx, stmt, expr, 0, gma, TY_var, 0) != K_NULLEXPR) {
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
		kExpr *texpr = SUGAR kkStmt_tyCheckByNameAt(kctx, stmt, expr, n, gma, pa->paramtypeItems[i].ty, 0);
		if(texpr == K_NULLEXPR) {
			return texpr;
		}
	}
	kMethod *mtd = KLIB kNameSpace_getMethodNULL(kctx, Stmt_nameSpace(stmt), TY_Func, MN_("invoke"), 0, MPOL_FIRST);
	DBG_ASSERT(mtd != NULL);
	KSETv(expr->cons->exprItems[1], expr->cons->exprItems[0]);
	return Expr_typedWithMethod(kctx, expr, mtd, rtype);
}

// ---------------------------------------------------------------------------
// Statement Expr

static KMETHOD StmtTyCheck_Expr(KonohaContext *kctx, KonohaStack *sfp)  // $expr
{
	VAR_StmtTyCheck(stmt, gma);
	kbool_t r = kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_var, TPOL_ALLOWVOID);
	kStmt_typed(stmt, EXPR);
	RETURNb_(r);
}

static KMETHOD StmtTyCheck_if(KonohaContext *kctx, KonohaStack *sfp)
{
	kbool_t r = 1;
	VAR_StmtTyCheck(stmt, gma);
	if((r = kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_Boolean, 0))) {
		kBlock *bkThen = SUGAR kStmt_getBlock(kctx, stmt, KW_BlockPattern, K_NULLBLOCK);
		kBlock *bkElse = SUGAR kStmt_getBlock(kctx, stmt, KW_else, K_NULLBLOCK);
		r = kBlock_tyCheckAll(kctx, bkThen, gma);
		r = r & kBlock_tyCheckAll(kctx, bkElse, gma);
		kStmt_typed(stmt, IF);
	}
	RETURNb_(r);
}

static kStmt* Stmt_lookupIfStmtWithoutElse(KonohaContext *kctx, kStmt *stmt)
{
	kBlock *bkElse = SUGAR kStmt_getBlock(kctx, stmt, KW_else, NULL);
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
		kBlock *bkElse = SUGAR kStmt_getBlock(kctx, stmt, KW_BlockPattern, K_NULLBLOCK);
		KLIB kObject_setObject(kctx, stmtIf, KW_else, TY_Block, bkElse);
		kStmt_done(stmt);
		r = kBlock_tyCheckAll(kctx, bkElse, gma);
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
	ktype_t rtype = Method_returnType(gma->genv->currentWorkingMethod);
	kStmt_typed(stmt, RETURN);
	if(rtype != TY_void) {
		r = kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, rtype, 0);
	} else {
		kExpr *expr = (kExpr*)kStmt_getObjectNULL(kctx, stmt, KW_ExprPattern);
		if (expr != NULL) {
			kStmt_p(stmt, WarnTag, "ignored return value");
			r = kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_var, 0);
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
		SUGAR kExpr_setVariable(kctx, expr, gma, TEXPR_LOCAL, ty, index);
		return true;
	}
	return false;
}

static kbool_t appendAssignmentStmt(KonohaContext *kctx, kExpr *expr, kStmt **lastStmtRef)
{
	kStmt *lastStmt = lastStmtRef[0];
	kStmt *newstmt = new_(Stmt, lastStmt->uline);
	kBlock_insertAfter(kctx, lastStmt->parentBlockNULL, lastStmt, newstmt);
	kStmt_setsyn(newstmt, SYN_(Stmt_nameSpace(newstmt), KW_ExprPattern));
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
			kExpr *vexpr = new_VariableExpr(kctx, gma, TEXPR_NULL, ty, 0);
			expr = new_TypedConsExpr(kctx, TEXPR_LET, TY_void, 3, K_NULL, expr, vexpr);
			return appendAssignmentStmt(kctx, expr, lastStmtRef);
		}
	}
	else if(expr->syn->keyword == KW_LET) {
		kExpr *lexpr = kExpr_at(expr, 1);
		if(SUGAR kkStmt_tyCheckByNameAt(kctx, stmt, expr, 2, gma, TY_var, 0) == K_NULLEXPR) {
			// this is neccesarry to avoid 'int a = a + 1;';
			return false;
		}
		if(ExprTerm_toVariable(kctx, stmt, lexpr, gma, ty)) {
			if(SUGAR kkStmt_tyCheckByNameAt(kctx, stmt, expr, 2, gma, ty, 0) != K_NULLEXPR) {
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
	kToken *tk  = SUGAR kStmt_getToken(kctx, stmt, KW_TypePattern, NULL);
	kExpr  *expr = SUGAR kStmt_getExpr(kctx, stmt, KW_ExprPattern, NULL);
	if(tk == NULL || !TK_isType(tk) || expr == NULL) {
		RETURNb_(false);
	}
	kStmt_done(stmt);
	RETURNb_(Expr_declType(kctx, stmt, expr, gma, TK_type(tk), &stmt));
}

// ------------------
// Method Utilities for MethodDecl

static KMETHOD MethodFunc_lazyCompilation(KonohaContext *kctx, KonohaStack *sfp)
{
	KonohaStack *esp = kctx->esp;
	kMethod *mtd = sfp[K_MTDIDX].mtdNC;
	kString *text = mtd->sourceCodeToken->text;
	kfileline_t uline = mtd->sourceCodeToken->uline;
	kNameSpace *ns = mtd->lazyCompileNameSpace;
	kMethod_compile(kctx, mtd, ns, text, uline);
	((KonohaContextVar*)kctx)->esp = esp;
	mtd->invokeMethodFunc(kctx, sfp); // call again;
}

static void kMethod_setLazyCompilation(KonohaContext *kctx, kMethodVar *mtd, kStmt *stmt, kNameSpace *ns)
{
	kToken *tcode = SUGAR kStmt_getToken(kctx, stmt, KW_BlockPattern, NULL);
	if(tcode != NULL && tcode->keyword == TK_CODE) {
		KSETv(mtd->sourceCodeToken, tcode);
		KSETv(mtd->lazyCompileNameSpace, ns);
		KLIB Method_setFunc(kctx, mtd, MethodFunc_lazyCompilation);
		KLIB kArray_add(kctx, ctxsugar->definedMethodList, mtd);
	}
}

static void kNameSpace_compileAllDefinedMethods(KonohaContext *kctx)
{
	size_t i, size = kArray_size(ctxsugar->definedMethodList);
	for (i = 0; i < size; ++i) {
		kMethod *mtd = ctxsugar->definedMethodList->methodItems[i];
		if (mtd->invokeMethodFunc == MethodFunc_lazyCompilation) {
			kString *text = mtd->sourceCodeToken->text;
			kfileline_t uline = mtd->sourceCodeToken->uline;
			kNameSpace *ns = mtd->lazyCompileNameSpace;
			kMethod_compile(kctx, mtd, ns, text, uline);
			assert(mtd->invokeMethodFunc != MethodFunc_lazyCompilation);
		}
	}
	KLIB kArray_clear(kctx, ctxsugar->definedMethodList, 0);
}

///* ------------------------------------------------------------------------ */
///* [ParamUtils] */

static kbool_t StmtTypeDecl_setParam(KonohaContext *kctx, kStmt *stmt, int n, kparamtype_t *p)
{
	kToken *tkT  = SUGAR kStmt_getToken(kctx, stmt, KW_TypePattern, NULL);
	kExpr  *expr = SUGAR kStmt_getExpr(kctx, stmt, KW_ExprPattern, NULL);
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

static kParam *kStmt_newMethodParamNULL(KonohaContext *kctx, kStmt *stmt, kGamma* gma)
{
	kParam *pa = (kParam*)kStmt_getObjectNULL(kctx, stmt, KW_ParamsPattern);
	if(pa == NULL || !IS_Param(pa)) {
		SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), KW_ParamsPattern);
		if(!Stmt_TyCheck(kctx, syn, stmt, gma)) {
			return NULL;
		}
	}
	pa = (kParam*)kStmt_getObjectNULL(kctx, stmt, KW_ParamsPattern);
	DBG_ASSERT(IS_Param(pa));
	return pa;
}

static KMETHOD StmtTyCheck_ParamsDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_StmtTyCheck(stmt, gma);
	kToken *tkT = SUGAR kStmt_getToken(kctx, stmt, KW_TypePattern, NULL); // type
	ktype_t rtype =  tkT == NULL ? TY_void : TK_type(tkT);
	kParam *pa = NULL;
	kBlock *params = (kBlock*)kStmt_getObjectNULL(kctx, stmt, KW_ParamsPattern);
	if(params == NULL) {
		pa = new_kParam(kctx, rtype, 0, NULL);
	}
	else if(IS_Block(params)) {
		size_t i, psize = kArray_size(params->stmtList);
		kparamtype_t p[psize];
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

///* ------------------------------------------------------------------------ */
///* [MethodDecl] */

static KDEFINE_FLAGNAME MethodDeclFlag[] = {
	{AKEY("@Public"),     kMethod_Public},
	{AKEY("@Virtual"),    kMethod_Virtual},
	{AKEY("@Final"),      kMethod_Final},
	{AKEY("@Const"),      kMethod_Const},
	{AKEY("@Static"),     kMethod_Static},
	{AKEY("@Restricted"), kMethod_Restricted},
	{AKEY("@Override"),   kMethod_Override},
	{NULL},
};

static ktype_t kStmt_getClassId(KonohaContext *kctx, kStmt *stmt, kNameSpace *ns, ksymbol_t kw, ktype_t defcid)
{
	kToken *tk = (kToken*)kStmt_getObjectNULL(kctx, stmt, kw);
	if(tk == NULL || !IS_Token(tk)) {
		return defcid;
	}
	else {
		DBG_ASSERT(TK_isType(tk));
		return TK_type(tk);
	}
}

static ksymbol_t kStmt_getMethodSymbol(KonohaContext *kctx, kStmt *stmt, kNameSpace *ns, ksymbol_t kw, kmethodn_t defmn)
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

static KMETHOD StmtTyCheck_MethodDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_StmtTyCheck(stmt, gma);
	kNameSpace *ns    = Stmt_nameSpace(stmt);
	uintptr_t flag    = kStmt_parseFlags(kctx, stmt, MethodDeclFlag, 0);
	ktype_t classId   = kStmt_getClassId(kctx, stmt, ns, KW_UsymbolPattern, O_classId(ns->scriptObject));
	kmethodn_t mn     = kStmt_getMethodSymbol(kctx, stmt, ns, KW_SymbolPattern, MN_new);
	kParam *pa        = kStmt_newMethodParamNULL(kctx, stmt, gma);
	if(TY_isSingleton(classId)) {
		flag |= kMethod_Static;
	}
	if(TY_isFinal(classId)) {
		flag |= kMethod_Final;
	}
	if(pa != NULL) {  // if pa is NULL, error is printed out.
		kMethod *mtd = KLIB new_kMethod(kctx, flag, classId, mn, NULL);
		PUSH_GCSTACK(mtd);
		KLIB Method_setParam(kctx, mtd, pa->rtype, pa->psize, (kparamtype_t*)pa->paramtypeItems);
		kMethod *foundMethod = kNameSpace_addMethod(kctx, ns, mtd);
		if(foundMethod != NULL) {
			pa = NULL;
			if(mtd->classId == foundMethod->classId) {
				kStmt_p(stmt, ErrTag, "method %s.%s%s has already defined", Method_t(mtd));
			}
			else {
				kStmt_p(stmt, ErrTag, "method %s.%s%s is final", Method_t(mtd));
			}
		}
		if(pa != NULL) {
			kMethod_setLazyCompilation(kctx, (kMethodVar*)mtd, stmt, ns);
		}
	}
	kStmt_done(stmt);
	RETURNb_(pa != NULL);
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
		{ TOKEN(DIV), _OP, .op2 = "/", .priority_op2 = 32, },
		{ TOKEN(MOD), _OP, .op2 = "%", .priority_op2 = 32, },
		{ TOKEN(MUL), _OP, .op2 = "*", .priority_op2 = 32, },
		{ TOKEN(ADD), _OP, .op1 = "+", .op2 = "+", .priority_op2 = 64, },
		{ TOKEN(SUB), _OP, .op1 = "-", .op2 = "-", .priority_op2 = 64, },
		{ TOKEN(LT), _OP, .op2 = "<", .priority_op2 = 256, },
		{ TOKEN(LTE), _OP, .op2 = "<=", .priority_op2 = 256, },
		{ TOKEN(GT), _OP, .op2 = ">", .priority_op2 = 256, },
		{ TOKEN(GTE), _OP, .op2 = ">=", .priority_op2 = 256, },
		{ TOKEN(EQ), _OP, .op2 = "==", .priority_op2 = 512, },
		{ TOKEN(NEQ), _OP, .op2 = "!=", .priority_op2 = 512, },
		{ TOKEN(AND), _OP, /*.op2 = ""unused*/ .priority_op2 = 1024, ExprTyCheck_(AND)},
		{ TOKEN(OR), _OP, /*.op2 = ""unused*/ .priority_op2 = 2048, ExprTyCheck_(OR)},
		{ TOKEN(NOT), _OP, .op1 = "!", },
//		{ TOKEN(":"),  _OP,  .priority_op2 = 3072,},
		{ TOKEN(LET),  _OPLeft, /*.op2 = "*"*/ .priority_op2 = 4096, },
		{ TOKEN(COMMA), ParseExpr_(COMMA), .op2 = "*", .priority_op2 = 8192, /*.flag = SYNFLAG_ExprLeftJoinOP2,*/ },
		{ TOKEN(DOLLAR), ParseExpr_(DOLLAR), },
		{ TOKEN(void), .type = TY_void, .rule ="$type [$USYMBOL \".\"] $SYMBOL $params [$block]", TopStmtTyCheck_(MethodDecl)},
		{ TOKEN(boolean), .type = TY_Boolean, },
		{ TOKEN(int),     .type = TY_Int, },
		{ TOKEN(true),  _TERM, ExprTyCheck_(true),},
		{ TOKEN(false),  _TERM, ExprTyCheck_(false),},
		{ TOKEN(if), .rule ="\"if\" \"(\" $expr \")\" $block [\"else\" else: $block]", TopStmtTyCheck_(if), StmtTyCheck_(if), },
		{ TOKEN(else), .rule = "\"else\" $block", TopStmtTyCheck_(else), StmtTyCheck_(else), },
		{ TOKEN(return), .rule ="\"return\" [$expr]", .flag = SYNFLAG_StmtBreakExec, StmtTyCheck_(return), },
		{ .keyword = KW_END, },
	};
	kNameSpace_defineSyntax(kctx, ns, SYNTAX);
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
