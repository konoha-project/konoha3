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

#ifndef MINIOKNOHA_H_
#define MINIOKNOHA_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define K_CLASSTABLE_INITSIZE 64
#define K_PAGESIZE        4096

#define K_VERSION   "0.2"
#define K_MAJOR_VERSION 0
#define K_MINOR_VERSION 2
#define K_PATCH_LEVEL   0

#include<minikonoha/stardate.h>
#define K_DATE      ((K_YEAR - 2006) * (12 * 24 * 32) + (K_MONTH * (24 * 32) + ((K_DAY * 24) + K_HOUR)))

#ifndef K_REVISION
#define K_REVISION 0
#endif

#ifndef K_PROGNAME
#define K_PROGNAME  "Mini Konoha"
/* - 2012/06/14 */
//#define K_CODENAME "Miyajima"
/*2012/06/14 -  */
//#define K_CODENAME "The Summer Palace, Beijing"
/*2012/09/22 -  */
#define K_CODENAME "Old Riga"
#else
#define K_CODENAME "based on MiniKonoha-" K_VERSION
#endif

#define USE_UTF8 1

#ifdef K_USE_PTHREAD
#if defined(__linux__) && !defined(__USE_UNIX98)
#define __USE_UNIX98 1
#endif
#endif

#if defined(HAVE_CONFIG_H) && !defined(HAVE_BZERO)
#define bzero(s, n) memset(s, 0, n)
#endif

#ifndef PLATAPIFORM_KERNEL
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#else
#include "platform_lkm.h"
#endif /* PLATAPIFORM_KERNEL */

#include <stddef.h>
#include <stdarg.h>

#ifndef __KERNEL__
#include <limits.h>
#include <float.h>
#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#else
#include <minikonoha/stdbool.h>
#endif
#include <stdint.h>
#endif

#ifdef __GCC__
#define __PRINTFMT(idx1, idx2) __attribute__((format(printf, idx1, idx2)))
#else
#define __PRINTFMT(idx1, idx2)
#endif

#ifdef _MSC_VER
#pragma warning(disable:4013)
#pragma warning(disable:4018)
#pragma warning(disable:4033)
#pragma warning(disable:4100)
#pragma warning(disable:4101)
#pragma warning(disable:4114)
#pragma warning(disable:4127)
#pragma warning(disable:4201)
#pragma warning(disable:4204)
#pragma warning(disable:4431)
#pragma warning(disable:4820)

#define inline __inline
typedef long long ssize_t;
#define __func__ __FUNCTION__
#endif

/*
 * differ prototype definition in *BSD and linux.
 */

#if defined(__NetBSD__)
#define	ICONV_INBUF_CONST	const
#else
#define	ICONV_INBUF_CONST
#endif

/* ------------------------------------------------------------------------ */
/* datatype */

#if defined(__LP64__) || defined(_WIN64)
#define USE_SYS64    1
typedef int32_t           kshort_t;
typedef uint32_t          kushort_t;
#ifdef USE_NOFLOAT
typedef uintptr_t         kfloat_t;
#else
typedef double            kfloat_t;
#endif

#ifndef LLONG_MIN
#define LLONG_MIN -9223372036854775807LL
#define LLONG_MAX  9223372036854775807LL
#endif

#ifndef ULLONG_MAX
#define ULLONG_MAX 18446744073709551615ULL
#endif

#define KINT_MAX               LLONG_MAX
#define KINT_MIN               LLONG_MIN
#define KINT_FMT               "%lld"

#else /* 32 */
#define USE_SYS32    1
typedef int16_t           kshort_t;
typedef uint16_t          kushort_t;
#ifdef USE_NOFLOAT
typedef uintptr_t         kfloat_t;
#else
typedef float             kfloat_t;
#endif

#define KINT_MAX               LONG_MAX
#define KINT_MIN               LONG_MIN
#define KINT_FMT               "%ld"
#define KINT0                  0UL

#endif

typedef bool             kbool_t;

typedef enum {
	K_FAILED, K_BREAK, K_CONTINUE
} kstatus_t;

typedef intptr_t         kint_t;
typedef uintptr_t        kuint_t;

typedef kushort_t        kshortflag_t;    /* flag field */

#define KFLAG_H(N)               ((sizeof(kshortflag_t)*8)-N)
#define KFLAG_H0                 ((kshortflag_t)(1 << KFLAG_H(1)))
#define KFLAG_H1                 ((kshortflag_t)(1 << KFLAG_H(2)))
#define KFLAG_H2                 ((kshortflag_t)(1 << KFLAG_H(3)))
#define KFLAG_H3                 ((kshortflag_t)(1 << KFLAG_H(4)))
#define KFLAG_H4                 ((kshortflag_t)(1 << KFLAG_H(5)))
#define KFLAG_H5                 ((kshortflag_t)(1 << KFLAG_H(6)))
#define KFLAG_H6                 ((kshortflag_t)(1 << KFLAG_H(7)))
#define KFLAG_H7                 ((kshortflag_t)(1 << KFLAG_H(8)))

#define KFlag_Is(T,f,op)          (((T)(f) & (T)(op)) == (T)(op))
#define KFlag_Set1(T,f,op)        f = (((T)(f)) | ((T)(op)))
#define KFlag_Set0(T,f,op)        f = (((T)(f)) & (~((T)(op))))
#define KFlag_Set(T,f,op,b)       if(b) {KFlag_Set1(T,f,op);} else {KFlag_Set0(T,f,op);}

#define FLAG_set(f,op)            KFlag_Set1(kshortflag_t,f,op)
#define FLAG_unset(f,op)          KFlag_Set0(kshortflag_t,f,op)
#define FLAG_is(f,op)             KFlag_Is(kshortflag_t,f,op)

/* fullsize id */
typedef uintptr_t                     kfileline_t;
#define NOPLINE                       0

/* halfsize id */
typedef kushort_t       kpackageId_t;     /* package id*/
typedef kushort_t       ktypeattr_t;      /* cid classid, ty type */
typedef kushort_t       ksymbol_t;
typedef kushort_t       kmethodn_t;
typedef kushort_t       kparamId_t;

typedef struct {
	uintptr_t flag;
	ksymbol_t symbol;
} KonohaFlagSymbolData;

/* ktypeattr_t */
#define TypeAttr_NewId          ((ktypeattr_t)-1)

#define TypeAttr_Attr(t)        ((t)  & (KFLAG_H0|KFLAG_H1|KFLAG_H2|KFLAG_H3))
#define TypeAttr_Unmask(t)      ((t)  & (~(KFLAG_H0|KFLAG_H1|KFLAG_H2|KFLAG_H3)))

#define TypeAttr_Boxed      KFLAG_H0    /* KeyValueStore, Field */
#define TypeAttr_ReadOnly   KFLAG_H1    /* Variable, Field */
#define TypeAttr_Coercion   KFLAG_H2    /* Variable, Field */

#define TypeAttr_Is(P, t)   (((t) & TypeAttr_##P) == TypeAttr_##P)

#define KSymbol_MAX            KFLAG_H3
#define KSymbol_Attr(sym)      (sym  & (KFLAG_H0|KFLAG_H1|KFLAG_H2|KFLAG_H3))
#define KSymbol_Unmask(sym)    (sym & (~(KFLAG_H0|KFLAG_H1|KFLAG_H2|KFLAG_H3)))

#define KSymbol_Noname          ((ksymbol_t)-1)
#define KSymbol_NewId           ((ksymbol_t)-2)
#define KSymbol_NewRaw          ((ksymbol_t)-3)
#define _NEWID                 KSymbol_NewId
#define _NEWRAW                KSymbol_NewRaw

#define SymbolAttr_Annotation        (KFLAG_H1|KFLAG_H2)
#define SymbolAttr_Pattern           (KFLAG_H0|KFLAG_H1|KFLAG_H2)
#define KSymbol_IsAnnotation(S)       ((S & SymbolAttr_Pattern) == SymbolAttr_Annotation)
#define KSymbol_IsPattern(S)          ((S & SymbolAttr_Pattern) == SymbolAttr_Pattern)

/* MethodName
 * 110   to$(TypeId)
 * 101   as${TypeId}
 * 010   get${symbol}
 * 001   set${symbol}
 */

#define MethodNameAttr_Type       KFLAG_H0
#define MethodNameAttr_Coercion   (KFLAG_H0|KFLAG_H1)
#define MethodNameAttr_Upcast     (KFLAG_H0|KFLAG_H2)
#define MethodNameAttr_Getter     KFLAG_H1
#define MethodNameAttr_Setter     KFLAG_H2

#define MethodName_Unmask(mn)        (mn & (~(KFLAG_H1|KFLAG_H2)))
#define MethodName_IsGetter(mn)      (KSymbol_Attr(mn) == MethodNameAttr_Getter)
#define MethodName_ToGetter(mn)      ((MethodName_Unmask(mn)) | MethodNameAttr_Getter)
#define MethodName_IsSetter(mn)      (KSymbol_Attr(mn) == MethodNameAttr_Setter)
#define MethodName_ToSetter(mn)      ((MethodName_Unmask(mn)) | MethodNameAttr_Setter)

#define MethodName_To(cid)            ((cid) | MethodNameAttr_Coercion)
#define MethodName_IsCoercion(mn)     ((KSymbol_Unmask(mn)) == MethodNameAttr_Coercion)
#define MethodName_As(cid)            ((cid) | MethodNameAttr_Upcast)
#define MethodName_IsUpcast(mn)       ((KSymbol_Unmask(mn)) == MethodNameAttr_Upcast)
#define MethodName_Fmt2(mn)           KSymbol_prefixText(mn), ((mn & MethodNameAttr_Type) == MethodNameAttr_Type ? KType_text(KSymbol_Unmask(mn)) : KSymbol_text(KSymbol_Unmask(mn)))

/* ------------------------------------------------------------------------ */
/* platform */

typedef const struct KonohaContextVar   KonohaContext;
typedef struct KonohaContextVar         KonohaContextVar;

typedef const struct KonohaFactory  PlatformApi;
typedef struct KonohaFactory        KonohaFactory;
typedef const struct KonohaLibVar    KonohaLib;
typedef struct KonohaLibVar          KonohaLibVar;

#define TEXTSIZE(T)   T, (sizeof(T) - 1)
#define PLATAPI       (kctx->platApi)->
#define KLIB          (kctx->klib)->
#define KLIBDECL      static

typedef enum {
	VerboseModule, ReleaseModule, ExperimentalModule
} ModuleType;

typedef kbool_t (*ModuleLoadFunc)(KonohaFactory *, ModuleType);

#define KDEFINE_PACKAGE KPackageHandler
typedef struct KPackageHandlerVar KPackageHandler;
typedef KPackageHandler* (*PackageLoadFunc)(void);

#define ICONV_NULL ((uintptr_t)-1)

#ifndef jmpbuf_i
#include <setjmp.h>
#define jmpbuf_i jmp_buf
#if defined(__MINGW64__)
static inline int setjmp_mingw(_JBTYPE* t)
{
	return _setjmp(t, NULL);
}
#define ksetjmp  setjmp_mingw
#elif defined(__MINGW32__) || defined(_MSC_VER)
#define ksetjmp  _setjmp
#else
#define ksetjmp  setjmp
#endif
#define klongjmp longjmp
#endif /*jmpbuf_i*/

#ifdef _MSC_VER
#include <malloc.h>
#endif
#define ALLOCA(T, SIZE) ((T *)alloca((SIZE) * sizeof(T)))

