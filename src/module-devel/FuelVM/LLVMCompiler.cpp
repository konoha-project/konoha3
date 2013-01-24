#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/IRBuilder.h>
#include <llvm/Intrinsics.h>
#include <llvm/DataLayout.h>
#include <llvm/Pass.h>
#include <llvm/PassManager.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/ExecutionEngine/JITMemoryManager.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <vector>
#include <map>
#include <string>
#include <iostream>

#include <stdio.h>
#include "codegen.h"
#include "visitor.h"
#include "FuelVM.h"
#include "LLVMType.h"

//#define USE_LLVMIR_DUMP 1
#include "KonohaRuntimeLib.h"

using namespace llvm;

namespace FuelVM {

template<typename T1, typename T2>
struct Pair {
private:
	T1 first_;
	T2 second_;
public:
	Pair() : first_(0), second_(0) {}
	Pair(T1 first, T2 second) : first_(first), second_(second) {}
	T1 first()  const { return first_; }
	T2 second() const { return second_; }
};

typedef Pair<Value *, bool> ValueInfo;

typedef struct LLVMIRBuilder {
	Visitor visitor;
	KonohaContext *kctx;
	IRBuilder<> *builder;
	Function *Func;
	Block    *Current;
	Value    *Vsfp;
	Value    *VsfpRef;
	Value    *Vstack_top;
	FuelIRBuilder *FuelBuilder;
	std::vector<ValueInfo> *Variables;
} LLVMIRBuilder;

static Module *GlobalModule = 0;
static ExecutionEngine *GlobalEngine = 0;
static std::map<kMethod *, Function *> CompiledCode;

template<typename T>
static void *void_cast(T *ptr) {
	union { void *ptr; T *val; } Val;
	Val.val = ptr;
	return Val.ptr;
}

static void InitLLVM()
{
	if(GlobalModule)
		return;
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	GlobalModule = new Module("LLVM", getGlobalContext());
	GlobalModule->setTargetTriple(LLVM_HOSTTRIPLE);
	std::string Error;
	GlobalEngine = EngineBuilder(GlobalModule)
		.setEngineKind(EngineKind::JIT)
		//.setUseMCJIT(true)
		.setJITMemoryManager(JITMemoryManager::CreateDefaultMemManager())
		.setErrorStr(&Error).create();

	if(Error != "") {
		fprintf(stderr, "[%s:%d] %s", __FILE__, __LINE__, Error.c_str());
		abort();
	}
	if(sys::DynamicLibrary::LoadLibraryPermanently("FuelVM" K_OSDLLEXT, &Error)) {
		fprintf(stderr, "%s\n", Error.c_str());
	}
	/*XXX LLVM need to dlopen libm for executing math intrinsics */
	if(sys::DynamicLibrary::LoadLibraryPermanently("libm" K_OSDLLEXT, &Error)) {
		fprintf(stderr, "%s\n", Error.c_str());
	}
	LLVMType_Init();
}

extern "C" void ExitLLVM()
{
	if(GlobalModule == 0)
		return;
	GlobalModule = 0;
	delete GlobalEngine;
	llvm_shutdown();
}

struct IntrinsicInfo {
	const char *name;
	unsigned len;
	unsigned ParamSize;
};

static IntrinsicInfo MathIntrinsic[] = {
#define DEFINE_INTRINSIC(IDX, X, N) {#X, sizeof(#X)-1, N}
	DEFINE_INTRINSIC( 0, cos, 1),
	DEFINE_INTRINSIC( 1, exp, 1),
	DEFINE_INTRINSIC( 2, exp2, 1),
	DEFINE_INTRINSIC( 3, fabs, 1),
	DEFINE_INTRINSIC( 4, floor, 1),
	DEFINE_INTRINSIC( 5, log, 1),
	DEFINE_INTRINSIC( 6, log10, 1),
	DEFINE_INTRINSIC( 7, log2, 1),
	DEFINE_INTRINSIC( 8, sqrt, 1),
	DEFINE_INTRINSIC( 9, pow, 2),
	DEFINE_INTRINSIC(10, powi, 2)
};

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
static Function *CreateMathIntrinsic(unsigned Idx)
{
	std::vector<Type*> List;
	Type *FloatTy = Type::getDoubleTy(getGlobalContext());
	Type *IntTy   = ToLLVMType(TYPE_int);
	List.push_back(FloatTy);
#define CASE_INTRINSIC(N, X, P) case N:\
	return Intrinsic::getDeclaration(GlobalModule, Intrinsic::X, List)
	switch(Idx) {
		CASE_INTRINSIC( 0, cos, 1);
		CASE_INTRINSIC( 1, exp, 1);
		CASE_INTRINSIC( 2, exp2, 1);
		CASE_INTRINSIC( 3, fabs, 1);
		CASE_INTRINSIC( 4, floor, 1);
		CASE_INTRINSIC( 5, log, 1);
		CASE_INTRINSIC( 6, log10, 1);
		CASE_INTRINSIC( 7, log2, 1);
		CASE_INTRINSIC( 8, sqrt, 1);
		CASE_INTRINSIC( 9, pow, 2);
		case 10:
			List.push_back(IntTy);
			return Intrinsic::getDeclaration(GlobalModule, Intrinsic::powi, List);
		default:
			break;
	}
	return 0;
}

static Value *GetValue(LLVMIRBuilder *writer, INode *Node);
static void SetValue(LLVMIRBuilder *writer, INode *Node, Value *Val, bool Undef);

static bool EmitMathAPI(LLVMIRBuilder *writer, ICall *Inst, kMethod *mtd, std::vector<Value *> &List)
{
	KonohaContext *kctx = writer->FuelBuilder->Context;

	if(KClass_(mtd->typeId) != writer->FuelBuilder->ClassInfo.cMath)
		return false;
#define METHOD_NAME(INFO) KLIB Ksymbol(kctx, (INFO).name, (INFO).len, StringPolicy_TEXT|StringPolicy_ASCII, _NEWID)
	for(unsigned i = 0; i < ARRAY_SIZE(MathIntrinsic); i++) {
		if(mtd->mn == METHOD_NAME(MathIntrinsic[i]) &&
				ARRAY_size(Inst->Params) == MathIntrinsic[i].ParamSize + 2) {
			Value *Val;
			Function *F = CreateMathIntrinsic(i);
			if(MathIntrinsic[i].ParamSize == 1) {
				Val = writer->builder->CreateCall(F, List[1]);
			} else {
				Val = writer->builder->CreateCall2(F, List[1], List[2]);
			}
			SetValue(writer, ToINode(Inst), Val, false);
			return true;
		}
	}
	return false;
}

static Value *GetFieldRef(LLVMIRBuilder *writer, IField *Inst);
static Value *GetArrayRef(LLVMIRBuilder *writer, ICall *Inst, std::vector<Value *> &List);
static void EmitWriteBarrier(LLVMIRBuilder *writer, Value *Parent, Value *OldRef, Value *New);

static bool IsCharSequence(kMethod *mtd)
{
	return mtd->typeId == KType_Array || mtd->typeId == KType_String;
}

static bool EmitKonohaAPI(LLVMIRBuilder *writer, ICall *Inst, kMethod *mtd, std::vector<Value *> &List)
{
	IRBuilder<> *builder = writer->builder;
	KonohaContext *kctx = writer->FuelBuilder->Context;

	IField Node;
	Node.base.Type = Inst->base.Type;
	Node.Node = *(ARRAY_n(Inst->Params, 1));
	Node.FieldIndex = mtd->delta;

	/* Getter/Setter */
	if(mtd->typeId == KType_Array && mtd->mn == KMethodName_("get")) {
		Value *Ref = GetArrayRef(writer, Inst, List);
		Value *Val = builder->CreateLoad(Ref);
		SetValue(writer, ToINode(Inst), Val, false);
		return true;
	}
	if(mtd->typeId == KType_Array && mtd->mn == KMethodName_("set")) {
		Value *Ref = GetArrayRef(writer, Inst, List);
		Value *Val = List[2];
		EmitWriteBarrier(writer, List[0], Ref, Val);
		builder->CreateStore(Val, Ref);
		return true;
	}
	if(IsCharSequence(mtd) && mtd->mn == KMethodName_("getSize")) {
		Value *Val = GetValue(writer, Node.Node);
		Val = builder->CreateBitCast(Val, ToType(ID_kCharSequence));
		Val = builder->CreateStructGEP(Val, kCharSequence_bytesize);
		Val = builder->CreateLoad(Val);
		if(mtd->typeId == KType_Array) {
			Val = builder->CreateSDiv(Val, builder->getInt64(sizeof(void *)));
		}
		SetValue(writer, ToINode(Inst), Val, false);
		return true;
	}
	if(!KMethodName_IsGetter(mtd->mn) && !KMethodName_IsSetter(mtd->mn)) {
		return false;
	}

	KClass *kclass = KClass_(ToKType(kctx, Node.Node->Type));
	ktypeattr_t type = ToKType(kctx, Inst->base.Type);
	Value *Ref = NULL;

	if(Node.FieldIndex >= kclass->fieldsize) {
		/* case prototype */
		return false;
	}
	for(size_t i = 0; i < kclass->fieldsize; i++) {
		if(type == kclass->fieldItems[i].attrTypeId && i == Node.FieldIndex) {
			Ref = GetFieldRef(writer, &Node);
			break;
		}
	}
	if(Ref != NULL) {
		if(KMethodName_IsGetter(mtd->mn)) {
			Value *Val = builder->CreateLoad(Ref);
			SetValue(writer, ToINode(Inst), Val, false);
		}
		else if(KMethodName_IsSetter(mtd->mn)) {
			INode *RHS = *(ARRAY_n(Inst->Params, 2));
			Value *Val = GetValue(writer, RHS);
			EmitWriteBarrier(writer, List[0], Ref, Val);
			builder->CreateStore(Val, Ref);
		}
		return true;
	}
	return false;
}

static const char *ConstructMethodName(KonohaContext *kctx, kMethod *mtd, KBuffer *wb, const char *suffix)
{
	KClass *kclass = KClass_(mtd->typeId);
	KLIB KBuffer_printf(kctx, wb, "%s_%s%s%s",
			KClass_text(kclass), KMethodName_Fmt2(mtd->mn), suffix);
	return KLIB KBuffer_text(kctx, wb, EnsureZero);
}

static Value *EmitConstant(IRBuilder<> *builder, bool bval)
{
	if(sizeof(void *) == 4)
		return builder->getInt32((int)bval);
	else
		return builder->getInt64((int)bval);
}

static Value *EmitConstant(IRBuilder<> *builder, int ival)
{
	return builder->getInt32(ival);
}

static Value *EmitConstant(IRBuilder<> *builder, int64_t ival)
{
	return builder->getInt64(ival);
}

static Value *EmitConstant(IRBuilder<> *builder, uint64_t ival)
{
	return builder->getInt64(ival);
}

static Value *EmitConstant(IRBuilder<> *builder, double fval)
{
	Type *Ty = builder->getDoubleTy();
	return ConstantFP::get(Ty, fval);
}

static Value *EmitConstant(IRBuilder<> *builder, void *ptr)
{
	PointerType *Ty = builder->getInt8PtrTy();
	Constant *C = (sizeof(void *) == 4) ?
		builder->getInt32((uint32_t) (uintptr_t) ptr):
		builder->getInt64((uint64_t) (uintptr_t) ptr);
	return ConstantExpr::getIntToPtr(C, Ty);
}

static Value *EmitGlobalVariable(Type *Ty, const char *Name, void *addr)
{
	GlobalVariable *G;
	if((G = GlobalModule->getNamedGlobal(Name)) != 0) {
		void *oldAddr = GlobalEngine->getPointerToGlobal(G);
		if(oldAddr == addr)
			return G;
	}
	G = new GlobalVariable(*GlobalModule, Ty, true, GlobalValue::ExternalLinkage, NULL, Name);
	GlobalEngine->addGlobalMapping(G, addr);
	return G;
}

static Value *EmitGlobalVariable(KonohaContext *kctx, Type *Ty, kMethod *mtd)
{
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	const char *name = ConstructMethodName(kctx, mtd, &wb, "_Native");
	void *addr = void_cast(mtd->invokeKMethodFunc);
	sys::DynamicLibrary::AddSymbol(name, addr);
	Value *G = EmitGlobalVariable(Ty, name, addr);
	KLIB KBuffer_Free(&wb);
	return G;
}

static Value *EmitConstant(IRBuilder<> *builder, kObject *obj)
{
	Constant *V = static_cast<Constant*>(EmitConstant(builder, (void *) obj));
	return builder->CreateBitCast(V, ToType(ID_PtrkObjectVar));
}

static Value *EmitConstant(IRBuilder<> *builder, kMethod *mtd)
{
	Constant *V = static_cast<Constant*>(EmitConstant(builder, (void *) mtd));
	return builder->CreateBitCast(V, ToType(ID_PtrkMethodVar));
}

static ValueInfo &GetValueInfoFromParent(LLVMIRBuilder *writer, INode *Node)
{
	return writer->Variables->at(Node->Id);
}

static Value *GetValue(LLVMIRBuilder *writer, INode *Node)
{
	ValueInfo &Info = GetValueInfoFromParent(writer, Node);
	assert(Info.first() != 0);
	return Info.first();
}

static void SetValue(LLVMIRBuilder *writer, INode *Node, Value *Val, bool IsUndef = false)
{
	if(Node->Unused == 1) {
		ValueInfo &OldInfo = GetValueInfoFromParent(writer, Node);
		Value *OldVal = OldInfo.first();
		if(OldVal && OldInfo.second()) {
			OldVal->replaceAllUsesWith(Val);
			if(Argument *A = cast<Argument>(OldVal)) {
				delete A;
			}
		}
		Node->Unused = 0;
	}
	std::vector<ValueInfo> *table = writer->Variables;
	ValueInfo Info(Val, IsUndef);
	(*table)[Node->Id] = Info;
}

static BasicBlock *GetBlock(LLVMIRBuilder *writer, Block *Target)
{
	ValueInfo &Info = GetValueInfoFromParent(writer, (INode *)Target);
	assert(Info.second() == false);
	return (BasicBlock *) Info.first();
}

static void SetBlock(LLVMIRBuilder *writer, Block *Target, BasicBlock *BB)
{
	SetValue(writer, (INode *)Target, (Value *) BB);
}

static void SetConstant(LLVMIRBuilder *writer, INode *Node, SValue Val)
{
	enum TypeId Type = Node->Type;

	IRBuilder<> *builder = writer->builder;
	switch(Type) {
		case TYPE_void:
			assert(0 && "FIXME");
		case TYPE_boolean:
			SetValue(writer, Node, EmitConstant(builder, Val.bval)); break;
		case TYPE_int:
			SetValue(writer, Node, EmitConstant(builder, Val.ival)); break;
		case TYPE_float:
			SetValue(writer, Node, EmitConstant(builder, Val.fval)); break;
		default:
			SetValue(writer, Node, EmitConstant(builder, (kObject *)Val.ptr));
			break;
	}
}

static void SetArgument(LLVMIRBuilder *writer, INode *Node, unsigned Index)
{
	Function *F = writer->Func;
	Function::arg_iterator args = F->arg_begin();
	args++; /* Skip Context */
	for(unsigned i = 0; i < Index; i++) {
		args++;
	}
	Value *Val = args;
	SetValue(writer, Node, Val);
}

static Value *GetContext(LLVMIRBuilder *writer)
{
	Function *F = writer->Func;
	return F->arg_begin();
}

static Value *GetVsfpRef(LLVMIRBuilder *writer)
{
	Value *VsfpRef = writer->VsfpRef;
	if(VsfpRef == 0) {
		IRBuilder<> *builder = writer->builder;
		Value *Vctx = GetContext(writer);
		VsfpRef = builder->CreateStructGEP(Vctx, KonohaContextVar_esp);
		writer->VsfpRef = VsfpRef;
	}
	return VsfpRef;
}

static Value *GetStackTop(LLVMIRBuilder *writer)
{
	Value *Vsfp = writer->Vsfp;
	if(Vsfp == 0) {
		IRBuilder<> *builder = writer->builder;
		Value *VsfpRef = GetVsfpRef(writer);
		Vsfp = builder->CreateLoad(VsfpRef);
		writer->Vsfp = Vsfp;
	}
	return Vsfp;
}

static void RestoreStackTop(LLVMIRBuilder *writer, Value *newVal)
{
	IRBuilder<> *builder = writer->builder;
	Value *VsfpRef = GetVsfpRef(writer);
	builder->CreateStore(newVal, VsfpRef, false);
}

static Value *ShiftStack(LLVMIRBuilder *writer, Value *Vsfp, unsigned Shift)
{
	IRBuilder<> *builder = writer->builder;
	return builder->CreateConstInBoundsGEP1_32(Vsfp, Shift);
}

static Value *PrepareCallStack(LLVMIRBuilder *writer, Value *Vsfp)
{
	Value *Vstack_top = writer->Vstack_top;
	if(Vstack_top == 0) {
		writer->Vstack_top = ShiftStack(writer, Vsfp, K_CALLDELTA);
		Vstack_top = writer->Vstack_top;
	}
	return Vstack_top;
}

static Value *PrepareCallStack(LLVMIRBuilder *writer, Value *Vsfp, unsigned ParamSize)
{
	Value *Vstack_top = PrepareCallStack(writer, Vsfp);
	Vstack_top = ShiftStack(writer, Vstack_top, ParamSize);
	return Vstack_top;
}

static void StoreValueToStack(IRBuilder<> *builder, enum TypeId Ty, Type *ReqTy, int Idx, Value *Vsfp, Value *V, const char *Name = "")
{
	int fieldIdx = IsUnBoxedType(Ty) ? KonohaValueVar_unboxValue : KonohaValueVar_asObject;

	Value *Dst = builder->CreateConstInBoundsGEP2_32(Vsfp, Idx, fieldIdx, Name);
	if(V->getType() == builder->getInt1Ty()) {
		V = builder->CreateZExt(V, ToType(ID_long));
	} else if(V->getType() == builder->getDoubleTy()) {
		Type *Ty = PointerType::get(builder->getDoubleTy(), 0);
		Dst = builder->CreateBitCast(Dst, Ty);
	}
	builder->CreateStore(V, Dst, false);
}

static Value *LoadValueFromStack(IRBuilder<> *builder, enum TypeId Ty, Type *ReqTy, int Idx, Value *Vsfp)
{
	int fieldIdx = IsUnBoxedType(Ty) ? KonohaValueVar_unboxValue : KonohaValueVar_asObject;
	Value *Src = builder->CreateConstInBoundsGEP2_32(Vsfp, Idx, fieldIdx);
	if(ReqTy == builder->getDoubleTy()) {
		Type *Ty = PointerType::get(builder->getDoubleTy(), 0);
		Src = builder->CreateBitCast(Src, Ty);
	}
	return builder->CreateLoad(Src);
}

static void EmitCall(LLVMIRBuilder *writer, ICall *Inst, IConstant *Mtd, std::vector<Value *> &List)
{
	KonohaContext *kctx = writer->kctx;
	IRBuilder<> *builder = writer->builder;
	kMethod *method = (kMethod *) Mtd->Value.ptr;

	if(EmitMathAPI(writer, Inst, method, List)) {
		return;
	}

	if(EmitKonohaAPI(writer, Inst, method, List)) {
		return;
	}

	Value *Vctx = GetContext(writer);

	if(Inst->Op != VirtualCall) {
		if(Function *F = CompiledCode[method]) {
			List.insert(List.begin(), Vctx);
			Value *Ret = builder->CreateCall(F, List);
			if(Inst->base.Type != TYPE_void)
				SetValue(writer, ToINode(Inst), Ret);
			return;
		}
	}

	Value *MtdPtr = EmitConstant(builder, method);
	Value *Vsfp = GetStackTop(writer);
	Value *Vtop = PrepareCallStack(writer, Vsfp);

	/* stack_top[-4].uline */
	uint64_t uline = Inst->uline;
	StoreValueToStack(builder, TYPE_int, getLongTy(), K_RTNIDX, Vtop,
			EmitConstant(builder, uline), "uline");

	/* stack_top[-4].defaultObject */
	KClass *NullType = KClass_(ToKType(kctx, ToUnBoxType(Inst->base.Type)));
	kObject *DefObj  = KLIB Knull(kctx, NullType);
	Value *DefObjPtr = EmitConstant(builder, DefObj);
	StoreValueToStack(builder, TYPE_Object, ToLLVMType(TYPE_Object),
			K_RTNIDX, Vtop, DefObjPtr, "default");

	if(Inst->Op == VirtualCall) {
		/* stack_top[-2].asNameSpace */
		kNameSpace *ns = writer->FuelBuilder->Method->ns;
		Value *NSPtr = EmitConstant(builder, UPCAST(ns));
		StoreValueToStack(builder, TYPE_Object, NSPtr->getType(),
				K_NSIDX, Vtop, NSPtr, "NS");
		GlobalVariable *G;
		if((G = GlobalModule->getNamedGlobal("FuelVM_LookupMethod")) == 0) {
			std::vector<Type *> Fields;
			Type *MethodTy = ToType(ID_PtrkMethodVar);
			Type *ObjectTy = ToType(ID_PtrkObjectVar);
			Fields.push_back(ToType(ID_PtrKonohaContextVar));
			Fields.push_back(ObjectTy);
			Fields.push_back(MethodTy);
			Fields.push_back(ObjectTy);
			FunctionType *FnTy = FunctionType::get(MethodTy, Fields, false);
			G = new GlobalVariable(*GlobalModule, FnTy, true,
					GlobalValue::ExternalLinkage, NULL, "FuelVM_LookupMethod");
			void *ptr = void_cast(FuelVM_LookupMethod);
			GlobalEngine->addGlobalMapping(G, ptr);
		}
		MtdPtr = builder->CreateCall4(G, Vctx, List[0], MtdPtr, NSPtr);
	}

	/* stack_top[-2].calledMethod */
	StoreValueToStack(builder, TYPE_int, getLongTy(), K_MTDIDX, Vtop,
			builder->CreatePtrToInt(MtdPtr, ToType(ID_long), "Method"));


	/* stack_top[0..List.size()] */
	unsigned i;
	INodePtr *x;
	enum TypeId Ty;
	FOR_EACH_ARRAY__(Inst->Params, x, i, 1) {
		Ty = (*x)->Type;
		Value *V = List[i-1];
		StoreValueToStack(builder, Ty, V->getType(), i-1, Vtop, V);
	}

	Value *FuncPtr;
	if(Inst->Op != VirtualCall && method->SourceToken->text == 0) {
		/* method is maybe 'Native' Method. */
		Type *FnTy = ToType(ID_KMethodFunc);
		FuncPtr = EmitGlobalVariable(kctx, FnTy, method);
	} else {
		FuncPtr = builder->CreateStructGEP(MtdPtr, kMethodVar_invokeKMethodFunc);
		FuncPtr = builder->CreateLoad(FuncPtr);
	}

	RestoreStackTop(writer, PrepareCallStack(writer, Vsfp, List.size()));

	builder->CreateCall2(FuncPtr, Vctx, Vtop);

	Ty = Inst->base.Type;
	if(Ty != TYPE_void) {
		Value *Ret = LoadValueFromStack(builder, Ty, ToLLVMType(Ty), K_RTNIDX, Vtop);
		SetValue(writer, ToINode(Inst), Ret);
	}
	RestoreStackTop(writer, Vsfp);
}

static void EmitRet(LLVMIRBuilder *writer, INode *Node)
{
	IRBuilder<> *builder = writer->builder;
	if(Node == 0) {
		builder->CreateRetVoid();
		return;
	}
	Value *Ret = GetValue(writer, Node);
	Type *RetTy = Ret->getType();
	if(!RetTy->isPointerTy()) {
		if(RetTy == builder->getInt1Ty()) {
			Ret = builder->CreateZExt(Ret, ToType(ID_long));
		}
	}
	builder->CreateRet(Ret);
}

static void EmitJump(LLVMIRBuilder *writer, IJump *Node)
{
	IRBuilder<> *builder = writer->builder;
	BasicBlock *Target = GetBlock(writer, Node->TargetBlock);
	builder->CreateBr(Target);
}

static Value *ToBoolValue(IRBuilder<> *builder, Value *Val)
{
	if(Val->getType() != builder->getInt1Ty()) {
		Val = builder->CreateTrunc(Val, builder->getInt1Ty());
	}
	return Val;
}
static Value *ToIntValueIfBool(IRBuilder<> *builder, Value *Val)
{
	if(Val->getType() == builder->getInt1Ty()) {
		Val = builder->CreateZExt(Val, ToType(ID_long));
	}
	return Val;
}
static Value *GetBoolValue(LLVMIRBuilder *writer, INode *Node)
{
	return ToBoolValue(writer->builder, GetValue(writer, Node));
}

static void EmitBranch(LLVMIRBuilder *writer, IBranch *Node)
{
	IRBuilder<> *builder = writer->builder;
	Value *Val = GetBoolValue(writer, Node->Cond);
	BasicBlock *ThenBB = GetBlock(writer, Node->ThenBB);
	BasicBlock *ElseBB = GetBlock(writer, Node->ElseBB);
	builder->CreateCondBr(Val, ThenBB, ElseBB);
}

static GlobalVariable *GetNewKObject()
{
	GlobalVariable *G;
	if((G = GlobalModule->getNamedGlobal("FuelVM_new_kObject")) == 0) {
		std::vector<Type *> Fields;
		Fields.push_back(ToType(ID_PtrKonohaContextVar));
		Fields.push_back(Type::getInt64Ty(LLVM_CONTEXT()));
		Fields.push_back(Type::getInt8PtrTy(LLVM_CONTEXT()));
		Fields.push_back(Type::getInt64Ty(LLVM_CONTEXT()));
		FunctionType *FnTy = FunctionType::get(ToType(ID_PtrkObjectVar), Fields, false);
		G = new GlobalVariable(*GlobalModule, FnTy, true,
				GlobalValue::ExternalLinkage, NULL, "FuelVM_new_kObject");
		void *ptr = void_cast(FuelVM_new_kObject);
		GlobalEngine->addGlobalMapping(G, ptr);
		sys::DynamicLibrary::AddSymbol("FuelVM_new_kObject", ptr);
	}
	return G;
}

static void EmitNewInst(LLVMIRBuilder *writer, INew *Node)
{
	enum TypeId type = Node->base.Type;
	uintptr_t conf   = Node->Conf;
	KonohaContext *kctx = writer->kctx;
	IRBuilder<> *builder = writer->builder;

	GlobalVariable *G = GetNewKObject();
	/* kObject *new_kObject(KonohaContext *, uint64_t, void *, uint64_t); */
	Value *Vctx = GetContext(writer);
	Value *Arg1 = EmitConstant(builder, (int64_t)0UL);
	Value *Arg2 = EmitConstant(builder, (void *) KClass_(type));
	Value *Arg3 = EmitConstant(builder, (uint64_t)conf);
	Value *Ret = builder->CreateCall4(G, Vctx, Arg1, Arg2, Arg3);
	SetValue(writer, (INode *)Node, Ret);
}

static Value *CreateBoxInst(LLVMIRBuilder *writer, IUnary *Node, Value *Val)
{
	GlobalVariable *G = GetNewKObject();
	enum TypeId type = ToUnBoxType(Node->base.Type);
	IRBuilder<> *builder = writer->builder;
	KonohaContext *kctx = writer->kctx;
	Value *Vctx = GetContext(writer);
	Value *Arg1 = EmitConstant(builder, (int64_t)0UL);
	Value *Arg2 = EmitConstant(builder, (void *) KClass_(ToKType(kctx, type)));
	Value *Ret = builder->CreateCall4(G, Vctx, Arg1, Arg2, Val);
	return Ret;
}

static void EmitUnaryInst(LLVMIRBuilder *writer, IUnary *Node)
{
	Value *Val = GetValue(writer, Node->Node);
	assert(Val != 0);
	enum TypeId Type = Node->base.Type;
	IRBuilder<> *builder = writer->builder;
#define VSET(WRITER, NODE, FUNC) SetValue(WRITER, (INode *)NODE, (WRITER)->builder->FUNC)
#define CASE(X, OP) case X: VSET(writer, Node, Create##OP(Val, #X));return
#define CASE_(X, OP) case X: do {\
	Value *Val_ = (writer)->builder->Create##OP(Val, #X);\
	Val_ = ToIntValueIfBool(writer->builder, Val_);\
	SetValue(writer, (INode *)Node, Val_);\
	return;\
} while(0)

