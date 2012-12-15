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

/* ************************************************************************ */

/* ************************************************************************ */

#ifdef __cplusplus
extern "C" {
#endif


/* ------------------------------------------------------------------------ */

static kExpr *CallExpressionFunc(KonohaContext *kctx, KSyntax *syn, kFunc *fo, int *countRef, kStmt *stmt, kArray *tokenList, int beginIdx, int operatorIdx, int endIdx)
{
	BEGIN_UnusedStack(lsfp);
	lsfp[0].unboxValue = (uintptr_t)syn;
	KUnsafeFieldSet(lsfp[0].asNameSpace, kStmt_ns(stmt));
	KUnsafeFieldSet(lsfp[1].asStmt, stmt);
	KUnsafeFieldSet(lsfp[2].asArray, tokenList);
	lsfp[3].intValue = beginIdx;
	lsfp[4].intValue = operatorIdx;
	lsfp[5].intValue = endIdx;
	countRef[0] += 1;
	CallSugarMethod(kctx, lsfp, fo, 5, UPCAST(K_NULLEXPR));
	END_UnusedStack();
	DBG_ASSERT(IS_Expr(lsfp[K_RTNIDX].asObject));
	return lsfp[K_RTNIDX].asExpr;
}

static kExpr *kStmt_ParseOperatorExpr(KonohaContext *kctx, kStmt *stmt, KSyntax *exprSyntax, kArray *tokenList, int beginIdx, int operatorIdx, int endIdx)
{
	int callCount = 0;
	KSyntax *currentSyntax = exprSyntax;
	while(true) {
		int index, size;
		kFunc **funcItems = KSyntax_funcTable(kctx, currentSyntax, SugarFunc_Expression, &size);
		for(index = size - 1; index >= 0; index--) {
			DBG_ASSERT(IS_Func(funcItems[index]));
			kExpr *texpr = CallExpressionFunc(kctx, exprSyntax, funcItems[index], &callCount, stmt, tokenList, beginIdx, operatorIdx, endIdx);
			if(kStmt_IsERR(stmt)) return K_NULLEXPR;
			if(texpr != K_NULLEXPR) return texpr;
		}
		if(currentSyntax->parentSyntaxNULL == NULL) break;
		currentSyntax = currentSyntax->parentSyntaxNULL;
	}
	const char *emesg = (callCount > 0) ? "syntax error: expression %s" : "undefined expression: %s";
	kStmt_Message(kctx, stmt, ErrTag, emesg, KToken_t(tokenList->TokenItems[operatorIdx]));
	return K_NULLEXPR;
}

static int kStmt_FindOperator(KonohaContext *kctx, kStmt *stmt, kArray *tokenList, int beginIdx, int endIdx)
{
	int isPrePosition = true;
	int idx = beginIdx, i, precedence = 0;
	for(i = beginIdx; i < endIdx; i++) {
		kToken *tk = tokenList->TokenItems[i];
		KSyntax *syn = tk->resolvedSyntaxInfo;
		if(isPrePosition) {
			if(syn->precedence_op1 > 0) {
				if(precedence < syn->precedence_op1) {
					precedence = syn->precedence_op1;
					idx = i;
				}
				continue;
			}
			isPrePosition = false;
		}
		else {
			if(syn->precedence_op2 > 0) {
				if(precedence < syn->precedence_op2 || (precedence == syn->precedence_op2 && !(FLAG_is(syn->flag, SYNFLAG_ExprLeftJoinOp2)) )) {
					precedence = syn->precedence_op2;
					idx = i;
				}
				if(!FLAG_is(syn->flag, SYNFLAG_ExprPostfixOp2)) isPrePosition = true;
			}
		}
	}
	return idx;
}

static kExpr* kStmt_ParseExpr(KonohaContext *kctx, kStmt *stmt, kArray *tokenList, int beginIdx, int endIdx, const char *hintBeforeText)
{
	if(!kStmt_IsERR(stmt)) {
		if(beginIdx < endIdx) {
			int idx = kStmt_FindOperator(kctx, stmt, tokenList, beginIdx, endIdx);
			KSyntax *syn = tokenList->TokenItems[idx]->resolvedSyntaxInfo;
			return kStmt_ParseOperatorExpr(kctx, stmt, syn, tokenList, beginIdx, idx, endIdx);
		}
		else {
			if(hintBeforeText == NULL) hintBeforeText = "";
			kStmt_Message(kctx, stmt, ErrTag, "expected expression after %s", hintBeforeText);
		}
	}
	return K_NULLEXPR;
}

static int kTokenArray_RemoveIndent(KonohaContext *kctx, kArray *tokenList, int s, int e)
{
	int i, p = s;
	for(i = s; i < e; i++) {
		kToken *tk = tokenList->TokenItems[i];
		if(kToken_IsIndent(tk)) continue;
		if(p < i) {
			tokenList->TokenItems[p] = tokenList->TokenItems[i]; // TODO: GC
		}
		p++;
	}
	return p;
}

static kExpr *kStmt_AddExprParam(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kArray *tokenList, int s, int e, const char *hintBeforeText/* if NULL empty isAllowed */)
{
	int i, start = s;
	e = kTokenArray_RemoveIndent(kctx, tokenList, s, e);
	for(i = s; i < e; i++) {
		kToken *tk = tokenList->TokenItems[i];
		if(tk->resolvedSyntaxInfo->keyword == KSymbol_COMMA) {
			if(start < i || hintBeforeText != NULL) {
				expr = kExpr_Add(kctx, expr, kStmt_ParseExpr(kctx, stmt, tokenList, start, i, hintBeforeText));
				if(hintBeforeText != NULL) hintBeforeText = ",";
			}
			start = i + 1;
		}
	}
	if(start < i || hintBeforeText != NULL) {
		expr = kExpr_Add(kctx, expr, kStmt_ParseExpr(kctx, stmt, tokenList, start, i, hintBeforeText));
	}
	return expr;
}

static kExpr *kStmt_RightJoinExpr(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kArray *tokenList, int c, int e)
{
	if(c < e && expr != K_NULLEXPR && !kStmt_IsERR(stmt)) {
		kToken *tk = tokenList->TokenItems[c];
		if(tk->resolvedSyntaxInfo->keyword == KSymbol_SymbolPattern || tk->resolvedSyntaxInfo->sugarFuncTable[SugarFunc_Expression] == NULL) {
			DBG_ASSERT(c >= 1);
			kToken *previousToken = tokenList->TokenItems[c-1];
			const char *white = kToken_Is(BeforeWhiteSpace, previousToken) ? " " : "";
			kStmtToken_Message(kctx, stmt, tk, ErrTag, "undefined syntax: %s%s%s ...", KToken_t(previousToken), white, KToken_t(tk));
			return K_NULLEXPR;
		}
		kStmtToken_Message(kctx, stmt, tk, WarnTag, "ignored term: %s...", KToken_t(tk));
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

static int TokenUtils_ParseTypePattern(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx, KClass **classRef);
static KClass* TokenUtils_ParseGenericsType(KonohaContext *kctx, kNameSpace *ns, KClass *baseClass, kArray *tokenList, int beginIdx, int endIdx)
{
	int currentIdx = beginIdx;
	size_t psize = 0;
	kparamtype_t *p = ALLOCA(kparamtype_t, endIdx);
	while(currentIdx < endIdx) {
		KClass *paramClass = NULL;
		currentIdx = TokenUtils_ParseTypePattern(kctx, ns, tokenList, currentIdx, endIdx, &paramClass);
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

static int TokenUtils_ParseTypePattern(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx, KClass **classRef)
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
			int sizeofBracketTokens = kArray_size(tk->subTokenList);
			if(isAllowedGenerics &&  sizeofBracketTokens > 0) {  // C[T][]
				KClass *foundGenericClass = TokenUtils_ParseGenericsType(kctx, ns, foundClass, tk->subTokenList, 0, sizeofBracketTokens);
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

static kArray* new_kArraySubset(KonohaContext *kctx, kArray *gcstack, kArray *a, int beginIdx, int endIdx)
{
	kArray *newa = new_(Array, endIdx - beginIdx, gcstack);
	int i;
	for(i = beginIdx; i < endIdx; i++) {
		KLIB kArray_Add(kctx, newa, a->ObjectItems[i]);
	}
	return newa;
}

static kTokenVar* kToken_ExpandGroupMacro(KonohaContext *kctx, kTokenVar *tk, kNameSpace *ns, KMacroSet *macroParam, kArray *gcstack)
{
	KTokenSeq source = {ns, tk->subTokenList, 0, kArray_size(tk->subTokenList)};
	if(source.endIdx > 0) {
		int isChanged = true;
		KTokenSeq group = {ns, GetSugarContext(kctx)->preparedTokenList};
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
			KFieldSet(groupToken, groupToken->subTokenList, new_kArraySubset(kctx, gcstack, group.tokenList, group.beginIdx, group.endIdx));
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
	int dstart = kArray_size(tokens->tokenList);
	KTokenSeq_Preprocess(kctx, tokens, macroParam, &macro, beginIdx + paramsize);
	DBG_P("dstart=%d, tokens->begin,end=%d, %d", dstart, tokens->beginIdx, tokens->endIdx);
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
		mp[i].tokenList = groupToken->subTokenList;
	}
	mp[paramsize].symbol = 0; /* sentinel */

	int p = 0, start = 0;
	for(i = 0; i < kArray_size(groupToken->subTokenList); i++) {
		kToken *tk = groupToken->subTokenList->TokenItems[i];
		if(tk->hintChar == ',') {
			mp[p].beginIdx = start;
			mp[p].endIdx = i;
			p++;
			start = i + 1;
		}
	}
	DBG_ASSERT(p < paramsize);
	mp[p].beginIdx = start;
	mp[p].endIdx = kArray_size(groupToken->subTokenList);
	KTokenSeq_ApplyMacro(kctx, tokens, macroTokenList, 0, kArray_size(macroTokenList), paramsize, mp);
}

static kbool_t kNameSpace_SetMacroData(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, int paramsize, const char *data, int optionMacro)
{
	KSyntaxVar *syn = (KSyntaxVar *)SUGAR kNameSpace_GetSyntax(kctx, ns, keyword, /*new*/true);
	if(syn->macroDataNULL == NULL) {
		KTokenSeq source = {ns, GetSugarContext(kctx)->preparedTokenList};
		KTokenSeq_Push(kctx, source);
		KTokenSeq_Tokenize(kctx, &source, data, 0);
		KTokenSeq tokens = {source.ns, source.tokenList, source.endIdx};
		tokens.TargetPolicy.ExpandingBraceGroup = true;
		KTokenSeq_Preprocess(kctx, &tokens, NULL, &source, source.beginIdx);
		syn->macroParamSize = paramsize;
		syn->macroDataNULL =  new_kArraySubset(kctx, ns->NameSpaceConstList, tokens.tokenList, tokens.beginIdx, tokens.endIdx);
		if(optionMacro) syn->flag |= SYNFLAG_Macro;
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
	astToken->resolvedSyntaxInfo = KSyntax_(tokens->ns, AST_type);
	KFieldSet(astToken, astToken->subTokenList, new_(TokenArray, 0, OnField));
	astToken->uline = openToken->uline;
	kToken_SetHintChar(astToken, openToken->hintChar, closech);
	{
		KTokenSeq nested = {source->ns, astToken->subTokenList};
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
	KTokenSeq source = {ns, GetSugarContext(kctx)->preparedTokenList};
	KTokenSeq_Push(kctx, source);
	KdumpToken(kctx, tk);
	KTokenSeq_Tokenize(kctx, &source, kString_text(tk->text), tk->uline);
	KFieldSet(tk, tk->subTokenList, new_(TokenArray, 0, OnField));
	tk->resolvedSyntaxInfo = KSyntax_(ns, KSymbol_BraceGroup);
	KTokenSeq tokens = {ns, tk->subTokenList, 0};
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
	kToken_SetHintChar(tk, 0, ',');
	return tk;
}

static int KTokenSeq_ApplyMacroSyntax(KonohaContext *kctx, KTokenSeq *tokens, KSyntax *syn, KMacroSet *macroParam, KTokenSeq *source, int currentIdx)
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
			int count = TokenUtils_Count(groupToken->subTokenList, 0, kArray_size(groupToken->subTokenList), KSymbol_COMMA);
			if(syn->macroParamSize == count + 2) {
				nextIdx = TokenUtils_SkipIndent(source->tokenList, nextIdx+1, source->endIdx);
				if(nextIdx < source->endIdx) {
					kTokenVar *tk = source->tokenList->TokenVarItems[nextIdx];
					if(tk->unresolvedTokenType == TokenType_CODE) {
						tk = kToken_ToBraceGroup(kctx, tk, tokens->ns, macroParam);
						new_CommaToken(kctx, groupToken->subTokenList);
						KLIB kArray_Add(kctx, groupToken->subTokenList, tk);
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
		tokens->TargetPolicy.syntaxSymbolPattern = KSyntax_(tokens->ns, KSymbol_SymbolPattern);
	}
	for(; currentIdx < source->endIdx; currentIdx++) {
		kTokenVar *tk = source->tokenList->TokenVarItems[currentIdx];
		if(kToken_IsIndent(tk) && tokens->TargetPolicy.RemovingIndent) {
			continue;  // remove INDENT in () or []
		}
		if(tk->unresolvedTokenType == TokenType_CODE && (tokens->TargetPolicy.ExpandingBraceGroup || macroParam != NULL)) {
			tk = kToken_ToBraceGroup(kctx, tk, tokens->ns, macroParam);
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
			if(keyword == TokenType_CODE) {
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
			if(tk->unresolvedTokenType == TokenType_ERR) {
				source->SourceConfig.foundErrorToken = tk;
				return source->endIdx;  // resolved no more
			}
			if(tk->unresolvedTokenType == TokenType_SYMBOL) {
				const char *t = kString_text(tk->text);
				ksymbol_t symbol = KAsciiSymbol(t, kString_size(tk->text), KSymbol_NewId);
				if(macroParam != NULL && KTokenSeq_ExpandMacro(kctx, tokens, symbol, macroParam)) {
					continue;
				}
				KSyntax *syn = KSyntax_(source->ns, symbol);
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
				tk->resolvedSyntaxInfo = KSyntax_(tokens->ns, tk->unresolvedTokenType);
				if(!kToken_Is(StatementSeparator, tk) && tk->unresolvedTokenType != TokenType_INDENT) {
					if(tk->resolvedSyntaxInfo == NULL) {
						kToken_ToError(kctx, tk, ErrTag, "undefined pattern: %s%s", KSymbol_Fmt2(tk->unresolvedTokenType));
						source->SourceConfig.foundErrorToken = tk;
						goto RETURN_ERROR;
					}
				}
			}
		}
		KLIB kArray_Add(kctx, tokens->tokenList, tk);
	}
	if(source->SourceConfig.openToken != NULL) {
		char buf[2] = {source->SourceConfig.stopChar, 0};
		ERROR_UnclosedToken(kctx, (kTokenVar *)source->SourceConfig.openToken, (const char *)buf);
		source->SourceConfig.foundErrorToken = source->SourceConfig.openToken;
	}
	RETURN_ERROR:;
	KTokenSeq_End(kctx, tokens);
	return source->endIdx;
}

/* ------------------------------------------------------------------------ */

static int CallPatternMatchFunc(KonohaContext *kctx, kFunc *fo, int *countRef, kStmt *stmt, ksymbol_t name, kArray *tokenList, int beginIdx, int endIdx)
{
	INIT_GCSTACK();
	BEGIN_UnusedStack(lsfp);
	KUnsafeFieldSet(lsfp[0].asNameSpace, kStmt_ns(stmt));
	KUnsafeFieldSet(lsfp[1].asStmt, stmt);
	lsfp[2].intValue = name;
	KUnsafeFieldSet(lsfp[3].asArray, tokenList);
	lsfp[4].intValue = beginIdx;
	lsfp[5].intValue = endIdx;
	countRef[0] += 1;
	CallSugarMethod(kctx, lsfp, fo, 5, KLIB Knull(kctx, KClass_Int));
	END_UnusedStack();
	RESET_GCSTACK();
	return (int)lsfp[K_RTNIDX].intValue;
}

static int KSyntax_MatchPattern(KonohaContext *kctx, KSyntax *syn, kToken *patternToken, kStmt *stmt, int name, kArray *tokenList, int beginIdx, int endIdx)
{
	int callCount = 0;
	if(syn != NULL) {
		while(true) {
			int index, size;
			kFunc **funcItems = KSyntax_funcTable(kctx, syn, SugarFunc_PatternMatch, &size);
			for(index = size - 1; index >= 0; index--) {
				int next = CallPatternMatchFunc(kctx, funcItems[index], &callCount, stmt, name, tokenList, beginIdx, endIdx);
				if(kStmt_IsERR(stmt)) return -1;
				if(next >= beginIdx) return next;
			}
			if(syn->parentSyntaxNULL == NULL) break;
			syn = syn->parentSyntaxNULL;
		}
	}
	if(callCount == 0) {
		kStmtToken_Message(kctx, stmt, patternToken, ErrTag, "undefined syntax pattern: %s%s", KSymbol_Fmt2(patternToken->resolvedSymbol));
	}
	return -1;
}

static int TokenUtils_SkipOnlyIndent(kArray *tokenList, int currentIdx, int endIdx)
{
	for(; currentIdx < endIdx; currentIdx++) {
		kToken *tk = tokenList->TokenItems[currentIdx];
		if(!kToken_IsIndent(tk)) break;
	}
	return currentIdx;
}

static int kStmt_MatchSyntaxPattern(KonohaContext *kctx, kStmt *stmt, KTokenSeq *tokens, KTokenSeq *patterns, kToken **errRuleRef)
{
	int patternIdx = patterns->beginIdx, tokenIdx = tokens->beginIdx;
	kNameSpace *ns = Stmt_ns(stmt);
//	KdumpTokenArray(kctx, patterns->tokenList, patterns->beginIdx, patterns->endIdx);
//	KdumpTokenArray(kctx, tokens->tokenList, tokens->beginIdx, tokens->endIdx);
	for(; patternIdx < patterns->endIdx; patternIdx++) {
		kToken *ruleToken = patterns->tokenList->TokenItems[patternIdx];
		L_ReDo:;
		tokenIdx = TokenUtils_SkipOnlyIndent(tokens->tokenList, tokenIdx, tokens->endIdx);
		//DBG_P("patternIdx=%d, tokenIdx=%d, tokenEndIdx=%d", patternIdx, tokenIdx, tokens->endIdx);
		if(tokenIdx < tokens->endIdx) {
			kToken *tk = tokens->tokenList->TokenItems[tokenIdx];
			errRuleRef[1] = tk;
			if(KSymbol_IsPattern(ruleToken->resolvedSymbol)) {
				KSyntax *syn = KSyntax_(ns, ruleToken->resolvedSymbol);
				int next = KSyntax_MatchPattern(kctx, syn, ruleToken, stmt, ruleToken->stmtEntryKey, tokens->tokenList, tokenIdx, tokens->endIdx);
				if(next < 0) {
					if(!kToken_Is(MatchPreviousPattern, ruleToken)) {
						errRuleRef[0] = ruleToken;
						return -1;
					}
					if(next == -2 /* see PatternMatch_Token */) {

					}
					continue;  /* to avoid check same rule */
				}
				else {  /*if(next != -1) */
					tokenIdx = next ;
				}
			}
			else if(ruleToken->resolvedSymbol == KSymbol_OptionalGroup) {
				KTokenSeq nrule = {ns, ruleToken->subTokenList, 0, kArray_size(ruleToken->subTokenList)};
				tokens->beginIdx = tokenIdx;
				int next = kStmt_MatchSyntaxPattern(kctx, stmt, tokens, &nrule, errRuleRef);
				errRuleRef[0] = NULL;
				if(kStmt_IsERR(stmt)) return -1;
				if(next != -1) {
					tokenIdx = next;
				}
			}
			else {
				if(ruleToken->resolvedSymbol != tk->resolvedSymbol) {
					errRuleRef[0] = ruleToken;
					return -1;
				}
				if(ruleToken->resolvedSymbol == KSymbol_ParenthesisGroup || ruleToken->resolvedSymbol == KSymbol_BracketGroup) {
					KTokenSeq nrule = {ns, ruleToken->subTokenList, 0, kArray_size(ruleToken->subTokenList)};
					KTokenSeq ntokens = {ns, tk->subTokenList, 0, kArray_size(tk->subTokenList)};
					int next = kStmt_MatchSyntaxPattern(kctx, stmt, &ntokens, &nrule, errRuleRef);
					if(next == -1) {
						return -1;
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

static KSyntax* kStmt_GuessStatementSyntax(KonohaContext *kctx, kStmt *stmt, kArray *tokenList, int beginIdx, int endIdx)
{
	kToken *tk = tokenList->TokenItems[beginIdx];
	KSyntax *syn = tk->resolvedSyntaxInfo;
	kNameSpace *ns = Stmt_ns(stmt);
	//DBG_P(">>>>>>>>>>>>>>>>>>> finding KSyntax=%s%s syn->syntaxPatternListNULL=%p", KSymbol_Fmt2(syn->keyword), syn->syntaxPatternListNULL);
	if(syn->syntaxPatternListNULL == NULL) {
		kNameSpace *currentNameSpace = ns;
		while(currentNameSpace != NULL) {
			kArray *stmtPatternList = currentNameSpace->stmtPatternListNULL_OnList;
			if(stmtPatternList != NULL) {
				int i;
				for(i = kArray_size(stmtPatternList) - 1; i >=0; i--) {
					kToken *patternToken = stmtPatternList->TokenItems[i];
					//DBG_P(">>>>>>>>>> searching patternToken=%s%s", KSymbol_Fmt2(patternToken->resolvedSymbol));
					if(KSyntax_MatchPattern(kctx, patternToken->resolvedSyntaxInfo, patternToken, stmt, 0, tokenList, beginIdx, endIdx) != -1) {
						return KSyntax_(ns, patternToken->stmtEntryKey);
					}
				}
			}
			currentNameSpace = currentNameSpace->parentNULL;
		}
		syn = KSyntax_(ns, KSymbol_ExprPattern);
	}
	return syn;
}

#define KWSTMT_t(kw)  StatementName(kctx, kw), StatementType(kw)

static const char* StatementName(KonohaContext *kctx, ksymbol_t keyword)
{
	const char *statement = KSymbol_text(keyword);
#ifndef USE_SMALLBUILD
	if(keyword == KSymbol_ExprPattern) statement = "expression";
	else if(keyword == KSymbol_TypeDeclPattern) statement = "variable";
	else if(keyword == KSymbol_MethodDeclPattern) statement =  "function";
#endif
	return statement;
}

static const char* StatementType(ksymbol_t keyword)
{
#ifdef USE_SMALLBUILD
	return "";
#else
	const char *postfix = " statement";
	if(keyword == KSymbol_ExprPattern) postfix = "";
	else if(keyword == KSymbol_TypeDeclPattern || keyword == KSymbol_MethodDeclPattern) postfix = " declaration";
	return postfix;
#endif
}

static int KTokenSeq_SelectSyntaxPattern(KonohaContext *kctx, KTokenSeq *patterns, kArray *patternList, int endIdx)
{
	int i;
	for(i = endIdx - 1; i >= 0; i--) {
		kToken *tk = patternList->TokenItems[i];
		if(IS_NULL(tk)) {
			patterns->endIdx = endIdx;
			patterns->beginIdx = i + 1;
			return i - 1;
		}
	}
	return -1;
}

static int kStmt_ParseBySyntaxPattern(KonohaContext *kctx, kStmt *stmt, int indent, kArray *tokenList, int beginIdx, int endIdx)
{
	KSyntax *stmtSyntax = kStmt_GuessStatementSyntax(kctx, stmt, tokenList, beginIdx, endIdx);
	((kStmtVar *)stmt)->syn = stmtSyntax;
	//DBG_P(">>>>>>>>>>>>>>>>>>> Found KSyntax=%s%s", KWSTMT_t(stmtSyntax->keyword));
	kToken *errRule[2] = {};
	kNameSpace *ns = Stmt_ns(stmt);
	KSyntax *currentSyntax = stmtSyntax;
	while(currentSyntax != NULL) {
		if(currentSyntax->syntaxPatternListNULL != NULL) {
			int patternEndIdx = kArray_size(currentSyntax->syntaxPatternListNULL);
			KTokenSeq tokens = {ns, tokenList, beginIdx, endIdx};
			KTokenSeq nrule  = {ns, currentSyntax->syntaxPatternListNULL, 0, kArray_size(currentSyntax->syntaxPatternListNULL)};
			do {
				patternEndIdx = KTokenSeq_SelectSyntaxPattern(kctx, &nrule, currentSyntax->syntaxPatternListNULL, patternEndIdx);
				errRule[0] = NULL; errRule[1] = NULL;
				int nextIdx = kStmt_MatchSyntaxPattern(kctx, stmt, &tokens, &nrule, errRule);
				if(kStmt_IsERR(stmt)) return -1;
				if(beginIdx < nextIdx) return nextIdx;
			} while(patternEndIdx > 0);
		}
		currentSyntax = currentSyntax->parentSyntaxNULL;
	}
	if(!kStmt_IsERR(stmt)) {
		DBG_ASSERT(errRule[0] != NULL);
//		KdumpTokenArray(kctx, tokenList, beginIdx, endIdx);
//		KdumpTokenArray(kctx, stmtSyntax->syntaxPatternListNULL, 0, kArray_size(stmtSyntax->syntaxPatternListNULL));
#ifdef USE_SMALLBULD
		kStmt_Message(kctx, stmt, ErrTag, "%s%s: %s%s is expected", KWSTMT_t(stmt->syn->keyword), KSymbol_Fmt2(errRule[0]->resolvedSymbol));
#else
		if(errRule[1] != NULL) {
			kStmt_Message(kctx, stmt, ErrTag, "%s%s: %s%s is expected before %s", KWSTMT_t(stmt->syn->keyword), KSymbol_Fmt2(errRule[0]->resolvedSymbol), KToken_t(errRule[1]));
		} else {
			kStmt_Message(kctx, stmt, ErrTag, "%s%s: %s%s is expected", KWSTMT_t(stmt->syn->keyword), KSymbol_Fmt2(errRule[0]->resolvedSymbol));
		}
#endif
	}
	return -1;
}

static int KTokenSeq_SkipStatementSeparator(KTokenSeq *tokens, int currentIdx)
{
	for(; currentIdx < tokens->endIdx; currentIdx++) {
		kToken *tk = tokens->tokenList->TokenItems[currentIdx];
		if(!kToken_Is(StatementSeparator, tk)) {
			break;
		}
	}
	tokens->beginIdx = currentIdx;
	return currentIdx;
}

static int KTokenSeq_SkipAnnotation(KonohaContext *kctx, KTokenSeq *tokens, int currentIdx)
{
	int count = 0;
	for(; currentIdx < tokens->endIdx; currentIdx++) {
		kToken *tk = tokens->tokenList->TokenItems[currentIdx];
		if(kToken_IsIndent(tk)) continue;
		if(KSymbol_IsAnnotation(tk->resolvedSymbol)) {
			count++;
			if(currentIdx + 1 < tokens->endIdx) {
				kToken *nextToken = tokens->tokenList->TokenItems[currentIdx+1];
				if(nextToken->resolvedSyntaxInfo != NULL && nextToken->resolvedSyntaxInfo->keyword == KSymbol_ParenthesisGroup) {
					currentIdx++;
				}
			}
			continue;
		}
		if(kToken_Is(StatementSeparator, tk)) {
			tokens->beginIdx = currentIdx+1;
			continue;
		}
		break;
	}
	return currentIdx;
}

static int kStmt_AddAnnotation(KonohaContext *kctx, kStmtVar *stmt, KTokenSeq *range, int currentIdx, int *indentRef)
{
	for(currentIdx = range->beginIdx; currentIdx < range->endIdx; currentIdx++) {
		kToken *tk = range->tokenList->TokenItems[currentIdx];
		if(kToken_IsIndent(tk)) {
			indentRef[0] = tk->indent;
			stmt->uline = tk->uline;
			continue;
		}
		if(!KSymbol_IsAnnotation(tk->resolvedSymbol)) break;
		if(currentIdx + 1 < range->endIdx) {
			kToken *nextToken = range->tokenList->TokenItems[currentIdx+1];
			kObject *value = UPCAST(K_TRUE);
			if(nextToken->resolvedSyntaxInfo != NULL && nextToken->resolvedSyntaxInfo->keyword == KSymbol_ParenthesisGroup) {
				value = (kObject *)kStmt_ParseExpr(kctx, stmt, nextToken->subTokenList, 0, kArray_size(nextToken->subTokenList), "(");
				currentIdx++;
			}
			if(value != NULL) {
				KLIB kObjectProto_SetObject(kctx, stmt, tk->resolvedSymbol, kObject_typeId(value), value);
			}
		}
	}
	return currentIdx;
}

static kbool_t kBlock_AddNewStmt(KonohaContext *kctx, kBlock *bk, KTokenSeq *tokens)
{
	//KdumpTokenArray(kctx, tokens->tokenList, tokens->beginIdx, tokens->endIdx);
	int currentIdx = KTokenSeq_SkipStatementSeparator(tokens, tokens->beginIdx);
	currentIdx = KTokenSeq_SkipAnnotation(kctx, tokens, currentIdx);
	if(currentIdx < tokens->endIdx) {
		int indent = 0;
		kStmtVar *stmt = new_(StmtVar, 0, bk->StmtList);
		KFieldInit(stmt, stmt->parentBlockNULL, bk);
		kStmt_AddAnnotation(kctx, stmt, tokens, tokens->beginIdx, &indent);
		if(stmt->uline == 0) {
			stmt->uline = tokens->tokenList->TokenItems[currentIdx]->uline;
		}
		currentIdx = kStmt_ParseBySyntaxPattern(kctx, stmt, indent, tokens->tokenList, currentIdx, tokens->endIdx);
		if(currentIdx == -1) {
			DBG_ASSERT(kStmt_IsERR(stmt));
			tokens->beginIdx = tokens->endIdx;
			return false;
		}
	}
	tokens->beginIdx = KTokenSeq_SkipStatementSeparator(tokens, currentIdx);
	return true;
}

static kBlock *new_kBlock(KonohaContext *kctx, kStmt *parent, KMacroSet *macro, KTokenSeq *source)
{
	kBlockVar *bk = /*G*/new_(BlockVar, source->ns, OnGcStack);
	if(parent != NULL) {
		KFieldInit(bk, bk->parentStmtNULL, parent);
	}
	KTokenSeq tokens = {source->ns, source->tokenList, kArray_size(source->tokenList)};
	source->SourceConfig.openToken = NULL;
	source->SourceConfig.stopChar = 0;
	KTokenSeq_Preprocess(kctx, &tokens, macro, source, source->beginIdx);
	while(tokens.beginIdx < tokens.endIdx) {
		kBlock_AddNewStmt(kctx, bk, &tokens);
	}
	return (kBlock *)bk;
}

static kBlock* kStmt_GetBlock(KonohaContext *kctx, kStmt *stmt, kNameSpace *ns, ksymbol_t kw, kBlock *def)
{
	kBlock *bk = (kBlock *)kStmt_GetObjectNULL(kctx, stmt, kw);
	if(bk == NULL) return def;
	if(IS_Token(bk)) {
		kToken *tk = (kToken *)bk;
		if(ns == NULL) ns = Stmt_ns(stmt);
		if(tk->resolvedSyntaxInfo->keyword == TokenType_CODE) {
			kToken_ToBraceGroup(kctx, (kTokenVar *)tk, ns, NULL);
		}
		if(tk->resolvedSyntaxInfo->keyword == KSymbol_BraceGroup) {
			KTokenSeq range = {ns, tk->subTokenList, 0, kArray_size(tk->subTokenList)};
			bk = new_kBlock(kctx, stmt, NULL, &range);
			KLIB kObjectProto_SetObject(kctx, stmt, kw, KType_Block, bk);
		}
	}
	return (IS_Block(bk)) ? bk : def;
}

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
	tk->resolvedSyntaxInfo = KSyntax_(tokens->ns, KSymbol_AST);
	KFieldSet(tk, tk->subTokenList, new_(TokenArray, 0, OnField));
	KTokenSeq nestedSourceRange = {tokens->ns, tokens->tokenList, currentIdx+1, ne};
	return kArray_AddSyntaxPattern(kctx, tk->subTokenList, &nestedSourceRange) ? ne : tokens->endIdx;
}

static kbool_t kArray_AddSyntaxPattern(KonohaContext *kctx, kArray *patternList, KTokenSeq *patterns)
{
	int i;
	ksymbol_t stmtEntryKey = 0;
	kTokenVar *tk, *prevToken = NULL;
	//KdumpTokenArray(kctx, rules->tokenList, rules->beginIdx, rules->endIdx);
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
			KTokenSeq nestedSourceRange = {patterns->ns, tk->subTokenList, 0, kArray_size(tk->subTokenList)};
			tk->resolvedSymbol = KSymbol_OptionalGroup;
			PUSH_GCSTACK2(tk->subTokenList);  // avoid gc
			KFieldSet(tk, tk->subTokenList, new_(TokenArray, 0, OnField));
			kArray_AddSyntaxPattern(kctx, tk->subTokenList, &nestedSourceRange);
			KLIB kArray_Add(kctx, patternList, tk);
			continue;
		}
		if(tk->hintChar == '$' && i+1 < patterns->endIdx) {  // $PatternName
			tk = patterns->tokenList->TokenVarItems[++i];
			if(IS_String(tk->text)) {
				tk->resolvedSymbol = KAsciiSymbol(kString_text(tk->text), kString_size(tk->text), KSymbol_NewRaw) | KSymbolAttr_Pattern;
				if(stmtEntryKey == 0) stmtEntryKey = tk->resolvedSymbol;
				tk->stmtEntryKey = stmtEntryKey;
				tk->resolvedSyntaxInfo = KSyntax_(patterns->ns, tk->resolvedSymbol/*KSymbol_SymbolPattern*/);
				stmtEntryKey = 0;
				KLIB kArray_Add(kctx, patternList, tk);
				continue;
			}
		}
		if(i + 1 < patterns->endIdx && patterns->tokenList->TokenItems[i+1]->hintChar == ':' && IS_String(tk->text)) {
			stmtEntryKey = KAsciiSymbol(kString_text(tk->text), kString_size(tk->text), KSymbol_NewRaw);
			i++;
			continue;
		}
		if(tk->hintChar == '*' && prevToken != NULL) {
			kToken_Set(MatchPreviousPattern, prevToken, true);
			tk = NULL;
			continue;
		}
		kToken_ToError(kctx, tk, ErrTag, "illegal syntax pattern: %s", KToken_t(tk));
		return false;
	}
	return true;
}

static void kNameSpace_ParseSyntaxPattern(KonohaContext *kctx, kNameSpace *ns, const char *ruleSource, kfileline_t uline, kArray *patternList)
{
	KTokenSeq source = {ns, GetSugarContext(kctx)->preparedTokenList};
	KTokenSeq_Push(kctx, source);
	KTokenSeq_Tokenize(kctx, &source, ruleSource, uline);
	KTokenSeq patterns = {ns, source.tokenList, source.endIdx};
	patterns.TargetPolicy.RemovingIndent = true;
	KTokenSeq_Preprocess(kctx, &patterns, NULL, &source, source.beginIdx);
	KLIB kArray_Add(kctx, patternList, K_NULLTOKEN);  // delim
	size_t firstPatternIdx = kArray_size(patternList);
	kArray_AddSyntaxPattern(kctx, patternList, &patterns);
	if(firstPatternIdx < kArray_size(patternList)) {
		kToken *firstPattern = patternList->TokenItems[firstPatternIdx];
		//DBG_P(">>>>>> firstPattern=%d", kToken_IsFirstPattern(firstPattern));
		if(kToken_IsFirstPattern(firstPattern)) {
			kNameSpace_AppendArrayRef(kctx, ns, &((kNameSpaceVar *)ns)->stmtPatternListNULL_OnList, UPCAST(firstPattern));
		}
	}
	KTokenSeq_Pop(kctx, source);
	//KdumpTokenArray(kctx, patternList, 0, kArray_size(patternList));
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
