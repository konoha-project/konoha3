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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"{
#endif

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Im       kMethod_Immutable
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

/* Symbol class */

static ksymbol_t StringToKsymbol(KonohaContext *kctx, kString *key)
{
	return ksymbolA(S_text(key), S_size(key), _NEWID);
}

static void kSymbol_p(KonohaContext *kctx, KonohaValue *v, int pos, KGrowingBuffer *wb)
{
	ksymbol_t symbol = (ksymbol_t)v[pos].unboxValue;
	DBG_P(">>> symbol=%d, %s%s", symbol, PSYM_t(symbol));
	KLIB Kwb_printf(kctx, wb, "%s%s", PSYM_t(symbol));
}

//## symbol String.toSymbol();
static KMETHOD String_toSymbol(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = StringToKsymbol(kctx, sfp[0].asString);
	DBG_P(">>> symbol=%d, %s%s", keyword, PSYM_t(keyword));
	KReturnUnboxValue(keyword);
}

static KonohaClass *loadSymbolClass(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	static KDEFINE_CLASS defSymbol = {0};
	defSymbol.structname = "symbol";
	defSymbol.cflag = CFLAG_int;
	defSymbol.init = CT_(TY_int)->init;
	defSymbol.unbox = CT_(TY_int)->unbox;
	defSymbol.p = kSymbol_p;
	KonohaClass *cSymbol = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defSymbol, trace);
	KDEFINE_METHOD MethodData[] = {
		_Public|_Coercion|_Const, _F(String_toSymbol), cSymbol->typeId, TY_String, MN_to(cSymbol->typeId), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return cSymbol;
}

// --------------------------------------------------------------------------
/* cid class */

static void kcid_p(KonohaContext *kctx, KonohaValue *v, int pos, KGrowingBuffer *wb)
{
	ktype_t cid = (ktype_t)v[pos].intValue;
	DBG_P(">>> Class=%s, cid=%d", SYM_t(CT_(cid)->classNameSymbol), cid);
	KLIB Kwb_printf(kctx, wb, "%s%s", PSYM_t(CT_(cid)->classNameSymbol));
}

//## cid Object.tocid();
static KMETHOD Object_tocid(KonohaContext *kctx, KonohaStack *sfp)
{
	kObject *o = sfp[0].asObject;
	ktype_t cid = O_typeId(o);
	DBG_P(">>> Class=%s, cid=%d", SYM_t(CT_(cid)->classNameSymbol), cid);
	KReturnUnboxValue(cid);
}

static KonohaClass *loadcidClass(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	static KDEFINE_CLASS defcid = {0};
	defcid.structname = "cid";
	defcid.cflag = CFLAG_int;
	defcid.init = CT_(TY_int)->init;
	defcid.unbox = CT_(TY_int)->unbox;
	defcid.p = kcid_p;
	KonohaClass *ccid = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defcid, trace);
	KDEFINE_METHOD MethodData[] = {
		_Public|_Coercion|_Const, _F(Object_tocid), ccid->typeId, TY_Object, MN_to(ccid->typeId), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return ccid;
}

// --------------------------------------------------------------------------
/* NameSpace */

static kbool_t SugarSyntax_hasSyntaxPatternList(SugarSyntax *syn)
{
	while(syn != NULL) {
		if(syn->syntaxPatternListNULL_OnList != NULL) return true;
		syn = syn->parentSyntaxNULL;
	}
	return false;
}

static kbool_t SugarSyntax_hasSugarFunc(SugarSyntax *syn, int index)
{
	while(syn != NULL) {
		if(syn->sugarFuncListTable[index] != NULL) return true;
		syn = syn->parentSyntaxNULL;
	}
	return false;
}

//## boolean NameSpace.definedSyntax(symbol keyword);
static KMETHOD NameSpace_DefinedSyntax(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	SugarSyntax* syn = SYN_(sfp[0].asNameSpace, keyword);
	KReturnUnboxValue(syn != NULL);
}

//## boolean NameSpace.definedLiteral(symbol keyword);
static KMETHOD NameSpace_DefinedLiteral(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	SugarSyntax* syn = SYN_(sfp[0].asNameSpace, keyword);
	KReturnUnboxValue(SugarSyntax_hasSugarFunc(syn, SugarFunc_TokenFunc));
}

//## boolean NameSpace.definedStatement(symbol keyword);
static KMETHOD NameSpace_DefinedStatement(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	SugarSyntax* syn = SYN_(sfp[0].asNameSpace, keyword);
	KReturnUnboxValue(SugarSyntax_hasSyntaxPatternList(syn) && SugarSyntax_hasSugarFunc(syn, SugarFunc_Statement));
}

//## boolean NameSpace.definedExpression(symbol keyword);
static KMETHOD NameSpace_DefinedExpression(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	SugarSyntax* syn = SYN_(sfp[0].asNameSpace, keyword);
	KReturnUnboxValue(syn != NULL && (syn->precedence_op2 > 0 || syn->precedence_op1 > 0));
}

//## boolean NameSpace.definedBinaryOperator(symbol keyword);
static KMETHOD NameSpace_DefinedBinaryOperator(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	SugarSyntax* syn = SYN_(sfp[0].asNameSpace, keyword);
	KReturnUnboxValue(syn != NULL && (syn->precedence_op2 > 0));
}

//## void NameSpace.setTokenFunc(symbol keyword, int konohaChar, Func f);
static KMETHOD NameSpace_setTokenFunc(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	int konohaChar = (int)sfp[2].intValue;
	SUGAR kNameSpace_SetTokenFunc(kctx, sfp[0].asNameSpace, keyword, konohaChar, sfp[3].asFunc);
}

////## void NameSpace.compileAllDefinedMethods();
//static KMETHOD NameSpace_compileAllDefinedMethods(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KLIB kNameSpace_compileAllDefinedMethods(kctx);
//}

//## void NameSpace.addPatternMatch(symbol keyword, Func f);
static KMETHOD NameSpace_AddPatternMatch(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	SUGAR kNameSpace_AddSugarFunc(kctx, sfp[0].asNameSpace, keyword, SugarFunc_PatternMatch, sfp[2].asFunc);
}

//## void NameSpace.addExpression(symbol keyword, Func f);
static KMETHOD NameSpace_AddExpression(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	SUGAR kNameSpace_AddSugarFunc(kctx, sfp[0].asNameSpace, keyword, SugarFunc_Expression, sfp[2].asFunc);
}

//## void NameSpace.addStatement(symbol keyword, Func f);
static KMETHOD NameSpace_AddStatement(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	SUGAR kNameSpace_AddSugarFunc(kctx, sfp[0].asNameSpace, keyword, SugarFunc_Statement, sfp[2].asFunc);
}


//## void NameSpace.addTopLevelStatement(symbol keyword, Func f);
static KMETHOD NameSpace_AddTopLevelStatement(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	SUGAR kNameSpace_AddSugarFunc(kctx, sfp[0].asNameSpace, keyword, SugarFunc_TopLevelStatement, sfp[2].asFunc);
}

//## void NameSpace.addTypeCheck(symbol keyword, Func f);
static KMETHOD NameSpace_AddTypeCheck(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	SUGAR kNameSpace_AddSugarFunc(kctx, sfp[0].asNameSpace, keyword, SugarFunc_TypeCheck, sfp[2].asFunc);
}

////## void NameSpace.addTermExpression(String keyword);
//static KMETHOD NameSpace_AddTermExpression(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kString *key = sfp[1].asString;
//	DBG_P("termparseexpr=%p", kmodsugar->Expression_Term);
//	SUGAR kNameSpace_AddSugarFunc(kctx, sfp[0].asNameSpace, ksymbolA(S_text(key), S_size(key), _NEWID), SugarFunc_Expression, kmodsugar->Expression_Term);
//}

// --------------------------------------------------------------------------

static void LoadNameSpaceMethodData(KonohaContext *kctx, kNameSpace *ns, int TY_symbol, KTraceInfo *trace)
{
	int FN_keyword = SYM_("keyword");
	int FN_func = SYM_("func");

	/* Func[Int, Token, String] */
	kparamtype_t P_FuncTokenize[] = {{TY_Token}, {TY_String}};
	int TY_FuncToken = (KLIB KonohaClass_Generics(kctx, CT_Func, TY_int, 2, P_FuncTokenize))->typeId;
	/* Func[Int, Stmt, Int, Token[], Int, Int] */
	kparamtype_t P_FuncPatternMatch[] = {{TY_Stmt}, {TY_int}, {TY_TokenArray}, {TY_int}, {TY_int}};
	int TY_FuncPatternMatch = (KLIB KonohaClass_Generics(kctx, CT_Func, TY_int, 5, P_FuncPatternMatch))->typeId;
	/* Func[Expr, Stmt, Token[], Int, Int, Int] */
	kparamtype_t P_FuncExpression[] = {{TY_Stmt}, {TY_TokenArray}, {TY_int}, {TY_int}, {TY_int}};
	int TY_FuncExpression = (KLIB KonohaClass_Generics(kctx, CT_Func, TY_Expr, 5, P_FuncExpression))->typeId;
	/* Func[Boolean, Stmt, Gamma] */
	kparamtype_t P_FuncStatement[] = {{TY_Stmt}, {TY_Gamma}};
	int TY_FuncStatement = (KLIB KonohaClass_Generics(kctx, CT_Func, TY_boolean, 2, P_FuncStatement))->typeId;
	/* Func[Expr, Stmt, Expr, Gamma, Int] */
	kparamtype_t P_FuncTypeCheck[] = {{TY_Stmt}, {TY_Expr}, {TY_Gamma}, {TY_int}};
	int TY_FuncTypeCheck = (KLIB KonohaClass_Generics(kctx, CT_Func, TY_Expr, 4, P_FuncTypeCheck))->typeId;
	//DBG_P("func=%s", TY_t(TY_FuncTypeCheck));

	KDEFINE_METHOD MethodData[] = {
		_Public|_Im, _F(NameSpace_DefinedSyntax), TY_boolean, TY_NameSpace, MN_("definedSyntax"), 1, TY_symbol, FN_keyword,
		_Public|_Im, _F(NameSpace_DefinedLiteral), TY_boolean, TY_NameSpace, MN_("definedLiteral"), 1, TY_symbol, FN_keyword,
		_Public|_Im, _F(NameSpace_DefinedStatement), TY_boolean, TY_NameSpace, MN_("definedStatement"), 1, TY_symbol, FN_keyword,
		_Public|_Im, _F(NameSpace_DefinedExpression), TY_boolean, TY_NameSpace, MN_("definedExpression"), 1, TY_symbol, FN_keyword,
		_Public|_Im, _F(NameSpace_DefinedBinaryOperator), TY_boolean, TY_NameSpace, MN_("definedBinaryOperator"), 1, TY_symbol, FN_keyword,
//		_Public, _F(NameSpace_compileAllDefinedMethods), TY_void, TY_NameSpace, MN_("compileAllDefinedMethods"), 0,
		_Public, _F(NameSpace_setTokenFunc), TY_void, TY_NameSpace, MN_("setTokenFunc"), 3, TY_symbol, FN_keyword, TY_int, FN_("kchar"), TY_FuncToken, FN_func,
		_Public, _F(NameSpace_AddPatternMatch), TY_void, TY_NameSpace, MN_("addPatternMatch"), 2, TY_symbol, FN_keyword, TY_FuncPatternMatch, FN_func,
		_Public, _F(NameSpace_AddExpression), TY_void, TY_NameSpace, MN_("addExpression"), 2, TY_symbol, FN_keyword, TY_FuncExpression, FN_func,
		_Public, _F(NameSpace_AddTopLevelStatement), TY_void, TY_NameSpace, MN_("addTopLevelStatement"), 2, TY_symbol, FN_keyword, TY_FuncStatement, FN_func,
		_Public, _F(NameSpace_AddStatement), TY_void, TY_NameSpace, MN_("addStatement"), 2, TY_symbol, FN_keyword, TY_FuncStatement, FN_func,
		_Public, _F(NameSpace_AddTypeCheck), TY_void, TY_NameSpace, MN_("addTypeCheck"), 2, TY_symbol, FN_keyword, TY_FuncTypeCheck, FN_func,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

// --------------------------------------------------------------------------
/* Token */

//## Token Token.new(symbol key);
static KMETHOD Token_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *)sfp[0].asToken;
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	tk->resolvedSymbol = key;
	KReturn(tk);
}

//## void Token.setKeyword(symbol keyword);
static KMETHOD Token_setUnresolvedTokenType(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *) sfp[0].asToken;
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	tk->unresolvedTokenType = keyword;
	DBG_P("setkeyword=%s%s", PSYM_t(keyword));
	KReturnVoid();
}

//## void Token.setText(String text);
static KMETHOD Token_setText(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *) sfp[0].asToken;
	kString *text = sfp[1].asString;
	KFieldSet(tk, tk->text, text);
	KReturnVoid();
}

//## void Token.setSubArray(String[] sub);
static KMETHOD Token_setSubArray(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *) sfp[0].asToken;
	kArray *sub = sfp[1].asArray;
	KFieldSet(tk, tk->subTokenList, sub);
	KReturnVoid();
}

////## boolean Token.isTypeName();
//static KMETHOD Token_isTypeName(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KReturnUnboxValue(Token_isVirtualTypeLiteral(sfp[0].asToken));
//}

//## boolean Token.isParenthesis();
static KMETHOD Token_isParenthesis(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *)sfp[0].asToken;
	if(tk->resolvedSyntaxInfo == NULL) {
		KReturnUnboxValue(false);
	}
	KReturnUnboxValue(tk->resolvedSyntaxInfo->keyword == KW_ParenthesisGroup);
}

//## String Token.getText();
static KMETHOD Token_getText(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *)sfp[0].asToken;
	KReturn(tk->text);
}

//## boolean Token.isSymbol();
static KMETHOD Token_isSymbol(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *)sfp[0].asToken;
	if(tk->resolvedSyntaxInfo == NULL) {
		KReturnUnboxValue(false);
	}
	KReturnUnboxValue(tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern);
}

//## Expr Token.newUntypedExpr();
static KMETHOD Token_newUntypedExpr(KonohaContext *kctx, KonohaStack *sfp)
{
	kToken *token = sfp[0].asToken;
	KReturn(SUGAR new_UntypedTermExpr(kctx, token));
}

//## boolean Token.isBeforeWhiteSpace();
static KMETHOD Token_isBeforeWhiteSpace(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kToken_is(BeforeWhiteSpace, sfp[0].asToken));
}

// --------------------------------------------------------------------------
/* Stmt */

//## int Stmt.getBuild();
static KMETHOD Stmt_getBuild(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].asStmt->build);
}

//## void Stmt.setBuild(int buildid);
static KMETHOD Stmt_setBuild(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmtVar *stmt = (kStmtVar *) sfp[0].asStmt;
	stmt->build = sfp[1].intValue;
}

//## Block Stmt.getBlock(symbol key, Block def);
static KMETHOD Stmt_getBlock(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	KReturn(SUGAR kStmt_getBlock(kctx, sfp[0].asStmt, NULL/*DefaultNameSpace*/, key, sfp[2].asBlock));
}

//## Token Stmt.getToken(symbol key, Token def);
static KMETHOD Stmt_getToken(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt   = sfp[0].asStmt;
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	kToken *def   = sfp[2].asToken;
	KReturn(SUGAR kStmt_getToken(kctx, stmt, key, def));
}

//## Expr Stmt.getExpr(symbol key, Expr def);
static KMETHOD Stmt_getExpr(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt   = sfp[0].asStmt;
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	kExpr *def    = sfp[2].asExpr;
	KReturn(SUGAR kStmt_getExpr(kctx, stmt, key, def));
}

//## void Stmt.setType(int build);
static KMETHOD Stmt_setType(KonohaContext *kctx, KonohaStack *sfp)
{
	Stmt_typed(sfp[0].asStmt, sfp[1].intValue);
	KReturnVoid();
}

//## boolean Stmt.TypeCheckExpr(symbol key, Gamma gma, cid typeId);
static KMETHOD Stmt_TypeCheckExpr(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	KReturnUnboxValue(SUGAR kStmt_TypeCheckByName(kctx, sfp[0].asStmt, key, sfp[2].asGamma, (ktype_t)sfp[3].intValue, 0));
}

//## boolean Stmt.TypeCheckExpr(symbol key, Gamma gma, cid typeId, int pol);
static KMETHOD Stmt_TypeCheckExprPol(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	KReturnUnboxValue(SUGAR kStmt_TypeCheckByName(kctx, sfp[0].asStmt, key, sfp[2].asGamma, (ktype_t)sfp[3].intValue, (int)sfp[4].intValue));
}

//## Expr Stmt.printError(String msg);
static KMETHOD Stmt_printMessage2rintError(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt   *stmt  = sfp[0].asStmt;
	kString *msg   = sfp[1].asString;
	SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, "%s", S_text(msg));
	KReturn(K_NULLEXPR);
}

//## Expr Stmt.newExpr(Token[] tokenList, int s, int e);
static KMETHOD Stmt_newExpr(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt  = sfp[0].asStmt;
	kArray *tokenList  = sfp[1].asArray;
	int s = sfp[2].intValue, e = sfp[3].intValue;
	KReturn(SUGAR kStmt_parseExpr(kctx, stmt, tokenList, s, e, NULL));
}

//## Expr Stmt.newUntypedCallStyleExpr(Token token, Expr expr1);
static KMETHOD Stmt_newUntypedCallStyleExpr2(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt   = sfp[0].asStmt;
	kToken *token = sfp[1].asToken;
	kExpr *expr1  = sfp[2].asExpr;
	SugarSyntax *syn = SYN_(Stmt_ns(stmt), KW_ExprMethodCall);
	KReturn(SUGAR new_UntypedCallStyleExpr(kctx, syn, 2, token, expr1));
}

//## Expr Stmt.newUntypedCallStyleExpr(Token token, Expr expr1, Expr expr2);
static KMETHOD Stmt_newUntypedCallStyleExpr3(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt   = sfp[0].asStmt;
	kToken *token = sfp[1].asToken;
	kExpr *expr1  = sfp[2].asExpr;
	kExpr *expr2  = sfp[3].asExpr;
	SugarSyntax *syn = SYN_(Stmt_ns(stmt), KW_ExprMethodCall);
	KReturn(SUGAR new_UntypedCallStyleExpr(kctx, syn, 3, token, expr1, expr2));
}

//## Expr Stmt.newUntypedCallStyleExpr(Token token, Expr expr1, Expr expr2, Expr expr3);
static KMETHOD Stmt_newUntypedCallStyleExpr4(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt   = sfp[0].asStmt;
	kToken *token = sfp[1].asToken;
	kExpr *expr1  = sfp[2].asExpr;
	kExpr *expr2  = sfp[3].asExpr;
	kExpr *expr3  = sfp[4].asExpr;
	SugarSyntax *syn = SYN_(Stmt_ns(stmt), KW_ExprMethodCall);
	KReturn(SUGAR new_UntypedCallStyleExpr(kctx, syn, 4, token, expr1, expr2, expr3));
}

//## Expr Stmt.newUntypedCallStyleExpr(Token token, Expr expr1, Expr expr2, Expr expr3, Expr expr4);
static KMETHOD Stmt_newUntypedCallStyleExpr5(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt   = sfp[0].asStmt;
	kToken *token = sfp[1].asToken;
	kExpr *expr1  = sfp[2].asExpr;
	kExpr *expr2  = sfp[3].asExpr;
	kExpr *expr3  = sfp[4].asExpr;
	kExpr *expr4  = sfp[5].asExpr;
	SugarSyntax *syn = SYN_(Stmt_ns(stmt), KW_ExprMethodCall);
	KReturn(SUGAR new_UntypedCallStyleExpr(kctx, syn, 5, token, expr1, expr2, expr3, expr4));
}

//## Expr Stmt.newUntypedCallStyleExpr(Token token, Expr expr1, Expr expr2, Expr expr3, Expr expr4, Expr expr5);
static KMETHOD Stmt_newUntypedCallStyleExpr6(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt   = sfp[0].asStmt;
	kToken *token = sfp[1].asToken;
	kExpr *expr1  = sfp[2].asExpr;
	kExpr *expr2  = sfp[3].asExpr;
	kExpr *expr3  = sfp[4].asExpr;
	kExpr *expr4  = sfp[5].asExpr;
	kExpr *expr5  = sfp[6].asExpr;
	SugarSyntax *syn = SYN_(Stmt_ns(stmt), KW_ExprMethodCall);
	KReturn(SUGAR new_UntypedCallStyleExpr(kctx, syn, 6, token, expr1, expr2, expr3, expr4, expr5));
}

//## Expr Stmt.newTypedCallExpr(Gamma gma, cid typeId, symbol methodName, Expr firstExpr);
static KMETHOD Stmt_newTypedCallExpr1(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt          = sfp[0].asStmt;
	kGamma *gma          = sfp[1].asGamma;
	ktype_t cid          = (ktype_t)sfp[2].intValue;
	ksymbol_t methodName = (ksymbol_t)sfp[3].intValue;
	kExpr *firstExpr     = sfp[4].asExpr;
	kMethod *method = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, Stmt_ns(stmt), cid, methodName, 1);
	if(method == NULL) {
		KReturn(KNULL(Expr));
	}
	KReturn(SUGAR new_TypedCallExpr(kctx, stmt, gma, cid, method, 1, firstExpr));
}

//## Expr Stmt.newTypedCallExpr(Gamma gma, cid typeId, String methodName, Expr firstExpr, Expr secondExpr);
static KMETHOD Stmt_newTypedCallExpr2(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt          = sfp[0].asStmt;
	kGamma *gma          = sfp[1].asGamma;
	ktype_t cid          = (ktype_t)sfp[2].intValue;
	ksymbol_t methodName = (ksymbol_t)sfp[3].intValue;
	kExpr *firstExpr     = sfp[4].asExpr;
	kExpr *secondExpr    = sfp[5].asExpr;
	kMethod *method = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, Stmt_ns(stmt), cid, methodName, 2);
	if(method == NULL) {
		KReturn(KNULL(Expr));
	}
	KReturn(SUGAR new_TypedCallExpr(kctx, stmt, gma, cid, method, 2, firstExpr, secondExpr));
}

//## Expr Stmt.rightJoinExpr(Expr expr, Token[] tokenList, int currentIdx, int endIdx);
static KMETHOD Stmt_rightJoinExpr(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt       = sfp[0].asStmt;
	kExpr *expr       = sfp[1].asExpr;
	kArray *tokenList = sfp[2].asArray;
	int currentIdx    = sfp[3].intValue;
	int endIdx        = sfp[4].intValue;
	KReturn(SUGAR kStmt_rightJoinExpr(kctx, stmt, expr, tokenList, currentIdx, endIdx));
}

//## Token[] Stmt.getTokenList(symbol keyword, Token[] def);
static KMETHOD Stmt_getTokenList(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt    *stmt    = sfp[0].asStmt;
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	kArray   *def     = sfp[2].asArray;
	kTokenArray *tokenList = (kTokenArray *)kStmt_getObject(kctx, stmt, keyword, def);
	kTokenArray *ret;
	if(!IS_Array(tokenList)) {
		ret = new_(TokenArray, 0, OnStack);
		KLIB kArray_add(kctx, ret, tokenList);
	}
	else {
		ret = tokenList;
	}
	KReturn(ret);
}

//## void Stmt.done();
static KMETHOD Stmt_done(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt_done(kctx, sfp[0].asStmt);
	KReturnVoid();
}

//## void Stmt.setExpr(symbol key, Expr expr);
//## void Stmt.setBlock(symbol key, Block block);
static KMETHOD Stmt_setObject(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt   = sfp[0].asStmt;
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	kObject *obj  = sfp[2].asObject;
	kStmt_setObject(kctx, stmt, key, obj);
	KReturnVoid();
}

//## boolean Stmt.declType(Gamma gma, cid typeId, Expr declExpr);
static KMETHOD Stmt_declType(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt     = sfp[0].asStmt;
	kGamma *gma     = sfp[1].asGamma;
	ktype_t cid     = (ktype_t)sfp[2].intValue;
	kExpr *declExpr = sfp[3].asExpr;
	KReturnUnboxValue(SUGAR kStmt_declType(kctx, stmt, gma, cid, declExpr, NULL, NULL, &stmt));
}

//## Block Stmt.newBlock(String macro);
static KMETHOD Stmt_newBlock(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt    *stmt = sfp[0].asStmt;
	kString *macro = sfp[1].asString;
	TokenSeq source = {Stmt_ns(stmt), GetSugarContext(kctx)->preparedTokenList/*TODO: set appropriate tokenList to TokenSeq*/};
	TokenSeq_push(kctx, source);
	SUGAR TokenSeq_tokenize(kctx, &source, S_text(macro), 0);
	kBlock *bk = SUGAR new_kBlock(kctx, stmt, NULL, &source);
	TokenSeq_pop(kctx, source);
	KReturn(bk);
}


// --------------------------------------------------------------------------
/* Expr */

//## Token Expr.getTermToken();
static KMETHOD Expr_getTermToken(KonohaContext *kctx, KonohaStack *sfp)
{
	kExpr *expr = sfp[0].asExpr;
	KReturn(expr->termToken);
}

//## Expr Expr.setConstValue(Object value);
static KMETHOD Expr_setConstValue(KonohaContext *kctx, KonohaStack *sfp)
{
	kExpr *expr = sfp[0].asExpr;
	KonohaClass *ct = O_ct(sfp[1].asObject);
	if(CT_isUnbox(ct)) {
		KReturn(SUGAR kExpr_setUnboxConstValue(kctx, expr, ct->typeId, sfp[1].unboxValue));
	}
	KReturn(SUGAR kExpr_setConstValue(kctx, expr, ct->typeId, sfp[1].asObject));
}

//## Expr Expr.setVariable(Gamma gma, int build, cid typeid, int index);
static KMETHOD Expr_setVariable(KonohaContext *kctx, KonohaStack *sfp)
{
	kExpr *expr    = sfp[0].asExpr;
	kGamma *gma    = sfp[1].asGamma;
	kexpr_t build  = (kexpr_t)sfp[2].intValue;
	ktype_t cid    = (ktype_t)sfp[3].intValue;
	intptr_t index = sfp[4].unboxValue;
	KReturn(SUGAR kExpr_setVariable(kctx, expr, gma, build, cid, index));
}

//## Expr Expr.new(Gamma gma, int build, cid typeid, int index);
static KMETHOD Expr_newVariableExpr(KonohaContext *kctx, KonohaStack *sfp)
{
	kGamma *gma    = sfp[1].asGamma;
	kexpr_t build  = (kexpr_t)sfp[2].intValue;
	ktype_t cid    = (ktype_t)sfp[3].intValue;
	intptr_t index = sfp[4].unboxValue;
	KReturn(new_VariableExpr(kctx, gma, build, cid, index));
}

//## Expr Expr.new(Object value);
static KMETHOD Expr_new(KonohaContext *kctx, KonohaStack *sfp)
{
	KonohaClass *ct = O_ct(sfp[1].asObject);
	if(CT_isUnbox(ct)) {
		KReturn(new_UnboxConstValueExpr(kctx, ct->typeId, sfp[1].unboxValue));
	}
	KReturn(new_ConstValueExpr(kctx, ct->typeId, sfp[1].asObject));
}

//## void Expr.setType(int build, cid typeid);
static KMETHOD Expr_setType(KonohaContext *kctx, KonohaStack *sfp)
{
	kExprVar *expr = (kExprVar *)sfp[0].asExpr;
	kexpr_t build  = (kexpr_t)sfp[1].intValue;
	ktype_t cid    = (ktype_t)sfp[2].intValue;
	expr->build = build;
	expr->ty = cid;
	KReturnVoid();
}


// --------------------------------------------------------------------------
// AST Method

//## boolean Blook.TypeCheckAll(Gamma gma);
static KMETHOD Block_TypeCheckAll(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(SUGAR kBlock_TypeCheckAll(kctx, sfp[0].asBlock, sfp[1].asGamma));
}

//## int Gamma.declareLocalVariable(cid typeId, symbol keyword);
static KMETHOD Gamma_declareLocalVariable(KonohaContext *kctx, KonohaStack *sfp)
{
	kGamma *gma       = sfp[0].asGamma;
	ktype_t cid       = (ktype_t)sfp[1].intValue;
	ksymbol_t keyword = (ksymbol_t)sfp[2].intValue;
	KReturnUnboxValue(SUGAR kGamma_declareLocalVariable(kctx, gma, cid, keyword));
}

//static SugarSyntax* get_syntax(KonohaContext *kctx, kNameSpace *ns, kString *key)
//{
//	USING_SUGAR;
//	symbol_t kw = KW_s(key);
//	if(kw == SYM_NONAME) {
//		kreportf(CritTag, "undefined keyword: %s", S_text(key));
//	}
//	SugarSyntax *syn = SYN_(ks, kw);
//	if(syn == NULL) {
//		kreportf(CritTag, "undefined syntax: %s", S_text(key));
//	}
//	return syn;
//}

////## Expr Token.printSyntaxError();
//static KMETHOD Token_printSyntaxError(KonohaContext *kctx, KonohaStack *sfp)
//{
//	USING_SUGAR;
//	kToken *tk  = sfp[0].asToken;
//	if(IS_String(tk->text)) {
//		SUGAR p(kctx, ErrTag, tk->uline, tk->lpos, "syntax error: %s", S_text(tk->text));
//	}
//	else {
//		SUGAR p(kctx, ErrTag, tk->uline, tk->lpos, "syntax error");
//	}
//	KReturn(K_NULLEXPR);
//}

////## Expr Stmt.newBlock(Token[] tokenList, int s, int e);
//static KMETHOD Stmt_newBlock(KonohaContext *kctx, KonohaStack *sfp)
//{
//	USING_SUGAR;
//	kStmt *stmt  = sfp[0].asStmt;
//	kArray *tokenList  = sfp[1].asArray;
//	int s = sfp[2].intValue, e = sfp[3].intValue;
//	KReturn(SUGAR new_kBlock(kctx, Stmt_ns(stmt), stmt, tokenList, s, e, ';'));
//}

////## Expr Stmt.newMethodCallExpr(Token key, Token self);
//static KMETHOD Stmt_newMethodCallExpr(KonohaContext *kctx, KonohaStack *sfp)
//{
//	USING_SUGAR;
//	kStmt *stmt  = sfp[0].asStmt;
//	kToken *tk   = sfp[1].asToken;
//	assert(tk->keyword != 0);
//	kExprVar *expr = /*G*/new_(ExprVar, SYN_(Stmt_ns(stmt), tk->keyword));
//	KFieldSet(expr, expr->tk, tk);
//	KFieldSet(expr, expr->cons, new_(Array, 8));
//	KReturn(expr);
//}

////## Expr Stmt.addExprParam(Token tk);
//static KMETHOD Stmt_addExprParam(KonohaContext *kctx, KonohaStack *sfp)
//{
//	USING_SUGAR;
//	kStmt *stmt  = sfp[0].asStmt;
//	kExpr *expr  = sfp[1].asExpr;
//	kToken *tk     = sfp[2].asToken;
//	if(tk->tt != KW_ParenthesisGroup || tk->tt != KW_BracketGroup) {
//		SUGAR p(kctx, WarnTag, tk->uline, tk->lpos, "not parameter token");
//		kObject_set(NullObject, expr, 1);
//	}
//	if(IS_NOTNULL(expr)) {
//		assert(IS_Array(tk->subTokenList));
//		expr = SUGAR kStmt_addExprParam(kctx, stmt, expr, tk->subTokenList, 0, kArray_size(tk->subTokenList), 1/*allowEmpty*/);
//	}
//	KReturn(expr);
//}

//## Expr Expr.addExpr(Expr expr, Expr o);
static KMETHOD Expr_addExpr(KonohaContext *kctx, KonohaStack *sfp)
{
	kExpr *expr  = sfp[0].asExpr;
	kExpr *o     = sfp[1].asExpr;
	if(IS_NULL(o) && IS_Array(expr->cons)) {
		kObject_set(NullObject, expr, 1);
	}
	if(IS_NOTNULL(expr)) {
		KLIB kArray_add(kctx, expr->cons, o);
	}
	KReturn(expr);
}

// --------------------------------------------------------------------------

static kbool_t RENAMEME_initNameSpace(KonohaContext *kctx, kNameSpace *packageNS, kNameSpace *ns, KTraceInfo *trace);

static kbool_t sugar_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KonohaClass *cSymbol = loadSymbolClass(kctx, ns, trace);
	KonohaClass *ccid = loadcidClass(kctx, ns, trace);
	KDEFINE_INT_CONST ClassData[] = {   // add Array as available
		{"Token", VirtualType_KonohaClass, (uintptr_t)CT_Token},
		{"Stmt", VirtualType_KonohaClass,  (uintptr_t)CT_Stmt},
		{"Expr", VirtualType_KonohaClass,  (uintptr_t)CT_Expr},
		{"Block", VirtualType_KonohaClass, (uintptr_t)CT_Block},
		{"Gamma", VirtualType_KonohaClass, (uintptr_t)CT_Gamma},
		{"NameSpace", VirtualType_KonohaClass, (uintptr_t)CT_NameSpace},
		{"symbol", VirtualType_KonohaClass, (uintptr_t)cSymbol},
		{NULL},
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KonohaConst_(ClassData), 0);

	int FN_buildid = FN_("buildid"), FN_key = FN_("key"), FN_defval = FN_("defval");
	int FN_typeid = FN_("typeid"), FN_gma = FN_("gma"), FN_pol = FN_("pol");
	int FN_msg = FN_("msg");
	int FN_x = FN_("x");
	int FN_tokenList = FN_("tokens"), FN_s = FN_("s"), FN_e = FN_("e");
	int FN_expr = FN_("expr");

	/* Array[String] */
	kparamtype_t P_StringArray[] = {{TY_String}};
	int TY_StringArray = (KLIB KonohaClass_Generics(kctx, CT_Array, TY_void, 1, P_StringArray))->typeId;

	ktype_t TY_symbol = cSymbol->typeId;
	ktype_t TY_cid = ccid->typeId;

	KDEFINE_METHOD MethodData[] = {
		/* Token */
		_Public, _F(Token_new), TY_Token, TY_Token, MN_("new"), 1, TY_symbol, FN_key,
		_Public, _F(Token_setUnresolvedTokenType),  TY_void, TY_Token, MN_("setUnresolvedTokenType"),  1, TY_symbol, FN_x,
		_Public, _F(Token_setText),  TY_void, TY_Token, MN_("setText"),  1, TY_String, FN_x,
		_Public, _F(Token_setSubArray), TY_void, TY_Token, MN_("setSubArray"), 1, TY_StringArray, FN_x,
//		_Public, _F(Token_isTypeName), TY_boolean, TY_Token, MN_("isTypeName"), 0,
		_Public, _F(Token_isParenthesis), TY_boolean, TY_Token, MN_("isParenthesis"), 0,
		_Public, _F(Token_getText), TY_String, TY_Token, MN_("getText"), 0,
		_Public, _F(Token_isSymbol), TY_boolean, TY_Token, MN_("isSymbol"), 0,
		_Public, _F(Token_newUntypedExpr), TY_Expr, TY_Token, MN_("newUntypedExpr"), 0,
		_Public, _F(Token_isBeforeWhiteSpace), TY_boolean, TY_Token, MN_("isBeforeWhiteSpace"), 0,

		/* Stmt */
		_Public, _F(Stmt_getBuild), TY_int, TY_Stmt,  MN_("getBuild"), 0,
		_Public, _F(Stmt_setBuild), TY_void, TY_Stmt, MN_("setBuild"), 1, TY_int, FN_buildid,
		_Public, _F(Stmt_getBlock), TY_Block, TY_Stmt, MN_("getBlock"), 2, TY_symbol, FN_key, TY_Block, FN_defval,
		_Public, _F(Stmt_getToken), TY_Token, TY_Stmt, MN_("getToken"), 2, TY_symbol, FN_key, TY_Token, FN_defval,
		_Public, _F(Stmt_getExpr), TY_Expr, TY_Stmt, MN_("getExpr"), 2, TY_symbol, FN_key, TY_Expr, FN_defval,
		_Public, _F(Stmt_TypeCheckExpr), TY_boolean, TY_Stmt, MN_("TypeCheckExpr"), 3, TY_symbol, FN_key, TY_Gamma, FN_gma, TY_cid, FN_typeid,
		_Public, _F(Stmt_TypeCheckExprPol), TY_boolean, TY_Stmt, MN_("TypeCheckExpr"), 4, TY_symbol, FN_key, TY_Gamma, FN_gma, TY_cid, FN_typeid, TY_int, FN_pol,
		_Public, _F(Stmt_printMessage2rintError), TY_Expr, TY_Stmt, MN_("printError"), 1, TY_String, FN_msg,

		_Public, _F(Stmt_newExpr), TY_Expr, TY_Stmt, MN_("newExpr"), 3, TY_TokenArray, FN_tokenList, TY_int, FN_s, TY_int, FN_e,
		_Public, _F(Stmt_setType), TY_void, TY_Stmt, MN_("setType"), 1, TY_int, FN_x,
//		_Public, _F(kStmt_printMessage2arsedExpr), TY_Expr, TY_Stmt, MN_("parseExpr"), 3, TY_TokenArray, FN_tokenList, TY_int, FN_s, TY_int, FN_e,
		_Public, _F(Stmt_newUntypedCallStyleExpr2), TY_Expr, TY_Stmt, MN_("newUntypedCallStyleExpr"), 2, TY_Token, FN_("token"), TY_Expr, FN_("expr1"),
		_Public, _F(Stmt_newUntypedCallStyleExpr3), TY_Expr, TY_Stmt, MN_("newUntypedCallStyleExpr"), 3, TY_Token, FN_("token"), TY_Expr, FN_("expr1"), TY_Expr, FN_("expr2"),
		_Public, _F(Stmt_newUntypedCallStyleExpr4), TY_Expr, TY_Stmt, MN_("newUntypedCallStyleExpr"), 4, TY_Token, FN_("token"), TY_Expr, FN_("expr1"), TY_Expr, FN_("expr2"), TY_Expr, FN_("expr3"),
		_Public, _F(Stmt_newUntypedCallStyleExpr5), TY_Expr, TY_Stmt, MN_("newUntypedCallStyleExpr"), 5, TY_Token, FN_("token"), TY_Expr, FN_("expr1"), TY_Expr, FN_("expr2"), TY_Expr, FN_("expr3"), TY_Expr, FN_("expr4"),
		_Public, _F(Stmt_newUntypedCallStyleExpr6), TY_Expr, TY_Stmt, MN_("newUntypedCallStyleExpr"), 6, TY_Token, FN_("token"), TY_Expr, FN_("expr1"), TY_Expr, FN_("expr2"), TY_Expr, FN_("expr3"), TY_Expr, FN_("expr4"), TY_Expr, FN_("expr5"),
		_Public, _F(Stmt_newTypedCallExpr1), TY_Expr, TY_Stmt, MN_("newTypedCallExpr"), 4, TY_Gamma, FN_gma, TY_cid, FN_typeid, TY_symbol, FN_("methodName"), TY_Expr, FN_("firstExpr"),
		_Public, _F(Stmt_newTypedCallExpr2), TY_Expr, TY_Stmt, MN_("newTypedCallExpr"), 5, TY_Gamma, FN_gma, TY_cid, FN_typeid, TY_symbol, FN_("methodName"), TY_Expr, FN_("firstExpr"), TY_Expr, FN_("secondExpr"),
		_Public, _F(Stmt_rightJoinExpr), TY_Expr, TY_Stmt, MN_("rightJoinExpr"), 4, TY_Expr, FN_expr, TY_TokenArray, FN_tokenList, TY_int, FN_s, TY_int, FN_e,
		_Public, _F(Stmt_getTokenList), TY_TokenArray, TY_Stmt, MN_("getTokenList"), 2, TY_symbol, FN_key, TY_TokenArray, FN_defval,
		_Public, _F(Stmt_done), TY_void, TY_Stmt, MN_("done"), 0,
		_Public, _F(Stmt_setObject), TY_void, TY_Stmt, MN_("setExpr"), 2, TY_symbol, FN_key, TY_Expr, FN_expr,
		_Public, _F(Stmt_setObject), TY_void, TY_Stmt, MN_("setBlock"), 2, TY_symbol, FN_key, TY_Block, FN_("block"),
		_Public, _F(Stmt_declType), TY_boolean, TY_Stmt, MN_("declType"), 3, TY_Gamma, FN_gma, TY_cid, FN_typeid, TY_Expr, FN_("declExpr"),
		_Public, _F(Stmt_newBlock), TY_Block, TY_Stmt, MN_("newBlock"), 1, TY_String, FN_("macro"),

		/* Expr */
		_Public, _F(Expr_getTermToken), TY_Token, TY_Expr, MN_("getTermToken"), 0,
		_Public, _F(Expr_setConstValue), TY_Expr, TY_Expr, MN_("setConstValue"), 1, TY_Object, FN_("value") | FN_COERCION,
		_Public, _F(Expr_setVariable), TY_Expr, TY_Expr, MN_("setVariable"), 4, TY_Gamma, FN_gma, TY_int, FN_buildid, TY_cid, FN_typeid, TY_int, FN_("index"),
		_Public, _F(Expr_newVariableExpr), TY_Expr, TY_Expr, MN_("new"), 4, TY_Gamma, FN_gma, TY_int, FN_buildid, TY_cid, FN_typeid, TY_int, FN_("index"),
		_Public, _F(Expr_new), TY_Expr, TY_Expr, MN_("new"), 1, TY_Object, FN_("value") | FN_COERCION,
		_Public, _F(Expr_setType), TY_void, TY_Expr, MN_("setType"), 2, TY_int, FN_buildid, TY_cid, FN_typeid,
		_Public, _F(Expr_addExpr), TY_Expr, TY_Expr, MN_("addExpr"), 1, TY_Expr, FN_expr,

		/* Block */
		_Public, _F(Block_TypeCheckAll), TY_boolean, TY_Block, MN_("TypeCheckAll"), 1, TY_Gamma, FN_gma,

		/* Gamma */
		_Public, _F(Gamma_declareLocalVariable), TY_int, TY_Gamma, MN_("declareLocalVariable"), 2, TY_cid, FN_typeid, TY_symbol, FN_key,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	LoadNameSpaceMethodData(kctx, ns, TY_symbol, trace);
	RENAMEME_initNameSpace(kctx, ns, ns, trace);
	return true;
}

static kbool_t sugar_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

static kbool_t isSubKeyword(KonohaContext *kctx, kArray *tokenList, int beginIdx, int endIdx)
{
	if(beginIdx+1 < endIdx && tokenList->TokenItems[beginIdx+1]->resolvedSyntaxInfo->keyword == KW_TextPattern) {
		const char *t = S_text(tokenList->TokenItems[beginIdx+1]->text);
		if(isalpha(t[0]) || t[0] < 0 /* multibytes char */) {
			return 1;
		}
	}
	return 0;
}

static SugarSyntaxVar *kNameSpace_guessSyntaxFromTokenList(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList)
{
	int beginIdx = 0, endIdx = kArray_size(tokenList);
	if(beginIdx < endIdx) {
		ksymbol_t keyword = tokenList->TokenItems[beginIdx]->resolvedSyntaxInfo->keyword;
		if(keyword == KW_TextPattern) {
			ksymbol_t kw;
			if(isSubKeyword(kctx, tokenList, beginIdx, endIdx)) {
				char buf[256];
				PLATAPI snprintf_i(buf, sizeof(buf), "%s_%s", S_text(tokenList->TokenItems[beginIdx]->text), S_text(tokenList->TokenItems[beginIdx+1]->text));
				kw = ksymbolA((const char *)buf, strlen(buf), SYM_NEWID);
			}
			else {
				kw = ksymbolA(S_text(tokenList->TokenItems[beginIdx]->text), S_size(tokenList->TokenItems[beginIdx]->text), SYM_NEWID);
			}
			return (SugarSyntaxVar *)NEWSYN_(ns, kw);
		}
		else if(keyword == KW_DOLLAR) { // $TokenPattern
			char buf[256];
			PLATAPI snprintf_i(buf, sizeof(buf), "$%s", S_text(tokenList->TokenItems[beginIdx+1]->text));
			ksymbol_t kw = ksymbolA((const char *)buf, strlen(buf), SYM_NEWID);
			return (SugarSyntaxVar *)NEWSYN_(ns, kw);
		}
	}
	return NULL;
}

// Copied from namespace.h.
static void kNameSpace_AppendArrayRef(KonohaContext *kctx, kNameSpace *ns, const kArray **arrayRef, kObject *o)
{
	if(arrayRef[0] == NULL) {
		((kArray**)arrayRef)[0] = new_(Array, 0, ns->NameSpaceConstList);
	}
	DBG_ASSERT(IS_Array(arrayRef[0]));
	KLIB kArray_add(kctx, arrayRef[0], o);
}

#define kToken_isFirstPattern(tk)   (KW_isPATTERN(tk->resolvedSymbol) && tk->stmtEntryKey != KW_ExprPattern)
static KMETHOD Statement_syntax(KonohaContext *kctx, KonohaStack *sfp)
{
	kbool_t r = 0;
	VAR_Statement(stmt, gma);
	kTokenArray *tokenList = (kTokenArray *)kStmt_getObject(kctx, stmt, KW_TokenPattern, NULL);
	if(tokenList != NULL) {
		if(!IS_Array(tokenList)) {
			// create tokenList from a Token
			kTokenArray *a = new_(TokenArray, 0, OnGcStack);
			KLIB kArray_add(kctx, a, tokenList);
			tokenList = a;
		}
		DBG_ASSERT(IS_Array(tokenList));
		kNameSpace *ns = Stmt_ns(stmt);
		SugarSyntaxVar *syn = kNameSpace_guessSyntaxFromTokenList(kctx, ns, tokenList);
		if(syn != NULL) {
			if(syn->syntaxPatternListNULL_OnList != NULL) {
				SUGAR kStmt_printMessage2(kctx, stmt, NULL, InfoTag, "overriding syntax: %s%s", PSYM_t(syn->keyword));
			}
			else {
				syn->syntaxPatternListNULL_OnList = new_(TokenArray, 0, ns->NameSpaceConstList);
			}
			TokenSeq tokens = {ns, tokenList, 0, kArray_size(tokenList)};
			// Referred to kNameSpace_parseSyntaxPattern in ast.h.
			kArray *patternList = syn->syntaxPatternListNULL_OnList;
			size_t firstPatternIdx = kArray_size(patternList);
			SUGAR kArray_addSyntaxRule(kctx, patternList, &tokens);
			if(firstPatternIdx < kArray_size(patternList)) {
				kToken *firstPattern = patternList->TokenItems[firstPatternIdx];
				if(kToken_isFirstPattern(firstPattern)) {
					kNameSpace_AppendArrayRef(kctx, ns, &((kNameSpaceVar *)ns)->stmtPatternListNULL_OnList, UPCAST(firstPattern));
				}
			}
			r = 1;
		}
		kStmt_done(kctx, stmt);
	}
	KReturnUnboxValue(r);
}

static kbool_t RENAMEME_initNameSpace(KonohaContext *kctx, kNameSpace *packageNS, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_INT_CONST IntData[] = {
#define DEFINE_KEYWORD(KW) {#KW, TY_int, KW}
		DEFINE_KEYWORD(KW_ExprPattern),
		DEFINE_KEYWORD(KW_SymbolPattern),
		DEFINE_KEYWORD(KW_TextPattern),
		DEFINE_KEYWORD(KW_NumberPattern),
		DEFINE_KEYWORD(KW_TypePattern),
		DEFINE_KEYWORD(KW_ParenthesisGroup),
		DEFINE_KEYWORD(KW_BracketGroup),
		DEFINE_KEYWORD(KW_BraceGroup),
		DEFINE_KEYWORD(KW_BlockPattern),
		DEFINE_KEYWORD(KW_ParamPattern),
		DEFINE_KEYWORD(KW_TokenPattern),
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
//		DEFINE_KEYWORD(TEXPR_BOX),
//		DEFINE_KEYWORD(TEXPR_UNBOX),
		DEFINE_KEYWORD(TEXPR_CALL),
		DEFINE_KEYWORD(TEXPR_AND),
		DEFINE_KEYWORD(TEXPR_OR),
		DEFINE_KEYWORD(TEXPR_LET),
		DEFINE_KEYWORD(TEXPR_STACKTOP),
		DEFINE_KEYWORD(TypeCheckPolicy_NOCHECK),
		DEFINE_KEYWORD(TypeCheckPolicy_ALLOWVOID),
		DEFINE_KEYWORD(TypeCheckPolicy_COERCION),
		DEFINE_KEYWORD(TypeCheckPolicy_CONST),
#undef DEFINE_KEYWORD
		{NULL},
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KonohaConst_(IntData), trace);
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("syntax"), 0, "\"syntax\" $Token*", 0, 0, NULL, NULL, Statement_syntax, NULL, NULL, },
		{ KW_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}

KDEFINE_PACKAGE* sugar_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "sugar", "1.0");
	d.PackupNameSpace    = sugar_PackupNameSpace;
	d.ExportNameSpace   = sugar_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
