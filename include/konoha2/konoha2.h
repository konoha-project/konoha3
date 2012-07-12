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

#ifndef KONOHA2_H_
#define KONOHA2_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define K_CLASSTABLE_INIT 64
#define K_PAGESIZE        4096

#ifndef K_OSDLLEXT
#if defined(__APPLE__)
#define K_OSDLLEXT        ".dylib"
#elif defined(__linux__)
#define K_OSDLLEXT        ".so"
#endif
#endif

/* - 2012/06/14 */
//#define K_CODENAME "Miyajima"
/*2012/06/14 -  */

#define K_CODENAME "The Summer Palace, Beijing"
#ifndef K_REVISION
#define K_REVISION  1
#endif

#define K_USING_UTF8 1

#define USE_BUILTINTEST  1

#ifndef PLATFORM_KERNEL
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#else
#include "konoha_lkm.h"
#endif /* PLATFORM_KERNEL */

#ifndef jmpbuf_i
#include <setjmp.h>
#define jmpbuf_i jmp_buf
#define ksetjmp  setjmp
#define klongjmp longjmp
#endif /*jmpbuf_i*/

#include <stddef.h>
#include <stdarg.h>

#if defined(K_USING_SIGNAL)
#include <signal.h>
#endif

#if defined(K_USING_PTHREAD)
#include <pthread.h>
#endif

/* ------------------------------------------------------------------------ */
/* platform */

#ifdef __GCC__
#define __PRINT_FMT(idx1, idx2) __attribute__((format(printf, idx1, idx2)))
#else
#define __PRINT_FMT(idx1, idx2)
#endif

#define PLAT (_ctx->plat)->

typedef enum {
	CritTag, ErrTag, WarnTag, NoticeTag, InfoTag, DebugTag, NoneTag
} kinfotag_t;

#define CRIT_ CritTag
#define ERR_  ErrTag
#define WARN_ WarnTag
#define INFO_ InfoTag
#define DEBUG_ DebugTag
#define PRINT_ NoneTag

typedef void FILE_i;

typedef struct kplatform_t {
	// settings
	const char *name;
	size_t stacksize;
	// low-level functions
	void* (*malloc_i)(size_t);
	void  (*free_i)(void *);
	int   (*setjmp_i)(jmpbuf_i);
	void   (*longjmp_i)(jmpbuf_i, int);

	char* (*realpath_i)(const char*, char*);
	FILE_i* (*fopen_i)(const char*, const char*);
	int     (*fgetc_i)(FILE_i *);
	int     (*feof_i)(FILE_i *);
	int     (*fclose_i)(FILE_i *);
	//
	void  (*syslog_i)(int priority, const char *message, ...) __PRINT_FMT(2, 3);
	void  (*vsyslog_i)(int priority, const char *message, va_list args);
	int   (*printf_i)(const char *fmt, ...) __PRINT_FMT(2, 3);
	int   (*vprintf_i)(const char *fmt, va_list args);
	int   (*snprintf_i)(char *str, size_t size, const char *fmt, ...);
	int   (*vsnprintf_i)(char *str, size_t size, const char *fmt, va_list args);
    void  (*qsort_i)(void *base, size_t nel, size_t width, int (*compar)(const void *, const void *));
    // abort
	void  (*exit_i)(int p);

	// high-level functions
	const char* (*packagepath)(char *buf, size_t bufsiz, const char *pkgname);
	const char* (*exportpath)(char *buf, size_t bufsiz, const char *pkgname);
	const char* (*begin)(kinfotag_t);
	const char* (*end)(kinfotag_t);
	void  (*dbg_p)(const char *file, const char *func, int line, const char *fmt, ...) __PRINT_FMT(4, 5);
} kplatform_t;

/* ------------------------------------------------------------------------ */
/* type */

#ifndef __KERNEL__
#include <limits.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#endif

#if defined(__LP64__) || defined(_WIN64)
#define K_USING_SYS64_    1
typedef int32_t           kshort_t;
typedef uint32_t          kushort_t;
#ifdef K_USING_NOFLOAT
typedef uintptr_t         kfloat_t;
#else
typedef double            kfloat_t;
#endif
#else
typedef int16_t           kshort_t;
typedef uint16_t          kushort_t;
#ifdef K_USING_NOFLOAT
typedef uintptr_t         kfloat_t;
#else
typedef float             kfloat_t;
#endif
#endif

typedef bool             kbool_t;

typedef enum {
	K_FAILED, K_CONTINUE, K_BREAK
} kstatus_t;

typedef intptr_t         kint_t;
typedef uintptr_t        kuint_t;

#ifdef K_USING_SYS64_

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

#else/*K_USING_SYS64_*/

#define KINT_MAX               LONG_MAX
#define KINT_MIN               LONG_MIN
#define KINT_FMT               "%ld"
#define KINT0                  0UL

#endif/*K_USING_SYS64_*/

typedef intptr_t         kindex_t;
typedef kushort_t        kflag_t;    /* flag field */

#define KFLAG_H(N)               ((sizeof(kflag_t)*8)-N)
#define KFLAG_H0                 ((kflag_t)(1 << KFLAG_H(1)))
#define KFLAG_H1                 ((kflag_t)(1 << KFLAG_H(2)))
#define KFLAG_H2                 ((kflag_t)(1 << KFLAG_H(3)))
#define KFLAG_H3                 ((kflag_t)(1 << KFLAG_H(4)))
#define KFLAG_H4                 ((kflag_t)(1 << KFLAG_H(5)))
#define KFLAG_H5                 ((kflag_t)(1 << KFLAG_H(6)))
#define KFLAG_H6                 ((kflag_t)(1 << KFLAG_H(7)))
#define KFLAG_H7                 ((kflag_t)(1 << KFLAG_H(8)))

#define TFLAG_is(T,f,op)          (((T)(f) & (T)(op)) == (T)(op))
#define TFLAG_set1(T,f,op)        f = (((T)(f)) | ((T)(op)))
#define TFLAG_set0(T,f,op)        f = (((T)(f)) & (~((T)(op))))
#define TFLAG_set(T,f,op,b)       if(b) {TFLAG_set1(T,f,op);} else {TFLAG_set0(T,f,op);}

#define FLAG_set(f,op)            TFLAG_set1(kflag_t,f,op)
#define FLAG_unset(f,op)          TFLAG_set0(kflag_t,f,op)
#define FLAG_is(f,op)             TFLAG_is(kflag_t,f,op)

/* uline */

typedef kushort_t                 kfileid_t;
#define URI_unknown               ((kfileid_t)-1)
#define URI_EVAL                  ((kfileid_t)0)
#define URI_UNMASK(fileid)           (fileid)

#define URI__(fileid) S_text(knh_getURN(_ctx, fileid))
#define shortfilename__(fileid) knh_sfile(URI__(fileid))

typedef uintptr_t                 kline_t;
#define NOPLINE                   0
#define new_ULINE(fileid, line)       ((((kline_t)fileid) << (sizeof(kfileid_t) * 8)) | ((kushort_t)line))
#define ULINE_setURI(line, fileid)    line |= (((kline_t)fileid) << (sizeof(kfileid_t) * 8))
#define ULINE_fileid(line)            ((kfileid_t)(line >> (sizeof(kfileid_t) * 8)))
#define ULINE_line(line)           (line & (kline_t)((kfileid_t)-1))

/* ------------------------------------------------------------------------ */

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

typedef struct karray_t {
	size_t bytesize;
	union {
		char  *bytebuf;
		const struct _kclass **cts;
		struct kvs_t          *kvs;
		struct kopl_t          *opl;
		const struct _kObject **objects;
		struct _kObject       **refhead;  // stack->ref
	};
	size_t bytemax;
} karray_t ;

typedef struct kwb_t {
	karray_t *m;
	size_t pos;
} kwb_t;

// kmap

typedef struct kmape_t {
	uintptr_t hcode;
	struct kmape_t *next;
	union {
		const struct _kString  *skey;
		const struct _kParam   *paramkey;
		uintptr_t        ukey;
		void            *pkey;
	};
	union {
		const struct _kObject  *ovalue;
		void            *pvalue;
		uintptr_t        uvalue;
	};
} kmape_t;

typedef struct kmap_t {
	kmape_t *arena;
	kmape_t *unused;
	kmape_t **hentry;
	size_t arenasize;
	size_t size;
	size_t hmax;
} kmap_t;

// classdef_t

typedef kushort_t       kpack_t;   /* package id*/
typedef kushort_t       kcid_t;    /* class id */
typedef kushort_t       ktype_t;     /* extended ktype_t */
typedef kushort_t       ksymbol_t;
typedef kushort_t       kmethodn_t;
typedef kushort_t       kparamid_t;

/* kcid_t */
#define CLASS_newid        ((ktype_t)-1)
#define TY_unknown         ((ktype_t)-2)

#define CT_(t)              (_ctx->share->ca.cts[t])
#define CT_cparam(CT)       (_ctx->share->paramdomList->params[(CT)->paramdom])
#define TY_isUnbox(t)       FLAG_is(CT_(t)->cflag, kClass_UnboxType)
#define CT_isUnbox(C)       FLAG_is(C->cflag, kClass_UnboxType)

#define SYM_MAX            KFLAG_H3
#define SYM_HEAD(sym)      (sym  & (KFLAG_H0|KFLAG_H1|KFLAG_H2))
#define SYM_UNMASK(sym)    (sym & (~(KFLAG_H0|KFLAG_H1|KFLAG_H2|KFLAG_H3)))

#define SYM_NONAME          ((ksymbol_t)-1)
#define SYM_NEWID           ((ksymbol_t)-2)
#define SYM_NEWRAW          ((ksymbol_t)-3)
#define _NEWID              SYM_NEWID
#define _NEWRAW             SYM_NEWRAW

#define SYMKEY_BOXED            KFLAG_H3
#define SYMKEY_unbox(sym)       (sym & ~(SYMKEY_BOXED))
#define SYMKEY_isBOXED(sym)     ((sym & SYMKEY_BOXED) == SYMKEY_BOXED)

#define FN_COERCION             KFLAG_H3
#define FN_Coersion             FN_COERCION
#define FN_isCOERCION(fn)       ((fn & FN_COERCION) == FN_COERCION)

#define MN_ISBOOL     KFLAG_H0
#define MN_GETTER     KFLAG_H1
#define MN_SETTER     KFLAG_H2

#define MN_Annotation (KFLAG_H1|KFLAG_H2)
#define MN_isAnnotation(S)   ((S & KW_PATTERN) == MN_Annotation)

#define MN_TOCID      (KFLAG_H0|KFLAG_H1)
#define MN_ASCID      (KFLAG_H0|KFLAG_H1|KFLAG_H2)
#define KW_PATTERN    (KFLAG_H0|KFLAG_H1|KFLAG_H2)
#define KW_isPATTERN(S)      ((S & KW_PATTERN) == KW_PATTERN)

#define MN_isISBOOL(mn)   (SYM_HEAD(mn) == MN_ISBOOL)
#define MN_toISBOOL(mn)   ((SYM_UNMASK(mn)) | MN_ISBOOL)
#define MN_isGETTER(mn)   (SYM_HEAD(mn) == MN_GETTER)
#define MN_toGETTER(mn)   ((SYM_UNMASK(mn)) | MN_GETTER)
#define MN_isSETTER(mn)   (SYM_HEAD(mn) == MN_SETTER)
#define MN_toSETTER(mn)   ((SYM_UNMASK(mn)) | MN_SETTER)

#define MN_to(cid)        ((CT_(cid)->nameid) | MN_TOCID)
#define MN_isTOCID(mn)    ((SYM_UNMASK(mn)) == MN_TOCID)
#define MN_as(cid)        ((CT_(cid)->nameid) | MN_ASCID)
#define MN_isASCID(mn)    ((SYM_UNMASK(mn)) == MN_ASCID)

/* ------------------------------------------------------------------------ */

typedef const struct kcontext_t* konoha_t;

struct  kcontext_t;
typedef const struct kcontext_t *const CTX_t;
#define CTX      CTX_t _ctx

#define MOD_MAX    32
struct _kObject;

#define MOD_logger   0
#define MOD_gc       1
#define MOD_code     2
#define MOD_sugar    3
#define MOD_float        11
#define MOD_iterator     12
#define MOD_iconv   13
//#define MOD_IO      14
//#define MOD_llvm    15
//#define MOD_REGEX   16

struct kmodlocal_t;
typedef struct kmodlocal_t {
	uintptr_t unique;
	void (*reftrace)(CTX, struct kmodlocal_t *);
	void (*free)(CTX, struct kmodlocal_t *);
} kmodlocal_t;

struct kmodshare_t;
typedef struct kmodshare_t {
	const char *name;
	int mod_id;
	void (*setup)(CTX, struct kmodshare_t *, int newctx);
	void (*reftrace)(CTX, struct kmodshare_t *);
	void (*free)(CTX, struct kmodshare_t *);
} kmodshare_t;

#define kContext_Debug          ((kflag_t)(1<<0))
#define kContext_Interactive    ((kflag_t)(1<<1))
#define kContext_CompileOnly    ((kflag_t)(1<<1))

#define CTX_isInteractive(X)  (TFLAG_is(kflag_t,(X)->stack->flag, kContext_Interactive))
#define CTX_isCompileOnly(X)  (TFLAG_is(kflag_t,(X)->stack->flag, kContext_CompileOnly))
#define CTX_setInteractive(X)  TFLAG_set1(kflag_t, (X)->stack->flag, kContext_Interactive)
#define CTX_setCompileOnly(X)  TFLAG_set1(kflag_t, (X)->stack->flag, kContext_CompileOnly)

typedef struct kcontext_t {
	int						          safepoint; // set to 1
	struct ksfp_t                    *esp;
	const struct _klib2              *lib2;
	const kplatform_t                *plat;
	/* TODO(imasahiro)
	 * checking modgc performance and remove
	 * memshare/memlocal from context
	 */
	struct kmemshare_t                *memshare;
	struct kmemlocal_t                *memlocal;
	struct kshare_t                   *share;
	struct kstack_t                   *stack;
	struct kmodshare_t               **modshare;
	struct kmodlocal_t               **modlocal;
} kcontext_t ;

typedef struct kshare_t {
	karray_t ca;
	struct kmap_t               *lcnameMapNN;
	/* system shared const */
	const struct _kObject       *constNull;
	const struct _kBoolean      *constTrue;
	const struct _kBoolean      *constFalse;
	const struct _kString       *emptyString;
	const struct _kArray        *emptyArray;

	const struct _kArray         *fileidList;    // file, http://
	struct kmap_t                *fileidMapNN;   //
	const struct _kArray         *packList;
	struct kmap_t                *packMapNN;
	const struct _kArray         *symbolList;  // NAME, Name, INT_MAX Int_MAX
	struct kmap_t                *symbolMapNN;
	const struct _kArray         *paramList;
	struct kmap_t                *paramMapNN;
	const struct _kArray         *paramdomList;
	struct kmap_t                *paramdomMapNN;
} kshare_t ;

#define K_FRAME_NCMEMBER \
		uintptr_t   ndata;  \
		kbool_t     bvalue; \
		kint_t      ivalue; \
		kuint_t     uvalue; \
		kfloat_t    fvalue; \
		intptr_t    shift;  \
		uintptr_t   uline; \
		struct kopl_t  *pc; \
		const struct _kMethod *mtdNC; \
		const char     *fname \

#define K_FRAME_MEMBER \
		const struct _kObject *o;  \
		struct _kObject       *Wo; \
		const struct _kInt    *i; \
		const struct _kFloat  *f; \
		const struct _kString *s; \
		const struct _kBytes  *ba; \
		const struct _kArray  *a; \
		const struct _kMethod            *mtd;\
		const struct _kFunc         *fo; \
		const struct _kNameSpace             *ks;\
		const struct _kToken             *tk;\
		const struct _kStmt              *stmt;\
		const struct _kExpr              *expr;\
		const struct _kBlock             *bk;\
		struct _kGamma  *gma;   \
		struct _kIterator *itr; \
		struct kClass  *c; \
		struct kDate *dt;\
		struct kRegex  *re; \
		struct kRange  *range; \
		struct kIterator *it; \
		struct kMap           *m;    \
		struct kInputStream  *in; \
		struct kOutputStream *w;  \
		struct kException         *e;\
		struct kExceptionHandler  *hdr; \
		struct kConverter         *conv;\
		struct kContext           *cx;\
		struct kScript            *scr;\
		kint_t     dummy_ivalue;\
		kfloat_t   dummy_fvalue \

