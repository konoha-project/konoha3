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

#ifndef KONOHA3_H_
#define KONOHA3_H_

#include "konoha3/konoha_init.h"

/* config */

#define K_CLASSTABLE_INITSIZE 64
#define K_PAGESIZE        4096

#define K_VERSION   "3.0"
#define K_MAJOR_VERSION 3
#define K_MINOR_VERSION 0
#define K_PATCH_LEVEL   0

#include "konoha3/stardate.h"
#define K_DATE      ((K_YEAR - 2006) * (12 * 24 * 32) + (K_MONTH * (24 * 32) + ((K_DAY * 24) + K_HOUR)))

#ifndef K_REVISION
#define K_REVISION 0
#endif

#ifndef K_PROGNAME
#define K_PROGNAME  "Konoha alpha"
/*2012/01/24 - */
#define K_CODENAME "Rome"  // eternal city
#else
#define K_CODENAME "based on Konoha-" K_VERSION
#endif

#define USE_UTF8 1

/* ------------------------------------------------------------------------ */
/* datatype */

typedef bool             kbool_t;

typedef intptr_t         kint_t;
typedef uintptr_t        kuint_t;

#if defined(__LP64__) || defined(_WIN64) /* In case of 64 bit system */
//#define USE_SYS64    1
typedef int32_t           khalfword_t;
typedef uint32_t          kuhalfword_t;

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

#else /* In case of 32 bit system */
//#define USE_SYS32    1
typedef int16_t           khalfword_t;
typedef uint16_t          kuhalfword_t;
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

typedef uint32_t              kid32_t;
#define KID32_FLAGTOP(N)     ((kid32_t)1<<(32-(N)))

/**
 * MSB
 * 10    Symbol
 * 11    MethodName
 * 00    TypeId
 * 01    ToType(MethodName)
 *
 * 1110*  GetSymbol
 * 1101*  SetSymbol
 * 1010*  @Symbol
 * 1001*  $Symbol
 * 1011*  %Symbol
 * 1***1    Canonical cannonical
 *
 * 00**** TypeId
 * 001***    Boxed
 * 00*1**    ReadOnly
 * 00**1*    LocalOnly?
 * 00***1    Coercion?
 */

typedef kuhalfword_t     khalfflag_t;    /* flag field */

#define KFLAG_H(N)               ((sizeof(khalfflag_t)*8)-N)
#define KFLAG_H0                 ((khalfflag_t)(1 << KFLAG_H(1)))
#define KFLAG_H1                 ((khalfflag_t)(1 << KFLAG_H(2)))
#define KFLAG_H2                 ((khalfflag_t)(1 << KFLAG_H(3)))
#define KFLAG_H3                 ((khalfflag_t)(1 << KFLAG_H(4)))
#define KFLAG_H4                 ((khalfflag_t)(1 << KFLAG_H(5)))
#define KFLAG_H5                 ((khalfflag_t)(1 << KFLAG_H(6)))
#define KFLAG_H6                 ((khalfflag_t)(1 << KFLAG_H(7)))
#define KFLAG_H7                 ((khalfflag_t)(1 << KFLAG_H(8)))

#define KFlag_Is(T,f,op)          (((T)(f) & (T)(op)) == (T)(op))
#define KFlag_Set1(T,f,op)        f = (((T)(f)) | ((T)(op)))
#define KFlag_Set0(T,f,op)        f = (((T)(f)) & (~((T)(op))))
#define KFlag_Set(T,f,op,b)       if(b) {KFlag_Set1(T,f,op);} else {KFlag_Set0(T,f,op);}

#define KHalfFlag_Set(f,op)            KFlag_Set1(khalfflag_t,f,op)
#define KHalfFlag_Unset(f,op)          KFlag_Set0(khalfflag_t,f,op)
#define KHalfFlag_Is(f,op)             KFlag_Is(khalfflag_t,f,op)

/* Konoha Interanal Ids */

/* halfsize id */

typedef kuhalfword_t       kpackageId_t;     /* package id e.g. Konoha.Math */
typedef kuhalfword_t       ksymbol_t;        /* Package package PACKAGE */
typedef kuhalfword_t       kmethodn_t;       /* Package package PACKAGE */
typedef kuhalfword_t       ktypeattr_t;      /* TypeId + Attribute */
typedef kuhalfword_t       kparamId_t;


/* fullsize id */
typedef uintptr_t          kfileline_t;     /* fileid (half) + linenum (half) */
#define NOPLINE            0

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

/* symbol_t 00010000000 */

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

#define KSymbol_END                ((ksymbol_t)-1)
#define KSymbol_IndentPattern      (((ksymbol_t)1)|KSymbolAttr_Pattern) /*$Indent*/
#define KSymbol_SymbolPattern      (((ksymbol_t)2)|KSymbolAttr_Pattern) /*$Symbol*/
#define KSymbol_TextPattern        (((ksymbol_t)3)|KSymbolAttr_Pattern) /*$Text*/
#define KSymbol_NumberPattern      (((ksymbol_t)4)|KSymbolAttr_Pattern) /*$Number*/
#define KSymbol_MemberPattern      (((ksymbol_t)5)|KSymbolAttr_Pattern) /*$Member*/
#define KSymbol_TypePattern        (((ksymbol_t)6)|KSymbolAttr_Pattern) /*$Type*/

#define KSymbol_ParenthesisGroup   (((ksymbol_t)7)) /*()*/
#define KSymbol_BracketGroup       (((ksymbol_t)8)) /*[]*/
#define KSymbol_BraceGroup         (((ksymbol_t)9)) /*{}*/
#define KSymbol_TypeCastGroup      (((ksymbol_t)7)|KSymbolAttr_Pattern)    /*$()*/
#define KSymbol_TypeParamGroup     (((ksymbol_t)8)|KSymbolAttr_Pattern)    /*$[]*/
#define KSymbol_OptionalGroup      (((ksymbol_t)8)|KSymbol_ATMARK)         /*@[]*/
#define KSymbol_ExprPattern        (((ksymbol_t)10)|KSymbolAttr_Pattern)    /*$Block*/
#define KSymbol_BlockPattern       (((ksymbol_t)11)|KSymbolAttr_Pattern)    /*$Block*/
#define KSymbol_ParamPattern       (((ksymbol_t)12)|KSymbolAttr_Pattern)   /*$Param*/
#define KSymbol_TypeDeclPattern    (((ksymbol_t)13)|KSymbolAttr_Pattern)   /*$TypeDecl*/
#define KSymbol_MethodDeclPattern  (((ksymbol_t)14)|KSymbolAttr_Pattern) /*$MethodDecl*/
#define KSymbol_TokenPattern       (((ksymbol_t)15)|KSymbolAttr_Pattern)   /*$Token*/

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
#define ksetjmp  _setjmp
#else
#define ksetjmp  setjmp
#endif
#define klongjmp longjmp
#endif /*jmpbuf_i*/

#if defined(__MINGW32__) || defined(_MSC_VER)
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
	typedef struct k##T##Var                k##T##Var;\

DefineBasicTypeList(TypeDefMacro)

#define TypeEnumMacro(T)                    KType_##T,

typedef enum {
	DefineBasicTypeList(TypeEnumMacro)
	KType_ERROR = -1 /*  sentinel */
} KType_;

#define kAbstractObject                 const void

/* sugar.h */
typedef const struct kTokenVar   kToken;
typedef struct kTokenVar         kTokenVar;
typedef struct kUntypedNode      kUntypedNode;

#define kTokenNULL kToken
#define kUntypedNodeNULL  kUntypedNode
#define kNameSpaceNULL kNameSpace

#define KMETHOD    void  /*CC_FASTCALL_*/
typedef KMETHOD   (*KMethodFunc)(KonohaContext*, struct KonohaValueVar *);

/* ------------ */
/* Module       */

typedef struct KModuleInfo {
	const char *name;
	const char *version;
	int patchlevel;
	const char *desc;
} KModuleInfo;

struct ExecutionEngineModule {
	const KModuleInfo            *ExecutionEngineInfo;
	const char                   *ArchType;
	void                        (*DeleteExecutionEngine)(KonohaContext *kctx);
	const struct KBuilderAPI   *(*GetDefaultBuilderAPI)(void);
	struct KVirtualCode        *(*GetDefaultBootCode)(void);
	struct KVirtualCode        *(*GenerateVirtualCode)(KonohaContext *, kMethod *mtd, kUntypedNode *block, int option);
	KMethodFunc                 (*GenerateMethodFunc)(KonohaContext *, struct KVirtualCode *);
	void                        (*SetMethodCode)(KonohaContext *, kMethodVar *mtd, struct KVirtualCode *, KMethodFunc func);
	struct KVirtualCode        *(*RunExecutionEngine)(KonohaContext *kctx, struct KonohaValueVar *sfp, struct KVirtualCode *pc);
};

struct KonohaFactory {
	// settings
	const char  *name;
	size_t       stacksize;
	volatile int safePointFlag;
	int          verbose;
	int          verbose_debug;
	int          verbose_sugar;
	int          verbose_code;
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
	uint64_t (*getTimeMilliSecond)(void);

	/* message */
	int    (*printf_i)(const char *fmt, ...) __PRINTFMT(2, 3);
	int    (*vprintf_i)(const char *fmt, va_list args);
	int    (*snprintf_i)(char *str, size_t size, const char *fmt, ...) __PRINTFMT(3, 4);
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
	ktypeattr_t   typeAttr;
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

typedef struct KRuntimeModel        KRuntimeModel;
typedef struct KModelContext        KModelContext;

struct KObjectVisitor;

struct KonohaContextVar {
	uintptr_t                  safepoint; // set to 1
	KonohaStack               *esp;
	PlatformApi               *platApi;
	KonohaLib                 *klib;
	KRuntime                  *share;
	KRuntimeContextVar        *stack;
	KRuntimeModel           **runtimeModels;
	KModelContext           **localContexts;
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

#define kContext_Debug          ((khalfflag_t)(1<<0))
#define kContext_Interactive    ((khalfflag_t)(1<<1))
#define kContext_CompileOnly    ((khalfflag_t)(1<<2))
#define kContext_Test           ((khalfflag_t)(1<<3))
#define kContext_Trace          ((khalfflag_t)(1<<4))

#define KonohaContext_Is(P, X)   (KFlag_Is(khalfflag_t,(X)->stack->flag, kContext_##P))
#define KonohaContext_Set(P, X)   KFlag_Set1(khalfflag_t, (X)->stack->flag, kContext_##P)

struct KRuntimeContextVar {
	KonohaStack               *stack;
	size_t                     stacksize;
	KonohaStack               *stack_uplimit;
	kArray                    *ContextConstList;
	kArray                    *gcStack; /* ContextConstList */
	KGrowingArray              cwb;
	// local info
	khalfflag_t               flag;
	KonohaContext             *rootctx;
	void                      *cstack_bottom;  // for GC
	// Eval
	ktypeattr_t                evalty;
	kuhalfword_t                  evalidx;
	// Exception
	kException                *ThrownException;
	jmpbuf_i                  *evaljmpbuf;
	KonohaStack               *bottomStack;
	KonohaStack               *topStack;
};

// model
#define KRuntimeModel_MAXSIZE    16
#define ParserModelIndex     0
#define CommonModelIndex     1

#define MOD_APACHE     10
#define MOD_EVENT      11

struct KRuntimeModel {
	const char  *name;
	int          modelIndex;
	void        (*setupModelContext)(KonohaContext*, struct KRuntimeModel *, int newctx);
	void        (*freeModel)(KonohaContext*, struct KRuntimeModel *);
	size_t       allocSize;
	kmutex_t    *modelMutex;
};

struct KModelContext {
	uintptr_t unique;
	void (*reftrace)(KonohaContext*, struct KModelContext *, struct KObjectVisitor *);
	void (*free)(KonohaContext*, struct KModelContext *);
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
	kInt        *asInt;\
	kString     *asString;\
	kArray      *asArray;\
	kMethod     *asMethod;\
	kFunc       *asFunc; \
	kNameSpace  *asNameSpace;\
	struct kSyntaxVar     *asSyntax;\
	kToken      *asToken;\
	kUntypedNode       *asNode;\
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
	const char     *structname;
	ktypeattr_t     typeId;         khalfflag_t    cflag;
	ktypeattr_t     baseTypeId;     ktypeattr_t     superTypeId;
	ktypeattr_t     rtype;          kuhalfword_t       cparamsize;
	struct kparamtype_t   *cParamItems;
	size_t         cstruct_size;
	KClassField   *fieldItems;
	kuhalfword_t      fieldsize;       kuhalfword_t fieldAllocSize;
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
	ktypeattr_t      typeId;       khalfflag_t    cflag;
	ktypeattr_t      baseTypeId;   ktypeattr_t     superTypeId;
	ktypeattr_t      p0;           kparamId_t      cparamdom;
	kmagicflag_t magicflag;
	size_t     cstruct_size;
	KClassField         *fieldItems;
	kuhalfword_t  fieldsize;     kuhalfword_t fieldAllocSize;
	const char               *DBG_NAME;
	ksymbol_t   classNameSymbol;  kuhalfword_t   optvalue;
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
	ktypeattr_t     typeAttr;
	ksymbol_t       name;
};

/* ----------------------------------------------------------------------- */

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

#define KClassFlag_TypeVar          ((khalfflag_t)(1<<0))
#define KClassFlag_UnboxType        ((khalfflag_t)(1<<1))
#define KClassFlag_Singleton        ((khalfflag_t)(1<<2))
#define KClassFlag_Immutable        ((khalfflag_t)(1<<3))
#define KClassFlag_Private          ((khalfflag_t)(1<<4))
#define KClassFlag_Nullable         ((khalfflag_t)(1<<5))
#define KClassFlag_Virtual          ((khalfflag_t)(1<<6))
#define KClassFlag_Newable          ((khalfflag_t)(1<<7))
#define KClassFlag_Final            ((khalfflag_t)(1<<8))
#define KClassFlag_Interface        ((khalfflag_t)(1<<9))
#define KClassFlag_Prototype        ((khalfflag_t)(1<<10))

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
#define KClass_Is(P, C)           (KFlag_Is(khalfflag_t, (C)->cflag, KClassFlag_##P))
#define KClass_Set(P, C, B)       KFlag_Set(khalfflag_t, (C)->cflag, KClassFlag_##P, B)

#define KType_Is(P, T)            (KFlag_Is(khalfflag_t, (KClass_(T))->cflag, KClassFlag_##P))

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

#define kObject_HashCode(O)          (uintptr_t)((O)->h.magicflag >> (sizeof(kuhalfword_t)*8))
#define kObject_flags(O)             ((kuhalfword_t)((O)->h.magicflag))
#define kObject_SetHashCode(O, HASH) ((kObjectVar *)O)->h.magicflag = (((uintptr_t)HASH) << (sizeof(kuhalfword_t)*8) | kObject_flags(O))

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

#define kArray_size(A)            (((A)->bytesize)/sizeof(void *))
#define kArray_SetSize(A, N)      ((kArrayVar *)A)->bytesize = ((N) * sizeof(void *))

struct kArrayVar {
	kObjectHeader h;
	size_t bytesize;
	union {
		uintptr_t              *unboxItems;
		kint_t                 *kintItems;
		kfloat_t               *kfloatItems;
		kObject        **ObjectItems;
		kString        **stringItems;
		kParam         **ParamItems;
		kMethod        **MethodItems;
		kFunc          **FuncItems;
		kNameSpace     **NameSpaceItems;
		struct kSyntaxVar **SyntaxItems;
		kToken         **TokenItems;
		kTokenVar      **TokenVarItems;
		kUntypedNode   **NodeItems;
	};
	size_t bytemax;
};

/* ------------------------------------------------------------------------ */
/* Param */

#define IS_Param(o)              (kObject_baseTypeId(o) == KType_Param)

typedef struct kparamtype_t {
	ktypeattr_t    typeAttr;
	ksymbol_t  name;
} kparamtype_t;

struct kParamVar {
	kObjectHeader h;
	ktypeattr_t rtype; kuhalfword_t psize;
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
	khalfword_t        delta;        kpackageId_t packageId;
	kToken         *SourceToken;
	union {
		kNameSpace   *LazyCompileNameSpace;       // lazy compilation
		kUntypedNode        *CompiledNode;
	};
	uintptr_t         serialNumber;
};


typedef struct KMethodMatch {
	kNameSpace   *ns;
	ksymbol_t     mn;
	kuhalfword_t     paramsize;
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
	ksymbol_t symbol;  kuhalfword_t fault;
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
	kpackageId_t packageId;            khalfflag_t syntaxOption;
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
#define kNameSpace_Is(P, ns)                         (KFlag_Is(khalfflag_t, (ns)->syntaxOption, kNameSpace_##P))
#define kNameSpace_Set(P, ns, B)                     KFlag_Set(khalfflag_t, ((kNameSpaceVar *)ns)->syntaxOption, kNameSpace_##P, B)

#define kNameSpace_Override                          ((khalfflag_t)(1<<1))
#define kNameSpace_Ambigious                         ((khalfflag_t)(1<<2))

#define kNameSpace_NoSemiColon                       ((khalfflag_t)(1<<3))
#define kNameSpace_TypeInference                     ((khalfflag_t)(1<<4))
#define kNameSpace_ImplicitField                     ((khalfflag_t)(1<<5))
#define kNameSpace_ImplicitGlobalVariable            ((khalfflag_t)(1<<6))
#define kNameSpace_ImplicitCoercion                  ((khalfflag_t)(1<<7))

#define kNameSpace_StaticError                       ((khalfflag_t)(1<<14))
#define KPushNameSpaceOption(ns)  khalfflag_t _syntaxOption = ns->syntaxOption
#define KPopNameSpaceOption(ns)   ns->syntaxOption = _syntaxOption


/* ------------------------------------------------------------------------ */
/* System */

#define IS_System(o)              (kObject_typeId(o) == KType_System)

struct kSystemVar {
	kObjectHeader h;
};

/* ------------------------------------------------------------------------ */
/* sugar.h */

// reserved
//#define MN_new     (8+KSymbol_void)
#define FN_this      KFieldName_("this")

/* KonohaChar */

typedef enum {
	KonohaChar_Null                 =  0,
	KonohaChar_Undefined            =  1,
	KonohaChar_Digit                =  2,
	KonohaChar_UpperCaseAlphabet    =  3,
	KonohaChar_LowerCaseAlphabet    =  4,
	KonohaChar_Unicode              =  5,
	KonohaChar_NewLine              =  6,
	KonohaChar_Tab                  =  7,
	KonohaChar_Space                =  8,
	KonohaChar_OpenParenthesis      =  9,
	KonohaChar_CloseParenthesis     = 10,
	KonohaChar_OpenBracket          = 11,
	KonohaChar_CloseBracket         = 12,
	KonohaChar_OpenBrace            = 13,
	KonohaChar_CloseBrace           = 14,
	KonohaChar_LessThan             = 15,
	KonohaChar_GreaterThan          = 16,
	KonohaChar_Quote                = 17,
	KonohaChar_DoubleQuote          = 18,
	KonohaChar_BackQuote            = 19,
	KonohaChar_Surprised            = 20,
	KonohaChar_Sharp                = 21,
	KonohaChar_Dollar               = 22,
	KonohaChar_Percent              = 23,
	KonohaChar_And                  = 24,
	KonohaChar_Star                 = 25,
	KonohaChar_Plus                 = 26,
	KonohaChar_Comma                = 27,
	KonohaChar_Minus                = 28,
	KonohaChar_Dot                  = 29,
	KonohaChar_Slash                = 30,
	KonohaChar_Colon                = 31,
	KonohaChar_SemiColon            = 32,
	KonohaChar_Equal                = 33,
	KonohaChar_Question             = 34,
	KonohaChar_AtMark               = 35,
	KonohaChar_Var                  = 36,
	KonohaChar_Childer              = 37,
	KonohaChar_BackSlash            = 38,
	KonohaChar_Hat                  = 39,
	KonohaChar_UnderBar             = 40,
	KonohaChar_MAX                  = 41
} KonohaChar;

#ifdef USE_AsciiToKonohaChar
static const char cMatrix[128] = {
	0/*nul*/, 1/*soh*/, 1/*stx*/, 1/*etx*/, 1/*eot*/, 1/*enq*/, 1/*ack*/, 1/*bel*/,
	1/*bs*/,  KonohaChar_Tab/*ht*/, KonohaChar_NewLine/*nl*/, 1/*vt*/, 1/*np*/, 1/*cr*/, 1/*so*/, 1/*si*/,
	/*020 dle  021 dc1  022 dc2  023 dc3  024 dc4  025 nak  026 syn  027 etb */
	1, 1, 1, 1,     1, 1, 1, 1,
	/*030 can  031 em   032 sub  033 esc  034 fs   035 gs   036 rs   037 us */
	1, 1, 1, 1,     1, 1, 1, 1,
	/*040 sp   041  !   042  "   043  #   044  $   045  %   046  &   047  ' */
	KonohaChar_Space, KonohaChar_Surprised, KonohaChar_DoubleQuote, KonohaChar_Sharp, KonohaChar_Dollar, KonohaChar_Percent, KonohaChar_And, KonohaChar_Quote,
	/*050  (   051  )   052  *   053  +   054  ,   055  -   056  .   057  / */
	KonohaChar_OpenParenthesis, KonohaChar_CloseParenthesis, KonohaChar_Star, KonohaChar_Plus, KonohaChar_Comma, KonohaChar_Minus, KonohaChar_Dot, KonohaChar_Slash,
	/*060  0   061  1   062  2   063  3   064  4   065  5   066  6   067  7 */
	KonohaChar_Digit, KonohaChar_Digit, KonohaChar_Digit, KonohaChar_Digit,  KonohaChar_Digit, KonohaChar_Digit, KonohaChar_Digit, KonohaChar_Digit,
	/*070  8   071  9   072  :   073  ;   074  <   075  =   076  >   077  ? */
	KonohaChar_Digit, KonohaChar_Digit, KonohaChar_Colon, KonohaChar_SemiColon, KonohaChar_LessThan, KonohaChar_Equal, KonohaChar_GreaterThan, KonohaChar_Question,
	/*100  @   101  A   102  B   103  C   104  D   105  E   106  F   107  G */
	KonohaChar_AtMark, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet,
	/*110  H   111  I   112  J   113  K   114  L   115  M   116  N   117  O */
	KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet,
	/*120  P   121  Q   122  R   123  S   124  T   125  U   126  V   127  W */
	KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet,
	/*130  X   131  Y   132  Z   133  [   134  \   135  ]   136  ^   137  _ */
	KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_UpperCaseAlphabet, KonohaChar_OpenBracket, KonohaChar_BackSlash, KonohaChar_CloseBracket, KonohaChar_Hat, KonohaChar_UnderBar,
	/*140  `   141  a   142  b   143  c   144  d   145  e   146  f   147  g */
	KonohaChar_BackQuote, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet,
	/*150  h   151  i   152  j   153  k   154  l   155  m   156  n   157  o */
	KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet,
	/*160  p   161  q   162  r   163  s   164  t   165  u   166  v   167  w */
	KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet,
	/*170  x   171  y   172  z   173  {   174  |   175  }   176  ~   177 del*/
	KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_LowerCaseAlphabet, KonohaChar_OpenBrace, KonohaChar_Var, KonohaChar_CloseBrace, KonohaChar_Childer, 1,
};

static int AsciiToKonohaChar(int ascii)
{
	return (ascii < 0) ? KonohaChar_Unicode : cMatrix[ascii];
}
#endif/*USE_AsciiToKonohaChar*/

#define KCHAR_MAX  KonohaChar_MAX
#define SIZEOF_TOKENMATRIX   (sizeof(void *) * KCHAR_MAX * 2)
typedef struct Tokenizer Tokenizer;
typedef int (*TokenizeFunc)(KonohaContext *, kTokenVar *, Tokenizer *, int);

struct Tokenizer {
	kNameSpace         *ns;
	const char         *source;
	size_t              sourceLength;
	kfileline_t         currentLine;
	kArray             *tokenList;
	int                 tabsize;
	int                 baseIndent;
	int                 currentIndent;
	const TokenizeFunc *cFuncItems;
	union {
		kFunc         **FuncItems;
		kArray        **funcListItems;
	};
	kString            *preparedString;
};

//#define VAR_TRACE
#ifndef VAR_TRACE
#define VAR_TRACE DBG_P("tracing..")
#endif

// int TokenFunc(Token tk, Source s)
#define VAR_TokenFunc(TK, S)\
		kTokenVar *TK = (kTokenVar *)sfp[1].asObject;\
		kString *S = sfp[2].asString;\
		Tokenizer *tokenizer = (Tokenizer *)sfp[1].unboxValue;\
		int tok_start = (ksymbol_t)sfp[2].intValue;\
		VAR_TRACE; (void)TK; (void)S; (void)tok_start; (void)tokenizer;

// int ReplaceFunc(kNameSpace *ns, Token[] tokenList, int s, int op, int e,Token[] bufferList)
#define VAR_ReplaceFunc(NS, TLS, S, OP, E, BUF)\
		kNameSpace *NS = sfp[1].asNameSpace;\
		kArray *TLS = (kArray *)sfp[2].asObject;\
		int S = (int)sfp[3].intValue;\
		int OP = (int)sfp[4].intValue;\
		int E = (int)sfp[5].intValue;\
		kArray *BUF = (kArray *)sfp[6]].asObject;\
		VAR_TRACE; (void)NS; (void)TLS; (void)S; (void)OP; (void)E; (void)BUF;


// int Parse(Node stmt, Symbol name, Token[] toks, int s, int op, int e)
#define VAR_Parse(STMT, NAME, TLS, S, OP, E)\
		kUntypedNode *STMT = (kUntypedNode *)sfp[1].asObject;\
		ksymbol_t NAME = (ksymbol_t)sfp[2].intValue;\
		kArray *TLS = (kArray *)sfp[3].asObject;\
		int S = (int)sfp[4].intValue;\
		int OP = (int)sfp[5].intValue;\
		int E = (int)sfp[6].intValue;\
		VAR_TRACE; (void)STMT; (void)NAME; (void)TLS; (void)S; (void)OP; (void)E

#define VAR_PatternMatch(STMT, NAME, TLS, S, E)\
		kUntypedNode *STMT = (kUntypedNode *)sfp[1].asObject;\
		ksymbol_t NAME = (ksymbol_t)sfp[2].intValue;\
		kArray *TLS = (kArray *)sfp[3].asObject;\
		int S = (int)sfp[4].intValue;\
		int E = (int)sfp[6].intValue;\
		VAR_TRACE; (void)STMT; (void)NAME; (void)TLS; (void)S; (void)E

#define VAR_Expression(STMT, TLS, S, C, E)\
		kSyntax *syn = (kSyntax *)sfp[0].unboxValue;\
		kUntypedNode *STMT = (kUntypedNode *)sfp[1].asObject;\
		kArray *TLS = (kArray *)sfp[3].asObject;\
		int S = (int)sfp[4].intValue;\
		int C = (int)sfp[5].intValue;\
		int E = (int)sfp[6].intValue;\
		VAR_TRACE; (void)syn; (void)STMT; (void)TLS; (void)S; (void)C; (void)E

// Node TypeCheck(Node expr, Gamma ns, Object type)
#define VAR_TypeCheck(EXPR, GMA, TY) \
		kUntypedNode *EXPR = (kUntypedNode *)sfp[1].asObject;\
		kNameSpace *GMA = kUntypedNode_ns(EXPR);\
		KClass* TY = kObject_class(sfp[3].asObject);\
		VAR_TRACE; (void)EXPR; (void)GMA; (void)TY

#define VAR_TypeCheck2(STMT, EXPR, GMA, TY) \
		kUntypedNode *EXPR = (kUntypedNode *)sfp[1].asObject;\
		kNameSpace *GMA = kUntypedNode_ns(EXPR);\
		KClass* TY = kObject_class(sfp[3].asObject);\
		kUntypedNode *STMT = expr;\
		VAR_TRACE; (void)STMT; (void)EXPR; (void)GMA; (void)TY

typedef const struct kSyntaxVar   kSyntax;
typedef struct kSyntaxVar         kSyntaxVar;

typedef enum {
	KSugarTokenFunc         = 0,
	KSugarParseFunc         = 1,
	KSugarTypeFunc          = 2,
	SugarFunc_SIZE          = 3
} SugerFunc;

#define SUGARFUNC   (kFunc *)

#define SYNFLAG_Macro               ((khalfflag_t)1)
#define SYNFLAG_MetaPattern         ((khalfflag_t)1 << 1)
#define SYNFLAG_NodeLeftJoinOp2     ((khalfflag_t)1 << 2)
#define SYNFLAG_Suffix              ((khalfflag_t)1 << 3)
#define SYNFLAG_TypeSuffix          ((khalfflag_t)1 << 4)

#define SYNFLAG_NodeBreakExec       ((khalfflag_t)1 << 6)  /* return, throw */
#define SYNFLAG_NodeJumpAhead0      ((khalfflag_t)1 << 7)  /* continue */
#define SYNFLAG_NodeJumpSkip0       ((khalfflag_t)1 << 8)  /* break */

#define SYNFLAG_CFunc               (SYNFLAG_CParseFunc|SYNFLAG_CTypeFunc|SYNFLAG_CTokenFunc)
#define SYNFLAG_CParseFunc          ((khalfflag_t)1 << 10)
#define SYNFLAG_CTypeFunc           ((khalfflag_t)1 << 11)
#define SYNFLAG_CTokenFunc          ((khalfflag_t)1 << 12)

#define SYNFLAG_CallNode            ((khalfflag_t)1 << 13)

#define kSyntax_Is(P, o)       (KFlag_Is(khalfflag_t,(o)->flag, SYNFLAG_##P))
#define kSyntax_Set(P,o,B)     KFlag_Set(khalfflag_t,(o)->flag, SYNFLAG_##P, B)

struct kSyntaxVar {
	kObjectHeader h;
	kNameSpace                       *packageNameSpace;
	ksymbol_t  keyword;               khalfflag_t  flag;
	kArray                           *syntaxPatternListNULL;
	kArray                           *macroDataNULL;
	kFunc                            *TokenFuncNULL;
	kFunc                            *ParseFuncNULL;
	kFunc                            *TypeFuncNULL;
	kFunc                            *ReplaceFuncNULL;
	khalfword_t tokenKonohaChar;         khalfword_t macroParamSize;
	khalfword_t precedence_op2;          khalfword_t precedence_op1;
};

// operator prcedence
// http://ja.cppreference.com/w/cpp/language/operator_precedence

typedef enum {
	Precedence_CPPStyleScope  =  50,
	Precedence_CStyleSuffixCall     = 100,  /*x(), x[], x.x x->x x++ */
	Precedence_CStylePrefixOperator = 200,  /*++x, --x, sizeof x &x +x -x !x (T)x  */
//	Precedence_CppMember      = 300,  /* .x ->x */
	Precedence_CStyleMUL      = 400,  /* x * x, x / x, x % x*/
	Precedence_CStyleADD      = 500,  /* x + x, x - x */
	Precedence_CStyleSHIFT    = 600,  /* x << x, x >> x */
	Precedence_CStyleCOMPARE  = 700,
	Precedence_CStyleEquals   = 800,
	Precedence_CStyleBITAND   = 900,
	Precedence_CStyleBITXOR   = 1000,
	Precedence_CStyleBITOR    = 1100,
	Precedence_CStyleAND      = 1200,
	Precedence_CStyleOR       = 1300,
	Precedence_CStyleTRINARY  = 1400,  /* ? : */
	Precedence_CStyleAssign   = 1500,
	Precedence_CStyleCOMMA    = 1600,
	Precedence_Statement      = 1900,
	Precedence_CStyleStatementEnd    = 2000
} Precedence;

typedef struct KDEFINE_SYNTAX {
	ksymbol_t    keyword;
	khalfflag_t flag;
	int precedence_op2;
	int precedence_op1;
	union {
		kFunc* parseFunc;
		KMethodFunc parseMethodFunc;
	};
	union {
		kFunc* typeFunc;
		KMethodFunc typeMethodFunc;
	};
	int tokenChar;
	union {
		kFunc* tokenFunc;
		KMethodFunc tokenMethodFunc;
	};
} KDEFINE_SYNTAX;

#define KSugarFunc(ns, F)     new_(Func, KLIB new_kMethod(kctx, (ns)->NameSpaceConstList, 0, 0, 0, F), (ns)->NameSpaceConstList)

/* Token */

struct kTokenVar {
	kObjectHeader h;
	kfileline_t     uline;
	union {
		kString *text;
		kArray  *GroupTokenList;
		kUntypedNode   *parsedNode;
	};
	union {
		ksymbol_t   tokenType;           // (resolvedSyntaxInfo == NULL)
//		ksymbol_t   symbol;      // symbol (resolvedSyntaxInfo != NULL)
	};
	union {
		kuhalfword_t   indent;               // indent when kw == TokenType_Indent
		kuhalfword_t   openCloseChar;
	};
	ksymbol_t   symbol;
	union {
		ktypeattr_t resolvedTypeId;      // typeid if KSymbol_TypePattern
		ksymbol_t   ruleNameSymbol;      // pattern rule
	};
	kSyntax   *resolvedSyntaxInfo;
};

#define kToken_SetOpenCloseChar(tk, ch, ch2)           tk->openCloseChar = ((ch << 8) | ((char)ch2))
#define kToken_GetOpenChar(tk)                         ((int)(tk->openCloseChar >> 8))
#define kToken_GetCloseChar(tk)                        ((char)tk->openCloseChar)

typedef enum {
	TokenType_Skip      = 0,
	TokenType_Indent    = KSymbol_IndentPattern,
	TokenType_Symbol    = KSymbol_SymbolPattern,
	TokenType_Text      = KSymbol_TextPattern,
	TokenType_Number    = KSymbol_NumberPattern,
	TokenType_Member    = KSymbol_MemberPattern,
	TokenType_LazyBlock = KSymbol_BraceGroup,
	TokenType_Error     = KSymbol_TokenPattern
} kTokenType;

#define kToken_IsIndent(T)  ((T)->tokenType == TokenType_Indent)
#define kToken_IsStatementSeparator(T)  ((T)->resolvedSyntaxInfo->precedence_op2 == Precedence_CStyleStatementEnd)

#define kTokenFlag_BeforeWhiteSpace      kObjectFlag_Local1
#define kTokenFlag_MatchPreviousPattern  kObjectFlag_Local2
#define kTokenFlag_RequiredReformat      kObjectFlag_Local2
#define kTokenFlag_OpenGroup             kObjectFlag_Local3/*reserved*/
#define kTokenFlag_CloseGroup            kObjectFlag_Local4/*reserved*/
//#define kTokenFlag_StatementSeparator    kObjectFlag_Local4/*obsolete*/

#define kToken_Is(P, o)      (KFlag_Is(uintptr_t,(o)->h.magicflag, kTokenFlag_##P))
#define kToken_Set(P,o,B)    KFlag_Set(uintptr_t,(o)->h.magicflag, kTokenFlag_##P, B)

typedef struct KMacroSet {
	ksymbol_t  symbol;
	kArray    *tokenList;
	int        beginIdx;
	int        endIdx;
} KMacroSet;

struct KTokenSeqSource {
	kToken *openToken;
	int     stopChar;
	kToken *foundErrorToken;
};

struct KTokenSeqTarget {
	int RemovingIndent;
	int ExpandingBraceGroup;
	kSyntax *syntaxSymbolPattern;
};

typedef struct KTokenSeq {
	kNameSpace *ns;
	kArray     *tokenList;
	int         beginIdx;
	int         endIdx;
	union {
		struct KTokenSeqSource SourceConfig;
		struct KTokenSeqTarget TargetPolicy;
	};
} KTokenSeq;

#define KTokenSeq_Push(kctx, tokens) \
	size_t _PopCheckIdx = kArray_size(tokens.tokenList);\
	tokens.beginIdx      = kArray_size(tokens.tokenList);\
	tokens.endIdx        = 0;\

#define KTokenSeq_Pop(kctx, tokens)   do {\
	KLIB kArray_Clear(kctx, tokens.tokenList, _PopCheckIdx);\
	DBG_ASSERT(_PopCheckIdx == kArray_size(tokens.tokenList));\
} while(0)

#define KTokenSeq_End(kctx, T)   T.endIdx = kArray_size(T.tokenList)

#define RangeGroup(A)                 A, 1, (kArray_size(A)-1)
#define RangeArray(A)                 A, 0, kArray_size(A)
#define RangeTokenSeq(T)              T.tokenList, T.beginIdx, T.endIdx

#define Token_isVirtualTypeLiteral(TK)     ((TK)->resolvedSyntaxInfo->keyword == KSymbol_TypePattern)
#define Token_typeLiteral(TK)              (TK)->resolvedTypeId

typedef khalfword_t       knode_t;

#define kUntypedNode_node(o)             ((o)->nodeType)
#define kUntypedNode_setnode(o, node)    ((kUntypedNode *)(o))->nodeType = (node)
#define kUntypedNode_IsConstValue(o)     (KNode_Const <= kUntypedNode_node(o) && kUntypedNode_node(o) <= KNode_UnboxConst)
#define kUntypedNode_IsValue(o)          (KNode_Const <= kUntypedNode_node(o) && kUntypedNode_node(o) <= KNode_Field)

#include "konoha3/node2.h"

#define kUntypedNode_uline(O)   (O)->KeyOperatorToken->uline

#define KNewNode(ns)     new_(UntypedNode, ns, OnGcStack)

#define kUntypedNode_IsRootNode(O)       IS_NameSpace(O->RootNodeNameSpace)
#define kUntypedNode_ns(O)               kUntypedNode_GetNameSpace(kctx, O)
static inline kNameSpace *kUntypedNode_GetNameSpace(KonohaContext *kctx, kUntypedNode *node)
{
	kNameSpace *ns = node->StmtNameSpace;
	while(!IS_NameSpace(ns)) {
		if(kUntypedNode_IsRootNode(node)) {
			ns = node->RootNodeNameSpace;
			break;
		}
		node = node->Parent;
		ns = node->StmtNameSpace;
	}
	return ns;
}

#define kUntypedNode_GetParent(kctx, node)  ((IS_Node(node->Parent)) ? node->Parent : K_NULLNODE)
#define kUntypedNode_GetParentNULL(stmt)    ((IS_Node(stmt->Parent)) ? stmt->Parent : NULL)
#define kUntypedNode_SetParent(kctx, node, parent)   KFieldSet(node, node->Parent, parent)


static inline kUntypedNode *kUntypedNode_Type(kUntypedNode *node, knode_t nodeType, ktypeattr_t typeAttr)
{
	if(kUntypedNode_node(node) != KNode_Error) {
		kUntypedNode_setnode(node, nodeType);
		node->typeAttr = typeAttr;
	}
	return node;
}

static inline size_t kUntypedNode_GetNodeListSize(KonohaContext *kctx, kUntypedNode *node)
{
	return (IS_Array(node->NodeList)) ? kArray_size(node->NodeList) : 0;
}

#define kUntypedNode_IsTerm(N)           IS_Token((N)->TermToken)

#define kUntypedNodeFlag_ObjectConst        kObjectFlag_Local1

#define kUntypedNodeFlag_OpenBlock          kObjectFlag_Local2  /* KNode_Block */
#define kUntypedNodeFlag_CatchContinue      kObjectFlag_Local3  /* KNode_Block */
#define kUntypedNodeFlag_CatchBreak         kObjectFlag_Local4  /* KNode_Block */

#define kUntypedNode_Is(P, O)       (KFlag_Is(uintptr_t,(O)->h.magicflag, kUntypedNodeFlag_##P))
#define kUntypedNode_Set(P, O, B)   KFlag_Set(uintptr_t,(O)->h.magicflag, kUntypedNodeFlag_##P, B)

#define kUntypedNode_At(E, N)            ((E)->NodeList->NodeItems[(N)])
#define kUntypedNode_IsError(STMT)         (kUntypedNode_node(STMT) == KNode_Error)

#define kUntypedNode_GetObjectNULL(CTX, O, K)            (KLIB kObject_getObject(CTX, UPCAST(O), K, NULL))
#define kUntypedNode_GetObject(CTX, O, K, DEF)           (KLIB kObject_getObject(CTX, UPCAST(O), K, DEF))
#define kUntypedNode_SetObject(CTX, O, K, V)             KLIB kObjectProto_SetObject(CTX, UPCAST(O), K, kObject_typeId(V), UPCAST(V))
#define kUntypedNode_SetUnboxValue(CTX, O, K, T, V)      KLIB kObjectProto_SetUnboxValue(CTX, UPCAST(O), K, T, V)
#define kUntypedNode_RemoveKey(CTX, O, K)                KLIB kObjectProto_RemoveKey(CTX, UPCAST(O), K)
#define kUntypedNode_DoEach(CTX, O, THUNK, F)            kObjectProto_DoEach(CTX, UPCAST(O), THUNK, F)

#define kUntypedNode_Message(kctx, STMT, PE, FMT, ...)            KLIB MessageNode(kctx, STMT, NULL, NULL, PE, FMT, ## __VA_ARGS__)
#define kUntypedNodeToken_Message(kctx, STMT, TK, PE, FMT, ...)   KLIB MessageNode(kctx, STMT, TK, NULL, PE, FMT, ## __VA_ARGS__)


typedef struct {
	ktypeattr_t    typeAttr;    ksymbol_t  name;
} KGammaStackDecl;

#define kNameSpace_TopLevel              (khalfflag_t)(1)
#define kNameSpace_IsTopLevel(GMA)       KFlag_Is(khalfflag_t, GMA->genv->flag, kNameSpace_TopLevel)

struct KGammaStack {
	KGammaStackDecl *varItems;
	size_t varsize;
	size_t capacity;
	size_t allocsize;  // set size if not allocated  (by default on stack)
} ;

struct KGammaLocalData {
	khalfflag_t  flag;   khalfflag_t cflag;
	KClass   *thisClass;
	kMethod  *currentWorkingMethod;
	struct KGammaStack    localScope;
} ;

/* ------------------------------------------------------------------------ */

#define KGetParserContext(kctx)    ((KParserContext *)kctx->localContexts[ParserModelIndex])
#define KPARSERM            ((KParserModel *)kctx->runtimeModels[ParserModelIndex])
#define KClass_Symbol       KPARSERM->cSymbol
#define KClass_SymbolVar    KPARSERM->cSymbol
#define KClass_Syntax       KPARSERM->cSyntax
#define KClass_SyntaxVar    KPARSERM->cSyntax
#define KClass_Token        KPARSERM->cToken
#define KClass_TokenVar     KPARSERM->cToken
#define KClass_Gamma        KPARSERM->cGamma
#define KClass_GammaVar     KPARSERM->cGamma


#define KClass_TokenArray       KPARSERM->cTokenArray
#define kTokenArray             kArray
#define KClass_NodeArray        KClass_Array
#define kUntypedNodeArray              kArray
#define KClass_NodeArray        KClass_Array
#define kUntypedNodeArray              kArray

#define IS_Syntax(O) (kObject_class(O) == KClass_Syntax)
#define IS_Token(O)  (kObject_class(O) == KClass_Token)
#define IS_Node(O)   (kObject_class(O) == KClass_UntypedNode)
#define IS_Gamma(O)  (kObject_class(O) == KClass_Gamma)

#define K_NULLTOKEN  ((kToken *)(KClass_Token)->defaultNullValue)
#define K_NULLNODE   (kUntypedNode *)((KClass_UntypedNode)->defaultNullValue)
#define K_NULLBLOCK  (kUntypedNode *)((KClass_UntypedNode)->defaultNullValue)

typedef kUntypedNode* (*KTypeDeclFunc)(KonohaContext *kctx, kUntypedNode *stmt, kNameSpace *ns, ktypeattr_t ty, kUntypedNode *termNode, kUntypedNode *vexpr, kObject *thunk);

typedef enum {
	ParseExpressionOption = 0,
	ParseMetaPatternOption = 1,
	OnlyPatternMatch = 1 << 2,
	ParseBlockOption = 1 << 3
} ParseOption;

struct KBuilder;

typedef kbool_t (*IsSeparatorFunc)(KonohaContext *kctx, kToken *tk);

typedef struct KParserModel {
	KRuntimeModel  h;
	KClass *cSymbol;
	KClass *cSyntax;
	KClass *cToken;
	KClass *cNode;
	KClass *cTokenArray;
	KClass *cUntypedNode;
#define DEFINE_NODE_CLASS(T)  KClass *c##T##Node;
	NODE_LIST_OP(DEFINE_NODE_CLASS);
#undef DEFINE_NODE_CLASS
	//
	kFunc  *termParseFunc;
	kFunc  *opParseFunc;
	kFunc  *patternParseFunc;
	kFunc  *methodTypeFunc;
	//
} KParserModel;

typedef struct {
	KModelContext     h;
	kArray            *preparedTokenList;
	KGrowingArray      errorMessageBuffer;
	kArray            *errorMessageList;
	int                errorMessageCount;
	kbool_t            isBlockedErrorMessage;
	kArray            *definedMethodList;
} KParserContext;

#define KClass_INFER    KClass_(KType_var)

typedef enum {
	TypeCheckPolicy_NoPolicy       = 0,
	TypeCheckPolicy_NoCheck        = (1 << 0),
	TypeCheckPolicy_AllowVoid      = (1 << 1),
	TypeCheckPolicy_Coercion       = (1 << 2),
	TypeCheckPolicy_AllowEmpty     = (1 << 3),
	TypeCheckPolicy_CONST          = (1 << 4),  /* Reserved */
	TypeCheckPolicy_Creation       = (1 << 6)   /* TypeCheckNodeByName */
} TypeCheckPolicy;

#define KPushMethodCall(gma)   KLIB AddLocalVariable(kctx, ns, KType_var, 0)

#define new_ConstNode(CTX, NS, T, O)              KLIB kUntypedNode_SetConst(CTX, KNewNode(NS), T, O)
#define new_UnboxConstNode(CTX, NS, T, D)         KLIB kUntypedNode_SetUnboxConst(CTX, KNewNode(NS), T, D)
#define new_VariableNode(CTX, NS, BLD, TY, IDX)   KLIB kUntypedNode_SetVariable(CTX, KNewNode(NS), BLD, TY, IDX)

#define SUGAR                                   ((const KParserModel *)KPARSERM)->
#define KType_Syntax                            SUGAR cSyntax->typeId
#define KType_Symbol                            SUGAR cSymbol->typeId
#define KType_Token                             SUGAR cToken->typeId
#define KType_Node                              SUGAR cNode->typeId
#define KType_TokenArray                        SUGAR cTokenArray->typeId

//#define KSymbol_(T)                               _e->keyword(kctx, T, sizeof(T)-1, KSymbol_Noname)
#define kSyntax_(NS, KW)                        KLIB kNameSpace_GetSyntax(kctx, NS, KW)
//#define NEWkSyntax_(KS, KW)                     (kSyntaxVar *)(KLIB kNameSpace_GetSyntax(kctx, KS, KW, 1))

#ifdef USE_SMALLBUILD
#define KDump(O)
#define KdumpToken(ctx, tk)
#define KdumpTokenArray(CTX, TLS, S, E)
#define KdumpKTokenSeq(CTX, MSG, R)
#define KdumpNode(CTX, EXPR)
#else
#define KDump(O)                         KLIB DumpObject(kctx, (kObject *)O, __FILE__, __FUNCTION__, __LINE__)
#define KdumpToken(ctx, tk)              KLIB dumpToken(ctx, tk, 0)
#define KdumpTokenArray(CTX, TLS, S, E)  DBG_P("@"); KLIB dumpTokenArray(CTX, 1, TLS, S, E)
#define KdumpKTokenSeq(CTX, MSG, R)     DBG_P(MSG); KLIB dumpTokenArray(CTX, 1, R->tokenList, R->beginIdx, R->endIdx)
#define KdumpNode(CTX, EXPR)             KLIB dumpNode(CTX, 0, 0, EXPR)
#endif

/* ------------------------------------------------------------------------ */
/* BuilderAPI */

struct KBuilder;
typedef struct KBuilder KBuilder;

typedef kbool_t (*KNodeVisitFunc)(KonohaContext *kctx, KBuilder *builder, kUntypedNode *stmt, void *thunk);

struct KBuilderCommon {
	const struct KBuilderAPI *api;
	int option;
	kfileline_t uline;
};

#define DefineVisitFunc(NAME) KNodeVisitFunc visit##NAME##Node;

struct KBuilderAPI {
	const char *target;
	const struct ExecutionEngineModule *ExecutionEngineModule;
	NODE_LIST_OP(DefineVisitFunc)
};

/* ------------------------------------------------------------------------ */
#ifdef __cplusplus
#define __CONST_CAST__(T, expr) (const_cast<T>(expr))
#else
#define __CONST_CAST__(T, expr) ((T)(expr))
#endif

#define kUntypedNode_isSymbolTerm(expr)   1



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

#define KStackSetLine(SFP, UL)    KStackSetUnboxValue(SFP[K_RTNIDX].calledFileLine, UL)
#define KStackSetArgc(SFP, ARGC)  ((KonohaContextVar *)kctx)->esp = (SFP + ARGC + 1)

#define KStackSetMethodAll(SFP, DEFVAL, UL, MTD, ARGC) { \
		KStackSetObjectValue(SFP[K_RTNIDX].asObject, ((kObject *)DEFVAL));\
		KStackSetLine(SFP, UL);\
		KStackSetArgc(SFP, ARGC);\
		KStackSetUnboxValue(SFP[K_MTDIDX].calledMethod, MTD); \
	} \


#define KStackSetFunc(SFP, FO) do {\
	KStackSetUnboxValue(SFP[K_MTDIDX].calledMethod, (FO)->method);\
}while(0);\

#define KStackSetFuncAll(SFP, DEFVAL, UL, FO, ARGC) { \
		KStackSetObjectValue(SFP[K_RTNIDX].asObject, ((kObject *)DEFVAL));\
		KStackSetLine(SFP, UL);\
		KStackSetArgc(SFP, ARGC);\
		KStackSetFunc(SFP, FO);\
	} \

// if you want to ignore (exception), use KRuntime_tryCallMethod
#define KStackCall(SFP) { \
		KStackSetUnboxValue(SFP[K_SHIFTIDX].previousStack, kctx->stack->topStack);\
		kctx->stack->topStack = SFP;\
		(SFP[K_MTDIDX].calledMethod)->invokeKMethodFunc(kctx, SFP);\
		kctx->stack->topStack = SFP[K_SHIFTIDX].previousStack;\
	} \

#define KStackCallAgain(SFP, MTD) { \
		KStackSetUnboxValue(SFP[K_MTDIDX].calledMethod, MTD);\
		(MTD)->invokeKMethodFunc(kctx, SFP);\
	} \


/* ----------------------------------------------------------------------- */
/* Package */

#if defined(_MSC_VER)
#define KPACKNAME(N, V) \
	K_DATE, N, V, 0, 0
#else
#define KPACKNAME(N, V) \
	.name = N, .version = V, .konoha_Checksum = K_DATE
#endif

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

#include "konoha3/obsolete.h"

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
	void                (*KBuffer_printf)(KonohaContext*, KBuffer *, const char *fmt, ...) __PRINTFMT(3, 4);
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
	KClass*             (*KClass_Generics)(KonohaContext*, KClass *, ktypeattr_t rty, kuhalfword_t psize, kparamtype_t *p);
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

	kparamId_t          (*Kparamdom)(KonohaContext*, kuhalfword_t, const kparamtype_t *);
	kMethodVar*         (*new_kMethod)(KonohaContext*, kArray *gcstack, uintptr_t, ktypeattr_t, kmethodn_t, KMethodFunc);
	kParam*             (*kMethod_SetParam)(KonohaContext*, kMethod *, ktypeattr_t, kuhalfword_t, const kparamtype_t *);
	void                (*kMethod_SetFunc)(KonohaContext*, kMethod*, KMethodFunc);
	kbool_t             (*kMethod_GenCode)(KonohaContext*, kMethod*, kUntypedNode *, int options);
	intptr_t            (*kMethod_indexOfField)(kMethod *);

	kbool_t             (*KRuntime_SetModule)(KonohaContext*, int, struct KRuntimeModel *, KTraceInfo *);

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
	void                (*ReportScriptMessage)(KonohaContext *, KTraceInfo *, kinfotag_t, const char *fmt, ...)  __PRINTFMT(4, 5);
	int                 (*DiagnosisFaultType)(KonohaContext *kctx, int fault, KTraceInfo *);
	void                (*DumpObject)(KonohaContext *, kObject *, const char *, const char *, int);


	/* Parser Func */
	kSyntax*      (*kNameSpace_GetSyntax)(KonohaContext *, kNameSpace *, ksymbol_t);
	void          (*kNameSpace_DefineSyntax)(KonohaContext *, kNameSpace *, KDEFINE_SYNTAX *, KTraceInfo *);
	void          (*kNameSpace_AddSyntax)(KonohaContext *, kNameSpace *, kSyntax *, KTraceInfo *);
	void          (*kNameSpace_UseDefaultVirtualMachine)(KonohaContext *, kNameSpace *);
	void          (*kSyntax_AddPattern)(KonohaContext *, kSyntax *, const char *rule, kfileline_t uline, KTraceInfo *);
	kbool_t       (*SetMacroData)(KonohaContext *, kNameSpace *, ksymbol_t, int, const char *, int optionMacro);

	void         (*Tokenize)(KonohaContext *, kNameSpace *, const char *, kfileline_t, int baseIndent, kArray *bufferList);
	void         (*ApplyMacroData)(KonohaContext *, kNameSpace *, kArray *, int, int, size_t, KMacroSet *, kArray *bufferList);
	void         (*Preprocess)(KonohaContext *, kNameSpace *, kArray *, int, int, KMacroSet *, kArray *bufferList);
	kstatus_t    (*EvalTokenList)(KonohaContext *, KTokenSeq *, KTraceInfo *);

	int          (*ParseTypePattern)(KonohaContext *, kNameSpace *, kArray *, int , int , KClass **classRef);
	kTokenVar*   (*kToken_ToBraceGroup)(KonohaContext *, kTokenVar *, kNameSpace *, KMacroSet *);

	void         (*kUntypedNode_AddParsedObject)(KonohaContext *, kUntypedNode *, ksymbol_t, kObject *o);
	int          (*FindEndOfStatement)(KonohaContext *, kNameSpace *, kArray *, int, int);

	uintptr_t    (*kUntypedNode_ParseFlag)(KonohaContext *kctx, kUntypedNode *stmt, KFlagSymbolData *flagData, uintptr_t flag);
	kToken*      (*kUntypedNode_GetToken)(KonohaContext *, kUntypedNode *, ksymbol_t kw, kToken *def);
	kUntypedNode*       (*kUntypedNode_GetNode)(KonohaContext *, kUntypedNode *, ksymbol_t kw, kUntypedNode *def);
	kUntypedNode*       (*kUntypedNode_AddNode)(KonohaContext *, kUntypedNode *, kUntypedNode *);
	void         (*kUntypedNode_InsertAfter)(KonohaContext *, kUntypedNode *, kUntypedNode *target, kUntypedNode *);

//	kUntypedNode*       (*kUntypedNode_Termnize)(KonohaContext *, kUntypedNode *, kToken *);
	kUntypedNode*       (*kUntypedNode_Op)(KonohaContext *kctx, kUntypedNode *, kToken *keyToken, int n, ...);
//	kUntypedNode*    (*new_UntypedOperatorNode)(KonohaContext *, kSyntax *syn, int n, ...);
	int          (*ParseSyntaxNode)(KonohaContext *, kSyntax *, kUntypedNode *, ksymbol_t, kArray *, int beginIdx, int opIdx, int endIdx);

	int          (*ParseNode)(KonohaContext *, kUntypedNode *, kArray *, int beginIdx, int endIdx, ParseOption, const char *requiredTokenText);
	kUntypedNode*       (*ParseNewNode)(KonohaContext *, kNameSpace *, kArray *tokenList, int* s, int e, ParseOption, const char *requiredTokenText);
	kUntypedNode*       (*AppendParsedNode)(KonohaContext *, kUntypedNode *, kArray *tokenList, int, int, IsSeparatorFunc, ParseOption, const char *requiredTokenText);

	kUntypedNode*       (*kUntypedNode_SetConst)(KonohaContext *, kUntypedNode *, KClass *, kObject *);
	kUntypedNode*       (*kUntypedNode_SetUnboxConst)(KonohaContext *, kUntypedNode *, ktypeattr_t, uintptr_t);
	kUntypedNode*       (*kUntypedNode_SetVariable)(KonohaContext *, kUntypedNode *, knode_t build, ktypeattr_t, intptr_t index);

	kUntypedNode*       (*new_MethodNode)(KonohaContext *, kNameSpace *, KClass *, kMethod *mtd, int n, ...);

	kUntypedNode*      (*TypeCheckNodeByName)(KonohaContext *, kUntypedNode*, ksymbol_t, kNameSpace *, KClass *, int);
	kUntypedNode*      (*TypeCheckNodeAt)(KonohaContext *, kUntypedNode *, size_t, kNameSpace *, KClass *, int);
	kUntypedNode *     (*TypeCheckMethodParam)(KonohaContext *, kMethod *mtd, kUntypedNode *, kNameSpace *, KClass *);
	int         (*AddLocalVariable)(KonohaContext *, kNameSpace *, ktypeattr_t, ksymbol_t);
	void        (*kUntypedNode_DeclType)(KonohaContext *, kUntypedNode *, kNameSpace *, ktypeattr_t, kUntypedNode *, kObject *, KTypeDeclFunc);
	kUntypedNode*      (*TypeVariableNULL)(KonohaContext *, kUntypedNode *, kNameSpace *, KClass *);

	void       (*kToken_ToError)(KonohaContext *, kTokenVar *, kinfotag_t, const char *fmt, ...);
	kUntypedNode *    (*MessageNode)(KonohaContext *, kUntypedNode *, kToken *, kNameSpace *, kinfotag_t, const char *fmt, ...);

	kbool_t    (*VisitNode)(KonohaContext *, struct KBuilder *, kUntypedNode *node, void *thunk);

	void (*dumpToken)(KonohaContext *kctx, kToken *tk, int n);
	void (*dumpTokenArray)(KonohaContext *kctx, int nest, kArray *a, int s, int e);
	void (*dumpNode)(KonohaContext *kctx, kUntypedNode *stmt);





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

