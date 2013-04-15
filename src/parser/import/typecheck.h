/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
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

static kUntypedNode *CallTypeFunc(KonohaContext *kctx, kFunc *fo, kUntypedNode *expr, kNameSpace *ns, kObject *reqType)
{
	INIT_GCSTACK();
	BEGIN_UnusedStack(lsfp);
	KStackSetObjectValue(lsfp[1].asNode, expr);
	KStackSetObjectValue(lsfp[2].asNameSpace, ns);
	KStackSetObjectValue(lsfp[3].asObject, reqType);
	CallSugarMethod(kctx, lsfp, fo, 4, UPCAST(K_NULLNODE));
	END_UnusedStack();
	RESET_GCSTACK();
	if(kUntypedNode_IsError(expr)) return expr;
	if(lsfp[K_RTNIDX].asNode == K_NULLNODE) {
		DBG_ASSERT(expr->typeAttr == KType_var); // untyped
	}
	DBG_ASSERT(IS_Node(lsfp[K_RTNIDX].asObject));
	return (kUntypedNode *)lsfp[K_RTNIDX].asObject;
}

static kUntypedNode *TypeNode(KonohaContext *kctx, kSyntax *syn, kUntypedNode *expr, kNameSpace *ns, KClass* reqtc)
{
	kObject *reqType = KLIB Knull(kctx, reqtc);
	int varsize = ns->genv->localScope.varsize;
	if(syn->TypeFuncNULL != NULL) {
		kUntypedNode *texpr = CallTypeFunc(kctx, syn->TypeFuncNULL, expr, ns, reqType);
		if(kUntypedNode_IsError(texpr) || texpr->typeAttr != KType_var) {
			if(!kUntypedNode_Is(OpenBlock, expr)) {
				ns->genv->localScope.varsize = varsize;
			}
			return texpr;
		}
	}
	size_t i;
	kArray *syntaxList = kNameSpace_GetSyntaxList(kctx, ns, syn->keyword);
	for(i = 1; i < kArray_size(syntaxList); i++) { /* ObjectItems[0] == syn */
		kSyntax *syn2 = syntaxList->SyntaxItems[i];
		if(syn2->TypeFuncNULL != NULL) {
			kUntypedNode *texpr = CallTypeFunc(kctx, syn2->TypeFuncNULL, expr, ns, reqType);
			if(kUntypedNode_IsError(texpr) || texpr->typeAttr != KType_var) {
				if(!kUntypedNode_Is(OpenBlock, expr)) {
					ns->genv->localScope.varsize = varsize;
				}
				return texpr;
			}
		}
	}
	if(!kUntypedNode_IsError(expr)) {
		KDump(expr);
		DBG_P("syn->TypeFuncNULL=%p", syn->TypeFuncNULL);
		expr = KLIB MessageNode(kctx, expr, NULL, ns, ErrTag, "undefined typing: %s%s '%s'", KSymbol_Fmt2(syn->keyword), KToken_t(expr->KeyOperatorToken));
		//DBG_ASSERT(kctx == NULL);
	}
	ns->genv->localScope.varsize = varsize;
	return expr;
}

static void PutConstNode(KonohaContext *kctx, kUntypedNode *expr, KonohaStack *sfp)
{
	if(kUntypedNode_node(expr) == KNode_Const) {
		KStackSetObjectValue(sfp[0].asObject, expr->ObjectConstValue);
		KStackSetUnboxValue(sfp[0].unboxValue, kObject_Unbox(expr->ObjectConstValue));
	} else if(kUntypedNode_node(expr) == KNode_New) {
		KStackSetObjectValue(sfp[0].asObject, KLIB new_kObject(kctx, OnField, KClass_(expr->typeAttr), 0));
//	} else if(kUntypedNode_node(expr) == KNode_MethodCall) {   /* case Object Object.boxing(UnboxType Val) */
//		kMethod *mtd = expr->NodeList->MethodItems[0];
//		kUntypedNode *texpr = expr->NodeList->NodeItems[1];
//		assert(mtd->mn == MN_box && kArray_size(expr->NodeList) == 2);
//		assert(KType_Is(UnboxType, expr->typeAttr) == true);
//		KStackSetObjectValue(sfp[0].asObject, KLIB new_kObject(kctx, OnField, KClass_(expr->typeAttr), texpr->unboxConstValue));
	} else {
		assert(kUntypedNode_node(expr) == KNode_Null);
		KStackSetObjectValue(sfp[0].asObject, KLIB Knull(kctx, KClass_(expr->typeAttr)));
		KStackSetUnboxValue(sfp[0].unboxValue, 0);
	}
}

