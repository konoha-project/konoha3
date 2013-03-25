/****************************************************************************
 * Copyright (c) 2012-2013, Masahiro Ide <ide@konohascript.org> All rights reserved.
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

#include "karray.h"
#include <stdint.h>

#ifndef BITMAP_H
#define BITMAP_H

#ifdef __cplusplus
extern "C" {
#endif

DEF_ARRAY_STRUCT(uintptr_t);
DEF_ARRAY_T(uintptr_t);
DEF_ARRAY_OP_NOPOINTER(uintptr_t);

#define BITS ((sizeof(unsigned long)) * 8)
typedef struct BitMap {
	ARRAY(uintptr_t) map;
} BitMap;

static inline void BitMap_resize(BitMap *map, unsigned newSize)
{
	assert(newSize >= BITS);
	int size = (1 << LOG2(newSize)) / BITS;
	while(size > 0) {
		ARRAY_add(uintptr_t, &map->map, 0);
		size--;
	}
}

static inline void BitMap_Init(BitMap *map, unsigned size)
{
	unsigned newSize = (size < BITS) ? BITS : size;
	ARRAY_init(uintptr_t, &map->map, newSize / BITS);
	BitMap_resize(map, newSize);
}

static inline void BitMap_Dispose(BitMap *map)
{
	ARRAY_dispose(uintptr_t, &map->map);
}

#define BitMap_reference(Map, Index) (ARRAY_n((Map)->map, ((Index) / BITS)))

static inline void BitMap_set(BitMap *map, unsigned index)
{
	uintptr_t mask = 1UL << (index % BITS);
	*BitMap_reference(map, index) |= mask;
}

static inline bool BitMap_get(BitMap *map, unsigned index)
{
	uintptr_t mask = 1UL << (index % BITS);
	return (*BitMap_reference(map, index) & mask) != 0;
}

static inline void BitMap_flip(BitMap *map, unsigned index)
{
	uintptr_t mask = 1UL << (index % BITS);
	*BitMap_reference(map, index) ^= mask;
}

static inline unsigned BitMap_findNextUnsetBit(BitMap *map)
{
	unsigned i;
	uintptr_t *x;
	FOR_EACH_ARRAY_(map->map, x, i) {
		if(*x != UINTPTR_MAX) {
			/* scan for the first unset bit */
			unsigned offset = (*x != 0) ? (__builtin_ctzll(~(*x))) : 0;
			return i * BITS + offset;
		}
	}
	unsigned oldSize = ARRAY_size(map->map);
	ARRAY_add(uintptr_t, &map->map, 0);
	return oldSize * BITS;
}

static inline int BitMap_findFirst(BitMap *map) {
	unsigned i;
	uintptr_t *x;
	FOR_EACH_ARRAY_(map->map, x, i) {
		if(*x != 0)
			return i * BITS + __builtin_ctzll(*x);
	}
	return -1;
}

#undef BITS
#undef BitMap_reference

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* end of include guard */
