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

#include <stdio.h>
#include "codegen.h"
#include "visitor.h"
#include "FuelVM.h"
#include "RegAlloc.h"

#ifdef __cplusplus
extern "C" {
#endif

//#define DUMP_IR      1
//#define DUMP_FUEL_IR 1

/* ------------------------------------------------------------------------- */
DEF_ARRAY_OP_NOPOINTER(BlockPtr);
#ifdef DEBUG
#define debug(FMT, ...) fprintf(stderr, FMT, ## __VA_ARGS__)
#else
#define debug(FMT, ...) /*fprintf(stderr, FMT, ## __VA_ARGS__)*/
#endif

/* ------------------------------------------------------------------------- */
/* [Dump IR] */
#ifdef DUMP_IR

static const char *OPTEXT[] = {
	"Error",
	"Block",
#define IR_TEXT_DECL(X) #X,
	IR_LIST(IR_TEXT_DECL)
#undef IR_TEXT_DECL
};

static const char *Type2String(FuelIRBuilder *builder, enum TypeId Type)
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
		CASE(BoolObj);
		CASE(IntObj);
		CASE(FloatObj);
#undef CASE
	}

	KonohaContext *kctx = builder->Context;
	static char buf[256];
	KClass *kclass = KClass_(ToKType(kctx, Type));
	snprintf(buf, 256, "%s", KClass_text(kclass));
	return buf;
}

static void Block_DumpName(Block *BB)
{
	debug("  Block $%d : preds=[", BB->Id);
	BlockPtr *x, *e;
	FOR_EACH_ARRAY(BB->preds, x, e) {
		if(x != ARRAY_n(BB->preds, 0)) {
			debug(", ");
		}
		debug("$%d", (*x)->Id);
	}
	debug("], succs=[");
	FOR_EACH_ARRAY(BB->succs, x, e) {
		if(x != ARRAY_n(BB->succs, 0)) {
			debug(", ");
		}
		debug("$%d", (*x)->Id);
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
		debug("{Type:%s, Id:$%d}", Type2String(builder, Node->Type), Node->Id);
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
	debug("    %02d: $%02d = %s %s%s%s [", builder->InstructionId++, Inst->Id,
			Type2String(builder, Inst->Type),
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
	FuelIRBuilder *builder = (FuelIRBuilder *)visitor->Context;
	printHeader(builder, Node, Tag);
	IField *Inst;
	if((Inst = CHECK_KIND(Node, IField)) != 0 && Inst->Op != LocalScope) {
		printNode(Inst->Node);
		debug(", Type:%s, %d]\n", Type2String(builder, Node->Type), Inst->FieldIndex);
	} else {
		debug("Type:%s, 0x%llx]\n", Type2String(builder, Node->Type), (unsigned long long) Val.bits);
	}
}
#endif

//static
void IRBuilder_DumpFunction(FuelIRBuilder *builder)
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
	Block *Current;
	KonohaContext *kctx;
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
		case Box: {
			assert(0 && "FIXME");
		}
		default:
		assert(0 && "unreachable");
		break;
#undef CASE
	}
}

static void EmitBinaryInst(ByteCodeWriter *writer, enum BinaryOp Op, enum TypeId Type, unsigned Dst, unsigned Left, unsigned Right)
{
	if(Type == TYPE_boolean) {
		switch(Op) {
#define CASE(X) case X: EMIT_LIR(writer, X, Dst, Left, Right); return
			CASE(Eq); CASE(Nq);
			default:
			assert(0 && "unreachable");
			break;
#undef CASE
		}
	} else if(Type == TYPE_int) {
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
	Block *cur = writer->Current;
	INodePtr *x, *e;
	FOR_EACH_ARRAY(cur->insts, x, e) {
		INode *Node = *x;
		if(CHECK_KIND(Node, IUpdate) != 0) {
			continue;
		}
		if(CHECK_KIND(Node, ICall) != 0 && Node->Type == TYPE_void) {
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

#define CASE(KIND) case IR_TYPE_##KIND:
static void EmitNode(ByteCodeWriter *writer, INode *Node)
{
	DUMP_REGISTER_NODE(Node, writer);
	switch(Node->Kind) {
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
			KonohaContext *kctx = writer->kctx;
			kObject *Obj = KLIB Knull(kctx, KClass_(ToKType(kctx, Node->Type)));
			EMIT_LIR(writer, Call, ARRAY_size(Inst->Params)-1, Mtd->Value.ptr, Obj, Inst->uline);
			if(Node->Type != TYPE_void) {
				unsigned Dst = RegAllocate(writer, NODE_ID(Inst));
				if(IsUnBoxedType(Node->Type)) {
					EMIT_LIR(writer, PopI, Dst);
				} else {
					EMIT_LIR(writer, PopO, Dst);
				}
			}
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
			unsigned Src = REGISTER_UNDEFINED;
			assert(Register_FindById(&writer->RegAllocator, NODE_ID(Inst->Cond), &Src) == true);
			EMIT_LIR(writer, CondBrTrue, Src, Inst->ThenBB);
			EMIT_LIR(writer, Jump, Inst->ElseBB);
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
			EMIT_LIR(writer, Jump, Inst->TargetBlock);
			break;
		}
		CASE(IThrow) {
			IThrow *Inst = (IThrow *) Node;
			unsigned Src = REGISTER_UNDEFINED;
			assert(Register_FindById(&writer->RegAllocator, NODE_ID(Inst->Val), &Src) == true);
			EMIT_LIR(writer, Throw, Src, Inst->uline, Inst->exception, Inst->fault);
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
			if(Node->Type == TYPE_Method) {
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
	Block *Current = writer->Current;
	unsigned Index = ARRAY_size(*writer->ByteCode);
	Current->Offset = Index;
}

static void ByteCode_Link(ARRAY(BlockPtr) *blocks, ByteCode *code)
{
	ByteCode *pc = code, *end = code + ((OPThreadedCode *) code)->CodeSize;
	while(pc < end) {
		switch(GetOpcode(pc)) {
			case OPCODE_CondBrTrue:
			case OPCODE_CondBrFalse: {
				OPCondBrTrue *Inst  = (OPCondBrTrue *) pc;
				unsigned offset = ((Block *)Inst->Block)->Offset;
				Inst->Block = (void *) (code + offset);
				break;
			}
			case OPCODE_Jump: {
				OPJump *Inst = (OPJump *) pc;
				unsigned offset = ((Block *)Inst->Block)->Offset;
				Inst->Block = (void *) (code + offset);
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
	writer.kctx = builder->Context;
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
	(*block)->Offset = 0;
	ByteCode_Link(&builder->Blocks, pc);

#ifdef DUMP_FUEL_IR
	ByteCode_Dump(pc);
#endif

	/* Direct Threading */
	FuelVM_Exec(0, 0, pc);

	struct KVirtualCodeAPI **api = (struct KVirtualCodeAPI **) (pc+1);
	api[-1] = &fuelvm_api;
	((unsigned *) pc)[0] = FUELVM_BYTECODE_MAGICNUMBER;

	Code.list = 0;
	return pc+1;
}

#ifndef FUELVM_USE_LLVM
ByteCode *IRBuilder_CompileToLLVMIR(FuelIRBuilder *builder, IMethod *Mtd, int opt)
{
	return IRBuilder_Lowering(builder);
}
#endif

ByteCode *IRBuilder_Compile(FuelIRBuilder *builder, IMethod *Mtd, int option, bool *JITCompiled)
{
	ARRAY_init(BlockPtr, &builder->Blocks, 1);
	bool UseLLVM = !(Mtd->Method->typeId == 0 && Mtd->Method->mn == 0);

#ifdef DEBUG
	KonohaContext *kctx = Mtd->Context;
#endif
	debug("Compiling: %p %s.%s%s\n", Mtd->Method, KType_text(Mtd->Method->typeId), KMethodName_Fmt2(Mtd->Method->mn));
#ifndef FUELVM_USE_LLVM
	UseLLVM = false;
	debug("%s:%d LLVM has not been linked in.\n", __func__, __LINE__);
#endif
	IRBuilder_Optimize(builder, Mtd->EntryBlock, UseLLVM);
	IRBuilder_DumpFunction(builder);
	ByteCode *code = NULL;

	if(UseLLVM) {
		code = IRBuilder_CompileToLLVMIR(builder, Mtd, option);
		*JITCompiled = true;
	} else {
		code = IRBuilder_Lowering(builder);
	}
	ARRAY_dispose(BlockPtr, &builder->Blocks);
	return code;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
