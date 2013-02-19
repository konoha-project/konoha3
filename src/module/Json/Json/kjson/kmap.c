/****************************************************************************
 * Copyright (c) 2012-2013, Masahiro Ide <ide@konohascript.org>
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

#ifdef __cplusplus
extern "C" {
#endif

#define KMAP_INITSIZE DICTMAP_THRESHOLD
#define DELTA 16

static void map_record_copy(map_record_t *dst, const map_record_t *src)
{
    *dst = *src;
}

/* [HASHMAP] */
static inline map_record_t *hashmap_at(hashmap_t *m, unsigned idx)
{
    assert(idx < (m->record_size_mask+1));
    return m->base.records+idx;
}

static void hashmap_record_reset(hashmap_t *m, size_t newsize)
{
    unsigned alloc_size = sizeof(map_record_t) * newsize;
    m->used_size = 0;
    (m->record_size_mask) = newsize - 1;
    m->base.records = (map_record_t *) calloc(1, alloc_size);
}

static map_status_t hashmap_set_no_resize(hashmap_t *m, map_record_t *rec)
{
    unsigned i, idx = rec->hash & m->record_size_mask;
    for(i = 0; i < DELTA; ++i) {
        map_record_t *r = m->base.records+idx;
        if(r->hash == 0) {
            map_record_copy(r, rec);
            ++m->used_size;
            return KMAP_ADDED;
        }
        if(r->hash == rec->hash && JSONString_equal(r->k, rec->k)) {
            JSON_free(toJSON(ValueP(r->v)));
            map_record_copy(r, rec);
            return KMAP_UPDATE;
        }
        idx = (idx + 1) & m->record_size_mask;
    }
    return KMAP_FAILED;
}

static void hashmap_record_resize(hashmap_t *m, unsigned newsize)
{
    unsigned i;
    unsigned oldsize = (m->record_size_mask+1);
    map_record_t *head = m->base.records;

    newsize *= 2;
    hashmap_record_reset(m, newsize);
    for(i = 0; i < oldsize; ++i) {
        map_record_t *r = head + i;
        if(r->hash && hashmap_set_no_resize(m, r) == KMAP_FAILED)
            continue;
    }
    free(head/*, oldsize*sizeof(map_record_t)*/);
}

static map_status_t hashmap_set(hashmap_t *m, map_record_t *rec)
{
    map_status_t res;
    do {
        if((res = hashmap_set_no_resize(m, rec)) != KMAP_FAILED)
            return res;
        hashmap_record_resize(m, (m->record_size_mask+1)*2);
    } while(1);
    /* unreachable */
    return KMAP_FAILED;
}

static map_record_t *hashmap_get(hashmap_t *m, unsigned hash, JSONString *key)
{
    unsigned i, idx = hash & m->record_size_mask;
    for(i = 0; i < DELTA; ++i) {
        map_record_t *r = m->base.records+idx;
        if(r->hash == hash && JSONString_equal(r->k, key)) {
            return r;
        }
        idx = (idx + 1) & m->record_size_mask;
    }
    return NULL;
}

static void hashmap_init(hashmap_t *m, unsigned init)
{
    if(init < KMAP_INITSIZE)
        init = KMAP_INITSIZE;
    hashmap_record_reset(m, 1U << LOG2(init));
}

static void hashmap_api_init(kmap_t *m, unsigned init)
{
    hashmap_init((hashmap_t *) m, init);
}

static void hashmap_api_dispose(kmap_t *_m)
{
    hashmap_t *m = (hashmap_t *) _m;
    unsigned i, size = (m->record_size_mask+1);
    for(i = 0; i < size; ++i) {
        map_record_t *r = hashmap_at(m, i);
        if(r->hash) {
            JSONString *key = r->k;
            JSON_free(toJSON(ValueS(key)));
        }
    }
    free(m->base.records/*, (m->record_size_mask+1) * sizeof(map_record_t)*/);
}

static map_record_t *hashmap_api_get(kmap_t *_m, JSONString *key)
{
    hashmap_t *m = (hashmap_t *) _m;
    unsigned hash = JSONString_hashCode(key);
    map_record_t *r = hashmap_get(m, hash, key);
    return r;
}

static map_status_t hashmap_api_set(kmap_t *_m, JSONString *key, uint64_t val)
{
    hashmap_t *m = (hashmap_t *) _m;
    map_record_t r;
    r.hash = JSONString_hashCode(key);
    r.k    = key;
    r.v    = val;
    return hashmap_set(m, &r);
}

static void hashmap_api_remove(kmap_t *_m, JSONString *key)
{
    hashmap_t *m = (hashmap_t *) _m;
    unsigned hash = JSONString_hashCode(key);
    map_record_t *r = hashmap_get(m, hash, key);
    if(r) {
        JSON_free(toJSON(ValueP(r->v)));
        r->hash = 0; r->k = 0;
        m->used_size -= 1;
    }
}

