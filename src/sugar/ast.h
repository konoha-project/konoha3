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

static int selectStmtLine(KonohaContext *kctx, kNameSpace *ns, int *indent, kArray *tls, int s, int e, int delim, kArray *tlsdst, kToken **tkERRRef);
static void Block_addStmtLine(KonohaContext *kctx, kBlock *bk, kArray *tls, int s, int e, kToken *tkERR);
static int makeTree(KonohaContext *kctx, kNameSpace *ns, ksymbol_t tt, kArray *tls, int s, int e, int closech, kArray *tlsdst, kToken **tkERRRef);

static kBlock *new_Block(KonohaContext *kctx, kNameSpace *ns, kStmt *parent, kArray *tls, int s, int e, int delim)
{
	kBlockVar *bk = new_Var(Block, ns);
	PUSH_GCSTACK(bk);
	if(parent != NULL) {
		KINITv(bk->parentStmtNULL, parent);
	}
	int i = s, indent = 0, atop = kArray_size(tls);
	while(i < e) {
		kToken *tkERR = NULL;
		DBG_ASSERT(atop == kArray_size(tls));
		i = selectStmtLine(kctx, ns, &indent, tls, i, e, delim, tls, &tkERR);
		int asize = kArray_size(tls);
		if(asize > atop) {
			Block_addStmtLine(kctx, bk, tls, atop, asize, tkERR);
			kArray_clear(tls, atop);
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
		kToken *tkT = tkP->sub->toks[i];
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
		if(ct->bcid == CLASS_Func) {
			ct = kClassTable_Generics(ct, p[0].ty, psize-1, p+1);
		}
		else if(ct->p0 == TY_void) {
			Token_pERR(kctx, tk, "not generic type: %s", TY_t(TK_type(tk)));
			return tk;
		}
		else {
			ct = kClassTable_Generics(ct, TY_void, psize, p);
		}
	}
	else {
		ct = CT_p0(kctx, CT_Array, TK_type(tk));
	}
	tk->ty = ct->cid;
	return tk;
}

static int appendKeyword(KonohaContext *kctx, kNameSpace *ns, kArray *tls, int s, int e, kArray *dst, kToken **tkERR)
{
	int next = s; // don't add
	kTokenVar *tk = tls->Wtoks[s];
	if(tk->keyword == TK_SYMBOL) {
		if(!Token_resolved(kctx, ns, tk)) {
			const char *t = S_text(tk->text);
			if(isalpha(t[0])) {
				KonohaClass *ct = kNameSpace_getCT(ns, NULL/*FIXME*/, S_text(tk->text), S_size(tk->text), TY_unknown);
				if(ct != NULL) {
					tk->keyword = KW_TypePattern;
					tk->ty = ct->cid;
				}
			}
			else {
				Token_pERR(kctx, tk, "undefined token: %s", kToken_s(tk));
				tkERR[0] = tk;
				return e;
			}
		}
	}
	kToken_setUnresolved(tk, false);
	if(TK_isType(tk)) {   // trying to resolve Type[Type, Type]
		kArray_add(dst, tk);
		while(next + 1 < e) {
			kToken *tkB = tls->toks[next + 1];
			int topch = kToken_topch(tkB);
			if(topch != '[') break;
			kArray *abuf = ctxsugar->preparedTokenList;
			size_t atop = kArray_size(abuf);
			next = makeTree(kctx, ns, AST_BRACKET, tls,  next+1, e, ']', abuf, tkERR);
			if(!(kArray_size(abuf) > atop)) return next;
			tkB = abuf->toks[atop];
			if(tkB->keyword == AST_BRACKET) {
				tk = TokenType_resolveGenerics(kctx, ns, tk, tkB);
				if(tk == NULL) {
					if(abuf != dst) {
						kArray_add(dst, tkB);
						kArray_clear(abuf, atop);
					}
					return next;
				}
			}
			kArray_clear(abuf, atop);
		}
	}
	else {
		kArray_add(dst, tk);
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

static int makeTree(KonohaContext *kctx, kNameSpace *ks, ksymbol_t astkw, kArray *tls, int s, int e, int closech, kArray *tlsdst, kToken **tkERRRef)
{
	int i, probablyCloseBefore = e - 1;
	kToken *tk = tls->toks[s];
	kTokenVar *tkP = new_Var(Token, 0);
	kArray_add(tlsdst, tkP);
	tkP->keyword = astkw;
	tkP->uline = tk->uline;
	KSETv(tkP->sub, new_(TokenArray, 0));
	for(i = s + 1; i < e; i++) {
		tk = tls->toks[i];
		if(tk->keyword == TK_ERR) break;  // ERR
		if(!kToken_needsKeywordResolved(tk)) {
			kArray_add(tkP->sub, tk);
			continue;
		}
		int topch = kToken_topch(tk);
		DBG_ASSERT(topch != '{');
		if(topch == '(') {
			i = makeTree(kctx, ks, AST_PARENTHESIS, tls, i, e, ')', tkP->sub, tkERRRef);
			continue;
		}
		if(topch == '[') {
			i = makeTree(kctx, ks, AST_BRACKET, tls, i, e, ']', tkP->sub, tkERRRef);
			continue;
		}
		if(topch == closech) {
			return i;
		}
		if(closech != '}') {
			if(tk->keyword == TK_INDENT) continue; // remove INDENT from tokens;
			if(tk->keyword == TK_CODE) probablyCloseBefore = i;
		}
		i = appendKeyword(kctx, ks, tls, i, e, tkP->sub, tkERRRef);
	}
	if(tk->keyword != TK_ERR) {
		Token_pERR(kctx, tkP, "'%c' is expected (probably before %s)", closech, kToken_s(tls->toks[probablyCloseBefore]));
	}
	else {
		tkP->keyword = TK_ERR;
		KSETv(tkP->text, tk->text);
	}
	tkERRRef[0] = tkP;
	return e;
}

static int selectStmtLine(KonohaContext *kctx, kNameSpace *ns, int *indent, kArray *tls, int s, int e, int delim, kArray *tlsdst, kToken **tkERRRef)
{
	int i = s;
	DBG_ASSERT(e <= kArray_size(tls));
	dumpTokenArray(kctx, 0, tls, s, e);
	for(; i < e - 1; i++) {
		kToken *tk = tls->toks[i];
		kTokenVar *tk1 = tls->Wtoks[i+1];
		int topch = kToken_topch(tk);
		if(topch == '@' && (tk1->keyword == TK_SYMBOL)) {
			tk1->keyword = ksymbolA(S_text(tk1->text), S_size(tk1->text), SYM_NEWID) | MN_Annotation;
			kArray_add(tlsdst, tk1); i++;
			tk1 = tls->Wtoks[i+1];
			topch = kToken_topch(tk1);
			if(i + 1 < e && topch == '(') {
				i = makeTree(kctx, ns, AST_PARENTHESIS, tls, i+1, e, ')', tlsdst, tkERRRef);
			}
			continue;
		}
		if(MN_isAnnotation(tk->keyword)) {  // already parsed
			kArray_add(tlsdst, tk);
			if(tk1->keyword == AST_PARENTHESIS) {
				kArray_add(tlsdst, tk1);
				i++;
			}
			continue;
		}
		if(tk->keyword != TK_INDENT) break;
		if(*indent == 0) *indent = tk->indent;
	}
	for(; i < e ; i++) {
		kToken *tk = tls->toks[i];
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
				i = makeTree(kctx, ns, AST_PARENTHESIS, tls,  i, e, ')', tlsdst, tkERRRef);
				continue;
			}
			else if(topch == '[') {
				i = makeTree(kctx, ns, AST_BRACKET, tls, i, e, ']', tlsdst, tkERRRef);
				continue;
			}
			i = appendKeyword(kctx, ns, tls, i, e, tlsdst, tkERRRef);
		}
		else {
			kArray_add(tlsdst, tk);
			continue;
		}
	}
	return i;
}

static kExpr* Stmt_newExpr2(KonohaContext *kctx, kStmt *stmt, kArray *tls, int s, int e);

static int Stmt_addAnnotation(KonohaContext *kctx, kStmt *stmt, kArray *tls, int s, int e)
{
	int i;
	for(i = s; i < e; i++) {
		kToken *tk = tls->toks[i];
		if(!MN_isAnnotation(tk->keyword)) break;
		if(i+1 < e) {
			kToken *tk1 = tls->toks[i+1];
			kObject *value = UPCAST(K_TRUE);
			if(tk1->keyword == AST_PARENTHESIS) {
				value = (kObject*)Stmt_newExpr2(kctx, stmt, tk1->sub, 0, kArray_size(tk1->sub));
				i++;
			}
			if(value != NULL) {
				kObject_setObject(stmt, tk->keyword, value);
			}
		}
	}
	return i;
}

static int PatternMatchFunc(KonohaContext *kctx, kFunc *fo, kStmt *stmt, ksymbol_t name, kArray *tls, int s, int e)
{
	INIT_GCSTACK();
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 5);
	KSETv(lsfp[K_CALLDELTA+0].o, fo->self);
	KSETv(lsfp[K_CALLDELTA+1].o, (kObject*)stmt);
	lsfp[K_CALLDELTA+2].ivalue = name;
	KSETv(lsfp[K_CALLDELTA+3].a, tls);
	lsfp[K_CALLDELTA+4].ivalue = s;
	lsfp[K_CALLDELTA+5].ivalue = e;
	KCALL(lsfp, 0, fo->mtd, 5, knull(CT_Int));
	END_LOCAL();
	RESET_GCSTACK();
	return (int)lsfp[0].ivalue;
}

static int PatternMatch(KonohaContext *kctx, SugarSyntax *syn, kStmt *stmt, ksymbol_t name, kArray *tls, int s, int e)
{
	kFunc *fo = syn->PatternMatch;
	int next;
	if(IS_Array(fo)) {
		int i;
		kArray *a = (kArray*)fo;
		for(i = kArray_size(a) - 1; i > 0; i--) {
			next = PatternMatchFunc(kctx, fo, stmt, name, tls, s, e);
			if(kStmt_isERR(stmt)) return -1;
			if(next > s) return next;
		}
		fo = a->funcs[0];
	}
	DBG_ASSERT(IS_Func(fo));
	next = PatternMatchFunc(kctx, fo, stmt, name, tls, s, e);
	if(kStmt_isERR(stmt)) return -1;
	return (next > s) ? next : -1;
}

static int lookAheadKeyword(kArray *tls, int s, int e, kToken *rule)
{
	int i;
	for(i = s; i < e; i++) {
		kToken *tk = tls->toks[i];
		if(rule->keyword == tk->keyword) return i;
	}
	return -1;
}

static int kStmt_printExpectedRule(KonohaContext *kctx, kStmt *stmt, kToken *tk, kToken *rule, int s, int canRollBack)
{
	if(canRollBack) return s;
	kToken_p(stmt, tk, ERR_, "%s%s expects %s%s", T_statement(stmt->syn->keyword), KW_t(rule->keyword));
	return -1;
}

