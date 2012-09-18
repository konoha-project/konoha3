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
#include "llvm/Target/TargetData.h"
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
#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/float.h>
#include <stdio.h>

struct kRawPtr {
	KonohaObjectHeader h;
	void *rawptr;
};

#define MOD_llvm 19/*TODO*/
#define kllvmmod ((kllvmmod_t*)kctx->mod[MOD_llvm])
#define kmodllvm ((kmodllvm_t*)kctx->modshare[MOD_llvm])
#define CT_Value (kmodllvm)->cValue
#define TY_Value (CT_Value)->typeId

typedef struct {
	KonohaModule h;
	KonohaClass *cValue;
} kmodllvm_t;

typedef struct {
	KonohaModuleContext h;
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
		T v = konoha::object_cast<T>(a->objectItems[i]);
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

#define LLVM_TODO(str) do {\
	fprintf(stderr, "(TODO: %s %d):", __func__, __LINE__);\
	fprintf(stderr, "%s\n", str);\
	abort();\
} while (0)

#define LLVM_WARN(str) do {\
	fprintf(stderr, "(WARN: %s %d):", __func__, __LINE__);\
	fprintf(stderr, "%s\n", str);\
} while (0)

#define _UNUSED_ __attribute__((unused))

#define PKG_NULVAL(T) PKG_NULVAL_##T
#define PKG_NULVAL_int    (0)
#define PKG_NULVAL_float  (0.0)
#define PKG_NULVAL_String (KNH_NULVAL(TY_String))
#define WRAP(ptr) ((void*)ptr)
#define Int_to(T, a)               ((T)a.intValue)
#define DEFAPI(T) T

static void Type_init(KonohaContext *kctx _UNUSED_, kObject *po, void *conf)
{
	konoha::SetRawPtr(po, conf);
}
static void Type_free(KonohaContext *kctx _UNUSED_, kObject *po)
{
	konoha::SetRawPtr(po, NULL);
}

static inline kObject *new_CppObject(KonohaContext *kctx, const KonohaClass *ct, void *ptr)
{
	kObject *ret = KLIB new_kObject(kctx, ct, (uintptr_t)ptr);
	konoha::SetRawPtr(ret, ptr);
	return ret;
}

static inline kObject *new_ReturnCppObject(KonohaContext *kctx, KonohaStack *sfp, void *ptr)
{
	kObject *defobj = sfp[(-(K_CALLDELTA))].o;
	kObject *ret = KLIB new_kObject(kctx, O_ct(defobj), (uintptr_t)ptr);
	konoha::SetRawPtr(ret, ptr);
	return ret;
}

//## @Const method Boolean Type.opEQ(Type value);
static KMETHOD Type_opEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	Type *p1 = konoha::object_cast<Type *>(sfp[0].asObject);
	Type *p2 = konoha::object_cast<Type *>(sfp[1].asObject);
	RETURNb_(p1 == p2);
}

//## @Static Type Type.getVoidTy();
static KMETHOD Type_getVoidTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getVoidTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static Type Type.getLabelTy();
static KMETHOD Type_getLabelTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getLabelTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static Type Type.getFloatTy();
static KMETHOD Type_getFloatTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getFloatTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static Type Type.getDoubleTy();
static KMETHOD Type_getDoubleTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getDoubleTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static Type Type.getMetadataTy();
static KMETHOD Type_getMetadataTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getMetadataTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static Type Type.getX86FP80Ty();
static KMETHOD Type_getX86FP80Ty(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getX86_FP80Ty(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static Type Type.getFP128Ty();
static KMETHOD Type_getFP128Ty(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getFP128Ty(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static Type Type.getPPCFP128Ty();
static KMETHOD Type_getPPCFP128Ty(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getPPC_FP128Ty(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static Type Type.getX86MMXTy();
static KMETHOD Type_getX86MMXTy(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;(void)(-(K_CALLDELTA));
	LLVM_TODO("NO SUPPORT");
#else
	const Type *ptr = Type::getX86_MMXTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
#endif
}

//## @Static IntegerType Type.getInt1Ty();
static KMETHOD Type_getInt1Ty(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getInt1Ty(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static IntegerType Type.getInt8Ty();
static KMETHOD Type_getInt8Ty(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getInt8Ty(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static IntegerType Type.getInt16Ty();
static KMETHOD Type_getInt16Ty(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getInt16Ty(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static IntegerType Type.getInt32Ty();
static KMETHOD Type_getInt32Ty(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getInt32Ty(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static IntegerType Type.getInt64Ty();
static KMETHOD Type_getInt64Ty(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getInt64Ty(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static PointerType Type.getFloatPtrTy();
static KMETHOD Type_getFloatPtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getFloatPtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static PointerType Type.getDoublePtrTy();
static KMETHOD Type_getDoublePtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getDoublePtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static PointerType Type.getX86FP80PtrTy();
static KMETHOD Type_getX86FP80PtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getX86_FP80PtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static PointerType Type.getFP128PtrTy();
static KMETHOD Type_getFP128PtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getFP128PtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static PointerType Type.getPPCFP128PtrTy();
static KMETHOD Type_getPPCFP128PtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getPPC_FP128PtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static PointerType Type.getX86MMXPtrTy();
static KMETHOD Type_getX86MMXPtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;(void)(-(K_CALLDELTA));
	LLVM_TODO("NO SUPPORT");
#else
	const Type *ptr = Type::getX86_MMXPtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
#endif
}

//## @Static PointerType Type.getInt1PtrTy();
static KMETHOD Type_getInt1PtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getInt1PtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static PointerType Type.getInt8PtrTy();
static KMETHOD Type_getInt8PtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getInt8PtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static PointerType Type.getInt16PtrTy();
static KMETHOD Type_getInt16PtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getInt16PtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static PointerType Type.getInt32PtrTy();
static KMETHOD Type_getInt32PtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getInt32PtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static PointerType Type.getInt64PtrTy();
static KMETHOD Type_getInt64PtrTy(KonohaContext *kctx, KonohaStack *sfp)
{
	const Type *ptr = Type::getInt64PtrTy(getGlobalContext());
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static PointerType PointerType.get(Type type);
static KMETHOD PointerType_get(KonohaContext *kctx, KonohaStack *sfp)
{
	Type *type = konoha::object_cast<Type *>(sfp[1].asObject);
	const Type *ptr  = PointerType::get(type, 0);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## IRBuilder IRBuilder.new(BasicBlock bb);
static KMETHOD IRBuilder_new(KonohaContext *kctx, KonohaStack *sfp)
{
	BasicBlock *bb = konoha::object_cast<BasicBlock *>(sfp[1].asObject);
	IRBuilder<> *self = new IRBuilder<>(bb);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(self));
	RETURN_(p);
}

//## ReturnInst IRBuilder.CreateRetVoid();
static KMETHOD IRBuilder_createRetVoid(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	ReturnInst *ptr = self->CreateRetVoid();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## ReturnInst IRBuilder.CreateRet(Value V);
static KMETHOD IRBuilder_createRet(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	ReturnInst *ptr = self->CreateRet(V);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

////## ReturnInst IRBuilder.CreateAggregateRet(Value retVals, int N);
//KMETHOD IRBuilder_createAggregateRet(KonohaContext *kctx, KonohaStack *sfp)
//{
//	LLVM_TODO("NO SUPPORT");
//	//IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
//	//Value *const retVals = konoha::object_cast<Value *const>(sfp[1].asObject);
//	//kint_t N = Int_to(kint_t,sfp[2]);
//	//ReturnInst *ptr = self->CreateAggregateRet(retVals, N);
//	//kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	//RETURN_(p);
//}

//## BranchInst IRBuilder.CreateBr(BasicBlock Dest);
static KMETHOD IRBuilder_createBr(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	BasicBlock *Dest = konoha::object_cast<BasicBlock *>(sfp[1].asObject);
	BranchInst *ptr = self->CreateBr(Dest);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## BranchInst IRBuilder.CreateCondBr(Value Cond, BasicBlock True, BasicBlock False);
static KMETHOD IRBuilder_createCondBr(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Cond = konoha::object_cast<Value *>(sfp[1].asObject);
	BasicBlock *True = konoha::object_cast<BasicBlock *>(sfp[2].o);
	BasicBlock *False = konoha::object_cast<BasicBlock *>(sfp[3].o);
	BranchInst *ptr = self->CreateCondBr(Cond, True, False);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## SwitchInst IRBuilder.CreateSwitch(Value V, BasicBlock Dest);
static KMETHOD IRBuilder_createSwitch(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	BasicBlock *Dest = konoha::object_cast<BasicBlock *>(sfp[2].o);
	SwitchInst *ptr = self->CreateSwitch(V, Dest);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## IndirectBrInst IRBuilder.CreateIndirectBr(Value Addr);
static KMETHOD IRBuilder_createIndirectBr(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Addr = konoha::object_cast<Value *>(sfp[1].asObject);
	IndirectBrInst *ptr = self->CreateIndirectBr(Addr);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## InvokeInst IRBuilder.CreateInvoke0(Value Callee, BasicBlock NormalDest, BasicBlock UnwindDest);
static KMETHOD IRBuilder_createInvoke0(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Callee = konoha::object_cast<Value *>(sfp[1].asObject);
	BasicBlock *NormalDest = konoha::object_cast<BasicBlock *>(sfp[2].o);
	BasicBlock *UnwindDest = konoha::object_cast<BasicBlock *>(sfp[3].o);
	InvokeInst *ptr = self->CreateInvoke(Callee, NormalDest, UnwindDest);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## InvokeInst IRBuilder.CreateInvoke1(Value Callee, BasicBlock NormalDest, BasicBlock UnwindDest, Value Arg1);
static KMETHOD IRBuilder_createInvoke1(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Callee = konoha::object_cast<Value *>(sfp[1].asObject);
	BasicBlock *NormalDest = konoha::object_cast<BasicBlock *>(sfp[2].o);
	BasicBlock *UnwindDest = konoha::object_cast<BasicBlock *>(sfp[3].o);
	Value *Arg1 = konoha::object_cast<Value *>(sfp[4].o);
	InvokeInst *ptr = self->CreateInvoke(Callee, NormalDest, UnwindDest, Arg1);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## InvokeInst IRBuilder.CreateInvoke3(Value Callee, BasicBlock NormalDest, BasicBlock UnwindDest, Value Arg1, Value Arg2, Value Arg3);
static KMETHOD IRBuilder_createInvoke3(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Callee = konoha::object_cast<Value *>(sfp[1].asObject);
	BasicBlock *NormalDest = konoha::object_cast<BasicBlock *>(sfp[2].o);
	BasicBlock *UnwindDest = konoha::object_cast<BasicBlock *>(sfp[3].o);
	Value *Arg1 = konoha::object_cast<Value *>(sfp[4].o);
	Value *Arg2 = konoha::object_cast<Value *>(sfp[5].o);
	Value *Arg3 = konoha::object_cast<Value *>(sfp[6].o);
	InvokeInst *ptr = self->CreateInvoke3(Callee, NormalDest, UnwindDest, Arg1, Arg2, Arg3);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

////## InvokeInst IRBuilder.CreateInvoke(Value Callee, BasicBlock NormalDest, BasicBlock UnwindDest, ArrayRef<Value> Args);
//KMETHOD IRBuilder_createInvoke(KonohaContext *kctx, KonohaStack *sfp)
//{
//	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
//	Value *Callee = konoha::object_cast<Value *>(sfp[1].asObject);
//	BasicBlock *NormalDest = konoha::object_cast<BasicBlock *>(sfp[2].o);
//	BasicBlock *UnwindDest = konoha::object_cast<BasicBlock *>(sfp[3].o);
//	kArray *Args = (sfp[4].asArray);
//	std::vector<Value*> List;
//	konoha::convert_array(List, Args);
//	InvokeInst *ptr = self->CreateInvoke(Callee, NormalDest, UnwindDest, List.begin(), List.end());
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	RETURN_(p);
//}

////## ResumeInst IRBuilder.CreateResume(Value Exn);
//KMETHOD IRBuilder_createResume(KonohaContext *kctx, KonohaStack *sfp)
//{
//	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
//	Value *Exn = konoha::object_cast<Value *>(sfp[1].asObject);
//	ResumeInst *ptr = self->CreateResume(Exn);
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	RETURN_(p);
//}
//
//## UnreachableInst IRBuilder.CreateUnreachable();
static KMETHOD IRBuilder_createUnreachable(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	UnreachableInst *ptr = self->CreateUnreachable();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateAdd(Value LHS, Value RHS);
static KMETHOD IRBuilder_createAdd(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateAdd(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateNSWAdd(Value LHS, Value RHS);
static KMETHOD IRBuilder_createNSWAdd(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateNSWAdd(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateNUWAdd(Value LHS, Value RHS);
static KMETHOD IRBuilder_createNUWAdd(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateNUWAdd(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFAdd(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFAdd(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateFAdd(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateSub(Value LHS, Value RHS);
static KMETHOD IRBuilder_createSub(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateSub(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateNSWSub(Value LHS, Value RHS);
static KMETHOD IRBuilder_createNSWSub(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateNSWSub(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateNUWSub(Value LHS, Value RHS);
static KMETHOD IRBuilder_createNUWSub(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateNUWSub(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFSub(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFSub(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateFSub(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateMul(Value LHS, Value RHS);
static KMETHOD IRBuilder_createMul(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateMul(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateNSWMul(Value LHS, Value RHS);
static KMETHOD IRBuilder_createNSWMul(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateNSWMul(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateNUWMul(Value LHS, Value RHS);
static KMETHOD IRBuilder_createNUWMul(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateNUWMul(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFMul(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFMul(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateFMul(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateUDiv(Value LHS, Value RHS);
static KMETHOD IRBuilder_createUDiv(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateUDiv(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateExactUDiv(Value LHS, Value RHS);
static KMETHOD IRBuilder_createExactUDiv(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;(void)(-(K_CALLDELTA));
	LLVM_TODO("NO SUPPORT");
#else
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateExactUDiv(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
#endif
}

//## Value IRBuilder.CreateSDiv(Value LHS, Value RHS);
static KMETHOD IRBuilder_createSDiv(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateSDiv(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateExactSDiv(Value LHS, Value RHS);
static KMETHOD IRBuilder_createExactSDiv(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateExactSDiv(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFDiv(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFDiv(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateFDiv(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateURem(Value LHS, Value RHS);
static KMETHOD IRBuilder_createURem(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateURem(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateSRem(Value LHS, Value RHS);
static KMETHOD IRBuilder_createSRem(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateSRem(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFRem(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFRem(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateFRem(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateShl(Value LHS, Value RHS);
static KMETHOD IRBuilder_createShl(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateShl(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateLShr(Value LHS, Value RHS);
static KMETHOD IRBuilder_createLShr(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateLShr(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateAShr(Value LHS, Value RHS);
static KMETHOD IRBuilder_createAShr(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateAShr(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateAnd(Value LHS, Value RHS);
static KMETHOD IRBuilder_createAnd(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateAnd(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateOr(Value LHS, Value RHS);
static KMETHOD IRBuilder_createOr(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateOr(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateXor(Value LHS, Value RHS);
static KMETHOD IRBuilder_createXor(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateXor(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateNeg(Value V);
static KMETHOD IRBuilder_createNeg(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *ptr = self->CreateNeg(V);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateNSWNeg(Value V);
static KMETHOD IRBuilder_createNSWNeg(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *ptr = self->CreateNSWNeg(V);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateNUWNeg(Value V);
static KMETHOD IRBuilder_createNUWNeg(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *ptr = self->CreateNUWNeg(V);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFNeg(Value V);
static KMETHOD IRBuilder_createFNeg(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *ptr = self->CreateFNeg(V);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateNot(Value V);
static KMETHOD IRBuilder_createNot(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *ptr = self->CreateNot(V);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## AllocaInst IRBuilder.CreateAlloca(Type Ty, Value ArraySize);
static KMETHOD IRBuilder_createAlloca(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Type *Ty = konoha::object_cast<Type *>(sfp[1].asObject);
	Value *ArraySize = konoha::object_cast<Value *>(sfp[2].o);
	AllocaInst *ptr = self->CreateAlloca(Ty, ArraySize);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## LoadInst IRBuilder.CreateLoad(Value Ptr, boolean isVolatile);
static KMETHOD IRBuilder_createLoad(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	kbool_t isVolatile = sfp[2].boolValue;
	LoadInst *ptr = self->CreateLoad(Ptr, isVolatile);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//@Native LoadInst LoadInst.new(Value ptr);
//## LoadInst IRBuilder.CreateLoad(Value Ptr, boolean isVolatile);
static KMETHOD LoadInst_new(KonohaContext *kctx, KonohaStack *sfp)
{
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	LoadInst *ptr = new LoadInst(Ptr);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## StoreInst IRBuilder.CreateStore(Value Val, Value Ptr, boolean isVolatile);
static KMETHOD IRBuilder_createStore(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Val = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[2].o);
	kbool_t isVolatile = sfp[3].boolValue;
	StoreInst *ptr = self->CreateStore(Val, Ptr, isVolatile);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

////## FenceInst IRBuilder.CreateFence(AtomicOrdering Ordering, SynchronizationScope SynchScope);
//KMETHOD IRBuilder_createFence(KonohaContext *kctx, KonohaStack *sfp)
//{
//	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
//	AtomicOrdering *Ordering = konoha::object_cast<AtomicOrdering *>(sfp[1].asObject);
//	SynchronizationScope *SynchScope = konoha::object_cast<SynchronizationScope *>(sfp[2].o);
//	FenceInst *ptr = self->CreateFence(Ordering, SynchScope);
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	RETURN_(p);
//}
//
////## AtomicCmpXchgInst IRBuilder.CreateAtomicCmpXchg(Value Ptr, Value Cmp, Value New, AtomicOrdering Ordering, SynchronizationScope SynchScope);
//KMETHOD IRBuilder_createAtomicCmpXchg(KonohaContext *kctx, KonohaStack *sfp)
//{
//	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
//	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
//	Value *Cmp = konoha::object_cast<Value *>(sfp[2].o);
//	Value *New = konoha::object_cast<Value *>(sfp[3].o);
//	AtomicOrdering *Ordering = konoha::object_cast<AtomicOrdering *>(sfp[4].o);
//	SynchronizationScope *SynchScope = konoha::object_cast<SynchronizationScope *>(sfp[5].o);
//	AtomicCmpXchgInst *ptr = self->CreateAtomicCmpXchg(Ptr, Cmp, New, Ordering, SynchScope);
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	RETURN_(p);
//}

//## @Native AllocaInst AllocaInst.new(Type ty, Value arraySize);
static KMETHOD AllocaInst_new(KonohaContext *kctx, KonohaStack *sfp)
{
	Type *Ty = konoha::object_cast<Type *>(sfp[1].asObject);
	Value *ArraySize = konoha::object_cast<Value *>(sfp[2].o);
	AllocaInst *ptr = new AllocaInst(Ty, ArraySize);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Native StoreInst StoreInst.new(Value val, Value ptr);
static KMETHOD StoreInst_new(KonohaContext *kctx, KonohaStack *sfp)
{
	Value *Val = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[2].o);
	StoreInst *ptr = new StoreInst(Val, Ptr);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
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
	RETURN_(p);
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
	RETURN_(p);
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
	RETURN_(p);
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
	RETURN_(p);
}

//## Value IRBuilder.CreateGEP1(Value Ptr, Value Idx);
static KMETHOD IRBuilder_createGEP1(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *Idx = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateGEP(Ptr, Idx);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateInBoundsGEP1(Value Ptr, Value Idx);
static KMETHOD IRBuilder_createInBoundsGEP1(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *Idx = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateInBoundsGEP(Ptr, Idx);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateConstGEP1_32(Value Ptr, int Idx0);
static KMETHOD IRBuilder_createConstGEP132(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	kint_t Idx0 = Int_to(kint_t,sfp[2]);
	Value *ptr = self->CreateConstGEP1_32(Ptr, Idx0);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateConstInBoundsGEP1_32(Value Ptr, int Idx0);
static KMETHOD IRBuilder_createConstInBoundsGEP132(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	kint_t Idx0 = Int_to(kint_t,sfp[2]);
	Value *ptr = self->CreateConstInBoundsGEP1_32(Ptr, Idx0);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
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
	RETURN_(p);
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
	RETURN_(p);
}

//## Value IRBuilder.CreateConstGEP1_64(Value Ptr, uint64_t Idx0);
static KMETHOD IRBuilder_createConstGEP164(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	kint_t Idx0 = sfp[2].intValue;
	Value *ptr = self->CreateConstGEP1_64(Ptr, Idx0);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateConstInBoundsGEP1_64(Value Ptr, uint64_t Idx0);
static KMETHOD IRBuilder_createConstInBoundsGEP164(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	kint_t Idx0 = sfp[2].intValue;
	Value *ptr = self->CreateConstInBoundsGEP1_64(Ptr, Idx0);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
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
	RETURN_(p);
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
	RETURN_(p);
}

//## Value IRBuilder.CreateStructGEP(Value Ptr, int Idx);
static KMETHOD IRBuilder_createStructGEP(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Ptr = konoha::object_cast<Value *>(sfp[1].asObject);
	kint_t Idx = Int_to(kint_t,sfp[2]);
	Value *ptr = self->CreateStructGEP(Ptr, Idx, "gep");
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateGlobalString(StringRef Str);
static KMETHOD IRBuilder_createGlobalString(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	kString *Str = sfp[1].asString;
	Value *ptr = self->CreateGlobalString(S_text(Str));
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateGlobalStringPtr(StringRef Str);
static KMETHOD IRBuilder_createGlobalStringPtr(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	kString *Str = sfp[1].asString;
	Value *ptr = self->CreateGlobalStringPtr(S_text(Str));
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateTrunc(Value V, Type DestTy);
static KMETHOD IRBuilder_createTrunc(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].o);
	Value *ptr = self->CreateTrunc(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateZExt(Value V, Type DestTy);
static KMETHOD IRBuilder_createZExt(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].o);
	Value *ptr = self->CreateZExt(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateSExt(Value V, Type DestTy);
static KMETHOD IRBuilder_createSExt(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].o);
	Value *ptr = self->CreateSExt(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFPToUI(Value V, Type DestTy);
static KMETHOD IRBuilder_createFPToUI(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].o);
	Value *ptr = self->CreateFPToUI(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFPToSI(Value V, Type DestTy);
static KMETHOD IRBuilder_createFPToSI(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].o);
	Value *ptr = self->CreateFPToSI(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateUIToFP(Value V, Type DestTy);
static KMETHOD IRBuilder_createUIToFP(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].o);
	Value *ptr = self->CreateUIToFP(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateSIToFP(Value V, Type DestTy);
static KMETHOD IRBuilder_createSIToFP(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].o);
	Value *ptr = self->CreateSIToFP(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFPTrunc(Value V, Type DestTy);
static KMETHOD IRBuilder_createFPTrunc(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].o);
	Value *ptr = self->CreateFPTrunc(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFPExt(Value V, Type DestTy);
static KMETHOD IRBuilder_createFPExt(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].o);
	Value *ptr = self->CreateFPExt(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreatePtrToInt(Value V, Type DestTy);
static KMETHOD IRBuilder_createPtrToInt(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].o);
	Value *ptr = self->CreatePtrToInt(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateIntToPtr(Value V, Type DestTy);
static KMETHOD IRBuilder_createIntToPtr(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].o);
	Value *ptr = self->CreateIntToPtr(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateBitCast(Value V, Type DestTy);
static KMETHOD IRBuilder_createBitCast(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].o);
	Value *ptr = self->CreateBitCast(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateZExtOrBitCast(Value V, Type DestTy);
static KMETHOD IRBuilder_createZExtOrBitCast(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].o);
	Value *ptr = self->CreateZExtOrBitCast(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateSExtOrBitCast(Value V, Type DestTy);
static KMETHOD IRBuilder_createSExtOrBitCast(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].o);
	Value *ptr = self->CreateSExtOrBitCast(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateTruncOrBitCast(Value V, Type DestTy);
static KMETHOD IRBuilder_createTruncOrBitCast(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].o);
	Value *ptr = self->CreateTruncOrBitCast(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreatePointerCast(Value V, Type DestTy);
static KMETHOD IRBuilder_createPointerCast(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].o);
	Value *ptr = self->CreatePointerCast(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateIntCast(Value V, Type DestTy, boolean isSigned);
static KMETHOD IRBuilder_createIntCast(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].o);
	kbool_t isSigned = sfp[3].boolValue;
	Value *ptr = self->CreateIntCast(V, DestTy, isSigned);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFPCast(Value V, Type DestTy);
static KMETHOD IRBuilder_createFPCast(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *DestTy = konoha::object_cast<Type *>(sfp[2].o);
	Value *ptr = self->CreateFPCast(V, DestTy);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateICmpEQ(Value LHS, Value RHS);
static KMETHOD IRBuilder_createICmpEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateICmpEQ(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateICmpNE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createICmpNE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateICmpNE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateICmpUGT(Value LHS, Value RHS);
static KMETHOD IRBuilder_createICmpUGT(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateICmpUGT(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateICmpUGE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createICmpUGE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateICmpUGE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateICmpULT(Value LHS, Value RHS);
static KMETHOD IRBuilder_createICmpULT(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateICmpULT(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateICmpULE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createICmpULE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateICmpULE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateICmpSGT(Value LHS, Value RHS);
static KMETHOD IRBuilder_createICmpSGT(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateICmpSGT(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateICmpSGE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createICmpSGE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateICmpSGE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateICmpSLT(Value LHS, Value RHS);
static KMETHOD IRBuilder_createICmpSLT(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateICmpSLT(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateICmpSLE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createICmpSLE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateICmpSLE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFCmpOEQ(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpOEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateFCmpOEQ(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFCmpOGT(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpOGT(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateFCmpOGT(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFCmpOGE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpOGE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateFCmpOGE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFCmpOLT(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpOLT(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateFCmpOLT(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFCmpOLE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpOLE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateFCmpOLE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFCmpONE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpONE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateFCmpONE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFCmpORD(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpORD(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateFCmpORD(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFCmpUNO(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpUNO(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateFCmpUNO(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFCmpUEQ(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpUEQ(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateFCmpUEQ(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFCmpUGT(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpUGT(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateFCmpUGT(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFCmpUGE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpUGE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateFCmpUGE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFCmpULT(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpULT(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateFCmpULT(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFCmpULE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpULE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateFCmpULE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateFCmpUNE(Value LHS, Value RHS);
static KMETHOD IRBuilder_createFCmpUNE(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateFCmpUNE(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
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
	RETURN_(p);
}

//## void IRBuilder.addIncoming(Type Ty, BasicBlock bb);
static KMETHOD PHINode_addIncoming(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	PHINode *self = konoha::object_cast<PHINode *>(sfp[0].asObject);
	Value *v = konoha::object_cast<Value *>(sfp[1].asObject);
	BasicBlock *bb = konoha::object_cast<BasicBlock *>(sfp[2].o);
	self->addIncoming(v, bb);
	RETURNvoid_();
}

//## CallInst IRBuilder.CreateCall1(Value Callee, Value Arg);
static KMETHOD IRBuilder_createCall1(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Callee = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *Arg = konoha::object_cast<Value *>(sfp[2].o);
	CallInst *ptr = self->CreateCall(Callee, Arg);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## CallInst IRBuilder.CreateCall2(Value Callee, Value Arg1, Value Arg2);
static KMETHOD IRBuilder_createCall2(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Callee = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *Arg1 = konoha::object_cast<Value *>(sfp[2].o);
	Value *Arg2 = konoha::object_cast<Value *>(sfp[3].o);
	CallInst *ptr = self->CreateCall2(Callee, Arg1, Arg2);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## CallInst IRBuilder.CreateCall3(Value Callee, Value Arg1, Value Arg2, Value Arg3);
static KMETHOD IRBuilder_createCall3(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Callee = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *Arg1 = konoha::object_cast<Value *>(sfp[2].o);
	Value *Arg2 = konoha::object_cast<Value *>(sfp[3].o);
	Value *Arg3 = konoha::object_cast<Value *>(sfp[4].o);
	CallInst *ptr = self->CreateCall3(Callee, Arg1, Arg2, Arg3);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## CallInst IRBuilder.CreateCall4(Value Callee, Value Arg1, Value Arg2, Value Arg3, Value Arg4);
static KMETHOD IRBuilder_createCall4(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Callee = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *Arg1 = konoha::object_cast<Value *>(sfp[2].o);
	Value *Arg2 = konoha::object_cast<Value *>(sfp[3].o);
	Value *Arg3 = konoha::object_cast<Value *>(sfp[4].o);
	Value *Arg4 = konoha::object_cast<Value *>(sfp[5].o);
	CallInst *ptr = self->CreateCall4(Callee, Arg1, Arg2, Arg3, Arg4);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## CallInst IRBuilder.CreateCall5(Value Callee, Value Arg1, Value Arg2, Value Arg3, Value Arg4, Value Arg5);
static KMETHOD IRBuilder_createCall5(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Callee = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *Arg1 = konoha::object_cast<Value *>(sfp[2].o);
	Value *Arg2 = konoha::object_cast<Value *>(sfp[3].o);
	Value *Arg3 = konoha::object_cast<Value *>(sfp[4].o);
	Value *Arg4 = konoha::object_cast<Value *>(sfp[5].o);
	Value *Arg5 = konoha::object_cast<Value *>(sfp[6].o);
	CallInst *ptr = self->CreateCall5(Callee, Arg1, Arg2, Arg3, Arg4, Arg5);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
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
	RETURN_(p);
}

//## Value IRBuilder.CreateSelect(Value C, Value True, Value False);
static KMETHOD IRBuilder_createSelect(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *C = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *True = konoha::object_cast<Value *>(sfp[2].o);
	Value *False = konoha::object_cast<Value *>(sfp[3].o);
	Value *ptr = self->CreateSelect(C, True, False);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## VAArgInst IRBuilder.CreateVAArg(Value List, Type Ty);
static KMETHOD IRBuilder_createVAArg(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *List = konoha::object_cast<Value *>(sfp[1].asObject);
	Type *Ty = konoha::object_cast<Type *>(sfp[2].o);
	VAArgInst *ptr = self->CreateVAArg(List, Ty);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateExtractElement(Value Vec, Value Idx);
static KMETHOD IRBuilder_createExtractElement(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Vec = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *Idx = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreateExtractElement(Vec, Idx);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateInsertElement(Value Vec, Value NewElt, Value Idx);
static KMETHOD IRBuilder_createInsertElement(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Vec = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *NewElt = konoha::object_cast<Value *>(sfp[2].o);
	Value *Idx = konoha::object_cast<Value *>(sfp[3].o);
	Value *ptr = self->CreateInsertElement(Vec, NewElt, Idx);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateShuffleVector(Value V1, Value V2, Value Mask);
static KMETHOD IRBuilder_createShuffleVector(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *V1 = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *V2 = konoha::object_cast<Value *>(sfp[2].o);
	Value *Mask = konoha::object_cast<Value *>(sfp[3].o);
	Value *ptr = self->CreateShuffleVector(V1, V2, Mask);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
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
//	RETURN_(p);
//}

////## Value IRBuilder.CreateInsertValue(Value Agg, Value Val, Array<int> Idxs);
//KMETHOD IRBuilder_createInsertValue(KonohaContext *kctx, KonohaStack *sfp)
//{
//	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
//	Value *Agg = konoha::object_cast<Value *>(sfp[1].asObject);
//	Value *Val = konoha::object_cast<Value *>(sfp[2].o);
//	kArray *Idxs = sfp[2].asArray;
//	std::vector<int> List;
//	konoha::convert_array_int(List, Idxs);
//	Value *ptr = self->CreateInsertValue(Agg, Val, List.begin(), List.end());
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	RETURN_(p);
//}

//## Value IRBuilder.CreateIsNull(Value Arg);
static KMETHOD IRBuilder_createIsNull(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Arg = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *ptr = self->CreateIsNull(Arg);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreateIsNotNull(Value Arg);
static KMETHOD IRBuilder_createIsNotNull(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *Arg = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *ptr = self->CreateIsNotNull(Arg);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Value IRBuilder.CreatePtrDiff(Value LHS, Value RHS);
static KMETHOD IRBuilder_createPtrDiff(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	Value *LHS = konoha::object_cast<Value *>(sfp[1].asObject);
	Value *RHS = konoha::object_cast<Value *>(sfp[2].o);
	Value *ptr = self->CreatePtrDiff(LHS, RHS);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## void IRBuilder.SetInsertPoint(BasicBlock BB);
static KMETHOD IRBuilder_setInsertPoint(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	BasicBlock * BB = konoha::object_cast<BasicBlock *>(sfp[1].asObject);
	self->SetInsertPoint(BB);
	RETURNvoid_();
}

//## BasicBlock IRBuilder.GetInsertBlock();
static KMETHOD IRBuilder_getInsertBlock(KonohaContext *kctx, KonohaStack *sfp)
{
	IRBuilder<> *self = konoha::object_cast<IRBuilder<> *>(sfp[0].asObject);
	BasicBlock *BB = self->GetInsertBlock();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(BB));
	RETURN_(p);
}

//## Function BasicBlock.getParent();
static KMETHOD BasicBlock_getParent(KonohaContext *kctx, KonohaStack *sfp)
{
	BasicBlock *self = konoha::object_cast<BasicBlock *>(sfp[0].asObject);
	Function *ptr = self->getParent();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Instruction BasicBlock.getTerminator();
static KMETHOD BasicBlock_getTerminator(KonohaContext *kctx, KonohaStack *sfp)
{
	BasicBlock *self = konoha::object_cast<BasicBlock *>(sfp[0].asObject);
	TerminatorInst *ptr = self->getTerminator();
	if (ptr) {
		kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
		RETURN_(p);
	} else {
		RETURN_(K_NULL);
	}
}


////## iterator BasicBlock.begin();
//KMETHOD BasicBlock_begin(KonohaContext *kctx, KonohaStack *sfp)
//{
//	BasicBlock *self = konoha::object_cast<BasicBlock *>(sfp[0].asObject);
//	*ptr = self->Create();
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	RETURN_(K_NULL);
//}
//
////## iterator BasicBlock.end();
//KMETHOD BasicBlock_end(KonohaContext *kctx, KonohaStack *sfp)
//{
//	BasicBlock *self = konoha::object_cast<BasicBlock *>(sfp[0].asObject);
//	*ptr = self->Create();
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	RETURN_(K_NULL);
//}

//## Instruction BasicBlock.getLastInst();
static KMETHOD BasicBlock_getLastInst(KonohaContext *kctx, KonohaStack *sfp)
{
	BasicBlock *self = konoha::object_cast<BasicBlock *>(sfp[0].asObject);
	BasicBlock::iterator I = self->end();
	Instruction *ptr;
	if (self->size() > 0)
		--I;
	ptr = I;
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Instruction BasicBlock.insertBefore(Instruction before, Instruction inst);
static KMETHOD BasicBlock_insertBefore(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	BasicBlock *self = konoha::object_cast<BasicBlock *>(sfp[0].asObject);
	Instruction *inst0 = konoha::object_cast<Instruction *>(sfp[1].asObject);
	Instruction *inst1 = konoha::object_cast<Instruction *>(sfp[2].o);
	self->getInstList().insert(inst0, inst1);
	RETURNvoid_();
}

//## int BasicBlock.size();
static KMETHOD BasicBlock_size(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	BasicBlock *self = konoha::object_cast<BasicBlock *>(sfp[0].asObject);
	int ret = self->size();
	RETURNi_(ret);
}

//## boolean BasicBlock.empty();
static KMETHOD BasicBlock_empty(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	BasicBlock *self = konoha::object_cast<BasicBlock *>(sfp[0].asObject);
	bool isEmpty = self->empty();
	RETURNb_(isEmpty);
}

//## Argument Argument.new(Type ty, int scid);
static KMETHOD Argument_new(KonohaContext *kctx, KonohaStack *sfp)
{
	Type *ty = konoha::object_cast<Type *>(sfp[1].asObject);
	Value *v = new Argument(ty, "", 0);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(v));
	RETURN_(p);
}

//static void str_replace (std::string& str, const std::string& from, const std::string& to) {
//	std::string::size_type pos = 0;
//	while ((pos = str.find(from, pos)) != std::string::npos) {
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
	Module *M = new Module(S_text(name), Context);
#if 0
	Triple T(sys::getDefaultTargetTriple());
	const Target *Target = 0;
	std::string Arch = T.getArchName();
	for (TargetRegistry::iterator it = TargetRegistry::begin(),
			ie = TargetRegistry::end(); it != ie; ++it) {
		std::string tmp(it->getName());
		str_replace(tmp, "-", "_");
		if (Arch == tmp) {
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
	RETURN_(p);
}

//## void Module.dump();
static KMETHOD Module_dump(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	Module *self = konoha::object_cast<Module *>(sfp[0].asObject);
	(*self).dump();
	RETURNvoid_();
}

//## Type Module.getTypeByName(String name);
static KMETHOD Module_getTypeByName(KonohaContext *kctx, KonohaStack *sfp)
{
	Module *self = konoha::object_cast<Module *>(sfp[0].asObject);
	kString *name = sfp[1].asString;
	Type *ptr = CONST_CAST(Type*, self->getTypeByName(S_text(name)));
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## void BasicBlock.dump();
static KMETHOD BasicBlock_dump(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	BasicBlock *self = konoha::object_cast<BasicBlock *>(sfp[0].asObject);
	(*self).dump();
	RETURNvoid_();
}

//## Function Module.getOrInsertFunction(String name, FunctionType fnTy);
static KMETHOD Module_getOrInsertFunction(KonohaContext *kctx, KonohaStack *sfp)
{
	Module *self = konoha::object_cast<Module *>(sfp[0].asObject);
	kString *name = sfp[1].asString;
	FunctionType *fnTy = konoha::object_cast<FunctionType *>(sfp[2].o);
	Function *ptr = cast<Function>(self->getOrInsertFunction(S_text(name), fnTy));
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static @Native Function Function.create(String name, FunctionType fnTy, Module m, Linkage linkage);
static KMETHOD Function_create(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *name = sfp[1].asString;
	FunctionType *fnTy = konoha::object_cast<FunctionType *>(sfp[2].o);
	Module *m = konoha::object_cast<Module *>(sfp[3].o);
	kint_t v = sfp[4].intValue;
	GlobalValue::LinkageTypes linkage = (GlobalValue::LinkageTypes) v;
	Function *ptr = Function::Create(fnTy, linkage, S_text(name), m);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}
//## @Static @Native Type Function.getReturnType();
static KMETHOD Function_getReturnType(KonohaContext *kctx, KonohaStack *sfp)
{
	Function *F = konoha::object_cast<Function *>(sfp[0].asObject);
	LLVMTYPE *ptr = F->getReturnType();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Native void Function.dump();
static KMETHOD Function_dump(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	Function *func = konoha::object_cast<Function *>(sfp[0].asObject);
	func->dump();
	RETURNvoid_();
}

//## @Native void Function.addFnAttr(Int attributes);
static KMETHOD Function_addFnAttr(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	Function *F = konoha::object_cast<Function *>(sfp[0].asObject);
	Attributes N = (Attributes) sfp[1].intValue;
	F->addFnAttr(N);
	RETURNvoid_();
}

//## ExecutionEngine Module.createExecutionEngine(int optLevel);
static KMETHOD Module_createExecutionEngine(KonohaContext *kctx, KonohaStack *sfp)
{
	Module *self = konoha::object_cast<Module *>(sfp[0].asObject);
	CodeGenOpt::Level OptLevel = (CodeGenOpt::Level) sfp[1].intValue;
	ExecutionEngine *ptr = EngineBuilder(self).setEngineKind(EngineKind::JIT).setOptLevel(OptLevel).create();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

static int BasicBlock_compareTo(kObject *p1, kObject *p2)
{
	BasicBlock *bb1 = konoha::object_cast<BasicBlock*>(p1);
	BasicBlock *bb2 = konoha::object_cast<BasicBlock*>(p2);
	return (bb1 != bb2);
}

//void defBasicBlock(KonohaContext *kctx _UNUSED_, ktype_t cid _UNUSED_, kclassdef_t *cdef)
//{
//	cdef->name = "llvm::BasicBlock";
//	cdef->compareTo = BasicBlock_compareTo;
//}

//## @Static BasicBlock BasicBlock.create(Function parent, String name);
static KMETHOD BasicBlock_create(KonohaContext *kctx, KonohaStack *sfp)
{
	Function * parent = konoha::object_cast<Function *>(sfp[1].asObject);
	kString *name = sfp[2].s;
	const char *bbname = "";
	if (IS_NOTNULL(name)) {
		bbname = S_text(name);
	}
	BasicBlock *ptr = BasicBlock::Create(getGlobalContext(), bbname, parent);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
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
	RETURN_(p);
}

//## @Native Value ConstantInt.get(Type type, int v);
static KMETHOD ConstantInt_get(KonohaContext *kctx, KonohaStack *sfp)
{
	Type *type  = konoha::object_cast<Type *>(sfp[1].asObject);
	kint_t v = sfp[2].intValue;
	Value *ptr = ConstantInt::get(type, v);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Native Value ConstantFP.get(Type type, float v);
static KMETHOD ConstantFP_get(KonohaContext *kctx, KonohaStack *sfp)
{
	Type *type  = konoha::object_cast<Type *>(sfp[1].asObject);
	kfloat_t v = sfp[2].floatValue;
	Value *ptr = ConstantFP::get(type, v);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Static @Native Value ConstantPointerNull.get(Type type);
static KMETHOD ConstantPointerNull_get(KonohaContext *kctx, KonohaStack *sfp)
{
	PointerType *type  = konoha::object_cast<PointerType *>(sfp[1].asObject);
	Value *ptr = ConstantPointerNull::get(type);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
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
	RETURN_(p);
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
	RETURN_(p);
}

//## @Static @Native StructType.create(Array<Type> args, String name, boolean isPacked);
static KMETHOD StructType_create(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *args = sfp[1].asArray;
#if LLVM_VERSION > 209
	kString *name = sfp[2].s;
#endif
	kbool_t isPacked = sfp[3].boolValue;
	StructType *ptr;
	if (IS_NULL(args)) {
#if LLVM_VERSION <= 209
		ptr = StructType::get(getGlobalContext());
#else
		ptr = StructType::create(getGlobalContext(), S_text(name));
#endif
	} else if (kArray_size(args) == 0) {
#if LLVM_VERSION <= 209
		std::vector<const Type*> List;
		ptr = StructType::get(getGlobalContext(), List, isPacked);
#else
		std::vector<Type*> List;
		ptr = StructType::create(getGlobalContext(), S_text(name));
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
		ptr = StructType::create(List, S_text(name), isPacked);
#endif
	}
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Native @Static ArrayType ArrayType.get(Type t, int elemSize);
static KMETHOD ArrayType_get(KonohaContext *kctx, KonohaStack *sfp)
{
	Type *Ty = konoha::object_cast<Type *>(sfp[1].asObject);
	kint_t N = sfp[2].boolValue;
	ArrayType *ptr = ArrayType::get(Ty, N);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## @Native void StructType.setBody(Array<Type> args, boolean isPacked);
static KMETHOD StructType_setBody(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
#if LLVM_VERSION <= 209
	(void)kctx;(void)sfp;(void)(-(K_CALLDELTA));
	LLVM_TODO("NO SUPPORT");
#else
	StructType *type  = konoha::object_cast<StructType *>(sfp[0].asObject);
	kArray *args = sfp[1].asArray;
	kbool_t isPacked = sfp[2].boolValue;
	std::vector<Type*> List;
	konoha::convert_array(List, args);
	type->setBody(List, isPacked);
	RETURNvoid_();
#endif
}

//## @Native boolean StructType.isOpaque();
static KMETHOD StructType_isOpaque(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	bool ret = false;
#if LLVM_VERSION <= 209
	LLVM_TODO("NO SUPPORT");
#else
	StructType *type  = konoha::object_cast<StructType *>(sfp[0].asObject);
	ret = type->isOpaque();
#endif
	RETURNb_(ret);
}

//## NativeFunction ExecutionEngine.getPointerToFunction(Function func);
static KMETHOD ExecutionEngine_getPointerToFunction(KonohaContext *kctx, KonohaStack *sfp)
{
	ExecutionEngine *ee = konoha::object_cast<ExecutionEngine *>(sfp[0].asObject);
	Function *func = konoha::object_cast<Function *>(sfp[1].asObject);
	void *ptr = ee->getPointerToFunction(func);
	//kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURNi_((uintptr_t)ptr);
}
//## @Native void ExecutionEngine.addGlobalMapping(GlobalVariable g, int addr);
static KMETHOD ExecutionEngine_addGlobalMapping(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	ExecutionEngine *ee = konoha::object_cast<ExecutionEngine *>(sfp[0].asObject);
	GlobalVariable *g   = konoha::object_cast<GlobalVariable *>(sfp[1].asObject);
	long addr = sfp[2].intValue;
	ee->addGlobalMapping(g, (void*)addr);
	RETURNvoid_();
}
//## @Native GlobalVariable GlobalVariable.new(Module m, Type ty, Constant c, Linkage linkage, String name);
static KMETHOD GlobalVariable_new(KonohaContext *kctx, KonohaStack *sfp)
{
	Module *m     = konoha::object_cast<Module *>(sfp[1].asObject);
	Type *ty      = konoha::object_cast<Type *>(sfp[2].o);
	Constant *c   = konoha::object_cast<Constant *>(sfp[3].o);
	GlobalValue::LinkageTypes linkage = (GlobalValue::LinkageTypes) sfp[4].intValue;
	kString *name = sfp[5].s;
	bool isConstant = (c) ? true : false;
	GlobalVariable *ptr = new GlobalVariable(*m, ty, isConstant, linkage, c, S_text(name));
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
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
	RETURN_(p);
}

static KMETHOD PassManagerBuilder_populateModulePassManager(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	PassManagerBuilder *self = konoha::object_cast<PassManagerBuilder *>(sfp[0].asObject);
	PassManager *manager = konoha::object_cast<PassManager *>(sfp[1].asObject);
	self->populateModulePassManager(*manager);
	RETURNvoid_();
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
	RETURN_(p);
}

//## void PassManager.run(Function func)
static KMETHOD PassManager_run(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	PassManager *self = konoha::object_cast<PassManager *>(sfp[0].asObject);
	Module *m = konoha::object_cast<Module *>(sfp[1].asObject);
	self->run(*m);
	RETURNvoid_();
}
//## void PassManager.add(Pass p)
static KMETHOD PassManager_addPass(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	PassManager *self = konoha::object_cast<PassManager *>(sfp[0].asObject);
	Pass *pass = konoha::object_cast<Pass *>(sfp[1].asObject);
	self->add(pass);
	RETURNvoid_();
}
//## void PassManager.add(Pass p)
static KMETHOD PassManager_addImmutablePass(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	PassManager *self = konoha::object_cast<PassManager *>(sfp[0].asObject);
	ImmutablePass *pass = konoha::object_cast<ImmutablePass *>(sfp[1].asObject);
	self->add(pass);
	RETURNvoid_();
}
//## void PassManager.addFunctionPass(Pass p)
static KMETHOD PassManager_addFunctionPass(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	PassManager *self = konoha::object_cast<PassManager *>(sfp[0].asObject);
	FunctionPass *pass = konoha::object_cast<FunctionPass *>(sfp[1].asObject);
	self->add(pass);
	RETURNvoid_();
}
//## void PassManager.addModulePass(Pass p)
static KMETHOD PassManager_addModulePass(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	PassManager *self = konoha::object_cast<PassManager *>(sfp[0].asObject);
	ModulePass *pass = konoha::object_cast<ModulePass *>(sfp[1].asObject);
	self->add(pass);
	RETURNvoid_();
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
	RETURN_(p);
}
//## void FuncitonPassManager.add(Pass p)
static KMETHOD FunctionPassManager_add(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	FunctionPassManager *self = konoha::object_cast<FunctionPassManager *>(sfp[0].asObject);
	Pass *pass = konoha::object_cast<Pass *>(sfp[1].asObject);
	self->add(pass);
	RETURNvoid_();
}
//## void FunctionPassManager.doInitialization()
static KMETHOD FunctionPassManager_doInitialization(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	FunctionPassManager *self = konoha::object_cast<FunctionPassManager *>(sfp[0].asObject);
	self->doInitialization();
	RETURNvoid_();
}

//## void FunctionPassManager.run(Function func)
static KMETHOD FunctionPassManager_run(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	FunctionPassManager *self = konoha::object_cast<FunctionPassManager *>(sfp[0].asObject);
	Function *func = konoha::object_cast<Function *>(sfp[1].asObject);
	self->run(*func);
	RETURNvoid_();
}

//## TargetData ExecutionEngine.getTargetData();
static KMETHOD ExecutionEngine_getTargetData(KonohaContext *kctx, KonohaStack *sfp)
{
	ExecutionEngine *ee = konoha::object_cast<ExecutionEngine *>(sfp[0].asObject);
	TargetData *ptr = new TargetData(*(ee->getTargetData()));
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## void Method.setFunction(NativeFunction func);
static KMETHOD kMethod_setFunction(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = (kMethod*) sfp[0].asObject;
	kObject *po = sfp[1].asObject;
	union anyptr { void *p; MethodFunc f;} ptr;
	ptr.p = konoha::object_cast<void*>(po);
	KLIB kMethod_setFunc(kctx, mtd, ptr.f);
	RETURNvoid_();
}

//## @Native Array<Value> Function.getArguments();
static KMETHOD Function_getArguments(KonohaContext *kctx, KonohaStack *sfp)
{
	Function *func = konoha::object_cast<Function *>(sfp[0].asObject);
	ktype_t cid = TY_Value;
	/*FIXME Generics Array */
	//ktype_t rtype = sfp[K_MTDIDX].mtdNC->pa->rtype;
	//ktype_t cid = CT_(rtype)->p1;
	kArray *a = new_(Array, 0);
	for (Function::arg_iterator I = func->arg_begin(), E = func->arg_end();
			I != E; ++I) {
		Value *v = I;
		kObject *o = new_CppObject(kctx, CT_(cid)/*"Value"*/, WRAP(v));
		KLIB kArray_add(kctx, a, o);
	}
	RETURN_(a);
}
//## void Value.replaceAllUsesWith(Value v);
static KMETHOD Value_replaceAllUsesWith(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	Value *self = konoha::object_cast<Value *>(sfp[0].asObject);
	Value *v = konoha::object_cast<Value *>(sfp[1].asObject);
	self->replaceAllUsesWith(v);
	RETURNvoid_();
}
//## Value Value.setName(String name);
static KMETHOD Value_setName(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	Value *self = konoha::object_cast<Value *>(sfp[0].asObject);
	kString *name = sfp[1].asString;
	self->setName(S_text(name));
	RETURNvoid_();
}
//## void LoadInst.setAlignment(int align);
static KMETHOD LoadInst_setAlignment(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	LoadInst *self = konoha::object_cast<LoadInst *>(sfp[0].asObject);
	int align = sfp[1].intValue;
	self->setAlignment(align);
	RETURNvoid_();
}
//## void StoreInst.setAlignment(int align);
static KMETHOD StoreInst_setAlignment(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	StoreInst *self = konoha::object_cast<StoreInst *>(sfp[0].asObject);
	int align = sfp[1].intValue;
	self->setAlignment(align);
	RETURNvoid_();
}
//## Type Value.getType();
static KMETHOD Value_getType(KonohaContext *kctx, KonohaStack *sfp)
{
	Value *self = konoha::object_cast<Value *>(sfp[0].asObject);
	const Type *ptr = self->getType();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## void Value.dump();
static KMETHOD Value_dump(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	Value *self = konoha::object_cast<Value *>(sfp[0].asObject);
	self->dump();
	RETURNvoid_();
}

//## @Native void Type.dump();
static KMETHOD Type_dump(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	Type *type = konoha::object_cast<Type *>(sfp[0].asObject);
	type->dump();
	RETURNvoid_();
}

//## @Static boolean DynamicLibrary.loadLibraryPermanently(String libname);
static KMETHOD DynamicLibrary_loadLibraryPermanently(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *libname = S_text(sfp[1].asString);
	std::string ErrMsg;
	kbool_t ret;
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;(void)(-(K_CALLDELTA));
	LLVM_TODO("NO SUPPORT");
	ret = LinkDynamicObject(libname, &ErrMsg);
#else
	ret = sys::DynamicLibrary::LoadLibraryPermanently(libname, &ErrMsg);
#endif
	if (ret == 0) {
		//TODO
		//KNH_NTRACE2(kctx, "LoadLibraryPermanently", K_FAILED, KNH_LDATA(LOG_s("libname", libname), LOG_msg(ErrMsg.c_str())));
	}
	RETURNb_(ret);
}

//## @Static Int DynamicLibrary.searchForAddressOfSymbol(String fname);
static KMETHOD DynamicLibrary_searchForAddressOfSymbol(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
	const char *fname = S_text(sfp[1].asString);
	kint_t ret = 0;
	void *symAddr = NULL;
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;(void)(-(K_CALLDELTA));(void)fname;
	LLVM_TODO("NO SUPPORT");
	//symAddr = GetAddressOfSymbol(fname);
#else
	symAddr = sys::DynamicLibrary::SearchForAddressOfSymbol(fname);
#endif
	if (symAddr) {
		ret = reinterpret_cast<kint_t>(symAddr);
	}
	RETURNi_(ret);
}

//## FunctionPass LLVM.createDomPrinterPass();
static KMETHOD LLVM_createDomPrinterPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createDomPrinterPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createDomOnlyPrinterPass();
static KMETHOD LLVM_createDomOnlyPrinterPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createDomOnlyPrinterPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createDomViewerPass();
static KMETHOD LLVM_createDomViewerPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createDomViewerPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createDomOnlyViewerPass();
static KMETHOD LLVM_createDomOnlyViewerPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createDomOnlyViewerPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createPostDomPrinterPass();
static KMETHOD LLVM_createPostDomPrinterPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createPostDomPrinterPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createPostDomOnlyPrinterPass();
static KMETHOD LLVM_createPostDomOnlyPrinterPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createPostDomOnlyPrinterPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createPostDomViewerPass();
static KMETHOD LLVM_createPostDomViewerPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createPostDomViewerPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createPostDomOnlyViewerPass();
static KMETHOD LLVM_createPostDomOnlyViewerPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createPostDomOnlyViewerPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createGlobalsModRefPass();
static KMETHOD LLVM_createGlobalsModRefPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createGlobalsModRefPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createAliasDebugger();
static KMETHOD LLVM_createAliasDebugger(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createAliasDebugger();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## ModulePass LLVM.createAliasAnalysisCounterPass();
static KMETHOD LLVM_createAliasAnalysisCounterPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createAliasAnalysisCounterPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createAAEvalPass();
static KMETHOD LLVM_createAAEvalPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createAAEvalPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createLibCallAliasAnalysisPass(LibCallInfo lci);
static KMETHOD LLVM_createLibCallAliasAnalysisPass(KonohaContext *kctx, KonohaStack *sfp)
{
	LibCallInfo *lci = konoha::object_cast<LibCallInfo *>(sfp[0].asObject);
	FunctionPass *ptr = createLibCallAliasAnalysisPass(lci);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createScalarEvolutionAliasAnalysisPass();
static KMETHOD LLVM_createScalarEvolutionAliasAnalysisPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createScalarEvolutionAliasAnalysisPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## ModulePass LLVM.createProfileLoaderPass();
static KMETHOD LLVM_createProfileLoaderPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createProfileLoaderPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createProfileEstimatorPass();
static KMETHOD LLVM_createProfileEstimatorPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createProfileEstimatorPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createProfileVerifierPass();
static KMETHOD LLVM_createProfileVerifierPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createProfileVerifierPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## ModulePass LLVM.createPathProfileLoaderPass();
static KMETHOD LLVM_createPathProfileLoaderPass(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;(void)(-(K_CALLDELTA));
	LLVM_TODO("NO SUPPORT");
#else
	ModulePass *ptr = createPathProfileLoaderPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
#endif
}

//## ModulePass LLVM.createPathProfileVerifierPass();
static KMETHOD LLVM_createPathProfileVerifierPass(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;(void)(-(K_CALLDELTA));
	LLVM_TODO("NO SUPPORT");
#else
	ModulePass *ptr = createPathProfileVerifierPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
#endif
}

//## FunctionPass LLVM.createLazyValueInfoPass();
static KMETHOD LLVM_createLazyValueInfoPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createLazyValueInfoPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## LoopPass LLVM.createLoopDependenceAnalysisPass();
static KMETHOD LLVM_createLoopDependenceAnalysisPass(KonohaContext *kctx, KonohaStack *sfp)
{
	LoopPass *ptr = createLoopDependenceAnalysisPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createInstCountPass();
static KMETHOD LLVM_createInstCountPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createInstCountPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createDbgInfoPrinterPass();
static KMETHOD LLVM_createDbgInfoPrinterPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createDbgInfoPrinterPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createRegionInfoPass();
static KMETHOD LLVM_createRegionInfoPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createRegionInfoPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}
//## Constant* ConstantExpr::getAlignOf(Type* ty);
static KMETHOD ConstantExpr_getAlignOf(KonohaContext *kctx, KonohaStack *sfp)
{
	Type* ty = konoha::object_cast<Type*>(sfp[1].asObject);
	Constant* ptr = ConstantExpr::getAlignOf(ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getSizeOf(Type* ty);
static KMETHOD ConstantExpr_getSizeOf(KonohaContext *kctx, KonohaStack *sfp)
{
	Type* ty = konoha::object_cast<Type*>(sfp[1].asObject);
	Constant* ptr = ConstantExpr::getSizeOf(ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getOffsetOf(StructType* sTy, unsigned fieldNo);
static KMETHOD ConstantExpr_getOffsetOf(KonohaContext *kctx, KonohaStack *sfp)
{
	StructType* sTy = konoha::object_cast<StructType*>(sfp[1].asObject);
	unsigned fieldNo = (sfp[2].intValue);
	Constant* ptr = ConstantExpr::getOffsetOf(sTy, fieldNo);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

////## Constant* ConstantExpr::getOffsetOf(Type* ty, Constant* fieldNo);
//static KMETHOD ConstantExpr_getOffsetOf(KonohaContext *kctx, KonohaStack *sfp)
//{
//	Type* ty = konoha::object_cast<Type*>(sfp[1].asObject);
//	Constant* fieldNo = konoha::object_cast<Constant*>(sfp[2].o);
//	Constant* ptr = ConstantExpr::getOffsetOf(ty, fieldNo);
//	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	RETURN_(p);
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
	RETURN_(p);
}

//## Constant* ConstantExpr::getFNeg(Constant* c);
static KMETHOD ConstantExpr_getFNeg(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* ptr = ConstantExpr::getFNeg(c);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getNot(Constant* c);
static KMETHOD ConstantExpr_getNot(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* ptr = ConstantExpr::getNot(c);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getAdd(Constant* c1, Constant* c2, bool hasNUW, bool hasNSW);
static KMETHOD ConstantExpr_getAdd(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr;
#if LLVM_VERSION <= 209
	ptr = ConstantExpr::getAdd(c1, c2);
#else
	bool hasNUW = sfp[3].boolValue;
	bool hasNSW = sfp[4].boolValue;
	ptr = ConstantExpr::getAdd(c1, c2, hasNUW, hasNSW);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getFAdd(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getFAdd(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getFAdd(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getSub(Constant* c1, Constant* c2, bool hasNUW, bool hasNSW);
static KMETHOD ConstantExpr_getSub(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr;
#if LLVM_VERSION <= 209
	ptr = ConstantExpr::getSub(c1, c2);
#else
	bool hasNUW = sfp[3].boolValue;
	bool hasNSW = sfp[4].boolValue;
	ptr = ConstantExpr::getSub(c1, c2, hasNUW, hasNSW);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getFSub(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getFSub(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getFSub(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getMul(Constant* c1, Constant* c2, bool hasNUW, bool hasNSW);
static KMETHOD ConstantExpr_getMul(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr;
#if LLVM_VERSION <= 209
	ptr = ConstantExpr::getMul(c1, c2);
#else
	bool hasNUW = sfp[3].boolValue;
	bool hasNSW = sfp[4].boolValue;
	ptr = ConstantExpr::getMul(c1, c2, hasNUW, hasNSW);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getFMul(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getFMul(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getFMul(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getUDiv(Constant* c1, Constant* c2, bool isExact);
static KMETHOD ConstantExpr_getUDiv(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr;
#if LLVM_VERSION <= 209
	ptr = ConstantExpr::getUDiv(c1, c2);
#else
	bool isExact = sfp[3].boolValue;
	ptr = ConstantExpr::getUDiv(c1, c2, isExact);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getSDiv(Constant* c1, Constant* c2, bool isExact);
static KMETHOD ConstantExpr_getSDiv(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr;
#if LLVM_VERSION <= 209
	ptr = ConstantExpr::getSDiv(c1, c2);
#else
	bool isExact = sfp[3].boolValue;
	ptr = ConstantExpr::getSDiv(c1, c2, isExact);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getFDiv(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getFDiv(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getFDiv(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getURem(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getURem(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getURem(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getSRem(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getSRem(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getSRem(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getFRem(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getFRem(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getFRem(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getAnd(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getAnd(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getAnd(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getOr(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getOr(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getOr(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getXor(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getXor(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getXor(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getShl(Constant* c1, Constant* c2, bool hasNUW, bool hasNSW);
static KMETHOD ConstantExpr_getShl(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr;
#if LLVM_VERSION <= 209
	ptr = ConstantExpr::getShl(c1, c2);
#else
	bool hasNUW = sfp[3].boolValue;
	bool hasNSW = sfp[4].boolValue;
	ptr = ConstantExpr::getShl(c1, c2, hasNUW, hasNSW);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getLShr(Constant* c1, Constant* c2, bool isExact);
static KMETHOD ConstantExpr_getLShr(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr;
#if LLVM_VERSION <= 209
	ptr = ConstantExpr::getLShr(c1, c2);
#else
	bool isExact = sfp[3].boolValue;
	ptr = ConstantExpr::getLShr(c1, c2, isExact);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getAShr(Constant* c1, Constant* c2, bool isExact);
static KMETHOD ConstantExpr_getAShr(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr;
#if LLVM_VERSION <= 209
	ptr = ConstantExpr::getAShr(c1, c2);
#else
	bool isExact = sfp[3].boolValue;
	ptr = ConstantExpr::getAShr(c1, c2, isExact);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getTrunc(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getTrunc(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getTrunc(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getSExt(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getSExt(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getSExt(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getZExt(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getZExt(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getZExt(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getFPTrunc(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getFPTrunc(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getFPTrunc(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getFPExtend(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getFPExtend(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getFPExtend(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getUIToFP(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getUIToFP(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getUIToFP(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getSIToFP(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getSIToFP(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getSIToFP(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getFPToUI(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getFPToUI(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getFPToUI(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getFPToSI(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getFPToSI(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getFPToSI(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getPtrToInt(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getPtrToInt(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getPtrToInt(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getIntToPtr(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getIntToPtr(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getIntToPtr(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getBitCast(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getBitCast(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getBitCast(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getNSWNeg(Constant* c);
static KMETHOD ConstantExpr_getNSWNeg(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* ptr = ConstantExpr::getNSWNeg(c);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getNUWNeg(Constant* c);
static KMETHOD ConstantExpr_getNUWNeg(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* ptr = ConstantExpr::getNUWNeg(c);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getNSWAdd(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getNSWAdd(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getNSWAdd(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getNUWAdd(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getNUWAdd(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getNUWAdd(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getNSWSub(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getNSWSub(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getNSWSub(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getNUWSub(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getNUWSub(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getNUWSub(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getNSWMul(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getNSWMul(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getNSWMul(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getNUWMul(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getNUWMul(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getNUWMul(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getNSWShl(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getNSWShl(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;(void)(-(K_CALLDELTA));
	LLVM_TODO("NO SUPPORT");
#else
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getNSWShl(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
#endif
}

//## Constant* ConstantExpr::getNUWShl(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getNUWShl(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;(void)(-(K_CALLDELTA));
	LLVM_TODO("NO SUPPORT");
#else
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getNUWShl(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
#endif
}

//## Constant* ConstantExpr::getExactSDiv(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getExactSDiv(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getExactSDiv(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getExactUDiv(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getExactUDiv(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;(void)(-(K_CALLDELTA));
	LLVM_TODO("NO SUPPORT");
#else
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getExactUDiv(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
#endif
}

//## Constant* ConstantExpr::getExactAShr(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getExactAShr(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;(void)(-(K_CALLDELTA));
	LLVM_TODO("NO SUPPORT");
#else
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getExactAShr(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
#endif
}

//## Constant* ConstantExpr::getExactLShr(Constant* c1, Constant* c2);
static KMETHOD ConstantExpr_getExactLShr(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;(void)(-(K_CALLDELTA));
	LLVM_TODO("NO SUPPORT");
#else
	Constant* c1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* c2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getExactLShr(c1, c2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
#endif
}

//## Constant* ConstantExpr::getZExtOrBitCast(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getZExtOrBitCast(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getZExtOrBitCast(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getSExtOrBitCast(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getSExtOrBitCast(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getSExtOrBitCast(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getTruncOrBitCast(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getTruncOrBitCast(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getTruncOrBitCast(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getPointerCast(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getPointerCast(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getPointerCast(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getIntegerCast(Constant* c, Type* ty, bool isSigned);
static KMETHOD ConstantExpr_getIntegerCast(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].o);
	bool isSigned = sfp[3].boolValue;
	Constant* ptr = ConstantExpr::getIntegerCast(c, ty, isSigned);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getFPCast(Constant* c, Type* ty);
static KMETHOD ConstantExpr_getFPCast(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Type* ty = konoha::object_cast<Type*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getFPCast(c, ty);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getSelect(Constant* c, Constant* v1, Constant* v2);
static KMETHOD ConstantExpr_getSelect(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* v1 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* v2 = konoha::object_cast<Constant*>(sfp[3].o);
	Constant* ptr = ConstantExpr::getSelect(c, v1, v2);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
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
//	RETURN_(p);
//}

//## Constant* ConstantExpr::getElementPtr(Constant* c, Constant* idx, bool InBounds);
static KMETHOD ConstantExpr_getElementPtr0(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* idx = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr;
#if LLVM_VERSION <= 209
	Constant *IdxList[] = {idx};
	ptr = ConstantExpr::getGetElementPtr(c, IdxList, 0);
#else
	bool InBounds = sfp[3].boolValue;
	ptr = ConstantExpr::getGetElementPtr(c, idx, InBounds);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
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
	RETURN_(p);
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
//	RETURN_(p);
//}

//## Constant* ConstantExpr::getInBoundsGetElementPtr(Constant* c, Constant* idx);
static KMETHOD ConstantExpr_getInBoundsGetElementPtr0(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* ptr;
	Constant* c = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* idx = konoha::object_cast<Constant*>(sfp[2].o);
#if LLVM_VERSION <= 209
	Constant *IdxList[] = {idx};
	ptr = ConstantExpr::getInBoundsGetElementPtr(c, IdxList, 0);
#else
	ptr = ConstantExpr::getInBoundsGetElementPtr(c, idx);
#endif
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
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
	RETURN_(p);
}

//## Constant* ConstantExpr::getExtractElement(Constant* vec, Constant* idx);
static KMETHOD ConstantExpr_getExtractElement(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* vec = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* idx = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* ptr = ConstantExpr::getExtractElement(vec, idx);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getInsertElement(Constant* vec, Constant* elt,Constant* idx);
static KMETHOD ConstantExpr_getInsertElement(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* vec = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* elt = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* idx = konoha::object_cast<Constant*>(sfp[3].o);
	Constant* ptr = ConstantExpr::getInsertElement(vec, elt, idx);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Constant* ConstantExpr::getShuffleVector(Constant* v1, Constant* v2, Constant* mask);
static KMETHOD ConstantExpr_getShuffleVector(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* v1 = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* v2 = konoha::object_cast<Constant*>(sfp[2].o);
	Constant* mask = konoha::object_cast<Constant*>(sfp[3].o);
	Constant* ptr = ConstantExpr::getShuffleVector(v1, v2, mask);
	kObject* p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
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
	RETURN_(p);
}

//## Constant* ConstantExpr::getInsertValue(Constant* Agg, Constant* val, ArrayRef<unsigned> idxs);
static KMETHOD ConstantExpr_getInsertValue(KonohaContext *kctx, KonohaStack *sfp)
{
	Constant* Agg = konoha::object_cast<Constant*>(sfp[1].asObject);
	Constant* val = konoha::object_cast<Constant*>(sfp[2].o);
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
	RETURN_(p);
}

//## ModulePass LLVM.createModuleDebugInfoPrinterPass();
static KMETHOD LLVM_createModuleDebugInfoPrinterPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createModuleDebugInfoPrinterPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createMemDepPrinter();
static KMETHOD LLVM_createMemDepPrinter(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;(void)(-(K_CALLDELTA));
	LLVM_TODO("NO SUPPORT");
#else
	FunctionPass *ptr = createMemDepPrinter();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
#endif
}

//## FunctionPass LLVM.createPostDomTree();
static KMETHOD LLVM_createPostDomTree(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;(void)(-(K_CALLDELTA));
	LLVM_TODO("NO SUPPORT");
#else
	FunctionPass *ptr = createPostDomTree();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
#endif
}

//## FunctionPass LLVM.createRegionViewerPass();
static KMETHOD LLVM_createRegionViewerPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createRegionViewerPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createRegionOnlyViewerPass();
static KMETHOD LLVM_createRegionOnlyViewerPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createRegionOnlyViewerPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createRegionPrinterPass();
static KMETHOD LLVM_createRegionPrinterPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createRegionPrinterPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createRegionOnlyPrinterPass();
static KMETHOD LLVM_createRegionOnlyPrinterPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createRegionOnlyPrinterPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createLintPass();
static KMETHOD LLVM_createLintPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createLintPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

////## ModulePass LLVM.createPrintModulePass(raw_ostream *OS);
//KMETHOD LLVM_createPrintModulePass(KonohaContext *kctx, KonohaStack *sfp)
//{
//	raw_ostream **OS = konoha::object_cast<raw_ostream *>(sfp[0].asObject);
//	ModulePass *ptr = createPrintModulePass(*OS);
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	RETURN_(p);
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
//	RETURN_(p);
//}

////## ModulePass LLVM.createEdgeProfilerPass();
//KMETHOD LLVM_createEdgeProfilerPass(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ModulePass *ptr = createEdgeProfilerPass();
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	RETURN_(p);
//}

////## ModulePass LLVM.createOptimalEdgeProfilerPass();
//KMETHOD LLVM_createOptimalEdgeProfilerPass(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ModulePass *ptr = createOptimalEdgeProfilerPass();
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	RETURN_(p);
//}

////## ModulePass LLVM.createPathProfilerPass();
//KMETHOD LLVM_createPathProfilerPass(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ModulePass *ptr = createPathProfilerPass();
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	RETURN_(p);
//}

////## ModulePass LLVM.createGCOVProfilerPass(boolean emitNotes, boolean emitData, boolean use402Format);
//KMETHOD LLVM_createGCOVProfilerPass(KonohaContext *kctx, KonohaStack *sfp)
//{
//	bool emitNotes = sfp[0].boolValue;
//	bool emitData = sfp[1].boolValue;
//	bool use402Format = sfp[2].boolValue;
//	ModulePass *ptr = createGCOVProfilerPass(emitNotes,emitData,use402Format);
//	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
//	RETURN_(p);
//}

//## ModulePass LLVM.createStripSymbolsPass(bool onlyDebugInfo);
static KMETHOD LLVM_createStripSymbolsPass(KonohaContext *kctx, KonohaStack *sfp)
{
	bool onlyDebugInfo = sfp[0].boolValue;
	ModulePass *ptr = createStripSymbolsPass(onlyDebugInfo);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## ModulePass LLVM.createStripNonDebugSymbolsPass();
static KMETHOD LLVM_createStripNonDebugSymbolsPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createStripNonDebugSymbolsPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## ModulePass LLVM.createStripDeadDebugInfoPass();
static KMETHOD LLVM_createStripDeadDebugInfoPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createStripDeadDebugInfoPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## ModulePass LLVM.createConstantMergePass();
static KMETHOD LLVM_createConstantMergePass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createConstantMergePass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## ModulePass LLVM.createGlobalOptimizerPass();
static KMETHOD LLVM_createGlobalOptimizerPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createGlobalOptimizerPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## ModulePass LLVM.createGlobalDCEPass();
static KMETHOD LLVM_createGlobalDCEPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createGlobalDCEPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createFunctionInliningPass(int threshold);
static KMETHOD LLVM_createFunctionInliningPass(KonohaContext *kctx, KonohaStack *sfp)
{
	int threshold = sfp[0].intValue;
	Pass *ptr = createFunctionInliningPass(threshold);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createAlwaysInlinerPass();
static KMETHOD LLVM_createAlwaysInlinerPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createAlwaysInlinerPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createPruneEHPass();
static KMETHOD LLVM_createPruneEHPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createPruneEHPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## ModulePass LLVM.createInternalizePass(bool allButMain);
static KMETHOD LLVM_createInternalizePass(KonohaContext *kctx, KonohaStack *sfp)
{
	bool allButMain = sfp[0].boolValue;
	ModulePass *ptr = createInternalizePass(allButMain);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## ModulePass LLVM.createDeadArgEliminationPass();
static KMETHOD LLVM_createDeadArgEliminationPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createDeadArgEliminationPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createArgumentPromotionPass(int maxElements);
static KMETHOD LLVM_createArgumentPromotionPass(KonohaContext *kctx, KonohaStack *sfp)
{
	int maxElements = sfp[0].intValue;
	Pass *ptr = createArgumentPromotionPass(maxElements);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## ModulePass LLVM.createIPConstantPropagationPass();
static KMETHOD LLVM_createIPConstantPropagationPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createIPConstantPropagationPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## ModulePass LLVM.createIPSCCPPass();
static KMETHOD LLVM_createIPSCCPPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createIPSCCPPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createLoopExtractorPass();
static KMETHOD LLVM_createLoopExtractorPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createLoopExtractorPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createSingleLoopExtractorPass();
static KMETHOD LLVM_createSingleLoopExtractorPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createSingleLoopExtractorPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## ModulePass LLVM.createBlockExtractorPass();
static KMETHOD LLVM_createBlockExtractorPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createBlockExtractorPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## ModulePass LLVM.createStripDeadPrototypesPass();
static KMETHOD LLVM_createStripDeadPrototypesPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createStripDeadPrototypesPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createFunctionAttrsPass();
static KMETHOD LLVM_createFunctionAttrsPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createFunctionAttrsPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## ModulePass LLVM.createMergeFunctionsPass();
static KMETHOD LLVM_createMergeFunctionsPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createMergeFunctionsPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## ModulePass LLVM.createPartialInliningPass();
static KMETHOD LLVM_createPartialInliningPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ModulePass *ptr = createPartialInliningPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createConstantPropagationPass();
static KMETHOD LLVM_createConstantPropagationPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createConstantPropagationPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createSCCPPass();
static KMETHOD LLVM_createSCCPPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createSCCPPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createDeadInstEliminationPass();
static KMETHOD LLVM_createDeadInstEliminationPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createDeadInstEliminationPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createDeadCodeEliminationPass();
static KMETHOD LLVM_createDeadCodeEliminationPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createDeadCodeEliminationPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createDeadStoreEliminationPass();
static KMETHOD LLVM_createDeadStoreEliminationPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createDeadStoreEliminationPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createAggressiveDCEPass();
static KMETHOD LLVM_createAggressiveDCEPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createAggressiveDCEPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createScalarReplAggregatesPass(int threshold);
static KMETHOD LLVM_createScalarReplAggregatesPass(KonohaContext *kctx, KonohaStack *sfp)
{
	int threshold = sfp[0].intValue;
	FunctionPass *ptr = createScalarReplAggregatesPass(threshold);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createIndVarSimplifyPass();
static KMETHOD LLVM_createIndVarSimplifyPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createIndVarSimplifyPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createInstructionCombiningPass();
static KMETHOD LLVM_createInstructionCombiningPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createInstructionCombiningPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createLICMPass();
static KMETHOD LLVM_createLICMPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createLICMPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createLoopUnswitchPass(bool optimizeForSize);
static KMETHOD LLVM_createLoopUnswitchPass(KonohaContext *kctx, KonohaStack *sfp)
{
	bool optimizeForSize = sfp[0].boolValue;
	Pass *ptr = createLoopUnswitchPass(optimizeForSize);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createLoopInstSimplifyPass();
static KMETHOD LLVM_createLoopInstSimplifyPass(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;(void)(-(K_CALLDELTA));
	LLVM_TODO("NO SUPPORT");
#else
	Pass *ptr = createLoopInstSimplifyPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
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
	RETURN_(p);
}

//## Pass LLVM.createLoopRotatePass();
static KMETHOD LLVM_createLoopRotatePass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createLoopRotatePass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createLoopIdiomPass();
static KMETHOD LLVM_createLoopIdiomPass(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;(void)(-(K_CALLDELTA));
	LLVM_TODO("NO SUPPORT");
#else
	Pass *ptr = createLoopIdiomPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
#endif
}

//## FunctionPass LLVM.createPromoteMemoryToRegisterPass();
static KMETHOD LLVM_createPromoteMemoryToRegisterPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createPromoteMemoryToRegisterPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createDemoteRegisterToMemoryPass();
static KMETHOD LLVM_createDemoteRegisterToMemoryPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createDemoteRegisterToMemoryPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createReassociatePass();
static KMETHOD LLVM_createReassociatePass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createReassociatePass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createJumpThreadingPass();
static KMETHOD LLVM_createJumpThreadingPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createJumpThreadingPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createCFGSimplificationPass();
static KMETHOD LLVM_createCFGSimplificationPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createCFGSimplificationPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createBreakCriticalEdgesPass();
static KMETHOD LLVM_createBreakCriticalEdgesPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createBreakCriticalEdgesPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createLoopSimplifyPass();
static KMETHOD LLVM_createLoopSimplifyPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createLoopSimplifyPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createTailCallEliminationPass();
static KMETHOD LLVM_createTailCallEliminationPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createTailCallEliminationPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createLowerSwitchPass();
static KMETHOD LLVM_createLowerSwitchPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createLowerSwitchPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createBlockPlacementPass();
static KMETHOD LLVM_createBlockPlacementPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createBlockPlacementPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createLCSSAPass();
static KMETHOD LLVM_createLCSSAPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createLCSSAPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createEarlyCSEPass();
static KMETHOD LLVM_createEarlyCSEPass(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;(void)(-(K_CALLDELTA));
	LLVM_TODO("NO SUPPORT");
#else
	FunctionPass *ptr = createEarlyCSEPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
#endif
}

//## FunctionPass LLVM.createGVNPass(bool noLoads);
static KMETHOD LLVM_createGVNPass(KonohaContext *kctx, KonohaStack *sfp)
{
	bool noLoads = sfp[0].boolValue;
	FunctionPass *ptr = createGVNPass(noLoads);
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createMemCpyOptPass();
static KMETHOD LLVM_createMemCpyOptPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createMemCpyOptPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createLoopDeletionPass();
static KMETHOD LLVM_createLoopDeletionPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createLoopDeletionPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createSimplifyLibCallsPass();
static KMETHOD LLVM_createSimplifyLibCallsPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createSimplifyLibCallsPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createInstructionNamerPass();
static KMETHOD LLVM_createInstructionNamerPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createInstructionNamerPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## FunctionPass LLVM.createSinkingPass();
static KMETHOD LLVM_createSinkingPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createSinkingPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createLowerAtomicPass();
static KMETHOD LLVM_createLowerAtomicPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createLowerAtomicPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createCorrelatedValuePropagationPass();
static KMETHOD LLVM_createCorrelatedValuePropagationPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createCorrelatedValuePropagationPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

#if LLVM_VERSION >= 300
//## Pass LLVM.createObjCARCExpandPass();
static KMETHOD LLVM_createObjCARCExpandPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createObjCARCExpandPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createObjCARCContractPass();
static KMETHOD LLVM_createObjCARCContractPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createObjCARCContractPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## Pass LLVM.createObjCARCOptPass();
static KMETHOD LLVM_createObjCARCOptPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createObjCARCOptPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}
#endif

//## FunctionPass LLVM.createInstructionSimplifierPass();
static KMETHOD LLVM_createInstructionSimplifierPass(KonohaContext *kctx, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;(void)(-(K_CALLDELTA));
	LLVM_TODO("NO SUPPORT");
#else
	FunctionPass *ptr = createInstructionSimplifierPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
#endif
}

#if LLVM_VERSION >= 300
//## FunctionPass LLVM.createLowerExpectIntrinsicPass();
static KMETHOD LLVM_createLowerExpectIntrinsicPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createLowerExpectIntrinsicPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}
#endif

//## Pass LLVM.createUnifyFunctionExitNodesPass();
static KMETHOD LLVM_createUnifyFunctionExitNodesPass(KonohaContext *kctx, KonohaStack *sfp)
{
	Pass *ptr = createUnifyFunctionExitNodesPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## ImmutablePass LLVM.createTypeBasedAliasAnalysisPass();
static KMETHOD LLVM_createTypeBasedAliasAnalysisPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ImmutablePass *ptr = createTypeBasedAliasAnalysisPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## ImmutablePass LLVM.createBasicAliasAnalysisPass();
static KMETHOD LLVM_createBasicAliasAnalysisPass(KonohaContext *kctx, KonohaStack *sfp)
{
	ImmutablePass *ptr = createBasicAliasAnalysisPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

//## ImmutablePass LLVM.createVerifierPass();
static KMETHOD LLVM_createVerifierPass(KonohaContext *kctx, KonohaStack *sfp)
{
	FunctionPass *ptr = createVerifierPass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
}

#if LLVM_VERSION >= 301
//## BasicBlockPass LLVM.createBBVectorizePass();
static KMETHOD LLVM_createBBVectorizePass(KonohaContext *kctx, KonohaStack *sfp)
{
	BasicBlockPass *ptr = createBBVectorizePass();
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(ptr));
	RETURN_(p);
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
	RETURN_(p);
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
	RETURN_(p);
}

static KMETHOD LLVM_parseBitcodeFile(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *Str = sfp[1].asString;
	LLVMContext &Context = getGlobalContext();
	std::string ErrMsg;
	OwningPtr<MemoryBuffer> BufferPtr;
#if LLVM_VERSION <= 208
	const char *fname = S_text(Str);
	BufferPtr.reset(MemoryBuffer::getFile(fname, &ErrMsg));
	if (!BufferPtr) {
		std::cout << "Could not open file " << ErrMsg << std::endl;
	}
#else
	std::string fname(S_text(Str));
	if (error_code ec = MemoryBuffer::getFile(fname, BufferPtr)) {
		std::cout << "Could not open file " << ec.message() << std::endl;
	}
#endif
	MemoryBuffer *Buffer = BufferPtr.take();
	//Module *m = getLazyBitcodeModule(Buffer, Context, &ErrMsg);
	Module *m = ParseBitcodeFile(Buffer, Context, &ErrMsg);
	if (!m) {
		std::cout << "error" << ErrMsg << std::endl;
	}
	kObject *p = new_ReturnCppObject(kctx, sfp, WRAP(m));
	RETURN_(p);
}

//TODO Scriptnize
static KMETHOD Instruction_setMetadata(KonohaContext *kctx _UNUSED_, KonohaStack *sfp)
{
#if LLVM_VERSION <= 208
	(void)kctx;(void)sfp;(void)(-(K_CALLDELTA));
	LLVM_TODO("NO SUPPORT");
#else
	Instruction *inst = konoha::object_cast<Instruction *>(sfp[0].asObject);
	Module *m = konoha::object_cast<Module *>(sfp[1].asObject);
	kString *Str = sfp[2].s;
	kint_t N = Int_to(kint_t,sfp[3]);
	Value *Info[] = {
		ConstantInt::get(Type::getInt32Ty(getGlobalContext()), N)
	};
	LLVMContext &Context = getGlobalContext();
	MDNode *node = MDNode::get(Context, Info);
	NamedMDNode *NMD = m->getOrInsertNamedMetadata(S_text(Str));
	unsigned KindID = Context.getMDKindID(S_text(Str));
	NMD->addOperand(node);
	inst->setMetadata(KindID, node);
#endif
	RETURNvoid_();
}

//FIXME TODO stupid down cast
static KMETHOD Object_toValue(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	RETURN_(sfp[0].asObject);
}

//FIXME TODO stupid down cast
static KMETHOD Object_toType(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	RETURN_(sfp[0].asObject);
}

//FIXME TODO stupid down cast
static KMETHOD Object_toModule(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	RETURN_(sfp[0].asObject);
}

//FIXME TODO stupid down cast
static KMETHOD Object_toExecutionEngine(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	RETURN_(sfp[0].asObject);
}

//FIXME TODO stupid down cast
static KMETHOD Object_asFunction(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	RETURN_(sfp[0].asObject);
}

//FIXME TODO stupid down cast
static KMETHOD Object_toIRBuilder(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	RETURN_(sfp[0].asObject);
}

//FIXME TODO stupid down cast
static KMETHOD Object_asFunctionType(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	RETURN_(sfp[0].asObject);
}

//FIXME TODO stupid down cast
static KMETHOD Object_toLLVMBasicBlock(KonohaContext *kctx, KonohaStack *sfp)
{
	(void)kctx;
	RETURN_(sfp[0].asObject);
}

static KDEFINE_INT_CONST IntIntrinsic[] = {
	{"Pow"  , TY_int, (int) Intrinsic::pow},
	{"Sqrt" , TY_int, (int) Intrinsic::sqrt},
	{"Exp"  , TY_int, (int) Intrinsic::exp},
	{"Log10", TY_int, (int) Intrinsic::log10},
	{"Log"  , TY_int, (int) Intrinsic::log},
	{"Sin"  , TY_int, (int) Intrinsic::sin},
	{"Cos"  , TY_int, (int) Intrinsic::cos},
	{NULL, 0, 0}
};

static KDEFINE_INT_CONST IntGlobalVariable[] = {
	{"ExternalLinkage",                 TY_int, GlobalValue::ExternalLinkage},
	{"AvailableExternallyLinkage",      TY_int, GlobalValue::AvailableExternallyLinkage},
	{"LinkOnceAnyLinkage",              TY_int, GlobalValue::LinkOnceODRLinkage},
	{"WeakAnyLinkage",                  TY_int, GlobalValue::WeakAnyLinkage},
	{"WeakODRLinkage",                  TY_int, GlobalValue::WeakODRLinkage},
	{"AppendingLinkage",                TY_int, GlobalValue::AppendingLinkage},
	{"InternalLinkage",                 TY_int, GlobalValue::InternalLinkage},
	{"PrivateLinkage",                  TY_int, GlobalValue::PrivateLinkage},
	{"LinkerPrivateLinkage",            TY_int, GlobalValue::LinkerPrivateLinkage},
	{"LinkerPrivateWeakLinkage",        TY_int, GlobalValue::LinkerPrivateWeakLinkage},
#if LLVM_VERSION < 302
	{"LinkerPrivateWeakDefAutoLinkage", TY_int, GlobalValue::LinkerPrivateWeakDefAutoLinkage},
#endif
	{"DLLImportLinkage",                TY_int, GlobalValue::DLLImportLinkage},
	{"DLLExportLinkage",                TY_int, GlobalValue::DLLExportLinkage},
	{"ExternalWeakLinkage",             TY_int, GlobalValue::ExternalWeakLinkage},
	{"CommonLinkage",                   TY_int, GlobalValue::CommonLinkage},
	{NULL, 0, 0}
};

#if LLVM_VERSION >= 301
#define C_(S) {#S , TY_int, S ## _i}
#else
#define C_(S) {#S , TY_int, S}
#endif
using namespace llvm::Attribute;
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
//void defGlobalValue(KonohaContext *kctx _UNUSED_, ktype_t cid _UNUSED_, kclassdef_t *cdef)
//{
//	cdef->name = "GlobalValue";
//}
//
//void constGlobalValue(KonohaContext *kctx, ktype_t cid, const knh_LoaderAPI_t *kapi)
//{
//	kapi->loadClassIntConst(kctx, cid, IntGlobalVariable);
//}
//
//void defIntrinsic(KonohaContext *kctx _UNUSED_, ktype_t cid _UNUSED_, kclassdef_t *cdef)
//{
//	cdef->name = "Intrinsic";
//}
//
//void constIntrinsic(KonohaContext *kctx, ktype_t cid, const knh_LoaderAPI_t *kapi)
//{
//	kapi->loadClassIntConst(kctx, cid, IntIntrinsic);
//}
//
//
//void defAttributes(KonohaContext *kctx _UNUSED_, ktype_t cid _UNUSED_, kclassdef_t *cdef)
//{
//	cdef->name = "Attributes";
//}
//
//void constAttributes(KonohaContext *kctx _UNUSED_, ktype_t cid _UNUSED_, const knh_LoaderAPI_t *kapi)
//{
//	kapi->loadClassIntConst(kctx, cid, IntAttributes);
//}

static void kmodllvm_setup(KonohaContext *kctx, struct KonohaModule *def, int newctx)
{
	(void)kctx;(void)def;(void)newctx;
}

static void kmodllvm_reftrace(KonohaContext *kctx, struct KonohaModule *baseh)
{
	(void)kctx;(void)baseh;
}

static void kmodllvm_free(KonohaContext *kctx, struct KonohaModule *baseh)
{
	KFREE(baseh, sizeof(kmodllvm_t));
}

#define _Public   kMethod_Public
#define _Static   kMethod_Static
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _Im       kMethod_Immutable
#define _F(F)   (intptr_t)(F)

static kbool_t llvm_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char **args, kfileline_t pline)
{
	KRequirePackage("konoha.float", pline);
	(void)argc;(void)args;
	kmodllvm_t *base = (kmodllvm_t*)KCALLOC(sizeof(kmodllvm_t), 1);
	base->h.name     = "llvm";
	base->h.setup    = kmodllvm_setup;
	base->h.reftrace = kmodllvm_reftrace;
	base->h.free     = kmodllvm_free;
	KLIB KonohaRuntime_setModule(kctx, MOD_llvm, &base->h, pline);

#define DEFINE_CLASS_CPP(\
	/*const char * */structname,\
	/*ktype_t      */typeId,         /*kshortflag_t    */cflag,\
	/*ktype_t      */baseTypeId,     /*ktype_t         */superTypeId,\
	/*ktype_t      */rtype,          /*kushort_t       */cparamsize,\
	/*struct kparamtype_t   **/cparamItems,\
	/*size_t     */cstruct_size,\
	/*KonohaClassField   **/fieldItems,\
	/*kushort_t  */fieldsize,       /*kushort_t */fieldAllocSize,\
		init,\
		reftrace,\
		free,\
		fnull,\
		p,\
		unbox,\
		compareObject,\
		compareUnboxValue,\
		hasField,\
		getFieldObjectValue,\
		setFieldObjectValue,\
		getFieldUnboxValue,\
		setFieldUnboxValue,\
		initdef,\
		isSubType,\
		realtype) {\
	/*const char * */structname,\
	/*ktype_t      */typeId,         /*kshortflag_t    */cflag,\
	/*ktype_t      */baseTypeId,     /*ktype_t         */superTypeId,\
	/*ktype_t      */rtype,          /*kushort_t       */cparamsize,\
	/*struct kparamtype_t   * */cparamItems,\
	/*size_t     */cstruct_size,\
	/*KonohaClassField   * */fieldItems,\
	/*kushort_t  */fieldsize,       /*kushort_t */fieldAllocSize,\
		init,\
		reftrace,\
		free,\
		fnull,\
		p,\
		unbox,\
		compareObject,\
		compareUnboxValue,\
		hasField,\
		getFieldObjectValue,\
		setFieldObjectValue,\
		getFieldUnboxValue,\
		setFieldUnboxValue,\
		initdef,\
		isSubType,\
		realtype}

#define DEFINE_CLASS_0(NAME, FN_INIT, FN_FREE, FN_COMPARE) DEFINE_CLASS_CPP(\
		NAME,\
		TY_newid, 0,\
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
		FN_COMPARE/*compareObject*/,\
		0/*compareUnboxValue*/,\
		0/*hasField*/,\
		0/*getFieldObjectValue*/,\
		0/*setFieldObjectValue*/,\
		0/*getFieldUnboxValue*/,\
		0/*setFieldUnboxValue*/,\
		0/*initdef*/,\
		0/*isSubType*/,\
		0/*realtype*/)


	static KDEFINE_CLASS ValueDef = DEFINE_CLASS_0("Value", 0, 0, 0);
	base->cValue = KLIB kNameSpace_defineClass(kctx, ns, NULL, &ValueDef, pline);

	static const char *TypeDefName[] = {
		"Type",
		"IntegerType",
		"PointerType",
		"FunctionType",
		"ArrayType",
		"StructType"
	};
	KonohaClass *CT_TypeTBL[6];
	KonohaClass *CT_BasicBlock, *CT_IRBuilder;
#define TY_BasicBlock  (CT_BasicBlock)->typeId
#define TY_IRBuilder   (CT_IRBuilder)->typeId
#define TY_Type         (CT_TypeTBL[0])->typeId
#define TY_integerType  (CT_TypeTBL[1])->typeId
#define TY_PointerType  (CT_TypeTBL[2])->typeId
#define TY_FunctionType (CT_TypeTBL[3])->typeId
#define TY_ArrayType    (CT_TypeTBL[4])->typeId
#define TY_StructType   (CT_TypeTBL[5])->typeId
	{
		static KDEFINE_CLASS TypeDef;
		bzero(&TypeDef, sizeof(KDEFINE_CLASS));
		TypeDef.typeId  = TY_newid;
		TypeDef.init = Type_init;
		TypeDef.free = Type_free;
		for (int i = 0; i < 6; i++) {
			TypeDef.structname = TypeDefName[i];
			CT_TypeTBL[i] = KLIB kNameSpace_defineClass(kctx, ns, NULL, &TypeDef, 0);
		}
	}
	static KDEFINE_CLASS BasicBlockDef = DEFINE_CLASS_0("LLVMBasicBlock",0, 0, BasicBlock_compareTo);
	CT_BasicBlock = KLIB kNameSpace_defineClass(kctx, ns, NULL, &BasicBlockDef, pline);

	static KDEFINE_CLASS IRBuilderDef = DEFINE_CLASS_0("IRBuilder", 0, 0, 0);
	CT_IRBuilder = KLIB kNameSpace_defineClass(kctx, ns, NULL, &IRBuilderDef, pline);
#if LLVM_VERSION >= 300
	static KDEFINE_CLASS PassManagerBuilderDef = DEFINE_CLASS_0("PassManagerBuilder",
			PassManagerBuilder_ptr_init, PassManagerBuilder_ptr_free, 0);
	KonohaClass *CT_PassManagerBuilder = KLIB kNameSpace_defineClass(kctx, ns, NULL, &PassManagerBuilderDef, pline);
#define TY_PassManagerBuilder         (CT_PassManagerBuilder)->typeId
#endif
	static KDEFINE_CLASS PassManagerDef = DEFINE_CLASS_0("PassManager",
		PassManager_ptr_init, PassManager_ptr_free, 0);
	static KDEFINE_CLASS FunctionPassManagerDef = DEFINE_CLASS_0("FunctionPassManager",
			FunctionPassManager_ptr_init, FunctionPassManager_ptr_free, 0);
	KonohaClass *CT_PassManager = KLIB kNameSpace_defineClass(kctx, ns, NULL, &PassManagerDef, pline);
	KonohaClass *CT_FunctionPassManager = KLIB kNameSpace_defineClass(kctx, ns, NULL, &FunctionPassManagerDef, pline);
	KonohaClass *CT_InstTBL[21];
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
		InstDef.typeId  = TY_newid;
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif
		//InstDef.init = Inst_init;
		//InstDef.free = Inst_free;
		for (unsigned int i = 0; i < ARRAY_SIZE(InstDefName); i++) {
			InstDef.structname = InstDefName[i];
			CT_InstTBL[i] = KLIB kNameSpace_defineClass(kctx, ns, NULL, &InstDef, pline);
		}
	}
#define TY_Instruction         (CT_InstTBL[ 0])->typeId
#define TY_AllocaInst          (CT_InstTBL[ 1])->typeId
#define TY_LoadInst            (CT_InstTBL[ 2])->typeId
#define TY_StoreInst           (CT_InstTBL[ 3])->typeId
#define TY_GetElementPtrInst   (CT_InstTBL[ 4])->typeId
#define TY_PHINode             (CT_InstTBL[ 5])->typeId
#define TY_Module              (CT_InstTBL[ 6])->typeId
#define TY_Function            (CT_InstTBL[ 7])->typeId
#define TY_ExecutionEngine     (CT_InstTBL[ 8])->typeId
#define TY_GlobalVariable      (CT_InstTBL[ 9])->typeId
#define TY_Argument            (CT_InstTBL[10])->typeId
#define TY_Constant            (CT_InstTBL[11])->typeId
#define TY_ConstantInt         (CT_InstTBL[12])->typeId
#define TY_ConstantFP          (CT_InstTBL[13])->typeId
#define TY_ConstantStruct      (CT_InstTBL[14])->typeId
#define TY_ConstantPointerNull (CT_InstTBL[15])->typeId
#define TY_ConstantExpr        (CT_InstTBL[16])->typeId
#define TY_LLVM                (CT_InstTBL[17])->typeId
#define TY_LibCallInfo         (CT_InstTBL[18])->typeId
#define TY_DynamicLibrary      (CT_InstTBL[19])->typeId
#define TY_intrinsic           (CT_InstTBL[20])->typeId

	KonohaClass *CT_PassTBL[4];
	{
		static const char *PassDefName[] = {
			"Pass",
			"ImmutablePass",
			"FunctionPass",
			"ModulePass",
		};
		static KDEFINE_CLASS PassDef;
		bzero(&PassDef, sizeof(KDEFINE_CLASS));
		PassDef.typeId  = TY_newid;
		//InstDef.init = Inst_init;
		//InstDef.free = Inst_free;
		for (int i = 0; i < 4; i++) {
			PassDef.structname = PassDefName[i];
			CT_PassTBL[i] = KLIB kNameSpace_defineClass(kctx, ns, NULL, &PassDef, pline);
		}
	}
#define TY_Pass          (CT_PassTBL[0])->typeId
#define TY_ImmutablePass (CT_PassTBL[1])->typeId
#define TY_FunctionPass  (CT_PassTBL[2])->typeId
#define TY_ModulePass    (CT_PassTBL[3])->typeId

#define TY_PassManager         (CT_PassManager)->typeId
#define TY_FunctionPassManager (CT_FunctionPassManager)->typeId
	/* TODO */
	kparamtype_t P_TypeArray[] = {{TY_Type, 0}};
	int TY_TypeArray = (KLIB KonohaClass_Generics(kctx, CT_Array, TY_void, 1, P_TypeArray))->typeId;

	kparamtype_t P_ValueArray[] = {{TY_Value, 0}};
	int TY_ValueArray = (KLIB KonohaClass_Generics(kctx, CT_Array, TY_void, 1, P_ValueArray))->typeId;
#define TY_Array_Value    (TY_ValueArray)
#define TY_Array_Type     (TY_TypeArray)
#define TY_Array_Constant (TY_Array)
#define TY_Array_Int      (TY_Array)
#define TY_NativeFunction (TY_int)

	assert(CT_Float != NULL && "please import konoha.float PACKAGE first");
	intptr_t methoddata[] = {
		_Public|_Static, _F(Type_getVoidTy), TY_Type, TY_Type, MN_("getVoidTy"), 0,
		_Public|_Static, _F(Type_getLabelTy), TY_Type, TY_Type, MN_("getLabelTy"), 0,
		_Public|_Static, _F(Type_getFloatTy), TY_Type, TY_Type, MN_("getFloatTy"), 0,
		_Public|_Static, _F(Type_getDoubleTy), TY_Type, TY_Type, MN_("getDoubleTy"), 0,
		_Public|_Static, _F(Type_getMetadataTy), TY_Type, TY_Type, MN_("getMetadataTy"), 0,
		_Public|_Static, _F(Type_getX86FP80Ty), TY_Type, TY_Type, MN_("getX86_FP80Ty"), 0,
		_Public|_Static, _F(Type_getFP128Ty), TY_Type, TY_Type, MN_("getFP128Ty"), 0,
		_Public|_Static, _F(Type_getPPCFP128Ty), TY_Type, TY_Type, MN_("getPPC_FP128Ty"), 0,
		_Public|_Static, _F(Type_getX86MMXTy), TY_Type, TY_Type, MN_("getX86_MMXTy"), 0,
		_Public|_Static, _F(Type_getInt1Ty), TY_Type, TY_Type, MN_("getInt1Ty"), 0,
		_Public|_Static, _F(Type_getInt8Ty), TY_Type, TY_Type, MN_("getInt8Ty"), 0,
		_Public|_Static, _F(Type_getInt16Ty), TY_Type, TY_Type, MN_("getInt16Ty"), 0,
		_Public|_Static, _F(Type_getInt32Ty), TY_Type, TY_Type, MN_("getInt32Ty"), 0,
		_Public|_Static, _F(Type_getInt64Ty), TY_Type, TY_Type, MN_("getInt64Ty"), 0,
		_Public|_Static, _F(PointerType_get), TY_Type/*TODO*/, TY_PointerType, MN_("get"), 1, TY_Type, FN_("type"),
		_Public|_Static, _F(Type_getFloatPtrTy),    TY_Type, TY_Type, MN_("getFloatPtrTy"), 0,
		_Public|_Static, _F(Type_getDoublePtrTy),   TY_Type, TY_Type, MN_("getDoublePtrTy"), 0,
		_Public|_Static, _F(Type_getX86FP80PtrTy),  TY_Type, TY_Type, MN_("getX86_FP80PtrTy"), 0,
		_Public|_Static, _F(Type_getFP128PtrTy),    TY_Type, TY_Type, MN_("getFP128PtrTy"), 0,
		_Public|_Static, _F(Type_getPPCFP128PtrTy), TY_Type, TY_Type, MN_("getPPC_FP128PtrTy"), 0,
		_Public|_Static, _F(Type_getX86MMXPtrTy),   TY_Type, TY_Type, MN_("getX86_MMXPtrTy"), 0,
		_Public|_Static, _F(Type_getInt1PtrTy),  TY_Type, TY_Type, MN_("getInt1PtrTy"), 0,
		_Public|_Static, _F(Type_getInt8PtrTy),  TY_Type, TY_Type, MN_("getInt8PtrTy"), 0,
		_Public|_Static, _F(Type_getInt16PtrTy), TY_Type, TY_Type, MN_("getInt16PtrTy"), 0,
		_Public|_Static, _F(Type_getInt32PtrTy), TY_Type, TY_Type, MN_("getInt32PtrTy"), 0,
		_Public|_Static, _F(Type_getInt64PtrTy), TY_Type, TY_Type, MN_("getInt64PtrTy"), 0,
		_Public, _F(IRBuilder_new), TY_IRBuilder, TY_IRBuilder, MN_("new"), 1, TY_BasicBlock, FN_("bb"),
		_Public, _F(IRBuilder_createRetVoid), TY_Value, TY_IRBuilder, MN_("createRetVoid"), 0,
		_Public, _F(IRBuilder_createRet),     TY_Value, TY_IRBuilder, MN_("createRet"), 1, TY_Value, FN_("v"),
		_Public, _F(IRBuilder_createBr),      TY_Value, TY_IRBuilder, MN_("createBr"), 1, TY_BasicBlock, FN_("dest"),
		_Public, _F(IRBuilder_createCondBr),  TY_Value, TY_IRBuilder, MN_("createCondBr"), 3, TY_Value, FN_("cond"),TY_BasicBlock, FN_("trueBB"),TY_BasicBlock, FN_("falseBB"),
		_Public, _F(IRBuilder_createSwitch),  TY_Value, TY_IRBuilder, MN_("createSwitch"), 2, TY_Value, FN_("v"),TY_BasicBlock, FN_("dest"),
		_Public, _F(IRBuilder_createIndirectBr), TY_Value, TY_IRBuilder, MN_("createIndirectBr"), 1, TY_Value, FN_("addr"),
		_Public, _F(IRBuilder_createInvoke0), TY_Value, TY_IRBuilder, MN_("createInvoke0"), 3, TY_Value, FN_("callee"),TY_BasicBlock, FN_("normalDest"),TY_BasicBlock, FN_("unwindDest"),
		_Public, _F(IRBuilder_createInvoke1), TY_Value, TY_IRBuilder, MN_("createInvoke1"), 4, TY_Value, FN_("callee"),TY_BasicBlock, FN_("normalDest"),TY_BasicBlock, FN_("unwindDest"),TY_Value, FN_("arg1"),
		_Public, _F(IRBuilder_createInvoke3), TY_Value, TY_IRBuilder, MN_("createInvoke3"), 6, TY_Value, FN_("callee"),TY_BasicBlock, FN_("normalDest"),TY_BasicBlock, FN_("unwindDest"),TY_Value, FN_("arg1"),TY_Value, FN_("arg2"),TY_Value, FN_("arg3"),
		_Public, _F(IRBuilder_createUnreachable), TY_Value, TY_IRBuilder, MN_("createUnreachable"), 0,
		_Public, _F(IRBuilder_createAdd),    TY_Value, TY_IRBuilder, MN_("createAdd"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createNSWAdd), TY_Value, TY_IRBuilder, MN_("createNSWAdd"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createNUWAdd), TY_Value, TY_IRBuilder, MN_("createNUWAdd"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createFAdd),   TY_Value, TY_IRBuilder, MN_("createFAdd"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createSub),    TY_Value, TY_IRBuilder, MN_("createSub"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createNSWSub), TY_Value, TY_IRBuilder, MN_("createNSWSub"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createNUWSub), TY_Value, TY_IRBuilder, MN_("createNUWSub"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createFSub),   TY_Value, TY_IRBuilder, MN_("createFSub"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createMul),    TY_Value, TY_IRBuilder, MN_("createMul"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createNSWMul), TY_Value, TY_IRBuilder, MN_("createNSWMul"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createNUWMul), TY_Value, TY_IRBuilder, MN_("createNUWMul"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createFMul),   TY_Value, TY_IRBuilder, MN_("createFMul"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createUDiv),   TY_Value, TY_IRBuilder, MN_("createUDiv"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createExactUDiv), TY_Value, TY_IRBuilder, MN_("createExactUDiv"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createSDiv),      TY_Value, TY_IRBuilder, MN_("createSDiv"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createExactSDiv), TY_Value, TY_IRBuilder, MN_("createExactSDiv"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createFDiv),   TY_Value, TY_IRBuilder, MN_("createFDiv"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createURem),   TY_Value, TY_IRBuilder, MN_("createURem"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createSRem),   TY_Value, TY_IRBuilder, MN_("createSRem"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createFRem),   TY_Value, TY_IRBuilder, MN_("createFRem"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createShl),    TY_Value, TY_IRBuilder, MN_("createShl"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createLShr),   TY_Value, TY_IRBuilder, MN_("createLShr"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createAShr),   TY_Value, TY_IRBuilder, MN_("createAShr"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createAnd),    TY_Value, TY_IRBuilder, MN_("createAnd"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createOr),     TY_Value, TY_IRBuilder, MN_("createOr"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createXor),    TY_Value, TY_IRBuilder, MN_("createXor"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createNeg),    TY_Value, TY_IRBuilder, MN_("createNeg"), 1, TY_Value, FN_("v"),
		_Public, _F(IRBuilder_createNSWNeg), TY_Value, TY_IRBuilder, MN_("createNSWNeg"), 1, TY_Value, FN_("v"),
		_Public, _F(IRBuilder_createNUWNeg), TY_Value, TY_IRBuilder, MN_("createNUWNeg"), 1, TY_Value, FN_("v"),
		_Public, _F(IRBuilder_createFNeg),   TY_Value, TY_IRBuilder, MN_("createFNeg"), 1, TY_Value, FN_("v"),
		_Public, _F(IRBuilder_createNot),    TY_Value, TY_IRBuilder, MN_("createNot"), 1, TY_Value, FN_("v"),
		_Public, _F(IRBuilder_createAlloca), TY_Value, TY_IRBuilder, MN_("createAlloca"), 2, TY_Type, FN_("ty"),TY_Value, FN_("arraySize"),
		_Public, _F(AllocaInst_new), TY_AllocaInst, TY_AllocaInst, MN_("new"), 2, TY_Type, FN_("ty"),TY_Value, FN_("arraySize"),
		_Public, _F(IRBuilder_createLoad), TY_Value, TY_IRBuilder, MN_("createLoad"), 2, TY_Value, FN_("ptr"),TY_boolean, FN_("isVolatile"),
		_Public, _F(LoadInst_new), TY_LoadInst, TY_LoadInst, MN_("new"), 1, TY_Value, FN_("ptr"),
		_Public, _F(IRBuilder_createStore), TY_Value, TY_IRBuilder, MN_("createStore"), 3, TY_Value, FN_("val"),TY_Value, FN_("ptr"),TY_boolean, FN_("isVolatile"),
		_Public, _F(StoreInst_new), TY_StoreInst, TY_StoreInst, MN_("new"), 2, TY_Value, FN_("val"),TY_Value, FN_("ptr"),
		_Public|_Static, _F(GetElementPtrInst_create), TY_GetElementPtrInst, TY_GetElementPtrInst, MN_("create"), 2, TY_Value, FN_("ptr"),TY_Array_Value, FN_("idxList"),
		_Public|_Static, _F(GetElementPtrInst_createInBounds), TY_GetElementPtrInst, TY_GetElementPtrInst, MN_("createInBounds"), 2, TY_Value, FN_("ptr"),TY_Array_Value, FN_("idxList"),
		_Public, _F(IRBuilder_createGEP), TY_Value, TY_IRBuilder, MN_("createGEP"), 2, TY_Value, FN_("ptr"),TY_Array_Value, FN_("idxList"),
		_Public, _F(IRBuilder_createInBoundsGEP), TY_Value, TY_IRBuilder, MN_("createInBoundsGEP"), 2, TY_Value, FN_("ptr"),TY_Array_Value, FN_("idxList"),
		_Public, _F(IRBuilder_createGEP1), TY_Value, TY_IRBuilder, MN_("createGEP1"), 2, TY_Value, FN_("ptr"),TY_Value, FN_("idx"),
		_Public, _F(IRBuilder_createInBoundsGEP1), TY_Value, TY_IRBuilder, MN_("createInBoundsGEP1"), 2, TY_Value, FN_("ptr"),TY_Value, FN_("idx"),
		_Public, _F(IRBuilder_createConstGEP132),  TY_Value, TY_IRBuilder, MN_("createConstGEP1_32"), 2, TY_Value, FN_("ptr"),TY_int, FN_("idx0"),
		_Public, _F(IRBuilder_createConstGEP232),  TY_Value, TY_IRBuilder, MN_("createConstGEP2_32"), 3, TY_Value, FN_("ptr"),TY_int, FN_("idx0"),TY_int, FN_("idx1"),
		_Public, _F(IRBuilder_createConstGEP164),  TY_Value, TY_IRBuilder, MN_("createConstGEP1_64"), 2, TY_Value, FN_("ptr"),TY_int, FN_("idx0"),
		_Public, _F(IRBuilder_createConstGEP264),  TY_Value, TY_IRBuilder, MN_("createConstGEP2_64"), 3, TY_Value, FN_("ptr"),TY_int, FN_("idx0"),TY_int, FN_("idx1"),
		_Public, _F(IRBuilder_createConstInBoundsGEP132), TY_Value, TY_IRBuilder, MN_("createConstInBoundsGEP1_32"), 2, TY_Value, FN_("ptr"),TY_int, FN_("idx0"),
		_Public, _F(IRBuilder_createConstInBoundsGEP232), TY_Value, TY_IRBuilder, MN_("createConstInBoundsGEP2_32"), 3, TY_Value, FN_("ptr"),TY_int, FN_("idx0"),TY_int, FN_("idx1"),
		_Public, _F(IRBuilder_createConstInBoundsGEP164), TY_Value, TY_IRBuilder, MN_("createConstInBoundsGEP1_64"), 2, TY_Value, FN_("ptr"),TY_int, FN_("idx0"),
		_Public, _F(IRBuilder_createConstInBoundsGEP264), TY_Value, TY_IRBuilder, MN_("createConstInBoundsGEP2_64"), 3, TY_Value, FN_("ptr"),TY_int, FN_("idx0"),TY_int, FN_("idx1"),
		_Public, _F(IRBuilder_createStructGEP), TY_Value, TY_IRBuilder, MN_("createStructGEP"), 2, TY_Value, FN_("ptr"),TY_int, FN_("idx"),
		_Public, _F(IRBuilder_createGlobalString), TY_Value, TY_IRBuilder, MN_("createGlobalString"), 1, TY_String, FN_("str"),
		_Public, _F(IRBuilder_createGlobalStringPtr), TY_Value, TY_IRBuilder, MN_("createGlobalStringPtr"), 1, TY_String, FN_("str"),
		_Public, _F(IRBuilder_createTrunc),    TY_Value, TY_IRBuilder, MN_("createTrunc"), 2, TY_Value, FN_("v"),TY_Type, FN_("destTy"),
		_Public, _F(IRBuilder_createZExt),     TY_Value, TY_IRBuilder, MN_("createZExt"), 2, TY_Value, FN_("v"),TY_Type, FN_("destTy"),
		_Public, _F(IRBuilder_createSExt),     TY_Value, TY_IRBuilder, MN_("createSExt"), 2, TY_Value, FN_("v"),TY_Type, FN_("destTy"),
		_Public, _F(IRBuilder_createFPToUI),   TY_Value, TY_IRBuilder, MN_("createFPToUI"), 2, TY_Value, FN_("v"),TY_Type, FN_("destTy"),
		_Public, _F(IRBuilder_createFPToSI),   TY_Value, TY_IRBuilder, MN_("createFPToSI"), 2, TY_Value, FN_("v"),TY_Type, FN_("destTy"),
		_Public, _F(IRBuilder_createUIToFP),   TY_Value, TY_IRBuilder, MN_("createUIToFP"), 2, TY_Value, FN_("v"),TY_Type, FN_("destTy"),
		_Public, _F(IRBuilder_createSIToFP),   TY_Value, TY_IRBuilder, MN_("createSIToFP"), 2, TY_Value, FN_("v"),TY_Type, FN_("destTy"),
		_Public, _F(IRBuilder_createFPTrunc),  TY_Value, TY_IRBuilder, MN_("createFPTrunc"), 2, TY_Value, FN_("v"),TY_Type, FN_("destTy"),
		_Public, _F(IRBuilder_createFPExt),    TY_Value, TY_IRBuilder, MN_("createFPExt"), 2, TY_Value, FN_("v"),TY_Type, FN_("destTy"),
		_Public, _F(IRBuilder_createPtrToInt), TY_Value, TY_IRBuilder, MN_("createPtrToInt"), 2, TY_Value, FN_("v"),TY_Type, FN_("destTy"),
		_Public, _F(IRBuilder_createIntToPtr), TY_Value, TY_IRBuilder, MN_("createIntToPtr"), 2, TY_Value, FN_("v"),TY_Type, FN_("destTy"),
		_Public, _F(IRBuilder_createBitCast),  TY_Value, TY_IRBuilder, MN_("createBitCast"), 2, TY_Value, FN_("v"),TY_Type, FN_("destTy"),
		_Public, _F(IRBuilder_createZExtOrBitCast),  TY_Value, TY_IRBuilder, MN_("createZExtOrBitCast"), 2, TY_Value, FN_("v"),TY_Type, FN_("destTy"),
		_Public, _F(IRBuilder_createSExtOrBitCast),  TY_Value, TY_IRBuilder, MN_("createSExtOrBitCast"), 2, TY_Value, FN_("v"),TY_Type, FN_("destTy"),
		_Public, _F(IRBuilder_createTruncOrBitCast), TY_Value, TY_IRBuilder, MN_("createTruncOrBitCast"), 2, TY_Value, FN_("v"),TY_Type, FN_("destTy"),
		_Public, _F(IRBuilder_createPointerCast),    TY_Value, TY_IRBuilder, MN_("createPointerCast"), 2, TY_Value, FN_("v"),TY_Type, FN_("destTy"),
		_Public, _F(IRBuilder_createIntCast), TY_Value, TY_IRBuilder, MN_("createIntCast"), 3, TY_Value, FN_("v"),TY_Type, FN_("destTy"),TY_boolean, FN_("isSigned"),
		_Public, _F(IRBuilder_createFPCast),  TY_Value, TY_IRBuilder, MN_("createFPCast"), 2, TY_Value, FN_("v"),TY_Type, FN_("destTy"),
		_Public, _F(IRBuilder_createICmpEQ),  TY_Value, TY_IRBuilder, MN_("createICmpEQ"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createICmpNE),  TY_Value, TY_IRBuilder, MN_("createICmpNE"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createICmpUGT), TY_Value, TY_IRBuilder, MN_("createICmpUGT"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createICmpUGE), TY_Value, TY_IRBuilder, MN_("createICmpUGE"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createICmpULT), TY_Value, TY_IRBuilder, MN_("createICmpULT"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createICmpULE), TY_Value, TY_IRBuilder, MN_("createICmpULE"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createICmpSGT), TY_Value, TY_IRBuilder, MN_("createICmpSGT"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createICmpSGE), TY_Value, TY_IRBuilder, MN_("createICmpSGE"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createICmpSLT), TY_Value, TY_IRBuilder, MN_("createICmpSLT"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createICmpSLE), TY_Value, TY_IRBuilder, MN_("createICmpSLE"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createFCmpOEQ), TY_Value, TY_IRBuilder, MN_("createFCmpOEQ"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createFCmpOGT), TY_Value, TY_IRBuilder, MN_("createFCmpOGT"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createFCmpOGE), TY_Value, TY_IRBuilder, MN_("createFCmpOGE"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createFCmpOLT), TY_Value, TY_IRBuilder, MN_("createFCmpOLT"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createFCmpOLE), TY_Value, TY_IRBuilder, MN_("createFCmpOLE"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createFCmpONE), TY_Value, TY_IRBuilder, MN_("createFCmpONE"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createFCmpORD), TY_Value, TY_IRBuilder, MN_("createFCmpORD"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createFCmpUNO), TY_Value, TY_IRBuilder, MN_("createFCmpUNO"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createFCmpUEQ), TY_Value, TY_IRBuilder, MN_("createFCmpUEQ"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createFCmpUGT), TY_Value, TY_IRBuilder, MN_("createFCmpUGT"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createFCmpUGE), TY_Value, TY_IRBuilder, MN_("createFCmpUGE"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createFCmpULT), TY_Value, TY_IRBuilder, MN_("createFCmpULT"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createFCmpULE), TY_Value, TY_IRBuilder, MN_("createFCmpULE"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createFCmpUNE), TY_Value, TY_IRBuilder, MN_("createFCmpUNE"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_createPHI),   TY_PHINode, TY_IRBuilder, MN_("createPHI"), 2, TY_Type, FN_("ty"),TY_int, FN_("numReservedValues"),
		_Public, _F(PHINode_addIncoming),   TY_void, TY_PHINode, MN_("addIncoming"), 2, TY_Value, FN_("v"),TY_BasicBlock, FN_("bb"),
		_Public, _F(IRBuilder_createCall1), TY_Value, TY_IRBuilder, MN_("createCall1"), 2, TY_Value, FN_("callee"),TY_Value, FN_("arg"),
		_Public, _F(IRBuilder_createCall2), TY_Value, TY_IRBuilder, MN_("createCall2"), 3, TY_Value, FN_("callee"),TY_Value, FN_("arg1"),TY_Value, FN_("arg2"),
		_Public, _F(IRBuilder_createCall3), TY_Value, TY_IRBuilder, MN_("createCall3"), 4, TY_Value, FN_("callee"),TY_Value, FN_("arg1"),TY_Value, FN_("arg2"),TY_Value, FN_("arg3"),
		_Public, _F(IRBuilder_createCall4), TY_Value, TY_IRBuilder, MN_("createCall4"), 5, TY_Value, FN_("callee"),TY_Value, FN_("arg1"),TY_Value, FN_("arg2"),TY_Value, FN_("arg3"),TY_Value, FN_("arg4"),
		_Public, _F(IRBuilder_createCall5), TY_Value, TY_IRBuilder, MN_("createCall5"), 6, TY_Value, FN_("callee"),TY_Value, FN_("arg1"),TY_Value, FN_("arg2"),TY_Value, FN_("arg3"),TY_Value, FN_("arg4"),TY_Value, FN_("arg5"),
		_Public, _F(IRBuilder_createCall),  TY_Value, TY_IRBuilder, MN_("createCall"), 2, TY_Value, FN_("callee"),TY_Array_Value, FN_("args"),
		_Public, _F(IRBuilder_createSelect), TY_Value, TY_IRBuilder, MN_("createSelect"), 3, TY_Value, FN_("c"),TY_Value, FN_("trueV"),TY_Value, FN_("falseV"),
		_Public, _F(IRBuilder_createVAArg),  TY_Value, TY_IRBuilder, MN_("createVAArg"), 2, TY_Value, FN_("list"),TY_Type, FN_("ty"),
		_Public, _F(IRBuilder_createExtractElement), TY_Value, TY_IRBuilder, MN_("createExtractElement"), 2, TY_Value, FN_("vec"),TY_Value, FN_("idx"),
		_Public, _F(IRBuilder_createInsertElement),  TY_Value, TY_IRBuilder, MN_("createInsertElement"), 3, TY_Value, FN_("vec"),TY_Value, FN_("newElt"),TY_Value, FN_("idx"),
		_Public, _F(IRBuilder_createShuffleVector),  TY_Value, TY_IRBuilder, MN_("createShuffleVector"), 3, TY_Value, FN_("v1"),TY_Value, FN_("v2"),TY_Value, FN_("mask"),
		_Public, _F(IRBuilder_createIsNull),    TY_Value, TY_IRBuilder, MN_("createIsNull"), 1, TY_Value, FN_("arg"),
		_Public, _F(IRBuilder_createIsNotNull), TY_Value, TY_IRBuilder, MN_("createIsNotNull"), 1, TY_Value, FN_("arg"),
		_Public, _F(IRBuilder_createPtrDiff),   TY_Value, TY_IRBuilder, MN_("createPtrDiff"), 2, TY_Value, FN_("lhs"),TY_Value, FN_("rhs"),
		_Public, _F(IRBuilder_setInsertPoint),  TY_Value, TY_IRBuilder, MN_("setInsertPoint"), 1, TY_BasicBlock, FN_("bb"),
		_Public, _F(IRBuilder_getInsertBlock),  TY_BasicBlock, TY_IRBuilder, MN_("getInsertBlock"), 0,
		_Public, _F(BasicBlock_getParent), TY_Function, TY_BasicBlock, MN_("getParent"), 0,
		_Public, _F(BasicBlock_insertBefore), TY_void, TY_BasicBlock, MN_("insertBefore"), 2, TY_Instruction, FN_("before"),TY_Instruction, FN_("inst"),
		_Public, _F(BasicBlock_getLastInst),  TY_Value/*TODO*/, TY_BasicBlock, MN_("getLastInst"), 0,
		_Public, _F(BasicBlock_getTerminator), TY_Value/*TODO*/, TY_BasicBlock, MN_("getTerminator"), 0,
		_Public, _F(Instruction_setMetadata), TY_void, TY_Instruction, MN_("setMetadata"), 3, TY_Module, FN_("m"),TY_String, FN_("name"),TY_int, FN_("value"),
		_Public, _F(Function_dump), TY_void, TY_Function, MN_("dump"), 0,
		_Public, _F(Value_dump), TY_void, TY_Value, MN_("dump"), 0,
		_Public, _F(Type_dump), TY_void, TY_Type, MN_("dump"), 0,
		_Public, _F(BasicBlock_dump), TY_void, TY_BasicBlock, MN_("dump"), 0,
		_Public|_Static, _F(Function_create), TY_Function, TY_Function, MN_("create"), 4, TY_String, FN_("name"),TY_FunctionType, FN_("fnTy"),TY_Module, FN_("m"),TY_int, FN_("linkage"),
		_Public, _F(Function_addFnAttr), TY_void, TY_Function, MN_("addFnAttr"), 1, TY_int, FN_("attributes"),
		_Public, _F(BasicBlock_size), TY_int, TY_BasicBlock, MN_("size"), 0,
		_Public, _F(BasicBlock_empty), TY_boolean, TY_BasicBlock, MN_("empty"), 0,
		_Public, _F(Module_new), TY_Module, TY_Module, MN_("new"), 1, TY_String, FN_("name"),
		_Public, _F(Module_getTypeByName), TY_Type, TY_Module, MN_("getTypeByName"), 1, TY_String, FN_("name"),
		_Public, _F(Module_dump), TY_void, TY_Module, MN_("dump"), 0,
		_Public, _F(Module_getOrInsertFunction), TY_Function, TY_Module, MN_("getOrInsertFunction"), 2, TY_String, FN_("name"),TY_FunctionType, FN_("fnTy"),
		_Public, _F(Module_createExecutionEngine), TY_ExecutionEngine, TY_Module, MN_("createExecutionEngine"), 1, TY_int, FN_("optLevel"),
		_Public|_Static, _F(BasicBlock_create), TY_BasicBlock, TY_BasicBlock, MN_("create"), 2, TY_Function, FN_("parent"),TY_String, FN_("name"),
		_Public|_Static, _F(FunctionType_get), TY_FunctionType, TY_FunctionType, MN_("get"), 3, TY_Type, FN_("retTy"),TY_Array_Type, FN_("args"),TY_boolean, FN_("b"),
		_Public|_Static, _F(ArrayType_get),    TY_Type, TY_ArrayType, MN_("get"), 2, TY_Type, FN_("t"),TY_int, FN_("elemSize"),
		_Public|_Static, _F(StructType_get),   TY_Type, TY_StructType, MN_("get"), 2, TY_Array_Type, FN_("args"),TY_boolean, FN_("isPacked"),
		_Public|_Static, _F(StructType_create), TY_Type, TY_StructType, MN_("create"), 3, TY_Array_Type, FN_("args"),TY_String, FN_("name"),TY_boolean, FN_("isPacked"),
		_Public, _F(StructType_setBody), TY_void, TY_StructType, MN_("setBody"), 2, TY_Array_Type, FN_("args"),TY_boolean, FN_("isPacked"),
		_Public, _F(StructType_isOpaque), TY_boolean, TY_StructType, MN_("isOpaque"), 0,
		_Public, _F(ExecutionEngine_getPointerToFunction), TY_NativeFunction, TY_ExecutionEngine, MN_("getPointerToFunction"), 1, TY_Function, FN_("func"),
		_Public, _F(ExecutionEngine_addGlobalMapping), TY_void, TY_ExecutionEngine, MN_("addGlobalMapping"), 2, TY_GlobalVariable, FN_("g"),TY_int, FN_("addr"),
		_Public, _F(GlobalVariable_new), TY_Value, TY_GlobalVariable, MN_("new"), 5, TY_Module, FN_("m"),TY_Type, FN_("ty"),TY_Constant, FN_("c"),TY_int, FN_("linkage"),TY_String, FN_("name"),
#if LLVM_VERSION >= 300
		_Public, _F(PassManagerBuilder_new), TY_PassManagerBuilder, TY_PassManagerBuilder, MN_("new"), 0,
		_Public, _F(PassManagerBuilder_populateModulePassManager), TY_void, TY_PassManagerBuilder, MN_("populateModulePassManager"), 1, TY_PassManager, FN_("manager"),
#endif
		_Public, _F(PassManager_new), TY_PassManager, TY_PassManager, MN_("new"), 0,
		_Public, _F(FunctionPassManager_new), TY_FunctionPassManager, TY_FunctionPassManager, MN_("new"), 1, TY_Module, FN_("m"),
		_Public, _F(PassManager_run), TY_void, TY_PassManager, MN_("run"), 1, TY_Function, FN_("func"),
		_Public, _F(PassManager_addPass), TY_void, TY_PassManager, MN_("addPass"), 1, TY_Pass, FN_("p"),
		_Public, _F(PassManager_addImmutablePass), TY_void, TY_PassManager, MN_("addImmutablePass"), 1, TY_ImmutablePass, FN_("p"),
		_Public, _F(PassManager_addFunctionPass), TY_void, TY_PassManager, MN_("addFunctionPass"), 1, TY_FunctionPass, FN_("p"),
		_Public, _F(PassManager_addModulePass), TY_void, TY_PassManager, MN_("addModulePass"), 1, TY_ModulePass, FN_("p"),
		_Public, _F(FunctionPassManager_add), TY_void, TY_FunctionPassManager, MN_("add"), 1, TY_Pass, FN_("p"),
		_Public, _F(FunctionPassManager_run), TY_void, TY_FunctionPassManager, MN_("run"), 1, TY_Function, FN_("func"),
		_Public, _F(FunctionPassManager_doInitialization), TY_void, TY_FunctionPassManager, MN_("doInitialization"), 0,
		_Public, _F(ExecutionEngine_getTargetData), TY_String/*TODO*/, TY_ExecutionEngine, MN_("getTargetData"), 0,
		_Public, _F(Argument_new), TY_Value/*TODO*/, TY_Argument, MN_("new"), 1, TY_Type, FN_("type"),
		_Public, _F(Value_replaceAllUsesWith), TY_void, TY_Value, MN_("replaceAllUsesWith"), 1, TY_Value, FN_("v"),
		_Public, _F(Value_setName), TY_void, TY_Value, MN_("setName"), 1, TY_String, FN_("name"),
		_Public, _F(Value_getType), TY_Type, TY_Value, MN_("getType"), 0,
		_Public, _F(Function_getArguments), TY_Array_Value, TY_Function, MN_("getArguments"), 0,
		_Public, _F(Function_getReturnType), TY_Type, TY_Function, MN_("getReturnType"), 0,
		_Public, _F(LoadInst_setAlignment),  TY_void, TY_LoadInst, MN_("setAlignment"), 1, TY_int, FN_("align"),
		_Public, _F(StoreInst_setAlignment), TY_void, TY_StoreInst, MN_("setAlignment"), 1, TY_int, FN_("align"),
		_Public, _F(kMethod_setFunction), TY_void, TY_Method, MN_("setFunction"), 1, TY_NativeFunction, FN_("nf"),
		_Public|_Static, _F(ConstantInt_get),         TY_Constant, TY_ConstantInt, MN_("getValue"), 2, TY_Type, FN_("type"),TY_int, FN_("v"),
		_Public|_Static, _F(ConstantFP_get),          TY_Constant, TY_ConstantFP, MN_("getValue"), 2, TY_Type, FN_("type"),TY_float, FN_("v"),
		_Public|_Static, _F(ConstantFP_get),          TY_Constant, TY_ConstantFP, MN_("getValueFromBits"), 2, TY_Type, FN_("type"),TY_int, FN_("v"),
		_Public|_Static, _F(ConstantPointerNull_get), TY_Constant, TY_ConstantPointerNull, MN_("getValue"), 1, TY_Type, FN_("type"),
		_Public|_Static, _F(ConstantStruct_get),      TY_Constant, TY_ConstantStruct, MN_("getValue"), 2, TY_Type, FN_("type"),TY_Array_Constant, FN_("v"),
		_Public|_Static, _F(DynamicLibrary_loadLibraryPermanently),   TY_boolean, TY_DynamicLibrary, MN_("loadLibraryPermanently"), 1, TY_String, FN_("libname"),
		_Public|_Static, _F(DynamicLibrary_searchForAddressOfSymbol), TY_int, TY_DynamicLibrary, MN_("searchForAddressOfSymbol"), 1, TY_String, FN_("fname"),
		_Public|_Static, _F(LLVM_createDomPrinterPass),     TY_Pass, TY_LLVM, MN_("createDomPrinterPass"), 0,
		_Public|_Static, _F(LLVM_createDomOnlyPrinterPass), TY_Pass, TY_LLVM, MN_("createDomOnlyPrinterPass"), 0,
		_Public|_Static, _F(LLVM_createDomViewerPass),      TY_Pass, TY_LLVM, MN_("createDomViewerPass"), 0,
		_Public|_Static, _F(LLVM_createDomOnlyViewerPass),  TY_Pass, TY_LLVM, MN_("createDomOnlyViewerPass"), 0,
		_Public|_Static, _F(LLVM_createPostDomPrinterPass), TY_Pass, TY_LLVM, MN_("createPostDomPrinterPass"), 0,
		_Public|_Static, _F(LLVM_createPostDomOnlyPrinterPass),     TY_Pass, TY_LLVM, MN_("createPostDomOnlyPrinterPass"), 0,
		_Public|_Static, _F(LLVM_createPostDomViewerPass),          TY_Pass, TY_LLVM, MN_("createPostDomViewerPass"), 0,
		_Public|_Static, _F(LLVM_createPostDomOnlyViewerPass),      TY_Pass, TY_LLVM, MN_("createPostDomOnlyViewerPass"), 0,
		_Public|_Static, _F(LLVM_createGlobalsModRefPass),          TY_Pass, TY_LLVM, MN_("createGlobalsModRefPass"), 0,
		_Public|_Static, _F(LLVM_createAliasDebugger),              TY_Pass, TY_LLVM, MN_("createAliasDebugger"), 0,
		_Public|_Static, _F(LLVM_createAliasAnalysisCounterPass),   TY_Pass, TY_LLVM, MN_("createAliasAnalysisCounterPass"), 0,
		_Public|_Static, _F(LLVM_createAAEvalPass),                 TY_Pass, TY_LLVM, MN_("createAAEvalPass"), 0,
		_Public|_Static, _F(LLVM_createLibCallAliasAnalysisPass),   TY_Pass, TY_LLVM, MN_("createLibCallAliasAnalysisPass"), 1, TY_LibCallInfo, FN_("lci"),
		_Public|_Static, _F(LLVM_createScalarEvolutionAliasAnalysisPass), TY_Pass, TY_LLVM, MN_("createScalarEvolutionAliasAnalysisPass"), 0,
		_Public|_Static, _F(LLVM_createProfileLoaderPass),          TY_Pass, TY_LLVM, MN_("createProfileLoaderPass"), 0,
		_Public|_Static, _F(LLVM_createProfileEstimatorPass),       TY_Pass, TY_LLVM, MN_("createProfileEstimatorPass"), 0,
		_Public|_Static, _F(LLVM_createProfileVerifierPass),        TY_Pass, TY_LLVM, MN_("createProfileVerifierPass"), 0,
		_Public|_Static, _F(LLVM_createPathProfileLoaderPass),      TY_Pass, TY_LLVM, MN_("createPathProfileLoaderPass"), 0,
		_Public|_Static, _F(LLVM_createPathProfileVerifierPass),    TY_Pass, TY_LLVM, MN_("createPathProfileVerifierPass"), 0,
		_Public|_Static, _F(LLVM_createLazyValueInfoPass),          TY_Pass, TY_LLVM, MN_("createLazyValueInfoPass"), 0,
		_Public|_Static, _F(LLVM_createLoopDependenceAnalysisPass), TY_Pass, TY_LLVM, MN_("createLoopDependenceAnalysisPass"), 0,
		_Public|_Static, _F(LLVM_createInstCountPass),              TY_Pass, TY_LLVM, MN_("createInstCountPass"), 0,
		_Public|_Static, _F(LLVM_createDbgInfoPrinterPass),         TY_Pass, TY_LLVM, MN_("createDbgInfoPrinterPass"), 0,
		_Public|_Static, _F(LLVM_createRegionInfoPass),             TY_Pass, TY_LLVM, MN_("createRegionInfoPass"), 0,
		_Public|_Static, _F(LLVM_createModuleDebugInfoPrinterPass), TY_Pass, TY_LLVM, MN_("createModuleDebugInfoPrinterPass"), 0,
		_Public|_Static, _F(LLVM_createMemDepPrinter),              TY_Pass, TY_LLVM, MN_("createMemDepPrinter"), 0,
		_Public|_Static, _F(LLVM_createPostDomTree),                TY_Pass, TY_LLVM, MN_("createPostDomTree"), 0,
		_Public|_Static, _F(LLVM_createRegionViewerPass),           TY_Pass, TY_LLVM, MN_("createRegionViewerPass"), 0,
		_Public|_Static, _F(LLVM_createRegionOnlyViewerPass),       TY_Pass, TY_LLVM, MN_("createRegionOnlyViewerPass"), 0,
		_Public|_Static, _F(LLVM_createRegionPrinterPass),          TY_Pass, TY_LLVM, MN_("createRegionPrinterPass"), 0,
		_Public|_Static, _F(LLVM_createRegionOnlyPrinterPass),      TY_Pass, TY_LLVM, MN_("createRegionOnlyPrinterPass"), 0,
		_Public|_Static, _F(LLVM_createLintPass),                   TY_Pass, TY_LLVM, MN_("createLintPass"), 0,
		_Public|_Static, _F(LLVM_createStripSymbolsPass),           TY_Pass, TY_LLVM, MN_("createStripSymbolsPass"), 1, TY_boolean, FN_("onlyDebugInfo"),
		_Public|_Static, _F(LLVM_createStripNonDebugSymbolsPass),   TY_Pass, TY_LLVM, MN_("createStripNonDebugSymbolsPass"), 0,
		_Public|_Static, _F(LLVM_createStripDeadDebugInfoPass),     TY_Pass, TY_LLVM, MN_("createStripDeadDebugInfoPass"), 0,
		_Public|_Static, _F(LLVM_createConstantMergePass),          TY_Pass, TY_LLVM, MN_("createConstantMergePass"), 0,
		_Public|_Static, _F(LLVM_createGlobalOptimizerPass),        TY_Pass, TY_LLVM, MN_("createGlobalOptimizerPass"), 0,
		_Public|_Static, _F(LLVM_createGlobalDCEPass),              TY_Pass, TY_LLVM, MN_("createGlobalDCEPass"), 0,
		_Public|_Static, _F(LLVM_createFunctionInliningPass),       TY_Pass, TY_LLVM, MN_("createFunctionInliningPass"), 1, TY_int, FN_("threshold"),
		_Public|_Static, _F(LLVM_createAlwaysInlinerPass),          TY_Pass, TY_LLVM, MN_("createAlwaysInlinerPass"), 0,
		_Public|_Static, _F(LLVM_createPruneEHPass),                TY_Pass, TY_LLVM, MN_("createPruneEHPass"), 0,
		_Public|_Static, _F(LLVM_createInternalizePass),            TY_Pass, TY_LLVM, MN_("createInternalizePass"), 1, TY_boolean, FN_("allButMain"),
		_Public|_Static, _F(LLVM_createDeadArgEliminationPass),     TY_Pass, TY_LLVM, MN_("createDeadArgEliminationPass"), 0,
		_Public|_Static, _F(LLVM_createArgumentPromotionPass),      TY_Pass, TY_LLVM, MN_("createArgumentPromotionPass"), 1, TY_int, FN_("maxElements"),
		_Public|_Static, _F(LLVM_createIPConstantPropagationPass),  TY_Pass, TY_LLVM, MN_("createIPConstantPropagationPass"), 0,
		_Public|_Static, _F(LLVM_createIPSCCPPass),                 TY_Pass, TY_LLVM, MN_("createIPSCCPPass"), 0,
		_Public|_Static, _F(LLVM_createLoopExtractorPass),          TY_Pass, TY_LLVM, MN_("createLoopExtractorPass"), 0,
		_Public|_Static, _F(LLVM_createSingleLoopExtractorPass),    TY_Pass, TY_LLVM, MN_("createSingleLoopExtractorPass"), 0,
		_Public|_Static, _F(LLVM_createBlockExtractorPass),         TY_Pass, TY_LLVM, MN_("createBlockExtractorPass"), 0,
		_Public|_Static, _F(LLVM_createStripDeadPrototypesPass),    TY_Pass, TY_LLVM, MN_("createStripDeadPrototypesPass"), 0,
		_Public|_Static, _F(LLVM_createFunctionAttrsPass),          TY_Pass, TY_LLVM, MN_("createFunctionAttrsPass"), 0,
		_Public|_Static, _F(LLVM_createMergeFunctionsPass),         TY_Pass, TY_LLVM, MN_("createMergeFunctionsPass"), 0,
		_Public|_Static, _F(LLVM_createPartialInliningPass),        TY_Pass, TY_LLVM, MN_("createPartialInliningPass"), 0,
		_Public|_Static, _F(LLVM_createConstantPropagationPass),    TY_Pass, TY_LLVM, MN_("createConstantPropagationPass"), 0,
		_Public|_Static, _F(LLVM_createSCCPPass),                   TY_Pass, TY_LLVM, MN_("createSCCPPass"), 0,
		_Public|_Static, _F(LLVM_createDeadInstEliminationPass),    TY_Pass, TY_LLVM, MN_("createDeadInstEliminationPass"), 0,
		_Public|_Static, _F(LLVM_createDeadCodeEliminationPass),    TY_Pass, TY_LLVM, MN_("createDeadCodeEliminationPass"), 0,
		_Public|_Static, _F(LLVM_createDeadStoreEliminationPass),   TY_Pass, TY_LLVM, MN_("createDeadStoreEliminationPass"), 0,
		_Public|_Static, _F(LLVM_createAggressiveDCEPass),          TY_Pass, TY_LLVM, MN_("createAggressiveDCEPass"), 0,
		_Public|_Static, _F(LLVM_createScalarReplAggregatesPass),   TY_Pass, TY_LLVM, MN_("createScalarReplAggregatesPass"), 1, TY_int, FN_("threshold"),
		_Public|_Static, _F(LLVM_createIndVarSimplifyPass),         TY_Pass, TY_LLVM, MN_("createIndVarSimplifyPass"), 0,
		_Public|_Static, _F(LLVM_createInstructionCombiningPass),   TY_Pass, TY_LLVM, MN_("createInstructionCombiningPass"), 0,
		_Public|_Static, _F(LLVM_createLICMPass),                   TY_Pass, TY_LLVM, MN_("createLICMPass"), 0,
		_Public|_Static, _F(LLVM_createLoopUnswitchPass),           TY_Pass, TY_LLVM, MN_("createLoopUnswitchPass"), 1, TY_boolean, FN_("optimizeForSize"),
		_Public|_Static, _F(LLVM_createLoopInstSimplifyPass),       TY_Pass, TY_LLVM, MN_("createLoopInstSimplifyPass"), 0,
		_Public|_Static, _F(LLVM_createLoopUnrollPass),             TY_Pass, TY_LLVM, MN_("createLoopUnrollPass"), 3, TY_int, FN_("threshold"),TY_int, FN_("count"),TY_int, FN_("allowPartial"),
		_Public|_Static, _F(LLVM_createLoopRotatePass),             TY_Pass, TY_LLVM, MN_("createLoopRotatePass"), 0,
		_Public|_Static, _F(LLVM_createLoopIdiomPass),              TY_Pass, TY_LLVM, MN_("createLoopIdiomPass"), 0,
		_Public|_Static, _F(LLVM_createPromoteMemoryToRegisterPass), TY_Pass, TY_LLVM, MN_("createPromoteMemoryToRegisterPass"), 0,
		_Public|_Static, _F(LLVM_createDemoteRegisterToMemoryPass),  TY_Pass, TY_LLVM, MN_("createDemoteRegisterToMemoryPass"), 0,
		_Public|_Static, _F(LLVM_createReassociatePass),             TY_Pass, TY_LLVM, MN_("createReassociatePass"), 0,
		_Public|_Static, _F(LLVM_createJumpThreadingPass),           TY_Pass, TY_LLVM, MN_("createJumpThreadingPass"), 0,
		_Public|_Static, _F(LLVM_createCFGSimplificationPass),       TY_Pass, TY_LLVM, MN_("createCFGSimplificationPass"), 0,
		_Public|_Static, _F(LLVM_createBreakCriticalEdgesPass),      TY_Pass, TY_LLVM, MN_("createBreakCriticalEdgesPass"), 0,
		_Public|_Static, _F(LLVM_createLoopSimplifyPass),            TY_Pass, TY_LLVM, MN_("createLoopSimplifyPass"), 0,
		_Public|_Static, _F(LLVM_createTailCallEliminationPass),     TY_Pass, TY_LLVM, MN_("createTailCallEliminationPass"), 0,
		_Public|_Static, _F(LLVM_createLowerSwitchPass),             TY_Pass, TY_LLVM, MN_("createLowerSwitchPass"), 0,
		_Public|_Static, _F(LLVM_createBlockPlacementPass),          TY_Pass, TY_LLVM, MN_("createBlockPlacementPass"), 0,
		_Public|_Static, _F(LLVM_createLCSSAPass),                   TY_Pass, TY_LLVM, MN_("createLCSSAPass"), 0,
		_Public|_Static, _F(LLVM_createEarlyCSEPass),                TY_Pass, TY_LLVM, MN_("createEarlyCSEPass"), 0,
		_Public|_Static, _F(LLVM_createGVNPass),                     TY_Pass, TY_LLVM, MN_("createGVNPass"), 1, TY_boolean, FN_("noLoads"),
		_Public|_Static, _F(LLVM_createMemCpyOptPass),               TY_Pass, TY_LLVM, MN_("createMemCpyOptPass"), 0,
		_Public|_Static, _F(LLVM_createLoopDeletionPass),            TY_Pass, TY_LLVM, MN_("createLoopDeletionPass"), 0,
		_Public|_Static, _F(LLVM_createSimplifyLibCallsPass),        TY_Pass, TY_LLVM, MN_("createSimplifyLibCallsPass"), 0,
		_Public|_Static, _F(LLVM_createInstructionNamerPass),        TY_Pass, TY_LLVM, MN_("createInstructionNamerPass"), 0,
		_Public|_Static, _F(LLVM_createSinkingPass),                 TY_Pass, TY_LLVM, MN_("createSinkingPass"), 0,
		_Public|_Static, _F(LLVM_createLowerAtomicPass),             TY_Pass, TY_LLVM, MN_("createLowerAtomicPass"), 0,
		_Public|_Static, _F(LLVM_createCorrelatedValuePropagationPass), TY_Pass, TY_LLVM, MN_("createCorrelatedValuePropagationPass"), 0,
#if LLVM_VERSION >= 300
		_Public|_Static, _F(LLVM_createObjCARCExpandPass),   TY_Pass, TY_LLVM, MN_("createObjCARCExpandPass"), 0,
		_Public|_Static, _F(LLVM_createObjCARCContractPass), TY_Pass, TY_LLVM, MN_("createObjCARCContractPass"), 0,
		_Public|_Static, _F(LLVM_createObjCARCOptPass),      TY_Pass, TY_LLVM, MN_("createObjCARCOptPass"), 0,
		_Public|_Static, _F(LLVM_createLowerExpectIntrinsicPass), TY_Pass, TY_LLVM, MN_("createLowerExpectIntrinsicPass"), 0,
#endif
#if LLVM_VERSION >= 301
		_Public|_Static, _F(LLVM_createBBVectorizePass),     TY_Pass, TY_LLVM, MN_("createBBVectorizePass"), 0,
#endif
		_Public|_Static, _F(LLVM_createInstructionSimplifierPass),  TY_Pass, TY_LLVM, MN_("createInstructionSimplifierPass"), 0,
		_Public|_Static, _F(LLVM_createUnifyFunctionExitNodesPass), TY_Pass, TY_LLVM, MN_("createUnifyFunctionExitNodesPass"), 0,
		_Public|_Static, _F(LLVM_createTypeBasedAliasAnalysisPass), TY_Pass, TY_LLVM, MN_("createTypeBasedAliasAnalysisPass"), 0,
		_Public|_Static, _F(LLVM_createBasicAliasAnalysisPass),     TY_Pass, TY_LLVM, MN_("createBasicAliasAnalysisPass"), 0,
		_Public|_Static, _F(LLVM_createVerifierPass),               TY_Pass, TY_LLVM, MN_("createVerifierPass"), 0,
		_Public|_Static, _F(Intrinsic_getType), TY_Type, TY_intrinsic, MN_("getType"), 2, TY_int, FN_("id"),TY_Array_Type, FN_("args"),
		_Public|_Static, _F(Intrinsic_getDeclaration), TY_Function, TY_intrinsic, MN_("getDeclaration"), 3, TY_Module, FN_("m"),TY_int, FN_("id"),TY_Array_Type, FN_("args"),
		_Public|_Static, _F(LLVM_parseBitcodeFile), TY_Value, TY_LLVM, MN_("parseBitcodeFile"), 1, TY_String, FN_("bcfile"),

		_Public|_Static, _F(ConstantExpr_getAlignOf), TY_Constant, TY_ConstantExpr, MN_("getAlignOf"), 1,TY_Type, FN_("ty"),
		_Public|_Static, _F(ConstantExpr_getSizeOf), TY_Constant, TY_ConstantExpr, MN_("getSizeOf"), 1,TY_Type, FN_("ty"),
		_Public|_Static, _F(ConstantExpr_getOffsetOf), TY_Constant, TY_ConstantExpr, MN_("getOffsetOf"), 2,TY_StructType, FN_("sTy"), TY_int, FN_("fieldNo"),
		_Public|_Static, _F(ConstantExpr_getOffsetOf), TY_Constant, TY_ConstantExpr, MN_("getOffsetOf"), 2,TY_Type, FN_("ty"), TY_Constant, FN_("fieldNo"),
		_Public|_Static, _F(ConstantExpr_getNeg), TY_Constant, TY_ConstantExpr, MN_("getNeg"), 3,TY_Constant, FN_("c"), TY_boolean, FN_("hasNUW"), TY_boolean, FN_("hasNSW"),
		_Public|_Static, _F(ConstantExpr_getFNeg), TY_Constant, TY_ConstantExpr, MN_("getFNeg"), 1,TY_Constant, FN_("c"),
		_Public|_Static, _F(ConstantExpr_getNot), TY_Constant, TY_ConstantExpr, MN_("getNot"), 1,TY_Constant, FN_("c"),
		_Public|_Static, _F(ConstantExpr_getAdd), TY_Constant, TY_ConstantExpr, MN_("getAdd"), 4,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"), TY_boolean, FN_("hasNUW"), TY_boolean, FN_("hasNSW"),
		_Public|_Static, _F(ConstantExpr_getFAdd), TY_Constant, TY_ConstantExpr, MN_("getFAdd"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getSub), TY_Constant, TY_ConstantExpr, MN_("getSub"), 4,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"), TY_boolean, FN_("hasNUW"), TY_boolean, FN_("hasNSW"),
		_Public|_Static, _F(ConstantExpr_getFSub), TY_Constant, TY_ConstantExpr, MN_("getFSub"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getMul), TY_Constant, TY_ConstantExpr, MN_("getMul"), 4,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"), TY_boolean, FN_("hasNUW"), TY_boolean, FN_("hasNSW"),
		_Public|_Static, _F(ConstantExpr_getFMul), TY_Constant, TY_ConstantExpr, MN_("getFMul"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getUDiv), TY_Constant, TY_ConstantExpr, MN_("getUDiv"), 3,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"), TY_boolean, FN_("isExact"),
		_Public|_Static, _F(ConstantExpr_getSDiv), TY_Constant, TY_ConstantExpr, MN_("getSDiv"), 3,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"), TY_boolean, FN_("isExact"),
		_Public|_Static, _F(ConstantExpr_getFDiv), TY_Constant, TY_ConstantExpr, MN_("getFDiv"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getURem), TY_Constant, TY_ConstantExpr, MN_("getURem"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getSRem), TY_Constant, TY_ConstantExpr, MN_("getSRem"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getFRem), TY_Constant, TY_ConstantExpr, MN_("getFRem"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getAnd), TY_Constant, TY_ConstantExpr, MN_("getAnd"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getOr), TY_Constant, TY_ConstantExpr, MN_("getOr"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getXor), TY_Constant, TY_ConstantExpr, MN_("getXor"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getShl), TY_Constant, TY_ConstantExpr, MN_("getShl"), 4,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"), TY_boolean, FN_("hasNUW"), TY_boolean, FN_("hasNSW"),
		_Public|_Static, _F(ConstantExpr_getLShr), TY_Constant, TY_ConstantExpr, MN_("getLShr"), 3,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"), TY_boolean, FN_("isExact"),
		_Public|_Static, _F(ConstantExpr_getAShr), TY_Constant, TY_ConstantExpr, MN_("getAShr"), 3,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"), TY_boolean, FN_("isExact"),
		_Public|_Static, _F(ConstantExpr_getTrunc), TY_Constant, TY_ConstantExpr, MN_("getTrunc"), 2,TY_Constant, FN_("c"), TY_Type, FN_("ty"),
		_Public|_Static, _F(ConstantExpr_getSExt), TY_Constant, TY_ConstantExpr, MN_("getSExt"), 2,TY_Constant, FN_("c"), TY_Type, FN_("ty"),
		_Public|_Static, _F(ConstantExpr_getZExt), TY_Constant, TY_ConstantExpr, MN_("getZExt"), 2,TY_Constant, FN_("c"), TY_Type, FN_("ty"),
		_Public|_Static, _F(ConstantExpr_getFPTrunc), TY_Constant, TY_ConstantExpr, MN_("getFPTrunc"), 2,TY_Constant, FN_("c"), TY_Type, FN_("ty"),
		_Public|_Static, _F(ConstantExpr_getFPExtend), TY_Constant, TY_ConstantExpr, MN_("getFPExtend"), 2,TY_Constant, FN_("c"), TY_Type, FN_("ty"),
		_Public|_Static, _F(ConstantExpr_getUIToFP), TY_Constant, TY_ConstantExpr, MN_("getUIToFP"), 2,TY_Constant, FN_("c"), TY_Type, FN_("ty"),
		_Public|_Static, _F(ConstantExpr_getSIToFP), TY_Constant, TY_ConstantExpr, MN_("getSIToFP"), 2,TY_Constant, FN_("c"), TY_Type, FN_("ty"),
		_Public|_Static, _F(ConstantExpr_getFPToUI), TY_Constant, TY_ConstantExpr, MN_("getFPToUI"), 2,TY_Constant, FN_("c"), TY_Type, FN_("ty"),
		_Public|_Static, _F(ConstantExpr_getFPToSI), TY_Constant, TY_ConstantExpr, MN_("getFPToSI"), 2,TY_Constant, FN_("c"), TY_Type, FN_("ty"),
		_Public|_Static, _F(ConstantExpr_getPtrToInt), TY_Constant, TY_ConstantExpr, MN_("getPtrToInt"), 2,TY_Constant, FN_("c"), TY_Type, FN_("ty"),
		_Public|_Static, _F(ConstantExpr_getIntToPtr), TY_Constant, TY_ConstantExpr, MN_("getIntToPtr"), 2,TY_Constant, FN_("c"), TY_Type, FN_("ty"),
		_Public|_Static, _F(ConstantExpr_getBitCast), TY_Constant, TY_ConstantExpr, MN_("getBitCast"), 2,TY_Constant, FN_("c"), TY_Type, FN_("ty"),
		_Public|_Static, _F(ConstantExpr_getNSWNeg), TY_Constant, TY_ConstantExpr, MN_("getNSWNeg"), 1,TY_Constant, FN_("c"),
		_Public|_Static, _F(ConstantExpr_getNUWNeg), TY_Constant, TY_ConstantExpr, MN_("getNUWNeg"), 1,TY_Constant, FN_("c"),
		_Public|_Static, _F(ConstantExpr_getNSWAdd), TY_Constant, TY_ConstantExpr, MN_("getNSWAdd"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getNUWAdd), TY_Constant, TY_ConstantExpr, MN_("getNUWAdd"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getNSWSub), TY_Constant, TY_ConstantExpr, MN_("getNSWSub"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getNUWSub), TY_Constant, TY_ConstantExpr, MN_("getNUWSub"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getNSWMul), TY_Constant, TY_ConstantExpr, MN_("getNSWMul"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getNUWMul), TY_Constant, TY_ConstantExpr, MN_("getNUWMul"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getNSWShl), TY_Constant, TY_ConstantExpr, MN_("getNSWShl"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getNUWShl), TY_Constant, TY_ConstantExpr, MN_("getNUWShl"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getExactSDiv), TY_Constant, TY_ConstantExpr, MN_("getExactSDiv"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getExactUDiv), TY_Constant, TY_ConstantExpr, MN_("getExactUDiv"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getExactAShr), TY_Constant, TY_ConstantExpr, MN_("getExactAShr"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getExactLShr), TY_Constant, TY_ConstantExpr, MN_("getExactLShr"), 2,TY_Constant, FN_("c1"), TY_Constant, FN_("c2"),
		_Public|_Static, _F(ConstantExpr_getZExtOrBitCast), TY_Constant, TY_ConstantExpr, MN_("getZExtOrBitCast"), 2,TY_Constant, FN_("c"), TY_Type, FN_("ty"),
		_Public|_Static, _F(ConstantExpr_getSExtOrBitCast), TY_Constant, TY_ConstantExpr, MN_("getSExtOrBitCast"), 2,TY_Constant, FN_("c"), TY_Type, FN_("ty"),
		_Public|_Static, _F(ConstantExpr_getTruncOrBitCast), TY_Constant, TY_ConstantExpr, MN_("getTruncOrBitCast"), 2,TY_Constant, FN_("c"), TY_Type, FN_("ty"),
		_Public|_Static, _F(ConstantExpr_getPointerCast), TY_Constant, TY_ConstantExpr, MN_("getPointerCast"), 2,TY_Constant, FN_("c"), TY_Type, FN_("ty"),
		_Public|_Static, _F(ConstantExpr_getIntegerCast), TY_Constant, TY_ConstantExpr, MN_("getIntegerCast"), 3,TY_Constant, FN_("c"), TY_Type, FN_("ty"), TY_boolean, FN_("isSigned"),
		_Public|_Static, _F(ConstantExpr_getFPCast), TY_Constant, TY_ConstantExpr, MN_("getFPCast"), 2,TY_Constant, FN_("c"), TY_Type, FN_("ty"),
		_Public|_Static, _F(ConstantExpr_getSelect), TY_Constant, TY_ConstantExpr, MN_("getSelect"), 3,TY_Constant, FN_("c"), TY_Constant, FN_("v1"), TY_Constant, FN_("v2"),
		_Public|_Static, _F(ConstantExpr_getElementPtr0), TY_Constant, TY_ConstantExpr, MN_("getElementPtr"), 3,TY_Constant, FN_("c"), TY_Constant, FN_("idx"), TY_boolean, FN_("InBounds"),
		_Public|_Static, _F(ConstantExpr_getElementPtr), TY_Constant, TY_ConstantExpr, MN_("getElementPtr"), 3,TY_Constant, FN_("c"), TY_Array_Value, FN_("IdxList"), TY_boolean, FN_("InBounds"),
		_Public|_Static, _F(ConstantExpr_getInBoundsGetElementPtr0), TY_Constant, TY_ConstantExpr, MN_("getInBoundsGetElementPtr0"), 2,TY_Constant, FN_("c"), TY_Constant, FN_("idx"),
		_Public|_Static, _F(ConstantExpr_getInBoundsGetElementPtr), TY_Constant, TY_ConstantExpr, MN_("getInBoundsGetElementPtr"), 2,TY_Constant, FN_("c"), TY_Array_Value, FN_("idxList"),
		_Public|_Static, _F(ConstantExpr_getExtractElement), TY_Constant, TY_ConstantExpr, MN_("getExtractElement"), 2,TY_Constant, FN_("vec"), TY_Constant, FN_("idx"),
		_Public|_Static, _F(ConstantExpr_getInsertElement), TY_Constant, TY_ConstantExpr, MN_("getInsertElement"), 3,TY_Constant, FN_("vec"), TY_Constant, FN_("elt"), TY_Constant, FN_("idx"),
		_Public|_Static, _F(ConstantExpr_getShuffleVector), TY_Constant, TY_ConstantExpr, MN_("getShuffleVector"), 3,TY_Constant, FN_("v1"), TY_Constant, FN_("v2"), TY_Constant, FN_("mask"),
		_Public|_Static, _F(ConstantExpr_getExtractValue), TY_Constant, TY_ConstantExpr, MN_("getExtractValue"), 2,TY_Constant, FN_("Agg"), TY_Array_Int, FN_("idxs"),
		_Public|_Static, _F(ConstantExpr_getInsertValue), TY_Constant, TY_ConstantExpr, MN_("getInsertValue"), 3,TY_Constant, FN_("Agg"), TY_Constant, FN_("val"), TY_Array_Int, FN_("idxs"),
		_Public, _F(Type_opEQ), TY_boolean, TY_Type, MN_("=="), 1,TY_Type, FN_("t"),
		//FIXME
	//_Public|_Const|_Im|_Coercion, _F(Float_toInt), TY_int, TY_float, MN_to(TY_int), 0,
		_Public|_Const|_Coercion|_Im, _F(Object_toValue), TY_Value, TY_Object, MN_to(TY_Value), 0,
		_Public|_Const|_Coercion|_Im, _F(Object_toType),  TY_Type,  TY_Object, MN_to(TY_Type), 0,
		_Public|_Const|_Coercion|_Im, _F(Object_toModule), TY_Module,  TY_Object, MN_to(TY_Module), 0,
		_Public|_Const|_Coercion|_Im, _F(Object_toExecutionEngine),  TY_ExecutionEngine,  TY_Object, MN_to(TY_ExecutionEngine), 0,
		_Public|_Const|_Coercion|_Im, _F(Object_asFunction), TY_Function, TY_Object, MN_to(TY_Function), 0,
		_Public|_Const|_Coercion|_Im, _F(Object_toIRBuilder), TY_IRBuilder, TY_Object, MN_to(TY_IRBuilder), 0,
		_Public|_Const|_Coercion|_Im, _F(Object_asFunctionType), TY_FunctionType, TY_Object, MN_to(TY_FunctionType), 0,
		_Public|_Const|_Coercion|_Im, _F(Object_toLLVMBasicBlock), TY_BasicBlock, TY_Object, MN_to(TY_BasicBlock), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, NULL, methoddata);
	KLIB kNameSpace_loadConstData(kctx, ns, (const char **)IntAttributes, 0);
	KLIB kNameSpace_loadConstData(kctx, ns, (const char **)IntIntrinsic, 0);
	KLIB kNameSpace_loadConstData(kctx, ns, (const char **)IntGlobalVariable, 0);

	return true;
}

static kbool_t llvm_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	(void)kctx;(void)ns;(void)pline;
	return true;
}

static kbool_t llvm_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	(void)kctx;(void)ns;(void)pline;
	return true;
}

static kbool_t llvm_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	(void)kctx;(void)ns;(void)pline;
	return true;
}

KDEFINE_PACKAGE* llvm_init(void)
{
	InitializeNativeTarget();
	static KDEFINE_PACKAGE d = {
		K_CHECKSUM,
		"llvm", "3.0", "", "", "",
		llvm_initPackage,
		llvm_setupPackage,
		llvm_initNameSpace,
		llvm_setupNameSpace,
		K_REVISION
	};

	return &d;
}

#ifdef __cplusplus
}
#endif
