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

static void NameSpace_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kNameSpaceVar *ns = (kNameSpaceVar*)o;
	bzero(&ns->parentNULL, sizeof(kNameSpace) - sizeof(KonohaObjectHeader));
	ns->parentNULL = conf;
	KINITv(ns->methodList, K_EMPTYARRAY);
	KINITv(ns->scriptObject, KLIB Knull(kctx, CT_System));
}

static void syntax_reftrace(KonohaContext *kctx, KUtilsHashMapEntry *p)
{
	SugarSyntax *syn = (SugarSyntax*)p->unboxValue;
	BEGIN_REFTRACE(6);
	KREFTRACEn(syn->syntaxRuleNULL);
	KREFTRACEv(syn->PatternMatch);
	KREFTRACEv(syn->ParseExpr);
	KREFTRACEv(syn->TopStmtTyCheck);
	KREFTRACEv(syn->StmtTyCheck);
	KREFTRACEv(syn->ExprTyCheck);
	END_REFTRACE();
}

static void NameSpace_reftrace(KonohaContext *kctx, kObject *o)
{
	kNameSpace *ns = (kNameSpace*)o;
	if(ns->syntaxMapNN != NULL) {
		KLIB Kmap_reftrace(kctx, ns->syntaxMapNN, syntax_reftrace);
	}
	size_t i, size = ns->constTable.bytesize / sizeof(KUtilsKeyValue);
	BEGIN_REFTRACE(size+3);
	for(i = 0; i < size; i++) {
		if(SYMKEY_isBOXED(ns->constTable.keyvalueItems[i].key)) {
			KREFTRACEv(ns->constTable.keyvalueItems[i].objectValue);
		}
	}
	KREFTRACEn(ns->parentNULL);
	KREFTRACEv(ns->scriptObject);
	KREFTRACEv(ns->methodList);
	END_REFTRACE();
}

static void syntaxMap_free(KonohaContext *kctx, void *p)
{
	KFREE(p, sizeof(SugarSyntax));
}

static void NameSpace_free(KonohaContext *kctx, kObject *o)
{
	kNameSpaceVar *ns = (kNameSpaceVar*)o;
	if(ns->syntaxMapNN != NULL) {
		KLIB Kmap_free(kctx, ns->syntaxMapNN, syntaxMap_free);
	}
	if(ns->tokenMatrix != NULL) {
		KFREE((void*)ns->tokenMatrix, SIZEOF_TOKENMATRIX);
	}
	KLIB Karray_free(kctx, &ns->constTable);
}

/* --------------- */
/* Token */

static void Token_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kTokenVar *tk = (kTokenVar*)o;
	tk->uline     =   0;
	tk->keyword        =   (ksymbol_t)(intptr_t)conf;
	KINITv(tk->text, TS_EMPTY);
}

static void Token_reftrace(KonohaContext *kctx, kObject *o)
{
	kToken *tk = (kToken*)o;
	BEGIN_REFTRACE(1);
	KREFTRACEv(tk->text);
	END_REFTRACE();
}


/* --------------- */
/* Expr */

static void Expr_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kExprVar *expr      =   (kExprVar*)o;
	expr->build      =   TEXPR_UNTYPED;
	expr->ty         =   TY_var;
	KINITv(expr->termToken, K_NULLTOKEN);
	expr->syn = (SugarSyntax*)conf;
}

static void Expr_reftrace(KonohaContext *kctx, kObject *o)
{
	kExpr *expr = (kExpr*)o;
	BEGIN_REFTRACE(2);
	KREFTRACEv(expr->termToken);
	if(Expr_hasObjectConstValue(expr)) {
		KREFTRACEv(expr->objectConstValue);
	}
	END_REFTRACE();
}

static kExprVar* Expr_vadd(KonohaContext *kctx, kExprVar *expr, int n, va_list ap)
{
	int i;
	if(!IS_Array(expr->cons)) {
		KSETv(expr->cons, new_(Array, 8));
	}
	for(i = 0; i < n; i++) {
		kObject *v =  (kObject*)va_arg(ap, kObject*);
		if(v == NULL || v == (kObject*)K_NULLEXPR) {
			return (kExprVar*)K_NULLEXPR;
		}
		KLIB kArray_add(kctx, expr->cons, v);
	}
	return expr;
}

static kExpr* new_ConsExpr(KonohaContext *kctx, SugarSyntax *syn, int n, ...)
{
	va_list ap;
	va_start(ap, n);
	DBG_ASSERT(syn != NULL);
	kExprVar *expr = GCSAFE_new(ExprVar, syn);
	expr = Expr_vadd(kctx, expr, n, ap);
	va_end(ap);
	return (kExpr*)expr;
}

static kExpr* new_TypedConsExpr(KonohaContext *kctx, int build, ktype_t ty, int n, ...)
{
	va_list ap;
	va_start(ap, n);
	kExprVar *expr = GCSAFE_new(ExprVar, NULL);
	expr = Expr_vadd(kctx, expr, n, ap);
	va_end(ap);
	expr->build = build;
	expr->ty = ty;
	return (kExpr*)expr;
}

static kExpr *kStmt_tyCheckCallParamExpr(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kMethod *mtd, kGamma *gma, ktype_t reqty);

static kExpr* new_TypedMethodCall(KonohaContext *kctx, kStmt *stmt, ktype_t ty, kMethod *mtd, kGamma *gma, int n, ...)
{
	va_list ap;
	va_start(ap, n);
	kExprVar *expr = GCSAFE_new(ExprVar, NULL);
	KSETv(expr->cons, new_(Array, 8));
	KLIB kArray_add(kctx, expr->cons, mtd);
	expr = Expr_vadd(kctx, expr, n, ap);
	va_end(ap);
	expr->build = TEXPR_CALL;
	expr->ty = ty;
	return kStmt_tyCheckCallParamExpr(kctx, stmt, (kExpr*)expr, mtd, gma, ty);
}


static kExpr* Expr_add(KonohaContext *kctx, kExpr *expr, kExpr *e)
{
	DBG_ASSERT(IS_Array(expr->cons));
	if(expr != K_NULLEXPR && e != NULL && e != K_NULLEXPR) {
		KLIB kArray_add(kctx, expr->cons, e);
		return expr;
	}
	return K_NULLEXPR;
}

static kExpr* SUGAR kExpr_setConstValue(KonohaContext *kctx, kExpr *expr, ktype_t ty, kObject *o)
{
	kExprVar *Wexpr = (expr == NULL) ? GCSAFE_new(ExprVar, 0) : (kExprVar*)expr;
	Wexpr->ty = ty;
	if(TY_isUnbox(ty)) {
		Wexpr->build = TEXPR_NCONST;
		Wexpr->unboxConstValue = N_toint(o);
	}
	else {
		Wexpr->build = TEXPR_CONST;
		KINITv(Wexpr->objectConstValue, o);
		Expr_setObjectConstValue(Wexpr, 1);
	}
	return (kExpr*)Wexpr;
}

static kExpr* SUGAR kExpr_setUnboxConstValue(KonohaContext *kctx, kExpr *expr, ktype_t ty, uintptr_t unboxValue)
{
	kExprVar *Wexpr = (expr == NULL) ? GCSAFE_new(ExprVar, 0) : (kExprVar*)expr;
	Wexpr->build = TEXPR_NCONST;
	Wexpr->unboxConstValue = unboxValue;
	Wexpr->ty = ty;
	return (kExpr*)Wexpr;
}

static kExpr* SUGAR kExpr_setVariable(KonohaContext *kctx, kExpr *expr, kGamma *gma, int build, ktype_t ty, intptr_t index)
{
	kExprVar *Wexpr = (expr == NULL) ? GCSAFE_new(ExprVar, 0) : (kExprVar*)expr;
	Wexpr->build = build;
	Wexpr->ty = ty;
	Wexpr->index = index;
	if(build == TEXPR_LOCAL && gma->genv->blockScopeShiftSize > 0 && index >= gma->genv->blockScopeShiftSize) {
		Wexpr->build = TEXPR_STACKTOP;
		Wexpr->index -= gma->genv->blockScopeShiftSize;
	}
	return (kExpr*)Wexpr;
}

/* --------------- */
/* Stmt */

static void Stmt_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kStmtVar *stmt = (kStmtVar*)o;
	stmt->uline    = (kfileline_t)conf;
	stmt->syn      = NULL;
	stmt->parentBlockNULL = NULL;
}

static void Stmt_reftrace(KonohaContext *kctx, kObject *o)
{
	kStmt *stmt = (kStmt*)o;
	BEGIN_REFTRACE(1);
	KREFTRACEn(stmt->parentBlockNULL);
	END_REFTRACE();
}


#define AKEY(T)   T, (sizeof(T)-1)

typedef struct {
	const char *key;
	size_t keysize;
	uintptr_t flag;
} KDEFINE_FLAGNAME ;

static uintptr_t kStmt_parseFlags(KonohaContext *kctx, kStmt *stmt, KDEFINE_FLAGNAME *fop, uintptr_t flag)
{
	while(fop->key != NULL) {
		ksymbol_t kw = ksymbolA(fop->key, fop->keysize, SYM_NONAME);
		if(kw != SYM_NONAME) {
			kObject *op = kStmt_getObjectNULL(kctx, stmt, kw);
			if(op != NULL) {
				flag |= fop->flag;
			}
		}
		fop++;
	}
	return flag;
}

#define kStmt_is(STMT, KW) Stmt_is(kctx, STMT, KW)

static inline kbool_t Stmt_is(KonohaContext *kctx, kStmt *stmt, ksymbol_t kw)
{
	return (kStmt_getObjectNULL(kctx, stmt, kw) != NULL);
}

static kToken* kStmt_getToken(KonohaContext *kctx, kStmt *stmt, ksymbol_t kw, kToken *def)
{
	kToken *tk = (kToken*)kStmt_getObjectNULL(kctx, stmt, kw);
	if(tk != NULL && IS_Token(tk)) {
		return tk;
	}
	return def;
}

static kExpr* kStmt_getExpr(KonohaContext *kctx, kStmt *stmt, ksymbol_t kw, kExpr *def)
{
	kExpr *expr = (kExpr*)kStmt_getObjectNULL(kctx, stmt, kw);
	if(expr != NULL && IS_Expr(expr)) {
		return expr;
	}
	return def;
}

static const char* kStmt_getText(KonohaContext *kctx, kStmt *stmt, ksymbol_t kw, const char *def)
{
	kExpr *expr = (kExpr*)kStmt_getObjectNULL(kctx, stmt, kw);
	if(expr != NULL) {
		if(IS_Expr(expr) && Expr_isTerm(expr)) {
			return S_text(expr->termToken->text);
		}
		else if(IS_Token(expr)) {
			kToken *tk = (kToken*)expr;
			if(IS_String(tk->text)) return S_text(tk->text);
		}
	}
	return def;
}

static kbool_t Token_toBRACE(KonohaContext *kctx, kTokenVar *tk, kNameSpace *ns);
static kBlock *new_Block(KonohaContext *kctx, kNameSpace* ns, kStmt *stmt, kArray *tokenArray, int s, int e, int delim);
static kBlock* kStmt_getBlock(KonohaContext *kctx, kStmt *stmt, ksymbol_t kw, kBlock *def)
{
	kBlock *bk = (kBlock*)kStmt_getObjectNULL(kctx, stmt, kw);
	if(bk != NULL) {
		if(IS_Token(bk)) {
			kToken *tk = (kToken*)bk;
			if (tk->keyword == TK_CODE) {
				Token_toBRACE(kctx, (kTokenVar*)tk, Stmt_nameSpace(stmt));
			}
			if (tk->keyword == AST_BRACE) {
				bk = new_Block(kctx, Stmt_nameSpace(stmt), stmt, tk->sub, 0, kArray_size(tk->sub), ';');
				KLIB kObject_setObject(kctx, stmt, kw, TY_Block, bk);
			}
		}
		if(IS_Block(bk)) return bk;
	}
	return def;
}

/* --------------- */
/* Block */

static void Block_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kBlockVar *bk = (kBlockVar*)o;
	kNameSpace *ns = (conf != NULL) ? (kNameSpace*)conf : KNULL(NameSpace);
	bk->parentStmtNULL = NULL;
	KINITv(bk->blockNameSpace, ns);
	KINITv(bk->stmtList, new_(StmtArray, 0));
	KINITv(bk->esp, new_(Expr, 0));
}

static void Block_reftrace(KonohaContext *kctx, kObject *o)
{
	kBlock *bk = (kBlock*)o;
	BEGIN_REFTRACE(4);
	KREFTRACEv(bk->blockNameSpace);
	KREFTRACEv(bk->stmtList);
	KREFTRACEv(bk->esp);
	KREFTRACEn(bk->parentStmtNULL);
	END_REFTRACE();
}

static void kBlock_insertAfter(KonohaContext *kctx, kBlock *bk, kStmt *target, kStmt *stmt)
{
	//DBG_ASSERT(stmt->parentNULL == NULL);
	KSETv(((kStmtVar*)stmt)->parentBlockNULL, bk);
	size_t i;
	for(i = 0; i < kArray_size(bk->stmtList); i++) {
		if(bk->stmtList->stmtItems[i] == target) {
			KLIB kArray_insert(kctx, bk->stmtList, i+1, stmt);
			return;
		}
	}
	DBG_ABORT("target was not found!!");
}

/* --------------- */
/* Block */

static void Gamma_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kGammaVar *gma = (kGammaVar*)o;
	gma->genv = NULL;
}

