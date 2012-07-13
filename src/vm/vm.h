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
#define TY_BasicBlock    kmodcode->cBasicBlock->cid
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
	KonohaContextModule      h;
	kfileline_t      uline;
	kArray          *codeList;
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

struct klr_LDMTD_t;
typedef void (*klr_Fth)(KonohaContext *kctx, struct VirtualMachineInstruction *, void**);
typedef void (*klr_Floadmtd)(KonohaContext *kctx, KonohaStack *, struct klr_LDMTD_t *);
typedef kbool_t (*Fcallcc)(KonohaContext *kctx, KonohaStack *, int, int, void *);

typedef struct {
	ktype_t cid; kmethodn_t mn;
} kcachedata_t;

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

#define rshift(rbp, x_) (rbp+(x_))
#define SFP(rbp)  ((KonohaStack*)(rbp))
#define SFPIDX(n) ((n)/2)
#define RBP(sfp)  ((krbp_t*)(sfp))

#define OPEXEC_NOP() (void)op

#define OPEXEC_THCODE(F) { \
		F(kctx, pc, OPJUMP); \
		pc = PC_NEXT(pc);\
		goto L_RETURN; \
	}\

#define OPEXEC_ENTER() {\
		(void)op;\
		VirtualMachineInstruction *vpc = PC_NEXT(pc);\
		pc = (rbp[K_MTDIDX2].mtdNC)->pc_start;\
		rbp[K_SHIFTIDX2].shift = 0;\
		rbp[K_PCIDX2].pc = vpc;\
		GOTO_PC(pc); \
	}\

#define OPEXEC_NCALL() { \
		(void)op;\
		(rbp[K_MTDIDX2].mtdNC)->invokeMethodFunc(kctx, SFP(rbp));\
		OPEXEC_RET();\
	} \

#define OPEXEC_EXIT() {\
		(void)op;\
		pc = NULL; \
		goto L_RETURN;\
	}\

#define OPEXEC_NSET(A, N, CT) rbp[(A)].ndata = N
#define OPEXEC_NMOV(A, B, CT) rbp[(A)].ndata = rbp[(B)].ndata
#define OPEXEC_NMOVx(A, B, BX, CT) rbp[(A)].o = (rbp[(B)].toObjectVar)->fieldObjectItems[(BX)]
#define OPEXEC_XNMOV(A, AX, B, CT) (rbp[(A)].toObjectVar)->fieldObjectItems[AX] = rbp[(B)].o

#define OPEXEC_NEW(A, P, CT)   KSETv(rbp[(A)].o, KLIB new_kObject(kctx, CT, P))
#define OPEXEC_NULL(A, CT)     KSETv(rbp[(A)].o, knull(CT))
#define OPEXEC_BOX(A, B, CT)   KSETv(rbp[(A)].o, KLIB new_kObject(kctx, CT, rbp[(B)].ivalue))
#define OPEXEC_UNBOX(A, B, CT) rbp[(A)].ivalue = N_toint(rbp[B].o)

#define PC_NEXT(pc)   pc+1

#define OPEXEC_CHKSTACK(UL) \
	if(unlikely(kctx->esp > kctx->stack->stack_uplimit)) {\
		kreportf(CritTag, UL, "stack overflow");\
	}\


#define OPEXEC_CALL(UL, THIS, espshift, CTO) { \
		kMethod *mtd_ = rbp[THIS+K_MTDIDX2].mtdNC;\
		klr_setesp(kctx, SFP(rshift(rbp, espshift)));\
		OPEXEC_CHKSTACK(UL);\
		rbp = rshift(rbp, THIS);\
		rbp[K_ULINEIDX2-1].o = CTO;\
		rbp[K_ULINEIDX2].uline = UL;\
		rbp[K_SHIFTIDX2].shift = THIS;\
		rbp[K_PCIDX2].pc = PC_NEXT(pc);\
		pc = (mtd_)->pc_start;\
		GOTO_PC(pc); \
	} \

