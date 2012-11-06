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
// duplicated from sugar.c

static kstatus_t kNameSpace_Eval(KonohaContext *kctx, kNameSpace *ns, const char *script, kfileline_t uline)
{
	kstatus_t result;
	kmodsugar->h.setupModuleContext(kctx, (KonohaModule *)kmodsugar, 0/*lazy*/);
	INIT_GCSTACK();
	{
		TokenSequence tokens = {ns, KonohaContext_getSugarContext(kctx)->preparedTokenList};
		TokenSequence_push(kctx, tokens);
		SUGAR TokenSequence_tokenize(kctx, &tokens, script, uline);
		result = SUGAR TokenSequence_eval(kctx, &tokens);
		TokenSequence_pop(kctx, tokens);
	}
	RESET_GCSTACK();
	return result;
}

// --------------------------------------------------------------------------

//## boolean System.eval(String command);
static KMETHOD System_eval(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *script = S_text(sfp[1].asString);
	kNameSpace *ns = KGetLexicalNameSpace(sfp);
	DBG_ASSERT(IS_NameSpace(ns));
	kfileline_t uline = FILEID_("(eval)") | 1;
	KReturnUnboxValue(kNameSpace_Eval(kctx, ns, script, uline) == K_CONTINUE);
}


// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Static   kMethod_Static
#define _Virtual  kMethod_Virtual
#define _F(F)   (intptr_t)(F)

static void eval_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Static|_Virtual, _F(System_eval), TY_boolean, TY_System, MN_("eval"), 1, TY_String, FN_("command"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

// --------------------------------------------------------------------------

static kbool_t eval_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	eval_defineMethod(kctx, ns, trace);
	return true;
}

static kbool_t eval_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* eval_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "eval", "1.0");
	d.initPackage    = eval_initPackage;
	d.setupPackage   = eval_setupPackage;
	return &d;
}

#ifdef __cplusplus
}
#endif
