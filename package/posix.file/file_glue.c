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
#include <minikonoha/bytes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"{
#endif

typedef const struct kFILEVar kFILE;
struct kFILEVar {
	KonohaObjectHeader h;
	FILE *fp;
	const char *realpath;
};

/* ------------------------------------------------------------------------ */

static void File_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct kFILEVar *file = (struct kFILEVar*)o;
	file->fp = (conf != NULL) ? conf : NULL;
	file->realpath = NULL;
}

static void File_free(KonohaContext *kctx, kObject *o)
{
	struct kFILEVar *file = (struct kFILEVar*)o;
	if(file->fp != NULL) {
		int ret = fclose(file->fp);
		if(ret != 0) {
			// TODO: throw
			OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
					LogText("@", "fclose"),
					LogText("errstr", strerror(errno))
			);
		}
		file->fp = NULL;
	}
	if(file->realpath != NULL) {
		free((void*)file->realpath); // free path
		file->realpath = NULL;
	}
}

static void File_p(KonohaContext *kctx, KonohaValue *v, int pos, KGrowingBuffer *wb)
{
	kFILE *file = (kFILE*)v[pos].asObject;
	FILE *fp = file->fp;
	KLIB Kwb_printf(kctx, wb, "FILE :%p, path=%s", fp, file->realpath);
}

/* ------------------------------------------------------------------------ */
//## @Native @Throwable FILE System.fopen(String path, String mode);
static KMETHOD System_fopen(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s = sfp[1].asString;
	const char *mode = IS_NULL(sfp[2].asString) ? "r" : S_text(sfp[2].asString);
	FILE *fp = fopen(S_text(s), mode);
	DBG_P("fp=%p, filepath=%s", fp, S_text(s));
	if(fp == NULL) {
		// TODO: throw
		OLDTRACE_SWITCH_TO_KTrace(_SystemFault|_SoftwareFault,
				LogText("@", "fopen"),
				LogText("path", S_text(s)),
				LogUint("mode", mode),
				LogText("errstr", strerror(errno))
		);
	}
	struct kFILEVar *file = (struct kFILEVar*)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), (uintptr_t)fp);
	file->realpath = realpath(S_text(s), NULL);
	KReturn(file);
}

//## @Native int File.read(Bytes buf, int offset, int len);
static KMETHOD File_read(KonohaContext *kctx, KonohaStack *sfp)
{
	kFILE *file = (kFILE*)sfp[0].asObject;
	FILE *fp = file->fp;
	size_t size = 0;
	if(fp != NULL) {
		kBytes *ba = sfp[1].asBytes;
		size_t offset = (size_t)sfp[2].intValue;
		size_t len = (size_t)sfp[3].intValue;
		size = ba->bytesize;
		if(!(offset < size)) {
			KMakeTrace(trace, sfp);  // FIXME
			kreportf(CritTag, trace, "OutOfRange!!, offset=%d, size=%d", offset, size);
		}
		if(len == 0) len = size - offset;
		size = fread(ba->buf + offset, 1, len, fp);
		if(size == 0 && ferror(fp) != 0){
			// TODO: throw
			OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
					LogText("@", "fread"),
					LogText("errstr", strerror(errno))
			);
			clearerr(fp);
		}
	}
	KReturnUnboxValue(size);
}

//## @Native int File.write(Bytes buf, int offset, int len);
static KMETHOD File_write(KonohaContext *kctx , KonohaStack *sfp)
{
	kFILE *file = (kFILE*)sfp[0].asObject;
	FILE *fp = file->fp;
	size_t size = 0;
	if(fp != NULL) {
		kBytes *ba = sfp[1].asBytes;
		size_t offset = (size_t)sfp[2].intValue;
		size_t len = (size_t)sfp[3].intValue;
		size = ba->bytesize;
		if(len == 0) len = size - offset;
		size = fwrite(ba->buf + offset, 1, len, fp);
		if(size < len) {
			// TODO: throw
			OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
					LogText("@", "fwrite"),
					LogText("errstr", strerror(errno))
			);
		}
	}
	KReturnUnboxValue(size);
}

//## @Native void File.close();
static KMETHOD File_close(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kFILEVar *file = (struct kFILEVar*)sfp[0].asObject;
	FILE *fp = file->fp;
	if(fp != NULL) {
		int ret = fclose(fp);
		if(ret != 0) {
			// TODO: throw
			OLDTRACE_SWITCH_TO_KTrace(_SystemFault,
					LogText("@", "fclose"),
					LogText("errstr", strerror(errno))
			);
		}
		file->fp = NULL;
	}
	KReturnVoid();
}

//## @Native int File.getC();
static KMETHOD File_getC(KonohaContext *kctx, KonohaStack *sfp)
{
	FILE *fp = ((kFILE*)sfp[0].asObject)->fp;
	int ch = EOF;
	if(fp != NULL) {
		ch = fgetc(fp);
		if(ch == EOF && ferror(fp) != 0) {
			// TODO: throw
			OLDTRACE_SWITCH_TO_KTrace(LOGPOL_DEBUG | _DataFault,
					LogText("@", "fgetc"),
					LogText("errstr", strerror(errno))
			);
		}
	}
	KReturnUnboxValue(ch);
}

//## @Native boolean File.putC(int ch);
static KMETHOD File_putC(KonohaContext *kctx, KonohaStack *sfp)
{
	FILE *fp = ((kFILE*)sfp[0].asObject)->fp;
	if(fp != NULL) {
		int ch = fputc(sfp[1].intValue, fp);
		if(ch == EOF) {
			// TODO: throw
			OLDTRACE_SWITCH_TO_KTrace(LOGPOL_DEBUG | _DataFault,
					LogText("@", "fputc"),
					LogText("errstr", strerror(errno))
			);
		}
		KReturnUnboxValue(ch != EOF);
	}
	KReturnUnboxValue(0);
}

//## int System.umask(int cmask)
static KMETHOD System_umask(KonohaContext *kctx, KonohaStack *sfp)
{
	mode_t cmask = sfp[1].intValue;
	mode_t ret = umask(cmask);
	KReturnUnboxValue(ret);
}

//## int System.mkdir(String path, int mode)
static KMETHOD System_mkdir(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *path = S_text(sfp[1].asString);
	mode_t mode = sfp[2].intValue;
	int ret = mkdir(path, mode);
	if(ret == -1) {
		// TODO: throw
	}
	KReturnUnboxValue(ret);
}

//## int System.rmdir(String path)
static KMETHOD System_rmdir(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *path = S_text(sfp[1].asString);
	int ret = rmdir(path);
	if(ret == -1) {
		// TODO: throw
	}
	KReturnUnboxValue(ret);
}

//## int System.truncate(String path, int length)
static KMETHOD System_truncate(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *path = S_text(sfp[1].asString);
	off_t length = sfp[2].intValue;
	int ret = truncate(path, length);
	if(ret == -1) {
		// TODO: throw
	}
	KReturnUnboxValue(ret);
}

//## int System.chmod(String path, int mode)
static KMETHOD System_chmod(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *path = S_text(sfp[1].asString);
	mode_t mode = sfp[2].intValue;
	int ret = chmod(path, mode);
	if(ret == -1) {
		// TODO: throw
	}
	KReturnUnboxValue(ret);
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Static   kMethod_Static
#define _Coercion kMethod_Coercion
#define _Im kMethod_Immutable
#define _F(F)   (intptr_t)(F)

#define CT_File         cFile
#define TY_File         cFile->typeId
#define IS_File(O)      ((O)->h.ct == CT_File)

static kbool_t file_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	KImportPackage(ns, "konoha.bytes", trace);
	KDEFINE_CLASS defFile = {
		STRUCTNAME(FILE),
		.cflag = kClass_Final,
		.init  = File_init,
		.free  = File_free,
		.p     = File_p,
	};
	KonohaClass *cFile = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defFile, trace);

	KDEFINE_METHOD MethodData[] = {
		_Public|_Static|_Const|_Im, _F(System_fopen), TY_File, TY_System, MN_("fopen"), 2, TY_String, FN_("path"), TY_String, FN_("mode"),
		_Public|_Const|_Im, _F(File_close), TY_void, TY_File, MN_("close"), 0,
		_Public|_Const|_Im, _F(File_getC), TY_int, TY_File, MN_("getC"), 0,
		_Public|_Const|_Im, _F(File_putC), TY_boolean, TY_File, MN_("putC"), 1, TY_int, FN_("ch"),
		_Public|_Static|_Const|_Im, _F(System_umask), TY_int, TY_System, MN_("umask"), 1, TY_int, FN_("cmask"),
		_Public|_Static|_Const|_Im, _F(System_mkdir), TY_int, TY_System, MN_("mkdir"), 2, TY_String, FN_("path"), TY_int, FN_("mode"),
		_Public|_Static|_Const|_Im, _F(System_rmdir), TY_int, TY_System, MN_("rmdir"), 1, TY_String, FN_("path"),
		_Public|_Static|_Const|_Im, _F(System_truncate), TY_int, TY_System, MN_("truncate"), 2, TY_String, FN_("path"), TY_int, FN_("length"),
		_Public|_Static|_Const|_Im, _F(System_chmod), TY_int, TY_System, MN_("chmod"), 2, TY_String, FN_("path"), TY_int, FN_("mode"),
		// the function below uses Bytes
		// FIXME
//		_Public|_Const, _F(File_write), TY_int, TY_File, MN_("write"), 3, TY_Bytes, FN_("buf"), TY_int, FN_("offset"), TY_int, FN_("len"),
//		_Public|_Const, _F(File_read), TY_int, TY_File, MN_("read"), 3, TY_Bytes, FN_("buf"), TY_int, FN_("offset"), TY_int, FN_("len"),
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t file_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	return true;
}

static kbool_t file_initNameSpace(KonohaContext *kctx, kNameSpace *packageNS, kNameSpace *ns, KTraceInfo *trace)
{
	return true;
}

static kbool_t file_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNS, kNameSpace *ns, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* file_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("file", "1.0"),
		.initPackage    = file_initPackage,
		.setupPackage   = file_setupPackage,
		.initNameSpace  = file_initNameSpace,
		.setupNameSpace = file_setupNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
