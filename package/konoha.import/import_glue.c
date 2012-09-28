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
#include <stdio.h>

#ifdef __cplusplus
extern "C"{
#endif

static KMETHOD StmtTyCheck_import(KonohaContext *kctx, KonohaStack *sfp)
{
	int ret = false;
	VAR_StmtTyCheck(stmt, gma);
	kTokenArray *tokenList = (kTokenArray *) kStmt_getObjectNULL(kctx, stmt, KW_TokenPattern);
	if (tokenList == NULL) {
		RETURNb_(false);
	}
	ksymbol_t star = SYM_("*");
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	int i = 0;
	if (i + 2 < kArray_size(tokenList)) {
		for (; i < kArray_size(tokenList)-1; i+=2) {
			/* name . */
			kToken *tk  = tokenList->tokenItems[i+0];
			//assert(tk->keyword  == TokenType_SYMBOL);
			//assert(dot->keyword == KW_DOT);
			if (i+2 < kArray_size(tokenList)) {
				kToken *startTk = tokenList->tokenItems[i+2];
				if (startTk->resolvedSyntaxInfo->keyword == star) {
					break;
				}
			}
			KLIB Kwb_write(kctx, &wb, S_text(tk->text), S_size(tk->text));
			KLIB Kwb_write(kctx, &wb, ".", 1);
		}
	}
	kString *name = tokenList->tokenItems[i]->text;
	KLIB Kwb_write(kctx, &wb, S_text(name), S_size(name));
	kString *pkgname = KLIB new_kString(kctx, KLIB Kwb_top(kctx, &wb, 1), Kwb_bytesize(&wb), 0);
	kNameSpace *ns = Stmt_nameSpace(stmt);
	SugarSyntaxVar *syn1 = (SugarSyntaxVar*) SYN_(ns, KW_ExprMethodCall);
	kTokenVar *tkImport = GCSAFE_new(TokenVar, 0);
	kExpr *ePKG = new_ConstValueExpr(kctx, TY_String, UPCAST(pkgname));
	tkImport->resolvedSymbol = MN_("import");
	kExpr *expr = SUGAR new_UntypedCallStyleExpr(kctx, syn1, 3, tkImport, new_ConstValueExpr(kctx, O_typeId(ns), UPCAST(ns)), ePKG);
	KLIB kObject_setObject(kctx, stmt, KW_ExprPattern, TY_Expr, expr);
	ret = SUGAR kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_boolean, 0);
	if (ret) {
		kStmt_typed(stmt, EXPR);
	}
	RETURNb_(ret);
}

// --------------------------------------------------------------------------

static kbool_t import_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	return true;
}

static kbool_t import_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t import_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("import"), .rule = "\"import\" $Token [ \".*\"] ", TopStmtTyCheck_(import)},
		{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNameSpace);
	return true;
}

static kbool_t import_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* import_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("import", "1.0"),
		.initPackage = import_initPackage,
		.setupPackage = import_setupPackage,
		.initNameSpace = import_initNameSpace,
		.setupNameSpace = import_setupNameSpace,
	};
	return &d;
}
#ifdef __cplusplus
}
#endif

