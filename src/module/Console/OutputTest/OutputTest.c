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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <errno.h>
#include <konoha3/konoha.h>
#include <konoha3/klib.h>

// -------------------------------------------------------------------------
// JenkinsTest

static const char *filename2 = NULL;
static FILE *stdlog2 = NULL;

static const char *GetProfile()
{
	char *p = getenv("KONOHA_TESTPROFILE");
	return (p == NULL)? "minimum" /* for compatibility */: (const char *)p;
}

static FILE *GetLogFile(void)
{
	if(stdlog2 == NULL) {
		if(filename2 == NULL) return stdout;
		char result_file[256];
		snprintf(result_file, sizeof(result_file), "%s.%s_tested", filename2, GetProfile());
		stdlog2 = fopen(result_file, "w");
		if(stdlog2 == NULL) {
			fprintf(stdout, "cannot open logfile: %s (%s)\n", result_file, strerror(errno));
			exit(1);
		}
	}
	return stdlog2;
}

static void BEFORE_LoadScript(KonohaContext *kctx, const char *filename)
{
	//filename = (filename == NULL) ? "shell" : filename;
	filename2 = filename;
}

static int check_result2(FILE *fp0, FILE *fp1)
{
	char buf0[4096];
	char buf1[4096];
	while(fgets(buf0, sizeof(buf0), fp0) != NULL) {
		char *p = fgets(buf1, sizeof(buf1), fp1);
		if(p == NULL) return 1;//FAILED
		if(strcmp(buf0, buf1) != 0) {
			return 1;//FAILED
		}
	}
	return 0; //OK
}

static void AFTER_LoadScript(KonohaContext *kctx, const char *filename)
{
	int stdlog_count = 0;
	if(stdlog2 != NULL) {
		stdlog_count = 1;
		fprintf(stdlog2, "Q.E.D.\n");   // Q.E.D.
		fclose(stdlog2);
	}
	//if(PLATAPI exitStatus != 0) return;
	if(stdlog_count != 0) {
		//filename = (filename == NULL) ? "shell" : filename;
		char proof_file[256];
		char result_file[256];
		PLATAPI snprintf_i(proof_file, sizeof(proof_file), "%s.%s_proof",  filename, GetProfile());
		PLATAPI snprintf_i(result_file, sizeof(result_file),  "%s.%s_tested", filename, GetProfile());
		FILE *fp = fopen(proof_file, "r");
		if(fp == NULL) {
			fprintf(stdout, "no proof file: %s\n", proof_file);
			((KonohaFactory *)kctx->platApi)->exitStatus = 1;
			return;
		}
		FILE *fp2 = fopen(result_file, "r");
		DBG_ASSERT(fp2 != NULL);
		int ret = check_result2(fp, fp2);
		if(ret != 0) {
			fprintf(stdout, "proof file mismatched: %s\n", proof_file);
			((KonohaFactory *)kctx->platApi)->exitStatus = 78;
		}
		else {
			((KonohaFactory *)kctx->platApi)->exitStatus = 0;
		}
		fclose(fp);
		fclose(fp2);
	}
}

static void NOP_ReportDebugMessage(const char *file, const char *func, int line, const char *fmt, ...)
{

}

static int TEST_vprintf(const char *fmt, va_list ap)
{
	return vfprintf(GetLogFile(), fmt, ap);
}

static int TEST_printf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int res = vfprintf(GetLogFile(), fmt, ap);
	va_end(ap);
	return res;
}

static void TEST_ReportUserMessage(KonohaContext *kctx, kinfotag_t level, kfileline_t pline, const char *msg, int isNewLine)
{
	const char *kLF = isNewLine ? "\n" : "";
	PLATAPI printf_i("LINE%d: '%s'%s" ,(int)(kushort_t)pline, msg, kLF);
	fprintf(stdout, "LINE %d: '%s'%s", (int)(kushort_t)pline, msg, kLF);
}

static void TEST_ReportCompilerMessage(KonohaContext *kctx, kinfotag_t taglevel, kfileline_t pline, const char *msg)
{
	if(taglevel < DebugTag) {
		PLATAPI printf_i("LINE%d: %s\n", (int)(kushort_t)pline, TAG_t(taglevel));
		fprintf(stdout, "LINE %d: %s\n", (int)(kushort_t)pline, msg);
	}
}

static void TEST_reportCaughtException(KonohaContext *kctx, kException *e, struct KonohaValueVar *bottomStack, struct KonohaValueVar *topStack)
{
	int line = (topStack != NULL) ? (kushort_t)topStack[K_RTNIDX].calledFileLine : 0;
	const char *exceptionName = KSymbol_text(e->symbol);
	PLATAPI printf_i("LINE%d: %s\n", line, exceptionName);
	fprintf(stdout, "LINE %d: %s %s\n", line, exceptionName, kString_text(e->Message));
}


static char* TEST_InputUserText(KonohaContext *kctx, const char *message, int flag)
{
	return NULL;
}

static int TEST_InputUserApproval(KonohaContext *kctx, const char *message, const char *yes, const char *no, int defval)
{
	return defval;
}

static char* TEST_InputUserPassword(KonohaContext *kctx, const char *message)
{
	return NULL;
}

// -------------------------------------------------------------------------

kbool_t LoadOutputTestModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"OutputTest", "0.1", 0, "test",
	};
	factory->BEFORE_LoadScript = BEFORE_LoadScript;
	factory->AFTER_LoadScript  = AFTER_LoadScript;

	factory->printf_i               = TEST_printf;
	factory->vprintf_i              = TEST_vprintf;

	factory->ConsoleModule.ConsoleInfo     = &ModuleInfo;
	factory->ConsoleModule.ReportDebugMessage     = NOP_ReportDebugMessage;
	factory->ConsoleModule.ReportUserMessage      = TEST_ReportUserMessage;
	factory->ConsoleModule.ReportCompilerMessage  = TEST_ReportCompilerMessage;
	factory->ConsoleModule.ReportCaughtException  = TEST_reportCaughtException;
	factory->ConsoleModule.InputUserApproval        = TEST_InputUserApproval;
	factory->ConsoleModule.InputUserText            = TEST_InputUserText;
	factory->ConsoleModule.InputUserPassword        = TEST_InputUserPassword;

	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
