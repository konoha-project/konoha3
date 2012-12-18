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

static void DeclVariable(KonohaContext *kctx, kStmt *stmt, kGamma *gma, ktypeattr_t ty, kExpr *termExpr)
{
	DBG_ASSERT(kExpr_isSymbolTerm(termExpr));
	kToken *termToken = termExpr->TermToken;
	if(Gamma_isTopLevel(gma)) {
		kNameSpace *ns = Stmt_ns(stmt);
		if(ns->globalObjectNULL_OnList == NULL) {
			kStmtToken_Message(kctx, stmt, termToken, ErrTag, "unavailable global variable");
			return;
		}
		kStmtToken_Message(kctx, stmt, termToken, InfoTag, "global variable %s%s has type %s", KSymbol_Fmt2(termToken->resolvedSymbol), KType_text(ty));
		KLIB KClass_AddField(kctx, kObject_class(ns->globalObjectNULL_OnList), ty, termToken->resolvedSymbol);
	}
	else {
		kStmtToken_Message(kctx, stmt, termToken, InfoTag, "%s%s has type %s", KSymbol_Fmt2(termToken->resolvedSymbol), KType_text(ty));
		SUGAR kGamma_AddLocalVariable(kctx, gma, ty, termToken->resolvedSymbol);
	}
}

static KMETHOD TypeCheck_UntypedAssign(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kExprVar *leftHandExpr = (kExprVar *)kExpr_At(expr, 1);
	if(kExpr_isSymbolTerm(leftHandExpr)) {
		kExpr *texpr = SUGAR kStmt_TypeCheckVariableNULL(kctx, stmt, leftHandExpr, gma, KClass_INFER);
		if(texpr == NULL) {
			kExpr *rightHandExpr = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 2, gma, KClass_INFER, 0);
			if(rightHandExpr != K_NULLEXPR) {
				DeclVariable(kctx, stmt, gma, rightHandExpr->attrTypeId, leftHandExpr);
			}
		}
		else {
			KFieldSet(expr->NodeList, expr->NodeList->ExprItems[1], texpr);
		}
	}
}

static kbool_t untyped_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequirePackage("konoha.var", trace);
	SUGAR kNameSpace_AddSugarFunc(kctx, ns, KSymbol_("="), SugarFunc_TypeCheck, new_SugarFunc(ns, TypeCheck_UntypedAssign));
	return true;
}

static kbool_t untyped_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *untyped_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", "1.0");
	d.PackupNameSpace    = untyped_PackupNameSpace;
	d.ExportNameSpace   = untyped_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif