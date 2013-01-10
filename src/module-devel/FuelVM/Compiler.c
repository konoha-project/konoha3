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

#include "codegen.h"
#include "FuelVM.h"

#ifdef __cplusplus
extern "C" {
#endif

DEF_ARRAY_OP_NOPOINTER(INodePtr);

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
	return expr->NodeList->MethodItems[0];
}

static int CallExpr_getArgCount(kExpr *expr)
{
	return kArray_size(expr->NodeList) - 2;
}

static kString *Stmt_getErrorMessage(KonohaContext *kctx, kStmt *stmt)
{
	kString* msg = (kString *)kStmt_GetObjectNULL(kctx, stmt, KSymbol_ERR);
	DBG_ASSERT(IS_String(msg));
	return msg;
}

static void kStmt_SetLabelBlock(KonohaContext *kctx, kStmt *stmt, ksymbol_t label, Block *block)
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

static INode *FetchINode(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr, unsigned idx, ktypeattr_t reqTy)
{
	kExpr *exprN = kExpr_At(expr, idx);
	DBG_ASSERT(IS_Expr(exprN));
	SUGAR VisitExpr(kctx, builder, stmt, exprN);
	INode *Node = FuelVM_getExpression(builder);
	assert(Node->Type != TYPE_void);
	return Node;
}

static INode *CreateSpecialInstruction(KonohaContext *kctx, KBuilder *builder, kMethod *mtd, kStmt *stmt, kExpr *expr)
{
	ktypeattr_t thisTy = mtd->typeId;
	kmethodn_t  mn     = mtd->mn;
	ktypeattr_t retTy  = kMethod_GetReturnType(mtd)->typeId;
	kParam     *params = kMethod_GetParam(mtd);
	if(thisTy == KType_boolean) {
		if(params->psize == 0) { /* UnaryOperator */
			if(retTy == KType_boolean) {
				/* booleaen booleaen.opNEG() */
				enum UnaryOp Op = KMethodName_toUnaryOperator(kctx, mn);
				INode *Param = FetchINode(kctx, builder, stmt, expr, 1, KType_boolean);
				return CreateUnaryInst(BLD(builder), Op, Param, stmt->uline);
			}
		}
		else if(params->psize == 1) { /* BinaryOperator */
			ktypeattr_t ptype = params->paramtypeItems[0].attrTypeId;
			if(retTy == KType_boolean && ptype == KType_boolean) {
				/* boolean boolean.(opEQ|opNE) (boolean x) */
				enum BinaryOp Op = KMethodName_toBinaryOperator(kctx, mn);
				assert(Op == Eq || Op == Nq);
				INode *LHS = FetchINode(kctx, builder, stmt, expr, 1, KType_boolean);
				INode *RHS = FetchINode(kctx, builder, stmt, expr, 2, KType_boolean);
				return CreateBinaryInst(BLD(builder), Op, LHS, RHS, stmt->uline);
			}
		}
	}
	else if(thisTy == KType_int) {
		if(params->psize == 0) { /* UnaryOperator */
			if(retTy == KType_int) {
				/* int int.opSUB() */
				enum UnaryOp Op = KMethodName_toUnaryOperator(kctx, mn);
				INode *Param = FetchINode(kctx, builder, stmt, expr, 1, KType_int);
				return CreateUnaryInst(BLD(builder), Op, Param, stmt->uline);
			}
		}
		else if(params->psize == 1) { /* BinaryOperator */
			ktypeattr_t ptype = params->paramtypeItems[0].attrTypeId;
			if(retTy == KType_boolean && ptype == KType_int) {
				/* boolean int.(opEQ|opNE|opGT|opGE|opLT|opLE) (int x) */
				enum BinaryOp Op = KMethodName_toBinaryOperator(kctx, mn);
				INode *LHS = FetchINode(kctx, builder, stmt, expr, 1, KType_int);
				INode *RHS = FetchINode(kctx, builder, stmt, expr, 2, KType_int);
				return CreateBinaryInst(BLD(builder), Op, LHS, RHS, stmt->uline);
			}
			else if(retTy == KType_int && ptype == KType_int) {
				/* int int.(opADD|opSUB|opMUL|opDIV|opMOD|opLSHIFT|opRSHIFT|opAND|opOR|opXOR) (int x) */
				enum BinaryOp Op = KMethodName_toBinaryOperator(kctx, mn);
				INode *LHS = FetchINode(kctx, builder, stmt, expr, 1, KType_int);
				INode *RHS = FetchINode(kctx, builder, stmt, expr, 2, KType_int);
				return CreateBinaryInst(BLD(builder), Op, LHS, RHS, stmt->uline);
			}
		}
	}
	else if(FloatIsDefined() && thisTy == KType_float) {
		if(params->psize == 0) { /* UnaryOperator */
			if(retTy == KType_float) {
				/* int int.opSUB() */
				enum UnaryOp Op = KMethodName_toUnaryOperator(kctx, mn);
				INode *Param = FetchINode(kctx, builder, stmt, expr, 1, KType_float);
				return CreateUnaryInst(BLD(builder), Op, Param, stmt->uline);
			}
		}
		else if(params->psize == 1) { /* BinaryOperator */
			ktypeattr_t ptype = params->paramtypeItems[0].attrTypeId;
			if(retTy == KType_boolean && ptype == KType_float) {
				/* boolean float.(opEQ|opNE|opGT|opGE|opLT|opLE) (float x) */
				enum BinaryOp Op = KMethodName_toBinaryOperator(kctx, mn);
				INode *LHS = FetchINode(kctx, builder, stmt, expr, 1, KType_float);
				INode *RHS = FetchINode(kctx, builder, stmt, expr, 2, KType_float);
				return CreateBinaryInst(BLD(builder), Op, LHS, RHS, stmt->uline);
			}
			else if(retTy == KType_float && ptype == KType_float) {
				/* float float.(opADD|opSUB|opMUL|opDIV) (float x) */
				enum BinaryOp Op = KMethodName_toBinaryOperator(kctx, mn);
				INode *LHS = FetchINode(kctx, builder, stmt, expr, 1, KType_float);
				INode *RHS = FetchINode(kctx, builder, stmt, expr, 2, KType_float);
				return CreateBinaryInst(BLD(builder), Op, LHS, RHS, stmt->uline);
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
	CreateThrow(BLD(builder), Node, KException_("RuntimeScript"), SoftwareFault, stmt->uline);
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
	{ /* then */
		IRBuilder_setBlock(BLD(builder), ThenBB);
		SUGAR VisitBlock(kctx, builder, Stmt_getFirstBlock(kctx, stmt));
		if(!Block_HasTerminatorInst(BLD(builder)->Current)) {
			IRBuilder_JumpTo(BLD(builder), MergeBB);
		}
	}
	{ /* else */
		IRBuilder_setBlock(BLD(builder), ElseBB);
		SUGAR VisitBlock(kctx, builder, Stmt_getElseBlock(kctx, stmt));
		if(!Block_HasTerminatorInst(BLD(builder)->Current)) {
			IRBuilder_JumpTo(BLD(builder), MergeBB);
		}
	}
	/* endif */
	IRBuilder_setBlock(BLD(builder), MergeBB);
	return true;
}

static bool FuelVM_VisitLoopStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	Block *HeadBB  = CreateBlock(BLD(builder));
	Block *BodyBB  = CreateBlock(BLD(builder));
	Block *ItrBB   = CreateBlock(BLD(builder));
	Block *MergeBB = CreateBlock(BLD(builder));

	kStmt_SetLabelBlock(kctx, stmt, KSymbol_("continue"), ItrBB);
	kStmt_SetLabelBlock(kctx, stmt, KSymbol_("break"),    MergeBB);

	kBlock *itrBlock = SUGAR kStmt_GetBlock(kctx, stmt, NULL, KSymbol_("Iterator"), NULL);
	if(itrBlock != NULL) {
		IRBuilder_JumpTo(BLD(builder), HeadBB);
	}
	else if(!kStmt_Is(RedoLoop, stmt)) {
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
		SUGAR VisitExpr(kctx, builder, stmt, Stmt_getFirstExpr(kctx, stmt));
		CreateBranch(BLD(builder), FuelVM_getExpression(builder), BodyBB, MergeBB);
	}

	{ /* Body */
		IRBuilder_setBlock(BLD(builder), BodyBB);
		SUGAR VisitBlock(kctx, builder, Stmt_getFirstBlock(kctx, stmt));
		IRBuilder_JumpTo(BLD(builder), ItrBB);

		/* Itr */
		IRBuilder_setBlock(BLD(builder), ItrBB);
		if(itrBlock != NULL) {
			SUGAR VisitBlock(kctx, builder, itrBlock);
		}
		IRBuilder_JumpTo(BLD(builder), HeadBB);
	}

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
	kObject *v = expr->ObjectConstValue;
	DBG_ASSERT(!KType_Is(UnboxType, expr->attrTypeId));
	DBG_ASSERT(kExpr_HasObjectConstValue(expr));
	builder->Value = CreateObject(BLD(builder), expr->attrTypeId, (void *)v);
}

static void FuelVM_VisitNConstExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	DBG_ASSERT(KType_Is(UnboxType, expr->attrTypeId));
	DBG_ASSERT(!kExpr_HasObjectConstValue(expr));

	SValue Val = {};
	Val.bits = expr->unboxConstValue;
	builder->Value = CreateConstant(BLD(builder), ConvertToTypeId(kctx, expr->attrTypeId), Val);
}

static void FuelVM_VisitNewExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	enum TypeId Type = ConvertToTypeId(kctx, expr->attrTypeId);
	INode *Expr = CreateNew(BLD(builder), expr->unboxConstValue, Type);
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
	if((Inst = IRBuilder_FindLocalVarByHash(BLD(builder), ConvertToTypeId(kctx, expr->attrTypeId), expr->index)) == 0) {
		Inst = CreateLocal(BLD(builder), ConvertToTypeId(kctx, expr->attrTypeId));
		IField_setHash((IField *) Inst, expr->index);
	}
	builder->Value = Inst;
}

