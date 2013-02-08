/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
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

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/klib.h>

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

static kNode *ParseSource(KonohaContext *kctx, kNameSpace *ns, const char *script, size_t len)
{
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	KLIB KBuffer_Write(kctx, &wb, "(", 1);
	KLIB KBuffer_Write(kctx, &wb, script, len);
	KLIB KBuffer_Write(kctx, &wb, ")", 1);

	KTokenSeq tokens = {ns, KGetParserContext(kctx)->preparedTokenList};
	KTokenSeq_Push(kctx, tokens);
	const char *buf = KLIB KBuffer_text(kctx, &wb, EnsureZero);
	SUGAR Tokenize(kctx, ns, buf, 0, 0, tokens.tokenList);
	KTokenSeq_End(kctx, tokens);

	KTokenSeq step2 = {ns, tokens.tokenList, kArray_size(tokens.tokenList)};
	SUGAR Preprocess(kctx, ns, RangeTokenSeq(tokens), NULL, step2.tokenList);
	KTokenSeq_End(kctx, step2);
	kNode *newexpr = SUGAR ParseNewNode(kctx, ns, step2.tokenList, &step2.beginIdx, step2.endIdx, 0, NULL);
	KTokenSeq_Pop(kctx, tokens);
	KLIB KBuffer_Free(&wb);
	return newexpr;
}

static KMETHOD Expression_ExtendedTextLiteral(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(expr, tokenList, beginIdx, opIdx, endIdx);
	kNameSpace *ns = kNode_ns(expr);
	kToken *tk = tokenList->TokenItems[opIdx];
	INIT_GCSTACK();
	kString *text = remove_escapes(kctx, tk);
	if(beginIdx != opIdx) {
		/* FIXME */
		assert(0 && "FIXME");
		KReturnUnboxValue(-1);
	}

	if(text == NULL) {
		/* text contain unsupported escape sequences */
		RESET_GCSTACK();
		KReturnUnboxValue(-1);
	}

	const char *str = kString_text(text);
	const char *end = NULL;
	const char *start = strstr(str, "${");
	if(start == NULL) {
		/* text does not contain Interpolation expressions */
		RESET_GCSTACK();
		KReturnUnboxValue(beginIdx+1);
	}
	kSyntax *addSyntax = kSyntax_(ns, KSymbol_("+"));
	kTokenVar *opToken = tokenList->TokenVarItems[beginIdx];
	opToken->symbol = KSymbol_("+");
	opToken->text   = KLIB new_kString(kctx, OnGcStack, "+", 1, 0);
	KFieldSet(opToken, opToken->resolvedSyntaxInfo, addSyntax);
	SUGAR kNode_Op(kctx, expr, opToken, 0);

	/* [before] "aaa${bbb}ccc"
	 * [after]  "" + "aaa" + bbb + "ccc"
	 */
	SUGAR kNode_AddNode(kctx, expr, new_ConstNode(kctx, ns, NULL, UPCAST(TS_EMPTY)));
	while(true) {
		start = strstr(str, "${");
		if(start == NULL)
			break;
		if(start == strstr(str, "${}")) {
			str += 3;
			continue;
		}
		end = strchr(start, '}');
		if(end == NULL)
			break;
		kNode *newexpr = ParseSource(kctx, ns, start+2, end-(start+2));
		if(start - str > 0) {
			kNode *first = new_ConstNode(kctx, ns, NULL,
					UPCAST(KLIB new_kString(kctx, OnGcStack, str, (start - str), 0)));
			SUGAR kNode_AddNode(kctx, expr, first);
		}
		SUGAR kNode_AddNode(kctx, expr, newexpr);
		str = end + 1;
	}

	if((start == NULL) || (start != NULL && end == NULL)) {
		kNode *rest = new_ConstNode(kctx, ns, KClass_String,
				UPCAST(KLIB new_kString(kctx, OnGcStack, str, strlen(str), 0)));
		SUGAR kNode_AddNode(kctx, expr, rest);
	}

	/* (+ 1 2 3 4) => (+ (+ (+ 1 2) 3 ) 4) */
	int i, size = kNode_GetNodeListSize(kctx, expr);
	assert(size > 2);
	kNode *leftNode = kNode_At(expr, 1), *rightNode;
	for(i = 2; i < size-1; i++) {
		kNode *node = KNewNode(ns);
		rightNode = kNode_At(expr, i);
		SUGAR kNode_Op(kctx, node, opToken, 2, leftNode, rightNode);
		leftNode = node;
	}
	rightNode = kNode_At(expr, i);
	KLIB kArray_Clear(kctx, expr->NodeList, 1);
	KLIB kArray_Add(kctx, expr->NodeList, leftNode);
	KLIB kArray_Add(kctx, expr->NodeList, rightNode);
	RESET_GCSTACK();
	KReturnUnboxValue(beginIdx+1);
}

// --------------------------------------------------------------------------

static kbool_t StringInterpolation_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
#define PATTERN(X) KSymbol_##X##Pattern
	kSyntax *textSyntax = kSyntax_(KNULL(NameSpace), PATTERN(Text));
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_TextPattern, SYNFLAG_CParseFunc, 0, 0, {SUGARFUNC Expression_ExtendedTextLiteral}, {textSyntax->TypeFuncNULL}, },
		{ KSymbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}

static kbool_t StringInterpolation_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *StringInterpolation_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "Konoha", "1.0");
	d.PackupNameSpace = StringInterpolation_PackupNameSpace;
	d.ExportNameSpace = StringInterpolation_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
