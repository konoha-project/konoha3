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
#include "codegen.h"
#include "visitor.h"
#include "FuelVM.h"
#include "LLVMType.h"

using namespace llvm;

typedef struct LLVMIRBuilder {
	Visitor visitor;
	KonohaContext *kctx;
	IRBuilder<> *builder;
	Function *Func;
	Block    *Current;
	std::vector<Value *> *ValueTable;
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

static Value *EmitConstant(IRBuilder<> *builder, bool bval)
{
	return builder->getInt1(bval);
}

static Value *EmitConstant(IRBuilder<> *builder, int64_t ival)
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

static void SetValue(LLVMIRBuilder *writer, INode *Node, Value *Val)
{
	unsigned Idx = NODE_ID(Node);
	std::vector<Value *> *table = writer->ValueTable;
	table->reserve(Idx+1);
	(*table)[Idx] = Val;
}

static Value *GetValue(LLVMIRBuilder *writer, INode *Node)
{
	unsigned Idx = NODE_ID(Node);
	std::vector<Value *> *table = writer->ValueTable;
	Value *Val = table->at(Idx);
	assert(Val != 0 && "undefined value");
	return Val;
}

static BasicBlock *GetBlock(LLVMIRBuilder *writer, Block *Target)
{
	return (BasicBlock *) GetValue(writer, ToINode(Target));
}

static void SetBlock(LLVMIRBuilder *writer, Block *Target, BasicBlock *BB)
{
	SetValue(writer, ToINode(Target), (Value *) BB);
}

static void SetConstant(LLVMIRBuilder *writer, INode *Node, SValue Val)
{
	enum TypeId Type = Node->Type;
	//unsigned Id = NODE_ID(Node);

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
	for(unsigned i = 1; i < Index; i++) {
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

static Value *GetStackTop(LLVMIRBuilder *writer)
{
	IRBuilder<> *builder = writer->builder;
	Value *Vctx = GetContext(writer);
	Value *Vsfp = builder->CreateStructGEP(Vctx, KonohaContextVar_esp);
	Vsfp = builder->CreateLoad(Vsfp);
	return Vsfp;
}

static void RestoreStackTop(LLVMIRBuilder *writer, Value *newVal)
{
	IRBuilder<> *builder = writer->builder;
	Value *Vctx = GetContext(writer);
	Value *Vsfp = builder->CreateStructGEP(Vctx, KonohaContextVar_esp);
	builder->CreateStore(newVal, Vsfp, false);
}

static Value *ShiftStack(LLVMIRBuilder *writer, Value *Vsfp, unsigned Shift)
{
	IRBuilder<> *builder = writer->builder;
	return builder->CreateConstInBoundsGEP1_32(Vsfp, Shift);
}

static Value *PrepareCallStack(LLVMIRBuilder *writer, Value *Vsfp)
{
	return ShiftStack(writer, Vsfp, K_CALLDELTA);
}

static void StoreValueToStack(IRBuilder<> *builder, KClass *ct, int Idx, Value *Vsfp, Value *V)
{
	int fieldIdx = KClass_Is(UnboxType, ct) ?
		KonohaValueVar_unboxValue : KonohaValueVar_asObject;
	Value *Dst = builder->CreateConstInBoundsGEP2_32(Vsfp, Idx, fieldIdx);
	if(V->getType() == Type::getInt1Ty(LLVM_CONTEXT())) {
		V = builder->CreateZExt(V, GetLLVMType(ID_long));
	}
	builder->CreateStore(V, Dst, false);
}

static Value *LoadValueFromStack(IRBuilder<> *builder, KClass *ct, int Idx, Value *Vsfp)
{
	int fieldIdx = KClass_Is(UnboxType, ct) ?
		KonohaValueVar_unboxValue : KonohaValueVar_asObject;
	Value *Src = builder->CreateConstInBoundsGEP2_32(Vsfp, Idx, fieldIdx);
	return builder->CreateLoad(Src);
}

static void EmitCall(LLVMIRBuilder *writer, ICall *Inst, IConstant *Mtd, std::vector<Value *> &List)
{
	KonohaContext *kctx = writer->kctx;
	IRBuilder<> *builder = writer->builder;
	kMethod *method = (kMethod *) Mtd->Value.ptr;
	Value *MtdPtr  = EmitConstant(builder, method);
	Value *FuncPtr = builder->CreateStructGEP(MtdPtr, kMethodVar_invokeKMethodFunc);

	Value *Vctx = GetContext(writer);
	Value *Vsfp = GetStackTop(writer);
	Value *Vtop = PrepareCallStack(writer, Vsfp);

	kParam *params = kMethod_GetParam(method);

	int64_t uline = Inst->uline;
	StoreValueToStack(builder, KClass_Int, K_RTNIDX, Vsfp, EmitConstant(builder, uline));
	StoreValueToStack(builder, KClass_(method->typeId), 0, Vsfp, List[0]); /* this object */
	for(unsigned i = 1; i < params->psize+1; ++i) {
		ktypeattr_t type = params->paramtypeItems[i-1].attrTypeId;
		Value *V = List[i];
		StoreValueToStack(builder, KClass_(type), i, Vsfp, V);
	}

	FuncPtr = builder->CreateLoad(FuncPtr);
	RestoreStackTop(writer, Vtop);
	Value *Ret = builder->CreateCall2(FuncPtr, Vctx, Vtop);
	RestoreStackTop(writer, Vsfp);
	SetValue(writer, ToINode(Inst), Ret);
}

static void EmitRet(LLVMIRBuilder *writer, INode *Node)
{
	IRBuilder<> *builder = writer->builder;
	if(Node) {
		Value *Ret = GetValue(writer, Node);
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

static void EmitBranch(LLVMIRBuilder *writer, IBranch *Node)
{
	IRBuilder<> *builder = writer->builder;
	Value *Cond = GetValue(writer, Node->Cond);
	BasicBlock *ThenBB = GetBlock(writer, Node->ThenBB);
	BasicBlock *ElseBB = GetBlock(writer, Node->ElseBB);
	builder->CreateCondBr(Cond, ThenBB, ElseBB);
}

static void EmitUnaryInst(LLVMIRBuilder *writer, IUnary *Node)
{
	Value *Val = GetValue(writer, Node->Node);
#define VSET(WRITER, NODE, FUNC) SetValue(WRITER, (INode *)NODE, (WRITER)->builder->FUNC)

	enum TypeId Type = Node->base.Type;
	assert(Type == TYPE_int);
#define CASE(X, OP) case X: VSET(writer, Node, Create##OP(Val, #X));return
	if(Type == TYPE_int) {
		switch(Op) {
			CASE(Not, Not); CASE(Neg, Neg);
			default:
				assert(0 && "unreachable");
				break;
		}
	} else if(Type == TYPE_float) {
		switch(Op) {
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
#define VSET(WRITER, NODE, FUNC) SetValue(WRITER, (INode *)NODE, (WRITER)->builder->FUNC)

	enum TypeId Type = Node->LHS->Type;
#define CASE(X, OP) case X: VSET(writer, Node, Create##OP(LHS, RHS, #X));return
	if(Type == TYPE_int) {
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

#define CASE(KIND) case IR_TYPE_##KIND:

static void EmitNode(LLVMIRBuilder *writer, INode *Node)
{
	switch(Node->Kind) {
		CASE(ICond) {
			assert(0 && "TODO");
			break;
		}
		CASE(INew) {
			assert(0 && "TODO");
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
				List.push_back(GetValue(writer, *x));
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
			assert(0 && "TODO");
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
					SetValue(writer, Node, 0);
					break;
			}
			return;
		}
		default:
			assert(0 && "unreachable");
	}
}

static const char *ConstructMethodName(KonohaContext *kctx, kMethod *mtd, const char *suffix)
{
	KClass *kclass = KClass_(mtd->typeId);
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	KLIB KBuffer_printf(kctx, &wb, "%s_%s%s%s",
			KClass_text(kclass), KMethodName_Fmt2(mtd->mn), suffix);
	return KLIB KBuffer_text(kctx, &wb, EnsureZero);
}

/* Emit Method Specific interface */
static Function *EmitInternalFunction(KonohaContext *kctx, Module *M, kMethod *mtd)
{
	KClass *RetTy = kMethod_GetReturnType(mtd);
	std::vector<Type *> ParamTy;
	kParam *params = kMethod_GetParam(mtd);

	ParamTy.push_back(GetLLVMType(ID_PtrKonohaContextVar));
	for(unsigned i = 0; i < params->psize; ++i) {
		Type *Ty = convert_type(kctx, params->paramtypeItems[i].attrTypeId);
		ParamTy.push_back(Ty);
	}
	const char *name = ConstructMethodName(kctx, mtd, "Impl");
	FunctionType *FnTy = FunctionType::get(convert_type(kctx, RetTy), ParamTy, false);
	Function *F = Function::Create(FnTy, GlobalValue::ExternalLinkage, name, M);
	Function::arg_iterator args = F->arg_begin();

	for(unsigned i = 0; i < params->psize; ++i) {
		const char *name = KSymbol_text(params->paramtypeItems[i].name);
		SetName(args++, name);
	}
	return F;
}

/* Emit KMETHOD interface */
static Function *EmitFunction(KonohaContext *kctx, Module *M, kMethod *mtd, Function *Func)
{
	std::vector<Type *> ParamTy;
	ParamTy.push_back(GetLLVMType(ID_PtrKonohaContextVar));
	ParamTy.push_back(GetLLVMType(ID_PtrKonohaValueVar));

	const char *name = ConstructMethodName(kctx, mtd, "");
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
	for(unsigned i = 0; i < params->psize; ++i) {
		ktypeattr_t type = params->paramtypeItems[i].attrTypeId;
		Value *V = LoadValueFromStack(builder, KClass_(type), i, Vsfp);
		Params.push_back(V);
	}
	KClass *RetTy = kMethod_GetReturnType(mtd);
	Value *Ret = builder->CreateCall(Func, Params);
	if(RetTy != KClass_void) {
		StoreValueToStack(builder, RetTy, K_RTNIDX, Vsfp, Ret);
	}
	builder->CreateRetVoid();
	return FWrap;
}

static void EmitPrologue(LLVMIRBuilder *writer, FuelIRBuilder *builder, IMethod *Mtd)
{
	BasicBlock *EntryBB;
	writer->Func = EmitInternalFunction(Mtd->Context, GlobalModule, Mtd->Method);
	EntryBB = BasicBlock::Create(getGlobalContext(), "EntryBlock", writer->Func);
	writer->builder = new IRBuilder<>(EntryBB);

	BlockPtr *x, *e;
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		if(x == ARRAY_n(builder->Blocks, 0)) {
			continue;
		}
		SetBlock(writer, *x, BasicBlock::Create(getGlobalContext(), "BB", writer->Func));
	}
}

extern "C" {

ByteCode *IRBuilder_CompileToLLVMIR(FuelIRBuilder *builder, IMethod *Mtd)
{
	InitLLVM();
	LLVMIRBuilder writer = {{
		0,
			visitElement,
			LLVMIRBuilder_visitList,
			LLVMIRBuilder_visitValue},
				Mtd->Context,
				0, 0, 0, 0
	};
	EmitPrologue(&writer, builder, Mtd);

	std::vector<Value *> ValueTable(builder->LastNodeId);

	writer.ValueTable = &ValueTable;

	BlockPtr *x, *e;
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		writer.Current = *x;
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

	pm.doInitialization();
	pm.run(*Wrapper);
	pm.run(*writer.Func);

	GlobalModule->dump();
	void *f = GlobalEngine->getPointerToFunction(Wrapper);
	return (ByteCode *) f;
}

void FuelVM_GenerateLLVMIR(KonohaContext *kctx, kMethod *mtd, kBlock *block, int option)
{
	fprintf(stderr, "START LLVM IR GENERATION..\n");
	kNameSpace *ns = block->BlockNameSpace;
	struct KVirtualCode *vcode = ns->builderApi->GenerateKVirtualCode(kctx, mtd, block, option);
	union { struct KVirtualCode *ptr; KMethodFunc func; } V;
	V.ptr = vcode;
	((kMethodVar *)mtd)->invokeKMethodFunc = V.func;
	((kMethodVar *)mtd)->vcode_start = 0;
}

} /* extern "C" */
