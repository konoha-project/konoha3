/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
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

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>

#ifdef __cplusplus
extern "C"{
#endif

// --------------------------------------------------------------------------
// duplicated from sugar.c

static kstatus_t kNameSpace_Eval(KonohaContext *kctx, kNameSpace *ns, const char *script, kfileline_t uline, KTraceInfo *trace)
{
	kstatus_t result;
	KPARSERM->h.setupModuleContext(kctx, (KRuntimeModule *)KPARSERM, 0/*lazy*/);
	INIT_GCSTACK();
	{
		KTokenSeq tokens = {ns, KGetParserContext(kctx)->preparedTokenList};
		KTokenSeq_Push(kctx, tokens);
		SUGAR Tokenize(kctx, ns, script, uline, 0, tokens.tokenList);
		KTokenSeq_End(kctx, tokens);
		result = SUGAR EvalTokenList(kctx, &tokens, trace);
		KTokenSeq_Pop(kctx, tokens);
	}
	RESET_GCSTACK();
	return result;
}

// --------------------------------------------------------------------------

//## boolean NameSpace.eval(String command);
static KMETHOD NameSpace_Eval(KonohaContext *kctx, KonohaStack *sfp)
{
	kNameSpace *ns = sfp[0].asNameSpace;
	const char *script = kString_text(sfp[1].asString);
	DBG_ASSERT(IS_NameSpace(ns));
	kfileline_t uline = FILEID_("(eval)") | 1;
	KMakeTrace(trace, sfp);
	KReturnUnboxValue(kNameSpace_Eval(kctx, ns, script, uline, trace) == K_CONTINUE);
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _F(F)   (intptr_t)(F)

static void eval_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(NameSpace_Eval), KType_Boolean, KType_NameSpace, KMethodName_("eval"), 1, KType_String, KFieldName_("command"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

// --------------------------------------------------------------------------

static kbool_t eval_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	eval_defineMethod(kctx, ns, trace);
	return true;
}

static kbool_t eval_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE *Eval_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", K_VERSION);
	d.PackupNameSpace    = eval_PackupNameSpace;
	d.ExportNameSpace   = eval_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
