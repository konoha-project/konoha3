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

static kExpr *callFuncParseExpr(KonohaContext *kctx, SugarSyntax *syn, kFunc *fo, kStmt *stmt, kArray *tokenList, int s, int c, int e)
{
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 6);
	lsfp[K_CALLDELTA+0].unboxValue = (uintptr_t)syn;
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+0].o, fo->self, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+1].o, (kObject*)stmt, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+2].asArray, tokenList, GC_NO_WRITE_BARRIER);
	lsfp[K_CALLDELTA+3].intValue = s;
	lsfp[K_CALLDELTA+4].intValue = c;
	lsfp[K_CALLDELTA+5].intValue = e;
	KCALL(lsfp, 0, fo->mtd, 5, K_NULLEXPR);
	END_LOCAL();
	DBG_ASSERT(IS_Expr(lsfp[0].asObject));
	return lsfp[0].asExpr;
}

static kExpr *kStmt_parseOperatorExpr(KonohaContext *kctx, kStmt *stmt, SugarSyntax *syn, kArray *tokenList, int beginIdx, int operatorIdx, int endIdx)
{
	if(syn->sugarFuncTable[SUGARFUNC_ParseExpr] != NULL) {
		kFunc *fo = syn->sugarFuncTable[SUGARFUNC_ParseExpr];
		kExpr *texpr;
		if(IS_Array(fo)) {
			int i;
			kArray *a = (kArray*)fo;
			for(i = kArray_size(a) - 1; i > 0; i--) {
				texpr = callFuncParseExpr(kctx, syn, fo, stmt, tokenList, beginIdx, operatorIdx, endIdx);
				if(Stmt_isERR(stmt)) return K_NULLEXPR;
				if(texpr != K_NULLEXPR) return texpr;
			}
			fo = a->funcItems[0];
		}
		DBG_ASSERT(IS_Func(fo));
		texpr = callFuncParseExpr(kctx, syn, fo, stmt, tokenList, beginIdx, operatorIdx, endIdx);
		if(Stmt_isERR(stmt)) {
			return K_NULLEXPR;
		}
		if(texpr != K_NULLEXPR) {
			return texpr;
		}
	}
	if(syn->precedence_op2 > 0 || syn->precedence_op1 > 0) {
		syn = SYN_(Stmt_nameSpace(stmt), KW_ExprOperator);
		return kStmt_parseOperatorExpr(kctx, stmt, syn, tokenList, beginIdx, operatorIdx, endIdx);
	}
	if(syn->ty != TY_unknown || syn->sugarFuncTable[SUGARFUNC_ExprTyCheck] != NULL) {
		syn = SYN_(Stmt_nameSpace(stmt), KW_ExprTerm);
		return kStmt_parseOperatorExpr(kctx, stmt, syn, tokenList, beginIdx, operatorIdx, endIdx);
	}
	kkStmt_printMessage(stmt, ErrTag, "undefined expression parser for '%s'", Token_text(tokenList->tokenItems[operatorIdx]));
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
			const char *where = "", *token = "";
			if (0 < beginIdx - 1) {
				where = " after "; token = Token_text(tokenList->tokenItems[beginIdx-1]);
			}
			else if(endIdx < kArray_size(tokenList)) {
				where = " before "; token = Token_text(tokenList->tokenItems[endIdx]);
			}
			kkStmt_printMessage(stmt, ErrTag, "expected expression%s%s", where, token);
		}
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
	//KLIB kArray_clear(kctx, tokenList, s);
	return expr;
}

static kExpr *kStmt_rightJoinExpr(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kArray *tokenList, int c, int e)
{
	if(c < e && expr != K_NULLEXPR && !Stmt_isERR(stmt)) {
		WARN_IgnoredTokens(kctx, tokenList, c, e);
	}
	return expr;
}

/* ------------------------------------------------------------------------ */
/* new ast parser */

typedef struct {
	kNameSpace *ns;
	kArray *tokenList;
	int beginIdx;
	int endIdx;
	kArray *stmtTokenList;
	kToken *errToken;
	SugarSyntax *symbolSyntaxInfo;
} ASTEnv;

