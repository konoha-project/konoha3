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
// Block

static int selectStmtLine(KonohaContext *kctx, kNameSpace *ns, int *indent, kArray *tokenList, int beginIdx, int endIdx, int delim, kArray *tokenArraydst, kToken **tkERRRef);
static void Block_addStmtLine(KonohaContext *kctx, kBlock *bk, kArray *tokenList, int beginIdx, int end, kToken *tkERR);
static int makeTree(KonohaContext *kctx, kNameSpace *ns, ksymbol_t tt, kArray *tokenList, int beginIdx, int endIdx, int closech, kArray *tokenArraydst, kToken **tkERRRef);

static kBlock *new_Block(KonohaContext *kctx, kNameSpace *ns, kStmt *parent, kArray *tokenList, int beginIdx, int endIdx, int delim)
{
	kBlockVar *bk = GCSAFE_new(BlockVar, ns);
	if(parent != NULL) {
		KINITv(bk->parentStmtNULL, parent);
	}
	int i = beginIdx, indent = 0, atop = kArray_size(tokenList);
	while(i < endIdx) {
		kToken *tkERR = NULL;
		DBG_ASSERT(atop == kArray_size(tokenList));
		i = selectStmtLine(kctx, ns, &indent, tokenList, i, endIdx, delim, tokenList, &tkERR);
		int asize = kArray_size(tokenList);
		if(asize > atop) {
			Block_addStmtLine(kctx, bk, tokenList, atop, asize, tkERR);
			KLIB kArray_clear(kctx, tokenList, atop);
		}
	}
	return (kBlock*)bk;
}

static kbool_t Token_resolved(KonohaContext *kctx, kNameSpace *ns, kTokenVar *tk)
{
	ksymbol_t kw = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NONAME);
	if(kw != SYM_NONAME) {
		SugarSyntax *syn = SYN_(ns, kw);
		if(syn != NULL) {
			if(syn->ty != TY_unknown) {
				tk->keyword = KW_TypePattern;
				tk->ty = syn->ty;
			}
			else {
				tk->keyword = kw;
			}
			return 1;
		}
	}
	return 0;
}

static kTokenVar* TokenType_resolveGenerics(KonohaContext *kctx, kNameSpace *ns, kTokenVar *tk, kToken *tkP)
{
	size_t i, psize= 0, size = kArray_size(tkP->sub);
	kparamtype_t p[size];
	for(i = 0; i < size; i++) {
		kToken *tkT = tkP->sub->tokenItems[i];
		if(tkT->keyword == KW_COMMA) continue;
		if(TK_isType(tkT)) {
			p[psize].ty = TK_type(tkT);
			psize++;
			continue;
		}
		return NULL; // new int[10];  // not generics
	}
	KonohaClass *ct = NULL;
	if(psize > 0) {
		ct = CT_(TK_type(tk));
		if(ct->baseclassId == TY_Func) {
			ct = KLIB KonohaClass_Generics(kctx, ct, p[0].ty, psize-1, p+1);
		}
		else if(ct->p0 == TY_void) {
			Token_pERR(kctx, tk, "not generic type: %s", TY_t(TK_type(tk)));
			return tk;
		}
		else {
			ct = KLIB KonohaClass_Generics(kctx, ct, TY_void, psize, p);
		}
	}
	else {
		ct = CT_p0(kctx, CT_Array, TK_type(tk));
	}
	tk->ty = ct->classId;
	return tk;
}

