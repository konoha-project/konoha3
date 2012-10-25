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
#include <minikonoha/sugar.h>

#ifdef __cplusplus
extern "C"{
#endif

// --------------------------------------------------------------------------
/* Decl */

static void DeclVariable(KonohaContext *kctx, kStmt *stmt, kGamma *gma, ktype_t ty, kExpr *termExpr)
{
	DBG_ASSERT(Expr_isSymbolTerm(termExpr));
	kToken *termToken = termExpr->termToken;
	if(Gamma_isTopLevel(gma)) {
		kNameSpace *ns = Stmt_nameSpace(stmt);
		if(ns->globalObjectNULL_OnList == NULL) {
			kStmtToken_printMessage(kctx, stmt, termToken, ErrTag, "unavailable global variable");
			return;
		}
		kStmtToken_printMessage(kctx, stmt, termToken, InfoTag, "global variable %s%s has type %s", PSYM_t(termToken->resolvedSymbol), TY_t(ty));
		KLIB KonohaClass_addField(kctx, O_ct(ns->globalObjectNULL_OnList), kField_Getter|kField_Setter, ty, termToken->resolvedSymbol);
	}
	else {
		kStmtToken_printMessage(kctx, stmt, termToken, InfoTag, "%s%s has type %s", PSYM_t(termToken->resolvedSymbol), TY_t(ty));
		SUGAR kGamma_declareLocalVariable(kctx, gma, ty, termToken->resolvedSymbol);
	}
}

static KMETHOD TypeCheck_UntypedAssign(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kExpr *leftHandExpr = kExpr_at(expr, 1);
	if(Expr_isSymbolTerm(leftHandExpr)) {
		kExpr *texpr = SUGAR kStmt_tyCheckVariableNULL(kctx, stmt, leftHandExpr, gma, TY_var);
		if(texpr == NULL) {
			kExpr *rightHandExpr = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 2, gma, TY_var, 0);
			if(rightHandExpr != K_NULLEXPR) {
				DeclVariable(kctx, stmt, gma, rightHandExpr->ty, leftHandExpr);
			}
		}
		else {
			KFieldSet(expr->cons, expr->cons->ExprItems[1], texpr);
		}
	}
}

static kbool_t untyped_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	KRequirePackage("konoha.var", trace);
	SUGAR kNameSpace_addSugarFunc(kctx, ns, SYM_("="), SugarFunc_TypeCheck, new_SugarFunc(ns, TypeCheck_UntypedAssign));
	return true;
}

static kbool_t untyped_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* untyped_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", "1.0");
	d.initPackage    = untyped_initPackage;
	d.setupPackage   = untyped_setupPackage;
	return &d;
}

#ifdef __cplusplus
}
#endif
