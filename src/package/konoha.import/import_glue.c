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
//#define makeStringConstValue(kctx, text) new_ConstValueExpr(kctx, TY_String, UPCAST(text))
//
//static kExpr *CreateImportCall(KonohaContext *kctx, SugarSyntaxVar *syn, kToken *tkImport, kNameSpace *ns, kString *pkgname)
//{
//	kExpr *ePKG = makeStringConstValue(kctx, pkgname);
//	kExpr *expr = SUGAR new_UntypedCallStyleExpr(kctx, syn, 3,
//			tkImport, new_ConstValueExpr(kctx, O_typeId(ns), UPCAST(ns)), ePKG);
//	return expr;
//}
//
//static KMETHOD Statement_import(KonohaContext *kctx, KonohaStack *sfp)
//{
//	int ret = false;
//	VAR_Statement(stmt, gma);
//	kTokenArray *tokenList = (kTokenArray *) kStmt_GetObjectNULL(kctx, stmt, KW_TokenPattern);
//	if(tokenList == NULL) {
//		KReturnUnboxValue(false);
//	}
//	kNameSpace *ns = Stmt_ns(stmt);
//	SugarSyntaxVar *syn = (SugarSyntaxVar *) SYN_(ns, KW_ExprMethodCall);
//	kExpr *expr;
//	kTokenVar *tkImport = /*G*/new_(TokenVar, 0, OnGcStack);
//	tkImport->resolvedSymbol = MN_("import");
//	if(IS_Token(tokenList)) {
//		kTokenArray *list = ((kToken *) tokenList)->subTokenList;
//		if(IS_String(list)) {
//			/* case: import cstyle; */
//			kString *pkgname = (kString *) list;
//			expr = CreateImportCall(kctx, syn, tkImport, ns, pkgname);
//		}
//		else if(kArray_size(list) == 1) {
//			/* case : import("konoha.import"); */
//			kExpr *param0 = makeStringConstValue(kctx, list->TokenItems[0]->text);
//			expr = SUGAR new_UntypedCallStyleExpr(kctx, syn, 3,
//					tkImport, new_ConstValueExpr(kctx, O_typeId(ns), UPCAST(ns)), param0);
//		}
//		else if(kArray_size(list) == 2) {
//			/* case : import("konoha.import", "import"); */
//			kExpr *param0 = makeStringConstValue(kctx, list->TokenItems[0]->text);
//			kExpr *param1 = makeStringConstValue(kctx, list->TokenItems[1]->text);
//			expr = SUGAR new_UntypedCallStyleExpr(kctx, syn, 4,
//					tkImport, new_ConstValueExpr(kctx, O_typeId(ns), UPCAST(ns)),
//					param0, param1);
//		} else {
//			KReturnUnboxValue(false);
//		}
//	} else {
//		KGrowingBuffer wb;
//		KLIB Kwb_Init(&(kctx->stack->cwb), &wb);
//		/* case : import konoha.import */
//		ksymbol_t star = SYM_("*");
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
//				KLIB Kwb_Write(kctx, &wb, S_text(tk->text), S_size(tk->text));
//				KLIB Kwb_Write(kctx, &wb, ".", 1);
//			}
//		}
//		kString *name = tokenList->TokenItems[i]->text;
//		KLIB Kwb_Write(kctx, &wb, S_text(name), S_size(name));
//
//		kString *pkgname = KLIB new_kString(kctx, OnGcStack, KLIB Kwb_top(kctx, &wb, 1), Kwb_bytesize(&wb), 0);
//		expr = CreateImportCall(kctx, syn, tkImport, ns, pkgname);
//	}
//	KLIB kObjectProto_SetObject(kctx, stmt, KW_ExprPattern, TY_Expr, expr);
//	ret = SUGAR kStmt_TypeCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_void, TypeCheckPolicy_ALLOWVOID);
//	if(ret) {
//		kStmt_typed(stmt, EXPR);
//	}
//	KReturnUnboxValue(ret);
//}
//
//// --------------------------------------------------------------------------
//
//static kbool_t import_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
//{
//	KDEFINE_SYNTAX SYNTAX[] = {
//		{ SYM_("import"), 0, "\"import\" $Token $Token* [ \".*\"] ", 0, 0, NULL, NULL, Statement_import, NULL, NULL, },
//		{ KW_END, },
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
