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

/* ************************************************************************ */

#include "vm.h"

#if defined(K_USING_THCODE_)
#define TADDR   NULL, 0/*counter*/
#else
#define TADDR   0/*counter*/
#endif/*K_USING_THCODE_*/
#define ASMLINE  0

#define OP_T(T) union { kopl_t op; T op_; }

#define NC_(sfpidx)    (((sfpidx) * 2) + 1)
#define OC_(sfpidx)    ((sfpidx) * 2)
#define SFP_(sfpidx)   ((sfpidx) * 2)
#define RIX_(rix)      rix

#define BBSIZE(BB)  ((BB)->op.bytesize / sizeof(kopl_t))
#define BBOP(BB)     (BB)->op.opl
#define GammaBuilderLabel(n)   (kBasicBlock*)(ctxcode->lstacks->list[n])

#define ASM(T, ...) do {\
	klr_##T##_t op_ = {TADDR, OPCODE_##T, ASMLINE, ## __VA_ARGS__};\
	union { kopl_t op; klr_##T##_t op_; } tmp_; tmp_.op_ = op_;\
	BUILD_asm(_ctx, &tmp_.op, sizeof(klr_##T##_t));\
} while (0)

#define ASMop(T, OP, ...) do {\
	klr_##T##_t op_ = {TADDR, OP, ASMLINE, ## __VA_ARGS__};\
	union { kopl_t op; klr_##T##_t op_; } tmp_; tmp_.op_ = op_;\
	BUILD_asm(_ctx, &tmp_.op, sizeof(klr_##T##_t));\
} while (0)

#define ASMbranch(T, lb, ...) do {\
	klr_##T##_t op_ = {TADDR, OPCODE_##T, ASMLINE, NULL, ## __VA_ARGS__};\
	union { kopl_t op; klr_##T##_t op_; } tmp_; tmp_.op_ = op_;\
	ASM_BRANCH_(_ctx, lb, &tmp_.op, sizeof(klr_##T##_t)); \
} while (0)

#define kBasicBlock_add(bb, T, ...) do { \
	klr_##T##_t op_ = {TADDR, OPCODE_##T, ASMLINE, ## __VA_ARGS__};\
	union { kopl_t op; klr_##T##_t op_; } tmp_; tmp_.op_ = op_;\
	BasicBlock_add(_ctx, bb, 0, &tmp_.op, sizeof(klr_##T##_t));\
} while (0)

#ifdef _CLASSICVM
#include "../../module/classicvm/classicvm_gen.h"
#include "../../module/classicvm/classicvm.h"
#else
#include "minivm.h"
#endif

#include <konoha2/konoha2_local.h>

/* ************************************************************************ */

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */
/* kcode */

int verbose_code = 0;  // global variable

static void EXPR_asm(CTX, int a, kExpr *expr, int shift, int espidx);

static struct _kBasicBlock* new_BasicBlockLABEL(CTX)
{
	struct _kBasicBlock *bb = new_W(BasicBlock, 0);
	bb->id = kArray_size(ctxcode->insts);
	kArray_add(ctxcode->insts, bb);
	return bb;
}

static void BasicBlock_add(CTX, struct _kBasicBlock *bb, kushort_t line, kopl_t *op, size_t size)
{
	if(bb->op.bytemax == 0) {
		KARRAY_INIT(&(bb->op), 1 * sizeof(kopl_t));
	}
	else if(bb->op.bytesize == bb->op.bytemax) {
		KARRAY_EXPAND(&(bb->op), 4 * sizeof(kopl_t));
	}
	kopl_t *pc = bb->op.opl + KARRAYSIZE(bb->op.bytesize, kopl);
	memcpy(pc, op, size == 0 ? sizeof(kopl_t) : size);
	pc->line = line;
	bb->op.bytesize += sizeof(kopl_t);
}

static void BUILD_asm(CTX, kopl_t *op, size_t opsize)
{
	assert(op->opcode != OPCODE_JMPF);
	BasicBlock_add(_ctx, ctxcode->WcurbbNC, ctxcode->uline, op, opsize);
}

static int BUILD_asmJMPF(CTX, klr_JMPF_t *op)
{
	struct _kBasicBlock *bb = ctxcode->WcurbbNC;
	DBG_ASSERT(op->opcode == OPCODE_JMPF);
	int swap = 0;
#ifdef _CLASSICVM
	if (CLASSICVM_BUILD_asmJMPF(_ctx, bb, op, &swap)) {
		return swap;
	}
#endif
	BasicBlock_add(_ctx, bb, ctxcode->uline, (kopl_t*)op, 0);
	return swap;
}

/* ------------------------------------------------------------------------ */
/* new_KonohaCode */

static inline kopcode_t BasicBlock_opcode(kBasicBlock *bb)
{
	if(bb->op.bytesize == 0) return OPCODE_NOP;
	return bb->op.opl->opcode;
}

static void BasicBlock_strip0(CTX, struct _kBasicBlock *bb)
{
	L_TAIL:;
	if(BasicBlock_isVisited(bb)) return;
	BasicBlock_setVisited(bb, 1);
	if(bb->jumpNC != NULL) {
		L_JUMP:;
		struct _kBasicBlock *bbJ = (struct _kBasicBlock*)bb->jumpNC;
		if(bbJ->op.bytesize == 0 && bbJ->jumpNC != NULL && bbJ->nextNC == NULL) {
			//DBG_P("DIRECT JMP id=%d JMP to id=%d", bbJ->id, DP(bbJ->jumpNC)->id);
			bbJ->incoming -= 1;
			bb->jumpNC = bbJ->jumpNC;
			bb->WjumpNC->incoming += 1;
			goto L_JUMP;
		}
		if(bbJ->op.bytesize == 0 && bbJ->jumpNC == NULL && bbJ->nextNC != NULL) {
			//DBG_P("DIRECT JMP id=%d NEXT to id=%d", bbJ->id, DP(bbJ->nextNC)->id);
			bbJ->incoming -= 1;
			bb->jumpNC = bbJ->nextNC;
			bb->WjumpNC->incoming += 1;
			goto L_JUMP;
		}
		if(bb->nextNC == NULL) {
			if(bbJ->incoming == 1 ) {
				//DBG_P("REMOVED %d JMP TO %d", bb->id, bbJ->id);
				bb->nextNC = bbJ;
				bb->jumpNC = NULL;
				goto L_NEXT;
			}
		}
		BasicBlock_strip0(_ctx, bbJ);
	}
	if(bb->jumpNC != NULL && bb->nextNC != NULL) {
		bb = bb->WnextNC;
		goto L_TAIL;
	}
	L_NEXT:;
	if(bb->nextNC != NULL) {
		struct _kBasicBlock *bbN = bb->WnextNC;
		if(bbN->op.bytesize == 0 && bbN->nextNC != NULL && bbN->jumpNC == NULL) {
			//DBG_P("DIRECT NEXT id=%d to NEXT id=%d", bbN->id, DP(bbN->nextNC)->id);
			bbN->incoming -= 1;
			bb->nextNC = bbN->nextNC;
			bb->WnextNC->incoming += 1;
			goto L_NEXT;
		}
		if(bbN->op.bytesize == 0 && bbN->nextNC == NULL && bbN->jumpNC != NULL) {
			//DBG_P("DIRECT NEXT id=%d to JUMP id=%d", bbN->id, DP(bbN->jumpNC)->id);
			bbN->incoming -= 1;
			bb->nextNC = NULL;
			bb->jumpNC = bbN->jumpNC;
			bb->WjumpNC->incoming += 1;
			goto L_JUMP;
		}
		bb = bb->WnextNC;
		goto L_TAIL;
	}
}

static void BasicBlock_join(CTX, struct _kBasicBlock *bb, struct _kBasicBlock *bbN)
{
	//DBG_P("join %d(%s) size=%d and %d(%s) size=%d", bb->id, BB(bb), bb->size, bbN->id, BB(bbN), bbN->size);
	bb->nextNC = bbN->nextNC;
	bb->jumpNC = bbN->jumpNC;
	if(bbN->op.bytesize == 0) {
		return;
	}
	if(bb->op.bytesize == 0) {
		DBG_ASSERT(bb->op.bytemax == 0);
		bb->op = bbN->op;
		bbN->op.opl = NULL;
		bbN->op.bytemax = 0;
		bbN->op.bytesize = 0;
		return;
	}
	if(bb->op.bytemax < bb->op.bytesize + bbN->op.bytesize) {
		KARRAY_EXPAND(&bb->op, (bb->op.bytesize + bbN->op.bytesize));
	}
	memcpy(bb->op.bytebuf + bb->op.bytesize, bbN->op.bytebuf, bbN->op.bytesize);
	bb->op.bytesize += bbN->op.bytesize;
	KARRAY_FREE(&bbN->op);
}

static void BasicBlock_strip1(CTX, struct _kBasicBlock *bb)
{
	L_TAIL:;
	if(!BasicBlock_isVisited(bb)) return;
	BasicBlock_setVisited(bb, 0);  // MUST call after strip0
	if(bb->jumpNC != NULL) {
		if(bb->nextNC == NULL) {
			bb = bb->WjumpNC;
			goto L_TAIL;
		}
		else {
			//DBG_P("** branch next=%d, jump%d", bb->nextNC->id, DP(bb->jumpNC)->id);
			BasicBlock_strip1(_ctx, bb->WjumpNC);
			bb = bb->WnextNC;
			goto L_TAIL;
		}
	}
	if(bb->nextNC != NULL) {
		struct _kBasicBlock *bbN = bb->WnextNC;
		if(bbN->incoming == 1 && BasicBlock_opcode(bbN) != OPCODE_RET) {
			BasicBlock_join(_ctx, bb, bbN);
			BasicBlock_setVisited(bb, 1);
			goto L_TAIL;
		}
		bb = bb->WnextNC;
		goto L_TAIL;
	}
}

static size_t BasicBlock_peephole(CTX, kBasicBlock *bb)
{
	size_t i, bbsize = BBSIZE(bb);
#ifdef _CLASSICVM
	CLASSICVM_BasicBlock_peephole(_ctx, bb);
#endif
	for(i = 0; i < BBSIZE(bb); i++) {
		kopl_t *op = BBOP(bb) + i;
		if(op->opcode == OPCODE_NOP) {
			bbsize--;
		}
	}
	if(bbsize < BBSIZE(bb)) {
		kopl_t *opD = BBOP(bb);
		for(i = 0; i < BBSIZE(bb); i++) {
			kopl_t *opS = BBOP(bb) + i;
			if(opS->opcode == OPCODE_NOP) continue;
			if(opD != opS) {
				*opD = *opS;
			}
			opD++;
		}
		((struct _kBasicBlock*)bb)->op.bytesize = bbsize * sizeof(kopl_t);
	}
	return BBSIZE(bb); /*bbsize*/;
}

#define BB_(bb)   (bb != NULL) ? bb->id : -1

static size_t BasicBlock_size(CTX, kBasicBlock *bb, size_t c)
{
	L_TAIL:;
	if(bb == NULL || BasicBlock_isVisited(bb)) return c;
	BasicBlock_setVisited(bb, 1);
	if(bb->nextNC != NULL) {
		if(BasicBlock_isVisited(bb) || BasicBlock_opcode(bb->nextNC) == OPCODE_RET) {
			struct _kBasicBlock *bb2 = (struct _kBasicBlock*)new_BasicBlockLABEL(_ctx);
			bb2->jumpNC = bb->nextNC;
			((struct _kBasicBlock*)bb)->WnextNC = bb2;
		}
	}
	if(bb->jumpNC != NULL && bb->nextNC != NULL) {
		DBG_ASSERT(bb->jumpNC != bb->nextNC);
		c = BasicBlock_size(_ctx, bb->nextNC, c + BasicBlock_peephole(_ctx, bb));
		bb = bb->jumpNC; goto L_TAIL;
	}
	if(bb->jumpNC != NULL) {
		DBG_ASSERT(bb->nextNC == NULL);
		kBasicBlock_add((struct _kBasicBlock*)bb, JMP);
		c = BasicBlock_peephole(_ctx, bb) + c;
		bb = bb->jumpNC;
		goto L_TAIL;
	}
	c = BasicBlock_peephole(_ctx, bb) + c;
	bb = bb->nextNC;
	goto L_TAIL;
}

static kopl_t* BasicBlock_copy(CTX, kopl_t *dst, struct _kBasicBlock *bb, struct _kBasicBlock **prev)
{
	BasicBlock_setVisited(bb, 0);
	DBG_ASSERT(!BasicBlock_isVisited(bb));
//	DBG_P("BB%d: asm nextNC=BB%d, jumpNC=BB%d", BB_(bb), BB_(bb->nextNC), BB_(bb->jumpNC));
	if(bb->code != NULL) {
		return dst;
	}
	if(prev[0] != NULL && prev[0]->nextNC == NULL && prev[0]->jumpNC == bb) {
		dst -= 1;
		//DBG_P("BB%d: REMOVE unnecessary JMP/(?%s)", BB_(bb), T_opcode(dst->opcode));
		DBG_ASSERT(dst->opcode == OPCODE_JMP/* || dst->opcode == OPJMP_*/);
		prev[0]->jumpNC = NULL;
		prev[0]->nextNC = bb;
	}
	bb->code = dst;
	if(BBSIZE(bb) > 0) {
		memcpy(dst, BBOP(bb), sizeof(kopl_t) * BBSIZE(bb));
		if(bb->jumpNC != NULL) {
			bb->opjmp = (dst + (BBSIZE(bb) - 1));
		}
		dst = dst + BBSIZE(bb);
		KARRAY_FREE(&bb->op);
		prev[0] = bb;
	}
	if(bb->nextNC != NULL) {
		//DBG_P("BB%d: NEXT=BB%d", BB_(bb), BB_(bb->nextNC));
		DBG_ASSERT(bb->nextNC->code == NULL);
		dst = BasicBlock_copy(_ctx, dst, bb->WnextNC, prev);
	}
	if(bb->jumpNC != NULL) {
		dst = BasicBlock_copy(_ctx, dst, bb->WjumpNC, prev);
	}
	return dst;
}

static void BasicBlock_setjump(struct _kBasicBlock *bb)
{
	while(bb != NULL) {
		BasicBlock_setVisited(bb, 1);
		if(bb->jumpNC != NULL) {
			struct _kBasicBlock *bbJ = bb->WjumpNC;
			klr_JMP_t *j = (klr_JMP_t*)bb->opjmp;
			j->jumppc = bbJ->code;
			bb->jumpNC = NULL;
			if(!BasicBlock_isVisited(bbJ)) {
				BasicBlock_setVisited(bbJ, 1);
				BasicBlock_setjump(bbJ);
			}
		}
		bb = bb->WnextNC;
	}
}

static kKonohaCode* new_KonohaCode(CTX, kBasicBlock *bb, kBasicBlock *bbRET)
{
	struct _kKonohaCode *kcode = new_W(KonohaCode, NULL);
	struct _kBasicBlock *prev[1] = {};
	W(kBasicBlock, bb);
	W(kBasicBlock, bbRET);
	kcode->fileid = ctxcode->uline; //TODO
	kcode->codesize = BasicBlock_size(_ctx, bb, 0) * sizeof(kopl_t);
	kcode->code = (kopl_t*)KCALLOC(kcode->codesize, 1);
	WbbRET->code = kcode->code; // dummy
	{
		kopl_t *op = BasicBlock_copy(_ctx, kcode->code, Wbb, prev);
		DBG_ASSERT(op - kcode->code > 0);
		WbbRET->code = NULL;
		BasicBlock_copy(_ctx, op, WbbRET, prev);
		BasicBlock_setjump(Wbb);
	}
	WASSERT(bb); WASSERT(bbRET);
	return kcode;
}

/* ------------------------------------------------------------------------ */

static void dumpOPCODE(CTX, kopl_t *c, kopl_t *pc_start)
{
	size_t i, size = OPDATA[c->opcode].size;
	const kushort_t *vmt = OPDATA[c->opcode].types;
	if(pc_start == NULL) {
		DUMP_P("[%p:%d]\t%s(%d)", c, c->line, T_opcode(c->opcode), (int)c->opcode);
	}
	else {
		DUMP_P("[L%d:%d]\t%s(%d)", (int)(c - pc_start), c->line, T_opcode(c->opcode), (int)c->opcode);
	}
	for(i = 0; i < size; i++) {
		DUMP_P(" ");
		switch(vmt[i]) {
		case VMT_VOID: break;
		case VMT_ADDR:
			if(pc_start == NULL) {
				DUMP_P("%p", c->p[i]);
			}
			else {
				DUMP_P("L%d", (int)((kopl_t*)c->p[i] - pc_start));
			}
			break;
		case VMT_R:
			DUMP_P("sfp[%d,r=%d]", (int)c->data[i]/2, (int)c->data[i]);
			break;
		case VMT_U:
			DUMP_P("u%lu", c->data[i]); break;
		case VMT_I:
		case VMT_INT:
			DUMP_P("i%ld", c->data[i]); break;
		case VMT_F:
			DUMP_P("function(%p)", c->p[i]); break;
		case VMT_CID:
			DUMP_P("CT(%s)", CT_t(c->ct[i])); break;
		case VMT_CO:
			DUMP_P("CT(%s)", CT_t(O_ct(c->o[i]))); break;
		}/*switch*/
	}
	DUMP_P("\n");
}

static KMETHOD Fmethod_runVM(CTX, ksfp_t *sfp _RIX)
{
	DBG_ASSERT(K_RIX == K_RTNIDX);
	DBG_ASSERT(IS_Method(sfp[K_MTDIDX].mtdNC));
	VirtualMachine_run(_ctx, sfp, CODE_ENTER);
}

static void Method_threadCode(CTX, kMethod *mtd, kKonohaCode *kcode)
{
	W(kMethod, mtd);
	kMethod_setFunc(mtd, Fmethod_runVM);
	KSETv(Wmtd->kcode, kcode);
	Wmtd->pc_start = VirtualMachine_run(_ctx, _ctx->esp + 1, kcode->code);
	if(verbose_code) {
		DBG_P("DUMP CODE");
		kopl_t *pc = mtd->pc_start;
		while(1) {
			dumpOPCODE(_ctx, pc, mtd->pc_start);
			if (pc->opcode == OPCODE_RET) {
				break;
			}
			pc++;
		}
	}
	WASSERT(mtd);
}

static void BUILD_compile(CTX, kMethod *mtd, kBasicBlock *bb, kBasicBlock *bbRET)
{
	W(kBasicBlock, bb);
	BasicBlock_strip0(_ctx, Wbb);
	BasicBlock_strip1(_ctx, Wbb);
	kKonohaCode *kcode = new_KonohaCode(_ctx, bb, bbRET);
	Method_threadCode(_ctx, mtd, kcode);
	kArray_clear(ctxcode->insts, 0);
	WASSERT(bb);
}

static void ASM_LABEL(CTX, kBasicBlock *label)
{
	W(kBasicBlock, label);
	if(label != NULL) {
		struct _kBasicBlock *bb = ctxcode->WcurbbNC;
		if(bb != NULL) {
			bb->nextNC = Wlabel;
			bb->jumpNC = NULL;
			Wlabel->incoming += 1;
		}
		ctxcode->WcurbbNC = Wlabel;
	}
	WASSERT(label);
}

static void ASM_JMP(CTX, kBasicBlock *label)
{
	struct _kBasicBlock *bb = ctxcode->WcurbbNC;
	if(bb != NULL) {
		W(kBasicBlock, label);
		bb->nextNC = NULL;
		bb->jumpNC = label;
		Wlabel->incoming += 1;
		WASSERT(label);
	}
	ctxcode->curbbNC = NULL;
}

static kBasicBlock* ASM_JMPF(CTX, int flocal, kBasicBlock *lbJUMP)
{
	struct _kBasicBlock *bb = ctxcode->WcurbbNC;
	struct _kBasicBlock *lbNEXT = new_BasicBlockLABEL(_ctx);
	klr_JMPF_t op = {TADDR, OPCODE_JMPF, ASMLINE, NULL, NC_(flocal)};
	if(BUILD_asmJMPF(_ctx, &op)) {
		bb->jumpNC = lbNEXT;
		bb->nextNC = lbJUMP;
	}
	else {
		bb->jumpNC = lbJUMP;
		bb->nextNC = lbNEXT;
	}
	lbNEXT->incoming += 1;
	ctxcode->WcurbbNC = lbNEXT;
	W(kBasicBlock, lbJUMP);
	WlbJUMP->incoming += 1;
	WASSERT(lbJUMP);
	return lbJUMP;
}

static kBasicBlock* EXPR_asmJMPIF(CTX, int a, kExpr *expr, int isTRUE, kBasicBlock* label, int shift, int espidx)
{
	EXPR_asm(_ctx, a, expr, shift, espidx);
	if(isTRUE) {
		ASM(BNOT, NC_(a), NC_(a));
	}
	return ASM_JMPF(_ctx, a, label);
}

/* ------------------------------------------------------------------------ */
/* CALL */

#define ESP_(sfpidx, args)   SFP_(sfpidx + args + K_CALLDELTA + 1)

static kObject* BUILD_addConstPool(CTX, kObject *o)
{
	kArray_add(ctxcode->constPools, o);
	return o;
}

static void BLOCK_asm(CTX, kBlock *bk, int shift);
static void CALL_asm(CTX, int a, kExpr *expr, int shift, int espidx);
static void AND_asm(CTX, int a, kExpr *expr, int shift, int espidx);
static void OR_asm(CTX, int a, kExpr *expr, int shift, int espidx);
static void LETEXPR_asm(CTX, int a, kExpr *expr, int shift, int espidx);

static void NMOV_asm(CTX, int a, ktype_t ty, int b)
{
	if(TY_isUnbox(ty)) {
		ASM(NMOV, NC_(a), NC_(b), CT_(ty));
	}
	else {
		ASM(NMOV, OC_(a), OC_(b), CT_(ty));
	}
}

static void EXPR_asm(CTX, int a, kExpr *expr, int shift, int espidx)
{
	DBG_ASSERT(expr != NULL);
	//DBG_P("a=%d, shift=%d, espidx=%d", a, shift, espidx);
	switch(expr->build) {
	case TEXPR_CONST : {
		kObject *v = expr->data;
		if(TY_isUnbox(expr->ty)) {
			ASM(NSET, NC_(a), (uintptr_t)N_toint(v), CT_(expr->ty));
		}
		else {
			v = BUILD_addConstPool(_ctx, v);
			ASM(NSET, OC_(a), (uintptr_t)v, CT_(expr->ty));
		}
		break;
	}
	case TEXPR_NEW   : {
		ASM(NEW, OC_(a), expr->index, CT_(expr->ty));
		break;
	}
	case TEXPR_NULL  : {
		if(TY_isUnbox(expr->ty)) {
			ASM(NSET, NC_(a), 0, CT_(expr->ty));
		}
		else {
			ASM(NULL, OC_(a), CT_(expr->ty));
		}
		break;
	}
	case TEXPR_NCONST : {
		ASM(NSET, NC_(a), expr->ndata, CT_(expr->ty));
		break;
	}
	case TEXPR_LOCAL : {
		NMOV_asm(_ctx, a, expr->ty, expr->index);
		break;
	}
	case TEXPR_BLOCK : {
		DBG_ASSERT(IS_Block(expr->block));
		BLOCK_asm(_ctx, expr->block, espidx);
		NMOV_asm(_ctx, a, expr->ty, /*expr->index*/ espidx);
		break;
	}
	case TEXPR_FIELD : {
		kshort_t index = (kshort_t)expr->index;
		kshort_t xindex = (kshort_t)(expr->index >> (sizeof(kshort_t)*8));
		if(TY_isUnbox(expr->ty)) {
			ASM(NMOVx, NC_(a), OC_(index), xindex, CT_(expr->ty));
		}
		else {
			ASM(NMOVx, OC_(a), OC_(index), xindex, CT_(expr->ty));
		}
		break;
	}
	case TEXPR_BOX   : {
		DBG_ASSERT(IS_Expr(expr->single));
		EXPR_asm(_ctx, a, expr->single, shift, espidx);
		ASM(BOX, OC_(a), NC_(a), CT_(expr->single->ty));
		break;
	}
	case TEXPR_UNBOX   : {
		ASM(UNBOX, NC_(a), OC_(a), CT_(expr->ty));
		break;
	}
	case TEXPR_CALL  :
		CALL_asm(_ctx, a, expr, shift, espidx);
		if(a != espidx) {
			NMOV_asm(_ctx, a, expr->ty, espidx);
		}
		break;
	case TEXPR_AND  :
		AND_asm(_ctx, a, expr, shift, espidx);
		break;
	case TEXPR_OR  :
		OR_asm(_ctx, a, expr, shift, espidx);
		break;
	case TEXPR_LET  :
		LETEXPR_asm(_ctx, a, expr, shift, espidx);
		break;
	case TEXPR_STACKTOP  :
		//DBG_P("STACKTOP mov %d, %d, < %d", a, expr->index + shift, espidx);
		DBG_ASSERT(expr->index + shift < espidx);
		NMOV_asm(_ctx, a, expr->ty, expr->index + shift);
		break;
	default:
		DBG_ABORT("unknown expr=%d", expr->build);
	}
}

static KMETHOD Fmethod_abstract(CTX, ksfp_t *sfp _RIX);

static void CALL_asm(CTX, int a, kExpr *expr, int shift, int espidx)
{
	kMethod *mtd = expr->cons->methods[0];
	DBG_ASSERT(IS_Method(mtd));
	int i, s = kMethod_isStatic(mtd) ? 2 : 1, thisidx = espidx + K_CALLDELTA;
#ifdef _CLASSICVM
	if (CLASSICVM_CALL_asm(_ctx, mtd, expr, shift, espidx)) {
		return;
	}
#endif
	for(i = s; i < kArray_size(expr->cons); i++) {
		kExpr *exprN = kExpr_at(expr, i);
		DBG_ASSERT(IS_Expr(exprN));
		EXPR_asm(_ctx, thisidx + i - 1, exprN, shift, thisidx + i - 1);
	}
	int argc = kArray_size(expr->cons) - 2;
//	if (mtd->mn == MN_new && mtd->fcall_1 == Fmethod_abstract) {
//		/* do nothing */
//	} else
	if(kMethod_isVirtual(mtd)) {
		ASM(NSET, NC_(thisidx-1), (intptr_t)mtd, CT_Method);
		ASM(CALL, ctxcode->uline, SFP_(thisidx), ESP_(espidx, argc), knull(CT_(expr->ty)));
	}
	else {
		if(mtd->fcall_1 != Fmethod_runVM) {
			ASM(SCALL, ctxcode->uline, SFP_(thisidx), ESP_(espidx, argc), mtd, knull(CT_(expr->ty)));
		}
		else {
			ASM(VCALL, ctxcode->uline, SFP_(thisidx), ESP_(espidx, argc), mtd, knull(CT_(expr->ty)));
		}
	}
}

static void OR_asm(CTX, int a, kExpr *expr, int shift, int espidx)
{
	int i, size = kArray_size(expr->cons);
	kBasicBlock*  lbTRUE = new_BasicBlockLABEL(_ctx);
	kBasicBlock*  lbFALSE = new_BasicBlockLABEL(_ctx);
	for(i = 1; i < size; i++) {
		EXPR_asmJMPIF(_ctx, a, kExpr_at(expr, i), 1/*TRUE*/, lbTRUE, shift, espidx);
	}
	ASM(NSET, NC_(a), 0/*O_data(K_FALSE)*/, CT_Boolean);
	ASM_JMP(_ctx, lbFALSE);
	ASM_LABEL(_ctx, lbTRUE);
	ASM(NSET, NC_(a), 1/*O_data(K_TRUE)*/, CT_Boolean);
	ASM_LABEL(_ctx, lbFALSE); // false
}

static void AND_asm(CTX, int a, kExpr *expr, int shift, int espidx)
{
	int i, size = kArray_size(expr->cons);
	kBasicBlock*  lbTRUE = new_BasicBlockLABEL(_ctx);
	kBasicBlock*  lbFALSE = new_BasicBlockLABEL(_ctx);
	for(i = 1; i < size; i++) {
		EXPR_asmJMPIF(_ctx, a, kExpr_at(expr, i), 0/*FALSE*/, lbFALSE, shift, espidx);
	}
	ASM(NSET, NC_(a), 1/*O_data(K_TRUE)*/, CT_Boolean);
	ASM_JMP(_ctx, lbTRUE);
	ASM_LABEL(_ctx, lbFALSE); // false
	ASM(NSET, NC_(a), 0/*O_data(K_FALSE)*/, CT_Boolean);
	ASM_LABEL(_ctx, lbTRUE);   // TRUE
}

static void LETEXPR_asm(CTX, int a, kExpr *expr, int shift, int espidx)
{
	kExpr *exprL = kExpr_at(expr, 1);
	kExpr *exprR = kExpr_at(expr, 2);
	if(exprL->build == TEXPR_LOCAL) {
		EXPR_asm(_ctx, exprL->index, exprR, shift, espidx);
		if(a != espidx) {
			NMOV_asm(_ctx, a, exprL->ty, espidx);
		}
	}
	else if(exprL->build == TEXPR_STACKTOP) {
		DBG_P("LET TEXPR_STACKTOP a=%d, exprL->index=%d, espidx=%d", a, exprL->index, espidx);
		EXPR_asm(_ctx, exprL->index + shift, exprR, shift, espidx);
		if(a != espidx) {
			NMOV_asm(_ctx, a, exprL->ty, exprL->index + espidx);
		}
	}
	else{
		assert(exprL->build == TEXPR_FIELD);
		EXPR_asm(_ctx, espidx, exprR, shift, espidx);
		kshort_t index = (kshort_t)exprL->index;
		kshort_t xindex = (kshort_t)(exprL->index >> (sizeof(kshort_t)*8));
		if(TY_isUnbox(exprR->ty)) {
			ASM(XNMOV, OC_(index), xindex, NC_(espidx), CT_(exprL->ty));
		}
		else {
			ASM(XNMOV, OC_(index), xindex, OC_(espidx), CT_(exprL->ty));
		}
		if(a != espidx) {
			NMOV_asm(_ctx, a, exprL->ty, espidx);
		}
	}
}

/* ------------------------------------------------------------------------ */
/* [LABEL]  */

//static void DO_asm(CTX, kStmtExpr *stmt)
//{
//	kBasicBlock* lbCONTINUE = new_BasicBlockLABEL(_ctx);
//	kBasicBlock* lbBREAK = new_BasicBlockLABEL(_ctx);
//	BUILD_pushLABEL(_ctx, stmt, lbCONTINUE, lbBREAK);
//	ASM_LABEL(_ctx, lbCONTINUE);
//	ASM_SAFEPOINT(_ctx, DP(stmt)->espidx);
//	Tn_asmBLOCK(_ctx, stmt, 0);
//	EXPR_asmJMPIF(_ctx, stmt, 1, 0/*FALSE*/, lbBREAK, DP(stmt)->espidx);
//	ASM_JMP(_ctx, lbCONTINUE);
//	ASM_LABEL(_ctx, lbBREAK);
//	BUILD_popLABEL(_ctx);
//}

/* ------------------------------------------------------------------------ */

static void ASM_SAFEPOINT(CTX, int espidx)
{
	kBasicBlock *bb = ctxcode->curbbNC;
	size_t i;
	for(i = 0; i < BBSIZE(bb); i++) {
		kopl_t *op = BBOP(bb) + i;
		if(op->opcode == OPCODE_SAFEPOINT) return;
	}
	ASM(SAFEPOINT, SFP_(espidx));
}

static void ErrStmt_asm(CTX, kStmt *stmt, int shift, int espidx)
{
	kString *msg = (kString*)kObject_getObjectNULL(stmt, KW_ERR);
	DBG_ASSERT(IS_String(msg));
	ASM(ERROR, SFP_(espidx), msg);
}

static void ExprStmt_asm(CTX, kStmt *stmt, int shift, int espidx)
{
	kExpr *expr = (kExpr*)kObject_getObjectNULL(stmt, KW_ExprPattern);
	if(IS_Expr(expr)) {
		EXPR_asm(_ctx, espidx, expr, shift, espidx);
	}
}

static void BlockStmt_asm(CTX, kStmt *stmt, int shift, int espidx)
{
	USING_SUGAR;
	BLOCK_asm(_ctx, kStmt_block(stmt, KW_BlockPattern, K_NULLBLOCK), shift);
}

static void IfStmt_asm(CTX, kStmt *stmt, int shift, int espidx)
{
	USING_SUGAR;
	kBasicBlock*  lbELSE = new_BasicBlockLABEL(_ctx);
	kBasicBlock*  lbEND  = new_BasicBlockLABEL(_ctx);
	/* if */
	lbELSE = EXPR_asmJMPIF(_ctx, espidx, kStmt_expr(stmt, KW_ExprPattern, NULL), 0/*FALSE*/, lbELSE, shift, espidx);
	/* then */
	BLOCK_asm(_ctx, kStmt_block(stmt, KW_BlockPattern, K_NULLBLOCK), shift);
	ASM_JMP(_ctx, lbEND);
	/* else */
	ASM_LABEL(_ctx, lbELSE);
	BLOCK_asm(_ctx, kStmt_block(stmt, KW_else, K_NULLBLOCK), shift);
	/* endif */
	ASM_LABEL(_ctx, lbEND);
}

static void ReturnStmt_asm(CTX, kStmt *stmt, int shift, int espidx)
{
	kExpr *expr = (kExpr*)kObject_getObjectNULL(stmt, KW_ExprPattern);
	if(expr != NULL && IS_Expr(expr) && expr->ty != TY_void) {
		EXPR_asm(_ctx, K_RTNIDX, expr, shift, espidx);
	}
	ASM_JMP(_ctx, ctxcode->lbEND);  // RET
}

static void LoopStmt_asm(CTX, kStmt *stmt, int shift, int espidx)
{
	USING_SUGAR;
	kBasicBlock* lbCONTINUE = new_BasicBlockLABEL(_ctx);
	kBasicBlock* lbBREAK = new_BasicBlockLABEL(_ctx);
//	BUILD_pushLABEL(_ctx, stmt, lbCONTINUE, lbBREAK);
	kObject_setObject(stmt, SYM_("continue"), lbCONTINUE);
	kObject_setObject(stmt, SYM_("break"), lbBREAK);
	ASM_LABEL(_ctx, lbCONTINUE);
	ASM_SAFEPOINT(_ctx, espidx);
	EXPR_asmJMPIF(_ctx, espidx, kStmt_expr(stmt, KW_ExprPattern, NULL), 0/*FALSE*/, lbBREAK, shift, espidx);
	//BLOCK_asm(_ctx, kStmt_block(stmt, KW_("iteration"), K_NULLBLOCK));
	BLOCK_asm(_ctx, kStmt_block(stmt, KW_BlockPattern, K_NULLBLOCK), shift);
	ASM_JMP(_ctx, lbCONTINUE);
	ASM_LABEL(_ctx, lbBREAK);
//	BUILD_popLABEL(_ctx);
}

static void JumpStmt_asm(CTX, kStmt *stmt, int shift, int espidx)
{
	ksyntax_t *syn = stmt->syn;
	kStmt *jump = (kStmt*)kObject_getObject(stmt, syn->kw, NULL);
	DBG_ASSERT(jump != NULL);
	DBG_ASSERT(IS_Stmt(jump));
	kBasicBlock* lbJUMP = (kBasicBlock*)kObject_getObject(jump, syn->kw, NULL);
	DBG_ASSERT(lbJUMP != NULL);
	DBG_ASSERT(IS_BasicBlock(lbJUMP));
	ASM_JMP(_ctx, lbJUMP);
}

static void UndefinedStmt_asm(CTX, kStmt *stmt, int shift, int espidx)
{
	DBG_ABORT("undefined asm syntax kw='%s'", SYM_t(stmt->syn->kw));
}

static void BLOCK_asm(CTX, kBlock *bk, int shift)
{
	int i, espidx = (bk->esp->build == TEXPR_STACKTOP) ? shift + bk->esp->index : bk->esp->index;
	//DBG_P("shift=%d, espidx=%d build=%d", shift, espidx, bk->esp->build);
	for(i = 0; i < kArray_size(bk->blocks); i++) {
		kStmt *stmt = bk->blocks->stmts[i];
		if(stmt->syn == NULL) continue;
		ctxcode->uline = stmt->uline;
		switch(stmt->build) {
		case TSTMT_ERR:    ErrStmt_asm(_ctx, stmt, shift, espidx);   return;
		case TSTMT_EXPR:   ExprStmt_asm(_ctx, stmt, shift, espidx);  break;
		case TSTMT_BLOCK:  BlockStmt_asm(_ctx, stmt, shift, espidx); break;
		case TSTMT_RETURN: ReturnStmt_asm(_ctx, stmt, shift, espidx); return;
		case TSTMT_IF:     IfStmt_asm(_ctx, stmt, shift, espidx);     break;
		case TSTMT_LOOP:   LoopStmt_asm(_ctx, stmt, shift, espidx);     break;
		case TSTMT_JUMP:   JumpStmt_asm(_ctx, stmt, shift, espidx);     break;
		default: UndefinedStmt_asm(_ctx, stmt, shift, espidx); break;
		}
	}
}

/* ------------------------------------------------------------------------ */

static void _THCODE(CTX, kopl_t *pc, void **codeaddr)
{
#ifdef K_USING_THCODE_
	while(1) {
		pc->codeaddr = codeaddr[pc->opcode];
		if(pc->opcode == OPCODE_RET) break;
		pc++;
	}
#endif
}

static void Method_genCode(CTX, kMethod *mtd, kBlock *bk)
{
	DBG_P("START CODE GENERATION..");
	INIT_GCSTACK();
	if(ctxcode == NULL) {
		kmodcode->h.setup(_ctx, NULL, 0);
	}
	kMethod_setFunc(mtd, Fmethod_runVM);
	DBG_ASSERT(kArray_size(ctxcode->insts) == 0);
	kBasicBlock* lbINIT  = new_BasicBlockLABEL(_ctx);
	kBasicBlock* lbBEGIN = new_BasicBlockLABEL(_ctx);
	ctxcode->lbEND = new_BasicBlockLABEL(_ctx);
	PUSH_GCSTACK(lbINIT);
	PUSH_GCSTACK(lbBEGIN);
	PUSH_GCSTACK(ctxcode->lbEND);
	ctxcode->curbbNC = lbINIT;
//	BUILD_pushLABEL(_ctx, NULL, lbBEGIN, lbEND);
	ASM(THCODE, _THCODE);
	ASM_LABEL(_ctx, lbBEGIN);
	BLOCK_asm(_ctx, bk, 0);
	ASM_LABEL(_ctx, ctxcode->lbEND);
	if (mtd->mn == MN_new) {
		ASM(NMOV, OC_(K_RTNIDX), OC_(0), CT_(mtd->cid));   // FIXME: Type 'This' must be resolved
	}
	ASM(RET);
	assert(ctxcode->lbEND);/* scan-build: remove warning */
//	BUILD_popLABEL(_ctx);
	BUILD_compile(_ctx, mtd, lbINIT, ctxcode->lbEND);
	ctxcode->lbEND = NULL;
	RESET_GCSTACK();
}

/* ------------------------------------------------------------------------ */
/* [datatype] */

//#define PACKSUGAR    .packid = 1, .packdom = 1

static void BasicBlock_init(CTX, kObject *o, void *conf)
{
	struct _kBasicBlock *bb = (struct _kBasicBlock*)o;
	bb->op.bytemax = 0;
	bb->op.bytesize = 0;
	bb->code = NULL;
	bb->id = 0;
	bb->incoming = 0;
	bb->nextNC  = NULL;
	bb->jumpNC  = NULL;
	bb->op.opl = NULL;
	bb->opjmp = NULL;
}

static void BasicBlock_free(CTX, kObject *o)
{
	struct _kBasicBlock *bb = (struct _kBasicBlock*)o;
	KARRAY_FREE(&bb->op);
}

static void KonohaCode_init(CTX, kObject *o, void *conf)
{
	struct _kKonohaCode *b = (struct _kKonohaCode*)o;
	b->codesize = 0;
	b->code = NULL;
	b->fileid = 0;
	KINITv(b->source, TS_EMPTY);
}

static void KonohaCode_reftrace(CTX, kObject *o)
{
	kKonohaCode *b = (kKonohaCode*)o;
	BEGIN_REFTRACE(1);
	KREFTRACEv(b->source);
	END_REFTRACE();
}

static void KonohaCode_free(CTX, kObject *o)
{
	kKonohaCode *b = (kKonohaCode*)o;
	KFREE(b->code, b->codesize);
}

static KMETHOD Fmethod_abstract(CTX, ksfp_t *sfp _RIX)
{
//	kMethod *mtd = sfp[K_MTDIDX].mtdNC;
//	ktype_t rtype = mtd->pa->rtype;
//	if (rtype != TY_void) {
//		if (TY_isUnbox(rtype)) {
//			RETURNi_(0);
//		} else {
//			kclass_t *ct = CT_(rtype);
//			kObject *nulval = ct->nulvalNUL;
//			RETURN_(nulval);
//		}
//	}
	RETURNi_(0);
}

//static kbool_t Method_isAbstract(kMethod *mtd)
//{
//	return (mtd->fcall_1 == Fmethod_abstract);
//}

static void Method_setFunc(CTX, kMethod *mtd, knh_Fmethod func)
{
	func = (func == NULL) ? Fmethod_abstract : func;
	((struct _kMethod*)mtd)->fcall_1 = func;
	((struct _kMethod*)mtd)->pc_start = CODE_NCALL;

}

/* ------------------------------------------------------------------------ */
/* [ctxcode] */

static void ctxcode_reftrace(CTX, struct kmodlocal_t *baseh)
{
	ctxcode_t *base = (ctxcode_t*)baseh;
	BEGIN_REFTRACE(2);
	KREFTRACEv(base->insts);
	KREFTRACEv(base->constPools);
	END_REFTRACE();
}
static void ctxcode_free(CTX, struct kmodlocal_t *baseh)
{
	ctxcode_t *base = (ctxcode_t*)baseh;
	KFREE(base, sizeof(ctxcode_t));
}

static void kmodcode_setup(CTX, struct kmodshare_t *def, int newctx)
{
	if(!newctx) { // lazy setup
		assert(_ctx->modlocal[MOD_code] == NULL);
		ctxcode_t *base = (ctxcode_t*)KCALLOC(sizeof(ctxcode_t), 1);
		base->h.reftrace = ctxcode_reftrace;
		base->h.free     = ctxcode_free;
		KINITv(base->insts, new_(Array, K_PAGESIZE/sizeof(void*)));
		KINITv(base->constPools, new_(Array, 64));
		_ctx->modlocal[MOD_code] = (kmodlocal_t*)base;
	}
}

static void kmodcode_reftrace(CTX, struct kmodshare_t *baseh)
{
	kmodcode_t *base = (kmodcode_t*)baseh;
	BEGIN_REFTRACE(1);
	KREFTRACEn(base->codeNull);
	END_REFTRACE();
}

static void kmodcode_free(CTX, struct kmodshare_t *baseh)
{
//	kmodcode_t *base = (kmodcode_t*)baseh;
	KFREE(baseh, sizeof(kmodcode_t));
}

void MODCODE_init(CTX, kcontext_t *ctx)
{
	kmodcode_t *base = (kmodcode_t*)KCALLOC(sizeof(kmodcode_t), 1);
	opcode_check();
	base->h.name     = "minivm";
	base->h.setup    = kmodcode_setup;
	base->h.reftrace = kmodcode_reftrace;
	base->h.free     = kmodcode_free;
	Konoha_setModule(MOD_code, &base->h, 0);

	KDEFINE_CLASS defBasicBlock = {
		STRUCTNAME(BasicBlock),
		.init = BasicBlock_init,
		.free = BasicBlock_free,
	};

	KDEFINE_CLASS defKonohaCode = {
		STRUCTNAME(KonohaCode),
		.init = KonohaCode_init,
		.reftrace = KonohaCode_reftrace,
		.free = KonohaCode_free,
	};

	base->cBasicBlock = Konoha_addClassDef(PN_sugar, PN_sugar, NULL, &defBasicBlock, 0);
	base->cKonohaCode = Konoha_addClassDef(PN_sugar, PN_sugar, NULL, &defKonohaCode, 0);
	kmodcode_setup(_ctx, &base->h, 0/*lazy*/);
	{
		INIT_GCSTACK();
		struct _kBasicBlock* ia = (struct _kBasicBlock*)new_(BasicBlock, 0);
		struct _kBasicBlock* ib = (struct _kBasicBlock*)new_(BasicBlock, 0);
		PUSH_GCSTACK(ia);
		PUSH_GCSTACK(ib);
		kBasicBlock_add(ia, THCODE, _THCODE);
		kBasicBlock_add(ia, NCALL); // FUNCCALL
		kBasicBlock_add(ia, ENTER);
		kBasicBlock_add(ia, EXIT);
		kBasicBlock_add(ib, RET);   // NEED TERMINATION
		ia->WnextNC = ib;
		kKonohaCode *kcode = new_KonohaCode(_ctx, ia, ib);
		KINITv(kmodcode->codeNull, kcode);
		kopl_t *pc = VirtualMachine_run(_ctx, _ctx->esp, kcode->code);
		CODE_ENTER = pc;
		CODE_ENTER = pc+1;
		kArray_clear(ctxcode->insts, 0);
		RESET_GCSTACK();
	}
	struct _klib2 *l = (struct _klib2*)_ctx->lib2;
	l->KMethod_setFunc = Method_setFunc;
	l->KMethod_genCode = Method_genCode;
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
