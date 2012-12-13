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

static kExpr *CallTypeCheckFunc(KonohaContext *kctx, kFunc *fo, int *countRef, kStmt *stmt, kExpr *expr, kGamma *gma, int reqty)
{
	INIT_GCSTACK();
	BEGIN_UnusedStack(lsfp);
	KUnsafeFieldSet(lsfp[0].asNameSpace, Stmt_ns(stmt));
	KUnsafeFieldSet(lsfp[1].asStmt, stmt);
	KUnsafeFieldSet(lsfp[2].asExpr, expr);
	KUnsafeFieldSet(lsfp[3].asGamma, gma);
	lsfp[4].intValue = reqty;
	countRef[0] += 1;
	CallSugarMethod(kctx, lsfp, fo, 5, UPCAST(K_NULLEXPR));
	END_UnusedStack();
	RESET_GCSTACK();
	DBG_ASSERT(IS_Expr(lsfp[K_RTNIDX].asObject));
	return (kExpr *)lsfp[K_RTNIDX].asObject;
}

static kExpr *TypeCheck(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kGamma *gma, KonohaClass* reqtc)
{
	int callCount = 0;
	SugarSyntax *syn = expr->syn;
	//DBG_P("syn=%p, parent=%p, syn->keyword='%s%s'", syn, syn->parentSyntaxNULL, PSYM_t(syn->keyword));
	while(true) {
		int index, size;
		kFunc **FuncItems = SugarSyntax_funcTable(kctx, syn, SugarFunc_TypeCheck, &size);
		for(index = size - 1; index >= 0; index--) {
			kExpr *texpr = CallTypeCheckFunc(kctx, FuncItems[index], &callCount, stmt, expr, gma, reqtc->typeId);
			if(kStmt_isERR(stmt)) return K_NULLEXPR;
			if(texpr->attrTypeId != KType_var) {
				DBG_ASSERT(texpr->build != -1);
				return texpr;
			}
		}
		if(syn->parentSyntaxNULL == NULL) break;
		syn = syn->parentSyntaxNULL;
	}
	if(callCount == 0 || !kStmt_isERR(stmt)) {
		if(Expr_isTerm(expr)) {
			return kStmtToken_Message(kctx, stmt, expr->termToken, ErrTag, "undefined term: %s", KToken_t(expr->termToken));
		}
		else {
			DBG_P("syn=%p, parent=%p, syn->keyword='%s%s'", expr->syn, expr->syn->parentSyntaxNULL, PSYM_t(syn->keyword));
			return kStmt_Message(kctx, stmt, ErrTag, "undefined operator: %s%s",  PSYM_t(expr->syn->keyword));
		}
	}
	return K_NULLEXPR;
}

static void kExpr_PutConstValue(KonohaContext *kctx, kExpr *expr, KonohaStack *sfp)
{
	if((kvisit_t)expr->build == TEXPR_CONST) {
		KUnsafeFieldSet(sfp[0].asObject, expr->objectConstValue);
		sfp[0].unboxValue = kObject_Unbox(expr->objectConstValue);
	} else if(expr->build == TEXPR_NCONST) {
		sfp[0].unboxValue = expr->unboxConstValue;
	} else if(expr->build == TEXPR_NEW) {
		KUnsafeFieldSet(sfp[0].asObject, KLIB new_kObject(kctx, OnField, KClass_(expr->attrTypeId), 0));
	} else if(expr->build == TEXPR_CALL) {
		/* case Object Object.boxing(UnboxType Val) */
		kMethod *mtd = expr->cons->MethodItems[0];
		kExpr *texpr = expr->cons->ExprItems[1];
		assert(mtd->mn == MN_box && kArray_size(expr->cons) == 2);
		assert(KType_Is(UnboxType, expr->attrTypeId) == true);
		KUnsafeFieldSet(sfp[0].asObject, KLIB new_kObject(kctx, OnField, KClass_(expr->attrTypeId), texpr->unboxConstValue));
	} else {
		assert((kvisit_t)expr->build == TEXPR_NULL);
		KUnsafeFieldSet(sfp[0].asObject, KLIB Knull(kctx, KClass_(expr->attrTypeId)));
		sfp[0].unboxValue = 0;
	}
}

