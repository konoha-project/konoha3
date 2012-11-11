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
	KReturnUnboxValue(O_ct(sfp[0].asObject)->typeId);
}

// boolean Object.instanceOf(Object o)
static KMETHOD Object_instanceOf(KonohaContext *kctx, KonohaStack *sfp)
{
	KonohaClass *selfClass = O_ct(sfp[0].asObject), *targetClass = O_ct(sfp[1].asObject);
	KReturnUnboxValue(selfClass == targetClass || selfClass->isSubType(kctx, selfClass, targetClass));
}

// @SmartReturn Object Object.as(Object target)
static KMETHOD Object_as(KonohaContext *kctx, KonohaStack *sfp)
{
	KonohaClass *selfClass = O_ct(sfp[0].asObject), *targetClass = KGetReturnType(sfp);
	kObject *returnValue;
	if(selfClass == targetClass || selfClass->isSubType(kctx, selfClass, targetClass)) {
		returnValue = sfp[0].asObject;
	}
	else {
		returnValue = KLIB Knull(kctx, targetClass);
	}
	sfp[K_RTNIDX].unboxValue = O_unbox(returnValue);
	KReturn(returnValue);
}

// void NameSpace_AllowImplicitCoercion(boolean t)
static KMETHOD NameSpace_AllowImplicitCoercion(KonohaContext *kctx, KonohaStack *sfp)
{
	kNameSpaceVar *ns = (kNameSpaceVar *)sfp[0].asNameSpace;
	kNameSpace_Set(ImplicitCoercion, ns, sfp[1].boolValue);
}

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Ignored  kMethod_IgnoredOverride
#define _Im       kMethod_Immutable
#define _Final    kMethod_Final
#define _Virtual  kMethod_Virtual
#define _Hidden   kMethod_Hidden
#define _F(F)   (intptr_t)(F)

static void object_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Im|_Const|_Final, _F(Object_getTypeId), TY_int, TY_Object, MN_("getTypeId"), 0,
		_Public|_Hidden|_Im|_Const, _F(Object_instanceOf), TY_boolean, TY_Object, MN_("<:"), 1, TY_Object, FN_("type"),
		_Public|_Hidden|_Im|_Const|kMethod_SmartReturn, _F(Object_as), TY_Object, TY_Object, MN_("as"), 0,
		_Public|_Const|_Ignored, _F(NameSpace_AllowImplicitCoercion), TY_void, TY_NameSpace, MN_("AllowImplicitCoercion"), 1, TY_boolean, FN_("allow"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

/* Syntax */

static KMETHOD TypeCheck_InstanceOf(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kExpr *selfExpr   = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 1, gma, TY_var, 0);
	kExpr *targetExpr = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 2, gma, TY_var, 0);
	if(selfExpr != K_NULLEXPR && targetExpr != K_NULLEXPR) {
		KonohaClass *selfClass = CT_(selfExpr->ty), *targetClass = CT_(targetExpr->ty);
		if(CT_is(Final, selfClass)) {
			kbool_t staticSubType = (selfClass == targetClass || selfClass->isSubType(kctx, selfClass, targetClass));
			KReturn(SUGAR kExpr_setUnboxConstValue(kctx, expr, TY_boolean, staticSubType));
		}
		kNameSpace *ns = Stmt_ns(stmt);
		kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, TY_Object, MN_("<:"), 1);
		DBG_ASSERT(mtd != NULL);
		KFieldSet(expr->cons, expr->cons->MethodItems[0], mtd);
		kExpr *classValue = SUGAR kExpr_setConstValue(kctx,
				expr->cons->ExprItems[2], targetExpr->ty, KLIB Knull(kctx, targetClass));
		KFieldSet(expr->cons, expr->cons->ExprItems[2], classValue);
		KReturn(SUGAR kStmtExpr_TypeCheckCallParam(kctx, stmt, expr, mtd, gma, TY_boolean));
	}
}

static KMETHOD TypeCheck_as(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kExpr *targetExpr = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 2, gma, TY_var, 0);
	kExpr *selfExpr   = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 1, gma, targetExpr->ty, TypeCheckPolicy_NOCHECK);
	if(selfExpr != K_NULLEXPR && targetExpr != K_NULLEXPR) {
		KonohaClass *selfClass = CT_(selfExpr->ty), *targetClass = CT_(targetExpr->ty);
		if(selfExpr->ty == targetExpr->ty || selfClass->isSubType(kctx, selfClass, targetClass)) {
			KReturn(selfExpr);
		}
		if(selfClass->isSubType(kctx, targetClass, selfClass)) {
			kNameSpace *ns = Stmt_ns(stmt);
			kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, TY_Object, MN_("as"), 0);
			DBG_ASSERT(mtd != NULL);
			KReturn(SUGAR kStmtExpr_TypeCheckCallParam(kctx, stmt, expr, mtd, gma, targetClass->typeId));
		}
		KReturn(kStmtExpr_printMessage(kctx, stmt, selfExpr, ErrTag, "unable to downcast: %s as %s", TY_t(selfExpr->ty), TY_t(targetExpr->ty)));
	}
}

static KMETHOD TypeCheck_to(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kExpr *targetExpr = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 2, gma, TY_var, 0);
	kExpr *selfExpr   = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 1, gma, targetExpr->ty, TypeCheckPolicy_NOCHECK);
	if(selfExpr != K_NULLEXPR && targetExpr != K_NULLEXPR) {
		KonohaClass *selfClass = CT_(selfExpr->ty), *targetClass = CT_(targetExpr->ty);
		if(selfExpr->ty == targetExpr->ty || selfClass->isSubType(kctx, selfClass, targetClass)) {
			kStmtExpr_printMessage(kctx, stmt, selfExpr, InfoTag, "no need: %s to %s", TY_t(selfExpr->ty), TY_t(targetExpr->ty));
			KReturn(selfExpr);
		}
		kNameSpace *ns = Stmt_ns(stmt);
		kMethod *mtd = KLIB kNameSpace_GetCoercionMethodNULL(kctx, ns, selfExpr->ty, targetExpr->ty);
		if(mtd == NULL) {
			mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, selfExpr->ty, MN_("to"), 0);
			DBG_ASSERT(mtd != NULL);  // because Object.to is found.
			if(mtd->typeId != selfExpr->ty) {
				KReturn(kStmtExpr_printMessage(kctx, stmt, selfExpr, ErrTag, "undefined coercion: %s to %s", TY_t(selfExpr->ty), TY_t(targetExpr->ty)));
			}
		}
		KReturn(SUGAR kStmtExpr_TypeCheckCallParam(kctx, stmt, expr, mtd, gma, targetClass->typeId));
	}
}


static kbool_t subtype_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("<:"), 0, NULL, Precedence_CStyleMUL, 0, NULL, NULL, NULL, NULL, TypeCheck_InstanceOf, },
		{ SYM_("as"), 0, NULL, Precedence_CStyleMUL, 0, NULL, NULL, NULL, NULL, TypeCheck_as},
		{ SYM_("to"), 0, NULL, Precedence_CStyleMUL, 0, NULL, NULL, NULL, NULL, TypeCheck_to},
		{ KW_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}

// --------------------------------------------------------------------------

static kbool_t object_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
//	KRequirePackage("konoha.subtype", trace);
	KDEFINE_INT_CONST ClassData[] = {   // add Object as available
		{"Object", VirtualType_KonohaClass, (uintptr_t)CT_(TY_Object)},
		{NULL},
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KonohaConst_(ClassData), 0);
	object_defineMethod(kctx, ns, trace);
	subtype_defineSyntax(kctx, ns, trace);
	return true;
}

static kbool_t object_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* object_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", "1.0");
	d.PackupNameSpace    = object_PackupNameSpace;
	d.ExportNameSpace   = object_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
