/****************************************************************************
 * Copyright (c) 2012-2013, Masahiro Ide <ide@konohascript.org> All rights reserved.
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

#include "codegen.h"

#ifdef __cplusplus
extern "C" {
#endif

DEF_ARRAY_OP_NOPOINTER(BlockPtr);
DEF_ARRAY_OP_NOPOINTER(INodePtr);

/* Node API */

#define CREATE_NODE(TYPE) ((TYPE *) newINode(builder, IR_TYPE_##TYPE))
static INode *newINode(struct FuelIRBuilder *builder, enum IRType Kind);
static void disposeINode(INode *Node);
static void disposeINodeImpl(INode *Node) {}

static Block *newBlock(FuelIRBuilder *builder)
{
	Block *Node = (Block *) newINode(builder, TYPE_IR_Block);
	ARRAY_init(BlockPtr, &Node->preds, 0);
	ARRAY_init(BlockPtr, &Node->succs, 0);
	ARRAY_init(INodePtr, &Node->insts, 0);
	Block_SetVisited(Node, 0);
	return (Node);
}

static void disposeBlock(INode *Node)
{
	Block *block = (Block *) Node;
	ARRAY_dispose(BlockPtr, &block->preds);
	ARRAY_dispose(BlockPtr, &block->succs);
	ARRAY_dispose(INodePtr, &block->insts);
}

static IConstant *newIConstant(FuelIRBuilder *builder, enum TypeId typeId, SValue value)
{
	IConstant *Node = CREATE_NODE(IConstant);
	Node->base.Type = typeId;
	Node->Value     = value;
	return (Node);
}

#define disposeIConstant disposeINodeImpl

static IArgument *newIArgument(FuelIRBuilder *builder, unsigned Index)
{
	IArgument *Node = CREATE_NODE(IArgument);
	Node->Index = Index;
	return (Node);
}

#define disposeIArgument disposeINodeImpl

static IField *newIField(FuelIRBuilder *builder, enum ScopeOp Op, enum TypeId Type, unsigned Index, INode *Val, unsigned FieldIdx)
{
	IField *Node = CREATE_NODE(IField);
	Node->Op = Op;
	if(Op == LocalScope) {
		Node->Id = Index;
	} else {
		Node->Node = Val;
	}
	Node->Hash = 0;
	Node->FieldIndex = FieldIdx;
	Node->base.Type = Type;
	return (Node);
}

#define disposeIField disposeINodeImpl

static INew *newINew(FuelIRBuilder *builder, uintptr_t Conf, enum TypeId Type)
{
	INew *Node = CREATE_NODE(INew);
	ARRAY_init(INodePtr, &Node->Params, 0);
	Node->base.Type = Type;
	Node->Conf = Conf;
	return (Node);
}

static void disposeINew(INode *Node)
{
	INew *New = (INew *) Node;
	ARRAY_dispose(INodePtr, &New->Params);
}

static ICall *newICall(FuelIRBuilder *builder, enum CallOp Op, uintptr_t uline)
{
	ICall *Node = CREATE_NODE(ICall);
	Node->Op = Op;
	Node->uline = uline;
	ARRAY_init(INodePtr, &Node->Params, 2);
	return (Node);
}

static void disposeICall(INode *Node)
{
	ICall *Call = (ICall *) Node;
	ARRAY_dispose(INodePtr, &Call->Params);
}

static IFunction *newIFunction(FuelIRBuilder *builder)
{
	IFunction *Node = CREATE_NODE(IFunction);
	return (Node);
}

#define disposeIFunction disposeINodeImpl

static IUpdate *newIUpdate(FuelIRBuilder *builder, IField *LHS, INode *RHS)
{
	IUpdate *Node = CREATE_NODE(IUpdate);
	Node->LHS = LHS;
	Node->RHS = RHS;
	INode_setType((INode *) Node, LHS->base.Type);
	return (Node);
}

#define disposeIUpdate disposeINodeImpl

static IBranch *newIBranch(FuelIRBuilder *builder, INode *Cond, Block *thenBB, Block *elseBB)
{
	IBranch *Node = CREATE_NODE(IBranch);
	Node->Cond = Cond;
	Node->ThenBB = thenBB;
	Node->ElseBB = elseBB;
	return (Node);
}

#define disposeIBranch disposeINodeImpl

static ITest *newITest(FuelIRBuilder *builder, enum TestOp Op, IField *Field, Block *Block)
{
	ITest *Node = CREATE_NODE(ITest);
	Node->Op = Op;
	Node->Value = Field;
	Node->TargetBlock = Block;
	return (Node);
}

#define disposeITest disposeINodeImpl

static IReturn *newIReturn(FuelIRBuilder *builder, INode *expr)
{
	IReturn *Node = CREATE_NODE(IReturn);
	Node->Inst = expr;
	return (Node);
}

#define disposeIReturn disposeINodeImpl

static IJump *newIJump(FuelIRBuilder *builder, Block *Block)
{
	IJump *Node = CREATE_NODE(IJump);
	Node->TargetBlock = Block;
	return (Node);
}

#define disposeIJump disposeINodeImpl

static IThrow *newIThrow(FuelIRBuilder *builder, INode *Val, int exception, int fault, uintptr_t uline)
{
	IThrow *Node = CREATE_NODE(IThrow);
	Node->Val = Val;
	Node->uline = uline;
	Node->exception = exception;
	Node->fault = fault;
	return (Node);
}

#define disposeIThrow disposeINodeImpl

static ITry *newITry(FuelIRBuilder *builder, Block *tryBlock, Block *finallyBlock)
{
	ITry *Node = CREATE_NODE(ITry);
	ARRAY_init(BlockPtr, &Node->CatchBBs, 0);
	return (Node);
}

static void disposeITry(INode *Node)
{
	ITry *Inst = (ITry *) Node;
	ARRAY_dispose(BlockPtr, &Inst->CatchBBs);
}

static IYield *newIYield(FuelIRBuilder *builder, INode *INode)
{
	IYield *Node = CREATE_NODE(IYield);
	return (Node);
}

#define disposeIYield disposeINodeImpl

static IUnary *newIUnary(FuelIRBuilder *builder, enum UnaryOp Op, INode *Val, uintptr_t uline)
{
	IUnary *Node = CREATE_NODE(IUnary);
	Node->Op   = Op;
	Node->Node = Val;
	Node->uline = uline;
	return (Node);
}

#define disposeIUnary disposeINodeImpl

static IBinary *newIBinary(FuelIRBuilder *builder, enum BinaryOp Op, INode *LHS, INode *RHS, uintptr_t uline)
{
	IBinary *Node = CREATE_NODE(IBinary);
	Node->Op = Op;
	Node->LHS = LHS;
	Node->RHS = RHS;
	Node->uline = uline;
	return (Node);
}

#define disposeIBinary disposeINodeImpl

static IPHI *newIPHI(FuelIRBuilder *builder, INode *Val)
{
	IPHI *Node = CREATE_NODE(IPHI);
	IField *Inst = CHECK_KIND(Val, IField);
	assert(Inst && "Val is IFeild Instruction");
	assert(Inst->Op == LocalScope);
	Node->Val = Val;
	ARRAY_init(INodePtr, &Node->Args, 0);
	INode_setType((INode *) Node, Val->Type);
	return (Node);
}

static void disposeIPHI(INode *Node)
{
	IPHI *Inst = (IPHI *) Node;
	ARRAY_dispose(INodePtr, &Inst->Args);
}

/* Builder API */
static void *IRBuilder_AllocNode(FuelIRBuilder *builder, size_t Size)
{
	return FuelVMMemoryPool_Alloc(&builder->mp, Size);
}

static INode *newINode(struct FuelIRBuilder *builder, enum IRType Kind)
{
	INode *Node = (INode *) IRBuilder_AllocNode(builder, sizeof(INodeImpl));
	Node->Kind = Kind;
	Node->Id   = builder->LastNodeId++;
	Node->Marked = 0;
	Node->Unused = 0;
	if(Kind != TYPE_IR_Block) {
		INode_setParent(Node, (INode *) builder->Current);
	}
	return (Node);
}

static void disposeERROR(INode *Node)
{
	assert(0 && "unreachable");
}

static void disposeINode(INode *Node)
{
	typedef void (*FnNode)(INode *);
	static FnNode Fn[] = {
		disposeERROR,
		disposeBlock,
#define IR_TYPE_DECL(X) dispose##X,
		IR_LIST(IR_TYPE_DECL)
#undef IR_TYPE_DECL
	};
	Fn[Node->Kind](Node);
	Node->Kind = TYPE_IR_ERROR;
}

static void visitInit(FuelIRBuilder *builder)
{
	FuelVMMemoryPool_Init(&builder->mp);
	ARRAY_init(INodePtr, &builder->LocalVar, 0);
}

static void PageDelete(char *begin, char *end)
{
	INodeImpl *p = (INodeImpl *) begin;
	INodeImpl *e = (INodeImpl *) end;
	while(p < e) {
		INode *Node = (INode *) p;
		if(Node->Kind != TYPE_IR_ERROR) {
			disposeINode(Node);
		}
		p++;
	}
}

static void visitExit(FuelIRBuilder *builder)
{
	ARRAY_dispose(INodePtr, &builder->LocalVar);
	FuelVMMemoryPool_Delete(&builder->mp, PageDelete);
}

static const struct IRBuilderAPI API = {
	visitInit,
	visitExit,
	disposeINode,
	newBlock,
#define IR_API_DECL(X) new##X,
	IR_LIST(IR_API_DECL)
#undef IR_API_DECL
};

/* -------------------------------------------------------------------------- */
/* [API] */

static void Block_add(Block *BB, INode *Stmt)
{
	ARRAY_add(INodePtr, &BB->insts, Stmt);
}

void IRBuilder_add(FuelIRBuilder *builder, INode *Stmt)
{
	Block_add(builder->Current, Stmt);
}

void INewInst_addParam(INew *Inst, INode *Param)
{
	ARRAY_add(INodePtr, &Inst->Params, Param);
}

void CallInst_addParam(ICall *Inst, INode *Param)
{
	ARRAY_add(INodePtr, &Inst->Params, Param);
}

void PHIInst_addParam(IPHI *Inst, IUpdate *Val)
{
	ARRAY_add(INodePtr, &Inst->Args, (INode *) Val);
}

void IRBuilder_Init(FuelIRBuilder *builder, KonohaContext *kctx, kNameSpace *ns)
{
	builder->API = &API;
	builder->API->Fn_Init(builder);
	builder->Context = kctx;
	builder->ClassInfo.cMath = KLIB kNameSpace_GetClassByFullName(kctx, ns, "Math", 4, NULL);
}

void IRBuilder_Exit(FuelIRBuilder *builder)
{
	builder->API->Fn_Exit(builder);
}

INode *CreateLocal(FuelIRBuilder *builder, enum TypeId type)
{
	unsigned Id = builder->LocalId++;
	INode *Node = (INode *) builder->API->newField(builder, LocalScope, type, Id, 0, 0);
	ARRAY_add(INodePtr, &builder->LocalVar, Node);
	assert(ARRAY_size(builder->LocalVar) == Id+1);
	IRBuilder_add(builder, Node);
	return Node;
}

INode *IRBuilder_FindLocalVarByHash(FuelIRBuilder *builder, enum TypeId Type, uintptr_t Hash)
{
	INodePtr *x, *e;
	FOR_EACH_ARRAY(builder->LocalVar, x, e) {
		IField *Node = (IField *) *x;
		enum TypeId NodeTy = ToINode(Node)->Type;
		if((NodeTy == Type || Type == TYPE_Object) && Node->Hash == Hash) {
			return ToINode(Node);
		}
	}
	return 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