static kExpr* kStmtExpr_ToConstValue(KonohaContext *kctx, kStmt *stmt, kExprVar *expr, kArray *cons, KonohaClass *rtype)
{
	size_t i, size = kArray_size(cons), psize = size - 2;
	kMethod *mtd = cons->MethodItems[0];
	BEGIN_UnusedStack(lsfp);
	for(i = 1; i < size; i++) {
		kExpr_PutConstValue(kctx, cons->ExprItems[i], lsfp + i - 1);
	}
	KStackSetMethodAll(lsfp, KLIB Knull(kctx, KClass_(expr->attrTypeId)), stmt->uline, mtd, psize);
	KStackCall(lsfp);
	END_UnusedStack();
	if(KClass_Is(UnboxType, rtype) /* || rtype->typeId == KType_void*/) {
		return SUGAR kExpr_SetUnboxConstValue(kctx, expr, rtype->typeId, lsfp[K_RTNIDX].unboxValue);
	}
	return SUGAR kExpr_SetConstValue(kctx, expr, NULL, lsfp[K_RTNIDX].asObject);
}

static kExpr *kStmtExpr_ToBox(KonohaContext *kctx, kStmt *stmt, kExpr *texpr, kGamma *gma, KonohaClass* reqClass)
{
	KonohaClass *c = KClass_(texpr->attrTypeId);
	if(c->typeId != KType_boolean) c = KClass_Int;
	kMethod *mtd = kNameSpace_GetMethodByParamSizeNULL(kctx, kStmt_ns(stmt), c, MN_box, 0, MethodMatch_CamelStyle);
	DBG_ASSERT(mtd != NULL);
	//return new_TypedCallExpr(kctx, stmt, gma, reqClass, mtd, 1, texpr);
	kExprVar *expr = new_UntypedCallStyleExpr(kctx, SYN_(Stmt_ns(stmt), Symbol_ExprMethodCall), 2, mtd, texpr);
	return kExpr_typed(expr, CALL, c->typeId/*reqClass->typeId*/);
}

static kExpr *kExpr_TypeCheck(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kGamma *gma, KonohaClass* reqClass, int pol)
{
	kExpr *texpr = expr;
	if(kStmt_isERR(stmt)) texpr = K_NULLEXPR;
	if(TypeAttr_Unmask(expr->attrTypeId) == KType_var && expr != K_NULLEXPR) {
		if(!IS_Expr(expr)) {
			expr = new_ConstValueExpr(kctx, NULL, UPCAST(expr));
		}
		texpr = TypeCheck(kctx, stmt, expr, gma, reqClass);
		if(kStmt_isERR(stmt)) texpr = K_NULLEXPR;
	}
	if(texpr != K_NULLEXPR) {
		kNameSpace *ns = Stmt_ns(stmt);
		KonohaClass *typedClass = KClass_(texpr->attrTypeId);
		if(typedClass->typeId == KType_void) {
			if(!FLAG_is(pol, TypeCheckPolicy_ALLOWVOID)) {
				texpr = kStmtExpr_Message(kctx, stmt, expr, ErrTag, "void is not acceptable");
			}
			return texpr;
		}
		if(KClass_Is(TypeVar, typedClass)) {
			return kStmtExpr_Message(kctx, stmt, expr, ErrTag, "not type variable %s", KClass_t(typedClass));
		}
		if(reqClass->typeId == KType_var || typedClass == reqClass || FLAG_is(pol, TypeCheckPolicy_NOCHECK)) {
			return texpr;
		}
		if(KClass_Isa(kctx, typedClass, reqClass)) {
			if(KClass_Is(UnboxType, typedClass) && !KClass_Is(UnboxType, reqClass)) {
				return kStmtExpr_ToBox(kctx, stmt, texpr, gma, reqClass);
			}
			return texpr;
		}
		kMethod *mtd = kNameSpace_GetCoercionMethodNULL(kctx, ns, typedClass, reqClass);
		if(mtd != NULL) {
			if(kMethod_Is(Coercion, mtd) || FLAG_is(pol, TypeCheckPolicy_COERCION)) {
				return new_TypedCallExpr(kctx, stmt, gma, reqClass, mtd, 1, texpr);
			}
			if(kNameSpace_IsAllowed(ImplicitCoercion, ns)) {
				kStmtExpr_Message(kctx, stmt, expr, InfoTag, "implicit type coercion: %s to %s", KClass_t(typedClass), KClass_t(reqClass));
				return new_TypedCallExpr(kctx, stmt, gma, reqClass, mtd, 1, texpr);
			}
		}
		DBG_P("%s(%d) is requested, but %s(%d) is given", KClass_t(reqClass), reqClass->typeId, KClass_t(typedClass), typedClass->typeId);
		return kStmtExpr_Message(kctx, stmt, expr, ErrTag, "%s is requested, but %s is given", KClass_t(reqClass), KClass_t(typedClass));
	}
	return texpr;
}

static kExpr* kStmt_TypeCheckExprAt(KonohaContext *kctx, kStmt *stmt, kExpr *exprP, size_t pos, kGamma *gma, KonohaClass *reqClass, int pol)
{
	if(!Expr_isTerm(exprP) && pos < kArray_size(exprP->cons)) {
		kExpr *expr = exprP->cons->ExprItems[pos];
		expr = kExpr_TypeCheck(kctx, stmt, expr, gma, reqClass, pol);
		KFieldSet(exprP->cons, exprP->cons->ExprItems[pos], expr);
		return expr;
	}
	return K_NULLEXPR;
}

static kbool_t kStmt_TypeCheckByName(KonohaContext *kctx, kStmt *stmt, ksymbol_t symbol, kGamma *gma, KonohaClass *reqClass, int pol)
{
	kExpr *expr = (kExpr *)kStmt_GetObjectNULL(kctx, stmt, symbol);
	DBG_ASSERT(expr != NULL);
	if(IS_Expr(expr)) {
		kExpr *texpr = kExpr_TypeCheck(kctx, stmt, expr, gma, reqClass, pol);
		if(texpr != K_NULLEXPR && texpr != expr) {
			KLIB kObjectProto_SetObject(kctx, stmt, symbol, KType_Expr, texpr);
		}
	}
	return !(kStmt_isERR(stmt));
}

/* ------------------------------------------------------------------------ */

static kbool_t CallStatementFunc(KonohaContext *kctx, kFunc *fo, int *countRef, kStmt *stmt, kGamma *gma)
{
	BEGIN_UnusedStack(lsfp);
	KUnsafeFieldSet(lsfp[0].asNameSpace, Stmt_ns(stmt));
	KUnsafeFieldSet(lsfp[1].asStmt,  stmt);
	KUnsafeFieldSet(lsfp[2].asGamma, gma);
	countRef[0] += 1;
	CallSugarMethod(kctx, lsfp, fo, 3, UPCAST(K_FALSE));
	END_UnusedStack();
	return lsfp[K_RTNIDX].boolValue;
}

static kbool_t SugarSyntax_TypeCheckStmt(KonohaContext *kctx, SugarSyntax *syn, kStmt *stmt, kGamma *gma)
{
	int SugarFunc_index = Gamma_isTopLevel(gma) ? SugarFunc_TopLevelStatement : SugarFunc_Statement;
	int callCount = 0;
	while(true) {
		int index, size;
		kFunc **FuncItems = SugarSyntax_funcTable(kctx, syn, SugarFunc_index, &size);
		for(index = size - 1; index >= 0; index--) {
			CallStatementFunc(kctx, FuncItems[index], &callCount, stmt, gma);
			if(Stmt_isDone(stmt)) return true;
			if(kStmt_isERR(stmt)) return false;
			if(stmt->build != TSTMT_UNDEFINED) {
				return true;
			}
		}
		if(syn->parentSyntaxNULL == NULL) break;
		syn = syn->parentSyntaxNULL;
	}
	if(callCount == 0) {
		const char *location = Gamma_isTopLevel(gma) ? "at the top level" : "inside the function";
		kStmt_Message(kctx, stmt, ErrTag, "%s%s is not available %s", KWSTMT_t(stmt->syn->keyword), location);
		return false;
	}
	if(stmt->build != TSTMT_ERR) {
		kStmt_Message(kctx, stmt, ErrTag, "statement typecheck error: %s%s", KWSTMT_t(syn->keyword));
	}
	return false;
}

