///****************************************************************************
// * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
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
//#include <konoha3/konoha.h>
//#include <konoha3/sugar.h>
//#include <konoha3/klib.h>
//#include <konoha3/konoha_common.h>
//
//#ifdef __cplusplus
//extern "C" {
//#endif
//
//// ---------------------------------------------------------------------------
//
//#define KType_IsIterator(T)     (KClass_(T)->baseTypeId == KType_Iterator)
//
//static kToken* new_TypeToken(KonohaContext *kctx, kNameSpace *ns, ktypeattr_t typeId)
//{
//	kToken *TypeToken = new_(Token, 0, OnGcStack);
//	kToken_SetTypeId(kctx, TypeToken, ns, typeId);
//	return TypeToken;
//}
//
//static kToken* new_ParsedNodeToken(KonohaContext *kctx, kNameSpace *ns, kNode *expr)
//{
//	kTokenVar *ParsedNodeToken = new_(TokenVar, 0, OnGcStack);
//	ParsedNodeToken->resolvedSyntaxInfo = kSyntax_(ns, KSymbol_NodePattern);
//	KFieldSet(ParsedNodeToken, ParsedNodeToken->parsedNode, expr);
//	return (kToken *)ParsedNodeToken;
//}
//
//static void KMacroSet_SetTokenAt(KonohaContext *kctx, KMacroSet *macroSet, int index, kArray *tokenList, const char *symbol, ...)
//{
//	DBG_ASSERT(macroSet[index].tokenList == NULL);
//	macroSet[index].symbol = KLIB Ksymbol(kctx, symbol, strlen(symbol), StringPolicy_TEXT|StringPolicy_ASCII, _NEWID);
//	macroSet[index].tokenList = tokenList;
//	macroSet[index].beginIdx = kArray_size(tokenList);
//	kToken *tk;
//	va_list ap;
//	va_start(ap , symbol);
//	while((tk = va_arg(ap, kToken *)) != NULL) {
//		DBG_ASSERT(IS_Token(tk));
//		KLIB kArray_Add(kctx, tokenList, tk);
//	}
//	va_end(ap);
//	macroSet[index].endIdx = kArray_size(tokenList);
//}
//
///* This implementation is a little tricky (by kimio)
// * The syntax of loop is defined as if statement
// * Typechecking is overloaded as while statement (@see while_glue);
// */
//
//static kNode *new_MacroNode(KonohaContext *kctx, kNode *stmt, kToken *IteratorTypeToken, kToken *IteratorNodeToken, kToken *TypeToken, kToken *VariableToken)
//{
//	kNameSpace *ns = kNode_ns(stmt);
//	KTokenSeq source = {ns, KGetParserContext(kctx)->preparedTokenList};
//	KTokenSeq_Push(kctx, source);
//	/* FIXME(imasahiro)
//	 * we need to implement template as Node
//	 * "T _ = E; if(_.hasNext()) { N = _.next(); }"
//	 *                           ^^^^^^^^^^^^^^^^^
//	 */
//	SUGAR KTokenSeq_Tokenize(kctx, &source, "T _ = E; if(_.hasNext()) N = _.next();", 0);
//	KMacroSet macroSet[4] = {{0, NULL, 0, 0}};
//	KMacroSet_SetTokenAt(kctx, macroSet, 0, source.tokenList, "T", IteratorTypeToken, NULL);
//	KMacroSet_SetTokenAt(kctx, macroSet, 1, source.tokenList, "E", IteratorNodeToken, NULL);
//	if(TypeToken == NULL) {
//		KMacroSet_SetTokenAt(kctx, macroSet, 2, source.tokenList, "N", VariableToken, NULL);
//	}
//	else {
//		KMacroSet_SetTokenAt(kctx, macroSet, 2, source.tokenList, "N", TypeToken, VariableToken, NULL);
//	}
//	kNode *bk = SUGAR new_BlockNode(kctx, stmt, macroSet, &source);
//	KTokenSeq_Pop(kctx, source);
//	return bk;
//}
//
//static void kNode_appendNode(KonohaContext *kctx, kNode *stmt, kNode *bk)
//{
//	if(bk != NULL) {
//		kNode *block = SUGAR kNode_GetNode(kctx, stmt, kNode_ns(stmt), KSymbol_NodePattern, NULL);
//		size_t i;
//		for(i = 0; i < kArray_size(bk->NodeList); i++) {
//			KLIB kArray_Add(kctx, block->NodeList, bk->NodeList->NodeItems[i]);
//		}
//	}
//}
//
//static KMETHOD Statement_for(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_TypeCheck(stmt, ns, reqc);
//	DBG_P("for statement .. ");
//	int isOkay = false;
//	if(SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_NodePattern, ns, KClass_INFER, 0)) {
//		kNameSpace *ns = kNode_ns(stmt);
//		kToken *TypeToken = SUGAR kNode_GetToken(kctx, stmt, KSymbol_TypePattern, NULL);
//		kToken *VariableToken  = SUGAR kNode_GetToken(kctx, stmt, KSymbol_SymbolPattern, NULL);
//		DBG_P("typeToken=%p, varToken=%p", TypeToken, VariableToken);
//		kNode *IteratorNode = SUGAR kNode_GetNode(kctx, stmt, KSymbol_NodePattern, NULL);
//		if(!KType_IsIterator(IteratorNode->attrTypeId)) {
//			kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, KClass_(IteratorNode->attrTypeId), KMethodName_To(KType_Iterator), 0, KMethodMatch_NoOption);
//			if(mtd == NULL) {
//				SUGAR MessageNode(kctx, stmt, IteratorNode, ErrTag, "expected Iterator expression after in");
//				KReturnUnboxValue(false);
//			}
//			IteratorNode = SUGAR new_MethodNode(kctx, stmt, ns, KClass_INFER, mtd, 1, IteratorNode);
//			kNode_SetObject(kctx, stmt, KSymbol_NodePattern, IteratorNode);
//		}
//		kNode *block = new_MacroNode(kctx, stmt, new_TypeToken(kctx, ns, KClass_(IteratorNode->attrTypeId)), new_ParsedNodeToken(kctx, ns, IteratorNode), TypeToken, VariableToken);
//		kNode *IfNode = block->NodeList->NodeItems[1]; // @see macro;
//		kNode_appendNode(kctx, IfNode, SUGAR kNode_GetNode(kctx, stmt, ns, KSymbol_NodePattern, NULL));
//		kNode_Set(CatchBreak, IfNode, true);
//		kNode_Set(CatchContinue, IfNode, true);
//		isOkay = SUGAR TypeCheckBlock(kctx, block, gma);
//		if(isOkay) {
//			kNode_Type(kctx, IfNode, LOOP);
//			kNode_SetObject(kctx, stmt, KSymbol_NodePattern, block);
//			kNode_Type(kctx, stmt, BLOCK);
//		}
//	}
//	KReturnUnboxValue(isOkay);
//}
//
//static kbool_t foreach_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
//{
//	KDEFINE_SYNTAX SYNTAX[] = {
//		{ KSymbol_("for"), 0, "\"for\" \"(\" [$Type] $Symbol \"in\" $Node  \")\" [$Node] ", 0, 0, NULL, NULL, NULL, Statement_for, NULL, },
//		{ KSymbol_END, },
//	};
//	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
//	return true;
//}
//
//static kbool_t foreach_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
//{
//	KRequirePackage("konoha.iterator", trace);
//	KImportPackageSymbol(ns, "cstyle", "break", trace);
//	KImportPackageSymbol(ns, "cstyle", "continue", trace);
//	foreach_defineSyntax(kctx, ns, trace);
//	return true;
//}
//
//static kbool_t foreach_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
//{
//	return true;
//}
//
//
//KDEFINE_PACKAGE *foreach_Init(void)
//{
//	static KDEFINE_PACKAGE d = {0};
//	KSetPackageName(d, "konoha", "1.0");
//	d.PackupNameSpace    = foreach_PackupNameSpace;
//	d.ExportNameSpace   = foreach_ExportNameSpace;
//	return &d;
//}
//
//#ifdef __cplusplus
//}
//#endif