	if(Type == TYPE_boolean) {
		Val = ToBoolValue(builder, Val);
		switch(Node->Op) {
			CASE_(Not, Not);
			default:
				assert(0 && "unreachable");
				break;
		}
	}
	else if(Type == TYPE_int) {
		switch(Node->Op) {
			CASE_(Not, Not); CASE(Neg, Neg);
			default:
				assert(0 && "unreachable");
				break;
		}
	} else if(Type == TYPE_float) {
		switch(Node->Op) {
			CASE(Neg, FNeg);
			default:
			assert(0 && "unreachable");
			break;
		}
	} else if(IsUnBoxedType(ToUnBoxType(Type))) {
		SetValue(writer, (INode *) Node, CreateBoxInst(writer, Node, Val));
		return;
	}
#undef CASE
#undef CASE_
}

static void EmitBinaryInst(LLVMIRBuilder *writer, IBinary *Node)
{
	Value *LHS = GetValue(writer, Node->LHS);
	Value *RHS = GetValue(writer, Node->RHS);
	assert(LHS != 0 && RHS != 0);
	IRBuilder<> *builder = writer->builder;
	enum TypeId Type = Node->LHS->Type;
#define CASE(X, OP)  case X: VSET(writer, Node, Create##OP(LHS, RHS, #X));return
#define CASE_(X, OP) case X: do {\
	Value *Val_ = (writer)->builder->Create##OP(LHS, RHS, #X);\
	Val_ = ToIntValueIfBool(writer->builder, Val_);\
	SetValue(writer, (INode *)Node, Val_);\
	return;\
} while(0)

	if(Type == TYPE_boolean) {
		LHS = ToBoolValue(builder, LHS);
		RHS = ToBoolValue(builder, RHS);
		switch(Node->Op) {
			CASE_(Eq, ICmpEQ); CASE_(Nq, ICmpNE);
			default:
				assert(0 && "unreachable");
				break;
		}
	} else if(Type == TYPE_int) {
		switch(Node->Op) {
			CASE(Add, Add); CASE(Sub, Sub); CASE(Mul, Mul); CASE(Div, SDiv); CASE(Mod, SRem);
			CASE(LShift, Shl); CASE(RShift, AShr); CASE(And, And); CASE(Or, Or); CASE(Xor, Xor);
			CASE_(Eq, ICmpEQ);  CASE_(Nq, ICmpNE);  CASE_(Gt, ICmpSGT);
			CASE_(Ge, ICmpSGE); CASE_(Lt, ICmpSLT); CASE_(Le, ICmpSLE);
			default:
				assert(0 && "unreachable");
				break;
		}
	} else if(Type == TYPE_float) {
		switch(Node->Op) {
			CASE(Add, FAdd); CASE(Sub, FSub); CASE(Mul, FMul); CASE(Div, FDiv);
			CASE_(Eq, FCmpOEQ); CASE_(Nq, FCmpONE); CASE_(Gt, FCmpOGT);
			CASE_(Ge, FCmpOGE); CASE_(Lt, FCmpOLT); CASE_(Le, FCmpOLE);
			default:
			assert(0 && "unreachable");
			break;
		}
	}
	assert(0 && "unreachable");
