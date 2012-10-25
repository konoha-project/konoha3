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
		kToken *tk = tokenList->TokenItems[currentIdx];
		if(kToken_is(StatementSeparator, tk)) continue;
		if(!kToken_isIndent(tk)) break;
	}
	return currentIdx;
}

/* TokenSequence_checkCStyleParam
 * copied from sugarfunc.h
 */
static void TokenSequence_checkCStyleParam(KonohaContext *kctx, TokenSequence* tokens)
{
	int i;
	for(i = 0; i < tokens->endIdx; i++) {
		kTokenVar *tk = tokens->tokenList->TokenVarItems[i];
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
	kToken* tk = tokenList->TokenItems[nextIdx];
	if (tk->resolvedSyntaxInfo->keyword != KW_ParenthesisGroup) {
		KReturn(kStmt_printMessage(kctx, stmt, ErrTag, "expected parameter after 'function'"));
	}
	kArray *paramTokenList = tk->subTokenList;
	TokenSequence param = {Stmt_nameSpace(stmt), paramTokenList, 0, kArray_size(paramTokenList)};
	TokenSequence_checkCStyleParam(kctx, &param);
	kBlock *bk = SUGAR new_kBlock(kctx, stmt, NULL, &param); /* GCSAFE */

	/* parsing parameter of closure function */
	size_t i, psize = (bk->StmtList) ? kArray_size(bk->StmtList) : 0;
	kparamtype_t *p = ALLOCA(kparamtype_t, psize);
	for (i = 0; i < psize; i++) {
		p[i].ty = TY_void; p[i].fn = 0;
		kStmt *stmtParam = bk->StmtList->StmtItems[i];
		if (stmtParam->syn->keyword != KW_TypeDeclPattern || !StmtTypeDecl_setParam(kctx, stmtParam, i, p)) {
			KReturn(kStmt_printMessage(kctx, stmt, ErrTag, "invalid parameter"));
			break;
		}
	}

	/* checking return type */
	nextIdx = TokenUtils_skipIndent(tokenList, nextIdx+1, kArray_size(tokenList));
	tk = tokenList->TokenItems[nextIdx];
	if (strcmp(tk->text->text, "=>") != 0) {
		KReturn(kStmt_printMessage(kctx, stmt, ErrTag, "expected '=>' token after closure parameter"));
	}
	nextIdx = TokenUtils_skipIndent(tokenList, nextIdx+1, kArray_size(tokenList));
	KonohaClass *retClass = NULL;
	nextIdx = SUGAR TokenUtils_parseTypePattern(kctx, Stmt_nameSpace(stmt), tokenList, nextIdx, kArray_size(tokenList), &retClass);
	if (!retClass) {
		KReturn(kStmt_printMessage(kctx, stmt, ErrTag, "expected return type after '=>' token"));
	}

	kParam *pa = new_kParam(kctx, retClass->typeId, psize, p); /* GCSAFE */

	/* syntax is OK */
	TokenSequence blockSeq = {Stmt_nameSpace(stmt), KonohaContext_getSugarContext(kctx)->preparedTokenList};
	TokenSequence_push(kctx, blockSeq);
	SUGAR TokenSequence_tokenize(kctx, &blockSeq,  S_text(tokenList->TokenItems[nextIdx]->text), tokenList->TokenItems[nextIdx]->uline);
	kBlock *bkBody = SUGAR new_kBlock(kctx, stmt, NULL, &blockSeq); /* GCSAFE */
	TokenSequence_pop(kctx, blockSeq);
	SugarSyntax *synFunc = SYN_(Stmt_nameSpace(stmt), SYM_("function"));
	kExpr *expr = SUGAR new_UntypedCallStyleExpr(kctx, synFunc, 2, pa, bkBody);
	KReturn(expr);

}

static KMETHOD TypeCheck_Closure(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	INIT_GCSTACK();
	kExpr *retExpr = (kExpr*)K_NULL;
	kParam *pa = expr->cons->ParamItems[0];
	kBlock *bk = (kBlock*)expr->cons->ObjectItems[1];
	kMethod *oldMethod = gma->genv->currentWorkingMethod;
	uintptr_t flag = 0;
	kMethod *mtd = KLIB new_kMethod(kctx, _GcStack, flag, 0/*typeId*/, 0/*mn*/, NULL);
	PUSH_GCSTACK2(mtd);
	KLIB kMethod_setParam(kctx, mtd, pa->rtype, pa->psize, (kparamtype_t*)pa->paramtypeItems);
	size_t i;

	/* declare arguments of closure as local variable */
	for (i = 0; i < pa->psize; i++) {
		SUGAR kGamma_declareLocalVariable(kctx, gma, pa->paramtypeItems[i].ty, pa->paramtypeItems[i].fn);
	}
	PUSH_GCSTACK2(oldMethod);

	/* Type checker of a closure expression
	 * now current working method is parent function,
	 * thus we need to push a closure to current working method in order to checking type
	 */
	gma->genv->currentWorkingMethod = mtd;
	int ret = SUGAR kBlock_tyCheckAll(kctx, bk, gma);
	gma->genv->currentWorkingMethod = oldMethod;

	/* type check is OK */
	if (ret) {
		KonohaClass *ctFunc = KLIB KonohaClass_Generics(kctx, CT_Func, pa->rtype, pa->psize, (kparamtype_t*)pa->paramtypeItems);
		kFuncVar *fo = (kFuncVar*)KLIB new_kObject(kctx, _GcStack, ctFunc, (uintptr_t)mtd);
		PUSH_GCSTACK2(fo);
		SUGAR kExpr_setConstValue(kctx, expr, fo->h.ct->typeId, (kObject*)fo);
		kExpr *bkExpr = new_ConstValueExpr(kctx, TY_Block, (kObject*)bk);
		PUSH_GCSTACK2(bk);
		retExpr = SUGAR new_TypedConsExpr(kctx, TEXPR_CLOSURE, fo->h.ct->typeId, 2, expr, bkExpr);
	}
	RESET_GCSTACK();
	KReturn(retExpr);
}

static kbool_t closure_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("function"), 0, NULL/*"\"function\" $Param \"=>\" $Type $Block"*/, Precedence_CStyleASSIGN-1, Precedence_CStyleASSIGN-1, NULL, Expression_Closure, NULL, NULL, TypeCheck_Closure, },
		{ KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX);
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
	KSetPackageName(d, "closure", "1.0");
	d.initPackage = closure_initPackage;
	d.setupPackage = closure_setupPackage;
	return &d;
}
