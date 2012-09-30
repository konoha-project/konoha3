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

#define K_USE_PTHREAD
#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include "minikonoha/gc.h"
#include <minikonoha/klib.h>
#define USE_BUILTINTEST 1
#include "../shell/testkonoha.h"
#include <getopt.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------------------------------------

typedef struct {
	long interval;
	pthread_mutex_t lock;
} ThreadArg_t;

// -------------------------------------------------------------------------

kstatus_t MODSUGAR_eval(KonohaContext *kctx, const char *script, size_t len, kfileline_t uline);
kstatus_t MODSUGAR_loadScript(KonohaContext *kctx, const char *path, size_t len, kfileline_t pline);

// -------------------------------------------------------------------------
// getopt

static int compileonly_flag = 0;
static int interactive_flag = 0;
static int enable_monitor   = 0;

extern int verbose_debug;
extern int verbose_code;
extern int verbose_sugar;
extern int verbose_gc;

#include <minikonoha/platform.h>
#include <minikonoha/libcode/minishell.h>

// -------------------------------------------------------------------------
// KonohaContext*est

static FILE *stdlog;
static int   stdlog_count = 0;

static const char* TEST_begin(kinfotag_t t)
{
	return "";
}

static const char* TEST_end(kinfotag_t t)
{
	return "";
}

static const char* TEST_shortText(const char *msg)
{
	return "(omitted..)";
}

static int TEST_vprintf(const char *fmt, va_list ap)
{
	stdlog_count++;
	return vfprintf(stdlog, fmt, ap);
}

static int TEST_printf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int res = vfprintf(stdlog, fmt, ap);
	va_end(ap);
	return res;
}

static void TEST_reportCaughtException(const char *exceptionName, const char *scriptName, int line, const char *optionalMessage)
{
	if(line != 0) {
		fprintf(stdlog, " ** %s (%s:%d)\n", exceptionName, scriptName, line);
	}
	else {
		fprintf(stdlog, " ** %s\n", exceptionName);
	}
}

static int check_result2(FILE *fp0, FILE *fp1)
{
	char buf0[4096];
	char buf1[4096];
	while (fgets(buf0, sizeof(buf0), fp0) != NULL) {
		char *p = fgets(buf1, sizeof(buf1), fp1);
		if(p == NULL) return 1;//FAILED
		if((p = strstr(buf0, "(error) (")) != NULL) {
			p = strstr(p+8, ")");
			if(strncmp(buf0, buf1, p - buf1 + 1) != 0) return 1; //FAILED;
			continue;
		}
		if((p = strstr(buf0, "(warning) (")) != NULL) {
			p = strstr(p+10, ")");
			if(strncmp(buf0, buf1, p - buf1 + 1) != 0) return 1; //FAILED;
			continue;
		}
		if (strcmp(buf0, buf1) != 0) {
			return 1;//FAILED
		}
	}
	return 0; //OK
}

static void make_report(const char *testname)
{
	char *path = getenv("KONOHA_REPORT");
	if(path != NULL) {
		char report_file[256];
		char script_file[256];
		char correct_file[256];
		char result_file[256];
		snprintf(report_file, 256,  "%s/REPORT_%s.txt", path, shortFilePath(testname));
		snprintf(script_file, 256,  "%s", testname);
		snprintf(correct_file, 256, "%s.proof", script_file);
		snprintf(result_file, 256,  "%s.tested", script_file);
		FILE *fp = fopen(report_file, "w");
		FILE *fp2 = fopen(script_file, "r");
		int ch;
		while((ch = fgetc(fp2)) != EOF) {
			fputc(ch, fp);
		}
		fclose(fp2);
		fprintf(fp, "Expected Result (in %s)\n=====\n", result_file);
		fp2 = fopen(correct_file, "r");
		while((ch = fgetc(fp2)) != EOF) {
			fputc(ch, fp);
		}
		fclose(fp2);
		fprintf(fp, "Result (in %s)\n=====\n", result_file);
		fp2 = fopen(result_file, "r");
		while((ch = fgetc(fp2)) != EOF) {
			fputc(ch, fp);
		}
		fclose(fp2);
		fclose(fp);
	}
}

extern int konoha_detectFailedAssert;

static int KonohaContext_test(KonohaContext *kctx, const char *testname)
{
	int ret = 1; //FAILED
	char script_file[256];
	char correct_file[256];
	char result_file[256];
	PLATAPI snprintf_i(script_file, 256,  "%s", testname);
	PLATAPI snprintf_i(correct_file, 256, "%s.proof", script_file);
	PLATAPI snprintf_i(result_file, 256,  "%s.tested", script_file);
	FILE *fp = fopen(correct_file, "r");
	stdlog = fopen(result_file, "w");
	konoha_load((KonohaContext*)kctx, script_file);
	fprintf(stdlog, "Q.E.D.\n");   // Q.E.D.
	fclose(stdlog);

	if(fp != NULL) {
		FILE *fp2 = fopen(result_file, "r");
		ret = check_result2(fp, fp2);
		if(ret == 0) {
			fprintf(stdout, "[PASS]: %s\n", testname);
		}
		else {
			fprintf(stdout, "[FAIL]: %s\n", testname);
			make_report(testname);
			konoha_detectFailedAssert = 1;
		}
		fclose(fp);
		fclose(fp2);
	}
	else {
		//fprintf(stdout, "stdlog_count: %d\n", stdlog_count);
		if(stdlog_count == 0) {
			if(konoha_detectFailedAssert == 0) {
				fprintf(stdout, "[PASS]: %s\n", testname);
				return 0; // OK
			}
		}
		else {
			fprintf(stdout, "no proof file: %s\n", testname);
			konoha_detectFailedAssert = 1;
		}
		fprintf(stdout, "[FAIL]: %s\n", testname);
		return 1;
	}
	return ret;
}

#ifdef USE_BUILTINTEST
extern DEFINE_TESTFUNC KonohaTestSet[];
static BuiltInTestFunc lookupTestFunc(DEFINE_TESTFUNC *d, const char *name)
{
	while(d->name != NULL) {
		if(strcasecmp(name, d->name) == 0) {
			return d->f;
		}
		d++;
	}
	return NULL;
}
#endif

static int CommandLine_doBuiltInTest(KonohaContext* konoha, const char* name)
{
#ifdef USE_BUILTINTEST
	BuiltInTestFunc f = lookupTestFunc(KonohaTestSet, name);
	if(f != NULL) {
		return f(konoha);
	}
	fprintf(stderr, "Built-in test is not found: '%s'\n", name);
#else
	fprintf(stderr, "Built-in tests are not built; rebuild with -DUSE_BUILTINTEST\n");
#endif
	return 1;
}

static void CommandLine_define(KonohaContext *kctx, char *keyvalue)
{
	char *p = strchr(keyvalue, '=');
	if(p != NULL) {
		size_t len = p-keyvalue;
		char namebuf[len+1];
		memcpy(namebuf, keyvalue, len); namebuf[len] = 0;
		DBG_P("name='%s'", namebuf);
		ksymbol_t key = KLIB Ksymbol(kctx, namebuf, len, 0, SYM_NEWID);
		uintptr_t unboxValue;
		ktype_t ty;
		if(isdigit(p[1])) {
			ty = TY_int;
			unboxValue = (uintptr_t)strtol(p+1, NULL, 0);
		}
		else {
			ty = TY_TEXT;
			unboxValue = (uintptr_t)(p+1);
		}
		if(!KLIB kNameSpace_setConstData(kctx, KNULL(NameSpace), key, ty, unboxValue, 0)) {
			PLATAPI exit_i(EXIT_FAILURE);
		}
	}
	else {
		fprintf(stdout, "invalid define option: use -D<key>=<value>\n");
		PLATAPI exit_i(EXIT_FAILURE);
	}
}

static void CommandLine_import(KonohaContext *kctx, char *packageName)
{
	size_t len = strlen(packageName)+1;
	char bufname[len];
	memcpy(bufname, packageName, len);
	if(!(KLIB kNameSpace_importPackage(kctx, KNULL(NameSpace), bufname, 0))) {
		PLATAPI exit_i(EXIT_FAILURE);
	}
}

static void konoha_startup(KonohaContext *kctx, const char *startup_script)
{
	char buf[256];
	const char *path = PLATAPI getenv_i("KONOHA_SCRIPTPATH"), *local = "";
	if(path == NULL) {
		path = PLATAPI getenv_i("KONOHA_HOME");
		local = "/script";
	}
	if(path == NULL) {
		path = PLATAPI getenv_i("HOME");
		local = "/.minikonoha/script";
	}
	snprintf(buf, sizeof(buf), "%s%s/%s.k", path, local, startup_script);
	if(!konoha_load((KonohaContext*)kctx, (const char*)buf)) {
		PLATAPI exit_i(EXIT_FAILURE);
	}
}

static void CommandLine_setARGV(KonohaContext *kctx, int argc, char** argv)
{
	KonohaClass *CT_StringArray0 = CT_p0(kctx, CT_Array, TY_String);
	kArray *a = (kArray*)KLIB new_kObject(kctx, CT_StringArray0, 0);
	int i;
	for(i = 0; i < argc; i++) {
		DBG_P("argv=%d, '%s'", i, argv[i]);
		KLIB kArray_add(kctx, a, KLIB new_kString(kctx, argv[i], strlen(argv[i]), SPOL_TEXT));
	}
	KDEFINE_OBJECT_CONST ObjectData[] = {
			{"SCRIPT_ARGV", CT_StringArray0->typeId, (kObject*)a},
			{}
	};
	KLIB kNameSpace_loadConstData(kctx, KNULL(NameSpace), KonohaConst_(ObjectData), 0);
}

// -------------------------------------------------------------------------
// ** logger **

static char *writeFloatToBuffer(float f, char *const buftop, const char *const bufend)
{
	char *pos = (char *)buftop;
	char *e = (char *)bufend;
	uintptr_t u = f;
	pos = writeUnsingedIntToBuffer(u, pos, e);
	pos[0] = '.'; pos++;
	uintptr_t decimal = (f * 1000000) - (u * 1000000);
	return writeUnsingedIntToBuffer(decimal, pos, e);
}

#define LOG_f 4

static void writeResourceDataLogToBuffer(void *arg, va_list ap, char *buftop, char *bufend)
{
	int c = 0, logtype;
	buftop[0] = '{'; buftop++;
	while((logtype = va_arg(ap, int)) != LOG_END) {
		if(c > 0 && buftop + 3 < bufend) {
			buftop[0] = ',';
			buftop[1] = ' ';
			buftop+=2;
		}
		switch(logtype) {
		case LOG_s: {
			const char *key = va_arg(ap, const char*);
			const char *text = va_arg(ap, const char*);
			buftop = writeKeyToBuffer(key, strlen(key), buftop, bufend);
			buftop = writeTextToBuffer(text, buftop, bufend);
			break;
		}
		case LOG_u: {
			const char *key = va_arg(ap, const char*);
			buftop = writeKeyToBuffer(key, strlen(key), buftop, bufend);
			buftop = writeUnsingedIntToBuffer(va_arg(ap, uintptr_t), buftop, bufend);
			break;
		}
		case LOG_f: {
			const char *key = va_arg(ap, const char*);
			buftop = writeKeyToBuffer(key, strlen(key), buftop, bufend);
			buftop = writeFloatToBuffer(va_arg(ap, double), buftop, bufend);
			break;
		}
		}
		c++;
	}
	buftop[0] = '}'; buftop++;
	buftop[0] = '\0';
}

#define EBUFSIZ 1024

static void traceResourceDataLog(void *arg, ...)
{
	char buf[EBUFSIZ];
	va_list ap;
	va_start(ap, arg);
	writeResourceDataLogToBuffer(arg, ap, buf, buf + (EBUFSIZ - 4));
	syslog(LOG_NOTICE, "%s", buf);
	if(verbose_debug) {
		fprintf(stderr, "TRACE %s\n", buf);
	}
	va_end(ap);
}

#define trace(arg, ...) do {\
	traceResourceDataLog(arg, __VA_ARGS__, LOG_END);\
} while(0)

// -------------------------------------------------------------------------
// ** libproc **

#define SIGNAL_STRING
#define KLONG long long    // not typedef; want "unsigned KLONG" to work
#define P_G_SZ 20

typedef struct proc_t {
// 1st 16 bytes
    int
        tid,		// (special)       task id, the POSIX thread ID (see also: tgid)
    	ppid;		// stat,status     pid of parent process
    unsigned
        pcpu;           // stat (special)  %CPU usage (is not filled in by readproc!!!)
    char
    	state,		// stat,status     single-char code for process state (S=sleeping)
    	pad_1,		// n/a             padding
    	pad_2,		// n/a             padding
    	pad_3;		// n/a             padding
// 2nd 16 bytes
    unsigned long long
	utime,		// stat            user-mode CPU time accumulated by process
	stime,		// stat            kernel-mode CPU time accumulated by process
// and so on...
	cutime,		// stat            cumulative utime of process and reaped children
	cstime,		// stat            cumulative stime of process and reaped children
	start_time;	// stat            start time of process -- seconds since 1-1-70
#ifdef SIGNAL_STRING
    char
	// Linux 2.1.7x and up have 64 signals. Allow 64, plus '\0' and padding.
	signal[18],	// status          mask of pending signals, per-task for readtask() but per-proc for readproc()
	blocked[18],	// status          mask of blocked signals
	sigignore[18],	// status          mask of ignored signals
	sigcatch[18],	// status          mask of caught  signals
	_sigpnd[18];	// status          mask of PER TASK pending signals
#else
    long long
	// Linux 2.1.7x and up have 64 signals.
	signal,		// status          mask of pending signals, per-task for readtask() but per-proc for readproc()
	blocked,	// status          mask of blocked signals
	sigignore,	// status          mask of ignored signals
	sigcatch,	// status          mask of caught  signals
	_sigpnd;	// status          mask of PER TASK pending signals
#endif
    unsigned KLONG
	start_code,	// stat            address of beginning of code segment
	end_code,	// stat            address of end of code segment
	start_stack,	// stat            address of the bottom of stack for the process
	kstk_esp,	// stat            kernel stack pointer
	kstk_eip,	// stat            kernel instruction pointer
	wchan;		// stat (special)  address of kernel wait channel proc is sleeping in
    long
	priority,	// stat            kernel scheduling priority
	nice,		// stat            standard unix nice level of process
	rss,		// stat            resident set size from /proc/#/stat (pages)
	alarm,		// stat            ?
    // the next 7 members come from /proc/#/statm
	size,		// statm           total # of pages of memory
	resident,	// statm           number of resident set (non-swapped) pages (4k)
	share,		// statm           number of pages of shared (mmap'd) memory
	trs,		// statm           text resident set size
	lrs,		// statm           shared-lib resident set size
	drs,		// statm           data resident set size
	dt;		// statm           dirty pages
    unsigned long
	vm_size,        // status          same as vsize in kb
	vm_lock,        // status          locked pages in kb
	vm_rss,         // status          same as rss in kb
	vm_data,        // status          data size
	vm_stack,       // status          stack size
	vm_exe,         // status          executable size
	vm_lib,         // status          library size (all pages, not just used ones)
	rtprio,		// stat            real-time priority
	sched,		// stat            scheduling class
	vsize,		// stat            number of pages of virtual memory ...
	rss_rlim,	// stat            resident set size limit?
	flags,		// stat            kernel flags for the process
	min_flt,	// stat            number of minor page faults since process start
	maj_flt,	// stat            number of major page faults since process start
	cmin_flt,	// stat            cumulative min_flt of process and child processes
	cmaj_flt;	// stat            cumulative maj_flt of process and child processes
    char
	**environ,	// (special)       environment string vector (/proc/#/environ)
	**cmdline;	// (special)       command line string vector (/proc/#/cmdline)
    char
	// Be compatible: Digital allows 16 and NT allows 14 ???
    	euser[P_G_SZ],	// stat(),status   effective user name
    	ruser[P_G_SZ],	// status          real user name
    	suser[P_G_SZ],	// status          saved user name
    	fuser[P_G_SZ],	// status          filesystem user name
    	rgroup[P_G_SZ],	// status          real group name
    	egroup[P_G_SZ],	// status          effective group name
    	sgroup[P_G_SZ],	// status          saved group name
    	fgroup[P_G_SZ],	// status          filesystem group name
    	cmd[16];	// stat,status     basename of executable file in call to exec(2)
    struct proc_t
	*ring,		// n/a             thread group ring
	*next;		// n/a             various library uses
    int
	pgrp,		// stat            process group id
	session,	// stat            session id
	nlwp,		// stat,status     number of threads, or 0 if no clue
	tgid,		// (special)       task group ID, the POSIX PID (see also: tid)
	tty,		// stat            full device number of controlling terminal
        euid, egid,     // stat(),status   effective
        ruid, rgid,     // status          real
        suid, sgid,     // status          saved
        fuid, fgid,     // status          fs (used for file access only)
	tpgid,		// stat            terminal process group id
	exit_signal,	// stat            might not be SIGCHLD
	processor;      // stat            current (or most recent?) CPU
} proc_t;

typedef unsigned long long jiff;

#if !defined(restrict) && __STDC_VERSION__ < 199901
#if __GNUC__ > 2 || __GNUC_MINOR__ >= 92
#define restrict __restrict__
#else
#warning No restrict keyword?
#define restrict
#endif
#endif

struct libproc {
	// global variables
	unsigned long long *Hertz;   /* clock tick frequency */
	unsigned long *kb_main_buffers;
	unsigned long *kb_main_cached;
	unsigned long *kb_main_free;
	unsigned long *kb_main_total;
	unsigned long *kb_swap_used;

	// API
	void (*meminfo)(void);
	void (*getstat)(jiff *restrict, jiff *restrict, jiff *restrict, jiff *restrict, jiff *restrict, jiff *restrict, jiff *restrict, jiff *restrict,
			unsigned long *restrict, unsigned long *restrict, unsigned long *restrict, unsigned long *restrict,
			unsigned *restrict, unsigned *restrict,
			unsigned int *restrict, unsigned int *restrict,
			unsigned int *restrict, unsigned int *restrict);
	proc_t *(*get_proc_stats)(pid_t, proc_t*);
};

static struct libproc *getLibproc(void) {
	static struct libproc LIBPROC;
	void *handler;
	handler = dlopen("/lib/libproc-3.2.8.so", RTLD_LAZY);
	if(!handler) {
		fprintf(stderr, "%s\n", dlerror());
		exit(1);
	}
	if(!(LIBPROC.Hertz = dlsym(handler, "Hertz"))) {
		fprintf(stderr, "%s\n", dlerror());
		exit(1);
	}
	if(!(LIBPROC.kb_main_buffers = dlsym(handler, "kb_main_buffers"))) {
		fprintf(stderr, "%s\n", dlerror());
		exit(1);
	}
	if(!(LIBPROC.kb_main_cached = dlsym(handler, "kb_main_cached"))) {
		fprintf(stderr, "%s\n", dlerror());
		exit(1);
	}
	if(!(LIBPROC.kb_main_free = dlsym(handler, "kb_main_free"))) {
		fprintf(stderr, "%s\n", dlerror());
		exit(1);
	}
	if(!(LIBPROC.kb_main_total = dlsym(handler, "kb_main_total"))) {
		fprintf(stderr, "%s\n", dlerror());
		exit(1);
	}
	if(!(LIBPROC.kb_swap_used = dlsym(handler, "kb_swap_used"))) {
		fprintf(stderr, "%s\n", dlerror());
		exit(1);
	}
	if(!(LIBPROC.meminfo = dlsym(handler, "meminfo"))) {
		fprintf(stderr, "%s\n", dlerror());
		exit(1);
	}
	if(!(LIBPROC.getstat = dlsym(handler, "getstat"))) {
		fprintf(stderr, "%s\n", dlerror());
		exit(1);
	}
	if(!(LIBPROC.get_proc_stats = dlsym(handler, "get_proc_stats"))) {
		fprintf(stderr, "%s\n", dlerror());
		exit(1);
	}
	return &LIBPROC;
}

// -------------------------------------------------------------------------
// ** io **

typedef struct io_t {
	unsigned long rchar;
	unsigned long wchar;
	unsigned long syscr;
	unsigned long syscw;
	unsigned long read_bytes;
	unsigned long write_bytes;
	unsigned long cancelled_write_bytes;
} io_t;

#include <fcntl.h>

static int file2str(const char *directory, const char *what, char *ret, int cap) {
    static char filename[80];
    int fd, num_read;

    sprintf(filename, "%s/%s", directory, what);
    fd = open(filename, O_RDONLY, 0);
    if(unlikely(fd==-1)) return -1;
    num_read = read(fd, ret, cap - 1);
    close(fd);
    if(unlikely(num_read<=0)) return -1;
    ret[num_read] = '\0';
    return num_read;
}

static void str2io(const char *S, io_t *io)
{
	unsigned num;

	num = sscanf(S,
			"rchar: %lu\n"
			"wchar: %lu\n"
			"syscr: %lu\n"
			"syscw: %lu\n"
			"read_bytes: %lu\n"
			"write_bytes: %lu\n"
			"cancelled_write_bytes: %lu\n",
			&io->rchar,
			&io->wchar,
			&io->syscr,
			&io->syscw,
			&io->read_bytes,
			&io->write_bytes,
			&io->cancelled_write_bytes
			);
	if(num < 7) exit(1);

	return;
}

static io_t *get_io_stats(pid_t pid, io_t *io)
{
	static char path[PATH_MAX], sbuf[1024];
	struct stat statbuf;

	sprintf(path, "/proc/%d", pid);
	if (stat(path, &statbuf)) {
		perror("stat");
		return NULL;
	}

	if (file2str(path, "io", sbuf, sizeof sbuf) >= 0)
		str2io(sbuf, io);	/* parse /proc/#/io */

	return io;
}

// -------------------------------------------------------------------------
// ** resource monitor **

typedef unsigned long long TIC_t;
static float     Frame_tscale;          // so we can '*' vs. '/' WHEN 'pcpu'

static void prochlp (struct libproc *LIBPROC, proc_t *this)
{
   static TIC_t prev_tics;
   TIC_t new_tics;
   TIC_t tics_diff;
   static struct timeval oldtimev;
   struct timeval timev;
   struct timezone timez;
   float et;

   gettimeofday(&timev, &timez);
   et = (timev.tv_sec - oldtimev.tv_sec)
      + (float)(timev.tv_usec - oldtimev.tv_usec) / 1000000.0;
   oldtimev.tv_sec = timev.tv_sec;
   oldtimev.tv_usec = timev.tv_usec;

// if in Solaris mode, adjust our scaling for all cpus
//   Frame_tscale = 100.0f / ((float)Hertz * (float)et * (Rc.mode_irixps ? 1 : Cpu_tot));
   Frame_tscale = 100.0f / ((float)*(LIBPROC->Hertz) * (float)et);
   if(!this) return;

   /* calculate time in this process; the sum of user time (utime) and
      system time (stime) -- but PLEASE dont waste time and effort on
      calcs and saves that go unused, like the old top! */
   new_tics = tics_diff = (this->utime + this->stime);
   tics_diff -= prev_tics;
   prev_tics = new_tics;

   // we're just saving elapsed tics, to be converted into %cpu if
   // this task wins it's displayable screen row lottery... */
   this->pcpu = tics_diff;
   return;
}

static unsigned long dataUnit=1024;

static unsigned long unitConvert(unsigned int size){
 float cvSize;
 cvSize=(float)size/dataUnit*1024;
 return ((unsigned long) cvSize);
}

static unsigned long long getTime(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

#define LogFloat(K,V)    LOG_f, (K), ((float)V)

static unsigned Page_size;
static unsigned page_to_kb_shift;

#define PAGES_TO_KB(n)  (unsigned long)( (n) << page_to_kb_shift )

// _diagnosis must be called by only one thread
static void _diagnosis(void) {
	static pid_t              pid;
	static struct             libproc *LIBPROC;
	unsigned int              hz;
	unsigned int              running,blocked,dummy_1,dummy_2;
	static jiff               cpu_use[2], cpu_nic[2], cpu_sys[2], cpu_idl[2], cpu_iow[2], cpu_xxx[2], cpu_yyy[2], cpu_zzz[2];
	jiff                      duse, dsys, didl, diow, dstl, Div, divo2;
	static unsigned long      pgpgin[2], pgpgout[2], pswpin[2], pswpout[2];
	static unsigned int       intr[2], ctxt[2];
	static unsigned long long prev_time;
	unsigned long long        new_time, time_diff;
	unsigned                  sleep_time, sleep_half;
	unsigned long             kb_per_page = sysconf(_SC_PAGESIZE) / 1024ul;
	int                       debt = 0;  // handle idle ticks running backwards
	static unsigned int       isFirst = true;
	static unsigned int       tog = 0;
	unsigned                  swap_si, swap_so, io_bi, io_bo, system_in, system_cs;
	static unsigned           cpu_us, cpu_sy, cpu_id, cpu_wa;
	unsigned long             memory_swpd, memory_free, memory_buff, memory_cache;
	float                     percent_cpu_usage, percent_mem_usage;
	unsigned                  kb_mem_usage;

	if(isFirst) {
		prev_time = getTimeMilliSecond();
		pid = getpid();
		LIBPROC = getLibproc();
		hz = *(LIBPROC->Hertz);
		LIBPROC->getstat(cpu_use,cpu_nic,cpu_sys,cpu_idl,cpu_iow,cpu_xxx,cpu_yyy,cpu_zzz,
				pgpgin,pgpgout,pswpin,pswpout,
				intr,ctxt,
				&running,&blocked,
				&dummy_1, &dummy_2);
		duse= *cpu_use + *cpu_nic;
		dsys= *cpu_sys + *cpu_xxx + *cpu_yyy;
		didl= *cpu_idl;
		diow= *cpu_iow;
		dstl= *cpu_zzz;
		Div= duse+dsys+didl+diow+dstl;
		divo2= Div/2UL;

		swap_si   = (unsigned)((*pswpin  * unitConvert(kb_per_page) * hz + divo2) / Div );
		swap_so   = (unsigned)((*pswpout * unitConvert(kb_per_page) * hz + divo2) / Div );
		io_bi     = (unsigned)((*pgpgin  * hz + divo2) / Div );
		io_bo     = (unsigned)((*pgpgout * hz + divo2) / Div );
		system_in = (unsigned)((*intr    * hz + divo2) / Div );
		system_cs = (unsigned)((*ctxt    * hz + divo2) / Div );
		cpu_us    = (unsigned)((100*duse + divo2) / Div );
		cpu_sy    = (unsigned)((100*dsys + divo2) / Div );
		cpu_id    = (unsigned)((100*didl + divo2) / Div );
		cpu_wa    = (unsigned)((100*diow + divo2) / Div );

		isFirst = false;
	}
	else {
		new_time = getTimeMilliSecond();
		time_diff = (new_time - prev_time) / 1000;
		prev_time = new_time;
		sleep_time = (time_diff > 1) ? time_diff : 1;
		sleep_half = sleep_time/2;
		tog= !tog;
		LIBPROC->getstat(cpu_use+tog,cpu_nic+tog,cpu_sys+tog,cpu_idl+tog,cpu_iow+tog,cpu_xxx+tog,cpu_yyy+tog,cpu_zzz+tog,
				pgpgin+tog,pgpgout+tog,pswpin+tog,pswpout+tog,
				intr+tog,ctxt+tog,
				&running,&blocked,
				&dummy_1,&dummy_2);
		duse= cpu_use[tog]-cpu_use[!tog] + cpu_nic[tog]-cpu_nic[!tog];
		dsys= cpu_sys[tog]-cpu_sys[!tog] + cpu_xxx[tog]-cpu_xxx[!tog] + cpu_yyy[tog]-cpu_yyy[!tog];
		didl= cpu_idl[tog]-cpu_idl[!tog];
		diow= cpu_iow[tog]-cpu_iow[!tog];
		dstl= cpu_zzz[tog]-cpu_zzz[!tog];

		/* idle can run backwards for a moment -- kernel "feature" */
		if(debt){
		  didl = (int)didl + debt;
		  debt = 0;
		}
		if( (int)didl < 0 ){
		  debt = (int)didl;
		  didl = 0;
		}
		Div= duse+dsys+didl+diow+dstl;
		divo2= Div/2UL;
		swap_si   = (unsigned)(((pswpin[tog] - pswpin[!tog])*unitConvert(kb_per_page)+sleep_half)/sleep_time);
		swap_so   = (unsigned)(((pswpout[tog] - pswpout[!tog])*unitConvert(kb_per_page)+sleep_half)/sleep_time );
		io_bi     = (unsigned)((pgpgin[tog] - pgpgin[!tog]   +sleep_half)/sleep_time);
		io_bo     = (unsigned)((pgpgout[tog] - pgpgout[!tog] +sleep_half)/sleep_time);
		system_in = (unsigned)((intr[tog] - intr[!tog]       +sleep_half)/sleep_time);
		system_cs = (unsigned)((ctxt[tog] - ctxt[!tog]       +sleep_half)/sleep_time);
		if(Div) {
			cpu_us = (unsigned)((100*duse + divo2) / Div );
			cpu_sy = (unsigned)((100*dsys + divo2) / Div );
			cpu_id = (unsigned)((100*didl + divo2) / Div );
			cpu_wa = (unsigned)((100*diow + divo2) / Div );
		}
	}

	LIBPROC->meminfo();
	memory_swpd  = *(LIBPROC->kb_swap_used);
	memory_free  = *(LIBPROC->kb_main_free);
	memory_buff  = *(LIBPROC->kb_main_buffers);
	memory_cache = *(LIBPROC->kb_main_cached);

	proc_t *p = (proc_t *)alloca(sizeof(proc_t));
	p = LIBPROC->get_proc_stats(pid, p);
	prochlp(LIBPROC, p);
	percent_cpu_usage = (float)((float)p->pcpu * Frame_tscale);
	percent_mem_usage = (float)((float)PAGES_TO_KB(p->resident) * 100 / *(LIBPROC->kb_main_total));
	kb_mem_usage      = (unsigned)PAGES_TO_KB(p->resident);

	io_t *io = (io_t *)alloca(sizeof(io_t));
	io = get_io_stats(pid, io);

	static void *arg;
	trace(&arg,
			LogUint("time(ms)",                           getTime()),
			LogUint("procs_running",                      running),
			LogUint("procs_blocked",                      blocked),
			LogUint("memory_swpd(kb)",                    memory_swpd),
			LogUint("memory_free(kb)",                    memory_free),
			LogUint("memory_buff(kb)",                    memory_buff),
			LogUint("memory_cache(kb)",                   memory_cache),
			LogUint("swap_si(kb/s)",                      swap_si),
			LogUint("swap_so(kb/s)",                      swap_so),
			LogUint("io_bi(block/s)",                     io_bi),
			LogUint("io_bo(block/s)",                     io_bo),
			LogUint("system_in",                          system_in),
			LogUint("system_cs",                          system_cs),
			LogUint("cpu_us(%)",                          cpu_us),
			LogUint("cpu_sy(%)",                          cpu_sy),
			LogUint("cpu_id(%)",                          cpu_id),
			LogUint("cpu_wa(%)",                          cpu_wa),
			LogFloat("[process] cpu_usage(%)",            percent_cpu_usage),
			LogFloat("[process] mem_usage(%)",            percent_mem_usage),
			LogUint("[process] mem_usage(kb)",            kb_mem_usage),
			LogUint("[process] rchar(b)",                 io->rchar),
			LogUint("[process] wchar(b)",                 io->wchar),
			LogUint("[process] syscr(b)",                 io->syscr),
			LogUint("[process] syscw(b)",                 io->syscw),
			LogUint("[process] read_bytes(b)",            io->read_bytes),
			LogUint("[process] write_bytes(b)",           io->write_bytes),
			LogUint("[process] cancelled_write_bytes(b)", io->cancelled_write_bytes)
		);
}

static void *monitor_func(void *arg)
{
	ThreadArg_t *targ = (ThreadArg_t *)arg;
	struct timespec sleepTime;
	sleepTime.tv_sec  = targ->interval / 1000;
	sleepTime.tv_nsec = (targ->interval % 1000) * 1000;
	while(true) {
		pthread_mutex_lock(&targ->lock);
		_diagnosis();
		pthread_mutex_unlock(&targ->lock);
		nanosleep(&sleepTime, NULL);
	}
	return NULL;
}

// -------------------------------------------------------------------------
// ** parse args **

static struct option long_options2[] = {
	/* These options set a flag. */
	{"verbose",       no_argument,       &verbose_debug,  1},
	{"verbose:gc",    no_argument,       &verbose_gc,     1},
	{"verbose:sugar", no_argument,       &verbose_sugar,  1},
	{"verbose:code",  no_argument,       &verbose_code,   1},
	{"monitor",       no_argument,       &enable_monitor, 1},
	{"typecheck",     no_argument,       0,             'c'},
	{"interactive",   no_argument,       0,             'i'},
	{"builtin-test",  required_argument, 0,             'B'},
	{"define",        required_argument, 0,             'D'},
	{"import",        required_argument, 0,             'I'},
	{"interval",      required_argument, 0,             'N'},
	{"startwith",     required_argument, 0,             'S'},
	{"test",          required_argument, 0,             'T'},
	{"test-with",     required_argument, 0,             'T'},
	{NULL,            0,                 0,               0},
};

static int konoha_parseopt(KonohaContext* konoha, PlatformApiVar *plat, int argc, char **argv)
{
	char lineOfArgs[128];
	char *p = lineOfArgs;
	*p = '\0';
	int i = 0;
	size_t len;
	while(i < argc) {
		len = strlen(argv[i]);
		memcpy(p, argv[i], len);
		i++;
		p += len;
		if(i >= argc) break;
		p[0] = ' ';
		p++;
	}
	p[0] = '\0';

	kbool_t ret = true;
	long interval = 1000; // [ms]
	int scriptidx = 0;
	while (1) {
		int option_index = 0;
		int c = getopt_long(argc, argv, "ciD:I:S:", long_options2, &option_index);
		if (c == -1) break; /* Detect the end of the options. */
		switch (c) {
		case 0:
			/* If this option set a flag, do nothing else now. */
			if (long_options2[option_index].flag != 0)
				break;
			printf ("option %s", long_options2[option_index].name);
			if (optarg)
				printf (" with arg %s", optarg);
			printf ("\n");
			break;

		case 'c': {
			compileonly_flag = 1;
			KonohaContext_setCompileOnly(konoha);
		}
		break;

		case 'i': {
			interactive_flag = 1;
			KonohaContext_setInteractive(konoha);
		}
		break;

		case 'B':
			return CommandLine_doBuiltInTest(konoha, optarg);

		case 'D':
			CommandLine_define(konoha, optarg);
			break;

		case 'I':
			CommandLine_import(konoha, optarg);
			break;

		case 'S':
			konoha_startup(konoha, optarg);
			break;

		case 'N':
			if(!enable_monitor) {
				enable_monitor = 1;
			}
			interval = strtol(optarg, NULL, 0);
			if(interval < 1) {
				// default interval time is 1000[ms]
				interval = 1000;
			}
			break;

		case 'T':
//			DUMP_P ("option --test-with `%s'\n", optarg);
			verbose_debug = 0;
			verbose_sugar = 0;
			verbose_gc    = 0;
			verbose_code  = 0;
			plat->debugPrintf = NOP_debugPrintf;
			plat->printf_i  = TEST_printf;
			plat->vprintf_i = TEST_vprintf;
			plat->beginTag  = TEST_begin;
			plat->endTag    = TEST_end;
			plat->shortText = TEST_shortText;
			plat->reportCaughtException = TEST_reportCaughtException;
			return KonohaContext_test(konoha, optarg);

		case '?':
			/* getopt_long already printed an error message. */
			break;

		default:
			return 1;
		}
	}
	scriptidx = optind;
	CommandLine_setARGV(konoha, argc - scriptidx, argv + scriptidx);

	pid_t thispid = getpid();
	Page_size = getpagesize(); // for mem usage
	i = Page_size;
	while(i > 1024) {
		i >>= 1;
		page_to_kb_shift++;
	}
	openlog("loggerkonoha", LOG_PID, LOG_LOCAL7); // for using syslog
	void *arg;
	char *scriptname = (argv[scriptidx]) ? argv[scriptidx] : "";
	trace(&arg,
			LogText("@", "start"),
			LogText("scriptname", scriptname),
			LogText("argv", lineOfArgs),
			LogUint("pid", thispid),
			LogUint("ppid", getppid()),
			LogUint("uid", getuid())
		 );
	_diagnosis();

	pthread_t logging_thread;
	if(enable_monitor) {
		ThreadArg_t targ;
		targ.interval = interval;
		pthread_mutex_init(&targ.lock, NULL);
		pthread_create(&logging_thread, NULL, monitor_func, &targ);
	}


	if(scriptidx < argc) {
		ret = konoha_load(konoha, argv[scriptidx]);
	}
	else {
		interactive_flag = 1;
		KonohaContext_setInteractive(konoha);
	}
	if(ret && interactive_flag) {
		CommandLine_import(konoha, "konoha.i");
		ret = konoha_shell(konoha);
	}
	if(enable_monitor) {
		pthread_cancel(logging_thread);
	}
	return (ret == true) ? 0 : 1;
}

// -------------------------------------------------------------------------
// ** main **

int main(int argc, char *argv[])
{
	kbool_t ret = 1;
	if(getenv("KONOHA_DEBUG") != NULL) {
		verbose_debug = 1;
		verbose_gc = 1;
		verbose_sugar = 1;
		verbose_code = 1;
	}
	PlatformApi *logger_platform = KonohaUtils_getDefaultPlatformApi();
	PlatformApiVar *logger_platformVar = (PlatformApiVar *)logger_platform;
	logger_platformVar->diagnosis = _diagnosis;
	KonohaContext* konoha = konoha_open(logger_platform);
	ret = konoha_parseopt(konoha, logger_platformVar, argc, argv);
	konoha_close(konoha);
	return ret ? konoha_detectFailedAssert: 0;
}

#ifdef __cplusplus
}
#endif
