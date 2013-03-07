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

#include "FuelVM.h"
#include "codegen.h"

#ifdef __cplusplus
extern "C" {
#endif

DEF_ARRAY_OP_NOPOINTER(BlockPtr);
DEF_ARRAY_OP_NOPOINTER(INodePtr);
typedef ARRAY(INodePtr) INodeList;
/* ------------------------------------------------------------------------- */
static void Block_addPred(Block *BB, Block *Pred)
{
	ARRAY_add(BlockPtr, &BB->preds, Pred);
}

static void Block_addSucc(Block *BB, Block *Succ)
{
	ARRAY_add(BlockPtr, &BB->succs, Succ);
}

static void RemoveEdge(Block *To, Block *From)
{
	BlockPtr *x;
	unsigned i;
	FOR_EACH_ARRAY_(To->succs, x, i) {
		if(*x == From)
			break;
	}
	ARRAY_RemoveAt(BlockPtr, &To->succs, i);
	FOR_EACH_ARRAY_(From->preds, x, i) {
		if(*x == To)
			break;
	}
	ARRAY_RemoveAt(BlockPtr, &From->preds, i);
}

static void AddEdge(Block *To, Block *From)
{
	Block_addSucc(To, From);
	Block_addPred(From, To);
}

/* ------------------------------------------------------------------------- */
/* Replace/Remove API */
static void ReplaceValue(INode **NodePtr, INode *oldVal, INode *newVal)
{
	if((*NodePtr)->Id == oldVal->Id) {
		*NodePtr = newVal;
		newVal->Parent = oldVal->Parent;
	}
}

static void ReplaceArrayElementWith(INodeList *List, INode *oldVal, INode *newVal)
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
			break;
		CASE(IField) {
			IField *Inst = (IField *) Node;
			switch(Inst->Op) {
				case GlobalScope:
				case EnvScope:
				case FieldScope:
					ReplaceValue((INode **)&(Inst->Node), oldVal, newVal);
					break;
				case LocalScope:
					break;
			}
			break;
		}
		case IR_TYPE_INew:
			break;
		CASE(ICall) {
			ReplaceArrayElementWith(&((ICall *)Node)->Params, oldVal, newVal);
			break;
		}
		CASE(IFunction) {
			IFunction *Inst = (IFunction *) Node;
			ReplaceValue(&(Inst->Func), oldVal, newVal);
			ReplaceValue(&(Inst->Env), oldVal, newVal);
			ReplaceValue(&(Inst->Method), oldVal, newVal);
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
			ReplaceValue(&((IThrow *)Node)->Val, oldVal, newVal);
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
		CASE(IPHI) {
			ReplaceArrayElementWith(&((IPHI *)Node)->Args, oldVal, newVal);
			break;
		}
		default:
			assert(0 && "unreachable");
#undef CASE
	}
}

static void ReplaceValueWith(FuelIRBuilder *builder, INode *oldVal, INode *newVal)
{
	BlockPtr *x, *e;
	oldVal->Unused = 1;
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		INodePtr *Inst, *End;
		FOR_EACH_ARRAY((*x)->insts, Inst, End) {
			ReplaceOldValueWith(*Inst, oldVal, newVal);
		}
	}
}

/* ------------------------------------------------------------------------- */
/* RemoveDuplicatedConstantInBasicBlock */
#ifdef USE_RemoveDuplicatedConstantInBasicBlock
static void AddConstantListIfNotDefined(FuelIRBuilder *builder, INodeList *ConstList, IConstant *newVal)
{
	INodePtr *x, *e;
	FOR_EACH_ARRAY(*ConstList, x, e) {
		IConstant *Inst = (IConstant *) *x;
		if(ToINode(Inst)->Type == ToINode(newVal)->Type &&
				Inst->Value.bits == newVal->Value.bits) {
			ReplaceValueWith(builder, (INode *) newVal, (INode *) Inst);
			return;
		}
	}
	ARRAY_add(INodePtr, ConstList, (INode *) newVal);
}

static void RemoveRedundantConstants(FuelIRBuilder *builder)
{
	BlockPtr *x, *e;
	INodeList ConstList;
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
#endif

/* ------------------------------------------------------------------------- */
/* FoldStdCall */
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
	SValue val; val.ival = !arg0.ival;
	return val;
}
static inline SValue __Neg(SValue arg0)
{
	SValue val; val.ival = -arg0.ival;
	return val;
}

#include "FuelVMInsts.h"

static INode *FoldIBinary(FuelIRBuilder *builder, IBinary *Inst)
{
	SValue ret;
	enum BinaryOp Op = Inst->Op;
	KonohaContext *kctx = builder->Context;
	IConstant *LHS = (IConstant *) Inst->LHS;
	IConstant *RHS = (IConstant *) Inst->RHS;
	if(ToINode(LHS)->Type == TYPE_boolean) {
		if(ToINode(RHS)->Type == TYPE_boolean) {
			switch(Op) {
#define CASE(X) case X: ret = X##_int_int(LHS->Value, RHS->Value); break
				CASE(Eq); CASE(Nq);
#undef CASE
				default:
					assert(0 && "unreachable");
					break;
			}
			return (INode *) builder->API->newConstant(builder, Inst->base.Type, ret);
		}
	}
	if(ToINode(LHS)->Type == TYPE_int) {
		if(ToINode(RHS)->Type == TYPE_int) {
			switch(Op) {
#define CASE(X) case X: ret = X##_int_int(LHS->Value, RHS->Value); break
				CASE(Add); CASE(Sub); CASE(Mul);
				case Div: {
					if(unlikely(RHS->Value.ival == 0)) {
						INode   *Node = CreateObject(builder, TYPE_String, (void *)TS_EMPTY);
						return (INode *) builder->API->newThrow(builder, Node,
								KException_("ZeroDivided"), SoftwareFault, Inst->uline);
					}
					ret = Div_int_int(LHS->Value, RHS->Value);
					break;
				}
				CASE(Mod);
				CASE(LShift); CASE(RShift); CASE(And); CASE(Or); CASE(Xor);
				CASE(Eq); CASE(Nq); CASE(Gt); CASE(Ge); CASE(Lt); CASE(Le);
#undef CASE
				default:
				assert(0 && "unreachable");
				break;
			}
			return (INode *) builder->API->newConstant(builder, Inst->base.Type, ret);
		}
	}
	if(ToINode(LHS)->Type == TYPE_float) {
		if(ToINode(RHS)->Type == TYPE_float) {
			switch(Op) {
#define CASE(X) case X: ret = X##_float_float(LHS->Value, RHS->Value); break
				CASE(Add); CASE(Sub); CASE(Mul);
				case Div: {
					if(unlikely(RHS->Value.ival == 0)) {
						INode   *Node = CreateObject(builder, TYPE_String, (void *)TS_EMPTY);
						return (INode *) builder->API->newThrow(builder, Node,
								KException_("ZeroDivided"), SoftwareFault, Inst->uline);
					}
					ret = Div_float_float(LHS->Value, RHS->Value);
					break;
				}
				CASE(Eq); CASE(Nq); CASE(Gt); CASE(Ge); CASE(Lt); CASE(Le);
#undef CASE
				default:
				assert(0 && "unreachable");
				break;
			}
			return (INode *) builder->API->newConstant(builder, Inst->base.Type, ret);
		}
	}
	return NULL;
}

static INode *FoldAssert(FuelIRBuilder *builder, ICall *Inst)
{
	/* Remove NameSpace.assert(true) */
	KonohaContext *kctx = builder->Context;
	INode **MtdPtr = ARRAY_n(Inst->Params, 0);
	kMethod *mtd = (kMethod *) ((IConstant *) *MtdPtr)->Value.obj;
	if(mtd->typeId == TYPE_NameSpace && mtd->mn == KMethodName_("assert")) {
		if(ARRAY_size(Inst->Params) != 3) {
			return 0;
		}
		IConstant *Val;
		if((Val = CHECK_KIND(*ARRAY_n(Inst->Params, 2), IConstant))) {
			if(Val->base.Type == TYPE_boolean && Val->Value.bval == true) {
				Inst->base.Unused = 1;
				return (INode *) builder->API->newConstant(builder, Val->base.Type, Val->Value);
			}
		}
	}
	return 0;
}

static INode *FoldICall(FuelIRBuilder *builder, ICall *Inst)
{
	SValue val = {};
	unsigned i;
	INodePtr *x, *MtdPtr = ARRAY_n(Inst->Params, 0);
	kMethod *mtd = (kMethod *) ((IConstant *) *MtdPtr)->Value.obj;

	INode *ret = NULL;
	if((ret = FoldAssert(builder, Inst)) != 0) {
		return ret;
	}
	if(!kMethod_Is(Const, mtd)) {
		return NULL;
	}
	FOR_EACH_ARRAY_(Inst->Params, x, i) {
		if(!CHECK_KIND(*x, IConstant))
			return NULL;
	}

	KonohaContext *kctx = builder->Context;
	unsigned psize = ARRAY_size(Inst->Params) - 1;
	BEGIN_UnusedStack(lsfp);

	enum TypeId Type = ToUnBoxType(Inst->base.Type);
	KClass  *kclass = KClass_(ToKType(kctx, Type));
	kObject *DefObj = KLIB Knull(kctx, kclass);
	KStackSetObjectValue(lsfp[K_RTNIDX].asObject, DefObj);
	KStackSetUnboxValue(lsfp[K_RTNIDX].unboxValue, kObject_Unbox(DefObj));

	FOR_EACH_ARRAY__(Inst->Params, x, i, 1) {
		IConstant *C = (IConstant *) *x;
		if(IsUnBoxedType(C->base.Type)) {
			KStackSetUnboxValue(lsfp[i-1].unboxValue, C->Value.bits);
		} else {
			KStackSetObjectValue(lsfp[i-1].asObject, C->Value.obj);
			KStackSetUnboxValue(lsfp[i-1].unboxValue, kObject_Unbox(C->Value.obj));
		}
	}
	if(Inst->Op == VirtualCall) {
		kNameSpace *ns = builder->Method->ns;
		KStackSetObjectValue(lsfp[K_NSIDX].asNameSpace, ns);
	}
	KStackSetMethodAll(lsfp, DefObj, Inst->uline, mtd, psize);
	KStackCall(lsfp);
	END_UnusedStack();

	Type = Inst->base.Type;
	if(IsUnBoxedType(Type)) {
		val.bits = lsfp[K_RTNIDX].unboxValue;
	} else {
		val.obj  = lsfp[K_RTNIDX].asObject;
	}
	return (INode *) builder->API->newConstant(builder, Type, val);
}

INode *FoldInst(FuelIRBuilder *builder, INode *Node)
{
	if(Node->Kind == IR_TYPE_ICall) {
		return FoldICall(builder, (ICall *) Node);
	}
	if(Node->Kind == IR_TYPE_IUnary) {
		IUnary *Inst = (IUnary *) Node;
		IConstant *Val;
		if((Val = CHECK_KIND(Inst->Node, IConstant))) {
			if(ToINode(Val)->Type == TYPE_int) {
				SValue ret;
				switch(Inst->Op) {
#define CASE(X) case X: ret = __##X((Val)->Value); break
					CASE(Not); CASE(Neg);
#undef CASE
					case Box: {
						return NULL;
						//FIXME
						//KClass *ct =...
						//return KLIB new_kObject(kctx, OnStack, ct, Val->Value);
					}
					default: assert(0 && "unreachable");
				}
				return (INode *) builder->API->newConstant(builder, TYPE_int, ret);
			}
		}
	}
	if(Node->Kind == IR_TYPE_IBinary) {
		IBinary *Inst = (IBinary *) Node;
		if(CHECK_KIND(Inst->LHS, IConstant)) {
			if(CHECK_KIND(Inst->RHS, IConstant)) {
				return FoldIBinary(builder, Inst);
			}
		}
	}
	return NULL;
}


typedef struct ValueReplacer {
	FuelIRBuilder *builder;
	INodeList Vals;
	unsigned Count;
} ValueReplacer;

static void ValueReplacer_Init(ValueReplacer *Repl, FuelIRBuilder *builder)
{
	Repl->Count = 0;
	Repl->builder = builder;
	ARRAY_init(INodePtr, &Repl->Vals, 0);
}

static unsigned CommitReplaceValue(ValueReplacer *Repl)
{
	INodePtr *x, *e;
	FOR_EACH_ARRAY(Repl->Vals, x, e) {
		INode *oldNode = (INode *) *(x);
		INode *newNode = (INode *) *(x+1);
		ReplaceValueWith(Repl->builder, oldNode, newNode);
		++x;
	}
	ARRAY_dispose(INodePtr, &Repl->Vals);
	return Repl->Count;
}

static void ScheduleReplaceValue(ValueReplacer *Repl, INode *oldNode, INode *newNode, bool forceUpdate)
{
	if(forceUpdate) {
		ReplaceValueWith(Repl->builder, oldNode, newNode);
	} else {
		ARRAY_add(INodePtr, &Repl->Vals, oldNode);
		ARRAY_add(INodePtr, &Repl->Vals, newNode);
	}
}

static void Block_insertNode(Block *block, INodeList *List)
{
	unsigned shift = ARRAYp_size(List);
	if(shift) {
		unsigned size = ARRAY_size(block->insts);
		INode *Insts[size];
		ARRAY_ensureSize(INodePtr, &block->insts, size + shift + 1);
		memcpy(Insts, block->insts.list, sizeof(INodePtr)*size);
		memcpy(block->insts.list, List->list, sizeof(INodePtr)*shift);
		memcpy(block->insts.list+shift, Insts, sizeof(INodePtr)*size);
		ARRAY_size(block->insts) = ARRAY_size(block->insts) + shift;
	}
}

static void FoldStdCall(FuelIRBuilder *builder)
{
	INodeList Modified;
	BlockPtr *x, *e;

	ValueReplacer Repl;
	ValueReplacer_Init(&Repl, builder);

	ARRAY_init(INodePtr, &Modified, 0);
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		INodePtr *Inst, *End;
		ARRAY_clear(Modified);
		FOR_EACH_ARRAY((*x)->insts, Inst, End) {
			if((*Inst)->Unused == 1)
				continue;
			INode *Node = FoldInst(builder, *Inst);
			if(Node) {
				ScheduleReplaceValue(&Repl, *Inst, Node, false);
				ARRAY_add(INodePtr, &Modified, Node);
			}
		}
		Block_insertNode(*x, &Modified);
	}
	CommitReplaceValue(&Repl);
	ARRAY_dispose(INodePtr, &Modified);
}

/* ------------------------------------------------------------------------- */
/* FoldCondBranch */

static void FoldCondBranch(FuelIRBuilder *builder)
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
					Block *TrueBB, *FalseBB;
					if(Cond->Value.bval) {
						TrueBB  = Br->ThenBB;
						FalseBB = Br->ElseBB;
					} else {
						TrueBB  = Br->ElseBB;
						FalseBB = Br->ThenBB;
					}
					RemoveEdge(*x, FalseBB);
					*NodePtr = (INode *) builder->API->newJump(builder, TrueBB);
				}
			}
		}
	}
}

