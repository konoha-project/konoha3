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

// for isCommand
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
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

static const char *expandDollarToken(KonohaContext *kctx, kTokenArray *tokens, size_t start, size_t end)
{
	// TODO: Expand $token by using konoha variable, return char pointer.
	return NULL;
}

static kbool_t checkPath(const char *path, const char *cmd)
{
	struct stat sb;
	char buf[PATH_MAX];
	const char *fullpath;
	if(path != NULL) {
		snprintf(buf, PATH_MAX, "%s/%s", path, cmd);
		fullpath = buf;
	}
	else {
		fullpath = cmd;
	}
	if(lstat(fullpath, &sb) == -1) {
		return false;
	}
	return (sb.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH));
}

static kbool_t isCommand(const char *cmd)
{
	if(strchr(cmd, ' ') != NULL) {
		return false;
	}
	if(cmd[0] == '/') {
		return checkPath(NULL, cmd);
	}
	size_t bufsize = confstr(_CS_PATH, NULL, 0);
	char buf[bufsize];
	confstr(_CS_PATH, buf, bufsize);
	char *pos, *p = buf;
	while(pos < buf + bufsize) {
		if ((pos = strchr(p, ':')) == NULL) {
			if(checkPath(p, cmd)) return true;
			break;
		}
		p[pos - p] = '\0';
		if(checkPath(p, cmd)) return true;
		p = pos + 1;
	}
	return false;
}

static kString *splitWhiteSpace(KonohaContext *kctx, kTokenArray *tokenList)
{
	size_t i;
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	if(O_typeId(tokenList) == TY_Token) {
		/* Single token was passed (e.g. "dsh ls;"). */
		kToken *token = (kToken *)tokenList;
		KLIB Kwb_write(kctx, &wb, S_text(token->text), S_size(token->text));
	}
	else {
		/* Multiple tokens was passed (e.g. "dsh ls -la;"). */
		for(i = 0; i < kArray_size(tokenList); i++) {
			kToken *token = tokenList->tokenItems[i];
			if(token->resolvedSymbol == SYM_("|")) {
				// TODO: PIPE
			}
			else if(token->resolvedSymbol == SYM_("$")) {
				// TODO: parse dollar token ($token)
				size_t start = i;
				while(i < kArray_size(tokenList) && !kToken_is(BeforeWhiteSpace, tokenList->tokenItems[i])) {
					++i;
				}
				const char *dollarstr = expandDollarToken(kctx, tokenList, start, i-1);
				if(dollarstr == NULL) {
					KLIB Kwb_free(&wb);
					return NULL;
				}
				else {
					KLIB Kwb_write(kctx, &wb, dollarstr, strlen(dollarstr));
				}
			}
			else {
				KLIB Kwb_write(kctx, &wb, S_text(token->text), S_size(token->text));
			}
			if(kToken_is(BeforeWhiteSpace, token)) {
				KLIB Kwb_write(kctx, &wb, " ", 1);
			}
		}
	}
	kString *cmd = KLIB new_kString(kctx, KLIB Kwb_top(kctx, &wb, 0), Kwb_bytesize(&wb), 0);
	KLIB Kwb_free(&wb);
	return cmd;
}

static KMETHOD Statement_dsh(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kTokenArray *tokenList = (kTokenArray *)kStmt_getObjectNULL(kctx, stmt, KW_TokenPattern);
	if(tokenList == NULL) {
		RETURNb_(false);
	}

	kString *cmd = splitWhiteSpace(kctx, tokenList);
	if(cmd == NULL) {
		RETURNb_(false);
	}
	DBG_P("cmd=%s", S_text(cmd));
	DBG_P("iscommand=%d", isCommand(S_text(cmd)));

	//TODO: generate eval("cmd") syntax

	kNameSpace *ns = Stmt_nameSpace(stmt);
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

static kbool_t DSLib_checkExecutablePath(KonohaContext *kctx, const char *path, const char *cmd)
{
	char buf[PATH_MAX];
	struct stat sb;
	const char *fullpath;
	if(path != NULL) {
		snprintf(buf, PATH_MAX, "%s/%s", path, cmd);
		fullpath = buf;
	}
	else {
		fullpath = cmd;
	}
	DBG_P("path='%s'", fullpath);
	if(lstat(fullpath, &sb) == -1) {
		return false;
	}
	return (sb.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH));
}

static kbool_t DSLib_isCommand(KonohaContext *kctx, const char *cmd)
{
	size_t bufsize = confstr(_CS_PATH, NULL, 0);
	char buf[bufsize];
	confstr(_CS_PATH, buf, bufsize);
	char *pos, *p = buf;
	while(p < buf + bufsize) {
		if ((pos = strchr(p, ':')) == NULL) {
			if(DSLib_checkExecutablePath(kctx, p, cmd)) return true;
			break;
		}
		p[pos - p] = '\0';
		if(DSLib_checkExecutablePath(kctx, p, cmd)) return true;
		p = pos + 1;
	}
	return DSLib_checkExecutablePath(kctx, "/bin", cmd);
}


static KMETHOD PatternMatch_Shell(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, nameid, tokenList, beginIdx, endIdx);
	kToken *firstToken = tokenList->tokenItems[beginIdx];
	DBG_P("firstToken='%s', isCommand=%d", S_text(firstToken->text), DSLib_isCommand(kctx, S_text(firstToken->text)));
	RETURNi_((firstToken->resolvedSyntaxInfo->keyword == KW_SymbolPattern && DSLib_isCommand(kctx, S_text(firstToken->text))) ? beginIdx : -1);
}

static KMETHOD Statement_Shell(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kTokenArray *tokenList = (kTokenArray *)kStmt_getObjectNULL(kctx, stmt, KW_TokenPattern);
	if(tokenList != NULL) {
		kString *cmd = NULL;
		if(IS_Token(tokenList)) {
			cmd = ((kToken*)tokenList)->text;
		}
		else {
			DBG_ASSERT(IS_Array(tokenList));
			cmd = splitWhiteSpace(kctx, tokenList);  // forget GC
			PUSH_GCSTACK(cmd);
		}
		DBG_P("cmd=%s", S_text(cmd));
		system(S_text(cmd));  // FIXME: This is for demo
		kStmt_done(kctx, stmt);
	}
	RETURNb_(false);
}

// ----------------------------------------------------------------------------
/* define class */

static kbool_t shell_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	//KImportPackage(ns, "dscript.dollar", pline);
	KImportPackage(ns, "dscript.subproc", pline);
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("dsh"), 0, "\"dsh\" $Token*", 0, 0, NULL, NULL, Statement_dsh, Statement_dsh, NULL, },
		{ SYM_("$Shell"), 0, "$Shell $Token*", 0, 0, PatternMatch_Shell, NULL, Statement_Shell, Statement_Shell},
		{ KW_END, },
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
	static KDEFINE_PACKAGE d = {0};
	KSETPACKNAME(d, "dscript.shell", "1.0");
	d.initPackage    = shell_initPackage;
	d.setupPackage   = shell_setupPackage;
	d.initNameSpace  = shell_initNameSpace;
	d.setupNameSpace = shell_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
