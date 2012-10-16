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

//## symbol String.toSymbol(String keyword);
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
	KonohaClass *cSymbol = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defSymbol, trace);
	KDEFINE_METHOD MethodData[] = {
		_Public|_Coercion|_Const, _F(String_toSymbol), TY_boolean, TY_String, MN_to(cSymbol->typeId), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return cSymbol;
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
static KMETHOD NameSpace_definedSyntax(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	SugarSyntax* syn = SYN_(sfp[0].asNameSpace, keyword);
	KReturnUnboxValue(syn != NULL);
}

//## boolean NameSpace.definedLiteral(symbol keyword);
static KMETHOD NameSpace_definedLiteral(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	SugarSyntax* syn = SYN_(sfp[0].asNameSpace, keyword);
	KReturnUnboxValue(SugarSyntax_hasSugarFunc(syn, SugarFunc_TokenFunc));
}

//## boolean NameSpace.definedStatement(symbol keyword);
static KMETHOD NameSpace_definedStatement(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	SugarSyntax* syn = SYN_(sfp[0].asNameSpace, keyword);
	KReturnUnboxValue(SugarSyntax_hasSyntaxPatternList(syn) && SugarSyntax_hasSugarFunc(syn, SugarFunc_Statement));
}

//## boolean NameSpace.definedExpression(symbol keyword);
static KMETHOD NameSpace_definedExpression(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	SugarSyntax* syn = SYN_(sfp[0].asNameSpace, keyword);
	KReturnUnboxValue(syn != NULL && (syn->precedence_op2 > 0 || syn->precedence_op1 > 0));
}

//## boolean NameSpace.definedBinaryOperator(symbol keyword);
static KMETHOD NameSpace_definedBinaryOperator(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	SugarSyntax* syn = SYN_(sfp[0].asNameSpace, keyword);
	KReturnUnboxValue(syn != NULL && (syn->precedence_op2 > 0));
}

static void loadNameSpaceMethodData(KonohaContext *kctx, kNameSpace *ns, int TY_symbol)
{
	int FN_keyword = SYM_("keyword");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Im, _F(NameSpace_definedSyntax), TY_boolean, TY_NameSpace, MN_("definedSyntax"), 1, TY_symbol, FN_keyword,
		_Public|_Im, _F(NameSpace_definedLiteral), TY_boolean, TY_NameSpace, MN_("definedLiteral"), 1, TY_symbol, FN_keyword,
		_Public|_Im, _F(NameSpace_definedStatement), TY_boolean, TY_NameSpace, MN_("definedStatement"), 1, TY_symbol, FN_keyword,
		_Public|_Im, _F(NameSpace_definedExpression), TY_boolean, TY_NameSpace, MN_("definedExpression"), 1, TY_symbol, FN_keyword,
		_Public|_Im, _F(NameSpace_definedBinaryOperator), TY_boolean, TY_NameSpace, MN_("definedBinaryOperator"), 1, TY_symbol, FN_keyword,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
}

/* Token */

//## void Token.setKeyword(String keywork);
static KMETHOD Token_setUnresolvedTokenType(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *) sfp[0].asToken;
	kString *key = sfp[1].asString;
	ksymbol_t keyword = ksymbolA(S_text(key), S_size(key), _NEWID);
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
//
////## boolean Token.isParenthesis();
//static KMETHOD Token_isParenthesis(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KReturnUnboxValue(sfp[0].asToken->keyword == KW_ParenthesisGroup);
//}

//## String Token.getText();
static KMETHOD Token_getText(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *)sfp[0].asToken;
	KReturn(tk->text);
}

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

//## Block Stmt.getBlock(String key, Block def);
static KMETHOD Stmt_getBlock(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *key = sfp[1].asString;
	KReturn(SUGAR kStmt_getBlock(kctx, sfp[0].asStmt, NULL/*DefaultNameSpace*/, ksymbolA(S_text(key), S_size(key), _NEWID), sfp[2].asBlock));
}

//## void Stmt.setType(int build);
static KMETHOD Stmt_setType(KonohaContext *kctx, KonohaStack *sfp)
{
	Stmt_typed(sfp[0].asStmt, sfp[1].intValue);
	KReturnVoid();
}

//## boolean Stmt.tyCheckExpr(String key, Gamma gma, int typeid, int pol);
static KMETHOD kStmt_tyCheckByName(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *key = sfp[1].asString;
	KReturnUnboxValue(SUGAR kStmt_tyCheckByName(kctx, sfp[0].asStmt, ksymbolA(S_text(key), S_size(key), _NEWID), sfp[2].asGamma, (ktype_t)sfp[3].intValue, (int)sfp[4].intValue));
}

//## boolean Blook.tyCheckAll(Gamma gma);
static KMETHOD kBlock_tyCheckAll(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(SUGAR kBlock_tyCheckAll(kctx, sfp[0].asBlock, sfp[1].asGamma));
}

// --------------------------------------------------------------------------


//## Token Expr.getTermToken();
static KMETHOD Expr_getTermToken(KonohaContext *kctx, KonohaStack *sfp)
{
	kExpr *expr = sfp[0].asExpr;
	KReturn(expr->termToken);
}


//## Expr Expr.setUnboxConstValue(int cid, int value);
static KMETHOD Expr_setUnboxConstValue(KonohaContext *kctx, KonohaStack *sfp)
{
	kExpr *expr = sfp[0].asExpr;
	ktype_t tid = sfp[1].intValue;
	int value = sfp[2].intValue;
	KReturn(SUGAR kExpr_setUnboxConstValue(kctx, expr, tid, value));
}
// --------------------------------------------------------------------------

//## void NameSpace.setTokenFunc(String keyword, int konohaChar, Func f);
static KMETHOD NameSpace_setTokenFunc(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *key = sfp[1].asString;
	int konohaChar = (int)sfp[2].intValue;
	SUGAR kNameSpace_setTokenFunc(kctx, sfp[0].asNameSpace, ksymbolA(S_text(key), S_size(key), _NEWID), konohaChar, sfp[3].asFunc);
	KLIB kNameSpace_compileAllDefinedMethods(kctx);
}

//## void NameSpace.compileAllDefinedMethods();
static KMETHOD NameSpace_compileAllDefinedMethods(KonohaContext *kctx, KonohaStack *sfp)
{
	KLIB kNameSpace_compileAllDefinedMethods(kctx);
}

//## void NameSpace.addPatternMatch(String keyword, Func f);
static KMETHOD NameSpace_addPatternMatch(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *key = sfp[1].asString;
	SUGAR kNameSpace_addSugarFunc(kctx, sfp[0].asNameSpace, ksymbolA(S_text(key), S_size(key), _NEWID), SugarFunc_PatternMatch, sfp[2].asFunc);
	KLIB kNameSpace_compileAllDefinedMethods(kctx);
}

//## void NameSpace.addExpression(String keyword, Func f);
static KMETHOD NameSpace_addExpression(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *key = sfp[1].asString;
	SUGAR kNameSpace_addSugarFunc(kctx, sfp[0].asNameSpace, ksymbolA(S_text(key), S_size(key), _NEWID), SugarFunc_Expression, sfp[2].asFunc);
	KLIB kNameSpace_compileAllDefinedMethods(kctx);
}

//## void NameSpace.addStatement(String keyword, Func f);
static KMETHOD NameSpace_addStatement(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *key = sfp[1].asString;
	SUGAR kNameSpace_addSugarFunc(kctx, sfp[0].asNameSpace, ksymbolA(S_text(key), S_size(key), _NEWID), SugarFunc_Statement, sfp[2].asFunc);
	KLIB kNameSpace_compileAllDefinedMethods(kctx);
}


//## void NameSpace.addTopLevelStatement(String keyword, Func f);
static KMETHOD NameSpace_addTopLevelStatement(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *key = sfp[1].asString;
	SUGAR kNameSpace_addSugarFunc(kctx, sfp[0].asNameSpace, ksymbolA(S_text(key), S_size(key), _NEWID), SugarFunc_TopLevelStatement, sfp[2].asFunc);
	KLIB kNameSpace_compileAllDefinedMethods(kctx);
}

//## void NameSpace.addTypeCheck(String keyword, Func f);
static KMETHOD NameSpace_addTypeCheck(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *key = sfp[1].asString;
	SUGAR kNameSpace_addSugarFunc(kctx, sfp[0].asNameSpace, ksymbolA(S_text(key), S_size(key), _NEWID), SugarFunc_TypeCheck, sfp[2].asFunc);
	KLIB kNameSpace_compileAllDefinedMethods(kctx);
}


////## void NameSpace.addTermExpression(String keyword);
//static KMETHOD NameSpace_addTermExpression(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kString *key = sfp[1].asString;
//	DBG_P("termparseexpr=%p", kmodsugar->Expression_Term);
//	SUGAR kNameSpace_addSugarFunc(kctx, sfp[0].asNameSpace, ksymbolA(S_text(key), S_size(key), _NEWID), SugarFunc_Expression, kmodsugar->Expression_Term);
//}

//## Expr Stmt.printError(String msg);
static KMETHOD kStmt_printMessage2rintError(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt   *stmt  = sfp[0].asStmt;
	kString *msg   = sfp[1].asString;
	SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, "%s", S_text(msg));
	KReturn(K_NULLEXPR);
}

// --------------------------------------------------------------------------
// AST Method

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
//	KReturn(SUGAR new_kBlock(kctx, Stmt_nameSpace(stmt), stmt, tokenList, s, e, ';'));
//}

////## Expr Stmt.newExpr(Token[] tokenList, int s, int e);
//static KMETHOD Stmt_newExpr(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt *stmt  = sfp[0].asStmt;
//	kArray *tokenList  = sfp[1].asArray;
//	int s = sfp[2].intValue, e = sfp[3].intValue;
//	KReturn(SUGAR kStmt_parseExpr(kctx, stmt, tokenList, s, e, NULL));
//}

////## Expr Stmt.newMethodCallExpr(Token key, Token self);
//static KMETHOD Stmt_newMethodCallExpr(KonohaContext *kctx, KonohaStack *sfp)
//{
//	USING_SUGAR;
//	kStmt *stmt  = sfp[0].asStmt;
//	kToken *tk   = sfp[1].asToken;
//	assert(tk->keyword != 0);
//	kExprVar *expr = /*G*/new_(ExprVar, SYN_(Stmt_nameSpace(stmt), tk->keyword));
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

////## Expr Expr.addExpr(Expr expr, Expr o);
//static KMETHOD Expr_addExpr(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kExpr *expr  = sfp[0].asExpr;
//	kExpr *o     = sfp[1].asExpr;
//	if(IS_NULL(o) && IS_Array(expr->cons)) {
//		kObject_set(NullObject, expr, 1);
//	}
//	if(IS_NOTNULL(expr)) {
//		KLIB kArray_add(kctx, expr->cons, o);
//	}
//	KReturn(expr);
//}

// --------------------------------------------------------------------------

static kbool_t sugar_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	KonohaClass *cSymbol = loadSymbolClass(kctx, ns, trace);
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
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(ClassData), 0);

	int FN_buildid = FN_("buildid"), FN_key = FN_("key"), FN_defval = FN_("defval");
	int FN_typeid = FN_("typeid"), FN_gma = FN_("gma"), FN_pol = FN_("pol");
	int FN_func = FN_("func"), FN_msg = FN_("msg");
	int FN_x = FN_("x");
//	int FN_tokenList = FN_("tokens"), FN_s = FN_("s"), FN_e = FN_("e");

	/* Array[String] */
	kparamtype_t P_StringArray[] = {{TY_String}};
	int TY_StringArray = (KLIB KonohaClass_Generics(kctx, CT_Array, TY_void, 1, P_StringArray))->typeId;
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
		_Public, _F(Token_setUnresolvedTokenType),  TY_void, TY_Token, MN_("setUnresolvedTokenType"),  1, TY_String, FN_x,
		_Public, _F(Token_setText),  TY_void, TY_Token, MN_("setText"),  1, TY_String, FN_x,
		_Public, _F(Token_setSubArray), TY_void, TY_Token, MN_("setSubArray"), 1, TY_StringArray, FN_x,
//		_Public, _F(Token_isTypeName), TY_boolean, TY_Token, MN_("isTypeName"), 0,
//		_Public, _F(Token_isParenthesis), TY_boolean, TY_Token, MN_("isParenthesis"), 0,
		_Public, _F(Token_getText), TY_String, TY_Token, MN_("getText"), 0,

		_Public, _F(Stmt_getBuild), TY_int, TY_Stmt,  MN_("getBuild"), 0,
		_Public, _F(Stmt_setBuild), TY_void, TY_Stmt, MN_("setBuild"), 1, TY_int, FN_buildid,
		_Public, _F(Stmt_getBlock), TY_Block, TY_Stmt, MN_("getBlock"), 2, TY_String, FN_key, TY_Object, FN_defval,
		_Public, _F(kStmt_tyCheckByName), TY_boolean, TY_Stmt, MN_("tyCheckExpr"), 4, TY_String, FN_key, TY_Gamma, FN_gma, TY_int, FN_typeid, TY_int, FN_pol,
		_Public, _F(kBlock_tyCheckAll), TY_boolean, TY_Block, MN_("tyCheckAll"), 1, TY_Gamma, FN_gma,

		_Public, _F(Expr_getTermToken), TY_Token, TY_Expr, MN_("getTermToken"), 0,
		_Public, _F(Expr_setUnboxConstValue), TY_Expr, TY_Expr, MN_("setUnboxConstValue"), 2, TY_int, FN_("type"), TY_int, FN_("value"),
		_Public, _F(NameSpace_compileAllDefinedMethods), TY_void, TY_NameSpace, MN_("compileAllDefinedMethods"), 0,
		_Public, _F(NameSpace_setTokenFunc), TY_void, TY_NameSpace, MN_("setTokenFunc"), 3, TY_String, FN_key, TY_int, FN_("kchar"), TY_FuncToken, FN_func,
		_Public, _F(NameSpace_addPatternMatch), TY_void, TY_NameSpace, MN_("addPatternMatch"), 2, TY_String, FN_key, TY_FuncPatternMatch, FN_func,
		_Public, _F(NameSpace_addExpression), TY_void, TY_NameSpace, MN_("addExpression"), 2, TY_String, FN_key, TY_FuncExpression, FN_func,
		_Public, _F(NameSpace_addTopLevelStatement), TY_void, TY_NameSpace, MN_("addTopLevelStatement"), 2, TY_String, FN_key, TY_FuncStatement, FN_func,
		_Public, _F(NameSpace_addStatement), TY_void, TY_NameSpace, MN_("addStatement"), 2, TY_String, FN_key, TY_FuncStatement, FN_func,
		_Public, _F(NameSpace_addTypeCheck), TY_void, TY_NameSpace, MN_("addTypeCheck"), 2, TY_String, FN_key, TY_FuncTypeCheck, FN_func,

		_Public, _F(kStmt_printMessage2rintError), TY_Expr, TY_Stmt, MN_("printError"), 1, TY_String, FN_msg,

//		_Public, _F(Stmt_newExpr), TY_Expr, TY_Stmt, MN_("newExpr"), 1, TY_String, FN_key,
		_Public, _F(Stmt_setType), TY_void, TY_Stmt, MN_("setType"), 1, TY_int, FN_x,
//		_Public, _F(kStmt_printMessage2arsedExpr), TY_Expr, TY_Stmt, MN_("parseExpr"), 3, TY_TokenArray, FN_tokenList, TY_int, FN_s, TY_int, FN_e,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	loadNameSpaceMethodData(kctx, ns, cSymbol->typeId);
	return true;
}

static kbool_t sugar_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
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
		if(tokenList->TokenItems[beginIdx]->resolvedSyntaxInfo->keyword == KW_TextPattern) {
			ksymbol_t kw;
			if(isSubKeyword(kctx, tokenList, beginIdx, endIdx)) {
				char buf[256];
				PLATAPI snprintf_i(buf, sizeof(buf), "%s_%s", S_text(tokenList->TokenItems[beginIdx]->text), S_text(tokenList->TokenItems[beginIdx+1]->text));
				kw = ksymbolA((const char*)buf, strlen(buf), SYM_NEWID);
			}
			else {
				kw = ksymbolA(S_text(tokenList->TokenItems[beginIdx]->text), S_size(tokenList->TokenItems[beginIdx]->text), SYM_NEWID);
			}
			return (SugarSyntaxVar*)NEWSYN_(ns, kw);
		}
	}
	return NULL;
}

static KMETHOD Statement_syntax(KonohaContext *kctx, KonohaStack *sfp)
{
	kbool_t r = 0;
	VAR_Statement(stmt, gma);
	kTokenArray *tokenList = (kTokenArray*)kStmt_getObject(kctx, stmt, KW_TokenPattern, NULL);
	if(tokenList != NULL) {
		FIXME_ASSERT(IS_Array(tokenList));  // tokenList can be Token
		kNameSpace *ns = Stmt_nameSpace(stmt);
		SugarSyntaxVar *syn = kNameSpace_guessSyntaxFromTokenList(kctx, ns, tokenList);
		if(syn != NULL) {
			if(syn->syntaxPatternListNULL_OnList != NULL) {
				SUGAR kStmt_printMessage2(kctx, stmt, NULL, InfoTag, "overriding syntax: %s%s", PSYM_t(syn->keyword));
			}
			else {
				syn->syntaxPatternListNULL_OnList = new_(Array, 8, ns->NameSpaceConstList);
			}
			TokenSequence tokens = {ns, tokenList, 0, kArray_size(tokenList)};
			SUGAR kArray_addSyntaxRule(kctx, syn->syntaxPatternListNULL_OnList, &tokens);
			r = 1;
		}
		kStmt_done(kctx, stmt);
	}
	KReturnUnboxValue(r);
}

static kbool_t sugar_initNameSpace(KonohaContext *kctx, kNameSpace *packageNS, kNameSpace *ns, KTraceInfo *trace)
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
#undef DEFINE_KEYWORD
		{NULL},
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(IntData), trace);
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("syntax"), 0, "\"syntax\" $Token $Token*", 0, 0, NULL, NULL, Statement_syntax, NULL, NULL, },
		{ KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNS);
	return true;
}

static kbool_t sugar_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNS, kNameSpace *ns, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* sugar_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "sugar", "1.0");
	d.initPackage    = sugar_initPackage;
	d.setupPackage   = sugar_setupPackage;
	d.initNameSpace  = sugar_initNameSpace;
	d.setupNameSpace = sugar_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
