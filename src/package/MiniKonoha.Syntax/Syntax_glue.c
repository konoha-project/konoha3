/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
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

#include <konoha3/konoha.h>
#define USE_AsciiToKonohaChar
#include <konoha3/sugar.h>
#include <konoha3/klib.h>
#include <konoha3/konoha_common.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"{
#endif

#include <konoha3/import/methoddecl.h>

#define TP_kw           KType_Symbol,     KFieldName_("keyword")
#define TP_source       KType_String,     KFieldName_("source")
#define TP_pos          KType_Int,        KFieldName_("pos")
#define TP_tokens       KType_TokenArray, KFieldName_("tokens")
#define TP_begin        KType_Int,        KFieldName_("begin")
#define TP_end          KType_Int,        KFieldName_("end")
#define TP_message      KType_String,     KFieldName_("message")
#define TP_level        KType_Int,        KFieldName_("level")
#define TP_token        KType_Token,      KFieldName_("token")
#define TP_syntax       KType_Syntax,     KFieldName_("syntax")
#define TP_name         KType_String,     KFieldName_("name")

#include "import/namespace_method.h"
#include "import/syntax_method.h"
#include "import/token_method.h"
#include "import/node_method.h"

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
	kNode   *node  = sfp[0].asNode;
	kinfotag_t level = (kinfotag_t)sfp[1].intValue;
	kString *msg   = sfp[2].asString;
	KReturn(SUGAR MessageNode(kctx, node, NULL, kNode_ns(node), level, "%s", kString_text(msg)));
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
		_Public, _F(Node_Message), KType_Node, KType_Node, KMethodName_("Message"), 2, TP_level, TP_message,
		_Public, _F(NodeToken_Message), KType_Node, KType_Node, KMethodName_("Message"), 3, TP_level, TP_token, TP_message,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

// --------------------------------------------------------------------------
/* Node(Block) */

//## Node new(NameSpace ns)
static KMETHOD Node_new(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(new_(Node, sfp[1].asNameSpace, OnStack));
}

//## Node Node.ParseNewNode(Token[] tokenList, int beginIdx, int endIdx, boolean isMetaPattern);
static KMETHOD Node_ParseNewNode(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode    *stmt = sfp[0].asNode;
	kArray   *tokenList = sfp[1].asArray;
	int beginIdx = (int)sfp[2].intValue;
	int endIdx   = (int)sfp[3].intValue;
	ParseOption opt = (sfp[4].boolValue == true) ? ParseMetaPatternOption : 0;
	KReturn(SUGAR ParseNewNode(kctx, kNode_ns(stmt), tokenList, &beginIdx, endIdx, opt, NULL));
}

//## Array[Node] Node.GetNodeList();
static KMETHOD Node_GetNodeList(KonohaContext *kctx, KonohaStack *sfp)
{
	if(sfp[0].asNode->NodeList != NULL && IS_Array(sfp[0].asNode->NodeList)) {
		KReturn(sfp[0].asNode->NodeList);
	}
	else {
		KReturn(K_NULL);
	}
}

//## Node Node.GetParentNode();
static KMETHOD Node_GetParentNode(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(kNode_GetParent(kctx, sfp[0].asNode));
}

//## Node Node.newMethodNode(Object type, Symbol keyword, Node expr1);
static KMETHOD Node_newMethodNode1(KonohaContext *kctx, KonohaStack *sfp)
{
	kNameSpace *ns = kNode_ns(sfp[0].asNode);
	KClass *type = kObject_class(sfp[1].asObject);
	ksymbol_t keyword = (ksymbol_t)sfp[2].intValue;
	kNode *expr1 = sfp[3].asNode;
	kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, type, keyword, 0, KMethodMatch_NoOption);
	if(mtd == NULL) {
		KReturn(KNULL(Node));
	}
	KReturn(SUGAR new_MethodNode(kctx, ns, type, mtd, 1, expr1));
}

//## Node Node.newMethodNode(Object type, Symbol keyword, Node expr1, Node expr2);
static KMETHOD Node_newMethodNode2(KonohaContext *kctx, KonohaStack *sfp)
{
	kNameSpace *ns = kNode_ns(sfp[0].asNode);
	KClass *type = kObject_class(sfp[1].asObject);
	ksymbol_t keyword = (ksymbol_t)sfp[2].intValue;
	kNode *expr1 = sfp[3].asNode;
	kNode *expr2 = sfp[4].asNode;
	kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, type, keyword, 1, KMethodMatch_NoOption);
	if(mtd == NULL) {
		KReturn(KNULL(Node));
	}
	KReturn(SUGAR new_MethodNode(kctx, ns, type, mtd, 2, expr1, expr2));
}

//#define TP_ns       KType_NameSpace, KFieldName_("ns")
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

