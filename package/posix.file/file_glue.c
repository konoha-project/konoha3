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

#ifndef K_PATHMAX
#define K_PATHMAX 1024
#endif

typedef const struct kFileVar kFile;

struct kFileVar {
	KonohaObjectHeader h;
	FILE *fp;
	const char *realpath;
	uintptr_t readerIconv;
	uintptr_t writerIconv;
};

static int guessFaultFromErrno(KonohaContext *kctx, int fault)
{
	int foundErrno = errno;
	switch(errno) {  // C Standard Library
	case EDOM: /*Results from a parameter outside a function's domain, for example sqrt(-1) */
	case ERANGE: /* Results from a result outside a function's range, for example strtol("0xfffffffff",NULL,0) */
	case EILSEQ: /* Results from an illegal byte sequence */
		return fault | SoftwareFault;
	}

	switch(errno) {
	case EPERM:  /* 1. Operation not permitted (Linux) */
		return fault | SoftwareFault | DataFault;
	case ENOENT: /* 2. No such file or directory */
		return fault | SoftwareFault | DataFault;
	case ESRCH:  /* 3. No such process */
		return fault | SystemFault | SoftwareFault | DataFault;
	case EINTR: /* 4. Interrupted system call */
		return fault;
	case EIO: /* 5. I/O error */
		return fault | SystemFault ;
	case ENXIO:  /* 6. No such device or address */
		return fault | SoftwareFault | DataFault | SystemFault;
	case E2BIG:  /* 7. Arg list too long */
		return fault | SoftwareFault | DataFault;
	case ENOEXEC: /* 8. Exec format error */
		return fault | SystemFault;
	case EBADF:  /* 9. Bad file number */
		return fault | SoftwareFault;
	case ECHILD: /* 10. No child processes */
		return fault | SoftwareFault | SystemFault;
	case EAGAIN: /* 11. Try again */
		return 0;  /* not fault */
	case ENOMEM: /* 12. Out of memory */
		/* If you try to exec() another process or just ask for more memory in this process */
		return fault | SoftwareFault | SystemFault;
	case EACCES: /* 13. Permission denied */
		return fault | DataFault;
	case EFAULT: /* 14 Bad address */
		return fault | SoftwareFault; /* At the C-Level */
	case ENOTBLK: /* 15 Block device required */
		return fault | SystemFault; /* in case of unmount device */
	case EBUSY: /* 16 Device or resource busy */
		return fault | SystemFault;
	case EEXIST: /*17 File exists */
		return fault | SoftwareFault | DataFault | SystemFault;
	case EXDEV:  /* 18. Cross-device link */
		return fault | SystemFault;
	case ENODEV: /* 19 No such device */
	case ENOTDIR: /*20 Not a directory */
	case EISDIR: /* 21 Is a directory */
		return fault | SoftwareFault | DataFault | SystemFault;
	case EINVAL: /* 22 Invalid argument */
		/* EINVAL gets used a lot.
		 * TCP has the concept of "out of band data" (urgent data).
		 * If a reading process checks for this, and there isn't any, it get EINVAL.
		 * The plock() function ( which locks areas of a process into memory) returns this
		 * if you attempt to use it twice on the same memory segment.
		 * If you try to specify SIGKILL or SIGSTOP to sigaction(), you'll get this return.
		 * The readv() and writev() calls complain this way if you give them too large an array of buffers.
		 * As mentioned above, drivers may return this for inappropriate ioctl() calls.
		 * The mmap() call will return this if you've specified a specific address but that address can't be used.
		 * A seek() to before the beginning of a file returns this.
		 * Streams use this if you attempt to link a stream onto itself. It's used for many IPC errors also. */
		return fault | SoftwareFault | DataFault | SystemFault;
	case ENFILE:  /* 23. File table overflow */
		return fault | SoftwareFault | DataFault | SystemFault;
	case EMFILE: /* 24. Too many open files */
		return fault | SoftwareFault;
	case ENOTTY: /* 25 Not a typewriter */
		return fault | SoftwareFault | DataFault | SystemFault;
	case ETXTBSY:  /* 26 Text file busy */
		/* It's illegal to write to a binary while it is executing- */
		return fault | SoftwareFault;
	case EFBIG: /* 27 File too large */
		return fault | SoftwareFault;
	case ENOSPC: /* 28 No space left on device */
	case ESPIPE: /* 29 Illegal seek */
		return fault | SoftwareFault;
	case EROFS:  /* 30 Read-only file system */
		return fault | SoftwareFault | DataFault;
	case EMLINK: /* 31 Too many links */
		return fault | SoftwareFault | DataFault;
	case EPIPE: /* 32 Broken pipe */
		return fault | SystemFault | DataFault;
	}
	return fault;
}

//static int diagnosisFaultType(KonohaContext *kctx, int fault, KTraceInfo *trace)
//{
//	if(TFLAG_is(int, fault, SystemError)) {
//		fault =
//	}
//}

/* ------------------------------------------------------------------------ */

static void File_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct kFileVar *file = (struct kFileVar *)o;
	file->fp = (conf != NULL) ? conf : NULL;
	file->realpath = NULL;
	file->readerIconv = ICONV_NULL;
	file->writerIconv = ICONV_NULL;
}

static void File_free(KonohaContext *kctx, kObject *o)
{
	struct kFileVar *file = (struct kFileVar *)o;
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
		free((void *)file->realpath); // free path
		file->realpath = NULL;
	}
}

static void File_p(KonohaContext *kctx, KonohaValue *v, int pos, KGrowingBuffer *wb)
{
	kFile *file = (kFile *)v[pos].asObject;
	FILE *fp = file->fp;
	KLIB Kwb_printf(kctx, wb, "FILE :%p, path=%s", fp, file->realpath);
}

/* ------------------------------------------------------------------------ */
//## @Native @Throwable FILE System.fopen(String path, String mode);

static KMETHOD Libc_fopen(KonohaContext *kctx, KonohaStack *sfp)
{
	char buffer[K_PATHMAX];
	KMakeTrace(trace, sfp);  // FIXME
	const char *systemPath = PLATAPI formatSystemPath(kctx, buffer, K_PATHMAX, S_text(sfp[1].asString), S_size(sfp[1].asString), trace);
	const char *mode = S_text(sfp[2].asString);
	FILE *fp = fopen(systemPath, mode);
	if(fp == NULL) {
		KTraceErrorPoint(trace, SoftwareFault|SystemFault|DataFault, "fopen",
			LogText("filename", S_text(sfp[1].asString)), LogText("mode", mode), LogErrno);
		KLIB KonohaRuntime_raise(kctx, EXPT_("IO"), SoftwareFault|SystemFault|DataFault, NULL, sfp);
	}
	struct kFileVar *file = (struct kFileVar *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), (uintptr_t)fp);
	file->realpath = realpath(systemPath, NULL);
	KReturn(file);
}

static void MethodLib_FileRead(KonohaContext *kctx, KonohaStack *sfp, kFile *file, kBytes *ba, size_t offset, size_t len)
{
	KCheckIndex(offset, ba->bytesize);
	KCheckIndex(offset+len, ba->bytesize);
	size_t size = fread(ba->buf + offset, 1, len, file->fp);
	if(ferror(file->fp) != 0){
		KMakeTrace(trace, sfp);
		KTraceErrorPoint(trace, SystemFault, "fread",
			LogText("filename", file->realpath), LogErrno);
		KLIB KonohaRuntime_raise(kctx, EXPT_("IO"), SystemFault, NULL, sfp);
	}
	KReturnUnboxValue(size);
}

//## int File.read(Bytes buf, int offset, int len);
static KMETHOD File_read3(KonohaContext *kctx, KonohaStack *sfp)
{
	MethodLib_FileRead(kctx, sfp, (kFile*)sfp[0].asObject, sfp[1].asBytes, sfp[2].intValue, sfp[3].intValue);
}

//## String File.readLine();
static KMETHOD File_readLine(KonohaContext *kctx, KonohaStack *sfp)
{
	kFile *file = (kFile *)sfp[0].asObject;
	FILE *fp = file->fp;
	KGrowingBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	int ch, pos = 0, hasUTF8 = false, bufferCount = 0, policy = StringPolicy_ASCII;
	char buffer[K_PAGESIZE];
	KMakeTrace(trace, sfp);
	while((ch == fgetc(fp)) != EOF) {
		if(ferror(fp) != 0) {
			KTraceErrorPoint(trace, SoftwareFault|SystemFault, "fgetc",
				LogText("filename", file->realpath), LogErrno);
			KLIB KonohaRuntime_raise(kctx, EXPT_("IO"), SoftwareFault|SystemFault, NULL, sfp);
		}
		if(ch == '\r') continue;
		if(ch == '\n') {
			if(bufferCount == 0 && (!hasUTF8 || file->readerIconv == ICONV_NULL)) {
				KReturn(KLIB new_kString(kctx, OnStack, buffer, pos, policy));
			}
			break;
		}
		if(ch > 127) {
			hasUTF8 = true;
			policy = StringPolicy_UTF8;
		}
		buffer[pos] = ch; pos++;
		if(!(pos < K_PAGESIZE)) {
			if(hasUTF8 && file->readerIconv != ICONV_NULL) {
				KLIB Kwb_iconv(kctx, &wb, file->readerIconv, buffer, pos, trace);
			}
			else {
				KLIB Kwb_write(kctx, &wb, buffer, pos);
			}
			bufferCount++;
			hasUTF8 = false;
			pos = 0;
		}
	}
	if(pos > 0) {
		if(hasUTF8 && file->readerIconv != ICONV_NULL) {
			KLIB Kwb_iconv(kctx, &wb, file->readerIconv, buffer, pos, trace);
		}
		else {
			KLIB Kwb_write(kctx, &wb, buffer, pos);
		}
	}
	KReturnWith(
		KLIB new_kString(kctx, OnStack, KLIB Kwb_top(kctx, &wb, 0), Kwb_bytesize(&wb), policy),
		KLIB Kwb_free(&wb)
	);
}


static void MethodLib_FileWrite(KonohaContext *kctx, KonohaStack *sfp, kFile *file, kBytes *ba, size_t offset, size_t len)
{
	KCheckIndex(offset, ba->bytesize);
	KCheckIndex(offset+len, ba->bytesize);
	size_t size = fwrite(ba->buf + offset, 1, len, file->fp);
	if(ferror(file->fp) != 0){
		KMakeTrace(trace, sfp);
		KTraceErrorPoint(trace, SystemFault, "fwrite",
			LogText("filename", file->realpath), LogErrno);
		KLIB KonohaRuntime_raise(kctx, EXPT_("IO"), SystemFault, NULL, sfp);
	}
	KReturnUnboxValue(size);
}

//## @Native int File.write(Bytes buf, int offset, int len);
static KMETHOD File_write3(KonohaContext *kctx , KonohaStack *sfp)
{
	MethodLib_FileWrite(kctx, sfp, (kFile*)sfp[0].asObject, sfp[1].asBytes, sfp[2].intValue, sfp[3].intValue);
}

//## @Native void File.close();
static KMETHOD File_close(KonohaContext *kctx, KonohaStack *sfp)
{
	struct kFileVar *file = (struct kFileVar *)sfp[0].asObject;
	FILE *fp = file->fp;
	if(fp != NULL) {
		int ret = fclose(fp);
		if(ret != 0) {
			KMakeTrace(trace, sfp);
			KTraceErrorPoint(trace, SystemFault, "fclose", LogText("filename", file->realpath), LogErrno);
			KLIB KonohaRuntime_raise(kctx, EXPT_("IO"), SystemFault, NULL, sfp);
		}
		file->fp = NULL;
	}
	KReturnVoid();
}

//## @Native int File.getC();
static KMETHOD File_getC(KonohaContext *kctx, KonohaStack *sfp)
{
	FILE *fp = ((kFile *)sfp[0].asObject)->fp;
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
	FILE *fp = ((kFile *)sfp[0].asObject)->fp;
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

#define TY_Bytes        cBytes->typeId

static kbool_t file_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	KImportPackage(ns, "konoha.bytes", trace);
	KDEFINE_CLASS defFile = {
		.structname = "FILE",
		.typeId = TY_newid,
		.cstruct_size = sizeof(kFile),
		.cflag = kClass_Final,
		.init  = File_init,
		.free  = File_free,
		.p     = File_p,
	};
	KonohaClass *cFile = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defFile, trace);
	KonohaClass *cBytes = KLIB kNameSpace_getClass(kctx, ns, "konoha.bytes.Bytes", strlen("konoha.bytes.Bytes"), NULL);
	
	KDEFINE_METHOD MethodData[] = {
		_Public|_Static|_Const|_Im, _F(Libc_fopen), TY_File, TY_System, MN_("fopen"), 2, TY_String, FN_("path"), TY_String, FN_("mode"),
		_Public|_Const|_Im, _F(File_close), TY_void, TY_File, MN_("close"), 0,
		_Public|_Const|_Im, _F(File_getC), TY_int, TY_File, MN_("getC"), 0,
		_Public|_Const|_Im, _F(File_putC), TY_boolean, TY_File, MN_("putC"), 1, TY_int, FN_("ch"),
		_Public|_Static|_Const|_Im, _F(System_umask), TY_int, TY_System, MN_("umask"), 1, TY_int, FN_("cmask"),
		_Public|_Static|_Const|_Im, _F(System_mkdir), TY_int, TY_System, MN_("mkdir"), 2, TY_String, FN_("path"), TY_int, FN_("mode"),
		_Public|_Static|_Const|_Im, _F(System_rmdir), TY_int, TY_System, MN_("rmdir"), 1, TY_String, FN_("path"),
		_Public|_Static|_Im, _F(System_truncate), TY_int, TY_System, MN_("truncate"), 2, TY_String, FN_("path"), TY_int, FN_("length"),
		_Public|_Static|_Im, _F(System_chmod), TY_int, TY_System, MN_("chmod"), 2, TY_String, FN_("path"), TY_int, FN_("mode"),
		_Public, _F(File_readLine), TY_String, TY_File, MN_("readLine"), 0,
		// the function below uses Bytes
		_Public|_Im, _F(File_write3), TY_int, TY_File, MN_("write"), 3, TY_Bytes, FN_("buf"), TY_int, FN_("offset"), TY_int, FN_("len"),
		_Public|_Im, _F(File_read3), TY_int, TY_File, MN_("read"), 3, TY_Bytes, FN_("buf"), TY_int, FN_("offset"), TY_int, FN_("len"),
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t file_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
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
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
