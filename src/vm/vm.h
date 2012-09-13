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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>

#ifndef KONOHA_VM_H_
#define KONOHA_VM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define K_USING_THCODE_

typedef struct kBasicBlockVar         kBasicBlock;
typedef const struct kByteCodeVar     kByteCode;
typedef struct kByteCodeVar           kByteCodeVar;

#define ctxcode          ((ctxcode_t*)kctx->modlocal[MOD_code])
#define kmodcode         ((KModuleByteCode*)kctx->modshare[MOD_code])
#define CT_BasicBlock    kmodcode->cBasicBlock
#define TY_BasicBlock    kmodcode->cBasicBlock->typeId
#define CT_ByteCode      kmodcode->cByteCode

#define IS_BasicBlock(O)  ((O)->h.ct == CT_BasicBlock)
#define IS_ByteCode(O)    ((O)->h.ct == CT_ByteCode)

#define CODE_ENTER        kmodcode->PRECOMPILED_ENTER
#define CODE_NCALL        kmodcode->PRECOMPILED_NCALL

typedef struct {
	KonohaModule     h;
	KonohaClass    *cBasicBlock;
	KonohaClass    *cByteCode;
	kByteCode      *codeNull;
	struct VirtualMachineInstruction  *PRECOMPILED_ENTER;
	struct VirtualMachineInstruction  *PRECOMPILED_NCALL;
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

typedef void (*ThreadCodeFunc)(KonohaContext *kctx, struct VirtualMachineInstruction *, void**);
typedef void (*TraceFunc)(KonohaContext *kctx, KonohaStack *sfp, kfileline_t pline);

typedef struct {
	kMethod *mtd;
	ktype_t typeId; kparamid_t signature;
} kMethodInlineCache;

#if defined(K_USING_THCODE_)
#define KCODE_HEAD \
	void *codeaddr; \
	size_t count; \
	kushort_t opcode; \
	kushort_t line

#else
#define KCODE_HEAD \
	size_t count; \
	kopcode_t opcode; \
	uintptr_t line \

#endif/*K_USING_THCODE_*/

typedef struct VirtualMachineInstruction {
	KCODE_HEAD;
	union {
		intptr_t data[5];
		void *p[5];
		kObject *o[5];
		KonohaClass *ct[5];
		char *u[5];
	};
} VirtualMachineInstruction;

/* ------------------------------------------------------------------------ */

#define BasicBlock_isVisited(o)      (TFLAG_is(uintptr_t,(o)->h.magicflag,kObject_Local1))
#define BasicBlock_setVisited(o,B)   TFLAG_set(uintptr_t,((kObjectVar*)o)->h.magicflag,kObject_Local1,B)

struct kBasicBlockVar {
	KonohaObjectHeader h;
	kushort_t id;     kushort_t incoming;
	KUtilsGrowingArray codeTable;
	kBasicBlock        *nextBlock;
	kBasicBlock        *branchBlock;
	VirtualMachineInstruction *code;
	VirtualMachineInstruction *opjmp;
};

struct kByteCodeVar {
	KonohaObjectHeader h;
	VirtualMachineInstruction*   code;
	size_t    codesize;
	kString  *source;
	kfileline_t   fileid;
};

//-------------------------------------------------------------------------

static void kNameSpace_lookupMethodWithInlineCache(KonohaContext *kctx, KonohaStack *sfp, kNameSpace *ns, kMethod **cache)
{
	ktype_t typeId = O_typeId(sfp[0].asObject);
	kMethod *mtd = cache[0];
	if(mtd->typeId != typeId) {
		mtd = KLIB kNameSpace_getMethodBySignatureNULL(kctx, ns, typeId, mtd->mn, mtd->paramdom, 0, NULL);
		cache[0] = mtd;
	}
	sfp[K_MTDIDX].mtdNC = mtd;
}

static VirtualMachineInstruction* KonohaVirtualMachine_run(KonohaContext *, KonohaStack *, VirtualMachineInstruction *);

static VirtualMachineInstruction *KonohaVirtualMachine_tryJump(KonohaContext *kctx, KonohaStack *sfp, VirtualMachineInstruction *pc)
{
	int jmpresult;
	INIT_GCSTACK();
	KonohaStackRuntimeVar *base = kctx->stack;
	jmpbuf_i lbuf = {};
	if(base->evaljmpbuf == NULL) {
		base->evaljmpbuf = (jmpbuf_i*)KCALLOC(sizeof(jmpbuf_i), 1);
	}
	memcpy(&lbuf, base->evaljmpbuf, sizeof(jmpbuf_i));
	if((jmpresult = PLATAPI setjmp_i(*base->evaljmpbuf)) == 0) {
		pc = KonohaVirtualMachine_run(kctx, sfp, pc);
	}
	else {
		DBG_P("Catch eval exception jmpresult=%d", jmpresult);
		//KSETv(sfp[exceptionIdx].e, ..);
		pc = NULL;
	}
	memcpy(base->evaljmpbuf, &lbuf, sizeof(jmpbuf_i));
	RESET_GCSTACK();
	return pc;
}

static void KonohaVirtualMachine_onSafePoint(KonohaContext *kctx, KonohaStack *sfp, kfileline_t pline)
{
	KNH_SAFEPOINT(kctx, sfp);
}

//-------------------------------------------------------------------------

#define rshift(rbp, x_) (rbp+(x_))
#define SFP(rbp)  ((KonohaStack*)(rbp))
#define SFPIDX(n) ((n)/2)
#define RBP(sfp)  ((krbp_t*)(sfp))

#define OPEXEC_NOP() (void)op

#define OPEXEC_THCODE(F) do {\
	F(kctx, pc, OPJUMP); \
	pc = PC_NEXT(pc);\
	goto L_RETURN; \
} while (0)

#define OPEXEC_ENTER() do {\
	(void)op;\
	VirtualMachineInstruction *vpc = PC_NEXT(pc);\
	pc = (rbp[K_MTDIDX2].mtdNC)->pc_start;\
	rbp[K_SHIFTIDX2].shift = 0;\
	rbp[K_PCIDX2].pc = vpc;\
	GOTO_PC(pc); \
} while (0)

#define OPEXEC_NCALL() do {\
	(void)op;\
	(rbp[K_MTDIDX2].mtdNC)->invokeMethodFunc(kctx, SFP(rbp));\
	OPEXEC_RET();\
} while (0)

#define OPEXEC_YIELD() do {\
	(void)op;\
	return pc;\
} while (0)

#define OPEXEC_EXIT() do {\
	(void)op;\
	pc = NULL; \
	goto L_RETURN;\
} while (0)

#define OPEXEC_NSET(A, N, CT) rbp[(A)].unboxValue = N
#define OPEXEC_NMOV(A, B, CT) rbp[(A)].unboxValue = rbp[(B)].unboxValue
#define OPEXEC_NMOVx(A, B, BX, CT) rbp[(A)].o = (rbp[(B)].asObjectVar)->fieldObjectItems[(BX)]
#define OPEXEC_XNMOV(A, AX, B, CT) (rbp[(A)].asObjectVar)->fieldObjectItems[AX] = rbp[(B)].o

#define OPEXEC_NEW(A, P, CT)   KSETv_AND_WRITE_BARRIER(NULL, rbp[(A)].o, KLIB new_kObject(kctx, CT, P), GC_NO_WRITE_BARRIER)
#define OPEXEC_NULL(A, CT)     KSETv_AND_WRITE_BARRIER(NULL, rbp[(A)].o, KLIB Knull(kctx, CT), GC_NO_WRITE_BARRIER)
#define OPEXEC_BOX(A, B, CT)   KSETv_AND_WRITE_BARRIER(NULL, rbp[(A)].o, KLIB new_kObject(kctx, CT, rbp[(B)].intValue), GC_NO_WRITE_BARRIER)
#define OPEXEC_UNBOX(A, B, CT) rbp[(A)].unboxValue = N_toint(rbp[B].o)

#define PC_NEXT(pc)   pc+1

#define OPEXEC_LOOKUP(THIS, NS, MTD) do {\
	kNameSpace_lookupMethodWithInlineCache(kctx, SFP(rshift(rbp, THIS)), NS, (kMethod**)&MTD);\
} while (0)

#define OPEXEC_CALL(UL, THIS, espshift, CTO) do {\
	kMethod *mtd_ = rbp[THIS+K_MTDIDX2].mtdNC;\
	KonohaStack *sfp_ = SFP(rshift(rbp, THIS)); \
	sfp_[K_RTNIDX].o = CTO;\
	sfp_[K_RTNIDX].uline = UL;\
	sfp_[K_SHIFTIDX].shift = THIS; \
	sfp_[K_PCIDX].pc = PC_NEXT(pc);\
	sfp_[K_MTDIDX].mtdNC = mtd_;\
	KonohaRuntime_setesp(kctx, SFP(rshift(rbp, espshift)));\
	(mtd_)->invokeMethodFunc(kctx, sfp_); \
	sfp_[K_MTDIDX].mtdNC = NULL;\
} while (0)

#define OPEXEC_VCALL(UL, THIS, espshift, mtdO, CTO) do {\
	kMethod *mtd_ = mtdO;\
	KonohaRuntime_setesp(kctx, SFP(rshift(rbp, espshift)));\
	OPEXEC_CHKSTACK(UL);\
	rbp = rshift(rbp, THIS);\
	rbp[K_ULINEIDX2-1].o = CTO;\
	rbp[K_ULINEIDX2].uline = UL;\
	rbp[K_SHIFTIDX2].shift = THIS;\
	rbp[K_PCIDX2].pc = PC_NEXT(pc);\
	pc = (mtd_)->pc_start;\
	GOTO_PC(pc); \
} while (0)

#define OPEXEC_SCALL(UL, THIS, espshift, mtdO, CTO) do {\
	kMethod *mtd_ = mtdO;\
	/*prefetch((mtd_)->invokeMethodFunc);*/\
	KonohaStack *sfp_ = SFP(rshift(rbp, THIS)); \
	sfp_[K_RTNIDX].o = CTO;\
	sfp_[K_RTNIDX].uline = UL;\
	sfp_[K_SHIFTIDX].shift = THIS; \
	sfp_[K_PCIDX].pc = PC_NEXT(pc);\
	sfp_[K_MTDIDX].mtdNC = mtd_;\
	KonohaRuntime_setesp(kctx, SFP(rshift(rbp, espshift)));\
	(mtd_)->invokeMethodFunc(kctx, sfp_); \
	sfp_[K_MTDIDX].mtdNC = NULL;\
} while (0)


#define OPEXEC_RET() do {\
	(void)op;\
	intptr_t vshift = rbp[K_SHIFTIDX2].shift;\
	VirtualMachineInstruction *vpc = rbp[K_PCIDX2].pc;\
	rbp[K_MTDIDX2].mtdNC = NULL;\
	rbp = rshift(rbp, -vshift); \
	pc = vpc; \
	GOTO_PC(pc);\
} while (0)

#define OPEXEC_JMP(PC, JUMP) do {\
	PC; \
	goto JUMP; \
} while (0)

#define OPEXEC_JMPT(PC, JUMP, N) do {\
	if(rbp[N].boolValue) {\
		OPEXEC_JMP(PC, JUMP); \
	} \
} while (0)

#define OPEXEC_JMPF(PC, JUMP, N) do {\
	if(!rbp[N].boolValue) {\
		OPEXEC_JMP(PC, JUMP); \
	} \
} while (0)

#define OPEXEC_TRYJMP(PC, JUMP) do {\
	pc = KonohaVirtualMachine_tryJump(kctx, (KonohaStack*)rbp, PC+1);\
	if(pc == NULL) {\
		OPEXEC_JMP(PC, JUMP); \
	} \
} while (0)

#define OPEXEC_BNOT(c, a)     rbp[c].boolValue = !(rbp[a].boolValue)

#define OPEXEC_TRACE(UL, THIS, F) do {\
	F(kctx, SFP(rshift(rbp, THIS)), UL);\
} while (0)

