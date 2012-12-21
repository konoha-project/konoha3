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

#include <minikonoha/minikonoha.h>
#include <minikonoha/klib.h>
#include <minikonoha/sugar.h>
#include "codegen.h"
#include "FuelVM.h"

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/
/* Konoha AST API */

static kNode *Node_getFirstNode(KonohaContext *kctx, kNode *stmt)
{
	return SUGAR kNode_GetNode(kctx, stmt, NULL, KSymbol_NodePattern, K_NULLBLOCK);
}

static kNode *Node_getElseNode(KonohaContext *kctx, kNode *stmt)
{
	return SUGAR kNode_GetNode(kctx, stmt, NULL, KSymbol_else, K_NULLBLOCK);
}

static kNode *Node_getFirstNode(KonohaContext *kctx, kNode *stmt)
{
	return SUGAR kNode_GetNode(kctx, stmt, KSymbol_NodePattern, NULL);
}

static kNode *kNode_GetNode(KonohaContext *kctx, kNode *stmt, ksymbol_t kw)
{
	return (kNode *) kNode_GetObject(kctx, stmt, kw, NULL);
}

static kMethod *CallNode_getMethod(kNode *expr)
{
	return expr->NodeList->MethodItems[0];
}

static int CallNode_getArgCount(kNode *expr)
{
	return kArray_size(expr->NodeList) - 2;
}

static kString *Node_getErrorMessage(KonohaContext *kctx, kNode *stmt)
{
	kString* msg = (kString *)kNode_GetObjectNULL(kctx, stmt, KSymbol_ERR);
	DBG_ASSERT(IS_String(msg));
	return msg;
}

static void kNode_SetLabelNode(KonohaContext *kctx, kNode *stmt, ksymbol_t label, Node *block)
{
	KLIB kObjectProto_SetUnboxValue(kctx, stmt, label, KType_int, (uintptr_t) block);
}

static Node *kNode_GetTargetNode(KonohaContext *kctx, kNode *stmt, ksymbol_t keyword)
{
	KKeyValue *kvs = KLIB kObjectProto_GetKeyValue(kctx, stmt, keyword);
	if(kvs != 0) {
		return (Node *) kvs->unboxValue;
	}
	return 0;
}

/*----------------------------------------------------------------------------*/
/* Visitor */

struct KBuilder { /* FuelVM Builder */
	struct KBuilderCommon common;
	FuelIRBuilder *builder;
	Node *Parent;
	INode *Value;
};

#define BLD(BUILDER) ((BUILDER)->builder)
#define NotSupportedAPI() assert(0 && "NotSupportedAPI")
#define TODO()            assert(0 && "TODO")

static INode *FuelVM_getExpression(KBuilder *builder)
{
	INode *Node = builder->Value;
	builder->Value = NULL;
	assert(Node != NULL);
	return Node;
}

#define MN_opNOT  KKMethodName_("!")
#define MN_opADD  KKMethodName_("+")
#define MN_opSUB  KKMethodName_("-")
#define MN_opMUL  KKMethodName_("*")
#define MN_opDIV  KKMethodName_("/")
#define MN_opMOD  KKMethodName_("%")
#define MN_opEQ   KKMethodName_("==")
#define MN_opNEQ  KKMethodName_("!=")
#define MN_opLT   KKMethodName_("<")
#define MN_opLTE  KKMethodName_("<=")
#define MN_opGT   KKMethodName_(">")
#define MN_opGTE  KKMethodName_(">=")
#define MN_opLAND KKMethodName_("&")
#define MN_opLOR  KKMethodName_("|")
#define MN_opLXOR KKMethodName_("^")
#define MN_opLSFT KKMethodName_("<<")
#define MN_opRSFT KKMethodName_(">>")

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

static INode *FetchINode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, kNode *expr, unsigned idx, ktypeattr_t reqTy)
{
	kNode *exprN = kNode_At(expr, idx);
	DBG_ASSERT(IS_Node(exprN));
	SUGAR VisitNode(kctx, builder, stmt, exprN);
	INode *Node = FuelVM_getExpression(builder);
	if(Node->Type == KType_void) {
		INode_setType(Node, ConvertToTypeId(kctx, reqTy));
	}
	return Node;
}

