/****************************************************************************
 * Copyright (c) 2012, the Konoha project authors. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: *
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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>

#ifndef KONOHA_VM_H_
#define KONOHA_VM_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _MSC_VER
#define USE_DIRECT_THREADED_CODE
#endif

typedef struct kBasicBlockVar         kBasicBlock;
typedef const struct kByteCodeVar     kByteCode;
typedef struct kByteCodeVar           kByteCodeVar;

#define ctxcode          ((ctxcode_t *)kctx->modlocal[MOD_code])
#define kmodcode         ((KModuleByteCode *)kctx->modshare[MOD_code])
#define CT_BasicBlock    kmodcode->cBasicBlock
#define TY_BasicBlock    kmodcode->cBasicBlock->typeId
#define CT_ByteCode      kmodcode->cByteCode

#define IS_BasicBlock(O)  (O_ct(O) == CT_BasicBlock)
#define IS_ByteCode(O)    (O_ct(O) == CT_ByteCode)

#define CODE_ENTER        kmodcode->PRECOMPILED_ENTER
#define CODE_NCALL        kmodcode->PRECOMPILED_NCALL

typedef struct {
	KonohaModule     header;
	KonohaClass     *cBasicBlock;
	KonohaClass     *cByteCode;
	kByteCode       *codeNull;
	struct VirtualCode  *PRECOMPILED_ENTER;
	struct VirtualCode  *PRECOMPILED_NCALL;
} KModuleByteCode;

typedef struct {
	KonohaModuleContext      h;
	kfileline_t      uline;
	kArray          *codeList;
	kBasicBlock     *lbINIT; // ON GCSTACK
	kBasicBlock     *lbEND;  // ON GCSTACK
	kArray          *constPools;
	kBasicBlock     *currentWorkingBlock;
} ctxcode_t;

/* ------------------------------------------------------------------------ */
/* KCODE */

typedef uintptr_t  kopcode_t;
typedef intptr_t   kreg_t;
typedef intptr_t   ksfpidx_t;
typedef intptr_t   kregO_t;
typedef intptr_t   kregN_t;
typedef struct ksfx_t {
	ksfpidx_t i;
	ksfpidx_t n;
} ksfx_t;

typedef void (*ThreadCodeFunc)(KonohaContext *kctx, struct VirtualCode *, void**);
typedef void (*TraceFunc)(KonohaContext *kctx, KonohaStack *sfp, KTraceInfo *trace);

typedef struct {
	kMethod *mtd;
	ktype_t typeId; kparamId_t signature;
} kMethodInlineCache;

#if defined(USE_DIRECT_THREADED_CODE)
#define KCODE_HEAD \
	void *codeaddr; \
	size_t count; \
	kushort_t opcode; \
	kfileline_t line

#else
#define KCODE_HEAD \
	size_t count; \
	kopcode_t opcode; \
	kfileline_t line  \

#endif/*USE_DIRECT_THREADED_CODE*/

typedef struct VirtualCode {
	KCODE_HEAD;
	union {
		intptr_t data[5];
		void *p[5];
		kObject *o[5];
		KonohaClass *ct[5];
		char *u[5];
	};
} VirtualCode;

/* ------------------------------------------------------------------------ */

#define BasicBlock_isVisited(o)      (TFLAG_is(uintptr_t,(o)->h.magicflag,kObject_Local1))
#define BasicBlock_setVisited(o,B)   TFLAG_set(uintptr_t,((kObjectVar *)o)->h.magicflag,kObject_Local1,B)

struct kBasicBlockVar {
	KonohaObjectHeader h;
	kushort_t id;     kushort_t incoming;
	KGrowingArray codeTable;
	kBasicBlock        *nextBlock;
	kBasicBlock        *branchBlock;
	VirtualCode *code;
	VirtualCode *opjmp;
};

struct kByteCodeVar {
	KonohaObjectHeader h;
	VirtualCode*   code;
	size_t    codesize;
	kString  *source;
	kfileline_t   fileid;
};

//-------------------------------------------------------------------------

#define OPCODE_NOP ((kopcode_t)0)
typedef struct OPNOP {
	KCODE_HEAD;
} OPNOP;

#define OPCODE_THCODE ((kopcode_t)1)
typedef struct OPTHCODE {
	KCODE_HEAD;
	ThreadCodeFunc threadCode;
} OPTHCODE;

#define OPCODE_ENTER ((kopcode_t)2)
typedef struct OPENTER {
	KCODE_HEAD;
} OPENTER;

#define OPCODE_EXIT ((kopcode_t)3)
typedef struct OPEXIT {
	KCODE_HEAD;
} OPEXIT;

#define OPCODE_NSET ((kopcode_t)4)
typedef struct OPNSET {
	KCODE_HEAD;
	kreg_t a;
	kint_t n;
	KonohaClass* ty;
} OPNSET;

#define OPCODE_NMOV ((kopcode_t)5)
typedef struct OPNMOV {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	KonohaClass* ty;
} OPNMOV;

#define OPCODE_NMOVx ((kopcode_t)6)
typedef struct OPNMOVx {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	uintptr_t bx;
	KonohaClass* ty;
} OPNMOVx;

#define OPCODE_XNMOV ((kopcode_t)7)
typedef struct OPXNMOV {
	KCODE_HEAD;
	kreg_t a;
	uintptr_t ax;
	kreg_t b;
	KonohaClass* ty;
} OPXNMOV;

#define OPCODE_NEW ((kopcode_t)8)
typedef struct OPNEW {
	KCODE_HEAD;
	kreg_t a;
	uintptr_t p;
	KonohaClass* ty;
} OPNEW;

#define OPCODE_NULL ((kopcode_t)9)
typedef struct OPNULL {
	KCODE_HEAD;
	kreg_t a;
	KonohaClass* ty;
} OPNULL;

#define OPCODE_LOOKUP ((kopcode_t)10)
typedef struct OPLOOKUP {
	KCODE_HEAD;
	kreg_t thisidx;
	kNameSpace* ns;
	kMethod* mtd;
} OPLOOKUP;

#define OPCODE_CALL ((kopcode_t)11)
typedef struct OPCALL {
	KCODE_HEAD;
	uintptr_t uline;
	kreg_t thisidx;
	kreg_t espshift;
	kObject* tyo;
} OPCALL;

#define OPCODE_RET ((kopcode_t)12)
typedef struct OPRET {
	KCODE_HEAD;
} OPRET;

#define OPCODE_NCALL ((kopcode_t)13)
typedef struct OPNCALL {
	KCODE_HEAD;
} OPNCALL;

#define OPCODE_BNOT ((kopcode_t)14)
typedef struct OPBNOT {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
} OPBNOT;

#define OPCODE_JMP ((kopcode_t)15)
typedef struct OPJMP {
	KCODE_HEAD;
	VirtualCode  *jumppc;
} OPJMP;

#define OPCODE_JMPF ((kopcode_t)16)
typedef struct OPJMPF {
	KCODE_HEAD;
	VirtualCode  *jumppc;
	kreg_t a;
} OPJMPF;

#define OPCODE_TRYJMP ((kopcode_t)17)
typedef struct OPTRYJMP {
	KCODE_HEAD;
	VirtualCode  *jumppc;
} OPTRYJMP;

#define OPCODE_YIELD ((kopcode_t)18)
typedef struct OPYIELD {
	KCODE_HEAD;
} OPYIELD;

#define OPCODE_ERROR ((kopcode_t)19)
typedef struct OPERROR {
	KCODE_HEAD;
	uintptr_t uline;
	kString* msg;
	kreg_t esp;
} OPERROR;

#define OPCODE_SAFEPOINT ((kopcode_t)20)
typedef struct OPSAFEPOINT {
	KCODE_HEAD;
	uintptr_t uline;
	kreg_t esp;
} OPSAFEPOINT;

#define OPCODE_CHKSTACK ((kopcode_t)21)
typedef struct OPCHKSTACK {
	KCODE_HEAD;
	uintptr_t uline;
} OPCHKSTACK;

#define OPCODE_TRACE ((kopcode_t)22)
typedef struct OPTRACE {
	KCODE_HEAD;
	uintptr_t uline;
	kreg_t thisidx;
	TraceFunc trace;
} OPTRACE;

#define KOPCODE_MAX ((kopcode_t)23)

#define VMT_VOID       0
#define VMT_ADDR       1
#define VMT_R          2
#define VMT_RN         2
#define VMT_RO         2
#define VMT_U          3
#define VMT_I          4
#define VMT_CID        5
#define VMT_CO         6
#define VMT_INT        7
#define VMT_FLOAT      8
#define VMT_HCACHE     9
#define VMT_F         10/*function*/
#define VMT_STRING    11
#define VMT_METHOD    12
#define VMT_NAMESPACE 13


/* ------------------------------------------------------------------------ */
/* [data] */

typedef struct {
	const char *name;
	kshortflag_t   flag;
	kushort_t size;
	kushort_t types[6];
} DEFINE_OPSPEC;

static const DEFINE_OPSPEC OPDATA[] = {
	{"NOP", 0, 0, { VMT_VOID}}, 
	{"THCODE", 0, 1, { VMT_F, VMT_VOID}}, 
	{"ENTER", 0, 0, { VMT_VOID}}, 
	{"EXIT", 0, 0, { VMT_VOID}}, 
	{"NSET", 0, 3, { VMT_RN, VMT_INT, VMT_CID, VMT_VOID}}, 
	{"NMOV", 0, 3, { VMT_RN, VMT_RN, VMT_CID, VMT_VOID}}, 
	{"NMOVx", 0, 4, { VMT_RN, VMT_RO, VMT_U, VMT_CID, VMT_VOID}}, 
	{"XNMOV", 0, 4, { VMT_RO, VMT_U, VMT_RN, VMT_CID, VMT_VOID}}, 
	{"NEW", 0, 3, { VMT_RO, VMT_U, VMT_CID, VMT_VOID}}, 
	{"NULL", 0, 2, { VMT_RO, VMT_CID, VMT_VOID}}, 
	{"LOOKUP", 0, 3, { VMT_RO, VMT_NAMESPACE, VMT_METHOD, VMT_VOID}}, 
	{"CALL", 0, 4, { VMT_U, VMT_RO, VMT_RO, VMT_CO, VMT_VOID}}, 
	{"RET", 0, 0, { VMT_VOID}}, 
	{"NCALL", 0, 0, { VMT_VOID}}, 
	{"BNOT", 0, 2, { VMT_RN, VMT_RN, VMT_VOID}}, 
	{"JMP", 0, 1, { VMT_ADDR, VMT_VOID}}, 
	{"JMPF", 0, 2, { VMT_ADDR, VMT_RN, VMT_VOID}}, 
	{"TRYJMP", 0, 1, { VMT_ADDR, VMT_VOID}}, 
	{"YIELD", 0, 0, { VMT_VOID}}, 
	{"ERROR", 0, 3, { VMT_U, VMT_STRING, VMT_RO, VMT_VOID}}, 
	{"SAFEPOINT", 0, 2, { VMT_U, VMT_RO, VMT_VOID}}, 
	{"CHKSTACK", 0, 1, { VMT_U, VMT_VOID}}, 
	{"TRACE", 0, 3, { VMT_U, VMT_RO, VMT_F, VMT_VOID}}, 
};

static void opcode_check(void)
{
	assert(sizeof(OPNOP) <= sizeof(VirtualCode));
	assert(sizeof(OPTHCODE) <= sizeof(VirtualCode));
	assert(sizeof(OPENTER) <= sizeof(VirtualCode));
	assert(sizeof(OPEXIT) <= sizeof(VirtualCode));
	assert(sizeof(OPNSET) <= sizeof(VirtualCode));
	assert(sizeof(OPNMOV) <= sizeof(VirtualCode));
	assert(sizeof(OPNMOVx) <= sizeof(VirtualCode));
	assert(sizeof(OPXNMOV) <= sizeof(VirtualCode));
	assert(sizeof(OPNEW) <= sizeof(VirtualCode));
	assert(sizeof(OPNULL) <= sizeof(VirtualCode));
	assert(sizeof(OPLOOKUP) <= sizeof(VirtualCode));
	assert(sizeof(OPCALL) <= sizeof(VirtualCode));
	assert(sizeof(OPRET) <= sizeof(VirtualCode));
	assert(sizeof(OPNCALL) <= sizeof(VirtualCode));
	assert(sizeof(OPBNOT) <= sizeof(VirtualCode));
	assert(sizeof(OPJMP) <= sizeof(VirtualCode));
	assert(sizeof(OPJMPF) <= sizeof(VirtualCode));
	assert(sizeof(OPTRYJMP) <= sizeof(VirtualCode));
	assert(sizeof(OPYIELD) <= sizeof(VirtualCode));
	assert(sizeof(OPERROR) <= sizeof(VirtualCode));
	assert(sizeof(OPSAFEPOINT) <= sizeof(VirtualCode));
	assert(sizeof(OPCHKSTACK) <= sizeof(VirtualCode));
	assert(sizeof(OPTRACE) <= sizeof(VirtualCode));
}

static const char *T_opcode(kopcode_t opcode)
{
	return OPDATA[opcode].name;
}

#ifdef __cplusplus
}
#endif

#endif /*KONOHA_VM_H_*/
