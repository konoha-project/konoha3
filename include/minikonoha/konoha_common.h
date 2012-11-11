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

#ifndef KONOHA_COMMON_H_
#define KONOHA_COMMON_H_

#ifndef MINIOKNOHA_H_
#error Do not include float.h without minikonoha.h.
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define KGetKonohaCommonModule()    ((KonohaCommonModule *)kctx->modshare[MOD_konoha])
#define KDefinedKonohaCommonModule() (kctx->modshare[MOD_konoha] != NULL)

#define KRequireKonohaCommonModule(TRACE) \
	if(KGetKonohaCommonModule() == NULL) {\
		KonohaCommonModule_init(kctx, TRACE);\
	}\

typedef struct {
	KonohaModule h;
	KonohaClass *cFloat;
	KonohaClass *cRegExp;

	KonohaClass *cIterator;
	KonohaClass *cStringIterator;
	KonohaClass *cGenericIterator;

	KonohaClass *cBytes;
	KonohaClass *cFile;
	struct kFileVar       *fileStdIn_OnGlobalConstList;
	struct kFileVar       *fileStdOut_OnGlobalConstList;
	struct kFileVar       *fileStdErr_OnGlobalConstList;
} KonohaCommonModule;


static void KonohaCommonModule_init(KonohaContext *kctx, KTraceInfo *trace)
{
	KonohaCommonModule *base = (KonohaCommonModule *)KCalloc(sizeof(KonohaCommonModule), 1, trace);
	base->h.name      = "KonohaCommon";
	base->h.allocSize = sizeof(KonohaCommonModule);
	KLIB KonohaRuntime_setModule(kctx, MOD_konoha, &base->h, trace);
}

/* ------------------------------------------------------------------------ */
/* Bytes */

#define CT_Bytes         (KGetKonohaCommonModule()->cBytes)
#define TY_Bytes         ((CT_Bytes)->typeId)
#define IS_Bytes(O)      (O_ct(O) == CT_Bytes)

/* ------------------------------------------------------------------------ */
/* RegExp */

#define CT_RegExp         (KGetKonohaCommonModule()->cRegExp)
#define TY_RegExp         ((CT_RegExp)->typeId)
#define IS_RegExp(O)      (O_ct(O) == CT_RegExp)


/* ------------------------------------------------------------------------ */
/* Float */

#define CT_Float          (KGetKonohaCommonModule()->cFloat)
#define TY_float          ((CT_Float)->typeId)
#define IS_Float(O)       (O_ct(O) == CT_Float)
#define KFLOAT_FMT        "%.4f"  // NEVER CHANGE THIS

typedef const struct kFloatVar kFloat;
struct kFloatVar {
	KonohaObjectHeader h;
	kfloat_t floatValue;
};

/* ------- */
/* Iterator */

#define CFLAG_Iterator         kClass_Final
#define CT_Iterator            KGetKonohaCommonModule()->cIterator
#define TY_Iterator            KGetKonohaCommonModule()->cIterator->typeId
#define CT_StringIterator      KGetKonohaCommonModule()->cStringIterator
#define TY_StringIterator      KGetKonohaCommonModule()->cStringIterator->typeId
#define CT_GenericIterator      KGetKonohaCommonModule()->cGenericIterator
#define TY_GenericIterator      KGetKonohaCommonModule()->cGenericIterator->typeId

#define IS_Iterator(O)         (O_ct(O)->baseTypeId == TY_Iterator)

typedef struct kIteratorVar kIterator;
struct kIteratorVar {
	KonohaObjectHeader h;
	kbool_t (*hasNext)(KonohaContext *kctx, KonohaStack *);
	void    (*setNextResult)(KonohaContext *kctx, KonohaStack *);
	size_t current_pos;
	union {
		kObject  *source;
		kArray   *arrayList;
		kFunc    *funcHasNext;
	};
	kFunc        *funcNext;
};

/* .... */

#define CT_File         KGetKonohaCommonModule()->cFile
#define TY_File         (CT_File)->typeId
#define IS_File(O)      (O_ct(O) == CT_File)
#define CT_FILE         KGetKonohaCommonModule()->cFile
#define TY_FILE         (CT_File)->typeId
#define KFileStdIn       KGetKonohaCommonModule()->fileStdIn_OnGlobalConstList
#define KFileStdOut      KGetKonohaCommonModule()->fileStdOut_OnGlobalConstList
#define KFileStdErr      KGetKonohaCommonModule()->fileStdErr_OnGlobalConstList


#define kFileFlag_ChangeLessStream    kObject_Local1
#define kFile_is(P, o)        (TFLAG_is(uintptr_t,(o)->h.magicflag, kFileFlag_##P))
#define kFile_set(P, o, b)     TFLAG_set(uintptr_t,(o)->h.magicflag, kFileFlag_##P, b)

typedef struct kFileVar kFile;

struct kFileVar {
	KonohaObjectHeader h;
#ifdef USE_FILE
	FILE *fp;
#else
	void *fp;
#endif/*USE_FILE*/
	kString *PathInfoNULL;
	uintptr_t readerIconv;
	uintptr_t writerIconv;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KONOHA_COMMON_H_ */
