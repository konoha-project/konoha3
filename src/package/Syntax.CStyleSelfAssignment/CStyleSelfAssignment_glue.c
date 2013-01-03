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

/* ------------------------------------------------------------------------ */
/* assignment */

static KMETHOD Expression_BinarySugar(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, opIdx, endIdx);
	kToken  *opToken = tokenList->TokenItems[opIdx];
	kSyntax *opSyntax = opToken->resolvedSyntaxInfo;
	DBG_ASSERT(opSyntax->macroParamSize == 2);
	KTokenSeq macro = {kNode_ns(stmt), tokenList};
	KTokenSeq_Push(kctx, macro);
	KMacroSet macroParam[] = {
			{KSymbol_("X"), tokenList, beginIdx, opIdx},
			{KSymbol_("Y"), tokenList, opIdx+1, endIdx},
			{0, NULL, 0, 0},
	};
	macro.TargetPolicy.RemovingIndent = true;
	SUGAR KTokenSeq_ApplyMacro(kctx, &macro, opSyntax->macroDataNULL, 0, kArray_size(opSyntax->macroDataNULL), opSyntax->macroParamSize, macroParam);
	SUGAR ParseNode(kctx, stmt, macro.tokenList, macro.beginIdx, macro.endIdx, ParseExpressionOption, NULL);
	KTokenSeq_Pop(kctx, macro);
	KReturnUnboxValue(endIdx);
}

static kbool_t cstyle_defineAssign(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	kFunc *binaryParseFunc = KSugarFunc(ns, Expression_BinarySugar);
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("+="), (SYNFLAG_NodeLeftJoinOp2), Precedence_CStyleAssign, 0, {binaryParseFunc}, {NULL}},
		{ KSymbol_("-="), (SYNFLAG_NodeLeftJoinOp2), Precedence_CStyleAssign, 0, {binaryParseFunc}, {NULL}},
		{ KSymbol_("*="), (SYNFLAG_NodeLeftJoinOp2), Precedence_CStyleAssign, 0, {binaryParseFunc}, {NULL}},
		{ KSymbol_("/="), (SYNFLAG_NodeLeftJoinOp2), Precedence_CStyleAssign, 0, {binaryParseFunc}, {NULL}},
		{ KSymbol_("%="), (SYNFLAG_NodeLeftJoinOp2), Precedence_CStyleAssign, 0, {binaryParseFunc}, {NULL}},
		{ KSymbol_("|="), (SYNFLAG_NodeLeftJoinOp2), Precedence_CStyleAssign, 0, {binaryParseFunc}, {NULL}},
		{ KSymbol_("&="), (SYNFLAG_NodeLeftJoinOp2), Precedence_CStyleAssign, 0, {binaryParseFunc}, {NULL}},
		{ KSymbol_("<<="), (SYNFLAG_NodeLeftJoinOp2), Precedence_CStyleAssign, 0, {binaryParseFunc}, {NULL}},
		{ KSymbol_(">>="), (SYNFLAG_NodeLeftJoinOp2), Precedence_CStyleAssign, 0, {binaryParseFunc}, {NULL}},
		{ KSymbol_("^="), (SYNFLAG_NodeLeftJoinOp2), Precedence_CStyleAssign, 0, {binaryParseFunc}, {NULL}},
		{ KSymbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("+="), 2,  "X Y X = (X) + (Y)", false);
	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("-="), 2,  "X Y X = (X) - (Y)", false);
	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("*="), 2,  "X Y X = (X) * (Y)", false);
	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("/="), 2,  "X Y X = (X) / (Y)", false);
	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("%="), 2,  "X Y X = (X) % (Y)", false);
	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("|="), 2,  "X Y X = (X) | (Y)", false);
	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("&="), 2,  "X Y X = (X) & (Y)", false);
	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("<<="), 2, "X Y X = (X) << (Y)", false);
	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_(">>="), 2, "X Y X = (X) >> (Y)", false);
	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("^="), 2,  "X Y X = (X) ^ (Y)", false);
	return true;
}

// --------------------------------------------------------------------------

static kbool_t CStyleSelfAssignment_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	cstyle_defineAssign(kctx, ns, trace);
	return true;
}

static kbool_t CStyleSelfAssignment_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* CStyleSelfAssignment_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "CStyle", "1.0");
	d.PackupNameSpace   = CStyleSelfAssignment_PackupNameSpace;
	d.ExportNameSpace   = CStyleSelfAssignment_ExportNameSpace;
	return &d;
}

// --------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif