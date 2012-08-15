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

static kExpr *callExprTyCheckFunc(KonohaContext *kctx, kFunc *fo, kStmt *stmt, kExpr *expr, kGamma *gma, int reqty)
{
	INIT_GCSTACK();
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 5);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+0].o, fo->self, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+1].o, (kObject*)stmt, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+2].o, (kObject*)expr, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+3].o, (kObject*)gma,  GC_NO_WRITE_BARRIER);
	lsfp[K_CALLDELTA+4].intValue = reqty;
	KCALL(lsfp, 0, fo->mtd, 5, K_NULLEXPR);
	END_LOCAL();
	RESET_GCSTACK();
	DBG_ASSERT(IS_Expr(lsfp[0].asObject));
	return (kExpr*)lsfp[0].asObject;
}

static kExpr *ExprTyCheck(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kGamma *gma, int reqty)
{
	kFunc *fo = expr->syn->sugarFuncTable[SUGARFUNC_ExprTyCheck];
	if(fo == NULL) {
		if(Expr_isTerm(expr)) {
			return kToken_p(stmt, expr->termToken, ErrTag, "undefined token type checker: '%s'", Token_text(expr->termToken));
		}
		else {
			return kkStmt_printMessage(stmt, ErrTag, "undefined operator type checker: %s%s",  KW_t(expr->syn->keyword));
		}
	}
	kExpr *texpr;
	if(IS_Array(fo)) {
		int i;
		kArray *a = (kArray*)fo;
		for(i = kArray_size(a) - 1; i > 0; i--) {
			texpr = callExprTyCheckFunc(kctx, a->funcItems[i], stmt, expr, gma, reqty);
			if(Stmt_isERR(stmt)) return K_NULLEXPR;
			if(texpr->ty != TY_var) return texpr;
		}
		fo = a->funcItems[0];
	}
	DBG_ASSERT(IS_Func(fo));
	texpr = callExprTyCheckFunc(kctx, fo, stmt, expr, gma, reqty);
	if(Stmt_isERR(stmt)) return K_NULLEXPR;
//	FIXME: CHECK ALL VAR_ExprTyCheck
//	if(texpr->ty == TY_var && texpr != K_NULLEXPR) {
//		texpr = kExpr_p(stmt, expr, ErrTag, "typing error");
//	}
	return texpr;
}

static void Expr_putConstValue(KonohaContext *kctx, kExpr *expr, KonohaStack *sfp)
{
	if(expr->build == TEXPR_CONST) {
		KSETv_AND_WRITE_BARRIER(NULL, sfp[0].asObject, expr->objectConstValue, GC_NO_WRITE_BARRIER);
		sfp[0].unboxValue = O_unbox(expr->objectConstValue);
	}else if(expr->build == TEXPR_NCONST) {
		sfp[0].unboxValue = expr->unboxConstValue;
	}else if(expr->build == TEXPR_NEW) {
		KSETv_AND_WRITE_BARRIER(NULL, sfp[0].asObject, KLIB new_kObject(kctx, CT_(expr->ty), 0), GC_NO_WRITE_BARRIER);
	}else {
		assert(expr->build == TEXPR_NULL);
		KSETv_AND_WRITE_BARRIER(NULL, sfp[0].asObject, KLIB Knull(kctx, CT_(expr->ty)), GC_NO_WRITE_BARRIER);
		sfp[0].unboxValue = 0;
	}
}

static kExpr* ExprCall_toConstValue(KonohaContext *kctx, kExpr *expr, kArray *cons, ktype_t rtype)
{
	size_t i, size = kArray_size(cons), psize = size - 2;
	kMethod *mtd = cons->methodItems[0];
	BEGIN_LOCAL(lsfp, K_CALLDELTA + psize);
	for(i = 1; i < size; i++) {
		Expr_putConstValue(kctx, cons->exprItems[i], lsfp + K_CALLDELTA + i - 1);
	}
	KCALL(lsfp, 0, mtd, psize, KLIB Knull(kctx, CT_(expr->ty)));
	END_LOCAL();
	if(TY_isUnbox(rtype) || rtype == TY_void) {
		return SUGAR kExpr_setUnboxConstValue(kctx, expr, rtype, lsfp[0].unboxValue);
	}
	return SUGAR kExpr_setConstValue(kctx, expr, rtype, lsfp[0].asObject);
}

static kbool_t CT_isa(KonohaContext *kctx, ktype_t cid1, ktype_t cid2)
{
	DBG_ASSERT(cid1 != cid2); // should be checked
	KonohaClass *ct = CT_(cid1), *t = CT_(cid2);
	return ct->isSubType(kctx, ct, t);
}

static kExpr *Expr_tyCheck(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kGamma *gma, ktype_t reqty, int pol)
{
	kExpr *texpr = expr;
	if(Stmt_isERR(stmt)) texpr = K_NULLEXPR;
	if(expr->ty == TY_var && expr != K_NULLEXPR) {
		if(!IS_Expr(expr)) {
			expr = new_ConstValueExpr(kctx, O_typeId(expr), UPCAST(expr));
		}
		texpr = ExprTyCheck(kctx, stmt, expr, gma, reqty);
	}
	if(Stmt_isERR(stmt)) texpr = K_NULLEXPR;
	if(texpr != K_NULLEXPR) {
		//DBG_P("type=%s, reqty=%s", TY_t(expr->ty), TY_t(reqty));
		if(texpr->ty == TY_void) {
			if(!FLAG_is(pol, TPOL_ALLOWVOID)) {
				texpr = kExpr_p(stmt, expr, ErrTag, "void is not acceptable");
			}
			return texpr;
		}
		if(reqty == TY_var || texpr->ty == reqty || FLAG_is(pol, TPOL_NOCHECK)) {
			return texpr;
		}
		if(CT_isa(kctx, texpr->ty, reqty)) {
			if(TY_isUnbox(texpr->ty) && !TY_isUnbox(reqty)) {
				ktype_t unboxType = texpr->ty == TY_Boolean ? TY_Boolean : TY_Int;
				kMethod *mtd = kNameSpace_getMethodNULL(kctx, Stmt_nameSpace(stmt), unboxType, MN_box, 0, MPOL_PARAMSIZE);
				return new_TypedCallExpr(kctx, stmt, gma, texpr->ty, mtd, 1, texpr);
			}
			return texpr;
		}
		kMethod *mtd = kNameSpace_getCastMethodNULL(kctx, Stmt_nameSpace(stmt), texpr->ty, reqty);
		DBG_P("finding cast %s => %s: %p", TY_t(texpr->ty), TY_t(reqty), mtd);
		if(mtd != NULL && (Method_isCoercion(mtd) || FLAG_is(pol, TPOL_COERCION))) {
			return new_TypedCallExpr(kctx, stmt, gma, reqty, mtd, 1, texpr);
		}
		return kExpr_p(stmt, expr, ErrTag, "%s is requested, but %s is given", TY_t(reqty), TY_t(texpr->ty));
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

static kbool_t kStmt_tyCheckByName(KonohaContext *kctx, kStmt *stmt, ksymbol_t nameid, kGamma *gma, ktype_t reqty, int pol)
{
	kExpr *expr = (kExpr*)kStmt_getObjectNULL(kctx, stmt, nameid);
	if(expr != NULL && IS_Expr(expr)) {
		kExpr *texpr = Expr_tyCheck(kctx, stmt, expr, gma, reqty, pol);
//		DBG_P("reqty=%s, texpr->ty=%s isnull=%d", TY_t(reqty), TY_t(texpr->ty), (texpr == K_NULLEXPR));
		if(texpr != K_NULLEXPR) {
			if(texpr != expr) {
				KLIB kObject_setObject(kctx, stmt, nameid, TY_Expr, texpr);
			}
			return 1;
		}
	}
	return 0;
}

/* ------------------------------------------------------------------------ */

static kbool_t Stmt_TyCheckFunc(KonohaContext *kctx, kFunc *fo, kStmt *stmt, kGamma *gma)
{
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 3);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+0].o, (kObject*)fo->self, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+1].o, (kObject*)stmt, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+2].o, (kObject*)gma , GC_NO_WRITE_BARRIER);
	KCALL(lsfp, 0, fo->mtd, 3, K_FALSE);
	END_LOCAL();
	return lsfp[0].boolValue;
}

