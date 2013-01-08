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
#define TP_level        KType_int,        KFieldName_("level")
#define TP_token        KType_Token,      KFieldName_("token")
#define TP_syntax       KType_Syntax,     KFieldName_("syntax")

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

//## int Token.setText(Symbol keyword, String s, int begin, int end);
static KMETHOD Token_SetText(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *) sfp[0].asToken;
	if(IS_NOTNULL(tk)) {
		kString *text = sfp[2].asString;
		size_t len = kString_size(text);
		size_t beginIdx = (size_t)sfp[3].intValue;
		size_t endIdx   = (size_t)sfp[4].intValue;
		if(beginIdx <= endIdx && endIdx <= len) {
			ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
			tk->tokenType = keyword;
			KFieldSet(tk, tk->text, KLIB new_kString(kctx, OnField, kString_text(text) + beginIdx, endIdx - beginIdx, 0));
		}
	}
	KReturnUnboxValue(sfp[4].intValue);
}

//## int Token.setError(String s, String s, int begin, int end);
static KMETHOD Token_SetError(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *) sfp[0].asToken;
	if(IS_NOTNULL(tk)) {
		SUGAR kToken_ToError(kctx, tk, ErrTag, "%s", kString_text(sfp[1].asString));
	}
	KReturnUnboxValue(sfp[4].intValue);
}

static void KBuffer_WriteToken(KonohaContext *kctx, KBuffer *wb, kToken *tk)
{
	char c = kToken_GetOpenChar(tk);
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
		kArray *a = tk->GroupTokenList;
		KLIB KBuffer_Write(kctx, wb, &c, 1);
		for(i = 0; i < kArray_size(a); i++) {
			KLIB KBuffer_Write(kctx, wb, " ", 1);
			KBuffer_WriteToken(kctx, wb, a->TokenItems[i]);
		}
		KLIB KBuffer_Write(kctx, wb, " ", 1);
		c = kToken_GetCloseChar(tk);
		KLIB KBuffer_Write(kctx, wb, &c, 1);
	}
	else {
		KLIB KBuffer_printf(kctx, wb, "%s%s", KSymbol_Fmt2(tk->symbol));
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
	SUGAR Tokenize(kctx, source.ns, kString_text(sfp[1].asString), 0, source.tokenList);
	KReturnWith(a, RESET_GCSTACK());
}

//## Token[] NameSpace.Preprocess(String s);
static KMETHOD NameSpace_Preprocess(KonohaContext *kctx, KonohaStack *sfp)
{
	INIT_GCSTACK();
	kArray *a = (kArray *)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), 0);
	KTokenSeq source = {sfp[0].asNameSpace, KGetParserContext(kctx)->preparedTokenList};
	KTokenSeq_Push(kctx, source);
	SUGAR Tokenize(kctx, source.ns, kString_text(sfp[1].asString), 0, source.tokenList);
	KTokenSeq tokens = {source.ns, a, 0};
	tokens.TargetPolicy.ExpandingBraceGroup = true;
	SUGAR Preprocess(kctx, source.ns, source.tokenList, source.beginIdx, tokens.endIdx, NULL, tokens.tokenList);
	KTokenSeq_Pop(kctx, source);
	KReturnWith(a, RESET_GCSTACK());
}

//## Token Token.new(Symbol key);
static KMETHOD Token_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *)sfp[0].asToken;
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	tk->symbol = key;
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
	if(IS_Array(tk->GroupTokenList)) {
		KReturnField(tk->GroupTokenList);
	}
}

//## boolean Token.isBeforeWhiteSpace();
static KMETHOD Token_isBeforeWhiteSpace(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kToken_Is(BeforeWhiteSpace, sfp[0].asToken));
}

static void Syntax_defineTokenFunc(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Im|_Const|_Static, _F(NameSpace_Ascii), KType_String, KType_NameSpace, KMethodName_("Ascii"), 1, TP_source,
		_Public, _F(String_AsciiAt), KType_int, KType_String, KMethodName_("AsciiAt"), 1, TP_pos,
		_Public, _F(Token_SetText), KType_int, KType_Token, KMethodName_("SetText"), 4, TP_kw, TP_source, TP_begin, TP_end,
		_Public, _F(Token_SetError), KType_int, KType_Token, KMethodName_("SetError"), 4, TP_message, TP_source, TP_begin, TP_end,
		_Public|_Coercion|_Im|_Const, _F(Token_toString), KType_String, KType_Token, KMethodName_To(KType_String), 0,
		_Public|_Const, _F(Token_Is), KType_boolean, KType_Token, KMethodName_("Is"), 1, TP_kw,
		_Public|_Const, _F(Token_getText), KType_String, KType_Token, KMethodName_("GetText"), 0,
		_Public|_Const, _F(Token_getSubTokenList), KType_TokenArray, KType_Token, KMethodName_("GetSubTokenList"), 0,
		_Public, _F(Token_isBeforeWhiteSpace), KType_boolean, KType_Token, KMethodName_("isBeforeWhiteSpace"), 0,
		_Public, _F(Token_new), KType_Token, KType_Token, KMethodName_("new"), 1, TP_kw,
		_Public|_Im, _F(NameSpace_Tokenize), KType_TokenArray, KType_NameSpace, KMethodName_("Tokenize"), 1, TP_source,
		_Public|_Im, _F(NameSpace_Preprocess), KType_TokenArray, KType_NameSpace, KMethodName_("Preprocess"), 1, TP_source,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

// --------------------------------------------------------------------------
/* CompilerError */

//## boolean Node.==(Node rhs);
static KMETHOD Node_opEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *self  = sfp[0].asNode;
	kNode *other = sfp[1].asNode;
	KReturnUnboxValue(self == other);
}

//## Node Node.message(int error, String msg);
static KMETHOD Node_Message(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode   *stmt  = sfp[0].asNode;
	kinfotag_t level = (kinfotag_t)sfp[1].intValue;
	kString *msg   = sfp[2].asString;
	KReturn(SUGAR MessageNode(kctx, stmt, NULL, kNode_ns(stmt), level, "%s", kString_text(msg)));
}

//## Node Node.message(int error, Token tk, String msg);
static KMETHOD NodeToken_Message(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode   *stmt  = sfp[0].asNode;
	kinfotag_t level = (kinfotag_t)sfp[1].intValue;
	kString *msg   = sfp[3].asString;
	KReturn(SUGAR MessageNode(kctx, stmt, sfp[2].asToken, kNode_ns(stmt), level, "%s", kString_text(msg)));
}

static void Syntax_defineMessageMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Node_Message), KType_Node, KType_Node, KMethodName_("message"), 2, TP_level, TP_message,
		_Public, _F(NodeToken_Message), KType_Node, KType_Node, KMethodName_("message"), 3, TP_level, TP_token, TP_message,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

// --------------------------------------------------------------------------
/* Node(Block) */

////## Node new(NameSpace ns)
//static KMETHOD Node_new(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KReturn(new_(Node, sfp[1].asNameSpace, OnStack));
//}

//## Node Node.newNode(Token[] tokenList, int beginIdx, endIdx);
static KMETHOD Node_newNode(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode    *stmt = sfp[0].asNode;
	kArray   *tokenList = sfp[1].asArray;
	int beginIdx = (int)sfp[2].intValue;
	int endIdx   = (int)sfp[3].intValue;
//	SUGAR Tokenize(kctx, &source, kString_text(macro), 0);
	//KReturn(SUGAR new_BlockNode(kctx, stmt, NULL/*nameSpace*/, &source));
	KReturn(SUGAR ParseNewNode(kctx, kNode_ns(stmt), tokenList, &beginIdx, endIdx, ParseBlockOption|ParseMetaPatternOption, NULL));
}

//## boolean Node.TypeCheckAll();
static KMETHOD Node_TypeCheckAll(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kNode_IsError(SUGAR TypeCheckBlock(kctx, sfp[0].asNode, kNode_ns(sfp[0].asNode), KClass_void)));
}

//## Array[Node] Node.GetNodeList();
static KMETHOD Node_GetNodeList(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(sfp[0].asNode->NodeList);
}

//## Node Node.GetParentNode();
static KMETHOD Node_GetParentNode(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(kNode_GetParent(kctx, sfp[0].asNode));
}

#define TP_ns       KType_NameSpace, KFieldName_("ns")
#define KType_NodeArray (KClass_p0(kctx, KClass_Array, KType_Node)->typeId)

// --------------------------------------------------------------------------
/* Pattern */

/**
int patternMatchShell(Node stmt, int nameid, Token[] tokenList, int beginIdx, int endIdx) {
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
//	SUGAR Tokenize(kctx, &source, kString_text(sfp[2].asString), 0);
//	KTokenSeq tokens = {source.ns, source.tokenList, source.endIdx};
//	//tokens.TargetPolicy.ExpandingBraceGroup = (flag == 1);
//	SUGAR Preprocess(kctx, &tokens, NULL, &source, source.beginIdx);
//	ksymbol_t kw   = (ksymbol_t)sfp[1].intValue;
//	kSyntax *syn = kSyntax_(sfp[0].asNameSpace, kw);
//	int ret = 0;
//	ret = SUGAR kNode_MatchPattern(kctx, KNULL(Node), syn, -1, tokens.tokenList, tokens.beginIdx, tokens.endIdx, &ret/*dummy*/);
//	KTokenSeq_Pop(kctx, source);
//	KReturnWith(ret != -1, RESET_GCSTACK());
//}

////## void Node.AddParsedObject(keyword nameid, Object obj);
//static KMETHOD Node_AddParsedObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNode   *stmt  = sfp[0].asNode;
//	ksymbol_t symbol = (ksymbol_t)sfp[1].intValue;
//	SUGAR kNode_AddParsedObject(kctx, stmt, symbol, sfp[2].asObject);
//	KReturnVoid();
//}

////## SmartReturn Object Node.GetParsedObject(keyword nameid);
//static KMETHOD Node_GetParsedObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNode *stmt  = sfp[0].asNode;
//	ksymbol_t symbol = (ksymbol_t)sfp[1].intValue;
//	kObject *o = SUGAR kNode_GetObject(kctx, stmt, symbol, NULL);
//	if(o != NULL) {
//
//	}
//	KReturnDefaultValue();
//}

//## Token Node.getToken(Symbol key, Token def);
static KMETHOD Node_getToken(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *stmt   = sfp[0].asNode;
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	kToken *def   = sfp[2].asToken;
	KReturn(SUGAR kNode_GetToken(kctx, stmt, key, def));
}

////## int Node.Match(Symbol key, Symbol nameid, Token[] tokenList, int beginIdx, int endIdx);
//static KMETHOD Node_Match(KonohaContext *kctx, KonohaStack *sfp)
//{
//	int ret = -1;
//	kNode   *stmt  = sfp[0].asNode;
//	ksymbol_t kw   = (ksymbol_t)sfp[1].intValue;
//	int nameid   = (ksymbol_t)sfp[2].intValue;
//	kArray *tokenList = sfp[3].asArray;
//	int beginIdx = sfp[4].intValue;
//	int endIdx = sfp[5].intValue;
//	kSyntax *syn = kSyntax_(kNode_ns(stmt), kw);
//	if(syn != NULL) {
//		int dummy = 0;
//		ret = SUGAR kNode_MatchPattern(kctx, stmt, syn, nameid, tokenList, beginIdx, endIdx, &dummy);
//	}
//	KReturnUnboxValue(ret);
//}

// --------------------------------------------------------------------------
/* Node(Stmt) */

////## int Node.getBuild();
//static KMETHOD Node_getBuild(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KReturnUnboxValue(sfp[0].asNode->node);
//}
//
////## void Node.setBuild(int buildid);
//static KMETHOD Node_SetBuild(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNodeVar *stmt = (kNodeVar *) sfp[0].asNode;
//	stmt->node = sfp[1].intValue;
//}

//## boolean Node.TypeCheckNode(Symbol key, Object type, int policy);
static KMETHOD Node_TypeCheckNode(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	KClass *type = kObject_class(sfp[2].asObject);
	int policy = sfp[3].intValue;
	KReturnUnboxValue(kNode_IsError(SUGAR TypeCheckNodeByName(kctx, sfp[0].asNode, key, kNode_ns(sfp[0].asNode), type, policy)));
}

////## Node Node.newNode(Token[] tokenList, int s, int e);
//static KMETHOD Node_newNode(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNode *stmt  = sfp[0].asNode;
//	kArray *tokenList  = sfp[1].asArray;
//	int s = sfp[2].intValue, e = sfp[3].intValue;
//	KReturn(SUGAR ParseNewNode(kctx, stmt, tokenList, s, e, NULL));
//}

//## Node Node.newUntypedCallStyleNode(Token token);
static KMETHOD Node_newUntypedCallStyleNode1(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *stmt   = sfp[0].asNode;
	kToken *token = sfp[1].asToken;
	kSyntax *syn = kSyntax_(kNode_ns(stmt), KSymbol_ParamPattern/*MethodCall*/);
	KReturn(SUGAR new_UntypedOperatorNode(kctx, syn, 1, token));
}

//## Node Node.newUntypedCallStyleNode(Token token, Node expr1);
static KMETHOD Node_newUntypedCallStyleNode2(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *stmt   = sfp[0].asNode;
	kToken *token = sfp[1].asToken;
	kNode *expr1  = sfp[2].asNode;
	kSyntax *syn = kSyntax_(kNode_ns(stmt), KSymbol_ParamPattern/*MethodCall*/);
	KReturn(SUGAR new_UntypedOperatorNode(kctx, syn, 2, token, expr1));
}

//## Node Node.newUntypedCallStyleNode(Token token, Node expr1, Node expr2);
static KMETHOD Node_newUntypedCallStyleNode3(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *stmt   = sfp[0].asNode;
	kToken *token = sfp[1].asToken;
	kNode *expr1  = sfp[2].asNode;
	kNode *expr2  = sfp[3].asNode;
	kSyntax *syn = kSyntax_(kNode_ns(stmt), KSymbol_ParamPattern/*MethodCall*/);
	KReturn(SUGAR new_UntypedOperatorNode(kctx, syn, 3, token, expr1, expr2));
}

//## Node Node.newUntypedCallStyleNode(Token token, Node expr1, Node expr2, Node expr3);
static KMETHOD Node_newUntypedCallStyleNode4(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *stmt   = sfp[0].asNode;
	kToken *token = sfp[1].asToken;
	kNode *expr1  = sfp[2].asNode;
	kNode *expr2  = sfp[3].asNode;
	kNode *expr3  = sfp[4].asNode;
	kSyntax *syn = kSyntax_(kNode_ns(stmt), KSymbol_ParamPattern/*MethodCall*/);
	KReturn(SUGAR new_UntypedOperatorNode(kctx, syn, 4, token, expr1, expr2, expr3));
}

//## Node Node.newUntypedCallStyleNode(Token token, Node expr1, Node expr2, Node expr3, Node expr4);
static KMETHOD Node_newUntypedCallStyleNode5(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *stmt   = sfp[0].asNode;
	kToken *token = sfp[1].asToken;
	kNode *expr1  = sfp[2].asNode;
	kNode *expr2  = sfp[3].asNode;
	kNode *expr3  = sfp[4].asNode;
	kNode *expr4  = sfp[5].asNode;
	kSyntax *syn = kSyntax_(kNode_ns(stmt), KSymbol_ParamPattern/*MethodCall*/);
	KReturn(SUGAR new_UntypedOperatorNode(kctx, syn, 5, token, expr1, expr2, expr3, expr4));
}

//## Node Node.newUntypedCallStyleNode(Token token, Node expr1, Node expr2, Node expr3, Node expr4, Node expr5);
static KMETHOD Node_newUntypedCallStyleNode6(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *stmt   = sfp[0].asNode;
	kToken *token = sfp[1].asToken;
	kNode *expr1  = sfp[2].asNode;
	kNode *expr2  = sfp[3].asNode;
	kNode *expr3  = sfp[4].asNode;
	kNode *expr4  = sfp[5].asNode;
	kNode *expr5  = sfp[6].asNode;
	kSyntax *syn = kSyntax_(kNode_ns(stmt), KSymbol_ParamPattern/*MethodCall*/);
	KReturn(SUGAR new_UntypedOperatorNode(kctx, syn, 6, token, expr1, expr2, expr3, expr4, expr5));
}

//## Node Node.newUntypedCallStyleNode(Token token, Node expr1, Node expr2, Node expr3, Node expr4, Node expr5, Node expr6);
static KMETHOD Node_newUntypedCallStyleNode7(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *stmt   = sfp[0].asNode;
	kToken *token = sfp[1].asToken;
	kNode *expr1  = sfp[2].asNode;
	kNode *expr2  = sfp[3].asNode;
	kNode *expr3  = sfp[4].asNode;
	kNode *expr4  = sfp[5].asNode;
	kNode *expr5  = sfp[6].asNode;
	kNode *expr6 = sfp[7].asNode;
	kSyntax *syn = kSyntax_(kNode_ns(stmt), KSymbol_ParamPattern/*MethodCall*/);
	KReturn(SUGAR new_UntypedOperatorNode(kctx, syn, 7, token, expr1, expr2, expr3, expr4, expr5, expr6));
}

//## void Node.setType(int type);
static KMETHOD Node_SetType(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode_Type(kctx, sfp[0].asNode, sfp[1].intValue, KType_void);
	KReturnVoid();
}

//## Token[] Node.getTokenList(Symbol keyword, Token[] def);
static KMETHOD Node_getTokenList(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode    *stmt    = sfp[0].asNode;
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	kArray   *def     = sfp[2].asArray;
	kTokenArray *tokenList = (kTokenArray *)kNode_GetObject(kctx, stmt, keyword, def);
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

//## void Node.setNode(Symbol key, Node node);
static KMETHOD Node_SetObject(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *stmt   = sfp[0].asNode;
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	kObject *obj  = sfp[2].asObject;
	kNode_SetObject(kctx, stmt, key, obj);
	KReturnVoid();
}

//## Node Node.getNode(Symbol key);
static KMETHOD Node_getObject(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *stmt   = sfp[0].asNode;
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	KReturn(kNode_GetObject(kctx, stmt, key, K_NULL));
}

static kNode* kNode_LookupNodeNULL(KonohaContext *kctx, kNode *stmt, ksymbol_t keyword)
{
	int i;
	kArray *bka = stmt->Parent->NodeList;
	kNode *prevTargetNode = NULL;
	for(i = 0; kArray_size(bka); i++) {
		kNode *s = bka->NodeItems[i];
		if(s == stmt) {
			return prevTargetNode ? prevTargetNode : NULL;
		}
		if(s->syn == NULL) continue;  // this is done
		prevTargetNode = (s->syn->keyword == keyword) ? s : NULL;
	}
	return NULL;
}

//## Node.LookupNode(keyword key)
static KMETHOD Node_LookupNode(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *stmt = sfp[0].asNode;
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	kNode *ret = kNode_LookupNodeNULL(kctx, stmt, keyword);
	KReturn((!kNode_IsError(ret) ? ret : K_NULLNODE));
}

//## void Node.done();
static KMETHOD Node_done(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode_Type(kctx, sfp[0].asNode, KNode_Done, KType_void);
	KReturnVoid();
}

//## boolean Node.keywordIs(Symbol keyword);
static KMETHOD Node_keywordIs(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	KReturnUnboxValue(sfp[0].asNode->syn->keyword == keyword);
}

//## boolean Node.getNameSpace();
static KMETHOD Node_getNameSpace(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(kNode_ns(sfp[0].asNode));
}

// --------------------------------------------------------------------------
/* Node(Expr) */

//## Node Node.new(Object value);
static KMETHOD Node_new(KonohaContext *kctx, KonohaStack *sfp)
{
	KClass *ct = kObject_class(sfp[1].asObject);
	if(KClass_Is(UnboxType, ct)) {
		KReturn(new_UnboxConstNode(kctx, kNode_ns(sfp[0].asNode), ct->typeId, sfp[1].unboxValue));
	}
	KReturn(new_ConstNode(kctx, kNode_ns(sfp[0].asNode), ct, sfp[1].asObject));
}

//## Token Node.getTermToken();
static KMETHOD Node_getTermToken(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *expr = sfp[0].asNode;
	KReturn(expr->TermToken);
}

//## Node Node.setConstValue(Object value);
static KMETHOD Node_setConstValue(KonohaContext *kctx, KonohaStack *sfp)
{
	kNodeVar *expr = (kNodeVar *)sfp[0].asNode;
	KClass *ct = kObject_class(sfp[1].asObject);
	if(KClass_Is(UnboxType, (ct))) {
		KReturn(SUGAR kNode_SetUnboxConst(kctx, expr, ct->typeId, sfp[1].unboxValue));
	}
	KReturn(SUGAR kNode_SetConst(kctx, expr, ct, sfp[1].asObject));
}

#define TP_type KType_Object, KFieldName_("type")
#define TP_pol  KType_int, KFieldName_("policy")
#define TP_ArgNode(n) KType_Node, KFieldName_("expr" #n)

static void Syntax_defineNodeMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	int FN_key = KFieldName_("key"), FN_defval = KFieldName_("defval");
	KDEFINE_METHOD MethodData[] = {
		/* Block */
//		_Public|_Const, _F(Node_new), KType_Node, KType_Node, KMethodName_("new"), 1, KType_NameSpace, KFieldName_("namespace"),
		_Public, _F(Node_TypeCheckAll), KType_boolean, KType_Node, KMethodName_("TypeCheckAll"), 0,
		_Public, _F(Node_newNode), KType_Node, KType_Node, KMethodName_("newNode"), 3, TP_tokens, TP_begin, TP_end,
		_Public, _F(Node_GetNodeList), KType_NodeArray, KType_Node, KMethodName_("GetNodeList"), 0,
		_Public, _F(Node_GetParentNode), KType_Node, KType_Node, KMethodName_("GetParentNode"), 0,

		_Public, _F(Node_opEQ), KType_boolean, KType_Node, KMethodName_("=="), 1, KType_Node, KFieldName_("other"),
		_Public, _F(Node_TypeCheckNode), KType_boolean, KType_Node, KMethodName_("TypeCheckNode"), 4, TP_kw, TP_ns, TP_type, TP_pol,
//		_Public, _F(Node_newNode), KType_Node, KType_Node, KMethodName_("newNode"), 3, TP_tokens, TP_begin, TP_end,
		_Public, _F(Node_SetType), KType_void, KType_Node, KMethodName_("setType"), 1, TP_type,
		_Public, _F(Node_LookupNode), KType_Node, KType_Node, KMethodName_("lookupNode"), 1, TP_kw,
//////		_Public, _F(MessageNodearsedNode), KType_Node, KType_Node, KMethodName_("parseNode"), 3, KType_TokenArray, FN_tokenList, KType_int, FN_s, KType_int, FN_e,
		_Public, _F(Node_newUntypedCallStyleNode1), KType_Node, KType_Node, KMethodName_("newUntypedCallStyleNode"), 1, TP_token,
		_Public, _F(Node_newUntypedCallStyleNode2), KType_Node, KType_Node, KMethodName_("newUntypedCallStyleNode"), 2, TP_token, TP_ArgNode(1),
		_Public, _F(Node_newUntypedCallStyleNode3), KType_Node, KType_Node, KMethodName_("newUntypedCallStyleNode"), 3, TP_token, TP_ArgNode(1), TP_ArgNode(2),
		_Public, _F(Node_newUntypedCallStyleNode4), KType_Node, KType_Node, KMethodName_("newUntypedCallStyleNode"), 4, TP_token, TP_ArgNode(1), TP_ArgNode(2), TP_ArgNode(3),
		_Public, _F(Node_newUntypedCallStyleNode5), KType_Node, KType_Node, KMethodName_("newUntypedCallStyleNode"), 5, TP_token, TP_ArgNode(1), TP_ArgNode(2), TP_ArgNode(3), TP_ArgNode(4),
		_Public, _F(Node_newUntypedCallStyleNode6), KType_Node, KType_Node, KMethodName_("newUntypedCallStyleNode"), 6, TP_token, TP_ArgNode(1), TP_ArgNode(2), TP_ArgNode(3), TP_ArgNode(4), TP_ArgNode(5),
		_Public, _F(Node_newUntypedCallStyleNode7), KType_Node, KType_Node, KMethodName_("newUntypedCallStyleNode"), 7, TP_token, TP_ArgNode(1), TP_ArgNode(2), TP_ArgNode(3), TP_ArgNode(4), TP_ArgNode(5), TP_ArgNode(6),
////		_Public, _F(Node_rightJoinNode), KType_Node, KType_Node, KMethodName_("rightJoinNode"), 4, KType_Node, FN_expr, KType_TokenArray, FN_tokenList, KType_int, FN_s, KType_int, FN_e,
		_Public, _F(Node_SetObject), KType_void, KType_Node, KMethodName_("setNode"), 2, KType_Symbol, FN_key, KType_Node, KFieldName_("node"),
		_Public, _F(Node_getObject), KType_Node, KType_Node, KMethodName_("getNode"), 1, KType_Symbol, FN_key,
		_Public, _F(Node_getTokenList), KType_TokenArray, KType_Node, KMethodName_("getTokenList"), 2, KType_Symbol, FN_key, KType_TokenArray, FN_defval,
		_Public, _F(Node_getToken), KType_Token, KType_Node, KMethodName_("getToken"), 2, KType_Symbol, FN_key, KType_Token, FN_defval,
		_Public, _F(Node_done), KType_void, KType_Node, KMethodName_("done"), 0,
		_Public, _F(Node_keywordIs), KType_boolean, KType_Node, KMethodName_("keywordIs"), 1, KType_Symbol, FN_key,
		_Public, _F(Node_getNameSpace), KType_NameSpace, KType_Node, KMethodName_("getNameSpace"), 0,

		/* Expr */
		_Public, _F(Node_new), KType_Node, KType_Node, KMethodName_("new"), 1, KType_Object, KFieldName_("value"),
		_Public, _F(Node_getTermToken), KType_Token, KType_Node, KMethodName_("getTermToken"), 0,
		_Public, _F(Node_setConstValue), KType_Node, KType_Node, KMethodName_("setConstValue"), 1, KType_Object, KFieldName_("value") | KTypeAttr_Coercion,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}
////## Node Node.newTypedCallNode(Gamma ns, cid typeId, String methodName, Node firstNode, Node secondNode);
//static KMETHOD Node_newTypedCallNode2(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNode *stmt          = sfp[0].asNode;
//	kNameSpace *ns          = sfp[1].asGamma;
//	ktypeattr_t cid          = (ktypeattr_t)sfp[2].intValue;
//	ksymbol_t methodName = (ksymbol_t)sfp[3].intValue;
//	kNode *firstNode     = sfp[4].asNode;
//	kNode *secondNode    = sfp[5].asNode;
//	kMethod *method = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, kNode_ns(stmt), cid, methodName, 2, KMethodMatch_CamelStyle);
//	if(method == NULL) {
//		KReturn(KNULL(Node));
//	}
//	KReturn(SUGAR new_MethodNode(kctx, stmt, ns, cid, method, 2, firstNode, secondNode));
//}
//
////## Node Node.rightJoinNode(Node expr, Token[] tokenList, int currentIdx, int endIdx);
//static KMETHOD Node_rightJoinNode(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNode *stmt       = sfp[0].asNode;
//	kNode *expr       = sfp[1].asNode;
//	kArray *tokenList = sfp[2].asArray;
//	int currentIdx    = sfp[3].intValue;
//	int endIdx        = sfp[4].intValue;
//	KReturn(SUGAR kNode_RightJoinNode(kctx, stmt, expr, tokenList, currentIdx, endIdx));
//}

////## boolean Node.declType(Gamma ns, cid typeId, Node declNode);
//static KMETHOD Node_declType(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNode *stmt     = sfp[0].asNode;
//	kNameSpace *ns     = sfp[1].asGamma;
//	ktypeattr_t cid     = (ktypeattr_t)sfp[2].intValue;
//	kNode *declNode = sfp[3].asNode;
//	KReturnUnboxValue(SUGAR kNode_DeclType(kctx, stmt, ns, cid, declNode, NULL, NULL, &stmt));
//}
//
//

////## Token Node.getTermToken();
//static KMETHOD Node_getTermToken(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNode *expr = sfp[0].asNode;
//	KReturn(expr->TermToken);
//}
//
////## Node Node.setConstValue(Object value);
//static KMETHOD Node_SetConstValue(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNode *expr = sfp[0].asNode;
//	KClass *ct = kObject_class(sfp[1].asObject);
//	if(KClass_Is(UnboxType, (ct)) {
//		KReturn(SUGAR kNode_SetUnboxConst(kctx, expr, ct->typeId, sfp[1].unboxValue));
//	}
//	KReturn(SUGAR kNode_SetConst(kctx, expr, ct->typeId, sfp[1].asObject));
//}
//
////## Node Node.setVariable(Gamma ns, int build, cid typeid, int index);
//static KMETHOD Node_SetVariable(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNode *expr    = sfp[0].asNode;
//	kNameSpace *ns    = sfp[1].asGamma;
//	knode_t build  = (knode_t)sfp[2].intValue;
//	ktypeattr_t cid    = (ktypeattr_t)sfp[3].intValue;
//	intptr_t index = sfp[4].unboxValue;
//	KReturn(SUGAR kNode_SetVariable(kctx, expr, build, cid, index));
//}
//
////## Node Node.new(Gamma ns, int build, cid typeid, int index);
//static KMETHOD Node_newVariableNode(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNameSpace *ns    = sfp[1].asGamma;
//	knode_t build  = (knode_t)sfp[2].intValue;
//	ktypeattr_t cid    = (ktypeattr_t)sfp[3].intValue;
//	intptr_t index = sfp[4].unboxValue;
//	KReturn(new_VariableNode(kctx, ns, build, cid, index));
//}

////## void Node.setType(int build, cid typeid);
//static KMETHOD Node_SetType(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNodeVar *expr = (kNodeVar *)sfp[0].asNode;
//	knode_t build  = (knode_t)sfp[1].intValue;
//	ktypeattr_t cid    = (ktypeattr_t)sfp[2].intValue;
//	expr->node = build;
//	expr->attrTypeId = cid;
//	KReturnVoid();
//}
//

//// --------------------------------------------------------------------------
//// AST Method
//
////## int Gamma.declareLocalVariable(cid typeId, Symbol keyword);
//static KMETHOD Gamma_declareLocalVariable(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNameSpace *ns       = sfp[0].asGamma;
//	ktypeattr_t cid       = (ktypeattr_t)sfp[1].intValue;
//	ksymbol_t keyword = (ksymbol_t)sfp[2].intValue;
//	KReturnUnboxValue(SUGAR kNameSpace_AddLocalVariable(kctx, ns, cid, keyword));
//}

//static kSyntax* get_syntax(KonohaContext *kctx, kNameSpace *ns, kString *key)
//{
//	USING_SUGAR;
//	symbol_t kw = KSymbol_s(key);
//	if(kw == KSymbol_Noname) {
//		kreportf(CritTag, "undefined keyword: %s", kString_text(key));
//	}
//	kSyntax *syn = kSyntax_(ks, kw);
//	if(syn == NULL) {
//		kreportf(CritTag, "undefined syntax: %s", kString_text(key));
//	}
//	return syn;
//}

////## Node Token.printSyntaxError();
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
//	KReturn(K_NULLNODE);
//}

////## Node Node.newNode(Token[] tokenList, int s, int e);
//static KMETHOD Node_newNode(KonohaContext *kctx, KonohaStack *sfp)
//{
//	USING_SUGAR;
//	kNode *stmt  = sfp[0].asNode;
//	kArray *tokenList  = sfp[1].asArray;
//	int s = sfp[2].intValue, e = sfp[3].intValue;
//	KReturn(SUGAR new_BlockNode(kctx, kNode_ns(stmt), stmt, tokenList, s, e, ';'));
//}

////## Node Node.newMethodCallNode(Token key, Token self);
//static KMETHOD Node_newMethodCallNode(KonohaContext *kctx, KonohaStack *sfp)
//{
//	USING_SUGAR;
//	kNode *stmt  = sfp[0].asNode;
//	kToken *tk   = sfp[1].asToken;
//	assert(tk->keyword != 0);
//	kNodeVar *expr = /*G*/new_(NodeVar, kSyntax_(kNode_ns(stmt), tk->keyword));
//	KFieldSet(expr, expr->tk, tk);
//	KFieldSet(expr, expr->NodeList, new_(Array, 8));
//	KReturn(expr);
//}

////## Node Node.addNodeParam(Token tk);
//static KMETHOD Node_AddNodeParam(KonohaContext *kctx, KonohaStack *sfp)
//{
//	USING_SUGAR;
//	kNode *stmt  = sfp[0].asNode;
//	kNode *expr  = sfp[1].asNode;
//	kToken *tk     = sfp[2].asToken;
//	if(tk->tt != KSymbol_ParenthesisGroup || tk->tt != KSymbol_BracketGroup) {
//		SUGAR p(kctx, WarnTag, tk->uline, tk->lpos, "not parameter token");
//		kObject_Set(NullObject, expr, 1);
//	}
//	if(IS_NOTNULL(expr)) {
//		assert(IS_Array(tk->GroupTokenList));
//		expr = SUGAR AddParamNode(kctx, ns, expr, RangeGroup(tk->GroupTokenList), 1/*allowEmpty*/);
//	}
//	KReturn(expr);
//}

////## Node Node.addNode(Node expr, Node o);
//static KMETHOD kNode_AddNode(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNode *expr  = sfp[0].asNode;
//	kNode *o     = sfp[1].asNode;
//	if(IS_NULL(o) && IS_Array(expr->NodeList)) {
//		kObject_Set(NullObject, expr, 1);
//	}
//	if(IS_NOTNULL(expr)) {
//		KLIB kArray_Add(kctx, expr->NodeList, o);
//	}
//	KReturn(expr);
//}

// --------------------------------------------------------------------------

//
////// --------------------------------------------------------------------------
////// AST Method
////
//////## int Gamma.declareLocalVariable(cid typeId, Symbol keyword);
////static KMETHOD Gamma_declareLocalVariable(KonohaContext *kctx, KonohaStack *sfp)
////{
////	kNameSpace *ns       = sfp[0].asGamma;
////	ktypeattr_t cid       = (ktypeattr_t)sfp[1].intValue;
////	ksymbol_t keyword = (ksymbol_t)sfp[2].intValue;
////	KReturnUnboxValue(SUGAR AddLocalVariable(kctx, ns, cid, keyword));
////}
//
////static kSyntax* get_syntax(KonohaContext *kctx, kNameSpace *ns, kString *key)
////{
////	USING_SUGAR;
////	symbol_t kw = KSymbol_s(key);
////	if(kw == KSymbol_Noname) {
////		kreportf(CritTag, "undefined keyword: %s", kString_text(key));
////	}
////	kSyntax *syn = kSyntax_(ks, kw);
////	if(syn == NULL) {
////		kreportf(CritTag, "undefined syntax: %s", kString_text(key));
////	}
////	return syn;
////}
//
//////## Node Token.printSyntaxError();
////static KMETHOD Token_printSyntaxError(KonohaContext *kctx, KonohaStack *sfp)
////{
////	USING_SUGAR;
////	kToken *tk  = sfp[0].asToken;
////	if(IS_String(tk->text)) {
////		SUGAR p(kctx, ErrTag, tk->uline, tk->lpos, "syntax error: %s", kString_text(tk->text));
////	}
////	else {
////		SUGAR p(kctx, ErrTag, tk->uline, tk->lpos, "syntax error");
////	}
////	KReturn(K_NULLNODE);
////}
//
//////## Node Node.newNode(Token[] tokenList, int s, int e);
////static KMETHOD Node_newNode(KonohaContext *kctx, KonohaStack *sfp)
////{
////	USING_SUGAR;
////	kNode *stmt  = sfp[0].asNode;
////	kArray *tokenList  = sfp[1].asArray;
////	int s = sfp[2].intValue, e = sfp[3].intValue;
////	KReturn(SUGAR new_BlockNode(kctx, kNode_ns(stmt), stmt, tokenList, s, e, ';'));
////}
//
//////## Node Node.newMethodCallNode(Token key, Token self);
////static KMETHOD Node_newMethodCallNode(KonohaContext *kctx, KonohaStack *sfp)
////{
////	USING_SUGAR;
////	kNode *stmt  = sfp[0].asNode;
////	kToken *tk   = sfp[1].asToken;
////	assert(tk->keyword != 0);
////	kNodeVar *expr = /*G*/new_(NodeVar, kSyntax_(kNode_ns(stmt), tk->keyword));
////	KFieldSet(expr, expr->tk, tk);
////	KFieldSet(expr, expr->NodeList, new_(Array, 8));
////	KReturn(expr);
////}
//
//////## Node Node.addNodeParam(Token tk);
////static KMETHOD Node_AddNodeParam(KonohaContext *kctx, KonohaStack *sfp)
////{
////	USING_SUGAR;
////	kNode *stmt  = sfp[0].asNode;
////	kNode *expr  = sfp[1].asNode;
////	kToken *tk     = sfp[2].asToken;
////	if(tk->tt != KSymbol_ParenthesisGroup || tk->tt != KSymbol_BracketGroup) {
////		SUGAR p(kctx, WarnTag, tk->uline, tk->lpos, "not parameter token");
////		kObject_Set(NullObject, expr, 1);
////	}
////	if(IS_NOTNULL(expr)) {
////		assert(IS_Array(tk->GroupTokenList));
////		expr = SUGAR AddParamNode(kctx, ns, expr, RangeGroup(tk->GroupTokenList), 1/*allowEmpty*/);
////	}
////	KReturn(expr);
////}
//
//////## Node Node.addNode(Node expr, Node o);
////static KMETHOD kNode_AddNode(KonohaContext *kctx, KonohaStack *sfp)
////{
////	kNode *expr  = sfp[0].asNode;
////	kNode *o     = sfp[1].asNode;
////	if(IS_NULL(o) && IS_Array(expr->NodeList)) {
////		kObject_Set(NullObject, expr, 1);
////	}
////	if(IS_NOTNULL(expr)) {
////		KLIB kArray_Add(kctx, expr->NodeList, o);
////	}
////	KReturn(expr);
////}
//
//// --------------------------------------------------------------------------
//
//
////static kbool_t Syntax_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
////{
////	KClass *cSymbol = defineSymbolClass(kctx, ns, trace);
////	KDEFINE_INT_CONST ClassData[] = {   // add Array as available
////		{"Token", VirtualType_KClass, (uintptr_t)KClass_Token},
////		{"Node", VirtualType_KClass,  (uintptr_t)KClass_Node},
////		{"Node", VirtualType_KClass,  (uintptr_t)KClass_Node},
////		{"Node", VirtualType_KClass, (uintptr_t)KClass_Node},
////		{"Gamma", VirtualType_KClass, (uintptr_t)KClass_Gamma},
////		{"NameSpace", VirtualType_KClass, (uintptr_t)KClass_NameSpace},
////		{"symbol", VirtualType_KClass, (uintptr_t)cSymbol},
////		{NULL},
////	};
////	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(ClassData), 0);
////
////	int KType_Symbol = cSymbol->typeId;
////	Syntax_defineTokenFunc(kctx, ns, KType_Symbol, trace);
////	Syntax_defineMessageMethod(kctx, ns, KType_Symbol, trace);
////
////
////	int FN_buildid = KFieldName_("buildid"), FN_key = KFieldName_("key"), FN_defval = KFieldName_("defval");
////	int FN_typeid = KFieldName_("typeid"), FN_gma = KFieldName_("gma"), FN_pol = KFieldName_("pol");
////	int FN_msg = KFieldName_("msg");
////	int FN_x = KFieldName_("x");
////	int FN_tokenList = KFieldName_("tokens"), FN_s = KFieldName_("s"), FN_e = KFieldName_("e");
////	int FN_expr = KFieldName_("expr");
////
////	/* Array[String] */
////	kparamtype_t P_StringArray[] = {{KType_String}};
////	int KType_StringArray = (KLIB KClass_Generics(kctx, KClass_Array, KType_void, 1, P_StringArray))->typeId;
////
////	ktypeattr_t KType_Symbol = cSymbol->typeId;
////	ktypeattr_t KType_cid = ccid->typeId;
////
////	KDEFINE_METHOD MethodData[] = {
////		/* Token */
////		_Public, _F(Token_new), KType_Token, KType_Token, KMethodName_("new"), 1, KType_Symbol, FN_key,
////		_Public, _F(Token_SetUnresolvedTokenType),  KType_void, KType_Token, KMethodName_("setUnresolvedTokenType"),  1, KType_Symbol, FN_x,
////		_Public, _F(Token_SetText),  KType_void, KType_Token, KMethodName_("setText"),  1, KType_String, FN_x,
////		_Public, _F(Token_SetSubArray), KType_void, KType_Token, KMethodName_("setSubArray"), 1, KType_StringArray, FN_x,
//////		_Public, _F(Token_isTypeName), KType_boolean, KType_Token, KMethodName_("isTypeName"), 0,
////		_Public, _F(Token_isParenthesis), KType_boolean, KType_Token, KMethodName_("isParenthesis"), 0,
////		_Public, _F(Token_newUntypedNode), KType_Node, KType_Token, KMethodName_("newUntypedNode"), 0,
////
////		/* Node */
////		_Public, _F(Node_getBuild), KType_int, KType_Node,  KMethodName_("getBuild"), 0,
////		_Public, _F(Node_SetBuild), KType_void, KType_Node, KMethodName_("setBuild"), 1, KType_int, FN_buildid,
////		_Public, _F(Node_getNode), KType_Node, KType_Node, KMethodName_("getNode"), 2, KType_Symbol, FN_key, KType_Node, FN_defval,
////		_Public, _F(Node_getToken), KType_Token, KType_Node, KMethodName_("getToken"), 2, KType_Symbol, FN_key, KType_Token, FN_defval,
////		_Public, _F(Node_getNode), KType_Node, KType_Node, KMethodName_("getNode"), 2, KType_Symbol, FN_key, KType_Node, FN_defval,
////		_Public, _F(Node_TypeCheckNode), KType_boolean, KType_Node, KMethodName_("TypeCheckNode"), 3, KType_Symbol, FN_key, KType_Gamma, FN_ns, KType_cid, FN_typeid,
////		_Public, _F(Node_TypeCheckNodePol), KType_boolean, KType_Node, KMethodName_("TypeCheckNode"), 4, KType_Symbol, FN_key, KType_Gamma, FN_ns, KType_cid, FN_typeid, KType_int, FN_pol,
////		_Public, _F(Node_Message2rintError), KType_Node, KType_Node, KMethodName_("printError"), 1, KType_String, FN_msg,
////
////		_Public, _F(Node_newNode), KType_Node, KType_Node, KMethodName_("newNode"), 3, KType_TokenArray, FN_tokenList, KType_int, FN_s, KType_int, FN_e,
////		_Public, _F(Node_SetType), KType_void, KType_Node, KMethodName_("setType"), 1, KType_int, FN_x,
//////		_Public, _F(MessageNodearsedNode), KType_Node, KType_Node, KMethodName_("parseNode"), 3, KType_TokenArray, FN_tokenList, KType_int, FN_s, KType_int, FN_e,
////		_Public, _F(Node_newUntypedCallStyleNode2), KType_Node, KType_Node, KMethodName_("newUntypedCallStyleNode"), 2, KType_Token, KFieldName_("token"), KType_Node, KFieldName_("expr1"),
////		_Public, _F(Node_newUntypedCallStyleNode3), KType_Node, KType_Node, KMethodName_("newUntypedCallStyleNode"), 3, KType_Token, KFieldName_("token"), KType_Node, KFieldName_("expr1"), KType_Node, KFieldName_("expr2"),
////		_Public, _F(Node_newUntypedCallStyleNode4), KType_Node, KType_Node, KMethodName_("newUntypedCallStyleNode"), 4, KType_Token, KFieldName_("token"), KType_Node, KFieldName_("expr1"), KType_Node, KFieldName_("expr2"), KType_Node, KFieldName_("expr3"),
////		_Public, _F(Node_newUntypedCallStyleNode5), KType_Node, KType_Node, KMethodName_("newUntypedCallStyleNode"), 5, KType_Token, KFieldName_("token"), KType_Node, KFieldName_("expr1"), KType_Node, KFieldName_("expr2"), KType_Node, KFieldName_("expr3"), KType_Node, KFieldName_("expr4"),
////		_Public, _F(Node_newUntypedCallStyleNode6), KType_Node, KType_Node, KMethodName_("newUntypedCallStyleNode"), 6, KType_Token, KFieldName_("token"), KType_Node, KFieldName_("expr1"), KType_Node, KFieldName_("expr2"), KType_Node, KFieldName_("expr3"), KType_Node, KFieldName_("expr4"), KType_Node, KFieldName_("expr5"),
////		_Public, _F(Node_newTypedCallNode1), KType_Node, KType_Node, KMethodName_("newTypedCallNode"), 4, KType_Gamma, FN_ns, KType_cid, FN_typeid, KType_Symbol, KFieldName_("methodName"), KType_Node, KFieldName_("firstNode"),
////		_Public, _F(Node_newTypedCallNode2), KType_Node, KType_Node, KMethodName_("newTypedCallNode"), 5, KType_Gamma, FN_ns, KType_cid, FN_typeid, KType_Symbol, KFieldName_("methodName"), KType_Node, KFieldName_("firstNode"), KType_Node, KFieldName_("secondNode"),
////		_Public, _F(Node_rightJoinNode), KType_Node, KType_Node, KMethodName_("rightJoinNode"), 4, KType_Node, FN_expr, KType_TokenArray, FN_tokenList, KType_int, FN_s, KType_int, FN_e,
////		_Public, _F(Node_getTokenList), KType_TokenArray, KType_Node, KMethodName_("getTokenList"), 2, KType_Symbol, FN_key, KType_TokenArray, FN_defval,
////		_Public, _F(Node_declType), KType_boolean, KType_Node, KMethodName_("declType"), 3, KType_Gamma, FN_ns, KType_cid, FN_typeid, KType_Node, KFieldName_("declNode"),
////		_Public, _F(Node_newNode), KType_Node, KType_Node, KMethodName_("newNode"), 1, KType_String, KFieldName_("macro"),
////
////		/* Node */
////		_Public, _F(Node_getTermToken), KType_Token, KType_Node, KMethodName_("getTermToken"), 0,
////		_Public, _F(Node_SetConstValue), KType_Node, KType_Node, KMethodName_("setConstValue"), 1, KType_Object, KFieldName_("value") | FN_COERCION,
////		_Public, _F(Node_SetVariable), KType_Node, KType_Node, KMethodName_("setVariable"), 4, KType_Gamma, FN_ns, KType_int, FN_buildid, KType_cid, FN_typeid, KType_int, KFieldName_("index"),
////		_Public, _F(Node_newVariableNode), KType_Node, KType_Node, KMethodName_("new"), 4, KType_Gamma, FN_ns, KType_int, FN_buildid, KType_cid, FN_typeid, KType_int, KFieldName_("index"),
////		_Public, _F(Node_new), KType_Node, KType_Node, KMethodName_("new"), 1, KType_Object, KFieldName_("value") | FN_COERCION,
////		_Public, _F(Node_SetType), KType_void, KType_Node, KMethodName_("setType"), 2, KType_int, FN_buildid, KType_cid, FN_typeid,
////		_Public, _F(kNode_AddNode), KType_Node, KType_Node, KMethodName_("addNode"), 1, KType_Node, FN_expr,
////
////		/* Gamma */
////		_Public, _F(Gamma_declareLocalVariable), KType_int, KType_Gamma, KMethodName_("declareLocalVariable"), 2, KType_cid, FN_typeid, KType_Symbol, FN_key,
////		DEND,
////	};
////	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
////	LoadNameSpaceMethodData(kctx, ns, KType_Symbol, trace);
////	desuga_defineSyntaxStatement(kctx, ns, ns, trace);
////	return true;
////}
//
//
//// --------------------------------------------------------------------------
///* NameSpace */
//
//static kbool_t kSyntax_hasSyntaxPatternList(kSyntax *syn)
//{
//	while(syn != NULL) {
//		if(syn->syntaxPatternListNULL != NULL) return true;
//		syn = syn->parentSyntaxNULL;
//	}
//	return false;
//}

//static kbool_t kSyntax_hasSugarFunc(kSyntax *syn, int index)
//{
//	while(syn != NULL) {
//		if(syn->sugarFuncListTable[index] != NULL) return true;
//		syn = syn->parentSyntaxNULL;
//	}
//	return false;
//}

////## boolean NameSpace.definedSyntax(Symbol keyword);
//static KMETHOD NameSpace_DefinedSyntax(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
//	kSyntax* syn = kSyntax_(sfp[0].asNameSpace, keyword);
//	KReturnUnboxValue(syn != NULL);
//}

////## boolean NameSpace.definedLiteral(Symbol keyword);
//static KMETHOD NameSpace_DefinedLiteral(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
//	kSyntax* syn = kSyntax_(sfp[0].asNameSpace, keyword);
//	KReturnUnboxValue(kSyntax_hasSugarFunc(syn, KSugarTokenFunc));
//}

////## boolean NameSpace.definedStatement(Symbol keyword);
//static KMETHOD NameSpace_DefinedStatement(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
//	kSyntax* syn = kSyntax_(sfp[0].asNameSpace, keyword);
//	KReturnUnboxValue(kSyntax_hasSyntaxPatternList(syn) && kSyntax_hasSugarFunc(syn, SugarFunc_Statement));
//}

////## boolean NameSpace.definedExpression(Symbol keyword);
//static KMETHOD NameSpace_DefinedExpression(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
//	kSyntax* syn = kSyntax_(sfp[0].asNameSpace, keyword);
//	KReturnUnboxValue(syn != NULL && (syn->precedence_op2 > 0 || syn->precedence_op1 > 0));
//}

////## boolean NameSpace.definedBinaryOperator(Symbol keyword);
//static KMETHOD NameSpace_DefinedBinaryOperator(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
//	kSyntax* syn = kSyntax_(sfp[0].asNameSpace, keyword);
//	KReturnUnboxValue(syn != NULL && (syn->precedence_op2 > 0));
//}

////## void NameSpace.AddTokenizer(Symbol keyword, int konohaChar, Func f);
//static KMETHOD NameSpace_AddTokenizer(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
//	int konohaChar = AsciiToKonohaChar((int)sfp[2].intValue);
//	SUGAR kNameSpace_SetTokenFunc(kctx, sfp[0].asNameSpace, keyword, konohaChar, sfp[3].asFunc);
//}
//
////## void NameSpace.addPatternMatch(Symbol keyword, Func f);
//static KMETHOD NameSpace_AddPatternMatch(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
//	SUGAR kNameSpace_AddSugarFunc(kctx, sfp[0].asNameSpace, keyword, KSugarParseFunc, sfp[2].asFunc);
//}
//
////## void NameSpace.addExpression(Symbol keyword, Func f);
//static KMETHOD NameSpace_AddExpression(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
//	SUGAR kNameSpace_AddSugarFunc(kctx, sfp[0].asNameSpace, keyword, KSugarParseFunc, sfp[2].asFunc);
//}
//
////## void NameSpace.addStatement(Symbol keyword, Func f);
//static KMETHOD NameSpace_AddStatement(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
//	SUGAR kNameSpace_AddSugarFunc(kctx, sfp[0].asNameSpace, keyword, SugarFunc_Statement, sfp[2].asFunc);
//}
//
////## void NameSpace.addTopLevelStatement(Symbol keyword, Func f);
//static KMETHOD NameSpace_AddTopLevelStatement(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
//	SUGAR kNameSpace_AddSugarFunc(kctx, sfp[0].asNameSpace, keyword, SugarFunc_TopLevelStatement, sfp[2].asFunc);
//}
//
////## void NameSpace.addTypeCheck(Symbol keyword, Func f);
//static KMETHOD NameSpace_AddTypeCheck(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
//	SUGAR kNameSpace_AddSugarFunc(kctx, sfp[0].asNameSpace, keyword, KSugarTypeFunc, sfp[2].asFunc);
//}

//new Syntax(F, F);
//
//Sytanx parent= NameSpace.GetSyntax("ADD");
//Syntax syntax = new Syntax("symbol", parent);
//syntax.SetTokenFunc(function);
//syntax.SetParseFunc(function, op1, op2);
//syntax.SetTypeFunc(function);
//syntax.SetPattern(".....");
//syntax.SetMacro(2, "X Y X + Y");
//DefineSyntax(syntax);
//
//ADD(1, 2)  => 1 + 2
//
//Syntax syntax = new Syntax("and");
//syntax.setMacro(0, "&&");

// ---------------------------------------------------------------------------
/* Syntax */

//## Syntax Syntax.new(Symbol keyword);
static KMETHOD Syntax_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kSyntaxVar *syn = new_(SyntaxVar, 0, OnStack);
	syn->keyword = (ksymbol_t)sfp[1].intValue;
	KReturn(syn);
}

//## void Syntax.SetTokenFunc(int konohaChar, Func[Int, Token, String] func);
static KMETHOD Syntax_SetTokenFunc(KonohaContext *kctx, KonohaStack *sfp)
{
	kSyntaxVar *syn = (kSyntaxVar *)sfp[0].asObject;
	syn->tokenKonohaChar = AsciiToKonohaChar(sfp[1].intValue);
	KFieldSet(syn, syn->sugarFuncTable[KSugarTokenFunc], sfp[2].asFunc);
	KReturnVoid();
}

//## void Syntax.SetParseFunc(Func[Int, Node, Int, Token[], Int, Int, Int] func, int op1, int op2);
static KMETHOD Syntax_SetParseFunc(KonohaContext *kctx, KonohaStack *sfp)
{
	kSyntaxVar *syn = (kSyntaxVar *)sfp[0].asObject;
	KFieldSet(syn, syn->sugarFuncTable[KSugarParseFunc], sfp[1].asFunc);
	syn->precedence_op1 = sfp[2].intValue;
	syn->precedence_op2 = sfp[3].intValue;
	KReturnVoid();
}

//## void Syntax.SetTypeFunc(Func[Node, Node, NameSpace, Object] func);
static KMETHOD Syntax_SetTypeFunc(KonohaContext *kctx, KonohaStack *sfp)
{
	kSyntaxVar *syn = (kSyntaxVar *)sfp[0].asObject;
	KFieldSet(syn, syn->sugarFuncTable[KSugarTypeFunc], sfp[1].asFunc);
	KReturnVoid();
}

//## void Syntax.SetMacro(int number, Symbol macro);
static KMETHOD Syntax_SetMacro(KonohaContext *kctx, KonohaStack *sfp)
{
	KTODO();
	//kSyntaxVar *syn = (kSyntaxVar *)sfp[0].asObject;
	//int number = sfp[1].intValue;
	//ksymbol_t macro = (ksymbol_t)sfp[2].intValue;
	//KReturnVoid();
}

static void Syntax_defineSyntaxMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	/* Func[Int, Token, String] */
	kparamtype_t P_FuncTokenize[] = {{KType_Token}, {KType_String}};
	int KType_FuncToken = (KLIB KClass_Generics(kctx, KClass_Func, KType_int, 2, P_FuncTokenize))->typeId;
	/* Func[Int, Node, Int, Token[], Int, Int, Int] */
	kparamtype_t P_FuncParse[] = {{KType_Node}, {KType_int}, {KType_TokenArray}, {KType_int}, {KType_int}, {KType_int}};
	int KType_FuncParse = (KLIB KClass_Generics(kctx, KClass_Func, KType_int, 6, P_FuncParse))->typeId;
	/* Func[Node, Node, NameSpace, Object] */
	kparamtype_t P_FuncType[] = {{KType_Node}, {KType_NameSpace}, {KType_Object}};
	int KType_FuncType = (KLIB KClass_Generics(kctx, KClass_Func, KType_Node, 3, P_FuncType))->typeId;
	KDEFINE_METHOD MethodData[] = {
		_Public|_Im, _F(Syntax_new), KType_Syntax, KType_Syntax, KMethodName_("new"), 1, TP_kw,
		_Public,     _F(Syntax_SetTokenFunc), KType_void, KType_Syntax, KMethodName_("SetTokenFunc"), 2, KType_int, KFieldName_("kchar"), KType_FuncToken, KFieldName_("func"),
		_Public,     _F(Syntax_SetParseFunc), KType_void, KType_Syntax, KMethodName_("SetParseFunc"), 3, KType_FuncParse, KFieldName_("func"), KType_int, KFieldName_("op1"), KType_int, KFieldName_("op2"),
		_Public,     _F(Syntax_SetTypeFunc), KType_void, KType_Syntax, KMethodName_("SetTypeFunc"), 1, KType_FuncType, KFieldName_("func"),
		_Public,     _F(Syntax_SetMacro), KType_void, KType_Syntax, KMethodName_("SetMacro"), 2, KType_int, KFieldName_("number"), KType_Symbol, KFieldName_("macro"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

// ---------------------------------------------------------------------------
/* NameSpace */

//## Syntax NameSpace.GetSyntax(Symbol keyword);
static KMETHOD NameSpace_GetSyntax(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(kSyntax_(sfp[0].asNameSpace, (ksymbol_t)sfp[1].intValue));
}

//## boolean NameSpace.DefineSyntax(Syntax syntax);
static KMETHOD NameSpace_DefineSyntax(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	kNameSpace *ns = sfp[0].asNameSpace;
	kSyntax *syn = (kSyntax *)sfp[1].asObject;
	SUGAR kNameSpace_AddSyntax(kctx, ns, syn, trace);
	KReturnVoid();
}

static void Syntax_defineNameSpaceMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const, _F(NameSpace_GetSyntax), KType_Syntax, KType_NameSpace, KMethodName_("GetSyntax"), 1, TP_kw,
		_Public       , _F(NameSpace_DefineSyntax), KType_boolean, KType_NameSpace, KMethodName_("DefineSyntax"), 1, TP_syntax,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

////## Node Token.newUntypedNode();
//static KMETHOD Token_newUntypedNode(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kToken *token = sfp[0].asToken;
//	KReturn(SUGAR kNode_Termnize(kctx, X,  token));
//}


// ---------------------------------------------------------------------------
/* syntax */

//static kbool_t isSubKeyword(KonohaContext *kctx, kArray *tokenList, int beginIdx, int endIdx)
//{
//	if(beginIdx+1 < endIdx && tokenList->TokenItems[beginIdx+1]->resolvedSyntaxInfo->keyword == KSymbol_TextPattern) {
//		const char *t = kString_text(tokenList->TokenItems[beginIdx+1]->text);
//		if(isalpha(t[0]) || t[0] < 0 /* multibytes char */) {
//			return 1;
//		}
//	}
//	return 0;
//}

//static kSyntaxVar *kNameSpace_guessSyntaxFromTokenList(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList)
//{
//	int beginIdx = 0, endIdx = kArray_size(tokenList);
//	if(beginIdx < endIdx) {
//		ksymbol_t keyword = tokenList->TokenItems[beginIdx]->resolvedSyntaxInfo->keyword;
//		if(keyword == KSymbol_TextPattern) {
//			ksymbol_t kw;
//			if(isSubKeyword(kctx, tokenList, beginIdx, endIdx)) {
//				char buf[256];
//				PLATAPI snprintf_i(buf, sizeof(buf), "%s_%s", kString_text(tokenList->TokenItems[beginIdx]->text), kString_text(tokenList->TokenItems[beginIdx+1]->text));
//				kw = KAsciiSymbol((const char *)buf, strlen(buf), KSymbol_NewId);
//			}
//			else {
//				kw = KAsciiSymbol(kString_text(tokenList->TokenItems[beginIdx]->text), kString_size(tokenList->TokenItems[beginIdx]->text), KSymbol_NewId);
//			}
//			return (kSyntaxVar *)NEWkSyntax_(ns, kw);
//		}
//		else if(keyword == KSymbol_DOLLAR) { // $TokenPattern
//			char buf[256];
//			PLATAPI snprintf_i(buf, sizeof(buf), "$%s", kString_text(tokenList->TokenItems[beginIdx+1]->text));
//			ksymbol_t kw = KAsciiSymbol((const char *)buf, strlen(buf), KSymbol_NewId);
//			return (kSyntaxVar *)NEWkSyntax_(ns, kw);
//		}
//	}
//	return NULL;
//}

//// Copied from namespace.h.
//static void kNameSpace_AppendArrayRef(KonohaContext *kctx, kNameSpace *ns, const kArray **arrayRef, kObject *o)
//{
//	if(arrayRef[0] == NULL) {
//		((kArray**)arrayRef)[0] = new_(Array, 0, ns->NameSpaceConstList);
//	}
//	DBG_ASSERT(IS_Array(arrayRef[0]));
//	KLIB kArray_Add(kctx, arrayRef[0], o);
//}

//#define kToken_IsFirstPattern(tk)   (KSymbol_IsPattern(tk->symbol) && tk->ruleNameSymbol != KSymbol_NodePattern)
//static KMETHOD Statement_syntax(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kbool_t r = 0;
//	VAR_TypeCheck(stmt, ns, reqc);
//	kTokenArray *tokenList = (kTokenArray *)kNode_GetObject(kctx, stmt, KSymbol_TokenPattern, NULL);
//	if(tokenList == NULL) {
//		SUGAR MessageNode(kctx, stmt, NULL, ErrTag, "empty syntax");
//	}
//	if(tokenList != NULL) {
//		if(!IS_Array(tokenList)) { // create tokenList from a Token
//			kTokenArray *a = new_(TokenArray, 0, OnGcStack);
//			KLIB kArray_Add(kctx, a, tokenList);
//			tokenList = a;
//		}
//		DBG_ASSERT(IS_Array(tokenList));
//		kNameSpace *ns = kNode_ns(stmt);
//		kSyntaxVar *syn = kNameSpace_guessSyntaxFromTokenList(kctx, ns, tokenList);
//		if(syn != NULL) {
//			if(syn->syntaxPatternListNULL != NULL) {
//				SUGAR MessageNode(kctx, stmt, NULL, InfoTag, "oveloading syntax: %s%s", KSymbol_Fmt2(syn->keyword));
//			}
//			else {
//				syn->syntaxPatternListNULL = new_(TokenArray, 0, ns->NameSpaceConstList);
//			}
//			KTokenSeq tokens = {ns, tokenList, 0, kArray_size(tokenList)};
//			// Referred to kNameSpace_ParseSyntaxPattern in ast.h.
//			kArray *patternList = syn->syntaxPatternListNULL;
//			size_t firstPatternIdx = kArray_size(patternList);
//			SUGAR kArray_AddSyntaxRule(kctx, patternList, &tokens);
//			if(firstPatternIdx < kArray_size(patternList)) {
//				kToken *firstPattern = patternList->TokenItems[firstPatternIdx];
//				if(kToken_IsFirstPattern(firstPattern)) {
//					kNameSpace_AppendArrayRef(kctx, ns, &((kNameSpaceVar *)ns)->metaPatternListNULL, UPCAST(firstPattern));
//				}
//			}
//			r = 1;
//		}
//		kNode_Type(kctx, stmt, KNode_Done, KType_void);
//	}
//	KReturnUnboxValue(r);
//}

//static kbool_t Syntax_defineSyntaxStatement(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
//{
//	KDEFINE_SYNTAX SYNTAX[] = {
//		{ KSymbol_("syntax"), 0, "\"syntax\" $Token*", 0, 0, NULL, NULL, Statement_syntax, NULL, NULL, },
//		{ KSymbol_END, },
//	};
//	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
//	return true;
//}

static kbool_t Syntax_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KDEFINE_INT_CONST ClassData[] = {   // add Array as available
		{"Symbol", VirtualType_KClass, (uintptr_t)KClass_Symbol},
		{"Syntax",   VirtualType_KClass,  (uintptr_t)KClass_Syntax},
		{"Token",  VirtualType_KClass, (uintptr_t)KClass_Token},
		{"Node",   VirtualType_KClass,  (uintptr_t)KClass_Node},
		{"NameSpace", VirtualType_KClass, (uintptr_t)KClass_NameSpace},
		{NULL},
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(ClassData), 0);
	Syntax_defineTokenFunc(kctx, ns, trace);
	Syntax_defineMessageMethod(kctx, ns, trace);
	Syntax_defineNodeMethod(kctx, ns, trace);
	Syntax_defineSyntaxMethod(kctx, ns, trace);
	Syntax_defineNameSpaceMethod(kctx, ns, trace);
	return true;
}

static kbool_t Syntax_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	KDEFINE_INT_CONST IntData[] = {
#define DEFINE_KEYWORD(KW) {#KW, KType_int, KW}
		DEFINE_KEYWORD(ErrTag),
		DEFINE_KEYWORD(WarnTag),
		DEFINE_KEYWORD(NoticeTag),
		DEFINE_KEYWORD(InfoTag),
		DEFINE_KEYWORD(DebugTag),
//		DEFINE_KEYWORD(KNode_Error),
//		DEFINE_KEYWORD(KNode_EXPR),
//		DEFINE_KEYWORD(KNode_BLOCK),
//		DEFINE_KEYWORD(KNode_RETURN),
//		DEFINE_KEYWORD(KNode_IF),
//		DEFINE_KEYWORD(KNode_LOOP),
//		DEFINE_KEYWORD(KNode_JUMP),
//		DEFINE_KEYWORD(KNode_Const),
//		DEFINE_KEYWORD(KNode_New),
//		DEFINE_KEYWORD(KNode_Null),
//		DEFINE_KEYWORD(KNode_UnboxConst),
//		DEFINE_KEYWORD(KNode_Local),
//		DEFINE_KEYWORD(KNode_BLOCK),
//		DEFINE_KEYWORD(KNode_Field),
////		DEFINE_KEYWORD(KNode_BOX),
////		DEFINE_KEYWORD(KNode_UNBOX),
//		DEFINE_KEYWORD(KNode_MethodCall),
//		DEFINE_KEYWORD(KNode_AND),
//		DEFINE_KEYWORD(KNode_OR),
//		DEFINE_KEYWORD(KNode_Assign),
//		DEFINE_KEYWORD(KNode_STACKTOP),
		DEFINE_KEYWORD(TypeCheckPolicy_NoCheck),
		DEFINE_KEYWORD(TypeCheckPolicy_AllowVoid),
		DEFINE_KEYWORD(TypeCheckPolicy_Coercion),
		DEFINE_KEYWORD(TypeCheckPolicy_CONST),

		DEFINE_KEYWORD(Precedence_CPPStyleScope),
		DEFINE_KEYWORD(Precedence_CStyleSuffixCall),  /*x(), x[], x.x x->x x++ */
		DEFINE_KEYWORD(Precedence_CStylePrefixOperator),  /*++x, --x, sizeof x &x +x -x !x (T)x  */
	//	DEFINE_KEYWORD(Precedence_CppMember),  /* .x ->x */
		DEFINE_KEYWORD(Precedence_CStyleMUL),  /* x * x, x / x, x % x*/
		DEFINE_KEYWORD(Precedence_CStyleADD),  /* x + x, x - x */
		DEFINE_KEYWORD(Precedence_CStyleSHIFT),  /* x << x, x >> x */
		DEFINE_KEYWORD(Precedence_CStyleCOMPARE),
		DEFINE_KEYWORD(Precedence_CStyleEquals),
		DEFINE_KEYWORD(Precedence_CStyleBITAND),
		DEFINE_KEYWORD(Precedence_CStyleBITXOR),
		DEFINE_KEYWORD(Precedence_CStyleBITOR),
		DEFINE_KEYWORD(Precedence_CStyleAND),
		DEFINE_KEYWORD(Precedence_CStyleOR),
		DEFINE_KEYWORD(Precedence_CStyleTRINARY),  /* ? : */
		DEFINE_KEYWORD(Precedence_CStyleAssign),
		DEFINE_KEYWORD(Precedence_CStyleCOMMA),
		DEFINE_KEYWORD(Precedence_Statement),
		DEFINE_KEYWORD(Precedence_CStyleStatementEnd),

#undef DEFINE_KEYWORD
		{NULL},
	};
	KLIB kNameSpace_LoadConstData(kctx, exportNS, KConst_(IntData), trace);
	return true;
}

KDEFINE_PACKAGE* Syntax_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "MiniKonoha", K_VERSION);
	d.PackupNameSpace   = Syntax_PackupNameSpace;
	d.ExportNameSpace   = Syntax_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
