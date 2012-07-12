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

/* ************************************************************************ */

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */
/* [perror] */

static int isPRINT(KonohaContext *kctx, int pe)
{
	if(verbose_sugar) return true;
	if(pe == INFO_) {
		if(KonohaContext_isInteractive(kctx) || KonohaContext_isCompileOnly(kctx)) {
			return true;
		}
		return false;
	}
	return true;
}

static kString* vperrorf(KonohaContext *kctx, int pe, kfileline_t uline, int lpos, const char *fmt, va_list ap)
{
	if(isPRINT(kctx, pe)) {
		const char *msg = TAG_t(pe);
		size_t errref = ((size_t)-1);
		SugarContext *base = ctxsugar;
		KUtilsWriteBuffer wb;
		kwb_init(&base->errorMessageBuffer, &wb);
		size_t pos = wb.m->bytesize;
		if(uline > 0) {
			const char *file = SS_t(uline);
			kwb_printf(&wb, "%s(%s:%d) " , msg, shortfilename(file), (kushort_t)uline);
		}
		else {
			kwb_printf(&wb, "%s" , msg);
		}
		size_t len = wb.m->bytesize - pos;
		kwb_vprintf(&wb, fmt, ap);
		msg = KUtilsWriteBufferop(&wb, 1);
		kreportf(pe, uline, "%s", msg + len);
		kString *emsg = new_kString(msg, strlen(msg), 0);
		errref = kArray_size(base->errorMessageList);
		kArray_add(base->errorMessageList, emsg);
		if(pe == ERR_ || pe == CRIT_) {
			base->errorMessageCount ++;
		}
		return emsg;
	}
	return NULL;
}

#define pWARN(UL, FMT, ...) sugar_p(kctx, WARN_, UL, -1, FMT, ## __VA_ARGS__)

static kString* sugar_p(KonohaContext *kctx, int pe, kfileline_t uline, int lpos, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	kString *errmsg = vperrorf(kctx, pe, uline, lpos, fmt, ap);
	va_end(ap);
	return errmsg;
}

static void Token_pERR(KonohaContext *kctx, kTokenVar *tk, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	kString *errmsg = vperrorf(kctx, ERR_, tk->uline, -1, fmt, ap);
	va_end(ap);
	KSETv(tk->text, errmsg);
	tk->keyword = TK_ERR;
	kToken_setUnresolved(tk, true);
}

#define kStmt_toERR(STMT, ENO)  Stmt_toERR(kctx, STMT, ENO)
#define kStmt_isERR(STMT)       ((STMT)->build == TSTMT_ERR)
static SugarSyntax* NameSpace_syn(KonohaContext *kctx, kNameSpace *ns0, ksymbol_t kw, int isnew);

static void Stmt_toERR(KonohaContext *kctx, kStmt *stmt, kString *errmsg)
{
	((kStmtVar*)stmt)->syn   = SYN_(kStmt_nameSpace(stmt), KW_ERR);
	((kStmtVar*)stmt)->build = TSTMT_ERR;
	kObject_setObject(stmt, KW_ERR, errmsg);
}

static inline void kStmt_errline(kStmt *stmt, kfileline_t uline)
{
	((kStmtVar*)stmt)->uline = uline;
}

static kfileline_t Expr_uline(KonohaContext *kctx, kExpr *expr, kfileline_t uline)
{
	kToken *tk = expr->tk;
	DBG_ASSERT(IS_Expr(expr));
	if(IS_Token(tk) && tk->uline >= uline) {
		return tk->uline;
	}
	kArray *a = expr->cons;
	if(a != NULL && IS_Array(a)) {
		size_t i;
		for(i=0; i < kArray_size(a); i++) {
			tk = a->toks[i];
			if(IS_Token(tk) && tk->uline >= uline) {
				return tk->uline;
			}
			if(IS_Expr(tk)) {
				return Expr_uline(kctx, a->exprs[i], uline);
			}
		}
	}
	return uline;
}

#define kStmt_p(STMT, PE, FMT, ...)        Stmt_p(kctx, STMT, NULL, PE, FMT, ## __VA_ARGS__)
#define kToken_p(STMT, TK, PE, FMT, ...)   Stmt_p(kctx, STMT, TK, PE, FMT, ## __VA_ARGS__)
#define kExpr_p(STMT, EXPR, PE, FMT, ...)  Stmt_p(kctx, STMT, (kToken*)EXPR, PE, FMT, ## __VA_ARGS__)

static kExpr* Stmt_p(KonohaContext *kctx, kStmt *stmt, kToken *tk, int pe, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	kfileline_t uline = stmt->uline;
	if(tk != NULL && pe <= ERR_ ) {
		if(IS_Token(tk)) {
			uline = tk->uline;
		}
		else if(IS_Expr(tk)) {
			uline = Expr_uline(kctx, (kExpr*)tk, uline);
		}
	}
	kString *errmsg = vperrorf(kctx, pe, uline, -1, fmt, ap);
	if(pe <= ERR_ && !kStmt_isERR(stmt)) {
		kStmt_toERR(stmt, errmsg);
	}
	va_end(ap);
	return K_NULLEXPR;
}

#define kToken_s(tk) kToken_s_(kctx, tk)
static const char *kToken_s_(KonohaContext *kctx, kToken *tk)
{
	switch((int)tk->keyword) {
	case TK_INDENT: return "indent";
	case TK_CODE: ;
	case AST_BRACE: return "{... }";
	case AST_PARENTHESIS: return "(... )";
	case AST_BRACKET: return "[... ]";
	default:  return S_text(tk->text);
	}
}

static void WARN_Ignored(KonohaContext *kctx, kArray *tls, int s, int e)
{
	if(s < e) {
		int i = s;
		KUtilsWriteBuffer wb;
		kwb_init(&(kctx->stack->cwb), &wb);
		kwb_printf(&wb, "%s", kToken_s(tls->toks[i])); i++;
		while(i < e) {
			kwb_printf(&wb, " %s", kToken_s(tls->toks[i])); i++;
		}
		pWARN(tls->toks[s]->uline, "ignored tokens: %s", KUtilsWriteBufferop(&wb, 1));
		kwb_free(&wb);
	}
}

#ifdef __cplusplus
}
#endif
