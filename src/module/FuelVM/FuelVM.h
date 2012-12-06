#include "vmcommon.h"

#ifndef FUEL_VM_H
#define FUEL_VM_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef int  VMRegister;
typedef enum TypeId TypeId;
typedef SValue (*Method)(void *Context, SValue *Args);
typedef void *Cache;
typedef void *Stack;
typedef void *TestFunc;
typedef void *IArray;
typedef void *Address;
typedef kMethod *kMethodPtr;

typedef union LIRHeader {
	unsigned opcode;
	void *codeaddr;
} LIRHeader;

#define PACKED /*__attribute__((packed))*/
#include "LIR.h"

typedef union ByteCode {
#define DEFINE_ABSTRACT_INST(X) OP##X X;
	BYTECODE_LIST(DEFINE_ABSTRACT_INST)
#undef DEFINE_ABSTRACT_INST
} ByteCode;

typedef enum OPCODE {
#define DEFINE_ENUM_OPCODE(X) X##_ = OPCODE_##X,
	BYTECODE_LIST(DEFINE_ENUM_OPCODE)
#undef DEFINE_ENUM_OPCODE
	OPCODE_MAX
} OPCODE;

#define ByteCode_Header(OP) ((OP)->Header)

static inline OPCODE GetOpcode(ByteCode *code)
{
	return (OPCODE) ((LIRHeader *)(code))->opcode;
}

extern void ByteCode_Dump(ByteCode *code);
extern void FuelVM_Exec(KonohaContext *kctx, KonohaStack *Stack, ByteCode *code);

extern void FuelVM_GenerateLLVMIR(KonohaContext *kctx, kMethod *mtd, kBlock *block, int option);
extern void RecompileMethod(KonohaContext *kctx, kMethod *mtd);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* end of include guard */
