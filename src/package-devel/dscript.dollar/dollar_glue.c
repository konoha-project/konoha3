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

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>

#ifdef __cplusplus
extern "C" {
#endif
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------

static KMETHOD Expression_dollar(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, currentIdx, endIdx);
	DBG_ASSERT(beginIdx == currentIdx);
	if(currentIdx + 1 < endIdx) {
		kToken *nextToken = tokenList->TokenItems[currentIdx+1];
		DBG_P("nextToken='%s'", kString_text(nextToken->text));
//		if(nextToken->resolvedSyntaxInfo->keyword == KSymbol_SymbolPattern) {
//			KReturn(Expression_DollarSymbol(kctx, stmt, nextToken));
//		}

	}
//	KClass *foundClass = NULL;
//	int nextIdx = SUGAR ParseTypePattern(kctx, stmt, kNode_ns(stmt), tokenList, beginIdx + 1, endIdx, &foundClass);
//	if(nextIdx != -1 && nextIdx < kArray_size(tokenList)) {
//		kToken *nextTokenAfterClassName = tokenList->TokenItems[nextIdx];
////		if(ct->typeId == KType_void) {
////			KReturn(SUGAR MessageNode(kctx, stmt, tk1, ErrTag, "undefined class: %s", kString_text(tk1->text)));
////		} else if(KClass_Is(Virtual, ct)) {
////			SUGAR MessageNode(kctx, stmt, NULL, ErrTag, "invalid application of 'dollar' to incomplete class %s", KClass_text(ct));
////		}
//		if(nextTokenAfterClassName->resolvedSyntaxInfo->keyword == KSymbol_ParenthesisGroup) {  // dollar C (...)
//			kSyntax *syn = kSyntax_(kNode_ns(stmt), KSymbol_ParamPattern/*MethodCall*/);
//			kNode *expr = SUGAR dollar_UntypedCallStyleNode(kctx, syn, 2, dollarToken, NewNode(kctx, syn, tokenList->TokenVarItems[beginIdx+1], foundClass->typeId));
//			dollarToken->symbol = MN_dollar;
//			KReturn(expr);
//		}
//		if(nextTokenAfterClassName->resolvedSyntaxInfo->keyword == KSymbol_BracketGroup) {     // dollar int [100]
//			kSyntax *syn = kSyntax_(kNode_ns(stmt), KSymbol_("dollar"));
//			KClass *arrayClass = KClass_p0(kctx, KClass_Array, foundClass->typeId);
//			dollarToken->symbol = KMethodName_("dollarArray");
//			kNode *expr = SUGAR dollar_UntypedCallStyleNode(kctx, syn, 2, dollarToken, NewNode(kctx, syn, tokenList->TokenVarItems[beginIdx+1], arrayClass->typeId));
//			KReturn(expr);
//		}
//	}
}

// ----------------------------------------------------------------------------
/* define class */

static kbool_t dollar_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("$"), 0, NULL, 0, Precedence_CStyleSuffixCall, NULL, Expression_dollar, NULL, NULL, NULL, },
		{ KSymbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}

static kbool_t dollar_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	dollar_defineSyntax(kctx, ns, trace);
	return true;
}

static kbool_t dollar_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *dollar_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "dscript", "1.0");
	d.PackupNameSpace    = dollar_PackupNameSpace;
	d.ExportNameSpace   = dollar_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
