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

static kNode *CallTypeCheckFunc(KonohaContext *kctx, kFunc *fo, kNode *expr, kGamma *gma, kObject *reqType)
{
	INIT_GCSTACK();
	BEGIN_UnusedStack(lsfp);
//	KUnsafeFieldSet(lsfp[0].asNameSpace, kNode_ns(stmt));
	KUnsafeFieldSet(lsfp[1].asNode, expr);
	KUnsafeFieldSet(lsfp[2].asGamma, gma);
	KUnsafeFieldSet(lsfp[3].asObject, reqType);
	CallSugarMethod(kctx, lsfp, fo, 4, UPCAST(K_NULLNODE));
	END_UnusedStack();
	RESET_GCSTACK();
	DBG_ASSERT(IS_Node(lsfp[K_RTNIDX].asObject));
	return (kNode *)lsfp[K_RTNIDX].asObject;
}

static kNode *TypeCheck(KonohaContext *kctx, KSyntax *syn, kNode *expr, kGamma *gma, KClass* reqtc)
{
	int callCount = 0;
	//DBG_P("syn=%p, parent=%p, syn->keyword='%s%s'", syn, syn->parentSyntaxNULL, KSymbol_Fmt2(syn->keyword));
	kObject *reqType = KLIB Knull(kctx, reqtc);
	while(true) {
		int index, size;
		kFunc **FuncItems = KSyntax_funcTable(kctx, syn, KSugarTypeCheckFunc, &size);
		for(index = size - 1; index >= 0; index--) {
			kNode *texpr = CallTypeCheckFunc(kctx, FuncItems[index], expr, gma, reqType);
			callCount++;
			if(kNode_IsERR(expr)) return K_NULLNODE;
			if(texpr->attrTypeId != KType_var) {
				DBG_ASSERT(texpr->node != -1);
				return texpr;
			}
		}
		if(syn->parentSyntaxNULL == NULL) break;
		syn = syn->parentSyntaxNULL;
	}
	if(/*callCount == 0 || */!kNode_IsERR(expr)) {
		return kNode_Message(kctx, expr, ErrTag, "undefined typing: %s", KToken_t(expr->KeyOperatorToken));
//		if(kNode_IsTerm(expr)) {
//			return kNode_Message(kctx, stmt, expr->TermToken, ErrTag, "undefined typing: %s", KToken_t(expr->TermToken));
//		}
//		else {
//			DBG_P("syn=%p, parent=%p, syn->keyword='%s%s'", expr->syn, expr->syn->parentSyntaxNULL, KSymbol_Fmt2(syn->keyword));
//			return kNode_Message(kctx, stmt, ErrTag, "undefined operator: %s%s",  KSymbol_Fmt2(expr->syn->keyword));
//		}
	}
	return K_NULLNODE;
}

static void kNode_PutConstValue(KonohaContext *kctx, kNode *expr, KonohaStack *sfp)
{
	if(expr->node == KNode_Const) {
		KUnsafeFieldSet(sfp[0].asObject, expr->ObjectConstValue);
		sfp[0].unboxValue = kObject_Unbox(expr->ObjectConstValue);
	} else if(expr->node == KNode_UnboxConst) {
		sfp[0].unboxValue = expr->unboxConstValue;
	} else if(expr->node == KNode_New) {
		KUnsafeFieldSet(sfp[0].asObject, KLIB new_kObject(kctx, OnField, KClass_(expr->attrTypeId), 0));
	} else if(expr->node == KNode_MethodCall) {   /* case Object Object.boxing(UnboxType Val) */
		kMethod *mtd = expr->NodeList->MethodItems[0];
		kNode *texpr = expr->NodeList->NodeItems[1];
		assert(mtd->mn == MN_box && kArray_size(expr->NodeList) == 2);
		assert(KType_Is(UnboxType, expr->attrTypeId) == true);
		KUnsafeFieldSet(sfp[0].asObject, KLIB new_kObject(kctx, OnField, KClass_(expr->attrTypeId), texpr->unboxConstValue));
	} else {
		assert(expr->node == KNode_Null);
		KUnsafeFieldSet(sfp[0].asObject, KLIB Knull(kctx, KClass_(expr->attrTypeId)));
		sfp[0].unboxValue = 0;
	}
}

static kNode* kNodeNode_ToConstValue(KonohaContext *kctx, kNode *stmt, kNodeVar *expr, kArray *cons, KClass *rtype)
{
	size_t i, size = kArray_size(cons), psize = size - 2;
	kMethod *mtd = cons->MethodItems[0];
	BEGIN_UnusedStack(lsfp);
	for(i = 1; i < size; i++) {
		kNode_PutConstValue(kctx, cons->NodeItems[i], lsfp + i - 1);
	}
	KStackSetMethodAll(lsfp, KLIB Knull(kctx, KClass_(expr->attrTypeId)), kNode_uline(stmt), mtd, psize);
	KStackCall(lsfp);
	END_UnusedStack();
	if(KClass_Is(UnboxType, rtype) /* || rtype->typeId == KType_void*/) {
		return SUGAR kNode_SetUnboxConstValue(kctx, expr, rtype->typeId, lsfp[K_RTNIDX].unboxValue);
	}
	return SUGAR kNode_SetConstValue(kctx, expr, NULL, lsfp[K_RTNIDX].asObject);
}

static kNode *kNodeNode_ToBox(KonohaContext *kctx, kNode *stmt, kNode *texpr, kGamma *gma, KClass* reqClass)
{
	KClass *c = KClass_(texpr->attrTypeId);
	if(c->typeId != KType_boolean) c = KClass_Int;
	kMethod *mtd = kNameSpace_GetMethodByParamSizeNULL(kctx, kNode_ns(stmt), c, MN_box, 0, KMethodMatch_CamelStyle);
	DBG_ASSERT(mtd != NULL);
	//return new_TypedCallNode(kctx, stmt, gma, reqClass, mtd, 1, texpr);
	kNodeVar *expr = new_UntypedOperatorNode(kctx, KSyntax_(kNode_ns(stmt), KSymbol_NodeMethodCall), 2, mtd, texpr);
	return kNode_Type(kctx, expr, KNode_MethodCall, c->typeId/*reqClass->typeId*/);
}

static kNode *kNode_TypeCheck(KonohaContext *kctx, kNode *stmt, kNode *expr, kGamma *gma, KClass* reqClass, int pol)
{
	kNode *texpr = expr;
	if(kNode_IsERR(stmt)) texpr = K_NULLNODE;
	if(KTypeAttr_Unmask(expr->attrTypeId) == KType_var && expr != K_NULLNODE) {
		if(!IS_Node(expr)) {
			expr = new_ConstValueNode(kctx, NULL, UPCAST(expr));
		}
		texpr = TypeCheck(kctx, expr->syn, expr, gma, reqClass);
		if(kNode_IsERR(stmt)) texpr = K_NULLNODE;
	}
	if(texpr != K_NULLNODE) {
		kNameSpace *ns = kNode_ns(stmt);
		KClass *typedClass = KClass_(texpr->attrTypeId);
		if(typedClass->typeId == KType_void) {
			if(!FLAG_is(pol, TypeCheckPolicy_ALLOWVOID)) {
				texpr = kNodeNode_Message(kctx, stmt, expr, ErrTag, "void is not acceptable");
			}
			return texpr;
		}
		if(KClass_Is(TypeVar, typedClass)) {
			return kNodeNode_Message(kctx, stmt, expr, ErrTag, "not type variable %s", KClass_text(typedClass));
		}
		if(reqClass->typeId == KType_var || typedClass == reqClass || FLAG_is(pol, TypeCheckPolicy_NOCHECK)) {
			return texpr;
		}
		if(KClass_Isa(kctx, typedClass, reqClass)) {
			if(KClass_Is(UnboxType, typedClass) && !KClass_Is(UnboxType, reqClass)) {
				return kNodeNode_ToBox(kctx, stmt, texpr, gma, reqClass);
			}
			return texpr;
		}
		kMethod *mtd = kNameSpace_GetCoercionMethodNULL(kctx, ns, typedClass, reqClass);
		if(mtd != NULL) {
			if(kMethod_Is(Coercion, mtd) || FLAG_is(pol, TypeCheckPolicy_COERCION)) {
				return new_TypedCallNode(kctx, stmt, gma, reqClass, mtd, 1, texpr);
			}
			if(kNameSpace_IsAllowed(ImplicitCoercion, ns)) {
				kNodeNode_Message(kctx, stmt, expr, InfoTag, "implicit type coercion: %s to %s", KClass_text(typedClass), KClass_text(reqClass));
				return new_TypedCallNode(kctx, stmt, gma, reqClass, mtd, 1, texpr);
			}
		}
		DBG_P("%s(%d) is requested, but %s(%d) is given", KClass_text(reqClass), reqClass->typeId, KClass_text(typedClass), typedClass->typeId);
		return kNodeNode_Message(kctx, stmt, expr, ErrTag, "%s is requested, but %s is given", KClass_text(reqClass), KClass_text(typedClass));
	}
	return texpr;
}

