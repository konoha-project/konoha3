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
#define USE_STRINGLIB
#include <minikonoha/klib.h>
#include <minikonoha/iterator.h>

#ifdef __cplusplus
extern "C" {
#endif


static kbool_t Nothing_hasNext(KonohaContext *kctx, KonohaStack* sfp)
{
	return false;
}

static void Nothing_setNextResult(KonohaContext *kctx, KonohaStack* sfp)
{
	kIterator *itr = (kIterator*)sfp[0].asObject;
	RETURN_(itr->source);
}

static void Nothing_setNextResultUnbox(KonohaContext *kctx, KonohaStack* sfp)
{
	RETURNi_(0);
}

static void Iterator_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kIterator *itr = (kIterator*)o;
	int isUnboxEntry = TY_isUnbox(O_ct(itr)->p0);
	KINITv(itr->source, K_NULL);
	itr->current_pos = 0;
	itr->hasNext = Nothing_hasNext;
	itr->setNextResult = isUnboxEntry ? Nothing_setNextResultUnbox : Nothing_setNextResult;
}

static void Iterator_p(KonohaContext *kctx, KonohaStack *sfp, int pos, KUtilsWriteBuffer *wb, int level)
{
//	KLIB Kwb_printf(kctx, wb, KFLOAT_FMT, sfp[pos].floatValue);
}

/* ------------------------------------------------------------------------ */

static KMETHOD Iterator_hasNext(KonohaContext *kctx, KonohaStack *sfp)
{
	kIterator *itr = sfp[0].itr;
	RETURNb_(itr->hasNext(kctx, sfp));
}

static KMETHOD Iterator_next(KonohaContext *kctx, KonohaStack *sfp)
{
	kIterator *itr = sfp[0].itr;
	itr->setNextResult(kctx, sfp);
}

//static kbool_t callFuncHasNext(KonohaContext *kctx, KonohaStack *sfp)
//{
//	itr->funcHasNext;
//}
//
//static kbool_t callFuncNext(KonohaContext *kctx, KonohaStack *sfp)
//{
//
//}
//
//static KMETHOD Iterator_new(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kIterator *itr = (kIterator*)sfp[0].asObject;
//	KSETv(itr, itr->funcHasNext, sfp[1].fo);
//	KSETv(itr, itr->funcNext,sfp[2].asFunc);
//	itr->hasNext = callFuncHasNext;
//	itr->setNextResult = callFuncNext;
//	RETURN_(itr);
//}

static kbool_t Array_hasNext(KonohaContext *kctx, KonohaStack* sfp)
{
	kIterator *itr = (kIterator*)sfp[0].asObject;
	return (itr->current_pos < kArray_size(itr->arrayList));
}

static void Array_setNextResult(KonohaContext *kctx, KonohaStack* sfp)
{
	kIterator *itr = (kIterator*)sfp[0].asObject;
	size_t n = itr->current_pos;
	itr->current_pos += 1;
	DBG_ASSERT(n < kArray_size(itr->arrayList));
	RETURN_(itr->arrayList->objectItems[n]);
}

static void Array_setNextResultUnbox(KonohaContext *kctx, KonohaStack* sfp)
{
	kIterator *itr = (kIterator*)sfp[0].asObject;
	size_t n = itr->current_pos;
	itr->current_pos += 1;
	DBG_ASSERT(n < kArray_size(itr->arrayList));
	RETURNd_(itr->arrayList->kintItems[n]);
}

static KMETHOD Array_toIterator(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	KonohaClass *cIterator = CT_p0(kctx, CT_Iterator, O_ct(a)->p0);
	kIterator *itr = (kIterator*)KLIB new_kObject(kctx, cIterator, 0);
	KSETv(itr, itr->arrayList, a);
	itr->hasNext = Array_hasNext;
	itr->setNextResult = TY_isUnbox(O_ct(a)->p0) ? Array_setNextResultUnbox : Array_setNextResult;
	RETURN_(itr);
}

#define utf8len(c)    _utf8len[(int)c]

static const char _utf8len[] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 0, 0,
};

static kbool_t String_hasNext(KonohaContext *kctx, KonohaStack* sfp)
{
	kIterator *itr = sfp[0].itr;
	kString *s = (kString*)itr->source;
	return (itr->current_pos < S_size(s));
}

static void String_setNextResult(KonohaContext *kctx, KonohaStack* sfp)
{
	kIterator *itr = sfp[0].itr;
	kString *s = (kString*)itr->source;
	const char *t = S_text(s) + itr->current_pos;
	size_t charsize = utf8len(t[0]);
	itr->current_pos += charsize;
	RETURN_(KLIB new_kString(kctx, t, charsize, (charsize == 1) ? SPOL_ASCII : SPOL_UTF8));
}

static KMETHOD String_toIterator(KonohaContext *kctx, KonohaStack *sfp)
{
	kIterator *itr = (kIterator*)KLIB new_kObject(kctx, CT_StringIterator, 0);
	KSETv(itr, itr->source, sfp[0].asObject);
	itr->hasNext = String_hasNext;
	itr->setNextResult = String_setNextResult;
	RETURN_(itr);
}

/* ------------------------------------------------------------------------ */

static void kmoditerator_setup(KonohaContext *kctx, struct KonohaModule *def, int newctx) {}
static void kmoditerator_reftrace(KonohaContext *kctx, struct KonohaModule *baseh) { }
static void kmoditerator_free(KonohaContext *kctx, struct KonohaModule *baseh) { KFREE(baseh, sizeof(KonohaIteratorModule)); }

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Im       kMethod_Immutable
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

static kbool_t iterator_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KonohaIteratorModule *base = (KonohaIteratorModule*)KCALLOC(sizeof(KonohaIteratorModule), 1);
	base->h.name     = "iterator";
	base->h.setup    = kmoditerator_setup;
	base->h.reftrace = kmoditerator_reftrace;
	base->h.free     = kmoditerator_free;
	KLIB Konoha_setModule(kctx, MOD_iterator, &base->h, pline);

	kparamtype_t IteratorParam = {
		.ty = TY_Object,
	};
	KDEFINE_CLASS defIterator = {
		STRUCTNAME(Iterator),
		.cflag  = CFLAG_Iterator,
		.init   = Iterator_init,
		.p      = Iterator_p,
		.cparamsize  = 1,
		.cparamItems = &IteratorParam,
	};
	base->cIterator = KLIB Konoha_defineClass(kctx, ns->packageId, PN_konoha, NULL, &defIterator, pline);
	base->cStringIterator = CT_p0(kctx, base->cIterator, TY_String);
	base->cGenericIterator = CT_p0(kctx, base->cIterator, TY_0);
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Iterator_hasNext), TY_boolean, TY_Iterator, MN_("hasNext"), 0,
		_Public, _F(Iterator_next), TY_0, TY_Iterator, MN_("next"), 0,
		_Public, _F(Array_toIterator),  base->cGenericIterator->typeId, TY_Array, MN_("toIterator"), 0,
		_Public, _F(String_toIterator), TY_StringIterator, TY_String, MN_("toIterator"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t iterator_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

#define TY_isIterator(T)     (CT_(T)->baseTypeId == TY_Iterator)

static kToken* new_TypeToken(KonohaContext *kctx, kNameSpace *ns, ktype_t typeId)
{
	kToken *TypeToken = GCSAFE_new(Token, 0);
	kToken_setTypeId(kctx, TypeToken, ns, typeId);
	return TypeToken;
}

static kToken* new_ParsedExprToken(KonohaContext *kctx, kNameSpace *ns, kExpr *expr)
{
	kTokenVar *ParsedExprToken = GCSAFE_new(TokenVar, 0);
	ParsedExprToken->resolvedSyntaxInfo = SYN_(ns, KW_ExprPattern);
	KSETv(ParsedExprToken, ParsedExprToken->parsedExpr, expr);
	return (kToken*)ParsedExprToken;
}

static void MacroSet_setTokenAt(KonohaContext *kctx, MacroSet *macroSet, int index, kArray *tokenList, const char *symbol, ...)
{
	DBG_ASSERT(macroSet[index].tokenList == NULL);
	macroSet[index].symbol = KLIB Ksymbol(kctx, symbol, strlen(symbol), SPOL_TEXT|SPOL_ASCII, _NEWID);
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
	kArray *tokenList = KonohaContext_getSugarContext(kctx)->preparedTokenList;
	TokenRange macroRangeBuf, *macroRange = SUGAR new_TokenListRange(kctx, ns, tokenList, &macroRangeBuf);
	SUGAR TokenRange_tokenize(kctx, macroRange, "T _ = E; if(_.hasNext()) {N = _.next(); }", 0);
	MacroSet macroSet[4] = {{0, NULL, 0, 0}};
	MacroSet_setTokenAt(kctx, macroSet, 0, tokenList, "T", IteratorTypeToken, NULL);
	MacroSet_setTokenAt(kctx, macroSet, 1, tokenList, "E", IteratorExprToken, NULL);
	if(TypeToken == NULL) {
		MacroSet_setTokenAt(kctx, macroSet, 2, tokenList, "N", VariableToken, NULL);
	}
	else {
		MacroSet_setTokenAt(kctx, macroSet, 2, tokenList, "N", TypeToken, VariableToken, NULL);
	}
	macroRange->macroSet = macroSet;
	TokenRange expandedRangeBuf, *expandedRange = SUGAR new_TokenListRange(kctx, ns, tokenList, &expandedRangeBuf);
	SUGAR TokenRange_resolved(kctx, expandedRange, macroRange);
	return SUGAR new_kBlock(kctx, stmt, expandedRange, NULL);
}

static void kStmt_appendBlock(KonohaContext *kctx, kStmt *stmt, kBlock *bk)
{
	if(bk != NULL) {
		kBlock *block = SUGAR kStmt_getBlock(kctx, stmt, Stmt_nameSpace(stmt), KW_BlockPattern, NULL);
		int i;
		for(i = 0; i < kArray_size(bk->stmtList); i++) {
			KLIB kArray_add(kctx, block->stmtList, bk->stmtList->stmtItems[i]);
		}
	}
}

static KMETHOD StmtTyCheck_for(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_StmtTyCheck(stmt, gma);
	DBG_P("for statement .. ");
	int isOkay = false;
	if(SUGAR kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_var, 0)) {
		kNameSpace *ns = Stmt_nameSpace(stmt);
		kToken *TypeToken = SUGAR kStmt_getToken(kctx, stmt, KW_TypePattern, NULL);
		kToken *VariableToken  = SUGAR kStmt_getToken(kctx, stmt, KW_SymbolPattern, NULL);
		DBG_P("typeToken=%p, varToken=%p", TypeToken, VariableToken);
		kExpr *IteratorExpr = SUGAR kStmt_getExpr(kctx, stmt, KW_ExprPattern, NULL);
		if(!TY_isIterator(IteratorExpr->ty)) {
			kMethod *mtd = KLIB kNameSpace_getMethodNULL(kctx, ns, IteratorExpr->ty, MN_to(TY_Iterator), 0, MPOL_PARAMSIZE);
			if(mtd == NULL) {
				kStmtExpr_printMessage(kctx, stmt, IteratorExpr, ErrTag, "expected Iterator expression after in");
				RETURNb_(false);
			}
			IteratorExpr = SUGAR new_TypedCallExpr(kctx, stmt, gma, TY_var, mtd, 1, IteratorExpr);
			kStmt_setObject(kctx, stmt, KW_ExprPattern, IteratorExpr);
		}
		kBlock *block = new_MacroBlock(kctx, stmt, new_TypeToken(kctx, ns, IteratorExpr->ty), new_ParsedExprToken(kctx, ns, IteratorExpr), TypeToken, VariableToken);
		kStmt *IfStmt = block->stmtList->stmtItems[1]; // @see macro;
		kStmt_appendBlock(kctx, IfStmt, SUGAR kStmt_getBlock(kctx, stmt, ns, KW_BlockPattern, NULL));
		Stmt_setCatchBreak(IfStmt, true);
		Stmt_setCatchContinue(IfStmt, true);
		isOkay = SUGAR kBlock_tyCheckAll(kctx, block, gma);
		if(isOkay) {
			kStmt_typed(IfStmt, LOOP);
			kStmt_setObject(kctx, stmt, KW_BlockPattern, block);
			kStmt_typed(stmt, BLOCK);
		}
	}
	RETURNb_(isOkay);
}

static kbool_t iterator_initNameSpace(KonohaContext *kctx,  kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("for"), StmtTyCheck_(for),
			.rule = "\"for\" \"(\" [$Type] $Symbol \"in\" $Expr  \")\" [$Block] ", },
		{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX);
	return true;
}

static kbool_t iterator_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* iterator_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("iterator", "1.0"),
		.initPackage =iterator_initPackage,
		.setupPackage = iterator_setupPackage,
		.initNameSpace = iterator_initNameSpace,
		.setupNameSpace = iterator_setupNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
