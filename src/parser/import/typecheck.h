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

static kNode *kNode_Rebase(KonohaContext *kctx, kNode *node, size_t stacktop)
{
	if(!kNode_IsValue(node) && node->stacktop != stacktop) {
		size_t i, size = kNode_GetNodeListSize(kctx, node);
		if(node->node == KNode_Block) {
			for(i = 0; i < size; i++) {
				kNode *sub = node->NodeList->NodeItems[i];
				if(kNode_IsValue(sub)) continue;
				kNode_Rebase(kctx, sub, stacktop + (sub->stacktop - node->stacktop));
			}
		}
		else if(IS_Array(node->NodeList)) {
			for(i = 1; i < size; i++) {
				kNode *sub = node->NodeList->NodeItems[i];
				if(kNode_IsValue(sub)) continue;
				kNode_Rebase(kctx, sub, (i - 1) + stacktop + K_CALLDELTA);
			}
		}
		else if(IS_Node(node->NodeToPush)) {
			kNode_Rebase(kctx, node->NodeToPush, stacktop);
		}
		node->stacktop = stacktop;
	}
	return node;
}

static kNode *CallTypeFunc(KonohaContext *kctx, kFunc *fo, kNode *expr, kGamma *gma, kObject *reqType)
{
	INIT_GCSTACK();
	BEGIN_UnusedStack(lsfp);
	KUnsafeFieldSet(lsfp[1].asNode, expr);
	KUnsafeFieldSet(lsfp[2].asGamma, gma);
	KUnsafeFieldSet(lsfp[3].asObject, reqType);
	CallSugarMethod(kctx, lsfp, fo, 4, UPCAST(K_NULLNODE));
	END_UnusedStack();
	RESET_GCSTACK();
	if(kNode_IsError(expr)) return expr;
	if(lsfp[K_RTNIDX].asNode == K_NULLNODE) {
		DBG_ASSERT(expr->attrTypeId == KType_var); // untyped
	}
	DBG_ASSERT(IS_Node(lsfp[K_RTNIDX].asObject));
	return (kNode *)lsfp[K_RTNIDX].asObject;
}

static kNode *TypeNode(KonohaContext *kctx, KSyntax *syn0, kNode *expr, kGamma *gma, KClass* reqtc)
{
	KSyntax *syn = syn0;
	kObject *reqType = KLIB Knull(kctx, reqtc);
	int varsize = gma->genv->localScope.varsize;
	expr->stacktop = varsize;
	DBG_P(">>>>>>>>>> #stacktop = %d", varsize);
//	if(KFlag_Is(kshortflag_t, syn->flag, SYNFLAG_CallNode)) {
//		KPushMethodCall(gma);
//		KPushMethodCall(gma);
//		KPushMethodCall(gma);
//		KPushMethodCall(gma);
//	}
	while(true) {
		int index, size;
		kFunc **FuncItems = KSyntax_funcTable(kctx, syn, KSugarTypeFunc, &size);
		for(index = size - 1; index >= 0; index--) {
			kNode *texpr = CallTypeFunc(kctx, FuncItems[index], expr, gma, reqType);
			if(kNode_IsError(texpr) || texpr->attrTypeId != KType_var) {
				if(!kNode_Is(OpenBlock, expr)) {
					gma->genv->localScope.varsize = varsize;
				}
//				if(!kNode_IsValue(texpr)) {
//					texpr->stacktop = varsize;
//				}
				return texpr;
			}
		}
		if(syn->parentSyntaxNULL == NULL) break;
		syn = syn->parentSyntaxNULL;
	}
	if(!kNode_IsError(expr)) {
		expr = SUGAR MessageNode(kctx, expr, NULL, gma, ErrTag, "undefined typing: %s%s %s", KSymbol_Fmt2(syn0->keyword), KToken_t(expr->KeyOperatorToken));
	}
	gma->genv->localScope.varsize = varsize;
	return expr;
}

static void PutConstNode(KonohaContext *kctx, kNode *expr, KonohaStack *sfp)
{
	if(expr->node == KNode_Const) {
		KUnsafeFieldSet(sfp[0].asObject, expr->ObjectConstValue);
		sfp[0].unboxValue = kObject_Unbox(expr->ObjectConstValue);
	} else if(expr->node == KNode_UnboxConst) {
		sfp[0].unboxValue = expr->unboxConstValue;
	} else if(expr->node == KNode_New) {
		KUnsafeFieldSet(sfp[0].asObject, KLIB new_kObject(kctx, OnField, KClass_(expr->attrTypeId), 0));
//	} else if(expr->node == KNode_MethodCall) {   /* case Object Object.boxing(UnboxType Val) */
//		kMethod *mtd = expr->NodeList->MethodItems[0];
//		kNode *texpr = expr->NodeList->NodeItems[1];
//		assert(mtd->mn == MN_box && kArray_size(expr->NodeList) == 2);
//		assert(KType_Is(UnboxType, expr->attrTypeId) == true);
//		KUnsafeFieldSet(sfp[0].asObject, KLIB new_kObject(kctx, OnField, KClass_(expr->attrTypeId), texpr->unboxConstValue));
	} else {
		assert(expr->node == KNode_Null);
		KUnsafeFieldSet(sfp[0].asObject, KLIB Knull(kctx, KClass_(expr->attrTypeId)));
		sfp[0].unboxValue = 0;
	}
}

static kNode* MakeNodeConst(KonohaContext *kctx, kNode *expr, KClass *rtype)
{
	size_t i, size = kArray_size(expr->NodeList), psize = size - 2;
	kMethod *mtd = expr->NodeList->MethodItems[0];
	BEGIN_UnusedStack(lsfp);
	for(i = 1; i < size; i++) {
		PutConstNode(kctx, expr->NodeList->NodeItems[i], lsfp + i - 1);
	}
	KStackSetMethodAll(lsfp, KLIB Knull(kctx, KClass_(expr->attrTypeId)), kNode_uline(expr), mtd, psize);
	KStackCall(lsfp);
	END_UnusedStack();
	if(KClass_Is(UnboxType, rtype) /* || rtype->typeId == KType_void*/) {
		return SUGAR kNode_SetUnboxConst(kctx, expr, rtype->typeId, lsfp[K_RTNIDX].unboxValue);
	}
	return SUGAR kNode_SetConst(kctx, expr, NULL, lsfp[K_RTNIDX].asObject);
}

static kNode *BoxNode(KonohaContext *kctx, kNode *expr, kGamma *gma, KClass* reqClass)
{
	kNode *node = KNewNode(kNode_ns(node));
	KFieldSet(node, node->NodeToPush, node);
	DBG_ASSERT(kctx == NULL);
	return kNode_Type(kctx, node, KNode_And, node->attrTypeId);
}

static kNode *TypeCheckNode(KonohaContext *kctx, kNode *expr, kGamma *gma, KClass* reqClass, int pol)
{
	DBG_ASSERT(IS_Node(expr) && expr != K_NULLNODE);
	if(kNode_IsError(expr)) return expr;
	kNameSpace *ns = kNode_ns(expr);
	if(KTypeAttr_Unmask(expr->attrTypeId) == KType_var) {
		expr = TypeNode(kctx, expr->syn, expr, gma, reqClass);
		if(kNode_IsError(expr)) return expr;
	}
	KClass *typedClass = KClass_(expr->attrTypeId);
	if(reqClass->typeId == KType_void) {
		expr->attrTypeId = KType_void;
		return expr;
	}
	if(typedClass->typeId == KType_void) {
		if(!FLAG_is(pol, TypeCheckPolicy_AllowVoid)) {
			expr = SUGAR MessageNode(kctx, expr, NULL, gma, ErrTag, "void is unacceptable");
			DBG_ASSERT(kctx==NULL);
		}
		return expr;
	}
	if(KClass_Is(TypeVar, typedClass)) {
		return SUGAR MessageNode(kctx, expr, NULL, gma, ErrTag, "not type variable %s", KClass_text(typedClass));
	}
	if(reqClass->typeId == KType_var || typedClass == reqClass || FLAG_is(pol, TypeCheckPolicy_NoCheck)) {
		return expr;
	}
	if(KClass_Isa(kctx, typedClass, reqClass)) {
		if(KClass_Is(UnboxType, typedClass) && !KClass_Is(UnboxType, reqClass)) {
			return BoxNode(kctx, expr, gma, reqClass);
		}
		return expr;
	}
	kMethod *mtd = kNameSpace_GetCoercionMethodNULL(kctx, ns, typedClass, reqClass);
	if(mtd != NULL) {
		if(kMethod_Is(Coercion, mtd) || FLAG_is(pol, TypeCheckPolicy_Coercion)) {
			return new_MethodNode(kctx, ns, gma, reqClass, mtd, 1, expr);
		}
		if(kNameSpace_IsAllowed(ImplicitCoercion, ns)) {
			SUGAR MessageNode(kctx, expr, NULL, gma, InfoTag, "implicit type coercion: %s to %s", KClass_text(typedClass), KClass_text(reqClass));
			return new_MethodNode(kctx, ns, gma, reqClass, mtd, 1, expr);
		}
	}
	DBG_P("%s(%d) is requested, but %s(%d) is given", KClass_text(reqClass), reqClass->typeId, KClass_text(typedClass), typedClass->typeId);
	return SUGAR MessageNode(kctx, expr, NULL, gma, ErrTag, "%s is requested, but %s is given", KClass_text(reqClass), KClass_text(typedClass));
}

