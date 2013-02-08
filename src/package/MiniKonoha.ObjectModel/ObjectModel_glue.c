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

// ---------------------------------------------------------------------------
/* KLIB extension */

static KMETHOD KMethodFunc_ObjectFieldGetter(KonohaContext *kctx, KonohaStack *sfp)
{
	size_t delta = sfp[K_MTDIDX].calledMethod->delta;
	KReturn((sfp[0].asObject)->fieldObjectItems[delta]);
}
static KMETHOD KMethodFunc_UnboxFieldGetter(KonohaContext *kctx, KonohaStack *sfp)
{
	size_t delta = sfp[K_MTDIDX].calledMethod->delta;
	KReturnUnboxValue((sfp[0].asObject)->fieldUnboxItems[delta]);
}
static KMETHOD KMethodFunc_ObjectFieldSetter(KonohaContext *kctx, KonohaStack *sfp)
{
	size_t delta = sfp[K_MTDIDX].calledMethod->delta;
	kObjectVar *o = sfp[0].asObjectVar;
	KFieldSet(o, o->fieldObjectItems[delta], sfp[1].asObject);
	KReturn(sfp[1].asObject);
}
static KMETHOD KMethodFunc_UnboxFieldSetter(KonohaContext *kctx, KonohaStack *sfp)
{
	size_t delta = sfp[K_MTDIDX].calledMethod->delta;
	(sfp[0].asObjectVar)->fieldUnboxItems[delta] = sfp[1].unboxValue;
	KReturnUnboxValue(sfp[1].unboxValue);
}
static kMethod *new_FieldGetter(KonohaContext *kctx, kArray *gcstack, ktypeattr_t cid, ksymbol_t sym, ktypeattr_t ty, int idx)
{
	kmethodn_t mn = KMethodName_ToGetter(sym);
	KMethodFunc f = (KType_Is(UnboxType, ty)) ? KMethodFunc_UnboxFieldGetter : KMethodFunc_ObjectFieldGetter;
	kMethod *mtd = KLIB new_kMethod(kctx, gcstack, kMethod_Public|kMethod_Immutable, cid, mn, f);
	KLIB kMethod_SetParam(kctx, mtd, ty, 0, NULL);
	((kMethodVar *)mtd)->delta = idx;  // FIXME
	return mtd;
}

static kMethod *new_FieldSetter(KonohaContext *kctx, kArray *gcstack, ktypeattr_t cid, kmethodn_t sym, ktypeattr_t ty, int idx)
{
	kmethodn_t mn = KMethodName_ToSetter(sym);
	KMethodFunc f = (KType_Is(UnboxType, ty)) ? KMethodFunc_UnboxFieldSetter : KMethodFunc_ObjectFieldSetter;
	kparamtype_t p = {ty, KFieldName_("x")};
	kMethod *mtd = KLIB new_kMethod(kctx, gcstack, kMethod_Public, cid, mn, f);
	KLIB kMethod_SetParam(kctx, mtd, ty, 1, &p);
	((kMethodVar *)mtd)->delta = idx;   // FIXME
	return mtd;
}

static intptr_t KLIB2_Method_indexOfField(kMethod *mtd)
{
	KMethodFunc f = mtd->invokeKMethodFunc;
	if(f== KMethodFunc_ObjectFieldGetter || f == KMethodFunc_UnboxFieldGetter || f == KMethodFunc_ObjectFieldSetter || f == KMethodFunc_UnboxFieldSetter) {
		return (intptr_t)mtd->delta;
	}
	return -1;
}

static KMETHOD KMethodFunc_ObjectPrototypeGetter(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[K_MTDIDX].calledMethod;
	ksymbol_t key = (ksymbol_t)mtd->delta;
	KReturn(KLIB kObject_getObject(kctx, sfp[0].asObject, key, sfp[K_RTNIDX].asObject));
}

static KMETHOD KMethodFunc_UnboxPrototypeGetter(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[K_MTDIDX].calledMethod;
	ksymbol_t key = (ksymbol_t)mtd->delta;
	KKeyValue *kvs = KLIB kObjectProto_GetKeyValue(kctx, sfp[0].asObject, key);
	KReturnUnboxValue((kvs == NULL) ? 0 : kvs->unboxValue);
}

static KMETHOD KMethodFunc_ObjectPrototypeSetter(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[K_MTDIDX].calledMethod;
	ksymbol_t key = (ksymbol_t)mtd->delta;
	KLIB kObjectProto_SetObject(kctx, sfp[0].asObject, key, kObject_typeId(sfp[1].asObject), sfp[1].asObject);
	KReturn(sfp[1].asObject);
}

static KMETHOD KMethodFunc_UnboxPrototypeSetter(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[K_MTDIDX].calledMethod;
	ksymbol_t key = (ksymbol_t)mtd->delta;
	kParam *pa = kMethod_GetParam(mtd);
	KLIB kObjectProto_SetUnboxValue(kctx, sfp[0].asObject, key, pa->paramtypeItems[0].attrTypeId, sfp[1].unboxValue);
	KReturnUnboxValue(sfp[1].unboxValue);
}

static kMethod *new_PrototypeGetter(KonohaContext *kctx, kArray *gcstack, ktypeattr_t cid, ksymbol_t sym, ktypeattr_t ty)
{
	kmethodn_t mn = KMethodName_ToGetter(sym);
	KMethodFunc f = (KType_Is(UnboxType, ty)) ? KMethodFunc_UnboxPrototypeGetter : KMethodFunc_ObjectPrototypeGetter;
	kMethod *mtd = KLIB new_kMethod(kctx, gcstack, kMethod_Public|kMethod_Immutable, cid, mn, f);
	KLIB kMethod_SetParam(kctx, mtd, ty, 0, NULL);
	((kMethodVar *)mtd)->delta = sym;
	return mtd;
}

static kMethod *new_PrototypeSetter(KonohaContext *kctx, kArray *gcstack, ktypeattr_t cid, ksymbol_t sym, ktypeattr_t ty)
{
	kmethodn_t mn = KMethodName_ToSetter(sym);
	KMethodFunc f = (KType_Is(UnboxType, ty)) ? KMethodFunc_UnboxPrototypeSetter : KMethodFunc_ObjectPrototypeSetter;
	kparamtype_t p = {ty, KFieldName_("x")};
	kMethod *mtd = KLIB new_kMethod(kctx, gcstack, kMethod_Public, cid, mn, f);
	KLIB kMethod_SetParam(kctx, mtd, ty, 1, &p);
	((kMethodVar *)mtd)->delta = sym;
	return mtd;
}

static kbool_t KClass_AddField(KonohaContext *kctx, KClass *ct, ktypeattr_t typeattr, ksymbol_t sym)
{
	kushort_t pos = ct->fieldsize;
	if(unlikely(ct->classMethodList == K_EMPTYARRAY)) {
		((KClassVar *)ct)->classMethodList = new_(MethodArray, 8, OnGlobalConstList);
		/*FIXME WriteBarrier */
	}
	INIT_GCSTACK();
	if(pos < ct->fieldAllocSize) {
		KClassVar *definedClass = (KClassVar *)ct;
		definedClass->fieldsize += 1;
		definedClass->fieldItems[pos].name = sym;
		if(KType_Is(UnboxType, typeattr)) {
			definedClass->defaultNullValueVar->fieldUnboxItems[pos] = 0;
			definedClass->fieldItems[pos].attrTypeId = typeattr;
		}
		else {
			kObjectVar *o = definedClass->defaultNullValueVar;
			KFieldSet(o, o->fieldObjectItems[pos], KLIB Knull(kctx, KClass_(typeattr)));
			definedClass->fieldItems[pos].attrTypeId = typeattr | KTypeAttr_Boxed;
		}
		if(1/*FLAG_is(flag, kField_Getter)*/) {
			kMethod *mtd = new_FieldGetter(kctx, _GcStack, definedClass->typeId, sym, KTypeAttr_Unmask(typeattr), pos);
			KLIB kArray_Add(kctx, ct->classMethodList, mtd);
		}
		if(!KTypeAttr_Is(ReadOnly, typeattr)/*FLAG_is(flag, kField_Setter)*/) {
			kMethod *mtd = new_FieldSetter(kctx, _GcStack, definedClass->typeId, sym, KTypeAttr_Unmask(typeattr), pos);
			KLIB kArray_Add(kctx, ct->classMethodList, mtd);
		}
	}
	else {
		if(1/*FLAG_is(flag, kField_Getter)*/) {
			kMethod *mtd = new_PrototypeGetter(kctx, _GcStack, ct->typeId, sym, KTypeAttr_Unmask(typeattr));
			KLIB kArray_Add(kctx, ct->classMethodList, mtd);
		}
		if(!KTypeAttr_Is(ReadOnly, typeattr)/*FLAG_is(flag, kField_Setter)*/) {
			kMethod *mtd = new_PrototypeSetter(kctx, _GcStack, ct->typeId, sym, KTypeAttr_Unmask(typeattr));
			KLIB kArray_Add(kctx, ct->classMethodList, mtd);
		}
	}
	RESET_GCSTACK();
	return true;
}

// --------------------------------------------------------------------------

static KMETHOD TypeCheck_Getter(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck2(stmt, expr, ns, reqc);
	kToken *fieldToken = expr->NodeList->TokenItems[0];
	ksymbol_t fn = fieldToken->symbol;
	kNode *self = SUGAR TypeCheckNodeAt(kctx, expr, 1, ns, KClass_INFER, 0);
	if(self != K_NULLNODE) {
		kMethod *mtd = KLIB kNameSpace_GetGetterMethodNULL(kctx, ns, KClass_(self->attrTypeId), fn);
		if(mtd != NULL) {
			KFieldSet(expr->NodeList, expr->NodeList->MethodItems[0], mtd);
			KReturn(SUGAR TypeCheckMethodParam(kctx, mtd, expr, ns, reqc));
		}
		else {  // dynamic field    o.name => o.get(name)
			kparamtype_t p[1] = {{KType_Symbol}};
			kparamId_t paramdom = KLIB Kparamdom(kctx, 1, p);
			mtd = KLIB kNameSpace_GetMethodBySignatureNULL(kctx, ns, KClass_(self->attrTypeId), KMethodNameAttr_Getter, paramdom, 1, p);
			if(mtd != NULL) {
				KFieldSet(expr->NodeList, expr->NodeList->MethodItems[0], mtd);
				KLIB kArray_Add(kctx, expr->NodeList, new_UnboxConstNode(kctx, ns, KType_Symbol, KSymbol_Unmask(fn)));
				KReturn(SUGAR TypeCheckMethodParam(kctx, mtd, expr, ns, reqc));
			}
		}
		SUGAR MessageNode(kctx, stmt, fieldToken, ns, ErrTag, "undefined field: %s", kString_text(fieldToken->text));
	}
}

static kbool_t field_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("."), SYNFLAG_CTypeFunc, -1, 0, {NULL}, {SUGARFUNC TypeCheck_Getter}},
		{ KSymbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}

// --------------------------------------------------------------------------

static kbool_t field_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KSetKLibFunc(ns->packageId, kMethod_indexOfField, KLIB2_Method_indexOfField, trace);
	KSetKLibFunc(ns->packageId, KClass_AddField, KClass_AddField, trace);
	field_defineSyntax(kctx, ns, trace);
	return true;
}

static kbool_t field_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE *ObjectModel_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "MiniKonoha", K_VERSION);
	d.PackupNameSpace    = field_PackupNameSpace;
	d.ExportNameSpace   = field_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
