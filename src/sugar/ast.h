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

static kExpr *callFuncExpression(KonohaContext *kctx, SugarSyntax *syn, kFunc *fo, int *countRef, kStmt *stmt, kArray *tokenList, int beginIdx, int operatorIdx, int endIdx)
{
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 6);
	lsfp[K_CALLDELTA+0].unboxValue = (uintptr_t)syn;
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+0].o, fo->self, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+1].o, (kObject*)stmt, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+2].asArray, tokenList, GC_NO_WRITE_BARRIER);
	lsfp[K_CALLDELTA+3].intValue = beginIdx;
	lsfp[K_CALLDELTA+4].intValue = operatorIdx;
	lsfp[K_CALLDELTA+5].intValue = endIdx;
	countRef[0] += 1;
	{
		KonohaStack *sfp = lsfp + K_CALLDELTA;
		KSetMethodCallStack(sfp, 0/*UL*/, fo->mtd, 5, K_NULLEXPR);
		KonohaRuntime_callMethod(kctx, sfp);
	}
	END_LOCAL();
	DBG_ASSERT(IS_Expr(lsfp[0].asObject));
	return lsfp[0].asExpr;
}

static kExpr *kStmt_parseOperatorExpr(KonohaContext *kctx, kStmt *stmt, SugarSyntax *exprSyntax, kArray *tokenList, int beginIdx, int operatorIdx, int endIdx)
{
	int callCount = 0;
	SugarSyntax *currentSyntax = exprSyntax;
	while(true) {
		int index, size;
		kFunc **funcItems = SugarSyntax_funcTable(kctx, currentSyntax, SugarFunc_Expression, &size);
		for(index = size - 1; index >= 0; index--) {
			DBG_ASSERT(IS_Func(funcItems[index]));
			kExpr *texpr = callFuncExpression(kctx, exprSyntax, funcItems[index], &callCount, stmt, tokenList, beginIdx, operatorIdx, endIdx);
			if(Stmt_isERR(stmt)) return K_NULLEXPR;
			if(texpr != K_NULLEXPR) return texpr;
		}
		if(currentSyntax->parentSyntaxNULL == NULL) break;
		currentSyntax = currentSyntax->parentSyntaxNULL;
	}
	const char *emesg = (callCount > 0) ? "syntax error: expression %s" : "undefined expression: %s";
	kStmt_printMessage(kctx, stmt, ErrTag, emesg, Token_text(tokenList->tokenItems[operatorIdx]));
	return K_NULLEXPR;
}