#undef CASE
#undef CASE_
}

static void EmitThrowInst(LLVMIRBuilder *writer, IThrow *Node)
{
	IRBuilder<> *builder = writer->builder;
	Value *Err = GetValue(writer, Node->Val);
	GlobalVariable *G;
	if((G = GlobalModule->getNamedGlobal("FuelVM_KRuntime_raise")) == 0) {
		std::vector<Type *> Args;
		union {
			void (*KRuntime_raise)(KonohaContext*, int, int, kString *, KonohaStack *);
			void *ptr;
		} F;
		F.KRuntime_raise = FuelVM_KRuntime_raise;

		Args.push_back(ToType(ID_PtrKonohaContextVar));
		Args.push_back(ToType(ID_int));
		Args.push_back(ToType(ID_int));
		Args.push_back(ToType(ID_PtrkObjectVar));
		Args.push_back(ToType(ID_PtrKonohaValueVar));
		FunctionType *FnTy = FunctionType::get(Type::getVoidTy(LLVM_CONTEXT()), Args, false);
		G = new GlobalVariable(*GlobalModule, FnTy, true, GlobalValue::ExternalLinkage, NULL, "FuelVM_KRuntime_raise");

		GlobalEngine->addGlobalMapping(G, F.ptr);
	}

	Value *Vctx = GetContext(writer);
	Value *Vsfp = GetStackTop(writer);
	Value *Vtop = PrepareCallStack(writer, Vsfp);
	/* stack_top[-4].uline */
	uint64_t uline = Node->uline;
	StoreValueToStack(builder, TYPE_int, getLongTy(), K_RTNIDX, Vtop,
			EmitConstant(builder, uline), "uline");

	Value *vsymbol = EmitConstant(builder, Node->exception);
	Value *vfault  = EmitConstant(builder, Node->fault);

	builder->CreateCall5(G, Vctx, vsymbol, vfault, Err, Vtop);
	builder->CreateUnreachable();
}

