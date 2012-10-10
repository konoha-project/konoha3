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

#define OP_T(T) union { VirtualMachineInstruction op; T op_; }

#define NC_(sfpidx)    (((sfpidx) * 2) + 1)
#define OC_(sfpidx)    ((sfpidx) * 2)
#define SFP_(sfpidx)   ((sfpidx) * 2)
#define RIX_(rix)      rix

#define BasicBlock_codesize(BB)  ((BB)->codeTable.bytesize / sizeof(VirtualMachineInstruction))
#define BBOP(BB)     (BB)->codeTable.codeItems
#define GammaBuilderLabel(n)   (kBasicBlock*)(ctxcode->lstacks->objectItems[n])

#define ASM(T, ...) do {												\
	OP##T op_ = {TADDR, OPCODE_##T, ASMLINE, ## __VA_ARGS__};		\
	union { VirtualMachineInstruction op; OP##T op_; } tmp_; tmp_.op_ = op_; \
	BUILD_asm(kctx, &tmp_.op, sizeof(OP##T));						\
} while (0)

#define ASMop(T, OP, ...) do {											\
	OP##T op_ = {TADDR, OP, ASMLINE, ## __VA_ARGS__};				\
	union { VirtualMachineInstruction op; OP##T op_; } tmp_; tmp_.op_ = op_; \
	BUILD_asm(kctx, &tmp_.op, sizeof(OP##T));						\
} while (0)

#define ASMbranch(T, lb, ...) do {										\
	OP##T op_ = {TADDR, OPCODE_##T, ASMLINE, NULL, ## __VA_ARGS__};	\
	union { VirtualMachineInstruction op; OP##T op_; } tmp_; tmp_.op_ = op_; \
	ASM_BRANCH_(kctx, lb, &tmp_.op, sizeof(OP##T));					\
} while (0)

#define kBasicBlock_add(bb, T, ...) do {								\
	OP##T op_ = {TADDR, OPCODE_##T, ASMLINE, ## __VA_ARGS__};		\
	union { VirtualMachineInstruction op; OP##T op_; } tmp_; tmp_.op_ = op_; \
	BasicBlock_add(kctx, bb, 0, &tmp_.op, sizeof(OP##T));			\
} while (0)

#ifdef _CLASSICVM
#include "../../module/classicvm/classicvm_gen.h"
#include "../../module/classicvm/classicvm.h"
#else
#include "minivm.h"
#endif

#include <minikonoha/local.h>

/* ************************************************************************ */

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */
/* kcode */

int verbose_code = 0;  // global variable

static void EXPR_asm(KonohaContext *kctx, kStmt *stmt, int a, kExpr *expr, int shift, int espidx);

static kBasicBlock* new_BasicBlockLABEL(KonohaContext *kctx)
{
	kBasicBlock *bb = new_(BasicBlock, 0);
	bb->id = kArray_size(ctxcode->codeList);
	KLIB kArray_add(kctx, ctxcode->codeList, bb);
	return bb;
}

static void BasicBlock_add(KonohaContext *kctx, kBasicBlock *bb, kushort_t line, VirtualMachineInstruction *op, size_t size)
{
	if(bb->codeTable.bytemax == 0) {
		KLIB Karray_init(kctx, &(bb->codeTable), 1 * sizeof(VirtualMachineInstruction));
	}
	else if(bb->codeTable.bytesize == bb->codeTable.bytemax) {
		KLIB Karray_expand(kctx, &(bb->codeTable), 4 * sizeof(VirtualMachineInstruction));
	}
	VirtualMachineInstruction *tailcode = bb->codeTable.codeItems + (bb->codeTable.bytesize/sizeof(VirtualMachineInstruction));
	memcpy(tailcode, op, size == 0 ? sizeof(VirtualMachineInstruction) : size);
	tailcode->line = line;
	bb->codeTable.bytesize += sizeof(VirtualMachineInstruction);
}

static void BUILD_asm(KonohaContext *kctx, VirtualMachineInstruction *op, size_t opsize)
{
	assert(op->opcode != OPCODE_JMPF);
	BasicBlock_add(kctx, ctxcode->currentWorkingBlock, ctxcode->uline, op, opsize);
}

static int BUILD_asmJMPF(KonohaContext *kctx, OPJMPF *op)
{
	kBasicBlock *bb = ctxcode->currentWorkingBlock;
	DBG_ASSERT(op->opcode == OPCODE_JMPF);
	int swap = 0;
#ifdef _CLASSICVM
	if (CLASSICVM_BUILD_asmJMPF(kctx, bb, op, &swap)) {
		return swap;
	}
#endif
	BasicBlock_add(kctx, bb, ctxcode->uline, (VirtualMachineInstruction*)op, 0);
	return swap;
}

/* ------------------------------------------------------------------------ */
/* new_ByteCode */

static inline kopcode_t BasicBlock_opcode(kBasicBlock *bb)
{
	if(bb->codeTable.bytesize == 0) return OPCODE_NOP;
	return bb->codeTable.codeItems->opcode;
}

static void BasicBlock_strip0(KonohaContext *kctx, kBasicBlock *bb)
{
	while(true) {
		if(BasicBlock_isVisited(bb)) return;
		BasicBlock_setVisited(bb, 1);
		if(bb->branchBlock != NULL) {
			L_JUMP:; {
				kBasicBlock *bbJ = (kBasicBlock*)bb->branchBlock;
				if(bbJ->codeTable.bytesize == 0 && bbJ->branchBlock != NULL && bbJ->nextBlock == NULL) {
					//DBG_P("DIRECT JMP id=%d JMP to id=%d", bbJ->id, DP(bbJ->branchBlock)->id);
					bbJ->incoming -= 1;
					bb->branchBlock = bbJ->branchBlock;
					bb->branchBlock->incoming += 1;
					goto L_JUMP;
				}
				if(bbJ->codeTable.bytesize == 0 && bbJ->branchBlock == NULL && bbJ->nextBlock != NULL) {
					//DBG_P("DIRECT JMP id=%d NEXT to id=%d", bbJ->id, DP(bbJ->nextBlock)->id);
					bbJ->incoming -= 1;
					bb->branchBlock = bbJ->nextBlock;
					bb->branchBlock->incoming += 1;
					goto L_JUMP;
				}
				if(bb->nextBlock == NULL) {
					if(bbJ->incoming == 1 ) {
						//DBG_P("REMOVED %d JMP TO %d", bb->id, bbJ->id);
						bb->nextBlock = bbJ;
						bb->branchBlock = NULL;
						goto L_NEXT;
					}
				}
				BasicBlock_strip0(kctx, bbJ);
			}
		}
		if(bb->branchBlock != NULL && bb->nextBlock != NULL) {
			bb = bb->nextBlock;
			continue;
		}
		L_NEXT:;
		if(bb->nextBlock != NULL) {
			kBasicBlock *bbN = bb->nextBlock;
			if(bbN->codeTable.bytesize == 0 && bbN->nextBlock != NULL && bbN->branchBlock == NULL) {
				bbN->incoming -= 1;
				bb->nextBlock = bbN->nextBlock;
				bb->nextBlock->incoming += 1;
				goto L_NEXT;
			}
			if(bbN->codeTable.bytesize == 0 && bbN->nextBlock == NULL && bbN->branchBlock != NULL) {
				bbN->incoming -= 1;
				bb->nextBlock = NULL;
				bb->branchBlock = bbN->branchBlock;
				bb->branchBlock->incoming += 1;
				goto L_JUMP;
			}
			bb = bb->nextBlock;
		}
	}
}

static void BasicBlock_join(KonohaContext *kctx, kBasicBlock *bb, kBasicBlock *bbN)
{
	//DBG_P("join %d(%s) size=%d and %d(%s) size=%d", bb->id, BB(bb), bb->size, bbN->id, BB(bbN), bbN->size);
	bb->nextBlock = bbN->nextBlock;
	bb->branchBlock = bbN->branchBlock;
	if(bbN->codeTable.bytesize == 0) {
		return;
	}
	if(bb->codeTable.bytesize == 0) {
		DBG_ASSERT(bb->codeTable.bytemax == 0);
		bb->codeTable = bbN->codeTable;
		bbN->codeTable.codeItems = NULL;
		bbN->codeTable.bytemax = 0;
		bbN->codeTable.bytesize = 0;
		return;
	}
	if(bb->codeTable.bytemax < bb->codeTable.bytesize + bbN->codeTable.bytesize) {
		KLIB Karray_expand(kctx, &bb->codeTable, (bb->codeTable.bytesize + bbN->codeTable.bytesize));
	}
	memcpy(bb->codeTable.bytebuf + bb->codeTable.bytesize, bbN->codeTable.bytebuf, bbN->codeTable.bytesize);
	bb->codeTable.bytesize += bbN->codeTable.bytesize;
	KLIB Karray_free(kctx, &bbN->codeTable);
}

static void BasicBlock_strip1(KonohaContext *kctx, kBasicBlock *bb)
{
	while(true) {
		if(!BasicBlock_isVisited(bb)) return;
		BasicBlock_setVisited(bb, 0);  // MUST call after strip0
		if(bb->branchBlock != NULL) {
			if(bb->nextBlock == NULL) {
				bb = bb->branchBlock;
			}
			else {
				BasicBlock_strip1(kctx, bb->branchBlock);
				bb = bb->nextBlock;
			}
			continue;
		}
		if(bb->nextBlock != NULL) {
			kBasicBlock *bbN = bb->nextBlock;
			if(bbN->incoming == 1 && BasicBlock_opcode(bbN) != OPCODE_RET) {
				BasicBlock_join(kctx, bb, bbN);
				BasicBlock_setVisited(bb, 1);
				continue;
			}
			bb = bb->nextBlock;
		}
	}
}

static size_t BasicBlock_peephole(KonohaContext *kctx, kBasicBlock *bb)
{
	size_t i, bbsize = BasicBlock_codesize(bb);
#ifdef _CLASSICVM
	CLASSICVM_BasicBlock_peephole(kctx, bb);
#endif
	for(i = 0; i < BasicBlock_codesize(bb); i++) {
		VirtualMachineInstruction *op = BBOP(bb) + i;
		if(op->opcode == OPCODE_NOP) {
			bbsize--;
		}
	}
	if(bbsize < BasicBlock_codesize(bb)) {
		VirtualMachineInstruction *opD = BBOP(bb);
		for(i = 0; i < BasicBlock_codesize(bb); i++) {
			VirtualMachineInstruction *opS = BBOP(bb) + i;
			if(opS->opcode == OPCODE_NOP) continue;
			if(opD != opS) {
				*opD = *opS;
			}
			opD++;
		}
		((kBasicBlock*)bb)->codeTable.bytesize = bbsize * sizeof(VirtualMachineInstruction);
	}
	return BasicBlock_codesize(bb); /*bbsize*/;
}

#define BB_(bb)   (bb != NULL) ? bb->id : -1

static size_t BasicBlock_size(KonohaContext *kctx, kBasicBlock *bb, size_t c)
{
	L_TAIL:;
	if(bb == NULL || BasicBlock_isVisited(bb)) return c;
	BasicBlock_setVisited(bb, 1);
	if(bb->nextBlock != NULL) {
		if(BasicBlock_isVisited(bb) || BasicBlock_opcode(bb->nextBlock) == OPCODE_RET) {
			kBasicBlock *bb2 = (kBasicBlock*)new_BasicBlockLABEL(kctx);
			bb2->branchBlock = bb->nextBlock;
			((kBasicBlock*)bb)->nextBlock = bb2;
		}
	}
	if(bb->branchBlock != NULL && bb->nextBlock != NULL) {
		DBG_ASSERT(bb->branchBlock != bb->nextBlock);
		c = BasicBlock_size(kctx, bb->nextBlock, c + BasicBlock_peephole(kctx, bb));
		bb = bb->branchBlock; goto L_TAIL;
	}
	if(bb->branchBlock != NULL) {
		DBG_ASSERT(bb->nextBlock == NULL);
		kBasicBlock_add(bb, JMP);
		c = BasicBlock_peephole(kctx, bb) + c;
		bb = bb->branchBlock;
		goto L_TAIL;
	}
	c = BasicBlock_peephole(kctx, bb) + c;
	bb = bb->nextBlock;
	goto L_TAIL;
}

static VirtualMachineInstruction* BasicBlock_copy(KonohaContext *kctx, VirtualMachineInstruction *dst, kBasicBlock *bb, kBasicBlock **prev)
{
	BasicBlock_setVisited(bb, 0);
	DBG_ASSERT(!BasicBlock_isVisited(bb));
//	DBG_P("BB%d: asm nextBlock=BB%d, branchBlock=BB%d", BB_(bb), BB_(bb->nextBlock), BB_(bb->branchBlock));
	if(bb->code != NULL) {
		return dst;
	}
	if(prev[0] != NULL && prev[0]->nextBlock == NULL && prev[0]->branchBlock == bb) {
		dst -= 1;
		//DBG_P("BB%d: REMOVE unnecessary JMP/(?%s)", BB_(bb), T_opcode(dst->opcode));
		DBG_ASSERT(dst->opcode == OPCODE_JMP/* || dst->opcode == OPJMP_*/);
		prev[0]->branchBlock = NULL;
		prev[0]->nextBlock = bb;
	}
	bb->code = dst;
	if(BasicBlock_codesize(bb) > 0) {
		memcpy(dst, BBOP(bb), sizeof(VirtualMachineInstruction) * BasicBlock_codesize(bb));
		if(bb->branchBlock != NULL) {
			bb->opjmp = (dst + (BasicBlock_codesize(bb) - 1));
		}
		dst = dst + BasicBlock_codesize(bb);
		KLIB Karray_free(kctx, &bb->codeTable);
		prev[0] = bb;
	}
	if(bb->nextBlock != NULL) {
		//DBG_P("BB%d: NEXT=BB%d", BB_(bb), BB_(bb->nextBlock));
		DBG_ASSERT(bb->nextBlock->code == NULL);
		dst = BasicBlock_copy(kctx, dst, bb->nextBlock, prev);
	}
	if(bb->branchBlock != NULL) {
		dst = BasicBlock_copy(kctx, dst, bb->branchBlock, prev);
	}
	return dst;
}

static void BasicBlock_setjump(kBasicBlock *bb)
{
	while(bb != NULL) {
		BasicBlock_setVisited(bb, 1);
		if(bb->branchBlock != NULL) {
			kBasicBlock *bbJ = bb->branchBlock;
			OPJMP *j = (OPJMP*)bb->opjmp;
			j->jumppc = bbJ->code;
			bb->branchBlock = NULL;
			if(!BasicBlock_isVisited(bbJ)) {
				BasicBlock_setVisited(bbJ, 1);
				BasicBlock_setjump(bbJ);
			}
		}
		bb = bb->nextBlock;
	}
}

static kByteCode* new_ByteCode(KonohaContext *kctx, kBasicBlock *beginBlock, kBasicBlock *endBlock)
{
#define CT_ByteCodeVar CT_ByteCode
	kByteCodeVar *kcode = GCSAFE_new(ByteCodeVar, NULL);
	kBasicBlock *prev[1] = {};
	kcode->fileid = ctxcode->uline; //TODO
	kcode->codesize = BasicBlock_size(kctx, beginBlock, 0) * sizeof(VirtualMachineInstruction);
	kcode->code = (VirtualMachineInstruction*)KCALLOC(kcode->codesize, 1);
	endBlock->code = kcode->code; // dummy
	{
		VirtualMachineInstruction *op = BasicBlock_copy(kctx, kcode->code, beginBlock, prev);
		DBG_ASSERT(op - kcode->code > 0);
		endBlock->code = NULL;
		BasicBlock_copy(kctx, op, endBlock, prev);
		BasicBlock_setjump(beginBlock);
	}
	return kcode;
}

/* ------------------------------------------------------------------------ */

static void dumpOPCODE(KonohaContext *kctx, VirtualMachineInstruction *c, VirtualMachineInstruction *pc_start)
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
				DUMP_P("L%d", (int)((VirtualMachineInstruction*)c->p[i] - pc_start));
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

static KMETHOD MethodFunc_runVirtualMachine(KonohaContext *kctx, KonohaStack *sfp)
{
	DBG_ASSERT(IS_Method(sfp[K_MTDIDX].mtdNC));
	KonohaVirtualMachine_run(kctx, sfp, CODE_ENTER);
}

static void Method_threadCode(KonohaContext *kctx, kMethod *mtd, kByteCode *kcode)
{
	kMethodVar *Wmtd = (kMethodVar*)mtd;
	KLIB kMethod_setFunc(kctx, mtd, MethodFunc_runVirtualMachine);
	KSETv(Wmtd, Wmtd->kcode, kcode);
	Wmtd->pc_start = KonohaVirtualMachine_run(kctx, kctx->esp + 1, kcode->code);
	if(verbose_code) {
		DBG_P("DUMP CODE");
		VirtualMachineInstruction *pc = mtd->pc_start;
		while(1) {
			dumpOPCODE(kctx, pc, mtd->pc_start);
			if (pc->opcode == OPCODE_RET) {
				break;
			}
			pc++;
		}
	}
}

static void BUILD_compile(KonohaContext *kctx, kMethod *mtd, kBasicBlock *beginBlock, kBasicBlock *endBlock)
{
	BasicBlock_strip0(kctx, beginBlock);
	BasicBlock_strip1(kctx, beginBlock);
	kByteCode *kcode = new_ByteCode(kctx, beginBlock, endBlock);
	Method_threadCode(kctx, mtd, kcode);
	KLIB kArray_clear(kctx, ctxcode->codeList, 0);
}

static void ASM_LABEL(KonohaContext *kctx, kBasicBlock *labelBlock)
{
	if(labelBlock != NULL) {
		kBasicBlock *bb = ctxcode->currentWorkingBlock;
		if(bb != NULL) {
			bb->nextBlock = labelBlock;
			bb->branchBlock = NULL;
			labelBlock->incoming += 1;
		}
		ctxcode->currentWorkingBlock = labelBlock;
	}
}

static void ASM_JMP(KonohaContext *kctx, kBasicBlock *labelBlock)
{
	kBasicBlock *bb = ctxcode->currentWorkingBlock;
	if(bb != NULL) {
		bb->nextBlock = NULL;
		bb->branchBlock = labelBlock;
		labelBlock->incoming += 1;
	}
	ctxcode->currentWorkingBlock = NULL;
}

static kBasicBlock* ASM_JMPF(KonohaContext *kctx, int flocal, kBasicBlock *lbJUMP)
{
	kBasicBlock *bb = ctxcode->currentWorkingBlock;
	kBasicBlock *lbNEXT = new_BasicBlockLABEL(kctx);
	OPJMPF op = {TADDR, OPCODE_JMPF, ASMLINE, NULL, NC_(flocal)};
	if(BUILD_asmJMPF(kctx, &op)) {
		bb->branchBlock = lbNEXT;
		bb->nextBlock = lbJUMP;
	}
	else {
		bb->branchBlock = lbJUMP;
		bb->nextBlock = lbNEXT;
	}
	lbNEXT->incoming += 1;
	ctxcode->currentWorkingBlock = lbNEXT;
	lbJUMP->incoming += 1;
	return lbJUMP;
}

static kBasicBlock* EXPR_asmJMPIF(KonohaContext *kctx, kStmt *stmt, int a, kExpr *expr, int isTRUE, kBasicBlock* label, int shift, int espidx)
{
	EXPR_asm(kctx, stmt, a, expr, shift, espidx);
	if(isTRUE) {
		ASM(BNOT, NC_(a), NC_(a));
	}
	return ASM_JMPF(kctx, a, label);
}

/* ------------------------------------------------------------------------ */
/* CALL */

#define ESP_(sfpidx, args)   SFP_(sfpidx + args + K_CALLDELTA + 1)

static kObject* BUILD_addConstPool(KonohaContext *kctx, kObject *o)
{
	KLIB kArray_add(kctx, ctxcode->constPools, o);
	return o;
}

static void BLOCK_asm(KonohaContext *kctx, kBlock *bk, int shift);
static void CALL_asm(KonohaContext *kctx, kStmt *stmt, int a, kExpr *expr, int shift, int espidx);
static void AND_asm(KonohaContext *kctx, kStmt *stmt, int a, kExpr *expr, int shift, int espidx);
static void OR_asm(KonohaContext *kctx, kStmt *stmt, int a, kExpr *expr, int shift, int espidx);
static void LETEXPR_asm(KonohaContext *kctx, kStmt *stmt, int a, kExpr *expr, int shift, int espidx);

static void NMOV_asm(KonohaContext *kctx, int a, ktype_t ty, int b)
{
	if(TY_isUnbox(ty)) {
		ASM(NMOV, NC_(a), NC_(b), CT_(ty));
	}
	else {
		ASM(NMOV, OC_(a), OC_(b), CT_(ty));
	}
}

static void EXPR_asm(KonohaContext *kctx, kStmt *stmt, int a, kExpr *expr, int shift, int espidx)
{
	DBG_ASSERT(expr != NULL);
	//DBG_P("a=%d, shift=%d, espidx=%d", a, shift, espidx);
	switch(expr->build) {
	case TEXPR_CONST : {
		kObject *v = expr->objectConstValue;
		DBG_ASSERT(!TY_isUnbox(expr->ty));
		DBG_ASSERT(Expr_hasObjectConstValue(expr));
		v = BUILD_addConstPool(kctx, v);
		ASM(NSET, OC_(a), (uintptr_t)v, CT_(expr->ty));
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
		ASM(NSET, NC_(a), expr->unboxConstValue, CT_(expr->ty));
		break;
	}
	case TEXPR_LOCAL : {
		NMOV_asm(kctx, a, expr->ty, expr->index);
		break;
	}
	case TEXPR_BLOCK : {
		DBG_ASSERT(IS_Block(expr->block));
		BLOCK_asm(kctx, expr->block, espidx);
		NMOV_asm(kctx, a, expr->ty, /*expr->index*/ espidx);
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
	case TEXPR_CALL  :
		CALL_asm(kctx, stmt, a, expr, shift, espidx);
		if(a != espidx) {
			NMOV_asm(kctx, a, expr->ty, espidx);
		}
		break;
	case TEXPR_AND  :
		AND_asm(kctx, stmt, a, expr, shift, espidx);
		break;
	case TEXPR_OR  :
		OR_asm(kctx, stmt, a, expr, shift, espidx);
		break;
	case TEXPR_LET  :
		LETEXPR_asm(kctx, stmt, a, expr, shift, espidx);
		break;
	case TEXPR_STACKTOP  :
		//DBG_P("STACKTOP mov %d, %d, < %d", a, expr->index + shift, espidx);
		DBG_ASSERT(expr->index + shift < espidx);
		NMOV_asm(kctx, a, expr->ty, expr->index + shift);
		break;
	default:
		DBG_ABORT("unknown expr=%d", expr->build);
	}
}

static KMETHOD MethodFunc_invokeAbstractMethod(KonohaContext *kctx, KonohaStack *sfp);

static void CALL_asm(KonohaContext *kctx, kStmt *stmt, int a, kExpr *expr, int shift, int espidx)
{
	kMethod *mtd = expr->cons->methodItems[0];
	DBG_ASSERT(IS_Method(mtd));
	int i, s = kMethod_is(Static, mtd) ? 2 : 1, thisidx = espidx + K_CALLDELTA;
#ifdef _CLASSICVM
	if (CLASSICVM_CALL_asm(kctx, mtd, expr, shift, espidx)) {
		return;
	}
#endif
	for(i = s; i < kArray_size(expr->cons); i++) {
		kExpr *exprN = kExpr_at(expr, i);
		DBG_ASSERT(IS_Expr(exprN));
		EXPR_asm(kctx, stmt, thisidx + i - 1, exprN, shift, thisidx + i - 1);
	}
	int argc = kArray_size(expr->cons) - 2;
//	if (mtd->mn == MN_new && mtd->invokeMethodFunc == MethodFunc_abstract) {
//		/* do nothing */
//	} else
//	if(kMethod_is(Final, mtd) || !kMethod_is(Virtual, mtd)) {
//		if(mtd->invokeMethodFunc != MethodFunc_runVirtualMachine) {
//		ASM(SCALL, ctxcode->uline, SFP_(thisidx), ESP_(espidx, argc), mtd, KLIB Knull(kctx, CT_(expr->ty)));
//		}
//		else {
//			ASM(VCALL, ctxcode->uline, SFP_(thisidx), ESP_(espidx, argc), mtd, KLIB Knull(kctx, CT_(expr->ty)));
//		}
//	}
//	else {
	if(kMethod_is(Final, mtd) || !kMethod_is(Virtual, mtd)) {
		ASM(NSET, NC_(thisidx-1), (intptr_t)mtd, CT_Method);
	}
	else {
		ASM(LOOKUP, SFP_(thisidx), Stmt_nameSpace(stmt), mtd);
	}
	ASM(CALL, ctxcode->uline, SFP_(thisidx), ESP_(espidx, argc), KLIB Knull(kctx, CT_(expr->ty)));
}

static void OR_asm(KonohaContext *kctx, kStmt *stmt, int a, kExpr *expr, int shift, int espidx)
{
	int i, size = kArray_size(expr->cons);
	kBasicBlock*  lbTRUE = new_BasicBlockLABEL(kctx);
	kBasicBlock*  lbFALSE = new_BasicBlockLABEL(kctx);
	for(i = 1; i < size; i++) {
		EXPR_asmJMPIF(kctx, stmt, a, kExpr_at(expr, i), 1/*TRUE*/, lbTRUE, shift, espidx);
	}
	ASM(NSET, NC_(a), 0/*O_data(K_FALSE)*/, CT_Boolean);
	ASM_JMP(kctx, lbFALSE);
	ASM_LABEL(kctx, lbTRUE);
	ASM(NSET, NC_(a), 1/*O_data(K_TRUE)*/, CT_Boolean);
	ASM_LABEL(kctx, lbFALSE); // false
}

static void AND_asm(KonohaContext *kctx, kStmt *stmt, int a, kExpr *expr, int shift, int espidx)
{
	int i, size = kArray_size(expr->cons);
	kBasicBlock*  lbTRUE = new_BasicBlockLABEL(kctx);
	kBasicBlock*  lbFALSE = new_BasicBlockLABEL(kctx);
	for(i = 1; i < size; i++) {
		EXPR_asmJMPIF(kctx, stmt, a, kExpr_at(expr, i), 0/*FALSE*/, lbFALSE, shift, espidx);
	}
	ASM(NSET, NC_(a), 1/*O_data(K_TRUE)*/, CT_Boolean);
	ASM_JMP(kctx, lbTRUE);
	ASM_LABEL(kctx, lbFALSE); // false
	ASM(NSET, NC_(a), 0/*O_data(K_FALSE)*/, CT_Boolean);
	ASM_LABEL(kctx, lbTRUE);   // TRUE
}

static void LETEXPR_asm(KonohaContext *kctx, kStmt *stmt, int a, kExpr *expr, int shift, int espidx)
{
	kExpr *leftHandExpr = kExpr_at(expr, 1);
	kExpr *rightHandExpr = kExpr_at(expr, 2);
	DBG_P("LET (%s) a=%d, shift=%d, espidx=%d", TY_t(expr->ty), a, shift, espidx);
	if(leftHandExpr->build == TEXPR_LOCAL) {
		EXPR_asm(kctx, stmt, leftHandExpr->index, rightHandExpr, shift, espidx);
		if(expr->ty != TY_void && a != leftHandExpr->index) {
			NMOV_asm(kctx, a, leftHandExpr->ty, leftHandExpr->index);
		}
	}
	else if(leftHandExpr->build == TEXPR_STACKTOP) {
		DBG_P("LET TEXPR_STACKTOP a=%d, leftHandExpr->index=%d, espidx=%d", a, leftHandExpr->index, espidx);
		EXPR_asm(kctx, stmt, leftHandExpr->index + shift, rightHandExpr, shift, espidx);
		if(expr->ty != TY_void && a != leftHandExpr->index + shift) {
			NMOV_asm(kctx, a, leftHandExpr->ty, leftHandExpr->index + shift);
		}
	}
	else{
		assert(leftHandExpr->build == TEXPR_FIELD);
		EXPR_asm(kctx, stmt, espidx, rightHandExpr, shift, espidx);
		kshort_t index = (kshort_t)leftHandExpr->index;
		kshort_t xindex = (kshort_t)(leftHandExpr->index >> (sizeof(kshort_t)*8));
		if(TY_isUnbox(rightHandExpr->ty)) {
			ASM(XNMOV, OC_(index), xindex, NC_(espidx), CT_(leftHandExpr->ty));
			if(expr->ty != TY_void) {
				ASM(NMOVx, NC_(a), OC_(index), xindex, CT_(leftHandExpr->ty));
			}
		}
		else {
			ASM(XNMOV, OC_(index), xindex, OC_(espidx), CT_(leftHandExpr->ty));
			if(expr->ty != TY_void) {
				ASM(NMOVx, OC_(a), OC_(index), xindex, CT_(leftHandExpr->ty));
			}
		}
	}
}

/* ------------------------------------------------------------------------ */
/* [LABEL]  */

//static void DO_asm(KonohaContext *kctx, kStmtExpr *stmt)
//{
//	kBasicBlock* lbCONTINUE = new_BasicBlockLABEL(kctx);
//	kBasicBlock* lbBREAK = new_BasicBlockLABEL(kctx);
//	BUILD_pushLABEL(kctx, stmt, lbCONTINUE, lbBREAK);
//	ASM_LABEL(kctx, lbCONTINUE);
//	ASM_SAFEPOINT(kctx, DP(stmt)->espidx);
//	Tn_asmBLOCK(kctx, stmt, 0);
//	EXPR_asmJMPIF(kctx, stmt, 1, 0/*FALSE*/, lbBREAK, DP(stmt)->espidx);
//	ASM_JMP(kctx, lbCONTINUE);
//	ASM_LABEL(kctx, lbBREAK);
//	BUILD_popLABEL(kctx);
//}

/* ------------------------------------------------------------------------ */

static void ASM_SAFEPOINT(KonohaContext *kctx, int espidx)
{
	kBasicBlock *bb = ctxcode->currentWorkingBlock;
	size_t i;
	for(i = 0; i < BasicBlock_codesize(bb); i++) {
		VirtualMachineInstruction *op = BBOP(bb) + i;
		if(op->opcode == OPCODE_SAFEPOINT) return;
	}
	ASM(SAFEPOINT, SFP_(espidx));
}

static void ErrStmt_asm(KonohaContext *kctx, kStmt *stmt, int shift, int espidx)
{
	kString *msg = (kString*)kStmt_getObjectNULL(kctx, stmt, KW_ERR);
	DBG_ASSERT(IS_String(msg));
	ASM(ERROR, stmt->uline, msg);
}

static void ExprStmt_asm(KonohaContext *kctx, kStmt *stmt, int shift, int espidx)
{
	kExpr *expr = (kExpr*)kStmt_getObjectNULL(kctx, stmt, KW_ExprPattern);
	if(IS_Expr(expr)) {
		EXPR_asm(kctx, stmt, espidx, expr, shift, espidx);
	}
}

static void BlockStmt_asm(KonohaContext *kctx, kStmt *stmt, int shift, int espidx)
{
	BLOCK_asm(kctx, SUGAR kStmt_getBlock(kctx, stmt, NULL/*DefaultNameSpace*/, KW_BlockPattern, K_NULLBLOCK), shift);
}

static void IfStmt_asm(KonohaContext *kctx, kStmt *stmt, int shift, int espidx)
{
	kBasicBlock*  lbELSE = new_BasicBlockLABEL(kctx);
	kBasicBlock*  lbEND  = new_BasicBlockLABEL(kctx);
	/* if */
	lbELSE = EXPR_asmJMPIF(kctx, stmt, espidx, SUGAR kStmt_getExpr(kctx, stmt, KW_ExprPattern, NULL), 0/*FALSE*/, lbELSE, shift, espidx);
	/* then */
	BLOCK_asm(kctx, SUGAR kStmt_getBlock(kctx, stmt, NULL/*DefaultNameSpace*/, KW_BlockPattern, K_NULLBLOCK), shift);
	ASM_JMP(kctx, lbEND);
	/* else */
	ASM_LABEL(kctx, lbELSE);
	BLOCK_asm(kctx, SUGAR kStmt_getBlock(kctx, stmt, NULL/*DefaultNameSpace*/, KW_else, K_NULLBLOCK), shift);
	/* endif */
	ASM_LABEL(kctx, lbEND);
}

static void ReturnStmt_asm(KonohaContext *kctx, kStmt *stmt, int shift, int espidx)
{
	kExpr *expr = (kExpr*)kStmt_getObjectNULL(kctx, stmt, KW_ExprPattern);
	if(expr != NULL && IS_Expr(expr) && expr->ty != TY_void) {
		EXPR_asm(kctx, stmt, K_RTNIDX, expr, shift, espidx);
	}
	ASM_JMP(kctx, ctxcode->lbEND);  // RET
}

static void LoopStmt_asm(KonohaContext *kctx, kStmt *stmt, int shift, int espidx)
{
	kBasicBlock* lbCONTINUE = new_BasicBlockLABEL(kctx);
	kBasicBlock* lbBREAK = new_BasicBlockLABEL(kctx);
//	BUILD_pushLABEL(kctx, stmt, lbCONTINUE, lbBREAK);
	KLIB kObject_setObject(kctx, stmt, SYM_("continue"), TY_BasicBlock, lbCONTINUE);
	KLIB kObject_setObject(kctx, stmt, SYM_("break"), TY_BasicBlock, lbBREAK);
	ASM_LABEL(kctx, lbCONTINUE);
	ASM_SAFEPOINT(kctx, espidx);
	EXPR_asmJMPIF(kctx, stmt, espidx, SUGAR kStmt_getExpr(kctx, stmt, KW_ExprPattern, NULL), 0/*FALSE*/, lbBREAK, shift, espidx);
	//BLOCK_asm(kctx, SUGAR kStmt_getBlock(kctx, stmt, KW_("iteration"), K_NULLBLOCK));
	BLOCK_asm(kctx, SUGAR kStmt_getBlock(kctx, stmt, NULL/*DefaultNameSpace*/, KW_BlockPattern, K_NULLBLOCK), shift);
	ASM_JMP(kctx, lbCONTINUE);
	ASM_LABEL(kctx, lbBREAK);
//	BUILD_popLABEL(kctx);
}

static void JumpStmt_asm(KonohaContext *kctx, kStmt *stmt, int shift, int espidx)
{
	SugarSyntax *syn = stmt->syn;
	kStmt *jump = (kStmt*)kStmt_getObject(kctx, stmt, syn->keyword, NULL);
	DBG_ASSERT(jump != NULL);
	DBG_ASSERT(IS_Stmt(jump));
	kBasicBlock* lbJUMP = (kBasicBlock*)KLIB kObject_getObject(kctx, jump, syn->keyword, NULL);
	DBG_ASSERT(lbJUMP != NULL);
	DBG_ASSERT(IS_BasicBlock(lbJUMP));
	ASM_JMP(kctx, lbJUMP);
}

static void ASMBranch(KonohaContext *kctx, kBasicBlock *jumpBB)
{
	kBasicBlock *bb = ctxcode->currentWorkingBlock;
	kBasicBlock *newbb = new_BasicBlockLABEL(kctx);
	OPTRYJMP op_ = {TADDR, OPCODE_TRYJMP, ASMLINE, NULL};
	union { VirtualMachineInstruction op; OPTRYJMP op_; } tmp_; tmp_.op_ = op_;
	BUILD_asm(kctx, &tmp_.op, sizeof(OPTRYJMP));

	bb->branchBlock = jumpBB; jumpBB->incoming += 1;
	bb->nextBlock   = newbb;  newbb->incoming  += 1;
	ctxcode->currentWorkingBlock = newbb;
}

static void TryStmt_asm(KonohaContext *kctx, kStmt *stmt, int shift, int espidx)
{
	kBasicBlock* lbCatched = new_BasicBlockLABEL(kctx);
	kBasicBlock* lbFinally = new_BasicBlockLABEL(kctx);
	kBlock *catchBlock   = SUGAR kStmt_getBlock(kctx, stmt, NULL, SYM_("catch"),   K_NULLBLOCK);
	kBlock *finallyBlock = SUGAR kStmt_getBlock(kctx, stmt, NULL, SYM_("finally"), K_NULLBLOCK);
	PUSH_GCSTACK(catchBlock);
	PUSH_GCSTACK(finallyBlock);
	KLIB kObject_setObject(kctx, stmt, SYM_("catch"),   TY_BasicBlock, lbCatched);
	KLIB kObject_setObject(kctx, stmt, SYM_("finally"), TY_BasicBlock, lbFinally);
	ASMBranch(kctx/*, OPCODE_TRYJMP*/, lbCatched);

	BLOCK_asm(kctx, SUGAR kStmt_getBlock(kctx, stmt, NULL, KW_BlockPattern, K_NULLBLOCK), shift);
	ASM_JMP(kctx, lbFinally);
	ASM_LABEL(kctx, lbCatched);
	BLOCK_asm(kctx, catchBlock, shift);
	ASM_JMP(kctx, lbFinally);
	ASM_LABEL(kctx, lbFinally);
	BLOCK_asm(kctx, finallyBlock, shift);
}

static void UndefinedStmt_asm(KonohaContext *kctx, kStmt *stmt, int shift, int espidx)
{
	DBG_P("undefined asm syntax kw='%s'", SYM_t(stmt->syn->keyword));
}

static void BLOCK_asm(KonohaContext *kctx, kBlock *bk, int shift)
{
	int i, espidx = (bk->esp->build == TEXPR_STACKTOP) ? shift + bk->esp->index : bk->esp->index;
	//DBG_P("shift=%d, espidx=%d build=%d", shift, espidx, bk->esp->build);
	for(i = 0; i < kArray_size(bk->stmtList); i++) {
		kStmt *stmt = bk->stmtList->stmtItems[i];
		if(stmt->syn == NULL) continue;
		ctxcode->uline = stmt->uline;
		switch(stmt->build) {
		case TSTMT_ERR:    ErrStmt_asm(kctx, stmt, shift, espidx);   return;
		case TSTMT_EXPR:   ExprStmt_asm(kctx, stmt, shift, espidx);  break;
		case TSTMT_BLOCK:  BlockStmt_asm(kctx, stmt, shift, espidx); break;
		case TSTMT_RETURN: ReturnStmt_asm(kctx, stmt, shift, espidx); return;
		case TSTMT_IF:     IfStmt_asm(kctx, stmt, shift, espidx);     break;
		case TSTMT_LOOP:   LoopStmt_asm(kctx, stmt, shift, espidx);     break;
		case TSTMT_JUMP:   JumpStmt_asm(kctx, stmt, shift, espidx);     break;
		case TSTMT_TRY:    TryStmt_asm(kctx, stmt, shift, espidx);      break;
		default: UndefinedStmt_asm(kctx, stmt, shift, espidx); break;
		}
	}
}

/* ------------------------------------------------------------------------ */

static void _THCODE(KonohaContext *kctx, VirtualMachineInstruction *pc, void **codeaddr)
{
#ifdef K_USING_THCODE_
	while(1) {
		pc->codeaddr = codeaddr[pc->opcode];
		if(pc->opcode == OPCODE_RET) break;
		pc++;
	}
#endif
}

static void kMethod_genCode(KonohaContext *kctx, kMethod *mtd, kBlock *bk)
{
	DBG_P("START CODE GENERATION..");
	INIT_GCSTACK();
	if(ctxcode == NULL) {
		kmodcode->h.setup(kctx, NULL, 0);
	}
	KLIB kMethod_setFunc(kctx, mtd, MethodFunc_runVirtualMachine);
	DBG_ASSERT(kArray_size(ctxcode->codeList) == 0);
	kBasicBlock* lbINIT  = new_BasicBlockLABEL(kctx);
	kBasicBlock* lbBEGIN = new_BasicBlockLABEL(kctx);
	ctxcode->lbEND = new_BasicBlockLABEL(kctx);
	PUSH_GCSTACK(lbINIT);
	PUSH_GCSTACK(lbBEGIN);
	PUSH_GCSTACK(ctxcode->lbEND);
	ctxcode->currentWorkingBlock = lbINIT;
//	BUILD_pushLABEL(kctx, NULL, lbBEGIN, lbEND);
	ASM(THCODE, _THCODE);
	ASM(CHKSTACK, 0);
	ASM_LABEL(kctx, lbBEGIN);
	BLOCK_asm(kctx, bk, 0);
	ASM_LABEL(kctx, ctxcode->lbEND);
	if (mtd->mn == MN_new) {
		ASM(NMOV, OC_(K_RTNIDX), OC_(0), CT_(mtd->typeId));   // FIXME: Type 'This' must be resolved
	}
	ASM(RET);
	assert(ctxcode->lbEND);/* scan-build: remove warning */
//	BUILD_popLABEL(kctx);
	BUILD_compile(kctx, mtd, lbINIT, ctxcode->lbEND);
	ctxcode->lbEND = NULL;
	RESET_GCSTACK();
}

/* ------------------------------------------------------------------------ */
/* [datatype] */

//#define PACKSUGAR    .packageId = 1, .packageDomain = 1

static void BasicBlock_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kBasicBlock *bb = (kBasicBlock*)o;
	bb->codeTable.bytemax = 0;
	bb->codeTable.bytesize = 0;
	bb->code = NULL;
	bb->id = 0;
	bb->incoming = 0;
	bb->nextBlock  = NULL;
	bb->branchBlock  = NULL;
	bb->codeTable.codeItems = NULL;
	bb->opjmp = NULL;
}

static void BasicBlock_free(KonohaContext *kctx, kObject *o)
{
	kBasicBlock *bb = (kBasicBlock*)o;
	KLIB Karray_free(kctx, &bb->codeTable);
}

static void ByteCode_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kByteCodeVar *b = (kByteCodeVar*)o;
	b->codesize = 0;
	b->code = NULL;
	b->fileid = 0;
	KINITv(b->source, TS_EMPTY);
}

static void ByteCode_reftrace(KonohaContext *kctx, kObject *o)
{
	kByteCode *b = (kByteCode*)o;
	BEGIN_REFTRACE(1);
	KREFTRACEv(b->source);
	END_REFTRACE();
}

static void ByteCode_free(KonohaContext *kctx, kObject *o)
{
	kByteCode *b = (kByteCode*)o;
	KFREE(b->code, b->codesize);
}

static KMETHOD MethodFunc_invokeAbstractMethod(KonohaContext *kctx, KonohaStack *sfp)
{
//	kMethod *mtd = sfp[K_MTDIDX].mtdNC;
//	ktype_t rtype = mtd->pa->rtype;
//	if (rtype != TY_void) {
//		if (TY_isUnbox(rtype)) {
//			RETURNi_(0);
//		} else {
//			KonohaClass *ct = CT_(rtype);
//			kObject *nulval = ct->defaultValueAsNull;
//			RETURN_(nulval);
//		}
//	}
	RETURNi_(0);
}

//static kbool_t Method_isAbstract(kMethod *mtd)
//{
//	return (mtd->invokeMethodFunc == MethodFunc_abstract);
//}

static void kMethod_setFunc(KonohaContext *kctx, kMethod *mtd, MethodFunc func)
{
	func = (func == NULL) ? MethodFunc_invokeAbstractMethod : func;
	((kMethodVar*)mtd)->invokeMethodFunc = func;
	((kMethodVar*)mtd)->pc_start = CODE_NCALL;
}

/* ------------------------------------------------------------------------ */
/* [ctxcode] */

static void ctxcode_reftrace(KonohaContext *kctx, struct KonohaModuleContext *baseh)
{
	ctxcode_t *base = (ctxcode_t*)baseh;
	BEGIN_REFTRACE(2);
	KREFTRACEv(base->codeList);
	KREFTRACEv(base->constPools);
	END_REFTRACE();
}
static void ctxcode_free(KonohaContext *kctx, struct KonohaModuleContext *baseh)
{
	ctxcode_t *base = (ctxcode_t*)baseh;
	KFREE(base, sizeof(ctxcode_t));
}

static void kmodcode_setup(KonohaContext *kctx, struct KonohaModule *def, int newctx)
{
	if(!newctx) { // lazy setup
		assert(kctx->modlocal[MOD_code] == NULL);
		ctxcode_t *base = (ctxcode_t*)KCALLOC(sizeof(ctxcode_t), 1);
		base->h.reftrace = ctxcode_reftrace;
		base->h.free     = ctxcode_free;
		KINITv(base->codeList, new_(Array, K_PAGESIZE/sizeof(void*)));
		KINITv(base->constPools, new_(Array, 64));
		kctx->modlocal[MOD_code] = (KonohaModuleContext*)base;
	}
}

static void kmodcode_reftrace(KonohaContext *kctx, struct KonohaModule *baseh)
{
	KModuleByteCode *base = (KModuleByteCode*)baseh;
	BEGIN_REFTRACE(1);
	KREFTRACEn(base->codeNull);
	END_REFTRACE();
}

static void kmodcode_free(KonohaContext *kctx, struct KonohaModule *baseh)
{
//	KModuleByteCode *base = (KModuleByteCode*)baseh;
	KFREE(baseh, sizeof(KModuleByteCode));
}

void MODCODE_init(KonohaContext *kctx, KonohaContextVar *ctx)
{
	KModuleByteCode *base = (KModuleByteCode*)KCALLOC(sizeof(KModuleByteCode), 1);
	opcode_check();
	base->h.name     = "minivm";
	base->h.setup    = kmodcode_setup;
	base->h.reftrace = kmodcode_reftrace;
	base->h.free     = kmodcode_free;
	KLIB KonohaRuntime_setModule(kctx, MOD_code, &base->h, 0);

	KDEFINE_CLASS defBasicBlock = {
		STRUCTNAME(BasicBlock),
		.init = BasicBlock_init,
		.free = BasicBlock_free,
	};

	KDEFINE_CLASS defByteCode = {
		STRUCTNAME(ByteCode),
		.init     = ByteCode_init,
		.reftrace = ByteCode_reftrace,
		.free     = ByteCode_free,
	};

	base->cBasicBlock = KLIB KonohaClass_define(kctx, PackageId_sugar, NULL, &defBasicBlock, 0);
	base->cByteCode   = KLIB KonohaClass_define(kctx, PackageId_sugar, NULL, &defByteCode, 0);
	kmodcode_setup(kctx, &base->h, 0/*lazy*/);
	{
		INIT_GCSTACK();
		kBasicBlock* ia = (kBasicBlock*)new_(BasicBlock, 0);
		kBasicBlock* ib = (kBasicBlock*)new_(BasicBlock, 0);
		PUSH_GCSTACK(ia);
		PUSH_GCSTACK(ib);
		kBasicBlock_add(ia, THCODE, _THCODE);
		kBasicBlock_add(ia, NCALL); // FUNCCALL
		kBasicBlock_add(ia, ENTER);
		kBasicBlock_add(ia, EXIT);
		kBasicBlock_add(ib, RET);   // NEED TERMINATION
		ia->nextBlock = ib;
		kByteCode *kcode = new_ByteCode(kctx, ia, ib);
		KINITv(kmodcode->codeNull, kcode);
		VirtualMachineInstruction *pc = KonohaVirtualMachine_run(kctx, kctx->esp, kcode->code);
		CODE_ENTER = pc;
		CODE_ENTER = pc+1;
		KLIB kArray_clear(kctx, ctxcode->codeList, 0);
		RESET_GCSTACK();
	}
	KonohaLibVar *l = (KonohaLibVar*)kctx->klib;
	l->kMethod_setFunc = kMethod_setFunc;
	l->kMethod_genCode = kMethod_genCode;
}

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
