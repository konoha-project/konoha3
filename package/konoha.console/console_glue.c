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
//#include <minikonoha/sugar.h>
#include <minikonoha/konoha_common.h>

typedef struct kConsoleVar kConsole;
struct kConsoleVar {
	KonohaObjectHeader h;
};

/* ------------------------------------------------------------------------ */

//## boolean Console.inputUserApproval(String msg, String yes, String no, kbool_t defval)

static KMETHOD Console_inputUserApproval(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(PLATAPI InputUserApproval(kctx, S_text(sfp[1].asString), S_text(sfp[2].asString), S_text(sfp[3].asString), sfp[4].boolValue));
}

/* ------------------------------------------------------------------------ */

#define _Public   kMethod_Public
#define _Static   kMethod_Static
#define _F(F)   (intptr_t)(F)
#define TY_Date     cDate->typeId

static kbool_t console_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	KDEFINE_CLASS defConsole = {0};
	SETSTRUCTNAME(defConsole, Console);
	defConsole.cflag = kClass_Final | kClass_Singleton;

	KonohaClass *cConsole = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defConsole, trace);
	int TY_Console = cConsole->typeId;

	KDEFINE_METHOD MethodData[] = {
		_Public|_Static, _F(Console_inputUserApproval), TY_boolean, TY_Console, MN_("inputUserApproval"), 4,
		    TY_String, FN_("message"), TY_String, FN_("yes"), TY_String, FN_("no"), TY_boolean, FN_("defval"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	return true;
}

static kbool_t console_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* console_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", "1.0");
	d.initPackage    = console_initPackage;
	d.setupPackage   = console_setupPackage;
	return &d;
}

#ifdef __cplusplus
}
#endif
