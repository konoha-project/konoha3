#include "minikonoha/minikonoha.h"
#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>

#ifndef TEST_KONOHA_H_
#define TEST_KONOHA_H_
static const char* packname(const char *str)
{
	char *p = strrchr(str, '.');
	return (p == NULL) ? str : (const char*)p+1;
}

static const char* packagepath(char *buf, size_t bufsiz, const char *fname)
{
	char *path = PLATAPI getenv_i("KONOHA_PACKAGEPATH"), *local = "";
	if(path == NULL) {
		path = PLATAPI getenv_i("KONOHA_HOME");
		local = "/package";
	}
	if(path == NULL) {
		path = PLATAPI getenv_i("HOME");
		local = "/.minikonoha/package";
	}
	snprintf(buf, bufsiz, "%s%s/%s/%s_glue.k", path, local, fname, packname(fname));
#ifdef K_PREFIX
	FILE *fp = fopen(buf, "r");
	if(fp != NULL) {
		fclose(fp);
	}
	else {
		snprintf(buf, bufsiz, K_PREFIX "/minikonoha/package" "/%s/%s_glue.k", fname, packname(fname));
	}
#endif
	return (const char*)buf;
}

static const char* exportpath(char *pathbuf, size_t bufsiz, const char *pname)
{
	char *p = strrchr(pathbuf, '/');
	snprintf(p, bufsiz - (p  - pathbuf), "/%s_exports.k", packname(pname));
	FILE *fp = fopen(pathbuf, "r");
	if(fp != NULL) {
		fclose(fp);
		return (const char*)pathbuf;
	}
	return NULL;
}

static const char* begin(kinfotag_t t) { return ""; }
static const char* end(kinfotag_t t) { return ""; }
static void debugPrintf(const char *file, const char *func, int line, const char *fmt, ...) {}


static PlatformApi plat = {
	"test", 4096,
	malloc,
	free,
	ksetjmp,
	klongjmp,
//	realpath,
//	(FILE_i* (*)(const char*, const char*))fopen,
//	(int     (*)(FILE_i *))fgetc,
//	(int     (*)(FILE_i *))feof,
//	(int     (*)(FILE_i *))fclose,
	syslog,
	vsyslog,
	printf,
	vprintf,
	snprintf,
	vsnprintf,
	qsort,
	exit,
	packagepath,
	exportpath,
	begin,
	end,
	debugPrintf,
};

#endif /* end of include guard */