#define OPEXEC_VCALL(UL, THIS, espshift, mtdO, CTO) { \
		kMethod *mtd_ = mtdO;\
		klr_setesp(kctx, SFP(rshift(rbp, espshift)));\
		OPEXEC_CHKSTACK(UL);\
		rbp = rshift(rbp, THIS);\
		rbp[K_ULINEIDX2-1].o = CTO;\
		rbp[K_ULINEIDX2].uline = UL;\
		rbp[K_SHIFTIDX2].shift = THIS;\
		rbp[K_PCIDX2].pc = PC_NEXT(pc);\
		pc = (mtd_)->pc_start;\
		GOTO_PC(pc); \
	} \

#define OPEXEC_SCALL(UL, thisidx, espshift, mtdO, CTO) { \
		kMethod *mtd_ = mtdO;\
		/*prefetch((mtd_)->invokeMethodFunc);*/\
		KonohaStack *sfp_ = SFP(rshift(rbp, thisidx)); \
		sfp_[K_RTNIDX].o = CTO;\
		sfp_[K_RTNIDX].uline = UL;\
		sfp_[K_SHIFTIDX].shift = thisidx; \
		sfp_[K_PCIDX].pc = PC_NEXT(pc);\
		sfp_[K_MTDIDX].mtdNC = mtd_;\
		klr_setesp(kctx, SFP(rshift(rbp, espshift)));\
		(mtd_)->invokeMethodFunc(kctx, sfp_); \
		sfp_[K_MTDIDX].mtdNC = NULL;\
	} \


#define OPEXEC_RET() { \
		(void)op;\
		intptr_t vshift = rbp[K_SHIFTIDX2].shift;\
		VirtualMachineInstruction *vpc = rbp[K_PCIDX2].pc;\
		rbp[K_MTDIDX2].mtdNC = NULL;\
		rbp = rshift(rbp, -vshift); \
		pc = vpc; \
		GOTO_PC(pc);\
	}\

#define OPEXEC_JMP(PC, JUMP) {\
	PC; \
	goto JUMP; \
}\

#define OPEXEC_JMPT(PC, JUMP, N) \
	if(rbp[N].bvalue) {\
		OPEXEC_JMP(PC, JUMP); \
	} \

#define OPEXEC_JMPF(PC, JUMP, N) \
	if(!rbp[N].bvalue) {\
		OPEXEC_JMP(PC, JUMP); \
	} \

#define OPEXEC_BNOT(c, a)     rbp[c].bvalue = !(rbp[a].bvalue)

#ifdef K_USING_SAFEPOINT
#define KLR_SAFEPOINT(espidx) \
	if(kctx->safepoint != 0) { \
		klr_setesp(kctx, SFP(rshift(rbp, espidx)));\
		knh_checkSafePoint(kctx, (KonohaStack*)rbp, __FILE__, __LINE__); \
	} \

#else
#define OPEXEC_SAFEPOINT(RS)   klr_setesp(kctx, SFP(rshift(rbp, RS)));
#endif

#define OPEXEC_ERROR(start, msg) {\
		kreportf(NoneTag, 0, "RuntimeScriptException: %s", S_text(msg));\
		kraise(0);\
	}\

#define OPEXEC_ERROR2(start, msg) { \
		kException *e_ = new_Error(kctx, 0, msg);\
		KonohaContext_setThrowingException(kctx, e_);\
		knh_throw(kctx, SFP(rbp), SFPIDX(start)); \
	} \


#define KLR_LDMTD(ctx, thisidx, ldmtd, hc, mtdO) { \
		ldmtd(ctx, SFP(rbp), op);\
	} \


#ifdef OPOLD
/* ------------------------------------------------------------------------ */
/* KCODE */

#define R_NEXTIDX (K_NEXTIDX)
#define Rn_(x)    (rshift(rbp,x)->ndata)
#define Ri_(x)    (rshift(rbp,x)->ivalue)
#define Rf_(x)    (rshift(rbp,x)->fvalue)
#define Rb_(x)    (rshift(rbp,x)->bvalue)
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

#define KLR_HALT() {\
	THROW_Halt(kctx, SFP(rbp), "HALT"); \
	goto L_RETURN;\
}\

/* [MOV, SET] */

/* NSET */

#define OPEXEC_NNMOV(a, b, c, d) {\
		Rn_(a) = Rn_(b);\
		Rn_(c) = Rn_(d);\
	}\

#define OPEXEC_NSET2(a, n, n2) {\
		Rn_(a) = n;\
		Rn_(a+R_NEXTIDX) = n2;\
	}\

#define OPEXEC_NSET3(a, n, n2, n3) {\
		Rn_(a) = n;\
		Rn_(a+R_NEXTIDX) = n2;\
		Rn_(a+R_NEXTIDX+R_NEXTIDX) = n3;\
	}\

#define OPEXEC_NSET4(a, n, n2, n3, n4) {\
		Rn_(a) = n;\
		Rn_(a+R_NEXTIDX) = n2;\
		Rn_(a+R_NEXTIDX+R_NEXTIDX) = n3;\
		Rn_(a+R_NEXTIDX+R_NEXTIDX+R_NEXTIDX) = n4;\
	}\

#define OPEXEC_XNSET(a, b)    RXd_(a) = b
#define OPEXEC_XNMOVx(a, b)   RXd_(a) = RXd_(b)

/* OSET */
#define knh_Object_RCinc(v_) ((void)v_)
#define knh_Object_RCdec(v_) ((void)v_)
#define Object_isRC0(v_) (false)
#define knh_Object_RCfree(kctx, v_) ((void)v_)

#define OPEXEC_RCINC(a) {\
		RCGC_(kObject *v_ = Ro_(a);)\
		knh_Object_RCinc(v_);\
	}\

#define OPEXEC_RCDEC(a) {\
		kObject *v_ = Ro_(a);\
		knh_Object_RCinc(v_);\
		knh_Object_RCdec(v_);\
		if(Object_isRC0(v_)) {\
			knh_Object_RCfree(kctx, v_);\
		}\
	}\

#define OPEXEC_RCINCx(a) {\
		RCGC_(kObject *v_ = RXo_(a);)\
		knh_Object_RCinc(v_);\
	}\

#define OPEXEC_RCDECx(a) {\
		kObject *v_ = RXo_(a);\
		knh_Object_RCdec(v_);\
		if(Object_isRC0(v_)) {\
			knh_Object_RCfree(kctx, v_);\
		}\
	}\

#ifdef K_USING_GENGC
#define klr_xmov(parent, v1, v2) {\
	kObject *v1_ = (kObject*)v1;\
	kObject *v2_ = (kObject*)v2;\
	knh_Object_RCinc(v2_);\
	knh_Object_RCdec(v1_);\
	if(Object_isRC0(v1_)) {\
		knh_Object_RCfree(ctx, v1_);\
	}\
	knh_writeBarrier(parent, v2_);\
	v1 = v2_;\
}\

#define klr_mov(ctx, v1, v2) {\
	kObject *v1_ = (kObject*)v1;\
	kObject *v2_ = (kObject*)v2;\
	knh_Object_RCinc(v2_);\
	knh_Object_RCdec(v1_);\
	if(Object_isRC0(v1_)) {\
		knh_Object_RCfree(ctx, v1_);\
	}\
	v1 = v2_;\
}\

#else

#define klr_mov(v1, v2) {\
	kObject *v1_ = (kObject*)v1;\
	kObject *v2_ = (kObject*)v2;\
	knh_Object_RCinc(v2_);\
	knh_Object_RCdec(v1_);\
	if(Object_isRC0(v1_)) {\
		knh_Object_RCfree(ctx, v1_);\
	}\
	v1 = v2_;\
}\

#endif

#define OPEXEC_OSET(a, v) {\
	klr_mov(Ro_(a), v);\
}\

