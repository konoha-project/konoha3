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
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <dirent.h>

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

//## String System.realpath(String path)
static KMETHOD System_realpath(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX], filepath[K_PATHMAX] = {0};
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	char *cwd = realpath(systemPath, filepath);
	if(cwd != NULL) {
		const char *konohaPath = PLATAPI formatKonohaPath(kctx, buffer, sizeof(buffer), cwd, strlen(cwd), trace);
		KReturn(KLIB new_kString(kctx, OnStack, konohaPath, strlen(konohaPath), 0));
	}
	else {
		KTraceErrorPoint(trace, SystemFault, "realpath", LogFileName(S_text(path)), LogErrno);
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

static void path_defineAccessConst(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_INT_CONST intData[] = {
		/*for System.access*/
		{KDefineConstInt(R_OK)},
		{KDefineConstInt(W_OK)},
		{KDefineConstInt(X_OK)},
		{KDefineConstInt(F_OK)},
		{NULL} /* sentinel */
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KonohaConst_(intData), 0);
}

// boolean System.access(String path, int mode);
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
		int fault = KLIB DiagnosisFaultType(kctx, kString_guessUserFault(path)|kString_guessUserFault(path2)|SystemError, trace);
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
		int fault = KLIB DiagnosisFaultType(kctx, kString_guessUserFault(path)|kString_guessUserFault(path2)|SystemError, trace);
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
		int fault = KLIB DiagnosisFaultType(kctx, kString_guessUserFault(path)|kString_guessUserFault(path2)|SystemError, trace);
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
		int fault = KLIB DiagnosisFaultType(kctx, kString_guessUserFault(path)|SystemError, trace);
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
		int fault = KLIB DiagnosisFaultType(kctx, kString_guessUserFault(path)|SystemError, trace);
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
		int fault = KLIB DiagnosisFaultType(kctx, kString_guessUserFault(path)|SystemError, trace);
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
		int fault = KLIB DiagnosisFaultType(kctx, kString_guessUserFault(path)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "rmdir", LogFileName(S_text(path)), LogErrno);
	}
	else {
		KTraceChangeSystemPoint(trace, "rmdir", LogFileName(S_text(path)));
	}
	KReturnUnboxValue(ret != -1);
}

/* DIR */

typedef struct kDirVar kDir;
struct kDirVar {
	KonohaObjectHeader h;
	DIR *dirp;
	kString *PathInfoNULL;
	uintptr_t readerIconv;
};

static void kDir_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kDir *dir = (kDir *)o;
	dir->dirp = conf;
	dir->PathInfoNULL = NULL;
	dir->readerIconv  = ICONV_NULL;
}

static void kDir_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kDir *dir = (kDir *)o;
	KREFTRACEn(dir->PathInfoNULL);
}

static void kDir_close(KonohaContext *kctx, kDir *dir)
{
	if(dir->dirp != NULL) {
		closedir(dir->dirp);
		dir->dirp = NULL;
	}
	if(dir->readerIconv != ICONV_NULL) {
		PLATAPI iconv_close_i(kctx, dir->readerIconv);
		dir->readerIconv = ICONV_NULL;
	}
}

static void kDir_free(KonohaContext *kctx, kObject *o)
{
	kDir_close(kctx, (kDir *)o);
}

static void kDir_p(KonohaContext *kctx, KonohaValue *v, int pos, KGrowingBuffer *wb)
{
	kDir *dir = (kDir *)v[pos].asObject;
	KLIB Kwb_printf(kctx, wb, "DIR: %s", S_text(dir->PathInfoNULL));
}

//## DIR System.opendir(String path)
static KMETHOD System_opendir(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI formatSystemPath(kctx, buffer, sizeof(buffer), S_text(path), S_size(path), trace);
	DIR *d = opendir(systemPath);
	if(d == NULL) {
		int fault = KLIB DiagnosisFaultType(kctx, kString_guessUserFault(path)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "opendir", LogText("dirname", S_text(path)), LogErrno);
		KLIB KonohaRuntime_raise(kctx, EXPT_("IO"), fault, NULL, sfp);
	}
	kDir *dir = (kDir *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), (uintptr_t)d);
	KFieldSet(dir, dir->PathInfoNULL, path);
	if(!PLATAPI isSystemCharsetUTF8(kctx)) {
		dir->readerIconv = PLATAPI iconvSystemCharsetToUTF8(kctx, trace);
	}
	KReturn(dir);
}

//## void DIR.close()
static KMETHOD DIR_close(KonohaContext *kctx, KonohaStack *sfp)
{
	kDir_close(kctx, (kDir *)sfp[0].asObject);
	KReturnVoid();
}

//## String DIR.readFileName()
static KMETHOD DIR_readFileName(KonohaContext *kctx, KonohaStack *sfp)
{
	kDir *dir = (kDir *)sfp[0].asObject;
	if(dir->dirp != NULL) {
		KMakeTrace(trace, sfp);
		struct dirent entry, *result;
		int ret = readdir_r(dir->dirp, &entry, &result);
		if(result != NULL) {
			char *d_name = result->d_name;
			if(dir->readerIconv == ICONV_NULL) {
				KReturn(KLIB new_kString(kctx, OnStack, d_name, strlen(d_name), StringPolicy_SystemInfo));
			}
			else {
				KGrowingBuffer wb;
				KLIB Kwb_init(&(kctx->stack->cwb), &wb);
				KLIB Kwb_iconv(kctx, &wb, dir->readerIconv, d_name, strlen(d_name), trace);
				KReturnWith(
					KLIB new_kString(kctx, OnStack, KLIB Kwb_top(kctx, &wb, 0), Kwb_bytesize(&wb), StringPolicy_SystemInfo),
					KLIB Kwb_free(&wb)
				);
			}
		}
		if(ret == -1) {
			KTraceErrorPoint(trace, SystemFault, "readdir", LogErrno);
		}
		kDir_close(kctx, dir);
	}
	KReturn(KNULL(String));
}

//## String DIR.readPath()
static KMETHOD DIR_readPath(KonohaContext *kctx, KonohaStack *sfp)
{
	kDir *dir = (kDir *)sfp[0].asObject;
	if(dir->dirp != NULL) {
		KMakeTrace(trace, sfp);
		struct dirent entry, *result;
		int ret = readdir_r(dir->dirp, &entry, &result);
		if(result != NULL) {
			char *d_name = result->d_name, delim[2] = {'/', 0};
			KGrowingBuffer wb;
			KLIB Kwb_init(&(kctx->stack->cwb), &wb);
			KLIB Kwb_write(kctx, &wb, S_text(dir->PathInfoNULL), S_size(dir->PathInfoNULL));
			KLIB Kwb_write(kctx, &wb, delim, 1);
			if(dir->readerIconv != ICONV_NULL) {
				KLIB Kwb_write(kctx, &wb, d_name, strlen(d_name));
			}
			else {
				KLIB Kwb_iconv(kctx, &wb, dir->readerIconv, d_name, strlen(d_name), trace);
			}
			KReturnWith(
				KLIB new_kString(kctx, OnStack, KLIB Kwb_top(kctx, &wb, 0), Kwb_bytesize(&wb), StringPolicy_SystemInfo),
				KLIB Kwb_free(&wb)
			);
		}
		if(ret == -1) {
			KTraceErrorPoint(trace, SystemFault, "readdir", LogErrno);
		}
		kDir_close(kctx, dir);
	}
	KReturn(KNULL(String));
}

#define _Public   kMethod_Public
#define _Static   kMethod_Static
#define _C        kMethod_CCompatible
#define _F(F)   (intptr_t)(F)
#define _Iter     kMethod_Iterative

static void path_defineDIR(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_CLASS defDIR = {};
	defDIR.structname = "DIR";
	defDIR.typeId = TY_newid;
	defDIR.cstruct_size = sizeof(struct kDirVar);
	defDIR.cflag = kClass_Final;
	defDIR.init  = kDir_init;
	defDIR.reftrace  = kDir_reftrace;
	defDIR.free  = kDir_free;
	defDIR.p     = kDir_p;
	KonohaClass *cDIR = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defDIR, trace);
	int TY_DIR = cDIR->typeId;
	KDEFINE_METHOD MethodData[] = {
		_Public|_Static|_C, _F(System_opendir),   TY_DIR,    TY_System, MN_("opendir"),  1, TY_String, FN_("dirname"),
		_Public,            _F(DIR_close),        TY_void,   TY_DIR,    MN_("close"), 0,
		_Public|_Iter,      _F(DIR_readFileName), TY_String, TY_DIR,    MN_("readFileName"), 0,
		_Public|_Iter,      _F(DIR_readPath),     TY_String, TY_DIR,    MN_("readPath"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

// --------------------------------------------------------------------------

static kbool_t path_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	//	KRequireKonohaCommonModule(trace);
	KDEFINE_METHOD MethodData[] = {
		_Public|_Static|_C, _F(System_getcwd),   TY_String,  TY_System, MN_("getcwd"), 0,
		_Public|_Static|_C, _F(System_realpath), TY_String,  TY_System, MN_("realpath"),  1, TY_String, FN_("path"),
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

		// isdir() is not posix api
		_Public|_Static,    _F(System_isDir),    TY_boolean, TY_System, MN_("isdir"),    1, TY_String, FN_("path"),
		_Public|_Static|_C, _F(System_mkdir),    TY_boolean, TY_System, MN_("mkdir"),    2, TY_String, FN_("path"), TY_int, FN_("mode"),
		_Public|_Static|_C, _F(System_rmdir),    TY_boolean, TY_System, MN_("rmdir"),    1, TY_String, FN_("path"),
		_Public|_Static|_C, _F(System_truncate), TY_boolean, TY_System, MN_("truncate"), 2, TY_String, FN_("path"), TY_int, FN_("length"),

//		_Public|_Static, _F(System_stat),  TY_Stat, TY_System, MN_("stat"), 1, TY_String, FN_("path"),
//		_Public|_Static, _F(System_lstat), TY_Stat, TY_System, MN_("lstat"), 1, TY_String, FN_("path"),

		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	path_defineAccessConst(kctx, ns, trace);
	path_defineDIR(kctx, ns, trace);
	return true;
}

static kbool_t path_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* path_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("posix", "1.0"),
		.PackupNameSpace    = path_PackupNameSpace,
		.ExportNameSpace   = path_ExportNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
