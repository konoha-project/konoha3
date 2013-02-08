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

#include <konoha3/import/methoddecl.h>

static void object_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Im|_Const|_Final, _F(Object_getTypeId), KType_Int, KType_Object, KMethodName_("getTypeId"), 0,
		_Public|_Hidden|_Im|_Const|_Final, _F(Object_instanceOf), KType_Boolean, KType_Object, KMethodName_("instanceof"), 1, KType_Object, KFieldName_("type"),
		_Public|_Hidden|_Im|_Const|kMethod_SmartReturn|_Final, _F(Object_as), KType_Object, KType_Object, KMethodName_("as"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

/* Syntax */

static KMETHOD TypeCheck_InstanceOf(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck2(stmt, expr, ns, reqc);
	/* selfNode and targetNode allow void type
	 * e.g. "'a' instanceof void" */
	kNode *selfNode   = SUGAR TypeCheckNodeAt(kctx, expr, 1, ns, KClass_INFER, TypeCheckPolicy_AllowVoid);
	kNode *targetNode = SUGAR TypeCheckNodeAt(kctx, expr, 2, ns, KClass_INFER, TypeCheckPolicy_AllowVoid);
	if(selfNode != K_NULLNODE && targetNode != K_NULLNODE) {
		KClass *selfClass = KClass_(selfNode->attrTypeId), *targetClass = KClass_(targetNode->attrTypeId);
		if(KClass_Is(Final, selfClass)) {
			kbool_t staticSubType = (selfClass == targetClass || selfClass->isSubType(kctx, selfClass, targetClass));
			KReturn(SUGAR kNode_SetUnboxConst(kctx, expr, KType_Boolean, staticSubType));
		}
		kNameSpace *ns = kNode_ns(stmt);
		kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, KClass_Object, KMethodName_("instanceof"), 1, KMethodMatch_NoOption);
		DBG_ASSERT(mtd != NULL);
		kNode *classValue = SUGAR kNode_SetConst(kctx, expr->NodeList->NodeVarItems[2], NULL, KLIB Knull(kctx, targetClass));
		KFieldSet(expr->NodeList, expr->NodeList->NodeItems[2], classValue);
		KReturn(SUGAR TypeCheckMethodParam(kctx, mtd, expr, ns, KClass_Boolean));
	}
}

static KMETHOD TypeCheck_as(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck2(stmt, expr, ns, reqc);
	kNode *targetNode = SUGAR TypeCheckNodeAt(kctx, expr, 2, ns, KClass_INFER, 0);
	kNode *selfNode   = SUGAR TypeCheckNodeAt(kctx, expr, 1, ns, KClass_(targetNode->attrTypeId), TypeCheckPolicy_NoCheck);
	if(selfNode != K_NULLNODE && targetNode != K_NULLNODE) {
		KClass *selfClass = KClass_(selfNode->attrTypeId), *targetClass = KClass_(targetNode->attrTypeId);
		if(selfClass->typeId == targetClass->typeId || selfClass->isSubType(kctx, selfClass, targetClass)) {
			KReturn(selfNode);
		}
		if(selfClass->isSubType(kctx, targetClass, selfClass)) {
			kNameSpace *ns = kNode_ns(stmt);
			kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, KClass_Object, KMethodName_("as"), 0, KMethodMatch_CamelStyle);
			DBG_ASSERT(mtd != NULL);
			KReturn(SUGAR TypeCheckMethodParam(kctx, mtd, expr, ns, targetClass));
		}
		KReturn(SUGAR MessageNode(kctx, selfNode, NULL, ns, ErrTag, "unable to downcast: %s as %s", KType_text(selfNode->attrTypeId), KType_text(targetNode->attrTypeId)));
	}
}

static KMETHOD TypeCheck_to(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck2(stmt, expr, ns, reqc);
	kNode *targetNode = SUGAR TypeCheckNodeAt(kctx, expr, 2, ns, KClass_INFER, 0);
	kNode *selfNode   = SUGAR TypeCheckNodeAt(kctx, expr, 1, ns, KClass_(targetNode->attrTypeId), TypeCheckPolicy_NoCheck);
	if(selfNode != K_NULLNODE && targetNode != K_NULLNODE) {
		KClass *selfClass = KClass_(selfNode->attrTypeId), *targetClass = KClass_(targetNode->attrTypeId);
		if(selfNode->attrTypeId == targetNode->attrTypeId || selfClass->isSubType(kctx, selfClass, targetClass)) {
			SUGAR MessageNode(kctx, selfNode, NULL, ns, InfoTag, "no need: %s to %s", KType_text(selfNode->attrTypeId), KType_text(targetNode->attrTypeId));
			KReturn(selfNode);
		}
		kNameSpace *ns = kNode_ns(stmt);
		kMethod *mtd = KLIB kNameSpace_GetCoercionMethodNULL(kctx, ns, selfClass, targetClass);
		if(mtd == NULL) {
			mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, selfClass, KMethodName_("to"), 0, KMethodMatch_CamelStyle);
			DBG_ASSERT(mtd != NULL);  // because Object.to is found.
			if(mtd->typeId != selfClass->typeId) {
				KReturn(SUGAR MessageNode(kctx, selfNode, NULL, ns, ErrTag, "undefined coercion: %s to %s", KClass_text(selfClass), KClass_text(targetClass)));
			}
		}
		KReturn(SUGAR TypeCheckMethodParam(kctx, mtd, expr, ns, targetClass));
	}
}


static kbool_t subtype_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("instanceof"), SYNFLAG_CTypeFunc, Precedence_CStyleMUL, 0, {SUGAR opParseFunc}, {SUGARFUNC TypeCheck_InstanceOf}},
		{ KSymbol_("as"), SYNFLAG_CTypeFunc, Precedence_CStyleMUL, 0, {SUGAR opParseFunc}, {SUGARFUNC TypeCheck_as}},
		{ KSymbol_("to"), SYNFLAG_CTypeFunc, Precedence_CStyleMUL, 0, {SUGAR opParseFunc}, {SUGARFUNC TypeCheck_to}},
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
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(ClassData), trace);
	object_defineMethod(kctx, ns, trace);
	subtype_defineSyntax(kctx, ns, trace);
	return true;
}

static kbool_t object_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE *Object_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "JavaStyle", K_VERSION);
	d.PackupNameSpace    = object_PackupNameSpace;
	d.ExportNameSpace   = object_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
