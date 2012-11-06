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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sys/ioctl.h>
//#include <asm/termbits.h>
#include <minikonoha/minikonoha.h>
#include <minikonoha/klib.h>

// -------------------------------------------------------------------------
/* Console */

static void UI_ReportUserMessage(KonohaContext *kctx, kinfotag_t level, kfileline_t pline, const char *msg, int isNewLine)
{

}

static void UI_ReportCompilerMessage(KonohaContext *kctx, kinfotag_t taglevel, kfileline_t pline, const char *msg)
{
}

static void Kwb_writeValue(KonohaContext *kctx, KGrowingBuffer *wb, KonohaClass *c, KonohaStack *sfp)
{

}

static void UI_ReportCaughtException(KonohaContext *kctx, const char *exceptionName, int fault, const char *optionalMessage, KonohaStack *bottomStack, KonohaStack *topStack)
{

}

static void ReportDebugMessage(const char *file, const char *func, int line, const char *fmt, ...)
{

}


static int InputUserApproval(KonohaContext *kctx, const char *message, const char *yes, const char *no, int defval)
{

}

static const char* InputUserText(KonohaContext *kctx, const char *message, int flag)
{

}

static const char* InputUserPassword(KonohaContext *kctx, const char *message)
{

	return "";
}

// -------------------------------------------------------------------------

kbool_t LoadDScriptConsoleModule(KonohaFactory *factory, ModuleType type)
{

	factory->ReportUserMessage        = UI_ReportUserMessage;
	factory->ReportCompilerMessage    = UI_ReportCompilerMessage;
	factory->ReportCaughtException    = UI_ReportCaughtException;
	factory->ReportDebugMessage       = ReportDebugMessage;
	factory->InputUserApproval        = InputUserApproval;

	factory->InputUserText            = InputUserText;
	factory->InputUserPassword        = InputUserPassword;
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