static kNode* TypeCheckNodeAt(KonohaContext *kctx, kNode *node, size_t pos, kGamma *gma, KClass *reqClass, int pol)
{
	DBG_ASSERT(IS_Array(node->NodeList));
	DBG_ASSERT(pos < kArray_size(node->NodeList));
	kNode *expr = node->NodeList->NodeItems[pos];
	kNode *texpr = TypeCheckNode(kctx, expr, gma, reqClass, pol);
	if(texpr != expr) {
		KFieldSet(node->NodeList, node->NodeList->NodeItems[pos], texpr);
		KFieldSet(texpr, texpr->Parent, node);
	}
	return expr;
}

static kNode* TypeCheckNodeByName(KonohaContext *kctx, kNode *stmt, ksymbol_t symbol, kGamma *gma, KClass *reqClass, int pol)
{
	kNode *expr = (kNode *)kNode_GetObjectNULL(kctx, stmt, symbol);
	if(expr != NULL) {
		if(IS_Token(expr)) {
			kTokenVar *tk = (kTokenVar *)expr;
			kNameSpace *ns = kNode_ns(stmt);
			if(tk->resolvedSyntaxInfo->keyword == TokenType_CODE) {
				kToken_ToBraceGroup(kctx, (kTokenVar *)tk, ns, NULL);
			}
			if(tk->resolvedSyntaxInfo->keyword == KSymbol_BraceGroup) {
				int beginIdx = 0;
				expr = ParseNewNode(kctx, ns, tk->subTokenList, &beginIdx, kArray_size(tk->subTokenList), ParseMetaPatternOption|ParseBlockOption, NULL);
				KLIB kObjectProto_SetObject(kctx, stmt, symbol, kObject_typeId(expr), expr);
			}
		}
		if(IS_Node(expr)) {
			kNode *texpr = TypeCheckNode(kctx, expr, gma, reqClass, pol);
			if(texpr != expr) {
				KLIB kObjectProto_SetObject(kctx, stmt, symbol, KType_Node, texpr);
				KFieldSet(texpr, texpr->Parent, stmt);
			}
			return texpr;
		}
	}
	return NULL; //error
}

/* ------------------------------------------------------------------------ */

static kNode* TypeCheckNodeList(KonohaContext *kctx, kArray *nodeList, size_t n, kGamma *gma, KClass *reqc)
{
	kNode *stmt = nodeList->NodeItems[n];
	if(stmt->attrTypeId != KType_var) return stmt;
	if(!kNode_IsError(stmt)) {
		stmt = TypeNode(kctx, stmt->syn, stmt, gma, reqc);
		KFieldSet(nodeList, nodeList->NodeItems[n], stmt);
	}
	return stmt;
}

static kNode* TypeCheckBlock(KonohaContext *kctx, kNode *block, kGamma *gma, KClass *reqc)
{
	DBG_P("########## starting block");
	KDump(block);
	int i, size = kNode_GetNodeListSize(kctx, block) - 1;
	for(i = 0; i < size; i++) {
		DBG_P("###### Block[%d]", i);
		KDump(kNode_At(block, i));
		kNode *stmt = TypeCheckNodeList(kctx, block->NodeList, i, gma, KClass_void);
		KDump(stmt);
		if(kNode_IsError(stmt)) {
			return stmt;  // untyped
		}
	}
	if(size >= 0) {
		DBG_P("###### Block[%d]", size);
		KDump(kNode_At(block, size));
		kNode *stmt = TypeCheckNodeList(kctx, block->NodeList, size, gma, reqc);
		KDump(stmt);
		if(!kNode_IsError(stmt)) {
			kNode_Type(kctx, block, KNode_Block, stmt->attrTypeId);
		}
	}
	else {
		if(block != K_NULLBLOCK) {
			kNode_Type(kctx, block, KNode_Block, KType_void);
		}
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

static kNode* kMethod_ParseBodyNode(KonohaContext *kctx, kMethod *mtd, kNameSpace *ns, kString *source, kfileline_t uline)
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
	kNode *node = ParseStatementNode(kctx, ns, tokens.tokenList, tokens.beginIdx, tokens.endIdx);
	KTokenSeq_Pop(kctx, tokens);
	return node;
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
	kNode *node = kMethod_ParseBodyNode(kctx, mtd, ns, text, uline);
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
	node = TypeCheckNode(kctx, node, gma, KClass_void, 0);
	KLIB kMethod_GenCode(kctx, mtd, node, options);
	KPopGammaStack(gma, &newgma);
	RESET_GCSTACK();
	return mtd;
}

/* ------------------------------------------------------------------------ */
// eval

static kNode* kNode_CheckReturnType(KonohaContext *kctx, kNode *node)
{
	if(node->attrTypeId != KType_void) {
		kNode *stmt = new_TypedNode(kctx, kNode_ns(node), KNode_Return, KClass_void, 0);
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
	stmt = TypeCheckNode(kctx, stmt, gma, KClass_var, TypeCheckPolicy_AllowVoid);
	KPopGammaStack(gma, &newgma);
	if(kNode_IsError(stmt)) {
		return K_BREAK;  // to avoid duplicated error message
	}
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

static kbool_t KTokenSeq_PreprocessSingleStatement(KonohaContext *kctx, KTokenSeq *tokens, KTokenSeq *source)
{
	int beginIdx, endIdx;
	for(beginIdx = source->beginIdx; beginIdx < source->endIdx; beginIdx++) {
		kToken *tk = source->tokenList->TokenItems[beginIdx];
		if(tk->tokenType == TokenType_INDENT/* || kToken_Is(StatementSeparator, tk)*/) continue;
		break;
	}
	for(endIdx = beginIdx; endIdx < source->endIdx; endIdx++) {
		kToken *tk = source->tokenList->TokenItems[endIdx];
		if(tk->tokenType == TokenType_INDENT/* || kToken_Is(StatementSeparator, tk)*/) {
			break;
		}
	}
	if(beginIdx < endIdx) {
		int pushEndIdx = source->endIdx; source->endIdx = endIdx;
		KTokenSeq_Preprocess(kctx, tokens, NULL, source, beginIdx);
		source->beginIdx = endIdx;
		source->endIdx = pushEndIdx;
		return true;
	}
	return false;
}

static kstatus_t KTokenSeq_Eval(KonohaContext *kctx, KTokenSeq *source, KTraceInfo *trace)
{
	kstatus_t status = K_CONTINUE;
	kMethod *mtd = NULL;
	INIT_GCSTACK();
	KTokenSeq tokens = {source->ns, KGetParserContext(kctx)->preparedTokenList};
	KTokenSeq_Push(kctx, tokens);
	while(KTokenSeq_PreprocessSingleStatement(kctx, &tokens, source)) {
		int currentIdx = FindFirstStatementToken(kctx, tokens.tokenList, tokens.beginIdx, tokens.endIdx);
		while(currentIdx < tokens.endIdx) {
			kNode *node = ParseNewNode(kctx, source->ns, tokens.tokenList, &currentIdx, tokens.endIdx, ParseMetaPatternOption, NULL);
			if(mtd == NULL) {
				mtd = KLIB new_kMethod(kctx, _GcStack, kMethod_Static, 0, 0, NULL);
				KLIB kMethod_SetParam(kctx, mtd, KType_Object, 0, NULL);
			}
			status = kNode_Eval(kctx, node, mtd, trace);
			if(status != K_CONTINUE) break;
			currentIdx = FindFirstStatementToken(kctx, tokens.tokenList, currentIdx, tokens.endIdx);
		}
		KTokenSeq_Pop(kctx, tokens);
		if(status != K_CONTINUE) break;
	}
	RESET_GCSTACK();
	return status;
}

/* ------------------------------------------------------------------------ */
