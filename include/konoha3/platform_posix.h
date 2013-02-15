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

#ifndef PLATFORM_POSIX_H_
#define PLATFORM_POSIX_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef K_USE_PTHREAD
#include <pthread.h>
#endif

#ifndef K_OSDLLEXT
#if defined(__APPLE__)
#define K_OSDLLEXT        ".dylib"
#elif defined(__linux__)
#define K_OSDLLEXT        ".so"
#endif
#endif

#ifdef PATH_MAX
#define K_PATHMAX PATH_MAX
#else
#define K_PATHMAX 256
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <sys/time.h>
#include <syslog.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#define kunused __attribute__((unused))

#include <konoha3/klib.h>

#ifndef K_PREFIX
#define K_PREFIX  "/usr/local"
#endif

// -------------------------------------------------------------------------
/* LoadPlatformModule */

static kbool_t HasFile(char *path)
{
	FILE *fp = fopen(path, "r");
	if(fp != NULL) {
		fclose(fp);
		return true;
	}
	return false;
}

static kbool_t FormatModulePath(KonohaFactory *factory, char *buf, size_t bufsiz, const char *moduleName, const char *ext)
{
	const char *path = factory->getenv_i("KONOHA_HOME");
	const char *local = "/module";
	if(path == NULL) {
		path = factory->getenv_i("HOME");
		local = "/.konoha/module";
	}
	snprintf(buf, bufsiz, "%s%s/%s/%s%s", path, local, moduleName, moduleName, ext);
#ifdef K_PREFIX
	if(!HasFile(buf)) {
		snprintf(buf, bufsiz, K_PREFIX "/lib/konoha/" K_VERSION "/module" "/%s/%s%s", moduleName, moduleName, ext);
	}
#endif
	return HasFile(buf);
}

static kbool_t LoadPlatformModule(KonohaFactory *factory, const char *moduleName, ModuleType type)
{
	char pathbuf[K_PATHMAX];
	if(FormatModulePath(factory, pathbuf, sizeof(pathbuf), moduleName, K_OSDLLEXT)) {
		void *gluehdr = dlopen(pathbuf, RTLD_LAZY);  // don't close until the program ends
		if(gluehdr != NULL) {
			char funcbuf[256];
			snprintf(funcbuf, sizeof(funcbuf), "Load%sModule", moduleName);
			ModuleLoadFunc load = (ModuleLoadFunc)dlsym(gluehdr, funcbuf);
			if(load != NULL) {
				return load(factory, type);
			}
		}
	}
	return false;
}

// -------------------------------------------------------------------------
/* Package */

static const char* ShortPackageName(const char *str)
{
	char *p = (char *) strrchr(str, '.');
	return (p == NULL) ? str : (const char *)p+1;
}

static const char* FormatPackagePath(KonohaContext *kctx, char *buf, size_t bufsiz, const char *packageName, const char *ext)
{
	const char *path = PLATAPI getenv_i("KONOHA_HOME");
	const char *local = "/package";
	if(path == NULL) {
		path = PLATAPI getenv_i("HOME");
		local = "/.konoha/package";
	}
	snprintf(buf, bufsiz, "%s%s/%s/%s%s", path, local, packageName, ShortPackageName(packageName), ext);
#ifdef K_PREFIX
	if(!HasFile(buf)) {
		snprintf(buf, bufsiz, K_PREFIX "/lib/konoha/" K_VERSION "/package" "/%s/%s%s", packageName, ShortPackageName(packageName), ext);
	}
#endif
	return HasFile(buf) ? (const char *)buf : NULL;
}

static KPackageHandler *LoadPackageHandler(KonohaContext *kctx, const char *packageName)
{
	char pathbuf[256];
	FormatPackagePath(kctx, pathbuf, sizeof(pathbuf), packageName, "_glue" K_OSDLLEXT);
	void *gluehdr = dlopen(pathbuf, RTLD_LAZY);
	if(gluehdr != NULL) {
		char funcbuf[80];
		snprintf(funcbuf, sizeof(funcbuf), "%s_Init", ShortPackageName(packageName));
		PackageLoadFunc f = (PackageLoadFunc)dlsym(gluehdr, funcbuf);
		if(f != NULL) {
			return f();
		}
	}
	return NULL;
}

static void BEFORE_LoadScript(KonohaContext *kctx, const char *filename) { }
static void AFTER_LoadScript(KonohaContext *kctx, const char *filename)  { }

// -------------------------------------------------------------------------
/* I18N */

static uintptr_t I18N_iconv_open(KonohaContext *kctx, const char *targetCharset, const char *sourceCharset, KTraceInfo *trace)
{
	return ICONV_NULL;
}

static size_t I18N_iconv(KonohaContext *kctx, uintptr_t ic, ICONV_INBUF_CONST char **inbuf, size_t *inBytesLeft, char **outbuf, size_t *outBytesLeft, int *isTooBigSourceRef, KTraceInfo *trace)
{
	return -1;
}

static int I18N_iconv_close(KonohaContext *kctx, uintptr_t ic)
{
	return -1;
}

static kbool_t I18N_isSystemCharsetUTF8(KonohaContext *kctx)
{
	return true;
}

static uintptr_t I18N_iconvSystemCharsetToUTF8(KonohaContext *kctx, KTraceInfo *trace)
{
	return ICONV_NULL;
}

static uintptr_t I18N_iconvUTF8ToSystemCharset(KonohaContext *kctx, KTraceInfo *trace)
{
	return ICONV_NULL;
}

static const char* I18N_formatKonohaPath(KonohaContext *kctx, char *buf, size_t bufsiz, const char *path, size_t pathsize, KTraceInfo *trace)
{
	return path;
}

static const char* I18N_formatSystemPath(KonohaContext *kctx, char *buf, size_t bufsiz, const char *path, size_t pathsize, KTraceInfo *trace)
{
	return path;  // stub (in case of no conversion)
}

static void loadI18N(KonohaFactory *plat, const char *defaultCharSet)
{
	plat->I18NModule.systemCharset  = (defaultCharSet == NULL) ? "UTF-8" : defaultCharSet;
	plat->I18NModule.iconv_open_i   = I18N_iconv_open;
	plat->I18NModule.iconv_i        = I18N_iconv;
	plat->I18NModule.iconv_close_i  = I18N_iconv_close;
	plat->I18NModule.isSystemCharsetUTF8 = I18N_isSystemCharsetUTF8;
	plat->I18NModule.iconvSystemCharsetToUTF8 = I18N_iconvSystemCharsetToUTF8;
	plat->I18NModule.iconvUTF8ToSystemCharset = I18N_iconvUTF8ToSystemCharset;
	plat->I18NModule.formatKonohaPath = I18N_formatKonohaPath;
	plat->I18NModule.formatSystemPath = I18N_formatSystemPath;
}

// -------------------------------------------------------------------------

static unsigned long long getTimeMilliSecond(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// -------------------------------------------------------------------------

static int kpthread_mutex_init_recursive(kmutex_t *mutex)
{
#ifdef K_USE_PTHREAD
	pthread_mutexattr_t attr;
	bzero(&attr, sizeof(pthread_mutexattr_t));
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	return pthread_mutex_init((pthread_mutex_t *)mutex, &attr);
#else
	return 0;
#endif
}

#ifdef K_USE_PTHREAD

static int kpthread_create(kthread_t *thread, const kthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
	return pthread_create(thread, attr, start_routine, arg);
}

static int kpthread_join(kthread_t thread, void **value_ptr)
{
	return pthread_join(thread, value_ptr);
}

static int kpthread_mutex_destroy(kmutex_t *mutex)
{
	return pthread_mutex_destroy(mutex);
}

static int kpthread_mutex_init(kmutex_t *mutex, const kmutexattr_t *attr)
{
	return pthread_mutex_init(mutex, attr);
}

static int kpthread_mutex_lock(kmutex_t *mutex)
{
	return pthread_mutex_lock(mutex);
}

static int kpthread_mutex_trylock(kmutex_t *mutex)
{
	return pthread_mutex_trylock(mutex);
}

static int kpthread_mutex_unlock(kmutex_t *mutex)
{
	return pthread_mutex_unlock(mutex);
}

static int kpthread_cond_init(kmutex_cond_t *cond, const kmutex_condattr_t *attr)
{
	return pthread_cond_init(cond, attr);
}

static int kpthread_cond_wait(kmutex_cond_t *cond, kmutex_t *mutex)
{
	return pthread_cond_wait(cond, mutex);
}

static int kpthread_cond_signal(kmutex_cond_t *cond)
{
	return pthread_cond_signal(cond);
}

static int kpthread_cond_broadcast(kmutex_cond_t *cond)
{
	return pthread_cond_broadcast(cond);
}

static int kpthread_cond_destroy(kmutex_cond_t *cond)
{
	return pthread_cond_destroy(cond);
}

#else

static int kpthread_create(kthread_t *thread, const kthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
	return 0;
}

static int kpthread_join(kthread_t thread, void **value_ptr)
{
	return 0;
}

static int kpthread_mutex_destroy(kmutex_t *mutex)
{
	return 0;
}

static int kpthread_mutex_init(kmutex_t *mutex, const kmutexattr_t *attr)
{
	return 0;
}

static int kpthread_mutex_lock(kmutex_t *mutex)
{
	return 0;
}

static int kpthread_mutex_trylock(kmutex_t *mutex)
{
	return 0;
}

static int kpthread_mutex_unlock(kmutex_t *mutex)
{
	return 0;
}

static int kpthread_cond_init(kmutex_cond_t *cond, const kmutex_condattr_t *attr)
{
	return 0;
}

static int kpthread_cond_wait(kmutex_cond_t *cond, kmutex_t *mutex)
{
	return 0;
}

static int kpthread_cond_signal(kmutex_cond_t *cond)
{
	return 0;
}

static int kpthread_cond_broadcast(kmutex_cond_t *cond)
{
	return 0;
}

static int kpthread_cond_destroy(kmutex_cond_t *cond)
{
	return 0;
}

#endif

// -------------------------------------------------------------------------

static kbool_t isDir(const char *path)
{
	struct stat buf;
//	char pathbuf[K_PATHMAX];
//	if(stat(I18N_formatSystemPath(pathbuf, sizeof(pathbuf), path), &buf) == 0) {
	if(stat(path, &buf) == 0) {
		return S_ISDIR(buf.st_mode);
	}
	return false;
}

// -------------------------------------------------------------------------

typedef struct {
	char   *buffer;
	size_t  size;
	size_t  allocSize;
} SimpleBuffer;

static void SimpleBuffer_putc(SimpleBuffer *simpleBuffer, int ch)
{
	if(!(simpleBuffer->size < simpleBuffer->allocSize)) {
		simpleBuffer->allocSize *= 2;
		simpleBuffer->buffer = (char *)realloc(simpleBuffer->buffer, simpleBuffer->allocSize);
	}
	simpleBuffer->buffer[simpleBuffer->size] = ch;
	simpleBuffer->size += 1;
}

static kfileline_t readQuote(FILE *fp, kfileline_t line, SimpleBuffer *simpleBuffer, int quote)
{
	int ch, prev = quote;
	while((ch = fgetc(fp)) != EOF) {
		if(ch == '\r') continue;
		if(ch == '\n') line++;
		SimpleBuffer_putc(simpleBuffer, ch);
		if(ch == quote && prev != '\\') {
			return line;
		}
		prev = ch;
	}
	return line;
}

static kfileline_t readComment(FILE *fp, kfileline_t line, SimpleBuffer *simpleBuffer)
{
	int ch, prev = 0, level = 1;
	while((ch = fgetc(fp)) != EOF) {
		if(ch == '\r') continue;
		if(ch == '\n') line++;
		SimpleBuffer_putc(simpleBuffer, ch);
		if(prev == '*' && ch == '/') level--;
		if(prev == '/' && ch == '*') level++;
		if(level == 0) return line;
		prev = ch;
	}
	return line;
}

static kfileline_t readChunk(FILE *fp, kfileline_t line, SimpleBuffer *simpleBuffer)
{
	int ch;
	int prev = 0, isBLOCK = 0;
	while((ch = fgetc(fp)) != EOF) {
		if(ch == '\r') continue;
		if(ch == '\n') line++;
		SimpleBuffer_putc(simpleBuffer, ch);
		if(prev == '/' && ch == '*') {
			line = readComment(fp, line, simpleBuffer);
			continue;
		}
		if(ch == '\'' || ch == '"' || ch == '`') {
			line = readQuote(fp, line, simpleBuffer, ch);
			continue;
		}
		if(isBLOCK != 1 && prev == '\n' && ch == '\n') {
			break;
		}
		if(prev == '{') {
			isBLOCK = 1;
		}
		if(prev == '\n' && ch == '}') {
			isBLOCK = 0;
		}
		prev = ch;
	}
	return line;
}

static int isEmptyChunk(const char *t, size_t len)
{
	size_t i;
	for(i = 0; i < len; i++) {
		if(!isspace(t[i])) return false;
	}
	return true;
}

static int loadScript(const char *filePath, long uline, void *thunk, int (*evalFunc)(const char*, long, int *, void *))
{
	int isSuccessfullyLoading = false;
	if(isDir(filePath)) {
		return isSuccessfullyLoading;
	}
	FILE *fp = fopen(filePath, "r");
	if(fp != NULL) {
		SimpleBuffer simpleBuffer;
		simpleBuffer.buffer = (char *)malloc(K_PAGESIZE);
		simpleBuffer.allocSize = K_PAGESIZE;
		isSuccessfullyLoading = true;
		while(!feof(fp)) {
			kfileline_t rangeheadline = uline;
			kshort_t sline = (kshort_t)uline;
			bzero(simpleBuffer.buffer, simpleBuffer.allocSize);
			simpleBuffer.size = 0;
			uline = readChunk(fp, uline, &simpleBuffer);
			const char *script = (const char *)simpleBuffer.buffer;
			if(sline == 1 && simpleBuffer.size > 2 && script[0] == '#' && script[1] == '!') {
				// fall through this line
				simpleBuffer.size = 0;
				//TODO: do we increment uline??
			}
			if(!isEmptyChunk(script, simpleBuffer.size)) {
				int isBreak = false;
				isSuccessfullyLoading = evalFunc(script, rangeheadline, &isBreak, thunk);
				if(!isSuccessfullyLoading|| isBreak) {
					break;
				}
			}
		}
		free(simpleBuffer.buffer);
		fclose(fp);
	}
	return isSuccessfullyLoading;
}

static const char* shortFilePath(const char *path)
{
	char *p = (char *) strrchr(path, '/');
	return (p == NULL) ? path : (const char *)p+1;
}

//static const char* shortText(const char *msg)
//{
//	return msg;
//}

static const char *formatTransparentPath(char *buf, size_t bufsiz, const char *parentPath, const char *path)
{
	const char *p = strrchr(parentPath, '/');
	if(p != NULL && path[0] != '/') {
		size_t len = (p - parentPath) + 1;
		if(len < bufsiz) {
			memcpy(buf, parentPath, len);
			snprintf(buf + len, bufsiz - len, "%s", path);
			return (const char *)buf;
		}
	}
	return path;
}


static void ReportDebugMessage(const char *file, const char *func, int line, const char *fmt, ...)
{
	va_list ap;
	va_start(ap , fmt);
	fflush(stdout);
	fprintf(stderr, "DEBUG(%s:%d) ", func, line);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

static void NOP_ReportDebugMessage(const char *file, const char *func, int line, const char *fmt, ...)
{

}

// --------------------------------------------------------------------------

#include "libcode/libc_readline.h"

static void PlatformApi_loadReadline(KonohaFactory *plat)
{
	void *handler = dlopen("libreadline" K_OSDLLEXT, RTLD_LAZY);
	if(handler != NULL) {
		plat->readline_i = (char* (*)(const char *))dlsym(handler, "readline");
		plat->add_history_i = (int (*)(const char *))dlsym(handler, "add_history");
	}
	if(plat->readline_i == NULL) {
		plat->readline_i = readline;
	}
	if(plat->add_history_i == NULL) {
		plat->add_history_i = add_history;
	}
}

static kunused int DEOS_guessFaultFromErrno(KonohaContext *kctx, int userFault)
{
	switch(errno) {  // C Standard Library
	case EILSEQ: /* Results from an illegal byte sequence */
		return userFault | SoftwareFault;
	}

	switch(errno) {
	case EPERM:  /* 1. Operation not permitted (Linux) */
		return userFault | SoftwareFault;
	case ENOENT: /* 2. No such file or directory */
		return userFault | SoftwareFault | SystemFault;
	case ESRCH:  /* 3. No such process */
		return userFault | SystemFault | SoftwareFault;
	case EINTR: /* 4. Interrupted system call */
		return SystemFault;
	case EIO: /* 5. I/O error */
		return SystemFault ;
	case ENXIO:  /* 6. No such device or address */
		return userFault | SoftwareFault | SystemFault;
	case E2BIG:  /* 7. Arg list too long */
		return userFault | SoftwareFault | UserFault;
	case ENOEXEC: /* 8. Exec format error */
		return SystemFault;
	case EBADF:  /* 9. Bad file number */
		return SoftwareFault;
	case ECHILD: /* 10. No child processes */
		return userFault | SoftwareFault | SystemFault;
	case EAGAIN: /* 11. Try again */
		return userFault;  /* not fault */
	case ENOMEM: /* 12. Out of memory */
		/* If you try to exec() another process or just ask for more memory in this process */
		return userFault | SoftwareFault | SystemFault;
	case EACCES: /* 13. Permission denied */
		return UserFault | SystemFault;  /* running software can be wrong.. */
	case EFAULT: /* 14 Bad address */
		return userFault | SoftwareFault; /* At the C-Level */
	case ENOTBLK: /* 15 Node device required */
		return SystemFault; /* in case of unmount device */
	case EBUSY: /* 16 Device or resource busy */
		return SystemFault;
	case EEXIST: /*17 File exists */
		return userFault | SoftwareFault | SystemFault;
	case EXDEV:  /* 18. Cross-device link */
		return SystemFault;
	case ENODEV: /* 19 No such device */
	case ENOTDIR: /*20 Not a directory */
	case EISDIR: /* 21 Is a directory */
		return userFault | SoftwareFault | SystemFault;
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
		return SoftwareFault;
	case ENFILE:  /* 23. File table overflow */
		return userFault | SoftwareFault | SystemFault;
	case EMFILE: /* 24. Too many open files */
		return userFault | SoftwareFault;
	case ENOTTY: /* 25 Not a typewriter */
		return userFault | SoftwareFault | SystemFault;
	case ETXTBSY:  /* 26 Text file busy */
		/* It's illegal to write to a binary while it is executing- */
		return userFault | SoftwareFault;
	case EFBIG: /* 27 File too large */
		return userFault | SoftwareFault;
	case ENOSPC: /* 28 No space left on device */
		return SoftwareFault | SystemFault;
	case ESPIPE: /* 29 Illegal seek */
		return userFault | SoftwareFault;
	case EROFS:  /* 30 Read-only file system */
		return userFault | SoftwareFault;
	case EMLINK: /* 31 Too many links */
		return userFault | SoftwareFault;
	case EPIPE: /* 32 Broken pipe */
		return userFault | SystemFault | UserFault;
	case EDOM: /* 33 Math argument out of domain of func */
		/*Results from a parameter outside a function's domain, for example sqrt(-1) */
	case ERANGE: /* 34 Math result not representable */
		/* Results from a result outside a function's range, for example strtol("0xfffffffff",NULL,0) */
		return userFault | SoftwareFault;
	case EDEADLK:       /* 35 Resource deadlock would occur */
		return SoftwareFault |SystemFault;
	case ENAMETOOLONG:  /* 36 File name too long */
		return userFault | SoftwareFault;
	case ENOLCK: /* 37 No record locks available UNSURE*/
	case ENOSYS: /* 38 Function not implemented  UNSHRE*/
	case ENOTEMPTY: /* 39 Directory not empty */
		return userFault | SoftwareFault | SystemFault;
	case ELOOP: /* 40 Too many symbolic links encountered */
		return SystemFault;
		/*** ***/
	//case EWOULDBLOCK: /* Operation would block */
	case ENOMSG:   /* No message of desired type */
	case EIDRM:    /* Identifier removed */
	//case ECHRNG:   /* Channel number out of range */
	//case EL2NSYNC: /* Level 2 not synchronized */
	//case EL3HLT:   /* Level 3 halted */
	//case EL3RST:   /* Level 3 reset */
	//case ELNRNG:   /* Link number out of range */
	//case EUNATCH:  /* Protocol driver not attached */
	//case ENOCSI:   /* No CSI structure available */
	//case EL2HLT:   /* Level 2 halted */
	//case EBADE:    /* Invalid exchange */
	//case EBADR:    /* Invalid request descriptor */
	//case EXFULL:   /* Exchange full */
	//case ENOANO:   /* No anode */
	//case EBADRQC:  /* Invalid request code */
	//case EBADSLT:  /* Invalid slot */
	//case EDEADLOCK:
	//case EBFONT:   /* Bad font file format */
	case ENOSTR:   /* Device not a stream */
	case ENODATA:  /* No data available */
	case ETIME:    /* Timer expired */
	case ENOSR:    /* Out of streams resources */
	//case ENONET:   /* Machine is not on the network */
	//case ENOPKG:   /* Package not installed */
	case EREMOTE:  /* Object is remote */
	case ENOLINK:  /* Link has been severed */
	//case EADV:     /* Advertise error */
	//case ESRMNT:   /* Srmount error */
	//case ECOMM:    /* Communication error on send */
	case EPROTO:   /* Protocol error */
	case EMULTIHOP: /* Multihop attempted */
	//case EDOTDOT:   /* RFS specific error */
	case EBADMSG:   /* Not a data message */
	case EOVERFLOW: /* Value too large for defined data type */
	//case ENOTUNIQ: /* Name not unique on network */
	//case EBADFD:  /* File descriptor in bad state */
	//case EREMCHG: /* Remote address changed */
	//case ELIBACC: /* Can not access a needed shared library */
	//case ELIBBAD: /* Accessing a corrupted shared library */
	//case ELIBSCN: /* .lib section in a.out corrupted */
	//case ELIBMAX: /* Attempting to link in too many shared libraries */
	//case ELIBEXEC:/* Cannot exec a shared library directly */
	case EILSEQ:  /* Illegal byte sequence */
	//case ERESTART:/* Interrupted system call should be restarted */
	//case ESTRPIPE:/* Streams pipe error */
	case EUSERS:  /* Too many users */
	case ENOTSOCK: /* Socket operation on non-socket */
	case EDESTADDRREQ: /* Destination address required */
	case EMSGSIZE:    /* Message too long */
	case EPROTOTYPE:  /* Protocol wrong type for socket */
	case ENOPROTOOPT: /* Protocol not available */
	case EPROTONOSUPPORT: /* Protocol not supported */
	case ESOCKTNOSUPPORT: /* Socket type not supported */
	case EOPNOTSUPP:   /* Operation not supported on transport endpoint */
	case EPFNOSUPPORT: /* Protocol family not supported */
	case EAFNOSUPPORT: /* Address family not supported by protocol */
	case EADDRINUSE:   /* Address already in use */
	case EADDRNOTAVAIL: /* Cannot assign requested address */
	case ENETDOWN:    /* Network is down */
	case ENETUNREACH: /* Network is unreachable */
	case ENETRESET:   /* Network dropped connection because of reset */
	case ECONNABORTED: /* Software caused connection abort */
	case ECONNRESET:/* Connection reset by peer */
	case ENOBUFS:   /* No buffer space available */
	case EISCONN:   /* Transport endpoint is already connected */
	case ENOTCONN:  /* Transport endpoint is not connected */
	case ESHUTDOWN: /* Cannot send after transport endpoint shutdown */
	case ETOOMANYREFS: /* Too many references: cannot splice */
	case ETIMEDOUT: /* Connection timed out */
	case ECONNREFUSED: /* Connection refused */
	case EHOSTDOWN: /* Host is down */
	case EHOSTUNREACH: /* No route to host */
	case EALREADY: /* Operation already in progress */
	case EINPROGRESS: /* Operation now in progress */
	case ESTALE:  /* Stale NFS file handle */
	//case EUCLEAN: /* Structure needs cleaning */
	//case ENOTNAM: /* Not a XENIX named type file */
	//case ENAVAIL: /* No XENIX semaphores available */
	//case EISNAM:  /* Is a named type file */
	//case EREMOTEIO: /* Remote I/O error */
	case EDQUOT:  /* Quota exceeded */
	//case ENOMEDIUM: /* No medium found */
	//case EMEDIUMTYPE: /* Wrong medium type */
		break;

		/*** ***/


	}
	return userFault | SoftwareFault |SystemFault;
}

// --------------------------------------------------------------------------

static kunused void diagnosis(void)
{

}

// --------------------------------------------------------------------------

static const char* BeginTag(KonohaContext *kctx, kinfotag_t t)
{
	DBG_ASSERT(t <= NoneTag);
	if(!KonohaContext_Is(Interactive, kctx)) t = NoneTag;
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

static const char* EndTag(KonohaContext *kctx, kinfotag_t t)
{
	DBG_ASSERT(t <= NoneTag);
	if(!KonohaContext_Is(Interactive, kctx)) t = NoneTag;
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

static void UI_ReportUserMessage(KonohaContext *kctx, kinfotag_t level, kfileline_t pline, const char *msg, int isNewLine)
{
	const char *beginTag = BeginTag(kctx, level);
	const char *endTag = EndTag(kctx, level);
	const char *kLF = isNewLine ? "\n" : "";
	if(pline > 0) {
		const char *file = KFileLine_textFileName(pline);
		PLATAPI printf_i("%s - (%s:%d) %s%s%s" , beginTag, PLATAPI shortFilePath(file), (kushort_t)pline, msg, kLF, endTag);
	}
	else {
		PLATAPI printf_i("%s%s%s%s", beginTag,  msg, kLF, endTag);
	}
}

static void UI_ReportCompilerMessage(KonohaContext *kctx, kinfotag_t taglevel, kfileline_t pline, const char *msg)
{
	const char *beginTag = BeginTag(kctx, taglevel);
	const char *endTag = EndTag(kctx, taglevel);
	PLATAPI printf_i("%s - %s%s\n", beginTag, msg, endTag);
}

static void KBuffer_WriteValue(KonohaContext *kctx, KBuffer *wb, KClass *c, KonohaStack *sfp)
{
	if(KClass_Is(UnboxType, c)) {
		c->format(kctx, sfp, 0, wb);
	}
	else {
		KLIB kObject_WriteToBuffer(kctx, sfp[0].asObject, false/*delim*/, wb, NULL, 0);
	}
}

static void UI_ReportCaughtException(KonohaContext *kctx, kException *e, KonohaStack *bottomStack, KonohaStack *topStack)
{
	DBG_ASSERT(IS_Exception(e));
	const char *exceptionName = KSymbol_text(e->symbol);
	const char *optionalMessage = kString_text(e->Message);
	int fault = e->fault;
	PLATAPI printf_i("%s", BeginTag(kctx, ErrTag));
	if(optionalMessage != NULL && optionalMessage[0] != 0) {
		PLATAPI printf_i("%s: SoftwareFault %s", exceptionName, optionalMessage);
	}
	else {
		PLATAPI printf_i("%s:", exceptionName);
		if(KFlag_Is(int, fault, SoftwareFault)) {
			PLATAPI printf_i(" SoftwareFault");
		}
		if(KFlag_Is(int, fault, UserFault)) {
			PLATAPI printf_i(" UserFault");
		}
		if(KFlag_Is(int, fault, SystemFault)) {
			PLATAPI printf_i(" SystemFault");
		}
		if(KFlag_Is(int, fault, ExternalFault)) {
			PLATAPI printf_i(" ExternalFault");
		}
	}
	PLATAPI printf_i("%s\n", EndTag(kctx, ErrTag));
	PLATAPI printf_i("%sStackTrace\n", BeginTag(kctx, InfoTag));

	KonohaStack *sfp = topStack;
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	while(bottomStack < sfp) {
		kMethod *mtd = sfp[K_MTDIDX].calledMethod;
		kfileline_t uline = sfp[K_RTNIDX].calledFileLine;
		const char *file = PLATAPI shortFilePath(KFileLine_textFileName(uline));
		PLATAPI printf_i(" [%ld] (%s:%d) %s.%s%s(", (sfp - kctx->stack->stack), file, (kushort_t)uline, kMethod_Fmt3(mtd));
		KClass *cThis = KClass_(mtd->typeId);
		if(!KClass_Is(UnboxType, cThis)) {
			cThis = kObject_class(sfp[0].asObject);
		}
		if(!kMethod_Is(Static, mtd)) {
			KBuffer_WriteValue(kctx, &wb, cThis, sfp);
			PLATAPI printf_i("this=(%s) %s, ", KClass_text(cThis), KLIB KBuffer_text(kctx, &wb, 1));
			KLIB KBuffer_Free(&wb);
		}
		unsigned i;
		kParam *param = kMethod_GetParam(mtd);
		for(i = 0; i < param->psize; i++) {
			if(i > 0) {
				PLATAPI printf_i(", ");
			}
			KClass *c = KClass_(param->paramtypeItems[i].attrTypeId);
			c = c->realtype(kctx, c, cThis);
			KBuffer_WriteValue(kctx, &wb, c, sfp + i + 1);
			PLATAPI printf_i("%s=(%s) %s", KSymbol_text(KSymbol_Unmask(param->paramtypeItems[i].name)), KClass_text(c), KLIB KBuffer_text(kctx, &wb, 1));
			KLIB KBuffer_Free(&wb);
		}
		PLATAPI printf_i(")\n");
		sfp = sfp[K_SHIFTIDX].previousStack;
	}
	KLIB KBuffer_Free(&wb);
	PLATAPI printf_i("%s\n", EndTag(kctx, InfoTag));
}

static char* InputUserText(KonohaContext *kctx, const char *message, int flag)
{
	PLATAPI ConsoleModule.ReportUserMessage(kctx, WarnTag, 0, "unsupported; use -M Console option or the like", true);
	return NULL;
}

static int InputUserApproval(KonohaContext *kctx, const char *message, const char *yes, const char *no, int defval)
{
	PLATAPI ConsoleModule.ReportUserMessage(kctx, WarnTag, 0, "unsupported; use -M Console option or the like", true);
	return defval;
}

static char* InputUserPassword(KonohaContext *kctx, const char *message)
{
	PLATAPI ConsoleModule.ReportUserMessage(kctx, WarnTag, 0, "unsupported; use -M Console option or the like", true);
	return NULL;
}

static void exit_i(int status, const char *file, int line)
{
	if(status != 0) {
		fprintf(stderr, "exiting status = %d, file='%s' linenum=%d\n", status, file, line);
	}
	exit(status);
}

// --------------------------------------------------------------------------
static kunused void PosixFactory(KonohaFactory *factory)
{
	factory->name            = "shell";
	factory->stacksize       = K_PAGESIZE * 4;
	factory->getenv_i        = (const char *(*)(const char *))getenv;
	factory->malloc_i        = malloc;
	factory->free_i          = free;
	factory->setjmp_i        = ksetjmp;
	factory->longjmp_i       = klongjmp;

	factory->printf_i        = printf;
	factory->vprintf_i       = vprintf;
	factory->snprintf_i      = snprintf;  // avoid to use Xsnprintf
	factory->vsnprintf_i     = vsnprintf; // retreating..
	factory->qsort_i         = qsort;
	factory->exit_i          = exit_i;

	// mutex
	factory->ThreadModule.thread_create_i        = kpthread_create;
	factory->ThreadModule.thread_join_i          = kpthread_join;
	factory->ThreadModule.thread_mutex_init_i    = kpthread_mutex_init;
	factory->ThreadModule.thread_mutex_init_recursive = kpthread_mutex_init_recursive;
	factory->ThreadModule.thread_mutex_lock_i    = kpthread_mutex_lock;
	factory->ThreadModule.thread_mutex_unlock_i  = kpthread_mutex_unlock;
	factory->ThreadModule.thread_mutex_trylock_i = kpthread_mutex_trylock;
	factory->ThreadModule.thread_mutex_destroy_i = kpthread_mutex_destroy;
	factory->ThreadModule.thread_cond_init_i     = kpthread_cond_init;
	factory->ThreadModule.thread_cond_wait_i     = kpthread_cond_wait;
	factory->ThreadModule.thread_cond_signal_i   = kpthread_cond_signal;
	factory->ThreadModule.thread_cond_broadcast_i= kpthread_cond_broadcast;
	factory->ThreadModule.thread_cond_destroy_i  = kpthread_cond_destroy;

	factory->LoadPlatformModule   = LoadPlatformModule;
	factory->FormatPackagePath   = FormatPackagePath;
	factory->LoadPackageHandler  = LoadPackageHandler;
	factory->BEFORE_LoadScript   = BEFORE_LoadScript;
	factory->AFTER_LoadScript    = AFTER_LoadScript;

	factory->shortFilePath       = shortFilePath;
	factory->formatTransparentPath = formatTransparentPath;
	factory->loadScript            = loadScript;

	// timer
	factory->getTimeMilliSecond    = getTimeMilliSecond;

	loadI18N(factory, "UTF-8");

	// readline
	PlatformApi_loadReadline(factory);

	// logger
	//factory->LoggerModule.syslog_i              = syslog;
	//factory->LoggerModule.vsyslog_i             = vsyslog;
	//factory->LoggerModule.TraceDataLog          = TraceDataLog;
	//factory->DiagnosisErrorCode    = DEOS_DiagnosisErrorCode;

	factory->ConsoleModule.ReportDebugMessage    = (!verbose_debug) ? NOP_ReportDebugMessage : ReportDebugMessage;
	factory->ConsoleModule.ReportUserMessage     = UI_ReportUserMessage;
	factory->ConsoleModule.ReportCompilerMessage = UI_ReportCompilerMessage;
	factory->ConsoleModule.ReportCaughtException = UI_ReportCaughtException;
	factory->ConsoleModule.InputUserApproval     = InputUserApproval;
	factory->ConsoleModule.InputUserText         = InputUserText;
	factory->ConsoleModule.InputUserPassword     = InputUserPassword;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PLATFORM_POSIX_H_ */
