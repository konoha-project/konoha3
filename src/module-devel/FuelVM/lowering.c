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

#include <stdio.h>
#include "codegen.h"
#include "visitor.h"
#include "FuelVM.h"
#include "RegAlloc.h"

#define DUMP_IR      1
#define DUMP_FUEL_IR 1
#define DEBUG 1

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------- */
DEF_ARRAY_OP_NOPOINTER(NodePtr);
DEF_ARRAY_OP_NOPOINTER(INodePtr);

#define debug(FMT, ...) do {\
	if(DEBUG) {\
		fprintf(stderr, FMT, ## __VA_ARGS__);\
	}\
} while(0)

static void Node_addPred(Node *BB, Node *Pred)
{
	ARRAY_add(NodePtr, &BB->preds, Pred);
}

static void Node_addSucc(Node *BB, Node *Succ)
{
	ARRAY_add(NodePtr, &BB->succs, Succ);
}

/* ------------------------------------------------------------------------- */

static void LinkNodes(FuelIRBuilder *builder, Node *BB, bool IsVisited)
{
	while(Node_IsVisited(BB) != IsVisited) {
		Node_SetVisited(BB, IsVisited);
		INode *Inst = *(ARRAY_last(BB->insts));
		enum IRType Kind = Inst->Kind;
		assert(IsBranchInst(Inst));
		ARRAY_add(NodePtr, &builder->Nodes, BB);
		switch(Kind) {
		case IR_TYPE_IBranch:
			Node_addSucc(BB, ((IBranch *)Inst)->ThenBB);
			Node_addSucc(BB, ((IBranch *)Inst)->ElseBB);
			Node_addPred(((IBranch *)Inst)->ThenBB, BB);
			Node_addPred(((IBranch *)Inst)->ElseBB, BB);
			LinkNodes(builder, ((IBranch *)Inst)->ThenBB, IsVisited);
			BB = ((IBranch *)Inst)->ElseBB;
			break;
		case IR_TYPE_ITest:
			Node_addSucc(BB, ((ITest *)Inst)->TargetNode);
			Node_addPred(((ITest *)Inst)->TargetNode, BB);
			BB = ((ITest *)Inst)->TargetNode;
			break;
		case IR_TYPE_IReturn:
		case IR_TYPE_IThrow:
		case IR_TYPE_IYield:
			break;
		case IR_TYPE_ITry:
			assert(0 && "TODO");
			break;
		case IR_TYPE_IJump:
			Node_addSucc(BB, ((IJump *)Inst)->TargetNode);
			Node_addPred(((IJump *)Inst)->TargetNode, BB);
			BB = ((IJump *)Inst)->TargetNode;
			break;
		default:
			assert(0 && "unreachable");
			break;
		}
	}
}

static void IRBuilder_LinkNodes(FuelIRBuilder *builder, Node *BB, bool IsVisited)
{
	if(ARRAY_size(builder->Nodes) > 0) {
		NodePtr *x, *e;
		FOR_EACH_ARRAY(builder->Nodes, x, e) {
			ARRAY_clear((*x)->preds);
			ARRAY_clear((*x)->succs);
		}
		ARRAY_clear(builder->Nodes);
	}
	LinkNodes(builder, BB, IsVisited);
}

static void IRBuilder_SimplifyCFG(FuelIRBuilder *builder)
{
	/* Simplify Control Flow
	 * [Case1]
	 *     [Before]            [After]
	 * BB0: IUpdate $a $b | BB0: IUpdate $a $b
	 *      IJump BB1     |      iAdd $c $d
	 * BB1: iAdd $c $d    |      ...
	 *      ...           |
	 */
	NodePtr *x, *e;
	FOR_EACH_ARRAY(builder->Nodes, x, e) {
		if(ARRAY_size((*x)->succs) == 1) {
			Node *Target = ARRAY_get(NodePtr, &((*x)->succs), 0);
			if(ARRAY_size(Target->preds) > 1) {
				return;
			}
			if(*x == Target)
				return;
			INodePtr *Inst, *End;
			INode *LastInst = *(ARRAY_last((*x)->insts));
			assert(CHECK_KIND(LastInst, IJump));
			(*x)->insts.size -= 1;
			FOR_EACH_ARRAY(Target->insts, Inst, End) {
				ARRAY_add(INodePtr, &(*x)->insts, *Inst);
			}
			ARRAY_clear(Target->insts);
			ARRAY_clear(Target->succs);
			ARRAY_clear((*x)->succs);
		}
	}
}

static void IRBuilder_RemoveInstructionAfterBranchInst(FuelIRBuilder *builder)
{
	NodePtr *x, *e;
	FOR_EACH_ARRAY(builder->Nodes, x, e) {
		INodePtr *Inst;
		INodePtr *LastInst = (ARRAY_last((*x)->insts));
		unsigned i;
		FOR_EACH_ARRAY_((*x)->insts, Inst, i) {
			if(IsBranchInst(*Inst) && Inst != LastInst) {
				ARRAY_size((*x)->insts) =  i + 1;
			}
		}
	}
}

