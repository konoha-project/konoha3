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

#ifdef __cplusplus
extern "C" {
#endif

static void kStmt_AddParsedObject(KonohaContext *kctx, kStmt *stmt, ksymbol_t keyid, kObject *o)
{
	kArray* valueList = (kArray *)KLIB kObject_getObject(kctx, stmt, keyid, NULL);
	if(valueList == NULL) {
		KLIB kObjectProto_SetObject(kctx, stmt, keyid, O_typeId(o), o);
	}
	else {
		//DBG_P(">>> keyid=%s%s valueList=%s, value=%s", PSYM_t(keyid), CT_t(O_ct(valueList)), CT_t(O_ct(o)));
		if(!IS_Array(valueList)) {
			INIT_GCSTACK();
			kArray *newList = /*G*/new_(Array, 0, _GcStack);
			KLIB kArray_Add(kctx, newList, valueList);
			KLIB kObjectProto_SetObject(kctx, stmt, keyid, O_typeId(newList), newList);
			valueList = newList;
			RESET_GCSTACK();
		}
		KLIB kArray_Add(kctx, valueList, o);
	}
}

static int kNameSpace_FindEndOfStatement(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx)
{
	int c, isNoSemiColon = kNameSpace_IsAllowed(NoSemiColon, ns);
	for(c = beginIdx; c < endIdx; c++) {
		kToken *tk = tokenList->TokenItems[c];
		if(kToken_is(StatementSeparator, tk)) return c;
		if(isNoSemiColon && kToken_isIndent(tk)) {
			return c;
		}
	}
	return endIdx;
}

static KMETHOD PatternMatch_Expression(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	int returnIdx = -1;
	endIdx = kNameSpace_FindEndOfStatement(kctx, Stmt_ns(stmt), tokenList, beginIdx, endIdx);
	if(beginIdx < endIdx) {
		INIT_GCSTACK();
		kExpr *expr = kStmt_ParseExpr(kctx, stmt, tokenList, beginIdx, endIdx, NULL);
		if(expr != K_NULLEXPR) {
			//KdumpExpr(kctx, expr);
			kStmt_AddParsedObject(kctx, stmt, name, UPCAST(expr));
			returnIdx = endIdx;
		}
		RESET_GCSTACK();
	}
	KReturnUnboxValue(returnIdx);
}

static KMETHOD PatternMatch_Type(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	KonohaClass *foundClass = NULL;
	int returnIdx = TokenUtils_ParseTypePattern(kctx, Stmt_ns(stmt), tokenList, beginIdx, endIdx, &foundClass);
	DBG_P("tk=%s, returnIdx=%d", tokenList->TokenItems[beginIdx], returnIdx);
	if(foundClass != NULL) {
		kTokenVar *tk = new_(TokenVar, 0, OnVirtualField);
		kStmt_AddParsedObject(kctx, stmt, name, UPCAST(tk));
		kToken_setTypeId(kctx, tk, Stmt_ns(stmt), foundClass->typeId);
	}
	KReturnUnboxValue(returnIdx);
}

static KMETHOD PatternMatch_MethodName(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	kTokenVar *tk = tokenList->TokenVarItems[beginIdx];
	int returnIdx = -1;
	if(tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern || tk->resolvedSyntaxInfo->precedence_op1 > 0 || tk->resolvedSyntaxInfo->precedence_op2 > 0) {
		kStmt_AddParsedObject(kctx, stmt, name, UPCAST(tk));
		returnIdx = beginIdx + 1;
	}
	KReturnUnboxValue(returnIdx);
}

static void TokenSeq_CheckCStyleParam(KonohaContext *kctx, TokenSeq* tokens)
{
	int i;
	for(i = 0; i < tokens->endIdx; i++) {
		kTokenVar *tk = tokens->tokenList->TokenVarItems[i];
		if(tk->resolvedSymbol == KW_void) {
			tokens->endIdx = i; //  f(void) = > f()
			return;
		}
		if(tk->resolvedSyntaxInfo->keyword == KW_COMMA) {
			kToken_set(StatementSeparator, tk, true);
		}
	}
}

static KMETHOD PatternMatch_CStyleParam(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	int returnIdx = -1;
	kToken *tk = tokenList->TokenItems[beginIdx];
	if(tk->resolvedSyntaxInfo->keyword == KW_ParenthesisGroup) {
		TokenSeq param = {Stmt_ns(stmt), tk->subTokenList, 0, kArray_size(tk->subTokenList)};
		TokenSeq_CheckCStyleParam(kctx, &param);
		kBlock *bk = new_kBlock(kctx, stmt, NULL, &param);
		kStmt_AddParsedObject(kctx, stmt, name, UPCAST(bk));
		returnIdx = beginIdx + 1;
	}
	KReturnUnboxValue(returnIdx);
}

static KMETHOD PatternMatch_CStyleBlock(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	kToken *tk = tokenList->TokenItems[beginIdx];
//	KdumpTokenArray(kctx, tokenList, beginIdx, endIdx);
	if(tk->resolvedSyntaxInfo->keyword == TokenType_CODE || tk->resolvedSyntaxInfo->keyword == KW_BraceGroup) {
		kStmt_AddParsedObject(kctx, stmt, name, UPCAST(tk));
		KReturnUnboxValue(beginIdx+1);
	}
	int newEndIdx = kNameSpace_FindEndOfStatement(kctx, Stmt_ns(stmt), tokenList, beginIdx, endIdx);
	TokenSeq tokens = {Stmt_ns(stmt), tokenList, beginIdx, newEndIdx};
	kBlock *bk = new_kBlock(kctx, stmt, NULL, &tokens);
	kStmt_AddParsedObject(kctx, stmt, name, UPCAST(bk));
	KReturnUnboxValue(newEndIdx);
}

static KMETHOD PatternMatch_Token(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	DBG_ASSERT(beginIdx < endIdx);
	kToken *tk = tokenList->TokenItems[beginIdx];
	if(!kToken_is(StatementSeparator, tk) && !kToken_isIndent(tk)) {
		kStmt_AddParsedObject(kctx, stmt, name, UPCAST(tk));
		KReturnUnboxValue(beginIdx+1);
	}
	KReturnUnboxValue(-2);
}

static KMETHOD PatternMatch_TypeDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	KonohaClass *foundClass = NULL;
	int nextIdx = TokenUtils_ParseTypePattern(kctx, Stmt_ns(stmt), tokenList, beginIdx, endIdx, &foundClass);
	//DBG_P("@ nextIdx = %d < %d", nextIdx, endIdx);
	if(nextIdx != -1) {
		nextIdx = TokenUtils_SkipIndent(tokenList, nextIdx, endIdx);
		if(nextIdx < endIdx) {
			kToken *tk = tokenList->TokenItems[nextIdx];
			if(tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern) {
				KReturnUnboxValue(beginIdx);
			}
		}
	}
	KReturnUnboxValue(-1);
}

static KMETHOD PatternMatch_MethodDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	kNameSpace *ns = Stmt_ns(stmt);
	KonohaClass *foundClass = NULL;
	int nextIdx = TokenUtils_ParseTypePattern(kctx, ns, tokenList, beginIdx, endIdx, &foundClass);
	//DBG_P("@ nextIdx = %d < %d", nextIdx, endIdx);
	if(nextIdx != -1) {
		nextIdx = TokenUtils_SkipIndent(tokenList, nextIdx, endIdx);
		if(nextIdx < endIdx) {
			kToken *tk = tokenList->TokenItems[nextIdx];
			if(TokenUtils_ParseTypePattern(kctx, ns, tokenList, nextIdx, endIdx, NULL) != -1) {
				KReturnUnboxValue(beginIdx);
			}
			if(tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern) {
				int symbolNextIdx = TokenUtils_SkipIndent(tokenList, nextIdx + 1, endIdx);
				if(symbolNextIdx < endIdx && tokenList->TokenItems[symbolNextIdx]->resolvedSyntaxInfo->keyword == KW_ParenthesisGroup) {
					KReturnUnboxValue(beginIdx);
				}
				KReturnUnboxValue(-1);
			}
			if(tk->resolvedSyntaxInfo->keyword != KW_DOT && ((tk->resolvedSyntaxInfo->precedence_op1 > 0 || tk->resolvedSyntaxInfo->precedence_op2 > 0))) {
				KReturnUnboxValue(beginIdx);
			}
		}
	}
	KReturnUnboxValue(-1);
}

/* ------------------------------------------------------------------------ */

static KMETHOD Expression_ParsedExpr(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	if(beginIdx == operatorIdx) {
		kToken *tk = tokenList->TokenItems[operatorIdx];
		DBG_ASSERT(IS_Expr(tk->parsedExpr));
		KReturn(tk->parsedExpr);
	}
}

