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
 * AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
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
#include <minikonoha/klib.h>
#include <minikonoha/import/methoddecl.h>

#ifdef __cplusplus
extern C {
#endif

/* ------------------------------------------------------------------------ */
/* CStyleWhile */
static KMETHOD Statement_while(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	DBG_P("while statement .. ");
	int ret = false;
	if(SUGAR kStmt_TypeCheckByName(kctx, stmt, KSymbol_ExprPattern, gma, KClass_Boolean, 0)) {
		kBlock *bk = SUGAR kStmt_GetBlock(kctx, stmt, NULL/*DefaultNameSpace*/, KSymbol_BlockPattern, K_NULLBLOCK);
		kStmt_Set(CatchContinue, stmt, true);  // set before TypeCheckAll
		kStmt_Set(CatchBreak, stmt, true);
		ret = SUGAR kBlock_TypeCheckAll(kctx, bk, gma);
		if(ret) {
			kStmt_typed(stmt, LOOP);
		}
	}
	KReturnUnboxValue(ret);
}

static KMETHOD Statement_do(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	DBG_P("do statement .. ");
	int ret = false;
	if(SUGAR kStmt_TypeCheckByName(kctx, stmt, KSymbol_ExprPattern, gma, KClass_Boolean, 0)) {
		kBlock *bk = SUGAR kStmt_GetBlock(kctx, stmt, NULL/*DefaultNameSpace*/, KSymbol_BlockPattern, K_NULLBLOCK);
		kStmt_Set(CatchContinue, stmt, true);  // set before TypeCheckAll
		kStmt_Set(CatchBreak, stmt, true);
		ret = SUGAR kBlock_TypeCheckAll(kctx, bk, gma);
		if(ret) {
			kStmt_Set(RedoLoop, stmt, true);
			kStmt_typed(stmt, LOOP);  // FIXME
		}
	}
	KReturnUnboxValue(ret);
}

static inline kStmt* kStmt_GetParentNULL(kStmt *stmt)
{
	return stmt->parentBlockNULL->parentStmtNULL;
}

static KMETHOD Statement_break(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kStmt *p = stmt;
	while((p = kStmt_GetParentNULL(p)) != NULL) {
		if(kStmt_Is(CatchBreak, p)) {
			KLIB kObjectProto_SetObject(kctx, stmt, stmt->syn->keyword, KType_Stmt, p);
			kStmt_typed(stmt, JUMP);
			KReturnUnboxValue(true);
		}
	}
	SUGAR kStmt_Message2(kctx, stmt, NULL, ErrTag, "break statement not within a loop");
}

static KMETHOD Statement_continue(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kStmt *p = stmt;
	while((p = kStmt_GetParentNULL(p)) != NULL) {
		if(kStmt_Is(CatchContinue, p)) {
			KLIB kObjectProto_SetObject(kctx, stmt, stmt->syn->keyword, KType_Stmt, p);
			kStmt_typed(stmt, JUMP);
			KReturnUnboxValue(true);
		}
	}
	SUGAR kStmt_Message2(kctx, stmt, NULL, ErrTag, "continue statement not within a loop");
}


// --------------------------------------------------------------------------
static kbool_t CStyleWhile_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("break"), 0, "\"break\"", 0, 0, NULL, NULL, NULL, Statement_break, NULL, },
		{ KSymbol_("continue"), 0, "\"continue\"", 0, 0, NULL, NULL, NULL, Statement_continue, NULL, },
		{ KSymbol_("while"), 0, "\"while\" \"(\" $Expr \")\" $Block", 0, 0, NULL, NULL, NULL, Statement_while, NULL, },
		{ KSymbol_("do"), 0, "\"do\"  $Block \"while\" \"(\" $Expr \")\"", 0, 0, NULL, NULL, NULL, Statement_do, NULL, },
		{ KSymbol_END, }, /* sentinental */
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);

	return true;
}

static kbool_t CStyleWhile_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* CStyleWhile_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "CStyleWhile", "0.0");
	d.PackupNameSpace = CStyleWhile_PackupNameSpace;
	d.ExportNameSpace = CStyleWhile_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
} /* extern C */
#endif
