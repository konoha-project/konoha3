///****************************************************************************
// * Copyright (c) 2012, the Konoha project authors. All rights reserved.
// * Redistribution and use in source and binary forms, with or without
// * modification, are permitted provided that the following conditions are met:
// *
// *  * Redistributions of source code must retain the above copyright notice,
// *	this list of conditions and the following disclaimer.
// *  * Redistributions in binary form must reproduce the above copyright
// *	notice, this list of conditions and the following disclaimer in the
// *	documentation and/or other materials provided with the distribution.
// *
// * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ***************************************************************************/
//
//#include <minikonoha/minikonoha.h>
//#include <minikonoha/sugar.h>
////#include <minikonoha/arch/minivm.h>
//#include "vm.h"
//
///* ************************************************************************ */
//
//#ifdef __cplusplus
//extern "C" {
//#endif
//
//int verbose_code = 0;  // global variable
//
//static void kMethod_setFunc(KonohaContext *kctx, kMethod *mtd, MethodFunc func);
//
///* ------------------------------------------------------------------------ */
//
//struct KBuilderAPI {
//	VisitStmtFunc visitErrStmt;
//	VisitStmtFunc visitExprStmt;
//	VisitStmtFunc visitBlockStmt;
//	VisitStmtFunc visitReturnStmt;
//	VisitStmtFunc visitIfStmt;
//	VisitStmtFunc visitLoopStmt;
//	VisitStmtFunc visitJumpStmt;
//	VisitStmtFunc visitTryStmt;
//	VisitStmtFunc visitUndefinedStmt;
//	VisitExprFunc visitConstExpr;
//	VisitExprFunc visitNConstExpr;
//	VisitExprFunc visitNewExpr;
//	VisitExprFunc visitNullExpr;
//	VisitExprFunc visitLocalExpr;
//	VisitExprFunc visitBlockExpr;
//	VisitExprFunc visitFieldExpr;
//	VisitExprFunc visitCallExpr;
//	VisitExprFunc visitAndExpr;
//	VisitExprFunc visitOrExpr;
//	VisitExprFunc visitLetExpr;
//	VisitExprFunc visitStackTopExpr;
//	void (*fn_Init)(KonohaContext *kctx, KBuilder *builder, kMethod *method);
//	void (*fn_Free)(KonohaContext *kctx, KBuilder *builder, kMethod *method);
//	struct VirtualCode *(*GenVirtualCode)(KonohaContext *, KBuilder *build, kBlock *block);
//	MethodFunc (*GetVirtualMethod)(struct VirtualCode*);
//};
//
//#define VISITOR_LIST(OP) \
//	OP(ErrStmt)\
//	OP(ExprStmt)\
//	OP(BlockStmt)\
//	OP(ReturnStmt)\
//	OP(IfStmt)\
//	OP(LoopStmt)\
//	OP(JumpStmt)\
//	OP(TryStmt)\
//	OP(UndefinedStmt)\
//	OP(ConstExpr)\
//	OP(NConstExpr)\
//	OP(NewExpr)\
//	OP(NullExpr)\
//	OP(LocalExpr)\
//	OP(BlockExpr)\
//	OP(FieldExpr)\
//	OP(CallExpr)\
//	OP(AndExpr)\
//	OP(OrExpr)\
//	OP(LetExpr)\
//	OP(StackTopExpr)
//
//struct BuilderCommon {
//	struct KBuilderAPI api;
//	int a; /* whatis a ? */
//	int shift;
//	int espidx;
//};
//
//struct KBuilder {
//	struct BuilderCommon common;
//	void *local_fields;
//};
//
////typedef struct DumpVisitor {
////	struct KBuilder base;
////} DumpVisitor;
////
////typedef struct DumpVisitorLocal {
////	int indent;
////	kbool_t isIndentEmitted;
////	kMethod *visitingMethod;
////} DumpVisitorLocal;
////
////typedef DumpVisitor JSVisitor;
////typedef DumpVisitorLocal JSVisitorLocal;
////
////struct VMCodeBuilder {
////	struct KBuilder base;
////};
//
//static void VisitStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
//{
////	kStmt *beforeStmt = builder->common.currentStmt;
////	builder->common.currentStmt = stmt;
//	switch(stmt->build) {
//		case TSTMT_ERR:    builder->common.api.visitErrStmt(kctx, builder, stmt);    return;
//		case TSTMT_EXPR:   builder->common.api.visitExprStmt(kctx, builder, stmt);   break;
//		case TSTMT_BLOCK:  builder->common.api.visitBlockStmt(kctx, builder, stmt);  break;
//		case TSTMT_RETURN: builder->common.api.visitReturnStmt(kctx, builder, stmt); return;
//		case TSTMT_IF:     builder->common.api.visitIfStmt(kctx, builder, stmt);     break;
//		case TSTMT_LOOP:   builder->common.api.visitLoopStmt(kctx, builder, stmt);   break;
//		case TSTMT_JUMP:   builder->common.api.visitJumpStmt(kctx, builder, stmt);   break;
//		case TSTMT_TRY:    builder->common.api.visitTryStmt(kctx, builder, stmt);    break;
//		default: builder->common.api.visitUndefinedStmt(kctx, builder, stmt);        break;
//	}
////	builder->common.currentStmt = beforeStmt;
//}
//
//static void VisitExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
//{
//	int a = builder->common.a;
//	int espidx = builder->common.espidx;
//	int shift = builder->common.shift;
//	switch(expr->build) {
//	case TEXPR_CONST:    builder->common.api.visitConstExpr(kctx, builder, stmt, expr);  break;
//	case TEXPR_NEW:      builder->common.api.visitNewExpr(kctx, builder, stmt, expr);    break;
//	case TEXPR_NULL:     builder->common.api.visitNullExpr(kctx, builder, stmt, expr);   break;
//	case TEXPR_NCONST:   builder->common.api.visitNConstExpr(kctx, builder, stmt, expr); break;
//	case TEXPR_LOCAL:    builder->common.api.visitLocalExpr(kctx, builder, stmt, expr);  break;
//	case TEXPR_BLOCK:    builder->common.api.visitBlockExpr(kctx, builder, stmt, expr);  break;
//	case TEXPR_FIELD:    builder->common.api.visitFieldExpr(kctx, builder, stmt, expr);  break;
//	case TEXPR_CALL:     builder->common.api.visitCallExpr(kctx, builder, stmt, expr);   break;
//	case TEXPR_AND:      builder->common.api.visitAndExpr(kctx, builder, stmt, expr);    break;
//	case TEXPR_OR:       builder->common.api.visitOrExpr(kctx, builder, stmt, expr);     break;
//	case TEXPR_LET:      builder->common.api.visitLetExpr(kctx, builder, stmt, expr);    break;
//	case TEXPR_STACKTOP: builder->common.api.visitStackTopExpr(kctx, builder, stmt, expr);break;
//	default: DBG_ABORT("unknown expr=%d", expr->build);
//	}
//	builder->common.a = a;
//	builder->common.espidx = espidx;
//	builder->common.shift = shift;
//}
//
//static void visitBlock(KonohaContext *kctx, KBuilder *builder, kBlock *block)
//{
//	int a = builder->common.a;
//	int espidx = builder->common.espidx;
//	int shift = builder->common.shift;
//	builder->common.espidx = (block->esp->build == TEXPR_STACKTOP) ? shift + block->esp->index : block->esp->index;
//	size_t i;
//	for (i = 0; i < kArray_size(block->StmtList); i++) {
//		kStmt *stmt = block->StmtList->StmtItems[i];
//		if(stmt->syn == NULL) continue;
//		ctxcode->uline = stmt->uline;
//		VisitStmt(kctx, builder, stmt);
//	}
//	builder->common.a = a;
//	builder->common.espidx = espidx;
//	builder->common.shift = shift;
//}
//
///* ------------------------------------------------------------------------ */
///* [Statement/Expression API] */
//
//static kBlock* Stmt_getFirstBlock(KonohaContext *kctx, kStmt *stmt)
//{
//	return SUGAR kStmt_GetBlock(kctx, stmt, NULL, KW_BlockPattern, K_NULLBLOCK);
//}
//
//static kBlock* Stmt_getElseBlock(KonohaContext *kctx, kStmt *stmt)
//{
//	return SUGAR kStmt_GetBlock(kctx, stmt, NULL, KW_else, K_NULLBLOCK);
//}
//
//static kExpr* Stmt_getFirstExpr(KonohaContext *kctx, kStmt *stmt)
//{
//	return SUGAR kStmt_GetExpr(kctx, stmt, KW_ExprPattern, NULL);
//}
//
//static kStmt *kStmt_GetStmt(KonohaContext *kctx, kStmt *stmt, ksymbol_t kw)
//{
//	return (kStmt *) kStmt_GetObject(kctx, stmt, kw, NULL);
//}
//
//static kMethod* CallExpr_getMethod(kExpr *expr)
//{
//	return expr->cons->MethodItems[0];
//}
//
//static int CallExpr_getArgCount(kExpr *expr)
//{
//	return kArray_size(expr->cons) - 2;
//}
//
//static kString* Stmt_getErrorMessage(KonohaContext *kctx, kStmt *stmt)
//{
//	kString* msg = (kString *)kStmt_GetObjectNULL(kctx, stmt, KW_ERR);
//	DBG_ASSERT(IS_String(msg));
//	return msg;
//}
//
///* ------------------------------------------------------------------------ */
//
//typedef struct BasicBlock BasicBlock;
//
//struct BasicBlock {
//	int incoming;
//	int newid;
//	int nextid;
//	int branchid;
//	int codeoffset;
//	int lastoffset;
//	int size;
//	int max;
//};
//
//static BasicBlock *BasicBlock_(KonohaContext *kctx, int id)
//{
//	BasicBlock *bb = NULL;
//	while(id != -1) {
//		bb = (BasicBlock*)(kctx->stack->cwb.bytebuf + id);
//		id = bb->newid;
//	}
//	return bb;
//}
//
//static int BasicBlock_id(KonohaContext *kctx, BasicBlock *bb)
//{
//	while(bb->newid != -1) {
//		bb = BasicBlock_(kctx, bb->newid);
//	}
//	return ((char*)bb) - kctx->stack->cwb.bytebuf;
//}
//
//static BasicBlock* BasicBlock_self(KonohaContext *kctx, BasicBlock *bb)
//{
//	while(bb->newid != -1) {
//		bb = BasicBlock_(kctx, bb->newid);
//	}
//	return bb;
//}
//
//static void DumpVirtualCode(KonohaContext *kctx, VirtualCode *op, size_t n)
//{
//	int i;
//	for(i = 0; i < n; i++) {
//		DBG_P(" %p [%d] %s", op, i, T_opcode(op[i].opcode));
//	}
//}
//
//static void BasicBlock_Dump(KonohaContext *kctx, BasicBlock *bb)
//{
//	size_t n = (bb->size - sizeof(BasicBlock)) / sizeof(VirtualCode);
//	VirtualCode *op = (VirtualCode *)((char*)bb + sizeof(BasicBlock));
//	int id = BasicBlock_id(kctx, bb);
//	DBG_P("BB(%d) newid=%d, nextid=%d, branch=%d, codeoffset=%d, lastoffset=%d", id, bb->newid, bb->nextid, bb->branchid, bb->codeoffset, bb->lastoffset);
//	DumpVirtualCode(kctx, op, n);
//}
//
//static BasicBlock* new_BasicBlock(KonohaContext *kctx, size_t max, BasicBlock *oldbb)
//{
//	KGrowingBuffer wb;
//	KLIB Kwb_Init(&(kctx->stack->cwb), &wb);
//	BasicBlock *bb = (BasicBlock*)KLIB Kwb_Alloca(kctx, &wb, max);
//	if(oldbb != NULL) {
//		memcpy(bb, oldbb, oldbb->size);
//		oldbb->newid = BasicBlock_id(kctx, bb);
//		oldbb->size = 0;
//	}
//	else {
//		bb->size = sizeof(BasicBlock);
//		bb->newid    = -1;
//		bb->nextid   = -1;
//		bb->branchid = -1;
//	}
//	bb->max = max;
//	bb->codeoffset   = -1;
//	bb->lastoffset   = -1;
//	return bb;
//}
//
//static size_t newsize2(size_t max) {
//	return ((max - sizeof(BasicBlock)) * 2) + sizeof(BasicBlock);
//}
//
//static int BasicBlock_Add(KonohaContext *kctx, BasicBlock *bb, kfileline_t uline, VirtualCode *op, size_t size, size_t padding_size)
//{
//	DBG_ASSERT(bb->newid == -1);
//	DBG_ASSERT(size <= padding_size);
//	DBG_ASSERT(bb->nextid == -1 && bb->branchid == -1);
//	if(!(bb->size + size < bb->max)) {
//		size_t newsize = newsize2(bb->max);
//		if(newsize < 1000) newsize = newsize2(newsize);
//		bb = new_BasicBlock(kctx, newsize, bb);
//	}
//	memcpy(((char*)bb) + bb->size, op, size);
//	bb->size += padding_size;
//	return BasicBlock_id(kctx, bb);
//}
//
///* ------------------------------------------------------------------------ */
//
//static int new_BasicBlockLABEL(KonohaContext *kctx)
//{
//	return BasicBlock_id(kctx, new_BasicBlock(kctx, sizeof(VirtualCode) * 2 + sizeof(BasicBlock), NULL));
//}
//
//#if defined(USE_DIRECT_THREADED_CODE)
//#define TADDR   NULL, 0/*counter*/
//#else
//#define TADDR   0/*counter*/
//#endif/*USE_DIRECT_THREADED_CODE*/
//#define ASMLINE  0
//
//#define NC_(sfpidx)       (((sfpidx) * 2) + 1)
//#define OC_(sfpidx)       ((sfpidx) * 2)
//#define TC_(sfpidx, TYPE) ((TY_isUnbox(TYPE)) ? NC_(sfpidx) : OC_(sfpidx))
//#define SFP_(sfpidx)   ((sfpidx) * 2)
//
//#define ASM(T, ...) do {\
//	OP##T op_ = {TADDR, OPCODE_##T, ASMLINE, ## __VA_ARGS__};\
//	union { VirtualCode op; OP##T op_; } tmp_; tmp_.op_ = op_; \
//	BUILD_asm(kctx, &tmp_.op, sizeof(OP##T));\
//} while(0)
//
//static void BUILD_asm(KonohaContext *kctx, VirtualCode *op, size_t opsize)
//{
//	ctxcode->bbCurId = BasicBlock_Add(kctx, BasicBlock_(kctx, ctxcode->bbCurId), ctxcode->uline, op, opsize, sizeof(VirtualCode));
//}
//
//static void kStmt_setLabelBlock(KonohaContext *kctx, kStmt *stmt, ksymbol_t label, int labelId)
//{
//	KLIB kObject_setUnboxValue(kctx, stmt, label, TY_int, labelId);
//}
//
//static int kStmt_GetLabelBlock(KonohaContext *kctx, kStmt *stmt, ksymbol_t label)
//{
//	return KLIB kObject_getUnboxValue(kctx, stmt, label, -1);
//}
//
//static void ASM_LABEL(KonohaContext *kctx, int labelId)
//{
//	BasicBlock *bb = BasicBlock_(kctx, ctxcode->bbCurId);
//	DBG_ASSERT(bb != NULL);
//	DBG_ASSERT(bb->nextid == -1);
//	BasicBlock *labelBlock = BasicBlock_(kctx, labelId);
//	if(bb->branchid == -1) {
//		DBG_ASSERT(bb->nextid == -1);
//		bb->nextid = BasicBlock_id(kctx, labelBlock);
//	}
//	labelBlock->incoming += 1;
//	ctxcode->bbCurId = BasicBlock_id(kctx, labelBlock);
//}
//
//static void ASM_JMP(KonohaContext *kctx, int labelId)
//{
//	BasicBlock *bb = BasicBlock_(kctx, ctxcode->bbCurId);
//	DBG_ASSERT(bb != NULL);
//	DBG_ASSERT(bb->nextid == -1);
//	if(bb->branchid == -1) {
//		ASM(JMP, NULL);
//		BasicBlock *labelBlock = BasicBlock_(kctx, labelId);
//		bb = BasicBlock_(kctx, ctxcode->bbCurId);
//		bb->branchid = BasicBlock_id(kctx, labelBlock);
//		labelBlock->incoming += 1;
//		ctxcode->bbCurId = BasicBlock_id(kctx, bb);
//	}
//}
//
//static int ASM_JMPF(KonohaContext *kctx, int flocal, int jumpId)
//{
//	BasicBlock *bb = BasicBlock_(kctx, ctxcode->bbCurId);
//	DBG_ASSERT(bb != NULL);
//	DBG_ASSERT(bb->nextid == -1 && bb->branchid == -1);
//	int nextId = new_BasicBlockLABEL(kctx);
//	BasicBlock *lbJUMP = BasicBlock_(kctx, jumpId);
//	BasicBlock *lbNEXT = BasicBlock_(kctx, nextId);
//	ASM(JMPF, NULL, NC_(flocal));
//	bb = BasicBlock_(kctx, ctxcode->bbCurId);
//	bb->branchid = BasicBlock_id(kctx, lbJUMP);
//	bb->nextid = nextId;
//	lbNEXT->incoming += 1;
//	lbJUMP->incoming += 1;
//	ctxcode->bbCurId = nextId;
//	return nextId;
//}
//
//static int CodeOffset(KGrowingBuffer *wb)
//{
//	return Kwb_bytesize(wb);
//}
//
//static void BasicBlock_writeBuffer(KonohaContext *kctx, int blockId, KGrowingBuffer *wb)
//{
//	BasicBlock *bb = BasicBlock_(kctx, blockId);
//	while(bb != NULL && bb->codeoffset == -1) {
//		size_t len = bb->size - sizeof(BasicBlock);
//		bb->codeoffset = CodeOffset(wb);
//		if(len > 0) {
//			int id = BasicBlock_id(kctx, bb);
//			char buf[len];  // bb is growing together with wb.
//			memcpy(buf, ((char*)bb) + sizeof(BasicBlock), len);
//			KLIB Kwb_write(kctx, wb, buf, len);
//			bb = BasicBlock_(kctx, id);  // recheck
//			bb->lastoffset = CodeOffset(wb) - sizeof(VirtualCode);
//			DBG_ASSERT(bb->codeoffset + ((len / sizeof(VirtualCode)) - 1) * sizeof(VirtualCode) == bb->lastoffset);
//		}
//		else {
//			DBG_ASSERT(bb->branchid == -1);
//		}
//		bb = BasicBlock_(kctx, bb->nextid);
//	}
//	bb = BasicBlock_(kctx, blockId);
//	while(bb != NULL) {
//		if(bb->branchid != -1 /*&& bb->branchid != ctxcode->bbEndId*/) {
//			BasicBlock *bbJ = BasicBlock_(kctx, bb->branchid);
//			if(bbJ->codeoffset == -1) {
//				BasicBlock_writeBuffer(kctx, bb->branchid, wb);
//			}
//		}
//		bb = BasicBlock_(kctx, bb->nextid);
//	}
//}
//
//static int BasicBlock_size(BasicBlock *bb)
//{
//	return (bb->size - sizeof(BasicBlock)) / sizeof(VirtualCode);
//}
//
//static BasicBlock *BasicBlock_leapJump(KonohaContext *kctx, BasicBlock *bb)
//{
//	while(bb->nextid != -1) {
//		if(BasicBlock_size(bb) != 0) return bb;
//		bb = BasicBlock_(kctx, bb->nextid);
//	}
//	if(bb->nextid == -1 && bb->branchid != -1 && BasicBlock_size(bb) == 1) {
//		return BasicBlock_leapJump(kctx, BasicBlock_(kctx, bb->branchid));
//	}
//	return bb;
//}
//
//#define BasicBlock_isVisited(bb)     (bb->incoming == -1)
//#define BasicBlock_setVisited(bb)    bb->incoming = -1
//
//static void BasicBlock_setJumpAddr(KonohaContext *kctx, BasicBlock *bb, char *vcode)
//{
//	while(bb != NULL) {
//		BasicBlock_setVisited(bb);
//		if(bb->branchid != -1) {
//			BasicBlock *bbJ = BasicBlock_leapJump(kctx, BasicBlock_(kctx, bb->branchid));
//			OPJMP *j = (OPJMP *)(vcode + bb->lastoffset);
//			BasicBlock_Dump(kctx, bbJ);
//			DBG_ASSERT(j->opcode == OPCODE_JMP || j->opcode == OPCODE_JMPF);
//			j->jumppc = (VirtualCode*)(vcode + bbJ->codeoffset);
//			bbJ = BasicBlock_(kctx, bb->branchid);
//			if(!BasicBlock_isVisited(bbJ)) {
//				BasicBlock_setVisited(bbJ);
//				BasicBlock_setJumpAddr(kctx, bbJ, vcode);
//			}
//		}
//		bb = BasicBlock_(kctx, bb->nextid);
//	}
//}
//
//static kObject* BUILD_AddConstPool(KonohaContext *kctx, kObject *o)
//{
//	KLIB kArray_Add(kctx, ctxcode->constPools, o);
//	return o;
//}
//
//static void ASM_SAFEPOINT(KonohaContext *kctx, kfileline_t uline, int espidx)
//{
//	ASM(SAFEPOINT, uline, SFP_(espidx));
//}
//
//static void NMOV_asm(KonohaContext *kctx, int a, ktype_t ty, int b)
//{
//	ASM(NMOV, TC_(a, ty), TC_(b, ty), CT_(ty));
//}
//
//static int KonohaVisitor_asmJMPIF(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr, int isTRUE, int labelId)
//{
//	int a = builder->common.a;
//	VisitExpr(kctx, builder, stmt, expr);
//	if(isTRUE) {
//		ASM(BNOT, NC_(a), NC_(a));
//	}
//	return ASM_JMPF(kctx, a, labelId);
//}
//
//static void KonohaVisitor_visitErrStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
//{
//	ASM(ERROR, stmt->uline, Stmt_getErrorMessage(kctx, stmt));
//}
//
//static void KonohaVisitor_visitExprStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
//{
//	int a = builder->common.a;
//	builder->common.a = builder->common.espidx;
//	VisitExpr(kctx, builder, stmt, Stmt_getFirstExpr(kctx, stmt));
//	builder->common.a = a;
//}
//
//static void KonohaVisitor_visitBlockStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
//{
//	visitBlock(kctx, builder, Stmt_getFirstBlock(kctx, stmt));
//}
//
//static void KonohaVisitor_visitReturnStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
//{
//	kExpr *expr = SUGAR kStmt_GetExpr(kctx, stmt, KW_ExprPattern, NULL);
//	if(expr != NULL && IS_Expr(expr) && expr->ty != TY_void) {
//		int a = builder->common.a;
//		builder->common.a = K_RTNIDX;
//		VisitExpr(kctx, builder, stmt, expr);
//		builder->common.a = a;
//	}
//	ASM_JMP(kctx, ctxcode->bbEndId); // RET
//}
//
//static void KonohaVisitor_visitIfStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
//{
//	int espidx = builder->common.espidx;
//	int a = builder->common.a;
//	int lbELSE = new_BasicBlockLABEL(kctx);
//	int lbEND  = new_BasicBlockLABEL(kctx);
//	/* if */
//	builder->common.a = espidx;
//	KonohaVisitor_asmJMPIF(kctx, builder, stmt, Stmt_getFirstExpr(kctx, stmt), 0/*FALSE*/, lbELSE);
//	builder->common.a = a;
//	/* then */
//	visitBlock(kctx, builder, Stmt_getFirstBlock(kctx, stmt));
//	ASM_JMP(kctx, lbEND);
//	/* else */
//	ASM_LABEL(kctx, lbELSE);
//	visitBlock(kctx, builder, Stmt_getElseBlock(kctx, stmt));
//	/* endif */
//	ASM_LABEL(kctx, lbEND);
//}
//
//static void KonohaVisitor_visitLoopStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
//{
//	int espidx = builder->common.espidx;
//	int a = builder->common.a;
//	int lbCONTINUE = new_BasicBlockLABEL(kctx);
//	int lbENTRY    = new_BasicBlockLABEL(kctx);
//	int lbBREAK    = new_BasicBlockLABEL(kctx);
//	kStmt_setLabelBlock(kctx, stmt, SYM_("continue"), lbCONTINUE);
//	kStmt_setLabelBlock(kctx, stmt, SYM_("break"),    lbBREAK);
//	if(kStmt_Is(RedoLoop, stmt)) {
//		ASM_JMP(kctx, lbENTRY);
//	}
//	ASM_LABEL(kctx, lbCONTINUE);
//	ASM_SAFEPOINT(kctx, stmt->uline, espidx);
//	kBlock *iterBlock = SUGAR kStmt_GetBlock(kctx, stmt, NULL, SYM_("Iterator"), NULL);
//	if(iterBlock != NULL) {
//		visitBlock(kctx, builder, iterBlock);
//		ASM_LABEL(kctx, lbENTRY);
//		builder->common.a = espidx;
//		KonohaVisitor_asmJMPIF(kctx, builder, stmt, Stmt_getFirstExpr(kctx, stmt), 0/*FALSE*/, lbBREAK);
//		builder->common.a = a;
//	}
//	else {
//		builder->common.a = espidx;
//		KonohaVisitor_asmJMPIF(kctx, builder, stmt, Stmt_getFirstExpr(kctx, stmt), 0/*FALSE*/, lbBREAK);
//		builder->common.a = a;
//		ASM_LABEL(kctx, lbENTRY);
//	}
//	visitBlock(kctx, builder, Stmt_getFirstBlock(kctx, stmt));
//	ASM_JMP(kctx, lbCONTINUE);
//	ASM_LABEL(kctx, lbBREAK);
//}
//
//static void KonohaVisitor_visitJumpStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
//{
//	SugarSyntax *syn = stmt->syn;
//	kStmt *jump = kStmt_GetStmt(kctx, stmt, syn->keyword);
//	DBG_ASSERT(jump != NULL && IS_Stmt(jump));
//	int lbJUMP = kStmt_GetLabelBlock(kctx, jump, syn->keyword);
//	ASM_JMP(kctx, lbJUMP);
//}
//
//static void KonohaVisitor_visitTryStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
//{
//	//FIXME
//	//kBlock *catchBlock   = SUGAR kStmt_GetBlock(kctx, stmt, NULL, SYM_("catch"),   K_NULLBLOCK);
//	//kBlock *finallyBlock = SUGAR kStmt_GetBlock(kctx, stmt, NULL, SYM_("finally"), K_NULLBLOCK);
//	//if(catchBlock != K_NULLBLOCK){
//	//}
//	//if(finallyBlock != K_NULLBLOCK){
//	//}
//}
//
//
//static void KonohaVisitor_visitUndefinedStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
//{
//	DBG_P("undefined asm syntax kw='%s'", SYM_t(stmt->syn->keyword));
//}
//
//static void KonohaVisitor_visitConstExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
//{
//	int a = builder->common.a;
//	kObject *v = expr->objectConstValue;
//	DBG_ASSERT(!TY_isUnbox(expr->ty));
//	DBG_ASSERT(Expr_hasObjectConstValue(expr));
//	v = BUILD_AddConstPool(kctx, v);
//	ASM(NSET, OC_(a), (uintptr_t)v, CT_(expr->ty));
//}
//
//static void KonohaVisitor_visitNConstExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
//{
//	int a = builder->common.a;
//	ASM(NSET, NC_(a), expr->unboxConstValue, CT_(expr->ty));
//}
//
//static void KonohaVisitor_visitNewExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
//{
//	int a = builder->common.a;
//	ASM(NEW, OC_(a), expr->index, CT_(expr->ty));
//}
//
//static void KonohaVisitor_visitNullExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
//{
//	int a = builder->common.a;
//	if(TY_isUnbox(expr->ty)) {
//		ASM(NSET, NC_(a), 0, CT_(expr->ty));
//	}
//	else {
//		ASM(NULL, OC_(a), CT_(expr->ty));
//	}
//}
//
//static void KonohaVisitor_visitLocalExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
//{
//	NMOV_asm(kctx, builder->common.a, expr->ty, expr->index);
//}
//
//static void KonohaVisitor_visitBlockExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
//{
//	int a      = builder->common.a;
//	int shift  = builder->common.shift;
//	int espidx = builder->common.espidx;
//	DBG_ASSERT(IS_Block(expr->block));
//	builder->common.shift = builder->common.espidx;
//	visitBlock(kctx, builder, expr->block);
//	builder->common.shift = shift;
//	NMOV_asm(kctx, a, expr->ty, espidx);
//	builder->common.espidx = espidx;
//}
//
//static void KonohaVisitor_visitFieldExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
//{
//	int a = builder->common.a;
//	kshort_t index = (kshort_t)expr->index;
//	kshort_t xindex = (kshort_t)(expr->index >> (sizeof(kshort_t)*8));
//	ASM(NMOVx, TC_(a, expr->ty), OC_(index), xindex, CT_(expr->ty));
//}
//
//static void KonohaVisitor_visitCallExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
//{
//	kMethod *mtd = CallExpr_getMethod(expr);
//	DBG_ASSERT(IS_Method(mtd));
//
//	/*
//	 * [CallExpr] := this.method(arg1, arg2, ...)
//	 * expr->cons = [method, this, arg1, arg2, ...]
//	 **/
//	int i, a = builder->common.a;
//	int s = kMethod_is(Static, mtd) ? 2 : 1;
//	int espidx  = builder->common.espidx;
//	int thisidx = espidx + K_CALLDELTA;
//	int argc = CallExpr_getArgCount(expr);
//	for (i = s; i < argc + 2; i++) {
//		kExpr *exprN = kExpr_at(expr, i);
//		DBG_ASSERT(IS_Expr(exprN));
//		builder->common.a = builder->common.espidx = thisidx + i - 1;
//		VisitExpr(kctx, builder, stmt, exprN);
//	}
//	builder->common.espidx = espidx;
//	builder->common.a = a;
//
//	if(kMethod_is(Final, mtd) || !kMethod_is(Virtual, mtd)) {
//		ASM(NSET, NC_(thisidx-1), (intptr_t)mtd, CT_Method);
//		if(kMethod_is(Virtual, mtd)) {
//			// set namespace to enable method lookups
//			ASM(NSET, OC_(thisidx-2), (intptr_t)Stmt_ns(stmt), CT_NameSpace);
//		}
//	}
//	else {
//		ASM(NSET, OC_(thisidx-2), (intptr_t)Stmt_ns(stmt), CT_NameSpace);
//		ASM(LOOKUP, SFP_(thisidx), Stmt_ns(stmt), mtd);
//	}
//
//	int esp_ = SFP_(espidx + argc + K_CALLDELTA + 1);
//	ASM(CALL, ctxcode->uline, SFP_(thisidx), esp_, KLIB Knull(kctx, CT_(expr->ty)));
//
//	if(mtd->mn == MN_box) {  /* boxed value of unbox value must be shifted to OC */
//		((kExprVar *)expr)->ty = TY_Object;
//	}
//
//	if(a != espidx) {
//		NMOV_asm(kctx, a, expr->ty, espidx);
//	}
//}
//
//static void KonohaVisitor_visitAndExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
//{
//	int a = builder->common.a;
//	int lbTRUE  = new_BasicBlockLABEL(kctx);
//	int lbFALSE = new_BasicBlockLABEL(kctx);
//	/*
//	 * [AndExpr] := (arg0 && arg1 && arg2 && ...)
//	 * expr->cons = [NULL, arg0, arg1, arg2, ...]
//	 **/
//	int i, size = kArray_size(expr->cons);
//	for (i = 1; i < size; i++) {
//		KonohaVisitor_asmJMPIF(kctx, builder, stmt, kExpr_at(expr, i), 0/*FALSE*/, lbFALSE);
//	}
//	ASM(NSET, NC_(a), 1/*TRUE*/, CT_Boolean);
//	ASM_JMP(kctx, lbTRUE);
//	ASM_LABEL(kctx, lbFALSE); // false
//	ASM(NSET, NC_(a), 0/*FALSE*/, CT_Boolean);
//	ASM_LABEL(kctx, lbTRUE);   // TRUE
//}
//
//static void KonohaVisitor_visitOrExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
//{
//	int a = builder->common.a;
//	int lbTRUE  = new_BasicBlockLABEL(kctx);
//	int lbFALSE = new_BasicBlockLABEL(kctx);
//	int i, size = kArray_size(expr->cons);
//	for (i = 1; i < size; i++) {
//		KonohaVisitor_asmJMPIF(kctx, builder, stmt, kExpr_at(expr, i), 1/*TRUE*/, lbTRUE);
//	}
//	ASM(NSET, NC_(a), 0/*FALSE*/, CT_Boolean);
//	ASM_JMP(kctx, lbFALSE);
//	ASM_LABEL(kctx, lbTRUE);
//	ASM(NSET, NC_(a), 1/*TRUE*/, CT_Boolean);
//	ASM_LABEL(kctx, lbFALSE); // false
//}
//
//static void KonohaVisitor_visitLetExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
//{
//	int a = builder->common.a;
//	int shift = builder->common.shift;
//	int espidx = builder->common.espidx;
//
//	/*
//	 * [LetExpr] := lhs = rhs
//	 * expr->cons = [NULL, lhs, rhs]
//	 **/
//
//	kExpr *leftHandExpr = kExpr_at(expr, 1);
//	kExpr *rightHandExpr = kExpr_at(expr, 2);
//	DBG_P("LET (%s) a=%d, shift=%d, espidx=%d", TY_t(expr->ty), a, shift, espidx);
//	if(leftHandExpr->build == TEXPR_LOCAL) {
//		builder->common.a = leftHandExpr->index;
//		VisitExpr(kctx, builder, stmt, rightHandExpr);
//		builder->common.a = a;
//		if(expr->ty != TY_void && a != leftHandExpr->index) {
//			NMOV_asm(kctx, a, leftHandExpr->ty, leftHandExpr->index);
//		}
//	}
//	else if(leftHandExpr->build == TEXPR_STACKTOP) {
//		DBG_P("LET TEXPR_STACKTOP a=%d, leftHandExpr->index=%d, espidx=%d", a, leftHandExpr->index, espidx);
//		builder->common.a = leftHandExpr->index + shift;
//		VisitExpr(kctx, builder, stmt, rightHandExpr);
//		builder->common.a = a;
//		if(expr->ty != TY_void && a != leftHandExpr->index + shift) {
//			NMOV_asm(kctx, a, leftHandExpr->ty, leftHandExpr->index + shift);
//		}
//	}
//	else{
//		assert(leftHandExpr->build == TEXPR_FIELD);
//		builder->common.a = espidx;
//		VisitExpr(kctx, builder, stmt, rightHandExpr);
//		builder->common.a = a;
//		kshort_t index  = (kshort_t)leftHandExpr->index;
//		kshort_t xindex = (kshort_t)(leftHandExpr->index >> (sizeof(kshort_t)*8));
//		KonohaClass *lhsClass = CT_(leftHandExpr->ty);
//		ASM(XNMOV, OC_(index), xindex, TC_(espidx, rightHandExpr->ty), lhsClass);
//		if(expr->ty != TY_void) {
//			ASM(NMOVx, TC_(a, rightHandExpr->ty), OC_(index), xindex, lhsClass);
//		}
//	}
//}
//
//static void KonohaVisitor_visitStackTopExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
//{
//	int shift = builder->common.shift;
//	int a = builder->common.a;
//	int espidx = builder->common.espidx;
//	DBG_ASSERT(expr->index + shift < espidx);
//	NMOV_asm(kctx, a, expr->ty, expr->index + shift);
//}
//
//static void _THCODE(KonohaContext *kctx, VirtualCode *pc, void **codeaddr, size_t codesize);
//static void BUILD_compile(KonohaContext *kctx, kMethod *mtd, int beginBlock, int endBlock);
//
//static void KonohaVisitor_Init(KonohaContext *kctx, struct KBuilder *builder, kMethod *mtd)
//{
//	builder->common.espidx = 0;
//	builder->common.a = 0;
//	builder->common.shift = 0;
//
////	KLIB kMethod_setFunc(kctx, mtd, builder->common.api->GetVirtualMethodFunc());
//
//	DBG_ASSERT(kArray_size(ctxcode->codeList) == 0);
//	int lbINIT  = new_BasicBlockLABEL(kctx);
//	int lbBEGIN = new_BasicBlockLABEL(kctx);
//	ctxcode->bbInitId = lbINIT;
//	ctxcode->bbEndId  = new_BasicBlockLABEL(kctx);
//	ctxcode->bbCurId = lbINIT;
//	ASM(THCODE, 0, _THCODE);
//	ASM(CHKSTACK, 0);
//	ASM_LABEL(kctx, lbBEGIN);
//}
//
//static void KonohaVisitor_Free(KonohaContext *kctx, struct KBuilder *builder, kMethod *mtd)
//{
//	builder->common.shift = 0;
//	ASM_LABEL(kctx, ctxcode->bbEndId);
//	if(mtd->mn == MN_new) {
//		ASM(NMOV, OC_(K_RTNIDX), OC_(0), CT_(mtd->typeId));   // FIXME: Type 'This' must be resolved
//	}
//	ASM(RET);
////	assert(ctxcode->lbEND);/* scan-build: remove warning */
//	BUILD_compile(kctx, mtd, ctxcode->bbInitId, ctxcode->bbEndId);
//	ctxcode->bbInitId = -1;
//	ctxcode->bbEndId  = -1;
//}
//
//static KBuilder *createKonohaVisitor(KBuilder *builder)
//{
//#define DEFINE_BUILDER_API(NAME) builder->common.api.visit##NAME = KonohaVisitor_visit##NAME;
//	VISITOR_LIST(DEFINE_BUILDER_API);
//#undef DEFINE_BUILDER_API
//	builder->common.api.fn_Init = KonohaVisitor_Init;
//	builder->common.api.fn_Free = KonohaVisitor_Free;
//	return builder;
//}
//
//
//#define CT_ByteCodeVar CT_ByteCode
//
//static kByteCode* new_ByteCode(KonohaContext *kctx, int beginBlock, int returnId, kArray *gcstackNULL)
//{
//	KGrowingBuffer wb;
//	KLIB Kwb_Init(&(kctx->stack->cwb), &wb);
//	BasicBlock_writeBuffer(kctx, beginBlock, &wb);
//	BasicBlock_writeBuffer(kctx, returnId, &wb);
//
//	kByteCodeVar *kcode = new_(ByteCodeVar, NULL, gcstackNULL);
//	kcode->codesize = Kwb_bytesize(&wb);
//	DBG_P(">>>>>> codesize=%d", kcode->codesize);
//	DBG_ASSERT(kcode->codesize != 0);
//	kcode->code     = (VirtualCode *)KCalloc_UNTRACE(kcode->codesize, 1);
//	memcpy((void*)kcode->code, KLIB Kwb_top(kctx, &wb, 0), kcode->codesize);
//	BasicBlock_setJumpAddr(kctx, BasicBlock_(kctx, beginBlock), (char*)kcode->code);
//	KLIB Kwb_Free(&wb);
//	return kcode;
//}
//
//static void dumpOPCODE(KonohaContext *kctx, VirtualCode *c, VirtualCode *pc_start)
//{
//	size_t i, size = OPDATA[c->opcode].size;
//	const kushort_t *vmt = OPDATA[c->opcode].types;
//	if(pc_start == NULL) {
//		DUMP_P("[%p:%d]\t%s(%d)", c, c->line, T_opcode(c->opcode), (int)c->opcode);
//	}
//	else {
//		DUMP_P("[L%d:%d]\t%s(%d)", (int)(c - pc_start), c->line, T_opcode(c->opcode), (int)c->opcode);
//	}
//	for (i = 0; i < size; i++) {
//		DUMP_P(" ");
//		switch(vmt[i]) {
//		case VMT_VOID: break;
//		case VMT_ADDR:
//			if(pc_start == NULL) {
//				DUMP_P("%p", c->p[i]);
//			}
//			else {
//				DUMP_P("L%d", (int)((VirtualCode *)c->p[i] - pc_start));
//			}
//			break;
//		case VMT_R:
//			DUMP_P("sfp[%d,r=%d]", (int)c->data[i]/2, (int)c->data[i]);
//			break;
//		case VMT_U:
//			DUMP_P("u%lu", c->data[i]); break;
//		case VMT_I:
//		case VMT_INT:
//			DUMP_P("i%ld", c->data[i]); break;
//		case VMT_F:
//			DUMP_P("function(%p)", c->p[i]); break;
//		case VMT_CID:
//			DUMP_P("CT(%s)", CT_t(c->ct[i])); break;
//		case VMT_CO:
//			DUMP_P("CT(%s)", CT_t(O_ct(c->o[i]))); break;
//		}/*switch*/
//	}
//	DUMP_P("\n");
//}
//
//static void _THCODE(KonohaContext *kctx, VirtualCode *pc, void **codeaddr, size_t codesize)
//{
//#ifdef USE_DIRECT_THREADED_CODE
//	size_t i, n = codesize / sizeof(VirtualCode);
//	for(i = 0; i < n; i++) {
//		pc->codeaddr = codeaddr[pc->opcode];
//		pc++;
//	}
//#endif
//}
//
///* ------------------------------------------------------------------------ */
//
//static void Method_threadCode(KonohaContext *kctx, kMethod *mtd, kByteCode *kcode)
//{
////	kMethodVar *Wmtd = (kMethodVar *)mtd;
////	KLIB kMethod_setFunc(kctx, mtd, PLATAPI GetVirtualMachineMethodFunc());
////	KFieldSet(Wmtd, Wmtd->CodeObject, kcode);
////	OPTHCODE *opTHCODE = (OPTHCODE *)kcode->code;
////	opTHCODE->codesize = kcode->codesize;
////	Wmtd->pc_start = PLATAPI RunVirtualMachine(kctx, kctx->esp + 1, kcode->code);
////	if(verbose_code) {
////		DBG_P("DUMP CODE");
////		VirtualCode *pc = mtd->pc_start;
////		size_t i, n = kcode->codesize / sizeof(VirtualCode);
////		for(i = 1; i < n; i++) {
////			dumpOPCODE(kctx, pc, mtd->pc_start);
////			pc++;
////		}
////	}
//}
//
//static void BUILD_compile(KonohaContext *kctx, kMethod *mtd, int beginBlock, int endBlock)
//{
//	INIT_GCSTACK();
//	kByteCode *kcode = new_ByteCode(kctx, beginBlock, endBlock, _GcStack);
//	Method_threadCode(kctx, mtd, kcode);
//	KLIB kArray_clear(kctx, ctxcode->codeList, 0);
//	RESET_GCSTACK();
//}
//
//static void kMethod_GenCode(KonohaContext *kctx, kMethod *mtd, kBlock *bk, int options)
//{
//	DBG_P("START CODE GENERATION..");
//	if(PLATAPI GenerateCode != NULL){
//		PLATAPI GenerateCode(kctx, mtd, bk, options);
//		return;
//	}
//	INIT_GCSTACK();
//	if(ctxcode == NULL) {
//		kmodcode->header.setupModuleContext(kctx, NULL, 1/*new ctx*/);
//	}
//
//	KBuilder *builder, builderbuf;
//	// TODO: set options
//	builder = createKonohaVisitor(&builderbuf);
//	KGrowingBuffer wb;
//	KLIB Kwb_Init(&(kctx->stack->cwb), &wb);
//	builder->common.api.fn_Init(kctx, builder, mtd);
//	visitBlock(kctx, builder, bk);
//	builder->common.api.fn_Free(kctx, builder, mtd);
//	KLIB Kwb_Free(&wb);
//	RESET_GCSTACK();
//}
//
///* ------------------------------------------------------------------------ */
///* [datatype] */
//
//static KMETHOD MethodFunc_invokeAbstractMethod(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KReturnUnboxValue(0);
//}
//
//static void kMethod_setFunc(KonohaContext *kctx, kMethod *mtd, MethodFunc func)
//{
//	func = (func != NULL) ? func : MethodFunc_invokeAbstractMethod;
//	((kMethodVar *)mtd)->invokeMethodFunc = func;
//	((kMethodVar *)mtd)->pc_start = CODE_NCALL;
//}
//
//static void ByteCode_Init(KonohaContext *kctx, kObject *o, void *conf)
//{
//	kByteCodeVar *b = (kByteCodeVar *)o;
//	b->codesize = 0;
//	b->code = NULL;
//	b->fileid = 0;
//	KFieldInit(b, b->source, TS_EMPTY);
//}
//
//static void ByteCode_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
//{
//	kByteCode *b = (kByteCode *)o;
//	KRefTrace(b->source);
//}
//
//static void ByteCode_Free(KonohaContext *kctx, kObject *o)
//{
//	kByteCode *b = (kByteCode *)o;
//	KFree(b->code, b->codesize);
//}
//
//static void ctxcode_Reftrace(KonohaContext *kctx, struct KonohaModuleContext *baseh, KObjectVisitor *visitor)
//{
//	ctxcode_t *base = (ctxcode_t *)baseh;
//	KRefTrace(base->codeList);
//	KRefTrace(base->constPools);
//}
//
//static void ctxcode_Free(KonohaContext *kctx, struct KonohaModuleContext *baseh)
//{
//	ctxcode_t *base = (ctxcode_t *)baseh;
//	KFree(base, sizeof(ctxcode_t));
//}
//
//static void kmodcode_Setup(KonohaContext *kctx, struct KonohaModule *def, int newctx)
//{
//	if(newctx) { // lazy setup
//		assert(kctx->modlocal[MOD_code] == NULL);
//		ctxcode_t *base = (ctxcode_t *)KCalloc_UNTRACE(sizeof(ctxcode_t), 1);
//		base->h.reftrace = ctxcode_Reftrace;
//		base->h.free     = ctxcode_Free;
//		kctx->modlocal[MOD_code] = (KonohaModuleContext *)base;
//
//		DBG_ASSERT(OnContextConstList != NULL);
//		base->codeList   = new_(Array, K_PAGESIZE/sizeof(void *), OnContextConstList);
//		base->constPools = new_(Array, 64, OnContextConstList);
//	}
//}
//
//void MODCODE_Init(KonohaContext *kctx, KonohaContextVar *ctx)
//{
//	KModuleByteCode *base = (KModuleByteCode *)KCalloc_UNTRACE(sizeof(KModuleByteCode), 1);
//	opcode_Check();
//	base->header.name      = "minivm";
//	base->header.allocSize = sizeof(KModuleByteCode);
//	base->header.setupModuleContext    = kmodcode_Setup;
//	KLIB KonohaRuntime_setModule(kctx, MOD_code, &base->header, 0);
//
////	KDEFINE_CLASS defBasicBlock = {0};
////	SETSTRUCTNAME(defBasicBlock, BasicBlock);
////	defBasicBlock.init = BasicBlock_Init;
////	defBasicBlock.free = BasicBlock_Free;
//
//	KDEFINE_CLASS defByteCode = {0};
//	SETSTRUCTNAME(defByteCode, ByteCode);
//	defByteCode.init = ByteCode_Init;
//	defByteCode.reftrace = ByteCode_Reftrace;
//	defByteCode.free = ByteCode_Free;
//
////	base->cBasicBlock = KLIB KonohaClass_define(kctx, PackageId_sugar, NULL, &defBasicBlock, 0);
//	base->cByteCode = KLIB KonohaClass_define(kctx, PackageId_sugar, NULL, &defByteCode, 0);
//	kmodcode_Setup(kctx, &base->header, 0);
//	KonohaLibVar *l = (KonohaLibVar *)kctx->klib;
//	l->kMethod_GenCode = kMethod_GenCode;
//	l->kMethod_setFunc = kMethod_setFunc;
//}
//
//#ifdef __cplusplus
//}
//#endif
