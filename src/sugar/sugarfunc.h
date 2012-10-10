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

static void kStmt_addParsedObject(KonohaContext *kctx, kStmt *stmt, ksymbol_t keyid, kObject *o)
{
	kArray* valueList = (kArray*)KLIB kObject_getObject(kctx, stmt, keyid, NULL);
	if(valueList == NULL) {
		KLIB kObject_setObject(kctx, stmt, keyid, O_typeId(o), o);
	}
	else {
		//DBG_P(">>> keyid=%s%s valueList=%s, value=%s", PSYM_t(keyid), CT_t(O_ct(valueList)), CT_t(O_ct(o)));
		if(!IS_Array(valueList)) {
			kArray *newList = GCSAFE_new(Array, 0);
			KLIB kArray_add(kctx, newList, valueList);
			KLIB kObject_setObject(kctx, stmt, keyid, O_typeId(newList), newList);
			valueList = newList;
		}
		KLIB kArray_add(kctx, valueList, o);
	}
}

static int TokenUtils_findEndOfStatement(KonohaContext *kctx, kArray *tokenList, int beginIdx, int endIdx, int isNoSemiColon)
{
	int c;
	for(c = beginIdx; c < endIdx; c++) {
		kToken *tk = tokenList->tokenItems[c];
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
	INIT_GCSTACK();
	int returnIdx = -1;
	endIdx = TokenUtils_findEndOfStatement(kctx, tokenList, beginIdx, endIdx, kNameSpace_isAllowed(NoSemiColon, Stmt_nameSpace(stmt)));
	kExpr *expr = kStmt_parseExpr(kctx, stmt, tokenList, beginIdx, endIdx, NULL);
	if(expr != K_NULLEXPR) {
		KdumpExpr(kctx, expr);
		kStmt_addParsedObject(kctx, stmt, name, UPCAST(expr));
		returnIdx = endIdx;
	}
	RESET_GCSTACK();
	RETURNi_(returnIdx);
}

static KMETHOD PatternMatch_Type(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	KonohaClass *foundClass = NULL;
	int returnIdx = TokenUtils_parseTypePattern(kctx, Stmt_nameSpace(stmt), tokenList, beginIdx, endIdx, &foundClass);
	DBG_P("tk=%s, returnIdx=%d", tokenList->tokenItems[beginIdx], returnIdx);
	if(foundClass != NULL) {
		kTokenVar *tk = GCSAFE_new(TokenVar, 0);
		kStmt_addParsedObject(kctx, stmt, name, UPCAST(tk));
		kToken_setTypeId(kctx, tk, Stmt_nameSpace(stmt), foundClass->typeId);
	}
	RETURNi_(returnIdx);
}

static KMETHOD PatternMatch_MethodName(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	kTokenVar *tk = tokenList->tokenVarItems[beginIdx];
	int returnIdx = -1;
	if(tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern || tk->resolvedSyntaxInfo->precedence_op1 > 0 || tk->resolvedSyntaxInfo->precedence_op2 > 0) {
		kStmt_addParsedObject(kctx, stmt, name, UPCAST(tk));
		returnIdx = beginIdx + 1;
	}
	RETURNi_(returnIdx);
}

static void TokenSequence_checkCStyleParam(KonohaContext *kctx, TokenSequence* tokens)
{
	int i;
	for(i = 0; i < tokens->endIdx; i++) {
		kTokenVar *tk = tokens->tokenList->tokenVarItems[i];
		if(tk->resolvedSyntaxInfo->keyword == KW_void) {
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
	kToken *tk = tokenList->tokenItems[beginIdx];
	if(tk->resolvedSyntaxInfo->keyword == KW_ParenthesisGroup) {
		TokenSequence param = {Stmt_nameSpace(stmt), tk->subTokenList, 0, kArray_size(tk->subTokenList)};
		TokenSequence_checkCStyleParam(kctx, &param);
		kBlock *bk = new_kBlock(kctx, stmt, NULL, &param);
		kStmt_addParsedObject(kctx, stmt, name, UPCAST(bk));
		returnIdx = beginIdx + 1;
	}
	RETURNi_(returnIdx);
}

static KMETHOD PatternMatch_CStyleBlock(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	kToken *tk = tokenList->tokenItems[beginIdx];
//	KdumpTokenArray(kctx, tokenList, beginIdx, endIdx);
	if(tk->resolvedSyntaxInfo->keyword == TokenType_CODE || tk->resolvedSyntaxInfo->keyword == KW_BraceGroup) {
		kStmt_addParsedObject(kctx, stmt, name, UPCAST(tk));
		RETURNi_(beginIdx+1);
	}
	int newEndIdx = TokenUtils_findEndOfStatement(kctx, tokenList, beginIdx, endIdx, true);
	TokenSequence tokens = {Stmt_nameSpace(stmt), tokenList, beginIdx, newEndIdx};
	kBlock *bk = new_kBlock(kctx, stmt, NULL, &tokens);
	kStmt_addParsedObject(kctx, stmt, name, UPCAST(bk));
	RETURNi_(newEndIdx);
}

static KMETHOD PatternMatch_Token(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	DBG_ASSERT(beginIdx < endIdx);
	kToken *tk = tokenList->tokenItems[beginIdx];
	if(!kToken_is(StatementSeparator, tk)) {
		kStmt_addParsedObject(kctx, stmt, name, UPCAST(tk));
		RETURNi_(beginIdx+1);
	}
	RETURNi_(-1);
}

static KMETHOD PatternMatch_TypeDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	KonohaClass *foundClass = NULL;
	int nextIdx = TokenUtils_parseTypePattern(kctx, Stmt_nameSpace(stmt), tokenList, beginIdx, endIdx, &foundClass);
	//DBG_P("@ nextIdx = %d < %d", nextIdx, endIdx);
	if(nextIdx != -1) {
		nextIdx = TokenUtils_skipIndent(tokenList, nextIdx, endIdx);
		if(nextIdx < endIdx) {
			kToken *tk = tokenList->tokenItems[nextIdx];
			if(tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern) {
				RETURNi_(beginIdx);
			}
		}
	}
	RETURNi_(-1);
}

static KMETHOD PatternMatch_MethodDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	kNameSpace *ns = Stmt_nameSpace(stmt);
	KonohaClass *foundClass = NULL;
	int nextIdx = TokenUtils_parseTypePattern(kctx, ns, tokenList, beginIdx, endIdx, &foundClass);
	//DBG_P("@ nextIdx = %d < %d", nextIdx, endIdx);
	if(nextIdx != -1) {
		nextIdx = TokenUtils_skipIndent(tokenList, nextIdx, endIdx);
		if(nextIdx < endIdx) {
			kToken *tk = tokenList->tokenItems[nextIdx];
			if(TokenUtils_parseTypePattern(kctx, ns, tokenList, nextIdx, endIdx, NULL) != -1) {
				RETURNi_(beginIdx);
			}
			if(tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern) {
				int symbolNextIdx = TokenUtils_skipIndent(tokenList, nextIdx + 1, endIdx);
				if(symbolNextIdx < endIdx && tokenList->tokenItems[symbolNextIdx]->resolvedSyntaxInfo->keyword == KW_ParenthesisGroup) {
					RETURNi_(beginIdx);
				}
				RETURNi_(-1);
			}
			if(tk->resolvedSyntaxInfo->keyword != KW_DOT && ((tk->resolvedSyntaxInfo->precedence_op1 > 0 || tk->resolvedSyntaxInfo->precedence_op2 > 0))) {
				RETURNi_(beginIdx);
			}
		}
	}
	RETURNi_(-1);
}

/* ------------------------------------------------------------------------ */

static KMETHOD Expression_ParsedExpr(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	if(beginIdx == operatorIdx) {
		kToken *tk = tokenList->tokenItems[operatorIdx];
		DBG_ASSERT(IS_Expr(tk->parsedExpr));
		RETURN_(tk->parsedExpr);
	}
}

static KMETHOD Expression_Term(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	if(beginIdx == operatorIdx) {
		kToken *tk = tokenList->tokenItems[operatorIdx];
		KonohaClass *foundClass = NULL;
		int nextIdx = TokenUtils_parseTypePattern(kctx, Stmt_nameSpace(stmt), tokenList, beginIdx, endIdx, &foundClass);
		if(foundClass != NULL) {
			kToken_setTypeId(kctx, tk, Stmt_nameSpace(stmt), foundClass->typeId);
		}
		else {
			nextIdx = operatorIdx + 1;
		}
		RETURN_(kStmt_rightJoinExpr(kctx, stmt, new_UntypedTermExpr(kctx, tk), tokenList, nextIdx, endIdx));
	}
}

static KMETHOD Expression_OperatorMethod(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	if(/*syn->keyword != KW_LET && */syn->sugarFuncTable[SugarFunc_TypeCheck] == NULL) {
		DBG_P("switching type checker of %s%s to MethodCall ..", PSYM_t(syn->keyword));
		syn = SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall);  // switch type checker
	}
	kTokenVar *tk = tokenList->tokenVarItems[operatorIdx];
	kExpr *expr, *rexpr = kStmt_parseExpr(kctx, stmt, tokenList, operatorIdx + 1, endIdx, Token_text(tk));
	PUSH_GCSTACK(rexpr);
	if(beginIdx == operatorIdx) { // unary operator
		expr = new_UntypedCallStyleExpr(kctx, syn, 2, tk, rexpr);
	}
	else {   // binary operator
		kExpr *lexpr = kStmt_parseExpr(kctx, stmt, tokenList, beginIdx, operatorIdx, NULL);
		expr = new_UntypedCallStyleExpr(kctx, syn, 3, tk, lexpr, rexpr);
	}
	RETURN_(expr);
}

static inline kbool_t isFieldName(kArray *tokenList, int operatorIdx, int endIdx)
{
	if(operatorIdx + 1 < endIdx) {
		kToken *tk = tokenList->tokenItems[operatorIdx + 1];
		return (tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern);
	}
	return false;
}

static KMETHOD Expression_DOT(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	if(beginIdx < operatorIdx && isFieldName(tokenList, operatorIdx, endIdx)) {
		kExpr *expr = kStmt_parseExpr(kctx, stmt, tokenList, beginIdx, operatorIdx, NULL);
		expr = new_UntypedCallStyleExpr(kctx, syn, 2, tokenList->tokenItems[operatorIdx +1], expr);
		RETURN_(kStmt_rightJoinExpr(kctx, stmt, expr, tokenList, operatorIdx +2, endIdx));
	}
}

static KMETHOD Expression_Parenthesis(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	kToken *tk = tokenList->tokenItems[operatorIdx];
	if(beginIdx == operatorIdx) {
		kExpr *expr = kStmt_parseExpr(kctx, stmt, tk->subTokenList, 0, kArray_size(tk->subTokenList), "(");
		RETURN_(kStmt_rightJoinExpr(kctx, stmt, expr, tokenList, operatorIdx + 1, endIdx));
	}
	else {
		kExpr *lexpr = kStmt_parseExpr(kctx, stmt, tokenList, beginIdx, operatorIdx, NULL);
		if(lexpr == K_NULLEXPR) {
			RETURN_(lexpr);
		}
		if(lexpr->syn->keyword == KW_DOT) {
			((kExprVar*)lexpr)->syn = SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall); // CALL
		}
		else if(lexpr->syn->keyword != KW_ExprMethodCall) {
			syn = SYN_(Stmt_nameSpace(stmt), KW_ParenthesisGroup);    // (f null ())
			lexpr  = new_UntypedCallStyleExpr(kctx, syn, 2, lexpr, K_NULL);
		}
		if(kArray_size(tk->subTokenList) > 0) {
			lexpr = kStmt_addExprParam(kctx, stmt, lexpr, tk->subTokenList, 0, kArray_size(tk->subTokenList), "(");
		}
		RETURN_(kStmt_rightJoinExpr(kctx, stmt, lexpr, tokenList, operatorIdx + 1, endIdx));
	}
}

static KMETHOD Expression_COMMA(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	kExpr *expr = new_UntypedCallStyleExpr(kctx, syn, 1, tokenList->tokenItems[operatorIdx]);
	expr = kStmt_addExprParam(kctx, stmt, expr, tokenList, beginIdx, endIdx, 0/*allowEmpty*/);
	RETURN_(expr);
}

static KMETHOD Expression_DOLLAR(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	if(beginIdx == operatorIdx && operatorIdx +1 < endIdx) {
		kToken *tk = tokenList->tokenItems[operatorIdx +1];
		if(tk->resolvedSyntaxInfo->keyword == TokenType_CODE) {
			kToken_transformToBraceGroup(kctx, (kTokenVar*)tk, Stmt_nameSpace(stmt), NULL);
		}
		if(tk->resolvedSyntaxInfo->keyword == KW_BraceGroup) {
			kExprVar *expr = GCSAFE_new(ExprVar, SYN_(Stmt_nameSpace(stmt), KW_BlockPattern));
			TokenSequence range = {Stmt_nameSpace(stmt), tk->subTokenList, 0, kArray_size(tk->subTokenList)};
			KSETv(expr, expr->block, SUGAR new_kBlock(kctx, stmt, NULL, &range));
			RETURN_(expr);
		}
	}
}

/* ------------------------------------------------------------------------ */
/* Expression TyCheck */

static kString *kToken_resolvedEscapeSequence(KonohaContext *kctx, kToken *tk, size_t start)
{
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	const char *text = S_text(tk->text) + start;
	const char *end  = S_text(tk->text) + S_size(tk->text);
	KLIB Kwb_write(kctx, &wb, S_text(tk->text), start);
	while (text < end) {
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
			KLIB Kwb_write(kctx, &wb, (const char*)buf, 1);
		}
		text++;
	}
	kString *s = KLIB new_kString(kctx, KLIB Kwb_top(kctx, &wb, 1), Kwb_bytesize(&wb), 0);
	PUSH_GCSTACK(s);
	KLIB Kwb_free(&wb);
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
		text = kToken_resolvedEscapeSequence(kctx, tk, escape - S_text(text));
		if(text == NULL) {
			RETURN_(ERROR_UndefinedEscapeSequence(kctx, stmt, tk));
		}
	}
	RETURN_(SUGAR kExpr_setConstValue(kctx, expr, TY_String, UPCAST(text)));
}

