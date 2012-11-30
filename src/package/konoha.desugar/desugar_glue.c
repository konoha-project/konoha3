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

#define TP_kw           TY_Symbol, FN_("keyword")
#define TP_source       TY_String, FN_("source")
#define TP_pos          TY_int, FN_("pos")
#define TP_tokens       TY_TokenArray, FN_("tokens")
#define TP_begin        TY_int, FN_("begin")
#define TP_end          TY_int, FN_("end")
#define TP_message      TY_String, FN_("message")
#define TP_source       TY_String, FN_("source")

#define TP_level         TY_int, FN_("level")
#define TP_token         TY_Token, FN_("token")
#define TP_expr          TY_Expr,  FN_("expr")


// --------------------------------------------------------------------------
/* TokenFunc */

/** sample code
 * int SingleQuoteToken (Token tk, String source) {
	int pos, ch, prev = '\'';
	while((ch = source.AsciiAt(pos)) != 0) {
		if(ch == '\'' && prev != '\\') {
			tk.setText("$SingleQuotedChar", source, 1, pos - 1);
			return pos + 1;
		}
		prev = ch;
	}
	return tk.setError("must closed with '", source, 0, end);
}
 *
 ***/

//## int String.asciiAt(int pos);
static KMETHOD String_AsciiAt(KonohaContext *kctx, KonohaStack *sfp)
{
	int ch = 0;
	size_t n = (size_t)sfp[1].intValue, len = S_size(sfp[0].asString);
	if(n < len) {
		ch = ((unsigned char *)S_text(sfp[0].asString))[n];
	}
	KReturnUnboxValue(ch);
}

//## int Token.setText(symbol keyword, String s, int begin, int end);
static KMETHOD Token_setText(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *) sfp[0].asToken;
	if(IS_NOTNULL(tk)) {
		kString *text = sfp[2].asString;
		size_t len = S_size(text);
		size_t beginIdx = (size_t)sfp[3].intValue;
		size_t endIdx   = (size_t)sfp[4].intValue;
		if(beginIdx <= endIdx && endIdx < len) {
			ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
			tk->unresolvedTokenType = keyword;
			KFieldSet(tk, tk->text, KLIB new_kString(kctx, OnField, S_text(text) + beginIdx, endIdx - beginIdx, 0));
		}
	}
	KReturnUnboxValue(sfp[4].intValue);
}

//## int Token.setError(String s, String s, int begin, int end);
static KMETHOD Token_setError(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *) sfp[0].asToken;
	if(IS_NOTNULL(tk)) {
		SUGAR kToken_ToError(kctx, tk, ErrTag, "%s", S_text(sfp[1].asString));
	}
	KReturnUnboxValue(sfp[4].intValue);
}

static void KBuffer_WriteToken(KonohaContext *kctx, KGrowingBuffer *wb, kToken *tk)
{
	char c = kToken_GetOpenHintChar(tk);
	if(IS_String(tk)) {
		if(c != 0) {
			KLIB KBuffer_Write(kctx, wb, &c, 1);
			KLIB KBuffer_Write(kctx, wb, S_text(tk->text), S_size(tk->text));
			KLIB KBuffer_Write(kctx, wb, &c, 1);
		}
		else {
			KLIB KBuffer_Write(kctx, wb, S_text(tk->text), S_size(tk->text));
		}
	}
	else if(IS_Array(tk)) {
		size_t i;
		kArray *a = tk->subTokenList;
		KLIB KBuffer_Write(kctx, wb, &c, 1);
		for(i = 0; i < kArray_size(a); i++) {
			KLIB KBuffer_Write(kctx, wb, " ", 1);
			KBuffer_WriteToken(kctx, wb, a->TokenItems[i]);
		}
		KLIB KBuffer_Write(kctx, wb, " ", 1);
		c = kToken_GetCloseHintChar(tk);
		KLIB KBuffer_Write(kctx, wb, &c, 1);
	}
	else {
		KLIB KBuffer_printf(kctx, wb, "%s%s", PSYM_t(tk->resolvedSymbol));
	}
}

//## String Token.toString();
static KMETHOD Token_toString(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *) sfp[0].asToken;
	if(IS_String(tk->text)) {
		KReturn(tk->text);
	}
	else {
		KGrowingBuffer wb;
		KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
		KBuffer_WriteToken(kctx, &wb, tk);
		KReturnWith(KLIB new_kString(kctx, OnStack, KLIB KBuffer_Stringfy(kctx, &wb, 1), KBuffer_bytesize(&wb), 0), KLIB KBuffer_Free(&wb));
	}
}

//## Token[] NameSpace.tokenize(String s);
static KMETHOD NameSpace_tokenize(KonohaContext *kctx, KonohaStack *sfp)
{
	INIT_GCSTACK();
	kArray *a = (kArray *)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), 0);
	TokenSeq source = {sfp[0].asNameSpace, GetSugarContext(kctx)->preparedTokenList};
	TokenSeq_Push(kctx, source);
	SUGAR TokenSeq_Tokenize(kctx, &source, S_text(sfp[1].asString), 0);
	TokenSeq tokens = {source.ns, a, 0};
	tokens.TargetPolicy.ExpandingBraceGroup = true;
	SUGAR TokenSeq_Resolve(kctx, &tokens, NULL, &source, source.beginIdx);
	TokenSeq_Pop(kctx, source);
	KReturnWith(a, RESET_GCSTACK());
}

////## boolean NameSpace.DefineMacro(Symbol symbol, int param, String source);
//static KMETHOD NameSpace_tokenize(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
//	size_t beginIdx = (size_t)sfp[2].intValue;
//	kString *source = sfp[3].asString;
//	if(SUGAR kNameSpace_SetMacroData(kctx, sfp[0].asNameSpace, keyword, paramsize, S_text(source))) {
//		;
//	}
//	KReturnWith(a, RESET_GCSTACK());
//}

////## Token Token.new(symbol key);
//static KMETHOD Token_new(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kTokenVar *tk = (kTokenVar *)sfp[0].asToken;
//	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
//	tk->resolvedSymbol = key;
//	KReturn(tk);
//}

//## boolean Token.Is(Symbol s);
static KMETHOD Token_Is(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *)sfp[0].asToken;
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	KReturnUnboxValue(tk->resolvedSyntaxInfo != NULL && tk->resolvedSyntaxInfo->keyword == keyword);
}

//## String Token.getText();
static KMETHOD Token_getText(KonohaContext *kctx, KonohaStack *sfp)
{
	kToken *tk = sfp[0].asToken;
	if(IS_String(tk->text)) {
		KReturnField(tk->text);
	}
}

//## void Token.getSubTokenList();
static KMETHOD Token_getSubTokenList(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *) sfp[0].asToken;
	if(IS_String(tk->subTokenList)) {
		KReturnField(tk->subTokenList);
	}
}

////## boolean Token.isBeforeWhiteSpace();
//static KMETHOD Token_isBeforeWhiteSpace(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KReturnUnboxValue(kToken_is(BeforeWhiteSpace, sfp[0].asToken));
//}

static void desugar_defineTokenFunc(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(String_AsciiAt), TY_int, TY_String, MN_("AsciiAt"), 1, TP_pos,
		_Public, _F(Token_setText), TY_int, TY_Token, MN_("SetText"), 4, TP_kw, TP_source, TP_begin, TP_end,
		_Public, _F(Token_setError), TY_int, TY_Token, MN_("SetError"), 4, TP_message, TP_source, TP_begin, TP_end,
		_Public|_Coercion|_Im|_Const, _F(Token_toString), TY_String, TY_Token, MN_to(TY_String), 0,
		_Public|_Const, _F(Token_Is), TY_boolean, TY_Token, MN_("Is"), 1, TP_kw,
		_Public|_Const, _F(Token_getText), TY_String, TY_Token, MN_("GetText"), 0,
		_Public|_Const, _F(Token_getSubTokenList), TY_TokenArray, TY_Token, MN_("GetSubTokenList"), 0,
		_Public|_Im, _F(NameSpace_tokenize), TY_TokenArray, TY_NameSpace, MN_("Tokenize"), 1, TP_source,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

// --------------------------------------------------------------------------
/* CompilerError */

//## Expr Stmt.message(int error, String msg);
static KMETHOD Stmt_Message(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt   *stmt  = sfp[0].asStmt;
	kinfotag_t level = (kinfotag_t)sfp[1].intValue;
	kString *msg   = sfp[2].asString;
	KReturn(SUGAR kStmt_Message2(kctx, stmt, NULL, level, "%s", S_text(msg)));
}

//## Expr Stmt.message(int error, Token tk, String msg);
static KMETHOD StmtToken_Message(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt   *stmt  = sfp[0].asStmt;
	kinfotag_t level = (kinfotag_t)sfp[1].intValue;
	kString *msg   = sfp[3].asString;
	KReturn(SUGAR kStmt_Message2(kctx, stmt, sfp[2].asToken, level, "%s", S_text(msg)));
}

//## Expr Stmt.message(int error, Expr expr, String msg);
static KMETHOD StmtExpr_Message(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt   *stmt  = sfp[0].asStmt;
	kinfotag_t level = (kinfotag_t)sfp[1].intValue;
	kString *msg   = sfp[3].asString;
	KReturn(SUGAR kStmt_Message2(kctx, stmt, sfp[2].asToken, level, "%s", S_text(msg)));
}

static void desugar_defineMessageMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Stmt_Message), TY_Expr, TY_Stmt, MN_("message"), 2, TP_level, TP_message,
		_Public, _F(StmtToken_Message), TY_Expr, TY_Stmt, MN_("message"), 3, TP_level, TP_token, TP_message,
		_Public, _F(StmtExpr_Message), TY_Expr, TY_Stmt, MN_("message"), 3, TP_level, TP_expr, TP_message,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

// --------------------------------------------------------------------------
/* Block */

// Block new(NameSpace, ns)
//static KMETHOD Block_new(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KReturn(new_(Block, sfp[1].asNameSpace, OnStack));
//}

//## Block Stmt.newBlock(Token[] tokenList, int beginIdx, endIdx);
//static KMETHOD Stmt_newBlock(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt    *stmt = sfp[0].asStmt;
//	kArray   *tokenList = sfp[1].asArray;
//	int beginIdx = (int)sfp[2].intValue;
//	int endIdx   = (int)sfp[3].intValue;
//	TokenSeq source = {Stmt_ns(stmt), tokenList, beginIdx, endIdx};
////	SUGAR TokenSeq_Tokenize(kctx, &source, S_text(macro), 0);
//	KReturn(SUGAR new_kBlock(kctx, stmt, NULL/*nameSpace*/, &source));
//}

//## boolean Blook.TypeCheckAll(Gamma gma);
static KMETHOD Block_TypeCheckAll(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(SUGAR kBlock_TypeCheckAll(kctx, sfp[0].asBlock, sfp[1].asGamma));
}

#define TP_gma       TY_Gamma, FN_("gma")

//static void desugar_defineBlockMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
//{
//	KDEFINE_METHOD MethodData[] = {
//		_Public, _F(Block_TypeCheckAll), TY_boolean, TY_Block, MN_("TypeCheckAll"), 1, TP_gma,
//		DEND,
//	};
//	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
//}

// --------------------------------------------------------------------------
/* Pattern */

/**
int patternMatchShell(Stmt stmt, int nameid, Token[] tokenList, int beginIdx, int endIdx) {
	Token firstToken = tokenList[beginIdx];
	if(firstToken.isSymbol() && SubProc.isCommand(firstToken.getText())) {
		int i = beginIdx + 1;
		while(i < endIdx) {
			if(tokenList[i].isParenthesis()) {
				return -1;
			}
			i = i + 1;
		}
		return beginIdx;
	}
	return -1;
}
**/

////## Token[] NameSpace.TestPatternMatch(Symbol symbol, String source);
//static KMETHOD NameSpace_TestPatternMatch(KonohaContext *kctx, KonohaStack *sfp)
//{
//	INIT_GCSTACK();
//	TokenSeq source = {sfp[0].asNameSpace, GetSugarContext(kctx)->preparedTokenList};
//	TokenSeq_Push(kctx, source);
//	SUGAR TokenSeq_Tokenize(kctx, &source, S_text(sfp[2].asString), 0);
//	TokenSeq tokens = {source.ns, source.tokenList, source.endIdx};
//	//tokens.TargetPolicy.ExpandingBraceGroup = (flag == 1);
//	SUGAR TokenSeq_Resolve(kctx, &tokens, NULL, &source, source.beginIdx);
//	ksymbol_t kw   = (ksymbol_t)sfp[1].intValue;
//	SugarSyntax *syn = SYN_(sfp[0].asNameSpace, kw);
//	int ret = 0;
//	ret = SUGAR kStmt_MatchPattern(kctx, KNULL(Stmt), syn, -1, tokens.tokenList, tokens.beginIdx, tokens.endIdx, &ret/*dummy*/);
//	TokenSeq_Pop(kctx, source);
//	KReturnWith(ret != -1, RESET_GCSTACK());
//}

////## void Stmt.AddParsedObject(keyword nameid, Object obj);
//static KMETHOD Stmt_AddParsedObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt   *stmt  = sfp[0].asStmt;
//	ksymbol_t symbol = (ksymbol_t)sfp[1].intValue;
//	SUGAR kStmt_AddParsedObject(kctx, stmt, symbol, sfp[2].asObject);
//	KReturnVoid();
//}

////## SmartReturn Object Stmt.GetParsedObject(keyword nameid);
//static KMETHOD Stmt_GetParsedObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt *stmt  = sfp[0].asStmt;
//	ksymbol_t symbol = (ksymbol_t)sfp[1].intValue;
//	kObject *o = SUGAR kStmt_GetObject(kctx, stmt, symbol, NULL);
//	if(o != NULL) {
//
//	}
//	KReturnDefaultValue();
//}

////## Block Stmt.getBlock(symbol key, Block def);
//static KMETHOD Stmt_getBlock(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
//	KReturn(SUGAR kStmt_GetBlock(kctx, sfp[0].asStmt, NULL/*DefaultNameSpace*/, key, sfp[2].asBlock));
//}
//
////## Token Stmt.getToken(symbol key, Token def);
//static KMETHOD Stmt_getToken(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt *stmt   = sfp[0].asStmt;
//	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
//	kToken *def   = sfp[2].asToken;
//	KReturn(SUGAR kStmt_GetToken(kctx, stmt, key, def));
//}
//
////## Expr Stmt.getExpr(symbol key, Expr def);
//static KMETHOD Stmt_getExpr(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt *stmt   = sfp[0].asStmt;
//	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
//	kExpr *def    = sfp[2].asExpr;
//	KReturn(SUGAR kStmt_GetExpr(kctx, stmt, key, def));
//}
//
////## void Stmt.setType(int build);
//static KMETHOD Stmt_setType(KonohaContext *kctx, KonohaStack *sfp)
//{
//	Stmt_typed(sfp[0].asStmt, sfp[1].intValue);
//	KReturnVoid();
//}

////## int Stmt.Match(Symbol key, symbol nameid, Token[] tokenList, int beginIdx, int endIdx);
//static KMETHOD Stmt_Match(KonohaContext *kctx, KonohaStack *sfp)
//{
//	int ret = -1;
//	kStmt   *stmt  = sfp[0].asStmt;
//	ksymbol_t kw   = (ksymbol_t)sfp[1].intValue;
//	int nameid   = (ksymbol_t)sfp[2].intValue;
//	kArray *tokenList = sfp[3].asArray;
//	int beginIdx = sfp[4].intValue;
//	int endIdx = sfp[5].intValue;
//	SugarSyntax *syn = SYN_(kStmt_ns(stmt), kw);
//	if(syn != NULL) {
//		int dummy = 0;
//		ret = SUGAR kStmt_MatchPattern(kctx, stmt, syn, nameid, tokenList, beginIdx, endIdx, &dummy);
//	}
//	KReturnUnboxValue(ret);
//}

// --------------------------------------------------------------------------
/* Stmt */

////## int Stmt.getBuild();
//static KMETHOD Stmt_getBuild(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KReturnUnboxValue(sfp[0].asStmt->build);
//}
//
////## void Stmt.setBuild(int buildid);
//static KMETHOD Stmt_setBuild(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmtVar *stmt = (kStmtVar *) sfp[0].asStmt;
//	stmt->build = sfp[1].intValue;
//}

////## boolean Stmt.TypeCheckExpr(symbol key, Gamma gma, Object type, int policy);
//static KMETHOD Stmt_TypeCheckExpr(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
//	kattrtype_t typeId = O_typeId(sfp[3].asObject);
//	KReturnUnboxValue(SUGAR kStmt_TypeCheckByName(kctx, sfp[0].asStmt, key, sfp[2].asGamma, typeId, sfp[4].intValue));
//}
//
////## Expr Stmt.newExpr(Token[] tokenList, int s, int e);
//static KMETHOD Stmt_newExpr(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt *stmt  = sfp[0].asStmt;
//	kArray *tokenList  = sfp[1].asArray;
//	int s = sfp[2].intValue, e = sfp[3].intValue;
//	KReturn(SUGAR kStmt_ParseExpr(kctx, stmt, tokenList, s, e, NULL));
//}
//
////## Expr Stmt.newUntypedCallStyleExpr(Token token, Expr expr1);
//static KMETHOD Stmt_newUntypedCallStyleExpr2(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt *stmt   = sfp[0].asStmt;
//	kToken *token = sfp[1].asToken;
//	kExpr *expr1  = sfp[2].asExpr;
//	SugarSyntax *syn = SYN_(Stmt_ns(stmt), KW_ExprMethodCall);
//	KReturn(SUGAR new_UntypedCallStyleExpr(kctx, syn, 2, token, expr1));
//}

#define TP_type TY_Object, FN_("type")
#define TP_pol  TY_int, FN_("policy")

//static void desugar_defineExprMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
//{
//	KDEFINE_METHOD MethodData[] = {
//		_Public, _F(Stmt_TypeCheckExpr), TY_boolean, TY_Stmt, MN_("TypeCheckExpr"), 4, TP_kw, TP_gma, TP_type, TP_pol,
//		_Public, _F(Stmt_newExpr), TY_Expr, TY_Stmt, MN_("newExpr"), 3, TP_tokens, TP_begin, TP_end,
//		_Public, _F(Stmt_newUntypedCallStyleExpr2), TY_Expr, TY_Stmt, MN_("newUntypedCallStyleExpr"), 2, TP_token, TP_expr,
//
////		_Public, _F(Stmt_setType), TY_void, TY_Stmt, MN_("setType"), 1, TY_int, FN_x,
//////		_Public, _F(kStmt_Message2arsedExpr), TY_Expr, TY_Stmt, MN_("parseExpr"), 3, TY_TokenArray, FN_tokenList, TY_int, FN_s, TY_int, FN_e,
////		_Public, _F(Stmt_newUntypedCallStyleExpr3), TY_Expr, TY_Stmt, MN_("newUntypedCallStyleExpr"), 3, TY_Token, FN_("token"), TY_Expr, FN_("expr1"), TY_Expr, FN_("expr2"),
////		_Public, _F(Stmt_newUntypedCallStyleExpr4), TY_Expr, TY_Stmt, MN_("newUntypedCallStyleExpr"), 4, TY_Token, FN_("token"), TY_Expr, FN_("expr1"), TY_Expr, FN_("expr2"), TY_Expr, FN_("expr3"),
////		_Public, _F(Stmt_newUntypedCallStyleExpr5), TY_Expr, TY_Stmt, MN_("newUntypedCallStyleExpr"), 5, TY_Token, FN_("token"), TY_Expr, FN_("expr1"), TY_Expr, FN_("expr2"), TY_Expr, FN_("expr3"), TY_Expr, FN_("expr4"),
////		_Public, _F(Stmt_newUntypedCallStyleExpr6), TY_Expr, TY_Stmt, MN_("newUntypedCallStyleExpr"), 6, TY_Token, FN_("token"), TY_Expr, FN_("expr1"), TY_Expr, FN_("expr2"), TY_Expr, FN_("expr3"), TY_Expr, FN_("expr4"), TY_Expr, FN_("expr5"),
////		_Public, _F(Stmt_newTypedCallExpr1), TY_Expr, TY_Stmt, MN_("newTypedCallExpr"), 4, TY_Gamma, FN_gma, TY_cid, FN_typeid, TY_Symbol, FN_("methodName"), TY_Expr, FN_("firstExpr"),
////		_Public, _F(Stmt_newTypedCallExpr2), TY_Expr, TY_Stmt, MN_("newTypedCallExpr"), 5, TY_Gamma, FN_gma, TY_cid, FN_typeid, TY_Symbol, FN_("methodName"), TY_Expr, FN_("firstExpr"), TY_Expr, FN_("secondExpr"),
////		_Public, _F(Stmt_rightJoinExpr), TY_Expr, TY_Stmt, MN_("rightJoinExpr"), 4, TY_Expr, FN_expr, TY_TokenArray, FN_tokenList, TY_int, FN_s, TY_int, FN_e,
////		_Public, _F(Stmt_getTokenList), TY_TokenArray, TY_Stmt, MN_("getTokenList"), 2, TY_Symbol, FN_key, TY_TokenArray, FN_defval,
////		_Public, _F(Stmt_done), TY_void, TY_Stmt, MN_("done"), 0,
//		DEND,
//	};
//	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
//}

////## Expr Stmt.newUntypedCallStyleExpr(Token token, Expr expr1, Expr expr2);
//static KMETHOD Stmt_newUntypedCallStyleExpr3(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt *stmt   = sfp[0].asStmt;
//	kToken *token = sfp[1].asToken;
//	kExpr *expr1  = sfp[2].asExpr;
//	kExpr *expr2  = sfp[3].asExpr;
//	SugarSyntax *syn = SYN_(Stmt_ns(stmt), KW_ExprMethodCall);
//	KReturn(SUGAR new_UntypedCallStyleExpr(kctx, syn, 3, token, expr1, expr2));
//}
//
////## Expr Stmt.newUntypedCallStyleExpr(Token token, Expr expr1, Expr expr2, Expr expr3);
//static KMETHOD Stmt_newUntypedCallStyleExpr4(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt *stmt   = sfp[0].asStmt;
//	kToken *token = sfp[1].asToken;
//	kExpr *expr1  = sfp[2].asExpr;
//	kExpr *expr2  = sfp[3].asExpr;
//	kExpr *expr3  = sfp[4].asExpr;
//	SugarSyntax *syn = SYN_(Stmt_ns(stmt), KW_ExprMethodCall);
//	KReturn(SUGAR new_UntypedCallStyleExpr(kctx, syn, 4, token, expr1, expr2, expr3));
//}
//
////## Expr Stmt.newUntypedCallStyleExpr(Token token, Expr expr1, Expr expr2, Expr expr3, Expr expr4);
//static KMETHOD Stmt_newUntypedCallStyleExpr5(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt *stmt   = sfp[0].asStmt;
//	kToken *token = sfp[1].asToken;
//	kExpr *expr1  = sfp[2].asExpr;
//	kExpr *expr2  = sfp[3].asExpr;
//	kExpr *expr3  = sfp[4].asExpr;
//	kExpr *expr4  = sfp[5].asExpr;
//	SugarSyntax *syn = SYN_(Stmt_ns(stmt), KW_ExprMethodCall);
//	KReturn(SUGAR new_UntypedCallStyleExpr(kctx, syn, 5, token, expr1, expr2, expr3, expr4));
//}
//
////## Expr Stmt.newUntypedCallStyleExpr(Token token, Expr expr1, Expr expr2, Expr expr3, Expr expr4, Expr expr5);
//static KMETHOD Stmt_newUntypedCallStyleExpr6(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt *stmt   = sfp[0].asStmt;
//	kToken *token = sfp[1].asToken;
//	kExpr *expr1  = sfp[2].asExpr;
//	kExpr *expr2  = sfp[3].asExpr;
//	kExpr *expr3  = sfp[4].asExpr;
//	kExpr *expr4  = sfp[5].asExpr;
//	kExpr *expr5  = sfp[6].asExpr;
//	SugarSyntax *syn = SYN_(Stmt_ns(stmt), KW_ExprMethodCall);
//	KReturn(SUGAR new_UntypedCallStyleExpr(kctx, syn, 6, token, expr1, expr2, expr3, expr4, expr5));
//}

////## Expr Stmt.newTypedCallExpr(Gamma gma, cid typeId, symbol methodName, Expr firstExpr);
//static KMETHOD Stmt_newTypedCallExpr1(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt *stmt          = sfp[0].asStmt;
//	kGamma *gma          = sfp[1].asGamma;
//	kattrtype_t cid          = (kattrtype_t)sfp[2].intValue;
//	ksymbol_t methodName = (ksymbol_t)sfp[3].intValue;
//	kExpr *firstExpr     = sfp[4].asExpr;
//	kMethod *method = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, Stmt_ns(stmt), cid, methodName, 1, MethodMatch_CamelStyle);
//	if(method == NULL) {
//		KReturn(KNULL(Expr));
//	}
//	KReturn(SUGAR new_TypedCallExpr(kctx, stmt, gma, cid, method, 1, firstExpr));
//}
//
////## Expr Stmt.newTypedCallExpr(Gamma gma, cid typeId, String methodName, Expr firstExpr, Expr secondExpr);
//static KMETHOD Stmt_newTypedCallExpr2(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt *stmt          = sfp[0].asStmt;
//	kGamma *gma          = sfp[1].asGamma;
//	kattrtype_t cid          = (kattrtype_t)sfp[2].intValue;
//	ksymbol_t methodName = (ksymbol_t)sfp[3].intValue;
//	kExpr *firstExpr     = sfp[4].asExpr;
//	kExpr *secondExpr    = sfp[5].asExpr;
//	kMethod *method = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, Stmt_ns(stmt), cid, methodName, 2, MethodMatch_CamelStyle);
//	if(method == NULL) {
//		KReturn(KNULL(Expr));
//	}
//	KReturn(SUGAR new_TypedCallExpr(kctx, stmt, gma, cid, method, 2, firstExpr, secondExpr));
//}
//
////## Expr Stmt.rightJoinExpr(Expr expr, Token[] tokenList, int currentIdx, int endIdx);
//static KMETHOD Stmt_rightJoinExpr(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt *stmt       = sfp[0].asStmt;
//	kExpr *expr       = sfp[1].asExpr;
//	kArray *tokenList = sfp[2].asArray;
//	int currentIdx    = sfp[3].intValue;
//	int endIdx        = sfp[4].intValue;
//	KReturn(SUGAR kStmt_RightJoinExpr(kctx, stmt, expr, tokenList, currentIdx, endIdx));
//}
//
////## Token[] Stmt.getTokenList(symbol keyword, Token[] def);
//static KMETHOD Stmt_getTokenList(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt    *stmt    = sfp[0].asStmt;
//	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
//	kArray   *def     = sfp[2].asArray;
//	kTokenArray *tokenList = (kTokenArray *)kStmt_GetObject(kctx, stmt, keyword, def);
//	kTokenArray *ret;
//	if(!IS_Array(tokenList)) {
//		ret = new_(TokenArray, 0, OnStack);
//		KLIB kArray_Add(kctx, ret, tokenList);
//	}
//	else {
//		ret = tokenList;
//	}
//	KReturn(ret);
//}
//
////## void Stmt.done();
//static KMETHOD Stmt_done(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt_done(kctx, sfp[0].asStmt);
//	KReturnVoid();
//}
//
////## void Stmt.setExpr(symbol key, Expr expr);
////## void Stmt.setBlock(symbol key, Block block);
//static KMETHOD Stmt_setObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt *stmt   = sfp[0].asStmt;
//	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
//	kObject *obj  = sfp[2].asObject;
//	kStmt_setObject(kctx, stmt, key, obj);
//	KReturnVoid();
//}
//
////## boolean Stmt.declType(Gamma gma, cid typeId, Expr declExpr);
//static KMETHOD Stmt_declType(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt *stmt     = sfp[0].asStmt;
//	kGamma *gma     = sfp[1].asGamma;
//	kattrtype_t cid     = (kattrtype_t)sfp[2].intValue;
//	kExpr *declExpr = sfp[3].asExpr;
//	KReturnUnboxValue(SUGAR kStmt_DeclType(kctx, stmt, gma, cid, declExpr, NULL, NULL, &stmt));
//}
//
//
//// --------------------------------------------------------------------------
///* Expr */
//
////## Token Expr.getTermToken();
//static KMETHOD Expr_getTermToken(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kExpr *expr = sfp[0].asExpr;
//	KReturn(expr->termToken);
//}
//
////## Expr Expr.setConstValue(Object value);
//static KMETHOD Expr_setConstValue(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kExpr *expr = sfp[0].asExpr;
//	KonohaClass *ct = O_ct(sfp[1].asObject);
//	if(CT_IsUnbox(ct)) {
//		KReturn(SUGAR kExpr_SetUnboxConstValue(kctx, expr, ct->typeId, sfp[1].unboxValue));
//	}
//	KReturn(SUGAR kExpr_SetConstValue(kctx, expr, ct->typeId, sfp[1].asObject));
//}
//
////## Expr Expr.setVariable(Gamma gma, int build, cid typeid, int index);
//static KMETHOD Expr_setVariable(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kExpr *expr    = sfp[0].asExpr;
//	kGamma *gma    = sfp[1].asGamma;
//	kexpr_t build  = (kexpr_t)sfp[2].intValue;
//	kattrtype_t cid    = (kattrtype_t)sfp[3].intValue;
//	intptr_t index = sfp[4].unboxValue;
//	KReturn(SUGAR kExpr_SetVariable(kctx, expr, gma, build, cid, index));
//}
//
////## Expr Expr.new(Gamma gma, int build, cid typeid, int index);
//static KMETHOD Expr_newVariableExpr(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kGamma *gma    = sfp[1].asGamma;
//	kexpr_t build  = (kexpr_t)sfp[2].intValue;
//	kattrtype_t cid    = (kattrtype_t)sfp[3].intValue;
//	intptr_t index = sfp[4].unboxValue;
//	KReturn(new_VariableExpr(kctx, gma, build, cid, index));
//}
//
////## Expr Expr.new(Object value);
//static KMETHOD Expr_new(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KonohaClass *ct = O_ct(sfp[1].asObject);
//	if(CT_IsUnbox(ct)) {
//		KReturn(new_UnboxConstValueExpr(kctx, ct->typeId, sfp[1].unboxValue));
//	}
//	KReturn(new_ConstValueExpr(kctx, ct->typeId, sfp[1].asObject));
//}
//
////## void Expr.setType(int build, cid typeid);
//static KMETHOD Expr_setType(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kExprVar *expr = (kExprVar *)sfp[0].asExpr;
//	kexpr_t build  = (kexpr_t)sfp[1].intValue;
//	kattrtype_t cid    = (kattrtype_t)sfp[2].intValue;
//	expr->build = build;
//	expr->attrTypeId = cid;
//	KReturnVoid();
//}
//
//// --------------------------------------------------------------------------
//// AST Method
//
////## int Gamma.declareLocalVariable(cid typeId, symbol keyword);
//static KMETHOD Gamma_declareLocalVariable(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kGamma *gma       = sfp[0].asGamma;
//	kattrtype_t cid       = (kattrtype_t)sfp[1].intValue;
//	ksymbol_t keyword = (ksymbol_t)sfp[2].intValue;
//	KReturnUnboxValue(SUGAR kGamma_AddLocalVariable(kctx, gma, cid, keyword));
//}

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
//static KMETHOD Stmt_AddExprParam(KonohaContext *kctx, KonohaStack *sfp)
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
//		expr = SUGAR kStmt_AddExprParam(kctx, stmt, expr, tk->subTokenList, 0, kArray_size(tk->subTokenList), 1/*allowEmpty*/);
//	}
//	KReturn(expr);
//}

////## Expr Expr.addExpr(Expr expr, Expr o);
//static KMETHOD kExpr_AddExpr(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kExpr *expr  = sfp[0].asExpr;
//	kExpr *o     = sfp[1].asExpr;
//	if(IS_NULL(o) && IS_Array(expr->cons)) {
//		kObject_set(NullObject, expr, 1);
//	}
//	if(IS_NOTNULL(expr)) {
//		KLIB kArray_Add(kctx, expr->cons, o);
//	}
//	KReturn(expr);
//}

// --------------------------------------------------------------------------


//static kbool_t desugar_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
//{
//	KonohaClass *cSymbol = defineSymbolClass(kctx, ns, trace);
//	KDEFINE_INT_CONST ClassData[] = {   // add Array as available
//		{"Token", VirtualType_KonohaClass, (uintptr_t)CT_Token},
//		{"Stmt", VirtualType_KonohaClass,  (uintptr_t)CT_Stmt},
//		{"Expr", VirtualType_KonohaClass,  (uintptr_t)CT_Expr},
//		{"Block", VirtualType_KonohaClass, (uintptr_t)CT_Block},
//		{"Gamma", VirtualType_KonohaClass, (uintptr_t)CT_Gamma},
//		{"NameSpace", VirtualType_KonohaClass, (uintptr_t)CT_NameSpace},
//		{"symbol", VirtualType_KonohaClass, (uintptr_t)cSymbol},
//		{NULL},
//	};
//	KLIB kNameSpace_LoadConstData(kctx, ns, KonohaConst_(ClassData), 0);
//
//	int TY_Symbol = cSymbol->typeId;
//	desugar_defineTokenFunc(kctx, ns, TY_Symbol, trace);
//	desugar_defineMessageMethod(kctx, ns, TY_Symbol, trace);
//
//
//	int FN_buildid = FN_("buildid"), FN_key = FN_("key"), FN_defval = FN_("defval");
//	int FN_typeid = FN_("typeid"), FN_gma = FN_("gma"), FN_pol = FN_("pol");
//	int FN_msg = FN_("msg");
//	int FN_x = FN_("x");
//	int FN_tokenList = FN_("tokens"), FN_s = FN_("s"), FN_e = FN_("e");
//	int FN_expr = FN_("expr");
//
//	/* Array[String] */
//	kparamtype_t P_StringArray[] = {{TY_String}};
//	int TY_StringArray = (KLIB KonohaClass_Generics(kctx, CT_Array, TY_void, 1, P_StringArray))->typeId;
//
//	kattrtype_t TY_Symbol = cSymbol->typeId;
//	kattrtype_t TY_cid = ccid->typeId;
//
//	KDEFINE_METHOD MethodData[] = {
//		/* Token */
//		_Public, _F(Token_new), TY_Token, TY_Token, MN_("new"), 1, TY_Symbol, FN_key,
//		_Public, _F(Token_setUnresolvedTokenType),  TY_void, TY_Token, MN_("setUnresolvedTokenType"),  1, TY_Symbol, FN_x,
//		_Public, _F(Token_setText),  TY_void, TY_Token, MN_("setText"),  1, TY_String, FN_x,
//		_Public, _F(Token_setSubArray), TY_void, TY_Token, MN_("setSubArray"), 1, TY_StringArray, FN_x,
////		_Public, _F(Token_isTypeName), TY_boolean, TY_Token, MN_("isTypeName"), 0,
//		_Public, _F(Token_isParenthesis), TY_boolean, TY_Token, MN_("isParenthesis"), 0,
//		_Public, _F(Token_newUntypedExpr), TY_Expr, TY_Token, MN_("newUntypedExpr"), 0,
//
//		/* Stmt */
//		_Public, _F(Stmt_getBuild), TY_int, TY_Stmt,  MN_("getBuild"), 0,
//		_Public, _F(Stmt_setBuild), TY_void, TY_Stmt, MN_("setBuild"), 1, TY_int, FN_buildid,
//		_Public, _F(Stmt_getBlock), TY_Block, TY_Stmt, MN_("getBlock"), 2, TY_Symbol, FN_key, TY_Block, FN_defval,
//		_Public, _F(Stmt_getToken), TY_Token, TY_Stmt, MN_("getToken"), 2, TY_Symbol, FN_key, TY_Token, FN_defval,
//		_Public, _F(Stmt_getExpr), TY_Expr, TY_Stmt, MN_("getExpr"), 2, TY_Symbol, FN_key, TY_Expr, FN_defval,
//		_Public, _F(Stmt_TypeCheckExpr), TY_boolean, TY_Stmt, MN_("TypeCheckExpr"), 3, TY_Symbol, FN_key, TY_Gamma, FN_gma, TY_cid, FN_typeid,
//		_Public, _F(Stmt_TypeCheckExprPol), TY_boolean, TY_Stmt, MN_("TypeCheckExpr"), 4, TY_Symbol, FN_key, TY_Gamma, FN_gma, TY_cid, FN_typeid, TY_int, FN_pol,
//		_Public, _F(Stmt_Message2rintError), TY_Expr, TY_Stmt, MN_("printError"), 1, TY_String, FN_msg,
//
//		_Public, _F(Stmt_newExpr), TY_Expr, TY_Stmt, MN_("newExpr"), 3, TY_TokenArray, FN_tokenList, TY_int, FN_s, TY_int, FN_e,
//		_Public, _F(Stmt_setType), TY_void, TY_Stmt, MN_("setType"), 1, TY_int, FN_x,
////		_Public, _F(kStmt_Message2arsedExpr), TY_Expr, TY_Stmt, MN_("parseExpr"), 3, TY_TokenArray, FN_tokenList, TY_int, FN_s, TY_int, FN_e,
//		_Public, _F(Stmt_newUntypedCallStyleExpr2), TY_Expr, TY_Stmt, MN_("newUntypedCallStyleExpr"), 2, TY_Token, FN_("token"), TY_Expr, FN_("expr1"),
//		_Public, _F(Stmt_newUntypedCallStyleExpr3), TY_Expr, TY_Stmt, MN_("newUntypedCallStyleExpr"), 3, TY_Token, FN_("token"), TY_Expr, FN_("expr1"), TY_Expr, FN_("expr2"),
//		_Public, _F(Stmt_newUntypedCallStyleExpr4), TY_Expr, TY_Stmt, MN_("newUntypedCallStyleExpr"), 4, TY_Token, FN_("token"), TY_Expr, FN_("expr1"), TY_Expr, FN_("expr2"), TY_Expr, FN_("expr3"),
//		_Public, _F(Stmt_newUntypedCallStyleExpr5), TY_Expr, TY_Stmt, MN_("newUntypedCallStyleExpr"), 5, TY_Token, FN_("token"), TY_Expr, FN_("expr1"), TY_Expr, FN_("expr2"), TY_Expr, FN_("expr3"), TY_Expr, FN_("expr4"),
//		_Public, _F(Stmt_newUntypedCallStyleExpr6), TY_Expr, TY_Stmt, MN_("newUntypedCallStyleExpr"), 6, TY_Token, FN_("token"), TY_Expr, FN_("expr1"), TY_Expr, FN_("expr2"), TY_Expr, FN_("expr3"), TY_Expr, FN_("expr4"), TY_Expr, FN_("expr5"),
//		_Public, _F(Stmt_newTypedCallExpr1), TY_Expr, TY_Stmt, MN_("newTypedCallExpr"), 4, TY_Gamma, FN_gma, TY_cid, FN_typeid, TY_Symbol, FN_("methodName"), TY_Expr, FN_("firstExpr"),
//		_Public, _F(Stmt_newTypedCallExpr2), TY_Expr, TY_Stmt, MN_("newTypedCallExpr"), 5, TY_Gamma, FN_gma, TY_cid, FN_typeid, TY_Symbol, FN_("methodName"), TY_Expr, FN_("firstExpr"), TY_Expr, FN_("secondExpr"),
//		_Public, _F(Stmt_rightJoinExpr), TY_Expr, TY_Stmt, MN_("rightJoinExpr"), 4, TY_Expr, FN_expr, TY_TokenArray, FN_tokenList, TY_int, FN_s, TY_int, FN_e,
//		_Public, _F(Stmt_getTokenList), TY_TokenArray, TY_Stmt, MN_("getTokenList"), 2, TY_Symbol, FN_key, TY_TokenArray, FN_defval,
//		_Public, _F(Stmt_done), TY_void, TY_Stmt, MN_("done"), 0,
//		_Public, _F(Stmt_setObject), TY_void, TY_Stmt, MN_("setExpr"), 2, TY_Symbol, FN_key, TY_Expr, FN_expr,
//		_Public, _F(Stmt_setObject), TY_void, TY_Stmt, MN_("setBlock"), 2, TY_Symbol, FN_key, TY_Block, FN_("block"),
//		_Public, _F(Stmt_declType), TY_boolean, TY_Stmt, MN_("declType"), 3, TY_Gamma, FN_gma, TY_cid, FN_typeid, TY_Expr, FN_("declExpr"),
//		_Public, _F(Stmt_newBlock), TY_Block, TY_Stmt, MN_("newBlock"), 1, TY_String, FN_("macro"),
//
//		/* Expr */
//		_Public, _F(Expr_getTermToken), TY_Token, TY_Expr, MN_("getTermToken"), 0,
//		_Public, _F(Expr_setConstValue), TY_Expr, TY_Expr, MN_("setConstValue"), 1, TY_Object, FN_("value") | FN_COERCION,
//		_Public, _F(Expr_setVariable), TY_Expr, TY_Expr, MN_("setVariable"), 4, TY_Gamma, FN_gma, TY_int, FN_buildid, TY_cid, FN_typeid, TY_int, FN_("index"),
//		_Public, _F(Expr_newVariableExpr), TY_Expr, TY_Expr, MN_("new"), 4, TY_Gamma, FN_gma, TY_int, FN_buildid, TY_cid, FN_typeid, TY_int, FN_("index"),
//		_Public, _F(Expr_new), TY_Expr, TY_Expr, MN_("new"), 1, TY_Object, FN_("value") | FN_COERCION,
//		_Public, _F(Expr_setType), TY_void, TY_Expr, MN_("setType"), 2, TY_int, FN_buildid, TY_cid, FN_typeid,
//		_Public, _F(kExpr_AddExpr), TY_Expr, TY_Expr, MN_("addExpr"), 1, TY_Expr, FN_expr,
//
//
//		/* Gamma */
//		_Public, _F(Gamma_declareLocalVariable), TY_int, TY_Gamma, MN_("declareLocalVariable"), 2, TY_cid, FN_typeid, TY_Symbol, FN_key,
//		DEND,
//	};
//	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
//	LoadNameSpaceMethodData(kctx, ns, TY_Symbol, trace);
//	desuga_defineSyntaxStatement(kctx, ns, ns, trace);
//	return true;
//}


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

static void desugar_defineNameSpaceMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
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
		_Public|_Im, _F(NameSpace_DefinedSyntax), TY_boolean, TY_NameSpace, MN_("DefinedSyntax"), 1, TY_Symbol, FN_keyword,
		_Public|_Im, _F(NameSpace_DefinedLiteral), TY_boolean, TY_NameSpace, MN_("DefinedLiteral"), 1, TY_Symbol, FN_keyword,
		_Public|_Im, _F(NameSpace_DefinedStatement), TY_boolean, TY_NameSpace, MN_("DefinedStatement"), 1, TY_Symbol, FN_keyword,
		_Public|_Im, _F(NameSpace_DefinedExpression), TY_boolean, TY_NameSpace, MN_("DefinedExpression"), 1, TY_Symbol, FN_keyword,
		_Public|_Im, _F(NameSpace_DefinedBinaryOperator), TY_boolean, TY_NameSpace, MN_("DefinedBinaryOperator"), 1, TY_Symbol, FN_keyword,
//		_Public, _F(NameSpace_compileAllDefinedMethods), TY_void, TY_NameSpace, MN_("compileAllDefinedMethods"), 0,
		_Public, _F(NameSpace_setTokenFunc), TY_void, TY_NameSpace, MN_("SetTokenFunc"), 3, TY_Symbol, FN_keyword, TY_int, FN_("kchar"), TY_FuncToken, FN_func,
		_Public, _F(NameSpace_AddPatternMatch), TY_void, TY_NameSpace, MN_("AddPatternMatch"), 2, TY_Symbol, FN_keyword, TY_FuncPatternMatch, FN_func,
		_Public, _F(NameSpace_AddExpression), TY_void, TY_NameSpace, MN_("AddExpression"), 2, TY_Symbol, FN_keyword, TY_FuncExpression, FN_func,
		_Public, _F(NameSpace_AddTopLevelStatement), TY_void, TY_NameSpace, MN_("AddTopLevelStatement"), 2, TY_Symbol, FN_keyword, TY_FuncStatement, FN_func,
		_Public, _F(NameSpace_AddStatement), TY_void, TY_NameSpace, MN_("AddStatement"), 2, TY_Symbol, FN_keyword, TY_FuncStatement, FN_func,
		_Public, _F(NameSpace_AddTypeCheck), TY_void, TY_NameSpace, MN_("AddTypeCheck"), 2, TY_Symbol, FN_keyword, TY_FuncTypeCheck, FN_func,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

////## Expr Token.newUntypedExpr();
//static KMETHOD Token_newUntypedExpr(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kToken *token = sfp[0].asToken;
//	KReturn(SUGAR new_TermExpr(kctx, token));
//}


// ---------------------------------------------------------------------------

/* syntax */

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
	KLIB kArray_Add(kctx, arrayRef[0], o);
}

#define kToken_isFirstPattern(tk)   (KW_isPATTERN(tk->resolvedSymbol) && tk->stmtEntryKey != KW_ExprPattern)
static KMETHOD Statement_syntax(KonohaContext *kctx, KonohaStack *sfp)
{
	kbool_t r = 0;
	VAR_Statement(stmt, gma);
	kTokenArray *tokenList = (kTokenArray *)kStmt_GetObject(kctx, stmt, KW_TokenPattern, NULL);
	if(tokenList == NULL) {
		SUGAR kStmt_Message2(kctx, stmt, NULL, ErrTag, "empty syntax");
	}
	if(tokenList != NULL) {
		if(!IS_Array(tokenList)) { // create tokenList from a Token
			kTokenArray *a = new_(TokenArray, 0, OnGcStack);
			KLIB kArray_Add(kctx, a, tokenList);
			tokenList = a;
		}
		DBG_ASSERT(IS_Array(tokenList));
		kNameSpace *ns = Stmt_ns(stmt);
		SugarSyntaxVar *syn = kNameSpace_guessSyntaxFromTokenList(kctx, ns, tokenList);
		if(syn != NULL) {
			if(syn->syntaxPatternListNULL_OnList != NULL) {
				SUGAR kStmt_Message2(kctx, stmt, NULL, InfoTag, "oveloading syntax: %s%s", PSYM_t(syn->keyword));
			}
			else {
				syn->syntaxPatternListNULL_OnList = new_(TokenArray, 0, ns->NameSpaceConstList);
			}
			TokenSeq tokens = {ns, tokenList, 0, kArray_size(tokenList)};
			// Referred to kNameSpace_ParseSyntaxPattern in ast.h.
			kArray *patternList = syn->syntaxPatternListNULL_OnList;
			size_t firstPatternIdx = kArray_size(patternList);
			SUGAR kArray_AddSyntaxRule(kctx, patternList, &tokens);
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

static kbool_t desugar_defineSyntaxStatement(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("syntax"), 0, "\"syntax\" $Token*", 0, 0, NULL, NULL, Statement_syntax, NULL, NULL, },
		{ KW_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}

static kbool_t desugar_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KDEFINE_INT_CONST ClassData[] = {   // add Array as available
		{"Symbol", VirtualType_KonohaClass, (uintptr_t)CT_Symbol},
		{"Token", VirtualType_KonohaClass, (uintptr_t)CT_Token},
		{"Stmt", VirtualType_KonohaClass,  (uintptr_t)CT_Stmt},
		{"Expr", VirtualType_KonohaClass,  (uintptr_t)CT_Expr},
		{"Block", VirtualType_KonohaClass, (uintptr_t)CT_Block},
		{"Gamma", VirtualType_KonohaClass, (uintptr_t)CT_Gamma},
		{"NameSpace", VirtualType_KonohaClass, (uintptr_t)CT_NameSpace},
		{NULL},
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KonohaConst_(ClassData), false/*isOverride*/, 0);
	desugar_defineTokenFunc(kctx, ns, trace);
	desugar_defineMessageMethod(kctx, ns, trace);
	desugar_defineNameSpaceMethod(kctx, ns, trace);
	desugar_defineSyntaxStatement(kctx, ns, trace);
	return true;
}

static kbool_t desugar_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	KDEFINE_INT_CONST IntData[] = {
#define DEFINE_KEYWORD(KW) {#KW, TY_int, KW}
		DEFINE_KEYWORD(ErrTag),
		DEFINE_KEYWORD(WarnTag),
		DEFINE_KEYWORD(NoticeTag),
		DEFINE_KEYWORD(InfoTag),
		DEFINE_KEYWORD(DebugTag),
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
	KLIB kNameSpace_LoadConstData(kctx, exportNS, KonohaConst_(IntData), false/*isOverride*/, trace);
	return true;
}

KDEFINE_PACKAGE* desugar_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", "1.0");
	d.PackupNameSpace    = desugar_PackupNameSpace;
	d.ExportNameSpace   = desugar_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
