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

#include <iconv.h>
#include <errno.h>
#include <minikonoha/minikonoha.h>
#include "opcode.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */
/* [data] */


/* VirtualMacine */

static void kNameSpace_lookupMethodWithInlineCache(KonohaContext *kctx, KonohaStack *sfp, kNameSpace *ns, kMethod **cache)
{
	ktype_t typeId = O_typeId(sfp[0].asObject);
	kMethod *mtd = cache[0];
	if(mtd->typeId != typeId) {
		mtd = KLIB kNameSpace_getMethodBySignatureNULL(kctx, ns, typeId, mtd->mn, mtd->paramdom, 0, NULL);
		cache[0] = mtd;
	}
	sfp[K_MTDIDX].methodCallInfo = mtd;
}

static VirtualMachineInstruction* KonohaVirtualMachine_run(KonohaContext *, KonohaStack *, VirtualMachineInstruction *);

static VirtualMachineInstruction *KonohaVirtualMachine_tryJump(KonohaContext *kctx, KonohaStack *sfp, VirtualMachineInstruction *pc)
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

static void KonohaVirtualMachine_onSafePoint(KonohaContext *kctx, KonohaStack *sfp, kfileline_t uline)
{
	if(kctx->modshare[MOD_EVENT] != NULL) {
		KLIB KscheduleEvent(kctx);
	}
	KCheckSafePoint(kctx, sfp);
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
#define TC(c)
#define DISPATCH_START(pc) goto *OPJUMP[pc->opcode]
#define DISPATCH_END(pc)
#define GOTO_PC(pc)        GOTO_NEXT()
#else/*USE_DIRECT_THREADED_CODE*/
#define OPJUMP      NULL
#define CASE(x)     case OPCODE_##x :
#define NEXT_OP     L_HEAD
#define GOTO_NEXT() goto NEXT_OP
#define JUMP        L_HEAD
#define TC(c)
#define DISPATCH_START(pc) L_HEAD:;switch(pc->opcode) {
#define DISPATCH_END(pc)   } /*KNH_DIE("unknown opcode=%d", (int)pc->opcode)*/;
#define GOTO_PC(pc)         GOTO_NEXT()
#endif/*USE_DIRECT_THREADED_CODE*/

static VirtualMachineInstruction* KonohaVirtualMachine_run(KonohaContext *kctx, KonohaStack *sfp0, VirtualMachineInstruction *pc)
{
#ifdef USE_DIRECT_THREADED_CODE
	static void *OPJUMP[] = {
		&&L_NOP, &&L_THCODE, &&L_ENTER, &&L_EXIT,
		&&L_NSET, &&L_NMOV, &&L_NMOVx, &&L_XNMOV,
		&&L_NEW, &&L_NULL, &&L_LOOKUP, &&L_CALL,
		&&L_RET, &&L_NCALL, &&L_BNOT, &&L_JMP,
		&&L_JMPF, &&L_TRYJMP, &&L_YIELD, &&L_ERROR,
		&&L_SAFEPOINT, &&L_CHKSTACK, &&L_TRACE,
	};
#endif
	krbp_t *rbp = (krbp_t *)sfp0;
	DISPATCH_START(pc);
	CASE(NOP) {
		OPNOP *op = (OPNOP *)pc;
		OPEXEC_NOP();  pc++;
		GOTO_NEXT();
	}
	CASE(THCODE) {
		OPTHCODE *op = (OPTHCODE *)pc;
		OPEXEC_THCODE(op->threadCode); pc++;
		GOTO_NEXT();
	}
	CASE(ENTER) {
		OPENTER *op = (OPENTER *)pc;
		OPEXEC_ENTER(); pc++;
		GOTO_NEXT();
	}
	CASE(EXIT) {
		OPEXIT *op = (OPEXIT *)pc;
		OPEXEC_EXIT(); pc++;
		GOTO_NEXT();
	}
	CASE(NSET) {
		OPNSET *op = (OPNSET *)pc;
		OPEXEC_NSET(op->a, op->n, op->ty); pc++;
		GOTO_NEXT();
	}
	CASE(NMOV) {
		OPNMOV *op = (OPNMOV *)pc;
		OPEXEC_NMOV(op->a, op->b, op->ty); pc++;
		GOTO_NEXT();
	}
	CASE(NMOVx) {
		OPNMOVx *op = (OPNMOVx *)pc;
		OPEXEC_NMOVx(op->a, op->b, op->bx, op->ty); pc++;
		GOTO_NEXT();
	}
	CASE(XNMOV) {
		OPXNMOV *op = (OPXNMOV *)pc;
		OPEXEC_XNMOV(op->a, op->ax, op->b, op->ty); pc++;
		GOTO_NEXT();
	}
	CASE(NEW) {
		OPNEW *op = (OPNEW *)pc;
		OPEXEC_NEW(op->a, op->p, op->ty); pc++;
		GOTO_NEXT();
	}
	CASE(NULL) {
		OPNULL *op = (OPNULL *)pc;
		OPEXEC_NULL(op->a, op->ty); pc++;
		GOTO_NEXT();
	}
	CASE(LOOKUP) {
		OPLOOKUP *op = (OPLOOKUP *)pc;
		OPEXEC_LOOKUP(op->thisidx, op->ns, op->mtd); pc++;
		GOTO_NEXT();
	}
	CASE(CALL) {
		OPCALL *op = (OPCALL *)pc;
		OPEXEC_CALL(op->uline, op->thisidx, op->espshift, op->tyo); pc++;
		GOTO_NEXT();
	}
	CASE(RET) {
		OPRET *op = (OPRET *)pc;
		OPEXEC_RET(); pc++;
		GOTO_NEXT();
	}
	CASE(NCALL) {
		OPNCALL *op = (OPNCALL *)pc;
		OPEXEC_NCALL(); pc++;
		GOTO_NEXT();
	}
	CASE(BNOT) {
		OPBNOT *op = (OPBNOT *)pc;
		OPEXEC_BNOT(op->c, op->a); pc++;
		GOTO_NEXT();
	}
	CASE(JMP) {
		OPJMP *op = (OPJMP *)pc;
		OPEXEC_JMP(pc = op->jumppc, JUMP); pc++;
		GOTO_NEXT();
	}
	CASE(JMPF) {
		OPJMPF *op = (OPJMPF *)pc;
		OPEXEC_JMPF(pc = op->jumppc, JUMP, op->a);pc++;
		GOTO_NEXT();
	}
	CASE(TRYJMP) {
		OPTRYJMP *op = (OPTRYJMP *)pc;
		OPEXEC_TRYJMP(pc = op->jumppc, JUMP); pc++;
		GOTO_NEXT();
	}
	CASE(YIELD) {
		OPYIELD *op = (OPYIELD *)pc;
		OPEXEC_YIELD(); pc++;
		GOTO_NEXT();
	}
	CASE(ERROR) {
		OPERROR *op = (OPERROR *)pc;
		OPEXEC_ERROR(op->uline, op->msg, op->esp); pc++;
		GOTO_NEXT();
	}
	CASE(SAFEPOINT) {
		OPSAFEPOINT *op = (OPSAFEPOINT *)pc;
		OPEXEC_SAFEPOINT(op->uline, op->esp); pc++;
		GOTO_NEXT();
	}
	CASE(CHKSTACK) {
		OPCHKSTACK *op = (OPCHKSTACK *)pc;
		OPEXEC_CHKSTACK(op->uline); pc++;
		GOTO_NEXT();
	}
	CASE(TRACE) {
		OPTRACE *op = (OPTRACE *)pc;
		OPEXEC_TRACE(op->uline, op->thisidx, op->trace); pc++;
		GOTO_NEXT();
	}
	DISPATCH_END(pc);
	L_RETURN:;
	return pc;
}

// -------------------------------------------------------------------------

kbool_t LoadMiniVMModule(KonohaFactory *factory, ModuleType type)
{
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

