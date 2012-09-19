import("sugar");
import("konoha.float");
import("llvm");
import("konoha.array");
import("konoha.assign");
import("konoha.null");
import("konoha.string");
import("konoha.class");

//enum ctype {
//	/*TY_VOID*/1,
//	/*TY_LONG*/2,
//	/*TY_INT*/3,
//	/*TY_SHORT*/4,
//	/*TY_FLOAT*/5,
//	/*TY_PTR*/6,
//	/*TY_CHAR*/7,
//	/*TY_BOOL*/8,
//	/*TY_STRING*/9,
//	/*TY_TY*/10,
//	/*TY_METHOD*/11,
//	/*TY_OBJECT*/12,
//	/*TY_HOBJECT*/13,
//	/*TY_FUNCTION*/14,
//	/*TY_FPTR*/15,
//	/*TY_CONTEXT*/16,
//	/*TY_STACK*/17,
//	/*TY_NOP*/-1 = -1
//};

int Expr.getBuild() {
	int x = getAddr();
	return System.getPointer(x, 36, 16);
}

Object Expr.getData() {
	int x = getAddr();
	int addr = System.getPointer(x, 48, 64);
	return System.convertToObject(addr);
}

int Expr.getIndex() {
	int x = getAddr();
	return System.getPointer(x, 56, 64);
}

int Expr.getNdata() {
	int x = getAddr();
	return System.getPointer(x, 56, 64);
}

int Expr.getType() {
	int x = getAddr();
	return System.getPointer(x, 32, 16);
}

Method Object.asMethod() {
	int ptr = getAddr();
	return System.convertToMethod(ptr);
}

Expr at(Expr expr, int i) {
	Array[Expr] a = expr.getCons();
	return a.get(i);
}

void Method.setFunc(int faddr) {
	int x = getAddr();
	System.setPointer(x, 32, 64, faddr);
}

String method2string(Method mtd) {
	String cname = mtd.getCname();
	String fname = mtd.getFname();
	return cname + "_" + fname;
}

void vstack_setValue(int idx, Value value) {
	if (idx < 0) {
		System.setRetVal(value);
	} else {
		System.setValue(idx, value);
	}
}

Type ctype2Type(int ctype) {
	if (ctype == /*TY_LONG*/2) {
		//TODO
		return Type.getInt64Ty();
	}
	if (ctype == /*TY_INT*/3) {
		return Type.getInt64Ty();
	}
	if (ctype == /*TY_FLOAT*/5) {
		//TODO
		return Type.getDoubleTy();
	}
	if (ctype == /*TY_SHORT*/4) {
		//TODO
		return Type.getInt16Ty();
	}
	if (ctype == /*TY_PTR*/6) {
		return Type.getInt8PtrTy();
	}
	if (ctype == /*TY_CHAR*/7) {
		return Type.getInt8Ty();
	}
	if (ctype == /*TY_BOOL*/8) {
		return Type.getInt1Ty();
	}
	if (ctype == /*TY_STRING*/9) {
		return Type.getInt8PtrTy();
	}
	if (ctype == /*TY_TY*/10) {
		//TODO
		return Type.getInt8PtrTy();
	}
	if (ctype == /*TY_OBJECT*/12) {
		return System.getTyObject();
	}
	if (ctype == /*TY_HOBJECT*/13) {
		return System.getTyHObject();
	}
	if (ctype == /*TY_METHOD*/11) {
		return System.getTyMethod();
	}
	if (ctype == /*TY_FUNCTION*/14) {
		return System.getTyFunction();
	}
	if (ctype == /*TY_FPTR*/15) {
		return System.getTyFunctionP();
	}
	if (ctype == /*TY_CONTEXT*/16) {
		return System.getTyContext();
	}
	if (ctype == /*TY_STACK*/17) {
		return System.getTyStack();
	}
	if (ctype == /*TY_VOID*/1) {
		return Type.getVoidTy();
	}
	System.p("[WARN] ctype not matched");
	return Type.getVoidTy();
}

Value createConstValue(int ctype, Object v, int i, float f) {
	if (ctype == /*TY_INT*/3) {
		return ConstantInt.getValue(ctype2Type(ctype), i);
	}
	if (ctype == /*TY_BOOL*/8) {
		return ConstantInt.getValue(ctype2Type(ctype), i);
	}
	if (ctype == /*TY_FLOAT*/5) {
		return ConstantFP.getValueFromBits(ctype2Type(ctype), i);
	}
	if (ctype == /*TY_OBJECT*/12) {
		//assert(LLVMTYPE_OBJECT != null);
		Constant val = ConstantInt.getValue(ctype2Type(/*TY_LONG*/2), v.getAddr());
		return ConstantExpr.getIntToPtr(val, ctype2Type(ctype));
	}
	if (ctype == /*TY_METHOD*/11) {
		//assert(LLVMTYPE_OBJECT != null);
		Constant val = ConstantInt.getValue(ctype2Type(/*TY_LONG*/2), v.getAddr());
		return ConstantExpr.getIntToPtr(val, ctype2Type(ctype));
	}
	if (ctype == /*TY_FUNCTION*/14) {
		//assert(LLVMTYPE_FUNCTION != null);
		Constant val = ConstantInt.getValue(ctype2Type(/*TY_LONG*/2), v.getAddr());
		return ConstantExpr.getIntToPtr(val, PointerType.get(ctype2Type(ctype)));
	}
	if (ctype == /*TY_FPTR*/15) {
		//assert(LLVMTYPE_FUNCTIONP != null);
		Constant val = ConstantInt.getValue(ctype2Type(/*TY_LONG*/2), v.getAddr());
		return ConstantExpr.getIntToPtr(val, PointerType.get(ctype2Type(ctype)));
	}
	System.p("createConstValue: [WARN]");
	return null;
}

int cid2ctype(int type) {
	if (type == getCid("Int", -1))     { return /*TY_INT*/3; }
	if (type == getCid("Boolean", -1)) { return /*TY_BOOL*/8; }
	if (type == getCid("Float", -1))   { return /*TY_FLOAT*/5; }
	if (type == getCid("void", -1)) { return /*TY_VOID*/1; }
	return /*TY_OBJECT*/12;
}

Type cid2Type(int type) {
	return ctype2Type(cid2ctype(type));
}

int type2cid(Type type) {
	if (type == ctype2Type(/*TY_INT*/3))   { return getCid("Int", -1); }
	if (type == ctype2Type(/*TY_BOOL*/8))  { return getCid("Boolean", -1); }
	if (type == ctype2Type(/*TY_FLOAT*/5)) { return getCid("Float", -1); }
	if (type == ctype2Type(/*TY_VOID*/1)) { return getCid("void", -1); }
	return getCid("Object", -1);
}

boolean ty_isUnbox(int cid) {
	if (cid == getCid("Int", -1)) { return true; }
	if (cid == getCid("Boolean", -1)) { return true; }
	if (cid == getCid("Float", -1)) { return true; }
	return false;
}

boolean isUnBoxTy(int cid) {
	return ty_isUnbox(cid);
}

boolean isBoxTy(int cid) {
	if (isUnBoxTy(cid)) {
		return false;
	} else {
		return true;
	}
}

Value getvctx() {
	Value v = System.getCTX();
	Type  t = v.getType();
	return v;
}
Value getvsfp() {
	Value v = System.getSFP();
	Type  t = v.getType();
	return v;
}

Value boxing(Value v) {
	Module m = System.getModule();
	IRBuilder builder = System.getBuilder();
	Array[Type] argsTy = new Type[3];
	argsTy.set(0, ctype2Type(/*TY_CONTEXT*/16));
	argsTy.set(1, ctype2Type(/*TY_INT*/3));
	argsTy.set(2, ctype2Type(/*TY_INT*/3));
	FunctionType fnTy = FunctionType.get(ctype2Type(/*TY_OBJECT*/12), argsTy, false);
	Function box = m.getOrInsertFunction("boxing_jit", fnTy);
	Array[Value] args = box.getArguments();

	int cid = type2cid(v.getType());
	Array[Value] params = new Value[3];
	params.set(0, getvctx());
	params.set(1, createConstValue(/*TY_INT*/3, null, cid, 0.0));
	if (v.getType() == ctype2Type(/*TY_INT*/3)) {
		params.set(2, v);
	} else if (v.getType() == ctype2Type(/*TY_BOOL*/8)) {
		Value v0 = builder.createZExt(v, ctype2Type(/*TY_LONG*/2));
		params.set(2, v0);
	} else {
		System.p("TODO float type");
		System.assert(false);
		Value v0 = builder.createZExt(v, ctype2Type(/*TY_LONG*/2));
		params.set(2, v0);
	}

	Value ret = builder.createCall(box, params);
	return ret;
}

Value llvm_getBool(boolean v) {
	if (v) {
		return ConstantInt.getValue(ctype2Type(/*TY_BOOL*/8), 1);
	} else {
		return ConstantInt.getValue(ctype2Type(/*TY_BOOL*/8), 0);
	}
}

Value llvm_getInt(int v) {
	return (ConstantInt.getValue(ctype2Type(/*TY_INT*/3),    (v)));
}

Value llvm_getFloat(float v) {
	return (ConstantInt.getValue(ctype2Type(/*TY_FLOAT*/5),  (v)));
}


//#define nc(sfpidx)    (((sfpidx) * 2) + 1)
//#define oc(sfpidx)    ((sfpidx) * 2)
//#define sfp(sfpidx)   ((sfpidx) * 2)
//#define rfp(rbpidx)   ((sfpidx) / 2)
int nc(int sfpidx)  { return (sfpidx); }
int oc(int sfpidx)  { return (sfpidx); }
int sfp(int sfpidx) { return (sfpidx); }
int rfp(int rbpidx) { return (rbpidx); }

int esp(int sfpidx, int args) { return  sfp(sfpidx + args + /*K_CALLDELTA*/4 + 1)}

/* ------------------------------------------------------------------------ */
Value create_object(int a, int index, int cid) {
	Module m = System.getModule();
	IRBuilder builder = System.getBuilder();
	Array[Type] argsTy = new Type[2];
	argsTy.set(0, ctype2Type(/*TY_CONTEXT*/16));
	argsTy.set(1, ctype2Type(/*TY_INT*/3));
	FunctionType fnTy = FunctionType.get(ctype2Type(/*TY_OBJECT*/12), argsTy, false);
	Function fnew = m.getOrInsertFunction("new_object_jit", fnTy);
	Array[Value] args = fnew.getArguments();

	Array[Value] params = new Value[2];
	Value vctx = getvctx();
	Value arg1 = createConstValue(/*TY_INT*/3, null, cid, 0.0);
	arg1.dump();
	params.set(0, vctx);
	params.set(1, arg1);
	Value ret = builder.createCall(fnew, params);
	return ret;
}

int tri(boolean cond, int a, int b) {
	if (cond) {
		return a;
	} else {
		return b;
	}
}

Value nmov_asm(int a, int ty, int b) {
	if(ty_isUnbox(ty)) {
		Value v = System.getValue(nc(b));
		vstack_setValue(nc(a), v);
		//ASM(NMOV, nc(a), nc(b), CT_(ty));
		return v;
	}
	else {
		Value v = System.getValue(oc(b));
		vstack_setValue(oc(a), v);
		return v;
	}
}

Value asm_nset(int data, int cid, int a) {
	Value v = createConstValue(cid2ctype(cid), null, data, 0.0);
	int idx = nc(a);
	vstack_setValue(idx, v);
	return v;
}

Value asm_oset(Object data, int cid, int a) {
	Value v = createConstValue(cid2ctype(cid), data, 0, 0.0);
	int idx = oc(a);
	vstack_setValue(idx, v);
	return v;
}

Value expr_asm(int a, Expr expr, int espidx) {
	//System.p("build="+expr.getBuild()+":"+expr);
	if (expr.getBuild() == TEXPR_CONST) {
		Object v = expr.getData();
		boolean isUnbox = ty_isUnbox(expr.getType());
		if(isUnbox) {
			//ASM(NSET, nc(a), (uintptr_t)N_toint(v), CT_(expr.getType()));
			System.p("FIXME TODO");
			return asm_nset(v.getAddr(), expr.getType(), a);
		}
		else {
			int addr = v.getAddr();
			System.addConstPool(v);
			return asm_oset(v, expr.getType(), a);
		}
	}
	else if(expr.getBuild() == TEXPR_NEW) {
		int index = expr.getIndex();
		int type  = expr.getType();
		//ASM(NEW, oc(a), expr.index, CT_(expr.getType()));
		return create_object(a, index, type);
	}
	else if(expr.getBuild() == TEXPR_NULL) {
		boolean isUnbox = ty_isUnbox(expr.getType());
		if(isUnbox) {
			return asm_nset(0, (expr.getType()), a);
		}
		else {
			return asm_oset(System.KLIB Knull(kctx, expr.getType()), expr.getType(), a);
		}
	}
	else if(expr.getBuild() == TEXPR_NCONST) {
		return asm_nset(expr.getNdata(), expr.getType(), a);
		//ASM(NSET, nc(a), expr.unboxValue, CT_(expr.getType()));
	}
	else if(expr.getBuild() == TEXPR_LOCAL) {
		//System.p("type="+expr.getType()+",index="+expr.getIndex());
		return nmov_asm(a, expr.getType(), expr.getIndex());
	}
	else if(expr.getBuild() == TEXPR_BLOCK) {
		assert(false);
	}
	else if(expr.getBuild() == TEXPR_FIELD) {
		assert(false);
		//kshort_t index = (kshort_t)expr.index;
		//kshort_t xindex = (kshort_t)(expr.index >> (sizeof(kshort_t)*8));
		//if(ty_isUnbox(expr.getType())) {
		//	ASM(NMOVx, nc(a), oc(index), xindex, CT_(expr.getType()));
		//}
		//else {
		//	ASM(NMOVx, oc(a), oc(index), xindex, CT_(expr.getType()));
		//}
	}
	else if(expr.getBuild() == TEXPR_BOX) {
		Expr single = expr.getSingle();
		Value v = expr_asm(a, single, espidx);
		return boxing(v);
	}
	else if(expr.getBuild() == TEXPR_UNBOX) {
		System.p("TODO");
		assert(false);
		//ASM(UNBOX, nc(a), oc(a), CT_(expr.getType()));
	}
	else if(expr.getBuild() == TEXPR_CALL) {
		Value v = call_asm(a, expr, espidx);
		if(a != espidx) {
			return nmov_asm(a, expr.getType(), espidx);
		}
		return v;
	}
	else if(expr.getBuild() == TEXPR_AND) {
		Value v = and_asm(a, expr, espidx);
		return v;
	}
	else if(expr.getBuild() == TEXPR_OR) {
		Value v = or_asm(a, expr, espidx);
		return v;
	}
	else if(expr.getBuild() == TEXPR_LET) {
		return letexpr_asm(a, expr, espidx);
	} else {
		System.p("unknown expr="+ expr.getBuild());
		System.assert(false);
	}
	//abort();
}

void storesfp(Value val, int reqtype, int sfpidx) {
	IRBuilder builder = System.getBuilder();
	Value vsfp  = getvsfp();
	if (type2cid(val.getType()) == getCid("Object", -1)) {
		Value vsfp0 = loadsfp(builder, vsfp, getCid("Object", -1), sfpidx);
		builder.createStore(val, vsfp0, false);
	} else {
		Value vsfp0 = loadsfp(builder, vsfp, reqtype, sfpidx);
		builder.createStore(val, vsfp0, false);
	}
}

void emit_param(Method mtd, Expr expr, int thisidx, int i) {
	Array[Expr] cons = expr.getCons();
	Method orig_mtd = mtd;
	int size = cons.getSize();
	int idx = thisidx + i - 1;
	if (i < cons.getSize()) {
		Expr exprN = at(expr,i);
		Value v = expr_asm(idx, exprN, idx);
		storesfp(v, exprN.getType(), idx);
		emit_param(orig_mtd, expr, thisidx, i+1);
	}
}

void asm_shift_esp(int idx) {
	IRBuilder builder = System.getBuilder();
	Value vesp = builder.createStructGEP(getvctx(), 1); /* esp */
	Value vsfp = builder.createConstInBoundsGEP1_32(getvsfp(), idx);
	builder.createStore(vsfp, vesp, false);
}

Value loadMtdFromSfp(IRBuilder builder, Value vsfp, int mtdidx) {
	Value v = loadsfp(builder, vsfp, getCid("Int", -1), mtdidx);
	return builder.createBitCast(v, PointerType.get(ctype2Type(/*TY_METHOD*/11)));
}

void prepare_call_stack(int thisidx, Value vsfp, Object def, Method mtd, int uline) {
	int rtnidx = thisidx + /*K_RTNIDX*/-4;
	int mtdidx = thisidx + /*K_MTDIDX*/-1;
	IRBuilder builder = System.getBuilder();
	if ((def).getCid() != getCid("void", -1)) {
		Value vsfp0 = loadsfp(builder, vsfp, (def).getCid(), rtnidx);
		if (ty_isUnbox(def.getCid())) {
			int unboxValue = def.unbox();
			Value vdef  = createConstValue(cid2ctype(def.getCid()), null, unboxValue, 0);
			builder.createStore(vdef, vsfp0, false);
		} else {
			Value vdef  = createConstValue(/*TY_OBJECT*/12, (def), 0, 0);
			builder.createStore(vdef, vsfp0, false);
		}
	}
	Value vsfp1 = loadMtdFromSfp(builder, vsfp, mtdidx);
	Value vmtd  = createConstValue(/*TY_METHOD*/11, (mtd), 0, 0);
	builder.createStore(vmtd, vsfp1, false);

	Value vsfp2 = loadsfp(builder, vsfp, getCid("Int", -1), rtnidx);
	Value vline = createConstValue(/*TY_INT*/3, null, uline, 0);
	builder.createStore(vline, vsfp2, false);

}

void param_setCtxSfp(Array params, int sfpidx) {
	IRBuilder builder = System.getBuilder();
	Function f = System.getFunction();
	Array[Value] args = f.getArguments();
	Value vctx = args.get(0);
	Value vsfp = args.get(1);

	params.set(0, vctx);
	if (sfpidx > 0) {
		Array[Int] arg0 = new Int[1]; arg0.set(0, llvm_getInt(sfpidx));
		Value vsfp1 = builder.createGEP(vsfp, arg0);
		params.set(1, vsfp1);
	} else {
		params.set(1, vsfp);
	}
}

boolean check_asm_int_op(String mn, int argc) {
	if (argc == 0) {
		if(mn == "opNEG") {
			return true;
		}
	}
	if (argc == 1) {
		if(mn == "+" ) { return true; }
		if(mn == "-" ) { return true; }
		if(mn == "*" ) { return true; }
		if(mn == "/" ) { return true; }
		if(mn == "%" ) { return true; }
		if(mn == "=="  ) { return true; }
		if(mn == "!="  ) { return true; }
		if(mn == "<"  ) { return true; }
		if(mn == "<=" ) { return true; }
		if(mn == ">"  ) { return true; }
		if(mn == ">=" ) { return true; }
		if(mn == "&") { return true; }
		if(mn == "|" ) { return true; }
		if(mn == "^") { return true; }
		if(mn == "<<") { return true; }
		if(mn == ">>") { return true; }
	}
	return false;
}

Value asm_int_op(String mn, Expr exprN, int local) {
	IRBuilder builder = System.getBuilder();
	if(mn == "opNEG") {
		Value va = expr_asm(local+0, at(exprN,1), local+0);
		return builder.createNeg(va);
	} else {
		Value va = expr_asm(local+0, at(exprN,1), local+0);
		Value vb = expr_asm(local+1, at(exprN,2), local+1);
		if(mn == "+" ) { return builder.createAdd(va, vb);}
		if(mn == "-" ) { return builder.createSub(va, vb);}
		if(mn == "*" ) { return builder.createMul(va, vb);}
		if(mn == "/" ) { return builder.createSDiv(va, vb);}
		if(mn == "%" ) { return builder.createSRem(va, vb);}
		if(mn == "=="  ) { return builder.createICmpEQ(va, vb);}
		if(mn == "!="  ) { return builder.createICmpNE(va, vb);}
		if(mn == "<"  ) { return builder.createICmpSLT(va, vb);}
		if(mn == "<=" ) { return builder.createICmpSLE(va, vb);}
		if(mn == ">"  ) { return builder.createICmpSGT(va, vb);}
		if(mn == ">=" ) { return builder.createICmpSGE(va, vb);}
		if(mn == "&") { return builder.createAnd( va, vb);}
		if(mn == "|" ) { return builder.createOr(va, vb);}
		if(mn == "^") { return builder.createXor(va, vb);}
		if(mn == "<<") { return builder.createShl(va, vb);}
		if(mn == ">>") { return builder.createAShr(va, vb);}
	}
	return null;
}

boolean check_asm_float_op(String mn, int argc) {
	if (argc == 0) {
		if(mn == "opNEG") {
			return true;
		}
	}
	if (argc == 1) {
		if(mn == "+" ) { return true; }
		if(mn == "-" ) { return true; }
		if(mn == "*" ) { return true; }
		if(mn == "/" ) { return true; }
		if(mn == "=="  ) { return true; }
		if(mn == "!="  ) { return true; }
		if(mn == "<"  ) { return true; }
		if(mn == "<=" ) { return true; }
		if(mn == ">"  ) { return true; }
		if(mn == ">=" ) { return true; }
	}
	return false;
}

Value asm_float_op(String mn, Expr exprN, int local) {
	IRBuilder builder = System.getBuilder();
	if(mn == "opNEG") {
		Value va = expr_asm(local+0, at(exprN,1), local+0);
		return builder.createNeg(va);
	} else {
		Value va = expr_asm(local+0, at(exprN,1), local+0);
		Value vb = expr_asm(local+1, at(exprN,2), local+1);
		if(mn == "+" ) { return builder.createFAdd(va, vb);}
		if(mn == "-" ) { return builder.createFSub(va, vb);}
		if(mn == "*" ) { return builder.createFMul(va, vb);}
		if(mn == "/" ) { return builder.createFDiv(va, vb);}
		if(mn == "=="  ) { return builder.createFCmpOEQ(va, vb);}
		if(mn == "!="  ) { return builder.createFCmpONE(va, vb);}
		if(mn == "<"  ) { return builder.createFCmpOLT(va, vb);}
		if(mn == "<=" ) { return builder.createFCmpOLE(va, vb);}
		if(mn == ">"  ) { return builder.createFCmpOGT(va, vb);}
		if(mn == ">=" ) { return builder.createFCmpOGE(va, vb);}
	}
	System.p("hi");
	return null;
}
void push_back_args(Array args, Expr expr, int local, int s, int i, int n) {
	if (i < n) {
		Value v = expr_asm(local+i, at(expr,i), local+i);
		args.set(i - s + 2, v);
		push_back_args(args, expr, local, s, i+1, n);
	}
}

Value asm_function_call(int uline, Expr expr, int thisidx, int s, Function f) {
	Array[Expr] cons = expr.getCons();
	Array[Value] args = new Value[cons.getSize()];
	IRBuilder builder = System.getBuilder();
	args.set(0, getvctx());
	args.set(1, getvsfp());
	push_back_args(args, expr, thisidx+s-1, s, s, cons.getSize());
	return builder.createCall(f, args);
}

void asm_scall(int uline, int sfpidx, int espidx, Method mtd, Object defval) {
	int thisidx = rfp(sfpidx);
	IRBuilder builder = System.getBuilder();
	Value vmtd  = createConstValue(/*TY_METHOD*/11, mtd, 0, 0.0);
	Value funcp = builder.createStructGEP(vmtd, 1);
	Value func  = builder.createLoad(funcp, false);
	prepare_call_stack(thisidx, getvsfp(), defval, mtd, uline);
	Array[Value] params = new Value[3];
	param_setCtxSfp(params, thisidx);
	params.set(2, llvm_getInt(/*K_RTNIDX*/-4));
	asm_shift_esp(espidx);
	builder.createCall(func, params);
}

Value call_asm(int a, Expr expr, int espidx) {
	Array cons = expr.getCons();
	Method mtd = cons.get(0).asMethod();
	int s = tri(mtd.isStatic(), 2, 1);
	int thisidx = espidx + /*K_CALLDELTA*/4;
	int argc = (cons).getSize() - 2;
	if(mtd.isVirtual()) {
		System.p("TODO");
		assert(false);
		//ASM(NSET, nc(thisidx-1), (intptr_t)mtd, CT_Method);
		//ASM(CALL, kmodjit.uline, sfp(thisidx), esp(espidx, argc), System.KLIB Knull(kctx, expr.getType()));
	}
	else {
		if (mtd.getCid() == getCid("Int", -1)) {
			if (check_asm_int_op(mtd.getFname(), argc)) {
				Value v = asm_int_op(mtd.getFname(), expr, thisidx + s - 1);
				vstack_setValue(nc(espidx), v);
				return v;
			}
		}
		if (mtd.getCid() == getCid("Float", -1)) {
			if (check_asm_float_op(mtd.getFname(), argc)) {
				Value v = asm_float_op(mtd.getFname(), expr, thisidx + s - 1);
				vstack_setValue(nc(espidx), v);
				return v;
			}
		}
		Function f = System.getJITCache(mtd);
		if(f.isNull()) {
			//System.p(method2string(mtd));
			emit_param(mtd, expr, thisidx, s);
			asm_scall(System.getUline(), sfp(thisidx), esp(espidx, argc), mtd, System.KLIB Knull(kctx, expr.getType()));
		}
		else {
			Value v = asm_function_call(System.getUline(), expr, thisidx, s, f);
			if (v.isNull()) {
			} else {
				vstack_setValue(nc(espidx), v);
			}
			return v;
		}
	}
	if (mtd.getReturnType() != getCid("void", -1)) {
		IRBuilder builder = System.getBuilder();
		Value v = loadsfp(builder, getvsfp(), mtd.getReturnType(), sfp(thisidx) + /*K_RTNIDX*/-4);
		Value v1 = builder.createLoad(v, false);
		vstack_setValue(tri(ty_isUnbox(mtd.getReturnType()), nc(espidx), oc(espidx)), v1);
		return v1;
	}
	return null;
}

Object Array.end() {
	Array a = this;
	int size = a.getSize();
	if (size > 0) {
		return a.get(size-1);
	}
	System.p("FIXME: a.size="+size);
	return null;
}

void bool_createPHI(Array stmtList, PHINode phi, Value vfirst, Value vlast, int i) {
	if (i < stmtList.getSize()) {
		LLVMBasicBlock bb = stmtList.get(i);
		int j = i + 1;
		if (j == stmtList.getSize()) {
			phi.addIncoming(vfirst, bb);
		} else {
			phi.addIncoming(vlast, bb);
		}
		bool_createPHI(stmtList, phi, vfirst, vlast, i+1);
	}
}

void or_asm_helper(Expr expr, Array[LLVMBasicBlock] stmtList, LLVMBasicBlock merge, int i, int size, int espidx) {
	IRBuilder builder = System.getBuilder();
	if (i < size) {
		LLVMBasicBlock next = LLVMBasicBlock.create(System.getFunction(), "bbOrExprCond");
		builder.setInsertPoint(stmtList.get(i-1));
		stmtList.set(i, next);
		Expr e = at(expr,i);
		Value cond = expr_asm(espidx+1, at(expr,i), espidx+1);
		builder.createCondBr(cond, next, merge);
		and_asm_helper(expr, stmtList, merge, i+1, size, espidx);
	} else {
		LLVMBasicBlock cond = stmtList.get(i-1);
		builder.setInsertPoint(cond);
		builder.createBr(merge);
	}
}

Value or_asm(int a, Expr expr, int espidx) {
	Array cons = expr.getCons();
	int size = cons.getSize();
	IRBuilder builder = System.getBuilder();
	LLVMBasicBlock bbMerge = LLVMBasicBlock.create(System.getFunction(), "bbOrExprMerge");
	LLVMBasicBlock bbCond = LLVMBasicBlock.create(System.getFunction(), "bbOrExprCond");
	Array[LLVMBasicBlock] stmtList = new LLVMBasicBlock[size];
	builder.createBr(bbCond);
	builder.setInsertPoint(bbMerge);
	stmtList.set(0, bbCond);
	or_asm_helper(expr, stmtList, bbMerge, 1, size, espidx);
	builder.setInsertPoint(bbMerge);
	PHINode phi = builder.createPHI(ctype2Type(/*TY_BOOL*/8), 0);
	bool_createPHI(stmtList, phi, llvm_getBool(true), llvm_getBool(false), 0);
	return phi;
}

void and_asm_helper(Expr expr, Array[LLVMBasicBlock] stmtList, LLVMBasicBlock merge, int i, int size, int espidx) {
	IRBuilder builder = System.getBuilder();
	if (i < size) {
		LLVMBasicBlock next = LLVMBasicBlock.create(System.getFunction(), "bbAndExprCond");
		builder.setInsertPoint(stmtList.get(i-1));
		stmtList.set(i, next);
		Expr e = at(expr,i);
		Value cond = expr_asm(espidx+1, at(expr,i), espidx+1);
		builder.createCondBr(cond, next, merge);
		and_asm_helper(expr, stmtList, merge, i+1, size, espidx);
	} else {
		LLVMBasicBlock cond = stmtList.get(i-1);
		builder.setInsertPoint(cond);
		builder.createBr(merge);
	}
}

Value and_asm(int a, Expr expr, int espidx) {
	Array cons = expr.getCons();
	int size = cons.getSize();
	IRBuilder builder = System.getBuilder();
	LLVMBasicBlock bbMerge = LLVMBasicBlock.create(System.getFunction(), "bbAndExprMerge");
	LLVMBasicBlock bbCond = LLVMBasicBlock.create(System.getFunction(), "bbAndExprCond");
	Array[LLVMBasicBlock] stmtList = new LLVMBasicBlock[size];
	builder.createBr(bbCond);
	builder.setInsertPoint(bbMerge);
	stmtList.set(0, bbCond);
	and_asm_helper(expr, stmtList, bbMerge, 1, size, espidx);
	builder.setInsertPoint(bbMerge);
	PHINode phi = builder.createPHI(ctype2Type(/*TY_BOOL*/8), 0);
	bool_createPHI(stmtList, phi, llvm_getBool(true), llvm_getBool(false), 0);
	return phi;
}

Value letexpr_asm(int a, Expr expr, int espidx) {
	Expr exprL = at(expr,1);
	Expr exprR = at(expr,2);
	if(exprL.getBuild() == TEXPR_LOCAL) {
		Value v = expr_asm(exprL.getIndex(), exprR, espidx);
		if(a != espidx) {
			return nmov_asm(a, exprL.getType(), espidx);
		}
		return v;
	}
	else{
		assert(exprL.getBuild() == TEXPR_FIELD);
		Value v = expr_asm(espidx, exprR, espidx);
		System.p("TODO");
		//abort();
		//kshort_t index = (kshort_t)exprL.index;
		//kshort_t xindex = (kshort_t)(exprL.index >> (sizeof(kshort_t)*8));
		if(ty_isUnbox(exprR.getType())) {
			System.p("TODO field access");
			//ASM(XNMOV, oc(index), xindex, nc(espidx));
			return null;
		}
		else {
			System.p("TODO field access");
			//ASM(XNMOV, oc(index), xindex, oc(espidx));
			return null;
		}
		if(a != espidx) {
			return nmov_asm(a, exprL.getType(), espidx);
		}
	}
	assert(false);// "non-reachable"
	return null;
}

void errstmt_asm(Stmt stmt, int espidx) {
	Object o = stmt.getObjectNULL(0);
	String msg = System.convertToString(o.getAddr());
	System.p("TODO");
	assert(false);
	//DBG_ASSERT(IS_String(msg));
	//ASM(ERROR, sfp(espidx), msg);
}

Boolean isExpr(Expr expr) {
	int x   = expr.getAddr();
	int ct  = System.getPointer(x, 8, 64);
	int cid = System.getPointer(ct, 88, 16);
	return cid == getCid("Expr", -1);
}
Expr Stmt.getExpr(int id, Expr def) {
	Expr expr = getObjectNULL(id);
	if (expr.isNull()) {
		return def;
	}
	return expr;
}

void exprItemstmt_asm(Stmt stmt, int espidx) {
	Expr expr = stmt.getObjectNULL(KW_ExprPattern);
	if(isExpr(expr)) {
		expr_asm(espidx, expr, espidx);
	}
}

void stmtListtmt_asm(Stmt stmt, int espidx) {
	Block bk = stmt.getBlock("$Block", null);
	block_asm(bk);
}

void ifstmt_asm(Stmt stmt, int espidx) {
	IRBuilder builder = System.getBuilder();
	LLVMBasicBlock bbThen = LLVMBasicBlock.create(System.getFunction(), "bbThen");
	LLVMBasicBlock bbElse = LLVMBasicBlock.create(System.getFunction(), "bbElse");
	LLVMBasicBlock bbMerge = LLVMBasicBlock.create(System.getFunction(), "bbMerge");

	/* cond codegen */
	Value cond = expr_asm(espidx, stmt.getExpr(1, null), espidx);
	builder.createCondBr(cond, bbThen, bbElse);

	/* bbThen codegen */
	builder.setInsertPoint(bbThen);
	block_asm(stmt.getBlock("$Block", null));
	//bbThen = builder.getInsertBlock();
	if (builder.getInsertBlock().getTerminator().isNull()) {
		builder.createBr(bbMerge);
	}

	/* bbElse codegen */
	builder.setInsertPoint(bbElse);
	block_asm(stmt.getBlock("else", null));
	//bbElse = builder.getInsertBlock();
	if (builder.getInsertBlock().getTerminator().isNull()) {
		builder.createBr(bbMerge);
	}

	/* TODO PHI Node */

	/* bbMerge codegen */
	builder.setInsertPoint(bbMerge);
}

void returnstmt_asm(Stmt stmt, int espidx) {
	Expr expr = stmt.getObjectNULL(KW_ExprPattern);
	IRBuilder builder = System.getBuilder();
	if(isExpr(expr) && expr.getType() != getCid("void", -1)) {
		Value retval = expr_asm(/*K_RTNIDX*/-4, expr, espidx);
		Function f = System.getFunction();
		Type retTy = f.getReturnType();
		int cid1 = type2cid(retTy);
		int cid2 = type2cid(retval.getType());
		if (isBoxTy(type2cid(retTy)) && isUnBoxTy(type2cid(retval.getType()))) {
			Value boxed = boxing(retval);
			builder.createRet(boxed);
		} else {
			builder.createRet(retval);
		}
	} else if (expr.getType() == getCid("void", -1)) {
		builder.createRetVoid();
	}
}

void loopstmt_asm(Stmt stmt, int espidx) {
	System.p("TODO loop stmt");
}

void undefinedstmt_asm(Stmt stmt, int espidx) {
	System.p("undefined asm syntax");
}

int block_asm2(Block bk, int i, int espidx) {
	Array blockS = bk.getBlocks();
	if (i < blockS.getSize()) {
		Stmt stmt = blockS.get(i);
		System.setUline(stmt.getUline());
		if(stmt.hasSyntax()) {
			if (stmt.getBuild() == TSTMT_ERR) {
				errstmt_asm(stmt, espidx);
				return 0;
			} else if (stmt.getBuild() == TSTMT_EXPR) {
				exprItemstmt_asm(stmt, espidx);
			} else if (stmt.getBuild() == TSTMT_BLOCK) {
				stmtListtmt_asm(stmt, espidx);
			} else if (stmt.getBuild() == TSTMT_RETURN) {
				returnstmt_asm(stmt, espidx);
				return 0;
			} else if (stmt.getBuild() == TSTMT_IF) {
				ifstmt_asm(stmt, espidx);
			} else if (stmt.getBuild() == TSTMT_LOOP) {
				loopstmt_asm(stmt, espidx);
			} else {
				undefinedstmt_asm(stmt, espidx);
			}
		}
		block_asm2(bk, i+1, espidx);
	}
	return 0;
}
void block_asm(Block bk) {
	int espidx = bk.getEspIndex();
	block_asm2(bk, 0, espidx);
}

Type createStructType(Module m, String name, Array fieldsTy, boolean needPtr) {
	//TODO FIXME StructType should be requested
	Type structTy = StructType.create(fieldsTy, name, false);
	if (needPtr) {
		return structTy;
	}
	//TODO FIXME PointerType should be requested
	Type ptrTy = PointerType.get(structTy);
	return ptrTy;
}

FunctionType createFunctionType(Type retTy, Array argsTy) {
	FunctionType fnTy = FunctionType.get(retTy, argsTy, false);
	return fnTy;
}

void construct_args(Array argsTy, Param pa, int n, int psize) {
	if (n < psize) {
		argsTy.set(n+2/*ctx+sfp*/, cid2Type(pa.getType(n)));
		construct_args(argsTy, pa, n+1, psize);
	}
}

Value loadsfp(IRBuilder builder, Value vsfp, int reqtype, int sfpidx) {
	int idx = tri(ty_isUnbox(reqtype), 1, 0);
	Value v = builder.createConstGEP2_32(vsfp, sfpidx, idx);
	if (reqtype != getCid("Boolean", -1) && !(reqtype == getCid("Float", -1))) {
		return v;
	}
	return builder.createBitCast(v, PointerType.get(cid2Type(reqtype)));
}

void push_back_params(Array params, IRBuilder builder, Value vsfp, Param pa, int i, int psize) {
	if (i < psize) {
		Value v = loadsfp(builder, vsfp, pa.getType(i), i+1);
		params.set(i+2, builder.createLoad(v, false));
		push_back_params(params, builder, vsfp, pa, i+1, psize);
	}
}

Function build_wrapper_func(Method mtd, Module m, Function f) {
	Param pa = mtd.getParam();
	int retTy = mtd.getReturnType();
	String name = method2string(mtd) + "_wrapper";
	FunctionType fnTy = System.getTyFunction();
	Function wrap = m.getOrInsertFunction(name, fnTy);
	LLVMBasicBlock bb = LLVMBasicBlock.create(wrap, "EntryBlock");
	IRBuilder builder = IRBuilder.new(bb);

	Array args = wrap.getArguments();
	Value argkctx = args.get(0);
	Value arg_sfp = args.get(1);
	Value arg_rix = args.get(2);

	argkctx.setName("ctx");
	arg_sfp.setName("sfp");
	arg_rix.setName("rix");

	builder.setInsertPoint(bb);
	/* load konoha args from sfp and build call params*/
	Array[Value] params = new Value[(2+mtd.getParamSize())];
	params.set(0, argkctx);
	params.set(1, arg_sfp);
	push_back_params(params, builder, arg_sfp, pa, 0, mtd.getParamSize());

	/* call function and  set return values */
	Value callinst = builder.createCall(f, params);
	if (retTy != getCid("void", -1)) {
		Value v = loadsfp(builder, arg_sfp, retTy, /*K_RTNIDX*/-4);
		builder.createStore(callinst, v, false/*isVolatile*/);
	}
	builder.createRetVoid();
	return wrap;
}

void function_set_params(Module m, Array args, Param pa, int i, int n) {
	if (i < n) {
		int reg = i+1;
		int idx = tri(ty_isUnbox(pa.getType(i)), nc(reg), oc(reg));
		Value v = args.get(i);
		vstack_setValue(idx, v);
		function_set_params(m, args, pa, i+1, n);
	}
}

void push_back_fparams(Array params, Array args, int i, int n) {
	if (i < n) {
		Value v = args.get(i+2);
		params.set(i, v);
		push_back_fparams(params, args, i+1, n);
	}
}

Function create_function(Module m, Method mtd) {
	int psize = mtd.getParamSize();
	int rtype = mtd.getReturnType();
	int argc  = 2 + psize;
	Array[Type] argsTy = new Type[(argc)];
	argsTy.set(0, ctype2Type(/*TY_CONTEXT*/16));
	argsTy.set(1, ctype2Type(/*TY_STACK*/17));
	construct_args(argsTy, mtd.getParam(), 0, psize);
	FunctionType fnTy = createFunctionType(cid2Type(rtype), argsTy);
	String fname = method2string(mtd);
	Function f = Function.create(fname, fnTy, m, ExternalLinkage);

	Array args = f.getArguments();
	Value vctx = args.get(0);
	Value vsfp = args.get(1);

	vctx.setName("lctx");
	vsfp.setName("lsfp");

	System.setCTX(vctx);
	System.setSFP(vsfp);

	Array[Value] a = new Value[psize];
	push_back_fparams(a, args, 0, psize);
	function_set_params(m, a, mtd.getParam(), 0, psize);
	return f;
}

Type getLongTy() {
	return Type.getInt64Ty();
	//if (POINTER_SIZE == 8) {
	//	return Type.getInt64Ty();
	//}
	//return Type.getInt32Ty();
}
Type getShortTy() {
	return Type.getInt32Ty();
	//if (POINTER_SIZE == 8) {
	//	return Type.getInt64Ty();
	//}
	//return Type.getInt32Ty();
}

void jit_init() {
	Module m = Module.new("test");
	ExecutionEngine ee = m.createExecutionEngine(2);

	Array[Type] a0 = new Type[0];
	Type kklib = StructType.create(a0, "kklib", false);
	Type kclass = StructType.create(a0, "kclass", false);
	Type KUtilsGrowingArray = StructType.create(a0, "KUtilsGrowingArray", false);
	Type PlatformApi = StructType.create(a0, "PlatformApi", false);
	Type KonohaRuntime = StructType.create(a0, "KonohaRuntime", false);
	Type klocal_t = StructType.create(a0, "klocal_t", false);
	Type kmemshare_t = StructType.create(a0, "kmemshare_t", false);
	Type kmemlocal_t = StructType.create(a0, "kmemlocal_t", false);
	Type KonohaModule = StructType.create(a0, "KonohaModule", false);
	Type KonohaModuleContext = StructType.create(a0, "KonohaModuleContext", false);
	Type KonohaStackRuntimeVar = StructType.create(a0, "KonohaStackRuntimeVar", false);
	Type klogger_t = StructType.create(a0, "klogger_t", false);
	Type VirtualMachineInstruction = StructType.create(a0, "VirtualMachineInstruction", false);
	/*------- KonohaObjectHeader ------*/
	Array[Type] KonohaObjectHeaderTy = new Type[4];
	KonohaObjectHeaderTy.set(0, getLongTy() /* magicflag */ );
	KonohaObjectHeaderTy.set(1, PointerType.get(kclass) /* ct */ );
	KonohaObjectHeaderTy.set(2, getLongTy() /* refc */ /*union*/);
	KonohaObjectHeaderTy.set(3, PointerType.get(KUtilsGrowingArray) /* kvproto */ );
	Type KonohaObjectHeader = StructType.create(KonohaObjectHeaderTy, "KonohaObjectHeader", false)
	System.setTyHObject(KonohaObjectHeader);

	/*------- _kObject ------*/
	Array[Type] kObjectTy = new Type[2];
	kObjectTy.set(0, KonohaObjectHeader /* h */ );
	kObjectTy.set(1, PointerType.get(getLongTy()) /* fields */ /*union*/);
	Type kObject = StructType.create(kObjectTy, "kObject", false)
	System.setTyObject(PointerType.get(kObject));

	/*------- KonohaStack ------*/
	Array[Type] KonohaStackTy = new Type[2];
	KonohaStackTy.set(0, PointerType.get(kObject) /* o */ /*union*/);
	KonohaStackTy.set(1, getLongTy() /* unboxValue */ /*union*/);
	Type KonohaStack = StructType.create(KonohaStackTy, "KonohaStack", false)
	System.setTyStack(PointerType.get(KonohaStack));

	/*------- KonohaContextVar ------*/
	Array[Type] KonohaContextVarTy = new Type[12];
	KonohaContextVarTy.set(0, Type.getInt32Ty() /* safepoint */ );
	KonohaContextVarTy.set(1, PointerType.get(KonohaStack) /* esp */ );
	KonohaContextVarTy.set(2, PointerType.get(kklib) /* klib */ );
	KonohaContextVarTy.set(3, PointerType.get(PlatformApi) /* plat */ );
	KonohaContextVarTy.set(4, PointerType.get(kmemshare_t) /* memshare */ );
	KonohaContextVarTy.set(5, PointerType.get(kmemlocal_t) /* memlocal */ );
	KonohaContextVarTy.set(6, PointerType.get(KonohaRuntime) /* share */ );
	KonohaContextVarTy.set(7, PointerType.get(klocal_t) /* local */ );
	KonohaContextVarTy.set(8, PointerType.get(KonohaStackRuntimeVar) /* stack */ );
	KonohaContextVarTy.set(9, PointerType.get(klogger_t) /* logger */ );
	KonohaContextVarTy.set(10, PointerType.get(PointerType.get(KonohaModule)) /* modshare */ );
	KonohaContextVarTy.set(11, PointerType.get(PointerType.get(KonohaModuleContext)) /* modlocal */ );
	Type KonohaContextVar = StructType.create(KonohaContextVarTy, "KonohaContextVar", false)

	/*------- _kArray ------*/
	Array[Type] kArrayTy = new Type[4];
	kArrayTy.set(0, KonohaObjectHeader /* h */ );
	kArrayTy.set(1, getLongTy() /* bytesize */ );
	kArrayTy.set(2, PointerType.get(getLongTy()) /* unboxValue */ /*union*/);
	kArrayTy.set(3, getLongTy() /* bytemax */ );
	Type kArray = StructType.create(kArrayTy, "kArray", false)


	Array[Type] argsTy = new Type[2];
	argsTy.set(0, PointerType.get(KonohaContextVar));
	argsTy.set(1, PointerType.get(KonohaStack));
	FunctionType funcTy = FunctionType.get(Type.getVoidTy(), argsTy, false);

	/*------- _kMethod ------*/
	Array[Type] kMethodTy = new Type[13];
	kMethodTy.set(0, KonohaObjectHeader /* h */ );
	kMethodTy.set(1, PointerType.get(funcTy) /* invokeMethodFunc */ /*union*/);
	kMethodTy.set(2, PointerType.get(VirtualMachineInstruction) /* pc_start */ /*union*/);
	kMethodTy.set(3, getLongTy() /* flag */ );
	kMethodTy.set(4, Type.getInt32Ty() /* cid */ );
	kMethodTy.set(5, Type.getInt32Ty() /* mn */ );
	kMethodTy.set(6, Type.getInt32Ty() /* paramid */ );
	kMethodTy.set(7, Type.getInt32Ty() /* paramdom */ );
	kMethodTy.set(8, Type.getInt32Ty() /* delta */ );
	kMethodTy.set(9, Type.getInt32Ty() /* packageId */ );
	kMethodTy.set(10, PointerType.get(kObject) /* tcode */ );
	kMethodTy.set(11, PointerType.get(kObject) /* objdata */ /*union*/);
	kMethodTy.set(12, PointerType.get(kObject) /* proceedNUL */ );
	Type kMethod = StructType.create(kMethodTy, "kMethod", false)

	System.setTyContext(PointerType.get(KonohaContextVar));
	System.setTyFunction(funcTy);
	System.setTyFunctionP(PointerType.get(funcTy));
	System.setTyMethod(PointerType.get(kMethod));

	System.setModule(m);
	System.setExecutionEngine(ee);
	//base.jitcache = KLIB Kmap_init(kctx, 0);
}

void System.genCode(Method mtd, Block bk) {
	//System.p("START CODE GENERATION..");
	if (System.getModule().isNull()) {
		jit_init();
	}
	Module m = System.getModule();
	Function f = create_function(m, mtd);
	System.setJITCache(mtd, f);
	System.setFunction(f);
	//assert(System.getBuilder().isNull());
	LLVMBasicBlock bb = LLVMBasicBlock.create(f, "BB");
	IRBuilder builder = IRBuilder.new(bb);
	System.setBuilder(builder);
	block_asm(bk);
	LLVMBasicBlock iblock = builder.getInsertBlock();
	if (iblock.getTerminator().isNull()) {
		int rtype = mtd.getReturnType();
		if (rtype == getCid("void", -1)) {
			builder.createRetVoid();
		} else {
			Value v = createConstValue(cid2ctype(rtype), System.KLIB Knull(kctx, rtype), 0, 0.0);
			builder.createRet(v);
		}
	}
	Function wrap = build_wrapper_func(mtd, m, f);
	//f.dump();
	//m.dump();

	FunctionPassManager pm = FunctionPassManager.new(m);
	pm.add(LLVM.createVerifierPass());
	pm.doInitialization();
	pm.run(f);
	pm.run(wrap);

	ExecutionEngine ee = System.getExecutionEngine();
	int fptr = ee.getPointerToFunction(wrap);
	//System.p(fptr);
	System.assert(fptr != 0);
	System.clearValue();
	System.setBuilder(null);
	mtd.setFunc(fptr);
}

