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

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Hidden   kMethod_Hidden
#define _Imm    kMethod_Immutable
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

static	kbool_t new_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	return true;
}

static kbool_t new_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

// --------------------------------------------------------------------------

static kExpr* NewExpr(KonohaContext *kctx, SugarSyntax *syn, kToken *tk, ktype_t ty)
{
	kExprVar *expr = GCSAFE_new(ExprVar, syn);
	KSETv(expr, expr->termToken, tk);
	Expr_setTerm(expr, 1);
	expr->build = TEXPR_NEW;
	expr->ty = ty;
	return (kExpr*)expr;
}

static KMETHOD ParseExpr_new(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tokenArray, beginIdx, currentIdx, endIdx);
	DBG_ASSERT(beginIdx == currentIdx);
	kTokenVar *newToken = tokenArray->tokenVarItems[beginIdx];   // new Class (
	KonohaClass *foundClass = NULL;
	int nextIdx = SUGAR kStmt_parseTypePattern(kctx, stmt, Stmt_nameSpace(stmt), tokenArray, beginIdx + 1, endIdx, &foundClass);
	if(nextIdx != -1 && nextIdx < kArray_size(tokenArray)) {
		kToken *nextTokenAfterClassName = tokenArray->tokenItems[nextIdx];
//		if (ct->typeId == TY_void) {
//			RETURN_(SUGAR Stmt_p(kctx, stmt, tk1, ErrTag, "undefined class: %s", S_text(tk1->text)));
//		} else if (CT_isVirtual(ct)) {
//			SUGAR Stmt_p(kctx, stmt, NULL, ErrTag, "invalid application of 'new' to incomplete class %s", CT_t(ct));
//		}
		if(nextTokenAfterClassName->resolvedSyntaxInfo->keyword == KW_ParenthesisGroup) {  // new C (...)
			SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall);
			kExpr *expr = SUGAR new_ConsExpr(kctx, syn, 2, newToken, NewExpr(kctx, syn, tokenArray->tokenVarItems[beginIdx+1], foundClass->typeId));
			newToken->resolvedSymbol = MN_new;
			RETURN_(expr);
		}
		if(nextTokenAfterClassName->resolvedSyntaxInfo->keyword == KW_BracketGroup) {     // new int [100]
			SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), KW_new);
			KonohaClass *arrayClass = CT_p0(kctx, CT_Array, foundClass->typeId);
			newToken->resolvedSymbol = MN_("newArray");
			kExpr *expr = SUGAR new_ConsExpr(kctx, syn, 2, newToken, NewExpr(kctx, syn, tokenArray->tokenVarItems[beginIdx+1], arrayClass->typeId));
			RETURN_(expr);
		}
	}
}

// ----------------------------------------------------------------------------
/* define class */

static kbool_t new_initNameSpace(KonohaContext *kctx,  kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("new"), ParseExpr_(new), .precedence_op1 = 300},
		{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX);
	return true;
}

static kbool_t new_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* new_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("new", "1.0"),
		.initPackage    = new_initPackage,
		.setupPackage   = new_setupPackage,
		.initNameSpace  = new_initNameSpace,
		.setupNameSpace = new_setupNameSpace,
	};
	return &d;
}
