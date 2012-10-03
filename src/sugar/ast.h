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

static kExpr *callFuncParseExpr(KonohaContext *kctx, SugarSyntax *syn, kFunc *fo, int *countRef, kStmt *stmt, kArray *tokenList, int beginIdx, int operatorIdx, int endIdx)
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
		kFunc *fo = currentSyntax->sugarFuncTable[SUGARFUNC_ParseExpr];
		if(fo != NULL) {
			kFunc **funcItems = &fo;
			int index = 0;
			if(IS_Array(fo)) {
				funcItems = currentSyntax->sugarFuncListTable[SUGARFUNC_ParseExpr]->funcItems;
				index =  kArray_size(currentSyntax->sugarFuncListTable[SUGARFUNC_ParseExpr]) - 1;
			}
			for(; index >= 0; index--) {
				DBG_ASSERT(IS_Func(funcItems[index]));
				kExpr *texpr = callFuncParseExpr(kctx, exprSyntax, funcItems[index], &callCount, stmt, tokenList, beginIdx, operatorIdx, endIdx);
				if(Stmt_isERR(stmt)) return K_NULLEXPR;
				if(texpr != K_NULLEXPR) return texpr;
			}
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

static kExpr* kStmt_parseExpr(KonohaContext *kctx, kStmt *stmt, kArray *tokenList, int beginIdx, int endIdx)
{
	if(!Stmt_isERR(stmt)) {
		if(beginIdx < endIdx) {
			int idx = kStmt_findOperator(kctx, stmt, tokenList, beginIdx, endIdx);
			SugarSyntax *syn = tokenList->tokenItems[idx]->resolvedSyntaxInfo;
			return kStmt_parseOperatorExpr(kctx, stmt, syn, tokenList, beginIdx, idx, endIdx);
		}
		else {
#ifdef BE_COMPACT
			kStmt_printMessage(kctx, stmt, ErrTag, "syntax error: empty");
#else
			const char *where = "", *token = "";
			if (0 < beginIdx - 1) {
				where = " after "; token = Token_text(tokenList->tokenItems[beginIdx-1]);
			}
			else if(endIdx < kArray_size(tokenList)) {
				where = " before "; token = Token_text(tokenList->tokenItems[endIdx]);
			}
			kStmt_printMessage(kctx, stmt, ErrTag, "expected expression%s%s", where, token);
		}
#endif
	}
	return K_NULLEXPR;
}

static kExpr *kStmt_addExprParam(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kArray *tokenList, int s, int e, int allowEmpty)
{
	int i, start = s;
	for(i = s; i < e; i++) {
		kToken *tk = tokenList->tokenItems[i];
		if(tk->resolvedSyntaxInfo->keyword == KW_COMMA) {
			expr = Expr_add(kctx, expr, kStmt_parseExpr(kctx, stmt, tokenList, start, i));
			start = i + 1;
		}
	}
	if(allowEmpty == 0 || start < i) {
		expr = Expr_add(kctx, expr, kStmt_parseExpr(kctx, stmt, tokenList, start, i));
	}
	return expr;
}

static kExpr *kStmt_rightJoinExpr(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kArray *tokenList, int c, int e)
{
	if(c < e && expr != K_NULLEXPR && !Stmt_isERR(stmt)) {
		kToken *tk = tokenList->tokenItems[c];
		if(tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern || tk->resolvedSyntaxInfo->sugarFuncTable[SUGARFUNC_ParseExpr] == NULL) {
			kStmtToken_printMessage(kctx, stmt, tk, ErrTag, "undefined operator: %s", Token_text(tk));
			return K_NULLEXPR;
		}
		kStmtToken_printMessage(kctx, stmt, tk, WarnTag, "ignored term: %s...", Token_text(tk));
	}
	return expr;
}

/* ------------------------------------------------------------------------ */
/* new ast parser */

static int TokenList_skipIndent(kArray *tokenList, int currentIdx, int endIdx)
{
	for(; currentIdx < endIdx; currentIdx++) {
		kToken *tk = tokenList->tokenItems[currentIdx];
		if(kToken_is(StatementSeparator, tk)) return endIdx; // ;
		if(tk->unresolvedTokenType != TokenType_INDENT) break;
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
	else if(tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern) { // recheck
		foundClass = KLIB kNameSpace_getClass(kctx, ns, S_text(tk->text), S_size(tk->text), NULL);
		if(foundClass != NULL) {
			kToken_setTypeId(kctx, tk, ns, foundClass->typeId);
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

#define T_statement(kw)  StatementName(kctx, kw), StatementType(kw)

static const char* StatementName(KonohaContext *kctx, ksymbol_t keyword)
{
	const char *statement = SYM_t(keyword);
	if(keyword == KW_ExprPattern) statement = "expression";
	else if(keyword == KW_StmtTypeDecl) statement = "variable";
	else if(keyword == KW_StmtMethodDecl) statement =  "function";
	return statement;
}

static const char* StatementType(ksymbol_t keyword)
{
	const char *postfix = " statement";
	if(keyword == KW_ExprPattern) postfix = "";
	else if(keyword == KW_StmtTypeDecl || keyword == KW_StmtMethodDecl) postfix = " declaration";
	return postfix;
}

static SugarSyntax* kNameSpace_getStatementSyntax(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx)
{
	kToken *tk = tokenList->tokenItems[beginIdx];
	SugarSyntax *syn = tk->resolvedSyntaxInfo;
	if(syn->syntaxRuleNULL == NULL) {
		int nextIdx = TokenUtils_parseTypePattern(kctx, ns, tokenList, beginIdx, endIdx, NULL);
		DBG_P("@ nextIdx = %d < %d", nextIdx, endIdx);
		if(nextIdx != -1) {
			nextIdx = TokenList_skipIndent(tokenList, nextIdx, endIdx);
			if(nextIdx < endIdx) {
				if(TokenUtils_parseTypePattern(kctx, ns, tokenList, nextIdx, endIdx, NULL) != -1) {
					DBG_P("MethodDecl2");
					return SYN_(ns, KW_StmtMethodDecl);
				}
				kToken *tk = tokenList->tokenItems[nextIdx];
				KdumpToken(kctx, tk);
				if(tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern) {
					int symbolNextIdx = TokenList_skipIndent(tokenList, nextIdx + 1, endIdx);
					if(symbolNextIdx < endIdx && tokenList->tokenItems[symbolNextIdx]->resolvedSyntaxInfo->keyword == KW_ParenthesisGroup) {
						DBG_P("FuncDecl");
						return SYN_(ns, KW_StmtMethodDecl);
					}
					DBG_P("StmtTypeDecl");
					return SYN_(ns, KW_StmtTypeDecl);
				}
				if(tk->resolvedSyntaxInfo->keyword != KW_DOT && ((tk->resolvedSyntaxInfo->precedence_op1 > 0 || tk->resolvedSyntaxInfo->precedence_op2 > 0))) {
					DBG_P("Operator");
					return SYN_(ns, KW_StmtMethodDecl);
				}
			}
		}
		return SYN_(ns, KW_ExprPattern);
	}
	return syn;
}

// ---------------------------------------------------------------------------

static void kToken_transformToBraceGroup(KonohaContext *kctx, kTokenVar *tk, kNameSpace *ns, MacroSet *macroSet)
{
	if(tk->resolvedSyntaxInfo->keyword == TokenType_CODE) {
		INIT_GCSTACK();
		TokenSequence range = {ns, new_(TokenArray, 0), 0, 0};
		PUSH_GCSTACK(range.tokenList);
		TokenSequence_tokenize(kctx, &range, S_text(tk->text), tk->uline);
		KSETv(tk, tk->subTokenList, range.tokenList);
		tk->resolvedSyntaxInfo = SYN_(ns, KW_BraceGroup);
		RESET_GCSTACK();
	}
}

/* ------------------------------------------------------------------------ */

static int TokenSequence_resolved2(KonohaContext *kctx, TokenSequence *tokens, MacroSet *, TokenSequence *source, int beginIdx);

static kbool_t TokenSequence_expandMacro(KonohaContext *kctx, TokenSequence *tokens, ksymbol_t symbol, MacroSet *macroSet)
{
	while(macroSet->symbol != 0) {
		if(macroSet->symbol == symbol) {
			TokenSequence paramtokens = {tokens->ns, macroSet->tokenList, macroSet->beginIdx, macroSet->endIdx};
			TokenSequence_resolved2(kctx, tokens, NULL, &paramtokens, macroSet->beginIdx);
			return true;
		}
		macroSet++;
	}
	return false;
}

static kbool_t TokenSequence_applyMacro(KonohaContext *kctx, TokenSequence *tokens, kArray *macroTokenList, size_t paramsize, kToken *group)
{
	TokenSequence macro = {tokens->ns, macroTokenList, paramsize, kArray_size(macroTokenList)};
	if(paramsize == 0) {
		TokenSequence_resolved2(kctx, tokens, NULL, &macro, 0);
		return true;
	}
	else {
		MacroSet mp[paramsize+1];
		int p = 0, start = 0, i;
		for(i = 0; i < kArray_size(group->subTokenList); i++) {
			kToken *tk = group->subTokenList->tokenItems[i];
			if(tk->topCharHint == ',') {
				mp[p].symbol = macroTokenList->tokenItems[p]->resolvedSymbol;
				mp[p].tokenList = group->subTokenList;
				mp[p].beginIdx = start;
				mp[p].endIdx = i;
				p++;
				start = i + 1;
				if(!(p < paramsize)) break;
			}
		}
		if(p + 1 == paramsize) {
			mp[p].symbol = macroTokenList->tokenItems[p]->resolvedSymbol;
			mp[p].tokenList = group->subTokenList;
			mp[p].beginIdx = start;
			mp[p].endIdx = kArray_size(group->subTokenList);
			mp[p+1].symbol = 0; /* sentinel */
			TokenSequence_resolved2(kctx, tokens, mp, &macro, paramsize/*start*/);
			return true;
		}
		return false;
	}
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

static void kNameSpace_setMacroData(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, int paramsize, const char *data)
{
	SugarSyntaxVar *syn = (SugarSyntaxVar *)SUGAR kNameSpace_getSyntax(kctx, ns, keyword, /*new*/true);
	TokenSequence tokens = {ns, KonohaContext_getSugarContext(kctx)->preparedTokenList};
	TokenSequence_push(kctx, tokens);
	TokenSequence_tokenize(kctx, &tokens, data, 0);
	syn->macroParamSize = paramsize;
	KSETv(ns, syn->macroDataNULL, kArray_slice(kctx, tokens.tokenList, tokens.beginIdx, tokens.endIdx));
	TokenSequence_pop(kctx, tokens);
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
		TokenSequence nested = {source->ns, astToken->subTokenList, 0, 0};
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

static int TokenSequence_resolved2(KonohaContext *kctx, TokenSequence *tokens, MacroSet *macro, TokenSequence *source, int beginIdx)
{
	int currentIdx = beginIdx;
	if(tokens->TargetPolicy.syntaxSymbolPattern == NULL) {
		tokens->TargetPolicy.syntaxSymbolPattern = SYN_(tokens->ns, KW_SymbolPattern);
	}
	for(; currentIdx < source->endIdx; currentIdx++) {
		kTokenVar *tk = source->tokenList->tokenVarItems[currentIdx];
		if(tk->unresolvedTokenType == TokenType_INDENT && tokens->TargetPolicy.RemovingIndent) {
			continue;  // remove INDENT in () or []
		}
		if(tk->resolvedSyntaxInfo == NULL) {
			if(source->SourceConfig.openToken != NULL && source->SourceConfig.stopChar == tk->topCharHint) {
				return currentIdx;
			}
			if(tk->topCharHint == '(' || tk->topCharHint == '[') {
				currentIdx = TokenSequence_addGroup(kctx, tokens, macro, source, currentIdx+1, tk);
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
				if(macro != NULL && TokenSequence_expandMacro(kctx, tokens, symbol, macro)) {
					continue;
				}
				SugarSyntax *syn = SYN_(source->ns, symbol);
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
		DBG_ASSERT(source->SourceConfig.openToken != NULL);
		ERROR_UnclosedToken(kctx, (kTokenVar*)source->SourceConfig.openToken, (const char*)buf);
	}
	TokenSequence_end(kctx, tokens);
	return source->endIdx;
}

/* ------------------------------------------------------------------------ */

static int PatternMatch2(KonohaContext *kctx, SugarSyntax *syn, kStmt *stmt, ksymbol_t name, TokenSequence *tokens)
{
	int callCount = 0;
	if(syn != NULL) {
		while(true) {
			kFunc *fo = syn->sugarFuncTable[SUGARFUNC_PatternMatch];
			if(fo != NULL) {
				kFunc **funcItems = &fo;
				int index = 0, next;
				if(IS_Array(fo)) {
					funcItems = syn->sugarFuncListTable[SUGARFUNC_PatternMatch]->funcItems;
					index = kArray_size(syn->sugarFuncListTable[SUGARFUNC_PatternMatch]) - 1;
				}
				for(; index >= 0; index--) {
					next = callPatternMatchFunc(kctx, funcItems[index], &callCount, stmt, name, tokens->tokenList, tokens->beginIdx, tokens->endIdx);
					if(Stmt_isERR(stmt)) return -1;
					if(next >= tokens->beginIdx) return next;
				}
			}
			if(syn->parentSyntaxNULL == NULL) break;
			syn = syn->parentSyntaxNULL;
		}
	}
	if(callCount == 0) {
		kStmt_printMessage(kctx, stmt, ErrTag, "undefined syntax pattern: %s%s", PSYM_t(syn->keyword));
	}
	return -1;
}

static int kStmt_currentTokenIndex(KonohaContext *kctx, kStmt *stmt, kArray *tokenList, int beginIdx, int endIdx)
{
	int c;
	for(c = beginIdx; c < endIdx; c++) {
		kToken *tk = tokenList->tokenItems[c];
		if(tk->unresolvedTokenType == TokenType_INDENT) {
			continue;
		}
		if(tk->unresolvedTokenType == TokenType_ERR) {
			kStmt_toERR(kctx, stmt, tk->text);
			return -1;
		}
		break;
	}
	if(!(c < endIdx)) return -1;
	return c;

}

static int kStmt_matchSyntaxRule2(KonohaContext *kctx, kStmt *stmt, TokenSequence *tokens, TokenSequence *rules, kToken **errRuleRef)
{
	int currentRuleIdx = rules->beginIdx, currentTokenIdx = tokens->beginIdx;
	for(; currentRuleIdx < rules->endIdx; currentRuleIdx++) {
		kToken *ruleToken = rules->tokenList->tokenItems[currentRuleIdx];
		currentTokenIdx = kStmt_currentTokenIndex(kctx, stmt, tokens->tokenList, currentTokenIdx, tokens->endIdx);
		if(currentTokenIdx == -1) {
			while(ruleToken->resolvedSymbol == KW_OptionalGroup && currentRuleIdx + 1 < rules->endIdx) {
				currentRuleIdx++;
				ruleToken = rules->tokenList->tokenItems[currentRuleIdx];
			}
			if(ruleToken->resolvedSymbol != KW_OptionalGroup) {
				errRuleRef[0] = ruleToken;
				return -1;
			}
			return tokens->endIdx;
		}
		if(KW_isPATTERN(ruleToken->resolvedSymbol)) {
			SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), ruleToken->resolvedSymbol);
			tokens->beginIdx = currentTokenIdx;
			currentTokenIdx = PatternMatch2(kctx, syn, stmt, ruleToken->stmtEntryKey, tokens);
			if(currentTokenIdx == -1) {
				errRuleRef[0] = ruleToken;
				return -1;
			}
		}
		else if(ruleToken->resolvedSymbol == KW_OptionalGroup) {
			kToken *tk = tokens->tokenList->tokenItems[currentTokenIdx];
			if(!kToken_is(StatementSeparator, tk)) { // matching return; with return [$Expr]
				TokenSequence nrule = {Stmt_nameSpace(stmt), ruleToken->subTokenList, 0, kArray_size(ruleToken->subTokenList)};
				tokens->beginIdx = currentTokenIdx;
				int next = kStmt_matchSyntaxRule2(kctx, stmt, tokens, &nrule, errRuleRef);
				errRuleRef[0] = NULL;
				if(Stmt_isERR(stmt)) return -1;
				if(next != -1) {
					currentTokenIdx = next;
				}
			}
		}
		else {
			kToken *tk = tokens->tokenList->tokenItems[currentTokenIdx];
			if(ruleToken->resolvedSymbol != tk->resolvedSymbol) {
				errRuleRef[0] = ruleToken;
				return -1;
			}
			if(ruleToken->resolvedSymbol == KW_ParenthesisGroup || ruleToken->resolvedSymbol == KW_BracketGroup) {
				TokenSequence nrule = {Stmt_nameSpace(stmt), ruleToken->subTokenList, 0, kArray_size(ruleToken->subTokenList)};
				TokenSequence ntokens = {Stmt_nameSpace(stmt), tk->subTokenList, 0, kArray_size(tk->subTokenList)};
				int next = kStmt_matchSyntaxRule2(kctx, stmt, &ntokens, &nrule, errRuleRef);
				if(next == -1) {
					return -1;
				}
			}
			currentTokenIdx++;
		}
	}
	return currentTokenIdx;
}

static int kStmt_parseBySyntaxRule2(KonohaContext *kctx, kStmt *stmt, int indent, kArray *tokenList, int beginIdx, int endIdx)
{
	kNameSpace *ns = Stmt_nameSpace(stmt);
	SugarSyntax *stmtSyntax = kNameSpace_getStatementSyntax(kctx, ns, tokenList, beginIdx, endIdx);
	SugarSyntax *currentSyntax = stmtSyntax;
	((kStmtVar*)stmt)->syn = stmtSyntax;
	kToken *errRule = NULL;
	while(currentSyntax != NULL) {
		if(currentSyntax->syntaxRuleNULL != NULL) {
			TokenSequence nrule  = {ns, currentSyntax->syntaxRuleNULL, 0, kArray_size(currentSyntax->syntaxRuleNULL)};
			TokenSequence tokens = {ns, tokenList, beginIdx, endIdx};
			errRule = NULL;
			int nextIdx = kStmt_matchSyntaxRule2(kctx, stmt, &tokens, &nrule, &errRule);
			if(Stmt_isERR(stmt)) return endIdx;
			if(beginIdx < nextIdx) return nextIdx;
		}
		currentSyntax = currentSyntax->parentSyntaxNULL;
	}
	if(!Stmt_isERR(stmt)) {
		DBG_ASSERT(errRule != NULL);
		kStmt_printMessage(kctx, stmt, ErrTag, "%s%s: %s%s is expected", T_statement(stmt->syn->keyword), PSYM_t(errRule->resolvedSymbol));
	}
	return endIdx;
}

static int TokenSequence_skipAnnotation(KonohaContext *kctx, TokenSequence *range, int currentIdx)
{
	for(currentIdx = range->beginIdx; currentIdx < range->endIdx; currentIdx++) {
		kToken *tk = range->tokenList->tokenItems[currentIdx];
		if(tk->unresolvedTokenType == TokenType_INDENT) {
			continue;
		}
		if(!MN_isAnnotation(tk->resolvedSymbol)) break;
		if(currentIdx + 1 < range->endIdx) {
			kToken *nextToken = range->tokenList->tokenItems[currentIdx+1];
			if(nextToken->resolvedSyntaxInfo != NULL && nextToken->resolvedSyntaxInfo->keyword == KW_ParenthesisGroup) {
				currentIdx++;
			}
		}
	}
	return currentIdx;
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

static int kStmt_addAnnotation2(KonohaContext *kctx, kStmtVar *stmt, TokenSequence *range, int currentIdx, int *indentRef)
{
	for(currentIdx = range->beginIdx; currentIdx < range->endIdx; currentIdx++) {
		kToken *tk = range->tokenList->tokenItems[currentIdx];
		if(tk->unresolvedTokenType == TokenType_INDENT) {
			indentRef[0] = tk->indent;
			stmt->uline = tk->uline;
			continue;
		}
		if(!MN_isAnnotation(tk->resolvedSymbol)) break;
		if(currentIdx + 1 < range->endIdx) {
			kToken *nextToken = range->tokenList->tokenItems[currentIdx+1];
			kObject *value = UPCAST(K_TRUE);
			if(nextToken->resolvedSyntaxInfo != NULL && nextToken->resolvedSyntaxInfo->keyword == KW_ParenthesisGroup) {
				value = (kObject*)kStmt_parseExpr(kctx, stmt, nextToken->subTokenList, 0, kArray_size(nextToken->subTokenList));
				currentIdx++;
			}
			if(value != NULL) {
				KLIB kObject_setObject(kctx, stmt, tk->resolvedSymbol, O_typeId(value), value);
			}
		}
	}
	return currentIdx;
}

static void kBlock_addNewStmt2(KonohaContext *kctx, kBlock *bk, TokenSequence *tokens)
{
	int currentIdx = TokenSequence_skipStatementSeparator(tokens, tokens->beginIdx);
	currentIdx = TokenSequence_skipAnnotation(kctx, tokens, tokens->beginIdx);
	if(currentIdx < tokens->endIdx) {
		int indent = 0;
		kStmtVar *stmt = new_(StmtVar, 0);
		KLIB kArray_add(kctx, bk->stmtList, stmt);
		KINITp(stmt, stmt->parentBlockNULL, bk);
		kStmt_addAnnotation2(kctx, stmt, tokens, tokens->beginIdx, &indent);
		if(stmt->uline == 0) {
			stmt->uline = tokens->tokenList->tokenItems[currentIdx]->uline;
		}
		currentIdx = kStmt_parseBySyntaxRule2(kctx, stmt, indent, tokens->tokenList, currentIdx, tokens->endIdx);
		if(currentIdx == -1) {
			DBG_ASSERT(Stmt_isERR(stmt));
			tokens->beginIdx = tokens->endIdx;
		}
	}
	tokens->beginIdx = TokenSequence_skipStatementSeparator(tokens, currentIdx);
}

static kBlock *new_kBlock2(KonohaContext *kctx, kStmt *parent, MacroSet *macro, TokenSequence *source)
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
		kBlock_addNewStmt2(kctx, bk, &tokens);
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
			bk = new_kBlock2(kctx, stmt, NULL, &range);
			KLIB kObject_setObject(kctx, stmt, kw, TY_Block, bk);
		}
	}
	return (IS_Block(bk)) ? bk : def;
}


/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
