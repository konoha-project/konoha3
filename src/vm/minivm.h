/****************************************************************************
 * Copyright (c) 2012, the Konoha project authors. All rights reserved.
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
#ifndef MINIVM_H
#define MINIVM_H
// THIS FILE WAS AUTOMATICALLY GENERATED

#ifndef __GNUC__
#ifdef K_USING_THCODE_
#undef K_USING_THCODE_
#endif
#endif

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
	VirtualMachineInstruction  *jumppc;
} OPJMP;

#define OPCODE_JMPF ((kopcode_t)16)
typedef struct OPJMPF {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
} OPJMPF;

#define OPCODE_TRYJMP ((kopcode_t)17)
typedef struct OPTRYJMP {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
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
/* [common] */

/* ------------------------------------------------------------------------ */
/* [data] */

#define _CONST 1
#define _JIT   (1<<1)
#define _DEF   (1<<2)
typedef struct {
	const char *name;
	kshortflag_t   flag;
	kushort_t size;
	kushort_t types[6];
} kOPDATA_t;

static const kOPDATA_t OPDATA[] = {
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
	assert(sizeof(OPNOP) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPTHCODE) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPENTER) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPEXIT) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPNSET) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPNMOV) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPNMOVx) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPXNMOV) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPNEW) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPNULL) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPLOOKUP) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPCALL) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPRET) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPNCALL) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPBNOT) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPJMP) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPJMPF) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPTRYJMP) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPYIELD) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPERROR) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPSAFEPOINT) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPCHKSTACK) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(OPTRACE) <= sizeof(VirtualMachineInstruction));
}

static const char *T_opcode(kopcode_t opcode)
{
	return OPDATA[opcode].name;
}

#ifdef OLD
static size_t kopcode_size(kopcode_t opcode)
{
	return OPDATA[opcode].size;
}

static kbool_t kopcode_hasjump(kopcode_t opcode)
{
	return (OPDATA[opcode].types[0] == VMT_ADDR);
}
#endif

/* ------------------------------------------------------------------------ */



/* ------------------------------------------------------------------------ */
/* [exec] */


//#if (defined(K_USING_LINUX_) && (defined(__i386__) || defined(__x86_64__)) && (defined(__GNUC__) && __GNUC__ >= 3))
//#define K_USING_VMASMDISPATCH 1
//#endif

#ifdef K_USING_THCODE_
#define CASE(x)  L_##x : 
#define NEXT_OP   (pc->codeaddr)
#define JUMP      *(NEXT_OP)
#ifdef K_USING_VMASMDISPATCH
#define GOTO_NEXT()     \
	asm volatile("jmp *%0;": : "g"(NEXT_OP));\
	goto *(NEXT_OP)

#else
#define GOTO_NEXT()     goto *(NEXT_OP)
#endif
#define TC(c) 
#define DISPATCH_START(pc) goto *OPJUMP[pc->opcode]
#define DISPATCH_END(pc)
#define GOTO_PC(pc)        GOTO_NEXT()
#else/*K_USING_THCODE_*/
#define OPJUMP      NULL
#define CASE(x)     case OPCODE_##x : 
#define NEXT_OP     L_HEAD
#define GOTO_NEXT() goto NEXT_OP
#define JUMP        L_HEAD
#define TC(c)
#define DISPATCH_START(pc) L_HEAD:;switch(pc->opcode) {
#define DISPATCH_END(pc)   } /*KNH_DIE("unknown opcode=%d", (int)pc->opcode)*/; 
#define GOTO_PC(pc)         GOTO_NEXT()
#endif/*K_USING_THCODE_*/

static VirtualMachineInstruction* KonohaVirtualMachine_run(KonohaContext *kctx, KonohaStack *sfp0, VirtualMachineInstruction *pc)
{
#ifdef K_USING_THCODE_
	static void *OPJUMP[] = {
		&&L_NOP, &&L_THCODE, &&L_ENTER, &&L_EXIT, 
		&&L_NSET, &&L_NMOV, &&L_NMOVx, &&L_XNMOV, 
		&&L_NEW, &&L_NULL, &&L_LOOKUP, &&L_CALL, 
		&&L_RET, &&L_NCALL, &&L_BNOT, &&L_JMP, 
		&&L_JMPF, &&L_TRYJMP, &&L_YIELD, &&L_ERROR, 
		&&L_SAFEPOINT, &&L_CHKSTACK, &&L_TRACE, 
	};
#endif
	krbp_t *rbp = (krbp_t*)sfp0;
	DISPATCH_START(pc);

	CASE(NOP) {
		OPNOP *op = (OPNOP*)pc;
		OPEXEC_NOP(); pc++;
		GOTO_NEXT();
	} 
	CASE(THCODE) {
		OPTHCODE *op = (OPTHCODE*)pc;
		OPEXEC_THCODE(op->threadCode); pc++;
		GOTO_NEXT();
	} 
	CASE(ENTER) {
		OPENTER *op = (OPENTER*)pc;
		OPEXEC_ENTER(); pc++;
		GOTO_NEXT();
	} 
	CASE(EXIT) {
		OPEXIT *op = (OPEXIT*)pc;
		OPEXEC_EXIT(); pc++;
		GOTO_NEXT();
	} 
	CASE(NSET) {
		OPNSET *op = (OPNSET*)pc;
		OPEXEC_NSET(op->a, op->n, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(NMOV) {
		OPNMOV *op = (OPNMOV*)pc;
		OPEXEC_NMOV(op->a, op->b, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(NMOVx) {
		OPNMOVx *op = (OPNMOVx*)pc;
		OPEXEC_NMOVx(op->a, op->b, op->bx, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(XNMOV) {
		OPXNMOV *op = (OPXNMOV*)pc;
		OPEXEC_XNMOV(op->a, op->ax, op->b, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(NEW) {
		OPNEW *op = (OPNEW*)pc;
		OPEXEC_NEW(op->a, op->p, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(NULL) {
		OPNULL *op = (OPNULL*)pc;
		OPEXEC_NULL(op->a, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(LOOKUP) {
		OPLOOKUP *op = (OPLOOKUP*)pc;
		OPEXEC_LOOKUP(op->thisidx, op->ns, op->mtd); pc++;
		GOTO_NEXT();
	} 
	CASE(CALL) {
		OPCALL *op = (OPCALL*)pc;
		OPEXEC_CALL(op->uline, op->thisidx, op->espshift, op->tyo); pc++;
		GOTO_NEXT();
	} 
	CASE(RET) {
		OPRET *op = (OPRET*)pc;
		OPEXEC_RET(); pc++;
		GOTO_NEXT();
	} 
	CASE(NCALL) {
		OPNCALL *op = (OPNCALL*)pc;
		OPEXEC_NCALL(); pc++;
		GOTO_NEXT();
	} 
	CASE(BNOT) {
		OPBNOT *op = (OPBNOT*)pc;
		OPEXEC_BNOT(op->c, op->a); pc++;
		GOTO_NEXT();
	} 
	CASE(JMP) {
		OPJMP *op = (OPJMP*)pc;
		OPEXEC_JMP(pc = op->jumppc, JUMP); pc++;
		GOTO_NEXT();
	} 
	CASE(JMPF) {
		OPJMPF *op = (OPJMPF*)pc;
		OPEXEC_JMPF(pc = op->jumppc, JUMP, op->a); pc++;
		GOTO_NEXT();
	} 
	CASE(TRYJMP) {
		OPTRYJMP *op = (OPTRYJMP*)pc;
		OPEXEC_TRYJMP(pc = op->jumppc, JUMP); pc++;
		GOTO_NEXT();
	} 
	CASE(YIELD) {
		OPYIELD *op = (OPYIELD*)pc;
		OPEXEC_YIELD(); pc++;
		GOTO_NEXT();
	} 
	CASE(ERROR) {
		OPERROR *op = (OPERROR*)pc;
		OPEXEC_ERROR(op->uline, op->msg, op->esp); pc++;
		GOTO_NEXT();
	} 
	CASE(SAFEPOINT) {
		OPSAFEPOINT *op = (OPSAFEPOINT*)pc;
		OPEXEC_SAFEPOINT(op->uline, op->esp); pc++;
		GOTO_NEXT();
	} 
	CASE(CHKSTACK) {
		OPCHKSTACK *op = (OPCHKSTACK*)pc;
		OPEXEC_CHKSTACK(op->uline); pc++;
		GOTO_NEXT();
	} 
	CASE(TRACE) {
		OPTRACE *op = (OPTRACE*)pc;
		OPEXEC_TRACE(op->uline, op->thisidx, op->trace); pc++;
		GOTO_NEXT();
	} 
	DISPATCH_END(pc);
	L_RETURN:;
	return pc;
}

#endif /* MINIVM_H */
