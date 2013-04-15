/****************************************************************************
 * Copyright (c) 2013, the Konoha project authors. All rights reserved.
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

#ifndef _MSC_VER
#define USE_DIRECT_THREADED_CODE
#endif
#define USE_EXECUTIONENGINE

#include "konoha3.h"

#include "konoha3/import/module.h"
#include "minivm.h"

#ifdef _MSC_VER
#undef ERROR
#endif

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
	MACRO(NUL)\
	MACRO(BOX)\
	MACRO(BBOX)\
	MACRO(LOOKUP)\
	MACRO(CALL)\
	MACRO(RET)\
	MACRO(NCALL)\
	MACRO(JMP)\
	MACRO(JMPF)\
	MACRO(TRYJMP)\
	MACRO(YIELD)\
	MACRO(ERROR)\
	MACRO(SAFEPOINT)\
	MACRO(CHKSTACK)\

#define OPCODE(T)  OPCODE_##T,
typedef enum MiniVMOpCode {
	OPDEFINE(OPCODE)
	OPCODE_MAX
} MiniVMOpCode;

/* ------------------------------------------------------------------------ */
/* [data] */

typedef struct {
	const char *name;
	khalfflag_t   flag;
	kuhalfword_t argsize;
	KVirtualCodeType arg1;
	KVirtualCodeType arg2;
	KVirtualCodeType arg3;
	KVirtualCodeType arg4;
} DEFINE_OPSPEC;

#define OPSPEC(T)   {#T, 0, VPARAM_##T},
static const DEFINE_OPSPEC OPDATA[] = {
	OPDEFINE(OPSPEC)
};

static void DumpOpArgument(KonohaContext *kctx, KBuffer *wb, KVirtualCodeType type, KVirtualCode *c, size_t i, KVirtualCode *vcode_start)
{
	switch(type) {
	case VMT_VOID: break;
	case VMT_ADDR:
		KLIB KBuffer_printf(kctx, wb, " L%d", (int)((KVirtualCode *)c->p[i] - vcode_start));
		break;
	case VMT_UL: {
		kfileline_t uline = (kfileline_t)c->data[i];
		KLIB KBuffer_printf(kctx, wb, " (%s:%d)", PLATAPI shortFilePath(KFileLine_textFileName(uline)), (khalfword_t)uline);
		break;
	}
	case VMT_R: {
		KLIB KBuffer_printf(kctx, wb, " sfp[%d,r=%d]", (int)c->data[i]/2, (int)c->data[i]);
		break;
	}
	case VMT_FX: {
		khalfword_t index  = (khalfword_t)c->data[i];
		khalfword_t xindex = (khalfword_t)(c->data[i] >> (sizeof(khalfword_t)*8));
		KLIB KBuffer_printf(kctx, wb, " sfp[%d,r=%d][%d]", (int)index/2, (int)index, (int)xindex);
		break;
	}
	case VMT_U:
		KLIB KBuffer_printf(kctx, wb, " i%ld", (long)c->data[i]); break;
	case VMT_C:
	case VMT_TY:
		KLIB KBuffer_printf(kctx, wb, "(%s)", KClass_text(c->ct[i])); break;
	case VMT_F:
		KLIB KBuffer_printf(kctx, wb, " function(%p)", c->p[i]); break;
	case VMT_Object: {
		kObject *o = c->o[i];
		if(IS_Method(o)) {
			kMethod *mtd = (kMethod *)o;
			KLIB KBuffer_printf(kctx, wb, " %s.%s%s", KType_text(mtd->typeId), KMethodName_Fmt2(mtd->mn));
		}
		else {
			KLIB KBuffer_printf(kctx, wb, " (%s)", KClass_text(kObject_class(o)));
			KLIB kObject_WriteToBuffer(kctx, o, 0, wb, NULL, 0);
		}
		break;
	}
	case VMT_HCACHE:
		break;
	}/*switch*/
}

static void WriteVirtualCode1(KonohaContext *kctx, KBuffer *wb, KVirtualCode *c, KVirtualCode *vcode_start)
{
	KLIB KBuffer_printf(kctx, wb, "[L%d:%d] %s(%d)", (int)(c - vcode_start), c->line, OPDATA[c->opcode].name, (int)c->opcode);
	DumpOpArgument(kctx, wb, OPDATA[c->opcode].arg1, c, 0, vcode_start);
	DumpOpArgument(kctx, wb, OPDATA[c->opcode].arg2, c, 1, vcode_start);
	DumpOpArgument(kctx, wb, OPDATA[c->opcode].arg3, c, 2, vcode_start);
	DumpOpArgument(kctx, wb, OPDATA[c->opcode].arg4, c, 3, vcode_start);
	KLIB KBuffer_printf(kctx, wb, "\n");
}

static void WriteVirtualCode(KonohaContext *kctx, KBuffer *wb, KVirtualCode *c)
{
	OPTHCODE *opTHCODE = (OPTHCODE *)(c-1);
	size_t i, n = (opTHCODE->codesize / sizeof(KVirtualCode)) - 1;
	for(i = 0; i < n; i++) {
		WriteVirtualCode1(kctx, wb, c+i, c);
	}
}

static void DumpVirtualCode(KonohaContext *kctx, KVirtualCode *c)
{
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	WriteVirtualCode(kctx, &wb, c);
	DBG_P(">>>\n%s", KLIB KBuffer_text(kctx, &wb, EnsureZero));
	KLIB KBuffer_Free(&wb);
}

/* ------------------------------------------------------------------------ */
/* VirtualMacine Function */

static void kNameSpace_LookupMethodWithInlineCache(KonohaContext *kctx, KonohaStack *sfp, kNameSpace *ns, kMethod **cache)
{
	ktypeattr_t typeId = kObject_typeId(sfp[0].asObject);
	kMethod *mtd = cache[0];
	if(mtd->typeId != typeId) {
		KClass *ct = kObject_class(sfp[0].asObject);
		mtd =  KLIB kNameSpace_GetMethodBySignatureNULL(kctx, ns, ct, mtd->mn, mtd->paramdom, 0, NULL);
		cache[0] = mtd;
	}
	KStackSetUnboxValue(sfp[0].unboxValue, kObject_Unbox(sfp[0].asObject));
	KStackSetUnboxValue(sfp[K_MTDIDX].calledMethod, mtd);
}

static KVirtualCode *MiniVM_RunVirtualMachine(KonohaContext *, KonohaStack *, KVirtualCode *);

static KVirtualCode *KonohaVirtualMachine_tryJump(KonohaContext *kctx, KonohaStack *sfp, KVirtualCode *pc)
{
	int jmpresult;
	INIT_GCSTACK();
	KRuntimeContextVar *base = kctx->stack;
	jmpbuf_i lbuf = {0};
	if(base->evaljmpbuf == NULL) {
		base->evaljmpbuf = (jmpbuf_i *)KCalloc_UNTRACE(sizeof(jmpbuf_i), 1);
	}
	memcpy(&lbuf, base->evaljmpbuf, sizeof(jmpbuf_i));
	if((jmpresult = PLATAPI setjmp_i(*base->evaljmpbuf)) == 0) {
		pc = MiniVM_RunVirtualMachine(kctx, sfp, pc);
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
#define NEXT_OP     L_HEAD
#define GOTO_NEXT() goto NEXT_OP
#define JUMP        L_HEAD
#define DISPATCH_START(pc) L_HEAD:;switch(pc->opcode) {
#define DISPATCH_END(pc)   } /*KNH_DIE("unknown opcode=%d", (int)pc->opcode)*/;
#define GOTO_PC(pc)         GOTO_NEXT()

#define OPEXEC(T)  case OPCODE_##T : { OPEXEC_##T(); pc++; GOTO_NEXT(); }

#endif/*USE_DIRECT_THREADED_CODE*/

static struct KVirtualCode *MiniVM_RunVirtualMachine(KonohaContext *kctx, KonohaStack *sfp0, struct KVirtualCode *pc)
{
#ifdef USE_DIRECT_THREADED_CODE
	static void *OPJUMP[] = {
		OPDEFINE(OPLABEL)
	};
#endif
	krbp_t *rbp = (krbp_t *)sfp0;
	DISPATCH_START(pc);
	OPDEFINE(OPEXEC);
	DISPATCH_END(pc);
	L_RETURN:;
	return pc;
}

/* ------------------------------------------------------------------------ */

typedef intptr_t bblock_t;

struct KBuilder { /* MiniVM Builder */
	struct KBuilderCommon common;
	// minivm local setting
	kArray    *constPools;
	kMethod   *currentMtd;
	KGrowingArray localVar;
	intptr_t   localVarSize;
	bblock_t   bbBeginId;
	bblock_t   bbMainId;
	bblock_t   bbReturnId;
	intptr_t   stackbase;
	intptr_t   Value;
	intptr_t   InstructionSize;
};

typedef struct tagBasicBlock {
	long     incoming;
	bblock_t newid;
	bblock_t nextid;
	bblock_t branchid;
	long     codeoffset;
	long     lastoffset;
	size_t   size;
	size_t   max;
} BasicBlock;

static BasicBlock *BasicBlock_FindById(KonohaContext *kctx, bblock_t id)
{
	BasicBlock *bb = NULL;
	while(id != -1) {
		bb = (BasicBlock *)(kctx->stack->cwb.bytebuf + id);
		id = bb->newid;
	}
	return bb;
}

static bblock_t BasicBlock_id(KonohaContext *kctx, BasicBlock *bb)
{
	while(bb->newid != -1) {
		bb = BasicBlock_FindById(kctx, bb->newid);
	}
	return ((char *)bb) - kctx->stack->cwb.bytebuf;
}

/*----------------------------------------------------------------------------*/

static BasicBlock *new_BasicBlock(KonohaContext *kctx, size_t max, bblock_t oldId)
{
	BasicBlock *bb;
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	bb = (BasicBlock *)KLIB KBuffer_Alloca(kctx, &wb, max);
	if(oldId != -1) {
		BasicBlock *oldbb = BasicBlock_FindById(kctx, oldId);
		if(((char *)oldbb) + oldbb->max == (char *)bb) {
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

static bblock_t BasicBlock_Add(KonohaContext *kctx, bblock_t blockId, kfileline_t uline, KVirtualCode *op, size_t size, size_t padding_size)
{
	BasicBlock *bb = BasicBlock_FindById(kctx, blockId);
	DBG_ASSERT(bb->newid == -1);
	DBG_ASSERT(size <= padding_size);
	DBG_ASSERT(bb->nextid == -1 && bb->branchid == -1);
	if(!(bb->size + size < bb->max)) {
		size_t newsize = newsize2(bb->max);
		bb = new_BasicBlock(kctx, newsize, blockId);
	}
	op->line = uline;
	memcpy(((char *)bb) + bb->size, op, size);
	bb->size += padding_size;
	return BasicBlock_id(kctx, bb);
}

typedef struct ByteCodeWriter {
	KVirtualCode *current;
	int offset;
	int codesize;
} ByteCodeWriter;

static void BasicBlock_Optimize(KonohaContext *kctx, bblock_t blockId, ByteCodeWriter *writer)
{
	BasicBlock *bb = BasicBlock_FindById(kctx, blockId);
	while(bb != NULL && bb->lastoffset == -1) {
		bb->lastoffset = 0;
		if(bb->nextid == bb->branchid  && bb->nextid != -1) {
			//bb->branchid = -1;
			//writer->codesize -= sizeof(KVirtualCode);
			//bb->size -= sizeof(KVirtualCode); // remove unnecesarry jump ..
		}
		bb = BasicBlock_FindById(kctx, bb->nextid);
	}
	bb = BasicBlock_FindById(kctx, blockId);
	while(bb != NULL) {
		if(bb->branchid != -1) {
			BasicBlock *bbJ = BasicBlock_FindById(kctx, bb->branchid);
			if(bbJ->lastoffset == -1) {
				BasicBlock_Optimize(kctx, bb->branchid, writer);
			}
		}
		bb = BasicBlock_FindById(kctx, bb->nextid);
	}
}

static int CodeOffset(ByteCodeWriter *writer)
{
	return writer->offset;
}

static void WriteByteCode(ByteCodeWriter *writer, char *code, size_t len)
{
	memcpy(writer->current, code, len);
	writer->current += len / sizeof(KVirtualCode);
	writer->offset  += len;
}

static int BasicBlock_size(BasicBlock *bb)
{
	return (bb->size - sizeof(BasicBlock)) / sizeof(KVirtualCode);
}

static void BasicBlock_WriteByteCode(KonohaContext *kctx, bblock_t blockId, ByteCodeWriter *writer)
{
	BasicBlock *bb = BasicBlock_FindById(kctx, blockId);
	while(bb != NULL && bb->codeoffset == -1) {
		intptr_t len = bb->size - sizeof(BasicBlock);
		bb->codeoffset = CodeOffset(writer);
		bb->lastoffset = -1; // reset lastoffset
		if(len > 0) {
			bblock_t id = BasicBlock_id(kctx, bb);
			WriteByteCode(writer, ((char *)bb) + sizeof(BasicBlock), len);
			bb = BasicBlock_FindById(kctx, id);  // recheck
			bb->lastoffset = CodeOffset(writer) - sizeof(KVirtualCode);
			DBG_ASSERT(bb->codeoffset + ((len / sizeof(KVirtualCode)) - 1) * sizeof(KVirtualCode) == (size_t) bb->lastoffset);
		}
		else {
			DBG_ASSERT(bb->branchid == -1);
		}
		bb = BasicBlock_FindById(kctx, bb->nextid);
	}
	bb = BasicBlock_FindById(kctx, blockId);
	while(bb != NULL) {
		if(bb->branchid != -1) {
			BasicBlock *bbJ = BasicBlock_FindById(kctx, bb->branchid);
			if(bbJ->codeoffset == -1) {
				BasicBlock_WriteByteCode(kctx, bb->branchid, writer);
			}
		}
		bb = BasicBlock_FindById(kctx, bb->nextid);
	}
}

static bool Block_HasTerminatorInst(KonohaContext *kctx, bblock_t blockId)
{
	BasicBlock *bb = BasicBlock_FindById(kctx, blockId);
	unsigned size = BasicBlock_size(bb);
	KVirtualCode *code, *lastInst;
	if(size == 0)
		return false;
	code = (KVirtualCode *)(((char *)bb) + sizeof(BasicBlock));
	lastInst = code + (size - 1);
	switch(lastInst->opcode) {
#define CASE(OP) case OPCODE_##OP: return true;
		CASE(EXIT);
		CASE(RET);
		CASE(JMP);
		CASE(JMPF);
		CASE(YIELD);
		CASE(ERROR);
#undef CASE
	}
	return false;
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

#define BasicBlock_isVisited(bb)  (bb->incoming == -1)
#define BasicBlock_SetVisited(bb)  bb->incoming =  -1
#define BasicBlock_size(bb)  ((bb)->size - sizeof(BasicBlock))

static void BasicBlock_SetJumpAddr(KonohaContext *kctx, BasicBlock *bb, char *vcode)
{
	while(bb != NULL) {
		BasicBlock_SetVisited(bb);
		if(bb->branchid != -1) {
			BasicBlock   *bbJ = BasicBlock_leapJump(kctx, BasicBlock_FindById(kctx, bb->branchid));
			KVirtualCode *op  = (KVirtualCode *)(vcode + bb->lastoffset);

			OPJMP *j = (OPJMP *) op;
			DBG_ASSERT(j->opcode == OPCODE_JMP || j->opcode == OPCODE_JMPF);
			j->jumppc = (KVirtualCode *)(vcode + bbJ->codeoffset);
			if(BasicBlock_size(bb) > 1) {
				KVirtualCode *opPREV = (KVirtualCode *)(vcode + bb->lastoffset - sizeof(KVirtualCode));
				if(opPREV->opcode == OPCODE_JMPF && bb->nextid != -1) {
					BasicBlock *block = BasicBlock_leapJump(kctx, BasicBlock_FindById(kctx, bb->nextid));
					((OPJMPF *)opPREV)->jumppc = (KVirtualCode *)(vcode + block->codeoffset);
				}
			}
			bbJ = BasicBlock_FindById(kctx, bb->branchid);
			if(!BasicBlock_isVisited(bbJ)) {
				BasicBlock_SetVisited(bbJ);
				BasicBlock_SetJumpAddr(kctx, bbJ, vcode);
			}
		}
		bb = BasicBlock_FindById(kctx, bb->nextid);
	}
}

static bblock_t new_BasicBlockLABEL(KonohaContext *kctx)
{
	BasicBlock *bb = new_BasicBlock(kctx, sizeof(KVirtualCode) * 2 + sizeof(BasicBlock), -1);
	return BasicBlock_id(kctx, bb);
}

#define ASM(T, ...) do {\
	OP##T op_ = {OP_(T), ## __VA_ARGS__};\
	union { KVirtualCode op; OP##T op_; } tmp_; tmp_.op_ = op_; \
	KBuilder_Asm(kctx, builder, &tmp_.op, sizeof(OP##T));\
} while(0)

#define NC_(sfpidx)    ((sfpidx) * 2 + 1)
#define OC_(sfpidx)    ((sfpidx) * 2 + 0)
#define TC_(sfpidx, C) ((KClass_Is(UnboxType, C)) ? NC_(sfpidx) : OC_(sfpidx))
#define SFP_(sfpidx)   ((sfpidx) * 2)

#define KBuilder_uline(builder) ((builder)->common.uline)

static void KBuilder_Asm(KonohaContext *kctx, KBuilder *builder, KVirtualCode *op, size_t opsize)
{
	builder->bbMainId = BasicBlock_Add(kctx, builder->bbMainId, KBuilder_uline(builder), op, opsize, sizeof(KVirtualCode));
	builder->InstructionSize += 1;
}

static void kUntypedNode_SetLabelBlock(KonohaContext *kctx, kUntypedNode *node, ksymbol_t label, bblock_t labelId)
{
	KLIB kObjectProto_SetUnboxValue(kctx, node, label, KType_Int, labelId);
}

static bblock_t kUntypedNode_GetLabelNode(KonohaContext *kctx, kUntypedNode *node, ksymbol_t label)
{
	KKeyValue *kvs = KLIB kObjectProto_GetKeyValue(kctx, node, label);
	if(kvs != NULL) {
		return (bblock_t)kvs->unboxValue;
	}
	return -1;
}

/*----------------------------------------------------------------------------*/
/* Visitor */

static intptr_t MiniVM_getExpression(KBuilder *builder)
{
	intptr_t Value = builder->Value;
	builder->Value = -1;
	assert(Value != -1);
	return Value;
}

static kObject *MiniVM_AddConstPool(KonohaContext *kctx, KBuilder *builder, kObject *o)
{
	KLIB kArray_Add(kctx, builder->constPools, o);
	return o;
}

static void ASM_NMOV(KonohaContext *kctx, KBuilder *builder, KClass *Ty, int dst, int src)
{
	if(dst != src) {
		ASM(NMOV, TC_(dst, Ty), TC_(src, Ty), Ty);
	}
}

static void MiniVMBuilder_JumpTo(KonohaContext *kctx, KBuilder *builder, bblock_t labelId)
{
	BasicBlock *bb = BasicBlock_FindById(kctx, builder->bbMainId);
	DBG_ASSERT(bb != NULL);
	//DBG_ASSERT(bb->nextid == -1);
	if(bb->branchid == -1) {
		BasicBlock *labelNode;
		ASM(JMP, NULL);
		labelNode = BasicBlock_FindById(kctx, labelId);
		bb = BasicBlock_FindById(kctx, builder->bbMainId);
		bb->branchid = BasicBlock_id(kctx, labelNode);
		labelNode->incoming += 1;
	}
}

static void CreateUpdate(KonohaContext *kctx, KBuilder *builder, ktypeattr_t type, intptr_t dst, intptr_t src)
{
	ASM_NMOV(kctx, builder, KClass_(type), dst, src);
}

static void CreateBranch(KonohaContext *kctx, KBuilder *builder, intptr_t src, bblock_t ThenBB, bblock_t ElseBB, bool emitJumpToThenBlock)
{
	BasicBlock *ThenBlock, *ElseBlock;
	BasicBlock *bb = BasicBlock_FindById(kctx, builder->bbMainId);
	DBG_ASSERT(bb != NULL);
	DBG_ASSERT(/*bb->nextid == -1 &&*/ bb->branchid == -1);
	ASM(JMPF, NULL, NC_(src));
	if(emitJumpToThenBlock) {
		ASM(JMP, NULL);
	}
	bb = BasicBlock_FindById(kctx, builder->bbMainId);
	ThenBlock = BasicBlock_FindById(kctx, ThenBB);
	ElseBlock = BasicBlock_FindById(kctx, ElseBB);
	bb->branchid = ElseBB;
	bb->nextid   = ThenBB;
	ThenBlock->incoming += 1;
	ElseBlock->incoming += 1;
}

static void MiniVMBuilder_setBlock(KonohaContext *kctx, KBuilder *builder, bblock_t labelId)
{
	BasicBlock *labelNode;
	BasicBlock *bb = BasicBlock_FindById(kctx, builder->bbMainId);
	DBG_ASSERT(bb != NULL);
	labelNode = BasicBlock_FindById(kctx, labelId);
	labelNode->incoming += 1;
	builder->bbMainId = BasicBlock_id(kctx, labelNode);
}

typedef struct LocalVarInfo {
	intptr_t index;
	ksymbol_t   sym;
	ktypeattr_t type;
} LocalVarInfo;

#ifndef LOG2
#define LOG2(N) ((unsigned)((sizeof(void *) * 8) - CLZ((N) - 1)))
#endif

static intptr_t AddLocal(KonohaContext *kctx, KBuilder *builder, intptr_t index, ksymbol_t sym, ktypeattr_t type)
{
	LocalVarInfo *newlocal;
	int offset = builder->localVar.bytesize / sizeof(LocalVarInfo);
	if(builder->localVar.bytesize == builder->localVar.bytemax) {
		size_t newsize = sizeof(LocalVarInfo) * (1 << LOG2(offset * 2 + 1));
		KLIB KArray_Expand(kctx, &builder->localVar, newsize);
	}
	newlocal = &((LocalVarInfo *)builder->localVar.bytebuf)[offset];
	newlocal->index = index;
	newlocal->sym   = sym;
	newlocal->type  = type;
	builder->localVar.bytesize += sizeof(LocalVarInfo);
	return index;

}

static intptr_t MiniVMBuilder_FindLocalVar(KonohaContext *kctx, KBuilder *builder, ksymbol_t symbol, ktypeattr_t typeId, intptr_t index)
{
	unsigned i, localSize;
	kParam *params;
	kMethod *mtd = builder->currentMtd;
	if(mtd->typeId != KType_void && mtd->typeId == typeId && index == 0) {
		return 0;
	}

	params = kMethod_GetParam(mtd);
	for(i = 0; i < params->psize; ++i) {
		ktypeattr_t ptype = params->paramtypeItems[i].typeAttr;
		if(ptype == typeId && index == i ) {
			return i;
		}
	}

	localSize = builder->localVar.bytesize / sizeof(LocalVarInfo);
	for(i = 0; i < localSize; i++) {
		LocalVarInfo *local = &((LocalVarInfo *)builder->localVar.bytebuf)[i];
		if(local->index == index && local->type == typeId && local->sym == symbol) {
			return local->index;
		}
	}
	return -1;
}

/*----------------------------------------------------------------------------*/
/* Visitor API */
static kbool_t MiniVM_VisitDoneNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	return true;
}

static kbool_t MiniVM_VisitBoxNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	/*
	 * [box] := box(this)
	 **/
	KLIB VisitNode(kctx, builder, node->NodeToPush, thunk);
	ASM(BOX, OC_(builder->stackbase), NC_(MiniVM_getExpression(builder)), KClass_(node->typeAttr));
	builder->Value = builder->stackbase;
	return true;
}

static kbool_t MiniVM_VisitPushNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	KLIB VisitNode(kctx, builder, node->NodeToPush, thunk);
	return true;
}

static kbool_t MiniVM_VisitErrorNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	kString *Err = kUntypedNode_getErrorMessage(kctx, node);
	ASM(ERROR, kUntypedNode_uline(node), Err);
	return false;
}

static kbool_t MiniVM_VisitThrowNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	assert(0 && "Not Implemented");
	return true;
}

static kbool_t MiniVM_VisitBlockNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *block, void *thunk)
{
	size_t i;
	bblock_t NewBlock = new_BasicBlockLABEL(kctx);
	MiniVMBuilder_JumpTo(kctx, builder, NewBlock);
	MiniVMBuilder_setBlock(kctx, builder, NewBlock);
	for(i = 0; i < kUntypedNode_GetNodeListSize(kctx, block); i++) {
		kUntypedNode *stmt = block->NodeList->NodeItems[i];
		builder->common.uline = kUntypedNode_uline(stmt);
		if(!KLIB VisitNode(kctx, builder, stmt, thunk))
			break;
	}
	return true;
}

static kbool_t MiniVM_VisitReturnNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	kUntypedNode *expr = KLIB kUntypedNode_GetNode(kctx, node, KSymbol_ExprPattern, NULL);
	if(expr != NULL && IS_Node(expr) && expr->typeAttr != KType_void) {
		KLIB VisitNode(kctx, builder, expr, thunk);
		ASM_NMOV(kctx, builder, KClass_(expr->typeAttr), K_RTNIDX, MiniVM_getExpression(builder));
	}
	MiniVMBuilder_JumpTo(kctx, builder, builder->bbReturnId);
	return false;
}

static kbool_t MiniVM_VisitIfNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	bblock_t ThenBB  = new_BasicBlockLABEL(kctx);
	bblock_t ElseBB  = new_BasicBlockLABEL(kctx);
	bblock_t MergeBB = new_BasicBlockLABEL(kctx);
	/* if */
	KLIB VisitNode(kctx, builder, kUntypedNode_getFirstNode(kctx, node), thunk);
	CreateBranch(kctx, builder, MiniVM_getExpression(builder), ThenBB, ElseBB, false);
	{ /* then */
		MiniVMBuilder_setBlock(kctx, builder, ThenBB);
		KLIB VisitNode(kctx, builder, kUntypedNode_getFirstBlock(kctx, node), thunk);
		if(!Block_HasTerminatorInst(kctx, builder->bbMainId))
			MiniVMBuilder_JumpTo(kctx, builder, MergeBB);
	}
	{ /* else */
		MiniVMBuilder_setBlock(kctx, builder, ElseBB);
		KLIB VisitNode(kctx, builder, kUntypedNode_getElseBlock(kctx, node), thunk);
		if(!Block_HasTerminatorInst(kctx, builder->bbMainId))
			MiniVMBuilder_JumpTo(kctx, builder, MergeBB);
	}
	/* endif */
	MiniVMBuilder_setBlock(kctx, builder, MergeBB);
	return true;
}

enum LoopType {
	WhileLoop,
	DoWhileLoop,
	ForLoop
};

static kbool_t MiniVM_VisitLoopNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk, enum LoopType Loop)
{
	kUntypedNode *ItrBlock;
	bblock_t HeadBB  = new_BasicBlockLABEL(kctx);
	bblock_t BodyBB  = new_BasicBlockLABEL(kctx);
	bblock_t ItrBB   = new_BasicBlockLABEL(kctx);
	bblock_t MergeBB = new_BasicBlockLABEL(kctx);

	kUntypedNode_SetLabelBlock(kctx, node, KSymbol_("continue"), ItrBB);
	kUntypedNode_SetLabelBlock(kctx, node, KSymbol_("break"),    MergeBB);

	ItrBlock = KLIB kUntypedNode_GetNode(kctx, node, KSymbol_("Iterator"), NULL);
	if(ItrBlock != NULL) {
		assert(Loop == ForLoop);
		MiniVMBuilder_JumpTo(kctx, builder, HeadBB);
	}
	else if(Loop == WhileLoop) {
		MiniVMBuilder_JumpTo(kctx, builder, HeadBB);
	} else {
		MiniVMBuilder_JumpTo(kctx, builder, BodyBB);
	}

	{ /* Head */
		MiniVMBuilder_setBlock(kctx, builder, HeadBB);
		KLIB VisitNode(kctx, builder, kUntypedNode_getFirstNode(kctx, node), thunk);
		CreateBranch(kctx, builder, MiniVM_getExpression(builder), BodyBB, MergeBB, false);
	}

	{ /* Body */
		MiniVMBuilder_setBlock(kctx, builder, BodyBB);
		KLIB VisitNode(kctx, builder, kUntypedNode_getFirstBlock(kctx, node), thunk);
		MiniVMBuilder_JumpTo(kctx, builder, ItrBB);

		/* Itr */
		MiniVMBuilder_setBlock(kctx, builder, ItrBB);
		if(ItrBlock != NULL) {
			KLIB VisitNode(kctx, builder, ItrBlock, thunk);
		}
		MiniVMBuilder_JumpTo(kctx, builder, HeadBB);
	}

	MiniVMBuilder_setBlock(kctx, builder, MergeBB);
	return true;
}

static kbool_t MiniVM_VisitWhileNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	return MiniVM_VisitLoopNode(kctx, builder, node, thunk, WhileLoop);
}

static kbool_t MiniVM_VisitDoWhileNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	bblock_t HeadBB  = new_BasicBlockLABEL(kctx);
	bblock_t BodyBB  = new_BasicBlockLABEL(kctx);
	bblock_t MergeBB = new_BasicBlockLABEL(kctx);

	kUntypedNode_SetLabelBlock(kctx, node, KSymbol_("continue"), HeadBB);
	kUntypedNode_SetLabelBlock(kctx, node, KSymbol_("break"),    MergeBB);

	MiniVMBuilder_JumpTo(kctx, builder, BodyBB);
	{ /* Head */
		MiniVMBuilder_setBlock(kctx, builder, HeadBB);
		KLIB VisitNode(kctx, builder, kUntypedNode_getFirstNode(kctx, node), thunk);
		CreateBranch(kctx, builder, MiniVM_getExpression(builder), MergeBB, BodyBB, true);
	}

	{ /* Body */
		MiniVMBuilder_setBlock(kctx, builder, BodyBB);
		KLIB VisitNode(kctx, builder, kUntypedNode_getFirstBlock(kctx, node), thunk);
		MiniVMBuilder_JumpTo(kctx, builder, HeadBB);
	}

	MiniVMBuilder_setBlock(kctx, builder, MergeBB);
	return true;
}