static KMETHOD TypeCheck_Type(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	DBG_ASSERT(Token_isVirtualTypeLiteral(expr->termToken));
	RETURN_(SUGAR kExpr_setVariable(kctx, expr, gma, TEXPR_NULL, expr->termToken->resolvedTypeId, 0));
}

static KMETHOD TypeCheck_true(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	RETURN_(SUGAR kExpr_setUnboxConstValue(kctx, expr, TY_boolean, (uintptr_t)1));
}

static KMETHOD TypeCheck_false(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	RETURN_(SUGAR kExpr_setUnboxConstValue(kctx, expr, TY_boolean, (uintptr_t)0));
}

static KMETHOD TypeCheck_IntLiteral(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->termToken;
	long long n = strtoll(S_text(tk->text), NULL, 0);
	RETURN_(SUGAR kExpr_setUnboxConstValue(kctx, expr, TY_int, (uintptr_t)n));
}

static KMETHOD TypeCheck_AndOperator(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	if(SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 1, gma, TY_boolean, 0) != K_NULLEXPR) {
		if(SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 2, gma, TY_boolean, 0) != K_NULLEXPR) {
			RETURN_(kExpr_typed(expr, AND, TY_boolean));
		}
	}
}

static KMETHOD TypeCheck_OrOperator(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	if(SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 1, gma, TY_boolean, 0) != K_NULLEXPR) {
		if(SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 2, gma, TY_boolean, 0) != K_NULLEXPR) {
			RETURN_(kExpr_typed(expr, OR, TY_boolean));
		}
	}
}

static KMETHOD TypeCheck_Assign(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kNameSpace *ns = Stmt_nameSpace(stmt);  // leftHandExpr = rightHandExpr
	kExpr *leftHandExpr = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 1, gma, TY_var, TPOL_ALLOWVOID);
	kExpr *rightHandExpr = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 2, gma, leftHandExpr->ty, 0);
	if(rightHandExpr != K_NULLEXPR && leftHandExpr != K_NULLEXPR) {
		if(leftHandExpr->build == TEXPR_LOCAL || leftHandExpr->build == TEXPR_FIELD || leftHandExpr->build == TEXPR_STACKTOP) {
			((kExprVar*)expr)->build = TEXPR_LET;
			((kExprVar*)expr)->ty    = leftHandExpr->ty;
			((kExprVar*)rightHandExpr)->ty = leftHandExpr->ty;
			RETURN_(expr);
		}
		if(leftHandExpr->build == TEXPR_CALL) {  // check getter and transform to setter
			kMethod *mtd = leftHandExpr->cons->methodItems[0];
			DBG_ASSERT(IS_Method(mtd));
			if(MN_isGETTER(mtd->mn)) {
				ktype_t cid = leftHandExpr->cons->exprItems[1]->ty;
				ktype_t paramType = leftHandExpr->ty;
				ksymbol_t sym = SYM_UNMASK(mtd->mn);
				kMethod *foundMethod = KLIB kNameSpace_getSetterMethodNULL(kctx, ns, cid, sym, paramType);  // FIXME
				if(foundMethod != NULL) {
					KSETv(leftHandExpr->cons, leftHandExpr->cons->methodItems[0], foundMethod);
					KLIB kArray_add(kctx, leftHandExpr->cons, rightHandExpr);
					RETURN_(SUGAR kStmt_tyCheckCallParamExpr(kctx, stmt, leftHandExpr, foundMethod, gma, reqty));
				}
				kParam *pa = Method_param(mtd);
				if (pa->psize == 1) { /* transform "T1 A.get(T2)" to "void A.set(T2, T1)" */
					kparamtype_t p[2] = {{pa->paramtypeItems[0].ty}, {pa->rtype}};
					kparamid_t paramdom = KLIB Kparamdom(kctx, 2, p);
					foundMethod = kNameSpace_getMethodBySignatureNULL(kctx, ns, cid, MN_toSETTER(sym), paramdom, 2, p);
					if(foundMethod != NULL) {
						KSETv(leftHandExpr->cons, leftHandExpr->cons->methodItems[0], foundMethod);
						KLIB kArray_add(kctx, leftHandExpr->cons, rightHandExpr);
						RETURN_(SUGAR kStmt_tyCheckCallParamExpr(kctx, stmt, leftHandExpr, foundMethod, gma, reqty));
					}
				}
			}
		}
		SUGAR kStmt_printMessage2(kctx, stmt, (kToken*)expr, ErrTag, "variable name is expected");
	}
	RETURN_(K_NULLEXPR);
}

static int kGamma_declareLocalVariable(KonohaContext *kctx, kGamma *gma, ktype_t ty, ksymbol_t fn)
{
	GammaStack *s = &gma->genv->localScope;
	int index = s->varsize;
	if(!(s->varsize < s->capacity)) {
		s->capacity *= 2;
		size_t asize = sizeof(GammaStackDecl) * s->capacity;
		GammaStackDecl *v = (GammaStackDecl*)KMALLOC(asize);
		memcpy(v, s->varItems, asize/2);
		if(s->allocsize > 0) {
			KFREE(s->varItems, s->allocsize);
		}
		s->varItems = v;
		s->allocsize = asize;
	}
	s->varItems[index].ty = ty;
	s->varItems[index].fn = fn;
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
		if(kArray_size(bk->stmtList) > 0) {
			kStmt *stmt = bk->stmtList->stmtItems[kArray_size(bk->stmtList)-1];
			if(stmt->syn->keyword == KW_ExprPattern) {
				lastExpr = stmt;
			}
			uline = stmt->uline;
		}
		if(lastExpr != NULL) {
			int lvarsize = gma->genv->localScope.varsize;
			int popBlockScopeShiftSize = gma->genv->blockScopeShiftSize;
			gma->genv->blockScopeShiftSize = lvarsize;
			if(!kBlock_tyCheckAll(kctx, bk, gma)) {
				RETURN_(texpr);
			}
			kExpr *lvar = new_VariableExpr(kctx, gma, TEXPR_LOCAL, TY_var, kGamma_declareLocalVariable(kctx, gma, TY_var, 0/*FN_*/));
			kExpr *rexpr = SUGAR kStmt_getExpr(kctx, lastExpr, KW_ExprPattern, NULL);
			DBG_ASSERT(rexpr != NULL);
			ktype_t ty = rexpr->ty;
			if(ty != TY_void) {
				kExpr *letexpr = new_TypedConsExpr(kctx, TEXPR_LET, TY_void, 3, K_NULL, lvar, rexpr);
				KLIB kObject_setObject(kctx, lastExpr, KW_ExprPattern, TY_Expr, letexpr);
				texpr = SUGAR kExpr_setVariable(kctx, expr, gma, TEXPR_BLOCK, ty, lvarsize);
			}
			gma->genv->blockScopeShiftSize = popBlockScopeShiftSize;
			if(lvarsize < gma->genv->localScope.varsize) {
				gma->genv->localScope.varsize = lvarsize;
			}
		}
		if(texpr == K_NULLEXPR) {
			((kStmtVar*)stmt)->uline = uline;
			kStmt_printMessage(kctx, stmt, ErrTag, "block has no value");
		}
		RETURN_(texpr);
	}
	kStmtExpr_printMessage(kctx, stmt, expr, ErrTag, "undefined expression: %s", Token_text(expr->termToken));
}

