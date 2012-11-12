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

static void Karray_init(KonohaContext *kctx, KGrowingArray *m, size_t bytemax)
{
	m->bytesize = 0;
	m->bytemax  = bytemax;
	m->bytebuf = (char *)KCalloc_UNTRACE(bytemax, 1);
}

static void Karray_resize(KonohaContext *kctx, KGrowingArray *m, size_t newsize)
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

static void Karray_expand(KonohaContext *kctx, KGrowingArray *m, size_t minsize)
{
	if(m->bytemax == 0) {
		if(minsize > 0) Karray_init(kctx, m, minsize);
	}
	else {
		size_t oldsize = m->bytemax, newsize = oldsize * 2;
		if(minsize > newsize) newsize = minsize;
		Karray_resize(kctx, m, newsize);
	}
}

static void Karray_free(KonohaContext *kctx, KGrowingArray *m)
{
	if(m->bytemax > 0) {
		KFree(m->bytebuf, m->bytemax);
		m->bytebuf = NULL;
		m->bytesize = 0;
		m->bytemax  = 0;
	}
}

static void Kwb_init(KGrowingArray *m, KGrowingBuffer *wb)
{
	wb->m = m;
	wb->pos = m->bytesize;
}

static void Kwb_write(KonohaContext *kctx, KGrowingBuffer *wb, const char *data, size_t bytelen)
{
	KGrowingArray *m = wb->m;
	if(!(m->bytesize + bytelen < m->bytemax)) {
		Karray_expand(kctx, m, m->bytesize + bytelen);
	}
	memcpy(m->bytebuf + m->bytesize, data, bytelen);
	m->bytesize += bytelen;
}

static void Kwb_vprintf(KonohaContext *kctx, KGrowingBuffer *wb, const char *fmt, va_list ap)
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
		Karray_expand(kctx, m, n + 1);
		n = PLATAPI vsnprintf_i(m->bytebuf + s, m->bytemax - s, fmt, ap2);
	}
	va_end(ap2);
	m->bytesize += n;
}

static void Kwb_printf(KonohaContext *kctx, KGrowingBuffer *wb, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	Kwb_vprintf(kctx, wb, fmt, ap);
	va_end(ap);
}

static const char* Kwb_top(KonohaContext *kctx, KGrowingBuffer *wb, int ensureZero)
{
	KGrowingArray *m = wb->m;
	if(ensureZero) {
		if(!(m->bytesize + 1 < m->bytemax)) {
			Karray_expand(kctx, m, m->bytesize + 1);
		}
		m->bytebuf[m->bytesize] = 0;
	}
	return (const char *)m->bytebuf + wb->pos;
}

static void Kwb_free(KGrowingBuffer *wb)
{
	KGrowingArray *m = wb->m;
	bzero(m->bytebuf + wb->pos, m->bytesize - wb->pos);
	m->bytesize = wb->pos;
}

static kbool_t Kwb_iconv(KonohaContext *kctx, KGrowingBuffer* wb, uintptr_t ic, const char *sourceBuf, size_t sourceSize, KTraceInfo *trace)
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
//		size_t iconv_ret = PLATAPI iconv_i(kctx, iconv, inbuf, &inBytesLeft, outbuf, &outBytesLeft, &isTooBig, trace);
		size_t iconv_ret = PLATAPI iconv_i_memcpyStyle(kctx, ic, outbuf, &outBytesLeft, inbuf, &inBytesLeft, &isTooBig, trace);
		size_t processedSize = K_PAGESIZE - outBytesLeft;
		KLIB Kwb_write(kctx, wb, convBuf, processedSize);
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
// KHashMap

#define HMAP_INIT 83

static void kmap_makeFreeList(KHashMap *kmap, size_t s, size_t e)
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

static void kmap_rehash(KonohaContext *kctx, KHashMap *kmap)
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

static void kmap_shiftptr(KHashMap *kmap, intptr_t shift)
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

static KHashMapEntry *Kmap_newEntry(KonohaContext *kctx, KHashMap *kmap, kuint_t hcode)
{
	KHashMapEntry *e;
	if(kmap->unused == NULL) {
		size_t oarenasize = kmap->arenasize;
		char *oarena = (char *)kmap->arena;
		kmap->arenasize *= 2;
		kmap->arena = (KHashMapEntry *)KMalloc_UNTRACE(kmap->arenasize * sizeof(KHashMapEntry));
		memcpy(kmap->arena, oarena, oarenasize * sizeof(KHashMapEntry));
		kmap_shiftptr(kmap, (char *)kmap->arena - oarena);
		kmap_makeFreeList(kmap, oarenasize, kmap->arenasize);
		KFree(oarena, oarenasize * sizeof(KHashMapEntry));
		kmap_rehash(kctx, kmap);
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

static KHashMap *Kmap_init(KonohaContext *kctx, size_t init)
{
	KHashMap *kmap = (KHashMap *)KCalloc_UNTRACE(sizeof(KHashMap), 1);
	if(init < HMAP_INIT) init = HMAP_INIT;
	kmap->arenasize = (init * 3) / 4;
	kmap->arena = (KHashMapEntry *)KMalloc_UNTRACE(kmap->arenasize * sizeof(KHashMapEntry));
	kmap_makeFreeList(kmap, 0, kmap->arenasize);
	kmap->hentry = (KHashMapEntry**)KCalloc_UNTRACE(init, sizeof(KHashMapEntry *));
	kmap->hmax = init;
	kmap->size = 0;
	return (KHashMap *)kmap;
}

static void Kmap_each(KonohaContext *kctx, KHashMap *kmap, void *thunk, void (*f)(KonohaContext *kctx, KHashMapEntry *, void *thunk))
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

static void Kmap_free(KonohaContext *kctx, KHashMap *kmap, void (*f)(KonohaContext *kctx, void *))
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

static KHashMapEntry *Kmap_getentry(KonohaContext *kctx, KHashMap* kmap, kuint_t hcode)
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

static void kmap_unuse(KHashMap *kmap, KHashMapEntry *e)
{
	e->next = kmap->unused;
	kmap->unused = e;
	e->hcode = ((uintptr_t)-1);
	e->unboxValue  = 0;
	kmap->size--;
}

static void Kmap_remove(KHashMap* kmap, KHashMapEntry *oe)
{
	KHashMapEntry **hlist = kmap->hentry;
	size_t idx = oe->hcode % kmap->hmax;
	KHashMapEntry *e = hlist[idx];
	while(e != NULL) {
		if(e->next == oe) {
			e->next = oe->next;
			kmap_unuse(kmap, oe);
			return;
		}
		e = e->next;
	}
	hlist[idx] = oe->next;
	kmap_unuse(kmap, oe);
}

// key management

static void Kmap_addStringUnboxValue(KonohaContext *kctx, KHashMap *kmp, uintptr_t hcode, kString *StringKey, uintptr_t unboxValue)
{
	KHashMapEntry *e = KLIB Kmap_newEntry(kctx, kmp, hcode);
	KUnsafeFieldInit(e->StringKey, StringKey);
	e->unboxValue = unboxValue;
}

static ksymbol_t Kmap_getcode(KonohaContext *kctx, KHashMap *kmp, kArray *list, const char *name, size_t len, uintptr_t hcode, int spol, ksymbol_t def)
{
	KHashMapEntry *e = KLIB Kmap_get(kctx, kmp, hcode);
	while(e != NULL) {
		if(e->hcode == hcode && len == S_size(e->StringKey) && strncmp(S_text(e->StringKey), name, len) == 0) {
			return (ksymbol_t)e->unboxValue;
		}
		e = e->next;
	}
	if(def == SYM_NEWID) {
		uintptr_t sym = kArray_size(list);
		kString *stringKey = KLIB new_kString(kctx, list, name, len, spol);
		Kmap_addStringUnboxValue(kctx, kmp, hcode, stringKey, sym);
		return (ksymbol_t)sym;
	}
	return def;
}

static kfileline_t KfileId(KonohaContext *kctx, const char *name, size_t len, int spol, ksymbol_t def)
{
	uintptr_t hcode = strhash(name, len);
	KLock(kctx->share->filepackMutex);
	kfileline_t uline = Kmap_getcode(kctx, kctx->share->fileIdMap_KeyOnList, kctx->share->fileIdList_OnGlobalConstList, name, len, hcode, spol, def);
	KUnlock(kctx->share->filepackMutex);
	return uline << (sizeof(kshort_t) * 8);
}

static kpackageId_t KpackageId(KonohaContext *kctx, const char *name, size_t len, int spol, ksymbol_t def)
{
	uintptr_t hcode = strhash(name, len);
	KLock(kctx->share->filepackMutex);
	kpackageId_t packid = Kmap_getcode(kctx, kctx->share->packageIdMap_KeyOnList, kctx->share->packageIdList_OnGlobalConstList, name, len, hcode, spol | StringPolicy_ASCII, def);
	KUnlock(kctx->share->filepackMutex);
	return packid;
}

static ksymbol_t Ksymbol(KonohaContext *kctx, const char *name, size_t len, int spol, ksymbol_t def)
{
	ksymbol_t mask = 0;
	int ch0 = name[0], ch1 = name[1];
	if(def != SYM_NEWRAW) {
		if(ch1 == 'e' && name[2] == 't') {
			if(ch0 == 'g' || ch0 == 'G') {
				len -= 3; name += 3;
				mask = MN_GETTER;
			}
			else if(ch0 == 's' || ch0 == 'S') {
				len -= 3; name += 3;
				mask = MN_SETTER;
			}
		}
		else if(ch0 == '@') {
			len -= 1; name += 1;
			mask = MN_Annotation;
		}
		else if(ch0 == '$') {
			len -= 1; name += 1;
			mask = KW_PATTERN; // Pattern
		}
	}
	else {
		def = SYM_NEWID;
	}
	uintptr_t hcode = strhash(name, len);
	KLock(kctx->share->symbolMutex);
	ksymbol_t sym = Kmap_getcode(kctx, kctx->share->symbolMap_KeyOnList, kctx->share->symbolList_OnGlobalConstList, name, len, hcode, spol | StringPolicy_ASCII, def);
	KUnlock(kctx->share->symbolMutex);
	return (sym == def) ? def : (sym | mask);
}

// -------------------------------------------------------------------------
// library

static void kObject_FreeField(KonohaContext *kctx, kObjectVar *o)
{
	KonohaClass *ct = O_ct(o);
	protomap_delete((Kprotomap_t *)o->h.kvproto);
	ct->free(kctx, o);
}

static void kObject_ReftraceField(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	unsigned map_size;
	O_ct(o)->reftrace(kctx, o, visitor);
	map_size = protomap_size((Kprotomap_t *)o->h.kvproto);
	if(map_size) {
		protomap_iterator itr = {0};
		KKeyValue *d;
		BEGIN_REFTRACE(map_size);
		while((d = protomap_next((Kprotomap_t *)o->h.kvproto, &itr)) != NULL) {
			if(Symbol_isBoxedKey(d->key)) {
				KREFTRACEv(d->ObjectValue);
			}
		}
		END_REFTRACE();
	}
}

static kObject* kObject_getObjectNULL(KonohaContext *kctx, kAbstractObject *o, ksymbol_t key, kAbstractObject *defval)
{
	kObject *v = (kObject *)o;
	KKeyValue *d = protomap_get((Kprotomap_t *)v->h.kvproto, key | SYMKEY_BOXED);
	return (d != NULL) ? d->ObjectValue : defval;
}

static void kObject_setObject(KonohaContext *kctx, kAbstractObject *o, ksymbol_t key, ktype_t ty, kAbstractObject *val)
{
	kObjectVar *v = (kObjectVar *)o;
	protomap_set((Kprotomap_t **)&v->h.kvproto, key | SYMKEY_BOXED, ty, (void *)val);
	PLATAPI WriteBarrier(kctx, v);
}

static uintptr_t kObject_getUnboxValue(KonohaContext *kctx, kAbstractObject *o, ksymbol_t key, uintptr_t defval)
{
	kObject *v = (kObject *)o;
	KKeyValue *d = protomap_get((Kprotomap_t *)v->h.kvproto, key);
	return (d != NULL) ? d->unboxValue : defval;
}

static void kObject_setUnboxValue(KonohaContext *kctx, kAbstractObject *o, ksymbol_t key, ktype_t ty, uintptr_t unboxValue)
{
	kObjectVar *v = (kObjectVar *)o;
	PLATAPI WriteBarrier(kctx, v);   // why ? need this? by kimio
	protomap_set((Kprotomap_t **)&v->h.kvproto, key, ty, (void *)unboxValue);
}

static void kObject_removeKey(KonohaContext *kctx, kAbstractObject *o, ksymbol_t key)
{
	kObjectVar *v = (kObjectVar *)o;
	KKeyValue *d = protomap_get((Kprotomap_t *)v->h.kvproto, key | SYMKEY_BOXED);
	if(d != NULL) {
		d->key = 0; d->ty = 0; d->unboxValue = 0;
	}
	d = protomap_get((Kprotomap_t *)v->h.kvproto, key);
	if(d != NULL) {
		d->key = 0; d->ty = 0; d->unboxValue = 0;
	}
}

typedef void (*feach)(KonohaContext *kctx, void *, KKeyValue *d);
static void kObject_protoEach(KonohaContext *kctx, kAbstractObject *o, void *thunk, feach f)
{
	kObjectVar *v = (kObjectVar *)o;
	KKeyValue *r;
	protomap_iterator itr = {0};
	while((r = protomap_next((Kprotomap_t *)v->h.kvproto, &itr)) != NULL) {
		f(kctx, thunk, r);
	}
}

// -------------------------------------------------------------------------

static kbool_t KonohaRuntime_tryCallMethod(KonohaContext *kctx, KonohaStack *sfp)
{
	KonohaStackRuntimeVar *runtime = kctx->stack;
	KonohaStack *bottomStack = runtime->bottomStack;
	jmpbuf_i lbuf = {};
	if(runtime->evaljmpbuf == NULL) {
		KMakeTrace(trace, sfp);
		runtime->evaljmpbuf = (jmpbuf_i *)KCalloc(sizeof(jmpbuf_i), 1, trace);
	}
	memcpy(&lbuf, runtime->evaljmpbuf, sizeof(jmpbuf_i));
	runtime->bottomStack = sfp;
	KUnsafeFieldSet(runtime->OptionalErrorInfo, TS_EMPTY);
	kbool_t result = true;
	int jumpResult;
	INIT_GCSTACK();
	if((jumpResult = PLATAPI setjmp_i(*runtime->evaljmpbuf)) == 0) {
		KonohaRuntime_callMethod(kctx, sfp);
	}
	else {
		PLATAPI ReportCaughtException(kctx, SYM_t(jumpResult), runtime->faultInfo, S_text(runtime->OptionalErrorInfo), runtime->bottomStack, runtime->topStack);
		result = false;
	}
	RESET_GCSTACK();
	runtime->bottomStack = bottomStack;
	memcpy(runtime->evaljmpbuf, &lbuf, sizeof(jmpbuf_i));
	return result;
}

static void KonohaRuntime_raise(KonohaContext *kctx, int symbol, int fault, kString *optionalErrorInfo, KonohaStack *top)
{
	KonohaStackRuntimeVar *runtime = kctx->stack;
	KNH_ASSERT(symbol != 0);
	if(runtime->evaljmpbuf != NULL) {
		runtime->topStack = top;
		runtime->faultInfo = fault;
		if(optionalErrorInfo != NULL) {
			KUnsafeFieldSet(runtime->OptionalErrorInfo, optionalErrorInfo);
		}
		PLATAPI longjmp_i(*runtime->evaljmpbuf, symbol);  // in setjmp 0 means good
	}
	KExit(EXIT_FAILURE);
}


static int DiagnosisFaultType(KonohaContext *kctx, int fault, KTraceInfo *trace)
{
	//DBG_P("IN fault=%d %d,%d,%d,%d", fault, TFLAG_is(int, fault, SoftwareFault), TFLAG_is(int, fault, UserFault), TFLAG_is(int, fault, SystemFault), TFLAG_is(int, fault, ExternalFault));
	if(TFLAG_is(int, fault, SystemError)) {
		fault = PLATAPI DiagnosisSystemError(kctx, fault);
	}
	if(TFLAG_is(int, fault, NotSoftwareFault)) {
		fault ^= SoftwareFault;
	}
	if(TFLAG_is(int, fault, NotUserFault)) {
		fault ^= UserFault;
	}
	if(TFLAG_is(int, fault, NotSystemFault)) {
		fault ^= SystemFault;
	}
	if(TFLAG_is(int, fault, NotExternalFault)) {
		fault ^= ExternalFault;
	}
	if(TFLAG_is(int, fault, SoftwareFault)) {
		if(PLATAPI DiagnosisCheckSoftwareTestIsPass(kctx, FileId_t(trace->pline), (kushort_t)trace->pline)) {
			TFLAG_set(int, fault, SoftwareFault, false);
		}
	}
	return fault;
}


static void CheckSafePoint(KonohaContext *kctx, KonohaStack *sfp, kfileline_t uline)
{
	PLATAPI ScheduleGC(kctx, NULL); // FIXME: NULL
	if(kctx->modshare[MOD_EVENT] != NULL) {
		KLIB KscheduleEvent(kctx);
	}
//	if(PLATAPI ScheduleEvent != NULL) {
//		PLATAPI ScheduleEvent(kctx, NULL); // FIXME: NULL
//	}
}

/* ------------------------------------------------------------------------ */

void TRACE_ReportScriptMessage(KonohaContext *kctx, KTraceInfo *trace, kinfotag_t taglevel, const char *fmt, ...);

static void klib_init(KonohaLibVar *l)
{
	l->Karray_init   = Karray_init;
	l->Karray_resize = Karray_resize;
	l->Karray_expand = Karray_expand;
	l->Karray_free   = Karray_free;
	l->Kwb_init      = Kwb_init;
	l->Kwb_write     = Kwb_write;
	l->Kwb_vprintf   = Kwb_vprintf;
	l->Kwb_printf    = Kwb_printf;
	l->Kwb_top       = Kwb_top;
	l->Kwb_free      = Kwb_free;
	l->Kwb_iconv     = Kwb_iconv;
	l->Kmap_init     = Kmap_init;
	l->Kmap_free     = Kmap_free;
	l->Kmap_each     = Kmap_each;
	l->Kmap_newEntry = Kmap_newEntry;
	l->Kmap_get      = Kmap_getentry;
	l->Kmap_remove   = Kmap_remove;
	l->Kmap_getcode  = Kmap_getcode;
	l->kObject_FreeField     = kObject_FreeField;
	l->kObject_ReftraceField = kObject_ReftraceField;

	l->kObject_getObject     = kObject_getObjectNULL;
	l->kObject_setObject     = kObject_setObject;
	l->kObject_getUnboxValue = kObject_getUnboxValue;
	l->kObject_setUnboxValue = kObject_setUnboxValue;
	l->kObject_removeKey     = kObject_removeKey;
	l->kObject_protoEach     = kObject_protoEach;
	l->KfileId       = KfileId;
	l->KpackageId    = KpackageId;
	l->Ksymbol       = Ksymbol;
	l->KonohaRuntime_tryCallMethod = KonohaRuntime_tryCallMethod;
	l->KonohaRuntime_raise         = KonohaRuntime_raise;
	l->ReportScriptMessage        = TRACE_ReportScriptMessage; /* perror.h */
	l->CheckSafePoint              = CheckSafePoint;
	l->DiagnosisFaultType          = DiagnosisFaultType;
}