static void IRBuilder_FlattenICond(FuelIRBuilder *builder)
{
	NodePtr *x, *e;
	FOR_EACH_ARRAY(builder->Nodes, x, e) {
		INodePtr *Inst;
		INodePtr *LastInst = (ARRAY_last((*x)->insts));
		unsigned i;
		FOR_EACH_ARRAY_((*x)->insts, Inst, i) {
			ICond *Cond;
			if((Cond = CHECK_KIND(*Inst, ICond)) != 0) {
				IBranch *Branch;
				if((Branch = CHECK_KIND(*LastInst, IBranch)) != 0) {
					if(Cond == (ICond *) Branch->Cond)
						CondInst_SetBranchInst(Cond, Branch);
				}
			}
		}
	}
}


static void CopyIfNodeHasSingleInst(Node *BB, INode *LastInst, Node *Node)
{
	if(ARRAY_size(Node->insts) == 1) {
		INode *Inst = ARRAY_get(INodePtr, &Node->insts, 0);
		if(CHECK_KIND(LastInst, IJump) && CHECK_KIND(Inst, IReturn)) {
			//debug("Jump %d=>%d\n", BB->base.Id, Node->base.Id);
			memcpy(LastInst, Inst, sizeof(INodeImpl));
		}
		else if(CHECK_KIND(LastInst, IBranch) && CHECK_KIND(Inst, IJump)) {
			//debug("Branch %d=>%d\n", BB->base.Id, Node->base.Id);
			IBranch *Branch = (IBranch *) LastInst;
			IJump   *Jump   = (IJump *) Inst;
			if(Branch->ThenBB == Node) {
				Branch->ThenBB = Jump->TargetNode;
			} else {
				Branch->ElseBB = Jump->TargetNode;
			}
		}
	}
}

static void IRBuilder_RemoveIndirectJumpNode(FuelIRBuilder *builder, Node *BB, bool IsVisited)
{
	/* Remove Node that contain only one Jump Instruction
	 * [Case1]
	 *     [Before]            [After]
	 * BB0: IUpdate $a $b | BB0: IUpdate $a $b
	 *      IJump BB1     |      IJump BB2
	 * BB1: IJump BB2     |
	 * BB2: ...           | BB2: ...
	 * [Case2]
	 *     [Before]            [After]
	 * BB0: Branch BB1 BB2| BB0: Branch BB3 BB2
	 * BB1: IJump BB3     |
	 * BB2: IUpdate $a $c | BB2: IUpdate $a $c
	 *      IJump BB3     |      IJump BB3
	 * BB3: ...           | BB3: ...
	 */
	while(Node_IsVisited(BB) != IsVisited) {
		Node_SetVisited(BB, IsVisited);
		INode *Inst = *(ARRAY_last(BB->insts));
		assert(IsBranchInst(Inst));
		IBranch *Branch;
		ITest   *Test;
		IJump   *Jump;
		if((Branch = CHECK_KIND(Inst, IBranch)) != 0) {
			CopyIfNodeHasSingleInst(BB, ToINode(Branch), ((IBranch *)Inst)->ThenBB);
			CopyIfNodeHasSingleInst(BB, ToINode(Branch), ((IBranch *)Inst)->ElseBB);
			IRBuilder_RemoveIndirectJumpNode(builder, ((IBranch *)Inst)->ThenBB, IsVisited);
			BB = ((IBranch *)Inst)->ElseBB;
		}
		else if((Jump = CHECK_KIND(Inst, IJump)) != 0) {
			Node *NextBB = ((IJump *)Inst)->TargetNode;
			CopyIfNodeHasSingleInst(BB, ToINode(Jump), ((IJump *)Inst)->TargetNode);
			BB = NextBB;
		}
		else if((Test = CHECK_KIND(Inst, ITest)) != 0) {
			BB = ((ITest *)Inst)->TargetNode;
		}
		else if(CHECK_KIND(Inst, ITry) != 0) {
			assert(0 && "TODO");
		}
	}
}

static inline void INode_SetMarked(INode *Node)
{
	Node->Marked = 1;
}

static inline bool INode_IsMarked(INode *Node)
{
	return Node->Marked;
}