static kExpr* new_GetterExpr(KonohaContext *kctx, kToken *tkU, kMethod *mtd, kExpr *expr)
{
	kExprVar *expr1 = (kExprVar *)new_TypedConsExpr(kctx, TEXPR_CALL, Method_returnType(mtd), 2, mtd, expr);
	//KSETv(expr1->tk, tkU); // for uline
	return (kExpr*)expr1;
}

static kExpr* kStmt_tyCheckVariableNULL(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kGamma *gma, ktype_t reqty)
{
	DBG_ASSERT(expr->ty == TY_var);
	kToken *tk = expr->termToken;
	ksymbol_t symbol = tk->resolvedSymbol;
	kNameSpace *ns = Stmt_nameSpace(stmt);
	int i;
	GammaAllocaData *genv = gma->genv;
	for(i = genv->localScope.varsize - 1; i >= 0; i--) {
		if(genv->localScope.varItems[i].fn == symbol) {
			return SUGAR kExpr_setVariable(kctx, expr, gma, TEXPR_LOCAL, genv->localScope.varItems[i].ty, i);
		}
	}
	if(kNameSpace_isAllowed(ImplicitField, ns)) {
		if(genv->localScope.varItems[0].ty != TY_void) {
			DBG_ASSERT(genv->this_cid == genv->localScope.varItems[0].ty);
			KonohaClass *ct = CT_(genv->this_cid);
			if (ct->fieldsize > 0) {
				for(i = ct->fieldsize; i >= 0; i--) {
					if(ct->fieldItems[i].fn == symbol && ct->fieldItems[i].ty != TY_void) {
						return SUGAR kExpr_setVariable(kctx, expr, gma, TEXPR_FIELD, ct->fieldItems[i].ty, longid((kshort_t)i, 0));
					}
				}
			}
			kMethod *mtd = kNameSpace_getGetterMethodNULL(kctx, ns, genv->this_cid, symbol, TY_var);
			if(mtd != NULL) {
				return new_GetterExpr(kctx, tk, mtd, new_VariableExpr(kctx, gma, TEXPR_LOCAL, genv->this_cid, 0));
			}
		}
	}
	if((Gamma_isTopLevel(gma) || kNameSpace_isAllowed(TransparentGlobalVariable, ns)) && ns->globalObjectNULL != NULL) {
		ktype_t cid = O_typeId(ns->globalObjectNULL);
		kMethod *mtd = kNameSpace_getGetterMethodNULL(kctx, ns, cid, symbol, TY_var);
		if(mtd != NULL) {
			return new_GetterExpr(kctx, tk, mtd, new_ConstValueExpr(kctx, cid, ns->globalObjectNULL));
		}
	}
	kMethod *mtd = kNameSpace_getNameSpaceFuncNULL(kctx, ns, symbol, reqty);  // finding function
	if(mtd != NULL) {
		kParam *pa = Method_param(mtd);
		KonohaClass *ct = KLIB KonohaClass_Generics(kctx, CT_Func, pa->rtype, pa->psize, (kparamtype_t*)pa->paramtypeItems);
		kFuncVar *fo = (kFuncVar*)KLIB new_kObjectOnGCSTACK(kctx, ct, (uintptr_t)mtd);
		KSETv(fo, fo->self, UPCAST(ns));
		return new_ConstValueExpr(kctx, ct->typeId, UPCAST(fo));
	}
	if(symbol != SYM_NONAME) {
		KUtilsKeyValue *kv = kNameSpace_getConstNULL(kctx, ns, symbol);
		if(kv != NULL) {
			if(SYMKEY_isBOXED(kv->key)) {
				SUGAR kExpr_setConstValue(kctx, expr, kv->ty, kv->objectValue);
			}
			else {
				SUGAR kExpr_setUnboxConstValue(kctx, expr, kv->ty, kv->unboxValue);
			}
			return expr;
		}
	}
	return NULL;
}

static KMETHOD TypeCheck_Symbol(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kExpr *texpr = kStmt_tyCheckVariableNULL(kctx, stmt, expr, gma, reqty);
	if(texpr == NULL) {
		kToken *tk = expr->termToken;
		texpr = kStmtToken_printMessage(kctx, stmt, tk, ErrTag, "undefined name: %s", Token_text(tk));
	}
	RETURN_(texpr);
}

static ktype_t ktype_var(KonohaContext *kctx, ktype_t ty, KonohaClass *this_ct)
{
	KonohaClass *ct = CT_(ty);
	ct = ct->realtype(kctx, ct, this_ct);
	return ct->typeId;
}

static int param_policy(ksymbol_t fn)
{
	int pol = 0;
	if(FN_isCOERCION(fn)) {
		pol = pol | TPOL_COERCION;
	}
	return pol;
}

static kMethod *kStmt_lookupOverloadedMethod(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kMethod *mtd, kGamma *gma)
{
	KonohaClass *thisClass = CT_(expr->cons->exprItems[1]->ty);
	size_t i, psize = kArray_size(expr->cons) - 2;
	kparamtype_t *p = ALLOCA(kparamtype_t, psize);
	kParam *pa = Method_param(mtd);
	for(i = 0; i < psize; i++) {
		size_t n = i + 2;
		ktype_t paramType = (i < pa->psize) ? ktype_var(kctx, pa->paramtypeItems[i].ty, thisClass) : TY_var;
		kExpr *texpr = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, n, gma, paramType, TPOL_NOCHECK);
		if(texpr == K_NULLEXPR) {
			return NULL;
		}
		p[i].ty = expr->cons->exprItems[i+2]->ty;
	}
	kparamid_t paramdom = KLIB Kparamdom(kctx, psize, p);
	kMethod *foundMethod = kNameSpace_getMethodBySignatureNULL(kctx, Stmt_nameSpace(stmt), thisClass->typeId, mtd->mn, paramdom, psize, p);
	DBG_P("paradom=%d, foundMethod=%p", paramdom, foundMethod);
	return foundMethod;
}

static kExpr* kExpr_typedWithMethod(KonohaContext *kctx, kExpr *expr, kMethod *mtd, ktype_t reqty)
{
	kExpr *thisExpr = kExpr_at(expr, 1);
	KSETv(expr->cons, expr->cons->methodItems[0], mtd);
	if(thisExpr->build == TEXPR_NEW) {
		kExpr_typed(expr, CALL, thisExpr->ty);
	}
	else {
		kExpr_typed(expr, CALL, kMethod_is(SmartReturn, mtd) ? reqty : ktype_var(kctx, Method_returnType(mtd), CT_(thisExpr->ty)));
	}
	return expr;
}

