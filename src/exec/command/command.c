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
#include <konoha3/sugar.h>
#include <konoha3/klib.h>

#ifdef __GNUC__
#include <getopt.h>
#else
#include "./getopt.c"
#endif /*__GNUC__ */

#ifdef __cplusplus
extern "C" {
#endif

kstatus_t MODSUGAR_Eval(KonohaContext *kctx, const char *script, size_t len, kfileline_t uline);
kstatus_t MODSUGAR_loadScript(KonohaContext *kctx, const char *path, size_t len, KTraceInfo *trace);

// -------------------------------------------------------------------------
// getopt

static int compileonly_flag = 0;
static int interactive_flag = 0;

extern int verbose_debug;
int verbose_code;
extern int verbose_sugar;

#include <konoha3/platform.h>
#include <konoha3/libcode/minishell.h>

// -------------------------------------------------------------------------
// KonohaContext

#ifdef _MSC_VER
#define strcasecmp stricmp
#endif

static void CommandLine_Define(KonohaContext *kctx, char *keyvalue, KTraceInfo *trace)
{
	char *p = strchr(keyvalue, '=');
	if(p != NULL) {
		size_t len = p-keyvalue;
		char *namebuf = ALLOCA(char, len+1);
		memcpy(namebuf, keyvalue, len); namebuf[len] = 0;
//		DBG_P("name='%s'", namebuf);
		ksymbol_t key = KLIB Ksymbol(kctx, namebuf, len, 0, KSymbol_NewId);
		uintptr_t unboxValue;
		ktypeattr_t ty;
		if(isdigit(p[1])) {
			ty = KType_Int;
			unboxValue = (uintptr_t)strtol(p+1, NULL, 0);
		}
		else {
			ty = VirtualType_Text;
			unboxValue = (uintptr_t)(p+1);
		}
		KLIB kNameSpace_SetConstData(kctx, KNULL(NameSpace), key, ty, unboxValue, trace);
	}
	else {
		fprintf(stdout, "invalid define option: use -D<key>=<value>\n");
		KExit(EXIT_FAILURE);
	}
}

static void CommandLine_Import(KonohaContext *kctx, char *packageName, KTraceInfo *trace)
{
	size_t len = strlen(packageName)+1;
	char *bufname = ALLOCA(char, len);
	memcpy(bufname, packageName, len);
	KLIB kNameSpace_ImportPackage(kctx, KNULL(NameSpace), bufname, trace);
}

//static void konoha_startup(KonohaContext *kctx, const char *startup_script)
//{
//	char buf[256];
//	const char *path = PLATAPI getenv_i("KONOHA_SCRIPTPATH"), *local = "";
//	if(path == NULL) {
//		path = PLATAPI getenv_i("KONOHA_HOME");
//		local = "/script";
//	}
//	if(path == NULL) {
//		path = PLATAPI getenv_i("HOME");
//		local = "/.konoha/script";
//	}
//	snprintf(buf, sizeof(buf), "%s%s/%s.k", path, local, startup_script);
//	if(!Konoha_LoadScript((KonohaContext *)kctx, (const char *)buf)) {
//		KExit(EXIT_FAILURE);
//	}
//}

static void CommandLine_SetARGV(KonohaContext *kctx, int argc, char** argv, KTraceInfo *trace)
{
	INIT_GCSTACK();
	KClass *KClass_StringArray0 = KClass_p0(kctx, KClass_Array, KType_String);
	kArray *a = (kArray *)KLIB new_kObject(kctx, _GcStack, KClass_StringArray0, 0);
	int i;
	for(i = 0; i < argc; i++) {
		KLIB kArray_Add(kctx, a, KLIB new_kString(kctx, _GcStack, argv[i], strlen(argv[i]), StringPolicy_TEXT));
	}
	KDEFINE_OBJECT_CONST ObjectData[] = {
			{"SCRIPT_ARGV", KClass_StringArray0->typeId, (kObject *)a},
			{}
	};
	KLIB kNameSpace_LoadConstData(kctx, KNULL(NameSpace), KConst_(ObjectData), trace);
	RESET_GCSTACK();
}

static struct option long_options2[] = {
	/* These options set a flag. */
	{"verbose",         no_argument,       &verbose_debug, 1},
	{"verbose:sugar",   no_argument,       &verbose_sugar, 1},
	{"verbose:code",    no_argument,       &verbose_code,  1},
	{"interactive",     no_argument,       0, 'i'},
	{"typecheck",       no_argument,       0, 'c'},
	{"define",          required_argument, 0, 'D'},
	{"import",          required_argument, 0, 'I'},
	{"module",          required_argument, 0, 'M'},
	{"startwith",       required_argument, 0, 'S'},
	{"id",              no_argument,       0, 'q'},
	{NULL, 0, 0, 0},  /* sentinel */
};

static kbool_t Konoha_ParseCommandOption(KonohaContext* kctx, int argc, char **argv)
{
	kbool_t ret = true;
	int scriptidx = 0;
	KBaseTrace(trace);

	while(1) {
		int option_index = 0;
		int c = getopt_long (argc, argv, "icqD:I:M:S:f:", long_options2, &option_index);
		if(c == -1) break; /* Detect the end of the options. */
		switch (c) {

		case 'c': {
			compileonly_flag = 1;
			KonohaContext_Set(CompileOnly, kctx);
		}
		break;

		case 'i': {
			interactive_flag = 1;
			KonohaContext_Set(Interactive, kctx);
		}
		break;

		case 'q': {
			fprintf(stdout, "%s-%lu\n", K_VERSION, (long unsigned)K_DATE);
			KExit(EXIT_SUCCESS);  //
		}
		break;

		case 'D':
			CommandLine_Define(kctx, optarg, trace);
			break;

		case 'I':
			CommandLine_Import(kctx, optarg, trace);
			break;

		case 'M':
			// already checked in KonohaFactory_SetDefaultModule
			if(optarg != NULL && strcmp(optarg, "OutputTest") == 0) {
				KonohaContext_Set(Debug, kctx);
				verbose_debug = 0;
				verbose_sugar = 0;
				verbose_code  = 0;
			}
			break;

//		case 'S':
//			konoha_startup(kctx, optarg);
//			break;

		case '?':
			/* getopt_long already printed an error message. */
			break;

		default:
			((KonohaFactory *)kctx->platApi)->exitStatus = EXIT_FAILURE;
			return false;
		}
	}
	scriptidx = optind;
	CommandLine_SetARGV(kctx, argc - scriptidx, argv + scriptidx, trace);
	if(scriptidx < argc) {
		ret = Konoha_LoadScript(kctx, argv[scriptidx]);
	}
	else {
		interactive_flag = 1;
		KonohaContext_Set(Interactive, kctx);
	}
	if(interactive_flag) {
		CommandLine_Import(kctx, "MiniKonoha.Man", trace);
		ret = konoha_shell(kctx);
	}
	return ret;
}

// -------------------------------------------------------------------------
// ** main **

int main(int argc, char *argv[])
{
	struct KonohaFactory factory = {};
	KonohaFactory_SetDefaultFactory(&factory, PosixFactory, argc, argv);

	if(factory.getenv_i("KONOHA_DEBUG") != NULL) {
		verbose_debug = 1;
		verbose_sugar = 1;
		verbose_code = 1;
	}

	KonohaContext* konoha = KonohaFactory_CreateKonoha(&factory);
	Konoha_ParseCommandOption(konoha, argc, argv);
	return Konoha_Destroy(konoha);
}

#ifdef __cplusplus
}
#endif