static void TraceNode1(INode *Node)
{
#define CASE(KIND) case IR_TYPE_##KIND:
	switch(Node->Kind) {
		case IR_TYPE_IConstant:
		case IR_TYPE_IArgument:
		case IR_TYPE_ILabel:
			break;
			CASE(IField) {
				IField *Inst = (IField *) Node;
				switch(Inst->Op) {
					case GlobalScope:
					case EnvScope:
					case FieldScope:
						INode_SetMarked(Node);
						INode_SetMarked(Inst->Node);
						break;
					case LocalScope:
						break;
				}
				break;
			}
			CASE(ICond) {
				ICond *Inst = (ICond *) Node;
				INodePtr *x, *e;
				FOR_EACH_ARRAY(Inst->Insts, x, e) {
					INode_SetMarked(*x);
				}
				break;
			}
			CASE(INew) {
				INode_SetMarked(Node);
				break;
			}
			CASE(ICall) {
				ICall *Inst = (ICall *) Node;
				INode_SetMarked(Node);
				INodePtr *x;
				unsigned i = 0;
				FOR_EACH_ARRAY_(Inst->Params, x, i) {
					if(i != 0) {
						INode_SetMarked(*x);
					}
				}
				break;
			}
			CASE(IFunction) {
				IFunction *Inst = (IFunction *) Node;
				INodePtr *x, *e;
				FOR_EACH_ARRAY(Inst->Env, x, e) {
					INode_SetMarked(*x);
				}
				break;
			}
			CASE(IUpdate) {
				/* IUpdate Inst is Marked at TraceNode2() */
				break;
			}
			CASE(IBranch) {
				INode_SetMarked(Node);
				INode_SetMarked(((IBranch *) Node)->Cond);
				break;
			}
			CASE(ITest) {
				INode_SetMarked(Node);
				INode_SetMarked((INode *)((ITest *) Node)->Value);
				assert(0 && "TODO");
				break;
			}
			CASE(IReturn) {
				IReturn *Inst = (IReturn *) Node;
				INode_SetMarked(Node);
				if(Inst->Inst) {
					INode_SetMarked(Inst->Inst);
				}
				break;
			}
			CASE(IJump) {
				INode_SetMarked(Node);
				break;
			}
			CASE(IThrow) {
				IThrow *Inst = (IThrow *) Node;
				INode_SetMarked(Node);
				INode_SetMarked((INode *) Inst->Val);
				break;
			}
			CASE(ITry) {
				INode_SetMarked(Node);
				assert(0 && "TODO");
				break;
			}
			CASE(IYield) {
				INode_SetMarked(Node);
				INode_SetMarked(((IYield *) Node)->Value);
				break;
			}
			CASE(IUnary) {
				INode_SetMarked(((IUnary *) Node)->Node);
				break;
			}
			CASE(IBinary) {
				IBinary *Inst  = (IBinary *) Node;
				INode_SetMarked(Inst->LHS);
				INode_SetMarked(Inst->RHS);
				break;
			}
		default:
			assert(0 && "unreachable");
#undef CASE
	}
}

static void TraceNode2(INode *Node)
{
	IUpdate *Inst;
	if(INode_IsMarked(Node)) {
		return;
	}
	if((Inst = CHECK_KIND(Node, IUpdate)) == 0) {
		return;
	}
	IField *LHS = Inst->LHS;
	switch(LHS->Op) {
		case GlobalScope:
		case EnvScope:
			assert(0 && "TODO");
			break;
		case FieldScope:
		case LocalScope:
			if(INode_IsMarked((INode *)LHS)) {
				INode_SetMarked(Node);
				INode_SetMarked(Inst->RHS);
			}
			break;
	}
}

static void IRBuilder_RemoveUnusedVariable(FuelIRBuilder *builder)
{
	NodePtr *x, *e;
	FOR_EACH_ARRAY(builder->Nodes, x, e) {
		INodePtr *Inst, *End;
		FOR_EACH_ARRAY((*x)->insts, Inst, End) {
			TraceNode1(*Inst);
		}
	}

	FOR_EACH_ARRAY(builder->Nodes, x, e) {
		INodePtr *Inst, *End;
		FOR_EACH_ARRAY((*x)->insts, Inst, End) {
			TraceNode2(*Inst);
		}
	}

	ARRAY(INodePtr) Insts;
	ARRAY_init(INodePtr, &Insts, 32);
	FOR_EACH_ARRAY(builder->Nodes, x, e) {
		INodePtr *Inst, *End;
		FOR_EACH_ARRAY((*x)->insts, Inst, End) {
			if(INode_IsMarked(*Inst)) {
				ARRAY_add(INodePtr, &Insts, *Inst);
			}
		}
		ARRAY(INodePtr) *list = &(*x)->insts;
		memcpy(list->list, Insts.list, sizeof(INodePtr)*ARRAY_size(Insts));
		list->size = ARRAY_size(Insts);
		Insts.size = 0;
	}
	ARRAY_dispose(INodePtr, &Insts);
}

/* ------------------------------------------------------------------------- */
/* [Dump IR] */
#ifdef DUMP_IR

static const char *OPTEXT[] = {
	"Node",
#define IR_TEXT_DECL(X) #X,
	IR_LIST(IR_TEXT_DECL)
#undef IR_TEXT_DECL
};

#define DEBUG_TYPE_ID 1

static const char *Type2String(enum TypeId Type)
{
	switch(Type) {
#define CASE(X) case TYPE_##X: return #X
		CASE(void);
		CASE(boolean);
		CASE(int);
		CASE(float);
		CASE(Object);
		CASE(String);
		CASE(Function);
		CASE(Array);
		CASE(Method);
		CASE(NameSpace);
		CASE(Any);
#undef CASE
	}
#ifdef DEBUG_TYPE_ID
	static char buf[128];
	snprintf(buf, 128, "%d", Type);
	return buf;
#else
	return "Undef";
#endif
}

