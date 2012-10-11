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

static ktype_t kStmt_getClassId(KonohaContext *kctx, kStmt *stmt, kNameSpace *ns, ksymbol_t kw, ktype_t defcid)
{
	kToken *tk = (kToken*)kStmt_getObjectNULL(kctx, stmt, kw);
	if(tk == NULL || !IS_Token(tk)) {
		return defcid;
	}
	else {
		DBG_ASSERT(Token_isVirtualTypeLiteral(tk));
		return Token_typeLiteral(tk);
	}
}

static ksymbol_t kStmt_getMethodSymbol(KonohaContext *kctx, kStmt *stmt, kNameSpace *ns, ksymbol_t kw, kmethodn_t defmn)
{
	kToken *tk = (kToken*)kStmt_getObjectNULL(kctx, stmt, kw);
	if(tk == NULL || !IS_Token(tk) || !IS_String(tk->text)) {
		return defmn;
	}
	else {
		return tk->resolvedSymbol;
	}
}

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

static int addGammaStack(KonohaContext *kctx, GammaStack *s, ktype_t ty, ksymbol_t fn)
{
	int index = s->varsize;
	if(!(s->varsize < s->capacity)) {
		s->capacity *= 2;
		size_t asize = sizeof(GammaStackDecl) * s->capacity;
		GammaStackDecl *v = (GammaStackDecl*)KMALLOC(asize);
		memcpy(v, s->varItems, asize/2);
		if(s->allocsize > 0) {
			KFREE(s->varItems, s->allocsize);
		}
		s->varItems = v;
		s->allocsize = asize;
	}
	DBG_P("index=%d, ty=%s fn=%s", index, TY_t(ty), SYM_t(fn));
	s->varItems[index].ty = ty;
	s->varItems[index].fn = fn;
	s->varsize += 1;
	return index;
}

static int TokenUtils_skipIndent(kArray *tokenList, int currentIdx, int endIdx)
{
	for(; currentIdx < endIdx; currentIdx++) {
		kToken *tk = tokenList->tokenItems[currentIdx];
		if(kToken_is(StatementSeparator, tk)) continue;
		if(!kToken_isIndent(tk)) break;
	}
	return currentIdx;
}

static KMETHOD PatternMatch_Closure(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	kNameSpace *ns = Stmt_nameSpace(stmt);
	KonohaClass *foundClass = NULL;
	int nextIdx = SUGAR TokenUtils_parseTypePattern(kctx, ns, tokenList, beginIdx, endIdx, &foundClass);
	//DBG_P("@ nextIdx = %d < %d", nextIdx, endIdx);
	if(nextIdx != -1) {
		nextIdx = TokenUtils_skipIndent(tokenList, nextIdx, endIdx);
		if(nextIdx < endIdx) {
			kToken *tk = tokenList->tokenItems[nextIdx];
			if(SUGAR TokenUtils_parseTypePattern(kctx, ns, tokenList, nextIdx, endIdx, NULL) != -1) {
				RETURNi_(beginIdx);
			}
			if(tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern) {
				int symbolNextIdx = TokenUtils_skipIndent(tokenList, nextIdx + 1, endIdx);
				if(symbolNextIdx < endIdx && tokenList->tokenItems[symbolNextIdx]->resolvedSyntaxInfo->keyword == KW_ParenthesisGroup) {
					RETURNi_(beginIdx);
				}
				RETURNi_(-1);
			}
			if(tk->resolvedSyntaxInfo->keyword != KW_DOT && ((tk->resolvedSyntaxInfo->precedence_op1 > 0 || tk->resolvedSyntaxInfo->precedence_op2 > 0))) {
				RETURNi_(beginIdx);
			}
		}
	}
	RETURNi_(-1);
}

static KMETHOD Statement_closure(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	//uintptr_t flag    = kStmt_parseFlag(kctx, stmt, MethodDeclFlag, 0);
	uintptr_t flag    = 0;
	int ret = false;
	kNameSpace *ns    = Stmt_nameSpace(stmt);
	ktype_t typeId   = kStmt_getClassId(kctx, stmt, ns, SYM_("ClassName"), O_typeId(ns));
	kmethodn_t mn     = kStmt_getMethodSymbol(kctx, stmt, ns, KW_SymbolPattern, MN_new);
	kParam *pa        = kStmt_newMethodParamNULL(kctx, stmt, gma);
	if(pa != NULL) {  // if pa is NULL, error is printed out.
		kMethod *mtd = KLIB new_kMethod(kctx, flag, typeId, mn, NULL);
		KLIB kObject_setObject(kctx, stmt, SYM_("Method"), TY_Method, mtd);
		KLIB kMethod_setParam(kctx, mtd, pa->rtype, pa->psize, (kparamtype_t*)pa->paramtypeItems);
		kMethod *oldMethod = gma->genv->currentWorkingMethod;
		gma->genv->currentWorkingMethod = mtd;
		//printf("rtype: %zd psize: %zd\n", pa->rtype, pa->psize);
		KonohaClass *ct = CT_(mtd->typeId);
		kToken *blockToken = (kToken*)kStmt_getObjectNULL(kctx, stmt, KW_BlockPattern);

		kToken *tkSymbol = (kToken*)kStmt_getObjectNULL(kctx, stmt, KW_SymbolPattern);
		//printf("resolvedsymbol %zd\n", tkSymbol->resolvedSymbol);
		kExpr *exprTerm = SUGAR new_UntypedTermExpr(kctx, tkSymbol);
		KonohaClass *ctFunc = KLIB KonohaClass_Generics(kctx, CT_Func, pa->rtype, pa->psize, (kparamtype_t*)pa->paramtypeItems);
		kFuncVar *fo = (kFuncVar*)KLIB new_kObjectOnGCSTACK(kctx, ctFunc, (uintptr_t)mtd);
		KFieldSet(fo, fo->self, UPCAST(ns));
		kExpr *exprFunc =  new_ConstValueExpr(kctx, ctFunc->typeId, UPCAST(fo));
		kExpr *exprMtd =  new_ConstValueExpr(kctx, ctFunc->typeId, UPCAST(mtd));
		kExpr *exprDecl = SUGAR new_UntypedCallStyleExpr(kctx, SYN_(Stmt_nameSpace(stmt), KW_LET), 3, exprTerm, exprTerm, exprFunc);
		kStmt *newstmt = stmt;

		if (SUGAR kStmt_declType(kctx, stmt, gma, ctFunc->typeId, exprDecl, NULL, NULL, &newstmt)) {
			KLIB kObject_setObject(kctx, stmt, SYM_("Expr"), TY_Expr, exprDecl);
			//printf("hi\n");
		}

		if (blockToken != NULL && blockToken->resolvedSyntaxInfo->keyword == KW_BlockPattern) {
			TokenSequence range = {Stmt_nameSpace(stmt), KonohaContext_getSugarContext(kctx)->preparedTokenList};
			TokenSequence_push(kctx, range);
			SUGAR TokenSequence_tokenize(kctx, &range, S_text(blockToken->text), blockToken->uline);
			kBlock *bk = SUGAR new_kBlock(kctx, stmt, &range, NULL);
			ret = SUGAR kBlock_tyCheckAll(kctx, bk, gma);
			if (ret) {
				((kStmtVar*)stmt)->syn = SYN_(Stmt_nameSpace(stmt), KW_ParamPattern);
				KLIB kObject_setObject(kctx, stmt, SYM_("Block"), TY_Block, bk);
				kStmt_typed(stmt, CLOSURE);
				gma->genv->currentWorkingMethod = oldMethod;
				RETURNb_(1);
			}
		}
	}
	RETURNb_(0);
}

#define PATTERN(T)    .keyword = KW_##T##Pattern
static kbool_t closure_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ PATTERN(MethodDecl), 0, "$MethodDecl $Type $Symbol $Param $Block", 0, 0, PatternMatch_Closure, NULL, NULL, Statement_closure, NULL, },
		{ KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNameSpace);
	return true;
}

static kbool_t closure_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* closure_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("closure", "1.0"),
		.initPackage = closure_initPackage,
		.setupPackage = closure_setupPackage,
		.initNameSpace = closure_initNameSpace,
		.setupNameSpace = closure_setupNameSpace,
	};
	return &d;
}
