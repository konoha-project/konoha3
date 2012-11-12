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
#include <minikonoha/klib.h>
#include <minikonoha/sugar.h>
#include "tracevm.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#define OPSPEC_(T)  #T, 0, VPARAM_##T

static const DEFINE_OPSPEC OPDATA[] = {
	{OPSPEC_(NOP)},
	{OPSPEC_(THCODE)},
	{OPSPEC_(ENTER)},
	{OPSPEC_(EXIT)},
	{OPSPEC_(NMOV)},
	{OPSPEC_(NMOVx)},
	{OPSPEC_(XNMOV)},
	{OPSPEC_(NEW)},
	{OPSPEC_(NULL)},
	{OPSPEC_(LOOKUP)},
	{OPSPEC_(CALL)},
	{OPSPEC_(RET)},
	{OPSPEC_(NCALL)},
	{OPSPEC_(BNOT)},
	{OPSPEC_(JMP)},
	{OPSPEC_(JMPF)},
	{OPSPEC_(TRYJMP)},
	{OPSPEC_(YIELD)},
	{OPSPEC_(ERROR)},
	{OPSPEC_(SAFEPOINT)},
	{OPSPEC_(CHKSTACK)},
	{OPSPEC_(TRACE)},
};

static void DumpOpArgument(KonohaContext *kctx, KGrowingBuffer *wb, VirtualCodeType type, VirtualCode *c, size_t i, VirtualCode *pc_start)
{
	switch(type) {
	case VMT_VOID: break;
	case VMT_ADDR:
		KLIB Kwb_printf(kctx, wb, " L%d", (int)((VirtualCode *)c->p[i] - pc_start));
		break;
	case VMT_R:
		KLIB Kwb_printf(kctx, wb, " sfp[%d,r=%d]", (int)c->data[i]/2, (int)c->data[i]);
		break;
	case VMT_U:
		KLIB Kwb_printf(kctx, wb, " u(%lu, ", c->data[i]); break;
	case VMT_F:
		KLIB Kwb_printf(kctx, wb, " function(%p)", c->p[i]); break;
	case VMT_TY:
		KLIB Kwb_printf(kctx, wb, "(%s)", CT_t(c->ct[i])); break;
	}/*switch*/
}

static void DumpOpCode(KonohaContext *kctx, KGrowingBuffer *wb, VirtualCode *c, VirtualCode *pc_start)
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

}

/* ------------------------------------------------------------------------ */
/* VirtualMacine */

static void _THCODE(KonohaContext *kctx, VirtualCode *pc, void **codeaddr)
{
#ifdef USE_DIRECT_THREADED_CODE
	while(1) {
		pc->codeaddr = codeaddr[pc->opcode];
		if(pc->opcode == OPCODE_RET || pc->opcode == OPCODE_EXIT) break;
		pc++;
	}
#endif
}

static void kNameSpace_lookupMethodWithInlineCache(KonohaContext *kctx, KonohaStack *sfp, kNameSpace *ns, kMethod **cache)
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

static VirtualCode* KonohaVirtualMachine_run(KonohaContext *, KonohaStack *, VirtualCode *);

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
	// TODO
}

#ifdef USE_DIRECT_THREADED_CODE
#define CASE(x)  L_##x :
#define NEXT_OP   (pc->codeaddr)
#define JUMP      *(NEXT_OP)
#ifdef K_USING_VMASMDISPATCH
#define GOTO_NEXT()								\
	asm volatile("jmp *%0;": : "g"(NEXT_OP));	\
	goto *(NEXT_OP)

#else
#define GOTO_NEXT()     goto *(NEXT_OP)
#define GIVEOUT_COVERAGEELEMENT  op->count++
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

static struct VirtualCode* KonohaVirtualMachine_run(KonohaContext *kctx, KonohaStack *sfp0, struct VirtualCode *pc)
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
		OPEXEC_NOP();  GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(THCODE) {
		OPTHCODE *op = (OPTHCODE *)pc;
		OPEXEC_THCODE(op->threadCode); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(ENTER) {
		OPENTER *op = (OPENTER *)pc;
		OPEXEC_ENTER(); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(EXIT) {
		OPEXIT *op = (OPEXIT *)pc;
		OPEXEC_EXIT(); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(NSET) {
		OPNSET *op = (OPNSET *)pc;
		OPEXEC_NSET(op->a, op->n, op->ty); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(NMOV) {
		OPNMOV *op = (OPNMOV *)pc;
		OPEXEC_NMOV(op->a, op->b, op->ty); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(NMOVx) {
		OPNMOVx *op = (OPNMOVx *)pc;
		OPEXEC_NMOVx(op->a, op->b, op->bx, op->ty); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(XNMOV) {
		OPXNMOV *op = (OPXNMOV *)pc;
		OPEXEC_XNMOV(op->a, op->ax, op->b, op->ty); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(NEW) {
		OPNEW *op = (OPNEW *)pc;
		OPEXEC_NEW(op->a, op->p, op->ty); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(NULL) {
		OPNULL *op = (OPNULL *)pc;
		OPEXEC_NULL(op->a, op->ty); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(LOOKUP) {
		OPLOOKUP *op = (OPLOOKUP *)pc;
		OPEXEC_LOOKUP(op->thisidx, op->ns, op->mtd); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(CALL) {
		OPCALL *op = (OPCALL *)pc;
		OPEXEC_CALL(op->uline, op->thisidx, op->espshift, op->tyo); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(RET) {
		OPRET *op = (OPRET *)pc;
		OPEXEC_RET(); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(NCALL) {
		OPNCALL *op = (OPNCALL *)pc;
		OPEXEC_NCALL(); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(BNOT) {
		OPBNOT *op = (OPBNOT *)pc;
		OPEXEC_BNOT(op->c, op->a); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(JMP) {
		OPJMP *op = (OPJMP *)pc;
		OPEXEC_JMP(pc = op->jumppc, JUMP); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(JMPF) {
		OPJMPF *op = (OPJMPF *)pc;
		OPEXEC_JMPF(pc = op->jumppc, JUMP, op->a);GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(TRYJMP) {
		OPTRYJMP *op = (OPTRYJMP *)pc;
		OPEXEC_TRYJMP(pc = op->jumppc, JUMP); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(YIELD) {
		OPYIELD *op = (OPYIELD *)pc;
		OPEXEC_YIELD(); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(ERROR) {
		OPERROR *op = (OPERROR *)pc;
		OPEXEC_ERROR(op->uline, op->msg, op->esp); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(SAFEPOINT) {
		OPSAFEPOINT *op = (OPSAFEPOINT *)pc;
		OPEXEC_SAFEPOINT(op->uline, op->esp); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(CHKSTACK) {
		OPCHKSTACK *op = (OPCHKSTACK *)pc;
		OPEXEC_CHKSTACK(op->uline); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	CASE(TRACE) {
		OPTRACE *op = (OPTRACE *)pc;
		OPEXEC_TRACE(op->uline, op->thisidx, op->trace); GIVEOUT_COVERAGEELEMENT; pc++;
		GOTO_NEXT();
	}
	DISPATCH_END(pc);
L_RETURN:;
	return pc;
}

// -------------------------------------------------------------------------

static struct VirtualCode  *BOOTCODE_ENTER = NULL;
static struct VirtualCode  *BOOTCODE_NCALL = NULL;

static void SetUpBootCode(void)
{
	if(BOOTCODE_ENTER == NULL) {
		static struct VirtualCode InitCode[6] = {};
		struct OPTHCODE thcode = {OP_(THCODE), _THCODE};
		struct OPNCALL ncall = {OP_(NCALL)};
		struct OPENTER enter = {OP_(ENTER)};
		struct OPEXIT  exit  = {OP_(EXIT)};
		memcpy(InitCode,   &thcode, sizeof(VirtualCode));
		memcpy(InitCode+1, &ncall,  sizeof(VirtualCode));
		memcpy(InitCode+2, &enter,  sizeof(VirtualCode));
		memcpy(InitCode+3, &exit,   sizeof(VirtualCode));
		VirtualCode *pc = KonohaVirtualMachine_run(NULL, NULL, InitCode);
		BOOTCODE_NCALL = pc;
		BOOTCODE_ENTER = pc+1;
	}
}

static kbool_t IsSupportedVirtualCode(int opcode)
{
	return (((size_t)opcode) < OPCODE_MAX);
}

static KMETHOD MethodFunc_runVirtualMachine(KonohaContext *kctx, KonohaStack *sfp)
{
	DBG_ASSERT(IS_Method(sfp[K_MTDIDX].calledMethod));
	PLATAPI RunVirtualMachine(kctx, sfp, BOOTCODE_ENTER);
}

static MethodFunc GetVirtualMachineMethodFunc(void)
{
	return MethodFunc_runVirtualMachine;
}

static struct VirtualCode* GetBootCodeOfNativeMethodCall(void)
{
	return BOOTCODE_NCALL;
}

// -------------------------------------------------------------------------

#ifdef HAVE_DB_H
#if defined(__linux__)
#include <db_185.h>
#else
#include <db.h>
#endif /*defined(__linux__)*/
#endif
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSIZE 64
static void TraceVMStoreCoverageLog(KonohaContext *kctx, const char *key, int value)
{
#ifdef HAVE_DB_H
#define DATABASE "konoha_coverage.db" //TODO change name for ET.

	DB *db = NULL;
	DBT DBkey = {};
	DBT DBvalue = {};
	char buffer[BUFSIZE];

	if((db = dbopen(DATABASE, O_CREAT | O_RDWR, S_IRWXU, DB_BTREE, NULL)) == NULL) {
		exit(EXIT_FAILURE);
	}

	DBkey.data = (char *)key;
	DBkey.size = strlen(key);

	PLATAPI snprintf_i(buffer, BUFSIZE, "%d", value);
	DBvalue.data = buffer;
	DBvalue.size = strlen(buffer);

	db->put(db, &DBkey, &DBvalue, R_NOOVERWRITE);
	db->close(db);
#endif
}

static void TraceVMGiveOutCoverageLog(KonohaContext *kctx, VirtualCode *pc)
{
	kfileline_t uline = 0;
	while(true) {
		if (pc->opcode == OPCODE_RET) {
			break;
		}
		if(pc->count > 0) {
			if((kushort_t)uline != (kushort_t)pc->line) {
				char key[BUFSIZE];
				uline = pc->line;
				PLATAPI syslog_i(5/*LOG_NOTICE*/, "{\"Method\": \"DScriptResult\", \"ScriptName\": \"%s\", \"ScriptLine\": %d , \"Count\": %d}", FileId_t(pc->line), (kushort_t)pc->line, pc->count);
				PLATAPI snprintf_i(key, BUFSIZE, "\"%s:%d\"", FileId_t(pc->line), (kushort_t)pc->line);
				TraceVMStoreCoverageLog(kctx, key, pc->count);
			}
		}
		pc++;
	}
}

static void TraceVMDeleteVirtualMachine(KonohaContext *kctx)
{
	KonohaRuntime *share = kctx->share;
	size_t i;
	for(i = 0; i < kArray_size(share->GlobalConstList); i++) {
		kObject *o = share->GlobalConstList->ObjectItems[i];
		if(IS_NameSpace(o)) {
			kNameSpace *ns = (kNameSpace *) o;
			size_t j;
			for(j = 0; j < kArray_size(ns->methodList_OnList); j++) {
				kMethod *mtd = ns->methodList_OnList->MethodItems[j];
				if(IS_ByteCode(mtd->CodeObject)) {
					TraceVMGiveOutCoverageLog(kctx, mtd->CodeObject->code);
				}
			}
		}
	}
}

// -------------------------------------------------------------------------

kbool_t LoadTraceVMModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"TraceVM", "0.1", 0, "minivm tracevm",
	};
	SetUpBootCode();
	factory->VirtualMachineInfo            = &ModuleInfo;
	factory->IsSupportedVirtualCode        = IsSupportedVirtualCode;
	factory->RunVirtualMachine             = KonohaVirtualMachine_run;
	factory->DeleteVirtualMachine          = TraceVMDeleteVirtualMachine;
	factory->GetVirtualMachineMethodFunc   = GetVirtualMachineMethodFunc;
	factory->GetBootCodeOfNativeMethodCall = GetBootCodeOfNativeMethodCall;
	return true;
}
#ifdef __cplusplus
} /* extern "C" */
#endif

