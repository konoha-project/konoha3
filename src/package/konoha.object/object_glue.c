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
	KReturnUnboxValue(kObject_class(sfp[0].asObject)->typeId);
}

// boolean Object.instanceOf(Object o)
static KMETHOD Object_instanceOf(KonohaContext *kctx, KonohaStack *sfp)
{
	KClass *selfClass = kObject_class(sfp[0].asObject), *targetClass = kObject_class(sfp[1].asObject);
	KReturnUnboxValue(selfClass == targetClass || selfClass->isSubType(kctx, selfClass, targetClass));
}

// @SmartReturn Object Object.as(Object target)
static KMETHOD Object_as(KonohaContext *kctx, KonohaStack *sfp)
{
	KClass *selfClass = kObject_class(sfp[0].asObject), *targetClass = KGetReturnType(sfp);
	kObject *returnValue;
	if(selfClass == targetClass || selfClass->isSubType(kctx, selfClass, targetClass)) {
		returnValue = sfp[0].asObject;
	}
	else {
		returnValue = KLIB Knull(kctx, targetClass);
	}
	sfp[K_RTNIDX].unboxValue = kObject_Unbox(returnValue);
	KReturn(returnValue);
}

#include <minikonoha/import/methoddecl.h>

static void object_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Im|_Const|_Final, _F(Object_getTypeId), KType_int, KType_Object, KMethodName_("getTypeId"), 0,
		_Public|_Hidden|_Im|_Const|_Final, _F(Object_instanceOf), KType_boolean, KType_Object, KMethodName_("<:"), 1, KType_Object, KFieldName_("type"),
		_Public|_Hidden|_Im|_Const|kMethod_SmartReturn|_Final, _F(Object_as), KType_Object, KType_Object, KMethodName_("as"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

/* Syntax */

static KMETHOD TypeCheck_InstanceOf(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kExpr *selfExpr   = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 1, gma, KClass_INFER, 0);
	kExpr *targetExpr = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 2, gma, KClass_INFER, 0);
	if(selfExpr != K_NULLEXPR && targetExpr != K_NULLEXPR) {
		KClass *selfClass = KClass_(selfExpr->attrTypeId), *targetClass = KClass_(targetExpr->attrTypeId);
		if(KClass_Is(Final, selfClass)) {
			kbool_t staticSubType = (selfClass == targetClass || selfClass->isSubType(kctx, selfClass, targetClass));
			KReturn(SUGAR kExpr_SetUnboxConstValue(kctx, expr, KType_boolean, staticSubType));
		}
		kNameSpace *ns = Stmt_ns(stmt);
		kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, KClass_Object, KMethodName_("<:"), 1, KMethodMatch_NoOption);
		DBG_ASSERT(mtd != NULL);
		KFieldSet(expr->NodeList, expr->NodeList->MethodItems[0], mtd);
		kExpr *classValue = SUGAR kExpr_SetConstValue(kctx, expr->NodeList->ExprVarItems[2], NULL, KLIB Knull(kctx, targetClass));
		KFieldSet(expr->NodeList, expr->NodeList->ExprItems[2], classValue);
		KReturn(SUGAR kStmtkExpr_TypeCheckCallParam(kctx, stmt, expr, mtd, gma, KClass_Boolean));
	}
}

static KMETHOD TypeCheck_as(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kExpr *targetExpr = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 2, gma, KClass_INFER, 0);
	kExpr *selfExpr   = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 1, gma, KClass_(targetExpr->attrTypeId), TypeCheckPolicy_NOCHECK);
	if(selfExpr != K_NULLEXPR && targetExpr != K_NULLEXPR) {
		KClass *selfClass = KClass_(selfExpr->attrTypeId), *targetClass = KClass_(targetExpr->attrTypeId);
		if(selfClass->typeId == targetClass->typeId || selfClass->isSubType(kctx, selfClass, targetClass)) {
			KReturn(selfExpr);
		}
		if(selfClass->isSubType(kctx, targetClass, selfClass)) {
			kNameSpace *ns = Stmt_ns(stmt);
			kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, KClass_Object, KMethodName_("as"), 0, KMethodMatch_CamelStyle);
			DBG_ASSERT(mtd != NULL);
			KReturn(SUGAR kStmtkExpr_TypeCheckCallParam(kctx, stmt, expr, mtd, gma, targetClass));
		}
		KReturn(kStmtExpr_Message(kctx, stmt, selfExpr, ErrTag, "unable to downcast: %s as %s", KType_text(selfExpr->attrTypeId), KType_text(targetExpr->attrTypeId)));
	}
}

static KMETHOD TypeCheck_to(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kExpr *targetExpr = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 2, gma, KClass_INFER, 0);
	kExpr *selfExpr   = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 1, gma, KClass_(targetExpr->attrTypeId), TypeCheckPolicy_NOCHECK);
	if(selfExpr != K_NULLEXPR && targetExpr != K_NULLEXPR) {
		KClass *selfClass = KClass_(selfExpr->attrTypeId), *targetClass = KClass_(targetExpr->attrTypeId);
		if(selfExpr->attrTypeId == targetExpr->attrTypeId || selfClass->isSubType(kctx, selfClass, targetClass)) {
			kStmtExpr_Message(kctx, stmt, selfExpr, InfoTag, "no need: %s to %s", KType_text(selfExpr->attrTypeId), KType_text(targetExpr->attrTypeId));
			KReturn(selfExpr);
		}
		kNameSpace *ns = Stmt_ns(stmt);
		kMethod *mtd = KLIB kNameSpace_GetCoercionMethodNULL(kctx, ns, selfClass, targetClass);
		if(mtd == NULL) {
			mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, selfClass, KMethodName_("to"), 0, KMethodMatch_CamelStyle);
			DBG_ASSERT(mtd != NULL);  // because Object.to is found.
			if(mtd->typeId != selfClass->typeId) {
				KReturn(kStmtExpr_Message(kctx, stmt, selfExpr, ErrTag, "undefined coercion: %s to %s", KClass_text(selfClass), KClass_text(targetClass)));
			}
		}
		KReturn(SUGAR kStmtkExpr_TypeCheckCallParam(kctx, stmt, expr, mtd, gma, targetClass));
	}
}


static kbool_t subtype_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("<:"), 0, NULL, Precedence_CStyleMUL, 0, NULL, NULL, NULL, NULL, TypeCheck_InstanceOf, },
		{ KSymbol_("as"), 0, NULL, Precedence_CStyleMUL, 0, NULL, NULL, NULL, NULL, TypeCheck_as},
		{ KSymbol_("to"), 0, NULL, Precedence_CStyleMUL, 0, NULL, NULL, NULL, NULL, TypeCheck_to},
		{ KSymbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}

// --------------------------------------------------------------------------

static kbool_t object_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
//	KRequirePackage("konoha.subtype", trace);
	KDEFINE_INT_CONST ClassData[] = {   // add Object as available
		{"Object", VirtualType_KClass, (uintptr_t)KClass_(KType_Object)},
		{NULL},
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(ClassData), false/*isOverride*/, trace);
	object_defineMethod(kctx, ns, trace);
	subtype_defineSyntax(kctx, ns, trace);
	return true;
}

static kbool_t object_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* object_Init(void)
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