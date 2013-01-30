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


#define DICTMAP_THRESHOLD 4
#ifndef CLZ
#define CLZ(n) __builtin_clzl(n)
#endif

#ifndef BITS
#define BITS (sizeof(void *) * 8)
#endif

#ifndef POWER_OF_TWO
#define POWER_OF_TWO(N) ((unsigned)(BITS - CLZ(N - 1)))
#endif

#define MAP_INITSIZE DICTMAP_THRESHOLD

#define _MALLOC(SIZE)    malloc(SIZE)
#define _FREE(PTR, SIZE) free(PTR)

typedef struct protomap_record {
	kushort_t hash;
	kushort_t type;
	uintptr_t v;
} map_record_t;

struct map_api;
typedef struct map_api protomap_api_t;

struct map_base {
	const protomap_api_t *api;
	map_record_t *records;
	unsigned used_size;
} __attribute__ ((packed));

typedef struct hashmap_t {
	struct map_base base;
	unsigned record_size_mask;
} hashmap_t;

typedef struct dictmap_t {
	struct map_base base;
	kushort_t hash_list[DICTMAP_THRESHOLD];
} dictmap_t;

typedef union Kprotomap_t {
	hashmap_t h;
	dictmap_t d;
} Kprotomap_t;

typedef struct map_iterator {
	long index;
} protomap_iterator;

typedef enum map_status_t {
	PROTOMAP_FAILED = 0,
	PROTOMAP_UPDATE = 1,
	PROTOMAP_ADDED  = 2
} map_status_t;

struct map_api {
	map_record_t *(*_get)(Kprotomap_t *m, unsigned hash);
	map_status_t  (*_Set)(Kprotomap_t *m, unsigned hash, unsigned type, void *val, Kprotomap_t **map);
	map_record_t *(*_next)(Kprotomap_t *m, protomap_iterator *itr);
	void (*_Remove)(Kprotomap_t *m, unsigned hash);
	void (*_Init)(Kprotomap_t *m, unsigned init);
	void (*_dispose)(Kprotomap_t *m);
};

static Kprotomap_t *Kprotomap_new(unsigned init);
static void protomap_delete(Kprotomap_t *m);

static inline void protomap_dispose(Kprotomap_t *m)
{
	m->h.base.api->_dispose(m);
}

static inline KKeyValue *protomap_get(Kprotomap_t *m, unsigned hash)
{
	return (KKeyValue *) m->h.base.api->_get(m, hash);
}

static inline map_status_t protomap_Set(Kprotomap_t **m, unsigned hash, unsigned type, void *val)
{
	return (*m)->h.base.api->_Set(*m, hash, type, val, m);
}

static inline void protomap_Remove(Kprotomap_t *m, unsigned hash)
{
	return m->h.base.api->_Remove(m, hash);
}

static inline KKeyValue *protomap_next(Kprotomap_t *m, protomap_iterator *itr)
{
	return (KKeyValue *) m->h.base.api->_next(m, itr);
}

static inline unsigned protomap_size(Kprotomap_t *m)
{
	return m->h.base.used_size;
}

static inline Kprotomap_t *protomap_create(const protomap_api_t *api)
{
	Kprotomap_t *m = (Kprotomap_t *) _MALLOC(sizeof(*m));
	m->h.base.api = api;
	m->h.base.used_size = 0;
	return m;
}

static inline void map_record_copy(map_record_t *dst, const map_record_t *src)
{
	memcpy(dst, src, sizeof(map_record_t));
}

static inline map_record_t *hashmap_at(hashmap_t *m, unsigned idx)
{
	assert(idx < (m->record_size_mask+1));
	return m->base.records+idx;
}

/* [UTILS] */
static void dictmap_convert2hashmap(dictmap_t *m);
static void hashmap_record_reset(hashmap_t *m, size_t newsize)
{
	unsigned alloc_size = sizeof(map_record_t) * newsize;
	m->base.used_size = 0;
	(m->record_size_mask) = newsize - 1;
	m->base.records = (map_record_t *) calloc(1, alloc_size);
}

#define DELTA 8

/* [HASHMAP] */
static map_status_t hashmap_Set_no_resize(hashmap_t *m, map_record_t *rec)
{
	unsigned i, idx = rec->hash & m->record_size_mask;
	for (i = 0; i < DELTA; ++i) {
		map_record_t *r = m->base.records+idx;
		if(r->hash == 0 && r->type != rec->type) {
			map_record_copy(r, rec);
			++m->base.used_size;
			return PROTOMAP_ADDED;
		}
		if(r->hash == rec->hash) {
			uintptr_t old = r->v;
			map_record_copy(r, rec);
			rec->v  = old;
			return PROTOMAP_UPDATE;
		}
		idx = (idx + 1) & m->record_size_mask;
	}
	return PROTOMAP_FAILED;
}

static void hashmap_record_resize(hashmap_t *m)
{
	unsigned oldsize = (m->record_size_mask+1);
	unsigned newsize = oldsize;
	map_record_t *head = m->base.records;

	do {
		unsigned i;
		newsize *= 2;
		hashmap_record_reset(m, newsize);
		for (i = 0; i < oldsize; ++i) {
			map_record_t *r = head + i;
			if(r->hash != 0 && hashmap_Set_no_resize(m, r) == PROTOMAP_FAILED)
				continue;
		}
	} while(0);
	_FREE(head, oldsize*sizeof(map_record_t));
}

static map_status_t hashmap_Set(hashmap_t *m, map_record_t *rec)
{
	map_status_t res;
	do {
		if((res = hashmap_Set_no_resize(m, rec)) != PROTOMAP_FAILED)
			return res;
		hashmap_record_resize(m);
	} while(1);
	return PROTOMAP_FAILED;
}

static map_record_t *hashmap_get(hashmap_t *m, unsigned hash)
{
	unsigned i, idx = hash & m->record_size_mask;
	for (i = 0; i < DELTA; ++i) {
		map_record_t *r = m->base.records+idx;
		if(r->hash == hash) {
			return r;
		}
		idx = (idx + 1) & m->record_size_mask;
	}
	return NULL;
}

static void hashmap_Init(hashmap_t *m, unsigned init)
{
	if(init < MAP_INITSIZE)
		init = MAP_INITSIZE;
	hashmap_record_reset(m, 1U << (POWER_OF_TWO(init)));
}

static void hashmap_api_Init(Kprotomap_t *m, unsigned init)
{
	hashmap_Init((hashmap_t *) m, init);
}

static void hashmap_api_dispose(Kprotomap_t *_m)
{
	hashmap_t *m = (hashmap_t *) _m;
	_FREE(m->base.records, (m->record_size_mask+1) * sizeof(map_record_t));
}

static map_record_t *hashmap_api_get(Kprotomap_t *_m, unsigned hash)
{
	hashmap_t *m = (hashmap_t *) _m;
	map_record_t *r = hashmap_get(m, hash);
	return r;
}

static map_status_t hashmap_api_Set(Kprotomap_t *_m, unsigned hash, unsigned type, void *val, Kprotomap_t **ptr)
{
	(void)ptr;
	hashmap_t *m = (hashmap_t *) _m;
	map_record_t r;
	r.hash = hash;
	r.type = type;
	r.v  = (uintptr_t) val;
	return hashmap_Set(m, &r);
}

static void hashmap_api_Remove(Kprotomap_t *_m, unsigned hash)
{
	hashmap_t *m = (hashmap_t *) _m;
	map_record_t *r = hashmap_get(m, hash);
	if(r) {
		r->hash = 0;
		r->type = 0;
		m->base.used_size -= 1;
	}
}

static map_record_t *hashmap_api_next(Kprotomap_t *_m, protomap_iterator *itr)
{
	hashmap_t *m = (hashmap_t *) _m;
	unsigned i, size = (m->record_size_mask);
	for (i = itr->index; i <= size; ++i) {
		map_record_t *r = hashmap_at(m, i);
		if(r->hash != 0) {
			itr->index = i+1;
			return r;
		}
	}
	itr->index = size;
	return NULL;
}

static const protomap_api_t HASH_API = {
	hashmap_api_get,
	hashmap_api_Set,
	hashmap_api_next,
	hashmap_api_Remove,
	hashmap_api_Init,
	hashmap_api_dispose
};

static Kprotomap_t *hashmap_new(unsigned init)
{
	Kprotomap_t *m = protomap_create(&HASH_API);
	hashmap_Init((hashmap_t *) m, init);
	return m;
}

/* [DICTMAP] */
static Kprotomap_t *dictmap_Init(dictmap_t *m)
{
	int i;
	const size_t allocSize = sizeof(map_record_t)*DICTMAP_THRESHOLD;
	m->base.records = (map_record_t *) _MALLOC(allocSize);

	for (i = 0; i < DICTMAP_THRESHOLD; ++i) {
		m->hash_list[i] = 0;
		m->base.records[i].type = 0;
	}
	return (Kprotomap_t *) m;
}

static void dictmap_api_Init(Kprotomap_t *_m, unsigned init)
{
	dictmap_Init((dictmap_t *) _m);
	(void)init;
}

static void dictmap_record_copy(map_record_t *dst, const map_record_t *src)
{
	memcpy(dst, src, sizeof(map_record_t));
}

static inline map_record_t *dictmap_at(dictmap_t *m, unsigned idx)
{
	return (map_record_t *)(m->base.records+idx);
}

static map_status_t dictmap_Set_newentry(dictmap_t *m, map_record_t *rec, int i)
{
	map_record_t *r = dictmap_at(m, i);
	m->hash_list[i] = rec->hash;
	dictmap_record_copy(r, rec);
	m->base.used_size += 1;
	assert(m->base.used_size <= 4);
	return PROTOMAP_ADDED;
}

