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

#include "FuelVM.h"
#include "codegen.h"

#ifdef __cplusplus
extern "C" {
#endif

DEF_ARRAY_OP_NOPOINTER(INodePtr);

/* ------------------------------------------------------------------------- */
/* Replace/Remove API */
static void ReplaceValue(INode **NodePtr, INode *oldVal, INode *newVal)
{
	if((*NodePtr)->Id == oldVal->Id) {
		*NodePtr = newVal;
	}
}

static void ReplaceArrayElementWith(ARRAY(INodePtr) *List, INode *oldVal, INode *newVal)
{

	INodePtr *x, *e;
	FOR_EACH_ARRAY((*List), x, e) {
		ReplaceValue(x, oldVal, newVal);
	}
}

static void ReplaceOldValueWith(INode *Node, INode *oldVal, INode *newVal)
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
				case GlobalScope:;
				case EnvScope:;
				case FieldScope:;
						assert(0 && "TODO");
				case LocalScope:;
						break;
			}
			break;
		}
		case IR_TYPE_ICond:
			ReplaceArrayElementWith(&((ICond *)Node)->Insts, oldVal, newVal);
			break;
		case IR_TYPE_INew:
			break;
		CASE(ICall) {
			ReplaceArrayElementWith(&((ICall *)Node)->Params, oldVal, newVal);
			break;
		}
		CASE(IFunction) {
			ReplaceArrayElementWith(&((IFunction *)Node)->Env, oldVal, newVal);
			break;
		}
		CASE(IUpdate) {
			IUpdate *Inst = (IUpdate *) Node;
			ReplaceValue((INode **)&(Inst->LHS), oldVal, newVal);
			ReplaceValue((INode **)&(Inst->RHS), oldVal, newVal);
			break;
		}
		CASE(IBranch) {
			ReplaceValue(&((IBranch *) Node)->Cond, oldVal, newVal);
			break;
		}
		CASE(ITest) {
			ReplaceValue((INode **)&((ITest *) Node)->Value, oldVal, newVal);
			break;
		}
		CASE(IReturn) {
			IReturn *Inst = (IReturn *) Node;
			if(Inst->Inst) {
				ReplaceValue(&Inst->Inst, oldVal, newVal);
			}
			break;
		}
		case IR_TYPE_IJump:
		case IR_TYPE_ITry:
		break;
		CASE(IThrow) {
			assert(0 && "TODO");
			break;
		}
		CASE(IYield) {
			ReplaceValue(&((IYield *) Node)->Value, oldVal, newVal);
			break;
		}
		CASE(IUnary) {
			ReplaceValue(&((IUnary *) Node)->Node, oldVal, newVal);
			break;
		}
		CASE(IBinary) {
			IBinary *Inst  = (IBinary *) Node;
			ReplaceValue(&(Inst->LHS), oldVal, newVal);
			ReplaceValue(&(Inst->RHS), oldVal, newVal);
			break;
		}
		default:
			assert(0 && "unreachable");
#undef CASE
	}
}

static void IRBuilder_ReplaceValueWith(FuelIRBuilder *builder, INode *oldVal, INode *newVal)
{
	BlockPtr *x, *e;
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		INodePtr *Inst, *End;
		FOR_EACH_ARRAY((*x)->insts, Inst, End) {
			ReplaceOldValueWith(*Inst, oldVal, newVal);
		}
	}
}

/* ------------------------------------------------------------------------- */
/* RemoveDuplicatedConstantInBasicBlock */
static void AddConstantListIfNotDefined(FuelIRBuilder *builder, ARRAY(INodePtr) *ConstList, IConstant *newVal)
{
	INodePtr *x, *e;
	FOR_EACH_ARRAY(*ConstList, x, e) {
		IConstant *Inst = (IConstant *) *x;
		if(ToINode(Inst)->Type == ToINode(newVal)->Type &&
				Inst->Value.bits == newVal->Value.bits) {
			IRBuilder_ReplaceValueWith(builder, (INode *) newVal, (INode *) Inst);
			return;
		}
	}
	ARRAY_add(INodePtr, ConstList, (INode *) newVal);
}

void IRBuilder_RemoveRedundantConstants(FuelIRBuilder *builder)
{
	BlockPtr *x, *e;
	ARRAY(INodePtr) ConstList;
	ARRAY_init(INodePtr, &ConstList, 0);
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		INodePtr *Itr, *End;
		ARRAY_clear(ConstList);
		FOR_EACH_ARRAY((*x)->insts, Itr, End) {
			IConstant *Inst;
			if((Inst = CHECK_KIND(*Itr, IConstant))) {
				AddConstantListIfNotDefined(builder, &ConstList, Inst);
			}
		}
	}
	ARRAY_dispose(INodePtr, &ConstList);
}

/* ------------------------------------------------------------------------- */
/* SimplifyStdCall */
#define __Add(LHS, RHS)    ((LHS) + (RHS))
#define __Sub(LHS, RHS)    ((LHS) - (RHS))
#define __Mul(LHS, RHS)    ((LHS) * (RHS))
#define __Div(LHS, RHS)    ((LHS) / (RHS))
#define __Mod(LHS, RHS)    ((LHS) % (RHS))
#define __LShift(LHS, RHS) ((LHS) << (RHS))
#define __RShift(LHS, RHS) ((LHS) >> (RHS))
#define __And(LHS, RHS)    ((LHS) &  (RHS))
#define __Or(LHS, RHS)     ((LHS) |  (RHS))
#define __Xor(LHS, RHS)    ((LHS) ^  (RHS))
#define __Eq(LHS, RHS)     ((LHS) == (RHS))
#define __Nq(LHS, RHS)     ((LHS) != (RHS))
#define __Gt(LHS, RHS)     ((LHS) >  (RHS))
#define __Ge(LHS, RHS)     ((LHS) >= (RHS))
#define __Lt(LHS, RHS)     ((LHS) <  (RHS))
#define __Le(LHS, RHS)     ((LHS) <= (RHS))

static inline SValue __Not(SValue arg0)
{
	SValue val; val.bval = !arg0.ival;
	return val;
}
static inline SValue __Neg(SValue arg0)
{
	SValue val; val.ival = -arg0.ival;
	return val;
}

#include "FuelVMInsts.h"

static INode *SimplifyIBinary(FuelIRBuilder *builder, enum BinaryOp Op, IConstant *LHS, IConstant *RHS)
{
	SValue ret;
	if(ToINode(LHS)->Type == TYPE_int) {
		if(ToINode(RHS)->Type == TYPE_int) {
			switch(Op) {
#define CASE(X) case X: ret = X##_int_int(LHS->Value, RHS->Value); break
				CASE(Add); CASE(Sub); CASE(Mul); CASE(Div); CASE(Mod);
				CASE(LShift); CASE(RShift); CASE(And); CASE(Or); CASE(Xor);
				CASE(Eq); CASE(Nq); CASE(Gt); CASE(Ge); CASE(Lt); CASE(Le);
#undef CASE
				default:
				assert(0 && "unreachable");
				break;
			}
			return (INode *) builder->API->newConstant(builder, TYPE_int, ret);
		}
	}
	return NULL;
}

static INode *SimplifyInst(FuelIRBuilder *builder, INode *Node)
{
	if(Node->Kind == IR_TYPE_ICall) {
		ICall *Inst = (ICall *) Node;
		INodePtr *x;
		//INode **MtdPtr = (ARRAY_n(Inst->Params, 0));
		//IConstant *Mtd = (IConstant *) *MtdPtr;
		unsigned i = 0;
		FOR_EACH_ARRAY_(Inst->Params, x, i) {
			/* Skip method object */
			if(i == 0) continue;
			if(!CHECK_KIND(*x, IConstant))
				return NULL;
		}
		//asm volatile("int3");
		//fprintf(stderr, "%s\n", "hi");
	}
	if(Node->Kind == IR_TYPE_IUnary) {
		IUnary *Inst = (IUnary *) Node;
		IConstant *Val;
		if((Val = CHECK_KIND(Inst->Node, IConstant))) {
			if(ToINode(Val)->Type == TYPE_int) {
				SValue ret;
				switch(Op) {
#define CASE(X) case X: ret = __##X((Val)->Value); break
					CASE(Not); CASE(Neg);
#undef CASE
					default: assert(0 && "unreachable");
				}
				return (INode *) builder->API->newConstant(builder, TYPE_int, ret);
			}
		}
	}
	if(Node->Kind == IR_TYPE_IBinary) {
		IBinary *Inst = (IBinary *) Node;
		IConstant *LHS, *RHS;
		if((LHS = CHECK_KIND(Inst->LHS, IConstant))) {
			if((RHS = CHECK_KIND(Inst->RHS, IConstant))) {
				return SimplifyIBinary(builder, Inst->Op, LHS, RHS);
			}
		}
	}
	return NULL;
}

static void SchedulingReplaceValue(FuelIRBuilder *builder, INode *oldNode, INode *newNode)
{
	IRBuilder_ReplaceValueWith(builder, oldNode, newNode);
}

static void Block_insertNode(Block *block, ARRAY(INodePtr) *List)
{
	unsigned shift = ARRAYp_size(List);
	ARRAY_ensureSize(INodePtr, &block->insts, ARRAY_size(block->insts) + shift);
	memmove(block->insts.list+shift, block->insts.list, sizeof(BlockPtr)*shift);
	memcpy(block->insts.list, List->list, sizeof(BlockPtr)*shift);
}

void IRBuilder_SimplifyStdCall(FuelIRBuilder *builder)
{
	ARRAY(INodePtr) Modified;
	BlockPtr *x, *e;
	ARRAY_init(INodePtr, &Modified, 0);
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		INodePtr *Inst, *End;
		FOR_EACH_ARRAY((*x)->insts, Inst, End) {
			INode *Node = SimplifyInst(builder, *Inst);
			if(Node) {
				SchedulingReplaceValue(builder, *Inst, Node);
				ARRAY_add(INodePtr, &Modified, Node);
			}
		}
		Block_insertNode(*x, &Modified);
	}
	ARRAY_dispose(INodePtr, &Modified);
}

/* ------------------------------------------------------------------------- */
/* RemoveTrivialCondBranch */

void IRBuilder_RemoveTrivialCondBranch(FuelIRBuilder *builder)
{
	BlockPtr *x, *e;
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		if(ARRAY_size((*x)->insts) > 0) {
			INode **NodePtr = ARRAY_last((*x)->insts);
			IBranch *Br;
			if((Br = CHECK_KIND(*NodePtr, IBranch))) {
				IConstant *Cond;
				if((Cond = CHECK_KIND(Br->Cond, IConstant))) {
					assert(ToINode(Cond)->Type == TYPE_boolean);
					Block *TargetBB, *RemovedBB;
					if(Cond->Value.bval) {
						TargetBB  = Br->ThenBB;
						RemovedBB = Br->ElseBB;
					} else {
						TargetBB  = Br->ElseBB;
						RemovedBB = Br->ThenBB;
					}
					ARRAY_clear(RemovedBB->insts);
					*NodePtr = (INode *) builder->API->newJump(builder, TargetBB);
				}
			}
		}
	}
}
#ifdef __cplusplus
} /* extern "C" */
#endif
