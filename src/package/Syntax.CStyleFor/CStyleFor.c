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
/* CStyleFor */

static KMETHOD PatternMatch_ForBlock(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	int i;
	for(i = beginIdx; i < endIdx; i++) {
		kTokenVar *tk = tokenList->TokenVarItems[i];
		if(tk->resolvedSymbol == KSymbol_SEMICOLON) {
			kToken_Set(StatementSeparator, tk, false);
			break;
		}
	}
	if(beginIdx < i) {
		KTokenSeq tokens = {Stmt_ns(stmt), tokenList, beginIdx, i};
		kBlock *bk = SUGAR new_kBlock(kctx, stmt, NULL, &tokens);
		SUGAR kStmt_AddParsedObject(kctx, stmt, name, UPCAST(bk));
	}
	KReturnUnboxValue(i);
}

static void kStmt_copy(KonohaContext *kctx, kStmtVar *dStmt, ksymbol_t kw, kStmt *sStmt)
{
	kObject *o = kStmt_GetObject(kctx, sStmt, kw, NULL);
	if(o != NULL) {
		kStmt_SetObject(kctx, dStmt, kw, o);
	}
}

static KMETHOD Statement_CStyleFor(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	int ret = true;
	int KSymbol_InitBlock = KSymbol_("init"), KSymbol_IteratorBlock = KSymbol_("Iterator");
	kBlock *initBlock = SUGAR kStmt_GetBlock(kctx, stmt, NULL/*defaultNS*/, KSymbol_InitBlock, NULL);
	if(initBlock == NULL) {  // with out init
		DBG_P(">>>>>>>>> Without init block");
		if(SUGAR kStmt_TypeCheckByName(kctx, stmt, KSymbol_ExprPattern, gma, KClass_Boolean, 0)) {
			kBlock *bk = SUGAR kStmt_GetBlock(kctx, stmt, NULL/*DefaultNameSpace*/, KSymbol_BlockPattern, K_NULLBLOCK);
			kStmt_Set(CatchContinue, stmt, true);  // set before TypeCheckAll
			kStmt_Set(CatchBreak, stmt, true);
			SUGAR kBlock_TypeCheckAll(kctx, bk, gma);
			kBlock *iterBlock = SUGAR kStmt_GetBlock(kctx, stmt, NULL/*defaultNS*/, KSymbol_IteratorBlock, NULL);
			if(iterBlock != NULL) {
				SUGAR kBlock_TypeCheckAll(kctx, iterBlock, gma);
			}
			kStmt_Set(RedoLoop, stmt, true);
			kStmt_typed(stmt, LOOP);
		}
	}
	else {
		kStmtVar *forStmt = SUGAR new_kStmt(kctx, OnGcStack, stmt->syn, 0);
		DBG_ASSERT(IS_Block(initBlock));
		SUGAR kBlock_InsertAfter(kctx, initBlock, NULL, forStmt);
		forStmt->uline = stmt->uline;
		kStmt_copy(kctx, forStmt, KSymbol_ExprPattern, stmt);
		kStmt_copy(kctx, forStmt, KSymbol_BlockPattern, stmt);
		kStmt_copy(kctx, forStmt, KSymbol_IteratorBlock, stmt);
		SUGAR kBlock_TypeCheckAll(kctx, initBlock, gma);
		kStmt_SetObject(kctx, stmt, KSymbol_BlockPattern, initBlock);
		kStmt_typed(stmt, BLOCK);
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
static kbool_t CStyleFor_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("break"), 0, "\"break\"", 0, 0, NULL, NULL, NULL, Statement_break, NULL, },
		{ KSymbol_("continue"), 0, "\"continue\"", 0, 0, NULL, NULL, NULL, Statement_continue, NULL, },
		{ KSymbol_("$ForStmt"), 0, NULL, 0, 0, PatternMatch_ForBlock, NULL, NULL, NULL, NULL, },
		{ KSymbol_("for"), 0, "\"for\" \"(\" init: $ForStmt \";\" $Expr \";\" Iterator: $ForStmt \")\" $Block", 0, 0, NULL, NULL, NULL, Statement_CStyleFor, NULL, },
		{ KSymbol_END, }, /* sentinental */
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);

	return true;
}

static kbool_t CStyleFor_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* CStyleFor_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "CStyleFor", "0.0");
	d.PackupNameSpace = CStyleFor_PackupNameSpace;
	d.ExportNameSpace = CStyleFor_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
} /* extern C */
#endif
