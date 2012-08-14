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

/* ************************************************************************ */

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>

#ifdef __cplusplus
extern "C" {
#endif
// --------------------------------------------------------------------------

static kbool_t while_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	return true;
}

static kbool_t while_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

// --------------------------------------------------------------------------
//
static KMETHOD StmtTyCheck_while(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_StmtTyCheck(stmt, gma);
	DBG_P("while statement .. ");
	int ret = false;
	if(SUGAR kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_Boolean, 0)) {
		kBlock *bk = SUGAR kStmt_getBlock(kctx, stmt, NULL/*DefaultNameSpace*/, KW_BlockPattern, K_NULLBLOCK);
		ret = SUGAR kBlock_tyCheckAll(kctx, bk, gma);
		kStmt_typed(stmt, LOOP);
	}
	RETURNb_(ret);
}

static KMETHOD StmtTyCheck_for(KonohaContext *kctx, KonohaStack *sfp)
{
//	VAR_StmtTyCheck(stmt, gma);
//	DBG_P("for statement .. ");
//	kToken *typeToken = SUGAR kStmt_getToken(kctx, stmt, KW_TypePattern, NULL);
//	kToken *varToken  = SUGAR kStmt_getToken(kctx, stmt, KW_SymbolPattern, NULL);
//	if(typeToken != NULL) {
//
//	}
//	if(!SUGAR kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_Boolean, 0)) {
//		RETURNb_(false);
//	}
//	kBlock *block = SUGAR new_kBlock(kctx, stmt, NULL, NULL);
//	if(typeToke != NULL) {
//		KLIB kArray_add(kctx, block->stmtList, kBlock_newStmt(kctx, block, KW_TypeDecl, KW_TypePattern, typeToken, varToken));
//	}
//	kStmt *whileStmt = new_Stmt();
//	whileStmt =
//	if(varBlock != NULL) {  // for(n: it)
//
//	}
//	if(SUGAR kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_Boolean, 0)) {
//		kBlock *bk = SUGAR kStmt_getBlock(kctx, stmt, NULL/*DefaultNameSpace*/, KW_BlockPattern, K_NULLBLOCK);
//		ret = SUGAR kBlock_tyCheckAll(kctx, bk, gma);
//		kStmt_typed(stmt, LOOP);
//	}
//	RETURNb_(ret);
}

static inline kStmt* kStmt_getParentNULL(kStmt *stmt)
{
	return stmt->parentBlockNULL->parentStmtNULL;
}

static KMETHOD StmtTyCheck_break(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_StmtTyCheck(stmt, gma);
	kStmt *p = stmt;
	while((p = kStmt_getParentNULL(p)) != NULL) {
		if(FLAG_is(p->syn->flag, SYNFLAG_StmtJumpSkip)) {
			KLIB kObject_setObject(kctx, stmt, stmt->syn->keyword, TY_Stmt, p);
			kStmt_typed(stmt, JUMP);
			RETURNb_(true);
		}
	}
	SUGAR Stmt_p(kctx, stmt, NULL, ErrTag, "break statement not within a loop");
	RETURNb_(false);
}

static KMETHOD StmtTyCheck_continue(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_StmtTyCheck(stmt, gma);
	DBG_P("continue statement .. ");
	kStmt *p = stmt;
	while((p = kStmt_getParentNULL(p)) != NULL) {
		if(FLAG_is(p->syn->flag, SYNFLAG_StmtJumpAhead)) {
			KLIB kObject_setObject(kctx, stmt, stmt->syn->keyword, TY_Stmt, p);
			kStmt_typed(stmt, JUMP);
			RETURNb_(true);
		}
	}
	SUGAR Stmt_p(kctx, stmt, NULL, ErrTag, "continue statement not within a loop");
	RETURNb_((false));
}

#define _LOOP .flag = (SYNFLAG_StmtJumpAhead|SYNFLAG_StmtJumpSkip)

static kbool_t while_initNameSpace(KonohaContext *kctx,  kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("while"), _LOOP, StmtTyCheck_(while), .rule = "\"while\" \"(\" $Expr \")\" $Block",},
		{ .keyword = SYM_("break"), StmtTyCheck_(break), .rule = "\"break\" [ $Symbol ]", },
		{ .keyword = SYM_("continue"), StmtTyCheck_(continue), .rule = "\"continue\" [ $Symbol ]", },
//		{ .keyword = SYM_("for"), _LOOP, StmtTyCheck_(for), .rule = "\"for\" \"(\" [ var: $Block \";\" $Expr \";\" each: $Block ] \")\" $Block", },
		{ .keyword = SYM_("for"), _LOOP, StmtTyCheck_(for),
			.rule = "\"for\" \"(\" [$Type] $Symbol \"in\" $Expr  \")\" [$Block] ", },
		{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX);
	return true;
}

static kbool_t while_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* while_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("C-compatible while", "1.0"),
		.initPackage      = while_initPackage,
		.setupPackage     = while_setupPackage,
		.initNameSpace  = while_initNameSpace,
		.setupNameSpace = while_setupNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