static INode *CreateSpecialInstruction(KonohaContext *kctx, KBuilder *builder, kMethod *mtd, kNode *stmt, kNode *expr)
{
	ktypeattr_t thisTy = mtd->typeId;
	kmethodn_t  mn     = mtd->mn;
	ktypeattr_t retTy  = kMethod_GetReturnType(mtd)->typeId;
	kParam     *params = kMethod_GetParam(mtd);
	if(thisTy == KType_int) {
		if(params->psize == 0) { /* UnaryOperator */
			if(retTy == KType_int) {
				/* int int.opSUB() */
				enum UnaryOp Op = KMethodName_toUnaryOperator(kctx, mn);
				INode *Param = FetchINode(kctx, builder, stmt, expr, 1, KType_int);
				return CreateUnaryInst(BLD(builder), Op, Param);
			}
		}
		else if(params->psize == 1) { /* BinaryOperator */
			ktypeattr_t ptype = params->paramtypeItems[0].attrTypeId;
			if(retTy == KType_boolean && ptype == KType_int) {
				/* boolean int.(opEQ|opNE|opGT|opGE|opLT|opLE) (int x) */
				enum BinaryOp Op = KMethodName_toBinaryOperator(kctx, mn);
				INode *LHS = FetchINode(kctx, builder, stmt, expr, 1, KType_int);
				INode *RHS = FetchINode(kctx, builder, stmt, expr, 2, KType_int);
				return CreateBinaryInst(BLD(builder), Op, LHS, RHS);
			}
			else if(retTy == KType_int && ptype == KType_int) {
				/* int int.(opADD|opSUB|opMUL|opDIV|opMOD|opLSHIFT|opRSHIFT|opAND|opOR|opXOR) (int x) */
				enum BinaryOp Op = KMethodName_toBinaryOperator(kctx, mn);
				INode *LHS = FetchINode(kctx, builder, stmt, expr, 1, KType_int);
				INode *RHS = FetchINode(kctx, builder, stmt, expr, 2, KType_int);
				return CreateBinaryInst(BLD(builder), Op, LHS, RHS);
			}
		}
	}
	else if(FloatIsDefined() && thisTy == KType_float) {
		if(params->psize == 0) { /* UnaryOperator */
			if(retTy == KType_float) {
				/* int int.opSUB() */
				enum UnaryOp Op = KMethodName_toUnaryOperator(kctx, mn);
				INode *Param = FetchINode(kctx, builder, stmt, expr, 1, KType_float);
				return CreateUnaryInst(BLD(builder), Op, Param);
			}
		}
		else if(params->psize == 1) { /* BinaryOperator */
			ktypeattr_t ptype = params->paramtypeItems[0].attrTypeId;
			if(retTy == KType_boolean && ptype == KType_float) {
				/* boolean float.(opEQ|opNE|opGT|opGE|opLT|opLE) (float x) */
				enum BinaryOp Op = KMethodName_toBinaryOperator(kctx, mn);
				INode *LHS = FetchINode(kctx, builder, stmt, expr, 1, KType_float);
				INode *RHS = FetchINode(kctx, builder, stmt, expr, 2, KType_float);
				return CreateBinaryInst(BLD(builder), Op, LHS, RHS);
			}
			else if(retTy == KType_float && ptype == KType_float) {
				/* float float.(opADD|opSUB|opMUL|opDIV) (float x) */
				enum BinaryOp Op = KMethodName_toBinaryOperator(kctx, mn);
				INode *LHS = FetchINode(kctx, builder, stmt, expr, 1, KType_float);
				INode *RHS = FetchINode(kctx, builder, stmt, expr, 2, KType_float);
				return CreateBinaryInst(BLD(builder), Op, LHS, RHS);
			}
		}
	}
	return 0;
}

/*----------------------------------------------------------------------------*/
/* Visitor API */
static bool FuelVM_VisitErrNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt)
{
	kString *Err  = Node_getErrorMessage(kctx, stmt);
	INode   *Node = CreateObject(BLD(builder), TYPE_String, (void *)Err);
	CreateThrow(BLD(builder), Node, kNode_uline(stmt));
	return true;
}

static bool FuelVM_VisitNodeNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt)
{
	SUGAR VisitNode(kctx, builder, stmt, Node_getFirstNode(kctx, stmt));
	return true;
}

static bool FuelVM_VisitNodeNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt)
{
	Node *NewNode = CreateNode(BLD(builder));
	IRBuilder_JumpTo(BLD(builder), NewNode);
	IRBuilder_setNode(BLD(builder), NewNode);
	SUGAR VisitNode(kctx, builder, Node_getFirstNode(kctx, stmt));
	return true;
}

static bool FuelVM_VisitReturnNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt)
{
	kNode *expr = SUGAR kNode_GetNode(kctx, stmt, KSymbol_NodePattern, NULL);
	if(expr != NULL && IS_Node(expr) && expr->attrTypeId != KType_void) {
		SUGAR VisitNode(kctx, builder, stmt, expr);
		INode *Ret  = FuelVM_getExpression(builder);
		INode *Inst = CreateReturn(BLD(builder), Ret);
		INode_setType(Inst, ConvertToTypeId(kctx, expr->attrTypeId));
	} else {
		CreateReturn(BLD(builder), 0);
	}
	return true;
}

static bool FuelVM_VisitIfNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt)
{
	kNode *expr    = Node_getFirstNode(kctx, stmt);
	Node *ThenBB  = CreateNode(BLD(builder));
	Node *ElseBB  = CreateNode(BLD(builder));
	Node *MergeBB = CreateNode(BLD(builder));
	/* if */
	SUGAR VisitNode(kctx, builder, stmt, expr);
	CreateBranch(BLD(builder), FuelVM_getExpression(builder), ThenBB, ElseBB);
	/* then */
	IRBuilder_setNode(BLD(builder), ThenBB);
	SUGAR VisitNode(kctx, builder, Node_getFirstNode(kctx, stmt));
	if(!Node_HasTerminatorInst(BLD(builder)->Current)) {
		IRBuilder_JumpTo(BLD(builder), MergeBB);
	}
	/* else */
	IRBuilder_setNode(BLD(builder), ElseBB);
	SUGAR VisitNode(kctx, builder, Node_getElseNode(kctx, stmt));
	if(!Node_HasTerminatorInst(BLD(builder)->Current)) {
		IRBuilder_JumpTo(BLD(builder), MergeBB);
	}
	/* endif */
	IRBuilder_setNode(BLD(builder), MergeBB);
	return true;
}

static bool FuelVM_VisitLoopNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt)
{
	Node *HeaderBB = CreateNode(BLD(builder));
	Node *BodyBB   = CreateNode(BLD(builder));
	Node *ItrBB    = CreateNode(BLD(builder));
	Node *MergeBB  = CreateNode(BLD(builder));
	/* Head  : if(COND) { goto Body } else {goto Merge }
	 * Body  :   Body1
	 *           Body2
	 *           goto Itr
	 * Itr   :   Body3
	 *           goto Header
	 * Merge : ...
	 */

	kNode_SetLabelNode(kctx, stmt, KSymbol_("continue"), ItrBB);
	kNode_SetLabelNode(kctx, stmt, KSymbol_("break"),    MergeBB);

	if(kNode_Is(RedoLoop, stmt)) {
		IRBuilder_JumpTo(BLD(builder), BodyBB);
	} else {
		IRBuilder_JumpTo(BLD(builder), HeaderBB);
	}

	IRBuilder_setNode(BLD(builder), HeaderBB);
	SUGAR VisitNode(kctx, builder, stmt, Node_getFirstNode(kctx, stmt));
	CreateBranch(BLD(builder), FuelVM_getExpression(builder), BodyBB, MergeBB);

	IRBuilder_setNode(BLD(builder), BodyBB);
	SUGAR VisitNode(kctx, builder, Node_getFirstNode(kctx, stmt));
	IRBuilder_JumpTo(BLD(builder), ItrBB);

	IRBuilder_setNode(BLD(builder), ItrBB);
	kNode *itrNode = SUGAR kNode_GetNode(kctx, stmt, NULL, KSymbol_("Iterator"), NULL);
	if(itrNode != NULL) {
		SUGAR VisitNode(kctx, builder, itrNode);
	}

	IRBuilder_JumpTo(BLD(builder), HeaderBB);
	IRBuilder_setNode(BLD(builder), MergeBB);
	return true;
}

static bool FuelVM_VisitJumpNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt)
{
	KSyntax *syn = stmt->syn;
	kNode *jump = kNode_GetNode(kctx, stmt, syn->keyword);
	DBG_ASSERT(jump != NULL && IS_Node(jump));
	Node *target = kNode_GetTargetNode(kctx, jump, syn->keyword);
	IRBuilder_JumpTo(BLD(builder), target);
	return true;
}

static bool FuelVM_VisitTryNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt)
{
	//FIXME
	TODO();
	return true;
}


static bool FuelVM_VisitUndefinedNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt)
{
	DBG_P("undefined asm syntax kw='%s'", KSymbol_text(stmt->syn->keyword));
	return true;
}

