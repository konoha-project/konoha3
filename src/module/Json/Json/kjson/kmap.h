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

#include <stdint.h>

#ifndef KJSON_MAP_H
#define KJSON_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

struct JSONString;
typedef struct kmap_record {
    unsigned hash;
    struct JSONString *k;
    uint64_t v;
} __attribute__((__aligned__(8))) map_record_t;

struct map_api;
typedef struct map_api kmap_api_t;

struct map_base {
    const kmap_api_t *api;
    map_record_t *records;
} __attribute__ ((packed));

typedef struct hashmap_t {
    struct map_base base;
    unsigned used_size;
    unsigned record_size_mask;
} __attribute__((__aligned__(8))) hashmap_t;

#define DICTMAP_THRESHOLD 3
typedef struct dictmap_t {
    struct map_base base;
    unsigned  used_size;
    unsigned  hash_list[DICTMAP_THRESHOLD];
} __attribute__((__aligned__(8))) dictmap_t;

typedef union kmap_t {
    hashmap_t h;
    dictmap_t d;
} kmap_t;

typedef struct map_iterator {
    uintptr_t index;
} kmap_iterator;

typedef enum map_status_t {
    KMAP_FAILED = 0,
    KMAP_UPDATE = 1,
    KMAP_ADDED  = 2
} map_status_t;

struct map_api {
    map_record_t *(*_get)(kmap_t *m, struct JSONString *key);
    map_status_t  (*_set)(kmap_t *m, struct JSONString *key, uint64_t val);
    map_record_t *(*_next)(kmap_t *m, kmap_iterator *itr);
    void (*_remove)(kmap_t *m, struct JSONString *key);
    void (*_init)(kmap_t *m, unsigned init);
    void (*_dispose)(kmap_t *m);
};

/* [kmap inline] */

static inline void kmap_dispose(kmap_t *m)
{
    m->h.base.api->_dispose(m);
}

static inline map_record_t *kmap_get(kmap_t *m, struct JSONString *key)
{
    return m->h.base.api->_get(m, key);
}

static inline map_status_t kmap_set(kmap_t *m, struct JSONString *key, uint64_t val)
{
    return m->h.base.api->_set(m, key, val);
}

static inline void kmap_remove(kmap_t *m, struct JSONString *key)
{
    return m->h.base.api->_remove(m, key);
}

static inline map_record_t *kmap_next(kmap_t *m, kmap_iterator *itr)
{
    return m->h.base.api->_next(m, itr);
}

static inline unsigned kmap_size(kmap_t *m)
{
    return m->h.used_size;
}

extern const kmap_api_t DICT_API;
extern const kmap_api_t HASH_API;
static inline void kmap_init(kmap_t *m, unsigned init)
{
    const kmap_api_t *api = (init > DICTMAP_THRESHOLD) ? &HASH_API:&DICT_API;
    m->h.base.api = api;
    api->_init(m, init);
}

#ifdef __cplusplus
}
#endif
#endif /* end of include guard */
