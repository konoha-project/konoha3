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

#define  USE_MethodFlagData
#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/klib.h>

#ifdef __cplusplus
extern "C"{
#endif

//struct fn {
//	uintptr_t  flag;
//	const char *aname;
//};
//
//#define _P(T)  kMethod_##T, #T
//
//static struct fn attrs[] = {
//	{_P(Public)}, {_P(Static)}, {_P(Const)}, {_P(Abstract)},
//	{_P(Virtual)}, {_P(Final)}, /*{_P(Overloaded)},*/
//	{_P(Coercion)}, {_P(SmartReturn)},
//	{_P(Restricted)}, {_P(Immutable)},
////	{_P(FASTCALL)}, {_P(CALLCC)},
//	{_P(Hidden)},
//	{0, NULL}
//};
//
//static void MethodAttribute_p(KonohaContext *kctx, kMethod *mtd, KGrowingBuffer *wb)
//{
//	uintptr_t i;
//	for(i = 0; i < 30; i++) {
//		if(attrs[i].aname == NULL) break;
//		if((attrs[i].flag & mtd->flag) == attrs[i].flag) {
//			KLIB Kwb_printf(kctx, wb, "@%s ", attrs[i].aname);
//		}
//	}
//	if(Method_isCast(mtd)) {
//		KLIB Kwb_printf(kctx, wb, "@Cast ");
//	}
////	if(Method_isTransCast(mtd)) {
////		KLIB Kwb_printf(kctx, wb, "@T ");
////	}
//}

static void Method_writeAttributeToBuffer(KonohaContext *kctx, kMethod *mtd, KGrowingBuffer *wb)
{
	size_t i;
	for(i = 0; i < sizeof(MethodFlagData)/sizeof(const char *); i++) {
		uintptr_t flagmask = 1 << i;
		if((mtd->flag && flagmask) == flagmask) {
			KLIB Kwb_printf(kctx, wb, "@%s ", MethodFlagData[i]);
		}
	}
}

static void kMethod_writeToBuffer(KonohaContext *kctx, kMethod *mtd, KGrowingBuffer *wb)
{
	kParam *pa = Method_param(mtd);
	Method_writeAttributeToBuffer(kctx, mtd, wb);
	KLIB Kwb_printf(kctx, wb, "%s %s.%s%s", TY_t(pa->rtype), TY_t(mtd->typeId), T_mn(mtd->mn));
	{
		size_t i;
		KLIB Kwb_write(kctx, wb, "(", 1);
		for(i = 0; i < pa->psize; i++) {
			if(i > 0) {
				KLIB Kwb_write(kctx, wb, ", ", 2);
			}
			if(FN_isCOERCION(pa->paramtypeItems[i].fn)) {
				KLIB Kwb_printf(kctx, wb, "@Coercion ");
			}
			KLIB Kwb_printf(kctx, wb, "%s %s", TY_t(pa->paramtypeItems[i].ty), SYM_t(pa->paramtypeItems[i].fn));
		}
		KLIB Kwb_write(kctx, wb, ")", 1);
	}
}

// --------------------------------------------------------------------------

static void copyMethodList(KonohaContext *kctx, ktype_t cid, kArray *s, kArray *d)
{
	size_t i;
	for(i = 0; i < kArray_size(s); i++) {
		kMethod *mtd = s->MethodItems[i];
		if(mtd->typeId != cid) continue;
		KLIB kArray_add(kctx, d, mtd);
	}
}

static void dumpMethod(KonohaContext *kctx, KonohaStack *sfp, kMethod *mtd)
{
	KGrowingBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	kMethod_writeToBuffer(kctx, mtd, &wb);
	PLATAPI printf_i("%s\n", KLIB Kwb_top(kctx, &wb, 1));
	KLIB Kwb_free(&wb);
	return;
}

static void dumpMethodList(KonohaContext *kctx, KonohaStack *sfp, size_t start, kArray *list)
{
	size_t i;
	for(i = start; i < kArray_size(list); i++) {
		dumpMethod(kctx, sfp, list->MethodItems[i]);
	}
}

static KMETHOD NameSpace_man(KonohaContext *kctx, KonohaStack *sfp)
{
	INIT_GCSTACK();
	kArray *list = kctx->stack->gcstack_OnContextConstList;
	size_t start = kArray_size(list);
	kNameSpace *ns = sfp[0].asNameSpace;
	KonohaClass *ct = O_ct(sfp[1].asObject);
	DBG_P("*** man %s", TY_t(ct->typeId));
	while(ns != NULL) {
		copyMethodList(kctx, ct->typeId, ns->methodList_OnList, list);
		ns = ns->parentNULL;
	}
	copyMethodList(kctx, ct->typeId, ct->methodList_OnGlobalConstList, list);
	dumpMethodList(kctx, sfp, start, list);
	RESET_GCSTACK();
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _F(F)   (intptr_t)(F)

static kbool_t i_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	KImportPackage(ns, "konoha.global", trace);
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(NameSpace_man), TY_void, TY_NameSpace, MN_("man"), 1, TY_Object, FN_("x") | FN_COERCION,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t i_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* i_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha.i", "1.0");
	d.initPackage    = i_initPackage;
	d.setupPackage   = i_setupPackage;
	return &d;
}

#ifdef __cplusplus
}
#endif
