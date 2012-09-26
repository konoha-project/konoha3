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
		if (n < 0) n = (short)tk->uline;
		if(tk->resolvedSyntaxInfo == NULL) {
			if(tk->unresolvedTokenType == TokenType_INDENT) {
				DUMP_P("Token[%d] '%s' TokenType=%s%s indent=%d\n", n, Token_text(tk), PSYM_t(tk->unresolvedTokenType), tk->indent);
			}
			else {
				DUMP_P("Token[%d] '%s' TokenType=``%s%s''\n", n, Token_text(tk), PSYM_t(tk->unresolvedTokenType));
			}
		}
//		else if(Token_isRule(tk)) {
//			DUMP_P("RuleToken(%d) '%s' resolvedSymbol=%s%s classNameSymbol=%s%s\n", n, Token_text(tk), PSYM_t(tk->resolvedSymbol), PSYM_t(tk->indent));
//		}
		else if(tk->resolvedSyntaxInfo->keyword == KW_TypePattern) {
			DUMP_P("Token(%d) '%s' type=%s\n", n, Token_text(tk), TY_t(tk->resolvedTypeId));
		}
		else {
			DUMP_P("Token(%d) '%s' syntax=%s%s, symbol=``%s%s''\n", n, Token_text(tk), PSYM_t(tk->resolvedSyntaxInfo->keyword), PSYM_t(tk->resolvedSymbol));
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
	case KW_ParenthesisGroup: return '(';
	case KW_BraceGroup: return '{';
	case KW_BracketGroup: return '[';
	}
	return '<';
}

static int dumpEndTokenList(int closure)
{
	switch(closure) {
	case KW_ParenthesisGroup: return ')';
	case KW_BraceGroup: return '}';
	case KW_BracketGroup: return ']';
	}
	return '>';
}

static void dumpTokenArray(KonohaContext *kctx, int nest, kArray *a, int s, int e)
{
	if(verbose_sugar) {
		while(s < e) {
			kToken *tk = a->tokenItems[s];
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

static void dumpExpr(KonohaContext *kctx, int n, int nest, kExpr *expr)
{
	DBG_ASSERT(IS_Expr(expr));
	if(verbose_sugar) {
		dumpIndent(kctx, nest);
		if(expr == K_NULLEXPR) {
			DUMP_P("[%d] NullObject", n);
		}
		else if(Expr_isTerm(expr)) {
			DUMP_P("[%d] TermToken: ", n);
			dumpToken(kctx, expr->termToken, -1);
		}
		else {
			int i;
			if(expr->syn == NULL) {
				DUMP_P("[%d] Expr: kw=NULL, size=%ld", n, kArray_size(expr->cons));
				DBG_ASSERT(IS_Array(expr->cons));
			}
			else {
				DUMP_P("[%d] Expr: kw='%s%s', syn=%p, size=%ld", n, PSYM_t(expr->syn->keyword), expr->syn, kArray_size(expr->cons));
				DUMP_P("\n");
				for(i=0; i < kArray_size(expr->cons); i++) {
					kObject *o = expr->cons->objectItems[i];
					if(IS_Expr(o)) {
						dumpExpr(kctx, i, nest+1, (kExpr*)o);
					}
					else {
						dumpIndent(kctx, nest+1);
						if(O_ct(o) == CT_Token) {
							kToken *tk = (kToken*)o;
							DUMP_P("[%d] O: %s ", i, CT_t(o->h.ct));
							dumpToken(kctx, tk, -1);
						}
						else if(o == K_NULL) {
							DUMP_P("[%d] O: null\n", i);
						}
						else {
							DUMP_P("[%d] O: %s\n", i, CT_t(o->h.ct));
						}
					}
				}
			}
		}
	}
}

static void dumpEntry(KonohaContext *kctx, void *arg, KUtilsKeyValue *d)
{
	if((d->key & SYMKEY_BOXED) == SYMKEY_BOXED) {
		ksymbol_t key = ~SYMKEY_BOXED & d->key;
		DUMP_P("key='%s%s': ", PSYM_t(key));
		if(IS_Token(d->objectValue)) {
			dumpToken(kctx, (kToken*)d->objectValue, -1);
		} else if (IS_Expr(d->objectValue)) {
			dumpExpr(kctx, 0, 0, (kExpr *) d->objectValue);
		}
		else {
			DUMP_P("ObjectType %s\n", CT_t(O_ct(d->objectValue)));
		}
	}
}

static void dumpStmt(KonohaContext *kctx, kStmt *stmt)
{
	if(verbose_sugar) {
		if(stmt->syn == NULL) {
			DUMP_P("STMT (DONE)\n");
		}
		else {
			DUMP_P("STMT %s%s {\n", T_statement(stmt->syn->keyword));
			KLIB kObject_protoEach(kctx, stmt, NULL, dumpEntry);
			DUMP_P("\n}\n");
		}
	}
}

#endif

