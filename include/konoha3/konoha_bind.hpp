/****************************************************************************
 * Copyright (c) 2013, Masahiro Ide <ide@konohascript.org>
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

extern "C" {
#include "konoha3.h"
#include "konoha3/import/methoddecl.h"
#include "konoha3/konoha_common.h"
};

namespace konoha {

template<typename T>
struct IsUnboxTypeImpl {
	static const bool value = false;
};

template<>
struct IsUnboxTypeImpl<bool> {
	static const bool value = true;
};

template<>
struct IsUnboxTypeImpl<int> {
	static const bool value = true;
};

template<>
struct IsUnboxTypeImpl<float> {
	static const bool value = true;
};

template<>
struct IsUnboxTypeImpl<double> {
	static const bool value = true;
};

template<>
struct IsUnboxTypeImpl<intptr_t> {
	static const bool value = true;
};

template<typename T = void>
struct IsUnboxType : public IsUnboxTypeImpl<T> {};


template<typename TargetType, typename SourceType>
static TargetType any_cast(SourceType Src) {
	union {
		SourceType Source;
		TargetType Target;
	} Val;
	Val.Source = Src;
	return Val.Target;
}

template<typename T>
struct Wrapper {
	kObjectHeader h;
	T *rawptr;
	static void init(KonohaContext *, kObject *obj, void *conf) {
		Wrapper<T> *Ref = reinterpret_cast<Wrapper<T> *>(obj);
		Ref->rawptr = 0;
	}
	static void reftrace(KonohaContext *, kObject *, struct KObjectVisitor *visitor) {}
	static void free(KonohaContext *, kObject *obj) {
		Wrapper<T> *Ref = reinterpret_cast<Wrapper<T> *>(obj);
		if(T *Ptr = Ref->rawptr) {
			delete Ptr;
		}
		Ref->rawptr = 0;
	}
	static uintptr_t unbox(KonohaContext *, kObject *obj) {
		Wrapper<T> *Ref = reinterpret_cast<Wrapper<T> *>(obj);
		return reinterpret_cast<uintptr_t>(Ref->rawptr);
	}

	template<typename Type, bool IsUnBoxType>
	struct CompareFunc {};
	
	template<typename Type>
	struct CompareFunc<Type, false> {
		static int compareTo(KonohaContext *, kObject *Obj1, kObject *Obj2) {
			Wrapper<Type> *Ref1 = reinterpret_cast<Wrapper<Type> *>(Obj1);
			Wrapper<Type> *Ref2 = reinterpret_cast<Wrapper<Type> *>(Obj2);
			return (Ref1->rawptr) == (Ref2->rawptr);
		}
		static int compareUnboxValue(uintptr_t Val1, uintptr_t Val2) {
			Type *Ptr1 = reinterpret_cast<Type *>(Val1);
			Type *Ptr2 = reinterpret_cast<Type *>(Val2);
			return Ptr1 == Ptr2;
		}
	};

	template<typename Type>
	struct CompareFunc<Type, true> {
		static int compareTo(KonohaContext *, kObject *Obj1, kObject *Obj2) {
			abort(); /* FIXME */
			Wrapper<Type> *Ref1 = reinterpret_cast<Wrapper<T> *>(Obj1);
			Wrapper<Type> *Ref2 = reinterpret_cast<Wrapper<T> *>(Obj2);
			return (Ref1->rawptr) == (Ref2->rawptr);
			//kNumber *Ref1 = reinterpret_cast<kNumber *>(Obj1);
			//kNumber *Ref2 = reinterpret_cast<kNumber *>(Obj2);
			//return KonohaType<T>(Ref1)::Get(Ref1) == KonohaType<T>::Get(Ref2);
		}
	
		static int compareUnboxValue(uintptr_t Val1, uintptr_t Val2) {
			T V1 = reinterpret_cast<T>(Val1);
			T V2 = reinterpret_cast<T>(Val2);
			return V1 == V2;
		}
	};

	static int compareTo(KonohaContext *kctx, kObject *Obj1, kObject *Obj2) {
		return CompareFunc<T, IsUnboxType<T>::value >::compareTo(kctx, Obj1, Obj2);
	}

	static int compareUnboxValue(uintptr_t Val1, uintptr_t Val2) {
		return CompareFunc<T, IsUnboxType<T>::value >::compareUnboxValue(Val1, Val2);
	}
};

template<typename T>
struct KonohaType {
	//static inline int TypeId(KonohaContext *kctx) { return A_Class->typeId; }
	static inline T *Get(KonohaContext *kctx, KonohaStack *sfp) {
		Wrapper<T> *Ref = reinterpret_cast<Wrapper<T> *>(sfp[0].asObject);
		return (Ref->rawptr);
	}
	static inline void Set(KonohaContext *kctx, KonohaStack *sfp, T *Val) {
		Wrapper<T> *Ref = reinterpret_cast<Wrapper<T> *>(sfp[0].asObject);
		Ref->rawptr = Val;
	}
};

template<>
struct KonohaType<void> {
	static inline int TypeId(KonohaContext *kctx) { return KType_void; }
};

template<>
struct KonohaType<bool> {
	static inline int TypeId(KonohaContext *kctx) { return KType_Boolean; }
	static inline bool Get(KonohaContext *kctx, KonohaStack *sfp) {
		return static_cast<bool>(sfp[0].intValue);
	}
	static inline void Set(KonohaContext *kctx, KonohaStack *sfp, bool Val) {
		sfp[0].boolValue = Val;
	}
};

template<>
struct KonohaType<int> {
	static inline int TypeId(KonohaContext *kctx) { return KType_Int; }
	static inline int Get(KonohaContext *kctx, KonohaStack *sfp) {
		return sfp[0].intValue;
	}
	static inline void Set(KonohaContext *kctx, KonohaStack *sfp, int Val) {
		sfp[0].intValue = Val;
	}
};

template<>
struct KonohaType<float> {
	static inline int TypeId(KonohaContext *kctx) { return KType_float; }
	static inline float Get(KonohaContext *kctx, KonohaStack *sfp) {
		return sfp[0].floatValue;
	}
	static inline void Set(KonohaContext *kctx, KonohaStack *sfp, float Val) {
		sfp[0].floatValue = Val;
	}
};

template<>
struct KonohaType<double> {
	static inline int TypeId(KonohaContext *kctx) { return KType_float; }
	static inline double Get(KonohaContext *kctx, KonohaStack *sfp) {
		return sfp[0].floatValue;
	}
	static inline void Set(KonohaContext *kctx, KonohaStack *sfp, double Val) {
		sfp[0].floatValue = Val;
	}
};

template<>
struct KonohaType<const char *> {
	static int TypeId(KonohaContext *kctx) { return KType_String; }
	static const char *Get(KonohaContext *kctx, KonohaStack *sfp) {
		kString *Str = sfp[0].asString;
		return kString_text(Str);
	}
	static inline void Set(KonohaContext *kctx, KonohaStack *sfp, const char *Val, size_t Len) {
		kString *Obj = KLIB new_kString(kctx, OnField, Val, Len, 0);
		KStackSetObjectValue(sfp[0].asString, Obj);
	}
	static inline void Set(KonohaContext *kctx, KonohaStack *sfp, const char *Val) {
		Set(kctx, sfp, Val, strlen(Val));
	}
};

template<>
struct KonohaType<std::string> {
	static int TypeId(KonohaContext *kctx) { return KType_String; }
};

template<typename T>
struct MethodBinder {
	//static const int ParamSize = 0;
	//static int GetReturnType(KonohaContext *kctx); /* = delete */
};

template<>
struct MethodBinder<void> {
	static const int ParamSize = 0;
	static int GetReturnType(KonohaContext *kctx) {
		return KType_void;
	}
};

template<typename RetType>
struct MethodBinder<RetType (*)()> {
	typedef RetType ReturnType;
	typedef void    ThisType;
	typedef ReturnType (FuncType)();
	static const int ParamSize = 0;

	static int GetReturnType(KonohaContext *kctx) {
		return KonohaType<ReturnType>::TypeId(kctx);
	}
	static void SetupMethodDecl(KonohaContext *kctx, KDEFINE_METHOD *MethodDataRef) {}

	static KMETHOD CallMethod(KonohaContext *kctx, KonohaStack *sfp, FuncType Func) {
		ReturnType Ret = (*Func)();
		KonohaType<ReturnType>::Set(kctx, sfp + K_RTNIDX, Ret);
	}

	static KMETHOD CallConstructor(KonohaContext *kctx, KonohaStack *sfp, FuncType Func) {
		ReturnType Ptr = (*Func)();
		kObject *Default = sfp[-K_CALLDELTA].asObject;
		kObject *Obj = KLIB new_kObject(kctx, OnStack, kObject_class(Default), 0);
		KStackSetObjectValue(sfp[K_RTNIDX].asObject, Obj);
		/*FIXME typeof Ptr is T*, not T** */
		KonohaType<ReturnType>::Set(kctx, sfp + K_RTNIDX, reinterpret_cast<ReturnType*>(Ptr));
	}
};

template<typename RetType, typename ParamType0>
struct MethodBinder<RetType (*)(ParamType0)> {
	typedef RetType ReturnType;
	typedef void    ThisType;
	typedef ReturnType (FuncType)(ParamType0);
	static const int ParamSize = 1;

	static int GetReturnType(KonohaContext *kctx) {
		return KonohaType<ReturnType>::TypeId(kctx);
	}
	static void SetupMethodDecl(KonohaContext *kctx, KDEFINE_METHOD *MethodDataRef) {
		int Type0 = KonohaType<ParamType0>::TypeId(kctx);
		MethodDataRef[0] = Type0;
		MethodDataRef[1] = KFieldName_("p0");;
	}

	static KMETHOD CallMethod(KonohaContext *kctx, KonohaStack *sfp, FuncType Func) {
		ParamType0 Param0 = KonohaType<ParamType0>::Get(kctx, sfp+1);
		ReturnType Ret = (*Func)(Param0);
		KonohaType<ReturnType>::Set(kctx, sfp + K_RTNIDX, Ret);
	}

	static KMETHOD CallConstructor(KonohaContext *kctx, KonohaStack *sfp, FuncType Func) {
		ParamType0 Param0 = KonohaType<ParamType0>::Get(kctx, sfp+1);
		ReturnType Ptr = (*Func)(Param0);
		kObject *Default = sfp[-K_CALLDELTA].asObject;
		kObject *Obj = KLIB new_kObject(kctx, OnStack, kObject_class(Default), 0);
		KStackSetObjectValue(sfp[K_RTNIDX].asObject, Obj);
		/*FIXME typeof Ptr is T*, not T** */
		KonohaType<ReturnType>::Set(kctx, sfp + K_RTNIDX, reinterpret_cast<ReturnType*>(Ptr));
	}
};


template<typename Class, typename RetType>
struct MethodBinder<RetType (Class::*)()> {
	typedef Class   ThisType;
	typedef RetType ReturnType;

	typedef ReturnType (Class::*FuncType)();
	static const int ParamSize = 0;
	static int GetReturnType(KonohaContext *kctx) {
		return KonohaType<ReturnType>::TypeId(kctx);
	}
	static void SetupMethodDecl(KonohaContext *kctx, KDEFINE_METHOD *MethodDataRef) {
	}
	static KMETHOD CallMethod(KonohaContext *kctx, KonohaStack *sfp, FuncType Func) {
		ThisType *thisObj = KonohaType<ThisType>::Get(kctx, sfp);
		ReturnType Ret = (thisObj->*Func)();
		KonohaType<ReturnType>::Set(kctx, sfp + K_RTNIDX, Ret);
	}
};

template<typename Class, typename RetType, typename ParamType0>
struct MethodBinder<RetType (Class::*)(ParamType0)> {
	typedef Class    ThisType;
	typedef RetType  ReturnType;
	typedef ReturnType (ThisType ::*FuncType)(ParamType0);

	static const int ParamSize = 1;
	static int GetReturnType(KonohaContext *kctx) {
		return KonohaType<ReturnType>::TypeId(kctx);
	}
	static void SetupMethodDecl(KonohaContext *kctx, KDEFINE_METHOD *MethodDataRef) {
		int Type0 = KonohaType<ParamType0>::TypeId(kctx);
		MethodDataRef[0] = Type0;
		MethodDataRef[1] = KFieldName_("p0");;
	}
	static KMETHOD CallMethod(KonohaContext *kctx, KonohaStack *sfp, FuncType Func) {
		ThisType *thisObj = KonohaType<ThisType>::Get(kctx, sfp);
		ParamType0 Param0 = KonohaType<ParamType0>::Get(kctx, sfp+1);
		ReturnType Ret = (thisObj->*Func)(Param0);
		KonohaType<ReturnType>::Set(kctx, sfp + K_RTNIDX, Ret);
	}
};

template<typename FuncType>
struct MethodWrapper {
	template<FuncType Func>
	struct Data {
		static KMethodFunc GetMethod() { return &CallMethod; };
		static KMethodFunc GetConstructor() { return &CallConstructor; };

		static const MethodBinder<FuncType> Binder;

		static KMETHOD CallMethod(KonohaContext *kctx, KonohaStack *sfp) {
			MethodBinder<FuncType>::CallMethod(kctx, sfp, Func);
		}
		static KMETHOD CallConstructor(KonohaContext *kctx, KonohaStack *sfp) {
			MethodBinder<FuncType>::CallConstructor(kctx, sfp, Func);
		}
	};

	template<FuncType Func>
	static Data<Func> Get() {
		Data<Func> D;
		return D;
	}
};

template<class T>
struct ClassBinder {
public:
	void DefineClass(KonohaContext *kctx, kNameSpace *ns, const char *className) {
		KDEFINE_CLASS Def = {};
		Def.typeId            = KTypeAttr_NewId;
		Def.structname        = className;
		Def.init              = &Wrapper<T>::init;
		Def.reftrace          = &Wrapper<T>::reftrace;
		Def.free              = &Wrapper<T>::free;
		Def.unbox             = &Wrapper<T>::unbox;
		Def.compareTo         = &Wrapper<T>::compareTo;
		Def.compareUnboxValue = &Wrapper<T>::compareUnboxValue;

		KBaseTrace(trace);
		ClassTable = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &Def, trace);
	};

	template<typename Wrapper>
	void DefineMethod(KonohaContext *kctx, kNameSpace *ns, const char *methodName, Wrapper W) {
		KBaseTrace(trace);

		KDEFINE_METHOD MethodData[1/*sentinel*/ + 6 + (W.Binder.ParamSize * 2)];
		MethodData[0] = _Public;
		MethodData[1] = _F(any_cast<uintptr_t>(W.GetMethod()));
		MethodData[2] = W.Binder.GetReturnType(kctx);
		MethodData[3] = static_cast<int>(ClassTable->typeId);
		MethodData[4] = KLIB Ksymbol(kctx, methodName, strlen(methodName), StringPolicy_ASCII|StringPolicy_TEXT, _NEWID);
		MethodData[5] = W.Binder.ParamSize;
		W.Binder.SetupMethodDecl(kctx, MethodData + 6);

		MethodData[6 + (W.Binder.ParamSize * 2)] = DEND;

		KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	}

	template<typename Wrapper>
	void DefineConstructor(KonohaContext *kctx, kNameSpace *ns, Wrapper W) {
		KBaseTrace(trace);
		KDEFINE_METHOD MethodData[1/*sentinel*/ + 6 + (W.Binder.ParamSize * 2)];
		MethodData[0] = _Public;
		MethodData[1] = _F(any_cast<uintptr_t>(W.GetConstructor()));

		MethodData[2] = static_cast<int>(ClassTable->typeId);
		MethodData[3] = static_cast<int>(ClassTable->typeId);
		MethodData[4] = KMethodName_("new");
		MethodData[5] = W.Binder.ParamSize;
		W.Binder.SetupMethodDecl(kctx, MethodData + 6);

		MethodData[6 + (W.Binder.ParamSize * 2)] = DEND;

		KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	}
private:
	KClass *ClassTable;
};

} /* namespace konoha */

///*------------------ [SampleCode] ------------------*/
//#include "konoha_bind.hpp"
//#include <iostream>
//
//class A {
//public:
//    A(int   a) : a(a)      { std::cout << "new0 a:" << a << std::endl; }
//    A(float a) : a(int(a)) { std::cout << "new1 a:" << a << std::endl; }
//    int f(int x) { std::cout << "a.f(x):" << a << std::endl; return a + x; }
//    int g(int x) { std::cout << "a.g(x):" << a << std::endl; return a + x; }
//    int g()      { std::cout << "a.g():"  << a << std::endl; return a; }
//    ~A() { std::cout << "delete a:" << a << std::endl; }
//private:
//    int a;
//};
//
//using namespace konoha;
//
//template<typename T>
//struct Constructor {
//public:
//    static T *New() { return new T(0); }
//    static T *New2(int x) { return new T(x); }
//};
//
//static kbool_t DefineClassAndMethods(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
//{
//    ClassBinder<A> ClassA;
//    ClassA.DefineClass(kctx,  ns, "A");
//
//    ClassA.DefineMethod(kctx, ns, "f", MethodWrapper<int (A::*)(int)>::Get<&A::f>());  /* define A.f(int) */
//    ClassA.DefineMethod(kctx, ns, "g", MethodWrapper<int (A::*)()>::Get<&A::g>());     /* define A.g() */
//    ClassA.DefineMethod(kctx, ns, "g", MethodWrapper<int (A::*)(int)>::Get<&A::g>());  /* define A.f(int) */
//
//    ClassA.DefineConstructor(kctx, ns, MethodWrapper<A *(*)()>::Get<&Constructor<A>::New>());     /* define A.new() */
//    ClassA.DefineConstructor(kctx, ns, MethodWrapper<A *(*)(int)>::Get<&Constructor<A>::New2>()); /* define A.new(int) */
//    return true;
//}