#define OPEXEC_OSET2(a, v, v2) {\
	OPEXEC_OSET(a, v);\
	klr_mov(Ro_(a+R_NEXTIDX), v2);\
}\

#define OPEXEC_OSET3(a, v, v2, v3) {\
	OPEXEC_OSET2(a, v, v2);\
	klr_mov(Ro_(a+R_NEXTIDX+R_NEXTIDX), v3);\
}\

#define OPEXEC_OSET4(a, v, v2, v3, v4) {\
	OPEXEC_OSET3(a, v, v2, v3);\
	klr_mov(Ro_(a+R_NEXTIDX+R_NEXTIDX+R_NEXTIDX), v4);\
}\

#define OPEXEC_OMOV(a, b) { \
	klr_mov(Ro_(a), Ro_(b));\
}\

#define OPEXEC_ONMOV(a, b, c, d) {\
	OPEXEC_OMOV(a, b);\
	OPEXEC_NMOV(c, d, /*TODO*/0);\
}\

#define OPEXEC_OOMOV(a, b, c, d) {\
	OPEXEC_OMOV(a, b);\
	OPEXEC_OMOV(c, d);\
}\

#define OPEXEC_OMOVx(a, b) {\
	kObject *v_ = RXo_(b);\
	klr_mov(Ro_(a), v_);\
}\

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

#define OPEXEC_FASTCALL0(c, thisidx, rix, espidx, fcall) { \
		klr_setesp(kctx, SFP(rshift(rbp, espidx)));\
		fcall(kctx, SFP(rshift(rbp, thisidx)), (long)rix);\
	} \

/* ------------------------------------------------------------------------- */
/* VCALL */

#define OPEXEC_VCALL_(UL, THIS, espshift, mtdO, CTO) { \
		kMethod *mtd_ = mtdO;\
		klr_setesp(kctx, SFP(rshift(rbp, espshift)));\
		OPEXEC_CHKSTACK(UL);\
		rbp = rshift(rbp, THIS);\
		rbp[K_ULINEIDX2-1].o = CTO;\
		rbp[K_ULINEIDX2].uline = UL;\
		rbp[K_SHIFTIDX2].shift = THIS;\
		rbp[K_PCIDX2].pc = PC_NEXT(pc);\
		pc = (mtd_)->pc_start;\
		GOTO_PC(pc); \
	} \


#define OPEXEC_JMP_(PC, JUMP)   OPEXEC_RET()

#define OPEXEC_YIELD(espidx) {\
		klr_setesp(kctx, SFP(rshift(rbp,espidx)));\
		goto L_RETURN;\
	}\

#define OPEXEC_LDMTD(thisidx, ldmtd, hc, mtdO) { \
		ldmtd(kctx, SFP(rbp), op);\
	} \

/**
#define OPEXEC_VINVOKE(ctx, rtnidx, thisidx, espshift) { \
		kMethod *mtd_ = (rbp[thisidx].fo)->mtd;\
		klr_setesp(ctx, SFP(rshift(rbp, espshift)));\
		rbp = rshift(rbp, thisidx);\
		rbp[K_SHIFTIDX2].shift = thisidx;\
		rbp[K_PCIDX2].pc = PC_NEXT(pc);\
		rbp[K_MTDIDX2].mtdNC = mtd_;\
		pc = (mtd_)->pc_start;\
		GOTO_PC(pc); \
	} \
**/

#define OPEXEC_THUNK(rtnidx, thisidx, espshift, mtdO) { \
		kMethod *mtd_ = mtdO == NULL ? rbp[thisidx+K_MTDIDX2].mtdNC : mtdO;\
		klr_setesp(kctx, SFP(rshift(rbp, espshift)));\
		knh_stack_newThunk(kctx, (KonohaStack*)rshift(rbp, thisidx));\
	} \

#define OPEXEC_FUNCCALL() { \
		(rbp[K_MTDIDX2].mtdNC)->invokeMethodFunc(kctx, SFP(rbp), K_RTNIDX);\
		KLR_RET();\
	} \

#define OPEXEC_VEXEC() {\
		VirtualMachineInstruction *vpc = PC_NEXT(pc);\
		pc = (rbp[K_MTDIDX2].mtdNC)->pc_start;\
		rbp[K_SHIFTIDX2].shift = 0;\
		rbp[K_PCIDX2].pc = vpc;\
		GOTO_PC(pc); \
	}\

/* ------------------------------------------------------------------------- */

#define OPEXEC_iCAST(c, a) {\
	Ri_(c) = (kint_t)Rf_(a); \
}\

#define OPEXEC_fCAST(c, a) {\
	Rf_(c) = (kfloat_t)Ri_(a); \
}\

#define OPEXEC_SCAST(rtnidx, thisidx, rix, espidx, tmr)  { \
		klr_setesp(kctx, SFP(rshift(rbp, espidx)));\
		knh_TypeMap_exec(kctx, tmr, SFP(rshift(rbp,thisidx)), rix); \
	} \

#define OPEXEC_TCAST(kctx, rtnidx, thisidx, rix, espidx, tmr)  { \
		kTypeMap *tmr_ = tmr; \
		KonohaStack *sfp_ = SFP(rshift(rbp,thisidx));\
		KonohaClass scid = SP(tmr_)->scid, this_cid = O_cid(sfp_[0].o);\
		if(this_cid != scid) {\
			tmr_ = knh_findTypeMapNULL(kctx, scid, SP(tmr)->tcid);\
			KSETv(((klr_TCAST_t*)op)->cast, tmr_);\
		}\
		klr_setesp(kctx, SFP(rshift(rbp, espidx)));\
		knh_TypeMap_exec(kctx, tmr_, sfp_, rix); \
	} \

#define OPEXEC_ACAST(rtnidx, thisidx, rix, espidx, tmr)  { \
		kTypeMap *tmr_ = tmr; \
		KonohaClass tcid = SP(tmr_)->tcid, this_cid = O_cid(Ro_(thisidx));\
		if(!class_isa(this_cid, tcid)) {\
			KonohaClass scid = SP(tmr_)->scid;\
			if(this_cid != scid) {\
				tmr_ = knh_findTypeMapNULL(kctx, scid, tcid);\
				KNH_SETv(((klr_ACAST_t*)op)->cast, tmr_);\
			}\
			/*klr_setesp(kctx, SFP(rshift(rbp, espidx)));*/\
			knh_TypeMap_exec(kctx, tmr_, SFP(rshift(rbp,thisidx)), rix); \
		}\
	} \

#define OPEXEC_TR(c, a, rix, ct, f) { \
	f(kctx, SFP(rshift(rbp, a)), (long)rix, ct);\
}\

/* ------------------------------------------------------------------------ */

#define OPEXEC_ONCE(PC, JUMP) { \
	((klr_ONCE_t*)op)->opcode = OPCODE_JMP;\
}\

#define OPEXEC_bNUL(c, a)  Rb_(c) = IS_NULL(Ro_(a))
#define OPEXEC_bNN(c, a)   Rb_(c) = IS_NOTNULL(Ro_(a))

/* ------------------------------------------------------------------------- */

#define OPEXEC_NEXT(PC, JUMP, rtnidx, ib, rix, espidx) { \
	KonohaStack *itrsfp_ = SFP(rshift(rbp, ib)); \
	DBG_ASSERT(IS_bIterator(itrsfp_[0].it));\
	klr_setesp(kctx, SFP(rshift(rbp, espidx)));\
	if(!((itrsfp_[0].it)->fnext_1(kctx, itrsfp_, rix))) { \
		OPEXEC_JMP(PC, JUMP); \
	} \
} \

/* ------------------------------------------------------------------------- */

//#define NPC  /* for KNH_TRY */

#ifdef K_USING_SETJMP_

#define OPEXEC_TRY(PC, JUMP, hn)  {\
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
		klr_setesp(ctx, (ctx->stack + _hdr->espidx));\
		op = _hdrEX->op;\
		((KonohaContextVar*)ctx)->ehdrNC = _hdr->parentNC;\
		OPEXEC_JMP(PC, JUMP);\
	}\
} \

#define OPEXEC_TRYEND(ctx, hn)  {\
	kExceptionHandler* _hdr = Rh_(hn); \
	DBG_ASSERT(IS_ExceptionHandler(_hdr)); \
	((KonohaContextVar*)ctx)->ehdrNC = _hdr->parentNC;\
	klr_mov(ctx, Ro_(hn), KNH_TNULL(ExceptionHandler));\
} \

#else

#define OPEXEC_TRY(PC, JUMP, hn)  {\
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
		klr_setesp(ctx, (ctx->stack + _hdr->espidx));\
		op = _hdrEX->op;\
		((KonohaContextVar*)ctx)->ehdrNC = _hdr->parentNC;\
		OPEXEC_JMP(PC, JUMP);\
	}\
} \

#define OPEXEC_TRYEND(ctx, hn)  {\
	kExceptionHandler* _hdr = Rh_(hn); \
	DBG_ASSERT(IS_ExceptionHandler(_hdr)); \
	DP(_hdr)->return_address = NULL;\
	DP(_hdr)->frame_address  = NULL;\
	((KonohaContextVar*)ctx)->ehdrNC = _hdr->parentNC;\
	klr_mov(ctx, Ro_(hn), KNH_TNULL(ExceptionHandler));\
} \

#endif

#define OPEXEC_THROW(ctx, start) { \
	knh_throw(ctx, SFP(rbp), SFPIDX(start)); \
} \

#define OPEXEC_ASSERT(ctx, start, uline) { \
	knh_assert(ctx, SFP(rbp), SFPIDX(start), uline); \
} \

#define OPEXEC_ERR(ctx, start, msg) { \
	kException *e_ = new_Error(ctx, 0, msg);\
	KonohaContext_setThrowingException(ctx, e_);\
	knh_throw(ctx, SFP(rbp), SFPIDX(start)); \
} \

#define OPEXEC_CATCH0(PC, JUMP, en, emsg)

#define OPEXEC_CATCH(PC, JUMP, en, emsg) { \
		if(!isCATCH(ctx, rbp, en, emsg)) { \
			OPEXEC_JMP(PC, JUMP); \
		} \
	} \

#define OPEXEC_CHKIN(ctx, on, fcheckin)  {\
		kObject *o_ = Ro_(on);\
		fcheckin(ctx, SFP(rbp), RAWPTR(o_));\
		Context_push(ctx, o_);\
	}\

#define OPEXEC_CHKOUT(ctx, on, fcheckout)  {\
		kObject *o_ = Context_pop(ctx);\
		DBG_ASSERT(o_ == Ro_(on));\
		fcheckout(ctx, RAWPTR(o_), 0);\
	}\

/* ------------------------------------------------------------------------ */

#define OPEXEC_P(ctx, fprint, flag, msg, n) fprint(ctx, SFP(rbp), op)

#define OPEXEC_PROBE(ctx, sfpidx, fprobe, n, ns) { \
	fprobe(ctx, SFP(rbp), op);\
}\

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
#define OPEXEC_iDIV2(ctx, c, a, b)  { \
		SYSLOG_iZERODIV(ctx, sfp, Ri_(b)); \
		Ri_(c) = (Ri_(a) / Ri_(b)); \
	} \

#define OPEXEC_iDIVC(c, a, n)  Ri_(c) = (Ri_(a) / n)
#define OPEXEC_iMOD(c, a, b)  Ri_(c) = (Ri_(a) % Ri_(b))
#define OPEXEC_iMOD2(c, a, b)  { \
		SYSLOG_iZERODIV(ctx, sfp, Ri_(b)); \
		Ri_(c) = (Ri_(a) % Ri_(b)); \
	} \

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
#define OPEXEC_fDIV2(ctx, c, a, b)  { \
		SYSLOG_fZERODIV2(ctx, sfp, Rf_(b)); \
		Rf_(c) = (Rf_(a) / Rf_(b)); \
	} \

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

#define OPEXEC_CHKIDX(aidx, nidx) {\
		size_t size_ = kArray_size(rbp[aidx].a);\
		size_t n_ = Ri_(nidx);\
		if(unlikely(n_ >= size_)) THROW_OutOfRange(kctx, SFP(rbp), n_, size_);\
	}\

#define OPEXEC_CHKIDXC(aidx, n) {\
		size_t size_ = kArray_size(rbp[aidx].a);\
		if(unlikely(n >= size_)) THROW_OutOfRange(kctx, SFP(rbp), n, size_);\
	}\

#define OPEXEC_BGETIDXC(cidx, aidx, N) {\
		kbytes_t *b_ = &BA_tobytes(rbp[aidx].ba);\
		size_t n_ = klr_array_index(N, b_->len);\
		klr_array_check(n_, b_->len);\
		Ri_(cidx) = b_->utext[n_];\
	}\

#define OPEXEC_BGETIDX(cidx, aidx, nidx) OPEXEC_BGETIDXC(cidx, aidx, Ri_(nidx))

#define OPEXEC_BSETIDXC(cidx, aidx, N, vidx) {\
		kbytes_t *b_ = &BA_tobytes(Rba_(aidx));\
		size_t n_ = klr_array_index(N, b_->len);\
		klr_array_check(n_, b_->len);\
		b_->ubuf[n_] = (kchar_t)Ri_(vidx);\
		Ri_(cidx) = Ri_(vidx);\
	}\

#define OPEXEC_BSETIDX(cidx, aidx, nidx, vidx) OPEXEC_BSETIDXC(cidx, aidx, Ri_(nidx), vidx)

#define OPEXEC_OGETIDXC(cidx, aidx, N) {\
		kArray *a_ = Ra_(aidx);\
		size_t n_ = klr_array_index(N, kArray_size(a_));\
		klr_array_check(n_, kArray_size(a_));\
		kObject *v_ = (a_)->objectItems[n_];\
		klr_mov(Ro_(cidx), v_);\
	}\

#define OPEXEC_OGETIDX(cidx, aidx, nidx) OPEXEC_OGETIDXC(cidx, aidx, Ri_(nidx))

#define OPEXEC_OSETIDXC(cidx, aidx, N, vidx) {\
		kArray *a_ = Ra_(aidx);\
		size_t n_ = klr_array_index(N, kArray_size(a_));\
		klr_array_check(n_, kArray_size(a_));\
		klr_mov((a_)->objectItems[n_], Ro_(vidx));\
		klr_mov(Ro_(cidx), Ro_(vidx));\
	}\

#define OPEXEC_OSETIDX(cidx, aidx, nidx, vidx) OPEXEC_OSETIDXC(cidx, aidx, Ri_(nidx), vidx)

#define OPEXEC_NGETIDXC(cidx, aidx, N) {\
		kArray *a_ = Ra_(aidx);\
		size_t n_ = klr_array_index(N, kArray_size(a_));\
		klr_array_check(n_, kArray_size(a_));\
		Rn_(cidx) = (a_)->unboxItems[n_];\
	}\

#define OPEXEC_NGETIDX(cidx, aidx, nidx) OPEXEC_NGETIDXC(cidx, aidx, Ri_(nidx))

#define OPEXEC_NSETIDXC(cidx, aidx, N, vidx) {\
		kArray *a_ = Ra_(aidx);\
		size_t n_ = klr_array_index(N, kArray_size(a_));\
		klr_array_check(n_, kArray_size(a_));\
		Rn_(cidx) = (a_)->unboxItems[n_] = Rn_(vidx);\
	}\

#define OPEXEC_NSETIDX(cidx, aidx, nidx, vidx) OPEXEC_NSETIDXC(cidx, aidx, Ri_(nidx), vidx)

#endif

#ifdef __cplusplus
}
#endif

#endif /*KONOHA_VM_H_*/
