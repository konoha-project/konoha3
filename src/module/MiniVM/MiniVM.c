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

#define USE_DIRECT_THREADED_CODE

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
		KLIB KBuffer_printf(kctx, wb, " (%s:%d)", PLATAPI shortFilePath(KFileLine_textFileName(uline)), (kshort_t)uline);
		break;
	}
	case VMT_R: {
		KLIB KBuffer_printf(kctx, wb, " sfp[%d,r=%d]", (int)c->data[i]/2, (int)c->data[i]);
		break;
	}
	case VMT_FX: {
		kshort_t index  = (kshort_t)c->data[i];
		kshort_t xindex = (kshort_t)(c->data[i] >> (sizeof(kshort_t)*8));
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
		//FIXME: Node unknown error
		//mtd = KLIB kNameSpace_GetMethodBySignatrueNULL(kctx, ns, kObject_class(sfp[0].asObject), mtd->mn, mtd->paramdom, 0, NULL);
		cache[0] = mtd;
	}
	sfp[0].unboxValue = kObject_Unbox(sfp[0].asObject);
	sfp[K_MTDIDX].calledMethod = mtd;
}

static KVirtualCode* KonohaVirtualMachine_Run(KonohaContext *, KonohaStack *, KVirtualCode *);

static KVirtualCode *KonohaVirtualMachine_tryJump(KonohaContext *kctx, KonohaStack *sfp, KVirtualCode *pc)
{
	int jmpresult;
	INIT_GCSTACK();
	KRuntimeContextVar *base = kctx->stack;
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

static struct KVirtualCode* KonohaVirtualMachine_Run(KonohaContext *kctx, KonohaStack *sfp0, struct KVirtualCode *pc)
{
#ifdef USE_DIRECT_THREADED_CODE
	static void *OPJUMP[] = {
		OPDEFINE(OPLABEL)
	};
#endif
	krbp_t *rbp = (krbp_t *)sfp0;
	DISPATCH_START(pc);
	OPDEFINE(OPEXEC)
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

typedef struct BasicNode BasicNode;

struct BasicNode {
	long     incoming;
	bblock_t newid;
	bblock_t nextid;
	bblock_t branchid;
	long     codeoffset;
	size_t   lastoffset;
	size_t   size;
	size_t   max;
};

static BasicNode *BasicNode_FindById(KonohaContext *kctx, bblock_t id)
{
	BasicNode *bb = NULL;
	while(id != -1) {
		bb = (BasicNode *)(kctx->stack->cwb.bytebuf + id);
		id = bb->newid;
	}
	return bb;
}

static bblock_t BasicNode_id(KonohaContext *kctx, BasicNode *bb)
{
	while(bb->newid != -1) {
		bb = BasicNode_FindById(kctx, bb->newid);
	}
	return ((char *)bb) - kctx->stack->cwb.bytebuf;
}

static BasicNode* new_BasicNode(KonohaContext *kctx, size_t max, bblock_t oldId)
{
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	BasicNode *bb = (BasicNode *)KLIB KBuffer_Alloca(kctx, &wb, max);
	if(oldId != -1) {
		BasicNode *oldbb = BasicNode_FindById(kctx, oldId);
		if(((char *)oldbb) + oldbb->max == (char *)bb) {
			oldbb->max += (max - sizeof(BasicNode));
			wb.m->bytesize -= sizeof(BasicNode);
			return oldbb;
		}
		memcpy(bb, oldbb, oldbb->size);
		oldbb->newid = BasicNode_id(kctx, bb);
		oldbb->size = 0;
	}
	else {
		bb->size = sizeof(BasicNode);
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
	return ((max - sizeof(BasicNode)) * 2) + sizeof(BasicNode);
}

static bblock_t BasicNode_Add(KonohaContext *kctx, bblock_t blockId, kfileline_t uline, KVirtualCode *op, size_t size, size_t padding_size)
{
	BasicNode *bb = BasicNode_FindById(kctx, blockId);
	DBG_ASSERT(bb->newid == -1);
	DBG_ASSERT(size <= padding_size);
	DBG_ASSERT(bb->nextid == -1 && bb->branchid == -1);
	if(!(bb->size + size < bb->max)) {
		size_t newsize = newsize2(bb->max);
		bb = new_BasicNode(kctx, newsize, blockId);
	}
	memcpy(((char *)bb) + bb->size, op, size);
	bb->size += padding_size;
	return BasicNode_id(kctx, bb);
}

static int CodeOffset(KBuffer *wb)
{
	return KBuffer_bytesize(wb);
}

static void BasicNode_WriteBuffer(KonohaContext *kctx, bblock_t blockId, KBuffer *wb)
{
	BasicNode *bb = BasicNode_FindById(kctx, blockId);
	while(bb != NULL && bb->codeoffset == -1) {
		size_t len = bb->size - sizeof(BasicNode);
		bb->codeoffset = CodeOffset(wb);
		if(bb->nextid == bb->branchid  && bb->nextid != -1) {
			bb->branchid = -1;
			len -= sizeof(KVirtualCode); // remove unnecesarry jump ..
		}
		if(len > 0) {
			bblock_t id = BasicNode_id(kctx, bb);
			char buf[len];  // bb is growing together with wb.
			memcpy(buf, ((char *)bb) + sizeof(BasicNode), len);
			KLIB KBuffer_Write(kctx, wb, buf, len);
			bb = BasicNode_FindById(kctx, id);  // recheck
			bb->lastoffset = CodeOffset(wb) - sizeof(KVirtualCode);
			DBG_ASSERT(bb->codeoffset + ((len / sizeof(KVirtualCode)) - 1) * sizeof(KVirtualCode) == bb->lastoffset);
		}
		else {
			DBG_ASSERT(bb->branchid == -1);
		}
		bb = BasicNode_FindById(kctx, bb->nextid);
	}
	bb = BasicNode_FindById(kctx, blockId);
	while(bb != NULL) {
		if(bb->branchid != -1 /*&& bb->branchid != builder->bbReturnId*/) {
			BasicNode *bbJ = BasicNode_FindById(kctx, bb->branchid);
			if(bbJ->codeoffset == -1) {
				BasicNode_WriteBuffer(kctx, bb->branchid, wb);
			}
		}
		bb = BasicNode_FindById(kctx, bb->nextid);
	}
}

static int BasicNode_size(BasicNode *bb)
{
	return (bb->size - sizeof(BasicNode)) / sizeof(KVirtualCode);
}

static BasicNode *BasicNode_leapJump(KonohaContext *kctx, BasicNode *bb)
{
	while(bb->nextid != -1) {
		if(BasicNode_size(bb) != 0) return bb;
		bb = BasicNode_FindById(kctx, bb->nextid);
	}
	if(bb->nextid == -1 && bb->branchid != -1 && BasicNode_size(bb) == 1) {
		return BasicNode_leapJump(kctx, BasicNode_FindById(kctx, bb->branchid));
	}
	return bb;
}

#define BasicNode_isVisited(bb)     (bb->incoming == -1)
#define BasicNode_SetVisited(bb)    bb->incoming = -1

static void BasicNode_SetJumpAddr(KonohaContext *kctx, BasicNode *bb, char *vcode)
{
	while(bb != NULL) {
		BasicNode_SetVisited(bb);
		if(bb->branchid != -1) {
			BasicNode *bbJ = BasicNode_leapJump(kctx, BasicNode_FindById(kctx, bb->branchid));
			OPJMP *j = (OPJMP *)(vcode + bb->lastoffset);
			DBG_ASSERT(j->opcode == OPCODE_JMP || j->opcode == OPCODE_JMPF);
			j->jumppc = (KVirtualCode *)(vcode + bbJ->codeoffset);
			bbJ = BasicNode_FindById(kctx, bb->branchid);
			if(!BasicNode_isVisited(bbJ)) {
				BasicNode_SetVisited(bbJ);
				BasicNode_SetJumpAddr(kctx, bbJ, vcode);
			}
		}
		bb = BasicNode_FindById(kctx, bb->nextid);
	}
}


/* ------------------------------------------------------------------------ */

static bblock_t new_BasicNodeLABEL(KonohaContext *kctx)
{
	BasicNode *bb = new_BasicNode(kctx, sizeof(KVirtualCode) * 2 + sizeof(BasicNode), -1);
	return BasicNode_id(kctx, bb);
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


static void KBuilder_Asm(KonohaContext *kctx, KBuilder *builder, KVirtualCode *op, size_t opsize)
{
	builder->bbMainId = BasicNode_Add(kctx, builder->bbMainId, builder->common.uline, op, opsize, sizeof(KVirtualCode));
}

static void kNode_SetLabelNode(KonohaContext *kctx, kNode *stmt, ksymbol_t label, bblock_t labelId)
{
	KLIB kObjectProto_SetUnboxValue(kctx, stmt, label, KType_int, labelId);
}

static bblock_t kNode_GetLabelNode(KonohaContext *kctx, kNode *stmt, ksymbol_t label)
{
	KKeyValue *kvs = KLIB kObjectProto_GetKeyValue(kctx, stmt, label);
	if(kvs != NULL) {
		return (bblock_t)kvs->unboxValue;
	}
	return -1;
}

static void ASM_LABEL(KonohaContext *kctx, KBuilder *builder, bblock_t labelId)
{
	BasicNode *bb = BasicNode_FindById(kctx, builder->bbMainId);
	DBG_ASSERT(bb != NULL);
	DBG_ASSERT(bb->nextid == -1);
	BasicNode *labelNode = BasicNode_FindById(kctx, labelId);
	labelNode->incoming += 1;
	builder->bbMainId = BasicNode_id(kctx, labelNode);
	bb->nextid = builder->bbMainId;
}

static void ASM_JMP(KonohaContext *kctx, KBuilder *builder, bblock_t labelId)
{
	BasicNode *bb = BasicNode_FindById(kctx, builder->bbMainId);
	DBG_ASSERT(bb != NULL);
	DBG_ASSERT(bb->nextid == -1);
	if(bb->branchid == -1) {
		ASM(JMP, NULL);
		BasicNode *labelNode = BasicNode_FindById(kctx, labelId);
		bb = BasicNode_FindById(kctx, builder->bbMainId);
		bb->branchid = BasicNode_id(kctx, labelNode);
		labelNode->incoming += 1;
	}
}

static bblock_t KBuilder_AsmJMPF(KonohaContext *kctx, KBuilder *builder, int flocal, bblock_t jumpId)
{
	BasicNode *bb = BasicNode_FindById(kctx, builder->bbMainId);
	DBG_ASSERT(bb != NULL);
	DBG_ASSERT(bb->nextid == -1 && bb->branchid == -1);
	bblock_t nextId = new_BasicNodeLABEL(kctx);
	ASM(JMPF, NULL, NC_(flocal));
	bb = BasicNode_FindById(kctx, builder->bbMainId);
	BasicNode *lbJUMP = BasicNode_FindById(kctx, jumpId);
	BasicNode *lbNEXT = BasicNode_FindById(kctx, nextId);
	bb->branchid = BasicNode_id(kctx, lbJUMP);
	bb->nextid = nextId;
	lbNEXT->incoming += 1;
	lbJUMP->incoming += 1;
	builder->bbMainId = nextId;
	return nextId;
}

static bblock_t KBuilder_asmJMPIF(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk, int isTRUE, bblock_t labelId)
{
	intptr_t a = ((intptr_t*)thunk)[0];
	SUGAR VisitNode(kctx, builder, expr, thunk);
	return KBuilder_AsmJMPF(kctx, builder, a, labelId);
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

static void KBuilder_AsmNMOV(KonohaContext *kctx, KBuilder *builder, int a, KClass *ty, int b)
{
	ASM(NMOV, TC_(a, ty), TC_(b, ty), ty);
}

//----------------------------------------------------------------------------

static kNode* Node_getFirstBlock(KonohaContext *kctx, kNode *stmt)
{
	return SUGAR kNode_GetNode(kctx, stmt, KSymbol_BlockPattern, K_NULLBLOCK);
}

static kNode* Node_getElseNode(KonohaContext *kctx, kNode *stmt)
{
	return SUGAR kNode_GetNode(kctx, stmt, KSymbol_else, K_NULLBLOCK);
}

static kNode* Node_getFirstExpr(KonohaContext *kctx, kNode *stmt)
{
	return SUGAR kNode_GetNode(kctx, stmt, KSymbol_ExprPattern, NULL);
}

static kNode *kNode_GetNode(KonohaContext *kctx, kNode *stmt, ksymbol_t kw)
{
	return (kNode *) kNode_GetObject(kctx, stmt, kw, NULL);
}

static kMethod* CallNode_getMethod(kNode *expr)
{
	return expr->NodeList->MethodItems[0];
}

static int CallNode_getArgCount(kNode *expr)
{
	return kArray_size(expr->NodeList) - 2;
}

/* Visitor */

static kbool_t KBuilder_VisitDoneNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	return true;
}

static kbool_t KBuilder_VisitConstNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	DBG_ASSERT(!KType_Is(UnboxType, expr->attrTypeId));
//	DBG_ASSERT(KNode_Is(ObjectConst, (expr)));
	kObject *v = KBuilder_AddConstPool(kctx, builder, expr->ObjectConstValue);
	intptr_t a = ((intptr_t*)thunk)[0];
	ASM(NSET, OC_(a), (uintptr_t)v, KClass_(expr->attrTypeId));
	return true;
}

static kbool_t KBuilder_VisitUnboxConstNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	intptr_t a = ((intptr_t*)thunk)[0];
	ASM(NSET, NC_(a), expr->unboxConstValue, KClass_(expr->attrTypeId));
	return true;
}

static kbool_t KBuilder_VisitNewNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	intptr_t a = ((intptr_t*)thunk)[0];
	ASM(NEW, OC_(a), expr->index, KClass_(expr->attrTypeId));
	return true;
}

static kbool_t KBuilder_VisitNullNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	intptr_t a = ((intptr_t*)thunk)[0];
	if(KType_Is(UnboxType, expr->attrTypeId)) {
		ASM(NSET, NC_(a), 0, KClass_(expr->attrTypeId));
	}
	else {
		ASM(NUL, OC_(a), KClass_(expr->attrTypeId));
	}
	return true;
}

static kbool_t KBuilder_VisitLocalNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	intptr_t a = ((intptr_t*)thunk)[0];
	KBuilder_AsmNMOV(kctx, builder, a, KClass_(expr->attrTypeId), expr->index);
	return true;
}

static kbool_t KBuilder_VisitFieldNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	intptr_t a = ((intptr_t*)thunk)[0];
	kshort_t index = (kshort_t)expr->index;
	kshort_t xindex = (kshort_t)(expr->index >> (sizeof(kshort_t)*8));
	KClass *ty = KClass_(expr->attrTypeId);
	ASM(NMOVx, TC_(a, ty), OC_(index), xindex, ty);
	return true;
}

static kbool_t KBuilder_VisitErrorNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	ASM(ERROR, kNode_uline(stmt), stmt->ErrorMessage);
	return false;
}

static void inline AssignLocal(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	if(node->attrTypeId != KType_void && ((intptr_t*)thunk)[0] != node->stackbase) {
		KBuilder_AsmNMOV(kctx, builder, ((intptr_t*)thunk)[0], KClass_(node->attrTypeId), node->stackbase);
	}
}

static kbool_t KBuilder_VisitBlockNode(KonohaContext *kctx, KBuilder *builder, kNode *block, void *thunk)
{
	size_t i;
	intptr_t espidx = block->stackbase;
	for (i = 0; i < kNode_GetNodeListSize(kctx, block); i++) {
		kNode *stmt = block->NodeList->NodeItems[i];
		builder->common.uline = kNode_uline(stmt);
		if(!SUGAR VisitNode(kctx, builder, stmt, &espidx)) break;
	}
	AssignLocal(kctx, builder, block, thunk);
	return true;
}

static kbool_t KBuilder_VisitReturnNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	kNode *expr = SUGAR kNode_GetNode(kctx, stmt, KSymbol_ExprPattern, NULL);
	if(expr != NULL && IS_Node(expr) && expr->attrTypeId != KType_void) {
		intptr_t a = K_RTNIDX;
		SUGAR VisitNode(kctx, builder, expr, &a);
	}
	ASM_JMP(kctx, builder, builder->bbReturnId); // RET
	return false;
}

static kbool_t KBuilder_VisitIfNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	intptr_t espidx = stmt->stackbase;
	bblock_t lbELSE = new_BasicNodeLABEL(kctx);
	bblock_t lbEND  = new_BasicNodeLABEL(kctx);
	/* if */
	KBuilder_asmJMPIF(kctx, builder, Node_getFirstExpr(kctx, stmt), &espidx, 0/*FALSE*/, lbELSE);
	/* then */
	SUGAR VisitNode(kctx, builder, Node_getFirstBlock(kctx, stmt), &espidx);
	ASM_JMP(kctx, builder, lbEND);
	/* else */
	ASM_LABEL(kctx, builder, lbELSE);
	SUGAR VisitNode(kctx, builder, Node_getElseNode(kctx, stmt), &espidx);
	//ASM(NOP);
	/* endif */
	ASM_LABEL(kctx, builder, lbEND);
	AssignLocal(kctx, builder, stmt, thunk);
	return true;
}

static kbool_t KBuilder_VisitWhileNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	intptr_t espidx = stmt->stackbase;
	bblock_t lbCONTINUE = new_BasicNodeLABEL(kctx);
	bblock_t lbBREAK    = new_BasicNodeLABEL(kctx);
	kNode_SetLabelNode(kctx, stmt, KSymbol_("continue"), lbCONTINUE);
	kNode_SetLabelNode(kctx, stmt, KSymbol_("break"),    lbBREAK);
	ASM_LABEL(kctx, builder, lbCONTINUE);
	KBuilder_AsmSAFEPOINT(kctx, builder, kNode_uline(stmt), espidx);
	KBuilder_asmJMPIF(kctx, builder, Node_getFirstExpr(kctx, stmt), &espidx, 0/*FALSE*/, lbBREAK);
	SUGAR VisitNode(kctx, builder, Node_getFirstBlock(kctx, stmt), &espidx);
	ASM_JMP(kctx, builder, lbCONTINUE);
	ASM_LABEL(kctx, builder, lbBREAK);
	AssignLocal(kctx, builder, stmt, thunk);
	return true;
}

static kbool_t KBuilder_VisitDoWhileNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	intptr_t espidx = stmt->stackbase;
	bblock_t lbCONTINUE = new_BasicNodeLABEL(kctx);
	bblock_t lbENTRY    = new_BasicNodeLABEL(kctx);
	bblock_t lbBREAK    = new_BasicNodeLABEL(kctx);
	kNode_SetLabelNode(kctx, stmt, KSymbol_("continue"), lbCONTINUE);
	kNode_SetLabelNode(kctx, stmt, KSymbol_("break"),    lbBREAK);
	ASM_JMP(kctx, builder, lbENTRY);
	ASM_LABEL(kctx, builder, lbCONTINUE);
	KBuilder_AsmSAFEPOINT(kctx, builder, kNode_uline(stmt), espidx);
	KBuilder_asmJMPIF(kctx, builder, Node_getFirstExpr(kctx, stmt), &espidx, 0/*FALSE*/, lbBREAK);
	ASM_LABEL(kctx, builder, lbENTRY);
	SUGAR VisitNode(kctx, builder, Node_getFirstBlock(kctx, stmt), &espidx);
	ASM_JMP(kctx, builder, lbCONTINUE);
	ASM_LABEL(kctx, builder, lbBREAK);
	AssignLocal(kctx, builder, stmt, thunk);
	return true;
}

//static kbool_t KBuilder_VisitForNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
//{
//	intptr_t espidx = stmt->stackbase;
//	bblock_t lbCONTINUE = new_BasicNodeLABEL(kctx);
//	bblock_t lbENTRY    = new_BasicNodeLABEL(kctx);
//	bblock_t lbBREAK    = new_BasicNodeLABEL(kctx);
//	kNode_SetLabelNode(kctx, stmt, KSymbol_("continue"), lbCONTINUE);
//	kNode_SetLabelNode(kctx, stmt, KSymbol_("break"),    lbBREAK);
//	ASM_JMP(kctx, builder, lbENTRY);
//	ASM_LABEL(kctx, builder, lbCONTINUE);
//	KBuilder_AsmSAFEPOINT(kctx, builder, kNode_uline(stmt), espidx);
//	kNode *iterNode = SUGAR kNode_GetBlock(kctx, stmt, NULL, KSymbol_("Iterator"), NULL);
//	if(iterNode != NULL) {
//		SUGAR VisitNode(kctx, builder, iterNode, &espidx);
//		ASM_LABEL(kctx, builder, lbENTRY);
//		KBuilder_asmJMPIF(kctx, builder, Node_getFirstExpr(kctx, stmt), &espidx, 0/*FALSE*/, lbBREAK);
//	}
//	else {
//		KBuilder_asmJMPIF(kctx, builder, Node_getFirstExpr(kctx, stmt), &espidx, 0/*FALSE*/, lbBREAK);
//		ASM_LABEL(kctx, builder, lbENTRY);
//	}
//	SUGAR VisitNode(kctx, builder, Node_getFirstBlock(kctx, stmt), &espidx);
//	ASM_JMP(kctx, builder, lbCONTINUE);
//	ASM_LABEL(kctx, builder, lbBREAK);
//	AssignLocal(kctx, builder, stmt, thunk);
//	return true;
//}

static kbool_t KBuilder_VisitContinueNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	ksymbol_t label = KSymbol_("continue");
	kNode *jump = kNode_GetNode(kctx, stmt, label);
	DBG_ASSERT(jump != NULL && IS_Node(jump));
	bblock_t lbJUMP = kNode_GetLabelNode(kctx, jump, label);
	ASM_JMP(kctx, builder, lbJUMP);
	return false;
}

static kbool_t KBuilder_VisitBreakNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	ksymbol_t label = KSymbol_("break");
	kNode *jump = kNode_GetNode(kctx, stmt, label);
	DBG_ASSERT(jump != NULL && IS_Node(jump));
	bblock_t lbJUMP = kNode_GetLabelNode(kctx, jump, label);
	ASM_JMP(kctx, builder, lbJUMP);
	return false;
}

static kbool_t KBuilder_VisitTryNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	//FIXME
	//kNode *catchNode   = SUGAR kNode_GetNode(kctx, stmt, NULL, KSymbol_("catch"),   K_NULLBLOCK);
	//kNode *finallyNode = SUGAR kNode_GetNode(kctx, stmt, NULL, KSymbol_("finally"), K_NULLBLOCK);
	//if(catchNode != K_NULLBLOCK){
	//}
	//if(finallyNode != K_NULLBLOCK){
	//}
	return true;
}

static kbool_t KBuilder_VisitThrowNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	return true;
}


static kbool_t KBuilder_VisitMethodCallNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	intptr_t espidx = expr->stackbase, thisidx = espidx + K_CALLDELTA;
	kMethod *mtd = CallNode_getMethod(expr);
	DBG_ASSERT(IS_Method(mtd));
	/*
	 * [CallNode] := this.method(arg1, arg2, ...)
	 * expr->NodeList = [method, this, arg1, arg2, ...]
	 **/
	int i, s = kMethod_Is(Static, mtd) ? 2 : 1;
	int argc = CallNode_getArgCount(expr);
	for (i = s; i < argc + 2; i++) {
		intptr_t a = thisidx + i - 1;
		kNode *exprN = kNode_At(expr, i);
		SUGAR VisitNode(kctx, builder, exprN, &a);
	}
	if(kMethod_Is(Final, mtd) || !kMethod_Is(Virtual, mtd)) {
		ASM(NSET, NC_(thisidx-1), (intptr_t)mtd, KClass_Method);
		if(kMethod_Is(Virtual, mtd)) {
			// set namespace to enable method lookups
			ASM(NSET, OC_(thisidx-2), (intptr_t)kNode_ns(expr), KClass_NameSpace);
		}
	}
	else {
		ASM(NSET, OC_(thisidx-2), (intptr_t)kNode_ns(expr), KClass_NameSpace);
		ASM(LOOKUP, SFP_(thisidx), kNode_ns(expr), mtd);
	}

	int esp_ = SFP_(espidx + argc + K_CALLDELTA + 1);
	ASM(CALL, builder->common.uline, SFP_(thisidx), esp_, KLIB Knull(kctx, KClass_(expr->attrTypeId)));
	AssignLocal(kctx, builder, expr, thunk);
	return true;
}

static kbool_t KBuilder_VisitAndNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	bblock_t lbFINAL  = new_BasicNodeLABEL(kctx);
	KBuilder_asmJMPIF(kctx, builder, kNode_At(expr, 1), thunk, 0/*FALSE*/, lbFINAL);
	SUGAR VisitNode(kctx, builder, kNode_At(expr, 2), thunk);
	ASM_LABEL(kctx, builder, lbFINAL);
	return true;
}

static kbool_t KBuilder_VisitOrNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	bblock_t lbFALSE = new_BasicNodeLABEL(kctx);
	bblock_t lbFINAL  = new_BasicNodeLABEL(kctx);
	KBuilder_asmJMPIF(kctx, builder, kNode_At(expr, 1), thunk, 0/*FALSE*/, lbFALSE);
	ASM_JMP(kctx, builder, lbFINAL);
	ASM_LABEL(kctx, builder, lbFALSE); // false
	SUGAR VisitNode(kctx, builder, kNode_At(expr, 2), thunk);
	ASM_LABEL(kctx, builder, lbFINAL);
	return true;
}

static kbool_t KBuilder_VisitAssignNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	kNode *leftHandNode  = kNode_At(expr, 1);
	kNode *rightHandNode = kNode_At(expr, 2);
	//DBG_P("LET (%s) a=%d, shift=%d, espidx=%d", KType_text(expr->attrTypeId), a, shift, espidx);
	if(leftHandNode->node == KNode_Local) {
		intptr_t a = ((intptr_t*)thunk)[0];
		SUGAR VisitNode(kctx, builder, rightHandNode, &(leftHandNode->index));
		if(expr->attrTypeId != KType_void && a != leftHandNode->index) {
			KBuilder_AsmNMOV(kctx, builder, a, KClass_(leftHandNode->attrTypeId), leftHandNode->index);
		}
	}
	else{
		assert(leftHandNode->node == KNode_Field);
		intptr_t espidx = expr->stackbase;
		SUGAR VisitNode(kctx, builder, rightHandNode, &espidx);
		kshort_t index  = (kshort_t)leftHandNode->index;
		kshort_t xindex = (kshort_t)(leftHandNode->index >> (sizeof(kshort_t)*8));
		KClass *lhsClass = KClass_(leftHandNode->attrTypeId), *rhClass = KClass_(rightHandNode->attrTypeId);
		ASM(XNMOV, OC_(index), xindex, TC_(espidx, rhClass), lhsClass);
		if(expr->attrTypeId != KType_void) {
			intptr_t a = ((intptr_t*)thunk)[0];
			ASM(NMOVx, TC_(a, rhClass), OC_(index), xindex, lhsClass);
		}
	}
	return true;
}

//static kbool_t KBuilder_VisitPushNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
//{
//	SUGAR VisitNode(kctx, builder, expr->NodeToPush, &(expr->stackbase));
//	return true;
//}

static kbool_t KBuilder_VisitBoxNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	intptr_t a = ((intptr_t*)thunk)[0];
	SUGAR VisitNode(kctx, builder, expr->NodeToPush, thunk);
	ASM(BOX, OC_(a), NC_(a), KClass_(expr->attrTypeId));
	return true;
}

// end of Visitor

static void FreeVirtualCode(KonohaContext *kctx, struct KVirtualCode *vcode)
{
	OPTHCODE * opTHCODE = (OPTHCODE *)(vcode - 1);
	if(opTHCODE->opcode == OPCODE_THCODE && opTHCODE->codesize > 0) {
		KFree(opTHCODE, opTHCODE->codesize);
	}
}

static struct KVirtualCodeAPI vapi = {
		FreeVirtualCode, WriteVirtualCode
};

static struct KVirtualCode *MakeThreadedCode(KonohaContext *kctx, KBuilder *builder, KVirtualCode *vcode, size_t codesize)
{
	OPTHCODE *opTHCODE = (OPTHCODE *)vcode;
	opTHCODE->codesize = codesize;
	struct KVirtualCodeAPI** p = (struct KVirtualCodeAPI **)builder->common.api->RunVirtualMachine(kctx, kctx->esp + 1, vcode);
	p[-1] = &vapi;
	return (KVirtualCode *)p;
}

static struct KVirtualCode *CompileVirtualCode(KonohaContext *kctx, KBuilder *builder, bblock_t beginId, bblock_t returnId)
{
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	BasicNode_WriteBuffer(kctx, beginId, &wb);
	BasicNode_WriteBuffer(kctx, returnId, &wb);

	size_t codesize = KBuffer_bytesize(&wb);
	DBG_P(">>>>>> codesize=%d", codesize);
	DBG_ASSERT(codesize != 0);
	KVirtualCode *vcode = (KVirtualCode *)KCalloc_UNTRACE(codesize, 1);
	memcpy((void *)vcode, KLIB KBuffer_text(kctx, &wb, NonZero), codesize);
	BasicNode_SetJumpAddr(kctx, BasicNode_FindById(kctx, beginId), (char *)vcode);
	KLIB KBuffer_Free(&wb);
	vcode = MakeThreadedCode(kctx, builder, vcode, codesize);
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

static struct KVirtualCode* MiniVM_GenerateKVirtualCode(KonohaContext *kctx, kMethod *mtd, kNode *block, int option)
{
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);

	INIT_GCSTACK();
	KBuilder builderbuf = {}, *builder = &builderbuf;
	kNameSpace *ns = kNode_ns(block);
	builder->common.api = ns->builderApi;
	builder->constPools = ns->NameSpaceConstList;
	builder->bbBeginId = new_BasicNodeLABEL(kctx);
	builder->bbReturnId = new_BasicNodeLABEL(kctx);
	builder->bbMainId = builder->bbBeginId;
	ASM(THCODE, 0, _THCODE);
	ASM(CHKSTACK, 0);
	//ASM_LABEL(kctx, builder, builder->bbBeginId);

	SUGAR VisitNode(kctx, builder, block, &block->stackbase);

	ASM_LABEL(kctx, builder,  builder->bbReturnId);
	if(mtd->mn == MN_new) {
		ASM(NMOV, OC_(K_RTNIDX), OC_(0), KClass_(mtd->typeId));   // FIXME: Type 'This' must be resolved
	}
	ASM(RET);
	KVirtualCode *vcode = CompileVirtualCode(kctx, builder, builder->bbBeginId, builder->bbReturnId);
	RESET_GCSTACK();
	KLIB KBuffer_Free(&wb);
	return vcode;
}

// -------------------------------------------------------------------------

static struct KVirtualCode  *BOOTCODE_ENTER = NULL;
static struct KVirtualCode  *BOOTCODE_NCALL = NULL;

static void SetUpBootCode(void)
{
	if(BOOTCODE_ENTER == NULL) {
		static struct KVirtualCode InitCode[6] = {};
		struct OPTHCODE thcode = {OP_(THCODE), 4 * sizeof(KVirtualCode), _THCODE};
		struct OPNCALL ncall = {OP_(NCALL)};
		struct OPENTER enter = {OP_(ENTER)};
		struct OPEXIT  exit  = {OP_(EXIT)};
		memcpy(InitCode,   &thcode, sizeof(OPTHCODE));
		memcpy(InitCode+1, &ncall,  sizeof(OPNCALL));
		memcpy(InitCode+2, &enter,  sizeof(OPENTER));
		memcpy(InitCode+3, &exit,   sizeof(OPEXIT));
		KVirtualCode *pc = KonohaVirtualMachine_Run(NULL, NULL, InitCode);
		BOOTCODE_NCALL = pc;
		BOOTCODE_ENTER = pc+1;
//		struct KVirtualCodeAPI **vapi = pc;  // check NULL
//		DBG_ASSERT(vapi[-1] == NULL);
//		vapi = pc + 1;
//		DBG_ASSERT(vapi[-1] == NULL);
	}
}

static KMETHOD KMethodFunc_RunVirtualMachine(KonohaContext *kctx, KonohaStack *sfp)
{
	DBG_ASSERT(IS_Method(sfp[K_MTDIDX].calledMethod));
	KonohaVirtualMachine_Run(kctx, sfp, BOOTCODE_ENTER);
}

static KMethodFunc MiniVM_GenerateKMethodFunc(KonohaContext *kctx, KVirtualCode *vcode)
{
	return KMethodFunc_RunVirtualMachine;
}

static void MiniVM_SetMethodCode(KonohaContext *kctx, kMethodVar *mtd, KVirtualCode *vcode, KMethodFunc func)
{
	KLIB kMethod_SetFunc(kctx, mtd, func);
	mtd->vcode_start = vcode;
}

static struct KVirtualCode* GetDefaultBootCode(void)
{
	return BOOTCODE_NCALL;
}

static void InitStaticBuilderApi(struct KBuilderAPI *builderApi)
{
	builderApi->target = "minivm";
#define DEFINE_BUILDER_API(NAME) builderApi->visit##NAME##Node = KBuilder_Visit##NAME##Node;
	KNodeList(DEFINE_BUILDER_API);
#undef DEFINE_BUILDER_API
	builderApi->GenerateKVirtualCode = MiniVM_GenerateKVirtualCode;
	builderApi->GenerateKMethodFunc = MiniVM_GenerateKMethodFunc;
	builderApi->SetMethodCode = MiniVM_SetMethodCode;
	builderApi->RunVirtualMachine   = KonohaVirtualMachine_Run;
}

static void MiniVMDeleteVirtualMachine(KonohaContext *kctx)
{
}

static struct KBuilderAPI* GetDefaultBuilderAPI(void)
{
	static struct KBuilderAPI builderApi = {};
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
	factory->VirtualMachineInfo   = &ModuleInfo;
	factory->GetDefaultBootCode   = GetDefaultBootCode;
	factory->GetDefaultBuilderAPI = GetDefaultBuilderAPI;
	factory->DeleteVirtualMachine = MiniVMDeleteVirtualMachine;
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

