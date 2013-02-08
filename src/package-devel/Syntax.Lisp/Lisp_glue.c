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

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/klib.h>
#include <konoha3/import/methoddecl.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */
/* Lisp */

static KMETHOD Expression_LispOperator(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(expr, tokenList, beginIdx, currentIdx, endIdx);
	kNameSpace *ns = kNode_ns(expr);
	if(beginIdx == currentIdx && beginIdx + 1 < endIdx) {
		kTokenVar *opToken = tokenList->TokenVarItems[beginIdx];
		kNode_Type(kctx, expr, KNode_Block, KType_var);
		int i = beginIdx + 1;
		SUGAR kNode_Op(kctx, expr, opToken, 0);
		while(i < endIdx) {
			int orig = i;
			kNode *node = SUGAR ParseNewNode(kctx, ns, tokenList, &i, i+1, ParseExpressionOption, "(");
			SUGAR kNode_AddNode(kctx, expr, node);
			assert(i != orig);
		}
		int size = kNode_GetNodeListSize(kctx, expr);
		if(size == 1) { /* case (+) */
			assert(0 && "(+) is not supported");
		}
		else if(size == 2) { /* case (+ 1) */
			KReturnUnboxValue(endIdx);
		}
		/* (+ 1 2 3 4) => (+ (+ (+ 1 2) 3 ) 4) */
		kNode *leftNode = kNode_At(expr, 1), *rightNode;
		for(i = 2; i < size-1; i++) {
			kNode *node = KNewNode(ns);
			rightNode = kNode_At(expr, i);
			SUGAR kNode_Op(kctx, node, opToken, 2, leftNode, rightNode);
			leftNode = node;
		}
		rightNode = kNode_At(expr, i);
		KLIB kArray_Clear(kctx, expr->NodeList, 1);
		KLIB kArray_Add(kctx, expr->NodeList, leftNode);
		KLIB kArray_Add(kctx, expr->NodeList, rightNode);
		KDump(expr);
		KReturnUnboxValue(endIdx);
	}
}

/* ------------------------------------------------------------------------ */
static kbool_t Lisp_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KImportPackage(ns, "Syntax.CStyleBitwiseOperator", trace);
	KDEFINE_SYNTAX SYNTAX[] = {
#define TOKEN(T) KSymbol_##T
		{ TOKEN(ADD), SYNFLAG_CParseFunc,0, Precedence_CStylePrefixOperator, {SUGARFUNC Expression_LispOperator}, {SUGAR methodTypeFunc},},
		{ TOKEN(SUB), SYNFLAG_CParseFunc,0, Precedence_CStylePrefixOperator, {SUGARFUNC Expression_LispOperator}, {SUGAR methodTypeFunc},},
		{ TOKEN(MUL), SYNFLAG_CParseFunc,0, Precedence_CStylePrefixOperator, {SUGARFUNC Expression_LispOperator}, {SUGAR methodTypeFunc},},
		{ TOKEN(DIV), SYNFLAG_CParseFunc,0, Precedence_CStylePrefixOperator, {SUGARFUNC Expression_LispOperator}, {SUGAR methodTypeFunc},},
		{ TOKEN(MOD), SYNFLAG_CParseFunc,0, Precedence_CStylePrefixOperator, {SUGARFUNC Expression_LispOperator}, {SUGAR methodTypeFunc},},
		{ KSymbol_END, }, /* sentinental */
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);

	return true;
}

static kbool_t Lisp_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *Lisp_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "Lisp", "0.0");
	d.PackupNameSpace = Lisp_PackupNameSpace;
	d.ExportNameSpace = Lisp_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
