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

static inline kToken* TokenArray_nextToken(KonohaContext *kctx, kArray *tokenList, int beginIdx, int endIdx)
{
	return (beginIdx < endIdx) ? tokenList->tokenItems[beginIdx] : K_NULLTOKEN;
}

/* ------------------------------------------------------------------------ */

static KMETHOD UndefinedParseExpr(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tokenArray, s, c, e);
	kStmt_p(stmt, ErrTag, "undefined expression parser for '%s'", Token_text(tokenArray->tokenItems[c]));
}

static kExpr *callFuncParseExpr(KonohaContext *kctx, SugarSyntax *syn, kFunc *fo, kStmt *stmt, kArray *tokenArray, int s, int c, int e)
{
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 6);
	KSETv(lsfp[K_CALLDELTA+0].o, fo->self);
	lsfp[K_CALLDELTA+0].unboxValue = (uintptr_t)syn;
	KSETv(lsfp[K_CALLDELTA+1].o, (kObject*)stmt);
	KSETv(lsfp[K_CALLDELTA+2].asArray, tokenArray);
	lsfp[K_CALLDELTA+3].intValue = s;
	lsfp[K_CALLDELTA+4].intValue = c;
	lsfp[K_CALLDELTA+5].intValue = e;
	KCALL(lsfp, 0, fo->mtd, 5, K_NULLEXPR);
	END_LOCAL();
	DBG_ASSERT(IS_Expr(lsfp[0].asObject));
	return lsfp[0].asExpr;
}

static kExpr *kStmt_parseOperatorExpr(KonohaContext *kctx, kStmt *stmt, kArray *tokenArray, int beginIdx, int currentIdx, int endIdx)
{
	SugarSyntax *syn = tokenArray->tokenItems[currentIdx]->resolvedSyntaxInfo;
	kFunc *fo = (syn->ParseExpr == NULL) ? kmodsugar->UndefinedParseExpr : syn->ParseExpr;
	kExpr *texpr;
	if(IS_Array(fo)) {
		int i;
		kArray *a = (kArray*)fo;
		for(i = kArray_size(a) - 1; i > 0; i--) {
			texpr = callFuncParseExpr(kctx, syn, fo, stmt, tokenArray, beginIdx, currentIdx, endIdx);
			if(Stmt_isERR(stmt)) return K_NULLEXPR;
			if(texpr != K_NULLEXPR) return texpr;
		}
		fo = a->funcItems[0];
	}
	DBG_ASSERT(IS_Func(fo));
	texpr = callFuncParseExpr(kctx, syn, fo, stmt, tokenArray, beginIdx, currentIdx, endIdx);
	if(texpr == K_NULLEXPR && !Stmt_isERR(stmt)) {
		kStmt_p(stmt, ErrTag, "syntax error: operator %s", Token_text(tokenArray->tokenItems[currentIdx]));
	}
	return texpr;
}

