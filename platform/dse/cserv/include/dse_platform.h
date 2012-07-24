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

/* ************************************************************************ */

#ifndef DSE_PLATFORM_H_
#define DSE_PLATFORM_H_

#include <setjmp.h>
#include <syslog.h>
#include <minikonoha/minikonoha.h>

static const char* dse_packname(const char *str)
{
	char *p = strrchr(str, '.');
	return (p == NULL) ? str : (const char*)p+1;
}

static const char* dse_formatPackagePath(char *buf, size_t bufsiz, const char *packageName, const char *ext)
{
	FILE *fp = NULL;
	char *path = (char *)getenv("KONOHA_PACKAGEPATH");
	const char *local = "";
	if(path == NULL) {
		path = getenv("KONOHA_HOME");
		local = "/package";
	}
	if(path == NULL) {
		path = getenv("HOME");
		local = "/.minikonoha/package";
	}
	snprintf(buf, bufsiz, "%s%s/%s/%s%s", path, local, packageName, dse_packname(packageName), ext);
#ifdef K_PREFIX
	fp = fopen(buf, "r");
	if(fp != NULL) {
		fclose(fp);
		return (const char*)buf;
	}
	snprintf(buf, bufsiz, K_PREFIX "/minikonoha/package" "/%s/%s%s", packageName, dse_packname(packageName), ext);
#endif
	fp = fopen(buf, "r");
	if(fp != NULL) {
		fclose(fp);
		return (const char*)buf;
	}
	return NULL;
}

static const char* dse_formatTransparentPath(char *buf, size_t bufsiz, const char *parentPath, const char *path)
{
	const char *p = strrchr(parentPath, '/');
	if(p != NULL && path[0] != '/') {
		size_t len = (p - parentPath) + 1;
		if(len < bufsiz) {
			memcpy(buf, parentPath, len);
			snprintf(buf + len, bufsiz - len, "%s", path);
			return (const char*)buf;
		}
	}
	return path;
}

static const char* dse_beginTag(kinfotag_t t) { (void)t; return ""; }
static const char* dse_endTag(kinfotag_t t) { (void)t; return ""; }

static void dse_debugPrintf(const char *file, const char *func, int L, const char *fmt, ...)
{
	va_list ap;
	va_start(ap , fmt);
	fflush(stdout);
	fprintf(stderr, "DEBUG(%s:%s:%d) ", file, func, L);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

static void dse_NOP_debugPrintf(const char *file, const char *func, int line, const char *fmt, ...)
{
}

static PlatformApi *platform_dse(void)
{
	static PlatformApiVar dse = {
		.name			= "dse",
		.stacksize		= 4096,
		.malloc_i		= malloc,
		.free_i			= free,
		.setjmp_i		= ksetjmp,
		.longjmp_i		= klongjmp,
		.syslog_i		= syslog,
		.vsyslog_i		= vsyslog,
		.printf_i		= printf,
		.vprintf_i		= vprintf,
		.snprintf_i		= snprintf,  // avoid to use Xsnprintf
		.vsnprintf_i	= vsnprintf, // retreating..
		.qsort_i		= qsort,
		.exit_i			= exit,
		// high level
		.formatPackagePath	= dse_formatPackagePath,
		.formatTransparentPath		= dse_formatTransparentPath,
		.beginTag		= dse_beginTag,
		.endTag			= dse_endTag,
		.debugPrintf	= dse_NOP_debugPrintf,
	};
	if(getenv("KONOHA_DEBUG") != NULL) {
		dse.debugPrintf = dse_debugPrintf;
	}
	return (PlatformApi*)(&dse);
}

#endif /* DSE_PLATFORM_H_ */
