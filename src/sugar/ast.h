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

static int selectStmtLine(KonohaContext *kctx, kNameSpace *ns, int *indent, kArray *tokenList, int beginidx, int endidx, int delim, kArray *tlsdst, kToken **tkERRRef);
static void Block_addStmtLine(KonohaContext *kctx, kBlock *bk, kArray *tokenList, int beginidx, int end, kToken *tkERR);
static int makeTree(KonohaContext *kctx, kNameSpace *ns, ksymbol_t tt, kArray *tokenList, int beginidx, int endidx, int closech, kArray *tlsdst, kToken **tkERRRef);

static kBlock *new_Block(KonohaContext *kctx, kNameSpace *ns, kStmt *parent, kArray *tokenList, int beginidx, int endidx, int delim)
{
	kBlockVar *bk = GCSAFE_new(BlockVar, ns);
	if(parent != NULL) {
		KINITv(bk->parentStmtNULL, parent);
	}
	int i = beginidx, indent = 0, atop = kArray_size(tokenList);
	while(i < endidx) {
		kToken *tkERR = NULL;
		DBG_ASSERT(atop == kArray_size(tokenList));
		i = selectStmtLine(kctx, ns, &indent, tokenList, i, endidx, delim, tokenList, &tkERR);
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
	kparam_t p[size];
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
		if(ct->bcid == TY_Func) {
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
	tk->ty = ct->cid;
	return tk;
}

static int appendKeyword(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginidx, int endidx, kArray *dst, kToken **tkERR)
{
	int next = beginidx; // don't add
	kTokenVar *tk = tokenList->tokenVarItems[beginidx];
	if(tk->keyword == TK_SYMBOL) {
		if(!Token_resolved(kctx, ns, tk)) {
			const char *t = S_text(tk->text);
			if(isalpha(t[0])) {
				KonohaClass *ct = KLIB kNameSpace_getCT(kctx, ns, NULL/*FIXME*/, S_text(tk->text), S_size(tk->text), TY_unknown);
				if(ct != NULL) {
					tk->keyword = KW_TypePattern;
					tk->ty = ct->cid;
				}
			}
			else {
				Token_pERR(kctx, tk, "undefined token: %s", kToken_s(tk));
				tkERR[0] = tk;
				return endidx;
			}
		}
	}
	kToken_setUnresolved(tk, false);
	if(TK_isType(tk)) {   // trying to resolve Type[Type, Type]
		KLIB kArray_add(kctx, dst, tk);
		while(next + 1 < endidx) {
			kToken *tkB = tokenList->tokenItems[next + 1];
			int topch = kToken_topch(tkB);
			if(topch != '[') break;
			kArray *abuf = ctxsugar->preparedTokenList;
			size_t atop = kArray_size(abuf);
			next = makeTree(kctx, ns, AST_BRACKET, tokenList,  next+1, endidx, ']', abuf, tkERR);
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
		NameSpace_tokenize(kctx, ns, S_text(tk->text), tk->uline, a);
		tk->keyword = AST_BRACE;
		KSETv(tk->sub, a);
		RESET_GCSTACK();
		return 1;
	}
	return 0;
}

static int makeTree(KonohaContext *kctx, kNameSpace *ns, ksymbol_t astkw, kArray *tokenList, int beginidx, int endidx, int closech, kArray *tlsdst, kToken **tkERRRef)
{
	int i, probablyCloseBefore = endidx - 1;
	kToken *tk = tokenList->tokenItems[beginidx];
	kTokenVar *tkP = new_(TokenVar, 0);
	KLIB kArray_add(kctx, tlsdst, tkP);
	tkP->keyword = astkw;
	tkP->uline = tk->uline;
	KSETv(tkP->sub, new_(TokenArray, 0));
	for(i = beginidx + 1; i < endidx; i++) {
		tk = tokenList->tokenItems[i];
		if(tk->keyword == TK_ERR) break;  // ERR
		if(!kToken_needsKeywordResolved(tk)) {
			KLIB kArray_add(kctx, tkP->sub, tk);
			continue;
		}
		int topch = kToken_topch(tk);
		DBG_ASSERT(topch != '{');
		if(topch == '(') {
			i = makeTree(kctx, ns, AST_PARENTHESIS, tokenList, i, endidx, ')', tkP->sub, tkERRRef);
			continue;
		}
		if(topch == '[') {
			i = makeTree(kctx, ns, AST_BRACKET, tokenList, i, endidx, ']', tkP->sub, tkERRRef);
			continue;
		}
		if(topch == closech) {
			return i;
		}
		if(closech != '}') {
			if(tk->keyword == TK_INDENT) continue; // remove INDENT from tokens;
			if(tk->keyword == TK_CODE) probablyCloseBefore = i;
		}
		i = appendKeyword(kctx, ns, tokenList, i, endidx, tkP->sub, tkERRRef);
	}
	if(tk->keyword != TK_ERR) {
		Token_pERR(kctx, tkP, "'%c' is expected (probably before %s)", closech, kToken_s(tokenList->tokenItems[probablyCloseBefore]));
	}
	else {
		tkP->keyword = TK_ERR;
		KSETv(tkP->text, tk->text);
	}
	tkERRRef[0] = tkP;
	return endidx;
}

static int selectStmtLine(KonohaContext *kctx, kNameSpace *ns, int *indent, kArray *tokenList, int beginidx, int endidx, int delim, kArray *tlsdst, kToken **tkERRRef)
{
	int i = beginidx;
	DBG_ASSERT(endidx <= kArray_size(tokenList));
	dumpTokenArray(kctx, 0, tokenList, beginidx, endidx);
	for(; i < endidx - 1; i++) {
		kToken *tk = tokenList->tokenItems[i];
		kTokenVar *tk1 = tokenList->tokenVarItems[i+1];
		int topch = kToken_topch(tk);
		if(topch == '@' && (tk1->keyword == TK_SYMBOL)) {
			tk1->keyword = ksymbolA(S_text(tk1->text), S_size(tk1->text), SYM_NEWID) | MN_Annotation;
			KLIB kArray_add(kctx, tlsdst, tk1); i++;
			tk1 = tokenList->tokenVarItems[i+1];
			topch = kToken_topch(tk1);
			if(i + 1 < endidx && topch == '(') {
				i = makeTree(kctx, ns, AST_PARENTHESIS, tokenList, i+1, endidx, ')', tlsdst, tkERRRef);
			}
			continue;
		}
		if(MN_isAnnotation(tk->keyword)) {  // already parsed
			KLIB kArray_add(kctx, tlsdst, tk);
			if(tk1->keyword == AST_PARENTHESIS) {
				KLIB kArray_add(kctx, tlsdst, tk1);
				i++;
			}
			continue;
		}
		if(tk->keyword != TK_INDENT) break;
		if(*indent == 0) *indent = tk->indent;
	}
	for(; i < endidx ; i++) {
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
				i = makeTree(kctx, ns, AST_PARENTHESIS, tokenList,  i, endidx, ')', tlsdst, tkERRRef);
				continue;
			}
			else if(topch == '[') {
				i = makeTree(kctx, ns, AST_BRACKET, tokenList, i, endidx, ']', tlsdst, tkERRRef);
				continue;
			}
			i = appendKeyword(kctx, ns, tokenList, i, endidx, tlsdst, tkERRRef);
		}
		else {
			KLIB kArray_add(kctx, tlsdst, tk);
			continue;
		}
	}
	return i;
}

static kExpr* Stmt_newExpr2(KonohaContext *kctx, kStmt *stmt, kArray *tls, int s, int e);

static int Stmt_addAnnotation(KonohaContext *kctx, kStmt *stmt, kArray *tokenList, int beginidx, int endidx)
{
	int i;
	for(i = beginidx; i < endidx; i++) {
		kToken *tk = tokenList->tokenItems[i];
		if(!MN_isAnnotation(tk->keyword)) break;
		if(i+1 < endidx) {
			kToken *tk1 = tokenList->tokenItems[i+1];
			kObject *value = UPCAST(K_TRUE);
			if(tk1->keyword == AST_PARENTHESIS) {
				value = (kObject*)Stmt_newExpr2(kctx, stmt, tk1->sub, 0, kArray_size(tk1->sub));
				i++;
			}
			if(value != NULL) {
				KLIB kObject_setObject(kctx, stmt, tk->keyword, O_cid(value), value);
			}
		}
	}
	return i;
}

static int PatternMatchFunc(KonohaContext *kctx, kFunc *fo, kStmt *stmt, ksymbol_t name, kArray *tokenList, int beginidx, int endidx)
{
	INIT_GCSTACK();
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 5);
	KSETv(lsfp[K_CALLDELTA+0].o, fo->self);
	KSETv(lsfp[K_CALLDELTA+1].o, (kObject*)stmt);
	lsfp[K_CALLDELTA+2].ivalue = name;
	KSETv(lsfp[K_CALLDELTA+3].toArray, tokenList);
	lsfp[K_CALLDELTA+4].ivalue = beginidx;
	lsfp[K_CALLDELTA+5].ivalue = endidx;
	KCALL(lsfp, 0, fo->mtd, 5, KLIB Knull(kctx, CT_Int));
	END_LOCAL();
	RESET_GCSTACK();
	return (int)lsfp[0].ivalue;
}

static int PatternMatch(KonohaContext *kctx, SugarSyntax *syn, kStmt *stmt, ksymbol_t name, kArray *tokenList, int beginidx, int endidx)
{
	kFunc *fo = syn->PatternMatch;
	int next;
	if(IS_Array(fo)) {
		int i;
		kArray *a = (kArray*)fo;
		for(i = kArray_size(a) - 1; i > 0; i--) {
			next = PatternMatchFunc(kctx, fo, stmt, name, tokenList, beginidx, endidx);
			if(kStmt_isERR(stmt)) return -1;
			if(next > beginidx) return next;
		}
		fo = a->funcItems[0];
	}
	DBG_ASSERT(IS_Func(fo));
	next = PatternMatchFunc(kctx, fo, stmt, name, tokenList, beginidx, endidx);
	if(kStmt_isERR(stmt)) return -1;
	return (next > beginidx) ? next : -1;
}

static int lookAheadKeyword(kArray *tokenList, int beginidx, int endidx, kToken *rule)
{
	int i;
	for(i = beginidx; i < endidx; i++) {
		kToken *tk = tokenList->tokenItems[i];
		if(rule->keyword == tk->keyword) return i;
	}
	return -1;
}

static int kStmt_printExpectedRule(KonohaContext *kctx, kStmt *stmt, kToken *tk, kToken *rule, int beginidx, int canRollBack)
{
	if(canRollBack) return beginidx;
	kToken_p(stmt, tk, ErrTag, "%s%s expects %s%s", T_statement(stmt->syn->keyword), KW_t(rule->keyword));
	return -1;
}

static int matchSyntaxRule(KonohaContext *kctx, kStmt *stmt, kArray *rules, kArray *tokenList, int beginidx, int endidx, int canRollBack)
{
	int ruleidx, tokenidx, rulesize = kArray_size(rules);
	dumpTokenArray(kctx, 0, tokenList, beginidx, endidx);
	dumpTokenArray(kctx, 0, rules, 0, rulesize);
	tokenidx = beginidx;
	for(ruleidx = 0; ruleidx < rulesize && tokenidx < endidx; ruleidx++) {
		DBG_P("matching token=%d<%d, rule=%d<%d", tokenidx, endidx, ruleidx, rulesize);
		kToken *rule = rules->tokenItems[ruleidx];
		kToken *tk = tokenList->tokenItems[tokenidx];
		if(KW_isPATTERN(rule->keyword)) {
			SugarSyntax *syn = SYN_(kStmt_nameSpace(stmt), rule->keyword);
			if(syn == NULL || syn->PatternMatch == kmodsugar->UndefinedParseExpr/*NULL*/) {
				kToken_p(stmt, tk, ErrTag, "unknown syntax pattern: %s%s", KW_t(rule->keyword));
				return -1;
			}
			int c = endidx;
			kToken *rule1 = rules->tokenItems[ruleidx+1];
			if(ruleidx + 1 < rulesize && (!KW_isPATTERN(rule1->keyword) && rule1->keyword != AST_OPTIONAL)) {
				DBG_P("NEXT rule=%s%s, rule_size=%d,%d", KW_t(rule1->keyword), ruleidx+1, rulesize);
				c = lookAheadKeyword(tokenList, tokenidx+1, endidx, rule1);
				if(c == -1) {
					DBG_P("@");
					return kStmt_printExpectedRule(kctx, stmt, tk, rule, beginidx, canRollBack);
				}
				ruleidx++;
			}
			DBG_P("syntax rule=%s%s", KW_t(syn->keyword));
			int next = PatternMatch(kctx, syn, stmt, rule->patternKey, tokenList, tokenidx, c);
			DBG_P("matched '%s%s' patternKey='%s%s', next=%d=>%d", KW_t(rule->keyword), KW_t(rule->patternKey), tokenidx, next);
			if(next == -1) {
				DBG_P("@");
				return kStmt_printExpectedRule(kctx, stmt, tk, rule, beginidx, canRollBack);
			}
			tokenidx = (c == endidx) ? next : c + 1;
		}
		else if(rule->keyword == AST_OPTIONAL) {
			int next = matchSyntaxRule(kctx, stmt, rule->sub, tokenList, tokenidx, endidx, 1);
			if(next == -1) return -1;
			tokenidx = next;
		}
		else {
			if(rule->keyword != tk->keyword) {
				//DBG_P("matching rule=%d,%s%s token=%d,%s%s", ri,  KW_t(rule->keyword), ti-s, KW_t(tk->keyword));
				DBG_P("@");
				return kStmt_printExpectedRule(kctx, stmt, tk, rule, beginidx, canRollBack);
			}
			if(rule->keyword == AST_PARENTHESIS || rule->keyword == AST_BRACKET) {
				int next = matchSyntaxRule(kctx, stmt, rule->sub, tk->sub, 0, kArray_size(tk->sub), 0);
				if(next == -1) return -1;
			}
			tokenidx++;
		}
		DBG_P("matching next token=%d<%d, rule=%d<%d", tokenidx, endidx, ruleidx+1, rulesize);
	}
	if(!canRollBack) {
		for(; ruleidx < rulesize; ruleidx++) {
			kToken *rule = rules->tokenItems[ruleidx];
			if(rule->keyword != AST_OPTIONAL) {
				kStmt_p(stmt, ErrTag, "%s%s needs syntax pattern: %s%s", T_statement(stmt->syn->keyword), KW_t(rule->keyword));
				return -1;
			}
		}
		if(tokenidx < endidx) {
			kStmt_p(stmt, ErrTag, "%s%s: unexpected token %s", T_statement(stmt->syn->keyword), kToken_s(tokenList->tokenItems[tokenidx]));
			return -1;
		}
	}
	return tokenidx;
}

static inline kToken* TokenArray_nextToken(KonohaContext *kctx, kArray *tokenList, int beginidx, int endidx)
{
	return (beginidx < endidx) ? tokenList->tokenItems[beginidx] : K_NULLTOKEN;
}

static SugarSyntax* NameSpace_getSyntaxRule(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginidx, int endidx)
{
	kToken *tk = tokenList->tokenItems[beginidx];
	if(TK_isType(tk)) {
		tk = TokenArray_nextToken(kctx, tokenList, beginidx+1, endidx);
		if(tk->keyword == TK_SYMBOL || tk->keyword == TK_TYPE) {
			tk = TokenArray_nextToken(kctx, tokenList, beginidx+2, endidx);
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
		kToken *tk1 = TokenArray_nextToken(kctx, tokenList, beginidx+1, endidx);
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
		for(i = beginidx + 1; i < endidx; i++) {
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

static kbool_t Stmt_parseSyntaxRule(KonohaContext *kctx, kStmt *stmt, kArray *tokenList, int beginidx, int endidx)
{
	kbool_t ret = false;
	SugarSyntax *syn = NameSpace_getSyntaxRule(kctx, kStmt_nameSpace(stmt), tokenList, beginidx, endidx);
	DBG_ASSERT(syn != NULL);
	if(syn->syntaxRuleNULL != NULL) {
		((kStmtVar*)stmt)->syn = syn;
		ret = (matchSyntaxRule(kctx, stmt, syn->syntaxRuleNULL, tokenList, beginidx, endidx, 0) != -1);
	}
	else {
		kStmt_p(stmt, ErrTag, "undefined syntax rule for '%s%s'", KW_t(syn->keyword));
	}
	return ret;
}

static void Block_addStmtLine(KonohaContext *kctx, kBlock *bk, kArray *tokenList, int beginidx, int endidx, kToken *tkERR)
{
	kStmtVar *stmt = new_(StmtVar, tokenList->tokenItems[beginidx]->uline);
	KLIB kArray_add(kctx, bk->stmtList, stmt);
	KINITv(stmt->parentBlockNULL, bk);
	if(tkERR != NULL) {
		kStmt_toERR(stmt, tkERR->text);
	}
	else {
		int c = Stmt_addAnnotation(kctx, stmt, tokenList, beginidx, endidx);
		Stmt_parseSyntaxRule(kctx, stmt, tokenList, c, endidx);
	}
	DBG_ASSERT(stmt->syn != NULL);
}

/* ------------------------------------------------------------------------ */

static KMETHOD UndefinedParseExpr(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tls, s, c, e);
	kStmt_p(stmt, ErrTag, "undefined expression parser for '%s'", kToken_s(tls->tokenItems[c]));
}

static kExpr *ParseExprFunc(KonohaContext *kctx, SugarSyntax *syn, kFunc *fo, kStmt *stmt, kArray *tls, int s, int c, int e)
{
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 6);
	KSETv(lsfp[K_CALLDELTA+0].o, fo->self);
	lsfp[K_CALLDELTA+0].ndata = (uintptr_t)syn;
	KSETv(lsfp[K_CALLDELTA+1].o, (kObject*)stmt);
	KSETv(lsfp[K_CALLDELTA+2].toArray, tls);
	lsfp[K_CALLDELTA+3].ivalue = s;
	lsfp[K_CALLDELTA+4].ivalue = c;
	lsfp[K_CALLDELTA+5].ivalue = e;
	KCALL(lsfp, 0, fo->mtd, 5, K_NULLEXPR);
	END_LOCAL();
	DBG_ASSERT(IS_Expr(lsfp[0].toObject));
	return lsfp[0].expr;
}

static kExpr *ParseExpr(KonohaContext *kctx, SugarSyntax *syn, kStmt *stmt, kArray *tls, int s, int c, int e)
{
	kFunc *fo = (syn == NULL || syn->ParseExpr == NULL) ? kmodsugar->UndefinedParseExpr : syn->ParseExpr;
	kExpr *texpr;
	if(IS_Array(fo)) {
		int i;
		kArray *a = (kArray*)fo;
		for(i = kArray_size(a) - 1; i > 0; i--) {
			texpr = ParseExprFunc(kctx, syn, fo, stmt, tls, s, c, e);
			if(kStmt_isERR(stmt)) return K_NULLEXPR;
			if(texpr != K_NULLEXPR) return texpr;
		}
		fo = a->funcItems[0];
	}
	DBG_ASSERT(IS_Func(fo));
	texpr = ParseExprFunc(kctx, syn, fo, stmt, tls, s, c, e);
	if(texpr == K_NULLEXPR && !kStmt_isERR(stmt)) {
		kStmt_p(stmt, ErrTag, "syntax error: operator %s", kToken_s(tls->tokenItems[c]));
	}
	return texpr;
}

/* ------------------------------------------------------------------------ */

static kbool_t Stmt_isUnaryOp(KonohaContext *kctx, kStmt *stmt, kToken *tk)
{
	SugarSyntax *syn = SYN_(kStmt_nameSpace(stmt), tk->keyword);
	return (syn->op1 != SYM_NONAME);
}

static int Stmt_skipUnaryOp(KonohaContext *kctx, kStmt *stmt, kArray *tls, int s, int e)
{
	int i;
	for(i = s; i < e; i++) {
		kToken *tk = tls->tokenItems[i];
		if(!Stmt_isUnaryOp(kctx, stmt, tk)) break;
	}
	return i;
}

static int Stmt_findBinaryOp(KonohaContext *kctx, kStmt *stmt, kArray *tls, int s, int e, SugarSyntax **synRef)
{
	int idx = -1, i, prif = 0;
	for(i = Stmt_skipUnaryOp(kctx, stmt, tls, s, e) + 1; i < e; i++) {
		kToken *tk = tls->tokenItems[i];
		SugarSyntax *syn = SYN_(kStmt_nameSpace(stmt), tk->keyword);
		if(syn->priority > 0) {
			if(prif < syn->priority || (prif == syn->priority && !(FLAG_is(syn->flag, SYNFLAG_ExprLeftJoinOp2)) )) {
				prif = syn->priority;
				idx = i;
				*synRef = syn;
			}
			if(!FLAG_is(syn->flag, SYNFLAG_ExprPostfixOp2)) {  /* check if real binary operator to parse f() + 1 */
				i = Stmt_skipUnaryOp(kctx, stmt, tls, i+1, e) - 1;
			}
		}
	}
	return idx;
}

static kExpr *Stmt_addExprParams(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kArray *tls, int s, int e, int allowEmpty)
{
	int i, start = s;
	for(i = s; i < e; i++) {
		kToken *tk = tls->tokenItems[i];
		if(tk->keyword == KW_COMMA) {
			expr = Expr_add(kctx, expr, Stmt_newExpr2(kctx, stmt, tls, start, i));
			start = i + 1;
		}
	}
	if(allowEmpty == 0 || start < i) {
		expr = Expr_add(kctx, expr, Stmt_newExpr2(kctx, stmt, tls, start, i));
	}
	KLIB kArray_clear(kctx, tls, s);
	return expr;
}

static kExpr* Stmt_newExpr2(KonohaContext *kctx, kStmt *stmt, kArray *tls, int s, int e)
{
	if(!kStmt_isERR(stmt)) {
		if(s < e) {
			SugarSyntax *syn = NULL;
			int idx = Stmt_findBinaryOp(kctx, stmt, tls, s, e, &syn);
			if(idx != -1) {
				//DBG_P("** Found BinaryOp: s=%d, idx=%d, e=%d, '%s'**", s, idx, e, kToken_s(tls->tokenItems[idx]));
				return ParseExpr(kctx, syn, stmt, tls, s, idx, e);
			}
			int c = s;
			syn = SYN_(kStmt_nameSpace(stmt), (tls->tokenItems[c])->keyword);
			return ParseExpr(kctx, syn, stmt, tls, c, c, e);
		}
		if (0 < s - 1) {
			kStmt_p(stmt, ErrTag, "expected expression after %s", kToken_s(tls->tokenItems[s-1]));
		}
		else if(e < kArray_size(tls)) {
			kStmt_p(stmt, ErrTag, "expected expression before %s", kToken_s(tls->tokenItems[e]));
		}
		else {
			kStmt_p(stmt, ErrTag, "expected expression");
		}
	}
	return K_NULLEXPR;
}

#define kExpr_rightJoin(EXPR, STMT, TLS, S, C, E)    Expr_rightJoin(kctx, EXPR, STMT, TLS, S, C, E)

static kExpr *Expr_rightJoin(KonohaContext *kctx, kExpr *expr, kStmt *stmt, kArray *tls, int s, int c, int e)
{
	if(c < e && expr != K_NULLEXPR && !kStmt_isERR(stmt)) {
		WarnTagIgnored(kctx, tls, c, e);
	}
	return expr;
}

static KMETHOD ParseExpr_Term(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tls, s, c, e);
	DBG_ASSERT(s == c);
	kToken *tk = tls->tokenItems[c];
	kExprVar *expr = new_(ExprVar, SYN_(kStmt_nameSpace(stmt), tk->keyword));
	KSETv(expr->termToken, tk);
	Expr_setTerm(expr, 1);
	RETURN_(kExpr_rightJoin(expr, stmt, tls, s+1, c+1, e));
}

static KMETHOD ParseExpr_Op(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tls, s, c, e);
	kTokenVar *tk = tls->tokenVarItems[c];
	kExpr *expr, *rexpr = Stmt_newExpr2(kctx, stmt, tls, c+1, e);
	kmethodn_t mn = (s == c) ? syn->op1 : syn->op2;
	if(mn != SYM_NONAME && syn->ExprTyCheck == kmodsugar->UndefinedExprTyCheck) {
		tk->keyword = mn;
		syn = SYN_(kStmt_nameSpace(stmt), KW_ExprMethodCall);  // switch type checker
	}
	if(s == c) { // unary operator
		expr = new_ConsExpr(kctx, syn, 2, tk, rexpr);
	}
	else {   // binary operator
		kExpr *lexpr = Stmt_newExpr2(kctx, stmt, tls, s, c);
		expr = new_ConsExpr(kctx, syn, 3, tk, lexpr, rexpr);
	}
	RETURN_(expr);
}