static int matchSyntaxRule(KonohaContext *kctx, kStmt *stmt, kArray *rules, kArray *tls, int s, int e, int canRollBack)
{
	int ri, ti, rule_size = kArray_size(rules);
	dumpTokenArray(kctx, 0, tls, s, e);
	dumpTokenArray(kctx, 0, rules, 0, rule_size);
	ti = s;
	for(ri = 0; ri < rule_size && ti < e; ri++) {
		DBG_P("matching token=%d<%d, rule=%d<%d", ti, e, ri, rule_size);
		kToken *rule = rules->toks[ri];
		kToken *tk = tls->toks[ti];
		if(KW_isPATTERN(rule->keyword)) {
			SugarSyntax *syn = SYN_(kStmt_nameSpace(stmt), rule->keyword);
			if(syn == NULL || syn->PatternMatch == kmodsugar->UndefinedParseExpr/*NULL*/) {
				kToken_p(stmt, tk, ERR_, "unknown syntax pattern: %s%s", KW_t(rule->keyword));
				return -1;
			}
			int c = e;
			kToken *rule1 = rules->toks[ri+1];
			if(ri + 1 < rule_size && (!KW_isPATTERN(rule1->keyword) && rule1->keyword != AST_OPTIONAL)) {
				DBG_P("NEXT rule=%s%s, rule_size=%d,%d", KW_t(rule1->keyword), ri+1, rule_size);
				c = lookAheadKeyword(tls, ti+1, e, rule1);
				if(c == -1) {
					DBG_P("@");
					return kStmt_printExpectedRule(kctx, stmt, tk, rule, s, canRollBack);
				}
				ri++;
			}
			DBG_P("syntax rule=%s%s", KW_t(syn->keyword));
			int next = PatternMatch(kctx, syn, stmt, rule->patternKey, tls, ti, c);
			DBG_P("matched '%s%s' patternKey='%s%s', next=%d=>%d", KW_t(rule->keyword), KW_t(rule->patternKey), ti, next);
			if(next == -1) {
				DBG_P("@");
				return kStmt_printExpectedRule(kctx, stmt, tk, rule, s, canRollBack);
			}
			ti = (c == e) ? next : c + 1;
		}
		else if(rule->keyword == AST_OPTIONAL) {
			int next = matchSyntaxRule(kctx, stmt, rule->sub, tls, ti, e, 1);
			if(next == -1) return -1;
			ti = next;
		}
		else {
			if(rule->keyword != tk->keyword) {
				//DBG_P("matching rule=%d,%s%s token=%d,%s%s", ri,  KW_t(rule->keyword), ti-s, KW_t(tk->keyword));
				DBG_P("@");
				return kStmt_printExpectedRule(kctx, stmt, tk, rule, s, canRollBack);
			}
			if(rule->keyword == AST_PARENTHESIS || rule->keyword == AST_BRACKET) {
				int next = matchSyntaxRule(kctx, stmt, rule->sub, tk->sub, 0, kArray_size(tk->sub), 0);
				if(next == -1) return -1;
			}
			ti++;
		}
		DBG_P("matching next token=%d<%d, rule=%d<%d", ti, e, ri+1, rule_size);
	}
	if(!canRollBack) {
		for(; ri < rule_size; ri++) {
			kToken *rule = rules->toks[ri];
			if(rule->keyword != AST_OPTIONAL) {
				kStmt_p(stmt, ERR_, "%s%s needs syntax pattern: %s%s", T_statement(stmt->syn->keyword), KW_t(rule->keyword));
				return -1;
			}
		}
		if(ti < e) {
			kStmt_p(stmt, ERR_, "%s%s: unexpected token %s", T_statement(stmt->syn->keyword), kToken_s(tls->toks[ti]));
			return -1;
		}
	}
	return ti;
}

