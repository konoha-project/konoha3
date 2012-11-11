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

typedef const struct _kDynamic kDynamic;
struct _kDynamic {
	KonohaObjectHeader h;
};

//static void Dynamic_init(KonohaContext *kctx, kObject *o, void *conf)
//{
//}
//
//static void Dynamic_p(KonohaContext *kctx, KonohaValue *v, int pos, KGrowingBuffer *wb)
//{
//}
//
//static void Dynamic_free(KonohaContext *kctx, kObject *o)
//{
//}

static kbool_t kMethod_checkMethodCallStack(KonohaContext *kctx, KonohaStack *sfp, kMethod *mtd, int argc)
{
	int i;
	kParam *param = Method_param(mtd);
	for(i = 1; i <= argc; i++) {
		KonohaClass *paramClass = O_ct(sfp[i].asObject);
		ktype_t ptype = param->paramtypeItems[i-1].ty;
		if(ptype != paramClass->typeId) {
			return false;
		}
		if(CT_isUnbox(paramClass)) {
			sfp[i].unboxValue = O_unbox(sfp[i].asObject);
		}
	}
	return true;

}

//## Object Dynamic.(Object o);
static KMETHOD Dynamic_(KonohaContext *kctx, KonohaStack *sfp)
{
	kObject *obj = sfp[0].asObject;
	int argc = kctx->esp - sfp - 2;   // believe me
	ksymbol_t symbol = (ksymbol_t)(kctx->esp[-1].intValue);
//	kString  *symbolString = kctx->esp[-1].asString;
	kNameSpace *ns = sfp[K_NSIDX].asNameSpace;
	DBG_ASSERT(IS_NameSpace(ns));
	kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, O_typeId(obj), symbol, argc);
	if(mtd != NULL) {
		if(kMethod_checkMethodCallStack(kctx, sfp, mtd, argc)) {
			KonohaRuntime_setesp(kctx, kctx->esp - 1);
			sfp[K_MTDIDX].calledMethod = mtd;
			//kObject *returnValue = sfp[K_RTNIDX].asObject;
			KonohaRuntime_callMethod(kctx, sfp);
			return;
		}
	}
	KLIB KonohaRuntime_raise(kctx, EXPT_("NoSuchMethod"), SoftwareFault, NULL, sfp);
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Im       kMethod_Immutable
#define _F(F)   (intptr_t)(F)

static kbool_t dynamic_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	static KDEFINE_CLASS defDynamic = {0};
	defDynamic.structname = "dynamic";
	defDynamic.cflag = kClass_Final|kClass_TypeVar;
	KonohaClass *cDynamic = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defDynamic, trace);
	int TY_Dynamic = cDynamic->typeId;
	KDEFINE_METHOD MethodData[] = {
		_Public|_Im, _F(Dynamic_), TY_Object, TY_Dynamic, MN_(""), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

static kbool_t dynamic_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* dynamic_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", "1.0");
	d.PackupNameSpace     = dynamic_PackupNameSpace;
	d.ExportNameSpace = dynamic_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
