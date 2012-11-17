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

//static void syntaxMap_Reftrace(KonohaContext *kctx, KHashMapEntry *p, void *thunk)
//{
//	KObjectVisitor *visitor = (KObjectVisitor *) thunk;
//	SugarSyntax *syn = (SugarSyntax *)p->unboxValue;
//	BEGIN_REFTRACE(6);
//	KREFTRACEn(syn->syntaxPatternListNULL);
//	size_t i;
//	for(i = 0; i < SugarFunc_SIZE; i++) {
//		KREFTRACEn(syn->sugarFuncTable[i]);
//	}
//	END_REFTRACE();
//}
//static void kNameSpace_ReftraceSugarExtension(KonohaContext *kctx, kNameSpace *ns, KObjectVisitor *visitor)
//{
//	if(ns->syntaxMapNN != NULL) {
//		KLIB Kmap_each(kctx, ns->syntaxMapNN, (void *)visitor, syntaxMap_Reftrace);
//	}
//	if(ns->tokenMatrix != NULL) {
//		BEGIN_REFTRACE(KCHAR_MAX);
//		size_t i;
//		kFunc** items = ((kFunc**)ns->tokenMatrix) + KCHAR_MAX;
//		for(i = 0; i < KCHAR_MAX; i++) {
//			KREFTRACEn(items[i]);
//		}
//		END_REFTRACE();
//	}
//}

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
/* Token */

static void kToken_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kTokenVar *tk = (kTokenVar *)o;
	tk->uline     =   0;
	tk->unresolvedTokenType = (ksymbol_t)(intptr_t)conf;
	if(tk->unresolvedTokenType == 0  || SYM_UNMASK(tk->unresolvedTokenType) != tk->unresolvedTokenType) {
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
	KREFTRACEv(tk->text);
}

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
		KLIB Kwb_write(kctx, wb, "[", 1);
		if(kArray_size(a) > 0) {
			KUnsafeFieldSet(values[pos+1].asToken, a->TokenItems[0]);
			kToken_p(kctx, values, pos+1, wb);
		}
		for(i = 1; i < kArray_size(a); i++) {
			KLIB Kwb_write(kctx, wb, " ", 1);
			KUnsafeFieldSet(values[pos+1].asToken, a->TokenItems[i]);
			kToken_p(kctx, values, pos+1, wb);
		}
		KLIB Kwb_write(kctx, wb, "]", 1);
	}
#endif
}

/* --------------- */
/* Expr */

static void Expr_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kExprVar *expr = (kExprVar *)o;
	expr->build    = TEXPR_UNTYPED;
	expr->ty       = TY_var;
	expr->index    = 0;
	KFieldInit(expr, expr->termToken, K_NULLTOKEN);
	expr->syn = (SugarSyntax *)conf;
}

static void Expr_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kExpr *expr = (kExpr *)o;
	BEGIN_REFTRACE(2);
	KREFTRACEv(expr->termToken);
	if(Expr_hasObjectConstValue(expr)) {
		KREFTRACEv(expr->objectConstValue);
	}
	END_REFTRACE();
}


static kExpr* new_UntypedTermExpr(KonohaContext *kctx, kToken *tk)
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

static kExpr* new_UntypedCallStyleExpr(KonohaContext *kctx, SugarSyntax *syn, int n, ...)
{
	va_list ap;
	va_start(ap, n);
	DBG_ASSERT(syn != NULL);
	kExprVar *expr = new_(ExprVar, syn, OnGcStack);
	expr = kExpr_AddSeveral(kctx, expr, n, ap);
	va_end(ap);
	return (kExpr *)expr;
}

static kExpr* new_TypedConsExpr(KonohaContext *kctx, int build, ktype_t ty, int n, ...)
{
	kExprVar *expr = new_(ExprVar, NULL, OnGcStack);
	va_list ap;
	va_start(ap, n);
	expr = kExpr_AddSeveral(kctx, expr, n, ap);
	va_end(ap);
	expr->build = build;
	expr->ty = ty;
	return (kExpr *)expr;
}

static kExpr *kStmtkExpr_TypeCheckCallParam(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kMethod *mtd, kGamma *gma, ktype_t reqty);

static kExpr* new_TypedCallExpr(KonohaContext *kctx, kStmt *stmt, kGamma *gma, ktype_t ty, kMethod *mtd, int n, ...)
{
	va_list ap;
	va_start(ap, n);
	kExprVar *expr = new_(ExprVar, NULL, OnGcStack);
	KFieldSet(expr, expr->cons, new_(Array, 8, OnField));
	KLIB kArray_Add(kctx, expr->cons, mtd);
	expr = kExpr_AddSeveral(kctx, expr, n, ap);
	va_end(ap);
	expr->build = TEXPR_CALL;
	expr->ty = ty;
	return kStmtkExpr_TypeCheckCallParam(kctx, stmt, (kExpr *)expr, mtd, gma, ty);
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

static kExpr* SUGAR kExpr_SetConstValue(KonohaContext *kctx, kExpr *expr, ktype_t ty, kObject *o)
{
	kExprVar *Wexpr = (expr == NULL) ? new_(ExprVar, 0, OnGcStack) : (kExprVar *)expr;
	Wexpr->ty = ty;
	if(TY_isUnbox(ty)) {
		Wexpr->build = TEXPR_NCONST;
		Wexpr->unboxConstValue = N_toint(o);
	}
	else {
		Wexpr->build = TEXPR_CONST;
		KFieldInit(Wexpr, Wexpr->objectConstValue, o);
		Expr_setObjectConstValue(Wexpr, 1);
	}
	return (kExpr *)Wexpr;
}

static kExpr* SUGAR kExpr_SetUnboxConstValue(KonohaContext *kctx, kExpr *expr, ktype_t ty, uintptr_t unboxValue)
{
	kExprVar *Wexpr = (expr == NULL) ? new_(ExprVar, 0, OnGcStack) : (kExprVar *)expr;
	Wexpr->build = TEXPR_NCONST;
	Wexpr->unboxConstValue = unboxValue;
	Wexpr->ty = ty;
	return (kExpr *)Wexpr;
}

static kExpr* SUGAR kExpr_SetVariable(KonohaContext *kctx, kExpr *expr, kGamma *gma, kexpr_t build, ktype_t ty, intptr_t index)
{
	kExprVar *Wexpr = (expr == NULL) ? new_(ExprVar, 0, OnGcStack) : (kExprVar *)expr;
	Wexpr->build = build;
	Wexpr->ty = ty;
	Wexpr->index = index;
	if(build == TEXPR_LOCAL && gma->genv->blockScopeShiftSize > 0 && index >= gma->genv->blockScopeShiftSize) {
		Wexpr->build = TEXPR_STACKTOP;
		Wexpr->index -= gma->genv->blockScopeShiftSize;
	}
	return (kExpr *)Wexpr;
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
	KREFTRACEn(stmt->parentBlockNULL);
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
	KLIB Kwb_write(kctx, wb, "}", 1);
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
	BEGIN_REFTRACE(4);
	KREFTRACEv(bk->BlockNameSpace);
	KREFTRACEv(bk->StmtList);
	KREFTRACEv(bk->esp);
	KREFTRACEn(bk->parentStmtNULL);
	END_REFTRACE();
}

static void kBlock_InsertAfter(KonohaContext *kctx, kBlock *bk, kStmtNULL *target, kStmt *stmt)
{
	KFieldSet(stmt, ((kStmtVar *)stmt)->parentBlockNULL, bk);
	if(target != NULL) {
		size_t i;
		for(i = 0; i < kArray_size(bk->StmtList); i++) {
			if(bk->StmtList->StmtItems[i] == target) {
				KLIB kArray_insert(kctx, bk->StmtList, i+1, stmt);
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

