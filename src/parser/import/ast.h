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

typedef enum {
	PatternNoMatch = -1,
} ParsePattern_;

static int CallParseFunc(KonohaContext *kctx, kFunc *fo, kNode *node, ksymbol_t symbol, kArray *tokenList, int beginIdx, int operatorIdx, int endIdx)
{
	BEGIN_UnusedStack(lsfp);
	KUnsafeFieldSet(lsfp[1].asNode, node);
	lsfp[2].intValue = symbol;
	KUnsafeFieldSet(lsfp[3].asArray, tokenList);
	lsfp[4].intValue = beginIdx;
	lsfp[5].intValue = operatorIdx;
	lsfp[6].intValue = endIdx;
	CallSugarMethod(kctx, lsfp, fo, 6, (kObject*)KNULL(Int));
	END_UnusedStack();
	if(kNode_IsError(node)) return endIdx;
	return (int)lsfp[K_RTNIDX].intValue;
}

static int ParseSyntaxNode(KonohaContext *kctx, kSyntax *syn, kNode *node, ksymbol_t symbol, kArray *tokenList, int beginIdx, int opIdx, int endIdx)
{
	int callCount = 0;
	if(opIdx != PatternNoMatch) {
		KFieldSet(node, node->TermToken, tokenList->TokenItems[opIdx]);
		node->syn = syn;/*node->TermToken->resolvedSyntaxInfo*/;
	}
	if(syn->sugarFuncTable[KSugarParseFunc] != NULL) {
		int nextIdx = CallParseFunc(kctx, syn->sugarFuncTable[KSugarParseFunc], node, symbol, tokenList, beginIdx, opIdx, endIdx);
		if(nextIdx != PatternNoMatch) return nextIdx;
		callCount++;
	}
	size_t i;
	kArray *syntaxList = kNameSpace_GetSyntaxList(kctx, kNode_ns(node), syn->keyword);
	for(i = 1; i < kArray_size(syntaxList); i++) { /* ObjectItems[0] == syn */
		kSyntax *syn2 = syntaxList->SyntaxItems[i];
		if(syn2->sugarFuncTable[KSugarParseFunc] != NULL) {
			int nextIdx = CallParseFunc(kctx, syn2->sugarFuncTable[KSugarParseFunc], node, symbol, tokenList, beginIdx, opIdx, endIdx);
			if(nextIdx != PatternNoMatch) return nextIdx;
			callCount++;
		}
	}
	if(opIdx != PatternNoMatch/* && !kNode_IsError(node)*/) {
		const char *emesg = (callCount > 0) ? "syntax error: %s%s" : "undefined: %s%s";
		kToken *tk = tokenList->TokenItems[opIdx];
		SUGAR MessageNode(kctx, node, tk, NULL, ErrTag, emesg, KSymbol_Fmt2(syn->keyword)/*KToken_t(tk)*/);
		return endIdx;
	}
	return PatternNoMatch;
}

static int FindFirstStatementToken(KonohaContext *kctx, kArray *tokenList, int currentIdx, int endIdx)
{
	for(; currentIdx < endIdx; currentIdx++) {
		kToken *tk = tokenList->TokenItems[currentIdx];
		if((tk)->tokenType == TokenType_INDENT || kToken_Is(StatementSeparator, tk)) continue;
		break;
	}
	return currentIdx;
}

static int FindEndOfStatement(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx)
{
	int c, isNoSemiColon = kNameSpace_Is(NoSemiColon, ns);
	for(c = beginIdx; c < endIdx; c++) {
		kToken *tk = tokenList->TokenItems[c];
		if(kToken_Is(StatementSeparator, tk)) return c;
		if(isNoSemiColon && kToken_IsIndent(tk)) {
			return c;
		}
	}
	return endIdx;
}

static int SkipAnnotation(KonohaContext *kctx, kArray *tokenList, int currentIdx, int endIdx)
{
	for(; currentIdx < endIdx; currentIdx++) {
		kToken *tk = tokenList->TokenItems[currentIdx];
		if(kToken_IsIndent(tk)) continue;
		if(KSymbol_IsAnnotation(tk->resolvedSymbol)) {
			if(currentIdx + 1 < endIdx) {
				kToken *nextToken = tokenList->TokenItems[currentIdx+1];
				if(nextToken->resolvedSyntaxInfo != NULL && nextToken->resolvedSyntaxInfo->keyword == KSymbol_ParenthesisGroup) {
					currentIdx++;
				}
			}
			continue;
		}
		break;
	}
	return currentIdx;
}

static void kNode_AddAnnotation(KonohaContext *kctx, kNode *stmt, kArray *tokenList, int beginIdx, int endIdx)
{
	int currentIdx;
	for(currentIdx = beginIdx; currentIdx < endIdx; currentIdx++) {
		kToken *tk = tokenList->TokenItems[currentIdx];
		if(kToken_IsIndent(tk)) {
			continue;
		}
		if(KSymbol_IsAnnotation(tk->resolvedSymbol)) {
			kObject *value = UPCAST(K_TRUE);
			if(currentIdx + 1 < endIdx) {
				kToken *nextToken = tokenList->TokenItems[currentIdx+1];
				if(nextToken->resolvedSyntaxInfo != NULL && nextToken->resolvedSyntaxInfo->keyword == KSymbol_ParenthesisGroup) {
					int start = 0;
					value = (kObject *)SUGAR ParseNewNode(kctx, kNode_ns(stmt), nextToken->GroupTokenList, &start, kArray_size(nextToken->GroupTokenList), ParseExpressionOption, "(");
					currentIdx++;
				}
			}
			KLIB kObjectProto_SetObject(kctx, stmt, tk->resolvedSymbol, kObject_typeId(value), value);
			continue;
		}
		break;
	}
}

static int ParseMetaPattern(KonohaContext *kctx, kNameSpace *ns, kNode *node, kArray *tokenList, int beginIdx, int endIdx)
{
	int i;
	SUGAR dumpTokenArray(kctx, 0, tokenList, beginIdx, endIdx);
	for(i = beginIdx; i < endIdx; i++) {
		kToken *tk = tokenList->TokenItems[i];
		if(tk->resolvedSyntaxInfo->precedence_op2 == Precedence_CStyleStatementEnd) {
			return ParseSyntaxNode(kctx, tk->resolvedSyntaxInfo, node, 0, tokenList, beginIdx, i, endIdx);
		}
	}
	int currentIdx = SkipAnnotation(kctx, tokenList, beginIdx, endIdx);
	if(!(currentIdx < endIdx)) {
		return endIdx;  // empty
	}
	kToken *tk = tokenList->TokenItems[currentIdx];
	kSyntax *syn = tk->resolvedSyntaxInfo;
	if(syn->syntaxPatternListNULL == NULL) {
		kNameSpace *currentNameSpace = ns;
		KFieldSet(node, node->KeyOperatorToken, tk);
		while(currentNameSpace != NULL) {
			kArray *metaPatternList = currentNameSpace->metaPatternList;
			intptr_t i;
			for(i = kArray_size(metaPatternList) - 1; i >=0; i--) {
				kSyntax *patternSyntax = metaPatternList->SyntaxItems[i];
				DBG_ASSERT(IS_Syntax(patternSyntax));
				node->syn = patternSyntax;
				DBG_P(">>>>>>>>>> searching meta i=%d, pattern = %s%s index=%d,%d", i, KSymbol_Fmt2(patternSyntax->keyword), beginIdx, endIdx);
				int nextIdx = ParseSyntaxNode(kctx, patternSyntax, node, 0, tokenList, currentIdx, PatternNoMatch, endIdx);
				//DBG_P(">>>>>>>>>> searching meta pattern = %s%s index=%d,%d,%d", KSymbol_Fmt2(patternToken->resolvedSymbol), beginIdx, nextIdx, endIdx);
				if(nextIdx != PatternNoMatch) {
					if(beginIdx < currentIdx) {
						kNode_AddAnnotation(kctx, node, tokenList, beginIdx, currentIdx);
					}
					return nextIdx;
				}
				if(kNode_IsError(node)) return endIdx;
				node->syn = NULL;
				kNode_Reset(kctx, node);
			}
			currentNameSpace = currentNameSpace->parentNULL;
		}
	}
	KFieldSet(node, node->KeyOperatorToken, K_NULLTOKEN);
	return PatternNoMatch;
}

static int FindOperator(KonohaContext *kctx, kNode *node, kArray *tokenList, int beginIdx, int endIdx)
{
	int isPrePosition = true;
	int operatorIdx = beginIdx, i, precedence = 0;
	for(i = beginIdx; i < endIdx; i++) {
		kToken *tk = tokenList->TokenItems[i];
		kSyntax *syn = tk->resolvedSyntaxInfo;
		if(isPrePosition) {
			if(syn->precedence_op1 > 0) {
				if(precedence < syn->precedence_op1) {
					precedence = syn->precedence_op1;
					operatorIdx = i;
				}
				continue;
			}
			isPrePosition = false;
		}
		else {
			if(syn->precedence_op2 > 0) {
				if(precedence < syn->precedence_op2 || (precedence == syn->precedence_op2 && !(FLAG_is(syn->flag, SYNFLAG_NodeLeftJoinOp2)) )) {
					precedence = syn->precedence_op2;
					operatorIdx = i;
				}
				if(!FLAG_is(syn->flag, SYNFLAG_Suffix)) isPrePosition = true;
			}
		}
	}
	return operatorIdx;
}

static int ParseNode(KonohaContext *kctx, kNode *node, kArray *tokenList, int beginIdx, int endIdx, ParseOption option, const char *hintBeforeText)
{
	if(beginIdx < endIdx) {
		int opIdx = FindOperator(kctx, node, tokenList, beginIdx, endIdx);
		kToken *keyOperator = tokenList->TokenItems[opIdx];
		DBG_P("KeyOperator >>>>>>>> %d<%d<%d, %s", beginIdx, opIdx, endIdx, KToken_t(keyOperator));
		//kNode_Termnize(kctx, node, keyOperator);
		return ParseSyntaxNode(kctx, keyOperator->resolvedSyntaxInfo, node, 0, tokenList, beginIdx, opIdx, endIdx);
	}
	else {
		if(hintBeforeText == NULL) hintBeforeText = "";
		kNode_Message(kctx, node, ErrTag, "expected expression after %s", hintBeforeText);
	}
	return endIdx;
}

static kNode* ParseNewNode(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int *beginIdx, int endIdx, ParseOption option, const char *hintBeforeText)
{
	kNode *node = new_UntypedNode(kctx, OnGcStack, ns);
	//DBG_P("begin,end=(%d,%d)", beginIdx[0], endIdx);
	int nextIdx = PatternNoMatch;
	if(KFlag_Is(int, option, ParseMetaPatternOption)) {
		nextIdx = ParseMetaPattern(kctx, ns, node, tokenList, beginIdx[0], endIdx);
	}
	if(nextIdx == PatternNoMatch) {
		nextIdx = ParseNode(kctx, node, tokenList, beginIdx[0], endIdx, option, hintBeforeText);
	}
	beginIdx[0] = nextIdx;
	KDump(node);
	return node;
}

static kNode *ParseStatementNode(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx)
{
	KTokenSeq source = {ns, tokenList, beginIdx, endIdx};
	KTokenSeq tokens = {ns, KGetParserContext(kctx)->preparedTokenList, 0};
	KTokenSeq_Push(kctx, tokens);
	SUGAR KTokenSeq_Preprocess(kctx, &tokens, NULL, &source, beginIdx);
	DBG_ASSERT(source.SourceConfig.openToken == NULL);
	DBG_ASSERT(source.SourceConfig.stopChar == 0);
	kNode *node = ParseNewNode(kctx, ns, tokens.tokenList, &tokens.beginIdx, tokens.endIdx, ParseMetaPatternOption, NULL);
	KTokenSeq_Pop(kctx, tokens);
	return node;
}

static kNode *AddParamNode(KonohaContext *kctx, kNameSpace *ns, kNode *node, kArray *tokenList, int s, int e, const char *hintBeforeText/* if NULL empty isAllowed */)
{
	int i, start = s;
//	e = kTokenArray_RemoveIndent(kctx, tokenList, s, e);
	for(i = s; i < e; i++) {
		kToken *tk = tokenList->TokenItems[i];
		if(tk->resolvedSyntaxInfo->keyword == KSymbol_COMMA) {
			if(start < i || hintBeforeText != NULL) {
				kNode_AddNode(kctx, node, ParseNewNode(kctx, ns, tokenList, &start, i, ParseExpressionOption, hintBeforeText));
				if(hintBeforeText != NULL) hintBeforeText = ",";
			}
			start = i + 1;
		}
	}
	if(start < i || hintBeforeText != NULL) {
		kNode_AddNode(kctx, node, ParseNewNode(kctx, ns, tokenList, &start, i, ParseExpressionOption, hintBeforeText));
	}
	return node;
}

