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

/* Expression */

static KMETHOD Statement_while(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	DBG_P("while statement .. ");
	int ret = false;
	if(SUGAR kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_boolean, 0)) {
		kBlock *bk = SUGAR kStmt_getBlock(kctx, stmt, NULL/*DefaultNameSpace*/, KW_BlockPattern, K_NULLBLOCK);
		kStmt_set(CatchContinue, stmt, true);  // set before tyCheckAll
		kStmt_set(CatchBreak, stmt, true);
		ret = SUGAR kBlock_tyCheckAll(kctx, bk, gma);
		if(ret) {
			kStmt_typed(stmt, LOOP);
		}
	}
	KReturnUnboxValue(ret);
}

static KMETHOD Statement_do(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	DBG_P("do statement .. ");
	int ret = false;
	if(SUGAR kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_boolean, 0)) {
		kBlock *bk = SUGAR kStmt_getBlock(kctx, stmt, NULL/*DefaultNameSpace*/, KW_BlockPattern, K_NULLBLOCK);
		kStmt_set(CatchContinue, stmt, true);  // set before tyCheckAll
		kStmt_set(CatchBreak, stmt, true);
		ret = SUGAR kBlock_tyCheckAll(kctx, bk, gma);
		if(ret) {
			kStmt_typed(stmt, LOOP);  // FIXME
		}
	}
	KReturnUnboxValue(ret);
}

static KMETHOD Statement_CStyleFor(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	DBG_P("for statement .. ");
	int ret = false;
	if(SUGAR kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_boolean, 0)) {
		kBlock *bk = SUGAR kStmt_getBlock(kctx, stmt, NULL/*DefaultNameSpace*/, KW_BlockPattern, K_NULLBLOCK);
		kStmt_set(CatchContinue, stmt, true);  // set before tyCheckAll
		kStmt_set(CatchBreak, stmt, true);
		ret = SUGAR kBlock_tyCheckAll(kctx, bk, gma);
		if(ret) {
			kStmt_typed(stmt, LOOP);
		}
	}
	KReturnUnboxValue(ret);
}

static inline kStmt* kStmt_getParentNULL(kStmt *stmt)
{
	return stmt->parentBlockNULL->parentStmtNULL;
}

static KMETHOD Statement_break(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kStmt *p = stmt;
	while((p = kStmt_getParentNULL(p)) != NULL) {
		if(kStmt_is(CatchBreak, p)) {
			KLIB kObject_setObject(kctx, stmt, stmt->syn->keyword, TY_Stmt, p);
			kStmt_typed(stmt, JUMP);
			KReturnUnboxValue(true);
		}
	}
	SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, "break statement not within a loop");
}

static KMETHOD Statement_continue(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kStmt *p = stmt;
	while((p = kStmt_getParentNULL(p)) != NULL) {
		if(kStmt_is(CatchContinue, p)) {
			KLIB kObject_setObject(kctx, stmt, stmt->syn->keyword, TY_Stmt, p);
			kStmt_typed(stmt, JUMP);
			KReturnUnboxValue(true);
		}
	}
	SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, "continue statement not within a loop");
}

/* Literal */

static KMETHOD TokenFunc_SingleQuotedChar(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *)sfp[1].asObject;
	int ch, prev = '/', pos = 1;
	const char *source = S_text(sfp[2].asString);
	while((ch = source[pos++]) != 0) {
		if(ch == '\n') {
			break;
		}
		if(ch == '\'' && prev != '\\') {
			if(IS_NOTNULL(tk)) {
				KFieldSet(tk, tk->text, KLIB new_kString(kctx, OnField, source + 1, (pos-2), 0));
				tk->unresolvedTokenType = SYM_("$SingleQuotedChar");
			}
			KReturnUnboxValue(pos);
		}
		prev = ch;
	}
	if(IS_NOTNULL(tk)) {
		SUGAR kToken_printMessage(kctx, tk, ErrTag, "must close with %s", "'");
	}
	KReturnUnboxValue(0);
}

static KMETHOD TypeCheck_SingleQuotedChar(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->termToken;
	if(S_size(tk->text) == 1) {
		int ch = S_text(tk->text)[0];
		KReturn(SUGAR kExpr_setUnboxConstValue(kctx, expr, TY_int, ch));
	}
	else if(S_size(tk->text) == 2) {
		int ch = S_text(tk->text)[0];
		if(ch == '\\') {
			ch = S_text(tk->text)[1];
			switch(ch) {
			case '\'': ch = '\''; break;
			case '\\': ch = '\\'; break;
			case 'b':  ch = '\b'; break;
			case 'f':  ch = '\f'; break;
			case 'n':  ch = '\n'; break;
			case 'r':  ch = '\r'; break;
			case 't':  ch = '\t'; break;
			default:
				KReturn(K_NULLEXPR);
			}
			KReturn(SUGAR kExpr_setUnboxConstValue(kctx, expr, TY_int, ch));
		}
	}
	KReturn(K_NULLEXPR);
}

