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
#include <konoha3/konoha_common.h>
#include <konoha3/import/methoddecl.h>

#ifdef __cplusplus
extern "C"{
#endif


static void kPrototype_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	KLIB KBuffer_Write(kctx, wb, "{", 1);
	KLIB kObjectProto_format(kctx, v, pos, wb, 0);
	KLIB KBuffer_Write(kctx, wb, "}", 1);
}

static void prototype_defineClass(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	if(KClass_Prototype == NULL) {
		KDEFINE_CLASS defPrototype = {
			.structname = "Prototype",
			.typeId = KTypeAttr_NewId,
			.baseTypeId = KType_Object,
			.format = kPrototype_format,
		};
		KClass_Prototype = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defPrototype, trace);
	}
}

// @SmartReturn Object Prototype.get(Symbol symbol)
static KMETHOD Prototype_get(KonohaContext *kctx, KonohaStack *sfp)
{
	KClass *targetClass = KGetReturnType(sfp);
	DBG_P("requesting type=%s", KClass_text(targetClass));
	ksymbol_t symbol = sfp[1].intValue;
	KKeyValue *kvs = KLIB kObjectProto_GetKeyValue(kctx, sfp[0].asObject, symbol);
	if(kvs != NULL) {
		KClass *c = KClass_(kvs->attrTypeId);
		if(targetClass == c) {
			if(KClass_Is(UnboxType, targetClass)) {
				KReturnUnboxValue(kvs->unboxValue);
			}
			else {
				KReturnField(kvs->ObjectValue);
			}
		}
		DBG_P("requesting type=%s instanceof %s ? %d", KClass_text(c), KClass_text(targetClass), c->isSubType(kctx, c, targetClass));
		if(c->isSubType(kctx, c, targetClass)) {
			if(KClass_Is(UnboxType, c)) {
				if(KClass_Is(UnboxType, targetClass)) {
					KReturnUnboxValue(kvs->unboxValue);
				}
				else {
					DBG_P("boxing type=%s instanceof %s ? %d", KClass_text(c), KClass_text(targetClass), c->isSubType(kctx, c, targetClass));
					KReturn(KLIB new_kObject(kctx, OnStack, c, kvs->unboxValue));
				}
			}
			KReturnField(kvs->ObjectValue);
		}
	}
	KReturnUnboxValue(0);  // return default value
}

// @SmartReturn void Object.set(Symbol symbol, Object value)
static KMETHOD Prototype_SetObject(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t symbol = (ksymbol_t)sfp[1].intValue;
	KClass *c = kObject_class(sfp[2].asObject);
	if(KClass_Is(UnboxType, c)) {
		KLIB kObjectProto_SetUnboxValue(kctx, sfp[0].asObject, symbol, c->typeId, kObject_Unbox(sfp[2].asObject));
	}
	else {
		KLIB kObjectProto_SetObject(kctx, sfp[0].asObject, symbol, c->typeId, sfp[2].asObject);
	}
}

// @SmartReturn void Object.set(Symbol symbol, int value)
static KMETHOD Prototype_SetInt(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t symbol = (ksymbol_t)sfp[1].intValue;
	KLIB kObjectProto_SetUnboxValue(kctx, sfp[0].asObject, symbol, KType_Int, sfp[2].unboxValue);
}

static void ThrowTypeError(KonohaContext *kctx, KonohaStack *sfp, int argc)
{
	KLIB KRuntime_raise(kctx, KException_("TypeError"), SoftwareFault, NULL, sfp);
}

static void KStackDynamicTypeCheck(KonohaContext *kctx, KonohaStack *sfp, kMethod *mtd, KClass *thisClass)
{
	kushort_t i;
	kParam *pa = kMethod_GetParam(mtd);
	for(i = 0; i < pa->psize; i++) {
		KClass *objectType = kObject_class(sfp[i+1].asObject);
		KClass *paramType = KClass_(pa->paramtypeItems[i].attrTypeId);
		paramType = paramType->realtype(kctx, paramType, thisClass);
		if(objectType == paramType || objectType->isSubType(kctx, objectType, paramType)) {
			if(KClass_Is(UnboxType, paramType)) {
				sfp[i+1].unboxValue = kObject_Unbox(sfp[i+1].asObject);
			}
			continue; // OK
		}
		ThrowTypeError(kctx, sfp, i + 1);
	}
}

static void KStackReturnTypeCheck(KonohaContext *kctx, KonohaStack *sfp, kMethod *mtd, KClass *reqType, KClass *thisClass)
{
	if(!kMethod_Is(SmartReturn, mtd)) {
		KClass *returnType = kMethod_GetReturnType(mtd);
		returnType = returnType->realtype(kctx, returnType, thisClass);
		if(reqType == returnType || returnType->isSubType(kctx, returnType, reqType)) {
			if(KClass_Is(UnboxType, returnType) && !KClass_Is(UnboxType, reqType)) {
				KReturn(KLIB new_kObject(kctx, OnStack, returnType, sfp[K_RTNIDX].unboxValue));
			}
		}
		else {
			ThrowTypeError(kctx, sfp, -1);
		}
	}
}

#define KDynamicCallArgument(sfp)         kctx->esp - sfp - 2
#define KDynamicCallSymbol(sfp)          (ksymbol_t)kctx->esp[-1].intValue

//## Prototype Prototype.(Object o);
static KMETHOD Prototype_(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t symbol = KDynamicCallSymbol(sfp);
	KKeyValue *kvs = KLIB kObjectProto_GetKeyValue(kctx, sfp[0].asObject, symbol);
	if(kvs != NULL) {
		KClass *c = KClass_(kvs->attrTypeId);
		kParam *cparam = KClass_cparam(c);
		if(KClass_isFunc(c) && cparam->psize <= KDynamicCallArgument(sfp)) {
			KClass *thisClass = kObject_class(sfp[0].asObject), *returnType = KGetReturnType(sfp);
			kFunc *fo = (kFunc *)kvs->FuncValue;
			KStackSetFunc(sfp, fo);
			KStackDynamicTypeCheck(kctx, sfp, fo->method, thisClass);
			KStackCall(sfp);
			KStackReturnTypeCheck(kctx, sfp, fo->method, thisClass, returnType);
		}
	}
}

static void prototype_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	int FN_key = KFieldName_("key"), FN_value = KFieldName_("value");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Im|_Const|kMethod_SmartReturn|_Final, _F(Prototype_get), KType_Object, KType_Prototype, KMethodName_("get"), 1, KType_Symbol, FN_key,
		_Public|_Final, _F(Prototype_SetObject), KType_void, KType_Prototype, KMethodName_("set"), 2, KType_Symbol, FN_key, KType_Object, FN_value,
		_Public|_Final, _F(Prototype_SetInt), KType_void, KType_Prototype, KMethodName_("set"), 2, KType_Symbol, FN_key, KType_Int, FN_value,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

// --------------------------------------------------------------------------

static kbool_t prototype_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequireKonohaCommonModule(trace);
	prototype_defineClass(kctx, ns, option, trace);
	prototype_defineMethod(kctx, ns, trace);
	return true;
}

static kbool_t prototype_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE *Prototype_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "JavaScript", "1.4");
	d.PackupNameSpace = prototype_PackupNameSpace;
	d.ExportNameSpace = prototype_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
