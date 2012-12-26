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
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/IPO.h>
#include <vector>
#include <string>
#include <iostream>

#include <minikonoha/minikonoha.h>
#include <minikonoha/konoha_common.h>
#include <minikonoha/sugar.h>
#include <minikonoha/klib.h>
#include "codegen.h"
#include "visitor.h"
#include "FuelVM.h"
#include "LLVMType.h"

#define USE_LLVMIR_DUMP 1
using namespace llvm;

namespace FuelVM {
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
};

static Module *GlobalModule = 0;
static ExecutionEngine *GlobalEngine = 0;
static int Init = 0;

static void InitLLVM()
{
	if(Init != 0)
		return;
	Init = 1;
	InitializeNativeTarget();
	GlobalModule = new Module("LLVM", getGlobalContext());
	std::string Error;
	GlobalEngine = EngineBuilder(GlobalModule)
		.setEngineKind(EngineKind::JIT)
		.setErrorStr(&Error).create();
	if(Error != "") {
		std::cout << Error << std::endl;
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
	if((G = GlobalModule->getNamedGlobal(Name)) == 0) {
		G = new GlobalVariable(*GlobalModule, Ty, true, GlobalValue::ExternalLinkage, NULL, Name);
	}
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
	return builder->CreateBitCast(V, GetLLVMType(ID_PtrkObjectVar));
}

static Value *EmitConstant(IRBuilder<> *builder, kMethod *mtd)
{
	Constant *V = static_cast<Constant*>(EmitConstant(builder, (void *) mtd));
	return builder->CreateBitCast(V, GetLLVMType(ID_PtrkMethodVar));
}

static Value *GetValueFromParent(Block *BB, unsigned Id)
{
	std::vector<Value *> *table = reinterpret_cast<std::vector<Value *> *>(BB->Table);
	table->reserve(Id+1);
	Value *Val = table->at(Id);
	return Val;
}

static Value *GetValueFromParent(Block *BB, INode *Node)
{
	return GetValueFromParent(BB, NODE_ID(Node));
}

static Value *GetValue(LLVMIRBuilder *writer, INode *Node)
{
	Block *BB = writer->Current;
	while(true) {
		if(Value *Val = GetValueFromParent(BB, Node)) {
			return Val;
		}
		if(ARRAY_size(BB->preds) != 1)
			break;
		BB = *ARRAY_n(BB->preds, 0);
	}
	return NULL;
}

static void SetValueToBlock(Block *BB, INode *Node, Value *Val)
{
	unsigned Idx = NODE_ID(Node);
	std::vector<Value *> *table = reinterpret_cast<std::vector<Value *> *>(BB->Table);
	if(Node->Unused == 1 && CHECK_KIND(Node, IUpdate)) {
		if(Value *OldVal = GetValueFromParent(BB, NODE_ID(Node))) {
			OldVal->replaceAllUsesWith(Val);
		}
	}
	table->reserve(Idx+1);
	(*table)[Idx] = Val;
}

static void SetValue(LLVMIRBuilder *writer, INode *Node, Value *Val)
{
	Block *BB = writer->Current;
	SetValueToBlock(BB, Node, Val);
}

static BasicBlock *GetBlock(LLVMIRBuilder *writer, Block *Target)
{
	BlockPtr *EntryBBPtr = ARRAY_n(writer->FuelBuilder->Blocks, 0);
	return (BasicBlock *) GetValueFromParent(*EntryBBPtr, ToINode(Target));
}

static BasicBlock *GetParent(LLVMIRBuilder *writer, INode *Node)
{
	BlockPtr *EntryBBPtr = ARRAY_n(writer->FuelBuilder->Blocks, 0);
	return (BasicBlock *) GetValueFromParent(*EntryBBPtr, Node->ParentId);
}

static void SetBlock(LLVMIRBuilder *writer, Block *Target, BasicBlock *BB)
{
	BlockPtr *EntryBBPtr = ARRAY_n(writer->FuelBuilder->Blocks, 0);
	SetValueToBlock(*EntryBBPtr, ToINode(Target), (Value *) BB);
}

static void SetConstant(LLVMIRBuilder *writer, INode *Node, SValue Val)
{
	enum TypeId Type = Node->Type;

	IRBuilder<> *builder = writer->builder;
	switch(Type) {
		case TYPE_void:
			assert(0 && "FIXME");
		case TYPE_boolean: SetValue(writer, Node, EmitConstant(builder, Val.bval)); break;
		case TYPE_int:     SetValue(writer, Node, EmitConstant(builder, Val.ival)); break;
		case TYPE_float:   SetValue(writer, Node, EmitConstant(builder, Val.fval)); break;
		case TYPE_String:
		case TYPE_Function:
		case TYPE_Array:
		case TYPE_Method:
		case TYPE_Any:
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

static bool IsUnboxType(KClass *ct, Type *Type)
{
	if(KClass_Is(UnboxType, ct)) {
		if(ct->typeId == KType_0 && Type->isPointerTy())
			return false;
		return true;
	}
	return false;
}

static void StoreValueToStack(IRBuilder<> *builder, KClass *ct, Type *ReqTy, int Idx, Value *Vsfp, Value *V, const char *Name = "")
{
	int fieldIdx = IsUnboxType(ct, ReqTy) ?
		KonohaValueVar_unboxValue : KonohaValueVar_asObject;
	Value *Dst = builder->CreateConstInBoundsGEP2_32(Vsfp, Idx, fieldIdx, Name);
	if(V->getType() == builder->getInt1Ty()) {
		V = builder->CreateZExt(V, GetLLVMType(ID_long));
	} else if(V->getType() == builder->getDoubleTy()) {
		Type *Ty = PointerType::get(builder->getDoubleTy(), 0);
		Dst = builder->CreateBitCast(Dst, Ty);
	}
	builder->CreateStore(V, Dst, false);
}

static Value *LoadValueFromStack(IRBuilder<> *builder, KClass *ct, Type *ReqTy, int Idx, Value *Vsfp)
{
	int fieldIdx = IsUnboxType(ct, ReqTy) ?
		KonohaValueVar_unboxValue : KonohaValueVar_asObject;
	Value *Src = builder->CreateConstInBoundsGEP2_32(Vsfp, Idx, fieldIdx);
	if(ReqTy == builder->getDoubleTy()) {
		Type *Ty = PointerType::get(builder->getDoubleTy(), 0);
		Src = builder->CreateBitCast(Src, Ty);
	}
	return builder->CreateLoad(Src);
}

//static void check(KonohaStack *sfp, KonohaStack *top)
//{
//	asm volatile("int3");
//}

static void EmitCall(LLVMIRBuilder *writer, ICall *Inst, IConstant *Mtd, std::vector<Value *> &List)
{
	KonohaContext *kctx = writer->kctx;
	IRBuilder<> *builder = writer->builder;
	kMethod *method = (kMethod *) Mtd->Value.ptr;
	Value *MtdPtr  = EmitConstant(builder, method);

	Value *Vctx = GetContext(writer);
	Value *Vsfp = GetStackTop(writer);
	Value *Vtop = PrepareCallStack(writer, Vsfp);

	kParam *params = kMethod_GetParam(method);

	/* stack_top[-4].uline */
	uint64_t uline = Inst->uline;
	StoreValueToStack(builder, KClass_Int, getLongTy(), K_RTNIDX, Vtop,
			EmitConstant(builder, uline), "uline");

	/* stack_top[-4].defaultObject */
	KClass *NullType = KClass_(Inst->base.Type);
	kObject *DefObj  = KLIB Knull(kctx, NullType);
	Value *DefObjPtr = EmitConstant(builder, DefObj);
	StoreValueToStack(builder, KClass_Object, convert_type(kctx, KClass_Object),
			K_RTNIDX, Vtop, DefObjPtr, "default");

	/* stack_top[-2].calledMethod */
	StoreValueToStack(builder, KClass_Int, getLongTy(), K_MTDIDX, Vtop,
			builder->CreateBitCast(MtdPtr, GetLLVMType(ID_long), "Method"));

	/* stack_top[0..List.size()] */
	StoreValueToStack(builder, KClass_(method->typeId), List[0]->getType(),
			0, Vtop, List[0], "receiver");
	for(unsigned i = 1; i < params->psize+1; ++i) {
		ktypeattr_t type = params->paramtypeItems[i-1].attrTypeId;
		Value *V = List[i];
		StoreValueToStack(builder, KClass_(type), V->getType(), i, Vtop, V);
	}

	Value *FuncPtr;
	if(method->SourceToken->text == 0) {/* method is maybe 'Native' Method. */
		Type *FnTy = GetLLVMType(ID_KMethodFunc);
		FuncPtr = EmitGlobalVariable(kctx, FnTy, method);
	} else {
		FuncPtr = builder->CreateStructGEP(MtdPtr, kMethodVar_invokeKMethodFunc);
		FuncPtr = builder->CreateLoad(FuncPtr);
	}

	RestoreStackTop(writer, PrepareCallStack(writer, Vsfp, List.size()));

#if 0
	{
		GlobalVariable *G;
		if((G = GlobalModule->getNamedGlobal("check")) == 0) {
			std::vector<Type *> Fields;
			Fields.push_back(GetLLVMType(ID_PtrKonohaValueVar));
			Fields.push_back(GetLLVMType(ID_PtrKonohaValueVar));
			FunctionType *FnTy = FunctionType::get(Type::getVoidTy(LLVM_CONTEXT()), Fields, false);
			G = new GlobalVariable(*GlobalModule, FnTy, true,
					GlobalValue::ExternalLinkage, NULL, "check");
		}
		GlobalEngine->addGlobalMapping(G, (void *) check);
		builder->CreateCall2(G, Vsfp, Vtop);
	}
#endif

	builder->CreateCall2(FuncPtr, Vctx, Vtop);

	KClass *RetTy = kMethod_GetReturnType(method);
	Value *Ret = LoadValueFromStack(builder, RetTy, ToLLVMType(ToINode(Inst)->Type), K_RTNIDX, Vtop);
	RestoreStackTop(writer, Vsfp);
	SetValue(writer, ToINode(Inst), Ret);
}

static void EmitRet(LLVMIRBuilder *writer, INode *Node)
{
	IRBuilder<> *builder = writer->builder;
	if(Node) {
		Value *Ret = GetValue(writer, Node);
		Type *RetTy = Ret->getType();
		if(!RetTy->isPointerTy()) {
			if(RetTy == builder->getInt1Ty()) {
				Ret = builder->CreateZExt(Ret, GetLLVMType(ID_long));
			}
		}
		builder->CreateRet(Ret);
	} else {
		builder->CreateRetVoid();
	}
}

static void EmitJump(LLVMIRBuilder *writer, IJump *Node)
{
	IRBuilder<> *builder = writer->builder;
	BasicBlock *Target = GetBlock(writer, Node->TargetBlock);
	builder->CreateBr(Target);
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

		Value *Val = GetValue(writer, *x);
		if(Val->getType() != builder->getInt1Ty()) {
			Val = builder->CreateTrunc(Val, builder->getInt1Ty());
		}

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
		Value *Val = GetValue(writer, Node->Cond);
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
		Fields.push_back(GetLLVMType(ID_PtrKonohaContextVar));
		Fields.push_back(builder->getInt64Ty());
		Fields.push_back(builder->getInt8PtrTy());
		Fields.push_back(builder->getInt64Ty());
		FunctionType *FnTy = FunctionType::get(GetLLVMType(ID_PtrkObjectVar), Fields, false);
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
	enum TypeId Type = Node->base.Type;
	IRBuilder<> *builder = writer->builder;
#define VSET(WRITER, NODE, FUNC) SetValue(WRITER, (INode *)NODE, (WRITER)->builder->FUNC)
#define CASE(X, OP) case X: VSET(writer, Node, Create##OP(Val, #X));return
	if(Type == TYPE_boolean) {
		if(Val->getType() != builder->getInt1Ty()) {
			Val = builder->CreateTrunc(Val, builder->getInt1Ty());
		}
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
	IRBuilder<> *builder = writer->builder;
#define VSET(WRITER, NODE, FUNC) SetValue(WRITER, (INode *)NODE, (WRITER)->builder->FUNC)

	enum TypeId Type = Node->LHS->Type;
#define CASE(X, OP) case X: VSET(writer, Node, Create##OP(LHS, RHS, #X));return
	if(Type == TYPE_boolean) {
		if(LHS->getType() != builder->getInt1Ty()) {
			LHS = builder->CreateTrunc(LHS, builder->getInt1Ty());
		}
		if(RHS->getType() != builder->getInt1Ty()) {
			RHS = builder->CreateTrunc(RHS, builder->getInt1Ty());
		}
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
	(void)writer;(void)Node;
	assert(0 && "TODO");
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
				LHS = GetValue(writer, *Inst0);
				if(LHS->getType() != builder->getInt1Ty()) {
					LHS = builder->CreateTrunc(LHS, builder->getInt1Ty());
				}
				FOR_EACH_ARRAY(Inst->Insts, x, e) {
					if(x == Inst0) continue;
					RHS = GetValue(writer, *x);
					if(RHS->getType() != builder->getInt1Ty()) {
						RHS = builder->CreateTrunc(RHS, builder->getInt1Ty());
					}
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
				case FieldScope:
					assert(0 && "TODO");
					break;
				case LocalScope: {
					Value *NewVal = GetValue(writer, Inst->RHS);
					asm volatile("int3");
					SetValue(writer, Node, NewVal);
					SetValue(writer, (INode *)LHS, NewVal);
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
				asm volatile("int3");
			EmitBinaryInst(writer, (IBinary *) Node);
			break;
		}
		CASE(IPHI) {
			IPHI  *Inst = (IPHI *) Node;
			IRBuilder<> *builder = writer->builder;
			INode *Left  = (INode *) Inst->LHS;
			INode *Right = (INode *) Inst->RHS;
			Value *LHS = 0, *RHS = 0;
			if(Left && Right) {
				LHS = GetValue(writer, Left);
				RHS = GetValue(writer, Right);
				PHINode *PHI = builder->CreatePHI(LHS->getType(), 2, "PHI");
				PHI->addIncoming(LHS, GetParent(writer, Left));
				PHI->addIncoming(RHS, GetParent(writer, Right));
				SetValueToBlock(writer->Current, Inst->Val,  PHI);
			}
			break;
		}
		default:
			assert(0 && "unreachable");
	}
}

static Value *SetName(Value *v, const char *name)
{
	v->setName(name);
	return v;
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
					/* TODO */
					assert(0 && "TODO");
					//unsigned Src = REGISTER_UNDEFINED;
					//assert(Register_FindById(Allocator, NODE_ID(Inst->Node), &Src) == true);
					//unsigned Dst = Register_FindByIdOrAllocate(Allocator, NODE_ID(Node));
					//EMIT_LIR(writer, LoadField, Dst, Src, Inst->FieldIndex);
					return;
				}
				case LocalScope:
					//SetValue(writer, Node, 0);
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
	KClass *RetTy = kMethod_GetReturnType(mtd);
	std::vector<Type *> ParamTy;
	kParam *params = kMethod_GetParam(mtd);

	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);

	ParamTy.push_back(GetLLVMType(ID_PtrKonohaContextVar));
	ParamTy.push_back(convert_type(kctx, mtd->typeId));
	for(unsigned i = 0; i < params->psize; ++i) {
		Type *Ty = convert_type(kctx, params->paramtypeItems[i].attrTypeId);
		ParamTy.push_back(Ty);
	}
	const char *name = ConstructMethodName(kctx, mtd, &wb, "Impl");
	FunctionType *FnTy = FunctionType::get(convert_type(kctx, RetTy), ParamTy, false);
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
	ParamTy.push_back(GetLLVMType(ID_PtrKonohaContextVar));
	ParamTy.push_back(GetLLVMType(ID_PtrKonohaValueVar));

	const char *name = ConstructMethodName(kctx, mtd, &wb, "");
	FunctionType *FnTy = FunctionType::get(GetLLVMType(ID_void), ParamTy, false);
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
	KClass *kclass = KClass_(mtd->typeId);
	Params.push_back(LoadValueFromStack(builder, kclass, convert_type(kctx, kclass), 0, Vsfp));
	for(unsigned i = 0; i < params->psize; ++i) {
		ktypeattr_t type = params->paramtypeItems[i].attrTypeId;
		kclass = KClass_(type);
		Value *V = LoadValueFromStack(builder, kclass, convert_type(kctx, kclass), i+1, Vsfp);
		Params.push_back(V);
	}
	KClass *RetTy = kMethod_GetReturnType(mtd);
	Value *Ret = builder->CreateCall(Func, Params);
	if(RetTy != KClass_void) {
		StoreValueToStack(builder, RetTy, convert_type(kctx, RetTy), K_RTNIDX, Vsfp, Ret);
	}
	builder->CreateRetVoid();
	KLIB KBuffer_Free(&wb);
	return FWrap;
}

static void SetLocalVariable(Block *BB, INode *Node)
{
	Type *Ty = ToLLVMType(Node->Type);
	Value *Val = new Argument(Ty);
	SetValueToBlock(BB, Node, Val);
	Node->Unused = 1;
}

static void PrepareLocalVariable(FuelIRBuilder *builder)
{
	BlockPtr *x, *e;
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		Block *BB = *x;
		INodePtr *Ptr, *End;
		FOR_EACH_ARRAY(BB->insts, Ptr, End) {
			if(CHECK_KIND(*Ptr, IUpdate)) {
				SetLocalVariable(BB, *Ptr);
			}
			else if(IPHI *Inst = CHECK_KIND(*Ptr, IPHI)) {
				SetLocalVariable(BB, (INode *) Inst->LHS);
				SetLocalVariable(BB, (INode *) Inst->RHS);
			}
		}
	}
}

static void EmitPrologue(LLVMIRBuilder *writer, FuelIRBuilder *builder, IMethod *Mtd)
{
	BlockPtr *x, *e;
	writer->Func = EmitInternalFunction(Mtd->Context, GlobalModule, Mtd->Method);
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		(*x)->Table = (void *)(new std::vector<Value *>(builder->LastNodeId));
		SetBlock(writer, *x, BasicBlock::Create(getGlobalContext(), "BB", writer->Func));
	}

	PrepareLocalVariable(builder);

	BasicBlock *EntryBB = GetBlock(writer, Mtd->EntryBlock);
	writer->builder = new IRBuilder<>(EntryBB);
	writer->builder->SetInsertPoint(EntryBB);
	Value *Vsfp = GetStackTop(writer);
	PrepareCallStack(writer, Vsfp);
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
		0, 0, 0, 0, 0, 0, builder
	};

	EmitPrologue(&writer, builder, Mtd);

	BlockPtr *x, *e;
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		writer.Current = *x;
		BasicBlock *BB = GetBlock(&writer, *x);
		writer.builder->SetInsertPoint(BB);
		INodePtr *Inst, *End;
		FOR_EACH_ARRAY((*x)->insts, Inst, End) {
			visitINode(&writer.visitor, *Inst);
		}
	}
	Function *Wrapper = EmitFunction(Mtd->Context, GlobalModule, Mtd->Method, writer.Func);

	/* optimization */
	FunctionPassManager pm(GlobalModule);
	pm.add(new DataLayout(*(GlobalEngine->getDataLayout())));
	pm.add(createVerifierPass());
	pm.add(createInstructionCombiningPass()); // Cleanup for scalarrepl.
	pm.add(createLICMPass());                 // Hoist loop invariants
	pm.add(createIndVarSimplifyPass());       // Canonicalize indvars
	pm.add(createLoopDeletionPass());         // Delete dead loops

	writer.Func->dump();
	pm.doInitialization();
	pm.run(*Wrapper);
	pm.run(*writer.Func);

#ifdef USE_LLVMIR_DUMP
	writer.Func->dump();
	//Wrapper->dump();
#endif
	void *f = GlobalEngine->getPointerToFunction(Wrapper);
	return (ByteCode *) f;
}

} /* extern "C" */
