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
#include <konoha3/konoha_common.h>
#include <konoha3/import/methoddecl.h>

typedef struct kConsoleVar kConsole;
struct kConsoleVar {
	kObjectHeader h;
};

#define CONSOLEAPI PLATAPI ConsoleModule.

/* ------------------------------------------------------------------------ */

//## void Console.notify(String msg)
static KMETHOD Console_notify(KonohaContext *kctx, KonohaStack *sfp)
{
	CONSOLEAPI ReportUserMessage(kctx, NoticeTag, (sfp[K_RTNIDX].calledFileLine), kString_text(sfp[1].asString), true);
}

//## void Console.readLine(String prompt)
static KMETHOD Console_readLine(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s;
	char *p = CONSOLEAPI InputUserText(kctx, kString_text(sfp[1].asString), 0);
	if(p != NULL) {
		s = KLIB new_kString(kctx, OnStack, p, strlen(p), 0);
		free(p);
	}
	else {
		s = KNULL(String);
	}
	KReturn(s);
}

//## boolean Console.inputUserApproval(String msg, String yes, String no, kbool_t defval)
static KMETHOD Console_inputUserApproval(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(CONSOLEAPI InputUserApproval(kctx, kString_text(sfp[1].asString), kString_text(sfp[2].asString), kString_text(sfp[3].asString), sfp[4].boolValue));
}

static kString* kConsole_inputUserPassword(KonohaContext *kctx, const char *message)
{
	kString *s;
	char *p = CONSOLEAPI InputUserPassword(kctx, message);
	if(p != NULL) {
		s = KLIB new_kString(kctx, OnStack, p, strlen(p), 0);
		free(p);
	}
	else {
		s = KNULL(String);
	}
	return s;
}

//## boolean Console.inputUserPassword(String msg)
static KMETHOD Console_inputUserPassword(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(kConsole_inputUserPassword(kctx, kString_text(sfp[1].asString)));
}

//## boolean Console.inputUserPassword(String msg)
static KMETHOD Console_inputUserPassword0(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(kConsole_inputUserPassword(kctx, "Password:"));
}

/* ------------------------------------------------------------------------ */

static kbool_t console_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KDEFINE_CLASS defConsole = {0};
	SETSTRUCTNAME(defConsole, Console);
	defConsole.cflag = KClassFlag_Final | KClassFlag_Singleton;

	KClass *cConsole = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defConsole, trace);
	int KType_Console = cConsole->typeId;

	KDEFINE_METHOD MethodData[] = {
		_Public|_Static, _F(Console_notify), KType_void, KType_Console, KMethodName_("notify"), 1, KType_String, KFieldName_("message"),
		_Public|_Static, _F(Console_readLine), KType_String, KType_Console, KMethodName_("readLine"), 1, KType_String, KFieldName_("message"),
		_Public|_Static, _F(Console_inputUserApproval), KType_Boolean, KType_Console, KMethodName_("inputUserApproval"), 4,
		    KType_String, KFieldName_("message"), KType_String, KFieldName_("yes"), KType_String, KFieldName_("no"), KType_Boolean, KFieldName_("defval"),
		_Public|_Static, _F(Console_inputUserPassword), KType_String, KType_Console, KMethodName_("inputUserPassword"), 1, KType_String, KFieldName_("message"),
		_Public|_Static, _F(Console_inputUserPassword0), KType_String, KType_Console, KMethodName_("inputUserPassword"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	return true;
}

static kbool_t console_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *Console_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", "1.0");
	d.PackupNameSpace    = console_PackupNameSpace;
	d.ExportNameSpace   = console_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