static kbool_t MiniVM_VisitForNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	kUntypedNode *initNode = kUntypedNode_GetNode(kctx, node, KSymbol_("init"));
	if(initNode != NULL) {
		KLIB VisitNode(kctx, builder, initNode, thunk);
	}
	return MiniVM_VisitLoopNode(kctx, builder, node, thunk, ForLoop);
}

static kbool_t MiniVM_VisitJumpNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk, ksymbol_t label)
{
	bblock_t target;
	kUntypedNode *jump = kUntypedNode_GetNode(kctx, node, label);
	DBG_ASSERT(jump != NULL && IS_Node(jump));
	target = kUntypedNode_GetLabelNode(kctx, jump, label);
	MiniVMBuilder_JumpTo(kctx, builder, target);
	return true;
}

static kbool_t MiniVM_VisitContinueNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	return MiniVM_VisitJumpNode(kctx, builder, node, thunk, KSymbol_("continue"));
}

static kbool_t MiniVM_VisitBreakNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	return MiniVM_VisitJumpNode(kctx, builder, node, thunk, KSymbol_("break"));
}

static kbool_t MiniVM_VisitTryNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	assert(0 && "TODO");
	return true;
}

static kbool_t MiniVM_VisitConstNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	kObject *v = MiniVM_AddConstPool(kctx, builder, node->ObjectConstValue);
	DBG_ASSERT(!KType_Is(UnboxType, node->typeAttr));
	ASM(NSET, OC_(builder->stackbase), (uintptr_t) v, KClass_(node->typeAttr));
	builder->Value = builder->stackbase;
	return true;
}

static kbool_t MiniVM_VisitUnboxConstNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	uintptr_t Val = node->unboxConstValue;
	DBG_ASSERT(KType_Is(UnboxType, node->typeAttr));
	ASM(NSET, NC_(builder->stackbase), Val, KClass_(node->typeAttr));
	builder->Value = builder->stackbase;
	return true;
}

static kbool_t MiniVM_VisitNewNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	ASM(NEW, OC_(builder->stackbase), 0, KClass_(node->typeAttr));
	builder->Value = builder->stackbase;
	return true;
}

static kbool_t MiniVM_VisitNullNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	if(KType_Is(UnboxType, node->typeAttr)) {
		ASM(NSET, NC_(builder->stackbase), 0, KClass_(node->typeAttr));
	} else {
		ASM(NUL, OC_(builder->stackbase), KClass_(node->typeAttr));
	}
	builder->Value = builder->stackbase;
	return true;
}

static kbool_t MiniVM_VisitLocalNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	intptr_t src;
	ksymbol_t symbol;
	assert(IS_Token(node->TermToken));
	symbol = node->TermToken->symbol;
	if((src = MiniVMBuilder_FindLocalVar(kctx, builder, symbol, node->typeAttr, node->index)) == -1) {
		src = AddLocal(kctx, builder, node->index, symbol, node->typeAttr);
	}
	builder->Value = src;
	return true;
}

static kbool_t MiniVM_VisitFieldNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	ksymbol_t symbol;
	intptr_t src;
	KClass *ty;

	khalfword_t index  = (khalfword_t)(node->index);
	khalfword_t xindex = (khalfword_t)(node->index >> (sizeof(khalfword_t)*8));
	assert(IS_Token(node->TermToken));
	symbol = node->TermToken->symbol;
	if((src = MiniVMBuilder_FindLocalVar(kctx, builder, symbol, KType_Object, index)) == -1) {
		src = AddLocal(kctx, builder, index, symbol, KType_Object);
	}
	ty = KClass_(node->typeAttr);
	ASM(NMOVx, TC_(builder->stackbase, ty), OC_(src), xindex, ty);
	builder->Value = builder->stackbase;
	return true;
}

static kbool_t MiniVM_VisitMethodCallNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	kMethod *mtd = CallNode_getMethod(node);

	/*
	 * [CallExpr] := this.method(arg1, arg2, ...)
	 * node->NodeList = [method, this, arg1, arg2, ...]
	 **/
	intptr_t stackbase, thisidx;
	int i, s, argc = CallNode_getArgCount(node);
	DBG_ASSERT(IS_Method(mtd));

	s = kMethod_Is(Static, mtd) ? 2 : 1;
	if(kMethod_Is(Static, mtd)) {
		KClass *selfTy = KClass_(mtd->typeId);
		kObject *obj = KLIB Knull(kctx, selfTy);
		ASM(NSET, OC_(builder->stackbase), (uintptr_t) obj, selfTy);
	}

	stackbase = builder->stackbase;
	thisidx = stackbase + K_CALLDELTA;
	for (i = s; i < argc + 2; i++) {
		kUntypedNode *exprN = kUntypedNode_At(node, i);
		builder->stackbase = thisidx + i - 1;
		DBG_ASSERT(IS_Node(exprN));
		KLIB VisitNode(kctx, builder, exprN, thunk);
		if(builder->Value != builder->stackbase) {
			KClass *ty = KClass_(exprN->typeAttr);
			ASM_NMOV(kctx, builder, ty, builder->stackbase, builder->Value);
		}
	}

	if(kMethod_Is(Final, mtd) || !kMethod_Is(Virtual, mtd)) {
		ASM(NSET, NC_(thisidx-1), (intptr_t)mtd, KClass_Method);
		if(kMethod_Is(Virtual, mtd)) {
			// set namespace to enable method lookups
			ASM(NSET, OC_(thisidx-2), (intptr_t)kUntypedNode_ns(node), KClass_NameSpace);
		}
	}
	else {
		ASM(NSET, OC_(thisidx-2), (intptr_t)kUntypedNode_ns(node), KClass_NameSpace);
		ASM(LOOKUP, SFP_(thisidx), kUntypedNode_ns(node), mtd);
	}
	ASM(CALL, kUntypedNode_uline(node), SFP_(thisidx), SFP_(thisidx + argc + 1), KLIB Knull(kctx, KClass_(node->typeAttr)));

	builder->stackbase = stackbase;
	builder->Value = builder->stackbase;
	return true;
}

enum ConditionalOp {
	LogicalOr, LogicalAnd
};

static void CreateCond(KonohaContext *kctx, KBuilder *builder, kUntypedNode *expr, enum ConditionalOp Op, void *thunk)
{
	intptr_t cond;
	kUntypedNode *LHS = kUntypedNode_At(expr, 1);
	kUntypedNode *RHS = kUntypedNode_At(expr, 2);

	bblock_t HeadBB  = new_BasicBlockLABEL(kctx);
	bblock_t ThenBB  = new_BasicBlockLABEL(kctx);
	bblock_t MergeBB = new_BasicBlockLABEL(kctx);

	/* [CondExpr]
	 * LogicalAnd case
	 *       | goto Head
	 * Head  | let bval = LHS
	 *       | if(bval) { goto Then } else { goto Merge }
	 * Then  | bval = RHS
	 *       | goto Merge
	 * Merge | ...
	 */

	MiniVMBuilder_JumpTo(kctx, builder, HeadBB);
	cond = builder->stackbase;
	builder->stackbase += 1;
	{ /* Head */
		MiniVMBuilder_setBlock(kctx, builder, HeadBB);
		KLIB VisitNode(kctx, builder, LHS, thunk);
		CreateUpdate(kctx, builder, KType_Boolean, cond, MiniVM_getExpression(builder));
		if(Op == LogicalAnd)
			CreateBranch(kctx, builder, cond, ThenBB, MergeBB, false);
		else {
			CreateBranch(kctx, builder, cond, ThenBB, MergeBB, true);
		}
	}
	{ /* Then */
		MiniVMBuilder_setBlock(kctx, builder, ThenBB);
		KLIB VisitNode(kctx, builder, RHS, thunk);
		CreateUpdate(kctx, builder, KType_Boolean, cond, MiniVM_getExpression(builder));
		MiniVMBuilder_JumpTo(kctx, builder, MergeBB);
	}

	MiniVMBuilder_setBlock(kctx, builder, MergeBB);
	builder->stackbase -= 1;
	builder->Value = builder->stackbase;
}

static kbool_t MiniVM_VisitAndNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	CreateCond(kctx, builder, node, LogicalAnd, thunk);
	return true;
}

static kbool_t MiniVM_VisitOrNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	CreateCond(kctx, builder, node, LogicalOr, thunk);
	return true;
}

static kbool_t MiniVM_VisitAssignNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	/*
	 * [LetExpr] := lhs = rhs
	 * expr->NodeList = [NULL, lhs, rhs]
	 **/

	intptr_t dst = -1;
	kUntypedNode *left = kUntypedNode_At(node, 1);
	kUntypedNode *right = kUntypedNode_At(node, 2);
	ktypeattr_t type = left->typeAttr;
	ksymbol_t symbol;

	assert(IS_Token(left->TermToken));
	symbol = left->TermToken->symbol;
	if(kUntypedNode_node(left) == KNode_Local) {
		if((dst = MiniVMBuilder_FindLocalVar(kctx, builder, symbol, type, left->index)) == -1) {
			dst = AddLocal(kctx, builder, left->index, symbol, type);
		}
		KLIB VisitNode(kctx, builder, right, thunk);
		CreateUpdate(kctx, builder, type, dst, MiniVM_getExpression(builder));
		builder->Value = dst;
	}
	else {
		khalfword_t index  = (khalfword_t)left->index;
		khalfword_t xindex = (khalfword_t)(left->index >> (sizeof(khalfword_t)*8));
		KClass *lhsClass = KClass_(left->typeAttr);
		KClass *rhsClass = KClass_(right->typeAttr);
		assert(kUntypedNode_node(left) == KNode_Field);
		if((dst = MiniVMBuilder_FindLocalVar(kctx, builder, symbol, KType_Object, index)) == -1) {
			dst = AddLocal(kctx, builder, index, symbol, KType_Object);
		}
		KLIB VisitNode(kctx, builder, right, thunk);
		ASM(XNMOV, OC_(index), xindex, TC_(MiniVM_getExpression(builder), rhsClass), lhsClass);
		if(node->typeAttr != KType_void) {
			builder->Value = builder->stackbase;
			ASM(NMOVx, TC_(builder->stackbase, rhsClass), OC_(index), xindex, lhsClass);
		}
	}
	return true;
}

static kbool_t MiniVM_VisitFunctionNode(KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk)
{
	/*
	 * [FunctionExpr] := new Function(method, env1, env2, ...)
	 * expr->NodeList = [method, defObj, env1, env2, ...]
	 **/
	assert(0 && "FIXME");
	return false;
}

/* end of Visitor */
/*----------------------------------------------------------------------------*/
/* begin of Local Variable Analysis Visitor */

#define KVISITOR_PARAM    KonohaContext *kctx, KBuilder *builder, kUntypedNode *node, void *thunk
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define LOCAL(TYPE, INDEX) (builder->localVarSize = MAX(builder->localVarSize, INDEX))
static kbool_t CollectLocalVar_Visit_NotImplemented(KVISITOR_PARAM)
{
	assert(0 && "NotImplemented");
	return false;
}
static kbool_t CollectLocalVar_VisitDoneNode(KVISITOR_PARAM)
{
	return true;
}
static kbool_t CollectLocalVar_VisitPushNode(KVISITOR_PARAM)
{
	KLIB VisitNode(kctx, builder, node->NodeToPush, thunk);
	return true;
}

#define CollectLocalVar_VisitBoxNode   CollectLocalVar_VisitPushNode
#define CollectLocalVar_VisitErrorNode CollectLocalVar_VisitDoneNode

#define CollectLocalVar_VisitThrowNode CollectLocalVar_Visit_NotImplemented

static kbool_t CollectLocalVar_VisitBlockNode(KVISITOR_PARAM)
{
	unsigned i;
	for(i = 0; i < kUntypedNode_GetNodeListSize(kctx, node); i++) {
		kUntypedNode *stmt = node->NodeList->NodeItems[i];
		if(!KLIB VisitNode(kctx, builder, stmt, thunk))
			break;
	}
	return true;
}

static kbool_t CollectLocalVar_VisitReturnNode(KVISITOR_PARAM)
{
	kUntypedNode *expr = KLIB kUntypedNode_GetNode(kctx, node, KSymbol_ExprPattern, NULL);
	if(expr != NULL && IS_Node(expr) && expr->typeAttr != KType_void) {
		KLIB VisitNode(kctx, builder, expr, thunk);
	}
	return false;
}

static kbool_t CollectLocalVar_VisitIfNode(KVISITOR_PARAM)
{
	KLIB VisitNode(kctx, builder, kUntypedNode_getFirstNode(kctx, node), thunk);
	KLIB VisitNode(kctx, builder, kUntypedNode_getFirstBlock(kctx, node), thunk);
	KLIB VisitNode(kctx, builder, kUntypedNode_getElseBlock(kctx, node), thunk);
	return true;
}

static kbool_t CollectLocalVar_VisitWhileNode(KVISITOR_PARAM)
{
	kUntypedNode *ItrBlock;
	KLIB VisitNode(kctx, builder, kUntypedNode_getFirstNode(kctx, node), thunk);
	KLIB VisitNode(kctx, builder, kUntypedNode_getFirstBlock(kctx, node), thunk);
	ItrBlock = KLIB kUntypedNode_GetNode(kctx, node, KSymbol_("Iterator"), NULL);
	if(ItrBlock != NULL) {
		KLIB VisitNode(kctx, builder, ItrBlock, thunk);
	}
	return true;
}

#define CollectLocalVar_VisitDoWhileNode CollectLocalVar_VisitWhileNode

static kbool_t CollectLocalVar_VisitForNode(KVISITOR_PARAM)
{
	kUntypedNode *initNode = kUntypedNode_GetNode(kctx, node, KSymbol_("init"));
	if(initNode != NULL) {
		KLIB VisitNode(kctx, builder, initNode, thunk);
	}
	CollectLocalVar_VisitWhileNode(kctx, builder, node, thunk);
	return true;
}

#define CollectLocalVar_VisitTryNode        CollectLocalVar_Visit_NotImplemented
#define CollectLocalVar_VisitContinueNode   CollectLocalVar_VisitDoneNode
#define CollectLocalVar_VisitBreakNode      CollectLocalVar_VisitDoneNode
#define CollectLocalVar_VisitConstNode      CollectLocalVar_VisitDoneNode
#define CollectLocalVar_VisitUnboxConstNode CollectLocalVar_VisitDoneNode
#define CollectLocalVar_VisitNewNode        CollectLocalVar_VisitDoneNode
#define CollectLocalVar_VisitNullNode       CollectLocalVar_VisitDoneNode

static kbool_t CollectLocalVar_VisitLocalNode(KVISITOR_PARAM)
{
	LOCAL(node->typeAttr, (khalfword_t)(node->index));
	return true;
}

static kbool_t CollectLocalVar_VisitFieldNode(KVISITOR_PARAM)
{
	LOCAL(KType_Object, (khalfword_t)(node->index));
	return true;
}

static kbool_t CollectLocalVar_VisitMethodCallNode(KVISITOR_PARAM)
{
	kMethod *mtd = CallNode_getMethod(node);
	int i, s = kMethod_Is(Static, mtd) ? 2 : 1;
	int argc = CallNode_getArgCount(node);
	for (i = s; i < argc + 2; i++) {
		kUntypedNode *exprN = kUntypedNode_At(node, i);
		KLIB VisitNode(kctx, builder, exprN, thunk);
	}
	return true;
}

static kbool_t CollectLocalVar_VisitAndNode(KVISITOR_PARAM)
{
	KLIB VisitNode(kctx, builder, kUntypedNode_At(node, 1), thunk);
	KLIB VisitNode(kctx, builder, kUntypedNode_At(node, 2), thunk);
	return true;
}

#define CollectLocalVar_VisitOrNode CollectLocalVar_VisitAndNode

static kbool_t CollectLocalVar_VisitAssignNode(KVISITOR_PARAM)
{
	kUntypedNode *left = kUntypedNode_At(node, 1);
	kUntypedNode *right = kUntypedNode_At(node, 2);
	if(kUntypedNode_node(left) == KNode_Local) {
		LOCAL(left->typeAttr, left->index);
		KLIB VisitNode(kctx, builder, right, thunk);
	} else{
		LOCAL(KType_Object, (khalfword_t)left->index);
		KLIB VisitNode(kctx, builder, right, thunk);
	}
	return true;
}

static kbool_t CollectLocalVar_VisitFunctionNode(KVISITOR_PARAM)
{
	size_t i, ParamSize = kArray_size(node->NodeList)-2;
	for(i = 0; i < ParamSize; i++) {
		kUntypedNode *expr = kUntypedNode_At(node, i+2);
		KLIB VisitNode(kctx, builder, expr, thunk);
	}
	return true;
}

#define DEFINE_LOCALVAR_COLLECTOR_API(NAME) CollectLocalVar_Visit##NAME##Node,
static const struct KBuilderAPI CollectLocalVar_BuilderAPI = {
	"CollectLocalVar",
	NULL,
	KNodeList(DEFINE_LOCALVAR_COLLECTOR_API)
};

/* end of Local Variable Analysis Visitor */
/*----------------------------------------------------------------------------*/

static void FreeVirtualCode(KonohaContext *kctx, struct KVirtualCode *vcode)
{
	OPTHCODE *opTHCODE = (OPTHCODE *)(vcode - 1);
	if(opTHCODE->opcode == OPCODE_THCODE && opTHCODE->codesize > 0) {
		KFree(opTHCODE, opTHCODE->codesize);
	}
}

static struct KVirtualCodeAPI vapi = {
		FreeVirtualCode, WriteVirtualCode
};

static struct KVirtualCode *MakeThreadedCode(KonohaContext *kctx, KBuilder *builder, KVirtualCode *vcode, size_t codesize)
{
	struct KVirtualCodeAPI **p;
	OPTHCODE *opTHCODE = (OPTHCODE *)vcode;
	opTHCODE->codesize = codesize;
	p = (struct KVirtualCodeAPI **)builder->common.api->ExecutionEngineModule->RunExecutionEngine(kctx, kctx->esp + 1, vcode);
	p[-1] = &vapi;
	return (KVirtualCode *)p;
}

static struct KVirtualCode *CompileVirtualCode(KonohaContext *kctx, KBuilder *builder, bblock_t beginId, bblock_t returnId)
{
	KVirtualCode *vcode;
	ByteCodeWriter writer = {NULL, 0, builder->InstructionSize * sizeof(KVirtualCode)};
	BasicBlock_Optimize(kctx, beginId, &writer);
	DBG_P(">>>>>> codesize=%d", writer.codesize);
	DBG_ASSERT(writer.codesize != 0);
	vcode = (KVirtualCode *)KCalloc_UNTRACE(writer.codesize, 1);
	writer.current = vcode;
	BasicBlock_WriteByteCode(kctx, beginId, &writer);
	BasicBlock_WriteByteCode(kctx, returnId, &writer);
	BasicBlock_SetJumpAddr(kctx, BasicBlock_FindById(kctx, beginId), (char *)vcode);
	vcode = MakeThreadedCode(kctx, builder, vcode, writer.codesize);
	DumpVirtualCode(kctx, vcode);
	return vcode;
}

static void _THCODE(KonohaContext *kctx, KVirtualCode *pc, void **codeaddr, size_t codesize)
{
#ifdef USE_DIRECT_THREADED_CODE
	size_t i, n = codesize / sizeof(KVirtualCode);
	for(i = 0; i < n; i++) {
		pc->codeaddr = codeaddr[pc->opcode];
		pc++;
	}
#endif
}

static struct KVirtualCode *MiniVM_GenerateVirtualCode(KonohaContext *kctx, kMethod *mtd, kUntypedNode *block, int option)
{
	KVirtualCode *vcode;
	KBuffer wb;
	KBuilder builderbuf = {{0}}, *builder = &builderbuf;
	kNameSpace *ns = kUntypedNode_ns(block);

	INIT_GCSTACK();
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	builder->common.api = ns->builderApi;
	builder->constPools = ns->NameSpaceConstList;
	builder->bbBeginId  = new_BasicBlockLABEL(kctx);
	builder->bbReturnId = new_BasicBlockLABEL(kctx);
	builder->bbMainId   = builder->bbBeginId;
	builder->currentMtd = mtd;
	builder->Value      = 0;
	builder->stackbase  = 0;
	builder->InstructionSize = 0;

	builder->common.api = &CollectLocalVar_BuilderAPI;
	KLIB VisitNode(kctx, builder, block, NULL);
	builder->stackbase += builder->localVarSize + 1/* == this object */;
	builder->common.api = ns->builderApi;

	KLIB KArray_Init(kctx, &builder->localVar, sizeof(LocalVarInfo) * builder->stackbase);

	ASM(THCODE, 0, _THCODE);
	ASM(CHKSTACK, 0);

	KLIB VisitNode(kctx, builder, block, NULL);

	if(!Block_HasTerminatorInst(kctx, builder->bbMainId)) {
		MiniVMBuilder_JumpTo(kctx, builder, builder->bbReturnId);
	}

	MiniVMBuilder_setBlock(kctx, builder, builder->bbReturnId);
	if(mtd->mn == MN_new) {
		// FIXME: Type 'This' must be resolved
		ASM(NMOV, OC_(K_RTNIDX), OC_(0), KClass_(mtd->typeId));
	}
	ASM(RET);
	vcode = CompileVirtualCode(kctx, builder, builder->bbBeginId, builder->bbReturnId);

	KLIB KArray_Free(kctx, &builder->localVar);
	RESET_GCSTACK();
	KLIB KBuffer_Free(&wb);
	return vcode;

}

static struct KVirtualCode *BOOTCODE_ENTER = NULL;
static struct KVirtualCode *BOOTCODE_NCALL = NULL;

static void SetUpBootCode(void)
{
	if(BOOTCODE_ENTER == NULL) {
		static struct KVirtualCode InitCode[6] = {{0}};
		struct OPTHCODE thcode = {OP_(THCODE), 4 * sizeof(KVirtualCode), _THCODE};
		struct OPNCALL ncall = {OP_(NCALL)};
		struct OPENTER enter = {OP_(ENTER)};
		struct OPEXIT  exit  = {OP_(EXIT)};
		KVirtualCode *pc;
		memcpy(InitCode+0, &thcode, sizeof(OPTHCODE));
		memcpy(InitCode+1, &ncall,  sizeof(OPNCALL));
		memcpy(InitCode+2, &enter,  sizeof(OPENTER));
		memcpy(InitCode+3, &exit,   sizeof(OPEXIT));
		pc = MiniVM_RunVirtualMachine(NULL, NULL, InitCode);
		BOOTCODE_NCALL = pc;
		BOOTCODE_ENTER = pc+1;
	}
}

static struct KVirtualCode *GetDefaultBootCode(void)
{
	return BOOTCODE_NCALL;
}

static KMETHOD KMethodFunc_RunVirtualMachine(KonohaContext *kctx, KonohaStack *sfp)
{
	DBG_ASSERT(IS_Method(sfp[K_MTDIDX].calledMethod));
	MiniVM_RunVirtualMachine(kctx, sfp, BOOTCODE_ENTER);
}

static KMethodFunc MiniVM_GenerateMethodFunc(KonohaContext *kctx, KVirtualCode *vcode)
{
	return KMethodFunc_RunVirtualMachine;
}

static void MiniVM_SetMethodCode(KonohaContext *kctx, kMethodVar *mtd, struct KVirtualCode *vcode, KMethodFunc func)
{
	/* already compiled */
	if(mtd->invokeKMethodFunc == KMethodFunc_RunVirtualMachine) {
		mtd->virtualCodeApi_plus1[-1]->FreeVirtualCode(kctx, mtd->vcode_start);
	}
	KLIB kMethod_SetFunc(kctx, mtd, func);
	mtd->vcode_start = vcode;
}

static void MiniVM_DeleteVirtualMachine(KonohaContext *kctx)
{
}

static const KModuleInfo ModuleInfo = {
	"MiniVM", K_VERSION, 0, "MiniVM",
};

static const struct KBuilderAPI *GetDefaultBuilderAPI(void);

static const struct ExecutionEngineModule MiniVM_Module = {
	&ModuleInfo, "C",
	MiniVM_DeleteVirtualMachine,
	GetDefaultBuilderAPI,
	GetDefaultBootCode,
	MiniVM_GenerateVirtualCode,
	MiniVM_GenerateMethodFunc,
	MiniVM_SetMethodCode,
	MiniVM_RunVirtualMachine
};

static const struct KBuilderAPI MiniVM_BuilderAPI = {
	"MiniVM",
	&MiniVM_Module,
#define DEFINE_BUILDER_API(NAME) MiniVM_Visit##NAME##Node,
	KNodeList(DEFINE_BUILDER_API)
#undef DEFINE_BUILDER_API
};

static const struct KBuilderAPI *GetDefaultBuilderAPI(void)
{
	return &MiniVM_BuilderAPI;
}

// -------------------------------------------------------------------------

KONOHA_EXPORT(kbool_t) LoadMiniVMModule(KonohaFactory *factory, ModuleType type)
{
	if(BOOTCODE_ENTER == NULL) {
		SetUpBootCode();
	}
	memcpy(&factory->ExecutionEngineModule, &MiniVM_Module, sizeof(MiniVM_Module));
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
