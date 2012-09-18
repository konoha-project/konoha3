#include <minikonoha/float.h>
#ifndef CLASSICVM_H_
#define CLASSICVM_H_

static void EXPR_asm(KonohaContext *kctx, int a, kExpr *expr, int shift, int espidx);
static kBasicBlockVar* new_BasicBlockLABEL(KonohaContext *kctx);

static void BUILD_asm(KonohaContext *kctx, VirtualMachineInstruction *op, size_t opsize);
#define MN_isNotNull MN_("isNotNull")
#define MN_isNull    MN_("isNull")
#define MN_get    MN_("get")
#define MN_set    MN_("set")
#define MN_opNOT  MN_("!")
#define MN_opNEG  MN_("opNEG")
#define MN_opADD  MN_("+")
#define MN_opSUB  MN_("-")
#define MN_opMUL  MN_("*")
#define MN_opDIV  MN_("/")
#define MN_opMOD  MN_("%")
#define MN_opEQ   MN_("==")
#define MN_opNEQ  MN_("!=")
#define MN_opLT   MN_("<")
#define MN_opLTE  MN_("<=")
#define MN_opGT   MN_(">")
#define MN_opGTE  MN_(">=")
#define MN_opLAND MN_("&")
#define MN_opLOR  MN_("|")
#define MN_opLXOR MN_("^")
#define MN_opLSFT MN_("<<")
#define MN_opRSFT MN_(">>")

static kbool_t CLASSICVM_BUILD_asmJMPF(KonohaContext *kctx, kBasicBlock *bb, klr_JMPF_t *op, int *swap)
{
	while(bb->op.bytesize > 0) {
		VirtualMachineInstruction *opP = BBOP(bb) + (BBSIZE(bb) - 1);
		if(opP->opcode == OPCODE_BNOT) {
			klr_BNOT_t *opN = (klr_BNOT_t*)opP;
			//DBG_P("REWRITE JMPF index %d => %d", op->a, opN->a);
			op->a = opN->a;
			*swap = (*swap == 0) ? 1 : 0;
			((kBasicBlockVar *)bb)->op.bytesize -= sizeof(VirtualMachineInstruction);
			continue;
		}
		if(OPCODE_iEQ <= opP->opcode && opP->opcode <= OPCODE_iGTE) {
			klr_iJEQ_t *opN = (klr_iJEQ_t*)opP;
			ksfpidx_t a = ((klr_iEQ_t*)opP)->a;
			ksfpidx_t b = ((klr_iEQ_t*)opP)->b;
			opN->jumppc = (op)->jumppc;
			opN->a = a; opN->b = b;
			opP->opcode = OPCODE_iJEQ + ((opP)->opcode - OPCODE_iEQ);
			return 1;
		}
		if(OPCODE_iEQC <= opP->opcode && opP->opcode <= OPCODE_iGTEC) {
			klr_iJEQC_t *opN = (klr_iJEQC_t*)opP;
			ksfpidx_t a = ((klr_iEQC_t*)opP)->a;
			kint_t n = ((klr_iEQC_t*)opP)->n;
			opN->jumppc = (op)->jumppc;
			opN->a = a; opN->n = n;
			opP->opcode = OPCODE_iJEQC + ((opP)->opcode - OPCODE_iEQC);
			return 1;
		}
		if(OPCODE_fEQ <= opP->opcode && opP->opcode <= OPCODE_fGTE) {
			klr_fJEQ_t *opN = (klr_fJEQ_t*)opP;
			ksfpidx_t a = ((klr_fEQ_t*)opP)->a;
			ksfpidx_t b = ((klr_fEQ_t*)opP)->b;
			opN->jumppc = (op)->jumppc;
			opN->a = a; opN->b = b;
			opP->opcode = OPCODE_fJEQ + ((opP)->opcode - OPCODE_fEQ);
			return 1;
		}
		if(OPCODE_fEQC <= opP->opcode && opP->opcode <= OPCODE_fGTEC) {
			klr_fJEQC_t *opN = (klr_fJEQC_t*)opP;
			ksfpidx_t a = ((klr_fEQC_t*)opP)->a;
			kfloat_t n = ((klr_fEQC_t*)opP)->n;
			opN->jumppc = (op)->jumppc;
			opN->a = a; opN->n = n;
			opP->opcode = OPCODE_fJEQC + ((opP)->opcode - OPCODE_fEQC);
			return 1;
		}
		break;
	}
	return 0;
}


#define TONOP(op)  op->opcode = OPCODE_NOP
#define _REMOVE(opX)              TONOP(opX); continue
#define _REMOVE2(opX, opX2)       TONOP(opX); _REMOVE(opX2)
#define _REMOVE3(opX, opX2, opX3) TONOP(opX); _REMOVE2(opX2, opX3)

static void CLASSICVM_BasicBlock_peephole(KonohaContext *kctx, kBasicBlock *bb)
{
	size_t i;
	for(i = 1; i < BBSIZE(bb); i++) {
		VirtualMachineInstruction *opP = BBOP(bb) + (i - 1);
		VirtualMachineInstruction *op  = BBOP(bb) + (i);
		if((op->opcode == OPCODE_fCAST || op->opcode == OPCODE_iCAST) && opP->opcode == OPCODE_NMOV) {
			klr_fCAST_t *opCAST = (klr_fCAST_t*)op;
			klr_NMOV_t *opNMOV = (klr_NMOV_t*)opP;
			if(opNMOV->a == opCAST->b && opCAST->a == opCAST->b) {
				opCAST->b = opNMOV->b;
				_REMOVE(opP);
			}
		}
		if (op->opcode == OPCODE_NMOV || op->opcode == OPCODE_OMOV) {
			klr_NMOV_t *opNMOV = (klr_NMOV_t *) op;
			if(opNMOV->a == opNMOV->b) {
				_REMOVE(op);
			}
		}
		if(opP->opcode == OPCODE_NSET && op->opcode == OPCODE_NSET) {
			klr_NSET_t *op1 = (klr_NSET_t*)opP;
			klr_NSET_t *op2 = (klr_NSET_t*)op;
			if(op1->a + K_NEXTIDX != op2->a)
				continue;
			if(sizeof(uintptr_t) == sizeof(kuint_t)) {
				klr_NSET_t *op3 = (klr_NSET_t*)(BBOP(bb) + i + 1);
				klr_NSET_t *op4 = (klr_NSET_t*)(BBOP(bb) + i + 2);
				if(op3->opcode != OPCODE_NSET || op2->a + K_NEXTIDX != op3->a) goto L_NSET2;
				if(op4->opcode == OPCODE_NSET && op3->a + K_NEXTIDX == op4->a) {
					klr_NSET4_t *opNSET = (klr_NSET4_t*)opP;
					opNSET->opcode = OPCODE_NSET4;
					opNSET->n2 = op2->n;
					opNSET->n3 = op3->n;
					opNSET->n4 = op4->n;
					_REMOVE3(op2, op3, op4);
				}
				else {
					klr_NSET3_t *opNSET = (klr_NSET3_t*)opP;
					opNSET->opcode = OPCODE_NSET3;
					opNSET->n2 = op2->n;
					opNSET->n3 = op3->n;
					_REMOVE2(op2, op3);
				}
			}
			L_NSET2:;
			klr_NSET2_t *opNSET = (klr_NSET2_t*)opP;
			opNSET->opcode = OPCODE_NSET2;
			opNSET->n2 = op2->n;
			_REMOVE(op2);
		}
		if(opP->opcode == OPCODE_OSET && op->opcode == OPCODE_OSET) {
			klr_OSET_t *op1 = (klr_OSET_t*)opP;
			klr_OSET_t *op2 = (klr_OSET_t*)op;
			if(op1->a + K_NEXTIDX != op2->a) continue;
			{
				klr_OSET_t *op3 = (klr_OSET_t*)(BBOP(bb) + i + 1);
				klr_OSET_t *op4 = (klr_OSET_t*)(BBOP(bb) + i + 2);
				if(op3->opcode != OPCODE_OSET || op2->a + K_NEXTIDX != op3->a) goto L_OSET2;
				if(op4->opcode == OPCODE_OSET && op3->a + K_NEXTIDX == op4->a) {
					klr_OSET4_t *opOSET = (klr_OSET4_t*)opP;
					opOSET->opcode = OPCODE_OSET4;
					opOSET->v2 = op2->o;
					opOSET->v3 = op3->o;
					opOSET->v4 = op4->o;
					_REMOVE3(op2, op3, op4);
				}
				else {
					klr_OSET3_t *opOSET = (klr_OSET3_t*)opP;
					opOSET->opcode = OPCODE_OSET3;
					opOSET->v2 = op2->o;
					opOSET->v3 = op3->o;
					_REMOVE2(op2, op3);
				}
			}
			L_OSET2:;
			klr_OSET2_t *opOSET = (klr_OSET2_t*)opP;
			opOSET->opcode = OPCODE_OSET2;
			opOSET->v2 = op2->o;
			_REMOVE(op2);
		}
		if(op->opcode == OPCODE_NMOV) {
#ifdef OPCODE_NNMOV
			if(opP->opcode == OPCODE_NMOV) {
				klr_NNMOV_t *opMOV = (klr_NNMOV_t*)opP;
				opMOV->c = ((klr_NMOV_t*)op)->a;
				opMOV->d = ((klr_NMOV_t*)op)->b;
				opP->opcode = OPCODE_NNMOV;
				_REMOVE(op);
			}
			if(opP->opcode == OPCODE_OMOV) {
				klr_ONMOV_t *opMOV = (klr_ONMOV_t *)opP;
				opMOV->c = ((klr_NMOV_t*)op)->a;
				opMOV->d = ((klr_NMOV_t*)op)->b;
				opP->opcode = OPCODE_ONMOV;
				_REMOVE(op);
			}
#endif
		}
		if(op->opcode == OPCODE_OMOV) {
#ifdef OPCODE_OOMOV
			if(opP->opcode == OPCODE_OMOV) {
				klr_OOMOV_t *opMOV = (klr_OOMOV_t*)opP;
				opMOV->c = ((klr_OMOV_t*)op)->a;
				opMOV->d = ((klr_OMOV_t*)op)->b;
				opP->opcode = OPCODE_OOMOV;
				_REMOVE(op);
			}
			if(opP->opcode == OPCODE_OMOV) {
				klr_ONMOV_t *opMOV = (klr_ONMOV_t *)opP;
				opMOV->c = opMOV->a;
				opMOV->d = opMOV->b;
				opMOV->a = ((klr_OMOV_t*)op)->a;
				opMOV->b = ((klr_OMOV_t*)op)->b;
				opP->opcode = OPCODE_ONMOV;
				_REMOVE(op);
			}
#endif
		}
	}
}

