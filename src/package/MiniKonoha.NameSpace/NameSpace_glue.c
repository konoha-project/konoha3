/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
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

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <konoha3/import/methoddecl.h>
#define TP_name         KType_String,     KFieldName_("name")
#define TP_paramsize    KType_Int,        KFieldName_("paramsize")
#define TP_source       KType_String,     KFieldName_("source")

//## NameSpace NameSpace.GetNameSpace();
static KMETHOD NameSpace_GetNameSpace(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnField(sfp[0].asNameSpace);
}

//## NameSpace NameSpace.GetParentNameSpace();
static KMETHOD NameSpace_GetParentNameSpace(KonohaContext *kctx, KonohaStack *sfp)
{
	kNameSpace *ns = (kNameSpace *)sfp[0].asNameSpace;
	if(ns->parentNULL != NULL) {
		ns = ns->parentNULL;
	}
	KReturnField(ns);
}

//## boolean NameSpace.DefineMacro(String symbol, String source);
static KMETHOD NameSpace_DefineMacro2(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = KAsciiSymbol(kString_text(sfp[1].asString), kString_size(sfp[1].asString), _NEWID);
	kString *source = sfp[2].asString;
	KReturnUnboxValue(SUGAR SetMacroData(kctx, sfp[0].asNameSpace, keyword, 0, kString_text(source), true));
}

//## boolean NameSpace.DefineMacro(String symbol, int param, String source);
static KMETHOD NameSpace_DefineMacro(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t keyword = KAsciiSymbol(kString_text(sfp[1].asString), kString_size(sfp[1].asString), _NEWID);
	int paramsize = (int)sfp[2].intValue;
	kString *source = sfp[3].asString;
	KReturnUnboxValue(SUGAR SetMacroData(kctx, sfp[0].asNameSpace, keyword, paramsize, kString_text(source), true));
}

static void namespace_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(NameSpace_GetNameSpace), KType_NameSpace, KType_NameSpace, KMethodName_("GetNameSpace"), 0,
		_Public|_Const|_Im, _F(NameSpace_GetParentNameSpace), KType_NameSpace, KType_NameSpace, KMethodName_("GetParentNameSpace"), 0,
		_Public, _F(NameSpace_DefineMacro2), KType_Boolean, KType_NameSpace, KMethodName_("DefineMacro"), 2, TP_name, TP_source,
		_Public, _F(NameSpace_DefineMacro), KType_Boolean, KType_NameSpace, KMethodName_("DefineMacro"), 3, TP_name, TP_paramsize, TP_source,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}


// --------------------------------------------------------------------------

static KMETHOD Statement_namespace(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, ns, reqc);
	kstatus_t result = K_CONTINUE;
	kToken *tk = SUGAR kNode_GetToken(kctx, stmt, KSymbol_BlockPattern, NULL);
	if(tk != NULL && tk->resolvedSyntaxInfo->keyword == TokenType_LazyBlock) {
		INIT_GCSTACK();
		kNameSpace *ns = new_(NameSpace, kNode_ns(stmt), _GcStack);
		KTokenSeq range = {ns, KGetParserContext(kctx)->preparedTokenList};
		KTokenSeq_Push(kctx, range);
		SUGAR Tokenize(kctx, ns, kString_text(tk->text), tk->uline, tk->indent, range.tokenList);
		KTokenSeq_End(kctx, range);
		result = SUGAR EvalTokenList(kctx, &range, NULL/*trace*/);
		KTokenSeq_Pop(kctx, range);
		RESET_GCSTACK();
		kNode_Type(kctx, stmt, KNode_Done, KType_void);
	}
	KReturnUnboxValue(result == K_CONTINUE);
}

/* const CONST = VALUE */

static KMETHOD Statement_ConstDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, ns, reqc);
	kToken *symbolToken = SUGAR kNode_GetToken(kctx, stmt, KSymbol_SymbolPattern, NULL);
	ksymbol_t unboxKey = symbolToken->symbol;
	kNode *constNode = SUGAR TypeCheckNodeByName(kctx, stmt, KSymbol_ExprPattern, ns, KClass_INFER, TypeCheckPolicy_CONST);
	if(!kNode_IsError(constNode)) {
		KClass *constClass = KClass_(constNode->attrTypeId);
		ktypeattr_t type = constClass->typeId;
		uintptr_t unboxValue;
		kbool_t result = false;
		if(kNode_node(constNode) == KNode_Null) {   // const C = String
			type = VirtualType_KClass;
			unboxValue = (uintptr_t)constClass;
			result = true;
		}
		else if(kNode_node(constNode) == KNode_Const) {   // const C = "1"
			unboxValue = (uintptr_t)constNode->ObjectConstValue;
			result = true;
		}
		else if(kNode_node(constNode) == KNode_UnboxConst) {  // const c = 1
			unboxValue = constNode->unboxConstValue;
			result = true;
		}
		if(result) {
			KMakeTraceUL(trace, sfp, kNode_uline(stmt));
			result = KLIB kNameSpace_SetConstData(kctx, ns, unboxKey, type, unboxValue, trace);
		}
		else {
			kNode_Message(kctx, stmt, ErrTag, "constant value is expected: %s%s", KSymbol_Fmt2(unboxKey));
		}
		constNode = kNode_Type(kctx, stmt, KNode_Done, KType_void);
	}
	KReturn(constNode);
}

/* defined (EXPR) */

static void FilterDefinedParam(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx)
{
	int i;
	for(i = beginIdx; i < endIdx; i++) {
		if(i + 1 == endIdx || tokenList->TokenItems[i+1]->resolvedSyntaxInfo->keyword == KSymbol_COMMA) {
			kTokenVar *tk = tokenList->TokenVarItems[i];
			if(tk->resolvedSyntaxInfo->keyword != KSymbol_SymbolPattern) {  // defined
				tk->resolvedSyntaxInfo = kSyntax_(ns, KSymbol_TextPattern);  // switch to text pattern
			}
			i++;
		}
		while(i < endIdx) {
			kTokenVar *tk = tokenList->TokenVarItems[i];
			i++;
			if(tk->resolvedSyntaxInfo->keyword == KSymbol_COMMA) break;
		}
	}
}


static KMETHOD Expression_Defined(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(expr, tokenList, beginIdx, currentIdx, endIdx);
	kNameSpace *ns = kNode_ns(expr);
	if(beginIdx == currentIdx && beginIdx + 1 < endIdx) {
		kTokenVar *definedToken = tokenList->TokenVarItems[beginIdx];   // defined
		kTokenVar *pToken = tokenList->TokenVarItems[beginIdx+1];
		if(IS_Array(pToken->GroupTokenList)) {
			SUGAR kNode_Op(kctx, expr, definedToken, 0);
			FilterDefinedParam(kctx, ns, RangeGroup(pToken->GroupTokenList));
			KReturn(SUGAR AppendParsedNode(kctx, expr, RangeGroup(pToken->GroupTokenList), NULL, ParseExpressionOption, "("));
		}
	}
}

static KMETHOD TypeCheck_Defined(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck2(stmt, expr, ns, reqc);
	size_t i;
	kbool_t isDefined = true;
	KParserContext *sugarContext = KGetParserContext(kctx);
	int popIsBlockingErrorMessage = sugarContext->isBlockedErrorMessage;
	sugarContext->isBlockedErrorMessage = true;
	for(i = 1; i < kArray_size(expr->NodeList); i++) {
		kNode *typedNode = SUGAR TypeCheckNodeAt(kctx, expr, i, ns, KClass_INFER, TypeCheckPolicy_AllowVoid);
		if(kNode_IsError(typedNode)) {
			isDefined = false;
			break;
		}
	}
	sugarContext->isBlockedErrorMessage = popIsBlockingErrorMessage;
	KReturn(SUGAR kNode_SetUnboxConst(kctx, expr, KType_Boolean, isDefined));
}

static kbool_t namespace_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("namespace"), SYNFLAG_CTypeFunc, 0, Precedence_Statement, {SUGAR patternParseFunc}, {SUGARFUNC Statement_namespace}},
		{ KSymbol_("const"), SYNFLAG_CTypeFunc, 0, Precedence_Statement, {SUGAR patternParseFunc}, {SUGARFUNC Statement_ConstDecl}},
		{ KSymbol_("defined"), SYNFLAG_CFunc,0, Precedence_CStylePrefixOperator, {SUGARFUNC Expression_Defined}, {SUGARFUNC TypeCheck_Defined},},
		{ KSymbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	SUGAR kSyntax_AddPattern(kctx, kSyntax_(ns, KSymbol_("namespace")), "\"namespace\" $Expr", 0, trace);
	SUGAR kSyntax_AddPattern(kctx, kSyntax_(ns, KSymbol_("const")), "\"const\" $Symbol = $Expr", 0, trace);
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

KDEFINE_PACKAGE *NameSpace_Init(void)
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
