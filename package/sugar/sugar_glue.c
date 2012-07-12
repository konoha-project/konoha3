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

#include <konoha2/konoha2.h>
#include <konoha2/sugar.h>
#include <stdio.h>

//## boolean Token.isTypeName();
static KMETHOD Token_isTypeName(CTX, ksfp_t *sfp _RIX)
{
	RETURNb_(TK_isType(sfp[0].tk));
}

//## boolean Token.isParenthesis();
static KMETHOD Token_isParenthesis(CTX, ksfp_t *sfp _RIX)
{
	RETURNb_(sfp[0].tk->kw == AST_PARENTHESIS);
}

//## int Stmt.getBuild();
static KMETHOD Stmt_getBuild(CTX, ksfp_t *sfp _RIX)
{
	RETURNi_(sfp[0].stmt->build);
}

//## void Stmt.setBuild(int buildid);
static KMETHOD Stmt_setBuild(CTX, ksfp_t *sfp _RIX)
{
	struct _kStmt *stmt = (struct _kStmt *) sfp[0].stmt;
	stmt->build = sfp[1].ivalue;
}

//## Block Stmt.getBlock(String key, Block def);
static KMETHOD Stmt_getBlock(CTX, ksfp_t *sfp _RIX)
{
	USING_SUGAR;
	kString *key = sfp[1].s;
	RETURN_(kStmt_block(sfp[0].stmt, ksymbolA(S_text(key), S_size(key), _NEWID), sfp[2].bk));
}

//## boolean Stmt.tyCheckExpr(String key, Gamma gma, int typeid, int pol);
static KMETHOD Stmt_tyCheckExpr(CTX, ksfp_t *sfp _RIX)
{
	USING_SUGAR;
	kString *key = sfp[1].s;
	RETURNb_(SUGAR Stmt_tyCheckExpr(_ctx, sfp[0].stmt, ksymbolA(S_text(key), S_size(key), _NEWID), sfp[2].gma, (ktype_t)sfp[3].ivalue, (int)sfp[4].ivalue));
}

//## boolean Blook.tyCheckAll(Gamma gma);
static KMETHOD Block_tyCheckAll(CTX, ksfp_t *sfp _RIX)
{
	USING_SUGAR;
	RETURNb_(SUGAR Block_tyCheckAll(_ctx, sfp[0].bk, sfp[1].gma));
}

// --------------------------------------------------------------------------

//## void NameSpace.addTokenizeFunc(String keyword, Func f);
static KMETHOD NameSpace_addTokenizeFunc(CTX, ksfp_t *sfp _RIX)
{
	USING_SUGAR;
	SUGAR NameSpace_setTokenizeFunc(_ctx, sfp[0].ks, S_text(sfp[1].s)[0], NULL, sfp[2].fo, 1/*isAddition*/);
}

//## void NameSpace.addPatternMatch(String keyword, Func f);
static KMETHOD NameSpace_addPatternMatch(CTX, ksfp_t *sfp _RIX)
{
	USING_SUGAR;
	kString *key = sfp[1].s;
	SUGAR SYN_addSugarFunc(_ctx, sfp[0].ks, ksymbolA(S_text(key), S_size(key), _NEWID), SYNIDX_PatternMatch, sfp[2].fo);
}

//## void NameSpace.addParseExpr(String keyword, Func f);
static KMETHOD NameSpace_addParseExpr(CTX, ksfp_t *sfp _RIX)
{
	USING_SUGAR;
	kString *key = sfp[1].s;
	SUGAR SYN_addSugarFunc(_ctx, sfp[0].ks, ksymbolA(S_text(key), S_size(key), _NEWID), SYNIDX_ParseExpr, sfp[2].fo);
}

//## void NameSpace.addStmtTyCheck(String keyword, Func f);
static KMETHOD NameSpace_addStmtTyCheck(CTX, ksfp_t *sfp _RIX)
{
	USING_SUGAR;
	kString *key = sfp[1].s;
	SUGAR SYN_addSugarFunc(_ctx, sfp[0].ks, ksymbolA(S_text(key), S_size(key), _NEWID), SYNIDX_StmtTyCheck, sfp[2].fo);
}


