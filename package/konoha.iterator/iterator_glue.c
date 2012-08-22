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
		_Public, _F(Iterator_hasNext), TY_Boolean, TY_Iterator, MN_("hasNext"), 0,
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

//static kStmt* new_TypedWhileStmt(KonohaContext *kctx, kStmt *stmt, kGamma *gma, )
//{
////	kExpr *iteratorExpr = SUGAR new_UntypedTermExpr(kctx, itToken);
////	kMethod *mtd = kNameSpace_getMethodNULL(kctx, Stmt_nameSpace(stmt), TY_Iterator, MN_("hasNext"), 0, MPOL_PARAMSIZE);
////	kExpr *hasNextExpr = SUGAR new_TypedCallExpr(kctx, ns, gma, TY_Boolean, mtd, 1, iteratorExpr);
////
////	kStmt *whileStmt = SUGAR new_kStmt(kctx, ns, KW_StmtTypeDecl/*dummy*/, KW_ExprPattern, hasNextExpr, KW_BlockPattern, loopBlock, 0);;
////
////	//	if(SUGAR kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_Boolean, 0)) {
////	//		kBlock *bk = SUGAR kStmt_getBlock(kctx, stmt, NULL/*DefaultNameSpace*/, KW_BlockPattern, K_NULLBLOCK);
////	//		ret = SUGAR kBlock_tyCheckAll(kctx, bk, gma);
////	//		kStmt_typed(stmt, LOOP);
////	//	}
//}

#define TY_isIterator(T)     (CT_(T)->baseTypeId == TY_Iterator)

static KMETHOD StmtTyCheck_for(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_StmtTyCheck(stmt, gma);
	DBG_P("for statement .. ");
	kNameSpace *ns = Stmt_nameSpace(stmt);
	kToken *typeToken = SUGAR kStmt_getToken(kctx, stmt, KW_TypePattern, NULL);
	kToken *varToken  = SUGAR kStmt_getToken(kctx, stmt, KW_SymbolPattern, NULL);
	DBG_P("typeToken=%p, varToken=%p", typeToken, varToken);
//	if(!SUGAR kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_var, 0)) {
//		RETURNb_(false);
//	}
//	kExpr *iteratorExpr = SUGAR kStmt_getExpr(kctx, stmt, KW_ExprPattern, NULL);
//	if(!TY_isIterator(iteratorExpr->ty)) {
//		kMethod *mtd = kNameSpace_getMethodNULL(kctx, ns, iteratorExpr->ty, MN_to(TY_Iterator), 0, MPOL_PARAMSIZE);
//		if(mtd == NULL) {
//
//		}
//		iteratorExpr = SUGAR new_TypedCallExpr(kctx, ns, gma, TY_var, mtd, 1, iteratorExpr);
//	}
//	if(typeToken != NULL) {
//		KonohaClass *cIterator = CT_p0(kctx, CT_Iterator, typeToken->resolvedTypeId);
//		if(!SUGAR kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, cIterator->typeId, 0)) {
//			RETURNb_(false);
//		}
//	}
//	else {
//
//	}
//	TokenRange empty = {Stmt_nameSpace(stmt)};
//	kBlock *block = SUGAR new_kBlock(kctx, stmt, &empty, NULL);
//	if(typeToken != NULL) {   // declare local variable
//		kExpr *termExpr = SUGAR new_UntypedTermExpr(kctx, varToken);
//		kStmt *declStmt = SUGAR new_kStmt(kctx, ns, KW_StmtTypeDecl, KW_TypePattern, typeToken, KW_ExprPattern, termExpr, 0);
//		SUGAR kBlock_insertAfter(kctx, block, /*lastStmt*/NULL, declStmt);
//	}
//	{
//		kTokenVar *itToken = GCSAFE_new(TokenVar, 0);
//		itToken->resolvedSyntaxInfo = varToken->resolvedSyntaxInfo; // KW_SymbolPattern
//		// _ = A;
//		kExpr *termExpr = SUGAR new_UntypedTermExpr(kctx, itToken);
//		kExpr *letExpr = SUGAR new_kStmt(kctx, ns, KW_LET, KW_);
//		kExpr new_UntypedExpr
//	}
//	kStmt *whileStmt = new_TypedWhileStmt(kctx, stmt, varToken, itToken);
//	SUGAR kBlock_insertAfter(kctx, block, NULL, whileStmt);
//	kStmt_setObject(kctx, stmt, KW_BlockPattern, block);
	RETURNb_(true);
}

#define _LOOP .flag = (SYNFLAG_StmtJumpAhead|SYNFLAG_StmtJumpSkip)

static kbool_t iterator_initNameSpace(KonohaContext *kctx,  kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("for"), _LOOP, StmtTyCheck_(for),
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
