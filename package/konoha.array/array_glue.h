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


static KMETHOD Array_newArray(KonohaContext *kctx, KonohaStack *sfp)
{
	kArrayVar *a = (kArrayVar *)sfp[0].asObject;
	size_t asize = (size_t)sfp[1].intValue;
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

static KMETHOD Array_new(KonohaContext *kctx, KonohaStack *sfp)
{

	RETURN_(KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0));
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

static kbool_t array_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

//
//#define Token_text(tk) kToken_t_(kctx, tk)
//static const char *kToken_t_(KonohaContext *kctx, kToken *tk)
//{
//	switch((int)tk->keyword) {
//	case TokenType_INDENT: return "indent";
//	case TokenType_CODE: ;
//	case KW_BraceGroup: return "{... }";
//	case KW_ParenthesisGroup: return "(... )";
//	case KW_BracketGroup: return "[... ]";
//	default:  return S_text(tk->text);
//	}
//}
//
//#define KdumpTokenArray(CTX, TLS, S, E) 	dumpTokenArray(CTX, 0, TLS, S, E)
//#define KdumpExpr(CTX, EXPR)                dumpExpr(CTX, 0, 0, EXPR)
//
//static void dumpToken(KonohaContext *kctx, kToken *tk)
//{
//	DUMP_P("%s%s %d: kw=%s%s '%s'\n", KW_t(tk->keyword), (short)tk->uline, KW_t(tk->keyword), Token_text(tk));
//}
//
//static void dumpIndent(KonohaContext *kctx, int nest)
//{
//	int i;
//	for(i = 0; i < nest; i++) {
//		DUMP_P("  ");
//	}
//}
//
//static int dumpBeginTokenList(kToken *tk)
//{
//	switch(tk->keyword) {
//	case KW_ParenthesisGroup: return '(';
//	case KW_BraceGroup: return '{';
//	case KW_BracketGroup: return '[';
//	}
//	return '<';
//}
//
//static int dumpEndTokenList(kToken *tk)
//{
//	switch(tk->keyword) {
//	case KW_ParenthesisGroup: return ')';
//	case KW_BraceGroup: return '}';
//	case KW_BracketGroup: return ']';
//	}
//	return '>';
//}
//
//static void dumpTokenArray(KonohaContext *kctx, int nest, kArray *a, int s, int e)
//{
//	if(nest == 0) DUMP_P("\n");
//	while(s < e) {
//		kToken *tk = a->tokenItems[s];
//		dumpIndent(kctx, nest);
//		if(IS_Array(tk->subTokenList)) {
//			DUMP_P("%c\n", dumpBeginTokenList(tk));
//			dumpTokenArray(kctx, nest+1, tk->subTokenList, 0, kArray_size(tk->subTokenList));
//			dumpIndent(kctx, nest);
//			DUMP_P("%c\n", dumpEndTokenList(tk));
//		}
//		else {
//			DUMP_P("TK(%d) ", s);
//			dumpToken(kctx, tk);
//		}
//		s++;
//	}
//	if(nest == 0) DUMP_P("====\n");
//}
//
//
//static void dumpExpr(KonohaContext *kctx, int n, int nest, kExpr *expr)
//{
//	if(nest == 0) DUMP_P("\n");
//	dumpIndent(kctx, nest);
//	if(expr == K_NULLEXPR) {
//		DUMP_P("[%d] ExprTerm: null", n);
//	}
//	else if(Expr_isTerm(expr)) {
//		DUMP_P("[%d] ExprTerm: kw='%s%s' %s", n, KW_t(expr->termToken->keyword), Token_text(expr->termToken));
//		if(expr->ty != TY_var) {
//
//		}
//		DUMP_P("\n");
//	}
//	else {
//		int i;
//		if(expr->syn == NULL) {
//			DUMP_P("[%d] Cons: kw=NULL, size=%ld", n, kArray_size(expr->cons));
//		}
//		else {
//			DUMP_P("[%d] Cons: kw='%s%s', size=%ld", n, KW_t(expr->syn->keyword), kArray_size(expr->cons));
//		}
//		if(expr->ty != TY_var) {
//
//		}
//		DUMP_P("\n");
//		for(i=0; i < kArray_size(expr->cons); i++) {
//			kObject *o = expr->cons->objectItems[i];
//			if(O_ct(o) == CT_Expr) {
//				dumpExpr(kctx, i, nest+1, (kExpr*)o);
//			}
//			else {
//				dumpIndent(kctx, nest+1);
//				if(O_ct(o) == CT_Token) {
//					kToken *tk = (kToken*)o;
//					DUMP_P("[%d] O: %s ", i, CT_t(o->h.ct));
//					dumpToken(kctx, tk);
//				}
//				else if(o == K_NULL) {
//					DUMP_P("[%d] O: null\n", i);
//				}
//				else {
//					DUMP_P("[%d] O: %s\n", i, CT_t(o->h.ct));
//				}
//			}
//		}
//	}
//}

/*
http://www.amazon.co.jp/exec/obidos/ASIN/0201914654/
*/

int numofbits(long bits) {
  bits = (bits & 0x55555555) + (bits >> 1 & 0x55555555);
  bits = (bits & 0x33333333) + (bits >> 2 & 0x33333333);
  bits = (bits & 0x0f0f0f0f) + (bits >> 4 & 0x0f0f0f0f);
  bits = (bits & 0x00ff00ff) + (bits >> 8 & 0x00ff00ff);
  return (bits & 0x0000ffff) + (bits >>16 & 0x0000ffff);
}

static KonohaClass *kGetParamTypeFromExprCons(KonohaContext *kctx, kArray *exprCons, int beginIdx)
{
	size_t i = 0;
	DBG_ASSERT(beginIdx < kArray_size(exprCons));
	kint_t types = 0; // 64bits
	for (i = beginIdx; i < kArray_size(exprCons); i++) {
		kExpr* expr = exprCons->exprItems[i];
		ktype_t ty = expr->ty;
		DBG_P("ty='%s%s'", TY_t(ty));
		types |= 1 << ty;

	}
	DBG_P("numofbits=%d",numofbits(types));
	return NULL;
}


static KMETHOD ExprTyCheck_BRACKET(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kArray *a1 = expr->cons;
	kTokenVar *tkN = a1->tokenVarItems[0];
	// tycheck first
	int i;
	KonohaClass *ct = CT_(reqty);
	// var, or array[t1];
	kParam *cparam = CT_cparam(ct);
	DBG_ASSERT(cparam->psize < 2);
	ktype_t pt0 = TY_var;
	DBG_P("paramsize=%d", cparam->psize);
	if (cparam->psize == 0){
		// var or only Array
		reqty = (CT_p0(kctx, CT_Array, TY_Object))->classId;
	} else if(cparam->psize == 1) {
		pt0 = cparam->paramtypeItems[0].ty;
		DBG_P("param0 ty=%s", TY_t(pt0));
	} else {
		RETURN_(K_NULLEXPR);
	}
//	DBG_P("expr->cons=%d", kArray_size(expr->cons));
	for (i = 1; i < kArray_size(a1); i++) {
		kExpr *rexpr = SUGAR kStmt_tyCheckByNameAt(kctx, stmt, expr, i, gma, pt0, TPOL_ALLOWVOID);
		if (rexpr == K_NULLEXPR) {
			RETURN_(K_NULLEXPR);
		}
	}

	// make newArray TK
	kArray *a2 = tkN->subTokenList;
	//kToken *tkMethod = expr->cons->tokenItems[0];
	kTokenVar *tkType = a2->tokenVarItems[1];
	//kTokenVar *tkInt = expr->cons->tokenVarItems[2];
	tkType->resolvedTypeId = TY_Int; // tmp
	tkType->text = KLIB new_kString(kctx, "int", sizeof("int"), SPOL_POOL|SPOL_ASCII);
	kExpr *exprType = SUGAR kStmt_parseExpr(kctx, stmt, a2, 1, 2);
	kExpr *exprInt = SUGAR kStmt_parseExpr(kctx, stmt, a2, 2, 3);
	KLIB kArray_insert(kctx, a2, 1, exprType);
	KLIB kArray_insert(kctx, a2, 2, exprInt);
	KLIB kArray_clear(kctx, a2, 3);

	// remove [] token
	kExprVar *lexpr = expr;
	SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall);
	lexpr->syn = syn;
	lexpr->ty = reqty;
	lexpr->cons = a2;
	lexpr->build = TEXPR_NEW;
	//KdumpExpr(kctx, lexpr);

	RETURN_(lexpr);
}

static KMETHOD ParseExpr_BRACKET(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tokenArray, beginIdx, currentIdx, endIdx);

	KonohaClass *genericsClass = NULL;
	int nextIdx = SUGAR kStmt_parseTypePattern(kctx, stmt, Stmt_nameSpace(stmt), tokenList, beginIdx, endIdx, &genericsClass);
	if (nextIdx != -1) {
		RETURN_(SUGAR kStmt_parseOperatorExpr(kctx, stmt, tokenList, beginIdx, beginIdx, endIdx));
	}

	kToken *tk = tokenArray->tokenItems[currentIdx];
	if(beginIdx == currentIdx) { // TODO
//		// $Type $symbol = [1,2,3];
//		// --> 1. $variable = $Type.newArray(0);
//		//     2. $variable.add(1);
//		//     3. $variable.add(2);
//		//     4. $variable.add(3);
//		kToken *symTk = tokenArray->tokenItems[currentIdx - 2]; // symbol name
//		if(symTk->keyword != KW_SymbolPattern) {
//			RETURN_(K_NULLEXPR);
//		}
//		// line1
//		kTokenVar *tk1 = GCSAFE_new(TokenVar, 0);
//		tk1->keyword = MN_("newArray");
//		tk1->uline = tk->uline;
//		tk1->text = KLIB new_kString(kctx, "new", sizeof("new"), SPOL_POOL|SPOL_ASCII);
//		// empty $Type
//		kTokenVar *tk2 = GCSAFE_new(TokenVar, 0);
//		tk2->keyword = KW_TypePattern;
//		tk2->uline = tk->uline;
//		tk2->resolvedSyntaxInfo = SYN_(Stmt_nameSpace(stmt), KW_TypePattern);
//		// token without typing
//		kTokenVar *tk3 = GCSAFE_new(TokenVar, 0);
//		tk3->keyword = TokenType_INT;
//		tk3->text = KLIB new_kString(kctx, "0", sizeof("0"), SPOL_POOL|SPOL_ASCII);
//		tk3->resolvedSyntaxInfo = SYN_(Stmt_nameSpace(stmt), KW_NumberPattern);
//
//		SugarSyntax *methodSyntax = SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall);
//		kExpr *methodExpr = SUGAR new_ConsExpr(kctx, methodSyntax,3, tk1, tk2, tk3);
//
//		// line2
//		kTokenVar *tk4 = GCSAFE_new(TokenVar, 0);
//		tk4->keyword = MN_("add");
//		tk4->uline = tk->uline;
//		tk4->text = KLIB new_kString(kctx, "add", sizeof("add"), SPOL_POOL|SPOL_ASCII);
//
//		kTokenVar *tk5 = GCSAFE_new(TokenVar, 0);
//		tk5->keyword = KW_SymbolPattern;
//		tk5->uline = tk->uline;
//		tk5->text = KLIB new_kString(kctx, S_text(symTk->text), S_size(symTk->text), SPOL_POOL|SPOL_ASCII);
//
//		kTokenVar *tk6 = GCSAFE_new(TokenVar, 0);
//		kToken_setTypeId(kctx, tk6, Stmt_nameSpace(stmt), TY_Object);
//		//tk6->keyword = KW_TypePattern; // tmporary
//		//tk6->uline = tk->uline;
//		//tk6->resolvedSyntaxInfo = SYN_(Stmt_nameSpace(stmt), KW_TypePattern);
//
//		SugarSyntax *addSyntax = SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall);
//		kExpr *addExpr = SUGAR new_ConsExpr(kctx, addSyntax, 3, tk4, tk5, tk6);
//
//		SugarSyntax *bracketSyntax = SYN_(Stmt_nameSpace(stmt),  SYM_("[]"));
//
//		kExpr *lexpr = SUGAR new_ConsExpr(kctx, bracketSyntax, 2, methodExpr, addExpr);
//
//		lexpr = SUGAR kStmt_addExprParam(kctx, stmt, lexpr, tk->subTokenList, 0, kArray_size(tk->subTokenList), 1);
//		RETURN_(lexpr);
	}
	else {  // Func [int]
		kExpr *lexpr = SUGAR kStmt_parseExpr(kctx, stmt, tokenArray, beginIdx, currentIdx);
		if(lexpr == K_NULLEXPR) {
			RETURN_(lexpr);
		}
		if(lexpr->syn->keyword == KW_new) {  // new int[100]
			kExpr_setsyn(lexpr, SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall));
			lexpr = SUGAR kStmt_addExprParam(kctx, stmt, lexpr, tk->subTokenList, 0, kArray_size(tk->subTokenList), 0/*allowEmpty*/);
		}
		else {   // X[1] => get X 1
			kTokenVar *tkN = GCSAFE_new(TokenVar, 0);
			tkN->resolvedSymbol = MN_toGETTER(0);
			tkN->uline = tk->uline;
			SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall);
			lexpr  = SUGAR new_ConsExpr(kctx, syn, 2, tkN, lexpr);
			lexpr = SUGAR kStmt_addExprParam(kctx, stmt, lexpr, tk->subTokenList, 0, kArray_size(tk->subTokenList), 1/*allowEmpty*/);
		}
		RETURN_(SUGAR kStmt_rightJoinExpr(kctx, stmt, lexpr, tokenArray, currentIdx + 1, endIdx));
	}
}

static kbool_t array_initNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("[]"), .flag = SYNFLAG_ExprPostfixOp2, ExprTyCheck_(BRACKET), ParseExpr_(BRACKET), .precedence_op2 = 300 },  //KW_BracketGroup
		{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX);
	return true;
}

static kbool_t array_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

#endif /* ARRAY_GLUE_H_ */
