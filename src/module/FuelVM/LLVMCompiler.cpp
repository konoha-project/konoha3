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

static Value *CreateConstant(IRBuilder<> *builder, bool bval)
{
	return builder->getInt1(bval);
}

static Value *CreateConstant(IRBuilder<> *builder, int64_t ival)
{
	return builder->getInt64(ival);
}

static Value *CreateConstant(IRBuilder<> *builder, double dval)
{
	Type *Ty = builder->getDoubleTy();
	return ConstantFP::get(Ty, dval);
}

static Value *CreateConstant(IRBuilder<> *builder, void *ptr)
{
	PointerType *Ty = builder->getInt8PtrTy();
	Constant *C = (sizeof(void*) == 4) ?
		builder->getInt32((uint32_t) (uintptr_t) ptr):
		builder->getInt64((uint64_t) (uintptr_t) ptr);
	return ConstantExpr::getIntToPtr(C, Ty);
}

static Value *CreateConstant(IRBuilder<> *builder, kObject *obj)
{
	Constant *V = static_cast<Constant*>(CreateConstant(builder, (void *) obj));
	return builder->CreateBitCast(V, GetLLVMType(ID_PtrkObjectVar));
}

static Value *CreateConstant(IRBuilder<> *builder, kMethod *mtd)
{
	Constant *V = static_cast<Constant*>(CreateConstant(builder, (void *) mtd));
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
	return table->at(Idx);
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
		case TYPE_boolean: SetValue(writer, Node, CreateConstant(builder, Val.bval)); break;
		case TYPE_int:     SetValue(writer, Node, CreateConstant(builder, Val.ival)); break;
		case TYPE_float:   SetValue(writer, Node, CreateConstant(builder, Val.dval)); break;
		case TYPE_String:
		case TYPE_Function:
		case TYPE_Array:
		case TYPE_Method:
		case TYPE_Any:
		default:
			SetValue(writer, Node, CreateConstant(builder, (kObject *)Val.ptr));
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
	Value *stack_top = Vsfp;//builder->CreateConstInBoundsGEP1_32(Vsfp, K_CALLDELTA);
	return stack_top;
}

static void StoreValueToStack(IRBuilder<> *builder, KonohaClass *ct, int Idx, Value *Vsfp, Value *V)
{
	int fieldIdx = (KClass_IsUnbox(ct)) ?KonohaValueVar_unboxValue : KonohaValueVar_asObject;
	Value *Dst = builder->CreateConstInBoundsGEP2_32(Vsfp, Idx, fieldIdx);
	builder->CreateStore(V, Dst, false);
}


static Value *LoadValueFromStack(IRBuilder<> *builder, KonohaClass *ct, int Idx, Value *Vsfp)
{
	int fieldIdx = (KClass_IsUnbox(ct)) ?KonohaValueVar_unboxValue : KonohaValueVar_asObject;
	Value *Src = builder->CreateConstInBoundsGEP2_32(Vsfp, Idx, fieldIdx);
	return builder->CreateLoad(Src);
}


static void CreateCall(LLVMIRBuilder *writer, ICall *Inst, IConstant *Mtd, std::vector<Value *> &List)
{
	KonohaContext *kctx = writer->kctx;
	IRBuilder<> *builder = writer->builder;
	kMethod *method = (kMethod *) Mtd->Value.ptr;
	Value *MtdPtr  = CreateConstant(builder, method);
	Value *FuncPtr = builder->CreateStructGEP(MtdPtr, kMethodVar_invokeMethodFunc);

	Value *Vctx = GetContext(writer);
	Value *Vsfp = GetStackTop(writer);

	kParam *params = kMethod_GetParam(method);
	StoreValueToStack(builder, KClass_(method->typeId), 0, Vsfp, List[0]);
	for(unsigned i = 1; i < params->psize+1; ++i) {
		kattrtype_t type = params->paramtypeItems[i-1].attrTypeId;
		Value *V = List[i];
		StoreValueToStack(builder, KClass_(type), i, Vsfp, V);
	}

	FuncPtr = builder->CreateLoad(FuncPtr);
	Value *Ret = builder->CreateCall2(FuncPtr, Vctx, Vsfp);
	SetValue(writer, ToINode(Inst), Ret);
}

static void CreateRet(LLVMIRBuilder *writer, INode *Node)
{
	IRBuilder<> *builder = writer->builder;
	if(Node) {
		Value *Ret = GetValue(writer, Node);
		builder->CreateRet(Ret);
	} else {
		builder->CreateRetVoid();
	}
}

static void CreateJump(LLVMIRBuilder *writer, IJump *Node)
{
	IRBuilder<> *builder = writer->builder;
	BasicBlock *Target = GetBlock(writer, Node->TargetBlock);
	builder->CreateBr(Target);
}

static void CreateBranch(LLVMIRBuilder *writer, IBranch *Node)
{
	IRBuilder<> *builder = writer->builder;
	Value *Cond = GetValue(writer, Node->Cond);
	BasicBlock *ThenBB = GetBlock(writer, Node->ThenBB);
	BasicBlock *ElseBB = GetBlock(writer, Node->ElseBB);
	builder->CreateCondBr(Cond, ThenBB, ElseBB);
}

static void CreateBinaryInst(LLVMIRBuilder *writer, IBinary *Node)
{
	Value *LHS = GetValue(writer, Node->LHS);
	Value *RHS = GetValue(writer, Node->RHS);
#define VSET(WRITER, NODE, FUNC) SetValue(WRITER, (INode *)NODE, (WRITER)->builder->FUNC)
	switch(Node->Op) {
#define CASE(X) case X:
		CASE(Add) {
			VSET(writer, Node, CreateAdd(LHS, RHS, "add"));break;
		}
		CASE(Sub) {
			VSET(writer, Node, CreateSub(LHS, RHS, "sub"));break;
		}
		CASE(Mul) {
			VSET(writer, Node, CreateMul(LHS, RHS, "mul"));break;
		}
		CASE(Div) {
			VSET(writer, Node, CreateSDiv(LHS, RHS, "div"));break;
		}
		CASE(Mod) {
			VSET(writer, Node, CreateSRem(LHS, RHS, "mod"));break;
		}
		CASE(LShift) {
			VSET(writer, Node, CreateShl(LHS, RHS, "lshr"));break;
		}
		CASE(RShift) {
			VSET(writer, Node, CreateAShr(LHS, RHS, "rshr"));break;
		}
		CASE(And) {
			VSET(writer, Node, CreateAnd(LHS, RHS, "and"));break;
		}
		CASE(Or) {
			VSET(writer, Node, CreateOr(LHS, RHS, "or"));break;
		}
		CASE(Xor) {
			VSET(writer, Node, CreateXor(LHS, RHS, "xor"));break;
		}
		CASE(Eq) {
			VSET(writer, Node, CreateICmpEQ(LHS, RHS, "eq"));break;
		}
		CASE(Nq) {
			VSET(writer, Node, CreateICmpNE(LHS, RHS, "ne"));break;
		}
		CASE(Gt) {
			VSET(writer, Node, CreateICmpSGT(LHS, RHS, "gt"));break;
		}
		CASE(Ge) {
			VSET(writer, Node, CreateICmpSGE(LHS, RHS, "ge"));break;
		}
		CASE(Lt) {
			VSET(writer, Node, CreateICmpSLT(LHS, RHS, "lt"));break;
		}
		CASE(Le) {
			VSET(writer, Node, CreateICmpSLE(LHS, RHS, "le"));break;
		}
#undef CASE
	}
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
			CreateCall(writer, Inst, Mtd, List);
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
			CreateBranch(writer, (IBranch *) Node);
			break;
		}
		CASE(ITest) {
			assert(0 && "TODO");
			break;
		}
		CASE(IReturn) {
			IReturn *Inst = (IReturn *) Node;
			CreateRet(writer, Inst->Inst);
			break;
		}
		CASE(IJump) {
			CreateJump(writer, (IJump *) Node);
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
			assert(0 && "TODO");
			break;
		}
		CASE(IBinary) {
			CreateBinaryInst(writer, (IBinary *) Node);
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
		CASE(ILabel) {
			assert(0 && "unreachable");
			break;
		}
		CASE(IField) {
			asm volatile("int3");
			return;
		}
		default:
			assert(0 && "unreachable");
	}
}

static const char *ConstructMethodName(KonohaContext *kctx, kMethod *mtd, const char *suffix)
{
	KonohaClass *kclass = KClass_(mtd->typeId);
	KGrowingBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	KLIB KBuffer_printf(kctx, &wb, "%s_%s%s%s",
			KClass_t(kclass), MethodName_Fmt2(mtd->mn), suffix);
	return KLIB KBuffer_Stringfy(kctx, &wb, 1);
}

/* Create Method Specific interface */
static Function *CreateInternalFunction(KonohaContext *kctx, Module *M, kMethod *mtd)
{
	KonohaClass *RetTy = kMethod_GetReturnType(mtd);
	std::vector<Type *> ParamTy;
	kParam *params = kMethod_GetParam(mtd);

	ParamTy.push_back(GetLLVMType(ID_PtrKonohaContextVar));
	for(unsigned i = 0; i < params->psize; ++i) {
		Type *Ty = convert_type(kctx, params->paramtypeItems[i].attrTypeId);
		ParamTy.push_back(Ty);
	}
	const char *name = ConstructMethodName(kctx, mtd, "Impl");
	FunctionType *FnTy = FunctionType::get(convert_type(kctx, RetTy), ParamTy, false);
	Function *F = cast<Function>(M->getOrInsertFunction(name, FnTy));
	Function::arg_iterator args = F->arg_begin();

	for(unsigned i = 0; i < params->psize; ++i) {
		const char *name = SYM_t(params->paramtypeItems[i].name);
		SetName(args++, name);
	}
	return F;
}

/* Create KMETHOD interface */
static Function *CreateFunction(KonohaContext *kctx, Module *M, kMethod *mtd, Function *Func)
{
	std::vector<Type *> ParamTy;
	ParamTy.push_back(GetLLVMType(ID_PtrKonohaContextVar));
	ParamTy.push_back(GetLLVMType(ID_PtrKonohaValueVar));

	const char *name = ConstructMethodName(kctx, mtd, "");
	FunctionType *FnTy = FunctionType::get(GetLLVMType(ID_void), ParamTy, false);
	Function *FWrap = cast<Function>(M->getOrInsertFunction(name, FnTy));
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
		kattrtype_t type = params->paramtypeItems[i].attrTypeId;
		Value *V = LoadValueFromStack(builder, KClass_(type), i, Vsfp);
		Params.push_back(V);
	}
	KonohaClass *RetTy = kMethod_GetReturnType(mtd);
	Value *Ret = builder->CreateCall(Func, Params);
	if(RetTy != KClass_void) {
		StoreValueToStack(builder, RetTy, -4, Vsfp, Ret);
	}
	builder->CreateRetVoid();
	return FWrap;
}

static void EmitPrologue(LLVMIRBuilder *writer, FuelIRBuilder *builder, IMethod *Mtd)
{
	BasicBlock *EntryBB;
	writer->Func = CreateInternalFunction(Mtd->Context, GlobalModule, Mtd->Method);
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
	Function *Wrapper = CreateFunction(Mtd->Context, GlobalModule, Mtd->Method, writer.Func);
	GlobalModule->dump();
	void *f = GlobalEngine->getPointerToFunction(Wrapper);
	return (ByteCode *) f;
}

void FuelVM_GenerateLLVMIR(KonohaContext *kctx, kMethod *mtd, kBlock *block, int option)
{
	fprintf(stderr, "START LLVM IR GENERATION..\n");
	kNameSpace *ns = block->BlockNameSpace;
	struct VirtualCode *vcode = ns->builderApi->GenerateVirtualCode(kctx, mtd, block, option);
	union { struct VirtualCode *ptr; MethodFunc func; } V;
	V.ptr = vcode;
	((kMethodVar *)mtd)->invokeMethodFunc = V.func;
	((kMethodVar *)mtd)->vcode_start = 0;
}

} /* extern "C" */
