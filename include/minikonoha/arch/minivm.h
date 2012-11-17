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

/* minivm arch */

typedef intptr_t   kreg_t;
typedef void (*ThreadCodeFunc)(KonohaContext *kctx, struct VirtualCode *, void**);
typedef void (*TraceFunc)(KonohaContext *kctx, KonohaStack *sfp, KTraceInfo *trace);

typedef struct {
	kMethod *mtd;
	ktype_t typeId; kparamId_t signature;
} kMethodInlineCache;

#if defined(USE_DIRECT_THREADED_CODE)
#define OP_(T)  NULL, 0, OPCODE_##T, 0
#define KCODE_HEAD\
	void *codeaddr;\
	size_t count;\
	kushort_t opcode;\
	kfileline_t line

#else
#define OP_(T)  0, OPCODE_##T, 0
#define KCODE_HEAD \
	size_t count; \
	kopcode_t opcode; \
	kfileline_t line

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

typedef enum {
	OPCODE_NOP,
	OPCODE_THCODE,
	OPCODE_ENTER,
	OPCODE_EXIT,
	// OPCODE_CALLBACK,
	OPCODE_NSET,
	OPCODE_NMOV,
	OPCODE_NMOVx,
	OPCODE_XNMOV,
	OPCODE_NEW,
	OPCODE_NULL,
	OPCODE_LOOKUP,
	OPCODE_CALL,
	OPCODE_RET,
	OPCODE_NCALL,
	OPCODE_BNOT,
	OPCODE_JMP,
	OPCODE_JMPF,
	OPCODE_TRYJMP,
	OPCODE_YIELD,
	OPCODE_ERROR,
	OPCODE_SAFEPOINT,
	OPCODE_CHKSTACK,
	OPCODE_TRACE,
	OPCODE_MAX,
} MiniVM;

typedef enum {
	VMT_VOID,
	VMT_ADDR,
	VMT_UL,
	VMT_R,
	VMT_FX,
	VMT_U,
	VMT_C,
	VMT_TY,
	VMT_F,        /*function*/
	VMT_Object,
	VMT_HCACHE,
} VirtualCodeType;

#define PC_NEXT(pc)   pc+1

/* NOP */
#define VPARAM_NOP 0
typedef struct OPNOP {
	KCODE_HEAD;
} OPNOP;

#ifndef OPEXEC_NOP
#define OPEXEC_NOP() (void)op
#endif

/* THCODE */
#define VPARAM_THCODE 1, VMT_F
typedef struct OPTHCODE {
	KCODE_HEAD;
	ThreadCodeFunc threadCode;
} OPTHCODE;

#ifndef OPEXEC_THCODE
#define OPEXEC_THCODE(F) do {\
	F(kctx, pc, OPJUMP); \
	pc = PC_NEXT(pc);\
	goto L_RETURN; \
} while(0)
#endif

/* ENTER */
#define VPARAM_ENTER 0
typedef struct OPENTER {
	KCODE_HEAD;
} OPENTER;

#ifndef OPEXEC_ENTER
#define OPEXEC_ENTER() do {\
	(void)op;\
	rbp[K_PCIDX2].pc = PC_NEXT(pc);\
	pc = (rbp[K_MTDIDX2].calledMethod)->pc_start;\
	GOTO_PC(pc); \
} while(0)
#endif

/* EXIT */
#define VPARAM_EXIT 0
typedef struct OPEXIT {
	KCODE_HEAD;
} OPEXIT;

#ifndef OPEXEC_EXIT
#define OPEXEC_EXIT() do {\
	(void)op;\
	pc = NULL; \
	goto L_RETURN;\
} while(0)
#endif

/* NCALL */
#define VPARAM_NCALL     0
typedef struct OPNCALL {
	KCODE_HEAD;
} OPNCALL;

#ifndef OPEXEC_NCALL
#define OPEXEC_NCALL() do {\
	(void)op;\
	(rbp[K_MTDIDX2].calledMethod)->invokeMethodFunc(kctx, (KonohaStack *)(rbp));\
	OPEXEC_RET();\
} while(0)
#endif

/* NSET */
#define VPARAM_NSET        3, VMT_R, VMT_U, VMT_TY
typedef struct OPNSET {
	KCODE_HEAD;
	kreg_t a;
	kint_t n;
	KonohaClass* ty;
} OPNSET;

#ifndef OPEXEC_NSET
#define OPEXEC_NSET(A, N, CT) rbp[(A)].unboxValue = (N)
#endif

/* NMOV */
#define VPARAM_NMOV       3, VMT_R, VMT_R, VMT_TY
typedef struct OPNMOV {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	KonohaClass* ty;
} OPNMOV;

#ifndef OPEXEC_NMOV
#define OPEXEC_NMOV(A, B, CT) rbp[(A)].unboxValue = rbp[(B)].unboxValue
#endif

/* NMOVx */
#define VPARAM_NMOVx      4, VMT_R, VMT_R, VMT_FX, VMT_TY
typedef struct OPNMOVx {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	uintptr_t bx;
	KonohaClass* ty;
} OPNMOVx;

#ifndef OPEXEC_NMOVx
#define OPEXEC_NMOVx(A, B, BX, CT) rbp[(A)].unboxValue = (rbp[(B)].asObject)->fieldUnboxItems[(BX)]
#endif

/* XNMOV */
#define VPARAM_XNMOV     4, VMT_R, VMT_FX, VMT_R, VMT_TY
typedef struct OPXNMOV {
	KCODE_HEAD;
	kreg_t a;
	uintptr_t ax;
	kreg_t b;
	KonohaClass* ty;
} OPXNMOV;

#ifndef OPEXEC_XNMOV
#define OPEXEC_XNMOV(A, AX, B, CT) (rbp[(A)].asObjectVar)->fieldUnboxItems[AX] = rbp[(B)].unboxValue
#endif

/* NEW */
#define VPARAM_NEW       3, VMT_R, VMT_U, VMT_C
typedef struct OPNEW {
	KCODE_HEAD;
	kreg_t a;
	uintptr_t p;
	KonohaClass* ty;
} OPNEW;

#ifndef OPEXEC_NEW
#define OPEXEC_NEW(A, P, CT)   rbp[(A)].asObject = KLIB new_kObject(kctx, OnStack, CT, P)
#endif

/* OPNULL */
#define VPARAM_NULL       2, VMT_R, VMT_C
typedef struct OPNULL {
	KCODE_HEAD;
	kreg_t a;
	KonohaClass* ty;
} OPNULL;

#ifndef OPEXEC_NULL
#define OPEXEC_NULL(A, CT)     rbp[(A)].asObject = KLIB Knull(kctx, CT)
#endif

/* OPLOOKUP */
#define VPARAM_LOOKUP    3, VMT_R, VMT_Object, VMT_Object
typedef struct OPLOOKUP {
	KCODE_HEAD;
	kreg_t thisidx;
	kNameSpace* ns;
	kMethod*    mtd;
	ktype_t typeId; kparamId_t signature;  // invisible
} OPLOOKUP;

#ifndef OPEXEC_LOOKUP
#define OPEXEC_LOOKUP(THIS, NS, MTD) do {\
	kNameSpace_LookupMethodWithInlineCache(kctx, (KonohaStack *)(rbp + THIS), NS, (kMethod**)&MTD);\
} while(0)
#endif

/* OPCALL*/
#define VPARAM_CALL     4, VMT_UL, VMT_R, VMT_R, VMT_Object
typedef struct OPCALL {
	KCODE_HEAD;
	uintptr_t uline;
	kreg_t thisidx;
	kreg_t espshift;
	kObject* tyo;
} OPCALL;

#ifndef OPEXEC_CALL
#define OPEXEC_CALL(UL, THIS, espshift, CTO) do {\
	kMethod *mtd_ = rbp[THIS+K_MTDIDX2].calledMethod;\
	KonohaStack *sfp_ = (KonohaStack *)(rbp + THIS); \
	KUnsafeFieldSet(sfp_[K_RTNIDX].asObject, CTO);\
	sfp_[K_RTNIDX].calledFileLine = UL;\
	sfp_[K_SHIFTIDX].previousStack = (KonohaStack *)(rbp);\
	sfp_[K_PCIDX].pc = PC_NEXT(pc);\
	sfp_[K_MTDIDX].calledMethod = mtd_;\
	KonohaRuntime_setesp(kctx, (KonohaStack *)(rbp + espshift));\
	(mtd_)->invokeMethodFunc(kctx, sfp_); \
} while(0)
#endif

/* RET */
#define VPARAM_RET       0
typedef struct OPRET {
	KCODE_HEAD;
} OPRET;

#ifndef OPEXEC_RET
#define OPEXEC_RET() do {\
	(void)op;\
	VirtualCode *vpc = rbp[K_PCIDX2].pc;\
	rbp = (krbp_t *)rbp[K_SHIFTIDX2].previousStack;\
	pc = vpc; \
	GOTO_PC(pc);\
} while(0)
#endif


/* BNOT */
#define VPARAM_BNOT       2, VMT_R, VMT_R
typedef struct OPBNOT {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
} OPBNOT;

#ifndef OPEXEC_BNOT
#define OPEXEC_BNOT(c, a)     rbp[c].boolValue = !(rbp[a].boolValue)
#endif

/* JMP */
#define VPARAM_JMP       1, VMT_ADDR
typedef struct OPJMP {
	KCODE_HEAD;
	VirtualCode  *jumppc;
} OPJMP;

#ifndef OPEXEC_JMP
#define OPEXEC_JMP(PC, JUMP) do {\
	PC; \
	goto JUMP; \
} while(0)
#endif

/* JMPF */
#define VPARAM_JMPF      2, VMT_ADDR, VMT_R
typedef struct OPJMPF {
	KCODE_HEAD;
	VirtualCode  *jumppc;
	kreg_t a;
} OPJMPF;

#ifndef OPEXEC_JMPF
#define OPEXEC_JMPF(PC, JUMP, N) do {\
	if(!rbp[N].boolValue) {\
		OPEXEC_JMP(PC, JUMP); \
	} \
} while(0)
#endif

/* TRYJMP */
#define VPARAM_TRYJMP      1, VMT_ADDR
typedef struct OPTRYJMP {
	KCODE_HEAD;
	VirtualCode  *jumppc;
} OPTRYJMP;

#ifndef OPEXEC_TRYJMP
#define OPEXEC_TRYJMP(PC, JUMP) do {\
	pc = KonohaVirtualMachine_tryJump(kctx, (KonohaStack *)rbp, PC+1);\
	if(pc == NULL) {\
		OPEXEC_JMP(PC, JUMP); \
	} \
} while(0)
#endif

/* YIELD */
#define VPARAM_YIELD      0
typedef struct OPYIELD {
	KCODE_HEAD;
} OPYIELD;

#ifndef OPEXEC_YIELD
#define OPEXEC_YIELD() do {\
	(void)op;\
	return pc;\
} while(0)
#endif

/* ERROR */
#define VPARAM_ERROR          3, VMT_UL, VMT_Object, VMT_R
typedef struct OPERROR {
	KCODE_HEAD;
	uintptr_t uline;
	kString*  msg;
	kreg_t    esp;
} OPERROR;

#ifndef OPEXEC_ERROR
#define OPEXEC_ERROR(UL, msg, ESP) do {\
	((KonohaStack *)rbp)[K_RTNIDX].calledFileLine = UL;\
	KLIB KonohaRuntime_raise(kctx, EXPT_("RuntimeScript"), SoftwareFault, msg, (KonohaStack *)rbp);\
} while(0)
#endif

/* SAFEPOINT */
#define VPARAM_SAFEPOINT       2, VMT_UL, VMT_R
typedef struct OPSAFEPOINT {
	KCODE_HEAD;
	uintptr_t uline;
	kreg_t    esp;
} OPSAFEPOINT;

#ifndef OPEXEC_SAFEPOINT
#define OPEXEC_SAFEPOINT(UL, espidx) do {\
	KonohaRuntime_setesp(kctx, (KonohaStack *)(rbp+espidx));\
	KLIB CheckSafePoint(kctx, (KonohaStack *)rbp, UL);\
} while(0)
#endif

/* CHKSTACK */
#define VPARAM_CHKSTACK        1, VMT_UL
typedef struct OPCHKSTACK {
	KCODE_HEAD;
	uintptr_t uline;
} OPCHKSTACK;

#ifndef OPEXEC_CHKSTACK
#define OPEXEC_CHKSTACK(UL) do {\
	if(unlikely(kctx->esp > kctx->stack->stack_uplimit)) {\
		KLIB KonohaRuntime_raise(kctx, EXPT_("StackOverflow"), SoftwareFault, NULL, (KonohaStack *)(rbp));\
	}\
	(void)UL;\
} while(0)
#endif

/* TRACE */
#define VPARAM_TRACE           3, VMT_UL, VMT_R, VMT_F
typedef struct OPTRACE {
	KCODE_HEAD;
	uintptr_t uline;
	kreg_t    thisidx;
	TraceFunc trace;
} OPTRACE;

#ifndef OPEXEC_TRACE
#define OPEXEC_TRACE(UL, THIS, F) do {\
	KMakeTraceUL(trace, (KonohaStack *)(rbp), UL);\
	F(kctx, (KonohaStack *)(rbp + THIS), trace);\
} while(0)
#endif

#endif /* MINIVM_H */
