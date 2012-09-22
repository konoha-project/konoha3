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
extern "C" {
#endif

/* ------------------------------------------------------------------------ */

static kbool_t defined_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	return true;
}

static kbool_t defined_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

//----------------------------------------------------------------------------

static KMETHOD ExprTyCheck_Defined(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	size_t i;
	kbool_t isDefined = true;
	SugarContext *sugarContext = KonohaContext_getSugarContext(kctx);
	int popIsBlockingErrorMessage = sugarContext->isBlockedErrorMessage;
	sugarContext->isBlockedErrorMessage = true;
	for(i = 1; i < kArray_size(expr->cons); i++) {
		kExpr *typedExpr = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, i, gma, TY_var, TPOL_ALLOWVOID);
		if(typedExpr == K_NULLEXPR) {
			isDefined = false;
			break;
		}
	}
	sugarContext->isBlockedErrorMessage = popIsBlockingErrorMessage;
	RETURN_(SUGAR kExpr_setUnboxConstValue(kctx, expr, TY_boolean, isDefined));
}

static void filterArrayList(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx)
{
	int i;
	for(i = beginIdx; i < endIdx; i++) {
		if(i + 1 == endIdx || tokenList->tokenItems[i+1]->resolvedSyntaxInfo->keyword == KW_COMMA) {
			kTokenVar *tk = tokenList->tokenVarItems[i];
			if(tk->resolvedSyntaxInfo->keyword != KW_SymbolPattern) {  // defined
				tk->resolvedSyntaxInfo = SYN_(ns, KW_TextPattern);  // switch to text pattern
			}
			i++;
		}
		while(i < endIdx) {
			kTokenVar *tk = tokenList->tokenVarItems[i];
			i++;
			if(tk->resolvedSyntaxInfo->keyword == KW_COMMA) break;
		}
	}
}


static KMETHOD ParseExpr_Defined(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tokenList, beginIdx, currentIdx, endIdx);
	if(beginIdx == currentIdx && beginIdx + 1 < endIdx) {
		kTokenVar *definedToken = tokenList->tokenVarItems[beginIdx];   // defined
		kTokenVar *pToken = tokenList->tokenVarItems[beginIdx+1];
		if(IS_Array(pToken->subTokenList)) {
			kExpr *expr = SUGAR new_UntypedCallStyleExpr(kctx, definedToken->resolvedSyntaxInfo, 1, definedToken);
			filterArrayList(kctx, Stmt_nameSpace(stmt), pToken->subTokenList, 0, kArray_size(pToken->subTokenList));
			RETURN_(SUGAR kStmt_addExprParam(kctx, stmt, expr, pToken->subTokenList, 0, kArray_size(pToken->subTokenList), 0/*isAllowEmpty*/));
		}
	}
}

static kbool_t defined_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("defined"), .precedence_op1 = C_PRECEDENCE_PREUNARY, ParseExpr_(Defined), ExprTyCheck_(Defined), },
		{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNameSpace);
	return true;
}

static kbool_t defined_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* defined_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("defined", "1.0"),
		.initPackage    = defined_initPackage,
		.setupPackage   = defined_setupPackage,
		.initNameSpace  = defined_initNameSpace,
		.setupNameSpace = defined_setupNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
