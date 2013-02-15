/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
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

KLIBDECL void KArray_Init(KonohaContext *kctx, KGrowingArray *m, size_t bytemax)
{
	m->bytesize = 0;
	m->bytemax  = bytemax;
	m->bytebuf = (char *)KCalloc_UNTRACE(bytemax, 1);
}

KLIBDECL void KArray_Resize(KonohaContext *kctx, KGrowingArray *m, size_t newsize)
{
	size_t oldsize = m->bytemax;
	char *newbody = (char *)KMalloc_UNTRACE(newsize);
	if(oldsize < newsize) {
		memcpy(newbody, m->bytebuf, oldsize);
		bzero(newbody + oldsize, newsize - oldsize);
	}
	else {
		memcpy(newbody, m->bytebuf, newsize);
	}
	KFree(m->bytebuf, oldsize);
	m->bytebuf = newbody;
	m->bytemax = newsize;
}

KLIBDECL void KArray_Expand(KonohaContext *kctx, KGrowingArray *m, size_t minsize)
{
	if(m->bytemax == 0) {
		if(minsize > 0) KArray_Init(kctx, m, minsize);
	}
	else {
		size_t oldsize = m->bytemax, newsize = oldsize * 2;
		if(minsize > newsize) newsize = minsize;
		KArray_Resize(kctx, m, newsize);
	}
}

KLIBDECL void KArray_Free(KonohaContext *kctx, KGrowingArray *m)
{
	if(m->bytemax > 0) {
		KFree(m->bytebuf, m->bytemax);
		m->bytebuf = NULL;
		m->bytesize = 0;
		m->bytemax  = 0;
	}
}

KLIBDECL void KBuffer_Init(KGrowingArray *m, KBuffer *wb)
{
	wb->m = m;
	wb->pos = m->bytesize;
}

KLIBDECL void* KBuffer_Alloca(KonohaContext *kctx, KBuffer *wb, size_t bytelen)
{
	KGrowingArray *m = wb->m;
	if(m->bytesize % sizeof(void *) != 0) {
		m->bytesize = ((m->bytesize / sizeof(void *))+1) * sizeof(void *);
	}
	if(bytelen % sizeof(void *) != 0) {
		bytelen = ((bytelen / sizeof(void *))+1) * sizeof(void *);
	}
	if(!(m->bytesize + bytelen < m->bytemax)) {
		KArray_Expand(kctx, m, m->bytesize + bytelen);
	}
	void *p = m->bytebuf + m->bytesize;
	bzero(p, bytelen);
	m->bytesize += bytelen;
	return p;
}

KLIBDECL void KBuffer_Write(KonohaContext *kctx, KBuffer *wb, const char *data, size_t bytelen)
{
	KGrowingArray *m = wb->m;
	if(!(m->bytesize + bytelen < m->bytemax)) {
		KArray_Expand(kctx, m, m->bytesize + bytelen);
	}
	memcpy(m->bytebuf + m->bytesize, data, bytelen);
	m->bytesize += bytelen;
}

static void KBuffer_vprintf(KonohaContext *kctx, KBuffer *wb, const char *fmt, va_list ap)
{
	va_list ap2;
#ifdef _MSC_VER
	ap2 = ap;
#else
	va_copy(ap2, ap);
#endif
	KGrowingArray *m = wb->m;
	size_t s = m->bytesize;
	size_t n = PLATAPI vsnprintf_i( m->bytebuf + s, m->bytemax - s, fmt, ap);
	if(n >= (m->bytemax - s)) {
		KArray_Expand(kctx, m, n + 1);
		n = PLATAPI vsnprintf_i(m->bytebuf + s, m->bytemax - s, fmt, ap2);
	}
	va_end(ap2);
	m->bytesize += n;
}

KLIBDECL void KBuffer_printf(KonohaContext *kctx, KBuffer *wb, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	KBuffer_vprintf(kctx, wb, fmt, ap);
	va_end(ap);
}

KLIBDECL const char* KBuffer_text(KonohaContext *kctx, KBuffer *wb, int ensureZero)
{
	KGrowingArray *m = wb->m;
	if(ensureZero) {
		if(!(m->bytesize + 1 < m->bytemax)) {
			KArray_Expand(kctx, m, m->bytesize + 1);
		}
		m->bytebuf[m->bytesize] = 0;
	}
	return (const char *)m->bytebuf + wb->pos;
}

KLIBDECL void KBuffer_Free(KBuffer *wb)
{
	KGrowingArray *m = wb->m;
	bzero(m->bytebuf + wb->pos, m->bytesize - wb->pos);
	m->bytesize = wb->pos;
}

