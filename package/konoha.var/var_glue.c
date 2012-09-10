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

#include<minikonoha/minikonoha.h>
#include<minikonoha/sugar.h>

// --------------------------------------------------------------------------

static	kbool_t var_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KDEFINE_INT_CONST ClassData[] = {   // add Array as available
		{"var", TY_TYPE, (uintptr_t)CT_(TY_var)},
		{NULL},
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(ClassData), 0);
	return true;
}

static kbool_t var_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

//static inline ksymbol_t tosymbol(KonohaContext *kctx, kExpr *expr)
//{
//	if(Expr_isSymbolTerm(expr)) {
//		return expr->termToken->resolvedSymbol;
//	}
//	return SYM_NONAME;
//}
//
//static kbool_t kStmt_inferDeclType(KonohaContext *kctx, kStmt *stmt, kGamma *gma, kExpr *declExpr, TypeDeclFunc TypeDecl, kStmt **lastStmtRef)
//{
//	if(declExpr->syn->keyword == KW_COMMA) {
//		size_t i;
//		for(i = 1; i < kArray_size(declExpr->cons); i++) {
//			if(!kStmt_inferDeclType(kctx, stmt, gma, kExpr_at(declExpr, i), TypeDecl, lastStmtRef)) return false;
//		}
//		return true;
//	}
//	else if(declExpr->syn->keyword == KW_LET && Expr_isSymbolTerm(kExpr_at(declExpr, 1))) {
//		if(SUGAR kStmt_tyCheckExprAt(kctx, stmt, declExpr, 2, gma, TY_var, 0) == K_NULLEXPR) {
//			// this is neccesarry to avoid 'int a = a + 1;';
//			return false;
//		}
//		kToken *termToken = kExpr_at(declExpr, 1)->termToken;
//		ktype_t inferedType = kExpr_at(declExpr, 2)->ty;
//		SUGAR kStmt_printMessage2(kctx, stmt, termToken, InfoTag, "%s has type %s", SYM_t(termToken->resolvedSymbol), TY_t(inferedType));
//		return SUGAR kStmt_declType(kctx, stmt, gma, inferedType, declExpr, TypeDecl, lastStmtRef);
//	}
//	if(Expr_isSymbolTerm(declExpr)) {
//		kToken *termToken = declExpr->termToken;
//		SUGAR kStmt_printMessage2(kctx, stmt, termToken, WarnTag, "var %s: an initial value is expected", SYM_t(termToken->resolvedSymbol));
//		SUGAR kStmt_printMessage2(kctx, stmt, termToken, InfoTag, "%s has type %s", SYM_t(termToken->resolvedSymbol), TY_t(TY_Object));
//	}
//	return SUGAR kStmt_declType(kctx, stmt, gma, TY_Object, declExpr, TypeDecl, lastStmtRef);
//}
//
//static KMETHOD StmtTyCheck_var(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_StmtTyCheck(stmt, gma);
//	DBG_P("var assign .. ");
//	kExpr  *declExpr = SUGAR kStmt_getExpr(kctx, stmt, KW_ExprPattern, NULL);
//	RETURNb_(kStmt_inferDeclType(kctx, stmt, gma, declExpr, NULL, &stmt));
//}

static kbool_t var_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
//	KDEFINE_SYNTAX SYNTAX[] = {
//		{ .keyword = SYM_("var"), TopStmtTyCheck_(var), StmtTyCheck_(var), .rule = "\"var\" $Expr", },
//		{ .keyword = KW_END, },
//	};
//	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNameSpace);
	return true;
}

static kbool_t var_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* var_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("konoha", "1.0"),
		.initPackage = var_initPackage,
		.setupPackage = var_setupPackage,
		.initNameSpace = var_initNameSpace,
		.setupNameSpace = var_setupNameSpace,
	};
	return &d;
}
