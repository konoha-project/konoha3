///****************************************************************************
// * Copyright (c) 2012, the Konoha project authors. All rights reserved.
// * Redistribution and use in source and binary forms, with or without
// * modification, are permitted provided that the following conditions are met:
// *
// *  * Redistributions of source code must retain the above copyright notice,
// *    this list of conditions and the following disclaimer.
// *  * Redistributions in binary form must reproduce the above copyright
// *    notice, this list of conditions and the following disclaimer in the
// *    documentation and/or other materials provided with the distribution.
// *
// * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ***************************************************************************/
//
///* ************************************************************************ */
//
//#include <minikonoha/minikonoha.h>
//#include <minikonoha/sugar.h>
//#include <minikonoha/klib.h>
//
//#ifdef __cplusplus
//extern "C" {
//#endif
//
//#define _Public   kMethod_Public
//#define _Const    kMethod_Const
//#define _Im       kMethod_Immutable
//#define _Final    kMethod_Final
//#define _F(F)   (intptr_t)(F)
//
///* Statement */
//
//static KMETHOD Statement_while(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_TypeCheck(stmt, gma, reqc);
//	DBG_P("while statement .. ");
//	int ret = false;
//	if(SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_NodePattern, gma, KClass_Boolean, 0)) {
//		kNode *bk = SUGAR kNode_GetNode(kctx, stmt, NULL/*DefaultNameSpace*/, KSymbol_NodePattern, K_NULLBLOCK);
//		kNode_Set(CatchContinue, stmt, true);  // set before TypeCheckAll
//		kNode_Set(CatchBreak, stmt, true);
//		ret = SUGAR TypeCheckBlock(kctx, bk, gma);
//		if(ret) {
//			kNode_Type(kctx, stmt, LOOP);
//		}
//	}
//	KReturnUnboxValue(ret);
//}
//
//static KMETHOD Statement_do(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_TypeCheck(stmt, gma, reqc);
//	DBG_P("do statement .. ");
//	int ret = false;
//	if(SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_NodePattern, gma, KClass_Boolean, 0)) {
//		kNode *bk = SUGAR kNode_GetNode(kctx, stmt, NULL/*DefaultNameSpace*/, KSymbol_NodePattern, K_NULLBLOCK);
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
//
//static inline kNode* kNode_GetParentNULL(kNode *stmt)
//{
//	return stmt->parentNodeNULL->parentNodeNULL;
//}
//
//static KMETHOD Statement_break(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_TypeCheck(stmt, gma, reqc);
//	kNode *p = stmt;
//	while((p = kNode_GetParentNULL(p)) != NULL) {
//		if(kNode_Is(CatchBreak, p)) {
//			KLIB kObjectProto_SetObject(kctx, stmt, stmt->syn->keyword, KType_Node, p);
//			kNode_Type(kctx, stmt, JUMP);
//			KReturnUnboxValue(true);
//		}
//	}
//	SUGAR MessageNode(kctx, stmt, NULL, ErrTag, "break statement not within a loop");
//}
//
//static KMETHOD Statement_continue(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_TypeCheck(stmt, gma, reqc);
//	kNode *p = stmt;
//	while((p = kNode_GetParentNULL(p)) != NULL) {
//		if(kNode_Is(CatchContinue, p)) {
//			KLIB kObjectProto_SetObject(kctx, stmt, stmt->syn->keyword, KType_Node, p);
//			kNode_Type(kctx, stmt, JUMP);
//			KReturnUnboxValue(true);
//		}
//	}
//	SUGAR MessageNode(kctx, stmt, NULL, ErrTag, "continue statement not within a loop");
//}
//
//static KMETHOD PatternMatch_Inc(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
//	int i, SYM_Inc = KSymbol_("++"), SYM_Dec = KSymbol_("--");
//	for(i = beginIdx; i < endIdx; i++) {
//		kTokenVar *tk = tokenList->TokenVarItems[i];
//		if(tk->resolvedSymbol == SYM_Inc || tk->resolvedSymbol == SYM_Dec) {
//			KReturnUnboxValue(beginIdx);
//		}
//		if(kToken_Is(StatementSeparator, tk) || kToken_IsIndent(tk)) {
//			break;
//		}
//	}
//	KReturnUnboxValue(-1);
//}
//
//static KMETHOD PatternMatch_IncNode(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
//	int i, SYM_Inc = KSymbol_("++"), SYM_Dec = KSymbol_("--"), start, end;
//	for(i = beginIdx; i < endIdx; i++) {
//		kToken *tk = tokenList->TokenItems[i];
//		if(tk->resolvedSymbol == SYM_Inc || tk->resolvedSymbol == SYM_Dec) {
//			break;
//		}
//	}
//	if(beginIdx == i) {
//		start = beginIdx + 1;
//		end = SUGAR FindEndOfStatement(kctx, kNode_ns(stmt), tokenList, i, endIdx);
//	}
//	else {
//		start = beginIdx;
//		end   = i;
//	}
//	if(start < end) {
//		kToken *opToken = tokenList->TokenItems[i];
//		KSyntax *opSyntax = opToken->resolvedSyntaxInfo;
//		KTokenSeq macro = {kNode_ns(stmt), tokenList};
//		KTokenSeq_Push(kctx, macro);
//		KMacroSet macroParam[] = {
//			{KSymbol_("X"), tokenList, start, end},
//			{0, NULL, 0, 0},   /* sentinel */
//		};
//		macro.TargetPolicy.RemovingIndent = true;
//		SUGAR KTokenSeq_ApplyMacro(kctx, &macro, opSyntax->macroDataNULL, 0, kArray_size(opSyntax->macroDataNULL), opSyntax->macroParamSize, macroParam);
//		kNode *expr = SUGAR ParseNewNode(kctx, stmt, macro.tokenList, macro.beginIdx, macro.endIdx, NULL);
//		if(expr != K_NULLNODE) {
//			SUGAR kNode_AddParsedObject(kctx, stmt, KSymbol_NodePattern, UPCAST(expr));
//			((kNodeVar *)stmt)->syn = KSyntax_(kNode_ns(stmt), KSymbol_NodePattern);
//		}
//		KTokenSeq_Pop(kctx, macro);
//		end = SUGAR FindEndOfStatement(kctx, kNode_ns(stmt), tokenList, end+1, endIdx);
//		KReturnUnboxValue(end);
//	}
//	KReturnUnboxValue(-1);
//}
//
//static void cstyle_DefineStatement(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
//{
//	KDEFINE_SYNTAX SYNTAX[] = {
//		{ KSymbol_("break"), 0, "\"break\"", 0, 0, NULL, NULL, NULL, Statement_break, NULL, },
//		{ KSymbol_("continue"), 0, "\"continue\"", 0, 0, NULL, NULL, NULL, Statement_continue, NULL, },
//		{ KSymbol_("while"), 0, "\"while\" \"(\" $Node \")\" $Node", 0, 0, NULL, NULL, NULL, Statement_while, NULL, },
//		{ KSymbol_("do"), 0, "\"do\"  $Node \"while\" \"(\" $Node \")\"", 0, 0, NULL, NULL, NULL, Statement_do, NULL, },
//		{ KSymbol_("$ForNode"), 0, NULL, 0, 0, PatternMatch_ForNode, NULL, NULL, NULL, NULL, },
//		{ KSymbol_("for"), 0, "\"for\" \"(\" init: $ForNode \";\" $Node \";\" Iterator: $ForNode \")\" $Node", 0, 0, NULL, NULL, NULL, Statement_CStyleFor, NULL, },
//		{ KSymbol_("$Inc"), 0, "$Inc $IncNode", 0, 0, PatternMatch_Inc, NULL, NULL, NULL, NULL, },
//		{ KSymbol_("$IncNode"), 0, NULL, 0, 0, PatternMatch_IncNode, NULL, NULL, NULL, NULL, },
//		{ KSymbol_END, }, /* sentinental */
//	};
//	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
//}
//
///* ArrayLiteral */
//
//static KMETHOD Array_newList(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kArrayVar *a = (kArrayVar *)sfp[0].asObject;
//	size_t i = 0;
//	KonohaStack *p = sfp+1;
//	if(kArray_Is(UnboxData, a)) {
//		for(i = 0; p + i < kctx->esp; i++) {
//			a->unboxItems[i] = p[i].unboxValue;
//		}
//	}
//	else {
//		for(i = 0; p + i < kctx->esp; i++) {
//			KFieldSet(a, a->ObjectItems[i], p[i].asObject);
//		}
//	}
//	kArray_SetSize(a, i);
//	DBG_ASSERT(a->bytesize <= a->bytemax);
//	KReturn(a);
//}
//
//static KMETHOD TypeCheck_ArrayLiteral(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_TypeCheck2(stmt, expr, gma, reqc);
//	kToken *termToken = expr->TermToken;
//	DBG_ASSERT(kNode_IsTerm(expr) && IS_Token(termToken));
//	if(termToken->tokenType == TokenType_CODE) {
//		SUGAR kToken_ToBraceGroup(kctx, (kTokenVar *)termToken, kNode_ns(stmt), NULL);
//	}
//	if(termToken->resolvedSyntaxInfo->keyword == KSymbol_BraceGroup) {
//		kNodeVar *arrayNode = SUGAR new_UntypedOperatorNode(kctx, stmt->syn/*DUMMY*/, 2, K_NULL, K_NULL);
//		SUGAR AddParamNode(kctx, ns, arrayNode, termToken->subTokenList, 0, kArray_size(termToken->subTokenList), NULL);
//		size_t i;
//		KClass *requestClass = reqc;
//		KClass *paramType = KClass_INFER;
//		if(requestClass->baseTypeId == KType_Array) {
//			paramType = KClass_(requestClass->p0);
//		}
//		else {
//			requestClass = NULL; // undefined
//		}
//		for(i = 2; i < kArray_size(arrayNode->NodeList); i++) {
//			kNode *typedNode = SUGAR TypeCheckNodeAt(kctx, arrayNode, i, gma, paramType, 0);
//			if(typedNode == K_NULLNODE) {
//				KReturn(typedNode);
//			}
////			DBG_P("i=%d, paramType=%s, typedNode->ty=%s", i, KType_text(paramType), KType_text(typedNode->ty));
//			if(paramType->typeId == KType_var) {
//				paramType = KClass_(typedNode->attrTypeId);
//			}
//		}
//		if(requestClass == NULL) {
//			requestClass = (paramType->typeId == KType_var) ? KClass_Array : KClass_p0(kctx, KClass_Array, paramType->typeId);
//		}
//		kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, kNode_ns(stmt), KClass_Array, KMethodName_("{}"), -1, KMethodMatch_NoOption);
//		DBG_ASSERT(mtd != NULL);
//		KFieldSet(arrayNode, arrayNode->NodeList->MethodItems[0], mtd);
//		KFieldSet(arrayNode, arrayNode->NodeList->NodeItems[1], SUGAR kNode_SetVariable(kctx, NULL, gma, KNode_New, requestClass->typeId, kArray_size(arrayNode->NodeList) - 2));
//		KReturn(Node_typed(arrayNode, KNode_MethodCall, requestClass->typeId));
//	}
//}
//
//static kbool_t cstyle_defineArrayLiteral(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
//{
//	SUGAR kNameSpace_AddSugarFunc(kctx, ns, KSymbol_NodePattern, KSugarTypeCheckFunc, KSugarFunc(ns, TypeCheck_ArrayLiteral));
//	KDEFINE_METHOD MethodData[] = {
//		_Public|kMethod_Hidden, _F(Array_newList), KType_Array, KType_Array, KMethodName_("{}"), 0,
//		DEND,
//	};
//	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
//
//	return true;
//}
//
//
///* Literal */
//
//static KMETHOD TokenFunc_SingleQuotedChar(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kTokenVar *tk = (kTokenVar *)sfp[1].asObject;
//	int ch, prev = '/', pos = 1;
//	const char *source = kString_text(sfp[2].asString);
//	while((ch = source[pos++]) != 0) {
//		if(ch == '\n') {
//			break;
//		}
//		if(ch == '\'' && prev != '\\') {
//			if(IS_NOTNULL(tk)) {
//				KFieldSet(tk, tk->text, KLIB new_kString(kctx, OnField, source + 1, (pos-2), 0));
//				tk->tokenType = KSymbol_("$SingleQuotedChar");
//			}
//			KReturnUnboxValue(pos);
//		}
//		prev = ch;
//	}
//	if(IS_NOTNULL(tk)) {
//		SUGAR kToken_ToError(kctx, tk, ErrTag, "must close with %s", "'");
//	}
//	KReturnUnboxValue(0);
//}
//
//static KMETHOD TypeCheck_SingleQuotedChar(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_TypeCheck2(stmt, expr, gma, reqc);
//	kToken *tk = expr->TermToken;
//	if(kString_size(tk->text) == 1) {
//		int ch = kString_text(tk->text)[0];
//		KReturn(SUGAR kNode_SetUnboxConst(kctx, expr, KType_int, ch));
//	}
//	else if(kString_size(tk->text) == 2) {
//		int ch = kString_text(tk->text)[0];
//		if(ch == '\\') {
//			ch = kString_text(tk->text)[1];
//			switch(ch) {
//			case '\'': ch = '\''; break;
//			case '\\': ch = '\\'; break;
//			case 'b':  ch = '\b'; break;
//			case 'f':  ch = '\f'; break;
//			case 'n':  ch = '\n'; break;
//			case 'r':  ch = '\r'; break;
//			case 't':  ch = '\t'; break;
//			default:
//				KReturn(K_NULLNODE);
//			}
//			KReturn(SUGAR kNode_SetUnboxConst(kctx, expr, KType_int, ch));
//		}
//	}
//	KReturn(K_NULLNODE);
//}
//
///* Expression */
//
//static KMETHOD Expression_Indexer(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
//	KClass *genericsClass = NULL;
//	kNameSpace *ns = kNode_ns(stmt);
//	int nextIdx = SUGAR ParseTypePattern(kctx, ns, tokenList, beginIdx, endIdx, &genericsClass);
//	if(nextIdx != -1) {  // to avoid Func[T]
//		KReturn(SUGAR kNode_ParseOperatorNode(kctx, stmt, tokenList->TokenItems[beginIdx]->resolvedSyntaxInfo, tokenList, beginIdx, beginIdx, endIdx));
//	}
//	DBG_P("beginIdx=%d, endIdx=%d", beginIdx, endIdx);
//	kToken *currentToken = tokenList->TokenItems[operatorIdx];
//	if(beginIdx < operatorIdx) {
//		kNode *leftNode = SUGAR ParseNewNode(kctx, stmt, tokenList, beginIdx, operatorIdx, NULL);
//		if(leftNode == K_NULLNODE) {
//			KReturn(leftNode);
//		}
//		/* transform 'Value0 [ Value1 ]=> (Call Value0 get (Value1)) */
//		kTokenVar *tkN = new_(TokenVar, 0, OnGcStack);
//		tkN->resolvedSymbol= KMethodName_ToGetter(0);
//		tkN->uline = currentToken->uline;
//		KSyntax *syn = KSyntax_(kNode_ns(stmt), KSymbol_NodeMethodCall);
//		leftNode  = SUGAR new_UntypedOperatorNode(kctx, syn, 2, tkN, leftNode);
//		leftNode = SUGAR AddParamNode(kctx, ns, leftNode, currentToken->subTokenList, 0, kArray_size(currentToken->subTokenList), "[");
//		KReturn(SUGAR kNode_RightJoinNode(kctx, stmt, leftNode, tokenList, operatorIdx + 1, endIdx));
//	}
//	DBG_P("nothing");
//}
//
//static KMETHOD Expression_Increment(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
//	kToken *tk = tokenList->TokenItems[operatorIdx];
//	KReturn(kNodeToken_Message(kctx, stmt, tk, ErrTag, "%s is defined as a statement", kString_text(tk->text)));
//}
//
//static void cstyle_defineExpression(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
//{
//	KDEFINE_SYNTAX SYNTAX[] = {
//		{ KSymbol_("[]"), SYNFLAG_Suffix, NULL, Precedence_CStyleSuffixCall, 0, NULL, Expression_Indexer, NULL, NULL, NULL, },
//		{ KSymbol_("++"), SYNFLAG_Suffix, NULL, Precedence_CStyleSuffixCall, Precedence_CStylePrefixOperator, NULL, Expression_Increment,},
//		{ KSymbol_("--"), SYNFLAG_Suffix, NULL, Precedence_CStyleSuffixCall, Precedence_CStylePrefixOperator, NULL, Expression_Increment,},
//		{ KSymbol_END, }, /* sentinental */
//	};
//	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
//	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("++"), 1,  "X X = (X) + 1", false);
//	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("--"), 1,  "X X = (X) - 1", false);
//}
//
///* ------------------------------------------------------------------------- */
//
//
//static KMETHOD Int_opPlus(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KReturnUnboxValue(+(sfp[0].intValue));
//}
//
//static KMETHOD Int_opCompl (KonohaContext *kctx, KonohaStack *sfp)
//{
//	KReturnUnboxValue(~sfp[0].intValue);
//}
//
//static KMETHOD Int_opLSHIFT (KonohaContext *kctx, KonohaStack *sfp)
//{
//	int lshift = sfp[1].intValue;
//	KReturnUnboxValue(sfp[0].intValue << lshift);
//}
//
//static KMETHOD Int_opRSHIFT (KonohaContext *kctx, KonohaStack *sfp)
//{
//	int rshift = sfp[1].intValue;
//	KReturnUnboxValue(sfp[0].intValue >> rshift);
//}
//
//static KMETHOD Int_opAND(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KReturnUnboxValue(sfp[0].intValue & sfp[1].intValue);
//}
//
//static KMETHOD Int_opOR(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KReturnUnboxValue(sfp[0].intValue | sfp[1].intValue);
//}
//
//static KMETHOD Int_opXOR(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KReturnUnboxValue(sfp[0].intValue ^ sfp[1].intValue);
//}
//
///* ------------------------------------------------------------------------ */
//
//static kbool_t int_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
//{
//	int FN_x = KFieldName_("x");
//	KDEFINE_METHOD MethodData[] = {
//		_Public|_Const|_Im, _F(Int_opPlus), KType_int, KType_int, KMethodName_("+"), 0,
//		_Public|_Const|_Im, _F(Int_opCompl), KType_int, KType_int, KMethodName_("~"), 0,
//		_Public|_Const|_Im, _F(Int_opLSHIFT), KType_int, KType_int, KMethodName_("<<"), 1, KType_int, FN_x,
//		_Public|_Const|_Im, _F(Int_opRSHIFT), KType_int, KType_int, KMethodName_(">>"), 1, KType_int, FN_x,
//		_Public|_Const|_Im, _F(Int_opAND), KType_int, KType_int, KMethodName_("&"), 1, KType_int, FN_x,
//		_Public|_Const|_Im, _F(Int_opOR ), KType_int, KType_int, KMethodName_("|"), 1, KType_int, FN_x,
//		_Public|_Const|_Im, _F(Int_opXOR), KType_int, KType_int, KMethodName_("^"), 1, KType_int, FN_x,
//		DEND,
//	};
//	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
//	KDEFINE_INT_CONST IntData[] = {
//		{"INT_MAX", KType_int, KINT_MAX},
//		{"INT_MIN", KType_int, KINT_MIN},
//		{NULL},
//	};
//	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(IntData), false/*isOverride*/, trace);
//	return true;
//}
//
//// --------------------------------------------------------------------------
///* Syntax */
//
//static char parseHexDigit(char c)
//{
//	return ('0' <= c && c <= '9') ? c - '0' :
//		('a' <= c && c <= 'f') ? c - 'a' + 10:
//		('A' <= c && c <= 'F') ? c - 'A' + 10:-1;
//}
//static char parseOctalDigit(char c)
//{
//	return ('0' <= c && c <= '7') ? c - '0' : -1;
//}
//static char parseDecimalDigit(char c)
//{
//	return ('0' <= c && c <= '9') ? c - '0' : -1;
//}
//
//static char parseBinaryDigit(char c)
//{
//	return ('0' == c || c == '1') ? c - '0' : -1;
//}
//
//static KMETHOD TokenFunc_ExtendedIntLiteral(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kTokenVar *tk = (kTokenVar *)sfp[1].asObject;
//	const char *source = kString_text(sfp[2].asString);
//	const char *start = source, *end;
//	int c = *source++;
//	/*
//	 * DIGIT  = 0-9
//	 * DIGITS = DIGIT | DIGIT DIGITS
//	 * HEX    = 0-9a-fA-F
//	 * HEXS   = HEX | HEX HEXS
//	 * BIN    = 0 | 1
//	 * BINS   = BIN | BIN BINS
//	 * TAG    = "0x"  | "0b"
//	 * HEXINT = ("0x" | "0X") HEXS
//	 * INT    = DIGITS | HEXS | BINS
//	 */
//	int base = 10;
//	bool isFloat = false;
//	char (*parseDigit)(char) = parseDecimalDigit;
//	if(c == '0') {
//		c = *source++;
//		switch (c) {
//			case 'b':
//				base = 2;  parseDigit = parseBinaryDigit; break;
//			case 'x':
//				base = 16; parseDigit = parseHexDigit; break;
//			case '0':case '1':case '2':case '3':
//			case '4':case '5':case '6':case '7':
//				base = 8; parseDigit = parseOctalDigit;
//				break;
//			default:
//				source--;
//				break;
//		}
//	}
//	for (; (c = *source) != 0; ++source) {
//		if(c == '_') continue;
//		if(parseDigit(c) == -1)
//			break;
//	}
//
//	/*
//	 * DIGIT  = 0-9
//	 * DIGITS = DIGIT | DIGIT DIGITS
//	 * INT    = DIGIT | DIGIT1-9 DIGITS
//	 * FLOAT  = INT
//	 *        | INT FRAC
//	 *        | INT EXP
//	 *        | INT FRAC EXP
//	 * FRAC   = "." digits
//	 * EXP    = E digits
//	 * E      = 'e' | 'e+' | 'e-' | 'E' | 'E+' | 'E-'
//	 */
//	if(base != 10 && c != '.' && c != 'e' && c != 'E') {
//		goto L_emit;
//	}
//	if(c == '.') {
//		isFloat = true;
//		source++;
//		for (; (c = *source) != 0; ++source) {
//			if(c == '_') continue;
//			if(parseDecimalDigit(c) == -1)
//				break;
//		}
//	}
//	if(c == 'e' || c == 'E') {
//		isFloat = true;
//		c = *(++source);
//		if(!('0' <= c && c <= '9') && !(c == '+' || c == '-')) {
//			source--;
//			goto L_emit;
//		}
//		if(c == '+' || c == '-') {
//			c = *source++;
//		}
//		for (; (c = *source) != 0; ++source) {
//			if(c == '_') continue;
//			if(parseDecimalDigit(c) == -1)
//				break;
//		}
//	}
//
//	L_emit:;
//	if(IS_NOTNULL(tk)) {
//		/* skip unit */
//		for (; (c = *source) != 0; ++source) {
//			if(c == '_') continue;
//			if(!isalpha(c))
//				break;
//		}
//		end = source;
//		KFieldSet(tk, tk->text, KLIB new_kString(kctx, OnField, start, end - start, StringPolicy_ASCII));
//		tk->tokenType = isFloat ? KSymbol_("$Float") : TokenType_NUM;
//	}
//	KReturnUnboxValue(source - start);
//}
//
//static kint_t _kstrtoll(const char *p, char (*parseDigit)(char), int base)
//{
//	long long tmp = 0, prev = 0;
//	char c;
//	for (; (c = *p) != 0; ++p) {
//		if(c == '_') continue;
//		c = parseDigit(c);
//		if(c == -1)
//			break;
//		tmp = tmp * base + c;
//		if(tmp < prev) {
//			/* Overflow!! */
//			return 0;
//		}
//		prev = tmp;
//	}
//	return (kint_t) tmp;
//}
//
//static kint_t kstrtoll(const char *p)
//{
//	if(*p == '0') {
//		if(*(p+1) == 'x' || *(p+1) == 'X') {
//		return _kstrtoll(p+2, parseHexDigit, 16);
//		}
//		if(*(p+1) == 'b') {
//			return _kstrtoll(p+2, parseBinaryDigit, 2);
//		}
//		if('0' <= *(p+1) && *(p+1) <= '7') {
//			return _kstrtoll(p+1, parseOctalDigit, 8);
//		}
//	}
//	return _kstrtoll(p, parseDecimalDigit, 10);
//}
//
//static KMETHOD TypeCheck_ExtendedIntLiteral(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_TypeCheck2(stmt, expr, gma, reqc);
//	kToken *tk = expr->TermToken;
//	long long n = kstrtoll(kString_text(tk->text));
//	KReturn(SUGAR kNode_SetUnboxConst(kctx, expr, KType_int, (uintptr_t)n));
//}
//
//static kbool_t int_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
//{
//	KDEFINE_SYNTAX SYNTAX[] = {
//		{ KSymbol_NumberPattern, SYNFLAG_CTypeFunc, 0, 0, {SUGAR termParseFunc}, {SUGARFUNC TypeCheck_ExtendedIntLiteral}},
//		{ KSymbol_("~"),  0, 0, Precedence_CStylePrefixOperator, {SUGAR opParseFunc}, {SUGAR callTypeCheckFunc} },
//		{ KSymbol_("<<"), 0, Precedence_CStyleSHIFT,  0, {SUGAR opParseFunc}, {SUGAR callTypeCheckFunc}},
//		{ KSymbol_(">>"), 0, Precedence_CStyleSHIFT,  0, {SUGAR opParseFunc}, {SUGAR callTypeCheckFunc}},
//		{ KSymbol_("&"),  0, Precedence_CStyleBITAND, 0, {SUGAR opParseFunc}, {SUGAR callTypeCheckFunc}},
//		{ KSymbol_("|"),  0, Precedence_CStyleBITOR,  0, {SUGAR opParseFunc}, {SUGAR callTypeCheckFunc}},
//		{ KSymbol_("^"),  0, Precedence_CStyleBITXOR, 0, {SUGAR opParseFunc}, {SUGAR callTypeCheckFunc}},
//		{ KSymbol_END, },
//	};
//	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
//	SUGAR kNameSpace_SetTokenFunc(kctx, ns, KSymbol_NumberPattern, KonohaChar_Digit, KSugarFunc(ns, TokenFunc_ExtendedIntLiteral));
//
//	KSyntaxVar *syn = (KSyntaxVar *)SUGAR kNameSpace_GetSyntax(kctx, ns, KSymbol_("+"), 0);
//	if(syn != NULL) {
//		syn->precedence_op1  = Precedence_CStylePrefixOperator;
//	}
//	return true;
//}
//
///* ------------------------------------------------------------------------ */
///* assignment */
//
//static KMETHOD Expression_BinarySugar(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
//	kToken *opToken = tokenList->TokenItems[operatorIdx];
//	KSyntax *opSyntax = opToken->resolvedSyntaxInfo;
//	if(opSyntax->macroParamSize == 2) {
//		KTokenSeq macro = {kNode_ns(stmt), tokenList};
//		KTokenSeq_Push(kctx, macro);
//		KMacroSet macroParam[] = {
//			{KSymbol_("X"), tokenList, beginIdx, operatorIdx},
//			{KSymbol_("Y"), tokenList, operatorIdx+1, endIdx},
//			{0, NULL, 0, 0},
//		};
//		macro.TargetPolicy.RemovingIndent = true;
//		SUGAR KTokenSeq_ApplyMacro(kctx, &macro, opSyntax->macroDataNULL, 0, kArray_size(opSyntax->macroDataNULL), opSyntax->macroParamSize, macroParam);
//		kNode *expr = SUGAR ParseNewNode(kctx, stmt, macro.tokenList, macro.beginIdx, macro.endIdx, NULL);
//		KTokenSeq_Pop(kctx, macro);
//		KReturn(expr);
//	}
//}
//
//static kbool_t cstyle_defineAssign(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
//{
//	KDEFINE_SYNTAX SYNTAX[] = {
//		{ KSymbol_("+="), (SYNFLAG_NodeLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
//		{ KSymbol_("-="), (SYNFLAG_NodeLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
//		{ KSymbol_("*="), (SYNFLAG_NodeLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
//		{ KSymbol_("/="), (SYNFLAG_NodeLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
//		{ KSymbol_("%="), (SYNFLAG_NodeLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
//
//		{ KSymbol_("<<"), 0, NULL, Precedence_CStyleSHIFT,  0,                     NULL, NULL, NULL, NULL, NULL, },
//		{ KSymbol_(">>"), 0, NULL, Precedence_CStyleSHIFT,  0,                     NULL, NULL, NULL, NULL, NULL, },
//		{ KSymbol_("&"),  0, NULL, Precedence_CStyleBITAND, 0,                     NULL, NULL, NULL, NULL, NULL, },
//		{ KSymbol_("|"),  0, NULL, Precedence_CStyleBITOR,  0,                     NULL, NULL, NULL, NULL, NULL, },
//		{ KSymbol_("^"),  0, NULL, Precedence_CStyleBITXOR, 0,                     NULL, NULL, NULL, NULL, NULL, },
//
//		{ KSymbol_("|="), (SYNFLAG_NodeLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
//		{ KSymbol_("&="), (SYNFLAG_NodeLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
//		{ KSymbol_("<<="), (SYNFLAG_NodeLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
//		{ KSymbol_(">>="), (SYNFLAG_NodeLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
//		{ KSymbol_("^="), (SYNFLAG_NodeLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
//		{ KSymbol_END, },
//	};
//	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
//	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("+="), 2,  "X Y X = (X) + (Y)", false);
//	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("-="), 2,  "X Y X = (X) - (Y)", false);
//	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("*="), 2,  "X Y X = (X) * (Y)", false);
//	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("/="), 2,  "X Y X = (X) / (Y)", false);
//	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("%="), 2,  "X Y X = (X) % (Y)", false);
//	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("|="), 2,  "X Y X = (X) | (Y)", false);
//	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("&="), 2,  "X Y X = (X) & (Y)", false);
//	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("<<="), 2, "X Y X = (X) << (Y)", false);
//	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_(">>="), 2, "X Y X = (X) >> (Y)", false);
//	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("^="), 2,  "X Y X = (X) ^ (Y)", false);
//	return true;
//}
//
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
//
//
//// --------------------------------------------------------------------------
//
//static kbool_t cstyle_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
//{
//	cstyle_DefineStatement(kctx, ns, trace);
//	KDEFINE_SYNTAX defLiteral[] = {
//		{ KSymbol_("$SingleQuotedChar"), 0, NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_SingleQuotedChar, },
//		{ KSymbol_END, }, /* sentinental */
//	};
//	SUGAR kNameSpace_DefineSyntax(kctx, ns, defLiteral, trace);
//	SUGAR kNameSpace_SetTokenFunc(kctx, ns, KSymbol_("$SingleQuotedChar"), KonohaChar_Quote, KSugarFunc(ns, TokenFunc_SingleQuotedChar));
//
//	cstyle_defineExpression(kctx, ns, option, trace);
//	cstyle_defineArrayLiteral(kctx, ns, trace);
//
//	int_defineMethod(kctx, ns, trace);
//	int_defineSyntax(kctx, ns, trace);
//	cstyle_defineAssign(kctx, ns, trace);
//
//	null_PackupNameSpace(kctx, ns, trace);
//	return true;
//}
//
//static kbool_t cstyle_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
//{
//	KDEFINE_INT_CONST ClassData[] = {   // long as alias
//		{"long", VirtualType_KClass, (uintptr_t)KClass_Int},
//		{NULL},
//	};
//	KLIB kNameSpace_LoadConstData(kctx, exportNS, KConst_(ClassData), false/*isOverride*/, trace);
//	return true;
//}
//
//// --------------------------------------------------------------------------
//
//KDEFINE_PACKAGE* cstyle_Init(void)
//{
//	static KDEFINE_PACKAGE d = {0};
//	KSetPackageName(d, "cstyle", "1.0");
//	d.PackupNameSpace    = cstyle_PackupNameSpace;
//	d.ExportNameSpace   = cstyle_ExportNameSpace;
//	return &d;
//}
//
//// --------------------------------------------------------------------------
//
//#ifdef __cplusplus
//}
//#endif
