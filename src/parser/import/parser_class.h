/****************************************************************************
 * Copyright (c) 2012, the Konoha project authors. All rights reserved.
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


#define PACKSUGAR    .packageId = 1, .packageDomain = 1

/* --------------- */
/* NameSpace */

static void syntaxMap_Free(KonohaContext *kctx, void *p)
{
	KFree(p, sizeof(SugarSyntax));
}

static void kNameSpace_FreeSugarExtension(KonohaContext *kctx, kNameSpaceVar *ns)
{
	if(ns->syntaxMapNN != NULL) {
		KLIB Kmap_Free(kctx, ns->syntaxMapNN, syntaxMap_Free);
	}
	if(ns->tokenMatrix != NULL) {
		KFree((void *)ns->tokenMatrix, SIZEOF_TOKENMATRIX);
	}
}


/* --------------- */
/* Symbol */

static void kSymbol_p(KonohaContext *kctx, KonohaValue *v, int pos, KGrowingBuffer *wb)
{
	ksymbol_t symbol = (ksymbol_t)v[pos].unboxValue;
	KLIB Kwb_printf(kctx, wb, "%s%s", PSYM_t(symbol));
}

/* --------------- */
/* Token */

static void kToken_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kTokenVar *tk = (kTokenVar *)o;
	tk->uline     =   0;
	tk->unresolvedTokenType = (ksymbol_t)(intptr_t)conf;
	if(tk->unresolvedTokenType == 0  || Symbol_Unmask(tk->unresolvedTokenType) != tk->unresolvedTokenType) {
		KUnsafeFieldInit(tk->text, TS_EMPTY);
	}
	else {
		KUnsafeFieldInit(tk->text, SYM_s(tk->unresolvedTokenType));
	}
	tk->resolvedSyntaxInfo = NULL;
}

static void kToken_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kToken *tk = (kToken *)o;
	KRefTrace(tk->text);
}

#define KToken_t(tk) kToken_t(kctx, tk)

static const char *kToken_t(KonohaContext *kctx, kToken *tk)
{
	if(IS_String(tk->text)) {
		if(tk->unresolvedTokenType == TokenType_CODE) {
			return "{... }";
		}
		return S_text(tk->text);
	}
	else {
		switch(tk->resolvedSymbol) {
			case TokenType_CODE:
			case KW_BraceGroup: return "{... }";
			case KW_ParenthesisGroup: return "(... )";
			case KW_BracketGroup: return "[... ]";
		}
		return "";
	}
}

#ifndef USE_SMALLBUILD
static void Kwb_WriteTokenSymbol(KonohaContext *kctx, KGrowingBuffer *wb, kToken *tk)
{
	if(tk->resolvedSymbol == TokenType_INDENT) {
		KLIB Kwb_printf(kctx, wb, "$Indent ");
	}
	else {
		ksymbol_t symbolType = tk->resolvedSyntaxInfo == NULL ? tk->resolvedSymbol : tk->resolvedSyntaxInfo->keyword;
		KLIB Kwb_printf(kctx, wb, "%s%s ", PSYM_t(symbolType));
	}
}

static void Kwb_WriteTokenText(KonohaContext *kctx, KGrowingBuffer *wb, kToken *tk)
{
	const char *text = IS_String(tk->text) ? S_text(tk->text) : "...";
	char c = kToken_GetOpenHintChar(tk);
	if(c != 0) {
		KLIB Kwb_printf(kctx, wb, "%c%s%c", c, text, kToken_GetCloseHintChar(tk));
	}
	else {
		KLIB Kwb_printf(kctx, wb, "%s", text);
	}
}
#endif/*USE_SMALLBUILD*/

static void kToken_p(KonohaContext *kctx, KonohaValue *values, int pos, KGrowingBuffer *wb)
{
#ifndef USE_SMALLBUILD
	kToken *tk = values[pos].asToken;
	Kwb_WriteTokenSymbol(kctx, wb, tk);
	if(IS_String(tk->text)) {
		KLIB Kwb_printf(kctx, wb, "'%s'", S_text(tk->text));
	}
	else if(IS_Array(tk->subTokenList)) {
		size_t i;
		kArray *a = tk->subTokenList;
		KLIB Kwb_Write(kctx, wb, "[", 1);
		if(kArray_size(a) > 0) {
			KUnsafeFieldSet(values[pos+1].asToken, a->TokenItems[0]);
			kToken_p(kctx, values, pos+1, wb);
		}
		for(i = 1; i < kArray_size(a); i++) {
			KLIB Kwb_Write(kctx, wb, " ", 1);
			KUnsafeFieldSet(values[pos+1].asToken, a->TokenItems[i]);
			kToken_p(kctx, values, pos+1, wb);
		}
		KLIB Kwb_Write(kctx, wb, "]", 1);
	}
#endif
}

/* --------------- */
/* Expr */

static void kExpr_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kExprVar *expr = (kExprVar *)o;
	expr->build    = TEXPR_UNTYPED;
	expr->attrTypeId       = TY_var;
	expr->index    = 0;
	KFieldInit(expr, expr->termToken, K_NULLTOKEN);
	expr->syn = (SugarSyntax *)conf;
}

static void kExpr_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kExpr *expr = (kExpr *)o;
	KRefTrace(expr->termToken);
	if(Expr_hasObjectConstValue(expr)) {
		KRefTrace(expr->objectConstValue);
	}
}

#ifndef USE_SMALLBUILD
static void kExprTerm_p(KonohaContext *kctx, kObject *o, KonohaValue *values, int pos, KGrowingBuffer *wb)
{
	if(IS_Token(o)) {
		Kwb_WriteTokenText(kctx, wb, (kToken *)o);
	}
	else {
		KUnsafeFieldSet(values[pos].asObject, o);
		O_ct(o)->p(kctx, values, pos, wb);
	}
}
#endif

static void kExpr_p(KonohaContext *kctx, KonohaValue *values, int pos, KGrowingBuffer *wb)
{
#ifndef USE_SMALLBUILD
	kExpr *expr = values[pos].asExpr;
	KLIB Kwb_Write(kctx, wb, "(", 1);
	if(expr->build == TEXPR_CONST) {
		KLIB Kwb_Write(kctx, wb, TEXTSIZE("const "));
		kExprTerm_p(kctx, (kObject *)expr->objectConstValue, values, pos+1, wb);
	}
	else if(expr->build == TEXPR_NEW) {
		KLIB Kwb_printf(kctx, wb, "new %s", TY_t(expr->attrTypeId));
	}
	else if(expr->build == TEXPR_NULL) {
		KLIB Kwb_Write(kctx, wb, TEXTSIZE("null"));
	}
	else if(expr->build == TEXPR_NCONST) {
		KLIB Kwb_Write(kctx, wb, TEXTSIZE("const "));
		values[pos+1].unboxValue = expr->unboxConstValue;
		CT_(expr->attrTypeId)->p(kctx, values, pos+1, wb);
	}
	else if(expr->build == TEXPR_LOCAL) {
		KLIB Kwb_printf(kctx, wb, "local sfp[%d]", (int)expr->index);
	}
	else if(expr->build == TEXPR_BLOCK) {
		KLIB Kwb_printf(kctx, wb, "block %d", expr->index);
	}
	else if(expr->build == TEXPR_FIELD) {
		kshort_t index  = (kshort_t)expr->index;
		kshort_t xindex = (kshort_t)(expr->index >> (sizeof(kshort_t)*8));
		KLIB Kwb_printf(kctx, wb, "field sfp[%d][%d]", (int)index, (int)xindex);
	}
	else if(expr->build == TEXPR_STACKTOP) {
		KLIB Kwb_printf(kctx, wb, "stack %d", expr->index);
	}
	else if(Expr_isTerm(expr)) {
		KLIB Kwb_Write(kctx, wb, TEXTSIZE("term "));
		kExprTerm_p(kctx, (kObject *)expr->termToken, values, pos+1, wb);
	}
	else if(IS_Array(expr->cons)) {
		size_t i;
		for(i = 0; i < kArray_size(expr->cons); i++) {
			if(i > 0) {
				KLIB Kwb_Write(kctx, wb, " ", 1);
			}
			kExprTerm_p(kctx, expr->cons->ObjectItems[i], values, pos+1, wb);
		}
	}
	KLIB Kwb_Write(kctx, wb, ")", 1);
	if(expr->attrTypeId != TY_var) {
		KLIB Kwb_printf(kctx, wb, ":%s", TY_t(expr->attrTypeId));
	}
#endif
}

static kExpr* new_TermExpr(KonohaContext *kctx, kToken *tk)
{
	kExprVar *expr = new_(ExprVar, tk->resolvedSyntaxInfo, OnGcStack);
	KFieldSet(expr, expr->termToken, tk);
	Expr_setTerm(expr, 1);
	return (kExpr *)expr;
}

static kExprVar* kExpr_AddSeveral(KonohaContext *kctx, kExprVar *expr, int n, va_list ap)
{
	int i;
	if(!IS_Array(expr->cons)) {
		KFieldSet(expr, expr->cons, new_(Array, 8, OnField));
	}
	for(i = 0; i < n; i++) {
		kObject *v =  (kObject *)va_arg(ap, kObject *);
		if(v == NULL || v == (kObject *)K_NULLEXPR) {
			return (kExprVar *)K_NULLEXPR;
		}
		KLIB kArray_Add(kctx, expr->cons, v);
	}
	return expr;
}

