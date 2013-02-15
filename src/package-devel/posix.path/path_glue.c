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

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <dirent.h>

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/import/methoddecl.h>
#define _Iter  kMethod_Iterative
#define _C     kMethod_CCompatible

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
		const char *konohaPath = PLATAPI I18NModule.formatKonohaPath(kctx, buffer, sizeof(buffer), cwd, strlen(cwd), trace);
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
	const char *systemPath = PLATAPI I18NModule.formatSystemPath(kctx, buffer, sizeof(buffer), kString_text(path), kString_size(path), trace);
	char *cwd = realpath(systemPath, filepath);
	if(cwd != NULL) {
		const char *konohaPath = PLATAPI I18NModule.formatKonohaPath(kctx, buffer, sizeof(buffer), cwd, strlen(cwd), trace);
		KReturn(KLIB new_kString(kctx, OnStack, konohaPath, strlen(konohaPath), 0));
	}
	else {
		KTraceErrorPoint(trace, SystemFault, "realpath", LogFileName(kString_text(path)), LogErrno);
		KReturn(KNULL(String));
	}
}

//## boolean chdir(String path)
static KMETHOD System_chdir(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI I18NModule.formatSystemPath(kctx, buffer, sizeof(buffer), kString_text(path), kString_size(path), trace);
	int ret = chdir(systemPath);
	if(ret == -1) {
		KTraceErrorPoint(trace, SystemFault, "chdir", LogFileName(kString_text(path)), LogErrno);
	}
	KReturnUnboxValue(ret != -1);
}