static void LinkBlocks1(FuelIRBuilder *builder, Block *BB, bool IsVisited)
{
	while(Block_IsVisited(BB) != IsVisited) {
		Block_SetVisited(BB, IsVisited);
		INode *Inst = *(ARRAY_last(BB->insts));
		enum IRType Kind = Inst->Kind;
		assert(IsBranchInst(Inst));
		ARRAY_add(BlockPtr, &builder->Blocks, BB);
		switch(Kind) {
		case IR_TYPE_IBranch:
			AddEdge(BB, ((IBranch *)Inst)->ThenBB);
			AddEdge(BB, ((IBranch *)Inst)->ElseBB);
			LinkBlocks1(builder, ((IBranch *)Inst)->ThenBB, IsVisited);
			BB = ((IBranch *)Inst)->ElseBB;
			break;
		case IR_TYPE_ITest:
			AddEdge(BB, ((ITest *)Inst)->TargetBlock);
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
			AddEdge(BB, ((IJump *)Inst)->TargetBlock);
			BB = ((IJump *)Inst)->TargetBlock;
			break;
		default:
			assert(0 && "unreachable");
			break;
		}
	}
}

static void LinkBlocks(FuelIRBuilder *builder, Block *BB, bool IsVisited)
{
	if(ARRAY_size(builder->Blocks) > 0) {
		BlockPtr *x, *e;
		FOR_EACH_ARRAY(builder->Blocks, x, e) {
			ARRAY_clear((*x)->preds);
			ARRAY_clear((*x)->succs);
		}
		ARRAY_clear(builder->Blocks);
	}
	LinkBlocks1(builder, BB, IsVisited);
}

static void FoldCFG(FuelIRBuilder *builder)
{
	/* Fold Control Flow
	 * [Case1]
	 *     [Before]            [After]
	 * BB0: IUpdate $a $b | BB0: IUpdate $a $b
	 *      IJump BB1     |      iAdd $c $d
	 * BB1: iAdd $c $d    |      ...
	 *      ...           |
	 */
	BlockPtr *x, *e;
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		if(ARRAY_size((*x)->succs) == 1) {
			Block *Target = ARRAY_get(BlockPtr, &((*x)->succs), 0);
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
				INode_setParent(*Inst, (INode *) *x);
			}
		}
	}
}

/* ------------------------------------------------------------------------- */
static void RemoveInstructionAfterBranchInst(FuelIRBuilder *builder)
{
	BlockPtr *x, *e;
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
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

/* ------------------------------------------------------------------------- */
static void CopyIfBlockHasSingleInst(Block *BB, INode *LastInst, Block *Block)
{
	if(ARRAY_size(Block->insts) > 1)
		return;
	INode *Inst = ARRAY_get(INodePtr, &Block->insts, 0);
	if(CHECK_KIND(LastInst, IJump) && CHECK_KIND(Inst, IReturn)) {
		memcpy(LastInst, Inst, sizeof(INodeImpl));
	}
	else if(CHECK_KIND(LastInst, IJump) && CHECK_KIND(Inst, IJump)) {
		if(ARRAY_size(Block->preds) == 1 && ARRAY_size(Block->succs) == 1) {
			memcpy(LastInst, Inst, sizeof(INodeImpl));
		}
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

static void RemoveIndirectJumpBlock(FuelIRBuilder *builder, Block *BB, bool IsVisited)
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
			CopyIfBlockHasSingleInst(BB, ToINode(Branch), Branch->ThenBB);
			CopyIfBlockHasSingleInst(BB, ToINode(Branch), Branch->ElseBB);
			RemoveIndirectJumpBlock(builder, Branch->ThenBB, IsVisited);
			BB = Branch->ElseBB;
		}
		else if((Jump = CHECK_KIND(Inst, IJump)) != 0) {
			Block *NextBB = Jump->TargetBlock;
			CopyIfBlockHasSingleInst(BB, ToINode(Jump), NextBB);
			BB = NextBB;
		}
		else if((Test = CHECK_KIND(Inst, ITest)) != 0) {
			BB = Test->TargetBlock;
		}
		else if(CHECK_KIND(Inst, ITry) != 0) {
			assert(0 && "TODO");
		}
	}
}

/* ------------------------------------------------------------------------- */
static inline void INode_SetMarked(INode *Node, bool Val)
{
	Node->Marked = Val;
}

static inline bool INode_IsMarked(INode *Node)
{
	return Node->Marked;
}

static void TraceNode1(INode *Node)
{
	if(Node->Unused) {
		return;
	}
#define CASE(KIND) case IR_TYPE_##KIND:
	switch(Node->Kind) {
		case IR_TYPE_IConstant:
		case IR_TYPE_IArgument:
			break;
		CASE(IField) {
			IField *Inst = (IField *) Node;
			switch(Inst->Op) {
				case GlobalScope:
				case EnvScope:
				case FieldScope:
					INode_SetMarked(Node, 1);
					INode_SetMarked(Inst->Node, 1);
					break;
				case LocalScope:
					break;
			}
			break;
		}
		CASE(INew) {
			INode_SetMarked(Node, 1);
			break;
		}
		CASE(ICall) {
			ICall *Inst = (ICall *) Node;
			INode_SetMarked(Node, 1);
			INodePtr *x;
			unsigned i;
			FOR_EACH_ARRAY__(Inst->Params, x, i, 1) {
				INode_SetMarked(*x, 1);
			}
			break;
		}
		CASE(IFunction) {
			IFunction *Inst = (IFunction *) Node;
			INode_SetMarked(Node, 1);
			INode_SetMarked(Inst->Func, 1);
			INode_SetMarked(Inst->Env, 1);
			INode_SetMarked(Inst->Method, 1);
			break;
		}
		CASE(IUpdate) {
			IUpdate *Inst = (IUpdate *) Node;
			INode_SetMarked(Node, 1);
			INode_SetMarked(Inst->RHS, 1);
			break;
		}
		CASE(IBranch) {
			INode_SetMarked(Node, 1);
			INode_SetMarked(((IBranch *) Node)->Cond, 1);
			break;
		}
		CASE(ITest) {
			INode_SetMarked(Node, 1);
			INode_SetMarked((INode *)((ITest *) Node)->Value, 1);
			assert(0 && "TODO");
			break;
		}
		CASE(IReturn) {
			IReturn *Inst = (IReturn *) Node;
			INode_SetMarked(Node, 1);
			if(Inst->Inst) {
				INode_SetMarked(Inst->Inst, 1);
			}
			break;
		}
		CASE(IJump) {
			INode_SetMarked(Node, 1);
			break;
		}
		CASE(IThrow) {
			IThrow *Inst = (IThrow *) Node;
			INode_SetMarked(Node, 1);
			INode_SetMarked((INode *) Inst->Val, 1);
			break;
		}
		CASE(ITry) {
			INode_SetMarked(Node, 1);
			assert(0 && "TODO");
			break;
		}
		CASE(IYield) {
			INode_SetMarked(Node, 1);
			INode_SetMarked(((IYield *) Node)->Value, 1);
			break;
		}
		CASE(IUnary) {
			INode_SetMarked(((IUnary *) Node)->Node, 1);
			break;
		}
		CASE(IBinary) {
			IBinary *Inst  = (IBinary *) Node;
			INode_SetMarked(Inst->LHS, 1);
			INode_SetMarked(Inst->RHS, 1);
			break;
		}
		CASE(IPHI) {
			IPHI *Inst  = (IPHI *) Node;
			INode_SetMarked(Node, 1);
			INode_SetMarked((INode *) Inst->Val, 1);

			INodePtr *x, *e;
			FOR_EACH_ARRAY(Inst->Args, x, e) {
				INode_SetMarked(*x, 1);
			}
			break;
		}
		default:
			assert(0 && "unreachable");
#undef CASE
	}
}

static void RemoveUnusedVariable(FuelIRBuilder *builder)
{
	BlockPtr *x, *e;
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		INodePtr *Inst, *End;
		FOR_EACH_ARRAY((*x)->insts, Inst, End) {
			INode_SetMarked(*Inst, 0);
		}
	}

	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		INodePtr *Inst, *End;
		INode_SetMarked((INode *)(*x), 1);
		FOR_EACH_ARRAY((*x)->insts, Inst, End) {
			TraceNode1(*Inst);
		}
	}

	INodeList Insts;
	ARRAY_init(INodePtr, &Insts, 32);
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		INodePtr *Inst, *End;
		FOR_EACH_ARRAY((*x)->insts, Inst, End) {
			if(INode_IsMarked(*Inst)) {
				ARRAY_add(INodePtr, &Insts, *Inst);
			}
		}
		INodeList *list = &(*x)->insts;
		memcpy(list->list, Insts.list, sizeof(INodePtr)*ARRAY_size(Insts));
		list->size = ARRAY_size(Insts);
		Insts.size = 0;
	}
	ARRAY_dispose(INodePtr, &Insts);
}

/* ------------------------------------------------------------------------- */
/* FoldPHIInst */

#ifdef FUELVM_USE_LLVM
static void FoldPHIInst1(IPHI *PHI)
{
	INodePtr *x, *e;
	INode *Val  = PHI->Val;
	INode *Inst = *ARRAY_n(PHI->Args, 0);
	FOR_EACH_ARRAY(PHI->Args, x, e) {
		if(Inst != *x)
			return;
		++x;
	}
	ARRAY_dispose(INodePtr, &PHI->Args);
	IUpdate *Node = (IUpdate *) PHI;
	Node->base.Kind = IR_TYPE_IUpdate;
	Node->LHS = (IField *) Val;
	Node->RHS = Inst;
}

static void FoldPHIInst(FuelIRBuilder *builder)
{
	BlockPtr *x, *e;
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		INodePtr *Inst, *End;
		FOR_EACH_ARRAY((*x)->insts, Inst, End) {
			if(CHECK_KIND(*Inst, IPHI)) {
				FoldPHIInst1((IPHI *) *Inst);
			}
		}
	}
}
#endif

/* ------------------------------------------------------------------------- */
/* ConstantPropagation */

#ifdef FUELVM_USE_LLVM
static void ConstantPropagation(FuelIRBuilder *builder)
{
	/* FIXME(ide) This rountine is very very slow with complicated control flow. */
#if 0
	BlockPtr *x, *e;
	ValueReplacer Repl;
	ValueReplacer_Init(&Repl, builder);

	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		INodePtr *Inst, *End;
		FOR_EACH_ARRAY((*x)->insts, Inst, End) {
			IUpdate *Update;
			if((Update = CHECK_KIND(*Inst, IUpdate))) {
				if(Update->LHS->Op == LocalScope) {
					if(CHECK_KIND(Update->RHS, IConstant)) {
						//ScheduleReplaceValue(&Repl, *Inst, Update->RHS, false);
					}
				}
			}
		}
	}
	CommitReplaceValue(&Repl);
#endif
}
#endif

/* ------------------------------------------------------------------------- */
extern void InsertPHINode(FuelIRBuilder *builder);

void IRBuilder_Optimize(FuelIRBuilder *builder, Block *BB, bool NeedPHI)
{
	bool Flag = false;
	LinkBlocks(builder, BB, Flag); Flag = !Flag;
	RemoveInstructionAfterBranchInst(builder);
	LinkBlocks(builder, BB, Flag); Flag = !Flag;

	FoldCondBranch(builder);
	FoldStdCall(builder);
	RemoveInstructionAfterBranchInst(builder);
	FoldCFG(builder);
	RemoveIndirectJumpBlock(builder, BB, Flag); Flag = !Flag;
	LinkBlocks(builder, BB, Flag); Flag = !Flag;
#ifdef FUELVM_USE_LLVM
	if(NeedPHI) {
		InsertPHINode(builder);
		FoldPHIInst(builder);
		ConstantPropagation(builder);
		FoldStdCall(builder);
		FoldStdCall(builder);
	}
#endif
	RemoveUnusedVariable(builder);
}

#ifdef __cplusplus
} /* extern "C" */
#endif