static void FuelVM_VisitConstNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, kNode *expr)
{
	kObject *v = expr->ObjectConstValue;
	DBG_ASSERT(!KType_Is(UnboxType, expr->attrTypeId));
	DBG_ASSERT(KNode_Is(ObjectConst, (expr));
	builder->Value = CreateObject(BLD(builder), expr->attrTypeId, (void *)v);
}

static void FuelVM_VisitNConstNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, kNode *expr)
{
	DBG_ASSERT(KType_Is(UnboxType, expr->attrTypeId));
	DBG_ASSERT(!KNode_Is(ObjectConst, (expr));

	SValue Val = {};
	Val.bits = expr->unboxConstValue;
	builder->Value = CreateConstant(BLD(builder), ConvertToTypeId(kctx, expr->attrTypeId), Val);
}

static void FuelVM_VisitNewNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, kNode *expr)
{
	enum TypeId Type = ConvertToTypeId(kctx, expr->attrTypeId);
	INode *Node = CreateNew(BLD(builder), expr->unboxConstValue, Type);
	builder->Value = Node;
}

static void FuelVM_VisitNullNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, kNode *expr)
{
	SValue Val = {};
	ktypeattr_t type = expr->attrTypeId;
	if(KType_Is(UnboxType, type)) {
		Val.bits = 0;
	} else {
		Val.ptr = (void *) KLIB Knull(kctx, KClass_(type));
	}
	builder->Value = CreateConstant(BLD(builder), ConvertToTypeId(kctx, expr->attrTypeId), Val);
}

static void FuelVM_VisitLocalNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, kNode *expr)
{
	INode *Inst;
	if((Inst = IRBuilder_FindLocalVarByHash(BLD(builder), ConvertToTypeId(kctx, expr->attrTypeId), expr->index)) == 0) {
		Inst = CreateLocal(BLD(builder), ConvertToTypeId(kctx, expr->attrTypeId));
		IField_setHash((IField *) Inst, expr->index);
	}
	builder->Value = Inst;
}

static void FuelVM_VisitNodeNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, kNode *expr)
{
	DBG_ASSERT(IS_Node(expr->block));
	TODO();
	//SUGAR VisitNode(kctx, builder, expr->block);
	//KBuilder_AsmNMOV(kctx, builder, a, expr->attrTypeId, espidx);
}

static void FuelVM_VisitFieldNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, kNode *expr)
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
}

static void FuelVM_VisitCallNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, kNode *expr)
{
	kMethod *mtd = CallNode_getMethod(expr);
	DBG_ASSERT(IS_Method(mtd));

	/*
	 * [CallNode] := this.method(arg1, arg2, ...)
	 * expr->NodeList = [method, this, arg1, arg2, ...]
	 **/
	INode *Inst;
	if((Inst = CreateSpecialInstruction(kctx, builder, mtd, stmt, expr)) != 0) {
		goto L_finally;
	}
	{
		int i, s = kMethod_Is(Static, mtd) ? 2 : 1;
		int argc = CallNode_getArgCount(expr);
		INode *Params[argc+2];
		kParam *params = kMethod_GetParam(mtd);

		if(kMethod_Is(Static, mtd)) {
			kObject *obj = KLIB Knull(kctx, KClass_(mtd->typeId));
			INode *Self = CreateObject(BLD(builder), mtd->typeId, (void *) obj);
			Params[1] = Self;
		}
		for (i = s; i < argc + 2; i++) {
			kNode *exprN = kNode_At(expr, i);
			DBG_ASSERT(IS_Node(exprN));
			SUGAR VisitNode(kctx, builder, stmt, exprN);
			INode *Node = FuelVM_getExpression(builder);
			if(Node->Type == TYPE_void) {
				ktypeattr_t type = params->paramtypeItems[i-2].attrTypeId;
				INode_setType(Node, type);
			}
			Params[i] = Node;
		}

		INode *MtdObj = CreateObject(BLD(builder), KType_Method, (void *) mtd);
		if(kMethod_Is(Virtual, mtd)) {
			Inst = CreateCall(BLD(builder), VirtualCall, kNode_uline(stmt));
			/* set namespace to enable method lookups */
			INode *Node = CreateObject(BLD(builder), KType_NameSpace, (void *) kNode_ns(stmt));
			CallInst_addParam((ICall *)Inst, MtdObj);
			CallInst_addParam((ICall *)Inst, Node);
			DBG_P("TODO VirtualCall");
			asm volatile("int3");
		} else {
			Inst = CreateCall(BLD(builder), DefaultCall, kNode_uline(stmt));
			CallInst_addParam((ICall *)Inst, MtdObj);
		}

		for (i = 1; i < argc + 2; i++) {
			INode *Node = Params[i];
			CallInst_addParam((ICall *)Inst, Node);
		}
	}

	L_finally:;
	INode_setType(Inst, ConvertToTypeId(kctx, expr->attrTypeId));
	if(mtd->mn == MN_box) { /* boxed value of unbox value must be shifted to OC */
		asm volatile("int3");
		((kNodeVar *)expr)->attrTypeId = KType_Object;
	}
	builder->Value = Inst;
}

static void CreateICond(KonohaContext *kctx, KBuilder *builder, kNode *stmt, kNode *expr, enum ConditionalOp Op)
{
	kNode *LHS = kNode_At(expr, 1);
	kNode *RHS = kNode_At(expr, 2);
	INode *Left, *Right;
	SUGAR VisitNode(kctx, builder, stmt, LHS);
	Left = FuelVM_getExpression(builder);
	SUGAR VisitNode(kctx, builder, stmt, RHS);
	Right = FuelVM_getExpression(builder);

	INode *Node = CreateCond(BLD(builder), Op);
	CondInst_addParam((ICond *) Node, Left);
	CondInst_addParam((ICond *) Node, Right);
	INode_setType(Node, TYPE_boolean);
	builder->Value = Node;
}

static void FuelVM_VisitAndNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, kNode *expr)
{
	CreateICond(kctx, builder, stmt, expr, LogicalAnd);
}

static void FuelVM_VisitOrNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, kNode *expr)
{
	CreateICond(kctx, builder, stmt, expr, LogicalOr);
}

static void FuelVM_VisitLetNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, kNode *expr)
{
	/*
	 * [LetNode] := lhs = rhs
	 * expr->NodeList = [NULL, lhs, rhs]
	 **/

	kNode *leftHandNode = kNode_At(expr, 1);
	kNode *rightHandNode = kNode_At(expr, 2);
	INode *Node;
	if(leftHandNode->node == KNode_Local) {
		enum TypeId type = ConvertToTypeId(kctx, leftHandNode->attrTypeId);
		if((Node = IRBuilder_FindLocalVarByHash(BLD(builder), type, leftHandNode->index)) == 0) {
			Node = CreateLocal(BLD(builder), type);
			IField_setHash((IField *) Node, leftHandNode->index);
		}
		SUGAR VisitNode(kctx, builder, stmt, rightHandNode);
		CreateUpdate(BLD(builder), Node, FuelVM_getExpression(builder));
	}
	else if(leftHandNode->node == KNode_STACKTOP) {
		assert(0 && "TODO");
		SUGAR VisitNode(kctx, builder, stmt, rightHandNode);
	}
	else{
		assert(leftHandNode->node == KNode_Field);
		SUGAR VisitNode(kctx, builder, stmt, rightHandNode);
		kshort_t index  = (kshort_t)leftHandNode->index;
		kshort_t xindex = (kshort_t)(leftHandNode->index >> (sizeof(kshort_t)*8));

		INode *Left;
		if((Left = IRBuilder_FindLocalVarByHash(BLD(builder), TYPE_Object, index)) == 0) {
			Left = CreateLocal(BLD(builder), TYPE_Object);
			IField_setHash((IField *) Left, index);
		}
		enum TypeId type = ConvertToTypeId(kctx, leftHandNode->attrTypeId);
		Node = CreateField(BLD(builder), FieldScope, type, Left, xindex);
		SUGAR VisitNode(kctx, builder, stmt, rightHandNode);
		CreateUpdate(BLD(builder), Node, FuelVM_getExpression(builder));
	}
	builder->Value = Node;
}

static void FuelVM_VisitStackTopNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, kNode *expr)
{
	assert(0 && "TODO");
	//KBuilder_AsmNMOV(kctx, builder, a, expr->attrTypeId, expr->index + shift);
}

/* end of Visitor */
/*----------------------------------------------------------------------------*/

static INode *SetUpArguments(KonohaContext *kctx, FuelIRBuilder *builder, kMethod *mtd)
{
	kParam *params = kMethod_GetParam(mtd);
	unsigned i;
	INode *Arg;
	INode *Val;
	INode *Arg0;
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

static struct KVirtualCode *FuelVM_GenerateKVirtualCode(KonohaContext *kctx, kMethod *mtd, kNode *block, int option)
{
	kNameSpace *ns = block->NodeNameSpace;
	KBuilder builderbuf = {}, *builder = &builderbuf;
	FuelIRBuilder Builder = {};
	INIT_GCSTACK();
	IRBuilder_Init(&Builder, kctx);
	builder->builder = &Builder;
	builder->common.api = ns->builderApi;
	Node *EntryNode = CreateNode(BLD(builder));
	IRBuilder_setNode(BLD(builder), EntryNode);
	INode *Self = SetUpArguments(kctx, &Builder, mtd);

	SUGAR VisitNode(kctx, builder, block);

	if(!Node_HasTerminatorInst(BLD(builder)->Current)) {
		if(mtd->mn == MN_new) {
			INode *Ret = CreateReturn(BLD(builder), Self);
			INode_setType(Ret, ConvertToTypeId(kctx, mtd->typeId));
		} else {
			CreateReturn(BLD(builder), 0);
		}
	}
	RESET_GCSTACK();
	IMethod Mtd = {kctx, mtd, EntryNode};
	union ByteCode *code = IRBuilder_Compile(BLD(builder), &Mtd, option);
	IRBuilder_Exit(&Builder);
	KFieldSet(mtd, ((kMethodVar *)mtd)->CompiledNode, block);
	return (struct KVirtualCode *) code;
}

// -------------------------------------------------------------------------

static struct KVirtualCode *FuelVM_Run(KonohaContext *kctx, struct KonohaValueVar *sfp, struct KVirtualCode *pc)
{
	NotSupportedAPI();
	return NULL;
}

#ifdef FUELVM_USE_LLVM
#define ENABLE_FULL_JIT 1
#endif

static KMETHOD FuelVM_RunVirtualMachine(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[K_MTDIDX].calledMethod;
	DBG_ASSERT(IS_Method(mtd));
#ifdef ENABLE_FULL_JIT
	if(mtd->mn != 0) {
		RecompileMethod(kctx, mtd);
		mtd->invokeKMethodFunc(kctx, sfp);
		return;
	}
#endif
	union ByteCode *code = (union ByteCode *) mtd->vcode_start;
	FuelVM_Exec(kctx, sfp, code);
}

static KMethodFunc FuelVM_GenerateKMethodFunc(KonohaContext *kctx, struct KVirtualCode *vcode)
{
	return FuelVM_RunVirtualMachine;
}

static struct KVirtualCode* GetDefaultBootCode(void)
{
	return NULL;
}

typedef void (*GenCodeFunc)(KonohaContext*, kMethod*, kNode *, int options);

void RecompileMethod(KonohaContext *kctx, kMethod *mtd)
{
	DBG_P("[Recompile]: %s.%s%s", KType_text(mtd->typeId), KMethodName_Fmt2(mtd->mn));
	KonohaLibVar *l = (KonohaLibVar *) kctx->klib;
	GenCodeFunc OldFunc = l->kMethod_GenCode;
#ifdef FUELVM_USE_LLVM
	l->kMethod_GenCode = FuelVM_GenerateLLVMIR;
#endif
	KLIB kMethod_GenCode(kctx, mtd, mtd->CompiledNode, O2Compile);
	l->kMethod_GenCode = OldFunc;
}

static void InitStaticBuilderApi(struct KBuilderAPI2 *builderApi)
{
	builderApi->target = "FuelVM";
#define DEFINE_BUILDER_API(NAME) builderApi->visit##NAME = FuelVM_Visit##NAME;
	KNodeList(DEFINE_BUILDER_API);
#undef DEFINE_BUILDER_API
	builderApi->GenerateKVirtualCode = FuelVM_GenerateKVirtualCode;
	builderApi->GenerateKMethodFunc  = FuelVM_GenerateKMethodFunc;
	builderApi->RunVirtualMachine   = FuelVM_Run;
}

static struct KBuilderAPI2 *GetDefaultBuilderAPI(void)
{
	static struct KBuilderAPI2 builderApi = {};
	if(builderApi.target == NULL) {
		InitStaticBuilderApi(&builderApi);
	}
	return &builderApi;
}

// -------------------------------------------------------------------------

kbool_t LoadFuelVMModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"FuelVM", K_VERSION, 0, "FuelVM",
	};
	factory->VirtualMachineInfo            = &ModuleInfo;
	factory->GetDefaultBootCode            = GetDefaultBootCode;
	factory->GetDefaultBuilderAPI          = GetDefaultBuilderAPI;
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