static inline kToken* TokenArray_nextToken(KonohaContext *kctx, kArray *tls, int s, int e)
{
	return (s < e) ? tls->toks[s] : K_NULLTOKEN;
}

static SugarSyntax* NameSpace_getSyntaxRule(KonohaContext *kctx, kNameSpace *ns, kArray *tls, int s, int e)
{
	kToken *tk = tls->toks[s];
	if(TK_isType(tk)) {
		tk = TokenArray_nextToken(kctx, tls, s+1, e);
		if(tk->keyword == TK_SYMBOL || tk->keyword == TK_TYPE) {
			tk = TokenArray_nextToken(kctx, tls, s+2, e);
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
		kToken *tk1 = TokenArray_nextToken(kctx, tls, s+1, e);
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
		for(i = s + 1; i < e; i++) {
			tk = tls->toks[i];
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

static kbool_t Stmt_parseSyntaxRule(KonohaContext *kctx, kStmt *stmt, kArray *tls, int s, int e)
{
	kbool_t ret = false;
	SugarSyntax *syn = NameSpace_getSyntaxRule(kctx, kStmt_nameSpace(stmt), tls, s, e);
	DBG_ASSERT(syn != NULL);
	if(syn->syntaxRuleNULL != NULL) {
		((kStmtVar*)stmt)->syn = syn;
		ret = (matchSyntaxRule(kctx, stmt, syn->syntaxRuleNULL, tls, s, e, 0) != -1);
	}
	else {
		kStmt_p(stmt, ERR_, "undefined syntax rule for '%s%s'", KW_t(syn->keyword));
	}
	return ret;
}

static void Block_addStmtLine(KonohaContext *kctx, kBlock *bk, kArray *tls, int s, int e, kToken *tkERR)
{
	kStmtVar *stmt = new_Var(Stmt, tls->toks[s]->uline);
	kArray_add(bk->stmtList, stmt);
	KINITv(stmt->parentBlockNULL, bk);
	if(tkERR != NULL) {
		kStmt_toERR(stmt, tkERR->text);
	}
	else {
		int c = Stmt_addAnnotation(kctx, stmt, tls, s, e);
		Stmt_parseSyntaxRule(kctx, stmt, tls, c, e);
	}
	DBG_ASSERT(stmt->syn != NULL);
}

/* ------------------------------------------------------------------------ */

static KMETHOD UndefinedParseExpr(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_ParseExpr(stmt, tls, s, c, e);
	kStmt_p(stmt, ERR_, "undefined expression parser for '%s'", kToken_s(tls->toks[c]));
}

static kExpr *ParseExprFunc(KonohaContext *kctx, SugarSyntax *syn, kFunc *fo, kStmt *stmt, kArray *tls, int s, int c, int e)
{
	BEGIN_LOCAL(lsfp, K_CALLDELTA + 6);
	KSETv(lsfp[K_CALLDELTA+0].o, fo->self);
	lsfp[K_CALLDELTA+0].ndata = (uintptr_t)syn;
	KSETv(lsfp[K_CALLDELTA+1].o, (kObject*)stmt);
	KSETv(lsfp[K_CALLDELTA+2].a, tls);
	lsfp[K_CALLDELTA+3].ivalue = s;
	lsfp[K_CALLDELTA+4].ivalue = c;
	lsfp[K_CALLDELTA+5].ivalue = e;
	KCALL(lsfp, 0, fo->mtd, 5, K_NULLEXPR);
	END_LOCAL();
	DBG_ASSERT(IS_Expr(lsfp[0].o));
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
		fo = a->funcs[0];
	}
	DBG_ASSERT(IS_Func(fo));
	texpr = ParseExprFunc(kctx, syn, fo, stmt, tls, s, c, e);
	if(texpr == K_NULLEXPR && !kStmt_isERR(stmt)) {
		kStmt_p(stmt, ERR_, "syntax error: operator %s", kToken_s(tls->toks[c]));
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
		kToken *tk = tls->toks[i];
		if(!Stmt_isUnaryOp(kctx, stmt, tk)) break;
	}
	return i;
}

static int Stmt_findBinaryOp(KonohaContext *kctx, kStmt *stmt, kArray *tls, int s, int e, SugarSyntax **synRef)
{
	int idx = -1, i, prif = 0;
	for(i = Stmt_skipUnaryOp(kctx, stmt, tls, s, e) + 1; i < e; i++) {
		kToken *tk = tls->toks[i];
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
		kToken *tk = tls->toks[i];
		if(tk->keyword == KW_COMMA) {
			expr = Expr_add(kctx, expr, Stmt_newExpr2(kctx, stmt, tls, start, i));
			start = i + 1;
		}
	}
	if(allowEmpty == 0 || start < i) {
		expr = Expr_add(kctx, expr, Stmt_newExpr2(kctx, stmt, tls, start, i));
	}
	kArray_clear(tls, s);
	return expr;
}

static kExpr* Stmt_newExpr2(KonohaContext *kctx, kStmt *stmt, kArray *tls, int s, int e)
{
	if(!kStmt_isERR(stmt)) {
		if(s < e) {
			SugarSyntax *syn = NULL;
			int idx = Stmt_findBinaryOp(kctx, stmt, tls, s, e, &syn);
			if(idx != -1) {
				//DBG_P("** Found BinaryOp: s=%d, idx=%d, e=%d, '%s'**", s, idx, e, kToken_s(tls->toks[idx]));
				return ParseExpr(kctx, syn, stmt, tls, s, idx, e);
			}
			int c = s;
			syn = SYN_(kStmt_nameSpace(stmt), (tls->toks[c])->keyword);
			return ParseExpr(kctx, syn, stmt, tls, c, c, e);
		}
		if (0 < s - 1) {
			kStmt_p(stmt, ERR_, "expected expression after %s", kToken_s(tls->toks[s-1]));
		}
		else if(e < kArray_size(tls)) {
			kStmt_p(stmt, ERR_, "expected expression before %s", kToken_s(tls->toks[e]));
		}
		else {
			kStmt_p(stmt, ERR_, "expected expression");
		}
	}
	return K_NULLEXPR;
}

#define kExpr_rightJoin(EXPR, STMT, TLS, S, C, E)    Expr_rightJoin(kctx, EXPR, STMT, TLS, S, C, E)

static kExpr *Expr_rightJoin(KonohaContext *kctx, kExpr *expr, kStmt *stmt, kArray *tls, int s, int c, int e)
{
	if(c < e && expr != K_NULLEXPR && !kStmt_isERR(stmt)) {
		WARN_Ignored(kctx, tls, c, e);
	}
	return expr;
}

static KMETHOD ParseExpr_Term(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_ParseExpr(stmt, tls, s, c, e);
	DBG_ASSERT(s == c);
	kToken *tk = tls->toks[c];
	kExprVar *expr = new_Var(Expr, SYN_(kStmt_nameSpace(stmt), tk->keyword));
	PUSH_GCSTACK(expr);
	Expr_setTerm(expr, 1);
	KSETv(expr->tk, tk);
	RETURN_(kExpr_rightJoin(expr, stmt, tls, s+1, c+1, e));
}

static KMETHOD ParseExpr_Op(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_ParseExpr(stmt, tls, s, c, e);
	kTokenVar *tk = tls->Wtoks[c];
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
		kToken *tk = tls->toks[c+1];
		return (tk->keyword == TK_SYMBOL);
	}
	return false;
}
static KMETHOD ParseExpr_DOT(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_ParseExpr(stmt, tls, s, c, e);
	if(s < c && isFieldName(tls, c, e)) {
		kExpr *expr = Stmt_newExpr2(kctx, stmt, tls, s, c);
		expr = new_ConsExpr(kctx, syn, 2, tls->toks[c+1], expr);
		RETURN_(kExpr_rightJoin(expr, stmt, tls, c+2, c+2, e));
	}
}

static KMETHOD ParseExpr_Parenthesis(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_ParseExpr(stmt, tls, s, c, e);
	kToken *tk = tls->toks[c];
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

static KMETHOD ParseExpr_COMMA(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_ParseExpr(stmt, tls, s, c, e);
	kExpr *expr = new_ConsExpr(kctx, syn, 1, tls->toks[c]);
	expr = Stmt_addExprParams(kctx, stmt, expr, tls, s, e, 0/*allowEmpty*/);
	RETURN_(expr);
}

static KMETHOD ParseExpr_DOLLAR(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_ParseExpr(stmt, tls, s, c, e);
	if(s == c && c + 1 < e) {
		kToken *tk = tls->toks[c+1];
		if(tk->keyword == TK_CODE) {
			Token_toBRACE(kctx, (kTokenVar*)tk, kStmt_nameSpace(stmt));
		}
		if(tk->keyword == AST_BRACE) {
			kExprVar *expr = new_Var(Expr, SYN_(kStmt_nameSpace(stmt), KW_BlockPattern));
			PUSH_GCSTACK(expr);
			Expr_setTerm(expr, 1);
			KSETv(expr->tk, tk);
			KSETv(expr->block, new_Block(kctx, kStmt_nameSpace(stmt), stmt, tk->sub, 0, kArray_size(tk->sub), ';'));
			RETURN_(expr);
		}
	}
}

/* ------------------------------------------------------------------------ */

static KMETHOD PatternMatch_Expr(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_PatternMatch(stmt, name, tls, s, e);
	INIT_GCSTACK();
	int r = -1;
	dumpTokenArray(kctx, 0, tls, s, e);
	kExpr *expr = Stmt_newExpr2(kctx, stmt, tls, s, e);
	if(expr != K_NULLEXPR) {
		dumpExpr(kctx, 0, 0, expr);
		kObject_setObject(stmt, name, expr);
		r = e;
	}
	RESET_GCSTACK();
	RETURNi_(r);
}

static KMETHOD PatternMatch_Type(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_PatternMatch(stmt, name, tls, s, e);
	int r = -1;
	kToken *tk = tls->toks[s];
	if(TK_isType(tk)) {
		kObject_setObject(stmt, name, tk);
		r = s + 1;
	}
	RETURNi_(r);
}

static KMETHOD PatternMatch_Usymbol(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_PatternMatch(stmt, name, tls, s, e);
	int r = -1;
	kToken *tk = tls->toks[s];
	if(tk->keyword == TK_SYMBOL && isUpperCaseSymbol(S_text(tk->text))) {
		kObject_setObject(stmt, name, tk);
		r = s + 1;
	}
	RETURNi_(r);
}

static KMETHOD PatternMatch_Symbol(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_PatternMatch(stmt, name, tls, s, e);
	int r = -1;
	kToken *tk = tls->toks[s];
	if(tk->keyword == TK_SYMBOL) {
		kObject_setObject(stmt, name, tk);
		r = s + 1;
	}
	RETURNi_(r);
}

static KMETHOD PatternMatch_Params(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_PatternMatch(stmt, name, tls, s, e);
	int r = -1;
	kToken *tk = tls->toks[s];
	if(tk->keyword == AST_PARENTHESIS) {
		kArray *tls = tk->sub;
		int ss = 0, ee = kArray_size(tls);
		if(0 < ee && tls->toks[0]->keyword == KW_void) ss = 1;  //  f(void) = > f()
		kBlock *bk = new_Block(kctx, kStmt_nameSpace(stmt), stmt, tls, ss, ee, ',');
		kObject_setObject(stmt, name, bk);
		r = s + 1;
	}
	RETURNi_(r);
}

static KMETHOD PatternMatch_Block(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_PatternMatch(stmt, name, tls, s, e);
	kToken *tk = tls->toks[s];
	dumpTokenArray(kctx, 0, tls, s, e);
	if(tk->keyword == TK_CODE) {
		kObject_setObject(stmt, name, tk);
		RETURNi_(s+1);
	}
//	else if(tk->keyword == AST_BRACE) {
//		kBlock *bk = new_Block(kctx, kStmt_nameSpace(stmt), stmt, tk->sub, 0, kArray_size(tk->sub), ';');
//		kObject_setObject(stmt, name, bk);
//		RETURNi_(s+1);
//	}
	else {
		kBlock *bk = new_Block(kctx, kStmt_nameSpace(stmt), stmt, tls, s, e, ';');
		kObject_setObject(stmt, name, bk);
		RETURNi_(e);
	}
	RETURNi_(-1); // ERROR
}

static KMETHOD PatternMatch_Toks(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	VAR_PatternMatch(stmt, name, tls, s, e);
	if(s < e) {
		kArray *a = new_(TokenArray, (intptr_t)(e - s));
		while(s < e) {
			kArray_add(a, tls->toks[s]);
			s++;
		}
		kObject_setObject(stmt, name, a);
		RETURNi_(e);
	}
	RETURNi_(-1);
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
