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

static const char *filename2;
static FILE *stdlog2 = NULL;

static const char *GetProfile()
{
	char *p = getenv("KONOHA_TESTPROFILE");
	return (p == NULL)? "minimum" /* for compatibility */: (const char*)p;
}

static FILE *GetLogFile(void)
{
	if(stdlog2 == NULL) {
		char result_file[256];
		snprintf(result_file, sizeof(result_file), "%s.%s_tested", filename2, GetProfile());
		stdlog2 = fopen(result_file, "w");
		if(stdlog2 == NULL) {
			fprintf(stdout, "cannot open logfile: %s\n", result_file);
			exit(1);
		}
	}
	return stdlog2;
}

static void BEFORE_LoadScript(KonohaContext *kctx, const char *filename)
{
	filename2 = filename;
}

static int check_result2(FILE *fp0, FILE *fp1)
{
	char buf0[4096];
	char buf1[4096];
	while (fgets(buf0, sizeof(buf0), fp0) != NULL) {
		char *p = fgets(buf1, sizeof(buf1), fp1);
		if(p == NULL) return 1;//FAILED
		if(strcmp(buf0, buf1) != 0) {
			return 1;//FAILED
		}
	}
	return 0; //OK
}

//static int check_result2(FILE *fp0, FILE *fp1)
//{
//	char buf0[128];
//	char buf1[128];
//	while (true) {
//		size_t len0, len1;
//		len0 = fread(buf0, 1, sizeof(buf0), fp0);
//		len1 = fread(buf1, 1, sizeof(buf1), fp1);
//		if(len0 != len1) {
//			return 1;//FAILED
//		}
//		if(len0 == 0) {
//			break;
//		}
//		if(memcmp(buf0, buf1, len0) != 0) {
//			return 1;//FAILED
//		}
//	}
//	return 0; //OK
//}

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
		char proof_file[256];
		char result_file[256];
		PLATAPI snprintf_i(proof_file, sizeof(proof_file), "%s.%s_proof",  filename, GetProfile());
		PLATAPI snprintf_i(result_file, sizeof(result_file),  "%s.%s_tested", filename, GetProfile());
		FILE *fp = fopen(proof_file, "r");
		if(fp == NULL) {
			fprintf(stdout, "no proof file: %s\n", proof_file);
			((KonohaFactory*)kctx->platApi)->exitStatus = 1;
			return;
		}
		FILE *fp2 = fopen(result_file, "r");
		int ret = check_result2(fp, fp2);
		if(ret != 0) {
			fprintf(stdout, "stdlog_count=%d, exitStatus=%d\n", stdlog_count, kctx->platApi->exitStatus);
			fprintf(stdout, "proof file mismatched: %s\n", proof_file);
			((KonohaFactory*)kctx->platApi)->exitStatus = 78;
			fprintf(stdout, "stdlog_count=%d, exitStatus=%d\n", stdlog_count, kctx->platApi->exitStatus);
		}
		else {
			((KonohaFactory*)kctx->platApi)->exitStatus = 0;
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
}

static void TEST_ReportCompilerMessage(KonohaContext *kctx, kinfotag_t taglevel, kfileline_t pline, const char *msg)
{
	PLATAPI printf_i("LINE%d: %s\n", (int)(kushort_t)pline, TAG_t(taglevel));
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
	factory->printf_i               = TEST_printf;
	factory->vprintf_i              = TEST_vprintf;

	factory->ReportUserMessage           = TEST_ReportUserMessage;
	factory->ReportCompilerMessage       = TEST_ReportCompilerMessage;
	factory->ReportCaughtException       = TEST_reportCaughtException;
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

