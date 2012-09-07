
static void DumpVisitor_visitErrStmt(struct IRBuilder *self, kStmt *stmt)
{
	KonohaContext *kctx = self->kctx;
	emit_string(S_text(kStmt_getObjectNULL(kctx, stmt, KW_ERR)), "", "", self->indent);
}

static void DumpVisitor_visitExprStmt(struct IRBuilder *self, kStmt *stmt)
{
	KonohaContext *kctx = self->kctx;
	handleExpr(self, Stmt_getFirstExpr(kctx, stmt));
}

static void DumpVisitor_visitBlockStmt(struct IRBuilder *self, kStmt *stmt)
{
	KonohaContext *kctx = self->kctx;
	visitBlock(self, Stmt_getFirstBlock(kctx, stmt)); 
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
	//KonohaContext *kctx = self->kctx;
	emit_string("Jump", "", "", self->indent);
}

static void DumpVisitor_visitUndefinedStmt(struct IRBuilder *self, kStmt *stmt)
{
	//KonohaContext *kctx = self->kctx;
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
	//KonohaContext *kctx = self->kctx;
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
	//KonohaContext *kctx = self->kctx;
	emit_string("BLOCK", "", "", self->indent);
	visitBlock(self, expr->block);
}

static void DumpVisitor_visitFieldExpr(struct IRBuilder *self, kExpr *expr)
{
	//KonohaContext *kctx = self->kctx;
	emit_string("FIELD", "", "", self->indent);
}

static void DumpVisitor_visitCallExpr(struct IRBuilder *self, kExpr *expr)
{
	KonohaContext *kctx = self->kctx;
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	kMethod *mtd = CallExpr_getMethod(expr);
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
	//KonohaContext *kctx = self->kctx;
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
	//KonohaContext *kctx = self->kctx;
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
