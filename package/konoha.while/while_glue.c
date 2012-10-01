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

static KMETHOD StmtTyCheck_while(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_StmtTyCheck(stmt, gma);
	DBG_P("while statement .. ");
	int ret = false;
	if(SUGAR kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_boolean, 0)) {
		kBlock *bk = SUGAR kStmt_getBlock(kctx, stmt, NULL/*DefaultNameSpace*/, KW_BlockPattern, K_NULLBLOCK);
		Stmt_setCatchContinue(stmt, true);  // set before tyCheckAll
		Stmt_setCatchBreak(stmt, true);
		ret = SUGAR kBlock_tyCheckAll(kctx, bk, gma);
		if(ret) {
			kStmt_typed(stmt, LOOP);
		}
	}
	RETURNb_(ret);
}

static inline kStmt* kStmt_getParentNULL(kStmt *stmt)
{
	return stmt->parentBlockNULL->parentStmtNULL;
}

static kbool_t while_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	KImportPackage(ns, "konoha.break", pline);
	KImportPackage(ns, "konoha.continue", pline);
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("while"), 0, "\"while\" \"(\" $Expr \")\" $Block", 0, 0, NULL, NULL, NULL, StmtTyCheck_while, NULL, },
		{ KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNameSpace);
	return true;
}

static kbool_t while_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* while_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSETPACKNAME(d, "while", "1.0");
	d.initPackage    = while_initPackage;
	d.setupPackage   = while_setupPackage;
	d.initNameSpace  = while_initNameSpace;
	d.setupNameSpace = while_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
