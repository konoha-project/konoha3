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

static void kNode_AddParsedObject(KonohaContext *kctx, kNode *stmt, ksymbol_t keyid, kObject *o)
{
	kArray* valueList = (kArray *)KLIB kObject_getObject(kctx, stmt, keyid, NULL);
	if(valueList == NULL) {
		KLIB kObjectProto_SetObject(kctx, stmt, keyid, kObject_typeId(o), o);
	}
	else {
		//DBG_P(">>> keyid=%s%s valueList=%s, value=%s", KSymbol_Fmt2(keyid), KClass_text(kObject_class(valueList)), KClass_text(kObject_class(o)));
		if(!IS_Array(valueList)) {
			INIT_GCSTACK();
			kArray *newList = /*G*/new_(Array, 0, _GcStack);
			KLIB kArray_Add(kctx, newList, valueList);
			KLIB kObjectProto_SetObject(kctx, stmt, keyid, kObject_typeId(newList), newList);
			valueList = newList;
			RESET_GCSTACK();
		}
		KLIB kArray_Add(kctx, valueList, o);
	}
}

static int kNameSpace_FindEndOfStatement(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx)
{
	int c, isNoSemiColon = kNameSpace_IsAllowed(NoSemiColon, ns);
	for(c = beginIdx; c < endIdx; c++) {
		kToken *tk = tokenList->TokenItems[c];
		if(kToken_Is(StatementSeparator, tk)) return c;
		if(isNoSemiColon && kToken_IsIndent(tk)) {
			return c;
		}
	}
	return endIdx;
}

static KMETHOD PatternMatch_Nodeession(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	int returnIdx = -1;
	endIdx = kNameSpace_FindEndOfStatement(kctx, Node_ns(stmt), tokenList, beginIdx, endIdx);
	if(beginIdx < endIdx) {
		INIT_GCSTACK();
		kNode *expr = kNode_ParseNode(kctx, stmt, tokenList, beginIdx, endIdx, NULL);
		if(expr != K_NULLNODE) {
			//KdumpNode(kctx, expr);
			kNode_AddParsedObject(kctx, stmt, name, UPCAST(expr));
			returnIdx = endIdx;
		}
		RESET_GCSTACK();
	}
	KReturnUnboxValue(returnIdx);
}

static KMETHOD PatternMatch_Type(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	KClass *foundClass = NULL;
	int returnIdx = TokenUtils_ParseTypePattern(kctx, Node_ns(stmt), tokenList, beginIdx, endIdx, &foundClass);
	DBG_P("tk=%s, returnIdx=%d", tokenList->TokenItems[beginIdx], returnIdx);
	if(foundClass != NULL) {
		kTokenVar *tk = new_(TokenVar, 0, OnVirtualField);
		kNode_AddParsedObject(kctx, stmt, name, UPCAST(tk));
		kToken_SetTypeId(kctx, tk, Node_ns(stmt), foundClass->typeId);
	}
	KReturnUnboxValue(returnIdx);
}

static KMETHOD PatternMatch_MethodName(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	kTokenVar *tk = tokenList->TokenVarItems[beginIdx];
	int returnIdx = -1;
	if(tk->resolvedSyntaxInfo->keyword == KSymbol_SymbolPattern || tk->resolvedSyntaxInfo->precedence_op1 > 0 || tk->resolvedSyntaxInfo->precedence_op2 > 0) {
		kNode_AddParsedObject(kctx, stmt, name, UPCAST(tk));
		returnIdx = beginIdx + 1;
	}
	KReturnUnboxValue(returnIdx);
}

static void KTokenSeq_CheckCStyleParam(KonohaContext *kctx, KTokenSeq* tokens)
{
	int i;
	for(i = 0; i < tokens->endIdx; i++) {
		kTokenVar *tk = tokens->tokenList->TokenVarItems[i];
		if(tk->resolvedSymbol == KSymbol_void) {
			tokens->endIdx = i; //  f(void) = > f()
			return;
		}
		if(tk->resolvedSyntaxInfo->keyword == KSymbol_COMMA) {
			kToken_Set(StatementSeparator, tk, true);
		}
	}
}

static KMETHOD PatternMatch_CStyleParam(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	int returnIdx = -1;
	kToken *tk = tokenList->TokenItems[beginIdx];
	if(tk->resolvedSyntaxInfo->keyword == KSymbol_ParenthesisGroup) {
		KTokenSeq param = {Node_ns(stmt), tk->subTokenList, 0, kArray_size(tk->subTokenList)};
		KTokenSeq_CheckCStyleParam(kctx, &param);
		kNode *bk = new_BlockNode(kctx, stmt, NULL, &param);
		kNode_AddParsedObject(kctx, stmt, name, UPCAST(bk));
		returnIdx = beginIdx + 1;
	}
	KReturnUnboxValue(returnIdx);
}

static KMETHOD PatternMatch_CStyleNode(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	kToken *tk = tokenList->TokenItems[beginIdx];
//	KdumpTokenArray(kctx, tokenList, beginIdx, endIdx);
	if(tk->resolvedSyntaxInfo->keyword == TokenType_CODE || tk->resolvedSyntaxInfo->keyword == KSymbol_BraceGroup) {
		kNode_AddParsedObject(kctx, stmt, name, UPCAST(tk));
		KReturnUnboxValue(beginIdx+1);
	}
	int newEndIdx = kNameSpace_FindEndOfStatement(kctx, Node_ns(stmt), tokenList, beginIdx, endIdx);
	KTokenSeq tokens = {Node_ns(stmt), tokenList, beginIdx, newEndIdx};
	kNode *bk = new_BlockNode(kctx, stmt, NULL, &tokens);
	kNode_AddParsedObject(kctx, stmt, name, UPCAST(bk));
	KReturnUnboxValue(newEndIdx);
}

static KMETHOD PatternMatch_Token(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	DBG_ASSERT(beginIdx < endIdx);
	kToken *tk = tokenList->TokenItems[beginIdx];
	if(!kToken_Is(StatementSeparator, tk) && !kToken_IsIndent(tk)) {
		kNode_AddParsedObject(kctx, stmt, name, UPCAST(tk));
		KReturnUnboxValue(beginIdx+1);
	}
	KReturnUnboxValue(-2);
}

static KMETHOD PatternMatch_TypeDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	KClass *foundClass = NULL;
	int nextIdx = TokenUtils_ParseTypePattern(kctx, Node_ns(stmt), tokenList, beginIdx, endIdx, &foundClass);
	//DBG_P("@ nextIdx = %d < %d", nextIdx, endIdx);
	if(nextIdx != -1) {
		nextIdx = TokenUtils_SkipIndent(tokenList, nextIdx, endIdx);
		if(nextIdx < endIdx) {
			kToken *tk = tokenList->TokenItems[nextIdx];
			if(tk->resolvedSyntaxInfo->keyword == KSymbol_SymbolPattern) {
				KReturnUnboxValue(beginIdx);
			}
		}
	}
	KReturnUnboxValue(-1);
}

static KMETHOD PatternMatch_MethodDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	kNameSpace *ns = Node_ns(stmt);
	KClass *foundClass = NULL;
	int nextIdx = TokenUtils_ParseTypePattern(kctx, ns, tokenList, beginIdx, endIdx, &foundClass);
	//DBG_P("@ nextIdx = %d < %d", nextIdx, endIdx);
	if(nextIdx != -1) {
		nextIdx = TokenUtils_SkipIndent(tokenList, nextIdx, endIdx);
		if(nextIdx < endIdx) {
			kToken *tk = tokenList->TokenItems[nextIdx];
			if(TokenUtils_ParseTypePattern(kctx, ns, tokenList, nextIdx, endIdx, NULL) != -1) {
				KReturnUnboxValue(beginIdx);
			}
			if(tk->resolvedSyntaxInfo->keyword == KSymbol_SymbolPattern) {
				int symbolNextIdx = TokenUtils_SkipIndent(tokenList, nextIdx + 1, endIdx);
				if(symbolNextIdx < endIdx && tokenList->TokenItems[symbolNextIdx]->resolvedSyntaxInfo->keyword == KSymbol_ParenthesisGroup) {
					KReturnUnboxValue(beginIdx);
				}
				KReturnUnboxValue(-1);
			}
			if(tk->resolvedSyntaxInfo->keyword != KSymbol_DOT && ((tk->resolvedSyntaxInfo->precedence_op1 > 0 || tk->resolvedSyntaxInfo->precedence_op2 > 0))) {
				KReturnUnboxValue(beginIdx);
			}
		}
	}
	KReturnUnboxValue(-1);
}

/* ------------------------------------------------------------------------ */

