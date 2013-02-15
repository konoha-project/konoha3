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
#include "FuelVM.h"

#ifdef __cplusplus
extern "C" {
#endif

DEF_ARRAY_OP_NOPOINTER(INodePtr);

static void kNode_SetLabelBlock(KonohaContext *kctx, kNode *stmt, ksymbol_t label, Block *block)
{
	KLIB kObjectProto_SetUnboxValue(kctx, stmt, label, KType_Int, (uintptr_t) block);
}

static Block *kNode_GetTargetBlock(KonohaContext *kctx, kNode *stmt, ksymbol_t keyword)
{
	KKeyValue *kvs = KLIB kObjectProto_GetKeyValue(kctx, stmt, keyword);
	if(kvs != 0) {
		return (Block *) kvs->unboxValue;
	}
	return 0;
}

/*----------------------------------------------------------------------------*/
/* Visitor */

struct KBuilder { /* FuelVM Builder */
	struct KBuilderCommon common;
	FuelIRBuilder *builder;
	Block *Parent;
	INode *Value;
};

#define BLD(BUILDER) ((BUILDER)->builder)
#define NotSupportedAPI() assert(0 && "NotSupportedAPI")
#define TODO()            assert(0 && "TODO")

static INode *FuelVM_getExpression(KBuilder *builder)
{
	INode *Expr = builder->Value;
	builder->Value = NULL;
	assert(Expr != NULL);
	return Expr;
}

#define MN_opNOT  KMethodName_("!")
#define MN_opADD  KMethodName_("+")
#define MN_opSUB  KMethodName_("-")
#define MN_opMUL  KMethodName_("*")
#define MN_opDIV  KMethodName_("/")
#define MN_opMOD  KMethodName_("%")
#define MN_opEQ   KMethodName_("==")
#define MN_opNEQ  KMethodName_("!=")
#define MN_opLT   KMethodName_("<")
#define MN_opLTE  KMethodName_("<=")
#define MN_opGT   KMethodName_(">")
#define MN_opGTE  KMethodName_(">=")
#define MN_opLAND KMethodName_("&")
#define MN_opLOR  KMethodName_("|")
#define MN_opLXOR KMethodName_("^")
#define MN_opLSFT KMethodName_("<<")
#define MN_opRSFT KMethodName_(">>")

static enum BinaryOp KMethodName_toBinaryOperator(KonohaContext *kctx, kmethodn_t mn)
{
	if(mn == MN_opADD ) return Add;
	if(mn == MN_opSUB ) return Sub;
	if(mn == MN_opMUL ) return Mul;
	if(mn == MN_opDIV ) return Div;
	if(mn == MN_opMOD ) return Mod;
	if(mn == MN_opEQ  ) return Eq;
	if(mn == MN_opNEQ ) return Nq;
	if(mn == MN_opLT  ) return Lt;
	if(mn == MN_opLTE ) return Le;
	if(mn == MN_opGT  ) return Gt;
	if(mn == MN_opGTE ) return Ge;
	if(mn == MN_opLAND) return And;
	if(mn == MN_opLOR ) return Or;
	if(mn == MN_opLXOR) return Xor;
	if(mn == MN_opLSFT) return LShift;
	if(mn == MN_opRSFT) return RShift;
	return BinaryOp_NotFound;
}

static enum UnaryOp KMethodName_toUnaryOperator(KonohaContext *kctx, kmethodn_t mn)
{
	if(mn == MN_opSUB) return Neg;
	if(mn == MN_opNOT) return Not;
	return UnaryOp_NotFound;
}

static INode *FetchINode(KonohaContext *kctx, KBuilder *builder, kNode *expr, unsigned idx, ktypeattr_t reqTy, void *thunk)
{
	kNode *exprN = kNode_At(expr, idx);
	SUGAR VisitNode(kctx, builder, exprN, thunk);
	INode *Node = FuelVM_getExpression(builder);
	assert(Node->Type != TYPE_void);
	return Node;
}

static INode *CreateSpecialInstruction(KonohaContext *kctx, KBuilder *builder, kMethod *mtd, kNode *expr, void *thunk)
{
	ktypeattr_t thisTy = mtd->typeId;
	kmethodn_t  mn     = mtd->mn;
	ktypeattr_t retTy  = kMethod_GetReturnType(mtd)->typeId;
	kParam     *params = kMethod_GetParam(mtd);
	kNode *stmt = expr->Parent;
	if(!IS_Node(stmt))
		return 0;
	if(thisTy == KType_Boolean) {
		if(params->psize == 0) { /* UnaryOperator */
			if(retTy == KType_Boolean) {
				/* booleaen booleaen.opNEG() */
				enum UnaryOp Op = KMethodName_toUnaryOperator(kctx, mn);
				INode *Param = FetchINode(kctx, builder, expr, 1, KType_Boolean, thunk);
				return CreateUnaryInst(BLD(builder), Op, Param, kNode_uline(expr));
			}
		}
		else if(params->psize == 1) { /* BinaryOperator */
			ktypeattr_t ptype = params->paramtypeItems[0].attrTypeId;
			if(retTy == KType_Boolean && ptype == KType_Boolean) {
				/* boolean boolean.(opEQ|opNE) (boolean x) */
				enum BinaryOp Op = KMethodName_toBinaryOperator(kctx, mn);
				assert(Op == Eq || Op == Nq);
				INode *LHS = FetchINode(kctx, builder, expr, 1, KType_Boolean, thunk);
				INode *RHS = FetchINode(kctx, builder, expr, 2, KType_Boolean, thunk);
				return CreateBinaryInst(BLD(builder), Op, LHS, RHS, kNode_uline(expr));
			}
		}
	}
	else if(thisTy == KType_Int) {
		if(params->psize == 0) { /* UnaryOperator */
			if(retTy == KType_Int) {
				/* int int.opSUB() */
				enum UnaryOp Op = KMethodName_toUnaryOperator(kctx, mn);
				INode *Param = FetchINode(kctx, builder, expr, 1, KType_Int, thunk);
				return CreateUnaryInst(BLD(builder), Op, Param, kNode_uline(expr));
			}
		}
		else if(params->psize == 1) { /* BinaryOperator */
			ktypeattr_t ptype = params->paramtypeItems[0].attrTypeId;
			if(retTy == KType_Boolean && ptype == KType_Int) {
				/* boolean int.(opEQ|opNE|opGT|opGE|opLT|opLE) (int x) */
				enum BinaryOp Op = KMethodName_toBinaryOperator(kctx, mn);
				INode *LHS = FetchINode(kctx, builder, expr, 1, KType_Int, thunk);
				INode *RHS = FetchINode(kctx, builder, expr, 2, KType_Int, thunk);
				return CreateBinaryInst(BLD(builder), Op, LHS, RHS, kNode_uline(expr));
			}
			else if(retTy == KType_Int && ptype == KType_Int) {
				/* int int.(opADD|opSUB|opMUL|opDIV|opMOD|opLSHIFT|opRSHIFT|opAND|opOR|opXOR) (int x) */
				enum BinaryOp Op = KMethodName_toBinaryOperator(kctx, mn);
				INode *LHS = FetchINode(kctx, builder, expr, 1, KType_Int, thunk);
				INode *RHS = FetchINode(kctx, builder, expr, 2, KType_Int, thunk);
				return CreateBinaryInst(BLD(builder), Op, LHS, RHS, kNode_uline(expr));
			}
		}
	}
	else if(FloatIsDefined() && thisTy == KType_float) {
		if(params->psize == 0) { /* UnaryOperator */
			if(retTy == KType_float) {
				/* int int.opSUB() */
				enum UnaryOp Op = KMethodName_toUnaryOperator(kctx, mn);
				INode *Param = FetchINode(kctx, builder, expr, 1, KType_float, thunk);
				return CreateUnaryInst(BLD(builder), Op, Param, kNode_uline(expr));
			}
		}
		else if(params->psize == 1) { /* BinaryOperator */
			ktypeattr_t ptype = params->paramtypeItems[0].attrTypeId;
			if(retTy == KType_Boolean && ptype == KType_float) {
				/* boolean float.(opEQ|opNE|opGT|opGE|opLT|opLE) (float x) */
				enum BinaryOp Op = KMethodName_toBinaryOperator(kctx, mn);
				INode *LHS = FetchINode(kctx, builder, expr, 1, KType_float, thunk);
				INode *RHS = FetchINode(kctx, builder, expr, 2, KType_float, thunk);
				return CreateBinaryInst(BLD(builder), Op, LHS, RHS, kNode_uline(expr));
			}
			else if(retTy == KType_float && ptype == KType_float) {
				/* float float.(opADD|opSUB|opMUL|opDIV) (float x) */
				enum BinaryOp Op = KMethodName_toBinaryOperator(kctx, mn);
				INode *LHS = FetchINode(kctx, builder, expr, 1, KType_float, thunk);
				INode *RHS = FetchINode(kctx, builder, expr, 2, KType_float, thunk);
				return CreateBinaryInst(BLD(builder), Op, LHS, RHS, kNode_uline(expr));
			}
		}
	}
	return 0;
}

/*----------------------------------------------------------------------------*/
/* Visitor API */
static kbool_t FuelVM_VisitDoneNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	return true;
}

static kbool_t FuelVM_VisitBoxNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	/*
	 * [box] := box(this)
	 **/
	enum TypeId Type = ConvertToTypeId(kctx, expr->attrTypeId);
	Type = ToBoxType(Type);
	SUGAR VisitNode(kctx, builder, expr->NodeToPush, thunk);
	INode *Node = FuelVM_getExpression(builder);
	INode *Inst = CreateUnaryInst(BLD(builder), Box, Node, kNode_uline(expr));
	INode_setType(Inst, Type);
	builder->Value = Inst;
	return true;
}

static kbool_t FuelVM_VisitPushNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	assert(0 && "Not Implemented");
	return true;
}

static kbool_t FuelVM_VisitErrorNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	kString *Err  = kNode_getErrorMessage(kctx, stmt);
	INode   *Node = CreateObject(BLD(builder), TYPE_String, (void *)Err);
	CreateThrow(BLD(builder), Node, KException_("RuntimeScript"), SoftwareFault, kNode_uline(stmt));
	return true;
}

static kbool_t FuelVM_VisitThrowNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	assert(0 && "Not Implemented");
	return true;
}

static kbool_t FuelVM_VisitBlockNode(KonohaContext *kctx, KBuilder *builder, kNode *block, void *thunk)
{
	unsigned size = ARRAY_size(BLD(builder)->Stack);
	Block *NewBlock = CreateBlock(BLD(builder));
	IRBuilder_JumpTo(BLD(builder), NewBlock);
	IRBuilder_setBlock(BLD(builder), NewBlock);
	size_t i;
	for(i = 0; i < kNode_GetNodeListSize(kctx, block); i++) {
		kNode *stmt = block->NodeList->NodeItems[i];
		builder->common.uline = kNode_uline(stmt);
		if(!SUGAR VisitNode(kctx, builder, stmt, thunk))
			break;
	}
	ARRAY_size(BLD(builder)->Stack) = size;
	if(builder->Value) {
		INode *Node = FuelVM_getExpression(builder);
		builder->Value = Node;
	}
	return true;
}

static kbool_t FuelVM_VisitReturnNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	kNode *expr = SUGAR kNode_GetNode(kctx, stmt, KSymbol_ExprPattern, NULL);
	if(expr != NULL && IS_Node(expr) && expr->attrTypeId != KType_void) {
		SUGAR VisitNode(kctx, builder, expr, thunk);
		INode *Ret  = FuelVM_getExpression(builder);
		INode *Inst = CreateReturn(BLD(builder), Ret);
		INode_setType(Inst, ConvertToTypeId(kctx, expr->attrTypeId));
	} else {
		CreateReturn(BLD(builder), 0);
	}
	return true;
}

static kbool_t FuelVM_VisitIfNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	kNode *expr    = kNode_getFirstNode(kctx, stmt);
	Block *ThenBB  = CreateBlock(BLD(builder));
	Block *ElseBB  = CreateBlock(BLD(builder));
	Block *MergeBB = CreateBlock(BLD(builder));
	/* if */
	SUGAR VisitNode(kctx, builder, expr, thunk);
	CreateBranch(BLD(builder), FuelVM_getExpression(builder), ThenBB, ElseBB);
	{ /* then */
		IRBuilder_setBlock(BLD(builder), ThenBB);
		SUGAR VisitNode(kctx, builder, kNode_getFirstBlock(kctx, stmt), thunk);
		if(!Block_HasTerminatorInst(BLD(builder)->Current)) {
			IRBuilder_JumpTo(BLD(builder), MergeBB);
		}
	}
	{ /* else */
		IRBuilder_setBlock(BLD(builder), ElseBB);
		SUGAR VisitNode(kctx, builder, kNode_getElseBlock(kctx, stmt), thunk);
		if(!Block_HasTerminatorInst(BLD(builder)->Current)) {
			IRBuilder_JumpTo(BLD(builder), MergeBB);
		}
	}
	/* endif */
	IRBuilder_setBlock(BLD(builder), MergeBB);
	return true;
}

enum LoopType {
	WhileLoop,
	DoWhileLoop,
	ForLoop
};

static kbool_t FuelVM_VisitLoopNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk, enum LoopType Loop)
{
	Block *HeadBB  = CreateBlock(BLD(builder));
	Block *BodyBB  = CreateBlock(BLD(builder));
	Block *ItrBB   = CreateBlock(BLD(builder));
	Block *MergeBB = CreateBlock(BLD(builder));

	kNode_SetLabelBlock(kctx, stmt, KSymbol_("continue"), ItrBB);
	kNode_SetLabelBlock(kctx, stmt, KSymbol_("break"),    MergeBB);

	kNode *itrBlock = SUGAR kNode_GetNode(kctx, stmt, KSymbol_("Iterator"), NULL);
	if(itrBlock != NULL) {
		assert(Loop == ForLoop);
		IRBuilder_JumpTo(BLD(builder), HeadBB);
	}
	else if(Loop == WhileLoop) {
		/* [WhileStmt]
		 * "Head" is Join Node
		 * Head  : if(COND) { goto Body } else {goto Merge }
		 * Body  : Body
		 *         ...
		 *         goto Itr
		 * Itr   : Body3
		 *         goto Head
		 * Merge : ...
		 */
		IRBuilder_JumpTo(BLD(builder), HeadBB);
	} else {
		/* [LoopStmt] (e.g. do-while loop)
		 * "Body" is Join Node
		 * Body  : Body
		 *         ...
		 *         goto Itr
		 * Itr   : Body3
		 *         goto Head
		 * Head  : if(COND) { goto Body } else {goto Merge }
		 * Merge : ...
		 */
		IRBuilder_JumpTo(BLD(builder), BodyBB);
	}

	{ /* Head */
		IRBuilder_setBlock(BLD(builder), HeadBB);
		SUGAR VisitNode(kctx, builder, kNode_getFirstNode(kctx, stmt), thunk);
		CreateBranch(BLD(builder), FuelVM_getExpression(builder), BodyBB, MergeBB);
	}

	{ /* Body */
		IRBuilder_setBlock(BLD(builder), BodyBB);
		SUGAR VisitNode(kctx, builder, kNode_getFirstBlock(kctx, stmt), thunk);
		IRBuilder_JumpTo(BLD(builder), ItrBB);

		/* Itr */
		IRBuilder_setBlock(BLD(builder), ItrBB);
		if(itrBlock != NULL) {
			SUGAR VisitNode(kctx, builder, itrBlock, thunk);
		}
		IRBuilder_JumpTo(BLD(builder), HeadBB);
	}

	IRBuilder_setBlock(BLD(builder), MergeBB);
	return true;
}

static kbool_t FuelVM_VisitWhileNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	return FuelVM_VisitLoopNode(kctx, builder, stmt, thunk, WhileLoop);
}

static kbool_t FuelVM_VisitDoWhileNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	return FuelVM_VisitLoopNode(kctx, builder, stmt, thunk, DoWhileLoop);
}

static kbool_t FuelVM_VisitForNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	kNode *initNode = kNode_GetNode(kctx, stmt, KSymbol_("init"));
	if(initNode != NULL) {
		SUGAR VisitNode(kctx, builder, initNode, thunk);
	}
	return FuelVM_VisitLoopNode(kctx, builder, stmt, thunk, ForLoop);
}

static kbool_t FuelVM_VisitJumpNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk, ksymbol_t label)
{
	kNode *jump = kNode_GetNode(kctx, stmt, label);
	DBG_ASSERT(jump != NULL && IS_Node(jump));
	Block *target = kNode_GetTargetBlock(kctx, jump, label);
	IRBuilder_JumpTo(BLD(builder), target);
	return true;
}

static kbool_t FuelVM_VisitContinueNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	ksymbol_t label = KSymbol_("continue");
	return FuelVM_VisitJumpNode(kctx, builder, stmt, thunk, label);
}

static kbool_t FuelVM_VisitBreakNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	ksymbol_t label = KSymbol_("break");
	return FuelVM_VisitJumpNode(kctx, builder, stmt, thunk, label);
}

