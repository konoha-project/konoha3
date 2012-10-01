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
	RETURNb_(IS_NULL(o));
}

//## Boolean Object.isNotNull();
static KMETHOD Object_isNotNull(KonohaContext *kctx, KonohaStack *sfp)
{
	kObject *o = sfp[0].asObject;
	RETURNb_(!IS_NULL(o));
}

// --------------------------------------------------------------------------

#define _F(F) (intptr_t)(F)
static kbool_t null_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	intptr_t MethodData[] = {
		kMethod_Public, _F(Object_isNull), TY_boolean, TY_Object, MN_("isNull"), 0,
		kMethod_Public, _F(Object_isNotNull), TY_boolean, TY_Object, MN_("isNotNull"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);

	return true;
}

static kbool_t null_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static KMETHOD ExprTyCheck_null(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	if(reqty == TY_var) reqty = TY_Object;
	RETURN_(SUGAR kExpr_setVariable(kctx, expr, gma, TEXPR_NULL, reqty, 0));
}

static KMETHOD ParseExpr_isNull(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	if(operatorIdx + 2 == endIdx) {
		DBG_P("checking .. x == null");
		kTokenVar *tk = tokenList->tokenVarItems[operatorIdx+1];
		if(tk->resolvedSymbol == SYM_("null")) {
			kExpr *leftHandExpr = SUGAR kStmt_parseExpr(kctx, stmt, tokenList, beginIdx, operatorIdx);
			tk->resolvedSymbol = SYM_("isNull");
			RETURN_(SUGAR new_UntypedCallStyleExpr(kctx, SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall), 2, tk, leftHandExpr));
		}
	}
	DBG_P("checking parent .. == ..");
}

static KMETHOD ParseExpr_isNotNull(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	if(operatorIdx + 2 == endIdx) {
		DBG_P("checking .. x != null");
		kTokenVar *tk = tokenList->tokenVarItems[operatorIdx+1];
		if(tk->resolvedSymbol == SYM_("null")) {
			kExpr *leftHandExpr = SUGAR kStmt_parseExpr(kctx, stmt, tokenList, beginIdx, operatorIdx);
			tk->resolvedSymbol = SYM_("isNotNull");
			RETURN_(SUGAR new_UntypedCallStyleExpr(kctx, SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall), 2, tk, leftHandExpr));
		}
	}
	DBG_P("checking parent .. != ..");
}


static kbool_t null_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("null"), 0, NULL, 0, 0, NULL, NULL, NULL, NULL, ExprTyCheck_null, },
		{ KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNameSpace);
	SUGAR kNameSpace_addSugarFunc(kctx, ns, SYM_("=="), SUGARFUNC_ParseExpr, new_SugarFunc(ParseExpr_isNull));
	SUGAR kNameSpace_addSugarFunc(kctx, ns, SYM_("!="), SUGARFUNC_ParseExpr, new_SugarFunc(ParseExpr_isNotNull));
	return true;
}

static kbool_t null_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* null_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSETPACKNAME(d, "null", "1.0");
	d.initPackage    = null_initPackage;
	d.setupPackage   = null_setupPackage;
	d.initNameSpace  = null_initNameSpace;
	d.setupNameSpace = null_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
