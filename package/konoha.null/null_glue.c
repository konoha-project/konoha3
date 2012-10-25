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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>

#ifdef __cplusplus
extern "C"{
#endif

// --------------------------------------------------------------------------
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


#define _F(F) (intptr_t)(F)

static kbool_t null_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		kMethod_Public, _F(Object_isNull), TY_boolean, TY_Object, MN_("isNull"), 0,
		kMethod_Public, _F(Object_isNotNull), TY_boolean, TY_Object, MN_("isNotNull"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

// --------------------------------------------------------------------------
/* Syntax */

static KMETHOD TypeCheck_null(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	if(reqty == TY_var) reqty = TY_Object;
	KReturn(SUGAR kExpr_setVariable(kctx, expr, gma, TEXPR_NULL, reqty, 0));
}

static KMETHOD Expression_isNull(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	if(operatorIdx + 2 == endIdx) {
		DBG_P("checking .. x == null");
		kTokenVar *tk = tokenList->TokenVarItems[operatorIdx+1];
		if(tk->resolvedSymbol == SYM_("null")) {
			kExpr *leftHandExpr = SUGAR kStmt_parseExpr(kctx, stmt, tokenList, beginIdx, operatorIdx, NULL);
			tk->resolvedSymbol = SYM_("isNull");
			KReturn(SUGAR new_UntypedCallStyleExpr(kctx, SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall), 2, tk, leftHandExpr));
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
		if(tk->resolvedSymbol == SYM_("null")) {
			kExpr *leftHandExpr = SUGAR kStmt_parseExpr(kctx, stmt, tokenList, beginIdx, operatorIdx, NULL);
			tk->resolvedSymbol = SYM_("isNotNull");
			KReturn(SUGAR new_UntypedCallStyleExpr(kctx, SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall), 2, tk, leftHandExpr));
		}
	}
	DBG_P("checking parent .. != ..");
}


static kbool_t null_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("null"), 0, NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_null, },
		{ KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX);
	SUGAR kNameSpace_addSugarFunc(kctx, ns, SYM_("=="), SugarFunc_Expression, new_SugarFunc(ns, Expression_isNull));
	SUGAR kNameSpace_addSugarFunc(kctx, ns, SYM_("!="), SugarFunc_Expression, new_SugarFunc(ns, Expression_isNotNull));
	return true;
}


// ----

static kbool_t null_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	null_defineMethod(kctx, ns, trace);
	null_defineSyntax(kctx, ns, trace);
	return true;
}

static kbool_t null_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* null_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "null", "1.0");
	d.initPackage    = null_initPackage;
	d.setupPackage   = null_setupPackage;
	return &d;
}

#ifdef __cplusplus
}
#endif
