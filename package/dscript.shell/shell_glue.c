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

// --------------------------------------------------------------------------

static kbool_t shell_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	return true;
}

static kbool_t shell_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

// --------------------------------------------------------------------------

static KMETHOD StmtTyCheck_dsh(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_StmtTyCheck(stmt, gma);
	kTokenArray *tokenList = (kTokenArray *)kStmt_getObjectNULL(kctx, stmt, KW_TokenPattern);
	if(tokenList == NULL) {
		RETURNb_(false);
	}

	size_t i;
	KUtilsWriteBuffer wb;
	kNameSpace *ns = Stmt_nameSpace(stmt);
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	for(i = 0; i < kArray_size(tokenList); i++) {
		kToken *token = tokenList->tokenItems[i];
		KLIB Kwb_write(kctx, &wb, S_text(token->text), S_size(token->text));
		if(Token_isBeforeWhiteSpace(token)) {
			KLIB Kwb_write(kctx, &wb, " ", 1);
		}
	}
	kString *cmd = KLIB new_kString(kctx, KLIB Kwb_top(kctx, &wb, 0), Kwb_bytesize(&wb), 0);
	PUSH_GCSTACK(cmd);
	KLIB Kwb_free(&wb);
	DBG_P("cmd$ %s", S_text(cmd));

	SugarSyntaxVar *syn = (SugarSyntaxVar*) SYN_(ns, KW_ExprMethodCall);
	kTokenVar *callToken = GCSAFE_new(TokenVar, 0);
	kExpr *callExpr = new_ConstValueExpr(kctx, TY_String, UPCAST(cmd));
	callToken->resolvedSymbol = MN_("call");
	const char cname[] = "Subproc";
	kExpr *expr = SUGAR new_UntypedCallStyleExpr(kctx, syn, 3, callToken,
			new_UnboxConstValueExpr(kctx, KLIB kNameSpace_getClass(kctx, ns, cname, strlen(cname), NULL)->typeId, 0), callExpr);
	KLIB kObject_setObject(kctx, stmt, KW_ExprPattern, TY_Expr, expr);
	kbool_t ret = SUGAR kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_int, 0);
	if(ret) {
		kStmt_typed(stmt, EXPR);
	}
	RETURNb_(ret);
}

// ----------------------------------------------------------------------------
/* define class */

static kbool_t shell_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	//KImportPackage(ns, "dscript.dollar", pline);
	KImportPackage(ns, "dscript.subproc", pline);
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("dsh"), .rule = "\"dsh\" $Token", TopStmtTyCheck_(dsh), StmtTyCheck_(dsh)},
		{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNameSpace);
	return true;
}

static kbool_t shell_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* shell_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("dscript.shell", "1.0"),
		.initPackage    = shell_initPackage,
		.setupPackage   = shell_setupPackage,
		.initNameSpace  = shell_initNameSpace,
		.setupNameSpace = shell_setupNameSpace,
	};
	return &d;
}
