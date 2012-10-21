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

#define USE_FILE 1
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <stdio.h>

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/konoha_common.h>

#ifdef __cplusplus
extern "C"{
#endif

#ifndef K_PATHMAX
#define K_PATHMAX 1024
#endif

#define LogFileName(S)     LogText("filename", S)
#define LogMode(mode)      LogUint("mode", mode)

static const char* kFile_textPath(KonohaContext *kctx, kFile *file)
{
	return (file->PathInfoNULL == NULL) ? "unknown" : S_text(file->PathInfoNULL);
}

//static int TRACE_fgetc(KonohaContext *kctx, kFile *file, KTraceInfo *trace)
//{
//	int ch = fgetc(file->fp);
//	if(ferror(file->fp) != 0) {
//		KTraceErrorPoint(trace, SystemFault, "fgetc", LogFile(file), LogErrno);
//		KLIB KonohaRuntime_raise(kctx, EXPT_("IO"), SystemFault, NULL, trace->baseStack);
//	}
//	return ch;
//}
//
//static int TRACE_fputc(KonohaContext *kctx, kFile *file, int ch, KTraceInfo *trace)
//{
//	if(fputc(ch, file->fp) != 0) {
//		KTraceErrorPoint(trace, SystemFault, "fputc", LogFileName(file), LogErrno);
//		KLIB KonohaRuntime_raise(kctx, EXPT_("IO"), SystemFault, NULL, trace->baseStack);
//	}
//	return ch;
//}

/* ------------------------------------------------------------------------ */

//## int System.umask(int cmask)
static KMETHOD System_umask(KonohaContext *kctx, KonohaStack *sfp)
{
	mode_t cmask = sfp[1].intValue;
	mode_t ret = umask(cmask);
	KReturnUnboxValue(ret);
}

//## int System.chmod(String path, int mode)
static KMETHOD System_chmod(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	mode_t mode = (mode_t)sfp[2].intValue;
	int ret = chmod(path, mode);
	if(ret == -1) {
		KTraceErrorPoint(trace, SystemFault, "chmod", LogFileName(S_text(path)), LogMode(mode), LogErrno);
	}
	else {
		KTraceChangeSystemPoint(trace, "chmod", LogFileName(S_text(path)), LogMode(mode));
	}
	KReturnUnboxValue(ret != -1);
}

//## boolean System.mkdir(String path, int mode)
static KMETHOD System_mkdir(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	mode_t mode = (mode_t)sfp[2].intValue;
	int ret = mkdir(systemPath, mode);
	if(ret == -1) {
		KTraceErrorPoint(trace, SystemFault, "mkdir", LogFileName(S_text(path)), LogMode(mode), LogErrno);
	}
	else {
		KTraceChangeSystemPoint(trace, "mkdir", LogFileName(S_text(path)), LogMode(mode));
	}
	KReturnUnboxValue(ret != -1);
}

//## boolean System.rmdir(String path)
static KMETHOD System_rmdir(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	int ret = rmdir(systemPath);
	if(ret == -1) {
		int fault = PLATAPI diagnosisFaultType(kctx, kString_guessUserFault(path)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "rmdir", LogFileName(S_text(path)), LogErrno);
	}
	else {
		KTraceChangeSystemPoint(trace, "rmdir", LogFileName(S_text(path)));
	}
	KReturnUnboxValue(ret != -1);
}

//## boolean System.truncate(String path, int length)
static KMETHOD System_truncate(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	off_t length = (off_t)sfp[2].intValue;
	int ret = truncate(systemPath, length);
	if(ret == -1) {
		int fault = PLATAPI diagnosisFaultType(kctx, kString_guessUserFault(path)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "truncate", LogFileName(S_text(path)), LogUint("length", length), LogErrno);
	}
	else {
		KTraceChangeSystemPoint(trace, "truncate", LogFileName(S_text(path)), LogUint("length", length));
	}
	KReturnUnboxValue(ret != -1);
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Static   kMethod_Static
#define _Coercion kMethod_Coercion
#define _Im kMethod_Immutable
#define _F(F)   (intptr_t)(F)

static kbool_t path_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	//	KRequireKonohaCommonModule(trace);
	KDEFINE_METHOD MethodData[] = {
		_Public|_Static|_Const|_Im, _F(System_umask), TY_int, TY_System, MN_("umask"), 1, TY_int, FN_("cmask"),
		_Public|_Static, _F(System_mkdir), TY_boolean, TY_System, MN_("mkdir"), 2, TY_String, FN_("path"), TY_int, FN_("mode"),
		_Public|_Static, _F(System_rmdir), TY_boolean, TY_System, MN_("rmdir"), 1, TY_String, FN_("path"),
		_Public|_Static|_Im, _F(System_truncate), TY_boolean, TY_System, MN_("truncate"), 2, TY_String, FN_("path"), TY_int, FN_("length"),
		_Public|_Static|_Im, _F(System_chmod), TY_int, TY_System, MN_("chmod"), 2, TY_String, FN_("path"), TY_int, FN_("mode"),
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t path_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* path_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("posix", "1.0"),
		.initPackage    = path_initPackage,
		.setupPackage   = path_setupPackage,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
