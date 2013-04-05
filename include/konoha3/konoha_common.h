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

#ifdef __cplusplus
extern "C" {
#endif

#define KGetCommonModel()    ((KonohaCommonModel *)kctx->runtimeModels[CommonModelIndex])
#define KDefinedKonohaCommonModel() (kctx->runtimeModels[CommonModelIndex] != NULL)

#define KRequireKonohaCommonModel(TRACE) \
	if(KGetCommonModel() == NULL) {\
		KonohaCommonModel_Init(kctx, TRACE);\
	}\

typedef struct {
	KRuntimeModel h;
	KClass *cPrototype;
	KClass *cFloat;
	KClass *cRegExp;

	KClass *cIterator;
	KClass *cStringIterator;
	KClass *cGenericIterator;

	KClass *cBytes;
	KClass *cFile;
	struct kFileVar *fileStdIn_OnGlobalConstList;
	struct kFileVar *fileStdOut_OnGlobalConstList;
	struct kFileVar *fileStdErr_OnGlobalConstList;
} KonohaCommonModel;


static inline void KonohaCommonModel_Init(KonohaContext *kctx, KTraceInfo *trace)
{
	KonohaCommonModel *base = (KonohaCommonModel *)KCalloc(sizeof(KonohaCommonModel), 1, trace);
	base->h.name      = "KonohaCommon";
	base->h.allocSize = sizeof(KonohaCommonModel);
	KLIB KRuntime_SetModule(kctx, CommonModelIndex, &base->h, trace);
}

/* ------------------------------------------------------------------------ */
/* Bytes */

#define KClass_Bytes     (KGetCommonModel()->cBytes)
#define KType_Bytes      ((KClass_Bytes)->typeId)
#define IS_Bytes(O)      (kObject_class(O) == KClass_Bytes)

/* ------------------------------------------------------------------------ */
/* RegExp */

#define KClass_RegExp     (KGetCommonModel()->cRegExp)
#define KType_RegExp      ((KClass_RegExp)->typeId)
#define IS_RegExp(O)      (kObject_class(O) == KClass_RegExp)

/* ------------------------------------------------------------------------ */
/* Prototype */

#define KClass_Prototype      (KGetCommonModel()->cPrototype)
#define KType_Prototype       ((KClass_Prototype)->typeId)
#define IS_Prototype(O)       (kObject_class(O)->baseTypeId == KType_Prototype)

typedef const struct kPrototypeVar kPrototype;
struct kPrototypeVar {
	kObjectHeader h;
};

/* ------------------------------------------------------------------------ */
/* Float */

#define KClass_Float      (KGetCommonModel()->cFloat)
#define KType_float       ((KClass_Float)->typeId)
#define IS_Float(O)       (kObject_class(O) == KClass_Float)
#define KFLOAT_FMT        "%.4f"  // NEVER CHANGE THIS

typedef const struct kFloatVar kFloat;
struct kFloatVar {
	kObjectHeader h;
	kfloat_t floatValue;
};

/* ------- */
/* Iterator */

#define KClassFlag_Iterator     KClassFlag_Final
#define KClass_Iterator         KGetCommonModel()->cIterator
#define KType_Iterator          KGetCommonModel()->cIterator->typeId
#define KClass_StringIterator   KGetCommonModel()->cStringIterator
#define KType_StringIterator    KGetCommonModel()->cStringIterator->typeId
#define KClass_GenericIterator  KGetCommonModel()->cGenericIterator
#define KType_GenericIterator   KGetCommonModel()->cGenericIterator->typeId

#define IS_Iterator(O)         (kObject_class(O)->baseTypeId == KType_Iterator)

typedef struct kIteratorVar kIterator;
struct kIteratorVar {
	kObjectHeader h;
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

#define KClass_File     (KGetCommonModel()->cFile)
#define KType_File      ((KClass_File)->typeId)
#define IS_File(O)      (kObject_class(O) == KClass_File)
#define KClass_FILE     (KGetCommonModel()->cFile)
#define KType_FILE      ((KClass_File)->typeId)
#define KFileStdIn      KGetCommonModel()->fileStdIn_OnGlobalConstList
#define KFileStdOut     KGetCommonModel()->fileStdOut_OnGlobalConstList
#define KFileStdErr     KGetCommonModel()->fileStdErr_OnGlobalConstList


#define kFileFlag_ChangeLessStream    kObjectFlag_Local1
#define kFile_is(P, o)        (KFlag_Is(uintptr_t,(o)->h.magicflag, kFileFlag_##P))
#define kFile_Set(P, o, b)     KFlag_Set(uintptr_t,(o)->h.magicflag, kFileFlag_##P, b)

typedef struct kFileVar kFile;

struct kFileVar {
	kObjectHeader h;
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
