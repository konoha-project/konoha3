using konoha.llvm.*;
using konoha.lang.*;

Map<String, Type> ktype = {};

int f(int x, int y);
void constructKonohaTypes(Module m);

@Public Value IRBuilder.getValueFromSfp(Value sfp, Type rTy, int sfpidx) {
	int idx;
	if(rTy == ktype["objectPtrTy"])
		idx = 0;
	else
		idx = 1;
	v = this.createConstGEP2_32(sfp, sfpidx, idx);
	return this.createBitCast(v, PointerType.get(rTy));
}

@Public Value IRBuilder.createLoadValueFromSfp(Value sfp, Type rTy, int sfpidx) {
	Value v = this.getValueFromSfp(sfp, rTy, sfpidx);
	return this.createLoad(v);
}
@Public void IRBuilder.createStoreValueToSfp(Value sfp, Type rTy, int sfpidx, Value v) {
	r  = this.getValueFromSfp(sfp, Type.getInt64Ty(), sfpidx);
	this.createStore(v, r);
}

void main (String[] args) {
	Module m = new Module("hello");
	constructKonohaTypes(m);
	Function func = m.getOrInsertFunction("test", (FunctionType)ktype["fmethodTy"]);
	BasicBlock bb = BasicBlock.create(func);
	IRBuilder builder = new IRBuilder(bb);
	Array<Value> vargs = func.getArguments();
	sfp = vargs[1];
	a1 = builder.createLoadValueFromSfp(sfp, Type.getInt64Ty(), 1);
	a2 = builder.createLoadValueFromSfp(sfp, Type.getInt64Ty(), 2);
	v  = builder.createAdd(a1, a2);
	builder.createStoreValueToSfp(sfp, Type.getInt64Ty(), -4, v);
	builder.CreateRetVoid();
	m.dump();
	ee = m.createExecutionEngine();
	Method mtd = method:Script.f;
	mtd.setFunction(ee.getPointerToFunction(m, func));
	print f(10, 20);
}