static map_record_t *hashmap_api_next(kmap_t *_m, kmap_iterator *itr)
{
    hashmap_t *m = (hashmap_t *) _m;
    unsigned i, size = (m->record_size_mask+1);
    for(i = itr->index; i < size; ++i) {
        map_record_t *r = hashmap_at(m, i);
        if(r->hash) {
            itr->index = i+1;
            return r;
        }
    }
    itr->index = i;
    assert(itr->index == size);
    return NULL;
}

const kmap_api_t HASH_API = {
    hashmap_api_get,
    hashmap_api_set,
    hashmap_api_next,
    hashmap_api_remove,
    hashmap_api_init,
    hashmap_api_dispose
};

/* [DICTMAP] */
static kmap_t *dictmap_init(dictmap_t *m)
{
    int i;
    const size_t allocSize = sizeof(map_record_t)*DICTMAP_THRESHOLD;
    m->base.records = (map_record_t *) malloc(allocSize);
    m->used_size = 0;
    for(i = 0; i < DICTMAP_THRESHOLD; ++i) {
        m->hash_list[i] = 0;
    }
    return (kmap_t *) m;
}

static void dictmap_api_init(kmap_t *_m, unsigned init)
{
    dictmap_init((dictmap_t *) _m);
}

static void dictmap_convert2hashmap(dictmap_t *_m)
{
    hashmap_t *m = (hashmap_t *) _m;
    m->base.api = &HASH_API;
    m->record_size_mask = DICTMAP_THRESHOLD-1;
    hashmap_record_resize(m, DELTA);
}

static inline map_record_t *dictmap_at(dictmap_t *m, unsigned idx)
{
    return (map_record_t *)(m->base.records+idx);
}

static map_status_t dictmap_set_new(dictmap_t *m, map_record_t *rec, int i)
{
    map_record_t *r = dictmap_at(m, i);
    m->hash_list[i] = rec->hash;
    map_record_copy(r, rec);
    ++m->used_size;
    return KMAP_ADDED;
}

static map_status_t dictmap_set(dictmap_t *m, map_record_t *rec)
{
    int i;
    for(i = 0; i < DICTMAP_THRESHOLD; ++i) {
        unsigned hash = m->hash_list[i];
        if(hash == 0) {
            return dictmap_set_new(m, rec, i);
        }
        else if(hash == rec->hash) {
            map_record_t *r = dictmap_at(m, i);
            if(!unlikely(JSONString_equal(r->k, rec->k))) {
                continue;
            }
            JSON_free(toJSON(ValueP(r->v)));
            map_record_copy(r, rec);
            return KMAP_UPDATE;
        }
    }
    dictmap_convert2hashmap(m);
    return hashmap_set((hashmap_t *) m, rec);
}

static map_record_t *dictmap_get(dictmap_t *m, unsigned hash, JSONString *key)
{
    int i;
    for(i = 0; i < DICTMAP_THRESHOLD; ++i) {
        if(hash == m->hash_list[i]) {
            map_record_t *r = dictmap_at(m, i);
            if(JSONString_equal(r->k, key)) {
                return r;
            }
        }
    }
    return NULL;
}

static map_status_t dictmap_api_set(kmap_t *_m, JSONString *key, uint64_t val)
{
    dictmap_t *m = (dictmap_t *)_m;
    map_record_t r;
    r.hash = JSONString_hashCode(key);
    r.k  = key;
    r.v  = val;
    return dictmap_set(m, &r);
}

static void dictmap_api_remove(kmap_t *_m, JSONString *key)
{
    dictmap_t *m = (dictmap_t *)_m;
    unsigned hash = JSONString_hashCode(key);
    map_record_t *r = dictmap_get(m, hash, key);
    if(r) {
        JSON_free(toJSON(ValueP(r->v)));
        r->hash = 0; r->k = 0;
        m->used_size -= 1;
    }
}

static map_record_t *dictmap_api_next(kmap_t *_m, kmap_iterator *itr)
{
    dictmap_t *m = (dictmap_t *)_m;
    unsigned i;
    for(i = itr->index; i < m->used_size; ++i) {
        map_record_t *r = dictmap_at(m, i);
        itr->index = i+1;
        return r;
    }
    itr->index = m->used_size;
    return NULL;
}

static map_record_t *dictmap_api_get(kmap_t *_m, JSONString *key)
{
    dictmap_t *m = (dictmap_t *)_m;
    unsigned hash = JSONString_hashCode(key);
    map_record_t *r = dictmap_get(m, hash, key);
    return r;
}

static void dictmap_api_dispose(kmap_t *_m)
{
    dictmap_t *m = (dictmap_t *)_m;
    unsigned i;
    for(i = 0; i < m->used_size; ++i) {
        if(likely(m->hash_list[i])) {
            map_record_t *r = dictmap_at(m, i);
            JSONString *key = r->k;
            _JSONString_free(key);
            JSON_free(toJSON(ValueP(r->v)));
        }
    }
    free(m->base.records/*, m->used_size * sizeof(map_record_t)*/);
}

const kmap_api_t DICT_API = {
    dictmap_api_get,
    dictmap_api_set,
    dictmap_api_next,
    dictmap_api_remove,
    dictmap_api_init,
    dictmap_api_dispose
};

#ifdef __cplusplus
} /* extern "C" */
#endif
