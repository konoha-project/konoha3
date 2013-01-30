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

static int MatchSyntaxPattern(KonohaContext *kctx, kNode *node, KTokenSeq *tokens, KTokenSeq *patterns, kToken **errRuleRef)
{
	int patternIdx = patterns->beginIdx, tokenIdx = tokens->beginIdx;
	kNameSpace *ns = kNode_ns(node);
	//SUGAR dumpTokenArray(kctx, 0, patterns->tokenList, patterns->beginIdx, patterns->endIdx);
	//SUGAR dumpTokenArray(kctx, 0, tokens->tokenList, tokens->beginIdx, tokens->endIdx);
	for(; patternIdx < patterns->endIdx; patternIdx++) {
		kToken *ruleToken = patterns->tokenList->TokenItems[patternIdx];
		L_ReDo:;
		//tokenIdx = TokenUtils_SkipOnlyIndent(tokens->tokenList, tokenIdx, tokens->endIdx);
		//DBG_P("patternIdx=%d, tokenIdx=%d, tokenEndIdx=%d", patternIdx, tokenIdx, tokens->endIdx);
		//KDump(node);
		if(tokenIdx < tokens->endIdx) {
			kToken *tk = tokens->tokenList->TokenItems[tokenIdx];
			errRuleRef[1] = tk;
			if(KSymbol_IsPattern(ruleToken->symbol)) {
				kSyntax *syn = kSyntax_(ns, ruleToken->symbol);
				int nextIdx = ParseSyntaxNode(kctx, syn, node, ruleToken->ruleNameSymbol, tokens->tokenList, tokenIdx, -1, tokens->endIdx);
				if(nextIdx == PatternNoMatch) {
					if(!kToken_Is(MatchPreviousPattern, ruleToken)) {
						errRuleRef[0] = ruleToken;
						return PatternNoMatch;
					}
					continue;  /* to avoid check same rule */
				}
				tokenIdx = nextIdx ;
			}
			else if(ruleToken->symbol == KSymbol_OptionalGroup) {
				KTokenSeq nrule = {ns, RangeGroup(ruleToken->GroupTokenList)};
				tokens->beginIdx = tokenIdx;
				int nextIdx = MatchSyntaxPattern(kctx, node, tokens, &nrule, errRuleRef);
				errRuleRef[0] = NULL;
				if(kNode_IsError(node)) return tokens->endIdx;
				if(nextIdx != PatternNoMatch) {
					tokenIdx = nextIdx;
				}
			}
			else {
				if(ruleToken->symbol != tk->symbol) {
					errRuleRef[0] = ruleToken;
					return PatternNoMatch;
				}
				if(ruleToken->tokenType == KSymbol_ParenthesisGroup || ruleToken->tokenType == KSymbol_BracketGroup) {
					KTokenSeq nrule = {ns, RangeGroup(ruleToken->GroupTokenList)};
					KTokenSeq ntokens = {ns, RangeGroup(tk->GroupTokenList)};
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
				if(ruleToken->symbol != KSymbol_OptionalGroup && !kToken_Is(MatchPreviousPattern,ruleToken)) {
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
			kNode_Message(kctx, node, ErrTag, "%s%s: %s%s is expected before %s", KSymbol_Fmt2(node->syn->keyword), KSymbol_Fmt2(errRule[0]->symbol), KToken_t(errRule[1]));
		} else {
			kNode_Message(kctx, node, ErrTag, "%s%s: %s%s is expected", KSymbol_Fmt2(node->syn->keyword), KSymbol_Fmt2(errRule[0]->symbol));
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
		if(kToken_IsStatementSeparator(tk)) continue;
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
		if(currentIdx < endIdx && tokenList->TokenItems[currentIdx]->symbol == KSymbol_COMMA) {
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
	kTokenVar *typeToken = tokenList->TokenVarItems[beginIdx];
	KClass *foundClass = NULL;
	if(typeToken->resolvedSyntaxInfo->keyword == KSymbol_TypePattern) {
		foundClass = KClass_(typeToken->resolvedTypeId);
		nextIdx = beginIdx + 1;
	}
	if(foundClass != NULL) {
		int isAllowedGenerics = true;
		for(; nextIdx < endIdx; nextIdx++) {
			kTokenVar *suffixToken = tokenList->TokenVarItems[nextIdx];
			if(suffixToken->tokenType == TokenType_Skip) {
				isAllowedGenerics = false;
				continue;
			}
			if(suffixToken->resolvedSyntaxInfo->keyword != KSymbol_BracketGroup) break;
			int sizeofBracketTokens = kArray_size(suffixToken->GroupTokenList);
			if(isAllowedGenerics &&  sizeofBracketTokens > 2) {  // C[T][]
				KClass *foundGenericClass = ParseGenericsType(kctx, ns, foundClass, RangeGroup(suffixToken->GroupTokenList));
				if(foundGenericClass == NULL) break;
				foundClass = foundGenericClass;
				suffixToken->tokenType = TokenType_Skip;
			}
			else {
				if(sizeofBracketTokens > 2) break;   // C[100] is treated as C  and the token [100] is set to nextIdx;
				foundClass = KClass_p0(kctx, KClass_Array, foundClass->typeId);  // C[] => Array[C]
				suffixToken->tokenType = TokenType_Skip;
			}
			isAllowedGenerics = false;
		}
	}
	if(foundClass != NULL) {
		if(typeToken->resolvedTypeId != foundClass->typeId) {
			typeToken->resolvedTypeId = foundClass->typeId;
			//DBG_P("Retype foundClass=%s", KClass_text(foundClass));
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
		tk->symbol = KSymbol_OptionalGroup;
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
			tk->symbol = tk->tokenType;
		}
	}
	tk->ruleNameSymbol = tk->symbol;
}

static void PreprocessSyntaxPattern2(KonohaContext *kctx, kTokenVar *tk, kTokenVar *tk2, void *thunk)
{
	if(tk->symbol == KSymbol_DOLLAR) {  // $PatternName
		tk2->symbol = tk2->symbol | KSymbolAttr_Pattern;
		tk2->ruleNameSymbol = tk->ruleNameSymbol != KSymbol_DOLLAR ? tk->ruleNameSymbol : tk2->symbol;
		tk->tokenType = TokenType_Skip;
	}
	if(tk2->symbol == KSymbol_COLON) {
		DBG_ASSERT(tk->ruleNameSymbol != 0);
		tk2->ruleNameSymbol = tk->ruleNameSymbol;
		tk->tokenType = TokenType_Skip;
	}
	if(tk->symbol == KSymbol_COLON) {
		DBG_ASSERT(tk->ruleNameSymbol != 0);
		tk2->ruleNameSymbol = tk->ruleNameSymbol;
		tk->tokenType = TokenType_Skip;
	}
	if(tk2->symbol == KSymbol_MUL) {
		kToken_Set(MatchPreviousPattern, tk, true);
		tk2->tokenType = TokenType_Skip;
	}
}

static void kSyntax_AddPattern(KonohaContext *kctx, kSyntax *syntax0, const char *ruleSource, kfileline_t uline, KTraceInfo *trace)
{
	if(IS_NOTNULL(syntax0)) {
		kSyntaxVar *syntax = (kSyntaxVar *)syntax0;
		kNameSpace *ns = syntax->packageNameSpace;
		if(syntax->syntaxPatternListNULL == NULL) {
			syntax->syntaxPatternListNULL = new_(TokenArray, 0, ns->NameSpaceConstList);
		}
		KTokenSeq source = {ns, KGetParserContext(kctx)->preparedTokenList};
		KTokenSeq_Push(kctx, source);
		Tokenize(kctx, ns, ruleSource, uline, 0, source.tokenList);
		KTokenSeq_End(kctx, source);
		KTokenSeq step1 = {ns, source.tokenList, kArray_size(source.tokenList)};
		Preprocess(kctx, ns, RangeTokenSeq(source), NULL, step1.tokenList);
		KTokenSeq_End(kctx, step1);
		TraverseTokenList(kctx, RangeTokenSeq(step1), PreprocessSyntaxPattern, NULL);
		TraverseTokenList2(kctx, RangeTokenSeq(step1), PreprocessSyntaxPattern2, NULL);
		Preprocess(kctx, ns, RangeTokenSeq(step1), NULL, syntax->syntaxPatternListNULL);
		KTokenSeq_Pop(kctx, source);
		SUGAR dumpTokenArray(kctx, 0, RangeArray(syntax->syntaxPatternListNULL));
	}
}