static inline kbool_t isFieldName(kArray *tls, int c, int e)
{
	if(c + 1 < e) {
		kToken *tk = tls->tokenItems[c+1];
		return (tk->keyword == TK_SYMBOL);
	}
	return false;
}
static KMETHOD ParseExpr_DOT(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tls, s, c, e);
	if(s < c && isFieldName(tls, c, e)) {
		kExpr *expr = Stmt_newExpr2(kctx, stmt, tls, s, c);
		expr = new_ConsExpr(kctx, syn, 2, tls->tokenItems[c+1], expr);
		RETURN_(kExpr_rightJoin(expr, stmt, tls, c+2, c+2, e));
	}
}

static KMETHOD ParseExpr_Parenthesis(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tls, s, c, e);
	kToken *tk = tls->tokenItems[c];
	if(s == c) {
		kExpr *expr = Stmt_newExpr2(kctx, stmt, tk->sub, 0, kArray_size(tk->sub));
		RETURN_(kExpr_rightJoin(expr, stmt, tls, s+1, c+1, e));
	}
	else {
		kExpr *lexpr = Stmt_newExpr2(kctx, stmt, tls, s, c);
		if(lexpr == K_NULLEXPR) {
			RETURN_(lexpr);
		}
		if(lexpr->syn->keyword == KW_DOT) {
			((kExprVar*)lexpr)->syn = SYN_(kStmt_nameSpace(stmt), KW_ExprMethodCall); // CALL
		}
		else if(lexpr->syn->keyword != KW_ExprMethodCall) {
			syn = SYN_(kStmt_nameSpace(stmt), KW_ParenthesisPattern);    // (f null ())
			lexpr  = new_ConsExpr(kctx, syn, 2, lexpr, K_NULL);
		}
		lexpr = Stmt_addExprParams(kctx, stmt, lexpr, tk->sub, 0, kArray_size(tk->sub), 1/*allowEmpty*/);
		RETURN_(kExpr_rightJoin(lexpr, stmt, tls, s+1, c+1, e));
	}
}

static KMETHOD ParseExpr_COMMA(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tls, s, c, e);
	kExpr *expr = new_ConsExpr(kctx, syn, 1, tls->tokenItems[c]);
	expr = Stmt_addExprParams(kctx, stmt, expr, tls, s, e, 0/*allowEmpty*/);
	RETURN_(expr);
}

static KMETHOD ParseExpr_DOLLAR(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tls, s, c, e);
	if(s == c && c + 1 < e) {
		kToken *tk = tls->tokenItems[c+1];
		if(tk->keyword == TK_CODE) {
			Token_toBRACE(kctx, (kTokenVar*)tk, kStmt_nameSpace(stmt));
		}
		if(tk->keyword == AST_BRACE) {
			kExprVar *expr = GCSAFE_new(ExprVar, SYN_(kStmt_nameSpace(stmt), KW_BlockPattern));
			KSETv(expr->block, new_Block(kctx, kStmt_nameSpace(stmt), stmt, tk->sub, 0, kArray_size(tk->sub), ';'));
			RETURN_(expr);
		}
	}
}

/* ------------------------------------------------------------------------ */

static KMETHOD PatternMatch_Expr(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tls, s, e);
	INIT_GCSTACK();
	int r = -1;
	dumpTokenArray(kctx, 0, tls, s, e);
	kExpr *expr = Stmt_newExpr2(kctx, stmt, tls, s, e);
	if(expr != K_NULLEXPR) {
		dumpExpr(kctx, 0, 0, expr);
		KLIB kObject_setObject(kctx, stmt, name, O_cid(expr), expr);
		r = e;
	}
	RESET_GCSTACK();
	RETURNi_(r);
}

static KMETHOD PatternMatch_Type(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tls, s, e);
	int r = -1;
	kToken *tk = tls->tokenItems[s];
	if(TK_isType(tk)) {
		KLIB kObject_setObject(kctx, stmt, name, O_cid(tk), tk);
		r = s + 1;
	}
	RETURNi_(r);
}

static KMETHOD PatternMatch_Usymbol(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tls, s, e);
	int r = -1;
	kToken *tk = tls->tokenItems[s];
	if(tk->keyword == TK_SYMBOL && isUpperCaseSymbol(S_text(tk->text))) {
		KLIB kObject_setObject(kctx, stmt, name, O_cid(tk), tk);
		r = s + 1;
	}
	RETURNi_(r);
}

static KMETHOD PatternMatch_Symbol(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tls, s, e);
	int r = -1;
	kToken *tk = tls->tokenItems[s];
	if(tk->keyword == TK_SYMBOL) {
		KLIB kObject_setObject(kctx, stmt, name, O_cid(tk), tk);
		r = s + 1;
	}
	RETURNi_(r);
}

static KMETHOD PatternMatch_Params(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tls, s, e);
	int r = -1;
	kToken *tk = tls->tokenItems[s];
	if(tk->keyword == AST_PARENTHESIS) {
		kArray *tls = tk->sub;
		int ss = 0, ee = kArray_size(tls);
		if(0 < ee && tls->tokenItems[0]->keyword == KW_void) ss = 1;  //  f(void) = > f()
		kBlock *bk = new_Block(kctx, kStmt_nameSpace(stmt), stmt, tls, ss, ee, ',');
		KLIB kObject_setObject(kctx, stmt, name, O_cid(bk), bk);
		r = s + 1;
	}
	RETURNi_(r);
}

static KMETHOD PatternMatch_Block(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tls, s, e);
	kToken *tk = tls->tokenItems[s];
	dumpTokenArray(kctx, 0, tls, s, e);
	if(tk->keyword == TK_CODE) {
		KLIB kObject_setObject(kctx, stmt, name, O_cid(tk), tk);
		RETURNi_(s+1);
	}
//	else if(tk->keyword == AST_BRACE) {
//		kBlock *bk = new_Block(kctx, kStmt_nameSpace(stmt), stmt, tk->sub, 0, kArray_size(tk->sub), ';');
//		KLIB kObject_setObject(kctx, stmt, name, bk);
//		RETURNi_(s+1);
//	}
	else {
		kBlock *bk = new_Block(kctx, kStmt_nameSpace(stmt), stmt, tls, s, e, ';');
		KLIB kObject_setObject(kctx, stmt, name, O_cid(bk), bk);
		RETURNi_(e);
	}
	RETURNi_(-1); // ERROR
}

static KMETHOD PatternMatch_Toks(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tls, s, e);
	if(s < e) {
		kArray *a = new_(TokenArray, (intptr_t)(e - s));
		while(s < e) {
			KLIB kArray_add(kctx, a, tls->tokenItems[s]);
			s++;
		}
		KLIB kObject_setObject(kctx, stmt, name, O_cid(a), a);
		RETURNi_(e);
	}
	RETURNi_(-1);
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
