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

#ifndef TRACEVM_H
#define TRACEVM_H

/* virtual machine */

typedef intptr_t   kreg_t;
typedef void (*ThreadCodeFunc)(KonohaContext *kctx, struct KVirtualCode *, void**);
typedef void (*TraceFunc)(KonohaContext *kctx, KonohaStack *sfp, KTraceInfo *trace);

typedef struct {
	kMethod *mtd;
	ktypeattr_t typeId; kparamId_t signature;
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

typedef struct KVirtualCode {
	KCODE_HEAD;
	union {
		intptr_t data[5];
		void *p[5];
		kObject *o[5];
		KClass *ct[5];
		char *u[5];
	};
} KVirtualCode;

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
} KVirtualCodeType;

/* ------------------------------------------------------------------------ */

#define BasicNode_isVisited(o)      (KFlag_Is(uintptr_t,(o)->h.magicflag,kObjectFlag_Local1))
#define BasicNode_SetVisited(o,B)   KFlag_Set(uintptr_t,((kObjectVar *)o)->h.magicflag,kObjectFlag_Local1,B)

typedef struct kBasicNodeVar         kBasicNode;
typedef const struct kByteCodeVar     kByteCode;
typedef struct kByteCodeVar           kByteCodeVar;

struct kBasicNodeVar {
	kObjectHeader h;
	kushort_t id;     kushort_t incoming;
	KGrowingArray codeTable;
	kBasicNode        *nextNode;
	kBasicNode        *branchNode;
	KVirtualCode *code;
	KVirtualCode *opjmp;
};

struct kByteCodeVar {
	kObjectHeader h;
	KVirtualCode*   code;
	size_t    codesize;
	kString  *source;
	kfileline_t   fileid;
};

#define ctxcode          ((ctxcode_t *)kctx->modlocal[MOD_code])
#define kmodcode         ((KModuleByteCode *)kctx->modshare[MOD_code])
#define KClass_BasicNode    kmodcode->cBasicNode
#define KType_BasicNode    kmodcode->cBasicNode->typeId
#define KClass_ByteCode      kmodcode->cByteCode

#define IS_BasicNode(O)  (kObject_class(O) == KClass_BasicNode)
#define IS_ByteCode(O)    (kObject_class(O) == KClass_ByteCode)

#define CODE_ENTER        kmodcode->PRECOMPILED_ENTER
#define CODE_NCALL        kmodcode->PRECOMPILED_NCALL

typedef struct {
	KRuntimeModule     header;
	KClass     *cBasicNode;
	KClass     *cByteCode;
	kByteCode       *codeNull;
	struct KVirtualCode  *PRECOMPILED_ENTER;
	struct KVirtualCode  *PRECOMPILED_NCALL;
} KModuleByteCode;

typedef struct {
	KContextModule      h;
	kfileline_t      uline;
	kArray          *codeList;
	kBasicNode     *lbINIT; // ON GCSTACK
	kBasicNode     *lbEND;  // ON GCSTACK
	kArray          *constPools;
	kBasicNode     *currentWorkingNode;
} ctxcode_t;

/* ------------------------------------------------------------------------- */

#define PC_NEXT(pc)   pc+1

/* NOP */
#define VPARAM_NOP 0
typedef struct OPNOP {
	KCODE_HEAD;
} OPNOP;

#define OPEXEC_NOP() (void)op

/* THCODE */
#define VPARAM_THCODE 1, VMT_F
typedef struct OPTHCODE {
	KCODE_HEAD;
	ThreadCodeFunc threadCode;
} OPTHCODE;

#define OPEXEC_THCODE(F) do {\
	F(kctx, pc, OPJUMP); \
	pc = PC_NEXT(pc);\
	goto L_RETURN; \
} while(0)

/* ENTER */
#define VPARAM_ENTER 0
typedef struct OPENTER {
	KCODE_HEAD;
} OPENTER;

#define OPEXEC_ENTER() do {\
	(void)op;\
	rbp[K_PCIDX2].pc = PC_NEXT(pc);\
	pc = (rbp[K_MTDIDX2].calledMethod)->vcode_start;\
	GOTO_PC(pc); \
} while(0)

/* EXIT */
#define VPARAM_EXIT 0
typedef struct OPEXIT {
	KCODE_HEAD;
} OPEXIT;

#define OPEXEC_EXIT() do {\
	(void)op;\
	pc = NULL; \
	goto L_RETURN;\
} while(0)

/* NCALL */
#define VPARAM_NCALL     0
typedef struct OPNCALL {
	KCODE_HEAD;
} OPNCALL;

#define OPEXEC_NCALL() do {\
	(void)op;\
	(rbp[K_MTDIDX2].calledMethod)->invokeKMethodFunc(kctx, (KonohaStack *)(rbp));\
	OPEXEC_RET();\
} while(0)




/* NSET */
#define VPARAM_NSET        3, VMT_R, VMT_U, VMT_TY
typedef struct OPNSET {
	KCODE_HEAD;
	kreg_t a;
	kint_t n;
	KClass* ty;
} OPNSET;

#define OPEXEC_NSET(A, N, CT) rbp[(A)].unboxValue = (N)

/* NMOV */
#define VPARAM_NMOV       3, VMT_R, VMT_R, VMT_TY
typedef struct OPNMOV {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	KClass* ty;
} OPNMOV;

#define OPEXEC_NMOV(A, B, CT) rbp[(A)].unboxValue = rbp[(B)].unboxValue

/* NMOVx */
#define VPARAM_NMOVx      4, VMT_R, VMT_R, VMT_FX, VMT_TY
typedef struct OPNMOVx {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	uintptr_t bx;
	KClass* ty;
} OPNMOVx;

#define OPEXEC_NMOVx(A, B, BX, CT) rbp[(A)].unboxValue = (rbp[(B)].asObject)->fieldUnboxItems[(BX)]

/* XNMOV */
#define VPARAM_XNMOV     4, VMT_R, VMT_FX, VMT_R, VMT_TY
typedef struct OPXNMOV {
	KCODE_HEAD;
	kreg_t a;
	uintptr_t ax;
	kreg_t b;
	KClass* ty;
} OPXNMOV;

#define OPEXEC_XNMOV(A, AX, B, CT) (rbp[(A)].asObjectVar)->fieldUnboxItems[AX] = rbp[(B)].unboxValue

/* NEW */
#define VPARAM_NEW       3, VMT_R, VMT_U, VMT_C
typedef struct OPNEW {
	KCODE_HEAD;
	kreg_t a;
	uintptr_t p;
	KClass* ty;
} OPNEW;


#define OPEXEC_NEW(A, P, CT)   rbp[(A)].asObject = KLIB new_kObject(kctx, OnStack, CT, P)

/* OPNULL */
#define VPARAM_NULL       2, VMT_R, VMT_C
typedef struct OPNULL {
	KCODE_HEAD;
	kreg_t a;
	KClass* ty;
} OPNULL;

#define OPEXEC_NULL(A, CT)     rbp[(A)].asObject = KLIB Knull(kctx, CT)

/* OPLOOKUP */
#define VPARAM_LOOKUP    3, VMT_R, VMT_Object, VMT_Object
typedef struct OPLOOKUP {
	KCODE_HEAD;
	kreg_t thisidx;
	kNameSpace* ns;
	kMethod*    mtd;
	ktypeattr_t typeId; kparamId_t signature;  // invisible
} OPLOOKUP;

#define OPEXEC_LOOKUP(THIS, NS, MTD) do {\
	kNameSpace_LookupMethodWithInlineCache(kctx, (KonohaStack *)(rbp + THIS), NS, (kMethod**)&MTD);\
} while(0)

/* OPCALL*/
#define VPARAM_CALL     4, VMT_UL, VMT_R, VMT_R, VMT_Object
typedef struct OPCALL {
	KCODE_HEAD;
	uintptr_t uline;
	kreg_t thisidx;
	kreg_t espshift;
	kObject* tyo;
} OPCALL;

#define OPEXEC_CALL(UL, THIS, espshift, CTO) do {\
	kMethod *mtd_ = rbp[THIS+K_MTDIDX2].calledMethod;\
	KonohaStack *sfp_ = (KonohaStack *)(rbp + THIS); \
	KUnsafeFieldSet(sfp_[K_RTNIDX].asObject, CTO);\
	sfp_[K_RTNIDX].calledFileLine = UL;\
	sfp_[K_SHIFTIDX].previousStack = (KonohaStack *)(rbp);\
	sfp_[K_PCIDX].pc = PC_NEXT(pc);\
	sfp_[K_MTDIDX].calledMethod = mtd_;\
	KStackSetArgc(kctx, (KonohaStack *)(rbp + espshift));\
	(mtd_)->invokeKMethodFunc(kctx, sfp_); \
} while(0)

/* RET */
#define VPARAM_RET       0
typedef struct OPRET {
	KCODE_HEAD;
} OPRET;

#define OPEXEC_RET() do {\
	(void)op;\
	KVirtualCode *vpc = rbp[K_PCIDX2].pc;\
	rbp = (krbp_t *)rbp[K_SHIFTIDX2].previousStack;\
	pc = vpc; \
	GOTO_PC(pc);\
} while(0)


/* BNOT */
#define VPARAM_BNOT       2, VMT_R, VMT_R
typedef struct OPBNOT {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
} OPBNOT;

#define OPEXEC_BNOT(c, a)     rbp[c].boolValue = !(rbp[a].boolValue)

/* JMP */
#define VPARAM_JMP       1, VMT_ADDR
typedef struct OPJMP {
	KCODE_HEAD;
	KVirtualCode  *jumppc;
} OPJMP;

#define OPEXEC_JMP(PC, JUMP) do {\
	PC; \
	goto JUMP; \
} while(0)

/* JMPF */
#define VPARAM_JMPF      2, VMT_ADDR, VMT_R
typedef struct OPJMPF {
	KCODE_HEAD;
	KVirtualCode  *jumppc;
	kreg_t a;
} OPJMPF;

#define OPEXEC_JMPF(PC, JUMP, N) do {\
	if(!rbp[N].boolValue) {\
		OPEXEC_JMP(PC, JUMP); \
	} \
} while(0)

/* TRYJMP */
#define VPARAM_TRYJMP      1, VMT_ADDR
typedef struct OPTRYJMP {
	KCODE_HEAD;
	KVirtualCode  *jumppc;
} OPTRYJMP;

#define OPEXEC_TRYJMP(PC, JUMP) do {\
	pc = KonohaVirtualMachine_tryJump(kctx, (KonohaStack *)rbp, PC+1);\
	if(pc == NULL) {\
		OPEXEC_JMP(PC, JUMP); \
	} \
} while(0)

/* YIELD */
#define VPARAM_YIELD      0
typedef struct OPYIELD {
	KCODE_HEAD;
} OPYIELD;

#define OPEXEC_YIELD() do {\
	(void)op;\
	return pc;\
} while(0)

/* ERROR */
#define VPARAM_ERROR          3, VMT_UL, VMT_Object, VMT_R
typedef struct OPERROR {
	KCODE_HEAD;
	uintptr_t uline;
	kString*  msg;
	kreg_t    esp;
} OPERROR;

#define OPEXEC_ERROR(UL, msg, ESP) do {\
	((KonohaStack *)rbp)[K_RTNIDX].calledFileLine = UL;\
	KLIB KRuntime_raise(kctx, KException_("RuntimeScript"), SoftwareFault, msg, (KonohaStack *)rbp);\
} while(0)

/* SAFEPOINT */
#define VPARAM_SAFEPOINT       2, VMT_UL, VMT_R
typedef struct OPSAFEPOINT {
	KCODE_HEAD;
	uintptr_t uline;
	kreg_t    esp;
} OPSAFEPOINT;

#define OPEXEC_SAFEPOINT(UL, espidx) do {\
	KStackSetArgc(kctx, (KonohaStack *)(rbp+espidx));\
	KonohaVirtualMachine_onSafePoint(kctx, (KonohaStack *)rbp, UL); \
} while(0)

/* CHKSTACK */
#define VPARAM_CHKSTACK        1, VMT_UL
typedef struct OPCHKSTACK {
	KCODE_HEAD;
	uintptr_t uline;
} OPCHKSTACK;

#define OPEXEC_CHKSTACK(UL) do {\
	if(unlikely(kctx->esp > kctx->stack->stack_uplimit)) {\
		KLIB KRuntime_raise(kctx, KException_("StackOverflow"), SoftwareFault, NULL, (KonohaStack *)(rbp));\
	}\
	kfileline_t uline = (UL == 0) ? rbp[K_ULINEIDX2].calledFileLine : UL;\
	KonohaVirtualMachine_onSafePoint(kctx, (KonohaStack *)rbp, uline);\
} while(0)

/* TRACE */
#define VPARAM_TRACE           3, VMT_UL, VMT_R, VMT_F
typedef struct OPTRACE {
	KCODE_HEAD;
	uintptr_t uline;
	kreg_t    thisidx;
	TraceFunc trace;
} OPTRACE;

#define OPEXEC_TRACE(UL, THIS, F) do {\
	KMakeTraceUL(trace, (KonohaStack *)(rbp), UL);\
	F(kctx, (KonohaStack *)(rbp + THIS), trace);\
} while(0)

#endif /* TRACEVM_H */
