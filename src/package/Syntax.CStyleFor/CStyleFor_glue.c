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
#include <minikonoha/klib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Statement */

static KMETHOD PatternMatch_ForStmt(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	int i;
	for(i = beginIdx; i < endIdx; i++) {
		kTokenVar *tk = tokenList->TokenVarItems[i];
		if(tk->symbol == KSymbol_SEMICOLON) {
//			kToken_Set(StatementSeparator, tk, false);
			break;
		}
	}
	if(beginIdx < i) {
		kNode *node = SUGAR ParseNewNode(kctx, kNode_ns(stmt), tokenList, &beginIdx, i, ParseMetaPatternOption, NULL);
		SUGAR kNode_AddParsedObject(kctx, stmt, name, UPCAST(node));
	}
	KReturnUnboxValue(i);
}

static KMETHOD Statement_CStyleFor(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, ns, reqc);
	int KSymbol_InitNode = KSymbol_("init"), KSymbol_IteratorNode = KSymbol_("Iterator");
	kNode *initNode = SUGAR kNode_GetNode(kctx, stmt, KSymbol_InitNode, NULL);
	if(initNode != NULL) {
		initNode = SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_InitNode, ns, KClass_void, 0);
		if(kNode_IsError(initNode)) {
			KReturn(initNode);
		}
	}
	kNode *iterNode = SUGAR kNode_GetNode(kctx, stmt, KSymbol_IteratorNode, NULL);
	if(iterNode != NULL) {
		iterNode = SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_IteratorNode, ns, KClass_Boolean, 0);
		if(kNode_IsError(iterNode)) {
			KReturn(iterNode);
		}
	}
	kNode *exprNode = SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_ExprPattern, ns, KClass_Boolean, 0);
	if(kNode_IsError(exprNode)) {
		KReturn(exprNode);
	}
	SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_BlockPattern, ns, KClass_void, 0);
	kNode_Set(CatchContinue, stmt, true);  // set before TypeCheckAll
	kNode_Set(CatchBreak, stmt, true);
	//kNode_Set(RedoLoop, stmt, true);
	KReturn(kNode_Type(kctx, stmt, KNode_While, KType_void));
}

// --------------------------------------------------------------------------

static kbool_t cstyle_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("$ForNode"), SYNFLAG_CParseFunc, 0, 0, {SUGARFUNC PatternMatch_ForStmt}, },
		{ KSymbol_("for"), SYNFLAG_CTypeFunc, 0, Precedence_Statement, {SUGAR patternParseFunc}, {SUGARFUNC Statement_CStyleFor} },
		{ KSymbol_END, }, /* sentinental */
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	SUGAR kNameSpace_AddSyntaxPattern(kctx, kSyntax_(ns, KSymbol_("for")), "\"for\" \"(\" init: $ForStmt \";\" $Expr \";\" Iterator: $ForStmt \")\" $Block", 0, trace);
	return true;
}

static kbool_t cstyle_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* CStyleFor_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "CStyle", K_VERSION);
	d.PackupNameSpace   = cstyle_PackupNameSpace;
	d.ExportNameSpace   = cstyle_ExportNameSpace;
	return &d;
}

// --------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif
