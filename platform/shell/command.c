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
#ifdef __GNUC__
#include <getopt.h>
#else

char *optarg = 0;
int optind   = 1;
int optopt   = 0;
int opterr   = 0;
int optreset = 0;

struct option {
	char *name;
	int has_arg;
	int *flag;
	int val;
};

/* The has_arg field should be one of: */
enum {
	no_argument,       /* no argument to the option is expect        */
	required_argument, /* an argument to the option is required      */
	optional_argument, /* an argument to the option may be presented */
};

static int getopt_long(int argc, char * const *argv, const char *optstring, const struct option *longopts, int *longindex);

#include <string.h>
#include <ctype.h>
static int getopt_long(int argc, char * const *argv, const char *optstring, const struct option *longopts, int *longindex)
{
	if(optind < argc) {
		char *arg = argv[optind];
		if(arg == 0)
			return -1;
		if(arg[0] == '-' && arg[1] == '-') {
			const struct option *opt = longopts;
			arg += 2;
			while (opt->name) {
				char *end = strchr(arg, '=');
				if(end == 0 && opt->has_arg == no_argument) {
					if(strcmp(arg, opt->name) == 0)
						*longindex = opt - longopts;
					optind++;
					return opt->val;
				}
				if(strncmp(arg, opt->name, end - arg) == 0) {
					*longindex = opt - longopts;
					optarg = end+1;
					optind++;
					return opt->val;
				}
				opt++;
			}
		}
		else if(arg[0] == '-') {
			arg += 1;
			const char *c = optstring;
			while (*c != 0) {
				if(*c == arg[0]) {
					if(*(c+1) == ':' && arg[1] == '=') {
						optarg = arg+2;
					}
					optind++;
					return arg[0];
				}
				c++;
			}
		}
	}
	return -1;
}

#endif /*__GNUC__ */

#ifdef __cplusplus
extern "C" {
#endif

kstatus_t MODSUGAR_eval(KonohaContext *kctx, const char *script, size_t len, kfileline_t uline);
kstatus_t MODSUGAR_loadScript(KonohaContext *kctx, const char *path, size_t len, KTraceInfo *trace);

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
// KonohaContext

#ifdef _MSC_VER
#define strcasecmp stricmp
#endif

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
		char *namebuf = ALLOCA(char, len+1);
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
			ty = VirtualType_Text;
			unboxValue = (uintptr_t)(p+1);
		}
		if(!KLIB kNameSpace_SetConstData(kctx, KNULL(NameSpace), key, ty, unboxValue, 0)) {
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
	char *bufname = ALLOCA(char, len);
	memcpy(bufname, packageName, len);
	BEGIN_LOCAL(lsfp, K_CALLDELTA);
	KMakeTrace(trace, kctx->esp);
	if(!(KLIB kNameSpace_importPackage(kctx, KNULL(NameSpace), bufname, trace))) {
		PLATAPI exit_i(EXIT_FAILURE);
	}
	END_LOCAL();
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
	if(!Konoha_LoadScript((KonohaContext*)kctx, (const char*)buf)) {
		PLATAPI exit_i(EXIT_FAILURE);
	}
}

static void CommandLine_setARGV(KonohaContext *kctx, int argc, char** argv)
{
	INIT_GCSTACK();
	KonohaClass *CT_StringArray0 = CT_p0(kctx, CT_Array, TY_String);
	kArray *a = (kArray*)KLIB new_kObject(kctx, _GcStack, CT_StringArray0, 0);
	int i;
	for(i = 0; i < argc; i++) {
		DBG_P("argv=%d, '%s'", i, argv[i]);
		KLIB kArray_add(kctx, a, KLIB new_kString(kctx, _GcStack, argv[i], strlen(argv[i]), StringPolicy_TEXT));
	}
	KDEFINE_OBJECT_CONST ObjectData[] = {
			{"SCRIPT_ARGV", CT_StringArray0->typeId, (kObject*)a},
			{}
	};
	KLIB kNameSpace_LoadConstData(kctx, KNULL(NameSpace), KonohaConst_(ObjectData), 0);
	RESET_GCSTACK();
}

static struct option long_options2[] = {
	/* These options set a flag. */
	{"verbose",         no_argument,       &verbose_debug, 1},
	{"verbose:gc",      no_argument,       &verbose_gc,    1},
	{"verbose:sugar",   no_argument,       &verbose_sugar, 1},
	{"verbose:code",    no_argument,       &verbose_code,  1},
	{"format",          required_argument, 0, 'f'},
	{"interactive",     no_argument,       0, 'i'},
	{"typecheck",       no_argument,       0, 'c'},
	{"define",          required_argument, 0, 'D'},
	{"import",          required_argument, 0, 'I'},
	{"module",          required_argument, 0, 'M'},
	{"startwith",       required_argument, 0, 'S'},
	{"builtin-test",    required_argument, 0, 'B'},
	{"trace",           no_argument,       0, 'F'},
	{"id",              no_argument,       0, 'q'},
	{NULL, 0, 0, 0},  /* sentinel */
};

static void konoha_parseopt(KonohaContext* konoha, int argc, char **argv)
{
	kbool_t ret = true;
	int scriptidx = 0;
	while (1) {
		int option_index = 0;
		int c = getopt_long (argc, argv, "icqD:I:M:S:f:", long_options2, &option_index);
		if(c == -1) break; /* Detect the end of the options. */
		switch (c) {
		case 0:
			/* If this option set a flag, do nothing else now. */
			if(long_options2[option_index].flag != 0)
				break;
			printf ("option %s", long_options2[option_index].name);
			if(optarg)
				printf (" with arg %s", optarg);
			printf ("\n");
			break;

		case 'c': {
			compileonly_flag = 1;
			KonohaContext_Set(CompileOnly, konoha);
		}
		break;

		case 'i': {
			interactive_flag = 1;
			KonohaContext_Set(Interactive, konoha);
		}
		break;

		case 'q': {
			fprintf(stdout, "%s-%lu\n", K_VERSION, (long unsigned)K_DATE);
			exit(0);  //
		}
		break;

		case 'B':
			CommandLine_doBuiltInTest(konoha, optarg);
			return;

		case 'D':
			CommandLine_define(konoha, optarg);
			break;

		case 'F':
			KonohaContext_Set(Trace, konoha);
			break;

		case 'I':
			CommandLine_import(konoha, optarg);
			break;

		case 'M':
			// already checked in KonohaFactory_SetDefaultModule
			if(optarg != NULL && strcmp(optarg, "OutputTest") == 0) {
				KonohaContext_Set(Debug, konoha);
				verbose_debug = 0;
				verbose_sugar = 0;
				verbose_gc    = 0;
				verbose_code  = 0;
			}
			break;

		case 'S':
			konoha_startup(konoha, optarg);
			break;

		case '?':
			/* getopt_long already printed an error message. */
			break;

		case 'f':
			//printf("%s\n", optarg);
			if(strcmp(optarg, "JS") == 0){
				KonohaContext_setVisitor(konoha, kVisitor_JS);
			}else if(strcmp(optarg, "Dump") == 0){
				KonohaContext_setVisitor(konoha, kVisitor_Dump);
			}else{
				KonohaContext_setVisitor(konoha, kVisitor_KonohaVM);
			}
			break;

		default:
			((KonohaFactory*)konoha->platApi)->exitStatus = 1;
			return;
		}
	}
	scriptidx = optind;
	CommandLine_setARGV(konoha, argc - scriptidx, argv + scriptidx);
	if(scriptidx < argc) {
		ret = Konoha_LoadScript(konoha, argv[scriptidx]);
	}
	else {
		interactive_flag = 1;
		KonohaContext_Set(Interactive, konoha);
	}
	if(interactive_flag) {
		CommandLine_import(konoha, "konoha.i");
		ret = konoha_shell(konoha);
	}
}

// -------------------------------------------------------------------------
// ** main **

void KonohaFactory_LoadRuntimeModule(KonohaFactory *factory, const char *name, ModuleType option);
void KonohaFactory_SetDefaultFactory(KonohaFactory *factory, void (*SetPlatformApi)(KonohaFactory *), int argc, char **argv);
KonohaContext* KonohaFactory_CreateKonoha(KonohaFactory *factory);
int Konoha_Destroy(KonohaContext *kctx);

int main(int argc, char *argv[])
{
	if(getenv("KONOHA_DEBUG") != NULL) {
		verbose_debug = 1;
		verbose_gc = 1;
		verbose_sugar = 1;
		verbose_code = 1;
	}
	struct KonohaFactory factory = {};
	KonohaFactory_SetDefaultFactory(&factory, PosixFactory, argc, argv);
	KonohaContext* konoha = KonohaFactory_CreateKonoha(&factory);
	konoha_parseopt(konoha, argc, argv);
	return Konoha_Destroy(konoha);
}

#ifdef __cplusplus
}
#endif
