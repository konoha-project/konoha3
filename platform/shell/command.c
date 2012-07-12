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
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <setjmp.h>
#include <syslog.h>
#include <dlfcn.h>
#include <minikonoha/klib.h>

#ifdef __cplusplus
extern "C" {
#endif

kstatus_t MODSUGAR_eval(CTX, const char *script, size_t len, kline_t uline);
kstatus_t MODSUGAR_loadscript(CTX, const char *path, size_t len, kline_t pline);

kplatform_t* platform_shell(void);

// -------------------------------------------------------------------------
// getopt

static int compileonly_flag = 0;
static int interactive_flag = 0;

extern int verbose_debug;
extern int verbose_code;
extern int verbose_sugar;
extern int verbose_gc;

// -------------------------------------------------------------------------
// minishell

static char *(*kreadline)(const char *);
static int  (*kadd_history)(const char *);

static char* readline(const char* prompt)
{
	static int checkCTL = 0;
	int ch, pos = 0;
	static char linebuf[1024]; // THREAD-UNSAFE
	fputs(prompt, stdout);
	while((ch = fgetc(stdin)) != EOF) {
		if(ch == '\r') continue;
		if(ch == 27) {
			/* ^[[A */;
			fgetc(stdin); fgetc(stdin);
			if(checkCTL == 0) {
				fprintf(stdout, " - use readline, it provides better shell experience.\n");
				checkCTL = 1;
			}
			continue;
		}
		if(ch == '\n' || pos == sizeof(linebuf) - 1) {
			linebuf[pos] = 0;
			break;
		}
		linebuf[pos] = ch;
		pos++;
	}
	if(ch == EOF) return NULL;
	char *p = (char*)malloc(pos+1);
	memcpy(p, linebuf, pos+1);
	return p;
}

static int add_history(const char* line)
{
	return 0;
}

static int checkstmt(const char *t, size_t len)
{
	size_t i = 0;
	int ch, quote = 0, nest = 0;
	L_NORMAL:
	for(; i < len; i++) {
		ch = t[i];
		if(ch == '{' || ch == '[' || ch == '(') nest++;
		if(ch == '}' || ch == ']' || ch == ')') nest--;
		if(ch == '\'' || ch == '"' || ch == '`') {
			if(t[i+1] == ch && t[i+2] == ch) {
				quote = ch; i+=2;
				goto L_TQUOTE;
			}
		}
	}
	return nest;
	L_TQUOTE:
	DBG_ASSERT(i > 0);
	for(; i < len; i++) {
		ch = t[i];
		if(t[i-1] != '\\' && ch == quote) {
			if(t[i+1] == ch && t[i+2] == ch) {
				i+=2;
				goto L_NORMAL;
			}
		}
	}
	return 1;
}

static kstatus_t readstmt(CTX, kwb_t *wb, kline_t *uline)
{
	int line = 1;
	kstatus_t status = K_CONTINUE;
//	fputs(TERM_BBOLD(_ctx), stdout);
	while(1) {
		int check;
		char *ln = kreadline(line == 1 ? ">>> " : "    ");
		if(ln == NULL) {
			kwb_free(wb);
			status = K_BREAK;
			break;
		}
		if(line > 1) kwb_putc(wb, '\n');
		kwb_write(wb, ln, strlen(ln));
		free(ln);
		if((check = checkstmt(kwb_top(wb, 0), kwb_bytesize(wb))) > 0) {
			uline[0]++;
			line++;
			continue;
		}
		if(check < 0) {
			fputs("(Cancelled)...\n", stdout);
			kwb_free(wb);
		}
		break;
	}
	if(kwb_bytesize(wb) > 0) {
		kadd_history(kwb_top(wb, 1));
	}
//	fputs(TERM_EBOLD(_ctx), stdout);
	fflush(stdout);
	uline[0]++;
	return status;
}

static void dumpEval(CTX, kwb_t *wb)
{
	kstack_t *base = _ctx->stack;
	ktype_t ty = base->evalty;
	if(ty != TY_void) {
		ksfp_t *lsfp = base->stack + base->evalidx;
		CT_(ty)->p(_ctx, lsfp, 0, wb, P_DUMP);
		fflush(stdout);
		fprintf(stdout, "TYPE=%s EVAL=%s\n", TY_t(ty), kwb_top(wb,1));
	}
}

static void shell(CTX)
{
	kwb_t wb;
	kwb_init(&(_ctx->stack->cwb), &wb);
	kline_t uline = FILEID_("(shell)") | 1;
	while(1) {
		kline_t inc = 0;
		kstatus_t status = readstmt(_ctx, &wb, &inc);
		if(status == K_CONTINUE && kwb_bytesize(&wb) > 0) {
			status = konoha_eval((konoha_t)_ctx, kwb_top(&wb, 1), uline);
			uline += inc;
			kwb_free(&wb);
			if(status != K_FAILED) {
				dumpEval(_ctx, &wb);
				kwb_free(&wb);
			}
		}
		if(status == K_BREAK) {
			break;
		}
	}
	kwb_free(&wb);
	fprintf(stdout, "\n");
	return;
}

static void show_version(CTX)
{
	int i;
	fprintf(stdout, "Konoha 2.0-beta (%s) (%x, %s)\n", K_CODENAME, K_REVISION, __DATE__);
	fprintf(stdout, "[gcc %s]\n", __VERSION__);
	fprintf(stdout, "options:");
	for(i = 0; i < MOD_MAX; i++) {
		if(_ctx->modshare[i] != NULL) {
			fprintf(stdout, " %s", _ctx->modshare[i]->name);
		}
	}
	fprintf(stdout, "\n");
}

static kbool_t konoha_shell(konoha_t konoha)
{
	void *handler = dlopen("libreadline" K_OSDLLEXT, RTLD_LAZY);
	void *f = (handler != NULL) ? dlsym(handler, "readline") : NULL;
	kreadline = (f != NULL) ? (char* (*)(const char*))f : readline;
	f = (handler != NULL) ? dlsym(handler, "add_history") : NULL;
	kadd_history = (f != NULL) ? (int (*)(const char*))f : add_history;
	show_version((CTX_t)konoha);
	shell((CTX_t)konoha);
	return true;
}

// -------------------------------------------------------------------------
// platform api

#ifndef K_PREFIX
#define K_PREFIX  "/usr/local"
#endif

static const char* packname(const char *str)
{
	char *p = strrchr(str, '.');
	return (p == NULL) ? str : (const char*)p+1;
}

static const char* packagepath(char *buf, size_t bufsiz, const char *fname)
{
	char *path = getenv("KONOHA_PACKAGEPATH"), *local = "";
	if(path == NULL) {
		path = getenv("KONOHA_HOME");
		local = "/package";
	}
	if(path == NULL) {
		path = getenv("HOME");
		local = "/.minikonoha/package";
	}
	snprintf(buf, bufsiz, "%s%s/%s/%s_glue.k", path, local, fname, packname(fname));
#ifdef K_PREFIX
	FILE *fp = fopen(buf, "r");
	if(fp != NULL) {
		fclose(fp);
	}
	else {
		snprintf(buf, bufsiz, K_PREFIX "/minikonoha/package" "/%s/%s_glue.k", fname, packname(fname));
	}
#endif
	return (const char*)buf;
}

static const char* exportpath(char *pathbuf, size_t bufsiz, const char *pname)
{
	char *p = strrchr(pathbuf, '/');
	snprintf(p, bufsiz - (p  - pathbuf), "/%s_exports.k", packname(pname));
	FILE *fp = fopen(pathbuf, "r");
	if(fp != NULL) {
		fclose(fp);
		return (const char*)pathbuf;
	}
	return NULL;
}

static const char* begin(kinfotag_t t)
{
	DBG_ASSERT(t <= NoneTag);
	static const char* tags[] = {
		"\x1b[1m\x1b[31m", /*CritTag*/
		"\x1b[1m\x1b[31m", /*ErrTag*/
		"\x1b[1m\x1b[31m", /*WarnTag*/
		"\x1b[1m", /*NoticeTag*/
		"\x1b[1m", /*InfoTag*/
		"", /*DebugTag*/
		"", /* NoneTag*/
	};
	return tags[(int)t];
}

static const char* end(kinfotag_t t)
{
	DBG_ASSERT(t <= NoneTag);
	static const char* tags[] = {
		"\x1b[0m", /*CritTag*/
		"\x1b[0m", /*ErrTag*/
		"\x1b[0m", /*WarnTag*/
		"\x1b[0m", /*NoticeTag*/
		"\x1b[0m", /*InfoTag*/
		"", /* Debug */
		"", /* NoneTag*/
	};
	return tags[(int)t];
}

static void dbg_p(const char *file, const char *func, int line, const char *fmt, ...)
{
	va_list ap;
	va_start(ap , fmt);
	fflush(stdout);
	fprintf(stderr, "DEBUG(%s:%d) ", func, line);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

static void NOP_dbg_p(const char *file, const char *func, int line, const char *fmt, ...)
{
}

kplatform_t* platform_shell(void)
{
	static kplatform_t plat = {
		.name            = "shell",
		.stacksize       = K_PAGESIZE * 4,
		.malloc_i        = malloc,
		.free_i          = free,
		.setjmp_i        = ksetjmp,
		.longjmp_i       = klongjmp,

		.realpath_i      = realpath,
		.fopen_i         = (FILE_i* (*)(const char*, const char*))fopen,
		.fgetc_i         = (int     (*)(FILE_i *))fgetc,
		.feof_i          = (int     (*)(FILE_i *))feof,
		.fclose_i        = (int     (*)(FILE_i *))fclose,
		.syslog_i        = syslog,
		.vsyslog_i       = vsyslog,
		.printf_i        = printf,
		.vprintf_i       = vprintf,
		.snprintf_i      = snprintf,  // avoid to use Xsnprintf
		.vsnprintf_i     = vsnprintf, // retreating..
		.qsort_i         = qsort,
		.exit_i          = exit,
		// high level
		.packagepath     = packagepath,
		.exportpath      = exportpath,
		.begin           = begin,
		.end             = end,
		.dbg_p           = dbg_p,
	};
	if(!verbose_debug) {
		plat.dbg_p = NOP_dbg_p;
	}
	return (&plat);
}

// -------------------------------------------------------------------------
// konoha_test

static FILE *stdlog;

static const char* TEST_begin(kinfotag_t t)
{
	return "";
}

static const char* TEST_end(kinfotag_t t)
{
	return "";
}

static int TEST_vprintf(const char *fmt, va_list ap)
{
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

static int check_result(FILE *fp0, FILE *fp1)
{
	char buf0[128];
	char buf1[128];
	while (true) {
		size_t len0, len1;
		len0 = fread(buf0, 1, sizeof(buf0), fp0);
		len1 = fread(buf1, 1, sizeof(buf1), fp1);
		if (len0 != len1) {
			return 1;//FAILED
		}
		if (len0 == 0) {
			break;
		}
		if (memcmp(buf0, buf1, len0) != 0) {
			return 1;//FAILED
		}
	}
	return 0; //OK
}

extern int konoha_AssertResult;

static int konoha_test(CTX, const char *testname)
{
	int ret = 1; //FAILED
	char script_file[256];
	char correct_file[256];
	char result_file[256];
	PLAT snprintf_i(script_file, 256,  "%s", testname);
	PLAT snprintf_i(correct_file, 256, "%s.proof", script_file);
	PLAT snprintf_i(result_file, 256,  "%s.tested", script_file);
	FILE *fp = fopen(correct_file, "r");
	if (fp == NULL) {
		fprintf(stdout, "no proof file: %s\n", testname);
	}
	stdlog = fopen(result_file, "w");
	konoha_load((konoha_t)_ctx, script_file);
	fprintf(stdlog, "Q.E.D.\n");   // Q.E.D.
	fclose(stdlog);

	if(fp != NULL) {
		FILE *fp2 = fopen(result_file, "r");
		ret = check_result(fp, fp2);
		if(ret == 0) {
			fprintf(stdout, "[PASS]: %s\n", testname);
		}
		else {
			fprintf(stdout, "[FAIL]: %s\n", testname);
			konoha_AssertResult = 1;
		}
		fclose(fp);
		fclose(fp2);
	}
	return ret;
}

#ifdef USE_BUILTINTEST
extern DEFINE_TESTFUNC KonohaTestSet[];
static Ftest lookupTestFunc(DEFINE_TESTFUNC *d, const char *name)
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

static int konoha_builtintest(konoha_t konoha, const char* name)
{
#ifdef USE_BUILTINTEST
	Ftest f = lookupTestFunc(KonohaTestSet, name);
	if(f != NULL) {
		return f((CTX_t)konoha);
	}
	fprintf(stderr, "Built-in test is not found: '%s'\n", name);
#else
	fprintf(stderr, "Built-in tests are not built; rebuild with -DUSE_BUILTINTEST\n");
#endif
	return 1;
}

static void konoha_define(CTX, char *keyvalue)
{
	char *p = strchr(keyvalue, '=');
	if(p != NULL) {
		if(isdigit(p[1])) {
			long n = strtol(p+1, NULL, 0);
			KDEFINE_INT_CONST ConstData[] = {
				{keyvalue, TY_Int, n}, {}
			};
			kNameSpace_loadConstData(KNULL(NameSpace), ConstData, 0);
		}
		else {
			KDEFINE_TEXT_CONST ConstData[] = {
				{keyvalue, TY_TEXT, p+1}, {}
			};
			kNameSpace_loadConstData(KNULL(NameSpace), ConstData, 0);
		}
	}
	else {
		fprintf(stdout, "invalid define option: use -D<key>=<value>\n");
	}
}

static void konoha_import(CTX, char *packagename)
{
	size_t len = strlen(packagename)+1;
	char bufname[len];
	memcpy(bufname, packagename, len);
	if(!KREQUIRE_PACKAGE(bufname, 0)) {
		PLAT exit_i(EXIT_FAILURE);
	}
	KEXPORT_PACKAGE(bufname, KNULL(NameSpace), 0);
}

static void konoha_startup(CTX, const char *startup_script)
{
	char buf[256];
	char *path = getenv("KONOHA_SCRIPTPATH"), *local = "";
	if(path == NULL) {
		path = getenv("KONOHA_HOME");
		local = "/script";
	}
	if(path == NULL) {
		path = getenv("HOME");
		local = "/.minikonoha/script";
	}
	snprintf(buf, sizeof(buf), "%s%s/%s.k", path, local, startup_script);
	if(!konoha_load((konoha_t)_ctx, (const char*)buf)) {
		PLAT exit_i(EXIT_FAILURE);
	}
}

static void konoha_commandline(CTX, int argc, char** argv)
{
	kclass_t *CT_StringArray0 = CT_p0(_ctx, CT_Array, TY_String);
	kArray *a = (kArray*)new_kObject(CT_StringArray0, NULL);
	int i;
	for(i = 0; i < argc; i++) {
		DBG_P("argv=%d, '%s'", i, argv[i]);
		kArray_add(a, new_kString(argv[i], strlen(argv[i]), SPOL_TEXT));
	}
	KDEFINE_OBJECT_CONST ConstData[] = {
			{"SCRIPT_ARGV", CT_StringArray0->cid, (kObject*)a},
			{}
	};
	kNameSpace_loadConstData(KNULL(NameSpace), ConstData, 0);
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

static int konoha_parseopt(konoha_t konoha, kplatform_t *plat, int argc, char **argv)
{
	int ret = true, scriptidx = 0;
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
			CTX_setCompileOnly(konoha);
		}
		break;

		case 'i': {
			interactive_flag = 1;
			CTX_setInteractive(konoha);
		}
		break;

		case 'B':
			return konoha_builtintest(konoha, optarg);

		case 'D':
			konoha_define(konoha, optarg);
			break;

		case 'I':
			konoha_import(konoha, optarg);
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
			plat->dbg_p = NOP_dbg_p;
			plat->printf_i  = TEST_printf;
			plat->vprintf_i = TEST_vprintf;
			plat->begin  = TEST_begin;
			plat->end    = TEST_end;
			return konoha_test(konoha, optarg);

		case '?':
			/* getopt_long already printed an error message. */
			break;

		default:
			return 1;
		}
	}
	scriptidx = optind;
	konoha_commandline(konoha, argc - scriptidx, argv + scriptidx);
	if(scriptidx < argc) {
		ret = konoha_load(konoha, argv[scriptidx]);
	}
	else {
		interactive_flag = 1;
		CTX_setInteractive(konoha);
	}
	if(ret && interactive_flag) {
		konoha_import(konoha, "konoha.i");
		ret = konoha_shell(konoha);
	}
	return (ret == true) ? 0 : 1;
}

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
	kplatform_t *plat = platform_shell();
	konoha_t konoha = konoha_open((const kplatform_t*)plat);
	ret = konoha_parseopt(konoha, plat, argc, argv);
	konoha_close(konoha);
	MODGC_check_malloced_size();
	return ret ? konoha_AssertResult: 0;
}



#ifdef __cplusplus
}
#endif
