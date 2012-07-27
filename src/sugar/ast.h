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
// Block

//static int selectStmtLine(KonohaContext *kctx, kNameSpace *ns, int *indent, kArray *tokenList, int beginIdx, int endIdx, int delim, kArray *tokenArraydst, kToken **errTokenRef);
//static void Block_addStmtLine(KonohaContext *kctx, kBlock *bk, kArray *tokenList, int beginIdx, int end, kToken *errToken);
//static int makeTree(KonohaContext *kctx, kNameSpace *ns, ksymbol_t tt, kArray *tokenList, int beginIdx, int endIdx, int closech, kArray *tokenArraydst, kToken **errTokenRef);
//
//static kBlock *new_Block0(KonohaContext *kctx, kNameSpace *ns, kStmt *parent, kArray *tokenList, int beginIdx, int endIdx, int delim)
//{
//	kBlockVar *bk = GCSAFE_new(BlockVar, ns);
//	if(parent != NULL) {
//		KINITv(bk->parentStmtNULL, parent);
//	}
//	int i = beginIdx, indent = 0, atop = kArray_size(tokenList);
//	while(i < endIdx) {
//		kToken *errToken = NULL;
//		DBG_ASSERT(atop == kArray_size(tokenList));
//		i = selectStmtLine(kctx, ns, &indent, tokenList, i, endIdx, delim, tokenList, &errToken);
//		int asize = kArray_size(tokenList);
//		if(asize > atop) {
//			Block_addStmtLine(kctx, bk, tokenList, atop, asize, errToken);
//			KLIB kArray_clear(kctx, tokenList, atop);
//		}
//	}
//	return (kBlock*)bk;
//}
//
//static kbool_t resolveSymbolToken(KonohaContext *kctx, kNameSpace *ns, kTokenVar *tk)
//{
//	ksymbol_t kw = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NONAME);
//	if(kw != SYM_NONAME) {
//		SugarSyntax *syn = SYN_(ns, kw);
//		if(syn != NULL) {
//			if(syn->ty != TY_unknown) {
//				tk->keyword = KW_TypePattern;
//				tk->ty = syn->ty;
//			}
//			else {
//				tk->keyword = kw;
//			}
//			return 1;
//		}
//	}
//	return 0;
//}
//
//
//static int appendKeyword(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx, kArray *dst, kToken **errToken)
//{
//	int next = beginIdx; // don't add
//	kTokenVar *tk = tokenList->tokenVarItems[beginIdx];
//	if(tk->keyword == TK_SYMBOL) {
//		if(!resolveSymbolToken(kctx, ns, tk)) {
//			const char *t = S_text(tk->text);
//			if(isalpha(t[0])) {
//				KonohaClass *ct = KLIB kNameSpace_getClass(kctx, ns, S_text(tk->text), S_size(tk->text), NULL);
//				if(ct != NULL) {
//					tk->keyword = KW_TypePattern;
//					tk->ty = ct->classId;
//				}
//			}
//			else {
//				Token_pERR(kctx, tk, "undefined token: %s", Token_text(tk));
//				errToken[0] = tk;
//				return endIdx;
//			}
//		}
//	}
//	Token_setUnresolved(tk, false);
//	if(Token_isVirtualTypeLiteral(tk)) {   // trying to resolve Type[Type, Type]
//		KLIB kArray_add(kctx, dst, tk);
//		while(next + 1 < endIdx) {
//			kToken *tkB = tokenList->tokenItems[next + 1];
//			int topch = Token_topch(tkB);
//			if(topch != '[') break;
//			kArray *abuf = ctxsugar->preparedTokenList;
//			size_t atop = kArray_size(abuf);
//			next = makeTree(kctx, ns, AST_BRACKET, tokenList,  next+1, endIdx, ']', abuf, errToken);
//			if(!(kArray_size(abuf) > atop)) return next;
//			tkB = abuf->tokenItems[atop];
//			if(tkB->keyword == AST_BRACKET) {
//				tk = TokenType_resolveGenerics(kctx, ns, tk, tkB);
//				if(tk == NULL) {
//					if(abuf != dst) {
//						KLIB kArray_add(kctx, dst, tkB);
//						KLIB kArray_clear(kctx, abuf, atop);
//					}
//					return next;
//				}
//			}
//			KLIB kArray_clear(kctx, abuf, atop);
//		}
//	}
//	else {
//		KLIB kArray_add(kctx, dst, tk);
//	}
//	return next;
//}
//static int makeTree(KonohaContext *kctx, kNameSpace *ns, ksymbol_t astkw, kArray *tokenList, int beginIdx, int endIdx, int closech, kArray *tokenArraydst, kToken **errTokenRef)
//{
//	int i, probablyCloseBefore = endIdx - 1;
//	kToken *tk = tokenList->tokenItems[beginIdx];
//	kTokenVar *tkP = new_(TokenVar, 0);
//	KLIB kArray_add(kctx, tokenArraydst, tkP);
//	tkP->keyword = astkw;
//	tkP->uline = tk->uline;
//	KSETv(tkP->subTokenList, new_(TokenArray, 0));
//	for(i = beginIdx + 1; i < endIdx; i++) {
//		tk = tokenList->tokenItems[i];
//		if(tk->keyword == TK_ERR) break;  // ERR
//		if(!kToken_needsKeywordResolved(tk)) {
//			KLIB kArray_add(kctx, tkP->subTokenList, tk);
//			continue;
//		}
//		int topch = Token_topch(tk);
//		DBG_ASSERT(topch != '{');
//		if(topch == '(') {
//			i = makeTree(kctx, ns, AST_PARENTHESIS, tokenList, i, endIdx, ')', tkP->subTokenList, errTokenRef);
//			continue;
//		}
//		if(topch == '[') {
//			i = makeTree(kctx, ns, AST_BRACKET, tokenList, i, endIdx, ']', tkP->subTokenList, errTokenRef);
//			continue;
//		}
//		if(topch == closech) {
//			return i;
//		}
//		if(closech != '}') {
//			if(tk->keyword == TK_INDENT) continue; // remove INDENT from tokens;
//			if(tk->keyword == TK_CODE) probablyCloseBefore = i;
//		}
//		i = appendKeyword(kctx, ns, tokenList, i, endIdx, tkP->subTokenList, errTokenRef);
//	}
//	if(tk->keyword != TK_ERR) {
//		Token_pERR(kctx, tkP, "'%c' is expected (probably before %s)", closech, Token_text(tokenList->tokenItems[probablyCloseBefore]));
//	}
//	else {
//		tkP->keyword = TK_ERR;
//		KSETv(tkP->text, tk->text);
//	}
//	errTokenRef[0] = tkP;
//	return endIdx;
//}
//
//static int selectStmtLine(KonohaContext *kctx, kNameSpace *ns, int *indent, kArray *tokenList, int beginIdx, int endIdx, int delim, kArray *tokenArraydst, kToken **errTokenRef)
//{
//	int i = beginIdx;
//	DBG_ASSERT(endIdx <= kArray_size(tokenList));
//	KdumpTokenArray(kctx, tokenList, beginIdx, endIdx);
//	for(; i < endIdx - 1; i++) {
//		kToken *tk = tokenList->tokenItems[i];
//		kTokenVar *tk1 = tokenList->tokenVarItems[i+1];
//		int topch = Token_topch(tk);
//		if(topch == '@' && (tk1->keyword == TK_SYMBOL)) {
//			tk1->keyword = ksymbolA(S_text(tk1->text), S_size(tk1->text), SYM_NEWID) | MN_Annotation;
//			KLIB kArray_add(kctx, tokenArraydst, tk1); i++;
//			tk1 = tokenList->tokenVarItems[i+1];
//			topch = Token_topch(tk1);
//			if(i + 1 < endIdx && topch == '(') {
//				i = makeTree(kctx, ns, AST_PARENTHESIS, tokenList, i+1, endIdx, ')', tokenArraydst, errTokenRef);
//			}
//			continue;
//		}
//		if(MN_isAnnotation(tk->keyword)) {  // already parsed
//			KLIB kArray_add(kctx, tokenArraydst, tk);
//			if(tk1->keyword == AST_PARENTHESIS) {
//				KLIB kArray_add(kctx, tokenArraydst, tk1);
//				i++;
//			}
//			continue;
//		}
//		if(tk->keyword != TK_INDENT) break;
//		if(*indent == 0) *indent = tk->indent;
//	}
//	for(; i < endIdx ; i++) {
//		kToken *tk = tokenList->tokenItems[i];
//		int topch = Token_topch(tk);
//		if(topch == delim) {
//			return i+1;
//		}
//		if(tk->keyword == TK_ERR) {
//			errTokenRef[0] = tk;
//			continue;
//		}
//		if(tk->keyword == TK_INDENT) {
//			if(tk->indent <= *indent) {
//				return i+1;
//			}
//			continue;
//		}
//		if(kToken_needsKeywordResolved(tk)) {
//			if(topch == '(') {
//				i = makeTree(kctx, ns, AST_PARENTHESIS, tokenList,  i, endIdx, ')', tokenArraydst, errTokenRef);
//				continue;
//			}
//			else if(topch == '[') {
//				i = makeTree(kctx, ns, AST_BRACKET, tokenList, i, endIdx, ']', tokenArraydst, errTokenRef);
//				continue;
//			}
//			i = appendKeyword(kctx, ns, tokenList, i, endIdx, tokenArraydst, errTokenRef);
//		}
//		else {
//			KLIB kArray_add(kctx, tokenArraydst, tk);
//			continue;
//		}
//	}
//	return i;
//}
//static int kStmt_printExpectedRule(KonohaContext *kctx, kStmt *stmt, kToken *tk, kToken *rule, int beginIdx, int canRollBack)
//{
//	if(canRollBack) return beginIdx;
//	if(!Stmt_isERR(stmt)) {
//		kToken_p(stmt, tk, ErrTag, "%s%s expects %s%s", T_statement(stmt->syn->keyword), KW_t(rule->keyword));
//	}
//	return -1;
//}
//
//static int matchSyntaxRule(KonohaContext *kctx, kStmt *stmt, kArray *rules, kArray *tokenList, int beginIdx, int endIdx, int canRollBack)
//{
//	int ruleidx, tokenidx, rulesize = kArray_size(rules);
//	dumpTokenArray(kctx, 0, tokenList, beginIdx, endIdx);
//	dumpTokenArray(kctx, 0, rules, 0, rulesize);
//	tokenidx = beginIdx;
//	for(ruleidx = 0; ruleidx < rulesize && tokenidx < endIdx; ruleidx++) {
//		//DBG_P("matching token=%d<%d, rule=%d<%d", tokenidx, endIdx, ruleidx, rulesize);
//		kToken *rule = rules->tokenItems[ruleidx];
//		kToken *tk = tokenList->tokenItems[tokenidx];
//		if(KW_isPATTERN(rule->keyword)) {
//			SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), rule->keyword);
//			if(syn == NULL || syn->PatternMatch == kmodsugar->UndefinedParseExpr/*NULL*/) {
//				kToken_p(stmt, tk, ErrTag, "unknown syntax pattern: %s%s", KW_t(rule->keyword));
//				return -1;
//			}
//			int c = endIdx;
//			kToken *rule1 = rules->tokenItems[ruleidx+1];
//			if(ruleidx + 1 < rulesize && (!KW_isPATTERN(rule1->keyword) && rule1->keyword != AST_OPTIONAL)) {
//				//DBG_P("NEXT rule=%s%s, rule_size=%d,%d", KW_t(rule1->keyword), ruleidx+1, rulesize);
//				c = TokenArray_findPrefetchedRuleToken(tokenList, tokenidx+1, endIdx, rule1);
//				if(c == -1) {
//					//DBG_P("@");
//					return kStmt_printExpectedRule(kctx, stmt, tk, rule, beginIdx, canRollBack);
//				}
//				ruleidx++;
//			}
//			//DBG_P("syntax rule=%s%s", KW_t(syn->keyword));
//			int next = PatternMatch(kctx, syn, stmt, rule->patternKey, tokenList, tokenidx, c);
//			//DBG_P("matched '%s%s' patternKey='%s%s', next=%d=>%d", KW_t(rule->keyword), KW_t(rule->patternKey), tokenidx, next);
//			if(next == -1) {
//				//DBG_P("@");
//				return kStmt_printExpectedRule(kctx, stmt, tk, rule, beginIdx, canRollBack);
//			}
//			tokenidx = (c == endIdx) ? next : c + 1;
//		}
//		else if(rule->keyword == AST_OPTIONAL) {
//			int next = matchSyntaxRule(kctx, stmt, rule->subTokenList, tokenList, tokenidx, endIdx, 1);
//			if(next == -1) return -1;
//			tokenidx = next;
//		}
//		else {
//			if(rule->keyword != tk->keyword) {
//				//DBG_P("matching rule=%d,%s%s token=%d,%s%s", ri,  KW_t(rule->keyword), ti-s, KW_t(tk->keyword));
//				//DBG_P("@");
//				return kStmt_printExpectedRule(kctx, stmt, tk, rule, beginIdx, canRollBack);
//			}
//			if(rule->keyword == AST_PARENTHESIS || rule->keyword == AST_BRACKET) {
//				int next = matchSyntaxRule(kctx, stmt, rule->subTokenList, tk->subTokenList, 0, kArray_size(tk->subTokenList), 0);
//				if(next == -1) return -1;
//			}
//			tokenidx++;
//		}
//		//DBG_P("matching next token=%d<%d, rule=%d<%d", tokenidx, endIdx, ruleidx+1, rulesize);
//	}
//	if(!canRollBack) {
//		for(; ruleidx < rulesize; ruleidx++) {
//			kToken *rule = rules->tokenItems[ruleidx];
//			if(rule->keyword != AST_OPTIONAL) {
//				kStmt_p(stmt, ErrTag, "%s%s needs syntax pattern: %s%s", T_statement(stmt->syn->keyword), KW_t(rule->keyword));
//				return -1;
//			}
//		}
//		if(tokenidx < endIdx) {
//			kStmt_p(stmt, ErrTag, "%s%s: unexpected token %s", T_statement(stmt->syn->keyword), Token_text(tokenList->tokenItems[tokenidx]));
//			return -1;
//		}
//	}
//	return tokenidx;
//}
//
//static void Block_addStmtLine(KonohaContext *kctx, kBlock *bk, kArray *tokenList, int beginIdx, int endIdx, kToken *errToken)
//{
//	kStmtVar *stmt = new_(StmtVar, tokenList->tokenItems[beginIdx]->uline);
//	KLIB kArray_add(kctx, bk->stmtList, stmt);
//	KINITv(stmt->parentBlockNULL, bk);
//	if(errToken != NULL) {
//		kStmt_toERR(kctx, stmt, errToken->text);
//	}
//	else {
//		int c = Stmt_addAnnotation(kctx, stmt, tokenList, beginIdx, endIdx);
//		kStmt_parseSyntaxRule(kctx, stmt, tokenList, c, endIdx);
//	}
//	DBG_ASSERT(stmt->syn != NULL);
//}

/* ------------------------------------------------------------------------ */

static KMETHOD UndefinedParseExpr(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tokenArray, s, c, e);
	kStmt_p(stmt, ErrTag, "undefined expression parser for '%s'", Token_text(tokenArray->tokenItems[c]));
}

static kExpr *ParseExprFunc(KonohaContext *kctx, SugarSyntax *syn, kFunc *fo, kStmt *stmt, kArray *tokenArray, int s, int c, int e)
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

static kExpr *ParseExpr(KonohaContext *kctx, kStmt *stmt, kArray *tokenArray, int beginIdx, int currentIdx, int endIdx)
{
	SugarSyntax *syn = tokenArray->tokenItems[currentIdx]->resolvedSyntaxInfo;
	kFunc *fo = (syn->ParseExpr == NULL) ? kmodsugar->UndefinedParseExpr : syn->ParseExpr;
	kExpr *texpr;
	if(IS_Array(fo)) {
		int i;
		kArray *a = (kArray*)fo;
		for(i = kArray_size(a) - 1; i > 0; i--) {
			texpr = ParseExprFunc(kctx, syn, fo, stmt, tokenArray, beginIdx, currentIdx, endIdx);
			if(Stmt_isERR(stmt)) return K_NULLEXPR;
			if(texpr != K_NULLEXPR) return texpr;
		}
		fo = a->funcItems[0];
	}
	DBG_ASSERT(IS_Func(fo));
	texpr = ParseExprFunc(kctx, syn, fo, stmt, tokenArray, beginIdx, currentIdx, endIdx);
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
			return ParseExpr(kctx, stmt, tokenArray, beginIdx, idx, endIdx);
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
		if(tk->keyword == KW_COMMA) {
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

typedef struct {
	kArray *tokenList;
	int beginIdx;
	int endIdx;
} RuleEnv;

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
		if(i < endIdx && tokenList->tokenItems[i]->keyword == KW_COMMA) {
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
	KonohaClass *foundClass = NULL;
	int nextIdx = -1;
	kToken *tk = tokenList->tokenItems[beginIdx];
	if(tk->keyword == TK_SYMBOL) {
		foundClass = KLIB kNameSpace_getClass(kctx, ns, S_text(tk->text), S_size(tk->text), NULL);
		if(foundClass != NULL) nextIdx = beginIdx + 1;
	}
	else {
		if(tk->resolvedSyntaxInfo->ty != TY_unknown) {
			nextIdx = beginIdx + 1;
			foundClass = CT_(tk->resolvedSyntaxInfo->ty);
		}
	}
	if(foundClass != NULL) {
		int isAllowedGenerics = true;
		while(nextIdx < endIdx) {
			kToken *tk = tokenList->tokenItems[nextIdx];
			if(tk->keyword != AST_BRACKET) break;
			int sizeofBracketTokens = kArray_size(tk->subTokenList);
			if(isAllowedGenerics &&  sizeofBracketTokens > 0) {  // C[T][]
				KonohaClass *foundGenericClass = kStmt_parseGenerics(kctx, stmt, ns, foundClass, tk->subTokenList, 0, sizeofBracketTokens);
				DBG_P("foundGenericClass=%p", foundGenericClass);
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
		if(ruleToken->keyword == tk->keyword) return i;
	}
	return -1;
}

static int kStmt_printMismatchedRule(KonohaContext *kctx, kStmt *stmt, kToken *tk, kToken *ruleToken, int returnIdx, int canRollBack)
{
	if(!canRollBack && !Stmt_isERR(stmt)) {
		kToken_p(stmt, tk, ErrTag, "%s%s expects %s%s", T_statement(stmt->syn->keyword), KW_t(ruleToken->keyword));
	}
	return returnIdx;
}

#define kTokenArray_skip(TLS, S, E)  S

static int kStmt_matchSyntaxRule(KonohaContext *kctx, kStmt *stmt, kArray *tokenList, int beginIdx, int endIdx, RuleEnv *rule, int canRollBack)
{
	int currentRuleIdx, currentTokenIdx = beginIdx, returnIdx = (canRollBack ? beginIdx : -1);
	DBG_P("Input tokens:");
	KdumpTokenArray(kctx, tokenList, beginIdx, endIdx);
	DBG_P("Syntax rules:");
	KdumpTokenArray(kctx, rule->tokenList, rule->beginIdx, rule->endIdx);
	for(currentRuleIdx = rule->beginIdx; currentRuleIdx < rule->endIdx && currentTokenIdx < endIdx; currentRuleIdx++) {
		kToken *ruleToken = rule->tokenList->tokenItems[currentRuleIdx];
		currentTokenIdx = kTokenArray_skip(tokenList, currentTokenIdx, endIdx);
		if(KW_isPATTERN(ruleToken->keyword)) {
			int patternEndIdx = endIdx;
			kToken *prefetchedRuleToken = rule->tokenList->tokenItems[currentRuleIdx+1];
			if(currentRuleIdx + 1 < rule->endIdx && (!KW_isPATTERN(prefetchedRuleToken->keyword) && prefetchedRuleToken->keyword != AST_OPTIONAL)) {
				patternEndIdx = TokenArray_findPrefetchedRuleToken(tokenList, currentTokenIdx+1, endIdx, prefetchedRuleToken);
				if(patternEndIdx == -1) {
					kToken *tk = tokenList->tokenItems[currentTokenIdx];
					return kStmt_printMismatchedRule(kctx, stmt, tk, ruleToken, returnIdx, canRollBack);
				}
				currentRuleIdx++;
			}
			SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), ruleToken->keyword);
			int next = PatternMatch(kctx, syn, stmt, ruleToken->patternKey, tokenList, currentTokenIdx, patternEndIdx);
			if(next == -1) {
				kToken *tk = tokenList->tokenItems[currentTokenIdx];
				return kStmt_printMismatchedRule(kctx, stmt, tk, ruleToken, returnIdx, canRollBack);
			}
			currentTokenIdx = (patternEndIdx == endIdx) ? next : patternEndIdx + 1;
		}
		else if(ruleToken->keyword == AST_OPTIONAL) {
			RuleEnv nrule = {ruleToken->subTokenList, 0, kArray_size(ruleToken->subTokenList)};
			int next = kStmt_matchSyntaxRule(kctx, stmt, tokenList, currentTokenIdx, endIdx, &nrule, 1/*roolback*/);
			if(next == -1) return returnIdx;
			currentTokenIdx = next;
		}
		else {
			kToken *tk = tokenList->tokenItems[currentTokenIdx];
			if(ruleToken->keyword != tk->keyword) {
				return kStmt_printMismatchedRule(kctx, stmt, tk, ruleToken, beginIdx, canRollBack);
			}
			if(ruleToken->keyword == AST_PARENTHESIS || ruleToken->keyword == AST_BRACKET) {
				RuleEnv nrule = {ruleToken->subTokenList, 0, kArray_size(ruleToken->subTokenList)};
				int next = kStmt_matchSyntaxRule(kctx, stmt, tk->subTokenList, 0, kArray_size(tk->subTokenList), &nrule, 0/*not rollbck*/);
				if(next == -1) return returnIdx;
			}
			currentTokenIdx++;
		}
	}
	DBG_P("rollback=%d, returnIdx=%d, currentTokenIdx=%d < %d", canRollBack, returnIdx, currentTokenIdx, endIdx);
	for(; currentRuleIdx < rule->endIdx; currentRuleIdx++) {
		kToken *ruleToken = rule->tokenList->tokenItems[currentRuleIdx];
		if(ruleToken->keyword != AST_OPTIONAL) {
			if(!canRollBack) {
				kStmt_p(stmt, ErrTag, "%s%s needs syntax pattern: %s%s", T_statement(stmt->syn->keyword), KW_t(ruleToken->keyword));
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
	KdumpTokenArray(kctx, tokenList, beginIdx, endIdx);
	int nextIdx = kStmt_parseTypePattern(kctx, NULL, ns, tokenList, beginIdx, endIdx, NULL);
	if(nextIdx != -1) {
		kToken *tk = TokenArray_nextToken(kctx, tokenList, nextIdx, endIdx);
		if(tk->keyword == TK_SYMBOL) {
			tk = TokenArray_nextToken(kctx, tokenList, nextIdx+1, endIdx);
			if(tk->keyword == AST_PARENTHESIS || tk->keyword == KW_DOT) {
				DBG_P("MethodDecl");
				return SYN_(ns, KW_StmtMethodDecl); //
			}
			DBG_P("TypeDecl");
			return SYN_(ns, KW_StmtTypeDecl);  //
		}
		return SYN_(ns, KW_ExprPattern);
	}
	kToken *tk = tokenList->tokenItems[beginIdx];
	if(tk->keyword == TK_SYMBOL && isUpperCaseSymbol(S_text(tk->text))) {
		kToken *tk1 = TokenArray_nextToken(kctx, tokenList, beginIdx+1, endIdx);
		if(tk1->keyword == KW_LET) {
			DBG_P("Const");
			return SYN_(ns, KW_StmtConstDecl);  // CONSTVAL = ...
		}
	}
	SugarSyntax *syn = tk->resolvedSyntaxInfo;
	DBG_P("tk->kw=%d,%s%s '%s' syn=%p", tk->keyword, KW_t(tk->keyword), Token_text(tk), syn);
	if(syn->syntaxRuleNULL == NULL) {
		return SYN_(ns, KW_ExprPattern);
	}
	return syn;
}

static kbool_t kStmt_parseBySyntaxRule(KonohaContext *kctx, kStmt *stmt, kArray *tokenList, int beginIdx, int endIdx)
{
	kbool_t ret = false;
	SugarSyntax *syn = kNameSpace_getSyntaxRule(kctx, Stmt_nameSpace(stmt), tokenList, beginIdx, endIdx);
	RuleEnv nrule = {syn->syntaxRuleNULL, 0, kArray_size(syn->syntaxRuleNULL)};
	DBG_ASSERT(syn->syntaxRuleNULL != NULL);
	((kStmtVar*)stmt)->syn = syn;
	ret = (kStmt_matchSyntaxRule(kctx, stmt, tokenList, beginIdx, endIdx, &nrule, 0) != -1);
	return ret;
}

// ast

static int kNameSpace_addStrucuredToken(KonohaContext *kctx, ASTEnv *env, ksymbol_t AST_type);

static int kNameSpace_addResolvedToken(KonohaContext *kctx, ASTEnv *env)
{
	kTokenVar *tk = env->tokenList->tokenVarItems[env->beginIdx];
	int topch = Token_topch(tk);
	if(tk->keyword == TK_ERR) {
		env->errToken = tk;
		return env->endIdx;
	}
	if(topch == '(') {
		return kNameSpace_addStrucuredToken(kctx, env, AST_PARENTHESIS);
	}
	if(topch == '[') {
		return kNameSpace_addStrucuredToken(kctx, env, AST_BRACKET);
	}
	if(topch == '@' && env->beginIdx + 1 < env->endIdx) {
		kTokenVar *tk1 = env->tokenList->tokenVarItems[env->beginIdx+1];
		if(tk1->keyword == TK_SYMBOL) {
			tk1->keyword = ksymbolA(S_text(tk1->text), S_size(tk1->text), SYM_NEWID) | MN_Annotation;
			tk1->resolvedSyntaxInfo = env->symbolSyntaxInfo;
			KLIB kArray_add(kctx, env->stmtTokenList, tk1);
			return env->beginIdx + 1;
		}
	}
	if(tk->keyword == TK_SYMBOL) {
		const char *t = S_text(tk->text);
		SugarSyntax *syn = SYN_(env->ns, ksymbolA(t, S_size(tk->text), SYM_NEWID));
		if(syn != NULL) {
			tk->keyword = syn->keyword;
			tk->resolvedSyntaxInfo = syn;
			KLIB kArray_add(kctx, env->stmtTokenList, tk);
			return env->beginIdx;
		}
		if(!isalpha(t[0])) {
			Token_pERR(kctx, tk, "undefined token: %s", Token_text(tk));
			env->errToken = tk;
			return env->endIdx;
		}
		tk->resolvedSyntaxInfo = env->symbolSyntaxInfo;
		KLIB kArray_add(kctx, env->stmtTokenList, tk);
		return env->beginIdx;
	}
	SugarSyntax *syn = SYN_(env->ns, tk->keyword);
	if(syn != NULL) {
		tk->resolvedSyntaxInfo = syn;
		KLIB kArray_add(kctx, env->stmtTokenList, tk);
	}
	return 	env->beginIdx;
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

static kbool_t checkEndOfParenthesis(KonohaContext *kctx, kArray *tokenList, int *currentIdx, int endIdx, int *probablyCloseBeforeRef, kArray *stmtTokenList, int beginIdx)
{
	kToken *tk = tokenList->tokenItems[*currentIdx];
	int topch = Token_topch(tk);
	return (topch == ')');
}

static kbool_t checkEndOfBracket(KonohaContext *kctx, kArray *tokenList, int *currentIdx, int endIdx, int *probablyCloseBeforeRef, kArray *stmtTokenList, int beginIdx)
{
	kToken *tk = tokenList->tokenItems[*currentIdx];
	int topch = Token_topch(tk);
	return (topch == ']');
}

static int kNameSpace_addStrucuredToken(KonohaContext *kctx, ASTEnv *env, ksymbol_t AST_type)
{
	ASTEnv newenv;
	int probablyCloseBefore = env->endIdx - 1;
	kTokenVar *astToken = new_(TokenVar, AST_type);
	DBG_ASSERT(IS_Array(env->stmtTokenList));
	KLIB kArray_add(kctx, env->stmtTokenList, astToken);
	astToken->resolvedSyntaxInfo = SYN_(env->ns, AST_type);
	KSETv(astToken->subTokenList, new_(TokenArray, 0));
	astToken->uline = env->tokenList->tokenItems[env->beginIdx]->uline;
	newenv = *env;
	newenv.beginIdx = env->beginIdx + 1;
	newenv.stmtTokenList = astToken->subTokenList;
	CheckEndOfStmtFunc f = (AST_type == AST_PARENTHESIS) ? checkEndOfParenthesis : checkEndOfBracket;
	int returnIdx = kNameSpace_selectStmtTokenList(kctx, &newenv, &probablyCloseBefore, f);
	if(returnIdx == env->endIdx && env->errToken != NULL) {
		int closech = (AST_type == AST_PARENTHESIS) ? ')': ']';
		Token_pERR(kctx, astToken, "'%c' is expected (probably before %s)", closech, Token_text(env->tokenList->tokenItems[probablyCloseBefore]));
		env->errToken = astToken;
	}
	return returnIdx;
}

static int kStmt_addAnnotation(KonohaContext *kctx, kStmt *stmt, kArray *tokenList, int beginIdx, int endIdx)
{
	int i;
	for(i = beginIdx; i < endIdx; i++) {
		kToken *tk = tokenList->tokenItems[i];
		if(!MN_isAnnotation(tk->keyword)) break;
		if(i + 1 < endIdx) {
			kToken *tk1 = tokenList->tokenItems[i+1];
			kObject *value = UPCAST(K_TRUE);
			if(tk1->keyword == AST_PARENTHESIS) {
				value = (kObject*)kStmt_parseExpr(kctx, stmt, tk1->subTokenList, 0, kArray_size(tk1->subTokenList));
				i++;
			}
			if(value != NULL) {
				KLIB kObject_setObject(kctx, stmt, tk->keyword, O_classId(value), value);
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
	if(Token_topch(tk) == ';') {
		found = true;
		do {
			*currentIdx += 1;
			if(!(*currentIdx < endIdx)) break;
			tk = tokenList->tokenItems[*currentIdx];
		}while(Token_topch(tk) == ';');
	}
	if(tk->keyword == TK_INDENT) {
		DBG_P("!! previous=%d, indent=%d", indentRef[0], tk->indent);
		found = (kArray_size(stmtTokenList) > beginIdx);
	}
	return found;
}

static kbool_t Comma(KonohaContext *kctx, kArray *tokenList, int *currentIdx, int endIdx, int *optionRef, kArray *stmtTokenList, int beginIdx)
{
	kToken *tk = tokenList->tokenItems[*currentIdx];
	if(Token_topch(tk) == ',') {
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
	env.symbolSyntaxInfo = SYN_(ns, TK_SYMBOL);
	int i = beginIdx, indent = 0, atop = kArray_size(tokenList);
	if(isEndOfStmt == NULL) {
		isEndOfStmt = SemiColon;
	}
	while(i < endIdx) {
		DBG_ASSERT(atop == kArray_size(tokenList));
		env.beginIdx = i;
		i = kNameSpace_selectStmtTokenList(kctx, &env, &indent, isEndOfStmt);
		int asize = kArray_size(tokenList);
		if(asize > atop) {
			kBlock_addNewStmt(kctx, bk, tokenList, atop, asize, env.errToken);
			KLIB kArray_clear(kctx, tokenList, atop);
		}
	}
	return (kBlock*)bk;
}

static kbool_t Token_toBRACE(KonohaContext *kctx, kTokenVar *tk, kNameSpace *ns)
{
	if(tk->keyword == TK_CODE) {
		INIT_GCSTACK();
		kArray *a = new_(TokenArray, 0);
		PUSH_GCSTACK(a);
		kNameSpace_tokenize(kctx, ns, S_text(tk->text), tk->uline, a);
		tk->keyword = AST_BRACE;
		KSETv(tk->subTokenList, a);
		RESET_GCSTACK();
		return 1;
	}
	return 0;
}

static kBlock* kStmt_getBlock(KonohaContext *kctx, kStmt *stmt, ksymbol_t kw, kBlock *def)
{
	kBlock *bk = (kBlock*)kStmt_getObjectNULL(kctx, stmt, kw);
	if(bk != NULL) {
		if(IS_Token(bk)) {
			kToken *tk = (kToken*)bk;
			if (tk->keyword == TK_CODE) {
				Token_toBRACE(kctx, (kTokenVar*)tk, Stmt_nameSpace(stmt));
			}
			if (tk->keyword == AST_BRACE) {
				bk = new_Block(kctx, Stmt_nameSpace(stmt), stmt, tk->subTokenList, 0, kArray_size(tk->subTokenList), SemiColon);
				KLIB kObject_setObject(kctx, stmt, kw, TY_Block, bk);
			}
		}
		if(IS_Block(bk)) return bk;
	}
	return def;
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
