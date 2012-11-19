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

//#define USE_DIRECT_THREADED_CODE

#include <minikonoha/minikonoha.h>
#include <minikonoha/klib.h>
#include <minikonoha/sugar.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OPDEFINE(MACRO)\
	MACRO(NOP)\
	MACRO(THCODE)\
	MACRO(ENTER)\
	MACRO(EXIT)\
	MACRO(NSET)\
	MACRO(NMOV)\
	MACRO(NMOVx)\
	MACRO(XNMOV)\
	MACRO(NEW)\
	MACRO(NULL)\
	MACRO(LOOKUP)\
	MACRO(CALL)\
	MACRO(RET)\
	MACRO(NCALL)\
	MACRO(BNOT)\
	MACRO(JMP)\
	MACRO(JMPF)\
	MACRO(TRYJMP)\
	MACRO(YIELD)\
	MACRO(ERROR)\
	MACRO(SAFEPOINT)\
	MACRO(CHKSTACK)\
	MACRO(TRACE)\

#include <minikonoha/arch/minivm.h>

#define OPCODE(T)  OPCODE_##T,
typedef enum {
	OPDEFINE(OPCODE)
	OPCODE_MAX,
} MiniVM;

/* ------------------------------------------------------------------------ */
/* [data] */

typedef struct {
	const char *name;
	kshortflag_t   flag;
	kushort_t argsize;
	VirtualCodeType arg1;
	VirtualCodeType arg2;
	VirtualCodeType arg3;
	VirtualCodeType arg4;
} DEFINE_OPSPEC;

#define OPSPEC(T)   {#T, 0, VPARAM_##T},
static const DEFINE_OPSPEC OPDATA[] = {
	OPDEFINE(OPSPEC)
};

static void DumpOpArgument(KonohaContext *kctx, KGrowingBuffer *wb, VirtualCodeType type, VirtualCode *c, size_t i, VirtualCode *pc_start)
{
	switch(type) {
	case VMT_VOID: break;
	case VMT_ADDR:
		KLIB Kwb_printf(kctx, wb, " L%d", (int)((VirtualCode *)c->p[i] - pc_start));
		break;
	case VMT_UL: {
		kfileline_t uline = (kfileline_t)c->data[i];
		KLIB Kwb_printf(kctx, wb, " (%s:%d)", PLATAPI shortFilePath(FileId_t(uline)), (kshort_t)uline);
		break;
	}
	case VMT_R: {
		KLIB Kwb_printf(kctx, wb, " sfp[%d,r=%d]", (int)c->data[i]/2, (int)c->data[i]);
		break;
	}
	case VMT_FX: {
		kshort_t index  = (kshort_t)c->data[i];
		kshort_t xindex = (kshort_t)(c->data[i] >> (sizeof(kshort_t)*8));
		KLIB Kwb_printf(kctx, wb, " sfp[%d,r=%d][%d]", (int)index/2, (int)index, (int)xindex);
		break;
	}
	case VMT_U:
		KLIB Kwb_printf(kctx, wb, " i%ld", (long)c->data[i]); break;
	case VMT_C:
	case VMT_TY:
		KLIB Kwb_printf(kctx, wb, "(%s)", CT_t(c->ct[i])); break;
	case VMT_F:
		KLIB Kwb_printf(kctx, wb, " function(%p)", c->p[i]); break;
	case VMT_Object: {
		kObject *o = c->o[i];
		if(IS_Method(o)) {
			kMethod *mtd = (kMethod*)o;
			KLIB Kwb_printf(kctx, wb, " %s.%s%s", TY_t(mtd->typeId), MethodName_t(mtd->mn));
		}
		else {
			KLIB Kwb_printf(kctx, wb, " (%s)", CT_t(O_ct(o)));
			KLIB kObject_WriteToBuffer(kctx, o, 0, wb, NULL, 0);
		}
		break;
	}
	case VMT_HCACHE:
		break;
	}/*switch*/
}

static void WriteVirtualCode1(KonohaContext *kctx, KGrowingBuffer *wb, VirtualCode *c, VirtualCode *pc_start)
{
	KLIB Kwb_printf(kctx, wb, "[L%d:%d] %s(%d)", (int)(c - pc_start), c->line, OPDATA[c->opcode].name, (int)c->opcode);
	DumpOpArgument(kctx, wb, OPDATA[c->opcode].arg1, c, 0, pc_start);
	DumpOpArgument(kctx, wb, OPDATA[c->opcode].arg2, c, 1, pc_start);
	DumpOpArgument(kctx, wb, OPDATA[c->opcode].arg3, c, 2, pc_start);
	DumpOpArgument(kctx, wb, OPDATA[c->opcode].arg4, c, 3, pc_start);
	KLIB Kwb_printf(kctx, wb, "\n");
}

static void WriteVirtualCode(KonohaContext *kctx, KGrowingBuffer *wb, VirtualCode *c)
{
	OPTHCODE *opTHCODE = (OPTHCODE *)(c-1);
	size_t i, n = (opTHCODE->codesize / sizeof(VirtualCode)) - 1;
	for(i = 0; i < n; i++) {
		WriteVirtualCode1(kctx, wb, c+i, c);
	}
}

static void DumpVirtualCode(KonohaContext *kctx, VirtualCode *c)
{
	KGrowingBuffer wb;
	KLIB Kwb_Init(&(kctx->stack->cwb), &wb);
	WriteVirtualCode(kctx, &wb, c);
	DBG_P(">>>\n%s", KLIB Kwb_top(kctx, &wb, true));
	KLIB Kwb_Free(&wb);
}

/* ------------------------------------------------------------------------ */
/* VirtualMacine Function */

