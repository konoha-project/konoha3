#include <assert.h>
#include <stdio.h>
#include "bitmap.h"

void testBitMap_findNextUnsetBit()
{
	BitMap mapbuf, *map = &mapbuf;
	BitMap_Init(map, 1);
	unsigned i, Idx;
	for(i = 0; i < 16; i++) {
		Idx = BitMap_findNextUnsetBit(map);
		assert(Idx == i);
		assert(BitMap_get(map, Idx) == false);
		BitMap_set(map, Idx);
	}
	BitMap_flip(map, 32);
	Idx = BitMap_findNextUnsetBit(map);
	assert(Idx == 16);
	BitMap_flip(map, 10);
	Idx = BitMap_findNextUnsetBit(map);
	assert(Idx == 10);
	BitMap_Dispose(map);
}

int main(int argc, char const* argv[])
{
	BitMap map;
	BitMap_Init(&map, 1);
	BitMap_set(&map, 0);
	assert(BitMap_get(&map, 0) == true);
	assert(BitMap_get(&map, 1) == false);
	BitMap_flip(&map, 0);
	assert(BitMap_get(&map, 0) == false);
	BitMap_Dispose(&map);

	testBitMap_findNextUnsetBit();
	return 0;
}