static KMETHOD Nodeession_ParsedNode(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Nodeession(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	if(beginIdx == operatorIdx) {
		kToken *tk = tokenList->TokenItems[operatorIdx];
		DBG_ASSERT(IS_Node(tk->parsedNode));
		KReturn(tk->parsedNode);
	}
}

static KMETHOD Nodeession_Term(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Nodeession(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	if(beginIdx == operatorIdx) {
		kToken *tk = tokenList->TokenItems[operatorIdx];
		KClass *foundClass = NULL;
		int nextIdx = TokenUtils_ParseTypePattern(kctx, Node_ns(stmt), tokenList, beginIdx, endIdx, &foundClass);
		if(foundClass != NULL) {
			kToken_SetTypeId(kctx, tk, Node_ns(stmt), foundClass->typeId);
		}
		else {
			nextIdx = operatorIdx + 1;
		}
		KReturn(kNode_RightJoinNode(kctx, stmt, new_TermNode(kctx, tk), tokenList, nextIdx, endIdx));
	}
}

static KMETHOD Nodeession_OperatorMethod(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Nodeession(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	if(/*syn->keyword != KSymbol_LET && */syn->sugarFuncTable[SugarFunc_TypeCheck] == NULL) {
		DBG_P("switching type checker of %s%s to MethodCall ..", KSymbol_Fmt2(syn->keyword));
		syn = KSyntax_(Node_ns(stmt), KSymbol_NodeMethodCall);  // switch type checker
	}
	kTokenVar *tk = tokenList->TokenVarItems[operatorIdx];
	kNode *expr, *rexpr = kNode_ParseNode(kctx, stmt, tokenList, operatorIdx + 1, endIdx, KToken_t(tk));
	if(beginIdx == operatorIdx) { // unary operator
		expr = new_UntypedCallStyleNode(kctx, syn, 2, tk, rexpr);
	}
	else {   // binary operator
		kNode *lexpr = kNode_ParseNode(kctx, stmt, tokenList, beginIdx, operatorIdx, NULL);
		expr = new_UntypedCallStyleNode(kctx, syn, 3, tk, lexpr, rexpr);
	}
	KReturn(expr);
}

static inline kbool_t isFieldName(kArray *tokenList, int operatorIdx, int endIdx)
{
	if(operatorIdx + 1 < endIdx) {
		kToken *tk = tokenList->TokenItems[operatorIdx + 1];
		return (tk->resolvedSyntaxInfo->keyword == KSymbol_SymbolPattern);
	}
	return false;
}

static KMETHOD Nodeession_DOT(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Nodeession(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	if(beginIdx < operatorIdx && isFieldName(tokenList, operatorIdx, endIdx)) {
		kNode *expr = kNode_ParseNode(kctx, stmt, tokenList, beginIdx, operatorIdx, NULL);
		expr = new_UntypedCallStyleNode(kctx, syn, 2, tokenList->TokenItems[operatorIdx +1], expr);
		KReturn(kNode_RightJoinNode(kctx, stmt, expr, tokenList, operatorIdx +2, endIdx));
	}
}

static KMETHOD Nodeession_Parenthesis(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Nodeession(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	kToken *tk = tokenList->TokenItems[operatorIdx];
	if(beginIdx == operatorIdx) {
		kNode *expr = kNode_ParseNode(kctx, stmt, tk->subTokenList, 0, kArray_size(tk->subTokenList), "(");
		KReturn(kNode_RightJoinNode(kctx, stmt, expr, tokenList, operatorIdx + 1, endIdx));
	}
	else {
		kNode *lexpr = kNode_ParseNode(kctx, stmt, tokenList, beginIdx, operatorIdx, NULL);
		if(lexpr == K_NULLNODE) {
			KReturn(lexpr);
		}
		if(lexpr->syn->keyword == KSymbol_DOT) {
			((kNodeVar *)lexpr)->syn = KSyntax_(Node_ns(stmt), KSymbol_NodeMethodCall); // CALL
		}
		else if(lexpr->syn->keyword != KSymbol_NodeMethodCall) {
			syn = KSyntax_(Node_ns(stmt), KSymbol_ParenthesisGroup);    // (f null ())
			lexpr  = new_UntypedCallStyleNode(kctx, syn, 2, lexpr, K_NULL);
		}
		if(kArray_size(tk->subTokenList) > 0) {
			lexpr = kNode_AddNodeParam(kctx, stmt, lexpr, tk->subTokenList, 0, kArray_size(tk->subTokenList), "(");
		}
		KReturn(kNode_RightJoinNode(kctx, stmt, lexpr, tokenList, operatorIdx + 1, endIdx));
	}
}

static KMETHOD Nodeession_COMMA(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Nodeession(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	kNode *expr = new_UntypedCallStyleNode(kctx, syn, 1, tokenList->TokenItems[operatorIdx]);
	expr = kNode_AddNodeParam(kctx, stmt, expr, tokenList, beginIdx, endIdx, 0/*allowEmpty*/);
	KReturn(expr);
}

static KMETHOD Nodeession_DOLLAR(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Nodeession(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	if(beginIdx == operatorIdx && operatorIdx +1 < endIdx) {
		kToken *tk = tokenList->TokenItems[operatorIdx +1];
		if(tk->resolvedSyntaxInfo->keyword == TokenType_CODE) {
			kToken_ToBraceGroup(kctx, (kTokenVar *)tk, Node_ns(stmt), NULL);
		}
		if(tk->resolvedSyntaxInfo->keyword == KSymbol_BraceGroup) {
			kNodeVar *expr = new_(NodeVar, KSyntax_(Node_ns(stmt), KSymbol_NodePattern), OnGcStack);
			KTokenSeq range = {Node_ns(stmt), tk->subTokenList, 0, kArray_size(tk->subTokenList)};
			KFieldSet(expr, expr->block, SUGAR new_BlockNode(kctx, stmt, NULL, &range));
			KReturn(expr);
		}
	}
}

static kNode* NewNode(KonohaContext *kctx, KSyntax *syn, kToken *tk, ktypeattr_t ty)
{
	kNodeVar *expr = new_(NodeVar, syn, OnGcStack);
	KFieldSet(expr, expr->TermToken, tk);
	kNode_SetTerm(expr, 1);
	expr->node = KNode_NEW;
	expr->attrTypeId = ty;
	return (kNode *)expr;
}

static KMETHOD Nodeession_new(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Nodeession(stmt, tokenList, beginIdx, currentIdx, endIdx);
	DBG_ASSERT(beginIdx == currentIdx);
	if(beginIdx + 1 < endIdx) {
		kTokenVar *newToken = tokenList->TokenVarItems[beginIdx];   // new Class (
		KClass *foundClass = NULL;
		kNameSpace *ns = Node_ns(stmt);
		int nextIdx = SUGAR TokenUtils_ParseTypePattern(kctx, ns, tokenList, beginIdx + 1, endIdx, &foundClass);
		if(foundClass == NULL) {
			kToken *classNameToken = tokenList->TokenVarItems[beginIdx+1];
			KReturn(SUGAR kNode_Message2(kctx, stmt, classNameToken, ErrTag, "not class: %s", KToken_t(classNameToken)));
		}
		if((size_t)nextIdx < kArray_size(tokenList)) {
			kToken *nextTokenAfterClassName = tokenList->TokenItems[nextIdx];
			if(nextTokenAfterClassName->resolvedSyntaxInfo->keyword == KSymbol_ParenthesisGroup) {  // new C (...)
				KSyntax *syn = KSyntax_(ns, KSymbol_NodeMethodCall);
				kNode *expr = SUGAR new_UntypedCallStyleNode(kctx, syn, 2, newToken, NewNode(kctx, syn, tokenList->TokenVarItems[beginIdx+1], foundClass->typeId));
				newToken->resolvedSymbol = MN_new;
				KReturn(expr);
			}
		}
	}
}

/* ------------------------------------------------------------------------ */
/* Nodeession TyCheck */

static kString *kToken_ResolveEscapeSequence(KonohaContext *kctx, kToken *tk, size_t start)
{
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	const char *text = kString_text(tk->text) + start;
	const char *end  = kString_text(tk->text) + kString_size(tk->text);
	KLIB KBuffer_Write(kctx, &wb, kString_text(tk->text), start);
	while(text < end) {
		int ch = *text;
		if(ch == '\\' && *(text+1) != '\0') {
			switch (*(text+1)) {
			/*
			 * compatible with ECMA-262
			 * http://ecma-international.org/ecma-262/5.1/#sec-7.8.4
			 */
			case 'b':  ch = '\b'; text++; break;
			case 't':  ch = '\t'; text++; break;
			case 'n':  ch = '\n'; text++; break;
			case 'v':  ch = '\v'; text++; break;
			case 'f':  ch = '\f'; text++; break;
			case 'r':  ch = '\r'; text++; break;
			case '"':  ch = '"';  text++; break;
			case '\'': ch = '\''; text++; break;
			case '\\': ch = '\\'; text++; break;
			default:	return NULL;
			}
		}
		{
			char buf[1] = {ch};
			KLIB KBuffer_Write(kctx, &wb, (const char *)buf, 1);
		}
		text++;
	}
	return KLIB KBuffer_Stringfy(kctx, &wb, OnGcStack, StringPolicy_FreeKBuffer);
}

static KMETHOD TypeCheck_TextLiteral(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->TermToken;
	kString *text = tk->text;
	if(kToken_Is(RequiredReformat, tk)) {
		const char *escape = strchr(kString_text(text), '\\');
		DBG_ASSERT(escape != NULL);
		text = kToken_ResolveEscapeSequence(kctx, tk, escape - kString_text(text));
		if(text == NULL) {
			KReturn(ERROR_UndefinedEscapeSequence(kctx, stmt, tk));
		}
	}
	kString_Set(Literal, ((kStringVar *)text), true);
	KReturn(SUGAR kNode_SetConstValue(kctx, expr, NULL, UPCAST(text)));
}

static KMETHOD TypeCheck_Type(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	DBG_ASSERT(Token_isVirtualTypeLiteral(expr->TermToken));
	KReturn(SUGAR kNode_SetVariable(kctx, expr, gma, KNode_NULL, expr->TermToken->resolvedTypeId, 0));
}

static KMETHOD TypeCheck_true(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	KReturn(SUGAR kNode_SetUnboxConstValue(kctx, expr, KType_boolean, (uintptr_t)1));
}

static KMETHOD TypeCheck_false(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	KReturn(SUGAR kNode_SetUnboxConstValue(kctx, expr, KType_boolean, (uintptr_t)0));
}

static KMETHOD TypeCheck_IntLiteral(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->TermToken;
	long long n = strtoll(kString_text(tk->text), NULL, 0);
	KReturn(SUGAR kNode_SetUnboxConstValue(kctx, expr, KType_int, (uintptr_t)n));
}

static KMETHOD TypeCheck_AndOperator(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	if(SUGAR kNode_TypeCheckNodeAt(kctx, stmt, expr, 1, gma, KClass_Boolean, 0) != K_NULLNODE) {
		if(SUGAR kNode_TypeCheckNodeAt(kctx, stmt, expr, 2, gma, KClass_Boolean, 0) != K_NULLNODE) {
			KReturn(kNode_typed(expr, AND, KType_boolean));
		}
	}
}

static KMETHOD TypeCheck_OrOperator(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	if(SUGAR kNode_TypeCheckNodeAt(kctx, stmt, expr, 1, gma, KClass_Boolean, 0) != K_NULLNODE) {
		if(SUGAR kNode_TypeCheckNodeAt(kctx, stmt, expr, 2, gma, KClass_Boolean, 0) != K_NULLNODE) {
			KReturn(kNode_typed(expr, OR, KType_boolean));
		}
	}
}

static kbool_t kNode_IsGetter(kNode *expr)
{
	if(expr->node == KNode_CALL) {  // check getter and transform to setter
		kMethod *mtd = expr->NodeList->MethodItems[0];
		DBG_ASSERT(IS_Method(mtd));
		if(KMethodName_IsGetter(mtd->mn)) return true;
	}
	return false;
}

static kNode* kNodeNode_ToSetter(KonohaContext *kctx, kNode *stmt, kNodeVar *expr, kGamma *gma, kNode *rightHandNode, KClass *reqClass)
{
	kMethod *mtd = expr->NodeList->MethodItems[0];
	DBG_ASSERT(KMethodName_IsGetter(mtd->mn));
	kNameSpace *ns = Node_ns(stmt);  // leftHandNode = rightHandNode
	KClass *c = KClass_(mtd->typeId);
	kParam *pa = kMethod_GetParam(mtd);
	int i, psize = pa->psize + 1;
	kparamtype_t p[psize];
	for(i = 0; i < pa->psize; i++) {
		p[i].attrTypeId = pa->paramtypeItems[i].attrTypeId;
	}
	p[pa->psize].attrTypeId = expr->attrTypeId;
	kparamId_t paramdom = KLIB Kparamdom(kctx, psize, p);
	kMethod *foundMethod = kNameSpace_GetMethodBySignatureNULL(kctx, ns, c, KMethodName_ToSetter(mtd->mn), paramdom, psize, p);
	if(foundMethod == NULL) {
		p[pa->psize].attrTypeId = pa->rtype;   /* transform "T1 A.get(T2)" to "void A.set(T2, T1)" */
		paramdom = KLIB Kparamdom(kctx, psize, p);
		foundMethod = kNameSpace_GetMethodBySignatureNULL(kctx, ns, c, KMethodName_ToSetter(mtd->mn), paramdom, psize, p);
	}
	if(foundMethod != NULL) {
		KFieldSet(expr->NodeList, expr->NodeList->MethodItems[0], foundMethod);
		KLIB kArray_Add(kctx, expr->NodeList, rightHandNode);
		return SUGAR kNodekNode_TypeCheckCallParam(kctx, stmt, expr, foundMethod, gma, reqClass);
	}
	return SUGAR kNode_Message2(kctx, stmt, (kToken *)expr, ErrTag, "undefined setter");
}

static KMETHOD TypeCheck_Assign(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kNode *leftHandNode = SUGAR kNode_TypeCheckNodeAt(kctx, stmt, expr, 1, gma, KClass_INFER, TypeCheckPolicy_ALLOWVOID);
	kNode *rightHandNode = SUGAR kNode_TypeCheckNodeAt(kctx, stmt, expr, 2, gma, KClass_(leftHandNode->attrTypeId), 0);
	kNode *returnNode = K_NULLNODE;
	if(rightHandNode != K_NULLNODE && leftHandNode != K_NULLNODE) {
		if(leftHandNode->node == KNode_LOCAL || leftHandNode->node == KNode_FIELD || leftHandNode->node == KNode_STACKTOP) {
			if(KTypeAttr_Is(ReadOnly, leftHandNode->attrTypeId)) {
				returnNode = SUGAR kNode_Message2(kctx, stmt, (kToken *)expr, ErrTag, "read only: %s", KToken_t(leftHandNode->TermToken));
			}
			else {
				expr->node      = KNode_LET;
				expr->attrTypeId = leftHandNode->attrTypeId;
				//((kNodeVar *)rightHandNode)->attrTypeId = leftHandNode->attrTypeId;
				returnNode = expr;
			}
		}
		else if(kNode_IsGetter(leftHandNode)) {
			returnNode = kNodeNode_ToSetter(kctx, stmt, (kNodeVar *)leftHandNode, gma, rightHandNode, KClass_(reqty));
		}
		else {
			returnNode = SUGAR kNode_Message2(kctx, stmt, (kToken *)expr, ErrTag, "assignment: variable name is expected");
		}
	}
	KReturn(returnNode);
}

static int kGamma_AddLocalVariable(KonohaContext *kctx, kGamma *gma, ktypeattr_t attrTypeId, ksymbol_t name)
{
	KGammaStack *s = &gma->genv->localScope;
	int index = s->varsize;
	if(!(s->varsize < s->capacity)) {
		s->capacity *= 2;
		size_t asize = sizeof(KGammaStackDecl) * s->capacity;
		KGammaStackDecl *v = (KGammaStackDecl *)KMalloc_UNTRACE(asize);
		memcpy(v, s->varItems, asize/2);
		if(s->allocsize > 0) {
			KFree(s->varItems, s->allocsize);
		}
		s->varItems = v;
		s->allocsize = asize;
	}
	s->varItems[index].attrTypeId = attrTypeId;
	s->varItems[index].name = name;
	s->varsize += 1;
	return index;
}

static KMETHOD TypeCheck_Node(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kNode *bk = expr->block;
	if(IS_Node(bk)) {  // this is special case of ${} by set internally
		kNode *texpr = K_NULLNODE;
		kNode *lastNode = NULL;
		kfileline_t uline = stmt->uline;
		if(kArray_size(bk->NodeList) > 0) {
			kNode *stmt = bk->NodeList->NodeItems[kArray_size(bk->NodeList)-1];
			if(stmt->syn->keyword == KSymbol_NodePattern) {
				lastNode = stmt;
			}
			uline = stmt->uline;
		}
		if(lastNode != NULL) {
			size_t lvarsize = gma->genv->localScope.varsize;
			int popNodeScopeShiftSize = gma->genv->blockScopeShiftSize;
			gma->genv->blockScopeShiftSize = lvarsize;
			if(!kNode_TypeCheckAll(kctx, bk, gma)) {
				KReturn(texpr);
			}
			kNode *lvar = new_VariableNode(kctx, gma, KNode_LOCAL, KType_var, kGamma_AddLocalVariable(kctx, gma, KType_var, 0/*FN_*/));
			kNode *rexpr = SUGAR kNode_GetNode(kctx, lastNode, KSymbol_NodePattern, NULL);
			DBG_ASSERT(rexpr != NULL);
			ktypeattr_t ty = rexpr->attrTypeId;
			if(ty != KType_void) {
				kNode *letexpr = new_TypedConsNode(kctx, KNode_LET, KClass_(KType_void), 3, K_NULL, lvar, rexpr);
				KLIB kObjectProto_SetObject(kctx, lastNode, KSymbol_NodePattern, KType_Node, letexpr);
				texpr = SUGAR kNode_SetVariable(kctx, expr, gma, KNode_BLOCK, ty, lvarsize);
			}
			gma->genv->blockScopeShiftSize = popNodeScopeShiftSize;
			if(lvarsize < gma->genv->localScope.varsize) {
				gma->genv->localScope.varsize = lvarsize;
			}
		}
		if(texpr == K_NULLNODE) {
			((kNodeVar *)stmt)->uline = uline;
			kNode_Message(kctx, stmt, ErrTag, "block has no value");
		}
		KReturn(texpr);
	}
	//kNodeNode_Message(kctx, stmt, expr, ErrTag, "undefined expression: %s", KToken_t(expr->TermToken));
}

static kNode* new_GetterNode(KonohaContext *kctx, kToken *tkU, kMethod *mtd, kNode *expr)
{
	return new_TypedConsNode(kctx, KNode_CALL, kMethod_GetReturnType(mtd), 2, mtd, expr);
}

static kNode* kNode_TypeCheckVariableNULL(KonohaContext *kctx, kNode *stmt, kNodeVar *expr, kGamma *gma, KClass *reqClass)
{
	DBG_ASSERT(expr->attrTypeId == KType_var);
	kToken *tk = expr->TermToken;
	ksymbol_t symbol = tk->resolvedSymbol;
	kNameSpace *ns = Node_ns(stmt);
	int i;
	KGammaAllocaData *genv = gma->genv;
	for(i = genv->localScope.varsize - 1; i >= 0; i--) {
		if(genv->localScope.varItems[i].name == symbol) {
			return SUGAR kNode_SetVariable(kctx, expr, gma, KNode_LOCAL, genv->localScope.varItems[i].attrTypeId, i);
		}
	}
	if(kNameSpace_IsAllowed(ImplicitField, ns)) {
		if(genv->localScope.varItems[0].attrTypeId != KType_void) {
			KClass *ct = genv->thisClass;
			if(ct->fieldsize > 0) {
				for(i = ct->fieldsize; i >= 0; i--) {
					if(ct->fieldItems[i].name == symbol && ct->fieldItems[i].attrTypeId != KType_void) {
						return SUGAR kNode_SetVariable(kctx, expr, gma, KNode_FIELD, ct->fieldItems[i].attrTypeId, longid((kshort_t)i, 0));
					}
				}
			}
			kMethod *mtd = kNameSpace_GetGetterMethodNULL(kctx, ns, genv->thisClass, symbol);
			if(mtd != NULL) {
				return new_GetterNode(kctx, tk, mtd, new_VariableNode(kctx, gma, KNode_LOCAL, genv->thisClass->typeId, 0));
			}
		}
	}
	if((Gamma_isTopLevel(gma) || kNameSpace_IsAllowed(ImplicitGlobalVariable, ns)) && ns->globalObjectNULL_OnList != NULL) {
		KClass *globalClass = kObject_class(ns->globalObjectNULL_OnList);
		kMethod *mtd = kNameSpace_GetGetterMethodNULL(kctx, ns, globalClass, symbol);
		if(mtd != NULL) {
			return new_GetterNode(kctx, tk, mtd, new_ConstValueNode(kctx, globalClass, ns->globalObjectNULL_OnList));
		}
	}
	kMethod *mtd = kNameSpace_GetNameSpaceFuncNULL(kctx, ns, symbol, reqClass);  // finding function
	if(mtd != NULL) {
		kParam *pa = kMethod_GetParam(mtd);
		KClass *ct = KLIB KClass_Generics(kctx, KClass_Func, pa->rtype, pa->psize, (kparamtype_t *)pa->paramtypeItems);
		kFuncVar *fo = (kFuncVar *)KLIB new_kObject(kctx, OnGcStack, ct, (uintptr_t)mtd);
		//KFieldSet(fo, fo->self, UPCAST(ns));
		return new_ConstValueNode(kctx, ct, UPCAST(fo));
	}
	if(symbol != KSymbol_Noname) {
		KKeyValue *kv = kNameSpace_GetConstNULL(kctx, ns, symbol);
		if(kv != NULL) {
			if(KTypeAttr_Is(Boxed, kv->attrTypeId)) {
				SUGAR kNode_SetConstValue(kctx, expr, NULL, kv->ObjectValue);
			}
			else {
				SUGAR kNode_SetUnboxConstValue(kctx, expr, kv->attrTypeId, kv->unboxValue);
			}
			return expr;
		}
	}
	return NULL;
}

static KMETHOD TypeCheck_Symbol(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kNode *texpr = kNode_TypeCheckVariableNULL(kctx, stmt, expr, gma, KClass_(reqty));
	if(texpr == NULL) {
		kToken *tk = expr->TermToken;
		texpr = kNodeToken_Message(kctx, stmt, tk, ErrTag, "undefined name: %s", KToken_t(tk));
	}
	KReturn(texpr);
}

static KClass* ResolveTypeVariable(KonohaContext *kctx, KClass *varType, KClass *thisClass)
{
	return varType->realtype(kctx, varType, thisClass);
}

static int TypeCheckPolicy_(ktypeattr_t attrtype)
{
	int pol = 0;
	if(KTypeAttr_Is(Coercion, attrtype)) {
		pol = pol | TypeCheckPolicy_COERCION;
	}
	return pol;
}

static kMethod *kNode_LookupOverloadedMethod(KonohaContext *kctx, kNode *stmt, kNode *expr, kMethod *mtd, kGamma *gma)
{
	KClass *thisClass = KClass_(expr->NodeList->NodeItems[1]->attrTypeId);
	size_t i, psize = kArray_size(expr->NodeList) - 2;
	kparamtype_t *p = ALLOCA(kparamtype_t, psize);
	kParam *pa = kMethod_GetParam(mtd);
	for(i = 0; i < psize; i++) {
		size_t n = i + 2;
		KClass *paramType = (i < pa->psize) ? ResolveTypeVariable(kctx, KClass_(pa->paramtypeItems[i].attrTypeId), thisClass) : KClass_INFER;
		kNode *texpr = SUGAR kNode_TypeCheckNodeAt(kctx, stmt, expr, n, gma, paramType, TypeCheckPolicy_NOCHECK);
		if(texpr == K_NULLNODE) {
			return NULL;
		}
		p[i].attrTypeId = texpr->attrTypeId;
	}
	kparamId_t paramdom = KLIB Kparamdom(kctx, psize, p);
	kMethod *foundMethod = kNameSpace_GetMethodBySignatureNULL(kctx, Node_ns(stmt), thisClass, mtd->mn, paramdom, psize, p);
	DBG_P("paradom=%d, foundMethod=%p", paramdom, foundMethod);
	return foundMethod;
}

static kNodeVar* TypeMethodCallNode(KonohaContext *kctx, kNodeVar *expr, kMethod *mtd, KClass *reqClass)
{
	kNode *thisNode = kNode_At(expr, 1);
	KFieldSet(expr->NodeList, expr->NodeList->MethodItems[0], mtd);
	KClass *typedClass = ResolveTypeVariable(kctx, kMethod_GetReturnType(mtd), KClass_(thisNode->attrTypeId));
	if(thisNode->node == KNode_NEW) {
		typedClass = KClass_(thisNode->attrTypeId);
	}
	else if(kMethod_Is(SmartReturn, mtd) && reqClass->typeId != KType_var) {
		typedClass = reqClass;
	}
	kNode_typed(expr, CALL, typedClass->typeId);
	return expr;
}

static kNode* BoxThisNode(KonohaContext *kctx, kNode *stmt, kGamma *gma, kNode *expr, kMethod *mtd, KClass **thisClassRef)
{
	kNode *thisNode = expr->NodeList->NodeItems[1];
	KClass *thisClass = KClass_(thisNode->attrTypeId);
	DBG_ASSERT(IS_Method(mtd));
	DBG_ASSERT(thisClass->typeId != KType_var);
	DBG_P("mtd_cid=%s this=%s", KType_text(mtd->typeId), KClass_text(thisClass));
	if(!KType_Is(UnboxType, mtd->typeId) && KClass_Is(UnboxType, thisClass)) {
		KFieldSet(expr->NodeList, expr->NodeList->NodeItems[1], kNodeNode_ToBox(kctx, stmt, thisNode, gma, thisClass));
	}
	thisClassRef[0] = thisClass;
	return thisNode;
}

static kNode *kNodekNode_TypeCheckCallParam(KonohaContext *kctx, kNode *stmt, kNodeVar *expr, kMethod *mtd, kGamma *gma, KClass* reqClass)
{
	KClass *thisClass = NULL;
	kNode *thisNode = BoxThisNode(kctx, stmt, gma, expr, mtd, &thisClass);
	kbool_t isConst = kNode_IsConstValue(thisNode);
	kParam *pa = kMethod_GetParam(mtd);
	DBG_ASSERT(pa->psize +2 <= kArray_size(expr->NodeList));
	size_t i;
	for(i = 0; i < pa->psize; i++) {
		size_t n = i + 2;
		KClass* paramType = ResolveTypeVariable(kctx, KClass_(pa->paramtypeItems[i].attrTypeId), thisClass);
		int tycheckPolicy = TypeCheckPolicy_(pa->paramtypeItems[i].attrTypeId);
		kNode *texpr = SUGAR kNode_TypeCheckNodeAt(kctx, stmt, expr, n, gma, paramType, tycheckPolicy);
		if(texpr == K_NULLNODE) {
			return kNodeNode_Message(kctx, stmt, expr, InfoTag, "%s.%s%s accepts %s at the parameter %d", kMethod_Fmt3(mtd), KClass_text(paramType), (int)i+1);
		}
		if(!kNode_IsConstValue(texpr)) isConst = 0;
	}
	expr = TypeMethodCallNode(kctx, expr, mtd, reqClass);
	if(isConst && kMethod_Is(Const, mtd)) {
		KClass *rtype = ResolveTypeVariable(kctx, KClass_(pa->rtype), thisClass);
		return kNodeNode_ToConstValue(kctx, stmt, expr, expr->NodeList, rtype);
	}
	return expr;
}

static kNode* TypeCheckDynamicCallParams(KonohaContext *kctx, kNode *stmt, kNodeVar *expr, kMethod *mtd, kGamma *gma, kString *name, kmethodn_t mn, KClass *reqClass)
{
	size_t i;
	kParam *pa = kMethod_GetParam(mtd);
	KClass* ptype = (pa->psize == 0) ? KClass_Object : KClass_(pa->paramtypeItems[0].attrTypeId);
	for(i = 2; i < kArray_size(expr->NodeList); i++) {
		kNode *texpr = SUGAR kNode_TypeCheckNodeAt(kctx, stmt, expr, i, gma, ptype, 0);
		if(texpr == K_NULLNODE) return texpr;
	}
	kNode_Add(kctx, expr, new_ConstValueNode(kctx, NULL, UPCAST(name)));
	return TypeMethodCallNode(kctx, expr, mtd, reqClass);
}

static kMethod *kNameSpace_GuessCoercionMethodNULL(KonohaContext *kctx, kNameSpace *ns, kToken *tk, KClass *thisClass)
{
	const char *name = kString_text(tk->text);
	if(name[1] == 'o' && (name[0] == 't' || name[0] == 'T')) {
		KClass *c = KLIB kNameSpace_GetClassByFullName(kctx, ns, name + 2, kString_size(tk->text) - 2, NULL);
		if(c != NULL) {
			return KLIB kNameSpace_GetCoercionMethodNULL(kctx, ns, thisClass, c);
		}
	}
	return NULL;
}

static kNode *kNodeNode_LookupMethod(KonohaContext *kctx, kNode *stmt, kNodeVar *expr, KClass *thisClass, kGamma *gma, KClass *reqClass)
{
	kNameSpace *ns = Node_ns(stmt);
	kTokenVar *methodToken = expr->NodeList->TokenVarItems[0];
	DBG_ASSERT(IS_Token(methodToken));
	size_t psize = kArray_size(expr->NodeList) - 2;
	kMethod *mtd = kNameSpace_GetMethodByParamSizeNULL(kctx, ns, thisClass, methodToken->resolvedSymbol, psize, KMethodMatch_CamelStyle);
	if(mtd == NULL && psize == 0) {
		mtd = kNameSpace_GuessCoercionMethodNULL(kctx, ns, methodToken, thisClass);
	}
	if(mtd == NULL) {
		if(methodToken->text != TS_EMPTY) {  // find Dynamic Call ..
			mtd = kNameSpace_GetMethodByParamSizeNULL(kctx, ns, thisClass, 0/*NONAME*/, 1, KMethodMatch_NoOption);
			if(mtd != NULL) {
				return TypeCheckDynamicCallParams(kctx, stmt, expr, mtd, gma, methodToken->text, methodToken->resolvedSymbol, reqClass);
			}
		}
		if(methodToken->resolvedSymbol == MN_new && psize == 0 && KClass_(kNode_At(expr, 1)->attrTypeId)->baseTypeId == KType_Object) {
			return kNode_At(expr, 1);  // new Person(); // default constructor
		}
		kNodeToken_Message(kctx, stmt, methodToken, ErrTag, "undefined method: %s.%s%s", KClass_text(thisClass), KSymbol_Fmt2(methodToken->resolvedSymbol));
	}
	if(mtd != NULL) {
		if(kMethod_Is(Overloaded, mtd)) {
			//DBG_P("found overloaded method %s.%s%s", kMethod_Fmt3(mtd));
			mtd = kNode_LookupOverloadedMethod(kctx, stmt, expr, mtd, gma);
		}
		if(mtd != NULL) {
			//DBG_P("found resolved method %s.%s%s isOverloaded=%d", kMethod_Fmt3(mtd), kMethod_Is(Overloaded, mtd));
			return kNodekNode_TypeCheckCallParam(kctx, stmt, expr, mtd, gma, reqClass);
		}
	}
	return K_NULLNODE;
}

static KMETHOD TypeCheck_MethodCall(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kNode *texpr = SUGAR kNode_TypeCheckNodeAt(kctx, stmt, expr, 1, gma, KClass_INFER, 0);
	if(texpr != K_NULLNODE) {
		KReturn(kNodeNode_LookupMethod(kctx, stmt, expr, KClass_(texpr->attrTypeId), gma, KClass_(reqty)));
	}
}

// --------------------------------------------------------------------------
// FuncStyleCall

static kNode *kNode_TypeCheckFuncParams(KonohaContext *kctx, kNode *stmt, kNodeVar *expr, kGamma *gma);

static kMethod* kNode_LookupFuncOrMethod(KonohaContext *kctx, kNameSpace *ns, kNodeVar *exprN, kGamma *gma, KClass *reqClass)
{
	kNodeVar *firstNode = (kNodeVar *)kNode_At(exprN, 0);
	kToken *termToken = firstNode->TermToken;
	ksymbol_t funcName = termToken->resolvedSymbol;
	KGammaAllocaData *genv = gma->genv;
	int i;
	for(i = genv->localScope.varsize - 1; i >= 0; i--) {
		if(genv->localScope.varItems[i].name == funcName && KType_IsFunc(genv->localScope.varItems[i].attrTypeId)) {
			SUGAR kNode_SetVariable(kctx, firstNode, gma, KNode_LOCAL, genv->localScope.varItems[i].attrTypeId, i);
			return NULL;
		}
	}
	int paramsize = kArray_size(exprN->NodeList) - 2;
	if(genv->localScope.varItems[0].attrTypeId != KType_void) {
		KClass *ct = genv->thisClass;
		kMethod *mtd = kNameSpace_GetMethodByParamSizeNULL(kctx, ns, genv->thisClass, funcName, paramsize, KMethodMatch_CamelStyle);
		if(mtd != NULL) {
			KFieldSet(exprN->NodeList, exprN->NodeList->NodeItems[1], new_VariableNode(kctx, gma, KNode_LOCAL, ct->typeId, 0));
			return mtd;
		}
		if(ct->fieldsize) {
			for(i = ct->fieldsize; i >= 0; i--) {
				if(ct->fieldItems[i].name == funcName && KType_IsFunc(ct->fieldItems[i].attrTypeId)) {
					SUGAR kNode_SetVariable(kctx, firstNode, gma, KNode_FIELD, ct->fieldItems[i].attrTypeId, longid((kshort_t)i, 0));
					return NULL;
				}
			}
		}
		mtd = kNameSpace_GetGetterMethodNULL(kctx, ns, genv->thisClass, funcName);
		if(mtd != NULL && kMethod_IsReturnFunc(mtd)) {
			KFieldSet(exprN->NodeList, exprN->NodeList->NodeItems[0], new_GetterNode(kctx, termToken, mtd, new_VariableNode(kctx, gma, KNode_LOCAL, genv->thisClass->typeId, 0)));
			return NULL;
		}
	}
	{
		KKeyValue* kvs = kNameSpace_GetConstNULL(kctx, ns, funcName);
		if(kvs != NULL && KTypeAttr_Unmask(kvs->attrTypeId) == VirtualType_StaticMethod) {
			KClass *c = KClass_((ktypeattr_t)kvs->unboxValue);
			ksymbol_t alias = (ksymbol_t)(kvs->unboxValue >> (sizeof(ktypeattr_t) * 8));
			kMethod *mtd = kNameSpace_GetMethodByParamSizeNULL(kctx, ns, c, alias, paramsize, KMethodMatch_NoOption);
			if(mtd != NULL && kMethod_Is(Static, mtd)) {
				KFieldSet(exprN->NodeList, exprN->NodeList->NodeItems[1], new_ConstValueNode(kctx, c, KLIB Knull(kctx, c)));
				return mtd;
			}
		}
	}
	{
		kMethod *mtd = kNameSpace_GetMethodByParamSizeNULL(kctx, ns, kObject_class(ns), funcName, paramsize, KMethodMatch_CamelStyle);
		if(mtd != NULL) {
			KFieldSet(exprN->NodeList, exprN->NodeList->NodeItems[1], new_ConstValueNode(kctx, kObject_class(ns), UPCAST(ns)));
			return mtd;
		}
	}
	if((Gamma_isTopLevel(gma) || kNameSpace_IsAllowed(ImplicitGlobalVariable,ns)) && ns->globalObjectNULL_OnList != NULL) {
		KClass *globalClass = kObject_class(ns->globalObjectNULL_OnList);
		kMethod *mtd = kNameSpace_GetGetterMethodNULL(kctx, ns, globalClass, funcName);
		if(mtd != NULL && kMethod_IsReturnFunc(mtd)) {
			KFieldSet(exprN->NodeList, exprN->NodeList->NodeItems[0], new_GetterNode(kctx, termToken, mtd, new_ConstValueNode(kctx, globalClass, ns->globalObjectNULL_OnList)));
			return NULL;
		}
		return mtd;
	}
	return NULL;
}

static KMETHOD TypeCheck_FuncStyleCall(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	DBG_ASSERT(IS_Node(kNode_At(expr, 0)));
	DBG_ASSERT(expr->NodeList->ObjectItems[1] == K_NULL);
	if(kNode_isSymbolTerm(kNode_At(expr, 0))) {
		kMethod *mtd = kNode_LookupFuncOrMethod(kctx, Node_ns(stmt), expr, gma, KClass_(reqty));
		if(mtd != NULL) {
			if(kMethod_Is(Overloaded, mtd)) {
				DBG_P("overloaded found %s.%s%s", kMethod_Fmt3(mtd));
				mtd = kNode_LookupOverloadedMethod(kctx, stmt, expr, mtd, gma);
			}
			KReturn(kNodekNode_TypeCheckCallParam(kctx, stmt, expr, mtd, gma, KClass_(reqty)));
		}
		if(!KType_IsFunc(kNode_At(expr, 0)->attrTypeId)) {
			kToken *tk = kNode_At(expr, 0)->TermToken;
			DBG_ASSERT(IS_Token(tk));  // TODO: make error message in case of not Token
			KReturn(kNodeToken_Message(kctx, stmt, tk, ErrTag, "undefined function: %s", KToken_t(tk)));
		}
	}
	else {
		if(kNode_TypeCheckNodeAt(kctx, stmt, expr, 0, gma, KClass_INFER, 0) != K_NULLNODE) {
			if(!KType_IsFunc(expr->NodeList->NodeItems[0]->attrTypeId)) {
				KReturn(kNodeNode_Message(kctx, stmt, expr, ErrTag, "function is expected"));
			}
		}
	}
	KReturn(kNode_TypeCheckFuncParams(kctx, stmt, expr, gma));
}

static kNode *kNode_TypeCheckFuncParams(KonohaContext *kctx, kNode *stmt, kNodeVar *expr, kGamma *gma)
{
	KClass *thisClass = KClass_(kNode_At(expr, 0)->attrTypeId);
	kParam *pa = KClass_cparam(thisClass);
	size_t i, size = kArray_size(expr->NodeList);
	if(pa->psize + 2 != size) {
		return kNodeNode_Message(kctx, stmt, expr, ErrTag, "function %s takes %d parameter(s), but given %d parameter(s)", KClass_text(thisClass), (int)pa->psize, (int)size-2);
	}
	for(i = 0; i < pa->psize; i++) {
		size_t n = i + 2;
		kNode *texpr = SUGAR kNode_TypeCheckNodeAt(kctx, stmt, expr, n, gma, KClass_(pa->paramtypeItems[i].attrTypeId), 0);
		if(texpr == K_NULLNODE) {
			return texpr;
		}
	}
	kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, Node_ns(stmt), KClass_Func, KKMethodName_("invoke"), -1, KMethodMatch_NoOption);
	DBG_ASSERT(mtd != NULL);
	KFieldSet(expr->NodeList, expr->NodeList->NodeItems[1], expr->NodeList->NodeItems[0]);
	return TypeMethodCallNode(kctx, expr, mtd, KClass_(thisClass->p0));
}

// ---------------------------------------------------------------------------
// Statement Node

static KMETHOD Statement_Nodeession(KonohaContext *kctx, KonohaStack *sfp)  // $Node
{
	VAR_Statement(stmt, gma);
	kbool_t r = kNode_TypeCheckByName(kctx, stmt, KSymbol_NodePattern, gma, KClass_INFER, TypeCheckPolicy_ALLOWVOID);
	kNode_typed(stmt, EXPR);
	KReturnUnboxValue(r);
}

#define DefaultNameSpace NULL
static KMETHOD Statement_if(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	if(kNode_TypeCheckByName(kctx, stmt, KSymbol_NodePattern, gma, KClass_Boolean, 0)) {
		kNode *bkThen = SUGAR kNode_GetNode(kctx, stmt, DefaultNameSpace, KSymbol_NodePattern, K_NULLBLOCK);
		kNode *bkElse = SUGAR kNode_GetNode(kctx, stmt, DefaultNameSpace, KSymbol_else, K_NULLBLOCK);
		kNode_TypeCheckAll(kctx, bkThen, gma);
		kNode_TypeCheckAll(kctx, bkElse, gma);
		kNode_typed(stmt, IF);
	}
}

static kNode* Node_LookupIfNodeWithoutElse(KonohaContext *kctx, kNode *stmt)
{
	kNode *bkElse = SUGAR kNode_GetNode(kctx, stmt, DefaultNameSpace, KSymbol_else, NULL);
	if(bkElse != NULL) {
		if(kArray_size(bkElse->NodeList) == 1) {
			kNode *stmtIf = bkElse->NodeList->NodeItems[0];
			if(stmtIf->syn->keyword == KSymbol_if) {
				return Node_LookupIfNodeWithoutElse(kctx, stmtIf);
			}
		}
		return NULL;
	}
	return stmt;
}

static kNode* Node_LookupIfNodeNULL(KonohaContext *kctx, kNode *stmt)
{
	int i;
	kArray *bka = stmt->parentNodeNULL->NodeList;
	kNode *prevIfNode = NULL;
	for(i = 0; kArray_size(bka); i++) {
		kNode *s = bka->NodeItems[i];
		if(s == stmt) {
			if(prevIfNode != NULL) {
				return Node_LookupIfNodeWithoutElse(kctx, prevIfNode);
			}
			return NULL;
		}
		if(s->syn == NULL) continue;  // this is done
		prevIfNode = (s->syn->keyword == KSymbol_if) ? s : NULL;
	}
	return NULL;
}

static KMETHOD Statement_else(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kNode *stmtIf = Node_LookupIfNodeNULL(kctx, stmt);
	if(stmtIf != NULL) {
		kNode *bkElse = SUGAR kNode_GetNode(kctx, stmt, NULL/*DefaultNameSpace*/, KSymbol_NodePattern, K_NULLBLOCK);
		KLIB kObjectProto_SetObject(kctx, stmtIf, KSymbol_else, KType_Node, bkElse);
		kNode_done(kctx, stmt);
		kNode_TypeCheckAll(kctx, bkElse, gma);
	}
	else {
		kNode_Message(kctx, stmt, ErrTag, "else is not statement");
//		r = 0;
	}
//	KReturnUnboxValue(r);
}

static KMETHOD Statement_return(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	KClass *returnType = kMethod_GetReturnType(gma->genv->currentWorkingMethod);
	kNode_typed(stmt, RETURN);
	if(returnType->typeId != KType_void) {
		kNode_TypeCheckByName(kctx, stmt, KSymbol_NodePattern, gma, returnType, 0);
	} else {
		kNode *expr = (kNode *)kNode_GetObjectNULL(kctx, stmt, KSymbol_NodePattern);
		if(expr != NULL) {
			kNode_Message(kctx, stmt, WarnTag, "ignored return value");
			KLIB kObjectProto_RemoveKey(kctx, stmt, KSymbol_NodePattern);
		}
	}
}

///* ------------------------------------------------------------------------ */

static kNode* TypeDeclLocalVariable(KonohaContext *kctx, kNode *stmt, kGamma *gma, ktypeattr_t attrTypeId, kNode *termNode, kNode *vexpr, kObject *thunk)
{
	DBG_ASSERT(kNode_isSymbolTerm(termNode));
	int index = kGamma_AddLocalVariable(kctx, gma, attrTypeId, termNode->TermToken->resolvedSymbol);
	SUGAR kNode_SetVariable(kctx, (kNodeVar *)termNode, gma, KNode_LOCAL, attrTypeId, index);
	kNodeVar *expr = (kNodeVar *)new_TypedConsNode(kctx, KNode_LET, KClass_(KType_void), 3, K_NULL, termNode, vexpr);
	kNode_typed(expr, LET, KType_void);
	kNode *newstmt = new_(Node, stmt->uline, OnGcStack);
	kNode_Setsyn(newstmt, KSyntax_(Node_ns(stmt), KSymbol_NodePattern));
	KLIB kObjectProto_SetObject(kctx, newstmt, KSymbol_NodePattern, KType_Node, expr);
	return newstmt;
}

static kbool_t kNode_DeclType(KonohaContext *kctx, kNode *stmt, kGamma *gma, ktypeattr_t attrTypeId, kNode *declNode, kObject *thunk, KTypeDeclFunc TypeDecl, kNode **lastNodeRef)
{
	kNode *newstmt = NULL;
	if(TypeDecl == NULL) TypeDecl = TypeDeclLocalVariable;
	if(declNode->syn->keyword == KSymbol_COMMA) {
		size_t i;
		for(i = 1; i < kArray_size(declNode->NodeList); i++) {
			if(!kNode_DeclType(kctx, stmt, gma, attrTypeId, kNode_At(declNode, i), thunk, TypeDecl, lastNodeRef)) return false;
		}
		return true;
	}
	else if(declNode->syn->keyword == KSymbol_LET && kNode_isSymbolTerm(kNode_At(declNode, 1))) {
		if(SUGAR kNode_TypeCheckNodeAt(kctx, stmt, declNode, 2, gma, KClass_(attrTypeId), 0) == K_NULLNODE) {
			// this is neccesarry to avoid 'int a = a + 1;';
			return false;
		}
		if(KTypeAttr_Unmask(attrTypeId) == KType_var) {
			ktypeattr_t attr = KTypeAttr_Attr(attrTypeId);
			kToken *termToken = kNode_At(declNode, 1)->TermToken;
			attrTypeId = kNode_At(declNode, 2)->attrTypeId | attr;
			kNodeToken_Message(kctx, stmt, termToken, InfoTag, "%s%s has type %s", KSymbol_Fmt2(termToken->resolvedSymbol), KType_text(attrTypeId));
		}
		newstmt = TypeDecl(kctx, stmt, gma, attrTypeId, kNode_At(declNode, 1), kNode_At(declNode, 2), thunk);
	}
	else if(kNode_isSymbolTerm(declNode)) {
		if(attrTypeId == KType_var  || !KType_Is(Nullable, attrTypeId)) {
			kNode_Message(kctx, stmt, ErrTag, "%s %s%s: initial value is expected", KType_text(attrTypeId), KSymbol_Fmt2(declNode->TermToken->resolvedSymbol));
			return false;
		}
		else {
			kNode *vexpr = new_VariableNode(kctx, gma, KNode_NULL, attrTypeId, 0);
			newstmt = TypeDecl(kctx, stmt, gma, attrTypeId, declNode, vexpr, thunk);
		}
	}
	else {
		kNode_Message(kctx, stmt, ErrTag, "type declaration: variable name is expected");
		return false;
	}
	if(newstmt != NULL) {
		kNode *lastNode = lastNodeRef[0];
		kNode_InsertAfter(kctx, lastNode->parentNodeNULL, lastNode, newstmt);
		lastNodeRef[0] = newstmt;
		kNode_done(kctx, stmt);
		return true;
	}
	return false;
}

static KMETHOD Statement_TypeDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kToken *tk  = SUGAR kNode_GetToken(kctx, stmt, KSymbol_TypePattern, NULL);
	kNode  *expr = SUGAR kNode_GetNode(kctx, stmt, KSymbol_NodePattern, NULL);
	ktypeattr_t attrTypeId = Token_typeLiteral(tk);
	kNode_DeclType(kctx, stmt, gma, attrTypeId, expr, NULL, TypeDeclLocalVariable, &stmt);
}

// ------------------
// Method Utilities for MethodDecl

static KMETHOD KMethodFunc_LazyCompilation(KonohaContext *kctx, KonohaStack *sfp)
{
	KonohaStack *esp = kctx->esp;
	kMethod *mtd = sfp[K_MTDIDX].calledMethod;
	kString *text = mtd->SourceToken->text;
	kfileline_t uline = mtd->SourceToken->uline;
	DBG_P("<<lazy compilation>>: %s.%s%s", KType_text(mtd->typeId), KMethodName_Fmt2(mtd->mn));
	kMethod_Compile(kctx, mtd, NULL, mtd->LazyCompileNameSpace, text, uline, DefaultCompileOption/*HatedLazyCompile*/);
	((KonohaContextVar *)kctx)->esp = esp;
	mtd->invokeKMethodFunc(kctx, sfp); // call again;
}

static void kMethod_SetLazyCompilation(KonohaContext *kctx, kMethodVar *mtd, kNode *stmt, kNameSpace *ns)
{
	kToken *tcode = SUGAR kNode_GetToken(kctx, stmt, KSymbol_NodePattern, NULL);
	if(tcode != NULL && tcode->resolvedSyntaxInfo->keyword == TokenType_CODE) {
		KFieldSet(mtd, mtd->SourceToken, tcode);
		KFieldSet(mtd, mtd->LazyCompileNameSpace, ns);
		KLIB kMethod_SetFunc(kctx, mtd, KMethodFunc_LazyCompilation);
		KLIB kArray_Add(kctx, KGetParserContext(kctx)->definedMethodList, mtd);
	}
}

/* In the future, DoLazyCompilation is extended to compile untyped parameters */

static kMethod* kMethod_DoLazyCompilation(KonohaContext *kctx, kMethod *mtd, kparamtype_t *callparamNULL, int options)
{
	if(mtd->invokeKMethodFunc == KMethodFunc_LazyCompilation) {
		kString *text = mtd->SourceToken->text;
		kfileline_t uline = mtd->SourceToken->uline;
		((kMethodVar *)mtd)->invokeKMethodFunc = NULL; // TO avoid recursive compile
		mtd = kMethod_Compile(kctx, mtd, callparamNULL, mtd->LazyCompileNameSpace, text, uline, options|HatedLazyCompile);
		DBG_ASSERT(mtd->invokeKMethodFunc != KMethodFunc_LazyCompilation);
	}
	return mtd;
}

///* ------------------------------------------------------------------------ */
///* [ParamUtils] */

static kbool_t NodeTypeDecl_setParam(KonohaContext *kctx, kNode *stmt, int n, kparamtype_t *p)
{
	kToken *tkT  = SUGAR kNode_GetToken(kctx, stmt, KSymbol_TypePattern, NULL);
	kNode  *expr = SUGAR kNode_GetNode(kctx, stmt, KSymbol_NodePattern, NULL);
	DBG_ASSERT(tkT != NULL);
	DBG_ASSERT(expr != NULL);
	if(kNode_isSymbolTerm(expr)) {
		kToken *tkN = expr->TermToken;
		ksymbol_t fn = KAsciiSymbol(kString_text(tkN->text), kString_size(tkN->text), KSymbol_NewId);
		p[n].name = fn;
		p[n].attrTypeId = Token_typeLiteral(tkT);
		return true;
	}
	return false;
}

static kParam *kNode_newMethodParamNULL(KonohaContext *kctx, kNode *stmt, kGamma* gma)
{
	kParam *pa = (kParam *)kNode_GetObjectNULL(kctx, stmt, KSymbol_ParamPattern);
	if(pa == NULL || !IS_Param(pa)) {
		KSyntax *syn = KSyntax_(Node_ns(stmt), KSymbol_ParamPattern);
		if(!KSyntax_TypeCheckNode(kctx, syn, stmt, gma)) {
			return NULL;
		}
	}
	pa = (kParam *)kNode_GetObjectNULL(kctx, stmt, KSymbol_ParamPattern);
	DBG_ASSERT(IS_Param(pa));
	return pa;
}

static KMETHOD Statement_ParamsDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kToken *tkT = SUGAR kNode_GetToken(kctx, stmt, KSymbol_TypePattern, NULL); // type
	ktypeattr_t rtype =  tkT == NULL ? KType_void : Token_typeLiteral(tkT);
	kParam *pa = NULL;
	kNode *params = (kNode *)kNode_GetObjectNULL(kctx, stmt, KSymbol_ParamPattern);
	if(params == NULL) {
		pa = new_kParam(kctx, rtype, 0, NULL);
	}
	else if(IS_Node(params)) {
		size_t i, psize = kArray_size(params->NodeList);
		kparamtype_t *p = ALLOCA(kparamtype_t, psize);
		for(i = 0; i < psize; i++) {
			p[i].attrTypeId = KType_void;
			p[i].name = 0;
			kNode *stmt = params->NodeList->NodeItems[i];
			if(stmt->syn->keyword != KSymbol_TypeDeclPattern || !NodeTypeDecl_setParam(kctx, stmt, i, p)) {
				break;
			}
		}
		pa = new_kParam(kctx, rtype, psize, p);
	}
	if(pa != NULL && IS_Param(pa)) {
		KLIB kObjectProto_SetObject(kctx, stmt, KSymbol_ParamPattern, KType_Param, pa);
		kNode_done(kctx, stmt);
		KReturnUnboxValue(true);
	}
	kNode_Message(kctx, stmt, ErrTag, "expected parameter declaration");
	KReturnUnboxValue(false);
}

///* ------------------------------------------------------------------------ */
///* [MethodDecl] */

static ktypeattr_t kNode_GetClassId(KonohaContext *kctx, kNode *stmt, kNameSpace *ns, ksymbol_t kw, ktypeattr_t defcid)
{
	kToken *tk = (kToken *)kNode_GetObjectNULL(kctx, stmt, kw);
	if(tk == NULL || !IS_Token(tk)) {
		return defcid;
	}
	else {
		DBG_ASSERT(Token_isVirtualTypeLiteral(tk));
		return Token_typeLiteral(tk);
	}
}

static ksymbol_t kNode_GetMethodSymbol(KonohaContext *kctx, kNode *stmt, kNameSpace *ns, ksymbol_t kw, kmethodn_t defmn)
{
	kToken *tk = (kToken *)kNode_GetObjectNULL(kctx, stmt, kw);
	if(tk == NULL || !IS_Token(tk) || !IS_String(tk->text)) {
		return defmn;
	}
	else {
		return tk->resolvedSymbol;
	}
}

static KMETHOD Statement_MethodDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	static KFlagSymbolData MethodDeclFlag[] = {
		{kMethod_Public}, {kMethod_Const}, {kMethod_Static},
		{kMethod_Virtual}, {kMethod_Final}, {kMethod_Override},
		{kMethod_Restricted},
	};
	if(MethodDeclFlag[0].symbol == 0) {   // this is a tricky technique
		MethodDeclFlag[0].symbol = KSymbol_("@Public");
		MethodDeclFlag[1].symbol = KSymbol_("@Const");
		MethodDeclFlag[2].symbol = KSymbol_("@Static");
		MethodDeclFlag[3].symbol = KSymbol_("@Virtual");
		MethodDeclFlag[4].symbol = KSymbol_("@Final");
		MethodDeclFlag[5].symbol = KSymbol_("@Override");
		MethodDeclFlag[6].symbol = KSymbol_("@Restricted");
	}
	uintptr_t flag    = kNode_ParseFlag(kctx, stmt, MethodDeclFlag, 0);
	kNameSpace *ns    = Node_ns(stmt);
	ktypeattr_t typeId    = kNode_GetClassId(kctx, stmt, ns, KSymbol_("ClassName"), kObject_typeId(ns));
	kmethodn_t mn     = kNode_GetMethodSymbol(kctx, stmt, ns, KSymbol_SymbolPattern, MN_new);
	kParam *pa        = kNode_newMethodParamNULL(kctx, stmt, gma);
	if(KType_Is(Singleton, typeId)) { flag |= kMethod_Static; }
	if(KType_Is(Final, typeId)) { flag |= kMethod_Final; }
	if(pa != NULL) {  // if pa is NULL, error is printed out.
		INIT_GCSTACK();
		kMethodVar *mtd = (kMethodVar *)KLIB new_kMethod(kctx, _GcStack, flag, typeId, mn, NULL);
		KLIB kMethod_SetParam(kctx, mtd, pa->rtype, pa->psize, (kparamtype_t *)pa->paramtypeItems);
		KMakeTrace(trace, sfp);
		if(kNameSpace_AddMethod(kctx, ns, mtd, trace)) {
			kMethod_SetLazyCompilation(kctx, mtd, stmt, ns);
		}
		RESET_GCSTACK();
	}
	kNode_done(kctx, stmt);
	KReturnUnboxValue(pa != NULL);
}

/* ------------------------------------------------------------------------ */

#define PATTERN(T)  KSymbol_##T##Pattern
#define GROUP(T)    KSymbol_##T##Group
#define TOKEN(T)    KSymbol_##T

static void DefineDefaultSyntax(KonohaContext *kctx, kNameSpace *ns)
{
	DBG_ASSERT(KSymbol_("$Param") == KSymbol_ParamPattern);
	DBG_ASSERT(KSymbol_(".") == KSymbol_DOT);
	DBG_ASSERT(KSymbol_(":") == KSymbol_COLON);
	DBG_ASSERT(KSymbol_("true") == KSymbol_true);
	DBG_ASSERT(KSymbol_("return") == KSymbol_return);
	DBG_P("MN_new=%d", KSymbol_("new"));
	DBG_ASSERT(KSymbol_("new") == MN_new);
	KDEFINE_SYNTAX SYNTAX[] = {
		{ TOKEN(ERR), SYNFLAG_NodeBreakExec, },
		{ PATTERN(Symbol),  0, NULL, 0, 0, PatternMatch_MethodName, Nodeession_Term, NULL, NULL, TypeCheck_Symbol,},
		{ PATTERN(Text),    0, NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_TextLiteral,},
		{ PATTERN(Number),  0, NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_IntLiteral,},
		{ GROUP(Parenthesis), SYNFLAG_NodePostfixOp2, NULL, Precedence_CStyleCALL, 0, NULL, Nodeession_Parenthesis, NULL, NULL, TypeCheck_FuncStyleCall,}, //KSymbol_ParenthesisGroup
		{ GROUP(Bracket),  },  //KSymbol_BracketGroup
		{ GROUP(Brace),  }, // KSymbol_BraceGroup
		{ PATTERN(Node), 0, NULL, 0, 0, PatternMatch_CStyleNode, NULL, NULL, NULL, TypeCheck_Node, },
		{ PATTERN(Param), 0, NULL, 0, 0, PatternMatch_CStyleParam, Nodeession_OperatorMethod, Statement_ParamsDecl, NULL, TypeCheck_MethodCall,},
		{ PATTERN(Token), 0, NULL, 0, 0, PatternMatch_Token/*PatternMatch_Toks*/, },
		{ TOKEN(DOT), 0, NULL, Precedence_CStyleCALL, 0, NULL, Nodeession_DOT, },
		{ TOKEN(DIV), 0, NULL, Precedence_CStyleMUL, },
		{ TOKEN(MOD), 0, NULL, Precedence_CStyleMUL, },
		{ TOKEN(MUL), 0, NULL, Precedence_CStyleMUL, },
		{ TOKEN(ADD), 0, NULL, Precedence_CStyleADD, },
		{ TOKEN(SUB), 0, NULL, Precedence_CStyleADD, Precedence_CStylePREUNARY, },
		{ TOKEN(LT),  0, NULL, Precedence_CStyleCOMPARE, },
		{ TOKEN(LTE), 0, NULL, Precedence_CStyleCOMPARE, },
		{ TOKEN(GT),  0, NULL, Precedence_CStyleCOMPARE, },
		{ TOKEN(GTE), 0, NULL, Precedence_CStyleCOMPARE, },
		{ TOKEN(EQ),  0, NULL, Precedence_CStyleEQUALS, },
		{ TOKEN(NEQ), 0, NULL, Precedence_CStyleEQUALS, },
		{ TOKEN(LET), SYNFLAG_NodeLeftJoinOp2, NULL, Precedence_CStyleASSIGN, 0, NULL, Nodeession_OperatorMethod, NULL, NULL, TypeCheck_Assign, },
		{ TOKEN(AND), 0, NULL, Precedence_CStyleAND, 0, NULL, Nodeession_OperatorMethod, NULL, NULL, TypeCheck_AndOperator, },
		{ TOKEN(OR),  0, NULL, Precedence_CStyleOR,  0, NULL, Nodeession_OperatorMethod, NULL, NULL, TypeCheck_OrOperator, },
		{ TOKEN(NOT), 0, NULL, 0, Precedence_CStylePREUNARY, },
		{ TOKEN(COLON), 0, NULL, Precedence_CStyleTRINARY, },  // colon
		{ TOKEN(COMMA),   0, NULL, Precedence_CStyleCOMMA, 0, NULL, Nodeession_COMMA, },
		{ TOKEN(DOLLAR),  0, NULL, 0, 0, NULL, Nodeession_DOLLAR, },
		{ TOKEN(true),    0, NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_true, },
		{ TOKEN(false),   0, NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_false, },
		{ PATTERN(Node),  0, "$Node", 0, 0, PatternMatch_Nodeession, Nodeession_ParsedNode, Statement_Nodeession, Statement_Nodeession, NULL, },
		{ PATTERN(Type),  0, NULL, 0, 0, PatternMatch_Type, NULL, NULL, NULL/*Statement_TypeDecl*/, TypeCheck_Type, },
		{ PATTERN(TypeDecl),   0, "$TypeDecl $Type $Node", 0, 0, PatternMatch_TypeDecl, NULL, NULL, Statement_TypeDecl, },
		{ PATTERN(MethodDecl), 0, "$MethodDecl $Type [ClassName: $Type \".\"] $Symbol $Param [$Node]", 0, 0, PatternMatch_MethodDecl, NULL, Statement_MethodDecl, NULL, NULL, },
		{ TOKEN(if),     0, "\"if\" \"(\" $Node \")\" $Node [\"else\" else: $Node]", 0, 0, NULL, NULL, NULL, Statement_if, NULL, },
		{ TOKEN(else),   0,  "\"else\" $Node", 0, 0, NULL, NULL, /*Statement_else*/NULL, Statement_else, NULL, },
		{ TOKEN(return), SYNFLAG_NodeBreakExec, "\"return\" [$Node]", 0, 0, NULL, NULL, NULL, Statement_return, NULL, },
		{ KSymbol_("new"), 0, NULL, 0, Precedence_CStyleCALL, NULL, Nodeession_new, NULL, NULL, NULL, },
		{ KSymbol_END, },
	};
	kNameSpace_DefineSyntax(kctx, ns, SYNTAX, NULL);
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
