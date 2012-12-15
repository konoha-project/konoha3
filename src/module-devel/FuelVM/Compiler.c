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
#include <minikonoha/konoha_common.h>
#include "codegen.h"
#include "FuelVM.h"

#define FloatIsDefined(kctx) (KDefinedKonohaCommonModule() && KClass_Float != NULL)

#ifdef __cplusplus
extern "C" {
#endif

static enum TypeId ConvertToTypeId(KonohaContext *kctx, ktypeattr_t type)
{
	if(FloatIsDefined(kctx) && type == KType_float)
		return TYPE_float;
	return type;
}

/*----------------------------------------------------------------------------*/
/* Konoha AST API */

static kBlock *Stmt_getFirstBlock(KonohaContext *kctx, kStmt *stmt)
{
	return SUGAR kStmt_GetBlock(kctx, stmt, NULL, KSymbol_BlockPattern, K_NULLBLOCK);
}

static kBlock *Stmt_getElseBlock(KonohaContext *kctx, kStmt *stmt)
{
	return SUGAR kStmt_GetBlock(kctx, stmt, NULL, KSymbol_else, K_NULLBLOCK);
}

static kExpr *Stmt_getFirstExpr(KonohaContext *kctx, kStmt *stmt)
{
	return SUGAR kStmt_GetExpr(kctx, stmt, KSymbol_ExprPattern, NULL);
}

static kStmt *kStmt_GetStmt(KonohaContext *kctx, kStmt *stmt, ksymbol_t kw)
{
	return (kStmt *) kStmt_GetObject(kctx, stmt, kw, NULL);
}

static kMethod *CallExpr_getMethod(kExpr *expr)
{
	return expr->cons->MethodItems[0];
}

static int CallExpr_getArgCount(kExpr *expr)
{
	return kArray_size(expr->cons) - 2;
}

static kString *Stmt_getErrorMessage(KonohaContext *kctx, kStmt *stmt)
{
	kString* msg = (kString *)kStmt_GetObjectNULL(kctx, stmt, KSymbol_ERR);
	DBG_ASSERT(IS_String(msg));
	return msg;
}

static void kStmt_setLabelBlock(KonohaContext *kctx, kStmt *stmt, ksymbol_t label, Block *block)
{
	KLIB kObjectProto_SetUnboxValue(kctx, stmt, label, KType_int, (uintptr_t) block);
}

static Block *kStmt_GetTargetBlock(KonohaContext *kctx, kStmt *stmt, ksymbol_t keyword)
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

#define MN_opNOT  MN_("!")
#define MN_opADD  MN_("+")
#define MN_opSUB  MN_("-")
#define MN_opMUL  MN_("*")
#define MN_opDIV  MN_("/")
#define MN_opMOD  MN_("%")
#define MN_opEQ   MN_("==")
#define MN_opNEQ  MN_("!=")
#define MN_opLT   MN_("<")
#define MN_opLTE  MN_("<=")
#define MN_opGT   MN_(">")
#define MN_opGTE  MN_(">=")
#define MN_opLAND MN_("&")
#define MN_opLOR  MN_("|")
#define MN_opLXOR MN_("^")
#define MN_opLSFT MN_("<<")
#define MN_opRSFT MN_(">>")

static enum BinaryOp MethodName_toBinaryOperator(KonohaContext *kctx, kmethodn_t mn)
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

static enum UnaryOp MethodName_toUnaryOperator(KonohaContext *kctx, kmethodn_t mn)
{
	if(mn == MN_opSUB) return Neg;
	if(mn == MN_opNOT) return Not;
	return UnaryOp_NotFound;
}

static INode *FetchINode(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr, unsigned idx, ktypeattr_t reqTy)
{
	kExpr *exprN = kExpr_at(expr, idx);
	DBG_ASSERT(IS_Expr(exprN));
	SUGAR VisitExpr(kctx, builder, stmt, exprN);
	INode *Node = FuelVM_getExpression(builder);
	if(Node->Type == KType_void) {
		INode_setType(Node, ConvertToTypeId(kctx, reqTy));
	}
	return Node;
}

static INode *CreateSpecialInstruction(KonohaContext *kctx, KBuilder *builder, kMethod *mtd, kStmt *stmt, kExpr *expr)
{
	ktypeattr_t thisTy = mtd->typeId;
	kmethodn_t  mn     = mtd->mn;
	ktypeattr_t retTy  = kMethod_GetReturnType(mtd)->typeId;
	kParam     *params = kMethod_GetParam(mtd);
	if(thisTy == KType_int) {
		if(params->psize == 0) { /* UnaryOperator */
			if(retTy == KType_int) {
				/* int int.opSUB() */
				enum UnaryOp Op = MethodName_toUnaryOperator(kctx, mn);
				INode *Param = FetchINode(kctx, builder, stmt, expr, 1, KType_int);
				return CreateUnaryInst(BLD(builder), Op, Param);
			}
		}
		else if(params->psize == 1) { /* BinaryOperator */
			ktypeattr_t ptype = params->paramtypeItems[0].attrTypeId;
			if(retTy == KType_boolean && ptype == KType_int) {
				/* boolean int.(opEQ|opNE|opGT|opGE|opLT|opLE) (int x) */
				enum BinaryOp Op = MethodName_toBinaryOperator(kctx, mn);
				INode *LHS = FetchINode(kctx, builder, stmt, expr, 1, KType_int);
				INode *RHS = FetchINode(kctx, builder, stmt, expr, 2, KType_int);
				return CreateBinaryInst(BLD(builder), Op, LHS, RHS);
			}
			else if(retTy == KType_int && ptype == KType_int) {
				/* int int.(opADD|opSUB|opMUL|opDIV|opMOD|opLSHIFT|opRSHIFT|opAND|opOR|opXOR) (int x) */
				enum BinaryOp Op = MethodName_toBinaryOperator(kctx, mn);
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
				enum UnaryOp Op = MethodName_toUnaryOperator(kctx, mn);
				INode *Param = FetchINode(kctx, builder, stmt, expr, 1, KType_float);
				return CreateUnaryInst(BLD(builder), Op, Param);
			}
		}
		else if(params->psize == 1) { /* BinaryOperator */
			ktypeattr_t ptype = params->paramtypeItems[0].attrTypeId;
			if(retTy == KType_boolean && ptype == KType_float) {
				/* boolean float.(opEQ|opNE|opGT|opGE|opLT|opLE) (float x) */
				enum BinaryOp Op = MethodName_toBinaryOperator(kctx, mn);
				INode *LHS = FetchINode(kctx, builder, stmt, expr, 1, KType_float);
				INode *RHS = FetchINode(kctx, builder, stmt, expr, 2, KType_float);
				return CreateBinaryInst(BLD(builder), Op, LHS, RHS);
			}
			else if(retTy == KType_float && ptype == KType_float) {
				/* float float.(opADD|opSUB|opMUL|opDIV) (float x) */
				enum BinaryOp Op = MethodName_toBinaryOperator(kctx, mn);
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
static bool FuelVM_VisitErrStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	kString *Err  = Stmt_getErrorMessage(kctx, stmt);
	INode   *Node = CreateObject(BLD(builder), TYPE_String, (void *)Err);
	CreateThrow(BLD(builder), Node, stmt->uline);
	return true;
}

static bool FuelVM_VisitExprStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	SUGAR VisitExpr(kctx, builder, stmt, Stmt_getFirstExpr(kctx, stmt));
	return true;
}

static bool FuelVM_VisitBlockStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	Block *NewBlock = CreateBlock(BLD(builder));
	IRBuilder_JumpTo(BLD(builder), NewBlock);
	IRBuilder_setBlock(BLD(builder), NewBlock);
	SUGAR VisitBlock(kctx, builder, Stmt_getFirstBlock(kctx, stmt));
	return true;
}

static bool FuelVM_VisitReturnStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	kExpr *expr = SUGAR kStmt_GetExpr(kctx, stmt, KSymbol_ExprPattern, NULL);
	if(expr != NULL && IS_Expr(expr) && expr->attrTypeId != KType_void) {
		SUGAR VisitExpr(kctx, builder, stmt, expr);
		INode *Ret  = FuelVM_getExpression(builder);
		INode *Inst = CreateReturn(BLD(builder), Ret);
		INode_setType(Inst, ConvertToTypeId(kctx, expr->attrTypeId));
	} else {
		CreateReturn(BLD(builder), 0);
	}
	return true;
}

static bool FuelVM_VisitIfStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	kExpr *expr    = Stmt_getFirstExpr(kctx, stmt);
	Block *ThenBB  = CreateBlock(BLD(builder));
	Block *ElseBB  = CreateBlock(BLD(builder));
	Block *MergeBB = CreateBlock(BLD(builder));
	/* if */
	SUGAR VisitExpr(kctx, builder, stmt, expr);
	CreateBranch(BLD(builder), FuelVM_getExpression(builder), ThenBB, ElseBB);
	/* then */
	IRBuilder_setBlock(BLD(builder), ThenBB);
	SUGAR VisitBlock(kctx, builder, Stmt_getFirstBlock(kctx, stmt));
	if(!Block_HasTerminatorInst(BLD(builder)->Current)) {
		IRBuilder_JumpTo(BLD(builder), MergeBB);
	}
	/* else */
	IRBuilder_setBlock(BLD(builder), ElseBB);
	SUGAR VisitBlock(kctx, builder, Stmt_getElseBlock(kctx, stmt));
	if(!Block_HasTerminatorInst(BLD(builder)->Current)) {
		IRBuilder_JumpTo(BLD(builder), MergeBB);
	}
	/* endif */
	IRBuilder_setBlock(BLD(builder), MergeBB);
	return true;
}

