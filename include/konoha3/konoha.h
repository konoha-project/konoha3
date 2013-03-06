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
#include <konoha3/stardate.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NEWSYNTAX 1

#define K_CLASSTABLE_INITSIZE 64
#define K_PAGESIZE        4096

#define K_VERSION   "3.0"
#define K_MAJOR_VERSION 3
#define K_MINOR_VERSION 0
#define K_PATCH_LEVEL   0

#define K_DATE      ((K_YEAR - 2006) * (12 * 24 * 32) + (K_MONTH * (24 * 32) + ((K_DAY * 24) + K_HOUR)))

#ifndef K_REVISION
#define K_REVISION 0
#endif

#ifndef K_PROGNAME
#define K_PROGNAME  "Konoha"
/*2012/01/24 - */
#define K_CODENAME "Rome"  // eternal city
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

#ifdef __cplusplus
} /* extern "C" */
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
#include <konoha3/stdbool.h>
#endif
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
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
#define ICONV_INBUF_CONST const
#else
#define ICONV_INBUF_CONST
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

#define FLAG_Set(f,op)            KFlag_Set1(kshortflag_t,f,op)
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
} KFlagSymbolData;

/* ktypeattr_t */
#define KTypeAttr_NewId          ((ktypeattr_t)-1)

#define KTypeAttr_Attr(t)        ((t)  & (KFLAG_H0|KFLAG_H1|KFLAG_H2|KFLAG_H3))
#define KTypeAttr_Unmask(t)      ((t)  & (~(KFLAG_H0|KFLAG_H1|KFLAG_H2|KFLAG_H3)))

#define KTypeAttr_Boxed      KFLAG_H0    /* KeyValueStore, Field */
#define KTypeAttr_ReadOnly   KFLAG_H1    /* Variable, Field */
#define KTypeAttr_LocalOnly  KFLAG_H1    /* KeyValueStore */
#define KTypeAttr_Coercion   KFLAG_H2    /* Variable, Field */

#define KTypeAttr_Is(P, t)   (((t) & KTypeAttr_##P) == KTypeAttr_##P)

#define KSymbol_MAX            KFLAG_H3
#define KSymbol_Attr(sym)      (sym  & (KFLAG_H0|KFLAG_H1|KFLAG_H2|KFLAG_H3))
#define KSymbol_Unmask(sym)    (sym & (~(KFLAG_H0|KFLAG_H1|KFLAG_H2|KFLAG_H3)))

#define KSymbol_Noname          ((ksymbol_t)-1)
#define KSymbol_NewId           ((ksymbol_t)-2)
#define KSymbol_NewRaw          ((ksymbol_t)-3)
#define _NEWID                 KSymbol_NewId
#define _NEWRAW                KSymbol_NewRaw

#define KSymbolAttr_Annotation        (KFLAG_H1|KFLAG_H2)
#define KSymbolAttr_Pattern           (KFLAG_H0|KFLAG_H1|KFLAG_H2)
#define KSymbolAttr_SyntaxList        (KFLAG_H3)
//#define KSymbolAttr_Annotation        (KFLAG_H0|KFLAG_H1)
//#define KSymbolAttr_Pattern           (KFLAG_H0|KFLAG_H2)
#define KSymbol_IsAnnotation(S)       ((S & KSymbolAttr_Pattern) == KSymbolAttr_Annotation)
#define KSymbol_IsPattern(S)          ((S & KSymbolAttr_Pattern) == KSymbolAttr_Pattern)

#ifdef USE_KEYWORD_LIST
static const char *KEYWORD_LIST[] = {
	"", "$Indent", "$Symbol", "$Text", "$Number", "$Member", "$Type",
	"()", "[]", "{}", "$Expr", "$Block", "$Param", "$TypeDecl", "$MethodDecl", "$Token",
	".", "/", "%", "*", "+", "-", "<", "<=", ">", ">=", "==", "!=",
	"&&", "||", "!", "=", ",", "$", ":", ";", /*"@",*/
	"true", "false", "if", "else", "return", // syn
	"new", "void", "script"
};
#endif

#define KSymbol_END              ((ksymbol_t)-1)
#define KSymbol_IndentPattern    (((ksymbol_t)1)|KSymbolAttr_Pattern) /*$Indent*/
#define KSymbol_SymbolPattern    (((ksymbol_t)2)|KSymbolAttr_Pattern) /*$Symbol*/
#define KSymbol_TextPattern      (((ksymbol_t)3)|KSymbolAttr_Pattern) /*$Text*/
#define KSymbol_NumberPattern    (((ksymbol_t)4)|KSymbolAttr_Pattern) /*$Number*/
#define KSymbol_MemberPattern    (((ksymbol_t)5)|KSymbolAttr_Pattern) /*$Member*/
#define KSymbol_TypePattern      (((ksymbol_t)6)|KSymbolAttr_Pattern) /*$Type*/

#define KSymbol_ParenthesisGroup (((ksymbol_t)7)) /*()*/
#define KSymbol_BracketGroup     (((ksymbol_t)8)) /*[]*/
#define KSymbol_BraceGroup       (((ksymbol_t)9)) /*{}*/
#define KSymbol_TypeCastGroup    (((ksymbol_t)7)|KSymbolAttr_Pattern)    /*$()*/
#define KSymbol_TypeParamGroup   (((ksymbol_t)8)|KSymbolAttr_Pattern)    /*$[]*/
#define KSymbol_OptionalGroup    (((ksymbol_t)8)|KSymbol_ATMARK)         /*@[]*/
#define KSymbol_ExprPattern      (((ksymbol_t)10)|KSymbolAttr_Pattern)    /*$Block*/
#define KSymbol_BlockPattern     (((ksymbol_t)11)|KSymbolAttr_Pattern)    /*$Block*/
#define KSymbol_ParamPattern     (((ksymbol_t)12)|KSymbolAttr_Pattern)   /*$Param*/
#define KSymbol_TypeDeclPattern  (((ksymbol_t)13)|KSymbolAttr_Pattern)   /*$TypeDecl*/
#define KSymbol_MethodDeclPattern  (((ksymbol_t)14)|KSymbolAttr_Pattern) /*$MethodDecl*/
#define KSymbol_TokenPattern     (((ksymbol_t)15)|KSymbolAttr_Pattern)   /*$Token*/

//#define KSymbol_NodeOperator        KSymbol_ParamPattern
//#define KSymbol_NodeTerm            KSymbol_SymbolPattern
//#define KSymbol_ParamPattern/*MethodCall*/      KSymbol_ParamPattern

#define KSymbol_DOT     16
#define KSymbol_DIV     (1+KSymbol_DOT)
#define KSymbol_MOD     (2+KSymbol_DOT)
#define KSymbol_MUL     (3+KSymbol_DOT)
#define KSymbol_ADD     (4+KSymbol_DOT)
#define KSymbol_SUB     (5+KSymbol_DOT)
#define KSymbol_LT      (6+KSymbol_DOT)
#define KSymbol_LTE     (7+KSymbol_DOT)
#define KSymbol_GT      (8+KSymbol_DOT)
#define KSymbol_GTE     (9+KSymbol_DOT)
#define KSymbol_EQ      (10+KSymbol_DOT)
#define KSymbol_NEQ     (11+KSymbol_DOT)
#define KSymbol_AND     (12+KSymbol_DOT)
#define KSymbol_OR      (13+KSymbol_DOT)
#define KSymbol_NOT     (14+KSymbol_DOT)
#define KSymbol_LET     (15+KSymbol_DOT)
#define KSymbol_COMMA   (16+KSymbol_DOT)
#define KSymbol_DOLLAR  KSymbolAttr_Pattern
#define KSymbol_ATMARK  KSymbolAttr_Annotation
#define KSymbol_COLON   (17+KSymbol_DOT)
#define KSymbol_SEMICOLON (18+KSymbol_DOT)

#define KSymbol_true      35
#define KSymbol_false     (1+KSymbol_true)
#define KSymbol_if        (2+KSymbol_true)
#define KSymbol_else      (3+KSymbol_true)
#define KSymbol_return    (4+KSymbol_true)
#define KSymbol_new       (5+KSymbol_true)
#define KSymbol_void      (6+KSymbol_true)
#define KSymbol_script    (7+KSymbol_true)