static int kStmt_parseTypePattern(KonohaContext *kctx, kStmt *stmt, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx, KonohaClass **classRef);

static KonohaClass* kStmt_parseGenerics(KonohaContext *kctx, kStmt *stmt, kNameSpace *ns, KonohaClass *baseClass, kArray *tokenList, int beginIdx, int endIdx)
{
	size_t i = beginIdx, psize = 0;
	kparamtype_t p[endIdx];
	while(i < endIdx) {
		KonohaClass *paramClass = NULL;
		i = kStmt_parseTypePattern(kctx, stmt, ns, tokenList, i, endIdx, &paramClass);
		if(paramClass == NULL) {
			return NULL;
		}
		p[psize].ty = paramClass->typeId;
		psize++;
		if(i < endIdx && tokenList->tokenItems[i]->resolvedSyntaxInfo->keyword == KW_COMMA) {
			i++;
		}
	}
	if(baseClass->baseTypeId == TY_Func) {
		return KLIB KonohaClass_Generics(kctx, baseClass, p[0].ty, psize-1, p+1);
	}
	else {
		return KLIB KonohaClass_Generics(kctx, baseClass, TY_void, psize, p);
	}
}

static int kStmt_parseTypePattern(KonohaContext *kctx, kStmt *stmt, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx, KonohaClass **classRef)
{
	int nextIdx = -1;
	kToken *tk = tokenList->tokenItems[beginIdx];
	KonohaClass *foundClass = NULL;
	if(tk->resolvedSyntaxInfo->keyword == KW_TypePattern) {
		foundClass = CT_(tk->resolvedTypeId);
		nextIdx = beginIdx + 1;
	}
	if(foundClass != NULL) {
		int isAllowedGenerics = true;
		while(nextIdx < endIdx) {
			kToken *tk = tokenList->tokenItems[nextIdx];
			if(tk->resolvedSyntaxInfo->keyword != KW_BracketGroup) break;
			int sizeofBracketTokens = kArray_size(tk->subTokenList);
			if(isAllowedGenerics &&  sizeofBracketTokens > 0) {  // C[T][]
				KonohaClass *foundGenericClass = kStmt_parseGenerics(kctx, stmt, ns, foundClass, tk->subTokenList, 0, sizeofBracketTokens);
				if(foundGenericClass == NULL) break;
				foundClass = foundGenericClass;
			}
			else {
				if(sizeofBracketTokens > 0) break;   // C[100] is treated as C  and the token [100] is set to nextIdx;
				foundClass = CT_p0(kctx, CT_Array, foundClass->typeId);  // C[] => Array[C]
			}
			isAllowedGenerics = false;
			nextIdx++;
		}
	}
	if(classRef != NULL) {
		classRef[0] = foundClass;
		if(foundClass==NULL) nextIdx = -1;
	}
	return nextIdx;
}

static int PatternMatchFunc(KonohaContext *kctx, kFunc *fo, kStmt *stmt, ksymbol_t name, kArray *tokenList, int beginIdx, int endIdx)
{
	INIT_GCSTACK();
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 5);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+0].o, fo->self, GC_NO_WRITE_BARRIER);
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+1].o, (kObject*)stmt, GC_NO_WRITE_BARRIER);
	lsfp[K_CALLDELTA+2].intValue = name;
	KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+3].asArray, tokenList, GC_NO_WRITE_BARRIER);
	lsfp[K_CALLDELTA+4].intValue = beginIdx;
	lsfp[K_CALLDELTA+5].intValue = endIdx;
	KCALL(lsfp, 0, fo->mtd, 5, KLIB Knull(kctx, CT_Int));
	END_LOCAL();
	RESET_GCSTACK();
	return (int)lsfp[0].intValue;
}

static int PatternMatch(KonohaContext *kctx, SugarSyntax *syn, kStmt *stmt, ksymbol_t name, kArray *tokenList, int beginIdx, int endIdx)
{
	if(syn == NULL || syn->sugarFuncTable[SUGARFUNC_PatternMatch] == NULL) {
		kkStmt_printMessage(stmt, ErrTag, "unknown syntax pattern: %s%s", KW_t(syn->keyword));
		return -1;
	}
	kFunc *fo = syn->sugarFuncTable[SUGARFUNC_PatternMatch];
	int next;
	if(IS_Array(fo)) {
		int i;
		kArray *a = (kArray*)fo;
		for(i = kArray_size(a) - 1; i > 0; i--) {
			next = PatternMatchFunc(kctx, fo, stmt, name, tokenList, beginIdx, endIdx);
			if(Stmt_isERR(stmt)) return -1;
			if(next > beginIdx) return next;
		}
		fo = a->funcItems[0];
	}
	DBG_ASSERT(IS_Func(fo));
	next = PatternMatchFunc(kctx, fo, stmt, name, tokenList, beginIdx, endIdx);
	if(Stmt_isERR(stmt)) return -1;
	return (next > beginIdx) ? next : -1;
}

static int TokenArray_findPrefetchedRuleToken(kArray *tokenList, int beginIdx, int endIdx, kToken *ruleToken)
{
	int i;
	for(i = beginIdx; i < endIdx; i++) {
		kToken *tk = tokenList->tokenItems[i];
		if(ruleToken->resolvedSymbol == tk->resolvedSymbol) return i;
	}
	return -1;
}

static int kStmt_printMismatchedRule(KonohaContext *kctx, kStmt *stmt, kToken *tk, kToken *ruleToken, int returnIdx, int canRollBack)
{
	if(!canRollBack && !Stmt_isERR(stmt)) {
		kToken_p(stmt, tk, ErrTag, "%s%s expects %s%s", T_statement(stmt->syn->keyword), KW_t(ruleToken->resolvedSymbol));
	}
	return returnIdx;
}

#define kTokenArray_skip(TLS, S, E)  S

static int kStmt_matchSyntaxRule(KonohaContext *kctx, kStmt *stmt, kArray *tokenList, int beginIdx, int endIdx, TokenRange *rule, int canRollBack)
{
	int currentRuleIdx, currentTokenIdx = beginIdx, returnIdx = (canRollBack ? beginIdx : -1);
//	DBG_P("Input tokens:");
//	KdumpTokenArray(kctx, tokenList, beginIdx, endIdx);
//	DBG_P("Syntax rules:");
//	KdumpTokenArray(kctx, rule->tokenList, rule->beginIdx, rule->endIdx);
	for(currentRuleIdx = rule->beginIdx; currentRuleIdx < rule->endIdx && currentTokenIdx < endIdx; currentRuleIdx++) {
		kToken *ruleToken = rule->tokenList->tokenItems[currentRuleIdx];
		currentTokenIdx = kTokenArray_skip(tokenList, currentTokenIdx, endIdx);
		dumpToken(kctx, ruleToken, currentRuleIdx);
		if(KW_isPATTERN(ruleToken->resolvedSymbol)) {
			//DBG_P("@Pattern");
			int patternEndIdx = endIdx;
			if(currentRuleIdx + 1 < rule->endIdx) {
				kToken *prefetchedRuleToken = rule->tokenList->tokenItems[currentRuleIdx+1];
				dumpToken(kctx, prefetchedRuleToken, currentRuleIdx+1);
				if((!KW_isPATTERN(prefetchedRuleToken->resolvedSymbol) && prefetchedRuleToken->resolvedSymbol != KW_OptionalGroup)) {
					patternEndIdx = TokenArray_findPrefetchedRuleToken(tokenList, currentTokenIdx+1, endIdx, prefetchedRuleToken);
					if(patternEndIdx == -1) {
						kToken *tk = tokenList->tokenItems[currentTokenIdx];
						return kStmt_printMismatchedRule(kctx, stmt, tk, ruleToken, returnIdx, canRollBack);
					}
					currentRuleIdx++;
				}
			}
			SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), ruleToken->resolvedSymbol);
			int next = PatternMatch(kctx, syn, stmt, ruleToken->stmtEntryKey, tokenList, currentTokenIdx, patternEndIdx);
			if(next == -1) {
				kToken *tk = tokenList->tokenItems[currentTokenIdx];
				return kStmt_printMismatchedRule(kctx, stmt, tk, ruleToken, returnIdx, canRollBack);
			}
			currentTokenIdx = (patternEndIdx == endIdx) ? next : patternEndIdx + 1;
		}
		else if(ruleToken->resolvedSymbol == KW_OptionalGroup) {
			//DBG_P("@Optional");
			TokenRange nrule = {Stmt_nameSpace(stmt), ruleToken->subTokenList, 0, kArray_size(ruleToken->subTokenList)};
			int next = kStmt_matchSyntaxRule(kctx, stmt, tokenList, currentTokenIdx, endIdx, &nrule, 1/*roolback*/);
			if(next == -1) return returnIdx;
			currentTokenIdx = next;
		}
		else {
			//DBG_P("@Symbol");
			kToken *tk = tokenList->tokenItems[currentTokenIdx];
			if(ruleToken->resolvedSymbol != tk->resolvedSymbol) {
				return kStmt_printMismatchedRule(kctx, stmt, tk, ruleToken, beginIdx, canRollBack);
			}
			if(ruleToken->resolvedSymbol == KW_ParenthesisGroup || ruleToken->resolvedSymbol == KW_BracketGroup) {
				TokenRange nrule = {Stmt_nameSpace(stmt), ruleToken->subTokenList, 0, kArray_size(ruleToken->subTokenList)};
				int next = kStmt_matchSyntaxRule(kctx, stmt, tk->subTokenList, 0, kArray_size(tk->subTokenList), &nrule, 0/*not rollbck*/);
				if(next == -1) return returnIdx;
			}
			currentTokenIdx++;
		}
	}
//	DBG_P("rollback=%d, returnIdx=%d, currentTokenIdx=%d < %d", canRollBack, returnIdx, currentTokenIdx, endIdx);
	for(; currentRuleIdx < rule->endIdx; currentRuleIdx++) {
		kToken *ruleToken = rule->tokenList->tokenItems[currentRuleIdx];
		if(ruleToken->resolvedSymbol != KW_OptionalGroup) {
			if(!canRollBack) {
				kkStmt_printMessage(stmt, ErrTag, "%s%s needs syntax pattern: %s%s", T_statement(stmt->syn->keyword), KW_t(ruleToken->resolvedSymbol));
				return returnIdx;
			}
		}
		return returnIdx;
	}
	if(currentTokenIdx < endIdx) {
		if(!canRollBack) {
			kkStmt_printMessage(stmt, ErrTag, "%s%s: unexpected token %s", T_statement(stmt->syn->keyword), Token_text(tokenList->tokenItems[currentTokenIdx]));
			return returnIdx;
		}
	}
	return currentTokenIdx;
}

static SugarSyntax* kNameSpace_getSyntaxRule(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx)
{
//	KdumpTokenArray(kctx, tokenList, beginIdx, endIdx);
	int nextIdx = kStmt_parseTypePattern(kctx, NULL, ns, tokenList, beginIdx, endIdx, NULL);
//	DBG_P("nextIdx=%d, endIdx=%d", nextIdx, endIdx);
	if(nextIdx != -1) {
		if(nextIdx < endIdx) {
			kToken *tk = tokenList->tokenItems[nextIdx];
			if(tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern) {
				if(nextIdx+1 < endIdx && tokenList->tokenItems[nextIdx+1]->resolvedSyntaxInfo->keyword == KW_ParenthesisGroup) {
//					DBG_P("MethodDecl");
					return SYN_(ns, KW_StmtMethodDecl); //
				}
//				DBG_P("TypeDecl");
				return SYN_(ns, KW_StmtTypeDecl);  //
			}
			if(tk->resolvedSyntaxInfo->keyword == KW_TypePattern
				|| ((tk->resolvedSyntaxInfo->precedence_op1 > 0 || tk->resolvedSyntaxInfo->precedence_op2 > 0) && tk->resolvedSyntaxInfo->keyword != KW_DOT)) {
//				DBG_P("MethodDecl");
				return SYN_(ns, KW_StmtMethodDecl); //
			}
		}
		return SYN_(ns, KW_ExprPattern);
	}
	kToken *tk = tokenList->tokenItems[beginIdx];
	if(tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern && isUpperCaseSymbol(S_text(tk->text))) {
		if(beginIdx+1 < endIdx && tokenList->tokenItems[beginIdx+1]->resolvedSyntaxInfo->keyword == KW_LET) {
//			DBG_P("Const");
			return SYN_(ns, KW_StmtConstDecl);  // CONSTVAL = ...
		}
	}
	SugarSyntax *syn = tk->resolvedSyntaxInfo;
	if(syn->syntaxRuleNULL == NULL) {
		return SYN_(ns, KW_ExprPattern);
	}
	return syn;
}

