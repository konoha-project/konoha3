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
#include "minikonoha/gc.h"
#include <minikonoha/klib.h>

#define USE_BUILTINTEST 1
#include "testkonoha.h"
#include <getopt.h>

#ifdef __cplusplus
extern "C" {
#endif

kstatus_t MODSUGAR_eval(KonohaContext *kctx, const char *script, size_t len, kfileline_t uline);
kstatus_t MODSUGAR_loadScript(KonohaContext *kctx, const char *path, size_t len, kfileline_t pline);

// -------------------------------------------------------------------------
// getopt

static int compileonly_flag = 0;
static int interactive_flag = 0;

extern int verbose_debug;
extern int verbose_code;
extern int verbose_sugar;
extern int verbose_gc;

#include <minikonoha/platform.h>
#include <minikonoha/libcode/minishell.h>

// -------------------------------------------------------------------------
// KonohaContext*est

static FILE *stdlog;
static int   stdlog_count = 0;

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
	int res = vfprintf(stdlog, fmt, ap);
	va_end(ap);
	return res;
}

static void TEST_reportCaughtException(const char *exceptionName, const char *scriptName, int line, const char *optionalMessage)
{
	if(line != 0) {
		fprintf(stdlog, " ** %s (%s:%d)\n", exceptionName, scriptName, line);
	}
	else {
		fprintf(stdlog, " ** %s\n", exceptionName);
	}
}

//static int check_result2(FILE *fp0, FILE *fp1)
//{
//	char buf0[128];
//	char buf1[128];
//	while (true) {
//		size_t len0, len1;
//		len0 = fread(buf0, 1, sizeof(buf0), fp0);
//		len1 = fread(buf1, 1, sizeof(buf1), fp1);
//		if (len0 != len1) {
//			return 1;//FAILED
//		}
//		if (len0 == 0) {
//			break;
//		}
//		if (memcmp(buf0, buf1, len0) != 0) {
//			return 1;//FAILED
//		}
//	}
//	return 0; //OK
//}

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
		if (strcmp(buf0, buf1) != 0) {
			return 1;//FAILED
		}
	}
	return 0; //OK
}

static void make_report(const char *testname)
{
	char *path = getenv("KONOHA_REPORT");
	if(path != NULL) {
		char report_file[256];
		char script_file[256];
		char correct_file[256];
		char result_file[256];
		snprintf(report_file, 256,  "%s/REPORT_%s.txt", path, shortFilePath(testname));
		snprintf(script_file, 256,  "%s", testname);
		snprintf(correct_file, 256, "%s.proof", script_file);
		snprintf(result_file, 256,  "%s.tested", script_file);
		FILE *fp = fopen(report_file, "w");
		FILE *fp2 = fopen(script_file, "r");
		int ch;
		while((ch = fgetc(fp2)) != EOF) {
			fputc(ch, fp);
		}
		fclose(fp2);
		fprintf(fp, "Expected Result (in %s)\n=====\n", result_file);
		fp2 = fopen(correct_file, "r");
		while((ch = fgetc(fp2)) != EOF) {
			fputc(ch, fp);
		}
		fclose(fp2);
		fprintf(fp, "Result (in %s)\n=====\n", result_file);
		fp2 = fopen(result_file, "r");
		while((ch = fgetc(fp2)) != EOF) {
			fputc(ch, fp);
		}
		fclose(fp2);
		fclose(fp);
	}
}

extern int konoha_detectFailedAssert;

static int KonohaContext_test(KonohaContext *kctx, const char *testname)
{
	int ret = 1; //FAILED
	char script_file[256];
	char correct_file[256];
	char result_file[256];
	PLATAPI snprintf_i(script_file, 256,  "%s", testname);
	PLATAPI snprintf_i(correct_file, 256, "%s.proof", script_file);
	PLATAPI snprintf_i(result_file, 256,  "%s.tested", script_file);
	FILE *fp = fopen(correct_file, "r");
	stdlog = fopen(result_file, "w");
	konoha_load((KonohaContext*)kctx, script_file);
	fprintf(stdlog, "Q.E.D.\n");   // Q.E.D.
	fclose(stdlog);

	if(fp != NULL) {
		FILE *fp2 = fopen(result_file, "r");
		ret = check_result2(fp, fp2);
		if(ret == 0) {
			fprintf(stdout, "[PASS]: %s\n", testname);
		}
		else {
			fprintf(stdout, "[FAIL]: %s\n", testname);
			make_report(testname);
			konoha_detectFailedAssert = 1;
		}
		fclose(fp);
		fclose(fp2);
	}
	else {
		//fprintf(stdout, "stdlog_count: %d\n", stdlog_count);
		if(stdlog_count == 0) {
			if(konoha_detectFailedAssert == 0) {
				fprintf(stdout, "[PASS]: %s\n", testname);
				return 0; // OK
			}
		}
		else {
			fprintf(stdout, "no proof file: %s\n", testname);
			konoha_detectFailedAssert = 1;
		}
		fprintf(stdout, "[FAIL]: %s\n", testname);
		return 1;
	}
	return ret;
}

#ifdef USE_BUILTINTEST
extern DEFINE_TESTFUNC KonohaTestSet[];
static BuiltInTestFunc lookupTestFunc(DEFINE_TESTFUNC *d, const char *name)
{
	while(d->name != NULL) {
		if(strcasecmp(name, d->name) == 0) {
			return d->f;
		}
		d++;
	}
	return NULL;
}
#endif

static int CommandLine_doBuiltInTest(KonohaContext* konoha, const char* name)
{
#ifdef USE_BUILTINTEST
	BuiltInTestFunc f = lookupTestFunc(KonohaTestSet, name);
	if(f != NULL) {
		return f(konoha);
	}
	fprintf(stderr, "Built-in test is not found: '%s'\n", name);
#else
	fprintf(stderr, "Built-in tests are not built; rebuild with -DUSE_BUILTINTEST\n");
#endif
	return 1;
}

static void CommandLine_define(KonohaContext *kctx, char *keyvalue)
{
	char *p = strchr(keyvalue, '=');
	if(p != NULL) {
		size_t len = p-keyvalue;
		char namebuf[len+1];
		memcpy(namebuf, keyvalue, len); namebuf[len] = 0;
		DBG_P("name='%s'", namebuf);
		ksymbol_t key = KLIB Ksymbol(kctx, namebuf, len, 0, SYM_NEWID);
		uintptr_t unboxValue;
		ktype_t ty;
		if(isdigit(p[1])) {
			ty = TY_int;
			unboxValue = (uintptr_t)strtol(p+1, NULL, 0);
		}
		else {
			ty = TY_TEXT;
			unboxValue = (uintptr_t)(p+1);
		}
		if(!KLIB kNameSpace_setConstData(kctx, KNULL(NameSpace), key, ty, unboxValue, 0)) {
			PLATAPI exit_i(EXIT_FAILURE);
		}
	}
	else {
		fprintf(stdout, "invalid define option: use -D<key>=<value>\n");
		PLATAPI exit_i(EXIT_FAILURE);
	}
}

static void CommandLine_import(KonohaContext *kctx, char *packageName)
{
	size_t len = strlen(packageName)+1;
	char bufname[len];
	memcpy(bufname, packageName, len);
	if(!(KLIB kNameSpace_importPackage(kctx, KNULL(NameSpace), bufname, 0))) {
		PLATAPI exit_i(EXIT_FAILURE);
	}
}

static void konoha_startup(KonohaContext *kctx, const char *startup_script)
{
	char buf[256];
	const char *path = PLATAPI getenv_i("KONOHA_SCRIPTPATH"), *local = "";
	if(path == NULL) {
		path = PLATAPI getenv_i("KONOHA_HOME");
		local = "/script";
	}
	if(path == NULL) {
		path = PLATAPI getenv_i("HOME");
		local = "/.minikonoha/script";
	}
	snprintf(buf, sizeof(buf), "%s%s/%s.k", path, local, startup_script);
	if(!konoha_load((KonohaContext*)kctx, (const char*)buf)) {
		PLATAPI exit_i(EXIT_FAILURE);
	}
}

static void CommandLine_setARGV(KonohaContext *kctx, int argc, char** argv)
{
	KonohaClass *CT_StringArray0 = CT_p0(kctx, CT_Array, TY_String);
	kArray *a = (kArray*)KLIB new_kObject(kctx, CT_StringArray0, 0);
	int i;
	for(i = 0; i < argc; i++) {
		DBG_P("argv=%d, '%s'", i, argv[i]);
		KLIB kArray_add(kctx, a, KLIB new_kString(kctx, argv[i], strlen(argv[i]), SPOL_TEXT));
	}
	KDEFINE_OBJECT_CONST ObjectData[] = {
			{"SCRIPT_ARGV", CT_StringArray0->typeId, (kObject*)a},
			{}
	};
	KLIB kNameSpace_loadConstData(kctx, KNULL(NameSpace), KonohaConst_(ObjectData), 0);
}

static struct option long_options2[] = {
	/* These options set a flag. */
	{"verbose", no_argument,       &verbose_debug, 1},
	{"verbose:gc",    no_argument, &verbose_gc, 1},
	{"verbose:sugar", no_argument, &verbose_sugar, 1},
	{"verbose:code",  no_argument, &verbose_code, 1},
	{"interactive", no_argument,   0, 'i'},
	{"typecheck",   no_argument,   0, 'c'},
	{"define",    required_argument, 0, 'D'},
	{"import",    required_argument, 0, 'I'},
	{"startwith", required_argument, 0, 'S'},
	{"test",  required_argument, 0, 'T'},
	{"test-with",  required_argument, 0, 'T'},
	{"builtin-test",  required_argument, 0, 'B'},
	{NULL, 0, 0, 0},
};

static int konoha_parseopt(KonohaContext* konoha, PlatformApiVar *plat, int argc, char **argv)
{
	kbool_t ret = true;
	int scriptidx = 0;
	while (1) {
		int option_index = 0;
		int c = getopt_long (argc, argv, "icD:I:S:", long_options2, &option_index);
		if (c == -1) break; /* Detect the end of the options. */
		switch (c) {
		case 0:
			/* If this option set a flag, do nothing else now. */
			if (long_options2[option_index].flag != 0)
				break;
			printf ("option %s", long_options2[option_index].name);
			if (optarg)
				printf (" with arg %s", optarg);
			printf ("\n");
			break;

		case 'c': {
			compileonly_flag = 1;
			KonohaContext_setCompileOnly(konoha);
		}
		break;

		case 'i': {
			interactive_flag = 1;
			KonohaContext_setInteractive(konoha);
		}
		break;

		case 'B':
			return CommandLine_doBuiltInTest(konoha, optarg);

		case 'D':
			CommandLine_define(konoha, optarg);
			break;

		case 'I':
			CommandLine_import(konoha, optarg);
			break;

		case 'S':
			konoha_startup(konoha, optarg);
			break;

		case 'T':
//			DUMP_P ("option --test-with `%s'\n", optarg);
			verbose_debug = 0;
			verbose_sugar = 0;
			verbose_gc    = 0;
			verbose_code  = 0;
			plat->debugPrintf = NOP_debugPrintf;
			plat->printf_i  = TEST_printf;
			plat->vprintf_i = TEST_vprintf;
			plat->beginTag  = TEST_begin;
			plat->endTag    = TEST_end;
			plat->shortText = TEST_shortText;
			plat->reportCaughtException = TEST_reportCaughtException;
			return KonohaContext_test(konoha, optarg);

		case '?':
			/* getopt_long already printed an error message. */
			break;

		default:
			return 1;
		}
	}
	scriptidx = optind;
	CommandLine_setARGV(konoha, argc - scriptidx, argv + scriptidx);
	if(scriptidx < argc) {
		ret = konoha_load(konoha, argv[scriptidx]);
	}
	else {
		interactive_flag = 1;
		KonohaContext_setInteractive(konoha);
	}
	if(ret && interactive_flag) {
		CommandLine_import(konoha, "konoha.i");
		ret = konoha_shell(konoha);
	}
	return (ret == true) ? 0 : 1;
}

//static void testDataLog(KonohaContext *kctx)
//{
//	unsigned long long timer;
//	KSetElaspedTimer(timer);
//	KTraceApi(SystemFault|ActionPoint, "test", LogText("start", "test"), LogUint("count", 1), LOG_ERRNO);
//	KTraceApiElapsedTimer(SystemFault, 0/*ms*/, "syslog", timer);
//}

// -------------------------------------------------------------------------
// ** main **

int main(int argc, char *argv[])
{
	kbool_t ret = 1;
	if(getenv("KONOHA_DEBUG") != NULL) {
		verbose_debug = 1;
		verbose_gc = 1;
		verbose_sugar = 1;
		verbose_code = 1;
	}
	PlatformApi *plat = KonohaUtils_getDefaultPlatformApi();
	KonohaContext* konoha = konoha_open(plat);
	ret = konoha_parseopt(konoha, (PlatformApiVar*)plat, argc, argv);
	konoha_close(konoha);
	return ret ? konoha_detectFailedAssert: 0;
}

#ifdef __cplusplus
}
#endif