static void kNameSpace_LookupMethodWithInlineCache(KonohaContext *kctx, KonohaStack *sfp, kNameSpace *ns, kMethod **cache)
{
	ktype_t typeId = O_typeId(sfp[0].asObject);
	kMethod *mtd = cache[0];
	if(mtd->typeId != typeId) {
		mtd = KLIB kNameSpace_GetMethodBySignatureNULL(kctx, ns, typeId, mtd->mn, mtd->paramdom, 0, NULL);
		cache[0] = mtd;
	}
	sfp[0].unboxValue = O_unbox(sfp[0].asObject);
	sfp[K_MTDIDX].calledMethod = mtd;
}

static VirtualCode* KonohaVirtualMachine_Run(KonohaContext *, KonohaStack *, VirtualCode *);

static VirtualCode *KonohaVirtualMachine_tryJump(KonohaContext *kctx, KonohaStack *sfp, VirtualCode *pc)
{
	int jmpresult;
	INIT_GCSTACK();
	KonohaStackRuntimeVar *base = kctx->stack;
	jmpbuf_i lbuf = {};
	if(base->evaljmpbuf == NULL) {
		base->evaljmpbuf = (jmpbuf_i *)KCalloc_UNTRACE(sizeof(jmpbuf_i), 1);
	}
	memcpy(&lbuf, base->evaljmpbuf, sizeof(jmpbuf_i));
	if((jmpresult = PLATAPI setjmp_i(*base->evaljmpbuf)) == 0) {
		pc = KonohaVirtualMachine_Run(kctx, sfp, pc);
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

#ifdef USE_DIRECT_THREADED_CODE
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
#define DISPATCH_START(pc) goto *OPJUMP[pc->opcode]
#define DISPATCH_END(pc)
#define GOTO_PC(pc)        GOTO_NEXT()

#define OPLABEL(T)  &&L_##T,
#define OPEXEC(T)  L_##T : { OPEXEC_##T(); pc++; goto *(pc->codeaddr); }


#else/*USE_DIRECT_THREADED_CODE*/
#define OPJUMP      NULL
#define CASE(x)     case OPCODE_##x :
#define NEXT_OP     L_HEAD
#define GOTO_NEXT() goto NEXT_OP
#define JUMP        L_HEAD
#define DISPATCH_START(pc) L_HEAD:;switch(pc->opcode) {
#define DISPATCH_END(pc)   } /*KNH_DIE("unknown opcode=%d", (int)pc->opcode)*/;
#define GOTO_PC(pc)         GOTO_NEXT()

#define OPEXEC(T)  case OPCODE_##T : { OPEXEC_##T(); pc++; GOTO_NEXT(); }

#endif/*USE_DIRECT_THREADED_CODE*/

#include<stdio.h>


static struct VirtualCode* KonohaVirtualMachine_Run(KonohaContext *kctx, KonohaStack *sfp0, struct VirtualCode *pc)
{
#ifdef USE_DIRECT_THREADED_CODE
	static void *OPJUMP[] = {
		OPDEFINE(OPLABEL)
	};
#endif
	krbp_t *rbp = (krbp_t *)sfp0;
	DISPATCH_START(pc);
	OPDEFINE(OPEXEC)
//	CASE(NOP) {
//		OPEXEC_NOP();  pc++;
//		GOTO_NEXT();
//	}
//	CASE(THCODE) {
//		OPEXEC_THCODE(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(ENTER) {
//		OPEXEC_ENTER(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(EXIT) {
//		OPEXEC_EXIT(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(NSET) {
//		OPEXEC_NSET(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(NMOV) {
//		OPEXEC_NMOV(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(NMOVx) {
//		OPEXEC_NMOVx(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(XNMOV) {
//		OPEXEC_XNMOV(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(NEW) {
//		OPEXEC_NEW(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(NULL) {
//		OPEXEC_NULL(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(LOOKUP) {
//		OPEXEC_LOOKUP(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(CALL) {
//		OPEXEC_CALL(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(RET) {
//		OPEXEC_RET(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(NCALL) {
//		OPEXEC_NCALL(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(BNOT) {
//		OPEXEC_BNOT(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(JMP) {
//		OPEXEC_JMP(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(JMPF) {
//		OPEXEC_JMPF();pc++;
//		GOTO_NEXT();
//	}
//	CASE(TRYJMP) {
//		OPEXEC_TRYJMP(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(YIELD) {
//		OPEXEC_YIELD(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(ERROR) {
//		OPEXEC_ERROR(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(SAFEPOINT) {
//		OPEXEC_SAFEPOINT(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(CHKSTACK) {
//		OPEXEC_CHKSTACK(); pc++;
//		GOTO_NEXT();
//	}
//	CASE(TRACE) {
//		OPEXEC_TRACE(); pc++;
//		GOTO_NEXT();
//	}

	DISPATCH_END(pc);
	L_RETURN:;
	return pc;
}

/* ------------------------------------------------------------------------ */

typedef intptr_t bblock_t;

struct KBuilder {   /* MiniVM Builder */
	struct KBuilderCommon common;
	// minivm local setting
	kArray          *constPools;
	bblock_t              bbBeginId;
	bblock_t              bbMainId;
	bblock_t              bbReturnId;
};

/* ------------------------------------------------------------------------ */

typedef struct BasicBlock BasicBlock;

struct BasicBlock {
	long     incoming;
	bblock_t newid;
	bblock_t nextid;
	bblock_t branchid;
	size_t   codeoffset;
	size_t   lastoffset;
	size_t   size;
	size_t   max;
};

static BasicBlock *BasicBlock_FindById(KonohaContext *kctx, bblock_t id)
{
	BasicBlock *bb = NULL;
	while(id != -1) {
		bb = (BasicBlock*)(kctx->stack->cwb.bytebuf + id);
		id = bb->newid;
	}
	return bb;
}

static bblock_t BasicBlock_id(KonohaContext *kctx, BasicBlock *bb)
{
	while(bb->newid != -1) {
		bb = BasicBlock_FindById(kctx, bb->newid);
	}
	return ((char*)bb) - kctx->stack->cwb.bytebuf;
}

static BasicBlock* new_BasicBlock(KonohaContext *kctx, size_t max, bblock_t oldId)
{
	KGrowingBuffer wb;
	KLIB Kwb_Init(&(kctx->stack->cwb), &wb);
	BasicBlock *bb = (BasicBlock*)KLIB Kwb_Alloca(kctx, &wb, max);
	if(oldId != -1) {
		BasicBlock *oldbb = BasicBlock_FindById(kctx, oldId);
		if(((char*)oldbb) + oldbb->max == (char*)bb) {
			oldbb->max += (max - sizeof(BasicBlock));
			wb.m->bytesize -= sizeof(BasicBlock);
			return oldbb;
		}
		memcpy(bb, oldbb, oldbb->size);
		oldbb->newid = BasicBlock_id(kctx, bb);
		oldbb->size = 0;
	}
	else {
		bb->size = sizeof(BasicBlock);
		bb->newid    = -1;
		bb->nextid   = -1;
		bb->branchid = -1;
	}
	bb->max = max;
	bb->codeoffset   = -1;
	bb->lastoffset   = -1;
	return bb;
}

static inline size_t newsize2(size_t max)
{
	return ((max - sizeof(BasicBlock)) * 2) + sizeof(BasicBlock);
}

static bblock_t BasicBlock_Add(KonohaContext *kctx, bblock_t blockId, kfileline_t uline, VirtualCode *op, size_t size, size_t padding_size)
{
	BasicBlock *bb = BasicBlock_FindById(kctx, blockId);
	DBG_ASSERT(bb->newid == -1);
	DBG_ASSERT(size <= padding_size);
	DBG_ASSERT(bb->nextid == -1 && bb->branchid == -1);
	if(!(bb->size + size < bb->max)) {
		size_t newsize = newsize2(bb->max);
		bb = new_BasicBlock(kctx, newsize, blockId);
	}
	memcpy(((char*)bb) + bb->size, op, size);
	bb->size += padding_size;
	return BasicBlock_id(kctx, bb);
}

/* ------------------------------------------------------------------------ */

static bblock_t new_BasicBlockLABEL(KonohaContext *kctx)
{
	BasicBlock *bb = new_BasicBlock(kctx, sizeof(VirtualCode) * 2 + sizeof(BasicBlock), -1);
	return BasicBlock_id(kctx, bb);
}

#define ASMLINE  0
#if defined(USE_DIRECT_THREADED_CODE)
#define ASM(T, ...) do {\
	OP##T op_ = {TADDR, OPCODE_##T, ASMLINE, ## __VA_ARGS__};\
	union { VirtualCode op; OP##T op_; } tmp_; tmp_.op_ = op_; \
	KBuilder_Asm(kctx, builder, &tmp_.op, sizeof(OP##T));\
} while(0)

#else
#define ASM(T, ...) do {\
	OP##T op_ = {OPCODE_##T, ASMLINE, ## __VA_ARGS__};\
	union { VirtualCode op; OP##T op_; } tmp_; tmp_.op_ = op_; \
	KBuilder_Asm(kctx, builder, &tmp_.op, sizeof(OP##T));\
} while(0)

#endif/*USE_DIRECT_THREADED_CODE*/

#define NC_(sfpidx)       (((sfpidx) * 2) + 1)
#define OC_(sfpidx)       ((sfpidx) * 2)
#define TC_(sfpidx, TYPE) ((TY_isUnbox(TYPE)) ? NC_(sfpidx) : OC_(sfpidx))
#define SFP_(sfpidx)   ((sfpidx) * 2)


static void KBuilder_Asm(KonohaContext *kctx, KBuilder *builder, VirtualCode *op, size_t opsize)
{
	builder->bbMainId = BasicBlock_Add(kctx, builder->bbMainId, builder->common.uline, op, opsize, sizeof(VirtualCode));
}

static void kStmt_setLabelBlock(KonohaContext *kctx, kStmt *stmt, ksymbol_t label, bblock_t labelId)
{
	KLIB kObject_setUnboxValue(kctx, stmt, label, TY_int, labelId);
}

static bblock_t kStmt_GetLabelBlock(KonohaContext *kctx, kStmt *stmt, ksymbol_t label)
{
	return KLIB kObject_getUnboxValue(kctx, stmt, label, -1);
}

static void ASM_LABEL(KonohaContext *kctx, KBuilder *builder, bblock_t labelId)
{
	BasicBlock *bb = BasicBlock_FindById(kctx, builder->bbMainId);
	DBG_ASSERT(bb != NULL);
	DBG_ASSERT(bb->nextid == -1);
	BasicBlock *labelBlock = BasicBlock_FindById(kctx, labelId);
	labelBlock->incoming += 1;
	builder->bbMainId = BasicBlock_id(kctx, labelBlock);
}

static void ASM_JMP(KonohaContext *kctx, KBuilder *builder, bblock_t labelId)
{
	BasicBlock *bb = BasicBlock_FindById(kctx, builder->bbMainId);
	DBG_ASSERT(bb != NULL);
	DBG_ASSERT(bb->nextid == -1);
	if(bb->branchid == -1) {
		ASM(JMP, NULL);
		BasicBlock *labelBlock = BasicBlock_FindById(kctx, labelId);
		bb = BasicBlock_FindById(kctx, builder->bbMainId);
		bb->branchid = BasicBlock_id(kctx, labelBlock);
		labelBlock->incoming += 1;
	}
}

static bblock_t KBuilder_AsmJMPF(KonohaContext *kctx, KBuilder *builder, int flocal, bblock_t jumpId)
{
	BasicBlock *bb = BasicBlock_FindById(kctx, builder->bbMainId);
	DBG_ASSERT(bb != NULL);
	DBG_ASSERT(bb->nextid == -1 && bb->branchid == -1);
	bblock_t nextId = new_BasicBlockLABEL(kctx);
	ASM(JMPF, NULL, NC_(flocal));
	bb = BasicBlock_FindById(kctx, builder->bbMainId);
	BasicBlock *lbJUMP = BasicBlock_FindById(kctx, jumpId);
	BasicBlock *lbNEXT = BasicBlock_FindById(kctx, nextId);
	bb->branchid = BasicBlock_id(kctx, lbJUMP);
	bb->nextid = nextId;
	lbNEXT->incoming += 1;
	lbJUMP->incoming += 1;
	builder->bbMainId = nextId;
	return nextId;
}

static bblock_t KBuilder_asmJMPIF(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr, int isTRUE, bblock_t labelId)
{
	int a = builder->common.a;
	SUGAR VisitExpr(kctx, builder, stmt, expr);
	if(isTRUE) {
		ASM(BNOT, NC_(a), NC_(a));
	}
	return KBuilder_AsmJMPF(kctx, builder, a, labelId);
}

static int CodeOffset(KGrowingBuffer *wb)
{
	return Kwb_bytesize(wb);
}

static void BasicBlock_WriteBuffer(KonohaContext *kctx, bblock_t blockId, KGrowingBuffer *wb)
{
	BasicBlock *bb = BasicBlock_FindById(kctx, blockId);
	while(bb != NULL && bb->codeoffset == -1) {
		size_t len = bb->size - sizeof(BasicBlock);
		bb->codeoffset = CodeOffset(wb);
		if(len > 0) {
			bblock_t id = BasicBlock_id(kctx, bb);
			char buf[len];  // bb is growing together with wb.
			memcpy(buf, ((char*)bb) + sizeof(BasicBlock), len);
			KLIB Kwb_Write(kctx, wb, buf, len);
			bb = BasicBlock_FindById(kctx, id);  // recheck
			bb->lastoffset = CodeOffset(wb) - sizeof(VirtualCode);
			DBG_ASSERT(bb->codeoffset + ((len / sizeof(VirtualCode)) - 1) * sizeof(VirtualCode) == bb->lastoffset);
		}
		else {
			DBG_ASSERT(bb->branchid == -1);
		}
		bb = BasicBlock_FindById(kctx, bb->nextid);
	}
	bb = BasicBlock_FindById(kctx, blockId);
	while(bb != NULL) {
		if(bb->branchid != -1 /*&& bb->branchid != builder->bbReturnId*/) {
			BasicBlock *bbJ = BasicBlock_FindById(kctx, bb->branchid);
			if(bbJ->codeoffset == -1) {
				BasicBlock_WriteBuffer(kctx, bb->branchid, wb);
			}
		}
		bb = BasicBlock_FindById(kctx, bb->nextid);
	}
}

static int BasicBlock_size(BasicBlock *bb)
{
	return (bb->size - sizeof(BasicBlock)) / sizeof(VirtualCode);
}

static BasicBlock *BasicBlock_leapJump(KonohaContext *kctx, BasicBlock *bb)
{
	while(bb->nextid != -1) {
		if(BasicBlock_size(bb) != 0) return bb;
		bb = BasicBlock_FindById(kctx, bb->nextid);
	}
	if(bb->nextid == -1 && bb->branchid != -1 && BasicBlock_size(bb) == 1) {
		return BasicBlock_leapJump(kctx, BasicBlock_FindById(kctx, bb->branchid));
	}
	return bb;
}

#define BasicBlock_isVisited(bb)     (bb->incoming == -1)
#define BasicBlock_setVisited(bb)    bb->incoming = -1

static void BasicBlock_setJumpAddr(KonohaContext *kctx, BasicBlock *bb, char *vcode)
{
	while(bb != NULL) {
		BasicBlock_setVisited(bb);
		if(bb->branchid != -1) {
			BasicBlock *bbJ = BasicBlock_leapJump(kctx, BasicBlock_FindById(kctx, bb->branchid));
			OPJMP *j = (OPJMP *)(vcode + bb->lastoffset);
			DBG_ASSERT(j->opcode == OPCODE_JMP || j->opcode == OPCODE_JMPF);
			j->jumppc = (VirtualCode*)(vcode + bbJ->codeoffset);
			bbJ = BasicBlock_FindById(kctx, bb->branchid);
			if(!BasicBlock_isVisited(bbJ)) {
				BasicBlock_setVisited(bbJ);
				BasicBlock_setJumpAddr(kctx, bbJ, vcode);
			}
		}
		bb = BasicBlock_FindById(kctx, bb->nextid);
	}
}

static kObject* KBuilder_AddConstPool(KonohaContext *kctx, KBuilder *builder, kObject *o)
{
	KLIB kArray_Add(kctx, builder->constPools, o);
	return o;
}

static void KBuilder_AsmSAFEPOINT(KonohaContext *kctx, KBuilder *builder, kfileline_t uline, int espidx)
{
	ASM(SAFEPOINT, uline, SFP_(espidx));
}

static void KBuilder_AsmNMOV(KonohaContext *kctx, KBuilder *builder, int a, ktype_t ty, int b)
{
	ASM(NMOV, TC_(a, ty), TC_(b, ty), CT_(ty));
}


//----------------------------------------------------------------------------

static kBlock* Stmt_getFirstBlock(KonohaContext *kctx, kStmt *stmt)
{
	return SUGAR kStmt_GetBlock(kctx, stmt, NULL, KW_BlockPattern, K_NULLBLOCK);
}

static kBlock* Stmt_getElseBlock(KonohaContext *kctx, kStmt *stmt)
{
	return SUGAR kStmt_GetBlock(kctx, stmt, NULL, KW_else, K_NULLBLOCK);
}

static kExpr* Stmt_getFirstExpr(KonohaContext *kctx, kStmt *stmt)
{
	return SUGAR kStmt_GetExpr(kctx, stmt, KW_ExprPattern, NULL);
}

static kStmt *kStmt_GetStmt(KonohaContext *kctx, kStmt *stmt, ksymbol_t kw)
{
	return (kStmt *) kStmt_GetObject(kctx, stmt, kw, NULL);
}

static kMethod* CallExpr_getMethod(kExpr *expr)
{
	return expr->cons->MethodItems[0];
}

static int CallExpr_getArgCount(kExpr *expr)
{
	return kArray_size(expr->cons) - 2;
}

static kString* Stmt_getErrorMessage(KonohaContext *kctx, kStmt *stmt)
{
	kString* msg = (kString *)kStmt_GetObjectNULL(kctx, stmt, KW_ERR);
	DBG_ASSERT(IS_String(msg));
	return msg;
}

/* Visitor */

static void KBuilder_VisitErrStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	ASM(ERROR, stmt->uline, Stmt_getErrorMessage(kctx, stmt));
}

static void KBuilder_VisitExprStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	int a = builder->common.a;
	builder->common.a = builder->common.espidx;
	SUGAR VisitExpr(kctx, builder, stmt, Stmt_getFirstExpr(kctx, stmt));
	builder->common.a = a;
}

static void KBuilder_VisitBlockStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	SUGAR VisitBlock(kctx, builder, Stmt_getFirstBlock(kctx, stmt));
}

static void KBuilder_VisitReturnStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	kExpr *expr = SUGAR kStmt_GetExpr(kctx, stmt, KW_ExprPattern, NULL);
	if(expr != NULL && IS_Expr(expr) && expr->ty != TY_void) {
		int a = builder->common.a;
		builder->common.a = K_RTNIDX;
		SUGAR VisitExpr(kctx, builder, stmt, expr);
		builder->common.a = a;
	}
	ASM_JMP(kctx, builder, builder->bbReturnId); // RET
}

static void KBuilder_VisitIfStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	int espidx = builder->common.espidx;
	int a = builder->common.a;
	bblock_t lbELSE = new_BasicBlockLABEL(kctx);
	bblock_t lbEND  = new_BasicBlockLABEL(kctx);
	/* if */
	builder->common.a = espidx;
	KBuilder_asmJMPIF(kctx, builder, stmt, Stmt_getFirstExpr(kctx, stmt), 0/*FALSE*/, lbELSE);
	builder->common.a = a;
	/* then */
	SUGAR VisitBlock(kctx, builder, Stmt_getFirstBlock(kctx, stmt));
	ASM_JMP(kctx, builder, lbEND);
	/* else */
	ASM_LABEL(kctx, builder, lbELSE);
	SUGAR VisitBlock(kctx, builder, Stmt_getElseBlock(kctx, stmt));
	/* endif */
	ASM_LABEL(kctx, builder, lbEND);
}

static void KBuilder_VisitLoopStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	int espidx = builder->common.espidx;
	int a = builder->common.a;
	bblock_t lbCONTINUE = new_BasicBlockLABEL(kctx);
	bblock_t lbENTRY    = new_BasicBlockLABEL(kctx);
	bblock_t lbBREAK    = new_BasicBlockLABEL(kctx);
	kStmt_setLabelBlock(kctx, stmt, SYM_("continue"), lbCONTINUE);
	kStmt_setLabelBlock(kctx, stmt, SYM_("break"),    lbBREAK);
	if(kStmt_Is(RedoLoop, stmt)) {
		ASM_JMP(kctx, builder, lbENTRY);
	}
	ASM_LABEL(kctx, builder, lbCONTINUE);
	KBuilder_AsmSAFEPOINT(kctx, builder, stmt->uline, espidx);
	kBlock *iterBlock = SUGAR kStmt_GetBlock(kctx, stmt, NULL, SYM_("Iterator"), NULL);
	if(iterBlock != NULL) {
		SUGAR VisitBlock(kctx, builder, iterBlock);
		ASM_LABEL(kctx, builder, lbENTRY);
		builder->common.a = espidx;
		KBuilder_asmJMPIF(kctx, builder, stmt, Stmt_getFirstExpr(kctx, stmt), 0/*FALSE*/, lbBREAK);
		builder->common.a = a;
	}
	else {
		builder->common.a = espidx;
		KBuilder_asmJMPIF(kctx, builder, stmt, Stmt_getFirstExpr(kctx, stmt), 0/*FALSE*/, lbBREAK);
		builder->common.a = a;
		ASM_LABEL(kctx, builder, lbENTRY);
	}
	SUGAR VisitBlock(kctx, builder, Stmt_getFirstBlock(kctx, stmt));
	ASM_JMP(kctx, builder, lbCONTINUE);
	ASM_LABEL(kctx, builder, lbBREAK);
}

static void KBuilder_VisitJumpStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	SugarSyntax *syn = stmt->syn;
	kStmt *jump = kStmt_GetStmt(kctx, stmt, syn->keyword);
	DBG_ASSERT(jump != NULL && IS_Stmt(jump));
	bblock_t lbJUMP = kStmt_GetLabelBlock(kctx, jump, syn->keyword);
	ASM_JMP(kctx, builder, lbJUMP);
}

static void KBuilder_VisitTryStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	//FIXME
	//kBlock *catchBlock   = SUGAR kStmt_GetBlock(kctx, stmt, NULL, SYM_("catch"),   K_NULLBLOCK);
	//kBlock *finallyBlock = SUGAR kStmt_GetBlock(kctx, stmt, NULL, SYM_("finally"), K_NULLBLOCK);
	//if(catchBlock != K_NULLBLOCK){
	//}
	//if(finallyBlock != K_NULLBLOCK){
	//}
}


static void KBuilder_VisitUndefinedStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	DBG_P("undefined asm syntax kw='%s'", SYM_t(stmt->syn->keyword));
}

static void KBuilder_VisitConstExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	int a = builder->common.a;
	kObject *v = expr->objectConstValue;
	DBG_ASSERT(!TY_isUnbox(expr->ty));
	DBG_ASSERT(Expr_hasObjectConstValue(expr));
	v = KBuilder_AddConstPool(kctx, builder, v);
	ASM(NSET, OC_(a), (uintptr_t)v, CT_(expr->ty));
}

static void KBuilder_VisitNConstExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	int a = builder->common.a;
	ASM(NSET, NC_(a), expr->unboxConstValue, CT_(expr->ty));
}

static void KBuilder_VisitNewExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	int a = builder->common.a;
	ASM(NEW, OC_(a), expr->index, CT_(expr->ty));
}

static void KBuilder_VisitNullExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	int a = builder->common.a;
	if(TY_isUnbox(expr->ty)) {
		ASM(NSET, NC_(a), 0, CT_(expr->ty));
	}
	else {
		ASM(NULL, OC_(a), CT_(expr->ty));
	}
}

static void KBuilder_VisitLocalExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	KBuilder_AsmNMOV(kctx, builder, builder->common.a, expr->ty, expr->index);
}

static void KBuilder_VisitBlockExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	int a      = builder->common.a;
	int shift  = builder->common.shift;
	int espidx = builder->common.espidx;
	DBG_ASSERT(IS_Block(expr->block));
	builder->common.shift = builder->common.espidx;
	SUGAR VisitBlock(kctx, builder, expr->block);
	builder->common.shift = shift;
	KBuilder_AsmNMOV(kctx, builder, a, expr->ty, espidx);
	builder->common.espidx = espidx;
}

static void KBuilder_VisitFieldExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	int a = builder->common.a;
	kshort_t index = (kshort_t)expr->index;
	kshort_t xindex = (kshort_t)(expr->index >> (sizeof(kshort_t)*8));
	ASM(NMOVx, TC_(a, expr->ty), OC_(index), xindex, CT_(expr->ty));
}

