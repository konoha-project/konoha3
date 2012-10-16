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
// --------------------------------------------------------------------------

static kbool_t dollar_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	return true;
}

static kbool_t dollar_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

//static kExpr* Expression_DollarSymbol(KonohaContext *kctx, kStmt *stmt, kToken *tk)
//{
//
//}


static KMETHOD Expression_dollar(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, currentIdx, endIdx);
	DBG_ASSERT(beginIdx == currentIdx);
	if(currentIdx + 1 < endIdx) {
		kToken *nextToken = tokenList->TokenItems[currentIdx+1];
		DBG_P("nextToken='%s'", S_text(nextToken->text));
//		if(nextToken->resolvedSyntaxInfo->keyword == KW_SymbolPattern) {
//			KReturn(Expression_DollarSymbol(kctx, stmt, nextToken));
//		}

	}
//	KonohaClass *foundClass = NULL;
//	int nextIdx = SUGAR TokenUtils_parseTypePattern(kctx, stmt, Stmt_nameSpace(stmt), tokenList, beginIdx + 1, endIdx, &foundClass);
//	if(nextIdx != -1 && nextIdx < kArray_size(tokenList)) {
//		kToken *nextTokenAfterClassName = tokenList->TokenItems[nextIdx];
////		if(ct->typeId == TY_void) {
////			KReturn(SUGAR kStmt_printMessage2(kctx, stmt, tk1, ErrTag, "undefined class: %s", S_text(tk1->text)));
////		} else if(CT_is(Virtual, ct)) {
////			SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, "invalid application of 'dollar' to incomplete class %s", CT_t(ct));
////		}
//		if(nextTokenAfterClassName->resolvedSyntaxInfo->keyword == KW_ParenthesisGroup) {  // dollar C (...)
//			SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall);
//			kExpr *expr = SUGAR dollar_UntypedCallStyleExpr(kctx, syn, 2, dollarToken, NewExpr(kctx, syn, tokenList->TokenVarItems[beginIdx+1], foundClass->typeId));
//			dollarToken->resolvedSymbol = MN_dollar;
//			KReturn(expr);
//		}
//		if(nextTokenAfterClassName->resolvedSyntaxInfo->keyword == KW_BracketGroup) {     // dollar int [100]
//			SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), SYM_("dollar"));
//			KonohaClass *arrayClass = CT_p0(kctx, CT_Array, foundClass->typeId);
//			dollarToken->resolvedSymbol = MN_("dollarArray");
//			kExpr *expr = SUGAR dollar_UntypedCallStyleExpr(kctx, syn, 2, dollarToken, NewExpr(kctx, syn, tokenList->TokenVarItems[beginIdx+1], arrayClass->typeId));
//			KReturn(expr);
//		}
//	}
}

// ----------------------------------------------------------------------------
/* define class */

static kbool_t dollar_initNameSpace(KonohaContext *kctx, kNameSpace *packageNS, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("$"), 0, NULL, 0, Precedence_CStyleCALL, NULL, Expression_dollar, NULL, NULL, NULL, },
		{ KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNS);
	return true;
}

static kbool_t dollar_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNS, kNameSpace *ns, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* dollar_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "dscript", "1.0");
	d.initPackage    = dollar_initPackage;
	d.setupPackage   = dollar_setupPackage;
	d.initNameSpace  = dollar_initNameSpace;
	d.setupNameSpace = dollar_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