static int kStmt_findOperator(KonohaContext *kctx, kStmt *stmt, kArray *tokenArray, int beginIdx, int endIdx)
{
	int isPrePosition = true;
	int idx = beginIdx, i, precedence = 0;
	for(i = beginIdx; i < endIdx; i++) {
		kToken *tk = tokenArray->tokenItems[i];
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

static kExpr* kStmt_parseExpr(KonohaContext *kctx, kStmt *stmt, kArray *tokenArray, int beginIdx, int endIdx)
{
	if(!Stmt_isERR(stmt)) {
		if(beginIdx < endIdx) {
			int idx = kStmt_findOperator(kctx, stmt, tokenArray, beginIdx, endIdx);
			return kStmt_parseOperatorExpr(kctx, stmt, tokenArray, beginIdx, idx, endIdx);
		}
		else {
			const char *where = "", *token = "";
			if (0 < beginIdx - 1) {
				where = " after "; token = Token_text(tokenArray->tokenItems[beginIdx-1]);
			}
			else if(endIdx < kArray_size(tokenArray)) {
				where = " before "; token = Token_text(tokenArray->tokenItems[endIdx]);
			}
			kStmt_p(stmt, ErrTag, "expected expression%s%s", where, token);
		}
	}
	return K_NULLEXPR;
}

static kExpr *kStmt_addExprParam(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kArray *tokenArray, int s, int e, int allowEmpty)
{
	int i, start = s;
	for(i = s; i < e; i++) {
		kToken *tk = tokenArray->tokenItems[i];
		if(tk->resolvedSyntaxInfo->keyword == KW_COMMA) {
			expr = Expr_add(kctx, expr, kStmt_parseExpr(kctx, stmt, tokenArray, start, i));
			start = i + 1;
		}
	}
	if(allowEmpty == 0 || start < i) {
		expr = Expr_add(kctx, expr, kStmt_parseExpr(kctx, stmt, tokenArray, start, i));
	}
	KLIB kArray_clear(kctx, tokenArray, s);
	return expr;
}

static kExpr *kStmt_rightJoinExpr(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kArray *tokenArray, int c, int e)
{
	if(c < e && expr != K_NULLEXPR && !Stmt_isERR(stmt)) {
		WARN_IgnoredTokens(kctx, tokenArray, c, e);
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
		p[psize].ty = paramClass->classId;
		psize++;
		if(i < endIdx && tokenList->tokenItems[i]->resolvedSyntaxInfo->keyword == KW_COMMA) {
			i++;
		}
	}
	if(baseClass->baseclassId == TY_Func) {
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
				foundClass = CT_p0(kctx, CT_Array, foundClass->classId);  // C[] => Array[C]
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
	KSETv(lsfp[K_CALLDELTA+0].o, fo->self);
	KSETv(lsfp[K_CALLDELTA+1].o, (kObject*)stmt);
	lsfp[K_CALLDELTA+2].intValue = name;
	KSETv(lsfp[K_CALLDELTA+3].asArray, tokenList);
	lsfp[K_CALLDELTA+4].intValue = beginIdx;
	lsfp[K_CALLDELTA+5].intValue = endIdx;
	KCALL(lsfp, 0, fo->mtd, 5, KLIB Knull(kctx, CT_Int));
	END_LOCAL();
	RESET_GCSTACK();
	return (int)lsfp[0].intValue;
}

static int PatternMatch(KonohaContext *kctx, SugarSyntax *syn, kStmt *stmt, ksymbol_t name, kArray *tokenList, int beginIdx, int endIdx)
{
	if(syn == NULL || syn->PatternMatch == kmodsugar->UndefinedParseExpr/*NULL*/) {
		kStmt_p(stmt, ErrTag, "unknown syntax pattern: %s%s", KW_t(syn->keyword));
		return -1;
	}
	kFunc *fo = syn->PatternMatch;
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

static int kStmt_matchSyntaxRule(KonohaContext *kctx, kStmt *stmt, kArray *tokenList, int beginIdx, int endIdx, TokenChunk *rule, int canRollBack)
{
	int currentRuleIdx, currentTokenIdx = beginIdx, returnIdx = (canRollBack ? beginIdx : -1);
//	DBG_P("Input tokens:");
//	KdumpTokenArray(kctx, tokenList, beginIdx, endIdx);
//	DBG_P("Syntax rules:");
//	KdumpTokenArray(kctx, rule->tokenList, rule->beginIdx, rule->endIdx);
	for(currentRuleIdx = rule->beginIdx; currentRuleIdx < rule->endIdx && currentTokenIdx < endIdx; currentRuleIdx++) {
		kToken *ruleToken = rule->tokenList->tokenItems[currentRuleIdx];
		currentTokenIdx = kTokenArray_skip(tokenList, currentTokenIdx, endIdx);
		if(KW_isPATTERN(ruleToken->resolvedSymbol)) {
			int patternEndIdx = endIdx;
			kToken *prefetchedRuleToken = rule->tokenList->tokenItems[currentRuleIdx+1];
			if(currentRuleIdx + 1 < rule->endIdx && (!KW_isPATTERN(prefetchedRuleToken->resolvedSymbol) && prefetchedRuleToken->resolvedSymbol != KW_OptionalGroupGroup)) {
				patternEndIdx = TokenArray_findPrefetchedRuleToken(tokenList, currentTokenIdx+1, endIdx, prefetchedRuleToken);
				if(patternEndIdx == -1) {
					kToken *tk = tokenList->tokenItems[currentTokenIdx];
					return kStmt_printMismatchedRule(kctx, stmt, tk, ruleToken, returnIdx, canRollBack);
				}
				currentRuleIdx++;
			}
			SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), ruleToken->resolvedSymbol);
			int next = PatternMatch(kctx, syn, stmt, ruleToken->stmtEntryKey, tokenList, currentTokenIdx, patternEndIdx);
			if(next == -1) {
				kToken *tk = tokenList->tokenItems[currentTokenIdx];
				return kStmt_printMismatchedRule(kctx, stmt, tk, ruleToken, returnIdx, canRollBack);
			}
			currentTokenIdx = (patternEndIdx == endIdx) ? next : patternEndIdx + 1;
		}
		else if(ruleToken->resolvedSymbol == KW_OptionalGroupGroup) {
			TokenChunk nrule = {ruleToken->subTokenList, 0, kArray_size(ruleToken->subTokenList)};
			int next = kStmt_matchSyntaxRule(kctx, stmt, tokenList, currentTokenIdx, endIdx, &nrule, 1/*roolback*/);
			if(next == -1) return returnIdx;
			currentTokenIdx = next;
		}
		else {
			kToken *tk = tokenList->tokenItems[currentTokenIdx];
			if(ruleToken->resolvedSymbol != tk->resolvedSymbol) {
				return kStmt_printMismatchedRule(kctx, stmt, tk, ruleToken, beginIdx, canRollBack);
			}
			if(ruleToken->resolvedSymbol == KW_ParenthesisGroup || ruleToken->resolvedSymbol == KW_BracketGroup) {
				TokenChunk nrule = {ruleToken->subTokenList, 0, kArray_size(ruleToken->subTokenList)};
				int next = kStmt_matchSyntaxRule(kctx, stmt, tk->subTokenList, 0, kArray_size(tk->subTokenList), &nrule, 0/*not rollbck*/);
				if(next == -1) return returnIdx;
			}
			currentTokenIdx++;
		}
	}
//	DBG_P("rollback=%d, returnIdx=%d, currentTokenIdx=%d < %d", canRollBack, returnIdx, currentTokenIdx, endIdx);
	for(; currentRuleIdx < rule->endIdx; currentRuleIdx++) {
		kToken *ruleToken = rule->tokenList->tokenItems[currentRuleIdx];
		if(ruleToken->resolvedSymbol != KW_OptionalGroupGroup) {
			if(!canRollBack) {
				kStmt_p(stmt, ErrTag, "%s%s needs syntax pattern: %s%s", T_statement(stmt->syn->keyword), KW_t(ruleToken->resolvedSymbol));
				return returnIdx;
			}
		}
		return returnIdx;
	}
	if(currentTokenIdx < endIdx) {
		if(!canRollBack) {
			kStmt_p(stmt, ErrTag, "%s%s: unexpected token %s", T_statement(stmt->syn->keyword), Token_text(tokenList->tokenItems[currentTokenIdx]));
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
	//		dumpToken(kctx, tk, -1);
			if(tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern) {
				if(nextIdx+1 < endIdx && tokenList->tokenItems[nextIdx+1]->resolvedSyntaxInfo->keyword == KW_ParenthesisGroup) {
					DBG_P("MethodDecl");
					return SYN_(ns, KW_StmtMethodDecl); //
				}
				DBG_P("TypeDecl");
				return SYN_(ns, KW_StmtTypeDecl);  //
			}
			if(tk->resolvedSyntaxInfo->keyword == KW_TypePattern
				|| ((tk->resolvedSyntaxInfo->precedence_op1 > 0 || tk->resolvedSyntaxInfo->precedence_op2 > 0) && tk->resolvedSyntaxInfo->keyword != KW_DOT)) {
				DBG_P("MethodDecl");
				return SYN_(ns, KW_StmtMethodDecl); //
			}
		}
		return SYN_(ns, KW_ExprPattern);
	}
	kToken *tk = tokenList->tokenItems[beginIdx];
	if(tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern && isUpperCaseSymbol(S_text(tk->text))) {
		kToken *tk1 = TokenArray_nextToken(kctx, tokenList, beginIdx+1, endIdx);
		if(tk1->resolvedSyntaxInfo->keyword == KW_LET) {
			DBG_P("Const");
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
	TokenChunk nrule = {syn->syntaxRuleNULL, 0, kArray_size(syn->syntaxRuleNULL)};
	DBG_ASSERT(syn->syntaxRuleNULL != NULL);
	((kStmtVar*)stmt)->syn = syn;
	ret = (kStmt_matchSyntaxRule(kctx, stmt, tokenList, beginIdx, endIdx, &nrule, 0) != -1);
	return ret;
}

// ast

static int kNameSpace_addStrucuredToken(KonohaContext *kctx, ASTEnv *env, ksymbol_t AST_type);

static int kNameSpace_addSymbolToken(KonohaContext *kctx, ASTEnv *env, kTokenVar *tk)
{
	const char *t = S_text(tk->text);
	ksymbol_t symbol = ksymbolA(t, S_size(tk->text), SYM_NEWID);
	SugarSyntax *syn = SYN_(env->ns, symbol);
	if(syn != NULL) {
		if(syn->ty != TY_unknown) {
			kToken_setTypeId(kctx, tk, env->ns, syn->ty);
		}
		else {
			tk->resolvedSymbol     = symbol;
			tk->resolvedSyntaxInfo = syn;
		}
	}
	else {
		KonohaClass *foundClass = KLIB kNameSpace_getClass(kctx, env->ns, t, S_size(tk->text), NULL);
		if(foundClass != NULL) {
			kToken_setTypeId(kctx, tk, env->ns, foundClass->classId);
		}
		else {
			if(!isalpha(t[0])) {
				Token_pERR(kctx, tk, "undefined token: %s", Token_text(tk));
				env->errToken = tk;
				return env->endIdx;  // end
//				while(t[0] != 0) {
//					ksymbol_t op1 = ksymbolA(t, 1, SYM_NEWID);
//					syn = SYN_(env->ns, op1);
//					if(syn == NULL) {
//						Token_pERR(kctx, tk, "undefined token: %s", Token_text(tk));
//						env->errToken = tk;
//						return env->endIdx;  // end
//					}
//					kTokenVar *splitToken = new_(TokenVar, syn->keyword);
//					KLIB kArray_add(kctx, env->stmtTokenList, splitToken);
//					splitToken->resolvedSyntaxInfo = syn;
//					splitToken->uline = tk->uline;
//					KSETv(splitToken->text, SYM_s(op1));
//					t++;
//				}
			}
			tk->resolvedSymbol = symbol;
			tk->resolvedSyntaxInfo = env->symbolSyntaxInfo;
		}
	}
	KLIB kArray_add(kctx, env->stmtTokenList, tk);
	return env->beginIdx;
}


static int kNameSpace_addResolvedToken(KonohaContext *kctx, ASTEnv *env)
{
	kTokenVar *tk = env->tokenList->tokenVarItems[env->beginIdx];
	if(tk->unresolvedTokenType == TokenType_ERR) {
		env->errToken = tk;
		return env->endIdx;  // resolved no more
	}
	if(tk->topCharHint == '(') {
		return kNameSpace_addStrucuredToken(kctx, env, KW_ParenthesisGroup);
	}
	if(tk->topCharHint == '[') {
		return kNameSpace_addStrucuredToken(kctx, env, KW_BracketGroup);
	}
	if(tk->topCharHint == '@' && env->beginIdx + 1 < env->endIdx) {
		kTokenVar *tk1 = env->tokenList->tokenVarItems[env->beginIdx+1];
		if(tk1->unresolvedTokenType == TokenType_SYMBOL) {
			tk1->resolvedSymbol = ksymbolA(S_text(tk1->text), S_size(tk1->text), SYM_NEWID) | MN_Annotation;
			tk1->resolvedSyntaxInfo = env->symbolSyntaxInfo;
			KLIB kArray_add(kctx, env->stmtTokenList, tk1);
			return env->beginIdx + 1;
		}
	}
	if(tk->unresolvedTokenType == TokenType_SYMBOL) {
		return kNameSpace_addSymbolToken(kctx, env, tk);
	}
	SugarSyntax *syn = SYN_(env->ns, tk->unresolvedTokenType);
	if(syn != NULL) {
		tk->resolvedSyntaxInfo = syn;
		KLIB kArray_add(kctx, env->stmtTokenList, tk);
	}
	return env->beginIdx;
}

static int kNameSpace_selectStmtTokenList(KonohaContext *kctx, ASTEnv *env, int *optionRef, CheckEndOfStmtFunc isEndOfStmt)
{
	int currentIdx, beginIdxOfStmtTokenList = kArray_size(env->stmtTokenList);
	for(currentIdx = env->beginIdx; currentIdx < env->endIdx; currentIdx++) {
		if(isEndOfStmt(kctx, env->tokenList, &currentIdx, env->endIdx, optionRef, env->stmtTokenList, beginIdxOfStmtTokenList)) {
			return currentIdx;
		}
		kToken *tk = env->tokenList->tokenItems[currentIdx];
		if(tk->resolvedSyntaxInfo != NULL) {
			KLIB kArray_add(kctx, env->stmtTokenList, tk);
		}
		else {
			env->beginIdx = currentIdx;
			currentIdx = kNameSpace_addResolvedToken(kctx, env);
		}
	}
	return currentIdx;
}

static kbool_t kNameSpace_resolveTokenArray(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx, kArray *resolvedTokenList)
{
	int i;
	ASTEnv env = {
		ns, tokenList, beginIdx, endIdx, resolvedTokenList, NULL, SYN_(ns, KW_SymbolPattern)
	};
	for(i = beginIdx; i < endIdx; i++) {
		kToken *tk = tokenList->tokenItems[i];
		if(tk->resolvedSyntaxInfo != NULL) {
			KLIB kArray_add(kctx, resolvedTokenList, tk);
		}
		else {
			env.beginIdx = i;
			i = kNameSpace_addResolvedToken(kctx, &env);
		}
	}
	return (env.errToken == NULL);
}

static kbool_t checkEndOfParenthesis(KonohaContext *kctx, kArray *tokenList, int *currentIdx, int endIdx, int *probablyCloseBeforeRef, kArray *stmtTokenList, int beginIdx)
{
	kToken *tk = tokenList->tokenItems[*currentIdx];
	return (tk->topCharHint == ')');
}

static kbool_t checkEndOfBracket(KonohaContext *kctx, kArray *tokenList, int *currentIdx, int endIdx, int *probablyCloseBeforeRef, kArray *stmtTokenList, int beginIdx)
{
	kToken *tk = tokenList->tokenItems[*currentIdx];
	return (tk->topCharHint == ']');
}

static int kNameSpace_addStrucuredToken(KonohaContext *kctx, ASTEnv *env, ksymbol_t AST_type)
{
	ASTEnv newenv = *env;
	int probablyCloseBefore = env->endIdx - 1;
	kTokenVar *astToken = new_(TokenVar, AST_type);
	KLIB kArray_add(kctx, env->stmtTokenList, astToken);
	astToken->resolvedSyntaxInfo = SYN_(env->ns, AST_type);
	KSETv(astToken->subTokenList, new_(TokenArray, 0));
	astToken->uline = env->tokenList->tokenItems[env->beginIdx]->uline;
	newenv.beginIdx = env->beginIdx + 1;
	newenv.stmtTokenList = astToken->subTokenList;
	CheckEndOfStmtFunc f = (AST_type == KW_ParenthesisGroup) ? checkEndOfParenthesis : checkEndOfBracket;
	int returnIdx = kNameSpace_selectStmtTokenList(kctx, &newenv, &probablyCloseBefore, f);
	if(newenv.errToken != NULL) {
		if(returnIdx == env->endIdx) {
			int closech = (AST_type == KW_ParenthesisGroup) ? ')': ']';
			Token_pERR(kctx, astToken, "'%c' is expected (probably before %s)", closech, Token_text(env->tokenList->tokenItems[probablyCloseBefore]));
			env->errToken = astToken;
		}
		env->errToken = newenv.errToken;
	}
	return returnIdx;
}

static int kStmt_addAnnotation(KonohaContext *kctx, kStmt *stmt, kArray *tokenList, int beginIdx, int endIdx)
{
	int i;
	for(i = beginIdx; i < endIdx; i++) {
		kToken *tk = tokenList->tokenItems[i];
		if(!MN_isAnnotation(tk->resolvedSymbol)) break;
		if(i + 1 < endIdx) {
			kToken *tk1 = tokenList->tokenItems[i+1];
			kObject *value = UPCAST(K_TRUE);
			if(tk1->resolvedSyntaxInfo->keyword == KW_ParenthesisGroup) {
				value = (kObject*)kStmt_parseExpr(kctx, stmt, tk1->subTokenList, 0, kArray_size(tk1->subTokenList));
				i++;
			}
			if(value != NULL) {
				KLIB kObject_setObject(kctx, stmt, tk->resolvedSymbol, O_classId(value), value);
			}
		}
	}
	return i;
}

static kStmt* kBlock_addNewStmt(KonohaContext *kctx, kBlock *bk, kArray *tokenList, int beginIdx, int endIdx, kToken *errToken)
{
	kStmtVar *stmt = new_(StmtVar, 0);
	KLIB kArray_add(kctx, bk->stmtList, stmt);
	KINITv(stmt->parentBlockNULL, bk);
	if(errToken != NULL) {
		kStmt_toERR(kctx, stmt, errToken->text);
	}
	else {
		int currentIdx = kStmt_addAnnotation(kctx, stmt, tokenList, beginIdx, endIdx);
		if(currentIdx < endIdx) {
			stmt->uline = tokenList->tokenItems[currentIdx]->uline;
			kStmt_parseBySyntaxRule(kctx, stmt, tokenList, currentIdx, endIdx);
		}
	}
	return stmt;
}

static kbool_t SemiColon(KonohaContext *kctx, kArray *tokenList, int *currentIdx, int endIdx, int *indentRef, kArray *stmtTokenList, int beginIdx)
{
	kbool_t found = false;
	kToken *tk = tokenList->tokenItems[*currentIdx];
	if(tk->topCharHint == ';') {
		found = true;
		do {
			*currentIdx += 1;
			if(!(*currentIdx < endIdx)) break;
			tk = tokenList->tokenItems[*currentIdx];
		}while(tk->topCharHint == ';');
	}
	if(tk->unresolvedTokenType == TokenType_INDENT) {
//		DBG_P("!! previous=%d, indent=%d", indentRef[0], tk->indent);
		found = (kArray_size(stmtTokenList) > beginIdx);
	}
	return found;
}

static kbool_t Comma(KonohaContext *kctx, kArray *tokenList, int *currentIdx, int endIdx, int *optionRef, kArray *stmtTokenList, int beginIdx)
{
	kToken *tk = tokenList->tokenItems[*currentIdx];
	if(tk->topCharHint == ',') {
		*currentIdx += 1;
		return true;
	}
	return false;
}

static kBlock *new_Block(KonohaContext *kctx, kNameSpace *ns, kStmt *parent, kArray *tokenList, int beginIdx, int endIdx, CheckEndOfStmtFunc isEndOfStmt)
{
	kBlockVar *bk = GCSAFE_new(BlockVar, ns);
	if(parent != NULL) {
		KINITv(bk->parentStmtNULL, parent);
	}
	ASTEnv env = {ns, tokenList, beginIdx, endIdx, tokenList, NULL};
	env.symbolSyntaxInfo = SYN_(ns, KW_SymbolPattern);
	int i = beginIdx, indent = 0, atop = kArray_size(tokenList);
	if(isEndOfStmt == NULL) {
		isEndOfStmt = SemiColon;
	}
	while(i < endIdx) {
//		DBG_ASSERT(atop == kArray_size(tokenList));
		env.beginIdx = i;
		i = kNameSpace_selectStmtTokenList(kctx, &env, &indent, isEndOfStmt);
		int asize = kArray_size(tokenList);
		if(asize > atop || env.errToken != NULL) {
			kBlock_addNewStmt(kctx, bk, tokenList, atop, asize, env.errToken);
		}
		KLIB kArray_clear(kctx, tokenList, atop);
	}
	return (kBlock*)bk;
}

static void Token_toBRACE(KonohaContext *kctx, kTokenVar *tk, kNameSpace *ns)
{
	if(tk->resolvedSyntaxInfo->keyword == TokenType_CODE) {
		kArray *a = GCSAFE_new(TokenArray, 0);
		kNameSpace_tokenize(kctx, ns, S_text(tk->text), tk->uline, a);
		KSETv(tk->subTokenList, a);
		tk->resolvedSyntaxInfo = SYN_(ns, KW_BraceGroup);
	}
}

static kBlock* kStmt_getBlock(KonohaContext *kctx, kStmt *stmt, ksymbol_t kw, kBlock *def)
{
	kBlock *bk = (kBlock*)kStmt_getObjectNULL(kctx, stmt, kw);
	if(bk == NULL) return def;
	if(IS_Token(bk)) {
		kToken *tk = (kToken*)bk;
		if (tk->resolvedSyntaxInfo->keyword == TokenType_CODE) {
			Token_toBRACE(kctx, (kTokenVar*)tk, Stmt_nameSpace(stmt));
		}
		if (tk->resolvedSyntaxInfo->keyword == KW_BraceGroup) {
			bk = new_Block(kctx, Stmt_nameSpace(stmt), stmt, tk->subTokenList, 0, kArray_size(tk->subTokenList), SemiColon);
			KLIB kObject_setObject(kctx, stmt, kw, TY_Block, bk);
		}
	}
	return (IS_Block(bk)) ? bk : def;
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
