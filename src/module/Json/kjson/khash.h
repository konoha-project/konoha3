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
#ifndef KHASH_H
#define KHASH_H

#ifdef __cplusplus
extern "C" {
#endif

//#define USE_SUPERFASTHASH
#define USE_FNV1A
//#define USE_DJBHASH
//#define USE_ONE_AT_A_TIME
//
#if !defined(HASH) && defined(USE_FNV1A)
#define HASH(STR, LEN) fnv1a(STR, LEN)
static uint32_t fnv1a_string(const uint8_t *s, uint32_t len, uint32_t hash)
{
    const uint8_t *e = s + len;
    while(s < e) {
        hash = (*s++ ^ hash) * 0x01000193;
    }
    return hash;
}

static uint32_t fnv1a(const char *p, uint32_t len)
{
    const uint8_t *str = (const uint8_t *) p;
    uint32_t hash = 0x811c9dc5;
#define UNROLL 4
    while(len >= UNROLL) {
      hash = fnv1a_string(str, UNROLL, hash);
      str += UNROLL;
      len -= UNROLL;
    }
    return fnv1a_string(str, len, hash);
}
#endif

#if !defined(HASH) && defined(USE_SUPERFASTHASH)
/* By Paul Hsieh (C) 2004, 2005.  Covered under the Paul Hsieh derivative
 * license. See:
 * http://www.azillionmonkeys.com/qed/weblicense.html for license details.
 * http://www.azillionmonkeys.com/qed/hash.html */

#define HASH(STR, LEN) SuperFastHash(STR, LEN)
#undef get16bits
#if(defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

static uint32_t SuperFastHash(const char * data, int len)
{
    uint32_t hash = len, tmp;
    int rem;

    if(len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for(;len > 0; len--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2 * sizeof (uint16_t);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch(rem) {
        case 3: hash += get16bits (data);
                hash ^= hash << 16;
                hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += (signed char)*data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}
#endif

#if !defined(HASH) && defined(USE_DJBHASH)
#define HASH(STR, LEN) djbhash(STR, LEN)
static uint32_t djbhash(const char *p, uint32_t len)
{
    uint32_t hash = 5381;
    const uint8_t *s = (const uint8_t *) p;
    const uint8_t *e = (const uint8_t *) p + len;
    while(s < e) {
        hash = ((hash << 5) + hash) + *s++;
    }
    return hash;
}
#endif

#if !defined(HASH) && defined(USE_ONE_AT_A_TIME)
#define HASH(STR, LEN) one_at_a_time(STR, LEN)
static uint32_t one_at_a_time(const char *p, uint32_t len)
{
    uint32_t i, hash = 0;

    for (i = 0; i < len; ++i) {
        hash += p[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* end of include guard */
