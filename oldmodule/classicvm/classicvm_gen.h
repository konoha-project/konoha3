#ifndef CLASSICVM_GEN_H
#define CLASSICVM_GEN_H
// THIS FILE WAS AUTOMATICALLY GENERATED


#define OPCODE_NOP ((kopcode_t)0)
typedef struct klr_NOP_t {
	KCODE_HEAD;
} klr_NOP_t;

#define OPCODE_THCODE ((kopcode_t)1)
typedef struct klr_THCODE_t {
	KCODE_HEAD;
	ThreadCodeFunc th;
} klr_THCODE_t;

#define OPCODE_ENTER ((kopcode_t)2)
typedef struct klr_ENTER_t {
	KCODE_HEAD;
} klr_ENTER_t;

#define OPCODE_EXIT ((kopcode_t)3)
typedef struct klr_EXIT_t {
	KCODE_HEAD;
} klr_EXIT_t;

#define OPCODE_NSET ((kopcode_t)4)
typedef struct klr_NSET_t {
	KCODE_HEAD;
	kreg_t a;
	kint_t n;
	KonohaClass* ty;
} klr_NSET_t;

#define OPCODE_NMOV ((kopcode_t)5)
typedef struct klr_NMOV_t {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	KonohaClass* ty;
} klr_NMOV_t;

#define OPCODE_NMOVx ((kopcode_t)6)
typedef struct klr_NMOVx_t {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	uintptr_t bx;
	KonohaClass* ty;
} klr_NMOVx_t;

#define OPCODE_XNMOV ((kopcode_t)7)
typedef struct klr_XNMOV_t {
	KCODE_HEAD;
	kreg_t a;
	uintptr_t ax;
	kreg_t b;
	KonohaClass* ty;
} klr_XNMOV_t;

#define OPCODE_NEW ((kopcode_t)8)
typedef struct klr_NEW_t {
	KCODE_HEAD;
	kreg_t a;
	uintptr_t p;
	KonohaClass* ty;
} klr_NEW_t;

#define OPCODE_NULL ((kopcode_t)9)
typedef struct klr_NULL_t {
	KCODE_HEAD;
	kreg_t a;
	KonohaClass* ty;
} klr_NULL_t;

#define OPCODE_BOX ((kopcode_t)10)
typedef struct klr_BOX_t {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	KonohaClass* ty;
} klr_BOX_t;

#define OPCODE_UNBOX ((kopcode_t)11)
typedef struct klr_UNBOX_t {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	KonohaClass* ty;
} klr_UNBOX_t;

#define OPCODE_CALL ((kopcode_t)12)
typedef struct klr_CALL_t {
	KCODE_HEAD;
	uintptr_t uline;
	kreg_t thisidx;
	kreg_t espshift;
	kObject* tyo;
} klr_CALL_t;

#define OPCODE_RET ((kopcode_t)13)
typedef struct klr_RET_t {
	KCODE_HEAD;
} klr_RET_t;

#define OPCODE_NCALL ((kopcode_t)14)
typedef struct klr_NCALL_t {
	KCODE_HEAD;
} klr_NCALL_t;

#define OPCODE_BNOT ((kopcode_t)15)
typedef struct klr_BNOT_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
} klr_BNOT_t;

#define OPCODE_JMP ((kopcode_t)16)
typedef struct klr_JMP_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
} klr_JMP_t;

#define OPCODE_JMPF ((kopcode_t)17)
typedef struct klr_JMPF_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
} klr_JMPF_t;

#define OPCODE_SAFEPOINT ((kopcode_t)18)
typedef struct klr_SAFEPOINT_t {
	KCODE_HEAD;
	kreg_t espshift;
} klr_SAFEPOINT_t;

#define OPCODE_ERROR ((kopcode_t)19)
typedef struct klr_ERROR_t {
	KCODE_HEAD;
	kreg_t start;
	kString* msg;
} klr_ERROR_t;

#define OPCODE_NNMOV ((kopcode_t)20)
typedef struct klr_NNMOV_t {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	kreg_t c;
	kreg_t d;
} klr_NNMOV_t;

#define OPCODE_NSET2 ((kopcode_t)21)
typedef struct klr_NSET2_t {
	KCODE_HEAD;
	kreg_t a;
	kint_t n;
	kint_t n2;
} klr_NSET2_t;

#define OPCODE_NSET3 ((kopcode_t)22)
typedef struct klr_NSET3_t {
	KCODE_HEAD;
	kreg_t a;
	uintptr_t n;
	uintptr_t n2;
	uintptr_t n3;
} klr_NSET3_t;

#define OPCODE_NSET4 ((kopcode_t)23)
typedef struct klr_NSET4_t {
	KCODE_HEAD;
	kreg_t a;
	uintptr_t n;
	uintptr_t n2;
	uintptr_t n3;
	uintptr_t n4;
} klr_NSET4_t;

#define OPCODE_iINC ((kopcode_t)24)
typedef struct klr_iINC_t {
	KCODE_HEAD;
	kreg_t a;
} klr_iINC_t;

#define OPCODE_iDEC ((kopcode_t)25)
typedef struct klr_iDEC_t {
	KCODE_HEAD;
	kreg_t a;
} klr_iDEC_t;

#define OPCODE_bNUL ((kopcode_t)26)
typedef struct klr_bNUL_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
} klr_bNUL_t;

#define OPCODE_bNN ((kopcode_t)27)
typedef struct klr_bNN_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
} klr_bNN_t;

#define OPCODE_iNEG ((kopcode_t)28)
typedef struct klr_iNEG_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
} klr_iNEG_t;

#define OPCODE_fNEG ((kopcode_t)29)
typedef struct klr_fNEG_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
} klr_fNEG_t;

#define OPCODE_iADD ((kopcode_t)30)
typedef struct klr_iADD_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_iADD_t;

#define OPCODE_iSUB ((kopcode_t)31)
typedef struct klr_iSUB_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_iSUB_t;

#define OPCODE_iMUL ((kopcode_t)32)
typedef struct klr_iMUL_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_iMUL_t;

#define OPCODE_iDIV ((kopcode_t)33)
typedef struct klr_iDIV_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_iDIV_t;

#define OPCODE_iMOD ((kopcode_t)34)
typedef struct klr_iMOD_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_iMOD_t;

#define OPCODE_iEQ ((kopcode_t)35)
typedef struct klr_iEQ_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_iEQ_t;

#define OPCODE_iNEQ ((kopcode_t)36)
typedef struct klr_iNEQ_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_iNEQ_t;

#define OPCODE_iLT ((kopcode_t)37)
typedef struct klr_iLT_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_iLT_t;

#define OPCODE_iLTE ((kopcode_t)38)
typedef struct klr_iLTE_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_iLTE_t;

#define OPCODE_iGT ((kopcode_t)39)
typedef struct klr_iGT_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_iGT_t;

#define OPCODE_iGTE ((kopcode_t)40)
typedef struct klr_iGTE_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_iGTE_t;

#define OPCODE_iAND ((kopcode_t)41)
typedef struct klr_iAND_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_iAND_t;

#define OPCODE_iOR ((kopcode_t)42)
typedef struct klr_iOR_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_iOR_t;

#define OPCODE_iXOR ((kopcode_t)43)
typedef struct klr_iXOR_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_iXOR_t;

#define OPCODE_iLSFT ((kopcode_t)44)
typedef struct klr_iLSFT_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_iLSFT_t;

#define OPCODE_iRSFT ((kopcode_t)45)
typedef struct klr_iRSFT_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t n;
} klr_iRSFT_t;

#define OPCODE_iADDC ((kopcode_t)46)
typedef struct klr_iADDC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kint_t n;
} klr_iADDC_t;

#define OPCODE_iSUBC ((kopcode_t)47)
typedef struct klr_iSUBC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kint_t n;
} klr_iSUBC_t;

#define OPCODE_iMULC ((kopcode_t)48)
typedef struct klr_iMULC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kint_t n;
} klr_iMULC_t;

#define OPCODE_iDIVC ((kopcode_t)49)
typedef struct klr_iDIVC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kint_t n;
} klr_iDIVC_t;

#define OPCODE_iMODC ((kopcode_t)50)
typedef struct klr_iMODC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kint_t n;
} klr_iMODC_t;

#define OPCODE_iEQC ((kopcode_t)51)
typedef struct klr_iEQC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kint_t n;
} klr_iEQC_t;

#define OPCODE_iNEQC ((kopcode_t)52)
typedef struct klr_iNEQC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kint_t n;
} klr_iNEQC_t;

#define OPCODE_iLTC ((kopcode_t)53)
typedef struct klr_iLTC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kint_t n;
} klr_iLTC_t;

#define OPCODE_iLTEC ((kopcode_t)54)
typedef struct klr_iLTEC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kint_t n;
} klr_iLTEC_t;

#define OPCODE_iGTC ((kopcode_t)55)
typedef struct klr_iGTC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kint_t n;
} klr_iGTC_t;

#define OPCODE_iGTEC ((kopcode_t)56)
typedef struct klr_iGTEC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kint_t n;
} klr_iGTEC_t;

#define OPCODE_iANDC ((kopcode_t)57)
typedef struct klr_iANDC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kint_t n;
} klr_iANDC_t;

#define OPCODE_iORC ((kopcode_t)58)
typedef struct klr_iORC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kint_t n;
} klr_iORC_t;

#define OPCODE_iXORC ((kopcode_t)59)
typedef struct klr_iXORC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kint_t n;
} klr_iXORC_t;

#define OPCODE_iLSFTC ((kopcode_t)60)
typedef struct klr_iLSFTC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kint_t n;
} klr_iLSFTC_t;

#define OPCODE_iRSFTC ((kopcode_t)61)
typedef struct klr_iRSFTC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kint_t n;
} klr_iRSFTC_t;

#define OPCODE_fADD ((kopcode_t)62)
typedef struct klr_fADD_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_fADD_t;

#define OPCODE_fSUB ((kopcode_t)63)
typedef struct klr_fSUB_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_fSUB_t;

#define OPCODE_fMUL ((kopcode_t)64)
typedef struct klr_fMUL_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_fMUL_t;

#define OPCODE_fDIV ((kopcode_t)65)
typedef struct klr_fDIV_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_fDIV_t;

#define OPCODE_fEQ ((kopcode_t)66)
typedef struct klr_fEQ_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_fEQ_t;

#define OPCODE_fNEQ ((kopcode_t)67)
typedef struct klr_fNEQ_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_fNEQ_t;

#define OPCODE_fLT ((kopcode_t)68)
typedef struct klr_fLT_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_fLT_t;

#define OPCODE_fLTE ((kopcode_t)69)
typedef struct klr_fLTE_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_fLTE_t;

#define OPCODE_fGT ((kopcode_t)70)
typedef struct klr_fGT_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_fGT_t;

#define OPCODE_fGTE ((kopcode_t)71)
typedef struct klr_fGTE_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t b;
} klr_fGTE_t;

#define OPCODE_fADDC ((kopcode_t)72)
typedef struct klr_fADDC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kfloat_t n;
} klr_fADDC_t;

#define OPCODE_fSUBC ((kopcode_t)73)
typedef struct klr_fSUBC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kfloat_t n;
} klr_fSUBC_t;

#define OPCODE_fMULC ((kopcode_t)74)
typedef struct klr_fMULC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kfloat_t n;
} klr_fMULC_t;

#define OPCODE_fDIVC ((kopcode_t)75)
typedef struct klr_fDIVC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kfloat_t n;
} klr_fDIVC_t;

#define OPCODE_fEQC ((kopcode_t)76)
typedef struct klr_fEQC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kfloat_t n;
} klr_fEQC_t;

#define OPCODE_fNEQC ((kopcode_t)77)
typedef struct klr_fNEQC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kfloat_t n;
} klr_fNEQC_t;

#define OPCODE_fLTC ((kopcode_t)78)
typedef struct klr_fLTC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kfloat_t n;
} klr_fLTC_t;

#define OPCODE_fLTEC ((kopcode_t)79)
typedef struct klr_fLTEC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kfloat_t n;
} klr_fLTEC_t;

#define OPCODE_fGTC ((kopcode_t)80)
typedef struct klr_fGTC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kfloat_t n;
} klr_fGTC_t;

#define OPCODE_fGTEC ((kopcode_t)81)
typedef struct klr_fGTEC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kfloat_t n;
} klr_fGTEC_t;

#define OPCODE_OSET ((kopcode_t)82)
typedef struct klr_OSET_t {
	KCODE_HEAD;
	kreg_t a;
	kObject* o;
} klr_OSET_t;

#define OPCODE_OMOV ((kopcode_t)83)
typedef struct klr_OMOV_t {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
} klr_OMOV_t;

#define OPCODE_OOMOV ((kopcode_t)84)
typedef struct klr_OOMOV_t {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	kreg_t c;
	kreg_t d;
} klr_OOMOV_t;

#define OPCODE_ONMOV ((kopcode_t)85)
typedef struct klr_ONMOV_t {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
	kreg_t c;
	kreg_t d;
} klr_ONMOV_t;

#define OPCODE_OSET2 ((kopcode_t)86)
typedef struct klr_OSET2_t {
	KCODE_HEAD;
	kreg_t a;
	kObject* v;
	kObject* v2;
} klr_OSET2_t;

#define OPCODE_OSET3 ((kopcode_t)87)
typedef struct klr_OSET3_t {
	KCODE_HEAD;
	kreg_t a;
	kObject* v;
	kObject* v2;
	kObject* v3;
} klr_OSET3_t;

#define OPCODE_OSET4 ((kopcode_t)88)
typedef struct klr_OSET4_t {
	KCODE_HEAD;
	kreg_t a;
	kObject* v;
	kObject* v2;
	kObject* v3;
	kObject* v4;
} klr_OSET4_t;

#define OPCODE_CHKSTACK ((kopcode_t)89)
typedef struct klr_CHKSTACK_t {
	KCODE_HEAD;
	intptr_t n;
} klr_CHKSTACK_t;

#define OPCODE_LDMTD ((kopcode_t)90)
typedef struct klr_LDMTD_t {
	KCODE_HEAD;
	intptr_t thisidx;
	klr_Floadmtd loadmtd;
	kcachedata_t cache;
	kMethod* mtd;
} klr_LDMTD_t;

#define OPCODE_SCALL ((kopcode_t)91)
typedef struct klr_SCALL_t {
	KCODE_HEAD;
	uintptr_t uline;
	kreg_t thisidx;
	kreg_t espshift;
	kMethod* mtd;
	kObject* tyo;
} klr_SCALL_t;

#define OPCODE_VCALL ((kopcode_t)92)
typedef struct klr_VCALL_t {
	KCODE_HEAD;
	uintptr_t uline;
	kreg_t thisidx;
	kreg_t espshift;
	kMethod* mtd;
	kObject* tyo;
} klr_VCALL_t;

#define OPCODE_VCALL_ ((kopcode_t)93)
typedef struct klr_VCALL__t {
	KCODE_HEAD;
	uintptr_t uline;
	kreg_t thisidx;
	kreg_t espshift;
	kMethod* mtd;
	kObject* tyo;
} klr_VCALL__t;

#define OPCODE_iCAST ((kopcode_t)94)
typedef struct klr_iCAST_t {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
} klr_iCAST_t;

#define OPCODE_fCAST ((kopcode_t)95)
typedef struct klr_fCAST_t {
	KCODE_HEAD;
	kreg_t a;
	kreg_t b;
} klr_fCAST_t;

#define OPCODE_JMP_ ((kopcode_t)96)
typedef struct klr_JMP__t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
} klr_JMP__t;

#define OPCODE_NGETIDX ((kopcode_t)97)
typedef struct klr_NGETIDX_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t n;
} klr_NGETIDX_t;

#define OPCODE_NSETIDX ((kopcode_t)98)
typedef struct klr_NSETIDX_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t n;
	kreg_t v;
} klr_NSETIDX_t;

#define OPCODE_NGETIDXC ((kopcode_t)99)
typedef struct klr_NGETIDXC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	uintptr_t n;
} klr_NGETIDXC_t;

#define OPCODE_NSETIDXC ((kopcode_t)100)
typedef struct klr_NSETIDXC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	uintptr_t n;
	kreg_t v;
} klr_NSETIDXC_t;

#define OPCODE_OGETIDX ((kopcode_t)101)
typedef struct klr_OGETIDX_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t n;
} klr_OGETIDX_t;

#define OPCODE_OSETIDX ((kopcode_t)102)
typedef struct klr_OSETIDX_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	kreg_t n;
	kreg_t v;
} klr_OSETIDX_t;

#define OPCODE_OGETIDXC ((kopcode_t)103)
typedef struct klr_OGETIDXC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	uintptr_t n;
} klr_OGETIDXC_t;

#define OPCODE_OSETIDXC ((kopcode_t)104)
typedef struct klr_OSETIDXC_t {
	KCODE_HEAD;
	kreg_t c;
	kreg_t a;
	uintptr_t n;
	kreg_t v;
} klr_OSETIDXC_t;

#define OPCODE_bJNUL ((kopcode_t)105)
typedef struct klr_bJNUL_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
} klr_bJNUL_t;

#define OPCODE_bJNN ((kopcode_t)106)
typedef struct klr_bJNN_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
} klr_bJNN_t;

#define OPCODE_bJNOT ((kopcode_t)107)
typedef struct klr_bJNOT_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
} klr_bJNOT_t;

#define OPCODE_iJEQ ((kopcode_t)108)
typedef struct klr_iJEQ_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kreg_t b;
} klr_iJEQ_t;

#define OPCODE_iJNEQ ((kopcode_t)109)
typedef struct klr_iJNEQ_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kreg_t b;
} klr_iJNEQ_t;

#define OPCODE_iJLT ((kopcode_t)110)
typedef struct klr_iJLT_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kreg_t b;
} klr_iJLT_t;

#define OPCODE_iJLTE ((kopcode_t)111)
typedef struct klr_iJLTE_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kreg_t b;
} klr_iJLTE_t;

#define OPCODE_iJGT ((kopcode_t)112)
typedef struct klr_iJGT_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kreg_t b;
} klr_iJGT_t;

#define OPCODE_iJGTE ((kopcode_t)113)
typedef struct klr_iJGTE_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kreg_t b;
} klr_iJGTE_t;

#define OPCODE_iJEQC ((kopcode_t)114)
typedef struct klr_iJEQC_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kint_t n;
} klr_iJEQC_t;

#define OPCODE_iJNEQC ((kopcode_t)115)
typedef struct klr_iJNEQC_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kint_t n;
} klr_iJNEQC_t;

#define OPCODE_iJLTC ((kopcode_t)116)
typedef struct klr_iJLTC_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kint_t n;
} klr_iJLTC_t;

#define OPCODE_iJLTEC ((kopcode_t)117)
typedef struct klr_iJLTEC_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kint_t n;
} klr_iJLTEC_t;

#define OPCODE_iJGTC ((kopcode_t)118)
typedef struct klr_iJGTC_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kint_t n;
} klr_iJGTC_t;

#define OPCODE_iJGTEC ((kopcode_t)119)
typedef struct klr_iJGTEC_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kint_t n;
} klr_iJGTEC_t;

#define OPCODE_fJEQ ((kopcode_t)120)
typedef struct klr_fJEQ_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kreg_t b;
} klr_fJEQ_t;

#define OPCODE_fJNEQ ((kopcode_t)121)
typedef struct klr_fJNEQ_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kreg_t b;
} klr_fJNEQ_t;

#define OPCODE_fJLT ((kopcode_t)122)
typedef struct klr_fJLT_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kreg_t b;
} klr_fJLT_t;

#define OPCODE_fJLTE ((kopcode_t)123)
typedef struct klr_fJLTE_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kreg_t b;
} klr_fJLTE_t;

#define OPCODE_fJGT ((kopcode_t)124)
typedef struct klr_fJGT_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kreg_t b;
} klr_fJGT_t;

#define OPCODE_fJGTE ((kopcode_t)125)
typedef struct klr_fJGTE_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kreg_t b;
} klr_fJGTE_t;

#define OPCODE_fJEQC ((kopcode_t)126)
typedef struct klr_fJEQC_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kfloat_t n;
} klr_fJEQC_t;

#define OPCODE_fJNEQC ((kopcode_t)127)
typedef struct klr_fJNEQC_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kfloat_t n;
} klr_fJNEQC_t;

#define OPCODE_fJLTC ((kopcode_t)128)
typedef struct klr_fJLTC_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kfloat_t n;
} klr_fJLTC_t;

#define OPCODE_fJLTEC ((kopcode_t)129)
typedef struct klr_fJLTEC_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kfloat_t n;
} klr_fJLTEC_t;

#define OPCODE_fJGTC ((kopcode_t)130)
typedef struct klr_fJGTC_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kfloat_t n;
} klr_fJGTC_t;

#define OPCODE_fJGTEC ((kopcode_t)131)
typedef struct klr_fJGTEC_t {
	KCODE_HEAD;
	VirtualMachineInstruction  *jumppc;
	kreg_t a;
	kfloat_t n;
} klr_fJGTEC_t;

#define OPCODE_CHKIDX ((kopcode_t)132)
typedef struct klr_CHKIDX_t {
	KCODE_HEAD;
	kreg_t a;
	kreg_t n;
} klr_CHKIDX_t;

#define OPCODE_CHKIDXC ((kopcode_t)133)
typedef struct klr_CHKIDXC_t {
	KCODE_HEAD;
	kreg_t a;
	uintptr_t n;
} klr_CHKIDXC_t;

	
#define KOPCODE_MAX ((kopcode_t)134)

#define VMT_VOID     0
#define VMT_ADDR     1
#define VMT_R        2
#define VMT_RN       2
#define VMT_RO       2
#define VMT_U        3
#define VMT_I        4
#define VMT_CID      5
#define VMT_CO       6
#define VMT_INT      7
#define VMT_FLOAT    8
#define VMT_HCACHE   9
#define VMT_F        10/*function*/
#define VMT_STRING   11
#define VMT_METHOD   12
#define VMT_OBJECT   13
#define VMT_SFPIDX   14


/* ------------------------------------------------------------------------ */
/* [common] */

/* ------------------------------------------------------------------------ */
/* [data] */

#define _CONST 1
#define _JIT   (1<<1)
#define _DEF   (1<<2)
typedef struct {
	const char *name;
	kshortflag_t   flag;
	kushort_t size;
	kushort_t types[6];
} kOPDATA_t;

static const kOPDATA_t OPDATA[] = {
	{"NOP", 0, 0, { VMT_VOID}}, 
	{"THCODE", 0, 1, { VMT_F, VMT_VOID}}, 
	{"ENTER", 0, 0, { VMT_VOID}}, 
	{"EXIT", 0, 0, { VMT_VOID}}, 
	{"NSET", 0, 3, { VMT_RN, VMT_INT, VMT_CID, VMT_VOID}}, 
	{"NMOV", 0, 3, { VMT_RN, VMT_RN, VMT_CID, VMT_VOID}}, 
	{"NMOVx", 0, 4, { VMT_RN, VMT_RO, VMT_U, VMT_CID, VMT_VOID}}, 
	{"XNMOV", 0, 4, { VMT_RO, VMT_U, VMT_RN, VMT_CID, VMT_VOID}}, 
	{"NEW", 0, 3, { VMT_RO, VMT_U, VMT_CID, VMT_VOID}}, 
	{"NULL", 0, 2, { VMT_RO, VMT_CID, VMT_VOID}}, 
	{"BOX", 0, 3, { VMT_RO, VMT_RN, VMT_CID, VMT_VOID}}, 
	{"UNBOX", 0, 3, { VMT_RN, VMT_RO, VMT_CID, VMT_VOID}}, 
	{"CALL", 0, 4, { VMT_U, VMT_RO, VMT_RO, VMT_CO, VMT_VOID}}, 
	{"RET", 0, 0, { VMT_VOID}}, 
	{"NCALL", 0, 0, { VMT_VOID}}, 
	{"BNOT", 0, 2, { VMT_RN, VMT_RN, VMT_VOID}}, 
	{"JMP", 0, 1, { VMT_ADDR, VMT_VOID}}, 
	{"JMPF", 0, 2, { VMT_ADDR, VMT_RN, VMT_VOID}}, 
	{"SAFEPOINT", 0, 1, { VMT_RO, VMT_VOID}}, 
	{"ERROR", 0, 2, { VMT_RO, VMT_STRING, VMT_VOID}}, 
	{"NNMOV", _DEF, 4, { VMT_RN, VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"NSET2", _DEF|_JIT, 3, { VMT_RN, VMT_INT, VMT_INT, VMT_VOID}}, 
	{"NSET3", _DEF|_JIT, 4, { VMT_RN, VMT_U, VMT_U, VMT_U, VMT_VOID}}, 
	{"NSET4", _DEF|_JIT, 5, { VMT_RN, VMT_U, VMT_U, VMT_U, VMT_U, VMT_VOID}}, 
	{"iINC", _DEF|_JIT, 1, { VMT_RN, VMT_VOID}}, 
	{"iDEC", _DEF|_JIT, 1, { VMT_RN, VMT_VOID}}, 
	{"bNUL", _DEF|_JIT, 2, { VMT_RN, VMT_RO, VMT_VOID}}, 
	{"bNN", _DEF|_JIT, 2, { VMT_RN, VMT_RO, VMT_VOID}}, 
	{"iNEG", _DEF|_JIT, 2, { VMT_RN, VMT_RN, VMT_VOID}}, 
	{"fNEG", _DEF|_JIT, 2, { VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iADD", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iSUB", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iMUL", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iDIV", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iMOD", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iEQ", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iNEQ", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iLT", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iLTE", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iGT", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iGTE", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iAND", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iOR", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iXOR", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iLSFT", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iRSFT", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iADDC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"iSUBC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"iMULC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"iDIVC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"iMODC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"iEQC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"iNEQC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"iLTC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"iLTEC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"iGTC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"iGTEC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"iANDC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"iORC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"iXORC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"iLSFTC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"iRSFTC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"fADD", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"fSUB", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"fMUL", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"fDIV", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"fEQ", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"fNEQ", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"fLT", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"fLTE", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"fGT", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"fGTE", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"fADDC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_FLOAT, VMT_VOID}}, 
	{"fSUBC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_FLOAT, VMT_VOID}}, 
	{"fMULC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_FLOAT, VMT_VOID}}, 
	{"fDIVC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_FLOAT, VMT_VOID}}, 
	{"fEQC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_FLOAT, VMT_VOID}}, 
	{"fNEQC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_FLOAT, VMT_VOID}}, 
	{"fLTC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_FLOAT, VMT_VOID}}, 
	{"fLTEC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_FLOAT, VMT_VOID}}, 
	{"fGTC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_FLOAT, VMT_VOID}}, 
	{"fGTEC", _DEF|_JIT, 3, { VMT_RN, VMT_RN, VMT_FLOAT, VMT_VOID}}, 
	{"OSET", _DEF|_JIT|_CONST, 2, { VMT_RO, VMT_OBJECT, VMT_VOID}}, 
	{"OMOV", _DEF|_JIT, 2, { VMT_RO, VMT_RO, VMT_VOID}}, 
	{"OOMOV", _DEF|_JIT, 4, { VMT_RO, VMT_RO, VMT_RO, VMT_RO, VMT_VOID}}, 
	{"ONMOV", _DEF|_JIT, 4, { VMT_RO, VMT_RO, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"OSET2", _JIT|_CONST, 3, { VMT_RO, VMT_OBJECT, VMT_OBJECT, VMT_VOID}}, 
	{"OSET3", _JIT|_CONST, 4, { VMT_RO, VMT_OBJECT, VMT_OBJECT, VMT_OBJECT, VMT_VOID}}, 
	{"OSET4", _JIT|_CONST, 5, { VMT_RO, VMT_OBJECT, VMT_OBJECT, VMT_OBJECT, VMT_OBJECT, VMT_VOID}}, 
	{"CHKSTACK", 0, 1, { VMT_SFPIDX, VMT_VOID}}, 
	{"LDMTD", 0, 4, { VMT_SFPIDX, VMT_F, VMT_HCACHE, VMT_METHOD, VMT_VOID}}, 
	{"SCALL", _DEF|_JIT, 5, { VMT_U, VMT_RO, VMT_RO, VMT_METHOD, VMT_CO, VMT_VOID}}, 
	{"VCALL", _DEF|_JIT, 5, { VMT_U, VMT_RO, VMT_RO, VMT_METHOD, VMT_CO, VMT_VOID}}, 
	{"VCALL", _DEF|_JIT, 5, { VMT_U, VMT_RO, VMT_RO, VMT_METHOD, VMT_CO, VMT_VOID}}, 
	{"iCAST", _DEF|_JIT, 2, { VMT_RN, VMT_RN, VMT_VOID}}, 
	{"fCAST", _DEF|_JIT, 2, { VMT_RN, VMT_RN, VMT_VOID}}, 
	{"JMP", _JIT, 1, { VMT_ADDR, VMT_VOID}}, 
	{"NGETIDX", _DEF|_JIT, 3, { VMT_RN, VMT_RO, VMT_RN, VMT_VOID}}, 
	{"NSETIDX", _DEF|_JIT, 4, { VMT_RN, VMT_RO, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"NGETIDXC", _DEF|_JIT, 3, { VMT_RN, VMT_RO, VMT_U, VMT_VOID}}, 
	{"NSETIDXC", _DEF|_JIT, 4, { VMT_RN, VMT_RO, VMT_U, VMT_RN, VMT_VOID}}, 
	{"OGETIDX", _DEF|_JIT, 3, { VMT_RO, VMT_RO, VMT_RN, VMT_VOID}}, 
	{"OSETIDX", _DEF|_JIT, 4, { VMT_RO, VMT_RO, VMT_RN, VMT_RO, VMT_VOID}}, 
	{"OGETIDXC", _DEF|_JIT, 3, { VMT_RO, VMT_RO, VMT_U, VMT_VOID}}, 
	{"OSETIDXC", _DEF|_JIT, 4, { VMT_RO, VMT_RO, VMT_U, VMT_RO, VMT_VOID}}, 
	{"bJNUL", _DEF|_JIT, 2, { VMT_ADDR, VMT_RO, VMT_VOID}}, 
	{"bJNN", _DEF|_JIT, 2, { VMT_ADDR, VMT_RO, VMT_VOID}}, 
	{"bJNOT", _DEF|_JIT, 2, { VMT_ADDR, VMT_RN, VMT_VOID}}, 
	{"iJEQ", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iJNEQ", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iJLT", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iJLTE", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iJGT", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iJGTE", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"iJEQC", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"iJNEQC", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"iJLTC", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"iJLTEC", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"iJGTC", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"iJGTEC", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_INT, VMT_VOID}}, 
	{"fJEQ", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"fJNEQ", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"fJLT", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"fJLTE", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"fJGT", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"fJGTE", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_RN, VMT_VOID}}, 
	{"fJEQC", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_FLOAT, VMT_VOID}}, 
	{"fJNEQC", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_FLOAT, VMT_VOID}}, 
	{"fJLTC", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_FLOAT, VMT_VOID}}, 
	{"fJLTEC", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_FLOAT, VMT_VOID}}, 
	{"fJGTC", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_FLOAT, VMT_VOID}}, 
	{"fJGTEC", _DEF|_JIT, 3, { VMT_ADDR, VMT_RN, VMT_FLOAT, VMT_VOID}}, 
	{"CHKIDX", _JIT, 2, { VMT_RO, VMT_RN, VMT_VOID}}, 
	{"CHKIDXC", _JIT, 2, { VMT_RO, VMT_U, VMT_VOID}}, 
};

static void opcode_check(void)
{
	assert(sizeof(klr_NOP_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_THCODE_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_ENTER_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_EXIT_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_NSET_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_NMOV_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_NMOVx_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_XNMOV_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_NEW_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_NULL_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_BOX_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_UNBOX_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_CALL_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_RET_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_NCALL_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_BNOT_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_JMP_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_JMPF_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_SAFEPOINT_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_ERROR_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_NNMOV_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_NSET2_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_NSET3_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_NSET4_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iINC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iDEC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_bNUL_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_bNN_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iNEG_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fNEG_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iADD_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iSUB_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iMUL_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iDIV_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iMOD_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iEQ_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iNEQ_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iLT_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iLTE_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iGT_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iGTE_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iAND_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iOR_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iXOR_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iLSFT_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iRSFT_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iADDC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iSUBC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iMULC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iDIVC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iMODC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iEQC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iNEQC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iLTC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iLTEC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iGTC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iGTEC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iANDC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iORC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iXORC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iLSFTC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iRSFTC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fADD_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fSUB_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fMUL_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fDIV_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fEQ_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fNEQ_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fLT_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fLTE_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fGT_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fGTE_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fADDC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fSUBC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fMULC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fDIVC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fEQC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fNEQC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fLTC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fLTEC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fGTC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fGTEC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_OSET_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_OMOV_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_OOMOV_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_ONMOV_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_OSET2_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_OSET3_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_OSET4_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_CHKSTACK_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_LDMTD_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_SCALL_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_VCALL_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_VCALL__t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iCAST_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fCAST_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_JMP__t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_NGETIDX_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_NSETIDX_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_NGETIDXC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_NSETIDXC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_OGETIDX_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_OSETIDX_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_OGETIDXC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_OSETIDXC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_bJNUL_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_bJNN_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_bJNOT_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iJEQ_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iJNEQ_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iJLT_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iJLTE_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iJGT_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iJGTE_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iJEQC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iJNEQC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iJLTC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iJLTEC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iJGTC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_iJGTEC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fJEQ_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fJNEQ_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fJLT_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fJLTE_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fJGT_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fJGTE_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fJEQC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fJNEQC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fJLTC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fJLTEC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fJGTC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_fJGTEC_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_CHKIDX_t) <= sizeof(VirtualMachineInstruction));
	assert(sizeof(klr_CHKIDXC_t) <= sizeof(VirtualMachineInstruction));
}

static const char *T_opcode(kopcode_t opcode)
{
	if(opcode < KOPCODE_MAX) {
		return OPDATA[opcode].name;
	}
	else {
		fprintf(stderr, "opcode=%d\n", (int)opcode);
		return "OPCODE_??";
	}
}

#ifdef OLD
static size_t kopcode_size(kopcode_t opcode)
{
	return OPDATA[opcode].size;
}

static kbool_t kopcode_hasjump(kopcode_t opcode)
{
	return (OPDATA[opcode].types[0] == VMT_ADDR);
}
#endif

/* ------------------------------------------------------------------------ */



/* ------------------------------------------------------------------------ */
/* [exec] */


//#ifdef K_USING_VMCOUNT_
//#define VMCOUNT(op)    ((op)->count)++;
//#else
//#define VMCOUNT(op)
//#endif

//#if(defined(K_USING_LINUX_) && (defined(__i386__) || defined(__x86_64__)) && (defined(__GNUC__) && __GNUC__ >= 3))
//#define K_USING_VMASMDISPATCH 1
//#endif

#ifdef K_USING_THCODE_
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
#else/*K_USING_THCODE_*/
#define OPJUMP      NULL
#define CASE(x)     case OPCODE_##x :
#define NEXT_OP     L_HEAD
#define GOTO_NEXT() goto NEXT_OP
#define JUMP        L_HEAD
#define TC(c)
#define DISPATCH_START(pc) L_HEAD:;switch(pc->opcode) {
#define DISPATCH_END(pc)   } /*KNH_DIE("unknown opcode=%d", (int)pc->opcode)*/;
#define GOTO_PC(pc)         GOTO_NEXT()
#endif/*K_USING_THCODE_*/

static VirtualMachineInstruction* KonohaVirtualMachine_run(KonohaContext *kctx, KonohaStack *sfp0, VirtualMachineInstruction *pc)
{
#ifdef K_USING_THCODE_
	static void *OPJUMP[] = {
		&&L_NOP, &&L_THCODE, &&L_ENTER, &&L_EXIT, 
		&&L_NSET, &&L_NMOV, &&L_NMOVx, &&L_XNMOV, 
		&&L_NEW, &&L_NULL, &&L_BOX, &&L_UNBOX, 
		&&L_CALL, &&L_RET, &&L_NCALL, &&L_BNOT, 
		&&L_JMP, &&L_JMPF, &&L_SAFEPOINT, &&L_ERROR, 
		&&L_NNMOV, &&L_NSET2, &&L_NSET3, &&L_NSET4, 
		&&L_iINC, &&L_iDEC, &&L_bNUL, &&L_bNN, 
		&&L_iNEG, &&L_fNEG, &&L_iADD, &&L_iSUB, 
		&&L_iMUL, &&L_iDIV, &&L_iMOD, &&L_iEQ, 
		&&L_iNEQ, &&L_iLT, &&L_iLTE, &&L_iGT, 
		&&L_iGTE, &&L_iAND, &&L_iOR, &&L_iXOR, 
		&&L_iLSFT, &&L_iRSFT, &&L_iADDC, &&L_iSUBC, 
		&&L_iMULC, &&L_iDIVC, &&L_iMODC, &&L_iEQC, 
		&&L_iNEQC, &&L_iLTC, &&L_iLTEC, &&L_iGTC, 
		&&L_iGTEC, &&L_iANDC, &&L_iORC, &&L_iXORC, 
		&&L_iLSFTC, &&L_iRSFTC, &&L_fADD, &&L_fSUB, 
		&&L_fMUL, &&L_fDIV, &&L_fEQ, &&L_fNEQ, 
		&&L_fLT, &&L_fLTE, &&L_fGT, &&L_fGTE, 
		&&L_fADDC, &&L_fSUBC, &&L_fMULC, &&L_fDIVC, 
		&&L_fEQC, &&L_fNEQC, &&L_fLTC, &&L_fLTEC, 
		&&L_fGTC, &&L_fGTEC, &&L_OSET, &&L_OMOV, 
		&&L_OOMOV, &&L_ONMOV, &&L_OSET2, &&L_OSET3, 
		&&L_OSET4, &&L_CHKSTACK, &&L_LDMTD, &&L_SCALL, 
		&&L_VCALL, &&L_VCALL_, &&L_iCAST, &&L_fCAST, 
		&&L_JMP_, &&L_NGETIDX, &&L_NSETIDX, &&L_NGETIDXC, 
		&&L_NSETIDXC, &&L_OGETIDX, &&L_OSETIDX, &&L_OGETIDXC, 
		&&L_OSETIDXC, &&L_bJNUL, &&L_bJNN, &&L_bJNOT, 
		&&L_iJEQ, &&L_iJNEQ, &&L_iJLT, &&L_iJLTE, 
		&&L_iJGT, &&L_iJGTE, &&L_iJEQC, &&L_iJNEQC, 
		&&L_iJLTC, &&L_iJLTEC, &&L_iJGTC, &&L_iJGTEC, 
		&&L_fJEQ, &&L_fJNEQ, &&L_fJLT, &&L_fJLTE, 
		&&L_fJGT, &&L_fJGTE, &&L_fJEQC, &&L_fJNEQC, 
		&&L_fJLTC, &&L_fJLTEC, &&L_fJGTC, &&L_fJGTEC, 
		&&L_CHKIDX, &&L_CHKIDXC, 
	};
#endif
	krbp_t *rbp = (krbp_t*)sfp0;
	DISPATCH_START(pc);

	CASE(NOP) {
		klr_NOP_t *op = (klr_NOP_t*)pc;
		OPEXEC_NOP(); pc++;
		GOTO_NEXT();
	} 
	CASE(THCODE) {
		klr_THCODE_t *op = (klr_THCODE_t*)pc;
		OPEXEC_THCODE(op->th); pc++;
		GOTO_NEXT();
	} 
	CASE(ENTER) {
		klr_ENTER_t *op = (klr_ENTER_t*)pc;
		OPEXEC_ENTER(); pc++;
		GOTO_NEXT();
	} 
	CASE(EXIT) {
		klr_EXIT_t *op = (klr_EXIT_t*)pc;
		OPEXEC_EXIT(); pc++;
		GOTO_NEXT();
	} 
	CASE(NSET) {
		klr_NSET_t *op = (klr_NSET_t*)pc;
		OPEXEC_NSET(op->a, op->n, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(NMOV) {
		klr_NMOV_t *op = (klr_NMOV_t*)pc;
		OPEXEC_NMOV(op->a, op->b, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(NMOVx) {
		klr_NMOVx_t *op = (klr_NMOVx_t*)pc;
		OPEXEC_NMOVx(op->a, op->b, op->bx, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(XNMOV) {
		klr_XNMOV_t *op = (klr_XNMOV_t*)pc;
		OPEXEC_XNMOV(op->a, op->ax, op->b, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(NEW) {
		klr_NEW_t *op = (klr_NEW_t*)pc;
		OPEXEC_NEW(op->a, op->p, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(NULL) {
		klr_NULL_t *op = (klr_NULL_t*)pc;
		OPEXEC_NULL(op->a, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(BOX) {
		klr_BOX_t *op = (klr_BOX_t*)pc;
		OPEXEC_BOX(op->a, op->b, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(UNBOX) {
		klr_UNBOX_t *op = (klr_UNBOX_t*)pc;
		OPEXEC_UNBOX(op->a, op->b, op->ty); pc++;
		GOTO_NEXT();
	} 
	CASE(CALL) {
		klr_CALL_t *op = (klr_CALL_t*)pc;
		OPEXEC_CALL(op->uline, op->thisidx, op->espshift, op->tyo); pc++;
		GOTO_NEXT();
	} 
	CASE(RET) {
		klr_RET_t *op = (klr_RET_t*)pc;
		OPEXEC_RET(); pc++;
		GOTO_NEXT();
	} 
	CASE(NCALL) {
		klr_NCALL_t *op = (klr_NCALL_t*)pc;
		OPEXEC_NCALL(); pc++;
		GOTO_NEXT();
	} 
	CASE(BNOT) {
		klr_BNOT_t *op = (klr_BNOT_t*)pc;
		OPEXEC_BNOT(op->c, op->a); pc++;
		GOTO_NEXT();
	} 
	CASE(JMP) {
		klr_JMP_t *op = (klr_JMP_t*)pc;
		OPEXEC_JMP(pc = op->jumppc, JUMP); pc++;
		GOTO_NEXT();
	} 
	CASE(JMPF) {
		klr_JMPF_t *op = (klr_JMPF_t*)pc;
		OPEXEC_JMPF(pc = op->jumppc, JUMP, op->a); pc++;
		GOTO_NEXT();
	} 
	CASE(SAFEPOINT) {
		klr_SAFEPOINT_t *op = (klr_SAFEPOINT_t*)pc;
		OPEXEC_SAFEPOINT(op->espshift); pc++;
		GOTO_NEXT();
	} 
	CASE(ERROR) {
		klr_ERROR_t *op = (klr_ERROR_t*)pc;
		OPEXEC_ERROR(op->start, op->msg); pc++;
		GOTO_NEXT();
	} 
	CASE(NNMOV) {
		klr_NNMOV_t *op = (klr_NNMOV_t*)pc;
		OPEXEC_NNMOV(op->a, op->b, op->c, op->d); pc++;
		GOTO_NEXT();
	} 
	CASE(NSET2) {
		klr_NSET2_t *op = (klr_NSET2_t*)pc;
		OPEXEC_NSET2(op->a, op->n, op->n2); pc++;
		GOTO_NEXT();
	} 
	CASE(NSET3) {
		klr_NSET3_t *op = (klr_NSET3_t*)pc;
		OPEXEC_NSET3(op->a, op->n, op->n2, op->n3); pc++;
		GOTO_NEXT();
	} 
	CASE(NSET4) {
		klr_NSET4_t *op = (klr_NSET4_t*)pc;
		OPEXEC_NSET4(op->a, op->n, op->n2, op->n3, op->n4); pc++;
		GOTO_NEXT();
	} 
	CASE(iINC) {
		klr_iINC_t *op = (klr_iINC_t*)pc;
		OPEXEC_iINC(op->a); pc++;
		GOTO_NEXT();
	} 
	CASE(iDEC) {
		klr_iDEC_t *op = (klr_iDEC_t*)pc;
		OPEXEC_iDEC(op->a); pc++;
		GOTO_NEXT();
	} 
	CASE(bNUL) {
		klr_bNUL_t *op = (klr_bNUL_t*)pc;
		OPEXEC_bNUL(op->c, op->a); pc++;
		GOTO_NEXT();
	} 
	CASE(bNN) {
		klr_bNN_t *op = (klr_bNN_t*)pc;
		OPEXEC_bNN(op->c, op->a); pc++;
		GOTO_NEXT();
	} 
	CASE(iNEG) {
		klr_iNEG_t *op = (klr_iNEG_t*)pc;
		OPEXEC_iNEG(op->c, op->a); pc++;
		GOTO_NEXT();
	} 
	CASE(fNEG) {
		klr_fNEG_t *op = (klr_fNEG_t*)pc;
		OPEXEC_fNEG(op->c, op->a); pc++;
		GOTO_NEXT();
	} 
	CASE(iADD) {
		klr_iADD_t *op = (klr_iADD_t*)pc;
		OPEXEC_iADD(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iSUB) {
		klr_iSUB_t *op = (klr_iSUB_t*)pc;
		OPEXEC_iSUB(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iMUL) {
		klr_iMUL_t *op = (klr_iMUL_t*)pc;
		OPEXEC_iMUL(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iDIV) {
		klr_iDIV_t *op = (klr_iDIV_t*)pc;
		OPEXEC_iDIV(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iMOD) {
		klr_iMOD_t *op = (klr_iMOD_t*)pc;
		OPEXEC_iMOD(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iEQ) {
		klr_iEQ_t *op = (klr_iEQ_t*)pc;
		OPEXEC_iEQ(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iNEQ) {
		klr_iNEQ_t *op = (klr_iNEQ_t*)pc;
		OPEXEC_iNEQ(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iLT) {
		klr_iLT_t *op = (klr_iLT_t*)pc;
		OPEXEC_iLT(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iLTE) {
		klr_iLTE_t *op = (klr_iLTE_t*)pc;
		OPEXEC_iLTE(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iGT) {
		klr_iGT_t *op = (klr_iGT_t*)pc;
		OPEXEC_iGT(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iGTE) {
		klr_iGTE_t *op = (klr_iGTE_t*)pc;
		OPEXEC_iGTE(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iAND) {
		klr_iAND_t *op = (klr_iAND_t*)pc;
		OPEXEC_iAND(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iOR) {
		klr_iOR_t *op = (klr_iOR_t*)pc;
		OPEXEC_iOR(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iXOR) {
		klr_iXOR_t *op = (klr_iXOR_t*)pc;
		OPEXEC_iXOR(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iLSFT) {
		klr_iLSFT_t *op = (klr_iLSFT_t*)pc;
		OPEXEC_iLSFT(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iRSFT) {
		klr_iRSFT_t *op = (klr_iRSFT_t*)pc;
		OPEXEC_iRSFT(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iADDC) {
		klr_iADDC_t *op = (klr_iADDC_t*)pc;
		OPEXEC_iADDC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iSUBC) {
		klr_iSUBC_t *op = (klr_iSUBC_t*)pc;
		OPEXEC_iSUBC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iMULC) {
		klr_iMULC_t *op = (klr_iMULC_t*)pc;
		OPEXEC_iMULC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iDIVC) {
		klr_iDIVC_t *op = (klr_iDIVC_t*)pc;
		OPEXEC_iDIVC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iMODC) {
		klr_iMODC_t *op = (klr_iMODC_t*)pc;
		OPEXEC_iMODC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iEQC) {
		klr_iEQC_t *op = (klr_iEQC_t*)pc;
		OPEXEC_iEQC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iNEQC) {
		klr_iNEQC_t *op = (klr_iNEQC_t*)pc;
		OPEXEC_iNEQC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iLTC) {
		klr_iLTC_t *op = (klr_iLTC_t*)pc;
		OPEXEC_iLTC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iLTEC) {
		klr_iLTEC_t *op = (klr_iLTEC_t*)pc;
		OPEXEC_iLTEC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iGTC) {
		klr_iGTC_t *op = (klr_iGTC_t*)pc;
		OPEXEC_iGTC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iGTEC) {
		klr_iGTEC_t *op = (klr_iGTEC_t*)pc;
		OPEXEC_iGTEC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iANDC) {
		klr_iANDC_t *op = (klr_iANDC_t*)pc;
		OPEXEC_iANDC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iORC) {
		klr_iORC_t *op = (klr_iORC_t*)pc;
		OPEXEC_iORC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iXORC) {
		klr_iXORC_t *op = (klr_iXORC_t*)pc;
		OPEXEC_iXORC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iLSFTC) {
		klr_iLSFTC_t *op = (klr_iLSFTC_t*)pc;
		OPEXEC_iLSFTC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iRSFTC) {
		klr_iRSFTC_t *op = (klr_iRSFTC_t*)pc;
		OPEXEC_iRSFTC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(fADD) {
		klr_fADD_t *op = (klr_fADD_t*)pc;
		OPEXEC_fADD(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(fSUB) {
		klr_fSUB_t *op = (klr_fSUB_t*)pc;
		OPEXEC_fSUB(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(fMUL) {
		klr_fMUL_t *op = (klr_fMUL_t*)pc;
		OPEXEC_fMUL(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(fDIV) {
		klr_fDIV_t *op = (klr_fDIV_t*)pc;
		OPEXEC_fDIV(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(fEQ) {
		klr_fEQ_t *op = (klr_fEQ_t*)pc;
		OPEXEC_fEQ(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(fNEQ) {
		klr_fNEQ_t *op = (klr_fNEQ_t*)pc;
		OPEXEC_fNEQ(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(fLT) {
		klr_fLT_t *op = (klr_fLT_t*)pc;
		OPEXEC_fLT(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(fLTE) {
		klr_fLTE_t *op = (klr_fLTE_t*)pc;
		OPEXEC_fLTE(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(fGT) {
		klr_fGT_t *op = (klr_fGT_t*)pc;
		OPEXEC_fGT(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(fGTE) {
		klr_fGTE_t *op = (klr_fGTE_t*)pc;
		OPEXEC_fGTE(op->c, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(fADDC) {
		klr_fADDC_t *op = (klr_fADDC_t*)pc;
		OPEXEC_fADDC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(fSUBC) {
		klr_fSUBC_t *op = (klr_fSUBC_t*)pc;
		OPEXEC_fSUBC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(fMULC) {
		klr_fMULC_t *op = (klr_fMULC_t*)pc;
		OPEXEC_fMULC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(fDIVC) {
		klr_fDIVC_t *op = (klr_fDIVC_t*)pc;
		OPEXEC_fDIVC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(fEQC) {
		klr_fEQC_t *op = (klr_fEQC_t*)pc;
		OPEXEC_fEQC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(fNEQC) {
		klr_fNEQC_t *op = (klr_fNEQC_t*)pc;
		OPEXEC_fNEQC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(fLTC) {
		klr_fLTC_t *op = (klr_fLTC_t*)pc;
		OPEXEC_fLTC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(fLTEC) {
		klr_fLTEC_t *op = (klr_fLTEC_t*)pc;
		OPEXEC_fLTEC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(fGTC) {
		klr_fGTC_t *op = (klr_fGTC_t*)pc;
		OPEXEC_fGTC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(fGTEC) {
		klr_fGTEC_t *op = (klr_fGTEC_t*)pc;
		OPEXEC_fGTEC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(OSET) {
		klr_OSET_t *op = (klr_OSET_t*)pc;
		OPEXEC_OSET(op->a, op->o); pc++;
		GOTO_NEXT();
	} 
	CASE(OMOV) {
		klr_OMOV_t *op = (klr_OMOV_t*)pc;
		OPEXEC_OMOV(op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(OOMOV) {
		klr_OOMOV_t *op = (klr_OOMOV_t*)pc;
		OPEXEC_OOMOV(op->a, op->b, op->c, op->d); pc++;
		GOTO_NEXT();
	} 
	CASE(ONMOV) {
		klr_ONMOV_t *op = (klr_ONMOV_t*)pc;
		OPEXEC_ONMOV(op->a, op->b, op->c, op->d); pc++;
		GOTO_NEXT();
	} 
	CASE(OSET2) {
		klr_OSET2_t *op = (klr_OSET2_t*)pc;
		OPEXEC_OSET2(op->a, op->v, op->v2); pc++;
		GOTO_NEXT();
	} 
	CASE(OSET3) {
		klr_OSET3_t *op = (klr_OSET3_t*)pc;
		OPEXEC_OSET3(op->a, op->v, op->v2, op->v3); pc++;
		GOTO_NEXT();
	} 
	CASE(OSET4) {
		klr_OSET4_t *op = (klr_OSET4_t*)pc;
		OPEXEC_OSET4(op->a, op->v, op->v2, op->v3, op->v4); pc++;
		GOTO_NEXT();
	} 
	CASE(CHKSTACK) {
		klr_CHKSTACK_t *op = (klr_CHKSTACK_t*)pc;
		OPEXEC_CHKSTACK(op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(LDMTD) {
		klr_LDMTD_t *op = (klr_LDMTD_t*)pc;
		OPEXEC_LDMTD(op->thisidx, op->loadmtd, op->cache, op->mtd); pc++;
		GOTO_NEXT();
	} 
	CASE(SCALL) {
		klr_SCALL_t *op = (klr_SCALL_t*)pc;
		OPEXEC_SCALL(op->uline, op->thisidx, op->espshift, op->mtd, op->tyo); pc++;
		GOTO_NEXT();
	} 
	CASE(VCALL) {
		klr_VCALL_t *op = (klr_VCALL_t*)pc;
		OPEXEC_VCALL(op->uline, op->thisidx, op->espshift, op->mtd, op->tyo); pc++;
		GOTO_NEXT();
	} 
	CASE(VCALL_) {
		klr_VCALL__t *op = (klr_VCALL__t*)pc;
		OPEXEC_VCALL_(op->uline, op->thisidx, op->espshift, op->mtd, op->tyo); pc++;
		GOTO_NEXT();
	} 
	CASE(iCAST) {
		klr_iCAST_t *op = (klr_iCAST_t*)pc;
		OPEXEC_iCAST(op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(fCAST) {
		klr_fCAST_t *op = (klr_fCAST_t*)pc;
		OPEXEC_fCAST(op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(JMP_) {
		klr_JMP__t *op = (klr_JMP__t*)pc;
		OPEXEC_JMP_(pc = op->jumppc, JUMP); pc++;
		GOTO_NEXT();
	} 
	CASE(NGETIDX) {
		klr_NGETIDX_t *op = (klr_NGETIDX_t*)pc;
		OPEXEC_NGETIDX(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(NSETIDX) {
		klr_NSETIDX_t *op = (klr_NSETIDX_t*)pc;
		OPEXEC_NSETIDX(op->c, op->a, op->n, op->v); pc++;
		GOTO_NEXT();
	} 
	CASE(NGETIDXC) {
		klr_NGETIDXC_t *op = (klr_NGETIDXC_t*)pc;
		OPEXEC_NGETIDXC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(NSETIDXC) {
		klr_NSETIDXC_t *op = (klr_NSETIDXC_t*)pc;
		OPEXEC_NSETIDXC(op->c, op->a, op->n, op->v); pc++;
		GOTO_NEXT();
	} 
	CASE(OGETIDX) {
		klr_OGETIDX_t *op = (klr_OGETIDX_t*)pc;
		OPEXEC_OGETIDX(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(OSETIDX) {
		klr_OSETIDX_t *op = (klr_OSETIDX_t*)pc;
		OPEXEC_OSETIDX(op->c, op->a, op->n, op->v); pc++;
		GOTO_NEXT();
	} 
	CASE(OGETIDXC) {
		klr_OGETIDXC_t *op = (klr_OGETIDXC_t*)pc;
		OPEXEC_OGETIDXC(op->c, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(OSETIDXC) {
		klr_OSETIDXC_t *op = (klr_OSETIDXC_t*)pc;
		OPEXEC_OSETIDXC(op->c, op->a, op->n, op->v); pc++;
		GOTO_NEXT();
	} 
	CASE(bJNUL) {
		klr_bJNUL_t *op = (klr_bJNUL_t*)pc;
		OPEXEC_bJNUL(pc = op->jumppc, JUMP, op->a); pc++;
		GOTO_NEXT();
	} 
	CASE(bJNN) {
		klr_bJNN_t *op = (klr_bJNN_t*)pc;
		OPEXEC_bJNN(pc = op->jumppc, JUMP, op->a); pc++;
		GOTO_NEXT();
	} 
	CASE(bJNOT) {
		klr_bJNOT_t *op = (klr_bJNOT_t*)pc;
		OPEXEC_bJNOT(pc = op->jumppc, JUMP, op->a); pc++;
		GOTO_NEXT();
	} 
	CASE(iJEQ) {
		klr_iJEQ_t *op = (klr_iJEQ_t*)pc;
		OPEXEC_iJEQ(pc = op->jumppc, JUMP, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iJNEQ) {
		klr_iJNEQ_t *op = (klr_iJNEQ_t*)pc;
		OPEXEC_iJNEQ(pc = op->jumppc, JUMP, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iJLT) {
		klr_iJLT_t *op = (klr_iJLT_t*)pc;
		OPEXEC_iJLT(pc = op->jumppc, JUMP, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iJLTE) {
		klr_iJLTE_t *op = (klr_iJLTE_t*)pc;
		OPEXEC_iJLTE(pc = op->jumppc, JUMP, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iJGT) {
		klr_iJGT_t *op = (klr_iJGT_t*)pc;
		OPEXEC_iJGT(pc = op->jumppc, JUMP, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iJGTE) {
		klr_iJGTE_t *op = (klr_iJGTE_t*)pc;
		OPEXEC_iJGTE(pc = op->jumppc, JUMP, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(iJEQC) {
		klr_iJEQC_t *op = (klr_iJEQC_t*)pc;
		OPEXEC_iJEQC(pc = op->jumppc, JUMP, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iJNEQC) {
		klr_iJNEQC_t *op = (klr_iJNEQC_t*)pc;
		OPEXEC_iJNEQC(pc = op->jumppc, JUMP, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iJLTC) {
		klr_iJLTC_t *op = (klr_iJLTC_t*)pc;
		OPEXEC_iJLTC(pc = op->jumppc, JUMP, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iJLTEC) {
		klr_iJLTEC_t *op = (klr_iJLTEC_t*)pc;
		OPEXEC_iJLTEC(pc = op->jumppc, JUMP, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iJGTC) {
		klr_iJGTC_t *op = (klr_iJGTC_t*)pc;
		OPEXEC_iJGTC(pc = op->jumppc, JUMP, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(iJGTEC) {
		klr_iJGTEC_t *op = (klr_iJGTEC_t*)pc;
		OPEXEC_iJGTEC(pc = op->jumppc, JUMP, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(fJEQ) {
		klr_fJEQ_t *op = (klr_fJEQ_t*)pc;
		OPEXEC_fJEQ(pc = op->jumppc, JUMP, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(fJNEQ) {
		klr_fJNEQ_t *op = (klr_fJNEQ_t*)pc;
		OPEXEC_fJNEQ(pc = op->jumppc, JUMP, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(fJLT) {
		klr_fJLT_t *op = (klr_fJLT_t*)pc;
		OPEXEC_fJLT(pc = op->jumppc, JUMP, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(fJLTE) {
		klr_fJLTE_t *op = (klr_fJLTE_t*)pc;
		OPEXEC_fJLTE(pc = op->jumppc, JUMP, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(fJGT) {
		klr_fJGT_t *op = (klr_fJGT_t*)pc;
		OPEXEC_fJGT(pc = op->jumppc, JUMP, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(fJGTE) {
		klr_fJGTE_t *op = (klr_fJGTE_t*)pc;
		OPEXEC_fJGTE(pc = op->jumppc, JUMP, op->a, op->b); pc++;
		GOTO_NEXT();
	} 
	CASE(fJEQC) {
		klr_fJEQC_t *op = (klr_fJEQC_t*)pc;
		OPEXEC_fJEQC(pc = op->jumppc, JUMP, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(fJNEQC) {
		klr_fJNEQC_t *op = (klr_fJNEQC_t*)pc;
		OPEXEC_fJNEQC(pc = op->jumppc, JUMP, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(fJLTC) {
		klr_fJLTC_t *op = (klr_fJLTC_t*)pc;
		OPEXEC_fJLTC(pc = op->jumppc, JUMP, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(fJLTEC) {
		klr_fJLTEC_t *op = (klr_fJLTEC_t*)pc;
		OPEXEC_fJLTEC(pc = op->jumppc, JUMP, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(fJGTC) {
		klr_fJGTC_t *op = (klr_fJGTC_t*)pc;
		OPEXEC_fJGTC(pc = op->jumppc, JUMP, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(fJGTEC) {
		klr_fJGTEC_t *op = (klr_fJGTEC_t*)pc;
		OPEXEC_fJGTEC(pc = op->jumppc, JUMP, op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(CHKIDX) {
		klr_CHKIDX_t *op = (klr_CHKIDX_t*)pc;
		OPEXEC_CHKIDX(op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	CASE(CHKIDXC) {
		klr_CHKIDXC_t *op = (klr_CHKIDXC_t*)pc;
		OPEXEC_CHKIDXC(op->a, op->n); pc++;
		GOTO_NEXT();
	} 
	DISPATCH_END(pc);
	L_RETURN:;
	return pc;
}

#endif /* CLASSICVM_GEN_H */