/* Expression */

static KMETHOD Expression_Indexer(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	KonohaClass *genericsClass = NULL;
	kNameSpace *ns = Stmt_nameSpace(stmt);
	int nextIdx = SUGAR TokenUtils_parseTypePattern(kctx, ns, tokenList, beginIdx, endIdx, &genericsClass);
	if(nextIdx != -1) {  // to avoid Func[T]
		KReturn(SUGAR kStmt_parseOperatorExpr(kctx, stmt, tokenList->TokenItems[beginIdx]->resolvedSyntaxInfo, tokenList, beginIdx, beginIdx, endIdx));
	}
	DBG_P("beginIdx=%d, endIdx=%d", beginIdx, endIdx);
	kToken *currentToken = tokenList->TokenItems[operatorIdx];
	if(beginIdx < operatorIdx) {
		kExpr *leftExpr = SUGAR kStmt_parseExpr(kctx, stmt, tokenList, beginIdx, operatorIdx, NULL);
		if(leftExpr == K_NULLEXPR) {
			KReturn(leftExpr);
		}
		/* transform 'Value0 [ Value1 ]=> (Call Value0 get (Value1)) */
		kTokenVar *tkN = new_(TokenVar, 0, OnGcStack);
		tkN->resolvedSymbol= MN_toGETTER(0);
		tkN->uline = currentToken->uline;
		SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall);
		leftExpr  = SUGAR new_UntypedCallStyleExpr(kctx, syn, 2, tkN, leftExpr);
		leftExpr = SUGAR kStmt_addExprParam(kctx, stmt, leftExpr, currentToken->subTokenList, 0, kArray_size(currentToken->subTokenList), "[");
		KReturn(SUGAR kStmt_rightJoinExpr(kctx, stmt, leftExpr, tokenList, operatorIdx + 1, endIdx));
	}
	DBG_P("nothing");
}

static KMETHOD Expression_Increment(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	DBG_P("beginIdx=%d, endIdx=%d", beginIdx, endIdx);
	kToken *currentToken = tokenList->TokenItems[operatorIdx];
	SugarSyntax *opSyntax = currentToken->resolvedSyntaxInfo;
	TokenSequence macro = {Stmt_nameSpace(stmt), tokenList};
	TokenSequence_push(kctx, macro);
	macro.TargetPolicy.RemovingIndent = true;
	if(beginIdx == endIdx) { /* ++A  MACRO    X X = (X) + 1 */
		MacroSet macroParam[] = {
			{SYM_("X"), tokenList, operatorIdx+1, endIdx},
			{0, NULL, 0, 0}, /* sentinel */
		};
		SUGAR TokenSequence_applyMacro(kctx, &macro, opSyntax->macroDataNULL_OnList, 0, 5, 1, macroParam);
	}
	else {/* (beginIdx < operatorIdx) MACRO ${ int _ = X; X = (X) + 1; _} */
		TokenSequence macro = {Stmt_nameSpace(stmt), tokenList};
		MacroSet macroParam[] = {
			{SYM_("X"), tokenList, beginIdx, operatorIdx},
			{0, NULL, 0, 0}, /* sentinel */
		};
		SUGAR TokenSequence_applyMacro(kctx, &macro, opSyntax->macroDataNULL_OnList, 5, kArray_size(opSyntax->macroDataNULL_OnList), 1, macroParam);
	}
	kExpr *expr = SUGAR kStmt_parseExpr(kctx, stmt, macro.tokenList, macro.beginIdx, macro.endIdx, NULL/*FIXME*/);
	TokenSequence_pop(kctx, macro);
	KReturn(expr);
}

/* ------------------------------------------------------------------------ */
/* assignment */

static KMETHOD Expression_BinarySugar(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	kToken *opToken = tokenList->TokenItems[operatorIdx];
	SugarSyntax *opSyntax = opToken->resolvedSyntaxInfo;
	if(opSyntax->macroParamSize == 2) {
		TokenSequence macro = {Stmt_nameSpace(stmt), tokenList};
		TokenSequence_push(kctx, macro);
		MacroSet macroParam[] = {
			{SYM_("X"), tokenList, beginIdx, operatorIdx},
			{SYM_("Y"), tokenList, operatorIdx+1, endIdx},
			{0, NULL, 0, 0},
		};
		macro.TargetPolicy.RemovingIndent = true;
		SUGAR TokenSequence_applyMacro(kctx, &macro, opSyntax->macroDataNULL_OnList, 0, kArray_size(opSyntax->macroDataNULL_OnList), opSyntax->macroParamSize, macroParam);
		kExpr *expr = SUGAR kStmt_parseExpr(kctx, stmt, macro.tokenList, macro.beginIdx, macro.endIdx, NULL);
		TokenSequence_pop(kctx, macro);
		KReturn(expr);
	}
}

static kbool_t cstyle_defineAssign(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("+="), (SYNFLAG_ExprLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
		{ SYM_("-="), (SYNFLAG_ExprLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
		{ SYM_("*="), (SYNFLAG_ExprLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
		{ SYM_("/="), (SYNFLAG_ExprLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
		{ SYM_("%="), (SYNFLAG_ExprLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },

		{ SYM_("<<"), 0, NULL, Precedence_CStyleSHIFT,  0,                     NULL, NULL, NULL, NULL, NULL, },
		{ SYM_(">>"), 0, NULL, Precedence_CStyleSHIFT,  0,                     NULL, NULL, NULL, NULL, NULL, },
		{ SYM_("&"),  0, NULL, Precedence_CStyleBITAND, 0,                     NULL, NULL, NULL, NULL, NULL, },
		{ SYM_("|"),  0, NULL, Precedence_CStyleBITOR,  0,                     NULL, NULL, NULL, NULL, NULL, },
		{ SYM_("^"),  0, NULL, Precedence_CStyleBITXOR, 0,                     NULL, NULL, NULL, NULL, NULL, },

		{ SYM_("|="), (SYNFLAG_ExprLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
		{ SYM_("&="), (SYNFLAG_ExprLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
		{ SYM_("<<="), (SYNFLAG_ExprLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
		{ SYM_(">>="), (SYNFLAG_ExprLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
		{ SYM_("^="), (SYNFLAG_ExprLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, Expression_BinarySugar, NULL, NULL, NULL, },
		{ KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX);
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_("+="), 2,  "X Y X = (X) + (Y)");
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_("-="), 2,  "X Y X = (X) - (Y)");
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_("*="), 2,  "X Y X = (X) * (Y)");
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_("/="), 2,  "X Y X = (X) / (Y)");
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_("%="), 2,  "X Y X = (X) % (Y)");
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_("|="), 2,  "X Y X = (X) | (Y)");
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_("&="), 2,  "X Y X = (X) & (Y)");
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_("<<="), 2, "X Y X = (X) << (Y)");
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_(">>="), 2, "X Y X = (X) >> (Y)");
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_("^="), 2,  "X Y X = (X) ^ (Y)");
	return true;
}

// --------------------------------------------------------------------------

static kbool_t cstyle_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	KDEFINE_SYNTAX defStatement[] = {
		{ SYM_("while"), 0, "\"while\" \"(\" $Expr \")\" $Block", 0, 0, NULL, NULL, NULL, Statement_while, NULL, },
		{ SYM_("do"), 0, "\"do\"  $Block \"while\" \"(\" $Expr \")\"", 0, 0, NULL, NULL, NULL, Statement_do, NULL, },
		{ SYM_("for"), 0, "\"for\" \"(\" $ForStmt \")\" $Block", 0, 0, NULL, NULL, NULL, Statement_CStyleFor, NULL, },
		{ SYM_("break"), 0, "\"break\"", 0, 0, NULL, NULL, NULL, Statement_break, NULL, },
		{ SYM_("continue"), 0, "\"continue\"", 0, 0, NULL, NULL, NULL, Statement_continue, NULL, },
		{ KW_END, }, /* sentinental */
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, defStatement);

	KDEFINE_SYNTAX defLiteral[] = {
		{ SYM_("$SingleQuotedChar"), 0, NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_SingleQuotedChar, },
		{ KW_END, }, /* sentinental */
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, defLiteral);
	SUGAR kNameSpace_setTokenFunc(kctx, ns, SYM_("$SingleQuotedChar"), KonohaChar_Quote, new_SugarFunc(ns, TokenFunc_SingleQuotedChar));

	KDEFINE_SYNTAX defExpression[] = {
		{ SYM_("[]"), SYNFLAG_ExprPostfixOp2, NULL, Precedence_CStyleCALL, 0, NULL, Expression_Indexer, NULL, NULL, NULL, },
		{ SYM_("++"), SYNFLAG_ExprPostfixOp2, NULL, Precedence_CStyleCALL, Precedence_CStylePREUNARY, NULL, Expression_Increment,},
		{ SYM_("--"), SYNFLAG_ExprPostfixOp2, NULL, Precedence_CStyleCALL, Precedence_CStylePREUNARY, NULL, Expression_Increment,},
		{ KW_END, }, /* sentinental */
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, defExpression);
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_("++"), 1,  "X X = (X) + 1 X ${int _ = X; X = (X) + 1; _}");
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_("--"), 1,  "X X = (X) - 1 X ${int _ = X; X = (X) - 1; _}");
	cstyle_defineAssign(kctx, ns, trace);
	return true;
}

static kbool_t cstyle_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	cstyle_defineAssign(kctx, ns, trace);
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* cstyle_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "cstyle", "1.0");
	d.initPackage    = cstyle_initPackage;
	d.setupPackage   = cstyle_setupPackage;
	return &d;
}

// --------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif
