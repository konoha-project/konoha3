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
#include <minikonoha/klib.h>
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
	if(kArray_Is(UnboxData, a)) {
		KReturnUnboxValue(a->unboxItems[n]);
	}
	else {
		KReturn(a->ObjectItems[n]);
	}
}

//## method void Array.set(Int n, T0 v);
static KMETHOD Array_Set(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	size_t n = (size_t)sfp[1].intValue;
	KCheckIndex(n, kArray_size(a));
	if(kArray_Is(UnboxData, a)) {
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
	if(!kArray_Is(UnboxData, a)) {
		size_t i;
		kObject *null = KLIB Knull(kctx, KClass_(kObject_p0(a)));
		for(i = 0; i < asize; i++) {
			KFieldSet(a, a->ObjectItems[i], null);
		}
	}
	KReturn(a);
}

// Array
struct _kAbstractArray {
	kObjectHeader h;
	KGrowingArray a;
};

static void UnboxArray_ensureMinimumSize(KonohaContext *kctx, struct _kAbstractArray *a, size_t min)
{
	size_t minbyte = min * sizeof(uintptr_t);
	if(!(minbyte < a->a.bytemax)) {
		if(minbyte < sizeof(kObject)) minbyte = sizeof(kObject);
		KLIB KArray_Expand(kctx, &a->a, minbyte);
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
	if(kArray_Is(UnboxData, a)) {
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
	if(kArray_Is(UnboxData, a)) {
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
	if(kArray_Is(UnboxData, a)) {
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

static void kArray_RemoveAt(KonohaContext *kctx, kArray *a, size_t n)
{
	size_t asize = kArray_size(a);
	if(kArray_Is(UnboxData, a)) {
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

static KMETHOD Array_RemoveAt(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	int n = (int)sfp[1].unboxValue;
	struct _kAbstractArray *a2 = (struct _kAbstractArray *)a;
	if(kArray_Is(UnboxData, a)) {
		uintptr_t v = a->unboxItems[n];
		kArray_RemoveAt(kctx, a, n);
		KReturnUnboxValue(v);
	}
	else {
		kObject *value = a2->a.ObjectItems[n];
		kArray_RemoveAt(kctx, a, n);
		KReturn(value);
	}
}

static KMETHOD Array_reverse(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	size_t asize = kArray_size(a);
	size_t asize_half = asize / 2;
	size_t i;
	if(kArray_Is(UnboxData, a)) {
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
//	ktypeattr_t resolve_type = kMethod_GetReturnType(f->mtd);  // FIXME
//	KClass *KClass_ArrayT0 = KClass_p0(kctx, KClass_Array, resolve_type);
//	kArrayVar *returnValue = (kArrayVar *)KLIB new_kObject(kctx, _GcStack, KClass_ArrayT0, asize);
//
//	size_t i;
//	if(kArray_Is(UnboxData, a)) {
//		for(i=0; i != asize; ++i) {
//			uintptr_t tmp = a->unboxItems[i];
//			BEGIN_UnusedStack(lsfp, K_CALLDELTA + 1);
//			KUnsafeFieldSet(lsfp[K_CALLDELTA+0].asObject, K_NULL);
//			lsfp[K_CALLDELTA+1].unboxValue = tmp;
//			{
//				KonohaStack *sfp = lsfp + K_CALLDELTA;
//				KStackSetMethodAll(sfp, 0/*UL*/, f->mtd, 1, KLIB Knull(kctx, KClass_(resolve_type)));
//				KStackCall(sfp);
//			}
//			END_UnusedStack();
//			returnValue->unboxItems[i] = lsfp[0].unboxValue;
//		}
//	}
//	else {
//		for(i=0; i != asize; ++i) {
//			kObject *tmp  = a->ObjectItems[i];
//			BEGIN_UnusedStack(lsfp, K_CALLDELTA + 1);
//			KUnsafeFieldSet(lsfp[K_CALLDELTA+0].asObject, K_NULL);
//			KUnsafeFieldSet(lsfp[K_CALLDELTA+1].asObject, tmp);
//			{
//				KonohaStack *sfp = lsfp + K_CALLDELTA;
//				KStackSetMethodAll(sfp, 0/*UL*/, f->mtd, 1, KLIB Knull(kctx, KClass_(resolve_type)));
//				KStackCall(sfp);
//			}
//			END_UnusedStack();
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
//	ktypeattr_t resolve_type = kMethod_GetReturnType(f->mtd);
//
//	size_t i;
//	if(kArray_Is(UnboxData, a)) {
//		uintptr_t tmp = 0;
//		BEGIN_UnusedStack(lsfp, K_CALLDELTA + 2);
//		for(i=0; i != asize; ++i) {
//			KUnsafeFieldSet(lsfp[K_CALLDELTA+0].asObject, K_NULL);
//			lsfp[K_CALLDELTA+1].unboxValue = tmp;
//			lsfp[K_CALLDELTA+2].unboxValue = a->unboxItems[i];
//			{
//				KonohaStack *sfp = lsfp + K_CALLDELTA;
//				KStackSetMethodAll(sfp, 0/*UL*/, f->mtd, 2, KLIB Knull(kctx, KClass_(resolve_type)));
//				KStackCall(sfp);
//			}
//			tmp = lsfp[0].unboxValue;
//		}
//		END_UnusedStack();
//		KReturnUnboxValue(tmp);
//	}
//	else {
//		INIT_GCSTACK();
//		kObject *tmp = (kObject *) KLIB new_kObject(kctx, _GcStack, KClass_(resolve_type), 0);
//		kObject *nulobj = KLIB Knull(kctx, KClass_(resolve_type));
//		BEGIN_UnusedStack(lsfp, K_CALLDELTA + 2);
//		for(i=0; i != asize; ++i) {
//			KUnsafeFieldSet(lsfp[K_CALLDELTA+0].asObject, nulobj);
//			KUnsafeFieldSet(lsfp[K_CALLDELTA+1].asObject, tmp);
//			KUnsafeFieldSet(lsfp[K_CALLDELTA+2].asObject, a->ObjectItems[i]);
//			{
//				KonohaStack *sfp = lsfp + K_CALLDELTA;
//				KStackSetMethodAll(sfp, 0/*UL*/, f->mtd, 2, nulobj);
//				KStackCall(sfp);
//			}
//			KUnsafeFieldSet(tmp, lsfp[0].asObject);
//		}
//		END_UnusedStack();
//		KReturnWith(tmp, RESET_GCSTACK());
//	}
//}

static KMETHOD Array_shift(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	struct _kAbstractArray *a2 = (struct _kAbstractArray *)a;
	if(kArray_Is(UnboxData, a)) {
		uintptr_t v = a->unboxItems[0];
		kArray_RemoveAt(kctx, a, 0);
		KReturnUnboxValue(v);
	}
	else {
		kObject *value = a2->a.ObjectItems[0];
		kArray_RemoveAt(kctx, a, 0);
		KReturn(value);
	}
}

//## method Array<T> Array.concat(Array<T> a1);
static KMETHOD Array_concat(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a0 = sfp[0].asArray;
	kArray *a1 = sfp[1].asArray;
	size_t i;
	if(kArray_Is(UnboxData, a1)) {
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
	if(kArray_Is(UnboxData, a)) {
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
//			KLIB KRuntime_raise(kctx, KException_("NotImplemented"), NULL, trace);
			if(kObject_class(o)->compareObject(a->ObjectItems[i], o) == 0) {
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
	if(kArray_Is(UnboxData, a)) {
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
//			KLIB KRuntime_raise(kctx, KException_("NotImplemented"), NULL, trace);
			if(kObject_class(o)->compareObject(a->ObjectItems[i], o) == 0) {
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
//	KBuffer wb;
//	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
//	if(kArray_size(a) < 1) {
//		KReturn(KNULL(String));
//	}
//	if(kArray_Is(UnboxData, a)) {
//		uintptr_t uv = a->unboxItems[i];
//		for (i = 0; i < kArray_size(a) - 1; i++) {
//			uv = a->unboxItems[i];
//			KLIB KBuffer_printf(kctx, &wb, "%ld,", uv);
//		}
//		uv = a->unboxItems[i];
//		KLIB KBuffer_printf(kctx, &wb, "%ld", uv);
//	} else {
//		kObject *obj;
//		BEGIN_UnusedStack(lsfp, 1);
//		for (i = 0; i < kArray_size(a) - 1; i++) {
//			obj = a->ObjectItems[i];
//			DBG_ASSERT(kObject_class(obj)->p);
//			// before call system.p, push variables
//			KUnsafeFieldSet(lsfp[0].asObject, obj);
//			kObject_class(obj)->p(kctx, lsfp, 0, &wb, 0);
//			KLIB KBuffer_printf(kctx, &wb, ",");
//		}
//		obj = a->ObjectItems[i];
//		KUnsafeFieldSet(lsfp[0].asObject, obj);
//		kObject_class(obj)->p(kctx, lsfp, 0, &wb, 0);
//		END_UnusedStack();
//
//	}
//	const char *KBufferTopChar = KLIB KBuffer_text(kctx, &wb, 0);
//	size_t strsize = strlen(KBufferTopChar);
//	kString *ret = KLIB new_kString(kctx, KBufferTopChar, strsize, 0);
//	KLIB KBuffer_Free(&wb);
//	KReturn(ret);
//}

static KMETHOD Array_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kArrayVar *a = (kArrayVar *)sfp[0].asObject;
	size_t asize = (size_t)sfp[1].intValue;
	a->bytemax = asize * sizeof(uintptr_t);
	kArray_SetSize(a, 0);
	a->ObjectItems = (kObject**)KCalloc_UNTRACE(a->bytemax, 1);
	if(!kArray_Is(UnboxData, a)) {
		size_t i;
		kObject *null = KLIB Knull(kctx, KClass_(kObject_p0(a)));
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
	if(kArray_Is(UnboxData, a)) {
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
	//KImportPackageSymbol(ns, "cstyle", "[]", trace);
	KDEFINE_INT_CONST ClassData[] = {   // add Array as available
		{"Array", VirtualType_KClass, (uintptr_t)KClass_(KType_Array)},
		{NULL},
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(ClassData), trace);

	KClass *KClass_ArrayT0 = KClass_p0(kctx, KClass_Array, KType_0);
	ktypeattr_t KType_ArrayT0 = KClass_ArrayT0->typeId;

//	kparamtype_t p[] = {{KType_0}};
//	ktypeattr_t KType_FuncMap = (KLIB KClass_Generics(kctx, KClass_Func, KType_0 , 1, p))->typeId;
//
//	kparamtype_t P_inject[] = {{KType_0},{KType_0}};
//	ktypeattr_t KType_FuncInject = (KLIB KClass_Generics(kctx, KClass_Func, KType_0 , 2, P_inject))->typeId;

	KDEFINE_METHOD MethodData[] = {
		_Public|_Im,    _F(Array_get), KType_0,   KType_Array, KMethodName_("get"), 1, KType_int, KFieldName_("index"),
		_Public,        _F(Array_Set), KType_void, KType_Array, KMethodName_("set"), 2, KType_int, KFieldName_("index"),  KType_0, KFieldName_("value"),
		_Public|_Im,    _F(Array_RemoveAt), KType_0,   KType_Array, KMethodName_("removeAt"), 1, KType_int, KFieldName_("index"),
		_Public|_Const, _F(Array_getSize), KType_int, KType_Array, KMethodName_("getSize"), 0,
		_Public|_Const, _F(Array_getSize), KType_int, KType_Array, KMethodName_("getlength"), 0,
		_Public,        _F(Array_clear), KType_void, KType_Array, KMethodName_("clear"), 0,
		_Public,        _F(Array_Add1), KType_void, KType_Array, KMethodName_("add"), 1, KType_0, KFieldName_("value"),
		_Public,        _F(Array_Push), KType_int, KType_Array, KMethodName_("push"), 1, KType_0, KFieldName_("value"),
		_Public,        _F(Array_Pop), KType_0, KType_Array, KMethodName_("pop"), 0,
		_Public,        _F(Array_shift), KType_0, KType_Array, KMethodName_("shift"), 0,
		_Public,        _F(Array_unshift), KType_int, KType_Array, KMethodName_("unshift"), 1, KType_0, KFieldName_("value"),
		_Public,        _F(Array_reverse), KType_Array, KType_Array, KMethodName_("reverse"), 0,
//		_Public|_Im,    _F(Array_map), KType_ArrayT0, KType_Array, KMethodName_("map"), 1, KType_FuncMap, KFieldName_("func"),
//		_Public|_Im,    _F(Array_inject), KType_0, KType_Array, KMethodName_("inject"), 1, KType_FuncInject, KFieldName_("func"),

		_Public,        _F(Array_concat), KType_ArrayT0, KType_Array, KMethodName_("concat"), 1, KType_ArrayT0, KFieldName_("a1"),
		_Public,        _F(Array_indexOf), KType_int, KType_Array, KMethodName_("indexOf"), 1, KType_0, KFieldName_("value"),
		_Public,        _F(Array_lastIndexOf), KType_int, KType_Array, KMethodName_("lastIndexOf"), 1, KType_0, KFieldName_("value"),
//		_Public,        _F(Array_toString), KType_String, KType_Array, KMethodName_("toString"), 0,
		_Public|_Im,    _F(Array_new), KType_void, KType_Array, KMethodName_("new"), 1, KType_int, KFieldName_("size"),
		_Public,        _F(Array_newArray), KType_Array, KType_Array, KMethodName_("newArray"), 1, KType_int, KFieldName_("size"),
		_Public|kMethod_Hidden, _F(Array_newList), KType_Array, KType_Array, KMethodName_("[]"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

// ---------------------------------------------------------------------------
/* Syntax */

//static KMETHOD TypeCheck_Bracket(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_TypeCheck2(stmt, expr, ns, reqc);
//	// [0] currentToken, [1] NULL, [2] ....
//	size_t i;
//	KClass *requestClass = reqc;
//	KClass *paramType = (requestClass->baseTypeId == KType_Array) ? KClass_(requestClass->p0) : KClass_INFER;
//	for(i = 2; i < kArray_size(expr->NodeList); i++) {
//		kNode *typedNode = SUGAR TypeCheckNodeAt(kctx, expr, i, ns, paramType, 0);
//		if(typedNode == K_NULLNODE) {
//			KReturn(typedNode);
//		}
//		if(paramType->typeId == KType_var) {
//			paramType = KClass_(typedNode->attrTypeId);
//		}
//	}
//	if(requestClass->baseTypeId != KType_Array) {
//		requestClass = (paramType->typeId == KType_var) ? KClass_Array : KClass_p0(kctx, KClass_Array, paramType->typeId);
//	}
//	kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, KClass_Array, KMethodName_("[]"), -1, KMethodMatch_NoOption);
//	DBG_ASSERT(mtd != NULL);
//	KFieldSet(expr, expr->NodeList->MethodItems[0], mtd);
//	KFieldSet(expr, expr->NodeList->NodeItems[1], SUGAR kNode_SetVariable(kctx, KNewNode(ns), KNode_New, requestClass->typeId, kArray_size(expr->NodeList) - 2));
//	KReturn(kNode_Type(kctx, expr, KNode_MethodCall, requestClass->typeId));
//}
//
//static KMETHOD Expression_Bracket(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
//	KClass *genericsClass = NULL;
//	kNameSpace *ns = kNode_ns(stmt);
//	int nextIdx = SUGAR ParseTypePattern(kctx, ns, tokenList, beginIdx, endIdx, &genericsClass);
//	if(nextIdx != -1) {  // to avoid Func[T]
//		KReturn(SUGAR kNode_ParseOperatorNode(kctx, stmt, tokenList->TokenItems[beginIdx]->resolvedSyntaxInfo, tokenList, beginIdx, beginIdx, endIdx));
//	}
//	kToken *currentToken = tokenList->TokenItems[operatorIdx];
//	if(beginIdx == operatorIdx) {
//		/* transform '[ Value1, Value2, ... ]' to '(Call Untyped new (Value1, Value2, ...))' */
//		DBG_ASSERT(currentToken->resolvedSyntaxInfo->keyword == KSymbol_BracketGroup);
//		kNode *arrayNode = SUGAR new_UntypedOperatorNode(kctx, currentToken->resolvedSyntaxInfo, 2, currentToken, K_NULL);
//		KReturn(SUGAR AddParamNode(kctx, ns, arrayNode, RangeGroup(currentToken->GroupTokenList), NULL));
//	}
//	else {
//		kNode *leftNode = SUGAR ParseNewNode(kctx, ns, tokenList, beginIdx, operatorIdx, 0, NULL);
//		if(leftNode == K_NULLNODE) {
//			KReturn(leftNode);
//		}
//		if(leftNode->syn->keyword == KSymbol_("new")) {  // new int[100], new int[]();
//			DBG_P("cur:%d, beg:%d, endIdx:%d", operatorIdx, beginIdx, endIdx);
//			size_t subTokenSize = kArray_size(currentToken->GroupTokenList);
//			if(subTokenSize == 0) {
//				/* transform 'new Type0 [ ]' => (Call Type0 new) */
//				leftNode->syn = kSyntax_(ns, KSymbol_ParamPattern/*MethodCall*/);
//			} else {
//				/* transform 'new Type0 [ Type1 ] (...) => new 'Type0<Type1>' (...) */
//				KClass *classT0 = NULL;
//				kArray *GroupTokenList = currentToken->GroupTokenList;
//				int beginIdx = -1;
//				if(kArray_size(GroupTokenList) > 0) {
//					beginIdx = SUGAR ParseTypePattern(kctx, ns, RangeGroup(currentToken->GroupTokenList), &classT0);
//				}
//				beginIdx = (beginIdx == -1) ? 0 : beginIdx;
//				leftNode->syn = kSyntax_(ns, KSymbol_ParamPattern/*MethodCall*/);
//				DBG_P("currentToken->subtoken:%d", kArray_size(GroupTokenList));
//				leftNode = SUGAR AddParamNode(kctx, ns, leftNode, GroupTokenList, beginIdx, kArray_size(GroupTokenList) CHECK RANGE, "[");
//			}
//		}
//		else {
//			/* transform 'Value0 [ Value1 ]=> (Call Value0 get (Value1)) */
//			kTokenVar *tkN = /*G*/new_(TokenVar, 0, OnGcStack);
//			tkN->symbol= KMethodName_ToGetter(0);
//			tkN->uline = currentToken->uline;
//			kSyntax *syn = kSyntax_(kNode_ns(stmt), KSymbol_ParamPattern/*MethodCall*/);
//			leftNode  = SUGAR new_UntypedOperatorNode(kctx, syn, 2, tkN, leftNode);
//			leftNode = SUGAR AddParamNode(kctx, ns, leftNode, RangeGroup(currentToken->GroupTokenList), "[");
//		}
//		KReturn(SUGAR kNode_RightJoinNode(kctx, stmt, leftNode, tokenList, operatorIdx + 1, endIdx));
//	}
//}

static kbool_t array_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
//	KDEFINE_SYNTAX SYNTAX[] = {
//		{ KSymbol_BracketGroup, SYNFLAG_Suffix|SYNFLAG_CFunc, Precedence_CStyleSuffixCall, 0, {SUGARFUNC Expression_Bracket}, {SUGARFUNC TypeCheck_Bracket}, },
//		{ KSymbol_END, },
//	};
//	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
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

KDEFINE_PACKAGE* Array_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "JavaScript", "1.4");
	d.PackupNameSpace   = array_PackupNameSpace;
	d.ExportNameSpace   = array_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
