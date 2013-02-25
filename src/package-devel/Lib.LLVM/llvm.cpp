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

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Intrinsics.h"
#include "llvm/Attributes.h"
#include "llvm/PassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/IPO.h"

#if LLVM_VERSION > 209
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#endif
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#if LLVM_VERSION >= 301
#include "llvm/Transforms/Vectorize.h"
#endif
#include "llvm/Analysis/Verifier.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/DomPrinter.h"
#if LLVM_VERSION > 208
#include "llvm/Analysis/RegionPass.h"
#endif
#include "llvm/Analysis/RegionPrinter.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/Lint.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#if LLVM_VERSION >= 302
#include "llvm/IRBuilder.h"
#else
#include "llvm/Support/IRBuilder.h"
#endif

#if LLVM_VERSION <= 208
#include "llvm/Support/DynamicLinker.h"
#else
#include "llvm/Support/DynamicLibrary.h"
#endif

#if LLVM_VERSION <= 209
#include "llvm/Target/TargetSelect.h"
#include "llvm/Target/TargetRegistry.h"
#else
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/TargetRegistry.h"
#endif
#if LLVM_VERSION <= 208
#include "llvm/System/Host.h"
#else
#include "llvm/Support/Host.h"
#endif
#include "llvm/Support/MemoryBuffer.h"
#if LLVM_VERSION > 208
#include "llvm/Support/system_error.h"
#endif
#include "llvm/Bitcode/ReaderWriter.h"
#if LLVM_VERSION <= 301
#include "llvm/Target/TargetData.h"
#endif
#include "llvm/Target/TargetMachine.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/Triple.h"

#include <iostream>

#undef HAVE_SYS_TYPES_H
#undef HAVE_SYS_STAT_H
#undef HAVE_UNISTD_H
#undef HAVE_SYS_TIME_H
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_VERSION
#include "../../../include/konoha3/konoha.h"
#include "../../../include/konoha3/sugar.h"
#include "../../../include/konoha3/konoha_common.h"
#include "../../../include/konoha3/import/methoddecl.h"
#include <stdio.h>

struct kRawPtr {
	kObjectHeader h;
	void *rawptr;
};

#define MOD_llvm 19/*TODO*/
#define kmodllvm ((kmodllvm_t *)kctx->modshare[MOD_llvm])
#define KClass_Value (kmodllvm)->cValue
#define KType_Value (KClass_Value)->typeId
#if LLVM_VERSION <= 209
#define DEPRICATE_API(MSG) do {\
    (void)kctx;(void)sfp;\
    fprintf(stderr, "FIXME " MSG "\n");\
    abort();\
} while(0)
#endif

typedef struct {
	KRuntimeModule h;
	KClass *cValue;
} kmodllvm_t;

typedef struct {
	KContextModule h;
} kllvmmod_t;

namespace konoha {
template <class T>
inline T object_cast(kObject *po)
{
	kObjectVar *o_ = const_cast<kObjectVar*>(po);
	kRawPtr *o = reinterpret_cast<kRawPtr*>(o_);
	return static_cast<T>(o->rawptr);
}

template <class T>
inline void convert_array(std::vector<T> &vec, kArray *a)
{
	size_t size = kArray_size(a);
	for (size_t i=0; i < size; i++) {
		T v = konoha::object_cast<T>(a->ObjectItems[i]);
		vec.push_back(v);
	}
}

inline void SetRawPtr(kObject *po, void *rawptr)
{
	kObjectVar *o_ = const_cast<kObjectVar*>(po);
	kRawPtr *o = reinterpret_cast<kRawPtr*>(o_);
	o->rawptr = rawptr;
}
}

#if LLVM_VERSION <= 209
#define _ITERATOR(ITR) (ITR).begin(), (ITR).end()
#else
#define _ITERATOR(ITR) (ITR)
#endif

#if LLVM_VERSION <= 209
#define CONST_CAST(T, V) (const_cast<T>(V))
#else
#define CONST_CAST(T, V) (V)
#endif

using namespace llvm;

#if LLVM_VERSION <= 209
typedef const Type LLVMTYPE;
#else
typedef Type LLVMTYPE;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define _UNUSED_ __attribute__((unused))

#define WRAP(ptr) ((void *)ptr)
#define Int_to(T, a)               ((T)a.intValue)

static void Type_init(KonohaContext *kctx _UNUSED_, kObject *po, void *conf)
{
	konoha::SetRawPtr(po, conf);
}
static void Type_free(KonohaContext *kctx _UNUSED_, kObject *po)
{
	konoha::SetRawPtr(po, NULL);
}

static inline kObject *new_CppObject(KonohaContext *kctx, const KClass *ct, void *ptr)
{
	kObject *ret = KLIB new_kObject(kctx, OnStack, ct, (uintptr_t)ptr);
	konoha::SetRawPtr(ret, ptr);
	return ret;
}

static inline kObject *new_ReturnCppObject(KonohaContext *kctx, KonohaStack *sfp, void *ptr)
{
	kObject *defobj = sfp[(-(K_CALLDELTA))].asObject;
	kObject *ret = KLIB new_kObject(kctx, OnStack, kObject_class(defobj), (uintptr_t)ptr);
	konoha::SetRawPtr(ret, ptr);
	return ret;
}

//## @Const method Boolean Type.opEQ(Type value);
static KMETHOD Type_opEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	Type *p1 = konoha::object_cast<Type *>(sfp[0].asObject);
	Type *p2 = konoha::object_cast<Type *>(sfp[1].asObject);
	KReturnUnboxValue(p1 == p2);
}

//## @Static Type Type.getVoidTy();
static KMETHOD Type_getVoidTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getVoidTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static Type Type.getLabelTy();
static KMETHOD Type_getLabelTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getLabelTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static Type Type.getFloatTy();
static KMETHOD Type_getFloatTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getFloatTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static Type Type.getDoubleTy();
static KMETHOD Type_getDoubleTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getDoubleTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static Type Type.getMetadataTy();
static KMETHOD Type_getMetadataTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getMetadataTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static Type Type.getX86FP80Ty();
static KMETHOD Type_getX86FP80Ty(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getX86_FP80Ty(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static Type Type.getFP128Ty();
static KMETHOD Type_getFP128Ty(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getFP128Ty(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static Type Type.getPPCFP128Ty();
static KMETHOD Type_getPPCFP128Ty(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getPPC_FP128Ty(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static Type Type.getX86MMXTy();
static KMETHOD Type_getX86MMXTy(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;
	DEPRICATE_API("NO SUPPORT");
#else
	const Type *ptr = Type::getX86_MMXTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
#endif
}

//## @Static IntegerType Type.getInt1Ty();
static KMETHOD Type_getInt1Ty(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getInt1Ty(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static IntegerType Type.getInt8Ty();
static KMETHOD Type_getInt8Ty(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getInt8Ty(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static IntegerType Type.getInt16Ty();
static KMETHOD Type_getInt16Ty(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getInt16Ty(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static IntegerType Type.getInt32Ty();
static KMETHOD Type_getInt32Ty(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getInt32Ty(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static IntegerType Type.getInt64Ty();
static KMETHOD Type_getInt64Ty(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getInt64Ty(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static PointerType Type.getFloatPtrTy();
static KMETHOD Type_getFloatPtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getFloatPtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static PointerType Type.getDoublePtrTy();
static KMETHOD Type_getDoublePtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getDoublePtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static PointerType Type.getX86FP80PtrTy();
static KMETHOD Type_getX86FP80PtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getX86_FP80PtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static PointerType Type.getFP128PtrTy();
static KMETHOD Type_getFP128PtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getFP128PtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static PointerType Type.getPPCFP128PtrTy();
static KMETHOD Type_getPPCFP128PtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getPPC_FP128PtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static PointerType Type.getX86MMXPtrTy();
static KMETHOD Type_getX86MMXPtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;
	DEPRICATE_API("NO SUPPORT");
#else
	const Type *ptr = Type::getX86_MMXPtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
#endif
}

//## @Static PointerType Type.getInt1PtrTy();
static KMETHOD Type_getInt1PtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getInt1PtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static PointerType Type.getInt8PtrTy();
static KMETHOD Type_getInt8PtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getInt8PtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static PointerType Type.getInt16PtrTy();
static KMETHOD Type_getInt16PtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getInt16PtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static PointerType Type.getInt32PtrTy();
static KMETHOD Type_getInt32PtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getInt32PtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static PointerType Type.getInt64PtrTy();
static KMETHOD Type_getInt64PtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getInt64PtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static PointerType PointerType.get(Type type);
static KMETHOD PointerType_get(KonohaContext *kctx, KonohaStack *sfp)
{
	Type *type = konoha::object_cast<Type *>(sfp[1].asObject);
	const Type *ptr  = PointerType::get(type, 0);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## IRBuilder IRBuilder.new(BasicBlock bb);
static KMETHOD IRBuilder_new(KonohaContext *kctx, KonohaStack *sfp)
{
	BasicBlock *bb = konoha::object_cast<BasicBlock *>(sfp[1].asObject);
	IRBuilder<> *self = new IRBuilder<>(bb);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(self));
	KReturn(p);
}

//## ReturnInst IRBuilder.CreateRetVoid();
static KMETHOD IRBuilder_createRetVoid(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	ReturnInst *ptr = self->CreateRetVoid();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## ReturnInst IRBuilder.CreateRet(Value V);
static KMETHOD IRBuilder_createRet(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	ReturnInst *ptr = self->CreateRet(V);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

////## ReturnInst IRBuilder.CreateAggregateRet(Value retVals, int N);
//KMETHOD IRBuilder_createAggregateRet(KonohaContext *kctx, KonohaStack *sfp)
//{
//	DEPRICATE_API("NO SUPPORT");
//	//IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
//	//Value *const retVals = konoha::object_cast<Value *const>(sfp[1].asObject);
//	//kint_t N = Int_to(kint_t,sfp[2]);
//	//ReturnInst *ptr = self->CreateAggregateRet(retVals, N);
//	//kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	//KReturn(p);
//}

//## BranchInst IRBuilder.CreateBr(BasicBlock Dest);
static KMETHOD IRBuilder_createBr(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	BasicBlock *Dest = konoha::object_cast<BasicBlock *>(sfp[1].asObject);
	BranchInst *ptr = self->CreateBr(Dest);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## BranchInst IRBuilder.CreateCondBr(Value Cond, BasicBlock True, BasicBlock False);
static KMETHOD IRBuilder_createCondBr(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Cond = konoha::object_cast<Value *>(sfp[1].asObject);
	BasicBlock *True = konoha::object_cast<BasicBlock *>(sfp[2].asObject);
	BasicBlock *False = konoha::object_cast<BasicBlock *>(sfp[3].asObject);
	BranchInst *ptr = self->CreateCondBr(Cond, True, False);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## SwitchInst IRBuilder.CreateSwitch(Value V, BasicBlock Dest);
static KMETHOD IRBuilder_createSwitch(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	BasicBlock *Dest = konoha::object_cast<BasicBlock *>(sfp[2].asObject);
	SwitchInst *ptr = self->CreateSwitch(V, Dest);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## IndirectBrInst IRBuilder.CreateIndirectBr(Value Addr);
static KMETHOD IRBuilder_createIndirectBr(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Addr = konoha::object_cast<Value *>(sfp[1].asObject);
	IndirectBrInst *ptr = self->CreateIndirectBr(Addr);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## InvokeInst IRBuilder.CreateInvoke0(Value Callee, BasicBlock NormalDest, BasicBlock UnwindDest);
static KMETHOD IRBuilder_createInvoke0(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Callee = konoha::object_cast<Value *>(sfp[1].asObject);
	BasicBlock *NormalDest = konoha::object_cast<BasicBlock *>(sfp[2].asObject);
	BasicBlock *UnwindDest = konoha::object_cast<BasicBlock *>(sfp[3].asObject);
	InvokeInst *ptr = self->CreateInvoke(Callee, NormalDest, UnwindDest);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## InvokeInst IRBuilder.CreateInvoke1(Value Callee, BasicBlock NormalDest, BasicBlock UnwindDest, Value Arg1);
static KMETHOD IRBuilder_createInvoke1(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Callee = konoha::object_cast<Value *>(sfp[1].asObject);
	BasicBlock *NormalDest = konoha::object_cast<BasicBlock *>(sfp[2].asObject);
	BasicBlock *UnwindDest = konoha::object_cast<BasicBlock *>(sfp[3].asObject);
	Value *Arg1 = konoha::object_cast<Value *>(sfp[4].asObject);
	InvokeInst *ptr = self->CreateInvoke(Callee, NormalDest, UnwindDest, Arg1);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## InvokeInst IRBuilder.CreateInvoke3(Value Callee, BasicBlock NormalDest, BasicBlock UnwindDest, Value Arg1, Value Arg2, Value Arg3);
static KMETHOD IRBuilder_createInvoke3(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Callee = konoha::object_cast<Value *>(sfp[1].asObject);
	BasicBlock *NormalDest = konoha::object_cast<BasicBlock *>(sfp[2].asObject);
	BasicBlock *UnwindDest = konoha::object_cast<BasicBlock *>(sfp[3].asObject);
	Value *Arg1 = konoha::object_cast<Value *>(sfp[4].asObject);
	Value *Arg2 = konoha::object_cast<Value *>(sfp[5].asObject);
	Value *Arg3 = konoha::object_cast<Value *>(sfp[6].asObject);
	InvokeInst *ptr = self->CreateInvoke3(Callee, NormalDest, UnwindDest, Arg1, Arg2, Arg3);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

////## InvokeInst IRBuilder.CreateInvoke(Value Callee, BasicBlock NormalDest, BasicBlock UnwindDest, ArrayRef<Value> Args);
//KMETHOD IRBuilder_createInvoke(KonohaContext *kctx, KonohaStack *sfp)
//{
//	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
//	Value *Callee = konoha::object_cast<Value *>(sfp[1].asObject);
//	BasicBlock *NormalDest = konoha::object_cast<BasicBlock *>(sfp[2].asObject);
//	BasicBlock *UnwindDest = konoha::object_cast<BasicBlock *>(sfp[3].asObject);
//	kArray *Args = (sfp[4].asArray);
//	std::vector<Value*> List;
//	konoha::convert_array(List, Args);
//	InvokeInst *ptr = self->CreateInvoke(Callee, NormalDest, UnwindDest, List.begin(), List.end());
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	KReturn(p);
//}

////## ResumeInst IRBuilder.CreateResume(Value Exn);
//KMETHOD IRBuilder_createResume(KonohaContext *kctx, KonohaStack *sfp)
//{
//	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
//	Value *Exn = konoha::object_cast<Value *>(sfp[1].asObject);
//	ResumeInst *ptr = self->CreateResume(Exn);
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	KReturn(p);
//}
//
//## UnreachableInst IRBuilder.CreateUnreachable();
static KMETHOD IRBuilder_createUnreachable(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	UnreachableInst *ptr = self->CreateUnreachable();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateAdd(Value LHS, Value RHS);
static KMETHOD IRBuilder_createAdd(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateAdd(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateNSWAdd(Value LHS, Value RHS);
static KMETHOD IRBuilder_createNSWAdd(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateNSWAdd(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateNUWAdd(Value LHS, Value RHS);
static KMETHOD IRBuilder_createNUWAdd(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateNUWAdd(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFAdd(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFAdd(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateFAdd(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateSub(Value LHS, Value RHS);
static KMETHOD IRBuilder_createSub(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateSub(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateNSWSub(Value LHS, Value RHS);
static KMETHOD IRBuilder_createNSWSub(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateNSWSub(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateNUWSub(Value LHS, Value RHS);
static KMETHOD IRBuilder_createNUWSub(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateNUWSub(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFSub(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFSub(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateFSub(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateMul(Value LHS, Value RHS);
static KMETHOD IRBuilder_createMul(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateMul(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateNSWMul(Value LHS, Value RHS);
static KMETHOD IRBuilder_createNSWMul(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateNSWMul(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateNUWMul(Value LHS, Value RHS);
static KMETHOD IRBuilder_createNUWMul(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateNUWMul(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFMul(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFMul(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateFMul(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateUDiv(Value LHS, Value RHS);
static KMETHOD IRBuilder_createUDiv(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateUDiv(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateExactUDiv(Value LHS, Value RHS);
static KMETHOD IRBuilder_createExactUDiv(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;
	DEPRICATE_API("NO SUPPORT");
#else
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateExactUDiv(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
#endif
}

//## Value IRBuilder.CreateSDiv(Value LHS, Value RHS);
static KMETHOD IRBuilder_createSDiv(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateSDiv(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateExactSDiv(Value LHS, Value RHS);
static KMETHOD IRBuilder_createExactSDiv(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateExactSDiv(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFDiv(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFDiv(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateFDiv(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateURem(Value LHS, Value RHS);
static KMETHOD IRBuilder_createURem(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateURem(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateSRem(Value LHS, Value RHS);
static KMETHOD IRBuilder_createSRem(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateSRem(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFRem(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFRem(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateFRem(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateShl(Value LHS, Value RHS);
static KMETHOD IRBuilder_createShl(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateShl(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateLShr(Value LHS, Value RHS);
static KMETHOD IRBuilder_createLShr(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateLShr(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateAShr(Value LHS, Value RHS);
static KMETHOD IRBuilder_createAShr(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateAShr(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateAnd(Value LHS, Value RHS);
static KMETHOD IRBuilder_createAnd(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateAnd(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateOr(Value LHS, Value RHS);
static KMETHOD IRBuilder_createOr(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateOr(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateXor(Value LHS, Value RHS);
static KMETHOD IRBuilder_createXor(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateXor(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateNeg(Value V);
static KMETHOD IRBuilder_createNeg(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *ptr = self->CreateNeg(V);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateNSWNeg(Value V);
static KMETHOD IRBuilder_createNSWNeg(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *ptr = self->CreateNSWNeg(V);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateNUWNeg(Value V);
static KMETHOD IRBuilder_createNUWNeg(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *ptr = self->CreateNUWNeg(V);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFNeg(Value V);
static KMETHOD IRBuilder_createFNeg(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *ptr = self->CreateFNeg(V);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateNot(Value V);
static KMETHOD IRBuilder_createNot(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *ptr = self->CreateNot(V);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## AllocaInst IRBuilder.CreateAlloca(Type Ty, Value ArraySize);
static KMETHOD IRBuilder_createAlloca(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Type *Ty = konoha::object_cast<Type *>(sfp[1].asObject);
	Value *ArraySize = konoha::object_cast<Value *>(sfp[2].asObject);
	AllocaInst *ptr = self->CreateAlloca(Ty, ArraySize);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## LoadInst IRBuilder.CreateLoad(Value Ptr, boolean isVolatile);
static KMETHOD IRBuilder_createLoad(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	kbool_t isVolatile = sfp[2].boolValue;
	LoadInst *ptr = self->CreateLoad(Ptr, isVolatile);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//@Native LoadInst LoadInst.new(Value ptr);
//## LoadInst IRBuilder.CreateLoad(Value Ptr, boolean isVolatile);
static KMETHOD LoadInst_new(KonohaContext *kctx, KonohaStack *sfp)
{
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	LoadInst *ptr = new LoadInst(Ptr);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## StoreInst IRBuilder.CreateStore(Value Val, Value Ptr, boolean isVolatile);
static KMETHOD IRBuilder_createStore(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Val = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[2].asObject);
	kbool_t isVolatile = sfp[3].boolValue;
	StoreInst *ptr = self->CreateStore(Val, Ptr, isVolatile);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

////## FenceInst IRBuilder.CreateFence(AtomicOrdering Ordering, SynchronizationScope SynchScope);
//KMETHOD IRBuilder_createFence(KonohaContext *kctx, KonohaStack *sfp)
//{
//	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
//	AtomicOrdering *Ordering = konoha::object_cast<AtomicOrdering *>(sfp[1].asObject);
//	SynchronizationScope *SynchScope = konoha::object_cast<SynchronizationScope *>(sfp[2].asObject);
//	FenceInst *ptr = self->CreateFence(Ordering, SynchScope);
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	KReturn(p);
//}
//
////## AtomicCmpXchgInst IRBuilder.CreateAtomicCmpXchg(Value Ptr, Value Cmp, Value New, AtomicOrdering Ordering, SynchronizationScope SynchScope);
//KMETHOD IRBuilder_createAtomicCmpXchg(KonohaContext *kctx, KonohaStack *sfp)
//{
//	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
//	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
//	Value *Cmp = konoha::object_cast<Value *>(sfp[2].asObject);
//	Value *New = konoha::object_cast<Value *>(sfp[3].asObject);
//	AtomicOrdering *Ordering = konoha::object_cast<AtomicOrdering *>(sfp[4].asObject);
//	SynchronizationScope *SynchScope = konoha::object_cast<SynchronizationScope *>(sfp[5].asObject);
//	AtomicCmpXchgInst *ptr = self->CreateAtomicCmpXchg(Ptr, Cmp, New, Ordering, SynchScope);
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	KReturn(p);
//}

//## @Native AllocaInst AllocaInst.new(Type ty, Value arraySize);
static KMETHOD AllocaInst_new(KonohaContext *kctx, KonohaStack *sfp)
{
	Type *Ty = konoha::object_cast<Type *>(sfp[1].asObject);
	Value *ArraySize = konoha::object_cast<Value *>(sfp[2].asObject);
	AllocaInst *ptr = new AllocaInst(Ty, ArraySize);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Native StoreInst StoreInst.new(Value val, Value ptr);
static KMETHOD StoreInst_new(KonohaContext *kctx, KonohaStack *sfp)
{
	Value *Val = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[2].asObject);
	StoreInst *ptr = new StoreInst(Val, Ptr);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Native @Static GetElementPtrInst GetElementPtrInst.create(Value ptr, Array<Value> idxList);
static KMETHOD GetElementPtrInst_create(KonohaContext *kctx, KonohaStack *sfp)
{
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	kArray *IdxList = sfp[2].asArray;
	std::vector<Value*> List;
	konoha::convert_array(List, IdxList);

	GetElementPtrInst *ptr = GetElementPtrInst::Create(Ptr, _ITERATOR(List));
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Native @Static GetElementPtrInst GetElementPtrInst.CreateInBounds(Value ptr, Array<Value> idxList);
static KMETHOD GetElementPtrInst_createInBounds(KonohaContext *kctx, KonohaStack *sfp)
{
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	kArray *IdxList = sfp[2].asArray;
	std::vector<Value*> List;
	konoha::convert_array(List, IdxList);
	GetElementPtrInst *ptr = GetElementPtrInst::CreateInBounds(Ptr, _ITERATOR(List));
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateGEP(Value Ptr, ArrayRef< Value > IdxList);
static KMETHOD IRBuilder_createGEP(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	kArray *IdxList = sfp[2].asArray;
	std::vector<Value*> List;
	konoha::convert_array(List, IdxList);
	Value *ptr = self->CreateGEP(Ptr, _ITERATOR(List));
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateInBoundsGEP(Value Ptr, ArrayRef< Value > IdxList);
static KMETHOD IRBuilder_createInBoundsGEP(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	kArray *IdxList = sfp[2].asArray;
	std::vector<Value*> List;
	konoha::convert_array(List, IdxList);
	Value *ptr = self->CreateInBoundsGEP(Ptr, _ITERATOR(List));
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateGEP1(Value Ptr, Value Idx);
static KMETHOD IRBuilder_createGEP1(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *Idx = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateGEP(Ptr, Idx);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateInBoundsGEP1(Value Ptr, Value Idx);
static KMETHOD IRBuilder_createInBoundsGEP1(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *Idx = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateInBoundsGEP(Ptr, Idx);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateConstGEP1_32(Value Ptr, int Idx0);
static KMETHOD IRBuilder_createConstGEP132(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	kint_t Idx0 = Int_to(kint_t,sfp[2]);
	Value *ptr = self->CreateConstGEP1_32(Ptr, Idx0);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateConstInBoundsGEP1_32(Value Ptr, int Idx0);
static KMETHOD IRBuilder_createConstInBoundsGEP132(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	kint_t Idx0 = Int_to(kint_t,sfp[2]);
	Value *ptr = self->CreateConstInBoundsGEP1_32(Ptr, Idx0);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateConstGEP2_32(Value Ptr, int Idx0, int Idx1);
static KMETHOD IRBuilder_createConstGEP232(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	kint_t Idx0 = Int_to(kint_t,sfp[2]);
	kint_t Idx1 = Int_to(kint_t,sfp[3]);
	Value *ptr = self->CreateConstGEP2_32(Ptr, Idx0, Idx1);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateConstInBoundsGEP2_32(Value Ptr, int Idx0, int Idx1);
static KMETHOD IRBuilder_createConstInBoundsGEP232(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	kint_t Idx0 = Int_to(kint_t,sfp[2]);
	kint_t Idx1 = Int_to(kint_t,sfp[3]);
	Value *ptr = self->CreateConstInBoundsGEP2_32(Ptr, Idx0, Idx1);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateConstGEP1_64(Value Ptr, uint64_t Idx0);
static KMETHOD IRBuilder_createConstGEP164(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	kint_t Idx0 = sfp[2].intValue;
	Value *ptr = self->CreateConstGEP1_64(Ptr, Idx0);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateConstInBoundsGEP1_64(Value Ptr, uint64_t Idx0);
static KMETHOD IRBuilder_createConstInBoundsGEP164(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	kint_t Idx0 = sfp[2].intValue;
	Value *ptr = self->CreateConstInBoundsGEP1_64(Ptr, Idx0);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateConstGEP2_64(Value Ptr, uint64_t Idx0, uint64_t Idx1);
static KMETHOD IRBuilder_createConstGEP264(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	kint_t Idx0 = sfp[2].intValue;
	kint_t Idx1 = sfp[3].intValue;
	Value *ptr = self->CreateConstGEP2_64(Ptr, Idx0, Idx1);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateConstInBoundsGEP2_64(Value Ptr, uint64_t Idx0, uint64_t Idx1);
static KMETHOD IRBuilder_createConstInBoundsGEP264(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	kint_t Idx0 = sfp[2].intValue;
	kint_t Idx1 = sfp[3].intValue;
	Value *ptr = self->CreateConstInBoundsGEP2_64(Ptr, Idx0, Idx1);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateStructGEP(Value Ptr, int Idx);
static KMETHOD IRBuilder_createStructGEP(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	kint_t Idx = Int_to(kint_t,sfp[2]);
	Value *ptr = self->CreateStructGEP(Ptr, Idx, "gep");
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateGlobalString(StringRef Str);
static KMETHOD IRBuilder_createGlobalString(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	kString *Str = sfp[1].asString;
	Value *ptr = self->CreateGlobalString(kString_text(Str));
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateGlobalStringPtr(StringRef Str);
static KMETHOD IRBuilder_createGlobalStringPtr(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	kString *Str = sfp[1].asString;
	Value *ptr = self->CreateGlobalStringPtr(kString_text(Str));
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateTrunc(Value V, Type DestTy);
static KMETHOD IRBuilder_createTrunc(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].asObject);
	Value *ptr = self->CreateTrunc(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateZExt(Value V, Type DestTy);
static KMETHOD IRBuilder_createZExt(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].asObject);
	Value *ptr = self->CreateZExt(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateSExt(Value V, Type DestTy);
static KMETHOD IRBuilder_createSExt(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].asObject);
	Value *ptr = self->CreateSExt(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFPToUI(Value V, Type DestTy);
static KMETHOD IRBuilder_createFPToUI(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].asObject);
	Value *ptr = self->CreateFPToUI(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFPToSI(Value V, Type DestTy);
static KMETHOD IRBuilder_createFPToSI(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].asObject);
	Value *ptr = self->CreateFPToSI(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateUIToFP(Value V, Type DestTy);
static KMETHOD IRBuilder_createUIToFP(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].asObject);
	Value *ptr = self->CreateUIToFP(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateSIToFP(Value V, Type DestTy);
static KMETHOD IRBuilder_createSIToFP(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].asObject);
	Value *ptr = self->CreateSIToFP(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFPTrunc(Value V, Type DestTy);
static KMETHOD IRBuilder_createFPTrunc(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].asObject);
	Value *ptr = self->CreateFPTrunc(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFPExt(Value V, Type DestTy);
static KMETHOD IRBuilder_createFPExt(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].asObject);
	Value *ptr = self->CreateFPExt(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreatePtrToInt(Value V, Type DestTy);
static KMETHOD IRBuilder_createPtrToInt(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].asObject);
	Value *ptr = self->CreatePtrToInt(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateIntToPtr(Value V, Type DestTy);
static KMETHOD IRBuilder_createIntToPtr(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].asObject);
	Value *ptr = self->CreateIntToPtr(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateBitCast(Value V, Type DestTy);
static KMETHOD IRBuilder_createBitCast(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].asObject);
	Value *ptr = self->CreateBitCast(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateZExtOrBitCast(Value V, Type DestTy);
static KMETHOD IRBuilder_createZExtOrBitCast(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].asObject);
	Value *ptr = self->CreateZExtOrBitCast(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateSExtOrBitCast(Value V, Type DestTy);
static KMETHOD IRBuilder_createSExtOrBitCast(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].asObject);
	Value *ptr = self->CreateSExtOrBitCast(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateTruncOrBitCast(Value V, Type DestTy);
static KMETHOD IRBuilder_createTruncOrBitCast(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].asObject);
	Value *ptr = self->CreateTruncOrBitCast(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreatePointerCast(Value V, Type DestTy);
static KMETHOD IRBuilder_createPointerCast(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].asObject);
	Value *ptr = self->CreatePointerCast(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateIntCast(Value V, Type DestTy, boolean isSigned);
static KMETHOD IRBuilder_createIntCast(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].asObject);
	kbool_t isSigned = sfp[3].boolValue;
	Value *ptr = self->CreateIntCast(V, DestTy, isSigned);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFPCast(Value V, Type DestTy);
static KMETHOD IRBuilder_createFPCast(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].asObject);
	Value *ptr = self->CreateFPCast(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateICmpEQ(Value LHS, Value RHS);
static KMETHOD IRBuilder_createICmpEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateICmpEQ(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateICmpNE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createICmpNE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateICmpNE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateICmpUGT(Value LHS, Value RHS);
static KMETHOD IRBuilder_createICmpUGT(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateICmpUGT(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateICmpUGE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createICmpUGE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateICmpUGE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateICmpULT(Value LHS, Value RHS);
static KMETHOD IRBuilder_createICmpULT(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateICmpULT(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateICmpULE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createICmpULE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateICmpULE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateICmpSGT(Value LHS, Value RHS);
static KMETHOD IRBuilder_createICmpSGT(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateICmpSGT(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateICmpSGE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createICmpSGE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateICmpSGE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateICmpSLT(Value LHS, Value RHS);
static KMETHOD IRBuilder_createICmpSLT(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateICmpSLT(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateICmpSLE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createICmpSLE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateICmpSLE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFCmpOEQ(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpOEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateFCmpOEQ(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFCmpOGT(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpOGT(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateFCmpOGT(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFCmpOGE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpOGE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateFCmpOGE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFCmpOLT(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpOLT(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateFCmpOLT(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFCmpOLE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpOLE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateFCmpOLE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFCmpONE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpONE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateFCmpONE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFCmpORD(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpORD(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateFCmpORD(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFCmpUNO(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpUNO(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateFCmpUNO(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFCmpUEQ(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpUEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateFCmpUEQ(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFCmpUGT(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpUGT(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateFCmpUGT(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFCmpUGE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpUGE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateFCmpUGE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFCmpULT(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpULT(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateFCmpULT(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFCmpULE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpULE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateFCmpULE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateFCmpUNE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpUNE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateFCmpUNE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## PHINode IRBuilder.CreatePHI(Type Ty, int numReservedValues);
static KMETHOD IRBuilder_createPHI(KonohaContext *kctx, KonohaStack *sfp)
{
	PHINode *ptr;
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Type *Ty = konoha::object_cast<Type *>(sfp[1].asObject);
#if LLVM_VERSION <= 209
	ptr = self->CreatePHI(Ty, "");
#else
	kint_t num = sfp[2].intValue;
	ptr = self->CreatePHI(Ty, num);
#endif
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## void IRBuilder.addIncoming(Type Ty, BasicBlock bb);
static KMETHOD PHINode_addIncoming(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	PHINode *self = konoha::object_cast<PHINode *>(sfp[0].asObject);
	Value *v = konoha::object_cast<Value *>(sfp[1].asObject);
	BasicBlock *bb = konoha::object_cast<BasicBlock *>(sfp[2].asObject);
	self->addIncoming(v, bb);
	KReturnVoid();
}

//## CallInst IRBuilder.CreateCall1(Value Callee, Value Arg);
static KMETHOD IRBuilder_createCall1(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Callee = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *Arg = konoha::object_cast<Value *>(sfp[2].asObject);
	CallInst *ptr = self->CreateCall(Callee, Arg);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## CallInst IRBuilder.CreateCall2(Value Callee, Value Arg1, Value Arg2);
static KMETHOD IRBuilder_createCall2(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Callee = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *Arg1 = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *Arg2 = konoha::object_cast<Value *>(sfp[3].asObject);
	CallInst *ptr = self->CreateCall2(Callee, Arg1, Arg2);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## CallInst IRBuilder.CreateCall3(Value Callee, Value Arg1, Value Arg2, Value Arg3);
static KMETHOD IRBuilder_createCall3(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Callee = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *Arg1 = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *Arg2 = konoha::object_cast<Value *>(sfp[3].asObject);
	Value *Arg3 = konoha::object_cast<Value *>(sfp[4].asObject);
	CallInst *ptr = self->CreateCall3(Callee, Arg1, Arg2, Arg3);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## CallInst IRBuilder.CreateCall4(Value Callee, Value Arg1, Value Arg2, Value Arg3, Value Arg4);
static KMETHOD IRBuilder_createCall4(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Callee = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *Arg1 = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *Arg2 = konoha::object_cast<Value *>(sfp[3].asObject);
	Value *Arg3 = konoha::object_cast<Value *>(sfp[4].asObject);
	Value *Arg4 = konoha::object_cast<Value *>(sfp[5].asObject);
	CallInst *ptr = self->CreateCall4(Callee, Arg1, Arg2, Arg3, Arg4);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## CallInst IRBuilder.CreateCall5(Value Callee, Value Arg1, Value Arg2, Value Arg3, Value Arg4, Value Arg5);
static KMETHOD IRBuilder_createCall5(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Callee = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *Arg1 = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *Arg2 = konoha::object_cast<Value *>(sfp[3].asObject);
	Value *Arg3 = konoha::object_cast<Value *>(sfp[4].asObject);
	Value *Arg4 = konoha::object_cast<Value *>(sfp[5].asObject);
	Value *Arg5 = konoha::object_cast<Value *>(sfp[6].asObject);
	CallInst *ptr = self->CreateCall5(Callee, Arg1, Arg2, Arg3, Arg4, Arg5);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## CallInst IRBuilder.CreateCall(Value Callee, ArrayRef< Value > Args);
static KMETHOD IRBuilder_createCall(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Callee = konoha::object_cast<Value *>(sfp[1].asObject);
	kArray *Args = sfp[2].asArray;
	std::vector<Value*> List;
	konoha::convert_array(List, Args);
	CallInst *ptr = self->CreateCall(Callee, _ITERATOR(List));
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateSelect(Value C, Value True, Value False);
static KMETHOD IRBuilder_createSelect(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *C = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *True = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *False = konoha::object_cast<Value *>(sfp[3].asObject);
	Value *ptr = self->CreateSelect(C, True, False);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## VAArgInst IRBuilder.CreateVAArg(Value List, Type Ty);
static KMETHOD IRBuilder_createVAArg(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *List = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *Ty = konoha::object_cast<Type *>(sfp[2].asObject);
	VAArgInst *ptr = self->CreateVAArg(List, Ty);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateExtractElement(Value Vec, Value Idx);
static KMETHOD IRBuilder_createExtractElement(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Vec = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *Idx = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreateExtractElement(Vec, Idx);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateInsertElement(Value Vec, Value NewElt, Value Idx);
static KMETHOD IRBuilder_createInsertElement(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Vec = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *NewElt = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *Idx = konoha::object_cast<Value *>(sfp[3].asObject);
	Value *ptr = self->CreateInsertElement(Vec, NewElt, Idx);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateShuffleVector(Value V1, Value V2, Value Mask);
static KMETHOD IRBuilder_createShuffleVector(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V1 = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *V2 = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *Mask = konoha::object_cast<Value *>(sfp[3].asObject);
	Value *ptr = self->CreateShuffleVector(V1, V2, Mask);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

////## Value IRBuilder.CreateExtractValue(Value Agg, Array<int> Idxs);
//KMETHOD IRBuilder_createExtractValue(KonohaContext *kctx, KonohaStack *sfp)
//{
//	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
//	Value *Agg = konoha::object_cast<Value *>(sfp[1].asObject);
//	kArray *Idxs = sfp[2].asArray;
//	std::vector<int> List;
//	konoha::convert_array_int(List, Idxs);
//	Value *ptr = self->CreateExtractValue(Agg, List.begin(), List.end());
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	KReturn(p);
//}

////## Value IRBuilder.CreateInsertValue(Value Agg, Value Val, Array<int> Idxs);
//KMETHOD IRBuilder_createInsertValue(KonohaContext *kctx, KonohaStack *sfp)
//{
//	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
//	Value *Agg = konoha::object_cast<Value *>(sfp[1].asObject);
//	Value *Val = konoha::object_cast<Value *>(sfp[2].asObject);
//	kArray *Idxs = sfp[2].asArray;
//	std::vector<int> List;
//	konoha::convert_array_int(List, Idxs);
//	Value *ptr = self->CreateInsertValue(Agg, Val, List.begin(), List.end());
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	KReturn(p);
//}

//## Value IRBuilder.CreateIsNull(Value Arg);
static KMETHOD IRBuilder_createIsNull(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Arg = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *ptr = self->CreateIsNull(Arg);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreateIsNotNull(Value Arg);
static KMETHOD IRBuilder_createIsNotNull(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Arg = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *ptr = self->CreateIsNotNull(Arg);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Value IRBuilder.CreatePtrDiff(Value LHS, Value RHS);
static KMETHOD IRBuilder_createPtrDiff(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].asObject);
	Value *ptr = self->CreatePtrDiff(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## void IRBuilder.SetInsertPoint(BasicBlock BB);
static KMETHOD IRBuilder_SetInsertPoint(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	BasicBlock * BB = konoha::object_cast<BasicBlock *>(sfp[1].asObject);
	self->SetInsertPoint(BB);
	KReturnVoid();
}

//## BasicBlock IRBuilder.GetInsertBlock();
static KMETHOD IRBuilder_getInsertBlock(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	BasicBlock *BB = self->GetInsertBlock();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(BB));
	KReturn(p);
}

//## Function BasicBlock.getParent();
static KMETHOD BasicBlock_getParent(KonohaContext *kctx, KonohaStack *sfp)
{
	BasicBlock *self = konoha::object_cast<BasicBlock *>(sfp[0].asObject);
	Function *ptr = self->getParent();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Instruction BasicBlock.getTerminator();
static KMETHOD BasicBlock_getTerminator(KonohaContext *kctx, KonohaStack *sfp)
{
	BasicBlock *self = konoha::object_cast<BasicBlock *>(sfp[0].asObject);
	TerminatorInst *ptr = self->getTerminator();
	if(ptr) {
		kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
		KReturn(p);
	} else {
		KReturn(K_NULL);
	}
}


////## iterator BasicBlock.begin();
//KMETHOD BasicBlock_begin(KonohaContext *kctx, KonohaStack *sfp)
//{
//	BasicBlock *self = konoha::object_cast<BasicBlock *>(sfp[0].asObject);
//	*ptr = self->Create();
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	KReturn(K_NULL);
//}
//
////## iterator BasicBlock.end();
//KMETHOD BasicBlock_end(KonohaContext *kctx, KonohaStack *sfp)
//{
//	BasicBlock *self = konoha::object_cast<BasicBlock *>(sfp[0].asObject);
//	*ptr = self->Create();
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	KReturn(K_NULL);
//}

//## Instruction BasicBlock.getLastInst();
static KMETHOD BasicBlock_getLastInst(KonohaContext *kctx, KonohaStack *sfp)
{
	BasicBlock *self = konoha::object_cast<BasicBlock *>(sfp[0].asObject);
	BasicBlock::iterator I = self->end();
	Instruction *ptr;
	if(self->size() > 0)
		--I;
	ptr = I;
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Instruction BasicBlock.insertBefore(Instruction before, Instruction inst);
static KMETHOD BasicBlock_insertBefore(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	BasicBlock *self = konoha::object_cast<BasicBlock *>(sfp[0].asObject);
	Instruction *inst0 = konoha::object_cast<Instruction *>(sfp[1].asObject);
	Instruction *inst1 = konoha::object_cast<Instruction *>(sfp[2].asObject);
	self->getInstList().insert(inst0, inst1);
	KReturnVoid();
}

//## int BasicBlock.size();
static KMETHOD BasicBlock_size(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	BasicBlock *self = konoha::object_cast<BasicBlock *>(sfp[0].asObject);
	int ret = self->size();
	KReturnUnboxValue(ret);
}

//## boolean BasicBlock.empty();
static KMETHOD BasicBlock_empty(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	BasicBlock *self = konoha::object_cast<BasicBlock *>(sfp[0].asObject);
	bool isEmpty = self->empty();
	KReturnUnboxValue(isEmpty);
}

//## Argument Argument.new(Type ty, int scid);
static KMETHOD Argument_new(KonohaContext *kctx, KonohaStack *sfp)
{
	Type *ty = konoha::object_cast<Type *>(sfp[1].asObject);
	Value *v = new Argument(ty, "", 0);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(v));
	KReturn(p);
}

//static void str_replace (std::string& str, const std::string& from, const std::string& to) {
//	std::string::size_type pos = 0;
//	while((pos = str.find(from, pos)) != std::string::npos) {
//		str.replace( pos, from.size(), to );
//		pos++;
//	}
//}
//
//## Module Module.new(String name);
static KMETHOD Module_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *name = sfp[1].asString;
	LLVMContext &Context = getGlobalContext();
	Module *M = new Module(kString_text(name), Context);
#if 0
	Triple T(sys::getDefaultTargetTriple());
	const Target *Target = 0;
	std::string Arch = T.getArchName();
	for (TargetRegistry::iterator it = TargetRegistry::begin(),
			ie = TargetRegistry::end(); it != ie; ++it) {
		std::string tmp(it->getName());
		str_replace(tmp, "-", "_");
		if(Arch == tmp) {
			Target = &*it;
			break;
		}
	}
	assert(Target != 0);
	std::string FeaturesStr;
	TargetOptions Options;
	TargetMachine *TM = Target->createTargetMachine(T.getTriple(), Target->getName(), FeaturesStr, Options);
	M->setTargetTriple(T.getTriple());
	M->setDataLayout(TM->getTargetData()->getStringRepresentation());
#endif
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(M));
	KReturn(p);
}

//## void Module.dump();
static KMETHOD Module_dump(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	Module *self = konoha::object_cast<Module *>(sfp[0].asObject);
	(*self).dump();
	KReturnVoid();
}

//## Type Module.getTypeByName(String name);
static KMETHOD Module_getTypeByName(KonohaContext *kctx, KonohaStack *sfp)
{
	Module *self = konoha::object_cast<Module *>(sfp[0].asObject);
	kString *name = sfp[1].asString;
	Type *ptr = CONST_CAST(Type*, self->getTypeByName(kString_text(name)));
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## void BasicBlock.dump();
static KMETHOD BasicBlock_dump(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	BasicBlock *self = konoha::object_cast<BasicBlock *>(sfp[0].asObject);
	(*self).dump();
	KReturnVoid();
}

//## Function Module.getOrInsertFunction(String name, FunctionType fnTy);
static KMETHOD Module_getOrInsertFunction(KonohaContext *kctx, KonohaStack *sfp)
{
	Module *self = konoha::object_cast<Module *>(sfp[0].asObject);
	kString *name = sfp[1].asString;
	FunctionType *fnTy = konoha::object_cast<FunctionType *>(sfp[2].asObject);
	Function *ptr = cast<Function>(self->getOrInsertFunction(kString_text(name), fnTy));
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static @Native Function Function.create(String name, FunctionType fnTy, Module m, Linkage linkage);
static KMETHOD Function_create(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *name = sfp[1].asString;
	FunctionType *fnTy = konoha::object_cast<FunctionType *>(sfp[2].asObject);
	Module *m = konoha::object_cast<Module *>(sfp[3].asObject);
	kint_t v = sfp[4].intValue;
	GlobalValue::LinkageTypes linkage = (GlobalValue::LinkageTypes) v;
	Function *ptr = Function::Create(fnTy, linkage, kString_text(name), m);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}
//## @Static @Native Type Function.getReturnType();
static KMETHOD Function_getReturnType(KonohaContext *kctx, KonohaStack *sfp)
{
	Function *F = konoha::object_cast<Function *>(sfp[0].asObject);
	LLVMTYPE *ptr = F->getReturnType();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Native void Function.dump();
static KMETHOD Function_dump(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	Function *func = konoha::object_cast<Function *>(sfp[0].asObject);
	func->dump();
	KReturnVoid();
}

//## @Native void Function.addFnAttr(Int attributes);
static KMETHOD Function_addFnAttr(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	Function *F = konoha::object_cast<Function *>(sfp[0].asObject);
#if LLVM_VERSION == 302
	Attributes::AttrVal N = (Attributes::AttrVal) sfp[1].intValue;
#else
	Attributes N = (Attributes) sfp[1].intValue;
#endif
	F->addFnAttr(N);
	KReturnVoid();
}

//## ExecutionEngine Module.createExecutionEngine(int optLevel);
static KMETHOD Module_createExecutionEngine(KonohaContext *kctx, KonohaStack *sfp)
{
	Module *self = konoha::object_cast<Module *>(sfp[0].asObject);
	CodeGenOpt::Level OptLevel = (CodeGenOpt::Level) sfp[1].intValue;
	ExecutionEngine *ptr = EngineBuilder(self).setEngineKind(EngineKind::JIT).setOptLevel(OptLevel).create();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

static int BasicBlock_compareTo(KonohaContext *kctx, kObject *p1, kObject *p2)
{
	BasicBlock *bb1 = konoha::object_cast<BasicBlock*>(p1);
	BasicBlock *bb2 = konoha::object_cast<BasicBlock*>(p2);
	return (bb1 != bb2);
}

//void defBasicBlock(KonohaContext *kctx _UNUSED_, ktypeattr_t cid _UNUSED_, kclassdef_t *cdef)
//{
//	cdef->name = "llvm::BasicBlock";
//	cdef->compareTo = BasicBlock_compareTo;
//}

//## @Static BasicBlock BasicBlock.create(Function parent, String name);
static KMETHOD BasicBlock_create(KonohaContext *kctx, KonohaStack *sfp)
{
	Function * parent = konoha::object_cast<Function *>(sfp[1].asObject);
	kString *name = sfp[2].asString;
	const char *bbname = "";
	if(IS_NOTNULL(name)) {
		bbname = kString_text(name);
	}
	BasicBlock *ptr = BasicBlock::Create(getGlobalContext(), bbname, parent);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static FunctionType.get(Type retTy, Array<Type> args, boolean b);
static KMETHOD FunctionType_get(KonohaContext *kctx, KonohaStack *sfp)
{
	Type *retTy = konoha::object_cast<Type *>(sfp[1].asObject);
	kArray * args = sfp[2].asArray;
	kbool_t b = sfp[3].boolValue;
#if LLVM_VERSION <= 209
	std::vector<const Type*> List;
#else
	std::vector<Type*> List;
#endif
	konoha::convert_array(List, args);
	FunctionType *ptr = FunctionType::get(retTy, List, b);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Native Value ConstantInt.get(Type type, int v);
static KMETHOD ConstantInt_get(KonohaContext *kctx, KonohaStack *sfp)
{
	Type *type  = konoha::object_cast<Type *>(sfp[1].asObject);
	kint_t v = sfp[2].intValue;
	Value *ptr = ConstantInt::get(type, v);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Native Value ConstantFP.get(Type type, float v);
static KMETHOD ConstantFP_get(KonohaContext *kctx, KonohaStack *sfp)
{
	Type *type  = konoha::object_cast<Type *>(sfp[1].asObject);
	kfloat_t v = sfp[2].floatValue;
	Value *ptr = ConstantFP::get(type, v);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static @Native Value ConstantPointerNull.get(Type type);
static KMETHOD ConstantPointerNull_get(KonohaContext *kctx, KonohaStack *sfp)
{
	PointerType *type  = konoha::object_cast<PointerType *>(sfp[1].asObject);
	Value *ptr = ConstantPointerNull::get(type);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static @Native Value ConstantStruct.get(Type type, Array<Constant> V);
static KMETHOD ConstantStruct_get(KonohaContext *kctx, KonohaStack *sfp)
{
	StructType *type  = konoha::object_cast<StructType *>(sfp[1].asObject);
	kArray *args = sfp[2].asArray;
	std::vector<Constant*> List;
	konoha::convert_array(List, args);
	Value *ptr = ConstantStruct::get(type, List);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static StructType.get(Array<Type> args, boolean isPacked);
static KMETHOD StructType_get(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *args = sfp[1].asArray;
	kbool_t isPacked = sfp[2].boolValue;
#if LLVM_VERSION <= 209
	std::vector<const Type*> List;
#else
	std::vector<Type*> List;
#endif
	konoha::convert_array(List, args);
	StructType *ptr = StructType::get(getGlobalContext(), List, isPacked);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Static @Native StructType.create(Array<Type> args, String name, boolean isPacked);
static KMETHOD StructType_create(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *args = sfp[1].asArray;
#if LLVM_VERSION > 209
	kString *name = sfp[2].asString;
#endif
	kbool_t isPacked = sfp[3].boolValue;
	StructType *ptr;
	if(IS_NULL(args)) {
#if LLVM_VERSION <= 209
		ptr = StructType::get(getGlobalContext());
#else
		ptr = StructType::create(getGlobalContext(), kString_text(name));
#endif
	} else if(kArray_size(args) == 0) {
#if LLVM_VERSION <= 209
		std::vector<const Type*> List;
		ptr = StructType::get(getGlobalContext(), List, isPacked);
#else
		std::vector<Type*> List;
		ptr = StructType::create(getGlobalContext(), kString_text(name));
		ptr->setBody(List, isPacked);
#endif
	} else {
#if LLVM_VERSION <= 209
		std::vector<const Type*> List;
		konoha::convert_array(List, args);
		ptr = StructType::get(getGlobalContext(), List, isPacked);
#else
		std::vector<Type*> List;
		konoha::convert_array(List, args);
		ptr = StructType::create(List, kString_text(name), isPacked);
#endif
	}
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Native @Static ArrayType ArrayType.get(Type t, int elemSize);
static KMETHOD ArrayType_get(KonohaContext *kctx, KonohaStack *sfp)
{
	Type *Ty = konoha::object_cast<Type *>(sfp[1].asObject);
	kint_t N = sfp[2].boolValue;
	ArrayType *ptr = ArrayType::get(Ty, N);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## @Native void StructType.setBody(Array<Type> args, boolean isPacked);
static KMETHOD StructType_SetBody(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
#if LLVM_VERSION <= 209
	(void)kctx;(void)sfp;
	DEPRICATE_API("NO SUPPORT");
#else
	StructType *type  = konoha::object_cast<StructType *>(sfp[0].asObject);
	kArray *args = sfp[1].asArray;
	kbool_t isPacked = sfp[2].boolValue;
	std::vector<Type*> List;
	konoha::convert_array(List, args);
	type->setBody(List, isPacked);
	KReturnVoid();
#endif
}

//## @Native boolean StructType.isOpaque();
static KMETHOD StructType_isOpaque(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	bool ret = false;
#if LLVM_VERSION <= 209
	DEPRICATE_API("NO SUPPORT");
#else
	StructType *type  = konoha::object_cast<StructType *>(sfp[0].asObject);
	ret = type->isOpaque();
#endif
	KReturnUnboxValue(ret);
}

//## NativeFunction ExecutionEngine.getPointerToFunction(Function func);
static KMETHOD ExecutionEngine_getPointerToFunction(KonohaContext *kctx, KonohaStack *sfp)
{
	ExecutionEngine *ee = konoha::object_cast<ExecutionEngine *>(sfp[0].asObject);
	Function *func = konoha::object_cast<Function *>(sfp[1].asObject);
	void *ptr = ee->getPointerToFunction(func);
	//kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturnUnboxValue((uintptr_t)ptr);
	(void)kctx;
}
//## @Native void ExecutionEngine.addGlobalMapping(GlobalVariable g, int addr);
static KMETHOD ExecutionEngine_addGlobalMapping(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	ExecutionEngine *ee = konoha::object_cast<ExecutionEngine *>(sfp[0].asObject);
	GlobalVariable *g   = konoha::object_cast<GlobalVariable *>(sfp[1].asObject);
	long addr = sfp[2].intValue;
	ee->addGlobalMapping(g, (void *)addr);
	KReturnVoid();
}
//## @Native GlobalVariable GlobalVariable.new(Module m, Type ty, Constant c, Linkage linkage, String name);
static KMETHOD GlobalVariable_new(KonohaContext *kctx, KonohaStack *sfp)
{
	Module *m     = konoha::object_cast<Module *>(sfp[1].asObject);
	Type *ty      = konoha::object_cast<Type *>(sfp[2].asObject);
	Constant *c   = konoha::object_cast<Constant *>(sfp[3].asObject);
	GlobalValue::LinkageTypes linkage = (GlobalValue::LinkageTypes) sfp[4].intValue;
	kString *name = sfp[5].asString;
	bool isConstant = (c) ? true : false;
	GlobalVariable *ptr = new GlobalVariable(*m, ty, isConstant, linkage, c, kString_text(name));
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

#if LLVM_VERSION >= 300
static void PassManagerBuilder_ptr_init(KonohaContext *kctx _UNUSED_, kObject *po, void *conf)
{
	konoha::SetRawPtr(po, conf);
}

static void PassManagerBuilder_ptr_free(KonohaContext *kctx _UNUSED_, kObject *po)
{
	PassManagerBuilder *o = konoha::object_cast<PassManagerBuilder *>(po);
	delete o;
}

static KMETHOD PassManagerBuilder_new(KonohaContext *kctx, KonohaStack *sfp)
{
	PassManagerBuilder *self = new PassManagerBuilder();
	self->OptLevel = 3;
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(self));
	KReturn(p);
}

static KMETHOD PassManagerBuilder_populateModulePassManager(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	PassManagerBuilder *self = konoha::object_cast<PassManagerBuilder *>(sfp[0].asObject);
	PassManager *manager = konoha::object_cast<PassManager *>(sfp[1].asObject);
	self->populateModulePassManager(*manager);
	KReturnVoid();
}
#endif

static void PassManager_ptr_init(KonohaContext *kctx _UNUSED_, kObject *po, void *conf)
{
	konoha::SetRawPtr(po, conf);
}

static void PassManager_ptr_free(KonohaContext *kctx _UNUSED_, kObject *po)
{
	PassManager *o = konoha::object_cast<PassManager *>(po);
	delete o;
}

//## PassManager PassManager.new()
static KMETHOD PassManager_new(KonohaContext *kctx, KonohaStack *sfp)
{
	PassManager *self = new PassManager();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(self));
	KReturn(p);
}

//## void PassManager.run(Function func)
static KMETHOD PassManager_run(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	PassManager *self = konoha::object_cast<PassManager *>(sfp[0].asObject);
	Module *m = konoha::object_cast<Module *>(sfp[1].asObject);
	self->run(*m);
	KReturnVoid();
}
//## void PassManager.add(Pass p)
static KMETHOD PassManager_addPass(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	PassManager *self = konoha::object_cast<PassManager *>(sfp[0].asObject);
	Pass *pass = konoha::object_cast<Pass *>(sfp[1].asObject);
	self->add(pass);
	KReturnVoid();
}
//## void PassManager.add(Pass p)
static KMETHOD PassManager_addImmutablePass(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	PassManager *self = konoha::object_cast<PassManager *>(sfp[0].asObject);
	ImmutablePass *pass = konoha::object_cast<ImmutablePass *>(sfp[1].asObject);
	self->add(pass);
	KReturnVoid();
}
//## void PassManager.addFunctionPass(Pass p)
static KMETHOD PassManager_addFunctionPass(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	PassManager *self = konoha::object_cast<PassManager *>(sfp[0].asObject);
	FunctionPass *pass = konoha::object_cast<FunctionPass *>(sfp[1].asObject);
	self->add(pass);
	KReturnVoid();
}
//## void PassManager.addModulePass(Pass p)
static KMETHOD PassManager_addModulePass(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	PassManager *self = konoha::object_cast<PassManager *>(sfp[0].asObject);
	ModulePass *pass = konoha::object_cast<ModulePass *>(sfp[1].asObject);
	self->add(pass);
	KReturnVoid();
}

static void FunctionPassManager_ptr_init(KonohaContext *kctx _UNUSED_, kObject *po, void *conf)
{
	konoha::SetRawPtr(po, conf);
}

static void FunctionPassManager_ptr_free(KonohaContext *kctx _UNUSED_, kObject *po)
{
	FunctionPassManager *o = konoha::object_cast<FunctionPassManager *>(po);
	delete o;
}

//## FunctionPassManager FunctionPassManager.new(Module m)
static KMETHOD FunctionPassManager_new(KonohaContext *kctx, KonohaStack *sfp)
{
	Module *m = konoha::object_cast<Module *>(sfp[1].asObject);
	FunctionPassManager *self = new FunctionPassManager(m);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(self));
	KReturn(p);
}
//## void FuncitonPassManager.add(Pass p)
static KMETHOD FunctionPassManager_add(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	FunctionPassManager *self = konoha::object_cast<FunctionPassManager *>(sfp[0].asObject);
	Pass *pass = konoha::object_cast<Pass *>(sfp[1].asObject);
	self->add(pass);
	KReturnVoid();
}
//## void FunctionPassManager.doInitialization()
static KMETHOD FunctionPassManager_doInitialization(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	FunctionPassManager *self = konoha::object_cast<FunctionPassManager *>(sfp[0].asObject);
	self->doInitialization();
	KReturnVoid();
}

//## void FunctionPassManager.run(Function func)
static KMETHOD FunctionPassManager_run(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	FunctionPassManager *self = konoha::object_cast<FunctionPassManager *>(sfp[0].asObject);
	Function *func = konoha::object_cast<Function *>(sfp[1].asObject);
	self->run(*func);
	KReturnVoid();
}

#if LLVM_VERSION < 302
//## TargetData ExecutionEngine.getTargetData();
static KMETHOD ExecutionEngine_getTargetData(KonohaContext *kctx, KonohaStack *sfp)
{
	ExecutionEngine *ee = konoha::object_cast<ExecutionEngine *>(sfp[0].asObject);
	TargetData *ptr = new TargetData(*(ee->getTargetData()));
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}
#endif

//## void Method.setFunction(NativeFunction func);
static KMETHOD kMethod_SetFunction(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = (kMethod *) sfp[0].asObject;
	kObject *po = sfp[1].asObject;
	union anyptr { void *ptr; KMethodFunc f;} ptr;
	ptr.ptr = konoha::object_cast<void*>(po);
	KLIB kMethod_SetFunc(kctx, mtd, ptr.f);
	KReturnVoid();
}

//## @Native Array<Value> Function.getArguments();
static KMETHOD Function_getArguments(KonohaContext *kctx, KonohaStack *sfp)
{
	Function *func = konoha::object_cast<Function *>(sfp[0].asObject);
	ktypeattr_t cid = KType_Value;
	/*FIXME Generics Array */
	//ktypeattr_t rtype = sfp[K_MTDIDX].mtdNC->pa->rtype;
	//ktypeattr_t cid = KClass_(rtype)->p1;
	kArray *a = new_(Array, 0, OnStack);
	for (Function::arg_iterator I = func->arg_begin(), E = func->arg_end();
			I != E; ++I) {
		Value *v = I;
		kObject *o = new_CppObject(kctx, KClass_(cid)/*"Value"*/, WRAP(v));
		KLIB kArray_Add(kctx, a, o);
	}
	KReturn(a);
}
//## void Value.replaceAllUsesWith(Value v);
static KMETHOD Value_replaceAllUsesWith(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	Value *self = konoha::object_cast<Value *>(sfp[0].asObject);
	Value *v = konoha::object_cast<Value *>(sfp[1].asObject);
	self->replaceAllUsesWith(v);
	KReturnVoid();
}
//## Value Value.setName(String name);
static KMETHOD Value_SetName(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	Value *self = konoha::object_cast<Value *>(sfp[0].asObject);
	kString *name = sfp[1].asString;
	self->setName(kString_text(name));
	KReturnVoid();
}
//## void LoadInst.setAlignment(int align);
static KMETHOD LoadInst_SetAlignment(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	LoadInst *self = konoha::object_cast<LoadInst *>(sfp[0].asObject);
	int align = sfp[1].intValue;
	self->setAlignment(align);
	KReturnVoid();
}
//## void StoreInst.setAlignment(int align);
static KMETHOD StoreInst_SetAlignment(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	StoreInst *self = konoha::object_cast<StoreInst *>(sfp[0].asObject);
	int align = sfp[1].intValue;
	self->setAlignment(align);
	KReturnVoid();
}
//## Type Value.getType();
static KMETHOD Value_getType(KonohaContext *kctx, KonohaStack *sfp)
{
	Value *self = konoha::object_cast<Value *>(sfp[0].asObject);
	const Type *ptr = self->getType();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## void Value.dump();
static KMETHOD Value_dump(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	Value *self = konoha::object_cast<Value *>(sfp[0].asObject);
	self->dump();
	KReturnVoid();
}

//## @Native void Type.dump();
static KMETHOD Type_dump(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	Type *type = konoha::object_cast<Type *>(sfp[0].asObject);
	type->dump();
	KReturnVoid();
}

//## @Static boolean DynamicLibrary.loadLibraryPermanently(String libname);
static KMETHOD DynamicLibrary_loadLibraryPermanently(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *libname = kString_text(sfp[1].asString);
	std::string ErrMsg;
	kbool_t ret;
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;
	DEPRICATE_API("NO SUPPORT");
	ret = LinkDynamicObject(libname, &ErrMsg);
#else
	ret = sys::DynamicLibrary::LoadLibraryPermanently(libname, &ErrMsg);
#endif
	if(ret == 0) {
		//TODO
		//KNH_NTRACE2(kctx, "LoadLibraryPermanently", K_FAILED, KNH_LDATA(LOG_s("libname", libname), LOG_msg(ErrMsg.c_str())));
	}
	KReturnUnboxValue(ret);
}

//## @Static Int DynamicLibrary.searchForAddressOfSymbol(String fname);
static KMETHOD DynamicLibrary_searchForAddressOfSymbol(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	const char *fname = kString_text(sfp[1].asString);
	kint_t ret = 0;
	void *symAddr = NULL;
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;(void)fname;
	DEPRICATE_API("NO SUPPORT");
	//symAddr = GetAddressOfSymbol(fname);
#else
	symAddr = sys::DynamicLibrary::SearchForAddressOfSymbol(fname);
#endif
	if(symAddr) {
		ret = reinterpret_cast<kint_t>(symAddr);
	}
	KReturnUnboxValue(ret);
}

//## FunctionPass LLVM.createDomPrinterPass();
static KMETHOD LLVM_createDomPrinterPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createDomPrinterPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createDomOnlyPrinterPass();
static KMETHOD LLVM_createDomOnlyPrinterPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createDomOnlyPrinterPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createDomViewerPass();
static KMETHOD LLVM_createDomViewerPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createDomViewerPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createDomOnlyViewerPass();
static KMETHOD LLVM_createDomOnlyViewerPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createDomOnlyViewerPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createPostDomPrinterPass();
static KMETHOD LLVM_createPostDomPrinterPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createPostDomPrinterPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createPostDomOnlyPrinterPass();
static KMETHOD LLVM_createPostDomOnlyPrinterPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createPostDomOnlyPrinterPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createPostDomViewerPass();
static KMETHOD LLVM_createPostDomViewerPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createPostDomViewerPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createPostDomOnlyViewerPass();
static KMETHOD LLVM_createPostDomOnlyViewerPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createPostDomOnlyViewerPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createGlobalsModRefPass();
static KMETHOD LLVM_createGlobalsModRefPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createGlobalsModRefPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createAliasDebugger();
static KMETHOD LLVM_createAliasDebugger(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createAliasDebugger();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## ModulePass LLVM.createAliasAnalysisCounterPass();
static KMETHOD LLVM_createAliasAnalysisCounterPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createAliasAnalysisCounterPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createAAEvalPass();
static KMETHOD LLVM_createAAEvalPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createAAEvalPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createLibCallAliasAnalysisPass(LibCallInfo lci);
static KMETHOD LLVM_createLibCallAliasAnalysisPass(KonohaContext *kctx, KonohaStack *sfp)
{
	LibCallInfo *lci = konoha::object_cast<LibCallInfo *>(sfp[0].asObject);
	FunctionPass *ptr = createLibCallAliasAnalysisPass(lci);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createScalarEvolutionAliasAnalysisPass();
static KMETHOD LLVM_createScalarEvolutionAliasAnalysisPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createScalarEvolutionAliasAnalysisPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## ModulePass LLVM.createProfileLoaderPass();
static KMETHOD LLVM_createProfileLoaderPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createProfileLoaderPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createProfileEstimatorPass();
static KMETHOD LLVM_createProfileEstimatorPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createProfileEstimatorPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createProfileVerifierPass();
static KMETHOD LLVM_createProfileVerifierPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createProfileVerifierPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## ModulePass LLVM.createPathProfileLoaderPass();
static KMETHOD LLVM_createPathProfileLoaderPass(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;
	DEPRICATE_API("NO SUPPORT");
#else
	ModulePass *ptr = createPathProfileLoaderPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
#endif
}

//## ModulePass LLVM.createPathProfileVerifierPass();
static KMETHOD LLVM_createPathProfileVerifierPass(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;
	DEPRICATE_API("NO SUPPORT");
#else
	ModulePass *ptr = createPathProfileVerifierPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
#endif
}

//## FunctionPass LLVM.createLazyValueInfoPass();
static KMETHOD LLVM_createLazyValueInfoPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createLazyValueInfoPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## LoopPass LLVM.createLoopDependenceAnalysisPass();
static KMETHOD LLVM_createLoopDependenceAnalysisPass(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 301
	LoopPass *ptr = createLoopDependenceAnalysisPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
#else
	(void)kctx;(void)sfp;
#endif
}

//## FunctionPass LLVM.createInstCountPass();
static KMETHOD LLVM_createInstCountPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createInstCountPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createDbgInfoPrinterPass();
static KMETHOD LLVM_createDbgInfoPrinterPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createDbgInfoPrinterPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createRegionInfoPass();
static KMETHOD LLVM_createRegionInfoPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createRegionInfoPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}
//## Constant* ConstantExpr::getAlignOf(Type* ty);
static KMETHOD ConstantExpr_getAlignOf(KonohaContext *kctx, KonohaStack *sfp)
{
	Type* ty = konoha::object_cast<Type*>(sfp[1].asObject);
	Constant* ptr = ConstantExpr::getAlignOf(ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getSizeOf(Type* ty);
static KMETHOD ConstantExpr_getSizeOf(KonohaContext *kctx, KonohaStack *sfp)
{
	Type* ty = konoha::object_cast<Type*>(sfp[1].asObject);
	Constant* ptr = ConstantExpr::getSizeOf(ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getOffsetOf(StructType* sTy, unsigned fieldNo);
static KMETHOD ConstantExpr_getOffsetOf(KonohaContext *kctx, KonohaStack *sfp)
{
	StructType* sTy = konoha::object_cast<StructType*>(sfp[1].asObject);
	unsigned fieldNo = (sfp[2].intValue);
	Constant* ptr = ConstantExpr::getOffsetOf(sTy, fieldNo);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

////## Constant* ConstantExpr::getOffsetOf(Type* ty, Constant* fieldNo);
//static KMETHOD ConstantExpr_getOffsetOf(KonohaContext *kctx, KonohaStack *sfp)
//{
//	Type* ty = konoha::object_cast<Type*>(sfp[1].asObject);
//	Constant* fieldNo = konoha::object_cast<Constant*>(sfp[2].asObject);
//	Constant* ptr = ConstantExpr::getOffsetOf(ty, fieldNo);
//	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	KReturn(p);
//}

//## Constant* ConstantExpr::getNeg(Constant* c, bool hasNUW, bool hasNSW);
static KMETHOD ConstantExpr_getNeg(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* ptr;
#if LLVM_VERSION <= 209
	ptr = ConstantExpr::getNeg(c);
#else
	bool hasNUW = sfp[2].boolValue;
	bool hasNSW = sfp[3].boolValue;
	ptr = ConstantExpr::getNeg(c, hasNUW, hasNSW);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getFNeg(Constant* c);
static KMETHOD ConstantExpr_getFNeg(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* ptr = ConstantExpr::getFNeg(c);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getNot(Constant* c);
static KMETHOD ConstantExpr_getNot(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* ptr = ConstantExpr::getNot(c);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getAdd(Constant* c1, Constant* c2, bool hasNUW, bool hasNSW);
static KMETHOD ConstantExpr_getAdd(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr;
#if LLVM_VERSION <= 209
	ptr = ConstantExpr::getAdd(c1, c2);
#else
	bool hasNUW = sfp[3].boolValue;
	bool hasNSW = sfp[4].boolValue;
	ptr = ConstantExpr::getAdd(c1, c2, hasNUW, hasNSW);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getFAdd(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getFAdd(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getFAdd(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getSub(Constant* c1, Constant* c2, bool hasNUW, bool hasNSW);
static KMETHOD ConstantExpr_getSub(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr;
#if LLVM_VERSION <= 209
	ptr = ConstantExpr::getSub(c1, c2);
#else
	bool hasNUW = sfp[3].boolValue;
	bool hasNSW = sfp[4].boolValue;
	ptr = ConstantExpr::getSub(c1, c2, hasNUW, hasNSW);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getFSub(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getFSub(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getFSub(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getMul(Constant* c1, Constant* c2, bool hasNUW, bool hasNSW);
static KMETHOD ConstantExpr_getMul(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr;
#if LLVM_VERSION <= 209
	ptr = ConstantExpr::getMul(c1, c2);
#else
	bool hasNUW = sfp[3].boolValue;
	bool hasNSW = sfp[4].boolValue;
	ptr = ConstantExpr::getMul(c1, c2, hasNUW, hasNSW);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getFMul(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getFMul(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getFMul(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getUDiv(Constant* c1, Constant* c2, bool isExact);
static KMETHOD ConstantExpr_getUDiv(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr;
#if LLVM_VERSION <= 209
	ptr = ConstantExpr::getUDiv(c1, c2);
#else
	bool isExact = sfp[3].boolValue;
	ptr = ConstantExpr::getUDiv(c1, c2, isExact);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getSDiv(Constant* c1, Constant* c2, bool isExact);
static KMETHOD ConstantExpr_getSDiv(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr;
#if LLVM_VERSION <= 209
	ptr = ConstantExpr::getSDiv(c1, c2);
#else
	bool isExact = sfp[3].boolValue;
	ptr = ConstantExpr::getSDiv(c1, c2, isExact);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getFDiv(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getFDiv(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getFDiv(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getURem(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getURem(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getURem(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getSRem(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getSRem(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getSRem(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getFRem(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getFRem(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getFRem(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getAnd(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getAnd(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getAnd(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getOr(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getOr(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getOr(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getXor(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getXor(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getXor(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getShl(Constant* c1, Constant* c2, bool hasNUW, bool hasNSW);
static KMETHOD ConstantExpr_getShl(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr;
#if LLVM_VERSION <= 209
	ptr = ConstantExpr::getShl(c1, c2);
#else
	bool hasNUW = sfp[3].boolValue;
	bool hasNSW = sfp[4].boolValue;
	ptr = ConstantExpr::getShl(c1, c2, hasNUW, hasNSW);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getLShr(Constant* c1, Constant* c2, bool isExact);
static KMETHOD ConstantExpr_getLShr(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr;
#if LLVM_VERSION <= 209
	ptr = ConstantExpr::getLShr(c1, c2);
#else
	bool isExact = sfp[3].boolValue;
	ptr = ConstantExpr::getLShr(c1, c2, isExact);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getAShr(Constant* c1, Constant* c2, bool isExact);
static KMETHOD ConstantExpr_getAShr(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr;
#if LLVM_VERSION <= 209
	ptr = ConstantExpr::getAShr(c1, c2);
#else
	bool isExact = sfp[3].boolValue;
	ptr = ConstantExpr::getAShr(c1, c2, isExact);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getTrunc(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getTrunc(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getTrunc(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getSExt(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getSExt(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getSExt(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getZExt(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getZExt(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getZExt(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getFPTrunc(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getFPTrunc(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getFPTrunc(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getFPExtend(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getFPExtend(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getFPExtend(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getUIToFP(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getUIToFP(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getUIToFP(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getSIToFP(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getSIToFP(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getSIToFP(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getFPToUI(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getFPToUI(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getFPToUI(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getFPToSI(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getFPToSI(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getFPToSI(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getPtrToInt(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getPtrToInt(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getPtrToInt(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getIntToPtr(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getIntToPtr(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getIntToPtr(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getBitCast(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getBitCast(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getBitCast(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getNSWNeg(Constant* c);
static KMETHOD ConstantExpr_getNSWNeg(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* ptr = ConstantExpr::getNSWNeg(c);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getNUWNeg(Constant* c);
static KMETHOD ConstantExpr_getNUWNeg(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* ptr = ConstantExpr::getNUWNeg(c);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getNSWAdd(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getNSWAdd(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getNSWAdd(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getNUWAdd(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getNUWAdd(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getNUWAdd(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getNSWSub(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getNSWSub(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getNSWSub(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getNUWSub(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getNUWSub(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getNUWSub(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getNSWMul(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getNSWMul(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getNSWMul(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getNUWMul(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getNUWMul(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getNUWMul(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getNSWShl(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getNSWShl(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;
	DEPRICATE_API("NO SUPPORT");
#else
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getNSWShl(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
#endif
}

//## Constant* ConstantExpr::getNUWShl(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getNUWShl(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;
	DEPRICATE_API("NO SUPPORT");
#else
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getNUWShl(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
#endif
}

//## Constant* ConstantExpr::getExactSDiv(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getExactSDiv(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getExactSDiv(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getExactUDiv(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getExactUDiv(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;
	DEPRICATE_API("NO SUPPORT");
#else
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getExactUDiv(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
#endif
}

//## Constant* ConstantExpr::getExactAShr(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getExactAShr(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;
	DEPRICATE_API("NO SUPPORT");
#else
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getExactAShr(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
#endif
}

//## Constant* ConstantExpr::getExactLShr(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getExactLShr(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;
	DEPRICATE_API("NO SUPPORT");
#else
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getExactLShr(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
#endif
}

//## Constant* ConstantExpr::getZExtOrBitCast(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getZExtOrBitCast(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getZExtOrBitCast(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getSExtOrBitCast(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getSExtOrBitCast(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getSExtOrBitCast(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getTruncOrBitCast(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getTruncOrBitCast(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getTruncOrBitCast(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getPointerCast(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getPointerCast(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getPointerCast(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getIntegerCast(Constant* c, Type* ty, bool isSigned);
static KMETHOD ConstantExpr_getIntegerCast(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].asObject);
	bool isSigned = sfp[3].boolValue;
	Constant* ptr = ConstantExpr::getIntegerCast(c, ty, isSigned);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getFPCast(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getFPCast(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getFPCast(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getSelect(Constant* c, Constant* v1, Constant* v2);
static KMETHOD ConstantExpr_getSelect(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* v1 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* v2 = konoha::object_cast<Constant*>(sfp[3].asObject);
	Constant* ptr = ConstantExpr::getSelect(c, v1, v2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

////## Constant* ConstantExpr::getElementPtr(Constant* c, ArrayRef<Constant*> IdxList, bool InBounds);
//static KMETHOD ConstantExpr_getElementPtr(KonohaContext *kctx, KonohaStack *sfp)
//{
//	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
//	kArray* _list = sfp[2].asArray;
//	std::vector<Constant*> IdxList;
//	konoha::convert_array(IdxList, _list);
//	bool InBounds = sfp[3].boolValue;
//	Constant* ptr = ConstantExpr::getElementPtr(c, IdxList, InBounds);
//	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	KReturn(p);
//}

//## Constant* ConstantExpr::getElementPtr(Constant* c, Constant* idx, bool InBounds);
static KMETHOD ConstantExpr_getElementPtr0(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* idx = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr;
#if LLVM_VERSION <= 209
	Constant *IdxList[] = {idx};
	ptr = ConstantExpr::getGetElementPtr(c, IdxList, 0);
#else
	bool InBounds = sfp[3].boolValue;
	ptr = ConstantExpr::getGetElementPtr(c, idx, InBounds);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getElementPtr(Constant* c, ArrayRef<Value*> IdxList, bool InBounds);
static KMETHOD ConstantExpr_getElementPtr(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	kArray* _list = sfp[2].asArray;
	std::vector<Value*> IdxList;
	konoha::convert_array(IdxList, _list);
	Constant* ptr;
#if LLVM_VERSION <= 209
	Value *const *List = &IdxList[0];
	ptr = ConstantExpr::getGetElementPtr(c, List, 0);
	fprintf(stderr, "WARN: TEST ME\n");
#else
	bool InBounds = sfp[3].boolValue;
	ptr = ConstantExpr::getGetElementPtr(c, IdxList, InBounds);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

////## Constant* ConstantExpr::getInBoundsGetElementPtr(Constant* c, ArrayRef<Constant*> IdxList);
//static KMETHOD ConstantExpr_getInBoundsGetElementPtr(KonohaContext *kctx, KonohaStack *sfp)
//{
//	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
//	kArray* _list = sfp[2].asArray;
//	std::vector<Constant*> IdxList;
//	konoha::convert_array(IdxList, _list);
//	Constant* ptr = ConstantExpr::getInBoundsGetElementPtr(c, IdxList);
//	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	KReturn(p);
//}

//## Constant* ConstantExpr::getInBoundsGetElementPtr(Constant* c, Constant* idx);
static KMETHOD ConstantExpr_getInBoundsGetElementPtr0(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* ptr;
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* idx = konoha::object_cast<Constant*>(sfp[2].asObject);
#if LLVM_VERSION <= 209
	Constant *IdxList[] = {idx};
	ptr = ConstantExpr::getInBoundsGetElementPtr(c, IdxList, 0);
#else
	ptr = ConstantExpr::getInBoundsGetElementPtr(c, idx);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getInBoundsGetElementPtr(Constant* c, ArrayRef<Value*> idxList);
static KMETHOD ConstantExpr_getInBoundsGetElementPtr(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	kArray* _list = sfp[2].asArray;
	std::vector<Value*> idxList;
	konoha::convert_array(idxList, _list);
	Constant* ptr;
#if LLVM_VERSION <= 209
	Value *const *List = &idxList[0];
	ptr = ConstantExpr::getGetElementPtr(c, List, 0);
	fprintf(stderr, "WARN: TEST ME\n");
#else
	ptr = ConstantExpr::getInBoundsGetElementPtr(c, idxList);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getExtractElement(Constant* vec, Constant* idx);
static KMETHOD ConstantExpr_getExtractElement(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* vec = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* idx = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* ptr = ConstantExpr::getExtractElement(vec, idx);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getInsertElement(Constant* vec, Constant* elt,Constant* idx);
static KMETHOD ConstantExpr_getInsertElement(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* vec = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* elt = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* idx = konoha::object_cast<Constant*>(sfp[3].asObject);
	Constant* ptr = ConstantExpr::getInsertElement(vec, elt, idx);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getShuffleVector(Constant* v1, Constant* v2, Constant* mask);
static KMETHOD ConstantExpr_getShuffleVector(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* v1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* v2 = konoha::object_cast<Constant*>(sfp[2].asObject);
	Constant* mask = konoha::object_cast<Constant*>(sfp[3].asObject);
	Constant* ptr = ConstantExpr::getShuffleVector(v1, v2, mask);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getExtractValue(Constant* Agg, ArrayRef<unsigned> idxs);
static KMETHOD ConstantExpr_getExtractValue(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* Agg = konoha::object_cast<Constant*>(sfp[1].asObject);
	kArray* _list = sfp[2].asArray;
	std::vector<unsigned> idxs(_list->kintItems, _list->kintItems+kArray_size(_list));
	Constant* ptr;
#if LLVM_VERSION <= 209
	ptr = ConstantExpr::getExtractValue(Agg, (const unsigned *) &idxs[0], 0);
	fprintf(stderr, "WARN: TEST ME\n");
#else
	ptr = ConstantExpr::getExtractValue(Agg, idxs);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Constant* ConstantExpr::getInsertValue(Constant* Agg, Constant* val, ArrayRef<unsigned> idxs);
static KMETHOD ConstantExpr_getInsertValue(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* Agg = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* val = konoha::object_cast<Constant*>(sfp[2].asObject);
	kArray* _list = sfp[3].asArray;
	std::vector<unsigned> idxs(_list->kintItems, _list->kintItems+kArray_size(_list));
	Constant* ptr;
#if LLVM_VERSION <= 209
	ptr = ConstantExpr::getInsertValue(Agg, val, (const unsigned *) &idxs[0], 0);
	fprintf(stderr, "WARN: TEST ME\n");
#else
	ptr = ConstantExpr::getInsertValue(Agg, val, idxs);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## ModulePass LLVM.createModuleDebugInfoPrinterPass();
static KMETHOD LLVM_createModuleDebugInfoPrinterPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createModuleDebugInfoPrinterPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createMemDepPrinter();
static KMETHOD LLVM_createMemDepPrinter(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;
	DEPRICATE_API("NO SUPPORT");
#else
	FunctionPass *ptr = createMemDepPrinter();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
#endif
}

//## FunctionPass LLVM.createPostDomTree();
static KMETHOD LLVM_createPostDomTree(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;
	DEPRICATE_API("NO SUPPORT");
#else
	FunctionPass *ptr = createPostDomTree();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
#endif
}

//## FunctionPass LLVM.createRegionViewerPass();
static KMETHOD LLVM_createRegionViewerPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createRegionViewerPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createRegionOnlyViewerPass();
static KMETHOD LLVM_createRegionOnlyViewerPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createRegionOnlyViewerPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createRegionPrinterPass();
static KMETHOD LLVM_createRegionPrinterPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createRegionPrinterPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createRegionOnlyPrinterPass();
static KMETHOD LLVM_createRegionOnlyPrinterPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createRegionOnlyPrinterPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createLintPass();
static KMETHOD LLVM_createLintPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createLintPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

////## ModulePass LLVM.createPrintModulePass(raw_ostream *OS);
//KMETHOD LLVM_createPrintModulePass(KonohaContext *kctx, KonohaStack *sfp)
//{
//	raw_ostream **OS = konoha::object_cast<raw_ostream *>(sfp[0].asObject);
//	ModulePass *ptr = createPrintModulePass(*OS);
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	KReturn(p);
//}
//
////## FunctionPass LLVM.createPrintFunctionPass(String banner, OutputStream os, boolean deleteStream);
//KMETHOD LLVM_createPrintFunctionPass(KonohaContext *kctx, KonohaStack *sfp)
//{
//	String *banner = konoha::object_cast<String *>(sfp[0].asObject);
//	OutputStream *os = konoha::object_cast<OutputStream *>(sfp[1].asObject);
//	bool deleteStream = sfp[2].boolValue;
//	FunctionPass *ptr = createPrintFunctionPass(banner,os,deleteStream);
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	KReturn(p);
//}

////## ModulePass LLVM.createEdgeProfilerPass();
//KMETHOD LLVM_createEdgeProfilerPass(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ModulePass *ptr = createEdgeProfilerPass();
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	KReturn(p);
//}

////## ModulePass LLVM.createOptimalEdgeProfilerPass();
//KMETHOD LLVM_createOptimalEdgeProfilerPass(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ModulePass *ptr = createOptimalEdgeProfilerPass();
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	KReturn(p);
//}

////## ModulePass LLVM.createPathProfilerPass();
//KMETHOD LLVM_createPathProfilerPass(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ModulePass *ptr = createPathProfilerPass();
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	KReturn(p);
//}

////## ModulePass LLVM.createGCOVProfilerPass(boolean emitNotes, boolean emitData, boolean use402Format);
//KMETHOD LLVM_createGCOVProfilerPass(KonohaContext *kctx, KonohaStack *sfp)
//{
//	bool emitNotes = sfp[0].boolValue;
//	bool emitData = sfp[1].boolValue;
//	bool use402Format = sfp[2].boolValue;
//	ModulePass *ptr = createGCOVProfilerPass(emitNotes,emitData,use402Format);
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	KReturn(p);
//}

//## ModulePass LLVM.createStripSymbolsPass(bool onlyDebugInfo);
static KMETHOD LLVM_createStripSymbolsPass(KonohaContext *kctx, KonohaStack *sfp)
{
	bool onlyDebugInfo = sfp[0].boolValue;
	ModulePass *ptr = createStripSymbolsPass(onlyDebugInfo);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## ModulePass LLVM.createStripNonDebugSymbolsPass();
static KMETHOD LLVM_createStripNonDebugSymbolsPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createStripNonDebugSymbolsPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## ModulePass LLVM.createStripDeadDebugInfoPass();
static KMETHOD LLVM_createStripDeadDebugInfoPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createStripDeadDebugInfoPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## ModulePass LLVM.createConstantMergePass();
static KMETHOD LLVM_createConstantMergePass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createConstantMergePass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## ModulePass LLVM.createGlobalOptimizerPass();
static KMETHOD LLVM_createGlobalOptimizerPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createGlobalOptimizerPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## ModulePass LLVM.createGlobalDCEPass();
static KMETHOD LLVM_createGlobalDCEPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createGlobalDCEPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createFunctionInliningPass(int threshold);
static KMETHOD LLVM_createFunctionInliningPass(KonohaContext *kctx, KonohaStack *sfp)
{
	int threshold = sfp[0].intValue;
	Pass *ptr = createFunctionInliningPass(threshold);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createAlwaysInlinerPass();
static KMETHOD LLVM_createAlwaysInlinerPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createAlwaysInlinerPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createPruneEHPass();
static KMETHOD LLVM_createPruneEHPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createPruneEHPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## ModulePass LLVM.createInternalizePass(bool allButMain);
static KMETHOD LLVM_createInternalizePass(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 301
	bool allButMain = sfp[0].boolValue;
	ModulePass *ptr = createInternalizePass(allButMain);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
#else
	(void)kctx;(void)sfp;
#endif
}

//## ModulePass LLVM.createDeadArgEliminationPass();
static KMETHOD LLVM_createDeadArgEliminationPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createDeadArgEliminationPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createArgumentPromotionPass(int maxElements);
static KMETHOD LLVM_createArgumentPromotionPass(KonohaContext *kctx, KonohaStack *sfp)
{
	int maxElements = sfp[0].intValue;
	Pass *ptr = createArgumentPromotionPass(maxElements);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## ModulePass LLVM.createIPConstantPropagationPass();
static KMETHOD LLVM_createIPConstantPropagationPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createIPConstantPropagationPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## ModulePass LLVM.createIPSCCPPass();
static KMETHOD LLVM_createIPSCCPPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createIPSCCPPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createLoopExtractorPass();
static KMETHOD LLVM_createLoopExtractorPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createLoopExtractorPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createSingleLoopExtractorPass();
static KMETHOD LLVM_createSingleLoopExtractorPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createSingleLoopExtractorPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## ModulePass LLVM.createBlockExtractorPass();
static KMETHOD LLVM_createBlockExtractorPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createBlockExtractorPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## ModulePass LLVM.createStripDeadPrototypesPass();
static KMETHOD LLVM_createStripDeadPrototypesPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createStripDeadPrototypesPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createFunctionAttrsPass();
static KMETHOD LLVM_createFunctionAttrsPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createFunctionAttrsPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## ModulePass LLVM.createMergeFunctionsPass();
static KMETHOD LLVM_createMergeFunctionsPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createMergeFunctionsPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## ModulePass LLVM.createPartialInliningPass();
static KMETHOD LLVM_createPartialInliningPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createPartialInliningPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createConstantPropagationPass();
static KMETHOD LLVM_createConstantPropagationPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createConstantPropagationPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createSCCPPass();
static KMETHOD LLVM_createSCCPPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createSCCPPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createDeadInstEliminationPass();
static KMETHOD LLVM_createDeadInstEliminationPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createDeadInstEliminationPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createDeadCodeEliminationPass();
static KMETHOD LLVM_createDeadCodeEliminationPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createDeadCodeEliminationPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createDeadStoreEliminationPass();
static KMETHOD LLVM_createDeadStoreEliminationPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createDeadStoreEliminationPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createAggressiveDCEPass();
static KMETHOD LLVM_createAggressiveDCEPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createAggressiveDCEPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createScalarReplAggregatesPass(int threshold);
static KMETHOD LLVM_createScalarReplAggregatesPass(KonohaContext *kctx, KonohaStack *sfp)
{
	int threshold = sfp[0].intValue;
	FunctionPass *ptr = createScalarReplAggregatesPass(threshold);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createIndVarSimplifyPass();
static KMETHOD LLVM_createIndVarSimplifyPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createIndVarSimplifyPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createInstructionCombiningPass();
static KMETHOD LLVM_createInstructionCombiningPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createInstructionCombiningPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createLICMPass();
static KMETHOD LLVM_createLICMPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createLICMPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createLoopUnswitchPass(bool optimizeForSize);
static KMETHOD LLVM_createLoopUnswitchPass(KonohaContext *kctx, KonohaStack *sfp)
{
	bool optimizeForSize = sfp[0].boolValue;
	Pass *ptr = createLoopUnswitchPass(optimizeForSize);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createLoopInstSimplifyPass();
static KMETHOD LLVM_createLoopInstSimplifyPass(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;
	DEPRICATE_API("NO SUPPORT");
#else
	Pass *ptr = createLoopInstSimplifyPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
#endif
}

//## Pass LLVM.createLoopUnrollPass(int threshold, int count, int allowPartial);
static KMETHOD LLVM_createLoopUnrollPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr;
#if LLVM_VERSION <= 209
	ptr = createLoopUnrollPass();
#else
	int threshold = sfp[0].intValue;
	int count = sfp[1].intValue;
	int allowPartial = sfp[2].intValue;
	ptr = createLoopUnrollPass(threshold,count,allowPartial);
#endif
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createLoopRotatePass();
static KMETHOD LLVM_createLoopRotatePass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createLoopRotatePass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createLoopIdiomPass();
static KMETHOD LLVM_createLoopIdiomPass(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;
	DEPRICATE_API("NO SUPPORT");
#else
	Pass *ptr = createLoopIdiomPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
#endif
}

//## FunctionPass LLVM.createPromoteMemoryToRegisterPass();
static KMETHOD LLVM_createPromoteMemoryToRegisterPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createPromoteMemoryToRegisterPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createDemoteRegisterToMemoryPass();
static KMETHOD LLVM_createDemoteRegisterToMemoryPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createDemoteRegisterToMemoryPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createReassociatePass();
static KMETHOD LLVM_createReassociatePass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createReassociatePass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createJumpThreadingPass();
static KMETHOD LLVM_createJumpThreadingPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createJumpThreadingPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createCFGSimplificationPass();
static KMETHOD LLVM_createCFGSimplificationPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createCFGSimplificationPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createBreakCriticalEdgesPass();
static KMETHOD LLVM_createBreakCriticalEdgesPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createBreakCriticalEdgesPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createLoopSimplifyPass();
static KMETHOD LLVM_createLoopSimplifyPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createLoopSimplifyPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createTailCallEliminationPass();
static KMETHOD LLVM_createTailCallEliminationPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createTailCallEliminationPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createLowerSwitchPass();
static KMETHOD LLVM_createLowerSwitchPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createLowerSwitchPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createBlockPlacementPass();
static KMETHOD LLVM_createBlockPlacementPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createBlockPlacementPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createLCSSAPass();
static KMETHOD LLVM_createLCSSAPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createLCSSAPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createEarlyCSEPass();
static KMETHOD LLVM_createEarlyCSEPass(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;
	DEPRICATE_API("NO SUPPORT");
#else
	FunctionPass *ptr = createEarlyCSEPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
#endif
}

//## FunctionPass LLVM.createGVNPass(bool noLoads);
static KMETHOD LLVM_createGVNPass(KonohaContext *kctx, KonohaStack *sfp)
{
	bool noLoads = sfp[0].boolValue;
	FunctionPass *ptr = createGVNPass(noLoads);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createMemCpyOptPass();
static KMETHOD LLVM_createMemCpyOptPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createMemCpyOptPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createLoopDeletionPass();
static KMETHOD LLVM_createLoopDeletionPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createLoopDeletionPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createSimplifyLibCallsPass();
static KMETHOD LLVM_createSimplifyLibCallsPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createSimplifyLibCallsPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createInstructionNamerPass();
static KMETHOD LLVM_createInstructionNamerPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createInstructionNamerPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## FunctionPass LLVM.createSinkingPass();
static KMETHOD LLVM_createSinkingPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createSinkingPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createLowerAtomicPass();
static KMETHOD LLVM_createLowerAtomicPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createLowerAtomicPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createCorrelatedValuePropagationPass();
static KMETHOD LLVM_createCorrelatedValuePropagationPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createCorrelatedValuePropagationPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

#if LLVM_VERSION >= 300
//## Pass LLVM.createObjCARCExpandPass();
static KMETHOD LLVM_createObjCARCExpandPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createObjCARCExpandPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createObjCARCContractPass();
static KMETHOD LLVM_createObjCARCContractPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createObjCARCContractPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## Pass LLVM.createObjCARCOptPass();
static KMETHOD LLVM_createObjCARCOptPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createObjCARCOptPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}
#endif

//## FunctionPass LLVM.createInstructionSimplifierPass();
static KMETHOD LLVM_createInstructionSimplifierPass(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;
	DEPRICATE_API("NO SUPPORT");
#else
	FunctionPass *ptr = createInstructionSimplifierPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
#endif
}

#if LLVM_VERSION >= 300
//## FunctionPass LLVM.createLowerExpectIntrinsicPass();
static KMETHOD LLVM_createLowerExpectIntrinsicPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createLowerExpectIntrinsicPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}
#endif

//## Pass LLVM.createUnifyFunctionExitNodesPass();
static KMETHOD LLVM_createUnifyFunctionExitNodesPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createUnifyFunctionExitNodesPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## ImmutablePass LLVM.createTypeBasedAliasAnalysisPass();
static KMETHOD LLVM_createTypeBasedAliasAnalysisPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ImmutablePass *ptr = createTypeBasedAliasAnalysisPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## ImmutablePass LLVM.createBasicAliasAnalysisPass();
static KMETHOD LLVM_createBasicAliasAnalysisPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ImmutablePass *ptr = createBasicAliasAnalysisPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//## ImmutablePass LLVM.createVerifierPass();
static KMETHOD LLVM_createVerifierPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createVerifierPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

#if LLVM_VERSION >= 301
//## BasicBlockPass LLVM.createBBVectorizePass();
static KMETHOD LLVM_createBBVectorizePass(KonohaContext *kctx, KonohaStack *sfp)
{
	BasicBlockPass *ptr = createBBVectorizePass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}
#endif

//FunctionType Intrinsic::getType(int id, Type[] args);
static KMETHOD Intrinsic_getType(KonohaContext *kctx, KonohaStack *sfp)
{
	Intrinsic::ID id = (Intrinsic::ID) sfp[1].intValue;
	kArray *args = sfp[2].asArray;
	std::vector<LLVMTYPE*> List;
	konoha::convert_array(List, args);
#if LLVM_VERSION <= 209
	const FunctionType *ptr;
	ptr = Intrinsic::getType(getGlobalContext(), id, (const Type **) &List[0]);
#else
	FunctionType *ptr;
	ptr = Intrinsic::getType(getGlobalContext(), id, List);
#endif
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

//Function     Intrinsic::getDeclaration(Module m, int id, Type[] args);
static KMETHOD Intrinsic_getDeclaration(KonohaContext *kctx, KonohaStack *sfp)
{
	Module *m = konoha::object_cast<Module *>(sfp[1].asObject);
	Intrinsic::ID id = (Intrinsic::ID) sfp[2].intValue;
	kArray *args = sfp[3].asArray;
	Function *ptr;
	std::vector<LLVMTYPE*> List;
	konoha::convert_array(List, args);
#if LLVM_VERSION <= 209
	ptr = Intrinsic::getDeclaration(m, id, (const Type **) &List[0]);
#else
	ptr = Intrinsic::getDeclaration(m, id, List);
#endif
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	KReturn(p);
}

static KMETHOD LLVM_parseBitcodeFile(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *Str = sfp[1].asString;
	LLVMContext &Context = getGlobalContext();
	std::string ErrMsg;
	OwningPtr<MemoryBuffer> BufferPtr;
#if LLVM_VERSION <= 208
	const char *fname = kString_text(Str);
	BufferPtr.reset(MemoryBuffer::getFile(fname, &ErrMsg));
	if(!BufferPtr) {
		std::cout << "Could not open file " << ErrMsg << std::endl;
	}
#else
	std::string fname(kString_text(Str));
	if(error_code ec = MemoryBuffer::getFile(fname, BufferPtr)) {
		std::cout << "Could not open file " << ec.message() << std::endl;
	}
#endif
	MemoryBuffer *Buffer = BufferPtr.take();
	//Module *m = getLazyBitcodeModule(Buffer, Context, &ErrMsg);
	Module *m = ParseBitcodeFile(Buffer, Context, &ErrMsg);
	if(!m) {
		std::cout << "error" << ErrMsg << std::endl;
	}
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(m));
	KReturn(p);
}

//TODO Scriptnize
static KMETHOD Instruction_SetMetadata(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;
	DEPRICATE_API("NO SUPPORT");
#else
	Instruction *inst = konoha::object_cast<Instruction *>(sfp[0].asObject);
	Module *m = konoha::object_cast<Module *>(sfp[1].asObject);
	kString *Str = sfp[2].asString;
	kint_t N = Int_to(kint_t,sfp[3]);
	Value *Info[] = {
		ConstantInt::get(Type::getInt32Ty(getGlobalContext()), N)
	};
	LLVMContext &Context = getGlobalContext();
	MDNode *node = MDNode::get(Context, Info);
	NamedMDNode *NMD = m->getOrInsertNamedMetadata(kString_text(Str));
	unsigned KindID = Context.getMDKindID(kString_text(Str));
	NMD->addOperand(node);
	inst->setMetadata(KindID, node);
#endif
	KReturnVoid();
}

//FIXME TODO stupid down cast
static KMETHOD Object_toValue(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	KReturn(sfp[0].asObject);
}

//FIXME TODO stupid down cast
static KMETHOD Object_toType(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	KReturn(sfp[0].asObject);
}

//FIXME TODO stupid down cast
static KMETHOD Object_toModule(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	KReturn(sfp[0].asObject);
}

//FIXME TODO stupid down cast
static KMETHOD Object_toExecutionEngine(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	KReturn(sfp[0].asObject);
}

//FIXME TODO stupid down cast
static KMETHOD Object_asFunction(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	KReturn(sfp[0].asObject);
}

//FIXME TODO stupid down cast
static KMETHOD Object_toIRBuilder(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	KReturn(sfp[0].asObject);
}

//FIXME TODO stupid down cast
static KMETHOD Object_asFunctionType(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	KReturn(sfp[0].asObject);
}

//FIXME TODO stupid down cast
static KMETHOD Object_toLLVMBasicBlock(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	KReturn(sfp[0].asObject);
}

static KDEFINE_INT_CONST IntIntrinsic[] = {
	{"Pow"  , KType_Int, (int) Intrinsic::pow},
	{"Sqrt" , KType_Int, (int) Intrinsic::sqrt},
	{"Exp"  , KType_Int, (int) Intrinsic::exp},
	{"Log10", KType_Int, (int) Intrinsic::log10},
	{"Log"  , KType_Int, (int) Intrinsic::log},
	{"Sin"  , KType_Int, (int) Intrinsic::sin},
	{"Cos"  , KType_Int, (int) Intrinsic::cos},
	{NULL, 0, 0}
};

static KDEFINE_INT_CONST IntGlobalVariable[] = {
	{"ExternalLinkage",                 KType_Int, GlobalValue::ExternalLinkage},
	{"AvailableExternallyLinkage",      KType_Int, GlobalValue::AvailableExternallyLinkage},
	{"LinkOnceAnyLinkage",              KType_Int, GlobalValue::LinkOnceODRLinkage},
	{"WeakAnyLinkage",                  KType_Int, GlobalValue::WeakAnyLinkage},
	{"WeakODRLinkage",                  KType_Int, GlobalValue::WeakODRLinkage},
	{"AppendingLinkage",                KType_Int, GlobalValue::AppendingLinkage},
	{"InternalLinkage",                 KType_Int, GlobalValue::InternalLinkage},
	{"PrivateLinkage",                  KType_Int, GlobalValue::PrivateLinkage},
	{"LinkerPrivateLinkage",            KType_Int, GlobalValue::LinkerPrivateLinkage},
	{"LinkerPrivateWeakLinkage",        KType_Int, GlobalValue::LinkerPrivateWeakLinkage},
#if LLVM_VERSION < 302
	{"LinkerPrivateWeakDefAutoLinkage", KType_Int, GlobalValue::LinkerPrivateWeakDefAutoLinkage},
#endif
	{"DLLImportLinkage",                KType_Int, GlobalValue::DLLImportLinkage},
	{"DLLExportLinkage",                KType_Int, GlobalValue::DLLExportLinkage},
	{"ExternalWeakLinkage",             KType_Int, GlobalValue::ExternalWeakLinkage},
	{"CommonLinkage",                   KType_Int, GlobalValue::CommonLinkage},
	{NULL, 0, 0}
};

#if LLVM_VERSION == 301
#define C_(S) {#S , KType_Int, S ## _i}
#elif LLVM_VERSION == 302
#define C_(S) {#S , KType_Int, Attributes::S}
#else
#define C_(S) {#S , KType_Int, S}
#endif
#if LLVM_VERSION <= 301
using namespace llvm::Attribute;
#endif
static KDEFINE_INT_CONST IntAttributes[] = {
	C_(None),
	C_(ZExt),
	C_(SExt),
	C_(NoReturn),
	C_(InReg),
	C_(StructRet),
	C_(NoUnwind),
	C_(NoAlias),
	C_(ByVal),
	C_(Nest),
	C_(ReadNone),
	C_(ReadOnly),
	C_(NoInline),
	C_(AlwaysInline),
	C_(OptimizeForSize),
	C_(StackProtect),
	C_(StackProtectReq),
	C_(Alignment),
	C_(NoCapture),
	C_(NoRedZone),
	C_(NoImplicitFloat),
	C_(Naked),
	C_(InlineHint),
	C_(StackAlignment),
#if LLVM_VERSION >= 300
	C_(ReturnsTwice),
	C_(UWTable),
	C_(NonLazyBind),
#endif
	{NULL, 0, 0}
};
#undef C_

//
//void defGlobalValue(KonohaContext *kctx _UNUSED_, ktypeattr_t cid _UNUSED_, kclassdef_t *cdef)
//{
//	cdef->name = "GlobalValue";
//}
//
//void constGlobalValue(KonohaContext *kctx, ktypeattr_t cid, const knh_LoaderAPI_t *kapi)
//{
//	kapi->loadClassIntConst(kctx, cid, IntGlobalVariable);
//}
//
//void defIntrinsic(KonohaContext *kctx _UNUSED_, ktypeattr_t cid _UNUSED_, kclassdef_t *cdef)
//{
//	cdef->name = "Intrinsic";
//}
//
//void constIntrinsic(KonohaContext *kctx, ktypeattr_t cid, const knh_LoaderAPI_t *kapi)
//{
//	kapi->loadClassIntConst(kctx, cid, IntIntrinsic);
//}
//
//
//void defAttributes(KonohaContext *kctx _UNUSED_, ktypeattr_t cid _UNUSED_, kclassdef_t *cdef)
//{
//	cdef->name = "Attributes";
//}
//
//void constAttributes(KonohaContext *kctx _UNUSED_, ktypeattr_t cid _UNUSED_, const knh_LoaderAPI_t *kapi)
//{
//	kapi->loadClassIntConst(kctx, cid, IntAttributes);
//}

static void kmodllvm_Setup(KonohaContext *kctx, struct KRuntimeModule *def, int newctx)
{
	(void)kctx;(void)def;(void)newctx;
}

static void kmodllvm_free(KonohaContext *kctx, struct KRuntimeModule *baseh)
{
	KFree(baseh, sizeof(kmodllvm_t));
}

static kbool_t llvm_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int argc, KTraceInfo *trace)
{
	KRequireKonohaCommonModule(trace)
	(void)argc;
	kmodllvm_t *base = (kmodllvm_t *)KCalloc_UNTRACE(sizeof(kmodllvm_t), 1);
	base->h.name     = "llvm";
	base->h.setupModuleContext = kmodllvm_Setup;
	base->h.freeModule         = kmodllvm_free;
	KLIB KRuntime_SetModule(kctx, MOD_llvm, &base->h, trace);

#define DEFINE_CLASS_CPP(\
	/*const char * */structname,\
	/*ktypeattr_t      */typeId,         /*kshortflag_t    */cflag,\
	/*ktypeattr_t      */baseTypeId,     /*ktypeattr_t         */superTypeId,\
	/*ktypeattr_t      */rtype,          /*kushort_t       */cparamsize,\
	/*struct kparamtype_t   **/cparamItems,\
	/*size_t     */cstruct_size,\
	/*KClassField   **/fieldItems,\
	/*kushort_t  */fieldsize,       /*kushort_t */fieldAllocSize,\
		init,\
		reftrace,\
		free,\
		fnull,\
		p,\
		unbox,\
		compareTo,\
		compareUnboxValue,\
		initdef,\
		isSubType,\
		realtype) {\
	/*const char * */structname,\
	/*ktypeattr_t      */typeId,         /*kshortflag_t    */cflag,\
	/*ktypeattr_t      */baseTypeId,     /*ktypeattr_t         */superTypeId,\
	/*ktypeattr_t      */rtype,          /*kushort_t       */cparamsize,\
	/*struct kparamtype_t   * */cparamItems,\
	/*size_t     */cstruct_size,\
	/*KClassField   * */fieldItems,\
	/*kushort_t  */fieldsize,       /*kushort_t */fieldAllocSize,\
		init,\
		reftrace,\
		free,\
		fnull,\
		p,\
		unbox,\
		compareTo,\
		compareUnboxValue,\
		initdef,\
		isSubType,\
		realtype}

#define DEFINE_CLASS_0(NAME, FN_INIT, FN_FREE, FN_COMPARE) DEFINE_CLASS_CPP(\
		NAME,\
		KTypeAttr_NewId, 0,\
		0, 0,\
		0, 0,\
		NULL,\
		sizeof(kRawPtr),\
		NULL, 0, 0,\
		FN_INIT/*init*/,\
		0/*reftrace*/,\
		FN_FREE/*free*/,\
		0/*fnull*/,\
		0/*p*/,\
		0/*unbox*/,\
		FN_COMPARE/*compareTo*/,\
		0/*compareUnboxValue*/,\
		0/*initdef*/,\
		0/*isSubType*/,\
		0/*realtype*/)


	static KDEFINE_CLASS ValueDef = DEFINE_CLASS_0("Value", 0, 0, 0);
	base->cValue = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &ValueDef, trace);

	static const char *TypeDefName[] = {
		"Type",
		"IntegerType",
		"PointerType",
		"FunctionType",
		"ArrayType",
		"StructType"
	};
	KClass *KClass_TypeTBL[6];
	KClass *KClass_BasicBlock, *KClass_IRBuilder;
#define KType_BasicBlock  (KClass_BasicBlock)->typeId
#define KType_IRBuilder   (KClass_IRBuilder)->typeId
#define KType_Type         (KClass_TypeTBL[0])->typeId
//#define KType_IntegerType  (KClass_TypeTBL[1])->typeId
#define KType_PointerType  (KClass_TypeTBL[2])->typeId
#define KType_FunctionType (KClass_TypeTBL[3])->typeId
#define KType_ArrayType    (KClass_TypeTBL[4])->typeId
#define KType_StructType   (KClass_TypeTBL[5])->typeId
	{
		static KDEFINE_CLASS TypeDef;
		bzero(&TypeDef, sizeof(KDEFINE_CLASS));
		TypeDef.typeId  = KTypeAttr_NewId;
		TypeDef.init = Type_init;
		TypeDef.free = Type_free;
		for (int i = 0; i < 6; i++) {
			TypeDef.structname = TypeDefName[i];
			KClass_TypeTBL[i] = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &TypeDef, 0);
		}
	}
	static KDEFINE_CLASS BasicBlockDef = DEFINE_CLASS_0("LLVMBasicBlock",0, 0, BasicBlock_compareTo);
	KClass_BasicBlock = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &BasicBlockDef, trace);

	static KDEFINE_CLASS IRBuilderDef = DEFINE_CLASS_0("IRBuilder", 0, 0, 0);
	KClass_IRBuilder = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &IRBuilderDef, trace);
#if LLVM_VERSION >= 300
	static KDEFINE_CLASS PassManagerBuilderDef = DEFINE_CLASS_0("PassManagerBuilder",
			PassManagerBuilder_ptr_init, PassManagerBuilder_ptr_free, 0);
	KClass *KClass_PassManagerBuilder = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &PassManagerBuilderDef, trace);
#define KType_PassManagerBuilder         (KClass_PassManagerBuilder)->typeId
#endif
	static KDEFINE_CLASS PassManagerDef = DEFINE_CLASS_0("PassManager",
		PassManager_ptr_init, PassManager_ptr_free, 0);
	static KDEFINE_CLASS FunctionPassManagerDef = DEFINE_CLASS_0("FunctionPassManager",
			FunctionPassManager_ptr_init, FunctionPassManager_ptr_free, 0);
	KClass *KClass_PassManager = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &PassManagerDef, trace);
	KClass *KClass_FunctionPassManager = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &FunctionPassManagerDef, trace);
	KClass *KClass_InstTBL[21];
	{
		static const char *InstDefName[] = {
			"Instruction",
			"AllocaInst",
			"LoadInst",
			"StoreInst",
			"GetElementPtrInst",
			"PHINode",
			"Module",/*TODO*/
			"Function",
			"ExecutionEngine",/*TODO*/
			"GlobalVariable",
			"Argument",
			"Constant",
			"ConstantInt",
			"ConstantFP",
			"ConstantStruct",
			"ConstantPointerNull",
			"ConstantExpr",
			"LLVM",
			"LibCallInfo",
			"DynamicLibrary",
			"Intrinsic",
		};
		static KDEFINE_CLASS InstDef;
		bzero(&InstDef, sizeof(KDEFINE_CLASS));
		InstDef.typeId  = KTypeAttr_NewId;
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif
		//InstDef.init = Inst_init;
		//InstDef.free = Inst_free;
		for (unsigned int i = 0; i < ARRAY_SIZE(InstDefName); i++) {
			InstDef.structname = InstDefName[i];
			KClass_InstTBL[i] = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &InstDef, trace);
		}
	}
#define KType_Instruction         (KClass_InstTBL[ 0])->typeId
#define KType_AllocaInst          (KClass_InstTBL[ 1])->typeId
#define KType_LoadInst            (KClass_InstTBL[ 2])->typeId
#define KType_StoreInst           (KClass_InstTBL[ 3])->typeId
#define KType_GetElementPtrInst   (KClass_InstTBL[ 4])->typeId
#define KType_PHINode             (KClass_InstTBL[ 5])->typeId
#define KType_Module              (KClass_InstTBL[ 6])->typeId
#define KType_Function            (KClass_InstTBL[ 7])->typeId
#define KType_ExecutionEngine     (KClass_InstTBL[ 8])->typeId
#define KType_GlobalVariable      (KClass_InstTBL[ 9])->typeId
#define KType_Argument            (KClass_InstTBL[10])->typeId
#define KType_Constant            (KClass_InstTBL[11])->typeId
#define KType_ConstantInt         (KClass_InstTBL[12])->typeId
#define KType_ConstantFP          (KClass_InstTBL[13])->typeId
#define KType_ConstantStruct      (KClass_InstTBL[14])->typeId
#define KType_ConstantPointerNull (KClass_InstTBL[15])->typeId
#define KType_ConstantExpr        (KClass_InstTBL[16])->typeId
#define KType_LLVM                (KClass_InstTBL[17])->typeId
#define KType_LibCallInfo         (KClass_InstTBL[18])->typeId
#define KType_DynamicLibrary      (KClass_InstTBL[19])->typeId
#define KType_Intrinsic           (KClass_InstTBL[20])->typeId

	KClass *KClass_PassTBL[4];
	{
		static const char *PassDefName[] = {
			"Pass",
			"ImmutablePass",
			"FunctionPass",
			"ModulePass",
		};
		static KDEFINE_CLASS PassDef;
		bzero(&PassDef, sizeof(KDEFINE_CLASS));
		PassDef.typeId  = KTypeAttr_NewId;
		//InstDef.init = Inst_init;
		//InstDef.free = Inst_free;
		for (int i = 0; i < 4; i++) {
			PassDef.structname = PassDefName[i];
			KClass_PassTBL[i] = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &PassDef, trace);
		}
	}
#define KType_Pass          (KClass_PassTBL[0])->typeId
#define KType_ImmutablePass (KClass_PassTBL[1])->typeId
#define KType_FunctionPass  (KClass_PassTBL[2])->typeId
#define KType_ModulePass    (KClass_PassTBL[3])->typeId

#define KType_PassManager         (KClass_PassManager)->typeId
#define KType_FunctionPassManager (KClass_FunctionPassManager)->typeId
	/* TODO */
	kparamtype_t P_TypeArray[] = {{KType_Type, 0}};
	int KType_TypeArray = (KLIB KClass_Generics(kctx, KClass_Array, KType_void, 1, P_TypeArray))->typeId;

	kparamtype_t P_ValueArray[] = {{KType_Value, 0}};
	int KType_ValueArray = (KLIB KClass_Generics(kctx, KClass_Array, KType_void, 1, P_ValueArray))->typeId;
#define KType_Array_Value    (KType_ValueArray)
#define KType_Array_Type     (KType_TypeArray)
#define KType_Array_Constant (KType_Array)
#define KType_Array_Int      (KType_Array)
#define KType_NativeFunction (KType_Int)

	assert(KClass_Float != NULL && "please import konoha.float PACKAGE first");
	intptr_t methoddata[] = {
		_Public|_Static, _F(Type_getVoidTy), KType_Type, KType_Type, KMethodName_("getVoidTy"), 0,
		_Public|_Static, _F(Type_getLabelTy), KType_Type, KType_Type, KMethodName_("getLabelTy"), 0,
		_Public|_Static, _F(Type_getFloatTy), KType_Type, KType_Type, KMethodName_("getFloatTy"), 0,
		_Public|_Static, _F(Type_getDoubleTy), KType_Type, KType_Type, KMethodName_("getDoubleTy"), 0,
		_Public|_Static, _F(Type_getMetadataTy), KType_Type, KType_Type, KMethodName_("getMetadataTy"), 0,
		_Public|_Static, _F(Type_getX86FP80Ty), KType_Type, KType_Type, KMethodName_("getX86_FP80Ty"), 0,
		_Public|_Static, _F(Type_getFP128Ty), KType_Type, KType_Type, KMethodName_("getFP128Ty"), 0,
		_Public|_Static, _F(Type_getPPCFP128Ty), KType_Type, KType_Type, KMethodName_("getPPC_FP128Ty"), 0,
		_Public|_Static, _F(Type_getX86MMXTy), KType_Type, KType_Type, KMethodName_("getX86_MMXTy"), 0,
		_Public|_Static, _F(Type_getInt1Ty), KType_Type, KType_Type, KMethodName_("getInt1Ty"), 0,
		_Public|_Static, _F(Type_getInt8Ty), KType_Type, KType_Type, KMethodName_("getInt8Ty"), 0,
		_Public|_Static, _F(Type_getInt16Ty), KType_Type, KType_Type, KMethodName_("getInt16Ty"), 0,
		_Public|_Static, _F(Type_getInt32Ty), KType_Type, KType_Type, KMethodName_("getInt32Ty"), 0,
		_Public|_Static, _F(Type_getInt64Ty), KType_Type, KType_Type, KMethodName_("getInt64Ty"), 0,
		_Public|_Static, _F(PointerType_get), KType_Type/*TODO*/, KType_PointerType, KMethodName_("get"), 1, KType_Type, KFieldName_("type"),
		_Public|_Static, _F(Type_getFloatPtrTy),    KType_Type, KType_Type, KMethodName_("getFloatPtrTy"), 0,
		_Public|_Static, _F(Type_getDoublePtrTy),   KType_Type, KType_Type, KMethodName_("getDoublePtrTy"), 0,
		_Public|_Static, _F(Type_getX86FP80PtrTy),  KType_Type, KType_Type, KMethodName_("getX86_FP80PtrTy"), 0,
		_Public|_Static, _F(Type_getFP128PtrTy),    KType_Type, KType_Type, KMethodName_("getFP128PtrTy"), 0,
		_Public|_Static, _F(Type_getPPCFP128PtrTy), KType_Type, KType_Type, KMethodName_("getPPC_FP128PtrTy"), 0,
		_Public|_Static, _F(Type_getX86MMXPtrTy),   KType_Type, KType_Type, KMethodName_("getX86_MMXPtrTy"), 0,
		_Public|_Static, _F(Type_getInt1PtrTy),  KType_Type, KType_Type, KMethodName_("getInt1PtrTy"), 0,
		_Public|_Static, _F(Type_getInt8PtrTy),  KType_Type, KType_Type, KMethodName_("getInt8PtrTy"), 0,
		_Public|_Static, _F(Type_getInt16PtrTy), KType_Type, KType_Type, KMethodName_("getInt16PtrTy"), 0,
		_Public|_Static, _F(Type_getInt32PtrTy), KType_Type, KType_Type, KMethodName_("getInt32PtrTy"), 0,
		_Public|_Static, _F(Type_getInt64PtrTy), KType_Type, KType_Type, KMethodName_("getInt64PtrTy"), 0,
		_Public, _F(IRBuilder_new), KType_IRBuilder, KType_IRBuilder, KMethodName_("new"), 1, KType_BasicBlock, KFieldName_("bb"),
		_Public, _F(IRBuilder_createRetVoid), KType_Value, KType_IRBuilder, KMethodName_("createRetVoid"), 0,
		_Public, _F(IRBuilder_createRet),     KType_Value, KType_IRBuilder, KMethodName_("createRet"), 1, KType_Value, KFieldName_("v"),
		_Public, _F(IRBuilder_createBr),      KType_Value, KType_IRBuilder, KMethodName_("createBr"), 1, KType_BasicBlock, KFieldName_("dest"),
		_Public, _F(IRBuilder_createCondBr),  KType_Value, KType_IRBuilder, KMethodName_("createCondBr"), 3, KType_Value, KFieldName_("cond"),KType_BasicBlock, KFieldName_("trueBB"),KType_BasicBlock, KFieldName_("falseBB"),
		_Public, _F(IRBuilder_createSwitch),  KType_Value, KType_IRBuilder, KMethodName_("createSwitch"), 2, KType_Value, KFieldName_("v"),KType_BasicBlock, KFieldName_("dest"),
		_Public, _F(IRBuilder_createIndirectBr), KType_Value, KType_IRBuilder, KMethodName_("createIndirectBr"), 1, KType_Value, KFieldName_("addr"),
		_Public, _F(IRBuilder_createInvoke0), KType_Value, KType_IRBuilder, KMethodName_("createInvoke0"), 3, KType_Value, KFieldName_("callee"),KType_BasicBlock, KFieldName_("normalDest"),KType_BasicBlock, KFieldName_("unwindDest"),
		_Public, _F(IRBuilder_createInvoke1), KType_Value, KType_IRBuilder, KMethodName_("createInvoke1"), 4, KType_Value, KFieldName_("callee"),KType_BasicBlock, KFieldName_("normalDest"),KType_BasicBlock, KFieldName_("unwindDest"),KType_Value, KFieldName_("arg1"),
		_Public, _F(IRBuilder_createInvoke3), KType_Value, KType_IRBuilder, KMethodName_("createInvoke3"), 6, KType_Value, KFieldName_("callee"),KType_BasicBlock, KFieldName_("normalDest"),KType_BasicBlock, KFieldName_("unwindDest"),KType_Value, KFieldName_("arg1"),KType_Value, KFieldName_("arg2"),KType_Value, KFieldName_("arg3"),
		_Public, _F(IRBuilder_createUnreachable), KType_Value, KType_IRBuilder, KMethodName_("createUnreachable"), 0,
		_Public, _F(IRBuilder_createAdd),    KType_Value, KType_IRBuilder, KMethodName_("createAdd"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createNSWAdd), KType_Value, KType_IRBuilder, KMethodName_("createNSWAdd"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createNUWAdd), KType_Value, KType_IRBuilder, KMethodName_("createNUWAdd"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createFAdd),   KType_Value, KType_IRBuilder, KMethodName_("createFAdd"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createSub),    KType_Value, KType_IRBuilder, KMethodName_("createSub"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createNSWSub), KType_Value, KType_IRBuilder, KMethodName_("createNSWSub"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createNUWSub), KType_Value, KType_IRBuilder, KMethodName_("createNUWSub"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createFSub),   KType_Value, KType_IRBuilder, KMethodName_("createFSub"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createMul),    KType_Value, KType_IRBuilder, KMethodName_("createMul"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createNSWMul), KType_Value, KType_IRBuilder, KMethodName_("createNSWMul"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createNUWMul), KType_Value, KType_IRBuilder, KMethodName_("createNUWMul"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createFMul),   KType_Value, KType_IRBuilder, KMethodName_("createFMul"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createUDiv),   KType_Value, KType_IRBuilder, KMethodName_("createUDiv"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createExactUDiv), KType_Value, KType_IRBuilder, KMethodName_("createExactUDiv"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createSDiv),      KType_Value, KType_IRBuilder, KMethodName_("createSDiv"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createExactSDiv), KType_Value, KType_IRBuilder, KMethodName_("createExactSDiv"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createFDiv),   KType_Value, KType_IRBuilder, KMethodName_("createFDiv"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createURem),   KType_Value, KType_IRBuilder, KMethodName_("createURem"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createSRem),   KType_Value, KType_IRBuilder, KMethodName_("createSRem"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createFRem),   KType_Value, KType_IRBuilder, KMethodName_("createFRem"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createShl),    KType_Value, KType_IRBuilder, KMethodName_("createShl"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createLShr),   KType_Value, KType_IRBuilder, KMethodName_("createLShr"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createAShr),   KType_Value, KType_IRBuilder, KMethodName_("createAShr"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createAnd),    KType_Value, KType_IRBuilder, KMethodName_("createAnd"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createOr),     KType_Value, KType_IRBuilder, KMethodName_("createOr"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createXor),    KType_Value, KType_IRBuilder, KMethodName_("createXor"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createNeg),    KType_Value, KType_IRBuilder, KMethodName_("createNeg"), 1, KType_Value, KFieldName_("v"),
		_Public, _F(IRBuilder_createNSWNeg), KType_Value, KType_IRBuilder, KMethodName_("createNSWNeg"), 1, KType_Value, KFieldName_("v"),
		_Public, _F(IRBuilder_createNUWNeg), KType_Value, KType_IRBuilder, KMethodName_("createNUWNeg"), 1, KType_Value, KFieldName_("v"),
		_Public, _F(IRBuilder_createFNeg),   KType_Value, KType_IRBuilder, KMethodName_("createFNeg"), 1, KType_Value, KFieldName_("v"),
		_Public, _F(IRBuilder_createNot),    KType_Value, KType_IRBuilder, KMethodName_("createNot"), 1, KType_Value, KFieldName_("v"),
		_Public, _F(IRBuilder_createAlloca), KType_Value, KType_IRBuilder, KMethodName_("createAlloca"), 2, KType_Type, KFieldName_("ty"),KType_Value, KFieldName_("arraySize"),
		_Public, _F(AllocaInst_new), KType_AllocaInst, KType_AllocaInst, KMethodName_("new"), 2, KType_Type, KFieldName_("ty"),KType_Value, KFieldName_("arraySize"),
		_Public, _F(IRBuilder_createLoad), KType_Value, KType_IRBuilder, KMethodName_("createLoad"), 2, KType_Value, KFieldName_("ptr"),KType_Boolean, KFieldName_("isVolatile"),
		_Public, _F(LoadInst_new), KType_LoadInst, KType_LoadInst, KMethodName_("new"), 1, KType_Value, KFieldName_("ptr"),
		_Public, _F(IRBuilder_createStore), KType_Value, KType_IRBuilder, KMethodName_("createStore"), 3, KType_Value, KFieldName_("val"),KType_Value, KFieldName_("ptr"),KType_Boolean, KFieldName_("isVolatile"),
		_Public, _F(StoreInst_new), KType_StoreInst, KType_StoreInst, KMethodName_("new"), 2, KType_Value, KFieldName_("val"),KType_Value, KFieldName_("ptr"),
		_Public|_Static, _F(GetElementPtrInst_create), KType_GetElementPtrInst, KType_GetElementPtrInst, KMethodName_("create"), 2, KType_Value, KFieldName_("ptr"),KType_Array_Value, KFieldName_("idxList"),
		_Public|_Static, _F(GetElementPtrInst_createInBounds), KType_GetElementPtrInst, KType_GetElementPtrInst, KMethodName_("createInBounds"), 2, KType_Value, KFieldName_("ptr"),KType_Array_Value, KFieldName_("idxList"),
		_Public, _F(IRBuilder_createGEP), KType_Value, KType_IRBuilder, KMethodName_("createGEP"), 2, KType_Value, KFieldName_("ptr"),KType_Array_Value, KFieldName_("idxList"),
		_Public, _F(IRBuilder_createInBoundsGEP), KType_Value, KType_IRBuilder, KMethodName_("createInBoundsGEP"), 2, KType_Value, KFieldName_("ptr"),KType_Array_Value, KFieldName_("idxList"),
		_Public, _F(IRBuilder_createGEP1), KType_Value, KType_IRBuilder, KMethodName_("createGEP1"), 2, KType_Value, KFieldName_("ptr"),KType_Value, KFieldName_("idx"),
		_Public, _F(IRBuilder_createInBoundsGEP1), KType_Value, KType_IRBuilder, KMethodName_("createInBoundsGEP1"), 2, KType_Value, KFieldName_("ptr"),KType_Value, KFieldName_("idx"),
		_Public, _F(IRBuilder_createConstGEP132),  KType_Value, KType_IRBuilder, KMethodName_("createConstGEP1_32"), 2, KType_Value, KFieldName_("ptr"),KType_Int, KFieldName_("idx0"),
		_Public, _F(IRBuilder_createConstGEP232),  KType_Value, KType_IRBuilder, KMethodName_("createConstGEP2_32"), 3, KType_Value, KFieldName_("ptr"),KType_Int, KFieldName_("idx0"),KType_Int, KFieldName_("idx1"),
		_Public, _F(IRBuilder_createConstGEP164),  KType_Value, KType_IRBuilder, KMethodName_("createConstGEP1_64"), 2, KType_Value, KFieldName_("ptr"),KType_Int, KFieldName_("idx0"),
		_Public, _F(IRBuilder_createConstGEP264),  KType_Value, KType_IRBuilder, KMethodName_("createConstGEP2_64"), 3, KType_Value, KFieldName_("ptr"),KType_Int, KFieldName_("idx0"),KType_Int, KFieldName_("idx1"),
		_Public, _F(IRBuilder_createConstInBoundsGEP132), KType_Value, KType_IRBuilder, KMethodName_("createConstInBoundsGEP1_32"), 2, KType_Value, KFieldName_("ptr"),KType_Int, KFieldName_("idx0"),
		_Public, _F(IRBuilder_createConstInBoundsGEP232), KType_Value, KType_IRBuilder, KMethodName_("createConstInBoundsGEP2_32"), 3, KType_Value, KFieldName_("ptr"),KType_Int, KFieldName_("idx0"),KType_Int, KFieldName_("idx1"),
		_Public, _F(IRBuilder_createConstInBoundsGEP164), KType_Value, KType_IRBuilder, KMethodName_("createConstInBoundsGEP1_64"), 2, KType_Value, KFieldName_("ptr"),KType_Int, KFieldName_("idx0"),
		_Public, _F(IRBuilder_createConstInBoundsGEP264), KType_Value, KType_IRBuilder, KMethodName_("createConstInBoundsGEP2_64"), 3, KType_Value, KFieldName_("ptr"),KType_Int, KFieldName_("idx0"),KType_Int, KFieldName_("idx1"),
		_Public, _F(IRBuilder_createStructGEP), KType_Value, KType_IRBuilder, KMethodName_("createStructGEP"), 2, KType_Value, KFieldName_("ptr"),KType_Int, KFieldName_("idx"),
		_Public, _F(IRBuilder_createGlobalString), KType_Value, KType_IRBuilder, KMethodName_("createGlobalString"), 1, KType_String, KFieldName_("str"),
		_Public, _F(IRBuilder_createGlobalStringPtr), KType_Value, KType_IRBuilder, KMethodName_("createGlobalStringPtr"), 1, KType_String, KFieldName_("str"),
		_Public, _F(IRBuilder_createTrunc),    KType_Value, KType_IRBuilder, KMethodName_("createTrunc"), 2, KType_Value, KFieldName_("v"),KType_Type, KFieldName_("destTy"),
		_Public, _F(IRBuilder_createZExt),     KType_Value, KType_IRBuilder, KMethodName_("createZExt"), 2, KType_Value, KFieldName_("v"),KType_Type, KFieldName_("destTy"),
		_Public, _F(IRBuilder_createSExt),     KType_Value, KType_IRBuilder, KMethodName_("createSExt"), 2, KType_Value, KFieldName_("v"),KType_Type, KFieldName_("destTy"),
		_Public, _F(IRBuilder_createFPToUI),   KType_Value, KType_IRBuilder, KMethodName_("createFPToUI"), 2, KType_Value, KFieldName_("v"),KType_Type, KFieldName_("destTy"),
		_Public, _F(IRBuilder_createFPToSI),   KType_Value, KType_IRBuilder, KMethodName_("createFPToSI"), 2, KType_Value, KFieldName_("v"),KType_Type, KFieldName_("destTy"),
		_Public, _F(IRBuilder_createUIToFP),   KType_Value, KType_IRBuilder, KMethodName_("createUIToFP"), 2, KType_Value, KFieldName_("v"),KType_Type, KFieldName_("destTy"),
		_Public, _F(IRBuilder_createSIToFP),   KType_Value, KType_IRBuilder, KMethodName_("createSIToFP"), 2, KType_Value, KFieldName_("v"),KType_Type, KFieldName_("destTy"),
		_Public, _F(IRBuilder_createFPTrunc),  KType_Value, KType_IRBuilder, KMethodName_("createFPTrunc"), 2, KType_Value, KFieldName_("v"),KType_Type, KFieldName_("destTy"),
		_Public, _F(IRBuilder_createFPExt),    KType_Value, KType_IRBuilder, KMethodName_("createFPExt"), 2, KType_Value, KFieldName_("v"),KType_Type, KFieldName_("destTy"),
		_Public, _F(IRBuilder_createPtrToInt), KType_Value, KType_IRBuilder, KMethodName_("createPtrToInt"), 2, KType_Value, KFieldName_("v"),KType_Type, KFieldName_("destTy"),
		_Public, _F(IRBuilder_createIntToPtr), KType_Value, KType_IRBuilder, KMethodName_("createIntToPtr"), 2, KType_Value, KFieldName_("v"),KType_Type, KFieldName_("destTy"),
		_Public, _F(IRBuilder_createBitCast),  KType_Value, KType_IRBuilder, KMethodName_("createBitCast"), 2, KType_Value, KFieldName_("v"),KType_Type, KFieldName_("destTy"),
		_Public, _F(IRBuilder_createZExtOrBitCast),  KType_Value, KType_IRBuilder, KMethodName_("createZExtOrBitCast"), 2, KType_Value, KFieldName_("v"),KType_Type, KFieldName_("destTy"),
		_Public, _F(IRBuilder_createSExtOrBitCast),  KType_Value, KType_IRBuilder, KMethodName_("createSExtOrBitCast"), 2, KType_Value, KFieldName_("v"),KType_Type, KFieldName_("destTy"),
		_Public, _F(IRBuilder_createTruncOrBitCast), KType_Value, KType_IRBuilder, KMethodName_("createTruncOrBitCast"), 2, KType_Value, KFieldName_("v"),KType_Type, KFieldName_("destTy"),
		_Public, _F(IRBuilder_createPointerCast),    KType_Value, KType_IRBuilder, KMethodName_("createPointerCast"), 2, KType_Value, KFieldName_("v"),KType_Type, KFieldName_("destTy"),
		_Public, _F(IRBuilder_createIntCast), KType_Value, KType_IRBuilder, KMethodName_("createIntCast"), 3, KType_Value, KFieldName_("v"),KType_Type, KFieldName_("destTy"),KType_Boolean, KFieldName_("isSigned"),
		_Public, _F(IRBuilder_createFPCast),  KType_Value, KType_IRBuilder, KMethodName_("createFPCast"), 2, KType_Value, KFieldName_("v"),KType_Type, KFieldName_("destTy"),
		_Public, _F(IRBuilder_createICmpEQ),  KType_Value, KType_IRBuilder, KMethodName_("createICmpEQ"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createICmpNE),  KType_Value, KType_IRBuilder, KMethodName_("createICmpNE"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createICmpUGT), KType_Value, KType_IRBuilder, KMethodName_("createICmpUGT"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createICmpUGE), KType_Value, KType_IRBuilder, KMethodName_("createICmpUGE"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createICmpULT), KType_Value, KType_IRBuilder, KMethodName_("createICmpULT"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createICmpULE), KType_Value, KType_IRBuilder, KMethodName_("createICmpULE"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createICmpSGT), KType_Value, KType_IRBuilder, KMethodName_("createICmpSGT"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createICmpSGE), KType_Value, KType_IRBuilder, KMethodName_("createICmpSGE"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createICmpSLT), KType_Value, KType_IRBuilder, KMethodName_("createICmpSLT"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createICmpSLE), KType_Value, KType_IRBuilder, KMethodName_("createICmpSLE"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createFCmpOEQ), KType_Value, KType_IRBuilder, KMethodName_("createFCmpOEQ"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createFCmpOGT), KType_Value, KType_IRBuilder, KMethodName_("createFCmpOGT"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createFCmpOGE), KType_Value, KType_IRBuilder, KMethodName_("createFCmpOGE"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createFCmpOLT), KType_Value, KType_IRBuilder, KMethodName_("createFCmpOLT"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createFCmpOLE), KType_Value, KType_IRBuilder, KMethodName_("createFCmpOLE"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createFCmpONE), KType_Value, KType_IRBuilder, KMethodName_("createFCmpONE"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createFCmpORD), KType_Value, KType_IRBuilder, KMethodName_("createFCmpORD"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createFCmpUNO), KType_Value, KType_IRBuilder, KMethodName_("createFCmpUNO"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createFCmpUEQ), KType_Value, KType_IRBuilder, KMethodName_("createFCmpUEQ"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createFCmpUGT), KType_Value, KType_IRBuilder, KMethodName_("createFCmpUGT"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createFCmpUGE), KType_Value, KType_IRBuilder, KMethodName_("createFCmpUGE"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createFCmpULT), KType_Value, KType_IRBuilder, KMethodName_("createFCmpULT"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createFCmpULE), KType_Value, KType_IRBuilder, KMethodName_("createFCmpULE"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createFCmpUNE), KType_Value, KType_IRBuilder, KMethodName_("createFCmpUNE"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_createPHI),   KType_PHINode, KType_IRBuilder, KMethodName_("createPHI"), 2, KType_Type, KFieldName_("ty"),KType_Int, KFieldName_("numReservedValues"),
		_Public, _F(PHINode_addIncoming),   KType_void, KType_PHINode, KMethodName_("addIncoming"), 2, KType_Value, KFieldName_("v"),KType_BasicBlock, KFieldName_("bb"),
		_Public, _F(IRBuilder_createCall1), KType_Value, KType_IRBuilder, KMethodName_("createCall1"), 2, KType_Value, KFieldName_("callee"),KType_Value, KFieldName_("arg"),
		_Public, _F(IRBuilder_createCall2), KType_Value, KType_IRBuilder, KMethodName_("createCall2"), 3, KType_Value, KFieldName_("callee"),KType_Value, KFieldName_("arg1"),KType_Value, KFieldName_("arg2"),
		_Public, _F(IRBuilder_createCall3), KType_Value, KType_IRBuilder, KMethodName_("createCall3"), 4, KType_Value, KFieldName_("callee"),KType_Value, KFieldName_("arg1"),KType_Value, KFieldName_("arg2"),KType_Value, KFieldName_("arg3"),
		_Public, _F(IRBuilder_createCall4), KType_Value, KType_IRBuilder, KMethodName_("createCall4"), 5, KType_Value, KFieldName_("callee"),KType_Value, KFieldName_("arg1"),KType_Value, KFieldName_("arg2"),KType_Value, KFieldName_("arg3"),KType_Value, KFieldName_("arg4"),
		_Public, _F(IRBuilder_createCall5), KType_Value, KType_IRBuilder, KMethodName_("createCall5"), 6, KType_Value, KFieldName_("callee"),KType_Value, KFieldName_("arg1"),KType_Value, KFieldName_("arg2"),KType_Value, KFieldName_("arg3"),KType_Value, KFieldName_("arg4"),KType_Value, KFieldName_("arg5"),
		_Public, _F(IRBuilder_createCall),  KType_Value, KType_IRBuilder, KMethodName_("createCall"), 2, KType_Value, KFieldName_("callee"),KType_Array_Value, KFieldName_("args"),
		_Public, _F(IRBuilder_createSelect), KType_Value, KType_IRBuilder, KMethodName_("createSelect"), 3, KType_Value, KFieldName_("c"),KType_Value, KFieldName_("trueV"),KType_Value, KFieldName_("falseV"),
		_Public, _F(IRBuilder_createVAArg),  KType_Value, KType_IRBuilder, KMethodName_("createVAArg"), 2, KType_Value, KFieldName_("list"),KType_Type, KFieldName_("ty"),
		_Public, _F(IRBuilder_createExtractElement), KType_Value, KType_IRBuilder, KMethodName_("createExtractElement"), 2, KType_Value, KFieldName_("vec"),KType_Value, KFieldName_("idx"),
		_Public, _F(IRBuilder_createInsertElement),  KType_Value, KType_IRBuilder, KMethodName_("createInsertElement"), 3, KType_Value, KFieldName_("vec"),KType_Value, KFieldName_("newElt"),KType_Value, KFieldName_("idx"),
		_Public, _F(IRBuilder_createShuffleVector),  KType_Value, KType_IRBuilder, KMethodName_("createShuffleVector"), 3, KType_Value, KFieldName_("v1"),KType_Value, KFieldName_("v2"),KType_Value, KFieldName_("mask"),
		_Public, _F(IRBuilder_createIsNull),    KType_Value, KType_IRBuilder, KMethodName_("createIsNull"), 1, KType_Value, KFieldName_("arg"),
		_Public, _F(IRBuilder_createIsNotNull), KType_Value, KType_IRBuilder, KMethodName_("createIsNotNull"), 1, KType_Value, KFieldName_("arg"),
		_Public, _F(IRBuilder_createPtrDiff),   KType_Value, KType_IRBuilder, KMethodName_("createPtrDiff"), 2, KType_Value, KFieldName_("lhs"),KType_Value, KFieldName_("rhs"),
		_Public, _F(IRBuilder_SetInsertPoint),  KType_Value, KType_IRBuilder, KMethodName_("setInsertPoint"), 1, KType_BasicBlock, KFieldName_("bb"),
		_Public, _F(IRBuilder_getInsertBlock),  KType_BasicBlock, KType_IRBuilder, KMethodName_("getInsertBlock"), 0,
		_Public, _F(BasicBlock_getParent), KType_Function, KType_BasicBlock, KMethodName_("getParent"), 0,
		_Public, _F(BasicBlock_insertBefore), KType_void, KType_BasicBlock, KMethodName_("insertBefore"), 2, KType_Instruction, KFieldName_("before"),KType_Instruction, KFieldName_("inst"),
		_Public, _F(BasicBlock_getLastInst),  KType_Value/*TODO*/, KType_BasicBlock, KMethodName_("getLastInst"), 0,
		_Public, _F(BasicBlock_getTerminator), KType_Value/*TODO*/, KType_BasicBlock, KMethodName_("getTerminator"), 0,
		_Public, _F(Instruction_SetMetadata), KType_void, KType_Instruction, KMethodName_("setMetadata"), 3, KType_Module, KFieldName_("m"),KType_String, KFieldName_("name"),KType_Int, KFieldName_("value"),
		_Public, _F(Function_dump), KType_void, KType_Function, KMethodName_("dump"), 0,
		_Public, _F(Value_dump), KType_void, KType_Value, KMethodName_("dump"), 0,
		_Public, _F(Type_dump), KType_void, KType_Type, KMethodName_("dump"), 0,
		_Public, _F(BasicBlock_dump), KType_void, KType_BasicBlock, KMethodName_("dump"), 0,
		_Public|_Static, _F(Function_create), KType_Function, KType_Function, KMethodName_("create"), 4, KType_String, KFieldName_("name"),KType_FunctionType, KFieldName_("fnTy"),KType_Module, KFieldName_("m"),KType_Int, KFieldName_("linkage"),
		_Public, _F(Function_addFnAttr), KType_void, KType_Function, KMethodName_("addFnAttr"), 1, KType_Int, KFieldName_("attributes"),
		_Public, _F(BasicBlock_size), KType_Int, KType_BasicBlock, KMethodName_("size"), 0,
		_Public, _F(BasicBlock_empty), KType_Boolean, KType_BasicBlock, KMethodName_("empty"), 0,
		_Public, _F(Module_new), KType_Module, KType_Module, KMethodName_("new"), 1, KType_String, KFieldName_("name"),
		_Public, _F(Module_getTypeByName), KType_Type, KType_Module, KMethodName_("getTypeByName"), 1, KType_String, KFieldName_("name"),
		_Public, _F(Module_dump), KType_void, KType_Module, KMethodName_("dump"), 0,
		_Public, _F(Module_getOrInsertFunction), KType_Function, KType_Module, KMethodName_("getOrInsertFunction"), 2, KType_String, KFieldName_("name"),KType_FunctionType, KFieldName_("fnTy"),
		_Public, _F(Module_createExecutionEngine), KType_ExecutionEngine, KType_Module, KMethodName_("createExecutionEngine"), 1, KType_Int, KFieldName_("optLevel"),
		_Public|_Static, _F(BasicBlock_create), KType_BasicBlock, KType_BasicBlock, KMethodName_("create"), 2, KType_Function, KFieldName_("parent"),KType_String, KFieldName_("name"),
		_Public|_Static, _F(FunctionType_get), KType_FunctionType, KType_FunctionType, KMethodName_("get"), 3, KType_Type, KFieldName_("retTy"),KType_Array_Type, KFieldName_("args"),KType_Boolean, KFieldName_("b"),
		_Public|_Static, _F(ArrayType_get),    KType_Type, KType_ArrayType, KMethodName_("get"), 2, KType_Type, KFieldName_("t"),KType_Int, KFieldName_("elemSize"),
		_Public|_Static, _F(StructType_get),   KType_Type, KType_StructType, KMethodName_("get"), 2, KType_Array_Type, KFieldName_("args"),KType_Boolean, KFieldName_("isPacked"),
		_Public|_Static, _F(StructType_create), KType_Type, KType_StructType, KMethodName_("create"), 3, KType_Array_Type, KFieldName_("args"),KType_String, KFieldName_("name"),KType_Boolean, KFieldName_("isPacked"),
		_Public, _F(StructType_SetBody), KType_void, KType_StructType, KMethodName_("setBody"), 2, KType_Array_Type, KFieldName_("args"),KType_Boolean, KFieldName_("isPacked"),
		_Public, _F(StructType_isOpaque), KType_Boolean, KType_StructType, KMethodName_("isOpaque"), 0,
		_Public, _F(ExecutionEngine_getPointerToFunction), KType_NativeFunction, KType_ExecutionEngine, KMethodName_("getPointerToFunction"), 1, KType_Function, KFieldName_("func"),
		_Public, _F(ExecutionEngine_addGlobalMapping), KType_void, KType_ExecutionEngine, KMethodName_("addGlobalMapping"), 2, KType_GlobalVariable, KFieldName_("g"),KType_Int, KFieldName_("addr"),
		_Public, _F(GlobalVariable_new), KType_Value, KType_GlobalVariable, KMethodName_("new"), 5, KType_Module, KFieldName_("m"),KType_Type, KFieldName_("ty"),KType_Constant, KFieldName_("c"),KType_Int, KFieldName_("linkage"),KType_String, KFieldName_("name"),
#if LLVM_VERSION >= 300
		_Public, _F(PassManagerBuilder_new), KType_PassManagerBuilder, KType_PassManagerBuilder, KMethodName_("new"), 0,
		_Public, _F(PassManagerBuilder_populateModulePassManager), KType_void, KType_PassManagerBuilder, KMethodName_("populateModulePassManager"), 1, KType_PassManager, KFieldName_("manager"),
#endif
		_Public, _F(PassManager_new), KType_PassManager, KType_PassManager, KMethodName_("new"), 0,
		_Public, _F(FunctionPassManager_new), KType_FunctionPassManager, KType_FunctionPassManager, KMethodName_("new"), 1, KType_Module, KFieldName_("m"),
		_Public, _F(PassManager_run), KType_void, KType_PassManager, KMethodName_("run"), 1, KType_Function, KFieldName_("func"),
		_Public, _F(PassManager_addPass), KType_void, KType_PassManager, KMethodName_("addPass"), 1, KType_Pass, KFieldName_("p"),
		_Public, _F(PassManager_addImmutablePass), KType_void, KType_PassManager, KMethodName_("addImmutablePass"), 1, KType_ImmutablePass, KFieldName_("p"),
		_Public, _F(PassManager_addFunctionPass), KType_void, KType_PassManager, KMethodName_("addFunctionPass"), 1, KType_FunctionPass, KFieldName_("p"),
		_Public, _F(PassManager_addModulePass), KType_void, KType_PassManager, KMethodName_("addModulePass"), 1, KType_ModulePass, KFieldName_("p"),
		_Public, _F(FunctionPassManager_add), KType_void, KType_FunctionPassManager, KMethodName_("add"), 1, KType_Pass, KFieldName_("p"),
		_Public, _F(FunctionPassManager_run), KType_void, KType_FunctionPassManager, KMethodName_("run"), 1, KType_Function, KFieldName_("func"),
		_Public, _F(FunctionPassManager_doInitialization), KType_void, KType_FunctionPassManager, KMethodName_("doInitialization"), 0,
#if LLVM_VERSION < 302
		_Public, _F(ExecutionEngine_getTargetData), KType_String/*TODO*/, KType_ExecutionEngine, KMethodName_("getTargetData"), 0,
#endif
		_Public, _F(Argument_new), KType_Value/*TODO*/, KType_Argument, KMethodName_("new"), 1, KType_Type, KFieldName_("type"),
		_Public, _F(Value_replaceAllUsesWith), KType_void, KType_Value, KMethodName_("replaceAllUsesWith"), 1, KType_Value, KFieldName_("v"),
		_Public, _F(Value_SetName), KType_void, KType_Value, KMethodName_("setName"), 1, KType_String, KFieldName_("name"),
		_Public, _F(Value_getType), KType_Type, KType_Value, KMethodName_("getType"), 0,
		_Public, _F(Function_getArguments), KType_Array_Value, KType_Function, KMethodName_("getArguments"), 0,
		_Public, _F(Function_getReturnType), KType_Type, KType_Function, KMethodName_("getReturnType"), 0,
		_Public, _F(LoadInst_SetAlignment),  KType_void, KType_LoadInst, KMethodName_("setAlignment"), 1, KType_Int, KFieldName_("align"),
		_Public, _F(StoreInst_SetAlignment), KType_void, KType_StoreInst, KMethodName_("setAlignment"), 1, KType_Int, KFieldName_("align"),
		_Public, _F(kMethod_SetFunction), KType_void, KType_Method, KMethodName_("setFunction"), 1, KType_NativeFunction, KFieldName_("nf"),
		_Public|_Static, _F(ConstantInt_get),         KType_Constant, KType_ConstantInt, KMethodName_("getValue"), 2, KType_Type, KFieldName_("type"),KType_Int, KFieldName_("v"),
		_Public|_Static, _F(ConstantFP_get),          KType_Constant, KType_ConstantFP, KMethodName_("getValue"), 2, KType_Type, KFieldName_("type"),KType_float, KFieldName_("v"),
		_Public|_Static, _F(ConstantFP_get),          KType_Constant, KType_ConstantFP, KMethodName_("getValueFromBits"), 2, KType_Type, KFieldName_("type"),KType_Int, KFieldName_("v"),
		_Public|_Static, _F(ConstantPointerNull_get), KType_Constant, KType_ConstantPointerNull, KMethodName_("getValue"), 1, KType_Type, KFieldName_("type"),
		_Public|_Static, _F(ConstantStruct_get),      KType_Constant, KType_ConstantStruct, KMethodName_("getValue"), 2, KType_Type, KFieldName_("type"),KType_Array_Constant, KFieldName_("v"),
		_Public|_Static, _F(DynamicLibrary_loadLibraryPermanently),   KType_Boolean, KType_DynamicLibrary, KMethodName_("loadLibraryPermanently"), 1, KType_String, KFieldName_("libname"),
		_Public|_Static, _F(DynamicLibrary_searchForAddressOfSymbol), KType_Int, KType_DynamicLibrary, KMethodName_("searchForAddressOfSymbol"), 1, KType_String, KFieldName_("fname"),
		_Public|_Static, _F(LLVM_createDomPrinterPass),     KType_Pass, KType_LLVM, KMethodName_("createDomPrinterPass"), 0,
		_Public|_Static, _F(LLVM_createDomOnlyPrinterPass), KType_Pass, KType_LLVM, KMethodName_("createDomOnlyPrinterPass"), 0,
		_Public|_Static, _F(LLVM_createDomViewerPass),      KType_Pass, KType_LLVM, KMethodName_("createDomViewerPass"), 0,
		_Public|_Static, _F(LLVM_createDomOnlyViewerPass),  KType_Pass, KType_LLVM, KMethodName_("createDomOnlyViewerPass"), 0,
		_Public|_Static, _F(LLVM_createPostDomPrinterPass), KType_Pass, KType_LLVM, KMethodName_("createPostDomPrinterPass"), 0,
		_Public|_Static, _F(LLVM_createPostDomOnlyPrinterPass),     KType_Pass, KType_LLVM, KMethodName_("createPostDomOnlyPrinterPass"), 0,
		_Public|_Static, _F(LLVM_createPostDomViewerPass),          KType_Pass, KType_LLVM, KMethodName_("createPostDomViewerPass"), 0,
		_Public|_Static, _F(LLVM_createPostDomOnlyViewerPass),      KType_Pass, KType_LLVM, KMethodName_("createPostDomOnlyViewerPass"), 0,
		_Public|_Static, _F(LLVM_createGlobalsModRefPass),          KType_Pass, KType_LLVM, KMethodName_("createGlobalsModRefPass"), 0,
		_Public|_Static, _F(LLVM_createAliasDebugger),              KType_Pass, KType_LLVM, KMethodName_("createAliasDebugger"), 0,
		_Public|_Static, _F(LLVM_createAliasAnalysisCounterPass),   KType_Pass, KType_LLVM, KMethodName_("createAliasAnalysisCounterPass"), 0,
		_Public|_Static, _F(LLVM_createAAEvalPass),                 KType_Pass, KType_LLVM, KMethodName_("createAAEvalPass"), 0,
		_Public|_Static, _F(LLVM_createLibCallAliasAnalysisPass),   KType_Pass, KType_LLVM, KMethodName_("createLibCallAliasAnalysisPass"), 1, KType_LibCallInfo, KFieldName_("lci"),
		_Public|_Static, _F(LLVM_createScalarEvolutionAliasAnalysisPass), KType_Pass, KType_LLVM, KMethodName_("createScalarEvolutionAliasAnalysisPass"), 0,
		_Public|_Static, _F(LLVM_createProfileLoaderPass),          KType_Pass, KType_LLVM, KMethodName_("createProfileLoaderPass"), 0,
		_Public|_Static, _F(LLVM_createProfileEstimatorPass),       KType_Pass, KType_LLVM, KMethodName_("createProfileEstimatorPass"), 0,
		_Public|_Static, _F(LLVM_createProfileVerifierPass),        KType_Pass, KType_LLVM, KMethodName_("createProfileVerifierPass"), 0,
		_Public|_Static, _F(LLVM_createPathProfileLoaderPass),      KType_Pass, KType_LLVM, KMethodName_("createPathProfileLoaderPass"), 0,
		_Public|_Static, _F(LLVM_createPathProfileVerifierPass),    KType_Pass, KType_LLVM, KMethodName_("createPathProfileVerifierPass"), 0,
		_Public|_Static, _F(LLVM_createLazyValueInfoPass),          KType_Pass, KType_LLVM, KMethodName_("createLazyValueInfoPass"), 0,
		_Public|_Static, _F(LLVM_createLoopDependenceAnalysisPass), KType_Pass, KType_LLVM, KMethodName_("createLoopDependenceAnalysisPass"), 0,
		_Public|_Static, _F(LLVM_createInstCountPass),              KType_Pass, KType_LLVM, KMethodName_("createInstCountPass"), 0,
		_Public|_Static, _F(LLVM_createDbgInfoPrinterPass),         KType_Pass, KType_LLVM, KMethodName_("createDbgInfoPrinterPass"), 0,
		_Public|_Static, _F(LLVM_createRegionInfoPass),             KType_Pass, KType_LLVM, KMethodName_("createRegionInfoPass"), 0,
		_Public|_Static, _F(LLVM_createModuleDebugInfoPrinterPass), KType_Pass, KType_LLVM, KMethodName_("createModuleDebugInfoPrinterPass"), 0,
		_Public|_Static, _F(LLVM_createMemDepPrinter),              KType_Pass, KType_LLVM, KMethodName_("createMemDepPrinter"), 0,
		_Public|_Static, _F(LLVM_createPostDomTree),                KType_Pass, KType_LLVM, KMethodName_("createPostDomTree"), 0,
		_Public|_Static, _F(LLVM_createRegionViewerPass),           KType_Pass, KType_LLVM, KMethodName_("createRegionViewerPass"), 0,
		_Public|_Static, _F(LLVM_createRegionOnlyViewerPass),       KType_Pass, KType_LLVM, KMethodName_("createRegionOnlyViewerPass"), 0,
		_Public|_Static, _F(LLVM_createRegionPrinterPass),          KType_Pass, KType_LLVM, KMethodName_("createRegionPrinterPass"), 0,
		_Public|_Static, _F(LLVM_createRegionOnlyPrinterPass),      KType_Pass, KType_LLVM, KMethodName_("createRegionOnlyPrinterPass"), 0,
		_Public|_Static, _F(LLVM_createLintPass),                   KType_Pass, KType_LLVM, KMethodName_("createLintPass"), 0,
		_Public|_Static, _F(LLVM_createStripSymbolsPass),           KType_Pass, KType_LLVM, KMethodName_("createStripSymbolsPass"), 1, KType_Boolean, KFieldName_("onlyDebugInfo"),
		_Public|_Static, _F(LLVM_createStripNonDebugSymbolsPass),   KType_Pass, KType_LLVM, KMethodName_("createStripNonDebugSymbolsPass"), 0,
		_Public|_Static, _F(LLVM_createStripDeadDebugInfoPass),     KType_Pass, KType_LLVM, KMethodName_("createStripDeadDebugInfoPass"), 0,
		_Public|_Static, _F(LLVM_createConstantMergePass),          KType_Pass, KType_LLVM, KMethodName_("createConstantMergePass"), 0,
		_Public|_Static, _F(LLVM_createGlobalOptimizerPass),        KType_Pass, KType_LLVM, KMethodName_("createGlobalOptimizerPass"), 0,
		_Public|_Static, _F(LLVM_createGlobalDCEPass),              KType_Pass, KType_LLVM, KMethodName_("createGlobalDCEPass"), 0,
		_Public|_Static, _F(LLVM_createFunctionInliningPass),       KType_Pass, KType_LLVM, KMethodName_("createFunctionInliningPass"), 1, KType_Int, KFieldName_("threshold"),
		_Public|_Static, _F(LLVM_createAlwaysInlinerPass),          KType_Pass, KType_LLVM, KMethodName_("createAlwaysInlinerPass"), 0,
		_Public|_Static, _F(LLVM_createPruneEHPass),                KType_Pass, KType_LLVM, KMethodName_("createPruneEHPass"), 0,
		_Public|_Static, _F(LLVM_createInternalizePass),            KType_Pass, KType_LLVM, KMethodName_("createInternalizePass"), 1, KType_Boolean, KFieldName_("allButMain"),
		_Public|_Static, _F(LLVM_createDeadArgEliminationPass),     KType_Pass, KType_LLVM, KMethodName_("createDeadArgEliminationPass"), 0,
		_Public|_Static, _F(LLVM_createArgumentPromotionPass),      KType_Pass, KType_LLVM, KMethodName_("createArgumentPromotionPass"), 1, KType_Int, KFieldName_("maxElements"),
		_Public|_Static, _F(LLVM_createIPConstantPropagationPass),  KType_Pass, KType_LLVM, KMethodName_("createIPConstantPropagationPass"), 0,
		_Public|_Static, _F(LLVM_createIPSCCPPass),                 KType_Pass, KType_LLVM, KMethodName_("createIPSCCPPass"), 0,
		_Public|_Static, _F(LLVM_createLoopExtractorPass),          KType_Pass, KType_LLVM, KMethodName_("createLoopExtractorPass"), 0,
		_Public|_Static, _F(LLVM_createSingleLoopExtractorPass),    KType_Pass, KType_LLVM, KMethodName_("createSingleLoopExtractorPass"), 0,
		_Public|_Static, _F(LLVM_createBlockExtractorPass),         KType_Pass, KType_LLVM, KMethodName_("createBlockExtractorPass"), 0,
		_Public|_Static, _F(LLVM_createStripDeadPrototypesPass),    KType_Pass, KType_LLVM, KMethodName_("createStripDeadPrototypesPass"), 0,
		_Public|_Static, _F(LLVM_createFunctionAttrsPass),          KType_Pass, KType_LLVM, KMethodName_("createFunctionAttrsPass"), 0,
		_Public|_Static, _F(LLVM_createMergeFunctionsPass),         KType_Pass, KType_LLVM, KMethodName_("createMergeFunctionsPass"), 0,
		_Public|_Static, _F(LLVM_createPartialInliningPass),        KType_Pass, KType_LLVM, KMethodName_("createPartialInliningPass"), 0,
		_Public|_Static, _F(LLVM_createConstantPropagationPass),    KType_Pass, KType_LLVM, KMethodName_("createConstantPropagationPass"), 0,
		_Public|_Static, _F(LLVM_createSCCPPass),                   KType_Pass, KType_LLVM, KMethodName_("createSCCPPass"), 0,
		_Public|_Static, _F(LLVM_createDeadInstEliminationPass),    KType_Pass, KType_LLVM, KMethodName_("createDeadInstEliminationPass"), 0,
		_Public|_Static, _F(LLVM_createDeadCodeEliminationPass),    KType_Pass, KType_LLVM, KMethodName_("createDeadCodeEliminationPass"), 0,
		_Public|_Static, _F(LLVM_createDeadStoreEliminationPass),   KType_Pass, KType_LLVM, KMethodName_("createDeadStoreEliminationPass"), 0,
		_Public|_Static, _F(LLVM_createAggressiveDCEPass),          KType_Pass, KType_LLVM, KMethodName_("createAggressiveDCEPass"), 0,
		_Public|_Static, _F(LLVM_createScalarReplAggregatesPass),   KType_Pass, KType_LLVM, KMethodName_("createScalarReplAggregatesPass"), 1, KType_Int, KFieldName_("threshold"),
		_Public|_Static, _F(LLVM_createIndVarSimplifyPass),         KType_Pass, KType_LLVM, KMethodName_("createIndVarSimplifyPass"), 0,
		_Public|_Static, _F(LLVM_createInstructionCombiningPass),   KType_Pass, KType_LLVM, KMethodName_("createInstructionCombiningPass"), 0,
		_Public|_Static, _F(LLVM_createLICMPass),                   KType_Pass, KType_LLVM, KMethodName_("createLICMPass"), 0,
		_Public|_Static, _F(LLVM_createLoopUnswitchPass),           KType_Pass, KType_LLVM, KMethodName_("createLoopUnswitchPass"), 1, KType_Boolean, KFieldName_("optimizeForSize"),
		_Public|_Static, _F(LLVM_createLoopInstSimplifyPass),       KType_Pass, KType_LLVM, KMethodName_("createLoopInstSimplifyPass"), 0,
		_Public|_Static, _F(LLVM_createLoopUnrollPass),             KType_Pass, KType_LLVM, KMethodName_("createLoopUnrollPass"), 3, KType_Int, KFieldName_("threshold"),KType_Int, KFieldName_("count"),KType_Int, KFieldName_("allowPartial"),
		_Public|_Static, _F(LLVM_createLoopRotatePass),             KType_Pass, KType_LLVM, KMethodName_("createLoopRotatePass"), 0,
		_Public|_Static, _F(LLVM_createLoopIdiomPass),              KType_Pass, KType_LLVM, KMethodName_("createLoopIdiomPass"), 0,
		_Public|_Static, _F(LLVM_createPromoteMemoryToRegisterPass), KType_Pass, KType_LLVM, KMethodName_("createPromoteMemoryToRegisterPass"), 0,
		_Public|_Static, _F(LLVM_createDemoteRegisterToMemoryPass),  KType_Pass, KType_LLVM, KMethodName_("createDemoteRegisterToMemoryPass"), 0,
		_Public|_Static, _F(LLVM_createReassociatePass),             KType_Pass, KType_LLVM, KMethodName_("createReassociatePass"), 0,
		_Public|_Static, _F(LLVM_createJumpThreadingPass),           KType_Pass, KType_LLVM, KMethodName_("createJumpThreadingPass"), 0,
		_Public|_Static, _F(LLVM_createCFGSimplificationPass),       KType_Pass, KType_LLVM, KMethodName_("createCFGSimplificationPass"), 0,
		_Public|_Static, _F(LLVM_createBreakCriticalEdgesPass),      KType_Pass, KType_LLVM, KMethodName_("createBreakCriticalEdgesPass"), 0,
		_Public|_Static, _F(LLVM_createLoopSimplifyPass),            KType_Pass, KType_LLVM, KMethodName_("createLoopSimplifyPass"), 0,
		_Public|_Static, _F(LLVM_createTailCallEliminationPass),     KType_Pass, KType_LLVM, KMethodName_("createTailCallEliminationPass"), 0,
		_Public|_Static, _F(LLVM_createLowerSwitchPass),             KType_Pass, KType_LLVM, KMethodName_("createLowerSwitchPass"), 0,
		_Public|_Static, _F(LLVM_createBlockPlacementPass),          KType_Pass, KType_LLVM, KMethodName_("createBlockPlacementPass"), 0,
		_Public|_Static, _F(LLVM_createLCSSAPass),                   KType_Pass, KType_LLVM, KMethodName_("createLCSSAPass"), 0,
		_Public|_Static, _F(LLVM_createEarlyCSEPass),                KType_Pass, KType_LLVM, KMethodName_("createEarlyCSEPass"), 0,
		_Public|_Static, _F(LLVM_createGVNPass),                     KType_Pass, KType_LLVM, KMethodName_("createGVNPass"), 1, KType_Boolean, KFieldName_("noLoads"),
		_Public|_Static, _F(LLVM_createMemCpyOptPass),               KType_Pass, KType_LLVM, KMethodName_("createMemCpyOptPass"), 0,
		_Public|_Static, _F(LLVM_createLoopDeletionPass),            KType_Pass, KType_LLVM, KMethodName_("createLoopDeletionPass"), 0,
		_Public|_Static, _F(LLVM_createSimplifyLibCallsPass),        KType_Pass, KType_LLVM, KMethodName_("createSimplifyLibCallsPass"), 0,
		_Public|_Static, _F(LLVM_createInstructionNamerPass),        KType_Pass, KType_LLVM, KMethodName_("createInstructionNamerPass"), 0,
		_Public|_Static, _F(LLVM_createSinkingPass),                 KType_Pass, KType_LLVM, KMethodName_("createSinkingPass"), 0,
		_Public|_Static, _F(LLVM_createLowerAtomicPass),             KType_Pass, KType_LLVM, KMethodName_("createLowerAtomicPass"), 0,
		_Public|_Static, _F(LLVM_createCorrelatedValuePropagationPass), KType_Pass, KType_LLVM, KMethodName_("createCorrelatedValuePropagationPass"), 0,
#if LLVM_VERSION >= 300
		_Public|_Static, _F(LLVM_createObjCARCExpandPass),   KType_Pass, KType_LLVM, KMethodName_("createObjCARCExpandPass"), 0,
		_Public|_Static, _F(LLVM_createObjCARCContractPass), KType_Pass, KType_LLVM, KMethodName_("createObjCARCContractPass"), 0,
		_Public|_Static, _F(LLVM_createObjCARCOptPass),      KType_Pass, KType_LLVM, KMethodName_("createObjCARCOptPass"), 0,
		_Public|_Static, _F(LLVM_createLowerExpectIntrinsicPass), KType_Pass, KType_LLVM, KMethodName_("createLowerExpectIntrinsicPass"), 0,
#endif
#if LLVM_VERSION >= 301
		_Public|_Static, _F(LLVM_createBBVectorizePass),     KType_Pass, KType_LLVM, KMethodName_("createBBVectorizePass"), 0,
#endif
		_Public|_Static, _F(LLVM_createInstructionSimplifierPass),  KType_Pass, KType_LLVM, KMethodName_("createInstructionSimplifierPass"), 0,
		_Public|_Static, _F(LLVM_createUnifyFunctionExitNodesPass), KType_Pass, KType_LLVM, KMethodName_("createUnifyFunctionExitNodesPass"), 0,
		_Public|_Static, _F(LLVM_createTypeBasedAliasAnalysisPass), KType_Pass, KType_LLVM, KMethodName_("createTypeBasedAliasAnalysisPass"), 0,
		_Public|_Static, _F(LLVM_createBasicAliasAnalysisPass),     KType_Pass, KType_LLVM, KMethodName_("createBasicAliasAnalysisPass"), 0,
		_Public|_Static, _F(LLVM_createVerifierPass),               KType_Pass, KType_LLVM, KMethodName_("createVerifierPass"), 0,
		_Public|_Static, _F(Intrinsic_getType), KType_Type, KType_Intrinsic, KMethodName_("getType"), 2, KType_Int, KFieldName_("id"),KType_Array_Type, KFieldName_("args"),
		_Public|_Static, _F(Intrinsic_getDeclaration), KType_Function, KType_Intrinsic, KMethodName_("getDeclaration"), 3, KType_Module, KFieldName_("m"),KType_Int, KFieldName_("id"),KType_Array_Type, KFieldName_("args"),
		_Public|_Static, _F(LLVM_parseBitcodeFile), KType_Value, KType_LLVM, KMethodName_("parseBitcodeFile"), 1, KType_String, KFieldName_("bcfile"),

		_Public|_Static, _F(ConstantExpr_getAlignOf), KType_Constant, KType_ConstantExpr, KMethodName_("getAlignOf"), 1,KType_Type, KFieldName_("ty"),
		_Public|_Static, _F(ConstantExpr_getSizeOf), KType_Constant, KType_ConstantExpr, KMethodName_("getSizeOf"), 1,KType_Type, KFieldName_("ty"),
		_Public|_Static, _F(ConstantExpr_getOffsetOf), KType_Constant, KType_ConstantExpr, KMethodName_("getOffsetOf"), 2,KType_StructType, KFieldName_("sTy"), KType_Int, KFieldName_("fieldNo"),
		_Public|_Static, _F(ConstantExpr_getOffsetOf), KType_Constant, KType_ConstantExpr, KMethodName_("getOffsetOf"), 2,KType_Type, KFieldName_("ty"), KType_Constant, KFieldName_("fieldNo"),
		_Public|_Static, _F(ConstantExpr_getNeg), KType_Constant, KType_ConstantExpr, KMethodName_("getNeg"), 3,KType_Constant, KFieldName_("c"), KType_Boolean, KFieldName_("hasNUW"), KType_Boolean, KFieldName_("hasNSW"),
		_Public|_Static, _F(ConstantExpr_getFNeg), KType_Constant, KType_ConstantExpr, KMethodName_("getFNeg"), 1,KType_Constant, KFieldName_("c"),
		_Public|_Static, _F(ConstantExpr_getNot), KType_Constant, KType_ConstantExpr, KMethodName_("getNot"), 1,KType_Constant, KFieldName_("c"),
		_Public|_Static, _F(ConstantExpr_getAdd), KType_Constant, KType_ConstantExpr, KMethodName_("getAdd"), 4,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"), KType_Boolean, KFieldName_("hasNUW"), KType_Boolean, KFieldName_("hasNSW"),
		_Public|_Static, _F(ConstantExpr_getFAdd), KType_Constant, KType_ConstantExpr, KMethodName_("getFAdd"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getSub), KType_Constant, KType_ConstantExpr, KMethodName_("getSub"), 4,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"), KType_Boolean, KFieldName_("hasNUW"), KType_Boolean, KFieldName_("hasNSW"),
		_Public|_Static, _F(ConstantExpr_getFSub), KType_Constant, KType_ConstantExpr, KMethodName_("getFSub"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getMul), KType_Constant, KType_ConstantExpr, KMethodName_("getMul"), 4,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"), KType_Boolean, KFieldName_("hasNUW"), KType_Boolean, KFieldName_("hasNSW"),
		_Public|_Static, _F(ConstantExpr_getFMul), KType_Constant, KType_ConstantExpr, KMethodName_("getFMul"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getUDiv), KType_Constant, KType_ConstantExpr, KMethodName_("getUDiv"), 3,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"), KType_Boolean, KFieldName_("isExact"),
		_Public|_Static, _F(ConstantExpr_getSDiv), KType_Constant, KType_ConstantExpr, KMethodName_("getSDiv"), 3,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"), KType_Boolean, KFieldName_("isExact"),
		_Public|_Static, _F(ConstantExpr_getFDiv), KType_Constant, KType_ConstantExpr, KMethodName_("getFDiv"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getURem), KType_Constant, KType_ConstantExpr, KMethodName_("getURem"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getSRem), KType_Constant, KType_ConstantExpr, KMethodName_("getSRem"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getFRem), KType_Constant, KType_ConstantExpr, KMethodName_("getFRem"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getAnd), KType_Constant, KType_ConstantExpr, KMethodName_("getAnd"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getOr), KType_Constant, KType_ConstantExpr, KMethodName_("getOr"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getXor), KType_Constant, KType_ConstantExpr, KMethodName_("getXor"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getShl), KType_Constant, KType_ConstantExpr, KMethodName_("getShl"), 4,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"), KType_Boolean, KFieldName_("hasNUW"), KType_Boolean, KFieldName_("hasNSW"),
		_Public|_Static, _F(ConstantExpr_getLShr), KType_Constant, KType_ConstantExpr, KMethodName_("getLShr"), 3,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"), KType_Boolean, KFieldName_("isExact"),
		_Public|_Static, _F(ConstantExpr_getAShr), KType_Constant, KType_ConstantExpr, KMethodName_("getAShr"), 3,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"), KType_Boolean, KFieldName_("isExact"),
		_Public|_Static, _F(ConstantExpr_getTrunc), KType_Constant, KType_ConstantExpr, KMethodName_("getTrunc"), 2,KType_Constant, KFieldName_("c"), KType_Type, KFieldName_("ty"),
		_Public|_Static, _F(ConstantExpr_getSExt), KType_Constant, KType_ConstantExpr, KMethodName_("getSExt"), 2,KType_Constant, KFieldName_("c"), KType_Type, KFieldName_("ty"),
		_Public|_Static, _F(ConstantExpr_getZExt), KType_Constant, KType_ConstantExpr, KMethodName_("getZExt"), 2,KType_Constant, KFieldName_("c"), KType_Type, KFieldName_("ty"),
		_Public|_Static, _F(ConstantExpr_getFPTrunc), KType_Constant, KType_ConstantExpr, KMethodName_("getFPTrunc"), 2,KType_Constant, KFieldName_("c"), KType_Type, KFieldName_("ty"),
		_Public|_Static, _F(ConstantExpr_getFPExtend), KType_Constant, KType_ConstantExpr, KMethodName_("getFPExtend"), 2,KType_Constant, KFieldName_("c"), KType_Type, KFieldName_("ty"),
		_Public|_Static, _F(ConstantExpr_getUIToFP), KType_Constant, KType_ConstantExpr, KMethodName_("getUIToFP"), 2,KType_Constant, KFieldName_("c"), KType_Type, KFieldName_("ty"),
		_Public|_Static, _F(ConstantExpr_getSIToFP), KType_Constant, KType_ConstantExpr, KMethodName_("getSIToFP"), 2,KType_Constant, KFieldName_("c"), KType_Type, KFieldName_("ty"),
		_Public|_Static, _F(ConstantExpr_getFPToUI), KType_Constant, KType_ConstantExpr, KMethodName_("getFPToUI"), 2,KType_Constant, KFieldName_("c"), KType_Type, KFieldName_("ty"),
		_Public|_Static, _F(ConstantExpr_getFPToSI), KType_Constant, KType_ConstantExpr, KMethodName_("getFPToSI"), 2,KType_Constant, KFieldName_("c"), KType_Type, KFieldName_("ty"),
		_Public|_Static, _F(ConstantExpr_getPtrToInt), KType_Constant, KType_ConstantExpr, KMethodName_("getPtrToInt"), 2,KType_Constant, KFieldName_("c"), KType_Type, KFieldName_("ty"),
		_Public|_Static, _F(ConstantExpr_getIntToPtr), KType_Constant, KType_ConstantExpr, KMethodName_("getIntToPtr"), 2,KType_Constant, KFieldName_("c"), KType_Type, KFieldName_("ty"),
		_Public|_Static, _F(ConstantExpr_getBitCast), KType_Constant, KType_ConstantExpr, KMethodName_("getBitCast"), 2,KType_Constant, KFieldName_("c"), KType_Type, KFieldName_("ty"),
		_Public|_Static, _F(ConstantExpr_getNSWNeg), KType_Constant, KType_ConstantExpr, KMethodName_("getNSWNeg"), 1,KType_Constant, KFieldName_("c"),
		_Public|_Static, _F(ConstantExpr_getNUWNeg), KType_Constant, KType_ConstantExpr, KMethodName_("getNUWNeg"), 1,KType_Constant, KFieldName_("c"),
		_Public|_Static, _F(ConstantExpr_getNSWAdd), KType_Constant, KType_ConstantExpr, KMethodName_("getNSWAdd"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getNUWAdd), KType_Constant, KType_ConstantExpr, KMethodName_("getNUWAdd"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getNSWSub), KType_Constant, KType_ConstantExpr, KMethodName_("getNSWSub"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getNUWSub), KType_Constant, KType_ConstantExpr, KMethodName_("getNUWSub"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getNSWMul), KType_Constant, KType_ConstantExpr, KMethodName_("getNSWMul"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getNUWMul), KType_Constant, KType_ConstantExpr, KMethodName_("getNUWMul"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getNSWShl), KType_Constant, KType_ConstantExpr, KMethodName_("getNSWShl"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getNUWShl), KType_Constant, KType_ConstantExpr, KMethodName_("getNUWShl"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getExactSDiv), KType_Constant, KType_ConstantExpr, KMethodName_("getExactSDiv"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getExactUDiv), KType_Constant, KType_ConstantExpr, KMethodName_("getExactUDiv"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getExactAShr), KType_Constant, KType_ConstantExpr, KMethodName_("getExactAShr"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getExactLShr), KType_Constant, KType_ConstantExpr, KMethodName_("getExactLShr"), 2,KType_Constant, KFieldName_("c1"), KType_Constant, KFieldName_("c2"),
		_Public|_Static, _F(ConstantExpr_getZExtOrBitCast), KType_Constant, KType_ConstantExpr, KMethodName_("getZExtOrBitCast"), 2,KType_Constant, KFieldName_("c"), KType_Type, KFieldName_("ty"),
		_Public|_Static, _F(ConstantExpr_getSExtOrBitCast), KType_Constant, KType_ConstantExpr, KMethodName_("getSExtOrBitCast"), 2,KType_Constant, KFieldName_("c"), KType_Type, KFieldName_("ty"),
		_Public|_Static, _F(ConstantExpr_getTruncOrBitCast), KType_Constant, KType_ConstantExpr, KMethodName_("getTruncOrBitCast"), 2,KType_Constant, KFieldName_("c"), KType_Type, KFieldName_("ty"),
		_Public|_Static, _F(ConstantExpr_getPointerCast), KType_Constant, KType_ConstantExpr, KMethodName_("getPointerCast"), 2,KType_Constant, KFieldName_("c"), KType_Type, KFieldName_("ty"),
		_Public|_Static, _F(ConstantExpr_getIntegerCast), KType_Constant, KType_ConstantExpr, KMethodName_("getIntegerCast"), 3,KType_Constant, KFieldName_("c"), KType_Type, KFieldName_("ty"), KType_Boolean, KFieldName_("isSigned"),
		_Public|_Static, _F(ConstantExpr_getFPCast), KType_Constant, KType_ConstantExpr, KMethodName_("getFPCast"), 2,KType_Constant, KFieldName_("c"), KType_Type, KFieldName_("ty"),
		_Public|_Static, _F(ConstantExpr_getSelect), KType_Constant, KType_ConstantExpr, KMethodName_("getSelect"), 3,KType_Constant, KFieldName_("c"), KType_Constant, KFieldName_("v1"), KType_Constant, KFieldName_("v2"),
		_Public|_Static, _F(ConstantExpr_getElementPtr0), KType_Constant, KType_ConstantExpr, KMethodName_("getElementPtr"), 3,KType_Constant, KFieldName_("c"), KType_Constant, KFieldName_("idx"), KType_Boolean, KFieldName_("InBounds"),
		_Public|_Static, _F(ConstantExpr_getElementPtr), KType_Constant, KType_ConstantExpr, KMethodName_("getElementPtr"), 3,KType_Constant, KFieldName_("c"), KType_Array_Value, KFieldName_("IdxList"), KType_Boolean, KFieldName_("InBounds"),
		_Public|_Static, _F(ConstantExpr_getInBoundsGetElementPtr0), KType_Constant, KType_ConstantExpr, KMethodName_("getInBoundsGetElementPtr0"), 2,KType_Constant, KFieldName_("c"), KType_Constant, KFieldName_("idx"),
		_Public|_Static, _F(ConstantExpr_getInBoundsGetElementPtr), KType_Constant, KType_ConstantExpr, KMethodName_("getInBoundsGetElementPtr"), 2,KType_Constant, KFieldName_("c"), KType_Array_Value, KFieldName_("idxList"),
		_Public|_Static, _F(ConstantExpr_getExtractElement), KType_Constant, KType_ConstantExpr, KMethodName_("getExtractElement"), 2,KType_Constant, KFieldName_("vec"), KType_Constant, KFieldName_("idx"),
		_Public|_Static, _F(ConstantExpr_getInsertElement), KType_Constant, KType_ConstantExpr, KMethodName_("getInsertElement"), 3,KType_Constant, KFieldName_("vec"), KType_Constant, KFieldName_("elt"), KType_Constant, KFieldName_("idx"),
		_Public|_Static, _F(ConstantExpr_getShuffleVector), KType_Constant, KType_ConstantExpr, KMethodName_("getShuffleVector"), 3,KType_Constant, KFieldName_("v1"), KType_Constant, KFieldName_("v2"), KType_Constant, KFieldName_("mask"),
		_Public|_Static, _F(ConstantExpr_getExtractValue), KType_Constant, KType_ConstantExpr, KMethodName_("getExtractValue"), 2,KType_Constant, KFieldName_("Agg"), KType_Array_Int, KFieldName_("idxs"),
		_Public|_Static, _F(ConstantExpr_getInsertValue), KType_Constant, KType_ConstantExpr, KMethodName_("getInsertValue"), 3,KType_Constant, KFieldName_("Agg"), KType_Constant, KFieldName_("val"), KType_Array_Int, KFieldName_("idxs"),
		_Public, _F(Type_opEQ), KType_Boolean, KType_Type, KMethodName_("=="), 1,KType_Type, KFieldName_("t"),
		//FIXME
	//_Public|_Const|_Im|_Coercion, _F(Float_toInt), KType_Int, KType_float, KMethodName_To(KType_Int), 0,
		_Public|_Const|_Coercion|_Im, _F(Object_toValue), KType_Value, KType_Object, KMethodName_To(KType_Value), 0,
		_Public|_Const|_Coercion|_Im, _F(Object_toType),  KType_Type,  KType_Object, KMethodName_To(KType_Type), 0,
		_Public|_Const|_Coercion|_Im, _F(Object_toModule), KType_Module,  KType_Object, KMethodName_To(KType_Module), 0,
		_Public|_Const|_Coercion|_Im, _F(Object_toExecutionEngine),  KType_ExecutionEngine,  KType_Object, KMethodName_To(KType_ExecutionEngine), 0,
		_Public|_Const|_Coercion|_Im, _F(Object_asFunction), KType_Function, KType_Object, KMethodName_To(KType_Function), 0,
		_Public|_Const|_Coercion|_Im, _F(Object_toIRBuilder), KType_IRBuilder, KType_Object, KMethodName_To(KType_IRBuilder), 0,
		_Public|_Const|_Coercion|_Im, _F(Object_asFunctionType), KType_FunctionType, KType_Object, KMethodName_To(KType_FunctionType), 0,
		_Public|_Const|_Coercion|_Im, _F(Object_toLLVMBasicBlock), KType_BasicBlock, KType_Object, KMethodName_To(KType_BasicBlock), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, methoddata, trace);
	KLIB kNameSpace_LoadConstData(kctx, ns, (const char **)IntAttributes, trace);
	KLIB kNameSpace_LoadConstData(kctx, ns, (const char **)IntIntrinsic, trace);
	KLIB kNameSpace_LoadConstData(kctx, ns, (const char **)IntGlobalVariable, trace);

	return true;
}

static kbool_t llvm_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	(void)kctx;(void)ns;(void)exportNS;(void)trace;(void)option;
	return true;
}

KDEFINE_PACKAGE *LLVM_Init(void)
{
	InitializeNativeTarget();
	static KDEFINE_PACKAGE d = {
		0,
		"llvm", "3.0", "", "",
		llvm_PackupNameSpace,
		llvm_ExportNameSpace,
	};

	return &d;
}

#ifdef __cplusplus
}
#endif