//## void NameSpace.addTopStmtTyCheck(String keyword, Func f);
static KMETHOD NameSpace_addTopStmtTyCheck(CTX, ksfp_t *sfp _RIX)
{
	USING_SUGAR;
	kString *key = sfp[1].s;
	SUGAR SYN_addSugarFunc(_ctx, sfp[0].ks, ksymbolA(S_text(key), S_size(key), _NEWID), SYNIDX_TopStmtTyCheck, sfp[2].fo);
}

//## void NameSpace.addExprTyCheck(String keyword, Func f);
static KMETHOD NameSpace_addExprTyCheck(CTX, ksfp_t *sfp _RIX)
{
	USING_SUGAR;
	kString *key = sfp[1].s;
	SUGAR SYN_addSugarFunc(_ctx, sfp[0].ks, ksymbolA(S_text(key), S_size(key), _NEWID), SYNIDX_ExprTyCheck, sfp[2].fo);
}


//## Expr Stmt.printError(String msg);
static KMETHOD Stmt_printError(CTX, ksfp_t *sfp _RIX)
{
	USING_SUGAR;
	kStmt   *stmt  = sfp[0].stmt;
	kString *msg   = sfp[1].s;
	SUGAR Stmt_p(_ctx, stmt, NULL, ERR_, "%s", S_text(msg));
	RETURN_(K_NULLEXPR);
}

// --------------------------------------------------------------------------
// AST Method

//static ksyntax_t* get_syntax(CTX, kNameSpace *ks, kString *key)
//{
//	USING_SUGAR;
//	symbol_t kw = KW_s(key);
//	if(kw == SYM_NONAME) {
//		kreportf(CRIT_, "undefined keyword: %s", S_text(key));
//	}
//	ksyntax_t *syn = SYN_(ks, kw);
//	if(syn == NULL) {
//		kreportf(CRIT_, "undefined syntax: %s", S_text(key));
//	}
//	return syn;
//}

////## Expr Token.printSyntaxError();
//static KMETHOD Token_printSyntaxError(CTX, ksfp_t *sfp _RIX)
//{
//	USING_SUGAR;
//	kToken *tk  = sfp[0].tk;
//	if(IS_String(tk->text)) {
//		SUGAR p(_ctx, ERR_, tk->uline, tk->lpos, "syntax error: %s", S_text(tk->text));
//	}
//	else {
//		SUGAR p(_ctx, ERR_, tk->uline, tk->lpos, "syntax error");
//	}
//	RETURN_(K_NULLEXPR);
//}

////## Expr Stmt.newBlock(Token[] tls, int s, int e);
//static KMETHOD Stmt_newBlock(CTX, ksfp_t *sfp _RIX)
//{
//	USING_SUGAR;
//	kStmt *stmt  = sfp[0].stmt;
//	kArray *tls  = sfp[1].a;
//	int s = sfp[2].ivalue, e = sfp[3].ivalue;
//	RETURN_(SUGAR new_Block(_ctx, kStmt_ks(stmt), stmt, tls, s, e, ';'));
//}

//## Expr Stmt.newExpr(Token[] tls, int s, int e);
static KMETHOD Stmt_newExpr(CTX, ksfp_t *sfp _RIX)
{
	USING_SUGAR;
	kStmt *stmt  = sfp[0].stmt;
	kArray *tls  = sfp[1].a;
	int s = sfp[2].ivalue, e = sfp[3].ivalue;
	RETURN_(SUGAR Stmt_newExpr2(_ctx, stmt, tls, s, e));
}

////## Expr Stmt.newMethodCallExpr(Token key, Token self);
//static KMETHOD Stmt_newMethodCallExpr(CTX, ksfp_t *sfp _RIX)
//{
//	USING_SUGAR;
//	kStmt *stmt  = sfp[0].stmt;
//	kToken *tk   = sfp[1].tk;
//	assert(tk->kw != 0);
//	struct _kExpr *expr = new_W(Expr, SYN_(kStmt_ks(stmt), tk->kw));
//	KSETv(expr->tk, tk);
//	KSETv(expr->cons, new_(Array, 8));
//	RETURN_(expr);
//}

////## Expr Stmt.addExprParam(Token tk);
//static KMETHOD Stmt_addExprParam(CTX, ksfp_t *sfp _RIX)
//{
//	USING_SUGAR;
//	kStmt *stmt  = sfp[0].stmt;
//	kExpr *expr  = sfp[1].expr;
//	kToken *tk     = sfp[2].tk;
//	if(tk->tt != AST_PARENTHESIS || tk->tt != AST_BRACKET) {
//		SUGAR p(_ctx, WARN_, tk->uline, tk->lpos, "not parameter token");
//		kObject_setNullObject(expr, 1);
//	}
//	if(IS_NOTNULL(expr)) {
//		assert(IS_Array(tk->sub));
//		expr = SUGAR Stmt_addExprParams(_ctx, stmt, expr, tk->sub, 0, kArray_size(tk->sub), 1/*allowEmpty*/);
//	}
//	RETURN_(expr);
//}

////## Expr Expr.addExpr(Expr expr, Expr o);
//static KMETHOD Expr_addExpr(CTX, ksfp_t *sfp _RIX)
//{
//	kExpr *expr  = sfp[0].expr;
//	kExpr *o     = sfp[1].expr;
//	if(IS_NULL(o) && IS_Array(expr->cons)) {
//		kObject_setNullObject(expr, 1);
//	}
//	if(IS_NOTNULL(expr)) {
//		kArray_add(expr->cons, o);
//	}
//	RETURN_(expr);
//}

// --------------------------------------------------------------------------

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

static	kbool_t sugar_initPackage(CTX, kNameSpace *ks, int argc, const char**args, kline_t pline)
{
	USING_SUGAR;
	int FN_buildid = FN_("buildid"), FN_key = FN_("key"), FN_defval = FN_("defval");
	int FN_typeid = FN_("typeid"), FN_gma = FN_("gma"), FN_pol = FN_("pol");
	int FN_func = FN_("func"), FN_msg = FN_("msg");
//	int FN_tls = FN_("tokens"), FN_s = FN_("s"), FN_e = FN_("e");

	/* Func[Int, Token, String] */
	kparam_t P_FuncTokenize[] = {{TY_Token}, {TY_String}};
	int TY_FuncTokenize = (kClassTable_Generics(CT_Func, TY_Int, 2, P_FuncTokenize))->cid;
	/* Func[Int, Stmt, Int, Token[], Int, Int] */
	kparam_t P_FuncPatternMatch[] = {{TY_Stmt}, {TY_Int}, {TY_TokenArray}, {TY_Int}, {TY_Int}};
	int TY_FuncPatternMatch = (kClassTable_Generics(CT_Func, TY_Int, 5, P_FuncPatternMatch))->cid;
	/* Func[Expr, Stmt, Token[], Int, Int, Int] */
	kparam_t P_FuncParseExpr[] = {{TY_Stmt}, {TY_TokenArray}, {TY_Int}, {TY_Int}, {TY_Int}};
	int TY_FuncParseExpr = (kClassTable_Generics(CT_Func, TY_Expr, 5, P_FuncParseExpr))->cid;
	/* Func[Boolean, Stmt, Gamma] */
	kparam_t P_FuncStmtTyCheck[] = {{TY_Stmt}, {TY_Gamma}};
	int TY_FuncStmtTyCheck = (kClassTable_Generics(CT_Func, TY_Boolean, 2, P_FuncStmtTyCheck))->cid;
	/* Func[Expr, Stmt, Expr, Gamma, Int] */
	kparam_t P_FuncExprTyCheck[] = {{TY_Stmt}, {TY_Expr}, {TY_Gamma}, {TY_Int}};
	int TY_FuncExprTyCheck = (kClassTable_Generics(CT_Func, TY_Expr, 4, P_FuncExprTyCheck))->cid;
	//DBG_P("func=%s", TY_t(TY_FuncExprTyCheck));

	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Token_isTypeName), TY_Boolean, TY_Token, MN_("isTypeName"), 0,
		_Public, _F(Token_isParenthesis), TY_Boolean, TY_Token, MN_("isParenthesis"), 0,

		_Public, _F(Stmt_getBuild), TY_Int, TY_Stmt,  MN_("getBuild"), 0,
		_Public, _F(Stmt_setBuild), TY_void, TY_Stmt, MN_("setBuild"), 1, TY_Int, FN_buildid,
		_Public, _F(Stmt_getBlock), TY_Block, TY_Stmt, MN_("getBlock"), 2, TY_String, FN_key, TY_Object, FN_defval,
		_Public, _F(Stmt_tyCheckExpr), TY_Boolean, TY_Stmt, MN_("tyCheckExpr"), 4, TY_String, FN_key, TY_Gamma, FN_gma, TY_Int, FN_typeid, TY_Int, FN_pol,
		_Public, _F(Block_tyCheckAll), TY_Boolean, TY_Block, MN_("tyCheckAll"), 1, TY_Gamma, FN_gma,

		_Public, _F(NameSpace_addTokenizeFunc), TY_void, TY_NameSpace, MN_("addTokenizeFunc"), 2, TY_String, FN_key, TY_FuncTokenize, FN_func,
		_Public, _F(NameSpace_addPatternMatch), TY_void, TY_NameSpace, MN_("addPatternMatch"), 2, TY_String, FN_key, TY_FuncPatternMatch, FN_func,
		_Public, _F(NameSpace_addParseExpr), TY_void, TY_NameSpace, MN_("addParseExpr"), 2, TY_String, FN_key, TY_FuncParseExpr, FN_func,
		_Public, _F(NameSpace_addTopStmtTyCheck), TY_void, TY_NameSpace, MN_("addTopStmtTyCheck"), 2, TY_String, FN_key, TY_FuncStmtTyCheck, FN_func,
		_Public, _F(NameSpace_addStmtTyCheck), TY_void, TY_NameSpace, MN_("addStmtTyCheck"), 2, TY_String, FN_key, TY_FuncStmtTyCheck, FN_func,
		_Public, _F(NameSpace_addExprTyCheck), TY_void, TY_NameSpace, MN_("addExprTyCheck"), 2, TY_String, FN_key, TY_FuncExprTyCheck, FN_func,

		_Public, _F(Stmt_printError), TY_Expr, TY_Stmt, MN_("printError"), 1, TY_String, FN_msg,

		_Public, _F(Stmt_newExpr), TY_Expr, TY_Stmt, MN_("newExpr"), 1, TY_String, FN_key,
//		_Public, _F(Stmt_parsedExpr), TY_Expr, TY_Stmt, MN_("parseExpr"), 3, TY_TokenArray, FN_tls, TY_Int, FN_s, TY_Int, FN_e,
		DEND,
	};
	kNameSpace_loadMethodData(NULL, MethodData);
	return true;
}

static kbool_t sugar_setupPackage(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

static kbool_t isSubKeyword(CTX, kArray *tls, int s, int e)
{
	if(s+1 < e && tls->toks[s+1]->kw == TK_TEXT) {
		const char *t = S_text(tls->toks[s+1]->text);
		if(isalpha(t[0]) || t[0] < 0 /* multibytes char */) {
			return 1;
		}
	}
	return 0;
}

static struct _ksyntax *toks_syntax(CTX, kNameSpace *ks, kArray *tls)
{
	USING_SUGAR;
	int s = 0, e = kArray_size(tls);
	if(s < e) {
		if(tls->toks[s]->kw == TK_TEXT) {
			ksymbol_t kw;
			if(isSubKeyword(_ctx, tls, s, e)) {
				char buf[256];
				snprintf(buf, sizeof(buf), "%s %s", S_text(tls->toks[s]->text), S_text(tls->toks[s+1]->text));
				kw = ksymbolA((const char*)buf, strlen(buf), SYM_NEWID);
			}
			else {
				kw = ksymbolA(S_text(tls->toks[s]->text), S_size(tls->toks[s]->text), SYM_NEWID);
			}
			return (struct _ksyntax*)NEWSYN_(ks, kw);
		}
	}
	return NULL;
}

static KMETHOD StmtTyCheck_sugar(CTX, ksfp_t *sfp _RIX)
{
	USING_SUGAR;
	kbool_t r = 0;
	VAR_StmtTyCheck(stmt, gma);
	kTokenArray *tls = (kTokenArray*)kObject_getObject(stmt, KW_ToksPattern, NULL);
	if(tls != NULL) {
		struct _ksyntax *syn = toks_syntax(_ctx, gma->genv->ks, tls);
		if(syn != NULL) {
			if(syn->syntaxRuleNULL != NULL) {
				SUGAR Stmt_p(_ctx, stmt, NULL, WARN_, "overriding syntax rule: %s", KW_t(syn->kw));
				kArray_clear(syn->syntaxRuleNULL, 0);
			}
			else {
				KINITv(syn->syntaxRuleNULL, new_(Array, 8));
			}
			if(SUGAR makeSyntaxRule(_ctx, tls, 0, kArray_size(tls), syn->syntaxRuleNULL)) {
				r = 1;
			}
			else {
				kArray_clear(syn->syntaxRuleNULL, 0);
			}
		}
		kStmt_done(stmt);
	}
	RETURNb_(r);
}

static kbool_t sugar_initNameSpace(CTX,  kNameSpace *ks, kline_t pline)
{
	USING_SUGAR;
	KDEFINE_INT_CONST IntData[] = {
#define DEFINE_KEYWORD(KW) {#KW, TY_Int, KW}
		DEFINE_KEYWORD(KW_ExprPattern),
		DEFINE_KEYWORD(KW_SymbolPattern),
		DEFINE_KEYWORD(KW_UsymbolPattern),
		DEFINE_KEYWORD(KW_TextPattern),
		DEFINE_KEYWORD(KW_IntPattern),
		DEFINE_KEYWORD(KW_FloatPattern),
		DEFINE_KEYWORD(KW_TypePattern),
		DEFINE_KEYWORD(KW_ParenthesisPattern),
		DEFINE_KEYWORD(KW_BracketPattern),
		DEFINE_KEYWORD(KW_BracePattern),
		DEFINE_KEYWORD(KW_BlockPattern),
		DEFINE_KEYWORD(KW_ParamsPattern),
		DEFINE_KEYWORD(KW_ToksPattern),
		DEFINE_KEYWORD(TSTMT_UNDEFINED),
		DEFINE_KEYWORD(TSTMT_ERR),
		DEFINE_KEYWORD(TSTMT_EXPR),
		DEFINE_KEYWORD(TSTMT_BLOCK),
		DEFINE_KEYWORD(TSTMT_RETURN),
		DEFINE_KEYWORD(TSTMT_IF),
		DEFINE_KEYWORD(TSTMT_LOOP),
		DEFINE_KEYWORD(TSTMT_JUMP),
		DEFINE_KEYWORD(TEXPR_CONST),
		DEFINE_KEYWORD(TEXPR_NEW),
		DEFINE_KEYWORD(TEXPR_NULL),
		DEFINE_KEYWORD(TEXPR_NCONST),
		DEFINE_KEYWORD(TEXPR_LOCAL),
		DEFINE_KEYWORD(TEXPR_BLOCK),
		DEFINE_KEYWORD(TEXPR_FIELD),
		DEFINE_KEYWORD(TEXPR_BOX),
		DEFINE_KEYWORD(TEXPR_UNBOX),
		DEFINE_KEYWORD(TEXPR_CALL),
		DEFINE_KEYWORD(TEXPR_AND),
		DEFINE_KEYWORD(TEXPR_OR),
		DEFINE_KEYWORD(TEXPR_LET),
		DEFINE_KEYWORD(TEXPR_STACKTOP),
#undef DEFINE_KEYWORD
		{NULL},
	};
	kNameSpace_loadConstData(ks, IntData, pline);
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .kw = SYM_("sugar"), .rule ="\"sugar\" $toks", TopStmtTyCheck_(sugar), },
		{ .kw = KW_END, },
	};
	SUGAR NameSpace_defineSyntax(_ctx, ks, SYNTAX);
	return true;
}

static kbool_t sugar_setupNameSpace(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* sugar_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("sugar", "1.0"),
		.initPackage = sugar_initPackage,
		.setupPackage = sugar_setupPackage,
		.initNameSpace = sugar_initNameSpace,
		.setupNameSpace = sugar_setupNameSpace,
	};
	return &d;
}