static KMETHOD Expression_Term(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	if(beginIdx == operatorIdx) {
		kToken *tk = tokenList->TokenItems[operatorIdx];
		KonohaClass *foundClass = NULL;
		int nextIdx = TokenUtils_ParseTypePattern(kctx, Stmt_ns(stmt), tokenList, beginIdx, endIdx, &foundClass);
		if(foundClass != NULL) {
			kToken_setTypeId(kctx, tk, Stmt_ns(stmt), foundClass->typeId);
		}
		else {
			nextIdx = operatorIdx + 1;
		}
		KReturn(kStmt_RightJoinExpr(kctx, stmt, new_TermExpr(kctx, tk), tokenList, nextIdx, endIdx));
	}
}

static KMETHOD Expression_OperatorMethod(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	if(/*syn->keyword != KW_LET && */syn->sugarFuncTable[SugarFunc_TypeCheck] == NULL) {
		DBG_P("switching type checker of %s%s to MethodCall ..", PSYM_t(syn->keyword));
		syn = SYN_(Stmt_ns(stmt), KW_ExprMethodCall);  // switch type checker
	}
	kTokenVar *tk = tokenList->TokenVarItems[operatorIdx];
	kExpr *expr, *rexpr = kStmt_ParseExpr(kctx, stmt, tokenList, operatorIdx + 1, endIdx, KToken_t(tk));
	if(beginIdx == operatorIdx) { // unary operator
		expr = new_UntypedCallStyleExpr(kctx, syn, 2, tk, rexpr);
	}
	else {   // binary operator
		kExpr *lexpr = kStmt_ParseExpr(kctx, stmt, tokenList, beginIdx, operatorIdx, NULL);
		expr = new_UntypedCallStyleExpr(kctx, syn, 3, tk, lexpr, rexpr);
	}
	KReturn(expr);
}

static inline kbool_t isFieldName(kArray *tokenList, int operatorIdx, int endIdx)
{
	if(operatorIdx + 1 < endIdx) {
		kToken *tk = tokenList->TokenItems[operatorIdx + 1];
		return (tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern);
	}
	return false;
}

static KMETHOD Expression_DOT(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	if(beginIdx < operatorIdx && isFieldName(tokenList, operatorIdx, endIdx)) {
		kExpr *expr = kStmt_ParseExpr(kctx, stmt, tokenList, beginIdx, operatorIdx, NULL);
		expr = new_UntypedCallStyleExpr(kctx, syn, 2, tokenList->TokenItems[operatorIdx +1], expr);
		KReturn(kStmt_RightJoinExpr(kctx, stmt, expr, tokenList, operatorIdx +2, endIdx));
	}
}

static KMETHOD Expression_Parenthesis(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	kToken *tk = tokenList->TokenItems[operatorIdx];
	if(beginIdx == operatorIdx) {
		kExpr *expr = kStmt_ParseExpr(kctx, stmt, tk->subTokenList, 0, kArray_size(tk->subTokenList), "(");
		KReturn(kStmt_RightJoinExpr(kctx, stmt, expr, tokenList, operatorIdx + 1, endIdx));
	}
	else {
		kExpr *lexpr = kStmt_ParseExpr(kctx, stmt, tokenList, beginIdx, operatorIdx, NULL);
		if(lexpr == K_NULLEXPR) {
			KReturn(lexpr);
		}
		if(lexpr->syn->keyword == KW_DOT) {
			((kExprVar *)lexpr)->syn = SYN_(Stmt_ns(stmt), KW_ExprMethodCall); // CALL
		}
		else if(lexpr->syn->keyword != KW_ExprMethodCall) {
			syn = SYN_(Stmt_ns(stmt), KW_ParenthesisGroup);    // (f null ())
			lexpr  = new_UntypedCallStyleExpr(kctx, syn, 2, lexpr, K_NULL);
		}
		if(kArray_size(tk->subTokenList) > 0) {
			lexpr = kStmt_AddExprParam(kctx, stmt, lexpr, tk->subTokenList, 0, kArray_size(tk->subTokenList), "(");
		}
		KReturn(kStmt_RightJoinExpr(kctx, stmt, lexpr, tokenList, operatorIdx + 1, endIdx));
	}
}

static KMETHOD Expression_COMMA(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	kExpr *expr = new_UntypedCallStyleExpr(kctx, syn, 1, tokenList->TokenItems[operatorIdx]);
	expr = kStmt_AddExprParam(kctx, stmt, expr, tokenList, beginIdx, endIdx, 0/*allowEmpty*/);
	KReturn(expr);
}

static KMETHOD Expression_DOLLAR(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	if(beginIdx == operatorIdx && operatorIdx +1 < endIdx) {
		kToken *tk = tokenList->TokenItems[operatorIdx +1];
		if(tk->resolvedSyntaxInfo->keyword == TokenType_CODE) {
			kToken_ToBraceGroup(kctx, (kTokenVar *)tk, Stmt_ns(stmt), NULL);
		}
		if(tk->resolvedSyntaxInfo->keyword == KW_BraceGroup) {
			kExprVar *expr = new_(ExprVar, SYN_(Stmt_ns(stmt), KW_BlockPattern), OnGcStack);
			TokenSeq range = {Stmt_ns(stmt), tk->subTokenList, 0, kArray_size(tk->subTokenList)};
			KFieldSet(expr, expr->block, SUGAR new_kBlock(kctx, stmt, NULL, &range));
			KReturn(expr);
		}
	}
}

static kExpr* NewExpr(KonohaContext *kctx, SugarSyntax *syn, kToken *tk, kattrtype_t ty)
{
	kExprVar *expr = new_(ExprVar, syn, OnGcStack);
	KFieldSet(expr, expr->termToken, tk);
	Expr_setTerm(expr, 1);
	expr->build = TEXPR_NEW;
	expr->attrTypeId = ty;
	return (kExpr *)expr;
}

static KMETHOD Expression_new(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, currentIdx, endIdx);
	DBG_ASSERT(beginIdx == currentIdx);
	if(beginIdx + 1 < endIdx) {
		kTokenVar *newToken = tokenList->TokenVarItems[beginIdx];   // new Class (
		KonohaClass *foundClass = NULL;
		kNameSpace *ns = Stmt_ns(stmt);
		int nextIdx = SUGAR TokenUtils_ParseTypePattern(kctx, ns, tokenList, beginIdx + 1, endIdx, &foundClass);
		if(foundClass == NULL) {
			kToken *classNameToken = tokenList->TokenVarItems[beginIdx+1];
			KReturn(SUGAR kStmt_Message2(kctx, stmt, classNameToken, ErrTag, "not class: %s", KToken_t(classNameToken)));
		}
		if((size_t)nextIdx < kArray_size(tokenList)) {
			kToken *nextTokenAfterClassName = tokenList->TokenItems[nextIdx];
			if(nextTokenAfterClassName->resolvedSyntaxInfo->keyword == KW_ParenthesisGroup) {  // new C (...)
				SugarSyntax *syn = SYN_(ns, KW_ExprMethodCall);
				kExpr *expr = SUGAR new_UntypedCallStyleExpr(kctx, syn, 2, newToken, NewExpr(kctx, syn, tokenList->TokenVarItems[beginIdx+1], foundClass->typeId));
				newToken->resolvedSymbol = MN_new;
				KReturn(expr);
			}
		}
	}
}

/* ------------------------------------------------------------------------ */
/* Expression TyCheck */

static kString *kToken_ResolveEscapeSequence(KonohaContext *kctx, kToken *tk, size_t start)
{
	KGrowingBuffer wb;
	KLIB Kwb_Init(&(kctx->stack->cwb), &wb);
	const char *text = S_text(tk->text) + start;
	const char *end  = S_text(tk->text) + S_size(tk->text);
	KLIB Kwb_Write(kctx, &wb, S_text(tk->text), start);
	while(text < end) {
		int ch = *text;
		if(ch == '\\' && *(text+1) != '\0') {
			switch (*(text+1)) {
			/*
			 * compatible with ECMA-262
			 * http://ecma-international.org/ecma-262/5.1/#sec-7.8.4
			 */
			case 'b':  ch = '\b'; text++; break;
			case 't':  ch = '\t'; text++; break;
			case 'n':  ch = '\n'; text++; break;
			case 'v':  ch = '\v'; text++; break;
			case 'f':  ch = '\f'; text++; break;
			case 'r':  ch = '\r'; text++; break;
			case '"':  ch = '"';  text++; break;
			case '\'': ch = '\''; text++; break;
			case '\\': ch = '\\'; text++; break;
			default:	return NULL;
			}
		}
		{
			char buf[1] = {ch};
			KLIB Kwb_Write(kctx, &wb, (const char *)buf, 1);
		}
		text++;
	}
	kString *s = KLIB new_kString(kctx, OnGcStack, KLIB Kwb_top(kctx, &wb, 1), Kwb_bytesize(&wb), 0);
	KLIB Kwb_Free(&wb);
	return s;
}

static KMETHOD TypeCheck_TextLiteral(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->termToken;
	kString *text = tk->text;
	if(kToken_is(RequiredReformat, tk)) {
		const char *escape = strchr(S_text(text), '\\');
		DBG_ASSERT(escape != NULL);
		text = kToken_ResolveEscapeSequence(kctx, tk, escape - S_text(text));
		if(text == NULL) {
			KReturn(ERROR_UndefinedEscapeSequence(kctx, stmt, tk));
		}
	}
	kString_set(Literal, ((kStringVar *)text), true);
	KReturn(SUGAR kExpr_SetConstValue(kctx, expr, NULL, UPCAST(text)));
}

static KMETHOD TypeCheck_Type(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	DBG_ASSERT(Token_isVirtualTypeLiteral(expr->termToken));
	KReturn(SUGAR kExpr_SetVariable(kctx, expr, gma, TEXPR_NULL, expr->termToken->resolvedTypeId, 0));
}

static KMETHOD TypeCheck_true(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	KReturn(SUGAR kExpr_SetUnboxConstValue(kctx, expr, TY_boolean, (uintptr_t)1));
}

static KMETHOD TypeCheck_false(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	KReturn(SUGAR kExpr_SetUnboxConstValue(kctx, expr, TY_boolean, (uintptr_t)0));
}

static KMETHOD TypeCheck_IntLiteral(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->termToken;
	long long n = strtoll(S_text(tk->text), NULL, 0);
	KReturn(SUGAR kExpr_SetUnboxConstValue(kctx, expr, TY_int, (uintptr_t)n));
}

static KMETHOD TypeCheck_AndOperator(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	if(SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 1, gma, CT_Boolean, 0) != K_NULLEXPR) {
		if(SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 2, gma, CT_Boolean, 0) != K_NULLEXPR) {
			KReturn(kExpr_typed(expr, AND, TY_boolean));
		}
	}
}

static KMETHOD TypeCheck_OrOperator(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	if(SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 1, gma, CT_Boolean, 0) != K_NULLEXPR) {
		if(SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 2, gma, CT_Boolean, 0) != K_NULLEXPR) {
			KReturn(kExpr_typed(expr, OR, TY_boolean));
		}
	}
}

static kbool_t kExpr_IsGetter(kExpr *expr)
{
	if(expr->build == TEXPR_CALL) {  // check getter and transform to setter
		kMethod *mtd = expr->cons->MethodItems[0];
		DBG_ASSERT(IS_Method(mtd));
		if(MN_isGETTER(mtd->mn)) return true;
	}
	return false;
}

static kExpr* kStmtExpr_ToSetter(KonohaContext *kctx, kStmt *stmt, kExprVar *expr, kGamma *gma, kExpr *rightHandExpr, KonohaClass *reqClass)
{
	kMethod *mtd = expr->cons->MethodItems[0];
	DBG_ASSERT(MN_isGETTER(mtd->mn));
	kNameSpace *ns = Stmt_ns(stmt);  // leftHandExpr = rightHandExpr
	KonohaClass *c = CT_(mtd->typeId);
	kParam *pa = kMethod_GetParam(mtd);
	int i, psize = pa->psize + 1;
	kparamtype_t p[psize];
	for(i = 0; i < pa->psize; i++) {
		p[i].attrTypeId = pa->paramtypeItems[i].attrTypeId;
	}
	p[pa->psize].attrTypeId = expr->attrTypeId;
	kparamId_t paramdom = KLIB Kparamdom(kctx, psize, p);
	kMethod *foundMethod = kNameSpace_GetMethodBySignatureNULL(kctx, ns, c, MN_toSETTER(mtd->mn), paramdom, psize, p);
	if(foundMethod != NULL) {
		p[pa->psize].attrTypeId = pa->rtype;   /* transform "T1 A.get(T2)" to "void A.set(T2, T1)" */
		paramdom = KLIB Kparamdom(kctx, psize, p);
		foundMethod = kNameSpace_GetMethodBySignatureNULL(kctx, ns, c, MN_toSETTER(mtd->mn), paramdom, psize, p);
	}
	if(foundMethod != NULL) {
		KFieldSet(expr->cons, expr->cons->MethodItems[0], foundMethod);
		KLIB kArray_Add(kctx, expr->cons, rightHandExpr);
		return SUGAR kStmtkExpr_TypeCheckCallParam(kctx, stmt, expr, foundMethod, gma, reqClass);
	}
	return SUGAR kStmt_Message2(kctx, stmt, (kToken *)expr, ErrTag, "undefined setter");
}

static KMETHOD TypeCheck_Assign(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kExpr *leftHandExpr = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 1, gma, CT_INFER, TypeCheckPolicy_ALLOWVOID);
	kExpr *rightHandExpr = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 2, gma, CT_(leftHandExpr->attrTypeId), 0);
	kExpr *returnExpr = K_NULLEXPR;
	if(rightHandExpr != K_NULLEXPR && leftHandExpr != K_NULLEXPR) {
		if(leftHandExpr->build == TEXPR_LOCAL || leftHandExpr->build == TEXPR_FIELD || leftHandExpr->build == TEXPR_STACKTOP) {
			if(TypeAttr_Is(ReadOnly, leftHandExpr->attrTypeId)) {
				returnExpr = SUGAR kStmt_Message2(kctx, stmt, (kToken *)expr, ErrTag, "read only: %s", KToken_t(leftHandExpr->termToken));
			}
			else {
				expr->build      = TEXPR_LET;
				expr->attrTypeId = leftHandExpr->attrTypeId;
				//((kExprVar *)rightHandExpr)->attrTypeId = leftHandExpr->attrTypeId;
				returnExpr = expr;
			}
		}
		else if(kExpr_IsGetter(leftHandExpr)) {
			returnExpr = kStmtExpr_ToSetter(kctx, stmt, (kExprVar *)leftHandExpr, gma, rightHandExpr, CT_(reqty));
		}
		else {
			returnExpr = SUGAR kStmt_Message2(kctx, stmt, (kToken *)expr, ErrTag, "assignment: variable name is expected");
		}
	}
	KReturn(returnExpr);
}

static int kGamma_AddLocalVariable(KonohaContext *kctx, kGamma *gma, kattrtype_t attrTypeId, ksymbol_t name)
{
	GammaStack *s = &gma->genv->localScope;
	int index = s->varsize;
	if(!(s->varsize < s->capacity)) {
		s->capacity *= 2;
		size_t asize = sizeof(GammaStackDecl) * s->capacity;
		GammaStackDecl *v = (GammaStackDecl *)KMalloc_UNTRACE(asize);
		memcpy(v, s->varItems, asize/2);
		if(s->allocsize > 0) {
			KFree(s->varItems, s->allocsize);
		}
		s->varItems = v;
		s->allocsize = asize;
	}
	s->varItems[index].attrTypeId = attrTypeId;
	s->varItems[index].name = name;
	s->varsize += 1;
	return index;
}

static KMETHOD TypeCheck_Block(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kBlock *bk = expr->block;
	if(IS_Block(bk)) {  // this is special case of ${} by set internally
		kExpr *texpr = K_NULLEXPR;
		kStmt *lastExpr = NULL;
		kfileline_t uline = stmt->uline;
		if(kArray_size(bk->StmtList) > 0) {
			kStmt *stmt = bk->StmtList->StmtItems[kArray_size(bk->StmtList)-1];
			if(stmt->syn->keyword == KW_ExprPattern) {
				lastExpr = stmt;
			}
			uline = stmt->uline;
		}
		if(lastExpr != NULL) {
			size_t lvarsize = gma->genv->localScope.varsize;
			int popBlockScopeShiftSize = gma->genv->blockScopeShiftSize;
			gma->genv->blockScopeShiftSize = lvarsize;
			if(!kBlock_TypeCheckAll(kctx, bk, gma)) {
				KReturn(texpr);
			}
			kExpr *lvar = new_VariableExpr(kctx, gma, TEXPR_LOCAL, TY_var, kGamma_AddLocalVariable(kctx, gma, TY_var, 0/*FN_*/));
			kExpr *rexpr = SUGAR kStmt_GetExpr(kctx, lastExpr, KW_ExprPattern, NULL);
			DBG_ASSERT(rexpr != NULL);
			kattrtype_t ty = rexpr->attrTypeId;
			if(ty != TY_void) {
				kExpr *letexpr = new_TypedConsExpr(kctx, TEXPR_LET, CT_(TY_void), 3, K_NULL, lvar, rexpr);
				KLIB kObjectProto_SetObject(kctx, lastExpr, KW_ExprPattern, TY_Expr, letexpr);
				texpr = SUGAR kExpr_SetVariable(kctx, expr, gma, TEXPR_BLOCK, ty, lvarsize);
			}
			gma->genv->blockScopeShiftSize = popBlockScopeShiftSize;
			if(lvarsize < gma->genv->localScope.varsize) {
				gma->genv->localScope.varsize = lvarsize;
			}
		}
		if(texpr == K_NULLEXPR) {
			((kStmtVar *)stmt)->uline = uline;
			kStmt_Message(kctx, stmt, ErrTag, "block has no value");
		}
		KReturn(texpr);
	}
	//kStmtExpr_Message(kctx, stmt, expr, ErrTag, "undefined expression: %s", KToken_t(expr->termToken));
}

