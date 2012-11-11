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

//----------------------------------------------------------------------------

static KMETHOD TypeCheck_Defined(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	size_t i;
	kbool_t isDefined = true;
	SugarContext *sugarContext = GetSugarContext(kctx);
	int popIsBlockingErrorMessage = sugarContext->isBlockedErrorMessage;
	sugarContext->isBlockedErrorMessage = true;
	for(i = 1; i < kArray_size(expr->cons); i++) {
		kExpr *typedExpr = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, i, gma, TY_var, TypeCheckPolicy_ALLOWVOID);
		if(typedExpr == K_NULLEXPR) {
			isDefined = false;
			break;
		}
	}
	sugarContext->isBlockedErrorMessage = popIsBlockingErrorMessage;
	KReturn(SUGAR kExpr_setUnboxConstValue(kctx, expr, TY_boolean, isDefined));
}

static void filterArrayList(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx)
{
	int i;
	for(i = beginIdx; i < endIdx; i++) {
		if(i + 1 == endIdx || tokenList->TokenItems[i+1]->resolvedSyntaxInfo->keyword == KW_COMMA) {
			kTokenVar *tk = tokenList->TokenVarItems[i];
			if(tk->resolvedSyntaxInfo->keyword != KW_SymbolPattern) {  // defined
				tk->resolvedSyntaxInfo = SYN_(ns, KW_TextPattern);  // switch to text pattern
			}
			i++;
		}
		while(i < endIdx) {
			kTokenVar *tk = tokenList->TokenVarItems[i];
			i++;
			if(tk->resolvedSyntaxInfo->keyword == KW_COMMA) break;
		}
	}
}


static KMETHOD Expression_Defined(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, currentIdx, endIdx);
	if(beginIdx == currentIdx && beginIdx + 1 < endIdx) {
		kTokenVar *definedToken = tokenList->TokenVarItems[beginIdx];   // defined
		kTokenVar *pToken = tokenList->TokenVarItems[beginIdx+1];
		if(IS_Array(pToken->subTokenList)) {
			kExpr *expr = SUGAR new_UntypedCallStyleExpr(kctx, definedToken->resolvedSyntaxInfo, 1, definedToken);
			filterArrayList(kctx, Stmt_ns(stmt), pToken->subTokenList, 0, kArray_size(pToken->subTokenList));
			KReturn(SUGAR kStmt_addExprParam(kctx, stmt, expr, pToken->subTokenList, 0, kArray_size(pToken->subTokenList), 0/*isAllowEmpty*/));
		}
	}
}

static kbool_t defined_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("defined"), 0, NULL, 0, Precedence_CStylePREUNARY, NULL, Expression_Defined, NULL, NULL, TypeCheck_Defined, },
		{ KW_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}

static kbool_t defined_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}


KDEFINE_PACKAGE* defined_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", "1.0");
	d.PackupNameSpace    = defined_PackupNameSpace;
	d.ExportNameSpace   = defined_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
