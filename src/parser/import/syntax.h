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

static KMETHOD PatternMatch_Expression(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	int returnIdx = beginIdx;
	kNode *expr = ParseNewNode(kctx, kNode_ns(stmt), tokenList, &returnIdx, endIdx, ParseExpressionOption|OnlyPatternMatch, NULL);
	if(expr != K_NULLNODE) {
		kNode_AddParsedObject(kctx, stmt, name, UPCAST(expr));
	}
	KReturnUnboxValue(returnIdx);
}

static KMETHOD PatternMatch_Type(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	KClass *foundClass = NULL;
	int returnIdx = ParseTypePattern(kctx, kNode_ns(stmt), tokenList, beginIdx, endIdx, &foundClass);
	if(foundClass != NULL) {
		kTokenVar *tk = new_(TokenVar, 0, OnVirtualField);
		kNode_AddParsedObject(kctx, stmt, name, UPCAST(tk));
		kToken_SetTypeId(kctx, tk, kNode_ns(stmt), foundClass->typeId);
	}
	KReturnUnboxValue(returnIdx);
}

static KMETHOD Expression_Term(KonohaContext *kctx, KonohaStack *sfp);
static KMETHOD PatternMatch_MethodName(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Parse(stmt, name, tokenList, beginIdx, opIdx, endIdx);
	if(opIdx == -1) {
		int returnIdx = -1;
		kTokenVar *tk = tokenList->TokenVarItems[beginIdx];
		if(IS_String(tk->text)) {
			kNode_AddParsedObject(kctx, stmt, name, UPCAST(tk));
			returnIdx = beginIdx + 1;
		}
		KReturnUnboxValue(returnIdx);
	}
	else {
		Expression_Term(kctx, sfp);
	}
}

static KMETHOD PatternMatch_CStyleBlock(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	kToken *tk = tokenList->TokenItems[beginIdx];
//	KdumpTokenArray(kctx, tokenList, beginIdx, endIdx);
	if(tk->tokenType == TokenType_LazyBlock || tk->resolvedSyntaxInfo->keyword == KSymbol_BraceGroup) {
		kNode_AddParsedObject(kctx, stmt, name, UPCAST(tk));
		KReturnUnboxValue(beginIdx+1);
	}
	kNode *block = ParseNewNode(kctx, kNode_ns(stmt), tokenList, &beginIdx, endIdx, ParseMetaPatternOption, NULL);
	kNode_AddParsedObject(kctx, stmt, name, UPCAST(block));
	KReturnUnboxValue(beginIdx);
}

static kbool_t IsStatementEnd(KonohaContext *kctx, kToken *tk)
{
	return kToken_IsStatementSeparator(tk);
}

static KMETHOD Parse_Block(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Parse(block, name, tokenList, beginIdx, opIdx, endIdx);
	if(opIdx != -1) {
		AppendParsedNode(kctx, block, tokenList, beginIdx, endIdx, IsStatementEnd, ParseMetaPatternOption, NULL);
		KReturnUnboxValue(endIdx);
	}
}

static KMETHOD Expression_Block(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(node, tokenList, beginIdx, opIdx, endIdx);
	if(beginIdx == opIdx) {
		kNameSpace *ns = kNode_ns(node);
		kToken *groupToken = kToken_ToBraceGroup(kctx, tokenList->TokenVarItems[beginIdx], ns, NULL);
		AppendParsedNode(kctx, node, RangeGroup(groupToken->GroupTokenList), IsStatementEnd, ParseMetaPatternOption, NULL);
		KReturnUnboxValue(beginIdx+1);
	}
	KReturnUnboxValue(-1);
}

static KMETHOD TypeCheck_Block(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck2(stmt, expr, ns, reqc);
	KReturn(TypeCheckBlock(kctx, expr, ns, reqc));
}

static KMETHOD PatternMatch_Token(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	DBG_ASSERT(beginIdx < endIdx);
	kToken *tk = tokenList->TokenItems[beginIdx];
	if(!kToken_IsStatementSeparator(tk) && !kToken_IsIndent(tk)) {
		kNode_AddParsedObject(kctx, stmt, name, UPCAST(tk));
		KReturnUnboxValue(beginIdx+1);
	}
	KReturnUnboxValue(-1);
}

static KMETHOD PatternMatch_TypeDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	kNameSpace *ns = kNode_ns(stmt);
	KClass *foundClass = NULL;
	int nextIdx = ParseTypePattern(kctx, ns, tokenList, beginIdx, endIdx, &foundClass);
	//DBG_P("@ nextIdx = %d < %d", nextIdx, endIdx);
	if(nextIdx != -1) {
		nextIdx = TokenUtils_SkipIndent(tokenList, nextIdx, endIdx);
		if(nextIdx < endIdx) {
			kToken *tk = tokenList->TokenItems[nextIdx];
			if(tk->tokenType == KSymbol_SymbolPattern) {
				KReturnUnboxValue(ParseSyntaxPattern(kctx, ns, stmt, stmt->syn, tokenList, beginIdx, endIdx));
			}
		}
	}
	KReturnUnboxValue(-1);
}

static KMETHOD PatternMatch_MethodDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	kNameSpace *ns = kNode_ns(stmt);
	KClass *foundClass = NULL;
	int nextIdx = ParseTypePattern(kctx, ns, tokenList, beginIdx, endIdx, &foundClass);
	//DBG_P("@ nextIdx = %d < %d found=%p", nextIdx, endIdx, foundClass);
	SUGAR dumpTokenArray(kctx, 0, tokenList, beginIdx, endIdx);
	if(nextIdx != -1) {
		nextIdx = TokenUtils_SkipIndent(tokenList, nextIdx, endIdx);
		if(nextIdx < endIdx) {
			kToken *tk = tokenList->TokenItems[nextIdx];
			if(ParseTypePattern(kctx, ns, tokenList, nextIdx, endIdx, NULL) != -1) {
				KReturnUnboxValue(ParseSyntaxPattern(kctx, ns, stmt, stmt->syn, tokenList, beginIdx, endIdx));
			}
			if(tk->tokenType == KSymbol_ParenthesisGroup) {
				KReturnUnboxValue(ParseSyntaxPattern(kctx, ns, stmt, stmt->syn, tokenList, beginIdx, endIdx));
			}
			if(tk->tokenType == KSymbol_SymbolPattern) {
				int symbolNextIdx = TokenUtils_SkipIndent(tokenList, nextIdx + 1, endIdx);
				if(symbolNextIdx < endIdx && tokenList->TokenItems[symbolNextIdx]->tokenType == KSymbol_ParenthesisGroup) {
					KReturnUnboxValue(ParseSyntaxPattern(kctx, ns, stmt, stmt->syn, tokenList, beginIdx, endIdx));
				}
				KReturnUnboxValue(-1);
			}
		}
	}
	KReturnUnboxValue(-1);
}

/* ------------------------------------------------------------------------ */

//static KMETHOD Expression_ParsedNode(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_Expression(stmt, tokenList, beginIdx, opIdx, endIdx);
//	if(beginIdx == opIdx) {
//		kToken *tk = tokenList->TokenItems[opIdx];
//		DBG_ASSERT(IS_Node(tk->parsedNode));
//		KReturn(tk->parsedNode);
//	}
//}

static KMETHOD Parse_Pattern(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Parse(node, name, tokenList, beginIdx, opIdx, endIdx);
	if(beginIdx == opIdx) {
		kToken *tk = tokenList->TokenVarItems[beginIdx];
		node->syn = tk->resolvedSyntaxInfo;
		KFieldSet(node, node->KeyOperatorToken, tk);
		KReturnUnboxValue(ParseSyntaxPattern(kctx, kNode_ns(node), node, node->syn, tokenList, beginIdx, endIdx));
	}
	KReturnUnboxValue(-1);
}

static KMETHOD Expression_Term(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(node, tokenList, beginIdx, opIdx, endIdx);
	if(beginIdx == opIdx) {
		kNameSpace *ns = kNode_ns(node);
		kToken *tk = tokenList->TokenItems[opIdx];
		KClass *foundClass = NULL;
		int nextIdx = ParseTypePattern(kctx, ns, tokenList, beginIdx, endIdx, &foundClass);
		if(foundClass != NULL) {
			kToken_SetTypeId(kctx, tk, ns, foundClass->typeId);
		}
		else {
			nextIdx = opIdx + 1;
		}
		kNode_Termnize(kctx, node, tk);
		KReturnUnboxValue(nextIdx);
	}
	KReturnUnboxValue(-1);
}

static KMETHOD Expression_Operator(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(node, tokenList, beginIdx, opIdx, endIdx);
	kNameSpace *ns = kNode_ns(node);
	int returnIdx = opIdx + 1;
	kTokenVar *tk = tokenList->TokenVarItems[opIdx];
	kNode *rexpr = ParseNewNode(kctx, ns, tokenList, &returnIdx, endIdx, ParseExpressionOption, KToken_t(tk));
	if(beginIdx == opIdx) { // unary operator
		kNode_Op(kctx, node, tk, 1, rexpr);
	}
	else {   // binary operator
		kNode_Op(kctx, node, tk, 2, ParseNewNode(kctx, ns, tokenList, &beginIdx, opIdx, ParseExpressionOption, NULL), rexpr);
	}
	KReturnUnboxValue(returnIdx);
}

static KMETHOD Expression_Member(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Parse(node, name, tokenList, beginIdx, opIdx, endIdx);
	if(opIdx == -1) {
		kNode_AddParsedObject(kctx, node, name, tokenList->ObjectItems[beginIdx]);
		KReturnUnboxValue(beginIdx + 1);
	}
	if(beginIdx < opIdx) {
		kNameSpace *ns = kNode_ns(node);
		kNode_Op(kctx, node, tokenList->TokenItems[opIdx], 1, ParseNewNode(kctx, ns, tokenList, &beginIdx, opIdx, ParseExpressionOption, NULL));
		KReturnUnboxValue(opIdx + 1);
	}
	KReturnUnboxValue(PatternNoMatch);
}

static KMETHOD Expression_Parenthesis(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(node, tokenList, beginIdx, opIdx, endIdx);
	kToken *parenthesisToken = tokenList->TokenItems[opIdx];
	if(beginIdx == opIdx) {
		ParseNode(kctx, node, RangeGroup(parenthesisToken->GroupTokenList), ParseExpressionOption, "(");
	}
	else {
		kNameSpace *ns = kNode_ns(node);
		kNode *lnode = ParseNewNode(kctx, ns, tokenList, &beginIdx, opIdx, ParseExpressionOption, NULL);
		kNode_Termnize(kctx, node, parenthesisToken);
		kNode_AddNode(kctx, node, lnode);
		kNode_AddNode(kctx, node, K_NULLNODE);
		if(kArray_size(parenthesisToken->GroupTokenList) > 2) {
			AppendParsedNode(kctx, node, RangeGroup(parenthesisToken->GroupTokenList), NULL, ParseExpressionOption, "(");
		}
	}
	KReturnUnboxValue(opIdx+1);
}

static KMETHOD Expression_Indexer(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, opIdx, endIdx);
	kNameSpace *ns = kNode_ns(stmt);
	if(beginIdx < opIdx) {
		kTokenVar *groupToken = tokenList->TokenVarItems[opIdx];
		groupToken->symbol = KMethodName_ToGetter(0);
//		getToken->resolvedSyntaxInfo = groupToken->kSyntax_(ns, KSymbol_MemberPattern/*MethodCall*/);
		kNode_Op(kctx, stmt, groupToken, 1, SUGAR ParseNewNode(kctx, ns, tokenList, &beginIdx, opIdx, ParseExpressionOption, NULL));
		AppendParsedNode(kctx, stmt, RangeGroup(groupToken->GroupTokenList), NULL, ParseExpressionOption, "[");
		KReturnUnboxValue(opIdx + 1);
	}
	KReturnUnboxValue(-1);
}

static KMETHOD TypeCheck_Getter(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck2(stmt, expr, ns, reqc);
	kNode *self = SUGAR TypeCheckNodeAt(kctx, expr, 1, ns, KClass_INFER, 0);
	if(kNode_IsError(self)) {
		KReturn(self);
	}
	kToken *fieldToken = expr->NodeList->TokenItems[0];
	ksymbol_t fn = fieldToken->symbol;
	kMethod *mtd = KLIB kNameSpace_GetGetterMethodNULL(kctx, ns, KClass_(self->attrTypeId), fn);
	if(mtd != NULL) {
		KReturn(SUGAR TypeCheckMethodParam(kctx, mtd, expr, ns, reqc));
	}
	else {  // dynamic field    o.name => o.get(name)
		kparamtype_t p[1] = {{KType_Symbol}};
		kparamId_t paramdom = KLIB Kparamdom(kctx, 1, p);
		mtd = KLIB kNameSpace_GetMethodBySignatureNULL(kctx, ns, KClass_(self->attrTypeId), KMethodNameAttr_Getter, paramdom, 1, p);
		if(mtd != NULL) {
			KLIB kArray_Add(kctx, expr->NodeList, new_UnboxConstNode(kctx, ns, KType_Symbol, KSymbol_Unmask(fn)));
			KReturn(SUGAR TypeCheckMethodParam(kctx, mtd, expr, ns, reqc));
		}
	}
	SUGAR MessageNode(kctx, stmt, fieldToken, ns, ErrTag, "undefined field: %s", kString_text(fieldToken->text));
}

static KMETHOD Expression_COMMA(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(node, tokenList, beginIdx, opIdx, endIdx);
	kNode_Op(kctx, node, tokenList->TokenItems[opIdx], 0);
	AppendParsedNode(kctx, node, tokenList, beginIdx, endIdx, NULL, ParseExpressionOption, NULL);
	KReturnUnboxValue(endIdx);
}

static KMETHOD Expression_new(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(expr, tokenList, beginIdx, opIdx, endIdx);
	if(beginIdx == opIdx && beginIdx + 1 < endIdx) {
		kNameSpace *ns = kNode_ns(expr);
		KClass *foundClass = NULL;
		int nextIdx = SUGAR ParseTypePattern(kctx, ns, tokenList, beginIdx + 1, endIdx, &foundClass);
		if(foundClass != NULL) {
			kNode_setnode(expr, KNode_New);
			expr->attrTypeId = foundClass->typeId;
			KReturnUnboxValue(nextIdx);
		}
	}
	KReturnUnboxValue(-1);
}

/* ------------------------------------------------------------------------ */
/* Expression TyCheck */

static kString *ResolveStringEscapeSequenceNULL(KonohaContext *kctx, kToken *tk, size_t start)
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
			/* compatible with ECMA-262
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
			default: return NULL;
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
	VAR_TypeCheck2(stmt, expr, ns, reqc);
	kToken *tk = expr->TermToken;
	kString *text = tk->text;
	if(kToken_Is(RequiredReformat, tk)) {
		const char *escape = strchr(kString_text(text), '\\');
		DBG_ASSERT(escape != NULL);
		text = ResolveStringEscapeSequenceNULL(kctx, tk, escape - kString_text(text));
		if(text == NULL) {
			KReturn(ERROR_UndefinedEscapeSequence(kctx, stmt, tk));
		}
	}
	kString_Set(Literal, ((kStringVar *)text), true);
	KReturn(SUGAR kNode_SetConst(kctx, expr, NULL, UPCAST(text)));
}

static KMETHOD TypeCheck_Type(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck2(stmt, expr, ns, reqc);
	DBG_ASSERT(Token_isVirtualTypeLiteral(expr->TermToken));
	KReturn(SUGAR kNode_SetVariable(kctx, expr, KNode_Null, expr->TermToken->resolvedTypeId, 0));
}

static KMETHOD TypeCheck_true(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck2(stmt, expr, ns, reqc);
	KReturn(SUGAR kNode_SetUnboxConst(kctx, expr, KType_Boolean, (uintptr_t)1));
}

static KMETHOD TypeCheck_false(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck2(stmt, expr, ns, reqc);
	KReturn(SUGAR kNode_SetUnboxConst(kctx, expr, KType_Boolean, (uintptr_t)0));
}

static KMETHOD TypeCheck_IntLiteral(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck2(stmt, expr, ns, reqc);
	kToken *tk = expr->TermToken;
	long long n = strtoll(kString_text(tk->text), NULL, 0);
	KReturn(SUGAR kNode_SetUnboxConst(kctx, expr, KType_Int, (uintptr_t)n));
}

static KMETHOD TypeCheck_AndOperator(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck2(stmt, expr, ns, reqc);
	kNode *returnNode = TypeCheckNodeAt(kctx, expr, 1, ns, KClass_Boolean, 0);
	if(!kNode_IsError(returnNode)) {
		returnNode = TypeCheckNodeAt(kctx, expr, 2, ns, KClass_Boolean, 0);
		if(!kNode_IsError(returnNode)) {
			returnNode = kNode_Type(kctx, expr, KNode_And, KType_Boolean);
		}
	}
	KReturn(returnNode);
}

static KMETHOD TypeCheck_OrOperator(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck2(stmt, expr, ns, reqc);
	kNode *returnNode = TypeCheckNodeAt(kctx, expr, 1, ns, KClass_Boolean, 0);
	if(!kNode_IsError(returnNode)) {
		returnNode = TypeCheckNodeAt(kctx, expr, 2, ns, KClass_Boolean, 0);
		if(!kNode_IsError(returnNode)) {
			returnNode = kNode_Type(kctx, expr, KNode_Or, KType_Boolean);
		}
	}
	KReturn(returnNode);
}

static kbool_t kNode_IsGetter(kNode *expr)
{
	if(kNode_node(expr) == KNode_MethodCall) {  // check getter and transform to setter
		kMethod *mtd = expr->NodeList->MethodItems[0];
		DBG_ASSERT(IS_Method(mtd));
		if(KMethodName_IsGetter(mtd->mn)) return true;
	}
	return false;
}

static kNode *MakeNodeSetter(KonohaContext *kctx, kNode *expr, kNameSpace *ns, kNode *rightHandNode, KClass *reqc)
{
	kMethod *mtd = expr->NodeList->MethodItems[0];
	DBG_ASSERT(KMethodName_IsGetter(mtd->mn));
	KClass *c = KClass_(mtd->typeId);
	kParam *pa = kMethod_GetParam(mtd);
	int i, psize = pa->psize + 1;
	kparamtype_t p[psize];
	for(i = 0; i < (int) pa->psize; i++) {
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
		return SUGAR TypeCheckMethodParam(kctx, foundMethod, expr, ns, reqc);
	}
	return SUGAR MessageNode(kctx, expr, NULL, ns, ErrTag, "undefined setter");
}

static KMETHOD TypeCheck_Assign(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck2(stmt, expr, ns, reqc);
	kNode *leftHandNode = SUGAR TypeCheckNodeAt(kctx, expr, 1, ns, KClass_INFER, TypeCheckPolicy_AllowVoid);
	kNode *rightHandNode = SUGAR TypeCheckNodeAt(kctx, expr, 2, ns, KClass_(leftHandNode->attrTypeId), 0);
	if(kNode_IsError(leftHandNode)) {
		KReturn(leftHandNode);
	}
	if(kNode_IsError(rightHandNode)) {
		KReturn(rightHandNode);
	}
	kNode *returnNode = K_NULLNODE;
	if(kNode_node(leftHandNode) == KNode_Local || kNode_node(leftHandNode) == KNode_Field) {
		if(KTypeAttr_Is(ReadOnly, leftHandNode->attrTypeId)) {
			returnNode = SUGAR MessageNode(kctx, expr, leftHandNode->TermToken, ns, ErrTag, "read only: %s", KToken_t(leftHandNode->TermToken));
		}
		else {
			returnNode = kNode_Type(kctx, expr, KNode_Assign, leftHandNode->attrTypeId);
		}
	}
	else if(kNode_IsGetter(leftHandNode)) {
		returnNode = MakeNodeSetter(kctx, leftHandNode, ns, rightHandNode, reqc);
	}
	else {
		KDump(leftHandNode);
		returnNode = SUGAR MessageNode(kctx, expr, NULL, ns, ErrTag, "assignment: variable name is expected");
	}
	KReturn(returnNode);
}

static int AddLocalVariable(KonohaContext *kctx, kNameSpace *ns, ktypeattr_t attrTypeId, ksymbol_t name)
{
	struct KGammaStack *s = &(ns->genv->localScope);
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

static kNode *new_GetterNode(KonohaContext *kctx, kToken *tkU, kMethod *mtd, kNode *expr)
{
	return new_TypedNode(kctx, kNode_ns(expr), KNode_MethodCall, kMethod_GetReturnType(mtd), 2, mtd, expr);
}

static kNode *TypeVariableNULL(KonohaContext *kctx, kNode *expr, kNameSpace *ns, KClass *reqc)
{
	DBG_ASSERT(expr->attrTypeId == KType_var);
	kToken *tk = expr->TermToken;
	ksymbol_t symbol = tk->symbol;
	int i;
	struct KGammaLocalData *genv = ns->genv;
	for(i = genv->localScope.varsize - 1; i >= 0; i--) {
		if(genv->localScope.varItems[i].name == symbol) {
			return SUGAR kNode_SetVariable(kctx, expr, KNode_Local, genv->localScope.varItems[i].attrTypeId, i);
		}
	}
	if(kNameSpace_Is(ImplicitField, ns)) {
		if(genv->localScope.varItems[0].attrTypeId != KType_void) {
			KClass *ct = genv->thisClass;
			if(ct->fieldsize > 0) {
				for(i = ct->fieldsize; i >= 0; i--) {
					if(ct->fieldItems[i].name == symbol && ct->fieldItems[i].attrTypeId != KType_void) {
						return SUGAR kNode_SetVariable(kctx, expr, KNode_Field, ct->fieldItems[i].attrTypeId, longid((kshort_t)i, 0));
					}
				}
			}
			kMethod *mtd = kNameSpace_GetGetterMethodNULL(kctx, ns, genv->thisClass, symbol);
			if(mtd != NULL) {
				return new_GetterNode(kctx, tk, mtd, new_VariableNode(kctx, ns, KNode_Local, genv->thisClass->typeId, 0));
			}
		}
	}
	if(genv->thisClass->baseTypeId == KType_Func) {
		kMethod *mtd = kNameSpace_GetGetterMethodNULL(kctx, ns, genv->thisClass, symbol);
		if(mtd != NULL) {
			return new_GetterNode(kctx, tk, mtd, new_VariableNode(kctx, ns, KNode_Local, genv->thisClass->typeId, 0));
		}
	}

	if((kNameSpace_IsTopLevel(ns) || kNameSpace_Is(ImplicitGlobalVariable, ns)) && ns->globalObjectNULL != NULL) {
		KClass *globalClass = kObject_class(ns->globalObjectNULL);
		kMethod *mtd = kNameSpace_GetGetterMethodNULL(kctx, ns, globalClass, symbol);
		if(mtd != NULL) {
			return new_GetterNode(kctx, tk, mtd, new_ConstNode(kctx, ns, globalClass, ns->globalObjectNULL));
		}
	}
	kMethod *mtd = kNameSpace_GetNameSpaceFuncNULL(kctx, ns, symbol, reqc);  // finding function
	if(mtd != NULL) {
		kParam *pa = kMethod_GetParam(mtd);
		KClass *ct = KLIB KClass_Generics(kctx, KClass_Func, pa->rtype, pa->psize, (kparamtype_t *)pa->paramtypeItems);
		kFuncVar *fo = (kFuncVar *)KLIB new_kObject(kctx, OnGcStack, ct, (uintptr_t)mtd);
		//KFieldSet(fo, fo->self, UPCAST(ns));
		return new_ConstNode(kctx, ns, ct, UPCAST(fo));
	}
	if(symbol != KSymbol_Noname) {
		KKeyValue *kv = kNameSpace_GetConstNULL(kctx, ns, symbol, false/*isLocalOnly*/);
		if(kv != NULL) {
			if(KTypeAttr_Is(Boxed, kv->attrTypeId)) {
				SUGAR kNode_SetConst(kctx, expr, NULL, kv->ObjectValue);
			}
			else {
				SUGAR kNode_SetUnboxConst(kctx, expr, kv->attrTypeId, kv->unboxValue);
			}
			return expr;
		}
	}
	return NULL;
}

static KMETHOD TypeCheck_Symbol(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck2(stmt, expr, ns, reqc);
	kNode *texpr = TypeVariableNULL(kctx, expr, ns, reqc);
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
		pol = pol | TypeCheckPolicy_Coercion;
	}
	return pol;
}

static kNodeVar* TypeMethodCallNode(KonohaContext *kctx, kNodeVar *expr, kMethod *mtd, KClass *reqClass)
{
	kNode *thisNode = kNode_At(expr, 1);
	KFieldSet(expr->NodeList, expr->NodeList->MethodItems[0], mtd);
	KClass *typedClass = ResolveTypeVariable(kctx, kMethod_GetReturnType(mtd), KClass_(thisNode->attrTypeId));
	if(kNode_node(thisNode) == KNode_New) {
		typedClass = KClass_(thisNode->attrTypeId);
	}
	else if(kMethod_Is(SmartReturn, mtd) && reqClass->typeId != KType_var) {
		typedClass = reqClass;
	}
	kNode_Type(kctx, expr, KNode_MethodCall, typedClass->typeId);
	return expr;
}

static kNode *BoxThisNode(KonohaContext *kctx, kNode *expr, kNameSpace *ns, kMethod *mtd, KClass **thisClassRef)
{
	kNode *thisNode = expr->NodeList->NodeItems[1];
	KClass *thisClass = KClass_(thisNode->attrTypeId);
	DBG_ASSERT(thisClass->typeId != KType_var);
	//DBG_P("mtd_cid=%s this=%s", KType_text(mtd->typeId), KClass_text(thisClass));
	if(!KType_Is(UnboxType, mtd->typeId) && KClass_Is(UnboxType, thisClass)) {
		thisNode = kNode_SetNodeAt(kctx, expr, 1, BoxNode(kctx, thisNode, ns, thisClass));
	}
	thisClassRef[0] = thisClass;
	return thisNode;
}

static kNode *TypeCheckMethodParam(KonohaContext *kctx, kMethod *mtd, kNode *expr, kNameSpace *ns, KClass* reqc)
{
	DBG_ASSERT(IS_Method(mtd));
	KFieldSet(expr->NodeList, expr->NodeList->MethodItems[0], mtd);
	KClass *thisClass = NULL;
	kNode *thisNode = BoxThisNode(kctx, expr, ns, mtd, &thisClass);
	kbool_t isConst = kNode_IsConstValue(thisNode);
	kParam *pa = kMethod_GetParam(mtd);
	DBG_ASSERT(pa->psize + 2U <= kArray_size(expr->NodeList));
	size_t i;
	for(i = 0; i < pa->psize; i++) {
		size_t n = i + 2;
		KClass* paramType = ResolveTypeVariable(kctx, KClass_(pa->paramtypeItems[i].attrTypeId), thisClass);
		int tycheckPolicy = TypeCheckPolicy_(pa->paramtypeItems[i].attrTypeId);
		kNode *texpr = SUGAR TypeCheckNodeAt(kctx, expr, n, ns, paramType, tycheckPolicy);
		if(kNode_IsError(texpr)) {
			SUGAR MessageNode(kctx, expr, NULL, ns, InfoTag, "%s.%s%s accepts %s at the parameter %d", kMethod_Fmt3(mtd), KClass_text(paramType), (int)i+1);
			return texpr;
		}
		if(!kNode_IsConstValue(texpr)) isConst = 0;
	}
	expr = TypeMethodCallNode(kctx, expr, mtd, reqc);
	if(isConst && kMethod_Is(Const, mtd)) {
		KClass *rtype = ResolveTypeVariable(kctx, KClass_(pa->rtype), thisClass);
		return MakeNodeConst(kctx, expr, rtype);
	}
	return kNode_Rebase(kctx, expr, ns->genv->localScope.varsize);
}

//static kNode *TypeCheckDynamicCallParams(KonohaContext *kctx, kNode *stmt, kNodeVar *expr, kMethod *mtd, kNameSpace *ns, kString *name, kmethodn_t mn, KClass *reqClass)
//{
//	size_t i;
//	kParam *pa = kMethod_GetParam(mtd);
//	KClass* ptype = (pa->psize == 0) ? KClass_Object : KClass_(pa->paramtypeItems[0].attrTypeId);
//	for(i = 2; i < kArray_size(expr->NodeList); i++) {
//		kNode *texpr = SUGAR TypeCheckNodeAt(kctx, expr, i, ns, ptype, 0);
//		if(kNode_IsError(texpr) /* texpr = K_NULLNODE */) return texpr;
//	}
//	kNode_AddNode(kctx, expr, new_ConstNode(kctx, kNode_ns(expr), NULL, UPCAST(name)));
//	return TypeMethodCallNode(kctx, expr, mtd, reqClass);
//}
//
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

static kMethod *LookupOverloadedMethod(KonohaContext *kctx, kMethod *mtd, kNode *expr, kNameSpace *ns)
{
	KClass *thisClass = KClass_(expr->NodeList->NodeItems[1]->attrTypeId);
	size_t i, psize = kArray_size(expr->NodeList) - 2;
	kparamtype_t *p = ALLOCA(kparamtype_t, psize);
	kParam *pa = kMethod_GetParam(mtd);
	for(i = 0; i < psize; i++) {
		size_t n = i + 2;
		KClass *paramType = (i < pa->psize) ? ResolveTypeVariable(kctx, KClass_(pa->paramtypeItems[i].attrTypeId), thisClass) : KClass_INFER;
		kNode *texpr = SUGAR TypeCheckNodeAt(kctx, expr, n, ns, paramType, TypeCheckPolicy_NoCheck);
		if(kNode_IsError(texpr)) return NULL;
		p[i].attrTypeId = texpr->attrTypeId;
	}
	kparamId_t paramdom = KLIB Kparamdom(kctx, psize, p);
	return kNameSpace_GetMethodBySignatureNULL(kctx, ns, thisClass, mtd->mn, paramdom, psize, p);
}

static kMethod *LookupMethod(KonohaContext *kctx, kNode *expr, kNameSpace *ns)
{
	KClass *thisClass = KClass_(kNode_At(expr, 1)->attrTypeId);
	kToken *methodToken = expr->NodeList->TokenVarItems[0];
	DBG_ASSERT(IS_Token(methodToken));
	size_t psize = kArray_size(expr->NodeList) - 2;
	kMethod *mtd = kNameSpace_GetMethodByParamSizeNULL(kctx, ns, thisClass, methodToken->symbol, psize, KMethodMatch_CamelStyle);
	if(mtd == NULL && psize == 0) {
		mtd = kNameSpace_GuessCoercionMethodNULL(kctx, ns, methodToken, thisClass);
	}
	if(mtd == NULL) {
		if(methodToken->text != TS_EMPTY) {  // find Dynamic Call ..
			mtd = kNameSpace_GetMethodByParamSizeNULL(kctx, ns, thisClass, 0/*NONAME*/, 1, KMethodMatch_NoOption);
//			if(mtd != NULL) {
//				return TypeCheckDynamicCallParams(kctx, stmt, expr, mtd, ns, methodToken->text, methodToken->symbol, reqc);
//			}
		}
//		if(methodToken->symbol == MN_new && psize == 0 && KClass_(kNode_At(expr, 1)->attrTypeId)->baseTypeId == KType_Object) {
//			return kNode_At(expr, 1);  // new Person(); // default constructor
//		}
		SUGAR MessageNode(kctx, expr, methodToken, ns, ErrTag, "undefined method: %s.%s%s", KClass_text(thisClass), KSymbol_Fmt2(methodToken->symbol));
	}
	if(mtd != NULL) {
		if(kMethod_Is(Overloaded, mtd)) {
			mtd = LookupOverloadedMethod(kctx, mtd, expr, ns);
		}
	}
	return mtd;
}

static KMETHOD TypeCheck_MethodCall(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(expr, ns, reqc);
	kNode *texpr = K_NULLNODE;
	KDump(expr);
	DBG_ASSERT(IS_Array(expr->NodeList));
	kMethod *mtd = expr->NodeList->MethodItems[0];
	DBG_ASSERT(mtd != NULL);
	if(!IS_Method(mtd)) {
		texpr = SUGAR TypeCheckNodeAt(kctx, expr, 1, ns, KClass_INFER, 0);
		mtd = (kNode_IsError(texpr)) ? NULL : LookupMethod(kctx, expr, ns);
	}
	if(mtd != NULL) {
		texpr = TypeCheckMethodParam(kctx, mtd, expr, ns, reqc);
		KReturn(texpr);
	}
	KReturn(expr);
}

// --------------------------------------------------------------------------
// FuncStyleCall

static kNode *TypeFuncParam(KonohaContext *kctx, kNodeVar *expr, kNameSpace *ns)
{
	KClass *thisClass = KClass_(kNode_At(expr, 0)->attrTypeId);
	kParam *pa = KClass_cparam(thisClass);
	size_t i, size = kArray_size(expr->NodeList);
	if(pa->psize + 2U != size) {
		return SUGAR MessageNode(kctx, expr, NULL, ns, ErrTag, "function %s takes %d parameter(s), but given %d parameter(s)", KClass_text(thisClass), (int)pa->psize, (int)size-2);
	}
	for(i = 0; i < pa->psize; i++) {
		size_t n = i + 2;
		kNode *texpr = SUGAR TypeCheckNodeAt(kctx, expr, n, ns, KClass_(pa->paramtypeItems[i].attrTypeId), 0);
		if(kNode_IsError(texpr) /* texpr = K_NULLNODE */) {
			return texpr;
		}
	}
	kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, kNode_ns(expr), KClass_Func, KMethodName_("Invoke"), -1, KMethodMatch_NoOption);
	/* [before] [Func, nulltoken, Arg1, Arg2, Arg3 ..]
	 * [after]  [null, Func, Arg1, Arg2, Arg3 ..]
	 */
	kArray *List = expr->NodeList;
	KFieldSet(List, List->ObjectItems[1], List->ObjectItems[0]);

	DBG_ASSERT(mtd != NULL);
	return TypeMethodCallNode(kctx, expr, mtd, KClass_(thisClass->p0));
}

static kMethod* TypeFirstNodeAndLookupMethod(KonohaContext *kctx, kNodeVar *exprN, kNameSpace *ns, KClass *reqc)
{
	kNodeVar *firstNode = (kNodeVar *)kNode_At(exprN, 0);
	kToken *termToken = firstNode->TermToken;
	ksymbol_t funcName = termToken->symbol;
	struct KGammaLocalData *genv = ns->genv;
	int i;
	for(i = genv->localScope.varsize - 1; i >= 0; i--) {
		if(genv->localScope.varItems[i].name == funcName && KType_IsFunc(genv->localScope.varItems[i].attrTypeId)) {
			SUGAR kNode_SetVariable(kctx, firstNode, KNode_Local, genv->localScope.varItems[i].attrTypeId, i);
			return NULL;
		}
	}
	int paramsize = kArray_size(exprN->NodeList) - 2;
	if(genv->localScope.varItems[0].attrTypeId != KType_void) {
		KClass *ct = genv->thisClass;
		kMethod *mtd = kNameSpace_GetMethodByParamSizeNULL(kctx, ns, genv->thisClass, funcName, paramsize, KMethodMatch_CamelStyle);
		if(mtd != NULL) {
			KFieldSet(exprN->NodeList, exprN->NodeList->NodeItems[1], new_VariableNode(kctx, ns, KNode_Local, ct->typeId, 0));
			return mtd;
		}
		if(ct->fieldsize) {
			for(i = ct->fieldsize; i >= 0; i--) {
				if(ct->fieldItems[i].name == funcName && KType_IsFunc(ct->fieldItems[i].attrTypeId)) {
					SUGAR kNode_SetVariable(kctx, firstNode, KNode_Field, ct->fieldItems[i].attrTypeId, longid((kshort_t)i, 0));
					return NULL;
				}
			}
		}
		mtd = kNameSpace_GetGetterMethodNULL(kctx, ns, genv->thisClass, funcName);
		if(mtd != NULL && kMethod_IsReturnFunc(mtd)) {
			KFieldSet(exprN->NodeList, exprN->NodeList->NodeItems[0], new_GetterNode(kctx, termToken, mtd, new_VariableNode(kctx, ns, KNode_Local, genv->thisClass->typeId, 0)));
			return NULL;
		}
	}
	{
		KKeyValue* kvs = kNameSpace_GetConstNULL(kctx, ns, funcName, false/*isLocalOnly*/);
		if(kvs != NULL && KTypeAttr_Unmask(kvs->attrTypeId) == VirtualType_StaticMethod) {
			KClass *c = KClass_((ktypeattr_t)kvs->unboxValue);
			ksymbol_t alias = (ksymbol_t)(kvs->unboxValue >> (sizeof(ktypeattr_t) * 8));
			kMethod *mtd = kNameSpace_GetMethodByParamSizeNULL(kctx, ns, c, alias, paramsize, KMethodMatch_NoOption);
			if(mtd != NULL && kMethod_Is(Static, mtd)) {
				KFieldSet(exprN->NodeList, exprN->NodeList->NodeItems[1], new_ConstNode(kctx, ns, c, KLIB Knull(kctx, c)));
				return mtd;
			}
		}
	}
	{
		kMethod *mtd = kNameSpace_GetMethodByParamSizeNULL(kctx, ns, kObject_class(ns), funcName, paramsize, KMethodMatch_CamelStyle);
		if(mtd != NULL) {
			KFieldSet(exprN->NodeList, exprN->NodeList->NodeItems[1], new_ConstNode(kctx, ns, kObject_class(ns), UPCAST(ns)));
			return mtd;
		}
	}
	if((kNameSpace_IsTopLevel(ns) || kNameSpace_Is(ImplicitGlobalVariable,ns)) && ns->globalObjectNULL != NULL) {
		KClass *globalClass = kObject_class(ns->globalObjectNULL);
		kMethod *mtd = kNameSpace_GetGetterMethodNULL(kctx, ns, globalClass, funcName);
		if(mtd != NULL && kMethod_IsReturnFunc(mtd)) {
			KFieldSet(exprN->NodeList, exprN->NodeList->NodeItems[0], new_GetterNode(kctx, termToken, mtd, new_ConstNode(kctx, ns, globalClass, ns->globalObjectNULL)));
			return NULL;
		}
		return mtd;
	}
	return NULL;
}

static KMETHOD TypeCheck_FuncStyleCall(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(expr, ns, reqc);
	DBG_ASSERT(expr->NodeList->NodeItems[1] == K_NULLNODE);
	kNode *firstNode = kNode_At(expr, 0);
	DBG_ASSERT(IS_Node(firstNode));
	DBG_P(">>>>>>>>>>>>> firstNode=%s%s", KSymbol_Fmt2(firstNode->syn->keyword));
	if(firstNode->syn->keyword == KSymbol_MemberPattern) {
		KFieldSet(expr, expr->KeyOperatorToken, firstNode->KeyOperatorToken);
		KFieldSet(expr->NodeList, expr->NodeList->ObjectItems[1], firstNode->NodeList->ObjectItems[1]);
		KFieldSet(expr->NodeList, expr->NodeList->ObjectItems[0], firstNode->NodeList->ObjectItems[0]);
		TypeCheck_MethodCall(kctx, sfp);
		return;
	}
	else if(firstNode->syn->keyword == KSymbol_new) {
		KFieldSet(expr, expr->KeyOperatorToken, firstNode->KeyOperatorToken);
		KFieldSet(expr->NodeList, expr->NodeList->NodeItems[1], firstNode);
		KFieldSet(expr->NodeList, expr->NodeList->TokenItems[0], firstNode->KeyOperatorToken);
		TypeCheck_MethodCall(kctx, sfp);
		return;
	}
	else if(kNode_isSymbolTerm(kNode_At(expr, 0))) {
		kMethod *mtd = TypeFirstNodeAndLookupMethod(kctx, expr, ns, reqc);
		if(mtd != NULL) {
			if(kMethod_Is(Overloaded, mtd)) {
				DBG_P("overloaded found %s.%s%s", kMethod_Fmt3(mtd));
				mtd = LookupOverloadedMethod(kctx, mtd, expr, ns);
			}
			KReturn(TypeCheckMethodParam(kctx, mtd, expr, ns, reqc));
		}
		if(!KType_IsFunc(kNode_At(expr, 0)->attrTypeId)) {
			kToken *tk = kNode_At(expr, 0)->TermToken;
			DBG_ASSERT(IS_Token(tk));  // TODO: make error message in case of not Token
			KReturn(SUGAR MessageNode(kctx, expr, tk, ns, ErrTag, "undefined function: %s", KToken_t(tk)));
		}
	}
	else {
		if(TypeCheckNodeAt(kctx, expr, 0, ns, KClass_INFER, 0) != K_NULLNODE) {
			if(!KType_IsFunc(expr->NodeList->NodeItems[0]->attrTypeId)) {
				KReturn(SUGAR MessageNode(kctx, expr, NULL, ns, ErrTag, "function is expected"));
			}
		}
	}
	KReturn(TypeFuncParam(kctx, expr, ns));
}

// ---------------------------------------------------------------------------
// Statement Node

static KMETHOD Statement_if(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, ns, reqc);
	TypeCheckNodeByName(kctx, stmt, KSymbol_ExprPattern, ns, KClass_Boolean, 0);
	kNode *thenNode = SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_BlockPattern, ns, reqc, TypeCheckPolicy_AllowEmpty);
	if(thenNode != NULL && !kNode_IsError(thenNode)) {
		SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_else, ns, KClass_(thenNode->attrTypeId), TypeCheckPolicy_AllowEmpty);
		KReturn(kNode_Type(kctx, stmt, KNode_If, thenNode->attrTypeId));
	}
}

static kNode *LookupNoElseIfNode(KonohaContext *kctx, kNode *ifNode)
{
	DBG_ASSERT(kNode_node(ifNode) == KNode_If);
	kNode *elseNode = SUGAR kNode_GetNode(kctx, ifNode, KSymbol_else, NULL);
	if(elseNode != NULL) {
		if(kNode_node(elseNode) == KNode_If) {
			return LookupNoElseIfNode(kctx, elseNode);
		}
		if(kNode_GetNodeListSize(kctx, elseNode) == 1) {
			ifNode = elseNode->NodeList->NodeItems[0];
			if(kNode_node(ifNode) == KNode_If) {
				return LookupNoElseIfNode(kctx, ifNode);
			}
		}
		return NULL;
	}
	return ifNode;
}

static kNode *kNode_LookupIfNodeNULL(KonohaContext *kctx, kNode *stmt)
{
	kNode *block = kNode_GetParent(kctx, stmt);
	if(IS_Array(block->NodeList)) {
		kNode *prevIfNode = NULL;
		int i;
		for(i = 0; kArray_size(block->NodeList); i++) {
			kNode *node = block->NodeList->NodeItems[i];
			if(node == stmt) {
				if(prevIfNode != NULL && kNode_node(prevIfNode) == KNode_If) {
					return LookupNoElseIfNode(kctx, prevIfNode);
				}
				return NULL;
			}
			/* skiping the typed 'else if' */
			if(kNode_node(node) == KNode_Done) continue;
			prevIfNode = node;
		}
	}
	return NULL;
}

static KMETHOD Statement_else(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, ns, reqc);
	kNode *ifNode = kNode_LookupIfNodeNULL(kctx, stmt);
	if(ifNode != NULL) {
		kObject *elseNode = kNode_GetObjectNULL(kctx, stmt, KSymbol_BlockPattern);
		DBG_ASSERT(elseNode != NULL);
		SUGAR kNode_AddParsedObject(kctx, ifNode, KSymbol_else, elseNode);
		SUGAR TypeCheckNodeByName(kctx, ifNode, KSymbol_else, ns, KClass_(ifNode->attrTypeId), 0);
		KReturn(kNode_Type(kctx, stmt, KNode_Done, KType_void));
	}
	else {
		KReturn(kNode_Message(kctx, stmt, ErrTag, "else is not statement"));
	}
}

static KMETHOD Statement_return(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, ns, reqc);
	KClass *returnType = kMethod_GetReturnType(ns->genv->currentWorkingMethod);
	if(returnType->typeId != KType_void) {
		TypeCheckNodeByName(kctx, stmt, KSymbol_ExprPattern, ns, returnType, 0);
	} else {
		kNode *expr = (kNode *)kNode_GetObjectNULL(kctx, stmt, KSymbol_ExprPattern);
		if(expr != NULL) {
			kNode_Message(kctx, stmt, WarnTag, "ignored return value");
			KLIB kObjectProto_RemoveKey(kctx, stmt, KSymbol_ExprPattern);
		}
	}
	KReturn(kNode_Type(kctx, stmt, KNode_Return, KType_void));
}

/* TypeDecl */

static kNode *TypeDeclLocalVariable(KonohaContext *kctx, kNode *stmt, kNameSpace *ns, ktypeattr_t attrTypeId, kNode *termNode, kNode *exprNode, kObject *thunk)
{
	int index = AddLocalVariable(kctx, ns, attrTypeId, termNode->TermToken->symbol);
	SUGAR kNode_SetVariable(kctx, termNode, KNode_Local, attrTypeId, index);
	return new_TypedNode(kctx, ns, KNode_Assign, KClass_void, 3, K_NULLTOKEN, termNode, exprNode);
}

static void kNode_DeclType(KonohaContext *kctx, kNode *stmt, kNameSpace *ns, ktypeattr_t attrTypeId, kNode *declNode, kObject *thunk, KTypeDeclFunc TypeDecl)
{
	kNode *newstmt = NULL;
	if(TypeDecl == NULL) TypeDecl = TypeDeclLocalVariable;
	if(declNode->syn->keyword == KSymbol_COMMA) {
		size_t i;
		for(i = 1; i < kArray_size(declNode->NodeList); i++) {
			kNode_DeclType(kctx, stmt, ns, attrTypeId, kNode_At(declNode, i), thunk, TypeDecl);
			if(kNode_IsError(stmt)) break;
		}
	}
	else if(declNode->syn->keyword == KSymbol_LET && kNode_isSymbolTerm(kNode_At(declNode, 1))) {
		kNode *exprNode = TypeCheckNodeAt(kctx, declNode, 2, ns, KClass_(attrTypeId), 0);
		if(kNode_IsError(exprNode)) {
			// this is neccesarry to avoid 'int a = a + 1;';
			kNode_ToError(kctx, stmt, exprNode->ErrorMessage);
			return;
		}
		kNode *nameNode = kNode_At(declNode, 1);
		if(KTypeAttr_Unmask(attrTypeId) == KType_var) {
			ktypeattr_t attr = KTypeAttr_Attr(attrTypeId);
			kToken *termToken = nameNode->TermToken;
			attrTypeId = exprNode->attrTypeId | attr;
			kNodeToken_Message(kctx, stmt, termToken, InfoTag, "%s%s has type %s", KSymbol_Fmt2(termToken->symbol), KType_text(attrTypeId));
		}
		newstmt = TypeDecl(kctx, stmt, ns, attrTypeId, nameNode, exprNode, thunk);
	}
	else if(kNode_isSymbolTerm(declNode)) {
		if(attrTypeId == KType_var  || !KType_Is(Nullable, attrTypeId)) {
			kNode_Message(kctx, stmt, ErrTag, "%s %s%s: initial value is expected", KType_text(attrTypeId), KSymbol_Fmt2(declNode->TermToken->symbol));
			return;
		}
		kNode *exprNode = new_VariableNode(kctx, ns, KNode_Null, attrTypeId, 0);
		newstmt = TypeDecl(kctx, stmt, ns, attrTypeId, declNode, exprNode, thunk);
	}
	else {
		kNode_Message(kctx, stmt, ErrTag, "type declaration: variable name is expected");
		return;
	}
	if(newstmt != NULL && !kNode_IsError(stmt)) {
		kNode_Set(OpenBlock, stmt, true);
		kNode_AddNode(kctx, stmt, newstmt);
		kNode_Type(kctx, stmt, KNode_Block, KType_void);
	}
}

static KMETHOD Statement_TypeDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, ns, reqc);
	if(kNameSpace_IsTopLevel(ns)) {
		SUGAR MessageNode(kctx, stmt, NULL, ns, ErrTag, "unsupported global variable; use Syntax.GlobalVariable");
		SUGAR MessageNode(kctx, stmt, NULL, ns, InfoTag, "global variable is defined in Syntax.GlobalVariable");
	}
	else {
		kToken *tk   = SUGAR kNode_GetToken(kctx, stmt, KSymbol_TypePattern, NULL);
		kNode  *expr = SUGAR kNode_GetNode(kctx, stmt, KSymbol_ExprPattern, NULL);
		ktypeattr_t attrTypeId = Token_typeLiteral(tk);
		kNode_DeclType(kctx, stmt, ns, attrTypeId, expr, NULL, TypeDeclLocalVariable);
	}
	KReturn(stmt);
}

// ------------------
// Method Utilities for MethodDecl

static KMETHOD KMethodFunc_LazyCompilation(KonohaContext *kctx, KonohaStack *sfp)
{
	KonohaStack *esp = kctx->esp;
	kMethod *mtd = sfp[K_MTDIDX].calledMethod;
	kString *text = mtd->SourceToken->text;
	kfileline_t uline = mtd->SourceToken->uline;
	int baseIndent = mtd->SourceToken->indent;
	DBG_P("<<lazy compilation>>: %s.%s%s baseIndent=%d", KType_text(mtd->typeId), KMethodName_Fmt2(mtd->mn), baseIndent);
	kMethod_Compile(kctx, mtd, NULL, mtd->LazyCompileNameSpace, text, uline, baseIndent, DefaultCompileOption/*HatedLazyCompile*/);
	((KonohaContextVar *)kctx)->esp = esp;
	mtd->invokeKMethodFunc(kctx, sfp); // call again;
}

static void kMethod_SetLazyCompilation(KonohaContext *kctx, kMethodVar *mtd, kNode *stmt, kNameSpace *ns)
{
	kToken *sourceToken = SUGAR kNode_GetToken(kctx, stmt, KSymbol_BlockPattern, NULL);
	if(sourceToken != NULL && sourceToken->tokenType == TokenType_LazyBlock) {
		KFieldSet(mtd, mtd->SourceToken, sourceToken);
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
		int baseIndent = mtd->SourceToken->indent;
		((kMethodVar *)mtd)->invokeKMethodFunc = NULL; // TO avoid recursive compile
		mtd = kMethod_Compile(kctx, mtd, callparamNULL, mtd->LazyCompileNameSpace, text, uline, baseIndent, options|HatedLazyCompile);
		DBG_ASSERT(mtd->invokeKMethodFunc != KMethodFunc_LazyCompilation);
	}
	return mtd;
}

/* ------------------------------------------------------------------------ */
/* [ParamDecl] */

static void CheckCStyleParam(KonohaContext *kctx, KTokenSeq* tokens)
{
	int i, count = 0;
	for(i = 0; i < tokens->endIdx; i++) {
		kTokenVar *tk = tokens->tokenList->TokenVarItems[i];
		if(tk->symbol == KSymbol_void) {
			tokens->endIdx = i; //  f(void) = > f()
			return;
		}
		if(tk->symbol == KSymbol_COMMA) {
			tk->resolvedSyntaxInfo = K_NULLTOKEN->resolvedSyntaxInfo;
			count++;
		}
	}
	if(count == 0 && tokens->beginIdx < tokens->endIdx) {
		KFieldSet(tokens->tokenList, tokens->tokenList->TokenItems[tokens->endIdx], K_NULLTOKEN); // to ensure block
		tokens->endIdx += 1;
	}
}

static KMETHOD PatternMatch_CStyleParam(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	int returnIdx = -1;
	kToken *tk = tokenList->TokenItems[beginIdx];
	if(tk->resolvedSyntaxInfo->keyword == KSymbol_ParenthesisGroup) {
		KTokenSeq param = {kNode_ns(stmt), RangeGroup(tk->GroupTokenList)};
		CheckCStyleParam(kctx, &param);
		kNode *block = ParseNewNode(kctx, param.ns, param.tokenList, &param.beginIdx, param.endIdx, ParseMetaPatternOption, NULL);
		kNode_AddParsedObject(kctx, stmt, name, UPCAST(block));
		returnIdx = beginIdx + 1;
	}
	KReturnUnboxValue(returnIdx);
}

static kParam *kNode_GetParamNULL(KonohaContext *kctx, kNode *stmt, kNameSpace* ns)
{
	kParam *pa = (kParam *)kNode_GetObjectNULL(kctx, stmt, KSymbol_ParamPattern);
	if(pa == NULL || !IS_Param(pa)) {
		kSyntax *syn = kSyntax_(kNode_ns(stmt), KSymbol_ParamPattern);
		TypeNode(kctx, syn, stmt, ns, KClass_void);
		if(kNode_IsError(stmt)) return NULL;
	}
	pa = (kParam *)kNode_GetObjectNULL(kctx, stmt, KSymbol_ParamPattern);
	DBG_ASSERT(IS_Param(pa));
	return pa;
}

static kbool_t SetParamType(KonohaContext *kctx, kNode *stmt, int n, kparamtype_t *p)
{
	kToken *typeToken  = SUGAR kNode_GetToken(kctx, stmt, KSymbol_TypePattern, NULL);
	kNode  *expr = SUGAR kNode_GetNode(kctx, stmt, KSymbol_ExprPattern, NULL);
	DBG_ASSERT(typeToken != NULL);
	DBG_ASSERT(expr != NULL);
	if(kNode_isSymbolTerm(expr)) {
		kToken *tkN = expr->TermToken;
		p[n].name = tkN->symbol;
		p[n].attrTypeId = Token_typeLiteral(typeToken);
		return true;
	}
	return false;
}

static KMETHOD Statement_ParamDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, ns, reqc);
	kToken *returnTypeToken = SUGAR kNode_GetToken(kctx, stmt, KSymbol_TypePattern, NULL); // type
	ktypeattr_t returnType =  (returnTypeToken == NULL) ? KType_void : Token_typeLiteral(returnTypeToken);
	kParam *pa = NULL;
	kNode *params = (kNode *)kNode_GetObjectNULL(kctx, stmt, KSymbol_ParamPattern);
	if(params == NULL) {
		pa = new_kParam(kctx, returnType, 0, NULL);
	}
	else if(IS_Node(params)) {
		size_t i, psize = kNode_GetNodeListSize(kctx, params);
		kparamtype_t *p = ALLOCA(kparamtype_t, psize);
		for(i = 0; i < psize; i++) {
			p[i].attrTypeId = KType_void;
			p[i].name = 0;
			kNode *node = params->NodeList->NodeItems[i];
			if(node->syn->keyword != KSymbol_TypeDeclPattern || !SetParamType(kctx, node, i, p)) {
				KReturn(SUGAR MessageNode(kctx, stmt, NULL, ns, ErrTag, "Argument(%d) No Type declaration", i));
			}
		}
		pa = new_kParam(kctx, returnType, psize, p);
	}
	if(pa != NULL && IS_Param(pa)) {
		KLIB kObjectProto_SetObject(kctx, stmt, KSymbol_ParamPattern, KType_Param, pa);
		KReturn(kNode_Type(kctx, stmt, KNode_Done, KType_void));
	}
	KReturn(SUGAR MessageNode(kctx, stmt, NULL, ns, ErrTag, "expected parameter declaration"));
}

/* MethodDecl */

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

static ksymbol_t kNode_GetMethodName(KonohaContext *kctx, kNode *stmt, kmethodn_t defmn)
{
	kToken *tk = (kToken *)kNode_GetObjectNULL(kctx, stmt, KSymbol_SymbolPattern);
	return (tk == NULL) ? defmn : tk->symbol;
}

static KMETHOD Statement_MethodDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, ns, reqc);
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
	uintptr_t flag      = kNode_ParseFlag(kctx, stmt, MethodDeclFlag, 0);
	ktypeattr_t typeId  = kNode_GetClassId(kctx, stmt, ns, KSymbol_("ClassName"), kObject_typeId(ns));
	kmethodn_t mn       = kNode_GetMethodName(kctx, stmt, MN_new);
	kParam *pa          = kNode_GetParamNULL(kctx, stmt, ns);
	if(KType_Is(Singleton, typeId)) { flag |= kMethod_Static; }
	if(KType_Is(Final, typeId)) { flag |= kMethod_Final; }
	if(pa != NULL) {  // if pa is NULL, error is printed out.
		INIT_GCSTACK();
		kMethodVar *mtd = (kMethodVar *)KLIB new_kMethod(kctx, _GcStack, flag, typeId, mn, NULL);
		KLIB kMethod_SetParam(kctx, mtd, pa->rtype, pa->psize, (kparamtype_t *)pa->paramtypeItems);
		KMakeTrace(trace, sfp);
		if((mtd = kNameSpace_AddMethod(kctx, ns, mtd, trace)) != NULL) {
			kMethod_SetLazyCompilation(kctx, mtd, stmt, ns);
		}
		RESET_GCSTACK();
	}
	KReturn(kNode_Type(kctx, stmt, KNode_Done, KType_void));
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
	DBG_ASSERT(KSymbol_("new") == MN_new);
	kFunc *TermFunc = KSugarFunc(ns, Expression_Term);
	kFunc *OperatorFunc = KSugarFunc(ns, Expression_Operator);
	kFunc *MethodCallFunc = KSugarFunc(ns, TypeCheck_MethodCall);
	kFunc *patternParseFunc = KSugarFunc(ns, Parse_Pattern);
	kSyntaxVar *nullSyntax = (kSyntaxVar *)KNULL(Syntax);
	nullSyntax->precedence_op2 = Precedence_CStyleStatementEnd;
	nullSyntax->precedence_op1 = Precedence_CStyleStatementEnd;
	nullSyntax->ParseFuncNULL = KSugarFunc(ns, Parse_Block);
	KDEFINE_SYNTAX SYNTAX[] = {
		{ PATTERN(Indent), SYNFLAG_CTypeFunc|SYNFLAG_NodeLeftJoinOp2, Precedence_CStyleStatementEnd, Precedence_CStyleStatementEnd, {nullSyntax->ParseFuncNULL}, {SUGARFUNC TypeCheck_Block}},
		{ TOKEN(SEMICOLON), SYNFLAG_CTypeFunc|SYNFLAG_NodeLeftJoinOp2, Precedence_CStyleStatementEnd, Precedence_CStyleStatementEnd, {nullSyntax->ParseFuncNULL}, {SUGARFUNC TypeCheck_Block}},
		{ PATTERN(Symbol),  SYNFLAG_CFunc, 0, 0, {SUGARFUNC PatternMatch_MethodName}, {SUGARFUNC TypeCheck_Symbol},},
		{ PATTERN(Text),    SYNFLAG_CTypeFunc, 0, 0, {TermFunc}, {SUGARFUNC TypeCheck_TextLiteral},},
		{ PATTERN(Number),  SYNFLAG_CTypeFunc, 0, 0, {TermFunc}, {SUGARFUNC TypeCheck_IntLiteral},},
		{ PATTERN(Member),  SYNFLAG_CFunc|SYNFLAG_Suffix, Precedence_CStyleSuffixCall, 0, {SUGARFUNC Expression_Member}, {SUGARFUNC TypeCheck_Getter}},
		{ GROUP(Parenthesis), SYNFLAG_CFunc|SYNFLAG_Suffix, Precedence_CStyleSuffixCall, 0, {SUGARFUNC Expression_Parenthesis}, {SUGARFUNC TypeCheck_FuncStyleCall}}, //KSymbol_ParenthesisGroup
		{ GROUP(Bracket),  SYNFLAG_CParseFunc|SYNFLAG_Suffix|SYNFLAG_TypeSuffix, Precedence_CStyleSuffixCall, 0, {SUGARFUNC Expression_Indexer}, {MethodCallFunc}}, //KSymbol_BracketGroup
		{ GROUP(Brace),  SYNFLAG_CFunc, 0, 0, {SUGARFUNC Expression_Block}, {SUGARFUNC TypeCheck_Block}},   // KSymbol_BraceGroup
		{ PATTERN(Block), SYNFLAG_CFunc, 0, 0, {SUGARFUNC PatternMatch_CStyleBlock}, {SUGARFUNC TypeCheck_Block}, },
		{ PATTERN(Param), SYNFLAG_CFunc, 0, 0, {SUGARFUNC PatternMatch_CStyleParam}, {SUGARFUNC Statement_ParamDecl},},
		{ PATTERN(Token), SYNFLAG_CFunc, 0, 0, {SUGARFUNC PatternMatch_Token}, {NULL}},
//		{ TOKEN(DOT), },
		{ TOKEN(DIV), 0, Precedence_CStyleMUL, 0, {OperatorFunc}, {MethodCallFunc}},
		{ TOKEN(MOD), 0, Precedence_CStyleMUL, 0, {OperatorFunc}, {MethodCallFunc}},
		{ TOKEN(MUL), 0, Precedence_CStyleMUL, 0, {OperatorFunc}, {MethodCallFunc}},
		{ TOKEN(ADD), 0, Precedence_CStyleADD, Precedence_CStylePrefixOperator, {OperatorFunc}, {MethodCallFunc}},
		{ TOKEN(SUB), 0, Precedence_CStyleADD, Precedence_CStylePrefixOperator, {OperatorFunc}, {MethodCallFunc}},
		{ TOKEN(LT),  0, Precedence_CStyleCOMPARE, 0, {OperatorFunc}, {MethodCallFunc}},
		{ TOKEN(LTE), 0, Precedence_CStyleCOMPARE, 0, {OperatorFunc}, {MethodCallFunc}},
		{ TOKEN(GT),  0, Precedence_CStyleCOMPARE, 0, {OperatorFunc}, {MethodCallFunc}},
		{ TOKEN(GTE), 0, Precedence_CStyleCOMPARE, 0, {OperatorFunc}, {MethodCallFunc}},
		{ TOKEN(EQ),  0, Precedence_CStyleEquals, 0, {OperatorFunc}, {MethodCallFunc}},
		{ TOKEN(NEQ), 0, Precedence_CStyleEquals, 0, {OperatorFunc}, {MethodCallFunc}},
		{ TOKEN(LET), SYNFLAG_CTypeFunc|SYNFLAG_NodeLeftJoinOp2, Precedence_CStyleAssign, 0, {OperatorFunc}, {SUGARFUNC TypeCheck_Assign}, },
		{ TOKEN(AND), SYNFLAG_CTypeFunc, Precedence_CStyleAND, 0, {OperatorFunc}, {SUGARFUNC TypeCheck_AndOperator}, },
		{ TOKEN(OR),  SYNFLAG_CTypeFunc, Precedence_CStyleOR,  0, {OperatorFunc}, {SUGARFUNC TypeCheck_OrOperator}, },
		{ TOKEN(NOT), 0, 0, Precedence_CStylePrefixOperator, {OperatorFunc}, {MethodCallFunc}},
		{ TOKEN(COLON), 0, Precedence_CStyleTRINARY, },  // colon
		{ TOKEN(COMMA), SYNFLAG_CFunc, Precedence_CStyleCOMMA, 0, {SUGARFUNC Expression_COMMA}, {NULL}},
//		{ TOKEN(DOLLAR),  /* 0, 0, 0, NULL, Expression_DOLLAR, */ },
		{ TOKEN(true),    SYNFLAG_CTypeFunc, 0, 0, {TermFunc}, {SUGARFUNC TypeCheck_true}, },
		{ TOKEN(false),   SYNFLAG_CTypeFunc, 0, 0, {TermFunc}, {SUGARFUNC TypeCheck_false}, },
		{ PATTERN(Expr),  SYNFLAG_CFunc, 0, 0, {SUGARFUNC PatternMatch_Expression}, {NULL}, },
		{ PATTERN(Type),  SYNFLAG_CFunc, 0, 0, {SUGARFUNC PatternMatch_Type}, {SUGARFUNC TypeCheck_Type}, },
		{ PATTERN(TypeDecl),   SYNFLAG_MetaPattern|SYNFLAG_CFunc, 0, 0, {SUGARFUNC PatternMatch_TypeDecl}, {SUGARFUNC Statement_TypeDecl}},
		{ PATTERN(MethodDecl), SYNFLAG_MetaPattern|SYNFLAG_CFunc, 0, 0, {SUGARFUNC PatternMatch_MethodDecl}, {SUGARFUNC Statement_MethodDecl}},
		{ TOKEN(if),           SYNFLAG_CTypeFunc, 0, Precedence_Statement, {patternParseFunc}, {SUGARFUNC Statement_if}},
		{ TOKEN(else),         SYNFLAG_CTypeFunc, 0, Precedence_Statement, {patternParseFunc}, {SUGARFUNC Statement_else}},
		{ TOKEN(return), SYNFLAG_CTypeFunc|SYNFLAG_NodeBreakExec, 0, Precedence_Statement, {patternParseFunc}, {SUGARFUNC Statement_return} },
		{ TOKEN(new), SYNFLAG_CFunc, 0, Precedence_CStyleSuffixCall, {SUGARFUNC Expression_new}, },
		{ KSymbol_END, },
	};
	kNameSpace_DefineSyntax(kctx, ns, SYNTAX, NULL);
	((kTokenVar *)K_NULLTOKEN)->resolvedSyntaxInfo = kNameSpace_GetSyntax(kctx, ns, KSymbol_SEMICOLON);
	KPARSERM->termParseFunc     = TermFunc;
	KPARSERM->opParseFunc       = OperatorFunc;
	KPARSERM->patternParseFunc  = patternParseFunc;
	KPARSERM->methodTypeFunc    = MethodCallFunc;
	// Syntax Rule
	kSyntax_AddPattern(kctx, kSyntax_(ns, PATTERN(TypeDecl)), "$Type $Expr", 0, NULL);
	kSyntax_AddPattern(kctx, kSyntax_(ns, PATTERN(MethodDecl)), "$Type [ClassName: $Type] [$Symbol] $Param [$Block]", 0, NULL);
	kSyntax_AddPattern(kctx, kSyntax_(ns, TOKEN(if)), "\"if\" \"(\" $Expr \")\" $Block [\"else\" else: $Block]", 0, NULL);
	kSyntax_AddPattern(kctx, kSyntax_(ns, TOKEN(else)), "\"else\" $Block", 0, NULL);
	kSyntax_AddPattern(kctx, kSyntax_(ns, TOKEN(return)), "\"return\" [$Expr]", 0, NULL);
}
