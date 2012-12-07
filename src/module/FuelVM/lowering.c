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
#include "visitor.h"
#include "FuelVM.h"
#include "RegAlloc.h"

//#define DUMP_IR      1
#define DUMP_FUEL_IR 1
#define DEBUG 0

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------- */
DEF_ARRAY_OP_NOPOINTER(BlockPtr);
DEF_ARRAY_OP_NOPOINTER(INodePtr);

#define debug(FMT, ...) do {\
	if(DEBUG) {\
		fprintf(stderr, FMT, ## __VA_ARGS__);\
	}\
} while(0)

static void Block_addPred(Block *BB, Block *Pred)
{
	ARRAY_add(BlockPtr, &BB->preds, Pred);
}

static void Block_addSucc(Block *BB, Block *Succ)
{
	ARRAY_add(BlockPtr, &BB->succs, Succ);
}

/* ------------------------------------------------------------------------- */

static void IRBuilder_LinkBlocks(FuelIRBuilder *builder, Block *BB, bool IsVisited)
{
	while(Block_IsVisited(BB) != IsVisited) {
		Block_SetVisited(BB, IsVisited);
		INode *Inst = *(ARRAY_last(BB->insts));
		enum IRType Kind = Inst->Kind;
		assert(IsBranchInst(Inst));
		ARRAY_add(BlockPtr, &builder->Blocks, BB);
		switch(Kind) {
		case IR_TYPE_IBranch:
			Block_addSucc(BB, ((IBranch *)Inst)->ThenBB);
			Block_addSucc(BB, ((IBranch *)Inst)->ElseBB);
			Block_addPred(((IBranch *)Inst)->ThenBB, BB);
			Block_addPred(((IBranch *)Inst)->ElseBB, BB);
			IRBuilder_LinkBlocks(builder, ((IBranch *)Inst)->ThenBB, IsVisited);
			BB = ((IBranch *)Inst)->ElseBB;
			break;
		case IR_TYPE_ITest:
			Block_addSucc(BB, ((ITest *)Inst)->TargetBlock);
			Block_addPred(((ITest *)Inst)->TargetBlock, BB);
			BB = ((ITest *)Inst)->TargetBlock;
			break;
		case IR_TYPE_IReturn:
		case IR_TYPE_IThrow:
		case IR_TYPE_IYield:
			break;
		case IR_TYPE_ITry:
			assert(0 && "TODO");
			break;
		case IR_TYPE_IJump:
			Block_addSucc(BB, ((IJump *)Inst)->TargetBlock);
			Block_addPred(((IJump *)Inst)->TargetBlock, BB);
			BB = ((IJump *)Inst)->TargetBlock;
			break;
		default:
			assert(0 && "unreachable");
		}
	}
}

static void CopyIfBlockHasSingleInst(Block *BB, INode *LastInst, Block *Block)
{
	if(ARRAY_size(Block->insts) == 1) {
		INode *Inst = ARRAY_get(INodePtr, &Block->insts, 0);
		if(CHECK_KIND(LastInst, IJump) && CHECK_KIND(Inst, IReturn)) {
			//debug("Jump %d=>%d\n", BB->base.Id, Block->base.Id);
			memcpy(LastInst, Inst, sizeof(INodeImpl));
		}
		else if(CHECK_KIND(LastInst, IBranch) && CHECK_KIND(Inst, IJump)) {
			//debug("Branch %d=>%d\n", BB->base.Id, Block->base.Id);
			IBranch *Branch = (IBranch *) LastInst;
			IJump   *Jump   = (IJump *) Inst;
			if(Branch->ThenBB == Block) {
				Branch->ThenBB = Jump->TargetBlock;
			} else {
				Branch->ElseBB = Jump->TargetBlock;
			}
		}
	}
}

static void IRBuilder_RemoveIndirectJumpBlock(FuelIRBuilder *builder, Block *BB, bool IsVisited)
{
	/* Remove Block that contain only one Jump Instruction
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
	while(Block_IsVisited(BB) != IsVisited) {
		Block_SetVisited(BB, IsVisited);
		INode *Inst = *(ARRAY_last(BB->insts));
		assert(IsBranchInst(Inst));
		IBranch *Branch;
		ITest   *Test;
		IJump   *Jump;
		if((Branch = CHECK_KIND(Inst, IBranch)) != 0) {
			CopyIfBlockHasSingleInst(BB, ToINode(Branch), ((IBranch *)Inst)->ThenBB);
			CopyIfBlockHasSingleInst(BB, ToINode(Branch), ((IBranch *)Inst)->ElseBB);
			IRBuilder_RemoveIndirectJumpBlock(builder, ((IBranch *)Inst)->ThenBB, IsVisited);
			BB = ((IBranch *)Inst)->ElseBB;
		}
		else if((Jump = CHECK_KIND(Inst, IJump)) != 0) {
			Block *NextBB = ((IJump *)Inst)->TargetBlock;
			CopyIfBlockHasSingleInst(BB, ToINode(Jump), ((IJump *)Inst)->TargetBlock);
			BB = NextBB;
		}
		else if((Test = CHECK_KIND(Inst, ITest)) != 0) {
			BB = ((ITest *)Inst)->TargetBlock;
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


static void TraceNode(INode *Node)
{
#define CASE(KIND) case IR_TYPE_##KIND:
	switch(Node->Kind) {
		case IR_TYPE_IConstant:
		case IR_TYPE_IArgument:
		case IR_TYPE_ILabel:
			break;
		CASE(IField) {
			return;
		}
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
			IUpdate *Inst = (IUpdate *) Node;
			INode_SetMarked(Node);
			INode_SetMarked((INode *)Inst->LHS);
			INode_SetMarked((INode *)Inst->RHS);
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
			INode_SetMarked(Node);
			assert(0 && "TODO");
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

static void IRBuilder_RemoveUnusedVariable(FuelIRBuilder *builder)
{
	BlockPtr *x, *e;
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		INodePtr *Inst, *End;
		FOR_EACH_ARRAY((*x)->insts, Inst, End) {
			TraceNode(*Inst);
		}
	}
	ARRAY(INodePtr) Insts;
	ARRAY_init(INodePtr, &Insts, 32);
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
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
}

/* ------------------------------------------------------------------------- */
/* [Dump IR] */
#ifdef DUMP_IR

static const char *OPTEXT[] = {
	"Block",
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
	CASE(String);
	CASE(Function);
	CASE(Array);
	CASE(Method);
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

static void Block_DumpName(Block *BB)
{
	debug("  Block $%d : preds=[", BB->base.Id);
	BlockPtr *x, *e;
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
	debug("Type:%s, 0x%llx]\n", Type2String(Node->Type), Val.bits);
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
	BlockPtr *x, *e;
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		Block_DumpName(*x);
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

#define EMIT_LIR(writer, OPCODE, ...) do {\
	OP##OPCODE tmp = {{/*0, */OPCODE_##OPCODE}, ## __VA_ARGS__};\
	EmitByteCode(writer, (ByteCode *) &tmp);\
} while(0)

DEF_ARRAY_STRUCT(ByteCode);
DEF_ARRAY_T(ByteCode);
DEF_ARRAY_OP(ByteCode);
typedef ARRAY(ByteCode) ByteCodeList;

typedef struct ByteCodeWriter {
	Visitor visitor;
	ByteCodeList *ByteCode;
	Block *Current;
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
				Inst->Dst = NewInst->Dst;
				return;
			}
		}
	}
	ARRAY_add(ByteCode, CodeBuf, Code);
}

static void EmitBinaryInst(ByteCodeWriter *writer, enum BinaryOp Op, unsigned Dst, unsigned Left, unsigned Right)
{
	switch(Op) {
#define CASE(X) case X: EMIT_LIR(writer, X, Dst, Left, Right); break
		CASE(Add); CASE(Sub); CASE(Mul); CASE(Div); CASE(Mod);
		CASE(LShift); CASE(RShift); CASE(And); CASE(Or); CASE(Xor);
		CASE(Eq); CASE(Nq); CASE(Gt); CASE(Ge); CASE(Lt); CASE(Le);
		default:
			assert(0 && "unreachable");
			break;
#undef CASE
	}
}

static void DeallocateWithoutLocalVar(ByteCodeWriter *writer)
{
	Block *cur = writer->Current;
	INodePtr *x, *e;
	FOR_EACH_ARRAY(cur->insts, x, e) {
		INode *Node = *x;
		IUpdate *Inst;
		if((Inst = CHECK_KIND(Node, IUpdate)) != 0) {
			if(Inst->LHS->Op == LocalScope)
				continue;
		}
		if(IsLocalVariable(Node))
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

#define CASE(KIND) case IR_TYPE_##KIND:
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

static void EmitNode(ByteCodeWriter *writer, INode *Node)
{
	DUMP_REGISTER_NODE(Node, writer);
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
			FOR_EACH_ARRAY(Inst->Params, x, e) {
				if(*x == 0)
					continue;
				INode *Param = *x;

				if(Param->Type == TY_Method) {
					asm volatile("int3");
				}
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
			EMIT_LIR(writer, Call, Dst, ARRAY_size(Inst->Params)-1, IsUnboxed, Mtd->Value.ptr);
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
				case FieldScope:
					assert(0 && "TODO");
					break;
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
			unsigned Src;
			assert(Register_FindById(&writer->RegAllocator, NODE_ID(Inst->Cond), &Src) == true);
			DeallocateWithoutLocalVar(writer);
			EMIT_LIR(writer, CondBr, Src, Inst->ThenBB);
			EMIT_LIR(writer, Jump, Inst->ElseBB);
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
			EMIT_LIR(writer, Jump, Inst->TargetBlock);
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
			IBinary *Inst  = (IBinary *) Node;
			unsigned Left, Right;
			assert(Register_FindById(&writer->RegAllocator, NODE_ID(Inst->LHS), &Left) == true);
			assert(Register_FindById(&writer->RegAllocator, NODE_ID(Inst->RHS), &Right) == true);
			unsigned Dst   = Register_FindByIdOrAllocate(&writer->RegAllocator, NODE_ID(Inst));
			EmitBinaryInst(writer, Inst->Op, Dst, Left, Right);
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
	switch(Node->Kind) {
		CASE(IConstant) {
			if(Node->Type == TY_Method) {
				/*XXX(ide) Need to Load Method Constant ??? */
				return;
			}
			unsigned Dst = Register_FindByIdOrAllocate(&writer->RegAllocator, NODE_ID(Node));
			EMIT_LIR(writer, LoadConstant, Dst, Val);
			return;
		}
		CASE(IArgument) {
			unsigned Dst = Register_FindByIdOrAllocate(&writer->RegAllocator, NODE_ID(Node));
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
			return;
		}
		default:
			assert(0 && "unreachable");
	}
}

static void ByteCodeWriter_SaveCurrentInstId(ByteCodeWriter *writer)
{
	Block *Current = writer->Current;
	unsigned Index = ARRAY_size(*writer->ByteCode);
	INode_setRangeEnd(ToINode(Current), Index);
}

static void ByteCode_Link(ARRAY(BlockPtr) *blocks, ByteCode *code)
{
	ByteCode *pc = code, *end = code + ((OPThreadedCode *) code)->CodeSize;
	while(pc < end) {
		switch(GetOpcode(pc)) {
			case OPCODE_CondBr: {
				OPCondBr *Inst  = (OPCondBr *) pc;
				unsigned offset = INode_getRangeEnd(ToINode((Block *)Inst->Block));
				Inst->Block = (void *) (code + offset);
				break;
			}
			case OPCODE_Jump: {
				OPJump *Inst = (OPJump *) pc;
				unsigned offset = INode_getRangeEnd(ToINode((Block *)Inst->Block));
				Inst->Block = (void *) (code + offset);
				break;
			}
			case OPCODE_Throw:
			case OPCODE_Try:
			case OPCODE_Yield:
				assert(0 && "TODO");
				break;
			default:
				break;
		}
		pc++;
	}
}

static void ByteCode_free(KonohaContext *kctx, struct VirtualCode *vcode)
{
	ByteCode *code = ((ByteCode *) vcode)-1;
	free(code);
}

static void ByteCode_write(KonohaContext *kctx, KGrowingBuffer *wb, struct VirtualCode *vcode)
{
	ByteCode *code = (ByteCode *) vcode;
	ByteCode_Dump(code);
}

static struct VirtualCodeAPI fuelvm_api = {
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

	BlockPtr *x, *e;
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
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
	BlockPtr *block = ARRAY_n(builder->Blocks, 0);
	INode_setRangeEnd(ToINode(*block), 0);
	ByteCode_Link(&builder->Blocks, pc);

#ifdef DUMP_FUEL_IR
	ByteCode_Dump(pc);
#endif

	/* Direct Threading */
	FuelVM_Exec(0, 0, pc);

	struct VirtualCodeAPI **api = (struct VirtualCodeAPI **) (pc+1);
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

ByteCode *IRBuilder_Compile(FuelIRBuilder *builder, IMethod *Mtd, int option)
{
	Block *BB = Mtd->EntryBlock;
	bool Flag = true;
	ARRAY_init(BlockPtr, &builder->Blocks, 1);
	IRBuilder_RemoveIndirectJumpBlock(builder, BB, Flag); Flag = !Flag;
	IRBuilder_LinkBlocks(builder, BB, Flag); Flag = !Flag;
	IRBuilder_RemoveUnusedVariable(builder);
	IRBuilder_DumpFunction(builder);
	ByteCode *code = NULL;
	if(option == O2Compile && Mtd->Method->mn != 0) {
		code = IRBuilder_CompileToLLVMIR(builder, Mtd);
	} else {
		code = IRBuilder_Lowering(builder);
	}
	ARRAY_dispose(BlockPtr, &builder->Blocks);
	return code;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
