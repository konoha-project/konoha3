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


#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>


static kbool_t closure_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	return true;
}

static kbool_t closure_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

/* Tokenutils_skipindent(
 * copied from ast.h
 */
static int TokenUtils_skipIndent(kArray *tokenList, int currentIdx, int endIdx)
{
	for(; currentIdx < endIdx; currentIdx++) {
		kToken *tk = tokenList->tokenItems[currentIdx];
		if(kToken_is(StatementSeparator, tk)) continue;
		if(!kToken_isIndent(tk)) break;
	}
	return currentIdx;
}

/* kStmt_newMethodParamNULL(
 * copied from sugarfunc.h
 */
static kParam *kStmt_newMethodParamNULL(KonohaContext *kctx, kStmt *stmt, kGamma* gma)
{
	kParam *pa = (kParam*)kStmt_getObjectNULL(kctx, stmt, KW_ParamPattern);
	if(pa == NULL || !IS_Param(pa)) {
		SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), KW_ParamPattern);
		if (!SUGAR SugarSyntax_tyCheckStmt(kctx, syn, stmt, gma)) {
			return NULL;
		}
	}
	pa = (kParam*)kStmt_getObjectNULL(kctx, stmt, KW_ParamPattern);
	DBG_ASSERT(IS_Param(pa));
	return pa;
}

/* TokenSequence_checkCStyleParam
 * copied from sugarfunc.h
 */
static void TokenSequence_checkCStyleParam(KonohaContext *kctx, TokenSequence* tokens)
{
	int i;
	for(i = 0; i < tokens->endIdx; i++) {
		kTokenVar *tk = tokens->tokenList->tokenVarItems[i];
		if(tk->resolvedSyntaxInfo->keyword == KW_void) {
			tokens->endIdx = i; //  f(void) = > f()
			return;
		}
		if(tk->resolvedSyntaxInfo->keyword == KW_COMMA) {
			kToken_set(StatementSeparator, tk, true);
		}
	}
}

/* Stmttypedecl_setparam
 * copied from sugarfunc.h
 */
static kbool_t StmtTypeDecl_setParam(KonohaContext *kctx, kStmt *stmt, int n, kparamtype_t *p)
{
	kToken *tkT  = SUGAR kStmt_getToken(kctx, stmt, KW_TypePattern, NULL);
	kExpr  *expr = SUGAR kStmt_getExpr(kctx, stmt, KW_ExprPattern, NULL);
	DBG_ASSERT(tkT != NULL);
	DBG_ASSERT(expr != NULL);
	if(Expr_isSymbolTerm(expr)) {
		kToken *tkN = expr->termToken;
		ksymbol_t fn = ksymbolA(S_text(tkN->text), S_size(tkN->text), SYM_NEWID);
		p[n].fn = fn;
		p[n].ty = Token_typeLiteral(tkT);
		return true;
	}
	return false;
}

static KMETHOD Expression_Closure(KonohaContext *kctx, KonohaStack *sfp)
{
	/* Closure Expression
	 * potentially exists indent token in between each tokens
	 * beginIdx   : "function"
	 * beginIdx+1 : $Param
	 * beginIdx+2 : "=>"
	 * beginIdx+3 : $Type, means return type
	 * beginIdx+4 : $Block
	 */
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	int nextIdx = TokenUtils_skipIndent(tokenList, beginIdx+1, kArray_size(tokenList));
	kToken* tk = tokenList->tokenItems[nextIdx];
	if (tk->resolvedSyntaxInfo->keyword != KW_ParenthesisGroup) {
		RETURN_(kStmt_printMessage(kctx, stmt, ErrTag, "expected parameter after 'function'"));
	}
	kArray *paramTokenList = tk->subTokenList;
	TokenSequence param = {Stmt_nameSpace(stmt), paramTokenList, 0, kArray_size(paramTokenList)};
	TokenSequence_checkCStyleParam(kctx, &param);
	kBlock *bk = SUGAR new_kBlock(kctx, stmt, NULL, &param); /* GCSAFE */

	/* parsing parameter of closure function */
	size_t i, psize = (bk->stmtList) ? kArray_size(bk->stmtList) : 0;
	kparamtype_t *p = ALLOCA(kparamtype_t, psize);
	for (i = 0; i < psize; i++) {
		p[i].ty = TY_void; p[i].fn = 0;
		kStmt *stmtParam = bk->stmtList->stmtItems[i];
		if (stmtParam->syn->keyword != KW_TypeDeclPattern || !StmtTypeDecl_setParam(kctx, stmtParam, i, p)) {
			RETURN_(kStmt_printMessage(kctx, stmt, ErrTag, "invalid parameter"));
			break;
		}
	}
	kParam *pa = new_kParam(kctx, TY_void, psize, p); /* GCSAFE */
	//KLIB kObject_setObject(kctx, stmt, KW_ParamPattern, TY_Param, pa);

	/* checking return type */
	nextIdx = TokenUtils_skipIndent(tokenList, nextIdx+1, kArray_size(tokenList));
	tk = tokenList->tokenItems[nextIdx];
	if (strcmp(tk->text->text, "=>") != 0) {
		RETURN_(kStmt_printMessage(kctx, stmt, ErrTag, "expected '=>' token after closure parameter"));
	}
	nextIdx = TokenUtils_skipIndent(tokenList, nextIdx+1, kArray_size(tokenList));
	KonohaClass *retClass = NULL;
	nextIdx = SUGAR TokenUtils_parseTypePattern(kctx, Stmt_nameSpace(stmt), tokenList, nextIdx, kArray_size(tokenList), &retClass);
	if (retClass) {
		nextIdx = TokenUtils_skipIndent(tokenList, nextIdx+1, kArray_size(tokenList));
		TokenSequence param = {Stmt_nameSpace(stmt), tokenList, nextIdx, kArray_size(tokenList)};
		kBlock *bkBody = SUGAR new_kBlock(kctx, stmt, NULL, &param); /* GCSAFE */
		//KLIB kObject_setObject(kctx, stmt, KW_TypePattern, retClass->typeId, retClass);
		//KLIB kObject_setObject(kctx, stmt, KW_BlockPattern, retClass->typeId, bkBody);
		SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), SYM_("function"));
		kExpr *expr = SUGAR new_UntypedCallStyleExpr(kctx, stmt->syn, 3, pa, retClass, bkBody);
		RETURN_(expr);
	} else {
		RETURN_(kStmt_printMessage(kctx, stmt, ErrTag, "expected return type after '=>' token"));
	}

}

static KMETHOD TypeCheck_Closure(KonohaContext *kctx, KonohaStack *sfp)
{
	printf("hihioh;sjdf;lkajsdf;j\n");
	asm("int3");
}

#define PATTERN(T)    .keyword = KW_##T##Pattern
static kbool_t closure_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("function"), 0, NULL/*"\"function\" $Param \"=>\" $Type $Block"*/, 0, 0, NULL, Expression_Closure, NULL, NULL, TypeCheck_Closure, },
		//{ SYM_("$ClosureDecl"), 0, "$ClosureDecl $Param \"=>\" $Type $Block", 0, 0, PatternMatch_Closure, NULL, NULL, Statement_closure, NULL, },
		{ KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNameSpace);
	return true;
}

static kbool_t closure_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* closure_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSETPACKNAME(d, "closure", "1.0");
	d.initPackage = closure_initPackage;
	d.setupPackage = closure_setupPackage;
	d.initNameSpace = closure_initNameSpace;
	d.setupNameSpace = closure_setupNameSpace;
	return &d;
}