static int appendKeyword(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx, kArray *dst, kToken **tkERR)
{
	int next = beginIdx; // don't add
	kTokenVar *tk = tokenList->tokenVarItems[beginIdx];
	if(tk->keyword == TK_SYMBOL) {
		if(!Token_resolved(kctx, ns, tk)) {
			const char *t = S_text(tk->text);
			if(isalpha(t[0])) {
				KonohaClass *ct = KLIB kNameSpace_getClass(kctx, ns, NULL/*FIXME*/, S_text(tk->text), S_size(tk->text), TY_unknown);
				if(ct != NULL) {
					tk->keyword = KW_TypePattern;
					tk->ty = ct->classId;
				}
			}
			else {
				Token_pERR(kctx, tk, "undefined token: %s", Token_text(tk));
				tkERR[0] = tk;
				return endIdx;
			}
		}
	}
	Token_textetUnresolved(tk, false);
	if(TK_isType(tk)) {   // trying to resolve Type[Type, Type]
		KLIB kArray_add(kctx, dst, tk);
		while(next + 1 < endIdx) {
			kToken *tkB = tokenList->tokenItems[next + 1];
			int topch = kToken_topch(tkB);
			if(topch != '[') break;
			kArray *abuf = ctxsugar->preparedTokenList;
			size_t atop = kArray_size(abuf);
			next = makeTree(kctx, ns, AST_BRACKET, tokenList,  next+1, endIdx, ']', abuf, tkERR);
			if(!(kArray_size(abuf) > atop)) return next;
			tkB = abuf->tokenItems[atop];
			if(tkB->keyword == AST_BRACKET) {
				tk = TokenType_resolveGenerics(kctx, ns, tk, tkB);
				if(tk == NULL) {
					if(abuf != dst) {
						KLIB kArray_add(kctx, dst, tkB);
						KLIB kArray_clear(kctx, abuf, atop);
					}
					return next;
				}
			}
			KLIB kArray_clear(kctx, abuf, atop);
		}
	}
	else {
		KLIB kArray_add(kctx, dst, tk);
	}
	return next;
}

static kbool_t Token_toBRACE(KonohaContext *kctx, kTokenVar *tk, kNameSpace *ns)
{
	if(tk->keyword == TK_CODE) {
		INIT_GCSTACK();
		kArray *a = new_(TokenArray, 0);
		PUSH_GCSTACK(a);
		kNameSpace_tokenize(kctx, ns, S_text(tk->text), tk->uline, a);
		tk->keyword = AST_BRACE;
		KSETv(tk->sub, a);
		RESET_GCSTACK();
		return 1;
	}
	return 0;
}

static int makeTree(KonohaContext *kctx, kNameSpace *ns, ksymbol_t astkw, kArray *tokenList, int beginIdx, int endIdx, int closech, kArray *tokenArraydst, kToken **tkERRRef)
{
	int i, probablyCloseBefore = endIdx - 1;
	kToken *tk = tokenList->tokenItems[beginIdx];
	kTokenVar *tkP = new_(TokenVar, 0);
	KLIB kArray_add(kctx, tokenArraydst, tkP);
	tkP->keyword = astkw;
	tkP->uline = tk->uline;
	KSETv(tkP->sub, new_(TokenArray, 0));
	for(i = beginIdx + 1; i < endIdx; i++) {
		tk = tokenList->tokenItems[i];
		if(tk->keyword == TK_ERR) break;  // ERR
		if(!kToken_needsKeywordResolved(tk)) {
			KLIB kArray_add(kctx, tkP->sub, tk);
			continue;
		}
		int topch = kToken_topch(tk);
		DBG_ASSERT(topch != '{');
		if(topch == '(') {
			i = makeTree(kctx, ns, AST_PARENTHESIS, tokenList, i, endIdx, ')', tkP->sub, tkERRRef);
			continue;
		}
		if(topch == '[') {
			i = makeTree(kctx, ns, AST_BRACKET, tokenList, i, endIdx, ']', tkP->sub, tkERRRef);
			continue;
		}
		if(topch == closech) {
			return i;
		}
		if(closech != '}') {
			if(tk->keyword == TK_INDENT) continue; // remove INDENT from tokens;
			if(tk->keyword == TK_CODE) probablyCloseBefore = i;
		}
		i = appendKeyword(kctx, ns, tokenList, i, endIdx, tkP->sub, tkERRRef);
	}
	if(tk->keyword != TK_ERR) {
		Token_pERR(kctx, tkP, "'%c' is expected (probably before %s)", closech, Token_text(tokenList->tokenItems[probablyCloseBefore]));
	}
	else {
		tkP->keyword = TK_ERR;
		KSETv(tkP->text, tk->text);
	}
	tkERRRef[0] = tkP;
	return endIdx;
}

static int selectStmtLine(KonohaContext *kctx, kNameSpace *ns, int *indent, kArray *tokenList, int beginIdx, int endIdx, int delim, kArray *tokenArraydst, kToken **tkERRRef)
{
	int i = beginIdx;
	DBG_ASSERT(endIdx <= kArray_size(tokenList));
	KdumpTokenArray(kctx, tokenList, beginIdx, endIdx);
	for(; i < endIdx - 1; i++) {
		kToken *tk = tokenList->tokenItems[i];
		kTokenVar *tk1 = tokenList->tokenVarItems[i+1];
		int topch = kToken_topch(tk);
		if(topch == '@' && (tk1->keyword == TK_SYMBOL)) {
			tk1->keyword = ksymbolA(S_text(tk1->text), S_size(tk1->text), SYM_NEWID) | MN_Annotation;
			KLIB kArray_add(kctx, tokenArraydst, tk1); i++;
			tk1 = tokenList->tokenVarItems[i+1];
			topch = kToken_topch(tk1);
			if(i + 1 < endIdx && topch == '(') {
				i = makeTree(kctx, ns, AST_PARENTHESIS, tokenList, i+1, endIdx, ')', tokenArraydst, tkERRRef);
			}
			continue;
		}
		if(MN_isAnnotation(tk->keyword)) {  // already parsed
			KLIB kArray_add(kctx, tokenArraydst, tk);
			if(tk1->keyword == AST_PARENTHESIS) {
				KLIB kArray_add(kctx, tokenArraydst, tk1);
				i++;
			}
			continue;
		}
		if(tk->keyword != TK_INDENT) break;
		if(*indent == 0) *indent = tk->indent;
	}
	for(; i < endIdx ; i++) {
		kToken *tk = tokenList->tokenItems[i];
		int topch = kToken_topch(tk);
		if(topch == delim) {
			return i+1;
		}
		if(tk->keyword == TK_ERR) {
			tkERRRef[0] = tk;
			continue;
		}
		if(tk->keyword == TK_INDENT) {
			if(tk->indent <= *indent) {
				return i+1;
			}
			continue;
		}
		if(kToken_needsKeywordResolved(tk)) {
			if(topch == '(') {
				i = makeTree(kctx, ns, AST_PARENTHESIS, tokenList,  i, endIdx, ')', tokenArraydst, tkERRRef);
				continue;
			}
			else if(topch == '[') {
				i = makeTree(kctx, ns, AST_BRACKET, tokenList, i, endIdx, ']', tokenArraydst, tkERRRef);
				continue;
			}
			i = appendKeyword(kctx, ns, tokenList, i, endIdx, tokenArraydst, tkERRRef);
		}
		else {
			KLIB kArray_add(kctx, tokenArraydst, tk);
			continue;
		}
	}
	return i;
}

static kExpr* kStmt_parseExpr(KonohaContext *kctx, kStmt *stmt, kArray *tokenArray, int s, int e);

static int Stmt_addAnnotation(KonohaContext *kctx, kStmt *stmt, kArray *tokenList, int beginIdx, int endIdx)
{
	int i;
	for(i = beginIdx; i < endIdx; i++) {
		kToken *tk = tokenList->tokenItems[i];
		if(!MN_isAnnotation(tk->keyword)) break;
		if(i+1 < endIdx) {
			kToken *tk1 = tokenList->tokenItems[i+1];
			kObject *value = UPCAST(K_TRUE);
			if(tk1->keyword == AST_PARENTHESIS) {
				value = (kObject*)kStmt_parseExpr(kctx, stmt, tk1->sub, 0, kArray_size(tk1->sub));
				i++;
			}
			if(value != NULL) {
				KLIB kObject_setObject(kctx, stmt, tk->keyword, O_classId(value), value);
			}
		}
	}
	return i;
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

static int lookAheadKeyword(kArray *tokenList, int beginIdx, int endIdx, kToken *rule)
{
	int i;
	for(i = beginIdx; i < endIdx; i++) {
		kToken *tk = tokenList->tokenItems[i];
		if(rule->keyword == tk->keyword) return i;
	}
	return -1;
}

static int kStmt_printExpectedRule(KonohaContext *kctx, kStmt *stmt, kToken *tk, kToken *rule, int beginIdx, int canRollBack)
{
	if(canRollBack) return beginIdx;
	if(!Stmt_isERR(stmt)) {
		kToken_p(stmt, tk, ErrTag, "%s%s expects %s%s", T_statement(stmt->syn->keyword), KW_t(rule->keyword));
	}
	return -1;
}

static int matchSyntaxRule(KonohaContext *kctx, kStmt *stmt, kArray *rules, kArray *tokenList, int beginIdx, int endIdx, int canRollBack)
{
	int ruleidx, tokenidx, rulesize = kArray_size(rules);
	dumpTokenArray(kctx, 0, tokenList, beginIdx, endIdx);
	dumpTokenArray(kctx, 0, rules, 0, rulesize);
	tokenidx = beginIdx;
	for(ruleidx = 0; ruleidx < rulesize && tokenidx < endIdx; ruleidx++) {
		//DBG_P("matching token=%d<%d, rule=%d<%d", tokenidx, endIdx, ruleidx, rulesize);
		kToken *rule = rules->tokenItems[ruleidx];
		kToken *tk = tokenList->tokenItems[tokenidx];
		if(KW_isPATTERN(rule->keyword)) {
			SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), rule->keyword);
			if(syn == NULL || syn->PatternMatch == kmodsugar->UndefinedParseExpr/*NULL*/) {
				kToken_p(stmt, tk, ErrTag, "unknown syntax pattern: %s%s", KW_t(rule->keyword));
				return -1;
			}
			int c = endIdx;
			kToken *rule1 = rules->tokenItems[ruleidx+1];
			if(ruleidx + 1 < rulesize && (!KW_isPATTERN(rule1->keyword) && rule1->keyword != AST_OPTIONAL)) {
				//DBG_P("NEXT rule=%s%s, rule_size=%d,%d", KW_t(rule1->keyword), ruleidx+1, rulesize);
				c = lookAheadKeyword(tokenList, tokenidx+1, endIdx, rule1);
				if(c == -1) {
					//DBG_P("@");
					return kStmt_printExpectedRule(kctx, stmt, tk, rule, beginIdx, canRollBack);
				}
				ruleidx++;
			}
			//DBG_P("syntax rule=%s%s", KW_t(syn->keyword));
			int next = PatternMatch(kctx, syn, stmt, rule->patternKey, tokenList, tokenidx, c);
			//DBG_P("matched '%s%s' patternKey='%s%s', next=%d=>%d", KW_t(rule->keyword), KW_t(rule->patternKey), tokenidx, next);
			if(next == -1) {
				//DBG_P("@");
				return kStmt_printExpectedRule(kctx, stmt, tk, rule, beginIdx, canRollBack);
			}
			tokenidx = (c == endIdx) ? next : c + 1;
		}
		else if(rule->keyword == AST_OPTIONAL) {
			int next = matchSyntaxRule(kctx, stmt, rule->sub, tokenList, tokenidx, endIdx, 1);
			if(next == -1) return -1;
			tokenidx = next;
		}
		else {
			if(rule->keyword != tk->keyword) {
				//DBG_P("matching rule=%d,%s%s token=%d,%s%s", ri,  KW_t(rule->keyword), ti-s, KW_t(tk->keyword));
				//DBG_P("@");
				return kStmt_printExpectedRule(kctx, stmt, tk, rule, beginIdx, canRollBack);
			}
			if(rule->keyword == AST_PARENTHESIS || rule->keyword == AST_BRACKET) {
				int next = matchSyntaxRule(kctx, stmt, rule->sub, tk->sub, 0, kArray_size(tk->sub), 0);
				if(next == -1) return -1;
			}
			tokenidx++;
		}
		//DBG_P("matching next token=%d<%d, rule=%d<%d", tokenidx, endIdx, ruleidx+1, rulesize);
	}
	if(!canRollBack) {
		for(; ruleidx < rulesize; ruleidx++) {
			kToken *rule = rules->tokenItems[ruleidx];
			if(rule->keyword != AST_OPTIONAL) {
				kStmt_p(stmt, ErrTag, "%s%s needs syntax pattern: %s%s", T_statement(stmt->syn->keyword), KW_t(rule->keyword));
				return -1;
			}
		}
		if(tokenidx < endIdx) {
			kStmt_p(stmt, ErrTag, "%s%s: unexpected token %s", T_statement(stmt->syn->keyword), Token_text(tokenList->tokenItems[tokenidx]));
			return -1;
		}
	}
	return tokenidx;
}

static inline kToken* TokenArray_nextToken(KonohaContext *kctx, kArray *tokenList, int beginIdx, int endIdx)
{
	return (beginIdx < endIdx) ? tokenList->tokenItems[beginIdx] : K_NULLTOKEN;
}

static SugarSyntax* NameSpace_getSyntaxRule(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx)
{
	kToken *tk = tokenList->tokenItems[beginIdx];
	if(TK_isType(tk)) {
		tk = TokenArray_nextToken(kctx, tokenList, beginIdx+1, endIdx);
		if(tk->keyword == TK_SYMBOL || tk->keyword == TK_TYPE) {
			tk = TokenArray_nextToken(kctx, tokenList, beginIdx+2, endIdx);
			if(tk->keyword == AST_PARENTHESIS || tk->keyword == KW_DOT) {
				DBG_P("MethodDecl");
				return SYN_(ns, KW_StmtMethodDecl); //
			}
			DBG_P("TypeDecl");
			return SYN_(ns, KW_StmtTypeDecl);  //
		}
		DBG_P("Expression");
		return SYN_(ns, KW_ExprPattern);  // expression
	}
	if(tk->keyword == TK_SYMBOL && isUpperCaseSymbol(S_text(tk->text))) {
		kToken *tk1 = TokenArray_nextToken(kctx, tokenList, beginIdx+1, endIdx);
		if(tk1->keyword == KW_LET) {
			DBG_P("Const");
			return SYN_(ns, KW_StmtConstDecl);  // CONSTVAL = ...
		}
		return SYN_(ns, KW_ExprPattern);
	}
	SugarSyntax *syn = SYN_(ns, tk->keyword);
	DBG_P("tk->keyword=%d,%d, tk->keyword=%s%s, syn=%p", tk->keyword, SYM_UNMASK(tk->keyword), KW_t(tk->keyword), syn);
	DBG_ASSERT(syn != NULL);
	if(syn->syntaxRuleNULL == NULL) {
		int i;
		for(i = beginIdx + 1; i < endIdx; i++) {
			tk = tokenList->tokenItems[i];
			syn = SYN_(ns, tk->keyword);
			//DBG_P("@ tk->keyword=%s%s, syn=%p", KW_t(tk->keyword), syn);
			if(syn->syntaxRuleNULL != NULL && syn->priority > 0) {
				return syn;
			}
		}
		return SYN_(ns, KW_ExprPattern);
	}
	return syn;
}

static kbool_t Stmt_parseSyntaxRule(KonohaContext *kctx, kStmt *stmt, kArray *tokenList, int beginIdx, int endIdx)
{
	kbool_t ret = false;
	SugarSyntax *syn = NameSpace_getSyntaxRule(kctx, Stmt_nameSpace(stmt), tokenList, beginIdx, endIdx);
	DBG_ASSERT(syn != NULL);
	if(syn->syntaxRuleNULL != NULL) {
		((kStmtVar*)stmt)->syn = syn;
		ret = (matchSyntaxRule(kctx, stmt, syn->syntaxRuleNULL, tokenList, beginIdx, endIdx, 0) != -1);
	}
	else {
		kStmt_p(stmt, ErrTag, "undefined syntax rule for '%s%s'", KW_t(syn->keyword));
	}
	return ret;
}

static void Block_addStmtLine(KonohaContext *kctx, kBlock *bk, kArray *tokenList, int beginIdx, int endIdx, kToken *tkERR)
{
	kStmtVar *stmt = new_(StmtVar, tokenList->tokenItems[beginIdx]->uline);
	KLIB kArray_add(kctx, bk->stmtList, stmt);
	KINITv(stmt->parentBlockNULL, bk);
	if(tkERR != NULL) {
		kStmt_toERR(kctx, stmt, tkERR->text);
	}
	else {
		int c = Stmt_addAnnotation(kctx, stmt, tokenList, beginIdx, endIdx);
		Stmt_parseSyntaxRule(kctx, stmt, tokenList, c, endIdx);
	}
	DBG_ASSERT(stmt->syn != NULL);
}

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

static kExpr *ParseExpr(KonohaContext *kctx, SugarSyntax *syn, kStmt *stmt, kArray *tokenArray, int s, int c, int e)
{
	kFunc *fo = (syn == NULL || syn->ParseExpr == NULL) ? kmodsugar->UndefinedParseExpr : syn->ParseExpr;
	kExpr *texpr;
	if(IS_Array(fo)) {
		int i;
		kArray *a = (kArray*)fo;
		for(i = kArray_size(a) - 1; i > 0; i--) {
			texpr = ParseExprFunc(kctx, syn, fo, stmt, tokenArray, s, c, e);
			if(Stmt_isERR(stmt)) return K_NULLEXPR;
			if(texpr != K_NULLEXPR) return texpr;
		}
		fo = a->funcItems[0];
	}
	DBG_ASSERT(IS_Func(fo));
	texpr = ParseExprFunc(kctx, syn, fo, stmt, tokenArray, s, c, e);
	if(texpr == K_NULLEXPR && !Stmt_isERR(stmt)) {
		kStmt_p(stmt, ErrTag, "syntax error: operator %s", Token_text(tokenArray->tokenItems[c]));
	}
	return texpr;
}

/* ------------------------------------------------------------------------ */

static kbool_t Stmt_isUnaryOp(KonohaContext *kctx, kStmt *stmt, kToken *tk)
{
	SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), tk->keyword);
	return (syn->op1 != SYM_NONAME);
}

static int Stmt_skipUnaryOp(KonohaContext *kctx, kStmt *stmt, kArray *tokenArray, int s, int e)
{
	int i;
	for(i = s; i < e; i++) {
		kToken *tk = tokenArray->tokenItems[i];
		if(!Stmt_isUnaryOp(kctx, stmt, tk)) break;
	}
	return i;
}

static int Stmt_findBinaryOp(KonohaContext *kctx, kStmt *stmt, kArray *tokenArray, int s, int e, SugarSyntax **synRef)
{
	int idx = -1, i, prif = 0;
	for(i = Stmt_skipUnaryOp(kctx, stmt, tokenArray, s, e) + 1; i < e; i++) {
		kToken *tk = tokenArray->tokenItems[i];
		SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), tk->keyword);
		if(syn->priority > 0) {
			if(prif < syn->priority || (prif == syn->priority && !(FLAG_is(syn->flag, SYNFLAG_ExprLeftJoinOp2)) )) {
				prif = syn->priority;
				idx = i;
				*synRef = syn;
			}
			if(!FLAG_is(syn->flag, SYNFLAG_ExprPostfixOp2)) {  /* check if real binary operator to parse f() + 1 */
				i = Stmt_skipUnaryOp(kctx, stmt, tokenArray, i+1, e) - 1;
			}
		}
	}
	return idx;
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

static kExpr* kStmt_parseExpr(KonohaContext *kctx, kStmt *stmt, kArray *tokenArray, int s, int e)
{
	if(!Stmt_isERR(stmt)) {
		if(s < e) {
			SugarSyntax *syn = NULL;
			int idx = Stmt_findBinaryOp(kctx, stmt, tokenArray, s, e, &syn);
			if(idx != -1) {
				//DBG_P("** Found BinaryOp: s=%d, idx=%d, e=%d, '%s'**", s, idx, e, Token_text(tokenArray->tokenItems[idx]));
				return ParseExpr(kctx, syn, stmt, tokenArray, s, idx, e);
			}
			int c = s;
			syn = SYN_(Stmt_nameSpace(stmt), (tokenArray->tokenItems[c])->keyword);
			return ParseExpr(kctx, syn, stmt, tokenArray, c, c, e);
		}
		if (0 < s - 1) {
			kStmt_p(stmt, ErrTag, "expected expression after %s", Token_text(tokenArray->tokenItems[s-1]));
		}
		else if(e < kArray_size(tokenArray)) {
			kStmt_p(stmt, ErrTag, "expected expression before %s", Token_text(tokenArray->tokenItems[e]));
		}
		else {
			kStmt_p(stmt, ErrTag, "expected expression");
		}
	}
	return K_NULLEXPR;
}

static kExpr *kStmt_rightJoinExpr(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kArray *tokenArray, int c, int e)
{
	if(c < e && expr != K_NULLEXPR && !Stmt_isERR(stmt)) {
		WARN_IgnoredTokens(kctx, tokenArray, c, e);
	}
	return expr;
}

static KMETHOD ParseExpr_Term(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tokenArray, s, c, e);
	DBG_ASSERT(s == c);
	kToken *tk = tokenArray->tokenItems[c];
	kExprVar *expr = new_(ExprVar, SYN_(Stmt_nameSpace(stmt), tk->keyword));
	KSETv(expr->termToken, tk);
	Expr_setTerm(expr, 1);
	RETURN_(kStmt_rightJoinExpr(kctx, stmt, expr, tokenArray, c+1, e));
}

static KMETHOD ParseExpr_Op(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tokenArray, s, c, e);
	kTokenVar *tk = tokenArray->tokenVarItems[c];
	kExpr *expr, *rexpr = kStmt_parseExpr(kctx, stmt, tokenArray, c+1, e);
	kmethodn_t mn = (s == c) ? syn->op1 : syn->op2;
	if(mn != SYM_NONAME && syn->ExprTyCheck == kmodsugar->UndefinedExprTyCheck) {
		tk->keyword = mn;
		syn = SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall);  // switch type checker
	}
	if(s == c) { // unary operator
		expr = new_ConsExpr(kctx, syn, 2, tk, rexpr);
	}
	else {   // binary operator
		kExpr *lexpr = kStmt_parseExpr(kctx, stmt, tokenArray, s, c);
		expr = new_ConsExpr(kctx, syn, 3, tk, lexpr, rexpr);
	}
	RETURN_(expr);
}

static inline kbool_t isFieldName(kArray *tokenArray, int c, int e)
{
	if(c + 1 < e) {
		kToken *tk = tokenArray->tokenItems[c+1];
		return (tk->keyword == TK_SYMBOL);
	}
	return false;
}
static KMETHOD ParseExpr_DOT(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tokenArray, s, c, e);
	if(s < c && isFieldName(tokenArray, c, e)) {
		kExpr *expr = kStmt_parseExpr(kctx, stmt, tokenArray, s, c);
		expr = new_ConsExpr(kctx, syn, 2, tokenArray->tokenItems[c+1], expr);
		RETURN_(kStmt_rightJoinExpr(kctx, stmt, expr, tokenArray, c+2, e));
	}
}

static KMETHOD ParseExpr_Parenthesis(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tokenArray, s, c, e);
	kToken *tk = tokenArray->tokenItems[c];
	if(s == c) {
		kExpr *expr = kStmt_parseExpr(kctx, stmt, tk->sub, 0, kArray_size(tk->sub));
		RETURN_(kStmt_rightJoinExpr(kctx, stmt, expr, tokenArray, c+1, e));
	}
	else {
		kExpr *lexpr = kStmt_parseExpr(kctx, stmt, tokenArray, s, c);
		if(lexpr == K_NULLEXPR) {
			RETURN_(lexpr);
		}
		if(lexpr->syn->keyword == KW_DOT) {
			((kExprVar*)lexpr)->syn = SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall); // CALL
		}
		else if(lexpr->syn->keyword != KW_ExprMethodCall) {
			syn = SYN_(Stmt_nameSpace(stmt), KW_ParenthesisPattern);    // (f null ())
			lexpr  = new_ConsExpr(kctx, syn, 2, lexpr, K_NULL);
		}
		lexpr = kStmt_addExprParam(kctx, stmt, lexpr, tk->sub, 0, kArray_size(tk->sub), 1/*allowEmpty*/);
		RETURN_(kStmt_rightJoinExpr(kctx, stmt, lexpr, tokenArray, c+1, e));
	}
}

static KMETHOD ParseExpr_COMMA(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tokenArray, s, c, e);
	kExpr *expr = new_ConsExpr(kctx, syn, 1, tokenArray->tokenItems[c]);
	expr = kStmt_addExprParam(kctx, stmt, expr, tokenArray, s, e, 0/*allowEmpty*/);
	RETURN_(expr);
}

static KMETHOD ParseExpr_DOLLAR(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tokenArray, s, c, e);
	if(s == c && c + 1 < e) {
		kToken *tk = tokenArray->tokenItems[c+1];
		if(tk->keyword == TK_CODE) {
			Token_toBRACE(kctx, (kTokenVar*)tk, Stmt_nameSpace(stmt));
		}
		if(tk->keyword == AST_BRACE) {
			kExprVar *expr = GCSAFE_new(ExprVar, SYN_(Stmt_nameSpace(stmt), KW_BlockPattern));
			KSETv(expr->block, new_Block(kctx, Stmt_nameSpace(stmt), stmt, tk->sub, 0, kArray_size(tk->sub), ';'));
			RETURN_(expr);
		}
	}
}

/* ------------------------------------------------------------------------ */

static KMETHOD PatternMatch_Expr(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenArray, s, e);
	INIT_GCSTACK();
	int r = -1;
	dumpTokenArray(kctx, 0, tokenArray, s, e);
	kExpr *expr = kStmt_parseExpr(kctx, stmt, tokenArray, s, e);
	if(expr != K_NULLEXPR) {
		KdumpExpr(kctx, expr);
		KLIB kObject_setObject(kctx, stmt, name, O_classId(expr), expr);
		r = e;
	}
	RESET_GCSTACK();
	RETURNi_(r);
}

static KMETHOD PatternMatch_Type(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenArray, s, e);
	int r = -1;
	kToken *tk = tokenArray->tokenItems[s];
	if(TK_isType(tk)) {
		KLIB kObject_setObject(kctx, stmt, name, O_classId(tk), tk);
		r = s + 1;
	}
	RETURNi_(r);
}

static KMETHOD PatternMatch_Usymbol(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenArray, s, e);
	int r = -1;
	kToken *tk = tokenArray->tokenItems[s];
	if(tk->keyword == TK_SYMBOL && isUpperCaseSymbol(S_text(tk->text))) {
		KLIB kObject_setObject(kctx, stmt, name, O_classId(tk), tk);
		r = s + 1;
	}
	RETURNi_(r);
}

static KMETHOD PatternMatch_Symbol(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenArray, s, e);
	int r = -1;
	kToken *tk = tokenArray->tokenItems[s];
	if(tk->keyword == TK_SYMBOL) {
		KLIB kObject_setObject(kctx, stmt, name, O_classId(tk), tk);
		r = s + 1;
	}
	RETURNi_(r);
}

static KMETHOD PatternMatch_Params(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenArray, s, e);
	int r = -1;
	kToken *tk = tokenArray->tokenItems[s];
	if(tk->keyword == AST_PARENTHESIS) {
		kArray *tokenArray = tk->sub;
		int ss = 0, ee = kArray_size(tokenArray);
		if(0 < ee && tokenArray->tokenItems[0]->keyword == KW_void) ss = 1;  //  f(void) = > f()
		kBlock *bk = new_Block(kctx, Stmt_nameSpace(stmt), stmt, tokenArray, ss, ee, ',');
		KLIB kObject_setObject(kctx, stmt, name, O_classId(bk), bk);
		r = s + 1;
	}
	RETURNi_(r);
}

static KMETHOD PatternMatch_Block(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenArray, s, e);
	kToken *tk = tokenArray->tokenItems[s];
	dumpTokenArray(kctx, 0, tokenArray, s, e);
	if(tk->keyword == TK_CODE) {
		KLIB kObject_setObject(kctx, stmt, name, O_classId(tk), tk);
		RETURNi_(s+1);
	}
//	else if(tk->keyword == AST_BRACE) {
//		kBlock *bk = new_Block(kctx, Stmt_nameSpace(stmt), stmt, tk->sub, 0, kArray_size(tk->sub), ';');
//		KLIB kObject_setObject(kctx, stmt, name, bk);
//		RETURNi_(s+1);
//	}
	else {
		kBlock *bk = new_Block(kctx, Stmt_nameSpace(stmt), stmt, tokenArray, s, e, ';');
		KLIB kObject_setObject(kctx, stmt, name, O_classId(bk), bk);
		RETURNi_(e);
	}
	RETURNi_(-1); // ERROR
}

static KMETHOD PatternMatch_Toks(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenArray, s, e);
	if(s < e) {
		kArray *a = new_(TokenArray, (intptr_t)(e - s));
		while(s < e) {
			KLIB kArray_add(kctx, a, tokenArray->tokenItems[s]);
			s++;
		}
		KLIB kObject_setObject(kctx, stmt, name, O_classId(a), a);
		RETURNi_(e);
	}
	RETURNi_(-1);
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