typedef struct ksfp_t {
	union {
		K_FRAME_MEMBER;
	};
	union {
		K_FRAME_NCMEMBER;
	};
} ksfp_t;

typedef struct krbp_t {
	union {
		K_FRAME_NCMEMBER;
		K_FRAME_MEMBER;
	};
} krbp_t;

typedef struct kstack_t {
	struct ksfp_t*               stack;
	size_t                       stacksize;
	struct ksfp_t*               stack_uplimit;
	const struct _kArray        *gcstack;
	karray_t                     cwb;
	// local info
	kflag_t                      flag;
	CTX_t                        *rootctx;
	void*                        cstack_bottom;  // for GC
	karray_t                     ref;   // reftrace
	struct _kObject**            reftail;
	ktype_t   evalty;
	kushort_t evalidx;
	jmpbuf_i  *evaljmpbuf;
} kstack_t;

typedef struct kfield_t {
	kflag_t    flag    ;
	kshort_t   isobj   ;
	ktype_t    ty      ;
	ksymbol_t  fn      ;
} kfield_t ;

#define P_STR    0
#define P_DUMP   1

struct _kclass;

#define KCLASSSPI \
		void (*init)(CTX, const struct _kObject*, void *conf);\
		void (*reftrace)(CTX, const struct _kObject*);\
		void (*free)(CTX, const struct _kObject*);\
		const struct _kObject* (*fnull)(CTX, const struct _kclass*);\
		void (*p)(CTX, ksfp_t *, int, kwb_t *, int);\
		uintptr_t (*unbox)(CTX, const struct _kObject*);\
		int  (*compareTo)(const struct _kObject*, const struct _kObject*);\
		void (*initdef)(CTX, struct _kclass*, kline_t);\
		kbool_t (*isSubType)(CTX, const struct _kclass*, const struct _kclass*);\
		const struct _kclass* (*realtype)(CTX, const struct _kclass*, const struct _kclass*)

typedef struct KDEFINE_CLASS {
	const char *structname;
	kcid_t     cid;         kflag_t    cflag;
	kcid_t     bcid;        kcid_t     supcid;
	ktype_t    rtype;       kushort_t  psize;
	struct kparam_t   *cparams;
	size_t     cstruct_size;
	kfield_t   *fields;
	kushort_t  fsize;       kushort_t fallocsize;
	KCLASSSPI;
} KDEFINE_CLASS;

#define STRUCTNAME(C) \
	.structname = #C,\
	.cid = CLASS_newid,\
	.cstruct_size = sizeof(k##C)\

struct _kString;
struct _kObject;
//struct _kclass;
typedef uintptr_t kmagicflag_t;

typedef const struct _kclass kclass_t;
struct _kclass {
	KCLASSSPI;
	kpack_t   packid;       kpack_t   packdom;
	kcid_t   cid;           kflag_t  cflag;
	kcid_t   bcid;          kcid_t   supcid;
	ktype_t  p0;            kparamid_t paramdom;
	kmagicflag_t magicflag;
	size_t     cstruct_size;
	kfield_t  *fields;
	kushort_t  fsize;         kushort_t fallocsize;
	const char               *DBG_NAME;
	ksymbol_t                 nameid;
	kushort_t                 optvalue;

	const struct _kArray     *methods;
	const struct _kString    *shortNameNULL;
	union {   // default value
		const struct _kObject  *nulvalNUL;
		struct _kObject        *WnulvalNUL;
	};
	struct kmap_t            *constPoolMapNO;
	kclass_t                 *searchSimilarClassNULL;
	kclass_t                 *searchSuperMethodClassNULL;
} ;

/* ----------------------------------------------------------------------- */
/* CLASS */

/* ------------------------------------------------------------------------ */
/* mini konoha */

#define CLASS_Tvoid             ((kcid_t)0)
#define CLASS_Tvar              ((kcid_t)1)
#define CLASS_Object            ((kcid_t)2)
#define CLASS_Boolean           ((kcid_t)3)
#define CLASS_Int               ((kcid_t)4)
#define CLASS_String            ((kcid_t)5)
#define CLASS_Array             ((kcid_t)6)
#define CLASS_Param             ((kcid_t)7)
#define CLASS_Method            ((kcid_t)8)
#define CLASS_Func              ((kcid_t)9)
#define CLASS_System            ((kcid_t)10)
#define CLASS_T0                ((kcid_t)11)    /* ParamType*/

#define CT_Object               CT_(CLASS_Object)
#define CT_Boolean              CT_(CLASS_Boolean)
#define CT_Int                  CT_(CLASS_Int)
#define CT_String               CT_(CLASS_String)
#define CT_Array                CT_(CLASS_Array)
#define CT_Param                CT_(CLASS_Param)
#define CT_Method               CT_(CLASS_Method)
#define CT_Func                 CT_(CLASS_Func)
#define CT_System               CT_(CLASS_System)

#define CT_StringArray          CT_Array
#define kStringArray            kArray
#define CT_MethodArray          CT_Array
#define kMethodArray            kArray

#define kClass_Ref              ((kflag_t)(1<<0))
#define kClass_Prototype        ((kflag_t)(1<<1))
#define kClass_Immutable        ((kflag_t)(1<<2))
#define kClass_Private          ((kflag_t)(1<<4))
#define kClass_Final            ((kflag_t)(1<<5))
#define kClass_Singleton        ((kflag_t)(1<<6))
#define kClass_UnboxType        ((kflag_t)(1<<7))
#define kClass_Interface        ((kflag_t)(1<<8))
#define kClass_TypeVar          ((kflag_t)(1<<9))
#define kClass_Forward          ((kflag_t)(1<<10))

#define CT_isPrivate(ct)      (TFLAG_is(kflag_t,(ct)->cflag, kClass_Private))

#define TY_isSingleton(T)     (TFLAG_is(kflag_t,(CT_(T))->cflag, kClass_Singleton))
#define CT_isSingleton(ct)    (TFLAG_is(kflag_t,(ct)->cflag, kClass_Singleton))

#define TY_isForward(T)     (TFLAG_is(kflag_t,(CT_(T))->cflag, kClass_Forward))
#define CT_isForward(ct)    (TFLAG_is(kflag_t,(ct)->cflag, kClass_Forward))

#define CT_isFinal(ct)         (TFLAG_is(kflag_t,(ct)->cflag, kClass_Final))

// this is used in konoha.class
#define CT_isDefined(ct)  ((ct)->fallocsize == 0 || (ct)->fsize == (ct)->fallocsize)

//#define TY_isUnboxType(t)    (TFLAG_is(kflag_t,(ClassTBL(t))->cflag, kClass_UnboxType))
//#define T_isInterface(t)    (TFLAG_is(kflag_t,(ClassTBL(t))->cflag, kClass_Interface))
//#define T_isTypeVar(t)      (TFLAG_is(kflag_t,(ClassTBL(t))->cflag, kClass_TypeVar))

#define TY_isFunc(T)    (CT_(T)->bcid == CLASS_Func)

/* magic flag */
#define MAGICFLAG(f)             (K_OBJECT_MAGIC | ((kmagicflag_t)(f) & K_CFLAGMASK))

#define kObject_NullObject       ((kmagicflag_t)(1<<0))

#define kObject_Local6           ((kmagicflag_t)(1<<10))
#define kObject_Local5           ((kmagicflag_t)(1<<11))
#define kObject_Local4           ((kmagicflag_t)(1<<12))
#define kObject_Local3           ((kmagicflag_t)(1<<13))
#define kObject_Local2           ((kmagicflag_t)(1<<14))
#define kObject_Local1           ((kmagicflag_t)(1<<15))

#define kObject_is(O,A)            (TFLAG_is(kmagicflag_t,(O)->h.magicflag,A))
#define kObject_set(O,A,B)         TFLAG_set(kmagicflag_t,(O)->h.magicflag,A,B)

#define kField_Hidden          ((kflag_t)(1<<0))
#define kField_Protected       ((kflag_t)(1<<1))
#define kField_Getter          ((kflag_t)(1<<2))
#define kField_Setter          ((kflag_t)(1<<3))
#define kField_Key             ((kflag_t)(1<<4))
#define kField_Volatile        ((kflag_t)(1<<5))
#define kField_ReadOnly        ((kflag_t)(1<<6))
#define kField_Property        ((kflag_t)(1<<7))

/* ------------------------------------------------------------------------ */
/* Type Variable */
//## @TypeVariable class Tvoid Tvoid;
//## @TypeVariable class Tvar  Tvoid;

#define OFLAG_Tvoid              MAGICFLAG(0)
#define CFLAG_Tvoid              kClass_TypeVar|kClass_UnboxType|kClass_Singleton|kClass_Final
#define TY_void                  CLASS_Tvoid
#define OFLAG_Tvar               MAGICFLAG(0)
#define CFLAG_Tvar               CFLAG_Tvoid
#define TY_var                   CLASS_Tvar

/* ------------------------------------------------------------------------ */

#define CFLAG_Object               0
#define OFLAG_Object               MAGICFLAG(0)
#define TY_Object                  CLASS_Object

#define kObject_NullObject         ((kmagicflag_t)(1<<0))
#define kObject_isNullObject(o)    (TFLAG_is(kmagicflag_t,(o)->h.magicflag,kObject_NullObject))
#define kObject_setNullObject(o,b) TFLAG_set(kmagicflag_t,((struct _kObject*)o)->h.magicflag,kObject_NullObject,b)
#define IS_NULL(o)                 ((((o)->h.magicflag) & kObject_NullObject) == kObject_NullObject)
#define IS_NOTNULL(o)              ((((o)->h.magicflag) & kObject_NullObject) != kObject_NullObject)

#define K_FASTMALLOC_SIZE  (sizeof(void*) * 8)

#define K_OBJECT_MAGIC           (578L << ((sizeof(kflag_t)*8)))
#define K_CFLAGMASK              (FLAG_Object_Ref)
#define KNH_MAGICFLAG(f)         (K_OBJECT_MAGIC | ((kmagicflag_t)(f) & K_CFLAGMASK))
#define DBG_ASSERT_ISOBJECT(o)   DBG_ASSERT(TFLAG_is(uintptr_t,(o)->h.magicflag, K_OBJECT_MAGIC))

typedef struct kObjectHeader {
	kmagicflag_t magicflag;
	kclass_t *ct;  //@RENAME
	union {
		uintptr_t refc;  // RCGC
		void *gcinfo;
		uintptr_t hashcode; // reserved
	};
	karray_t *kvproto;
} kObjectHeader ;

typedef const struct _kObject kObject;

struct _kObject {
	kObjectHeader h;
	union {
		const struct _kObject *fields[4];
		uintptr_t ndata[4];
	};
};

typedef struct kvs_t {
	ksymbol_t key;
	ktype_t   ty;
	union {
		uintptr_t                uval;
		kObject                 *oval;
		const struct _kString   *sval;
	};
} kvs_t;

#define O_ct(o)             ((o)->h.ct)
#define O_cid(o)            (O_ct(o)->cid)
#define O_bcid(o)           (O_ct(o)->bcid)
#define O_unbox(o)          (O_ct(o)->unbox(_ctx, o))
#define O_p0(o)             (O_ct(o)->p0)

/* ------------------------------------------------------------------------ */
//## @Immutable class Boolean Object;

#define CFLAG_Boolean              kClass_Immutable|kClass_UnboxType|kClass_Final
#define OFLAG_Boolean              MAGICFLAG(0)
#define TY_Boolean                 CLASS_Boolean
#define IS_Boolean(o)              (O_cid(o) == CLASS_Boolean)

#define ABSTRACT_NUMBER \
		union {\
			uintptr_t  ndata;\
			kbool_t    bvalue;\
			kint_t     ivalue;\
			kuint_t    uvalue;\
			kfloat_t   fvalue;\
		}\

typedef const struct _kNumber kNumber;
struct _kNumber {
	kObjectHeader h;
	ABSTRACT_NUMBER;
};

typedef const struct _kBoolean kBoolean;
struct _kBoolean /* extends kNumber */ {
	kObjectHeader h;
	ABSTRACT_NUMBER;
};

#define IS_TRUE(o)             (O_bcid(o) == CLASS_Boolean && N_tobool(o))
#define IS_FALSE(o)            (O_bcid(o) == CLASS_Boolean && N_tobool(o) == 0)
#define new_Boolean(_ctx, c)    ((c) ? K_TRUE : K_FALSE)
#define N_toint(o)             (((kBoolean*)o)->ivalue)
#define N_tofloat(o)           (((kBoolean*)o)->fvalue)
#define N_tobool(o)            (((kBoolean*)o)->bvalue)

/* ------------------------------------------------------------------------ */
//## @Immutable class Int Number;

#define CFLAG_Int              kClass_Immutable|kClass_UnboxType|kClass_Final
#define OFLAG_Int              MAGICFLAG(0)
#define TY_Int                 CLASS_Int
#define IS_Int(o)              (O_cid(o) == CLASS_Int)

typedef const struct _kInt kInt;
struct _kInt /* extends kNumber */ {
	kObjectHeader h;
	ABSTRACT_NUMBER;
	void *bignum;  /* reserved */
};

/* ------------------------------------------------------------------------ */
/* String */

#define CFLAG_String              kClass_Immutable|kClass_Final
#define OFLAG_String              MAGICFLAG(0)
#define TY_String                 CLASS_String
#define TY_TEXT                   TY_void    /*special use for const char*/
#define TY_TYPE                   TY_var     /*special use for kclass_t*/
#define IS_String(o)              (O_cid(o) == CLASS_String)

/*
 * Bit encoding for Rope String
 * 5432109876543210
 * 000xxxxxxxxxxxxx ==> magicflag bit representation
 * 001xxxxxxxxxxxxx LinerString
 * 011xxxxxxxxxxxxx ExterenalString
 * 010xxxxxxxxxxxxx InlinedString
 * 100xxxxxxxxxxxxx RopeString
 */
#define S_FLAG_MASK_BASE (13)
#define S_FLAG_LINER     ((1UL << (0)))
#define S_FLAG_NOFREE    ((1UL << (1)))
#define S_FLAG_ROPE      ((1UL << (2)))
#define S_FLAG_INLINE    (S_FLAG_NOFREE)
#define S_FLAG_EXTERNAL  (S_FLAG_LINER | S_FLAG_NOFREE)

#define S_isRope(o)          (TFLAG_is(uintptr_t,(o)->h.magicflag,kObject_Local1))
#define S_isTextSgm(o)       (TFLAG_is(uintptr_t,(o)->h.magicflag,kObject_Local2))
#define S_setTextSgm(o,b)    TFLAG_set(uintptr_t,(o)->h.magicflag,kObject_Local2,b)
#define S_isMallocText(o)    (TFLAG_is(uintptr_t,(o)->h.magicflag,kObject_Local3))
#define S_setMallocText(o,b) TFLAG_set(uintptr_t,(o)->h.magicflag,kObject_Local3,b)
#define S_isASCII(o)         (TFLAG_is(uintptr_t,(o)->h.magicflag,kObject_Local4))
#define S_setASCII(o,b)      TFLAG_set(uintptr_t,((struct _kObject*)o)->h.magicflag,kObject_Local4,b)
#define S_isPooled(o)        (TFLAG_is(uintptr_t,(o)->h.magicflag,kObject_Local5))
#define S_setPooled(o,b)     TFLAG_set(uintptr_t,(o)->h.magicflag,kObject_Local5,b)
#define SIZEOF_INLINETEXT    (sizeof(void*)*8 - sizeof(kBytes))

typedef const struct _kBytes kBytes;
struct _kBytes {
	kObjectHeader h;
	COMMON_BYTEARRAY;
};

typedef const struct _kString kString;
struct _kString /* extends _Bytes */ {
	kObjectHeader h;
	COMMON_BYTEARRAY;
	const char inline_text[SIZEOF_INLINETEXT];
};

#define SPOL_TEXT          (1<<0)
#define SPOL_ASCII         (1<<1)
#define SPOL_UTF8          (1<<2)
#define SPOL_POOL          (1<<3)
#define SPOL_NOPOOL        (1<<5)
#define SPOL_NOCOPY        (1<<4)

#define new_T(t)            new_kString(t, knh_strlen(t), SPOL_TEXT|SPOL_ASCII|SPOL_POOL)
#define new_S(T, L)         new_kString(T, L, SPOL_ASCII|SPOL_POOL)
#define S_text(s)           ((const char*) (O_ct(s)->unbox(_ctx, (kObject*)s)))
#define S_size(s)           ((s)->bytesize)

//#define S_equals(s, b)        knh_bytes_equals(S_tobytes(s), b)
//#define S_startsWith(s, b)    knh_bytes_startsWith_(S_tobytes(s), b)
//#define S_endsWith(s, b)      knh_bytes_endsWith_(S_tobytes(s), b)

/* ------------------------------------------------------------------------ */
//## class Array   Object;

#define CFLAG_Array              kClass_Final
#define OFLAG_Array              MAGICFLAG(0)
#define TY_Array                 CLASS_Array
#define IS_Array(o)              (O_bcid(o) == CLASS_Array)

#define kArray_isUnboxData(o)    (TFLAG_is(uintptr_t,(o)->h.magicflag,kObject_Local1))
#define kArray_setUnboxData(o,b) TFLAG_set(uintptr_t,(o)->h.magicflag,kObject_Local1,b)

typedef const struct _kArray kArray;
struct _kArray {
	kObjectHeader h;
	size_t bytesize;
	union {
		uintptr_t              *ndata;
		kint_t                 *ilist;
#ifdef K_USING_FLOAT
		kfloat_t               *flist;
#endif
		const struct _kObject        **list;
		const struct _kString        **strings;
		const struct _kParam         **params;
		const struct _kMethod        **methods;
		const struct _kFunc          **funcs;
		const struct _kToken         **toks;
		struct _kToken        **Wtoks;
		const struct _kExpr          **exprs;
		struct _kExpr         **Wexprs;
		const struct _kStmt          **stmts;
		struct _kStmt         **Wstmts;
	};
	size_t bytemax;
};

/* ------------------------------------------------------------------------ */
//## @Private @Immutable class Param Object;
//## flag Param VARGs  1 - is set * *;
//## flag Param RVAR   2 - is set * *;

#define CFLAG_Param              kClass_Final
#define OFLAG_Param              MAGICFLAG(0)
#define TY_Param                 CLASS_Param
#define IS_Param(o)              (O_bcid(o) == CLASS_Param)

typedef struct kparam_t {
	ktype_t    ty;  ksymbol_t  fn;
} kparam_t;

typedef const struct _kParam kParam;
struct _kParam {
	kObjectHeader h;
	ktype_t rtype; kushort_t psize;
	kparam_t p[3];
};

/* ------------------------------------------------------------------------ */
//## class Method Object;

#define CFLAG_Method              kClass_Final
#define OFLAG_Method              MAGICFLAG(0)
#define TY_Method                 CLASS_Method
#define IS_Method(o)              (O_bcid(o) == CLASS_Method)

typedef const struct _kMethod kMethod;

#define kMethod_Public               ((uintptr_t)(1<<0))
#define kMethod_Virtual              ((uintptr_t)(1<<1))
#define kMethod_Hidden               ((uintptr_t)(1<<2))
#define kMethod_Const                ((uintptr_t)(1<<3))
#define kMethod_Static               ((uintptr_t)(1<<4))
#define kMethod_Immutable            ((uintptr_t)(1<<5))
#define kMethod_Restricted           ((uintptr_t)(1<<6))
#define kMethod_Overloaded           ((uintptr_t)(1<<7))
#define kMethod_CALLCC               ((uintptr_t)(1<<8))
#define kMethod_FASTCALL             ((uintptr_t)(1<<9))
#define kMethod_D                    ((uintptr_t)(1<<10))
#define kMethod_Abstract             ((uintptr_t)(1<<11))
#define kMethod_Coercion             ((uintptr_t)(1<<12))
#define kMethod_SmartReturn          ((uintptr_t)(1<<13))

#define kMethod_isPublic(o)     (TFLAG_is(uintptr_t, (o)->flag,kMethod_Public))
//#define kMethod_setPublic(o,B)  TFLAG_set(uintptr_t, (o)->flag,kMethod_Public,B)
#define kMethod_isVirtual(o)     (TFLAG_is(uintptr_t, (o)->flag,kMethod_Virtual))
//#define kMethod_setVirtual(o,B)  TFLAG_set(uintptr_t, (o)->flag,kMethod_Virtual,B)
#define kMethod_isHidden(o)     (TFLAG_is(uintptr_t, (o)->flag,kMethod_Hidden))
#define kMethod_setHidden(o,B)  TFLAG_set(uintptr_t, (o)->flag,kMethod_Hidden,B)
#define kMethod_isStatic(o)     (TFLAG_is(uintptr_t, (o)->flag,kMethod_Static))
#define kMethod_setStatic(o,B)  TFLAG_set(uintptr_t, (o)->flag,kMethod_Static,B)
#define kMethod_isConst(o)      (TFLAG_is(uintptr_t, (o)->flag,kMethod_Const))
//#define kMethod_setConst(o,B)   TFLAG_set(uintptr_t, (o)->flag,kMethod_Const,B)
#define kMethod_isVirtual(o)     (TFLAG_is(uintptr_t, (o)->flag,kMethod_Virtual))
#define kMethod_setVirtual(o,B)  TFLAG_set(uintptr_t, (o)->flag,kMethod_Virtual,B)

#define kMethod_isSmartReturn(o)     (TFLAG_is(uintptr_t, (o)->flag, kMethod_SmartReturn))

#define kMethod_isTransCast(mtd)    MN_isTOCID(mtd->mn)
#define kMethod_isCast(mtd)         MN_isASCID(mtd->mn)
#define kMethod_isCoercion(mtd)    (TFLAG_is(uintptr_t, (mtd)->flag,kMethod_Coercion))
//#define kMethod_isOverload(o)  (TFLAG_is(uintptr_t,DP(o)->flag,kMethod_Overload))
//#define kMethod_setOverload(o,b) TFLAG_set(uintptr_t,DP(o)->flag,kMethod_Overload,b)

#define kMethod_param(mtd)        _ctx->share->paramList->params[mtd->paramid]
#define kMethod_rtype(mtd)        (kMethod_param(mtd))->rtype

/* method data */
#define DEND     (-1)

#if 1
#define _RIX         ,long _rix
#define K_RIX        _rix
#define K_RIXPARAM    ,K_RTNIDX
#define _KFASTCALL   ,long _rix
#define K_FASTRIX    _rix
#else
#define _RIX
#define K_RIX (-4)
#define K_RIXPARAM
#define _KFASTCALL
#define K_FASTRIX K_RIXPARAM
#endif

#ifdef K_USING_WIN32_
//#define KMETHOD  void CC_EXPORT
//#define ITRNEXT int   CC_EXPORT
//typedef void (CC_EXPORT *knh_Fmethod)(CTX, ksfp_t* _RIX);
//typedef int  (CC_EXPORT *knh_Fitrnext)(CTX, ksfp_t * _RIX);
#else
#define KMETHOD    void  /*CC_FASTCALL_*/
#define KMETHODCC  int  /*CC_FASTCALL_*/
typedef KMETHOD   (*knh_Fmethod)(CTX, ksfp_t* _RIX);
typedef KMETHOD   (*FmethodFastCall)(CTX, ksfp_t * _KFASTCALL);
typedef KMETHODCC (*FmethodCallCC)(CTX, ksfp_t *, int, int, struct kopl_t*);
#endif

struct _kMethod {
	kObjectHeader     h;
	union {
		knh_Fmethod          fcall_1;
		FmethodFastCall      fastcall_1;
	};
	union {/* body*/
		struct kopl_t        *pc_start;
		FmethodCallCC         callcc_1;
	};
	uintptr_t         flag;
	kcid_t            cid;      kmethodn_t  mn;
	kparamid_t        paramid;  kparamid_t paramdom;
	kshort_t          delta;    kpack_t packid;
	const struct _kToken        *tcode;
	union {
		kObject              *objdata;
		const struct _kKonohaCode   *kcode;
		const struct _kNameSpace  *lazyns;       // lazy compilation
	};
	const struct _kMethod           *proceedNUL;   // proceed
};

/* ------------------------------------------------------------------------ */
//## class Func Object;

#define CFLAG_Func              kClass_Final
#define OFLAG_Func              MAGICFLAG(0)
#define TY_Func                 CLASS_Func
#define IS_Func(o)              (O_bcid(o) == CLASS_Func)

typedef const struct _kFunc kFunc;
struct _kFunc {
	kObjectHeader h;
	kObject *self;
	kMethod *mtd;
};

/* ------------------------------------------------------------------------ */
//## @Singleton class System Object;

#define CFLAG_System              kClass_Singleton|kClass_Final
#define OFLAG_System              MAGICFLAG(0)
#define TY_System                 CLASS_System
#define IS_System(o)              (O_cid(o) == CLASS_System)

typedef const struct _kSystem kSystem;

struct _kSystem {
	kObjectHeader h;
};

/* ------------------------------------------------------------------------ */
//## class Tdynamic Object;

#define CFLAG_T0                    kClass_TypeVar|kClass_UnboxType|kClass_Singleton|kClass_Final
#define TY_T0                       CLASS_T0
#define TY_0                        CLASS_T0

//
//#define CFLAG_Tdynamic              kClass_Final
//#define OFLAG_Tdynamic              MAGICFLAG(0)
//#define TY_dynamic                  CLASS_Tdynamic
//#define CLASS_RawPtr                CLASS_Tdynamic
//#define T_RawPtr                    CLASS_Tdynamic
//
////typedef struct kObject {
////	kObjectHeader h;
////	void         *rawptr;
////	kArray       *gcbuf;
////	void         (*rawfree)(void *);
////} kObject ;

//#define new_ReturnRawPtr(_ctx, sfp, p)  new_ReturnCppObject(_ctx, sfp, p, NULL)

#define K_CALLDELTA   4
#define K_RTNIDX    (-4)
#define K_SHIFTIDX  (-3)
#define K_PCIDX     (-2)
#define K_MTDIDX    (-1)
#define K_TMRIDX    (0)
#define K_SELFIDX   0

//#define K_NEXTIDX    2
#define K_ULINEIDX2  (-7)
#define K_SHIFTIDX2  (-5)
#define K_PCIDX2     (-3)
#define K_MTDIDX2    (-1)

#define klr_setesp(_ctx, newesp)  ((kcontext_t*)_ctx)->esp = (newesp)
#define klr_setmtdNC(sfpA, mtdO)   sfpA.mtdNC = mtdO

//#define Method_isKonohaCode(mtd) ((mtd)->fcall_1 == Fmethod_runVM)
#define Method_isKonohaCode(mtd) (0)

#define BEGIN_LOCAL(V,N) \
	ksfp_t *V = _ctx->esp, *esp_ = _ctx->esp; (void)V;((kcontext_t*)_ctx)->esp = esp_+N;\

#define END_LOCAL() ((kcontext_t*)_ctx)->esp = esp_;

#define KCALL(LSFP, RIX, MTD, ARGC, DEFVAL) { \
		ksfp_t *tsfp = LSFP + RIX + K_CALLDELTA;\
		tsfp[K_MTDIDX].mtdNC = MTD;\
		tsfp[K_PCIDX].fname = __FILE__;\
		tsfp[K_SHIFTIDX].shift = 0;\
		KSETv(tsfp[K_RTNIDX].o, ((kObject*)DEFVAL));\
		tsfp[K_RTNIDX].uline = __LINE__;\
		klr_setesp(_ctx, tsfp + ARGC + 1);\
		(MTD)->fcall_1(_ctx, tsfp K_RIXPARAM);\
		tsfp[K_MTDIDX].mtdNC = NULL;\
	} \

#define KSELFCALL(TSFP, MTD) { \
		ksfp_t *tsfp = TSFP;\
		tsfp[K_MTDIDX].mtdNC = MTD;\
		(MTD)->fcall_1(_ctx, tsfp K_RIXPARAM);\
		tsfp[K_MTDIDX].mtdNC = NULL;\
	} \


/* ----------------------------------------------------------------------- */
// klib2

struct _kNameSpace;
struct klogconf_t;

typedef const struct _klib2  klib2_t;
struct _klib2 {
	void* (*Kmalloc)(CTX, size_t);
	void* (*Kzmalloc)(CTX, size_t);
	void  (*Kfree)(CTX, void *, size_t);

	void  (*Karray_init)(CTX, karray_t *, size_t);
	void  (*Karray_resize)(CTX, karray_t *, size_t);
	void  (*Karray_expand)(CTX, karray_t *, size_t);
	void  (*Karray_free)(CTX, karray_t *);

	void (*Kwb_init)(karray_t *, kwb_t *);
	void (*Kwb_write)(CTX, kwb_t *, const char *, size_t);
	void (*Kwb_putc)(CTX, kwb_t *, ...);
	void (*Kwb_vprintf)(CTX, kwb_t *, const char *fmt, va_list ap);
	void (*Kwb_printf)(CTX, kwb_t *, const char *fmt, ...);
	const char* (*Kwb_top)(CTX, kwb_t *, int);
	void (*Kwb_free)(kwb_t *);

