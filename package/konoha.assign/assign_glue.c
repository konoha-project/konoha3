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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>

#ifdef __cplusplus
extern "C"{
#endif
// --------------------------------------------------------------------------

static kbool_t assign_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	return true;
}

static kbool_t assign_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static KMETHOD ParseExpr_SelfAssign(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	kNameSpace *ns = Stmt_nameSpace(stmt);
	kToken *selfAssignToken = tokenList->tokenItems[operatorIdx];
	DBG_ASSERT(S_size(selfAssignToken->text) > 1);
	ksymbol_t opSymbol = KLIB Ksymbol(kctx, S_text(selfAssignToken->text), S_size(selfAssignToken->text) - 1, SPOL_ASCII, _NEWID);
	SugarSyntax *opSyntax = SYN_(ns, opSymbol);
	if(opSyntax != NULL) {
		TokenRange macroRangeBuf, *macroRange = SUGAR new_TokenListRange(kctx, Stmt_nameSpace(stmt), tokenList, &macroRangeBuf);
		SUGAR TokenRange_tokenize(kctx, macroRange, "A = ( A ) + ( B )", 0);

		TokenRange opRangeBuf, *opRange = SUGAR new_TokenStackRange(kctx, macroRange, &opRangeBuf);
		kTokenVar *opToken = GCSAFE_new(TokenVar, TokenType_SYMBOL);
		KSETv(opToken, opToken->text, SYM_s(opSymbol));
		KLIB kArray_add(kctx, opRange->tokenList, opToken);
		TokenRange_end(kctx, opRange);

		TokenRange newexprRangeBuf, *newexprRange = SUGAR new_TokenStackRange(kctx, opRange, &newexprRangeBuf);
		MacroSet macroSet[] = {
			{SYM_("A"), tokenList, beginIdx, operatorIdx},
			{SYM_("B"), tokenList, operatorIdx+1, endIdx},
			{SYM_("+"), opRange->tokenList, opRange->beginIdx, opRange->endIdx},
			{0, NULL, 0, 0},
		};
		macroRange->macroSet = macroSet;
		SUGAR TokenRange_resolved(kctx, newexprRange, macroRange);
//		KdumpTokenRange(kctx, "replaced", newexprRange);

		kExpr *expr = SUGAR kStmt_parseExpr(kctx, stmt, newexprRange->tokenList, newexprRange->beginIdx, newexprRange->endIdx);
		TokenRange_pop(kctx, macroRange);
		RETURN_(expr);
	}
	else {
		SUGAR kStmt_printMessage2(kctx, stmt, selfAssignToken, ErrTag, "undefined binary operator: %s", SYM_t(opSymbol));
	}
}

static kbool_t assign_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("+="), (SYNFLAG_ExprLeftJoinOp2), NULL, C_PRECEDENCE_ASSIGN, 0, NULL, ParseExpr_SelfAssign, NULL, NULL, NULL, },
		{ SYM_("-="), (SYNFLAG_ExprLeftJoinOp2), NULL, C_PRECEDENCE_ASSIGN, 0, NULL, ParseExpr_SelfAssign, NULL, NULL, NULL, },
		{ SYM_("*="), (SYNFLAG_ExprLeftJoinOp2), NULL, C_PRECEDENCE_ASSIGN, 0, NULL, ParseExpr_SelfAssign, NULL, NULL, NULL, },
		{ SYM_("/="), (SYNFLAG_ExprLeftJoinOp2), NULL, C_PRECEDENCE_ASSIGN, 0, NULL, ParseExpr_SelfAssign, NULL, NULL, NULL, },
		{ SYM_("%="), (SYNFLAG_ExprLeftJoinOp2), NULL, C_PRECEDENCE_ASSIGN, 0, NULL, ParseExpr_SelfAssign, NULL, NULL, NULL, },
		{ KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNameSpace);
	return true;
}

static kbool_t assign_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* assign_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSETPACKNAME(d, "assign", "1.0");
	d.initPackage    = assign_initPackage;
	d.setupPackage   = assign_setupPackage;
	d.initNameSpace  = assign_initNameSpace;
	d.setupNameSpace = assign_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
