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

static void karray_init(KonohaContext *kctx, karray_t *m, size_t bytemax)
{
	m->bytesize = 0;
	m->bytemax  = bytemax;
	m->bytebuf = (char*)KCALLOC(bytemax, 1);
}

static void karray_resize(KonohaContext *kctx, karray_t *m, size_t newsize)
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

static void karray_expand(KonohaContext *kctx, karray_t *m, size_t minsize)
{
	if(m->bytemax == 0) {
		if(minsize > 0) karray_init(kctx, m, minsize);
	}
	else {
		size_t oldsize = m->bytemax, newsize = oldsize * 2;
		if(minsize > newsize) newsize = minsize;
		karray_resize(kctx, m, newsize);
	}
}

static void karray_free(KonohaContext *kctx, karray_t *m)
{
	if(m->bytemax > 0) {
		KFREE(m->bytebuf, m->bytemax);
		m->bytebuf = NULL;
		m->bytesize = 0;
		m->bytemax  = 0;
	}
}

static void Kwb_init(karray_t *m, kwb_t *wb)
{
	wb->m = m;
	wb->pos = m->bytesize;
}

static void Kwb_write(KonohaContext *kctx, kwb_t *wb, const char *data, size_t bytelen)
{
	karray_t *m = wb->m;
	if(!(m->bytesize + bytelen < m->bytemax)) {
		karray_expand(kctx, m, m->bytesize + bytelen);
	}
	memcpy(m->bytebuf + m->bytesize, data, bytelen);
	m->bytesize += bytelen;
}

static void Kwb_putc(KonohaContext *kctx, kwb_t *wb, ...)
{
	char buf[256];
	int ch, len = 0;
	va_list ap;
	va_start(ap , wb);
	while((ch = (int)va_arg(ap, int)) != -1) {
		buf[len] = ch;
		len++;
 	}
	Kwb_write(kctx, wb, buf, len);
	va_end(ap);
}

static void Kwb_vprintf(KonohaContext *kctx, kwb_t *wb, const char *fmt, va_list ap)
{
	va_list ap2;
	va_copy(ap2, ap);
	karray_t *m = wb->m;
	size_t s = m->bytesize;
	size_t n = PLAT vsnprintf_i( m->bytebuf + s, m->bytemax - s, fmt, ap);
	if(n >= (m->bytemax - s)) {
		karray_expand(kctx, m, n + 1);
		n = PLAT vsnprintf_i(m->bytebuf + s, m->bytemax - s, fmt, ap2);
	}
	va_end(ap2);
	m->bytesize += n;
}

static void Kwb_printf(KonohaContext *kctx, kwb_t *wb, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	Kwb_vprintf(kctx, wb, fmt, ap);
	va_end(ap);
}

static const char* Kwb_top(KonohaContext *kctx, kwb_t *wb, int ensureZero)
{
	karray_t *m = wb->m;
	if(ensureZero) {
		if(!(m->bytesize + 1 < m->bytemax)) {
			karray_expand(kctx, m, m->bytesize + 1);
		}
		m->bytebuf[m->bytesize] = 0;
	}
	return (const char*)m->bytebuf + wb->pos;
}

static void Kwb_free(kwb_t *wb)
{
	karray_t *m = wb->m;
	bzero(m->bytebuf + wb->pos, m->bytesize - wb->pos);
	m->bytesize = wb->pos;
}

// -------------------------------------------------------------------------
// kmap_t

#define HMAP_INIT 83

static void kmap_makeFreeList(kmap_t *kmap, size_t s, size_t e)
{
	bzero(kmap->arena + s, (e - s) * sizeof(kmape_t));
	kmap->unused = kmap->arena + s;
	size_t i;
	for(i = s; i < e - 1; i++) {
		kmap->arena[i].hcode = ((uintptr_t)-1);
		kmap->arena[i].uvalue = 0;
		kmap->arena[i].next = kmap->arena + i + 1;
	}
	kmap->arena[e-1].hcode = ((uintptr_t)-1);
	kmap->arena[e-1].uvalue = 0;
	DBG_ASSERT(kmap->arena[e-1].next == NULL);
}