static kExprVar* new_UntypedCallStyleExpr(KonohaContext *kctx, SugarSyntax *syn, int n, ...)
{
	va_list ap;
	va_start(ap, n);
	DBG_ASSERT(syn != NULL);
	kExprVar *expr = new_(ExprVar, syn, OnGcStack);
	expr = kExpr_AddSeveral(kctx, expr, n, ap);
	va_end(ap);
	return expr;
}

static kExpr* new_TypedConsExpr(KonohaContext *kctx, int build, KonohaClass *ty, int n, ...)
{
	kExprVar *expr = new_(ExprVar, NULL, OnGcStack);
	va_list ap;
	va_start(ap, n);
	expr = kExpr_AddSeveral(kctx, expr, n, ap);
	va_end(ap);
	expr->build = build;
	expr->attrTypeId = ty->typeId;
	return (kExpr *)expr;
}

static kExpr *kStmtkExpr_TypeCheckCallParam(KonohaContext *kctx, kStmt *stmt, kExprVar *expr, kMethod *mtd, kGamma *gma, KonohaClass *reqClass);

static kExpr* new_TypedCallExpr(KonohaContext *kctx, kStmt *stmt, kGamma *gma, KonohaClass *reqClass, kMethod *mtd, int n, ...)
{
	va_list ap;
	va_start(ap, n);
	kExprVar *expr = new_(ExprVar, NULL, OnGcStack);
	KFieldSet(expr, expr->cons, new_(Array, 8, OnField));
	KLIB kArray_Add(kctx, expr->cons, mtd);
	expr = kExpr_AddSeveral(kctx, expr, n, ap);
	va_end(ap);
	expr->build = TEXPR_CALL;
	//expr->attrTypeId = reqClass->typeId;
	return kStmtkExpr_TypeCheckCallParam(kctx, stmt, expr, mtd, gma, reqClass);
}

static kExpr* kExpr_Add(KonohaContext *kctx, kExpr *expr, kExpr *e)
{
	DBG_ASSERT(IS_Array(expr->cons));
	if(expr != K_NULLEXPR && e != NULL && e != K_NULLEXPR) {
		KLIB kArray_Add(kctx, expr->cons, e);
		return expr;
	}
	return K_NULLEXPR;
}

static kExpr* SUGAR kExpr_SetConstValue(KonohaContext *kctx, kExprVar *expr, KonohaClass *typedClass, kObject *o)
{
	expr = (expr == NULL) ? new_(ExprVar, 0, OnGcStack) : expr;
	if(typedClass == NULL) typedClass = O_ct(o);
	expr->attrTypeId = typedClass->typeId;
	if(CT_Is(UnboxType, typedClass)) {
		expr->build = TEXPR_NCONST;
		expr->unboxConstValue = N_toint(o);
	}
	else {
		expr->build = TEXPR_CONST;
		KFieldInit(expr, expr->objectConstValue, o);
		Expr_setObjectConstValue(expr, 1);
	}
	return (kExpr *)expr;
}

static kExpr* SUGAR kExpr_SetUnboxConstValue(KonohaContext *kctx, kExprVar *expr, ktype_t attrTypeId, uintptr_t unboxValue)
{
	expr = (expr == NULL) ? new_(ExprVar, 0, OnGcStack) : (kExprVar *)expr;
	expr->build = TEXPR_NCONST;
	expr->unboxConstValue = unboxValue;
	expr->attrTypeId = attrTypeId;
	return (kExpr *)expr;
}

static kExpr* SUGAR kExpr_SetVariable(KonohaContext *kctx, kExprVar *expr, kGamma *gma, kexpr_t build, ktype_t attrTypeId, intptr_t index)
{
	expr = (expr == NULL) ? new_(ExprVar, 0, OnGcStack) : (kExprVar *)expr;
	expr->build = build;
	expr->attrTypeId = attrTypeId;
	expr->index = index;
	if(build == TEXPR_LOCAL && gma->genv->blockScopeShiftSize > 0 && index >= gma->genv->blockScopeShiftSize) {
		expr->build = TEXPR_STACKTOP;
		expr->index -= gma->genv->blockScopeShiftSize;
	}
	return (kExpr *)expr;
}

/* --------------- */
/* Stmt */

static void kStmt_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kStmtVar *stmt = (kStmtVar *)o;
	stmt->uline    = (kfileline_t)conf;
	stmt->syn      = NULL;
	stmt->build    = 0;

	stmt->parentBlockNULL = NULL;
	stmt->build    = 0;
}

static void kStmt_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kStmt *stmt = (kStmt *)o;
	KRefTraceNullable(stmt->parentBlockNULL);
}

static void kStmt_p(KonohaContext *kctx, KonohaValue *values, int pos, KGrowingBuffer *wb)
{
	kStmt *stmt = values[pos].asStmt;
	if(stmt->syn == NULL) {
		KLIB Kwb_printf(kctx, wb, "DONE {uline: %d, ", (kshort_t)stmt->uline);
	}
	else {
		KLIB Kwb_printf(kctx, wb, "%s%s {uline: %d, ", PSYM_t(stmt->syn->keyword), (kshort_t)stmt->uline);
	}
	KLIB kObjectProto_p(kctx, values, pos, wb, 0);
	KLIB Kwb_Write(kctx, wb, "}", 1);
}

static kStmtVar* new_kStmt(KonohaContext *kctx, kArray *gcstack, SugarSyntax *syn, ...)
{
	kStmtVar *stmt = new_(StmtVar, 0, gcstack);
	stmt->syn = syn;
	va_list ap;
	va_start(ap, syn);
	ksymbol_t kw = (ksymbol_t) va_arg(ap, int); /* 'ksymbol_t' is promoted to 'int' through to 'va_arg' */
	while(kw != 0) {
		kObject *v = va_arg(ap, kObject *);
		if(v == NULL) break;
		kStmt_setObject(kctx, stmt, kw, v);
		kw = (ksymbol_t) va_arg(ap, int);
	}
	va_end(ap);
	return stmt;
}

static uintptr_t kStmt_ParseFlag(KonohaContext *kctx, kStmt *stmt, KonohaFlagSymbolData *flagData, uintptr_t flag)
{
	while(flagData->flag != 0) {
		kObject *op = kStmt_GetObjectNULL(kctx, stmt, flagData->symbol);
		if(op != NULL) {
			flag |= flagData->flag;
		}
		flagData++;
	}
	return flag;
}

static kToken* kStmt_GetToken(KonohaContext *kctx, kStmt *stmt, ksymbol_t kw, kToken *def)
{
	kToken *tk = (kToken *)kStmt_GetObjectNULL(kctx, stmt, kw);
	if(tk != NULL && IS_Token(tk)) {
		return tk;
	}
	return def;
}

static kExpr* kStmt_GetExpr(KonohaContext *kctx, kStmt *stmt, ksymbol_t kw, kExpr *def)
{
	kExpr *expr = (kExpr *)kStmt_GetObjectNULL(kctx, stmt, kw);
	if(expr != NULL && IS_Expr(expr)) {
		return expr;
	}
	return def;
}

static const char* kStmt_GetText(KonohaContext *kctx, kStmt *stmt, ksymbol_t kw, const char *def)
{
	kExpr *expr = (kExpr *)kStmt_GetObjectNULL(kctx, stmt, kw);
	if(expr != NULL) {
		if(IS_Expr(expr) && Expr_isTerm(expr)) {
			return S_text(expr->termToken->text);
		}
		else if(IS_Token(expr)) {
			kToken *tk = (kToken *)expr;
			if(IS_String(tk->text)) return S_text(tk->text);
		}
	}
	return def;
}

/* --------------- */
/* Block */

static void kBlock_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kBlockVar *bk = (kBlockVar *)o;
	kNameSpace *ns = (conf != NULL) ? (kNameSpace *)conf : KNULL(NameSpace);
	bk->parentStmtNULL = NULL;
	KFieldInit(bk, bk->BlockNameSpace, ns);
	KFieldInit(bk, bk->StmtList, new_(StmtArray, 0, OnField));
	KFieldInit(bk, bk->esp, new_(Expr, 0, OnField));
}

static void kBlock_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kBlock *bk = (kBlock *)o;
	KRefTrace(bk->BlockNameSpace);
	KRefTrace(bk->StmtList);
	KRefTrace(bk->esp);
	KRefTraceNullable(bk->parentStmtNULL);
}

static void kBlock_InsertAfter(KonohaContext *kctx, kBlock *bk, kStmtNULL *target, kStmt *stmt)
{
	KFieldSet(stmt, ((kStmtVar *)stmt)->parentBlockNULL, bk);
	if(target != NULL) {
		size_t i;
		for(i = 0; i < kArray_size(bk->StmtList); i++) {
			if(bk->StmtList->StmtItems[i] == target) {
				KLIB kArray_Insert(kctx, bk->StmtList, i+1, stmt);
				return;
			}
		}
		DBG_ABORT("target was not found!!");
	}
	else {
		KLIB kArray_Add(kctx, bk->StmtList, stmt);
	}
}

/* --------------- */
/* Block */

static void Gamma_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kGammaVar *gma = (kGammaVar *)o;
	gma->genv = NULL;
}

