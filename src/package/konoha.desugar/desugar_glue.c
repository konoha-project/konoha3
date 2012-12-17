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
#define TP_expr          KType_Node,      KFieldName_("expr")

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
		if(beginIdx <= endIdx && endIdx <= len) {
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
	KReturn(SUGAR kNode_Message2(kctx, stmt, NULL, level, "%s", kString_text(msg)));
}

//## Node Node.message(int error, Token tk, String msg);
static KMETHOD NodeToken_Message(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode   *stmt  = sfp[0].asNode;
	kinfotag_t level = (kinfotag_t)sfp[1].intValue;
	kString *msg   = sfp[3].asString;
	KReturn(SUGAR kNode_Message2(kctx, stmt, sfp[2].asToken, level, "%s", kString_text(msg)));
}

//## Node Node.message(int error, Node expr, String msg);
static KMETHOD NodeNode_Message(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode   *stmt  = sfp[0].asNode;
	kinfotag_t level = (kinfotag_t)sfp[1].intValue;
	kString *msg   = sfp[3].asString;
	KReturn(SUGAR kNode_Message2(kctx, stmt, sfp[2].asToken, level, "%s", kString_text(msg)));
}

static void desugar_defineMessageMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Node_Message), KType_Node, KType_Node, KKMethodName_("message"), 2, TP_level, TP_message,
		_Public, _F(NodeToken_Message), KType_Node, KType_Node, KKMethodName_("message"), 3, TP_level, TP_token, TP_message,
		_Public, _F(NodeNode_Message), KType_Node, KType_Node, KKMethodName_("message"), 3, TP_level, TP_expr, TP_message,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

// --------------------------------------------------------------------------
/* Node */

//## Node new(NameSpace ns)
static KMETHOD Node_new(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(new_(Node, sfp[1].asNameSpace, OnStack));
}

//## Node Node.newNode(Token[] tokenList, int beginIdx, endIdx);
static KMETHOD Node_newNode(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode    *stmt = sfp[0].asNode;
	kArray   *tokenList = sfp[1].asArray;
	int beginIdx = (int)sfp[2].intValue;
	int endIdx   = (int)sfp[3].intValue;
	KTokenSeq source = {Node_ns(stmt), tokenList, beginIdx, endIdx};
//	SUGAR KTokenSeq_Tokenize(kctx, &source, kString_text(macro), 0);
	KReturn(SUGAR new_kNode(kctx, stmt, NULL/*nameSpace*/, &source));
}

//## boolean Node.TypeCheckAll(Gamma gma);
static KMETHOD Node_TypeCheckAll(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(SUGAR kNode_TypeCheckAll(kctx, sfp[0].asNode, sfp[1].asGamma));
}

//## Array[Node] Node.GetNodeList();
static KMETHOD Node_GetNodeList(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(sfp[0].asNode->NodeList);
}

//## Node Node.GetParentNode();
static KMETHOD Node_GetParentNode(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(sfp[0].asNode->parentNodeNULL);
}

#define TP_gma       KType_Gamma, KFieldName_("gma")
#define KType_NodeArray (KClass_p0(kctx, KClass_Array, KType_Node)->typeId)

static void desugar_defineNodeMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const, _F(Node_new), KType_Node, KType_Node, KKMethodName_("new"), 1, KType_NameSpace, KFieldName_("namespace"),
		_Public, _F(Node_TypeCheckAll), KType_boolean, KType_Node, KKMethodName_("TypeCheckAll"), 1, TP_gma,
		_Public, _F(Node_newNode), KType_Node, KType_Node, KKMethodName_("newNode"), 3, TP_tokens, TP_begin, TP_end,
		_Public, _F(Node_GetNodeList), KType_NodeArray, KType_Node, KKMethodName_("GetNodeList"), 0,
		_Public, _F(Node_GetParentNode), KType_Node, KType_Node, KKMethodName_("GetParentNode"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

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
//	SUGAR KTokenSeq_Tokenize(kctx, &source, kString_text(sfp[2].asString), 0);
//	KTokenSeq tokens = {source.ns, source.tokenList, source.endIdx};
//	//tokens.TargetPolicy.ExpandingBraceGroup = (flag == 1);
//	SUGAR KTokenSeq_Preprocess(kctx, &tokens, NULL, &source, source.beginIdx);
//	ksymbol_t kw   = (ksymbol_t)sfp[1].intValue;
//	KSyntax *syn = KSyntax_(sfp[0].asNameSpace, kw);
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

//## Token Node.getToken(symbol key, Token def);
static KMETHOD Node_getToken(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *stmt   = sfp[0].asNode;
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	kToken *def   = sfp[2].asToken;
	KReturn(SUGAR kNode_GetToken(kctx, stmt, key, def));
}

////## int Node.Match(Symbol key, symbol nameid, Token[] tokenList, int beginIdx, int endIdx);
//static KMETHOD Node_Match(KonohaContext *kctx, KonohaStack *sfp)
//{
//	int ret = -1;
//	kNode   *stmt  = sfp[0].asNode;
//	ksymbol_t kw   = (ksymbol_t)sfp[1].intValue;
//	int nameid   = (ksymbol_t)sfp[2].intValue;
//	kArray *tokenList = sfp[3].asArray;
//	int beginIdx = sfp[4].intValue;
//	int endIdx = sfp[5].intValue;
//	KSyntax *syn = KSyntax_(kNode_ns(stmt), kw);
//	if(syn != NULL) {
//		int dummy = 0;
//		ret = SUGAR kNode_MatchPattern(kctx, stmt, syn, nameid, tokenList, beginIdx, endIdx, &dummy);
//	}
//	KReturnUnboxValue(ret);
//}

// --------------------------------------------------------------------------
/* Node */

////## int Node.getBuild();
//static KMETHOD Node_getBuild(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KReturnUnboxValue(sfp[0].asNode->node);
//}
//
////## void Node.setBuild(int buildid);
//static KMETHOD Node_setBuild(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNodeVar *stmt = (kNodeVar *) sfp[0].asNode;
//	stmt->node = sfp[1].intValue;
//}

//## boolean Node.TypeCheckNode(symbol key, Gamma gma, Object type, int policy);
static KMETHOD Node_TypeCheckNode(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	ktypeattr_t typeId = kObject_typeId(sfp[3].asObject);
	KReturnUnboxValue(SUGAR kNode_TypeCheckByName(kctx, sfp[0].asNode, key, sfp[2].asGamma, typeId, sfp[4].intValue));
}

////## Node Node.newNode(Token[] tokenList, int s, int e);
//static KMETHOD Node_newNode(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNode *stmt  = sfp[0].asNode;
//	kArray *tokenList  = sfp[1].asArray;
//	int s = sfp[2].intValue, e = sfp[3].intValue;
//	KReturn(SUGAR kNode_ParseNode(kctx, stmt, tokenList, s, e, NULL));
//}

//## Node Node.newUntypedCallStyleNode(Token token);
static KMETHOD Node_newUntypedCallStyleNode1(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *stmt   = sfp[0].asNode;
	kToken *token = sfp[1].asToken;
	KSyntax *syn = KSyntax_(Node_ns(stmt), KSymbol_NodeMethodCall);
	KReturn(SUGAR new_UntypedCallStyleNode(kctx, syn, 1, token));
}

//## Node Node.newUntypedCallStyleNode(Token token, Node expr1);
static KMETHOD Node_newUntypedCallStyleNode2(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *stmt   = sfp[0].asNode;
	kToken *token = sfp[1].asToken;
	kNode *expr1  = sfp[2].asNode;
	KSyntax *syn = KSyntax_(Node_ns(stmt), KSymbol_NodeMethodCall);
	KReturn(SUGAR new_UntypedCallStyleNode(kctx, syn, 2, token, expr1));
}

//## Node Node.newUntypedCallStyleNode(Token token, Node expr1, Node expr2);
static KMETHOD Node_newUntypedCallStyleNode3(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *stmt   = sfp[0].asNode;
	kToken *token = sfp[1].asToken;
	kNode *expr1  = sfp[2].asNode;
	kNode *expr2  = sfp[3].asNode;
	KSyntax *syn = KSyntax_(Node_ns(stmt), KSymbol_NodeMethodCall);
	KReturn(SUGAR new_UntypedCallStyleNode(kctx, syn, 3, token, expr1, expr2));
}

//## Node Node.newUntypedCallStyleNode(Token token, Node expr1, Node expr2, Node expr3);
static KMETHOD Node_newUntypedCallStyleNode4(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *stmt   = sfp[0].asNode;
	kToken *token = sfp[1].asToken;
	kNode *expr1  = sfp[2].asNode;
	kNode *expr2  = sfp[3].asNode;
	kNode *expr3  = sfp[4].asNode;
	KSyntax *syn = KSyntax_(Node_ns(stmt), KSymbol_NodeMethodCall);
	KReturn(SUGAR new_UntypedCallStyleNode(kctx, syn, 4, token, expr1, expr2, expr3));
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
	KSyntax *syn = KSyntax_(Node_ns(stmt), KSymbol_NodeMethodCall);
	KReturn(SUGAR new_UntypedCallStyleNode(kctx, syn, 5, token, expr1, expr2, expr3, expr4));
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
	KSyntax *syn = KSyntax_(Node_ns(stmt), KSymbol_NodeMethodCall);
	KReturn(SUGAR new_UntypedCallStyleNode(kctx, syn, 6, token, expr1, expr2, expr3, expr4, expr5));
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
	KSyntax *syn = KSyntax_(Node_ns(stmt), KSymbol_NodeMethodCall);
	KReturn(SUGAR new_UntypedCallStyleNode(kctx, syn, 7, token, expr1, expr2, expr3, expr4, expr5, expr6));
}

//## void Node.setType(int type);
static KMETHOD Node_setType(KonohaContext *kctx, KonohaStack *sfp)
{
	Node_typed(sfp[0].asNode, sfp[1].intValue);
	KReturnVoid();
}

//## Token[] Node.getTokenList(symbol keyword, Token[] def);
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

//## void Node.setNode(symbol key, Node expr);
//## void Node.setNode(symbol key, Node block);
static KMETHOD Node_setObject(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *stmt   = sfp[0].asNode;
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	kObject *obj  = sfp[2].asObject;
	kNode_SetObject(kctx, stmt, key, obj);
	KReturnVoid();
}

//## Node Node.getNode(symbol key);
static KMETHOD Node_getObject(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *stmt   = sfp[0].asNode;
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	KReturn(kNode_GetObject(kctx, stmt, key, K_NULL));
}

//## Node Node.getParentNode();
static KMETHOD Node_getParentNode(KonohaContext *kctx, KonohaStack *sfp)
{	
	kNode *stmt   = sfp[0].asNode;
	KReturn(stmt->parentNodeNULL);
}

static kNode* kNode_LookupNodeNULL(KonohaContext *kctx, kNode *stmt, ksymbol_t keyword)
{
	int i;
	kArray *bka = stmt->parentNodeNULL->NodeList;
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
	KReturn(ret ? ret : K_NULL);
}

//## void Node.done();
static KMETHOD Node_done(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode_done(kctx, sfp[0].asNode);
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
	KReturn(Node_ns(sfp[0].asNode));
}

#define TP_type KType_Object, KFieldName_("type")
#define TP_pol  KType_int, KFieldName_("policy")
#define TP_ArgNode(n) KType_Node, KFieldName_("expr" #n)

static void desugar_defineNodeMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	int FN_key = KFieldName_("key"), FN_defval = KFieldName_("defval");
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Node_opEQ), KType_boolean, KType_Node, KKMethodName_("=="), 1, KType_Node, KFieldName_("other"),
		_Public, _F(Node_TypeCheckNode), KType_boolean, KType_Node, KKMethodName_("TypeCheckNode"), 4, TP_kw, TP_gma, TP_type, TP_pol,
//		_Public, _F(Node_newNode), KType_Node, KType_Node, KKMethodName_("newNode"), 3, TP_tokens, TP_begin, TP_end,
		_Public, _F(Node_setType), KType_void, KType_Node, KKMethodName_("setType"), 1, TP_type,
		_Public, _F(Node_LookupNode), KType_Node, KType_Node, KKMethodName_("lookupNode"), 1, TP_kw,
//////		_Public, _F(kNode_Message2arsedNode), KType_Node, KType_Node, KKMethodName_("parseNode"), 3, KType_TokenArray, FN_tokenList, KType_int, FN_s, KType_int, FN_e,
		_Public, _F(Node_newUntypedCallStyleNode1), KType_Node, KType_Node, KKMethodName_("newUntypedCallStyleNode"), 1, TP_token,
		_Public, _F(Node_newUntypedCallStyleNode2), KType_Node, KType_Node, KKMethodName_("newUntypedCallStyleNode"), 2, TP_token, TP_ArgNode(1),
		_Public, _F(Node_newUntypedCallStyleNode3), KType_Node, KType_Node, KKMethodName_("newUntypedCallStyleNode"), 3, TP_token, TP_ArgNode(1), TP_ArgNode(2),
		_Public, _F(Node_newUntypedCallStyleNode4), KType_Node, KType_Node, KKMethodName_("newUntypedCallStyleNode"), 4, TP_token, TP_ArgNode(1), TP_ArgNode(2), TP_ArgNode(3),
		_Public, _F(Node_newUntypedCallStyleNode5), KType_Node, KType_Node, KKMethodName_("newUntypedCallStyleNode"), 5, TP_token, TP_ArgNode(1), TP_ArgNode(2), TP_ArgNode(3), TP_ArgNode(4),
		_Public, _F(Node_newUntypedCallStyleNode6), KType_Node, KType_Node, KKMethodName_("newUntypedCallStyleNode"), 6, TP_token, TP_ArgNode(1), TP_ArgNode(2), TP_ArgNode(3), TP_ArgNode(4), TP_ArgNode(5),
		_Public, _F(Node_newUntypedCallStyleNode7), KType_Node, KType_Node, KKMethodName_("newUntypedCallStyleNode"), 7, TP_token, TP_ArgNode(1), TP_ArgNode(2), TP_ArgNode(3), TP_ArgNode(4), TP_ArgNode(5), TP_ArgNode(6),
////		_Public, _F(Node_rightJoinNode), KType_Node, KType_Node, KKMethodName_("rightJoinNode"), 4, KType_Node, FN_expr, KType_TokenArray, FN_tokenList, KType_int, FN_s, KType_int, FN_e,
		_Public, _F(Node_setObject), KType_void, KType_Node, KKMethodName_("setNode"), 2, KType_Symbol, FN_key, KType_Node, KFieldName_("expr"),
		_Public, _F(Node_setObject), KType_void, KType_Node, KKMethodName_("setNode"), 2, KType_Symbol, FN_key, KType_Node, KFieldName_("block"),
		_Public, _F(Node_getObject), KType_Node, KType_Node, KKMethodName_("getNode"), 1, KType_Symbol, FN_key,
		_Public, _F(Node_getParentNode), KType_Node, KType_Node, KKMethodName_("getParentNode"), 0,
		_Public, _F(Node_getTokenList), KType_TokenArray, KType_Node, KKMethodName_("getTokenList"), 2, KType_Symbol, FN_key, KType_TokenArray, FN_defval,
		_Public, _F(Node_getToken), KType_Token, KType_Node, KKMethodName_("getToken"), 2, KType_Symbol, FN_key, KType_Token, FN_defval,
		_Public, _F(Node_done), KType_void, KType_Node, KKMethodName_("done"), 0,
		_Public, _F(Node_keywordIs), KType_boolean, KType_Node, KKMethodName_("keywordIs"), 1, KType_Symbol, FN_key,
		_Public, _F(Node_getNameSpace), KType_NameSpace, KType_Node, KKMethodName_("getNameSpace"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}
////## Node Node.newTypedCallNode(Gamma gma, cid typeId, String methodName, Node firstNode, Node secondNode);
//static KMETHOD Node_newTypedCallNode2(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNode *stmt          = sfp[0].asNode;
//	kGamma *gma          = sfp[1].asGamma;
//	ktypeattr_t cid          = (ktypeattr_t)sfp[2].intValue;
//	ksymbol_t methodName = (ksymbol_t)sfp[3].intValue;
//	kNode *firstNode     = sfp[4].asNode;
//	kNode *secondNode    = sfp[5].asNode;
//	kMethod *method = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, Node_ns(stmt), cid, methodName, 2, KMethodMatch_CamelStyle);
//	if(method == NULL) {
//		KReturn(KNULL(Node));
//	}
//	KReturn(SUGAR new_TypedCallNode(kctx, stmt, gma, cid, method, 2, firstNode, secondNode));
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

////## boolean Node.declType(Gamma gma, cid typeId, Node declNode);
//static KMETHOD Node_declType(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNode *stmt     = sfp[0].asNode;
//	kGamma *gma     = sfp[1].asGamma;
//	ktypeattr_t cid     = (ktypeattr_t)sfp[2].intValue;
//	kNode *declNode = sfp[3].asNode;
//	KReturnUnboxValue(SUGAR kNode_DeclType(kctx, stmt, gma, cid, declNode, NULL, NULL, &stmt));
//}
//
//
// --------------------------------------------------------------------------
/* Node */

////## Token Node.getTermToken();
//static KMETHOD Node_getTermToken(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNode *expr = sfp[0].asNode;
//	KReturn(expr->TermToken);
//}
//
////## Node Node.setConstValue(Object value);
//static KMETHOD Node_setConstValue(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNode *expr = sfp[0].asNode;
//	KClass *ct = kObject_class(sfp[1].asObject);
//	if(KClass_Is(UnboxType, (ct)) {
//		KReturn(SUGAR kNode_SetUnboxConstValue(kctx, expr, ct->typeId, sfp[1].unboxValue));
//	}
//	KReturn(SUGAR kNode_SetConstValue(kctx, expr, ct->typeId, sfp[1].asObject));
//}
//
////## Node Node.setVariable(Gamma gma, int build, cid typeid, int index);
//static KMETHOD Node_setVariable(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNode *expr    = sfp[0].asNode;
//	kGamma *gma    = sfp[1].asGamma;
//	knode_t build  = (knode_t)sfp[2].intValue;
//	ktypeattr_t cid    = (ktypeattr_t)sfp[3].intValue;
//	intptr_t index = sfp[4].unboxValue;
//	KReturn(SUGAR kNode_SetVariable(kctx, expr, gma, build, cid, index));
//}
//
////## Node Node.new(Gamma gma, int build, cid typeid, int index);
//static KMETHOD Node_newVariableNode(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kGamma *gma    = sfp[1].asGamma;
//	knode_t build  = (knode_t)sfp[2].intValue;
//	ktypeattr_t cid    = (ktypeattr_t)sfp[3].intValue;
//	intptr_t index = sfp[4].unboxValue;
//	KReturn(new_VariableNode(kctx, gma, build, cid, index));
//}

//## Node Node.new(Object value);
static KMETHOD Node_new(KonohaContext *kctx, KonohaStack *sfp)
{
	KClass *ct = kObject_class(sfp[1].asObject);
	if(KClass_Is(UnboxType, ct)) {
		KReturn(new_UnboxConstValueNode(kctx, ct->typeId, sfp[1].unboxValue));
	}
	KReturn(new_ConstValueNode(kctx, ct, sfp[1].asObject));
}

////## void Node.setType(int build, cid typeid);
//static KMETHOD Node_setType(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNodeVar *expr = (kNodeVar *)sfp[0].asNode;
//	knode_t build  = (knode_t)sfp[1].intValue;
//	ktypeattr_t cid    = (ktypeattr_t)sfp[2].intValue;
//	expr->node = build;
//	expr->attrTypeId = cid;
//	KReturnVoid();
//}
//

static void desugar_defineNodeMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Node_new), KType_Node, KType_Node, KKMethodName_("new"), 1, KType_Object, KFieldName_("value"),
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
//	KReturn(K_NULLEXPR);
//}

////## Node Node.newNode(Token[] tokenList, int s, int e);
//static KMETHOD Node_newNode(KonohaContext *kctx, KonohaStack *sfp)
//{
//	USING_SUGAR;
//	kNode *stmt  = sfp[0].asNode;
//	kArray *tokenList  = sfp[1].asArray;
//	int s = sfp[2].intValue, e = sfp[3].intValue;
//	KReturn(SUGAR new_kNode(kctx, Node_ns(stmt), stmt, tokenList, s, e, ';'));
//}

////## Node Node.newMethodCallNode(Token key, Token self);
//static KMETHOD Node_newMethodCallNode(KonohaContext *kctx, KonohaStack *sfp)
//{
//	USING_SUGAR;
//	kNode *stmt  = sfp[0].asNode;
//	kToken *tk   = sfp[1].asToken;
//	assert(tk->keyword != 0);
//	kNodeVar *expr = /*G*/new_(NodeVar, KSyntax_(Node_ns(stmt), tk->keyword));
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
//		assert(IS_Array(tk->subTokenList));
//		expr = SUGAR kNode_AddNodeParam(kctx, stmt, expr, tk->subTokenList, 0, kArray_size(tk->subTokenList), 1/*allowEmpty*/);
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


//static kbool_t desugar_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
//{
//	KClass *cSymbol = defineSymbolClass(kctx, ns, trace);
//	KDEFINE_INT_CONST ClassData[] = {   // add Array as available
//		{"Token", VirtualType_KClass, (uintptr_t)KClass_Token},
//		{"Node", VirtualType_KClass,  (uintptr_t)KClass_Node},
//		{"Node", VirtualType_KClass,  (uintptr_t)KClass_Node},
//		{"Node", VirtualType_KClass, (uintptr_t)KClass_Node},
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
//		_Public, _F(Token_newUntypedNode), KType_Node, KType_Token, KKMethodName_("newUntypedNode"), 0,
//
//		/* Node */
//		_Public, _F(Node_getBuild), KType_int, KType_Node,  KKMethodName_("getBuild"), 0,
//		_Public, _F(Node_setBuild), KType_void, KType_Node, KKMethodName_("setBuild"), 1, KType_int, FN_buildid,
//		_Public, _F(Node_getNode), KType_Node, KType_Node, KKMethodName_("getNode"), 2, KType_Symbol, FN_key, KType_Node, FN_defval,
//		_Public, _F(Node_getToken), KType_Token, KType_Node, KKMethodName_("getToken"), 2, KType_Symbol, FN_key, KType_Token, FN_defval,
//		_Public, _F(Node_getNode), KType_Node, KType_Node, KKMethodName_("getNode"), 2, KType_Symbol, FN_key, KType_Node, FN_defval,
//		_Public, _F(Node_TypeCheckNode), KType_boolean, KType_Node, KKMethodName_("TypeCheckNode"), 3, KType_Symbol, FN_key, KType_Gamma, FN_gma, KType_cid, FN_typeid,
//		_Public, _F(Node_TypeCheckNodePol), KType_boolean, KType_Node, KKMethodName_("TypeCheckNode"), 4, KType_Symbol, FN_key, KType_Gamma, FN_gma, KType_cid, FN_typeid, KType_int, FN_pol,
//		_Public, _F(Node_Message2rintError), KType_Node, KType_Node, KKMethodName_("printError"), 1, KType_String, FN_msg,
//
//		_Public, _F(Node_newNode), KType_Node, KType_Node, KKMethodName_("newNode"), 3, KType_TokenArray, FN_tokenList, KType_int, FN_s, KType_int, FN_e,
//		_Public, _F(Node_setType), KType_void, KType_Node, KKMethodName_("setType"), 1, KType_int, FN_x,
////		_Public, _F(kNode_Message2arsedNode), KType_Node, KType_Node, KKMethodName_("parseNode"), 3, KType_TokenArray, FN_tokenList, KType_int, FN_s, KType_int, FN_e,
//		_Public, _F(Node_newUntypedCallStyleNode2), KType_Node, KType_Node, KKMethodName_("newUntypedCallStyleNode"), 2, KType_Token, KFieldName_("token"), KType_Node, KFieldName_("expr1"),
//		_Public, _F(Node_newUntypedCallStyleNode3), KType_Node, KType_Node, KKMethodName_("newUntypedCallStyleNode"), 3, KType_Token, KFieldName_("token"), KType_Node, KFieldName_("expr1"), KType_Node, KFieldName_("expr2"),
//		_Public, _F(Node_newUntypedCallStyleNode4), KType_Node, KType_Node, KKMethodName_("newUntypedCallStyleNode"), 4, KType_Token, KFieldName_("token"), KType_Node, KFieldName_("expr1"), KType_Node, KFieldName_("expr2"), KType_Node, KFieldName_("expr3"),
//		_Public, _F(Node_newUntypedCallStyleNode5), KType_Node, KType_Node, KKMethodName_("newUntypedCallStyleNode"), 5, KType_Token, KFieldName_("token"), KType_Node, KFieldName_("expr1"), KType_Node, KFieldName_("expr2"), KType_Node, KFieldName_("expr3"), KType_Node, KFieldName_("expr4"),
//		_Public, _F(Node_newUntypedCallStyleNode6), KType_Node, KType_Node, KKMethodName_("newUntypedCallStyleNode"), 6, KType_Token, KFieldName_("token"), KType_Node, KFieldName_("expr1"), KType_Node, KFieldName_("expr2"), KType_Node, KFieldName_("expr3"), KType_Node, KFieldName_("expr4"), KType_Node, KFieldName_("expr5"),
//		_Public, _F(Node_newTypedCallNode1), KType_Node, KType_Node, KKMethodName_("newTypedCallNode"), 4, KType_Gamma, FN_gma, KType_cid, FN_typeid, KType_Symbol, KFieldName_("methodName"), KType_Node, KFieldName_("firstNode"),
//		_Public, _F(Node_newTypedCallNode2), KType_Node, KType_Node, KKMethodName_("newTypedCallNode"), 5, KType_Gamma, FN_gma, KType_cid, FN_typeid, KType_Symbol, KFieldName_("methodName"), KType_Node, KFieldName_("firstNode"), KType_Node, KFieldName_("secondNode"),
//		_Public, _F(Node_rightJoinNode), KType_Node, KType_Node, KKMethodName_("rightJoinNode"), 4, KType_Node, FN_expr, KType_TokenArray, FN_tokenList, KType_int, FN_s, KType_int, FN_e,
//		_Public, _F(Node_getTokenList), KType_TokenArray, KType_Node, KKMethodName_("getTokenList"), 2, KType_Symbol, FN_key, KType_TokenArray, FN_defval,
//		_Public, _F(Node_declType), KType_boolean, KType_Node, KKMethodName_("declType"), 3, KType_Gamma, FN_gma, KType_cid, FN_typeid, KType_Node, KFieldName_("declNode"),
//		_Public, _F(Node_newNode), KType_Node, KType_Node, KKMethodName_("newNode"), 1, KType_String, KFieldName_("macro"),
//
//		/* Node */
//		_Public, _F(Node_getTermToken), KType_Token, KType_Node, KKMethodName_("getTermToken"), 0,
//		_Public, _F(Node_setConstValue), KType_Node, KType_Node, KKMethodName_("setConstValue"), 1, KType_Object, KFieldName_("value") | FN_COERCION,
//		_Public, _F(Node_setVariable), KType_Node, KType_Node, KKMethodName_("setVariable"), 4, KType_Gamma, FN_gma, KType_int, FN_buildid, KType_cid, FN_typeid, KType_int, KFieldName_("index"),
//		_Public, _F(Node_newVariableNode), KType_Node, KType_Node, KKMethodName_("new"), 4, KType_Gamma, FN_gma, KType_int, FN_buildid, KType_cid, FN_typeid, KType_int, KFieldName_("index"),
//		_Public, _F(Node_new), KType_Node, KType_Node, KKMethodName_("new"), 1, KType_Object, KFieldName_("value") | FN_COERCION,
//		_Public, _F(Node_setType), KType_void, KType_Node, KKMethodName_("setType"), 2, KType_int, FN_buildid, KType_cid, FN_typeid,
//		_Public, _F(kNode_AddNode), KType_Node, KType_Node, KKMethodName_("addNode"), 1, KType_Node, FN_expr,
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

//## boolean NameSpace.definedNodeession(symbol keyword);
static KMETHOD NameSpace_DefinedNodeession(KonohaContext *kctx, KonohaStack *sfp)
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

//## void NameSpace.addNodeession(symbol keyword, Func f);
static KMETHOD NameSpace_AddNodeession(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = (ksymbol_t)sfp[1].intValue;
	SUGAR kNameSpace_AddSugarFunc(kctx, sfp[0].asNameSpace, keyword, SugarFunc_Nodeession, sfp[2].asFunc);
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
	/* Func[Int, Node, Int, Token[], Int, Int] */
	kparamtype_t P_FuncPatternMatch[] = {{KType_Node}, {KType_int}, {KType_TokenArray}, {KType_int}, {KType_int}};
	int KType_FuncPatternMatch = (KLIB KClass_Generics(kctx, KClass_Func, KType_int, 5, P_FuncPatternMatch))->typeId;
	/* Func[Node, Node, Token[], Int, Int, Int] */
	kparamtype_t P_FuncNodeession[] = {{KType_Node}, {KType_TokenArray}, {KType_int}, {KType_int}, {KType_int}};
	int KType_FuncNodeession = (KLIB KClass_Generics(kctx, KClass_Func, KType_Node, 5, P_FuncNodeession))->typeId;
	/* Func[Boolean, Node, Gamma] */
	kparamtype_t P_FuncStatement[] = {{KType_Node}, {KType_Gamma}};
	int KType_FuncStatement = (KLIB KClass_Generics(kctx, KClass_Func, KType_boolean, 2, P_FuncStatement))->typeId;
	/* Func[Node, Node, Node, Gamma, Int] */
	kparamtype_t P_FuncTypeCheck[] = {{KType_Node}, {KType_Node}, {KType_Gamma}, {KType_int}};
	int KType_FuncTypeCheck = (KLIB KClass_Generics(kctx, KClass_Func, KType_Node, 4, P_FuncTypeCheck))->typeId;
	//DBG_P("func=%s", KType_text(KType_FuncTypeCheck));
	KDEFINE_METHOD MethodData[] = {
		_Public|_Im, _F(NameSpace_DefinedSyntax), KType_boolean, KType_NameSpace, KKMethodName_("DefinedSyntax"), 1, TP_kw,
		_Public|_Im, _F(NameSpace_DefinedLiteral), KType_boolean, KType_NameSpace, KKMethodName_("DefinedLiteral"), 1, TP_kw,
		_Public|_Im, _F(NameSpace_DefinedStatement), KType_boolean, KType_NameSpace, KKMethodName_("DefinedStatement"), 1, TP_kw,
		_Public|_Im, _F(NameSpace_DefinedNodeession), KType_boolean, KType_NameSpace, KKMethodName_("DefinedNodeession"), 1, TP_kw,
		_Public|_Im, _F(NameSpace_DefinedBinaryOperator), KType_boolean, KType_NameSpace, KKMethodName_("DefinedBinaryOperator"), 1, TP_kw,
		_Public, _F(NameSpace_AddTokenizer),         KType_void, KType_NameSpace, KKMethodName_("AddTokenizer"), 3, TP_kw, KType_int, KFieldName_("kchar"), KType_FuncToken, FN_func,
		_Public, _F(NameSpace_AddPatternMatch),      KType_void, KType_NameSpace, KKMethodName_("AddPatternMatch"), 2, TP_kw, KType_FuncPatternMatch, FN_func,
		_Public, _F(NameSpace_AddNodeession),        KType_void, KType_NameSpace, KKMethodName_("AddNodeession"), 2, TP_kw, KType_FuncNodeession, FN_func,
		_Public, _F(NameSpace_AddTopLevelStatement), KType_void, KType_NameSpace, KKMethodName_("AddTopLevelStatement"), 2, TP_kw, KType_FuncStatement, FN_func,
		_Public, _F(NameSpace_AddStatement),         KType_void, KType_NameSpace, KKMethodName_("AddStatement"), 2, TP_kw, KType_FuncStatement, FN_func,
		_Public, _F(NameSpace_AddTypeCheck),         KType_void, KType_NameSpace, KKMethodName_("AddTypeCheck"), 2, TP_kw, KType_FuncTypeCheck, FN_func,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

////## Node Token.newUntypedNode();
//static KMETHOD Token_newUntypedNode(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kToken *token = sfp[0].asToken;
//	KReturn(SUGAR new_TermNode(kctx, token));
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

#define kToken_IsFirstPattern(tk)   (KSymbol_IsPattern(tk->resolvedSymbol) && tk->stmtEntryKey != KSymbol_NodePattern)
static KMETHOD Statement_syntax(KonohaContext *kctx, KonohaStack *sfp)
{
	kbool_t r = 0;
	VAR_Statement(stmt, gma);
	kTokenArray *tokenList = (kTokenArray *)kNode_GetObject(kctx, stmt, KSymbol_TokenPattern, NULL);
	if(tokenList == NULL) {
		SUGAR kNode_Message2(kctx, stmt, NULL, ErrTag, "empty syntax");
	}
	if(tokenList != NULL) {
		if(!IS_Array(tokenList)) { // create tokenList from a Token
			kTokenArray *a = new_(TokenArray, 0, OnGcStack);
			KLIB kArray_Add(kctx, a, tokenList);
			tokenList = a;
		}
		DBG_ASSERT(IS_Array(tokenList));
		kNameSpace *ns = Node_ns(stmt);
		KSyntaxVar *syn = kNameSpace_guessSyntaxFromTokenList(kctx, ns, tokenList);
		if(syn != NULL) {
			if(syn->syntaxPatternListNULL != NULL) {
				SUGAR kNode_Message2(kctx, stmt, NULL, InfoTag, "oveloading syntax: %s%s", KSymbol_Fmt2(syn->keyword));
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
		kNode_done(kctx, stmt);
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
		{"Node",   VirtualType_KClass,  (uintptr_t)KClass_Node},
		{"Node",   VirtualType_KClass,  (uintptr_t)KClass_Node},
		{"Node",  VirtualType_KClass, (uintptr_t)KClass_Node},
		{"Gamma",  VirtualType_KClass, (uintptr_t)KClass_Gamma},
		{"NameSpace", VirtualType_KClass, (uintptr_t)KClass_NameSpace},
		{NULL},
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(ClassData), false/*isOverride*/, 0);
	desugar_defineTokenFunc(kctx, ns, trace);
	desugar_defineMessageMethod(kctx, ns, trace);
	desugar_defineNodeMethod(kctx, ns, trace);
	desugar_defineNodeMethod(kctx, ns, trace);
	desugar_defineNodeMethod(kctx, ns, trace);
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