static kExpr* new_GetterExpr(KonohaContext *kctx, kToken *tkU, kMethod *mtd, kExpr *expr)
{
	return new_TypedConsExpr(kctx, TEXPR_CALL, kMethod_GetReturnType(mtd), 2, mtd, expr);
}

static kExpr* kStmt_TypeCheckVariableNULL(KonohaContext *kctx, kStmt *stmt, kExprVar *expr, kGamma *gma, KonohaClass *reqClass)
{
	DBG_ASSERT(expr->attrTypeId == TY_var);
	kToken *tk = expr->termToken;
	ksymbol_t symbol = tk->resolvedSymbol;
	kNameSpace *ns = Stmt_ns(stmt);
	int i;
	GammaAllocaData *genv = gma->genv;
	for(i = genv->localScope.varsize - 1; i >= 0; i--) {
		if(genv->localScope.varItems[i].name == symbol) {
			return SUGAR kExpr_SetVariable(kctx, expr, gma, TEXPR_LOCAL, genv->localScope.varItems[i].attrTypeId, i);
		}
	}
	if(kNameSpace_IsAllowed(ImplicitField, ns)) {
		if(genv->localScope.varItems[0].attrTypeId != TY_void) {
			KonohaClass *ct = genv->thisClass;
			if(ct->fieldsize > 0) {
				for(i = ct->fieldsize; i >= 0; i--) {
					if(ct->fieldItems[i].name == symbol && ct->fieldItems[i].attrTypeId != TY_void) {
						return SUGAR kExpr_SetVariable(kctx, expr, gma, TEXPR_FIELD, ct->fieldItems[i].attrTypeId, longid((kshort_t)i, 0));
					}
				}
			}
			kMethod *mtd = kNameSpace_GetGetterMethodNULL(kctx, ns, genv->thisClass, symbol);
			if(mtd != NULL) {
				return new_GetterExpr(kctx, tk, mtd, new_VariableExpr(kctx, gma, TEXPR_LOCAL, genv->thisClass->typeId, 0));
			}
		}
	}
	if((Gamma_isTopLevel(gma) || kNameSpace_IsAllowed(ImplicitGlobalVariable, ns)) && ns->globalObjectNULL_OnList != NULL) {
		KonohaClass *globalClass = O_ct(ns->globalObjectNULL_OnList);
		kMethod *mtd = kNameSpace_GetGetterMethodNULL(kctx, ns, globalClass, symbol);
		if(mtd != NULL) {
			return new_GetterExpr(kctx, tk, mtd, new_ConstValueExpr(kctx, globalClass, ns->globalObjectNULL_OnList));
		}
	}
	kMethod *mtd = kNameSpace_GetNameSpaceFuncNULL(kctx, ns, symbol, reqClass);  // finding function
	if(mtd != NULL) {
		kParam *pa = kMethod_GetParam(mtd);
		KonohaClass *ct = KLIB KonohaClass_Generics(kctx, CT_Func, pa->rtype, pa->psize, (kparamtype_t *)pa->paramtypeItems);
		kFuncVar *fo = (kFuncVar *)KLIB new_kObject(kctx, OnGcStack, ct, (uintptr_t)mtd);
		KFieldSet(fo, fo->self, UPCAST(ns));
		return new_ConstValueExpr(kctx, ct, UPCAST(fo));
	}
	if(symbol != SYM_NONAME) {
		KKeyValue *kv = kNameSpace_GetConstNULL(kctx, ns, symbol);
		if(kv != NULL) {
			if(TypeAttr_Is(Boxed, kv->attrTypeId)) {
				SUGAR kExpr_SetConstValue(kctx, expr, NULL, kv->ObjectValue);
			}
			else {
				SUGAR kExpr_SetUnboxConstValue(kctx, expr, kv->attrTypeId, kv->unboxValue);
			}
			return expr;
		}
	}
	return NULL;
}

static KMETHOD TypeCheck_Symbol(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kExpr *texpr = kStmt_TypeCheckVariableNULL(kctx, stmt, expr, gma, CT_(reqty));
	if(texpr == NULL) {
		kToken *tk = expr->termToken;
		texpr = kStmtToken_Message(kctx, stmt, tk, ErrTag, "undefined name: %s", KToken_t(tk));
	}
	KReturn(texpr);
}

static KonohaClass* ResolveTypeVariable(KonohaContext *kctx, KonohaClass *varType, KonohaClass *thisClass)
{
	return varType->realtype(kctx, varType, thisClass);
}

static int TypeCheckPolicy_(kattrtype_t attrtype)
{
	int pol = 0;
	if(TypeAttr_Is(Coercion, attrtype)) {
		pol = pol | TypeCheckPolicy_COERCION;
	}
	return pol;
}

static kMethod *kStmt_LookupOverloadedMethod(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kMethod *mtd, kGamma *gma)
{
	KonohaClass *thisClass = CT_(expr->cons->ExprItems[1]->attrTypeId);
	size_t i, psize = kArray_size(expr->cons) - 2;
	kparamtype_t *p = ALLOCA(kparamtype_t, psize);
	kParam *pa = kMethod_GetParam(mtd);
	for(i = 0; i < psize; i++) {
		size_t n = i + 2;
		KonohaClass *paramType = (i < pa->psize) ? ResolveTypeVariable(kctx, CT_(pa->paramtypeItems[i].attrTypeId), thisClass) : CT_INFER;
		kExpr *texpr = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, n, gma, paramType, TypeCheckPolicy_NOCHECK);
		if(texpr == K_NULLEXPR) {
			return NULL;
		}
		p[i].attrTypeId = texpr->attrTypeId;
	}
	kparamId_t paramdom = KLIB Kparamdom(kctx, psize, p);
	kMethod *foundMethod = kNameSpace_GetMethodBySignatureNULL(kctx, Stmt_ns(stmt), thisClass, mtd->mn, paramdom, psize, p);
	DBG_P("paradom=%d, foundMethod=%p", paramdom, foundMethod);
	return foundMethod;
}

static kExprVar* TypeMethodCallExpr(KonohaContext *kctx, kExprVar *expr, kMethod *mtd, KonohaClass *reqClass)
{
	kExpr *thisExpr = kExpr_at(expr, 1);
	KFieldSet(expr->cons, expr->cons->MethodItems[0], mtd);
	KonohaClass *typedClass = ResolveTypeVariable(kctx, kMethod_GetReturnType(mtd), CT_(thisExpr->attrTypeId));
	if(thisExpr->build == TEXPR_NEW) {
		typedClass = CT_(thisExpr->attrTypeId);
	}
	else if(kMethod_Is(SmartReturn, mtd) && reqClass->typeId != TY_var) {
		typedClass = reqClass;
	}
	kExpr_typed(expr, CALL, typedClass->typeId);
	return expr;
}

static kExpr* BoxThisExpr(KonohaContext *kctx, kStmt *stmt, kGamma *gma, kExpr *expr, kMethod *mtd, KonohaClass **thisClassRef)
{
	kExpr *thisExpr = expr->cons->ExprItems[1];
	KonohaClass *thisClass = CT_(thisExpr->attrTypeId);
	DBG_ASSERT(IS_Method(mtd));
	DBG_ASSERT(thisClass->typeId != TY_var);
	if(!TY_isUnbox(mtd->typeId) && CT_IsUnbox(thisClass)) {
		KFieldSet(expr->cons, expr->cons->ExprItems[1], kStmtExpr_ToBox(kctx, stmt, thisExpr, gma, thisClass));
	}
	thisClassRef[0] = thisClass;
	return thisExpr;
}

static kExpr *kStmtkExpr_TypeCheckCallParam(KonohaContext *kctx, kStmt *stmt, kExprVar *expr, kMethod *mtd, kGamma *gma, KonohaClass* reqClass)
{
	KonohaClass *thisClass = NULL;
	kExpr *thisExpr = BoxThisExpr(kctx, stmt, gma, expr, mtd, &thisClass);
	kbool_t isConst = kExpr_IsConstValue(thisExpr);
	kParam *pa = kMethod_GetParam(mtd);
	DBG_ASSERT(pa->psize +2 <= kArray_size(expr->cons));
	size_t i;
	for(i = 0; i < pa->psize; i++) {
		size_t n = i + 2;
		KonohaClass* paramType = ResolveTypeVariable(kctx, CT_(pa->paramtypeItems[i].attrTypeId), thisClass);
		int tycheckPolicy = TypeCheckPolicy_(pa->paramtypeItems[i].attrTypeId);
		kExpr *texpr = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, n, gma, paramType, tycheckPolicy);
		if(texpr == K_NULLEXPR) {
			return kStmtExpr_Message(kctx, stmt, expr, InfoTag, "%s.%s%s accepts %s at the parameter %d", Method_t(mtd), CT_t(paramType), (int)i+1);
		}
		if(!kExpr_IsConstValue(texpr)) isConst = 0;
	}
	expr = TypeMethodCallExpr(kctx, expr, mtd, reqClass);
	if(isConst && kMethod_Is(Const, mtd)) {
		KonohaClass *rtype = ResolveTypeVariable(kctx, CT_(pa->rtype), thisClass);
		return kStmtExpr_ToConstValue(kctx, stmt, expr, expr->cons, rtype);
	}
	return expr;
}

static kExpr* TypeCheckDynamicCallParams(KonohaContext *kctx, kStmt *stmt, kExprVar *expr, kMethod *mtd, kGamma *gma, kString *name, kmethodn_t mn, KonohaClass *reqClass)
{
	size_t i;
	kParam *pa = kMethod_GetParam(mtd);
	KonohaClass* ptype = (pa->psize == 0) ? CT_Object : CT_(pa->paramtypeItems[0].attrTypeId);
	for(i = 2; i < kArray_size(expr->cons); i++) {
		kExpr *texpr = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, i, gma, ptype, 0);
		if(texpr == K_NULLEXPR) return texpr;
	}
	kExpr_Add(kctx, expr, new_ConstValueExpr(kctx, NULL, UPCAST(name)));
	return TypeMethodCallExpr(kctx, expr, mtd, reqClass);
}

static kMethod *kNameSpace_GuessCoercionMethodNULL(KonohaContext *kctx, kNameSpace *ns, kToken *tk, KonohaClass *thisClass)
{
	const char *name = S_text(tk->text);
	if(name[1] == 'o' && (name[0] == 't' || name[0] == 'T')) {
		KonohaClass *c = KLIB kNameSpace_GetClassByFullName(kctx, ns, name + 2, S_size(tk->text) - 2, NULL);
		if(c != NULL) {
			return KLIB kNameSpace_GetCoercionMethodNULL(kctx, ns, thisClass, c);
		}
	}
	return NULL;
}

static kExpr *kStmtExpr_LookupMethod(KonohaContext *kctx, kStmt *stmt, kExprVar *expr, KonohaClass *thisClass, kGamma *gma, KonohaClass *reqClass)
{
	kNameSpace *ns = Stmt_ns(stmt);
	kTokenVar *methodToken = expr->cons->TokenVarItems[0];
	DBG_ASSERT(IS_Token(methodToken));
	size_t psize = kArray_size(expr->cons) - 2;
	kMethod *mtd = kNameSpace_GetMethodByParamSizeNULL(kctx, ns, thisClass, methodToken->resolvedSymbol, psize, MethodMatch_CamelStyle);
	if(mtd == NULL && psize == 0) {
		mtd = kNameSpace_GuessCoercionMethodNULL(kctx, ns, methodToken, thisClass);
	}
	if(mtd == NULL) {
		if(methodToken->text != TS_EMPTY) {  // find Dynamic Call ..
			mtd = kNameSpace_GetMethodByParamSizeNULL(kctx, ns, thisClass, 0/*NONAME*/, 1, MethodMatch_NoOption);
			if(mtd != NULL) {
				return TypeCheckDynamicCallParams(kctx, stmt, expr, mtd, gma, methodToken->text, methodToken->resolvedSymbol, reqClass);
			}
		}
		if(methodToken->resolvedSymbol == MN_new && psize == 0 && CT_(kExpr_at(expr, 1)->attrTypeId)->baseTypeId == TY_Object) {
			return kExpr_at(expr, 1);  // new Person(); // default constructor
		}
		kStmtToken_Message(kctx, stmt, methodToken, ErrTag, "undefined method: %s.%s%s", CT_t(thisClass), PSYM_t(methodToken->resolvedSymbol));
	}
	if(mtd != NULL) {
		if(kMethod_Is(Overloaded, mtd)) {
			//DBG_P("found overloaded method %s.%s%s", Method_t(mtd));
			mtd = kStmt_LookupOverloadedMethod(kctx, stmt, expr, mtd, gma);
		}
		if(mtd != NULL) {
			//DBG_P("found resolved method %s.%s%s isOverloaded=%d", Method_t(mtd), kMethod_Is(Overloaded, mtd));
			return kStmtkExpr_TypeCheckCallParam(kctx, stmt, expr, mtd, gma, reqClass);
		}
	}
	return K_NULLEXPR;
}

static KMETHOD TypeCheck_MethodCall(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kExpr *texpr = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 1, gma, CT_INFER, 0);
	if(texpr != K_NULLEXPR) {
		KReturn(kStmtExpr_LookupMethod(kctx, stmt, expr, CT_(texpr->attrTypeId), gma, CT_(reqty)));
	}
}

// --------------------------------------------------------------------------
// FuncStyleCall

static kExpr *kExpr_TypeCheckFuncParams(KonohaContext *kctx, kStmt *stmt, kExprVar *expr, kGamma *gma);

static kMethod* kExpr_LookupFuncOrMethod(KonohaContext *kctx, kNameSpace *ns, kExprVar *exprN, kGamma *gma, KonohaClass *reqClass)
{
	kExprVar *firstExpr = (kExprVar *)kExpr_at(exprN, 0);
	kToken *termToken = firstExpr->termToken;
	ksymbol_t funcName = termToken->resolvedSymbol;
	GammaAllocaData *genv = gma->genv;
	int i;
	for(i = genv->localScope.varsize - 1; i >= 0; i--) {
		if(genv->localScope.varItems[i].name == funcName && TY_isFunc(genv->localScope.varItems[i].attrTypeId)) {
			SUGAR kExpr_SetVariable(kctx, firstExpr, gma, TEXPR_LOCAL, genv->localScope.varItems[i].attrTypeId, i);
			return NULL;
		}
	}
	int paramsize = kArray_size(exprN->cons) - 2;
	if(genv->localScope.varItems[0].attrTypeId != TY_void) {
		KonohaClass *ct = genv->thisClass;
		kMethod *mtd = kNameSpace_GetMethodByParamSizeNULL(kctx, ns, genv->thisClass, funcName, paramsize, MethodMatch_CamelStyle);
		if(mtd != NULL) {
			KFieldSet(exprN->cons, exprN->cons->ExprItems[1], new_VariableExpr(kctx, gma, TEXPR_LOCAL, ct->typeId, 0));
			return mtd;
		}
		if(ct->fieldsize) {
			for(i = ct->fieldsize; i >= 0; i--) {
				if(ct->fieldItems[i].name == funcName && TY_isFunc(ct->fieldItems[i].attrTypeId)) {
					SUGAR kExpr_SetVariable(kctx, firstExpr, gma, TEXPR_FIELD, ct->fieldItems[i].attrTypeId, longid((kshort_t)i, 0));
					return NULL;
				}
			}
		}
		mtd = kNameSpace_GetGetterMethodNULL(kctx, ns, genv->thisClass, funcName);
		if(mtd != NULL && kMethod_IsReturnFunc(mtd)) {
			KFieldSet(exprN->cons, exprN->cons->ExprItems[0], new_GetterExpr(kctx, termToken, mtd, new_VariableExpr(kctx, gma, TEXPR_LOCAL, genv->thisClass->typeId, 0)));
			return NULL;
		}
	}
	{
		KKeyValue* kvs = kNameSpace_GetConstNULL(kctx, ns, funcName);
		if(kvs != NULL && TypeAttr_Unmask(kvs->attrTypeId) == VirtualType_StaticMethod) {
			KonohaClass *c = CT_((kattrtype_t)kvs->unboxValue);
			ksymbol_t alias = (ksymbol_t)(kvs->unboxValue >> (sizeof(kattrtype_t) * 8));
			kMethod *mtd = kNameSpace_GetMethodByParamSizeNULL(kctx, ns, c, alias, paramsize, MethodMatch_NoOption);
			if(mtd != NULL && kMethod_Is(Static, mtd)) {
				KFieldSet(exprN->cons, exprN->cons->ExprItems[1], new_ConstValueExpr(kctx, c, KLIB Knull(kctx, c)));
				return mtd;
			}
		}
	}
	{
		kMethod *mtd = kNameSpace_GetMethodByParamSizeNULL(kctx, ns, O_ct(ns), funcName, paramsize, MethodMatch_CamelStyle);
		if(mtd != NULL) {
			KFieldSet(exprN->cons, exprN->cons->ExprItems[1], new_ConstValueExpr(kctx, O_ct(ns), UPCAST(ns)));
			return mtd;
		}
	}
	if((Gamma_isTopLevel(gma) || kNameSpace_IsAllowed(ImplicitGlobalVariable,ns)) && ns->globalObjectNULL_OnList != NULL) {
		KonohaClass *globalClass = O_ct(ns->globalObjectNULL_OnList);
		kMethod *mtd = kNameSpace_GetGetterMethodNULL(kctx, ns, globalClass, funcName);
		if(mtd != NULL && kMethod_IsReturnFunc(mtd)) {
			KFieldSet(exprN->cons, exprN->cons->ExprItems[0], new_GetterExpr(kctx, termToken, mtd, new_ConstValueExpr(kctx, globalClass, ns->globalObjectNULL_OnList)));
			return NULL;
		}
		return mtd;
	}
	return NULL;
}

static KMETHOD TypeCheck_FuncStyleCall(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	DBG_ASSERT(IS_Expr(kExpr_at(expr, 0)));
	DBG_ASSERT(expr->cons->ObjectItems[1] == K_NULL);
	if(kExpr_isSymbolTerm(kExpr_at(expr, 0))) {
		kMethod *mtd = kExpr_LookupFuncOrMethod(kctx, Stmt_ns(stmt), expr, gma, CT_(reqty));
		if(mtd != NULL) {
			if(kMethod_Is(Overloaded, mtd)) {
				DBG_P("overloaded found %s.%s%s", Method_t(mtd));
				mtd = kStmt_LookupOverloadedMethod(kctx, stmt, expr, mtd, gma);
			}
			KReturn(kStmtkExpr_TypeCheckCallParam(kctx, stmt, expr, mtd, gma, CT_(reqty)));
		}
		if(!TY_isFunc(kExpr_at(expr, 0)->attrTypeId)) {
			kToken *tk = kExpr_at(expr, 0)->termToken;
			DBG_ASSERT(IS_Token(tk));  // TODO: make error message in case of not Token
			KReturn(kStmtToken_Message(kctx, stmt, tk, ErrTag, "undefined function: %s", KToken_t(tk)));
		}
	}
	else {
		if(kStmt_TypeCheckExprAt(kctx, stmt, expr, 0, gma, CT_INFER, 0) != K_NULLEXPR) {
			if(!TY_isFunc(expr->cons->ExprItems[0]->attrTypeId)) {
				KReturn(kStmtExpr_Message(kctx, stmt, expr, ErrTag, "function is expected"));
			}
		}
	}
	KReturn(kExpr_TypeCheckFuncParams(kctx, stmt, expr, gma));
}

static kExpr *kExpr_TypeCheckFuncParams(KonohaContext *kctx, kStmt *stmt, kExprVar *expr, kGamma *gma)
{
	KonohaClass *thisClass = CT_(kExpr_at(expr, 0)->attrTypeId);
	kParam *pa = CT_cparam(thisClass);
	size_t i, size = kArray_size(expr->cons);
	if(pa->psize + 2 != size) {
		return kStmtExpr_Message(kctx, stmt, expr, ErrTag, "function %s takes %d parameter(s), but given %d parameter(s)", CT_t(thisClass), (int)pa->psize, (int)size-2);
	}
	for(i = 0; i < pa->psize; i++) {
		size_t n = i + 2;
		kExpr *texpr = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, n, gma, CT_(pa->paramtypeItems[i].attrTypeId), 0);
		if(texpr == K_NULLEXPR) {
			return texpr;
		}
	}
	kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, Stmt_ns(stmt), CT_Func, MN_("invoke"), -1, MethodMatch_NoOption);
	DBG_ASSERT(mtd != NULL);
	KFieldSet(expr->cons, expr->cons->ExprItems[1], expr->cons->ExprItems[0]);
	return TypeMethodCallExpr(kctx, expr, mtd, CT_(thisClass->p0));
}

// ---------------------------------------------------------------------------
// Statement Expr

static KMETHOD Statement_Expression(KonohaContext *kctx, KonohaStack *sfp)  // $Expr
{
	VAR_Statement(stmt, gma);
	kbool_t r = kStmt_TypeCheckByName(kctx, stmt, KW_ExprPattern, gma, CT_INFER, TypeCheckPolicy_ALLOWVOID);
	kStmt_typed(stmt, EXPR);
	KReturnUnboxValue(r);
}

#define DefaultNameSpace NULL
static KMETHOD Statement_if(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	if(kStmt_TypeCheckByName(kctx, stmt, KW_ExprPattern, gma, CT_Boolean, 0)) {
		kBlock *bkThen = SUGAR kStmt_GetBlock(kctx, stmt, DefaultNameSpace, KW_BlockPattern, K_NULLBLOCK);
		kBlock *bkElse = SUGAR kStmt_GetBlock(kctx, stmt, DefaultNameSpace, KW_else, K_NULLBLOCK);
		kBlock_TypeCheckAll(kctx, bkThen, gma);
		kBlock_TypeCheckAll(kctx, bkElse, gma);
		kStmt_typed(stmt, IF);
	}
}

static kStmt* Stmt_LookupIfStmtWithoutElse(KonohaContext *kctx, kStmt *stmt)
{
	kBlock *bkElse = SUGAR kStmt_GetBlock(kctx, stmt, DefaultNameSpace, KW_else, NULL);
	if(bkElse != NULL) {
		if(kArray_size(bkElse->StmtList) == 1) {
			kStmt *stmtIf = bkElse->StmtList->StmtItems[0];
			if(stmtIf->syn->keyword == KW_if) {
				return Stmt_LookupIfStmtWithoutElse(kctx, stmtIf);
			}
		}
		return NULL;
	}
	return stmt;
}

static kStmt* Stmt_LookupIfStmtNULL(KonohaContext *kctx, kStmt *stmt)
{
	int i;
	kArray *bka = stmt->parentBlockNULL->StmtList;
	kStmt *prevIfStmt = NULL;
	for(i = 0; kArray_size(bka); i++) {
		kStmt *s = bka->StmtItems[i];
		if(s == stmt) {
			if(prevIfStmt != NULL) {
				return Stmt_LookupIfStmtWithoutElse(kctx, prevIfStmt);
			}
			return NULL;
		}
		if(s->syn == NULL) continue;  // this is done
		prevIfStmt = (s->syn->keyword == KW_if) ? s : NULL;
	}
	return NULL;
}

static KMETHOD Statement_else(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kStmt *stmtIf = Stmt_LookupIfStmtNULL(kctx, stmt);
	if(stmtIf != NULL) {
		kBlock *bkElse = SUGAR kStmt_GetBlock(kctx, stmt, NULL/*DefaultNameSpace*/, KW_BlockPattern, K_NULLBLOCK);
		KLIB kObjectProto_SetObject(kctx, stmtIf, KW_else, TY_Block, bkElse);
		kStmt_done(kctx, stmt);
		kBlock_TypeCheckAll(kctx, bkElse, gma);
	}
	else {
		kStmt_Message(kctx, stmt, ErrTag, "else is not statement");
//		r = 0;
	}
//	KReturnUnboxValue(r);
}

static KMETHOD Statement_return(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	KonohaClass *returnType = kMethod_GetReturnType(gma->genv->currentWorkingMethod);
	kStmt_typed(stmt, RETURN);
	if(returnType->typeId != TY_void) {
		kStmt_TypeCheckByName(kctx, stmt, KW_ExprPattern, gma, returnType, 0);
	} else {
		kExpr *expr = (kExpr *)kStmt_GetObjectNULL(kctx, stmt, KW_ExprPattern);
		if(expr != NULL) {
			kStmt_Message(kctx, stmt, WarnTag, "ignored return value");
			KLIB kObjectProto_RemoveKey(kctx, stmt, KW_ExprPattern);
		}
	}
}

///* ------------------------------------------------------------------------ */

static kStmt* TypeDeclLocalVariable(KonohaContext *kctx, kStmt *stmt, kGamma *gma, kattrtype_t attrTypeId, kExpr *termExpr, kExpr *vexpr, kObject *thunk)
{
	DBG_ASSERT(kExpr_isSymbolTerm(termExpr));
	int index = kGamma_AddLocalVariable(kctx, gma, attrTypeId, termExpr->termToken->resolvedSymbol);
	SUGAR kExpr_SetVariable(kctx, (kExprVar *)termExpr, gma, TEXPR_LOCAL, attrTypeId, index);
	kExprVar *expr = (kExprVar *)new_TypedConsExpr(kctx, TEXPR_LET, CT_(TY_void), 3, K_NULL, termExpr, vexpr);
	kExpr_typed(expr, LET, TY_void);
	kStmt *newstmt = new_(Stmt, stmt->uline, OnGcStack);
	kStmt_setsyn(newstmt, SYN_(Stmt_ns(stmt), KW_ExprPattern));
	KLIB kObjectProto_SetObject(kctx, newstmt, KW_ExprPattern, TY_Expr, expr);
	return newstmt;
}

static kbool_t kStmt_DeclType(KonohaContext *kctx, kStmt *stmt, kGamma *gma, kattrtype_t attrTypeId, kExpr *declExpr, kObject *thunk, TypeDeclFunc TypeDecl, kStmt **lastStmtRef)
{
	kStmt *newstmt = NULL;
	if(TypeDecl == NULL) TypeDecl = TypeDeclLocalVariable;
	if(declExpr->syn->keyword == KW_COMMA) {
		size_t i;
		for(i = 1; i < kArray_size(declExpr->cons); i++) {
			if(!kStmt_DeclType(kctx, stmt, gma, attrTypeId, kExpr_at(declExpr, i), thunk, TypeDecl, lastStmtRef)) return false;
		}
		return true;
	}
	else if(declExpr->syn->keyword == KW_LET && kExpr_isSymbolTerm(kExpr_at(declExpr, 1))) {
		if(SUGAR kStmt_TypeCheckExprAt(kctx, stmt, declExpr, 2, gma, CT_(attrTypeId), 0) == K_NULLEXPR) {
			// this is neccesarry to avoid 'int a = a + 1;';
			return false;
		}
		if(TypeAttr_Unmask(attrTypeId) == TY_var) {
			kattrtype_t attr = TypeAttr_Attr(attrTypeId);
			kToken *termToken = kExpr_at(declExpr, 1)->termToken;
			attrTypeId = kExpr_at(declExpr, 2)->attrTypeId | attr;
			kStmtToken_Message(kctx, stmt, termToken, InfoTag, "%s%s has type %s", PSYM_t(termToken->resolvedSymbol), TY_t(attrTypeId));
		}
		newstmt = TypeDecl(kctx, stmt, gma, attrTypeId, kExpr_at(declExpr, 1), kExpr_at(declExpr, 2), thunk);
	}
	else if(kExpr_isSymbolTerm(declExpr)) {
		if(attrTypeId == TY_var  || !TY_is(Nullable, attrTypeId)) {
			kStmt_Message(kctx, stmt, ErrTag, "%s %s%s: initial value is expected", TY_t(attrTypeId), PSYM_t(declExpr->termToken->resolvedSymbol));
			return false;
		}
		else {
			kExpr *vexpr = new_VariableExpr(kctx, gma, TEXPR_NULL, attrTypeId, 0);
			newstmt = TypeDecl(kctx, stmt, gma, attrTypeId, declExpr, vexpr, thunk);
		}
	}
	else {
		kStmt_Message(kctx, stmt, ErrTag, "type declaration: variable name is expected");
		return false;
	}
	if(newstmt != NULL) {
		kStmt *lastStmt = lastStmtRef[0];
		kBlock_InsertAfter(kctx, lastStmt->parentBlockNULL, lastStmt, newstmt);
		lastStmtRef[0] = newstmt;
		kStmt_done(kctx, stmt);
		return true;
	}
	return false;
}

static KMETHOD Statement_TypeDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kToken *tk  = SUGAR kStmt_GetToken(kctx, stmt, KW_TypePattern, NULL);
	kExpr  *expr = SUGAR kStmt_GetExpr(kctx, stmt, KW_ExprPattern, NULL);
	kattrtype_t attrTypeId = Token_typeLiteral(tk);
	kStmt_DeclType(kctx, stmt, gma, attrTypeId, expr, NULL, TypeDeclLocalVariable, &stmt);
}

// ------------------
// Method Utilities for MethodDecl

static KMETHOD MethodFunc_LazyCompilation(KonohaContext *kctx, KonohaStack *sfp)
{
	KonohaStack *esp = kctx->esp;
	kMethod *mtd = sfp[K_MTDIDX].calledMethod;
	kString *text = mtd->SourceToken->text;
	kfileline_t uline = mtd->SourceToken->uline;
	DBG_P("<<lazy compilation>>: %s.%s%s", TY_t(mtd->typeId), MethodName_t(mtd->mn));
	kMethod_Compile(kctx, mtd, NULL, mtd->LazyCompileNameSpace, text, uline, DefaultCompileOption/*HatedLazyCompile*/);
	((KonohaContextVar *)kctx)->esp = esp;
	mtd->invokeMethodFunc(kctx, sfp); // call again;
}

static void kMethod_SetLazyCompilation(KonohaContext *kctx, kMethodVar *mtd, kStmt *stmt, kNameSpace *ns)
{
	kToken *tcode = SUGAR kStmt_GetToken(kctx, stmt, KW_BlockPattern, NULL);
	if(tcode != NULL && tcode->resolvedSyntaxInfo->keyword == TokenType_CODE) {
		KFieldSet(mtd, mtd->SourceToken, tcode);
		KFieldSet(mtd, mtd->LazyCompileNameSpace, ns);
		KLIB kMethod_SetFunc(kctx, mtd, MethodFunc_LazyCompilation);
		KLIB kArray_Add(kctx, GetSugarContext(kctx)->definedMethodList, mtd);
	}
}

/* In the future, DoLazyCompilation is extended to compile untyped parameters */

static kMethod* kMethod_DoLazyCompilation(KonohaContext *kctx, kMethod *mtd, kparamtype_t *callparamNULL, int options)
{
	if(mtd->invokeMethodFunc == MethodFunc_LazyCompilation) {
		kString *text = mtd->SourceToken->text;
		kfileline_t uline = mtd->SourceToken->uline;
		((kMethodVar *)mtd)->invokeMethodFunc = NULL; // TO avoid recursive compile
		mtd = kMethod_Compile(kctx, mtd, callparamNULL, mtd->LazyCompileNameSpace, text, uline, options|HatedLazyCompile);
		DBG_ASSERT(mtd->invokeMethodFunc != MethodFunc_LazyCompilation);
	}
	return mtd;
}

///* ------------------------------------------------------------------------ */
///* [ParamUtils] */

static kbool_t StmtTypeDecl_setParam(KonohaContext *kctx, kStmt *stmt, int n, kparamtype_t *p)
{
	kToken *tkT  = SUGAR kStmt_GetToken(kctx, stmt, KW_TypePattern, NULL);
	kExpr  *expr = SUGAR kStmt_GetExpr(kctx, stmt, KW_ExprPattern, NULL);
	DBG_ASSERT(tkT != NULL);
	DBG_ASSERT(expr != NULL);
	if(kExpr_isSymbolTerm(expr)) {
		kToken *tkN = expr->termToken;
		ksymbol_t fn = ksymbolA(S_text(tkN->text), S_size(tkN->text), SYM_NEWID);
		p[n].name = fn;
		p[n].attrTypeId = Token_typeLiteral(tkT);
		return true;
	}
	return false;
}

static kParam *kStmt_newMethodParamNULL(KonohaContext *kctx, kStmt *stmt, kGamma* gma)
{
	kParam *pa = (kParam *)kStmt_GetObjectNULL(kctx, stmt, KW_ParamPattern);
	if(pa == NULL || !IS_Param(pa)) {
		SugarSyntax *syn = SYN_(Stmt_ns(stmt), KW_ParamPattern);
		if(!SugarSyntax_TypeCheckStmt(kctx, syn, stmt, gma)) {
			return NULL;
		}
	}
	pa = (kParam *)kStmt_GetObjectNULL(kctx, stmt, KW_ParamPattern);
	DBG_ASSERT(IS_Param(pa));
	return pa;
}

static KMETHOD Statement_ParamsDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kToken *tkT = SUGAR kStmt_GetToken(kctx, stmt, KW_TypePattern, NULL); // type
	kattrtype_t rtype =  tkT == NULL ? TY_void : Token_typeLiteral(tkT);
	kParam *pa = NULL;
	kBlock *params = (kBlock *)kStmt_GetObjectNULL(kctx, stmt, KW_ParamPattern);
	if(params == NULL) {
		pa = new_kParam(kctx, rtype, 0, NULL);
	}
	else if(IS_Block(params)) {
		size_t i, psize = kArray_size(params->StmtList);
		kparamtype_t *p = ALLOCA(kparamtype_t, psize);
		for(i = 0; i < psize; i++) {
			p[i].attrTypeId = TY_void;
			p[i].name = 0;
			kStmt *stmt = params->StmtList->StmtItems[i];
			if(stmt->syn->keyword != KW_TypeDeclPattern || !StmtTypeDecl_setParam(kctx, stmt, i, p)) {
				break;
			}
		}
		pa = new_kParam(kctx, rtype, psize, p);
	}
	if(pa != NULL && IS_Param(pa)) {
		KLIB kObjectProto_SetObject(kctx, stmt, KW_ParamPattern, TY_Param, pa);
		kStmt_done(kctx, stmt);
		KReturnUnboxValue(true);
	}
	kStmt_Message(kctx, stmt, ErrTag, "expected parameter declaration");
	KReturnUnboxValue(false);
}

///* ------------------------------------------------------------------------ */
///* [MethodDecl] */

static kattrtype_t kStmt_GetClassId(KonohaContext *kctx, kStmt *stmt, kNameSpace *ns, ksymbol_t kw, kattrtype_t defcid)
{
	kToken *tk = (kToken *)kStmt_GetObjectNULL(kctx, stmt, kw);
	if(tk == NULL || !IS_Token(tk)) {
		return defcid;
	}
	else {
		DBG_ASSERT(Token_isVirtualTypeLiteral(tk));
		return Token_typeLiteral(tk);
	}
}

