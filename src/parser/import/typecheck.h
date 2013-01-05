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

static kNode *kNode_Rebase(KonohaContext *kctx, kNode *node, size_t stackbase)
{
	if(!kNode_IsValue(node)/* && node->stackbase != stackbase*/) {
		size_t i, size = kNode_GetNodeListSize(kctx, node);
		if(node->node == KNode_Block) {
			for(i = 0; i < size; i++) {
				kNode *sub = node->NodeList->NodeItems[i];
				if(kNode_IsValue(sub)) continue;
				kNode_Rebase(kctx, sub, stackbase + (sub->stackbase - node->stackbase));
			}
		}
		else if(IS_Array(node->NodeList)) {
			for(i = 1; i < size; i++) {
				kNode *sub = node->NodeList->NodeItems[i];
				if(kNode_IsValue(sub)) continue;
				kNode_Rebase(kctx, sub, (i - 1) + stackbase + K_CALLDELTA);
			}
		}
		else if(IS_Node(node->NodeToPush)) {
			kNode_Rebase(kctx, node->NodeToPush, stackbase);
		}
		node->stackbase = stackbase;
	}
	return node;
}

static kNode *CallTypeFunc(KonohaContext *kctx, kFunc *fo, kNode *expr, kNameSpace *ns, kObject *reqType)
{
	INIT_GCSTACK();
	BEGIN_UnusedStack(lsfp);
	KUnsafeFieldSet(lsfp[1].asNode, expr);
	KUnsafeFieldSet(lsfp[2].asNameSpace, ns);
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

static kNode *TypeNode(KonohaContext *kctx, kSyntax *syn, kNode *expr, kNameSpace *ns, KClass* reqtc)
{
	kObject *reqType = KLIB Knull(kctx, reqtc);
	int varsize = ns->genv->localScope.varsize;
	expr->stackbase = varsize;
//	DBG_P(">>>>>>>>>> #stackbase = %d", varsize);
//	if(KFlag_Is(kshortflag_t, syn->flag, SYNFLAG_CallNode)) {
//		KPushMethodCall(gma);
//		KPushMethodCall(gma);
//		KPushMethodCall(gma);
//		KPushMethodCall(gma);
//	}
	if(syn->sugarFuncTable[KSugarTypeFunc] != NULL) {
		kNode *texpr = CallTypeFunc(kctx, syn->sugarFuncTable[KSugarTypeFunc], expr, ns, reqType);
		if(kNode_IsError(texpr) || texpr->attrTypeId != KType_var) {
			if(!kNode_Is(OpenBlock, expr)) {
				ns->genv->localScope.varsize = varsize;
			}
			return texpr;
		}
	}
	size_t i;
	kArray *syntaxList = kNameSpace_GetSyntaxList(kctx, ns, syn->keyword);
	for(i = 1; i < kArray_size(syntaxList); i++) { /* ObjectItems[0] == syn */
		kSyntax *syn2 = syntaxList->SyntaxItems[i];
		if(syn2->sugarFuncTable[KSugarTypeFunc] != NULL) {
			kNode *texpr = CallTypeFunc(kctx, syn2->sugarFuncTable[KSugarTypeFunc], expr, ns, reqType);
			if(kNode_IsError(texpr) || texpr->attrTypeId != KType_var) {
				if(!kNode_Is(OpenBlock, expr)) {
					ns->genv->localScope.varsize = varsize;
				}
				return texpr;
			}
		}
	}
	if(!kNode_IsError(expr)) {
		KDump(expr);
		expr = SUGAR MessageNode(kctx, expr, NULL, ns, ErrTag, "undefined typing: %s%s %s", KSymbol_Fmt2(syn->keyword), KToken_t(expr->KeyOperatorToken));
		//DBG_ASSERT(kctx == NULL);
	}
	ns->genv->localScope.varsize = varsize;
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

static kNode *BoxNode(KonohaContext *kctx, kNode *expr, kNameSpace *ns, KClass* reqClass)
{
	kNode *node = KNewNode(ns);
	KFieldSet(node, node->NodeToPush, expr);
	return kNode_Type(kctx, node, KNode_Box, expr->attrTypeId);
}

static kNode *TypeCheckNode(KonohaContext *kctx, kNode *expr, kNameSpace *ns, KClass* reqClass, int pol)
{
	DBG_ASSERT(IS_Node(expr) && expr != K_NULLNODE);
	if(kNode_IsError(expr)) return expr;
	if(KTypeAttr_Unmask(expr->attrTypeId) == KType_var) {
		expr = TypeNode(kctx, expr->syn, expr, ns, reqClass);
		if(kNode_IsError(expr)) return expr;
	}
	KClass *typedClass = KClass_(expr->attrTypeId);
	if(reqClass->typeId == KType_void) {
		expr->attrTypeId = KType_void;
		return expr;
	}
	if(typedClass->typeId == KType_void) {
		if(!FLAG_is(pol, TypeCheckPolicy_AllowVoid)) {
			expr = SUGAR MessageNode(kctx, expr, NULL, ns, ErrTag, "void is unacceptable");
		}
		return expr;
	}
	if(KClass_Is(TypeVar, typedClass)) {
		return SUGAR MessageNode(kctx, expr, NULL, ns, ErrTag, "not type variable %s", KClass_text(typedClass));
	}
	if(reqClass->typeId == KType_var || typedClass == reqClass || FLAG_is(pol, TypeCheckPolicy_NoCheck)) {
		return expr;
	}
	if(KClass_Isa(kctx, typedClass, reqClass)) {
		if(KClass_Is(UnboxType, typedClass) && !KClass_Is(UnboxType, reqClass)) {
			return BoxNode(kctx, expr, ns, reqClass);
		}
		return expr;
	}
	kMethod *mtd = kNameSpace_GetCoercionMethodNULL(kctx, ns, typedClass, reqClass);
	if(mtd != NULL) {
		if(kMethod_Is(Coercion, mtd) || FLAG_is(pol, TypeCheckPolicy_Coercion)) {
			return new_MethodNode(kctx, ns, reqClass, mtd, 1, expr);
		}
		if(kNameSpace_Is(ImplicitCoercion, ns)) {
			SUGAR MessageNode(kctx, expr, NULL, ns, InfoTag, "implicit type coercion: %s to %s", KClass_text(typedClass), KClass_text(reqClass));
			return new_MethodNode(kctx, ns, reqClass, mtd, 1, expr);
		}
	}
	DBG_P("%s(%d) is requested, but %s(%d) is given", KClass_text(reqClass), reqClass->typeId, KClass_text(typedClass), typedClass->typeId);
	return SUGAR MessageNode(kctx, expr, NULL, ns, ErrTag, "%s is requested, but %s is given", KClass_text(reqClass), KClass_text(typedClass));
}

static kNode* TypeCheckNodeAt(KonohaContext *kctx, kNode *node, size_t pos, kNameSpace *ns, KClass *reqClass, int pol)
{
	DBG_ASSERT(IS_Array(node->NodeList));
	DBG_ASSERT(pos < kArray_size(node->NodeList));
	kNode *expr = node->NodeList->NodeItems[pos];
	//kNode_SetParent(kctx, expr, node);
	//DBG_ASSERT(kNode_GetParentNULL(expr) == node);
	kNode *texpr = TypeCheckNode(kctx, expr, ns, reqClass, pol);
	if(texpr != expr) {
		KFieldSet(node->NodeList, node->NodeList->NodeItems[pos], texpr);
		kNode_SetParent(kctx, texpr, node);
	}
	return texpr;
}

static kNode* TypeCheckNodeByName(KonohaContext *kctx, kNode *stmt, ksymbol_t symbol, kNameSpace *ns, KClass *reqClass, int pol)
{
	kNode *expr = (kNode *)kNode_GetObjectNULL(kctx, stmt, symbol);
	if(expr != NULL) {
		if(IS_Token(expr)) {
			kTokenVar *tk = (kTokenVar *)expr;
			kNameSpace *ns = kNode_ns(stmt);
			if(tk->tokenType == TokenType_LazyBlock) {
				kToken_ToBraceGroup(kctx, (kTokenVar *)tk, ns, NULL);
			}
			if(tk->resolvedSyntaxInfo->keyword == KSymbol_BraceGroup) {
				int beginIdx = 0;
				expr = ParseNewNode(kctx, ns, tk->GroupTokenList, &beginIdx, kArray_size(tk->GroupTokenList), ParseMetaPatternOption|ParseBlockOption, NULL);
				KLIB kObjectProto_SetObject(kctx, stmt, symbol, kObject_typeId(expr), expr);
			}
		}
		if(IS_Node(expr)) {
			kNode_SetParent(kctx, expr, stmt);
			kNode *texpr = TypeCheckNode(kctx, expr, ns, reqClass, pol);
			if(kNode_IsError(texpr)) {
				kNode_ToError(kctx, stmt, texpr->ErrorMessage);
				return texpr;
			}
			if(texpr != expr) {
				KLIB kObjectProto_SetObject(kctx, stmt, symbol, KType_Node, texpr);
				kNode_SetParent(kctx, texpr, stmt);
			}
			return texpr;
		}
	}
	return NULL; //error
}

static kNode* TypeCheckNodeList(KonohaContext *kctx, kNode *block, size_t n, kNameSpace *ns, KClass *reqc)
{
	kArray *nodeList = block->NodeList;
	kNode *stmt = nodeList->NodeItems[n];
	kNode_SetParent(kctx, stmt, block);
	if(stmt->attrTypeId != KType_var) return stmt;
	if(!kNode_IsError(stmt)) {
		kNode *tstmt = TypeNode(kctx, stmt->syn, stmt, ns, reqc);
		if(tstmt != stmt) {
			KFieldSet(nodeList, nodeList->NodeItems[n], tstmt);
			kNode_SetParent(kctx, tstmt, block);
		}
	}
	return stmt;
}

static kNode* TypeCheckBlock(KonohaContext *kctx, kNode *block, kNameSpace *ns, KClass *reqc)
{
	DBG_P(">>>>>>>>");
	KDump(block);
	int i, size = kNode_GetNodeListSize(kctx, block) - 1;
	for(i = 0; i < size; i++) {
		kNode *stmt = TypeCheckNodeList(kctx, block, i, ns, KClass_void);
		if(kNode_IsError(stmt)) {
			return reqc->typeId == KType_void ? kNode_Type(kctx, block, KNode_Block, KType_void) : stmt;  // untyped
		}
	}
	if(size >= 0) {
		kNode *stmt = TypeCheckNodeList(kctx, block, size, ns, reqc);
		if(kNode_IsError(stmt)) {
			return stmt;
		}
		kNode_Type(kctx, block, KNode_Block, stmt->attrTypeId == KType_var ? KType_void : stmt->attrTypeId);
	}
	else {
		kNode_Type(kctx, block, KNode_Block, KType_void);
	}
	return block;
}

/* ------------------------------------------------------------------------ */

static struct KGammaLocalData *kNameSpace_PushGamma(KonohaContext *kctx, kNameSpace *ns, struct KGammaLocalData *newone)
{
	struct KGammaLocalData *oldone = ns->genv;
	ns->genv = newone;
	return oldone;
}

static struct KGammaLocalData *kNameSpace_PopGamma(KonohaContext *kctx, kNameSpace *ns, struct KGammaLocalData *oldone, struct KGammaLocalData *checksum)
{
	struct KGammaLocalData *newone = ns->genv;
	assert(checksum == newone);
	ns->genv = oldone;
	if(newone->localScope.allocsize > 0) {
		KFree(newone->localScope.varItems, newone->localScope.allocsize);
	}
	return newone;
}

#define KPushGammaStack(G,B) struct KGammaLocalData *oldbuf_ = kNameSpace_PushGamma(kctx, G, B)
#define KPopGammaStack(G,B)  kNameSpace_PopGamma(kctx, G, oldbuf_, B)

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

static void kNameSpace_InitParam(KonohaContext *kctx, struct KGammaLocalData *genv, kParam *pa, kparamtype_t *callparam)
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
	kNode *node = kMethod_ParseBodyNode(kctx, mtd, ns, text, uline);
	struct KGammaLocalData newgma = {0};
	KGammaStackDecl lvarItems[32 + param->psize];
	bzero(lvarItems, sizeof(KGammaStackDecl) * (32 + param->psize));
	newgma.currentWorkingMethod = mtd;
	newgma.thisClass = KClass_((mtd)->typeId);
	newgma.localScope.varItems = lvarItems;
	newgma.localScope.capacity = 32 + param->psize;
	newgma.localScope.varsize = 0;
	newgma.localScope.allocsize = 0;

	KPushGammaStack(ns, &newgma);
	kNameSpace_InitParam(kctx, &newgma, param, callparamNULL);
	node = TypeCheckNode(kctx, node, ns, KClass_void, 0);
	KLIB kMethod_GenCode(kctx, mtd, node, options);
	KPopGammaStack(ns, &newgma);
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
		DBG_ASSERT(stmt->stackbase == 0);
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
	kNameSpace *ns = kNode_ns(stmt);
	KGammaStackDecl lvarItems[32] = {};
	struct KGammaLocalData newgma = {0};
	newgma.flag = kNameSpace_TopLevel;
	newgma.currentWorkingMethod = mtd;
	newgma.thisClass     = KClass_NameSpace;
	newgma.localScope.varItems  = lvarItems;
	newgma.localScope.capacity  = 32;
	newgma.localScope.varsize   = 0;
	newgma.localScope.allocsize = 0;

	KPushGammaStack(ns, &newgma);
//	kNameSpace_InitIt(kctx, &newns, kMethod_GetParam(mtd));
	stmt = TypeCheckNode(kctx, stmt, ns, KClass_var, TypeCheckPolicy_AllowVoid);
	KPopGammaStack(ns, &newgma);
	if(kNode_IsError(stmt)) {
		return K_BREAK;  // to avoid duplicated error message
	}
	KDump(stmt);
	kbool_t isTryEval = true;
	if(KonohaContext_Is(CompileOnly, kctx)) {
		isTryEval = false;
		if(stmt->node == KNode_MethodCall) {
			kMethod *callMethod = stmt->NodeList->MethodItems[0];
			DBG_ASSERT(IS_Method(callMethod));
			if(kMethod_Is(Compilation, callMethod)) {
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

static kbool_t PreprocessSingleStatement(KonohaContext *kctx, KTokenSeq *tokens, KTokenSeq *source)
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
	while(PreprocessSingleStatement(kctx, &tokens, source)) {
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
