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
		newVal->Parent = oldVal->Parent;
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
			assert(0 && "unreachable");
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
				CASE(Add); CASE(Sub); CASE(Mul);
				case Div: {
					if(unlikely(RHS->Value.ival == 0)) {
						KonohaContext *kctx = builder->Context;
						KBaseTrace(trace);
						KLIB KRuntime_raise(kctx, KException_("ZeroDivided"),
								SoftwareFault, NULL, trace->baseStack);
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
			return (INode *) builder->API->newConstant(builder, TYPE_int, ret);
		}
	}
	if(ToINode(LHS)->Type == TYPE_float) {
		if(ToINode(RHS)->Type == TYPE_float) {
			switch(Op) {
#define CASE(X) case X: ret = X##_float_float(LHS->Value, RHS->Value); break
				CASE(Add); CASE(Sub); CASE(Mul);
				case Div: {
					if(unlikely(RHS->Value.ival == 0)) {
						KonohaContext *kctx = builder->Context;
						KBaseTrace(trace);
						KLIB KRuntime_raise(kctx, KException_("ZeroDivided"),
								SoftwareFault, NULL, trace->baseStack);
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
			return (INode *) builder->API->newConstant(builder, TYPE_float, ret);
		}
	}
	return NULL;
}

static INode *SimplifyICond(FuelIRBuilder *builder, ICond *Inst)
{
	enum ConditionalOp Op = Inst->Op;
	INodePtr *x, *e;
	FOR_EACH_ARRAY(Inst->Insts, x, e) {
		if(!CHECK_KIND(*x, IConstant))
			return NULL;
	}
	SValue val = {}; val.bval = true;
	if(Op == LogicalAnd) {
		FOR_EACH_ARRAY(Inst->Insts, x, e) {
			IConstant *C = (IConstant *) *x;
			val.bval = val.bval && C->Value.bval;
		}
	} else if(Op == LogicalOr) {
		FOR_EACH_ARRAY(Inst->Insts, x, e) {
			IConstant *C = (IConstant *) *x;
			val.bval = val.bval || C->Value.bval;
		}
	} else {
		assert(0 && "unreachable");
	}
	return (INode *) builder->API->newConstant(builder, TYPE_boolean, val);
}

static INode *RemoveTrivialAssertion(FuelIRBuilder *builder, ICall *Inst)
{
	/* Remove NameSpace.assert(true) */
	KonohaContext *kctx = builder->Context;
	INode **MtdPtr = ARRAY_n(Inst->Params, 0);
	kMethod *mtd = (kMethod *) ((IConstant *) *MtdPtr)->Value.obj;
	if(mtd->typeId == TYPE_NameSpace && mtd->mn == KKMethodName_("assert")) {
		if(ARRAY_size(Inst->Params) != 3) {
			return 0;
		}
		IConstant *Val;
		if((Val = CHECK_KIND(*ARRAY_n(Inst->Params, 2), IConstant))) {
			if(Val->base.Type == TYPE_boolean && Val->Value.bval == true) {
				return (INode *) Val;
			}
		}
	}
	return 0;
}

static INode *SimplifyICall(FuelIRBuilder *builder, ICall *Inst)
{
	SValue val = {};
	unsigned i;
	INodePtr *x, *MtdPtr = ARRAY_n(Inst->Params, 0);
	kMethod *mtd = (kMethod *) ((IConstant *) *MtdPtr)->Value.obj;

	INode *ret = NULL;
	if((ret = RemoveTrivialAssertion(builder, Inst)) != 0) {
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
	KClass *kclass = kMethod_GetReturnType(mtd);
	unsigned psize = ARRAY_size(Inst->Params) - 1;
	BEGIN_UnusedStack(lsfp);

	FOR_EACH_ARRAY_(Inst->Params, x, i) {
		if(i == 0)
			continue;
		IConstant *C = (IConstant *) *x;
		if(IsUnBoxedType(C->base.Type)) {
			lsfp[i-1].unboxValue = C->Value.bits;
		} else {
			KUnsafeFieldSet(lsfp[i-1].asObject, C->Value.obj);
			lsfp[i-1].unboxValue = kObject_Unbox(C->Value.obj);
		}
	}
	KStackSetMethodAll(lsfp, KLIB Knull(kctx, kclass), Inst->uline, mtd, psize);
	KStackCall(lsfp);
	END_UnusedStack();

	enum TypeId Type = ConvertToTypeId(kctx, kclass->typeId);
	if(IsUnBoxedType(Type)) {
		val.bits = lsfp[K_RTNIDX].unboxValue;
	} else {
		val.obj  = lsfp[K_RTNIDX].asObject;
	}
	return (INode *) builder->API->newConstant(builder, Type, val);
}

static INode *SimplifyInst(FuelIRBuilder *builder, INode *Node)
{
	if(Node->Kind == IR_TYPE_ICond) {
		return SimplifyICond(builder, (ICond *) Node);
	}
	if(Node->Kind == IR_TYPE_ICall) {
		return SimplifyICall(builder, (ICall *) Node);
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

void IRBuilder_SimplifyStdCall(FuelIRBuilder *builder)
{
	ARRAY(INodePtr) Modified;
	BlockPtr *x, *e;
	ARRAY_init(INodePtr, &Modified, 0);
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		INodePtr *Inst, *End;
		ARRAY_clear(Modified);
		FOR_EACH_ARRAY((*x)->insts, Inst, End) {
			if((*Inst)->Unused == 1) {
				continue;
			}
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
					RemovedBB->base.Unused = 1;
					INodePtr *Inst, *End;
					FOR_EACH_ARRAY(RemovedBB->insts, Inst, End) {
						(*Inst)->Unused = 1;
					}
					*NodePtr = (INode *) builder->API->newJump(builder, TargetBB);
				}
			}
		}
	}
}

#ifdef __cplusplus
} /* extern "C" */
#endif
