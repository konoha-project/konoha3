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

#include "kmemory_pool.h"
#include "karray.h"
#include "vmcommon.h"
#include <stdlib.h>

#ifndef CODEGEN_H
#define CODEGEN_H

#ifdef __cplusplus
extern "C" {
#endif

struct Block;
typedef struct Block Block;
typedef Block *BlockPtr;
typedef struct INode INode;
typedef INode *INodePtr;

DEF_ARRAY_STRUCT(BlockPtr);
DEF_ARRAY_T(BlockPtr);

DEF_ARRAY_STRUCT(INodePtr);
DEF_ARRAY_T(INodePtr);

struct FuelIRBuilder;
typedef struct FuelIRBuilder FuelIRBuilder;

/*
 * IR Type Hierarchy
 *              [Node]
 *     +----------+------------+
 *     |                       |
 *  [Variable]           [Instruction]
 *     |                       |
 *  [Constant]           +-----+--------+
 *  [Argument]     [BranchInst]     [NAryInst]
 *  [Label]              |              |
 *  [Field]            [Branch]       [New]
 *                     [Test]         [Call]
 *                     [Return]       [Cond]
 *                     [Throw]        [Update]
 *                     [Try]          [Function]
 *                     [Yield]        [UnaryInst]
 *                     [Jump]         [BinaryInst]
 */

#define IR_LIST(OP)\
	OP(IConstant)\
	OP(IArgument)\
	OP(ILabel)\
	OP(IField)\
	OP(ICond)\
	OP(INew)\
	OP(ICall)\
	OP(IFunction)\
	OP(IUpdate)\
	OP(IBranch)\
	OP(ITest)\
	OP(IReturn)\
	OP(IJump)\
	OP(IThrow)\
	OP(ITry)\
	OP(IYield)\
	OP(IUnary)\
	OP(IBinary)

enum IRType {
	TYPE_IR_Block,
#define IR_TYPE_DECL(X) IR_TYPE_##X,
	IR_LIST(IR_TYPE_DECL)
#undef IR_TYPE_DECL
	TYPE_IR_MAX,
	TYPE_IR_ERROR = -1
};

/*$ Node */
struct INode {
	enum IRType Kind;
	enum TypeId Type;
	unsigned Id:31;
	unsigned Marked:1;
	intptr_t range_begin;
	intptr_t range_end;
};

static inline void INode_setRangeBegin(struct INode *Node, intptr_t Begin)
{
	Node->range_begin = Begin;
}

static inline void INode_setRangeEnd(struct INode *Node, intptr_t End)
{
	Node->range_end = End;
}

static inline intptr_t INode_getRangeBegin(struct INode *Node)
{
	return Node->range_begin;
}

static inline intptr_t INode_getRangeEnd(struct INode *Node)
{
	return Node->range_end;
}

static inline void INode_setRange(struct INode *Node, intptr_t Begin, intptr_t End)
{
	INode_setRangeBegin(Node, Begin);
	INode_setRangeEnd(Node, End);
}

static inline void INode_setType(struct INode *Node, enum TypeId Type)
{
	Node->Type = Type;
}

/*$ IMethod */
typedef struct IMethod {
	KonohaContext *Context;
	kMethod       *Method;
	Block         *EntryBlock;
} IMethod;

/*$ BasicBlock */
struct Block {
	INode base;
	ARRAY(BlockPtr) preds;
	ARRAY(BlockPtr) succs;
	ARRAY(INodePtr) insts;
};

/*$ Value Constant { Ptr, Double } */
typedef struct IConstant {
	INode base;
	SValue Value;
} IConstant;

/*$ Value Argument { Int } */
typedef struct IArgument {
	INode base;
	unsigned Index;
} IArgument;

/*$ Value Label { Int } */
typedef struct ILabel {
	INode base;
	unsigned Id;
} ILabel;

enum ScopeOp {
	GlobalScope, EnvScope, FieldScope, LocalScope
};

/*$ Value Field { FieldOp } */
typedef struct IField {
	INode base;
	enum ScopeOp Op;
	unsigned Id;
	unsigned NumOfUpdate; /* this field represents how many update this field. */
	uintptr_t Hash;
} IField;

enum ConditionalOp {
	LogicalOr, LogicalAnd
};

/*$ bool ICond { CondOp, [Inst] } */
typedef struct ICond {
	INode base;
	enum ConditionalOp Op;
	ARRAY(INodePtr) Insts;
} ICond;

/*$ Any  New { Method, [Inst] } */
typedef struct INew {
	INode base;
	unsigned Flags;
	ARRAY(INodePtr) Params;
} INew;

enum CallOp {
	DefaultCall, VirtualCall, NeedTypeCheck
};

/*$ Any  Call { CallOp, Method, Inst Reciver, [Inst] } */
typedef struct ICall {
	INode base;
	enum CallOp Op;
	ARRAY(INodePtr) Params;
	uintptr_t uline;
} ICall;

/*$ Any  Function { [Symbol] } */
typedef struct IFunction {
	INode base;
	ARRAY(INodePtr) Env;
	uintptr_t uline;
} IFunction;

/*$ void Update { Symbol, Inst } */
typedef struct IUpdate {
	INode base;
	IField *LHS;
	INode  *RHS;
} IUpdate;

/*$ void IBranch { Inst, Block, Block } */
typedef struct IBranch {
	INode base;
	INode *Cond;
	Block *ThenBB;
	Block *ElseBB;
} IBranch;

enum TestOp {
	TypeCheck, TypeGuard, SafePointCheck, Recompilation, BoundaryCheck
};

/*$ void Test { Variable, Block } */
typedef struct ITest {
	INode base;
	enum TestOp Op;
	IField *Value;
	Block *TargetBlock;
} ITest;

/*$ Any Return { Inst } */
typedef struct IReturn {
	INode base;
	INode *Inst;
} IReturn;

/*$ void Jump { Label } */
typedef struct IJump {
	INode base;
	Block *TargetBlock;
} IJump;

/*$ void Throw { Inst } */
typedef struct IThrow {
	INode base;
	INode *Val;
	uintptr_t uline;
} IThrow;

/*$ void Try { Block, [Field Block], Block } */
typedef struct ITry {
	INode base;
	Block *TryBB;
	Block *FinallyBB;
	ARRAY(BlockPtr) CatchBBs;
} ITry;

/*$ void Yield { Variable } */
typedef struct IYield {
	INode base;
	INode *Value;
} IYield;

enum UnaryOp {
	Not/*!*/, Neg/*-*/,
	UnaryOp_NotFound = -1
};

typedef struct IUnary {
	INode base;
	enum UnaryOp Op;
	INode *Node;
} IUnary;

enum BinaryOp {
	Add, Sub, Mul, Div, Mod,
	LShift, RShift, And, Or, Xor,
	Eq, Nq, Gt, Ge, Lt, Le,
	BinaryOp_NotFound = -1
} Op;

typedef struct IBinary {
	INode base;
	enum BinaryOp Op;
	INode *LHS;
	INode *RHS;
} IBinary;

typedef union INodeImpl {
	INode Node;
	Block Blk;
#define IR_TYPE_DECL(X) X _##X;
	IR_LIST(IR_TYPE_DECL)
#undef IR_TYPE_DECL
} INodeImpl;

typedef ARRAY(INodePtr) VariableTable;

typedef struct IRBuilderAPI {
	void (*Fn_Init)(FuelIRBuilder *builder);
	void (*Fn_Exit)(FuelIRBuilder *builder);
	void (*Fn_Dispose)(INode *Node);
	/* Create BasicBlock Node */
	Block *(*newBlock)(FuelIRBuilder *builder);
	/* Create Literal Node */
	IConstant *(*newConstant)(FuelIRBuilder *builder, enum TypeId typeId, SValue value);
	IArgument *(*newArgument)(FuelIRBuilder *builder, unsigned Index);
	ILabel    *(*newLabel)(FuelIRBuilder *builder, int Symbol);
	IField    *(*newField)(FuelIRBuilder *builder, enum ScopeOp Op, enum TypeId Type, unsigned Index);
	/* Create IInstruction Node */
	ICond     *(*newCond)(FuelIRBuilder *builder, enum ConditionalOp Op);
	INew      *(*newNew)(FuelIRBuilder *builder, enum TypeId Type);
	ICall     *(*newCall)(FuelIRBuilder *builder, enum CallOp Op, uintptr_t uline);
	IFunction *(*newFunction)(FuelIRBuilder *builder);
	IUpdate   *(*newUpdate)(FuelIRBuilder *builder, IField *Sym, INode *RHS);
	IBranch   *(*newBranch)(FuelIRBuilder *builder, INode *cond, Block *thenStmt, Block *elseEtmt);
	ITest     *(*newTest)(FuelIRBuilder *builder, enum TestOp Op, IField *Field, Block *TargetBlock);
	IReturn   *(*newReturn)(FuelIRBuilder *builder, INode *expr);
	IJump     *(*newJump)(FuelIRBuilder *builder, Block *TargetBlock);
	IThrow    *(*newThrow)(FuelIRBuilder *builder, INode *Exception, uintptr_t uline);
	ITry      *(*newTry)(FuelIRBuilder *builder, Block *tryBlock, Block *finallyBlock);
	IYield    *(*newYield)(FuelIRBuilder *builder, INode *INode);
	IUnary    *(*newUnary)(FuelIRBuilder *builder, enum UnaryOp Op, INode *INode);
	IBinary   *(*newBinary)(FuelIRBuilder *builder, enum BinaryOp Op, INode *LHS, INode *RHS);
} IRBuilderAPI;

struct FuelIRBuilder {
	const IRBuilderAPI *API;
	unsigned LastNodeId;
	unsigned LocalId;
	MemoryPool mp;
	VariableTable LocalVar;
	Block *Current;
	ARRAY(BlockPtr) Blocks;
	union {
		int   InstructionId; /* used by InstructionDumper */
		void *ByteCode;
	};
};

void IRBuilder_Init(FuelIRBuilder *builder);
void IRBuilder_Exit(FuelIRBuilder *builder);
union ByteCode *IRBuilder_Compile(FuelIRBuilder *builder, IMethod *Mtd, int option);
union ByteCode *IRBuilder_CompileToLLVMIR(FuelIRBuilder *builder, IMethod *Mtd);
INode *IRBuilder_FindLocalVarByHash(FuelIRBuilder *builder, enum TypeId type, uintptr_t Hash);

void IRBuilder_add(FuelIRBuilder *builder, INode *Stmt);
void INewInst_addParam(INew *Inst, INode *Param);
void CallInst_addParam(ICall *Inst, INode *Param);
void CondInst_addParam(ICond *Inst, INode *Param);

static inline void IField_setHash(IField *Inst, uintptr_t Hash)
{
	Inst->Hash = Hash;
}

static inline Block *CreateBlock(FuelIRBuilder *builder)
{
	return builder->API->newBlock(builder);
}

INode *CreateLocal(FuelIRBuilder *builder, enum TypeId type);

static inline INode *CreateArgument(FuelIRBuilder *builder, unsigned Index)
{
	INode *Node = (INode *) builder->API->newArgument(builder, Index);
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateConstant(FuelIRBuilder *builder, enum TypeId type, SValue S0)
{
	INode *Node = (INode *) builder->API->newConstant(builder, type, S0);
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateBool(FuelIRBuilder *builder, int64_t Val)
{
	SValue S0; S0.ival = Val;
	return CreateConstant(builder, TYPE_boolean, S0);
}

static inline INode *CreateInt(FuelIRBuilder *builder, int64_t Val)
{
	SValue S0; S0.ival = Val;
	return CreateConstant(builder, TYPE_int, S0);
}

static inline INode *CreateDouble(FuelIRBuilder *builder, double Val)
{
	SValue S0; S0.fval = Val;
	return CreateConstant(builder, TYPE_float, S0);
}

static inline INode *CreateObject(FuelIRBuilder *builder, enum TypeId Type, void *Val)
{
	SValue S0; S0.ptr = Val;
	return CreateConstant(builder, Type, S0);
}

static inline INode *CreateCond(FuelIRBuilder *builder, enum ConditionalOp Op)
{
	INode *Node = (INode *) builder->API->newCond(builder, Op);
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateUpdate(FuelIRBuilder *builder, INode *Field, INode *Inst)
{
	INode *Node = (INode *) builder->API->newUpdate(builder, (IField *) Field, Inst);
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateBranch(FuelIRBuilder *builder, INode *Val, Block *ThenBB, Block *ElseBB)
{
	INode *Node = (INode *) builder->API->newBranch(builder, Val, ThenBB, ElseBB);
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateJump(FuelIRBuilder *builder, Block *Block)
{
	INode *Node = (INode *) builder->API->newJump(builder, Block);
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateReturn(FuelIRBuilder *builder, INode *Val)
{
	INode *Node = (INode *) builder->API->newReturn(builder, Val);
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateThrow(FuelIRBuilder *builder, INode *Val, uintptr_t uline)
{
	INode *Node = (INode *) builder->API->newThrow(builder, Val, uline);
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateNew(FuelIRBuilder *builder, enum TypeId type)
{
	INode *Node = (INode *) builder->API->newNew(builder, type);
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateCall(FuelIRBuilder *builder, enum CallOp Op, uintptr_t uline)
{
	INode *Node = (INode *) builder->API->newCall(builder, Op, uline);
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateUnaryInst(FuelIRBuilder *builder, enum UnaryOp Op, INode *Val)
{
	INode *Node = (INode *) builder->API->newUnary(builder, Op, Val);
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateBinaryInst(FuelIRBuilder *builder, enum BinaryOp Op, INode *LHS, INode *RHS)
{
	INode *Node = (INode *) builder->API->newBinary(builder, Op, LHS, RHS);
	IRBuilder_add(builder, Node);
	return Node;
}

static inline void IRBuilder_setBlock(FuelIRBuilder *builder, Block *Block)
{
	builder->Current = Block;
}

static inline void IRBuilder_JumpTo(FuelIRBuilder *builder, Block *Block)
{
	CreateJump(builder, Block);
}

static inline INode *check_kind(INode *Node, enum IRType Kind)
{
	if(Node->Kind == Kind) {
		return Node;
	} else {
		return 0;
	}
}

static inline bool IsBranchInst(INode *Node)
{
	enum IRType Kind = Node->Kind;
	return (Kind == IR_TYPE_IBranch ||
			Kind == IR_TYPE_ITest ||
			Kind == IR_TYPE_IReturn ||
			Kind == IR_TYPE_IThrow ||
			Kind == IR_TYPE_ITry ||
			Kind == IR_TYPE_IYield ||
			Kind == IR_TYPE_IJump);
}

static inline bool Block_HasTerminatorInst(Block *block)
{
	if(ARRAY_size(block->insts) == 0) {
		return false;
	}
	INode *Inst = *(ARRAY_last(block->insts));
	return IsBranchInst(Inst);
}

#define CHECK_KIND(NODE, KIND) ((KIND *) check_kind(NODE, IR_TYPE_##KIND))
#define ToINode(NODE) (&(NODE)->base)
#define Block_IsVisited(BB) INode_getRangeBegin(&(BB)->base)
#define Block_SetVisited(BB, BOOLVAL) INode_setRangeBegin(&(BB)->base, BOOLVAL)

static inline unsigned GetNodeId(INode *Node)
{
	IUpdate *Inst;
	if((Inst = CHECK_KIND(Node, IUpdate)) != 0) {
		if(Inst->LHS->Op == LocalScope)
			return (((INode *)Inst->LHS)->Id);
	}
	return (Node)->Id;
}
#define NODE_ID(NODE) GetNodeId((INode *)NODE)

static inline bool IsLocalVariable(INode *Node)
{
	IField *Inst;
	if((Inst = CHECK_KIND(Node, IField)) != 0) {
		return (Inst->Op == LocalScope);
	}
	return false;
}

/* ------------------------------------------------------------------------- */
/* Optimizer API */
extern void IRBuilder_RemoveRedundantConstants(FuelIRBuilder *builder);
extern void IRBuilder_RemoveTrivialCondBranch(FuelIRBuilder *builder);
extern void IRBuilder_SimplifyStdCall(FuelIRBuilder *builder);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* end of include guard */
