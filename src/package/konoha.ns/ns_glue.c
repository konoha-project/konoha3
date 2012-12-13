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

#ifdef __cplusplus
extern "C" {
#endif

#include<minikonoha/import/methoddecl.h>
#define TP_name         TY_String,     FN_("name")
#define TP_paramsize    TY_int,        FN_("paramsize")
#define TP_source       TY_String,     FN_("source")

//## NameSpace NameSpace.GetNameSpace();
static KMETHOD NameSpace_GetNameSpace(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnField(sfp[0].asNameSpace);
}

//## NameSpace NameSpace.GetParentNameSpace();
static KMETHOD NameSpace_GetParentNameSpace(KonohaContext *kctx, KonohaStack *sfp)
{
	kNameSpace *ns = (kNameSpace*)sfp[0].asNameSpace;
	if(ns->parentNULL != NULL) {
		ns = ns->parentNULL;
	}
	KReturnField(ns);
}

//## boolean NameSpace.DefineMacro(String symbol, String source);
static KMETHOD NameSpace_DefineMacro2(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = ksymbolA(kString_text(sfp[1].asString), kString_size(sfp[1].asString), _NEWID);
	kString *source = sfp[2].asString;
	KReturnUnboxValue(SUGAR kNameSpace_SetMacroData(kctx, sfp[0].asNameSpace, keyword, 0, kString_text(source), true));
}

//## boolean NameSpace.DefineMacro(String symbol, int param, String source);
static KMETHOD NameSpace_DefineMacro(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = ksymbolA(kString_text(sfp[1].asString), kString_size(sfp[1].asString), _NEWID);
	int paramsize = (int)sfp[2].intValue;
	kString *source = sfp[3].asString;
	KReturnUnboxValue(SUGAR kNameSpace_SetMacroData(kctx, sfp[0].asNameSpace, keyword, paramsize, kString_text(source), true));
}

static void namespace_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(NameSpace_GetNameSpace), TY_NameSpace, TY_NameSpace, MN_("GetNameSpace"), 0,
		_Public|_Const|_Im, _F(NameSpace_GetParentNameSpace), TY_NameSpace, TY_NameSpace, MN_("GetParentNameSpace"), 0,
		_Public, _F(NameSpace_DefineMacro2), TY_boolean, TY_NameSpace, MN_("DefineMacro"), 2, TP_name, TP_source,
		_Public, _F(NameSpace_DefineMacro), TY_boolean, TY_NameSpace, MN_("DefineMacro"), 3, TP_name, TP_paramsize, TP_source,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
//	KDEFINE_INT_CONST IntData[] = {
//		{"INT_MAX", TY_int, KINT_MAX},
//		{"INT_MIN", TY_int, KINT_MIN},
//		{NULL},
//	};
//	KLIB kNameSpace_LoadConstData(kctx, ns, KonohaConst_(IntData), false/*isOverride*/, trace);
}


// --------------------------------------------------------------------------

static KMETHOD Statement_namespace(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kstatus_t result = K_CONTINUE;
	kToken *tk = SUGAR kStmt_GetToken(kctx, stmt, KW_BlockPattern, NULL);
	if(tk != NULL && tk->resolvedSyntaxInfo->keyword == TokenType_CODE) {
		INIT_GCSTACK();
		kNameSpace *ns = new_(NameSpace, Stmt_ns(stmt), _GcStack);
		kArray *a = GetSugarContext(kctx)->preparedTokenList;
		TokenSeq range = {ns, a, kArray_size(a), kArray_size(a)};
		SUGAR TokenSeq_Tokenize(kctx, &range, kString_text(tk->text), tk->uline);
		result = SUGAR TokenSeq_Eval(kctx, &range, NULL/*trace*/);
		RESET_GCSTACK();
		kStmt_done(kctx, stmt);
	}
	KReturnUnboxValue(result == K_CONTINUE);
}

/* const CONST = VALUE */

static KMETHOD Statement_ConstDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kNameSpace *ns = Stmt_ns(stmt);
	kToken *symbolToken = SUGAR kStmt_GetToken(kctx, stmt, KW_SymbolPattern, NULL);
	ksymbol_t unboxKey = symbolToken->resolvedSymbol;
	kbool_t result = SUGAR kStmt_TypeCheckByName(kctx, stmt, KW_ExprPattern, gma, CT_INFER, TypeCheckPolicy_CONST);
	if(result) {
		kExpr *constExpr = SUGAR kStmt_GetExpr(kctx, stmt, KW_ExprPattern, NULL);
		KonohaClass *constClass = CT_(constExpr->attrTypeId);
		ktypeattr_t type = constClass->typeId;
		uintptr_t unboxValue;
		result = false;
		if(constExpr->build == TEXPR_NULL) {   // const C = String
			type = VirtualType_KonohaClass;
			unboxValue = (uintptr_t)constClass;
			result = true;
		}
		else if(constExpr->build == TEXPR_CONST) {   // const C = "1"
			unboxValue = (uintptr_t)constExpr->objectConstValue;
			result = true;
		}
		else if(constExpr->build == TEXPR_NCONST) {  // const c = 1
			unboxValue = constExpr->unboxConstValue;
			result = true;
		}
		if(result) {
			KMakeTraceUL(trace, sfp, stmt->uline);
			result = KLIB kNameSpace_SetConstData(kctx, ns, unboxKey, type, unboxValue, false/*isOverride*/, trace);
		}
		else {
			kStmt_Message(kctx, stmt, ErrTag, "constant value is expected: %s%s", PSYM_t(unboxKey));
		}
	}
	kStmt_done(kctx, stmt);
	KReturnUnboxValue(result);
}

/* defined (EXPR) */

static KMETHOD TypeCheck_Defined(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	size_t i;
	kbool_t isDefined = true;
	SugarContext *sugarContext = GetSugarContext(kctx);
	int popIsBlockingErrorMessage = sugarContext->isBlockedErrorMessage;
	sugarContext->isBlockedErrorMessage = true;
	for(i = 1; i < kArray_size(expr->cons); i++) {
		kExpr *typedExpr = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, i, gma, CT_INFER, TypeCheckPolicy_ALLOWVOID);
		if(typedExpr == K_NULLEXPR) {
			isDefined = false;
			break;
		}
	}
	sugarContext->isBlockedErrorMessage = popIsBlockingErrorMessage;
	KReturn(SUGAR kExpr_SetUnboxConstValue(kctx, expr, TY_boolean, isDefined));
}

static void filterArrayList(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx)
{
	int i;
	for(i = beginIdx; i < endIdx; i++) {
		if(i + 1 == endIdx || tokenList->TokenItems[i+1]->resolvedSyntaxInfo->keyword == KW_COMMA) {
			kTokenVar *tk = tokenList->TokenVarItems[i];
			if(tk->resolvedSyntaxInfo->keyword != KW_SymbolPattern) {  // defined
				tk->resolvedSyntaxInfo = SYN_(ns, KW_TextPattern);  // switch to text pattern
			}
			i++;
		}
		while(i < endIdx) {
			kTokenVar *tk = tokenList->TokenVarItems[i];
			i++;
			if(tk->resolvedSyntaxInfo->keyword == KW_COMMA) break;
		}
	}
}


static KMETHOD Expression_Defined(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, currentIdx, endIdx);
	if(beginIdx == currentIdx && beginIdx + 1 < endIdx) {
		kTokenVar *definedToken = tokenList->TokenVarItems[beginIdx];   // defined
		kTokenVar *pToken = tokenList->TokenVarItems[beginIdx+1];
		if(IS_Array(pToken->subTokenList)) {
			kExpr *expr = SUGAR new_UntypedCallStyleExpr(kctx, definedToken->resolvedSyntaxInfo, 1, definedToken);
			filterArrayList(kctx, Stmt_ns(stmt), pToken->subTokenList, 0, kArray_size(pToken->subTokenList));
			KReturn(SUGAR kStmt_AddExprParam(kctx, stmt, expr, pToken->subTokenList, 0, kArray_size(pToken->subTokenList), 0/*isAllowEmpty*/));
		}
	}
}

static kbool_t namespace_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("namespace"), 0, "\"namespace\" $Block", 0, 0, NULL, NULL, Statement_namespace, NULL, NULL, },
		{ SYM_("const"), 0, "\"const\" $Symbol \"=\" $Expr", 0, 0, NULL, NULL, Statement_ConstDecl, NULL, NULL, },
		{ SYM_("defined"), 0, NULL, 0, Precedence_CStylePREUNARY, NULL, Expression_Defined, NULL, NULL, TypeCheck_Defined, },
		{ KW_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}


// --------------------------------------------------------------------------

static kbool_t namespace_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	namespace_defineMethod(kctx, ns, trace);
	namespace_defineSyntax(kctx, ns, trace);
	return true;
}

static kbool_t namespace_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* ns_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", K_VERSION);
	d.PackupNameSpace    = namespace_PackupNameSpace;
	d.ExportNameSpace   = namespace_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
