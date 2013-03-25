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
typedef void *Cache;
typedef void *Stack;
typedef void *TestFunc;
typedef void *IArray;
typedef void *Address;
typedef kMethod *kMethodPtr;
typedef kObject *kObjectPtr;

//#define DEBUG_BYTECODE 1

#ifdef DEBUG_BYTECODE
typedef struct LIRHeader
#else
typedef union LIRHeader
#endif
{
	unsigned opcode;
	void *codeaddr;
} LIRHeader;

#define PACKED /*__attribute__((packed))*/
#include "LIR.h"

typedef union ByteCode {
#define DEFINE_ABSTRAKClass_INST(X) OP##X X;
	BYTECODE_LIST(DEFINE_ABSTRAKClass_INST)
#undef DEFINE_ABSTRAKClass_INST
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

#define FUELVM_REGISTER_SIZE 512/*FIXME Implement clever register allocation algorithm */
#define FUELVM_BYTECODE_MAGICNUMBER 31416U

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* end of include guard */
