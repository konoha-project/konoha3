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

//#include <konoha2/konoha2.h>
//#include <konoha2/sugar.h>
//#include <konoha2/klib.h>
//
//// --------------------------------------------------------------------------
//
////## method @static void NameSpace.assert(boolean cond)
//static KMETHOD NameSpace_assert(CTX, ksfp_t *sfp _RIX)
//{
//	kbool_t cond = sfp[1].bvalue;
//	kline_t fileid  = sfp[K_RTNIDX].uline;
//	if (cond == false) {
//		const char *fname = SS_t(fileid);
//		uintptr_t line = ULINE_line(fileid);
//		fprintf(stderr, "Assertion!!: %s at line %lu\n", fname, line);
//	}
//	RETURNvoid_();
//}
//
//// --------------------------------------------------------------------------
//
//static KMETHOD StmtTyCheck_assert(CTX, ksfp_t *sfp _RIX)
//{
//	//FIXME
////	USING_SUGAR;
////	kbool_t r = 1;
////	VAR_StmtTyCheck(stmt, syn, gma);
////	if((r = SUGAR Stmt_tyCheckExpr(_ctx, stmt, KW_ExprPattern, gma, TY_Boolean, 0))) {
////		kExpr *expr = kStmt_expr(stmt, KW_ExprPattern, NULL);
////		kMethod *mtd = kNameSpace_getMethodNULL(gma->genv->ks, TY_NameSpace, MN_("assert"));
////		assert(expr != NULL);
////		assert(mtd != NULL);
////		kStmt_toExprCall(stmt, mtd, 2, gma->genv->ks, expr);
////		expr = kStmt_expr(stmt, KW_ExprPattern, NULL);
////		expr->build = TEXPR_CALL;
////	}
////	RETURNb_(r);
//}
//
//// --------------------------------------------------------------------------
//
//#define _Public   kMethod_Public
//#define _Static   kMethod_Static
//#define _Coercion kMethod_Coercion
//#define _F(F)   (intptr_t)(F)
//
//static kbool_t assert_initPackage(CTX, kNameSpace *ks, int argc, const char**args, kline_t pline)
//{
//	USING_SUGAR;
//	int FN_cond = FN_("cond");
//	KDEFINE_METHOD MethodData[] = {
//		_Static|_Public, _F(NameSpace_assert), TY_Int, TY_NameSpace, MN_("assert"), 1, TY_Boolean, FN_cond,
//		DEND,
//	};
//	kNameSpace_loadMethodData(ks, MethodData);
//	return true;
//}
//
//static kbool_t assert_setupPackage(CTX, kNameSpace *ks, kline_t pline)
//{
//	return true;
//}
//
//static kbool_t assert_initNameSpace(CTX,  kNameSpace *ks, kline_t pline)
//{
//	USING_SUGAR;
//	KDEFINE_SYNTAX SYNTAX[] = {
//		{ .kw = SYM_("assert"), .rule = "'assert' '(' $expr ')'", .TopStmtTyCheck = StmtTyCheck_assert, .StmtTyCheck = StmtTyCheck_assert},
//		{ .kw = KW_END, },
//	};
//	SUGAR NameSpace_defineSyntax(_ctx, ks, SYNTAX);
//
//	return true;
//}
//
//
//static kbool_t assert_setupNameSpace(CTX, kNameSpace *ks, kline_t pline)
//{
//	return true;
//}
//
//KDEFINE_PACKAGE* assert_init(void)
//{
//	static KDEFINE_PACKAGE d = {
//		KPACKNAME("assert", "1.0"),
//		.initPackage = assert_initPackage,
//		.setupPackage = assert_setupPackage,
//		.initNameSpace = assert_initNameSpace,
//		.setupNameSpace = assert_setupNameSpace,
//	};
//	return &d;
//}
