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
extern "C" {
#endif

/* ------------------------------------------------------------------------ */
/* Null */

//## Boolean Object.isNull();
static KMETHOD Object_isNull(KonohaContext *kctx, KonohaStack *sfp)
{
	kObject *o = sfp[0].asObject;
	KReturnUnboxValue(IS_NULL(o));
}

//## Boolean Object.isNotNull();
static KMETHOD Object_isNotNull(KonohaContext *kctx, KonohaStack *sfp)
{
	kObject *o = sfp[0].asObject;
	KReturnUnboxValue(!IS_NULL(o));
}

static kbool_t null_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Im|_Final|_Const, _F(Object_isNull),   KType_boolean, KType_Object, KMethodName_("isNull"), 0,
		_Public|_Im|_Final|_Const, _F(Object_isNotNull), KType_boolean, KType_Object, KMethodName_("isNotNull"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

/* null */

static KMETHOD TypeCheck_null(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	if(reqty == KType_var) reqty = KType_Object;
	KReturn(SUGAR kExpr_SetVariable(kctx, expr, gma, TEXPR_NULL, reqty, 0));
}

static KMETHOD Expression_isNull(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	if(operatorIdx + 2 == endIdx) {
		DBG_P("checking .. x == null");
		kTokenVar *tk = tokenList->TokenVarItems[operatorIdx+1];
		if(tk->resolvedSymbol == KSymbol_("null")) {
			kExpr *leftHandExpr = SUGAR kStmt_ParseExpr(kctx, stmt, tokenList, beginIdx, operatorIdx, NULL);
			tk->resolvedSymbol = KSymbol_("isNull");
			KReturn(SUGAR new_UntypedCallStyleExpr(kctx, KSyntax_(Stmt_ns(stmt), KSymbol_ExprMethodCall), 2, tk, leftHandExpr));
		}
	}
	DBG_P("checking parent .. == ..");
}

static KMETHOD Expression_isNotNull(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	if(operatorIdx + 2 == endIdx) {
		DBG_P("checking .. x != null");
		kTokenVar *tk = tokenList->TokenVarItems[operatorIdx+1];
		if(tk->resolvedSymbol == KSymbol_("null")) {
			kExpr *leftHandExpr = SUGAR kStmt_ParseExpr(kctx, stmt, tokenList, beginIdx, operatorIdx, NULL);
			tk->resolvedSymbol = KSymbol_("isNotNull");
			KReturn(SUGAR new_UntypedCallStyleExpr(kctx, KSyntax_(Stmt_ns(stmt), KSymbol_ExprMethodCall), 2, tk, leftHandExpr));
		}
	}
	DBG_P("checking parent .. != ..");
}

static kbool_t null_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("null"), 0, NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_null, },
		{ KSymbol_("NULL"), 0, NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_null, },
		{ KSymbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	SUGAR kNameSpace_AddSugarFunc(kctx, ns, KSymbol_("=="), SugarFunc_Expression, new_SugarFunc(ns, Expression_isNull));
	SUGAR kNameSpace_AddSugarFunc(kctx, ns, KSymbol_("!="), SugarFunc_Expression, new_SugarFunc(ns, Expression_isNotNull));
	return true;
}

// --------------------------------------------------------------------------
static kbool_t Null_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	null_defineMethod(kctx, ns, trace);
	null_defineSyntax(kctx, ns, trace);
	return true;
}

static kbool_t Null_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* Null_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "Null", "0.0");
	d.PackupNameSpace = Null_PackupNameSpace;
	d.ExportNameSpace = Null_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
