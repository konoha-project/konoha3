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

static kbool_t IsPrintableMessage(KonohaContext *kctx, KParserContext *sugarContext, kinfotag_t taglevel)
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

static kString *new_StringMessage(KonohaContext *kctx, kArray *gcstack, KBuffer *wb, kinfotag_t taglevel, kfileline_t uline, const char *fmt, va_list ap)
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

static kString *KParserContext_PrintMessage(KonohaContext *kctx, kinfotag_t taglevel, kfileline_t uline, const char *fmt, va_list ap)
{
	KParserContext *sugarContext = KGetParserContext(kctx);
	if(IsPrintableMessage(kctx, sugarContext, taglevel)) {
		KBuffer wb;
		KLIB KBuffer_Init(&sugarContext->errorMessageBuffer, &wb);
		kString *emsg = new_StringMessage(kctx, sugarContext->errorMessageList, &wb, taglevel, uline, fmt, ap);
		PLATAPI ConsoleModule.ReportCompilerMessage(kctx, taglevel, uline, kString_text(emsg));
		if(taglevel <= ErrTag) {
			sugarContext->errorMessageCount++;
		}
		return emsg;
	}
	return NULL;
}

static void kToken_ToError(KonohaContext *kctx, kTokenVar *tk, kinfotag_t taglevel, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	kString *errmsg = KParserContext_PrintMessage(kctx, taglevel, tk->uline, fmt, ap);
	va_end(ap);
	if(errmsg != NULL) {
		KFieldSet(tk, tk->text, errmsg);
		tk->tokenType = TokenType_Error;
		tk->resolvedSyntaxInfo = NULL;
	}
}

static void kNode_ToError(KonohaContext *kctx, kNode *node, kString *errmsg)
{
	if(errmsg == NULL) { // not in case of isBlockedErrorMessage
		errmsg = TS_EMPTY;
	}
	kNode_setnode(node, KNode_Error);
	node->attrTypeId = KType_void;
	KFieldSet(node, node->ErrorMessage, errmsg);
	kNode_Set(ObjectConst, node, false);
	//node->stackbase = ns == NULL ? 0 : ns->genv->localScope.varsize;
}

static kNode *MessageNode(KonohaContext *kctx, kNode *node, kTokenNULL *tk, kNameSpaceNULL *ns, kinfotag_t taglevel, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	kfileline_t uline = kNode_uline(node);
	if(tk != NULL) {
		assert(IS_Token(tk));
		uline = tk->uline;
	}
	kString *errmsg = KParserContext_PrintMessage(kctx, taglevel, uline, fmt, ap);
	if(taglevel <= ErrTag && !kNode_IsError(node)) {
		kNode_ToError(kctx, node, errmsg);
	}
	va_end(ap);
	return node;
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
		kString *emsg = KParserContext_PrintMessage(kctx, taglevel, uline, fmt, ap);
		va_end(ap);
		if(taglevel <= ErrTag && !kNode_IsError(stmt)) {
			kNode_ToError(kctx, stmt, emsg);
		}
	}
	else {
		INIT_GCSTACK();
		KBuffer wb;
		KLIB KBuffer_Init(&kctx->stack->cwb, &wb);
		kString *emsg = new_StringMessage(kctx, _GcStack, &wb, taglevel, Trace_pline(trace), fmt, ap);
		va_end(ap);
		PLATAPI ConsoleModule.ReportCompilerMessage(kctx, taglevel, Trace_pline(trace), kString_text(emsg));
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

static kNode *ERROR_SyntaxErrorToken(KonohaContext *kctx, kNode *stmt, kToken *tk)
{
	return kNodeToken_Message(kctx, stmt, tk, ErrTag, "syntax error at %s", KToken_t(tk));
}

#define ERROR_UndefinedEscapeSequence(kctx, stmt, tk) ERROR_SyntaxErrorToken(kctx, stmt, tk)

#else

static kNode *ERROR_UndefinedEscapeSequence(KonohaContext *kctx, kNode *stmt, kToken *tk)
{
	return kNodeToken_Message(kctx, stmt, tk, ErrTag, "undefined escape sequence: \"%s\"", kString_text(tk->text));
}

#endif

#ifdef __cplusplus
}
#endif