static kExpr* boxThisExpr(KonohaContext *kctx, kStmt *stmt, kGamma *gma, kExpr *expr, kMethod *mtd)
{
	kExpr *thisExpr = expr->cons->exprItems[1];
	KonohaClass *thisClass = CT_(thisExpr->ty);
	DBG_ASSERT(IS_Method(mtd));
	DBG_ASSERT(thisClass->typeId != TY_var);
	if(!TY_isUnbox(mtd->typeId) && CT_isUnbox(thisClass)) {
		ktype_t unboxType = thisClass->typeId == TY_boolean ? TY_boolean : TY_int;
		kMethod *boxMethod = kNameSpace_getMethodByParamSizeNULL(kctx, Stmt_nameSpace(stmt), unboxType, MN_box, 0);
		thisExpr = new_TypedCallExpr(kctx, stmt, gma, thisClass->typeId, boxMethod, 1, expr->cons->exprItems[1]);
		KSETv(expr->cons, expr->cons->exprItems[1], thisExpr);
	}
	return thisExpr;
}

static kExpr *kStmt_tyCheckCallParamExpr(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kMethod *mtd, kGamma *gma, ktype_t reqty)
{
	kExpr *thisExpr = boxThisExpr(kctx, stmt, gma, expr, mtd);
	KonohaClass *thisClass = CT_(thisExpr->ty);
	int isConst = (Expr_isCONST(thisExpr)) ? 1 : 0;
	kParam *pa = Method_param(mtd);
	size_t i;
	DBG_ASSERT(pa->psize +2 == kArray_size(expr->cons));
	for(i = 0; i < pa->psize; i++) {
		size_t n = i + 2;
		ktype_t paramType = ktype_var(kctx, pa->paramtypeItems[i].ty, thisClass);
		int tycheckPolicy = param_policy(pa->paramtypeItems[i].fn);
		kExpr *texpr = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, n, gma, paramType, tycheckPolicy);
		if(texpr == K_NULLEXPR) {
			return kStmtExpr_printMessage(kctx, stmt, expr, InfoTag, "%s.%s%s accepts %s at the parameter %d", Method_t(mtd), TY_t(paramType), (int)i+1);
		}
		if(!Expr_isCONST(texpr)) isConst = 0;
	}
	expr = kExpr_typedWithMethod(kctx, expr, mtd, reqty);
	if(isConst && kMethod_is(Const, mtd)) {
		ktype_t rtype = ktype_var(kctx, pa->rtype, thisClass);
		return kExprCall_toConstValue(kctx, expr, expr->cons, rtype);
	}
	return expr;
}

static kExpr* tyCheckDynamicCallParams(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kMethod *mtd, kGamma *gma, kString *name, kmethodn_t mn, ktype_t reqty)
{
	int i;
	kParam *pa = Method_param(mtd);
	ktype_t ptype = (pa->psize == 0) ? TY_Object : pa->paramtypeItems[0].ty;
	for(i = 2; i < kArray_size(expr->cons); i++) {
		kExpr *texpr = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, i, gma, ptype, 0);
		if(texpr == K_NULLEXPR) return texpr;
	}
	Expr_add(kctx, expr, new_ConstValueExpr(kctx, TY_String, UPCAST(name)));
	return kExpr_typedWithMethod(kctx, expr, mtd, reqty);
}

static const char* MethodType_t(KonohaContext *kctx, kmethodn_t mn, size_t psize)
{
	return "method";
}

static kExpr *kStmtExpr_lookupMethod(KonohaContext *kctx, kStmt *stmt, kExpr *expr, ktype_t this_cid, kGamma *gma, ktype_t reqty)
{
	kNameSpace *ns = Stmt_nameSpace(stmt);
	kTokenVar *tkMN = expr->cons->tokenVarItems[0];
	DBG_ASSERT(IS_Token(tkMN));
	size_t psize = kArray_size(expr->cons) - 2;
	kMethod *mtd = kNameSpace_getMethodByParamSizeNULL(kctx, ns, this_cid, tkMN->resolvedSymbol, psize);
	if(mtd == NULL) {
		if(tkMN->text != TS_EMPTY) {  // find Dynamic Call ..
			mtd = KLIB kNameSpace_getMethodByParamSizeNULL(kctx, ns, this_cid, 0/*NONAME*/, -1);
			if(mtd != NULL) {
				return tyCheckDynamicCallParams(kctx, stmt, expr, mtd, gma, tkMN->text, tkMN->resolvedSymbol, reqty);
			}
		}
		if(tkMN->resolvedSymbol == MN_new && psize == 0 && CT_(kExpr_at(expr, 1)->ty)->baseTypeId == TY_Object) {
			return kExpr_at(expr, 1);  // new Person(); // default constructor
		}
		kStmtToken_printMessage(kctx, stmt, tkMN, ErrTag, "undefined %s: %s.%s%s", MethodType_t(kctx, tkMN->resolvedSymbol, psize), TY_t(this_cid), PSYM_t(tkMN->resolvedSymbol));
	}
	if(mtd != NULL) {
		if(kMethod_is(Overloaded, mtd)) {
			DBG_P("found overloaded method %s.%s%s", Method_t(mtd));
			mtd = kStmt_lookupOverloadedMethod(kctx, stmt, expr, mtd, gma);
		}
		if (mtd != NULL) {
			DBG_P("found resolved method %s.%s%s isOverloaded=%d", Method_t(mtd), kMethod_is(Overloaded, mtd));
			return kStmt_tyCheckCallParamExpr(kctx, stmt, expr, mtd, gma, reqty);
		}
	}
	return K_NULLEXPR;
}

static KMETHOD TypeCheck_MethodCall(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kExpr *texpr = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 1, gma, TY_var, 0);
	if(texpr != K_NULLEXPR) {
		ktype_t this_cid = texpr->ty;
		RETURN_(kStmtExpr_lookupMethod(kctx, stmt, expr, this_cid, gma, reqty));
	}
}

// --------------------------------------------------------------------------
// FuncStyleCall

static kExpr *Expr_tyCheckFuncParams(KonohaContext *kctx, kStmt *stmt, kExpr *expr, KonohaClass *ct, kGamma *gma);