static kbool_t Stmt_TyCheck(KonohaContext *kctx, SugarSyntax *syn, kStmt *stmt, kGamma *gma)
{
	int SUGARFUNC_index = kGamma_isTOPLEVEL(gma) ? SUGARFUNC_TopStmtTyCheck : SUGARFUNC_StmtTyCheck;
	if(syn->sugarFuncTable[SUGARFUNC_index] == NULL) {
		const char *location = kGamma_isTOPLEVEL(gma) ? "at the top level" : "inside the function";
		kkStmt_printMessage(stmt, ErrTag, "%s%s is not available %s", T_statement(stmt->syn->keyword), location);
		return false;
	}
	else {
		kFunc *fo = syn->sugarFuncTable[SUGARFUNC_index];
		kbool_t result = true;
		if(IS_Array(fo)) { // @Future
			int i;
			kArray *a = (kArray*)fo;
			for(i = kArray_size(a) - 1; i > 0; i--) {
				result = Stmt_TyCheckFunc(kctx, a->funcItems[i], stmt, gma);
				if(stmt->syn == NULL) return true;
				if(stmt->build != TSTMT_UNDEFINED) return result;
			}
			fo = a->funcItems[0];
		}
		DBG_ASSERT(IS_Func(fo));
		result = Stmt_TyCheckFunc(kctx, fo, stmt, gma);
		if(stmt->syn == NULL) return true; // this means done;
		if(result == false && stmt->build != TSTMT_ERR) {
			kkStmt_printMessage(stmt, ErrTag, "statement typecheck error: %s%s", T_statement(syn->keyword));
		}
		return result;
	}
}

static kbool_t kBlock_tyCheckAll(KonohaContext *kctx, kBlock *bk, kGamma *gma)
{
	int i, result = true, lvarsize = gma->genv->localScope.varsize;
	for(i = 0; i < kArray_size(bk->stmtList); i++) {
		kStmt *stmt = (kStmt*)bk->stmtList->objectItems[i];
		SugarSyntax *syn = stmt->syn;
		KdumpStmt(kctx, stmt);
		if(syn == NULL) continue; /* This means 'done' */
		if(Stmt_isERR(stmt) || !Stmt_TyCheck(kctx, syn, stmt, gma)) {
			DBG_ASSERT(Stmt_isERR(stmt));
			kGamma_setERROR(gma, 1);
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

static GammaAllocaData *Gamma_push(KonohaContext *kctx, kGamma *gma, GammaAllocaData *newone)
{
	GammaAllocaData *oldone = gma->genv;
	gma->genv = newone;
	return oldone;
}

static GammaAllocaData *Gamma_pop(KonohaContext *kctx, kGamma *gma, GammaAllocaData *oldone, GammaAllocaData *checksum)
{
	GammaAllocaData *newone = gma->genv;
	assert(checksum == newone);
	gma->genv = oldone;
	if(newone->localScope.allocsize > 0) {
		KFREE(newone->localScope.varItems, newone->localScope.allocsize);
	}
	return newone;
}

#define GAMMA_PUSH(G,B) GammaAllocaData *oldbuf_ = Gamma_push(kctx, G, B)
#define GAMMA_POP(G,B)  Gamma_pop(kctx, G, oldbuf_, B)

// --------------------------------------------------------------------------

static kBlock* Method_newBlock(KonohaContext *kctx, kMethod *mtd, kNameSpace *ns, kString *source, kfileline_t uline)
{
	const char *script = S_text(source);
	if(IS_NULL(source) || script[0] == 0) {
		DBG_ASSERT(IS_Token(mtd->sourceCodeToken));
		script = S_text(mtd->sourceCodeToken->text);
		uline = mtd->sourceCodeToken->uline;
	}
	TokenRange rangeBuf, *range = new_TokenListRange(kctx, ns, KonohaContext_getSugarContext(kctx)->preparedTokenList, &rangeBuf);
	TokenRange_tokenize(kctx, range, script, uline);
	kBlock *bk = new_kBlock(kctx, NULL/*parentStmt*/, range, SemiColon);
	TokenRange_pop(kctx, range);
	return bk;
}

static void Gamma_initParam(KonohaContext *kctx, GammaAllocaData *genv, kParam *pa)
{
	int i, psize = (pa->psize + 1 < genv->localScope.capacity) ? pa->psize : genv->localScope.capacity - 1;
	for(i = 0; i < psize; i++) {
		genv->localScope.varItems[i+1].fn = pa->paramtypeItems[i].fn;
		genv->localScope.varItems[i+1].ty = pa->paramtypeItems[i].ty;
	}
	if(!Method_isStatic(genv->currentWorkingMethod)) {
		genv->localScope.varItems[0].fn = FN_this;
		genv->localScope.varItems[0].ty = genv->this_cid;
	}
	genv->localScope.varsize = psize+1;
}

static kbool_t kMethod_compile(KonohaContext *kctx, kMethod *mtd, kNameSpace *ns, kString *text, kfileline_t uline)
{
	INIT_GCSTACK();
	kGamma *gma = KonohaContext_getSugarContext(kctx)->preparedGamma;
	kBlock *bk = Method_newBlock(kctx, mtd, ns, text, uline);
	GammaStackDecl lvarItems[32] = {};
	GammaAllocaData newgma = {
		.currentWorkingMethod = mtd,
		.this_cid = (mtd)->typeId,
		.localScope.varItems = lvarItems, .localScope.capacity = 32, .localScope.varsize = 0, .localScope.allocsize = 0,
	};
	GAMMA_PUSH(gma, &newgma);
	Gamma_initParam(kctx, &newgma, Method_param(mtd));
	kBlock_tyCheckAll(kctx, bk, gma);
	KLIB kMethod_genCode(kctx, mtd, bk);
	GAMMA_POP(gma, &newgma);
	RESET_GCSTACK();
	return 1;
}

/* ------------------------------------------------------------------------ */
// eval

static void Gamma_initIt(KonohaContext *kctx, GammaAllocaData *genv, kParam *pa)
{
	KonohaContextRuntimeVar *base = kctx->stack;
	genv->localScope.varsize = 0;
	if(base->evalty != TY_void) {
		genv->localScope.varItems[1].fn = FN_("it");
		genv->localScope.varItems[1].ty = base->evalty;
		genv->localScope.varsize = 1;
	}
}

static ktype_t Stmt_checkReturnType(KonohaContext *kctx, kStmt *stmt)
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
	GammaAllocaData newgma = {
		.flag = kGamma_TOPLEVEL,
		.currentWorkingMethod = mtd,
		.this_cid     = TY_System,
		.localScope.varItems = lvarItems, .localScope.capacity = 32, .localScope.varsize = 0, .localScope.allocsize = 0,
	};
	GAMMA_PUSH(gma, &newgma);
	Gamma_initIt(kctx, &newgma, Method_param(mtd));
	kBlock_tyCheckAll(kctx, bk, gma);
	DBG_P("block size =%d", kArray_size(bk->stmtList));
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
		ktype_t rtype = Stmt_checkReturnType(kctx, stmt);
		KLIB kMethod_genCode(kctx, mtd, bk);
		return kMethod_runEval(kctx, mtd, rtype);
	}
}

static kstatus_t kMethod_runEval(KonohaContext *kctx, kMethod *mtd, ktype_t rtype)
{
	kstatus_t result = K_CONTINUE;
	INIT_GCSTACK();
	BEGIN_LOCAL(lsfp, K_CALLDELTA);
	{
		int jumpResult;
		KonohaContextRuntimeVar *runtime = kctx->stack;
		KonohaStack *jump_bottom = runtime->jump_bottom;
		jmpbuf_i lbuf = {};
		if(runtime->evaljmpbuf == NULL) {
			runtime->evaljmpbuf = (jmpbuf_i*)KCALLOC(sizeof(jmpbuf_i), 1);
		}
		memcpy(&lbuf, runtime->evaljmpbuf, sizeof(jmpbuf_i));
		runtime->jump_bottom = lsfp + K_CALLDELTA; // FIXME ??
		KSETv(K_NULL, runtime->optionalErrorMessage, TS_EMPTY);
		runtime->thrownScriptLine = 0;
		if((jumpResult = PLATAPI setjmp_i(*runtime->evaljmpbuf)) == 0) {
			//DBG_P("TY=%s, running EVAL..", TY_t(rtype));
			if(runtime->evalty != TY_void) {
				KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+1].o, runtime->stack[runtime->evalidx].o, GC_NO_WRITE_BARRIER);
				lsfp[K_CALLDELTA+1].intValue = runtime->stack[runtime->evalidx].intValue;
			}
			KCALL(lsfp, 0, mtd, 0, KLIB Knull(kctx, CT_(rtype)));
			runtime->evalty = rtype;
			runtime->evalidx = (lsfp - kctx->stack->stack);
		}
		else {
			//KLIB reportException(kctx);
			const char *file = PLATAPI shortFilePath(FileId_t(runtime->thrownScriptLine));
			PLATAPI reportCaughtException(SYM_t(jumpResult), file, (kushort_t)runtime->thrownScriptLine,  S_text(runtime->optionalErrorMessage));
			runtime->evalty = TY_void;  // no value
			result = K_BREAK;        // message must be reported;
		}
		runtime->jump_bottom = jump_bottom;
		memcpy(runtime->evaljmpbuf, &lbuf, sizeof(jmpbuf_i));
	}
	END_LOCAL();
	RESET_GCSTACK();
	return result;
}

static kstatus_t TokenRange_eval(KonohaContext *kctx, TokenRange *sourceRange)
{
	kstatus_t status = K_CONTINUE;
	kMethod *mtd = KLIB new_kMethod(kctx, kMethod_Static, 0, 0, NULL);
	PUSH_GCSTACK(mtd);
	KLIB kMethod_setParam(kctx, mtd, TY_Object, 0, NULL);
	int i = sourceRange->beginIdx, indent = 0;
	kBlock *singleBlock = KonohaContext_getSugarContext(kctx)->singleBlock;
	while(i < sourceRange->endIdx) {
		TokenRange rangeBuf, *range = new_TokenStackRange(kctx, sourceRange, &rangeBuf);
		sourceRange->beginIdx = i;
		i = TokenRange_selectStmtToken(kctx, range, sourceRange, SemiColon, &indent);
//		DBG_P("sourceRange->beginIdx=%d, i=%d, range=%d,%d", sourceRange->beginIdx, i, range->beginIdx, range->endIdx);
		if(range->errToken != NULL) return K_BREAK;
		if(range->endIdx > range->beginIdx) {
			KSETv(singleBlock, ((kBlockVar*)singleBlock)->blockNameSpace, sourceRange->ns);
			KLIB kArray_clear(kctx, singleBlock->stmtList, 0);
			kBlock_addNewStmt(kctx, singleBlock, range);
			TokenRange_pop(kctx, range);
			status = kBlock_genEvalCode(kctx, singleBlock, mtd);
			if(status != K_CONTINUE) break;
		}
	}
	return status;
}


/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
