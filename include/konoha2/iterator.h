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

#ifndef MODITERATOR_H_
#define MODITERATOR_H_

#define kiteratormod           ((kiteratormod_t*)_ctx->mod[MOD_iterator])
#define kmoditerator           ((kmoditerator_t*)_ctx->modshare[MOD_iterator])
#define IS_defineIterator()    (_ctx->modshare[MOD_iterator] != NULL)

#define CFLAG_Iterator         kClass_Final
#define CT_Iterator            kmoditerator->cIterator
#define TY_Iterator            kmoditerator->cIterator->cid
#define CT_StringIterator      kmoditerator->cStringIterator
#define TY_StringIterator      kmoditerator->cStringIterator->cid

#define IS_Iterator(O)         (O_ct(O)->bcid == TY_Iterator)

typedef struct {
	kmodshare_t h;
	kclass_t *cIterator;
	kclass_t *cStringIterator;
	kclass_t *cGenericIterator;
} kmoditerator_t;

typedef struct {
	kmodlocal_t h;
} kiteratormod_t;

typedef struct _kIterator kIterator;

struct _kIterator {
	kObjectHeader h;
	kbool_t (*hasNext)(CTX, ksfp_t *);
	void (*setNextResult)(CTX, ksfp_t* _RIX);
	size_t current_pos;
	union {
		kObject  *source;
		kArray   *arrayList;
		kFunc    *funcHasNext;
	};
	kFunc        *funcNext;
};

#endif /* MODITERATOR_H_ */
