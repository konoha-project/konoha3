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

// --------------------------------------------------------------------------

static KMETHOD Statement_namespace(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kstatus_t result = K_CONTINUE;
	kToken *tk = SUGAR kStmt_getToken(kctx, stmt, KW_BlockPattern, NULL);
	if(tk != NULL && tk->resolvedSyntaxInfo->keyword == TokenType_CODE) {
		INIT_GCSTACK();
		kNameSpace *ns = new_(NameSpace, Stmt_ns(stmt), _GcStack);
		kArray *a = GetSugarContext(kctx)->preparedTokenList;
		TokenSeq range = {ns, a, kArray_size(a), kArray_size(a)};
		SUGAR TokenSeq_tokenize(kctx, &range, S_text(tk->text), tk->uline);
		result = SUGAR TokenSeq_eval(kctx, &range, NULL/*trace*/);
		RESET_GCSTACK();
		kStmt_done(kctx, stmt);
	}
	KReturnUnboxValue(result == K_CONTINUE);
}

static kbool_t namespace_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("namespace"), 0, "\"namespace\" $Block", 0, 0, NULL, NULL, Statement_namespace, NULL, NULL, },
		{ KW_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}

// --------------------------------------------------------------------------





// --------------------------------------------------------------------------

static kbool_t namespace_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	namespace_defineSyntax(kctx, ns, trace);
	return true;
}

static kbool_t namespace_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* namespace_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", "1.0");
	d.PackupNameSpace    = namespace_PackupNameSpace;
	d.ExportNameSpace   = namespace_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