#define OPEXEC_CHKSTACK(UL) do {\
	if(unlikely(kctx->esp > kctx->stack->stack_uplimit)) {\
		kfileline_t uline = (UL == 0) ? rbp[K_ULINEIDX2].uline : UL;\
		KLIB KonohaRuntime_raise(kctx, EXPT_("StackOverflow"), SFP(rbp), uline, NULL);\
	}\
	if(kctx->safepoint != 0) { \
		kfileline_t uline = (UL == 0) ? rbp[K_ULINEIDX2].uline : UL;\
		KonohaVirtualMachine_onSafePoint(kctx, (KonohaStack*)rbp, uline);\
	} \
} while (0)


#define OPEXEC_SAFEPOINT(UL, espidx) do {\
	if(kctx->safepoint != 0) { \
		KonohaRuntime_setesp(kctx, SFP(rshift(rbp, espidx)));\
		KonohaVirtualMachine_onSafePoint(kctx, (KonohaStack*)rbp, UL); \
	} \
} while (0)

#define OPEXEC_ERROR(UL, msg, ESP) do {\
	KLIB KonohaRuntime_raise(kctx, EXPT_("RuntimeScript"), SFP(rbp), UL, msg);\
} while (0)

#define KLR_LDMTD(ctx, thisidx, ldmtd, hc, mtdO) do {\
	ldmtd(ctx, SFP(rbp), op);\
} while (0)


#ifdef OPOLD
/* ------------------------------------------------------------------------ */
/* KCODE */

#define R_NEXTIDX (K_NEXTIDX)
#define Rn_(x)    (rshift(rbp,x)->unboxValue)
#define Ri_(x)    (rshift(rbp,x)->intValue)
#define Rf_(x)    (rshift(rbp,x)->floatValue)
#define Rb_(x)    (rshift(rbp,x)->boolValue)
#define Ro_(x)    (rshift(rbp,x)->o)
#define Rh_(x)    (rshift(rbp,x)->hdr)
#define Rba_(x)   (rshift(rbp,x)->ba)
#define Ra_(x)    (rshift(rbp,x)->a)
#define Rx_(x)    (rshift(rbp,x)->ox)

#define RXo_(x)    (Rx_(x.i)->fieldObjectItems[x.n])
//#define RXd_(x)   (*((kunbox_t*) Rx_(x.i)->fields+x.n))
#define RXd_(x)   (*((kint_t*) Rx_(x.i)->fields+x.n))
#define SFP(rbp)  ((KonohaStack*)(rbp))
#define SFPIDX(n) ((n)/2)
#define RBP(sfp)  ((krbp_t*)(sfp))

#define PC_PREV(pc)   pc-1


/* [HALT] */

#define KLR_HALT() do {\
	THROW_Halt(kctx, SFP(rbp), "HALT"); \
	goto L_RETURN;\
} while (0)

/* [MOV, SET] */

/* NSET */

#define OPEXEC_NNMOV(a, b, c, d) do {\
	Rn_(a) = Rn_(b);\
	Rn_(c) = Rn_(d);\
} while (0)

#define OPEXEC_NSET2(a, n, n2) do {\
	Rn_(a) = n;\
	Rn_(a+R_NEXTIDX) = n2;\
} while (0)

#define OPEXEC_NSET3(a, n, n2, n3) do {\
	Rn_(a) = n;\
	Rn_(a+R_NEXTIDX) = n2;\
	Rn_(a+R_NEXTIDX+R_NEXTIDX) = n3;\
} while (0)

#define OPEXEC_NSET4(a, n, n2, n3, n4) do {\
	Rn_(a) = n;\
	Rn_(a+R_NEXTIDX) = n2;\
	Rn_(a+R_NEXTIDX+R_NEXTIDX) = n3;\
	Rn_(a+R_NEXTIDX+R_NEXTIDX+R_NEXTIDX) = n4;\
} while (0)

#define OPEXEC_XNSET(a, b)    RXd_(a) = b
#define OPEXEC_XNMOVx(a, b)   RXd_(a) = RXd_(b)

/* OSET */
#define knh_Object_RCinc(v_) ((void)v_)
#define knh_Object_RCdec(v_) ((void)v_)
#define Object_isRC0(v_) (false)
#define knh_Object_RCfree(kctx, v_) ((void)v_)

#define OPEXEC_RCINC(a) do {\
	RCGC_(kObject *v_ = Ro_(a);)\
	knh_Object_RCinc(v_);\
} while (0)

#define OPEXEC_RCDEC(a) do {\
	kObject *v_ = Ro_(a);\
	knh_Object_RCinc(v_);\
	knh_Object_RCdec(v_);\
	if(Object_isRC0(v_)) {\
		knh_Object_RCfree(kctx, v_);\
	}\
} while (0)

#define OPEXEC_RCINCx(a) do {\
	RCGC_(kObject *v_ = RXo_(a);)\
	knh_Object_RCinc(v_);\
} while (0)

#define OPEXEC_RCDECx(a) do {\
	kObject *v_ = RXo_(a);\
	knh_Object_RCdec(v_);\
	if(Object_isRC0(v_)) {\
		knh_Object_RCfree(kctx, v_);\
	}\
} while (0)

#ifdef K_USING_GENGC
#define klr_xmov(parent, v1, v2) do {\
	kObject *v1_ = (kObject*)v1;\
	kObject *v2_ = (kObject*)v2;\
	knh_Object_RCinc(v2_);\
	knh_Object_RCdec(v1_);\
	if(Object_isRC0(v1_)) {\
		knh_Object_RCfree(ctx, v1_);\
	}\
	knh_writeBarrier(parent, v2_);\
	v1 = v2_;\
} while (0)

#define klr_mov(ctx, v1, v2) do {\
	kObject *v1_ = (kObject*)v1;\
	kObject *v2_ = (kObject*)v2;\
	knh_Object_RCinc(v2_);\
	knh_Object_RCdec(v1_);\
	if(Object_isRC0(v1_)) {\
		knh_Object_RCfree(ctx, v1_);\
	}\
	v1 = v2_;\
} while (0)

#else

#define klr_mov(v1, v2) do {\
	kObject *v1_ = (kObject*)v1;\
	kObject *v2_ = (kObject*)v2;\
	knh_Object_RCinc(v2_);\
	knh_Object_RCdec(v1_);\
	if(Object_isRC0(v1_)) {\
		knh_Object_RCfree(ctx, v1_);\
	}\
	v1 = v2_;\
} while (0)

#endif

#define OPEXEC_OSET(a, v) do {\
	klr_mov(Ro_(a), v);\
} while (0)

#define OPEXEC_OSET2(a, v, v2) do {\
	OPEXEC_OSET(a, v);\
	klr_mov(Ro_(a+R_NEXTIDX), v2);\
} while (0)

#define OPEXEC_OSET3(a, v, v2, v3) do {\
	OPEXEC_OSET2(a, v, v2);\
	klr_mov(Ro_(a+R_NEXTIDX+R_NEXTIDX), v3);\
} while (0)

#define OPEXEC_OSET4(a, v, v2, v3, v4) do {\
	OPEXEC_OSET3(a, v, v2, v3);\
	klr_mov(Ro_(a+R_NEXTIDX+R_NEXTIDX+R_NEXTIDX), v4);\
} while (0)

#define OPEXEC_OMOV(a, b) do {\
	klr_mov(Ro_(a), Ro_(b));\
} while (0)

#define OPEXEC_ONMOV(a, b, c, d) do {\
	OPEXEC_OMOV(a, b);\
	OPEXEC_NMOV(c, d, /*TODO*/0);\
} while (0)

#define OPEXEC_OOMOV(a, b, c, d) do {\
	OPEXEC_OMOV(a, b);\
	OPEXEC_OMOV(c, d);\
} while (0)

#define OPEXEC_OMOVx(a, b) do {\
	kObject *v_ = RXo_(b);\
	klr_mov(Ro_(a), v_);\
} while (0)

#ifdef K_USING_GENGC
#define OPEXEC_XMOV(a, b)     klr_xmov(Rx_(a.i), RXo_(a), Ro_(b))
#define OPEXEC_XMOVx(a, b)    klr_xmov(Rx_(a.i), RXo_(a), RXo_(b))
#define OPEXEC_XOSET(a, b)    klr_xmov(Rx_(a.i), RXo_(a), b)
#else
#define OPEXEC_XMOV(a, b)     klr_mov(RXo_(a), Ro_(b))
#define OPEXEC_XMOVx(a, b)    klr_mov(RXo_(a), RXo_(b))
#define OPEXEC_XOSET(a, b)    klr_mov(RXo_(a), b)
#endif


/* ------------------------------------------------------------------------ */
/* [CALL] */

#define OPEXEC_FASTCALL0(c, thisidx, rix, espidx, fcall) do {\
	KonohaRuntime_setesp(kctx, SFP(rshift(rbp, espidx)));\
	fcall(kctx, SFP(rshift(rbp, thisidx)), (long)rix);\
} while (0)

/* ------------------------------------------------------------------------- */
/* VCALL */

#define OPEXEC_VCALL_(UL, THIS, espshift, mtdO, CTO) do {\
	kMethod *mtd_ = mtdO;\
	KonohaRuntime_setesp(kctx, SFP(rshift(rbp, espshift)));\
	OPEXEC_CHKSTACK(UL);\
	rbp = rshift(rbp, THIS);\
	rbp[K_ULINEIDX2-1].o = CTO;\
	rbp[K_ULINEIDX2].uline = UL;\
	rbp[K_SHIFTIDX2].shift = THIS;\
	rbp[K_PCIDX2].pc = PC_NEXT(pc);\
	pc = (mtd_)->pc_start;\
	GOTO_PC(pc); \
} while (0)


#define OPEXEC_JMP_(PC, JUMP)   OPEXEC_RET()

#define OPEXEC_YIELD(espidx) do {\
	KonohaRuntime_setesp(kctx, SFP(rshift(rbp,espidx)));\
	goto L_RETURN;\
} while (0)

#define OPEXEC_LDMTD(thisidx, ldmtd, hc, mtdO) do {\
	ldmtd(kctx, SFP(rbp), op);\
} while (0)

/**
#define OPEXEC_VINVOKE(ctx, rtnidx, thisidx, espshift) do {\
kMethod *mtd_ = (rbp[thisidx].fo)->mtd;\
KonohaRuntime_setesp(ctx, SFP(rshift(rbp, espshift)));\
rbp = rshift(rbp, thisidx);\
rbp[K_SHIFTIDX2].shift = thisidx;\
rbp[K_PCIDX2].pc = PC_NEXT(pc);\
rbp[K_MTDIDX2].mtdNC = mtd_;\
pc = (mtd_)->pc_start;\
GOTO_PC(pc); \
} while (0)
 **/

#define OPEXEC_THUNK(rtnidx, thisidx, espshift, mtdO) do {\
	kMethod *mtd_ = mtdO == NULL ? rbp[thisidx+K_MTDIDX2].mtdNC : mtdO;\
	KonohaRuntime_setesp(kctx, SFP(rshift(rbp, espshift)));\
	knh_stack_newThunk(kctx, (KonohaStack*)rshift(rbp, thisidx));\
} while (0)

#define OPEXEC_FUNCCALL() do {\
	(rbp[K_MTDIDX2].mtdNC)->invokeMethodFunc(kctx, SFP(rbp), K_RTNIDX);\
	KLR_RET();\
} while (0)

#define OPEXEC_VEXEC() do {\
	VirtualMachineInstruction *vpc = PC_NEXT(pc);\
	pc = (rbp[K_MTDIDX2].mtdNC)->pc_start;\
	rbp[K_SHIFTIDX2].shift = 0;\
	rbp[K_PCIDX2].pc = vpc;\
	GOTO_PC(pc); \
} while (0)

/* ------------------------------------------------------------------------- */

#define OPEXEC_iCAST(c, a) Ri_(c) = (kint_t)Rf_(a)
#define OPEXEC_fCAST(c, a) Rf_(c) = (kfloat_t)Ri_(a)

#define OPEXEC_SCAST(rtnidx, thisidx, rix, espidx, tmr) do {\
	KonohaRuntime_setesp(kctx, SFP(rshift(rbp, espidx)));\
	knh_TypeMap_exec(kctx, tmr, SFP(rshift(rbp,thisidx)), rix); \
} while (0)

#define OPEXEC_TCAST(kctx, rtnidx, thisidx, rix, espidx, tmr) do {\
	kTypeMap *tmr_ = tmr; \
	KonohaStack *sfp_ = SFP(rshift(rbp,thisidx));\
	KonohaClass scid = SP(tmr_)->scid, this_cid = O_typeId(sfp_[0].o);\
	if(this_cid != scid) {\
		tmr_ = knh_findTypeMapNULL(kctx, scid, SP(tmr)->tcid);\
		KSETv(((klr_TCAST_t*)op)->cast, tmr_);\
	}\
	KonohaRuntime_setesp(kctx, SFP(rshift(rbp, espidx)));\
	knh_TypeMap_exec(kctx, tmr_, sfp_, rix); \
} while (0)

#define OPEXEC_ACAST(rtnidx, thisidx, rix, espidx, tmr) do {\
	kTypeMap *tmr_ = tmr; \
	KonohaClass tcid = SP(tmr_)->tcid, this_cid = O_typeId(Ro_(thisidx));\
	if(!class_isa(this_cid, tcid)) {\
		KonohaClass scid = SP(tmr_)->scid;\
		if(this_cid != scid) {\
			tmr_ = knh_findTypeMapNULL(kctx, scid, tcid);\
			KNH_SETv(((klr_ACAST_t*)op)->cast, tmr_);\
		}\
		/*KonohaRuntime_setesp(kctx, SFP(rshift(rbp, espidx)));*/\
		knh_TypeMap_exec(kctx, tmr_, SFP(rshift(rbp,thisidx)), rix); \
	}\
} while (0)

#define OPEXEC_TR(c, a, rix, ct, f) f(kctx, SFP(rshift(rbp, a)), (long)rix, ct)

/* ------------------------------------------------------------------------ */

#define OPEXEC_ONCE(PC, JUMP) ((klr_ONCE_t*)op)->opcode = OPCODE_JMP

#define OPEXEC_bNUL(c, a)  Rb_(c) = IS_NULL(Ro_(a))
#define OPEXEC_bNN(c, a)   Rb_(c) = IS_NOTNULL(Ro_(a))

/* ------------------------------------------------------------------------- */

#define OPEXEC_NEXT(PC, JUMP, rtnidx, ib, rix, espidx) do {\
	KonohaStack *itrsfp_ = SFP(rshift(rbp, ib)); \
	DBG_ASSERT(IS_bIterator(itrsfp_[0].it));\
	KonohaRuntime_setesp(kctx, SFP(rshift(rbp, espidx)));\
	if(!((itrsfp_[0].it)->fnext_1(kctx, itrsfp_, rix))) { \
		OPEXEC_JMP(PC, JUMP); \
	} \
} while (0)

/* ------------------------------------------------------------------------- */

//#define NPC  /* for KNH_TRY */

#ifdef K_USING_SETJMP_

#define OPEXEC_TRY(PC, JUMP, hn) do {\
	kExceptionHandler* _hdr = Rh_(hn); \
	if(!IS_ExceptionHandler(_hdr)) { \
		_hdr = new_(ExceptionHandler); \
		klr_mov(Ro_(hn), _hdr); \
	} \
	int jump = knh_setjmp(DP(_hdr)->jmpbuf); \
	if(jump == 0) {\
		knh_ExceptionHandlerEX_t* _hdrEX = DP(Rh_(hn));\
		_hdrEX->pc = PC_NEXT(pc); \
		_hdrEX->op = op;\
		_hdrEX->sfpidx = (SFP(rbp) - ctx->stack); \
		_hdr = Rh_(hn);\
		_hdr->espidx = (ctx->esp - ctx->stack); \
		_hdr->parentNC = ctx->ehdrNC;\
		((KonohaContextVar*)ctx)->ehdrNC = _hdr; \
	} else { \
		_hdr = ctx->ehdrNC;\
		knh_ExceptionHandlerEX_t* _hdrEX = DP(_hdr);\
		pc = _hdrEX->pc; \
		rbp = RBP(ctx->stack + _hdrEX->sfpidx);\
		KonohaRuntime_setesp(ctx, (ctx->stack + _hdr->espidx));\
		op = _hdrEX->op;\
		((KonohaContextVar*)ctx)->ehdrNC = _hdr->parentNC;\
		OPEXEC_JMP(PC, JUMP);\
	}\
} while (0)

#define OPEXEC_TRYEND(ctx, hn) do {\
	kExceptionHandler* _hdr = Rh_(hn); \
	DBG_ASSERT(IS_ExceptionHandler(_hdr)); \
	((KonohaContextVar*)ctx)->ehdrNC = _hdr->parentNC;\
	klr_mov(ctx, Ro_(hn), KNH_TNULL(ExceptionHandler));\
} while (0)

#else

#define OPEXEC_TRY(PC, JUMP, hn) do {\
	kExceptionHandler* _hdr = Rh_(hn); \
	if(!IS_ExceptionHandler(_hdr)) { \
		_hdr = new_(ExceptionHandler); \
		klr_mov(ctx, Ro_(hn), _hdr); \
	} \
	_hdr = ExceptionHandler_setjmp(ctx, _hdr); \
	if(_hdr == NULL) {\
		knh_ExceptionHandlerEX_t* _hdrEX = DP(Rh_(hn));\
		_hdrEX->pc  = PC_NEXT(pc); \
		_hdrEX->op  = op;\
		_hdrEX->sfpidx = (SFP(rbp) - ctx->stack); \
		_hdr = Rh_(hn);\
		_hdr->espidx = (ctx->esp - ctx->stack); \
		_hdr->parentNC = ctx->ehdrNC;\
		((KonohaContextVar*)ctx)->ehdrNC = _hdr; \
	} else { \
		knh_ExceptionHandlerEX_t* _hdrEX = DP(_hdr);\
		pc = _hdrEX->pc; \
		rbp = RBP(ctx->stack + _hdrEX->sfpidx);\
		KonohaRuntime_setesp(ctx, (ctx->stack + _hdr->espidx));\
		op = _hdrEX->op;\
		((KonohaContextVar*)ctx)->ehdrNC = _hdr->parentNC;\
		OPEXEC_JMP(PC, JUMP);\
	}\
} while (0)

#define OPEXEC_TRYEND(ctx, hn) do {\
	kExceptionHandler* _hdr = Rh_(hn); \
	DBG_ASSERT(IS_ExceptionHandler(_hdr)); \
	DP(_hdr)->return_address = NULL;\
	DP(_hdr)->frame_address  = NULL;\
	((KonohaContextVar*)ctx)->ehdrNC = _hdr->parentNC;\
	klr_mov(ctx, Ro_(hn), KNH_TNULL(ExceptionHandler));\
} while (0)

#endif

#define OPEXEC_THROW(ctx, start) do {\
	knh_throw(ctx, SFP(rbp), SFPIDX(start)); \
} while (0)

#define OPEXEC_ASSERT(ctx, start, uline) do {\
	knh_assert(ctx, SFP(rbp), SFPIDX(start), uline); \
} while (0)

#define OPEXEC_ERR(ctx, start, msg) do {\
	kException *e_ = new_Error(ctx, 0, msg);\
	KonohaContext_setThrowingException(ctx, e_);\
	knh_throw(ctx, SFP(rbp), SFPIDX(start)); \
} while (0)

#define OPEXEC_CATCH0(PC, JUMP, en, emsg)

#define OPEXEC_CATCH(PC, JUMP, en, emsg) do {\
	if(!isCATCH(ctx, rbp, en, emsg)) { \
		OPEXEC_JMP(PC, JUMP); \
	} \
} while (0)

#define OPEXEC_CHKIN(ctx, on, fcheckin) do {\
	kObject *o_ = Ro_(on);\
	fcheckin(ctx, SFP(rbp), RAWPTR(o_));\
	Context_push(ctx, o_);\
} while (0)

#define OPEXEC_CHKOUT(ctx, on, fcheckout) do {\
	kObject *o_ = Context_pop(ctx);\
	DBG_ASSERT(o_ == Ro_(on));\
	fcheckout(ctx, RAWPTR(o_), 0);\
} while (0)

/* ------------------------------------------------------------------------ */

#define OPEXEC_P(ctx, fprint, flag, msg, n) fprint(ctx, SFP(rbp), op)

#define OPEXEC_PROBE(ctx, sfpidx, fprobe, n, ns) do {\
	fprobe(ctx, SFP(rbp), op);\
} while (0)

/* ------------------------------------------------------------------------ */

#define OPEXEC_bNOT(ctx, c, a)     Rb_(c) = !(Rb_(a))

#define OPEXEC_iINC(a)       Ri_(a)++
#define OPEXEC_iDEC(a)       Ri_(a)--

#define OPEXEC_iNEG(c, a)     Ri_(c) = -(Ri_(a))
#define OPEXEC_iTR(c, a, f)      Ri_(c) = f((long)Ri_(a))
#define OPEXEC_iADD(c, a, b)  Ri_(c) = (Ri_(a) + Ri_(b))
#define OPEXEC_iADDC(c, a, n) Ri_(c) = (Ri_(a) + n)
#define OPEXEC_iSUB(c, a, b)  Ri_(c) = (Ri_(a) - Ri_(b))
#define OPEXEC_iSUBC(c, a, n) Ri_(c) = (Ri_(a) - n)
#define OPEXEC_iMUL(c, a, b)  Ri_(c) = (Ri_(a) * Ri_(b))
#define OPEXEC_iMULC(c, a, n) Ri_(c) = (Ri_(a) * n)
#define OPEXEC_iDIV(c, a, b)  Ri_(c) = (Ri_(a) / Ri_(b));
#define OPEXEC_iDIV2(ctx, c, a, b) do {\
	SYSLOG_iZERODIV(ctx, sfp, Ri_(b)); \
	Ri_(c) = (Ri_(a) / Ri_(b)); \
} while (0)

#define OPEXEC_iDIVC(c, a, n)  Ri_(c) = (Ri_(a) / n)
#define OPEXEC_iMOD(c, a, b)  Ri_(c) = (Ri_(a) % Ri_(b))
#define OPEXEC_iMOD2(c, a, b) do {\
	SYSLOG_iZERODIV(ctx, sfp, Ri_(b)); \
	Ri_(c) = (Ri_(a) % Ri_(b)); \
} while (0)

#define OPEXEC_iMODC(c, a, n)  Ri_(c) = (Ri_(a) % n)
#define OPEXEC_iEQ(c, a, b)  Rb_(c) = (Ri_(a) == Ri_(b));
#define OPEXEC_iEQC(c, a, n)  Rb_(c) = (Ri_(a) == n);
#define OPEXEC_iNEQ(c, a, b)  Rb_(c) = (Ri_(a) != Ri_(b));
#define OPEXEC_iNEQC(c, a, n)  Rb_(c) = (Ri_(a) != n);
#define OPEXEC_iLT(c, a, b)  Rb_(c) = (Ri_(a) < Ri_(b));
#define OPEXEC_iLTC(c, a, n)  Rb_(c) = (Ri_(a) < n);
#define OPEXEC_iLTE(c, a, b)  Rb_(c) = (Ri_(a) <= Ri_(b));
#define OPEXEC_iLTEC(c, a, n)  Rb_(c) = (Ri_(a) <= n);
#define OPEXEC_iGT(c, a, b)  Rb_(c) = (Ri_(a) > Ri_(b));
#define OPEXEC_iGTC(c, a, n)  Rb_(c) = (Ri_(a) > n);
#define OPEXEC_iGTE(c, a, b)  Rb_(c) = (Ri_(a) >= Ri_(b));
#define OPEXEC_iGTEC(c, a, n)  Rb_(c) = (Ri_(a) >= n);

#define OPEXEC_iANDC(c, a, n)  Ri_(c) = (Ri_(a) & (n))
#define OPEXEC_iORC(c, a, n)   Ri_(c) = (Ri_(a) | (n))
#define OPEXEC_iXORC(c, a, n)  Ri_(c) = (Ri_(a) ^ (n))
#define OPEXEC_iLSFTC(c, a, n) Ri_(c) = (Ri_(a) << (n))
#define OPEXEC_iRSFTC(c, a, n) Ri_(c) = (Ri_(a) >> (n))
#define OPEXEC_iAND(c, a, b)   OPEXEC_iANDC(c, a, Ri_(b))
#define OPEXEC_iOR(c, a, b)    OPEXEC_iORC(c, a, Ri_(b))
#define OPEXEC_iXOR(c, a, b)   OPEXEC_iXORC(c, a, Ri_(b))
#define OPEXEC_iLSFT(c, a, b)  OPEXEC_iLSFTC(c, a, Ri_(b))
#define OPEXEC_iRSFT(c, a, b)  OPEXEC_iRSFTC(c, a, Ri_(b))

#define BR_(EXPR, PC, JUMP) if(EXPR) {} else {OPEXEC_JMP(PC, JUMP); }

#define OPEXEC_bJNUL(PC, JUMP, a)    BR_(IS_NULL(Ro_(a)), PC, JUMP)
#define OPEXEC_bJNN(PC, JUMP, a)     BR_(IS_NOTNULL(Ro_(a)), PC, JUMP)
#define OPEXEC_bJNOT(PC, JUMP, a)     BR_(!Rb_(a), PC, JUMP)
#define OPEXEC_iJEQ(PC, JUMP, a, b)   BR_((Ri_(a) == Ri_(b)), PC, JUMP)
#define OPEXEC_iJEQC(PC, JUMP, a, n)  BR_((Ri_(a) == n), PC, JUMP)
#define OPEXEC_iJNEQ(PC, JUMP, a, b)  BR_((Ri_(a) != Ri_(b)), PC, JUMP)
#define OPEXEC_iJNEQC(PC, JUMP, a, n) BR_((Ri_(a) != n), PC, JUMP)
#define OPEXEC_iJLT(PC, JUMP, a, b)   BR_((Ri_(a) < Ri_(b)), PC, JUMP)
#define OPEXEC_iJLTC(PC, JUMP, a, n)  BR_((Ri_(a) < n), PC, JUMP)
#define OPEXEC_iJLTE(PC, JUMP, a, b)  BR_((Ri_(a) <= Ri_(b)), PC, JUMP)
#define OPEXEC_iJLTEC(PC, JUMP, a, n) BR_((Ri_(a) <= n), PC, JUMP)
#define OPEXEC_iJGT(PC, JUMP, a, b)   BR_((Ri_(a) > Ri_(b)), PC, JUMP)
#define OPEXEC_iJGTC(PC, JUMP, a, n)  BR_((Ri_(a) > n), PC, JUMP)
#define OPEXEC_iJGTE(PC, JUMP, a, b)  BR_((Ri_(a) >= Ri_(b)), PC, JUMP)
#define OPEXEC_iJGTEC(PC, JUMP, a, n) BR_((Ri_(a) >= n), PC, JUMP)

/* ------------------------------------------------------------------------ */

#define OPEXEC_fNEG(c, a)     Rf_(c) = -(Rf_(a))
#define OPEXEC_fTR(c, a, f)      Rf_(c) = f((double)Rf_(a))
#define OPEXEC_fADD(c, a, b)  Rf_(c) = (Rf_(a) + Rf_(b))
#define OPEXEC_fADDC(c, a, n) Rf_(c) = (Rf_(a) + n)
#define OPEXEC_fSUB(c, a, b)  Rf_(c) = (Rf_(a) - Rf_(b))
#define OPEXEC_fSUBC(c, a, n) Rf_(c) = (Rf_(a) - n)
#define OPEXEC_fMUL(c, a, b)  Rf_(c) = (Rf_(a) * Rf_(b))
#define OPEXEC_fMULC(c, a, n) Rf_(c) = (Rf_(a) * n)
#define OPEXEC_fDIV(c, a, b)  Rf_(c) = (Rf_(a) / Rf_(b))
#define OPEXEC_fDIV2(ctx, c, a, b) do {\
	SYSLOG_fZERODIV2(ctx, sfp, Rf_(b)); \
	Rf_(c) = (Rf_(a) / Rf_(b)); \
} while (0)

#define OPEXEC_fDIVC(c, a, n)  Rf_(c) = (Rf_(a) / n)
#define OPEXEC_fEQ(c, a, b) Rb_(c) = (Rf_(a) == Rf_(b))
#define OPEXEC_fEQC(c, a, n) Rb_(c) = (Rf_(a) == n)
#define OPEXEC_fNEQ(c, a, b)  Rb_(c) = (Rf_(a) != Rf_(b))
#define OPEXEC_fNEQC(c, a, n)  Rb_(c) = (Rf_(a) != n)
#define OPEXEC_fLT(c, a, b)  Rb_(c) = (Rf_(a) < Rf_(b))
#define OPEXEC_fLTC(c, a, n)  Rb_(c) = (Rf_(a) < n)

#define OPEXEC_fLTE(c, a, b)  Rb_(c) = (Rf_(a) <= Rf_(b))
#define OPEXEC_fLTEC(c, a, n) Rb_(c) = (Rf_(a) <= n)
#define OPEXEC_fGT(c, a, b)  Rb_(c) = (Rf_(a) > Rf_(b))
#define OPEXEC_fGTC(c, a, n)  Rb_(c) = (Rf_(a) > n)
#define OPEXEC_fGTE(c, a, b)  Rb_(c) = (Rf_(a) >= Rf_(b))
#define OPEXEC_fGTEC(c, a, n)  Rb_(c) = (Rf_(a) >= n)

#define OPEXEC_fJEQ(PC, JUMP, a, b)   BR_((Rf_(a) == Rf_(b)), PC, JUMP)
#define OPEXEC_fJEQC(PC, JUMP, a, n)  BR_((Rf_(a) == n), PC, JUMP)
#define OPEXEC_fJNEQ(PC, JUMP, a, b)  BR_((Rf_(a) != Rf_(b)), PC, JUMP)
#define OPEXEC_fJNEQC(PC, JUMP, a, n) BR_((Rf_(a) != n), PC, JUMP)
#define OPEXEC_fJLT(PC, JUMP, a, b)   BR_((Rf_(a) < Rf_(b)), PC, JUMP)
#define OPEXEC_fJLTC(PC, JUMP, a, n)  BR_((Rf_(a) < n), PC, JUMP)
#define OPEXEC_fJLTE(PC, JUMP, a, b)  BR_((Rf_(a) <= Rf_(b)), PC, JUMP)
#define OPEXEC_fJLTEC(PC, JUMP, a, n) BR_((Rf_(a) <= n), PC, JUMP)
#define OPEXEC_fJGT(PC, JUMP, a, b)   BR_((Rf_(a) > Rf_(b)), PC, JUMP)
#define OPEXEC_fJGTC(PC, JUMP, a, n)  BR_((Rf_(a) > n), PC, JUMP)
#define OPEXEC_fJGTE(PC, JUMP, a, b)  BR_((Rf_(a) >= Rf_(b)), PC, JUMP)
#define OPEXEC_fJGTEC(PC, JUMP, a, n) BR_((Rf_(a) >= n), PC, JUMP)

/* ------------------------------------------------------------------------ */

#define klr_array_index(n, size)   (size_t)n
#define THROW_OutOfRange(kctx, lsfp, n, size) /*TODO*/
#ifdef OPCODE_CHKIDX
#define klr_array_check(n, size)
#else
#define klr_array_check(n, size) \
	if(unlikely(n >= size)) THROW_OutOfRange(kctx, SFP(rbp), n, size)

#endif

#define OPEXEC_CHKIDX(aidx, nidx) do {\
	size_t size_ = kArray_size(rbp[aidx].a);\
	size_t n_ = Ri_(nidx);\
	if(unlikely(n_ >= size_)) THROW_OutOfRange(kctx, SFP(rbp), n_, size_);\
} while (0)

#define OPEXEC_CHKIDXC(aidx, n) do {\
	size_t size_ = kArray_size(rbp[aidx].a);\
	if(unlikely(n >= size_)) THROW_OutOfRange(kctx, SFP(rbp), n, size_);\
} while (0)

#define OPEXEC_BGETIDXC(cidx, aidx, N) do {\
	kbytes_t *b_ = &BA_tobytes(rbp[aidx].ba);\
	size_t n_ = klr_array_index(N, b_->len);\
	klr_array_check(n_, b_->len);\
	Ri_(cidx) = b_->utext[n_];\
} while (0)

#define OPEXEC_BGETIDX(cidx, aidx, nidx) OPEXEC_BGETIDXC(cidx, aidx, Ri_(nidx))

#define OPEXEC_BSETIDXC(cidx, aidx, N, vidx) do {\
	kbytes_t *b_ = &BA_tobytes(Rba_(aidx));\
	size_t n_ = klr_array_index(N, b_->len);\
	klr_array_check(n_, b_->len);\
	b_->ubuf[n_] = (kchar_t)Ri_(vidx);\
	Ri_(cidx) = Ri_(vidx);\
} while (0)

#define OPEXEC_BSETIDX(cidx, aidx, nidx, vidx) OPEXEC_BSETIDXC(cidx, aidx, Ri_(nidx), vidx)

#define OPEXEC_OGETIDXC(cidx, aidx, N) do {\
	kArray *a_ = Ra_(aidx);\
	size_t n_ = klr_array_index(N, kArray_size(a_));\
	klr_array_check(n_, kArray_size(a_));\
	kObject *v_ = (a_)->objectItems[n_];\
	klr_mov(Ro_(cidx), v_);\
} while (0)

#define OPEXEC_OGETIDX(cidx, aidx, nidx) OPEXEC_OGETIDXC(cidx, aidx, Ri_(nidx))

#define OPEXEC_OSETIDXC(cidx, aidx, N, vidx) do {\
	kArray *a_ = Ra_(aidx);\
	size_t n_ = klr_array_index(N, kArray_size(a_));\
	klr_array_check(n_, kArray_size(a_));\
	klr_mov((a_)->objectItems[n_], Ro_(vidx));\
	klr_mov(Ro_(cidx), Ro_(vidx));\
} while (0)

#define OPEXEC_OSETIDX(cidx, aidx, nidx, vidx) OPEXEC_OSETIDXC(cidx, aidx, Ri_(nidx), vidx)

#define OPEXEC_NGETIDXC(cidx, aidx, N) do {\
	kArray *a_ = Ra_(aidx);\
	size_t n_ = klr_array_index(N, kArray_size(a_));\
	klr_array_check(n_, kArray_size(a_));\
	Rn_(cidx) = (a_)->unboxItems[n_];\
} while (0)

#define OPEXEC_NGETIDX(cidx, aidx, nidx) OPEXEC_NGETIDXC(cidx, aidx, Ri_(nidx))

#define OPEXEC_NSETIDXC(cidx, aidx, N, vidx) do {\
	kArray *a_ = Ra_(aidx);\
	size_t n_ = klr_array_index(N, kArray_size(a_));\
	klr_array_check(n_, kArray_size(a_));\
	Rn_(cidx) = (a_)->unboxItems[n_] = Rn_(vidx);\
} while (0)

#define OPEXEC_NSETIDX(cidx, aidx, nidx, vidx) OPEXEC_NSETIDXC(cidx, aidx, Ri_(nidx), vidx)

#endif

#ifdef __cplusplus
}
#endif

#endif /*KONOHA_VM_H_*/