void constructKonohaTypes(Module m) {
	Type[] fields = [];
	fields.add(Type.getInt32Ty());
	fields.add(Type.getInt32Ty());
	hcacheTy = StructType.get(fields, false);
	m.addTypeName("kcachedata_t", hcacheTy);

	fields = [];
	fields.add(Type.getInt64Ty());
	fields.add(Type.getInt64Ty());
	sfxTy = StructType.get(fields, false);
	m.addTypeName("ksfx_t", sfxTy);

	fields = [];
	fields.add(Type.getInt64Ty());
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt64Ty());
	fields.add(Type.getInt8PtrTy());
	hObjectTy = StructType.get(fields, false);
	m.addTypeName("hObject", hObjectTy);

	fields = [];
	fields.add(hObjectTy);
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	objectTy = StructType.get(fields, false);
	objectPtrTy = PointerType.get(objectTy);
	objectPtrPtrTy = PointerType.get(objectPtrTy)
	m.addTypeName("Object", objectTy);
	m.addTypeName("ObjectPtr", objectPtrTy);
	ktype["objectPtrTy"] = objectPtrTy;
	ktype["objectPtrPtrTy"] = objectPtrPtrTy;

	fields = [];
	fields.add(hObjectTy);
	fields.add(Type.getInt64PtrTy());
	fields.add(Type.getInt64Ty());
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	arrayTy = StructType.get(fields, false);
	arrayPtrTy = PointerType.get(arrayTy);
	m.addTypeName("Array", arrayTy);

	fields = [];
	fields.add(hObjectTy);
	fields.add(Type.getInt1PtrTy()); /* text */
	fields.add(Type.getInt64Ty());   /* len */
	stringTy = StructType.get(fields, false);
	stringPtrTy = PointerType.get(stringTy);
	m.addTypeName("String", stringTy);
	ktype["stringPtrTy"] = stringPtrTy;


	fields = [];
	fields.add(hObjectTy);
	fields.add(objectPtrPtrTy);
	fields.add(objectPtrTy);
	objectFieldTy    = StructType.get(fields, false);
	objectFieldPtrTy = PointerType.get(objectFieldTy)
	m.addTypeName("ObjectField", objectFieldTy);

	fields = [];
	fields.add(hObjectTy);
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt64Ty());
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	outputStreamTy = StructType.get(fields, false);
	outputStreamPtrTy = PointerType.get(outputStreamTy);
	m.addTypeName("OutputStream", outputStreamTy);
	ktype["outputStreamPtrTy"] = outputStreamPtrTy;

	fields = [];
	fields.add(hObjectTy);
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt64Ty());
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	inputStreamTy = StructType.get(fields, false);
	inputStreamPtrTy = PointerType.get(inputStreamTy);
	m.addTypeName("InputStream", inputStreamTy);
	ktype["inputStreamPtrTy"] = inputStreamPtrTy;

	fields = [];
	fields.add(objectPtrTy);
	fields.add(Type.getFloatTy());
	sfpTy = StructType.get(fields, false);
	sfpPtrTy = PointerType.get(sfpTy);
	m.addTypeName("sfp", sfpTy);
	ktype["sfpPtrTy"] = sfpPtrTy;

	fields = [];
	fields.add(Type.getInt64Ty());
	fields.add(Type.getInt64Ty());
	fields.add(Type.getInt8PtrTy());
	fields.add(inputStreamPtrTy);
	fields.add(outputStreamPtrTy);
	fields.add(outputStreamPtrTy);
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt64Ty());
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	fields.add(arrayPtrTy);
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	sysExTy = StructType.get(fields, false);

	fields = [];
	fields.add(hObjectTy);
	fields.add(sysExTy);
	sysTy = StructType.get(fields, false);
	sysPtrTy = PointerType.get(sysTy);
	m.addTypeName("System", sysTy);
	ktype["sysPtrTy"] = sysPtrTy;

	fields = [];
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	fields.add(sysPtrTy);
	fields.add(Type.getInt8PtrTy());
	fields.add(sfpPtrTy);
	fields.add(sfpPtrTy);
	fields.add(Type.getInt64Ty());
	fields.add(sfpPtrTy);
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	fields.add(objectPtrTy);
	fields.add(objectPtrTy);
	fields.add(Type.getInt64Ty());
	fields.add(Type.getInt64Ty());
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	fields.add(objectPtrPtrTy);
	fields.add(Type.getInt64Ty());
	fields.add(objectPtrPtrTy);
	fields.add(Type.getInt64Ty());
	fields.add(objectPtrPtrTy);
	fields.add(Type.getInt64Ty());
	fields.add(Type.getInt64Ty());
	fields.add(Type.getInt8PtrTy());
	fields.add(inputStreamPtrTy);
	fields.add(outputStreamPtrTy);
	fields.add(outputStreamPtrTy);
	fields.add(Type.getInt8PtrTy());
	fields.add(outputStreamPtrTy);
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt32Ty);
	fields.add(Type.getInt32Ty);
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	Type[] vec16_fields = [];
	for (i=0; i < 16; i++) {
		vec16_fields.add(Type.getInt1Ty());
	}
	fields.add(StructType.get(vec16_fields, false));
	fields.add(Type.getInt64Ty());
	fields.add(Type.getInt8PtrTy());
	fields.add(objectPtrTy);
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt8PtrTy());

	ctxTy = StructType.get(fields, false);
	ctxPtrTy = PointerType.get(ctxTy);
	m.addTypeName("context", ctxTy);
	m.addTypeName("KonohaContext *kctx", ctxPtrTy);
	ktype["ctxTy"] = ctxPtrTy;

	Type[] argsTy = [];
	argsTy.add(ctxPtrTy);
	argsTy.add(sfpPtrTy);
	argsTy.add(Type.getInt64Ty());
	FunctionType fmethodTy = FunctionType.get(Type.getVoidTy(), argsTy, false);
	fmethodPtrTy = PointerType.get(fmethodTy);
	ktype["fmethodTy"]    = fmethodTy;
	ktype["fmethodPtrTy"] = fmethodTy;

	fields = [];
	fields.add(Type.getInt8PtrTy()); /* codeaddr */
	fields.add(Type.getInt64Ty());   /* count */
	fields.add(Type.getInt32PtrTy());   /* opcode */
	fields.add(Type.getInt32Ty());   /* line */
	fields.add(Type.getInt64Ty());   /* data[0] */
	fields.add(Type.getInt64Ty());   /* data[1] */
	fields.add(Type.getInt64Ty());   /* data[2] */
	fields.add(Type.getInt64Ty());   /* data[3] */
	fields.add(Type.getInt64Ty());   /* data[4] */
	oplineTy = StructType.get(fields, false);
	oplinePtrTy = PointerType.get(oplineTy);
	m.addTypeName("opline_t", oplineTy);

	fields = [];
	fields.add(hObjectTy);
	fields.add(Type.getInt8PtrTy());
	fields.add(Type.getInt32Ty);
	fields.add(Type.getInt32Ty);
	fields.add(fmethodPtrTy);
	fields.add(oplinePtrTy);
	methodTy = StructType.get(fields, false);
	m.addTypeName("Method", methodTy);
	methodPtrTy = PointerType.get(methodTy);
	ktype["MethodPtrTy"] = methodPtrTy;

	fields = [];
	fields.add(hObjectTy);
	fields.add(Type.getInt8PtrTy());
	fields.add(fmethodPtrTy);
	iteratorTy = StructType.get(fields, false);
	m.addTypeName("Iterator", iteratorTy);
	ktype["IteratorPtrTy"] = PointerType.get(iteratorTy);

	/* TODO */
	classTBLPtrTy = Type.getInt8PtrTy();
	typeMapPtrTy  = Type.getInt8PtrTy();

}