static bool FuelVM_VisitLoopStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	Block *HeaderBB = CreateBlock(BLD(builder));
	Block *BodyBB   = CreateBlock(BLD(builder));
	Block *MergeBB  = CreateBlock(BLD(builder));
	/* Head  : if(COND) { goto Body } else {goto Merge }
	 * Body  :   Body1
	 *           Body2
	 *           goto Header
	 * Merge : ...
	 */

	kStmt_setLabelBlock(kctx, stmt, SYM_("continue"), HeaderBB);
	kStmt_setLabelBlock(kctx, stmt, SYM_("break"),    MergeBB);

	if(kStmt_Is(RedoLoop, stmt)) {
		IRBuilder_JumpTo(BLD(builder), BodyBB);
	} else {
		IRBuilder_JumpTo(BLD(builder), HeaderBB);
	}
	IRBuilder_setBlock(BLD(builder), HeaderBB);

	SUGAR VisitExpr(kctx, builder, stmt, Stmt_getFirstExpr(kctx, stmt));
	CreateBranch(BLD(builder), FuelVM_getExpression(builder), BodyBB, MergeBB);
	IRBuilder_setBlock(BLD(builder), BodyBB);
	SUGAR VisitBlock(kctx, builder, Stmt_getFirstBlock(kctx, stmt));

	kBlock *itrBlock = SUGAR kStmt_GetBlock(kctx, stmt, NULL, SYM_("Iterator"), NULL);
	if(itrBlock != NULL) {
		assert(!Block_HasTerminatorInst(BLD(builder)->Current));
		Block *BB = CreateBlock(BLD(builder));
		IRBuilder_JumpTo(BLD(builder), BB);
		IRBuilder_setBlock(BLD(builder), BB);
		SUGAR VisitBlock(kctx, builder, itrBlock);
	}

	IRBuilder_JumpTo(BLD(builder), HeaderBB);
	IRBuilder_setBlock(BLD(builder), MergeBB);
	return true;
}

static bool FuelVM_VisitJumpStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	KSyntax *syn = stmt->syn;
	kStmt *jump = kStmt_GetStmt(kctx, stmt, syn->keyword);
	DBG_ASSERT(jump != NULL && IS_Stmt(jump));
	Block *target = kStmt_GetTargetBlock(kctx, jump, syn->keyword);
	IRBuilder_JumpTo(BLD(builder), target);
	return true;
}

static bool FuelVM_VisitTryStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	//FIXME
	TODO();
	return true;
}


static bool FuelVM_VisitUndefinedStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	DBG_P("undefined asm syntax kw='%s'", KSymbol_text(stmt->syn->keyword));
	return true;
}

static void FuelVM_VisitConstExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	kObject *v = expr->objectConstValue;
	DBG_ASSERT(!KType_Is(UnboxType, expr->attrTypeId));
	DBG_ASSERT(Expr_hasObjectConstValue(expr));
	builder->Value = CreateObject(BLD(builder), expr->attrTypeId, (void *)v);
}

static void FuelVM_VisitNConstExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	DBG_ASSERT(KType_Is(UnboxType, expr->attrTypeId));
	DBG_ASSERT(!Expr_hasObjectConstValue(expr));

	SValue Val = {};
	Val.bits = expr->unboxConstValue;
	builder->Value = CreateConstant(BLD(builder), ConvertToTypeId(kctx, expr->attrTypeId), Val);
}

static void FuelVM_VisitNewExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	INode *Expr = CreateNew(BLD(builder), ConvertToTypeId(kctx, expr->attrTypeId));
	builder->Value = Expr;
}

static void FuelVM_VisitNullExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
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

static void FuelVM_VisitLocalExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	INode *Inst;
	if ((Inst = IRBuilder_FindLocalVarByHash(BLD(builder), ConvertToTypeId(kctx, expr->attrTypeId), expr->index)) == 0) {
		Inst = CreateLocal(BLD(builder), ConvertToTypeId(kctx, expr->attrTypeId));
		IField_setHash((IField *) Inst, expr->index);
	}
	builder->Value = Inst;
}

static void FuelVM_VisitBlockExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	TODO();
	DBG_ASSERT(IS_Block(expr->block));
	//SUGAR VisitBlock(kctx, builder, expr->block);
	//KBuilder_AsmNMOV(kctx, builder, a, expr->attrTypeId, espidx);
}

static void FuelVM_VisitFieldExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	TODO();
	//kshort_t index = (kshort_t)expr->index;
	//kshort_t xindex = (kshort_t)(expr->index >> (sizeof(kshort_t)*8));
	//ASM(NMOVx, TC_(a, expr->attrTypeId), OC_(index), xindex, KClass_(expr->attrTypeId));
}