static kbool_t kBlock_TypeCheckAll(KonohaContext *kctx, kBlock *bk, kGamma *gma)
{
	size_t i, lvarsize = gma->genv->localScope.varsize;
	int result = true;
	for(i = 0; i < kArray_size(bk->StmtList); i++) {
		kStmt *stmt = (kStmt *)bk->StmtList->ObjectItems[i];
		if(Stmt_isDone(stmt)) continue;
		KDump(stmt);
		if(kStmt_isERR(stmt) || !SugarSyntax_TypeCheckStmt(kctx, stmt->syn, stmt, gma)) {
			DBG_ASSERT(kStmt_isERR(stmt));
			Gamma_setERROR(gma, 1);
			result = false;
			break;
		}
	}
	if(bk != K_NULLBLOCK) {
		SUGAR kExpr_SetVariable(kctx, (kExprVar *)bk->esp, gma, TEXPR_LOCAL, KType_void, gma->genv->localScope.varsize);
	}
	if(lvarsize < gma->genv->localScope.varsize) {
		gma->genv->localScope.varsize = lvarsize;
	}
	return result;
}

/* ------------------------------------------------------------------------ */

static GammaAllocaData *kGamma_Push(KonohaContext *kctx, kGamma *gma, GammaAllocaData *newone)
{
	GammaAllocaData *oldone = gma->genv;
	gma->genv = newone;
	return oldone;
}

static GammaAllocaData *kGamma_Pop(KonohaContext *kctx, kGamma *gma, GammaAllocaData *oldone, GammaAllocaData *checksum)
{
	GammaAllocaData *newone = gma->genv;
	assert(checksum == newone);
	gma->genv = oldone;
	if(newone->localScope.allocsize > 0) {
		KFree(newone->localScope.varItems, newone->localScope.allocsize);
	}
	return newone;
}

#define GAMMA_PUSH(G,B) GammaAllocaData *oldbuf_ = kGamma_Push(kctx, G, B)
#define GAMMA_POP(G,B)  kGamma_Pop(kctx, G, oldbuf_, B)

// --------------------------------------------------------------------------

static kBlock* kMethod_newBlock(KonohaContext *kctx, kMethod *mtd, kNameSpace *ns, kString *source, kfileline_t uline)
{
	const char *script = kString_text(source);
	if(IS_NULL(source) || script[0] == 0) {
		DBG_ASSERT(IS_Token(mtd->SourceToken));
		script = kString_text(mtd->SourceToken->text);
		uline = mtd->SourceToken->uline;
	}
	TokenSeq tokens = {ns, GetSugarContext(kctx)->preparedTokenList, 0};
	TokenSeq_Push(kctx, tokens);
	TokenSeq_Tokenize(kctx, &tokens, script, uline);
	kBlock *bk = new_kBlock(kctx, NULL/*parentStmt*/, NULL/*macro*/, &tokens);
	TokenSeq_Pop(kctx, tokens);
	return bk;
}

static void kGamma_InitParam(KonohaContext *kctx, GammaAllocaData *genv, kParam *pa, kparamtype_t *callparam)
{
	int i, psize = (pa->psize + 1 < genv->localScope.capacity) ? pa->psize : genv->localScope.capacity - 1;
	for(i = 0; i < psize; i++) {
		genv->localScope.varItems[i+1].name = pa->paramtypeItems[i].name;
		genv->localScope.varItems[i+1].attrTypeId = (callparam == NULL) ? pa->paramtypeItems[i].attrTypeId : callparam[i].attrTypeId;
	}
	if(!kMethod_Is(Static, genv->currentWorkingMethod)) {
		genv->localScope.varItems[0].name = FN_this;
		genv->localScope.varItems[0].attrTypeId = genv->thisClass->typeId;
	}
	genv->localScope.varsize = psize+1;
}

static kMethod *kMethod_Compile(KonohaContext *kctx, kMethod *mtd, kparamtype_t *callparamNULL, kNameSpace *ns, kString *text, kfileline_t uline, int options)
{
	INIT_GCSTACK();
	kParam *param = kMethod_GetParam(mtd);
	if(callparamNULL != NULL) {
		//DynamicComplie();
	}
	kGamma *gma = GetSugarContext(kctx)->preparedGamma;
	kBlock *bk = kMethod_newBlock(kctx, mtd, ns, text, uline);
	GammaAllocaData newgma = {0};
	GammaStackDecl lvarItems[32 + param->psize];
	bzero(lvarItems, sizeof(GammaStackDecl) * (32 + param->psize));
	newgma.currentWorkingMethod = mtd;
	newgma.thisClass = KClass_((mtd)->typeId);
	newgma.localScope.varItems = lvarItems;
	newgma.localScope.capacity = 32 + param->psize;
	newgma.localScope.varsize = 0;
	newgma.localScope.allocsize = 0;

	GAMMA_PUSH(gma, &newgma);
	kGamma_InitParam(kctx, &newgma, param, callparamNULL);
	kBlock_TypeCheckAll(kctx, bk, gma);
	KLIB kMethod_GenCode(kctx, mtd, bk, options);
	GAMMA_POP(gma, &newgma);
	RESET_GCSTACK();
	return mtd;
}

/* ------------------------------------------------------------------------ */
// eval

static void kGamma_InitIt(KonohaContext *kctx, GammaAllocaData *genv, kParam *pa)
{
	KonohaStackRuntimeVar *base = kctx->stack;
	genv->localScope.varsize = 0;
	if(base->evalty != KType_void) {
		genv->localScope.varItems[1].name = FN_("it");
		genv->localScope.varItems[1].attrTypeId = base->evalty;
		genv->localScope.varsize = 1;
	}
}

static ktypeattr_t kStmt_CheckReturnType(KonohaContext *kctx, kStmt *stmt)
{
	if(stmt->syn != NULL && stmt->syn->keyword == Symbol_ExprPattern) {
		kExpr *expr = (kExpr *)kStmt_GetObjectNULL(kctx, stmt, Symbol_ExprPattern);
		DBG_ASSERT(expr != NULL);
		if(expr->attrTypeId != KType_void) {
			kStmt_setsyn(stmt, SYN_(Stmt_ns(stmt), Symbol_return));
			kStmt_typed(stmt, RETURN);
			return expr->attrTypeId;
		}
	}
	return KType_void;
}

static kstatus_t kMethod_RunEval(KonohaContext *kctx, kMethod *mtd, ktypeattr_t rtype, kfileline_t uline, KTraceInfo *trace)
{
	BEGIN_UnusedStack(lsfp);
	KonohaStackRuntimeVar *runtime = kctx->stack;
	if(runtime->evalty != KType_void) {
		KUnsafeFieldSet(lsfp[1].asObject, runtime->stack[runtime->evalidx].asObject);
		lsfp[1].intValue = runtime->stack[runtime->evalidx].intValue;
	}
	KStackSetMethodAll(lsfp, KLIB Knull(kctx, KClass_(rtype)), uline, mtd, 1);
	kstatus_t result = K_CONTINUE;
	if(KLIB KonohaRuntime_tryCallMethod(kctx, lsfp)) {
		runtime->evalidx = ((lsfp + K_RTNIDX) - kctx->stack->stack);
		runtime->evalty = rtype;
	}
	else {
		runtime->evalty = KType_void;  // no value
		result = K_BREAK;        // message must be reported;
	}
	END_UnusedStack();
	return result;
}