static kMethod* Expr_lookUpFuncOrMethod(KonohaContext *kctx, kNameSpace *ns, kExpr *exprN, kGamma *gma, ktype_t reqty)
{
	kExpr *firstExpr = kExpr_at(exprN, 0);
	kToken *termToken = firstExpr->termToken;
	ksymbol_t fn = ksymbolA(S_text(termToken->text), S_size(termToken->text), SYM_NONAME);
	GammaAllocaData *genv = gma->genv;
	int i;
	for(i = genv->localScope.varsize - 1; i >= 0; i--) {
		if(genv->localScope.varItems[i].fn == fn && TY_isFunc(genv->localScope.varItems[i].ty)) {
			SUGAR kExpr_setVariable(kctx, firstExpr, gma, TEXPR_LOCAL, genv->localScope.varItems[i].ty, i);
			return NULL;
		}
	}
	if(genv->localScope.varItems[0].ty != TY_void) {
		DBG_ASSERT(genv->this_cid == genv->localScope.varItems[0].ty);
		kMethod *mtd = KLIB kNameSpace_getMethodByParamSizeNULL(kctx, ns, genv->this_cid, fn, kArray_size(exprN->cons) - 2);
		if(mtd != NULL) {
			KSETv(exprN->cons, exprN->cons->exprItems[1], new_VariableExpr(kctx, gma, TEXPR_LOCAL, gma->genv->this_cid, 0));
			return mtd;
		}
		KonohaClass *ct = CT_(genv->this_cid);
		if (ct->fieldsize) {
			for(i = ct->fieldsize; i >= 0; i--) {
				if(ct->fieldItems[i].fn == fn && TY_isFunc(ct->fieldItems[i].ty)) {
					SUGAR kExpr_setVariable(kctx, firstExpr, gma, TEXPR_FIELD, ct->fieldItems[i].ty, longid((kshort_t)i, 0));
					return NULL;
				}
			}
		}
		mtd = kNameSpace_getGetterMethodNULL(kctx, ns, genv->this_cid, fn, TY_var);
		if(mtd != NULL && TY_isFunc(Method_returnType(mtd))) {
			KSETv(exprN->cons, exprN->cons->exprItems[0], new_GetterExpr(kctx, termToken, mtd, new_VariableExpr(kctx, gma, TEXPR_LOCAL, genv->this_cid, 0)));
			return NULL;
		}
	}
	{
		int paramsize = kArray_size(exprN->cons) - 2;
		kMethod *mtd = kNameSpace_getMethodByParamSizeNULL(kctx, ns, O_typeId(ns), fn, paramsize);
		if(mtd != NULL) {
			KSETv(exprN->cons, exprN->cons->exprItems[1], new_ConstValueExpr(kctx, O_typeId(ns), UPCAST(ns)));
			return mtd;
		}
	}
	if((Gamma_isTopLevel(gma) || kNameSpace_isAllowed(TransparentGlobalVariable,ns)) && ns->globalObjectNULL != NULL) {
		ktype_t cid = O_typeId(ns->globalObjectNULL);
		kMethod *mtd = kNameSpace_getGetterMethodNULL(kctx, ns, cid, fn, TY_var);
		if(mtd != NULL && TY_isFunc(Method_returnType(mtd))) {
			KSETv(exprN->cons, exprN->cons->exprItems[0], new_GetterExpr(kctx, termToken, mtd, new_ConstValueExpr(kctx, cid, ns->globalObjectNULL)));
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
	DBG_ASSERT(expr->cons->objectItems[1] == K_NULL);
	if(Expr_isSymbolTerm(kExpr_at(expr, 0))) {
		kMethod *mtd = Expr_lookUpFuncOrMethod(kctx, Stmt_nameSpace(stmt), expr, gma, reqty);
		if(mtd != NULL) {
			if(kMethod_is(Overloaded, mtd)) {
				DBG_P("overloaded found %s.%s%s", Method_t(mtd));
				mtd = kStmt_lookupOverloadedMethod(kctx, stmt, expr, mtd, gma);
			}
			RETURN_(kStmt_tyCheckCallParamExpr(kctx, stmt, expr, mtd, gma, reqty));
		}
		if(!TY_isFunc(kExpr_at(expr, 0)->ty)) {
			kToken *tk = kExpr_at(expr, 0)->termToken;
			DBG_ASSERT(IS_Token(tk));  // TODO: make error message in case of not Token
			RETURN_(kStmtToken_printMessage(kctx, stmt, tk, ErrTag, "undefined function: %s", Token_text(tk)));
		}
	}
	else {
		if(kStmt_tyCheckExprAt(kctx, stmt, expr, 0, gma, TY_var, 0) != K_NULLEXPR) {
			if(!TY_isFunc(expr->cons->exprItems[0]->ty)) {
				RETURN_(kStmtExpr_printMessage(kctx, stmt, expr, ErrTag, "function is expected"));
			}
		}
	}
	RETURN_(Expr_tyCheckFuncParams(kctx, stmt, expr, CT_(kExpr_at(expr, 0)->ty), gma));
}

static kExpr *Expr_tyCheckFuncParams(KonohaContext *kctx, kStmt *stmt, kExpr *expr, KonohaClass *ct, kGamma *gma)
{
	ktype_t rtype = ct->p0;
	kParam *pa = CT_cparam(ct);
	size_t i, size = kArray_size(expr->cons);
	if(pa->psize + 2 != size) {
		return kStmtExpr_printMessage(kctx, stmt, expr, ErrTag, "function %s takes %d parameter(s), but given %d parameter(s)", CT_t(ct), (int)pa->psize, (int)size-2);
	}
	for(i = 0; i < pa->psize; i++) {
		size_t n = i + 2;
		kExpr *texpr = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, n, gma, pa->paramtypeItems[i].ty, 0);
		if(texpr == K_NULLEXPR) {
			return texpr;
		}
	}
	kMethod *mtd = KLIB kNameSpace_getMethodByParamSizeNULL(kctx, Stmt_nameSpace(stmt), TY_Func, MN_("invoke"), -1);
	DBG_ASSERT(mtd != NULL);
	KSETv(expr->cons, expr->cons->exprItems[1], expr->cons->exprItems[0]);
	return kExpr_typedWithMethod(kctx, expr, mtd, rtype);
}

// ---------------------------------------------------------------------------
// Statement Expr

static KMETHOD Statement_Expression(KonohaContext *kctx, KonohaStack *sfp)  // $Expr
{
	VAR_Statement(stmt, gma);
	kbool_t r = kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_var, TPOL_ALLOWVOID);
	kStmt_typed(stmt, EXPR);
	RETURNb_(r);
}

#define DefaultNameSpace NULL
static KMETHOD Statement_if(KonohaContext *kctx, KonohaStack *sfp)
{
	kbool_t r = 1;
	VAR_Statement(stmt, gma);
	if((r = kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_boolean, 0))) {
		kBlock *bkThen = SUGAR kStmt_getBlock(kctx, stmt, DefaultNameSpace, KW_BlockPattern, K_NULLBLOCK);
		kBlock *bkElse = SUGAR kStmt_getBlock(kctx, stmt, DefaultNameSpace, KW_else, K_NULLBLOCK);
		r = kBlock_tyCheckAll(kctx, bkThen, gma);
		r = r & kBlock_tyCheckAll(kctx, bkElse, gma);
		kStmt_typed(stmt, IF);
	}
	RETURNb_(r);
}

static kStmt* Stmt_lookupIfStmtWithoutElse(KonohaContext *kctx, kStmt *stmt)
{
	kBlock *bkElse = SUGAR kStmt_getBlock(kctx, stmt, DefaultNameSpace, KW_else, NULL);
	if(bkElse != NULL) {
		if(kArray_size(bkElse->stmtList) == 1) {
			kStmt *stmtIf = bkElse->stmtList->stmtItems[0];
			if(stmtIf->syn->keyword == KW_if) {
				return Stmt_lookupIfStmtWithoutElse(kctx, stmtIf);
			}
		}
		return NULL;
	}
	return stmt;
}

