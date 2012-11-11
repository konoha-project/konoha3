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

static int isPrintMessage(KonohaContext *kctx, SugarContext *sugarContext, kinfotag_t taglevel)
{
	if(sugarContext->isBlockedErrorMessage) return false;
	if(verbose_sugar) return true;
	if(taglevel == InfoTag) {
		if(KonohaContext_Is(Interactive, kctx) || KonohaContext_Is(CompileOnly, kctx) || KonohaContext_Is(Debug, kctx)) {
			return true;
		}
		return false;
	}
	return true;
}

static kString* new_StringMessage(KonohaContext *kctx, kArray *gcstack, KGrowingBuffer *wb, kinfotag_t taglevel, kfileline_t uline, const char *fmt, va_list ap)
{
	const char *msg = TAG_t(taglevel);
	if(uline > 0) {
		const char *file = FileId_t(uline);
		KLIB Kwb_printf(kctx, wb, "%s(%s:%d) " , msg, PLATAPI shortFilePath(file), (kushort_t)uline);
	}
	else {
		KLIB Kwb_printf(kctx, wb, "%s" , msg);
	}
	KLIB Kwb_vprintf(kctx, wb, fmt, ap);
	msg = KLIB Kwb_top(kctx, wb, 0);
	return KLIB new_kString(kctx, gcstack, msg, Kwb_bytesize(wb), 0);
}

static kString* SugarContext_vprintMessage(KonohaContext *kctx, kinfotag_t taglevel, kfileline_t uline, const char *fmt, va_list ap)
{
	SugarContext *sugarContext = GetSugarContext(kctx);
	if(isPrintMessage(kctx, sugarContext, taglevel)) {
		KGrowingBuffer wb;
		KLIB Kwb_init(&sugarContext->errorMessageBuffer, &wb);
		kString *emsg = new_StringMessage(kctx, sugarContext->errorMessageList, &wb, taglevel, uline, fmt, ap);
		PLATAPI ReportCompilerMessage(kctx, taglevel, uline, S_text(emsg));
		if(taglevel <= ErrTag) {
			sugarContext->errorMessageCount++;
		}
		KLIB Kwb_free(&wb);
		return emsg;
	}
	return NULL;
}

static kString* SugarContext_printMessage(KonohaContext *kctx, kinfotag_t taglevel, kfileline_t uline, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	kString *errmsg = SugarContext_vprintMessage(kctx, taglevel, uline, fmt, ap);
	va_end(ap);
	return errmsg;
}

static void kToken_printMessage(KonohaContext *kctx, kTokenVar *tk, kinfotag_t taglevel, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	kString *errmsg = SugarContext_vprintMessage(kctx, taglevel, tk->uline, fmt, ap);
	va_end(ap);
	if(errmsg != NULL) {
		KFieldSet(tk, tk->text, errmsg);
		tk->unresolvedTokenType = TokenType_ERR;
		tk->resolvedSyntaxInfo = NULL;
	}
}

#define Stmt_isERR(STMT)       ((STMT)->build == TSTMT_ERR)
static SugarSyntax* kNameSpace_GetSyntax(KonohaContext *kctx, kNameSpace *ns0, ksymbol_t kw, int isnew);

static void kStmt_toERR(KonohaContext *kctx, kStmt *stmt, kString *errmsg)
{
	if(errmsg != NULL) { // not in case of isBlockedErrorMessage
		((kStmtVar *)stmt)->syn   = SYN_(Stmt_ns(stmt), KW_ERR);
		((kStmtVar *)stmt)->build = TSTMT_ERR;
		KLIB kObject_setObject(kctx, stmt, KW_ERR, TY_String, errmsg);
	}
}

static kfileline_t kExpr_uline(KonohaContext *kctx, kExpr *expr, kfileline_t uline)
{
	kToken *tk = expr->termToken;
	DBG_ASSERT(IS_Expr(expr));
	if(IS_Token(tk) && tk != K_NULLTOKEN && tk->uline >= uline) {
		return tk->uline;
	}
	kArray *a = expr->cons;
	if(a != NULL && IS_Array(a)) {
		size_t i;
		for(i=0; i < kArray_size(a); i++) {
			tk = a->TokenItems[i];
			if(IS_Token(tk) && tk->uline >= uline) {
				return tk->uline;
			}
			if(IS_Expr(tk)) {
				return kExpr_uline(kctx, a->ExprItems[i], uline);
			}
		}
	}
	return uline;
}

static kExpr* kStmt_printMessage2(KonohaContext *kctx, kStmt *stmt, kToken *tk, kinfotag_t taglevel, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	kfileline_t uline = stmt->uline;
	if(tk != NULL && taglevel <= ErrTag ) {
		if(IS_Token(tk)) {
			uline = tk->uline;
		}
		else if(IS_Expr(tk)) {
			uline = kExpr_uline(kctx, (kExpr *)tk, uline);
		}
	}
	kString *errmsg = SugarContext_vprintMessage(kctx, taglevel, uline, fmt, ap);
	if(taglevel <= ErrTag && !Stmt_isERR(stmt)) {
		kStmt_toERR(kctx, stmt, errmsg);
	}
	va_end(ap);
	return K_NULLEXPR;
}

int verbose_debug;

void TRACE_ReportScriptMessage(KonohaContext *kctx, KTraceInfo *trace, kinfotag_t taglevel, const char *fmt, ...)
{
	if(taglevel == DebugTag && !verbose_debug) return;
	va_list ap;
	va_start(ap, fmt);
	if(trace != NULL && IS_Stmt(trace->baseStack[1].asStmt)) {  // Static Compiler Message
		kStmt *stmt = trace->baseStack[1].asStmt;
		kfileline_t uline = stmt->uline;
		kString *emsg = SugarContext_vprintMessage(kctx, taglevel, uline, fmt, ap);
		va_end(ap);
		if(taglevel <= ErrTag && !Stmt_isERR(stmt)) {
			kStmt_toERR(kctx, stmt, emsg);
		}
	}
	else {
		INIT_GCSTACK();
		KGrowingBuffer wb;
		KLIB Kwb_init(&kctx->stack->cwb, &wb);
		kString *emsg = new_StringMessage(kctx, _GcStack, &wb, taglevel, Trace_pline(trace), fmt, ap);
		va_end(ap);
		PLATAPI ReportCompilerMessage(kctx, taglevel, Trace_pline(trace), S_text(emsg));
		if(taglevel <= ErrTag) {
			if(trace == NULL) {
				KExit(EXIT_FAILURE);
			}
			if(trace->errorSymbol != 0 && trace->faultType != 0) {
				KLIB KonohaRuntime_raise(kctx, trace->errorSymbol, trace->faultType, emsg, trace->baseStack);
				return; /* in case of that KonohaRuntime_raise cannot jump; */
			}
		}
		KLIB Kwb_free(&wb);
		RESET_GCSTACK();
	}
}

#define Token_text(tk) kToken_t_(kctx, tk)

static const char *kToken_t_(KonohaContext *kctx, kToken *tk)
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

// libperror

#ifdef USE_SMALLBUILD

static kExpr* ERROR_SyntaxErrorToken(KonohaContext *kctx, kStmt *stmt, kToken *tk)
{
	return kStmtToken_printMessage(kctx, stmt, tk, ErrTag, "syntax error at %s", Token_text(tk));
}

#define ERROR_UndefinedEscapeSequence(kctx, stmt, tk) ERROR_SyntaxErrorToken(kctx, stmt, tk)

#else

static kExpr* ERROR_UndefinedEscapeSequence(KonohaContext *kctx, kStmt *stmt, kToken *tk)
{
	return kStmtToken_printMessage(kctx, stmt, tk, ErrTag, "undefined escape sequence: \"%s\"", S_text(tk->text));
}

#endif

#ifdef __cplusplus
}
#endif
