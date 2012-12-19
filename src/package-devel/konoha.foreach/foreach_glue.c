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
#include <minikonoha/konoha_common.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------

#define KType_IsIterator(T)     (KClass_(T)->baseTypeId == KType_Iterator)

static kToken* new_TypeToken(KonohaContext *kctx, kNameSpace *ns, KClass *kclass)
{
	kToken *TypeToken = new_(Token, 0, OnGcStack);
	kToken_SetTypeId(kctx, TypeToken, ns, kclass->typeId);
	return TypeToken;
}

static kToken* new_ParsedExprToken(KonohaContext *kctx, kNameSpace *ns, kExpr *expr)
{
	kTokenVar *ParsedExprToken = new_(TokenVar, 0, OnGcStack);
	ParsedExprToken->resolvedSyntaxInfo = KSyntax_(ns, KSymbol_ExprPattern);
	KFieldSet(ParsedExprToken, ParsedExprToken->parsedExpr, expr);
	return (kToken *)ParsedExprToken;
}

static void KMacroSet_setTokenAt(KonohaContext *kctx, KMacroSet *macroSet, int index, kArray *tokenList, const char *symbol, ...)
{
	DBG_ASSERT(macroSet[index].tokenList == NULL);
	macroSet[index].symbol = KLIB Ksymbol(kctx, symbol, strlen(symbol), StringPolicy_TEXT|StringPolicy_ASCII, _NEWID);
	macroSet[index].tokenList = tokenList;
	macroSet[index].beginIdx = kArray_size(tokenList);
	kToken *tk;
	va_list ap;
	va_start(ap , symbol);
	while((tk = va_arg(ap, kToken *)) != NULL) {
		DBG_ASSERT(IS_Token(tk));
		KLIB kArray_Add(kctx, tokenList, tk);
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
	kNameSpace *ns = Stmt_ns(stmt);
	KTokenSeq source = {ns, KGetParserContext(kctx)->preparedTokenList};
	KTokenSeq_Push(kctx, source);
	/* FIXME(imasahiro)
	 * we need to implement template as Block
	 * "T _ = E; if(_.hasNext()) { N = _.next(); }"
	 *                           ^^^^^^^^^^^^^^^^^
	 */
	SUGAR KTokenSeq_Tokenize(kctx, &source, "T _ = E; if(_.hasNext()) N = _.next();", 0);
	KMacroSet macroSet[4] = {{0, NULL, 0, 0}};
	KMacroSet_setTokenAt(kctx, macroSet, 0, source.tokenList, "T", IteratorTypeToken, NULL);
	KMacroSet_setTokenAt(kctx, macroSet, 1, source.tokenList, "E", IteratorExprToken, NULL);
	if(TypeToken == NULL) {
		KMacroSet_setTokenAt(kctx, macroSet, 2, source.tokenList, "N", new_TypeToken(kctx, ns, KClass_INFER), VariableToken, NULL);
	}
	else {
		KMacroSet_setTokenAt(kctx, macroSet, 2, source.tokenList, "N", TypeToken, VariableToken, NULL);
	}
	kBlock *bk = SUGAR new_kBlock(kctx, stmt, macroSet, &source);
	KTokenSeq_Pop(kctx, source);
	return bk;
}

static void kStmt_appendBlock(KonohaContext *kctx, kStmt *stmt, kBlock *bk)
{
	if(bk != NULL) {
		kBlock *block = SUGAR kStmt_GetBlock(kctx, stmt, Stmt_ns(stmt), KSymbol_BlockPattern, NULL);
		size_t i;
		for(i = 0; i < kArray_size(bk->NodeList); i++) {
			KLIB kArray_Add(kctx, block->NodeList, bk->NodeList->StmtItems[i]);
		}
	}
}

static KMETHOD Statement_for(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	DBG_P("for statement .. ");
	int isOkay = false;
	if(SUGAR kStmt_TypeCheckByName(kctx, stmt, KSymbol_ExprPattern, gma, KClass_INFER, 0)) {
		kNameSpace *ns = Stmt_ns(stmt);
		kToken *TypeToken = SUGAR kStmt_GetToken(kctx, stmt, KSymbol_TypePattern, NULL);
		kToken *VariableToken  = SUGAR kStmt_GetToken(kctx, stmt, KSymbol_SymbolPattern, NULL);
		DBG_P("typeToken=%p, varToken=%p", TypeToken, VariableToken);
		kExpr *IteratorExpr = SUGAR kStmt_GetExpr(kctx, stmt, KSymbol_ExprPattern, NULL);
		if(!KType_IsIterator(IteratorExpr->attrTypeId)) {
			kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, KClass_(IteratorExpr->attrTypeId), KMethodName_To(KType_Iterator), 0, KMethodMatch_NoOption);
			if(mtd == NULL) {
				kStmtExpr_Message(kctx, stmt, IteratorExpr, ErrTag, "expected Iterator expression after in");
				KReturnUnboxValue(false);
			}
			IteratorExpr = SUGAR new_TypedCallExpr(kctx, stmt, gma, KClass_INFER, mtd, 1, IteratorExpr);
			kStmt_SetObject(kctx, stmt, KSymbol_ExprPattern, IteratorExpr);
		}
		kBlock *block = new_MacroBlock(kctx, stmt, new_TypeToken(kctx, ns, KClass_(IteratorExpr->attrTypeId)), new_ParsedExprToken(kctx, ns, IteratorExpr), TypeToken, VariableToken);
		kStmt *IfStmt = block->NodeList->StmtItems[1]; // @see macro;
		kStmt_appendBlock(kctx, IfStmt, SUGAR kStmt_GetBlock(kctx, stmt, ns, KSymbol_BlockPattern, NULL));
		kStmt_Set(CatchBreak, IfStmt, true);
		kStmt_Set(CatchContinue, IfStmt, true);
		isOkay = SUGAR kBlock_TypeCheckAll(kctx, block, gma);
		if(isOkay) {
			kStmt_typed(IfStmt, LOOP);
			kStmt_SetObject(kctx, stmt, KSymbol_BlockPattern, block);
			kStmt_typed(stmt, BLOCK);
		}
	}
	KReturnUnboxValue(isOkay);
}

static kbool_t foreach_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("for"), 0, "\"for\" \"(\" [$Type] $Symbol \"in\" $Expr  \")\" [$Block] ", 0, 0, NULL, NULL, NULL, Statement_for, NULL, },
		{ KSymbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}

static kbool_t foreach_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequirePackage("konoha.iterator", trace);
	KImportPackageSymbol(ns, "cstyle", "break", trace);
	KImportPackageSymbol(ns, "cstyle", "continue", trace);
	foreach_defineSyntax(kctx, ns, trace);
	return true;
}

static kbool_t foreach_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}


KDEFINE_PACKAGE *foreach_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", "1.0");
	d.PackupNameSpace    = foreach_PackupNameSpace;
	d.ExportNameSpace   = foreach_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
