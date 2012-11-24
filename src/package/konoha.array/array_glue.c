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
#include <minikonoha/konoha_common.h>

#ifdef __cplusplus
extern "C"{
#endif

/* ------------------------------------------------------------------------ */

//## @Immutable method T0 Array.get(Int n);
static KMETHOD Array_get(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	size_t n = (size_t)sfp[1].intValue;
	KCheckIndex(n, kArray_size(a));
	if(kArray_isUnboxData(a)) {
		KReturnUnboxValue(a->unboxItems[n]);
	}
	else {
		KReturn(a->ObjectItems[n]);
	}
}

//## method void Array.set(Int n, T0 v);
static KMETHOD Array_set(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	size_t n = (size_t)sfp[1].intValue;
	KCheckIndex(n, kArray_size(a));
	if(kArray_isUnboxData(a)) {
		a->unboxItems[n] = sfp[2].unboxValue;
	}
	else {
		KFieldSet(a, a->ObjectItems[n], sfp[2].asObject);
	}
}

//## method int Array.getSize();
static KMETHOD Array_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	KReturnUnboxValue(kArray_size(a));
}

static KMETHOD Array_newArray(KonohaContext *kctx, KonohaStack *sfp)
{
	kArrayVar *a = (kArrayVar *)sfp[0].asObject;
	size_t asize = (size_t)sfp[1].intValue;
	a->bytemax = asize * sizeof(uintptr_t);
	kArray_SetSize(a, asize);
	a->ObjectItems = (kObject**)KCalloc_UNTRACE(a->bytemax, 1);
	if(!kArray_isUnboxData(a)) {
		size_t i;
		kObject *null = KLIB Knull(kctx, CT_(O_p0(a)));
		for(i = 0; i < asize; i++) {
			KFieldSet(a, a->ObjectItems[i], null);
		}
	}
	KReturn(a);
}

// Array
struct _kAbstractArray {
	KonohaObjectHeader h;
	KGrowingArray a;
};

static void UnboxArray_ensureMinimumSize(KonohaContext *kctx, struct _kAbstractArray *a, size_t min)
{
	size_t minbyte = min * sizeof(uintptr_t);
	if(!(minbyte < a->a.bytemax)) {
		if(minbyte < sizeof(kObject)) minbyte = sizeof(kObject);
		KLIB Karray_Expand(kctx, &a->a, minbyte);
	}
}

static void UnboxArray_Add(KonohaContext *kctx, kArray *o, uintptr_t value)
{
	size_t asize = kArray_size(o);
	struct _kAbstractArray *a = (struct _kAbstractArray *)o;
	UnboxArray_ensureMinimumSize(kctx, a, asize+1);
	DBG_ASSERT(a->a.ObjectItems[asize] == NULL);
	o->unboxItems[asize] = value;
	kArray_SetSize(o, (asize+1));
}

static void UnboxArray_insert(KonohaContext *kctx, kArray *o, size_t n, uintptr_t v)
{
	size_t asize = kArray_size(o);
	struct _kAbstractArray *a = (struct _kAbstractArray *)o;
	if(!(n < asize)) {
		UnboxArray_Add(kctx, o, v);
	}
	else {
		UnboxArray_ensureMinimumSize(kctx, a, asize+1);
		DBG_ASSERT(a->a.ObjectItems[asize] == NULL);
		memmove(o->unboxItems+(n+1), o->unboxItems+n, sizeof(uintptr_t) * (asize - n));
		o->unboxItems[n] = v;
		kArray_SetSize(o, (asize+1));
	}
}

static KMETHOD Array_clear(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	KLIB kArray_Clear(kctx, a, 0);
}

static KMETHOD Array_Add1(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	if(kArray_isUnboxData(a)) {
		UnboxArray_Add(kctx, a, sfp[1].unboxValue);
	} else {
		KLIB kArray_Add(kctx, a, sfp[1].asObject);
	}
}

static KMETHOD Array_Push(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	Array_Add1(kctx, sfp);
	KReturnUnboxValue(kArray_size(a));
}

static KMETHOD Array_unshift(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	if(kArray_isUnboxData(a)) {
		UnboxArray_insert(kctx, a, 0, sfp[1].unboxValue);
	} else {
		KLIB kArray_Insert(kctx, a, 0, sfp[1].asObject);
	}
	KReturnUnboxValue(kArray_size(a));
}

static KMETHOD Array_Pop(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	if(kArray_size(a) == 0)
		KReturnDefaultValue();
	size_t n = kArray_size(a) - 1;
	if(kArray_isUnboxData(a)) {
		uintptr_t v = a->unboxItems[n];
		a->unboxItems[n] = 0;
		kArray_SetSize(a, n);
		KReturnUnboxValue(v);
	}
	else {
		struct _kAbstractArray *a2 = (struct _kAbstractArray *)a;
		kObject *value = a2->a.ObjectItems[n];
		kObject** null = NULL;
		KFieldInit(a2, a2->a.ObjectItems[n], null);
		a2->a.bytesize = n * sizeof(uintptr_t);
		KReturn(value);
	}
}

static void kArray_removeAt(KonohaContext *kctx, kArray *a, size_t n)
{
	size_t asize = kArray_size(a);
	if(kArray_isUnboxData(a)) {
		kArrayVar *a2 = (kArrayVar *)a;
		memmove(a2->unboxItems+n, a2->unboxItems+(n+1), sizeof(uintptr_t) * (asize - n - 1));
		a2->unboxItems[asize-1] = 0;
		kArray_SetSize(a2, asize - 1);
	}
	else {
		struct _kAbstractArray *a2 = (struct _kAbstractArray *)a;
		memmove(a2->a.ObjectItems+n, a2->a.ObjectItems+(n+1), sizeof(kObject *) * (asize - n - 1));
		a2->a.ObjectItems[asize-1] = NULL;
		a2->a.bytesize = (asize - 1) * sizeof(uintptr_t);
	}
}

static KMETHOD Array_removeAt(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	int n = (int)sfp[1].unboxValue;
	struct _kAbstractArray *a2 = (struct _kAbstractArray *)a;
	if(kArray_isUnboxData(a)) {
		uintptr_t v = a->unboxItems[n];
		kArray_removeAt(kctx, a, n);
		KReturnUnboxValue(v);
	}
	else {
		kObject *value = a2->a.ObjectItems[n];
		kArray_removeAt(kctx, a, n);
		KReturn(value);
	}
}

static KMETHOD Array_reverse(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	size_t asize = kArray_size(a);
	size_t asize_half = asize / 2;
	size_t i;
	if(kArray_isUnboxData(a)) {
		for(i = 0; i != asize_half; ++i) {
			uintptr_t temp = a->unboxItems[asize - 1 - i];
			a->unboxItems[asize - 1 - i] = a->unboxItems[i];
			a->unboxItems[i] = temp;
		}
	}
	else {
		for(i = 0; i != asize_half; ++i){
			kObject *temp = a->ObjectItems[asize - 1 - i];
			KFieldSet(a, a->ObjectItems[asize - 1 - i], a->ObjectItems[i]);
			KFieldSet(a, a->ObjectItems[i], temp);
		}
	}
	KReturn(a);
}

///* Array[T] Array[T].map(Func[T,T] func) */
//static KMETHOD Array_map(KonohaContext *kctx, KonohaStack *sfp)
//{
//	INIT_GCSTACK();
//	kArray *a = sfp[0].asArray;
//	kFunc  *f = sfp[1].asFunc;
//	size_t asize = kArray_size(a);
//	kattrtype_t resolve_type = kMethod_GetReturnType(f->mtd);  // FIXME
//	KonohaClass *CT_ArrayT0 = CT_p0(kctx, CT_Array, resolve_type);
//	kArrayVar *returnValue = (kArrayVar *)KLIB new_kObject(kctx, _GcStack, CT_ArrayT0, asize);
//
//	size_t i;
//	if(kArray_isUnboxData(a)) {
//		for(i=0; i != asize; ++i) {
//			uintptr_t tmp = a->unboxItems[i];
//			BEGIN_LOCAL(lsfp, K_CALLDELTA + 1);
//			KUnsafeFieldSet(lsfp[K_CALLDELTA+0].asObject, K_NULL);
//			lsfp[K_CALLDELTA+1].unboxValue = tmp;
//			{
//				KonohaStack *sfp = lsfp + K_CALLDELTA;
//				KSetMethodCallStack(sfp, 0/*UL*/, f->mtd, 1, KLIB Knull(kctx, CT_(resolve_type)));
//				KonohaRuntime_callMethod(kctx, sfp);
//			}
//			END_LOCAL();
//			returnValue->unboxItems[i] = lsfp[0].unboxValue;
//		}
//	}
//	else {
//		for(i=0; i != asize; ++i) {
//			kObject *tmp  = a->ObjectItems[i];
//			BEGIN_LOCAL(lsfp, K_CALLDELTA + 1);
//			KUnsafeFieldSet(lsfp[K_CALLDELTA+0].asObject, K_NULL);
//			KUnsafeFieldSet(lsfp[K_CALLDELTA+1].asObject, tmp);
//			{
//				KonohaStack *sfp = lsfp + K_CALLDELTA;
//				KSetMethodCallStack(sfp, 0/*UL*/, f->mtd, 1, KLIB Knull(kctx, CT_(resolve_type)));
//				KonohaRuntime_callMethod(kctx, sfp);
//			}
//			END_LOCAL();
//			KFieldSet(returnValue, returnValue->ObjectItems[i], lsfp[0].asObject);
//		}
//	}
//	kArray_SetSize(returnValue, asize);
//	KReturnWith(returnValue, RESET_GCSTACK());
//}
//
///* T Array[T].inject(Func[T,T,T] func) */
//static KMETHOD Array_inject(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kArray *a = sfp[0].asArray;
//	kFunc  *f = sfp[1].asFunc;
//	size_t asize = kArray_size(a);
//	kattrtype_t resolve_type = kMethod_GetReturnType(f->mtd);
//
//	size_t i;
//	if(kArray_isUnboxData(a)) {
//		uintptr_t tmp = 0;
//		BEGIN_LOCAL(lsfp, K_CALLDELTA + 2);
//		for(i=0; i != asize; ++i) {
//			KUnsafeFieldSet(lsfp[K_CALLDELTA+0].asObject, K_NULL);
//			lsfp[K_CALLDELTA+1].unboxValue = tmp;
//			lsfp[K_CALLDELTA+2].unboxValue = a->unboxItems[i];
//			{
//				KonohaStack *sfp = lsfp + K_CALLDELTA;
//				KSetMethodCallStack(sfp, 0/*UL*/, f->mtd, 2, KLIB Knull(kctx, CT_(resolve_type)));
//				KonohaRuntime_callMethod(kctx, sfp);
//			}
//			tmp = lsfp[0].unboxValue;
//		}
//		END_LOCAL();
//		KReturnUnboxValue(tmp);
//	}
//	else {
//		INIT_GCSTACK();
//		kObject *tmp = (kObject *) KLIB new_kObject(kctx, _GcStack, CT_(resolve_type), 0);
//		kObject *nulobj = KLIB Knull(kctx, CT_(resolve_type));
//		BEGIN_LOCAL(lsfp, K_CALLDELTA + 2);
//		for(i=0; i != asize; ++i) {
//			KUnsafeFieldSet(lsfp[K_CALLDELTA+0].asObject, nulobj);
//			KUnsafeFieldSet(lsfp[K_CALLDELTA+1].asObject, tmp);
//			KUnsafeFieldSet(lsfp[K_CALLDELTA+2].asObject, a->ObjectItems[i]);
//			{
//				KonohaStack *sfp = lsfp + K_CALLDELTA;
//				KSetMethodCallStack(sfp, 0/*UL*/, f->mtd, 2, nulobj);
//				KonohaRuntime_callMethod(kctx, sfp);
//			}
//			KUnsafeFieldSet(tmp, lsfp[0].asObject);
//		}
//		END_LOCAL();
//		KReturnWith(tmp, RESET_GCSTACK());
//	}
//}

static KMETHOD Array_shift(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	struct _kAbstractArray *a2 = (struct _kAbstractArray *)a;
	if(kArray_isUnboxData(a)) {
		uintptr_t v = a->unboxItems[0];
		kArray_removeAt(kctx, a, 0);
		KReturnUnboxValue(v);
	}
	else {
		kObject *value = a2->a.ObjectItems[0];
		kArray_removeAt(kctx, a, 0);
		KReturn(value);
	}
}

//## method Array<T> Array.concat(Array<T> a1);
static KMETHOD Array_concat(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a0 = sfp[0].asArray;
	kArray *a1 = sfp[1].asArray;
	size_t i;
	if(kArray_isUnboxData(a1)) {
		for (i = 0; i < kArray_size(a1); i++){
			UnboxArray_Add(kctx, a0, a1->unboxItems[i]);
		}
	} else {
		for (i = 0; i < kArray_size(a1); i++){
			KLIB kArray_Add(kctx, a0, a1->ObjectItems[i]);
		}
	}
	KReturn(a0);
}

//## method int Array.indexOf(T0 a1);
static KMETHOD Array_indexOf(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	kint_t res = -1;
	size_t i;
	if(kArray_isUnboxData(a)) {
		uintptr_t nv = sfp[1].unboxValue;
		for(i = 0; i < kArray_size(a); i++) {
			if(a->unboxItems[i] == nv) {
				res = i; break;
			}
		}
	} else {
		//TODO:Need to implement Object compareTo.
		kObject *o = sfp[1].asObject;
		for(i = 0; i < kArray_size(a); i++) {
//			KMakeTrace(trace, sfp);
//			KLIB KonohaRuntime_raise(kctx, EXPT_("NotImplemented"), NULL, trace);
			if(O_ct(o)->compareObject(a->ObjectItems[i], o) == 0) {
				res = i; break;
			}
		}
	}
	KReturnUnboxValue(res);
}

//## method int Array.lastIndexOf(T0 a1);
static KMETHOD Array_lastIndexOf(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	kint_t res = -1;
	size_t i = 0;
	if(kArray_isUnboxData(a)) {
		uintptr_t nv = sfp[1].unboxValue;
		for(i = kArray_size(a)- 1; i != 0; i--) {
			if(a->unboxItems[i] == nv) {
				break;
			}
		}
	} else {
		//TODO: Need to implement Object compareTo;
		kObject *o = sfp[1].asObject;
		for(i = kArray_size(a)- 1; i != 0; i--) {
//			KMakeTrace(trace, sfp);
//			KLIB KonohaRuntime_raise(kctx, EXPT_("NotImplemented"), NULL, trace);
			if(O_ct(o)->compareObject(a->ObjectItems[i], o) == 0) {
				break;
			}
		}
	}
	res = i;
	KReturnUnboxValue(res);
}

//## method String Array.toString();
//static KMETHOD Array_toString(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kArray *a = sfp[0].asArray;
//	size_t i = 0;
//	KGrowingBuffer wb;
//	KLIB Kwb_Init(&(kctx->stack->cwb), &wb);
//	if(kArray_size(a) < 1) {
//		KReturn(KNULL(String));
//	}
//	if(kArray_isUnboxData(a)) {
//		uintptr_t uv = a->unboxItems[i];
//		for (i = 0; i < kArray_size(a) - 1; i++) {
//			uv = a->unboxItems[i];
//			KLIB Kwb_printf(kctx, &wb, "%ld,", uv);
//		}
//		uv = a->unboxItems[i];
//		KLIB Kwb_printf(kctx, &wb, "%ld", uv);
//	} else {
//		kObject *obj;
//		BEGIN_LOCAL(lsfp, 1);
//		for (i = 0; i < kArray_size(a) - 1; i++) {
//			obj = a->ObjectItems[i];
//			DBG_ASSERT(O_ct(obj)->p);
//			// before call system.p, push variables
//			KUnsafeFieldSet(lsfp[0].asObject, obj);
//			O_ct(obj)->p(kctx, lsfp, 0, &wb, 0);
//			KLIB Kwb_printf(kctx, &wb, ",");
//		}
//		obj = a->ObjectItems[i];
//		KUnsafeFieldSet(lsfp[0].asObject, obj);
//		O_ct(obj)->p(kctx, lsfp, 0, &wb, 0);
//		END_LOCAL();
//
//	}
//	const char *KGrowingBufferTopChar = KLIB Kwb_top(kctx, &wb, 0);
//	size_t strsize = strlen(KGrowingBufferTopChar);
//	kString *ret = KLIB new_kString(kctx, KGrowingBufferTopChar, strsize, 0);
//	KLIB Kwb_Free(&wb);
//	KReturn(ret);
//}

static KMETHOD Array_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kArrayVar *a = (kArrayVar *)sfp[0].asObject;
	size_t asize = (size_t)sfp[1].intValue;
	a->bytemax = asize * sizeof(uintptr_t);
	kArray_SetSize(a, 0);
	a->ObjectItems = (kObject**)KCalloc_UNTRACE(a->bytemax, 1);
	if(!kArray_isUnboxData(a)) {
		size_t i;
		kObject *null = KLIB Knull(kctx, CT_(O_p0(a)));
		for(i = 0; i < asize; i++) {
			KFieldSet(a, a->ObjectItems[i], null);
		}
	}
	KReturn(a);
}

static KMETHOD Array_newList(KonohaContext *kctx, KonohaStack *sfp)
{
	kArrayVar *a = (kArrayVar *)sfp[0].asObject;
	size_t i = 0;
	KonohaStack *p = sfp+1;
	if(kArray_isUnboxData(a)) {
		for(i = 0; p + i < kctx->esp; i++) {
			a->unboxItems[i] = p[i].unboxValue;
		}
	}
	else {
		for(i = 0; p + i < kctx->esp; i++) {
			KFieldSet(a, a->ObjectItems[i], p[i].asObject);
		}
	}
	kArray_SetSize(a, i);
	DBG_ASSERT(a->bytesize <= a->bytemax);
	KReturn(a);
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Im       kMethod_Immutable
#define _F(F)     (intptr_t)(F)

static kbool_t array_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KRequireKonohaCommonModule(trace);
	KImportPackageSymbol(ns, "cstyle", "[]", trace);
	KDEFINE_INT_CONST ClassData[] = {   // add Array as available
		{"Array", VirtualType_KonohaClass, (uintptr_t)CT_(TY_Array)},
		{NULL},
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KonohaConst_(ClassData), false/*isOverride*/, trace);

	KonohaClass *CT_ArrayT0 = CT_p0(kctx, CT_Array, TY_0);
	kattrtype_t TY_ArrayT0 = CT_ArrayT0->typeId;

	kparamtype_t p[] = {{TY_0}};
	kattrtype_t TY_FuncMap = (KLIB KonohaClass_Generics(kctx, CT_Func, TY_0 , 1, p))->typeId;

	kparamtype_t P_inject[] = {{TY_0},{TY_0}};
	kattrtype_t TY_FuncInject = (KLIB KonohaClass_Generics(kctx, CT_Func, TY_0 , 2, P_inject))->typeId;

	KDEFINE_METHOD MethodData[] = {
		_Public|_Im,    _F(Array_get), TY_0,   TY_Array, MN_("get"), 1, TY_int, FN_("index"),
		_Public,        _F(Array_set), TY_void, TY_Array, MN_("set"), 2, TY_int, FN_("index"),  TY_0, FN_("value"),
		_Public|_Im,    _F(Array_removeAt), TY_0,   TY_Array, MN_("removeAt"), 1, TY_int, FN_("index"),
		_Public|_Const, _F(Array_getSize), TY_int, TY_Array, MN_("getSize"), 0,
		_Public|_Const, _F(Array_getSize), TY_int, TY_Array, MN_("getlength"), 0,
		_Public,        _F(Array_clear), TY_void, TY_Array, MN_("clear"), 0,
		_Public,        _F(Array_Add1), TY_void, TY_Array, MN_("add"), 1, TY_0, FN_("value"),
		_Public,        _F(Array_Push), TY_int, TY_Array, MN_("push"), 1, TY_0, FN_("value"),
		_Public,        _F(Array_Pop), TY_0, TY_Array, MN_("pop"), 0,
		_Public,        _F(Array_shift), TY_0, TY_Array, MN_("shift"), 0,
		_Public,        _F(Array_unshift), TY_int, TY_Array, MN_("unshift"), 1, TY_0, FN_("value"),
		_Public,        _F(Array_reverse), TY_Array, TY_Array, MN_("reverse"), 0,
//		_Public|_Im,    _F(Array_map), TY_ArrayT0, TY_Array, MN_("map"), 1, TY_FuncMap, FN_("func"),
//		_Public|_Im,    _F(Array_inject), TY_0, TY_Array, MN_("inject"), 1, TY_FuncInject, FN_("func"),

		_Public,        _F(Array_concat), TY_ArrayT0, TY_Array, MN_("concat"), 1, TY_ArrayT0, FN_("a1"),
		_Public,        _F(Array_indexOf), TY_int, TY_Array, MN_("indexOf"), 1, TY_0, FN_("value"),
		_Public,        _F(Array_lastIndexOf), TY_int, TY_Array, MN_("lastIndexOf"), 1, TY_0, FN_("value"),
//		_Public,        _F(Array_toString), TY_String, TY_Array, MN_("toString"), 0,
		_Public|_Im,    _F(Array_new), TY_void, TY_Array, MN_("new"), 1, TY_int, FN_("size"),
		_Public,        _F(Array_newArray), TY_Array, TY_Array, MN_("newArray"), 1, TY_int, FN_("size"),
		_Public|kMethod_Hidden, _F(Array_newList), TY_Array, TY_Array, MN_("[]"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

// ---------------------------------------------------------------------------
/* Syntax */

static KMETHOD TypeCheck_Bracket(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	// [0] currentToken, [1] NULL, [2] ....
	size_t i;
	KonohaClass *requestClass = CT_(reqty);
	KonohaClass *paramType = (requestClass->baseTypeId == TY_Array) ? CT_(requestClass->p0) : CT_INFER;
	for(i = 2; i < kArray_size(expr->cons); i++) {
		kExpr *typedExpr = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, i, gma, paramType, 0);
		if(typedExpr == K_NULLEXPR) {
			KReturn(typedExpr);
		}
		if(paramType->typeId == TY_var) {
			paramType = CT_(typedExpr->attrTypeId);
		}
	}
	if(requestClass->baseTypeId != TY_Array) {
		requestClass = (paramType->typeId == TY_var) ? CT_Array : CT_p0(kctx, CT_Array, paramType->typeId);
	}
	kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, Stmt_ns(stmt), CT_Array, MN_("[]"), -1, MethodMatch_NoOption);
	DBG_ASSERT(mtd != NULL);
	KFieldSet(expr, expr->cons->MethodItems[0], mtd);
	KFieldSet(expr, expr->cons->ExprItems[1], SUGAR kExpr_SetVariable(kctx, NULL, gma, TEXPR_NEW, requestClass->typeId, kArray_size(expr->cons) - 2));
	KReturn(Expr_typed(expr, TEXPR_CALL, requestClass->typeId));
}

static KMETHOD Expression_Bracket(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	KonohaClass *genericsClass = NULL;
	kNameSpace *ns = Stmt_ns(stmt);
	int nextIdx = SUGAR TokenUtils_ParseTypePattern(kctx, ns, tokenList, beginIdx, endIdx, &genericsClass);
	if(nextIdx != -1) {  // to avoid Func[T]
		KReturn(SUGAR kStmt_ParseOperatorExpr(kctx, stmt, tokenList->TokenItems[beginIdx]->resolvedSyntaxInfo, tokenList, beginIdx, beginIdx, endIdx));
	}
	kToken *currentToken = tokenList->TokenItems[operatorIdx];
	if(beginIdx == operatorIdx) {
		/* transform '[ Value1, Value2, ... ]' to '(Call Untyped new (Value1, Value2, ...))' */
		DBG_ASSERT(currentToken->resolvedSyntaxInfo->keyword == KW_BracketGroup);
		kExpr *arrayExpr = SUGAR new_UntypedCallStyleExpr(kctx, currentToken->resolvedSyntaxInfo, 2, currentToken, K_NULL);
		KReturn(SUGAR kStmt_AddExprParam(kctx, stmt, arrayExpr, currentToken->subTokenList, 0, kArray_size(currentToken->subTokenList), NULL));
	}
	else {
		kExpr *leftExpr = SUGAR kStmt_ParseExpr(kctx, stmt, tokenList, beginIdx, operatorIdx, NULL);
		if(leftExpr == K_NULLEXPR) {
			KReturn(leftExpr);
		}
		if(leftExpr->syn->keyword == SYM_("new")) {  // new int[100], new int[]();
			DBG_P("cur:%d, beg:%d, endIdx:%d", operatorIdx, beginIdx, endIdx);
			size_t subTokenSize = kArray_size(currentToken->subTokenList);
			if(subTokenSize == 0) {
				/* transform 'new Type0 [ ]' => (Call Type0 new) */
				kExpr_Setsyn(leftExpr, SYN_(ns, KW_ExprMethodCall));
			} else {
				/* transform 'new Type0 [ Type1 ] (...) => new 'Type0<Type1>' (...) */
				KonohaClass *classT0 = NULL;
				kArray *subTokenList = currentToken->subTokenList;
				int beginIdx = -1;
				if(kArray_size(subTokenList) > 0) {
					beginIdx = SUGAR TokenUtils_ParseTypePattern(kctx, ns, subTokenList, 0, kArray_size(subTokenList), &classT0);
				}
				beginIdx = (beginIdx == -1) ? 0 : beginIdx;
				kExpr_Setsyn(leftExpr, SYN_(ns, KW_ExprMethodCall));
				DBG_P("currentToken->subtoken:%d", kArray_size(subTokenList));
				leftExpr = SUGAR kStmt_AddExprParam(kctx, stmt, leftExpr, subTokenList, beginIdx, kArray_size(subTokenList), "[");
			}
		}
		else {
			/* transform 'Value0 [ Value1 ]=> (Call Value0 get (Value1)) */
			kTokenVar *tkN = /*G*/new_(TokenVar, 0, OnGcStack);
			tkN->resolvedSymbol= MN_toGETTER(0);
			tkN->uline = currentToken->uline;
			SugarSyntax *syn = SYN_(Stmt_ns(stmt), KW_ExprMethodCall);
			leftExpr  = SUGAR new_UntypedCallStyleExpr(kctx, syn, 2, tkN, leftExpr);
			leftExpr = SUGAR kStmt_AddExprParam(kctx, stmt, leftExpr, currentToken->subTokenList, 0, kArray_size(currentToken->subTokenList), "[");
		}
		KReturn(SUGAR kStmt_RightJoinExpr(kctx, stmt, leftExpr, tokenList, operatorIdx + 1, endIdx));
	}
}

static kbool_t array_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KW_BracketGroup, SYNFLAG_ExprPostfixOp2, NULL, Precedence_CStyleCALL, 0, NULL, Expression_Bracket, NULL, NULL, TypeCheck_Bracket, },
		{ KW_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}

static kbool_t array_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	array_defineMethod(kctx, ns, trace);
	array_defineSyntax(kctx, ns, trace);
	return true;
}

static kbool_t array_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* array_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", K_VERSION);
	d.PackupNameSpace    = array_PackupNameSpace;
	d.ExportNameSpace   = array_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif