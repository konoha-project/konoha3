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

/* platform configuration */

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
#include <stdlib.h>
#include <setjmp.h>
#include <sys/time.h>
#include <syslog.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef HAVE_ICONV_H
#include <iconv.h>
#endif /* HAVE_ICONV_H */

#ifdef __cplusplus
extern "C" {
#endif

#define kunused __attribute__((unused))

#include <minikonoha/klib.h>

// -------------------------------------------------------------------------
/* I18N */

#ifdef HAVE_ICONV_H

static uintptr_t I18N_iconv_open(KonohaContext *kctx, const char *targetCharset, const char *sourceCharset, KTraceInfo *trace)
{
	uintptr_t ic = (uintptr_t)iconv_open(targetCharset, sourceCharset);
	if(ic == ICONV_NULL) {
		KTraceApi(trace, DataFault|SoftwareFault, "iconv_open",
			LogText("tocode", targetCharset), LogText("fromcode", sourceCharset), LogErrno
		);
	}
	return (uintptr_t)ic;
}

static size_t I18N_iconv_memcpyStyle(KonohaContext *kctx, uintptr_t ic, char **outbuf, size_t *outBytesLeft, char **inbuf, size_t *inBytesLeft, int *isTooBigSourceRef, KTraceInfo *trace)
{
	DBG_ASSERT(ic != ICONV_NULL);
	size_t iconv_ret = iconv((iconv_t)ic, inbuf, inBytesLeft, outbuf, outBytesLeft);
	if(iconv_ret == ((size_t)-1)) {
		if(errno == E2BIG) {   // input is too big.
			isTooBigSourceRef[0] = true;
			return iconv_ret;
		}
		KTraceApi(trace, DataFault, "iconv", LogErrno);
	}
	isTooBigSourceRef[0] = false;
	return iconv_ret;
}

static size_t I18N_iconv(KonohaContext *kctx, uintptr_t ic, char **inbuf, size_t *inBytesLeft, char **outbuf, size_t *outBytesLeft, int *isTooBigSourceRef, KTraceInfo *trace)
{
	DBG_ASSERT(ic != ICONV_NULL);
	size_t iconv_ret = iconv((iconv_t)ic, inbuf, inBytesLeft, outbuf, outBytesLeft);
	if(iconv_ret == ((size_t)-1)) {
		if(errno == E2BIG) {   // input is too big.
			isTooBigSourceRef[0] = true;
			return iconv_ret;
		}
		KTraceApi(trace, DataFault, "iconv", LogErrno);
	}
	isTooBigSourceRef[0] = false;
	return iconv_ret;
}

static int I18N_iconv_close(KonohaContext *kctx, uintptr_t ic)
{
	return iconv_close((iconv_t)ic);
}

static kbool_t I18N_isSystemCharsetUTF8(KonohaContext *kctx)
{
	const char *t = PLATAPI systemCharset;
	return (t[0] == 'U' && t[5] == 0 && t[4] == '8' && t[3] == '-' && t[2] == 'F'); // "UTF-8"
}

static uintptr_t I18N_iconvSystemCharsetToUTF8(KonohaContext *kctx, KTraceInfo *trace)
{
	return PLATAPI iconv_open_i(kctx, "UTF-8", PLATAPI systemCharset, trace);
}

static uintptr_t I18N_iconvUTF8ToSystemCharset(KonohaContext *kctx, KTraceInfo *trace)
{
	return PLATAPI iconv_open_i(kctx, PLATAPI systemCharset, "UTF-8", trace);
}

static const char* I18N_formatKonohaPath(KonohaContext *kctx, char *buf, size_t bufsiz, const char *path, size_t pathsize, KTraceInfo *trace)
{
	uintptr_t ic = PLATAPI iconvUTF8ToSystemCharset(kctx, trace);
	size_t newsize;
	if(ic != ICONV_NULL) {
		int isTooBig;
		char *presentPtrFrom = (char *)path;
		char ** inbuf = &presentPtrFrom;
		char ** outbuf = &buf;
		size_t inBytesLeft = pathsize, outBytesLeft = bufsiz - 1;
		PLATAPI iconv_i_memcpyStyle(kctx, ic, outbuf, &outBytesLeft, inbuf, &inBytesLeft, &isTooBig, trace);
		newsize = (bufsiz - 1) - outBytesLeft;
	}
	else {
		DBG_ASSERT(bufsiz > pathsize);
		memcpy(buf, path, pathsize);
		newsize = pathsize;
	}
	buf[newsize] = 0;
	return (const char *)buf;  // stub (in case of no conversion)
}

static const char* I18N_formatSystemPath(KonohaContext *kctx, char *buf, size_t bufsiz, const char *path, size_t pathsize, KTraceInfo *trace)
{
	return path;  // stub (in case of no conversion)
}

#else/*HAVE_ICONV_H*/

static uintptr_t I18N_iconv_open(KonohaContext *kctx, const char *targetCharset, const char *sourceCharset, KTraceInfo *trace)
{
	return ICONV_NULL;
}

static size_t I18N_iconv_memcpyStyle(KonohaContext *kctx, uintptr_t ic, char **outbuf, size_t *outBytesLeft, char **inbuf, size_t *inBytesLeft, int *isTooBigSourceRef, KTraceInfo *trace)
{
	return -1;
}

static size_t I18N_iconv(KonohaContext *kctx, uintptr_t ic, char **inbuf, size_t *inBytesLeft, char **outbuf, size_t *outBytesLeft, int *isTooBigSourceRef, KTraceInfo *trace)
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

#endif/*HAVE_ICONV_H*/

static void loadI18N(PlatformApiVar *plat, const char *defaultCharSet)
{
	plat->systemCharset  = (defaultCharSet == NULL) ? "UTF-8" : defaultCharSet;
	plat->iconv_open_i   = I18N_iconv_open;
	plat->iconv_i        = I18N_iconv;
	plat->iconv_i_memcpyStyle = I18N_iconv_memcpyStyle;
	plat->iconv_close_i  = I18N_iconv_close;
	plat->isSystemCharsetUTF8 = I18N_isSystemCharsetUTF8;
	plat->iconvSystemCharsetToUTF8 = I18N_iconvSystemCharsetToUTF8;
	plat->iconvUTF8ToSystemCharset = I18N_iconvUTF8ToSystemCharset;
	plat->formatKonohaPath = I18N_formatKonohaPath;
	plat->formatSystemPath = I18N_formatSystemPath;
}

// -------------------------------------------------------------------------

static unsigned long long getTimeMilliSecond(void)
{
//#if defined(K_USING_WINDOWS)
//	DWORD tickCount = GetTickCount();
//	return (knh_int64_t)tickCount;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// -------------------------------------------------------------------------

#ifdef K_USE_PTHREAD
#include <pthread.h>

static int pthread_mutex_init_recursive(kmutex_t *mutex)
{
	pthread_mutexattr_t attr;
	bzero(&attr, sizeof(pthread_mutexattr_t));
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	return pthread_mutex_init((pthread_mutex_t *)mutex, &attr);
}

#else

static int pthread_mutex_destroy(kmutex_t *mutex)
{
	return 0;
}

static int pthread_mutex_init(kmutex_t *mutex, const kmutexattr_t *attr)
{
	return 0;
}

static int pthread_mutex_lock(kmutex_t *mutex)
{
	return 0;
}

static int pthread_mutex_trylock(kmutex_t *mutex)
{
	return 0;
}

static int pthread_mutex_unlock(kmutex_t *mutex)
{
	return 0;
}

static int pthread_mutex_init_recursive(kmutex_t *mutex)
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

static const char* formatPackagePath(char *buf, size_t bufsiz, const char *packageName, const char *ext)
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
		local = "/.minikonoha/package";
	}
	snprintf(buf, bufsiz, "%s%s/%s/%s%s", path, local, packageName, packname(packageName), ext);
#ifdef K_PREFIX
	fp = fopen(buf, "r");
	if(fp != NULL) {
		fclose(fp);
		return (const char *)buf;
	}
	snprintf(buf, bufsiz, K_PREFIX "/lib/minikonoha/" K_VERSION "/package" "/%s/%s%s", packageName, packname(packageName), ext);
#endif
	fp = fopen(buf, "r");
	if(fp != NULL) {
		fclose(fp);
		return (const char *)buf;
	}
	return NULL;
}

static KonohaPackageHandler *loadPackageHandler(const char *packageName)
{
	char pathbuf[256];
	formatPackagePath(pathbuf, sizeof(pathbuf), packageName, "_glue" K_OSDLLEXT);
	void *gluehdr = dlopen(pathbuf, RTLD_LAZY);
	//fprintf(stderr, "pathbuf=%s, gluehdr=%p", pathbuf, gluehdr);
	if(gluehdr != NULL) {
		char funcbuf[80];
		snprintf(funcbuf, sizeof(funcbuf), "%s_init", packname(packageName));
		PackageLoadFunc f = (PackageLoadFunc)dlsym(gluehdr, funcbuf);
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

static const char* endTag(kinfotag_t t)
{
	DBG_ASSERT(t <= NoneTag);
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

static void debugPrintf(const char *file, const char *func, int line, const char *fmt, ...)
{
	va_list ap;
	va_start(ap , fmt);
	fflush(stdout);
	fprintf(stderr, "DEBUG(%s:%d) ", func, line);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

//static void reportCaughtException(const char *exceptionName, const char *scriptName, int line, const char *optionalMessage)
//{
//	if(line != 0) {
//		if(optionalMessage != NULL && optionalMessage[0] != 0) {
//			fprintf(stderr, " ** (%s:%d) %s: %s\n", scriptName, line, exceptionName, optionalMessage);
//		}
//		else {
//			fprintf(stderr, " ** (%s:%d) %s\n", scriptName, line, exceptionName);
//		}
//	}
//	else {
//		if(optionalMessage != NULL && optionalMessage[0] != 0) {
//			fprintf(stderr, " ** %s: %s\n", exceptionName, optionalMessage);
//		}
//		else {
//			fprintf(stderr, " ** %s\n", exceptionName);
//		}
//	}
//}


static void NOP_debugPrintf(const char *file, const char *func, int line, const char *fmt, ...)
{
}

// --------------------------------------------------------------------------

#include "libcode/libc_readline.h"

static void PlatformApi_loadReadline(PlatformApiVar *plat)
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

// --------------------------------------------------------------------------

#include "libcode/logtext_formatter.h"

static void traceDataLog(KonohaContext *kctx, KTraceInfo *trace, int logkey, logconf_t *logconf, ...)
{
	char buf[K_PAGESIZE];
	va_list ap;
	va_start(ap, logconf);
	writeDataLogToBuffer(logconf, ap, buf, buf + (K_PAGESIZE - 4));
	syslog(LOG_NOTICE, "%s", buf);
	if(verbose_debug) {
		fprintf(stderr, "TRACE %s\n", buf);
	}
	va_end(ap);
}

static void diagnosis(void)
{
}

// --------------------------------------------------------------------------

static void UI_reportUserMessage(KonohaContext *kctx, kinfotag_t level, kfileline_t pline, const char *msg, int isNewLine)
{
	const char *beginTag = PLATAPI beginTag(level);
	const char *endTag = PLATAPI endTag(level);
	const char *LF = isNewLine ? "\n" : "";
	if(pline > 0) {
		const char *file = FileId_t(pline);
		PLATAPI printf_i("%s - (%s:%d) %s%s%s" ,
			beginTag, PLATAPI shortFilePath(file), (kushort_t)pline, msg, LF, endTag);
	}
	else {
		PLATAPI printf_i("%s%s%s%s", beginTag,  msg, LF, endTag);
	}
}

static void UI_reportCompilerMessage(KonohaContext *kctx, kinfotag_t level, const char *msg)
{
	const char *beginTag = PLATAPI beginTag(level);
	const char *endTag = PLATAPI endTag(level);
	PLATAPI printf_i("%s - %s%s\n", beginTag, msg, endTag);
}

static void Kwb_writeValue(KonohaContext *kctx, KGrowingBuffer *wb, KonohaClass *c, KonohaStack *sfp)
{
	if(CT_isUnbox(c)) {
		c->p(kctx, sfp, 0, wb);
	}
	else {
		KLIB kObject_writeToBuffer(kctx, sfp[0].asObject, false/*delim*/, wb, NULL, 0);
	}
}

static void UI_reportException(KonohaContext *kctx, const char *exceptionName, int fault, const char *optionalMessage, KonohaStack *bottomStack, KonohaStack *topStack)
{
	PLATAPI printf_i("%s", PLATAPI beginTag(ErrTag));
	if(optionalMessage != NULL && optionalMessage[0] != 0) {
		PLATAPI printf_i("%s: SoftwareFault %s", exceptionName, optionalMessage);
	}
	else {
		PLATAPI printf_i("%s:", exceptionName);
		if(TFLAG_is(int, fault, SoftwareFault)) {
			PLATAPI printf_i(" SoftwareFault");
		}
		if(TFLAG_is(int, fault, DataFault)) {
			PLATAPI printf_i(" DataFault");
		}
		if(TFLAG_is(int, fault, SystemFault)) {
			PLATAPI printf_i(" SystemFault");
		}
		if(TFLAG_is(int, fault, ExternalFault)) {
			PLATAPI printf_i(" ExternalFault");
		}
	}
	PLATAPI printf_i("%s\n\n", PLATAPI endTag(ErrTag));
	PLATAPI printf_i("%sStackTrace\n", PLATAPI beginTag(InfoTag));

	KonohaStack *sfp = topStack;
	KGrowingBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	while(bottomStack < sfp) {
		kMethod *mtd = sfp[K_MTDIDX].methodCallInfo;
		kfileline_t uline = sfp[K_RTNIDX].callerFileLine;
		const char *file = PLATAPI shortFilePath(FileId_t(uline));
		PLATAPI printf_i(" [%ld] (%s:%d) %s.%s%s(", (sfp - kctx->stack->stack), file, (kushort_t)uline, Method_t(mtd));
		KonohaClass *cThis = CT_(mtd->typeId);
		if(!CT_isUnbox(cThis)) {
			cThis = O_ct(sfp[0].asObject);
		}
		if(!kMethod_is(Static, mtd)) {
			Kwb_writeValue(kctx, &wb, cThis, sfp);
			PLATAPI printf_i("this=(%s) %s, ", CT_t(cThis), KLIB Kwb_top(kctx, &wb, 1));
			KLIB Kwb_free(&wb);
		}
		int i;
		kParam *param = Method_param(mtd);
		for(i = 0; i < param->psize; i++) {
			if(i > 0) {
				PLATAPI printf_i(", ");
			}
			KonohaClass *c = CT_(param->paramtypeItems[i].ty);
			c = c->realtype(kctx, c, cThis);
			Kwb_writeValue(kctx, &wb, c, sfp + i + 1);
			PLATAPI printf_i("%s=(%s) %s", SYM_t(SYM_UNMASK(param->paramtypeItems[i].fn)), CT_t(c), KLIB Kwb_top(kctx, &wb, 1));
			KLIB Kwb_free(&wb);
		}
		PLATAPI printf_i(")\n");
		sfp = sfp[K_SHIFTIDX].previousStack;
	}
	KLIB Kwb_free(&wb);
	PLATAPI printf_i("%s\n", PLATAPI endTag(InfoTag));
}

// --------------------------------------------------------------------------

static PlatformApi* KonohaUtils_getDefaultPlatformApi(void)
{
	static PlatformApiVar plat = {};
	plat.name            = "shell";
	plat.stacksize       = K_PAGESIZE * 4;
	plat.getenv_i        =  (const char *(*)(const char *))getenv;
	plat.malloc_i        = malloc;
	plat.free_i          = free;
	plat.setjmp_i        = ksetjmp;
	plat.longjmp_i       = klongjmp;
	loadI18N(&plat, "UTF-8");

	plat.printf_i        = printf;
	plat.vprintf_i       = vprintf;
	plat.snprintf_i      = snprintf;  // avoid to use Xsnprintf
	plat.vsnprintf_i     = vsnprintf; // retreating..
	plat.qsort_i         = qsort;
	plat.exit_i          = exit;

	// mutex
	plat.pthread_mutex_init_i = pthread_mutex_init;
	plat.pthread_mutex_init_recursive = pthread_mutex_init_recursive;
	plat.pthread_mutex_lock_i = pthread_mutex_lock;
	plat.pthread_mutex_unlock_i = pthread_mutex_unlock;
	plat.pthread_mutex_trylock_i = pthread_mutex_trylock;
	plat.pthread_mutex_destroy_i = pthread_mutex_destroy;

	plat.shortFilePath       = shortFilePath;
	plat.formatPackagePath   = formatPackagePath;
	plat.formatTransparentPath = formatTransparentPath;
	plat.loadPackageHandler  = loadPackageHandler;
	plat.loadScript          = loadScript;
	plat.beginTag            = beginTag;
	plat.endTag              = endTag;
	plat.shortText           = shortText;
	plat.debugPrintf         = (!verbose_debug) ? NOP_debugPrintf : debugPrintf;

	// timer
	plat.getTimeMilliSecond  = getTimeMilliSecond;

	// readline
	PlatformApi_loadReadline(&plat);

	// logger
	plat.LOGGER_NAME         = "syslog";
	plat.syslog_i            = syslog;
	plat.vsyslog_i           = vsyslog;
	plat.logger              = NULL;
	plat.traceDataLog        = traceDataLog;
	plat.diagnosis           = diagnosis;

	plat.reportUserMessage     = UI_reportUserMessage;
	plat.reportCompilerMessage = UI_reportCompilerMessage;
	plat.reportException       = UI_reportException;

	return (PlatformApi *)(&plat);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PLATFORM_POSIX_H_ */
