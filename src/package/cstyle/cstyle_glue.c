///****************************************************************************
// * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
// * Redistribution and use in source and binary forms, with or without
// * modification, are permitted provided that the following conditions are met:
// *
// *  * Redistributions of source code must retain the above copyright notice,
// *    this list of conditions and the following disclaimer.
// *  * Redistributions in binary form must reproduce the above copyright
// *    notice, this list of conditions and the following disclaimer in the
// *    documentation and/or other materials provided with the distribution.
// *
// * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ***************************************************************************/
//
///* ************************************************************************ */
//
//#include <konoha3/konoha.h>
//#include <konoha3/sugar.h>
//#include <konoha3/klib.h>
//#include <konoha3/import/methoddecl.h>
//
//#ifdef __cplusplus
//extern "C" {
//#endif
//
///* Statement */
//
//
//static KMETHOD PatternMatch_Inc(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
//	int i;
//	ksymbol_t SYM_Inc = KSymbol_("++"), SYM_Dec = KSymbol_("--");
//	for(i = beginIdx; i < endIdx; i++) {
//		kTokenVar *tk = tokenList->TokenVarItems[i];
//		if(tk->resolvedSymbol == SYM_Inc || tk->resolvedSymbol == SYM_Dec) {
//			KReturnUnboxValue(beginIdx);
//		}
//		if(kToken_Is(StatementSeparator, tk) || kToken_IsIndent(tk)) {
//			break;
//		}
//	}
//	KReturnUnboxValue(-1);
//}
//
//static KMETHOD PatternMatch_IncExpr(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
//	int i, start, end;
//	ksymbol_t SYM_Inc = KSymbol_("++"), SYM_Dec = KSymbol_("--");
//	for(i = beginIdx; i < endIdx; i++) {
//		kToken *tk = tokenList->TokenItems[i];
//		if(tk->resolvedSymbol == SYM_Inc || tk->resolvedSymbol == SYM_Dec) {
//			break;
//		}
//	}
//	if(beginIdx == i) {
//		start = beginIdx + 1;
//		end = SUGAR kNameSpace_FindEndOfStatement(kctx, kStmt_ns(stmt), tokenList, i, endIdx);
//	}
//	else {
//		start = beginIdx;
//		end   = i;
//	}
//	if(start < end) {
//		kToken *opToken = tokenList->TokenItems[i];
//		KSyntax *opSyntax = opToken->resolvedSyntaxInfo;
//		KTokenSeq macro = {Stmt_ns(stmt), tokenList};
//		KTokenSeq_Push(kctx, macro);
//		KMacroSet macroParam[] = {
//			{KSymbol_("X"), tokenList, start, end},
//			{0, NULL, 0, 0},   /* sentinel */
//		};
//		macro.TargetPolicy.RemovingIndent = true;
//		SUGAR KTokenSeq_ApplyMacro(kctx, &macro, opSyntax->macroDataNULL, 0, kArray_size(opSyntax->macroDataNULL), opSyntax->macroParamSize, macroParam);
//		kExpr *expr = SUGAR kStmt_ParseExpr(kctx, stmt, macro.tokenList, macro.beginIdx, macro.endIdx, NULL);
//		if(expr != K_NULLEXPR) {
//			SUGAR kStmt_AddParsedObject(kctx, stmt, KSymbol_ExprPattern, UPCAST(expr));
//			((kStmtVar *)stmt)->syn = KSyntax_(Stmt_ns(stmt), KSymbol_ExprPattern);
//		}
//		KTokenSeq_Pop(kctx, macro);
//		end = SUGAR kNameSpace_FindEndOfStatement(kctx, kStmt_ns(stmt), tokenList, end+1, endIdx);
//		KReturnUnboxValue(end);
//	}
//	KReturnUnboxValue(-1);
//}
//
//static void cstyle_DefineStatement(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
//{
//	KDEFINE_SYNTAX SYNTAX[] = {
//		{ KSymbol_("$Inc"), 0, "$Inc $IncExpr", 0, 0, PatternMatch_Inc, NULL, NULL, NULL, NULL, },
//		{ KSymbol_("$IncExpr"), 0, NULL, 0, 0, PatternMatch_IncExpr, NULL, NULL, NULL, NULL, },
//		{ KSymbol_END, }, /* sentinental */
//	};
//	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
//}
//
///* ArrayLiteral */
//
//static KMETHOD Array_newList(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kArrayVar *a = (kArrayVar *)sfp[0].asObject;
//	size_t i = 0;
//	KonohaStack *p = sfp+1;
//	if(kArray_Is(UnboxData, a)) {
//		for(i = 0; p + i < kctx->esp; i++) {
//			a->unboxItems[i] = p[i].unboxValue;
//		}
//	}
//	else {
//		for(i = 0; p + i < kctx->esp; i++) {
//			KFieldSet(a, a->ObjectItems[i], p[i].asObject);
//		}
//	}
//	kArray_SetSize(a, i);
//	DBG_ASSERT(a->bytesize <= a->bytemax);
//	KReturn(a);
//}
//
//static KMETHOD TypeCheck_ArrayLiteral(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_TypeCheck(stmt, expr, gma, reqty);
//	kToken *termToken = expr->TermToken;
//	DBG_ASSERT(kExpr_IsTerm(expr) && IS_Token(termToken));
//	if(termToken->unresolvedTokenType == TokenType_CODE) {
//		SUGAR kToken_ToBraceGroup(kctx, (kTokenVar *)termToken, Stmt_ns(stmt), NULL);
//	}
//	if(termToken->resolvedSyntaxInfo->keyword == KSymbol_BraceGroup) {
//		kExprVar *arrayExpr = SUGAR new_UntypedCallStyleExpr(kctx, stmt->syn/*DUMMY*/, 2, K_NULL, K_NULL);
//		SUGAR kStmt_AddExprParam(kctx, stmt, arrayExpr, termToken->subTokenList, 0, kArray_size(termToken->subTokenList), NULL);
//		size_t i;
//		KClass *requestClass = KClass_(reqty);
//		KClass *paramType = KClass_INFER;
//		if(requestClass->baseTypeId == KType_Array) {
//			paramType = KClass_(requestClass->p0);
//		}
//		else {
//			requestClass = NULL; // undefined
//		}
//		for(i = 2; i < kArray_size(arrayExpr->NodeList); i++) {
//			kExpr *typedExpr = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, arrayExpr, i, gma, paramType, 0);
//			if(typedExpr == K_NULLEXPR) {
//				KReturn(typedExpr);
//			}
////			DBG_P("i=%d, paramType=%s, typedExpr->ty=%s", i, KType_text(paramType), KType_text(typedExpr->ty));
//			if(paramType->typeId == KType_var) {
//				paramType = KClass_(typedExpr->attrTypeId);
//			}
//		}
//		if(requestClass == NULL) {
//			requestClass = (paramType->typeId == KType_var) ? KClass_Array : KClass_p0(kctx, KClass_Array, paramType->typeId);
//		}
//		kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, Stmt_ns(stmt), KClass_Array, KMethodName_("{}"), -1, KMethodMatch_NoOption);
//		DBG_ASSERT(mtd != NULL);
//		KFieldSet(arrayExpr, arrayExpr->NodeList->MethodItems[0], mtd);
//		KFieldSet(arrayExpr, arrayExpr->NodeList->ExprItems[1], SUGAR kExpr_SetVariable(kctx, NULL, gma, TEXPR_NEW, requestClass->typeId, kArray_size(arrayExpr->NodeList) - 2));
//		KReturn(Expr_typed(arrayExpr, TEXPR_CALL, requestClass->typeId));
//	}
//}
//
//static kbool_t cstyle_defineArrayLiteral(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
//{
//	SUGAR kNameSpace_AddSugarFunc(kctx, ns, KSymbol_BlockPattern, SugarFunc_TypeCheck, new_SugarFunc(ns, TypeCheck_ArrayLiteral));
//	KDEFINE_METHOD MethodData[] = {
//		_Public|kMethod_Hidden, _F(Array_newList), KType_Array, KType_Array, KMethodName_("{}"), 0,
//		DEND,
//	};
//	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
//
//	return true;
//}
//
//
///* Literal */
//
//static KMETHOD TokenFunc_SingleQuotedChar(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kTokenVar *tk = (kTokenVar *)sfp[1].asObject;
//	int ch, prev = '/', pos = 1;
//	const char *source = kString_text(sfp[2].asString);
//	while((ch = source[pos++]) != 0) {
//		if(ch == '\n') {
//			break;
//		}
//		if(ch == '\'' && prev != '\\') {
//			if(IS_NOTNULL(tk)) {
//				KFieldSet(tk, tk->text, KLIB new_kString(kctx, OnField, source + 1, (pos-2), 0));
//				tk->unresolvedTokenType = KSymbol_("$SingleQuotedChar");
//			}
//			KReturnUnboxValue(pos);
//		}
//		prev = ch;
//	}
//	if(IS_NOTNULL(tk)) {
//		SUGAR kToken_ToError(kctx, tk, ErrTag, "must close with %s", "'");
//	}
//	KReturnUnboxValue(0);
//}
//
//static KMETHOD TypeCheck_SingleQuotedChar(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_TypeCheck(stmt, expr, gma, reqty);
//	kToken *tk = expr->TermToken;
//	if(kString_size(tk->text) == 1) {
//		int ch = kString_text(tk->text)[0];
//		KReturn(SUGAR kExpr_SetUnboxConstValue(kctx, expr, KType_Int, ch));
//	}
//	else if(kString_size(tk->text) == 2) {
//		int ch = kString_text(tk->text)[0];
//		if(ch == '\\') {
//			ch = kString_text(tk->text)[1];
//			switch(ch) {
//			case '\'': ch = '\''; break;
//			case '\\': ch = '\\'; break;
//			case 'b':  ch = '\b'; break;
//			case 'f':  ch = '\f'; break;
//			case 'n':  ch = '\n'; break;
//			case 'r':  ch = '\r'; break;
//			case 't':  ch = '\t'; break;
//			default:
//				KReturn(K_NULLEXPR);
//			}
//			KReturn(SUGAR kExpr_SetUnboxConstValue(kctx, expr, KType_Int, ch));
//		}
//	}
//	KReturn(K_NULLEXPR);
//}
//
///* Expression */
//
//static KMETHOD Expression_Indexer(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
//	KClass *genericsClass = NULL;
//	kNameSpace *ns = Stmt_ns(stmt);
//	int nextIdx = SUGAR TokenUtils_ParseTypePattern(kctx, ns, tokenList, beginIdx, endIdx, &genericsClass);
//	if(nextIdx != -1) {  // to avoid Func[T]
//		KReturn(SUGAR kStmt_ParseOperatorExpr(kctx, stmt, tokenList->TokenItems[beginIdx]->resolvedSyntaxInfo, tokenList, beginIdx, beginIdx, endIdx));
//	}
//	DBG_P("beginIdx=%d, endIdx=%d", beginIdx, endIdx);
//	kToken *currentToken = tokenList->TokenItems[operatorIdx];
//	if(beginIdx < operatorIdx) {
//		kExpr *leftExpr = SUGAR kStmt_ParseExpr(kctx, stmt, tokenList, beginIdx, operatorIdx, NULL);
//		if(leftExpr == K_NULLEXPR) {
//			KReturn(leftExpr);
//		}
//		/* transform 'Value0 [ Value1 ]=> (Call Value0 get (Value1)) */
//		kTokenVar *tkN = new_(TokenVar, 0, OnGcStack);
//		tkN->resolvedSymbol= KMethodName_ToGetter(0);
//		tkN->uline = currentToken->uline;
//		KSyntax *syn = KSyntax_(Stmt_ns(stmt), KSymbol_ExprMethodCall);
//		leftExpr  = SUGAR new_UntypedCallStyleExpr(kctx, syn, 2, tkN, leftExpr);
//		leftExpr = SUGAR kStmt_AddExprParam(kctx, stmt, leftExpr, currentToken->subTokenList, 0, kArray_size(currentToken->subTokenList), "[");
//		KReturn(SUGAR kStmt_RightJoinExpr(kctx, stmt, leftExpr, tokenList, operatorIdx + 1, endIdx));
//	}
//	DBG_P("nothing");
//}
//
//static KMETHOD Expression_Increment(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
//	kToken *tk = tokenList->TokenItems[operatorIdx];
//	KReturn(kStmtToken_Message(kctx, stmt, tk, ErrTag, "%s is defined as a statement", kString_text(tk->text)));
//}
//
//static void cstyle_defineExpression(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
//{
//	KDEFINE_SYNTAX SYNTAX[] = {
//		{ KSymbol_("[]"), SYNFLAG_ExprPostfixOp2, NULL, Precedence_CStyleCALL, 0, NULL, Expression_Indexer, NULL, NULL, NULL, },
//		{ KSymbol_("++"), SYNFLAG_ExprPostfixOp2, NULL, Precedence_CStyleCALL, Precedence_CStylePREUNARY, NULL, Expression_Increment,},
//		{ KSymbol_("--"), SYNFLAG_ExprPostfixOp2, NULL, Precedence_CStyleCALL, Precedence_CStylePREUNARY, NULL, Expression_Increment,},
//		{ KSymbol_END, }, /* sentinental */
//	};
//	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
//	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("++"), 1,  "X X = (X) + 1", false);
//	SUGAR kNameSpace_SetMacroData(kctx, ns, KSymbol_("--"), 1,  "X X = (X) - 1", false);
//}
//
///* ------------------------------------------------------------------------ */
//
//static kbool_t int_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
//{
//	KDEFINE_INT_CONST IntData[] = {
//		{"INT_MAX", KType_Int, KINT_MAX},
//		{"INT_MIN", KType_Int, KINT_MIN},
//		{NULL},
//	};
//	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(IntData), false/*isOverride*/, trace);
//	return true;
//}
//
//// --------------------------------------------------------------------------
///* Syntax */
//
//static char parseHexDigit(char c)
//{
//	return ('0' <= c && c <= '9') ? c - '0' :
//		('a' <= c && c <= 'f') ? c - 'a' + 10:
//		('A' <= c && c <= 'F') ? c - 'A' + 10:-1;
//}
//static char parseOctalDigit(char c)
//{
//	return ('0' <= c && c <= '7') ? c - '0' : -1;
//}
//static char parseDecimalDigit(char c)
//{
//	return ('0' <= c && c <= '9') ? c - '0' : -1;
//}
//
//static char parseBinaryDigit(char c)
//{
//	return ('0' == c || c == '1') ? c - '0' : -1;
//}
//
//static KMETHOD TokenFunc_ExtendedIntLiteral(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kTokenVar *tk = (kTokenVar *)sfp[1].asObject;
//	const char *source = kString_text(sfp[2].asString);
//	const char *start = source, *end;
//	int c = *source++;
//	/*
//	 * DIGIT  = 0-9
//	 * DIGITS = DIGIT | DIGIT DIGITS
//	 * HEX    = 0-9a-fA-F
//	 * HEXS   = HEX | HEX HEXS
//	 * BIN    = 0 | 1
//	 * BINS   = BIN | BIN BINS
//	 * TAG    = "0x"  | "0b"
//	 * HEXINT = ("0x" | "0X") HEXS
//	 * INT    = DIGITS | HEXS | BINS
//	 */
//	int base = 10;
//	bool isFloat = false;
//	char (*parseDigit)(char) = parseDecimalDigit;
//	if(c == '0') {
//		c = *source++;
//		switch (c) {
//			case 'b':
//				base = 2;  parseDigit = parseBinaryDigit; break;
//			case 'x':
//				base = 16; parseDigit = parseHexDigit; break;
//			case '0':case '1':case '2':case '3':
//			case '4':case '5':case '6':case '7':
//				base = 8; parseDigit = parseOctalDigit;
//				break;
//			default:
//				source--;
//				break;
//		}
//	}
//	for (; (c = *source) != 0; ++source) {
//		if(c == '_') continue;
//		if(parseDigit(c) == -1)
//			break;
//	}
//
//	/*
//	 * DIGIT  = 0-9
//	 * DIGITS = DIGIT | DIGIT DIGITS
//	 * INT    = DIGIT | DIGIT1-9 DIGITS
//	 * FLOAT  = INT
//	 *        | INT FRAC
//	 *        | INT EXP
//	 *        | INT FRAC EXP
//	 * FRAC   = "." digits
//	 * EXP    = E digits
//	 * E      = 'e' | 'e+' | 'e-' | 'E' | 'E+' | 'E-'
//	 */
//	if(base != 10 && c != '.' && c != 'e' && c != 'E') {
//		goto L_emit;
//	}
//	if(c == '.') {
//		isFloat = true;
//		source++;
//		for (; (c = *source) != 0; ++source) {
//			if(c == '_') continue;
//			if(parseDecimalDigit(c) == -1)
//				break;
//		}
//	}
//	if(c == 'e' || c == 'E') {
//		isFloat = true;
//		c = *(++source);
//		if(!('0' <= c && c <= '9') && !(c == '+' || c == '-')) {
//			source--;
//			goto L_emit;
//		}
//		if(c == '+' || c == '-') {
//			c = *source++;
//		}
//		for (; (c = *source) != 0; ++source) {
//			if(c == '_') continue;
//			if(parseDecimalDigit(c) == -1)
//				break;
//		}
//	}
//
//	L_emit:;
//	if(IS_NOTNULL(tk)) {
//		/* skip unit */
//		for (; (c = *source) != 0; ++source) {
//			if(c == '_') continue;
//			if(!isalpha(c))
//				break;
//		}
//		end = source;
//		KFieldSet(tk, tk->text, KLIB new_kString(kctx, OnField, start, end - start, StringPolicy_ASCII));
//		tk->unresolvedTokenType = isFloat ? KSymbol_("$Float") : TokenType_INT;
//	}
//	KReturnUnboxValue(source - start);
//}
//
//static kint_t _kstrtoll(const char *p, char (*parseDigit)(char), int base)
//{
//	long long tmp = 0, prev = 0;
//	char c;
//	for (; (c = *p) != 0; ++p) {
//		if(c == '_') continue;
//		c = parseDigit(c);
//		if(c == -1)
//			break;
//		tmp = tmp * base + c;
//		if(tmp < prev) {
//			/* Overflow!! */
//			return 0;
//		}
//		prev = tmp;
//	}
//	return (kint_t) tmp;
//}
//
//static kint_t kstrtoll(const char *p)
//{
//	if(*p == '0') {
//		if(*(p+1) == 'x' || *(p+1) == 'X') {
//		return _kstrtoll(p+2, parseHexDigit, 16);
//		}
//		if(*(p+1) == 'b') {
//			return _kstrtoll(p+2, parseBinaryDigit, 2);
//		}
//		if('0' <= *(p+1) && *(p+1) <= '7') {
//			return _kstrtoll(p+1, parseOctalDigit, 8);
//		}
//	}
//	return _kstrtoll(p, parseDecimalDigit, 10);
//}
//
//static KMETHOD TypeCheck_ExtendedIntLiteral(KonohaContext *kctx, KonohaStack *sfp)
//{
//	VAR_TypeCheck(stmt, expr, gma, reqty);
//	kToken *tk = expr->TermToken;
//	long long n = kstrtoll(kString_text(tk->text));
//	KReturn(SUGAR kExpr_SetUnboxConstValue(kctx, expr, KType_Int, (uintptr_t)n));
//}
//
//static kbool_t int_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
//{
//
//	KDEFINE_SYNTAX SYNTAX[] = {
//		{ KSymbol_NumberPattern, 0,  NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_ExtendedIntLiteral, },
//		{ KSymbol_END, },
//	};
//	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
//
//	SUGAR kNameSpace_SetTokenFunc(kctx, ns, KSymbol_NumberPattern, KonohaChar_Digit, new_SugarFunc(ns, TokenFunc_ExtendedIntLiteral));
//	return true;
//}
//
//// --------------------------------------------------------------------------
//
//static kbool_t cstyle_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
//{
//	cstyle_DefineStatement(kctx, ns, trace);
//	KDEFINE_SYNTAX defLiteral[] = {
//		{ KSymbol_("$SingleQuotedChar"), 0, NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_SingleQuotedChar, },
//		{ KSymbol_END, }, /* sentinental */
//	};
//	SUGAR kNameSpace_DefineSyntax(kctx, ns, defLiteral, trace);
//	SUGAR kNameSpace_SetTokenFunc(kctx, ns, KSymbol_("$SingleQuotedChar"), KonohaChar_Quote, new_SugarFunc(ns, TokenFunc_SingleQuotedChar));
//
//	cstyle_defineExpression(kctx, ns, option, trace);
//	cstyle_defineArrayLiteral(kctx, ns, trace);
//
//	int_defineMethod(kctx, ns, trace);
//	int_defineSyntax(kctx, ns, trace);
//	return true;
//}
//
//static kbool_t cstyle_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
//{
//	KDEFINE_INT_CONST ClassData[] = {   // long as alias
//		{"long", VirtualType_KClass, (uintptr_t)KClass_Int},
//		{NULL},
//	};
//	KLIB kNameSpace_LoadConstData(kctx, exportNS, KConst_(ClassData), false/*isOverride*/, trace);
//	return true;
//}
//
//// --------------------------------------------------------------------------
//
//KDEFINE_PACKAGE *cstyle_Init(void)
//{
//	static KDEFINE_PACKAGE d = {0};
//	KSetPackageName(d, "cstyle", "1.0");
//	d.PackupNameSpace    = cstyle_PackupNameSpace;
//	d.ExportNameSpace   = cstyle_ExportNameSpace;
//	return &d;
//}
//
//// --------------------------------------------------------------------------
//
//#ifdef __cplusplus
//}
//#endif
