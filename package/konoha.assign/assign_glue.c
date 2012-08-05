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

#ifndef ASSIGN_GLUE_H_
#define ASSIGN_GLUE_H_

// --------------------------------------------------------------------------

// Expr Expr.tyCheckStub(Gamma gma, int reqtyid);
static KMETHOD ExprTyCheck_assign(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kNameSpace *ns = Stmt_nameSpace(stmt);  // leftHandExpr = rightHandExpr
	kExpr *leftHandExpr = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 1, gma, TY_var, TPOL_ALLOWVOID);
	kExpr *rightHandExpr = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 2, gma, leftHandExpr->ty, 0);
	if(rightHandExpr != K_NULLEXPR && leftHandExpr != K_NULLEXPR) {
		if(leftHandExpr->build == TEXPR_LOCAL || leftHandExpr->build == TEXPR_FIELD || leftHandExpr->build == TEXPR_STACKTOP) {
			((kExprVar*)expr)->build = TEXPR_LET;
			((kExprVar*)expr)->ty    = leftHandExpr->ty;
			((kExprVar*)rightHandExpr)->ty = leftHandExpr->ty;
			RETURN_(expr);
		}
		if(leftHandExpr->build == TEXPR_CALL) {  // check getter and transform to setter
			kMethod *mtd = leftHandExpr->cons->methodItems[0];
			DBG_ASSERT(IS_Method(mtd));
			if(MN_isGETTER(mtd->mn)) {
				ktype_t cid = leftHandExpr->cons->exprItems[1]->ty;
				ktype_t paramType = leftHandExpr->ty; //CT_(cid)->realtype(kctx, CT_(cid), CT_(leftHandExpr->ty));
				mtd = KLIB kNameSpace_getMethodNULL(kctx, ns, cid, MN_toSETTER(mtd->mn), paramType, MPOL_SETTER|MPOL_CANONICAL);
				DBG_P("cid=%s, mtd=%p", TY_t(cid), mtd);
				if(mtd != NULL) {
					KSETv(leftHandExpr->cons, leftHandExpr->cons->methodItems[0], mtd);
					KLIB kArray_add(kctx, leftHandExpr->cons, rightHandExpr);
					RETURN_(SUGAR kStmt_tyCheckCallParamExpr(kctx, stmt, leftHandExpr, mtd, gma, reqty));
				}
				SUGAR Stmt_p(kctx, stmt, (kToken*)expr, ErrTag, "setter is undefined");
			}
			SUGAR Stmt_p(kctx, stmt, (kToken*)expr, ErrTag, "getter is expected");
		}
		SUGAR Stmt_p(kctx, stmt, (kToken*)expr, ErrTag, "variable name is expected");
	}
	RETURN_(K_NULLEXPR);
}

// --------------------------------------------------------------------------

static	kbool_t assign_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	return true;
}

static kbool_t assign_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static KMETHOD StmtTyCheck_DefaultAssign(KonohaContext *kctx, KonohaStack *sfp)
{
}

#define setToken(tk, str, size, k) {\
		KSETv(tk, tk->text, KLIB new_kString(kctx, str, size, 0));\
		tk->keyword = k;\
	}

static inline void kToken_copy(KonohaContext *kctx, kTokenVar *destToken, kToken *origToken)
{
	destToken->resolvedSymbol = origToken->resolvedSymbol;
	KSETv(destToken, destToken->text, origToken->text);
	destToken->uline = origToken->uline;
	destToken->resolvedSyntaxInfo = origToken->resolvedSyntaxInfo;
}

static KMETHOD ParseExpr_SelfAssign(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	kNameSpace *ns = Stmt_nameSpace(stmt);
	kToken *selfAssignToken = tokenList->tokenItems[operatorIdx];

	size_t i, beginTemplateIdx = kArray_size(tokenList);
	SUGAR kNameSpace_tokenize(kctx, ns, "/*A*/ = ( /*A*/ ) /*+*/ ( /*B*/ )", selfAssignToken->uline, tokenList);

	size_t beginNewIdx = kArray_size(tokenList);
	for(i = beginIdx; i < operatorIdx; i++) {
		KLIB kArray_add(kctx, tokenList, tokenList->tokenItems[i]);
	}
	KLIB kArray_add(kctx, tokenList, tokenList->tokenItems[beginTemplateIdx+1]); // template: =
	KLIB kArray_add(kctx, tokenList, tokenList->tokenItems[beginTemplateIdx+2]); // template; (
	for(i = beginIdx; i < operatorIdx; i++) {
		kTokenVar *tk = GCSAFE_new(TokenVar, 0);
		kToken_copy(kctx, tk, tokenList->tokenItems[i]);
		KLIB kArray_add(kctx, tokenList, tk);
	}
	KLIB kArray_add(kctx, tokenList, tokenList->tokenItems[beginTemplateIdx+3]); // template: )
	kTokenVar *opToken = GCSAFE_new(TokenVar, TokenType_SYMBOL);
	KSETv(opToken, opToken->text, KLIB new_kString(kctx, S_text(selfAssignToken->text), S_size(selfAssignToken->text) - 1, SPOL_ASCII));
	KLIB kArray_add(kctx, tokenList, opToken);
	KLIB kArray_add(kctx, tokenList, tokenList->tokenItems[beginTemplateIdx+4]); // template: (
	for(i = operatorIdx+1; i < endIdx; i++) {
		KLIB kArray_add(kctx, tokenList, tokenList->tokenItems[i]);
	}
	KLIB kArray_add(kctx, tokenList, tokenList->tokenItems[beginTemplateIdx+5]); // template: )

	size_t beginResolovedIdx = kArray_size(tokenList);
	if(SUGAR kNameSpace_resolveTokenArray(kctx, ns, tokenList, beginNewIdx, beginResolovedIdx, tokenList)) {
		kExpr *expr = SUGAR kStmt_parseExpr(kctx, stmt, tokenList, beginResolovedIdx, kArray_size(tokenList));
		KLIB kArray_clear(kctx, tokenList, beginTemplateIdx);
		RETURN_(expr);
	}
	KLIB kArray_clear(kctx, tokenList, beginTemplateIdx);
}

static kbool_t assign_initNameSpace(KonohaContext *kctx,  kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("="), ExprTyCheck_(assign)},
		{ .keyword = SYM_("+="), _OPLeft, /*StmtTyCheck_(DefaultAssign),*/ ParseExpr_(SelfAssign), .precedence_op2 = 1600,},
		{ .keyword = SYM_("-="), _OPLeft, /*StmtTyCheck_(DefaultAssign),*/ ParseExpr_(SelfAssign), .precedence_op2 = 1600,},
		{ .keyword = SYM_("*="), _OPLeft, /*StmtTyCheck_(DefaultAssign),*/ ParseExpr_(SelfAssign), .precedence_op2 = 1600,},
		{ .keyword = SYM_("/="), _OPLeft, /*StmtTyCheck_(DefaultAssign),*/ ParseExpr_(SelfAssign), .precedence_op2 = 1600,},
		{ .keyword = SYM_("%="), _OPLeft, /*StmtTyCheck_(DefaultAssign),*/ ParseExpr_(SelfAssign), .precedence_op2 = 1600,},
		{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX);
	return true;
}

static kbool_t assign_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* assign_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("assign", "1.0"),
		.initPackage = assign_initPackage,
		.setupPackage = assign_setupPackage,
		.initNameSpace = assign_initNameSpace,
		.setupNameSpace = assign_setupNameSpace,
	};
	return &d;
}

#endif /* ASSIGN_GLUE_H_ */

