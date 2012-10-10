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
#include <minikonoha/float.h>

#ifdef __cplusplus
extern "C"{
#endif

/* ------------------------------------------------------------------------ */

//## @Immutable method T0 Array.get(Int n);
static KMETHOD Array_get(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	size_t n = check_index(kctx, sfp[1].intValue, kArray_size(a), sfp[K_RTNIDX].uline);
	if (kArray_isUnboxData(a)) {
		RETURNd_(a->unboxItems[n]);
	}
	else {
		RETURN_(a->objectItems[n]);
	}
}

//## method void Array.set(Int n, T0 v);
static KMETHOD Array_set(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	size_t n = check_index(kctx, sfp[1].intValue, kArray_size(a), sfp[K_RTNIDX].uline);
	if (kArray_isUnboxData(a)) {
		a->unboxItems[n] = sfp[2].unboxValue;
	}
	else {
		KSETv(a, a->objectItems[n], sfp[2].o);
	}
}

//## method int Array.getSize();
static KMETHOD Array_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	RETURNi_(kArray_size(a));
}

static KMETHOD Array_newArray(KonohaContext *kctx, KonohaStack *sfp)
{
	kArrayVar *a = (kArrayVar *)sfp[0].asObject;
	if (sfp[1].intValue < 0) {
		OLDTRACE_SWITCH_TO_KTrace(_UserInputFault,
				LogText("error", "Invalid argument"),
				LogUint("length", sfp[1].intValue)
		);
		RETURN_(a);
	}
	size_t asize = (size_t)sfp[1].intValue;
	a->bytemax = asize * sizeof(uintptr_t);
	kArray_setsize(a, asize);
	a->objectItems = (kObject**)KCALLOC(a->bytemax, 1);
	if (!kArray_isUnboxData(a)) {
		size_t i;
		kObject *null = KLIB Knull(kctx, CT_(O_p0(a)));
		for(i = 0; i < asize; i++) {
			KSETv(a, a->objectItems[i], null);
		}
	}
	RETURN_(a);
}

// Array
struct _kAbstractArray {
	KonohaObjectHeader h;
	KUtilsGrowingArray a;
};

static void UnboxArray_ensureMinimumSize(KonohaContext *kctx, struct _kAbstractArray *a, size_t min)
{
	size_t minbyte = min * sizeof(uintptr_t);
	if (!(minbyte < a->a.bytemax)) {
		if (minbyte < sizeof(kObject)) minbyte = sizeof(kObject);
		KLIB Karray_expand(kctx, &a->a, minbyte);
	}
}

static void UnboxArray_add(KonohaContext *kctx, kArray *o, uintptr_t value)
{
	size_t asize = kArray_size(o);
	struct _kAbstractArray *a = (struct _kAbstractArray*)o;
	UnboxArray_ensureMinimumSize(kctx, a, asize+1);
	DBG_ASSERT(a->a.objectItems[asize] == NULL);
	o->unboxItems[asize] = value;
	kArray_setsize(o, (asize+1));
}

static void UnboxArray_insert(KonohaContext *kctx, kArray *o, size_t n, uintptr_t v)
{
	size_t asize = kArray_size(o);
	struct _kAbstractArray *a = (struct _kAbstractArray*)o;
	if (!(n < asize)) {
		UnboxArray_add(kctx, o, v);
	}
	else {
		UnboxArray_ensureMinimumSize(kctx, a, asize+1);
		DBG_ASSERT(a->a.objectItems[asize] == NULL);
		memmove(o->unboxItems+(n+1), o->unboxItems+n, sizeof(uintptr_t) * (asize - n));
		o->unboxItems[n] = v;
		kArray_setsize(o, (asize+1));
	}
}

static KMETHOD Array_clear(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	KLIB kArray_clear(kctx, a, 0);
}

static KMETHOD Array_add1(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	if (kArray_isUnboxData(a)) {
		UnboxArray_add(kctx, a, sfp[1].unboxValue);
	} else {
		KLIB kArray_add(kctx, a, sfp[1].asObject);
	}
}

static KMETHOD Array_push(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	Array_add1(kctx, sfp);
	RETURNi_(kArray_size(a));
}

static KMETHOD Array_unshift(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	if (kArray_isUnboxData(a)) {
		UnboxArray_insert(kctx, a, 0, sfp[1].unboxValue);
	} else {
		KLIB kArray_insert(kctx, a, 0, sfp[1].asObject);
	}
	RETURNi_(kArray_size(a));
}

static KMETHOD Array_pop(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	if (kArray_size(a) == 0)
		RETURN_DefaultObjectValue();
	size_t n = kArray_size(a) - 1;
	if (kArray_isUnboxData(a)) {
		uintptr_t v = a->unboxItems[n];
		a->unboxItems[n] = 0;
		kArray_setsize(a, n);
		RETURNd_(v);
	}
	else {
		struct _kAbstractArray *a2 = (struct _kAbstractArray*)a;
		kObject *value = a2->a.objectItems[n];
		kObject** null = NULL;
		KINITp(a2, a2->a.objectItems[n], null);
		a2->a.bytesize = n * sizeof(uintptr_t);
		RETURN_(value);
	}
}

static void kArray_removeAt(KonohaContext *kctx, kArray *a, size_t n)
{
	size_t asize = kArray_size(a);
	if (kArray_isUnboxData(a)) {
		kArrayVar *a2 = (kArrayVar *)a;
		memmove(a2->unboxItems+n, a2->unboxItems+(n+1), sizeof(uintptr_t) * (asize - n - 1));
		kArray_setsize(a2, asize - 1);
	}
	else {
		struct _kAbstractArray *a2 = (struct _kAbstractArray*)a;
		memmove(a2->a.objectItems+n, a2->a.objectItems+(n+1), sizeof(kObject*) * (asize - n - 1));
		a2->a.bytesize = (asize - 1) * sizeof(uintptr_t);
	}
}

static KMETHOD Array_removeAt(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	int n = (int)sfp[1].unboxValue;
	struct _kAbstractArray *a2 = (struct _kAbstractArray*)a;
	if (kArray_isUnboxData(a)) {
		uintptr_t v = a->unboxItems[n];
		kArray_removeAt(kctx, a, n);
		RETURNd_(v);
	}
	else {
		kObject *value = a2->a.objectItems[n];
		kArray_removeAt(kctx, a, n);
		RETURN_(value);
	}
}

static KMETHOD Array_reverse(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	size_t asize = kArray_size(a);
	size_t asize_half = asize / 2;
	size_t i;
	if (kArray_isUnboxData(a)) {
		for(i = 0; i != asize_half; ++i) {
			uintptr_t temp = a->unboxItems[asize - 1 - i];
			a->unboxItems[asize - 1 - i] = a->unboxItems[i];
			a->unboxItems[i] = temp;
		}
	}
	else {
		for(i = 0; i != asize_half; ++i){
			kObject *temp = a->objectItems[asize - 1 - i];
			KSETv(a, a->objectItems[asize - 1 - i], a->objectItems[i]);
			KSETv(a, a->objectItems[i], temp);
		}
	}
	RETURN_(a);
}

/* Array[T] Array[T].map(Func[T,T] func) */
static KMETHOD Array_map(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	kFunc  *f = sfp[1].asFunc;
	size_t asize = kArray_size(a);
	ktype_t resolve_type = Method_returnType(f->mtd);

	KonohaClass *CT_ArrayT0 = CT_p0(kctx, CT_Array, resolve_type);
	kArrayVar *ret = (kArrayVar*)KLIB new_kObject(kctx, CT_ArrayT0, asize);

	size_t i;
	if (kArray_isUnboxData(a)) {
		for(i=0; i != asize; ++i) {
			uintptr_t tmp = a->unboxItems[i];
			BEGIN_LOCAL(lsfp, K_CALLDELTA + 1);
			KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+0].o, K_NULL, GC_NO_WRITE_BARRIER);
			lsfp[K_CALLDELTA+1].unboxValue = tmp;
			KCALL(lsfp, 0, f->mtd, 1, KLIB Knull(kctx, CT_(resolve_type)));
			END_LOCAL();
			ret->unboxItems[i] = lsfp[0].unboxValue;
		}
	}
	else {
		for(i=0; i != asize; ++i) {
			kObject *tmp  = a->objectItems[i];
			BEGIN_LOCAL(lsfp, K_CALLDELTA + 1);
			KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+0].o, K_NULL, GC_NO_WRITE_BARRIER);
			KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+1].o, tmp, GC_NO_WRITE_BARRIER);
			KCALL(lsfp, 0, f->mtd, 1, KLIB Knull(kctx, CT_(resolve_type)));
			END_LOCAL();
			KSETv(ret, ret->objectItems[i], lsfp[0].o);
		}
	}
	kArray_setsize(ret, asize);
	RETURN_(ret);
}