static kstatus_t kBlock_EvalAtTopLevel(KonohaContext *kctx, kBlock *bk, kMethod *mtd, KTraceInfo *trace)
{
	kGamma *gma = GetSugarContext(kctx)->preparedGamma;
	GammaStackDecl lvarItems[32] = {};
	GammaAllocaData newgma = {0};
	newgma.flag = kGamma_TopLevel;
	newgma.currentWorkingMethod = mtd;
	newgma.thisClass     = KClass_NameSpace;
	newgma.localScope.varItems  = lvarItems;
	newgma.localScope.capacity  = 32;
	newgma.localScope.varsize   = 0;
	newgma.localScope.allocsize = 0;

	GAMMA_PUSH(gma, &newgma);
	kGamma_InitIt(kctx, &newgma, kMethod_GetParam(mtd));
	kBlock_TypeCheckAll(kctx, bk, gma);
	GAMMA_POP(gma, &newgma);

	kStmt *stmt = bk->StmtList->StmtItems[0];
	if(stmt->syn == NULL && kArray_size(bk->StmtList) == 1) {
		kctx->stack->evalty = KType_void;
		return K_CONTINUE;
	}
	if(stmt->syn != NULL && stmt->syn->keyword == Symbol_ERR) {
		return K_BREAK;
	}
	kbool_t isTryEval = true;
	if(KonohaContext_Is(CompileOnly, kctx)) {
		size_t i;
		isTryEval = false;
		for(i = 0; i < kArray_size(bk->StmtList); i++) {
			kStmt *stmt = bk->StmtList->StmtItems[0];
			if(stmt->build == TSTMT_EXPR) {
				kExpr *expr = kStmt_GetExpr(kctx, stmt, Symbol_ExprPattern, NULL);
				DBG_ASSERT(expr != NULL);
				if(expr->build == TEXPR_CALL) {  // Check NameSpace method
					kMethod *callMethod = expr->cons->MethodItems[0];
					if(callMethod->typeId == KType_NameSpace && kMethod_Is(Public, callMethod) && !kMethod_Is(Static, callMethod)) {
						isTryEval = true;
						break;
					}
				}
			}
		}
	}
	if(isTryEval) {
		ktypeattr_t rtype = kStmt_CheckReturnType(kctx, stmt);
		KLIB kMethod_GenCode(kctx, mtd, bk, DefaultCompileOption);
		return kMethod_RunEval(kctx, mtd, rtype, stmt->uline, trace);
	}
	return K_CONTINUE;
}


static void TokenSeq_SelectStatement(KonohaContext *kctx, TokenSeq *tokens, TokenSeq *source)
{
	int currentIdx, sourceEndIdx = source->endIdx, isPreviousIndent = false;
	for(currentIdx = source->beginIdx; currentIdx < sourceEndIdx; currentIdx++) {
		kToken *tk = source->tokenList->TokenItems[currentIdx];
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
	KLIB kArray_Clear(kctx, tokens->tokenList, tokens->beginIdx);
	tokens->endIdx = 0;
	TokenSeq_Preprocess(kctx, tokens, NULL, source, source->beginIdx);
	//KdumpTokenArray(kctx, tokens->tokenList, tokens->beginIdx, tokens->endIdx);
	source->beginIdx = source->endIdx;
	source->endIdx = sourceEndIdx;
}

static kstatus_t TokenSeq_Eval(KonohaContext *kctx, TokenSeq *source, KTraceInfo *trace)
{
	kstatus_t status = K_CONTINUE;
	INIT_GCSTACK();
	kMethod *mtd = KLIB new_kMethod(kctx, _GcStack, kMethod_Static, 0, 0, NULL);
	KLIB kMethod_SetParam(kctx, mtd, KType_Object, 0, NULL);
	kBlock *singleBlock = new_(Block, source->ns, _GcStack);
	TokenSeq tokens = {source->ns, source->tokenList};

	while(source->beginIdx < source->endIdx) {
		TokenSeq_Push(kctx, tokens);
		TokenSeq_SelectStatement(kctx, &tokens, source);
		if(source->SourceConfig.foundErrorToken != NULL) {
			return K_BREAK;
		}
		while(tokens.beginIdx < tokens.endIdx) {
			KLIB kArray_Clear(kctx, singleBlock->StmtList, 0);
			if(!kBlock_AddNewStmt(kctx, singleBlock, &tokens)) {
				return K_BREAK;
			}
			if(kArray_size(singleBlock->StmtList) > 0) {
				status = kBlock_EvalAtTopLevel(kctx, singleBlock, mtd, trace);
				if(status != K_CONTINUE) break;
			}
		}
		TokenSeq_Pop(kctx, tokens);
		if(status != K_CONTINUE) break;
	}
	RESET_GCSTACK();
	return status;
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
