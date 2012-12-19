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

static int IsPrintableMessage(KonohaContext *kctx, KParserContext *sugarContext, kinfotag_t taglevel)
{
	if(sugarContext->isNodeedErrorMessage) return false;
	if(verbose_sugar) return true;
	if(taglevel == InfoTag) {
		if(KonohaContext_Is(Interactive, kctx) || KonohaContext_Is(CompileOnly, kctx) || KonohaContext_Is(Debug, kctx)) {
			return true;
		}
		return false;
	}
	return true;
}

static kString* new_StringMessage(KonohaContext *kctx, kArray *gcstack, KBuffer *wb, kinfotag_t taglevel, kfileline_t uline, const char *fmt, va_list ap)
{
	const char *msg = TAG_t(taglevel);
	if(uline > 0) {
		const char *file = KFileLine_textFileName(uline);
		KLIB KBuffer_printf(kctx, wb, "%s(%s:%d) " , msg, PLATAPI shortFilePath(file), (kushort_t)uline);
	}
	else {
		KLIB KBuffer_printf(kctx, wb, "%s " , msg);
	}
	KLIB KBuffer_vprintf(kctx, wb, fmt, ap);
	return KLIB KBuffer_Stringfy(kctx, wb, gcstack, StringPolicy_ASCII|StringPolicy_FreeKBuffer);
}

static kString* KParserContext_vprintMessage(KonohaContext *kctx, kinfotag_t taglevel, kfileline_t uline, const char *fmt, va_list ap)
{
	KParserContext *sugarContext = KGetParserContext(kctx);
	if(IsPrintableMessage(kctx, sugarContext, taglevel)) {
		KBuffer wb;
		KLIB KBuffer_Init(&sugarContext->errorMessageBuffer, &wb);
		kString *emsg = new_StringMessage(kctx, sugarContext->errorMessageList, &wb, taglevel, uline, fmt, ap);
		PLATAPI ReportCompilerMessage(kctx, taglevel, uline, kString_text(emsg));
		if(taglevel <= ErrTag) {
			sugarContext->errorMessageCount++;
		}
		return emsg;
	}
	return NULL;
}

//static kString* KParserContext_Message(KonohaContext *kctx, kinfotag_t taglevel, kfileline_t uline, const char *fmt, ...)
//{
//	va_list ap;
//	va_start(ap, fmt);
//	kString *errmsg = KParserContext_vprintMessage(kctx, taglevel, uline, fmt, ap);
//	va_end(ap);
//	return errmsg;
//}

static void kToken_ToError(KonohaContext *kctx, kTokenVar *tk, kinfotag_t taglevel, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	kString *errmsg = KParserContext_vprintMessage(kctx, taglevel, tk->uline, fmt, ap);
	va_end(ap);
	if(errmsg != NULL) {
		KFieldSet(tk, tk->text, errmsg);
		tk->unresolvedTokenType = TokenType_ERR;
		tk->resolvedSyntaxInfo = NULL;
	}
}

static KSyntax* kNameSpace_GetSyntax(KonohaContext *kctx, kNameSpace *ns0, ksymbol_t kw, int isnew);

#ifdef USE_NODE
static void kNode_toERR(KonohaContext *kctx, kNode *stmt, kString *errmsg)
{
	if(errmsg != NULL) { // not in case of isNodeedErrorMessage
		kNodeVar *node = (kNodeVar*)stmt;
		node->node = KNode_Error;
		KFieldSet(node, node->ErrorMessage, errmsg);
		kNode_Set(ObjectConst, node, true);
		//KLIB kObjectProto_SetObject(kctx, stmt, KSymbol_ERR, KType_String, errmsg);
	}
}

static kfileline_t kNode_uline(KonohaContext *kctx, kNode *expr, kfileline_t uline)
{
	return expr->uline;
}

#else

static void kNode_toERR(KonohaContext *kctx, kNode *stmt, kString *errmsg)
{
	if(errmsg != NULL) { // not in case of isNodeedErrorMessage
		((kNodeVar *)stmt)->syn   = KSyntax_(Node_ns(stmt), KSymbol_ERR);
		((kNodeVar *)stmt)->node = KNode_Error;
		KLIB kObjectProto_SetObject(kctx, stmt, KSymbol_ERR, KType_String, errmsg);
	}
}

static kfileline_t kNode_uline(KonohaContext *kctx, kNode *expr, kfileline_t uline)
{
	kToken *tk = expr->TermToken;
	DBG_ASSERT(IS_Node(expr));
	if(IS_Token(tk) && tk != K_NULLTOKEN && tk->uline >= uline) {
		return tk->uline;
	}
	kArray *a = expr->NodeList;
	if(a != NULL && IS_Array(a)) {
		size_t i;
		for(i=0; i < kArray_size(a); i++) {
			tk = a->TokenItems[i];
			if(IS_Token(tk) && tk->uline >= uline) {
				return tk->uline;
			}
			if(IS_Node(tk)) {
				return kNode_uline(kctx, a->NodeItems[i], uline);
			}
		}
	}
	return uline;
}

#endif

static kNode* kNode_Message2(KonohaContext *kctx, kNode *stmt, kToken *tk, kinfotag_t taglevel, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	kfileline_t uline = kNode_uline(stmt);
	if(tk != NULL && taglevel <= ErrTag ) {
		if(IS_Token(tk)) {
			uline = tk->uline;
		}
//		else if(IS_Node(tk)) {
//			uline = kNode_uline(kctx, (kNode *)tk, uline);
//		}
	}
	kString *errmsg = KParserContext_vprintMessage(kctx, taglevel, uline, fmt, ap);
	if(taglevel <= ErrTag && !kNode_IsERR(stmt)) {
		kNode_toERR(kctx, stmt, errmsg);
	}
	va_end(ap);
	return K_NULLNODE;
}

int verbose_debug;

void TRACE_ReportScriptMessage(KonohaContext *kctx, KTraceInfo *trace, kinfotag_t taglevel, const char *fmt, ...)
{
	if(taglevel == DebugTag && !verbose_debug) return;
	va_list ap;
	va_start(ap, fmt);
	if(trace != NULL && IS_Node(trace->baseStack[1].asNode)) {  // Static Compiler Message
		kNode *stmt = trace->baseStack[1].asNode;
		kfileline_t uline = kNode_uline(stmt);
		kString *emsg = KParserContext_vprintMessage(kctx, taglevel, uline, fmt, ap);
		va_end(ap);
		if(taglevel <= ErrTag && !kNode_IsERR(stmt)) {
			kNode_toERR(kctx, stmt, emsg);
		}
	}
	else {
		INIT_GCSTACK();
		KBuffer wb;
		KLIB KBuffer_Init(&kctx->stack->cwb, &wb);
		kString *emsg = new_StringMessage(kctx, _GcStack, &wb, taglevel, Trace_pline(trace), fmt, ap);
		va_end(ap);
		PLATAPI ReportCompilerMessage(kctx, taglevel, Trace_pline(trace), kString_text(emsg));
		if(taglevel <= ErrTag) {
			if(trace == NULL) {
				KExit(EXIT_FAILURE);
			}
			if(trace->errorSymbol != 0 && trace->faultType != 0) {
				KLIB KRuntime_raise(kctx, trace->errorSymbol, trace->faultType, emsg, trace->baseStack);
				return; /* in case of that KRuntime_raise cannot jump; */
			}
		}
		KLIB KBuffer_Free(&wb);
		RESET_GCSTACK();
	}
}

// libperror

#ifdef USE_SMALLBUILD

static kNode* ERROR_SyntaxErrorToken(KonohaContext *kctx, kNode *stmt, kToken *tk)
{
	return kNodeToken_Message(kctx, stmt, tk, ErrTag, "syntax error at %s", KToken_t(tk));
}

#define ERROR_UndefinedEscapeSequence(kctx, stmt, tk) ERROR_SyntaxErrorToken(kctx, stmt, tk)

#else

static kNode* ERROR_UndefinedEscapeSequence(KonohaContext *kctx, kNode *stmt, kToken *tk)
{
	return kNodeToken_Message(kctx, stmt, tk, ErrTag, "undefined escape sequence: \"%s\"", kString_text(tk->text));
}

#endif

#ifdef __cplusplus
}
#endif
