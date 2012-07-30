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

#include<minikonoha/minikonoha.h>
#include<minikonoha/sugar.h>
#include<minikonoha/float.h>

/* ------------------------------------------------------------------------ */

//## @Immutable method T0 Array.get(Int n);s
static KMETHOD Array_get(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	size_t n = check_index(kctx, sfp[1].intValue, kArray_size(a), sfp[K_RTNIDX].uline);
	if(kArray_isUnboxData(a)) {
		RETURNd_(a->unboxItems[n]);
	}
	else {
		RETURN_(a->objectItems[n]);
	}
}

//## method void Array.set(Int n, T0 v);
static KMETHOD Array_set(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	size_t n = check_index(kctx, sfp[1].intValue, kArray_size(a), sfp[K_RTNIDX].uline);
	if(kArray_isUnboxData(a)) {
		a->unboxItems[n] = sfp[2].unboxValue;
	}
	else {
		KSETv(a->objectItems[n], sfp[2].o);
	}
}

//## method int Array.getSize();
static KMETHOD Array_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	RETURNi_(kArray_size(a));
}

#define KARRAY_LIST_SIZE_MAX 1024
static KMETHOD Array_newArray(KonohaContext *kctx, KonohaStack *sfp)
{
	kArrayVar *a = (kArrayVar *)sfp[0].asObject;
	size_t asize = (size_t)sfp[1].intValue;
	if (asize < 0 || KARRAY_LIST_SIZE_MAX < asize) {
		ktrace(_UserInputFault,
				KEYVALUE_s("error", "Invalid argument"),
				KEYVALUE_u("length", asize)
		);
		RETURN_(a);
	}
	a->bytemax = asize * sizeof(void*);
	kArray_setsize((kArray*)a, asize);
	a->objectItems = (kObject**)KCALLOC(a->bytemax, 1);
	if(!kArray_isUnboxData(a)) {
		size_t i;
		kObject *null = KLIB Knull(kctx, CT_(O_p0(a)));
		for(i = 0; i < asize; i++) {
			KSETv(a->objectItems[i], null);
		}
	}
	RETURN_(a);
}

// Array
struct _kAbstractArray {
	KonohaObjectHeader h;
	KUtilsGrowingArray a;
};

static void NArray_ensureMinimumSize(KonohaContext *kctx, struct _kAbstractArray *a, size_t min)
{
	size_t minbyte = min * sizeof(void*);
	if(!(minbyte < a->a.bytemax)) {
		if(minbyte < sizeof(kObject)) minbyte = sizeof(kObject);
		KLIB Karray_expand(kctx, &a->a, minbyte);
	}
}

static void UnboxArray_add(KonohaContext *kctx, kArray *o, uintptr_t value)
{
	size_t asize = kArray_size(o);
	struct _kAbstractArray *a = (struct _kAbstractArray*)o;
	NArray_ensureMinimumSize(kctx, a, asize+1);
	DBG_ASSERT(a->a.objectItems[asize] == NULL);
	kArrayVar *a2 = (kArrayVar *)a;
	a2->unboxItems[asize] = value;
	kArray_setsize(a2, (asize+1));
}

static KMETHOD Array_add1(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = (kArray *)sfp[0].asObject;
	if (kArray_isUnboxData(a)) {
		UnboxArray_add(kctx, a, sfp[1].unboxValue);
	} else {
		KLIB kArray_add(kctx, a, sfp[1].asObject);
	}
}


static KMETHOD Array_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kArrayVar *a = (kArrayVar *)sfp[0].asObject;
	//DBG_P("objitem=%p", a->objectItems);
	size_t asize = (size_t)sfp[1].intValue;
	if (asize < 0) {

	}
	a->bytemax = asize * sizeof(void*);
	//kArray_setsize((kArray*)a, asize);
	a->objectItems = (kObject**)KCALLOC(a->bytemax, 1);
	if(!kArray_isUnboxData(a)) {
		size_t i;
		kObject *null = KLIB Knull(kctx, CT_(O_p0(a)));
		for(i = 0; i < asize; i++) {
			KSETv(a->objectItems[i], null);
		}
	}
	RETURN_(a);
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _Im       kMethod_Immutable
#define _F(F)     (intptr_t)(F)

static KMETHOD Array_newList(KonohaContext *kctx, KonohaStack *sfp);

static	kbool_t array_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KREQUIRE_PACKAGE("konoha.new", pline);
	KDEFINE_METHOD MethodData[] = {
		_Public|_Im, _F(Array_get), TY_0,   TY_Array, MN_("get"), 1, TY_Int, FN_("index"),
		_Public,     _F(Array_set), TY_void, TY_Array, MN_("set"), 2, TY_Int, FN_("index"),  TY_0, FN_("value"),
		_Public,     _F(Array_getSize), TY_Int, TY_Array, MN_("getSize"), 0,
		_Public,     _F(Array_newArray), TY_Array, TY_Array, MN_("newArray"), 1, TY_Int, FN_("size"),
		_Public,     _F(Array_add1), TY_void, TY_Array, MN_("add"), 1, TY_0, FN_("value"),
		_Public|kMethod_Hidden, _F(Array_newList), TY_Array, TY_Array, MN_("newList"), 0,
		_Public|_Im, _F(Array_new), TY_void, TY_Array, MN_("new"), 1, TY_Int, FN_("size"),
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t array_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static KMETHOD Array_newList(KonohaContext *kctx, KonohaStack *sfp)
{
	kArrayVar *a = (kArrayVar*)sfp[0].asObject;
	KonohaStack *p = sfp+1;
	if(!kArray_isUnboxData(a)) {
		for(; p < kctx->esp; p++) {
			KLIB kArray_add(kctx, a, p[0].asObject);
		}
	}
	else {
		for(; p < kctx->esp; p++) {
			UnboxArray_add(kctx, a, p[0].unboxValue);
		}
	}
	RETURN_(a);
}

static KMETHOD ExprTyCheck_Bracket(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	// [0] currentToken, [1] NULL, [2] ....
	size_t i;
	KonohaClass *requestClass = CT_(reqty);
	ktype_t paramType = TY_var; // default
	if(requestClass->baseclassId == TY_Array) {
		paramType = requestClass->p0;
	}
	else {
		requestClass = NULL; // undefined
	}
	for(i = 2; i < kArray_size(expr->cons); i++) {
		kExpr *typedExpr = SUGAR kStmt_tyCheckByNameAt(kctx, stmt, expr, i, gma, paramType, 0);
		if(typedExpr == K_NULLEXPR) {
			RETURN_(typedExpr);
		}
		DBG_P("i=%d, paramType=%s, typedExpr->ty=%s", i, TY_t(paramType), TY_t(typedExpr->ty));
		if(paramType == TY_var) {
			paramType = typedExpr->ty;
		}
	}
	if(requestClass == NULL) {
		requestClass = (paramType == TY_var) ? CT_Array : CT_p0(kctx, CT_Array, paramType);
	}
	kMethod *mtd = KLIB kNameSpace_getMethodNULL(kctx, Stmt_nameSpace(stmt), TY_Array, MN_("newList"), 0, MPOL_FIRST);
	DBG_ASSERT(mtd != NULL);
	KSETv(expr->cons->methodItems[0], mtd);
	KSETv(expr->cons->exprItems[1], SUGAR kExpr_setVariable(kctx, NULL, gma, TEXPR_NEW, requestClass->classId, 0));
	RETURN_(Expr_typed(expr, TEXPR_CALL, requestClass->classId));
}

static KMETHOD ParseExpr_Bracket(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tokenList, beginIdx, currentIdx, endIdx);

	KonohaClass *genericsClass = NULL;
	int nextIdx = SUGAR kStmt_parseTypePattern(kctx, stmt, Stmt_nameSpace(stmt), tokenList, beginIdx, endIdx, &genericsClass);
	if (nextIdx != -1) {  // to avoid Func[T]
		RETURN_(SUGAR kStmt_parseOperatorExpr(kctx, stmt, tokenList, beginIdx, beginIdx, endIdx));
	}
	kToken *currentToken = tokenList->tokenItems[currentIdx];
	if(beginIdx == currentIdx) {
		DBG_ASSERT(currentToken->resolvedSyntaxInfo->keyword == KW_BracketGroup);
		kExpr *arrayExpr = SUGAR new_ConsExpr(kctx, currentToken->resolvedSyntaxInfo, 2, currentToken, K_NULL);
		RETURN_(SUGAR kStmt_addExprParam(kctx, stmt, arrayExpr, currentToken->subTokenList, 0, kArray_size(currentToken->subTokenList), /*allowEmpty*/1));
	}
	else {
		kExpr *leftExpr = SUGAR kStmt_parseExpr(kctx, stmt, tokenList, beginIdx, currentIdx);
		if(leftExpr == K_NULLEXPR) {
			RETURN_(leftExpr);
		}
		if(leftExpr->syn->keyword == KW_new) {  // new int[100]
			kExpr_setsyn(leftExpr, SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall));
			leftExpr = SUGAR kStmt_addExprParam(kctx, stmt, leftExpr, currentToken->subTokenList, 0, kArray_size(currentToken->subTokenList), 0/*allowEmpty*/);
		}
		else {   // X[1] => get X 1
			kTokenVar *tkN = GCSAFE_new(TokenVar, 0);
			tkN->resolvedSymbol= MN_toGETTER(0);
			tkN->uline = currentToken->uline;
			SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall);
			leftExpr  = SUGAR new_ConsExpr(kctx, syn, 2, tkN, leftExpr);
			leftExpr = SUGAR kStmt_addExprParam(kctx, stmt, leftExpr, currentToken->subTokenList, 0, kArray_size(currentToken->subTokenList), 1/*allowEmpty*/);
		}
		RETURN_(SUGAR kStmt_rightJoinExpr(kctx, stmt, leftExpr, tokenList, currentIdx + 1, endIdx));
	}
}

#define GROUP(T)    .keyword = KW_##T##Group

static kbool_t array_initNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ GROUP(Bracket), .flag = SYNFLAG_ExprPostfixOp2, ExprTyCheck_(Bracket), ParseExpr_(Bracket), .precedence_op2 = 300, },
		{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX);
	return true;
}

static kbool_t array_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}


KDEFINE_PACKAGE* array_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("array", "1.0"),
		.initPackage = array_initPackage,
		.setupPackage = array_setupPackage,
		.initNameSpace = array_initNameSpace,
		.setupNameSpace = array_setupNameSpace,
	};
	return &d;
}
