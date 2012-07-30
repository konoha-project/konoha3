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
//	kToken *tk = expr->termToken;
//	sfp[4].definedValue = strtod(S_text(tk->text), NULL);   // just using tramsformation defined
//	RETURN_(SUGAR kExpr_setUnboxConstValue(kctx, expr, TY_Float, sfp[4].unboxValue));
}

//static KMETHOD ExprTyCheck_Defined(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_ExprTyCheck(stmt, expr, gma, reqty);
//	DBG_ASSERT(IS_Expr(kExpr_at(expr, 0)));
//	DBG_ASSERT(expr->cons->objectItems[1] == K_NULL);
//	if(Expr_isSymbolTerm(kExpr_at(expr, 0))) {
//		kMethod *mtd = Expr_lookUpFuncOrMethod(kctx, Stmt_nameSpace(stmt), expr, gma, reqty);
//		if(mtd != NULL) {
//			if(Method_isOverloaded(mtd)) {
//				DBG_P("overloaded found %s.%s%s", Method_t(mtd));
//				mtd = lookupOverloadedMethod(kctx, stmt, expr, mtd, gma);
//			}
//			RETURN_(kStmt_tyCheckCallParamExpr(kctx, stmt, expr, mtd, gma, reqty));
//		}
//		if(!TY_isFunc(kExpr_at(expr, 0)->ty)) {
//			kToken *tk = kExpr_at(expr, 0)->termToken;
//			DBG_ASSERT(IS_Token(tk));  // TODO: make error message in case of not Token
//			RETURN_(kToken_p(stmt, tk, ErrTag, "undefined function: %s", Token_text(tk)));
//		}
//	}
//	else {
//		if(kStmt_tyCheckByNameAt(kctx, stmt, expr, 0, gma, TY_var, 0) != K_NULLEXPR) {
//			if(!TY_isFunc(expr->cons->exprItems[0]->ty)) {
//				RETURN_(kExpr_p(stmt, expr, ErrTag, "function is expected"));
//			}
//		}
//	}
//	RETURN_(Expr_tyCheckFuncParams(kctx, stmt, expr, CT_(kExpr_at(expr, 0)->ty), gma));
//}


static kbool_t defined_initNameSpace(KonohaContext *kctx,  kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("defined"), .precedence_op1 = 300, ExprTyCheck_(Defined), },
		{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX);
	return true;
}

static kbool_t defined_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* defined_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("defined", "1.0"),
		.initPackage = defined_initPackage,
		.setupPackage = defined_setupPackage,
		.initNameSpace = defined_initNameSpace,
		.setupNameSpace = defined_setupNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