static void Node_DumpName(Node *BB)
{
	debug("  Node $%d : preds=[", BB->base.Id);
	NodePtr *x, *e;
	FOR_EACH_ARRAY(BB->preds, x, e) {
		if(x != ARRAY_n(BB->preds, 0)) {
			debug(", ");
		}
		debug("$%d", (*x)->base.Id);
	}
	debug("], succs=[");
	FOR_EACH_ARRAY(BB->succs, x, e) {
		if(x != ARRAY_n(BB->succs, 0)) {
			debug(", ");
		}
		debug("$%d", (*x)->base.Id);
	}
	debug("]\n");
}

static void IRBuilder_DumpLocalVariable(FuelIRBuilder *builder)
{
	INodePtr *x;
	debug("  Local [");
	unsigned i = 0;
	FOR_EACH_ARRAY_(builder->LocalVar, x, i) {
		INode *Node = *x;
		if(i != 0) {
			debug(", ");
		}
		debug("{Type:%s, Id:$%d}", Type2String(Node->Type), Node->Id);
	}
	debug("]\n");
}

static void printNode(INode *Inst)
{
	int id = (Inst) ? (int)Inst->Id : -1;
	debug("$%d", id);
}

static void printHeader(FuelIRBuilder *builder, INode *Inst, const char *Tag)
{
	const char *Padding = " ";
	if(Tag == 0) {
		Tag = "";
		Padding = "";
	}
	debug("    %02d: $%02d = %s%s%s [", builder->InstructionId++, Inst->Id,
			OPTEXT[Inst->Kind], Padding, Tag);
}

static void Dump_visitList(Visitor *visitor, INode *Inst, const char *Tag, unsigned ElmSize, INodePtr *Insts)
{
	printHeader((FuelIRBuilder *)visitor->Context, Inst, Tag);
	INodePtr *x = Insts, *e = Insts + ElmSize;
	while(x != e) {
		if(x != Insts) {
			debug(", ");
		}
		printNode(*x++);
	}
	debug("]\n");
}

static void Dump_visitValue(Visitor *visitor, INode *Node, const char *Tag, SValue Val)
{
	printHeader((FuelIRBuilder *)visitor->Context, Node, Tag);
	IField *Inst;
	if((Inst = CHECK_KIND(Node, IField)) != 0 && Inst->Op != LocalScope) {
		printNode(Inst->Node);
		debug(", Type:%s, %d]\n", Type2String(Node->Type), Inst->FieldIndex);
	} else {
		debug("Type:%s, 0x%llx]\n", Type2String(Node->Type), Val.bits);
	}
}
#endif

static void IRBuilder_DumpFunction(FuelIRBuilder *builder)
{
#ifdef DUMP_IR
	Visitor DumpVisitor = {
		(void *)builder,
		visitElement,
		Dump_visitList,
		Dump_visitValue
	};

	debug("Function {\n");
	IRBuilder_DumpLocalVariable(builder);
	NodePtr *x, *e;
	FOR_EACH_ARRAY(builder->Nodes, x, e) {
		Node_DumpName(*x);
		INodePtr *Inst, *End;
		FOR_EACH_ARRAY((*x)->insts, Inst, End) {
			visitINode(&DumpVisitor, *Inst);
		}
	}
	debug("}\n");
#endif
}

/* ------------------------------------------------------------------------- */
/* [Lowering] */

#ifdef DEBUG_BYTECODE
#define ASM_ADDR ,0
#else
#define ASM_ADDR
#endif

#define EMIT_LIR(writer, OPCODE, ...) do {\
	OP##OPCODE tmp = {{OPCODE_##OPCODE ASM_ADDR}, ## __VA_ARGS__};\
	EmitByteCode(writer, (ByteCode *) &tmp);\
} while(0)

DEF_ARRAY_STRUCT(ByteCode);
DEF_ARRAY_T(ByteCode);
DEF_ARRAY_OP(ByteCode);
typedef ARRAY(ByteCode) ByteCodeList;

typedef struct ByteCodeWriter {
	Visitor visitor;
	ByteCodeList *ByteCode;
	Node *Current;
	RegisterAllocator RegAllocator;
} ByteCodeWriter;

static void EmitByteCode(ByteCodeWriter *writer, ByteCode *Code)
{
	ByteCodeList *CodeBuf = (ByteCodeList *) writer->ByteCode;
	if(CodeBuf->size > 0) {
		ByteCode *prev = (ByteCode *) ARRAYp_last(CodeBuf);
		if(GetOpcode(prev) == OPCODE_LoadArgumentI || GetOpcode(prev) == OPCODE_LoadArgumentO) {
			OPLoadArgumentO *Inst = (OPLoadArgumentO *) prev;
			OPStoreLocal *NewInst = (OPStoreLocal *) Code;
			if(GetOpcode(Code) == OPCODE_StoreLocal && NewInst->Src == Inst->Dst) {
				/*
				 *       [Before]         |     [After]
				 * LoadArgument RegX Arg2 | LoadArgument RegY Arg2
				 * StoreLocal   RegY RegX |
				 */
				Inst->Dst = NewInst->Dst;
				return;
			}
		}
	}
	ARRAY_add(ByteCode, CodeBuf, Code);
}

static void EmitUnaryInst(ByteCodeWriter *writer, enum UnaryOp Op, unsigned Dst, unsigned Src)
{
	switch(Op) {
#define CASE(X) case X: EMIT_LIR(writer, X, Dst, Src); break
		CASE(Not); CASE(Neg);
		default:
		assert(0 && "unreachable");
		break;
#undef CASE
	}
}

static void EmitBinaryInst(ByteCodeWriter *writer, enum BinaryOp Op, enum TypeId Type, unsigned Dst, unsigned Left, unsigned Right)
{
	if(Type == TYPE_int) {
		switch(Op) {
#define CASE(X) case X: EMIT_LIR(writer, X, Dst, Left, Right); return
			CASE(Add); CASE(Sub); CASE(Mul); CASE(Div); CASE(Mod);
			CASE(LShift); CASE(RShift); CASE(And); CASE(Or); CASE(Xor);
			CASE(Eq); CASE(Nq); CASE(Gt); CASE(Ge); CASE(Lt); CASE(Le);
			default:
			assert(0 && "unreachable");
			break;
#undef CASE
		}
	} else if(Type == TYPE_float) {
		switch(Op) {
#define CASE(X) case X: EMIT_LIR(writer, F##X, Dst, Left, Right); return
			CASE(Add); CASE(Sub); CASE(Mul); CASE(Div);
			CASE(Eq); CASE(Nq); CASE(Gt); CASE(Ge); CASE(Lt); CASE(Le);
			default:
			assert(0 && "unreachable");
			break;
#undef CASE
		}
	}
	assert(0 && "unreachable");
}

static inline bool IsLocalOrField(INode *Node)
{
	IField *Inst;
	if((Inst = CHECK_KIND(Node, IField)) != 0) {
		return (Inst->Op == LocalScope || Inst->Op == FieldScope);
	}
	return false;
}

static void DeallocateWithoutLocalVar(ByteCodeWriter *writer)
{
	Node *cur = writer->Current;
	INodePtr *x, *e;
	FOR_EACH_ARRAY(cur->insts, x, e) {
		INode *Node = *x;
		ICond *Inst;
		if(CHECK_KIND(Node, IUpdate) != 0) {
			continue;
		}
		if((Inst = CHECK_KIND(Node, ICond)) != 0 && Inst->Branch != 0) {
			continue;
		}
		if(IsLocalOrField(Node))
			continue;
		if(IsBranchInst(Node))
			continue;
		Register_Deallocate(&writer->RegAllocator, NODE_ID(Node));
	}
}

static inline unsigned RegAllocate(ByteCodeWriter *writer, unsigned Id)
{
	return Register_Allocate(&writer->RegAllocator, Id);
}

#if 1
#define DUMP_REGISTER_NODE(NODE, WRITER)
#else
#define DUMP_REGISTER_NODE(NODE, WRITER) do {\
	debug("ID:%d\n", (NODE)->Id);\
	DumpRegister(WRITER);\
} while(0)

static void DumpRegister(ByteCodeWriter *writer)
{
	int i;
#define BITS (sizeof(uintptr_t)*8)
	for (i = BITS-1; i >= 0; i--) {
		if(i %10 == 0) {
			debug("%d", i / 10);
		} else {
			debug(" ");
		}
	}
	debug("\n");
	for (i = BITS-1; i >= 0; i--) {
		debug("%d", i % 10);
	}
	debug("\n");
	for(i = BITS - 1; i >= 0; i--) {
		RegisterAllocator *ra = &writer->RegAllocator;
		bool assigned = BitMap_get(&ra->AllocatedVariable, i);
		debug("%d", assigned);
	}
	debug("\n\n");
}
#endif

static void EmitCondBranch(ByteCodeWriter *writer, IBranch *Inst, ICond *Cond, int IsTopDecl)
{
	INodePtr *x, *e;
	Node *ThenBB, *ElseBB;
	unsigned Src;

	if(Cond->Op == LogicalOr) {
		ThenBB = Inst->ThenBB; ElseBB = Inst->ElseBB;
	} else {
		ThenBB = Inst->ElseBB; ElseBB = Inst->ThenBB;
	}

	FOR_EACH_ARRAY(Cond->Insts, x, e) {
		if(CHECK_KIND(*x, ICond) != 0) {
			EmitCondBranch(writer, Inst, (ICond *) *x, 0);
			continue;
		}
		assert(Register_FindById(&writer->RegAllocator, NODE_ID(*x), &Src) == true);
		if(Cond->Op == LogicalOr) {
			EMIT_LIR(writer, CondBrTrue, Src, ThenBB);
		} else {
			EMIT_LIR(writer, CondBrFalse, Src, ThenBB);
		}
	}

	if(IsTopDecl == true) {
		EMIT_LIR(writer, Jump, ElseBB);
	}
}

#define CASE(KIND) case IR_TYPE_##KIND:
static void EmitNode(ByteCodeWriter *writer, INode *Node)
{
	DUMP_REGISTER_NODE(Node, writer);
	switch(Node->Kind) {
		CASE(ICond) {
			ICond *Inst = (ICond *) Node;
			if(Inst->Branch == NULL) {
				INodePtr *x;
				unsigned i, LHS, RHS;
				unsigned Dst = RegAllocate(writer, NODE_ID(Inst));
				INodePtr *Inst0 = ARRAY_n(Inst->Insts, 0);
				assert(Register_FindById(&writer->RegAllocator, NODE_ID(*Inst0), &LHS) == true);
				FOR_EACH_ARRAY_(Inst->Insts, x, i) {
					if(i == 0)
						continue;
					assert(Register_FindById(&writer->RegAllocator, NODE_ID(*x), &RHS) == true);
					if(Inst->Op == LogicalOr) {
						EMIT_LIR(writer, Or, Dst, LHS, RHS);
					} else {
						EMIT_LIR(writer, And, Dst, LHS, RHS);
					}
					LHS = Dst;
				}
			}
			break;
		}
		CASE(INew) {
			INew *Inst = (INew *) Node;
			unsigned Dst = RegAllocate(writer, NODE_ID(Inst));
			EMIT_LIR(writer, New, Dst, Inst->Conf, Inst->base.Type);
			break;
		}
		CASE(ICall) {
			ICall *Inst = (ICall *) Node;
			INode **MtdPtr = ARRAY_n(Inst->Params, 0);
			IConstant *Mtd = (IConstant *) *MtdPtr;
			*MtdPtr = 0;
			INodePtr *x, *e;
			FOR_EACH_ARRAY(Inst->Params, x, e) {
				if(*x == 0)
					continue;
				INode *Param = *x;
				unsigned Reg;
				assert(Register_FindById(&writer->RegAllocator, NODE_ID(Param), &Reg) == true);
				assert(Reg != REGISTER_UNDEFINED);
				if(IsUnBoxedType(Param->Type)) {
					EMIT_LIR(writer, PushI, Reg);
				} else {
					EMIT_LIR(writer, PushO, Reg);
				}
			}
			unsigned Dst = RegAllocate(writer, NODE_ID(Inst));
			unsigned IsUnboxed = Node->Type != TYPE_void ? IsUnBoxedType(Node->Type) : 0;
			EMIT_LIR(writer, Call, Dst, ARRAY_size(Inst->Params)-1,
					IsUnboxed, Mtd->Value.ptr, Inst->uline);
			break;
		}
		CASE(IFunction) {
			assert(0 && "TODO");
			break;
		}
		CASE(IUpdate) {
			IUpdate *Inst = (IUpdate *) Node;
			IField  *LHS  = Inst->LHS;
			unsigned Src = REGISTER_UNDEFINED;
			unsigned Dst;
			assert(Register_FindById(&writer->RegAllocator, NODE_ID(Inst->RHS), &Src) == true);
			assert(CHECK_KIND((INode *)LHS, IField));
			switch(LHS->Op) {
				case GlobalScope:
				case EnvScope:
					assert(0 && "TODO");
					break;
				case FieldScope: {
					INode *obj = LHS->Node;
					Dst = Register_FindByIdOrAllocate(&writer->RegAllocator, NODE_ID(obj));
					EMIT_LIR(writer, StoreField, Dst, LHS->FieldIndex, Src);
					break;
				}
				case LocalScope: {
					Dst = Register_FindByIdOrAllocate(&writer->RegAllocator, NODE_ID(LHS));
					EMIT_LIR(writer, StoreLocal, Dst, Src);
					break;
				}
			}
			break;
		}
		CASE(IBranch) {
			IBranch *Inst = (IBranch *) Node;
			ICond *Cond;
			unsigned Src = REGISTER_UNDEFINED;
			if((Cond = CHECK_KIND(Inst->Cond, ICond)) != 0) {
				assert(Cond->Branch != 0);
				EmitCondBranch(writer, Inst, Cond, true);
			} else {
				assert(Register_FindById(&writer->RegAllocator, NODE_ID(Inst->Cond), &Src) == true);
				EMIT_LIR(writer, CondBrTrue, Src, Inst->ThenBB);
				EMIT_LIR(writer, Jump, Inst->ElseBB);
			}
			DeallocateWithoutLocalVar(writer);
			break;
		}
		CASE(ITest) {
			assert(0 && "TODO");
			break;
		}
		CASE(IReturn) {
			IReturn *Inst = (IReturn *) Node;
			if(Inst->Inst) {
				unsigned Src = REGISTER_UNDEFINED;
				assert(Register_FindById(&writer->RegAllocator, NODE_ID(Inst->Inst), &Src) == true);
				if(IsUnBoxedType(Node->Type)) {
					EMIT_LIR(writer, ReturnI, Src);
				} else {
					EMIT_LIR(writer, ReturnO, Src);
				}
			} else {
				EMIT_LIR(writer, ReturnVoid);
			}
			DeallocateWithoutLocalVar(writer);
			break;
		}
		CASE(IJump) {
			IJump *Inst = (IJump *) Node;
			DeallocateWithoutLocalVar(writer);
			EMIT_LIR(writer, Jump, Inst->TargetNode);
			break;
		}
		CASE(IThrow) {
			IThrow *Inst = (IThrow *) Node;
			unsigned Src = REGISTER_UNDEFINED;
			assert(Register_FindById(&writer->RegAllocator, NODE_ID(Inst->Val), &Src) == true);
			EMIT_LIR(writer, Throw, Src, Inst->uline);
			//DeallocateWithoutLocalVar(writer);
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
			IUnary *Inst = (IUnary *) Node;
			unsigned Src, Dst;
			assert(Register_FindById(&writer->RegAllocator, NODE_ID(Inst->Node), &Src) == true);
			Dst = Register_FindByIdOrAllocate(&writer->RegAllocator, NODE_ID(Inst));
			EmitUnaryInst(writer, Inst->Op, Dst, Src);
			break;
		}
		CASE(IBinary) {
			IBinary *Inst = (IBinary *) Node;
			unsigned Left, Right;
			assert(Register_FindById(&writer->RegAllocator, NODE_ID(Inst->LHS), &Left) == true);
			assert(Register_FindById(&writer->RegAllocator, NODE_ID(Inst->RHS), &Right) == true);
			unsigned Dst = Register_FindByIdOrAllocate(&writer->RegAllocator, NODE_ID(Inst));
			EmitBinaryInst(writer, Inst->Op, Inst->LHS->Type, Dst, Left, Right);
			break;
		}
		default:
		assert(0 && "unreachable");
	}
}

static void CodeWriter_visitList(Visitor *visitor, INode *Inst, const char *Tag, unsigned ElmSize, INodePtr *Insts)
{
	EmitNode((ByteCodeWriter *) visitor, Inst);
}

static void CodeWriter_visitValue(Visitor *visitor, INode *Node, const char *Tag, SValue Val)
{
	ByteCodeWriter *writer = (ByteCodeWriter *) visitor;
	DUMP_REGISTER_NODE(Node, writer);
	RegisterAllocator *Allocator = &writer->RegAllocator;
	switch(Node->Kind) {
		CASE(IConstant) {
			if(Node->Type == KType_Method) {
				/*XXX(ide) Need to Load Method Constant ??? */
				return;
			}
			unsigned Dst = Register_FindByIdOrAllocate(Allocator, NODE_ID(Node));
			EMIT_LIR(writer, LoadConstant, Dst, Val);
			return;
		}
		CASE(IArgument) {
			unsigned Dst = Register_FindByIdOrAllocate(Allocator, NODE_ID(Node));
			if(IsUnBoxedType(Node->Type)) {
				EMIT_LIR(writer, LoadArgumentI, Dst, Val.ival);
			} else {
				EMIT_LIR(writer, LoadArgumentO, Dst, Val.ival);
			}
			return;
		}
		CASE(ILabel) {
			assert(0 && "unreachable");
			break;
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
					//unsigned Src = REGISTER_UNDEFINED;
					//assert(Register_FindById(Allocator, NODE_ID(Inst->Node), &Src) == true);
					//unsigned Dst = Register_FindByIdOrAllocate(Allocator, NODE_ID(Node));
					//EMIT_LIR(writer, LoadField, Dst, Src, Inst->FieldIndex);
					return;
				}
				case LocalScope:
					break;
			}
			break;
		}
		default:
		assert(0 && "unreachable");
	}
}
#undef CASE

static void ByteCodeWriter_SaveCurrentInstId(ByteCodeWriter *writer)
{
	Node *Current = writer->Current;
	unsigned Index = ARRAY_size(*writer->ByteCode);
	INode_setRangeEnd(ToINode(Current), Index);
}

static void ByteCode_Link(ARRAY(NodePtr) *blocks, ByteCode *code)
{
	ByteCode *pc = code, *end = code + ((OPThreadedCode *) code)->CodeSize;
	while(pc < end) {
		switch(GetOpcode(pc)) {
			case OPCODE_CondBrTrue:
			case OPCODE_CondBrFalse: {
				OPCondBrTrue *Inst  = (OPCondBrTrue *) pc;
				unsigned offset = INode_getRangeEnd(ToINode((Node *)Inst->Node));
				Inst->Node = (void *) (code + offset);
				break;
			}
			case OPCODE_Jump: {
				OPJump *Inst = (OPJump *) pc;
				unsigned offset = INode_getRangeEnd(ToINode((Node *)Inst->Node));
				Inst->Node = (void *) (code + offset);
				break;
			}
			case OPCODE_Try:
				assert(0 && "TODO");
				break;
			default:
				break;
		}
		pc++;
	}
}

static void ByteCode_free(KonohaContext *kctx, struct KVirtualCode *vcode)
{
	ByteCode *code = ((ByteCode *) vcode)-1;
	free(code);
}

static void ByteCode_write(KonohaContext *kctx, KBuffer *wb, struct KVirtualCode *vcode)
{
	ByteCode *code = (ByteCode *) vcode;
	ByteCode_Dump(code);
}

static struct KVirtualCodeAPI fuelvm_api = {
	ByteCode_free, ByteCode_write
};

static ByteCode *IRBuilder_Lowering(FuelIRBuilder *builder)
{
	ByteCodeWriter writer = {{
		0,
		visitElement,
		CodeWriter_visitList,
		CodeWriter_visitValue},
	};
	ARRAY(ByteCode) Code;
	ARRAY_init(ByteCode, &Code, 4);
	RegisterAllocator_Init(&writer.RegAllocator, builder->LastNodeId);
	writer.ByteCode = (void *) &Code;

	EMIT_LIR(&writer, ThreadedCode, 0);

	NodePtr *x, *e;
	FOR_EACH_ARRAY(builder->Nodes, x, e) {
		writer.Current = *x;
		INodePtr *Inst, *End;
		ByteCodeWriter_SaveCurrentInstId(&writer);
		FOR_EACH_ARRAY((*x)->insts, Inst, End) {
			visitINode(&writer.visitor, *Inst);
		}
	}
	RegisterAllocator_Dispose(&writer.RegAllocator);
	ByteCode *pc = ARRAY_n(Code, 0);
	((OPThreadedCode *) (pc))->CodeSize = ARRAY_size(Code);
	NodePtr *block = ARRAY_n(builder->Nodes, 0);
	INode_setRangeEnd(ToINode(*block), 0);
	ByteCode_Link(&builder->Nodes, pc);

#ifdef DUMP_FUEL_IR
	ByteCode_Dump(pc);
#endif

	/* Direct Threading */
	FuelVM_Exec(0, 0, pc);

	struct KVirtualCodeAPI **api = (struct KVirtualCodeAPI **) (pc+1);
	api[-1] = &fuelvm_api;

	Code.list = 0;
	return pc+1;
}

#ifndef FUELVM_USE_LLVM
ByteCode *IRBuilder_CompileToLLVMIR(FuelIRBuilder *builder, IMethod *Mtd)
{
	fprintf(stderr, "%s:%d LLVM has not been linked in.\n", __func__, __LINE__);
	return IRBuilder_Lowering(builder);
}
#endif

static bool IRBuilder_Optimize(FuelIRBuilder *builder, Node *BB, bool Flag)
{
	IRBuilder_RemoveIndirectJumpNode(builder, BB, Flag); Flag = !Flag;
	IRBuilder_LinkNodes(builder, BB, Flag); Flag = !Flag;
	unsigned i;
	for(i = 0; i < 2; i++) {
		IRBuilder_SimplifyStdCall(builder);
	}
	IRBuilder_RemoveTrivialCondBranch(builder);
	IRBuilder_LinkNodes(builder, BB, Flag); Flag = !Flag;
	for(i = 0; i < 2; i++) {
		IRBuilder_SimplifyCFG(builder);
		IRBuilder_LinkNodes(builder, BB, Flag); Flag = !Flag;
	}
	return Flag;
}

ByteCode *IRBuilder_Compile(FuelIRBuilder *builder, IMethod *Mtd, int option)
{
	Node *BB = Mtd->EntryNode;
	bool Flag = true;
	ARRAY_init(NodePtr, &builder->Nodes, 1);
	IRBuilder_RemoveInstructionAfterBranchInst(builder);

	Flag = IRBuilder_Optimize(builder, BB, Flag);
	IRBuilder_FlattenICond(builder);
	Flag = IRBuilder_Optimize(builder, BB, Flag);

	IRBuilder_RemoveRedundantConstants(builder);
	IRBuilder_RemoveUnusedVariable(builder);
	IRBuilder_DumpFunction(builder);
	ByteCode *code = NULL;
	if(option == O2Compile && Mtd->Method->mn != 0) {
		code = IRBuilder_CompileToLLVMIR(builder, Mtd);
	} else {
		code = IRBuilder_Lowering(builder);
	}
	ARRAY_dispose(NodePtr, &builder->Nodes);
	return code;
}

#if 0
static void CheckByteCodeSize()
{
#define PRINT_SIZE(X) fprintf(stderr, "%s: %d\n", #X, (int)sizeof(OP##X));
	BYTECODE_LIST(PRINT_SIZE)
#undef PRINT_SIZE
}
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
