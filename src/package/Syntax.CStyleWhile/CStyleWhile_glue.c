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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/klib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Statement */

static KMETHOD Statement_while(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, gma, reqc);
	DBG_P("while statement .. ");
	kNode *exprNode = SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_ExprPattern, gma, KClass_Boolean, 0);
	if(kNode_IsError(exprNode)) {
		KReturn(exprNode);
	}
	SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_BlockPattern, gma, KClass_void, 0);
	kNode_Set(CatchContinue, stmt, true);  // set before TypeCheckAll
	kNode_Set(CatchBreak, stmt, true);
	KReturn(kNode_Type(kctx, stmt, KNode_While, KType_void));
}

//static KMETHOD Statement_do(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_TypeCheck(stmt, gma, reqc);
//	DBG_P("do statement .. ");
//	int ret = false;
//	if(SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_ExprPattern, gma, KClass_Boolean, 0)) {
//		kNode *bk = SUGAR kNode_GetNode(kctx, stmt, NULL/*DefaultNameSpace*/, KSymbol_BlockPattern, K_NULLBLOCK);
//		kNode_Set(CatchContinue, stmt, true);  // set before TypeCheckAll
//		kNode_Set(CatchBreak, stmt, true);
//		ret = SUGAR TypeCheckBlock(kctx, bk, gma);
//		if(ret) {
//			kNode_Set(RedoLoop, stmt, true);
//			kNode_Type(kctx, stmt, LOOP);  // FIXME
//		}
//	}
//	KReturnUnboxValue(ret);
//}
//
//static KMETHOD PatternMatch_ForNode(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
//	int i;
//	for(i = beginIdx; i < endIdx; i++) {
//		kTokenVar *tk = tokenList->TokenVarItems[i];
//		if(tk->resolvedSymbol == KSymbol_SEMICOLON) {
//			kToken_Set(StatementSeparator, tk, false);
//			break;
//		}
//	}
//	if(beginIdx < i) {
//		KTokenSeq tokens = {kNode_ns(stmt), tokenList, beginIdx, i};
//		kNode *bk = SUGAR new_BlockNode(kctx, stmt, NULL, &tokens);
//		SUGAR kNode_AddParsedObject(kctx, stmt, name, UPCAST(bk));
//	}
//	KReturnUnboxValue(i);
//}
//
//static void kNode_copy(KonohaContext *kctx, kNodeVar *dNode, ksymbol_t kw, kNode *sNode)
//{
//	kObject *o = kNode_GetObject(kctx, sNode, kw, NULL);
//	if(o != NULL) {
//		kNode_SetObject(kctx, dNode, kw, o);
//	}
//}
//
//static KMETHOD Statement_CStyleFor(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_TypeCheck(stmt, gma, reqc);
//	int ret = true;
//	int KSymbol_InitNode = KSymbol_("init"), KSymbol_IteratorNode = KSymbol_("Iterator");
//	kNode *initNode = SUGAR kNode_GetNode(kctx, stmt, NULL/*defaultNS*/, KSymbol_InitNode, NULL);
//	if(initNode == NULL) {  // with out init
//		DBG_P(">>>>>>>>> Without init block");
//		if(SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_NodePattern, gma, KClass_Boolean, 0)) {
//			kNode *bk = SUGAR kNode_GetNode(kctx, stmt, NULL/*DefaultNameSpace*/, KSymbol_NodePattern, K_NULLBLOCK);
//			kNode_Set(CatchContinue, stmt, true);  // set before TypeCheckAll
//			kNode_Set(CatchBreak, stmt, true);
//			SUGAR TypeCheckBlock(kctx, bk, gma);
//			kNode *iterNode = SUGAR kNode_GetNode(kctx, stmt, NULL/*defaultNS*/, KSymbol_IteratorNode, NULL);
//			if(iterNode != NULL) {
//				SUGAR TypeCheckBlock(kctx, iterNode, gma);
//			}
//			kNode_Set(RedoLoop, stmt, true);
//			kNode_Type(kctx, stmt, LOOP);
//		}
//	}
//	else {
//		kNodeVar *forNode = SUGAR new_BlockNode(kctx, OnGcStack, stmt->syn, 0);
//		DBG_ASSERT(IS_Node(initNode));
//		SUGAR kNode_InsertAfter(kctx, initNode, NULL, forNode);
//		forNode->uline = kNode_uline(stmt);
//		kNode_copy(kctx, forNode, KSymbol_NodePattern, stmt);
//		kNode_copy(kctx, forNode, KSymbol_NodePattern, stmt);
//		kNode_copy(kctx, forNode, KSymbol_IteratorNode, stmt);
//		SUGAR TypeCheckBlock(kctx, initNode, gma);
//		kNode_SetObject(kctx, stmt, KSymbol_NodePattern, initNode);
//		kNode_Type(kctx, stmt, BLOCK);
//	}
//	KReturnUnboxValue(ret);
//}

#define kNode_GetParentNULL(stmt)  IS_Node(stmt->Parent) ? stmt->Parent : NULL

static KMETHOD Statement_break(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, gma, reqc);
	kNode *p = stmt;
	while((p = kNode_GetParentNULL(p)) != NULL) {
		if(kNode_Is(CatchBreak, p)) {
			KLIB kObjectProto_SetObject(kctx, stmt, stmt->syn->keyword, KType_Node, p);
			KReturn(kNode_Type(kctx, stmt, KNode_Break, KType_void));
		}
	}
	SUGAR MessageNode(kctx, stmt, NULL, gma, ErrTag, "break statement not within a loop");
}

static KMETHOD Statement_continue(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, gma, reqc);
	kNode *p = stmt;
	while((p = kNode_GetParentNULL(p)) != NULL) {
		if(kNode_Is(CatchContinue, p)) {
			KLIB kObjectProto_SetObject(kctx, stmt, stmt->syn->keyword, KType_Node, p);
			KReturn(kNode_Type(kctx, stmt, KNode_Continue, KType_void));
		}
	}
	SUGAR MessageNode(kctx, stmt, NULL, gma, ErrTag, "continue statement not within a loop");
}

static void cstyle_DefineStatement(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("break"), SYNFLAG_CTypeFunc, 0, Precedence_Statement, {SUGAR patternParseFunc}, {SUGARFUNC Statement_break}},
		{ KSymbol_("continue"), SYNFLAG_CTypeFunc, 0, Precedence_Statement, {SUGAR patternParseFunc}, {SUGARFUNC Statement_continue}},
		{ KSymbol_("while"), SYNFLAG_CTypeFunc, 0, Precedence_Statement, {SUGAR patternParseFunc}, {SUGARFUNC Statement_while} },
//		{ KSymbol_("do"), 0, "\"do\"  $Node \"while\" \"(\" $Node \")\"", 0, 0, NULL, NULL, NULL, Statement_do, NULL, },
//		{ KSymbol_("$ForNode"), 0, NULL, 0, 0, PatternMatch_ForNode, NULL, NULL, NULL, NULL, },
//		{ KSymbol_("for"), 0, "\"for\" \"(\" init: $ForNode \";\" $Node \";\" Iterator: $ForNode \")\" $Node", 0, 0, NULL, NULL, NULL, Statement_CStyleFor, NULL, },
//		{ KSymbol_("$Inc"), 0, "$Inc $IncNode", 0, 0, PatternMatch_Inc, NULL, NULL, NULL, NULL, },
//		{ KSymbol_("$IncNode"), 0, NULL, 0, 0, PatternMatch_IncNode, NULL, NULL, NULL, NULL, },
		{ KSymbol_END, }, /* sentinental */
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	SUGAR kNameSpace_AddSyntaxPattern(kctx, ns, KSymbol_("while"), "\"while\" \"(\" $Expr \")\" $Block", 0, trace);
	SUGAR kNameSpace_AddSyntaxPattern(kctx, ns, KSymbol_("do"), "\"do\" $Block \"while\" \"(\" $Expr \")\"", 0, trace);
	SUGAR kNameSpace_AddSyntaxPattern(kctx, ns, KSymbol_("break"), "\"break\"", 0, trace);
	SUGAR kNameSpace_AddSyntaxPattern(kctx, ns, KSymbol_("continue"), "\"continue\"", 0, trace);
}


//// --------------------------------------------------------------------------
///* null */
//
////## Boolean Object.isNull();
//static KMETHOD Object_isNull(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kObject *o = sfp[0].asObject;
//	KReturnUnboxValue(IS_NULL(o));
//}
//
////## Boolean Object.isNotNull();
//static KMETHOD Object_isNotNull(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kObject *o = sfp[0].asObject;
//	KReturnUnboxValue(!IS_NULL(o));
//}
//
//static kbool_t null_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
//{
//	KDEFINE_METHOD MethodData[] = {
//		_Public|_Im|_Final|_Const, _F(Object_isNull),   KType_boolean, KType_Object, KMethodName_("isNull"), 0,
//		_Public|_Im|_Final|_Const, _F(Object_isNotNull), KType_boolean, KType_Object, KMethodName_("isNotNull"), 0,
//		DEND,
//	};
//	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
//	return true;
//}
//
///* null */
//
//static KMETHOD TypeCheck_null(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_TypeCheck2(stmt, expr, gma, reqc);
//	if(reqty == KType_var) reqty = KType_Object;
//	KReturn(SUGAR kNode_SetVariable(kctx, expr, KNode_Null, reqty, 0));
//}
//
//static KMETHOD Expression_isNull(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
//	if(operatorIdx + 2 == endIdx) {
//		DBG_P("checking .. x == null");
//		kTokenVar *tk = tokenList->TokenVarItems[operatorIdx+1];
//		if(tk->resolvedSymbol == KSymbol_("null")) {
//			kNode *leftHandNode = SUGAR ParseNewNode(kctx, stmt, tokenList, beginIdx, operatorIdx, NULL);
//			tk->resolvedSymbol = KSymbol_("isNull");
//			KReturn(SUGAR new_UntypedOperatorNode(kctx, KSyntax_(kNode_ns(stmt), KSymbol_NodeMethodCall), 2, tk, leftHandNode));
//		}
//	}
//	DBG_P("checking parent .. == ..");
//}
//
//static KMETHOD Expression_isNotNull(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
//	if(operatorIdx + 2 == endIdx) {
//		DBG_P("checking .. x != null");
//		kTokenVar *tk = tokenList->TokenVarItems[operatorIdx+1];
//		if(tk->resolvedSymbol == KSymbol_("null")) {
//			kNode *leftHandNode = SUGAR ParseNewNode(kctx, stmt, tokenList, beginIdx, operatorIdx, NULL);
//			tk->resolvedSymbol = KSymbol_("isNotNull");
//			KReturn(SUGAR new_UntypedOperatorNode(kctx, KSyntax_(kNode_ns(stmt), KSymbol_NodeMethodCall), 2, tk, leftHandNode));
//		}
//	}
//	DBG_P("checking parent .. != ..");
//}
//
//static kbool_t null_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
//{
//	KDEFINE_SYNTAX SYNTAX[] = {
//		{ KSymbol_("null"), 0, NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_null, },
//		{ KSymbol_("NULL"), 0, NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_null, },
//		{ KSymbol_END, },
//	};
//	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
//	SUGAR kNameSpace_AddSugarFunc(kctx, ns, KSymbol_("=="), KSugarParseFunc, KSugarFunc(ns, Expression_isNull));
//	SUGAR kNameSpace_AddSugarFunc(kctx, ns, KSymbol_("!="), KSugarParseFunc, KSugarFunc(ns, Expression_isNotNull));
//	return true;
//}
//
//static kbool_t null_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
//{
//	null_defineMethod(kctx, ns, trace);
//	null_defineSyntax(kctx, ns, trace);
//	return true;
//}

// --------------------------------------------------------------------------

static kbool_t cstyle_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	cstyle_DefineStatement(kctx, ns, trace);
	return true;
}

static kbool_t cstyle_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* CStyleWhile_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "CStyle", "1.0");
	d.PackupNameSpace    = cstyle_PackupNameSpace;
	d.ExportNameSpace   = cstyle_ExportNameSpace;
	return &d;
}

// --------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif
