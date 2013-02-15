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

/* --------------- */
/* Token */

#ifndef USE_SMALLBUILD

static void dumpToken(KonohaContext *kctx, kToken *tk, int n)
{
	if(verbose_sugar) {
		if(n < 0) n = (short) tk->uline;
		DUMP_P("Token[%d] '%s' TokenType=%s%s", n, KToken_t(tk), KSymbol_Fmt2(tk->tokenType));
		if(kToken_IsIndent(tk)) {
			DUMP_P(" indent=+%d", tk->indent);
		}
		if(tk->symbol > 0) {
			DUMP_P(" Symbol=%s%s", KSymbol_Fmt2(tk->symbol));
		}
		if(tk->resolvedSyntaxInfo != NULL && tk->resolvedSyntaxInfo->keyword == KSymbol_TypePattern) {
			DUMP_P(" resolvedType=%s", KType_text(tk->resolvedTypeId));
		}else {
			if(tk->ruleNameSymbol > 0) {
				DUMP_P(" ruleNameSymbol=%s%s", KSymbol_Fmt2(tk->ruleNameSymbol));
			}
		}
		if(tk->resolvedSyntaxInfo == NULL) {
			DUMP_P(" Syntax=NULL\n");
		}
		else {
			DUMP_P(" Syntax=%s%s\n", KSymbol_Fmt2(tk->resolvedSyntaxInfo->keyword));
		}
	}
}

static void dumpIndent(KonohaContext *kctx, int nest)
{
	int i;
	for(i = 0; i < nest; i++) {
		DUMP_P("  ");
	}
}

static void dumpTokenArray(KonohaContext *kctx, int nest, kArray *a, int s, int e)
{
	if(verbose_sugar) {
		if(nest == 0) DUMP_P(">>>\n");
		while(s < e) {
			kToken *tk = a->TokenItems[s];
			dumpIndent(kctx, nest);
			dumpToken(kctx, tk, s);
			if(IS_Array(tk->GroupTokenList)) {
				dumpIndent(kctx, nest);
				DUMP_P("%s\n", kString_text(tk->GroupTokenList->TokenItems[0]->text));
				dumpTokenArray(kctx, nest+1, RangeGroup(tk->GroupTokenList));
				dumpIndent(kctx, nest);
				DUMP_P("%s\n", kString_text(tk->GroupTokenList->TokenItems[kArray_size(tk->GroupTokenList)-1]->text));
			}
			s++;
		}
		if(nest == 0) DUMP_P("<<<\n");
	}
}

#endif