#define KStackSetObjectValue(VAR, VAL)  KUnsafeFieldSet(VAR, VAL)
#define KStackSetUnboxValue(VAR, VAL)   (VAR) = (VAL)

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
	KStackSetObjectValue(sfp[K_RTNIDX].asObject, ((kObject *)VAL));\
	CLEANUP;\
	KCheckSafePoint(kctx, sfp);\
	return; \
} while(0)

#define KReturnDefaultValue() do {\
	KStackSetUnboxValue(sfp[K_RTNIDX].intValue, 0);\
	return; \
} while(0)

#define KReturnField(vv) do {\
	KStackSetObjectValue(sfp[(-(K_CALLDELTA))].asObject, ((kObject *)vv));\
	return; \
} while(0)

#define KReturn(vv) do {\
	KStackSetObjectValue(sfp[(-(K_CALLDELTA))].asObject, ((kObject *)vv));\
	KCheckSafePoint(kctx, sfp);\
	return; \
} while(0)

#define KReturnInt(d) do {\
	KStackSetUnboxValue(sfp[K_RTNIDX].intValue, d); \
	return; \
} while(0)

#define KReturnUnboxValue(d) do {\
	KStackSetUnboxValue(sfp[K_RTNIDX].unboxValue, d); \
	return; \
} while(0)

