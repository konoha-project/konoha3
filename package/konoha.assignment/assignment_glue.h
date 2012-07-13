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

#ifndef ASSIGNMENT_GLUE_H_
#define ASSIGNMENT_GLUE_H_

// --------------------------------------------------------------------------

// Expr Expr.tyCheckStub(Gamma gma, int reqtyid);
static KMETHOD ExprTyCheck_assignment(KonohaContext *kctx, KonohaStack *sfp)
{
	USING_SUGAR;
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kExpr *lexpr = kExpr_tyCheckAt(stmt, expr, 1, gma, TY_var, TPOL_ALLOWVOID);
	kExpr *rexpr = kExpr_tyCheckAt(stmt, expr, 2, gma, lexpr->ty, 0);
	if(rexpr != K_NULLEXPR && lexpr != K_NULLEXPR) {
		if(rexpr != K_NULLEXPR) {
			if(lexpr->build == TEXPR_LOCAL || lexpr->build == TEXPR_LOCAL_ || lexpr->build == TEXPR_FIELD) {
				((kExprVar*)expr)->build = TEXPR_LET;
				((kExprVar*)rexpr)->ty = lexpr->ty;
				RETURN_(expr);
			}
			if(lexpr->build == TEXPR_CALL) {  // check getter and transform to setter
				kMethod *mtd = lexpr->cons->methodItems[0];
				DBG_ASSERT(IS_Method(mtd));
				if((MN_isGETTER(mtd->mn) || MN_isISBOOL(mtd->mn)) && !kMethod_isStatic(mtd)) {
					ktype_t cid = lexpr->cons->exprItems[1]->ty;
					mtd = KLIB kNameSpace_getMethodNULL(kctx, gma->genv->ns, cid, MN_toSETTER(mtd->mn));
					if(mtd != NULL) {
						KSETv(lexpr->cons->methodItems[0], mtd);
						KLIB kArray_add(kctx, lexpr->cons, rexpr);
						RETURN_(SUGAR Expr_tyCheckCallParams(kctx, stmt, lexpr, mtd, gma, reqty));
					}
				}
			}
			SUGAR Stmt_p(kctx, stmt, (kToken*)expr, ERR_, "variable name is expected");
		}
	}
	RETURN_(K_NULLEXPR);
}

// --------------------------------------------------------------------------

static	kbool_t assignment_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	return true;
}

static kbool_t assignment_setupPackage(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static KMETHOD StmtTyCheck_DefaultAssignment(KonohaContext *kctx, KonohaStack *sfp)
{
}

#define setToken(tk, str, size, k) {\
		KSETv(tk->text, KLIB new_kString(kctx, str, size, 0));\
		tk->keyword = k;\
	}

static int transform_oprAssignment(KonohaContext *kctx, kArray* tls, int s, int c, int e)
{
	kTokenVar *tkNew, *tkNewOp;
	kToken *tmp, *tkHead;
	int newc, news = e;
	int i = s;

	while (i < c) {
		tkNew = new_Var(Token, 0);
		tmp = tls->tokenItems[i];
		setToken(tkNew, S_text(tmp->text), S_size(tmp->text), tmp->keyword);
		KLIB kArray_add(kctx, tls, tkNew);
		i++;
	}

	// check operator
	tkNewOp = new_Var(Token, 0);
	tmp = tls->tokenItems[c];
	const char* opr = S_text(tmp->text);
	int osize = S_size(tmp->text);
	int j = 0;
	char newopr[osize];
	for (j = 0; j < osize-1; j++) {
		newopr[j] = opr[j];
	}
	newopr[osize-1] = '\0';
	setToken(tkNewOp, newopr, osize, SYM_(newopr));

	tkNew = new_Var(Token, 0);
	setToken(tkNew, "=", 1, KW_LET);
	KLIB kArray_add(kctx, tls, tkNew);
	newc = kArray_size(tls)-1;

	kTokenVar *newtk = new_Var(Token, 0);
	tkHead = tls->tokenItems[e+1];
	newtk->keyword = AST_PARENTHESIS;
	newtk->uline = tkHead->uline;
	//newtk->topch = tkHead->topch; newtk->lpos = tkHead->closech;
	KSETv(newtk->sub, new_(TokenArray, 0));
	i = news;

	while (i < newc) {
		tkNew = new_Var(Token, 0);
		tmp = tls->tokenItems[i];
		setToken(tkNew, S_text(tmp->text), S_size(tmp->text), tmp->keyword);
		KLIB kArray_add(kctx, newtk->sub, tkNew);
		i++;
	}
	KLIB kArray_add(kctx, tls, newtk);

	KLIB kArray_add(kctx, tls, tkNewOp);

	tkNew = new_Var(Token, 0);
	i = c+1;
	while (i < news) {
		tkNew = new_Var(Token, 0);
		tmp = tls->tokenItems[i];
		setToken(tkNew, S_text(tmp->text), S_size(tmp->text), tmp->keyword);
		KLIB kArray_add(kctx, tls, tkNew);
		i++;
	}
	return news;
}

static KMETHOD ParseExpr_OprAssignment(KonohaContext *kctx, KonohaStack *sfp)
{
	USING_SUGAR;
	VAR_ParseExpr(stmt, tls, s, c, e);
	size_t atop = kArray_size(tls);
	s = transform_oprAssignment(kctx, tls, s, c, e);
	kExpr *expr = SUGAR Stmt_newExpr2(kctx, stmt, tls, s, kArray_size(tls));
	KLIB kArray_clear(kctx, tls, atop);
	RETURN_(expr);
}

static kbool_t assignment_initNameSpace(KonohaContext *kctx,  kNameSpace *ns, kfileline_t pline)
{
	USING_SUGAR;
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("="), /*.op2 = "*", .priority_op2 = 4096,*/ ExprTyCheck_(assignment)},
		{ .keyword = SYM_("+="), _OPLeft, /*.priority_op2 =*/ StmtTyCheck_(DefaultAssignment), ParseExpr_(OprAssignment), .priority_op2 = 4096,},
		{ .keyword = SYM_("-="), _OPLeft, /*.priority_op2 =*/ StmtTyCheck_(DefaultAssignment), ParseExpr_(OprAssignment), .priority_op2 = 4096,},
		{ .keyword = SYM_("*="), _OPLeft, /*.priority_op2 =*/ StmtTyCheck_(DefaultAssignment), ParseExpr_(OprAssignment), .priority_op2 = 4096,},
		{ .keyword = SYM_("/="), _OPLeft, /*.priority_op2 =*/ StmtTyCheck_(DefaultAssignment), ParseExpr_(OprAssignment), .priority_op2 = 4096,},
		{ .keyword = SYM_("%="), _OPLeft, /*.priority_op2 =*/ StmtTyCheck_(DefaultAssignment), ParseExpr_(OprAssignment), .priority_op2 = 4096,},
		{ .keyword = KW_END, },
	};
	SUGAR NameSpace_defineSyntax(kctx, ns, SYNTAX);
	return true;
}

static kbool_t assignment_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}


#endif /* ASSIGNMENT_GLUE_H_ */
