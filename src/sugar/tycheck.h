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

static kExpr *callTypeCheckFunc(KonohaContext *kctx, kFunc *fo, int *countRef, kStmt *stmt, kExpr *expr, kGamma *gma, int reqty)
{
	INIT_GCSTACK();
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 5);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+0].o, fo->self, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+1].o, (kObject*)stmt, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+2].o, (kObject*)expr, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+3].o, (kObject*)gma,  GC_NO_WRITE_BARRIER);
	lsfp[K_CALLDELTA+4].intValue = reqty;
	countRef[0] += 1;
	{
		KonohaStack *sfp = lsfp + K_CALLDELTA;
		KSetMethodCallStack(sfp, 0/*UL*/, fo->mtd, 5, K_NULLEXPR);
		KonohaRuntime_callMethod(kctx, sfp);
	}
	END_LOCAL();
	RESET_GCSTACK();
	DBG_ASSERT(IS_Expr(lsfp[0].asObject));
	return (kExpr*)lsfp[0].asObject;
}

static kExpr *TypeCheck(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kGamma *gma, int reqty)
{
	int callCount = 0;
	SugarSyntax *syn = expr->syn;
	//DBG_P("syn=%p, parent=%p, syn->keyword='%s%s'", syn, syn->parentSyntaxNULL, PSYM_t(syn->keyword));
	while(true) {
		int index, size;
		kFunc **funcItems = SugarSyntax_funcTable(kctx, syn, SugarFunc_TypeCheck, &size);
		for(index = size - 1; index >= 0; index--) {
			kExpr *texpr = callTypeCheckFunc(kctx, funcItems[index], &callCount, stmt, expr, gma, reqty);
			if(Stmt_isERR(stmt)) return K_NULLEXPR;
			if(texpr->ty != TY_var) return texpr;
		}
		if(syn->parentSyntaxNULL == NULL) break;
		syn = syn->parentSyntaxNULL;
	}
	if(callCount == 0) {
		if(Expr_isTerm(expr)) {
			return kStmtToken_printMessage(kctx, stmt, expr->termToken, ErrTag, "undefined token type checker: '%s'", Token_text(expr->termToken));
		}
		else {
			DBG_P("syn=%p, parent=%p, syn->keyword='%s%s'", expr->syn, expr->syn->parentSyntaxNULL, PSYM_t(syn->keyword));
			return kStmt_printMessage(kctx, stmt, ErrTag, "undefined operator type checker: %s%s",  PSYM_t(expr->syn->keyword));
		}
	}
	return K_NULLEXPR;
}

static void kExpr_putConstValue(KonohaContext *kctx, kExpr *expr, KonohaStack *sfp)
{
	if(expr->build == TEXPR_CONST) {
		KSETv_AND_WRITE_BARRIER(NULL, sfp[0].asObject, expr->objectConstValue, GC_NO_WRITE_BARRIER);
		sfp[0].unboxValue = O_unbox(expr->objectConstValue);
	} else if(expr->build == TEXPR_NCONST) {
		sfp[0].unboxValue = expr->unboxConstValue;
	} else if(expr->build == TEXPR_NEW) {
		KSETv_AND_WRITE_BARRIER(NULL, sfp[0].asObject, KLIB new_kObject(kctx, CT_(expr->ty), 0), GC_NO_WRITE_BARRIER);
	} else {
		assert(expr->build == TEXPR_NULL);
		KSETv_AND_WRITE_BARRIER(NULL, sfp[0].asObject, KLIB Knull(kctx, CT_(expr->ty)), GC_NO_WRITE_BARRIER);
		sfp[0].unboxValue = 0;
	}
}

static kExpr* kExprCall_toConstValue(KonohaContext *kctx, kExpr *expr, kArray *cons, ktype_t rtype)
{
	size_t i, size = kArray_size(cons), psize = size - 2;
	kMethod *mtd = cons->methodItems[0];
	BEGIN_LOCAL(lsfp, K_CALLDELTA + psize);
	for(i = 1; i < size; i++) {
		kExpr_putConstValue(kctx, cons->exprItems[i], lsfp + K_CALLDELTA + i - 1);
	}
	{
		KonohaStack *sfp = lsfp + K_CALLDELTA;
		KSetMethodCallStack(sfp, 0/*UL*/, mtd, psize, KLIB Knull(kctx, CT_(expr->ty)));
		KonohaRuntime_callMethod(kctx, sfp);
	}
	END_LOCAL();
	if(TY_isUnbox(rtype) || rtype == TY_void) {
		return SUGAR kExpr_setUnboxConstValue(kctx, expr, rtype, lsfp[0].unboxValue);
	}
	return SUGAR kExpr_setConstValue(kctx, expr, rtype, lsfp[0].asObject);
}

static kExpr *Expr_tyCheck(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kGamma *gma, ktype_t reqty, int pol)
{
	kExpr *texpr = expr;
	if(Stmt_isERR(stmt)) texpr = K_NULLEXPR;
	if(expr->ty == TY_var && expr != K_NULLEXPR) {
		if(!IS_Expr(expr)) {
			expr = new_ConstValueExpr(kctx, O_typeId(expr), UPCAST(expr));
		}
		texpr = TypeCheck(kctx, stmt, expr, gma, reqty);
	}
	if(Stmt_isERR(stmt)) texpr = K_NULLEXPR;
	if(texpr != K_NULLEXPR) {
		//DBG_P("type=%s, reqty=%s", TY_t(expr->ty), TY_t(reqty));
		if(texpr->ty == TY_void) {
			if(!FLAG_is(pol, TPOL_ALLOWVOID)) {
				texpr = kStmtExpr_printMessage(kctx, stmt, expr, ErrTag, "void is not acceptable");
			}
			return texpr;
		}
		if(reqty == TY_var || texpr->ty == reqty || FLAG_is(pol, TPOL_NOCHECK)) {
			return texpr;
		}
		if(CT_isa(kctx, texpr->ty, reqty)) {
			if(TY_isUnbox(texpr->ty) && !TY_isUnbox(reqty)) {
				ktype_t unboxType = texpr->ty == TY_boolean ? TY_boolean : TY_int;
				kMethod *mtd = kNameSpace_getMethodByParamSizeNULL(kctx, Stmt_nameSpace(stmt), unboxType, MN_box, 0);
				return new_TypedCallExpr(kctx, stmt, gma, texpr->ty, mtd, 1, texpr);
			}
			return texpr;
		}
		kMethod *mtd = kNameSpace_getCastMethodNULL(kctx, Stmt_nameSpace(stmt), texpr->ty, reqty);
		DBG_P("finding cast %s => %s: %p", TY_t(texpr->ty), TY_t(reqty), mtd);
		if(mtd != NULL && (kMethod_is(Coercion, mtd) || FLAG_is(pol, TPOL_COERCION))) {
			return new_TypedCallExpr(kctx, stmt, gma, reqty, mtd, 1, texpr);
		}
		return kStmtExpr_printMessage(kctx, stmt, expr, ErrTag, "%s is requested, but %s is given", TY_t(reqty), TY_t(texpr->ty));
	}
	return texpr;
}

static kExpr* kStmt_tyCheckExprAt(KonohaContext *kctx, kStmt *stmt, kExpr *exprP, size_t pos, kGamma *gma, ktype_t reqty, int pol)
{
	if(!Expr_isTerm(exprP) && pos < kArray_size(exprP->cons)) {
		kExpr *expr = exprP->cons->exprItems[pos];
		expr = Expr_tyCheck(kctx, stmt, expr, gma, reqty, pol);
		KSETv(exprP->cons, exprP->cons->exprItems[pos], expr);
		return expr;
	}
	return K_NULLEXPR;
}

static kbool_t kStmt_tyCheckByName(KonohaContext *kctx, kStmt *stmt, ksymbol_t classNameSymbol, kGamma *gma, ktype_t reqty, int pol)
{
	kExpr *expr = (kExpr*)kStmt_getObjectNULL(kctx, stmt, classNameSymbol);
	if(expr != NULL && IS_Expr(expr)) {
		kExpr *texpr = Expr_tyCheck(kctx, stmt, expr, gma, reqty, pol);
//		DBG_P("reqty=%s, texpr->ty=%s isnull=%d", TY_t(reqty), TY_t(texpr->ty), (texpr == K_NULLEXPR));
		if(texpr != K_NULLEXPR) {
			if(texpr != expr) {
				KLIB kObject_setObject(kctx, stmt, classNameSymbol, TY_Expr, texpr);
			}
			return 1;
		}
	}
	return 0;
}

/* ------------------------------------------------------------------------ */

static kbool_t callStatementFunc(KonohaContext *kctx, kFunc *fo, int *countRef, kStmt *stmt, kGamma *gma)
{
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 3);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+0].o, (kObject*)fo->self, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+1].o, (kObject*)stmt, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+2].o, (kObject*)gma , GC_NO_WRITE_BARRIER);
	countRef[0] += 1;
	{
		KonohaStack *sfp = lsfp + K_CALLDELTA;
		KSetMethodCallStack(sfp, 0/*UL*/, fo->mtd, 3, K_FALSE);
		KonohaRuntime_callMethod(kctx, sfp);
	}
	END_LOCAL();
	return lsfp[0].boolValue;
}

static kbool_t SugarSyntax_tyCheckStmt(KonohaContext *kctx, SugarSyntax *syn, kStmt *stmt, kGamma *gma)
{
	int SugarFunc_index = Gamma_isTopLevel(gma) ? SugarFunc_TopLevelStatement : SugarFunc_Statement;
	int callCount = 0;
	while(true) {
		int index, size;
		kFunc **funcItems = SugarSyntax_funcTable(kctx, syn, SugarFunc_index, &size);
		for(index = size - 1; index >= 0; index--) {
			/*kbool_t result =*/ callStatementFunc(kctx, funcItems[index], &callCount, stmt, gma);
			if(Stmt_isDone(stmt)) return true;
			if(Stmt_isERR(stmt)) return false;
			if(stmt->build != TSTMT_UNDEFINED) {
				return true;
			}
		}
		if(syn->parentSyntaxNULL == NULL) break;
		syn = syn->parentSyntaxNULL;
	}
	if(callCount == 0) {
		const char *location = Gamma_isTopLevel(gma) ? "at the top level" : "inside the function";
		kStmt_printMessage(kctx, stmt, ErrTag, "%s%s is not available %s", KWSTMT_t(stmt->syn->keyword), location);
		return false;
	}
	if(stmt->build != TSTMT_ERR) {
		kStmt_printMessage(kctx, stmt, ErrTag, "statement typecheck error: %s%s", KWSTMT_t(syn->keyword));
	}
	return false;
}

static kbool_t kBlock_tyCheckAll(KonohaContext *kctx, kBlock *bk, kGamma *gma)
{
	int i, result = true, lvarsize = gma->genv->localScope.varsize;
	for(i = 0; i < kArray_size(bk->stmtList); i++) {
		kStmt *stmt = (kStmt*)bk->stmtList->objectItems[i];
		if(Stmt_isDone(stmt)) continue;
		KdumpStmt(kctx, stmt);
		if(Stmt_isERR(stmt) || !SugarSyntax_tyCheckStmt(kctx, stmt->syn, stmt, gma)) {
			DBG_ASSERT(Stmt_isERR(stmt));
			Gamma_setERROR(gma, 1);
			result = false;
			break;
		}
	}
	if(bk != K_NULLBLOCK) {
		SUGAR kExpr_setVariable(kctx, bk->esp, gma, TEXPR_LOCAL, TY_void, gma->genv->localScope.varsize);
	}
	if(lvarsize < gma->genv->localScope.varsize) {
		gma->genv->localScope.varsize = lvarsize;
	}
	return result;
}

/* ------------------------------------------------------------------------ */

static GammaAllocaData *kGamma_push(KonohaContext *kctx, kGamma *gma, GammaAllocaData *newone)
{
	GammaAllocaData *oldone = gma->genv;
	gma->genv = newone;
	return oldone;
}

static GammaAllocaData *kGamma_pop(KonohaContext *kctx, kGamma *gma, GammaAllocaData *oldone, GammaAllocaData *checksum)
{
	GammaAllocaData *newone = gma->genv;
	assert(checksum == newone);
	gma->genv = oldone;
	if(newone->localScope.allocsize > 0) {
		KFREE(newone->localScope.varItems, newone->localScope.allocsize);
	}
	return newone;
}

#define GAMMA_PUSH(G,B) GammaAllocaData *oldbuf_ = kGamma_push(kctx, G, B)
#define GAMMA_POP(G,B)  kGamma_pop(kctx, G, oldbuf_, B)

// --------------------------------------------------------------------------

static kBlock* kMethod_newBlock(KonohaContext *kctx, kMethod *mtd, kNameSpace *ns, kString *source, kfileline_t uline)
{
	const char *script = S_text(source);
	if(IS_NULL(source) || script[0] == 0) {
		DBG_ASSERT(IS_Token(mtd->sourceCodeToken));
		script = S_text(mtd->sourceCodeToken->text);
		uline = mtd->sourceCodeToken->uline;
	}
	TokenSequence tokens = {ns, KonohaContext_getSugarContext(kctx)->preparedTokenList, 0};
	TokenSequence_push(kctx, tokens);
	TokenSequence_tokenize(kctx, &tokens, script, uline);
	kBlock *bk = new_kBlock(kctx, NULL/*parentStmt*/, NULL/*macro*/, &tokens);
	TokenSequence_pop(kctx, tokens);
	return bk;
}

static void kGamma_initParam(KonohaContext *kctx, GammaAllocaData *genv, kParam *pa)
{
	int i, psize = (pa->psize + 1 < genv->localScope.capacity) ? pa->psize : genv->localScope.capacity - 1;
	for(i = 0; i < psize; i++) {
		genv->localScope.varItems[i+1].fn = pa->paramtypeItems[i].fn;
		genv->localScope.varItems[i+1].ty = pa->paramtypeItems[i].ty;
	}
	if(!kMethod_is(Static, genv->currentWorkingMethod)) {
		genv->localScope.varItems[0].fn = FN_this;
		genv->localScope.varItems[0].ty = genv->this_cid;
	}
	genv->localScope.varsize = psize+1;
}

static kbool_t kMethod_compile(KonohaContext *kctx, kMethod *mtd, kNameSpace *ns, kString *text, kfileline_t uline)
{
	INIT_GCSTACK();
	kGamma *gma = KonohaContext_getSugarContext(kctx)->preparedGamma;
	kBlock *bk = kMethod_newBlock(kctx, mtd, ns, text, uline);
	DBG_P("@@@@@@@@@ NS=%p", ns);
	GammaStackDecl lvarItems[32] = {};
	GammaAllocaData newgma = {0};
	newgma.currentWorkingMethod = mtd;
	newgma.this_cid = (mtd)->typeId;
	newgma.localScope.varItems = lvarItems;
	newgma.localScope.capacity = 32;
	newgma.localScope.varsize = 0;
	newgma.localScope.allocsize = 0;

	GAMMA_PUSH(gma, &newgma);
	kGamma_initParam(kctx, &newgma, Method_param(mtd));
	kBlock_tyCheckAll(kctx, bk, gma);
	KLIB kMethod_genCode(kctx, mtd, bk);
	GAMMA_POP(gma, &newgma);
	RESET_GCSTACK();
	return 1;
}

/* ------------------------------------------------------------------------ */
// eval

static void kGamma_initIt(KonohaContext *kctx, GammaAllocaData *genv, kParam *pa)
{
	KonohaStackRuntimeVar *base = kctx->stack;
	genv->localScope.varsize = 0;
	if(base->evalty != TY_void) {
		genv->localScope.varItems[1].fn = FN_("it");
		genv->localScope.varItems[1].ty = base->evalty;
		genv->localScope.varsize = 1;
	}
}

static ktype_t kStmt_checkReturnType(KonohaContext *kctx, kStmt *stmt)
{
	if(stmt->syn != NULL && stmt->syn->keyword == KW_ExprPattern) {
		kExpr *expr = (kExpr*)kStmt_getObjectNULL(kctx, stmt, KW_ExprPattern);
		DBG_ASSERT(expr != NULL);
		if(expr->ty != TY_void) {
			kStmt_setsyn(stmt, SYN_(Stmt_nameSpace(stmt), KW_return));
			kStmt_typed(stmt, RETURN);
			return expr->ty;
		}
	}
	return TY_void;
}

static kstatus_t kMethod_runEval(KonohaContext *kctx, kMethod *mtd, ktype_t rtype);

static kstatus_t kBlock_genEvalCode(KonohaContext *kctx, kBlock *bk, kMethod *mtd)
{
	kGamma *gma = KonohaContext_getSugarContext(kctx)->preparedGamma;
	GammaStackDecl lvarItems[32] = {};
	GammaAllocaData newgma = {0};
	newgma.flag = kGamma_TopLevel;
	newgma.currentWorkingMethod = mtd;
	newgma.this_cid     = TY_NameSpace;
	newgma.localScope.varItems = lvarItems;
	newgma.localScope.capacity = 32;
	newgma.localScope.varsize = 0;
	newgma.localScope.allocsize = 0;

	GAMMA_PUSH(gma, &newgma);
	kGamma_initIt(kctx, &newgma, Method_param(mtd));
	kBlock_tyCheckAll(kctx, bk, gma);
	GAMMA_POP(gma, &newgma);

	kStmt *stmt = bk->stmtList->stmtItems[0];
	if(stmt->syn == NULL && kArray_size(bk->stmtList) == 1) {
		kctx->stack->evalty = TY_void;
		return K_CONTINUE;
	}
	if(stmt->syn != NULL && stmt->syn->keyword == KW_ERR) {
		return K_BREAK;
	}
	else {
		ktype_t rtype = kStmt_checkReturnType(kctx, stmt);
		KLIB kMethod_genCode(kctx, mtd, bk);
		return kMethod_runEval(kctx, mtd, rtype);
	}
}

static kstatus_t kMethod_runEval(KonohaContext *kctx, kMethod *mtd, ktype_t rtype)
{
	BEGIN_LOCAL(lsfp, K_CALLDELTA);
	KonohaStackRuntimeVar *runtime = kctx->stack;
	if(runtime->evalty != TY_void) {
		KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+1].o, runtime->stack[runtime->evalidx].o, GC_NO_WRITE_BARRIER);
		lsfp[K_CALLDELTA+1].intValue = runtime->stack[runtime->evalidx].intValue;
	}
	KonohaStack *sfp = lsfp + K_CALLDELTA;
	KSetMethodCallStack(sfp, 0/*UL*/, mtd, 1, KLIB Knull(kctx, CT_(rtype)));
	kstatus_t result = K_CONTINUE;
	if(KLIB KonohaRuntime_tryCallMethod(kctx, sfp)) {
		runtime->evalty = rtype;
		runtime->evalidx = (lsfp - kctx->stack->stack);
	}
	else {
		runtime->evalty = TY_void;  // no value
		result = K_BREAK;        // message must be reported;
	}
	END_LOCAL();
	return result;
}

static void TokenSequence_selectStatement(KonohaContext *kctx, TokenSequence *tokens, TokenSequence *source)
{
	int currentIdx, sourceEndIdx = source->endIdx, isPreviousIndent = false;
	for(currentIdx = source->beginIdx; currentIdx < sourceEndIdx; currentIdx++) {
		kToken *tk = source->tokenList->tokenItems[currentIdx];
		if(kToken_is(StatementSeparator, tk)) {
			source->endIdx = currentIdx + 1;
			break;
		}
		if(isPreviousIndent && kToken_isIndent(tk)) {
			source->endIdx = currentIdx;
			break;
		}
		isPreviousIndent = (kToken_isIndent(tk));
	}
	KLIB kArray_clear(kctx, tokens->tokenList, tokens->beginIdx);
	tokens->endIdx = 0;
	TokenSequence_resolved2(kctx, tokens, NULL, source, source->beginIdx);
	KdumpTokenArray(kctx, tokens->tokenList, tokens->beginIdx, tokens->endIdx);
	source->beginIdx = source->endIdx;
	source->endIdx = sourceEndIdx;
}

static kstatus_t TokenSequence_eval(KonohaContext *kctx, TokenSequence *source)
{
	kstatus_t status = K_CONTINUE;
	kMethod *mtd = KLIB new_kMethod(kctx, kMethod_Static, 0, 0, NULL);
	PUSH_GCSTACK(mtd);
	KLIB kMethod_setParam(kctx, mtd, TY_Object, 0, NULL);
	kBlock *singleBlock = GCSAFE_new(Block, source->ns);
	TokenSequence tokens = {source->ns, source->tokenList};

	while(source->beginIdx < source->endIdx) {
		TokenSequence_push(kctx, tokens);
		TokenSequence_selectStatement(kctx, &tokens, source);
		if(source->SourceConfig.foundErrorToken != NULL) {
			return K_BREAK;
		}
		while(tokens.beginIdx < tokens.endIdx) {
			KLIB kArray_clear(kctx, singleBlock->stmtList, 0);
			if(!kBlock_addNewStmt(kctx, singleBlock, &tokens)) {
				return K_BREAK;
			}
			if(kArray_size(singleBlock->stmtList) > 0) {
				status = kBlock_genEvalCode(kctx, singleBlock, mtd);
				if(status != K_CONTINUE) break;
			}
		}
		TokenSequence_pop(kctx, tokens);
		if(status != K_CONTINUE) break;
	}
	return status;
}


/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
