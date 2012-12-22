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
		if(n < 0) n = (short)tk->uline;
		if(tk->resolvedSyntaxInfo == NULL) {
			if(kToken_IsIndent(tk)) {
				DUMP_P("Token[%d] '%s' TokenType=%s%s indent=%d\n", n, KToken_t(tk), KSymbol_Fmt2(tk->unresolvedTokenType), tk->indent);
			}
			else {
				DUMP_P("Token[%d] '%s' TokenType=``%s%s''\n", n, KToken_t(tk), KSymbol_Fmt2(tk->unresolvedTokenType));
			}
		}
//		else if(Token_isRule(tk)) {
//			DUMP_P("RuleToken(%d) '%s' resolvedSymbol=%s%s classNameSymbol=%s%s\n", n, KToken_t(tk), KSymbol_Fmt2(tk->resolvedSymbol), KSymbol_Fmt2(tk->indent));
//		}
		else if(tk->resolvedSyntaxInfo->keyword == KSymbol_TypePattern) {
			DUMP_P("Token(%d) '%s' type=%s\n", n, KToken_t(tk), KType_text(tk->resolvedTypeId));
		}
		else {
			DUMP_P("Token(%d) '%s' syntax=%s%s, symbol=``%s%s''\n", n, KToken_t(tk), KSymbol_Fmt2(tk->resolvedSyntaxInfo->keyword), KSymbol_Fmt2(tk->resolvedSymbol));
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

static int dumpBeginTokenList(int closure)
{
	switch(closure) {
	case KSymbol_ParenthesisGroup: return '(';
	case KSymbol_BraceGroup: return '{';
	case KSymbol_BracketGroup: return '[';
	}
	return '<';
}

static int dumpEndTokenList(int closure)
{
	switch(closure) {
	case KSymbol_ParenthesisGroup: return ')';
	case KSymbol_BraceGroup: return '}';
	case KSymbol_BracketGroup: return ']';
	}
	return '>';
}

static void dumpTokenArray(KonohaContext *kctx, int nest, kArray *a, int s, int e)
{
	if(verbose_sugar) {
		while(s < e) {
			kToken *tk = a->TokenItems[s];
			dumpIndent(kctx, nest);
			if(IS_Array(tk->subTokenList)) {
				ksymbol_t closure = (tk->resolvedSyntaxInfo == NULL) ? tk->resolvedSymbol : tk->resolvedSyntaxInfo->keyword;
				DUMP_P("%c\n", dumpBeginTokenList(closure));
				dumpTokenArray(kctx, nest+1, tk->subTokenList, 0, kArray_size(tk->subTokenList));
				dumpIndent(kctx, nest);
				DUMP_P("%c\n", dumpEndTokenList(closure));
			}
			else {
				dumpToken(kctx, tk, s);
			}
			s++;
		}
		if(nest == 0) DUMP_P("====\n");
	}
}

static void dumpExpr(KonohaContext *kctx, int n, int nest, kNode *expr)
{
	DBG_ASSERT(IS_Node(expr));
	if(verbose_sugar) {
		dumpIndent(kctx, nest);
		if(expr == K_NULLNODE) {
			DUMP_P("[%d] NullObject", n);
		}
		else if(kNode_IsTerm(expr)) {
			DUMP_P("[%d] TermToken: ", n);
			dumpToken(kctx, expr->TermToken, -1);
		}
		else {
			if(expr->syn == NULL) {
				DUMP_P("[%d] Node: kw=NULL, size=%ld", n, kArray_size(expr->NodeList));
				DBG_ASSERT(IS_Array(expr->NodeList));
			}
			else {
				DUMP_P("[%d] Node: kw='%s%s', syn=%p, size=%ld", n, KSymbol_Fmt2(expr->syn->keyword), expr->syn, kArray_size(expr->NodeList));
				DUMP_P("\n");
				size_t i;
				for(i=0; i < kArray_size(expr->NodeList); i++) {
					kObject *o = expr->NodeList->ObjectItems[i];
					if(IS_Node(o)) {
						dumpExpr(kctx, i, nest+1, (kNode *)o);
					}
					else {
						dumpIndent(kctx, nest+1);
						if(kObject_class(o) == KClass_Token) {
							kToken *tk = (kToken *)o;
							DUMP_P("[%d] O: %s ", i, KClass_text(o->h.ct));
							dumpToken(kctx, tk, -1);
						}
						else if(o == K_NULL) {
							DUMP_P("[%d] O: null\n", i);
						}
						else {
							DUMP_P("[%d] O: %s\n", i, KClass_text(o->h.ct));
						}
					}
				}
			}
		}
	}
}


#endif

