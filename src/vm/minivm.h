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


#define OPCODE_NOP ((kopcode_t)0)
typedef struct klr_NOP_t {
	KCODE_HEAD;
} klr_NOP_t;

#define OPCODE_THCODE ((kopcode_t)1)
typedef struct klr_THCODE_t {
	KCODE_HEAD;
	klr_Fth th;
} klr_THCODE_t;

#define OPCODE_ENTER ((kopcode_t)2)
typedef struct klr_ENTER_t {
	KCODE_HEAD;
} klr_ENTER_t;

#define OPCODE_EXIT ((kopcode_t)3)
typedef struct klr_EXIT_t {
	KCODE_HEAD;
} klr_EXIT_t;

#define OPCODE_NSET ((kopcode_t)4)
typedef struct klr_NSET_t {
	KCODE_HEAD;
	kreg_t a;
	kint_t n;
	kclass_t* ty;
} klr_NSET_t;

#define OPCODE_NMOV ((kopcode_t)5)
typedef struct klr_NMOV_t {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	kclass_t* ty;
} klr_NMOV_t;

#define OPCODE_NMOVx ((kopcode_t)6)
typedef struct klr_NMOVx_t {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	uintptr_t bx;
	kclass_t* ty;
} klr_NMOVx_t;

#define OPCODE_XNMOV ((kopcode_t)7)
typedef struct klr_XNMOV_t {
	KCODE_HEAD;
	kreg_t a;
	uintptr_t ax;
	kreg_t b;
	kclass_t* ty;
} klr_XNMOV_t;

#define OPCODE_NEW ((kopcode_t)8)
typedef struct klr_NEW_t {
	KCODE_HEAD;
	kreg_t a;
	uintptr_t p;
	kclass_t* ty;
} klr_NEW_t;

#define OPCODE_NULL ((kopcode_t)9)
typedef struct klr_NULL_t {
	KCODE_HEAD;
	kreg_t a;
	kclass_t* ty;
} klr_NULL_t;

#define OPCODE_BOX ((kopcode_t)10)
typedef struct klr_BOX_t {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	kclass_t* ty;
} klr_BOX_t;

#define OPCODE_UNBOX ((kopcode_t)11)
typedef struct klr_UNBOX_t {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	kclass_t* ty;
} klr_UNBOX_t;

#define OPCODE_CALL ((kopcode_t)12)
typedef struct klr_CALL_t {
	KCODE_HEAD;
	uintptr_t uline;
	kreg_t thisidx;
	kreg_t espshift;
	kObject* tyo;
} klr_CALL_t;

#define OPCODE_SCALL ((kopcode_t)13)
typedef struct klr_SCALL_t {
	KCODE_HEAD;
	uintptr_t uline;
	kreg_t thisidx;
	kreg_t espshift;
	kMethod* mtd;
	kObject* tyo;
} klr_SCALL_t;

#define OPCODE_RET ((kopcode_t)14)
typedef struct klr_RET_t {
	KCODE_HEAD;
} klr_RET_t;

#define OPCODE_NCALL ((kopcode_t)15)
typedef struct klr_NCALL_t {
	KCODE_HEAD;
} klr_NCALL_t;

#define OPCODE_BNOT ((kopcode_t)16)
typedef struct klr_BNOT_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
} klr_BNOT_t;

#define OPCODE_JMP ((kopcode_t)17)
typedef struct klr_JMP_t {
	KCODE_HEAD;
	kopl_t  *jumppc;
} klr_JMP_t;

#define OPCODE_JMPF ((kopcode_t)18)
typedef struct klr_JMPF_t {
	KCODE_HEAD;
	kopl_t  *jumppc;
	kreg_t a;
} klr_JMPF_t;

#define OPCODE_SAFEPOINT ((kopcode_t)19)
typedef struct klr_SAFEPOINT_t {
	KCODE_HEAD;
	kreg_t espshift;
} klr_SAFEPOINT_t;

#define OPCODE_ERROR ((kopcode_t)20)
typedef struct klr_ERROR_t {
	KCODE_HEAD;
	kreg_t start;
	kString* msg;
} klr_ERROR_t;

#define OPCODE_VCALL ((kopcode_t)21)
typedef struct klr_VCALL_t {
	KCODE_HEAD;
	uintptr_t uline;
	kreg_t thisidx;
	kreg_t espshift;
	kMethod* mtd;
	kObject* tyo;
} klr_VCALL_t;

	
#define KOPCODE_MAX ((kopcode_t)22)

#define VMT_VOID     0
#define VMT_ADDR     1
#define VMT_R        2
#define VMT_RN       2
#define VMT_RO       2
#define VMT_U        3
#define VMT_I        4
#define VMT_CID      5
#define VMT_CO       6
#define VMT_INT      7
#define VMT_FLOAT    8
#define VMT_HCACHE   9
#define VMT_F        10/*function*/
#define VMT_STRING   11
#define VMT_METHOD   12


/* ------------------------------------------------------------------------ */
/* [common] */

/* ------------------------------------------------------------------------ */
/* [data] */

#define _CONST 1
#define _JIT   (1<<1)
#define _DEF   (1<<2)
typedef struct {
	const char *name;
	kflag_t   flag;
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
	{"BOX", 0, 3, { VMT_RO, VMT_RN, VMT_CID, VMT_VOID}}, 
	{"UNBOX", 0, 3, { VMT_RN, VMT_RO, VMT_CID, VMT_VOID}}, 
	{"CALL", 0, 4, { VMT_U, VMT_RO, VMT_RO, VMT_CO, VMT_VOID}}, 
	{"SCALL", 0, 5, { VMT_U, VMT_RO, VMT_RO, VMT_METHOD, VMT_CO, VMT_VOID}}, 
	{"RET", 0, 0, { VMT_VOID}}, 
	{"NCALL", 0, 0, { VMT_VOID}}, 
	{"BNOT", 0, 2, { VMT_RN, VMT_RN, VMT_VOID}}, 
	{"JMP", 0, 1, { VMT_ADDR, VMT_VOID}}, 
	{"JMPF", 0, 2, { VMT_ADDR, VMT_RN, VMT_VOID}}, 
	{"SAFEPOINT", 0, 1, { VMT_RO, VMT_VOID}}, 
	{"ERROR", 0, 2, { VMT_RO, VMT_STRING, VMT_VOID}}, 
	{"VCALL", 0, 5, { VMT_U, VMT_RO, VMT_RO, VMT_METHOD, VMT_CO, VMT_VOID}}, 
};

static void opcode_check(void)
{
	assert(sizeof(klr_NOP_t) <= sizeof(kopl_t));
	assert(sizeof(klr_THCODE_t) <= sizeof(kopl_t));
	assert(sizeof(klr_ENTER_t) <= sizeof(kopl_t));
	assert(sizeof(klr_EXIT_t) <= sizeof(kopl_t));
	assert(sizeof(klr_NSET_t) <= sizeof(kopl_t));
	assert(sizeof(klr_NMOV_t) <= sizeof(kopl_t));
	assert(sizeof(klr_NMOVx_t) <= sizeof(kopl_t));
	assert(sizeof(klr_XNMOV_t) <= sizeof(kopl_t));
	assert(sizeof(klr_NEW_t) <= sizeof(kopl_t));
	assert(sizeof(klr_NULL_t) <= sizeof(kopl_t));
	assert(sizeof(klr_BOX_t) <= sizeof(kopl_t));
	assert(sizeof(klr_UNBOX_t) <= sizeof(kopl_t));
	assert(sizeof(klr_CALL_t) <= sizeof(kopl_t));
	assert(sizeof(klr_SCALL_t) <= sizeof(kopl_t));
	assert(sizeof(klr_RET_t) <= sizeof(kopl_t));
	assert(sizeof(klr_NCALL_t) <= sizeof(kopl_t));
	assert(sizeof(klr_BNOT_t) <= sizeof(kopl_t));
	assert(sizeof(klr_JMP_t) <= sizeof(kopl_t));
	assert(sizeof(klr_JMPF_t) <= sizeof(kopl_t));
	assert(sizeof(klr_SAFEPOINT_t) <= sizeof(kopl_t));
	assert(sizeof(klr_ERROR_t) <= sizeof(kopl_t));
	assert(sizeof(klr_VCALL_t) <= sizeof(kopl_t));
}

static const char *T_opcode(kopcode_t opcode)
{
	DBG_ASSERT(opcode < KOPCODE_MAX);
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

static kopl_t* VirtualMachine_run(KonohaContext *kctx, ksfp_t *sfp0, kopl_t *pc)
{
#ifdef K_USING_THCODE_
	static void *OPJUMP[] = {
		&&L_NOP, &&L_THCODE, &&L_ENTER, &&L_EXIT, 
		&&L_NSET, &&L_NMOV, &&L_NMOVx, &&L_XNMOV, 
		&&L_NEW, &&L_NULL, &&L_BOX, &&L_UNBOX, 
		&&L_CALL, &&L_SCALL, &&L_RET, &&L_NCALL, 
		&&L_BNOT, &&L_JMP, &&L_JMPF, &&L_SAFEPOINT, 
		&&L_ERROR, &&L_VCALL, 
	};
#endif
	krbp_t *rbp = (krbp_t*)sfp0;
	DISPATCH_START(pc);

	CASE(NOP) {
		klr_NOP_t *op = (klr_NOP_t*)pc;
		OPEXEC_NOP(); pc++;
		GOTO_NEXT();
	} 
	CASE(THCODE) {
		klr_THCODE_t *op = (klr_THCODE_t*)pc;
		OPEXEC_THCODE(op->th); pc++;
		GOTO_NEXT();
	} 
	CASE(ENTER) {
		klr_ENTER_t *op = (klr_ENTER_t*)pc;
		OPEXEC_ENTER(); pc++;
		GOTO_NEXT();
	} 
	CASE(EXIT) {
		klr_EXIT_t *op = (klr_EXIT_t*)pc;
		OPEXEC_EXIT(); pc++;
		GOTO_NEXT();
	} 
	CASE(NSET) {
		klr_NSET_t *op = (klr_NSET_t*)pc;
		OPEXEC_NSET(op->a, op->n, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(NMOV) {
		klr_NMOV_t *op = (klr_NMOV_t*)pc;
		OPEXEC_NMOV(op->a, op->b, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(NMOVx) {
		klr_NMOVx_t *op = (klr_NMOVx_t*)pc;
		OPEXEC_NMOVx(op->a, op->b, op->bx, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(XNMOV) {
		klr_XNMOV_t *op = (klr_XNMOV_t*)pc;
		OPEXEC_XNMOV(op->a, op->ax, op->b, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(NEW) {
		klr_NEW_t *op = (klr_NEW_t*)pc;
		OPEXEC_NEW(op->a, op->p, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(NULL) {
		klr_NULL_t *op = (klr_NULL_t*)pc;
		OPEXEC_NULL(op->a, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(BOX) {
		klr_BOX_t *op = (klr_BOX_t*)pc;
		OPEXEC_BOX(op->a, op->b, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(UNBOX) {
		klr_UNBOX_t *op = (klr_UNBOX_t*)pc;
		OPEXEC_UNBOX(op->a, op->b, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(CALL) {
		klr_CALL_t *op = (klr_CALL_t*)pc;
		OPEXEC_CALL(op->uline, op->thisidx, op->espshift, op->tyo); pc++;
		GOTO_NEXT();
	} 
	CASE(SCALL) {
		klr_SCALL_t *op = (klr_SCALL_t*)pc;
		OPEXEC_SCALL(op->uline, op->thisidx, op->espshift, op->mtd, op->tyo); pc++;
		GOTO_NEXT();
	} 
	CASE(RET) {
		klr_RET_t *op = (klr_RET_t*)pc;
		OPEXEC_RET(); pc++;
		GOTO_NEXT();
	} 
	CASE(NCALL) {
		klr_NCALL_t *op = (klr_NCALL_t*)pc;
		OPEXEC_NCALL(); pc++;
		GOTO_NEXT();
	} 
	CASE(BNOT) {
		klr_BNOT_t *op = (klr_BNOT_t*)pc;
		OPEXEC_BNOT(op->c, op->a); pc++;
		GOTO_NEXT();
	} 
	CASE(JMP) {
		klr_JMP_t *op = (klr_JMP_t*)pc;
		OPEXEC_JMP(pc = op->jumppc, JUMP); pc++;
		GOTO_NEXT();
	} 
	CASE(JMPF) {
		klr_JMPF_t *op = (klr_JMPF_t*)pc;
		OPEXEC_JMPF(pc = op->jumppc, JUMP, op->a); pc++;
		GOTO_NEXT();
	} 
	CASE(SAFEPOINT) {
		klr_SAFEPOINT_t *op = (klr_SAFEPOINT_t*)pc;
		OPEXEC_SAFEPOINT(op->espshift); pc++;
		GOTO_NEXT();
	} 
	CASE(ERROR) {
		klr_ERROR_t *op = (klr_ERROR_t*)pc;
		OPEXEC_ERROR(op->start, op->msg); pc++;
		GOTO_NEXT();
	} 
	CASE(VCALL) {
		klr_VCALL_t *op = (klr_VCALL_t*)pc;
		OPEXEC_VCALL(op->uline, op->thisidx, op->espshift, op->mtd, op->tyo); pc++;
		GOTO_NEXT();
	} 
	DISPATCH_END(pc);
	L_RETURN:;
	return pc;
}

#endif /* MINIVM_H */
