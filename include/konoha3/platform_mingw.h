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

#ifndef PLATFORM_MINGW_H_
#define PLATFORM_MINGW_H_

/* platform configuration */

#ifndef K_OSDLLEXT
#define K_OSDLLEXT        ".dll"
#endif

#ifdef PATH_MAX
#define K_PATHMAX PATH_MAX
#else
#define K_PATHMAX 256
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <sys/time.h>

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#include <sys/stat.h>
#include <errno.h>

#ifdef HAVE_ICONV_H
#include <iconv.h>
#endif /* HAVE_ICONV_H */

#ifdef K_USE_PTHREAD
#include <pthread.h>
#endif

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#define kunused __attribute__((unused))
// -------------------------------------------------------------------------

static const char *isSystemCharsetUTF8(void)
{
#if defined(K_USING_WINDOWS_)
	static char codepage[64];
	knh_snprintf(codepage, sizeof(codepage), "CP%d", (int)GetACP());
	return codepage;
#else
	return "UTF-8";
#endif
}

typedef uintptr_t (*ficonv_open)(const char *, const char *);
typedef size_t (*ficonv)(uintptr_t, char **, size_t *, char **, size_t *);
typedef int    (*ficonv_close)(uintptr_t);

static kunused uintptr_t dummy_iconv_open(const char *t, const char *f)
{
	return -1;
}
static kunused size_t dummy_iconv(uintptr_t i, char **t, size_t *ts, char **f, size_t *fs)
{
	return 0;
}
static kunused int dummy_iconv_close(uintptr_t i)
{
	return 0;
}

static void loadIconv(KonohaFactory *plat)
{
#ifdef _ICONV_H
	plat->iconv_open_i    = (ficonv_open)iconv_open;
	plat->iconv_i         = (ficonv)iconv;
	plat->iconv_close_i   = (ficonv_close)iconv_close;
#else
	HMODULE handler = LoadLibrary("libiconv" K_OSDLLEXT);
	if(handler != NULL) {
		plat->iconv_open_i = (ficonv_open)GetProcAddress(handler, "iconv_open");
		plat->iconv_i = (ficonv)GetProcAddress(handler, "iconv");
		plat->iconv_close_i = (ficonv_close)GetProcAddress(handler, "iconv_close");
	}
	else {
		plat->iconv_open_i = dummy_iconv_open;
		plat->iconv_i = dummy_iconv;
		plat->iconv_close_i = dummy_iconv_close;
	}
#endif /* _ICONV_H */
}

// -------------------------------------------------------------------------

static unsigned long long getTimeMilliSecond(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// -------------------------------------------------------------------------

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

// -------------------------------------------------------------------------

static const char* formatSystemPath(char *buf, size_t bufsiz, const char *path)
{
	return path;  // stub (in case of no conversion)
}

static const char* formatKonohaPath(char *buf, size_t bufsiz, const char *path)
{
	return path;  // stub (in case of no conversion)
}

static kbool_t isDir(const char *path)
{
	struct stat buf;
	char pathbuf[K_PATHMAX];
	if(stat(formatSystemPath(pathbuf, sizeof(pathbuf), path), &buf) == 0) {
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
			memset(simpleBuffer.buffer, 0, simpleBuffer.allocSize);
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
		fclose(fp);
	}
	return isSuccessfullyLoading;
}

static const char* shortFilePath(const char *path)
{
	char *p = (char *) strrchr(path, '/');
	return (p == NULL) ? path : (const char *)p+1;
}

static const char* shortText(const char *msg)
{
	return msg;
}

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

#ifndef K_PREFIX
#define K_PREFIX  "/usr/local"
#endif

static const char* packname(const char *str)
{
	char *p = (char *) strrchr(str, '.');
	return (p == NULL) ? str : (const char *)p+1;
}

static const char* FormatPackagePath(char *buf, size_t bufsiz, const char *packageName, const char *ext)
{
	FILE *fp = NULL;
	char *path = getenv("KONOHA_PACKAGEPATH");
	const char *local = "";
	if(path == NULL) {
		path = getenv("KONOHA_HOME");
		local = "/package";
	}
	if(path == NULL) {
		path = getenv("HOME");
		local = "/.konoha/package";
	}
	snprintf(buf, bufsiz, "%s%s/%s/%s%s", path, local, packageName, packname(packageName), ext);
#ifdef K_PREFIX
	fp = fopen(buf, "r");
	if(fp != NULL) {
		fclose(fp);
		return (const char *)buf;
	}
	snprintf(buf, bufsiz, K_PREFIX "/konoha/package" "/%s/%s%s", packageName, packname(packageName), ext);
#endif
	fp = fopen(buf, "r");
	if(fp != NULL) {
		fclose(fp);
		return (const char *)buf;
	}
	return NULL;
}

static KPackageHandler *LoadPackageHandler(const char *packageName)
{
	char pathbuf[256];
	FormatPackagePath(pathbuf, sizeof(pathbuf), packageName, "_glue" K_OSDLLEXT);
	HMODULE gluehdr = LoadLibrary(pathbuf);
	//fprintf(stderr, "pathbuf=%s, gluehdr=%p", pathbuf, gluehdr);
	if(gluehdr != NULL) {
		char funcbuf[80];
		snprintf(funcbuf, sizeof(funcbuf), "%s_Init", packname(packageName));
		PackageLoadFunc f = (PackageLoadFunc)GetProcAddress(gluehdr, funcbuf);
		if(f != NULL) {
			return f();
		}
	}
	return NULL;
}

static const char* beginTag(kinfotag_t t)
{
	DBG_ASSERT(t <= NoneTag);
	static const char* tags[] = {
		"", /*CritTag*/
		"", /*ErrTag*/
		"", /*WarnTag*/
		"", /*NoticeTag*/
		"", /*InfoTag*/
		"", /*DebugTag*/
		"", /* NoneTag*/
	};
	return tags[(int)t];
}

static const char* endTag(kinfotag_t t)
{
	DBG_ASSERT(t <= NoneTag);
	static const char* tags[] = {
		"", /*CritTag*/
		"", /*ErrTag*/
		"", /*WarnTag*/
		"", /*NoticeTag*/
		"", /*InfoTag*/
		"", /* Debug */
		"", /* NoneTag*/
	};
	return tags[(int)t];
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

static void reportCaughtException(const char *exceptionName, const char *scriptName, int line, const char *optionalMessage)
{
	if(line != 0) {
		if(optionalMessage != NULL && optionalMessage[0] != 0) {
			fprintf(stderr, " ** (%s:%d) %s: %s\n", scriptName, line, exceptionName, optionalMessage);
		}
		else {
			fprintf(stderr, " ** (%s:%d) %s\n", scriptName, line, exceptionName);
		}
	}
	else {
		if(optionalMessage != NULL && optionalMessage[0] != 0) {
			fprintf(stderr, " ** %s: %s\n", exceptionName, optionalMessage);
		}
		else {
			fprintf(stderr, " ** %s\n", exceptionName);
		}
	}
}


static void NOP_ReportDebugMessage(const char *file, const char *func, int line, const char *fmt, ...)
{
}

// --------------------------------------------------------------------------

#include "libcode/libc_readline.h"

static void PlatformApi_loadReadline(KonohaFactory *plat)
{
	HMODULE handler = LoadLibrary("libreadline" K_OSDLLEXT);
	if(handler != NULL) {
		plat->readline_i = (char* (*)(const char *))GetProcAddress(handler, "readline");
		plat->add_history_i = (int (*)(const char *))GetProcAddress(handler, "add_history");
	}
	if(plat->readline_i == NULL) {
		plat->readline_i = readline;
	}
	if(plat->add_history_i == NULL) {
		plat->add_history_i = add_history;
	}
}

// --------------------------------------------------------------------------

#define EBUFSIZ 1024
#include "libcode/logtext_formatter.h"

static void TraceDataLog(void *logger, int logkey, logconf_t *logconf, ...)
{
	char buf[EBUFSIZ];
	va_list ap;
	va_start(ap, logconf);
	writeDataLogToBuffer(logconf, ap, buf, buf + (EBUFSIZ - 4));
	//syslog(LOG_NOTICE, "%s", buf);
	if(verbose_debug) {
		fprintf(stderr, "TRACE %s\n", buf);
	}
	va_end(ap);
}

static void diagnosis(void) {
}

static PlatformApi* KonohaUtils_getDefaultPlatformApi(void)
{
	static KonohaFactory plat = {};
	plat.name            = "shell";
	plat.stacksize       = K_PAGESIZE * 4;
	plat.getenv_i        =  (const char *(*)(const char *))getenv;
	plat.malloc_i        = malloc;
	plat.free_i          = free;
	plat.setjmp_i        = ksetjmp;
	plat.longjmp_i       = klongjmp;
	loadIconv(&plat);
	plat.isSystemCharsetUTF8 = isSystemCharsetUTF8;
	plat.printf_i        = printf;
	plat.vprintf_i       = vprintf;
	plat.snprintf_i      = snprintf;  // avoid to use Xsnprintf
	plat.vsnprintf_i     = vsnprintf; // retreating..
	plat.qsort_i         = qsort;
	plat.exit_i          = exit;

	// mutex
	plat.ThreadModule.thread_mutex_init_i = kpthread_mutex_init;
	plat.ThreadModule.thread_mutex_init_recursive = kpthread_mutex_init_recursive;
	plat.ThreadModule.thread_mutex_lock_i    = kpthread_mutex_lock;
	plat.ThreadModule.thread_mutex_unlock_i  = kpthread_mutex_unlock;
	plat.ThreadModule.thread_mutex_trylock_i = kpthread_mutex_trylock;
	plat.ThreadModule.thread_mutex_destroy_i = kpthread_mutex_destroy;

	plat.shortFilePath       = shortFilePath;
	plat.FormatPackagePath   = FormatPackagePath;
	plat.formatTransparentPath = formatTransparentPath;
	plat.formatKonohaPath = formatKonohaPath;
	plat.formatSystemPath = formatSystemPath;
	plat.LoadPackageHandler  = LoadPackageHandler;
	plat.loadScript          = loadScript;
	plat.beginTag            = beginTag;
	plat.endTag              = endTag;
	plat.shortText           = shortText;
	plat.reportCaughtException = reportCaughtException;
	plat.ReportDebugMessage         = (!verbose_debug) ? NOP_ReportDebugMessage : ReportDebugMessage;

	// timer
	plat.getTimeMilliSecond  = getTimeMilliSecond;

	// readline
	PlatformApi_loadReadline(&plat);

	// logger
	//plat.LOGGER_NAME         = "syslog";
	//plat.syslog_i            = syslog;
	//plat.vsyslog_i           = vsyslog;
	plat.logger              = NULL;
	plat.TraceDataLog        = TraceDataLog;
	plat.diagnosis           = diagnosis;
	return (PlatformApi *)(&plat);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PLATFORM_MINGW_H_ */