static void KBuilder_VisitCallExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	kMethod *mtd = CallExpr_getMethod(expr);
	DBG_ASSERT(IS_Method(mtd));

	/*
	 * [CallExpr] := this.method(arg1, arg2, ...)
	 * expr->cons = [method, this, arg1, arg2, ...]
	 **/
	int i, a = builder->common.a;
	int s = kMethod_is(Static, mtd) ? 2 : 1;
	int espidx  = builder->common.espidx;
	int thisidx = espidx + K_CALLDELTA;
	int argc = CallExpr_getArgCount(expr);
	for (i = s; i < argc + 2; i++) {
		kExpr *exprN = kExpr_at(expr, i);
		DBG_ASSERT(IS_Expr(exprN));
		builder->common.a = builder->common.espidx = thisidx + i - 1;
		SUGAR VisitExpr(kctx, builder, stmt, exprN);
	}
	builder->common.espidx = espidx;
	builder->common.a = a;

	if(kMethod_is(Final, mtd) || !kMethod_is(Virtual, mtd)) {
		ASM(NSET, NC_(thisidx-1), (intptr_t)mtd, CT_Method);
		if(kMethod_is(Virtual, mtd)) {
			// set namespace to enable method lookups
			ASM(NSET, OC_(thisidx-2), (intptr_t)Stmt_ns(stmt), CT_NameSpace);
		}
	}
	else {
		ASM(NSET, OC_(thisidx-2), (intptr_t)Stmt_ns(stmt), CT_NameSpace);
		ASM(LOOKUP, SFP_(thisidx), Stmt_ns(stmt), mtd);
	}

	int esp_ = SFP_(espidx + argc + K_CALLDELTA + 1);
	ASM(CALL, builder->common.uline, SFP_(thisidx), esp_, KLIB Knull(kctx, CT_(expr->ty)));

	if(mtd->mn == MN_box) {  /* boxed value of unbox value must be shifted to OC */
		((kExprVar *)expr)->ty = TY_Object;
	}

	if(a != espidx) {
		KBuilder_AsmNMOV(kctx, builder, a, expr->ty, espidx);
	}
}

static void KBuilder_VisitAndExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
//	bblock_t lbFALSE = new_BasicBlockLABEL(kctx);
	bblock_t lbFINAL  = new_BasicBlockLABEL(kctx);
	KBuilder_asmJMPIF(kctx, builder, stmt, kExpr_at(expr, 1), 0/*FALSE*/, lbFINAL);
	SUGAR VisitExpr(kctx, builder, stmt, kExpr_at(expr, 2));
//	ASM_JMP(kctx, builder, lbFINAL);
//
//	ASM_LABEL(kctx, builder, lbFALSE); // false
	//ASM(NSET, NC_(a), 0/*FALSE*/, CT_Boolean);
	//ASM(NOP);
	ASM_LABEL(kctx, builder, lbFINAL);
}

static void KBuilder_VisitOrExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	bblock_t lbFALSE = new_BasicBlockLABEL(kctx);
	bblock_t lbFINAL  = new_BasicBlockLABEL(kctx);
	KBuilder_asmJMPIF(kctx, builder, stmt, kExpr_at(expr, 1), 0/*FALSE*/, lbFALSE);
	ASM_JMP(kctx, builder, lbFINAL);

	ASM_LABEL(kctx, builder, lbFALSE); // false
	SUGAR VisitExpr(kctx, builder, stmt, kExpr_at(expr, 2));
	ASM_LABEL(kctx, builder, lbFINAL);
}

static void KBuilder_VisitLetExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	int a = builder->common.a;
	int shift = builder->common.shift;
	int espidx = builder->common.espidx;

	/*
	 * [LetExpr] := lhs = rhs
	 * expr->cons = [NULL, lhs, rhs]
	 **/

	kExpr *leftHandExpr = kExpr_at(expr, 1);
	kExpr *rightHandExpr = kExpr_at(expr, 2);
	//DBG_P("LET (%s) a=%d, shift=%d, espidx=%d", TY_t(expr->ty), a, shift, espidx);
	if(leftHandExpr->build == TEXPR_LOCAL) {
		builder->common.a = leftHandExpr->index;
		SUGAR VisitExpr(kctx, builder, stmt, rightHandExpr);
		builder->common.a = a;
		if(expr->ty != TY_void && a != leftHandExpr->index) {
			KBuilder_AsmNMOV(kctx, builder, a, leftHandExpr->ty, leftHandExpr->index);
		}
	}
	else if(leftHandExpr->build == TEXPR_STACKTOP) {
		//DBG_P("LET TEXPR_STACKTOP a=%d, leftHandExpr->index=%d, espidx=%d", a, leftHandExpr->index, espidx);
		builder->common.a = leftHandExpr->index + shift;
		SUGAR VisitExpr(kctx, builder, stmt, rightHandExpr);
		builder->common.a = a;
		if(expr->ty != TY_void && a != leftHandExpr->index + shift) {
			KBuilder_AsmNMOV(kctx, builder, a, leftHandExpr->ty, leftHandExpr->index + shift);
		}
	}
	else{
		assert(leftHandExpr->build == TEXPR_FIELD);
		builder->common.a = espidx;
		SUGAR VisitExpr(kctx, builder, stmt, rightHandExpr);
		builder->common.a = a;
		kshort_t index  = (kshort_t)leftHandExpr->index;
		kshort_t xindex = (kshort_t)(leftHandExpr->index >> (sizeof(kshort_t)*8));
		KonohaClass *lhsClass = CT_(leftHandExpr->ty);
		ASM(XNMOV, OC_(index), xindex, TC_(espidx, rightHandExpr->ty), lhsClass);
		if(expr->ty != TY_void) {
			ASM(NMOVx, TC_(a, rightHandExpr->ty), OC_(index), xindex, lhsClass);
		}
	}
}

static void KBuilder_VisitStackTopExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	int shift = builder->common.shift;
	int a = builder->common.a;
	int espidx = builder->common.espidx;
	DBG_ASSERT(expr->index + shift < espidx);
	KBuilder_AsmNMOV(kctx, builder, a, expr->ty, expr->index + shift);
}

// end of Visitor

static void FreeVirtualCode(KonohaContext *kctx, struct VirtualCode *vcode)
{
	OPTHCODE * opTHCODE = (OPTHCODE *)(vcode - 1);
	if(opTHCODE->opcode == OPCODE_THCODE && opTHCODE->codesize > 0) {
		KFree(opTHCODE, opTHCODE->codesize);
	}
}

static struct VirtualCodeAPI vapi = {
		FreeVirtualCode, WriteVirtualCode
};

static struct VirtualCode *MakeThreadedCode(KonohaContext *kctx, KBuilder *builder, VirtualCode *vcode, size_t codesize)
{
	OPTHCODE *opTHCODE = (OPTHCODE *)vcode;
	opTHCODE->codesize = codesize;
	struct VirtualCodeAPI** p = (struct VirtualCodeAPI **)builder->common.api->RunVirtualMachine(kctx, kctx->esp + 1, vcode);
	p[-1] = &vapi;
	return (VirtualCode *)p;
}

static struct VirtualCode *CompileVirtualCode(KonohaContext *kctx, KBuilder *builder, bblock_t beginId, bblock_t returnId)
{
	KGrowingBuffer wb;
	KLIB Kwb_Init(&(kctx->stack->cwb), &wb);
	BasicBlock_WriteBuffer(kctx, beginId, &wb);
	BasicBlock_WriteBuffer(kctx, returnId, &wb);

	size_t codesize = Kwb_bytesize(&wb);
	DBG_P(">>>>>> codesize=%d", codesize);
	DBG_ASSERT(codesize != 0);
	VirtualCode *vcode = (VirtualCode *)KCalloc_UNTRACE(codesize, 1);
	memcpy((void*)vcode, KLIB Kwb_top(kctx, &wb, 0), codesize);
	BasicBlock_setJumpAddr(kctx, BasicBlock_FindById(kctx, beginId), (char*)vcode);
	KLIB Kwb_Free(&wb);
	vcode = MakeThreadedCode(kctx, builder, vcode, codesize);
	DumpVirtualCode(kctx, vcode);
	return vcode;
}

static void _THCODE(KonohaContext *kctx, VirtualCode *pc, void **codeaddr, size_t codesize)
{
#ifdef USE_DIRECT_THREADED_CODE
	size_t i, n = codesize / sizeof(VirtualCode);
	for(i = 0; i < n; i++) {
		pc->codeaddr = codeaddr[pc->opcode];
		pc++;
	}
#endif
}

static struct VirtualCode* MiniVM_GenerateVirtualCode(KonohaContext *kctx, kBlock *block, int option)
{
	KGrowingBuffer wb;
	KLIB Kwb_Init(&(kctx->stack->cwb), &wb);

	INIT_GCSTACK();
	KBuilder builderbuf = {}, *builder = &builderbuf;
	kNameSpace *ns = block->BlockNameSpace;
	builder->common.api = ns->builderApi;
	builder->common.espidx = 0;
	builder->common.a = 0;
	builder->common.shift = 0;

	builder->constPools = ns->NameSpaceConstList;
	builder->bbBeginId = new_BasicBlockLABEL(kctx);
	builder->bbReturnId = new_BasicBlockLABEL(kctx);
	builder->bbMainId = builder->bbBeginId;
	ASM(THCODE, 0, _THCODE);
	ASM(CHKSTACK, 0);
	//ASM_LABEL(kctx, builder, builder->bbBeginId);

	SUGAR VisitBlock(kctx, builder, block);

	builder->common.shift = 0;
	ASM_LABEL(kctx, builder,  builder->bbReturnId);
//	if(mtd->mn == MN_new) {
//		ASM(NMOV, OC_(K_RTNIDX), OC_(0), CT_(mtd->typeId));   // FIXME: Type 'This' must be resolved
//	}
	ASM(RET);
	VirtualCode *vcode = CompileVirtualCode(kctx, builder, builder->bbBeginId, builder->bbReturnId);
	RESET_GCSTACK();
	KLIB Kwb_Free(&wb);
	return vcode;
}

// -------------------------------------------------------------------------

static struct VirtualCode  *BOOTCODE_ENTER = NULL;
static struct VirtualCode  *BOOTCODE_NCALL = NULL;

static void SetUpBootCode(void)
{
	if(BOOTCODE_ENTER == NULL) {
		static struct VirtualCode InitCode[6] = {};
		struct OPTHCODE thcode = {OP_(THCODE), 4 * sizeof(VirtualCode), _THCODE};
		struct OPNCALL ncall = {OP_(NCALL)};
		struct OPENTER enter = {OP_(ENTER)};
		struct OPEXIT  exit  = {OP_(EXIT)};
		memcpy(InitCode,   &thcode, sizeof(VirtualCode));
		memcpy(InitCode+1, &ncall,  sizeof(VirtualCode));
		memcpy(InitCode+2, &enter,  sizeof(VirtualCode));
		memcpy(InitCode+3, &exit,   sizeof(VirtualCode));
		VirtualCode *pc = KonohaVirtualMachine_Run(NULL, NULL, InitCode);
		BOOTCODE_NCALL = pc;
		BOOTCODE_ENTER = pc+1;
	}
}

static kbool_t IsSupportedVirtualCode(int opcode)
{
	return (((size_t)opcode) < OPCODE_MAX);
}

static KMETHOD MethodFunc_RunVirtualMachine(KonohaContext *kctx, KonohaStack *sfp)
{
	DBG_ASSERT(IS_Method(sfp[K_MTDIDX].calledMethod));
	KonohaVirtualMachine_Run(kctx, sfp, BOOTCODE_ENTER);
}

static MethodFunc MiniVM_GenerateMethodFunc(KonohaContext *kctx, VirtualCode *vcode)
{
	return MethodFunc_RunVirtualMachine;
}

static struct VirtualCode* GetDefaultBootCode(void)
{
	return BOOTCODE_NCALL;
}

static void InitStaticBuilderApi(struct KBuilderAPI2 *builderApi)
{
	builderApi->target = "minivm";
#define DEFINE_BUILDER_API(NAME) builderApi->visit##NAME = KBuilder_Visit##NAME;
	VISITOR_LIST(DEFINE_BUILDER_API);
#undef DEFINE_BUILDER_API
	builderApi->GenerateVirtualCode = MiniVM_GenerateVirtualCode;
	builderApi->GenerateMethodFunc = MiniVM_GenerateMethodFunc;
	builderApi->RunVirtualMachine   = KonohaVirtualMachine_Run;
}

static struct KBuilderAPI2* GetDefaultBuilderAPI(void)
{
	static struct KBuilderAPI2 builderApi = {};
	if(builderApi.target == NULL) {
		InitStaticBuilderApi(&builderApi);
	}
	return &builderApi;
}

// -------------------------------------------------------------------------

kbool_t LoadMiniVMModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"MiniVM", K_VERSION, 0, "minivm",
	};
	SetUpBootCode();
	factory->VirtualMachineInfo            = &ModuleInfo;
	factory->IsSupportedVirtualCode        = IsSupportedVirtualCode;
//	factory->RunVirtualMachine             = KonohaVirtualMachine_Run;
//	factory->DeleteVirtualMachine          = KonohaVirtualMachine_delete;
//	factory->GetVirtualMachineMethodFunc   = GetVirtualMachineMethodFunc;
	factory->GetDefaultBootCode            = GetDefaultBootCode;
	factory->GetDefaultBuilderAPI          = GetDefaultBuilderAPI;
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

