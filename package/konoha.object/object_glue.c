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

//## String Object.toString();
static KMETHOD Object_toString(KonohaContext *kctx, KonohaStack *sfp)
{
	KGrowingBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	if(TY_isUnbox(O_typeId(sfp[0].asObject))) {
		sfp[0].unboxValue = (sfp[0].asNumber)->unboxValue;
		O_ct(sfp[0].asObject)->p(kctx, sfp, 0, &wb);
	}
	else {
		O_ct(sfp[0].asObject)->p(kctx, sfp, 0, &wb);
//		kObject_writeToBuffer(kctx, sfp[0].asObject, false/*delim*/, &wb, sfp, 0);
	}
	kString* returnValue = KLIB new_kString(kctx, OnStack, KLIB Kwb_top(kctx, &wb, 1), Kwb_bytesize(&wb), 0);
	KLIB Kwb_free(&wb);
	KReturn(returnValue);
}

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
	KonohaClass *selfClass = O_ct(sfp[0].asObject), *targetClass = O_ct(sfp[1].asObject);
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

// @SmartReturn Object Object.to(Object target)
static KMETHOD Object_to(KonohaContext *kctx, KonohaStack *sfp)
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
	KReturn(returnValue);
}

// void NameSpace_AllowImplicitCoercion(boolean t)
static KMETHOD NameSpace_AllowImplicitCoercion(KonohaContext *kctx, KonohaStack *sfp)
{
	kNameSpaceVar *ns = (kNameSpaceVar*)sfp[0].asNameSpace;
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
		_Public|_Im|_Const, _F(Object_toString), TY_String, TY_Object, MN_to(TY_String), 0,
		_Public|_Im|_Const|_Final, _F(Object_getTypeId), TY_int, TY_Object, MN_("getTypeId"), 0,
		_Public|_Hidden|_Im|_Const, _F(Object_instanceOf), TY_boolean, TY_Object, MN_("<:"), 1, TY_Object, FN_("type"),
		_Public|_Hidden|_Im|_Const|kMethod_SmartReturn, _F(Object_as), TY_Object, TY_Object, MN_("as"), 1, TY_Object, FN_("type"),

		_Public|_Const|_Ignored, _F(NameSpace_AllowImplicitCoercion), TY_void, TY_NameSpace, MN_("AllowImplicitCoercion"), 1, TY_boolean, FN_("allow"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

/* Syntax */

static KMETHOD TypeCheck_InstanceOf(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kExpr *selfExpr   = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 1, gma, TY_var, 0);
	kExpr *targetExpr = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 2, gma, TY_var, 0);
	if(selfExpr != K_NULLEXPR && targetExpr != K_NULLEXPR) {
		KonohaClass *selfClass = CT_(selfExpr->ty), *targetClass = CT_(targetExpr->ty);
		if(CT_is(Final, selfClass)) {
			kbool_t staticSubType = (selfClass == targetClass || selfClass->isSubType(kctx, selfClass, targetClass));
			KReturn(SUGAR kExpr_setUnboxConstValue(kctx, expr, TY_boolean, staticSubType));
		}
		kNameSpace *ns = Stmt_nameSpace(stmt);
		kMethod *mtd = KLIB kNameSpace_getMethodByParamSizeNULL(kctx, ns, TY_Object, MN_("<:"), 1);
		DBG_ASSERT(mtd != NULL);
		KFieldSet(expr->cons, expr->cons->MethodItems[0], mtd);
		kExpr *classValue = SUGAR kExpr_setConstValue(kctx,
				expr->cons->ExprItems[2], targetExpr->ty, KLIB Knull(kctx, targetClass));
		KFieldSet(expr->cons, expr->cons->ExprItems[2], classValue);
		KReturn(SUGAR kStmtExpr_TypeCheckCallParam(kctx, stmt, expr, mtd, gma, TY_boolean));
	}
}

static KMETHOD TypeCheck_As(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kExpr *targetExpr = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 2, gma, TY_var, 0);
	kExpr *selfExpr   = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 1, gma, targetExpr->ty, 0);
	if(selfExpr != K_NULLEXPR && targetExpr != K_NULLEXPR) {
		KonohaClass *selfClass = CT_(selfExpr->ty), *targetClass = CT_(targetExpr->ty);
		if(selfExpr->ty == targetExpr->ty || selfClass->isSubType(kctx, selfClass, targetClass)) {
			KReturn(selfExpr);
		}
		kNameSpace *ns = Stmt_nameSpace(stmt);
		kMethod *mtd = KLIB kNameSpace_getMethodByParamSizeNULL(kctx, ns, TY_Object, MN_("as"), 0);
		DBG_ASSERT(mtd != NULL);
		KFieldSet(expr->cons, expr->cons->MethodItems[0], mtd);
		kExpr *classValue = SUGAR kExpr_setConstValue(kctx,
				expr->cons->ExprItems[2], targetExpr->ty, KLIB Knull(kctx, targetClass));
		KFieldSet(expr->cons, expr->cons->ExprItems[2], classValue);
		KReturn(SUGAR kStmtExpr_TypeCheckCallParam(kctx, stmt, expr, mtd, gma, targetClass->typeId));
	}
}

static kbool_t subtype_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("<:"), 0, NULL, Precedence_CStyleMUL, 0, NULL, NULL, NULL, NULL, TypeCheck_InstanceOf, },
		{ SYM_("as"), 0, NULL, Precedence_CStyleMUL, 0, NULL, NULL, NULL, NULL, TypeCheck_As},
		{ KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX);
	return true;
}

// --------------------------------------------------------------------------

static kbool_t object_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
//	KRequirePackage("konoha.subtype", trace);
	KDEFINE_INT_CONST ClassData[] = {   // add Object as available
		{"Object", VirtualType_KonohaClass, (uintptr_t)CT_(TY_Object)},
		{NULL},
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(ClassData), 0);
	object_defineMethod(kctx, ns, trace);
	subtype_defineSyntax(kctx, ns, trace);
	return true;
}

static kbool_t object_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* object_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", "1.0");
	d.initPackage    = object_initPackage;
	d.setupPackage   = object_setupPackage;
	return &d;
}

#ifdef __cplusplus
}
#endif
