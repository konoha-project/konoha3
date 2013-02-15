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
	CallSugarMethod(kctx, lsfp, fo, 6, (kObject *)KNULL(Int));
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
	if(syn->ParseFuncNULL != NULL) {
		//DBG_P(">>> package=%s", KPackage_text(syn->packageNameSpace->packageId));
		int nextIdx = CallParseFunc(kctx, syn->ParseFuncNULL, node, symbol, tokenList, beginIdx, opIdx, endIdx);
		if(nextIdx != PatternNoMatch) return nextIdx;
		callCount++;
	}
	size_t i;
	kArray *syntaxList = kNameSpace_GetSyntaxList(kctx, kNode_ns(node), syn->keyword);
	for(i = 1; i < kArray_size(syntaxList); i++) { /* ObjectItems[0] == syn */
		kSyntax *syn2 = syntaxList->SyntaxItems[i];
		if(syn2->ParseFuncNULL != NULL) {
			//DBG_P(">>> package=%s", KPackage_text(syn2->packageNameSpace->packageId));
			int nextIdx = CallParseFunc(kctx, syn2->ParseFuncNULL, node, symbol, tokenList, beginIdx, opIdx, endIdx);
			if(nextIdx != PatternNoMatch && opIdx != -1) {
				node->syn = syn2;
				return nextIdx;
			}
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
		if(kToken_IsStatementSeparator(tk)) continue;
		break;
	}
	return currentIdx;
}

static int FindEndOfStatement(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx)
{
	int c;
	for(c = beginIdx; c < endIdx; c++) {
		kToken *tk = tokenList->TokenItems[c];
		if(kToken_IsStatementSeparator(tk)) return c;
	}
	return endIdx;
}

static int SkipAnnotation(KonohaContext *kctx, kArray *tokenList, int currentIdx, int endIdx)
{
	for(; currentIdx < endIdx; currentIdx++) {
		kToken *tk = tokenList->TokenItems[currentIdx];
		if(kToken_IsIndent(tk)) continue;
		if(KSymbol_IsAnnotation(tk->symbol)) {
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
		if(KSymbol_IsAnnotation(tk->symbol)) {
			kObject *value = UPCAST(K_TRUE);
			if(currentIdx + 1 < endIdx) {
				kToken *nextToken = tokenList->TokenItems[currentIdx+1];
				if(nextToken->resolvedSyntaxInfo != NULL && nextToken->resolvedSyntaxInfo->keyword == KSymbol_ParenthesisGroup) {
					int start = 0;
					value = (kObject *)SUGAR ParseNewNode(kctx, kNode_ns(stmt), nextToken->GroupTokenList, &start, kArray_size(nextToken->GroupTokenList), ParseExpressionOption, "(");
					currentIdx++;
				}
			}
			KLIB kObjectProto_SetObject(kctx, stmt, tk->symbol, kObject_typeId(value), value);
			continue;
		}
		break;
	}
}

static int ParseMetaPattern(KonohaContext *kctx, kNameSpace *ns, kNode *node, kArray *tokenList, int beginIdx, int endIdx)
{
	int i;
	//SUGAR dumpTokenArray(kctx, 0, tokenList, beginIdx, endIdx);
	for(i = beginIdx; i < endIdx; i++) {
		kToken *tk = tokenList->TokenItems[i];
		if(kToken_IsStatementSeparator(tk)) {
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
				int nextIdx = ParseSyntaxNode(kctx, patternSyntax, node, 0, tokenList, currentIdx, PatternNoMatch, endIdx);
				//DBG_P(">>>>>>>>>> searching meta pattern = %s%s index=%d,%d,%d", KSymbol_Fmt2(patternToken->symbol), beginIdx, nextIdx, endIdx);
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
	kbool_t isPrePosition = true;
	int opIdx = beginIdx, i, precedence = 0;
	kToken *typeToken = NULL;
	for(i = beginIdx; i < endIdx; i++) {
		kToken *tk = tokenList->TokenItems[i];
		kSyntax *syntax = tk->resolvedSyntaxInfo;
		if(isPrePosition) {
			if(syntax->precedence_op1 > 0) {
				if(precedence < syntax->precedence_op1) {
					precedence = syntax->precedence_op1;
					opIdx = i;
				}
				continue;
			}
			isPrePosition = false;
		}
		else {
			if(syntax->precedence_op2 > 0) {
				if(kSyntax_Is(TypeSuffix, syntax) && typeToken != NULL) {
					continue;
				}
				if(precedence < syntax->precedence_op2 || (precedence == syntax->precedence_op2 && !(FLAG_is(syntax->flag, SYNFLAG_NodeLeftJoinOp2)) )) {
					precedence = syntax->precedence_op2;
					opIdx = i;
				}
				if(!FLAG_is(syntax->flag, SYNFLAG_Suffix)) {
					isPrePosition = true;
				}
			}
		}
		typeToken = (tk->resolvedSyntaxInfo->keyword == KSymbol_TypePattern) ? tk : NULL;
	}
	return opIdx;
}

static int ParseNode(KonohaContext *kctx, kNode *node, kArray *tokenList, int beginIdx, int endIdx, ParseOption option, const char *requiredTokenText)
{
	if(beginIdx < endIdx) {
		kNameSpace *ns =kNode_ns(node);
		int nextIdx = PatternNoMatch;
		if(KFlag_Is(int, option, ParseMetaPatternOption)) {
			nextIdx = ParseMetaPattern(kctx, ns, node, tokenList, beginIdx, endIdx);
		}
		if(nextIdx == PatternNoMatch) {
			int opIdx = FindOperator(kctx, node, tokenList, beginIdx, endIdx);
			kToken *keyOperator = tokenList->TokenItems[opIdx];
			//DBG_P("KeyOperator >>>>>>>> %d<%d<%d, %s", beginIdx, opIdx, endIdx, KToken_t(keyOperator));
			nextIdx = ParseSyntaxNode(kctx, keyOperator->resolvedSyntaxInfo, node, 0, tokenList, beginIdx, opIdx, endIdx);
		}
		return nextIdx;
	}
	else {
		if(requiredTokenText != NULL) {
			kNode_Message(kctx, node, ErrTag, "expected expression after %s", requiredTokenText);
		}
		else {
			//DBG_ABORT("set null in future");
		}
	}
	return endIdx;
}

static kNode* ParseNewNode(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int *beginIdx, int endIdx, ParseOption option, const char *requiredTokenText)
{
	kNode *node = new_UntypedNode(kctx, OnGcStack, ns);
	int nextIdx = ParseNode(kctx, node, tokenList, beginIdx[0], endIdx, option, requiredTokenText);
	beginIdx[0] = nextIdx;
	return node;
}


static kbool_t IsCommaSeparator(KonohaContext *kctx, kToken *tk)
{
	return (tk->symbol == KSymbol_COMMA);
}

static kNode *AppendParsedNode(KonohaContext *kctx, kNode *node, kArray *tokenList, int beginIdx, int endIdx, IsSeparatorFunc isSeparator, ParseOption option, const char *requiredTokenText)
{
	int i, start = beginIdx;
	kNameSpace *ns = kNode_ns(node);
	if(isSeparator == NULL) {
		isSeparator = IsCommaSeparator;
	}
	//DBG_P("beginIdx=%d, endIdx=%d", beginIdx, endIdx);
	for(i = beginIdx; i < endIdx; i++) {
		kToken *tk = tokenList->TokenItems[i];
		if(isSeparator(kctx, tk)) {
			if(start < i || requiredTokenText != NULL) {
				kNode_AddNode(kctx, node, ParseNewNode(kctx, ns, tokenList, &start, i, option, requiredTokenText));
				if(requiredTokenText != NULL) requiredTokenText = ",";
			}
			start = i + 1;
		}
	}
	if(start < i || requiredTokenText != NULL) {
		kNode_AddNode(kctx, node, ParseNewNode(kctx, ns, tokenList, &start, i, option, requiredTokenText));
	}
	return node;
}

static kNode* ParseSource(KonohaContext *kctx, kNameSpace *ns, const char *script, kfileline_t uline, int baseIndent)
{
	kNode *node;
	KTokenSeq tokens = {ns, KGetParserContext(kctx)->preparedTokenList, 0};
	KTokenSeq_Push(kctx, tokens);
	Tokenize(kctx, ns, script, uline, baseIndent, tokens.tokenList);
	KTokenSeq_End(kctx, tokens);
	{
		KTokenSeq step2 = {ns, tokens.tokenList, kArray_size(tokens.tokenList)};
		Preprocess(kctx, ns, RangeTokenSeq(tokens), NULL, step2.tokenList);
		KTokenSeq_End(kctx, step2);
		node = ParseNewNode(kctx, ns, step2.tokenList, &step2.beginIdx, step2.endIdx, ParseMetaPatternOption, NULL);
	}
	KTokenSeq_Pop(kctx, tokens);
	return node;
}