static map_status_t dictmap_Set(dictmap_t *m, map_record_t *rec)
{
	int i;
	for (i = 0; i < DICTMAP_THRESHOLD; ++i) {
		unsigned hash = m->hash_list[i];
		map_record_t *r = dictmap_at(m, i);
		if(hash == 0 && unlikely(r->type == 0)) {
			return dictmap_Set_newentry(m, rec, i);
		}
		else if(hash == rec->hash) {
			uintptr_t old = r->v;
			dictmap_record_copy(r, rec);
			rec->v  = old;
			return PROTOMAP_UPDATE;
		}
	}
	dictmap_convert2hashmap(m);
	return hashmap_Set((hashmap_t *) m, rec);
}

static map_record_t *dictmap_get(dictmap_t *m, unsigned hash)
{
	int i;
	for (i = 0; i < DICTMAP_THRESHOLD; ++i) {
		if(hash == m->hash_list[i]) {
			map_record_t *r = dictmap_at(m, i);
			return r;
		}
	}
	return NULL;
}

static map_status_t dictmap_api_Set(Kprotomap_t *_m, unsigned hash, unsigned type, void *val, Kprotomap_t **ptr)
{
	(void)ptr;
	dictmap_t *m = (dictmap_t *)_m;
	map_record_t r;
	r.hash = hash;
	r.type = type;
	r.v  = (uintptr_t) val;
	return dictmap_Set(m, &r);
}

static void dictmap_api_Remove(Kprotomap_t *_m, unsigned hash)
{
	dictmap_t *m = (dictmap_t *)_m;
	map_record_t *r = dictmap_get(m, hash);
	if(r) {
		r->hash = 0;
		r->type = 0;
		m->base.used_size -= 1;
	}
}

static map_record_t *dictmap_api_next(Kprotomap_t *_m, protomap_iterator *itr)
{
	dictmap_t *m = (dictmap_t *)_m;
	unsigned i;
	for (i = itr->index; i < m->base.used_size; ++i) {
		itr->index = i+1;
		return dictmap_at(m, i);
	}
	itr->index = m->base.used_size;
	return NULL;
}

static map_record_t *dictmap_api_get(Kprotomap_t *_m, unsigned hash)
{
	dictmap_t *m = (dictmap_t *)_m;
	map_record_t *r = dictmap_get(m, hash);
	return r;
}

static void dictmap_api_dispose(Kprotomap_t *_m)
{
	dictmap_t *m = (dictmap_t *)_m;
	free((map_record_t *)m->base.records);
}

static const protomap_api_t DIKClass_API = {
	dictmap_api_get,
	dictmap_api_Set,
	dictmap_api_next,
	dictmap_api_Remove,
	dictmap_api_Init,
	dictmap_api_dispose
};

static Kprotomap_t *dictmap_new()
{
	Kprotomap_t *m = protomap_create(&DIKClass_API);
	return dictmap_Init((dictmap_t *) m);
}

/* [NULLMAP] */

typedef struct nullmap_t {
	struct map_base base;
} nullmap_t;

static void nullmap_api_Init(Kprotomap_t *m, unsigned init)
{
	(void)m; (void) init;
}

static map_status_t nullmap_api_Set(Kprotomap_t *_m, unsigned hash, unsigned type, void *val, Kprotomap_t **ptr)
{
	(void)_m;
	*ptr = dictmap_new();
	return dictmap_api_Set(*ptr, hash, type, val, ptr);
}

static void nullmap_api_Remove(Kprotomap_t *_m, unsigned hash)
{
	(void)_m; (void)hash;
}

static map_record_t *nullmap_api_next(Kprotomap_t *_m, protomap_iterator *itr)
{
	(void)_m; (void)itr;
	return NULL;
}

static map_record_t *nullmap_api_get(Kprotomap_t *_m, unsigned hash)
{
	(void)_m; (void)hash;
	return NULL;
}

static void nullmap_api_dispose(Kprotomap_t *_m)
{
	(void)_m;
}

static const protomap_api_t NULL_API = {
	nullmap_api_get,
	nullmap_api_Set,
	nullmap_api_next,
	nullmap_api_Remove,
	nullmap_api_Init,
	nullmap_api_dispose
};

static const nullmap_t nullmap = {
	{&NULL_API, 0, 0}
};

static inline Kprotomap_t *nullmap_new(void)
{
	return (Kprotomap_t *) &nullmap;
}

/* [PROTOMAP] */
static Kprotomap_t *Kprotomap_new(unsigned init)
{
	Kprotomap_t *m = (init == 0) ? nullmap_new() :
		(init <= DICTMAP_THRESHOLD) ? dictmap_new() : hashmap_new(init);
	return m;
}

static void protomap_delete(Kprotomap_t *m)
{
	protomap_dispose(m);
	if(m != (Kprotomap_t *)&nullmap) {
		free(m);
	}
}

static void dictmap_convert2hashmap(dictmap_t *_m)
{
	hashmap_t *m = (hashmap_t *) _m;
	m->base.api = &HASH_API;
	m->record_size_mask = DICTMAP_THRESHOLD-1;
	hashmap_record_resize(m);
}

