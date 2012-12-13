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

// ---------------------------------------------------------------------------
/* KLIB extension */

static KMETHOD MethodFunc_ObjectFieldGetter(KonohaContext *kctx, KonohaStack *sfp)
{
	size_t delta = sfp[K_MTDIDX].calledMethod->delta;
	KReturn((sfp[0].asObject)->fieldObjectItems[delta]);
}
static KMETHOD MethodFunc_UnboxFieldGetter(KonohaContext *kctx, KonohaStack *sfp)
{
	size_t delta = sfp[K_MTDIDX].calledMethod->delta;
	KReturnUnboxValue((sfp[0].asObject)->fieldUnboxItems[delta]);
}
static KMETHOD MethodFunc_ObjectFieldSetter(KonohaContext *kctx, KonohaStack *sfp)
{
	size_t delta = sfp[K_MTDIDX].calledMethod->delta;
	kObjectVar *o = sfp[0].asObjectVar;
	KFieldSet(o, o->fieldObjectItems[delta], sfp[1].asObject);
	KReturn(sfp[1].asObject);
}
static KMETHOD MethodFunc_UnboxFieldSetter(KonohaContext *kctx, KonohaStack *sfp)
{
	size_t delta = sfp[K_MTDIDX].calledMethod->delta;
	(sfp[0].asObjectVar)->fieldUnboxItems[delta] = sfp[1].unboxValue;
	KReturnUnboxValue(sfp[1].unboxValue);
}
static kMethod *new_FieldGetter(KonohaContext *kctx, kArray *gcstack, ktypeattr_t cid, ksymbol_t sym, ktypeattr_t ty, int idx)
{
	kmethodn_t mn = MethodName_ToGetter(sym);
	MethodFunc f = (KType_Is(UnboxType, ty)) ? MethodFunc_UnboxFieldGetter : MethodFunc_ObjectFieldGetter;
	kMethod *mtd = KLIB new_kMethod(kctx, gcstack, kMethod_Public|kMethod_Immutable, cid, mn, f);
	KLIB kMethod_SetParam(kctx, mtd, ty, 0, NULL);
	((kMethodVar *)mtd)->delta = idx;  // FIXME
	return mtd;
}

static kMethod *new_FieldSetter(KonohaContext *kctx, kArray *gcstack, ktypeattr_t cid, kmethodn_t sym, ktypeattr_t ty, int idx)
{
	kmethodn_t mn = MethodName_ToSetter(sym);
	MethodFunc f = (KType_Is(UnboxType, ty)) ? MethodFunc_UnboxFieldSetter : MethodFunc_ObjectFieldSetter;
	kparamtype_t p = {ty, FN_("x")};
	kMethod *mtd = KLIB new_kMethod(kctx, gcstack, kMethod_Public, cid, mn, f);
	KLIB kMethod_SetParam(kctx, mtd, ty, 1, &p);
	((kMethodVar *)mtd)->delta = idx;   // FIXME
	return mtd;
}

static intptr_t KLIB2_Method_indexOfField(kMethod *mtd)
{
	MethodFunc f = mtd->invokeMethodFunc;
	if(f== MethodFunc_ObjectFieldGetter || f == MethodFunc_UnboxFieldGetter || f == MethodFunc_ObjectFieldSetter || f == MethodFunc_UnboxFieldSetter) {
		return (intptr_t)mtd->delta;
	}
	return -1;
}

static KMETHOD MethodFunc_ObjectPrototypeGetter(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[K_MTDIDX].calledMethod;
	ksymbol_t key = (ksymbol_t)mtd->delta;
	KReturn(KLIB kObject_getObject(kctx, sfp[0].asObject, key, sfp[K_RTNIDX].asObject));
}

static KMETHOD MethodFunc_UnboxPrototypeGetter(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[K_MTDIDX].calledMethod;
	ksymbol_t key = (ksymbol_t)mtd->delta;
	KKeyValue *kvs = KLIB kObjectProto_GetKeyValue(kctx, sfp[0].asObject, key);
	KReturnUnboxValue((kvs == NULL) ? 0 : kvs->unboxValue);
}

static KMETHOD MethodFunc_ObjectPrototypeSetter(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[K_MTDIDX].calledMethod;
	ksymbol_t key = (ksymbol_t)mtd->delta;
	KLIB kObjectProto_SetObject(kctx, sfp[0].asObject, key, kObject_typeId(sfp[1].asObject), sfp[1].asObject);
	KReturn(sfp[1].asObject);
}

static KMETHOD MethodFunc_UnboxPrototypeSetter(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[K_MTDIDX].calledMethod;
	ksymbol_t key = (ksymbol_t)mtd->delta;
	kParam *pa = kMethod_GetParam(mtd);
	KLIB kObjectProto_SetUnboxValue(kctx, sfp[0].asObject, key, pa->paramtypeItems[0].attrTypeId, sfp[1].unboxValue);
	KReturnUnboxValue(sfp[1].unboxValue);
}

static kMethod *new_PrototypeGetter(KonohaContext *kctx, kArray *gcstack, ktypeattr_t cid, ksymbol_t sym, ktypeattr_t ty)
{
	kmethodn_t mn = MethodName_ToGetter(sym);
	MethodFunc f = (KType_Is(UnboxType, ty)) ? MethodFunc_UnboxPrototypeGetter : MethodFunc_ObjectPrototypeGetter;
	kMethod *mtd = KLIB new_kMethod(kctx, gcstack, kMethod_Public|kMethod_Immutable, cid, mn, f);
	KLIB kMethod_SetParam(kctx, mtd, ty, 0, NULL);
	((kMethodVar *)mtd)->delta = sym;
	return mtd;
}

static kMethod *new_PrototypeSetter(KonohaContext *kctx, kArray *gcstack, ktypeattr_t cid, ksymbol_t sym, ktypeattr_t ty)
{
	kmethodn_t mn = MethodName_ToSetter(sym);
	MethodFunc f = (KType_Is(UnboxType, ty)) ? MethodFunc_UnboxPrototypeSetter : MethodFunc_ObjectPrototypeSetter;
	kparamtype_t p = {ty, FN_("x")};
	kMethod *mtd = KLIB new_kMethod(kctx, gcstack, kMethod_Public, cid, mn, f);
	KLIB kMethod_SetParam(kctx, mtd, ty, 1, &p);
	((kMethodVar *)mtd)->delta = sym;
	return mtd;
}

static kbool_t KonohaClass_AddField(KonohaContext *kctx, KonohaClass *ct, ktypeattr_t typeattr, ksymbol_t sym)
{
	kushort_t pos = ct->fieldsize;
	if(unlikely(ct->methodList_OnGlobalConstList == K_EMPTYARRAY)) {
		((KonohaClassVar *)ct)->methodList_OnGlobalConstList = new_(MethodArray, 8, OnGlobalConstList);
	}
	INIT_GCSTACK();
	if(pos < ct->fieldAllocSize) {
		KonohaClassVar *definedClass = (KonohaClassVar *)ct;
		definedClass->fieldsize += 1;
		definedClass->fieldItems[pos].name = sym;
		if(KType_Is(UnboxType, typeattr)) {
			definedClass->defaultNullValueVar_OnGlobalConstList->fieldUnboxItems[pos] = 0;
			definedClass->fieldItems[pos].attrTypeId = typeattr;
		}
		else {
			kObjectVar *o = definedClass->defaultNullValueVar_OnGlobalConstList;
			KFieldSet(o, o->fieldObjectItems[pos], KLIB Knull(kctx, KClass_(typeattr)));
			definedClass->fieldItems[pos].attrTypeId = typeattr | TypeAttr_Boxed;
		}
		if(1/*FLAG_is(flag, kField_Getter)*/) {
			kMethod *mtd = new_FieldGetter(kctx, _GcStack, definedClass->typeId, sym, TypeAttr_Unmask(typeattr), pos);
			KLIB kArray_Add(kctx, ct->methodList_OnGlobalConstList, mtd);
		}
		if(!TypeAttr_Is(ReadOnly, typeattr)/*FLAG_is(flag, kField_Setter)*/) {
			kMethod *mtd = new_FieldSetter(kctx, _GcStack, definedClass->typeId, sym, TypeAttr_Unmask(typeattr), pos);
			KLIB kArray_Add(kctx, ct->methodList_OnGlobalConstList, mtd);
		}
	}
	else {
		if(1/*FLAG_is(flag, kField_Getter)*/) {
			kMethod *mtd = new_PrototypeGetter(kctx, _GcStack, ct->typeId, sym, TypeAttr_Unmask(typeattr));
			KLIB kArray_Add(kctx, ct->methodList_OnGlobalConstList, mtd);
		}
		if(!TypeAttr_Is(ReadOnly, typeattr)/*FLAG_is(flag, kField_Setter)*/) {
			kMethod *mtd = new_PrototypeSetter(kctx, _GcStack, ct->typeId, sym, TypeAttr_Unmask(typeattr));
			KLIB kArray_Add(kctx, ct->methodList_OnGlobalConstList, mtd);
		}
	}
	RESET_GCSTACK();
	return true;
}

// --------------------------------------------------------------------------

static KMETHOD TypeCheck_Getter(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kToken *fieldToken = expr->cons->TokenItems[0];
	ksymbol_t fn = fieldToken->resolvedSymbol;
	kExpr *self = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 1, gma, KClass_INFER, 0);
	kNameSpace *ns = Stmt_ns(stmt);
	if(self != K_NULLEXPR) {
		kMethod *mtd = KLIB kNameSpace_GetGetterMethodNULL(kctx, ns, KClass_(self->attrTypeId), fn);
		if(mtd != NULL) {
			KFieldSet(expr->cons, expr->cons->MethodItems[0], mtd);
			KReturn(SUGAR kStmtkExpr_TypeCheckCallParam(kctx, stmt, expr, mtd, gma, KClass_(reqty)));
		}
		else {  // dynamic field    o.name => o.get(name)
			kparamtype_t p[1] = {{KType_Symbol}};
			kparamId_t paramdom = KLIB Kparamdom(kctx, 1, p);
			mtd = KLIB kNameSpace_GetMethodBySignatureNULL(kctx, ns, KClass_(self->attrTypeId), MethodNameAttr_Getter, paramdom, 1, p);
			if(mtd != NULL) {
				KFieldSet(expr->cons, expr->cons->MethodItems[0], mtd);
				KLIB kArray_Add(kctx, expr->cons, new_UnboxConstValueExpr(kctx, KType_Symbol, Symbol_Unmask(fn)));
				KReturn(SUGAR kStmtkExpr_TypeCheckCallParam(kctx, stmt, expr, mtd, gma, KClass_(reqty)));
			}
		}
		SUGAR kStmt_Message2(kctx, stmt, fieldToken, ErrTag, "undefined field: %s", kString_text(fieldToken->text));
	}
}

static kbool_t field_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("."), 0, NULL, -1, 0, NULL, NULL, NULL, NULL, TypeCheck_Getter, },
		{ Symbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}

// --------------------------------------------------------------------------

static kbool_t field_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KSetKLibFunc(ns->packageId, kMethod_indexOfField, KLIB2_Method_indexOfField, trace);
	KSetKLibFunc(ns->packageId, KonohaClass_AddField, KonohaClass_AddField, trace);
	field_defineSyntax(kctx, ns, trace);
	return true;
}

static kbool_t field_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* field_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "field", "1.0");
	d.PackupNameSpace    = field_PackupNameSpace;
	d.ExportNameSpace   = field_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
