#include <assert.h>
#include <stdio.h>
#include "FuelVM.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline const char *Opcode2String(ByteCode *code)
{
	OPCODE opcode = GetOpcode(code);
	switch(opcode) {
#define CASE(X) case OPCODE_##X: return #X;
	BYTECODE_LIST(CASE)
#undef CASE
	default:
		assert(0 && "unreachable");
	}
	return "Undef";
}

#define CONCAT(x, y) CONCAT2(x, y)
#define CONCAT2(x, y) x##y

#define FMT_VMRegister "%s:%d"
#define FMT_Address    "%s:%d"
#define FMT_uint       "%s:%d"
#define FMT_ushort     "%s:%d"
#define FMT_uchar      "%s:%d"
#define FMT_TypeId     "%s:%d"
#define FMT_kMethodPtr "%s:%p"
#define FMT_kObjectPtr "%s:%p"
#define FMT_SValue     "%s:0x%llx"
#define FMT_Cache      "%s:%p"
#define FMT_IArray     "%s:%p"
#define FMT_TestFunc   "%s:%p"
#define FMT_uintptr_t  "%s:%ld"

#define CONV_DEFAULT(OPCODE, F)    (((OP##OPCODE *)(code))->F)
#define CONV_VMRegister(OPCODE, F) "Reg", CONV_DEFAULT(OPCODE,F)
#define CONV_Address(OPCODE, F)    "Addr", ((int)((ByteCode *)(((OP##OPCODE *)(code))->F)-base))
#define CONV_uint(OPCODE, F)       "uint", CONV_DEFAULT(OPCODE,F)
#define CONV_ushort(OPCODE, F)     "ushort", CONV_DEFAULT(OPCODE,F)
#define CONV_uchar(OPCODE, F)      "uchar",  CONV_DEFAULT(OPCODE,F)
#define CONV_TypeId(OPCODE, F)     "Type", CONV_DEFAULT(OPCODE,F)
#define CONV_kMethodPtr(OPCODE, F) "MtdObj", CONV_DEFAULT(OPCODE,F)
#define CONV_kObjectPtr(OPCODE, F) "Obj", CONV_DEFAULT(OPCODE,F)
#define CONV_Cache(OPCODE, F)      "Cache", CONV_DEFAULT(OPCODE,F)
#define CONV_IArray(OPCODE, F)     "IAry", CONV_DEFAULT(OPCODE,F)
#define CONV_TestFunc(OPCODE, F)   "TestF", CONV_DEFAULT(OPCODE,F)
#define CONV_SValue(OPCODE, F)     "SVal", ((unsigned long long) ((OP##OPCODE *)(code))->F.bits)
#define CONV_uintptr_t(OPCODE, F)  "uline", ((unsigned long)((OP##OPCODE *)(code))->F)

#define DUMP(OPCODE, T, F) CONCAT(CONV_, T)(OPCODE, F)

#define OP_0(OPCODE) fprintf(stderr, "%2d: %s\n", index, Opcode2String(code))
#define OP_1(OPCODE, T0, A0) \
	fprintf(stderr, "%2d: %s "\
			CONCAT(FMT_, T0) "\n",\
			index, Opcode2String(code), DUMP(OPCODE, T0, A0))

#define OP_2(OPCODE, T0, A0, T1, A1) \
	fprintf(stderr, "%2d: %s "\
			CONCAT(FMT_, T0) " " CONCAT(FMT_, T1) "\n",\
			index, Opcode2String(code), DUMP(OPCODE, T0, A0), DUMP(OPCODE, T1, A1))

#define OP_3(OPCODE, T0, A0, T1, A1, T2, A2) \
	fprintf(stderr, "%2d: %s "\
			CONCAT(FMT_, T0) " " CONCAT(FMT_, T1) " " CONCAT(FMT_, T2) "\n",\
			index, Opcode2String(code),\
			DUMP(OPCODE, T0, A0), DUMP(OPCODE, T1, A1), DUMP(OPCODE, T2, A2))

#define OP_4(OPCODE, T0, A0, T1, A1, T2, A2, T3, A3) \
	fprintf(stderr, "%2d: %s "\
			CONCAT(FMT_, T0) " " CONCAT(FMT_, T1) " " CONCAT(FMT_, T2) " " CONCAT(FMT_, T3) "\n",\
			index, Opcode2String(code),\
			DUMP(OPCODE, T0, A0), DUMP(OPCODE, T1, A1), DUMP(OPCODE, T2, A2), DUMP(OPCODE, T3, A3))

static void dump(ByteCode *base, ByteCode *code, unsigned index)
{
	OPCODE opcode = GetOpcode(code);
	switch(opcode) {
#define CASE(X) case OPCODE_##X: DUMP_##X(CONCAT(OP_, OPFIELDSIZE_##X)); break;
	BYTECODE_LIST(CASE)
#undef CASE
	default:
		assert(0 && "unreachable");
	}
}

void ByteCode_Dump(ByteCode *code)
{
	ByteCode *pc  = code;
	ByteCode *end = pc + ((OPThreadedCode *) code)->CodeSize;
	while(pc < end) {
		dump(code, pc, pc - code);
		pc++;
	}
}

static void CodeThreading(ByteCode *code, void **JUMP_TABLE)
{
	unsigned codesize = ((OPThreadedCode *) code)->CodeSize;
	ByteCode *pc  = code + 2;
	ByteCode *end = code + codesize;
	while(pc < end) {
		((LIRHeader *) pc)->codeaddr = JUMP_TABLE[GetOpcode(pc)];
		pc++;
	}
	((LIRHeader *) code)->opcode = OPCODE_Prologue;
}

static kObject *CreateFunction(SValue *Stack, IArray StackLayout)
{
	assert(0 && "TODO");
	return 0;
}

static SValue CallMethodWithCache(KonohaContext *kctx, unsigned ParamSize, void *Cache, kfileline_t uline)
{
	SValue Val = {};
	assert(0 && "TODO");
	return Val;
}

static void CallMethod(KonohaContext *kctx, kMethod *Mtd, uchar ParamSize, kObject *object, kfileline_t uline)
{
	KonohaStack *stack_top = kctx->esp - ParamSize;
	KonohaStack *sfp = stack_top + K_CALLDELTA;
	((KonohaContextVar *)kctx)->esp = sfp + ParamSize;
	KStackSetUnboxValue(sfp[K_MTDIDX].calledMethod, Mtd);
	KStackSetUnboxValue(sfp[K_RTNIDX].calledFileLine, uline);
	KStackSetObjectValue(sfp[K_RTNIDX].asObject, object);
	Mtd->invokeKMethodFunc(kctx, sfp);
	//if(IsUnboxed)
	//	Val.ival = sfp[K_RTNIDX].intValue;
	//else
	//	Val.obj  = sfp[K_RTNIDX].asObject;
	((KonohaContextVar *)kctx)->esp = stack_top;
	//return Val;
}

static void PushUnboxedValue(KonohaContext *kctx, SValue Val)
{
	KonohaStack *sp = kctx->esp;
	KStackSetUnboxValue(sp[0+K_CALLDELTA].unboxValue, Val.bits);
	((KonohaContextVar *)kctx)->esp = kctx->esp + 1;
}

static void PushBoxedValue(KonohaContext *kctx, SValue Val)
{
	KonohaStack *sp = kctx->esp;
	KStackSetObjectValue(sp[0+K_CALLDELTA].asObject, (kObject *) Val.ptr);
	((KonohaContextVar *)kctx)->esp = kctx->esp + 1;
}

static SValue PopUnboxedValue(KonohaContext *kctx)
{
	SValue Val = {};
	KonohaStack *sfp = kctx->esp + K_CALLDELTA;
	Val.bits = sfp[K_RTNIDX].unboxValue;
	return Val;
}

static SValue PopBoxedValue(KonohaContext *kctx)
{
	SValue Val = {};
	KonohaStack *sfp = kctx->esp + K_CALLDELTA;
	Val.obj = sfp[K_RTNIDX].asObject;
	return Val;
}

static void RaiseError(KonohaContext *kctx, KonohaStack *sfp, kString *ErrorInfo, int exception, int fault, kfileline_t uline)
{
	KStackSetUnboxValue(sfp[K_RTNIDX].calledFileLine, uline);
	KLIB KRuntime_raise(kctx, exception, fault, ErrorInfo, sfp);
}

#define CompileTimeAssert(...)
#define Yield(Reg, Stack)  (void)Reg; (void)Stack; assert(0 && "TODO")
#define Try_(Catch, Finaly, Stack) (void)Catch; (void)Stack; (void)Finaly; assert(0 && "TODO")

static kObject *CreateInstance(KonohaContext *kctx, uintptr_t Conf, TypeId Type)
{
	KClass *ct = KClass_(Type);
	kObject *obj = KLIB new_kObject(kctx, 0, ct, Conf);
	return obj;
}

static void AppendCache(TestFunc Func, SValue Val, void *Cache)
{
	assert(0 && "TODO");
}

#define HOOKFUNC_DECL(X) static void hook_##X(OP##X *PC, SValue *Reg) {}
#define HOOKFUNC(X) hook_##X((OP##X *)PC, Reg);
BYTECODE_LIST(HOOKFUNC_DECL);
#define LABELPTR(X) &&L_##X,
#define CASE(X)       L_##X: HOOKFUNC(X); /*OPEXEC_##X(PC);*//*dump(code, PC, PC - code);*/
#define DISPATCH_BEGIN(CODE) goto *JUMP_TABLE[GetOpcode(CODE)]
#define DISPATCH_END()
#define DISPATCH_JUMPTO(PC)  goto *(((LIRHeader *)(PC))->codeaddr)
#define DISPATCH_NEXT(PC)    ++PC; DISPATCH_JUMPTO(PC)

void FuelVM_Exec(KonohaContext *kctx, KonohaStack *Stack, ByteCode *code)
{
	register ByteCode *PC = code;
	SValue Reg[FUELVM_REGISTER_SIZE];
	static void *JUMP_TABLE[] = {
		BYTECODE_LIST(LABELPTR)
	};
	DISPATCH_BEGIN(PC);
#if 0
	BYTECODE_LIST(CASE)
#else
	CASE(ThreadedCode) {
		uint InvokedCount = ((OPThreadedCode *)PC)->InvokedCount;
		uint CodeSize = ((OPThreadedCode *)PC)->CodeSize;
		InvokedCount += 1;
		CodeThreading(PC, JUMP_TABLE);
		((OPThreadedCode *)PC)->InvokedCount = InvokedCount;
		(void)CodeSize;
		return;
	}
	CASE(Prologue) {
		uint StackSize = ((OPPrologue *)PC)->StackSize;
		Stack += StackSize;
		DISPATCH_NEXT(PC);
	}
	CASE(LoadConstant) {
		VMRegister Dst = ((OPLoadConstant *)PC)->Dst;
		SValue Src = ((OPLoadConstant *)PC)->Src;
		Reg[Dst] = Src;
		DISPATCH_NEXT(PC);
	}
	CASE(LoadArgumentI) {
		VMRegister Dst = ((OPLoadArgumentI *)PC)->Dst;
		uint SrcIdx = ((OPLoadArgumentI *)PC)->SrcIdx;
		Reg[Dst].ival = Stack[SrcIdx].intValue;
		DISPATCH_NEXT(PC);
	}
	CASE(LoadArgumentO) {
		VMRegister Dst = ((OPLoadArgumentO *)PC)->Dst;
		uint SrcIdx = ((OPLoadArgumentO *)PC)->SrcIdx;
		Reg[Dst].obj = Stack[SrcIdx].asObject;
		DISPATCH_NEXT(PC);
	}
	CASE(LoadLocal) {
		VMRegister Dst = ((OPLoadLocal *)PC)->Dst;
		VMRegister Src = ((OPLoadLocal *)PC)->Src;
		Reg[Dst] = Reg[Src];
		DISPATCH_NEXT(PC);
	}
	CASE(LoadField) {
		VMRegister Dst = ((OPLoadField *)PC)->Dst;
		VMRegister Src = ((OPLoadField *)PC)->Src;
		uint FieldIdx = ((OPLoadField *)PC)->FieldIdx;
		Reg[Dst].bits = Reg[Src].obj->fieldUnboxItems[FieldIdx];
		DISPATCH_NEXT(PC);
	}
	CASE(StoreLocal) {
		VMRegister Dst = ((OPStoreLocal *)PC)->Dst;
		VMRegister Src = ((OPStoreLocal *)PC)->Src;
		Reg[Dst] = Reg[Src];
		DISPATCH_NEXT(PC);
	}
	CASE(StoreField) {
		VMRegister Dst = ((OPStoreField *)PC)->Dst;
		uint FieldIdx = ((OPStoreField *)PC)->FieldIdx;
		VMRegister Src = ((OPStoreField *)PC)->Src;
		((kObjectVar *)Reg[Dst].obj)->fieldUnboxItems[FieldIdx] = Reg[Src].bits;
		DISPATCH_NEXT(PC);
	}
	CASE(New) {
		VMRegister Dst = ((OPNew *)PC)->Dst;
		uintptr_t Param = ((OPNew *)PC)->Param;
		TypeId Type = ((OPNew *)PC)->Type;
		Reg[Dst].obj = CreateInstance(kctx, Param, Type);
		DISPATCH_NEXT(PC);
	}
	CASE(Call) {
		uchar ParamSize = ((OPCall *)PC)->ParamSize;
		kMethodPtr Mtd = ((OPCall *)PC)->Mtd;
		kObjectPtr Obj = ((OPCall *)PC)->Obj;
		uintptr_t uline = ((OPCall *)PC)->uline;
		CallMethod(kctx, Mtd, ParamSize, Obj, uline);
		DISPATCH_NEXT(PC);
	}
	CASE(VCall) {
		uint ParamSize = ((OPVCall *)PC)->ParamSize;
		Cache CacheInfo = ((OPVCall *)PC)->CacheInfo;
		uintptr_t uline = ((OPVCall *)PC)->uline;
		CallMethodWithCache(kctx, ParamSize, CacheInfo, uline);
		DISPATCH_NEXT(PC);
	}
	CASE(PushI) {
		VMRegister Src = ((OPPushI *)PC)->Src;
		PushUnboxedValue(kctx, Reg[Src]);
		DISPATCH_NEXT(PC);
	}
	CASE(PushO) {
		VMRegister Src = ((OPPushO *)PC)->Src;
		PushBoxedValue(kctx, Reg[Src]);
		DISPATCH_NEXT(PC);
	}
	CASE(PopI) {
		VMRegister Dst = ((OPPopI *)PC)->Dst;
		Reg[Dst] = PopUnboxedValue(kctx);
		DISPATCH_NEXT(PC);
	}
	CASE(PopO) {
		VMRegister Dst = ((OPPopO *)PC)->Dst;
		Reg[Dst] = PopBoxedValue(kctx);
		DISPATCH_NEXT(PC);
	}
	CASE(Func) {
		VMRegister Dst = ((OPFunc *)PC)->Dst;
		IArray StackLayout = ((OPFunc *)PC)->StackLayout;
		Reg[Dst].obj = CreateFunction(Reg, StackLayout);
		DISPATCH_NEXT(PC);
	}
	CASE(Test) {
		VMRegister Src = ((OPTest *)PC)->Src;
		TestFunc Func = ((OPTest *)PC)->Func;
		Cache CacheInfo = ((OPTest *)PC)->CacheInfo;
		AppendCache(Func, Reg[Src], CacheInfo);
		DISPATCH_NEXT(PC);
	}
	CASE(ReturnI) {
		VMRegister Src = ((OPReturnI *)PC)->Src;
		Stack[K_RTNIDX].intValue = Reg[Src].ival;
		return;
	}
	CASE(ReturnO) {
		VMRegister Src = ((OPReturnI *)PC)->Src;
		Stack[K_RTNIDX].asObject = Reg[Src].obj;
		return;
	}
	CASE(ReturnVoid) {
		return;
	}
	CASE(CondBrTrue) {
		VMRegister Src = ((OPCondBrTrue *)PC)->Src;
		Address Block = ((OPCondBrTrue *)PC)->Block;
		if(Reg[Src].bval) {
			PC = (ByteCode *)Block;
			DISPATCH_JUMPTO(PC);
		} else {}
		DISPATCH_NEXT(PC);
	}
	CASE(CondBrFalse) {
		VMRegister Src = ((OPCondBrFalse *)PC)->Src;
		Address Block = ((OPCondBrFalse *)PC)->Block;
		if(!Reg[Src].bval) {
			PC = (ByteCode *)Block;
			DISPATCH_JUMPTO(PC);
		} else {}
		DISPATCH_NEXT(PC);
	}
	CASE(Jump) {
		Address Block = ((OPJump *)PC)->Block;
		PC = (ByteCode *)Block;
		DISPATCH_JUMPTO(Block);
		DISPATCH_NEXT(PC);
	}
	CASE(Throw) {
		VMRegister Src = ((OPThrow *)PC)->Src;
		int exception = ((OPThrow *)PC)->exception;
		int fault = ((OPThrow *)PC)->fault;
		uintptr_t uline = ((OPThrow *)PC)->uline;
		RaiseError(kctx, Stack, (kString *) Reg[Src].obj, exception, fault, uline);
	}
	CASE(Try) {
		Address Catch = ((OPTry *)PC)->Catch;
		Address Finaly = ((OPTry *)PC)->Finaly;
		Try_(Catch, Finaly, /*$*/Stack);
	}
	CASE(Yield) {
		VMRegister Src = ((OPYield *)PC)->Src;
		Yield(Reg[Src], /*$*/Stack);
		DISPATCH_NEXT(PC);
	}
	CASE(Not) {
		VMRegister Dst = ((OPNot *)PC)->Dst;
		VMRegister Src = ((OPNot *)PC)->Src;
		CompileTimeAssert((TypeOf(Src) == int) || (TypeOf(Src) == float));
		Reg[Dst].ival = !(Reg[Src].ival);
		DISPATCH_NEXT(PC);
	}
	CASE(Neg) {
		VMRegister Dst = ((OPNeg *)PC)->Dst;
		VMRegister Src = ((OPNeg *)PC)->Src;
		CompileTimeAssert((TypeOf(Src) == int) || (TypeOf(Src) == float));
		Reg[Dst].ival = -Reg[Src].ival;
		DISPATCH_NEXT(PC);
	}
	CASE(Add) {
		VMRegister Dst = ((OPAdd *)PC)->Dst;
		VMRegister LHS = ((OPAdd *)PC)->LHS;
		VMRegister RHS = ((OPAdd *)PC)->RHS;
		CompileTimeAssert((TypeOf(LHS) == int && TypeOf(RHS) == int));
		Reg[Dst].ival = Reg[LHS].ival + Reg[RHS].ival;
		DISPATCH_NEXT(PC);
	}

	CASE(Sub) {
		VMRegister Dst = ((OPSub *)PC)->Dst;
		VMRegister LHS = ((OPSub *)PC)->LHS;
		VMRegister RHS = ((OPSub *)PC)->RHS;
		CompileTimeAssert((TypeOf(LHS) == int && TypeOf(RHS) == int));
		Reg[Dst].ival = Reg[LHS].ival - Reg[RHS].ival;
		DISPATCH_NEXT(PC);
	}

	CASE(Mul) {
		VMRegister Dst = ((OPMul *)PC)->Dst;
		VMRegister LHS = ((OPMul *)PC)->LHS;
		VMRegister RHS = ((OPMul *)PC)->RHS;
		CompileTimeAssert((TypeOf(LHS) == int && TypeOf(RHS) == int));
		Reg[Dst].ival = Reg[LHS].ival * Reg[RHS].ival;
		DISPATCH_NEXT(PC);
	}

	CASE(Div) {
		VMRegister Dst = ((OPDiv *)PC)->Dst;
		VMRegister LHS = ((OPDiv *)PC)->LHS;
		VMRegister RHS = ((OPDiv *)PC)->RHS;
		CompileTimeAssert((TypeOf(LHS) == int && TypeOf(RHS) == int));
		Reg[Dst].ival = Reg[LHS].ival / Reg[RHS].ival;
		DISPATCH_NEXT(PC);
	}

	CASE(Mod) {
		VMRegister Dst = ((OPMod *)PC)->Dst;
		VMRegister LHS = ((OPMod *)PC)->LHS;
		VMRegister RHS = ((OPMod *)PC)->RHS;
		CompileTimeAssert(TypeOf(LHS) == int && TypeOf(RHS) == int);
		Reg[Dst].ival = Reg[LHS].ival % Reg[RHS].ival;
		DISPATCH_NEXT(PC);
	}

	CASE(LShift) {
		VMRegister Dst = ((OPLShift *)PC)->Dst;
		VMRegister LHS = ((OPLShift *)PC)->LHS;
		VMRegister RHS = ((OPLShift *)PC)->RHS;
		CompileTimeAssert(TypeOf(LHS) == int && TypeOf(RHS) == int);
		Reg[Dst].ival = Reg[LHS].ival << Reg[RHS].ival;
		DISPATCH_NEXT(PC);
	}

	CASE(RShift) {
		VMRegister Dst = ((OPRShift *)PC)->Dst;
		VMRegister LHS = ((OPRShift *)PC)->LHS;
		VMRegister RHS = ((OPRShift *)PC)->RHS;
		CompileTimeAssert(TypeOf(LHS) == int && TypeOf(RHS) == int);
		Reg[Dst].ival = Reg[LHS].ival >> Reg[RHS].ival;
		DISPATCH_NEXT(PC);
	}

	CASE(And) {
		VMRegister Dst = ((OPAnd *)PC)->Dst;
		VMRegister LHS = ((OPAnd *)PC)->LHS;
		VMRegister RHS = ((OPAnd *)PC)->RHS;
		CompileTimeAssert(TypeOf(LHS) == int && TypeOf(RHS) == int);
		Reg[Dst].ival = Reg[LHS].ival & Reg[RHS].ival;
		DISPATCH_NEXT(PC);
	}
	CASE(Or) {
		VMRegister Dst = ((OPOr *)PC)->Dst;
		VMRegister LHS = ((OPOr *)PC)->LHS;
		VMRegister RHS = ((OPOr *)PC)->RHS;
		CompileTimeAssert(TypeOf(LHS) == int && TypeOf(RHS) == int);
		Reg[Dst].ival = Reg[LHS].ival | Reg[RHS].ival;
		DISPATCH_NEXT(PC);
	}
	CASE(Xor) {
		VMRegister Dst = ((OPXor *)PC)->Dst;
		VMRegister LHS = ((OPXor *)PC)->LHS;
		VMRegister RHS = ((OPXor *)PC)->RHS;
		CompileTimeAssert(TypeOf(LHS) == int && TypeOf(RHS) == int);
		Reg[Dst].ival = Reg[LHS].ival ^ Reg[RHS].ival;
		DISPATCH_NEXT(PC);
	}

	CASE(Eq) {
		VMRegister Dst = ((OPEq *)PC)->Dst;
		VMRegister LHS = ((OPEq *)PC)->LHS;
		VMRegister RHS = ((OPEq *)PC)->RHS;
		Reg[Dst].bval = (Reg[LHS].ival == Reg[RHS].ival);
		DISPATCH_NEXT(PC);
	}

	CASE(Nq) {
		VMRegister Dst = ((OPNq *)PC)->Dst;
		VMRegister LHS = ((OPNq *)PC)->LHS;
		VMRegister RHS = ((OPNq *)PC)->RHS;
		Reg[Dst].bval = (Reg[LHS].ival != Reg[RHS].ival);
		DISPATCH_NEXT(PC);
	}

	CASE(Gt) {
		VMRegister Dst = ((OPGt *)PC)->Dst;
		VMRegister LHS = ((OPGt *)PC)->LHS;
		VMRegister RHS = ((OPGt *)PC)->RHS;
		Reg[Dst].bval = (Reg[LHS].ival >  Reg[RHS].ival);
		DISPATCH_NEXT(PC);
	}

	CASE(Ge) {
		VMRegister Dst = ((OPGe *)PC)->Dst;
		VMRegister LHS = ((OPGe *)PC)->LHS;
		VMRegister RHS = ((OPGe *)PC)->RHS;
		Reg[Dst].bval = (Reg[LHS].ival >= Reg[RHS].ival);
		DISPATCH_NEXT(PC);
	}

	CASE(Lt) {
		VMRegister Dst = ((OPLt *)PC)->Dst;
		VMRegister LHS = ((OPLt *)PC)->LHS;
		VMRegister RHS = ((OPLt *)PC)->RHS;
		Reg[Dst].bval = (Reg[LHS].ival <  Reg[RHS].ival);
		DISPATCH_NEXT(PC);
	}

	CASE(Le) {
		VMRegister Dst = ((OPLe *)PC)->Dst;
		VMRegister LHS = ((OPLe *)PC)->LHS;
		VMRegister RHS = ((OPLe *)PC)->RHS;
		Reg[Dst].bval = (Reg[LHS].ival <= Reg[RHS].ival);
		DISPATCH_NEXT(PC);
	}

	CASE(FNeg) {
		VMRegister Dst = ((OPFNeg *)PC)->Dst;
		VMRegister Src = ((OPFNeg *)PC)->Src;
		CompileTimeAssert((TypeOf(Src) == float));
		Reg[Dst].fval = -Reg[Src].fval;
		DISPATCH_NEXT(PC);
	}

	CASE(FAdd) {
		VMRegister Dst = ((OPFAdd *)PC)->Dst;
		VMRegister LHS = ((OPFAdd *)PC)->LHS;
		VMRegister RHS = ((OPFAdd *)PC)->RHS;
		CompileTimeAssert((TypeOf(LHS) == float && TypeOf(RHS) == float));
		Reg[Dst].fval = Reg[LHS].fval + Reg[RHS].fval;
		DISPATCH_NEXT(PC);
	}

	CASE(FSub) {
		VMRegister Dst = ((OPFSub *)PC)->Dst;
		VMRegister LHS = ((OPFSub *)PC)->LHS;
		VMRegister RHS = ((OPFSub *)PC)->RHS;
		CompileTimeAssert((TypeOf(LHS) == float && TypeOf(RHS) == float) ||
				(TypeOf(LHS) == float || TypeOf(RHS) == float));
		Reg[Dst].fval = Reg[LHS].fval - Reg[RHS].fval;
		DISPATCH_NEXT(PC);
	}

	CASE(FMul) {
		VMRegister Dst = ((OPFMul *)PC)->Dst;
		VMRegister LHS = ((OPFMul *)PC)->LHS;
		VMRegister RHS = ((OPFMul *)PC)->RHS;
		CompileTimeAssert((TypeOf(LHS) == float && TypeOf(RHS) == float));
		Reg[Dst].fval = Reg[LHS].fval * Reg[RHS].fval;
		DISPATCH_NEXT(PC);
	}

	CASE(FDiv) {
		VMRegister Dst = ((OPFDiv *)PC)->Dst;
		VMRegister LHS = ((OPFDiv *)PC)->LHS;
		VMRegister RHS = ((OPFDiv *)PC)->RHS;
		CompileTimeAssert((TypeOf(LHS) == float && TypeOf(RHS) == float));
		Reg[Dst].fval = Reg[LHS].fval / Reg[RHS].fval;
		DISPATCH_NEXT(PC);
	}

	CASE(FEq) {
		VMRegister Dst = ((OPFEq *)PC)->Dst;
		VMRegister LHS = ((OPFEq *)PC)->LHS;
		VMRegister RHS = ((OPFEq *)PC)->RHS;
		Reg[Dst].bval = (Reg[LHS].fval == Reg[RHS].fval);
		DISPATCH_NEXT(PC);
	}

	CASE(FNq) {
		VMRegister Dst = ((OPFNq *)PC)->Dst;
		VMRegister LHS = ((OPFNq *)PC)->LHS;
		VMRegister RHS = ((OPFNq *)PC)->RHS;
		Reg[Dst].bval = (Reg[LHS].fval != Reg[RHS].fval);
		DISPATCH_NEXT(PC);
	}

	CASE(FGt) {
		VMRegister Dst = ((OPFGt *)PC)->Dst;
		VMRegister LHS = ((OPFGt *)PC)->LHS;
		VMRegister RHS = ((OPFGt *)PC)->RHS;
		Reg[Dst].bval = (Reg[LHS].fval >  Reg[RHS].fval);
		DISPATCH_NEXT(PC);
	}

	CASE(FGe) {
		VMRegister Dst = ((OPFGe *)PC)->Dst;
		VMRegister LHS = ((OPFGe *)PC)->LHS;
		VMRegister RHS = ((OPFGe *)PC)->RHS;
		Reg[Dst].bval = (Reg[LHS].fval >= Reg[RHS].fval);
		DISPATCH_NEXT(PC);
	}

	CASE(FLt) {
		VMRegister Dst = ((OPFLt *)PC)->Dst;
		VMRegister LHS = ((OPFLt *)PC)->LHS;
		VMRegister RHS = ((OPFLt *)PC)->RHS;
		Reg[Dst].bval = (Reg[LHS].fval <  Reg[RHS].fval);
		DISPATCH_NEXT(PC);
	}
	CASE(FLe) {
		VMRegister Dst = ((OPFLe *)PC)->Dst;
		VMRegister LHS = ((OPFLe *)PC)->LHS;
		VMRegister RHS = ((OPFLe *)PC)->RHS;
		Reg[Dst].bval = (Reg[LHS].ival <= Reg[RHS].ival);
		DISPATCH_NEXT(PC);
	}
	CASE(Nop) {
		DISPATCH_NEXT(PC);
	}
#endif
	DISPATCH_END();
}

#ifdef __cplusplus
} /* extern "C" */
#endif