static kbool_t FuelVM_VisitTryNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	//FIXME
	TODO();
	return true;
}

static kbool_t FuelVM_VisitConstNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	kObject *v = expr->ObjectConstValue;
	DBG_ASSERT(!KType_Is(UnboxType, expr->attrTypeId));
	builder->Value = CreateObject(BLD(builder), expr->attrTypeId, (void *)v);
	return true;
}

static kbool_t FuelVM_VisitUnboxConstNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	DBG_ASSERT(KType_Is(UnboxType, expr->attrTypeId));

	SValue Val = {};
	Val.bits = expr->unboxConstValue;
	builder->Value = CreateConstant(BLD(builder), ConvertToTypeId(kctx, expr->attrTypeId), Val);
	return true;
}

static kbool_t FuelVM_VisitNewNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	enum TypeId Type = ConvertToTypeId(kctx, expr->attrTypeId);
	INode *Expr = CreateNew(BLD(builder), expr->unboxConstValue, Type);
	builder->Value = Expr;
	return true;
}

static kbool_t FuelVM_VisitNullNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	SValue Val = {};
	ktypeattr_t type = expr->attrTypeId;
	if(KType_Is(UnboxType, type)) {
		Val.bits = 0;
	} else {
		Val.ptr = (void *) KLIB Knull(kctx, KClass_(type));
	}
	builder->Value = CreateConstant(BLD(builder), ConvertToTypeId(kctx, expr->attrTypeId), Val);
	return true;
}

static kbool_t FuelVM_VisitLocalNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	INode *Inst = NULL;
	if((Inst = IRBuilder_FindLocalVarByHash(BLD(builder), ConvertToTypeId(kctx, expr->attrTypeId), expr->index)) == 0) {
		Inst = CreateLocal(BLD(builder), ConvertToTypeId(kctx, expr->attrTypeId));
		IField_setHash((IField *) Inst, expr->index);
	}
	builder->Value = Inst;
	return true;
}

static kbool_t FuelVM_VisitFieldNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	INode *Node;
	kshort_t index  = (kshort_t)expr->index;
	kshort_t xindex = (kshort_t)(expr->index >> (sizeof(kshort_t)*8));
	enum TypeId Type = ConvertToTypeId(kctx, expr->attrTypeId);
	if((Node = IRBuilder_FindLocalVarByHash(BLD(builder), TYPE_Object, index)) == 0) {
		Node = CreateLocal(BLD(builder), TYPE_Object);
		IField_setHash((IField *) Node, index);
	}
	Node = CreateField(BLD(builder), FieldScope, Type, Node, xindex);
	builder->Value = Node;
	return true;
}

static kbool_t FuelVM_VisitMethodCallNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	kMethod *mtd = CallNode_getMethod(expr);
	DBG_ASSERT(IS_Method(mtd));
	enum TypeId Type = ConvertToTypeId(kctx, expr->attrTypeId);
	if(mtd->mn == KMethodName_("box")) {
		Type = ToBoxType(Type);
	}

	/*
	 * [CallExpr] := this.method(arg1, arg2, ...)
	 * expr->NodeList = [method, this, arg1, arg2, ...]
	 **/
	INode *Inst;
	if((Inst = CreateSpecialInstruction(kctx, builder, mtd, expr, thunk)) != 0) {
		INode_setType(Inst, Type);
		builder->Value = Inst;
		return true;
	}
	int i, s = kMethod_Is(Static, mtd) ? 2 : 1;
	int argc = CallNode_getArgCount(expr);
	INode *Params[argc+2];

	if(kMethod_Is(Static, mtd)) {
		kObject *obj = KLIB Knull(kctx, KClass_(mtd->typeId));
		INode *Self = CreateObject(BLD(builder), mtd->typeId, (void *) obj);
		Params[1] = Self;
	}
	for (i = s; i < argc + 2; i++) {
		kNode *exprN = kNode_At(expr, i);
		DBG_ASSERT(IS_Node(exprN));
		SUGAR VisitNode(kctx, builder, exprN, thunk);
		INode *Node = FuelVM_getExpression(builder);
		assert(Node->Type != TYPE_void);
		Params[i] = Node;
	}

	INode *MtdObj = CreateObject(BLD(builder), KType_Method, (void *) mtd);
	enum CallOp Op = (kMethod_Is(Virtual, mtd)) ? VirtualCall : DefaultCall;
	Params[0] = MtdObj;
	Inst = CreateICall(BLD(builder), Type, Op, kNode_uline(expr), Params, argc + 2);
	builder->Value = Inst;
	return true;
}

static void CreateCond(KonohaContext *kctx, KBuilder *builder, kNode *expr, enum ConditionalOp Op, void *thunk)
{
	kNode *LHS = kNode_At(expr, 1);
	kNode *RHS = kNode_At(expr, 2);

	Block *HeadBB  = CreateBlock(BLD(builder));
	Block *ThenBB  = CreateBlock(BLD(builder));
	Block *MergeBB = CreateBlock(BLD(builder));

	/* [CondExpr]
	 * LogicalAnd case
	 *       | goto Head
	 * Head  | let bval = LHS
	 *       | if(bval) { goto Then } else { goto Merge }
	 * Then  | bval = RHS
	 *       | goto Merge
	 * Merge | ...
	 */

	INode *Node;
	IRBuilder_JumpTo(BLD(builder), HeadBB);
	{ /* Head */
		IRBuilder_setBlock(BLD(builder), HeadBB);
		Node = CreateLocal(BLD(builder), TYPE_boolean);
		SUGAR VisitNode(kctx, builder, LHS, thunk);
		INode *Left = FuelVM_getExpression(builder);
		CreateUpdate(BLD(builder), Node, Left);

		if(Op == LogicalAnd) {
			CreateBranch(BLD(builder), Left, ThenBB, MergeBB);
		} else {
			CreateBranch(BLD(builder), Left, MergeBB, ThenBB);
		}
	}
	{ /* Then */
		IRBuilder_setBlock(BLD(builder), ThenBB);
		SUGAR VisitNode(kctx, builder, RHS, thunk);
		INode *Right = FuelVM_getExpression(builder);
		CreateUpdate(BLD(builder), Node, Right);
		IRBuilder_JumpTo(BLD(builder), MergeBB);
	}

	IRBuilder_setBlock(BLD(builder), MergeBB);
	builder->Value = Node;
}

static kbool_t FuelVM_VisitAndNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	CreateCond(kctx, builder, expr, LogicalAnd, thunk);
	return true;
}

static kbool_t FuelVM_VisitOrNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	CreateCond(kctx, builder, expr, LogicalOr, thunk);
	return true;
}

static kbool_t FuelVM_VisitAssignNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	/*
	 * [LetExpr] := lhs = rhs
	 * expr->NodeList = [NULL, lhs, rhs]
	 **/

	kNode *left = kNode_At(expr, 1);
	kNode *right = kNode_At(expr, 2);
	INode *Node;
	FuelIRBuilder *flbuilder = BLD(builder);
	if(kNode_node(left) == KNode_Local) {
		enum TypeId type = ConvertToTypeId(kctx, left->attrTypeId);
		if((Node = IRBuilder_FindLocalVarByHash(flbuilder, type, left->index)) == 0) {
			Node = CreateLocal(flbuilder, type);
			IField_setHash((IField *) Node, left->index);
		}
		SUGAR VisitNode(kctx, builder, right, thunk);
		INode *RHS = FuelVM_getExpression(builder);
		//if(RHS->Type != Node->Type) {
		//	INode_setType(Node, RHS->Type);
		//}
		CreateUpdate(flbuilder, Node, RHS);
	}
	else{
		assert(kNode_node(left) == KNode_Field);
		SUGAR VisitNode(kctx, builder, right, thunk);
		kshort_t index  = (kshort_t)left->index;
		kshort_t xindex = (kshort_t)(left->index >> (sizeof(kshort_t)*8));

		INode *Left;
		if((Left = IRBuilder_FindLocalVarByHash(BLD(builder), TYPE_Object, index)) == 0) {
			Left = CreateLocal(BLD(builder), TYPE_Object);
			IField_setHash((IField *) Left, index);
		}
		enum TypeId type = ConvertToTypeId(kctx, left->attrTypeId);
		Node = CreateField(BLD(builder), FieldScope, type, Left, xindex);
		SUGAR VisitNode(kctx, builder, right, thunk);
		CreateUpdate(BLD(builder), Node, FuelVM_getExpression(builder));
	}
	builder->Value = Node;
	return true;
}

static kbool_t FuelVM_VisitFunctionNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	/*
	 * [FunctionExpr] := new Function(method, env1, env2, ...)
	 * expr->NodeList = [method, defObj, env1, env2, ...]
	 **/
	enum TypeId Type;
	kMethod *mtd = CallNode_getMethod(expr);
	kObject *obj = expr->NodeList->ObjectItems[1];
	INode *MtdObj = CreateObject(BLD(builder), KType_Method, (void *) mtd);

	Type = ConvertToTypeId(kctx, kObject_class(obj)->typeId);
	INode *NewEnv  = CreateNew(BLD(builder), 0, Type);

	size_t i, ParamSize = kArray_size(expr->NodeList)-2;
	for(i = 0; i < ParamSize; i++) {
		kNode *envN = kNode_At(expr, i+2);
		enum TypeId FieldType = ConvertToTypeId(kctx, envN->attrTypeId);
		INode *Node = CreateField(BLD(builder), FieldScope, FieldType, NewEnv, i);
		SUGAR VisitNode(kctx, builder, envN, thunk);
		CreateUpdate(BLD(builder), Node, FuelVM_getExpression(builder));
	}

	Type = ConvertToTypeId(kctx, expr->attrTypeId);
	INode *NewFunc = CreateNew(BLD(builder), 0, Type);

	kNameSpace *ns = kNode_ns(expr);
	mtd =  KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, KClass_Func, KMethodName_("_Create"), 2, KMethodMatch_NoOption);

	INode *CallMtd = CreateObject(BLD(builder), KType_Method, (void *) mtd);
	INode *Params[4];
	Params[0] = CallMtd;
	Params[1] = NewFunc;
	Params[2] = NewEnv;
	Params[3] = MtdObj;
	builder->Value = CreateICall(BLD(builder), Type, DefaultCall, kNode_uline(expr), Params, 4);

	return true;
}

/* end of Visitor */
/*----------------------------------------------------------------------------*/

static INode *SetUpArguments(KonohaContext *kctx, FuelIRBuilder *builder, kMethod *mtd)
{
	kParam *params = kMethod_GetParam(mtd);
	unsigned i;
	INode *Arg;
	INode *Val;
	INode *Arg0 = NULL;
	if(mtd->typeId != TYPE_void) {
		Arg = CreateArgument(builder, 0);
		Val = CreateLocal(builder, mtd->typeId);
		CreateUpdate(builder, Val, Arg);
		INode_setType(Arg, mtd->typeId);
		IField_setHash((IField *) Val, 0);
		Arg0 = Val;
	}
	for(i = 0; i < params->psize; ++i) {
		enum TypeId type = ConvertToTypeId(kctx, params->paramtypeItems[i].attrTypeId);
		Arg = CreateArgument(builder, i+1);
		Val = CreateLocal(builder, type);
		CreateUpdate(builder, Val, Arg);
		INode_setType(Arg, type);
		IField_setHash((IField *) Val, i+1);
	}
	return Arg0;
}

// -------------------------------------------------------------------------

static struct KVirtualCode *FuelVM_Run(KonohaContext *kctx, struct KonohaValueVar *sfp, struct KVirtualCode *pc)
{
	NotSupportedAPI();
	return NULL;
}

static KMETHOD FuelVM_RunVirtualMachine(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[K_MTDIDX].calledMethod;
	DBG_ASSERT(IS_Method(mtd));

	union ByteCode *code = (union ByteCode *) mtd->vcode_start;
	FuelVM_Exec(kctx, sfp, code);
}

static KMethodFunc FuelVM_GenerateMethodFunc(KonohaContext *kctx, struct KVirtualCode *vcode)
{
	return 0;
}

static struct KVirtualCode *GetDefaultBootCode(void)
{
	return NULL;
}

static KMethodFunc AbstractMethodPtr = 0;

static struct KVirtualCode *FuelVM_GenerateVirtualCode(KonohaContext *kctx, kMethod *mtd, kNode *block, int option)
{
	if(unlikely(AbstractMethodPtr == 0)) {
		AbstractMethodPtr = mtd->invokeKMethodFunc;
	}

	kNameSpace *ns = kNode_ns(block);
	KBuilder builderbuf = {}, *builder = &builderbuf;
	FuelIRBuilder Builder = {};
	INIT_GCSTACK();
	IRBuilder_Init(&Builder, kctx, ns);
	builder->builder = &Builder;
	builder->common.api = ns->builderApi;
	Block *EntryBlock = CreateBlock(BLD(builder));
	IRBuilder_setBlock(BLD(builder), EntryBlock);
	INode *Self = SetUpArguments(kctx, &Builder, mtd);

	SUGAR VisitNode(kctx, builder, block, NULL);

	if(!Block_HasTerminatorInst(BLD(builder)->Current)) {
		if(mtd->mn == MN_new) {
			INode *Ret = CreateReturn(BLD(builder), Self);
			INode_setType(Ret, ConvertToTypeId(kctx, mtd->typeId));
		} else {
			ktypeattr_t retTy = kMethod_GetReturnType(mtd)->typeId;
			if(retTy == KType_void) {
				CreateReturn(BLD(builder), 0);
			} else {
				enum TypeId Type = ConvertToTypeId(kctx, retTy);
				INode *Ret;
				if(KType_Is(UnboxType, retTy)) {
					SValue V; V.bits = 0;
					Ret = CreateConstant(BLD(builder), Type, V);
				} else {
					kObject *obj = KLIB Knull(kctx, KClass_(retTy));
					Ret = CreateObject(BLD(builder), Type, (void *)obj);
				}
				Ret = CreateReturn(BLD(builder), Ret);
				INode_setType(Ret, Type);
				CreateReturn(BLD(builder), 0);
			}
		}
	}
	RESET_GCSTACK();
	IMethod Mtd = {kctx, mtd, EntryBlock, ns};
	BLD(builder)->Method = &Mtd;
	bool JITCompiled = false;
	union ByteCode *code = IRBuilder_Compile(BLD(builder), &Mtd, option, &JITCompiled);
	if(mtd->invokeKMethodFunc == FuelVM_RunVirtualMachine) {
		mtd->virtualCodeApi_plus1[-1]->FreeVirtualCode(kctx, mtd->vcode_start);
	}
	KLIB kMethod_SetFunc(kctx, mtd, 0);
	if(JITCompiled) {
		KLIB kMethod_SetFunc(kctx, mtd, (KMethodFunc) code);
	}
	KFieldSet(mtd, ((kMethodVar *)mtd)->CompiledNode, block);
	IRBuilder_Exit(&Builder);
	return (struct KVirtualCode *) code;
}

void FuelVM_Recompile(KonohaContext *kctx, kMethod *mtd)
{
#ifdef FUELVM_USE_LLVM
	if(FuelVM_HasOptimizedCode(mtd) == false) {
		KLIB kMethod_GenCode(kctx, mtd, mtd->CompiledNode, O2Compile);
	}
#endif
}

static void FuelVM_SetMethodCode(KonohaContext *kctx, kMethodVar *mtd, struct KVirtualCode *vcode, KMethodFunc func)
{
	if(mtd->invokeKMethodFunc == AbstractMethodPtr) {
		KLIB kMethod_SetFunc(kctx, mtd, FuelVM_RunVirtualMachine);
		mtd->vcode_start = vcode;
	}
}

static void FuelVM_DeleteVirtualMachine(KonohaContext *kctx)
{
#ifdef FUELVM_USE_LLVM
	extern void ExitLLVM();
	ExitLLVM();
#endif
}

static const KModuleInfo ModuleInfo = {
	"FuelVM", K_VERSION, 0, "FuelVM",
};

static const struct KBuilderAPI *GetDefaultBuilderAPI(void);

static const struct ExecutionEngineModule FuelVM_Module = {
	&ModuleInfo,
	FuelVM_DeleteVirtualMachine,
	GetDefaultBuilderAPI,
	GetDefaultBootCode,
	FuelVM_GenerateVirtualCode,
	FuelVM_GenerateMethodFunc,
	FuelVM_SetMethodCode,
	FuelVM_Run
};

static const struct KBuilderAPI FuelVM_BuilderAPI = {
	"FuelVM",
	&FuelVM_Module,
#define DEFINE_BUILDER_API(NAME) FuelVM_Visit##NAME##Node,
	KNodeList(DEFINE_BUILDER_API)
#undef DEFINE_BUILDER_API
};

static const struct KBuilderAPI *GetDefaultBuilderAPI(void)
{
	return &FuelVM_BuilderAPI;
}

// -------------------------------------------------------------------------

kbool_t LoadFuelVMModule(KonohaFactory *factory, ModuleType type)
{
	memcpy(&factory->ExecutionEngineModule, &FuelVM_Module, sizeof(FuelVM_Module));
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