#define KReturnFloatValue(c) do {\
	KStackSetUnboxValue(sfp[(-(K_CALLDELTA))].floatValue, c); \
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
#define DBG_P(fmt, ...)     PLATAPI ConsoleModule.ReportDebugMessage(__FILE__, __FUNCTION__, __LINE__, fmt, ## __VA_ARGS__)
#define DBG_ABORT(fmt, ...) PLATAPI ConsoleModule.ReportDebugMessage(__FILE__, __FUNCTION__, __LINE__, fmt, ## __VA_ARGS__); DBG_ASSERT(kctx == NULL)
#define DUMP_P(fmt, ...)    PLATAPI printf_i(fmt, ## __VA_ARGS__)
#else
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

#else /* !defined(__GNUC__) */

#ifndef unlikely
#define unlikely(x)   (x)
#endif

#ifndef likely
#define likely(x)     (x)
#endif
#endif

#if !defined(_MSC_VER)
#define KONOHA_EXPORT(TYPE) extern TYPE
#define KONOHA_IMPORT(TYPE) extern TYPE
#else
#define KONOHA_EXPORT(TYPE) __declspec(dllexport) extern TYPE
#define KONOHA_IMPORT(TYPE) __declspec(dllimport) extern TYPE
#endif

///* Konoha API */
KONOHA_EXPORT(kbool_t) Konoha_LoadScript(KonohaContext* konoha, const char *scriptfile);
KONOHA_EXPORT(kbool_t) Konoha_Eval(KonohaContext* konoha, const char *script, kfileline_t uline);
KONOHA_EXPORT(kbool_t) Konoha_Run(KonohaContext* konoha);  // TODO

KONOHA_EXPORT(KonohaContext *) KonohaFactory_CreateKonoha(KonohaFactory *factory);
KONOHA_EXPORT(int) Konoha_Destroy(KonohaContext *kctx);

KONOHA_EXPORT(kbool_t) KonohaFactory_LoadPlatformModule(KonohaFactory *factory, const char *name, ModuleType option);
KONOHA_EXPORT(void) KonohaFactory_SetDefaultFactory(KonohaFactory *factory, void (*SetPlatformApi)(KonohaFactory *), int argc, char **argv);

#include "konoha3/klib.h"

static inline void kToken_SetTypeId(KonohaContext *kctx, kToken *tk, kNameSpace *ns, ktypeattr_t type)
{
	kTokenVar *token = __CONST_CAST__(kTokenVar *, tk);
	token->resolvedTypeId = type;
	token->resolvedSyntaxInfo = KLIB kNameSpace_GetSyntax(kctx, ns, KSymbol_TypePattern);
}

#endif /* KONOHA3_H_ */
