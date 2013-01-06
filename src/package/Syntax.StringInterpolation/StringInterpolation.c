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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/klib.h>

#ifdef __cplusplus
extern "C" {
#endif

// --------------------------------------------------------------------------
// String Interpolation

/* copied from src/sugar/sugarfunc.h */
static kString *kToken_ResolveEscapeSequence(KonohaContext *kctx, kToken *tk, size_t start)
{
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	const char *text = kString_text(tk->text) + start;
	const char *end  = kString_text(tk->text) + kString_size(tk->text);
	KLIB KBuffer_Write(kctx, &wb, kString_text(tk->text), start);
	while(text < end) {
		int ch = *text;
		if(ch == '\\' && *(text+1) != '\0') {
			switch (*(text+1)) {
			/*
			 * compatible with ECMA-262
			 * http://ecma-international.org/ecma-262/5.1/#sec-7.8.4
			 */
			case 'b':  ch = '\b'; text++; break;
			case 't':  ch = '\t'; text++; break;
			case 'n':  ch = '\n'; text++; break;
			case 'v':  ch = '\v'; text++; break;
			case 'f':  ch = '\f'; text++; break;
			case 'r':  ch = '\r'; text++; break;
			case '"':  ch = '"';  text++; break;
			case '\'': ch = '\''; text++; break;
			case '\\': ch = '\\'; text++; break;
			default: return NULL;
			}
		}
		{
			char buf[1] = {ch};
			KLIB KBuffer_Write(kctx, &wb, (const char *)buf, 1);
		}
		text++;
	}
	return KLIB KBuffer_Stringfy(kctx, &wb, OnGcStack, StringPolicy_FreeKBuffer);
}

static kString *remove_escapes(KonohaContext *kctx, kToken *tk)
{
	kString *text = tk->text;
	if(kToken_Is(RequiredReformat, tk)) {
		const char *escape = strchr(kString_text(text), '\\');
		DBG_ASSERT(escape != NULL);
		text = kToken_ResolveEscapeSequence(kctx, tk, escape - kString_text(text));
	}
	return text;
}

static KMETHOD TypeCheck_ExtendedTextLiteral(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kToken  *tk   = expr->TermToken;
	INIT_GCSTACK();
	kString *text = remove_escapes(kctx, tk);
	if(text == NULL) {
		KReturnWith(K_NULLEXPR, RESET_GCSTACK());
	}

	const char *end = NULL;
	const char *str = kString_text(text);
	const char *start = strstr(str, "${");
	if(start == NULL) {
		KReturnWith(K_NULLEXPR, RESET_GCSTACK());
	}
	expr = (kExprVar *) SUGAR kExpr_SetConstValue(kctx, expr, NULL, UPCAST(text));
	kNameSpace *ns = Stmt_ns(stmt);
	kMethod *concat = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, KClass_String, KMethodName_("+"), 1, KMethodMatch_NoOption);

	expr = (kExprVar *) new_ConstValueExpr(kctx, NULL, UPCAST(TS_EMPTY));
	while(true) {
		start = strstr(str, "${");
		if(start == NULL) {
			break;
		}
		if(start == strstr(str, "${}")) {
			str += 3;
			continue;
		}
		end = strchr(start, '}');
		if(end == NULL) {
			break;
		}

		KBuffer wb;
		KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
		KLIB KBuffer_Write(kctx, &wb, "(", 1);
		KLIB KBuffer_Write(kctx, &wb, start+2, end-(start+2));
		KLIB KBuffer_Write(kctx, &wb, ")", 1);

		KTokenSeq range = {ns, KGetParserContext(kctx)->preparedTokenList};
		KTokenSeq_Push(kctx, range);
		const char *buf = KLIB KBuffer_text(kctx, &wb, EnsureZero);
		SUGAR KTokenSeq_Tokenize(kctx, &range, buf, 0);

		{
			KTokenSeq tokens = {ns, KGetParserContext(kctx)->preparedTokenList};
			KTokenSeq_Push(kctx, tokens);
			SUGAR KTokenSeq_Preprocess(kctx, &tokens, NULL, &range, range.beginIdx);
			kExpr *newexpr = SUGAR kStmt_ParseExpr(kctx, stmt, tokens.tokenList, tokens.beginIdx, tokens.endIdx, NULL);
			KTokenSeq_Pop(kctx, tokens);

			if(start - str > 0) {
				kExpr *first = new_ConstValueExpr(kctx, NULL,
						UPCAST(KLIB new_kString(kctx, OnGcStack, str, (start - str), 0)));
				expr = (kExprVar *) SUGAR new_TypedCallExpr(kctx, stmt, gma, KClass_String, concat, 2, expr, first);
			}
			expr = (kExprVar *) SUGAR new_TypedCallExpr(kctx, stmt, gma, KClass_String, concat, 2, expr, newexpr);
		}
		KTokenSeq_Pop(kctx, range);
		KLIB KBuffer_Free(&wb);
		str = end + 1;
	}

	if((start == NULL) || (start != NULL && end == NULL)) {
		kExpr *rest = new_ConstValueExpr(kctx, KClass_String,
				UPCAST(KLIB new_kString(kctx, OnGcStack, str, strlen(str), 0)));
		expr = (kExprVar *) SUGAR new_TypedCallExpr(kctx, stmt, gma, KClass_String, concat, 2, expr, rest);
	}
	KReturnWith(expr, RESET_GCSTACK());
}


// --------------------------------------------------------------------------

static kbool_t StringInterpolationPackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_TextPattern, 0,  NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_ExtendedTextLiteral, },
		{ KSymbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}

static kbool_t StringInterpolationExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* StringInterpolation_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "JavaScript", "1.0");
	d.PackupNameSpace    = StringInterpolationPackupNameSpace;
	d.ExportNameSpace   = StringInterpolationExportNameSpace;
	return &d;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
