/****************************************************************************
 * Copyright (c) 2012, Masahiro Ide <ide@konohascript.org> All rights reserved.
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
#define DISPOSE_NODE(NODE) (disposeINode((INode *)(NODE)))
static void disposeINode(INode *Node);

static void disposeINodeImpl(INode *Node)
{
}

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
	INodePtr *x, *e;
	FOR_EACH_ARRAY(block->insts, x, e) {
		DISPOSE_NODE(*x);
	}
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

static ILabel *newILabel(FuelIRBuilder *builder, int Symbol)
{
	ILabel *Node = CREATE_NODE(ILabel);
	Node->Id = Symbol;
	return (Node);
}

#define disposeILabel disposeINodeImpl

static IField *newIField(FuelIRBuilder *builder, enum ScopeOp Op, enum TypeId Type, unsigned Index)
{
	IField *Node = CREATE_NODE(IField);
	Node->Op = Op;
	Node->Id = Index;
	Node->Hash = 0;
	Node->base.Type = Type;
	return (Node);
}

#define disposeIField disposeINodeImpl

static ICond *newICond(FuelIRBuilder *builder, enum ConditionalOp Op)
{
	ICond *Node = CREATE_NODE(ICond);
	ARRAY_init(INodePtr, &Node->Insts, 0);
	return (Node);
}

static void disposeICond(INode *Node)
{
	ICond *Cond = (ICond *) Node;
	INodePtr *x, *e;
	FOR_EACH_ARRAY(Cond->Insts, x, e) {
		DISPOSE_NODE(*x);
	}
}

static INew *newINew(FuelIRBuilder *builder, INode *Type)
{
	INew *Node = CREATE_NODE(INew);
	return (Node);
}

static void disposeINew(INode *Node)
{
	INew *New = (INew *) Node;
	INodePtr *x, *e;
	FOR_EACH_ARRAY(New->Params, x, e) {
		DISPOSE_NODE(*x);
	}
}

static ICall *newICall(FuelIRBuilder *builder, enum CallOp Op)
{
	ICall *Node = CREATE_NODE(ICall);
	Node->Op = Op;
	ARRAY_init(INodePtr, &Node->Params, 2);
	return (Node);
}

static void disposeICall(INode *Node)
{
	ICall *Call = (ICall *) Node;
	INodePtr *x, *e;
	FOR_EACH_ARRAY(Call->Params, x, e) {
		DISPOSE_NODE(*x);
	}
}

static IFunction *newIFunction(FuelIRBuilder *builder)
{
	IFunction *Node = CREATE_NODE(IFunction);
	return (Node);
}

static void disposeIFunction(INode *Node)
{
	IFunction *Func = (IFunction *) Node;
	INodePtr *x, *e;
	FOR_EACH_ARRAY(Func->Env, x, e) {
		DISPOSE_NODE(*x);
	}
}

static IUpdate *newIUpdate(FuelIRBuilder *builder, IField *LHS, INode *RHS)
{
	IUpdate *Node = CREATE_NODE(IUpdate);
	Node->LHS = LHS;
	Node->RHS = RHS;
	return (Node);
}

static void disposeIUpdate(INode *Node)
{
	IUpdate *Update = (IUpdate *) Node;
	DISPOSE_NODE(Update->LHS);
	DISPOSE_NODE(Update->RHS);
}

static IBranch *newIBranch(FuelIRBuilder *builder, INode *Cond, Block *thenBB, Block *elseBB)
{
	IBranch *Node = CREATE_NODE(IBranch);
	Node->Cond = Cond;
	Node->ThenBB = thenBB;
	Node->ElseBB = elseBB;
	return (Node);
}

static void disposeIBranch(INode *Node)
{
	IBranch *Inst = (IBranch *) Node;
	DISPOSE_NODE(Inst->Cond);
}

static ITest *newITest(FuelIRBuilder *builder, enum TestOp Op, IField *Field, Block *Block)
{
	ITest *Node = CREATE_NODE(ITest);
	Node->Op = Op;
	Node->Value = Field;
	Node->TargetBlock = Block;
	return (Node);
}

static void disposeITest(INode *Node)
{
	ITest *Inst = (ITest *) Node;
	DISPOSE_NODE(Inst->Value);
}

static IReturn *newIReturn(FuelIRBuilder *builder, INode *expr)
{
	IReturn *Node = CREATE_NODE(IReturn);
	Node->Inst = expr;
	return (Node);
}

static void disposeIReturn(INode *Node)
{
	IReturn *Inst = (IReturn *) Node;
	DISPOSE_NODE(Inst->Inst);
}

static IJump *newIJump(FuelIRBuilder *builder, Block *Block)
{
	IJump *Node = CREATE_NODE(IJump);
	Node->TargetBlock = Block;
	return (Node);
}

#define disposeIJump disposeINodeImpl

static IThrow *newIThrow(FuelIRBuilder *builder, INode *Val)
{
	IThrow *Node = CREATE_NODE(IThrow);
	return (Node);
}

static void disposeIThrow(INode *Node)
{
	IThrow *Inst = (IThrow *) Node;
	DISPOSE_NODE(Inst->Val);
}

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

static void disposeIYield(INode *Node)
{
	IYield *Inst = (IYield *) Node;
	DISPOSE_NODE(Inst->Value);
}

static IUnary *newIUnary(FuelIRBuilder *builder, enum UnaryOp Op, INode *Val)
{
	IUnary *Node = CREATE_NODE(IUnary);
	Node->Op   = Op;
	Node->Node = Val;
	return (Node);
}

static void disposeIUnary(INode *Node)
{
	IUnary *Inst = (IUnary *) Node;
	DISPOSE_NODE(Inst->Node);
}

static IBinary *newIBinary(FuelIRBuilder *builder, enum BinaryOp Op, INode *LHS, INode *RHS)
{
	IBinary *Node = CREATE_NODE(IBinary);
	Node->Op = Op;
	Node->LHS = LHS;
	Node->RHS = RHS;
	return (Node);
}

static void disposeIBinary(INode *Node)
{
	IBinary *Inst = (IBinary *) Node;
	DISPOSE_NODE(Inst->LHS);
	DISPOSE_NODE(Inst->RHS);
}

/* Builder API */
static void *IRBuilder_AllocNode(FuelIRBuilder *builder, size_t Size)
{
	return JSONMemoryPool_Alloc(&builder->mp, Size);
}

static INode *newINode(struct FuelIRBuilder *builder, enum IRType Kind)
{
	INode *Node = (INode *) IRBuilder_AllocNode(builder, sizeof(INodeImpl));
	Node->Kind = Kind;
	Node->Id   = builder->LastNodeId++;
	INode_setRange(Node, INTPTR_MIN, INTPTR_MAX);
	return (Node);
}

static void visitInit(FuelIRBuilder *builder)
{
	JSONMemoryPool_Init(&builder->mp);
	ARRAY_init(INodePtr, &builder->LocalVar, 0);
}

static void visitExit(FuelIRBuilder *builder)
{
	ARRAY_dispose(INodePtr, &builder->LocalVar);
	JSONMemoryPool_Delete(&builder->mp);
}

static void disposeINode(INode *Node)
{
	typedef void (*FnNode)(INode *);
	static FnNode Fn[] = {
		disposeBlock,
#define IR_TYPE_DECL(X) dispose##X,
		IR_LIST(IR_TYPE_DECL)
#undef IR_TYPE_DECL
	};
	Fn[Node->Kind](Node);
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

void CondInst_addParam(ICond *Inst, INode *Param)
{
	ARRAY_add(INodePtr, &Inst->Insts, Param);
}

void IRBuilder_Init(FuelIRBuilder *builder)
{
	builder->API = &API;
	builder->API->Fn_Init(builder);
}

void IRBuilder_Exit(FuelIRBuilder *builder)
{
	builder->API->Fn_Exit(builder);
}

INode *CreateLocal(FuelIRBuilder *builder, enum TypeId type)
{
	unsigned Id = builder->LocalId++;
	INode *Node = (INode *) builder->API->newField(builder, LocalScope, type, Id);
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
		if(ToINode(Node)->Type == Type && Node->Hash == Hash) {
			return ToINode(Node);
		}
	}
	return 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
