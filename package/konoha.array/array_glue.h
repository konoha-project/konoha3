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

#ifndef ARRAY_GLUE_H_
#define ARRAY_GLUE_H_

/* ------------------------------------------------------------------------ */

//## @Immutable method T0 Array.get(Int n);
static KMETHOD Array_get(KonohaContext *kctx, KonohaStack *sfp)
{
	kArray *a = sfp[0].asArray;
	size_t n = check_index(kctx, sfp[1].ivalue, kArray_size(a), sfp[K_RTNIDX].uline);
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
	size_t n = check_index(kctx, sfp[1].ivalue, kArray_size(a), sfp[K_RTNIDX].uline);
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


static KMETHOD Array_newArray(KonohaContext *kctx, KonohaStack *sfp)
{
	kArrayVar *a = (kArrayVar *)sfp[0].asObject;
	size_t asize = (size_t)sfp[1].ivalue;
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

static void NArray_add(KonohaContext *kctx, kArray *o, uintptr_t value)
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
		NArray_add(kctx, a, sfp[1].unboxValue);
	} else {
		KLIB kArray_add(kctx, a, sfp[1].asObject);
	}
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _Im       kMethod_Immutable
#define _F(F)     (intptr_t)(F)

static	kbool_t array_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Im, _F(Array_get), TY_0,   TY_Array, MN_("get"), 1, TY_Int, FN_("index"),
		_Public,     _F(Array_set), TY_void, TY_Array, MN_("set"), 2, TY_Int, FN_("index"),  TY_0, FN_("value"),
		_Public,     _F(Array_getSize), TY_Int, TY_Array, MN_("getSize"), 0,
		_Public,     _F(Array_newArray), TY_Array, TY_Array, MN_("newArray"), 1, TY_Int, FN_("size"),
		_Public,     _F(Array_add1), TY_void, TY_Array, MN_("add"), 1, TY_0, FN_("value"),
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t array_setupPackage(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static KMETHOD ParseExpr_BRACKET(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tokenArray, s, c, e);
	DBG_P("parse bracket!!");
	kToken *tk = tokenArray->tokenItems[c];
	if(s == c) { // TODO
		kExpr *expr = SUGAR kStmt_parseExpr(kctx, stmt, tk->sub, 0, kArray_size(tk->sub));
		RETURN_(SUGAR kStmt_rightJoinExpr(kctx, stmt, expr, tokenArray, c+1, e));
	}
	else {
		kExpr *lexpr = SUGAR kStmt_parseExpr(kctx, stmt, tokenArray, s, c);
		if(lexpr == K_NULLEXPR) {
			RETURN_(lexpr);
		}
		if(lexpr->syn->keyword == KW_new) {  // new int[100]
			kExpr_setsyn(lexpr, SYN_(kStmt_nameSpace(stmt), KW_ExprMethodCall));
			lexpr = SUGAR Stmt_addExprParams(kctx, stmt, lexpr, tk->sub, 0, kArray_size(tk->sub), 0/*allowEmpty*/);
		}
		else {   // X[1] => get X 1
			kTokenVar *tkN = GCSAFE_new(TokenVar, 0);
			tkN->keyword = MN_toGETTER(0);
			tkN->uline = tk->uline;
			SugarSyntax *syn = SYN_(kStmt_nameSpace(stmt), KW_ExprMethodCall);
			lexpr  = SUGAR new_ConsExpr(kctx, syn, 2, tkN, lexpr);
			lexpr = SUGAR Stmt_addExprParams(kctx, stmt, lexpr, tk->sub, 0, kArray_size(tk->sub), 1/*allowEmpty*/);
		}
		RETURN_(SUGAR kStmt_rightJoinExpr(kctx, stmt, lexpr, tokenArray, c+1, e));
	}
}

static kbool_t array_initNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("[]"), .flag = SYNFLAG_ExprPostfixOp2, ParseExpr_(BRACKET), .priority_op2 = 16, },  //AST_BRACKET
		{ .keyword = KW_END, },
	};
	SUGAR NameSpace_defineSyntax(kctx, ns, SYNTAX);
	return true;
}

static kbool_t array_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

#endif /* ARRAY_GLUE_H_ */
