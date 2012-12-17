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
#define USE_AsciiToKonohaChar
#include <minikonoha/sugar.h>
#include <minikonoha/klib.h>
#include <minikonoha/konoha_common.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"{
#endif

#include<minikonoha/import/methoddecl.h>

#define TP_kw           KType_Symbol,     KFieldName_("keyword")
#define TP_source       KType_String,     KFieldName_("source")
#define TP_pos          KType_int,        KFieldName_("pos")
#define TP_tokens       KType_TokenArray, KFieldName_("tokens")
#define TP_begin        KType_int,        KFieldName_("begin")
#define TP_end          KType_int,        KFieldName_("end")
#define TP_message      KType_String,     KFieldName_("message")

#define TP_level         KType_int,       KFieldName_("level")
#define TP_token         KType_Token,     KFieldName_("token")
#define TP_expr          KType_Expr,      KFieldName_("expr")

// --------------------------------------------------------------------------
/* TokenFunc */

//## int String.asciiAt(int pos);
static KMETHOD String_AsciiAt(KonohaContext *kctx, KonohaStack *sfp)
{
	int ch = 0;
	size_t n = (size_t)sfp[1].intValue, len = kString_size(sfp[0].asString);
	if(n < len) {
		ch = ((unsigned char *)kString_text(sfp[0].asString))[n];
	}
	KReturnUnboxValue(ch);
}

//## @Const @Static int NameSpace.Ascii(String source);
static KMETHOD NameSpace_Ascii(KonohaContext *kctx, KonohaStack *sfp)
{
	int ch = ((unsigned char *)kString_text(sfp[1].asString))[0];
	KReturnUnboxValue(ch);
}

//## int Token.setText(symbol keyword, String s, int begin, int end);
static KMETHOD Token_setText(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *) sfp[0].asToken;
	if(IS_NOTNULL(tk)) {
		kString *text = sfp[2].asString;
		size_t len = kString_size(text);
		size_t beginIdx = (size_t)sfp[3].intValue;
		size_t endIdx   = (size_t)sfp[4].intValue;
		if(beginIdx <= endIdx && endIdx < len) {
			ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
			tk->unresolvedTokenType = keyword;
			KFieldSet(tk, tk->text, KLIB new_kString(kctx, OnField, kString_text(text) + beginIdx, endIdx - beginIdx, 0));
		}
	}
	KReturnUnboxValue(sfp[4].intValue);
}

//## int Token.setError(String s, String s, int begin, int end);
static KMETHOD Token_setError(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *) sfp[0].asToken;
	if(IS_NOTNULL(tk)) {
		SUGAR kToken_ToError(kctx, tk, ErrTag, "%s", kString_text(sfp[1].asString));
	}
	KReturnUnboxValue(sfp[4].intValue);
}

static void KBuffer_WriteToken(KonohaContext *kctx, KBuffer *wb, kToken *tk)
{
	char c = kToken_GetOpenHintChar(tk);
	if(IS_String(tk)) {
		if(c != 0) {
			KLIB KBuffer_Write(kctx, wb, &c, 1);
			KLIB KBuffer_Write(kctx, wb, kString_text(tk->text), kString_size(tk->text));
			KLIB KBuffer_Write(kctx, wb, &c, 1);
		}
		else {
			KLIB KBuffer_Write(kctx, wb, kString_text(tk->text), kString_size(tk->text));
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
		KLIB KBuffer_printf(kctx, wb, "%s%s", KSymbol_Fmt2(tk->resolvedSymbol));
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
		KBuffer wb;
		KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
		KBuffer_WriteToken(kctx, &wb, tk);
		KReturn(KLIB KBuffer_Stringfy(kctx, &wb, OnStack, StringPolicy_FreeKBuffer));
	}
}

//## Token[] NameSpace.tokenize(String s);
static KMETHOD NameSpace_Tokenize(KonohaContext *kctx, KonohaStack *sfp)
{
	INIT_GCSTACK();
	kArray *a = (kArray *)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), 0);
	KTokenSeq source = {sfp[0].asNameSpace, a};
	SUGAR KTokenSeq_Tokenize(kctx, &source, kString_text(sfp[1].asString), 0);
	KReturnWith(a, RESET_GCSTACK());
}

//## Token[] NameSpace.Preprocess(String s);
static KMETHOD NameSpace_Preprocess(KonohaContext *kctx, KonohaStack *sfp)
{
	INIT_GCSTACK();
	kArray *a = (kArray *)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), 0);
	KTokenSeq source = {sfp[0].asNameSpace, KGetParserContext(kctx)->preparedTokenList};
	KTokenSeq_Push(kctx, source);
	SUGAR KTokenSeq_Tokenize(kctx, &source, kString_text(sfp[1].asString), 0);
	KTokenSeq tokens = {source.ns, a, 0};
	tokens.TargetPolicy.ExpandingBraceGroup = true;
	SUGAR KTokenSeq_Preprocess(kctx, &tokens, NULL, &source, source.beginIdx);
	KTokenSeq_Pop(kctx, source);
	KReturnWith(a, RESET_GCSTACK());
}

//## Token Token.new(Symbol key);
static KMETHOD Token_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *)sfp[0].asToken;
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	tk->resolvedSymbol = key;
	KReturn(tk);
}

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
	if(IS_Array(tk->subTokenList)) {
		KReturnField(tk->subTokenList);
	}
}

//## boolean Token.isBeforeWhiteSpace();
static KMETHOD Token_isBeforeWhiteSpace(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kToken_Is(BeforeWhiteSpace, sfp[0].asToken));
}

static void desugar_defineTokenFunc(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Im|_Const|_Static, _F(NameSpace_Ascii), KType_String, KType_NameSpace, KKMethodName_("Ascii"), 1, TP_source,
		_Public, _F(String_AsciiAt), KType_int, KType_String, KKMethodName_("AsciiAt"), 1, TP_pos,
		_Public, _F(Token_setText), KType_int, KType_Token, KKMethodName_("SetText"), 4, TP_kw, TP_source, TP_begin, TP_end,
		_Public, _F(Token_setError), KType_int, KType_Token, KKMethodName_("SetError"), 4, TP_message, TP_source, TP_begin, TP_end,
		_Public|_Coercion|_Im|_Const, _F(Token_toString), KType_String, KType_Token, KMethodName_To(KType_String), 0,
		_Public|_Const, _F(Token_Is), KType_boolean, KType_Token, KKMethodName_("Is"), 1, TP_kw,
		_Public|_Const, _F(Token_getText), KType_String, KType_Token, KKMethodName_("GetText"), 0,
		_Public|_Const, _F(Token_getSubTokenList), KType_TokenArray, KType_Token, KKMethodName_("GetSubTokenList"), 0,
		_Public, _F(Token_isBeforeWhiteSpace), KType_boolean, KType_Token, KKMethodName_("isBeforeWhiteSpace"), 0,
		_Public, _F(Token_new), KType_Token, KType_Token, KKMethodName_("new"), 1, TP_kw,
		_Public|_Im, _F(NameSpace_Tokenize), KType_TokenArray, KType_NameSpace, KKMethodName_("Tokenize"), 1, TP_source,
		_Public|_Im, _F(NameSpace_Preprocess), KType_TokenArray, KType_NameSpace, KKMethodName_("Preprocess"), 1, TP_source,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

// --------------------------------------------------------------------------
/* CompilerError */

//## boolean Stmt.==(Stmt rhs);
static KMETHOD Stmt_opEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *self  = sfp[0].asStmt;
	kStmt *other = sfp[1].asStmt;
	KReturnUnboxValue(self == other);
}

//## Expr Stmt.message(int error, String msg);
static KMETHOD Stmt_Message(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt   *stmt  = sfp[0].asStmt;
	kinfotag_t level = (kinfotag_t)sfp[1].intValue;
	kString *msg   = sfp[2].asString;
	KReturn(SUGAR kStmt_Message2(kctx, stmt, NULL, level, "%s", kString_text(msg)));
}

//## Expr Stmt.message(int error, Token tk, String msg);
static KMETHOD StmtToken_Message(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt   *stmt  = sfp[0].asStmt;
	kinfotag_t level = (kinfotag_t)sfp[1].intValue;
	kString *msg   = sfp[3].asString;
	KReturn(SUGAR kStmt_Message2(kctx, stmt, sfp[2].asToken, level, "%s", kString_text(msg)));
}

//## Expr Stmt.message(int error, Expr expr, String msg);
static KMETHOD StmtExpr_Message(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt   *stmt  = sfp[0].asStmt;
	kinfotag_t level = (kinfotag_t)sfp[1].intValue;
	kString *msg   = sfp[3].asString;
	KReturn(SUGAR kStmt_Message2(kctx, stmt, sfp[2].asToken, level, "%s", kString_text(msg)));
}

static void desugar_defineMessageMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Stmt_Message), KType_Expr, KType_Stmt, KKMethodName_("message"), 2, TP_level, TP_message,
		_Public, _F(StmtToken_Message), KType_Expr, KType_Stmt, KKMethodName_("message"), 3, TP_level, TP_token, TP_message,
		_Public, _F(StmtExpr_Message), KType_Expr, KType_Stmt, KKMethodName_("message"), 3, TP_level, TP_expr, TP_message,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

// --------------------------------------------------------------------------
/* Block */

//## Block new(NameSpace ns)
static KMETHOD Block_new(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(new_(Block, sfp[1].asNameSpace, OnStack));
}

//## Block Stmt.newBlock(Token[] tokenList, int beginIdx, endIdx);
static KMETHOD Stmt_newBlock(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt    *stmt = sfp[0].asStmt;
	kArray   *tokenList = sfp[1].asArray;
	int beginIdx = (int)sfp[2].intValue;
	int endIdx   = (int)sfp[3].intValue;
	KTokenSeq source = {Stmt_ns(stmt), tokenList, beginIdx, endIdx};
//	SUGAR KTokenSeq_Tokenize(kctx, &source, kString_text(macro), 0);
	KReturn(SUGAR new_kBlock(kctx, stmt, NULL/*nameSpace*/, &source));
}

//## boolean Block.TypeCheckAll(Gamma gma);
static KMETHOD Block_TypeCheckAll(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(SUGAR kBlock_TypeCheckAll(kctx, sfp[0].asBlock, sfp[1].asGamma));
}

//## Array[Stmt] Block.GetNodeList();
static KMETHOD Block_GetNodeList(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(sfp[0].asBlock->NodeList);
}

//## Stmt Block.GetParentStmt();
static KMETHOD Block_GetParentStmt(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(sfp[0].asBlock->parentStmtNULL);
}

#define TP_gma       KType_Gamma, KFieldName_("gma")
#define KType_StmtArray (KClass_p0(kctx, KClass_Array, KType_Stmt)->typeId)

static void desugar_defineBlockMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const, _F(Block_new), KType_Block, KType_Block, KKMethodName_("new"), 1, KType_NameSpace, KFieldName_("namespace"),
		_Public, _F(Block_TypeCheckAll), KType_boolean, KType_Block, KKMethodName_("TypeCheckAll"), 1, TP_gma,
		_Public, _F(Stmt_newBlock), KType_Block, KType_Stmt, KKMethodName_("newBlock"), 3, TP_tokens, TP_begin, TP_end,
		_Public, _F(Block_GetNodeList), KType_StmtArray, KType_Block, KKMethodName_("GetNodeList"), 0,
		_Public, _F(Block_GetParentStmt), KType_Stmt, KType_Block, KKMethodName_("GetParentStmt"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

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
//	KTokenSeq source = {sfp[0].asNameSpace, KGetParserContext(kctx)->preparedTokenList};
//	KTokenSeq_Push(kctx, source);
//	SUGAR KTokenSeq_Tokenize(kctx, &source, kString_text(sfp[2].asString), 0);
//	KTokenSeq tokens = {source.ns, source.tokenList, source.endIdx};
//	//tokens.TargetPolicy.ExpandingBraceGroup = (flag == 1);
//	SUGAR KTokenSeq_Preprocess(kctx, &tokens, NULL, &source, source.beginIdx);
//	ksymbol_t kw   = (ksymbol_t)sfp[1].intValue;
//	KSyntax *syn = KSyntax_(sfp[0].asNameSpace, kw);
//	int ret = 0;
//	ret = SUGAR kStmt_MatchPattern(kctx, KNULL(Stmt), syn, -1, tokens.tokenList, tokens.beginIdx, tokens.endIdx, &ret/*dummy*/);
//	KTokenSeq_Pop(kctx, source);
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

//## Token Stmt.getToken(symbol key, Token def);
static KMETHOD Stmt_getToken(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt   = sfp[0].asStmt;
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	kToken *def   = sfp[2].asToken;
	KReturn(SUGAR kStmt_GetToken(kctx, stmt, key, def));
}

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
//	KSyntax *syn = KSyntax_(kStmt_ns(stmt), kw);
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
//	KReturnUnboxValue(sfp[0].asStmt->node);
//}
//
////## void Stmt.setBuild(int buildid);
//static KMETHOD Stmt_setBuild(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmtVar *stmt = (kStmtVar *) sfp[0].asStmt;
//	stmt->node = sfp[1].intValue;
//}

//## boolean Stmt.TypeCheckExpr(symbol key, Gamma gma, Object type, int policy);
static KMETHOD Stmt_TypeCheckExpr(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	ktypeattr_t typeId = kObject_typeId(sfp[3].asObject);
	KReturnUnboxValue(SUGAR kStmt_TypeCheckByName(kctx, sfp[0].asStmt, key, sfp[2].asGamma, typeId, sfp[4].intValue));
}

////## Expr Stmt.newExpr(Token[] tokenList, int s, int e);
//static KMETHOD Stmt_newExpr(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt *stmt  = sfp[0].asStmt;
//	kArray *tokenList  = sfp[1].asArray;
//	int s = sfp[2].intValue, e = sfp[3].intValue;
//	KReturn(SUGAR kStmt_ParseExpr(kctx, stmt, tokenList, s, e, NULL));
//}

//## Expr Stmt.newUntypedCallStyleExpr(Token token);
static KMETHOD Stmt_newUntypedCallStyleExpr1(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt   = sfp[0].asStmt;
	kToken *token = sfp[1].asToken;
	KSyntax *syn = KSyntax_(Stmt_ns(stmt), KSymbol_ExprMethodCall);
	KReturn(SUGAR new_UntypedCallStyleExpr(kctx, syn, 1, token));
}

//## Expr Stmt.newUntypedCallStyleExpr(Token token, Expr expr1);
static KMETHOD Stmt_newUntypedCallStyleExpr2(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt   = sfp[0].asStmt;
	kToken *token = sfp[1].asToken;
	kExpr *expr1  = sfp[2].asExpr;
	KSyntax *syn = KSyntax_(Stmt_ns(stmt), KSymbol_ExprMethodCall);
	KReturn(SUGAR new_UntypedCallStyleExpr(kctx, syn, 2, token, expr1));
}

//## Expr Stmt.newUntypedCallStyleExpr(Token token, Expr expr1, Expr expr2);
static KMETHOD Stmt_newUntypedCallStyleExpr3(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt   = sfp[0].asStmt;
	kToken *token = sfp[1].asToken;
	kExpr *expr1  = sfp[2].asExpr;
	kExpr *expr2  = sfp[3].asExpr;
	KSyntax *syn = KSyntax_(Stmt_ns(stmt), KSymbol_ExprMethodCall);
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
	KSyntax *syn = KSyntax_(Stmt_ns(stmt), KSymbol_ExprMethodCall);
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
	KSyntax *syn = KSyntax_(Stmt_ns(stmt), KSymbol_ExprMethodCall);
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
	KSyntax *syn = KSyntax_(Stmt_ns(stmt), KSymbol_ExprMethodCall);
	KReturn(SUGAR new_UntypedCallStyleExpr(kctx, syn, 6, token, expr1, expr2, expr3, expr4, expr5));
}

//## Expr Stmt.newUntypedCallStyleExpr(Token token, Expr expr1, Expr expr2, Expr expr3, Expr expr4, Expr expr5, Expr expr6);
static KMETHOD Stmt_newUntypedCallStyleExpr7(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt   = sfp[0].asStmt;
	kToken *token = sfp[1].asToken;
	kExpr *expr1  = sfp[2].asExpr;
	kExpr *expr2  = sfp[3].asExpr;
	kExpr *expr3  = sfp[4].asExpr;
	kExpr *expr4  = sfp[5].asExpr;
	kExpr *expr5  = sfp[6].asExpr;
	kExpr *expr6 = sfp[7].asExpr;
	KSyntax *syn = KSyntax_(Stmt_ns(stmt), KSymbol_ExprMethodCall);
	KReturn(SUGAR new_UntypedCallStyleExpr(kctx, syn, 7, token, expr1, expr2, expr3, expr4, expr5, expr6));
}

//## void Stmt.setType(int type);
static KMETHOD Stmt_setType(KonohaContext *kctx, KonohaStack *sfp)
{
	Stmt_typed(sfp[0].asStmt, sfp[1].intValue);
	KReturnVoid();
}

//## Token[] Stmt.getTokenList(symbol keyword, Token[] def);
static KMETHOD Stmt_getTokenList(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt    *stmt    = sfp[0].asStmt;
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	kArray   *def     = sfp[2].asArray;
	kTokenArray *tokenList = (kTokenArray *)kStmt_GetObject(kctx, stmt, keyword, def);
	kTokenArray *ret;
	if(!IS_Array(tokenList)) {
		ret = new_(TokenArray, 0, OnStack);
		KLIB kArray_Add(kctx, ret, tokenList);
	}
	else {
		ret = tokenList;
	}
	KReturn(ret);
}

//## void Stmt.setExpr(symbol key, Expr expr);
//## void Stmt.setBlock(symbol key, Block block);
static KMETHOD Stmt_setObject(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt   = sfp[0].asStmt;
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	kObject *obj  = sfp[2].asObject;
	kStmt_SetObject(kctx, stmt, key, obj);
	KReturnVoid();
}

//## Block Stmt.getBlock(symbol key);
static KMETHOD Stmt_getObject(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt   = sfp[0].asStmt;
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	KReturn(kStmt_GetObject(kctx, stmt, key, K_NULL));
}

//## Block Stmt.getParentBlock();
static KMETHOD Stmt_getParentBlock(KonohaContext *kctx, KonohaStack *sfp)
{	
	kStmt *stmt   = sfp[0].asStmt;
	KReturn(stmt->parentBlockNULL);
}

static kStmt* kStmt_LookupStmtNULL(KonohaContext *kctx, kStmt *stmt, ksymbol_t keyword)
{
	int i;
	kArray *bka = stmt->parentBlockNULL->NodeList;
	kStmt *prevTargetStmt = NULL;
	for(i = 0; kArray_size(bka); i++) {
		kStmt *s = bka->StmtItems[i];
		if(s == stmt) {
			return prevTargetStmt ? prevTargetStmt : NULL;
		}
		if(s->syn == NULL) continue;  // this is done
		prevTargetStmt = (s->syn->keyword == keyword) ? s : NULL;
	}
	return NULL;
}

//## Stmt.LookupStmt(keyword key)
static KMETHOD Stmt_LookupStmt(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt *stmt = sfp[0].asStmt;
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	kStmt *ret = kStmt_LookupStmtNULL(kctx, stmt, keyword);
	KReturn(ret ? ret : K_NULL);
}

//## void Stmt.done();
static KMETHOD Stmt_done(KonohaContext *kctx, KonohaStack *sfp)
{
	kStmt_done(kctx, sfp[0].asStmt);
	KReturnVoid();
}

//## boolean Stmt.keywordIs(Symbol keyword);
static KMETHOD Stmt_keywordIs(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	KReturnUnboxValue(sfp[0].asStmt->syn->keyword == keyword);
}

//## boolean Stmt.getNameSpace();
static KMETHOD Stmt_getNameSpace(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(Stmt_ns(sfp[0].asStmt));
}

#define TP_type KType_Object, KFieldName_("type")
#define TP_pol  KType_int, KFieldName_("policy")
#define TP_ArgExpr(n) KType_Expr, KFieldName_("expr" #n)

static void desugar_defineStmtMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	int FN_key = KFieldName_("key"), FN_defval = KFieldName_("defval");
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Stmt_opEQ), KType_boolean, KType_Stmt, KKMethodName_("=="), 1, KType_Stmt, KFieldName_("other"),
		_Public, _F(Stmt_TypeCheckExpr), KType_boolean, KType_Stmt, KKMethodName_("TypeCheckExpr"), 4, TP_kw, TP_gma, TP_type, TP_pol,
//		_Public, _F(Stmt_newExpr), KType_Expr, KType_Stmt, KKMethodName_("newExpr"), 3, TP_tokens, TP_begin, TP_end,
		_Public, _F(Stmt_setType), KType_void, KType_Stmt, KKMethodName_("setType"), 1, TP_type,
		_Public, _F(Stmt_LookupStmt), KType_Stmt, KType_Stmt, KKMethodName_("lookupStmt"), 1, TP_kw,
//////		_Public, _F(kStmt_Message2arsedExpr), KType_Expr, KType_Stmt, KKMethodName_("parseExpr"), 3, KType_TokenArray, FN_tokenList, KType_int, FN_s, KType_int, FN_e,
		_Public, _F(Stmt_newUntypedCallStyleExpr1), KType_Expr, KType_Stmt, KKMethodName_("newUntypedCallStyleExpr"), 1, TP_token,
		_Public, _F(Stmt_newUntypedCallStyleExpr2), KType_Expr, KType_Stmt, KKMethodName_("newUntypedCallStyleExpr"), 2, TP_token, TP_ArgExpr(1),
		_Public, _F(Stmt_newUntypedCallStyleExpr3), KType_Expr, KType_Stmt, KKMethodName_("newUntypedCallStyleExpr"), 3, TP_token, TP_ArgExpr(1), TP_ArgExpr(2),
		_Public, _F(Stmt_newUntypedCallStyleExpr4), KType_Expr, KType_Stmt, KKMethodName_("newUntypedCallStyleExpr"), 4, TP_token, TP_ArgExpr(1), TP_ArgExpr(2), TP_ArgExpr(3),
		_Public, _F(Stmt_newUntypedCallStyleExpr5), KType_Expr, KType_Stmt, KKMethodName_("newUntypedCallStyleExpr"), 5, TP_token, TP_ArgExpr(1), TP_ArgExpr(2), TP_ArgExpr(3), TP_ArgExpr(4),
		_Public, _F(Stmt_newUntypedCallStyleExpr6), KType_Expr, KType_Stmt, KKMethodName_("newUntypedCallStyleExpr"), 6, TP_token, TP_ArgExpr(1), TP_ArgExpr(2), TP_ArgExpr(3), TP_ArgExpr(4), TP_ArgExpr(5),
		_Public, _F(Stmt_newUntypedCallStyleExpr7), KType_Expr, KType_Stmt, KKMethodName_("newUntypedCallStyleExpr"), 7, TP_token, TP_ArgExpr(1), TP_ArgExpr(2), TP_ArgExpr(3), TP_ArgExpr(4), TP_ArgExpr(5), TP_ArgExpr(6),
////		_Public, _F(Stmt_rightJoinExpr), KType_Expr, KType_Stmt, KKMethodName_("rightJoinExpr"), 4, KType_Expr, FN_expr, KType_TokenArray, FN_tokenList, KType_int, FN_s, KType_int, FN_e,
		_Public, _F(Stmt_setObject), KType_void, KType_Stmt, KKMethodName_("setExpr"), 2, KType_Symbol, FN_key, KType_Expr, KFieldName_("expr"),
		_Public, _F(Stmt_setObject), KType_void, KType_Stmt, KKMethodName_("setBlock"), 2, KType_Symbol, FN_key, KType_Block, KFieldName_("block"),
		_Public, _F(Stmt_getObject), KType_Block, KType_Stmt, KKMethodName_("getBlock"), 1, KType_Symbol, FN_key,
		_Public, _F(Stmt_getParentBlock), KType_Block, KType_Stmt, KKMethodName_("getParentBlock"), 0,
		_Public, _F(Stmt_getTokenList), KType_TokenArray, KType_Stmt, KKMethodName_("getTokenList"), 2, KType_Symbol, FN_key, KType_TokenArray, FN_defval,
		_Public, _F(Stmt_getToken), KType_Token, KType_Stmt, KKMethodName_("getToken"), 2, KType_Symbol, FN_key, KType_Token, FN_defval,
		_Public, _F(Stmt_done), KType_void, KType_Stmt, KKMethodName_("done"), 0,
		_Public, _F(Stmt_keywordIs), KType_boolean, KType_Stmt, KKMethodName_("keywordIs"), 1, KType_Symbol, FN_key,
		_Public, _F(Stmt_getNameSpace), KType_NameSpace, KType_Stmt, KKMethodName_("getNameSpace"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}
////## Expr Stmt.newTypedCallExpr(Gamma gma, cid typeId, String methodName, Expr firstExpr, Expr secondExpr);
//static KMETHOD Stmt_newTypedCallExpr2(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt *stmt          = sfp[0].asStmt;
//	kGamma *gma          = sfp[1].asGamma;
//	ktypeattr_t cid          = (ktypeattr_t)sfp[2].intValue;
//	ksymbol_t methodName = (ksymbol_t)sfp[3].intValue;
//	kExpr *firstExpr     = sfp[4].asExpr;
//	kExpr *secondExpr    = sfp[5].asExpr;
//	kMethod *method = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, Stmt_ns(stmt), cid, methodName, 2, KMethodMatch_CamelStyle);
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

////## boolean Stmt.declType(Gamma gma, cid typeId, Expr declExpr);
//static KMETHOD Stmt_declType(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kStmt *stmt     = sfp[0].asStmt;
//	kGamma *gma     = sfp[1].asGamma;
//	ktypeattr_t cid     = (ktypeattr_t)sfp[2].intValue;
//	kExpr *declExpr = sfp[3].asExpr;
//	KReturnUnboxValue(SUGAR kStmt_DeclType(kctx, stmt, gma, cid, declExpr, NULL, NULL, &stmt));
//}
//
//
// --------------------------------------------------------------------------
/* Expr */

////## Token Expr.getTermToken();
//static KMETHOD Expr_getTermToken(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kExpr *expr = sfp[0].asExpr;
//	KReturn(expr->TermToken);
//}
//
////## Expr Expr.setConstValue(Object value);
//static KMETHOD Expr_setConstValue(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kExpr *expr = sfp[0].asExpr;
//	KClass *ct = kObject_class(sfp[1].asObject);
//	if(KClass_Is(UnboxType, (ct)) {
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
//	knode_t build  = (knode_t)sfp[2].intValue;
//	ktypeattr_t cid    = (ktypeattr_t)sfp[3].intValue;
//	intptr_t index = sfp[4].unboxValue;
//	KReturn(SUGAR kExpr_SetVariable(kctx, expr, gma, build, cid, index));
//}
//
////## Expr Expr.new(Gamma gma, int build, cid typeid, int index);
//static KMETHOD Expr_newVariableExpr(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kGamma *gma    = sfp[1].asGamma;
//	knode_t build  = (knode_t)sfp[2].intValue;
//	ktypeattr_t cid    = (ktypeattr_t)sfp[3].intValue;
//	intptr_t index = sfp[4].unboxValue;
//	KReturn(new_VariableExpr(kctx, gma, build, cid, index));
//}

//## Expr Expr.new(Object value);
static KMETHOD Expr_new(KonohaContext *kctx, KonohaStack *sfp)
{
	KClass *ct = kObject_class(sfp[1].asObject);
	if(KClass_Is(UnboxType, ct)) {
		KReturn(new_UnboxConstValueExpr(kctx, ct->typeId, sfp[1].unboxValue));
	}
	KReturn(new_ConstValueExpr(kctx, ct, sfp[1].asObject));
}

////## void Expr.setType(int build, cid typeid);
//static KMETHOD Expr_setType(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kExprVar *expr = (kExprVar *)sfp[0].asExpr;
//	knode_t build  = (knode_t)sfp[1].intValue;
//	ktypeattr_t cid    = (ktypeattr_t)sfp[2].intValue;
//	expr->node = build;
//	expr->attrTypeId = cid;
//	KReturnVoid();
//}
//

static void desugar_defineExprMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Expr_new), KType_Expr, KType_Expr, KKMethodName_("new"), 1, KType_Object, KFieldName_("value"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

//// --------------------------------------------------------------------------
//// AST Method
//
////## int Gamma.declareLocalVariable(cid typeId, symbol keyword);
//static KMETHOD Gamma_declareLocalVariable(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kGamma *gma       = sfp[0].asGamma;
//	ktypeattr_t cid       = (ktypeattr_t)sfp[1].intValue;
//	ksymbol_t keyword = (ksymbol_t)sfp[2].intValue;
//	KReturnUnboxValue(SUGAR kGamma_AddLocalVariable(kctx, gma, cid, keyword));
//}

//static KSyntax* get_syntax(KonohaContext *kctx, kNameSpace *ns, kString *key)
//{
//	USING_SUGAR;
//	symbol_t kw = KSymbol_s(key);
//	if(kw == KSymbol_Noname) {
//		kreportf(CritTag, "undefined keyword: %s", kString_text(key));
//	}
//	KSyntax *syn = KSyntax_(ks, kw);
//	if(syn == NULL) {
//		kreportf(CritTag, "undefined syntax: %s", kString_text(key));
//	}
//	return syn;
//}

////## Expr Token.printSyntaxError();
//static KMETHOD Token_printSyntaxError(KonohaContext *kctx, KonohaStack *sfp)
//{
//	USING_SUGAR;
//	kToken *tk  = sfp[0].asToken;
//	if(IS_String(tk->text)) {
//		SUGAR p(kctx, ErrTag, tk->uline, tk->lpos, "syntax error: %s", kString_text(tk->text));
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
//	kExprVar *expr = /*G*/new_(ExprVar, KSyntax_(Stmt_ns(stmt), tk->keyword));
//	KFieldSet(expr, expr->tk, tk);
//	KFieldSet(expr, expr->NodeList, new_(Array, 8));
//	KReturn(expr);
//}

////## Expr Stmt.addExprParam(Token tk);
//static KMETHOD Stmt_AddExprParam(KonohaContext *kctx, KonohaStack *sfp)
//{
//	USING_SUGAR;
//	kStmt *stmt  = sfp[0].asStmt;
//	kExpr *expr  = sfp[1].asExpr;
//	kToken *tk     = sfp[2].asToken;
//	if(tk->tt != KSymbol_ParenthesisGroup || tk->tt != KSymbol_BracketGroup) {
//		SUGAR p(kctx, WarnTag, tk->uline, tk->lpos, "not parameter token");
//		kObject_Set(NullObject, expr, 1);
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
//	if(IS_NULL(o) && IS_Array(expr->NodeList)) {
//		kObject_Set(NullObject, expr, 1);
//	}
//	if(IS_NOTNULL(expr)) {
//		KLIB kArray_Add(kctx, expr->NodeList, o);
//	}
//	KReturn(expr);
//}

// --------------------------------------------------------------------------


//static kbool_t desugar_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
//{
//	KClass *cSymbol = defineSymbolClass(kctx, ns, trace);
//	KDEFINE_INT_CONST ClassData[] = {   // add Array as available
//		{"Token", VirtualType_KClass, (uintptr_t)KClass_Token},
//		{"Stmt", VirtualType_KClass,  (uintptr_t)KClass_Stmt},
//		{"Expr", VirtualType_KClass,  (uintptr_t)KClass_Expr},
//		{"Block", VirtualType_KClass, (uintptr_t)KClass_Block},
//		{"Gamma", VirtualType_KClass, (uintptr_t)KClass_Gamma},
//		{"NameSpace", VirtualType_KClass, (uintptr_t)KClass_NameSpace},
//		{"symbol", VirtualType_KClass, (uintptr_t)cSymbol},
//		{NULL},
//	};
//	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(ClassData), 0);
//
//	int KType_Symbol = cSymbol->typeId;
//	desugar_defineTokenFunc(kctx, ns, KType_Symbol, trace);
//	desugar_defineMessageMethod(kctx, ns, KType_Symbol, trace);
//
//
//	int FN_buildid = KFieldName_("buildid"), FN_key = KFieldName_("key"), FN_defval = KFieldName_("defval");
//	int FN_typeid = KFieldName_("typeid"), FN_gma = KFieldName_("gma"), FN_pol = KFieldName_("pol");
//	int FN_msg = KFieldName_("msg");
//	int FN_x = KFieldName_("x");
//	int FN_tokenList = KFieldName_("tokens"), FN_s = KFieldName_("s"), FN_e = KFieldName_("e");
//	int FN_expr = KFieldName_("expr");
//
//	/* Array[String] */
//	kparamtype_t P_StringArray[] = {{KType_String}};
//	int KType_StringArray = (KLIB KClass_Generics(kctx, KClass_Array, KType_void, 1, P_StringArray))->typeId;
//
//	ktypeattr_t KType_Symbol = cSymbol->typeId;
//	ktypeattr_t KType_cid = ccid->typeId;
//
//	KDEFINE_METHOD MethodData[] = {
//		/* Token */
//		_Public, _F(Token_new), KType_Token, KType_Token, KKMethodName_("new"), 1, KType_Symbol, FN_key,
//		_Public, _F(Token_setUnresolvedTokenType),  KType_void, KType_Token, KKMethodName_("setUnresolvedTokenType"),  1, KType_Symbol, FN_x,
//		_Public, _F(Token_setText),  KType_void, KType_Token, KKMethodName_("setText"),  1, KType_String, FN_x,
//		_Public, _F(Token_setSubArray), KType_void, KType_Token, KKMethodName_("setSubArray"), 1, KType_StringArray, FN_x,
////		_Public, _F(Token_isTypeName), KType_boolean, KType_Token, KKMethodName_("isTypeName"), 0,
//		_Public, _F(Token_isParenthesis), KType_boolean, KType_Token, KKMethodName_("isParenthesis"), 0,
//		_Public, _F(Token_newUntypedExpr), KType_Expr, KType_Token, KKMethodName_("newUntypedExpr"), 0,
//
//		/* Stmt */
//		_Public, _F(Stmt_getBuild), KType_int, KType_Stmt,  KKMethodName_("getBuild"), 0,
//		_Public, _F(Stmt_setBuild), KType_void, KType_Stmt, KKMethodName_("setBuild"), 1, KType_int, FN_buildid,
//		_Public, _F(Stmt_getBlock), KType_Block, KType_Stmt, KKMethodName_("getBlock"), 2, KType_Symbol, FN_key, KType_Block, FN_defval,
//		_Public, _F(Stmt_getToken), KType_Token, KType_Stmt, KKMethodName_("getToken"), 2, KType_Symbol, FN_key, KType_Token, FN_defval,
//		_Public, _F(Stmt_getExpr), KType_Expr, KType_Stmt, KKMethodName_("getExpr"), 2, KType_Symbol, FN_key, KType_Expr, FN_defval,
//		_Public, _F(Stmt_TypeCheckExpr), KType_boolean, KType_Stmt, KKMethodName_("TypeCheckExpr"), 3, KType_Symbol, FN_key, KType_Gamma, FN_gma, KType_cid, FN_typeid,
//		_Public, _F(Stmt_TypeCheckExprPol), KType_boolean, KType_Stmt, KKMethodName_("TypeCheckExpr"), 4, KType_Symbol, FN_key, KType_Gamma, FN_gma, KType_cid, FN_typeid, KType_int, FN_pol,
//		_Public, _F(Stmt_Message2rintError), KType_Expr, KType_Stmt, KKMethodName_("printError"), 1, KType_String, FN_msg,
//
//		_Public, _F(Stmt_newExpr), KType_Expr, KType_Stmt, KKMethodName_("newExpr"), 3, KType_TokenArray, FN_tokenList, KType_int, FN_s, KType_int, FN_e,
//		_Public, _F(Stmt_setType), KType_void, KType_Stmt, KKMethodName_("setType"), 1, KType_int, FN_x,
////		_Public, _F(kStmt_Message2arsedExpr), KType_Expr, KType_Stmt, KKMethodName_("parseExpr"), 3, KType_TokenArray, FN_tokenList, KType_int, FN_s, KType_int, FN_e,
//		_Public, _F(Stmt_newUntypedCallStyleExpr2), KType_Expr, KType_Stmt, KKMethodName_("newUntypedCallStyleExpr"), 2, KType_Token, KFieldName_("token"), KType_Expr, KFieldName_("expr1"),
//		_Public, _F(Stmt_newUntypedCallStyleExpr3), KType_Expr, KType_Stmt, KKMethodName_("newUntypedCallStyleExpr"), 3, KType_Token, KFieldName_("token"), KType_Expr, KFieldName_("expr1"), KType_Expr, KFieldName_("expr2"),
//		_Public, _F(Stmt_newUntypedCallStyleExpr4), KType_Expr, KType_Stmt, KKMethodName_("newUntypedCallStyleExpr"), 4, KType_Token, KFieldName_("token"), KType_Expr, KFieldName_("expr1"), KType_Expr, KFieldName_("expr2"), KType_Expr, KFieldName_("expr3"),
//		_Public, _F(Stmt_newUntypedCallStyleExpr5), KType_Expr, KType_Stmt, KKMethodName_("newUntypedCallStyleExpr"), 5, KType_Token, KFieldName_("token"), KType_Expr, KFieldName_("expr1"), KType_Expr, KFieldName_("expr2"), KType_Expr, KFieldName_("expr3"), KType_Expr, KFieldName_("expr4"),
//		_Public, _F(Stmt_newUntypedCallStyleExpr6), KType_Expr, KType_Stmt, KKMethodName_("newUntypedCallStyleExpr"), 6, KType_Token, KFieldName_("token"), KType_Expr, KFieldName_("expr1"), KType_Expr, KFieldName_("expr2"), KType_Expr, KFieldName_("expr3"), KType_Expr, KFieldName_("expr4"), KType_Expr, KFieldName_("expr5"),
//		_Public, _F(Stmt_newTypedCallExpr1), KType_Expr, KType_Stmt, KKMethodName_("newTypedCallExpr"), 4, KType_Gamma, FN_gma, KType_cid, FN_typeid, KType_Symbol, KFieldName_("methodName"), KType_Expr, KFieldName_("firstExpr"),
//		_Public, _F(Stmt_newTypedCallExpr2), KType_Expr, KType_Stmt, KKMethodName_("newTypedCallExpr"), 5, KType_Gamma, FN_gma, KType_cid, FN_typeid, KType_Symbol, KFieldName_("methodName"), KType_Expr, KFieldName_("firstExpr"), KType_Expr, KFieldName_("secondExpr"),
//		_Public, _F(Stmt_rightJoinExpr), KType_Expr, KType_Stmt, KKMethodName_("rightJoinExpr"), 4, KType_Expr, FN_expr, KType_TokenArray, FN_tokenList, KType_int, FN_s, KType_int, FN_e,
//		_Public, _F(Stmt_getTokenList), KType_TokenArray, KType_Stmt, KKMethodName_("getTokenList"), 2, KType_Symbol, FN_key, KType_TokenArray, FN_defval,
//		_Public, _F(Stmt_declType), KType_boolean, KType_Stmt, KKMethodName_("declType"), 3, KType_Gamma, FN_gma, KType_cid, FN_typeid, KType_Expr, KFieldName_("declExpr"),
//		_Public, _F(Stmt_newBlock), KType_Block, KType_Stmt, KKMethodName_("newBlock"), 1, KType_String, KFieldName_("macro"),
//
//		/* Expr */
//		_Public, _F(Expr_getTermToken), KType_Token, KType_Expr, KKMethodName_("getTermToken"), 0,
//		_Public, _F(Expr_setConstValue), KType_Expr, KType_Expr, KKMethodName_("setConstValue"), 1, KType_Object, KFieldName_("value") | FN_COERCION,
//		_Public, _F(Expr_setVariable), KType_Expr, KType_Expr, KKMethodName_("setVariable"), 4, KType_Gamma, FN_gma, KType_int, FN_buildid, KType_cid, FN_typeid, KType_int, KFieldName_("index"),
//		_Public, _F(Expr_newVariableExpr), KType_Expr, KType_Expr, KKMethodName_("new"), 4, KType_Gamma, FN_gma, KType_int, FN_buildid, KType_cid, FN_typeid, KType_int, KFieldName_("index"),
//		_Public, _F(Expr_new), KType_Expr, KType_Expr, KKMethodName_("new"), 1, KType_Object, KFieldName_("value") | FN_COERCION,
//		_Public, _F(Expr_setType), KType_void, KType_Expr, KKMethodName_("setType"), 2, KType_int, FN_buildid, KType_cid, FN_typeid,
//		_Public, _F(kExpr_AddExpr), KType_Expr, KType_Expr, KKMethodName_("addExpr"), 1, KType_Expr, FN_expr,
//
//		/* Gamma */
//		_Public, _F(Gamma_declareLocalVariable), KType_int, KType_Gamma, KKMethodName_("declareLocalVariable"), 2, KType_cid, FN_typeid, KType_Symbol, FN_key,
//		DEND,
//	};
//	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
//	LoadNameSpaceMethodData(kctx, ns, KType_Symbol, trace);
//	desuga_defineSyntaxStatement(kctx, ns, ns, trace);
//	return true;
//}


// --------------------------------------------------------------------------
/* NameSpace */

static kbool_t KSyntax_hasSyntaxPatternList(KSyntax *syn)
{
	while(syn != NULL) {
		if(syn->syntaxPatternListNULL != NULL) return true;
		syn = syn->parentSyntaxNULL;
	}
	return false;
}

static kbool_t KSyntax_hasSugarFunc(KSyntax *syn, int index)
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
	KSyntax* syn = KSyntax_(sfp[0].asNameSpace, keyword);
	KReturnUnboxValue(syn != NULL);
}

//## boolean NameSpace.definedLiteral(symbol keyword);
static KMETHOD NameSpace_DefinedLiteral(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	KSyntax* syn = KSyntax_(sfp[0].asNameSpace, keyword);
	KReturnUnboxValue(KSyntax_hasSugarFunc(syn, SugarFunc_TokenFunc));
}

//## boolean NameSpace.definedStatement(symbol keyword);
static KMETHOD NameSpace_DefinedStatement(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	KSyntax* syn = KSyntax_(sfp[0].asNameSpace, keyword);
	KReturnUnboxValue(KSyntax_hasSyntaxPatternList(syn) && KSyntax_hasSugarFunc(syn, SugarFunc_Statement));
}

//## boolean NameSpace.definedExpression(symbol keyword);
static KMETHOD NameSpace_DefinedExpression(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	KSyntax* syn = KSyntax_(sfp[0].asNameSpace, keyword);
	KReturnUnboxValue(syn != NULL && (syn->precedence_op2 > 0 || syn->precedence_op1 > 0));
}

//## boolean NameSpace.definedBinaryOperator(symbol keyword);
static KMETHOD NameSpace_DefinedBinaryOperator(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	KSyntax* syn = KSyntax_(sfp[0].asNameSpace, keyword);
	KReturnUnboxValue(syn != NULL && (syn->precedence_op2 > 0));
}

//## void NameSpace.AddTokenizer(symbol keyword, int konohaChar, Func f);
static KMETHOD NameSpace_AddTokenizer(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	int konohaChar = AsciiToKonohaChar((int)sfp[2].intValue);
	SUGAR kNameSpace_SetTokenFunc(kctx, sfp[0].asNameSpace, keyword, konohaChar, sfp[3].asFunc);
}

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
	int FN_func = KSymbol_("func");
	/* Func[Int, Token, String] */
	kparamtype_t P_FuncTokenize[] = {{KType_Token}, {KType_String}};
	int KType_FuncToken = (KLIB KClass_Generics(kctx, KClass_Func, KType_int, 2, P_FuncTokenize))->typeId;
	/* Func[Int, Stmt, Int, Token[], Int, Int] */
	kparamtype_t P_FuncPatternMatch[] = {{KType_Stmt}, {KType_int}, {KType_TokenArray}, {KType_int}, {KType_int}};
	int KType_FuncPatternMatch = (KLIB KClass_Generics(kctx, KClass_Func, KType_int, 5, P_FuncPatternMatch))->typeId;
	/* Func[Expr, Stmt, Token[], Int, Int, Int] */
	kparamtype_t P_FuncExpression[] = {{KType_Stmt}, {KType_TokenArray}, {KType_int}, {KType_int}, {KType_int}};
	int KType_FuncExpression = (KLIB KClass_Generics(kctx, KClass_Func, KType_Expr, 5, P_FuncExpression))->typeId;
	/* Func[Boolean, Stmt, Gamma] */
	kparamtype_t P_FuncStatement[] = {{KType_Stmt}, {KType_Gamma}};
	int KType_FuncStatement = (KLIB KClass_Generics(kctx, KClass_Func, KType_boolean, 2, P_FuncStatement))->typeId;
	/* Func[Expr, Stmt, Expr, Gamma, Int] */
	kparamtype_t P_FuncTypeCheck[] = {{KType_Stmt}, {KType_Expr}, {KType_Gamma}, {KType_int}};
	int KType_FuncTypeCheck = (KLIB KClass_Generics(kctx, KClass_Func, KType_Expr, 4, P_FuncTypeCheck))->typeId;
	//DBG_P("func=%s", KType_text(KType_FuncTypeCheck));
	KDEFINE_METHOD MethodData[] = {
		_Public|_Im, _F(NameSpace_DefinedSyntax), KType_boolean, KType_NameSpace, KKMethodName_("DefinedSyntax"), 1, TP_kw,
		_Public|_Im, _F(NameSpace_DefinedLiteral), KType_boolean, KType_NameSpace, KKMethodName_("DefinedLiteral"), 1, TP_kw,
		_Public|_Im, _F(NameSpace_DefinedStatement), KType_boolean, KType_NameSpace, KKMethodName_("DefinedStatement"), 1, TP_kw,
		_Public|_Im, _F(NameSpace_DefinedExpression), KType_boolean, KType_NameSpace, KKMethodName_("DefinedExpression"), 1, TP_kw,
		_Public|_Im, _F(NameSpace_DefinedBinaryOperator), KType_boolean, KType_NameSpace, KKMethodName_("DefinedBinaryOperator"), 1, TP_kw,
		_Public, _F(NameSpace_AddTokenizer),         KType_void, KType_NameSpace, KKMethodName_("AddTokenizer"), 3, TP_kw, KType_int, KFieldName_("kchar"), KType_FuncToken, FN_func,
		_Public, _F(NameSpace_AddPatternMatch),      KType_void, KType_NameSpace, KKMethodName_("AddPatternMatch"), 2, TP_kw, KType_FuncPatternMatch, FN_func,
		_Public, _F(NameSpace_AddExpression),        KType_void, KType_NameSpace, KKMethodName_("AddExpression"), 2, TP_kw, KType_FuncExpression, FN_func,
		_Public, _F(NameSpace_AddTopLevelStatement), KType_void, KType_NameSpace, KKMethodName_("AddTopLevelStatement"), 2, TP_kw, KType_FuncStatement, FN_func,
		_Public, _F(NameSpace_AddStatement),         KType_void, KType_NameSpace, KKMethodName_("AddStatement"), 2, TP_kw, KType_FuncStatement, FN_func,
		_Public, _F(NameSpace_AddTypeCheck),         KType_void, KType_NameSpace, KKMethodName_("AddTypeCheck"), 2, TP_kw, KType_FuncTypeCheck, FN_func,
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
	if(beginIdx+1 < endIdx && tokenList->TokenItems[beginIdx+1]->resolvedSyntaxInfo->keyword == KSymbol_TextPattern) {
		const char *t = kString_text(tokenList->TokenItems[beginIdx+1]->text);
		if(isalpha(t[0]) || t[0] < 0 /* multibytes char */) {
			return 1;
		}
	}
	return 0;
}

static KSyntaxVar *kNameSpace_guessSyntaxFromTokenList(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList)
{
	int beginIdx = 0, endIdx = kArray_size(tokenList);
	if(beginIdx < endIdx) {
		ksymbol_t keyword = tokenList->TokenItems[beginIdx]->resolvedSyntaxInfo->keyword;
		if(keyword == KSymbol_TextPattern) {
			ksymbol_t kw;
			if(isSubKeyword(kctx, tokenList, beginIdx, endIdx)) {
				char buf[256];
				PLATAPI snprintf_i(buf, sizeof(buf), "%s_%s", kString_text(tokenList->TokenItems[beginIdx]->text), kString_text(tokenList->TokenItems[beginIdx+1]->text));
				kw = KAsciiSymbol((const char *)buf, strlen(buf), KSymbol_NewId);
			}
			else {
				kw = KAsciiSymbol(kString_text(tokenList->TokenItems[beginIdx]->text), kString_size(tokenList->TokenItems[beginIdx]->text), KSymbol_NewId);
			}
			return (KSyntaxVar *)NEWKSyntax_(ns, kw);
		}
		else if(keyword == KSymbol_DOLLAR) { // $TokenPattern
			char buf[256];
			PLATAPI snprintf_i(buf, sizeof(buf), "$%s", kString_text(tokenList->TokenItems[beginIdx+1]->text));
			ksymbol_t kw = KAsciiSymbol((const char *)buf, strlen(buf), KSymbol_NewId);
			return (KSyntaxVar *)NEWKSyntax_(ns, kw);
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

#define kToken_IsFirstPattern(tk)   (KSymbol_IsPattern(tk->resolvedSymbol) && tk->stmtEntryKey != KSymbol_ExprPattern)
static KMETHOD Statement_syntax(KonohaContext *kctx, KonohaStack *sfp)
{
	kbool_t r = 0;
	VAR_Statement(stmt, gma);
	kTokenArray *tokenList = (kTokenArray *)kStmt_GetObject(kctx, stmt, KSymbol_TokenPattern, NULL);
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
		KSyntaxVar *syn = kNameSpace_guessSyntaxFromTokenList(kctx, ns, tokenList);
		if(syn != NULL) {
			if(syn->syntaxPatternListNULL != NULL) {
				SUGAR kStmt_Message2(kctx, stmt, NULL, InfoTag, "oveloading syntax: %s%s", KSymbol_Fmt2(syn->keyword));
			}
			else {
				syn->syntaxPatternListNULL = new_(TokenArray, 0, ns->NameSpaceConstList);
			}
			KTokenSeq tokens = {ns, tokenList, 0, kArray_size(tokenList)};
			// Referred to kNameSpace_ParseSyntaxPattern in ast.h.
			kArray *patternList = syn->syntaxPatternListNULL;
			size_t firstPatternIdx = kArray_size(patternList);
			SUGAR kArray_AddSyntaxRule(kctx, patternList, &tokens);
			if(firstPatternIdx < kArray_size(patternList)) {
				kToken *firstPattern = patternList->TokenItems[firstPatternIdx];
				if(kToken_IsFirstPattern(firstPattern)) {
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
		{ KSymbol_("syntax"), 0, "\"syntax\" $Token*", 0, 0, NULL, NULL, Statement_syntax, NULL, NULL, },
		{ KSymbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}

static kbool_t desugar_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KDEFINE_INT_CONST ClassData[] = {   // add Array as available
		{"Symbol", VirtualType_KClass, (uintptr_t)KClass_Symbol},
		{"Token",  VirtualType_KClass, (uintptr_t)KClass_Token},
		{"Stmt",   VirtualType_KClass,  (uintptr_t)KClass_Stmt},
		{"Expr",   VirtualType_KClass,  (uintptr_t)KClass_Expr},
		{"Block",  VirtualType_KClass, (uintptr_t)KClass_Block},
		{"Gamma",  VirtualType_KClass, (uintptr_t)KClass_Gamma},
		{"NameSpace", VirtualType_KClass, (uintptr_t)KClass_NameSpace},
		{NULL},
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(ClassData), false/*isOverride*/, 0);
	desugar_defineTokenFunc(kctx, ns, trace);
	desugar_defineMessageMethod(kctx, ns, trace);
	desugar_defineBlockMethod(kctx, ns, trace);
	desugar_defineStmtMethod(kctx, ns, trace);
	desugar_defineExprMethod(kctx, ns, trace);
	desugar_defineNameSpaceMethod(kctx, ns, trace);
	desugar_defineSyntaxStatement(kctx, ns, trace);
	return true;
}

static kbool_t desugar_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	KDEFINE_INT_CONST IntData[] = {
#define DEFINE_KEYWORD(KW) {#KW, KType_int, KW}
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
	KLIB kNameSpace_LoadConstData(kctx, exportNS, KConst_(IntData), false/*isOverride*/, trace);
	return true;
}

KDEFINE_PACKAGE* desugar_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", K_VERSION);
	d.PackupNameSpace    = desugar_PackupNameSpace;
	d.ExportNameSpace   = desugar_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
