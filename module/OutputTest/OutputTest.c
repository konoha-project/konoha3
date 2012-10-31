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
#include <errno.h>
#include <minikonoha/minikonoha.h>
#include <minikonoha/klib.h>

// -------------------------------------------------------------------------
// JenkinsTest

static FILE *stdlog;
static int   stdlog_count = 0;

static void BEFORE_LoadScript(KonohaContext *kctx, const char *filename)
{
	char result_file[256];
	PLATAPI snprintf_i(result_file, sizeof(result_file),  "%s.tested", filename);
	stdlog = fopen(result_file, "w");
}

static int check_result2(FILE *fp0, FILE *fp1)
{
	char buf0[4096];
	char buf1[4096];
	while (fgets(buf0, sizeof(buf0), fp0) != NULL) {
		char *p = fgets(buf1, sizeof(buf1), fp1);
		if(p == NULL) return 1;//FAILED
		if((p = strstr(buf0, "(error) (")) != NULL) {
			p = strstr(p+8, ")");
			if(strncmp(buf0, buf1, p - buf1 + 1) != 0) return 1; //FAILED;
			continue;
		}
		if((p = strstr(buf0, "(warning) (")) != NULL) {
			p = strstr(p+10, ")");
			if(strncmp(buf0, buf1, p - buf1 + 1) != 0) return 1; //FAILED;
			continue;
		}
		if(strcmp(buf0, buf1) != 0) {
			return 1;//FAILED
		}
	}
	return 0; //OK
}

static void AFTER_LoadScript(KonohaContext *kctx, const char *filename)
{
	fprintf(stdlog, "Q.E.D.\n");   // Q.E.D.
	fclose(stdlog);
	//
	char proof_file[256];
	char result_file[256];
	PLATAPI snprintf_i(proof_file, sizeof(proof_file), "%s.proof",  filename);
	PLATAPI snprintf_i(result_file, sizeof(result_file),  "%s.tested", filename);

	FILE *fp = fopen(proof_file, "r");
	int isPassed = false;
	if(fp != NULL) {
		FILE *fp2 = fopen(result_file, "r");
		int ret = check_result2(fp, fp2);
		if(ret == 0) {
			fprintf(stdout, "[PASS]: %s\n", filename);
			isPassed = true;
		}
		else {
			fprintf(stdout, "[FAIL]: %s\n", filename);
			((KonohaFactory*)kctx->platApi)->detectedAssertionFailure = true;
		}
		fclose(fp2);
	}
	else {
		if(stdlog_count == 0) {
			if(PLATAPI detectedAssertionFailure == 0) {
				fprintf(stdout, "[PASS]: %s\n", filename);
				isPassed = true;
			}
		}
		else {
			fprintf(stdout, "no proof file: %s\n", filename);
			fprintf(stdout, "[FAIL]: %s\n", filename);
		}
	}
	fclose(fp);
	if(!isPassed) {
		((KonohaFactory*)kctx->platApi)->detectedAssertionFailure = true;
	}
}

static void NOP_ReportDebugMessage(const char *file, const char *func, int line, const char *fmt, ...)
{

}

static const char* TEST_begin(kinfotag_t t)
{
	return "";
}

static const char* TEST_end(kinfotag_t t)
{
	return "";
}

static const char* TEST_shortText(const char *msg)
{
	return "(omitted..)";
}

static int TEST_vprintf(const char *fmt, va_list ap)
{
	stdlog_count++;
	return vfprintf(stdlog, fmt, ap);
}

static int TEST_printf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	stdlog_count++;
	int res = vfprintf(stdlog, fmt, ap);
	va_end(ap);
	return res;
}

static void TEST_ReportUserMessage(KonohaContext *kctx, kinfotag_t level, kfileline_t pline, const char *msg, int isNewLine)
{
	const char *kLF = isNewLine ? "\n" : "";
	PLATAPI printf_i("LINE%d: '%s'%s" ,(int)(kushort_t)pline, msg, kLF);
}

static void TEST_ReportCompilerMessage(KonohaContext *kctx, kinfotag_t taglevel, kfileline_t pline, const char *msg)
{
	PLATAPI printf_i("LINE%d: %s", (int)(kushort_t)pline, TAG_t(taglevel));
}

static void TEST_reportCaughtException(KonohaContext *kctx, const char *exceptionName, int fault, const char *optionalMessage, KonohaStack *bottom, KonohaStack *sfp)
{
	int line = (sfp != NULL) ? (kushort_t)sfp[K_RTNIDX].callerFileLine : 0;
	PLATAPI printf_i("LINE%d: %s\n", line, exceptionName);
}

// -------------------------------------------------------------------------

kbool_t LoadOutputTestModule(KonohaFactory *factory, ModuleType type)
{
	factory->BEFORE_LoadScript = BEFORE_LoadScript;
	factory->AFTER_LoadScript  = AFTER_LoadScript;

	factory->ReportDebugMessage     = NOP_ReportDebugMessage;
	factory->printf_i        = TEST_printf;
	factory->vprintf_i       = TEST_vprintf;
	factory->beginTag        = TEST_begin;
	factory->endTag          = TEST_end;
	factory->shortText       = TEST_shortText;

	factory->ReportUserMessage     = TEST_ReportUserMessage;
	factory->ReportCompilerMessage = TEST_ReportCompilerMessage;
	factory->ReportCaughtException       = TEST_reportCaughtException;
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