static kUntypedNode *MakeNodeConst(KonohaContext *kctx, kUntypedNode *expr, KClass *rtype)
{
	size_t i, size = kArray_size(expr->NodeList), psize = size - 2;
	kMethod *mtd = expr->NodeList->MethodItems[0];
	BEGIN_UnusedStack(lsfp);
	KStackSetObjectValue(lsfp[K_NSIDX].asNameSpace, kUntypedNode_ns(expr));
	for(i = 1; i < size; i++) {
		PutConstNode(kctx, expr->NodeList->NodeItems[i], lsfp + i - 1);
	}
	KStackSetMethodAll(lsfp, KLIB Knull(kctx, KClass_(expr->typeAttr)), kUntypedNode_uline(expr), mtd, psize);
	KStackCall(lsfp);
	END_UnusedStack();
	if(KClass_Is(UnboxType, rtype) /* || rtype->typeId == KType_void*/) {
		return KLIB kUntypedNode_SetUnboxConst(kctx, expr, rtype->typeId, lsfp[K_RTNIDX].unboxValue);
	}
	return KLIB kUntypedNode_SetConst(kctx, expr, NULL, lsfp[K_RTNIDX].asObject);
}

static kUntypedNode *BoxNode(KonohaContext *kctx, kUntypedNode *expr, kNameSpace *ns, KClass* reqClass)
{
	kUntypedNode *node = KNewNode(ns);
	KTODO("NewNode");
	//KFieldSet(node, node->Expr, expr);
	return kUntypedNode_Type(node, KNode_Box, expr->typeAttr);
}

static kUntypedNode *TypeCheckNode(KonohaContext *kctx, kUntypedNode *expr, kNameSpace *ns, KClass* reqClass, int pol)
{
	DBG_ASSERT(IS_Node(expr) && expr != K_NULLNODE);
	if(kUntypedNode_IsError(expr)) return expr;
	if(KTypeAttr_Unmask(expr->typeAttr) == KType_var) {
		expr = TypeNode(kctx, expr->syn, expr, ns, reqClass);
		if(kUntypedNode_IsError(expr)) return expr;
	}
	KClass *typedClass = KClass_(expr->typeAttr);
	if(reqClass->typeId == KType_void) {
		expr->typeAttr = KType_void;
		return expr;
	}
	if(typedClass->typeId == KType_void) {
		if(!KHalfFlag_Is(pol, TypeCheckPolicy_AllowVoid)) {
			expr = KLIB MessageNode(kctx, expr, NULL, ns, ErrTag, "void is unacceptable");
		}
		return expr;
	}
	if(KClass_Is(TypeVar, typedClass)) {
		return KLIB MessageNode(kctx, expr, NULL, ns, ErrTag, "not type variable %s", KClass_text(typedClass));
	}
	if(reqClass->typeId == KType_var || typedClass == reqClass || KHalfFlag_Is(pol, TypeCheckPolicy_NoCheck)) {
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
		if(kMethod_Is(Coercion, mtd) || KHalfFlag_Is(pol, TypeCheckPolicy_Coercion)) {
			return new_MethodNode(kctx, ns, reqClass, mtd, 1, expr);
		}
		if(kNameSpace_Is(ImplicitCoercion, ns)) {
			KLIB MessageNode(kctx, expr, NULL, ns, InfoTag, "implicit type coercion: %s to %s", KClass_text(typedClass), KClass_text(reqClass));
			return new_MethodNode(kctx, ns, reqClass, mtd, 1, expr);
		}
	}
	DBG_P("%s(%d) is requested, but %s(%d) is given", KClass_text(reqClass), reqClass->typeId, KClass_text(typedClass), typedClass->typeId);
	return KLIB MessageNode(kctx, expr, NULL, ns, ErrTag, "%s is requested, but %s is given", KClass_text(reqClass), KClass_text(typedClass));
}

static kUntypedNode *TypeCheckNodeAt(KonohaContext *kctx, kUntypedNode *node, size_t pos, kNameSpace *ns, KClass *reqClass, int pol)
{
	DBG_ASSERT(IS_Array(node->NodeList));
	DBG_ASSERT(pos < kArray_size(node->NodeList));
	kUntypedNode *expr = node->NodeList->NodeItems[pos];
	//kUntypedNode_SetParent(kctx, expr, node);
	//DBG_ASSERT(kUntypedNode_GetParentNULL(expr) == node);
	kUntypedNode *texpr = TypeCheckNode(kctx, expr, ns, reqClass, pol);
	if(texpr != expr) {
		KFieldSet(node->NodeList, node->NodeList->NodeItems[pos], texpr);
		kUntypedNode_SetParent(kctx, texpr, node);
	}
	return texpr;
}

static kUntypedNode *TypeCheckNodeByName(KonohaContext *kctx, kUntypedNode *stmt, ksymbol_t symbol, kNameSpace *ns, KClass *reqc, int pol)
{
	kUntypedNode *expr = (kUntypedNode *)kUntypedNode_GetObjectNULL(kctx, stmt, symbol);
	if(expr != NULL) {
		if(IS_Token(expr)) {
			kTokenVar *tk = (kTokenVar *)expr;
			kNameSpace *ns = kUntypedNode_ns(stmt);
			if(tk->tokenType == TokenType_LazyBlock) {
				kToken_ToBraceGroup(kctx, (kTokenVar *)tk, ns, NULL);
			}
			if(tk->resolvedSyntaxInfo->keyword == KSymbol_BraceGroup) {
				int beginIdx = 0;
				expr = ParseNewNode(kctx, ns, tk->GroupTokenList, &beginIdx, kArray_size(tk->GroupTokenList), (ParseOption)(ParseMetaPatternOption|ParseBlockOption), NULL);
				KLIB kObjectProto_SetObject(kctx, stmt, symbol, kObject_typeId(expr), expr);
			}
		}
		if(IS_Node(expr)) {
			kUntypedNode_SetParent(kctx, expr, stmt);
			kUntypedNode *texpr = TypeCheckNode(kctx, expr, ns, reqc, pol);
			if(kUntypedNode_IsError(texpr)) {
				if(!kUntypedNode_IsError(stmt)) {
					kUntypedNode_ToError(kctx, stmt, texpr->ErrorMessage);
				}
				return texpr;
			}
			if(texpr != expr) {
				KLIB kObjectProto_SetObject(kctx, stmt, symbol, KType_Node, texpr);
				kUntypedNode_SetParent(kctx, texpr, stmt);
			}
			return texpr;
		}
	}
	if(!KFlag_Is(int, pol, TypeCheckPolicy_AllowEmpty)) {
		return KLIB MessageNode(kctx, stmt, NULL, ns, ErrTag, "%s%s clause is empty", KSymbol_Fmt2(symbol));
	}
	return NULL; //error
}

static kUntypedNode *TypeCheckNodeList(KonohaContext *kctx, kUntypedNode *block, size_t n, kNameSpace *ns, KClass *reqc)
{
	kArray *nodeList = block->NodeList;
	kUntypedNode *stmt = nodeList->NodeItems[n];
	kUntypedNode_SetParent(kctx, stmt, block);
	if(stmt->typeAttr != KType_var) return stmt;
	if(!kUntypedNode_IsError(stmt)) {
		kUntypedNode *tstmt = TypeNode(kctx, stmt->syn, stmt, ns, reqc);
		if(tstmt != stmt) {
			KFieldSet(nodeList, nodeList->NodeItems[n], tstmt);
			kUntypedNode_SetParent(kctx, tstmt, block);
			stmt = tstmt;
		}
	}
	return stmt;
}

static kUntypedNode *TypeCheckBlock(KonohaContext *kctx, kUntypedNode *block, kNameSpace *ns, KClass *reqc)
{
	int i, size = kUntypedNode_GetNodeListSize(kctx, block) - 1;
	kbool_t hasValue = (reqc->typeId != KType_void);
	KDump(block);
	DBG_P(">>>>>>>> size=%d, varsize=%d, reqc=%s", size, ns->genv->localScope.varsize, KClass_text(reqc));

	for(i = 0; i < size; i++) {
		kUntypedNode *stmt = TypeCheckNodeList(kctx, block, i, ns, KClass_void);
		if(kUntypedNode_IsError(stmt)) {
			return hasValue ? stmt : kUntypedNode_Type(block, KNode_Block, KType_void);  // untyped
		}
	}
	if(size >= 0) {
		kUntypedNode *stmt = TypeCheckNodeList(kctx, block, size, ns, reqc);
		if(kUntypedNode_IsError(stmt)) {
			return hasValue ? stmt : kUntypedNode_Type(block, KNode_Block, KType_void);  // untyped
		}
		kUntypedNode_Type(block, KNode_Block, stmt->typeAttr);
	}
	else {
		kUntypedNode_Type(block, KNode_Block, KType_void);
	}
	DBG_P(">>>>>>>> size=%d, typed=%s", size, KType_text(block->typeAttr));
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

//static kUntypedNode *kMethod_ParseBodyNode(KonohaContext *kctx, kMethod *mtd, kNameSpace *ns, kString *source, kfileline_t uline, int baseIndent)
//{
//	const char *script = kString_text(source);
//	KTokenSeq tokens = {ns, KGetParserContext(kctx)->preparedTokenList, 0};
//	KTokenSeq_Push(kctx, tokens);
//	Tokenize(kctx, ns, script, uline, baseIndent, tokens.tokenList);
//	KTokenSeq_End(kctx, tokens);
//	kUntypedNode *node = ParsePreprocessNode(kctx, ns, tokens.tokenList, tokens.beginIdx, tokens.endIdx);
//	KTokenSeq_Pop(kctx, tokens);
//	return node;
//}

static void kNameSpace_InitParam(KonohaContext *kctx, struct KGammaLocalData *genv, kParam *pa, kparamtype_t *callparam)
{
	int i, psize = (pa->psize + 1U < genv->localScope.capacity) ? pa->psize : genv->localScope.capacity - 1;
	for(i = 0; i < psize; i++) {
		genv->localScope.varItems[i+1].name = pa->paramtypeItems[i].name;
		genv->localScope.varItems[i+1].typeAttr = (callparam == NULL) ? pa->paramtypeItems[i].typeAttr : callparam[i].typeAttr;
	}
	if(!kMethod_Is(Static, genv->currentWorkingMethod)) {
		genv->localScope.varItems[0].name = FN_this;
		genv->localScope.varItems[0].typeAttr = genv->thisClass->typeId;
	}
	genv->localScope.varsize = psize+1;
}

static kMethod *kMethod_Compile(KonohaContext *kctx, kMethod *mtd, kparamtype_t *callparamNULL, kNameSpace *ns, kString *text, kfileline_t uline, int baseIndent, int options)
{
	INIT_GCSTACK();
	int errorCount = KGetParserContext(kctx)->errorMessageCount;
	kParam *param = kMethod_GetParam(mtd);
	if(callparamNULL != NULL) {
		//DynamicComplie();
	}
	kUntypedNode *node = ParseSource(kctx, ns, kString_text(text), uline, baseIndent);
	struct KGammaLocalData newgma = {0};
	KGammaStackDecl *lvarItems = ALLOCA(KGammaStackDecl, (32 + param->psize));
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
	kMethod_Set(StaticError, ((kMethodVar *)mtd), (KGetParserContext(kctx)->errorMessageCount > errorCount));
	RESET_GCSTACK();
	return mtd;
}