static kStmt* Stmt_lookupIfStmtNULL(KonohaContext *kctx, kStmt *stmt)
{
	int i;
	kArray *bka = stmt->parentBlockNULL->stmtList;
	kStmt *prevIfStmt = NULL;
	for(i = 0; kArray_size(bka); i++) {
		kStmt *s = bka->stmtItems[i];
		if(s == stmt) {
			if(prevIfStmt != NULL) {
				return Stmt_lookupIfStmtWithoutElse(kctx, prevIfStmt);
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
	kbool_t r = 1;
	VAR_Statement(stmt, gma);
	kStmt *stmtIf = Stmt_lookupIfStmtNULL(kctx, stmt);
	if(stmtIf != NULL) {
		kBlock *bkElse = SUGAR kStmt_getBlock(kctx, stmt, NULL/*DefaultNameSpace*/, KW_BlockPattern, K_NULLBLOCK);
		KLIB kObject_setObject(kctx, stmtIf, KW_else, TY_Block, bkElse);
		kStmt_done(kctx, stmt);
		r = kBlock_tyCheckAll(kctx, bkElse, gma);
	}
	else {
		kStmt_printMessage(kctx, stmt, ErrTag, "else is not statement");
		r = 0;
	}
	RETURNb_(r);
}

static KMETHOD Statement_return(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kbool_t r = 1;
	ktype_t rtype = Method_returnType(gma->genv->currentWorkingMethod);
	kStmt_typed(stmt, RETURN);
	if(rtype != TY_void) {
		r = kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, rtype, 0);
	} else {
		kExpr *expr = (kExpr*)kStmt_getObjectNULL(kctx, stmt, KW_ExprPattern);
		if (expr != NULL) {
			kStmt_printMessage(kctx, stmt, WarnTag, "ignored return value");
			r = kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_var, 0);
			KLIB kObject_removeKey(kctx, stmt, 1);
		}
	}
	RETURNb_(r);
}

///* ------------------------------------------------------------------------ */

static kStmt* TypeDeclLocalVariable(KonohaContext *kctx, kStmt *stmt, kGamma *gma, ktype_t ty, kExpr *termExpr, kExpr *vexpr, kObject *thunk)
{
	DBG_ASSERT(Expr_isSymbolTerm(termExpr));
	kToken *tk = termExpr->termToken;
	int index = kGamma_declareLocalVariable(kctx, gma, ty, tk->resolvedSymbol);
	SUGAR kExpr_setVariable(kctx, termExpr, gma, TEXPR_LOCAL, ty, index);
	termExpr = new_TypedConsExpr(kctx, TEXPR_LET, TY_void, 3, K_NULL, termExpr, vexpr);
	kStmt *newstmt = GCSAFE_new(Stmt, stmt->uline);
	kStmt_setsyn(newstmt, SYN_(Stmt_nameSpace(stmt), KW_ExprPattern));
	kExpr_typed(termExpr, LET, TY_void);
	KLIB kObject_setObject(kctx, newstmt, KW_ExprPattern, TY_Expr, termExpr);
	return newstmt;
}

static kbool_t kStmt_declType(KonohaContext *kctx, kStmt *stmt, kGamma *gma, ktype_t ty, kExpr *declExpr, kObject *thunk, TypeDeclFunc TypeDecl, kStmt **lastStmtRef)
{
	kStmt *newstmt = NULL;
	if(TypeDecl == NULL) {
		TypeDecl = TypeDeclLocalVariable;
	}
	if(declExpr->syn->keyword == KW_COMMA) {
		size_t i;
		for(i = 1; i < kArray_size(declExpr->cons); i++) {
			if(!kStmt_declType(kctx, stmt, gma, ty, kExpr_at(declExpr, i), thunk, TypeDecl, lastStmtRef)) return false;
		}
		return true;
	}
	else if(declExpr->syn->keyword == KW_LET && Expr_isSymbolTerm(kExpr_at(declExpr, 1))) {
		if(SUGAR kStmt_tyCheckExprAt(kctx, stmt, declExpr, 2, gma, ty, 0) == K_NULLEXPR) {
			// this is neccesarry to avoid 'int a = a + 1;';
			return false;
		}
		if(ty == TY_var) {
			kToken *termToken = kExpr_at(declExpr, 1)->termToken;
			ktype_t inferedType = kExpr_at(declExpr, 2)->ty;
			kStmtToken_printMessage(kctx, stmt, termToken, InfoTag, "%s%s has type %s", PSYM_t(termToken->resolvedSymbol), TY_t(inferedType));
			ty = inferedType;
		}
		newstmt = TypeDecl(kctx, stmt, gma, ty, kExpr_at(declExpr, 1), kExpr_at(declExpr, 2), thunk);
	}
	else if(Expr_isSymbolTerm(declExpr)) {
		if(ty == TY_var  || !TY_is(Nullable, ty)) {
			kStmt_printMessage(kctx, stmt, ErrTag, "%s %s%s: initial value is expected", TY_t(ty), PSYM_t(declExpr->termToken->resolvedSymbol));
			return false;
		}
		else {
			kExpr *vexpr = new_VariableExpr(kctx, gma, TEXPR_NULL, ty, 0);
			newstmt = TypeDecl(kctx, stmt, gma, ty, declExpr, vexpr, thunk);
		}
	}
	else {
		kStmt_printMessage(kctx, stmt, ErrTag, "type declaration: variable name is expected");
		return false;
	}
	if(newstmt != NULL) {
		kStmt *lastStmt = lastStmtRef[0];
		kBlock_insertAfter(kctx, lastStmt->parentBlockNULL, lastStmt, newstmt);
		lastStmtRef[0] = newstmt;
		kStmt_done(kctx, stmt);
		return true;
	}
	return false;
}

static KMETHOD Statement_TypeDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kToken *tk  = SUGAR kStmt_getToken(kctx, stmt, KW_TypePattern, NULL);
	kExpr  *expr = SUGAR kStmt_getExpr(kctx, stmt, KW_ExprPattern, NULL);
	RETURNb_(kStmt_declType(kctx, stmt, gma, Token_typeLiteral(tk), expr, NULL, TypeDeclLocalVariable, &stmt));
}

// ------------------
// Method Utilities for MethodDecl

static KMETHOD MethodFunc_lazyCompilation(KonohaContext *kctx, KonohaStack *sfp)
{
	KonohaStack *esp = kctx->esp;
	kMethod *mtd = sfp[K_MTDIDX].mtdNC;
	kString *text = mtd->sourceCodeToken->text;
	kfileline_t uline = mtd->sourceCodeToken->uline;
	kNameSpace *ns = mtd->lazyCompileNameSpace;
	kMethod_compile(kctx, mtd, ns, text, uline);
	((KonohaContextVar*)kctx)->esp = esp;
	mtd->invokeMethodFunc(kctx, sfp); // call again;
}

static void kMethod_setLazyCompilation(KonohaContext *kctx, kMethodVar *mtd, kStmt *stmt, kNameSpace *ns)
{
	kToken *tcode = SUGAR kStmt_getToken(kctx, stmt, KW_BlockPattern, NULL);
	if(tcode != NULL && tcode->resolvedSyntaxInfo->keyword == TokenType_CODE) {
		KSETv(mtd, mtd->sourceCodeToken, tcode);
		KSETv(mtd, mtd->lazyCompileNameSpace, ns);
		KLIB kMethod_setFunc(kctx, mtd, MethodFunc_lazyCompilation);
		KLIB kArray_add(kctx, KonohaContext_getSugarContext(kctx)->definedMethodList, mtd);
	}
}