#ifndef K_USE_PTHREAD
typedef void *kthread_t;
typedef void kthread_attr_t;
typedef void kmutex_t;
typedef void kmutexattr_t;
typedef void kmutex_cond_t;
typedef void kmutex_condattr_t;
#define KInitLock(X)
#define KInitRrcureiveLock(X)
#define KLock(X)
#define KUnlock(X)
#define KFreeLock(X)
#else
#include <pthread.h>
typedef pthread_t           kthread_t;
typedef pthread_attr_t      kthread_attr_t;
typedef pthread_mutex_t     kmutex_t;
typedef pthread_mutexattr_t kmutexattr_t;
typedef pthread_condattr_t  kmutex_condattr_t;
typedef pthread_cond_t      kmutex_cond_t;
#define KInitLock(X)    do {\
	X = (kmutex_t *)KCalloc_UNTRACE(sizeof(kmutex_t), 1);\
	PLATAPI pthread_mutex_init_i(X, NULL);\
} while(0)

#define KInitRrcureiveLock(X)    PLATAPI pthread_mutex_init_recursive(X)
#define KLock(X)        PLATAPI pthread_mutex_lock_i(X)
#define KUnlock(X)      PLATAPI pthread_mutex_unlock_i(X)
#define KFreeLock(X)    do {\
	PLATAPI pthread_mutex_destroy_i(X);\
	KFree(X, sizeof(kmutex_t));\
	X = NULL;\
} while(0)

#endif

typedef enum {
	CritTag, ErrTag, WarnTag, NoticeTag, InfoTag, DebugTag, NoneTag
} kinfotag_t;

typedef enum {
	Unrecord = 0,
	isRecord = 1,
	// ErrorPoint
	SystemFault        =  (1<<1),  /* os, file system, etc. */
	SoftwareFault      =  (1<<2),  /* programmer's mistake */
	UserFault          =  (1<<3),  /* user input, data mistake */
	ExternalFault      =  (1<<4),  /* networking or remote services */
//	UnknownFault       =  (1<<5),  /* if you can distingish fault above */
	// LogPoint
	PeriodicPoint      =  (1<<6),  /* sampling */
	ResponseCheckPoint =  (1<<7),  /* log point to expect a long term action time*/
	SystemChangePoint  =  (1<<8),  /* log point to make permament change on systems */
	SecurityAudit      =  (1<<9),  /* security audit */
	// Otehr flag
	PrivacyCaution     =  (1<<10), /* including privacy information */
	// Internal Use
	SystemError        =  (1<<11),
	NotSystemFault     =  (1<<12),
	NotSoftwareFault   =  (1<<13),
	NotUserFault       =  (1<<14),
	NotExternalFault   =  (1<<15),
	LOGPOOL_INIT       =  (1<<17)
} logpolicy_t;

typedef struct logconf_t {
	logpolicy_t policy;
	void *formatter; // for precompiled formattings
} logconf_t;

typedef struct KModuleInfo {
	const char *name;
	const char *version;
	int patchlevel;
	const char *desc;
} KModuleInfo;

typedef struct KTraceInfo {
	struct KonohaValueVar *baseStack;
	kfileline_t pline;
	int errorSymbol;  int faultType;
} KTraceInfo;

#define KMakeTrace(TRACENAME, sfp) \
	KTraceInfo TRACENAME##REF_ = {sfp, sfp[K_RTNIDX].calledFileLine}, *TRACENAME = &TRACENAME##REF_;

#define KBaseTrace(TRACENAME) \
	KonohaStack *sfp_ = kctx->esp + K_CALLDELTA;\
	KTraceInfo TRACENAME##REF_ = {sfp_, 0}, *TRACENAME = &TRACENAME##REF_;

#define KMakeTraceUL(TRACENAME, sfp, UL) \
	KTraceInfo TRACENAME##REF_ = {sfp, UL}, *TRACENAME = &TRACENAME##REF_;

#define KBeginCritical(trace, T, F) \
	int errorKSymbol_ = trace->errorSymbol, faultType_ = trace->faultType;\
	trace->errorSymbol = EXPT_(T); trace->faultType = F

#define KEndCritical(trace) \
	trace->errorSymbol = errorKSymbol_; trace->faultType = faultType_

#define Trace_pline(trace) (trace == NULL ? 0 : trace->pline)

typedef uint64_t KJson_t;
struct JsonBuf;

typedef enum {
    KJSON_OBJECT,
    KJSON_ARRAY,
    KJSON_STRING,
    KJSON_INT,
    KJSON_DOUBLE,
    KJSON_BOOLEAN,
    KJSON_NULL,
    KJSON_INT64,
    KJSON_LONG
} KJSONTYPE;

typedef enum {
	SafePoint_NOWAIT = 0,
	SafePoint_GC     = 1,
	SafePoint_Event  = (1 << 2)
} SafePoint;

struct KObjectVisitor *visitor;

/* ------------------------------------------------------------------------ */

#define kAbstractObject                 const void
typedef const struct kObjectVar         kObject;
typedef struct kObjectVar               kObjectVar;
typedef const struct kBooleanVar        kBoolean;
typedef struct kBooleanVar              kBooleanVar;
typedef const struct kIntVar            kInt;
typedef struct kIntVar                  kIntVar;
typedef const struct kStringVar         kString;
typedef struct kStringVar               kStringVar;
typedef const struct kArrayVar          kArray;
typedef struct kArrayVar                kArrayVar;
typedef const struct kParamVar          kParam;
typedef struct kParamVar                kParamVar;
typedef const struct kMethodVar         kMethod;
typedef struct kMethodVar               kMethodVar;
typedef const struct kFuncVar           kFunc;
typedef struct kFuncVar                 kFuncVar;
typedef struct kNameSpaceVar            kNameSpace;
typedef struct kNameSpaceVar            kNameSpaceVar;

/* sugar.h */
typedef const struct kTokenVar          kToken;
typedef struct kTokenVar                kTokenVar;
typedef const struct kExprVar           kExpr;
typedef struct kExprVar                 kExprVar;
typedef const struct kStmtVar           kStmt;
typedef const struct kStmtVar           kStmtNULL;  // Nullable
typedef struct kStmtVar                 kStmtVar;
typedef const struct kBlockVar          kBlock;
typedef struct kBlockVar                kBlockVar;
typedef struct kGammaVar                kGamma;
typedef struct kGammaVar                kGammaVar;


struct KonohaFactory {
	// settings
	const char *name;
	size_t       stacksize;
	volatile int safePointFlag;
	int          verbose;
	int          exitStatus;

	/* memory allocation / deallocation */
	void *(*malloc_i)(size_t size);
	void  (*free_i)  (void *ptr);

	// setjmp
	int     (*setjmp_i)(jmpbuf_i);
	void    (*longjmp_i)(jmpbuf_i, int);

	// system info
	const char* (*getenv_i)(const char *);

	// time
	unsigned long long (*getTimeMilliSecond)(void);

	/* message */
	int     (*printf_i)(const char *fmt, ...) __PRINTFMT(2, 3);
	int     (*vprintf_i)(const char *fmt, va_list args);
	int     (*snprintf_i)(char *str, size_t size, const char *fmt, ...);
	int     (*vsnprintf_i)(char *str, size_t size, const char *fmt, va_list args);

	void    (*qsort_i)(void *base, size_t nel, size_t width, int (*compar)(const void *, const void *));
	// abort
	void    (*exit_i)(int p, const char *file, int line);

	// pthread
	int     (*pthread_create_i)(kthread_t *thread, const kthread_attr_t *attr, void *(*f)(void *), void *arg);
	int     (*pthread_join_i)(kthread_t thread, void **);
	int     (*pthread_mutex_init_i)(kmutex_t *mutex, const kmutexattr_t *attr);
	int     (*pthread_mutex_lock_i)(kmutex_t *mutex);
	int     (*pthread_mutex_trylock_i)(kmutex_t *mutex);
	int     (*pthread_mutex_unlock_i)(kmutex_t *mutex);
	int     (*pthread_mutex_destroy_i)(kmutex_t *mutex);
	int     (*pthread_mutex_init_recursive)(kmutex_t *mutex);
	int     (*pthread_cond_init_i)(kmutex_cond_t *cond, const kmutex_condattr_t *attr);
	int     (*pthread_cond_wait_i)(kmutex_cond_t *cond, kmutex_t *mutex);
	int     (*pthread_cond_signal_i)(kmutex_cond_t *cond);
	int     (*pthread_cond_broadcast_i)(kmutex_cond_t *cond);
	int     (*pthread_cond_destroy_i)(kmutex_cond_t *cond);

	/* high-level functions */
	kbool_t  (*LoadPlatformModule)(struct KonohaFactory*, const char *moduleName, ModuleType);

	// file load
	const char* (*FormatPackagePath)(KonohaContext *, char *buf, size_t bufsiz, const char *packageName, const char *ext);
	KPackageHandler* (*LoadPackageHandler)(KonohaContext *, const char *packageName);
	void (*BEFORE_LoadScript)(KonohaContext *, const char *filename);
	int (*loadScript)(const char *filePath, long uline, void *thunk, int (*evalFunc)(const char*, long, int *, void *));
	void (*AFTER_LoadScript)(KonohaContext *, const char *filename);

	const char* (*shortFilePath)(const char *path);
	const char* (*formatTransparentPath)(char *buf, size_t bufsiz, const char *parent, const char *path);

	// message (cui)
	char*  (*readline_i)(const char *prompt);
	int    (*add_history_i)(const char *);

	/* Logging API */
	KModuleInfo *LoggerInfo;
	void  (*syslog_i)(int priority, const char *message, ...) __PRINTFMT(2, 3);
	void  (*vsyslog_i)(int priority, const char *message, va_list args);
	void  (*TraceDataLog)(KonohaContext *kctx, KTraceInfo *trace, int, logconf_t *, ...);

	/* Diagnosis API */
	KModuleInfo *DiagnosisInfo;
	kbool_t (*CheckStaticRisk)(KonohaContext *, const char *keyword, size_t keylen, kfileline_t uline);
	void    (*CheckDynamicRisk)(KonohaContext *, const char *keyword, size_t keylen, KTraceInfo *);
	int (*DiagnosisSoftwareProcess)(KonohaContext *, kfileline_t uline, KTraceInfo *);
	int (*DiagnosisSystemError)(KonohaContext *, int fault);
	int (*DiagnosisSystemResource)(KonohaContext *, KTraceInfo *);
	int (*DiagnosisFileSystem)(KonohaContext *, const char *path, size_t pathlen, KTraceInfo *);
	int (*DiagnosisNetworking)(KonohaContext *, const char *path, size_t pathlen, int port, KTraceInfo *);
	kbool_t (*DiagnosisCheckSoftwareTestIsPass)(KonohaContext *, const char *filename, int line);

	/* Console API */
	KModuleInfo *ConsoleInfo;
	void  (*ReportUserMessage)(KonohaContext *, kinfotag_t, kfileline_t pline, const char *, int isNewLine);
	void  (*ReportCompilerMessage)(KonohaContext *, kinfotag_t, kfileline_t pline, const char *);
	void  (*ReportCaughtException)(KonohaContext *, const char *, int fault, const char *, struct KonohaValueVar *bottomStack, struct KonohaValueVar *topStack);
	void  (*ReportDebugMessage)(const char *file, const char *func, int line, const char *fmt, ...) __PRINTFMT(4, 5);
	int   (*InputUserApproval)(KonohaContext *, const char *message, const char *yes, const char *no, int defval);
	char* (*InputUserText)(KonohaContext *, const char *message, int flag);
	char* (*InputUserPassword)(KonohaContext *, const char *message);

	/* Garbage Collection API */
	KModuleInfo *GCInfo;
	void* (*Kmalloc)(KonohaContext*, size_t, KTraceInfo *);
	void* (*Kzmalloc)(KonohaContext*, size_t, KTraceInfo *);
	void  (*Kfree)(KonohaContext*, void *, size_t);
	void (*InitGcContext)(KonohaContext *kctx);
	void (*DeleteGcContext)(KonohaContext *kctx);
	void (*ScheduleGC)(KonohaContext *kctx, KTraceInfo *trace);
	struct kObjectVar *(*AllocObject)(KonohaContext *kctx, size_t size, KTraceInfo *);
	kbool_t (*IsKonohaObject)(KonohaContext *kctx, void *ptr);
	void  (*VisitObject)(struct KObjectVisitor *visitor, struct kObjectVar *obj);
	void  (*WriteBarrier)(KonohaContext *, const struct kObjectVar *);
	void  (*UpdateObjectField)(const struct kObjectVar *parent, const struct kObjectVar *oldPtr, const struct kObjectVar *newVal);

	/* Event Handler API */
	KModuleInfo *EventInfo;
	struct EventContext *eventContext;
	void (*StartEventHandler)(KonohaContext *kctx, void *args);
	void (*StopEventHandler)(KonohaContext *kctx);
	void (*EnterEventContext)(KonohaContext *kctx);
	void (*ExitEventContext)(KonohaContext *kctx);
	kbool_t (*EmitEvent)(KonohaContext *kctx, struct JsonBuf *json, KTraceInfo *);
	void (*DispatchEvent)(KonohaContext *kctx, kbool_t (*consume)(KonohaContext *kctx, struct JsonBuf *, KTraceInfo *), KTraceInfo *);
	void (*WaitEvent)(KonohaContext *kctx, kbool_t (*consume)(KonohaContext *kctx, struct JsonBuf *, KTraceInfo *), KTraceInfo *);

	// I18N Module
	KModuleInfo *I18NInfo;
	uintptr_t   (*iconv_open_i)(KonohaContext *, const char* tocode, const char* fromcode, KTraceInfo *);
	size_t      (*iconv_i)(KonohaContext *, uintptr_t iconv, ICONV_INBUF_CONST char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft, int *isTooBigRef, KTraceInfo *trace);
	size_t      (*iconv_i_memcpyStyle)(KonohaContext *, uintptr_t iconv, char **outbuf, size_t *outbytesleft, ICONV_INBUF_CONST char **inbuf, size_t *inbytesleft, int *isTooBigRef, KTraceInfo *trace);
	int         (*iconv_close_i)(KonohaContext *, uintptr_t iconv);
	const char* systemCharset;
	kbool_t     (*isSystemCharsetUTF8)(KonohaContext *);
	uintptr_t   (*iconvUTF8ToSystemCharset)(KonohaContext *, KTraceInfo *);
	uintptr_t   (*iconvSystemCharsetToUTF8)(KonohaContext *, KTraceInfo *);
	const char* (*formatSystemPath)(KonohaContext *kctx, char *buf, size_t bufsiz, const char *path, size_t pathsize, KTraceInfo *);
	const char* (*formatKonohaPath)(KonohaContext *kctx, char *buf, size_t bufsiz, const char *path, size_t pathsize, KTraceInfo *);

	// CodeGenerator
	KModuleInfo  *CodeGeneratorInfo;
	void*       (*GetCodeGenerateMethodFunc)(void);
	void*       (*GenerateCode)(KonohaContext *kctx, kMethod *mtd, kBlock *bk, int options);

	// VirtualMachine
	KModuleInfo            *VirtualMachineInfo;
	kbool_t               (*IsSupportedVirtualCode)(int opcode);
//	struct VirtualCode *  (*RunVirtualMachine)(KonohaContext *kctx, struct KonohaValueVar *sfp, struct VirtualCode *pc);
//	void                  (*DeleteVirtualMachine)(KonohaContext *kctx);
//	void *                (*GetVirtualMachineMethodFunc)(void);
	struct VirtualCode*   (*GetDefaultBootCode)(void);
	struct KBuilderAPI2*  (*GetDefaultBuilderAPI)(void);

	/* JSON_API */
	KModuleInfo *JsonDataInfo;
	void        *JsonHandler;  // define this in each module if necessary
	kbool_t     (*IsJsonType)(struct JsonBuf *, KJSONTYPE);
	struct JsonBuf* (*CreateJson)(KonohaContext *, struct JsonBuf *jsonbuf, KJSONTYPE type, ...);
	kbool_t     (*ParseJson)(KonohaContext *, struct JsonBuf *, const char *, size_t, KTraceInfo *);
	void        (*FreeJson)(KonohaContext *, struct JsonBuf *);
	const char* (*JsonToNewText)(KonohaContext *, struct JsonBuf *);
	size_t      (*DoJsonEach)(KonohaContext *, struct JsonBuf *, void *thunk, void (*doEach)(KonohaContext *, const char *, struct JsonBuf *, void *));

	kbool_t     (*RetrieveJsonKeyValue)(KonohaContext *, struct JsonBuf *, const char *key, size_t keylen, struct JsonBuf *newbuf);
	kbool_t     (*SetJsonKeyValue)(KonohaContext *, struct JsonBuf *, const char *key, size_t keylen, struct JsonBuf *otherbuf);
	kbool_t     (*SetJsonValue)(KonohaContext *, struct JsonBuf *, const char *key, size_t keylen, KJSONTYPE, ...);

	kbool_t     (*GetJsonBoolean)(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, kbool_t defval);
	int64_t     (*GetJsonInt)(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, int64_t defval);
	double      (*GetJsonFloat)(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, double defval);
	const char* (*GetJsonText)(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, const char *defval);
	size_t      (*GetJsonSize)(KonohaContext *kctx, struct JsonBuf *jsonbuf);
	kbool_t     (*RetrieveJsonArrayAt)(KonohaContext *kctx, struct JsonBuf *jsonbuf, size_t index, struct JsonBuf *otherbuf);
	kbool_t     (*SetJsonArrayAt)(KonohaContext *kctx, struct JsonBuf *jsonbuf, size_t index, struct JsonBuf *otherbuf);
	kbool_t     (*AppendJsonArray)(KonohaContext *kctx, struct JsonBuf *jsonbuf, struct JsonBuf *otherbuf);

};

#define LOG_END   0
#define LOG_s     1
#define LOG_u     2
#define LOG_ERRNO 3
#define LOG_a     4 /*char **/

#define LogInt(K,V)     LOG_u, (K), ((intptr_t)V)
#define LogUint(K,V)    LOG_u, (K), ((uintptr_t)V)
#define LogText(K,V)    LOG_s, (K), (V)
#define LogTextArray(K,V)  LOG_a, (K), (V)
#define LogErrno        LOG_ERRNO

#define KTraceErrorPoint(TRACE, POLICY, APINAME, ...)    do {\
		logconf_t _logconf = {(logpolicy_t)(isRecord|LOGPOOL_INIT|POLICY)};\
		if(trace != NULL && KFlag_Is(int, _logconf.policy, isRecord)) { \
			PLATAPI TraceDataLog(kctx, TRACE, 0/*LOGKEY*/, &_logconf, LogText("Api", APINAME), ## __VA_ARGS__, LOG_END);\
		}\
	} while(0)

#define KTraceChangeSystemPoint(TRACE, APINAME, ...)    do {\
		logconf_t _logconf = {(logpolicy_t)(isRecord|LOGPOOL_INIT|SystemChangePoint)};\
		if(trace != NULL && KFlag_Is(int, _logconf.policy, isRecord)) { \
			PLATAPI TraceDataLog(kctx, TRACE, 0/*LOGKEY*/, &_logconf, LogText("Api", APINAME), ## __VA_ARGS__, LOG_END);\
		}\
	} while(0)

#define KTraceApi(TRACE, POLICY, APINAME, ...)    do {\
		static logconf_t _logconf = {(logpolicy_t)(isRecord|LOGPOOL_INIT|POLICY)};\
		if(trace != NULL && KFlag_Is(int, _logconf.policy, isRecord)) {\
			PLATAPI TraceDataLog(kctx, TRACE, 0/*LOGKEY*/, &_logconf, LogText("Api", APINAME), ## __VA_ARGS__, LOG_END);\
		}\
	} while(0)

#define KTraceResponseCheckPoint(TRACE, POLICY, APINAME, STMT, ...)    do {\
		static logconf_t _logconf = {(logpolicy_t)(isRecord|LOGPOOL_INIT|POLICY)};\
		if(trace != NULL && KFlag_Is(int, _logconf.policy, isRecord)) {\
			uint64_t _startTime = PLATAPI getTimeMilliSecond();\
			PLATAPI TraceDataLog(kctx, TRACE, 0/*LOGKEY*/, &_logconf, LogText("Api", APINAME), ## __VA_ARGS__, LOG_END);\
			STMT;\
			uint64_t _endTime = PLATAPI getTimeMilliSecond();\
			PLATAPI TraceDataLog(kctx, TRACE, 0/*LOGKEY*/, &_logconf, LogText("Api", APINAME), LogUint("ElapsedTime", (_endTime - _startTime)), ## __VA_ARGS__, LOG_END);\
			/*counter++;*/\
		}else { \
			STMT;\
		}\
	} while(0)


#define OLDTRACE_SWITCH_TO_KTrace(POLICY, ...)

/* ------------------------------------------------------------------------ */

typedef struct {
	ksymbol_t key;
	ktypeattr_t   attrTypeId;
	union {
		uintptr_t                unboxValue;  //unboxValue
		kObject                 *ObjectValue;  //ObjectValue
		kString                 *StringValue;  //stringValue
		kFunc                   *FuncValue;
	};
} KKeyValue;

#define COMMON_BYTEARRAY \
	size_t bytesize;\
	union {\
		const char *byteptr;\
		const char *text;\
		const unsigned char *utext;\
		char *buf;\
		unsigned char *ubuf;\
	}\

#define KARRAYSIZE(BS, T)   ((BS)/sizeof(T##_t))

typedef struct KGrowingArray {
	size_t bytesize;
	union {
		char                              *bytebuf;
		const struct KonohaClassVar      **classItems;
		KKeyValue                         *keyValueItems;
		kObject                          **ObjectItems;
	};
	size_t bytemax;
} KGrowingArray;

typedef struct KBuffer {
	KGrowingArray *m;
	size_t pos;
} KBuffer;

typedef struct KDict {
	KGrowingArray data;
	size_t sortedData;
} KDict;

typedef struct KProtoMap {
	intptr_t refc;
	struct KProtoMap *parent;
	struct KDict dict;
} KProtoMap;

typedef struct KHashMapEntry {
	uintptr_t hcode;
	struct KHashMapEntry *next;
	union {
		uintptr_t        key;
		uintptr_t        unboxKey;
		kString         *StringKey;
		kParam          *paramKey_OnList;
		void            *ptrKey;
	};
	union {
		kObject         *ObjectValue;
		uintptr_t        unboxValue;
		void            *ptrValue;
	};
} KHashMapEntry;

typedef struct KHashMap KHashMap;
struct KHashMap {
	KHashMapEntry  *arena;
	KHashMapEntry  *unused;
	KHashMapEntry **hentry;
	size_t          arenasize;
	size_t          size;
	size_t          hmax;
};

/* ------------------------------------------------------------------------ */

typedef const struct KonohaClassVar     KonohaClass;
typedef struct KonohaClassVar           KonohaClassVar;
typedef struct KonohaClassField         KonohaClassField;
typedef struct KonohaClassField         KonohaClassFieldVar;

typedef const struct KonohaRuntimeVar           KonohaRuntime;
typedef struct KonohaRuntimeVar                 KonohaRuntimeVar;
typedef const struct KonohaStackRuntimeVar      KonohaStackRuntime;
typedef struct KonohaStackRuntimeVar            KonohaStackRuntimeVar;
typedef struct KonohaValueVar                   KonohaStack;
typedef struct KonohaValueVar                   KonohaValue;

typedef struct KonohaModule        KonohaModule;
typedef struct KonohaModuleContext KonohaModuleContext;
struct KObjectVisitor;

struct KonohaContextVar {
	uintptr_t                         safepoint; // set to 1
	KonohaStack                      *esp;
	PlatformApi                      *platApi;
	KonohaLib                        *klib;
	KonohaRuntime                    *share;
	KonohaStackRuntimeVar            *stack;
	KonohaModule                    **modshare;
	KonohaModuleContext             **modlocal;
	void                             *gcContext; // defined in each module
};

// share, local
struct KonohaRuntimeVar {
	KGrowingArray             classTable;
	KHashMap                 *longClassNameMapNN;
	kmutex_t                 *classTableMutex;
	/* system shared const */
	kArray                   *GlobalConstList;
	kObject                  *constNull_OnGlobalConstList;
	kBoolean                 *constTrue_OnGlobalConstList;
	kBoolean                 *constFalse_OnGlobalConstList;
	kString                  *emptyString_OnGlobalConstList;
	kArray                   *emptyArray_OnGlobalConstList;

	kmutex_t                 *filepackMutex;
	kArray                   *fileIdList_OnGlobalConstList;    // file, http://
	KHashMap                 *fileIdMap_KeyOnList;   //

	kArray                   *packageIdList_OnGlobalConstList;
	KHashMap                 *packageIdMap_KeyOnList;
	KHashMap                 *packageMapNO;

	kmutex_t                 *symbolMutex;
	kArray                   *symbolList_OnGlobalConstList;  // NAME, Name, INT_MAX Int_MAX
	KHashMap                 *symbolMap_KeyOnList;

	kmutex_t                 *paramMutex;
	kArray                   *paramList_OnGlobalConstList;
	KHashMap                 *paramMap_KeyOnList;
	kArray                   *paramdomList_OnGlobalConstList;
	KHashMap                 *paramdomMap_KeyOnList;
};

#define kContext_Debug          ((kshortflag_t)(1<<0))
#define kContext_Interactive    ((kshortflag_t)(1<<1))
#define kContext_CompileOnly    ((kshortflag_t)(1<<2))
#define kContext_Test           ((kshortflag_t)(1<<3))
#define kContext_Trace          ((kshortflag_t)(1<<4))

#define KonohaContext_Is(P, X)   (KFlag_Is(kshortflag_t,(X)->stack->flag, kContext_##P))
#define KonohaContext_Set(P, X)   KFlag_Set1(kshortflag_t, (X)->stack->flag, kContext_##P)

struct KonohaStackRuntimeVar {
	KonohaStack*               stack;
	size_t                     stacksize;
	KonohaStack*               stack_uplimit;
	kArray                    *ContextConstList;
	kArray                    *gcstack_OnContextConstList;
	KGrowingArray              cwb;
	// local info
	kshortflag_t               flag;
	KonohaContext             *rootctx;
	void*                      cstack_bottom;  // for GC
	ktypeattr_t                    evalty;
	kushort_t                  evalidx;
	kString                   *OptionalErrorInfo;
	int                        faultInfo;
	jmpbuf_i                  *evaljmpbuf;
	KonohaStack               *bottomStack;
	KonohaStack               *topStack;
};

// module
#define KonohaModule_MAXSIZE    32
#define MOD_logger     0
#define MOD_gc         1
#define MOD_code       2
#define MOD_sugar      3
#define MOD_konoha     6

#define MOD_exception  5
#define MOD_float      11
#define MOD_iterator   12
#define MOD_iconv      13
#define MOD_IO         14
//#define MOD_llvm       15
#define MOD_REGEXP     16
#define MOD_APACHE     17
#define MOD_EVENT      18
#define MOD_mpi        19

struct KonohaModule {
	const char *name;
	int         mod_id;
	void        (*setupModuleContext)(KonohaContext*, struct KonohaModule *, int newctx);
	void        (*freeModule)(KonohaContext*, struct KonohaModule *);
	size_t      allocSize;
	kmutex_t   *moduleMutex;
};

struct KonohaModuleContext {
	uintptr_t unique;
	void (*reftrace)(KonohaContext*, struct KonohaModuleContext *, struct KObjectVisitor *);
	void (*free)(KonohaContext*, struct KonohaModuleContext *);
};

#define K_FRAME_NCMEMBER \
	uintptr_t   unboxValue;\
	kbool_t     boolValue; \
	kint_t      intValue; \
	kfloat_t    floatValue; \
	struct KonohaValueVar *previousStack;\
	intptr_t    shift0;  \
	struct VirtualCode  *pc; \
	kMethod     *calledMethod;\
	kNameSpace  *calledNameSpace;\
	uintptr_t    calledFileLine

#define K_FRAME_MEMBER \
	kObject     *asObject;\
	kObjectVar  *asObjectVar; \
	const struct kNumberVar     *asNumber;\
	kInt        *asInt; \
	kString     *asString;\
	kArray      *asArray;\
	kMethod     *asMethod;\
	kFunc       *asFunc; \
	kNameSpace  *asNameSpace;\
	kToken      *asToken;\
	kStmt       *asStmt;\
	kExpr       *asExpr;\
	kBlock      *asBlock;\
	kGamma      *asGamma;\
	const struct kExceptionVar  *asException;\
	const struct kFloatVar      *asFloat; \
	struct kDateVar             *asDate;\
	struct kRegExpVar           *asRegExp; \
	const struct kBytesVar      *asBytes; \
	struct kFileVar     *asFile;\
	struct kIteratorVar *asIterator; \
	struct kMap           *asMap;    \
	struct kInputStream  *asInputStream; \
	struct kOutputStream *asOutputStream;  \
	struct kContext           *asContext;\
	kint_t     dummy_intValue;\
	kfloat_t   dummy_floatValue

struct KonohaValueVar {
	union {
		K_FRAME_MEMBER;
	};
	union {
		K_FRAME_NCMEMBER;
	};
};

typedef struct krbp_t {
	union {
		K_FRAME_NCMEMBER;
		K_FRAME_MEMBER;
	};
} krbp_t;

typedef enum {
	ToStringFormat, JSONFormat
} kformat_t;

#define CLASSAPI \
		void         (*init)(KonohaContext*, kObject*, void *conf);\
		void         (*reftrace)(KonohaContext*, kObject*, struct KObjectVisitor *visitor);\
		void         (*free)(KonohaContext*, kObject *);\
		kObject*     (*fnull)(KonohaContext*, KonohaClass *);\
		uintptr_t    (*unbox)(KonohaContext*, kObject *);\
		void         (*p)(KonohaContext*, KonohaValue *, int, KBuffer *);\
		int          (*compareObject)(kObject*, kObject *);\
		int          (*compareUnboxValue)(uintptr_t, uintptr_t);\
		kbool_t      (*hasField)(KonohaContext*, kObject*, ksymbol_t, ktypeattr_t);\
		kObject*     (*getFieldObjectValue)(KonohaContext*, kObject*, ksymbol_t, ktypeattr_t);\
		void         (*setFieldObjectValue)(KonohaContext*, kObject*, ksymbol_t, ktypeattr_t, kObject *);\
		uintptr_t    (*getFieldUnboxValue)(KonohaContext*, kObject*, ksymbol_t, ktypeattr_t);\
		void         (*setFieldUnboxValue)(KonohaContext*, kObject*, ksymbol_t, ktypeattr_t, uintptr_t);\
		void         (*initdef)(KonohaContext*, KonohaClassVar*, KTraceInfo *);\
		kbool_t      (*isSubType)(KonohaContext*, KonohaClass*, KonohaClass *);\
		KonohaClass* (*realtype)(KonohaContext*, KonohaClass*, KonohaClass *)


typedef struct KDEFINE_CLASS {
	const char *structname;
	ktypeattr_t     typeId;         kshortflag_t    cflag;
	ktypeattr_t     baseTypeId;     ktypeattr_t         superTypeId;
	ktypeattr_t     rtype;          kushort_t       cparamsize;
	struct kparamtype_t   *cParamItems;
	size_t     cstruct_size;
	KonohaClassField   *fieldItems;
	kushort_t  fieldsize;       kushort_t fieldAllocSize;
	CLASSAPI;
} KDEFINE_CLASS;

#define STRUCTNAME(C) \
	.structname = #C,\
	.typeId = TypeAttr_NewId,\
	.cstruct_size = sizeof(k##C)

#define UNBOXNAME(C) \
	.structname = #C,\
	.typeId = TypeAttr_NewId

#define SETSTRUCTNAME(VAR, C) do{\
		VAR.structname = #C;\
		VAR.typeId = TypeAttr_NewId;\
		VAR.cstruct_size = sizeof(k##C);\
	}while(0)

#define SETUNBOXNAME(VAR, C) do{\
		VAR.structname = #C;\
		VAR.typeId = TypeAttr_NewId;\
	}while(0)

//KonohaClassVar;
typedef uintptr_t kmagicflag_t;

struct KonohaClassVar {
	CLASSAPI;
	kpackageId_t     packageId;    kpackageId_t    packageDomain;
	ktypeattr_t      typeId;       kshortflag_t    cflag;
	ktypeattr_t      baseTypeId;   ktypeattr_t     superTypeId;
	ktypeattr_t      p0;           kparamId_t      cparamdom;
	kmagicflag_t magicflag;
	size_t     cstruct_size;
	KonohaClassField         *fieldItems;
	kushort_t  fieldsize;     kushort_t fieldAllocSize;
	const char               *DBG_NAME;
	ksymbol_t   classNameSymbol;  kushort_t   optvalue;
	size_t      sortedMethodList;
	kArray     *classMethodList /* OnGlobalConstList*/;
	kString    *shortClassNameNULL /* OnGlobalConstList*/;
	union {   // default value
		kObject           *defaultNullValue    /*OnGlobalConstList*/;
		kObjectVar        *defaultNullValueVar /*OnGlobalConstList*/;
	};
	KonohaClass              *searchSimilarClassNULL;
	KonohaClass              *searchSuperMethodClassNULL;
};

struct KonohaClassField {
	ktypeattr_t     attrTypeId;
	ksymbol_t       name;
};

/* ----------------------------------------------------------------------- */

#define KType_void              ((ktypeattr_t)0)
#define KType_var               ((ktypeattr_t)1)
#define KType_Object            ((ktypeattr_t)2)
#define KType_boolean           ((ktypeattr_t)3)
#define KType_int               ((ktypeattr_t)4)
#define KType_String            ((ktypeattr_t)5)
#define KType_Array             ((ktypeattr_t)6)
#define KType_Param             ((ktypeattr_t)7)
#define KType_Method            ((ktypeattr_t)8)
#define KType_Func              ((ktypeattr_t)9)
#define KType_NameSpace         ((ktypeattr_t)10)
#define KType_System            ((ktypeattr_t)11)
#define KType_0                 ((ktypeattr_t)12)    /* Parameter Type*/

#define KClass_void                 KClass_(KType_void)
#define KClass_Object               KClass_(KType_Object)
#define KClass_Boolean              KClass_(KType_boolean)
#define KClass_Int                  KClass_(KType_int)
#define KClass_String               KClass_(KType_String)
#define KClass_Array                KClass_(KType_Array)
#define KClass_Param                KClass_(KType_Param)
#define KClass_Method               KClass_(KType_Method)
#define KClass_Func                 KClass_(KType_Func)
#define KClass_NameSpace            KClass_(KType_NameSpace)
#define KClass_System               KClass_(KType_System)

#define KClass_StringArray          KClass_Array
#define kStringArray                kArray
#define KClass_MethodArray          KClass_Array
#define kMethodArray                kArray

#define KClassFlag_TypeVar          ((kshortflag_t)(1<<0))
#define KClassFlag_UnboxType        ((kshortflag_t)(1<<1))
#define KClassFlag_Singleton        ((kshortflag_t)(1<<2))
#define KClassFlag_Immutable        ((kshortflag_t)(1<<3))
#define KClassFlag_Private          ((kshortflag_t)(1<<4))
#define KClassFlag_Nullable         ((kshortflag_t)(1<<5))
#define KClassFlag_Virtual          ((kshortflag_t)(1<<6))
#define KClassFlag_Newable          ((kshortflag_t)(1<<7))
#define KClassFlag_Final            ((kshortflag_t)(1<<8))
#define KClassFlag_Interface        ((kshortflag_t)(1<<9))
#define KClassFlag_Prototype        ((kshortflag_t)(1<<10))

#define CFLAG_SUPERMASK         KClassFlag_Prototype|KClassFlag_Singleton

#define CFLAG_void              KClassFlag_TypeVar|KClassFlag_UnboxType|KClassFlag_Singleton|KClassFlag_Final
#define CFLAG_var               KClassFlag_TypeVar|KClassFlag_UnboxType|KClassFlag_Singleton|KClassFlag_Final
#define CFLAG_Object            KClassFlag_Nullable
#define CFLAG_boolean           KClassFlag_Nullable|KClassFlag_Immutable|KClassFlag_UnboxType|KClassFlag_Final
#define CFLAG_int               KClassFlag_Nullable|KClassFlag_Immutable|KClassFlag_UnboxType|KClassFlag_Final
#define CFLAG_String            KClassFlag_Nullable|KClassFlag_Immutable|KClassFlag_Final
#define CFLAG_Array             KClassFlag_Nullable|KClassFlag_Final
#define CFLAG_Param             KClassFlag_Nullable|KClassFlag_Final
#define CFLAG_Method            KClassFlag_Nullable|KClassFlag_Final
#define CFLAG_Func              KClassFlag_Nullable|KClassFlag_Final
#define CFLAG_NameSpace         KClassFlag_Nullable|KClassFlag_Final
#define CFLAG_System            KClassFlag_Nullable|KClassFlag_Singleton|KClassFlag_Final
#define CFLAG_0                 KClassFlag_TypeVar|KClassFlag_UnboxType|KClassFlag_Singleton|KClassFlag_Final

#define KClass_(T)                kctx->share->classTable.classItems[TypeAttr_Unmask(T)]
#define KClass_cparam(CT)         kctx->share->paramdomList_OnGlobalConstList->ParamItems[(CT)->cparamdom]
#define KClass_Is(P, C)           (KFlag_Is(kshortflag_t, (C)->cflag, KClassFlag_##P))
#define KClass_Set(P, C, B)       KFlag_Set(kshortflag_t, (C)->cflag, KClassFlag_##P, B)

#define KType_Is(P, T)            (KFlag_Is(kshortflag_t, (KClass_(T))->cflag, KClassFlag_##P))

#define KType_IsFunc(T)         (KClass_(T)->baseTypeId == KType_Func)
#define KClass_isFunc(C)         ((C)->baseTypeId == KType_Func)

/* ------------------------------------------------------------------------ */
/* Object */

// common
#define kObjectFlag_NullObject       ((kmagicflag_t)(1<<0))
#define kObjectFlag_GCFlag           ((kmagicflag_t)(1<<1))
#define kObjectFlag_Reserved1        ((kmagicflag_t)(1<<2))  // ## reserved
#define kObjectFlag_Reserved2        ((kmagicflag_t)(1<<3))  // ## reserved

// local
#define kObjectFlag_Local6           ((kmagicflag_t)(1<<4))
#define kObjectFlag_Local5           ((kmagicflag_t)(1<<5))
#define kObjectFlag_Local4           ((kmagicflag_t)(1<<6))
#define kObjectFlag_Local3           ((kmagicflag_t)(1<<7))
#define kObjectFlag_Local2           ((kmagicflag_t)(1<<8))
#define kObjectFlag_Local1           ((kmagicflag_t)(1<<9))

#define kObject_Is(P, O, A)      (KFlag_Is(kmagicflag_t,(O)->h.magicflag, kObjectFlag_##P))
#define kObject_Set(P, O, B)     KFlag_Set(kmagicflag_t, ((kObjectVar *)O)->h.magicflag, kObjectFlag_##P, B)

#define kObject_HashCode(O)          (uintptr_t)((O)->h.magicflag >> (sizeof(kushort_t)*8))
#define kObject_flags(O)             ((kushort_t)((O)->h.magicflag))
#define kObject_SetHashCode(O, HASH) ((kObjectVar *)O)->h.magicflag = (((uintptr_t)HASH) << (sizeof(kushort_t)*8) | kObject_flags(O))

#define IS_NULL(o)                 ((((o)->h.magicflag) & kObjectFlag_NullObject) == kObjectFlag_NullObject)
#define IS_NOTNULL(o)              ((((o)->h.magicflag) & kObjectFlag_NullObject) != kObjectFlag_NullObject)

typedef struct KonohaObjectHeader {
	kmagicflag_t magicflag;
	KonohaClass *ct;
	KProtoMap *prototypePtr;
} KonohaObjectHeader;

struct kObjectVar {
	KonohaObjectHeader h;
	union {
		kObject  *fieldObjectItems[5];
		uintptr_t fieldUnboxItems[5];
	};
};

#define kObject_class(o)             ((o)->h.ct)
#define kObject_typeId(o)            (kObject_class(o)->typeId)
#define kObject_baseTypeId(o)        (kObject_class(o)->baseTypeId)
#define kObject_Unbox(o)             (kObject_class(o)->unbox(kctx, o))
#define kObject_p0(o)                (kObject_class(o)->p0)

/* ------------------------------------------------------------------------ */
/* Boolean */

#define ABSTRACT_NUMBER \
	union {\
		uintptr_t  unboxValue;\
		kbool_t    boolValue;\
		kint_t     intValue;\
		kfloat_t   floatValue;\
	}\

typedef const struct kNumberVar kNumber;
typedef struct kNumberVar       kNumberVar;

struct kNumberVar {
	KonohaObjectHeader h;
	ABSTRACT_NUMBER;
};

struct kBooleanVar /* extends kNumber */ {
	KonohaObjectHeader h;
	ABSTRACT_NUMBER;
};

#define IS_Boolean(o)              (kObject_typeId(o) == KType_boolean)
#define IS_TRUE(o)                 (kObject_baseTypeId(o) == KType_boolean && kNumber_ToBool(o))
#define IS_FALSE(o)                (kObject_baseTypeId(o) == KType_boolean && kNumber_ToBool(o) == 0)
#define new_Boolean(kctx, c)       ((c) ? K_TRUE : K_FALSE)
#define kNumber_ToInt(o)                 (((kBoolean *)o)->intValue)
#define kNumber_ToFloat(o)               (((kBoolean *)o)->floatValue)
#define kNumber_ToBool(o)                (((kBoolean *)o)->boolValue)

/* ------------------------------------------------------------------------ */
/* Int */

struct kIntVar /* extends kNumber */ {
	KonohaObjectHeader h;
	ABSTRACT_NUMBER;
};

#define IS_Int(o)              (kObject_typeId(o) == KType_int)

/* ------------------------------------------------------------------------ */
/* String */

typedef enum {
	VirtualType_Text                  =   KType_void,    /*special use for const char*/
	VirtualType_KonohaClass           =   KType_var,     /*special use for KonohaClass*/
	VirtualType_StaticMethod          =   KType_0        /*special use for Method*/
} VirtualType;

#define IS_String(o)              (kObject_typeId(o) == KType_String)

#define kStringFlag_RopeReserved   kString_Loacl1  /* Don't change */
#define kStringFlag_TextSgm        kObjectFlag_Local2  /* Don't change */
#define kStringFlag_MallocText     kObjectFlag_Local3  /* Don't change */
#define kStringFlag_ASCII          kObjectFlag_Local4
#define kStringFlag_Literal        kObjectFlag_Local5

#define kString_Is(P, o)             (KFlag_Is(uintptr_t,(o)->h.magicflag, kStringFlag_##P))
#define kString_Set(P, o, b)         KFlag_Set(uintptr_t,(o)->h.magicflag, kStringFlag_##P, b)
#define kString_GuessUserFault(S)    ((kString_Is(Literal, S)) ? 0 : UserFault)

#define SIZEOF_INLINETEXT    (sizeof(void *)*8 - sizeof(kBytes))

typedef const struct kBytesVar kBytes;
struct kBytesVar {
	KonohaObjectHeader h;
	COMMON_BYTEARRAY;
};

struct kStringVar /* extends _Bytes */ {
	KonohaObjectHeader h;
	COMMON_BYTEARRAY;
	const char inline_text[SIZEOF_INLINETEXT];
};

typedef enum {
	StringPolicy_TEXT     =     (1<<0),
	StringPolicy_ASCII    =     (1<<1),
	StringPolicy_UTF8     =     (1<<2),
	StringPolicy_NOCOPY   =     (1<<3),
	StringPolicy_NOPOOL   =     (1<<4),   /* UNUSED in the future */
	StringPolicy_SystemInfo =   (1<<5),   /* UNUSED */
	StringPolicy_FreeKBuffer =  (1<<6),   /* KBuffer_Stringfy */
} StringPolicy;

typedef enum { NonZero, EnsureZero } StringfyPolicy;

#define K_NULLTEXT          "null"
#define new_T(t)            (KLIB new_kString(kctx, t, knh_strlen(t), StringPolicy_TEXT|StringPolicy_ASCII|StringPolicy_POOL))
#define new_S(T, L)         (KLIB new_kString(kctx, T, L, StringPolicy_ASCII|StringPolicy_POOL))
#define kString_text(s)           ((const char *) (kObject_class(s)->unbox(kctx, (kObject *)s)))
#define kString_size(s)           ((s)->bytesize)

/* ------------------------------------------------------------------------ */
//## class Array   Object;

#define IS_Array(o)              (kObject_baseTypeId(o) == KType_Array)

#define kArrayFlag_UnboxData     kObjectFlag_Local1
#define kArray_Is(P, o)          (KFlag_Is(uintptr_t,(o)->h.magicflag, kArrayFlag_##P))
#define kArray_Set(P, o, b)      KFlag_Set(uintptr_t,(o)->h.magicflag, kArrayFlag_##P,b)

struct kArrayVar {
	KonohaObjectHeader h;
	size_t bytesize;
	union {
		uintptr_t              *unboxItems;
		kint_t                 *kintItems;
#ifndef USE_NOFLOAT
		kfloat_t               *kfloatItems;
#endif
		kObject        **ObjectItems;
		kString        **stringItems;
		kParam         **ParamItems;
		kMethod        **MethodItems;
		kFunc          **FuncItems;
		kToken         **TokenItems;
		kTokenVar      **TokenVarItems;
		kExpr          **ExprItems;
		kExprVar       **ExprVarItems;
		kStmt          **StmtItems;
		kStmtVar       **StmtVarItems;
	};
	size_t bytemax;
};

/* ------------------------------------------------------------------------ */
/* Param */

#define IS_Param(o)              (kObject_baseTypeId(o) == KType_Param)

typedef struct kparamtype_t {
	ktypeattr_t    attrTypeId;
	ksymbol_t  name;
} kparamtype_t;

struct kParamVar {
	KonohaObjectHeader h;
	ktypeattr_t rtype; kushort_t psize;
	kparamtype_t paramtypeItems[3];
};

/* ------------------------------------------------------------------------ */
/* Method */

#define IS_Method(o)                 (kObject_baseTypeId(o) == KType_Method)

#ifdef USE_MethodFlagData
static const char* MethodFlagData[] = {
	"Public", "Virtual", "Final", "Const", "Static", "Immutable",
	"Coercion", "Restricted", "FastCall", "SmartReturn", "Variadic", "Iterative",
	"CCompatible", "JSCompatible", "JavaCompatible", "Accountable",
};
#endif

// property
#define kMethod_Public               ((uintptr_t)(1<<0))
#define kMethod_Virtual              ((uintptr_t)(1<<1))
#define kMethod_Final                ((uintptr_t)(1<<2))
#define kMethod_Const                ((uintptr_t)(1<<3))
#define kMethod_Static               ((uintptr_t)(1<<4))
#define kMethod_Immutable            ((uintptr_t)(1<<5))

// call rule
#define kMethod_Coercion             ((uintptr_t)(1<<6))
#define kMethod_Restricted           ((uintptr_t)(1<<7))
#define kMethod_FastCall             ((uintptr_t)(1<<8))
#define kMethod_SmartReturn          ((uintptr_t)(1<<9))
#define kMethod_Variadic             ((uintptr_t)(1<<10))
#define kMethod_Iterative            ((uintptr_t)(1<<11))

// compatible
#define kMethod_CCompatible          ((uintptr_t)(1<<12))
#define kMethod_JSCompatible         ((uintptr_t)(1<<13))
#define kMethod_JCompatible          ((uintptr_t)(1<<14))
#define kMethod_Accountable          ((uintptr_t)(1<<15))

// internal
#define kMethod_Hidden               ((uintptr_t)(1<<16))
#define kMethod_Abstract             ((uintptr_t)(1<<17))
#define kMethod_Overloaded           ((uintptr_t)(1<<18))
#define kMethod_Override             ((uintptr_t)(1<<19))
#define kMethod_IgnoredOverride      ((uintptr_t)(1<<20))
#define kMethod_DynamicCall          ((uintptr_t)(1<<21))

#define kMethod_Warning              ((uintptr_t)(1<<22))
#define kMethod_WeakCoercion         kMethod_Coercion|kMethod_Warning

#define kMethod_Is(P, MTD)            (KFlag_Is(uintptr_t, (MTD)->flag, kMethod_##P))
#define kMethod_Set(P, MTD, B)        KFlag_Set(uintptr_t, (MTD)->flag, kMethod_##P, B)

#define kMethod_GetParam(mtd)        kctx->share->paramList_OnGlobalConstList->ParamItems[mtd->paramid]
#define kMethod_GetReturnType(mtd)   KClass_((kMethod_GetParam(mtd))->rtype)
#define kMethod_IsReturnFunc(mtd)    (KClass_((kMethod_GetParam(mtd))->rtype)->baseTypeId == KType_Func)
#define kMethod_ParamSize(mtd)       ((kMethod_GetParam(mtd))->psize)
#define Method_t(mtd)                KType_text((mtd)->typeId),  MethodName_Fmt2((mtd)->mn)

/* method data */
#define DEND     (-1)

#define KMETHOD    void  /*CC_FASTCALL_*/
typedef KMETHOD   (*MethodFunc)(KonohaContext*, KonohaStack *);

struct VirtualCodeAPI {
	void (*FreeVirtualCode)(KonohaContext *kctx, struct VirtualCode *);
	void (*WriteVirtualCode)(KonohaContext *kctx, KBuffer *, struct VirtualCode *);
};

struct kMethodVar {
	KonohaObjectHeader     h;
	MethodFunc              invokeMethodFunc;
	union {
		struct VirtualCode     *vcode_start;
		struct VirtualCodeAPI   **virtualCodeApi_plus1;
	};
	uintptr_t         flag;
	ktypeattr_t       typeId;       kmethodn_t  mn;
	kparamId_t        paramid;      kparamId_t paramdom;
	kshort_t          delta;        kpackageId_t packageId;
	kToken           *SourceToken;
	union {
		kNameSpace   *LazyCompileNameSpace;       // lazy compilation
		kBlock       *CompiledBlock;
	};
	uintptr_t         serialNumber;
};

typedef struct MethodMatch {
	kNameSpace   *ns;
	ksymbol_t     mn;
	kushort_t     paramsize;
	kparamId_t    paramdom;
	kparamtype_t *param;
	kbool_t       isBreak;
	kMethod      *foundMethodNULL;
	kArray       *foundMethodListNULL;
} MethodMatch;

typedef enum {
	MethodMatch_NoOption   = 1,
	MethodMatch_CamelStyle = 1 << 1
} MethodMatchOption;

typedef kbool_t (*MethodMatchFunc)(KonohaContext *kctx, kMethod *mtd, MethodMatch *m);

#define K_CALLDELTA   4
#define K_RTNIDX    (-4)
#define K_SHIFTIDX  (-3)
#define K_PCIDX     (-2)
#define K_MTDIDX    (-1)
#define K_NSIDX     (-2)
#define K_TRACEIDX  (-3)

//#define K_NEXTIDX    2
#define K_ULINEIDX2  (-7)
#define K_SHIFTIDX2  (-5)
#define K_PCIDX2     (-3)
#define K_MTDIDX2    (-1)

/* ------------------------------------------------------------------------ */
/* Func */

#define IS_Func(o)              (kObject_baseTypeId(o) == KType_Func)

struct kFuncVar {
	KonohaObjectHeader h;
	kMethod *method;
	kObject *env;
};

/* ------------------------------------------------------------------------ */
/* NameSpace */

#define IS_NameSpace(O)  (kObject_class(O) == KClass_NameSpace)
#define kNameSpace_sizeConstTable(ns)    (ns->constTable.data.bytesize / sizeof(KKeyValue))

struct kNameSpaceVar {
	KonohaObjectHeader h;
	kpackageId_t packageId;  	       kshortflag_t syntaxOption;
	kArray                            *NameSpaceConstList;
	kNameSpace                        *parentNULL;
	KDict                              constTable;
	kObject                           *globalObjectNULL_OnList;
	kArray                            *methodList_OnList;   // default K_EMPTYARRAY
	size_t                             sortedMethodList;
	// the below references are defined in sugar
	void                              *tokenMatrix;
	KHashMap                          *syntaxMapNN;
	kArray                            *stmtPatternListNULL_OnList;
	struct KBuilderAPI2               *builderApi;
	KKeyValue                         *typeVariableItems;
	size_t                             typesize;
};

// NameSpace_syntaxOption

#define kNameSpace_DefaultSyntaxOption               kNameSpace_ImplicitField|kNameSpace_NoSemiColon
#define kNameSpace_IsAllowed(P, ns)                  (KFlag_Is(kshortflag_t, (ns)->syntaxOption, kNameSpace_##P))
#define kNameSpace_Set(P, ns, B)                     KFlag_Set(kshortflag_t, ((kNameSpaceVar *)ns)->syntaxOption, kNameSpace_##P, B)

#define kNameSpace_NoSemiColon                       ((kshortflag_t)(1<<1))
#define kNameSpace_TypeInference                     ((kshortflag_t)(1<<2))
#define kNameSpace_ImplicitField                     ((kshortflag_t)(1<<3))
#define kNameSpace_ImplicitGlobalVariable            ((kshortflag_t)(1<<4))
#define kNameSpace_ImplicitCoercion                  ((kshortflag_t)(1<<5))

/* ------------------------------------------------------------------------ */
/* System */

#define IS_System(o)              (kObject_typeId(o) == KType_System)

typedef const struct _kSystem kSystem;

struct _kSystem {
	KonohaObjectHeader h;
};

/* ------------------------------------------------------------------------ */
/* T0 */

/* ------------------------------------------------------------------------ */
/* macros */

#define KStackCheckOverflow(SFP) do {\
		if(unlikely(kctx->esp > kctx->stack->stack_uplimit)) {\
			KLIB KonohaRuntime_raise(kctx, EXPT_("StackOverflow"), SoftwareFault, NULL, SFP);\
		}\
}while(0)\

#define BEGIN_UnusedStack(SFP) KonohaStack *SFP = kctx->esp + K_CALLDELTA, *esp_ = kctx->esp; KStackCheckOverflow(kctx->stack->topStack);
#define END_UnusedStack()      ((KonohaContextVar *)kctx)->esp = esp_;

#define KStackSetLine(SFP, UL)    SFP[K_RTNIDX].calledFileLine   = UL
#define KStackSetArgc(SFP, ARGC)  ((KonohaContextVar *)kctx)->esp = (SFP + ARGC + 1)

#define KStackSetMethodAll(SFP, DEFVAL, UL, MTD, ARGC) { \
		KUnsafeFieldSet(SFP[K_RTNIDX].asObject, ((kObject *)DEFVAL));\
		KStackSetLine(SFP, UL);\
		KStackSetArgc(SFP, ARGC);\
		SFP[K_MTDIDX].calledMethod = MTD; \
	} \


#define KStackSetFunc(SFP, FO) do {\
	SFP[K_MTDIDX].calledMethod = (FO)->method;\
}while(0);\

#define KStackSetFuncAll(SFP, DEFVAL, UL, FO, ARGC) { \
		KUnsafeFieldSet(SFP[K_RTNIDX].asObject, ((kObject *)DEFVAL));\
		KStackSetLine(SFP, UL);\
		KStackSetArgc(SFP, ARGC);\
		KStackSetFunc(SFP, FO);\
	} \

// if you want to ignore (exception), use KonohaRuntime_tryCallMethod
#define KStackCall(SFP) { \
		SFP[K_SHIFTIDX].previousStack = kctx->stack->topStack;\
		kctx->stack->topStack = SFP;\
		(SFP[K_MTDIDX].calledMethod)->invokeMethodFunc(kctx, SFP);\
		kctx->stack->topStack = SFP[K_SHIFTIDX].previousStack;\
	} \

#define KStackCallAgain(SFP, MTD) { \
		SFP[K_MTDIDX].calledMethod = MTD;\
		(MTD)->invokeMethodFunc(kctx, SFP);\
	} \


/* ----------------------------------------------------------------------- */
/* Package */

#define KPACKNAME(N, V) \
	.name = N, .version = V, .konoha_Checksum = K_DATE

#define KSetPackageName(VAR, N, V) do{\
	VAR.name = N; VAR.version = V; VAR.konoha_Checksum = K_DATE;\
} while(0)

#define KPACKLIB(N, V) \
	.libname = N, .libversion = V
#define KSETPACKLIB(VAR, N, V) do{\
	VAR.libname = N; VAR.libversion = V;\
} while(0)

struct KPackageHandlerVar {
	long  konoha_Checksum;
	const char *name;
	const char *version;
	const char *libname;
	const char *libversion;
	kbool_t (*PackupNameSpace)    (KonohaContext *kctx, kNameSpace *, int, KTraceInfo *);
	kbool_t (*ExportNameSpace)    (KonohaContext *kctx, kNameSpace *, kNameSpace *, int, KTraceInfo *);
};

typedef struct KPackageVar KPackage;

struct KPackageVar {
	kpackageId_t                 packageId;
	kNameSpace                  *packageNS /* onGlobalConstList*/;
	KPackageHandler        *packageHandler;
	kfileline_t                  kickout_script;
};

/* ----------------------------------------------------------------------- */
// klib

struct klogconf_t;
typedef struct GcContext GcContext;

typedef enum {
	DefaultCompileOption = 0,
	CrossCompile  = 1,
	HatedLazyCompile   = (1 < 1),
	O2Compile     = (1 < 2),
	DebugCompile  = (1 < 3)
} CompileOption;

typedef struct KObjectVisitor {
	void (*fn_visit)(struct KObjectVisitor *vistor, kObject *object);
	void (*fn_visitRange)(struct KObjectVisitor *visitor, kObject **begin, kObject **end);
} KObjectVisitor;

struct KonohaLibVar {

	/* Event Handler API */
	/* This Must Be Going To Factory */
	void (*KscheduleEvent)  (KonohaContext *);

	void                (*KArray_Init)(KonohaContext *,  KGrowingArray *, size_t);
	void                (*KArray_Resize)(KonohaContext*, KGrowingArray *, size_t);
	void                (*KArray_Expand)(KonohaContext*, KGrowingArray *, size_t);
	void                (*KArray_Free)(KonohaContext*,   KGrowingArray *);

	void                (*KBuffer_Init)(KGrowingArray *, KBuffer *);
	void*               (*KBuffer_Alloca)(KonohaContext *, KBuffer *, size_t);
	void                (*KBuffer_Write)(KonohaContext*, KBuffer *, const char *, size_t);
	void                (*KBuffer_vprintf)(KonohaContext*, KBuffer *, const char *fmt, va_list ap);
	void                (*KBuffer_printf)(KonohaContext*, KBuffer *, const char *fmt, ...);
	const char*         (*KBuffer_text)(KonohaContext*, KBuffer *, int);
	void                (*KBuffer_Free)(KBuffer *);
	kString*            (*KBuffer_Stringfy)(KonohaContext *, KBuffer *, kArray *gcstack, int isClear);
	kbool_t             (*KBuffer_iconv)(KonohaContext *, KBuffer*, uintptr_t iconv, const char *, size_t, KTraceInfo *);

	KKeyValue*          (*KDict_GetNULL)(KonohaContext *, KDict *, ksymbol_t);
	void                (*KDict_Add)(KonohaContext *, KDict *, KKeyValue *);
	void                (*KDict_Remove)(KonohaContext *, KDict *, ksymbol_t);
	void                (*KDict_Set)(KonohaContext *, KDict *, KKeyValue *);
	void                (*KDict_MergeData)(KonohaContext *, KDict *, KKeyValue *, size_t, int isOverride);
	void                (*KDict_DoEach)(KonohaContext *, KDict *, void *, void (*)(KonohaContext*, void *, KKeyValue *));
	void                (*KDict_Reftrace)(KonohaContext *, KDict *, KObjectVisitor *);
	void                (*KDict_Free)(KonohaContext *, KDict *);

	KHashMap*           (*KHashMap_Init)(KonohaContext*, size_t);
	KHashMapEntry*      (*KHashMap_newEntry)(KonohaContext*, KHashMap *, uintptr_t);
	KHashMapEntry*      (*KHashMap_get)(KonohaContext*, KHashMap *, uintptr_t);
	void                (*KHashMap_Remove)(KHashMap *, KHashMapEntry *);
	void                (*KHashMap_DoEach)(KonohaContext*, KHashMap *, void *thunk, void (*)(KonohaContext*, KHashMapEntry*, void *));
	void                (*KHashMap_Free)(KonohaContext*, KHashMap *, void (*)(KonohaContext*, void *));
	ksymbol_t           (*KHashMap_getcode)(KonohaContext*, KHashMap *, kArray *, const char *, size_t, uintptr_t, int, ksymbol_t);

	kfileline_t         (*KfileId)(KonohaContext*, const char *, size_t, int spol, ksymbol_t def);
	kpackageId_t        (*KpackageId)(KonohaContext*, const char *, size_t, int spol, ksymbol_t def);
	ksymbol_t           (*Ksymbol)(KonohaContext*, const char*, size_t, int spol, ksymbol_t def);

	KonohaContextVar*   (*KonohaContext_Init)(KonohaContext *rootContext, const PlatformApi *api);
	void                (*KonohaContext_Free)(KonohaContext *rootContext, KonohaContextVar *ctx);
	void                (*ReftraceAll)(KonohaContext *kctx, KObjectVisitor *);

	KonohaContext*      (*KonohaFactory_CreateKonoha)(KonohaFactory *factory);
	int                 (*Konoha_Destroy)(KonohaContext *kctx);
	kbool_t             (*Konoha_LoadScript)(KonohaContext*, const char *);
	kbool_t             (*Konoha_Eval)(KonohaContext*, const char *, kfileline_t);

	KonohaClass*        (*Kclass)(KonohaContext*, ktypeattr_t, KTraceInfo *);
	kString*            (*KonohaClass_shortName)(KonohaContext*, KonohaClass *ct);
	KonohaClass*        (*KonohaClass_define)(KonohaContext*, kpackageId_t, kString *, KDEFINE_CLASS *, KTraceInfo *);
	KonohaClass*        (*KonohaClass_Generics)(KonohaContext*, KonohaClass *, ktypeattr_t rty, kushort_t psize, kparamtype_t *p);
	kbool_t             (*KonohaClass_isSubtype)(KonohaContext*, KonohaClass *, KonohaClass *);
	kbool_t             (*KonohaClass_AddField)(KonohaContext*, KonohaClass *, ktypeattr_t, ksymbol_t);

	kObject*            (*new_kObject)(KonohaContext*, kArray *gcstack, KonohaClass *, uintptr_t);
	kObject*            (*Knull)(KonohaContext*, KonohaClass *);
	void                (*kObjectProto_Free)(KonohaContext *kctx, kObjectVar *);
	void                (*kObjectProto_Reftrace)(KonohaContext *kctx, kObject *, KObjectVisitor *);
	KKeyValue*          (*kObjectProto_GetKeyValue)(KonohaContext*, kAbstractObject *, ksymbol_t);
	kObject*            (*kObject_getObject)(KonohaContext*, kAbstractObject *, ksymbol_t, kAbstractObject *);
	void                (*kObjectProto_SetObject)(KonohaContext*, kAbstractObject *, ksymbol_t, ktypeattr_t, kAbstractObject *);
	void                (*kObjectProto_SetUnboxValue)(KonohaContext*, kAbstractObject *, ksymbol_t, ktypeattr_t, uintptr_t);
	void                (*kObjectProto_RemoveKey)(KonohaContext*, kAbstractObject *, ksymbol_t);
	void                (*kObjectProto_DoEach)(KonohaContext*, kAbstractObject *, void *thunk, void (*f)(KonohaContext*, void *, KKeyValue *d));
	int                 (*kObjectProto_p)(KonohaContext *, KonohaStack *, int, KBuffer *, int count);
	void                (*kObject_WriteToBuffer)(KonohaContext *, kObject *, int, KBuffer *, KonohaValue *, int);

	kString*            (*new_kString)(KonohaContext*, kArray *gcstack, const char *, size_t, int);

	void                (*kArray_Add)(KonohaContext*, kArray *, kAbstractObject *);
	void                (*kArray_Insert)(KonohaContext*, kArray *, size_t, kAbstractObject *);
	void                (*kArray_Clear)(KonohaContext*, kArray *, size_t);

	kparamId_t          (*Kparamdom)(KonohaContext*, kushort_t, const kparamtype_t *);
	kMethodVar*         (*new_kMethod)(KonohaContext*, kArray *gcstack, uintptr_t, ktypeattr_t, kmethodn_t, MethodFunc);
	kParam*             (*kMethod_SetParam)(KonohaContext*, kMethod *, ktypeattr_t, kushort_t, const kparamtype_t *);
	void                (*kMethod_SetFunc)(KonohaContext*, kMethod*, MethodFunc);
	void                (*kMethod_GenCode)(KonohaContext*, kMethod*, kBlock *, int options);
	intptr_t            (*kMethod_indexOfField)(kMethod *);

	kbool_t             (*KonohaRuntime_setModule)(KonohaContext*, int, struct KonohaModule *, KTraceInfo *);

	void                (*kNameSpace_FreeSugarExtension)(KonohaContext *, kNameSpaceVar *);

	KPackage*      (*kNameSpace_RequirePackage)(KonohaContext*, const char *, KTraceInfo *);
	kbool_t             (*kNameSpace_ImportPackage)(KonohaContext*, kNameSpace*, const char *, KTraceInfo *);
	kbool_t             (*kNameSpace_ImportPackageSymbol)(KonohaContext *, kNameSpace *, const char *, ksymbol_t keyword, KTraceInfo *);

	KonohaClass*        (*kNameSpace_GetClassByFullName)(KonohaContext*, kNameSpace *, const char *, size_t, KonohaClass *);
	KonohaClass*        (*kNameSpace_DefineClass)(KonohaContext*, kNameSpace *, kString *, KDEFINE_CLASS *, KTraceInfo *);

	kbool_t             (*kNameSpace_SetConstData)(KonohaContext *, kNameSpace *, ksymbol_t, ktypeattr_t, uintptr_t, int isOverride, KTraceInfo *);
	kbool_t             (*kNameSpace_LoadConstData)(KonohaContext*, kNameSpace *, const char **d, int isOverride, KTraceInfo *);
	KKeyValue*          (*kNameSpace_GetConstNULL)(KonohaContext *, kNameSpace *, ksymbol_t);
	void                (*kNameSpace_LoadMethodData)(KonohaContext*, kNameSpace *, intptr_t *, KTraceInfo *);

	kMethod*            (*kNameSpace_GetGetterMethodNULL)(KonohaContext*, kNameSpace *, KonohaClass*, ksymbol_t mn);
	kMethod*            (*kNameSpace_GetSetterMethodNULL)(KonohaContext*, kNameSpace *, KonohaClass*, ksymbol_t mn, ktypeattr_t);
	kMethod*            (*kNameSpace_GetCoercionMethodNULL)(KonohaContext*, kNameSpace *, KonohaClass *, KonohaClass *);
	kMethod*            (*kNameSpace_GetMethodByParamSizeNULL)(KonohaContext*, kNameSpace *, KonohaClass *, kmethodn_t mn, int paramsize, MethodMatchOption option);
	kMethod*            (*kNameSpace_GetMethodBySignatureNULL)(KonohaContext*, kNameSpace *, KonohaClass *, kmethodn_t mn, int paramdom, int paramsize, kparamtype_t *);
	kMethod*            (*kMethod_DoLazyCompilation)(KonohaContext *kctx, kMethod *mtd, kparamtype_t *, int options);
//	void                (*kNameSpace_compileAllDefinedMethods)(KonohaContext *kctx);

	// runtime support
	void                (*CheckSafePoint)(KonohaContext *kctx, KonohaStack *sfp, kfileline_t uline);
	kbool_t             (*KonohaRuntime_tryCallMethod)(KonohaContext *, KonohaStack *);
	uintptr_t           (*ApplySystemFunc)(KonohaContext *, uintptr_t defval, const char *name, const char *param, ...);

	void                (*KonohaRuntime_raise)(KonohaContext*, int symbol, int fault, kString *Nullable, KonohaStack *);
	void                (*ReportScriptMessage)(KonohaContext *, KTraceInfo *, kinfotag_t, const char *fmt, ...);
	int                 (*DiagnosisFaultType)(KonohaContext *kctx, int fault, KTraceInfo *);
	void                (*DumpObject)(KonohaContext *, kObject *, const char *, const char *, int);
};

#define K_NULL            (kctx->share->constNull_OnGlobalConstList)
#define K_TRUE            (kctx->share->constTrue_OnGlobalConstList)
#define K_FALSE           (kctx->share->constFalse_OnGlobalConstList)
#define K_NULLPARAM       (kctx->share->paramList_OnGlobalConstList->ParamItems[0])
#define K_EMPTYARRAY      (kctx->share->emptyArray_OnGlobalConstList)
#define TS_EMPTY          (kctx->share->emptyString_OnGlobalConstList)

#define UPCAST(o)         ((kObject *)o)

#define KMalloc(size, TRACE)           PLATAPI Kmalloc(kctx, size, TRACE)
#define KCalloc(size, item, TRACE)     PLATAPI Kzmalloc(kctx, ((size) * (item)), TRACE)
#define KMalloc_UNTRACE(size)          PLATAPI Kmalloc(kctx, size, NULL)
#define KCalloc_UNTRACE(size, item)    PLATAPI Kzmalloc(kctx, ((size) * (item)), NULL)
#define KFree(p, size)                 PLATAPI Kfree(kctx, p, size)

//#define KLIB KBuffer_Write(W,...)          KLIB KBuffer_putc(kctx,W, ## __VA_ARGS__, -1)
#define KBuffer_bytesize(W)                 (((W)->m)->bytesize - (W)->pos)

#define kclass(CID, UL)           KLIB Kclass(kctx, CID, UL)

#define FILEID_(T)                KLIB KfileId(kctx, T, sizeof(T)-1, StringPolicy_TEXT|StringPolicy_ASCII, _NEWID)

#define PN_konoha                 0
#define PackageId_sugar           1
#define PN_(T)                    KLIB KpackageId(kctx, T, sizeof(T)-1, StringPolicy_TEXT|StringPolicy_ASCII, _NEWID)

#define ksymbolA(T, L, DEF)       KLIB Ksymbol(kctx, T, L, StringPolicy_ASCII, DEF)
#define ksymbolSPOL(T, L, SPOL, DEF)       KLIB Ksymbol(kctx, T, L, SPOL, DEF)
#define SYM_(T)                   KLIB Ksymbol(kctx, T, (sizeof(T)-1), StringPolicy_TEXT|StringPolicy_ASCII, _NEWID)
#define EXPT_(T)                  KLIB Ksymbol(kctx, (T "Exception"), (sizeof(T "Exception")-1), StringPolicy_TEXT|StringPolicy_ASCII, _NEWID)
#define FN_(T)                    KLIB Ksymbol(kctx, T, (sizeof(T)-1), StringPolicy_TEXT|StringPolicy_ASCII, _NEWID)
#define MN_(T)                    KLIB Ksymbol(kctx, T, (sizeof(T)-1), StringPolicy_TEXT|StringPolicy_ASCII, _NEWID)
#define MN_box                    MN_("box")

#define MN_new                    38  /* @see KSymbol_return + 1*/
#define KSymbol_void                   39

#define new_(C, A, STACK)                 (k##C *)(KLIB new_kObject(kctx, STACK, KClass_##C, ((uintptr_t)A)))
#define GcUnsafe                          NULL
#define OnStack                           NULL
#define OnField                           NULL
#define OnVirtualField                    NULL
#define OnGlobalConstList                 (kctx->share->GlobalConstList)
#define OnContextConstList                (kctx->stack->ContextConstList)
#define OnGcStack                         (kctx->stack->gcstack_OnContextConstList)

#define KNULL(C)                  (k##C *)KLIB Knull(kctx, KClass_##C)

#define kArray_size(A)            (((A)->bytesize)/sizeof(void *))
#define kArray_SetSize(A, N)      ((kArrayVar *)A)->bytesize = ((N) * sizeof(void *))
#define new_kParam(CTX, R, PSIZE, P)       (KLIB kMethod_SetParam(CTX, NULL, R, PSIZE, P))

#define KRequirePackage(NAME, TRACE)       KLIB kNameSpace_RequirePackage(kctx, NAME, TRACE)
#define KImportPackage(NS, NAME, TRACE)    KLIB kNameSpace_ImportPackage(kctx, NS, NAME, TRACE)
#define KImportPackageSymbol(NS, NAME, SYMBOL, TRACE) KLIB kNameSpace_ImportPackageSymbol(kctx, NS, NAME, SYM_(SYMBOL), TRACE)

typedef intptr_t  KDEFINE_METHOD;

#define KonohaConst_(D)  ((const char **)D)

#define KDefineConstInt(T) #T, KType_int, T

typedef struct {
	const char *key;
	uintptr_t ty;
	kint_t value;
} KDEFINE_INT_CONST;

typedef struct {
	const char *key;
	uintptr_t ty;
	const char* value;
} KDEFINE_TEXT_CONST;

typedef struct {
	const char *key;
	uintptr_t ty;
	kfloat_t value;
} KDEFINE_FLOAT_CONST;

typedef struct {
	const char *key;
	uintptr_t ty;
	kObject *value;
} KDEFINE_OBJECT_CONST;



#define KSetKLibFunc(PKGID, T, F, TRACE)   do {\
	void *func = kctx->klib->T;\
	((KonohaLibVar *)kctx->klib)->T = F;\
	if(TRACE && func != NULL) {\
		KLIB ReportScriptMessage(kctx, TRACE, DebugTag, "overriding KLIB function " #T " in %s", KPackage_text(PKGID));\
	}\
} while(0)

#define KSetClassFunc(PKGID, C, T, F, TRACE)   do {\
		void *func = C->T;\
		((KonohaClassVar *)C)->T = F;\
		if(TRACE && func != NULL) {\
			KLIB ReportScriptMessage(kctx, TRACE, DebugTag, "overriding CLASS %s funcion " #T " in %s", KClass_text(C), KPackage_text(PKGID)) ;\
		}\
	}while(0)\

/* [GarbageCollection] */

#if defined(_MSC_VER)
#define OBJECT_SET(var, val) var = (decltype(var))(val)
#else
#define OBJECT_SET(var, val) var = (typeof(var))(val)
#endif /* defined(_MSC_VER) */

#define INIT_GCSTACK()         kArray* _GcStack = kctx->stack->gcstack_OnContextConstList; size_t _gcstackpos = kArray_size(_GcStack)
#define RESET_GCSTACK()        KLIB kArray_Clear(kctx, _GcStack, _gcstackpos)
#define PUSH_GCSTACK2(o)       KLIB kArray_Add(kctx, kctx->stack->gcstack_OnContextConstList, o)

#define KRefIncObject(T, O)
#define KRefDecObject(T, O)

#define GC_WRITE_BARRIER(kctx, PARENT, VAR, VAL)\
	(PLATAPI UpdateObjectField((struct kObjectVar *)(PARENT), (struct kObjectVar *)(VAR), ((struct kObjectVar *)(VAL))))

#define KUnsafeFieldInit(VAR, VAL) OBJECT_SET(VAR, VAL)
#define KUnsafeFieldSet( VAR, VAL) (VAR) = (VAL) /* for c-compiler type check */
#define KStackSet(VAR, VAL)  (VAR) = (VAL)

#define KFieldInit(PARENT, VAR, VAL) GC_WRITE_BARRIER(kctx, PARENT, VAR, VAL); KUnsafeFieldInit(VAR, VAL)
#define KFieldSet(PARENT, VAR, VAL)  GC_WRITE_BARRIER(kctx, PARENT, VAR, VAL); KUnsafeFieldSet( VAR, VAL)

#define KSafeFieldSet(PARENT, VAR, VAL) do {\
	if(VAR == 0) {\
		KFieldInit(PARENT, VAR, VAL);\
	} else {\
		KFieldSet(PARENT, VAR, VAL);\
	}\
} while(0)

#define KRefTrace(p)  do {\
	DBG_ASSERT(p != NULL);\
	visitor->fn_visit(visitor, (kObject *)(p));\
} while(0)

#define KRefTraceNullable(p) do {\
	if(p != NULL) {\
		visitor->fn_visit(visitor, (kObject *)(p));\
	}\
} while(0)

#define KRefTraceRange(BEGIN, END)  do {\
	DBG_ASSERT(p != NULL);\
	visitor->fn_visitRange(visitor, (kObject **)(BEGIN), (kObject **)(END));\
} while(0)


#define KCheckSafePoint(kctx, sfp) do {\
	KLIB CheckSafePoint(kctx, sfp, 0);\
} while(0)

// method macro

#define KGetReturnObject(sfp)  (sfp[K_RTNIDX].asObject)
#define KGetReturnType(sfp)    kObject_class(sfp[K_RTNIDX].asObject)
#define KGetLexicalNameSpace(sfp)    sfp[K_NSIDX].asNameSpace

#define KReturnWith(VAL, CLEANUP) do {\
	KUnsafeFieldSet(sfp[K_RTNIDX].asObject, ((kObject *)VAL));\
	CLEANUP;\
	KCheckSafePoint(kctx, sfp);\
	return; \
} while(0)

#define KReturnDefaultValue() do {\
	sfp[K_RTNIDX].intValue = 0;\
	return; \
} while(0)

#define KReturnField(vv) do {\
	KUnsafeFieldSet(sfp[(-(K_CALLDELTA))].asObject, ((kObject *)vv));\
	return; \
} while(0)

#define KReturn(vv) do {\
	KUnsafeFieldSet(sfp[(-(K_CALLDELTA))].asObject, ((kObject *)vv));\
	KCheckSafePoint(kctx, sfp);\
	return; \
} while(0)

#define KReturnInt(d) do {\
	sfp[K_RTNIDX].intValue = d; \
	return; \
} while(0)

#define KReturnUnboxValue(d) do {\
	sfp[K_RTNIDX].unboxValue = d; \
	return; \
} while(0)

#define KReturnFloatValue(c) do {\
	sfp[(-(K_CALLDELTA))].floatValue = c; \
	return; \
} while(0)

#define KReturnVoid() do {\
	return; \
} while(0)

#ifdef USE_SMALLBUILD
#define KExit(S)           PLATAPI exit_i(S, NULL, 0)
#else
#define KExit(S)           PLATAPI exit_i(S, __FILE__, __LINE__)
#endif

#define FIXME_ASSERT(a)    assert(a)

#ifndef USE_SMALLBUILD
#ifdef _MSC_VER
#define KNH_ASSERT(a)
#define DBG_ASSERT(a)
#define TODO_ASSERT(a)
#else
#define KNH_ASSERT(a)       assert(a)
#define DBG_ASSERT(a)       assert(a)
#define TODO_ASSERT(a)      assert(a)
#endif /* _MSC_VER */
#define SAFECHECK(T)        (T)
#define DBG_P(fmt, ...)     PLATAPI ReportDebugMessage(__FILE__, __FUNCTION__, __LINE__, fmt, ## __VA_ARGS__)
#define DBG_ABORT(fmt, ...) PLATAPI ReportDebugMessage(__FILE__, __FUNCTION__, __LINE__, fmt, ## __VA_ARGS__); KExit(EXIT_FAILURE)
#define DUMP_P(fmt, ...)    PLATAPI printf_i(fmt, ## __VA_ARGS__)
#else
#define SAFECHECK(T)        (1)
#define KNH_ASSERT(a)
#define DBG_ASSERT(a)
#define TODO_ASSERT(a)
#define DBG_P(fmt, ...)
#define DBG_ABORT(fmt, ...)
#define DUMP_P(fmt, ...)
#endif

#ifdef __GNUC__
#ifndef unlikely
#define unlikely(x)   __builtin_expect(!!(x), 0)
#endif

#ifndef likely
#define likely(x)     __builtin_expect(!!(x), 1)
#endif
#else
#ifndef unlikely
#define unlikely(x)   (x)
#endif

#ifndef likely
#define likely(x)     (x)
#endif
#endif

///* Konoha API */
extern kbool_t Konoha_LoadScript(KonohaContext* konoha, const char *scriptfile);
extern kbool_t Konoha_Eval(KonohaContext* konoha, const char *script, kfileline_t uline);
extern kbool_t konoha_Run(KonohaContext* konoha);  // TODO

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MINIOKNOHA_H_ */