/* T Array[T].inject(Func[T,T,T] func) */
static KMETHOD Array_inject(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	kFunc  *f = sfp[1].asFunc;
	size_t asize = kArray_size(a);
	ktype_t resolve_type = Method_returnType(f->mtd);

	size_t i;
	if (kArray_isUnboxData(a)) {
		uintptr_t tmp = 0;
		BEGIN_LOCAL(lsfp, K_CALLDELTA + 2);
		for(i=0; i != asize; ++i) {
			KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+0].o, K_NULL, GC_NO_WRITE_BARRIER);
			lsfp[K_CALLDELTA+1].unboxValue = tmp;
			lsfp[K_CALLDELTA+2].unboxValue = a->unboxItems[i];
			KCALL(lsfp, 0, f->mtd, 2, KLIB Knull(kctx, CT_(resolve_type)));
			tmp = lsfp[0].unboxValue;
		}
		END_LOCAL();
		RETURNd_(tmp);
	}
	else {
		kObject *tmp = (kObject*) KLIB new_kObject(kctx, CT_(resolve_type), 0);
		kObject *nulobj = KLIB Knull(kctx, CT_(resolve_type));
		BEGIN_LOCAL(lsfp, K_CALLDELTA + 2);
		for(i=0; i != asize; ++i) {
			KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+0].o, K_NULL, GC_NO_WRITE_BARRIER);
			KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+1].o, tmp,    GC_NO_WRITE_BARRIER);
			KSETv_AND_WRITE_BARRIER(NULL, lsfp[K_CALLDELTA+2].o, a->objectItems[i], GC_NO_WRITE_BARRIER);
			KCALL(lsfp, 0, f->mtd, 2, nulobj);
			KSETv(NULL, tmp, lsfp[0].o);
		}
		END_LOCAL();
		RETURN_(tmp);
	}
}

static KMETHOD Array_shift(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	struct _kAbstractArray *a2 = (struct _kAbstractArray*)a;
	if (kArray_isUnboxData(a)) {
		uintptr_t v = a->unboxItems[0];
		kArray_removeAt(kctx, a, 0);
		RETURNd_(v);
	}
	else {
		kObject *value = a2->a.objectItems[0];
		kArray_removeAt(kctx, a, 0);
		RETURN_(value);
	}
}

//## method Array<T> Array.concat(Array<T> a1);
static KMETHOD Array_concat(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a0 = sfp[0].asArray;
	kArray *a1 = sfp[1].asArray;
	size_t i;
	if (kArray_isUnboxData(a1)) {
		for (i = 0; i < kArray_size(a1); i++){
			UnboxArray_add(kctx, a0, a1->unboxItems[i]);
		}
	} else {
		for (i = 0; i < kArray_size(a1); i++){
			KLIB kArray_add(kctx, a0, a1->objectItems[i]);
		}
	}
	RETURN_(a0);
}

//## method int Array.indexOf(T0 a1);
static KMETHOD Array_indexOf(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	kint_t res = -1;
	size_t i;
	if (kArray_isUnboxData(a)) {
		uintptr_t nv = sfp[1].unboxValue;
		for(i = 0; i < kArray_size(a); i++) {
			if (a->unboxItems[i] == nv) {
				res = i; break;
			}
		}
	} else {
		//TODO:Need to implement Object compareTo.
		kObject *o = sfp[1].asObject;
		for(i = 0; i < kArray_size(a); i++) {
			KLIB KonohaRuntime_raise(kctx, EXPT_("NotImplemented"), sfp, sfp[K_RTNIDX].uline, NULL);
			if (O_ct(o)->compareObject(a->objectItems[i], o) == 0) {
				res = i; break;
			}
		}
	}
	RETURNi_(res);
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
			KLIB KonohaRuntime_raise(kctx, EXPT_("NotImplemented"), sfp, sfp[K_RTNIDX].uline, NULL);
			if(O_ct(o)->compareObject(a->objectItems[i], o) == 0) {
				break;
			}
		}
	}
	res = i;
	RETURNi_(res);
}