static void kNameSpace_compileAllDefinedMethods(KonohaContext *kctx)
{
	size_t i, size = kArray_size(KonohaContext_getSugarContext(kctx)->definedMethodList);
	for (i = 0; i < size; ++i) {
		kMethod *mtd = KonohaContext_getSugarContext(kctx)->definedMethodList->methodItems[i];
		if (mtd->invokeMethodFunc == MethodFunc_lazyCompilation) {
			kString *text = mtd->sourceCodeToken->text;
			kfileline_t uline = mtd->sourceCodeToken->uline;
			kNameSpace *ns = mtd->lazyCompileNameSpace;
			kMethod_compile(kctx, mtd, ns, text, uline);
			assert(mtd->invokeMethodFunc != MethodFunc_lazyCompilation);
		}
	}
	KLIB kArray_clear(kctx, KonohaContext_getSugarContext(kctx)->definedMethodList, 0);
}

///* ------------------------------------------------------------------------ */
///* [ParamUtils] */

static kbool_t StmtTypeDecl_setParam(KonohaContext *kctx, kStmt *stmt, int n, kparamtype_t *p)
{
	kToken *tkT  = SUGAR kStmt_getToken(kctx, stmt, KW_TypePattern, NULL);
	kExpr  *expr = SUGAR kStmt_getExpr(kctx, stmt, KW_ExprPattern, NULL);
	DBG_ASSERT(tkT != NULL);
	DBG_ASSERT(expr != NULL);
	if(Expr_isSymbolTerm(expr)) {
		kToken *tkN = expr->termToken;
		ksymbol_t fn = ksymbolA(S_text(tkN->text), S_size(tkN->text), SYM_NEWID);
		p[n].fn = fn;
		p[n].ty = Token_typeLiteral(tkT);
		return true;
	}
	return false;
}

static kParam *kStmt_newMethodParamNULL(KonohaContext *kctx, kStmt *stmt, kGamma* gma)
{
	kParam *pa = (kParam*)kStmt_getObjectNULL(kctx, stmt, KW_ParamPattern);
	if(pa == NULL || !IS_Param(pa)) {
		SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), KW_ParamPattern);
		if(!SugarSyntax_tyCheckStmt(kctx, syn, stmt, gma)) {
			return NULL;
		}
	}
	pa = (kParam*)kStmt_getObjectNULL(kctx, stmt, KW_ParamPattern);
	DBG_ASSERT(IS_Param(pa));
	return pa;
}

static KMETHOD Statement_ParamsDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kToken *tkT = SUGAR kStmt_getToken(kctx, stmt, KW_TypePattern, NULL); // type
	ktype_t rtype =  tkT == NULL ? TY_void : Token_typeLiteral(tkT);
	kParam *pa = NULL;
	kBlock *params = (kBlock*)kStmt_getObjectNULL(kctx, stmt, KW_ParamPattern);
	if(params == NULL) {
		pa = new_kParam(kctx, rtype, 0, NULL);
	}
	else if(IS_Block(params)) {
		size_t i, psize = kArray_size(params->stmtList);
		kparamtype_t *p = ALLOCA(kparamtype_t, psize);
		for(i = 0; i < psize; i++) {
			p[i].ty = TY_void; p[i].fn = 0;
			kStmt *stmt = params->stmtList->stmtItems[i];
			if(stmt->syn->keyword != KW_TypeDeclPattern || !StmtTypeDecl_setParam(kctx, stmt, i, p)) {
				break;
			}
		}
		pa = new_kParam(kctx, rtype, psize, p);
	}
	if(pa != NULL && IS_Param(pa)) {
		KLIB kObject_setObject(kctx, stmt, KW_ParamPattern, TY_Param, pa);
		kStmt_done(kctx, stmt);
		RETURNb_(true);
	}
	kStmt_printMessage(kctx, stmt, ErrTag, "expected parameter declaration");
	RETURNb_(false);
}

///* ------------------------------------------------------------------------ */
///* [MethodDecl] */

static ktype_t kStmt_getClassId(KonohaContext *kctx, kStmt *stmt, kNameSpace *ns, ksymbol_t kw, ktype_t defcid)
{
	kToken *tk = (kToken*)kStmt_getObjectNULL(kctx, stmt, kw);
	if(tk == NULL || !IS_Token(tk)) {
		return defcid;
	}
	else {
		DBG_ASSERT(Token_isVirtualTypeLiteral(tk));
		return Token_typeLiteral(tk);
	}
}

static ksymbol_t kStmt_getMethodSymbol(KonohaContext *kctx, kStmt *stmt, kNameSpace *ns, ksymbol_t kw, kmethodn_t defmn)
{
	kToken *tk = (kToken*)kStmt_getObjectNULL(kctx, stmt, kw);
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
	uintptr_t flag    = kStmt_parseFlag(kctx, stmt, MethodDeclFlag, 0);
	kNameSpace *ns    = Stmt_nameSpace(stmt);
	ktype_t typeId    = kStmt_getClassId(kctx, stmt, ns, SYM_("ClassName"), O_typeId(ns));
	kmethodn_t mn     = kStmt_getMethodSymbol(kctx, stmt, ns, KW_SymbolPattern, MN_new);
	kParam *pa        = kStmt_newMethodParamNULL(kctx, stmt, gma);
	if(TY_is(Singleton, typeId)) { flag |= kMethod_Static; }
	if(TY_is(Final, typeId)) { flag |= kMethod_Final; }
	if(pa != NULL) {  // if pa is NULL, error is printed out.
		kMethod *mtd = KLIB new_kMethod(kctx, flag, typeId, mn, NULL);
		PUSH_GCSTACK(mtd);
		KLIB kMethod_setParam(kctx, mtd, pa->rtype, pa->psize, (kparamtype_t*)pa->paramtypeItems);
		kMethod *foundMethod = kNameSpace_addMethod(kctx, ns, mtd);
		if(foundMethod != NULL) {
			pa = NULL;
			if(mtd->typeId == foundMethod->typeId) {
				kStmt_printMessage(kctx, stmt, ErrTag, "method %s.%s%s has already defined", Method_t(mtd));
			}
			else {
				kStmt_printMessage(kctx, stmt, ErrTag, "method %s.%s%s is final", Method_t(mtd));
			}
		}
		if(pa != NULL) {
			kMethod_setLazyCompilation(kctx, (kMethodVar*)mtd, stmt, ns);
		}
	}
	kStmt_done(kctx, stmt);
	RETURNb_(pa != NULL);
}

/* ------------------------------------------------------------------------ */

#define PATTERN(T)  KW_##T##Pattern
#define GROUP(T)    KW_##T##Group
#define TOKEN(T)    KW_##T

static void defineDefaultSyntax(KonohaContext *kctx, kNameSpace *ns)
{
	DBG_ASSERT(SYM_("$Param") == KW_ParamPattern);
	DBG_ASSERT(SYM_(".") == KW_DOT);
	DBG_ASSERT(SYM_(":") == KW_COLON);
	DBG_ASSERT(SYM_("true") == KW_true);
	DBG_ASSERT(SYM_("return") == KW_return);
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
		{ KW_END, },
	};
	kNameSpace_defineSyntax(kctx, ns, SYNTAX, ns);
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