static kbool_t kStmt_parseBySyntaxRule(KonohaContext *kctx, kStmt *stmt, kArray *tokenList, int beginIdx, int endIdx)
{
	kbool_t ret = false;
	SugarSyntax *syn = kNameSpace_getSyntaxRule(kctx, Stmt_nameSpace(stmt), tokenList, beginIdx, endIdx);
	TokenRange nrule = {Stmt_nameSpace(stmt), syn->syntaxRuleNULL, 0, kArray_size(syn->syntaxRuleNULL)};
	DBG_ASSERT(syn->syntaxRuleNULL != NULL);
	((kStmtVar*)stmt)->syn = syn;
	ret = (kStmt_matchSyntaxRule(kctx, stmt, tokenList, beginIdx, endIdx, &nrule, 0) != -1);
	return ret;
}

static int TokenRange_addResolvedToken(KonohaContext *kctx, TokenRange *range, TokenRange *sourceRange, int currentIdx);

static kbool_t TokenRange_expandMacro(KonohaContext *kctx, TokenRange *range, ksymbol_t symbol, MacroSet *macro)
{
	while(macro->symbol != 0) {
		if(macro->symbol == symbol) {
			size_t i;
			TokenRange *macroRange = macro->macro;
			for(i = macroRange->beginIdx; i < macroRange->endIdx; i++) {
				kToken *tk = macroRange->tokenList->tokenItems[i];
				if(tk->resolvedSyntaxInfo != NULL) {
					KLIB kArray_add(kctx, range->tokenList, tk);
				}
				else {
					i = TokenRange_addResolvedToken(kctx, range, macroRange, i);
				}
			}
			return true;
		}
		macro++;
	}
	return false;
}

static int TokenRange_addSymbolToken(KonohaContext *kctx, TokenRange *range, TokenRange *sourceRange, int i)
{
	kTokenVar *tk = sourceRange->tokenList->tokenVarItems[i];
	const char *t = S_text(tk->text);
	ksymbol_t symbol = ksymbolA(t, S_size(tk->text), SYM_NEWID);
	if(sourceRange->macroSet != NULL && TokenRange_expandMacro(kctx, range, symbol, sourceRange->macroSet)) {
		return i;
	}
	SugarSyntax *syn = SYN_(range->ns, symbol);
	if(syn != NULL) {
		if(syn->ty != TY_unknown) {
			kToken_setTypeId(kctx, tk, range->ns, syn->ty);
		}
		else {
			tk->resolvedSymbol     = symbol;
			tk->resolvedSyntaxInfo = syn;
		}
	}
	else {
		KonohaClass *foundClass = KLIB kNameSpace_getClass(kctx, range->ns, t, S_size(tk->text), NULL);
		if(foundClass != NULL) {
			kToken_setTypeId(kctx, tk, range->ns, foundClass->typeId);
		}
		else {
//			if(!isalpha(t[0])) {
//				Token_pERR(kctx, tk, "undefined token: %s", Token_text(tk));
//				range->errToken = tk;
//				return sourceRange->endIdx;  // end
//				while(t[0] != 0) {
//					ksymbol_t op1 = ksymbolA(t, 1, SYM_NEWID);
//					syn = SYN_(range->ns, op1);
//					if(syn == NULL) {
//						Token_pERR(kctx, tk, "undefined token: %s", Token_text(tk));
//						range->errToken = tk;
//						return range->endIdx;  // end
//					}
//					kTokenVar *splitToken = new_(TokenVar, syn->keyword);
//					KLIB kArray_add(kctx, range->stmtTokenList, splitToken);
//					splitToken->resolvedSyntaxInfo = syn;
//					splitToken->uline = tk->uline;
//					KSETv(splitToken, splitToken->text, SYM_s(op1));
//					t++;
//				}
//			}
			tk->resolvedSymbol = symbol;
			tk->resolvedSyntaxInfo = SYN_(sourceRange->ns, KW_SymbolPattern);
		}
	}
	KLIB kArray_add(kctx, range->tokenList, tk);
	return i;
}

static int TokenRange_selectStmtToken(KonohaContext *kctx, TokenRange *range, TokenRange *sourceRange, CheckEndOfStmtFunc2 isEndOfStmt, int *optionRef)
{
	int currentIdx;
	for(currentIdx = sourceRange->beginIdx; currentIdx < sourceRange->endIdx; currentIdx++) {
		if(isEndOfStmt(kctx, range, sourceRange, &currentIdx, optionRef)) {
			break;
		}
		kToken *tk = sourceRange->tokenList->tokenItems[currentIdx];
		if(tk->resolvedSyntaxInfo != NULL) {
			KLIB kArray_add(kctx, range->tokenList, tk);
		}
		else {
			currentIdx = TokenRange_addResolvedToken(kctx, range, sourceRange, currentIdx);
		}
	}
	TokenRange_end(kctx, range);
	return currentIdx;
}

static kbool_t checkCloseParenthesis(KonohaContext *kctx, TokenRange *range, TokenRange *sourceRange, int *currentIdx, int *probablyCloseBeforeRef)
{
	kToken *tk = sourceRange->tokenList->tokenItems[*currentIdx];
	return (tk->topCharHint == ')');
}

static kbool_t checkCloseBracket(KonohaContext *kctx, TokenRange *range, TokenRange *sourceRange, int *currentIdx, int *probablyCloseBeforeRef)
{
	kToken *tk = sourceRange->tokenList->tokenItems[*currentIdx];
	return (tk->topCharHint == ']');
}


static int TokenRange_addStrucuredToken(KonohaContext *kctx, TokenRange *range, TokenRange *sourceRange, int currentIdx, ksymbol_t AST_type)
{
	int probablyCloseBefore = sourceRange->endIdx - 1;
	kTokenVar *astToken = new_(TokenVar, AST_type);
	KLIB kArray_add(kctx, range->tokenList, astToken);
	astToken->resolvedSyntaxInfo = SYN_(range->ns, AST_type);
	KSETv(astToken, astToken->subTokenList, new_(TokenArray, 0));
	astToken->uline = sourceRange->tokenList->tokenItems[currentIdx]->uline;
	{
		TokenRange nestedRangeBuf, *nestedRange = new_TokenListRange(kctx, sourceRange->ns, astToken->subTokenList, &nestedRangeBuf);
		CheckEndOfStmtFunc2 f = (AST_type == KW_ParenthesisGroup) ? checkCloseParenthesis : checkCloseBracket;
		sourceRange->beginIdx = currentIdx + 1;
		int returnIdx = TokenRange_selectStmtToken(kctx, nestedRange, sourceRange, f, &probablyCloseBefore);
		if(nestedRange->errToken != NULL) {
			if(returnIdx == sourceRange->endIdx) {
				int closech = (AST_type == KW_ParenthesisGroup) ? ')': ']';
				Token_pERR(kctx, astToken, "'%c' is expected (probably before %s)", closech, Token_text(sourceRange->tokenList->tokenItems[probablyCloseBefore]));
				nestedRange->errToken = astToken;
			}
			range->errToken = nestedRange->errToken;
		}
		return returnIdx;
	}
}

static int TokenRange_addResolvedToken(KonohaContext *kctx, TokenRange *range, TokenRange *sourceRange, int currentIdx)
{
	kTokenVar *tk = sourceRange->tokenList->tokenVarItems[currentIdx];
	if(tk->unresolvedTokenType == TokenType_ERR) {
		range->errToken = tk;
		return sourceRange->endIdx;  // resolved no more
	}
	if(tk->topCharHint == '(') {
		return TokenRange_addStrucuredToken(kctx, range, sourceRange, currentIdx, KW_ParenthesisGroup);
	}
	if(tk->topCharHint == '[') {
		return TokenRange_addStrucuredToken(kctx, range, sourceRange, currentIdx, KW_BracketGroup);
	}
	if(tk->topCharHint == '@' && currentIdx + 1 < sourceRange->endIdx) {
		kTokenVar *tk1 = sourceRange->tokenList->tokenVarItems[currentIdx+1];
		if(tk1->unresolvedTokenType == TokenType_SYMBOL) {
			tk1->resolvedSymbol = ksymbolA(S_text(tk1->text), S_size(tk1->text), SYM_NEWID) | MN_Annotation;
			tk1->resolvedSyntaxInfo = SYN_(sourceRange->ns, KW_SymbolPattern);
			KLIB kArray_add(kctx, range->tokenList, tk1);
			return currentIdx+1;
		}
	}
	if(tk->unresolvedTokenType == TokenType_SYMBOL) {
		return TokenRange_addSymbolToken(kctx, range, sourceRange, currentIdx);
	}
	else {
		SugarSyntax *syn = SYN_(range->ns, tk->unresolvedTokenType);
		if(syn != NULL) {
			tk->resolvedSyntaxInfo = syn;
			KLIB kArray_add(kctx, range->tokenList, tk);
		}
	}
	return currentIdx;
}

static kbool_t TokenRange_resolved(KonohaContext *kctx, TokenRange *range, TokenRange *sourceRange)
{
	int i;
	for(i = sourceRange->beginIdx; i < sourceRange->endIdx; i++) {
		kToken *tk = sourceRange->tokenList->tokenItems[i];
		if(tk->resolvedSyntaxInfo != NULL) {
			KLIB kArray_add(kctx, range->tokenList, tk);
		}
		else {
			i = TokenRange_addResolvedToken(kctx, range, sourceRange, i);
		}
	}
	TokenRange_end(kctx, range);
	return (range->errToken == NULL);
}

static int kStmt_addAnnotation(KonohaContext *kctx, kStmt *stmt, TokenRange *range)
{
	int i;
	for(i = range->beginIdx; i < range->endIdx; i++) {
		kToken *tk = range->tokenList->tokenItems[i];
		if(!MN_isAnnotation(tk->resolvedSymbol)) break;
		if(i + 1 < range->endIdx) {
			kToken *nextToken = range->tokenList->tokenItems[i+1];
			kObject *value = UPCAST(K_TRUE);
			if(nextToken->resolvedSyntaxInfo->keyword == KW_ParenthesisGroup) {
				value = (kObject*)kStmt_parseExpr(kctx, stmt, nextToken->subTokenList, 0, kArray_size(nextToken->subTokenList));
				i++;
			}
			if(value != NULL) {
				KLIB kObject_setObject(kctx, stmt, tk->resolvedSymbol, O_typeId(value), value);
			}
		}
	}
	return i;
}

static kStmt* kBlock_addNewStmt(KonohaContext *kctx, kBlock *bk, TokenRange *range)
{
	kStmtVar *stmt = new_(StmtVar, 0);
	KLIB kArray_add(kctx, bk->stmtList, stmt);
	KINITv(stmt->parentBlockNULL, bk);
	if(range->errToken != NULL) {
		kStmt_toERR(kctx, stmt, range->errToken->text);
	}
	else {
		int i;
		for(i = range->beginIdx; i < range->endIdx; i++) {
			DBG_ASSERT(range->tokenList->tokenItems[i]->resolvedSyntaxInfo != NULL);
		}
		int currentIdx = kStmt_addAnnotation(kctx, stmt, range);
		if(currentIdx < range->endIdx) {
			stmt->uline = range->tokenList->tokenItems[currentIdx]->uline;
			kStmt_parseBySyntaxRule(kctx, stmt, range->tokenList, currentIdx, range->endIdx);
		}
	}
	return stmt;
}

static kbool_t SemiColon(KonohaContext *kctx, TokenRange *range, TokenRange *sourceRange, int *currentIdxRef, int *indentRef)
{
	kbool_t found = false;
	kToken *tk = sourceRange->tokenList->tokenItems[*currentIdxRef];
	if(tk->topCharHint == ';') {
		found = true;
		do {
			*currentIdxRef += 1;
			if(!(*currentIdxRef < sourceRange->endIdx)) break;
			tk = sourceRange->tokenList->tokenItems[*currentIdxRef];
		}while(tk->topCharHint == ';');
	}
	if(tk->unresolvedTokenType == TokenType_INDENT) {
//		DBG_P("!! previous=%d, indent=%d", indentRef[0], tk->indent);
		found = (kArray_size(range->tokenList) > range->beginIdx);
	}
	return found;
}

static kbool_t Comma(KonohaContext *kctx, TokenRange *range, TokenRange *sourceRange, int *currentIdxRef, int *indentRef)
{
	kToken *tk = sourceRange->tokenList->tokenItems[*currentIdxRef];
	if(tk->topCharHint == ',') {
		*currentIdxRef += 1;
		return true;
	}
	return false;
}

static kBlock *new_kBlock(KonohaContext *kctx, kStmt *parent, TokenRange *sourceRange, CheckEndOfStmtFunc2 isEndOfStmt)
{
	kBlockVar *bk = GCSAFE_new(BlockVar, sourceRange->ns);
	if(parent != NULL) {
		KINITv(bk->parentStmtNULL, parent);
	}
	if(isEndOfStmt == NULL) {
		isEndOfStmt = SemiColon;
	}
	int i = sourceRange->beginIdx, indent = 0;
	while(i < sourceRange->endIdx) {
		TokenRange rangeBuf, *range = new_TokenStackRange(kctx, sourceRange, &rangeBuf);
		sourceRange->beginIdx = i;
		i = TokenRange_selectStmtToken(kctx, range, sourceRange, isEndOfStmt, &indent);
		if(range->errToken != NULL) {
			kBlock_addNewStmt(kctx, bk, range);
			break;
		}
		if(range->endIdx > range->beginIdx) {
			kBlock_addNewStmt(kctx, bk, range);
		}
	}
	return (kBlock*)bk;
}

static void kToken_transformToBraceGroup(KonohaContext *kctx, kTokenVar *tk, kNameSpace *ns)
{
	if(tk->resolvedSyntaxInfo->keyword == TokenType_CODE) {
		INIT_GCSTACK();
		TokenRange range = {ns, new_(TokenArray, 0), 0, 0};
		PUSH_GCSTACK(range.tokenList);
		TokenRange_tokenize(kctx, &range, S_text(tk->text), tk->uline);
		KSETv(tk, tk->subTokenList, range.tokenList);
		tk->resolvedSyntaxInfo = SYN_(ns, KW_BraceGroup);
		RESET_GCSTACK();
	}
}

static kBlock* kStmt_getBlock(KonohaContext *kctx, kStmt *stmt, kNameSpace *ns, ksymbol_t kw, kBlock *def)
{
	kBlock *bk = (kBlock*)kStmt_getObjectNULL(kctx, stmt, kw);
	if(bk == NULL) return def;
	if(IS_Token(bk)) {
		kToken *tk = (kToken*)bk;
		if(ns == NULL) ns = Stmt_nameSpace(stmt);
		if (tk->resolvedSyntaxInfo->keyword == TokenType_CODE) {
			kToken_transformToBraceGroup(kctx, (kTokenVar*)tk, ns);
		}
		if (tk->resolvedSyntaxInfo->keyword == KW_BraceGroup) {
			TokenRange range = {ns, tk->subTokenList, 0, kArray_size(tk->subTokenList)};
			bk = new_kBlock(kctx, stmt, &range, NULL);
			KLIB kObject_setObject(kctx, stmt, kw, TY_Block, bk);
		}
	}
	return (IS_Block(bk)) ? bk : def;
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
