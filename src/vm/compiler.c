/****************************************************************************
 * Copyright (c) 2012, the Konoha project authors. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *	this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
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
#include <minikonoha/sugar.h>
#include "vm.h"
#include "minivm.h"
/* ************************************************************************ */

#ifdef __cplusplus
extern "C" {
#endif

int verbose_code = 0;  // global variable

/* ------------------------------------------------------------------------ */
struct IRBuilder;

typedef struct IRBuilder IRBuilder;

typedef void (*VisitStmt_t)(KonohaContext *kctx, IRBuilder *self, kStmt *stmt);
typedef void (*VisitExpr_t)(KonohaContext *kctx, IRBuilder *self, kExpr *expr);

struct IRBuilderAPI {
	VisitStmt_t visitErrStmt;
	VisitStmt_t visitExprStmt;
	VisitStmt_t visitBlockStmt;
	VisitStmt_t visitReturnStmt;
	VisitStmt_t visitIfStmt;
	VisitStmt_t visitLoopStmt;
	VisitStmt_t visitJumpStmt;
	VisitStmt_t visitUndefinedStmt;
	VisitExpr_t visitConstExpr;
	VisitExpr_t visitNConstExpr;
	VisitExpr_t visitNewExpr;
	VisitExpr_t visitNullExpr;
	VisitExpr_t visitLocalExpr;
	VisitExpr_t visitBlockExpr;
	VisitExpr_t visitFieldExpr;
	VisitExpr_t visitCallExpr;
	VisitExpr_t visitAndExpr;
	VisitExpr_t visitOrExpr;
	VisitExpr_t visitLetExpr;
	VisitExpr_t visitStackTopExpr;
	void (*fn_init)(KonohaContext *kctx, struct IRBuilder *builder, kMethod *method);
	void (*fn_free)(KonohaContext *kctx, struct IRBuilder *builder, kMethod *method);
};

#define VISITOR_LIST(OP) \
	OP(ErrStmt)\
	OP(ExprStmt)\
	OP(BlockStmt)\
	OP(ReturnStmt)\
	OP(IfStmt)\
	OP(LoopStmt)\
	OP(JumpStmt)\
	OP(UndefinedStmt)\
	OP(ConstExpr)\
	OP(NConstExpr)\
	OP(NewExpr)\
	OP(NullExpr)\
	OP(LocalExpr)\
	OP(BlockExpr)\
	OP(FieldExpr)\
	OP(CallExpr)\
	OP(AndExpr)\
	OP(OrExpr)\
	OP(LetExpr)\
	OP(StackTopExpr)

struct IRBuilder {
	struct IRBuilderAPI api;
	void *local_fields;
	kStmt* currentStmt; /*FIXME(ide): need to reftrace currentStmt */
	int a; /* whatis a ? */
	int shift;
	int espidx;
};

typedef struct DumpVisitor {
	struct IRBuilder base;
} DumpVisitor;

typedef struct DumpVisitorLocal {
	int indent;
} DumpVisitorLocal;

typedef DumpVisitor JSVisitor;
typedef DumpVisitorLocal JSVisitorLocal;

struct VMCodeBuilder {
	struct IRBuilder base;
};

static void handleStmt(KonohaContext *kctx, IRBuilder *builder, kStmt *stmt)
{
	kStmt *beforeStmt = builder->currentStmt;
	builder->currentStmt = stmt;
	switch(stmt->build) {
		case TSTMT_ERR:    builder->api.visitErrStmt(kctx, builder, stmt);    return;
		case TSTMT_EXPR:   builder->api.visitExprStmt(kctx, builder, stmt);   break;
		case TSTMT_BLOCK:  builder->api.visitBlockStmt(kctx, builder, stmt);  break;
		case TSTMT_RETURN: builder->api.visitReturnStmt(kctx, builder, stmt); return;
		case TSTMT_IF:     builder->api.visitIfStmt(kctx, builder, stmt);     break;
		case TSTMT_LOOP:   builder->api.visitLoopStmt(kctx, builder, stmt);   break;
		case TSTMT_JUMP:   builder->api.visitJumpStmt(kctx, builder, stmt);   break;
		default: builder->api.visitUndefinedStmt(kctx, builder, stmt);        break;
	}
	builder->currentStmt = beforeStmt;
}

static void handleExpr(KonohaContext *kctx, IRBuilder *builder, kExpr *expr)
{
	int a = builder->a;
	int espidx = builder->espidx;
	int shift = builder->shift;
	switch(expr->build) {
		case TEXPR_CONST:    builder->api.visitConstExpr(kctx, builder, expr);  break;
		case TEXPR_NEW:      builder->api.visitNewExpr(kctx, builder, expr);    break;
		case TEXPR_NULL:     builder->api.visitNullExpr(kctx, builder, expr);   break;
		case TEXPR_NCONST:   builder->api.visitNConstExpr(kctx, builder, expr); break;
		case TEXPR_LOCAL:    builder->api.visitLocalExpr(kctx, builder, expr);  break;
		case TEXPR_BLOCK:    builder->api.visitBlockExpr(kctx, builder, expr);  break;
		case TEXPR_FIELD:    builder->api.visitFieldExpr(kctx, builder, expr);  break;
		case TEXPR_CALL:     builder->api.visitCallExpr(kctx, builder, expr);   break;
		case TEXPR_AND:      builder->api.visitAndExpr(kctx, builder, expr);    break;
		case TEXPR_OR:       builder->api.visitOrExpr(kctx, builder, expr);     break;
		case TEXPR_LET:      builder->api.visitLetExpr(kctx, builder, expr);    break;
		case TEXPR_STACKTOP: builder->api.visitStackTopExpr(kctx, builder, expr);break;
		default: DBG_ABORT("unknown expr=%d", expr->build);
	}
	builder->a = a;
	builder->espidx = espidx;
	builder->shift = shift;
}

static void visitBlock(KonohaContext *kctx, IRBuilder *builder, kBlock *bk)
{
	int a = builder->a;
	int espidx = builder->espidx;
	int shift = builder->shift;
	builder->espidx = (bk->esp->build == TEXPR_STACKTOP) ? shift + bk->esp->index : bk->esp->index;
	int i;
	for (i = 0; i < kArray_size(bk->stmtList); i++) {
		kStmt *stmt = bk->stmtList->stmtItems[i];
		if (stmt->syn == NULL) continue;
		ctxcode->uline = stmt->uline;
		handleStmt(kctx, builder, stmt);
	}
	builder->a = a;
	builder->espidx = espidx;
	builder->shift = shift;
}

/* ------------------------------------------------------------------------ */
/* [Statement/Expression API] */
static kBlock* Stmt_getFirstBlock(KonohaContext *kctx, kStmt *stmt)
{
	return SUGAR kStmt_getBlock(kctx, stmt, NULL, KW_BlockPattern, K_NULLBLOCK);
}

static kBlock* Stmt_getElseBlock(KonohaContext *kctx, kStmt *stmt)
{
	return SUGAR kStmt_getBlock(kctx, stmt, NULL, KW_else, K_NULLBLOCK);
}

static kExpr* Stmt_getFirstExpr(KonohaContext *kctx, kStmt *stmt)
{
	return SUGAR kStmt_getExpr(kctx, stmt, KW_ExprPattern, NULL);
}

static kStmt *kStmt_getStmt(KonohaContext *kctx, kStmt *stmt, ksymbol_t kw)
{
	return (kStmt*) kStmt_getObject(kctx, stmt, kw, NULL);
}

static kMethod* CallExpr_getMethod(kExpr *expr)
{
	return expr->cons->methodItems[0];
}

static int CallExpr_getArgCount(kExpr *expr)
{
	return kArray_size(expr->cons) - 2;
}

static kString* Stmt_getErrorMessage(KonohaContext *kctx, kStmt *stmt)
{
	kString* msg = (kString*)kStmt_getObjectNULL(kctx, stmt, KW_ERR);
	DBG_ASSERT(IS_String(msg));
	return msg;
}

static void kStmt_setLabelBlock(KonohaContext *kctx, kStmt *stmt, ksymbol_t label, kBasicBlock *block)
{
	KLIB kObject_setObject(kctx, stmt, label, TY_BasicBlock, block);
}

static kBasicBlock *kStmt_getLabelBlock(KonohaContext *kctx, kStmt *stmt, ksymbol_t label)
{
	return (kBasicBlock *) KLIB kObject_getObject(kctx, stmt, label, NULL);
}

/* ------------------------------------------------------------------------ */
#ifndef PLATAPIFORM_KERNEL
#include "dumpvisitor.c"
#include "jsvisitor.c"
#endif
/* ------------------------------------------------------------------------ */
#if defined(K_USING_THCODE_)
#define TADDR   NULL, 0/*counter*/
#else
#define TADDR   0/*counter*/
#endif/*K_USING_THCODE_*/
#define ASMLINE  0

#define OP_T(T) union { VirtualMachineInstruction op; T op_; }

#define NC_(sfpidx)       (((sfpidx) * 2) + 1)
#define OC_(sfpidx)       ((sfpidx) * 2)
#define TC_(sfpidx, TYPE) ((TY_isUnbox(TYPE)) ? NC_(sfpidx) : OC_(sfpidx))
#define SFP_(sfpidx)   ((sfpidx) * 2)
#define RIX_(rix)      rix

#define BasicBlock_codesize(BB)  ((BB)->codeTable.bytesize / sizeof(VirtualMachineInstruction))
#define BBOP(BB)     (BB)->codeTable.codeItems

#define ASM(T, ...) do {\
	OP##T op_ = {TADDR, OPCODE_##T, ASMLINE, ## __VA_ARGS__};\
	union { VirtualMachineInstruction op; OP##T op_; } tmp_; tmp_.op_ = op_;\
	BUILD_asm(kctx, &tmp_.op, sizeof(OP##T));\
} while (0)

#define ASMop(T, OP, ...) do {\
	OP##T op_ = {TADDR, OP, ASMLINE, ## __VA_ARGS__};\
	union { VirtualMachineInstruction op; OP##T op_; } tmp_; tmp_.op_ = op_;\
	BUILD_asm(kctx, &tmp_.op, sizeof(OP##T));\
} while (0)

#define ASMbranch(T, lb, ...) do {\
	OP##T op_ = {TADDR, OPCODE_##T, ASMLINE, NULL, ## __VA_ARGS__};\
	union { VirtualMachineInstruction op; OP##T op_; } tmp_; tmp_.op_ = op_;\
	ASM_BRANCH_(kctx, lb, &tmp_.op, sizeof(OP##T)); \
} while (0)

#define kBasicBlock_add(bb, T, ...) do { \
	OP##T op_ = {TADDR, OPCODE_##T, ASMLINE, ## __VA_ARGS__};\
	union { VirtualMachineInstruction op; OP##T op_; } tmp_; tmp_.op_ = op_;\
	BasicBlock_add(kctx, bb, 0, &tmp_.op, sizeof(OP##T));\
} while (0)

static kBasicBlock* new_BasicBlockLABEL(KonohaContext *kctx)
{
	kBasicBlock *bb = new_(BasicBlock, 0);
	bb->id = kArray_size(ctxcode->codeList);
	KLIB kArray_add(kctx, ctxcode->codeList, bb);
	return bb;
}

static void BasicBlock_add(KonohaContext *kctx, kBasicBlock *bb, kushort_t line, VirtualMachineInstruction *op, size_t size)
{
	if (bb->codeTable.bytemax == 0) {
		KLIB Karray_init(kctx, &(bb->codeTable), 1 * sizeof(VirtualMachineInstruction));
	}
	else if (bb->codeTable.bytesize == bb->codeTable.bytemax) {
		KLIB Karray_expand(kctx, &(bb->codeTable), 4 * sizeof(VirtualMachineInstruction));
	}
	VirtualMachineInstruction *tailcode = bb->codeTable.codeItems + (bb->codeTable.bytesize/sizeof(VirtualMachineInstruction));
	memcpy(tailcode, op, size == 0 ? sizeof(VirtualMachineInstruction) : size);
	tailcode->line = line;
	bb->codeTable.bytesize += sizeof(VirtualMachineInstruction);
}

static kObject* BUILD_addConstPool(KonohaContext *kctx, kObject *o)
{
	KLIB kArray_add(kctx, ctxcode->constPools, o);
	return o;
}

static void BUILD_asm(KonohaContext *kctx, VirtualMachineInstruction *op, size_t opsize)
{
	assert(op->opcode != OPCODE_JMPF);
	BasicBlock_add(kctx, ctxcode->currentWorkingBlock, ctxcode->uline, op, opsize);
}

static int BUILD_asmJMPF(KonohaContext *kctx, OPJMPF *op)
{
	kBasicBlock *bb = ctxcode->currentWorkingBlock;
	DBG_ASSERT(op->opcode == OPCODE_JMPF);
	BasicBlock_add(kctx, bb, ctxcode->uline, (VirtualMachineInstruction*)op, 0);
	return 0;
}

static void ASM_LABEL(KonohaContext *kctx, kBasicBlock *labelBlock)
{
	if (labelBlock != NULL) {
		kBasicBlock *bb = ctxcode->currentWorkingBlock;
		if (bb != NULL) {
			bb->nextBlock = labelBlock;
			bb->branchBlock = NULL;
			labelBlock->incoming += 1;
		}
		ctxcode->currentWorkingBlock = labelBlock;
	}
}

static void ASM_JMP(KonohaContext *kctx, kBasicBlock *labelBlock)
{
	kBasicBlock *bb = ctxcode->currentWorkingBlock;
	if (bb != NULL) {
		bb->nextBlock = NULL;
		bb->branchBlock = labelBlock;
		labelBlock->incoming += 1;
	}
	ctxcode->currentWorkingBlock = NULL;
}

static kBasicBlock* ASM_JMPF(KonohaContext *kctx, int flocal, kBasicBlock *lbJUMP)
{
	kBasicBlock *bb = ctxcode->currentWorkingBlock;
	kBasicBlock *lbNEXT = new_BasicBlockLABEL(kctx);
	OPJMPF op = {TADDR, OPCODE_JMPF, ASMLINE, NULL, NC_(flocal)};
	if (BUILD_asmJMPF(kctx, &op)) {
		bb->branchBlock = lbNEXT;
		bb->nextBlock = lbJUMP;
	}
	else {
		bb->branchBlock = lbJUMP;
		bb->nextBlock = lbNEXT;
	}
	lbNEXT->incoming += 1;
	ctxcode->currentWorkingBlock = lbNEXT;
	lbJUMP->incoming += 1;
	return lbJUMP;
}

static void ASM_SAFEPOINT(KonohaContext *kctx, int espidx)
{
	kBasicBlock *bb = ctxcode->currentWorkingBlock;
	size_t i;
	for (i = 0; i < BasicBlock_codesize(bb); i++) {
		VirtualMachineInstruction *op = BBOP(bb) + i;
		if (op->opcode == OPCODE_SAFEPOINT) return;
	}
	ASM(SAFEPOINT, SFP_(espidx));
}

static void NMOV_asm(KonohaContext *kctx, int a, ktype_t ty, int b)
{
	ASM(NMOV, TC_(a, ty), TC_(b, ty), CT_(ty));
}

static kBasicBlock* KonohaVisitor_asmJMPIF(KonohaContext *kctx, IRBuilder *self, kExpr *expr, int isTRUE, kBasicBlock* label)
{
	int a = self->a;
	handleExpr(kctx, self, expr);
	if (isTRUE) {
		ASM(BNOT, NC_(a), NC_(a));
	}
	return ASM_JMPF(kctx, a, label);
}


static void KonohaVisitor_visitErrStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	ASM(ERROR, stmt->uline, Stmt_getErrorMessage(kctx, stmt));
}

static void KonohaVisitor_visitExprStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	int a = self->a;
	self->a = self->espidx;
	handleExpr(kctx, self, Stmt_getFirstExpr(kctx, stmt));
	self->a = a;
}

static void KonohaVisitor_visitBlockStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	visitBlock(kctx, self, Stmt_getFirstBlock(kctx, stmt));
}

static void KonohaVisitor_visitReturnStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	kExpr *expr = SUGAR kStmt_getExpr(kctx, stmt, KW_ExprPattern, NULL);
	if (expr != NULL && IS_Expr(expr) && expr->ty != TY_void) {
		int a = self->a;
		self->a = K_RTNIDX;
		handleExpr(kctx, self, expr);
		self->a = a;
	}
	ASM_JMP(kctx, ctxcode->lbEND); // RET
}

static void KonohaVisitor_visitIfStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	int espidx = self->espidx;
	int a = self->a;
	kBasicBlock*  lbELSE = new_BasicBlockLABEL(kctx);
	kBasicBlock*  lbEND  = new_BasicBlockLABEL(kctx);
	/* if */
	self->a = espidx;
	lbELSE = KonohaVisitor_asmJMPIF(kctx, self, Stmt_getFirstExpr(kctx, stmt), 0/*FALSE*/, lbELSE);
	self->a = a;
	/* then */
	visitBlock(kctx, self, Stmt_getFirstBlock(kctx, stmt));
	ASM_JMP(kctx, lbEND);
	/* else */
	ASM_LABEL(kctx, lbELSE);
	visitBlock(kctx, self, Stmt_getElseBlock(kctx, stmt));
	/* endif */
	ASM_LABEL(kctx, lbEND);
}

static void KonohaVisitor_visitLoopStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	int espidx = self->espidx;
	int a = self->a;
	kBasicBlock* lbCONTINUE = new_BasicBlockLABEL(kctx);
	kBasicBlock* lbBREAK    = new_BasicBlockLABEL(kctx);

	kStmt_setLabelBlock(kctx, stmt, SYM_("continue"), lbCONTINUE);
	kStmt_setLabelBlock(kctx, stmt, SYM_("break"),    lbBREAK);
	ASM_LABEL(kctx, lbCONTINUE);
	ASM_SAFEPOINT(kctx, espidx);
	self->a = espidx;
	KonohaVisitor_asmJMPIF(kctx, self, Stmt_getFirstExpr(kctx, stmt), 0/*FALSE*/, lbBREAK);
	self->a = a;
	visitBlock(kctx, self, Stmt_getFirstBlock(kctx, stmt));
	ASM_JMP(kctx, lbCONTINUE);
	ASM_LABEL(kctx, lbBREAK);
}

static void KonohaVisitor_visitJumpStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	SugarSyntax *syn = stmt->syn;
	kStmt *jump = kStmt_getStmt(kctx, stmt, syn->keyword);
	DBG_ASSERT(jump != NULL && IS_Stmt(jump));
	kBasicBlock* lbJUMP = kStmt_getLabelBlock(kctx, jump, syn->keyword);
	DBG_ASSERT(lbJUMP != NULL && IS_BasicBlock(lbJUMP));
	ASM_JMP(kctx, lbJUMP);
}

static void KonohaVisitor_visitUndefinedStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	DBG_P("undefined asm syntax kw='%s'", SYM_t(stmt->syn->keyword));
}

static void KonohaVisitor_visitConstExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	int a = self->a;
	kObject *v = expr->objectConstValue;
	DBG_ASSERT(!TY_isUnbox(expr->ty));
	DBG_ASSERT(Expr_hasObjectConstValue(expr));
	v = BUILD_addConstPool(kctx, v);
	ASM(NSET, OC_(a), (uintptr_t)v, CT_(expr->ty));
}

static void KonohaVisitor_visitNConstExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	int a = self->a;
	ASM(NSET, NC_(a), expr->unboxConstValue, CT_(expr->ty));
}

static void KonohaVisitor_visitNewExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	int a = self->a;
	ASM(NEW, OC_(a), expr->index, CT_(expr->ty));
}

static void KonohaVisitor_visitNullExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	int a = self->a;
	if (TY_isUnbox(expr->ty)) {
		ASM(NSET, NC_(a), 0, CT_(expr->ty));
	}
	else {
		ASM(NULL, OC_(a), CT_(expr->ty));
	}
}

static void KonohaVisitor_visitLocalExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	NMOV_asm(kctx, self->a, expr->ty, expr->index);
}

static void KonohaVisitor_visitBlockExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	int a      = self->a;
	int shift  = self->shift;
	int espidx = self->espidx;
	DBG_ASSERT(IS_Block(expr->block));
	self->shift = self->espidx;
	visitBlock(kctx, self, expr->block);
	self->shift = shift;
	NMOV_asm(kctx, a, expr->ty, espidx);
	self->espidx = espidx;
}

static void KonohaVisitor_visitFieldExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	int a = self->a;
	kshort_t index = (kshort_t)expr->index;
	kshort_t xindex = (kshort_t)(expr->index >> (sizeof(kshort_t)*8));
	ASM(NMOVx, TC_(a, expr->ty), OC_(index), xindex, CT_(expr->ty));
}

static void KonohaVisitor_visitCallExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	kMethod *mtd = CallExpr_getMethod(expr);
	DBG_ASSERT(IS_Method(mtd));

	/*
	 * [CallExpr] := this.method(arg1, arg2, ...)
	 * expr->cons = [method, this, arg1, arg2, ...]
	 **/
	int i, a = self->a;
	int s = kMethod_is(Static, mtd) ? 2 : 1;
	int espidx  = self->espidx;
	int thisidx = espidx + K_CALLDELTA;
	int argc = CallExpr_getArgCount(expr);
	for (i = s; i < argc + 2; i++) {
		kExpr *exprN = kExpr_at(expr, i);
		DBG_ASSERT(IS_Expr(exprN));
		self->a = self->espidx = thisidx + i - 1;
		handleExpr(kctx, self, exprN);
	}
	self->espidx = espidx;
	self->a = a;

	if (kMethod_is(Final, mtd) || !kMethod_is(Virtual, mtd)) {
		ASM(NSET, NC_(thisidx-1), (intptr_t)mtd, CT_Method);
	}
	else {
		ASM(LOOKUP, SFP_(thisidx), Stmt_nameSpace(self->currentStmt), mtd);
	}

	int esp_ = SFP_(espidx + argc + K_CALLDELTA + 1);
	ASM(CALL, ctxcode->uline, SFP_(thisidx), esp_, KLIB Knull(kctx, CT_(expr->ty)));

	if (a != espidx) {
		NMOV_asm(kctx, a, expr->ty, espidx);
	}
}

static void KonohaVisitor_visitAndExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	int a = self->a;
	int i, size = kArray_size(expr->cons);
	kBasicBlock* lbTRUE  = new_BasicBlockLABEL(kctx);
	kBasicBlock* lbFALSE = new_BasicBlockLABEL(kctx);
	/*
	 * [AndExpr] := (arg0 && arg1 && arg2 && ...)
	 * expr->cons = [NULL, arg0, arg1, arg2, ...]
	 **/
	for (i = 1; i < size; i++) {
		KonohaVisitor_asmJMPIF(kctx, self, kExpr_at(expr, i), 0/*FALSE*/, lbFALSE);
	}
	ASM(NSET, NC_(a), 1/*TRUE*/, CT_Boolean);
	ASM_JMP(kctx, lbTRUE);
	ASM_LABEL(kctx, lbFALSE); // false
	ASM(NSET, NC_(a), 0/*FALSE*/, CT_Boolean);
	ASM_LABEL(kctx, lbTRUE);   // TRUE
}

static void KonohaVisitor_visitOrExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	int a = self->a;
	int i, size = kArray_size(expr->cons);
	kBasicBlock* lbTRUE  = new_BasicBlockLABEL(kctx);
	kBasicBlock* lbFALSE = new_BasicBlockLABEL(kctx);
	for (i = 1; i < size; i++) {
		KonohaVisitor_asmJMPIF(kctx, self, kExpr_at(expr, i), 1/*TRUE*/, lbTRUE);
	}
	ASM(NSET, NC_(a), 0/*FALSE*/, CT_Boolean);
	ASM_JMP(kctx, lbFALSE);
	ASM_LABEL(kctx, lbTRUE);
	ASM(NSET, NC_(a), 1/*TRUE*/, CT_Boolean);
	ASM_LABEL(kctx, lbFALSE); // false
}

static void KonohaVisitor_visitLetExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	int a = self->a;
	int shift = self->shift;
	int espidx = self->espidx;

	/*
	 * [LetExpr] := lhs = rhs
	 * expr->cons = [NULL, lhs, rhs]
	 **/

	kExpr *leftHandExpr = kExpr_at(expr, 1);
	kExpr *rightHandExpr = kExpr_at(expr, 2);
	DBG_P("LET (%s) a=%d, shift=%d, espidx=%d", TY_t(expr->ty), a, shift, espidx);
	if (leftHandExpr->build == TEXPR_LOCAL) {
		self->a = leftHandExpr->index;
		handleExpr(kctx, self, rightHandExpr);
		self->a = a;
		if (expr->ty != TY_void && a != leftHandExpr->index) {
			NMOV_asm(kctx, a, leftHandExpr->ty, leftHandExpr->index);
		}
	}
	else if (leftHandExpr->build == TEXPR_STACKTOP) {
		DBG_P("LET TEXPR_STACKTOP a=%d, leftHandExpr->index=%d, espidx=%d", a, leftHandExpr->index, espidx);
		self->a = leftHandExpr->index + shift;
		handleExpr(kctx, self, rightHandExpr);
		self->a = a;
		if (expr->ty != TY_void && a != leftHandExpr->index + shift) {
			NMOV_asm(kctx, a, leftHandExpr->ty, leftHandExpr->index + shift);
		}
	}
	else{
		assert(leftHandExpr->build == TEXPR_FIELD);
		self->a = espidx;
		handleExpr(kctx, self, rightHandExpr);
		self->a = a;
		kshort_t index  = (kshort_t)leftHandExpr->index;
		kshort_t xindex = (kshort_t)(leftHandExpr->index >> (sizeof(kshort_t)*8));
		KonohaClass *lhsClass = CT_(leftHandExpr->ty);
		ASM(XNMOV, OC_(index), xindex, TC_(espidx, rightHandExpr->ty), lhsClass);
		if (expr->ty != TY_void) {
			ASM(NMOVx, TC_(a, rightHandExpr->ty), OC_(index), xindex, lhsClass);
		}
	}
}

static void KonohaVisitor_visitStackTopExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	int shift = self->shift;
	int a = self->a;
	int espidx = self->espidx;
	DBG_ASSERT(expr->index + shift < espidx);
	NMOV_asm(kctx, a, expr->ty, expr->index + shift);
}

static KMETHOD MethodFunc_runVirtualMachine(KonohaContext *kctx, KonohaStack *sfp);
static void _THCODE(KonohaContext *kctx, VirtualMachineInstruction *pc, void **codeaddr);
static void BUILD_compile(KonohaContext *kctx, kMethod *mtd, kBasicBlock *beginBlock, kBasicBlock *endBlock);

static void KonohaVisitor_init(KonohaContext *kctx, struct IRBuilder *builder, kMethod *mtd)
{
	builder->espidx = 0;
	builder->a = 0;
	builder->currentStmt = NULL;
	builder->shift = 0;

	KLIB kMethod_setFunc(kctx, mtd, MethodFunc_runVirtualMachine);

	DBG_ASSERT(kArray_size(ctxcode->codeList) == 0);
	kBasicBlock* lbINIT  = new_BasicBlockLABEL(kctx);
	kBasicBlock* lbBEGIN = new_BasicBlockLABEL(kctx);
	ctxcode->lbINIT = lbINIT;
	ctxcode->lbEND  = new_BasicBlockLABEL(kctx);
	PUSH_GCSTACK(lbINIT);
	PUSH_GCSTACK(lbBEGIN);
	PUSH_GCSTACK(ctxcode->lbEND);
	ctxcode->currentWorkingBlock = lbINIT;
	ASM(THCODE, _THCODE);
	ASM(CHKSTACK, 0);
	ASM_LABEL(kctx, lbBEGIN);
}

static void KonohaVisitor_free(KonohaContext *kctx, struct IRBuilder *builder, kMethod *mtd)
{
	builder->shift = 0;
	ASM_LABEL(kctx, ctxcode->lbEND);
	if (mtd->mn == MN_new) {
		ASM(NMOV, OC_(K_RTNIDX), OC_(0), CT_(mtd->typeId));   // FIXME: Type 'This' must be resolved
	}
	ASM(RET);
	assert(ctxcode->lbEND);/* scan-build: remove warning */
	BUILD_compile(kctx, mtd, ctxcode->lbINIT, ctxcode->lbEND);
	ctxcode->lbINIT = NULL;
	ctxcode->lbEND  = NULL;
}

static IRBuilder *createKonohaVisitor(IRBuilder *builder)
{
#define DEFINE_BUILDER_API(NAME) builder->api.visit##NAME = KonohaVisitor_visit##NAME;
	VISITOR_LIST(DEFINE_BUILDER_API);
#undef DEFINE_BUILDER_API
	builder->api.fn_init = KonohaVisitor_init;
	builder->api.fn_free = KonohaVisitor_free;
	return builder;
}

/* ------------------------------------------------------------------------ */

static inline kopcode_t BasicBlock_opcode(kBasicBlock *bb)
{
	if (bb->codeTable.bytesize == 0)
		return OPCODE_NOP;
	else
		return bb->codeTable.codeItems->opcode;
}

static void BasicBlock_strip0(KonohaContext *kctx, kBasicBlock *bb)
{
	while (true) {
		if (BasicBlock_isVisited(bb)) return;
		BasicBlock_setVisited(bb, 1);
		if (bb->branchBlock != NULL) {
			L_JUMP:; {
				kBasicBlock *bbJ = (kBasicBlock*)bb->branchBlock;
				if (bbJ->codeTable.bytesize == 0) {
					if (bbJ->branchBlock != NULL && bbJ->nextBlock == NULL) {
						//DBG_P("DIRECT JMP id=%d JMP to id=%d", bbJ->id, DP(bbJ->branchBlock)->id);
						bbJ->incoming -= 1;
						bb->branchBlock = bbJ->branchBlock;
						bb->branchBlock->incoming += 1;
						goto L_JUMP;
					}
					if (bbJ->branchBlock == NULL && bbJ->nextBlock != NULL) {
						//DBG_P("DIRECT JMP id=%d NEXT to id=%d", bbJ->id, DP(bbJ->nextBlock)->id);
						bbJ->incoming -= 1;
						bb->branchBlock = bbJ->nextBlock;
						bb->branchBlock->incoming += 1;
						goto L_JUMP;
					}
				}
				if (bb->nextBlock == NULL) {
					if (bbJ->incoming == 1 ) {
						//DBG_P("REMOVED %d JMP TO %d", bb->id, bbJ->id);
						bb->nextBlock = bbJ;
						bb->branchBlock = NULL;
						goto L_NEXT;
					}
				}
				BasicBlock_strip0(kctx, bbJ);
			}
		}
		if (bb->branchBlock != NULL && bb->nextBlock != NULL) {
			bb = bb->nextBlock;
			continue;
		}
		L_NEXT:;
		if (bb->nextBlock != NULL) {
			kBasicBlock *bbN = bb->nextBlock;
			if (bbN->codeTable.bytesize == 0) {
				if (bbN->nextBlock != NULL && bbN->branchBlock == NULL) {
					bbN->incoming -= 1;
					bb->nextBlock = bbN->nextBlock;
					bb->nextBlock->incoming += 1;
					goto L_NEXT;
				}
				if (bbN->nextBlock == NULL && bbN->branchBlock != NULL) {
					bbN->incoming -= 1;
					bb->nextBlock = NULL;
					bb->branchBlock = bbN->branchBlock;
					bb->branchBlock->incoming += 1;
					goto L_JUMP;
				}
			}
			bb = bb->nextBlock;
		}
	}
}

static void BasicBlock_join(KonohaContext *kctx, kBasicBlock *bb, kBasicBlock *bbN)
{
	//DBG_P("join %d(%s) size=%d and %d(%s) size=%d", bb->id, BB(bb), bb->size, bbN->id, BB(bbN), bbN->size);
	bb->nextBlock = bbN->nextBlock;
	bb->branchBlock = bbN->branchBlock;
	if (bbN->codeTable.bytesize == 0) {
		return;
	}
	if (bb->codeTable.bytesize == 0) {
		DBG_ASSERT(bb->codeTable.bytemax == 0);
		bb->codeTable = bbN->codeTable;
		bbN->codeTable.codeItems = NULL;
		bbN->codeTable.bytemax = 0;
		bbN->codeTable.bytesize = 0;
		return;
	}
	if (bb->codeTable.bytemax < bb->codeTable.bytesize + bbN->codeTable.bytesize) {
		KLIB Karray_expand(kctx, &bb->codeTable, (bb->codeTable.bytesize + bbN->codeTable.bytesize));
	}
	memcpy(bb->codeTable.bytebuf + bb->codeTable.bytesize, bbN->codeTable.bytebuf, bbN->codeTable.bytesize);
	bb->codeTable.bytesize += bbN->codeTable.bytesize;
	KLIB Karray_free(kctx, &bbN->codeTable);
}

static void BasicBlock_strip1(KonohaContext *kctx, kBasicBlock *bb)
{
	while (true) {
		if (!BasicBlock_isVisited(bb)) return;
		BasicBlock_setVisited(bb, 0);  // MUST call after strip0
		if (bb->branchBlock != NULL) {
			if (bb->nextBlock == NULL) {
				bb = bb->branchBlock;
			}
			else {
				BasicBlock_strip1(kctx, bb->branchBlock);
				bb = bb->nextBlock;
			}
			continue;
		}
		if (bb->nextBlock != NULL) {
			kBasicBlock *bbN = bb->nextBlock;
			if (bbN->incoming == 1 && BasicBlock_opcode(bbN) != OPCODE_RET) {
				BasicBlock_join(kctx, bb, bbN);
				BasicBlock_setVisited(bb, 1);
				continue;
			}
			bb = bb->nextBlock;
		}
	}
}

static size_t BasicBlock_peephole(KonohaContext *kctx, kBasicBlock *bb)
{
	size_t i, bbsize = BasicBlock_codesize(bb);
	for (i = 0; i < BasicBlock_codesize(bb); i++) {
		VirtualMachineInstruction *op = BBOP(bb) + i;
		if (op->opcode == OPCODE_NOP) {
			bbsize--;
		}
	}
	if (bbsize < BasicBlock_codesize(bb)) {
		VirtualMachineInstruction *opD = BBOP(bb);
		for (i = 0; i < BasicBlock_codesize(bb); i++) {
			VirtualMachineInstruction *opS = BBOP(bb) + i;
			if (opS->opcode == OPCODE_NOP) continue;
			if (opD != opS) {
				*opD = *opS;
			}
			opD++;
		}
		((kBasicBlock*)bb)->codeTable.bytesize = bbsize * sizeof(VirtualMachineInstruction);
	}
	return BasicBlock_codesize(bb); /*bbsize*/;
}

#define BB_(bb)   (bb != NULL) ? bb->id : -1

static size_t BasicBlock_size(KonohaContext *kctx, kBasicBlock *bb, size_t c)
{
	L_TAIL:;
	if (bb == NULL || BasicBlock_isVisited(bb)) return c;
	BasicBlock_setVisited(bb, 1);
	if (bb->nextBlock != NULL) {
		if (BasicBlock_isVisited(bb) || BasicBlock_opcode(bb->nextBlock) == OPCODE_RET) {
			kBasicBlock *bb2 = (kBasicBlock*)new_BasicBlockLABEL(kctx);
			bb2->branchBlock = bb->nextBlock;
			((kBasicBlock*)bb)->nextBlock = bb2;
		}
	}
	if (bb->branchBlock != NULL && bb->nextBlock != NULL) {
		DBG_ASSERT(bb->branchBlock != bb->nextBlock);
		c = BasicBlock_size(kctx, bb->nextBlock, c + BasicBlock_peephole(kctx, bb));
		bb = bb->branchBlock; goto L_TAIL;
	}
	if (bb->branchBlock != NULL) {
		DBG_ASSERT(bb->nextBlock == NULL);
		kBasicBlock_add(bb, JMP);
		c = BasicBlock_peephole(kctx, bb) + c;
		bb = bb->branchBlock;
		goto L_TAIL;
	}
	c = BasicBlock_peephole(kctx, bb) + c;
	bb = bb->nextBlock;
	goto L_TAIL;
}

static VirtualMachineInstruction* BasicBlock_copy(KonohaContext *kctx, VirtualMachineInstruction *dst, kBasicBlock *bb, kBasicBlock **prev)
{
	BasicBlock_setVisited(bb, 0);
	DBG_ASSERT(!BasicBlock_isVisited(bb));
//	DBG_P("BB%d: asm nextBlock=BB%d, branchBlock=BB%d", BB_(bb), BB_(bb->nextBlock), BB_(bb->branchBlock));
	if (bb->code != NULL) {
		return dst;
	}
	if (prev[0] != NULL && prev[0]->nextBlock == NULL && prev[0]->branchBlock == bb) {
		dst -= 1;
		//DBG_P("BB%d: REMOVE unnecessary JMP/(?%s)", BB_(bb), T_opcode(dst->opcode));
		DBG_ASSERT(dst->opcode == OPCODE_JMP/* || dst->opcode == OPJMP_*/);
		prev[0]->branchBlock = NULL;
		prev[0]->nextBlock = bb;
	}
	bb->code = dst;
	if (BasicBlock_codesize(bb) > 0) {
		memcpy(dst, BBOP(bb), sizeof(VirtualMachineInstruction) * BasicBlock_codesize(bb));
		if (bb->branchBlock != NULL) {
			bb->opjmp = (dst + (BasicBlock_codesize(bb) - 1));
		}
		dst = dst + BasicBlock_codesize(bb);
		KLIB Karray_free(kctx, &bb->codeTable);
		prev[0] = bb;
	}
	if (bb->nextBlock != NULL) {
		//DBG_P("BB%d: NEXT=BB%d", BB_(bb), BB_(bb->nextBlock));
		DBG_ASSERT(bb->nextBlock->code == NULL);
		dst = BasicBlock_copy(kctx, dst, bb->nextBlock, prev);
	}
	if (bb->branchBlock != NULL) {
		dst = BasicBlock_copy(kctx, dst, bb->branchBlock, prev);
	}
	return dst;
}

static void BasicBlock_setjump(kBasicBlock *bb)
{
	while (bb != NULL) {
		BasicBlock_setVisited(bb, 1);
		if (bb->branchBlock != NULL) {
			kBasicBlock *bbJ = bb->branchBlock;
			OPJMP *j = (OPJMP*)bb->opjmp;
			j->jumppc = bbJ->code;
			bb->branchBlock = NULL;
			if (!BasicBlock_isVisited(bbJ)) {
				BasicBlock_setVisited(bbJ, 1);
				BasicBlock_setjump(bbJ);
			}
		}
		bb = bb->nextBlock;
	}
}

static kByteCode* new_ByteCode(KonohaContext *kctx, kBasicBlock *beginBlock, kBasicBlock *endBlock)
{
#define CT_ByteCodeVar CT_ByteCode
	kByteCodeVar *kcode = GCSAFE_new(ByteCodeVar, NULL);
	kBasicBlock *prev[1] = {};
	kcode->fileid = ctxcode->uline; //TODO
	kcode->codesize = BasicBlock_size(kctx, beginBlock, 0) * sizeof(VirtualMachineInstruction);
	kcode->code = (VirtualMachineInstruction*)KCALLOC(kcode->codesize, 1);
	endBlock->code = kcode->code; // dummy
	{
		VirtualMachineInstruction *op = BasicBlock_copy(kctx, kcode->code, beginBlock, prev);
		DBG_ASSERT(op - kcode->code > 0);
		endBlock->code = NULL;
		BasicBlock_copy(kctx, op, endBlock, prev);
		BasicBlock_setjump(beginBlock);
	}
	return kcode;
}

static void dumpOPCODE(KonohaContext *kctx, VirtualMachineInstruction *c, VirtualMachineInstruction *pc_start)
{
	size_t i, size = OPDATA[c->opcode].size;
	const kushort_t *vmt = OPDATA[c->opcode].types;
	if (pc_start == NULL) {
		DUMP_P("[%p:%d]\t%s(%d)", c, c->line, T_opcode(c->opcode), (int)c->opcode);
	}
	else {
		DUMP_P("[L%d:%d]\t%s(%d)", (int)(c - pc_start), c->line, T_opcode(c->opcode), (int)c->opcode);
	}
	for (i = 0; i < size; i++) {
		DUMP_P(" ");
		switch(vmt[i]) {
			case VMT_VOID: break;
			case VMT_ADDR:
				if (pc_start == NULL) {
					DUMP_P("%p", c->p[i]);
				}
				else {
					DUMP_P("L%d", (int)((VirtualMachineInstruction*)c->p[i] - pc_start));
				}
				break;
			case VMT_R:
				DUMP_P("sfp[%d,r=%d]", (int)c->data[i]/2, (int)c->data[i]);
				break;
			case VMT_U:
				DUMP_P("u%lu", c->data[i]); break;
			case VMT_I:
			case VMT_INT:
				DUMP_P("i%ld", c->data[i]); break;
			case VMT_F:
				DUMP_P("function(%p)", c->p[i]); break;
			case VMT_CID:
				DUMP_P("CT(%s)", CT_t(c->ct[i])); break;
			case VMT_CO:
				DUMP_P("CT(%s)", CT_t(O_ct(c->o[i]))); break;
		}/*switch*/
	}
	DUMP_P("\n");
}

static void _THCODE(KonohaContext *kctx, VirtualMachineInstruction *pc, void **codeaddr)
{
#ifdef K_USING_THCODE_
	while (1) {
		pc->codeaddr = codeaddr[pc->opcode];
		if (pc->opcode == OPCODE_RET) break;
		pc++;
	}
#endif
}

/* ------------------------------------------------------------------------ */

static KMETHOD MethodFunc_runVirtualMachine(KonohaContext *kctx, KonohaStack *sfp)
{
	DBG_ASSERT(IS_Method(sfp[K_MTDIDX].mtdNC));
	KonohaVirtualMachine_run(kctx, sfp, CODE_ENTER);
}

static void Method_threadCode(KonohaContext *kctx, kMethod *mtd, kByteCode *kcode)
{
	kMethodVar *Wmtd = (kMethodVar*)mtd;
	KLIB kMethod_setFunc(kctx, mtd, MethodFunc_runVirtualMachine);
	KSETv(Wmtd, Wmtd->kcode, kcode);
	Wmtd->pc_start = KonohaVirtualMachine_run(kctx, kctx->esp + 1, kcode->code);
	if (verbose_code) {
		DBG_P("DUMP CODE");
		VirtualMachineInstruction *pc = mtd->pc_start;
		while (1) {
			dumpOPCODE(kctx, pc, mtd->pc_start);
			if (pc->opcode == OPCODE_RET) {
				break;
			}
			pc++;
		}
	}
}

static void BUILD_compile(KonohaContext *kctx, kMethod *mtd, kBasicBlock *beginBlock, kBasicBlock *endBlock)
{
	BasicBlock_strip0(kctx, beginBlock);
	BasicBlock_strip1(kctx, beginBlock);
	kByteCode *kcode = new_ByteCode(kctx, beginBlock, endBlock);
	Method_threadCode(kctx, mtd, kcode);
	KLIB kArray_clear(kctx, ctxcode->codeList, 0);
}

static void kMethod_genCode(KonohaContext *kctx, kMethod *mtd, kBlock *bk)
{
	DBG_P("START CODE GENERATION..");
	INIT_GCSTACK();
	if (ctxcode == NULL) {
		kmodcode->h.setup(kctx, NULL, 1);
	}

	IRBuilder *builder, builderbuf;
#ifdef USE_DUMP_VISITOR
	{
		builder = createDumpVisitor(&builderbuf);
		builder->api.fn_init(kctx, builder, mtd);
		visitBlock(kctx, builder, bk);
		builder->api.fn_free(kctx, builder, mtd);
	}
#endif
#ifdef USE_JS_VISITOR
	{
		builder = createJSVisitor(&builderbuf);
		builder->api.fn_init(kctx, builder, mtd);
		visitBlock(kctx, builder, bk);
		builder->api.fn_free(kctx, builder, mtd);
	}
#endif
	{
		builder = createKonohaVisitor(&builderbuf);
		builder->api.fn_init(kctx, builder, mtd);
		visitBlock(kctx, builder, bk);
		builder->api.fn_free(kctx, builder, mtd);
	}

	RESET_GCSTACK();
}

/* ------------------------------------------------------------------------ */
/* [datatype] */

static KMETHOD MethodFunc_invokeAbstractMethod(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(0);
}

static void kMethod_setFunc(KonohaContext *kctx, kMethod *mtd, MethodFunc func)
{
	func = (func != NULL) ? func : MethodFunc_invokeAbstractMethod;
	((kMethodVar*)mtd)->invokeMethodFunc = func;
	((kMethodVar*)mtd)->pc_start = CODE_NCALL;
}

static void BasicBlock_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kBasicBlock *bb = (kBasicBlock*)o;
	bb->codeTable.bytemax = 0;
	bb->codeTable.bytesize = 0;
	bb->code = NULL;
	bb->id = 0;
	bb->incoming = 0;
	bb->nextBlock  = NULL;
	bb->branchBlock  = NULL;
	bb->codeTable.codeItems = NULL;
	bb->opjmp = NULL;
}

static void BasicBlock_free(KonohaContext *kctx, kObject *o)
{
	kBasicBlock *bb = (kBasicBlock*)o;
	KLIB Karray_free(kctx, &bb->codeTable);
}

static void ByteCode_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kByteCodeVar *b = (kByteCodeVar*)o;
	b->codesize = 0;
	b->code = NULL;
	b->fileid = 0;
	KINITv(b->source, TS_EMPTY);
}

static void ByteCode_reftrace(KonohaContext *kctx, kObject *o)
{
	kByteCode *b = (kByteCode*)o;
	BEGIN_REFTRACE(1);
	KREFTRACEv(b->source);
	END_REFTRACE();
}

static void ByteCode_free(KonohaContext *kctx, kObject *o)
{
	kByteCode *b = (kByteCode*)o;
	KFREE(b->code, b->codesize);
}

static void ctxcode_reftrace(KonohaContext *kctx, struct KonohaModuleContext *baseh)
{
	ctxcode_t *base = (ctxcode_t*)baseh;
	BEGIN_REFTRACE(2);
	KREFTRACEv(base->codeList);
	KREFTRACEv(base->constPools);
	END_REFTRACE();
}

static void ctxcode_free(KonohaContext *kctx, struct KonohaModuleContext *baseh)
{
	ctxcode_t *base = (ctxcode_t*)baseh;
	KFREE(base, sizeof(ctxcode_t));
}

static void kmodcode_setup(KonohaContext *kctx, struct KonohaModule *def, int newctx)
{
	if (newctx) { // lazy setup
		assert(kctx->modlocal[MOD_code] == NULL);
		ctxcode_t *base = (ctxcode_t*)KCALLOC(sizeof(ctxcode_t), 1);
		base->h.reftrace = ctxcode_reftrace;
		base->h.free     = ctxcode_free;
		KINITv(base->codeList, new_(Array, K_PAGESIZE/sizeof(void*)));
		KINITv(base->constPools, new_(Array, 64));
		kctx->modlocal[MOD_code] = (KonohaModuleContext*)base;
		INIT_GCSTACK();
		kBasicBlock* ia = (kBasicBlock*)new_(BasicBlock, 0);
		kBasicBlock* ib = (kBasicBlock*)new_(BasicBlock, 0);
		PUSH_GCSTACK(ia);
		PUSH_GCSTACK(ib);
		kBasicBlock_add(ia, THCODE, _THCODE);
		kBasicBlock_add(ia, NCALL); // FUNCCALL
		kBasicBlock_add(ia, ENTER);
		kBasicBlock_add(ia, EXIT);
		kBasicBlock_add(ib, RET);   // NEED TERMINATION
		ia->nextBlock = ib;
		kByteCode *kcode = new_ByteCode(kctx, ia, ib);
		KINITv(kmodcode->codeNull, kcode);
		VirtualMachineInstruction *pc = KonohaVirtualMachine_run(kctx, kctx->esp, kcode->code);
		CODE_ENTER = pc+1;
		KLIB kArray_clear(kctx, ctxcode->codeList, 0);
		RESET_GCSTACK();
	}
}

static void kmodcode_reftrace(KonohaContext *kctx, struct KonohaModule *baseh)
{
	KModuleByteCode *base = (KModuleByteCode*)baseh;
	BEGIN_REFTRACE(1);
	KREFTRACEn(base->codeNull);
	END_REFTRACE();
}

static void kmodcode_free(KonohaContext *kctx, struct KonohaModule *baseh)
{
	KFREE(baseh, sizeof(KModuleByteCode));
}

void MODCODE_init(KonohaContext *kctx, KonohaContextVar *ctx)
{
	KModuleByteCode *base = (KModuleByteCode*)KCALLOC(sizeof(KModuleByteCode), 1);
	opcode_check();
	base->h.name     = "minivm";
	base->h.setup    = kmodcode_setup;
	base->h.reftrace = kmodcode_reftrace;
	base->h.free     = kmodcode_free;
	KLIB KonohaRuntime_setModule(kctx, MOD_code, &base->h, 0);
	KDEFINE_CLASS defBasicBlock = {0};
	SETSTRUCTNAME(defBasicBlock, BasicBlock);
	defBasicBlock.init = BasicBlock_init;
	defBasicBlock.free = BasicBlock_free;
	
	KDEFINE_CLASS defByteCode = {0};
	SETSTRUCTNAME(defByteCode, ByteCode);
	defByteCode.init = ByteCode_init;
	defByteCode.reftrace = ByteCode_reftrace;
	defByteCode.free = ByteCode_free;
	
	base->cBasicBlock = KLIB KonohaClass_define(kctx, PackageId_sugar, NULL, &defBasicBlock, 0);
	base->cByteCode = KLIB KonohaClass_define(kctx, PackageId_sugar, NULL, &defByteCode, 0);
	kmodcode_setup(kctx, &base->h, 0);
	KonohaLibVar *l = (KonohaLibVar*)kctx->klib;
	l->kMethod_genCode = kMethod_genCode;
	l->kMethod_setFunc = kMethod_setFunc;
}

#ifdef __cplusplus
}
#endif