static ksymbol_t kStmt_GetMethodSymbol(KonohaContext *kctx, kStmt *stmt, kNameSpace *ns, ksymbol_t kw, kmethodn_t defmn)
{
	kToken *tk = (kToken *)kStmt_GetObjectNULL(kctx, stmt, kw);
	if(tk == NULL || !IS_Token(tk) || !IS_String(tk->text)) {
		return defmn;
	}
	else {
		return tk->resolvedSymbol;
	}
}

static KMETHOD Statement_MethodDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	static KonohaFlagSymbolData MethodDeclFlag[] = {
		{kMethod_Public}, {kMethod_Const}, {kMethod_Static},
		{kMethod_Virtual}, {kMethod_Final}, {kMethod_Override},
		{kMethod_Restricted},
	};
	if(MethodDeclFlag[0].symbol == 0) {   // this is a tricky technique
		MethodDeclFlag[0].symbol = SYM_("@Public");
		MethodDeclFlag[1].symbol = SYM_("@Const");
		MethodDeclFlag[2].symbol = SYM_("@Static");
		MethodDeclFlag[3].symbol = SYM_("@Virtual");
		MethodDeclFlag[4].symbol = SYM_("@Final");
		MethodDeclFlag[5].symbol = SYM_("@Override");
		MethodDeclFlag[6].symbol = SYM_("@Restricted");
	}
	uintptr_t flag    = kStmt_ParseFlag(kctx, stmt, MethodDeclFlag, 0);
	kNameSpace *ns    = Stmt_ns(stmt);
	kattrtype_t typeId    = kStmt_GetClassId(kctx, stmt, ns, SYM_("ClassName"), O_typeId(ns));
	kmethodn_t mn     = kStmt_GetMethodSymbol(kctx, stmt, ns, KW_SymbolPattern, MN_new);
	kParam *pa        = kStmt_newMethodParamNULL(kctx, stmt, gma);
	if(TY_is(Singleton, typeId)) { flag |= kMethod_Static; }
	if(TY_is(Final, typeId)) { flag |= kMethod_Final; }
	if(pa != NULL) {  // if pa is NULL, error is printed out.
		INIT_GCSTACK();
		kMethodVar *mtd = (kMethodVar *)KLIB new_kMethod(kctx, _GcStack, flag, typeId, mn, NULL);
		KLIB kMethod_SetParam(kctx, mtd, pa->rtype, pa->psize, (kparamtype_t *)pa->paramtypeItems);
		KMakeTrace(trace, sfp);
		if(kNameSpace_AddMethod(kctx, ns, mtd, trace)) {
			kMethod_SetLazyCompilation(kctx, mtd, stmt, ns);
		}
		RESET_GCSTACK();
	}
	kStmt_done(kctx, stmt);
	KReturnUnboxValue(pa != NULL);
}

/* ------------------------------------------------------------------------ */

#define PATTERN(T)  KW_##T##Pattern
#define GROUP(T)    KW_##T##Group
#define TOKEN(T)    KW_##T

static void DefineDefaultSyntax(KonohaContext *kctx, kNameSpace *ns)
{
	DBG_ASSERT(SYM_("$Param") == KW_ParamPattern);
	DBG_ASSERT(SYM_(".") == KW_DOT);
	DBG_ASSERT(SYM_(":") == KW_COLON);
	DBG_ASSERT(SYM_("true") == KW_true);
	DBG_ASSERT(SYM_("return") == KW_return);
	DBG_P("MN_new=%d", SYM_("new"));
	DBG_ASSERT(SYM_("new") == MN_new);
	KDEFINE_SYNTAX SYNTAX[] = {
		{ TOKEN(ERR), SYNFLAG_StmtBreakExec, },
		{ PATTERN(Symbol),  0, NULL, 0, 0, PatternMatch_MethodName, Expression_Term, NULL, NULL, TypeCheck_Symbol,},
		{ PATTERN(Text),    0, NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_TextLiteral,},
		{ PATTERN(Number),  0, NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_IntLiteral,},
		{ GROUP(Parenthesis), SYNFLAG_ExprPostfixOp2, NULL, Precedence_CStyleCALL, 0, NULL, Expression_Parenthesis, NULL, NULL, TypeCheck_FuncStyleCall,}, //KW_ParenthesisGroup
		{ GROUP(Bracket),  },  //KW_BracketGroup
		{ GROUP(Brace),  }, // KW_BraceGroup
		{ PATTERN(Block), 0, NULL, 0, 0, PatternMatch_CStyleBlock, NULL, NULL, NULL, TypeCheck_Block, },
		{ PATTERN(Param), 0, NULL, 0, 0, PatternMatch_CStyleParam, Expression_OperatorMethod, Statement_ParamsDecl, NULL, TypeCheck_MethodCall,},
		{ PATTERN(Token), 0, NULL, 0, 0, PatternMatch_Token/*PatternMatch_Toks*/, },
		{ TOKEN(DOT), 0, NULL, Precedence_CStyleCALL, 0, NULL, Expression_DOT, },
		{ TOKEN(DIV), 0, NULL, Precedence_CStyleMUL, },
		{ TOKEN(MOD), 0, NULL, Precedence_CStyleMUL, },
		{ TOKEN(MUL), 0, NULL, Precedence_CStyleMUL, },
		{ TOKEN(ADD), 0, NULL, Precedence_CStyleADD, },
		{ TOKEN(SUB), 0, NULL, Precedence_CStyleADD, Precedence_CStylePREUNARY, },
		{ TOKEN(LT),  0, NULL, Precedence_CStyleCOMPARE, },
		{ TOKEN(LTE), 0, NULL, Precedence_CStyleCOMPARE, },
		{ TOKEN(GT),  0, NULL, Precedence_CStyleCOMPARE, },
		{ TOKEN(GTE), 0, NULL, Precedence_CStyleCOMPARE, },
		{ TOKEN(EQ),  0, NULL, Precedence_CStyleEQUALS, },
		{ TOKEN(NEQ), 0, NULL, Precedence_CStyleEQUALS, },
		{ TOKEN(LET), SYNFLAG_ExprLeftJoinOp2, NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_OperatorMethod, NULL, NULL, TypeCheck_Assign, },
		{ TOKEN(AND), 0, NULL, Precedence_CStyleAND, 0, NULL, Expression_OperatorMethod, NULL, NULL, TypeCheck_AndOperator, },
		{ TOKEN(OR),  0, NULL, Precedence_CStyleOR,  0, NULL, Expression_OperatorMethod, NULL, NULL, TypeCheck_OrOperator, },
		{ TOKEN(NOT), 0, NULL, 0, Precedence_CStylePREUNARY, },
		{ TOKEN(COLON), 0, NULL, Precedence_CStyleTRINARY, },  // colon
		{ TOKEN(COMMA),   0, NULL, Precedence_CStyleCOMMA, 0, NULL, Expression_COMMA, },
		{ TOKEN(DOLLAR),  0, NULL, 0, 0, NULL, Expression_DOLLAR, },
		{ TOKEN(true),    0, NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_true, },
		{ TOKEN(false),   0, NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_false, },
		{ PATTERN(Expr),  0, "$Expr", 0, 0, PatternMatch_Expression, Expression_ParsedExpr, Statement_Expression, Statement_Expression, NULL, },
		{ PATTERN(Type),  0, NULL, 0, 0, PatternMatch_Type, NULL, NULL, NULL/*Statement_TypeDecl*/, TypeCheck_Type, },
		{ PATTERN(TypeDecl),   0, "$TypeDecl $Type $Expr", 0, 0, PatternMatch_TypeDecl, NULL, NULL, Statement_TypeDecl, },
		{ PATTERN(MethodDecl), 0, "$MethodDecl $Type [ClassName: $Type \".\"] $Symbol $Param [$Block]", 0, 0, PatternMatch_MethodDecl, NULL, Statement_MethodDecl, NULL, NULL, },
		{ TOKEN(if),     0, "\"if\" \"(\" $Expr \")\" $Block [\"else\" else: $Block]", 0, 0, NULL, NULL, NULL, Statement_if, NULL, },
		{ TOKEN(else),   0,  "\"else\" $Block", 0, 0, NULL, NULL, /*Statement_else*/NULL, Statement_else, NULL, },
		{ TOKEN(return), SYNFLAG_StmtBreakExec, "\"return\" [$Expr]", 0, 0, NULL, NULL, NULL, Statement_return, NULL, },
		{ SYM_("new"), 0, NULL, 0, Precedence_CStyleCALL, NULL, Expression_new, NULL, NULL, NULL, },
		{ KW_END, },
	};
	kNameSpace_DefineSyntax(kctx, ns, SYNTAX, NULL);
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif