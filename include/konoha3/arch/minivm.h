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
typedef void (*ThreadCodeFunc)(KonohaContext *kctx, struct KVirtualCode *, void**, size_t size);
typedef void (*TraceFunc)(KonohaContext *kctx, KonohaStack *sfp, KTraceInfo *trace);

typedef struct {
	kMethod *mtd;
	ktypeattr_t typeId; kparamId_t signature;
} kMethodInlineCache;

#ifndef KCODE_HEAD
#if defined(USE_DIRECT_THREADED_CODE)
#define OP_(T)  NULL, 0, OPCODE_##T, 0
#define KCODE_HEAD\
	void *codeaddr;\
	kushort_t count;\
	kushort_t opcode;\
	kfileline_t line

#else
#define OP_(T)  OPCODE_##T, 0
#define KCODE_HEAD\
	kushort_t opcode;\
	kushort_t line

#endif/*USE_DIRECT_THREADED_CODE*/
#endif

#define OPARGSIZE 4

typedef struct KVirtualCode {
	KCODE_HEAD;
	union {
		intptr_t data[OPARGSIZE];
		void *p[OPARGSIZE];
		kObject *o[OPARGSIZE];
		KClass *ct[OPARGSIZE];
		char *u[OPARGSIZE];
	};
} KVirtualCode;

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

#define PC_NEXT(pc)   pc+1

/* NOP */
#define VPARAM_NOP 0
typedef struct OPNOP {
	KCODE_HEAD;
} OPNOP;

#ifndef OPEXEC_NOP
#define OPEXEC_NOP()
#endif

/* THCODE */
#define VPARAM_THCODE 2, VMT_U, VMT_F
typedef struct OPTHCODE {
	KCODE_HEAD;
	size_t codesize;
	ThreadCodeFunc threadCode;
} OPTHCODE;

#ifndef OPEXEC_THCODE
#define OPEXEC_THCODE() do {\
	OPTHCODE *op = (OPTHCODE *)pc;\
	op->threadCode(kctx, pc, OPJUMP, op->codesize); \
	return PC_NEXT(pc);\
} while(0)
#endif

/* ENTER */
#define VPARAM_ENTER 0
typedef struct OPENTER {
	KCODE_HEAD;
} OPENTER;

#ifndef OPEXEC_ENTER
#define OPEXEC_ENTER() do {\
	rbp[K_PCIDX2].pc = PC_NEXT(pc);\
	pc = (rbp[K_MTDIDX2].calledMethod)->vcode_start;\
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
	(rbp[K_MTDIDX2].calledMethod)->invokeKMethodFunc(kctx, (KonohaStack *)(rbp));\
	OPEXEC_RET();\
} while(0)
#endif

/* NSET */
#define VPARAM_NSET        3, VMT_R, VMT_U, VMT_TY
typedef struct OPNSET {
	KCODE_HEAD;
	kreg_t a;
	kint_t n;
	KClass* ty;
} OPNSET;

#ifndef OPEXEC_NSET
#define OPEXEC_NSET() do {\
	OPNSET *op = (OPNSET *)pc;\
	rbp[op->a].unboxValue = (op->n);\
} while(0)
#endif

/* NMOV */
#define VPARAM_NMOV       3, VMT_R, VMT_R, VMT_TY
typedef struct OPNMOV {
	KCODE_HEAD;
	kreg_t dst;
	kreg_t src;
	KClass* ty;
} OPNMOV;

#ifndef OPEXEC_NMOV
#define OPEXEC_NMOV() do {\
	OPNMOV *op = (OPNMOV *)pc;\
	rbp[op->dst].unboxValue = rbp[op->src].unboxValue;\
} while(0)
#endif

/* NMOVx */
#define VPARAM_NMOVx      4, VMT_R, VMT_R, VMT_FX, VMT_TY
typedef struct OPNMOVx {
	KCODE_HEAD;
	kreg_t dst;
	kreg_t src;
	uintptr_t bx;
	KClass* ty;
} OPNMOVx;

#ifndef OPEXEC_NMOVx
#define OPEXEC_NMOVx() do {\
	OPNMOVx *op = (OPNMOVx *)pc;\
	rbp[op->dst].unboxValue = (rbp[op->src].asObject)->fieldUnboxItems[op->bx];\
} while(0)
#endif

/* XNMOV */
#define VPARAM_XNMOV     4, VMT_R, VMT_FX, VMT_R, VMT_TY
typedef struct OPXNMOV {
	KCODE_HEAD;
	kreg_t a;
	uintptr_t ax;
	kreg_t b;
	KClass* ty;
} OPXNMOV;

#ifndef OPEXEC_XNMOV
#define OPEXEC_XNMOV() do {\
	OPXNMOV *op = (OPXNMOV *)pc;\
	(rbp[op->a].asObjectVar)->fieldUnboxItems[op->ax] = rbp[op->b].unboxValue;\
} while(0)
#endif

/* NEW */
#define VPARAM_NEW       3, VMT_R, VMT_U, VMT_C
typedef struct OPNEW {
	KCODE_HEAD;
	kreg_t a;
	uintptr_t p;
	KClass* ty;
} OPNEW;

#ifndef OPEXEC_NEW
#define OPEXEC_NEW() do {\
	OPNEW *op = (OPNEW *)pc;\
	rbp[op->a].asObject = KLIB new_kObject(kctx, OnStack, op->ty, op->p);\
} while(0)
#endif

/* OPNLL */
#define VPARAM_NUL       2, VMT_R, VMT_C
typedef struct OPNLL {
	KCODE_HEAD;
	kreg_t a;
	KClass* ty;
} OPNUL;

#ifndef OPEXEC_NUL
#define OPEXEC_NUL() do {\
	OPNUL *op = (OPNUL *)pc;\
	rbp[op->a].asObject = KLIB Knull(kctx, op->ty);\
} while(0)
#endif

/* BOX */
#define VPARAM_BOX       3, VMT_R, VMT_R, VMT_C
typedef struct OPBOX {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	KClass* ty;
} OPBOX;

#ifndef OPEXEC_BOX
#define OPEXEC_BOX() do {\
	OPBOX *op = (OPBOX *)pc;\
	rbp[op->a].asObject = KLIB new_kObject(kctx, OnStack, op->ty, rbp[op->b].unboxValue);\
} while(0)
#endif

/* BBOX */
#define VPARAM_BBOX       3, VMT_R, VMT_R, VMT_C
typedef struct OPBBOX {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	KClass* ty;
} OPBBOX;

#ifndef OPEXEC_BBOX
#define OPEXEC_BBOX() do {\
	OPBBOX *op = (OPBBOX *)pc;\
	rbp[op->a].asBoolean = rbp[op->b].boolValue ? K_TRUE : K_FALSE;\
} while(0)
#endif

/* OPLOOKUP */
#define VPARAM_LOOKUP    3, VMT_R, VMT_Object, VMT_Object
typedef struct OPLOOKUP {
	KCODE_HEAD;
	kreg_t thisidx;
	kNameSpace* ns;
	kMethod*    mtd;
	ktypeattr_t typeId; kparamId_t signature;  // invisible
} OPLOOKUP;

#ifndef OPEXEC_LOOKUP
#define OPEXEC_LOOKUP() do {\
	OPLOOKUP *op = (OPLOOKUP *)pc;\
	kNameSpace_LookupMethodWithInlineCache(kctx, (KonohaStack *)(rbp + op->thisidx), op->ns, (kMethod**)&(op->mtd));\
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
#define OPEXEC_CALL() do {\
	OPCALL *op = (OPCALL *)pc;\
	kMethod *mtd_ = rbp[op->thisidx + K_MTDIDX2].calledMethod;\
	KonohaStack *sfp_ = (KonohaStack *)(rbp + op->thisidx); \
	KUnsafeFieldSet(sfp_[K_RTNIDX].asObject, op->tyo);\
	sfp_[K_RTNIDX].calledFileLine = op->uline;\
	sfp_[K_SHIFTIDX].previousStack = (KonohaStack *)(rbp);\
	sfp_[K_PCIDX].pc = PC_NEXT(pc);\
	sfp_[K_MTDIDX].calledMethod = mtd_;\
	((KonohaContextVar *)kctx)->esp = (KonohaStack *)(rbp + op->espshift);\
	(mtd_)->invokeKMethodFunc(kctx, sfp_); \
} while(0)
#endif

/* RET */
#define VPARAM_RET       0
typedef struct OPRET {
	KCODE_HEAD;
} OPRET;

#ifndef OPEXEC_RET
#define OPEXEC_RET() do {\
	KVirtualCode *vpc = rbp[K_PCIDX2].pc;\
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
#define OPEXEC_BNOT() do {\
	OPBNOT *op = (OPBNOT *)pc;\
	rbp[op->c].boolValue = !(rbp[op->a].boolValue);\
} while(0)
#endif

/* JMP */
#define VPARAM_JMP       1, VMT_ADDR
typedef struct OPJMP {
	KCODE_HEAD;
	KVirtualCode  *jumppc;
} OPJMP;

#ifndef OPEXEC_JMP
#define OPEXEC_JMP() do {\
	OPJMP *op = (OPJMP *)pc;\
	pc = op->jumppc;\
	goto JUMP; \
} while(0)
#endif

/* JMPF */
#define VPARAM_JMPF      2, VMT_ADDR, VMT_R
typedef struct OPJMPF {
	KCODE_HEAD;
	KVirtualCode  *jumppc;
	kreg_t a;
} OPJMPF;

#ifndef OPEXEC_JMPF
#define OPEXEC_JMPF() do {\
	OPJMPF *op = (OPJMPF *)pc;\
	if(!rbp[op->a].boolValue) {\
		pc = op->jumppc;\
		goto JUMP;\
	} \
} while(0)
#endif

/* TRYJMP */
#define VPARAM_TRYJMP      1, VMT_ADDR
typedef struct OPTRYJMP {
	KCODE_HEAD;
	KVirtualCode  *jumppc;
} OPTRYJMP;

#ifndef OPEXEC_TRYJMP
#define OPEXEC_TRYJMP() do {\
	OPTRYJMP *op = (OPTRYJMP *)pc;\
	pc = KonohaVirtualMachine_tryJump(kctx, (KonohaStack *)rbp, pc+1);\
	if(pc == NULL) {\
		pc = op->jumppc;\
		goto JUMP;\
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
#define OPEXEC_ERROR() do {\
	OPERROR *op = (OPERROR *)pc;\
	((KonohaStack *)rbp)[K_RTNIDX].calledFileLine = op->uline;\
	KLIB KRuntime_raise(kctx, KException_("RuntimeScript"), SoftwareFault, op->msg, (KonohaStack *)rbp);\
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
#define OPEXEC_SAFEPOINT() do {\
	OPSAFEPOINT *op = (OPSAFEPOINT *)pc;\
	KStackSetArgc((KonohaStack *)(rbp), op->esp);\
	KLIB CheckSafePoint(kctx, (KonohaStack *)rbp, op->uline);\
} while(0)
#endif

/* CHKSTACK */
#define VPARAM_CHKSTACK        1, VMT_UL
typedef struct OPCHKSTACK {
	KCODE_HEAD;
	uintptr_t uline;
} OPCHKSTACK;

#ifndef OPEXEC_CHKSTACK
#define OPEXEC_CHKSTACK() KStackCheckOverflow((KonohaStack *)(rbp))
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
#define OPEXEC_TRACE()
#endif

#endif /* MINIVM_H */
