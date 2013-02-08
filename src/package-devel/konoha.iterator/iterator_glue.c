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

/* ************************************************************************ */

#define USE_STRINGLIB 1
#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/klib.h>
#include <konoha3/konoha_common.h>


#ifdef __cplusplus
extern "C" {
#endif

static kbool_t Nothing_hasNext(KonohaContext *kctx, KonohaStack* sfp)
{
	return false;
}

static void Nothing_SetNextResult(KonohaContext *kctx, KonohaStack* sfp)
{
	kIterator *itr = (kIterator *)sfp[0].asObject;
	KReturn(itr->source);
}

static void Nothing_SetNextResultUnbox(KonohaContext *kctx, KonohaStack* sfp)
{
	KReturnUnboxValue(0);
}

static void Iterator_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kIterator *itr = (kIterator *)o;
	int isUnboxEntry = KType_Is(UnboxType, kObject_class(itr)->p0);
	KFieldInit(itr, itr->source, K_NULL);
	itr->current_pos = 0;
	itr->hasNext = Nothing_hasNext;
	itr->setNextResult = isUnboxEntry ? Nothing_SetNextResultUnbox : Nothing_SetNextResult;
}

/* ------------------------------------------------------------------------ */

static KMETHOD Iterator_hasNext(KonohaContext *kctx, KonohaStack *sfp)
{
	kIterator *itr = sfp[0].asIterator;
	KReturnUnboxValue(itr->hasNext(kctx, sfp));
}

static KMETHOD Iterator_next(KonohaContext *kctx, KonohaStack *sfp)
{
	kIterator *itr = sfp[0].asIterator;
	itr->setNextResult(kctx, sfp);
}

//static kbool_t callFuncHasNext(KonohaContext *kctx, KonohaStack *sfp)
//{
//	itr->funcHasNext;
//}
//
//static kbool_t callFuncNext(KonohaContext *kctx, KonohaStack *sfp)
//{
//
//}
//
//static KMETHOD Iterator_new(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kIterator *itr = (kIterator *)sfp[0].asObject;
//	KFieldSet(itr, itr->funcHasNext, sfp[1].fo);
//	KFieldSet(itr, itr->funcNext,sfp[2].asFunc);
//	itr->hasNext = callFuncHasNext;
//	itr->setNextResult = callFuncNext;
//	KReturn(itr);
//}

static kbool_t Array_hasNext(KonohaContext *kctx, KonohaStack* sfp)
{
	kIterator *itr = (kIterator *)sfp[0].asObject;
	return (itr->current_pos < kArray_size(itr->arrayList));
}

static void Array_SetNextResult(KonohaContext *kctx, KonohaStack* sfp)
{
	kIterator *itr = (kIterator *)sfp[0].asObject;
	size_t n = itr->current_pos;
	itr->current_pos += 1;
	DBG_ASSERT(n < kArray_size(itr->arrayList));
	KReturn(itr->arrayList->ObjectItems[n]);
}

static void Array_SetNextResultUnbox(KonohaContext *kctx, KonohaStack* sfp)
{
	kIterator *itr = (kIterator *)sfp[0].asObject;
	size_t n = itr->current_pos;
	itr->current_pos += 1;
	DBG_ASSERT(n < kArray_size(itr->arrayList));
	KReturnUnboxValue(itr->arrayList->kintItems[n]);
}

static KMETHOD Array_toIterator(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	KClass *cIterator = KClass_p0(kctx, KClass_Iterator, kObject_class(a)->p0);
	kIterator *itr = (kIterator *)KLIB new_kObject(kctx, OnStack, cIterator, 0);
	KFieldSet(itr, itr->arrayList, a);
	itr->hasNext = Array_hasNext;
	itr->setNextResult = KType_Is(UnboxType, kObject_class(a)->p0) ? Array_SetNextResultUnbox : Array_SetNextResult;
	KReturn(itr);
}

static kbool_t String_hasNext(KonohaContext *kctx, KonohaStack* sfp)
{
	kIterator *itr = sfp[0].asIterator;
	kString *s = (kString *)itr->source;
	return (itr->current_pos < kString_size(s));
}

static void String_SetNextResult(KonohaContext *kctx, KonohaStack* sfp)
{
	kIterator *itr = sfp[0].asIterator;
	kString *s = (kString *)itr->source;
	const char *t = kString_text(s) + itr->current_pos;
	size_t charsize = utf8len(t[0]);
	itr->current_pos += charsize;
	KReturn(KLIB new_kString(kctx, OnStack, t, charsize, (charsize == 1) ? StringPolicy_ASCII : StringPolicy_UTF8));
}

static KMETHOD String_toIterator(KonohaContext *kctx, KonohaStack *sfp)
{
	kIterator *itr = (kIterator *)KLIB new_kObject(kctx, OnStack, KClass_StringIterator, 0);
	KFieldSet(itr, itr->source, sfp[0].asObject);
	itr->hasNext = String_hasNext;
	itr->setNextResult = String_SetNextResult;
	KReturn(itr);
}

/* ------------------------------------------------------------------------ */

#define _Public   kMethod_Public
#define _F(F)   (intptr_t)(F)

static kbool_t iterator_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequireKonohaCommonModule(trace);
	if(KClass_Iterator == NULL) {
		kparamtype_t IteratorParam = {KType_Object};
		KDEFINE_CLASS defIterator = {0};
		SETSTRUCTNAME(defIterator, Iterator);
		defIterator.cflag  = KClassFlag_Iterator;
		defIterator.init   = Iterator_Init;
		defIterator.cparamsize  = 1;
		defIterator.cParamItems = &IteratorParam;

		KClass_Iterator = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defIterator, trace);
		KClass_StringIterator = KClass_p0(kctx, KClass_Iterator, KType_String);
		KClass_GenericIterator = KClass_p0(kctx, KClass_Iterator, KType_0);
	}

	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Iterator_hasNext), KType_Boolean, KType_Iterator, KMethodName_("hasNext"), 0,
		_Public, _F(Iterator_next), KType_0, KType_Iterator, KMethodName_("next"), 0,
		_Public, _F(Array_toIterator),  KType_GenericIterator, KType_Array, KMethodName_("toIterator"), 0,
		_Public, _F(String_toIterator), KType_StringIterator, KType_String, KMethodName_("toIterator"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

static kbool_t iterator_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *iterator_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "iterator", "1.0");
	d.PackupNameSpace    = iterator_PackupNameSpace;
	d.ExportNameSpace   = iterator_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