//## void Node.AddParsedObject(keyword nameid, Object obj);
static KMETHOD Node_AddParsedObject(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode   *stmt  = sfp[0].asNode;
	ksymbol_t symbol = (ksymbol_t)sfp[1].intValue;
	SUGAR kNode_AddParsedObject(kctx, stmt, symbol, sfp[2].asObject);
	KReturnVoid();
}

//## SmartReturn Object Node.GetParsedObject(keyword nameid);
static KMETHOD Node_GetParsedObject(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *stmt  = sfp[0].asNode;
	ksymbol_t symbol = (ksymbol_t)sfp[1].intValue;
	kObject *o = kNode_GetObject(kctx, stmt, symbol, NULL);
	if(o != NULL) {
		KReturn(o);
	}
	KReturnDefaultValue();
}

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

//## Node Node.TypeCheckNode(Symbol key, Object type, int policy);
static KMETHOD Node_TypeCheckNode(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *node = sfp[0].asNode;
	ksymbol_t key = (ksymbol_t)sfp[1].intValue;
	KClass *type = kObject_class(sfp[2].asObject);
	int policy = sfp[3].intValue;
	KReturn(SUGAR TypeCheckNodeByName(kctx, node, key, kNode_ns(node), type, policy));
}

//## Node Node.TypeCheckNodeAt(int pos, Object type, int policy);
static KMETHOD Node_TypeCheckNodeAt(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *node = sfp[0].asNode;
	size_t pos = sfp[1].intValue;
	KClass *type = kObject_class(sfp[2].asObject);
	int policy = sfp[3].intValue;
	KReturn(SUGAR TypeCheckNodeAt(kctx, node, pos, kNode_ns(node), type, policy));
}

//## void Node.SetType(int type);
static KMETHOD Node_SetType(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode_Type(kctx, sfp[0].asNode, sfp[1].intValue, KType_void);
	KReturnVoid();
}

//## int Node.GetType();
static KMETHOD Node_GetType(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(kNode_node(sfp[0].asNode));
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

//## NameSpace Node.getNameSpace();
static KMETHOD Node_getNameSpace(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(kNode_ns(sfp[0].asNode));
}

// --------------------------------------------------------------------------
/* Node(Expr) */

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

//## Object Node.getConstValue();
static KMETHOD Node_getConstValue(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *expr = sfp[0].asNode;
	assert(kNode_node(expr) == KNode_Const);
	KReturn(expr->ObjectConstValue);
}

//## Node Node.addNode(Node node);
static KMETHOD Node_AddNode(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *self  = sfp[0].asNode;
	kNode *node  = sfp[1].asNode;
	SUGAR kNode_AddNode(kctx, self, node);
	KReturn(node);
}

//## Node Node.insertAfter(Node target, Node node);
static KMETHOD Node_InsertAfter(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *self   = sfp[0].asNode;
	kNode *target = sfp[1].asNode;
	kNode *node   = sfp[2].asNode;
	SUGAR kNode_InsertAfter(kctx, self, target, node);
	KReturn(node);
}

//## Node Node.Op(Token tk);
static KMETHOD Node_Op1(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *self  = sfp[0].asNode;
	kToken *tk   = sfp[1].asToken;
	KReturn(SUGAR kNode_Op(kctx, self, tk, 0));
}

//## Node Node.Op(Token tk, Node expr1);
static KMETHOD Node_Op2(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *self  = sfp[0].asNode;
	kToken *tk   = sfp[1].asToken;
	kNode *expr1 = sfp[2].asNode;
	KReturn(SUGAR kNode_Op(kctx, self, tk, 1, expr1));
}

//## Node Node.Op(Token tk, Node expr1, Node expr2);
static KMETHOD Node_Op3(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *self  = sfp[0].asNode;
	kToken *tk   = sfp[1].asToken;
	kNode *expr1 = sfp[2].asNode;
	kNode *expr2 = sfp[3].asNode;
	KReturn(SUGAR kNode_Op(kctx, self, tk, 2, expr1, expr2));
}

//## Node Node.AppendParsedNode(Token[] tokenList, int beginIdx, int endIdx, String requiredTokenText);
static KMETHOD Node_AppendParsedNode(KonohaContext *kctx, KonohaStack *sfp)
{
	kNode *node = sfp[0].asNode;
	kArray *tokenList = sfp[1].asArray;
	int beginIdx = sfp[2].intValue;
	int endIdx = sfp[3].intValue;
	const char *requiredTokenText = IS_NULL(sfp[4].asString) ? NULL : kString_text(sfp[4].asString);
	KReturn(SUGAR AppendParsedNode(kctx, node, tokenList, beginIdx, endIdx, NULL, ParseExpressionOption, requiredTokenText));
}

#define TP_type KType_Object, KFieldName_("type")
#define TP_pol  KType_Int, KFieldName_("policy")
#define TP_ArgNode(n) KType_Node, KFieldName_("expr" #n)

static void Syntax_defineNodeMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	int FN_key = KFieldName_("key"), FN_defval = KFieldName_("defval");
	KDEFINE_METHOD MethodData[] = {
		/* Block */
		_Public|_Const, _F(Node_new), KType_Node, KType_Node, KMethodName_("new"), 1, KType_NameSpace, KFieldName_("namespace"),
		_Public, _F(Node_ParseNewNode), KType_Node, KType_Node, KMethodName_("ParseNewNode"), 4, TP_tokens, TP_begin, TP_end, KType_Boolean, KFieldName_("isMetaPattern"),
		_Public, _F(Node_GetNodeList), KType_NodeArray, KType_Node, KMethodName_("GetNodeList"), 0,
		_Public, _F(Node_GetParentNode), KType_Node, KType_Node, KMethodName_("GetParentNode"), 0,

		_Public, _F(Node_opEQ), KType_Boolean, KType_Node, KMethodName_("=="), 1, KType_Node, KFieldName_("other"),
		_Public, _F(Node_TypeCheckNode), KType_Node, KType_Node, KMethodName_("TypeCheckNode"), 3, TP_kw, TP_type, TP_pol,
		_Public, _F(Node_TypeCheckNodeAt), KType_Node, KType_Node, KMethodName_("TypeCheckNodeAt"), 3, TP_pos, TP_type, TP_pol,
		_Public, _F(Node_SetType), KType_void, KType_Node, KMethodName_("SetType"), 1, TP_type,
		_Public|_Im, _F(Node_GetType), KType_Int, KType_Node, KMethodName_("GetType"), 0,
		_Public, _F(Node_LookupNode), KType_Node, KType_Node, KMethodName_("lookupNode"), 1, TP_kw,
//////		_Public, _F(MessageNodearsedNode), KType_Node, KType_Node, KMethodName_("parseNode"), 3, KType_TokenArray, FN_tokenList, KType_Int, FN_s, KType_Int, FN_e,
		_Public, _F(Node_Op1), KType_Node, KType_Node, KMethodName_("Op"), 1, TP_token,
		_Public, _F(Node_Op2), KType_Node, KType_Node, KMethodName_("Op"), 2, TP_token, TP_ArgNode(1),
		_Public, _F(Node_Op3), KType_Node, KType_Node, KMethodName_("Op"), 3, TP_token, TP_ArgNode(1), TP_ArgNode(2),
//		_Public, _F(Node_Op4), KType_Node, KType_Node, KMethodName_("Op"), 4, TP_token, TP_ArgNode(1), TP_ArgNode(2), TP_ArgNode(3),
//		_Public, _F(Node_Op5), KType_Node, KType_Node, KMethodName_("Op"), 5, TP_token, TP_ArgNode(1), TP_ArgNode(2), TP_ArgNode(3), TP_ArgNode(4),
//		_Public, _F(Node_Op6), KType_Node, KType_Node, KMethodName_("Op"), 6, TP_token, TP_ArgNode(1), TP_ArgNode(2), TP_ArgNode(3), TP_ArgNode(4), TP_ArgNode(5),
//		_Public, _F(Node_Op7), KType_Node, KType_Node, KMethodName_("Op"), 7, TP_token, TP_ArgNode(1), TP_ArgNode(2), TP_ArgNode(3), TP_ArgNode(4), TP_ArgNode(5), TP_ArgNode(6),
////		_Public, _F(Node_rightJoinNode), KType_Node, KType_Node, KMethodName_("rightJoinNode"), 4, KType_Node, FN_expr, KType_TokenArray, FN_tokenList, KType_Int, FN_s, KType_Int, FN_e,
		_Public, _F(Node_getObject), KType_Node, KType_Node, KMethodName_("getNode"), 1, KType_Symbol, FN_key,
		_Public, _F(Node_getTokenList), KType_TokenArray, KType_Node, KMethodName_("getTokenList"), 2, KType_Symbol, FN_key, KType_TokenArray, FN_defval,
		_Public, _F(Node_getToken), KType_Token, KType_Node, KMethodName_("getToken"), 2, KType_Symbol, FN_key, KType_Token, FN_defval,
		_Public, _F(Node_done), KType_void, KType_Node, KMethodName_("done"), 0,
		_Public, _F(Node_keywordIs), KType_Boolean, KType_Node, KMethodName_("keywordIs"), 1, KType_Symbol, FN_key,
		_Public, _F(Node_getNameSpace), KType_NameSpace, KType_Node, KMethodName_("getNameSpace"), 0,
		_Public, _F(Node_AddNode), KType_Node, KType_Node, KMethodName_("addNode"), 1, KType_Node, KFieldName_("node"),
		_Public, _F(Node_InsertAfter), KType_Node, KType_Node, KMethodName_("insertAfter"), 2, KType_Node, KFieldName_("target"), KType_Node, KFieldName_("node"),

		/* Expr */
		_Public, _F(Node_getTermToken), KType_Token, KType_Node, KMethodName_("getTermToken"), 0,
		_Public, _F(Node_setConstValue), KType_Node, KType_Node, KMethodName_("setConstValue"), 1, KType_Object, KFieldName_("value") | KTypeAttr_Coercion,
		_Public, _F(Node_getConstValue), KType_Object, KType_Node, KMethodName_("getConstValue"), 0,
		_Public, _F(Node_AddParsedObject), KType_void, KType_Node, KMethodName_("AddParsedObject"), 2, TP_kw, KType_Object, KFieldName_("obj"),
		_Public, _F(Node_GetParsedObject), KType_Object, KType_Node, KMethodName_("GetParsedObject"), 1, TP_kw,
		_Public, _F(Node_AppendParsedNode), KType_Node, KType_Node, KMethodName_("AppendParsedNode"), 4, TP_tokens, TP_begin, TP_end, KType_String, KFieldName_("requiredTokenText"),
		_Public, _F(Node_newMethodNode1), KType_Node, KType_Node, KMethodName_("newMethodNode"), 3, TP_type, TP_kw, TP_ArgNode(1),
		_Public, _F(Node_newMethodNode2), KType_Node, KType_Node, KMethodName_("newMethodNode"), 4, TP_type, TP_kw, TP_ArgNode(1), TP_ArgNode(2),
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

////## boolean Node.declType(Gamma ns, cid typeId, Node declNode);
//static KMETHOD Node_declType(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kNode *stmt     = sfp[0].asNode;
//	kNameSpace *ns     = sfp[1].asGamma;
//	ktypeattr_t cid     = (ktypeattr_t)sfp[2].intValue;
//	kNode *declNode = sfp[3].asNode;
//	KReturnUnboxValue(SUGAR kNode_DeclType(kctx, stmt, ns, cid, declNode, NULL, NULL, &stmt));
//}

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
//		expr = SUGAR AppendParsedNode(kctx, ns, expr, RangeGroup(tk->GroupTokenList), 1/*allowEmpty*/);
//	}
//	KReturn(expr);
//}

// --------------------------------------------------------------------------

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
#define DEFINE_KEYWORD(KW) {#KW, KType_Int, KW}
		DEFINE_KEYWORD(ErrTag),
		DEFINE_KEYWORD(WarnTag),
		DEFINE_KEYWORD(NoticeTag),
		DEFINE_KEYWORD(InfoTag),
		DEFINE_KEYWORD(DebugTag),

		DEFINE_KEYWORD(TypeCheckPolicy_NoCheck),
		DEFINE_KEYWORD(TypeCheckPolicy_AllowVoid),
		DEFINE_KEYWORD(TypeCheckPolicy_Coercion),
		DEFINE_KEYWORD(TypeCheckPolicy_AllowEmpty),
		DEFINE_KEYWORD(TypeCheckPolicy_CONST),
		DEFINE_KEYWORD(TypeCheckPolicy_Creation),

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

		DEFINE_KEYWORD(KNode_Done),
		DEFINE_KEYWORD(KNode_Const),
		DEFINE_KEYWORD(KNode_New),
		DEFINE_KEYWORD(KNode_Null),
		DEFINE_KEYWORD(KNode_UnboxConst),
		DEFINE_KEYWORD(KNode_Local),
		DEFINE_KEYWORD(KNode_Field),
		DEFINE_KEYWORD(KNode_Box),
		DEFINE_KEYWORD(KNode_MethodCall),
		DEFINE_KEYWORD(KNode_And),
		DEFINE_KEYWORD(KNode_Or),
		DEFINE_KEYWORD(KNode_Assign),
		DEFINE_KEYWORD(KNode_Block),
		DEFINE_KEYWORD(KNode_If),
		DEFINE_KEYWORD(KNode_While),
		DEFINE_KEYWORD(KNode_DoWhile),
		DEFINE_KEYWORD(KNode_Return),
		DEFINE_KEYWORD(KNode_Break),
		DEFINE_KEYWORD(KNode_Continue),
		DEFINE_KEYWORD(KNode_Try),
		DEFINE_KEYWORD(KNode_Throw),
		DEFINE_KEYWORD(KNode_Error),

#undef DEFINE_KEYWORD
		{NULL},
	};
	KLIB kNameSpace_LoadConstData(kctx, exportNS, KConst_(IntData), trace);
	return true;
}

KDEFINE_PACKAGE *Syntax_Init(void)
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
