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

#define T_statement(kw)  StatementName(kctx, kw), StatementType(kw)

static const char* StatementName(KonohaContext *kctx, ksymbol_t keyword)
{
	const char *statement = SYM_t(keyword);
	if(keyword == KW_ExprPattern) statement = "expression";
	else if(keyword == KW_StmtTypeDecl) statement = "variable";
	else if(keyword == KW_StmtMethodDecl) statement =  "function";
	return statement;
}

static const char* StatementType(ksymbol_t keyword)
{
	const char *postfix = " statement";
	if(keyword == KW_ExprPattern) postfix = "";
	else if(keyword == KW_StmtTypeDecl || keyword == KW_StmtMethodDecl) postfix = " declaration";
	return postfix;
}

#ifdef USE_SMALLBUILD
#define KdumpTokenArray(CTX, TLS, S, E)
#define KdumpStmt(CTX, STMT)
#define KdumpExpr(CTX, EXPR)

#else
#define KdumpTokenArray(CTX, TLS, S, E) 	dumpTokenArray(CTX, 0, TLS, S, E)
#define KdumpStmt(CTX, STMT) 		        dumpStmt(CTX, STMT)
#define KdumpExpr(CTX, EXPR)                dumpExpr(CTX, 0, 0, EXPR)

/* --------------- */
/* Token */

static void dumpToken(KonohaContext *kctx, kToken *tk)
{
	DUMP_P("kw=%s%s syn=%p '%s' indent(or ty)=%d\n", KW_t(tk->keyword), tk->resolvedSyntaxInfo, Token_text(tk), tk->indent);
}

static void dumpIndent(KonohaContext *kctx, int nest)
{
	int i;
	for(i = 0; i < nest; i++) {
		DUMP_P("  ");
	}
}

static int dumpBeginTokenList(kToken *tk)
{
	switch(tk->keyword) {
	case AST_PARENTHESIS: return '(';
	case AST_BRACE: return '{';
	case AST_BRACKET: return '[';
	}
	return '<';
}

static int dumpEndTokenList(kToken *tk)
{
	switch(tk->keyword) {
	case AST_PARENTHESIS: return ')';
	case AST_BRACE: return '}';
	case AST_BRACKET: return ']';
	}
	return '>';
}

static void dumpTokenArray(KonohaContext *kctx, int nest, kArray *a, int s, int e)
{
	if(verbose_sugar) {
		if(nest == 0) DUMP_P("\n");
		while(s < e) {
			kToken *tk = a->tokenItems[s];
			dumpIndent(kctx, nest);
			if(IS_Array(tk->subTokenList)) {
				DUMP_P("%c\n", dumpBeginTokenList(tk));
				dumpTokenArray(kctx, nest+1, tk->subTokenList, 0, kArray_size(tk->subTokenList));
				dumpIndent(kctx, nest);
				DUMP_P("%c\n", dumpEndTokenList(tk));
			}
			else {
				DUMP_P("TK(%d) ", s);
				dumpToken(kctx, tk);
			}
			s++;
		}
		if(nest == 0) DUMP_P("====\n");
	}
}

static void dumpExpr(KonohaContext *kctx, int n, int nest, kExpr *expr)
{
	if(verbose_sugar) {
		if(nest == 0) DUMP_P("\n");
		dumpIndent(kctx, nest);
		if(expr == K_NULLEXPR) {
			DUMP_P("[%d] ExprTerm: null", n);
		}
		else if(Expr_isTerm(expr)) {
			DUMP_P("[%d] ExprTerm: kw='%s%s' %s", n, KW_t(expr->termToken->keyword), Token_text(expr->termToken));
			if(expr->ty != TY_var) {

			}
			DUMP_P("\n");
		}
		else {
			int i;
			if(expr->syn == NULL) {
				DUMP_P("[%d] Cons: kw=NULL, size=%ld", n, kArray_size(expr->cons));
			}
			else {
				DUMP_P("[%d] Cons: kw='%s%s', size=%ld", n, KW_t(expr->syn->keyword), kArray_size(expr->cons));
			}
			if(expr->ty != TY_var) {

			}
			DUMP_P("\n");
			for(i=0; i < kArray_size(expr->cons); i++) {
				kObject *o = expr->cons->objectItems[i];
				if(O_ct(o) == CT_Expr) {
					dumpExpr(kctx, i, nest+1, (kExpr*)o);
				}
				else {
					dumpIndent(kctx, nest+1);
					if(O_ct(o) == CT_Token) {
						kToken *tk = (kToken*)o;
						DUMP_P("[%d] O: %s ", i, CT_t(o->h.ct));
						dumpToken(kctx, tk);
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

static void dumpEntry(KonohaContext *kctx, void *arg, KUtilsKeyValue *d)
{
	if((d->key & SYMKEY_BOXED) == SYMKEY_BOXED) {
		ksymbol_t key = ~SYMKEY_BOXED & d->key;
		DUMP_P("key='%s%s': ", KW_t(key));
		if(IS_Token(d->objectValue)) {
			dumpToken(kctx, (kToken*)d->objectValue);
		} else if (IS_Expr(d->objectValue)) {
			dumpExpr(kctx, 0, 0, (kExpr *) d->objectValue);
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