static void kmap_rehash(KonohaContext *kctx, kmap_t *kmap)
{
	size_t i, newhmax = kmap->hmax * 2 + 1;
	kmape_t **newhentry = (kmape_t**)KCALLOC(newhmax, sizeof(kmape_t*));
	for(i = 0; i < kmap->arenasize / 2; i++) {
		kmape_t *e = kmap->arena + i;
		kuint_t ni = e->hcode % newhmax;
		e->next = newhentry[ni];
		newhentry[ni] = e;
	}
	KFREE(kmap->hentry, kmap->hmax * sizeof(kmape_t*));
	kmap->hentry = newhentry;
	kmap->hmax = newhmax;
}

static void kmap_shiftptr(kmap_t *kmap, intptr_t shift)
{
	size_t i, size = kmap->arenasize / 2;
	for(i = 0; i < size; i++) {
		kmape_t *e = kmap->arena + i;
		if(e->next != NULL) {
			e->next = (kmape_t*)(((char*)e->next) + shift);
			DBG_ASSERT(kmap->arena <= e->next && e->next < kmap->arena + size);
		}
	}
}

static kmape_t *Kmap_newentry(KonohaContext *kctx, kmap_t *kmap, kuint_t hcode)
{
	kmape_t *e;
	if(kmap->unused == NULL) {
		size_t oarenasize = kmap->arenasize;
		char *oarena = (char*)kmap->arena;
		kmap->arenasize *= 2;
		kmap->arena = KMALLOC(kmap->arenasize * sizeof(kmape_t));
		memcpy(kmap->arena, oarena, kmap->arenasize * sizeof(kmape_t));
		kmap_shiftptr(kmap, (char*)kmap->arena - oarena);
		kmap_makeFreeList(kmap, oarenasize, kmap->arenasize);
		KFREE(oarena, oarenasize * sizeof(kmape_t));
		kmap_rehash(kctx, kmap);
	}
	e = kmap->unused;
	kmap->unused = e->next;
	e->hcode = hcode;
	e->next = NULL;
	kmap->size++;
	{
		kmape_t **hlist = kmap->hentry;
		size_t idx = e->hcode % kmap->hmax;
		e->next = hlist[idx];
		hlist[idx] = e;
	}
	return e;
}

static kmap_t *Kmap_init(KonohaContext *kctx, size_t init)
{
	kmap_t *kmap = (kmap_t*)KCALLOC(sizeof(kmap_t), 1);
	if(init < HMAP_INIT) init = HMAP_INIT;
	kmap->arenasize = (init * 3) / 4;
	kmap->arena = (kmape_t*)KMALLOC(kmap->arenasize * sizeof(kmape_t));
	kmap_makeFreeList(kmap, 0, kmap->arenasize);
	kmap->hentry = (kmape_t**)KCALLOC(init, sizeof(kmape_t*));
	kmap->hmax = init;
	kmap->size = 0;
	return (kmap_t*)kmap;
}

static void Kmap_reftrace(KonohaContext *kctx, kmap_t *kmap, void (*f)(KonohaContext *kctx, kmape_t *))
{
	size_t i;
	for(i = 0; i < kmap->hmax; i++) {
		kmape_t *e = kmap->hentry[i];
		while(e != NULL) {
			f(kctx, e);
			e = e->next;
		}
	}
}

static void Kmap_free(KonohaContext *kctx, kmap_t *kmap, void (*f)(KonohaContext *kctx, void *))
{
	if(f != NULL) {
		size_t i;
		for(i = 0; i < kmap->hmax; i++) {
			kmape_t *e = kmap->hentry[i];
			while(e != NULL) {
				f(kctx, e->pvalue);
				e = e->next;
			}
		}
	}
	KFREE(kmap->arena, sizeof(kmape_t)*(kmap->arenasize));
	KFREE(kmap->hentry, sizeof(kmape_t*)*(kmap->hmax));
	KFREE(kmap, sizeof(kmap_t));
}

static kmape_t *Kmap_getentry(kmap_t* kmap, kuint_t hcode)
{
	kmape_t **hlist = kmap->hentry;
	size_t idx = hcode % kmap->hmax;
	kmape_t *e = hlist[idx];
	while(e != NULL) {
		if(e->hcode == hcode) return e;
		e = e->next;
	}
	return NULL;
}

static void kmap_unuse(kmap_t *kmap, kmape_t *e)
{
	e->next = kmap->unused;
	kmap->unused = e;
	e->hcode = ((uintptr_t)-1);
	e->uvalue  = 0;
	kmap->size--;
}

static void Kmap_remove(kmap_t* kmap, kmape_t *oe)
{
	kmape_t **hlist = kmap->hentry;
	size_t idx = oe->hcode % kmap->hmax;
	kmape_t *e = hlist[idx];
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

static void map_addStringUnboxValue(KonohaContext *kctx, kmap_t *kmp, uintptr_t hcode, kString *skey, uintptr_t uvalue)
{
	kmape_t *e = kmap_newentry(kmp, hcode);
	KINITv(e->skey, skey);
	e->uvalue = uvalue;
}

static ksymbol_t Kmap_getcode(KonohaContext *kctx, kmap_t *kmp, kArray *list, const char *name, size_t len, uintptr_t hcode, int spol, ksymbol_t def)
{
	kmape_t *e = kmap_get(kmp, hcode);
	while(e != NULL) {
		if(e->hcode == hcode && len == S_size(e->skey) && strncmp(S_text(e->skey), name, len) == 0) {
			return (ksymbol_t)e->uvalue;
		}
		e = e->next;
	}
	if(def == SYM_NEWID) {
		kString *skey = new_kString(name, len, spol);
		uintptr_t sym = kArray_size(list);
		kArray_add(list, skey);
		map_addStringUnboxValue(kctx, kmp, hcode, skey, sym);
		return (ksymbol_t)sym;
	}
	return def;
}

static kline_t Kfileid(KonohaContext *kctx, const char *name, size_t len, int spol, ksymbol_t def)
{
	uintptr_t hcode = strhash(name, len);
	kline_t uline = Kmap_getcode(kctx, kctx->share->fileidMapNN, kctx->share->fileidList, name, len, hcode, spol, def);
	//DBG_P("name='%s', fileid=%d", name, uline);
	return uline << (sizeof(kshort_t) * 8);
}

static kpack_t Kpack(KonohaContext *kctx, const char *name, size_t len, int spol, ksymbol_t def)
{
	uintptr_t hcode = strhash(name, len);
	return Kmap_getcode(kctx, kctx->share->packMapNN, kctx->share->packList, name, len, hcode, spol | SPOL_ASCII, def);
}

static ksymbol_t Ksymbol2(KonohaContext *kctx, const char *name, size_t len, int spol, ksymbol_t def)
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
		else if(ch1 == 's' && (ch0 == 'i'/* || ch0 == 'I'*/)) {
			len -= 2; name += 2;
			mask = MN_ISBOOL;
		}
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
	uintptr_t hcode = strhash(name, len);
	ksymbol_t sym = Kmap_getcode(kctx, kctx->share->symbolMapNN, kctx->share->symbolList, name, len, hcode, spol | SPOL_ASCII, def);
	return (sym == def) ? def : (sym | mask);
}

// -------------------------------------------------------------------------
// library

static karray_t *new_karray(KonohaContext *kctx, size_t bytesize, size_t bytemax)
{
	karray_t *m = (karray_t*)KCALLOC(sizeof(karray_t), 1);
	DBG_ASSERT(bytesize <= bytemax);
	if(bytemax > 0) {
		m->bytebuf = (char*)KCALLOC(bytemax, 1);
		m->bytesize = bytesize;
		m->bytemax = bytemax;
	}
	return m;
}

#define KVPROTO_INIT  8
#define KVPROTO_DELTA 7

static inline karray_t* kvproto_null(void)  // for proto_get safe null
{
	static kvs_t dnull[KVPROTO_DELTA] = {};
	static karray_t pnull = {
		.bytesize = sizeof(kvs_t), .bytemax = 0,
	};
	pnull.kvs = dnull;
	return &pnull;
}

void KONOHA_freeObjectField(KonohaContext *kctx, struct _kObject *o)
{
	kclass_t *ct = O_ct(o);
	if(o->h.kvproto->bytemax > 0) {
		karray_t *p = o->h.kvproto;
		KFREE(p->bytebuf, p->bytemax);
		KFREE(p, sizeof(karray_t));
		o->h.kvproto = kvproto_null();
	}
	ct->free(kctx, o);
}