//## boolean chroot(String path)
static KMETHOD System_chroot(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI I18NModule.formatSystemPath(kctx, buffer, sizeof(buffer), kString_text(path), kString_size(path), trace);
	int ret = chroot(systemPath);
	if(ret == -1) {
		KTraceErrorPoint(trace, SystemFault, "chroot", LogFileName(kString_text(path)), LogErrno);
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
	const char *systemPath = PLATAPI I18NModule.formatSystemPath(kctx, buffer, sizeof(buffer), kString_text(path), kString_size(path), trace);
	mode_t mode = (mode_t)sfp[2].intValue;
	int ret = chmod(systemPath, mode);
	if(ret == -1) {
		KTraceErrorPoint(trace, SystemFault, "chmod", LogFileName(kString_text(path)), LogMode(mode), LogErrno);
	}
	else {
		KTraceChangeSystemPoint(trace, "chmod", LogFileName(kString_text(path)), LogMode(mode));
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
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(intData), trace);
}

// boolean System.access(String path, int mode);
static KMETHOD System_access(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI I18NModule.formatSystemPath(kctx, buffer, sizeof(buffer), kString_text(path), kString_size(path), trace);
	mode_t mode = (mode_t)sfp[2].intValue;
	int ret = access(systemPath, mode);
	if(ret == -1) {
		KTraceErrorPoint(trace, SystemFault, "access", LogFileName(kString_text(path)), LogMode(mode), LogErrno);
	}
	KReturnUnboxValue(ret != -1);
}

static KMETHOD System_chown(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI I18NModule.formatSystemPath(kctx, buffer, sizeof(buffer), kString_text(path), kString_size(path), trace);
	uid_t owner = (uid_t)sfp[2].intValue;
	gid_t group = (gid_t)sfp[3].intValue;
	int ret = chown(systemPath, owner, group);
	if(ret == -1) {
		KTraceErrorPoint(trace, SystemFault, "chown", LogFileName(kString_text(path)), LogOwner(owner), LogGroup(group), LogErrno);
	}
	KReturnUnboxValue(ret != -1);
}

static KMETHOD System_lchown(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI I18NModule.formatSystemPath(kctx, buffer, sizeof(buffer), kString_text(path), kString_size(path), trace);
	uid_t owner = (uid_t)sfp[2].intValue;
	gid_t group = (gid_t)sfp[3].intValue;
	int ret = lchown(systemPath, owner, group);
	if(ret == -1) {
		KTraceErrorPoint(trace, SystemFault, "lchown", LogFileName(kString_text(path)), LogOwner(owner), LogGroup(group), LogErrno);
	}
	KReturnUnboxValue(ret != -1);
}

/* ------------------------------------------------------------------------ */

static KMETHOD System_link(KonohaContext *kctx, KonohaStack *sfp)
{
	char buffer[K_PATHMAX], buffer2[K_PATHMAX];
	kString *path = sfp[1].asString, *path2 = sfp[2].asString;
	KMakeTrace(trace, sfp);
	const char *oldpath = PLATAPI I18NModule.formatSystemPath(kctx, buffer, sizeof(buffer), kString_text(path), kString_size(path), trace);
	const char *newpath = PLATAPI I18NModule.formatSystemPath(kctx, buffer2, sizeof(buffer2), kString_text(path2), kString_size(path2), trace);
	int ret = link(oldpath, newpath);
	if(ret == -1) {
		int fault = KLIB DiagnosisFaultType(kctx, kString_GuessUserFault(path)|kString_GuessUserFault(path2)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "link", LogFileName(kString_text(path)), LogFileName2(kString_text(path2)), LogErrno);
	}
	else {
		KTraceChangeSystemPoint(trace, "link", LogFileName(kString_text(path)), LogFileName2(kString_text(path2)), LogErrno);
	}
	KReturnUnboxValue(ret != -1);
}

static KMETHOD System_symlink(KonohaContext *kctx, KonohaStack *sfp)
{
	char buffer[K_PATHMAX], buffer2[K_PATHMAX];
	kString *path = sfp[1].asString, *path2 = sfp[2].asString;
	KMakeTrace(trace, sfp);
	const char *oldpath = PLATAPI I18NModule.formatSystemPath(kctx, buffer, sizeof(buffer), kString_text(path), kString_size(path), trace);
	const char *newpath = PLATAPI I18NModule.formatSystemPath(kctx, buffer2, sizeof(buffer2), kString_text(path2), kString_size(path2), trace);
	int ret = symlink(oldpath, newpath);
	if(ret == -1) {
		int fault = KLIB DiagnosisFaultType(kctx, kString_GuessUserFault(path)|kString_GuessUserFault(path2)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "symlink", LogFileName(kString_text(path)), LogFileName2(kString_text(path2)), LogErrno);
	}
	else {
		KTraceChangeSystemPoint(trace, "symlink", LogFileName(kString_text(path)), LogFileName2(kString_text(path2)), LogErrno);
	}
	KReturnUnboxValue(ret != -1);
}

static KMETHOD System_rename(KonohaContext *kctx, KonohaStack *sfp)
{
	char buffer[K_PATHMAX], buffer2[K_PATHMAX];
	kString *path = sfp[1].asString, *path2 = sfp[2].asString;
	KMakeTrace(trace, sfp);
	const char *oldpath = PLATAPI I18NModule.formatSystemPath(kctx, buffer, sizeof(buffer), kString_text(path), kString_size(path), trace);
	const char *newpath = PLATAPI I18NModule.formatSystemPath(kctx, buffer2, sizeof(buffer2), kString_text(path2), kString_size(path2), trace);
	int ret = rename(oldpath, newpath);
	if(ret == -1) {
		int fault = KLIB DiagnosisFaultType(kctx, kString_GuessUserFault(path)|kString_GuessUserFault(path2)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "rename", LogFileName(kString_text(path)), LogFileName2(kString_text(path2)), LogErrno);
	}
	else {
		KTraceChangeSystemPoint(trace, "rename", LogFileName(kString_text(path)), LogFileName2(kString_text(path2)), LogErrno);
	}
	KReturnUnboxValue(ret != -1);
}

static KMETHOD System_unlink(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI I18NModule.formatSystemPath(kctx, buffer, sizeof(buffer), kString_text(path), kString_size(path), trace);
	int ret = unlink(systemPath);
	if(ret == -1) {
		int fault = KLIB DiagnosisFaultType(kctx, kString_GuessUserFault(path)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "unlink", LogFileName(kString_text(path)), LogErrno);
	}
	else {
		KTraceChangeSystemPoint(trace, "unlink", LogFileName(kString_text(path)));
	}
	KReturnUnboxValue(ret != -1);
}

static KMETHOD System_readlink(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI I18NModule.formatSystemPath(kctx, buffer, sizeof(buffer), kString_text(path), kString_size(path), trace);
	char pathbuf[K_PATHMAX];
	ssize_t ret = readlink(systemPath, pathbuf, K_PATHMAX);
	if(ret == -1) {
		int fault = KLIB DiagnosisFaultType(kctx, kString_GuessUserFault(path)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "readlink", LogFileName(kString_text(path)), LogErrno);
		KReturn(KNULL(String));
	}
	else {
		const char *konohaPath = PLATAPI I18NModule.formatKonohaPath(kctx, buffer, sizeof(buffer), pathbuf, strlen(pathbuf), trace);
		KReturn(KLIB new_kString(kctx, OnStack, konohaPath, strlen(konohaPath), 0));
	}
}

//## boolean System.truncate(String path, int length)
static KMETHOD System_truncate(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI I18NModule.formatSystemPath(kctx, buffer, sizeof(buffer), kString_text(path), kString_size(path), trace);
	off_t length = (off_t)sfp[2].intValue;
	int ret = truncate(systemPath, length);
	if(ret == -1) {
		int fault = KLIB DiagnosisFaultType(kctx, kString_GuessUserFault(path)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "truncate", LogFileName(kString_text(path)), LogUint("length", length), LogErrno);
	}
	else {
		KTraceChangeSystemPoint(trace, "truncate", LogFileName(kString_text(path)), LogUint("length", length));
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
	const char *systemPath = PLATAPI I18NModule.formatSystemPath(kctx, buffer, sizeof(buffer), kString_text(path), kString_size(path), trace);
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
	const char *systemPath = PLATAPI I18NModule.formatSystemPath(kctx, buffer, sizeof(buffer), kString_text(path), kString_size(path), trace);
	mode_t mode = (mode_t)sfp[2].intValue;
	int ret = mkdir(systemPath, mode);
	if(ret == -1) {
		KTraceErrorPoint(trace, SystemFault, "mkdir", LogFileName(kString_text(path)), LogMode(mode), LogErrno);
	}
	else {
		KTraceChangeSystemPoint(trace, "mkdir", LogFileName(kString_text(path)), LogMode(mode));
	}
	KReturnUnboxValue(ret != -1);
}

//## boolean System.rmdir(String path)
static KMETHOD System_rmdir(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI I18NModule.formatSystemPath(kctx, buffer, sizeof(buffer), kString_text(path), kString_size(path), trace);
	int ret = rmdir(systemPath);
	if(ret == -1) {
		int fault = KLIB DiagnosisFaultType(kctx, kString_GuessUserFault(path)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "rmdir", LogFileName(kString_text(path)), LogErrno);
	}
	else {
		KTraceChangeSystemPoint(trace, "rmdir", LogFileName(kString_text(path)));
	}
	KReturnUnboxValue(ret != -1);
}

/* DIR */

typedef struct kDirVar kDir;
struct kDirVar {
	kObjectHeader h;
	DIR *dirp;
	kString *PathInfoNULL;
	uintptr_t readerIconv;
};

static void kDir_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kDir *dir = (kDir *)o;
	dir->dirp = conf;
	dir->PathInfoNULL = NULL;
	dir->readerIconv  = ICONV_NULL;
}

static void kDir_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kDir *dir = (kDir *)o;
	KRefTraceNullable(dir->PathInfoNULL);
}

static void kDir_close(KonohaContext *kctx, kDir *dir)
{
	if(dir->dirp != NULL) {
		closedir(dir->dirp);
		dir->dirp = NULL;
	}
	if(dir->readerIconv != ICONV_NULL) {
		PLATAPI I18NModule.iconv_close_i(kctx, dir->readerIconv);
		dir->readerIconv = ICONV_NULL;
	}
}

static void kDir_Free(KonohaContext *kctx, kObject *o)
{
	kDir_close(kctx, (kDir *)o);
}

static void kDir_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kDir *dir = (kDir *)v[pos].asObject;
	KLIB KBuffer_printf(kctx, wb, "DIR: %s", kString_text(dir->PathInfoNULL));
}

//## DIR System.opendir(String path)
static KMETHOD System_opendir(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	char buffer[K_PATHMAX];
	kString *path = sfp[1].asString;
	const char *systemPath = PLATAPI I18NModule.formatSystemPath(kctx, buffer, sizeof(buffer), kString_text(path), kString_size(path), trace);
	DIR *d = opendir(systemPath);
	if(d == NULL) {
		int fault = KLIB DiagnosisFaultType(kctx, kString_GuessUserFault(path)|SystemError, trace);
		KTraceErrorPoint(trace, fault, "opendir", LogText("dirname", kString_text(path)), LogErrno);
		KLIB KRuntime_raise(kctx, KException_("IO"), fault, NULL, sfp);
	}
	kDir *dir = (kDir *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), (uintptr_t)d);
	KFieldSet(dir, dir->PathInfoNULL, path);
	if(!PLATAPI I18NModule.isSystemCharsetUTF8(kctx)) {
		dir->readerIconv = PLATAPI I18NModule.iconvSystemCharsetToUTF8(kctx, trace);
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
				KBuffer wb;
				KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
				KLIB KBuffer_iconv(kctx, &wb, dir->readerIconv, d_name, strlen(d_name), trace);
				KReturn(KLIB KBuffer_Stringfy(kctx, &wb, OnStack, StringPolicy_FreeKBuffer));
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
			KBuffer wb;
			KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
			KLIB KBuffer_Write(kctx, &wb, kString_text(dir->PathInfoNULL), kString_size(dir->PathInfoNULL));
			KLIB KBuffer_Write(kctx, &wb, delim, 1);
			if(dir->readerIconv != ICONV_NULL) {
				KLIB KBuffer_Write(kctx, &wb, d_name, strlen(d_name));
			}
			else {
				KLIB KBuffer_iconv(kctx, &wb, dir->readerIconv, d_name, strlen(d_name), trace);
			}
			KReturn(KLIB KBuffer_Stringfy(kctx, &wb, OnStack, StringPolicy_FreeKBuffer));
		}
		if(ret == -1) {
			KTraceErrorPoint(trace, SystemFault, "readdir", LogErrno);
		}
		kDir_close(kctx, dir);
	}
	KReturn(KNULL(String));
}

static void path_defineDIR(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_CLASS defDIR = {};
	defDIR.structname = "DIR";
	defDIR.typeId = KTypeAttr_NewId;
	defDIR.cstruct_size = sizeof(struct kDirVar);
	defDIR.cflag = KClassFlag_Final;
	defDIR.init  = kDir_Init;
	defDIR.reftrace  = kDir_Reftrace;
	defDIR.free  = kDir_Free;
	defDIR.format     = kDir_format;
	KClass *cDIR = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defDIR, trace);
	int KType_DIR = cDIR->typeId;
	KDEFINE_METHOD MethodData[] = {
		_Public|_Static|_C, _F(System_opendir),   KType_DIR,    KType_System, KMethodName_("opendir"),  1, KType_String, KFieldName_("dirname"),
		_Public,            _F(DIR_close),        KType_void,   KType_DIR,    KMethodName_("close"), 0,
		_Public|_Iter,      _F(DIR_readFileName), KType_String, KType_DIR,    KMethodName_("readFileName"), 0,
		_Public|_Iter,      _F(DIR_readPath),     KType_String, KType_DIR,    KMethodName_("readPath"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

// --------------------------------------------------------------------------

static kbool_t path_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	//	KRequireKonohaCommonModule(trace);
	KDEFINE_METHOD MethodData[] = {
		_Public|_Static|_C, _F(System_getcwd),   KType_String,  KType_System, KMethodName_("getcwd"), 0,
		_Public|_Static|_C, _F(System_realpath), KType_String,  KType_System, KMethodName_("realpath"),  1, KType_String, KFieldName_("path"),
		_Public|_Static|_C, _F(System_chdir),    KType_Boolean, KType_System, KMethodName_("chdir"),  1, KType_String, KFieldName_("path"),
		_Public|_Static|_C, _F(System_chroot),   KType_Boolean, KType_System, KMethodName_("chroot"), 1, KType_String, KFieldName_("path"),

		_Public|_Static|_C, _F(System_umask),    KType_Int,     KType_System, KMethodName_("umask"),  1, KType_Int,    KFieldName_("mode"),
		_Public|_Static|_C, _F(System_chmod),    KType_Boolean, KType_System, KMethodName_("chmod"),  2, KType_String, KFieldName_("path"), KType_Int, KFieldName_("mode"),
		_Public|_Static|_C, _F(System_access),   KType_Boolean, KType_System, KMethodName_("access"), 2, KType_String, KFieldName_("path"), KType_Int, KFieldName_("mode"),
		_Public|_Static|_C, _F(System_chown),    KType_Boolean, KType_System, KMethodName_("chown"),  3, KType_String, KFieldName_("path"), KType_Int, KFieldName_("owner"), KType_Int, KFieldName_("group"),
		_Public|_Static|_C, _F(System_lchown),   KType_Boolean, KType_System, KMethodName_("lchown"), 3, KType_String, KFieldName_("path"), KType_Int, KFieldName_("owner"), KType_Int, KFieldName_("group"),

		_Public|_Static|_C, _F(System_link),     KType_Boolean, KType_System, KMethodName_("link"), 2, KType_String, KFieldName_("oldpath"), KType_String, KFieldName_("newpath"),
		_Public|_Static|_C, _F(System_unlink),   KType_Boolean, KType_System, KMethodName_("unlink"), 1, KType_String, KFieldName_("path"),
		_Public|_Static|_C, _F(System_rename),   KType_Boolean, KType_System, KMethodName_("rename"), 2, KType_String, KFieldName_("oldpath"), KType_String, KFieldName_("newpath"),
		_Public|_Static|_C, _F(System_symlink),  KType_Boolean, KType_System, KMethodName_("symlink"), 2, KType_String, KFieldName_("oldpath"), KType_String, KFieldName_("newpath"),
		_Public|_Static|_C, _F(System_readlink), KType_String,  KType_System, KMethodName_("readlink"), 1, KType_String, KFieldName_("path"),

		// isdir() is not posix api
		_Public|_Static,    _F(System_isDir),    KType_Boolean, KType_System, KMethodName_("isdir"),    1, KType_String, KFieldName_("path"),
		_Public|_Static|_C, _F(System_mkdir),    KType_Boolean, KType_System, KMethodName_("mkdir"),    2, KType_String, KFieldName_("path"), KType_Int, KFieldName_("mode"),
		_Public|_Static|_C, _F(System_rmdir),    KType_Boolean, KType_System, KMethodName_("rmdir"),    1, KType_String, KFieldName_("path"),
		_Public|_Static|_C, _F(System_truncate), KType_Boolean, KType_System, KMethodName_("truncate"), 2, KType_String, KFieldName_("path"), KType_Int, KFieldName_("length"),

//		_Public|_Static, _F(System_stat),  KType_Stat, KType_System, KMethodName_("stat"), 1, KType_String, KFieldName_("path"),
//		_Public|_Static, _F(System_lstat), KType_Stat, KType_System, KMethodName_("lstat"), 1, KType_String, KFieldName_("path"),

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

KDEFINE_PACKAGE *path_Init(void)
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