KLIBDECL kString* KBuffer_Stringfy(KonohaContext *kctx, KBuffer *wb, kArray *gcstack, int policy)
{
	kString *s = KLIB new_kString(kctx, gcstack, KBuffer_text(kctx, wb, NonZero), KBuffer_bytesize(wb), policy);
	if(KFlag_Is(int, policy, StringPolicy_FreeKBuffer)) {
		KBuffer_Free(wb);
	}
	return s;
}

KLIBDECL kbool_t KBuffer_iconv(KonohaContext *kctx, KBuffer* wb, uintptr_t ic, const char *sourceBuf, size_t sourceSize, KTraceInfo *trace)
{
	char convBuf[K_PAGESIZE];
	char *presentPtrFrom = (char *)sourceBuf;
	char *presentPtrTo = convBuf;
	char ** inbuf = &presentPtrFrom;
	char ** outbuf = &presentPtrTo;
	size_t inBytesLeft = sourceSize, outBytesLeft = K_PAGESIZE;

	while(inBytesLeft > 0) {
		int isTooBig;
		memset(convBuf, '\0', K_PAGESIZE);
		size_t iconv_ret = PLATAPI I18NModule.iconv_i(kctx, ic, inbuf, &inBytesLeft, outbuf, &outBytesLeft, &isTooBig, trace);
		size_t processedSize = K_PAGESIZE - outBytesLeft;
		KLIB KBuffer_Write(kctx, wb, convBuf, processedSize);
		if(isTooBig) {   // input is too big. reset convbuf
			presentPtrTo = convBuf;
			outBytesLeft = K_PAGESIZE;
			continue;
		}
		if(iconv_ret == (size_t)-1) {
			return false;
		}
	} /* end of converting loop */
	return true;
}

// -------------------------------------------------------------------------
/* KDict */

KLIBDECL void KDict_Init(KonohaContext *kctx, KDict *dict)
{
	dict->data.bytemax  = 0;
	dict->data.bytesize = 0;
	dict->sortedData = 0;
}

#define KDict_size(dict)   (dict->data.bytesize / sizeof(KKeyValue))
#define KDict_max(dict)    (dict->data.bytemax / sizeof(KKeyValue))

KLIBDECL KKeyValue* KDict_GetNULL(KonohaContext *kctx, KDict *dict, ksymbol_t queryKey)
{
	size_t min = 0, max = dict->sortedData, size = KDict_size(dict);
	while(min < max) {
		size_t p = (max + min) / 2;
		ksymbol_t key = dict->data.keyValueItems[p].key;
		if(key == queryKey) return dict->data.keyValueItems + p;
		if((int)key < (int)queryKey) {
			min = p + 1;
		}
		else {
			max = p;
		}
	}
	for(min = dict->sortedData; min < size; min++) {
		ksymbol_t key = dict->data.keyValueItems[min].key;
		if(key == queryKey) return dict->data.keyValueItems + min;
	}
	return NULL;
}

static void KDict_Ensure(KonohaContext *kctx, KDict *dict, size_t nitems)
{
	size_t size = KDict_size(dict), max = KDict_max(dict);
	if(max == 0) {
		if(nitems < 4) nitems = 4;
		KLIB KArray_Init(kctx, &dict->data, (nitems) * sizeof(KKeyValue));
	}
	else if(!(size + nitems < max)) {
		KLIB KArray_Resize(kctx, &dict->data, (size + nitems + 8) * sizeof(KKeyValue));
	}
}

static int comprKeyVal(const void *a, const void *b)
{
	int akey = ((KKeyValue *)a)->key;
	int bkey = ((KKeyValue *)b)->key;
	return akey - bkey;
}

static void KDict_Sort(KonohaContext *kctx, KDict *dict)
{
	size_t nitems = KDict_size(dict);
	PLATAPI qsort_i(dict->data.keyValueItems, nitems, sizeof(KKeyValue), comprKeyVal);
	dict->sortedData = nitems;
}

KLIBDECL void KDict_Add(KonohaContext *kctx, KDict *dict, KKeyValue *kvs)
{
	size_t size = KDict_size(dict);
	KDict_Ensure(kctx, dict, 1);
	memcpy(dict->data.keyValueItems + size, kvs, sizeof(KKeyValue));
	dict->data.bytesize += sizeof(KKeyValue);
	if(size - dict->sortedData > 8) {
		KDict_Sort(kctx, dict);
	}
}

KLIBDECL void KDict_Set(KonohaContext *kctx, KDict *dict, KKeyValue *kvs0)
{
	KKeyValue *kvs = KDict_GetNULL(kctx, dict, kvs0->key);
	if(kvs != NULL) {
		//KObjectRefDec(KType_Is(Boxed, kvs->attrTypeId), kvs->ObjectValue);
		kvs->unboxValue = kvs0->unboxValue;
		kvs->attrTypeId = kvs0->attrTypeId;
		return;
	}
	KDict_Add(kctx, dict, kvs0);
}

KLIBDECL void KDict_Remove(KonohaContext *kctx, KDict *dict, ksymbol_t queryKey)
{
	KKeyValue *kvs = KDict_GetNULL(kctx, dict, queryKey);
	if(kvs != NULL) {
		//KObjectRefDec(KType_Is(Boxed, kvs->attrTypeId), kvs->ObjectValue);
		kvs->unboxValue = 0;
		kvs->attrTypeId = 0;
	}
}

KLIBDECL void KDict_MergeData(KonohaContext *kctx, KDict *dict, KKeyValue *kvs, size_t nitems, int isOverride)
{
	size_t i;
	if(KDict_size(dict) == 0) {
		KDict_Ensure(kctx, dict, nitems);
		memcpy(dict->data.keyValueItems, kvs, nitems * sizeof(KKeyValue));
		dict->data.bytesize = nitems * sizeof(KKeyValue);
		KDict_Sort(kctx, dict);
		return;
	}
	kbool_t foundItems[nitems];
	bzero(foundItems, sizeof(kbool_t) * nitems);
	for(i = 0; i < nitems; i++) {
		KKeyValue *stored = KLIB KDict_GetNULL(kctx, dict, kvs[i].key);
		if(stored != NULL) {
			foundItems[i] = true;
			if(isOverride) {
				stored[0] = kvs[i];
			}
		}
	}
	for(i = 0; i < nitems; i++) {
		if(!foundItems[i]) {
			KLIB KDict_Add(kctx, dict, kvs + i);
		}
	}
}

KLIBDECL void KDict_DoEach(KonohaContext *kctx, KDict *dict, void *thunk, void (*f)(KonohaContext*, void *, KKeyValue *))
{
	size_t i, size = KDict_size(dict);
	for(i = 0; i < size; i++) {
		KKeyValue *kvs = dict->data.keyValueItems + i;
		if(kvs->attrTypeId == 0 && kvs->unboxValue == 0) continue;
		f(kctx, thunk, kvs);
	}
}

KLIBDECL void KDict_Reftrace(KonohaContext *kctx, KDict *dict, KObjectVisitor *visitor)
{
	size_t i, size = KDict_size(dict);
	for(i = 0; i < size; i++) {
		if(KTypeAttr_Is(Boxed, dict->data.keyValueItems[i].attrTypeId)) {
			KRefTrace(dict->data.keyValueItems[i].ObjectValue);
		}
	}
}

KLIBDECL void KDict_Free(KonohaContext *kctx, KDict *dict)
{
	if(KDict_max(dict) > 0) {
		KArray_Free(kctx, &(dict->data));
		dict->sortedData = 0;
	}
}

// -------------------------------------------------------------------------
// KHashMap

#define HMAP_INIT 83

static void KHashMap_MakeFreeList(KHashMap *kmap, size_t s, size_t e)
{
	bzero(kmap->arena + s, (e - s) * sizeof(KHashMapEntry));
	kmap->unused = kmap->arena + s;
	size_t i;
	for(i = s; i < e - 1; i++) {
		kmap->arena[i].hcode = ((uintptr_t)-1);
		kmap->arena[i].unboxValue = 0;
		kmap->arena[i].next = kmap->arena + i + 1;
	}
	kmap->arena[e-1].hcode = ((uintptr_t)-1);
	kmap->arena[e-1].unboxValue = 0;
	DBG_ASSERT(kmap->arena[e-1].next == NULL);
}

static void KHashMap_Rehash(KonohaContext *kctx, KHashMap *kmap)
{
	size_t i, newhmax = kmap->hmax * 2 + 1;
	KHashMapEntry **newhentry = (KHashMapEntry**)KCalloc_UNTRACE(newhmax, sizeof(KHashMapEntry *));
	for(i = 0; i < kmap->arenasize / 2; i++) {
		KHashMapEntry *e = kmap->arena + i;
		kuint_t ni = e->hcode % newhmax;
		e->next = newhentry[ni];
		newhentry[ni] = e;
	}
	KFree(kmap->hentry, kmap->hmax * sizeof(KHashMapEntry *));
	kmap->hentry = newhentry;
	kmap->hmax = newhmax;
}

static void KHashMap_ShiftPointer(KHashMap *kmap, intptr_t shift)
{
	size_t i, size = kmap->arenasize / 2;
	for(i = 0; i < size; i++) {
		KHashMapEntry *e = kmap->arena + i;
		if(e->next != NULL) {
			e->next = (KHashMapEntry *)(((char *)e->next) + shift);
			DBG_ASSERT(kmap->arena <= e->next && e->next < kmap->arena + size);
		}
	}
}