static kvs_t* kvproto_get(karray_t *p, ksymbol_t key)
{
	size_t psize = p->bytesize / sizeof(kvs_t);
	kvs_t *d = p->kvs + (((size_t)key) % psize);
	if(d->key == key) return d; else d++;  // 3
	if(d->key == key) return d; else d++;
	if(d->key == key) return d; else d++;
	size_t i;
	for(i = 0; i < KVPROTO_DELTA - 3; i++) {
		if(d->key == key) return d;
		d++;
	}
	return NULL;
}

static inline void kvproto_findset(kvs_t *d, kvs_t *newd)
{
	size_t i;
	for(i = 0; i < KVPROTO_DELTA - 1; i++) {
		if(newd->key == 0) {
			*newd = *d;
			return;
		}
		newd++;
	}
}

static void kvproto_rehash(KonohaContext *kctx, karray_t *p)
{
	size_t i, pmax = (p->bytemax) / sizeof(kvs_t);
	size_t newpmax = pmax * 2, newpsize = newpmax - KVPROTO_DELTA;
	kvs_t *newkvs = (kvs_t*)KCALLOC(sizeof(kvs_t), newpmax);
	for(i = 0; i < pmax; i++) {
		kvs_t *d = p->kvs + i;
		if(d->key != 0) {
			kvs_t *newd = newkvs + ((size_t)d->key) % newpsize;
			if(newd->key == 0) {
				*newd = *d;
			}
			else {
				kvproto_findset(d, newd+1);
			}
		}
	}
	if(newpmax > 32) {
		DBG_P("newpmax=%d, %d bytes", newpmax, newpmax * sizeof(kvs_t));
	}
	KFREE(p->kvs, sizeof(kvs_t) * pmax);
	p->kvs = newkvs;
	p->bytemax = newpmax * sizeof(kvs_t) ;
	p->bytesize = newpsize * sizeof(kvs_t);
}

void KONOHA_reftraceObject(KonohaContext *kctx, kObject *o)
{
	kclass_t *ct = O_ct(o);
	if(o->h.kvproto->bytemax > 0) {
		size_t i, pmax = o->h.kvproto->bytemax / sizeof(kvs_t);
		kvs_t *d = o->h.kvproto->kvs;
		BEGIN_REFTRACE(pmax);
		for(i = 0; i < pmax; i++) {
			if(SYMKEY_isBOXED(d->key)) {
				KREFTRACEv(d->oval);
			}
			d++;
		}
		END_REFTRACE();
	}
	ct->reftrace(kctx, o);
}

static void kvproto_set(KonohaContext *kctx, karray_t **pval, ksymbol_t key, ktype_t ty, uintptr_t uval)
{
	karray_t *p = pval[0];
	if(p->bytemax == 0) {
		p = new_karray(kctx, (KVPROTO_INIT - KVPROTO_DELTA) * sizeof(kvs_t), KVPROTO_INIT * sizeof(kvs_t));
		pval[0] = p;
	}
	do {
		size_t i, psize = p->bytesize / sizeof(kvs_t);
		kvs_t *d = p->kvs + (((size_t)key) % psize);
		for(i = 0; i < KVPROTO_DELTA; i++) {
			if(d->key == key || d->key == 0) {
				d->key = key; d->ty = ty; d->uval = uval;
				return;
			}
			d++;
		}
		kvproto_rehash(kctx, p);
	}
	while(1);
}

static void KObject_protoEach(KonohaContext *kctx, kObject *o, void *thunk, void (*f)(KonohaContext *kctx, void *, kvs_t *d))
{
	size_t i, pmax = o->h.kvproto->bytemax / sizeof(kvs_t);
	kvs_t *d = o->h.kvproto->kvs;
	for (i = 0; i < pmax; ++i, ++d) {
		f(kctx, thunk, d);
	}
}

static kObject* KObject_getObjectNULL(KonohaContext *kctx, kObject *o, ksymbol_t key, kObject *defval)
{
	kvs_t *d = kvproto_get(o->h.kvproto, key | SYMKEY_BOXED);
	return (d != NULL) ? d->oval : defval;
}

static void KObject_setObject(KonohaContext *kctx, kObject *o, ksymbol_t key, ktype_t ty, kObject *val)
{
	W(kObject, o);
	kvproto_set(kctx, &Wo->h.kvproto, key | SYMKEY_BOXED, ty, (uintptr_t)val);
	WASSERT(o);
}

static uintptr_t KObject_getUnboxedValue(KonohaContext *kctx, kObject *o, ksymbol_t key, uintptr_t defval)
{
	kvs_t *d = kvproto_get(o->h.kvproto, key);
	return (d != NULL) ? d->uval : defval;
}

static void KObject_setUnboxedValue(KonohaContext *kctx, kObject *o, ksymbol_t key, ktype_t ty, uintptr_t uval)
{
	W(kObject, o);
	kvproto_set(kctx, &Wo->h.kvproto, key, ty, uval);
	WASSERT(o);
}

static void KObject_removeKey(KonohaContext *kctx, kObject *o, ksymbol_t key)
{
	kvs_t *d = kvproto_get(o->h.kvproto, key | SYMKEY_BOXED);
	if(d != NULL) {
		d->key = 0; d->ty = 0; d->uval = 0;
	}
	d = kvproto_get(o->h.kvproto, key);
	if(d != NULL) {
		d->key = 0; d->ty = 0; d->uval = 0;
	}
}

// -------------------------------------------------------------------------

/* debug mode */
int verbose_debug = 0;

static void Kreportf(KonohaContext *kctx, kinfotag_t level, kline_t pline, const char *fmt, ...)
{
	if(level == DEBUG_ && !verbose_debug) return;
	va_list ap;
	va_start(ap , fmt);
	const char *B = PLAT begin(level);
	const char *E = PLAT end(level);
	if(pline != 0) {
		const char *file = SS_t(pline);
		PLAT printf_i("%s - %s(%s:%d) " , B, TAG_t(level), shortfilename(file), (kushort_t)pline);
	}
	else {
		PLAT printf_i("%s - %s" , B, TAG_t(level));
	}
	PLAT vprintf_i(fmt, ap);
	PLAT printf_i("%s\n", E);
	va_end(ap);
	if(level == CRIT_) {
		kraise(0);
	}
}

// -------------------------------------------------------------------------

static void Kraise(KonohaContext *kctx, int param)
{
	LocalRuntimeVar *base = kctx->stack;
	if(base->evaljmpbuf != NULL) {
		PLAT longjmp_i(*base->evaljmpbuf, param+1);  // in setjmp 0 means good
	}
	PLAT exit_i(EXIT_FAILURE);
}

// -------------------------------------------------------------------------

static kbool_t KRUNTIME_setModule(KonohaContext *kctx, int x, kmodshare_t *d, kline_t pline);

static void klib2_init(struct _klib2 *l)
{
	l->Karray_init   = karray_init;
	l->Karray_resize = karray_resize;
	l->Karray_expand = karray_expand;
	l->Karray_free   = karray_free;
	l->Kwb_init      = Kwb_init;
	l->Kwb_write     = Kwb_write;
	l->Kwb_putc      = Kwb_putc;
	l->Kwb_vprintf   = Kwb_vprintf;
	l->Kwb_printf    = Kwb_printf;
	l->Kwb_top       = Kwb_top;
	l->Kwb_free      = Kwb_free;
	l->Kmap_init     = Kmap_init;
	l->Kmap_free     = Kmap_free;
	l->Kmap_reftrace = Kmap_reftrace;
	l->Kmap_newentry = Kmap_newentry;
	l->Kmap_get      = Kmap_getentry;
	l->Kmap_remove   = Kmap_remove;
	l->Kmap_getcode  = Kmap_getcode;
	l->KObject_getObject = KObject_getObjectNULL;
	l->KObject_setObject = KObject_setObject;
	l->KObject_getUnboxedValue = KObject_getUnboxedValue;
	l->KObject_setUnboxedValue = KObject_setUnboxedValue;
	l->KObject_removeKey = KObject_removeKey;
	l->KObject_protoEach = KObject_protoEach;
	l->Kfileid       = Kfileid;
	l->Kpack         = Kpack;
	l->Ksymbol2      = Ksymbol2;
	l->Kreportf      = Kreportf;
	l->Kraise        = Kraise;
	l->KsetModule    = KRUNTIME_setModule;
}
