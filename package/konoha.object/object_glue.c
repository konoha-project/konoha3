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

static KMETHOD ExprTyCheck_InstanceOf(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kExpr *selfExpr   = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 1, gma, TY_var, 0);
	kExpr *targetExpr = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 2, gma, TY_var, 0);
	if(selfExpr != K_NULLEXPR && targetExpr != K_NULLEXPR) {
		KonohaClass *selfClass = CT_(selfExpr->ty), *targetClass = CT_(targetExpr->ty);
		if(CT_isFinal(selfClass)) {
			kbool_t staticSubType = (selfClass == targetClass || selfClass->isSubType(kctx, selfClass, targetClass));
			RETURN_(SUGAR kExpr_setUnboxConstValue(kctx, expr, TY_Boolean, staticSubType));
		}
		kNameSpace *ns = Stmt_nameSpace(stmt);
		kMethod *mtd = KLIB kNameSpace_getMethodNULL(kctx, ns, TY_Object, MN_("<:"), 1, MPOL_PARAMSIZE);
		DBG_ASSERT(mtd != NULL);
		KSETv(expr->cons, expr->cons->methodItems[0], mtd);
		KSETv(expr->cons, expr->cons->exprItems[2], SUGAR kExpr_setConstValue(kctx, expr, targetExpr->ty, KLIB Knull(kctx, targetClass)));
		RETURN_(SUGAR kStmt_tyCheckCallParamExpr(kctx, stmt, expr, mtd, gma, TY_Boolean));
	}
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
		kMethod *mtd = KLIB kNameSpace_getMethodNULL(kctx, ns, TY_Object, MN_("as"), 0, 0);
		DBG_ASSERT(mtd != NULL);
		KSETv(expr->cons, expr->cons->methodItems[0], mtd);
		KSETv(expr->cons, expr->cons->exprItems[2], SUGAR kExpr_setConstValue(kctx, expr, targetExpr->ty, KLIB Knull(kctx, targetClass)));
		RETURN_(SUGAR kStmt_tyCheckCallParamExpr(kctx, stmt, expr, mtd, gma, targetClass->typeId));
	}
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Hidden   kMethod_Hidden
#define _Imm    kMethod_Immutable
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

static	kbool_t object_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KREQUIRE_PACKAGE("konoha.new", pline);
	int FN_type = FN_("type");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const, _F(Object_getTypeId), TY_Int, TY_Object, MN_("getTypeId"), 0,
		_Public|_Hidden|_Imm|_Const, _F(Object_instanceOf), TY_Boolean, TY_Object, MN_("<:"), 1, TY_Object, FN_type,
		_Public|_Hidden|_Imm|_Const|kMethod_SmartReturn, _F(Object_as), TY_Object, TY_Object, MN_("as"), 1, TY_Object, FN_type,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t object_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

// --------------------------------------------------------------------------

static KMETHOD ExprTyCheck_Getter(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kToken *tkN = expr->cons->tokenItems[0];
	ksymbol_t fn = tkN->resolvedSymbol;
	kExpr *self = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 1, gma, TY_var, 0);
	kNameSpace *ns = Stmt_nameSpace(stmt);
	if(self != K_NULLEXPR) {
		kMethod *mtd = KLIB kNameSpace_getMethodNULL(kctx, ns, self->ty, MN_toGETTER(fn), 0, MPOL_GETTER);
		if(mtd != NULL) {
			KSETv(expr->cons, expr->cons->methodItems[0], mtd);
			RETURN_(SUGAR kStmt_tyCheckCallParamExpr(kctx, stmt, expr, mtd, gma, reqty));
		}
		SUGAR kStmt_printMessage(kctx, stmt, tkN, ErrTag, "undefined field: %s", S_text(tkN->text));
	}
	RETURN_(K_NULLEXPR);
}

// ----------------------------------------------------------------------------

static kbool_t object_initNameSpace(KonohaContext *kctx,  kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("."), ExprTyCheck_(Getter) },
		{ .keyword = SYM_("<:"), ExprTyCheck_(InstanceOf), .precedence_op2 = C_PRECEDENCE_MUL/*FIXME*/ },
		{ .keyword = SYM_("as"), ExprTyCheck_(As), .precedence_op2 = C_PRECEDENCE_MUL/*FIXME*/ },
//		{ .keyword = SYM_("to"), .precedence_op2 = C_PRECEDENCE_MUL/*FIXME*/ },
		{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX);
	return true;
}

static kbool_t object_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* object_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("object", "1.0"),
		.initPackage = object_initPackage,
		.setupPackage = object_setupPackage,
		.initNameSpace = object_initNameSpace,
		.setupNameSpace = object_setupNameSpace,
	};
	return &d;
}
