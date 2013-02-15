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
#include <konoha3/klib.h>
#include <konoha3/konoha_common.h>
#include <konoha3/import/methoddecl.h>
#define _JS    kMethod_JSCompatible

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

// JavaScript Array.push accepts variable length arguments.
//## int Array.push(T0 value);
static KMETHOD Array_Push(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	Array_Add1(kctx, sfp);
	KReturnUnboxValue(kArray_size(a));
}

// JavaScript Array.unshift accepts variable length arguments.
//## int Array.unshift(T0 value);
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

//## T0 Array.pop();
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

//## Array Array.reverse();
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

//## T0 Array.shift();
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

//## Array Array.splice(int index, int length);
static KMETHOD Array_splice(KonohaContext *kctx, KonohaStack *sfp)
{
	INIT_GCSTACK();
	kArray *a = sfp[0].asArray;
	uintptr_t index = sfp[1].unboxValue;
	uintptr_t length = sfp[2].unboxValue;
	size_t i, asize = kArray_size(a);
	KCheckIndex(index, asize);
	KCheckIndex(index + length - 1, asize);
	kArray *returnValue = (kArray *)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), length);
	if(kArray_Is(UnboxData, a)) {
		for(i = 0; i < length; i++) {
			UnboxArray_Add(kctx, returnValue, a->unboxItems[index]);
			kArray_RemoveAt(kctx, a, index);
		}
	}
	else {
		for(i = 0; i < length; i++) {
			KLIB kArray_Add(kctx, returnValue, a->ObjectItems[index]);
			kArray_RemoveAt(kctx, a, index);
		}
	}
	KReturnWith(returnValue, RESET_GCSTACK());
}

//## method Array<T> Array.concat(Array<T> a1);
static KMETHOD Array_concat(KonohaContext *kctx, KonohaStack *sfp)
{
	INIT_GCSTACK();
	kArray *a0 = sfp[0].asArray;
	kArray *a1 = sfp[1].asArray;
	kArray *returnValue = (kArray *)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), kArray_size(a0) + kArray_size(a1));
	size_t i;
	if(kArray_Is(UnboxData, a0)) {
		for(i = 0; i < kArray_size(a0); i++){
			UnboxArray_Add(kctx, returnValue, a0->unboxItems[i]);
		}
		for(i = 0; i < kArray_size(a1); i++){
			UnboxArray_Add(kctx, returnValue, a1->unboxItems[i]);
		}
	}
	else {
		for(i = 0; i < kArray_size(a0); i++){
			KLIB kArray_Add(kctx, returnValue, a0->ObjectItems[i]);
		}
		for(i = 0; i < kArray_size(a1); i++){
			KLIB kArray_Add(kctx, returnValue, a1->ObjectItems[i]);
		}
	}
	KReturnWith(returnValue, RESET_GCSTACK());
}

static void kArray_join(KonohaContext *kctx, KBuffer *wb, kArray *a, const char *separator)
{
	size_t i, asize = kArray_size(a);
	size_t length = strlen(separator);
	BEGIN_UnusedStack(lsfp);
	KLIB KBuffer_Init(&(kctx->stack->cwb), wb);
	KClass *KClass_p0 = KClass_(kObject_p0(a));
	if(kArray_Is(UnboxData, a)) {
		for(i = 0; i < asize - 1; i++) {
			lsfp[0].unboxValue = a->unboxItems[i];
			KClass_p0->format(kctx, lsfp, 0, wb);
			KLIB KBuffer_Write(kctx, wb, separator, length);
		}
		if(asize > 0) {
			lsfp[0].unboxValue = a->unboxItems[asize - 1];
			KClass_p0->format(kctx, lsfp, 0, wb);
		}
	}
	else {
		for(i = 0; i < asize - 1; i++) {
			KUnsafeFieldSet(lsfp[0].asObject, a->ObjectItems[i]);
			KClass_p0->format(kctx, lsfp, 0, wb);
			KLIB KBuffer_Write(kctx, wb, separator, length);
		}
		if(asize > 0) {
			KUnsafeFieldSet(lsfp[0].asObject, a->ObjectItems[asize - 1]);
			KClass_p0->format(kctx, lsfp, 0, wb);
		}
	}
	END_UnusedStack();
}

//## String Array.join();
static KMETHOD Array_join0(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	KBuffer wb;
	kArray_join(kctx, &wb, a, ",");
	KReturnWith(
		KLIB new_kString(kctx, OnStack, KLIB KBuffer_text(kctx, &wb, 0), KBuffer_bytesize(&wb), 0),
		KLIB KBuffer_Free(&wb)
	);
}

//## String Array.join(String separator);
static KMETHOD Array_join1(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	const char *separator = kString_text(sfp[1].asString);
	KBuffer wb;
	kArray_join(kctx, &wb, a, separator);
	KReturnWith(
		KLIB new_kString(kctx, OnStack, KLIB KBuffer_text(kctx, &wb, 0), KBuffer_bytesize(&wb), 0),
		KLIB KBuffer_Free(&wb)
	);
}

static void kArray_slice(KonohaContext *kctx, kArray *toArray, kArray *fromArray, int begin, int end)
{
	DBG_ASSERT(begin < end);
	if(kArray_Is(UnboxData, fromArray)) {
		for(; begin < end; begin++) {
			UnboxArray_Add(kctx, toArray, fromArray->unboxItems[begin]);
		}
	}
	else {
		for(; begin < end; begin++) {
			KLIB kArray_Add(kctx, toArray, fromArray->ObjectItems[begin]);
		}
	}
}

//## Array Array.slice(int begin);
static KMETHOD Array_slice0(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	size_t asize = kArray_size(a);
	uintptr_t begin = sfp[1].intValue;
	if(begin >= asize) {
		KReturn(KLIB Knull(kctx, KGetReturnType(sfp)));
	}
	INIT_GCSTACK();
	KCheckIndex(begin, asize);
	kArray *returnValue = (kArray *)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), asize - begin);
	kArray_slice(kctx, returnValue, a, begin, asize);
	KReturnWith(returnValue, RESET_GCSTACK());
}

//## Array Array.slice(int begin, int end);
static KMETHOD Array_slice1(KonohaContext *kctx, KonohaStack *sfp)
{
	uintptr_t begin = sfp[1].intValue;
	uintptr_t end = sfp[2].intValue;
	if(begin >= end) {
		KReturn(KLIB Knull(kctx, KGetReturnType(sfp)));
	}
	INIT_GCSTACK();
	kArray *a = sfp[0].asArray;
	size_t asize = kArray_size(a);
	KCheckIndex(begin, asize);
	KCheckIndex(end - 1, asize);
	kArray *returnValue = (kArray *)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), end - begin);
	kArray_slice(kctx, returnValue, a, begin, end);
	KReturnWith(returnValue, RESET_GCSTACK());
}

static int kArray_indexOf(KonohaContext *kctx, kArray *a, uintptr_t e, size_t fromIndex)
{
	int res = -1;
	size_t i, asize = kArray_size(a);
	KClass *ElementClass =KClass_(kObject_p0(a));
	if(kArray_Is(UnboxData, a)) {
		for(i = fromIndex; i < asize; i++) {
			if(ElementClass->compareUnboxValue(a->unboxItems[i], e) == 0) {
				res = i; break;
			}
		}
	}
	else {
		kObject *o = (kObject *)e;
		for(i = fromIndex; i < asize; i++) {
			if(ElementClass->compareTo(kctx, a->ObjectItems[i], o) == 0) {
				res = i; break;
			}
		}
	}
	return res;
}

//## int Array.indexOf(T0 value);
static KMETHOD Array_indexOf0(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	uintptr_t value = kArray_Is(UnboxData, a) ?
		sfp[1].unboxValue : (uintptr_t) sfp[1].asObject;
	KReturnUnboxValue(kArray_indexOf(kctx, a, value, 0));
}

//## int Array.indexOf(T0 value, int fromIndex);
static KMETHOD Array_indexOf1(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	uintptr_t value = kArray_Is(UnboxData, a) ?
		sfp[1].unboxValue : (uintptr_t) sfp[1].asObject;
	uintptr_t fromIndex  = sfp[2].unboxValue;
	KReturnUnboxValue(kArray_indexOf(kctx, a, value, fromIndex));
}

static int kArray_lastIndexOf(KonohaContext *kctx, kArray *a, uintptr_t e, size_t fromIndex) {
	int res = -1;
	size_t i;
	if(kArray_Is(UnboxData, a)) {
		for(i = fromIndex; i >= 0; i--) {
			if(KClass_(kObject_p0(a))->compareUnboxValue(a->unboxItems[i], e) == 0) {
				res = i; break;
			}
		}
	}
	else {
		kObject *o = (kObject *)e;
		for(i = fromIndex; i >= 0; i--) {
			if(KClass_(kObject_p0(a))->compareTo(kctx, a->ObjectItems[i], o) == 0) {
				res = i; break;
			}
		}
	}
	return res;
}

//## int Array.lastIndexOf(T0 value);
static KMETHOD Array_lastIndexOf0(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	uintptr_t value = kArray_Is(UnboxData, a) ?
		sfp[1].unboxValue : (uintptr_t) sfp[1].asObject;
	KReturnUnboxValue(kArray_lastIndexOf(kctx, a, value, kArray_size(a)));
}

//## int Array.lastIndexOf(T0 value, int fromIndex);
static KMETHOD Array_lastIndexOf1(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	uintptr_t value = kArray_Is(UnboxData, a) ?
		sfp[1].unboxValue : (uintptr_t) sfp[1].asObject;
	uintptr_t fromIndex  = sfp[2].unboxValue;
	KReturnUnboxValue(kArray_lastIndexOf(kctx, a, value, fromIndex));
}

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

	KDEFINE_METHOD MethodData[] = {
		_Public|_Im,    _F(Array_get), KType_0,   KType_Array, KMethodName_("get"), 1, KType_Int, KFieldName_("index"),
		_Public,        _F(Array_Set), KType_void, KType_Array, KMethodName_("set"), 2, KType_Int, KFieldName_("index"),  KType_0, KFieldName_("value"),
		/* Mutator Methods */
		_JS|_Public,        _F(Array_Pop), KType_0, KType_Array, KMethodName_("pop"), 0,
		_JS|_Public,        _F(Array_Push), KType_Int, KType_Array, KMethodName_("push"), 1, KType_0, KFieldName_("value"),
		_JS|_Public,        _F(Array_reverse), KType_Array, KType_Array, KMethodName_("reverse"), 0,
		_JS|_Public,        _F(Array_shift), KType_0, KType_Array, KMethodName_("shift"), 0,
		_JS|_Public,        _F(Array_splice), KType_ArrayT0, KType_Array, KMethodName_("splice"), 2, KType_Int, KFieldName_("index"), KType_Int, KFieldName_("length"),
		_JS|_Public,        _F(Array_unshift), KType_Int, KType_Array, KMethodName_("unshift"), 1, KType_0, KFieldName_("value"),

		/* Accessor Methods */
		_JS|_Public|_Im,    _F(Array_concat), KType_ArrayT0, KType_Array, KMethodName_("concat"), 1, KType_ArrayT0, KFieldName_("a1"),
		_JS|_Public|_Im,    _F(Array_join0), KType_String, KType_Array, KMethodName_("join"), 0,
		_JS|_Public|_Im,    _F(Array_join1), KType_String, KType_Array, KMethodName_("join"), 1, KType_String, KFieldName_("separator"),
		_JS|_Public|_Im,    _F(Array_slice0), KType_ArrayT0, KType_Array, KMethodName_("slice"), 1, KType_Int, KFieldName_("begin"),
		_JS|_Public|_Im,    _F(Array_slice1), KType_ArrayT0, KType_Array, KMethodName_("slice"), 2, KType_Int, KFieldName_("begin"), KType_Int, KFieldName_("end"),
		_JS|_Public|_Im,    _F(Array_indexOf0), KType_Int, KType_Array, KMethodName_("indexOf"), 1, KType_0, KFieldName_("value"),
		_JS|_Public|_Im,    _F(Array_indexOf1), KType_Int, KType_Array, KMethodName_("indexOf"), 2, KType_0, KFieldName_("value"), KType_Int, KFieldName_("fromIndex"),
		_JS|_Public|_Im,    _F(Array_lastIndexOf0), KType_Int, KType_Array, KMethodName_("lastIndexOf"), 1, KType_0, KFieldName_("value"),
		_JS|_Public|_Im,    _F(Array_lastIndexOf1), KType_Int, KType_Array, KMethodName_("lastIndexOf"), 2, KType_0, KFieldName_("value"), KType_Int, KFieldName_("fromIndex"),

		_Public|_Im,    _F(Array_RemoveAt), KType_0,   KType_Array, KMethodName_("removeAt"), 1, KType_Int, KFieldName_("index"),
		_Public|_Const, _F(Array_getSize), KType_Int, KType_Array, KMethodName_("getSize"), 0,
		_Public|_Const, _F(Array_getSize), KType_Int, KType_Array, KMethodName_("getlength"), 0,
		_Public,        _F(Array_clear), KType_void, KType_Array, KMethodName_("clear"), 0,
		_Public,        _F(Array_Add1), KType_void, KType_Array, KMethodName_("add"), 1, KType_0, KFieldName_("value"),
		_Public|_Im,    _F(Array_new), KType_ArrayT0, KType_Array, KMethodName_("new"), 1, KType_Int, KFieldName_("size"),
		_Public,        _F(Array_newArray), KType_ArrayT0, KType_Array, KMethodName_("newArray"), 1, KType_Int, KFieldName_("size"),
		_Public|kMethod_Hidden, _F(Array_newList), KType_ArrayT0, KType_Array, KMethodName_("[]"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

// ---------------------------------------------------------------------------
/* Syntax */

static KMETHOD TypeCheck_ArrayLiteral(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck2(stmt, expr, ns, reqc);
	// [0] currentToken, [1] K_NULLNODE, [2] ....
	size_t i;
	/* if the requested type is Array[T], then type the element of [x, y, z]. */
	KClass *paramType = (reqc->baseTypeId == KType_Array) ? KClass_(reqc->p0) : KClass_INFER;
	for(i = 2; i < kArray_size(expr->NodeList); i++) {
		kNode *typedNode = SUGAR TypeCheckNodeAt(kctx, expr, i, ns, paramType, 0);
		if(kNode_IsError(typedNode)) {
			KReturn(typedNode);
		}
		if(paramType->typeId == KType_var) {
			paramType = KClass_(typedNode->attrTypeId);
		}
	}
	if(reqc->baseTypeId != KType_Array) {
		reqc = (paramType->typeId == KType_var) ? KClass_Array : KClass_p0(kctx, KClass_Array, paramType->typeId);
	}
	kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, KClass_Array, KMethodName_("[]"), -1, KMethodMatch_NoOption);
	DBG_ASSERT(mtd != NULL);
	KFieldSet(expr, expr->NodeList->MethodItems[0], mtd);
	KFieldSet(expr, expr->NodeList->NodeItems[1], SUGAR kNode_SetVariable(kctx, KNewNode(ns), KNode_New, reqc->typeId, kArray_size(expr->NodeList) - 2));
	KReturn(kNode_Type(kctx, expr, KNode_MethodCall, reqc->typeId));
}

static KMETHOD Expression_ArrayLiteral(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, opIdx, endIdx);
	kToken *groupToken = tokenList->TokenItems[opIdx];
	if(beginIdx == opIdx) {
		/* transform '[ Value1, Value2, ... ]' to '(Call Untyped new (Value1, Value2, ...))' */
		DBG_ASSERT(groupToken->resolvedSyntaxInfo->keyword == KSymbol_BracketGroup);
		SUGAR kNode_Op(kctx, stmt, groupToken, 1, K_NULLNODE);
		SUGAR AppendParsedNode(kctx, stmt, RangeGroup(groupToken->GroupTokenList), NULL, ParseExpressionOption, NULL);
		KReturnUnboxValue(opIdx+1);
	}
	KReturnUnboxValue(-1);
}

static kbool_t array_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_BracketGroup, SYNFLAG_Suffix|SYNFLAG_CFunc|SYNFLAG_TypeSuffix, Precedence_CStyleSuffixCall, 0, {SUGARFUNC Expression_ArrayLiteral}, {SUGARFUNC TypeCheck_ArrayLiteral}, },
		{ KSymbol_END, },
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

KDEFINE_PACKAGE *Array_Init(void)
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
