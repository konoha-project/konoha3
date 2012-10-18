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
#define KGetKonohaCommonContext()   ((KonohaCommonModuleContext *)kctx->mod[MOD_konoha])
#define KDefinedKonohaCommonModule() (kctx->modshare[MOD_float] != NULL)

#define CT_Float          (KGetKonohaCommonModule()->cFloat)
#define TY_float          (CT_Float->typeId)
#define IS_Float(O)       ((O)->h.ct == CT_Float)
#define KFLOAT_FMT        "%.6e"

#define CFLAG_Iterator         kClass_Final
#define CT_Iterator            KGetKonohaCommonModule()->cIterator
#define TY_Iterator            KGetKonohaCommonModule()->cIterator->typeId
#define CT_StringIterator      KGetKonohaCommonModule()->cStringIterator
#define TY_StringIterator      KGetKonohaCommonModule()->cStringIterator->typeId

#define IS_Iterator(O)         (O_ct(O)->baseTypeId == TY_Iterator)

typedef struct {
	KonohaModule h;
	KonohaClass *cFloat;

	KonohaClass *cIterator;
	KonohaClass *cStringIterator;
	KonohaClass *cGenericIterator;

	KonohaClass *cBytes;
	KonohaClass *cFile;
} KonohaCommonModule;

typedef struct {
	KonohaModuleContext h;
} KonohaCommonModuleContext;

typedef const struct kFloatVar kFloat;
struct kFloatVar {
	KonohaObjectHeader h;
	kfloat_t floatValue;
};

typedef struct {
	KonohaModule h;
	KonohaClass *cIterator;
	KonohaClass *cStringIterator;
	KonohaClass *cGenericIterator;
} KonohaIteratorModule;

typedef struct {
	KonohaModuleContext h;
} KonohaIteratorModuleContext;

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

typedef struct kFileVar  kFile;

struct kFileVar {
	KonohaObjectHeader h;
	FILE *fp;
	const char *realpath;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KONOHA_COMMON_H_ */