static int kStmt_findOperator(KonohaContext *kctx, kStmt *stmt, kArray *tokenList, int beginIdx, int endIdx)
{
	int isPrePosition = true;
	int idx = beginIdx, i, precedence = 0;
	for(i = beginIdx; i < endIdx; i++) {
		kToken *tk = tokenList->tokenItems[i];
		SugarSyntax *syn = tk->resolvedSyntaxInfo;
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

static kExpr* kStmt_parseExpr(KonohaContext *kctx, kStmt *stmt, kArray *tokenList, int beginIdx, int endIdx, const char *hintBeforeText)
{
	if(!Stmt_isERR(stmt)) {
		if(beginIdx < endIdx) {
			int idx = kStmt_findOperator(kctx, stmt, tokenList, beginIdx, endIdx);
			SugarSyntax *syn = tokenList->tokenItems[idx]->resolvedSyntaxInfo;
			return kStmt_parseOperatorExpr(kctx, stmt, syn, tokenList, beginIdx, idx, endIdx);
		}
		else {
			if(hintBeforeText == NULL) hintBeforeText = "";
			kStmt_printMessage(kctx, stmt, ErrTag, "expected expression after %s", hintBeforeText);
		}
	}
	return K_NULLEXPR;
}

static kExpr *kStmt_addExprParam(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kArray *tokenList, int s, int e, const char *hintBeforeText/* if NULL empty isAllowed */)
{
	int i, start = s;
	for(i = s; i < e; i++) {
		kToken *tk = tokenList->tokenItems[i];
		if(tk->resolvedSyntaxInfo->keyword == KW_COMMA) {
			if(start < i || hintBeforeText != NULL) {
				expr = Expr_add(kctx, expr, kStmt_parseExpr(kctx, stmt, tokenList, start, i, hintBeforeText));
				if(hintBeforeText != NULL) hintBeforeText = ",";
			}
			start = i + 1;
		}
	}
	if(start < i || hintBeforeText != NULL) {
		expr = Expr_add(kctx, expr, kStmt_parseExpr(kctx, stmt, tokenList, start, i, hintBeforeText));
	}
	return expr;
}

static kExpr *kStmt_rightJoinExpr(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kArray *tokenList, int c, int e)
{
	if(c < e && expr != K_NULLEXPR && !Stmt_isERR(stmt)) {
		kToken *tk = tokenList->tokenItems[c];
		if(tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern || tk->resolvedSyntaxInfo->sugarFuncTable[SugarFunc_Expression] == NULL) {
			DBG_ASSERT(c > 1);
			kToken *previousToken = tokenList->tokenItems[c-1];
			const char *white = kToken_is(BeforeWhiteSpace, previousToken) ? " " : "";
			kStmtToken_printMessage(kctx, stmt, tk, ErrTag, "undefined syntax: %s%s%s ...", Token_text(previousToken), white, Token_text(tk));
			return K_NULLEXPR;
		}
		kStmtToken_printMessage(kctx, stmt, tk, WarnTag, "ignored term: %s...", Token_text(tk));
	}
	return expr;
}

/* ------------------------------------------------------------------------ */
/* new ast parser */

static int TokenUtils_skipIndent(kArray *tokenList, int currentIdx, int endIdx)
{
	for(; currentIdx < endIdx; currentIdx++) {
		kToken *tk = tokenList->tokenItems[currentIdx];
		if(kToken_is(StatementSeparator, tk)) continue;
		if(!kToken_isIndent(tk)) break;
	}
	return currentIdx;
}

static int TokenUtils_parseTypePattern(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx, KonohaClass **classRef);
static KonohaClass* TokenUtils_parseGenericsType(KonohaContext *kctx, kNameSpace *ns, KonohaClass *baseClass, kArray *tokenList, int beginIdx, int endIdx)
{
	size_t currentIdx = beginIdx, psize = 0;
	kparamtype_t *p = ALLOCA(kparamtype_t, endIdx);
	while(currentIdx < endIdx) {
		KonohaClass *paramClass = NULL;
		currentIdx = TokenUtils_parseTypePattern(kctx, ns, tokenList, currentIdx, endIdx, &paramClass);
		if(paramClass == NULL) {
			return NULL;
		}
		p[psize].ty = paramClass->typeId;
		psize++;
		if(currentIdx < endIdx && tokenList->tokenItems[currentIdx]->resolvedSyntaxInfo->keyword == KW_COMMA) {
			currentIdx++;
		}
	}
	if(baseClass->baseTypeId == TY_Func) {
		return KLIB KonohaClass_Generics(kctx, baseClass, p[0].ty, psize-1, p+1);
	}
	else {
		return KLIB KonohaClass_Generics(kctx, baseClass, TY_void, psize, p);
	}
}

static int TokenUtils_parseTypePattern(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx, KonohaClass **classRef)
{
	int nextIdx = -1;
	kToken *tk = tokenList->tokenItems[beginIdx];
	KonohaClass *foundClass = NULL;
	if(tk->resolvedSyntaxInfo->keyword == KW_TypePattern) {
		foundClass = CT_(tk->resolvedTypeId);
		nextIdx = beginIdx + 1;
	}
	else if(tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern) { // check
		foundClass = KLIB kNameSpace_getClass(kctx, ns, S_text(tk->text), S_size(tk->text), NULL);
		if(foundClass != NULL) {
			//kToken_setTypeId(kctx, tk, ns, foundClass->typeId);
			nextIdx = beginIdx + 1;
		}
	}
	if(foundClass != NULL) {
		int isAllowedGenerics = true;
		for(; nextIdx < endIdx; nextIdx++) {
			tk = tokenList->tokenItems[nextIdx];
			if(tk->resolvedSyntaxInfo == NULL || tk->resolvedSyntaxInfo->keyword != KW_BracketGroup) {
				break;
			}
			int sizeofBracketTokens = kArray_size(tk->subTokenList);
			if(isAllowedGenerics &&  sizeofBracketTokens > 0) {  // C[T][]
				KonohaClass *foundGenericClass = TokenUtils_parseGenericsType(kctx, ns, foundClass, tk->subTokenList, 0, sizeofBracketTokens);
				if(foundGenericClass == NULL) break;
				foundClass = foundGenericClass;
			}
			else {
				if(sizeofBracketTokens > 0) break;   // C[100] is treated as C  and the token [100] is set to nextIdx;
				foundClass = CT_p0(kctx, CT_Array, foundClass->typeId);  // C[] => Array[C]
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

static int TokenSequence_resolved2(KonohaContext *kctx, TokenSequence *tokens, MacroSet *, TokenSequence *source, int beginIdx);

static kbool_t TokenSequence_expandMacro(KonohaContext *kctx, TokenSequence *tokens, ksymbol_t symbol, MacroSet *macroParam)
{
	while(macroParam->symbol != 0) {
		if(macroParam->symbol == symbol) {
			TokenSequence paramtokens = {tokens->ns, macroParam->tokenList, macroParam->beginIdx, macroParam->endIdx};
			TokenSequence_resolved2(kctx, tokens, NULL, &paramtokens, macroParam->beginIdx);
			return true;
		}
		macroParam++;
	}
	return false;
}

static kArray* kArray_slice(KonohaContext *kctx, kArray *a, int beginIdx, int endIdx)
{
	kArray *newa = new_(Array, endIdx - beginIdx);
	int i;
	for(i = beginIdx; i < endIdx; i++) {
		KLIB kArray_add(kctx, newa, a->objectItems[i]);
	}
	return newa;
}

static kTokenVar* kToken_expandGroupMacro(KonohaContext *kctx, kTokenVar *tk, kNameSpace *ns, MacroSet *macroParam)
{
	TokenSequence source = {ns, tk->subTokenList, 0, kArray_size(tk->subTokenList)};
	if(source.endIdx > 0) {
		int isChanged = true;
		TokenSequence group = {ns, KonohaContext_getSugarContext(kctx)->preparedTokenList};
		TokenSequence_push(kctx, group);
		TokenSequence_resolved2(kctx, &group, macroParam, &source, source.beginIdx);
		if(group.endIdx - group.beginIdx == source.endIdx) {
			int i;
			isChanged = false;
			for(i = 0; i < source.endIdx; i++) {
				if(source.tokenList->tokenItems[i] != group.tokenList->tokenItems[group.beginIdx+i]) {
					isChanged = true;
				}
			}
		}
		if(isChanged) {
			kTokenVar *groupToken = new_(TokenVar, tk->resolvedSymbol);
			KSETv(groupToken, groupToken->subTokenList, kArray_slice(kctx, group.tokenList, group.beginIdx, group.endIdx));
			groupToken->resolvedSyntaxInfo = tk->resolvedSyntaxInfo;
			groupToken->uline = tk->uline;
			tk = groupToken;
		}
		TokenSequence_pop(kctx, group);
	}
	return tk;
}

static kbool_t TokenSequence_applyMacro(KonohaContext *kctx, TokenSequence *tokens, kArray *macroTokenList, int beginIdx, int endIdx, size_t paramsize, MacroSet *macroParam)
{
	TokenSequence macro = {tokens->ns, macroTokenList, beginIdx + paramsize, endIdx};
	KdumpTokenArray(kctx, macro.tokenList, macro.beginIdx, macro.endIdx);
	int dstart = kArray_size(tokens->tokenList);
	TokenSequence_resolved2(kctx, tokens, macroParam, &macro, beginIdx + paramsize);
	DBG_P("dstart=%d, tokens->begin,end=%d, %d", dstart, tokens->beginIdx, tokens->endIdx);
	KdumpTokenArray(kctx, tokens->tokenList, dstart, tokens->endIdx);
	return true;
}

static void TokenSequence_applyMacroGroup(KonohaContext *kctx, TokenSequence *tokens, kArray *macroTokenList, int paramsize, kToken *groupToken)
{
	int i;
	MacroSet* mp = ALLOCA(MacroSet, paramsize+1);
	DBG_ASSERT(paramsize < kArray_size(macroTokenList));
	for(i = 0; i < paramsize; i++) {
		mp[i].symbol = macroTokenList->tokenItems[i]->resolvedSymbol;
		mp[i].tokenList = groupToken->subTokenList;
	}
	mp[paramsize].symbol = 0; /* sentinel */

	int p = 0, start = 0;
	for(i = 0; i < kArray_size(groupToken->subTokenList); i++) {
		kToken *tk = groupToken->subTokenList->tokenItems[i];
		if(tk->topCharHint == ',') {
			mp[p].beginIdx = start;
			mp[p].endIdx = i;
			p++;
			start = i + 1;
		}
	}
	DBG_ASSERT(p < paramsize);
	mp[p].beginIdx = start;
	mp[p].endIdx = kArray_size(groupToken->subTokenList);
	TokenSequence_applyMacro(kctx, tokens, macroTokenList, 0, kArray_size(macroTokenList), paramsize, mp);
}

static void kNameSpace_setMacroData(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, int paramsize, const char *data)
{
	SugarSyntaxVar *syn = (SugarSyntaxVar *)SUGAR kNameSpace_getSyntax(kctx, ns, keyword, /*new*/true);
	TokenSequence source = {ns, KonohaContext_getSugarContext(kctx)->preparedTokenList};
	TokenSequence_push(kctx, source);
	TokenSequence_tokenize(kctx, &source, data, 0);
	TokenSequence tokens = {source.ns, source.tokenList, source.endIdx};
	tokens.TargetPolicy.ExpandingBraceGroup = true;
	TokenSequence_resolved2(kctx, &tokens, NULL, &source, source.beginIdx);
	syn->macroParamSize = paramsize;
	KINITSETv(ns, syn->macroDataNULL, kArray_slice(kctx, tokens.tokenList, tokens.beginIdx + 1 /* removing head indent*/, tokens.endIdx));
	TokenSequence_pop(kctx, source);
}

/* ------------------------------------------------------------------------ */

static int TokenSequence_addGroup(KonohaContext *kctx, TokenSequence *tokens, MacroSet *macro, TokenSequence *source, int currentIdx, kToken *openToken)
{
	ksymbol_t AST_type = openToken->topCharHint == '(' ?  KW_ParenthesisGroup : KW_BracketGroup;
	int closech = (AST_type == KW_ParenthesisGroup) ? ')': ']';
	kTokenVar *astToken = new_(TokenVar, AST_type);
	KLIB kArray_add(kctx, tokens->tokenList, astToken);
	astToken->resolvedSyntaxInfo = SYN_(tokens->ns, AST_type);
	KSETv(astToken, astToken->subTokenList, new_(TokenArray, 0));
	astToken->uline = openToken->uline;
	{
		TokenSequence nested = {source->ns, astToken->subTokenList};
		kToken *pushOpenToken = source->SourceConfig.openToken;
		int pushStopChar = source->SourceConfig.stopChar;
		source->SourceConfig.openToken = openToken;
		source->SourceConfig.stopChar = closech;
		nested.TargetPolicy.RemovingIndent = true;
		int returnIdx = TokenSequence_resolved2(kctx, &nested, macro, source, currentIdx);
		source->SourceConfig.openToken = pushOpenToken;
		source->SourceConfig.stopChar = pushStopChar;
		return returnIdx;
	}
}

static kTokenVar* kToken_transformToBraceGroup(KonohaContext *kctx, kTokenVar *tk, kNameSpace *ns, MacroSet *macroSet)
{
	TokenSequence source = {ns, KonohaContext_getSugarContext(kctx)->preparedTokenList};
	TokenSequence_push(kctx, source);
	KdumpToken(kctx, tk);
	TokenSequence_tokenize(kctx, &source, S_text(tk->text), tk->uline);
	KSETv(tk, tk->subTokenList, new_(TokenArray, 0));
	tk->resolvedSyntaxInfo = SYN_(ns, KW_BraceGroup);
	TokenSequence tokens = {ns, tk->subTokenList, 0};
	TokenSequence_resolved2(kctx, &tokens, macroSet, &source, source.beginIdx);
	TokenSequence_pop(kctx, source);
	return tk;
}

static int TokenUtils_count(kArray *tokenList, int beginIdx, int endIdx, ksymbol_t keyword)
{
	int i, count = 0;
	for(i = beginIdx; i < endIdx; i++) {
		kToken *tk = tokenList->tokenItems[i];
		if(tk->resolvedSyntaxInfo->keyword == keyword) count++;
	}
	return count;
}

static kToken* new_CommaToken(KonohaContext *kctx)
{
	kTokenVar *tk = new_(TokenVar, KW_COMMA);
	tk->topCharHint = ',';
	return tk;
}

static int TokenSequence_applyMacroSyntax(KonohaContext *kctx, TokenSequence *tokens, SugarSyntax *syn, MacroSet *macroParam, TokenSequence *source, int currentIdx)
{
	if(syn->macroParamSize == 0) {
		TokenSequence_applyMacro(kctx, tokens, syn->macroDataNULL, 0, kArray_size(syn->macroDataNULL), 0, NULL);
		return currentIdx;
	}
	TokenSequence dummy = {tokens->ns, kctx->stack->gcstack};
	TokenSequence_push(kctx, dummy);
	int nextIdx = TokenUtils_skipIndent(source->tokenList, currentIdx, source->endIdx);
	if(nextIdx < source->endIdx) {
		kToken *groupToken = source->tokenList->tokenItems[nextIdx];
		if(groupToken->topCharHint == '(') {
			nextIdx = TokenSequence_addGroup(kctx, &dummy, macroParam, source, nextIdx+1, groupToken);
			if(source->SourceConfig.foundErrorToken != NULL) {
				return source->endIdx;
			}
			groupToken = dummy.tokenList->tokenItems[dummy.beginIdx];
		}
		DBG_ASSERT(groupToken->resolvedSyntaxInfo->keyword != KW_ParenthesisGroup);
		int count = TokenUtils_count(groupToken->subTokenList, 0, kArray_size(groupToken->subTokenList), KW_COMMA);
		if(syn->macroParamSize == count + 2) {
			nextIdx = TokenUtils_skipIndent(source->tokenList, nextIdx, source->endIdx);
			if(nextIdx < source->endIdx) {
				kTokenVar *tk = source->tokenList->tokenVarItems[nextIdx];
				if(tk->unresolvedTokenType == TokenType_CODE) {
					tk = kToken_transformToBraceGroup(kctx, tk, tokens->ns, macroParam);
					KLIB kArray_add(kctx, groupToken->subTokenList, new_CommaToken(kctx));
					KLIB kArray_add(kctx, groupToken->subTokenList, tk);
					count++;
				}
			}
		}
		if(syn->macroParamSize == count + 1) {
			TokenSequence_applyMacroGroup(kctx, tokens, syn->macroDataNULL, syn->macroParamSize, groupToken);
		}
		else {
			kTokenVar *tk = source->tokenList->tokenVarItems[currentIdx];
			kToken_printMessage(kctx, tk, ErrTag, "macro expects %d parameter(s)", (int)syn->macroParamSize);
			source->SourceConfig.foundErrorToken = tk;
			nextIdx = source->endIdx;
		}
	}
	TokenSequence_pop(kctx, dummy);
	return nextIdx;
}

static int TokenSequence_resolved2(KonohaContext *kctx, TokenSequence *tokens, MacroSet *macroParam, TokenSequence *source, int beginIdx)
{
	int currentIdx = beginIdx;
	if(tokens->TargetPolicy.syntaxSymbolPattern == NULL) {
		tokens->TargetPolicy.syntaxSymbolPattern = SYN_(tokens->ns, KW_SymbolPattern);
	}
	for(; currentIdx < source->endIdx; currentIdx++) {
		kTokenVar *tk = source->tokenList->tokenVarItems[currentIdx];
		if(kToken_isIndent(tk) && tokens->TargetPolicy.RemovingIndent) {
			continue;  // remove INDENT in () or []
		}
		if(tk->unresolvedTokenType == TokenType_CODE && (tokens->TargetPolicy.ExpandingBraceGroup || macroParam != NULL)) {
			tk = kToken_transformToBraceGroup(kctx, tk, tokens->ns, macroParam);
			KLIB kArray_add(kctx, tokens->tokenList, tk);
			continue;
		}
		if(macroParam != NULL && tk->resolvedSyntaxInfo != NULL) {
			ksymbol_t keyword = tk->resolvedSyntaxInfo->keyword;
			if(keyword == KW_SymbolPattern && TokenSequence_expandMacro(kctx, tokens, tk->resolvedSymbol, macroParam)) {
				continue;
			}
			if(keyword == KW_ParenthesisGroup || keyword == KW_BracketGroup || keyword == KW_BraceGroup) {
				tk = kToken_expandGroupMacro(kctx, tk, tokens->ns, macroParam);
			}
			if(keyword == TokenType_CODE) {
				tk = kToken_transformToBraceGroup(kctx, tk, tokens->ns, macroParam);
			}
		}
		if(tk->resolvedSyntaxInfo == NULL) {
			if(source->SourceConfig.openToken != NULL && source->SourceConfig.stopChar == tk->topCharHint) {
				return currentIdx;
			}
			if(tk->topCharHint == '(' || tk->topCharHint == '[') {
				currentIdx = TokenSequence_addGroup(kctx, tokens, macroParam, source, currentIdx+1, tk);
				if(source->SourceConfig.foundErrorToken != NULL) return source->endIdx;
				continue; // already added
			}
			if(tk->unresolvedTokenType == TokenType_ERR) {
				source->SourceConfig.foundErrorToken = tk;
				return source->endIdx;  // resolved no more
			}
			if(tk->unresolvedTokenType == TokenType_SYMBOL) {
				const char *t = S_text(tk->text);
				ksymbol_t symbol = ksymbolA(t, S_size(tk->text), SYM_NEWID);
				if(macroParam != NULL && TokenSequence_expandMacro(kctx, tokens, symbol, macroParam)) {
					continue;
				}
				SugarSyntax *syn = SYN_(source->ns, symbol);
				if(syn != NULL && FLAG_is(syn->flag, SYNFLAG_Macro)) {
					currentIdx = TokenSequence_applyMacroSyntax(kctx, tokens, syn, macroParam, source, currentIdx);
					continue;
				}
				tk->resolvedSymbol = symbol;
				tk->resolvedSyntaxInfo = (syn != NULL) ? syn : tokens->TargetPolicy.syntaxSymbolPattern;
			}
			else {
				tk->resolvedSyntaxInfo = SYN_(tokens->ns, tk->unresolvedTokenType);
				if(!kToken_is(StatementSeparator, tk) && tk->unresolvedTokenType != TokenType_INDENT) {
					DBG_ASSERT(tk->resolvedSyntaxInfo != NULL);
				}
			}
		}
		KLIB kArray_add(kctx, tokens->tokenList, tk);
	}
	if(source->SourceConfig.openToken != NULL) {
		char buf[2] = {source->SourceConfig.stopChar, 0};
		ERROR_UnclosedToken(kctx, (kTokenVar*)source->SourceConfig.openToken, (const char*)buf);
		source->SourceConfig.foundErrorToken = source->SourceConfig.openToken;
	}
	TokenSequence_end(kctx, tokens);
	return source->endIdx;
}

/* ------------------------------------------------------------------------ */

static int callPatternMatchFunc(KonohaContext *kctx, kFunc *fo, int *countRef, kStmt *stmt, ksymbol_t name, kArray *tokenList, int beginIdx, int endIdx)
{
	INIT_GCSTACK();
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 5);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+0].o, fo->self, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+1].o, (kObject*)stmt, GC_NO_WRITE_BARRIER);
	lsfp[K_CALLDELTA+2].intValue = name;
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+3].asArray, tokenList, GC_NO_WRITE_BARRIER);
	lsfp[K_CALLDELTA+4].intValue = beginIdx;
	lsfp[K_CALLDELTA+5].intValue = endIdx;
	countRef[0] += 1;
	{
		KonohaStack *sfp = lsfp + K_CALLDELTA;
		KSetMethodCallStack(sfp, 0/*UL*/, fo->mtd, 5, KLIB Knull(kctx, CT_Int));
		KonohaRuntime_callMethod(kctx, sfp);
	}
	END_LOCAL();
	RESET_GCSTACK();
	return (int)lsfp[0].intValue;
}

static int SugarSyntax_matchPattern(KonohaContext *kctx, SugarSyntax *syn, kToken *patternToken, kStmt *stmt, int name, kArray *tokenList, int beginIdx, int endIdx)
{
	int callCount = 0;
	if(syn != NULL) {
		while(true) {
			int index, size;
			kFunc **funcItems = SugarSyntax_funcTable(kctx, syn, SugarFunc_PatternMatch, &size);
			for(index = size - 1; index >= 0; index--) {
				int next = callPatternMatchFunc(kctx, funcItems[index], &callCount, stmt, name, tokenList, beginIdx, endIdx);
				if(Stmt_isERR(stmt)) return -1;
				if(next >= beginIdx) return next;
			}
			if(syn->parentSyntaxNULL == NULL) break;
			syn = syn->parentSyntaxNULL;
		}
	}
	if(callCount == 0) {
		kStmtToken_printMessage(kctx, stmt, patternToken, ErrTag, "undefined syntax pattern: %s%s", PSYM_t(patternToken->resolvedSymbol));
	}
	return -1;
}

static int kStmt_matchSyntaxPattern(KonohaContext *kctx, kStmt *stmt, TokenSequence *tokens, TokenSequence *patterns, kToken **errRuleRef)
{
	int patternIdx = patterns->beginIdx, tokenIdx = tokens->beginIdx;
	kNameSpace *ns = Stmt_nameSpace(stmt);
//	KdumpTokenArray(kctx, patterns->tokenList, patterns->beginIdx, patterns->endIdx);
//	KdumpTokenArray(kctx, tokens->tokenList, tokens->beginIdx, tokens->endIdx);
	for(; patternIdx < patterns->endIdx; patternIdx++) {
		kToken *ruleToken = patterns->tokenList->tokenItems[patternIdx];
		L_ReDo:;
		tokenIdx = TokenUtils_skipIndent(tokens->tokenList, tokenIdx, tokens->endIdx);
		DBG_P("patternIdx=%d, tokenIdx=%d, tokenEndIdx=%d", patternIdx, tokenIdx, tokens->endIdx);
		if(tokenIdx < tokens->endIdx) {
			kToken *tk = tokens->tokenList->tokenItems[tokenIdx];
			errRuleRef[1] = tk;
			if(KW_isPATTERN(ruleToken->resolvedSymbol)) {
				SugarSyntax *syn = SYN_(ns, ruleToken->resolvedSymbol);
				tokenIdx = SugarSyntax_matchPattern(kctx, syn, ruleToken, stmt, ruleToken->stmtEntryKey, tokens->tokenList, tokenIdx, tokens->endIdx);
				if(tokenIdx == -1 && !kToken_is(MatchPreviousPattern, ruleToken)) {
					errRuleRef[0] = ruleToken;
					return -1;
				}
			}
			else if(ruleToken->resolvedSymbol == KW_OptionalGroup) {
				TokenSequence nrule = {ns, ruleToken->subTokenList, 0, kArray_size(ruleToken->subTokenList)};
				tokens->beginIdx = tokenIdx;
				int next = kStmt_matchSyntaxPattern(kctx, stmt, tokens, &nrule, errRuleRef);
				errRuleRef[0] = NULL;
				if(Stmt_isERR(stmt)) return -1;
				if(next != -1) {
					tokenIdx = next;
				}
			}
			else {
				if(ruleToken->resolvedSymbol != tk->resolvedSymbol) {
					errRuleRef[0] = ruleToken;
					return -1;
				}
				if(ruleToken->resolvedSymbol == KW_ParenthesisGroup || ruleToken->resolvedSymbol == KW_BracketGroup) {
					TokenSequence nrule = {ns, ruleToken->subTokenList, 0, kArray_size(ruleToken->subTokenList)};
					TokenSequence ntokens = {ns, tk->subTokenList, 0, kArray_size(tk->subTokenList)};
					int next = kStmt_matchSyntaxPattern(kctx, stmt, &ntokens, &nrule, errRuleRef);
					if(next == -1) {
						return -1;
					}
				}
				tokenIdx++;
			}
			if(kToken_is(MatchPreviousPattern, ruleToken)) {
				goto L_ReDo;
			}
		}
		else { /* tokenIdx < patterns->endIdx */
			while(patternIdx + 1 < patterns->endIdx) {
				if(ruleToken->resolvedSymbol != KW_OptionalGroup && !kToken_is(MatchPreviousPattern,ruleToken)) {
					errRuleRef[0] = ruleToken;
					errRuleRef[1] = NULL;
					return -1;
				}
				patternIdx++;
				ruleToken = patterns->tokenList->tokenItems[patternIdx];
			}
			return tokenIdx;
		}
	}
	return tokenIdx;
}

