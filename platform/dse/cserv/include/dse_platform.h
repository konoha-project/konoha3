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
#include <konoha2/konoha2.h>

typedef kplatform_t dse_platform_t_h;

static const char* _packname(const char *str)
{
	char *p = strrchr(str, '.');
	return (p == NULL) ? str : (const char*)p+1;
}

static const char* _packagepath(char *buf, size_t bufsiz, const char *fname)
{
	char *path = getenv("KONOHA_PACKAGEPATH"), *local = "";
	if(path == NULL) {
		path = getenv("KONOHA_HOME");
		local = "/package";
	}
	if(path == NULL) {
		path = getenv("HOME");
		local = "/.konoha2/package";
	}
	snprintf(buf, bufsiz, "%s%s/%s/%s_glue.k", path, local, fname, _packname(fname));
#ifdef K_PREFIX
	FILE *fp = fopen(buf, "r");
	if(fp != NULL) {
		fclose(fp);
	}
	else {
		snprintf(buf, bufsiz, K_PREFIX "/konoha2/package" "/%s/%s_glue.k", fname, _packname(fname));
	}
#endif
	return (const char*)buf;
}

static const char* _exportpath(char *pathbuf, size_t bufsiz, const char *pname)
{
	char *p = strrchr(pathbuf, '/');
	snprintf(p, bufsiz - (p  - pathbuf), "/%s_exports.k", _packname(pname));
	FILE *fp = fopen(pathbuf, "r");
	if(fp != NULL) {
		fclose(fp);
		return (const char*)pathbuf;
	}
	return NULL;
}

static const char* _begin(kinfotag_t t) { (void)t; return ""; }
static const char* _end(kinfotag_t t) { (void)t; return ""; }

static void _dbg_p(const char *file, const char *func, int L, const char *fmt, ...)
{
	va_list ap;
	va_start(ap , fmt);
	fflush(stdout);
	fprintf(stderr, "DEBUG(%s:%s:%d) ", file, func, L);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

static void _NOP_dbg_p(const char *file, const char *func, int line, const char *fmt, ...)
{
}

kplatform_t *platform_dse(void)
{
	static kplatform_t dse = {
		.name			= "dse",
		.stacksize		= 4096,
		.malloc_i		= malloc,
		.free_i			= free,
		.setjmp_i		= ksetjmp,
		.longjmp_i		= klongjmp,

		.realpath_i = realpath,
		.fopen_i		= (FILE_i * (*)(const char *, const char *))fopen,
		.fgetc_i		= (int (*)(FILE_i *))fgetc,
		.feof_i			= (int (*)(FILE_i *))feof,
		.fclose_i		= (int (*)(FILE_i *))fclose,
		.syslog_i		= syslog,
		.vsyslog_i		= vsyslog,
		.printf_i		= printf,
		.vprintf_i		= vprintf,
		.snprintf_i		= snprintf,  // avoid to use Xsnprintf
		.vsnprintf_i	= vsnprintf, // retreating..
		.qsort_i		= qsort,
		.exit_i			= exit,
		// high level
		.packagepath = _packagepath,
		.exportpath = _exportpath,
		.begin			= _begin,
		.end			= _end,
		.dbg_p			= _NOP_dbg_p,
	};
	if(getenv("KONOHA_DEBUG") != NULL) {
		dse.dbg_p = _dbg_p;
	}
	return (&dse);
}



#endif /* DSE_PLATFORM_H_ */
