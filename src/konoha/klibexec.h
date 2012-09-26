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

static void Karray_init(KonohaContext *kctx, KUtilsGrowingArray *m, size_t bytemax)
{
	m->bytesize = 0;
	m->bytemax  = bytemax;
	m->bytebuf = (char*)KCALLOC(bytemax, 1);
}

static void Karray_resize(KonohaContext *kctx, KUtilsGrowingArray *m, size_t newsize)
{
	size_t oldsize = m->bytemax;
	char *newbody = (char*)KMALLOC(newsize);
	if(oldsize < newsize) {
		memcpy(newbody, m->bytebuf, oldsize);
		bzero(newbody + oldsize, newsize - oldsize);
	}
	else {
		memcpy(newbody, m->bytebuf, newsize);
	}
	KFREE(m->bytebuf, oldsize);
	m->bytebuf = newbody;
	m->bytemax = newsize;
}

static void Karray_expand(KonohaContext *kctx, KUtilsGrowingArray *m, size_t minsize)
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

static void Karray_free(KonohaContext *kctx, KUtilsGrowingArray *m)
{
	if(m->bytemax > 0) {
		KFREE(m->bytebuf, m->bytemax);
		m->bytebuf = NULL;
		m->bytesize = 0;
		m->bytemax  = 0;
	}
}

static void Kwb_init(KUtilsGrowingArray *m, KUtilsWriteBuffer *wb)
{
	wb->m = m;
	wb->pos = m->bytesize;
}

static void Kwb_write(KonohaContext *kctx, KUtilsWriteBuffer *wb, const char *data, size_t bytelen)
{
	KUtilsGrowingArray *m = wb->m;
	if(!(m->bytesize + bytelen < m->bytemax)) {
		Karray_expand(kctx, m, m->bytesize + bytelen);
	}
	memcpy(m->bytebuf + m->bytesize, data, bytelen);
	m->bytesize += bytelen;
}

static void Kwb_vprintf(KonohaContext *kctx, KUtilsWriteBuffer *wb, const char *fmt, va_list ap)
{
	va_list ap2;
	va_copy(ap2, ap);
	KUtilsGrowingArray *m = wb->m;
	size_t s = m->bytesize;
	size_t n = PLATAPI vsnprintf_i( m->bytebuf + s, m->bytemax - s, fmt, ap);
	if(n >= (m->bytemax - s)) {
		Karray_expand(kctx, m, n + 1);
		n = PLATAPI vsnprintf_i(m->bytebuf + s, m->bytemax - s, fmt, ap2);
	}
	va_end(ap2);
	m->bytesize += n;
}

static void Kwb_printf(KonohaContext *kctx, KUtilsWriteBuffer *wb, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	Kwb_vprintf(kctx, wb, fmt, ap);
	va_end(ap);
}

static const char* Kwb_top(KonohaContext *kctx, KUtilsWriteBuffer *wb, int ensureZero)
{
	KUtilsGrowingArray *m = wb->m;
	if(ensureZero) {
		if(!(m->bytesize + 1 < m->bytemax)) {
			Karray_expand(kctx, m, m->bytesize + 1);
		}
		m->bytebuf[m->bytesize] = 0;
	}
	return (const char*)m->bytebuf + wb->pos;
}

static void Kwb_free(KUtilsWriteBuffer *wb)
{
	KUtilsGrowingArray *m = wb->m;
	bzero(m->bytebuf + wb->pos, m->bytesize - wb->pos);
	m->bytesize = wb->pos;
}

// -------------------------------------------------------------------------
// KUtilsHashMap

#define HMAP_INIT 83

static void kmap_makeFreeList(KUtilsHashMap *kmap, size_t s, size_t e)
{
	bzero(kmap->arena + s, (e - s) * sizeof(KUtilsHashMapEntry));
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

static void kmap_rehash(KonohaContext *kctx, KUtilsHashMap *kmap)
{
	size_t i, newhmax = kmap->hmax * 2 + 1;
	KUtilsHashMapEntry **newhentry = (KUtilsHashMapEntry**)KCALLOC(newhmax, sizeof(KUtilsHashMapEntry*));
	for(i = 0; i < kmap->arenasize / 2; i++) {
		KUtilsHashMapEntry *e = kmap->arena + i;
		kuint_t ni = e->hcode % newhmax;
		e->next = newhentry[ni];
		newhentry[ni] = e;
	}
	KFREE(kmap->hentry, kmap->hmax * sizeof(KUtilsHashMapEntry*));
	kmap->hentry = newhentry;
	kmap->hmax = newhmax;
}

static void kmap_shiftptr(KUtilsHashMap *kmap, intptr_t shift)
{
	size_t i, size = kmap->arenasize / 2;
	for(i = 0; i < size; i++) {
		KUtilsHashMapEntry *e = kmap->arena + i;
		if(e->next != NULL) {
			e->next = (KUtilsHashMapEntry*)(((char*)e->next) + shift);
			DBG_ASSERT(kmap->arena <= e->next && e->next < kmap->arena + size);
		}
	}
}

static KUtilsHashMapEntry *Kmap_newEntry(KonohaContext *kctx, KUtilsHashMap *kmap, kuint_t hcode)
{
	KUtilsHashMapEntry *e;
	if(kmap->unused == NULL) {
		size_t oarenasize = kmap->arenasize;
		char *oarena = (char*)kmap->arena;
		kmap->arenasize *= 2;
		kmap->arena = KMALLOC(kmap->arenasize * sizeof(KUtilsHashMapEntry));
		memcpy(kmap->arena, oarena, oarenasize * sizeof(KUtilsHashMapEntry));
		kmap_shiftptr(kmap, (char*)kmap->arena - oarena);
		kmap_makeFreeList(kmap, oarenasize, kmap->arenasize);
		KFREE(oarena, oarenasize * sizeof(KUtilsHashMapEntry));
		kmap_rehash(kctx, kmap);
	}
	e = kmap->unused;
	kmap->unused = e->next;
	e->hcode = hcode;
	e->next = NULL;
	kmap->size++;
	{
		KUtilsHashMapEntry **hlist = kmap->hentry;
		size_t idx = e->hcode % kmap->hmax;
		e->next = hlist[idx];
		hlist[idx] = e;
	}
	return e;
}

static KUtilsHashMap *Kmap_init(KonohaContext *kctx, size_t init)
{
	KUtilsHashMap *kmap = (KUtilsHashMap*)KCALLOC(sizeof(KUtilsHashMap), 1);
	if(init < HMAP_INIT) init = HMAP_INIT;
	kmap->arenasize = (init * 3) / 4;
	kmap->arena = (KUtilsHashMapEntry*)KMALLOC(kmap->arenasize * sizeof(KUtilsHashMapEntry));
	kmap_makeFreeList(kmap, 0, kmap->arenasize);
	kmap->hentry = (KUtilsHashMapEntry**)KCALLOC(init, sizeof(KUtilsHashMapEntry*));
	kmap->hmax = init;
	kmap->size = 0;
	return (KUtilsHashMap*)kmap;
}

static void Kmap_each(KonohaContext *kctx, KUtilsHashMap *kmap, void *thunk, void (*f)(KonohaContext *kctx, KUtilsHashMapEntry *, void *thunk))
{
	size_t i;
	for(i = 0; i < kmap->hmax; i++) {
		KUtilsHashMapEntry *e = kmap->hentry[i];
		while(e != NULL) {
			f(kctx, e, thunk);
			e = e->next;
		}
	}
}

static void Kmap_free(KonohaContext *kctx, KUtilsHashMap *kmap, void (*f)(KonohaContext *kctx, void *))
{
	if(f != NULL) {
		size_t i;
		for(i = 0; i < kmap->hmax; i++) {
			KUtilsHashMapEntry *e = kmap->hentry[i];
			while(e != NULL) {
				f(kctx, e->ptrValue);
				e = e->next;
			}
		}
	}
	KFREE(kmap->arena, sizeof(KUtilsHashMapEntry)*(kmap->arenasize));
	KFREE(kmap->hentry, sizeof(KUtilsHashMapEntry*)*(kmap->hmax));
	KFREE(kmap, sizeof(KUtilsHashMap));
}

static KUtilsHashMapEntry *Kmap_getentry(KonohaContext *kctx, KUtilsHashMap* kmap, kuint_t hcode)
{
	KUtilsHashMapEntry **hlist = kmap->hentry;
	size_t idx = hcode % kmap->hmax;
	KUtilsHashMapEntry *e = hlist[idx];
	while(e != NULL) {
		if(e->hcode == hcode) return e;
		e = e->next;
	}
	return NULL;
}

static void kmap_unuse(KUtilsHashMap *kmap, KUtilsHashMapEntry *e)
{
	e->next = kmap->unused;
	kmap->unused = e;
	e->hcode = ((uintptr_t)-1);
	e->unboxValue  = 0;
	kmap->size--;
}

static void Kmap_remove(KUtilsHashMap* kmap, KUtilsHashMapEntry *oe)
{
	KUtilsHashMapEntry **hlist = kmap->hentry;
	size_t idx = oe->hcode % kmap->hmax;
	KUtilsHashMapEntry *e = hlist[idx];
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

static void Kmap_addStringUnboxValue(KonohaContext *kctx, KUtilsHashMap *kmp, uintptr_t hcode, kString *stringKey, uintptr_t unboxValue)
{
	KUtilsHashMapEntry *e = KLIB Kmap_newEntry(kctx, kmp, hcode);
	KINITv(e->stringKey, stringKey);
	e->unboxValue = unboxValue;
}

static ksymbol_t Kmap_getcode(KonohaContext *kctx, KUtilsHashMap *kmp, kArray *list, const char *name, size_t len, uintptr_t hcode, int spol, ksymbol_t def)
{
	KUtilsHashMapEntry *e = KLIB Kmap_get(kctx, kmp, hcode);
	while(e != NULL) {
		if(e->hcode == hcode && len == S_size(e->stringKey) && strncmp(S_text(e->stringKey), name, len) == 0) {
			return (ksymbol_t)e->unboxValue;
		}
		e = e->next;
	}
	if(def == SYM_NEWID) {
		kString *stringKey = KLIB new_kString(kctx, name, len, spol);
		uintptr_t sym = kArray_size(list);
		KLIB kArray_add(kctx, list, stringKey);
		Kmap_addStringUnboxValue(kctx, kmp, hcode, stringKey, sym);
		return (ksymbol_t)sym;
	}
	return def;
}

static kfileline_t KfileId(KonohaContext *kctx, const char *name, size_t len, int spol, ksymbol_t def)
{
	uintptr_t hcode = strhash(name, len);
	KLock(kctx->share->filepackMutex);
	kfileline_t uline = Kmap_getcode(kctx, kctx->share->fileidMapNN, kctx->share->fileidList, name, len, hcode, spol, def);
	KUnlock(kctx->share->filepackMutex);
	return uline << (sizeof(kshort_t) * 8);
}

static kpackage_t KpackageId(KonohaContext *kctx, const char *name, size_t len, int spol, ksymbol_t def)
{
	uintptr_t hcode = strhash(name, len);
	KLock(kctx->share->filepackMutex);
	kpackage_t packid = Kmap_getcode(kctx, kctx->share->packageIdMapNN, kctx->share->packageIdList, name, len, hcode, spol | SPOL_ASCII, def);
	KUnlock(kctx->share->filepackMutex);
	return packid;
}

static ksymbol_t Ksymbol(KonohaContext *kctx, const char *name, size_t len, int spol, ksymbol_t def)
{
	ksymbol_t mask = 0;
	int ch0 = name[0], ch1 = name[1];
	if(def != SYM_NEWRAW) {
		if(ch1 == 'e' && name[2] == 't') {
			if(ch0 == 'g'/* || ch0 == 'G'*/) {
				len -= 3; name += 3;
				mask = MN_GETTER;
			}
			else if(ch0 == 's'/* || ch0 == 'S'*/) {
				len -= 3; name += 3;
				mask = MN_SETTER;
			}
		}
//		else if(ch1 == 's' && (ch0 == 'i'/* || ch0 == 'I'*/)) {
//			len -= 2; name += 2;
//			mask = MN_ISBOOL;
//		}
		else if(ch1 == 'o' && (ch0 == 't'/* || ch0 == 'T'*/)) {
			len -= 2; name += 2;
			mask = MN_TOCID;
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
	ksymbol_t sym = Kmap_getcode(kctx, kctx->share->symbolMapNN, kctx->share->symbolList, name, len, hcode, spol | SPOL_ASCII, def);
	KUnlock(kctx->share->symbolMutex);
	return (sym == def) ? def : (sym | mask);
}

// -------------------------------------------------------------------------
// library

void KONOHA_freeObjectField(KonohaContext *kctx, kObjectVar *o)
{
	KonohaClass *ct = O_ct(o);
	protomap_delete((Kprotomap_t *)o->h.kvproto);
	ct->free(kctx, o);
}

void KONOHA_reftraceObject(KonohaContext *kctx, kObject *o)
{
	unsigned map_size;
	O_ct(o)->reftrace(kctx, o);
	map_size = protomap_size((Kprotomap_t *)o->h.kvproto);
	if (map_size) {
		protomap_iterator itr = {0};
		KUtilsKeyValue *d;
		BEGIN_REFTRACE(map_size);
		while ((d = protomap_next((Kprotomap_t *)o->h.kvproto, &itr)) != NULL) {
			if(SYMKEY_isBOXED(d->key)) {
				KREFTRACEv(d->objectValue);
			}
		}
		END_REFTRACE();
	}
}

static kObject* kObject_getObjectNULL(KonohaContext *kctx, kObject *o, ksymbol_t key, kObject *defval)
{
	KUtilsKeyValue *d = protomap_get((Kprotomap_t *)o->h.kvproto, key | SYMKEY_BOXED);
	return (d != NULL) ? d->objectValue : defval;
}

static void kObject_setObject(KonohaContext *kctx, kAbstractObject *o, ksymbol_t key, ktype_t ty, kObject *val)
{
	kObjectVar *v = (kObjectVar*)o;
	protomap_set((Kprotomap_t **)&v->h.kvproto, key | SYMKEY_BOXED, ty, (void*)val);
	KLIB Kwrite_barrier(kctx, v);
}

static uintptr_t kObject_getUnboxValue(KonohaContext *kctx, kObject *o, ksymbol_t key, uintptr_t defval)
{
	KUtilsKeyValue *d = protomap_get((Kprotomap_t *)o->h.kvproto, key);
	return (d != NULL) ? d->unboxValue : defval;
}

static void kObject_setUnboxValue(KonohaContext *kctx, kObject *o, ksymbol_t key, ktype_t ty, uintptr_t unboxValue)
{
	kObjectVar *v = (kObjectVar*)o;
	protomap_set((Kprotomap_t **)&v->h.kvproto, key, ty, (void*)unboxValue);
}

static void kObject_removeKey(KonohaContext *kctx, kObject *o, ksymbol_t key)
{
	KUtilsKeyValue *d = protomap_get((Kprotomap_t *)o->h.kvproto, key | SYMKEY_BOXED);
	if(d != NULL) {
		d->key = 0; d->ty = 0; d->unboxValue = 0;
	}
	d = protomap_get((Kprotomap_t *)o->h.kvproto, key);
	if(d != NULL) {
		d->key = 0; d->ty = 0; d->unboxValue = 0;
	}
}

typedef void (*feach)(KonohaContext *kctx, void *, KUtilsKeyValue *d);
static void kObject_protoEach(KonohaContext *kctx, kObject *o, void *thunk, feach f)
{
	KUtilsKeyValue *r;
	protomap_iterator itr = {0};
	while ((r = protomap_next((Kprotomap_t *)o->h.kvproto, &itr)) != NULL) {
		f(kctx, thunk, r);
	}
}

// -------------------------------------------------------------------------

/* debug mode */
int verbose_debug = 0;

static void Kreportf(KonohaContext *kctx, kinfotag_t level, kfileline_t pline, const char *fmt, ...)
{
	if(level == DebugTag && !verbose_debug) return;
	va_list ap;
	va_start(ap , fmt);
	const char *B = PLATAPI beginTag(level);
	const char *E = PLATAPI endTag(level);
	if(pline != 0) {
		const char *file = FileId_t(pline);
		PLATAPI printf_i("%s - %s(%s:%d) " , B, TAG_t(level), PLATAPI shortFilePath(file), (kushort_t)pline);
	}
	else {
		PLATAPI printf_i("%s - %s" , B, TAG_t(level));
	}
	PLATAPI vprintf_i(fmt, ap);
	PLATAPI printf_i("%s\n", E);
	va_end(ap);
}

// -------------------------------------------------------------------------

static kbool_t KonohaRuntime_tryCallMethod(KonohaContext *kctx, KonohaStack *sfp)
{
	KonohaStackRuntimeVar *runtime = kctx->stack;
	KonohaStack *jump_bottom = runtime->jump_bottom;
	jmpbuf_i lbuf = {};
	if(runtime->evaljmpbuf == NULL) {
		runtime->evaljmpbuf = (jmpbuf_i*)KCALLOC(sizeof(jmpbuf_i), 1);
	}
	memcpy(&lbuf, runtime->evaljmpbuf, sizeof(jmpbuf_i));
	runtime->jump_bottom = sfp;
	runtime->thrownScriptLine = 0;
	KSETv_AND_WRITE_BARRIER(NULL, runtime->optionalErrorMessage, TS_EMPTY, GC_NO_WRITE_BARRIER);
	kbool_t result = true;
	int jumpResult;
	INIT_GCSTACK();
	if((jumpResult = PLATAPI setjmp_i(*runtime->evaljmpbuf)) == 0) {
		KonohaRuntime_callMethod(kctx, sfp);
	}
	else {
		const char *file = PLATAPI shortFilePath(FileId_t(runtime->thrownScriptLine));
		PLATAPI reportCaughtException(SYM_t(jumpResult), file, (kushort_t)runtime->thrownScriptLine,  S_text(runtime->optionalErrorMessage));
		result = false;
	}
	RESET_GCSTACK();
	runtime->jump_bottom = jump_bottom;
	memcpy(runtime->evaljmpbuf, &lbuf, sizeof(jmpbuf_i));
	return result;
}

static void KonohaRuntime_raise(KonohaContext *kctx, int symbol, KonohaStack *sfp, kfileline_t pline, kString *optionalErrorMessage)
{
	KonohaStackRuntimeVar *runtime = kctx->stack;
	KNH_ASSERT(symbol != 0);
	if(runtime->evaljmpbuf != NULL) {
		runtime->thrownScriptLine = pline;
		if(optionalErrorMessage != NULL) {
			//KSETv(K_NULL, runtime->optionalErrorMessage, optionalErrorMessage);
			KSETv_AND_WRITE_BARRIER(NULL, runtime->optionalErrorMessage, optionalErrorMessage, GC_NO_WRITE_BARRIER);
		}
		PLATAPI longjmp_i(*runtime->evaljmpbuf, symbol);  // in setjmp 0 means good
	}
	PLATAPI exit_i(EXIT_FAILURE);
}

/* ------------------------------------------------------------------------ */

// Don't export KONOHA_reftail to packages
// Don't include KONOHA_reftail in shared header files  (kimio)

static kObjectVar** KONOHA_reftail(KonohaContext *kctx, size_t size)
{
	KonohaStackRuntimeVar *stack = kctx->stack;
	size_t ref_size = stack->reftail - stack->ref.refhead;
	if(stack->ref.bytemax/sizeof(void*) < size + ref_size) {
		KLIB Karray_expand(kctx, &stack->ref, (size + ref_size) * sizeof(kObject*));
		stack->reftail = stack->ref.refhead + ref_size;
	}
	kObjectVar **reftail = stack->reftail;
	stack->reftail = NULL;
	return reftail;
}

// -------------------------------------------------------------------------

static void klib_init(KonohaLibVar *l)
{
	l->Kobject_reftail = KONOHA_reftail;
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
	l->Kmap_init     = Kmap_init;
	l->Kmap_free     = Kmap_free;
	l->Kmap_each     = Kmap_each;
	l->Kmap_newEntry = Kmap_newEntry;
	l->Kmap_get      = Kmap_getentry;
	l->Kmap_remove   = Kmap_remove;
	l->Kmap_getcode  = Kmap_getcode;
	l->kObject_getObject     = (typeof(l->kObject_getObject))kObject_getObjectNULL;
	l->kObject_setObject     = (typeof(l->kObject_setObject))kObject_setObject;
	l->kObject_getUnboxValue = (typeof(l->kObject_getUnboxValue))kObject_getUnboxValue;
	l->kObject_setUnboxValue = (typeof(l->kObject_setUnboxValue))kObject_setUnboxValue;
	l->kObject_removeKey     = (typeof(l->kObject_removeKey))kObject_removeKey;
	l->kObject_protoEach     = (typeof(l->kObject_protoEach))kObject_protoEach;
	l->KfileId       = KfileId;
	l->KpackageId    = KpackageId;
	l->Ksymbol       = Ksymbol;
	l->Kreportf      = Kreportf;
	l->KonohaRuntime_tryCallMethod = KonohaRuntime_tryCallMethod;
	l->KonohaRuntime_raise         = KonohaRuntime_raise;
}
