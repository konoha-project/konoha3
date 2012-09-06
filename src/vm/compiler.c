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
#include <stdio.h>
/* ************************************************************************ */

int verbose_code = 0;  // global variable

/*static void EXPR_asm(KonohaContext *kctx, kStmt *stmt, int a, kExpr *expr, int shift, int espidx)
  {
  DBG_ASSERT(expr != NULL);
//DBG_P("a=%d, shift=%d, espidx=%d", a, shift, espidx);
switch(expr->build) {
case TEXPR_CONST :
break;
case TEXPR_NEW   :
break;
case TEXPR_NULL  :
break;
case TEXPR_NCONST :
break;
case TEXPR_LOCAL :
break;
case TEXPR_BLOCK :
break;
case TEXPR_FIELD :
break;
case TEXPR_CALL  :
break;
case TEXPR_AND  :
break;
case TEXPR_OR  :
break;
case TEXPR_LET  :
break;
case TEXPR_STACKTOP  :
break;
default:
DBG_ABORT("unknown expr=%d", expr->build);
}
}
 */
void emit_string(const char *str, const char *prefix, const char *suffix, int indent) {
	int i;
	for (i = 0; i < indent; i++){
		putchar(' ');
	}
	printf("%s%s%s\n", prefix, str, suffix);
}


//IfStmt_getThenBlock();
//IfStmt_getElseBlock();

struct IRBuilder;
typedef void (*VisitStmt_t)(struct IRBuilder *self, kStmt *stmt);
typedef void (*VisitExpr_t)(struct IRBuilder *self, kExpr *expr);
struct IRBuilderBase {
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
};

static void visitBlock(struct IRBuilder *builder, kBlock *bk);
static void handleExpr(struct IRBuilder *builder, kExpr *expr);

struct MiniVMBlock {
	unsigned id;
	char	*bytecode;
	struct MiniVMBlock *nextBlock;
	struct MiniVMBlock *jumpBlock;
};

struct IRBuilder {
	struct IRBuilderBase base;
	struct MiniVMBlock *blocks;
	int indent;
	KonohaContext *kctx;
};

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

static void DumpVisitor_visitErrStmt(struct IRBuilder *self, kStmt *stmt)
{
	KonohaContext *kctx = self->kctx;
	emit_string(S_text(kStmt_getObjectNULL(kctx, stmt, KW_ERR)), "", "", self->indent);
}

static void DumpVisitor_visitExprStmt(struct IRBuilder *self, kStmt *stmt)
{
	KonohaContext *kctx = self->kctx;
}

static void DumpVisitor_visitBlockStmt(struct IRBuilder *self, kStmt *stmt)
{
	KonohaContext *kctx = self->kctx;
}

static void DumpVisitor_visitReturnStmt(struct IRBuilder *self, kStmt *stmt)
{
	KonohaContext *kctx = self->kctx;
	emit_string("Return", "", "", self->indent);
	kExpr* expr = Stmt_getFirstExpr(kctx, stmt);
	if (expr != NULL && IS_Expr(expr)){
		handleExpr(self, expr);
	}
}

static void DumpVisitor_visitIfStmt(struct IRBuilder *self, kStmt *stmt)
{
	KonohaContext *kctx = self->kctx;
	self->indent++;
	emit_string("If", "", "", self->indent - 1);
	handleExpr(self, Stmt_getFirstExpr(kctx, stmt));
	emit_string("Then", "", "", self->indent - 1);
	visitBlock(self, Stmt_getFirstBlock(kctx, stmt)); 
	emit_string("Else", "", "", self->indent - 1);
	visitBlock(self, Stmt_getElseBlock(kctx, stmt)); 
	self->indent--;
}

static void DumpVisitor_visitLoopStmt(struct IRBuilder *self, kStmt *stmt)
{
	KonohaContext *kctx = self->kctx;
	self->indent++;
	emit_string("Loop", "", "", self->indent - 1);
	handleExpr(self, Stmt_getFirstExpr(kctx, stmt));
	emit_string("Body", "", "", self->indent - 1);
	visitBlock(self, Stmt_getFirstBlock(kctx, stmt)); 
	self->indent--;
}

static void DumpVisitor_visitJumpStmt(struct IRBuilder *self, kStmt *stmt)
{
	KonohaContext *kctx = self->kctx;
	emit_string("Jump", "", "", self->indent);
}

static void DumpVisitor_visitUndefinedStmt(struct IRBuilder *self, kStmt *stmt)
{
	KonohaContext *kctx = self->kctx;
}

static void DumpVisitor_visitConstExpr(struct IRBuilder *self, kExpr *expr)
{
	KonohaContext *kctx = self->kctx;
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	KonohaStack sfp[1];
	kObject *obj = expr->objectConstValue;
	sfp[0].o = obj;
	O_ct(obj)->p(kctx, sfp, 0, &wb, 0);
	char  *str = (char *) KLIB Kwb_top(kctx, &wb, 0);
	char buf[128];
	snprintf(buf, 128, "CONST:%s:'%s'", CT_t(O_ct(obj)), str);
	emit_string(buf, "", "", self->indent);
	KLIB Kwb_free(&wb);
}

static void DumpVisitor_visitNConstExpr(struct IRBuilder *self, kExpr *expr)
{
	KonohaContext *kctx = self->kctx;
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	KonohaStack sfp[1];
	unsigned long unboxVal = expr->unboxConstValue;
	KonohaClass *ct = CT_(expr->ty);
	sfp[0].unboxValue = unboxVal;
	ct->p(kctx, sfp, 0, &wb, 0);
	char  *str = (char *) KLIB Kwb_top(kctx, &wb, 0);
	char buf[128];
	snprintf(buf, 128, "NCONST:'%s'", str);
	emit_string(buf, "", "", self->indent);
	KLIB Kwb_free(&wb);
}

static void DumpVisitor_visitNewExpr(struct IRBuilder *self, kExpr *expr)
{
	KonohaContext *kctx = self->kctx;
	emit_string("NEW", "", "", self->indent);
}

static void DumpVisitor_visitNullExpr(struct IRBuilder *self, kExpr *expr)
{
	KonohaContext *kctx = self->kctx;
	char buf[128];
	snprintf(buf, 128, "%s.NULL", CT_t(CT_(expr->ty)));
	emit_string(buf, "", "", self->indent);
}

static void DumpVisitor_visitLocalExpr(struct IRBuilder *self, kExpr *expr)
{
	KonohaContext *kctx = self->kctx;
	char buf[128];
	snprintf(buf, 128, "LOCAL(%d, %s)", (int)expr->index, CT_t(CT_(expr->ty)));
	emit_string(buf, "", "", self->indent);

}

static void DumpVisitor_visitBlockExpr(struct IRBuilder *self, kExpr *expr)
{
	KonohaContext *kctx = self->kctx;
	emit_string("BLOCK", "", "", self->indent);
	visitBlock(self, expr->block);
}

static void DumpVisitor_visitFieldExpr(struct IRBuilder *self, kExpr *expr)
{
	KonohaContext *kctx = self->kctx;
	emit_string("FIELD", "", "", self->indent);
}

static void DumpVisitor_visitCallExpr(struct IRBuilder *self, kExpr *expr)
{
	KonohaContext *kctx = self->kctx;
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	kMethod *mtd = expr->cons->methodItems[0];
	KLIB Kwb_printf(kctx, &wb, "CALL: '%s%s'", T_mn(mtd->mn));
	self->indent++;
	emit_string(KLIB Kwb_top(kctx, &wb, 1), "(", "", self->indent);
	self->indent++;
	unsigned i;
	for(i = 1; i < kArray_size(expr->cons); ++i) {
		handleExpr(self, kExpr_at(expr, i));
	}
	self->indent--;
	emit_string(")", "", "", self->indent);
	self->indent--;
	KLIB Kwb_free(&wb);
}

static void DumpVisitor_visitAndExpr(struct IRBuilder *self, kExpr *expr)
{
	KonohaContext *kctx = self->kctx;
	emit_string("AND", "", "", self->indent);
	self->indent++;
	unsigned i;
	for(i = 1; i < kArray_size(expr->cons); ++i) {
		handleExpr(self, kExpr_at(expr, i));
	}
	self->indent--;
}

static void DumpVisitor_visitOrExpr(struct IRBuilder *self, kExpr *expr)
{
	KonohaContext *kctx = self->kctx;
	emit_string("OR", "", "", self->indent);
	self->indent++;
	unsigned i;
	for(i = 1; i < kArray_size(expr->cons); ++i) {
		handleExpr(self, kExpr_at(expr, i));
	}
	self->indent--;
}

static void DumpVisitor_visitLetExpr(struct IRBuilder *self, kExpr *expr)
{
	self->indent++;
	emit_string("LET ", "", "", self->indent);
	self->indent++;
	handleExpr(self, kExpr_at(expr, 1));
	emit_string(":=", "", "", self->indent);
	handleExpr(self, kExpr_at(expr, 2));
	self->indent--;
	self->indent--;
}

static void DumpVisitor_visitStackTopExpr(struct IRBuilder *self, kExpr *expr)
{
}

struct IRBuilder *createCppVisitor(struct IRBuilder *builder);
struct IRBuilder *createLLVMVisitor(struct IRBuilder *builder);
struct IRBuilder *createJSVisitor(struct IRBuilder *builder);

struct IRBuilder *createDumpVisitor(struct IRBuilder *builder)
{
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

#define DEFINE_BUILDER_API(NAME) builder->base.visit##NAME = DumpVisitor_visit##NAME;
	VISITOR_LIST(DEFINE_BUILDER_API);
#undef DEFINE_BUILDER_API
	builder->indent = 0;
	return builder;
}

static void visitBlock(struct IRBuilder *builder, kBlock *bk)
{
	int i;
	for(i = 0; i < kArray_size(bk->stmtList); i++) {
		kStmt *stmt = bk->stmtList->stmtItems[i];
		if(stmt->syn == NULL) continue;
		//ctxcode->uline = stmt->uline;
		switch(stmt->build) {
			case TSTMT_ERR:	builder->base.visitErrStmt(builder, stmt);       return;
			case TSTMT_EXPR:   builder->base.visitExprStmt(builder, stmt);   break;
			case TSTMT_BLOCK:  builder->base.visitBlockStmt(builder, stmt);  break;
			case TSTMT_RETURN: builder->base.visitReturnStmt(builder, stmt); return;
			case TSTMT_IF:	 builder->base.visitIfStmt(builder, stmt);       break;
			case TSTMT_LOOP:   builder->base.visitLoopStmt(builder, stmt);   break;
			case TSTMT_JUMP:   builder->base.visitJumpStmt(builder, stmt);   break;
			default: builder->base.visitUndefinedStmt(builder, stmt);        break;
		}
	}
}

static void handleExpr(struct IRBuilder *builder, kExpr *expr)
{
	switch(expr->build) {
		case TEXPR_CONST:    builder->base.visitConstExpr(builder, expr);  break;
		case TEXPR_NEW:      builder->base.visitNewExpr(builder, expr);    break;
		case TEXPR_NULL:     builder->base.visitNullExpr(builder, expr);   break;
		case TEXPR_NCONST:   builder->base.visitNConstExpr(builder, expr); break;
		case TEXPR_LOCAL:    builder->base.visitLocalExpr(builder, expr);  break;
		case TEXPR_BLOCK:    builder->base.visitBlockExpr(builder, expr);  break;
		case TEXPR_FIELD:    builder->base.visitFieldExpr(builder, expr);  break;
		case TEXPR_CALL:     builder->base.visitCallExpr(builder, expr);   break;
		case TEXPR_AND:      builder->base.visitAndExpr(builder, expr);    break;
		case TEXPR_OR:       builder->base.visitOrExpr(builder, expr);     break;
		case TEXPR_LET:      builder->base.visitLetExpr(builder, expr);    break;
		case TEXPR_STACKTOP:
		default: builder->base.visitStackTopExpr(builder, expr);    break;
	}
}

static void KonohaBuilder_visitErrStmt(struct IRBuilder *builder, kStmt *stmt)
{
}
static void KonohaBuilder_visitExprStmt(struct IRBuilder *builder, kStmt *stmt)
{
}
static void KonohaBuilder_visitBlockStmt(struct IRBuilder *builder, kStmt *stmt)
{
}
static void KonohaBuilder_visitReturnStmt(struct IRBuilder *builder, kStmt *stmt)
{
}
static void KonohaBuilder_visitIfStmt(struct IRBuilder *builder, kStmt *stmt)
{
}
static void KonohaBuilder_visitLoopStmt(struct IRBuilder *builder, kStmt *stmt)
{
}
static void KonohaBuilder_visitJumpStmt(struct IRBuilder *builder, kStmt *stmt)
{
}
static void KonohaBuilder_visitUndefinedStmt(struct IRBuilder *builder, kStmt *stmt)
{
}
static void KonohaBuilder_visitConstExpr(struct IRBuilder *builder, kExpr *expr)
{
}
static void KonohaBuilder_visitNConstExpr(struct IRBuilder *builder, kExpr *expr)
{
}
static void KonohaBuilder_visitNewExpr(struct IRBuilder *builder, kExpr *expr)
{
}
static void KonohaBuilder_visitNullExpr(struct IRBuilder *builder, kExpr *expr)
{
}
static void KonohaBuilder_visitLocalExpr(struct IRBuilder *builder, kExpr *expr)
{
}
static void KonohaBuilder_visitBlockExpr(struct IRBuilder *builder, kExpr *expr)
{
}
static void KonohaBuilder_visitFieldExpr(struct IRBuilder *builder, kExpr *expr)
{
}
static void KonohaBuilder_visitCallExpr(struct IRBuilder *builder, kExpr *expr)
{
}
static void KonohaBuilder_visitAndExpr(struct IRBuilder *builder, kExpr *expr)
{
}
static void KonohaBuilder_visitOrExpr(struct IRBuilder *builder, kExpr *expr)
{
}
static void KonohaBuilder_visitLetExpr(struct IRBuilder *builder, kExpr *expr)
{
}
static void KonohaBuilder_visitStackTopExpr(struct IRBuilder *builder, kExpr *expr)
{
}


static void kMethod_genCode(KonohaContext *kctx, kMethod *mtd, kBlock *bk)
{
	DBG_P("START CODE GENERATION..");
	INIT_GCSTACK();
	//if(ctxcode == NULL) {
	//	kmodcode->h.setup(kctx, NULL, 0);
	//}
	struct IRBuilder *builder, builderbuf;
	builder = createDumpVisitor(&builderbuf);
	builder->kctx = kctx;
	visitBlock(builder, bk);
	RESET_GCSTACK();
}

/* ------------------------------------------------------------------------ */
/* [datatype] */

//#define PACKSUGAR	.packageId = 1, .packageDomain = 1

static KMETHOD MethodFunc_invokeAbstractMethod(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(0);
}

static void kMethod_setFunc(KonohaContext *kctx, kMethod *mtd, MethodFunc func)
{
	func = (func == NULL) ? MethodFunc_invokeAbstractMethod : func;
	((kMethodVar*)mtd)->invokeMethodFunc = func;
	//((kMethodVar*)mtd)->pc_start = CODE_NCALL;
}

void MODCODE_init(KonohaContext *kctx, KonohaContextVar *ctx)
{
	KonohaLibVar *l = (KonohaLibVar*)kctx->klib;
	l->kMethod_genCode = kMethod_genCode;
	l->kMethod_setFunc = kMethod_setFunc;
}

#ifdef __cplusplus
}
#endif
