#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/IRBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/DataLayout.h>
#include <llvm/Pass.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <vector>
#include <string>

#include <minikonoha/minikonoha.h>
#include <minikonoha/konoha_common.h>
#include <minikonoha/sugar.h>
#include <minikonoha/klib.h>
#include <stdio.h>
#include "codegen.h"
#include "visitor.h"
#include "FuelVM.h"
#include "LLVMType.h"

#define USE_LLVMIR_DUMP 1
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

static void InitLLVM()
{
	if(GlobalModule)
		return;
	InitializeNativeTarget();
	GlobalModule = new Module("LLVM", getGlobalContext());
	std::string Error;
	GlobalEngine = EngineBuilder(GlobalModule)
		.setEngineKind(EngineKind::JIT)
		.setErrorStr(&Error).create();
	if(Error != "") {
		fprintf(stderr, "[%s:%d] %s", __FILE__, __LINE__, Error.c_str());
		abort();
	}
	LLVMType_Init();
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
	return builder->getInt1(bval);
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
	Constant *C = (sizeof(void*) == 4) ?
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
	union { void *ptr; KMethodFunc func; } Val;
	Val.func = mtd->invokeKMethodFunc;
	Value *G = EmitGlobalVariable(Ty, name, Val.ptr);
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
		}
		Node->Unused = 0;
	}
	std::vector<ValueInfo> *table = writer->Variables;
	ValueInfo Info(Val, IsUndef);
	(*table)[Node->Id] = Info;
}

static BasicBlock *GetBlock(LLVMIRBuilder *writer, Block *Target)
{
	ValueInfo &Info = GetValueInfoFromParent(writer, ToINode(Target));
	assert(Info.second() == false);
	return (BasicBlock *) Info.first();
}

static void SetBlock(LLVMIRBuilder *writer, Block *Target, BasicBlock *BB)
{
	SetValue(writer, ToINode(Target), (Value *) BB);
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
	Value *MtdPtr  = EmitConstant(builder, method);

	Value *Vctx = GetContext(writer);
	Value *Vsfp = GetStackTop(writer);
	Value *Vtop = PrepareCallStack(writer, Vsfp);

	/* stack_top[-4].uline */
	uint64_t uline = Inst->uline;
	StoreValueToStack(builder, TYPE_int, getLongTy(), K_RTNIDX, Vtop,
			EmitConstant(builder, uline), "uline");

	/* stack_top[-4].defaultObject */
	KClass *NullType = KClass_(ToKType(kctx, Inst->base.Type));
	kObject *DefObj  = KLIB Knull(kctx, NullType);
	Value *DefObjPtr = EmitConstant(builder, DefObj);
	StoreValueToStack(builder, TYPE_Object, ToLLVMType(TYPE_Object),
			K_RTNIDX, Vtop, DefObjPtr, "default");

	/* stack_top[-2].calledMethod */
	StoreValueToStack(builder, TYPE_int, getLongTy(), K_MTDIDX, Vtop,
			builder->CreateBitCast(MtdPtr, ToType(ID_long), "Method"));

	if(Inst->Op == VirtualCall) {
		/* stack_top[-2].asNameSpace */
		kNameSpace *ns = writer->FuelBuilder->Method->ns;
		Value *NSPtr = EmitConstant(builder, UPCAST(ns));
		StoreValueToStack(builder, TYPE_Object, NSPtr->getType(),
				K_NSIDX, Vtop, NSPtr, "NS");
	}

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
	if(method->SourceToken->text == 0) {/* method is maybe 'Native' Method. */
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
static Value *GetBoolValue(LLVMIRBuilder *writer, INode *Node)
{
	return ToBoolValue(writer->builder, GetValue(writer, Node));
}

static void EmitCondBranch(LLVMIRBuilder *writer, IBranch *Inst, ICond *Cond, int IsTopDecl)
{
	INodePtr *x, *e;
	BasicBlock *ThenBB;
	BasicBlock *ElseBB;
	if(Cond->Op == LogicalOr) {
		ThenBB = GetBlock(writer, Inst->ThenBB); ElseBB = GetBlock(writer, Inst->ElseBB);
	} else {
		ThenBB = GetBlock(writer, Inst->ElseBB); ElseBB = GetBlock(writer, Inst->ThenBB);
	}

	IRBuilder<> *builder = writer->builder;
	FOR_EACH_ARRAY(Cond->Insts, x, e) {
		if(CHECK_KIND(*x, ICond) != 0) {
			EmitCondBranch(writer, Inst, (ICond *) *x, 0);
			continue;
		}

		BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "BB", writer->Func);

		Value *Val = GetBoolValue(writer, *x);
		if(Cond->Op == LogicalOr) {
			builder->CreateCondBr(Val, ThenBB, BB);
		} else {
			builder->CreateCondBr(Val, BB, ThenBB);
		}
		builder->SetInsertPoint(BB);
	}

	if(IsTopDecl == true) {
		builder->CreateBr(ElseBB);
	}
}

static void EmitBranch(LLVMIRBuilder *writer, IBranch *Node)
{
	IRBuilder<> *builder = writer->builder;
	if(ICond *Cond = CHECK_KIND(Node->Cond, ICond)) {
		assert(Cond->Branch != 0);
		EmitCondBranch(writer, Node, Cond, true);
	} else {
		Value *Val = GetBoolValue(writer, Node->Cond);
		BasicBlock *ThenBB = GetBlock(writer, Node->ThenBB);
		BasicBlock *ElseBB = GetBlock(writer, Node->ElseBB);
		builder->CreateCondBr(Val, ThenBB, ElseBB);
	}
}

static void EmitNewInst(LLVMIRBuilder *writer, INew *Node)
{
	enum TypeId type = Node->base.Type;
	uintptr_t conf   = Node->Conf;
	KonohaContext *kctx = writer->kctx;
	IRBuilder<> *builder = writer->builder;

	GlobalVariable *G;
	if((G = GlobalModule->getNamedGlobal("new_kObject")) == 0) {
		std::vector<Type *> Fields;
		Fields.push_back(ToType(ID_PtrKonohaContextVar));
		Fields.push_back(builder->getInt64Ty());
		Fields.push_back(builder->getInt8PtrTy());
		Fields.push_back(builder->getInt64Ty());
		FunctionType *FnTy = FunctionType::get(ToType(ID_PtrkObjectVar), Fields, false);
		G = new GlobalVariable(*GlobalModule, FnTy, true,
				GlobalValue::ExternalLinkage, NULL, "new_kObject");
		typedef kObject *(*NewFunc)(KonohaContext *, kArray *, KClass *, uintptr_t);
		union {
			void *ptr;
			NewFunc f;
		} Val; Val.f = KLIB new_kObject;
		GlobalEngine->addGlobalMapping(G, Val.ptr);
	}

	/* kObject *new_kObject(KonohaContext *, uint64_t, void *, uint64_t); */
	Value *Vctx = GetContext(writer);
	Value *Arg1 = EmitConstant(builder, (int64_t)0UL);
	Value *Arg2 = EmitConstant(builder, (void *) KClass_(type));
	Value *Arg3 = EmitConstant(builder, (uint64_t)conf);
	Value *Ret = builder->CreateCall4(G, Vctx, Arg1, Arg2, Arg3);
	SetValue(writer, (INode *)Node, Ret);
}

static void EmitUnaryInst(LLVMIRBuilder *writer, IUnary *Node)
{
	Value *Val = GetValue(writer, Node->Node);
	assert(Val != 0);
	enum TypeId Type = Node->base.Type;
	IRBuilder<> *builder = writer->builder;
#define VSET(WRITER, NODE, FUNC) SetValue(WRITER, (INode *)NODE, (WRITER)->builder->FUNC)
#define CASE(X, OP) case X: VSET(writer, Node, Create##OP(Val, #X));return
	if(Type == TYPE_boolean) {
		Val = ToBoolValue(builder, Val);
		switch(Node->Op) {
			CASE(Not, Not);
			default:
				assert(0 && "unreachable");
				break;
		}
	}
	else if(Type == TYPE_int) {
		switch(Node->Op) {
			CASE(Not, Not); CASE(Neg, Neg);
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
	}
#undef CASE
}

static void EmitBinaryInst(LLVMIRBuilder *writer, IBinary *Node)
{
	Value *LHS = GetValue(writer, Node->LHS);
	Value *RHS = GetValue(writer, Node->RHS);
	assert(LHS != 0 && RHS != 0);
	IRBuilder<> *builder = writer->builder;
	enum TypeId Type = Node->LHS->Type;
#define CASE(X, OP) case X: VSET(writer, Node, Create##OP(LHS, RHS, #X));return
	if(Type == TYPE_boolean) {
		LHS = ToBoolValue(builder, LHS);
		RHS = ToBoolValue(builder, RHS);
		switch(Node->Op) {
			CASE(Eq, ICmpEQ); CASE(Nq, ICmpNE);
			default:
				assert(0 && "unreachable");
				break;
		}
	} else if(Type == TYPE_int) {
		switch(Node->Op) {
			CASE(Add, Add); CASE(Sub, Sub); CASE(Mul, Mul); CASE(Div, SDiv); CASE(Mod, SRem);
			CASE(LShift, Shl); CASE(RShift, AShr); CASE(And, And); CASE(Or, Or); CASE(Xor, Xor);
			CASE(Eq, ICmpEQ); CASE(Nq, ICmpNE); CASE(Gt, ICmpSGT);
			CASE(Ge, ICmpSGE); CASE(Lt, ICmpSLT); CASE(Le, ICmpSLE);
			default:
				assert(0 && "unreachable");
				break;
		}
	} else if(Type == TYPE_float) {
		switch(Node->Op) {
			CASE(Add, FAdd); CASE(Sub, FSub); CASE(Mul, FMul); CASE(Div, FDiv);
			CASE(Eq, FCmpOEQ); CASE(Nq, FCmpONE); CASE(Gt, FCmpOGT);
			CASE(Ge, FCmpOGE); CASE(Lt, FCmpOLT); CASE(Le, FCmpOLE);
			default:
			assert(0 && "unreachable");
			break;
		}
	}
	assert(0 && "unreachable");
#undef CASE
}

static void EmitThrowInst(LLVMIRBuilder *writer, IThrow *Node)
{
	IRBuilder<> *builder = writer->builder;
	Value *Err = GetValue(writer, Node->Val);
	GlobalVariable *G;
	KonohaContext *kctx = writer->kctx;
	if((G = GlobalModule->getNamedGlobal("KLIB_KRuntime_raise")) == 0) {
		std::vector<Type *> Args;
		union {
			void (*KRuntime_raise)(KonohaContext*, int, int, kString *, KonohaStack *);
			void *ptr;
		} F;
		F.KRuntime_raise = KLIB KRuntime_raise;

		Args.push_back(ToType(ID_PtrKonohaContextVar));
		Args.push_back(ToType(ID_int));
		Args.push_back(ToType(ID_int));
		Args.push_back(ToType(ID_PtrkObjectVar));
		Args.push_back(ToType(ID_PtrKonohaValueVar));
		FunctionType *FnTy = FunctionType::get(Type::getVoidTy(LLVM_CONTEXT()), Args, false);
		G = new GlobalVariable(*GlobalModule, FnTy, true, GlobalValue::ExternalLinkage, NULL, "KLIB_KRuntime_raise");

		GlobalEngine->addGlobalMapping(G, F.ptr);
	}

	Value *Vctx = GetContext(writer);
	Value *Vsfp = GetStackTop(writer);
	Value *Vtop = PrepareCallStack(writer, Vsfp);
	/* stack_top[-4].uline */
	uint64_t uline = Node->uline;
	StoreValueToStack(builder, TYPE_int, getLongTy(), K_RTNIDX, Vtop,
			EmitConstant(builder, uline), "uline");

	Value *vsymbol = EmitConstant(builder, (int)KException_("RuntimeScript"));
	Value *vfault  = EmitConstant(builder, (int)SoftwareFault);

	builder->CreateCall5(G, Vctx, vsymbol, vfault, Err, Vtop);
	builder->CreateUnreachable();
}

static void EmitPHIInst(LLVMIRBuilder *writer, IPHI *Node)
{
	IRBuilder<> *builder = writer->builder;
	unsigned Preds = ARRAY_size(writer->Current->preds);
	Type *Ty = ToLLVMType(Node->Val->Type);
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
	Dst =  builder->CreateConstInBoundsGEP1_32(Dst, Inst->FieldIndex);
	Dst = builder->CreateBitCast(Dst, PointerType::get(ToLLVMType(Inst->base.Type), 0));
	return Dst;
}

#define CASE(KIND) case IR_TYPE_##KIND:

static void EmitNode(LLVMIRBuilder *writer, INode *Node)
{
	switch(Node->Kind) {
		CASE(ICond) {
			ICond *Inst = (ICond *) Node;
			if(Inst->Branch == NULL) {
				INodePtr *x, *e;
				Value *LHS, *RHS;
				INodePtr *Inst0 = ARRAY_n(Inst->Insts, 0);
				IRBuilder<> *builder = writer->builder;
				LHS = GetBoolValue(writer, *Inst0);
				FOR_EACH_ARRAY(Inst->Insts, x, e) {
					if(x == Inst0) continue;
					RHS = GetBoolValue(writer, *x);
					if(Inst->Op == LogicalOr) {
						LHS = builder->CreateOr(LHS, RHS);
					} else {
						LHS = builder->CreateAnd(LHS, RHS);
					}
				}
				SetValue(writer, Node, LHS);
			}
			break;
		}
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
			assert(0 && "TODO");
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
					Value *Dst = GetFieldRef(writer, Inst->LHS);
					Value *Src = GetValue(writer, Inst->RHS);
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
	ParamTy.push_back(ToLLVMType(Type));
	for(unsigned i = 0; i < params->psize; ++i) {
		Type = ConvertToTypeId(kctx, params->paramtypeItems[i].attrTypeId);
		ParamTy.push_back(ToLLVMType(Type));
	}
	const char *name = ConstructMethodName(kctx, mtd, &wb, "Impl");
	FunctionType *FnTy = FunctionType::get(ToLLVMType(RetTy), ParamTy, false);
	Function *F = Function::Create(FnTy, GlobalValue::ExternalLinkage, name, M);
	Function::arg_iterator args = F->arg_begin();
	SetName(args++, "kctx");
	SetName(args++, "this");

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
	Params.push_back(LoadValueFromStack(builder, Type, ToLLVMType(Type), 0, Vsfp));
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

static void EmitPrologue(LLVMIRBuilder *writer, FuelIRBuilder *builder, IMethod *Mtd)
{
	BlockPtr *x, *e;
	writer->Func = EmitInternalFunction(Mtd->Context, GlobalModule, Mtd->Method);
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		SetBlock(writer, *x, BasicBlock::Create(getGlobalContext(), "BB", writer->Func));
	}

	PrepareLocalVariable(writer, builder);

	BasicBlock *EntryBB = GetBlock(writer, Mtd->EntryBlock);
	writer->builder = new IRBuilder<>(EntryBB);
	writer->builder->SetInsertPoint(EntryBB);
	Value *Vsfp = GetStackTop(writer);
	PrepareCallStack(writer, Vsfp);
}

static void visitIBlock(LLVMIRBuilder *writer, Block *block)
{
	writer->Current = block;
	block->base.Evaled = 1;
	BasicBlock *BB = GetBlock(writer, block);
	writer->builder->SetInsertPoint(BB);
	INodePtr *x, *e;
	FOR_EACH_ARRAY(block->insts, x, e) {
		visitINode(&writer->visitor, *x);
	}
}

} /* namespace FuelVM */

extern "C" {

ByteCode *IRBuilder_CompileToLLVMIR(FuelIRBuilder *builder, IMethod *Mtd)
{
	using namespace FuelVM;
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
	EmitPrologue(&writer, builder, Mtd);

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
	void *f = GlobalEngine->getPointerToFunction(Wrapper);
	return (ByteCode *) f;
}

} /* extern "C" */
