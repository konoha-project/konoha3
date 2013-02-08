/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
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

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/klib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Statement */

static KMETHOD PatternMatch_ForStmt(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	/* for(IniitExpr; CondExpr; IterateExpr) $Block */
	kNameSpace *ns = kNode_ns(stmt);
	int i, start = beginIdx;
	/* InitExpression Part */
	for(i = beginIdx; i < endIdx; i++) {
		kTokenVar *tk = tokenList->TokenVarItems[i];
		if(tk->symbol == KSymbol_SEMICOLON) {
			if(start < i) {
				kNode *node = SUGAR ParseNewNode(kctx, ns, tokenList, &start, i, ParseMetaPatternOption, NULL);
				SUGAR kNode_AddParsedObject(kctx, stmt, KSymbol_("init"), UPCAST(node));
			}
			start = i + 1;
			break;
		}
	}
	if(start == beginIdx) {
		KReturnUnboxValue(-1);
	}
	/* CondExpression Part */
	for(i = start; i < endIdx; i++) {
		kTokenVar *tk = tokenList->TokenVarItems[i];
		if(tk->symbol == KSymbol_SEMICOLON) {
			if(start < i) {
				kNode *node = SUGAR ParseNewNode(kctx, ns, tokenList, &start, i, ParseExpressionOption, NULL);
				KDump(node);
				SUGAR kNode_AddParsedObject(kctx, stmt, KSymbol_ExprPattern, UPCAST(node));
			}
			start = i + 1;
			break;
		}
	}
	if(i == endIdx) {
		KReturnUnboxValue(-1);
	}
	/* IterExpression Part */
	if(start < endIdx) {
		kNode *node = SUGAR ParseNewNode(kctx, ns, tokenList, &start, endIdx, ParseMetaPatternOption, NULL);
		SUGAR kNode_AddParsedObject(kctx, stmt, KSymbol_("Iterator"), UPCAST(node));
	}
	KReturnUnboxValue(endIdx);
}

static KMETHOD Statement_CStyleFor(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, ns, reqc);
	int KSymbol_InitNode = KSymbol_("init"), KSymbol_IteratorNode = KSymbol_("Iterator");
	KDump(stmt);
	kNode *initNode = SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_InitNode, ns, KClass_void, TypeCheckPolicy_AllowEmpty);
	if(initNode != NULL) {
		kNode_Set(OpenBlock, initNode, true);
	}
	SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_IteratorNode, ns, KClass_void, TypeCheckPolicy_AllowEmpty);
	SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_ExprPattern, ns, KClass_Boolean, 0);
	kNode_Set(CatchContinue, stmt, true);  // set before TypeCheckAll
	kNode_Set(CatchBreak, stmt, true);
	//kNode_Set(RedoLoop, stmt, true);
	SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_BlockPattern, ns, KClass_void, 0);
	KReturn(kNode_Type(kctx, stmt, KNode_For, KType_void));
}

/* copied from Syntax.CStyleWhile */
static KMETHOD Statement_break(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, ns, reqc);
	kNode *p = stmt;
	while(p != NULL) {
		if(kNode_Is(CatchBreak, p)) {
			KLIB kObjectProto_SetObject(kctx, stmt, KSymbol_("break"), KType_Node, p);
			KReturn(kNode_Type(kctx, stmt, KNode_Break, KType_void));
		}
		p = kNode_GetParentNULL(p);
	}
	KReturn(SUGAR MessageNode(kctx, stmt, NULL, ns, ErrTag, "break statement not within a loop"));
}

static KMETHOD Statement_continue(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, ns, reqc);
	kNode *p = stmt;
	while(p != NULL) {
		if(kNode_Is(CatchContinue, p)) {
			KLIB kObjectProto_SetObject(kctx, stmt, KSymbol_("continue"), KType_Node, p);
			KReturn(kNode_Type(kctx, stmt, KNode_Continue, KType_void));
		}
		p = kNode_GetParentNULL(p);
	}
	KReturn(SUGAR MessageNode(kctx, stmt, NULL, ns, ErrTag, "continue statement not within a loop"));
}


// --------------------------------------------------------------------------

static kbool_t cstyle_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("$ForStmt"), SYNFLAG_CParseFunc, 0, 0, {SUGARFUNC PatternMatch_ForStmt}, },
		{ KSymbol_("for"), SYNFLAG_CTypeFunc, 0, Precedence_Statement, {SUGAR patternParseFunc}, {SUGARFUNC Statement_CStyleFor} },

		/* copied from CStyleWhile */
		{ KSymbol_("break"), SYNFLAG_CTypeFunc, 0, Precedence_Statement, {SUGAR patternParseFunc}, {SUGARFUNC Statement_break}},
		{ KSymbol_("continue"), SYNFLAG_CTypeFunc, 0, Precedence_Statement, {SUGAR patternParseFunc}, {SUGARFUNC Statement_continue}},
		{ KSymbol_END, }, /* sentinental */
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	SUGAR kSyntax_AddPattern(kctx, kSyntax_(ns, KSymbol_("for")), "\"for\" \"(\" $ForStmt \")\" $Block", 0, trace);
	SUGAR kSyntax_AddPattern(kctx, kSyntax_(ns, KSymbol_("break")), "\"break\"", 0, trace);
	SUGAR kSyntax_AddPattern(kctx, kSyntax_(ns, KSymbol_("continue")), "\"continue\"", 0, trace);
	return true;
}

static kbool_t cstyle_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE *CStyleFor_Init(void)
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