/* MethodName
 * 110   to$(TypeId)
 * 101   as${TypeId}
 * 010   get${symbol}
 * 001   set${symbol}
 */

#define KMethodNameAttr_Type       KFLAG_H0
#define KMethodNameAttr_Coercion   (KFLAG_H0|KFLAG_H1)
#define KMethodNameAttr_Upcast     (KFLAG_H0|KFLAG_H2)
#define KMethodNameAttr_Getter     KFLAG_H1
#define KMethodNameAttr_Setter     KFLAG_H2

#define KMethodName_Unmask(mn)        (mn & (~(KFLAG_H1|KFLAG_H2)))
#define KMethodName_IsGetter(mn)      (KSymbol_Attr(mn) == KMethodNameAttr_Getter)
#define KMethodName_ToGetter(mn)      ((KMethodName_Unmask(mn)) | KMethodNameAttr_Getter)
#define KMethodName_IsSetter(mn)      (KSymbol_Attr(mn) == KMethodNameAttr_Setter)
#define KMethodName_ToSetter(mn)      ((KMethodName_Unmask(mn)) | KMethodNameAttr_Setter)

#define KMethodName_To(cid)            ((cid) | KMethodNameAttr_Coercion)
#define KMethodName_IsCoercion(mn)     ((KSymbol_Unmask(mn)) == KMethodNameAttr_Coercion)
#define KMethodName_As(cid)            ((cid) | KMethodNameAttr_Upcast)
#define KMethodName_IsUpcast(mn)       ((KSymbol_Unmask(mn)) == KMethodNameAttr_Upcast)
#define KMethodName_Fmt2(mn)           KSymbol_prefixText(mn), ((mn & KMethodNameAttr_Type) == KMethodNameAttr_Type ? KType_text(KSymbol_Unmask(mn)) : KSymbol_text(KSymbol_Unmask(mn)))

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
	return _Setjmp(t, NULL);
}
#define ksetjmp  setjmp_mingw
#elif defined(__MINGW32__) || defined(_MSC_VER)
#define ksetjmp  _Setjmp
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
	trace->errorSymbol = KException_(T); trace->faultType = F

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

struct KObjectVisitor;

/* ------------------------------------------------------------------------ */

#define DefineBasicTypeList(MACRO)\
	MACRO(void)\
	MACRO(var)\
	MACRO(Object)\
	MACRO(Boolean)\
	MACRO(Int)\
	MACRO(String)\
	MACRO(Array)\
	MACRO(Param)\
	MACRO(Method)\
	MACRO(Func)\
	MACRO(Exception)\
	MACRO(NameSpace)\
	MACRO(System)\
	MACRO(0)\

#define TypeDefMacro(T)\
	typedef /*const*/ struct k##T##Var      k##T;\
	typedef struct k##T##Var      k##T##Var;\

#define TypeEnumMacro(T)         KType_##T,

typedef enum {
	DefineBasicTypeList(TypeEnumMacro)
	KType_ERROR = -1 /*  sentinel */
} KType_;

DefineBasicTypeList(TypeDefMacro)

#define kAbstractObject                 const void
//typedef const struct kObjectVar         kObject;
//typedef struct kObjectVar               kObjectVar;
//typedef const struct kBooleanVar        kBoolean;
//typedef struct kBooleanVar              kBooleanVar;
//typedef const struct kIntVar            kInt;
//typedef struct kIntVar                  kIntVar;
//typedef const struct kStringVar         kString;
//typedef struct kStringVar               kStringVar;
//typedef const struct kArrayVar          kArray;
//typedef struct kArrayVar                kArrayVar;
//typedef const struct kParamVar          kParam;
//typedef struct kParamVar                kParamVar;
//typedef const struct kMethodVar         kMethod;
//typedef struct kMethodVar               kMethodVar;
//typedef const struct kFuncVar           kFunc;
//typedef struct kFuncVar                 kFuncVar;
//typedef const struct kExceptionVar           kException;
//typedef struct kExceptionVar                 kExceptionVar;
//typedef struct kNameSpaceVar            kNameSpace;
//typedef struct kNameSpaceVar            kNameSpaceVar;

/* sugar.h */

typedef const struct kTokenVar          kToken;
typedef struct kTokenVar                kTokenVar;
typedef struct kNodeVar                 kNode;
typedef struct kNodeVar                 kNodeVar;

#define kTokenNULL kToken
#define kNodeNULL  kNode
#define kNameSpaceNULL kNameSpace

#define KMETHOD    void  /*CC_FASTCALL_*/
typedef KMETHOD   (*KMethodFunc)(KonohaContext*, struct KonohaValueVar *);

struct ExecutionEngineModule {
	const KModuleInfo          *ExecutionEngineInfo;
	void                      (*DeleteExecutionEngine)(KonohaContext *kctx);
	const struct KBuilderAPI *(*GetDefaultBuilderAPI)(void);
	struct KVirtualCode      *(*GetDefaultBootCode)(void);
	struct KVirtualCode      *(*GenerateVirtualCode)(KonohaContext *, kMethod *mtd, kNode *block, int option);
	KMethodFunc               (*GenerateMethodFunc)(KonohaContext *, struct KVirtualCode *);
	void                      (*SetMethodCode)(KonohaContext *, kMethodVar *mtd, struct KVirtualCode *, KMethodFunc func);
	struct KVirtualCode     *(*RunExecutionEngine)(KonohaContext *kctx, struct KonohaValueVar *sfp, struct KVirtualCode *pc);
};

struct KonohaFactory {
	// settings
	const char  *name;
	size_t       stacksize;
	volatile int safePointFlag;
	int          verbose;
	int          exitStatus;

	/* LowLevel API */
	/* memory allocation / deallocation */
	void *(*malloc_i)(size_t size);
	void  (*free_i)  (void *ptr);

	// setjmp
	int     (*setjmp_i)(jmpbuf_i);
	void    (*longjmp_i)(jmpbuf_i, int);

	// system info
	const char *(*getenv_i)(const char *);

	// time
	unsigned long long (*getTimeMilliSecond)(void);

	/* message */
	int    (*printf_i)(const char *fmt, ...) __PRINTFMT(2, 3);
	int    (*vprintf_i)(const char *fmt, va_list args);
	int    (*snprintf_i)(char *str, size_t size, const char *fmt, ...);
	int    (*vsnprintf_i)(char *str, size_t size, const char *fmt, va_list args);
	char  *(*readline_i)(const char *prompt);
	int    (*add_history_i)(const char *);

	void    (*qsort_i)(void *base, size_t nel, size_t width, int (*compar)(const void *, const void *));
	// abort
	void    (*exit_i)(int p, const char *file, int line);

	// pthread
	struct ThreadModule {
		int     (*thread_create_i)(kthread_t *thread, const kthread_attr_t *attr, void *(*f)(void *), void *arg);
		int     (*thread_join_i)(kthread_t thread, void **);
		int     (*thread_mutex_init_i)(kmutex_t *mutex, const kmutexattr_t *attr);
		int     (*thread_mutex_lock_i)(kmutex_t *mutex);
		int     (*thread_mutex_trylock_i)(kmutex_t *mutex);
		int     (*thread_mutex_unlock_i)(kmutex_t *mutex);
		int     (*thread_mutex_destroy_i)(kmutex_t *mutex);
		int     (*thread_mutex_init_recursive)(kmutex_t *mutex);
		int     (*thread_cond_init_i)(kmutex_cond_t *cond, const kmutex_condattr_t *attr);
		int     (*thread_cond_wait_i)(kmutex_cond_t *cond, kmutex_t *mutex);
		int     (*thread_cond_signal_i)(kmutex_cond_t *cond);
		int     (*thread_cond_broadcast_i)(kmutex_cond_t *cond);
		int     (*thread_cond_destroy_i)(kmutex_cond_t *cond);
	} ThreadModule;

	/* high-level functions */
	kbool_t  (*LoadPlatformModule)(struct KonohaFactory*, const char *moduleName, ModuleType);

	// file load
	const char *(*FormatPackagePath)(KonohaContext *, char *buf, size_t bufsiz, const char *packageName, const char *ext);
	KPackageHandler* (*LoadPackageHandler)(KonohaContext *, const char *packageName);
	void (*BEFORE_LoadScript)(KonohaContext *, const char *filename);
	int (*loadScript)(const char *filePath, long uline, void *thunk, int (*evalFunc)(const char*, long, int *, void *));
	void (*AFTER_LoadScript)(KonohaContext *, const char *filename);

	const char *(*shortFilePath)(const char *path);
	const char *(*formatTransparentPath)(char *buf, size_t bufsiz, const char *parent, const char *path);

	/* Logging API */
	struct LoggerModule {
		const KModuleInfo *LoggerInfo;
		void  (*syslog_i)(int priority, const char *message, ...) __PRINTFMT(2, 3);
		void  (*vsyslog_i)(int priority, const char *message, va_list args);
		void  (*TraceDataLog)(KonohaContext *kctx, KTraceInfo *trace, int, logconf_t *, ...);
	} LoggerModule;

	/* Diagnosis API */
	struct DiagnosisModule {
		const KModuleInfo *DiagnosisInfo;
		kbool_t (*CheckStaticRisk)(KonohaContext *, const char *keyword, size_t keylen, kfileline_t uline);
		void    (*CheckDynamicRisk)(KonohaContext *, const char *keyword, size_t keylen, KTraceInfo *);
		int (*DiagnosisSoftwareProcess)(KonohaContext *, kfileline_t uline, KTraceInfo *);
		int (*DiagnosisSystemError)(KonohaContext *, int fault);
		int (*DiagnosisSystemResource)(KonohaContext *, KTraceInfo *);
		int (*DiagnosisFileSystem)(KonohaContext *, const char *path, size_t pathlen, KTraceInfo *);
		int (*DiagnosisNetworking)(KonohaContext *, const char *path, size_t pathlen, int port, KTraceInfo *);
		kbool_t (*DiagnosisCheckSoftwareTestIsPass)(KonohaContext *, const char *filename, int line);
	} DiagnosisModule;

	/* Console API */
	struct ConsoleModule {
		const KModuleInfo *ConsoleInfo;
		void  (*ReportUserMessage)(KonohaContext *, kinfotag_t, kfileline_t pline, const char *, int isNewLine);
		void  (*ReportCompilerMessage)(KonohaContext *, kinfotag_t, kfileline_t pline, const char *);
		void  (*ReportCaughtException)(KonohaContext *, kException *e, struct KonohaValueVar *bottomStack, struct KonohaValueVar *topStack);
		void  (*ReportDebugMessage)(const char *file, const char *func, int line, const char *fmt, ...) __PRINTFMT(4, 5);
		int   (*InputUserApproval)(KonohaContext *, const char *message, const char *yes, const char *no, int defval);
		char *(*InputUserText)(KonohaContext *, const char *message, int flag);
		char *(*InputUserPassword)(KonohaContext *, const char *message);
	} ConsoleModule;

	/* Garbage Collection API */
	struct GCModule {
		const KModuleInfo *GCInfo;
		void* (*Kmalloc)(KonohaContext*, size_t, KTraceInfo *);
		void* (*Kzmalloc)(KonohaContext*, size_t, KTraceInfo *);
		void  (*Kfree)(KonohaContext*, void *, size_t);
		void (*InitGcContext)(KonohaContext *kctx);
		void (*DeleteGcContext)(KonohaContext *kctx);
		void (*ScheduleGC)(KonohaContext *kctx, KTraceInfo *trace);
		struct kObjectVar *(*AllocObject)(KonohaContext *kctx, size_t size, KTraceInfo *);
		kbool_t (*IsKonohaObject)(KonohaContext *kctx, void *ptr);
		void  (*VisitObject)(struct KObjectVisitor *visitor, struct kObjectVar *obj);
		void  (*WriteBarrier)(KonohaContext *, struct kObjectVar *);
		void  (*UpdateObjectField)(struct kObjectVar *parent, struct kObjectVar *oldPtr, struct kObjectVar *newVal);
	} GCModule;

	/* Event Handler API */
	struct EventModule {
		const KModuleInfo *EventInfo;
		struct EventContext *eventContext;
		void (*StartEventHandler)(KonohaContext *kctx, void *args);
		void (*StopEventHandler)(KonohaContext *kctx, void *args);
		void (*EnterEventContext)(KonohaContext *kctx, void *args);
		void (*ExitEventContext)(KonohaContext *kctx, void *args);
		kbool_t (*EmitEvent)(KonohaContext *kctx, struct JsonBuf *json, KTraceInfo *);
		void (*DispatchEvent)(KonohaContext *kctx, kbool_t (*consume)(KonohaContext *kctx, struct JsonBuf *, KTraceInfo *), KTraceInfo *);
		void (*WaitEvent)(KonohaContext *kctx, kbool_t (*consume)(KonohaContext *kctx, struct JsonBuf *, KTraceInfo *), KTraceInfo *);
	} EventModule;

	/* I18N Module */
	struct I18NModule {
		const KModuleInfo *I18NInfo;
		uintptr_t   (*iconv_open_i)(KonohaContext *, const char *tocode, const char *fromcode, KTraceInfo *);
		size_t      (*iconv_i)(KonohaContext *, uintptr_t iconv, ICONV_INBUF_CONST char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft, int *isTooBigRef, KTraceInfo *trace);
		int         (*iconv_close_i)(KonohaContext *, uintptr_t iconv);
		const char *systemCharset;
		kbool_t     (*isSystemCharsetUTF8)(KonohaContext *);
		uintptr_t   (*iconvUTF8ToSystemCharset)(KonohaContext *, KTraceInfo *);
		uintptr_t   (*iconvSystemCharsetToUTF8)(KonohaContext *, KTraceInfo *);
		const char *(*formatSystemPath)(KonohaContext *kctx, char *buf, size_t bufsiz, const char *path, size_t pathsize, KTraceInfo *);
		const char *(*formatKonohaPath)(KonohaContext *kctx, char *buf, size_t bufsiz, const char *path, size_t pathsize, KTraceInfo *);
	} I18NModule;

	/* ExecutionEngine */
	struct ExecutionEngineModule ExecutionEngineModule;

	/* JSON_API */
	struct JsonModule {
		const KModuleInfo *JsonDataInfo;
		void        *JsonHandler;  // define this in each module if necessary
		void        (*InitJsonContext)(KonohaContext *kctx);
		void        (*DeleteJsonContext)(KonohaContext *kctx);
		kbool_t     (*IsJsonType)(struct JsonBuf *, KJSONTYPE);
		struct JsonBuf* (*CreateJson)(KonohaContext *, struct JsonBuf *jsonbuf, KJSONTYPE type, ...);
		kbool_t     (*ParseJson)(KonohaContext *, struct JsonBuf *, const char *, size_t, KTraceInfo *);
		void        (*FreeJson)(KonohaContext *, struct JsonBuf *);
		const char *(*JsonToNewText)(KonohaContext *, struct JsonBuf *);
		size_t      (*DoJsonEach)(KonohaContext *, struct JsonBuf *, void *thunk, void (*doEach)(KonohaContext *, const char *key, size_t len, struct JsonBuf *, void *));

		kbool_t     (*RetrieveJsonKeyValue)(KonohaContext *, struct JsonBuf *, const char *key, size_t keylen, struct JsonBuf *newbuf);
		kbool_t     (*SetJsonKeyValue)(KonohaContext *, struct JsonBuf *, const char *key, size_t keylen, struct JsonBuf *otherbuf);
		kbool_t     (*SetJsonValue)(KonohaContext *, struct JsonBuf *, const char *key, size_t keylen, KJSONTYPE, ...);

		kbool_t     (*GetJsonBoolean)(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, kbool_t defval);
		int64_t     (*GetJsonInt)(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, int64_t defval);
		double      (*GetJsonFloat)(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, double defval);
		const char *(*GetJsonText)(KonohaContext *kctx, struct JsonBuf *jsonbuf, const char *key, size_t keylen_or_zero, const char *defval);
		size_t      (*GetJsonSize)(KonohaContext *kctx, struct JsonBuf *jsonbuf);
		kbool_t     (*RetrieveJsonArrayAt)(KonohaContext *kctx, struct JsonBuf *jsonbuf, size_t index, struct JsonBuf *otherbuf);
		kbool_t     (*SetJsonArrayAt)(KonohaContext *kctx, struct JsonBuf *jsonbuf, size_t index, struct JsonBuf *otherbuf);
		kbool_t     (*AppendJsonArray)(KonohaContext *kctx, struct JsonBuf *jsonbuf, struct JsonBuf *otherbuf);
	} JsonModule;

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
			PLATAPI LoggerModule.TraceDataLog(kctx, TRACE, 0/*LOGKEY*/, &_logconf, LogText("Api", APINAME), ## __VA_ARGS__, LOG_END);\
		}\
	} while(0)

#define KTraceChangeSystemPoint(TRACE, APINAME, ...)    do {\
		logconf_t _logconf = {(logpolicy_t)(isRecord|LOGPOOL_INIT|SystemChangePoint)};\
		if(trace != NULL && KFlag_Is(int, _logconf.policy, isRecord)) { \
			PLATAPI LoggerModule.TraceDataLog(kctx, TRACE, 0/*LOGKEY*/, &_logconf, LogText("Api", APINAME), ## __VA_ARGS__, LOG_END);\
		}\
	} while(0)

#define KTraceApi(TRACE, POLICY, APINAME, ...)    do {\
		static logconf_t _logconf = {(logpolicy_t)(isRecord|LOGPOOL_INIT|POLICY)};\
		if(trace != NULL && KFlag_Is(int, _logconf.policy, isRecord)) {\
			PLATAPI LoggerModule.TraceDataLog(kctx, TRACE, 0/*LOGKEY*/, &_logconf, LogText("Api", APINAME), ## __VA_ARGS__, LOG_END);\
		}\
	} while(0)

#define KTraceResponseCheckPoint(TRACE, POLICY, APINAME, STMT, ...)    do {\
		static logconf_t _logconf = {(logpolicy_t)(isRecord|LOGPOOL_INIT|POLICY)};\
		if(trace != NULL && KFlag_Is(int, _logconf.policy, isRecord)) {\
			uint64_t _startTime = PLATAPI getTimeMilliSecond();\
			PLATAPI LoggerModule.TraceDataLog(kctx, TRACE, 0/*LOGKEY*/, &_logconf, LogText("Api", APINAME), ## __VA_ARGS__, LOG_END);\
			STMT;\
			uint64_t _endTime = PLATAPI getTimeMilliSecond();\
			PLATAPI LoggerModule.TraceDataLog(kctx, TRACE, 0/*LOGKEY*/, &_logconf, LogText("Api", APINAME), LogUint("ElapsedTime", (_endTime - _startTime)), ## __VA_ARGS__, LOG_END);\
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
		uintptr_t   unboxValue;
		kObject    *ObjectValue;
		kString    *StringValue;
		kFunc      *FuncValue;
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
		char                         *bytebuf;
		const struct KClassVar      **classItems;
		KKeyValue                    *keyValueItems;
		kObject                     **ObjectItems;
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

typedef const struct KClassVar     KClass;
typedef struct KClassVar           KClassVar;
typedef struct KClassField         KClassField;

typedef const struct KRuntimeVar             KRuntime;
typedef struct KRuntimeVar                   KRuntimeVar;
typedef const struct KRuntimeContextVar      KRuntimeContext;
typedef struct KRuntimeContextVar            KRuntimeContextVar;
typedef struct KonohaValueVar                KonohaStack;
typedef struct KonohaValueVar                KonohaValue;

typedef struct KRuntimeModule        KRuntimeModule;
typedef struct KContextModule        KContextModule;
struct KObjectVisitor;

struct KonohaContextVar {
	uintptr_t                  safepoint; // set to 1
	KonohaStack               *esp;
	PlatformApi               *platApi;
	KonohaLib                 *klib;
	KRuntime                  *share;
	KRuntimeContextVar        *stack;
	KRuntimeModule           **modshare;
	KContextModule           **modlocal;
	void                      *gcContext; // defined in each module
};

// share, local
struct KRuntimeVar {
	KGrowingArray             classTable;
	KHashMap                 *longClassNameMapNN;
	kmutex_t                 *classTableMutex;
	/* system shared const */
	kArray                   *GlobalConstList;
	kObject                  *constNull;   /*on GlobalConstList*/
	kBoolean                 *constTrue;   /*on GlobalConstList*/
	kBoolean                 *constFalse;  /*on GlobalConstList*/
	kString                  *emptyString; /*on GlobalConstList*/
	kArray                   *emptyArray;  /*on GlobalConstList*/

	kmutex_t                 *filepackMutex;
	kArray                   *fileIdList;  /*on GlobalConstList*/
	KHashMap                 *fileIdMap_KeyOnList;   //

	kArray                   *packageIdList; /*on GlobalConstList*/
	KHashMap                 *packageIdMap_KeyOnList;
	KHashMap                 *packageMapNO;

	kmutex_t                 *symbolMutex;
	kArray                   *symbolList;    /*on GlobalConstList*/
	KHashMap                 *symbolMap_KeyOnList;

	kmutex_t                 *paramMutex;
	kArray                   *paramList;     /*on GlobalConstList*/
	KHashMap                 *paramMap_KeyOnList;
	kArray                   *paramdomList;  /*on GlobalConstList*/
	KHashMap                 *paramdomMap_KeyOnList;
};

#define kContext_Debug          ((kshortflag_t)(1<<0))
#define kContext_Interactive    ((kshortflag_t)(1<<1))
#define kContext_CompileOnly    ((kshortflag_t)(1<<2))
#define kContext_Test           ((kshortflag_t)(1<<3))
#define kContext_Trace          ((kshortflag_t)(1<<4))

#define KonohaContext_Is(P, X)   (KFlag_Is(kshortflag_t,(X)->stack->flag, kContext_##P))
#define KonohaContext_Set(P, X)   KFlag_Set1(kshortflag_t, (X)->stack->flag, kContext_##P)

struct KRuntimeContextVar {
	KonohaStack               *stack;
	size_t                     stacksize;
	KonohaStack               *stack_uplimit;
	kArray                    *ContextConstList;
	kArray                    *gcStack; /* ContextConstList */
	KGrowingArray              cwb;
	// local info
	kshortflag_t               flag;
	KonohaContext             *rootctx;
	void                      *cstack_bottom;  // for GC
	// Eval
	ktypeattr_t                evalty;
	kushort_t                  evalidx;
	// Exception
	kException                *ThrownException;
	jmpbuf_i                  *evaljmpbuf;
	KonohaStack               *bottomStack;
	KonohaStack               *topStack;
};

// module
#define KRuntimeModule_MAXSIZE    32
#define MOD_gc         1
#define MOD_sugar      3
#define MOD_konoha     6

#define MOD_APACHE     17
#define MOD_EVENT      18

struct KRuntimeModule {
	const char *name;
	int         mod_id;
	void        (*setupModuleContext)(KonohaContext*, struct KRuntimeModule *, int newctx);
	void        (*freeModule)(KonohaContext*, struct KRuntimeModule *);
	size_t      allocSize;
	kmutex_t   *moduleMutex;
};

struct KContextModule {
	uintptr_t unique;
	void (*reftrace)(KonohaContext*, struct KContextModule *, struct KObjectVisitor *);
	void (*free)(KonohaContext*, struct KContextModule *);
};

#define K_FRAME_NCMEMBER \
	uintptr_t   unboxValue;\
	kbool_t     boolValue; \
	kint_t      intValue; \
	kfloat_t    floatValue; \
	struct KonohaValueVar *previousStack;\
	intptr_t    shift0;  \
	struct KVirtualCode  *pc; \
	kMethod     *calledMethod;\
	kNameSpace  *calledNameSpace;\
	uintptr_t    calledFileLine

#define K_FRAME_MEMBER \
	kObject     *asObject;\
	kObjectVar  *asObjectVar; \
	const struct kNumberVar     *asNumber;\
	kBoolean    *asBoolean;\
	kInt        *asInt; \
	kString     *asString;\
	kArray      *asArray;\
	kMethod     *asMethod;\
	kFunc       *asFunc; \
	kNameSpace  *asNameSpace;\
	struct kSyntaxVar     *asSyntax;\
	kToken      *asToken;\
	kNode       *asNode;\
	kException  *asException;\
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

typedef enum kformat_t {
	ToStringFormat, JSONFormat
} kformat_t;

#define CLASSAPI \
		void         (*init)(KonohaContext*, kObject*, void *conf);\
		void         (*reftrace)(KonohaContext*, kObject*, struct KObjectVisitor *visitor);\
		void         (*free)(KonohaContext*, kObject *);\
		kObject*     (*fnull)(KonohaContext*, KClass *);\
		uintptr_t    (*unbox)(KonohaContext*, kObject *);\
		void         (*format)(KonohaContext*, KonohaValue *, int, KBuffer *);\
		int          (*compareTo)(KonohaContext *, kObject *, kObject *);\
		int          (*compareUnboxValue)(uintptr_t, uintptr_t);\
		void         (*initdef)(KonohaContext*, KClassVar*, KTraceInfo *);\
		kbool_t      (*isSubType)(KonohaContext*, KClass*, KClass *);\
		KClass*      (*realtype)(KonohaContext*, KClass*, KClass *)

typedef struct KDEFINE_CLASS {
	const char *structname;
	ktypeattr_t     typeId;         kshortflag_t    cflag;
	ktypeattr_t     baseTypeId;     ktypeattr_t     superTypeId;
	ktypeattr_t     rtype;          kushort_t       cparamsize;
	struct kparamtype_t   *cParamItems;
	size_t         cstruct_size;
	KClassField   *fieldItems;
	kushort_t      fieldsize;       kushort_t fieldAllocSize;
	CLASSAPI;
} KDEFINE_CLASS;

#define STRUCTNAME(C) \
	.structname = #C,\
	.typeId = KTypeAttr_NewId,\
	.cstruct_size = sizeof(k##C)

#define UNBOXNAME(C) \
	.structname = #C,\
	.typeId = KTypeAttr_NewId

#define SETSTRUCTNAME(VAR, C) do{\
		VAR.structname = #C;\
		VAR.typeId = KTypeAttr_NewId;\
		VAR.cstruct_size = sizeof(k##C);\
	}while(0)

#define SETUNBOXNAME(VAR, C) do{\
		VAR.structname = #C;\
		VAR.typeId = KTypeAttr_NewId;\
	}while(0)

//KClassVar;
typedef uintptr_t kmagicflag_t;

struct KClassVar {
	CLASSAPI;
	kpackageId_t     packageId;    kpackageId_t    packageDomain;
	ktypeattr_t      typeId;       kshortflag_t    cflag;
	ktypeattr_t      baseTypeId;   ktypeattr_t     superTypeId;
	ktypeattr_t      p0;           kparamId_t      cparamdom;
	kmagicflag_t magicflag;
	size_t     cstruct_size;
	KClassField         *fieldItems;
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
	KClass              *searchSimilarClassNULL;
	KClass              *searchSuperMethodClassNULL;
};

struct KClassField {
	ktypeattr_t     attrTypeId;
	ksymbol_t       name;
};

/* ----------------------------------------------------------------------- */

//#define KType_void              ((ktypeattr_t)0)
//#define KType_var               ((ktypeattr_t)1)
//#define KType_Object            ((ktypeattr_t)2)
//#define KType_Boolean           ((ktypeattr_t)3)
//#define KType_Int               ((ktypeattr_t)4)
//#define KType_String            ((ktypeattr_t)5)
//#define KType_Array             ((ktypeattr_t)6)
//#define KType_Param             ((ktypeattr_t)7)
//#define KType_Method            ((ktypeattr_t)8)
//#define KType_Func              ((ktypeattr_t)9)
//#define KType_NameSpace         ((ktypeattr_t)10)
//#define KType_System            ((ktypeattr_t)11)
//#define KType_0                 ((ktypeattr_t)12)    /* Parameter Type*/

#define KClass_void                 KClass_(KType_void)
#define KClass_Object               KClass_(KType_Object)
#define KClass_Boolean              KClass_(KType_Boolean)
#define KClass_Int                  KClass_(KType_Int)
#define KClass_String               KClass_(KType_String)
#define KClass_Array                KClass_(KType_Array)
#define KClass_Param                KClass_(KType_Param)
#define KClass_Method               KClass_(KType_Method)
#define KClass_Func                 KClass_(KType_Func)
#define KClass_Exception            KClass_(KType_Exception)
#define KClass_NameSpace            KClass_(KType_NameSpace)
#define KClass_System               KClass_(KType_System)
#define KClass_var                  KClass_(KType_var)

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

#define KClassFlag_SUPERMASK         KClassFlag_Prototype|KClassFlag_Singleton

#define KClassFlag_void              KClassFlag_TypeVar|KClassFlag_UnboxType|KClassFlag_Singleton|KClassFlag_Final
#define KClassFlag_var               KClassFlag_TypeVar|KClassFlag_UnboxType|KClassFlag_Singleton|KClassFlag_Final
#define KClassFlag_Object            KClassFlag_Nullable
#define KClassFlag_boolean           KClassFlag_Nullable|KClassFlag_Immutable|KClassFlag_UnboxType|KClassFlag_Final
#define KClassFlag_int               KClassFlag_Nullable|KClassFlag_Immutable|KClassFlag_UnboxType|KClassFlag_Final
#define KClassFlag_String            KClassFlag_Nullable|KClassFlag_Immutable|KClassFlag_Final
#define KClassFlag_Array             KClassFlag_Nullable|KClassFlag_Final
#define KClassFlag_Param             KClassFlag_Nullable|KClassFlag_Final
#define KClassFlag_Method            KClassFlag_Nullable|KClassFlag_Final
#define KClassFlag_Func              KClassFlag_Nullable|KClassFlag_Final
#define KClassFlag_Exception         KClassFlag_Nullable|KClassFlag_Final
#define KClassFlag_NameSpace         KClassFlag_Nullable|KClassFlag_Final
#define KClassFlag_System            KClassFlag_Nullable|KClassFlag_Singleton|KClassFlag_Final
#define KClassFlag_0                 KClassFlag_TypeVar|KClassFlag_UnboxType|KClassFlag_Singleton|KClassFlag_Final

#define KClass_(T)                kctx->share->classTable.classItems[KTypeAttr_Unmask(T)]
#define KClass_cparam(CT)         kctx->share->paramdomList->ParamItems[(CT)->cparamdom]
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

typedef struct kObjectHeader {
	kmagicflag_t magicflag;
	KClass *ct;
	KProtoMap *prototypePtr;
} kObjectHeader;

struct kObjectVar {
	kObjectHeader h;
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
	kObjectHeader h;
	ABSTRACT_NUMBER;
};

struct kBooleanVar /* extends kNumber */ {
	kObjectHeader h;
	ABSTRACT_NUMBER;
};

#define IS_Boolean(o)              (kObject_typeId(o) == KType_Boolean)
#define IS_TRUE(o)                 (kObject_baseTypeId(o) == KType_Boolean && kNumber_ToBool(o))
#define IS_FALSE(o)                (kObject_baseTypeId(o) == KType_Boolean && kNumber_ToBool(o) == 0)
#define new_Boolean(kctx, c)       ((c) ? K_TRUE : K_FALSE)
#define kNumber_ToInt(o)                 (((kBoolean *)o)->intValue)
#define kNumber_ToFloat(o)               (((kBoolean *)o)->floatValue)
#define kNumber_ToBool(o)                (((kBoolean *)o)->boolValue)

/* ------------------------------------------------------------------------ */
/* Int */

struct kIntVar /* extends kNumber */ {
	kObjectHeader h;
	ABSTRACT_NUMBER;
};

#define IS_Int(o)              (kObject_typeId(o) == KType_Int)

/* ------------------------------------------------------------------------ */
/* String */

typedef enum {
	VirtualType_Text             =   KType_void,    /*special use for const char*/
	VirtualType_KClass           =   KType_var,     /*special use for KClass*/
	VirtualType_StaticMethod     =   KType_0        /*special use for Method*/
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
	kObjectHeader h;
	COMMON_BYTEARRAY;
};

struct kStringVar /* extends _Bytes */ {
	kObjectHeader h;
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
	StringPolicy_FreeKBuffer =  (1<<6)    /* KBuffer_Stringfy */
} StringPolicy;

typedef enum { NonZero, EnsureZero } StringfyPolicy;

#define K_NULLTEXT         "null"
#define kString_text(s)    ((const char *) (kObject_class(s)->unbox(kctx, (kObject *)s)))
#define kString_size(s)    ((s)->bytesize)

/* ------------------------------------------------------------------------ */
//## class Array   Object;

#define IS_Array(o)              (kObject_baseTypeId(o) == KType_Array)

#define kArrayFlag_UnboxData     kObjectFlag_Local1
#define kArrayFlag_Debug         kObjectFlag_Local2
#define kArray_Is(P, o)          (KFlag_Is(uintptr_t,(o)->h.magicflag, kArrayFlag_##P))
#define kArray_Set(P, o, b)      KFlag_Set(uintptr_t,(o)->h.magicflag, kArrayFlag_##P,b)

struct kArrayVar {
	kObjectHeader h;
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
		kNameSpace     **NameSpaceItems;
		struct kSyntaxVar **SyntaxItems;
		kToken         **TokenItems;
		kTokenVar      **TokenVarItems;
		kNode          **NodeItems;
		kNodeVar       **NodeVarItems;
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
	kObjectHeader h;
	ktypeattr_t rtype; kushort_t psize;
	kparamtype_t paramtypeItems[3];
};

/* ------------------------------------------------------------------------ */
/* Method */

#define IS_Method(o)                 (kObject_baseTypeId(o) == KType_Method)

#ifdef USE_MethodFlagData
static const char *MethodFlagData[] = {
	"Public", "Virtual", "Final", "Const", "Static", "Immutable", "Compilation",
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
#define kMethod_Compilation          ((uintptr_t)(1<<6))

// call rule
#define kMethod_Coercion             ((uintptr_t)(1<<7))
#define kMethod_Restricted           ((uintptr_t)(1<<8))
#define kMethod_FastCall             ((uintptr_t)(1<<9))
#define kMethod_SmartReturn          ((uintptr_t)(1<<10))
#define kMethod_Variadic             ((uintptr_t)(1<<11))
#define kMethod_Iterative            ((uintptr_t)(1<<12))

// compatible
#define kMethod_CCompatible          ((uintptr_t)(1<<13))
#define kMethod_JSCompatible         ((uintptr_t)(1<<14))
#define kMethod_JCompatible          ((uintptr_t)(1<<15))
#define kMethod_Accountable          ((uintptr_t)(1<<16))

// internal
#define kMethod_Hidden               ((uintptr_t)(1<<17))
#define kMethod_Abstract             ((uintptr_t)(1<<18))
#define kMethod_Overloaded           ((uintptr_t)(1<<19))
#define kMethod_Override             ((uintptr_t)(1<<20))
#define kMethod_IgnoredOverride      ((uintptr_t)(1<<21))
#define kMethod_DynamicCall          ((uintptr_t)(1<<22))
#define kMethod_StaticError          ((uintptr_t)(1<<23))
#define kMethod_Warning              ((uintptr_t)(1<<24))

#define kMethod_WeakCoercion         kMethod_Coercion|kMethod_Warning

#define kMethod_Is(P, MTD)            (KFlag_Is(uintptr_t, (MTD)->flag, kMethod_##P))
#define kMethod_Set(P, MTD, B)        KFlag_Set(uintptr_t, (MTD)->flag, kMethod_##P, B)

#define kMethod_GetParam(mtd)        kctx->share->paramList->ParamItems[mtd->paramid]
#define kMethod_GetReturnType(mtd)   KClass_((kMethod_GetParam(mtd))->rtype)
#define kMethod_IsReturnFunc(mtd)    (KClass_((kMethod_GetParam(mtd))->rtype)->baseTypeId == KType_Func)
#define kMethod_ParamSize(mtd)       ((kMethod_GetParam(mtd))->psize)
#define kMethod_Fmt3(mtd)            KType_text((mtd)->typeId),  KMethodName_Fmt2((mtd)->mn)

/* method data */
#define DEND     (-1)

struct KVirtualCodeAPI {
	void (*FreeVirtualCode)(KonohaContext *kctx, struct KVirtualCode *);
	void (*WriteVirtualCode)(KonohaContext *kctx, KBuffer *, struct KVirtualCode *);
};

struct kMethodVar {
	kObjectHeader     h;
	KMethodFunc       invokeKMethodFunc;
	union {
		struct KVirtualCode     *vcode_start;
		struct KVirtualCodeAPI **virtualCodeApi_plus1;
	};
	uintptr_t       flag;
	ktypeattr_t     typeId;       kmethodn_t  mn;
	kparamId_t      paramid;      kparamId_t paramdom;
	kshort_t        delta;        kpackageId_t packageId;
	kToken         *SourceToken;
	union {
		kNameSpace   *LazyCompileNameSpace;       // lazy compilation
		kNode        *CompiledNode;
	};
	uintptr_t         serialNumber;
};

typedef struct KMethodMatch {
	kNameSpace   *ns;
	ksymbol_t     mn;
	kushort_t     paramsize;
	kparamId_t    paramdom;
	kparamtype_t *param;
	kbool_t       isBreak;
	kMethod      *foundMethodNULL;
	kArray       *foundMethodListNULL;
} KMethodMatch;

typedef enum {
	KMethodMatch_NoOption   = 1,
	KMethodMatch_CamelStyle = 1 << 1
} KMethodMatchOption;

typedef kbool_t (*KMethodMatchFunc)(KonohaContext *kctx, kMethod *mtd, KMethodMatch *m);

/* Stack Layout
 * sfp |StackIdx       |   -4    |   -3    |   -2    |   -1    |    0    |    1    | ...
 *     |Boxed |Unboxed |    |    |    |Shft|    | PC |    |Mtd |    |    |    |    |
 */
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
	kObjectHeader h;
	kMethod *method;
	kObject *env;
};


/* ------------------------------------------------------------------------ */
/* Exception */

#define IS_Exception(o)              (kObject_baseTypeId(o) == KType_Exception)

struct kExceptionVar {
	kObjectHeader h;
	ksymbol_t symbol;  kushort_t fault;
	kfileline_t  uline;
	kString     *Message;
	kArray      *StackTraceList;
};

/* ------------------------------------------------------------------------ */
/* NameSpace */

#define IS_NameSpace(O)  (kObject_class(O) == KClass_NameSpace)
#define kNameSpace_sizeConstTable(ns)    (ns->constTable.data.bytesize / sizeof(KKeyValue))

struct kNameSpaceVar {
	kObjectHeader h;
	kNameSpace                        *parentNULL;
	kpackageId_t packageId;            kshortflag_t syntaxOption;
	kArray                            *NameSpaceConstList;
	kArray                            *importedNameSpaceList;
	KDict                              constTable;
	kArray                            *metaPatternList;
	kObject                           *globalObjectNULL;
	kArray                            *methodList_OnList;   // default K_EMPTYARRAY
	size_t                             sortedMethodList;
	// the below references are defined in sugar
	void                              *tokenMatrix;
//	KHashMap                          *syntaxMapNN;
	const struct KBuilderAPI          *builderApi;
	KKeyValue                         *typeVariableItems;
	size_t                             typesize;
	struct KGammaLocalData            *genv;
};

// NameSpace_syntaxOption

#define kNameSpace_DefaultSyntaxOption               kNameSpace_ImplicitField|kNameSpace_NoSemiColon
#define kNameSpace_Is(P, ns)                         (KFlag_Is(kshortflag_t, (ns)->syntaxOption, kNameSpace_##P))
#define kNameSpace_Set(P, ns, B)                     KFlag_Set(kshortflag_t, ((kNameSpaceVar *)ns)->syntaxOption, kNameSpace_##P, B)

#define kNameSpace_Override                          ((kshortflag_t)(1<<1))
#define kNameSpace_Ambigious                         ((kshortflag_t)(1<<2))

#define kNameSpace_NoSemiColon                       ((kshortflag_t)(1<<3))
#define kNameSpace_TypeInference                     ((kshortflag_t)(1<<4))
#define kNameSpace_ImplicitField                     ((kshortflag_t)(1<<5))
#define kNameSpace_ImplicitGlobalVariable            ((kshortflag_t)(1<<6))
#define kNameSpace_ImplicitCoercion                  ((kshortflag_t)(1<<7))

#define kNameSpace_StaticError                       ((kshortflag_t)(1<<14))
#define KPushNameSpaceOption(ns)  kshortflag_t _syntaxOption = ns->syntaxOption
#define KPopNameSpaceOption(ns)   ns->syntaxOption = _syntaxOption


/* ------------------------------------------------------------------------ */
/* System */

#define IS_System(o)              (kObject_typeId(o) == KType_System)

struct kSystemVar {
	kObjectHeader h;
};

/* ------------------------------------------------------------------------ */
/* T0 */

/* ------------------------------------------------------------------------ */
/* macros */

#define KStackCheckOverflow(SFP) do {\
		if(unlikely(kctx->esp > kctx->stack->stack_uplimit)) {\
			KLIB KRuntime_raise(kctx, KException_("StackOverflow"), SoftwareFault, NULL, SFP);\
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

// if you want to ignore (exception), use KRuntime_tryCallMethod
#define KStackCall(SFP) { \
		SFP[K_SHIFTIDX].previousStack = kctx->stack->topStack;\
		kctx->stack->topStack = SFP;\
		(SFP[K_MTDIDX].calledMethod)->invokeKMethodFunc(kctx, SFP);\
		kctx->stack->topStack = SFP[K_SHIFTIDX].previousStack;\
	} \

#define KStackCallAgain(SFP, MTD) { \
		SFP[K_MTDIDX].calledMethod = MTD;\
		(MTD)->invokeKMethodFunc(kctx, SFP);\
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
	kbool_t (*PackupNameSpace) (KonohaContext *kctx, kNameSpace *, int, KTraceInfo *);
	kbool_t (*ExportNameSpace) (KonohaContext *kctx, kNameSpace *, kNameSpace *, int, KTraceInfo *);
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

	void                (*KDict_Init)(KonohaContext *, KDict *);
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

	KonohaContextVar*   (*KonohaContext_Init)(KonohaContext *, const PlatformApi *api);
	void                (*KonohaContext_Free)(KonohaContext *, KonohaContextVar *ctx);
	void                (*ReftraceAll)(KonohaContext *kctx, KObjectVisitor *);

	KonohaContext*      (*KonohaFactory_CreateKonoha)(KonohaFactory *factory);
	int                 (*Konoha_Destroy)(KonohaContext *kctx);
	kbool_t             (*Konoha_LoadScript)(KonohaContext*, const char *);
	kbool_t             (*Konoha_Eval)(KonohaContext*, const char *, kfileline_t);

	KClass*             (*Kclass)(KonohaContext*, ktypeattr_t, KTraceInfo *);
	kString*            (*KClass_shortName)(KonohaContext*, KClass *ct);
	KClass*             (*KClass_define)(KonohaContext*, kpackageId_t, kString *, KDEFINE_CLASS *, KTraceInfo *);
	KClass*             (*KClass_Generics)(KonohaContext*, KClass *, ktypeattr_t rty, kushort_t psize, kparamtype_t *p);
	kbool_t             (*KClass_isSubtype)(KonohaContext*, KClass *, KClass *);
	kbool_t             (*KClass_AddField)(KonohaContext*, KClass *, ktypeattr_t, ksymbol_t);

	kObject*            (*new_kObject)(KonohaContext*, kArray *gcstack, KClass *, uintptr_t);
	kObject*            (*Knull)(KonohaContext*, KClass *);
	void                (*kObjectProto_Free)(KonohaContext *kctx, kObjectVar *);
	void                (*kObjectProto_Reftrace)(KonohaContext *kctx, kObject *, KObjectVisitor *);
	KKeyValue*          (*kObjectProto_GetKeyValue)(KonohaContext*, kAbstractObject *, ksymbol_t);
	kObject*            (*kObject_getObject)(KonohaContext*, kAbstractObject *, ksymbol_t, kAbstractObject *);
	void                (*kObjectProto_SetObject)(KonohaContext*, kAbstractObject *, ksymbol_t, ktypeattr_t, kAbstractObject *);
	void                (*kObjectProto_SetUnboxValue)(KonohaContext*, kAbstractObject *, ksymbol_t, ktypeattr_t, uintptr_t);
	void                (*kObjectProto_RemoveKey)(KonohaContext*, kAbstractObject *, ksymbol_t);
	void                (*kObjectProto_DoEach)(KonohaContext*, kAbstractObject *, void *thunk, void (*f)(KonohaContext*, void *, KKeyValue *d));
	int                 (*kObjectProto_format)(KonohaContext *, KonohaStack *, int, KBuffer *, int count);
	void                (*kObject_WriteToBuffer)(KonohaContext *, kObject *, int, KBuffer *, KonohaValue *, int);

	kString*            (*new_kString)(KonohaContext*, kArray *gcstack, const char *, size_t, int);

	void                (*kArray_Add)(KonohaContext*, kArray *, kAbstractObject *);
	void                (*kArray_Insert)(KonohaContext*, kArray *, size_t, kAbstractObject *);
	void                (*kArray_Clear)(KonohaContext*, kArray *, size_t);

	kparamId_t          (*Kparamdom)(KonohaContext*, kushort_t, const kparamtype_t *);
	kMethodVar*         (*new_kMethod)(KonohaContext*, kArray *gcstack, uintptr_t, ktypeattr_t, kmethodn_t, KMethodFunc);
	kParam*             (*kMethod_SetParam)(KonohaContext*, kMethod *, ktypeattr_t, kushort_t, const kparamtype_t *);
	void                (*kMethod_SetFunc)(KonohaContext*, kMethod*, KMethodFunc);
	kbool_t             (*kMethod_GenCode)(KonohaContext*, kMethod*, kNode *, int options);
	intptr_t            (*kMethod_indexOfField)(kMethod *);

	kbool_t             (*KRuntime_SetModule)(KonohaContext*, int, struct KRuntimeModule *, KTraceInfo *);

	void                (*kNameSpace_FreeSugarExtension)(KonohaContext *, kNameSpaceVar *);

	KPackage*           (*kNameSpace_RequirePackage)(KonohaContext*, const char *, KTraceInfo *);
	kbool_t             (*kNameSpace_ImportPackage)(KonohaContext*, kNameSpace*, const char *, KTraceInfo *);
	kbool_t             (*kNameSpace_LoadScript)(KonohaContext*, kNameSpace*, const char *, KTraceInfo *);

	KClass*             (*kNameSpace_GetClassByFullName)(KonohaContext*, kNameSpace *, const char *, size_t, KClass *);
	KClass*             (*kNameSpace_DefineClass)(KonohaContext*, kNameSpace *, kString *, KDEFINE_CLASS *, KTraceInfo *);

	kbool_t             (*kNameSpace_SetConstData)(KonohaContext *, kNameSpace *, ksymbol_t, ktypeattr_t, uintptr_t, KTraceInfo *);
	kbool_t             (*kNameSpace_LoadConstData)(KonohaContext*, kNameSpace *, const char **d, KTraceInfo *);
	KKeyValue*          (*kNameSpace_GetConstNULL)(KonohaContext *, kNameSpace *, ksymbol_t, int isLocalOnly);
	void                (*kNameSpace_LoadMethodData)(KonohaContext*, kNameSpace *, intptr_t *, KTraceInfo *);

	kMethod*            (*kNameSpace_GetGetterMethodNULL)(KonohaContext*, kNameSpace *, KClass*, ksymbol_t mn);
	kMethod*            (*kNameSpace_GetSetterMethodNULL)(KonohaContext*, kNameSpace *, KClass*, ksymbol_t mn, ktypeattr_t);
	kMethod*            (*kNameSpace_GetCoercionMethodNULL)(KonohaContext*, kNameSpace *, KClass *, KClass *);
	kMethod*            (*kNameSpace_GetMethodByParamSizeNULL)(KonohaContext*, kNameSpace *, KClass *, kmethodn_t mn, int paramsize, KMethodMatchOption option);
	kMethod*            (*kNameSpace_GetMethodBySignatureNULL)(KonohaContext*, kNameSpace *, KClass *, kmethodn_t mn, int paramdom, int paramsize, kparamtype_t *);
	kMethod*            (*kMethod_DoLazyCompilation)(KonohaContext *kctx, kMethod *mtd, kparamtype_t *, int options);

	// runtime support
	void                (*CheckSafePoint)(KonohaContext *kctx, KonohaStack *sfp, kfileline_t uline);
	kbool_t             (*KRuntime_tryCallMethod)(KonohaContext *, KonohaStack *);
	uintptr_t           (*ApplySystemFunc)(KonohaContext *, uintptr_t defval, const char *name, const char *param, ...);

	void                (*KRuntime_raise)(KonohaContext*, int symbol, int fault, kString *Nullable, KonohaStack *);
	void                (*ReportScriptMessage)(KonohaContext *, KTraceInfo *, kinfotag_t, const char *fmt, ...);
	int                 (*DiagnosisFaultType)(KonohaContext *kctx, int fault, KTraceInfo *);
	void                (*DumpObject)(KonohaContext *, kObject *, const char *, const char *, int);
};

#define K_NULL            (kctx->share->constNull)
#define K_TRUE            (kctx->share->constTrue)
#define K_FALSE           (kctx->share->constFalse)
#define K_NULLPARAM       (kctx->share->paramList->ParamItems[0])
#define K_EMPTYARRAY      (kctx->share->emptyArray)
#define TS_EMPTY          (kctx->share->emptyString)

#define UPCAST(o)         ((kObject *)o)

#define KMalloc(size, TRACE)           PLATAPI GCModule.Kmalloc(kctx, size, TRACE)
#define KCalloc(size, item, TRACE)     PLATAPI GCModule.Kzmalloc(kctx, ((size) * (item)), TRACE)
#define KMalloc_UNTRACE(size)          PLATAPI GCModule.Kmalloc(kctx, size, NULL)
#define KCalloc_UNTRACE(size, item)    PLATAPI GCModule.Kzmalloc(kctx, ((size) * (item)), NULL)
#define KFree(p, size)                 PLATAPI GCModule.Kfree(kctx, p, size)

#define KBuffer_bytesize(W)                 (((W)->m)->bytesize - (W)->pos)

#define kclass(CID, UL)           KLIB Kclass(kctx, CID, UL)

#define FILEID_(T)                KLIB KfileId(kctx, T, sizeof(T)-1, StringPolicy_TEXT|StringPolicy_ASCII, _NEWID)

#define PN_konoha                 0
#define PackageId_sugar           1
#define PN_(T)                    KLIB KpackageId(kctx, T, sizeof(T)-1, StringPolicy_TEXT|StringPolicy_ASCII, _NEWID)

#define KAsciiSymbol(T, L, DEF)            KLIB Ksymbol(kctx, T, L, StringPolicy_ASCII, DEF)
#define ksymbolSPOL(T, L, SPOL, DEF)       KLIB Ksymbol(kctx, T, L, SPOL, DEF)
#define KSymbol_(T)                        KLIB Ksymbol(kctx, T, (sizeof(T)-1), StringPolicy_TEXT|StringPolicy_ASCII, _NEWID)
#define KException_(T)                     KLIB Ksymbol(kctx, (T "Exception"), (sizeof(T "Exception")-1), StringPolicy_TEXT|StringPolicy_ASCII, _NEWID)
#define KFieldName_(T)                     KLIB Ksymbol(kctx, T, (sizeof(T)-1), StringPolicy_TEXT|StringPolicy_ASCII, _NEWID)
#define KMethodName_(T)                    KLIB Ksymbol(kctx, T, (sizeof(T)-1), StringPolicy_TEXT|StringPolicy_ASCII, _NEWID)

#define MN_new                             KSymbol_new  /* @see KSymbol_return + 1*/

#define new_(C, A, STACK)                 (k##C *)(KLIB new_kObject(kctx, STACK, KClass_##C, ((uintptr_t)A)))
#define GcUnsafe                          NULL
#define OnStack                           NULL
#define OnField                           NULL
#define OnVirtualField                    NULL
#define OnGlobalConstList                 (kctx->share->GlobalConstList)
#define OnContextConstList                (kctx->stack->ContextConstList)
#define OnGcStack                         (kctx->stack->gcStack)

#define KNULL(C)                  (k##C *)KLIB Knull(kctx, KClass_##C)

#define kArray_size(A)            (((A)->bytesize)/sizeof(void *))
#define kArray_SetSize(A, N)      ((kArrayVar *)A)->bytesize = ((N) * sizeof(void *))
#define new_kParam(CTX, R, PSIZE, P)       (KLIB kMethod_SetParam(CTX, NULL, R, PSIZE, P))

#define KRequirePackage(NAME, TRACE)       KLIB kNameSpace_RequirePackage(kctx, NAME, TRACE)
#define KImportPackage(NS, NAME, TRACE)    KLIB kNameSpace_ImportPackage(kctx, NS, NAME, TRACE)

typedef intptr_t  KDEFINE_METHOD;

#define KConst_(D)  ((const char **)D)

#define KDefineConstInt(T) #T, KType_Int, T

typedef struct {
	const char *key;
	uintptr_t ty;
	kint_t value;
} KDEFINE_INT_CONST;

typedef struct {
	const char *key;
	uintptr_t ty;
	const char *value;
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
		((KClassVar *)C)->T = F;\
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

#define INIT_GCSTACK()         kArray* _GcStack = kctx->stack->gcStack; size_t _gcstackpos = kArray_size(_GcStack)
#define RESET_GCSTACK()        KLIB kArray_Clear(kctx, _GcStack, _gcstackpos)
#define PUSH_GCSTACK2(o)       KLIB kArray_Add(kctx, kctx->stack->gcStack, o)

#define KRefIncObject(T, O)
#define KRefDecObject(T, O)

#define GC_WRITE_BARRIER(kctx, PARENT, VAR, VAL)\
	(PLATAPI GCModule.UpdateObjectField((struct kObjectVar *)(PARENT), (struct kObjectVar *)(VAR), ((struct kObjectVar *)(VAL))))

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

#define KTODO(A)          KExit(EXIT_FAILURE);

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
#define DBG_P(fmt, ...)     PLATAPI ConsoleModule.ReportDebugMessage(__FILE__, __FUNCTION__, __LINE__, fmt, ## __VA_ARGS__)
#define DBG_ABORT(fmt, ...) PLATAPI ConsoleModule.ReportDebugMessage(__FILE__, __FUNCTION__, __LINE__, fmt, ## __VA_ARGS__); DBG_ASSERT(kctx == NULL)
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
extern kbool_t Konoha_Run(KonohaContext* konoha);  // TODO

extern KonohaContext* KonohaFactory_CreateKonoha(KonohaFactory *factory);
extern int Konoha_Destroy(KonohaContext *kctx);

extern kbool_t KonohaFactory_LoadPlatformModule(KonohaFactory *factory, const char *name, ModuleType option);
extern void KonohaFactory_SetDefaultFactory(KonohaFactory *factory, void (*SetPlatformApi)(KonohaFactory *), int argc, char **argv);

#ifdef __cplusplus
} /* extern "C" */
#endif

#include "klib.h"

#endif /* MINIOKNOHA_H_ */
