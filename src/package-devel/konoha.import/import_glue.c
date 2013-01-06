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
//#include <minikonoha/minikonoha.h>
//#include <minikonoha/sugar.h>
//
//#ifdef __cplusplus
//extern "C"{
//#endif
//
//#define makeStringConstValue(kctx, text) new_ConstNode(kctx, ns, KClass_String, UPCAST(text))
//
//static kNode *CreateImportCall(KonohaContext *kctx, kSyntaxVar *syn, kToken *tkImport, kNameSpace *ns, kString *pkgname)
//{
//	kNode *ePKG = makeStringConstValue(kctx, pkgname);
//	kNode *expr = SUGAR new_UntypedOperatorNode(kctx, syn, 3,
//			tkImport, new_ConstNode(kctx, ns, kObject_class(ns), UPCAST(ns)), ePKG);
//	return expr;
//}
//
//static KMETHOD Statement_import(KonohaContext *kctx, KonohaStack *sfp)
//{
//	int ret = false;
//	VAR_TypeCheck(stmt, ns, reqc);
//	kTokenArray *tokenList = (kTokenArray *) kNode_GetObjectNULL(kctx, stmt, KSymbol_TokenPattern);
//	if(tokenList == NULL) {
//		KReturnUnboxValue(false);
//	}
//	kNameSpace *ns = kNode_ns(stmt);
//	kSyntaxVar *syn = (kSyntaxVar *) kSyntax_(ns, KSymbol_ParamPattern/*MethodCall*/);
//	kNode *expr;
//	kTokenVar *tkImport = /*G*/new_(TokenVar, 0, OnGcStack);
//	tkImport->symbol = KMethodName_("import");
//	if(IS_Token(tokenList)) {
//		kTokenArray *list = ((kToken *) tokenList)->GroupTokenList;
//		if(IS_String(list)) {
//			/* case: import cstyle; */
//			kString *pkgname = (kString *) list;
//			expr = CreateImportCall(kctx, syn, tkImport, ns, pkgname);
//		}
//		else if(kArray_size(list) == 1) {
//			/* case : import("konoha.import"); */
//			kNode *param0 = makeStringConstValue(kctx, list->TokenItems[0]->text);
//			expr = SUGAR new_UntypedOperatorNode(kctx, syn, 3,
//					tkImport, new_ConstNode(kctx, ns, kObject_class(ns), UPCAST(ns)), param0);
//		}
//		else if(kArray_size(list) == 2) {
//			/* case : import("konoha.import", "import"); */
//			kNode *param0 = makeStringConstValue(kctx, list->TokenItems[0]->text);
//			kNode *param1 = makeStringConstValue(kctx, list->TokenItems[1]->text);
//			expr = SUGAR new_UntypedOperatorNode(kctx, syn, 4,
//					tkImport, new_ConstNode(kctx, ns, kObject_class(ns), UPCAST(ns)),
//					param0, param1);
//		} else {
//			KReturnUnboxValue(false);
//		}
//	} else {
//		KBuffer wb;
//		KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
//		/* case : import konoha.import */
//		ksymbol_t star = KSymbol_("*");
//		size_t i = 0;
//		if(i + 2 < kArray_size(tokenList)) {
//			for (; i < kArray_size(tokenList)-1; i+=2) {
//				/* name . */
//				kToken *tk  = tokenList->TokenItems[i+0];
//				if(i+2 < kArray_size(tokenList)) {
//					kToken *startTk = tokenList->TokenItems[i+2];
//					if(startTk->resolvedSyntaxInfo->keyword == star) {
//						break;
//					}
//				}
//				KLIB KBuffer_Write(kctx, &wb, kString_text(tk->text), kString_size(tk->text));
//				KLIB KBuffer_Write(kctx, &wb, ".", 1);
//			}
//		}
//		kString *name = tokenList->TokenItems[i]->text;
//		KLIB KBuffer_Write(kctx, &wb, kString_text(name), kString_size(name));
//
//		kString *pkgname = KLIB new_kString(kctx, OnGcStack, KLIB KBuffer_text(kctx, &wb, 1), KBuffer_bytesize(&wb), 0);
//		expr = CreateImportCall(kctx, syn, tkImport, ns, pkgname);
//	}
//	KLIB kObjectProto_SetObject(kctx, stmt, KSymbol_NodePattern, KType_Node, expr);
//	ret = SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_NodePattern, ns, KClass_void, TypeCheckPolicy_AllowVoid);
//	if(ret) {
//		kNode_Type(kctx, stmt, EXPR);
//	}
//	KReturnUnboxValue(ret);
//}
//
//// --------------------------------------------------------------------------
//
//static kbool_t import_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
//{
//	KDEFINE_SYNTAX SYNTAX[] = {
//		{ KSymbol_("import"), 0, "\"import\" $Token $Token* [ \".*\"] ", 0, 0, NULL, NULL, Statement_import, NULL, NULL, },
//		{ KSymbol_END, },
//	};
//	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
//	return true;
//}
//
//static kbool_t import_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
//{
//	return true;
//}
//
//KDEFINE_PACKAGE* import_Init(void)
//{
//	static KDEFINE_PACKAGE d = {0};
//	KSetPackageName(d, "import", "1.0");
//	d.PackupNameSpace    = import_PackupNameSpace;
//	d.ExportNameSpace   = import_ExportNameSpace;
//	return &d;
//}
//
//#ifdef __cplusplus
//}
//#endif