	kmap_t*  (*Kmap_init)(CTX, size_t);
	kmape_t* (*Kmap_newentry)(CTX, kmap_t *, uintptr_t);
	kmape_t* (*Kmap_get)(kmap_t *, uintptr_t);
//	void (*Kmap_add)(kmap_t *, kmape_t *);
	void (*Kmap_remove)(kmap_t *, kmape_t *);
	void (*Kmap_reftrace)(CTX, kmap_t *, void (*)(CTX, kmape_t*));
	void (*Kmap_free)(CTX, kmap_t *, void (*)(CTX, void *));
	ksymbol_t (*Kmap_getcode)(CTX, kmap_t *, kArray *, const char *, size_t, uintptr_t, int, ksymbol_t);


	kline_t     (*Kfileid)(CTX, const char *, size_t, int spol, ksymbol_t def);
	kpack_t     (*Kpack)(CTX, const char *, size_t, int spol, ksymbol_t def);
	ksymbol_t   (*Ksymbol2)(CTX, const char*, size_t, int spol, ksymbol_t def);

	kbool_t     (*KimportPackage)(CTX, const struct _kNameSpace*, const char *, kline_t);
	kclass_t*   (*Kclass)(CTX, kcid_t, kline_t);
	kString*    (*KCT_shortName)(CTX, kclass_t *ct);
	kclass_t*   (*KCT_Generics)(CTX, kclass_t *ct, ktype_t rty, int psize, kparam_t *p);

	kObject*    (*Knew_Object)(CTX, kclass_t *, void *);
	kObject*    (*Knull)(CTX, kclass_t *);
	kObject*    (*KObject_getObject)(CTX, kObject *, ksymbol_t, kObject *);
	void        (*KObject_setObject)(CTX, kObject *, ksymbol_t, ktype_t, kObject *);
	uintptr_t   (*KObject_getUnboxedValue)(CTX, kObject *, ksymbol_t, uintptr_t);
	void        (*KObject_setUnboxedValue)(CTX, kObject *, ksymbol_t, ktype_t, uintptr_t);
	void        (*KObject_protoEach)(CTX, kObject *, void *thunk, void (*f)(CTX, void *, kvs_t *d));
	void        (*KObject_removeKey)(CTX, kObject *, ksymbol_t);

	kString*    (*Knew_String)(CTX, const char *, size_t, int);
	kString*    (*Knew_Stringf)(CTX, int, const char *, ...);
	kString*    (*KString)(CTX, int, kString *, kString *);

	void (*KArray_add)(CTX, kArray *, kObject *);
	void (*KArray_insert)(CTX, kArray *, size_t, kObject *);
	void (*KArray_clear)(CTX, kArray *, size_t);

//	kParam *   (*Knew_Param)(CTX, ktype_t, int, kparam_t *);
	kMethod *  (*Knew_Method)(CTX, uintptr_t, kcid_t, kmethodn_t, knh_Fmethod);
	kParam*    (*KMethod_setParam)(CTX, kMethod *, ktype_t, int, kparam_t *);
	void       (*KMethod_setFunc)(CTX, kMethod*, knh_Fmethod);
	void       (*KMethod_genCode)(CTX, kMethod*, const struct _kBlock *bk);
	intptr_t   (*KMethod_indexOfField)(kMethod *);

	kbool_t    (*KsetModule)(CTX, int, struct kmodshare_t *, kline_t);
	kclass_t*  (*KaddClassDef)(CTX, kpack_t, kpack_t, kString *, KDEFINE_CLASS *, kline_t);

	kclass_t*  (*KS_getCT)(CTX, const struct _kNameSpace *, kclass_t *, const char *, size_t, kcid_t def);
	void       (*KS_loadMethodData)(CTX, const struct _kNameSpace *, intptr_t *d);
	void       (*KS_loadConstData)(CTX, const struct _kNameSpace *, const char **d, kline_t);
	kMethod*   (*KS_getMethodNULL)(CTX, const struct _kNameSpace *ks, kcid_t cid, kmethodn_t mn);
	kMethod*   (*KS_getGetterMethodNULL)(CTX, const struct _kNameSpace *, ktype_t cid, ksymbol_t sym);

	void       (*KS_syncMethods)(CTX);
	void       (*KCodeGen)(CTX, kMethod *, const struct _kBlock *);
	void       (*Kreportf)(CTX, kinfotag_t, kline_t, const char *fmt, ...);
	void       (*Kraise)(CTX, int isContinue);     // module

	uintptr_t  (*Ktrace)(CTX, struct klogconf_t *logconf, ...);
};

#define K_NULL            (_ctx->share->constNull)
#define K_TRUE            (_ctx->share->constTrue)
#define K_FALSE           (_ctx->share->constFalse)
#define K_NULLPARAM       (_ctx->share->paramList->params[0])
#define K_EMPTYARRAY      (_ctx->share->emptyArray)
#define TS_EMPTY          (_ctx->share->emptyString)

#define UPCAST(o)         ((kObject*)o)
#define W(T, V)           struct _##T*const W##V = (struct _##T*const)(V); int _check##V
#define WASSERT(V)        DBG_ASSERT((void*)V == (void*)W##V); (void)_check##V

#define KPI                     (_ctx->lib2)

#define KMALLOC(size)          (KPI)->Kmalloc(_ctx, size)
#define KCALLOC(size, item)    (KPI)->Kzmalloc(_ctx, ((size) * (item)))
#define KFREE(p, size)         (KPI)->Kfree(_ctx, p, size)

#define KARRAY_INIT(VAR, init)           (KPI)->Karray_init(_ctx, VAR, (init))
#define KARRAY_RESIZE(VAR, newsize)      (KPI)->Karray_resize(_ctx, VAR, (newsize))
#define KARRAY_EXPAND(VAR, min)          (KPI)->Karray_expand(_ctx, VAR, (min))
#define KARRAY_FREE(VAR)                 (KPI)->Karray_free(_ctx, VAR)

#define kwb_init(M,W)            (KPI)->Kwb_init(M,W)
#define kwb_write(W,B,S)         (KPI)->Kwb_write(_ctx,W,B,S)
#define kwb_putc(W,...)          (KPI)->Kwb_putc(_ctx,W, ## __VA_ARGS__, -1)
#define kwb_vprintf(W,FMT,ARG)   (KPI)->Kwb_vprintf(_ctx,W, FMT, ARG)
#define kwb_printf(W,FMT,...)    (KPI)->Kwb_printf(_ctx, W, FMT, ## __VA_ARGS__)

#define kwb_top(W,IS)            (KPI)->Kwb_top(_ctx, W, IS)
#define kwb_bytesize(W)          (((W)->m)->bytesize - (W)->pos)
#define kwb_free(W)              (KPI)->Kwb_free(W)

#define kmap_init(INIT)           (KPI)->Kmap_init(_ctx, INIT)
#define kmap_newentry(M, H)       (KPI)->Kmap_newentry(_ctx, M, H)
#define kmap_get(M, K)            (KPI)->Kmap_get(M, K)
#define kmap_remove(M, E)         (KPI)->Kmap_remove(_ctx, M, E)
#define kmap_reftrace(M, F)       (KPI)->Kmap_reftrace(_ctx, M, F)
#define kmap_free(M, F)           (KPI)->Kmap_free(_ctx, M, F)
#define kmap_getcode(M,L,N,NL,H,POL,DEF)  (KPI)->Kmap_getcode(_ctx, M, L, N, NL, H, POL, DEF)

#define kclass(CID, UL)           (KPI)->Kclass(_ctx, CID, UL)

#define FILEID_NATIVE             0
#define FILEID_(T)                (KPI)->Kfileid(_ctx, T, sizeof(T)-1, SPOL_TEXT|SPOL_ASCII, _NEWID)
#define kfileid(T,L,SPOL,DEF)          (KPI)->Kfileid(_ctx, T, L, SPOL, DEF)
#define PN_konoha                 0
#define PN_sugar                  1
#define PN_(T)                    (KPI)->Kpack(_ctx, T, sizeof(T)-1, SPOL_TEXT|SPOL_ASCII|SPOL_POOL, _NEWID)
#define kpack(T,L,SPOL,DEF)       (KPI)->Kpack(_ctx, T, L, SPOL, DEF)

#define ksymbolA(T, L, DEF)       (KPI)->Ksymbol2(_ctx, T, L, SPOL_ASCII, DEF)
#define ksymbolSPOL(T, L, SPOL, DEF)       (KPI)->Ksymbol2(_ctx, T, L, SPOL, DEF)
#define ksymbol(T)
#define SYM_(T)                   (KPI)->Ksymbol2(_ctx, T, (sizeof(T)-1), SPOL_TEXT|SPOL_ASCII, _NEWID)
#define FN_(T)                    (KPI)->Ksymbol2(_ctx, T, (sizeof(T)-1), SPOL_TEXT|SPOL_ASCII, _NEWID)
#define MN_(T)                    (KPI)->Ksymbol2(_ctx, T, (sizeof(T)-1), SPOL_TEXT|SPOL_ASCII, _NEWID)
#define T_mn(X)                   SYM_PRE(X), SYM_t(X)

// #define KW_new (((ksymbol_t)39)|0) /*new*/
#define MN_new                    39  /* @see */

#define new_kObject(C, A)         (KPI)->Knew_Object(_ctx, C, (void*)(A))
#define new_(C, A)                (k##C*)(KPI)->Knew_Object(_ctx, CT_##C, (void*)(A))
#define new_W(C, A)               (struct _k##C*)(KPI)->Knew_Object(_ctx, CT_##C, (void*)(A))
#define knull(C)                  (KPI)->Knull(_ctx, C)
#define KNULL(C)                  (k##C*)(KPI)->Knull(_ctx, CT_##C)

#define kObject_getObjectNULL(O, K)            (KPI)->KObject_getObject(_ctx, UPCAST(O), K, NULL)
#define kObject_getObject(O, K, DEF)           (KPI)->KObject_getObject(_ctx, UPCAST(O), K, DEF)
#define kObject_setObject(O, K, V)             (KPI)->KObject_setObject(_ctx, UPCAST(O), K, O_cid(V), UPCAST(V))
#define kObject_getUnboxedValue(O, K, DEF)     (KPI)->KObject_getUnboxedValue(_ctx, UPCAST(O), K, DEF)
#define kObject_setUnboxedValue(O, K, T, V)    (KPI)->KObject_setUnboxedValue(_ctx, UPCAST(O), K, T, V)
#define kObject_removeKey(O, K)                (KPI)->KObject_removeKey(_ctx, UPCAST(O), K)
#define kObject_protoEach(O, THUNK, F)         (KPI)->KObject_protoEach(_ctx, UPCAST(O), THUNK, F)


#define new_kString(T,S,P)        (KPI)->Knew_String(_ctx, T, S, P)
#define new_kStringf(P, FMT, ...) (KPI)->Knew_Stringf(_ctx, P, FMT, ## __VA_ARGS__)

#define kArray_size(A)            (((A)->bytesize)/sizeof(void*))
#define kArray_setsize(A, N)  ((struct _kArray*)A)->bytesize = N * sizeof(void*)
#define kArray_add(A, V)          (KPI)->KArray_add(_ctx, A, UPCAST(V))
#define kArray_insert(A, N, V)    (KPI)->KArray_insert(_ctx, A, N, UPCAST(V))
#define kArray_clear(A, S)        (KPI)->KArray_clear(_ctx, A, S)

#define new_kParam(R,S,P)        (KPI)->Knew_Method(_ctx, R, S, P)
#define new_kMethod(F,C,M,FF)  (KPI)->Knew_Method(_ctx, F, C, M, FF)
#define kMethod_setParam(M, R, PSIZE, P)      (KPI)->KMethod_setParam(_ctx, M, R, PSIZE, P)
#define new_kParam2(R, PSIZE, P)  (KPI)->KMethod_setParam(_ctx, NULL, R, PSIZE, P)
#define kMethod_setFunc(M,F)     (KPI)->KMethod_setFunc(_ctx, M, F)
#define kMethod_genCode(M, BLOCK) (KPI)->KMethod_genCode(_ctx, M, BLOCK)

#define KREQUIRE_PACKAGE(NAME, UL)                   (KPI)->KimportPackage(_ctx, NULL, NAME, UL)
#define KEXPORT_PACKAGE(NAME, KS, UL)                (KPI)->KimportPackage(_ctx, KS, NAME, UL)

#define KCLASS(cid)                          S_text(CT(cid)->name)
#define kClassTable_Generics(CT, RTY, PSIZE, P)    (KPI)->KCT_Generics(_ctx, CT, RTY, PSIZE, P)
#define Konoha_setModule(N,D,P)              (KPI)->KsetModule(_ctx, N, D, P)
#define Konoha_addClassDef(PAC, DOM, NAME, DEF, UL)    (KPI)->KaddClassDef(_ctx, PAC, DOM, NAME, DEF, UL)
#define kNameSpace_getCT(NS, THIS, S, L, C)      (KPI)->KS_getCT(_ctx, NS, THIS, S, L, C)
#define kNameSpace_loadMethodData(NS, DEF)       (KPI)->KS_loadMethodData(_ctx, NS, DEF)
#define kNameSpace_loadConstData(KS, DEF, UL)    (KPI)->KS_loadConstData(_ctx, KS, (const char**)&(DEF), UL)
#define kNameSpace_getMethodNULL(KS, CID, MN)    (KPI)->KS_getMethodNULL(_ctx, KS, CID, MN)
#define kNameSpace_syncMethods()    (KPI)->KS_syncMethods(_ctx)

typedef intptr_t  KDEFINE_METHOD;

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

#define kreportf(LEVEL, UL, fmt, ...)  (KPI)->Kreportf(_ctx, LEVEL, UL, fmt, ## __VA_ARGS__)
#define kraise(PARAM)                  (KPI)->Kraise(_ctx, PARAM)

#define KSET_KLIB(T, UL)   do {\
		void *func = _ctx->lib2->K##T;\
		((struct _klib2*)_ctx->lib2)->K##T = K##T;\
		if(func != NULL) {\
			kreportf(DEBUG_, UL, "override of klib2->" #T ", file=%s, line=%d", __FILE__, __LINE__);\
		}\
	}while(0)\

#define KSET_KLIB2(T, F, UL)   do {\
		void *func = _ctx->lib2->K##T;\
		((struct _klib2*)_ctx->lib2)->K##T = F;\
		if(func != NULL) {\
			kreportf(DEBUG_, UL, "override of klib2->" #T ", file=%s, line=%d", __FILE__, __LINE__);\
		}\
	}while(0)\

#define KSET_CLASSFUNC(ct, T, PREFIX, UL)   do {\
		void *func = ct->T;\
		((struct _kclass*)ct)->T = PREFIX##_##T;\
		if(func != NULL) {\
			kreportf(DEBUG_, UL, "override of %s->" #T ", file=%s, line=%d", CT_t(ct), __FILE__, __LINE__);\
		}\
	}while(0)\

// gc

#if defined(_MSC_VER)
#define OBJECT_SET(var, val) do {\
	kObject **var_ = (kObject**)&val; \
	var_[0] = (val_); \
} while (0)
#else
#define OBJECT_SET(var, val) var = (typeof(var))(val)
#endif /* defined(_MSC_VER) */


#define INIT_GCSTACK()         size_t gcstack_ = kArray_size(_ctx->stack->gcstack)
#define PUSH_GCSTACK(o)        kArray_add(_ctx->stack->gcstack, o)
#define RESET_GCSTACK()        kArray_clear(_ctx->stack->gcstack, gcstack_)

#define KINITv(VAR, VAL)   OBJECT_SET(VAR, VAL)
#define KSETv(VAR, VAL)    /*OBJECT_SET(VAR, VAL)*/ VAR = VAL
#define KINITp(parent, v, o) KINITv(v, o)
#define KSETp(parent,  v, o) KSETv(v, o)
#define KUNUSEv(V)  (V)->h.ct->free(_ctx, (V))

#define BEGIN_REFTRACE(SIZE)  int _ref_ = (SIZE); struct _kObject** _tail = KONOHA_reftail(_ctx, (SIZE));
#define END_REFTRACE()        (void)_ref_; _ctx->stack->reftail = _tail;

#define KREFTRACEv(p)  do {\
	DBG_ASSERT(p != NULL);\
	_tail[0] = (struct _kObject*)p;\
	_tail++;\
} while (0)

#define KREFTRACEn(p) do {\
	if(p != NULL) {\
		_tail[0] = (struct _kObject*)p;\
		_tail++;\
	}\
} while (0)

// method macro

#define KNH_SAFEPOINT(_ctx, sfp)

#define RETURN_(vv) do {\
	KSETv(sfp[K_RIX].o, ((kObject*)vv));\
	KNH_SAFEPOINT(_ctx, sfp);\
	return; \
} while (0)

#define RETURNd_(d) do {\
	sfp[K_RIX].ndata = d; \
	return; \
} while (0)

#define RETURNb_(c) do {\
	sfp[K_RIX].bvalue = c; \
	return; \
} while(0)

#define RETURNi_(c) do {\
	sfp[K_RIX].ivalue = c; \
	return; \
} while (0)

#define RETURNf_(c) do {\
	sfp[K_RIX].fvalue = c; \
	return; \
} while (0)

#define RETURNvoid_() do {\
	(void)_rix;\
	return; \
} while (0)

//#ifndef K_NODEBUG
#define KNH_ASSERT(a)    assert(a)
#define DBG_ASSERT(a)    assert(a)
#define TODO_ASSERT(a)   assert(a)
#define DBG_P(fmt, ...)     PLAT dbg_p(__FILE__, __FUNCTION__, __LINE__, fmt, ## __VA_ARGS__)
#define DBG_ABORT(fmt, ...) PLAT dbg_p(__FILE__, __FUNCTION__, __LINE__, fmt, ## __VA_ARGS__); PLAT exit_i(EXIT_FAILURE)
#define DUMP_P(fmt, ...)    PLAT printf_i(fmt, ## __VA_ARGS__)
//#else
//#define KNH_ASSERT(a)
//#define DBG_ASSERT(a)
//#define TODO_ASSERT(a)
//#define DBG_P(fmt, ...)
//#define DBG_ABORT(fmt, ...)
//#define DUMP_P(fmt, ...)
//#endif

#ifndef unlikely
#define unlikely(x)   __builtin_expect(!!(x), 0)
#define likely(x)     __builtin_expect(!!(x), 1)

#endif /*unlikely*/

///* Konoha API */
extern konoha_t konoha_open(const kplatform_t *);
extern void konoha_close(konoha_t konoha);
extern kbool_t konoha_load(konoha_t konoha, const char *scriptfile);
extern kbool_t konoha_eval(konoha_t konoha, const char *script, kline_t uline);
extern kbool_t konoha_run(konoha_t konoha);  // TODO

#ifdef USE_BUILTINTEST
typedef int (*Ftest)(CTX);
typedef struct DEFINE_TESTFUNC {
	const char *name;
	Ftest f;
} DEFINE_TESTFUNC ;
#endif

#include "logger.h"

#endif /* KONOHA2_H_ */