static KHashMapEntry *KHashMap_newEntry(KonohaContext *kctx, KHashMap *kmap, kuint_t hcode)
{
	KHashMapEntry *e;
	if(kmap->unused == NULL) {
		size_t oarenasize = kmap->arenasize;
		char *oarena = (char *)kmap->arena;
		kmap->arenasize *= 2;
		kmap->arena = (KHashMapEntry *)KMalloc_UNTRACE(kmap->arenasize * sizeof(KHashMapEntry));
		memcpy(kmap->arena, oarena, oarenasize * sizeof(KHashMapEntry));
		KHashMap_ShiftPointer(kmap, (char *)kmap->arena - oarena);
		KHashMap_MakeFreeList(kmap, oarenasize, kmap->arenasize);
		KFree(oarena, oarenasize * sizeof(KHashMapEntry));
		KHashMap_Rehash(kctx, kmap);
	}
	e = kmap->unused;
	kmap->unused = e->next;
	e->hcode = hcode;
	e->next = NULL;
	kmap->size++;
	{
		KHashMapEntry **hlist = kmap->hentry;
		size_t idx = e->hcode % kmap->hmax;
		e->next = hlist[idx];
		hlist[idx] = e;
	}
	return e;
}

KLIBDECL KHashMap *KHashMap_Init(KonohaContext *kctx, size_t init)
{
	KHashMap *kmap = (KHashMap *)KCalloc_UNTRACE(sizeof(KHashMap), 1);
	if(init < HMAP_INIT) init = HMAP_INIT;
	kmap->arenasize = (init * 3) / 4;
	kmap->arena = (KHashMapEntry *)KMalloc_UNTRACE(kmap->arenasize * sizeof(KHashMapEntry));
	KHashMap_MakeFreeList(kmap, 0, kmap->arenasize);
	kmap->hentry = (KHashMapEntry**)KCalloc_UNTRACE(init, sizeof(KHashMapEntry *));
	kmap->hmax = init;
	kmap->size = 0;
	return (KHashMap *)kmap;
}

KLIBDECL void KHashMap_DoEach(KonohaContext *kctx, KHashMap *kmap, void *thunk, void (*f)(KonohaContext *kctx, KHashMapEntry *, void *thunk))
{
	size_t i;
	for(i = 0; i < kmap->hmax; i++) {
		KHashMapEntry *e = kmap->hentry[i];
		while(e != NULL) {
			f(kctx, e, thunk);
			e = e->next;
		}
	}
}

KLIBDECL void KHashMap_Free(KonohaContext *kctx, KHashMap *kmap, void (*f)(KonohaContext *kctx, void *))
{
	if(f != NULL) {
		size_t i;
		for(i = 0; i < kmap->hmax; i++) {
			KHashMapEntry *e = kmap->hentry[i];
			while(e != NULL) {
				f(kctx, e->ptrValue);
				e = e->next;
			}
		}
	}
	KFree(kmap->arena, sizeof(KHashMapEntry)*(kmap->arenasize));
	KFree(kmap->hentry, sizeof(KHashMapEntry *)*(kmap->hmax));
	KFree(kmap, sizeof(KHashMap));
}

static KHashMapEntry *KHashMap_getentry(KonohaContext *kctx, KHashMap* kmap, kuint_t hcode)
{
	KHashMapEntry **hlist = kmap->hentry;
	size_t idx = hcode % kmap->hmax;
	KHashMapEntry *e = hlist[idx];
	while(e != NULL) {
		if(e->hcode == hcode) return e;
		e = e->next;
	}
	return NULL;
}

static void KHashMap_Unuse(KHashMap *kmap, KHashMapEntry *e)
{
	e->next = kmap->unused;
	kmap->unused = e;
	e->hcode = ((uintptr_t)-1);
	e->unboxValue  = 0;
	kmap->size--;
}

static void KHashMap_Remove(KHashMap* kmap, KHashMapEntry *oe)
{
	KHashMapEntry **hlist = kmap->hentry;
	size_t idx = oe->hcode % kmap->hmax;
	KHashMapEntry *e = hlist[idx];
	while(e != NULL) {
		if(e->next == oe) {
			e->next = oe->next;
			KHashMap_Unuse(kmap, oe);
			return;
		}
		e = e->next;
	}
	hlist[idx] = oe->next;
	KHashMap_Unuse(kmap, oe);
}

// key management

static void KHashMap_AddStringUnboxValue(KonohaContext *kctx, KHashMap *kmp, uintptr_t hcode, kString *StringKey, uintptr_t unboxValue)
{
	KHashMapEntry *e = KLIB KHashMap_newEntry(kctx, kmp, hcode);
	KUnsafeFieldInit(e->StringKey, StringKey);
	e->unboxValue = unboxValue;
}

static ksymbol_t KHashMap_getcode(KonohaContext *kctx, KHashMap *kmp, kArray *list, const char *name, size_t len, uintptr_t hcode, int spol, ksymbol_t def)
{
	KHashMapEntry *e = KLIB KHashMap_get(kctx, kmp, hcode);
	while(e != NULL) {
		if(e->hcode == hcode && len == kString_size(e->StringKey) && strncmp(kString_text(e->StringKey), name, len) == 0) {
			return (ksymbol_t)e->unboxValue;
		}
		e = e->next;
	}
	if(def == KSymbol_NewId) {
		uintptr_t sym = kArray_size(list);
		kString *stringKey = KLIB new_kString(kctx, list, name, len, spol);
		KHashMap_AddStringUnboxValue(kctx, kmp, hcode, stringKey, sym);
		return (ksymbol_t)sym;
	}
	return def;
}

static kfileline_t KfileId(KonohaContext *kctx, const char *name, size_t len, int spol, ksymbol_t def)
{
	uintptr_t hcode = strhash(name, len);
	KLock(kctx->share->filepackMutex);
	kfileline_t uline = KHashMap_getcode(kctx, kctx->share->fileIdMap_KeyOnList, kctx->share->fileIdList, name, len, hcode, spol, def);
	KUnlock(kctx->share->filepackMutex);
	return uline << (sizeof(kshort_t) * 8);
}

static kpackageId_t KpackageId(KonohaContext *kctx, const char *name, size_t len, int spol, ksymbol_t def)
{
	uintptr_t hcode = strhash(name, len);
	KLock(kctx->share->filepackMutex);
	kpackageId_t packid = KHashMap_getcode(kctx, kctx->share->packageIdMap_KeyOnList, kctx->share->packageIdList, name, len, hcode, spol | StringPolicy_ASCII, def);
	KUnlock(kctx->share->filepackMutex);
	return packid;
}

static ksymbol_t Ksymbol(KonohaContext *kctx, const char *name, size_t len, int spol, ksymbol_t def)
{
	ksymbol_t mask = 0;
	int ch0 = name[0], ch1 = name[1];
	if(def != KSymbol_NewRaw) {
		if(ch1 == 'e' && name[2] == 't') {
			if(ch0 == 'g' || ch0 == 'G') {
				len -= 3; name += 3;
				mask = KMethodNameAttr_Getter;
			}
			else if(ch0 == 's' || ch0 == 'S') {
				len -= 3; name += 3;
				mask = KMethodNameAttr_Setter;
			}
		}
		else if(ch0 == '@') {
			len -= 1; name += 1;
			mask = KSymbolAttr_Annotation;
		}
		else if(ch0 == '$') {
			len -= 1; name += 1;
			mask = KSymbolAttr_Pattern; // Pattern
		}
	}
	else {
		def = KSymbol_NewId;
	}
	uintptr_t hcode = strhash(name, len);
	KLock(kctx->share->symbolMutex);
	ksymbol_t sym = KHashMap_getcode(kctx, kctx->share->symbolMap_KeyOnList, kctx->share->symbolList, name, len, hcode, spol | StringPolicy_ASCII, def);
	KUnlock(kctx->share->symbolMutex);
	return (sym == def) ? def : (sym | mask);
}

//---------------------------------------------------------------------------

#define KGetProtoMap(o)     ((o)->h.prototypePtr)
#define KSetProtoMap(o, p)   (o)->h.prototypePtr = p

static KProtoMap* new_KProtoMap(KonohaContext *kctx)
{
	KProtoMap* p = (KProtoMap *)KCalloc_UNTRACE(1, sizeof(KProtoMap));
	p->refc = 1;
	KLIB KArray_Init(kctx, &(p->dict.data), sizeof(KKeyValue) * 4);
	return p;
}

static void KProtoMap_Free(KonohaContext *kctx, KProtoMap* p)
{
	p->refc--;
	if(p->refc == 0) {
		KDict_Free(kctx, &(p->dict));
		KFree(p, sizeof(KProtoMap));
	}
}

static void kObjectProto_Free(KonohaContext *kctx, kObjectVar *o)
{
	KClass *ct = kObject_class(o);
	ct->free(kctx, o);
	if(KGetProtoMap(o) != NULL) {
		KProtoMap_Free(kctx, KGetProtoMap(o));
		KSetProtoMap(o, NULL);
	}
}

static void kObjectProto_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kObject_class(o)->reftrace(kctx, o, visitor);
	KProtoMap *pm = KGetProtoMap(o);
	if(pm != NULL) {
		KDict_Reftrace(kctx, &(pm->dict), visitor);
	}
}

static KKeyValue* kObjectProto_GetKeyValue(KonohaContext *kctx, kAbstractObject *ao, ksymbol_t key)
{
	KProtoMap *pm = KGetProtoMap((kObject *)ao);
	while(pm != NULL) {
		KDict *dict = &(pm->dict);
		KKeyValue *kvs = KDict_GetNULL(kctx, dict, key);
		if(kvs != NULL && kvs->attrTypeId != 0) return kvs;
		pm = pm->parent;
	}
	return NULL;
}

static kObject* kObjectProto_GetObject(KonohaContext *kctx, kAbstractObject *o, ksymbol_t key, kAbstractObject *defval)
{
	KKeyValue *d = kObjectProto_GetKeyValue(kctx, o, key);
	return (d != NULL) ? d->ObjectValue : (kObject *) defval;
}

static void kObjectProto_SetObject(KonohaContext *kctx, kAbstractObject *ao, ksymbol_t key, ktypeattr_t ty, kAbstractObject *val)
{
	kObjectVar *o = (kObjectVar *)ao;
	KKeyValue kvs = {key, ty | KTypeAttr_Boxed, {(uintptr_t)val}};
	if(KGetProtoMap(o) == NULL) {
		KSetProtoMap(o, new_KProtoMap(kctx));
	}
	KDict *dict = &(KGetProtoMap(o)->dict);
	KDict_Set(kctx, dict, &kvs);
	PLATAPI GCModule.WriteBarrier(kctx, o);
}

static void kObjectProto_SetUnboxValue(KonohaContext *kctx, kAbstractObject *ao, ksymbol_t key, ktypeattr_t ty, uintptr_t unboxValue)
{
	kObjectVar *o = (kObjectVar *)ao;
	KKeyValue kvs = {key, ty, {unboxValue}};
	if(KGetProtoMap(o) == NULL) {
		KSetProtoMap(o, new_KProtoMap(kctx));
	}
	KDict *dict = &(KGetProtoMap(o)->dict);
	KDict_Set(kctx, dict, &kvs);
}

static void kObjectProto_RemoveKey(KonohaContext *kctx, kAbstractObject *ao, ksymbol_t key)
{
	KProtoMap *pm = KGetProtoMap((kObjectVar *)ao);
	if(pm != NULL) {
		KDict_Remove(kctx, &(pm->dict), key);
	}
}

static void kObjectProto_DoEach(KonohaContext *kctx, kAbstractObject *ao, void *thunk, void (*feach)(KonohaContext *kctx, void *, KKeyValue *d))
{
	kObjectVar *o = (kObjectVar *)ao;
	KProtoMap *pm = KGetProtoMap(o);
	if(pm != NULL) {
		KDict_DoEach(kctx, &(pm->dict), thunk, feach);
	}
}

struct wbenv {
	KonohaValue *values;
	KBuffer *wb;
	int pos;
	int count;
};

static void dumpProto(KonohaContext *kctx, void *arg, KKeyValue *d)
{
	struct wbenv *w = (struct wbenv *)arg;
	ksymbol_t key = d->key;
	if(w->count > 0) {
		KLIB KBuffer_Write(kctx, w->wb, ", ", 2);
	}
	KLIB KBuffer_printf(kctx, w->wb, "%s%s: (%s)", KSymbol_Fmt2(key), KType_text(d->attrTypeId));
	if(KTypeAttr_Is(Boxed, d->attrTypeId)) {
		KUnsafeFieldSet(w->values[w->pos].asObject, d->ObjectValue);
	}
	else {
		w->values[w->pos].unboxValue = d->unboxValue;
	}
	w->count++;
	int i;
	for(i = 0; i < w->pos; i++) {
		if(w->values[i].asObject == d->ObjectValue) {
			KLIB KBuffer_Write(kctx, w->wb, "...", 3);
			return;
		}
	}
	KClass_(KTypeAttr_Unmask(d->attrTypeId))->format(kctx, w->values, w->pos, w->wb);
}

static int kObjectProto_format(KonohaContext *kctx, KonohaValue *values, int pos, KBuffer *wb, int count)
{
	struct wbenv w = {values, wb, pos+1, count};
	KLIB kObjectProto_DoEach(kctx, values[pos].asObject, &w, dumpProto);
	return w.count;
}