static void FuelVM_VisitBlockExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	DBG_ASSERT(IS_Block(expr->block));
	unsigned size = ARRAY_size(BLD(builder)->Stack);
	SUGAR VisitBlock(kctx, builder, expr->block);
	ARRAY_size(BLD(builder)->Stack) = size;
	INode *Node = FuelVM_getExpression(builder);
	builder->Value = Node;
}

static void FuelVM_VisitFieldExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
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

static void FuelVM_VisitCallExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	kMethod *mtd = CallExpr_getMethod(expr);
	DBG_ASSERT(IS_Method(mtd));
	enum TypeId Type = ConvertToTypeId(kctx, expr->attrTypeId);
	if(mtd->mn == MN_box) {
		Type = ToBoxType(Type);
	}

	/*
	 * [CallExpr] := this.method(arg1, arg2, ...)
	 * expr->NodeList = [method, this, arg1, arg2, ...]
	 **/
	INode *Inst;
	if((Inst = CreateSpecialInstruction(kctx, builder, mtd, stmt, expr)) != 0) {
		INode_setType(Inst, Type);
		builder->Value = Inst;
		return;
	}
	int i, s = kMethod_Is(Static, mtd) ? 2 : 1;
	int argc = CallExpr_getArgCount(expr);
	INode *Params[argc+2];

	if(kMethod_Is(Static, mtd)) {
		kObject *obj = KLIB Knull(kctx, KClass_(mtd->typeId));
		INode *Self = CreateObject(BLD(builder), mtd->typeId, (void *) obj);
		Params[1] = Self;
	}
	for (i = s; i < argc + 2; i++) {
		kExpr *exprN = kExpr_At(expr, i);
		DBG_ASSERT(IS_Expr(exprN));
		SUGAR VisitExpr(kctx, builder, stmt, exprN);
		INode *Node = FuelVM_getExpression(builder);
		assert(Node->Type != TYPE_void);
		Params[i] = Node;
	}

	INode *MtdObj = CreateObject(BLD(builder), KType_Method, (void *) mtd);
	enum CallOp Op = (kMethod_Is(Virtual, mtd)) ? VirtualCall : DefaultCall;
	Params[0] = MtdObj;
	Inst = CreateICall(BLD(builder), Type, Op, stmt->uline, Params, argc + 2);
	builder->Value = Inst;
}

static void CreateCond(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr, enum ConditionalOp Op)
{
	kExpr *LHS = kExpr_At(expr, 1);
	kExpr *RHS = kExpr_At(expr, 2);

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
		SUGAR VisitExpr(kctx, builder, stmt, LHS);
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
		SUGAR VisitExpr(kctx, builder, stmt, RHS);
		INode *Right = FuelVM_getExpression(builder);
		CreateUpdate(BLD(builder), Node, Right);
		IRBuilder_JumpTo(BLD(builder), MergeBB);
	}

	IRBuilder_setBlock(BLD(builder), MergeBB);
	builder->Value = Node;
}

static void FuelVM_VisitAndExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	CreateCond(kctx, builder, stmt, expr, LogicalAnd);
}

static void FuelVM_VisitOrExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	CreateCond(kctx, builder, stmt, expr, LogicalOr);
}

static void FuelVM_VisitLetExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	/*
	 * [LetExpr] := lhs = rhs
	 * expr->NodeList = [NULL, lhs, rhs]
	 **/

	kExpr *left = kExpr_At(expr, 1);
	kExpr *right = kExpr_At(expr, 2);
	INode *Node;
	FuelIRBuilder *flbuilder = BLD(builder);
	if(left->node == TEXPR_LOCAL) {
		enum TypeId type = ConvertToTypeId(kctx, left->attrTypeId);
		if((Node = IRBuilder_FindLocalVarByHash(flbuilder, type, left->index)) == 0) {
			Node = CreateLocal(flbuilder, type);
			IField_setHash((IField *) Node, left->index);
		}
		SUGAR VisitExpr(kctx, builder, stmt, right);
		INode *RHS = FuelVM_getExpression(builder);
		//if(RHS->Type != Node->Type) {
		//	INode_setType(Node, RHS->Type);
		//}
		CreateUpdate(flbuilder, Node, RHS);
	}
	else if(left->node == TEXPR_STACKTOP) {
		enum TypeId type = ConvertToTypeId(kctx, left->attrTypeId);
		uintptr_t Hash = (uintptr_t) left;
		if((Node = IRBuilder_FindLocalVarByHash(flbuilder, type, Hash)) == 0) {
			Node = CreateLocal(flbuilder, type);
			IField_setHash((IField *) Node, Hash);
			ARRAY_add(INodePtr, &BLD(builder)->Stack, Node);
		}
		SUGAR VisitExpr(kctx, builder, stmt, right);
		INode *RHS = FuelVM_getExpression(builder);
		if(RHS->Type != Node->Type)
			INode_setType(Node, RHS->Type);
		CreateUpdate(BLD(builder), Node, RHS);
	}
	else{
		assert(left->node == TEXPR_FIELD);
		SUGAR VisitExpr(kctx, builder, stmt, right);
		kshort_t index  = (kshort_t)left->index;
		kshort_t xindex = (kshort_t)(left->index >> (sizeof(kshort_t)*8));

		INode *Left;
		if((Left = IRBuilder_FindLocalVarByHash(BLD(builder), TYPE_Object, index)) == 0) {
			Left = CreateLocal(BLD(builder), TYPE_Object);
			IField_setHash((IField *) Left, index);
		}
		enum TypeId type = ConvertToTypeId(kctx, left->attrTypeId);
		Node = CreateField(BLD(builder), FieldScope, type, Left, xindex);
		SUGAR VisitExpr(kctx, builder, stmt, right);
		CreateUpdate(BLD(builder), Node, FuelVM_getExpression(builder));
	}
	builder->Value = Node;
}

static bool String_equal(KonohaContext *kctx, kString *str0, kString *str1)
{
	unsigned len0 = kString_size(str0);
	unsigned len1 = kString_size(str1);
	if(len0 != len1)
		return false;
	return strncmp(kString_text(str0), kString_text(str1), len0) == 0;
}

static bool Token_equal(KonohaContext *kctx, kToken *tk1, kToken *tk2)
{
	if(IS_String(tk1->text) && IS_String(tk2->text)) {
		return String_equal(kctx, tk1->text, tk2->text);
	}
	return false;
}

static void FuelVM_VisitStackTopExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	enum TypeId type = ConvertToTypeId(kctx, expr->attrTypeId);
	INodePtr *x, *e;
	FOR_EACH_ARRAY(BLD(builder)->Stack, x, e) {
		IField *Node = (IField *) *x;
		if((*x)->Type != type)
			continue;
		kExpr *e = (kExpr *) Node->Hash;
		if(IS_Token(e->TermToken) && IS_Token(expr->TermToken)) {
			if(Token_equal(kctx, e->TermToken, expr->TermToken)) {
				builder->Value = *x;
				return;
			}
		}
	}
	assert(0);
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

static KMethodFunc FuelVM_GenerateKMethodFunc(KonohaContext *kctx, struct KVirtualCode *vcode)
{
	return 0;
}

static struct KVirtualCode *GetDefaultBootCode(void)
{
	return NULL;
}

static KMethodFunc AbstractMethodPtr = 0;

static struct KVirtualCode *FuelVM_GenerateKVirtualCode(KonohaContext *kctx, kMethod *mtd, kBlock *block, int option)
{
	if(unlikely(AbstractMethodPtr == 0)) {
		AbstractMethodPtr = mtd->invokeKMethodFunc;
	}

	kNameSpace *ns = block->BlockNameSpace;
	KBuilder builderbuf = {}, *builder = &builderbuf;
	FuelIRBuilder Builder = {};
	INIT_GCSTACK();
	IRBuilder_Init(&Builder, kctx, ns);
	builder->builder = &Builder;
	builder->common.api = ns->builderApi;
	Block *EntryBlock = CreateBlock(BLD(builder));
	IRBuilder_setBlock(BLD(builder), EntryBlock);
	INode *Self = SetUpArguments(kctx, &Builder, mtd);

	SUGAR VisitBlock(kctx, builder, block);

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
	KFieldSet(mtd, ((kMethodVar *)mtd)->CompiledBlock, block);
	IRBuilder_Exit(&Builder);
	return (struct KVirtualCode *) code;
}

void FuelVM_Recompile(KonohaContext *kctx, kMethod *mtd)
{
	if(FuelVM_HasOptimizedCode(mtd) == false) {
		KLIB kMethod_GenCode(kctx, mtd, mtd->CompiledBlock, O2Compile);
	}
}

static void FuelVM_SetMethodCode(KonohaContext *kctx, kMethodVar *mtd, struct KVirtualCode *vcode, KMethodFunc func)
{
	if(mtd->invokeKMethodFunc == AbstractMethodPtr) {
		KLIB kMethod_SetFunc(kctx, mtd, FuelVM_RunVirtualMachine);
		mtd->vcode_start = vcode;
	}
}

static void InitStaticBuilderApi(struct KBuilderAPI *builderApi)
{
	builderApi->target = "FuelVM";
#define DEFINE_BUILDER_API(NAME) builderApi->visit##NAME = FuelVM_Visit##NAME;
	VISITOR_LIST(DEFINE_BUILDER_API);
#undef DEFINE_BUILDER_API
	builderApi->GenerateKVirtualCode = FuelVM_GenerateKVirtualCode;
	builderApi->GenerateKMethodFunc  = FuelVM_GenerateKMethodFunc;
	builderApi->SetMethodCode        = FuelVM_SetMethodCode;
	builderApi->RunVirtualMachine    = FuelVM_Run;
}

static struct KBuilderAPI *GetDefaultBuilderAPI(void)
{
	static struct KBuilderAPI builderApi = {};
	if(builderApi.target == NULL) {
		InitStaticBuilderApi(&builderApi);
	}
	return &builderApi;
}

static void FuelVMDeleteVirtualMachine(KonohaContext *kctx)
{
#ifdef FUELVM_USE_LLVM
	extern void ExitLLVM();
	ExitLLVM();
#endif
}

// -------------------------------------------------------------------------

kbool_t LoadFuelVMModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"FuelVM", K_VERSION, 0, "FuelVM",
	};
	factory->VirtualMachineInfo   = &ModuleInfo;
	factory->GetDefaultBootCode   = GetDefaultBootCode;
	factory->GetDefaultBuilderAPI = GetDefaultBuilderAPI;
	factory->DeleteVirtualMachine = FuelVMDeleteVirtualMachine;
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