//## method String Array.toString();
//static KMETHOD Array_toString(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kArray *a = sfp[0].asArray;
//	size_t i = 0;
//	KUtilsWriteBuffer wb;
//	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
//	if (kArray_size(a) < 1) {
//		RETURN_(KNULL(String));
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
//			obj = a->objectItems[i];
//			DBG_ASSERT(O_ct(obj)->p);
//			// before call system.p, push variables
//			KSETv_AND_WRITE_BARRIER(NULL, lsfp[0].o, obj, GC_NO_WRITE_BARRIER);
//			O_ct(obj)->p(kctx, lsfp, 0, &wb, 0);
//			KLIB Kwb_printf(kctx, &wb, ",");
//		}
//		obj = a->objectItems[i];
//		KSETv_AND_WRITE_BARRIER(NULL, lsfp[0].o, obj, GC_NO_WRITE_BARRIER);
//		O_ct(obj)->p(kctx, lsfp, 0, &wb, 0);
//		END_LOCAL();
//
//	}
//	const char *KUtilsWriteBufferTopChar = KLIB Kwb_top(kctx, &wb, 0);
//	size_t strsize = strlen(KUtilsWriteBufferTopChar);
//	kString *ret = KLIB new_kString(kctx, KUtilsWriteBufferTopChar, strsize, 0);
//	KLIB Kwb_free(&wb);
//	RETURN_(ret);
//}

static KMETHOD Array_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kArrayVar *a = (kArrayVar *)sfp[0].asObject;
	size_t asize = (size_t)sfp[1].intValue;
	a->bytemax = asize * sizeof(uintptr_t);
	kArray_setsize(a, 0);
	a->objectItems = (kObject**)KCALLOC(a->bytemax, 1);
	if (!kArray_isUnboxData(a)) {
		size_t i;
		kObject *null = KLIB Knull(kctx, CT_(O_p0(a)));
		for(i = 0; i < asize; i++) {
			KSETv(a, a->objectItems[i], null);
		}
	}
	RETURN_(a);
}

static KMETHOD Array_newList(KonohaContext *kctx, KonohaStack *sfp)
{
	kArrayVar *a = (kArrayVar*)sfp[0].asObject;
	KonohaStack *p = sfp+1;
	if (kArray_isUnboxData(a)) {
		for(; p < kctx->esp; p++) {
			UnboxArray_add(kctx, a, p[0].unboxValue);
		}
	}
	else {
		for(; p < kctx->esp; p++) {
			KLIB kArray_add(kctx, a, p[0].asObject);
		}
	}
	RETURN_(a);
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Im       kMethod_Immutable
#define _F(F)     (intptr_t)(F)

static kbool_t array_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KDEFINE_INT_CONST ClassData[] = {   // add Array as available
		{"Array", TY_TYPE, (uintptr_t)CT_(TY_Array)},
		{NULL},
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(ClassData), 0);

	KonohaClass *CT_ArrayT0 = CT_p0(kctx, CT_Array, TY_0);
	ktype_t TY_ArrayT0 = CT_ArrayT0->typeId;

	kparamtype_t p[] = {{TY_0}};
	ktype_t TY_FuncMap = (KLIB KonohaClass_Generics(kctx, CT_Func, TY_0 , 1, p))->typeId;

	kparamtype_t P_inject[] = {{TY_0},{TY_0}};
	ktype_t TY_FuncInject = (KLIB KonohaClass_Generics(kctx, CT_Func, TY_0 , 2, P_inject))->typeId;

	KDEFINE_METHOD MethodData[] = {
		_Public|_Im,    _F(Array_get), TY_0,   TY_Array, MN_("get"), 1, TY_int, FN_("index"),
		_Public,        _F(Array_set), TY_void, TY_Array, MN_("set"), 2, TY_int, FN_("index"),  TY_0, FN_("value"),
		_Public|_Im,    _F(Array_removeAt), TY_0,   TY_Array, MN_("removeAt"), 1, TY_int, FN_("index"),
		_Public|_Const, _F(Array_getSize), TY_int, TY_Array, MN_("getSize"), 0,
		_Public|_Const, _F(Array_getSize), TY_int, TY_Array, MN_("getlength"), 0,
		_Public,        _F(Array_clear), TY_void, TY_Array, MN_("clear"), 0,
		_Public,        _F(Array_add1), TY_void, TY_Array, MN_("add"), 1, TY_0, FN_("value"),
		_Public,        _F(Array_push), TY_int, TY_Array, MN_("push"), 1, TY_0, FN_("value"),
		_Public,        _F(Array_pop), TY_0, TY_Array, MN_("pop"), 0,
		_Public,        _F(Array_shift), TY_0, TY_Array, MN_("shift"), 0,
		_Public,        _F(Array_unshift), TY_int, TY_Array, MN_("unshift"), 1, TY_0, FN_("value"),
		_Public,        _F(Array_reverse), TY_Array, TY_Array, MN_("reverse"), 0,
		_Public|_Im,    _F(Array_map), TY_ArrayT0, TY_Array, MN_("map"), 1, TY_FuncMap, FN_("func"),
		_Public|_Im,    _F(Array_inject), TY_0, TY_Array, MN_("inject"), 1, TY_FuncInject, FN_("func"),

		_Public,        _F(Array_concat), TY_ArrayT0, TY_Array, MN_("concat"), 1, TY_ArrayT0, FN_("a1"),
		_Public,        _F(Array_indexOf), TY_int, TY_Array, MN_("indexOf"), 1, TY_0, FN_("value"),
		_Public,        _F(Array_lastIndexOf), TY_int, TY_Array, MN_("lastIndexOf"), 1, TY_0, FN_("value"),
//		_Public,        _F(Array_toString), TY_String, TY_Array, MN_("toString"), 0,
		_Public|_Im,    _F(Array_new), TY_void, TY_Array, MN_("new"), 1, TY_int, FN_("size"),
		_Public,        _F(Array_newArray), TY_Array, TY_Array, MN_("newArray"), 1, TY_int, FN_("size"),
		_Public|kMethod_Hidden, _F(Array_newList), TY_Array, TY_Array, MN_("newList"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t array_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static KMETHOD TypeCheck_Bracket(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	// [0] currentToken, [1] NULL, [2] ....
	size_t i;
	KonohaClass *requestClass = CT_(reqty);
	ktype_t paramType = TY_var; // default
	if (requestClass->baseTypeId == TY_Array) {
		paramType = requestClass->p0;
	}
	else {
		requestClass = NULL; // undefined
	}
	for(i = 2; i < kArray_size(expr->cons); i++) {
		kExpr *typedExpr = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, i, gma, paramType, 0);
		if (typedExpr == K_NULLEXPR) {
			RETURN_(typedExpr);
		}
		DBG_P("i=%d, paramType=%s, typedExpr->ty=%s", i, TY_t(paramType), TY_t(typedExpr->ty));
		if (paramType == TY_var) {
			paramType = typedExpr->ty;
		}
	}
	if (requestClass == NULL) {
		requestClass = (paramType == TY_var) ? CT_Array : CT_p0(kctx, CT_Array, paramType);
	}
	kMethod *mtd = KLIB kNameSpace_getMethodByParamSizeNULL(kctx, Stmt_nameSpace(stmt), TY_Array, MN_("newList"), -1);
	DBG_ASSERT(mtd != NULL);
	KSETv(expr, expr->cons->methodItems[0], mtd);
	KSETv(expr, expr->cons->exprItems[1], SUGAR kExpr_setVariable(kctx, NULL, gma, TEXPR_NEW, requestClass->typeId, 0));
	RETURN_(Expr_typed(expr, TEXPR_CALL, requestClass->typeId));
}

static KMETHOD Expression_Bracket(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	KonohaClass *genericsClass = NULL;
	kNameSpace *ns = Stmt_nameSpace(stmt);
	int nextIdx = SUGAR TokenUtils_parseTypePattern(kctx, ns, tokenList, beginIdx, endIdx, &genericsClass);
	if (nextIdx != -1) {  // to avoid Func[T]
		RETURN_(SUGAR kStmt_parseOperatorExpr(kctx, stmt, tokenList->tokenItems[beginIdx]->resolvedSyntaxInfo, tokenList, beginIdx, beginIdx, endIdx));
	}
	kToken *currentToken = tokenList->tokenItems[operatorIdx];
	if (beginIdx == operatorIdx) {
		/* transform '[ Value1, Value2, ... ]' to '(Call Untyped new (Value1, Value2, ...))' */
		DBG_ASSERT(currentToken->resolvedSyntaxInfo->keyword == KW_BracketGroup);
		kExpr *arrayExpr = SUGAR new_UntypedCallStyleExpr(kctx, currentToken->resolvedSyntaxInfo, 2, currentToken, K_NULL);
		RETURN_(SUGAR kStmt_addExprParam(kctx, stmt, arrayExpr, currentToken->subTokenList, 0, kArray_size(currentToken->subTokenList), NULL));
	}
	else {
		kExpr *leftExpr = SUGAR kStmt_parseExpr(kctx, stmt, tokenList, beginIdx, operatorIdx, NULL);
		if (leftExpr == K_NULLEXPR) {
			RETURN_(leftExpr);
		}
		if (leftExpr->syn->keyword == SYM_("new")) {  // new int[100], new int[]();
			DBG_P("cur:%d, beg:%d, endIdx:%d", operatorIdx, beginIdx, endIdx);
			size_t subTokenSize = kArray_size(currentToken->subTokenList);
			if (subTokenSize == 0) {
				/* transform 'new Type0 [ ]' => (Call Type0 new) */
				kExpr_setsyn(leftExpr, SYN_(ns, KW_ExprMethodCall));
			} else {
				/* transform 'new Type0 [ Type1 ] (...) => new 'Type0<Type1>' (...) */
				KonohaClass *classT0 = NULL;
				kArray *subTokenList = currentToken->subTokenList;
				int beginIdx = -1;
				if (kArray_size(subTokenList) > 0) {
					beginIdx = SUGAR TokenUtils_parseTypePattern(kctx, ns, subTokenList, 0, kArray_size(subTokenList), &classT0);
				}
				beginIdx = (beginIdx == -1) ? 0 : beginIdx;
				kExpr_setsyn(leftExpr, SYN_(ns, KW_ExprMethodCall));
				DBG_P("currentToken->subtoken:%d", kArray_size(subTokenList));
				leftExpr = SUGAR kStmt_addExprParam(kctx, stmt, leftExpr, subTokenList, beginIdx, kArray_size(subTokenList), "[");
			}
		}
		else {
			/* transform 'Value0 [ Value1 ]=> (Call Value0 get (Value1)) */
			kTokenVar *tkN = GCSAFE_new(TokenVar, 0);
			tkN->resolvedSymbol= MN_toGETTER(0);
			tkN->uline = currentToken->uline;
			SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall);
			leftExpr  = SUGAR new_UntypedCallStyleExpr(kctx, syn, 2, tkN, leftExpr);
			leftExpr = SUGAR kStmt_addExprParam(kctx, stmt, leftExpr, currentToken->subTokenList, 0, kArray_size(currentToken->subTokenList), "[");
		}
		RETURN_(SUGAR kStmt_rightJoinExpr(kctx, stmt, leftExpr, tokenList, operatorIdx + 1, endIdx));
	}
}

static kbool_t array_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KW_BracketGroup, SYNFLAG_ExprPostfixOp2, NULL, Precedence_CStyleCALL, 0, NULL, Expression_Bracket, NULL, NULL, TypeCheck_Bracket, },
		{ KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNameSpace);
	return true;
}

static kbool_t array_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* array_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSETPACKNAME(d, "array", "1.0");
	d.initPackage    = array_initPackage;
	d.setupPackage   = array_setupPackage;
	d.initNameSpace  = array_initNameSpace;
	d.setupNameSpace = array_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
