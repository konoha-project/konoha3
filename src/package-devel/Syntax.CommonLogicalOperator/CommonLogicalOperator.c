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
// * AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
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
//#include <konoha3/import/methoddecl.h>
//
//#ifdef __cplusplus
//extern "C" {
//#endif
//
///* ------------------------------------------------------------------------ */
///* CommonLogicalOperator ( or, not, and) */
//static KMETHOD Expression_PreUnarySugar(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
//	kToken *opToken = tokenList->TokenItems[operatorIdx];
//	KSyntax *opSyntax = opToken->resolvedSyntaxInfo;
//	if(opSyntax->macroParamSize == 1) {
//		KTokenSeq macro = {Stmt_ns(stmt), tokenList};
//		KTokenSeq_Push(kctx, macro);
//		KMacroSet macroParam[] = {
//			{KSymbol_("X"), tokenList, operatorIdx+1, endIdx},
//			{0, NULL, 0, 0},
//		};
//		macro.TargetPolicy.RemovingIndent = true;
//		SUGAR KTokenSeq_ApplyMacro(kctx, &macro, opSyntax->macroDataNULL, 0, kArray_size(opSyntax->macroDataNULL), opSyntax->macroParamSize, macroParam);
//		kExpr *expr = SUGAR kStmt_ParseExpr(kctx, stmt, macro.tokenList, macro.beginIdx, macro.endIdx, NULL);
//		KTokenSeq_Pop(kctx, macro);
//		KReturn(expr);
//	}
//}
//
//static KMETHOD Expression_BinarySugar(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
//	kToken *opToken = tokenList->TokenItems[operatorIdx];
//	KSyntax *opSyntax = opToken->resolvedSyntaxInfo;
//	if(opSyntax->macroParamSize == 2) {
//		KTokenSeq macro = {Stmt_ns(stmt), tokenList};
//		KTokenSeq_Push(kctx, macro);
//		KMacroSet macroParam[] = {
//			{KSymbol_("X"), tokenList, beginIdx, operatorIdx},
//			{KSymbol_("Y"), tokenList, operatorIdx+1, endIdx},
//			{0, NULL, 0, 0},
//		};
//		macro.TargetPolicy.RemovingIndent = true;
//		SUGAR KTokenSeq_ApplyMacro(kctx, &macro, opSyntax->macroDataNULL, 0, kArray_size(opSyntax->macroDataNULL), opSyntax->macroParamSize, macroParam);
//		kExpr *expr = SUGAR kStmt_ParseExpr(kctx, stmt, macro.tokenList, macro.beginIdx, macro.endIdx, NULL);
//		KTokenSeq_Pop(kctx, macro);
//		KReturn(expr);
//	}
//}
//
//
//// --------------------------------------------------------------------------
//static kbool_t CommonLogicalOperator_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
//{
//	KDEFINE_SYNTAX SYNTAX[] = {
//		{ KSymbol_("and"), SYNFLAG_ExprLeftJoinOp2, NULL, Precedence_CStyleAND, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
//		{ KSymbol_("or"), SYNFLAG_ExprLeftJoinOp2, NULL, Precedence_CStyleOR, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
//		{ KSymbol_("not"), 0, NULL, 0, Precedence_CStylePREUNARY, NULL, Expression_PreUnarySugar, },
//		{ KSymbol_END, },
//	};
//	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
//	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("and"), 2, "X Y (X) && (Y)", false);
//	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("or"), 2, "X Y (X) || (Y)", false);
//	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("not"), 1, "X !(X)", false);
//	return true;
//}
//
//static kbool_t CommonLogicalOperator_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
//{
//	return true;
//}
//
//KDEFINE_PACKAGE *CommonLogicalOperator_Init(void)
//{
//	static KDEFINE_PACKAGE d = {0};
//	KSetPackageName(d, "CommonLogicalOperator", "0.0");
//	d.PackupNameSpace = CommonLogicalOperator_PackupNameSpace;
//	d.ExportNameSpace = CommonLogicalOperator_ExportNameSpace;
//	return &d;
//}
//
//#ifdef __cplusplus
//} /* extern "C" */
//#endif