static void DumpObject(KonohaContext *kctx, kObject *o, const char *file, const char *func, int line)
{
	KBuffer wb;
	KonohaStack *lsfp = kctx->esp;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	KUnsafeFieldSet(lsfp[0].asObject, o);
	kObject_class(o)->format(kctx, lsfp, 0, &wb);
	const char *msg = KLIB KBuffer_text(kctx, &wb, EnsureZero);
	if(file == NULL) {
		PLATAPI printf_i("(%s)%s\n", KClass_text(kObject_class(o)), msg);
	}
	else {
		PLATAPI ConsoleModule.ReportDebugMessage(file, func, line, "(%s)%s", KClass_text(kObject_class(o)), msg);
	}
	KLIB KBuffer_Free(&wb);
}

// -------------------------------------------------------------------------

static kbool_t KRuntime_tryCallMethod(KonohaContext *kctx, KonohaStack *sfp)
{
	KRuntimeContextVar *runtime = kctx->stack;
	KonohaStack *bottomStack = runtime->bottomStack;
	jmpbuf_i lbuf = {};
	if(runtime->evaljmpbuf == NULL) {
		KMakeTrace(trace, sfp);
		runtime->evaljmpbuf = (jmpbuf_i *)KCalloc(sizeof(jmpbuf_i), 1, trace);
	}
	memcpy(&lbuf, runtime->evaljmpbuf, sizeof(jmpbuf_i));
	runtime->bottomStack = sfp;
	KUnsafeFieldSet(runtime->ThrownException, (kException *)K_NULL);
	kbool_t result = true;
	int jumpResult;
	INIT_GCSTACK();
	if((jumpResult = PLATAPI setjmp_i(*runtime->evaljmpbuf)) == 0) {
		KStackCall(sfp);
	}
	else {
		PLATAPI ConsoleModule.ReportCaughtException(kctx, runtime->ThrownException, runtime->bottomStack, runtime->topStack);
		result = false;
	}
	RESET_GCSTACK();
	runtime->bottomStack = bottomStack;
	memcpy(runtime->evaljmpbuf, &lbuf, sizeof(jmpbuf_i));
	return result;
}

static void KRuntime_raise(KonohaContext *kctx, int symbol, int fault, kString *optionalErrorInfo, KonohaStack *top)
{
	KRuntimeContextVar *runtime = kctx->stack;
	KNH_ASSERT(symbol != 0);
	if(runtime->evaljmpbuf != NULL) {
		kException *e = new_(Exception, optionalErrorInfo, NULL);
		e->symbol = symbol;
		e->fault  = fault;
		runtime->topStack = top;
		//runtime->faultInfo = fault;
//		if(optionalErrorInfo != NULL) {
		KUnsafeFieldSet(runtime->ThrownException, e);
//		}
		PLATAPI longjmp_i(*runtime->evaljmpbuf, symbol);  // in setjmp 0 means good
	}
	KExit(EXIT_FAILURE);
}

static void PushParam(KonohaContext *kctx, KonohaStack *sfp, const char *fmt, va_list ap)
{
	switch(fmt[0]) {
	case 'u': case 'i': case 'd':
		sfp[0].unboxValue = (uintptr_t)va_arg(ap, uintptr_t);
		break;
	case 'O':
		KUnsafeFieldSet(sfp[0].asObject, (kObject *)va_arg(ap, kObject *));
		break;
	}
}

static uintptr_t ApplySystemFunc(KonohaContext *kctx, uintptr_t defval, const char *name, const char *param, ...)
{
	ksymbol_t mn = KLIB Ksymbol(kctx, name, strlen(name), StringPolicy_TEXT, KSymbol_NewId);
	int i, psize = param == NULL ? 0 : strlen(param) / 2;
	kNameSpace *ns = KNULL(NameSpace);
	kMethod *mtd = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, kObject_class(ns), mn, psize, KMethodMatch_NoOption);
	if(mtd != NULL) {
		KClass *returnType = kMethod_GetReturnType(mtd);
		BEGIN_UnusedStack(lsfp);
		KUnsafeFieldSet(lsfp[0].asNameSpace, ns);
		for(i = 0; i < psize; i++) {
			va_list ap;
			va_start(ap, param);
			PushParam(kctx, lsfp + i + 1, param + (i * 2) + 1, ap);
			va_end(ap);
		}
		KStackSetMethodAll(lsfp, KLIB Knull(kctx, returnType), 0, mtd, psize);
		KStackCall(lsfp);
		END_UnusedStack();
		if(KClass_Is(UnboxType, returnType)) {
			return lsfp[K_RTNIDX].unboxValue;
		}
		else {
			return (uintptr_t)lsfp[K_RTNIDX].asObject;
		}
	}
	return defval;
}

static int DiagnosisFaultType(KonohaContext *kctx, int fault, KTraceInfo *trace)
{
	//DBG_P("IN fault=%d %d,%d,%d,%d", fault, KFlag_Is(int, fault, SoftwareFault), KFlag_Is(int, fault, UserFault), KFlag_Is(int, fault, SystemFault), KFlag_Is(int, fault, ExternalFault));
	if(KFlag_Is(int, fault, SystemError)) {
		fault = PLATAPI DiagnosisModule.DiagnosisSystemError(kctx, fault);
	}
	if(KFlag_Is(int, fault, NotSoftwareFault)) {
		fault ^= SoftwareFault;
	}
	if(KFlag_Is(int, fault, NotUserFault)) {
		fault ^= UserFault;
	}
	if(KFlag_Is(int, fault, NotSystemFault)) {
		fault ^= SystemFault;
	}
	if(KFlag_Is(int, fault, NotExternalFault)) {
		fault ^= ExternalFault;
	}
	if(KFlag_Is(int, fault, SoftwareFault)) {
		if(PLATAPI DiagnosisModule.DiagnosisCheckSoftwareTestIsPass(kctx, KFileLine_textFileName(trace->pline), (kushort_t)trace->pline)) {
			KFlag_Set(int, fault, SoftwareFault, false);
		}
	}
	return fault;
}


static void CheckSafePoint(KonohaContext *kctx, KonohaStack *sfp, kfileline_t uline)
{
	PLATAPI GCModule.ScheduleGC(kctx, NULL); // FIXME: NULL
	if(kctx->modshare[MOD_EVENT] != NULL) {
		KLIB KscheduleEvent(kctx);
	}
//	if(PLATAPI ScheduleEvent != NULL) {
//		PLATAPI ScheduleEvent(kctx, NULL); // FIXME: NULL
//	}
}

/* ------------------------------------------------------------------------ */

void TRACE_ReportScriptMessage(KonohaContext *kctx, KTraceInfo *trace, kinfotag_t taglevel, const char *fmt, ...);

static void klib_Init(KonohaLibVar *l)
{
	l->KArray_Init       = KArray_Init;
	l->KArray_Resize     = KArray_Resize;
	l->KArray_Expand     = KArray_Expand;
	l->KArray_Free       = KArray_Free;
	l->KBuffer_Init      = KBuffer_Init;
	l->KBuffer_Alloca    = KBuffer_Alloca;
	l->KBuffer_Write     = KBuffer_Write;
	l->KBuffer_vprintf   = KBuffer_vprintf;
	l->KBuffer_printf    = KBuffer_printf;
	l->KBuffer_text      = KBuffer_text;
	l->KBuffer_Free      = KBuffer_Free;
	l->KBuffer_Stringfy  = KBuffer_Stringfy;
	l->KBuffer_iconv     = KBuffer_iconv;

	l->KDict_Init        = KDict_Init;
	l->KDict_GetNULL     = KDict_GetNULL;
	l->KDict_Add         = KDict_Add;
	l->KDict_Remove      = KDict_Remove;
	l->KDict_Set         = KDict_Set;
	l->KDict_MergeData   = KDict_MergeData;
	l->KDict_DoEach      = KDict_DoEach;
	l->KDict_Reftrace    = KDict_Reftrace;
	l->KDict_Free        = KDict_Free;

	l->KHashMap_Init     = KHashMap_Init;
	l->KHashMap_Free     = KHashMap_Free;
	l->KHashMap_DoEach   = KHashMap_DoEach;
	l->KHashMap_newEntry = KHashMap_newEntry;
	l->KHashMap_get      = KHashMap_getentry;
	l->KHashMap_Remove   = KHashMap_Remove;
	l->KHashMap_getcode  = KHashMap_getcode;
	l->kObjectProto_Free     = kObjectProto_Free;
	l->kObjectProto_Reftrace = kObjectProto_Reftrace;

	l->kObject_getObject          = kObjectProto_GetObject;
	l->kObjectProto_SetObject     = kObjectProto_SetObject;
	l->kObjectProto_GetKeyValue   = kObjectProto_GetKeyValue;
	l->kObjectProto_SetUnboxValue = kObjectProto_SetUnboxValue;
	l->kObjectProto_RemoveKey     = kObjectProto_RemoveKey;
	l->kObjectProto_DoEach    = kObjectProto_DoEach;
	l->kObjectProto_format    = kObjectProto_format;
	l->DumpObject             = DumpObject;
	l->KfileId                = KfileId;
	l->KpackageId             = KpackageId;
	l->Ksymbol                = Ksymbol;
	l->KRuntime_tryCallMethod = KRuntime_tryCallMethod;
	l->ApplySystemFunc        = ApplySystemFunc;

	l->KRuntime_raise         = KRuntime_raise;
	l->ReportScriptMessage    = TRACE_ReportScriptMessage; /* perror.h */
	l->CheckSafePoint         = CheckSafePoint;
	l->DiagnosisFaultType     = DiagnosisFaultType;
}
