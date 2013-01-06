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

static int ParseSyntaxPattern(KonohaContext *kctx, kNameSpace *ns, kNode *node, kSyntax *stmtSyntax, kArray *tokenList, int beginIdx, int endIdx)
{
	kToken *errRule[2] = {NULL, NULL};
	DBG_ASSERT(stmtSyntax->syntaxPatternListNULL != NULL);
	KTokenSeq tokens = {ns, tokenList, beginIdx, endIdx};
	KTokenSeq nrule  = {ns, stmtSyntax->syntaxPatternListNULL, 0, kArray_size(stmtSyntax->syntaxPatternListNULL)};
	int nextIdx = MatchSyntaxPattern(kctx, node, &tokens, &nrule, errRule);
	if(nextIdx != PatternNoMatch) return nextIdx;
	if(kNode_IsError(node)) return endIdx;
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

static void PreprocessSyntaxPattern(KonohaContext *kctx, kTokenVar *tk, void *thunk)
{
	if(tk->tokenType == TokenType_Indent) {
		tk->tokenType = TokenType_Skip;
		return;
	}
	if(tk->resolvedSyntaxInfo->keyword == KSymbol_BracketGroup) {
		tk->tokenType = KSymbol_OptionalGroup;
		tk->resolvedSyntaxInfo = KNULL(Syntax); // no defined syntax
		return;
	}
	if(tk->tokenType == TokenType_Text) {
		const char *topchar = kString_text(tk->text);
		size_t len = kString_size(tk->text);
		tk->symbol = KAsciiSymbol(topchar, kString_size(tk->text), KSymbol_NewId);
		if(len == 1) {
			if(topchar[0] == '(' || topchar[0] == '[') {
				kToken_Set(OpenGroup, tk, true);
			}
			if(topchar[0] == ')' || topchar[0] == ']') {
				kToken_Set(CloseGroup, tk, true);
			}
			if(topchar[0] == '(' || topchar[0] == ')') {
				tk->tokenType = KSymbol_ParenthesisGroup;
			}
			if(topchar[0] == '[' || topchar[0] == ']') {
				tk->tokenType = KSymbol_BracketGroup;
			}
		}
	}
	tk->stmtEntryKey = tk->symbol;
}

static void PreprocessSyntaxPattern2(KonohaContext *kctx, kTokenVar *tk, kTokenVar *tk2, void *thunk)
{
	if(tk->symbol == KSymbol_DOLLAR) {  // $PatternName
		tk2->symbol = tk2->symbol | KSymbolAttr_Pattern;
		tk2->stmtEntryKey = tk2->symbol;
		tk->tokenType = TokenType_Skip;
	}
	if(tk->symbol == KSymbol_COLON || tk2->symbol == KSymbol_COLON) {
		DBG_ASSERT(tk->stmtEntryKey != 0);
		tk2->stmtEntryKey = tk->stmtEntryKey;
		tk->tokenType = TokenType_Skip;
	}
	if(tk2->symbol == KSymbol_MUL) {
		kToken_Set(MatchPreviousPattern, tk, true);
		tk2->tokenType = TokenType_Skip;
	}
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
	Tokenize(kctx, ns, ruleSource, uline, source.tokenList);
	KTokenSeq step1 = {ns, source.tokenList, kArray_size(source.tokenList)};
	Preprocess(kctx, ns, RangeTokenSeq(source), NULL, step1.tokenList);
	KTokenSeq_End(kctx, step1);
	TraverseTokenList(kctx, RangeTokenSeq(step1), PreprocessSyntaxPattern, NULL);
	TraverseTokenList2(kctx, RangeTokenSeq(step1), PreprocessSyntaxPattern2, NULL);
	Preprocess(kctx, ns, RangeTokenSeq(step1), NULL, syn->syntaxPatternListNULL);
	KTokenSeq_Pop(kctx, source);
}

//// ---------------------------------------------------------------------------
///* parsing syntax pattern */
//
//#define kTokenPattern_TopChar(tk) ((IS_String(tk->text) && kString_size((tk)->text) == 1) ? kString_text((tk)->text)[0] : 0)
//
//static int KTokenSeq_FindCloseCharAsTopChar(KonohaContext *kctx, KTokenSeq *tokens, int beginIdx, int closech)
//{
//	int i;
//	for(i = beginIdx; i < tokens->endIdx; i++) {
//		kToken *tk = tokens->tokenList->TokenItems[i];
//		if(kTokenPattern_TopChar(tk) == closech) return i;
//	}
//	return tokens->endIdx;
//}
//
//static kbool_t kArray_AddSyntaxPattern(KonohaContext *kctx, kArray *patternList, KTokenSeq *patterns);
//
//static int KTokenSeq_NestedSyntaxPattern(KonohaContext *kctx, KTokenSeq *tokens, int currentIdx, ksymbol_t KSymbol_AST, int closech)
//{
//	kTokenVar *tk = tokens->tokenList->TokenVarItems[currentIdx];
//	int ne = KTokenSeq_FindCloseCharAsTopChar(kctx, tokens, currentIdx+1, closech);
//	tk->resolvedSymbol = KSymbol_AST;
//	tk->resolvedSyntaxInfo = kSyntax_(tokens->ns, KSymbol_AST);
//	KFieldSet(tk, tk->GroupTokenList, new_(TokenArray, 0, OnField));
//	KTokenSeq nestedSourceRange = {tokens->ns, tokens->tokenList, currentIdx+1, ne};
//	return kArray_AddSyntaxPattern(kctx, tk->GroupTokenList, &nestedSourceRange) ? ne : tokens->endIdx;
//}
//
//static kbool_t kArray_AddSyntaxPattern(KonohaContext *kctx, kArray *patternList, KTokenSeq *patterns)
//{
//	int i;
//	ksymbol_t stmtEntryKey = 0;
//	kTokenVar *tk, *prevToken = NULL;
//	for(i = patterns->beginIdx; i < patterns->endIdx; i++, prevToken = tk) {
//		tk = patterns->tokenList->TokenVarItems[i];
//		DBG_ASSERT(!kToken_IsIndent(tk));
//		DBG_ASSERT(tk->resolvedSyntaxInfo != NULL);
//		if(tk->resolvedSyntaxInfo->keyword == KSymbol_TextPattern) {
//			int topch = kTokenPattern_TopChar(tk);
//			KLIB kArray_Add(kctx, patternList, tk);
//			if(topch == '(') {
//				i = KTokenSeq_NestedSyntaxPattern(kctx, patterns, i, KSymbol_ParenthesisGroup, ')');
//			}
//			else if(topch == '[') {
//				i = KTokenSeq_NestedSyntaxPattern(kctx, patterns, i, KSymbol_BracketGroup, ']');
//			}
//			else {
//				tk->resolvedSymbol = KAsciiSymbol(kString_text(tk->text), kString_size(tk->text), KSymbol_NewId);
//			}
//			continue;
//		}
//		if(tk->resolvedSyntaxInfo->keyword == KSymbol_BracketGroup) {
//			KTokenSeq nestedSourceRange = {patterns->ns, tk->GroupTokenList, 0, kArray_size(tk->GroupTokenList)};
//			tk->resolvedSymbol = KSymbol_OptionalGroup;
//			PUSH_GCSTACK2(tk->GroupTokenList);  // avoid gc
//			KFieldSet(tk, tk->GroupTokenList, new_(TokenArray, 0, OnField));
//			kArray_AddSyntaxPattern(kctx, tk->GroupTokenList, &nestedSourceRange);
//			KLIB kArray_Add(kctx, patternList, tk);
//			continue;
//		}
//		if(tk->resolvedSymbol == KSymbol_DOLLAR/*tk->hintChar == '$'*/ && i+1 < patterns->endIdx) {  // $PatternName
//			tk = patterns->tokenList->TokenVarItems[++i];
//			if(IS_String(tk->text)) {
//				tk->resolvedSymbol = KAsciiSymbol(kString_text(tk->text), kString_size(tk->text), KSymbol_NewRaw) | KSymbolAttr_Pattern;
//				if(stmtEntryKey == 0) stmtEntryKey = tk->resolvedSymbol;
//				tk->stmtEntryKey = stmtEntryKey;
//				tk->resolvedSyntaxInfo = kSyntax_(patterns->ns, tk->resolvedSymbol/*KSymbol_SymbolPattern*/);
//				stmtEntryKey = 0;
//				KLIB kArray_Add(kctx, patternList, tk);
//				continue;
//			}
//		}
//		if(i + 1 < patterns->endIdx && patterns->tokenList->TokenItems[i+1]->resolvedSymbol == KSymbol_COLON && IS_String(tk->text)) {
//			stmtEntryKey = KAsciiSymbol(kString_text(tk->text), kString_size(tk->text), KSymbol_NewRaw);
//			i++;
//			continue;
//		}
//		if(tk->resolvedSymbol == KSymbol_MUL/*tk->hintChar == '*'*/ && prevToken != NULL) {
//			kToken_Set(MatchPreviousPattern, prevToken, true);
//			tk = NULL;
//			continue;
//		}
//		kToken_ToError(kctx, tk, ErrTag, "illegal syntax pattern: %s", KToken_t(tk));
//		return false;
//	}
//	return true;
//}
//
//#define KPushBufferList(L) kArray *L = KGetParserContext(kctx)->preparedTokenList; _pushBufferIdx = kArray_size(L)
//#define KPopBufferList(L)  KLIB kArray_Clear(kctx, L, _pushBufferIdx)
//
//static void kNameSpace_AddSyntaxPattern(KonohaContext *kctx, kNameSpace *ns, ksymbol_t kw, const char *ruleSource, kfileline_t uline, KTraceInfo *trace)
//{
//	kSyntaxVar *syn = (kSyntaxVar*)kNameSpace_GetSyntax(kctx, ns, kw);
//	DBG_ASSERT(IS_NOTNULL(syn));
//	if(syn->syntaxPatternListNULL == NULL) {
//		syn->syntaxPatternListNULL = new_(TokenArray, 0, ns->NameSpaceConstList);
//	}
//	KTokenSeq source = {ns, KGetParserContext(kctx)->preparedTokenList};
//	KTokenSeq_Push(kctx, source);
//	Tokenize(kctx, &source, ruleSource, uline);
//	KTokenSeq patterns = {ns, source.tokenList, source.endIdx};
//	patterns.TargetPolicy.RemovingIndent = true;
//	Preprocess(kctx, &patterns, NULL, &source, source.beginIdx);
//	kArray_AddSyntaxPattern(kctx, syn->syntaxPatternListNULL, &patterns);
//	KTokenSeq_Pop(kctx, source);
//	//KdumpTokenArray(kctx, patternList, 0, kArray_size(patternList));
//}