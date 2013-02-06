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
 *  [Field]              |              |
 *                     [Branch]       [New]
 *                     [Test]         [Call]
 *                     [Return]       [Cond]
 *                     [Throw]        [Update]
 *                     [Try]          [Function]
 *                     [Yield]        [UnaryInst]
 *                     [Jump]         [BinaryInst]
 *                                    [PHI]
 */

#define IR_LIST(OP)\
	OP(IConstant)\
	OP(IArgument)\
	OP(IField)\
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
	OP(IBinary)\
	OP(IPHI)

enum IRType {
	TYPE_IR_ERROR = 0,
	TYPE_IR_Block,
#define IR_TYPE_DECL(X) IR_TYPE_##X,
	IR_LIST(IR_TYPE_DECL)
#undef IR_TYPE_DECL
	TYPE_IR_MAX
};

/*$ Node */
#define STRUCT_INODE\
	enum IRType Kind;\
	enum TypeId Type;\
	unsigned Id:27;\
	unsigned Marked:1;\
	unsigned Unused:1;\
	unsigned Visited:1;\
	unsigned Removed:1;\
	unsigned Evaled:1;\
	unsigned Offset

struct INode {
	STRUCT_INODE;
	struct INode *Parent;
};

static inline void INode_setParent(struct INode *Node, struct INode *Parent)
{
	Node->Parent = Parent;
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
	kNameSpace    *ns;
} IMethod;

/*$ BasicBlock */
struct Block {
	STRUCT_INODE;
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

enum ScopeOp {
	GlobalScope, EnvScope, FieldScope, LocalScope
};

/*$ Value Field { FieldOp } */
typedef struct IField {
	INode base;
	union {
		INode *Node; /* if Op != LocalScope */
		unsigned Id; /* if Op == LocalScope */
	};
	enum ScopeOp Op;
	unsigned FieldIndex;
	uintptr_t Hash;/* used at Compiling phase */
} IField;

enum ConditionalOp {
	LogicalOr, LogicalAnd
};

/*$ Any  New { Method, [Inst] } */
typedef struct INew {
	INode base;
	unsigned Flags;
	uintptr_t Conf;
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
	INode *Func;
	INode *Env;
	INode *Method;
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

/*$ void Jump { Block } */
typedef struct IJump {
	INode base;
	Block *TargetBlock;
} IJump;

/*$ void Throw { Inst } */
typedef struct IThrow {
	INode base;
	INode *Val;
	int exception;
	int fault;
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

/*$ T Unary { Node } */
enum UnaryOp {
	Not/*!*/, Neg/*-*/, Box,
	UnaryOp_NotFound = -1
};

typedef struct IUnary {
	INode base;
	enum UnaryOp Op;
	uintptr_t uline;
	INode *Node;
} IUnary;

/*$ T Binary { Node, Node } */
enum BinaryOp {
	Add, Sub, Mul, Div, Mod,
	LShift, RShift, And, Or, Xor,
	Eq, Nq, Gt, Ge, Lt, Le,
	BinaryOp_NotFound = -1
} Op;

typedef struct IBinary {
	INode base;
	enum BinaryOp Op;
	uintptr_t uline;
	INode *LHS;
	INode *RHS;
} IBinary;

/*$ T PHI { INode *Field, [IUpdate, ...]} */
typedef struct IPHI {
	INode base;
	INode *Val;
	ARRAY(INodePtr) Args;
} IPHI;

typedef union INodeImpl {
	INode Node;
	Block Blk;
#define IR_TYPE_DECL(X) X _##X;
	IR_LIST(IR_TYPE_DECL)
#undef IR_TYPE_DECL
} INodeImpl;

typedef struct IRBuilderAPI {
	void (*Fn_Init)(FuelIRBuilder *builder);
	void (*Fn_Exit)(FuelIRBuilder *builder);
	void (*Fn_Dispose)(INode *Node);
	/* Create BasicBlock Node */
	Block *(*newBlock)(FuelIRBuilder *builder);
	/* Create Literal Node */
	IConstant *(*newConstant)(FuelIRBuilder *builder, enum TypeId typeId, SValue value);
	IArgument *(*newArgument)(FuelIRBuilder *builder, unsigned Index);
	IField    *(*newField)(FuelIRBuilder *builder, enum ScopeOp Op, enum TypeId Type, unsigned Index, INode *Node, unsigned FieldIdx);
	/* Create IInstruction Node */
	INew      *(*newNew)(FuelIRBuilder *builder, uintptr_t Conf, enum TypeId Type);
	ICall     *(*newCall)(FuelIRBuilder *builder, enum CallOp Op, uintptr_t uline);
	IFunction *(*newFunction)(FuelIRBuilder *builder);
	IUpdate   *(*newUpdate)(FuelIRBuilder *builder, IField *Sym, INode *RHS);
	IBranch   *(*newBranch)(FuelIRBuilder *builder, INode *cond, Block *thenStmt, Block *elseEtmt);
	ITest     *(*newTest)(FuelIRBuilder *builder, enum TestOp Op, IField *Field, Block *TargetBlock);
	IReturn   *(*newReturn)(FuelIRBuilder *builder, INode *expr);
	IJump     *(*newJump)(FuelIRBuilder *builder, Block *TargetBlock);
	IThrow    *(*newThrow)(FuelIRBuilder *builder, INode *Exception, int exception, int fault, uintptr_t uline);
	ITry      *(*newTry)(FuelIRBuilder *builder, Block *tryBlock, Block *finallyBlock);
	IYield    *(*newYield)(FuelIRBuilder *builder, INode *INode);
	IUnary    *(*newUnary)(FuelIRBuilder *builder, enum UnaryOp Op, INode *INode, uintptr_t uline);
	IBinary   *(*newBinary)(FuelIRBuilder *builder, enum BinaryOp Op, INode *LHS, INode *RHS, uintptr_t uline);
	IPHI      *(*newPHI)(FuelIRBuilder *builder, INode *Val);
} IRBuilderAPI;

struct FuelIRBuilder {
	const IRBuilderAPI *API;
	unsigned LastNodeId;
	unsigned LocalId;
	Block *Current;
	union {
		int   InstructionId; /* used by InstructionDumper */
		void *ByteCode;
	};
	MemoryPool mp;
	ARRAY(INodePtr) Stack;
	ARRAY(INodePtr) LocalVar;
	ARRAY(BlockPtr) Blocks;
	KonohaContext *Context;
	IMethod *Method;
	struct KClassTable {
		KClass *cMath;
		KClass *cBytes;
	} ClassInfo;
};

void IRBuilder_Init(FuelIRBuilder *builder, KonohaContext *kctx, kNameSpace *ns);
void IRBuilder_Exit(FuelIRBuilder *builder);
union ByteCode *IRBuilder_Compile(FuelIRBuilder *builder, IMethod *Mtd, int option, bool *JITCompiled);
union ByteCode *IRBuilder_CompileToLLVMIR(FuelIRBuilder *builder, IMethod *Mtd, int option);
INode *IRBuilder_FindLocalVarByHash(FuelIRBuilder *builder, enum TypeId type, uintptr_t Hash);

void IRBuilder_add(FuelIRBuilder *builder, INode *Stmt);
void INewInst_addParam(INew *Inst, INode *Param);
void CallInst_addParam(ICall *Inst, INode *Param);
void PHIInst_addParam(IPHI *Inst, IUpdate *Val);

/* utility */
static inline void IField_setHash(IField *Inst, uintptr_t Hash)
{
	Inst->Hash = Hash;
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

extern INode *FoldInst(FuelIRBuilder *builder, INode *Node);
#define CHECK_KIND(NODE, KIND) ((KIND *) check_kind(((INode *)NODE), IR_TYPE_##KIND))
#define ToINode(NODE) (&(NODE)->base)
#define Block_IsVisited(BB) (BB->Visited)
#define Block_SetVisited(BB, BOOLVAL) ((BB)->Visited = BOOLVAL)
#define Block_SetRemoved(BB) ((BB)->Removed = 1)

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

static inline INode *CreateField(FuelIRBuilder *builder, enum ScopeOp Op, enum TypeId type, INode *Node, unsigned FieldIdx)
{
	assert(Op != LocalScope);
	Node = (INode *) builder->API->newField(builder, Op, type, 0, Node, FieldIdx);
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateUpdate(FuelIRBuilder *builder, INode *Field, INode *Inst)
{
	INode *Node = (INode *) builder->API->newUpdate(builder, (IField *) Field, Inst);
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateJump(FuelIRBuilder *builder, Block *Block)
{
	INode *Node = (INode *) builder->API->newJump(builder, Block);
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateBranch(FuelIRBuilder *builder, INode *Val, Block *ThenBB, Block *ElseBB)
{
	INode *Node = (INode *) builder->API->newBranch(builder, Val, ThenBB, ElseBB);
	IConstant *C;
	if((C = CHECK_KIND(Val, IConstant))) {
		Block *BB = (C->Value.bits) ? ThenBB : ElseBB;
		return CreateJump(builder, BB);
	}
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateReturn(FuelIRBuilder *builder, INode *Val)
{
	INode *Node = (INode *) builder->API->newReturn(builder, Val);
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateThrow(FuelIRBuilder *builder, INode *Val, int exception, int fault, uintptr_t uline)
{
	INode *Node = (INode *) builder->API->newThrow(builder, Val, exception, fault, uline);
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateNew(FuelIRBuilder *builder, uintptr_t conf, enum TypeId type)
{
	INode *Node = (INode *) builder->API->newNew(builder, conf, type);
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateICall(FuelIRBuilder *builder, enum TypeId Type, enum CallOp Op, uintptr_t uline, INode **Params, unsigned ParamSize)
{
	INode *Node = (INode *) builder->API->newCall(builder, Op, uline);
	INode_setType(Node, Type);
	unsigned i;
	for (i = 0; i < ParamSize; i++) {
		CallInst_addParam((ICall *)Node, Params[i]);
	}
	INode *C = FoldInst(builder, Node);
	if(C != NULL)
		Node = C;
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateIFunction(FuelIRBuilder *builder, enum TypeId Type, INode *Func, INode *EnvObj, INode *MtdObj)
{
	INode *Node = (INode *) builder->API->newFunction(builder);
	INode_setType(Node, Type);
	((IFunction *)Node)->Func   = Func;
	((IFunction *)Node)->Env    = EnvObj;
	((IFunction *)Node)->Method = MtdObj;
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateUnaryInst(FuelIRBuilder *builder, enum UnaryOp Op, INode *Val, uintptr_t uline)
{
	INode *Node = (INode *) builder->API->newUnary(builder, Op, Val, uline);
	INode *C = FoldInst(builder, Node);
	if(C != NULL)
		Node = C;
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreateBinaryInst(FuelIRBuilder *builder, enum BinaryOp Op, INode *LHS, INode *RHS, uintptr_t uline)
{
	INode *Node = (INode *) builder->API->newBinary(builder, Op, LHS, RHS, uline);
	INode *C = FoldInst(builder, Node);
	if(C != NULL)
		Node = C;
	IRBuilder_add(builder, Node);
	return Node;
}

static inline INode *CreatePHI(FuelIRBuilder *builder, INode *Val)
{
	INode *Node = (INode *) builder->API->newPHI(builder, Val);
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

extern void IRBuilder_Optimize(FuelIRBuilder *builder, Block *EntryBB, bool);
void FuelVM_Recompile(KonohaContext *kctx, kMethod *mtd);
bool FuelVM_HasOptimizedCode(kMethod *mtd);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* end of include guard */
