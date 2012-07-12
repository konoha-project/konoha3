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

#ifndef VAR_GLUE_H_
#define VAR_GLUE_H_

// --------------------------------------------------------------------------

static	kbool_t var_initPackage(KonohaContext *kctx, kNameSpace *ks, int argc, const char**args, kline_t pline)
{
	return true;
}

static kbool_t var_setupPackage(KonohaContext *kctx, kNameSpace *ks, kline_t pline)
{
	return true;
}

static KMETHOD StmtTyCheck_var(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	USING_SUGAR;
	VAR_StmtTyCheck(stmt, syn, gma);
//	DBG_P("global assignment .. ");
//	kObject *scr = gma->genv->ks->scrNUL;
//	if(scr == NULL) {
//		SUGAR p(kctx, ERR_, stmt->uline, -1, " global variables are not available");
//		RETURNb_(false);
//	}
//	kExpr *vexpr = kStmt_expr(stmt, KW_("var"), K_NULLEXPR);
//	ksymbol_t fn = tosymbol(kctx, vexpr);
//	if(fn == SYM_NONAME) {
//		SUGAR p(kctx, ERR_, stmt->uline, -1, "not variable name");
//		RETURNb_(false);
//	}
//	kExpr *expr = kStmt_expr(stmt, KW_ExprPattern, K_NULLEXPR);
//	DBG_P("expr kw='%s'", KW_t(expr->syn->kw));
//	if(!SUGAR Stmt_tyCheckExpr(kctx, stmt, KW_ExprPattern, gma, TY_var, 0)) {
//		SUGAR p(kctx, ERR_, stmt->uline, -1, "type error");
//		RETURNb_(false);
//	}
//	/*kExpr **/expr = kStmt_expr(stmt, KW_ExprPattern, K_NULLEXPR);
//	kMethod *mtd = Object_newProtoSetterNULL(kctx, scr, gma->genv->ks, expr->ty, fn, stmt->uline);
//	if(mtd == NULL) {
//		RETURNb_(false);
//	}
//	SUGAR p(kctx, INFO_, stmt->uline, -1, "%s has type %s", SYM_t(fn), TY_t(expr->ty));
//	expr = SUGAR new_TypedMethodCall(kctx, TY_void, mtd, gma, 2, new_ConstValue(O_cid(scr), scr), expr);
//	kObject_setObject(stmt, KW_ExprPattern, expr);
//	kStmt_typed(stmt, EXPR);
//	RETURNb_(true);
}

// ---------------------------------------------------------------------------

static kbool_t var_initNameSpace(KonohaContext *kctx,  kNameSpace *ks, kline_t pline)
{
	USING_SUGAR;
	KDEFINE_SYNTAX SYNTAX[] = {
		{ TOKEN("var"), StmtTyCheck_(var), .rule = "\"var\" var: $expr \"=\" $expr", },
		{ .kw = KW_END, },
	};
	SUGAR NameSpace_defineSyntax(kctx, ks, SYNTAX);
	return true;
}


static kbool_t var_setupNameSpace(KonohaContext *kctx, kNameSpace *ks, kline_t pline)
{
	return true;
}


#endif /* VAR_GLUE_H_ */