static void EmitPHIInst(LLVMIRBuilder *writer, IPHI *Node)
{
	IRBuilder<> *builder = writer->builder;
	unsigned Preds = ARRAY_size(writer->Current->preds);
	Type *Ty = ToLLVMType(Node->Val->Type);
	if(Node->Val->Type == TYPE_boolean)
		Ty = ToType(ID_long);
	PHINode *PHI = builder->CreatePHI(Ty, Preds, "PHI");
	SetValue(writer, (INode *) Node, PHI);
	SetValue(writer, Node->Val, PHI);

	INodePtr *x, *e;
	FOR_EACH_ARRAY(Node->Args, x, e) {
		INode *Node  = *(x);
		INode *block = *(x + 1);
		Value *Val = GetValue(writer, Node);
		BasicBlock *BB = GetBlock(writer, (Block *) block);
		assert(Val != 0);
		PHI->addIncoming(Val, BB);
		++x;
	}
}

static Value *GetFieldRef(LLVMIRBuilder *writer, IField *Inst)
{
	IRBuilder<> *builder = writer->builder;
	Value *Val = GetValue(writer, Inst->Node);
	Value *Dst = builder->CreateStructGEP(Val, kObjectVar_fieldObjectItems);
	Dst = builder->CreateConstInBoundsGEP1_32(Dst, Inst->FieldIndex);
	Dst = builder->CreateBitCast(Dst, PointerType::get(ToLLVMType(Inst->base.Type), 0));
	return Dst;
}

static Value *GetArrayRef(LLVMIRBuilder *writer, ICall *Inst, std::vector<Value *> &List)
{
	/* List = [This, Index, ...] */
	Value *Val = List[0];
	Value *Idx = List[1];
	Type *ReqTy;
	if(Inst->base.Type != TYPE_void) {
		ReqTy = PointerType::get(ToLLVMType(Inst->base.Type), 0);
	} else {
		ReqTy = PointerType::get(List[2]->getType(), 0);
	}
	IRBuilder<> *builder = writer->builder;
	Val = builder->CreateBitCast(Val, ToType(ID_kCharSequence));
	Val = builder->CreateStructGEP(Val, kCharSequence_byteptr);
	Val = builder->CreateLoad(Val);
	Val = builder->CreateGEP(Val, Idx);
	Val = builder->CreateBitCast(Val, ReqTy);
	return Val;
}

static void EmitWriteBarrier(LLVMIRBuilder *writer, Value *Parent, Value *OldRef, Value *New)
{
	GlobalVariable *G;
	if((G = GlobalModule->getNamedGlobal("FuelVM_UpdateObjectField")) == 0) {
		std::vector<Type *> Fields;
		Fields.push_back(ToType(ID_PtrKonohaContextVar));
		Fields.push_back(ToType(ID_PtrkObjectVar));
		Fields.push_back(ToType(ID_PtrkObjectVar));
		Fields.push_back(ToType(ID_PtrkObjectVar));
		FunctionType *FnTy = FunctionType::get(ToType(ID_void), Fields, false);
		G = new GlobalVariable(*GlobalModule, FnTy, true,
				GlobalValue::ExternalLinkage, NULL, "FuelVM_UpdateObjectField");
		void *ptr = void_cast(FuelVM_UpdateObjectField);
		GlobalEngine->addGlobalMapping(G, ptr);
	}
	if(!Parent->getType()->isPointerTy()) {
		Parent->dump();
		OldRef->dump();
		New->dump();
		asm volatile("int3");
	}
	if(New->getType()->isPointerTy()) {
		IRBuilder<> *builder = writer->builder;
		Value *Vctx = GetContext(writer);
		Value *Old  = builder->CreateLoad(OldRef);
		builder->CreateCall4(G, Vctx, Parent, Old, New);
	}
}

#define CASE(KIND) case IR_TYPE_##KIND:

static void EmitNode(LLVMIRBuilder *writer, INode *Node)
{
	switch(Node->Kind) {
		CASE(INew) {
			EmitNewInst(writer, (INew *) Node);
			break;
		}
		CASE(ICall) {
			ICall *Inst = (ICall *) Node;
			INode **MtdPtr = (ARRAY_n(Inst->Params, 0));
			IConstant *Mtd = (IConstant *) *MtdPtr;
			INodePtr *x, *e;
			*MtdPtr = 0;
			std::vector<Value *> List;
			FOR_EACH_ARRAY(Inst->Params, x, e) {
				if(*x == 0)
					continue;
				Value *V = GetValue(writer, *x);
				assert(V != 0);
				List.push_back(V);
			}
			EmitCall(writer, Inst, Mtd, List);
			break;
		}
		CASE(IFunction) {
			assert(0 && "unreachable");
			break;
		}
		CASE(IUpdate) {
			IUpdate *Inst = (IUpdate *) Node;
			IField  *LHS  = Inst->LHS;
			assert(CHECK_KIND((INode *)LHS, IField));
			switch(LHS->Op) {
				case GlobalScope:
				case EnvScope:
					assert(0 && "TODO");
				case FieldScope: {
					IRBuilder<> *builder = writer->builder;
					Value *Val = GetValue(writer, Inst->LHS->Node);
					Value *Dst = GetFieldRef(writer, Inst->LHS);
					Value *Src = GetValue(writer, Inst->RHS);
					EmitWriteBarrier(writer, Val, Dst, Src);
					builder->CreateStore(Src, Dst);
					break;
				}
				case LocalScope: {
					Value *NewVal = GetValue(writer, Inst->RHS);
					SetValue(writer, Node, NewVal);
					break;
				}
			}
			break;
		}
		CASE(IBranch) {
			EmitBranch(writer, (IBranch *) Node);
			break;
		}
		CASE(ITest) {
			assert(0 && "TODO");
			break;
		}
		CASE(IReturn) {
			IReturn *Inst = (IReturn *) Node;
			EmitRet(writer, Inst->Inst);
			break;
		}
		CASE(IJump) {
			EmitJump(writer, (IJump *) Node);
			break;
		}
		CASE(IThrow) {
			EmitThrowInst(writer, (IThrow *) Node);
			break;
		}
		CASE(ITry) {
			assert(0 && "TODO");
			break;
		}
		CASE(IYield) {
			assert(0 && "TODO");
			break;
		}
		CASE(IUnary) {
			EmitUnaryInst(writer, (IUnary *) Node);
			break;
		}
		CASE(IBinary) {
			EmitBinaryInst(writer, (IBinary *) Node);
			break;
		}
		CASE(IPHI) {
			EmitPHIInst(writer, (IPHI *) Node);
			break;
		}
		default:
			assert(0 && "unreachable");
	}
}

static void SetName(Value *v, const char *name)
{
	v->setName(name);
}

static void LLVMIRBuilder_visitList(Visitor *visitor, INode *Inst, const char *Tag, unsigned ElmSize, INodePtr *Insts)
{
	(void)Tag;(void)ElmSize;(void)Insts;
	EmitNode((LLVMIRBuilder *) visitor, Inst);
}

static void LLVMIRBuilder_visitValue(Visitor *visitor, INode *Node, const char *Tag, SValue Val)
{
	(void)Tag;
	LLVMIRBuilder *writer = (LLVMIRBuilder *) visitor;
	switch(Node->Kind) {
		CASE(IConstant) {
			SetConstant(writer, Node, Val);
			return;
		}
		CASE(IArgument) {
			SetArgument(writer, Node, Val.ival);
			return;
		}
		CASE(IField) {
			IField *Inst = (IField *) Node;
			switch(Inst->Op) {
				case GlobalScope:
				case EnvScope:
					assert(0 && "TODO");
					break;
				case FieldScope: {
					Value *Dst = GetFieldRef(writer, Inst);
					Dst = writer->builder->CreateLoad(Dst);
					SetValue(writer, Node, Dst);
					return;
				}
				case LocalScope:
					break;
			}
			return;
		}
		default:
			assert(0 && "unreachable");
	}
}

/* Emit Method Specific interface */
static Function *EmitInternalFunction(KonohaContext *kctx, Module *M, kMethod *mtd)
{
	enum TypeId RetTy = ConvertToTypeId(kctx, kMethod_GetReturnType(mtd)->typeId);
	std::vector<Type *> ParamTy;
	kParam *params = kMethod_GetParam(mtd);

	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);

	enum TypeId Type = ConvertToTypeId(kctx, mtd->typeId);
	ParamTy.push_back(ToType(ID_PtrKonohaContextVar));
	if(Type != TYPE_void) {
		ParamTy.push_back(ToLLVMType(Type));
	}
	for(unsigned i = 0; i < params->psize; ++i) {
		Type = ConvertToTypeId(kctx, params->paramtypeItems[i].attrTypeId);
		ParamTy.push_back(ToLLVMType(Type));
	}
	const char *name = ConstructMethodName(kctx, mtd, &wb, "Impl");
	FunctionType *FnTy = FunctionType::get(ToLLVMType(RetTy), ParamTy, false);
	Function *F = Function::Create(FnTy, GlobalValue::ExternalLinkage, name, M);
	Function::arg_iterator args = F->arg_begin();
	SetName(args++, "kctx");
	if(Type != TYPE_void) {
		SetName(args++, "this");
	}

	for(unsigned i = 0; i < params->psize; ++i) {
		const char *name = KSymbol_text(params->paramtypeItems[i].name);
		SetName(args++, name);
	}

	KLIB KBuffer_Free(&wb);
	return F;
}

/* Emit KMETHOD interface */
static Function *EmitFunction(KonohaContext *kctx, Module *M, kMethod *mtd, Function *Func)
{
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);

	std::vector<Type *> ParamTy;
	ParamTy.push_back(ToType(ID_PtrKonohaContextVar));
	ParamTy.push_back(ToType(ID_PtrKonohaValueVar));

	const char *name = ConstructMethodName(kctx, mtd, &wb, "");
	FunctionType *FnTy = FunctionType::get(ToType(ID_void), ParamTy, false);
	Function *FWrap = Function::Create(FnTy, GlobalValue::ExternalLinkage, name, M);
	Function::arg_iterator args = FWrap->arg_begin();
	Value *Vctx = args++;
	Value *Vsfp = args;
	SetName(Vctx, "kctx");
	SetName(Vsfp, "sfp");

	BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "EntryBlock", FWrap);
	IRBuilder<> *builder = new IRBuilder<>(BB);

	kParam *params = kMethod_GetParam(mtd);

	std::vector<Value *> Params;
	Params.push_back(Vctx);
	enum TypeId Type = ConvertToTypeId(kctx, mtd->typeId);
	if(Type != TYPE_void) {
		Params.push_back(LoadValueFromStack(builder, Type, ToLLVMType(Type), 0, Vsfp));
	}
	for(unsigned i = 0; i < params->psize; ++i) {
		ktypeattr_t type = params->paramtypeItems[i].attrTypeId;
		Type = ConvertToTypeId(kctx, type);
		Value *V = LoadValueFromStack(builder, Type, ToLLVMType(Type), i+1, Vsfp);
		Params.push_back(V);
	}
	Type = ConvertToTypeId(kctx, kMethod_GetReturnType(mtd)->typeId);

	Value *Ret = builder->CreateCall(Func, Params);
	if(Type != TYPE_void) {
		StoreValueToStack(builder, Type, ToLLVMType(Type), K_RTNIDX, Vsfp, Ret);
	}
	builder->CreateRetVoid();
	KLIB KBuffer_Free(&wb);
	delete builder;
	return FWrap;
}

static Value *CreateUndefinedValue(LLVMIRBuilder *writer, INode *Node)
{
	Type *Ty = ToLLVMType(Node->Type);
	Value *Val = new Argument(Ty);
	SetValue(writer, Node, Val, true);
	Node->Unused = 1;
	return Val;
}

static void PrepareLocalVariable(LLVMIRBuilder *writer, FuelIRBuilder *builder)
{
	BlockPtr *x, *e;
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		INodePtr *Ptr, *End;
		FOR_EACH_ARRAY((*x)->insts, Ptr, End) {
			if(CHECK_KIND(*Ptr, IUpdate)) {
				CreateUndefinedValue(writer, *Ptr);
			}
			else if(CHECK_KIND(*Ptr, IPHI)) {
				CreateUndefinedValue(writer, *Ptr);
			}
		}
	}
}

static void EmitRecompilationCheck(LLVMIRBuilder *writer, Function *F, BasicBlock *BB0, kMethod *mtd)
{
	IRBuilder<> *builder = writer->builder;

	Value *Vmtd = EmitConstant(builder, mtd);
	Vmtd = builder->CreateIntToPtr(Vmtd, ToType(ID_PtrkMethodVar));

	/* Vdelta = Vmtd->delta + 1 */
	Value *VdeltaRef = builder->CreateStructGEP(Vmtd, kMethodVar_delta);
	Value *Vdelta    = builder->CreateLoad(VdeltaRef);

	Constant *C01 = ConstantInt::get(Vdelta->getType(),  1);
	Constant *C10 = ConstantInt::get(Vdelta->getType(), 10);

	Vdelta = builder->CreateAdd(Vdelta, C01);
	builder->CreateStore(Vdelta, VdeltaRef);

	/* if(Vdelta > 10) { goto ThenBB } */
	BasicBlock *ThenBB  = BasicBlock::Create(getGlobalContext(), "Recompile", F);
	BasicBlock *MergeBB = BasicBlock::Create(getGlobalContext(), "BB", F);
	Value *Cond = builder->CreateICmpSGT(Vdelta, C10);
	builder->CreateCondBr(Cond, ThenBB, MergeBB);
	{
		/* ThenBB:
		 *   Recompile(Vkctx, Vmtd);
		 *   goto MergeBB;
		 **/
		builder->SetInsertPoint(ThenBB);
		GlobalVariable *G;
		if((G = GlobalModule->getNamedGlobal("FuelVM_Recompile")) == 0) {
			std::vector<Type *> Fields;
			Fields.push_back(ToType(ID_PtrKonohaContextVar));
			Fields.push_back(ToType(ID_PtrkMethodVar));
			FunctionType *FnTy = FunctionType::get(ToType(ID_void), Fields, false);
			G = new GlobalVariable(*GlobalModule, FnTy, true,
					GlobalValue::ExternalLinkage, NULL, "FuelVM_Recompile");
			void *ptr = void_cast(FuelVM_Recompile);
			GlobalEngine->addGlobalMapping(G, ptr);
		}
		Value *Vctx = GetContext(writer);
		builder->CreateCall2(G, Vctx, Vmtd);
		builder->CreateBr(MergeBB);
	}
	builder->SetInsertPoint(MergeBB);
	F->addFnAttr(Attributes::NoInline);
}

static void EmitPrologue(LLVMIRBuilder *writer, FuelIRBuilder *builder, IMethod *Mtd, int option)
{
	BlockPtr *x, *e;
	LLVMContext &Context = getGlobalContext();

	writer->Func = EmitInternalFunction(Mtd->Context, GlobalModule, Mtd->Method);

	if(CompiledCode[Mtd->Method]) {
		CompiledCode[Mtd->Method] = writer->Func;
		//F->replaceAllUsesWith(writer->Func);
	}
	BasicBlock *EntryBB = BasicBlock::Create(Context, "EntryBB", writer->Func);
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		SetBlock(writer, *x, BasicBlock::Create(Context, "BB", writer->Func));
	}

	PrepareLocalVariable(writer, builder);

	writer->builder = new IRBuilder<>(EntryBB);
	writer->builder->SetInsertPoint(EntryBB);
	Value *Vsfp = GetStackTop(writer);

	BasicBlock *BB0 = GetBlock(writer, Mtd->EntryBlock);
	if(option != O2Compile && Mtd->Method->mn != 0) {
		EmitRecompilationCheck(writer, writer->Func, BB0, Mtd->Method);
	}
	writer->builder->CreateBr(BB0);
	writer->builder->SetInsertPoint(BB0);
	PrepareCallStack(writer, Vsfp);
}

static void visitIBlock(LLVMIRBuilder *writer, Block *block)
{
	writer->Current = block;
	block->Evaled = 1;
	BasicBlock *BB = GetBlock(writer, block);
	writer->builder->SetInsertPoint(BB);
	INodePtr *x, *e;
	FOR_EACH_ARRAY(block->insts, x, e) {
		visitINode(&writer->visitor, *x);
	}
}

} /* namespace FuelVM */

using namespace FuelVM;
extern "C" {

bool FuelVM_HasOptimizedCode(kMethod *mtd)
{
	return CompiledCode[mtd] != 0;
}

ByteCode *IRBuilder_CompileToLLVMIR(FuelIRBuilder *builder, IMethod *Mtd, int option)
{
	InitLLVM();
	LLVMIRBuilder writer = {{
		0,
		visitElement,
		LLVMIRBuilder_visitList,
		LLVMIRBuilder_visitValue},
		Mtd->Context,
		0, 0, 0, 0, 0, 0, builder, 0
	};

	std::vector<ValueInfo> Variables(builder->LastNodeId);
	writer.Variables = &Variables;
	EmitPrologue(&writer, builder, Mtd, option);

	BlockPtr *x, *e;
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		visitIBlock(&writer, *x);
	}
	Function *Wrapper = EmitFunction(Mtd->Context, GlobalModule, Mtd->Method, writer.Func);

	/* optimization */
	FunctionPassManager FPM(GlobalModule);
	FPM.add(createVerifierPass());

	PassManager MPM;
	PassManagerBuilder Builder;
	Builder.OptLevel = 3;
	Builder.populateFunctionPassManager(FPM);
	Builder.populateModulePassManager(MPM);

	FPM.doInitialization();
	FPM.run(*Wrapper);
	FPM.run(*writer.Func);

	MPM.run(*GlobalModule);

#ifdef USE_LLVMIR_DUMP
	writer.Func->dump();
#endif

	delete writer.builder;
	void *fwrap = GlobalEngine->getPointerToFunction(Wrapper);
	if(option == O2Compile && Mtd->Method->mn != 0) {
		CompiledCode[Mtd->Method] = writer.Func;
	}
	return (ByteCode *) fwrap;
}

} /* extern "C" */
