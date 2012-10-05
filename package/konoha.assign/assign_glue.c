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

static KMETHOD ParseExpr_BinarySugar(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ParseExpr(stmt, tokenList, beginIdx, operatorIdx, endIdx);
	kToken *opToken = tokenList->tokenItems[operatorIdx];
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
		SUGAR TokenSequence_applyMacro(kctx, &macro, opSyntax->macroDataNULL, opSyntax->macroParamSize, macroParam);
		kExpr *expr = SUGAR kStmt_parseExpr(kctx, stmt, macro.tokenList, macro.beginIdx, macro.endIdx);
		TokenSequence_pop(kctx, macro);
		RETURN_(expr);
	}
}

static kbool_t assign_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("+="), (SYNFLAG_ExprLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, ParseExpr_BinarySugar, NULL, NULL, NULL, },
		{ SYM_("-="), (SYNFLAG_ExprLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, ParseExpr_BinarySugar, NULL, NULL, NULL, },
		{ SYM_("*="), (SYNFLAG_ExprLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, ParseExpr_BinarySugar, NULL, NULL, NULL, },
		{ SYM_("/="), (SYNFLAG_ExprLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, ParseExpr_BinarySugar, NULL, NULL, NULL, },
		{ SYM_("%="), (SYNFLAG_ExprLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, ParseExpr_BinarySugar, NULL, NULL, NULL, },
		{ SYM_("|="), (SYNFLAG_ExprLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, ParseExpr_BinarySugar, NULL, NULL, NULL, },
		{ SYM_("&="), (SYNFLAG_ExprLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, ParseExpr_BinarySugar, NULL, NULL, NULL, },
		{ SYM_("<<="), (SYNFLAG_ExprLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, ParseExpr_BinarySugar, NULL, NULL, NULL, },
		{ SYM_(">>="), (SYNFLAG_ExprLeftJoinOp2), NULL, Precedence_CStyleASSIGN, 0, NULL, ParseExpr_BinarySugar, NULL, NULL, NULL, },
		{ KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNameSpace);
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_("+="), 2,  "X Y X = (X) + (Y)");
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_("-="), 2,  "X Y X = (X) - (Y)");
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_("*="), 2,  "X Y X = (X) * (Y)");
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_("/="), 2,  "X Y X = (X) / (Y)");
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_("%="), 2,  "X Y X = (X) % (Y)");
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_("|="), 2,  "X Y X = (X) | (Y)");
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_("&="), 2,  "X Y X = (X) & (Y)");
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_("<<="), 2, "X Y X = (X) << (Y)");
	SUGAR kNameSpace_setMacroData(kctx, ns, SYM_(">>="), 2, "X Y X = (X) >> (Y)");
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