static int MatchSyntaxPattern(KonohaContext *kctx, kNode *node, KTokenSeq *tokens, KTokenSeq *patterns, kToken **errRuleRef)
{
	int patternIdx = patterns->beginIdx, tokenIdx = tokens->beginIdx;
	kNameSpace *ns = kNode_ns(node);
//	KdumpTokenArray(kctx, patterns->tokenList, patterns->beginIdx, patterns->endIdx);
//	KdumpTokenArray(kctx, tokens->tokenList, tokens->beginIdx, tokens->endIdx);
	for(; patternIdx < patterns->endIdx; patternIdx++) {
		kToken *ruleToken = patterns->tokenList->TokenItems[patternIdx];
		L_ReDo:;
		//tokenIdx = TokenUtils_SkipOnlyIndent(tokens->tokenList, tokenIdx, tokens->endIdx);
		//DBG_P("patternIdx=%d, tokenIdx=%d, tokenEndIdx=%d", patternIdx, tokenIdx, tokens->endIdx);
		//KDump(node);
		if(tokenIdx < tokens->endIdx) {
			kToken *tk = tokens->tokenList->TokenItems[tokenIdx];
			errRuleRef[1] = tk;
			if(KSymbol_IsPattern(ruleToken->resolvedSymbol)) {
				kSyntax *syn = kSyntax_(ns, ruleToken->resolvedSymbol);
				int nextIdx = ParseSyntaxNode(kctx, syn, node, ruleToken->stmtEntryKey, tokens->tokenList, tokenIdx, -1, tokens->endIdx);
				if(nextIdx == PatternNoMatch) {
					if(!kToken_Is(MatchPreviousPattern, ruleToken)) {
						errRuleRef[0] = ruleToken;
						return PatternNoMatch;
					}
					continue;  /* to avoid check same rule */
				}
				tokenIdx = nextIdx ;
			}
			else if(ruleToken->resolvedSymbol == KSymbol_OptionalGroup) {
				KTokenSeq nrule = {ns, ruleToken->GroupTokenList, 0, kArray_size(ruleToken->GroupTokenList)};
				tokens->beginIdx = tokenIdx;
				int nextIdx = MatchSyntaxPattern(kctx, node, tokens, &nrule, errRuleRef);
				errRuleRef[0] = NULL;
				if(kNode_IsError(node)) return tokens->endIdx;
				if(nextIdx != PatternNoMatch) {
					tokenIdx = nextIdx;
				}
			}
			else {
				if(ruleToken->resolvedSymbol != tk->resolvedSymbol) {
					errRuleRef[0] = ruleToken;
					return PatternNoMatch;
				}
				if(ruleToken->resolvedSymbol == KSymbol_ParenthesisGroup || ruleToken->resolvedSymbol == KSymbol_BracketGroup) {
					KTokenSeq nrule = {ns, ruleToken->GroupTokenList, 0, kArray_size(ruleToken->GroupTokenList)};
					KTokenSeq ntokens = {ns, tk->GroupTokenList, 0, kArray_size(tk->GroupTokenList)};
					int next = MatchSyntaxPattern(kctx, node, &ntokens, &nrule, errRuleRef);
					if(next == PatternNoMatch) {
						return PatternNoMatch;
					}
				}
				tokenIdx++;
			}
			if(kToken_Is(MatchPreviousPattern, ruleToken)) {
				goto L_ReDo;
			}
		}
		else { /* tokenIdx < patterns->endIdx */
			while(patternIdx + 1 < patterns->endIdx) {
				if(ruleToken->resolvedSymbol != KSymbol_OptionalGroup && !kToken_Is(MatchPreviousPattern,ruleToken)) {
					errRuleRef[0] = ruleToken;
					errRuleRef[1] = NULL;
					return -1;
				}
				patternIdx++;
				ruleToken = patterns->tokenList->TokenItems[patternIdx];
			}
			return tokenIdx;
		}
	}
	return tokenIdx;
}

static int SelectSyntaxPattern(KonohaContext *kctx, KTokenSeq *patterns, kArray *patternList, int endIdx)
{
	int i;
	for(i = endIdx - 1; i >= 0; i--) {
		kToken *tk = patternList->TokenItems[i];
		if(IS_NULL(tk)) {
			patterns->endIdx = endIdx;
			patterns->beginIdx = i + 1;
			if(KSymbol_IsPattern(patternList->TokenItems[patterns->beginIdx]->resolvedSymbol)) {
				patterns->beginIdx += 1; /* skip headding meta pattern */
			}
			return i - 1;
		}
	}
	return -1;
}

static int ParseSyntaxPattern(KonohaContext *kctx, kNameSpace *ns, kNode *node, kSyntax *stmtSyntax, kArray *tokenList, int beginIdx, int endIdx)
{
	kToken *errRule[2] = {};
	kSyntax *currentSyntax = stmtSyntax;
	DBG_ASSERT(currentSyntax->syntaxPatternListNULL != NULL);
	int patternEndIdx = kArray_size(currentSyntax->syntaxPatternListNULL);
	KTokenSeq tokens = {ns, tokenList, beginIdx, endIdx};
	KTokenSeq nrule  = {ns, currentSyntax->syntaxPatternListNULL, 0, kArray_size(currentSyntax->syntaxPatternListNULL)};
	do {
		patternEndIdx = SelectSyntaxPattern(kctx, &nrule, currentSyntax->syntaxPatternListNULL, patternEndIdx);
		errRule[0] = NULL; errRule[1] = NULL;
		int nextIdx = MatchSyntaxPattern(kctx, node, &tokens, &nrule, errRule);
		if(kNode_IsError(node)) return endIdx;
		if(nextIdx != PatternNoMatch) return nextIdx;
		kNode_Reset(kctx, node/*, ns*/);
	} while(patternEndIdx > 0);
	if(!kNode_IsError(node)) {
		DBG_ASSERT(errRule[0] != NULL);
//		KdumpTokenArray(kctx, tokenList, beginIdx, endIdx);
//		KdumpTokenArray(kctx, stmtSyntax->syntaxPatternListNULL, 0, kArray_size(stmtSyntax->syntaxPatternListNULL));
		if(errRule[1] != NULL) {
			kNode_Message(kctx, node, ErrTag, "%s%s: %s%s is expected before %s", KSymbol_Fmt2(node->syn->keyword), KSymbol_Fmt2(errRule[0]->resolvedSymbol), KToken_t(errRule[1]));
		} else {
			kNode_Message(kctx, node, ErrTag, "%s%s: %s%s is expected", KSymbol_Fmt2(node->syn->keyword), KSymbol_Fmt2(errRule[0]->resolvedSymbol));
		}
		return endIdx;
	}
	return PatternNoMatch;
}

static kNode *kNode_RightJoinNode(KonohaContext *kctx, kNode *stmt, kNode *expr, kArray *tokenList, int c, int e)
{
	if(c < e && expr != K_NULLNODE && !kNode_IsError(stmt)) {
		kToken *tk = tokenList->TokenItems[c];
		if(tk->resolvedSyntaxInfo->keyword == KSymbol_SymbolPattern || tk->resolvedSyntaxInfo->sugarFuncTable[KSugarParseFunc] == NULL) {
			DBG_ASSERT(c >= 1);
			kToken *previousToken = tokenList->TokenItems[c-1];
			const char *white = kToken_Is(BeforeWhiteSpace, previousToken) ? " " : "";
			kNodeToken_Message(kctx, stmt, tk, ErrTag, "undefined syntax: %s%s%s ...", KToken_t(previousToken), white, KToken_t(tk));
			return K_NULLNODE;
		}
		kNodeToken_Message(kctx, stmt, tk, WarnTag, "ignored term: %s...", KToken_t(tk));
	}
	return expr;
}

/* ------------------------------------------------------------------------ */
/* new ast parser */

static int TokenUtils_SkipIndent(kArray *tokenList, int currentIdx, int endIdx)
{
	for(; currentIdx < endIdx; currentIdx++) {
		kToken *tk = tokenList->TokenItems[currentIdx];
		if(kToken_Is(StatementSeparator, tk)) continue;
		if(!kToken_IsIndent(tk)) break;
	}
	return currentIdx;
}

static int ParseTypePattern(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx, KClass **classRef);
static KClass* ParseGenericsType(KonohaContext *kctx, kNameSpace *ns, KClass *baseClass, kArray *tokenList, int beginIdx, int endIdx)
{
	int currentIdx = beginIdx;
	size_t psize = 0;
	kparamtype_t *p = ALLOCA(kparamtype_t, endIdx);
	while(currentIdx < endIdx) {
		KClass *paramClass = NULL;
		currentIdx = ParseTypePattern(kctx, ns, tokenList, currentIdx, endIdx, &paramClass);
		if(paramClass == NULL) {
			return NULL;
		}
		p[psize].attrTypeId = paramClass->typeId;
		psize++;
		if(currentIdx < endIdx && tokenList->TokenItems[currentIdx]->resolvedSyntaxInfo->keyword == KSymbol_COMMA) {
			currentIdx++;
		}
	}
	if(baseClass->baseTypeId == KType_Func) {
		return KLIB KClass_Generics(kctx, baseClass, p[0].attrTypeId, psize-1, p+1);
	}
	else {
		return KLIB KClass_Generics(kctx, baseClass, KType_void, psize, p);
	}
}

static int ParseTypePattern(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx, KClass **classRef)
{
	int nextIdx = -1;
	kToken *tk = tokenList->TokenItems[beginIdx];
	KClass *foundClass = NULL;
	if(tk->resolvedSyntaxInfo->keyword == KSymbol_TypePattern) {
		foundClass = KClass_(tk->resolvedTypeId);
		nextIdx = beginIdx + 1;
	}
	else if(tk->resolvedSyntaxInfo->keyword == KSymbol_SymbolPattern) { // check
		foundClass = KLIB kNameSpace_GetClassByFullName(kctx, ns, kString_text(tk->text), kString_size(tk->text), NULL);
		if(foundClass != NULL) {
			nextIdx = beginIdx + 1;
		}
	}
	if(foundClass != NULL) {
		int isAllowedGenerics = true;
		for(; nextIdx < endIdx; nextIdx++) {
			tk = tokenList->TokenItems[nextIdx];
			if(tk->resolvedSyntaxInfo == NULL || tk->resolvedSyntaxInfo->keyword != KSymbol_BracketGroup) {
				break;
			}
			int sizeofBracketTokens = kArray_size(tk->GroupTokenList);
			if(isAllowedGenerics &&  sizeofBracketTokens > 0) {  // C[T][]
				KClass *foundGenericClass = ParseGenericsType(kctx, ns, foundClass, tk->GroupTokenList, 0, sizeofBracketTokens);
				if(foundGenericClass == NULL) break;
				foundClass = foundGenericClass;
			}
			else {
				if(sizeofBracketTokens > 0) break;   // C[100] is treated as C  and the token [100] is set to nextIdx;
				foundClass = KClass_p0(kctx, KClass_Array, foundClass->typeId);  // C[] => Array[C]
			}
			isAllowedGenerics = false;
		}
	}
	if(classRef != NULL) {
		classRef[0] = foundClass;
		if(foundClass==NULL) nextIdx = -1;
	}
	return nextIdx;
}

// ---------------------------------------------------------------------------
/* macro */

static int KTokenSeq_Preprocess(KonohaContext *kctx, KTokenSeq *tokens, KMacroSet *, KTokenSeq *source, int beginIdx);

static kbool_t KTokenSeq_ExpandMacro(KonohaContext *kctx, KTokenSeq *tokens, ksymbol_t symbol, KMacroSet *macroParam)
{
	while(macroParam->symbol != 0) {
		if(macroParam->symbol == symbol) {
			KTokenSeq paramtokens = {tokens->ns, macroParam->tokenList, macroParam->beginIdx, macroParam->endIdx};
			KTokenSeq_Preprocess(kctx, tokens, NULL, &paramtokens, macroParam->beginIdx);
			return true;
		}
		macroParam++;
	}
	return false;
}

static kArray* new_SubsetArray(KonohaContext *kctx, kArray *gcstack, kArray *a, int beginIdx, int endIdx)
{
	kArray *newa = new_(Array, endIdx - beginIdx, gcstack);
	int i;
	for(i = beginIdx; i < endIdx; i++) {
		KLIB kArray_Add(kctx, newa, a->ObjectItems[i]);
	}
//	DBG_P(">>>>>>>>>> beginIdx=%d,%d", beginIdx, endIdx);
//	SUGAR dumpTokenArray(kctx, 0, newa, 0, kArray_size(newa));
//	DBG_P("macroTokenList=%p", newa);
//	kArray_Set(Debug, ((kArrayVar*)newa), true);
	return newa;
}

static kTokenVar* kToken_ExpandGroupMacro(KonohaContext *kctx, kTokenVar *tk, kNameSpace *ns, KMacroSet *macroParam, kArray *gcstack)
{
	KTokenSeq source = {ns, tk->GroupTokenList, 0, kArray_size(tk->GroupTokenList)};
	if(source.endIdx > 0) {
		int isChanged = true;
		KTokenSeq group = {ns, KGetParserContext(kctx)->preparedTokenList};
		KTokenSeq_Push(kctx, group);
		KTokenSeq_Preprocess(kctx, &group, macroParam, &source, source.beginIdx);
		if(group.endIdx - group.beginIdx == source.endIdx) {
			int i;
			isChanged = false;
			for(i = 0; i < source.endIdx; i++) {
				if(source.tokenList->TokenItems[i] != group.tokenList->TokenItems[group.beginIdx+i]) {
					isChanged = true;
				}
			}
		}
		if(isChanged) {
			kTokenVar *groupToken = new_(TokenVar, tk->resolvedSymbol, gcstack);
			KFieldSet(groupToken, groupToken->GroupTokenList, new_SubsetArray(kctx, gcstack, group.tokenList, group.beginIdx, group.endIdx));
			groupToken->resolvedSyntaxInfo = tk->resolvedSyntaxInfo;
			groupToken->uline = tk->uline;
			tk = groupToken;
		}
		KTokenSeq_Pop(kctx, group);
	}
	return tk;
}

static kbool_t KTokenSeq_ApplyMacro(KonohaContext *kctx, KTokenSeq *tokens, kArray *macroTokenList, int beginIdx, int endIdx, size_t paramsize, KMacroSet *macroParam)
{
	KTokenSeq macro = {tokens->ns, macroTokenList, beginIdx + paramsize, endIdx};
	KdumpTokenArray(kctx, macro.tokenList, macro.beginIdx, macro.endIdx);
	SUGAR dumpTokenArray(kctx, 0, macroTokenList, 0, kArray_size(macroTokenList));
	DBG_P("macroTokenList=%p", macroTokenList);
	int dstart = kArray_size(tokens->tokenList);
	KTokenSeq_Preprocess(kctx, tokens, macroParam, &macro, beginIdx + paramsize);
	DBG_P("<<<<<<<<<<< dstart=%d, tokens->begin,end=%d, %d", dstart, tokens->beginIdx, tokens->endIdx);
	KdumpTokenArray(kctx, tokens->tokenList, dstart, tokens->endIdx);
	return true;
}

static void KTokenSeq_ApplyMacroGroup(KonohaContext *kctx, KTokenSeq *tokens, kArray *macroTokenList, int paramsize, kToken *groupToken)
{
	int i;
	KMacroSet* mp = ALLOCA(KMacroSet, paramsize+1);
	DBG_ASSERT(paramsize < kArray_size(macroTokenList));
	for(i = 0; i < paramsize; i++) {
		mp[i].symbol = macroTokenList->TokenItems[i]->resolvedSymbol;
		mp[i].tokenList = groupToken->GroupTokenList;
	}
	mp[paramsize].symbol = 0; /* sentinel */

	int p = 0, start = 0;
	for(i = 0; i < kArray_size(groupToken->GroupTokenList); i++) {
		kToken *tk = groupToken->GroupTokenList->TokenItems[i];
		if(tk->resolvedSymbol == KSymbol_COMMA/*tk->hintChar == ','*/) {
			mp[p].beginIdx = start;
			mp[p].endIdx = i;
			p++;
			start = i + 1;
		}
	}
	DBG_ASSERT(p < paramsize);
	mp[p].beginIdx = start;
	mp[p].endIdx = kArray_size(groupToken->GroupTokenList);
	KTokenSeq_ApplyMacro(kctx, tokens, macroTokenList, 0, kArray_size(macroTokenList), paramsize, mp);
}

static kbool_t kNameSpace_SetMacroData(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, int paramsize, const char *data, int optionMacro)
{
	kSyntaxVar *syn = (kSyntaxVar *)SUGAR kNameSpace_GetSyntax(kctx, ns, keyword);
	if(IS_NOTNULL(syn) && syn->macroDataNULL == NULL) {
		KTokenSeq source = {ns, KGetParserContext(kctx)->preparedTokenList};
		KTokenSeq_Push(kctx, source);
		KTokenSeq_Tokenize(kctx, &source, data, 0);
		{
			KTokenSeq tokens = {source.ns, source.tokenList};
			KTokenSeq_Push(kctx, tokens);
			tokens.TargetPolicy.ExpandingBraceGroup = true;
			KTokenSeq_Preprocess(kctx, &tokens, NULL, &source, source.beginIdx);
			syn->macroParamSize = paramsize;
			syn->macroDataNULL =  new_SubsetArray(kctx, ns->NameSpaceConstList, tokens.tokenList, tokens.beginIdx, tokens.endIdx);
			if(optionMacro) syn->flag |= SYNFLAG_Macro;
			KTokenSeq_Pop(kctx, tokens);
		}
		KTokenSeq_Pop(kctx, source);
		return true;
	}
	return false;
}

/* ------------------------------------------------------------------------ */

static int KTokenSeq_AddGroup(KonohaContext *kctx, KTokenSeq *tokens, KMacroSet *macro, KTokenSeq *source, int currentIdx, kToken *openToken)
{
	ksymbol_t AST_type = openToken->hintChar == '(' ?  KSymbol_ParenthesisGroup : KSymbol_BracketGroup;
	int closech = (AST_type == KSymbol_ParenthesisGroup) ? ')': ']';
	kTokenVar *astToken = new_(TokenVar, AST_type, tokens->tokenList);
	astToken->resolvedSyntaxInfo = kSyntax_(tokens->ns, AST_type);
	KFieldSet(astToken, astToken->GroupTokenList, new_(TokenArray, 0, OnField));
	astToken->uline = openToken->uline;
	kToken_SetHintChar(astToken, openToken->hintChar, closech);
	{
		KTokenSeq nested = {source->ns, astToken->GroupTokenList};
		kToken *pushOpenToken = source->SourceConfig.openToken;
		int pushStopChar = source->SourceConfig.stopChar;
		source->SourceConfig.openToken = openToken;
		source->SourceConfig.stopChar = closech;
		nested.TargetPolicy.RemovingIndent = true;
		int returnIdx = KTokenSeq_Preprocess(kctx, &nested, macro, source, currentIdx);
		source->SourceConfig.openToken = pushOpenToken;
		source->SourceConfig.stopChar = pushStopChar;
		return returnIdx;
	}
}

static kTokenVar* kToken_ToBraceGroup(KonohaContext *kctx, kTokenVar *tk, kNameSpace *ns, KMacroSet *macroSet)
{
	KTokenSeq source = {ns, KGetParserContext(kctx)->preparedTokenList};
	KTokenSeq_Push(kctx, source);
	KdumpToken(kctx, tk);
	KTokenSeq_Tokenize(kctx, &source, kString_text(tk->text), tk->uline);
	KFieldSet(tk, tk->GroupTokenList, new_(TokenArray, 0, OnField));
	tk->resolvedSyntaxInfo = kSyntax_(ns, KSymbol_BraceGroup);
	KTokenSeq tokens = {ns, tk->GroupTokenList, 0};
	KTokenSeq_Preprocess(kctx, &tokens, macroSet, &source, source.beginIdx);
	KTokenSeq_Pop(kctx, source);
	return tk;
}

static int TokenUtils_Count(kArray *tokenList, int beginIdx, int endIdx, ksymbol_t keyword)
{
	int i, count = 0;
	for(i = beginIdx; i < endIdx; i++) {
		kToken *tk = tokenList->TokenItems[i];
		if(tk->resolvedSyntaxInfo->keyword == keyword) count++;
	}
	return count;
}

static kToken* new_CommaToken(KonohaContext *kctx, kArray *gcstack)
{
	kTokenVar *tk = new_(TokenVar, KSymbol_COMMA, gcstack);
	//kToken_SetHintChar(tk, 0, ',');
	return tk;
}

static int KTokenSeq_ApplyMacroSyntax(KonohaContext *kctx, KTokenSeq *tokens, kSyntax *syn, KMacroSet *macroParam, KTokenSeq *source, int currentIdx)
{
	KTokenSeq dummy = {tokens->ns, kctx->stack->gcStack};
	KTokenSeq_Push(kctx, dummy);
	int nextIdx = TokenUtils_SkipIndent(source->tokenList, currentIdx+1, source->endIdx);
	kbool_t isApplied = false;
	if(nextIdx < source->endIdx) {
		kToken *groupToken = source->tokenList->TokenItems[nextIdx];
		if(groupToken->hintChar == '(') {
			nextIdx = KTokenSeq_AddGroup(kctx, &dummy, macroParam, source, nextIdx+1, groupToken);
			if(source->SourceConfig.foundErrorToken != NULL) {
				return source->endIdx;
			}
			groupToken = dummy.tokenList->TokenItems[dummy.beginIdx];
		}
		if(groupToken->resolvedSyntaxInfo != NULL && groupToken->resolvedSyntaxInfo->keyword == KSymbol_ParenthesisGroup) {
			int count = TokenUtils_Count(groupToken->GroupTokenList, 0, kArray_size(groupToken->GroupTokenList), KSymbol_COMMA);
			if(syn->macroParamSize == count + 2) {
				nextIdx = TokenUtils_SkipIndent(source->tokenList, nextIdx+1, source->endIdx);
				if(nextIdx < source->endIdx) {
					kTokenVar *tk = source->tokenList->TokenVarItems[nextIdx];
					if(tk->tokenType == TokenType_LazyBlock) {
						tk = kToken_ToBraceGroup(kctx, tk, tokens->ns, macroParam);
						new_CommaToken(kctx, groupToken->GroupTokenList);
						KLIB kArray_Add(kctx, groupToken->GroupTokenList, tk);
						count++;
					}
				}
			}
			if(syn->macroParamSize == count + 1) {
				KTokenSeq_ApplyMacroGroup(kctx, tokens, syn->macroDataNULL, syn->macroParamSize, groupToken);
				isApplied = true;
			}
		}
	}
	if(!isApplied) {
		kTokenVar *tk = source->tokenList->TokenVarItems[currentIdx];
		kToken_ToError(kctx, tk, ErrTag, "macro %s%s takes %d parameter(s)", KSymbol_Fmt2(syn->keyword), (int)syn->macroParamSize);
		source->SourceConfig.foundErrorToken = tk;
		nextIdx = source->endIdx;
	}
	KTokenSeq_Pop(kctx, dummy);
	return nextIdx;
}

static int KTokenSeq_Preprocess(KonohaContext *kctx, KTokenSeq *tokens, KMacroSet *macroParam, KTokenSeq *source, int beginIdx)
{
	int currentIdx = beginIdx;
	if(tokens->TargetPolicy.syntaxSymbolPattern == NULL) {
		tokens->TargetPolicy.syntaxSymbolPattern = kSyntax_(tokens->ns, KSymbol_SymbolPattern);
	}
	for(; currentIdx < source->endIdx; currentIdx++) {
		kTokenVar *tk = source->tokenList->TokenVarItems[currentIdx];
		/* filter */
		if(tk->tokenType == TokenType_INDENT && tokens->TargetPolicy.RemovingIndent) {
			continue;  /* filtering indent; */
		}
		if(tk->tokenType == TokenType_LazyBlock && (tokens->TargetPolicy.ExpandingBraceGroup || macroParam != NULL)) {
			tk = kToken_ToBraceGroup(kctx, tk, tokens->ns, macroParam);
			DBG_ASSERT(tk->resolvedSyntaxInfo != NULL);
			KLIB kArray_Add(kctx, tokens->tokenList, tk);
			continue;
		}
		if(macroParam != NULL && tk->resolvedSyntaxInfo != NULL) {
			ksymbol_t keyword = tk->resolvedSyntaxInfo->keyword;
			if(keyword == KSymbol_SymbolPattern && KTokenSeq_ExpandMacro(kctx, tokens, tk->resolvedSymbol, macroParam)) {
				continue;
			}
			if(keyword == KSymbol_ParenthesisGroup || keyword == KSymbol_BracketGroup || keyword == KSymbol_BraceGroup) {
				tk = kToken_ExpandGroupMacro(kctx, tk, tokens->ns, macroParam, OnGcStack);
			}
			if(keyword == KSymbol_BlockPattern) {
				tk = kToken_ToBraceGroup(kctx, tk, tokens->ns, macroParam);
			}
		}
		if(tk->resolvedSyntaxInfo == NULL) {
			if(source->SourceConfig.openToken != NULL && source->SourceConfig.stopChar == tk->hintChar) {
				return currentIdx;
			}
			if(tk->hintChar == '(' || tk->hintChar == '[') {
				currentIdx = KTokenSeq_AddGroup(kctx, tokens, macroParam, source, currentIdx+1, tk);
				if(source->SourceConfig.foundErrorToken != NULL) return source->endIdx;
				continue; // already added
			}
			if(tk->tokenType == TokenType_ERR) {
				source->SourceConfig.foundErrorToken = tk;
				return source->endIdx;  // resolved no more
			}
			if(tk->tokenType == TokenType_SYMBOL) {
				const char *t = kString_text(tk->text);
				ksymbol_t symbol = KAsciiSymbol(t, kString_size(tk->text), KSymbol_NewId);
				if(macroParam != NULL && KTokenSeq_ExpandMacro(kctx, tokens, symbol, macroParam)) {
					continue;
				}
				kSyntax *syn = kSyntax_(source->ns, symbol);
				if(syn != NULL && FLAG_is(syn->flag, SYNFLAG_Macro)) {
					if(syn->macroParamSize == 0) {
						KTokenSeq_ApplyMacro(kctx, tokens, syn->macroDataNULL, 0, kArray_size(syn->macroDataNULL), 0, NULL);
					}
					else {
						currentIdx = KTokenSeq_ApplyMacroSyntax(kctx, tokens, syn, macroParam, source, currentIdx);
					}
					continue;
				}
				tk->resolvedSymbol = symbol;
				tk->resolvedSyntaxInfo = (syn != NULL) ? syn : tokens->TargetPolicy.syntaxSymbolPattern;
			}
			else {
				tk->resolvedSyntaxInfo = kSyntax_(tokens->ns, tk->tokenType);
				if(tk->resolvedSyntaxInfo == NULL) {
					kToken_ToError(kctx, tk, ErrTag, "undefined pattern: %s%s", KSymbol_Fmt2(tk->tokenType));
					source->SourceConfig.foundErrorToken = tk;
					goto RETURN_ERROR;
				}
				if(tk->tokenType == KSymbol_MemberPattern) {
					const char *t = kString_text(tk->text);
					tk->resolvedSymbol = KAsciiSymbol(t, kString_size(tk->text), KSymbol_NewId);
				}
			}
		}
		DBG_ASSERT(tk->resolvedSyntaxInfo != NULL);
		KLIB kArray_Add(kctx, tokens->tokenList, tk);
	}
	if(source->SourceConfig.openToken != NULL) {
		char buf[2] = {source->SourceConfig.stopChar, 0};
		ERROR_UnclosedToken(kctx, (kTokenVar *)source->SourceConfig.openToken, (const char *)buf);
		source->SourceConfig.foundErrorToken = source->SourceConfig.openToken;
	}
	RETURN_ERROR:;
	KTokenSeq_End(kctx, tokens);
//	DBG_P(">>>>source");
//	SUGAR dumpTokenArray(kctx, 0, source->tokenList, beginIdx, source->endIdx);
//
//	DBG_P(">>>>tokens");
//	SUGAR dumpTokenArray(kctx, 0, tokens->tokenList, tokens->beginIdx, tokens->endIdx);
	return source->endIdx;
}

//static int AddGroup(KonohaContext *kctx, kToken *openToken, KTokenSeq *tokens, KMacroSet *macro, KTokenSeq *source, int currentIdx)
//{
//	kTokenVar *groupToken = new_(TokenVar, openToken->tokenType, tokens->tokenList);
//	groupToken->resolvedSyntaxInfo = kSyntax_(tokens->ns, openToken->tokenType);
//
//	//KFieldSet(groupToken, groupToken->GroupTokenList, new_(TokenArray, 0, OnField));
//	groupToken->uline = openToken->uline;
//
//	//
////	ksymbol_t AST_type = openToken->hintChar == '(' ?  KSymbol_ParenthesisGroup : KSymbol_BracketGroup;
////	int closech = (AST_type == KSymbol_ParenthesisGroup) ? ')': ']';
////	kToken_SetHintChar(astToken, openToken->hintChar, closech);
//	{
//		KTokenSeq nested = {source->ns, groupToken->GroupTokenList};
//		kToken *pushOpenToken = source->SourceConfig.openToken;
//		source->SourceConfig.openToken = openToken;
//		nested.TargetPolicy.RemovingIndent = true;
//		int returnIdx = Preprocess(kctx, &nested, macro, source, currentIdx);
//		source->SourceConfig.openToken = pushOpenToken;
//		return returnIdx;
//	}
//}
//
//static int Preprocess(KonohaContext *kctx, KTokenSeq *tokens, KMacroSet *macroParam, KTokenSeq *source, int beginIdx)
//{
//	int currentIdx = beginIdx;
////	if(tokens->TargetPolicy.syntaxSymbolPattern == NULL) {
////		tokens->TargetPolicy.syntaxSymbolPattern = kSyntax_(tokens->ns, KSymbol_SymbolPattern);
////	}
//	kToken *openToken = source->SourceConfig.openToken;
//	int hasMacro = false;
//	for(; currentIdx < source->endIdx; currentIdx++) {
//		kTokenVar *tk = source->tokenList->TokenVarItems[currentIdx];
//		if(tk->tokenType == TokenType_ERR) {
//			source->SourceConfig.foundErrorToken = tk;
//			return source->endIdx;  // stop
//		}
//		/* filter */
//		if(tk->tokenType == TokenType_INDENT && tokens->TargetPolicy.RemovingIndent) {
//			continue;  /* filtering indent; */
//		}
//		if(kToken_Is(OpenGroup, tk)) {
//			currentIdx = AddGroup(kctx, tk, tokens, macroParam, source, currentIdx+1);
//			source->SourceConfig.openToken = openToken;
//			if(source->SourceConfig.foundErrorToken != NULL) return source->endIdx;
//			continue; // already added
//		}
//		if(kToken_Is(CloseGroup, tk)) {
//			if(openToken != NULL && openToken->tokenType == tk->tokenType) {
//				return currentIdx;
//			}
//		}
//		if(tk->resolvedSyntaxInfo == NULL) {
//			if(tk->tokenType == TokenType_SYMBOL) {
//				const char *t = kString_text(tk->text);
//				ksymbol_t symbol = KAsciiSymbol(t, kString_size(tk->text), KSymbol_NewId);
//				if(macroParam != NULL && KTokenSeq_ExpandMacro(kctx, tokens, symbol, macroParam)) {
//					continue;
//				}
//				kSyntax *syn = kSyntax_(source->ns, symbol);
//				if(syn != NULL && FLAG_is(syn->flag, SYNFLAG_Macro)) {
//					if(syn->macroParamSize == 0) {
//						KTokenSeq_ApplyMacro(kctx, tokens, syn->macroDataNULL, 0, kArray_size(syn->macroDataNULL), 0, NULL);
//					}
//					else {
//						currentIdx = KTokenSeq_ApplyMacroSyntax(kctx, tokens, syn, macroParam, source, currentIdx);
//					}
//					continue;
//				}
//				tk->resolvedSymbol = symbol;
//				tk->resolvedSyntaxInfo = (syn != NULL) ? syn : tokens->TargetPolicy.syntaxSymbolPattern;
//			}
//			else {
//				tk->resolvedSyntaxInfo = kSyntax_(tokens->ns, tk->tokenType);
//				if(tk->resolvedSyntaxInfo == NULL) {
//					kToken_ToError(kctx, tk, ErrTag, "undefined pattern: %s%s", KSymbol_Fmt2(tk->tokenType));
//					source->SourceConfig.foundErrorToken = tk;
//					goto RETURN_ERROR;
//				}
//				if(tk->tokenType == KSymbol_MemberPattern) {
//					const char *t = kString_text(tk->text);
//					tk->resolvedSymbol = KAsciiSymbol(t, kString_size(tk->text), KSymbol_NewId);
//				}
//			}
//		}
//
//
//		if(tk->tokenType == TokenType_CODE && (tokens->TargetPolicy.ExpandingBraceGroup || macroParam != NULL)) {
//			tk = kToken_ToBraceGroup(kctx, tk, tokens->ns, macroParam);
//			KLIB kArray_Add(kctx, tokens->tokenList, tk);
//			continue;
//		}
//		if(macroParam != NULL && tk->resolvedSyntaxInfo != NULL) {
//			ksymbol_t keyword = tk->resolvedSyntaxInfo->keyword;
//			if(keyword == KSymbol_SymbolPattern && KTokenSeq_ExpandMacro(kctx, tokens, tk->resolvedSymbol, macroParam)) {
//				continue;
//			}
//			if(keyword == KSymbol_ParenthesisGroup || keyword == KSymbol_BracketGroup || keyword == KSymbol_BraceGroup) {
//				tk = kToken_ExpandGroupMacro(kctx, tk, tokens->ns, macroParam, OnGcStack);
//			}
//			if(keyword == TokenType_CODE) {
//				tk = kToken_ToBraceGroup(kctx, tk, tokens->ns, macroParam);
//			}
//		}
//		if(tk->resolvedSyntaxInfo == NULL) {
//			if(tk->tokenType == TokenType_SYMBOL) {
//				const char *t = kString_text(tk->text);
//				ksymbol_t symbol = KAsciiSymbol(t, kString_size(tk->text), KSymbol_NewId);
//				if(macroParam != NULL && KTokenSeq_ExpandMacro(kctx, tokens, symbol, macroParam)) {
//					continue;
//				}
//				kSyntax *syn = kSyntax_(source->ns, symbol);
//				if(syn != NULL && FLAG_is(syn->flag, SYNFLAG_Macro)) {
//					if(syn->macroParamSize == 0) {
//						KTokenSeq_ApplyMacro(kctx, tokens, syn->macroDataNULL, 0, kArray_size(syn->macroDataNULL), 0, NULL);
//					}
//					else {
//						currentIdx = KTokenSeq_ApplyMacroSyntax(kctx, tokens, syn, macroParam, source, currentIdx);
//					}
//					continue;
//				}
//				tk->resolvedSymbol = symbol;
//				tk->resolvedSyntaxInfo = (syn != NULL) ? syn : tokens->TargetPolicy.syntaxSymbolPattern;
//			}
//			else {
//				tk->resolvedSyntaxInfo = kSyntax_(tokens->ns, tk->tokenType);
//				if(tk->resolvedSyntaxInfo == NULL) {
//					kToken_ToError(kctx, tk, ErrTag, "undefined pattern: %s%s", KSymbol_Fmt2(tk->tokenType));
//					source->SourceConfig.foundErrorToken = tk;
//					goto RETURN_ERROR;
//				}
//				if(tk->tokenType == KSymbol_MemberPattern) {
//					const char *t = kString_text(tk->text);
//					tk->resolvedSymbol = KAsciiSymbol(t, kString_size(tk->text), KSymbol_NewId);
//				}
//			}
//		}
//		KLIB kArray_Add(kctx, tokens->tokenList, tk);
//	}
//	if(source->SourceConfig.openToken != NULL) {
//		char buf[2] = {source->SourceConfig.stopChar, 0};
//		ERROR_UnclosedToken(kctx, (kTokenVar *)source->SourceConfig.openToken, (const char *)buf);
//		source->SourceConfig.foundErrorToken = source->SourceConfig.openToken;
//	}
//	RETURN_ERROR:;
//	KTokenSeq_End(kctx, tokens);
//	return source->endIdx;
//}

// ---------------------------------------------------------------------------
/* parsing syntax pattern */

#define kTokenPattern_TopChar(tk) ((IS_String(tk->text) && kString_size((tk)->text) == 1) ? kString_text((tk)->text)[0] : 0)

static int KTokenSeq_FindCloseCharAsTopChar(KonohaContext *kctx, KTokenSeq *tokens, int beginIdx, int closech)
{
	int i;
	for(i = beginIdx; i < tokens->endIdx; i++) {
		kToken *tk = tokens->tokenList->TokenItems[i];
		if(kTokenPattern_TopChar(tk) == closech) return i;
	}
	return tokens->endIdx;
}

static kbool_t kArray_AddSyntaxPattern(KonohaContext *kctx, kArray *patternList, KTokenSeq *patterns);

static int KTokenSeq_NestedSyntaxPattern(KonohaContext *kctx, KTokenSeq *tokens, int currentIdx, ksymbol_t KSymbol_AST, int closech)
{
	kTokenVar *tk = tokens->tokenList->TokenVarItems[currentIdx];
	int ne = KTokenSeq_FindCloseCharAsTopChar(kctx, tokens, currentIdx+1, closech);
	tk->resolvedSymbol = KSymbol_AST;
	tk->resolvedSyntaxInfo = kSyntax_(tokens->ns, KSymbol_AST);
	KFieldSet(tk, tk->GroupTokenList, new_(TokenArray, 0, OnField));
	KTokenSeq nestedSourceRange = {tokens->ns, tokens->tokenList, currentIdx+1, ne};
	return kArray_AddSyntaxPattern(kctx, tk->GroupTokenList, &nestedSourceRange) ? ne : tokens->endIdx;
}

static kbool_t kArray_AddSyntaxPattern(KonohaContext *kctx, kArray *patternList, KTokenSeq *patterns)
{
	int i;
	ksymbol_t stmtEntryKey = 0;
	kTokenVar *tk, *prevToken = NULL;
	for(i = patterns->beginIdx; i < patterns->endIdx; i++, prevToken = tk) {
		tk = patterns->tokenList->TokenVarItems[i];
		DBG_ASSERT(!kToken_IsIndent(tk));
		DBG_ASSERT(tk->resolvedSyntaxInfo != NULL);
		if(tk->resolvedSyntaxInfo->keyword == KSymbol_TextPattern) {
			int topch = kTokenPattern_TopChar(tk);
			KLIB kArray_Add(kctx, patternList, tk);
			if(topch == '(') {
				i = KTokenSeq_NestedSyntaxPattern(kctx, patterns, i, KSymbol_ParenthesisGroup, ')');
			}
			else if(topch == '[') {
				i = KTokenSeq_NestedSyntaxPattern(kctx, patterns, i, KSymbol_BracketGroup, ']');
			}
			else {
				tk->resolvedSymbol = KAsciiSymbol(kString_text(tk->text), kString_size(tk->text), KSymbol_NewId);
			}
			continue;
		}
		if(tk->resolvedSyntaxInfo->keyword == KSymbol_BracketGroup) {
			KTokenSeq nestedSourceRange = {patterns->ns, tk->GroupTokenList, 0, kArray_size(tk->GroupTokenList)};
			tk->resolvedSymbol = KSymbol_OptionalGroup;
			PUSH_GCSTACK2(tk->GroupTokenList);  // avoid gc
			KFieldSet(tk, tk->GroupTokenList, new_(TokenArray, 0, OnField));
			kArray_AddSyntaxPattern(kctx, tk->GroupTokenList, &nestedSourceRange);
			KLIB kArray_Add(kctx, patternList, tk);
			continue;
		}
		if(tk->resolvedSymbol == KSymbol_DOLLAR/*tk->hintChar == '$'*/ && i+1 < patterns->endIdx) {  // $PatternName
			tk = patterns->tokenList->TokenVarItems[++i];
			if(IS_String(tk->text)) {
				tk->resolvedSymbol = KAsciiSymbol(kString_text(tk->text), kString_size(tk->text), KSymbol_NewRaw) | KSymbolAttr_Pattern;
				if(stmtEntryKey == 0) stmtEntryKey = tk->resolvedSymbol;
				tk->stmtEntryKey = stmtEntryKey;
				tk->resolvedSyntaxInfo = kSyntax_(patterns->ns, tk->resolvedSymbol/*KSymbol_SymbolPattern*/);
				stmtEntryKey = 0;
				KLIB kArray_Add(kctx, patternList, tk);
				continue;
			}
		}
		if(i + 1 < patterns->endIdx && patterns->tokenList->TokenItems[i+1]->resolvedSymbol == KSymbol_COLON && IS_String(tk->text)) {
			stmtEntryKey = KAsciiSymbol(kString_text(tk->text), kString_size(tk->text), KSymbol_NewRaw);
			i++;
			continue;
		}
		if(tk->resolvedSymbol == KSymbol_MUL/*tk->hintChar == '*'*/ && prevToken != NULL) {
			kToken_Set(MatchPreviousPattern, prevToken, true);
			tk = NULL;
			continue;
		}
		kToken_ToError(kctx, tk, ErrTag, "illegal syntax pattern: %s", KToken_t(tk));
		return false;
	}
	return true;
}

static void kNameSpace_AddSyntaxPattern(KonohaContext *kctx, kNameSpace *ns, ksymbol_t kw, const char *ruleSource, kfileline_t uline, KTraceInfo *trace)
{
	kSyntaxVar *syn = (kSyntaxVar*)kNameSpace_GetSyntax(kctx, ns, kw);
	DBG_ASSERT(IS_NOTNULL(syn));
	if(syn->syntaxPatternListNULL == NULL) {
		syn->syntaxPatternListNULL = new_(TokenArray, 0, ns->NameSpaceConstList);
	}
	KTokenSeq source = {ns, KGetParserContext(kctx)->preparedTokenList};
	KTokenSeq_Push(kctx, source);
	KTokenSeq_Tokenize(kctx, &source, ruleSource, uline);
	KTokenSeq patterns = {ns, source.tokenList, source.endIdx};
	patterns.TargetPolicy.RemovingIndent = true;
	KTokenSeq_Preprocess(kctx, &patterns, NULL, &source, source.beginIdx);
//	KLIB kArray_Add(kctx, syn->syntaxPatternListNULL, K_NULLTOKEN);  // delim
//	size_t firstPatternIdx = kArray_size(syn->syntaxPatternListNULL);
	kArray_AddSyntaxPattern(kctx, syn->syntaxPatternListNULL, &patterns);
//	if(firstPatternIdx < kArray_size(syn->syntaxPatternListNULL)) {
//		kToken *firstPattern = syn->syntaxPatternListNULL->TokenItems[firstPatternIdx];
//		//DBG_P(">>>>>> firstPattern=%d", kToken_IsFirstPattern(firstPattern));
//		if(kToken_IsFirstPattern(firstPattern)) {
//			kNameSpace_AppendArrayRef(kctx, ns, &((kNameSpaceVar *)ns)->metaPatternListNULL, UPCAST(firstPattern));
//		}
//	}
	KTokenSeq_Pop(kctx, source);
	//KdumpTokenArray(kctx, patternList, 0, kArray_size(patternList));
}