static kNode* kNode_TypeCheckNodeAt(KonohaContext *kctx, kNode *stmt, kNode *exprP, size_t pos, kGamma *gma, KClass *reqClass, int pol)
{
	if(!kNode_IsTerm(exprP) && pos < kArray_size(exprP->NodeList)) {
		kNode *expr = exprP->NodeList->NodeItems[pos];
		expr = kNode_TypeCheck(kctx, stmt, expr, gma, reqClass, pol);
		KFieldSet(exprP->NodeList, exprP->NodeList->NodeItems[pos], expr);
		return expr;
	}
	return K_NULLNODE;
}

static kbool_t kNode_TypeCheckByName(KonohaContext *kctx, kNode *stmt, ksymbol_t symbol, kGamma *gma, KClass *reqClass, int pol)
{
	kNode *expr = (kNode *)kNode_GetObjectNULL(kctx, stmt, symbol);
	DBG_ASSERT(expr != NULL);
	if(IS_Node(expr)) {
		kNode *texpr = kNode_TypeCheck(kctx, stmt, expr, gma, reqClass, pol);
		if(texpr != K_NULLNODE && texpr != expr) {
			KLIB kObjectProto_SetObject(kctx, stmt, symbol, KType_Node, texpr);
		}
	}
	return !(kNode_IsERR(stmt));
}

/* ------------------------------------------------------------------------ */

//static kbool_t CallStatementFunc(KonohaContext *kctx, kFunc *fo, int *countRef, kNode *stmt, kGamma *gma)
//{
//	BEGIN_UnusedStack(lsfp);
//	KUnsafeFieldSet(lsfp[0].asNameSpace, kNode_ns(stmt));
//	KUnsafeFieldSet(lsfp[1].asNode,  stmt);
//	KUnsafeFieldSet(lsfp[2].asGamma, gma);
//	countRef[0] += 1;
//	CallSugarMethod(kctx, lsfp, fo, 3, UPCAST(K_FALSE));
//	END_UnusedStack();
//	return lsfp[K_RTNIDX].boolValue;
//}
//
//static kbool_t KSyntax_TypeCheckNode(KonohaContext *kctx, KSyntax *syn, kNode *stmt, kGamma *gma)
//{
//	int SugarFunc_index = Gamma_isTopLevel(gma) ? SugarFunc_TopLevelStatement : SugarFunc_Statement;
//	int callCount = 0;
//	while(true) {
//		int index, size;
//		kFunc **FuncItems = KSyntax_funcTable(kctx, syn, SugarFunc_index, &size);
//		for(index = size - 1; index >= 0; index--) {
//			CallStatementFunc(kctx, FuncItems[index], &callCount, stmt, gma);
//			if(stmt->node == KNode_Done) return true;
//			if(kNode_IsERR(stmt)) return false;
//			if(stmt->node != KNode_Done) {
//				return true;
//			}
//		}
//		if(syn->parentSyntaxNULL == NULL) break;
//		syn = syn->parentSyntaxNULL;
//	}
//	if(callCount == 0) {
//		const char *location = Gamma_isTopLevel(gma) ? "at the top level" : "inside the function";
//		kNode_Message(kctx, stmt, ErrTag, "%s%s is not available %s", KWSTMT_t(stmt->syn->keyword), location);
//		return false;
//	}
//	if(stmt->node != KNode_Error) {
//		kNode_Message(kctx, stmt, ErrTag, "statement typecheck error: %s%s", KWSTMT_t(syn->keyword));
//	}
//	return false;
//}

static kNode* TypeCheckNodeList(KonohaContext *kctx, kArray *nodeList, size_t n, kGamma *gma, KClass *reqc)
{
	kNode *stmt = nodeList->NodeItems[n];
	if(stmt->node == KNode_Done) return stmt;
	if(!kNode_IsERR(stmt)) {
		stmt = TypeCheck(kctx, stmt->syn, stmt, gma, reqc);
		KFieldSet(nodeList, nodeList->NodeItems[n], stmt);
	}
	return stmt;
}

static kNode* TypeCheckBlock(KonohaContext *kctx, kNode *block, kGamma *gma, KClass *reqc)
{
	size_t lvarsize = gma->genv->localScope.varsize;
	if(block != K_NULLBLOCK) {
		block->stacktop = lvarsize;
	}
	int i, size = kArray_size(block->NodeList) - 1;
	for(i = 0; i < size; i++) {
		kNode *stmt = TypeCheckNodeList(kctx, block->NodeList, i, gma, KClass_void);
		if(kNode_IsERR(stmt)) {
//			Gamma_setERROR(gma, 1);
			return block;  // untyped
		}
	}
	if(size >= 0) {
		kNode *stmt = TypeCheckNodeList(kctx, block->NodeList, size, gma, reqc);
		if(!kNode_IsERR(stmt)) {
//			Gamma_setERROR(gma, 1);
			kNode_Type(kctx, block, KNode_Block, stmt->attrTypeId);
		}
	}
	else {
		kNode_Type(kctx, block, KNode_Block, KType_void);
	}
	if(lvarsize < gma->genv->localScope.varsize && !kNode_Is(OpenBlock, block)) {
		gma->genv->localScope.varsize = lvarsize;
	}
	return block;
}

/* ------------------------------------------------------------------------ */

static KGammaAllocaData *kGamma_Push(KonohaContext *kctx, kGamma *gma, KGammaAllocaData *newone)
{
	KGammaAllocaData *oldone = gma->genv;
	gma->genv = newone;
	return oldone;
}

static KGammaAllocaData *kGamma_Pop(KonohaContext *kctx, kGamma *gma, KGammaAllocaData *oldone, KGammaAllocaData *checksum)
{
	KGammaAllocaData *newone = gma->genv;
	assert(checksum == newone);
	gma->genv = oldone;
	if(newone->localScope.allocsize > 0) {
		KFree(newone->localScope.varItems, newone->localScope.allocsize);
	}
	return newone;
}

#define KPushGammaStack(G,B) KGammaAllocaData *oldbuf_ = kGamma_Push(kctx, G, B)
#define KPopGammaStack(G,B)  kGamma_Pop(kctx, G, oldbuf_, B)

// --------------------------------------------------------------------------

static kNode* kMethod_newNode(KonohaContext *kctx, kMethod *mtd, kNameSpace *ns, kString *source, kfileline_t uline)
{
	const char *script = kString_text(source);
	if(IS_NULL(source) || script[0] == 0) {
		DBG_ASSERT(IS_Token(mtd->SourceToken));
		script = kString_text(mtd->SourceToken->text);
		uline = mtd->SourceToken->uline;
	}
	KTokenSeq tokens = {ns, KGetParserContext(kctx)->preparedTokenList, 0};
	KTokenSeq_Push(kctx, tokens);
	KTokenSeq_Tokenize(kctx, &tokens, script, uline);
	kNode *block = new_BlockNode2(kctx, NULL/*parentNode*/, NULL/*macro*/, &tokens);
	KTokenSeq_Pop(kctx, tokens);
	return block;
}

static void kGamma_InitParam(KonohaContext *kctx, KGammaAllocaData *genv, kParam *pa, kparamtype_t *callparam)
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
	kGamma *gma = KGetParserContext(kctx)->preparedGamma;
	kNode *bk = kMethod_newNode(kctx, mtd, ns, text, uline);
	KGammaAllocaData newgma = {0};
	KGammaStackDecl lvarItems[32 + param->psize];
	bzero(lvarItems, sizeof(KGammaStackDecl) * (32 + param->psize));
	newgma.currentWorkingMethod = mtd;
	newgma.thisClass = KClass_((mtd)->typeId);
	newgma.localScope.varItems = lvarItems;
	newgma.localScope.capacity = 32 + param->psize;
	newgma.localScope.varsize = 0;
	newgma.localScope.allocsize = 0;

	KPushGammaStack(gma, &newgma);
	kGamma_InitParam(kctx, &newgma, param, callparamNULL);
	TypeCheckBlock(kctx, bk, gma, KClass_void);
	KLIB kMethod_GenCode(kctx, mtd, bk, options);
	KPopGammaStack(gma, &newgma);
	RESET_GCSTACK();
	return mtd;
}

/* ------------------------------------------------------------------------ */
// eval

static kNode* kNode_CheckReturnType(KonohaContext *kctx, kNode *node)
{
	if(node->attrTypeId != KType_void) {
		kNode *stmt = new_TypedNode(kctx, kNode_ns(node), KNode_Return, KType_void, 0);
		kNode_AddParsedObject(kctx, stmt, KSymbol_ExprPattern, UPCAST(node));
		DBG_ASSERT(stmt->stacktop == 0);
		return stmt;
	}
	return node;
}

static kstatus_t kMethod_RunEval(KonohaContext *kctx, kMethod *mtd, ktypeattr_t rtype, kfileline_t uline, KTraceInfo *trace)
{
	BEGIN_UnusedStack(lsfp);
	KRuntimeContextVar *runtime = kctx->stack;
	if(runtime->evalty != KType_void) {
		KUnsafeFieldSet(lsfp[1].asObject, runtime->stack[runtime->evalidx].asObject);
		lsfp[1].intValue = runtime->stack[runtime->evalidx].intValue;
	}
	KStackSetMethodAll(lsfp, KLIB Knull(kctx, KClass_(rtype)), uline, mtd, 1);
	kstatus_t result = K_CONTINUE;
	if(KLIB KRuntime_tryCallMethod(kctx, lsfp)) {
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

//static kstatus_t kNode_EvalAtTopLevel(KonohaContext *kctx, kNode *bk, kMethod *mtd, KTraceInfo *trace)
//{
//	kGamma *gma = KGetParserContext(kctx)->preparedGamma;
//	KGammaStackDecl lvarItems[32] = {};
//	KGammaAllocaData newgma = {0};
//	newgma.flag = kGamma_TopLevel;
//	newgma.currentWorkingMethod = mtd;
//	newgma.thisClass     = KClass_NameSpace;
//	newgma.localScope.varItems  = lvarItems;
//	newgma.localScope.capacity  = 32;
//	newgma.localScope.varsize   = 0;
//	newgma.localScope.allocsize = 0;
//
//	GAMMA_PUSH(gma, &newgma);
//	kGamma_InitIt(kctx, &newgma, kMethod_GetParam(mtd));
//	TypeCheckBlock(kctx, bk, gma, KClass_void);
//	GAMMA_POP(gma, &newgma);
//
//	kNode *stmt = bk->NodeList->NodeItems[0];
//	if(stmt->syn == NULL && kArray_size(bk->NodeList) == 1) {
//		kctx->stack->evalty = KType_void;
//		return K_CONTINUE;
//	}
//	if(stmt->syn != NULL && stmt->syn->keyword == KSymbol_ERR) {
//		return K_BREAK;
//	}
//	kbool_t isTryEval = true;
//	if(KonohaContext_Is(CompileOnly, kctx)) {
//		size_t i;
//		isTryEval = false;
//		for(i = 0; i < kArray_size(bk->NodeList); i++) {
//			kNode *stmt = bk->NodeList->NodeItems[0];
//			if(stmt->node == KNode_MethodCall) {
//				kMethod *callMethod = stmt->NodeList->MethodItems[0];
//				if(callMethod->typeId == KType_NameSpace && kMethod_Is(Public, callMethod) && !kMethod_Is(Static, callMethod)) {
//					isTryEval = true;
//					break;
//				}
//			}
//		}
//	}
//	if(isTryEval) {
//		ktypeattr_t rtype = kNode_CheckReturnType(kctx, stmt);
//		KLIB kMethod_GenCode(kctx, mtd, bk, DefaultCompileOption);
//		return kMethod_RunEval(kctx, mtd, rtype, kNode_uline(stmt), trace);
//	}
//	return K_CONTINUE;
//}
//
//
//static void KTokenSeq_SelectStatement(KonohaContext *kctx, KTokenSeq *tokens, KTokenSeq *source)
//{
//	int currentIdx, sourceEndIdx = source->endIdx, isPreviousIndent = false;
//	for(currentIdx = source->beginIdx; currentIdx < sourceEndIdx; currentIdx++) {
//		kToken *tk = source->tokenList->TokenItems[currentIdx];
//		if(kToken_Is(StatementSeparator, tk)) {
//			source->endIdx = currentIdx + 1;
//			break;
//		}
//		if(isPreviousIndent && kToken_IsIndent(tk)) {
//			source->endIdx = currentIdx;
//			break;
//		}
//		isPreviousIndent = (kToken_IsIndent(tk));
//	}
//	KLIB kArray_Clear(kctx, tokens->tokenList, tokens->beginIdx);
//	tokens->endIdx = 0;
//	KTokenSeq_Preprocess(kctx, tokens, NULL, source, source->beginIdx);
//	//KdumpTokenArray(kctx, tokens->tokenList, tokens->beginIdx, tokens->endIdx);
//	source->beginIdx = source->endIdx;
//	source->endIdx = sourceEndIdx;
//}

static kstatus_t kNode_Eval(KonohaContext *kctx, kNode *stmt, kMethod *mtd, KTraceInfo *trace)
{
	kGamma *gma = KGetParserContext(kctx)->preparedGamma;
	KGammaStackDecl lvarItems[32] = {};
	KGammaAllocaData newgma = {0};
	newgma.flag = kGamma_TopLevel;
	newgma.currentWorkingMethod = mtd;
	newgma.thisClass     = KClass_NameSpace;
	newgma.localScope.varItems  = lvarItems;
	newgma.localScope.capacity  = 32;
	newgma.localScope.varsize   = 0;
	newgma.localScope.allocsize = 0;

	KPushGammaStack(gma, &newgma);
//	kGamma_InitIt(kctx, &newgma, kMethod_GetParam(mtd));
	stmt = TypeCheck(kctx, stmt->syn, stmt, gma, KClass_void);
	KPopGammaStack(gma, &newgma);
//	if(kNode_IsERR(stmt)) {
//		return K_BREAK;
//	}
//	kNode *stmt = block->NodeList->NodeItems[0];
//	if(stmt->syn == NULL && kArray_size(block->NodeList) == 1) {
//		kctx->stack->evalty = KType_void;
//		return K_CONTINUE;
//	}
//	if(stmt->syn != NULL && stmt->syn->keyword == KSymbol_ERR) {
//		return K_BREAK;
//	}
	kbool_t isTryEval = true;
	if(KonohaContext_Is(CompileOnly, kctx)) {
		isTryEval = false;
		if(stmt->node == KNode_MethodCall) {
			kMethod *callMethod = stmt->NodeList->MethodItems[0];
			if(callMethod->typeId == KType_NameSpace && kMethod_Is(Public, callMethod) && !kMethod_Is(Static, callMethod)) {
				isTryEval = true;
			}
		}
	}
	if(isTryEval) {
		ktypeattr_t rtype = KTypeAttr_Unmask(stmt->attrTypeId);
		stmt = kNode_CheckReturnType(kctx, stmt);
		KLIB kMethod_GenCode(kctx, mtd, stmt, DefaultCompileOption);
		return kMethod_RunEval(kctx, mtd, rtype, kNode_uline(stmt), trace);
	}
	return K_CONTINUE;
}

static kstatus_t KTokenSeq_Eval(KonohaContext *kctx, KTokenSeq *source, KTraceInfo *trace)
{
	kstatus_t status = K_CONTINUE;
	INIT_GCSTACK();
	kMethod *mtd = KLIB new_kMethod(kctx, _GcStack, kMethod_Static, 0, 0, NULL);
	KLIB kMethod_SetParam(kctx, mtd, KType_Object, 0, NULL);
	int currentIdx = FindFirstStatementToken(kctx, source->tokenList, source->beginIdx, source->endIdx);
	while(currentIdx < source->endIdx) {
		kNode *node = ParseNewNode(kctx, source->ns, source->tokenList, &currentIdx, source->endIdx, ParseStatementPatternOption, NULL);
		status = kNode_Eval(kctx, node, mtd, trace);
		if(status != K_CONTINUE) break;
		currentIdx = FindFirstStatementToken(kctx, source->tokenList, currentIdx, source->endIdx);
	}
	RESET_GCSTACK();
	return status;
}

/* ------------------------------------------------------------------------ */
