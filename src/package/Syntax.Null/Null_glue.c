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

// --------------------------------------------------------------------------
/* null */

static KMETHOD TypeCheck_null(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck2(stmt, expr, ns, reqc);
	if(reqc->typeId == KType_var) reqc = KClass_Object;
	KReturn(SUGAR kNode_SetVariable(kctx, expr, KNode_Null, reqc->typeId, 0));
}

static KMETHOD Expression_isNull(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, opIdx, endIdx);
	if(opIdx + 2 == endIdx) {
		DBG_P("checking .. x == null");
		kTokenVar *tk = tokenList->TokenVarItems[opIdx+1];
		if(tk->symbol == KSymbol_("null") || tk->symbol == KSymbol_("NULL")) {
			kNameSpace *ns = kNode_ns(stmt);
			tk->symbol = KSymbol_("IsNull");
			tk->resolvedSyntaxInfo = tokenList->TokenVarItems[opIdx]->resolvedSyntaxInfo;
			SUGAR kNode_Op(kctx, stmt, tk, 1, SUGAR ParseNewNode(kctx, ns, tokenList, &beginIdx, opIdx, ParseExpressionOption, NULL));
			KReturnUnboxValue(opIdx + 2);
		}
	}
	DBG_P("checking parent .. == ..");
	KReturnUnboxValue(-1);
}

static KMETHOD Expression_isNotNull(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, opIdx, endIdx);
	if(opIdx + 2 == endIdx) {
		DBG_P("checking .. x != null");
		kTokenVar *tk = tokenList->TokenVarItems[opIdx+1];
		if(tk->symbol == KSymbol_("null") || tk->symbol == KSymbol_("NULL")) {
			kNameSpace *ns = kNode_ns(stmt);
			tk->symbol = KSymbol_("IsNotNull");
			tk->resolvedSyntaxInfo = tokenList->TokenVarItems[opIdx]->resolvedSyntaxInfo;
			SUGAR kNode_Op(kctx, stmt, tk, 1, SUGAR ParseNewNode(kctx, ns, tokenList, &beginIdx, opIdx, ParseExpressionOption, NULL));
			KReturnUnboxValue(opIdx + 2);
		}
	}
	DBG_P("checking parent .. != ..");
	KReturnUnboxValue(-1);
}

// --------------------------------------------------------------------------

static kbool_t cstyle_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	kFunc *nullTypeFunc = KSugarFunc(ns, TypeCheck_null);
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("null"), 0, 0, 0, {SUGAR termParseFunc}, {nullTypeFunc}, },
		{ KSymbol_("NULL"), 0, 0, 0, {SUGAR termParseFunc}, {nullTypeFunc}, },
		{ KSymbol_("=="),  0, Precedence_CStyleEquals, 0, {KSugarFunc(ns, Expression_isNull)}, {SUGAR methodTypeFunc}},
		{ KSymbol_("!="), 0, Precedence_CStyleEquals, 0, {KSugarFunc(ns, Expression_isNotNull)}, {SUGAR methodTypeFunc}},
		{ KSymbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}

static kbool_t cstyle_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE *Null_Init(void)
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
