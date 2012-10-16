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
#include <minikonoha/iterator.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------

static kbool_t foreach_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	return true;
}

static kbool_t foreach_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	return true;
}

#define TY_isIterator(T)     (CT_(T)->baseTypeId == TY_Iterator)

static kToken* new_TypeToken(KonohaContext *kctx, kNameSpace *ns, ktype_t typeId)
{
	kToken *TypeToken = new_(Token, 0, OnGcStack);
	kToken_setTypeId(kctx, TypeToken, ns, typeId);
	return TypeToken;
}

static kToken* new_ParsedExprToken(KonohaContext *kctx, kNameSpace *ns, kExpr *expr)
{
	kTokenVar *ParsedExprToken = new_(TokenVar, 0, OnGcStack);
	ParsedExprToken->resolvedSyntaxInfo = SYN_(ns, KW_ExprPattern);
	KFieldSet(ParsedExprToken, ParsedExprToken->parsedExpr, expr);
	return (kToken*)ParsedExprToken;
}

static void MacroSet_setTokenAt(KonohaContext *kctx, MacroSet *macroSet, int index, kArray *tokenList, const char *symbol, ...)
{
	DBG_ASSERT(macroSet[index].tokenList == NULL);
	macroSet[index].symbol = KLIB Ksymbol(kctx, symbol, strlen(symbol), StringPolicy_TEXT|StringPolicy_ASCII, _NEWID);
	macroSet[index].tokenList = tokenList;
	macroSet[index].beginIdx = kArray_size(tokenList);
	kToken *tk;
	va_list ap;
	va_start(ap , symbol);
	while((tk = va_arg(ap, kToken*)) != NULL) {
		DBG_ASSERT(IS_Token(tk));
		KLIB kArray_add(kctx, tokenList, tk);
	}
	va_end(ap);
	macroSet[index].endIdx = kArray_size(tokenList);
}

/* This implementation is a little tricky (by kimio)
 * The syntax of loop is defined as if statement
 * Typechecking is overloaded as while statement (@see while_glue);
 */

static kBlock *new_MacroBlock(KonohaContext *kctx, kStmt *stmt, kToken *IteratorTypeToken, kToken *IteratorExprToken, kToken *TypeToken, kToken *VariableToken)
{
	kNameSpace *ns = Stmt_nameSpace(stmt);
	TokenSequence source = {ns, KonohaContext_getSugarContext(kctx)->preparedTokenList};
	TokenSequence_push(kctx, source);
	/* FIXME(imasahiro)
	 * we need to implement template as Block
	 * "T _ = E; if(_.hasNext()) { N = _.next(); }"
	 *                           ^^^^^^^^^^^^^^^^^
	 */
	SUGAR TokenSequence_tokenize(kctx, &source, "T _ = E; if(_.hasNext()) N = _.next();", 0);
	MacroSet macroSet[4] = {{0, NULL, 0, 0}};
	MacroSet_setTokenAt(kctx, macroSet, 0, source.tokenList, "T", IteratorTypeToken, NULL);
	MacroSet_setTokenAt(kctx, macroSet, 1, source.tokenList, "E", IteratorExprToken, NULL);
	if(TypeToken == NULL) {
		MacroSet_setTokenAt(kctx, macroSet, 2, source.tokenList, "N", VariableToken, NULL);
	}
	else {
		MacroSet_setTokenAt(kctx, macroSet, 2, source.tokenList, "N", TypeToken, VariableToken, NULL);
	}
	kBlock *bk = SUGAR new_kBlock(kctx, stmt, macroSet, &source);
	TokenSequence_pop(kctx, source);
	return bk;
}

static void kStmt_appendBlock(KonohaContext *kctx, kStmt *stmt, kBlock *bk)
{
	if(bk != NULL) {
		kBlock *block = SUGAR kStmt_getBlock(kctx, stmt, Stmt_nameSpace(stmt), KW_BlockPattern, NULL);
		int i;
		for(i = 0; i < kArray_size(bk->StmtList); i++) {
			KLIB kArray_add(kctx, block->StmtList, bk->StmtList->StmtItems[i]);
		}
	}
}

static KMETHOD Statement_for(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	DBG_P("for statement .. ");
	int isOkay = false;
	if(SUGAR kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_var, 0)) {
		kNameSpace *ns = Stmt_nameSpace(stmt);
		kToken *TypeToken = SUGAR kStmt_getToken(kctx, stmt, KW_TypePattern, NULL);
		kToken *VariableToken  = SUGAR kStmt_getToken(kctx, stmt, KW_SymbolPattern, NULL);
		DBG_P("typeToken=%p, varToken=%p", TypeToken, VariableToken);
		kExpr *IteratorExpr = SUGAR kStmt_getExpr(kctx, stmt, KW_ExprPattern, NULL);
		if(!TY_isIterator(IteratorExpr->ty)) {
			kMethod *mtd = KLIB kNameSpace_getMethodByParamSizeNULL(kctx, ns, IteratorExpr->ty, MN_to(TY_Iterator), 0);
			if(mtd == NULL) {
				kStmtExpr_printMessage(kctx, stmt, IteratorExpr, ErrTag, "expected Iterator expression after in");
				KReturnUnboxValue(false);
			}
			IteratorExpr = SUGAR new_TypedCallExpr(kctx, stmt, gma, TY_var, mtd, 1, IteratorExpr);
			kStmt_setObject(kctx, stmt, KW_ExprPattern, IteratorExpr);
		}
		kBlock *block = new_MacroBlock(kctx, stmt, new_TypeToken(kctx, ns, IteratorExpr->ty), new_ParsedExprToken(kctx, ns, IteratorExpr), TypeToken, VariableToken);
		kStmt *IfStmt = block->StmtList->StmtItems[1]; // @see macro;
		kStmt_appendBlock(kctx, IfStmt, SUGAR kStmt_getBlock(kctx, stmt, ns, KW_BlockPattern, NULL));
		kStmt_set(CatchBreak, IfStmt, true);
		kStmt_set(CatchContinue, IfStmt, true);
		isOkay = SUGAR kBlock_tyCheckAll(kctx, block, gma);
		if(isOkay) {
			kStmt_typed(IfStmt, LOOP);
			kStmt_setObject(kctx, stmt, KW_BlockPattern, block);
			kStmt_typed(stmt, BLOCK);
		}
	}
	KReturnUnboxValue(isOkay);
}

static kbool_t foreach_initNameSpace(KonohaContext *kctx, kNameSpace *packageNS, kNameSpace *ns, KTraceInfo *trace)
{
	KImportPackage(ns, "konoha.iterator", trace);
	KImportPackage(ns, "konoha.break", trace);
	KImportPackage(ns, "konoha.continue", trace);
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("for"), 0, "\"for\" \"(\" [$Type] $Symbol \"in\" $Expr  \")\" [$Block] ", 0, 0, NULL, NULL, NULL, Statement_for, NULL, },
		{ KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNS);
	return true;
}

static kbool_t foreach_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNS, kNameSpace *ns, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* foreach_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "foreach", "1.0");
	d.initPackage    = foreach_initPackage;
	d.setupPackage   = foreach_setupPackage;
	d.initNameSpace  = foreach_initNameSpace;
	d.setupNameSpace = foreach_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
