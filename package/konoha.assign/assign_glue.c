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

//int verbose_sugar = 1;
//#include"../src/sugar/sugardump.h"

// --------------------------------------------------------------------------

// Expr Expr.tyCheckStub(Gamma gma, int reqtyid);
//static KMETHOD ExprTyCheck_assign(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_ExprTyCheck(stmt, expr, gma, reqty);
//	kNameSpace *ns = Stmt_nameSpace(stmt);  // leftHandExpr = rightHandExpr
//	kExpr *leftHandExpr = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 1, gma, TY_var, TPOL_ALLOWVOID);
//	kExpr *rightHandExpr = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 2, gma, leftHandExpr->ty, 0);
//	if(rightHandExpr != K_NULLEXPR && leftHandExpr != K_NULLEXPR) {
//		if(leftHandExpr->build == TEXPR_LOCAL || leftHandExpr->build == TEXPR_FIELD || leftHandExpr->build == TEXPR_STACKTOP) {
//			((kExprVar*)expr)->build = TEXPR_LET;
//			((kExprVar*)expr)->ty    = leftHandExpr->ty;
//			((kExprVar*)rightHandExpr)->ty = leftHandExpr->ty;
//			RETURN_(expr);
//		}
//		if(leftHandExpr->build == TEXPR_CALL) {  // check getter and transform to setter
//			kMethod *mtd = leftHandExpr->cons->methodItems[0];
//			DBG_ASSERT(IS_Method(mtd));
//			if(MN_isGETTER(mtd->mn)) {
//				ktype_t cid = leftHandExpr->cons->exprItems[1]->ty;
//				ktype_t paramType = leftHandExpr->ty; //CT_(cid)->realtype(kctx, CT_(cid), CT_(leftHandExpr->ty));
//				mtd = KLIB kNameSpace_getMethodNULL(kctx, ns, cid, MN_toSETTER(mtd->mn), paramType, MPOL_SETTER|MPOL_CANONICAL);
//				DBG_P("cid=%s, mtd=%p", TY_t(cid), mtd);
//				if(mtd != NULL) {
//					KSETv(leftHandExpr->cons, leftHandExpr->cons->methodItems[0], mtd);
//					KLIB kArray_add(kctx, leftHandExpr->cons, rightHandExpr);
//					RETURN_(SUGAR kStmt_tyCheckCallParamExpr(kctx, stmt, leftHandExpr, mtd, gma, reqty));
//				}
//			}
//		}
//		SUGAR Stmt_p(kctx, stmt, (kToken*)expr, ErrTag, "variable name is expected");
//	}
//	RETURN_(K_NULLEXPR);
//}

// --------------------------------------------------------------------------

static	kbool_t assign_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	return true;
}

static kbool_t assign_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

//static KMETHOD StmtTyCheck_DefaultAssign(KonohaContext *kctx, KonohaStack *sfp)
//{
//
//}

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
	DBG_ASSERT(S_size(selfAssignToken->text) > 1);
	ksymbol_t opSymbol = KLIB Ksymbol(kctx, S_text(selfAssignToken->text), S_size(selfAssignToken->text) - 1, SPOL_ASCII, _NEWID);
	SugarSyntax *opSyntax = SYN_(ns, opSymbol);
	if(opSyntax != NULL) {
		TokenRange macroRangeBuf, *macroRange = SUGAR new_TokenListRange(kctx, Stmt_nameSpace(stmt), tokenList, &macroRangeBuf);
		SUGAR TokenRange_tokenize(kctx, macroRange, "A = ( A ) + ( B )", 0);

		TokenRange opRangeBuf, *opRange = SUGAR new_TokenStackRange(kctx, macroRange, &opRangeBuf);
		kTokenVar *opToken = GCSAFE_new(TokenVar, TokenType_SYMBOL);
		KSETv(opToken, opToken->text, SYM_s(opSymbol));
		KLIB kArray_add(kctx, opRange->tokenList, opToken);
		TokenRange_end(kctx, opRange);

		TokenRange newexprRangeBuf, *newexprRange = SUGAR new_TokenStackRange(kctx, opRange, &newexprRangeBuf);
		TokenRange A = {tokenList, beginIdx, operatorIdx, ns};
		TokenRange B = {tokenList, operatorIdx+1, endIdx, ns};
		MacroSet macros[] = {{SYM_("A"), &A}, {SYM_("B"), &B}, {SYM_("+"), opRange}, {0, NULL}/* necessary*/};
		macroRange->macroSet = macros;
		SUGAR TokenRange_resolved(kctx, newexprRange, macroRange);
//		KdumpTokenRange(kctx, "replaced", newexprRange);

		kExpr *expr = SUGAR kStmt_parseExpr(kctx, stmt, newexprRange->tokenList, newexprRange->beginIdx, newexprRange->endIdx);
		TokenRange_pop(kctx, macroRange);
		RETURN_(expr);
	}
	else {
		SUGAR Stmt_p(kctx, stmt, selfAssignToken, ErrTag, "undefined binary operator: %s", SYM_t(opSymbol));
	}
}

static kbool_t assign_initNameSpace(KonohaContext *kctx,  kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
//		{ .keyword = SYM_("="), ExprTyCheck_(assign)},
		{ .keyword = SYM_("+="), _OPLeft, /*StmtTyCheck_(DefaultAssign),*/ ParseExpr_(SelfAssign), .precedence_op2 = C_PRECEDENCE_ASSIGN,},
		{ .keyword = SYM_("-="), _OPLeft, /*StmtTyCheck_(DefaultAssign),*/ ParseExpr_(SelfAssign), .precedence_op2 = C_PRECEDENCE_ASSIGN,},
		{ .keyword = SYM_("*="), _OPLeft, /*StmtTyCheck_(DefaultAssign),*/ ParseExpr_(SelfAssign), .precedence_op2 = C_PRECEDENCE_ASSIGN,},
		{ .keyword = SYM_("/="), _OPLeft, /*StmtTyCheck_(DefaultAssign),*/ ParseExpr_(SelfAssign), .precedence_op2 = C_PRECEDENCE_ASSIGN,},
		{ .keyword = SYM_("%="), _OPLeft, /*StmtTyCheck_(DefaultAssign),*/ ParseExpr_(SelfAssign), .precedence_op2 = C_PRECEDENCE_ASSIGN,},
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