static SugarSyntax* kStmt_resolveStatementSyntax(KonohaContext *kctx, kStmt *stmt, kArray *tokenList, int beginIdx, int endIdx)
{
	kToken *tk = tokenList->tokenItems[beginIdx];
	SugarSyntax *syn = tk->resolvedSyntaxInfo;
	kNameSpace *ns = Stmt_nameSpace(stmt);
	//DBG_P(">>>>>>>>>>>>>>>>>>> finding SugarSyntax=%s%s syn->SyntaxPatternListNULL=%p", PSYM_t(syn->keyword), syn->SyntaxPatternListNULL);
	if(syn->SyntaxPatternListNULL == NULL) {
		kNameSpace *currentNameSpace = ns;
		while(currentNameSpace != NULL) {
			kArray *stmtPatternList = currentNameSpace->StmtPatternListNULL;
			if(stmtPatternList != NULL) {
				int i;
				for(i = kArray_size(stmtPatternList) - 1; i >=0; i--) {
					kToken *patternToken = stmtPatternList->tokenItems[i];
					//DBG_P(">>>>>>>>>> searching patternToken=%s%s", PSYM_t(patternToken->resolvedSymbol));
					if(SugarSyntax_matchPattern(kctx, patternToken->resolvedSyntaxInfo, patternToken, stmt, 0, tokenList, beginIdx, endIdx) != -1) {
						return SYN_(ns, patternToken->stmtEntryKey);
					}
				}
			}
			currentNameSpace = currentNameSpace->parentNULL;
		}
		syn = SYN_(ns, KW_ExprPattern);
	}
	return syn;
}

#define KWSTMT_t(kw)  StatementName(kctx, kw), StatementType(kw)

static const char* StatementName(KonohaContext *kctx, ksymbol_t keyword)
{
	const char *statement = SYM_t(keyword);
#ifndef USE_SMALLBUILD
	if(keyword == KW_ExprPattern) statement = "expression";
	else if(keyword == KW_TypeDeclPattern) statement = "variable";
	else if(keyword == KW_MethodDeclPattern) statement =  "function";
#endif
	return statement;
}

static const char* StatementType(ksymbol_t keyword)
{
#ifdef USE_SMALLBUILD
	return "";
#else
	const char *postfix = " statement";
	if(keyword == KW_ExprPattern) postfix = "";
	else if(keyword == KW_TypeDeclPattern || keyword == KW_MethodDeclPattern) postfix = " declaration";
	return postfix;
#endif
}

static int TokenSequence_selectSyntaxPattern(KonohaContext *kctx, TokenSequence *patterns, kArray *patternList, int endIdx)
{
	int i;
	for(i = endIdx - 1; i >= 0; i--) {
		kToken *tk = patternList->tokenItems[i];
		if(IS_NULL(tk)) {
			patterns->endIdx = endIdx;
			patterns->beginIdx = i + 1;
			return i - 1;
		}
	}
	return -1;
}

static int kStmt_parseBySyntaxPattern(KonohaContext *kctx, kStmt *stmt, int indent, kArray *tokenList, int beginIdx, int endIdx)
{
	SugarSyntax *stmtSyntax = kStmt_resolveStatementSyntax(kctx, stmt, tokenList, beginIdx, endIdx);
	((kStmtVar*)stmt)->syn = stmtSyntax;
	//DBG_P(">>>>>>>>>>>>>>>>>>> Found SugarSyntax=%s%s", KWSTMT_t(stmtSyntax->keyword));
	kToken *errRule[2];
	kNameSpace *ns = Stmt_nameSpace(stmt);
	SugarSyntax *currentSyntax = stmtSyntax;
	while(currentSyntax != NULL) {
		if(currentSyntax->SyntaxPatternListNULL != NULL) {
			int patternEndIdx = kArray_size(currentSyntax->SyntaxPatternListNULL);
			TokenSequence tokens = {ns, tokenList, beginIdx, endIdx};
			TokenSequence nrule  = {ns, currentSyntax->SyntaxPatternListNULL};
			do {
				patternEndIdx = TokenSequence_selectSyntaxPattern(kctx, &nrule, currentSyntax->SyntaxPatternListNULL, patternEndIdx);
				errRule[0] = NULL; errRule[1] = NULL;
				int nextIdx = kStmt_matchSyntaxPattern(kctx, stmt, &tokens, &nrule, errRule);
				if(Stmt_isERR(stmt)) return -1;
				if(beginIdx < nextIdx) return nextIdx;
			} while(patternEndIdx > 0);
		}
		currentSyntax = currentSyntax->parentSyntaxNULL;
	}
	if(!Stmt_isERR(stmt)) {
		DBG_ASSERT(errRule[0] != NULL);
		KdumpTokenArray(kctx, tokenList, beginIdx, endIdx);
		KdumpTokenArray(kctx, stmtSyntax->SyntaxPatternListNULL, 0, kArray_size(stmtSyntax->SyntaxPatternListNULL));
#ifdef USE_SMALLBULD
		kStmt_printMessage(kctx, stmt, ErrTag, "%s%s: %s%s is expected", KWSTMT_t(stmt->syn->keyword), PSYM_t(errRule[0]->resolvedSymbol));
#else
		if(errRule[1] != NULL) {
			kStmt_printMessage(kctx, stmt, ErrTag, "%s%s: %s%s is expected before %s", KWSTMT_t(stmt->syn->keyword), PSYM_t(errRule[0]->resolvedSymbol), Token_text(errRule[1]));
		} else {
			kStmt_printMessage(kctx, stmt, ErrTag, "%s%s: %s%s is expected", KWSTMT_t(stmt->syn->keyword), PSYM_t(errRule[0]->resolvedSymbol));
		}
#endif
	}
	return -1;
}

static int TokenSequence_skipStatementSeparator(TokenSequence *tokens, int currentIdx)
{
	for(; currentIdx < tokens->endIdx; currentIdx++) {
		kToken *tk = tokens->tokenList->tokenItems[currentIdx];
		if(!kToken_is(StatementSeparator, tk)) {
			break;
		}
	}
	tokens->beginIdx = currentIdx;
	return currentIdx;
}

static int TokenSequence_skipAnnotation(KonohaContext *kctx, TokenSequence *tokens, int currentIdx)
{
	int count = 0;
	for(; currentIdx < tokens->endIdx; currentIdx++) {
		kToken *tk = tokens->tokenList->tokenItems[currentIdx];
		if(kToken_isIndent(tk)) continue;
		if(MN_isAnnotation(tk->resolvedSymbol)) {
			count++;
			if(currentIdx + 1 < tokens->endIdx) {
				kToken *nextToken = tokens->tokenList->tokenItems[currentIdx+1];
				if(nextToken->resolvedSyntaxInfo != NULL && nextToken->resolvedSyntaxInfo->keyword == KW_ParenthesisGroup) {
					currentIdx++;
				}
			}
			continue;
		}
		if(kToken_is(StatementSeparator, tk)) {
			tokens->beginIdx = currentIdx+1;
			continue;
		}
		break;
	}
	return currentIdx;
}

static int kStmt_addAnnotation(KonohaContext *kctx, kStmtVar *stmt, TokenSequence *range, int currentIdx, int *indentRef)
{
	for(currentIdx = range->beginIdx; currentIdx < range->endIdx; currentIdx++) {
		kToken *tk = range->tokenList->tokenItems[currentIdx];
		if(kToken_isIndent(tk)) {
			indentRef[0] = tk->indent;
			stmt->uline = tk->uline;
			continue;
		}
		if(!MN_isAnnotation(tk->resolvedSymbol)) break;
		if(currentIdx + 1 < range->endIdx) {
			kToken *nextToken = range->tokenList->tokenItems[currentIdx+1];
			kObject *value = UPCAST(K_TRUE);
			if(nextToken->resolvedSyntaxInfo != NULL && nextToken->resolvedSyntaxInfo->keyword == KW_ParenthesisGroup) {
				value = (kObject*)kStmt_parseExpr(kctx, stmt, nextToken->subTokenList, 0, kArray_size(nextToken->subTokenList), "(");
				currentIdx++;
			}
			if(value != NULL) {
				KLIB kObject_setObject(kctx, stmt, tk->resolvedSymbol, O_typeId(value), value);
			}
		}
	}
	return currentIdx;
}

static kbool_t kBlock_addNewStmt(KonohaContext *kctx, kBlock *bk, TokenSequence *tokens)
{
	//KdumpTokenArray(kctx, tokens->tokenList, tokens->beginIdx, tokens->endIdx);
	int currentIdx = TokenSequence_skipStatementSeparator(tokens, tokens->beginIdx);
	currentIdx = TokenSequence_skipAnnotation(kctx, tokens, currentIdx);
	if(currentIdx < tokens->endIdx) {
		int indent = 0;
		kStmtVar *stmt = new_(StmtVar, 0);
		KLIB kArray_add(kctx, bk->stmtList, stmt);
		KINITp(stmt, stmt->parentBlockNULL, bk);
		kStmt_addAnnotation(kctx, stmt, tokens, tokens->beginIdx, &indent);
		if(stmt->uline == 0) {
			stmt->uline = tokens->tokenList->tokenItems[currentIdx]->uline;
		}
		currentIdx = kStmt_parseBySyntaxPattern(kctx, stmt, indent, tokens->tokenList, currentIdx, tokens->endIdx);
		if(currentIdx == -1) {
			DBG_ASSERT(Stmt_isERR(stmt));
			tokens->beginIdx = tokens->endIdx;
			return false;
		}
	}
	tokens->beginIdx = TokenSequence_skipStatementSeparator(tokens, currentIdx);
	return true;
}

static kBlock *new_kBlock(KonohaContext *kctx, kStmt *parent, MacroSet *macro, TokenSequence *source)
{
	kBlockVar *bk = GCSAFE_new(BlockVar, source->ns);
	if(parent != NULL) {
		KINITv(bk->parentStmtNULL, parent);
	}
	TokenSequence tokens = {source->ns, source->tokenList, kArray_size(source->tokenList)};
	source->SourceConfig.openToken = NULL;
	source->SourceConfig.stopChar = 0;
	TokenSequence_resolved2(kctx, &tokens, macro, source, source->beginIdx);
	while(tokens.beginIdx < tokens.endIdx) {
		kBlock_addNewStmt(kctx, bk, &tokens);
	}
	return (kBlock*)bk;
}

static kBlock* kStmt_getBlock(KonohaContext *kctx, kStmt *stmt, kNameSpace *ns, ksymbol_t kw, kBlock *def)
{
	kBlock *bk = (kBlock*)kStmt_getObjectNULL(kctx, stmt, kw);
	if(bk == NULL) return def;
	if(IS_Token(bk)) {
		kToken *tk = (kToken*)bk;
		if(ns == NULL) ns = Stmt_nameSpace(stmt);
		if (tk->resolvedSyntaxInfo->keyword == TokenType_CODE) {
			kToken_transformToBraceGroup(kctx, (kTokenVar*)tk, ns, NULL);
		}
		if (tk->resolvedSyntaxInfo->keyword == KW_BraceGroup) {
			TokenSequence range = {ns, tk->subTokenList, 0, kArray_size(tk->subTokenList)};
			bk = new_kBlock(kctx, stmt, NULL, &range);
			KLIB kObject_setObject(kctx, stmt, kw, TY_Block, bk);
		}
	}
	return (IS_Block(bk)) ? bk : def;
}

// ---------------------------------------------------------------------------
/* parsing syntax pattern */

#define kTokenPattern_TopChar(tk) ((IS_String(tk->text) && S_size((tk)->text) == 1) ? S_text((tk)->text)[0] : 0)

static int TokenSequence_findCloseCharAsTopChar(KonohaContext *kctx, TokenSequence *tokens, int beginIdx, int closech)
{
	int i;
	for(i = beginIdx; i < tokens->endIdx; i++) {
		kToken *tk = tokens->tokenList->tokenItems[i];
		if(kTokenPattern_TopChar(tk) == closech) return i;
	}
	return tokens->endIdx;
}

static kbool_t kArray_addSyntaxPattern(KonohaContext *kctx, kArray *patternList, TokenSequence *patterns);

static int TokenSequence_nestedSyntaxPattern(KonohaContext *kctx, TokenSequence *tokens, int currentIdx, ksymbol_t KW_AST, int closech)
{
	kTokenVar *tk = tokens->tokenList->tokenVarItems[currentIdx];
	int ne = TokenSequence_findCloseCharAsTopChar(kctx, tokens, currentIdx+1, closech);
	tk->resolvedSymbol = KW_AST;
	tk->resolvedSyntaxInfo = SYN_(tokens->ns, KW_AST);
	KSETv(tk, tk->subTokenList, new_(TokenArray, 0));
	TokenSequence nestedSourceRange = {tokens->ns, tokens->tokenList, currentIdx+1, ne};
	return kArray_addSyntaxPattern(kctx, tk->subTokenList, &nestedSourceRange) ? ne : tokens->endIdx;
}

static kbool_t kArray_addSyntaxPattern(KonohaContext *kctx, kArray *patternList, TokenSequence *patterns)
{
	int i;
	ksymbol_t stmtEntryKey = 0;
	kTokenVar *tk, *prevToken = NULL;
	//KdumpTokenArray(kctx, rules->tokenList, rules->beginIdx, rules->endIdx);
	for(i = patterns->beginIdx; i < patterns->endIdx; i++, prevToken = tk) {
		tk = patterns->tokenList->tokenVarItems[i];
		DBG_ASSERT(!kToken_isIndent(tk));
		DBG_ASSERT(tk->resolvedSyntaxInfo != NULL);
		if(tk->resolvedSyntaxInfo->keyword == KW_TextPattern) {
			int topch = kTokenPattern_TopChar(tk);
			KLIB kArray_add(kctx, patternList, tk);
			if(topch == '(') {
				i = TokenSequence_nestedSyntaxPattern(kctx, patterns, i, KW_ParenthesisGroup, ')');
			}
			else if(topch == '[') {
				i = TokenSequence_nestedSyntaxPattern(kctx, patterns, i, KW_BracketGroup, ']');
			}
			else {
				tk->resolvedSymbol = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NEWID);
			}
			continue;
		}
		if(tk->resolvedSyntaxInfo->keyword == KW_BracketGroup) {
			TokenSequence nestedSourceRange = {patterns->ns, tk->subTokenList, 0, kArray_size(tk->subTokenList)};
			tk->resolvedSymbol = KW_OptionalGroup;
			PUSH_GCSTACK(tk->subTokenList);  // avoid gc
			KSETv(tk, tk->subTokenList, new_(TokenArray, 0));
			kArray_addSyntaxPattern(kctx, tk->subTokenList, &nestedSourceRange);
			KLIB kArray_add(kctx, patternList, tk);
			continue;
		}
		if(tk->topCharHint == '$' && i+1 < patterns->endIdx) {  // $PatternName
			tk = patterns->tokenList->tokenVarItems[++i];
			if(IS_String(tk->text)) {
				tk->resolvedSymbol = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NEWRAW) | KW_PATTERN;
				if(stmtEntryKey == 0) stmtEntryKey = tk->resolvedSymbol;
				tk->stmtEntryKey = stmtEntryKey;
				tk->resolvedSyntaxInfo = SYN_(patterns->ns, tk->resolvedSymbol/*KW_SymbolPattern*/);
				stmtEntryKey = 0;
				KLIB kArray_add(kctx, patternList, tk);
				continue;
			}
		}
		if(i + 1 < patterns->endIdx && patterns->tokenList->tokenItems[i+1]->topCharHint == ':' && IS_String(tk->text)) {
			stmtEntryKey = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NEWRAW);
			i++;
			continue;
		}
		if(tk->topCharHint == '*' && prevToken != NULL) {
			kToken_set(MatchPreviousPattern, prevToken, true);
			tk = NULL;
			continue;
		}
		kToken_printMessage(kctx, tk, ErrTag, "illegal syntax pattern: %s", Token_text(tk));
		return false;
	}
	return true;
}

static void kNameSpace_parseSyntaxPattern(KonohaContext *kctx, kNameSpace *ns, const char *ruleSource, kfileline_t uline, kArray *patternList)
{
	TokenSequence source = {ns, KonohaContext_getSugarContext(kctx)->preparedTokenList};
	TokenSequence_push(kctx, source);
	TokenSequence_tokenize(kctx, &source, ruleSource, uline);
	TokenSequence patterns = {ns, source.tokenList, source.endIdx};
	patterns.TargetPolicy.RemovingIndent = true;
	TokenSequence_resolved2(kctx, &patterns, NULL, &source, source.beginIdx);
	KLIB kArray_add(kctx, patternList, K_NULLTOKEN);  // delim
	int firstPatternIdx = kArray_size(patternList);
	kArray_addSyntaxPattern(kctx, patternList, &patterns);
	if(firstPatternIdx < kArray_size(patternList)) {
		kToken *firstPattern = patternList->tokenItems[firstPatternIdx];
		if(kToken_isFirstPattern(firstPattern)) {
			ArrayNULL_append(kctx, UPCAST(ns), &((kNameSpaceVar*)ns)->StmtPatternListNULL, UPCAST(firstPattern));
		}
	}
	TokenSequence_pop(kctx, source);
	//KdumpTokenArray(kctx, patternList, 0, kArray_size(patternList));
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