#ifdef OPCODE_CHKIDX
static void ASM_CHKIDX(KonohaContext *kctx, int aidx, int nidx)
{
	long i;
	kBasicBlock *bb = ctxcode->currentWorkingBlock;
	for(i = (long)BBSIZE(bb) - 1; i >= 0; i--) {
		klr_CHKIDX_t *op = (klr_CHKIDX_t*)(BBOP(bb) + i);
		kopcode_t opcode = op->opcode;
		if(opcode == OPCODE_CHKIDXC && op->a == aidx && op->n == nidx) {
			return;
		}
		if(OPCODE_SCALL <= opcode && opcode <= OPCODE_VCALL_) break;
	}
	ASM(CHKIDX, aidx, nidx);
}

static void ASM_CHKIDXC(KonohaContext *kctx, int aidx, int n)
{
	kBasicBlock *bb = ctxcode->currentWorkingBlock;
	long i;
	for(i = (long)BBSIZE(bb) - 1; i >= 0; i--) {
		klr_CHKIDXC_t *op = (klr_CHKIDXC_t*)(BBOP(bb) + i);
		kopcode_t opcode = op->opcode;
		if(opcode == OPCODE_CHKIDXC && op->a == aidx) {
			if(op->n < (kuint_t) n) op->n = n;
			return;
		}
		if(OPCODE_SCALL <= opcode && opcode <= OPCODE_VCALL_) break;
	}
	ASM(CHKIDXC, aidx, n);
}
#endif /* defined(OPCODE_CHKIDX) */

static kopcode_t OPimn(KonohaContext *kctx, kmethodn_t mn, int diff)
{
	if (mn == MN_opNEG ) return OPCODE_iNEG;
	if (mn == MN_opADD ) return OPCODE_iADD + diff;
	if (mn == MN_opSUB ) return OPCODE_iSUB + diff;
	if (mn == MN_opMUL ) return OPCODE_iMUL + diff;
	if (mn == MN_opDIV ) return OPCODE_iDIV + diff;
	if (mn == MN_opMOD ) return OPCODE_iMOD + diff;
	if (mn == MN_opEQ  ) return OPCODE_iEQ  + diff;
	if (mn == MN_opNEQ ) return OPCODE_iNEQ + diff;
	if (mn == MN_opLT  ) return OPCODE_iLT  + diff;
	if (mn == MN_opLTE ) return OPCODE_iLTE + diff;
	if (mn == MN_opGT  ) return OPCODE_iGT  + diff;
	if (mn == MN_opGTE ) return OPCODE_iGTE + diff;
#ifdef OPCODE_iAND
	if (mn == MN_opLAND) return OPCODE_iAND + diff;
	if (mn == MN_opLOR ) return OPCODE_iOR  + diff;
	if (mn == MN_opLXOR) return OPCODE_iXOR + diff;
	if (mn == MN_opLSFT) return OPCODE_iLSFT+ diff;
	if (mn == MN_opRSFT) return OPCODE_iRSFT+ diff;
#endif
	return OPCODE_NOP;
}

static kopcode_t OPfmn(KonohaContext *kctx, kmethodn_t mn, int diff)
{
	if (mn == MN_opNEG) return OPCODE_fNEG;
	if (mn == MN_opADD) return OPCODE_fADD + diff;
	if (mn == MN_opSUB) return OPCODE_fSUB + diff;
	if (mn == MN_opMUL) return OPCODE_fMUL + diff;
	if (mn == MN_opDIV) return OPCODE_fDIV + diff;
	if (mn == MN_opEQ ) return OPCODE_fEQ  + diff;
	if (mn == MN_opNEQ) return OPCODE_fNEQ + diff;
	if (mn == MN_opLT ) return OPCODE_fLT  + diff;
	if (mn == MN_opLTE) return OPCODE_fLTE + diff;
	if (mn == MN_opGT ) return OPCODE_fGT  + diff;
	if (mn == MN_opGTE) return OPCODE_fGTE + diff;
	return OPCODE_NOP;
}

static void kExpr_swap(kExpr *expr, int a, int b)
{
	kExpr *e0 = kExpr_at(expr, a);
	kExpr *e1 = kExpr_at(expr, b);
	kExpr_at(expr, a) = e1;
	kExpr_at(expr, b) = e0;
}

static kbool_t OPR_hasCONST(KonohaContext *kctx, kExpr *expr, kmethodn_t *mn, int swap)
{
	int isCONST = (kExpr_at(expr, 2)->build == TEXPR_NCONST);
	if(swap == 1 && kExpr_at(expr, 1)->build == TEXPR_NCONST) {
		kmethodn_t newmn = *mn;
		kExpr_swap(expr, 1, 2);
		if(*mn == MN_opLT) newmn = MN_opGT;  /* 1 < n ==> n > 1 */
		else if(*mn == MN_opLTE) newmn = MN_opGTE; /* 1 <= n => n >= 1 */
		else if(*mn == MN_opGT) newmn = MN_opLT;
		else if(*mn == MN_opGTE) newmn = MN_opLTE;
		//DBG_P("swap %s ==> %s", MN__(*mn), MN__(newmn));
		*mn = newmn;
		isCONST = 1;
	}
	return isCONST;
}

static kbool_t CLASSICVM_CALL_asm(KonohaContext *kctx, kMethod *mtd, kExpr *expr, int shift, int espidx)
{
	ktype_t mtd_cid = (mtd)->typeId;
	kmethodn_t mtd_mn = (mtd)->mn;
	int a = espidx + 1;
#if 1/*TODO*/
	if(mtd_cid == TY_Array) {
		ktype_t p1 = 0;//C_p1(cid);
		if(mtd_mn == MN_get) {
			EXPR_asm(kctx, a, kExpr_at(expr, 1), shift, a);
			if(kExpr_at(expr, 2)->build == TEXPR_NCONST) {
				intptr_t n = kExpr_at(expr, 2)->unboxValue;
				if(n < 0) {
					return 0;
				}
				ASM_CHKIDXC(kctx, OC_(a), n);
				if(TY_isUnbox(p1)) {
					ASM(NGETIDXC, NC_(espidx), OC_(a), n);
				}
				else {
					ASM(OGETIDXC, OC_(espidx), OC_(a), n);
				}
			}
			else {
				int an = espidx + 2;
				EXPR_asm(kctx, an, kExpr_at(expr, 2), shift, an);
				ASM_CHKIDX(kctx, OC_(a), NC_(an));
				if(TY_isUnbox(p1)) {
					ASM(NGETIDX, NC_(espidx), OC_(a), NC_(an));
				}
				else {
					ASM(OGETIDX, OC_(espidx), OC_(a), NC_(an));
				}
			}
			return 1;
		}
		if(mtd_mn == MN_set) {
			int v = espidx + 3;
			EXPR_asm(kctx, a, kExpr_at(expr, 1), shift, a);
			EXPR_asm(kctx, v, kExpr_at(expr, 3), shift, v);
			if(kExpr_at(expr, 2)->build == TEXPR_NCONST) {
				intptr_t n = kExpr_at(expr, 2)->unboxValue;
				if(n < 0) {
					return 0;
				}
				ktype_t p1 = 0;//TODO C_p1(cid);
				ASM_CHKIDXC(kctx, OC_(a), n);
				if(TY_isUnbox(p1)) {
					ASM(NSETIDXC, NC_(espidx), OC_(a), n, NC_(v));
				}
				else {
					ASM(OSETIDXC, OC_(espidx), OC_(a), n, OC_(v));
				}
			}
			else {
				int an = espidx + 2;
				EXPR_asm(kctx, an, kExpr_at(expr, 2), shift, an);
				ASM_CHKIDX(kctx, OC_(a), NC_(an));
				if(TY_isUnbox(p1)) {
					ASM(NSETIDX, NC_(espidx), OC_(a), NC_(an), NC_(v));
				}
				else {
					ASM(OSETIDX, OC_(espidx), OC_(a), NC_(an), OC_(v));
				}
			}
			return 1;
		}
	}
#endif
#if defined(OPCODE_BGETIDX)
	if(mtd_cid == TY_Bytes) {
		if(mtd_mn == MN_get) {
			EXPR_asm(kctx, a, kExpr_at(expr, 1), shift, a);
			if(kExpr_at(expr, 2)->build == TEXPR_NCONST) {
				intptr_t n = kExpr_at(expr, 2)->unboxValue;
				ASM_CHKIDXC(kctx, OC_(a), n);
				ASM(BGETIDXC, NC_(espidx), OC_(a), n);
			}
			else {
				int an = espidx + 2;
				EXPR_asm(kctx, an, kExpr_at(expr, 2), shift, an);
				ASM_CHKIDX(kctx, OC_(a), NC_(an));
				ASM(BGETIDX, NC_(espidx), OC_(a), NC_(an));
			}
			return 1;
		}
		if(mtd_mn == MN_set) {
			int v = espidx + 3;
			EXPR_asm(kctx, a, kExpr_at(expr, 1), shift, a);
			EXPR_asm(kctx, v, kExpr_at(expr, 3), shift, v);
			if(kExpr_at(expr, 2)->build == TEXPR_NCONST) {
				intptr_t n = kExpr_at(expr, 2)->unboxValue;
				if(n < 0) {
					return 0;
				}
				ASM_CHKIDXC(kctx, OC_(a), n);
				ASM(BSETIDXC, NC_(espidx), OC_(a), n, NC_(v));
			}
			else {
				int an = espidx + 2;
				EXPR_asm(kctx, an, kExpr_at(expr, 2), shift, an);
				ASM_CHKIDX(kctx, OC_(a), NC_(an));
				ASM(BSETIDX, NC_(espidx), OC_(a), NC_(an), NC_(v));
			}
			return 1;
		}
	}
#endif

#ifdef OPCODE_bNUL
	if(mtd_cid == TY_Object) {
		if(mtd_mn == MN_isNull) {
			EXPR_asm(kctx, a, kExpr_at(expr, 1), shift, a);
			ASM(bNUL, NC_(espidx), OC_(a));
			return 1;
		}
		else if(mtd_mn == MN_isNotNull) {
			EXPR_asm(kctx, a, kExpr_at(expr, 1), shift, a);
			ASM(bNN, NC_(espidx), OC_(a));
			return 1;
		}
	}
#endif
	kopcode_t opcode;
	ktype_t cid    = mtd_cid;
	kmethodn_t mn = mtd_mn;
	if(mtd_cid == TY_boolean && mtd_mn == MN_opNOT) {
		EXPR_asm(kctx, a, kExpr_at(expr, 1), shift, a);
		ASM(bNN, NC_(espidx), NC_(a));
		return 1;
	}
	if(mtd_cid == TY_int && ((opcode = OPimn(kctx, mn, 0)) != OPCODE_NOP)) {
		int swap = 1;
		if(mn == MN_opNEG) {
			EXPR_asm(kctx, a, kExpr_at(expr, 1), shift, a);
			ASM(iNEG, NC_(espidx), NC_(a));
			return 1;
		}
		if(mn == MN_opSUB || mn == MN_opDIV || mn == MN_opMOD ||
				mn == MN_opLSFT || mn == MN_opRSFT) swap = 0;
		if(OPR_hasCONST(kctx, expr, &mn, swap)) {
			EXPR_asm(kctx, a, kExpr_at(expr, 1), shift, a);
			kint_t b = kExpr_at(expr, 2)->unboxValue;
			if(b == 0 && (mn == MN_opDIV || mn == MN_opMOD)) {
				b = 1;
				//TODO
				//WarnTagDividedByZero(kctx);
			}
			opcode = OPimn(kctx, mn, (OPCODE_iADDC - OPCODE_iADD));
			ASMop(iADDC, opcode, NC_(espidx), NC_(espidx+1), b);
		}
		else {
			int b = espidx + 2;
			EXPR_asm(kctx, a, kExpr_at(expr, 1), shift, a);
			EXPR_asm(kctx, b, kExpr_at(expr, 2), shift, b);
			ASMop(iADD, opcode, NC_(espidx), NC_(a), NC_(b));
		}
		return 1;
	} /* TY_int */
	if(IS_DefinedFloat() && cid == TY_float && ((opcode = OPfmn(kctx, mn, 0)) != OPCODE_NOP)) {
		int swap = 1;
		if(mn == MN_opNEG) {
			EXPR_asm(kctx, a, kExpr_at(expr, 1), shift, a);
			ASM(fNEG, NC_(espidx), NC_(a));
			return 1;
		}
		if(mn == MN_opSUB || mn == MN_opDIV || mn == MN_opMOD) swap = 0;
		if(OPR_hasCONST(kctx, expr, &mn, swap)) {
			EXPR_asm(kctx, a, kExpr_at(expr, 1), shift, a);
			union { uintptr_t unboxValue; kfloat_t floatValue; } v;
			v.unboxValue = kExpr_at(expr, 2)->unboxValue;
			kfloat_t b = v.floatValue;
			/* TODO */
#define KFLOAT_ZERO 0.0
#define KFLOAT_ONE  1.0
			if(b == KFLOAT_ZERO && mn == MN_opDIV) {
				b = KFLOAT_ONE;
				//TODO
				//WarnTagDividedByZero(kctx);
			}
			opcode = OPfmn(kctx, mn, (OPCODE_fADDC - OPCODE_fADD));
			ASMop(fADDC, opcode, NC_(espidx), NC_(a), b);
		}
		else {
			int b = espidx + 2;
			EXPR_asm(kctx, a, kExpr_at(expr, 1), shift, a);
			EXPR_asm(kctx, b, kExpr_at(expr, 2), shift, b);
			ASMop(fADD, opcode, NC_(espidx), NC_(a), NC_(b));
		}
		return 1;
	} /* TY_float */
	return 0;
}

#endif /* end of include guard */