static void FuelVM_VisitCallExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	kMethod *mtd = CallExpr_getMethod(expr);
	DBG_ASSERT(IS_Method(mtd));

	/*
	 * [CallExpr] := this.method(arg1, arg2, ...)
	 * expr->cons = [method, this, arg1, arg2, ...]
	 **/
	INode *Inst;
	if((Inst = CreateSpecialInstruction(kctx, builder, mtd, stmt, expr)) != 0) {
		goto L_finally;
	}
	{
		int i, s = kMethod_Is(Static, mtd) ? 2 : 1;
		int argc = CallExpr_getArgCount(expr);
		INode *Params[argc+2];
		kParam *params = kMethod_GetParam(mtd);

		if(kMethod_Is(Static, mtd)) {
			kObject *obj = KLIB Knull(kctx, KClass_(mtd->typeId));
			INode *Self = CreateObject(BLD(builder), mtd->typeId, (void *) obj);
			Params[1] = Self;
		}
		for (i = s; i < argc + 2; i++) {
			kExpr *exprN = kExpr_at(expr, i);
			DBG_ASSERT(IS_Expr(exprN));
			SUGAR VisitExpr(kctx, builder, stmt, exprN);
			INode *Node = FuelVM_getExpression(builder);
			if(Node->Type == TYPE_void) {
				ktypeattr_t type = params->paramtypeItems[i-2].attrTypeId;
				INode_setType(Node, type);
			}
			Params[i] = Node;
		}

		INode *MtdObj = CreateObject(BLD(builder), KType_Method, (void *) mtd);
		if(kMethod_Is(Virtual, mtd)) {
			Inst = CreateCall(BLD(builder), VirtualCall, stmt->uline);
			/* set namespace to enable method lookups */
			INode *Node = CreateObject(BLD(builder), KType_NameSpace, (void *) Stmt_ns(stmt));
			CallInst_addParam((ICall *)Inst, MtdObj);
			CallInst_addParam((ICall *)Inst, Node);
			DBG_P("TODO VirtualCall");
			asm volatile("int3");
		} else {
			Inst = CreateCall(BLD(builder), DefaultCall, stmt->uline);
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
		((kExprVar *)expr)->attrTypeId = KType_Object;
	}
	builder->Value = Inst;
}

static void FuelVM_VisitAndExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	kExpr *LHS = kExpr_at(expr, 1);
	kExpr *RHS = kExpr_at(expr, 2);
	INode *Node = CreateCond(BLD(builder), LogicalAnd);
	SUGAR VisitExpr(kctx, builder, stmt, LHS);
	CondInst_addParam((ICond *) Node, FuelVM_getExpression(builder));
	SUGAR VisitExpr(kctx, builder, stmt, RHS);
	CondInst_addParam((ICond *) Node, FuelVM_getExpression(builder));
	builder->Value = Node;
}

static void FuelVM_VisitOrExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	kExpr *LHS = kExpr_at(expr, 1);
	kExpr *RHS = kExpr_at(expr, 2);
	INode *Node = CreateCond(BLD(builder), LogicalOr);
	SUGAR VisitExpr(kctx, builder, stmt, LHS);
	CondInst_addParam((ICond *) Node, FuelVM_getExpression(builder));
	SUGAR VisitExpr(kctx, builder, stmt, RHS);
	CondInst_addParam((ICond *) Node, FuelVM_getExpression(builder));
	builder->Value = Node;
}

static void FuelVM_VisitLetExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	/*
	 * [LetExpr] := lhs = rhs
	 * expr->cons = [NULL, lhs, rhs]
	 **/

	kExpr *leftHandExpr = kExpr_at(expr, 1);
	kExpr *rightHandExpr = kExpr_at(expr, 2);
	if(leftHandExpr->build == TEXPR_LOCAL) {
		INode *Field;
		enum TypeId type = ConvertToTypeId(kctx, leftHandExpr->attrTypeId);
		if ((Field = IRBuilder_FindLocalVarByHash(BLD(builder), type, leftHandExpr->index)) == 0) {
			Field = CreateLocal(BLD(builder), type);
			IField_setHash((IField *) Field, leftHandExpr->index);
		}
		SUGAR VisitExpr(kctx, builder, stmt, rightHandExpr);
		CreateUpdate(BLD(builder), Field, FuelVM_getExpression(builder));
	}
	else if(leftHandExpr->build == TEXPR_STACKTOP) {
		assert(0 && "TODO");
		SUGAR VisitExpr(kctx, builder, stmt, rightHandExpr);
	}
	else{
		assert(leftHandExpr->build == TEXPR_FIELD);
		assert(0 && "TODO");
		//SUGAR VisitExpr(kctx, builder, stmt, rightHandExpr);
		//kshort_t index  = (kshort_t)leftHandExpr->index;
		//kshort_t xindex = (kshort_t)(leftHandExpr->index >> (sizeof(kshort_t)*8));
		//KClass *lhsClass = KClass_(leftHandExpr->attrTypeId);
		//ASM(XNMOV, OC_(index), xindex, TC_(espidx, rightHandExpr->attrTypeId), lhsClass);
		//if(expr->attrTypeId != KType_void) {
		//	ASM(NMOVx, TC_(a, rightHandExpr->attrTypeId), OC_(index), xindex, lhsClass);
		//}
	}
}

static void FuelVM_VisitStackTopExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	assert(0 && "TODO");
	//KBuilder_AsmNMOV(kctx, builder, a, expr->attrTypeId, expr->index + shift);
}

/* end of Visitor */
/*----------------------------------------------------------------------------*/

static void SetUpArguments(KonohaContext *kctx, FuelIRBuilder *builder, kMethod *mtd)
{
	kParam *params = kMethod_GetParam(mtd);
	unsigned i;
	INode *Arg;
	INode *Val;
	if(mtd->typeId != TYPE_void) {
		Arg = CreateArgument(builder, 0);
		Val = CreateLocal(builder, mtd->typeId);
		CreateUpdate(builder, Val, Arg);
		INode_setType(Arg, mtd->typeId);
		IField_setHash((IField *) Val, 0);
	}
	for(i = 0; i < params->psize; ++i) {
		enum TypeId type = ConvertToTypeId(kctx, params->paramtypeItems[i].attrTypeId);
		Arg = CreateArgument(builder, i+1);
		Val = CreateLocal(builder, type);
		CreateUpdate(builder, Val, Arg);
		INode_setType(Arg, type);
		IField_setHash((IField *) Val, i+1);
	}
}

static struct KVirtualCode *FuelVM_GenerateKVirtualCode(KonohaContext *kctx, kMethod *mtd, kBlock *block, int option)
{
	kNameSpace *ns = block->BlockNameSpace;
	KBuilder builderbuf = {}, *builder = &builderbuf;
	FuelIRBuilder Builder = {};
	INIT_GCSTACK();
	IRBuilder_Init(&Builder);
	builder->builder = &Builder;
	builder->common.api = ns->builderApi;
	Block *EntryBlock = CreateBlock(BLD(builder));
	IRBuilder_setBlock(BLD(builder), EntryBlock);
	SetUpArguments(kctx, &Builder, mtd);

	SUGAR VisitBlock(kctx, builder, block);

	if(!Block_HasTerminatorInst(BLD(builder)->Current)) {
		CreateReturn(BLD(builder), 0);
	}
	RESET_GCSTACK();
	IMethod Mtd = {kctx, mtd, EntryBlock};
	union ByteCode *code = IRBuilder_Compile(BLD(builder), &Mtd, option);
	IRBuilder_Exit(&Builder);
	KFieldSet(mtd, ((kMethodVar *)mtd)->CompiledBlock, block);
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

typedef void (*GenCodeFunc)(KonohaContext*, kMethod*, kBlock *, int options);

void RecompileMethod(KonohaContext *kctx, kMethod *mtd)
{
	DBG_P("[Recompile]: %s.%s%s", KType_text(mtd->typeId), MethodName_Fmt2(mtd->mn));
	KonohaLibVar *l = (KonohaLibVar *) kctx->klib;
	GenCodeFunc OldFunc = l->kMethod_GenCode;
#ifdef FUELVM_USE_LLVM
	l->kMethod_GenCode = FuelVM_GenerateLLVMIR;
#endif
	KLIB kMethod_GenCode(kctx, mtd, mtd->CompiledBlock, O2Compile);
	l->kMethod_GenCode = OldFunc;
}

static void InitStaticBuilderApi(struct KBuilderAPI2 *builderApi)
{
	builderApi->target = "FuelVM";
#define DEFINE_BUILDER_API(NAME) builderApi->visit##NAME = FuelVM_Visit##NAME;
	VISITOR_LIST(DEFINE_BUILDER_API);
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
