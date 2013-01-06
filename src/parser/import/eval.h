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

static kbool_t PreprocessSingleStatement(KonohaContext *kctx, kNameSpace *ns, KTokenSeq *tokens, KTokenSeq *source)
{
	int beginIdx, endIdx;
	for(beginIdx = source->beginIdx; beginIdx < source->endIdx; beginIdx++) {
		kToken *tk = source->tokenList->TokenItems[beginIdx];
		if(tk->tokenType == TokenType_Indent) continue;
		break;
	}
	for(endIdx = beginIdx; endIdx < source->endIdx; endIdx++) {
		kToken *tk = source->tokenList->TokenItems[endIdx];
		if(tk->tokenType == TokenType_Indent) {
			break;
		}
	}
	if(beginIdx < endIdx) {
//		int pushEndIdx = source->endIdx; source->endIdx = endIdx;
		Preprocess(kctx, ns, source->tokenList, source->beginIdx, endIdx, NULL, tokens->tokenList);
		source->beginIdx = endIdx;
//		source->endIdx = pushEndIdx;
		return true;
	}
	source->beginIdx = endIdx;
	return false;
}

static kstatus_t EvalTokenList(KonohaContext *kctx, KTokenSeq *source, KTraceInfo *trace)
{
	kstatus_t status = K_CONTINUE;
	kMethod *mtd = NULL;
	INIT_GCSTACK();
	KTokenSeq tokens = {source->ns, KGetParserContext(kctx)->preparedTokenList};
	KTokenSeq_Push(kctx, tokens);
	while(PreprocessSingleStatement(kctx,source->ns, &tokens, source)) {
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