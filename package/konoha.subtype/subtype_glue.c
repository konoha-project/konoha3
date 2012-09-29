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

// Object.getTypeId()
static KMETHOD Object_getTypeId(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(O_ct(sfp[0].asObject)->typeId);
}

// Object.instanceOf(Object o)
static KMETHOD Object_instanceOf(KonohaContext *kctx, KonohaStack *sfp)
{
	KonohaClass *selfClass = O_ct(sfp[0].asObject), *targetClass = O_ct(sfp[1].asObject);
	RETURNb_(selfClass == targetClass || selfClass->isSubType(kctx, selfClass, targetClass));
}

static KMETHOD Object_as(KonohaContext *kctx, KonohaStack *sfp)
{
	KonohaClass *selfClass = O_ct(sfp[0].asObject), *targetClass = O_ct(sfp[1].asObject);
	kObject *returnValue;
	if(selfClass == targetClass || selfClass->isSubType(kctx, selfClass, targetClass)) {
		returnValue = sfp[0].asObject;
	}
	else {
		returnValue = KLIB Knull(kctx, targetClass);
	}
	sfp[K_RTNIDX].unboxValue = O_unbox(returnValue);
	RETURN_(returnValue);
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Hidden   kMethod_Hidden
#define _Imm    kMethod_Immutable
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

static kbool_t subtype_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	int FN_type = FN_("type");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const, _F(Object_getTypeId), TY_int, TY_Object, MN_("getTypeId"), 0,
		_Public|_Hidden|_Imm|_Const, _F(Object_instanceOf), TY_boolean, TY_Object, MN_("<:"), 1, TY_Object, FN_type,
		_Public|_Hidden|_Imm|_Const|kMethod_SmartReturn, _F(Object_as), TY_Object, TY_Object, MN_("as"), 1, TY_Object, FN_type,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t subtype_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static KMETHOD ExprTyCheck_InstanceOf(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kExpr *selfExpr   = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 1, gma, TY_var, 0);
	kExpr *targetExpr = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 2, gma, TY_var, 0);
	if(selfExpr != K_NULLEXPR && targetExpr != K_NULLEXPR) {
		KonohaClass *selfClass = CT_(selfExpr->ty), *targetClass = CT_(targetExpr->ty);
		if(CT_is(Final, selfClass)) {
			kbool_t staticSubType = (selfClass == targetClass || selfClass->isSubType(kctx, selfClass, targetClass));
			RETURN_(SUGAR kExpr_setUnboxConstValue(kctx, expr, TY_boolean, staticSubType));
		}
		kNameSpace *ns = Stmt_nameSpace(stmt);
		kMethod *mtd = KLIB kNameSpace_getMethodByParamSizeNULL(kctx, ns, TY_Object, MN_("<:"), 1);
		DBG_ASSERT(mtd != NULL);
		KSETv(expr->cons, expr->cons->methodItems[0], mtd);
		kExpr *classValue = SUGAR kExpr_setConstValue(kctx,
				expr->cons->exprItems[2], targetExpr->ty, KLIB Knull(kctx, targetClass));
		KSETv(expr->cons, expr->cons->exprItems[2], classValue);
		RETURN_(SUGAR kStmt_tyCheckCallParamExpr(kctx, stmt, expr, mtd, gma, TY_boolean));
	}
}

static KMETHOD ExprTyCheck_As(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kExpr *targetExpr = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 2, gma, TY_var, 0);
	kExpr *selfExpr   = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 1, gma, targetExpr->ty, 0);
	if(selfExpr != K_NULLEXPR && targetExpr != K_NULLEXPR) {
		KonohaClass *selfClass = CT_(selfExpr->ty), *targetClass = CT_(targetExpr->ty);
		if(selfExpr->ty == targetExpr->ty || selfClass->isSubType(kctx, selfClass, targetClass)) {
			RETURN_(selfExpr);
		}
		kNameSpace *ns = Stmt_nameSpace(stmt);
		kMethod *mtd = KLIB kNameSpace_getMethodByParamSizeNULL(kctx, ns, TY_Object, MN_("as"), 0);
		DBG_ASSERT(mtd != NULL);
		KSETv(expr->cons, expr->cons->methodItems[0], mtd);
		kExpr *classValue = SUGAR kExpr_setConstValue(kctx,
				expr->cons->exprItems[2], targetExpr->ty, KLIB Knull(kctx, targetClass));
		KSETv(expr->cons, expr->cons->exprItems[2], classValue);
		RETURN_(SUGAR kStmt_tyCheckCallParamExpr(kctx, stmt, expr, mtd, gma, targetClass->typeId));
	}
}

// ----------------------------------------------------------------------------

static kbool_t subtype_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("<:"), 0, NULL, C_PRECEDENCE_MUL, 0, NULL, NULL, NULL, NULL, ExprTyCheck_InstanceOf, },
		{ SYM_("as"), 0, NULL, C_PRECEDENCE_MUL, 0, NULL, NULL, NULL, NULL, ExprTyCheck_As},
		{ KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNameSpace);
	return true;
}

static kbool_t subtype_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* subtype_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSETPACKNAME(d, "subtype", "1.0");
	d.initPackage    = subtype_initPackage;
	d.setupPackage   = subtype_setupPackage;
	d.initNameSpace  = subtype_initNameSpace;
	d.setupNameSpace = subtype_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
