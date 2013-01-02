#include "bitmap.h"

#ifndef REGALLOC_H
#define REGALLOC_H

#define REGISTER_UNDEFINED ((unsigned)-1)

static unsigned BitMap_findAndSetNextUnsetBit(BitMap *map)
{
	unsigned Idx = BitMap_findNextUnsetBit(map);
	BitMap_set(map, Idx);
	return Idx;
}

typedef struct RegisterAllocator {
	BitMap/*<VariableId, bool>*/ AllocatedVariable;
	BitMap/*<Register, bool>*/   AllocatedRegister;
	ARRAY(uintptr_t)/* <VariableId, Register> */ RegTable;
} RegisterAllocator;

static void RegisterAllocator_Init(RegisterAllocator *RegAlloc, unsigned MaxVariables)
{
	BitMap_Init(&(RegAlloc->AllocatedVariable), MaxVariables);
	BitMap_Init(&(RegAlloc->AllocatedRegister), MaxVariables/2);
	ARRAY_init(uintptr_t, &RegAlloc->RegTable, 0);
}

static void RegisterAllocator_Dispose(RegisterAllocator *RegAlloc)
{
	BitMap_Dispose(&RegAlloc->AllocatedVariable);
	BitMap_Dispose(&RegAlloc->AllocatedRegister);
	ARRAY_dispose(uintptr_t, &RegAlloc->RegTable);
}

static unsigned Register_Allocate(RegisterAllocator *RegAlloc, unsigned Id)
{
	unsigned Reg;
	assert(BitMap_get(&RegAlloc->AllocatedVariable, Id) == false);
	BitMap_set(&RegAlloc->AllocatedVariable, Id);
	Reg = BitMap_findAndSetNextUnsetBit(&RegAlloc->AllocatedRegister);
	ARRAY_safe_set(uintptr_t, &RegAlloc->RegTable, Id, Reg);
	assert(Reg < FUELVM_REGISTER_SIZE);
	return Reg;
}

static void Register_Deallocate(RegisterAllocator *RegAlloc, unsigned Id)
{
	assert(BitMap_get(&RegAlloc->AllocatedVariable, Id) == true);
	unsigned Reg = ARRAY_get(uintptr_t, &RegAlloc->RegTable, Id);
	BitMap_flip(&RegAlloc->AllocatedRegister, Reg);
	BitMap_flip(&RegAlloc->AllocatedVariable, Id);
}

static bool Register_FindById(RegisterAllocator *RegAlloc, unsigned Id, unsigned *Reg)
{
	if(BitMap_get(&RegAlloc->AllocatedVariable, Id)) {
		*Reg = ARRAY_get(uintptr_t,&RegAlloc->RegTable, Id);
		return true;
	}
	return false;
}

static inline unsigned Register_FindByIdOrAllocate(RegisterAllocator *RegAlloc, unsigned Id)
{
	unsigned Reg = REGISTER_UNDEFINED;
	if(Register_FindById(RegAlloc, Id, &Reg)) {
		return Reg;
	}
	return Register_Allocate(RegAlloc, Id);
}

#endif /* end of include guard */
