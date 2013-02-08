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

static KMETHOD PatternMatch_Inc(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	int i;
	ksymbol_t KSymbol_Inc = KSymbol_("++"), KSymbol_Dec = KSymbol_("--");
	for(i = beginIdx; i < endIdx; i++) {
		kTokenVar *tk = tokenList->TokenVarItems[i];
		if(tk->symbol == KSymbol_Inc || tk->symbol == KSymbol_Dec) {
			int start, end;
			if(beginIdx == i) {
				start = beginIdx + 1;
				end = endIdx;
			}
			else {
				start = beginIdx;
				end   = i;
			}
			if(start < end) {
				kToken *opToken = tokenList->TokenItems[i];
				kSyntax *opSyntax = opToken->resolvedSyntaxInfo;
				KTokenSeq macro = {kNode_ns(stmt), tokenList};
				KTokenSeq_Push(kctx, macro);
				KMacroSet macroParam[] = {
					{KSymbol_("X"), tokenList, start, end},
					{0, NULL, 0, 0},   /* sentinel */
				};
				macro.TargetPolicy.RemovingIndent = true;
				SUGAR ApplyMacroData(kctx, macro.ns, opSyntax->macroDataNULL, 0, kArray_size(opSyntax->macroDataNULL), opSyntax->macroParamSize, macroParam, macro.tokenList);
				KTokenSeq_End(kctx, macro);
				SUGAR ParseNode(kctx, stmt, RangeTokenSeq(macro), ParseExpressionOption, NULL);
				KTokenSeq_Pop(kctx, macro);
				KReturnUnboxValue(endIdx);
			}
			break;
		}
	}
	KReturnUnboxValue(-1);
}

static KMETHOD Expression_Increment(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	kToken *tk = tokenList->TokenItems[operatorIdx];
	KReturn(SUGAR MessageNode(kctx, stmt, tk, NULL, ErrTag, "%s is defined as a statement", kString_text(tk->text)));
}

// --------------------------------------------------------------------------

static kbool_t cstyle_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("$Inc"), SYNFLAG_MetaPattern|SYNFLAG_CFunc, 0, Precedence_Statement, {SUGARFUNC PatternMatch_Inc}, },
//		{ KSymbol_("$IncStmt"), SYNFLAG_CFunc, 0, 0, {SUGARFUNC PatternMatch_IncStmt}, },
		{ KSymbol_("++"), SYNFLAG_NodeLeftJoinOp2|SYNFLAG_CFunc, Precedence_CStyleSuffixCall, Precedence_CStylePrefixOperator, {SUGARFUNC Expression_Increment},},
		{ KSymbol_("--"), SYNFLAG_NodeLeftJoinOp2|SYNFLAG_CFunc, Precedence_CStyleSuffixCall, Precedence_CStylePrefixOperator, {SUGARFUNC Expression_Increment},},

		{ KSymbol_END, }, /* sentinental */
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	SUGAR kSyntax_AddPattern(kctx, kSyntax_(ns, KSymbol_("$Inc")), "$IncStmt", 0, trace);
	SUGAR SetMacroData(kctx, ns, KSymbol_("++"), 1,  "X X = (X) + 1", false);
	SUGAR SetMacroData(kctx, ns, KSymbol_("--"), 1,  "X X = (X) - 1", false);

	return true;
}

static kbool_t cstyle_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE *GoStyleIncrement_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "GoStyle", "1.0");
	d.PackupNameSpace   = cstyle_PackupNameSpace;
	d.ExportNameSpace   = cstyle_ExportNameSpace;
	return &d;
}

// --------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif
