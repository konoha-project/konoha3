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
//#include <minikonoha/konoha_common.h>

#ifdef __cplusplus
extern "C"{
#endif

#ifndef K_PATHMAX
#define K_PATHMAX 1024
#endif

#define LogFileName(S)     LogText("filename", S)
#define LogFileName2(S)    LogText("filename2", S)  // adhoc..
#define LogMode(mode)      LogUint("mode", mode)
#define LogOwner(o)        LogUint("owner", o)
#define LogGroup(o)        LogUint("group", o)

//static const char* kFile_textPath(KonohaContext *kctx, kFile *file)
//{
//	return (file->PathInfoNULL == NULL) ? "unknown" : S_text(file->PathInfoNULL);
//}
//
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

//## String getcwd()
static KMETHOD System_getcwd(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char filepath[K_PATHMAX] = {0};
	char *cwd = getcwd(filepath, K_PATHMAX);
	if(cwd != NULL) {
		char buffer[K_PATHMAX] = {0};
		const char *konohaPath = PLATAPI formatKonohaPath(kctx, buffer, sizeof(buffer), cwd, strlen(cwd), trace);
		KReturn(KLIB new_kString(kctx, OnStack, konohaPath, strlen(konohaPath), 0));
	}
	else {
		KTraceErrorPoint(trace, SystemFault, "getcwd", LogErrno);
		KReturn(KNULL(String));
	}
}

//## boolean chdir(String path)
static KMETHOD System_chdir(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	int ret = chdir(systemPath);
	if(ret == -1) {
		KTraceErrorPoint(trace, SystemFault, "chdir", LogFileName(S_text(path)), LogErrno);
	}
	KReturnUnboxValue(ret != -1);
}

//## boolean chroot(String path)
static KMETHOD System_chroot(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	int ret = chroot(systemPath);
	if(ret == -1) {
		KTraceErrorPoint(trace, SystemFault, "chroot", LogFileName(S_text(path)), LogErrno);
	}
	KReturnUnboxValue(ret != -1);
}


//## int System.umask(int cmask)
static KMETHOD System_umask(KonohaContext *kctx, KonohaStack *sfp)
{
	mode_t cmask = sfp[1].intValue;
	mode_t oldmask = umask(cmask);
	KReturnUnboxValue(oldmask);
}

//## int System.chmod(String path, int mode)
static KMETHOD System_chmod(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	mode_t mode = (mode_t)sfp[2].intValue;
	int ret = chmod(systemPath, mode);
	if(ret == -1) {
		KTraceErrorPoint(trace, SystemFault, "chmod", LogFileName(S_text(path)), LogMode(mode), LogErrno);
	}
	else {
		KTraceChangeSystemPoint(trace, "chmod", LogFileName(S_text(path)), LogMode(mode));
	}
	KReturnUnboxValue(ret != -1);
}

static KMETHOD System_access(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	mode_t mode = (mode_t)sfp[2].intValue;
	int ret = access(systemPath, mode);
	if(ret == -1) {
		KTraceErrorPoint(trace, SystemFault, "access", LogFileName(S_text(path)), LogMode(mode), LogErrno);
	}
	KReturnUnboxValue(ret != -1);
}

static KMETHOD System_chown(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	uid_t owner = (uid_t)sfp[2].intValue;
	gid_t group = (gid_t)sfp[3].intValue;
	int ret = chown(systemPath, owner, group);
	if(ret == -1) {
		KTraceErrorPoint(trace, SystemFault, "chown", LogFileName(S_text(path)), LogOwner(owner), LogGroup(group), LogErrno);
	}
	KReturnUnboxValue(ret != -1);
}

static KMETHOD System_lchown(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	uid_t owner = (uid_t)sfp[2].intValue;
	gid_t group = (gid_t)sfp[3].intValue;
	int ret = lchown(systemPath, owner, group);
	if(ret == -1) {
		KTraceErrorPoint(trace, SystemFault, "lchown", LogFileName(S_text(path)), LogOwner(owner), LogGroup(group), LogErrno);
	}
	KReturnUnboxValue(ret != -1);
}

/* ------------------------------------------------------------------------ */

static KMETHOD System_link(KonohaContext *kctx, KonohaStack *sfp)
{
	char buffer[K_PATHMAX], buffer2[K_PATHMAX];
	kString *path = sfp[1].asString, *path2 = sfp[2].asString;
	KMakeTrace(trace, sfp);
	const char *oldpath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	const char *newpath = PLATAPI formatSystemPath(kctx, buffer2, sizeof(buffer2), S_text(path2), S_size(path2), trace);
	int ret = link(oldpath, newpath);
	if(ret == -1) {
		int fault = PLATAPI diagnosisFaultType(kctx, kString_guessUserFault(path)|kString_guessUserFault(path2)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "link", LogFileName(S_text(path)), LogFileName2(S_text(path2)), LogErrno);
	}
	else {
		KTraceChangeSystemPoint(trace, "link", LogFileName(S_text(path)), LogFileName2(S_text(path2)), LogErrno);
	}
	KReturnUnboxValue(ret != -1);
}

static KMETHOD System_symlink(KonohaContext *kctx, KonohaStack *sfp)
{
	char buffer[K_PATHMAX], buffer2[K_PATHMAX];
	kString *path = sfp[1].asString, *path2 = sfp[2].asString;
	KMakeTrace(trace, sfp);
	const char *oldpath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	const char *newpath = PLATAPI formatSystemPath(kctx, buffer2, sizeof(buffer2), S_text(path2), S_size(path2), trace);
	int ret = symlink(oldpath, newpath);
	if(ret == -1) {
		int fault = PLATAPI diagnosisFaultType(kctx, kString_guessUserFault(path)|kString_guessUserFault(path2)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "symlink", LogFileName(S_text(path)), LogFileName2(S_text(path2)), LogErrno);
	}
	else {
		KTraceChangeSystemPoint(trace, "symlink", LogFileName(S_text(path)), LogFileName2(S_text(path2)), LogErrno);
	}
	KReturnUnboxValue(ret != -1);
}

static KMETHOD System_rename(KonohaContext *kctx, KonohaStack *sfp)
{
	char buffer[K_PATHMAX], buffer2[K_PATHMAX];
	kString *path = sfp[1].asString, *path2 = sfp[2].asString;
	KMakeTrace(trace, sfp);
	const char *oldpath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	const char *newpath = PLATAPI formatSystemPath(kctx, buffer2, sizeof(buffer2), S_text(path2), S_size(path2), trace);
	int ret = rename(oldpath, newpath);
	if(ret == -1) {
		int fault = PLATAPI diagnosisFaultType(kctx, kString_guessUserFault(path)|kString_guessUserFault(path2)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "rename", LogFileName(S_text(path)), LogFileName2(S_text(path2)), LogErrno);
	}
	else {
		KTraceChangeSystemPoint(trace, "rename", LogFileName(S_text(path)), LogFileName2(S_text(path2)), LogErrno);
	}
	KReturnUnboxValue(ret != -1);
}

static KMETHOD System_unlink(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	int ret = unlink(systemPath);
	if(ret == -1) {
		int fault = PLATAPI diagnosisFaultType(kctx, kString_guessUserFault(path)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "unlink", LogFileName(S_text(path)), LogErrno);
	}
	else {
		KTraceChangeSystemPoint(trace, "unlink", LogFileName(S_text(path)));
	}
	KReturnUnboxValue(ret != -1);
}

static KMETHOD System_readlink(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	char pathbuf[K_PATHMAX];
	ssize_t ret = readlink(systemPath, pathbuf, K_PATHMAX);
	if(ret == -1) {
		int fault = PLATAPI diagnosisFaultType(kctx, kString_guessUserFault(path)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "readlink", LogFileName(S_text(path)), LogErrno);
		KReturn(KNULL(String));
	}
	else {
		const char *konohaPath = PLATAPI formatKonohaPath(kctx, buffer, sizeof(buffer), pathbuf, strlen(pathbuf), trace);
		KReturn(KLIB new_kString(kctx, OnStack, konohaPath, strlen(konohaPath), 0));
	}
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

/* ------------------------------------------------------------------------ */
/* dir */

//## boolean System.isDir(String path)
static KMETHOD System_isDir(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	struct stat buf;
	if(stat(systemPath, &buf) == 0) {
		KReturnUnboxValue(S_ISDIR(buf.st_mode));
	}
	KReturnUnboxValue(false);
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

// --------------------------------------------------------------------------
/* stat */

//## Stat System.stat(String path)
static KMETHOD System_stat(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *path = S_text(sfp[1].asString);
	struct stat buf;
	int ret = stat(path, &buf);
	struct kStatVar *stat = NULL;
	if(ret != -1) {
		stat = (struct kStatVar *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), (uintptr_t)&buf);
	}
	else {
		// TODO: throw
		KReturn(KLIB Knull(kctx, KGetReturnType(sfp)));
	}
	KReturn(stat);
}

//## Stat System.lstat(String path)
static KMETHOD System_lstat(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *path = S_text(sfp[1].asString);
	struct stat buf;
	int ret = lstat(path, &buf);
	struct kStatVar *stat = NULL;
	if(ret != -1) {
		stat = (struct kStatVar *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), (uintptr_t)&buf);
	}
	else {
		// TODO: throw
	}
	KReturn(stat);
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Static   kMethod_Static
#define _C        kMethod_CCompatible
#define _Coercion kMethod_Coercion
#define _Im kMethod_Immutable
#define _F(F)   (intptr_t)(F)

static kbool_t path_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	//	KRequireKonohaCommonModule(trace);
	KDEFINE_METHOD MethodData[] = {
		_Public|_Static|_C, _F(System_getcwd),   TY_String,  TY_System, MN_("getcwd"), 0,
		_Public|_Static|_C, _F(System_chdir),    TY_boolean, TY_System, MN_("chdir"),  1, TY_String, FN_("path"),
		_Public|_Static|_C, _F(System_chroot),   TY_boolean, TY_System, MN_("chroot"), 1, TY_String, FN_("path"),

		_Public|_Static|_C, _F(System_umask),    TY_int,     TY_System, MN_("umask"),  1, TY_int,    FN_("mode"),
		_Public|_Static|_C, _F(System_chmod),    TY_boolean, TY_System, MN_("chmod"),  2, TY_String, FN_("path"), TY_int, FN_("mode"),
		_Public|_Static|_C, _F(System_access),   TY_boolean, TY_System, MN_("access"), 2, TY_String, FN_("path"), TY_int, FN_("mode"),
		_Public|_Static|_C, _F(System_chown),    TY_boolean, TY_System, MN_("chown"),  3, TY_String, FN_("path"), TY_int, FN_("owner"), TY_int, FN_("group"),
		_Public|_Static|_C, _F(System_lchown),   TY_boolean, TY_System, MN_("lchown"), 3, TY_String, FN_("path"), TY_int, FN_("owner"), TY_int, FN_("group"),

		_Public|_Static|_C, _F(System_link),     TY_boolean, TY_System, MN_("link"), 2, TY_String, FN_("oldpath"), TY_String, FN_("newpath"),
		_Public|_Static|_C, _F(System_unlink),   TY_boolean, TY_System, MN_("unlink"), 1, TY_String, FN_("path"),
		_Public|_Static|_C, _F(System_rename),   TY_boolean, TY_System, MN_("rename"), 2, TY_String, FN_("oldpath"), TY_String, FN_("newpath"),
		_Public|_Static|_C, _F(System_symlink),  TY_boolean, TY_System, MN_("symlink"), 2, TY_String, FN_("oldpath"), TY_String, FN_("newpath"),
		_Public|_Static|_C, _F(System_readlink), TY_String,  TY_System, MN_("readlink"), 1, TY_String, FN_("path"),

		_Public|_Static,    _F(System_isDir),    TY_boolean, TY_System, MN_("isdir"),    1, TY_String, FN_("path"),
		_Public|_Static|_C, _F(System_mkdir),    TY_boolean, TY_System, MN_("mkdir"),    2, TY_String, FN_("path"), TY_int, FN_("mode"),
		_Public|_Static|_C, _F(System_rmdir),    TY_boolean, TY_System, MN_("rmdir"),    1, TY_String, FN_("path"),
		_Public|_Static|_C, _F(System_truncate), TY_boolean, TY_System, MN_("truncate"), 2, TY_String, FN_("path"), TY_int, FN_("length"),

//		_Public|_Static, _F(System_stat),  TY_Stat, TY_System, MN_("stat"), 1, TY_String, FN_("path"),
//		_Public|_Static, _F(System_lstat), TY_Stat, TY_System, MN_("lstat"), 1, TY_String, FN_("path"),

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
