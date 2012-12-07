#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Type.h>
#include <llvm/DerivedTypes.h>

#ifndef LLVMTYPE_H
#define LLVMTYPE_H

enum STRUCT_TYPE_ID {
	ID_void,
	ID_Ptrvoid,
	ID_long,
	ID_short,
	ID_int,
	ID_PtrKonohaContextVar,
	ID_KonohaObjectHeader,
	ID_PtrkObjectVar,
	ID_PtrPtrkObjectVar,
	ID_PtrKonohaValueVar,
	ID_PtrMethodFunc,
	ID_PtrkMethodVar,
	ID_MAX,
	ID_uint = ID_int,
	ID_ulong = ID_long,
	ID_PtrKonohaClassVar = ID_Ptrvoid,
	ID_PtrKProtoMap = ID_Ptrvoid,
	ID_PtrKonohaFactory = ID_Ptrvoid,
	ID_PtrKonohaLibVar = ID_Ptrvoid,
	ID_PtrKonohaRuntimeVar = ID_Ptrvoid,
	ID_PtrKonohaStackRuntimeVar = ID_Ptrvoid,
	ID_PtrPtrKonohaModule = ID_Ptrvoid,
	ID_PtrPtrKonohaModuleContext = ID_Ptrvoid,
	ID_PtrVirtualCode = ID_Ptrvoid,
	ID_PtrkTokenVar = ID_Ptrvoid,
	ID_PtrkNameSpaceVar = ID_Ptrvoid,
	ID_ERROR = -1
};

struct TypeInfo {
	const char *Name;
	bool     IsFunction;
	enum STRUCT_TYPE_ID ReturnTypeId;
	unsigned ParamSize;
	struct Param {
		enum STRUCT_TYPE_ID TypeId;
#ifdef PARAM_DEBUG
		const char *name;
#endif
	} Params[13];
};

#define TYPE(NAME, PARAMS)\
	static const struct TypeInfo NAME##Type = { #NAME, 0, ID_void, PARAMS };
#define FUNCTION(NAME, RTYPE, PARAMS)\
	static const struct TypeInfo NAME##Type = { #NAME, 1, ID_##RTYPE, PARAMS };
#ifdef PARAM_DEBUG
#define OP(TYPE, NAME) ID_##TYPE, #NAME
#else
#define OP(TYPE, NAME) ID_##TYPE
#endif
#define PARAM2(NAME, P0, P1) 2, { {P0}, {P1} }
#define PARAM3(NAME, P0, P1, P2) 3, { {P0}, {P1}, {P2} }
#define PARAM9(NAME, P0, P1, P2, P3, P4, P5, P6, P7, P8)\
	9, { {P0}, {P1}, {P2}, {P3}, {P4}, {P5}, {P6}, {P7}, {P8} }
#define PARAM13(NAME, P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12) \
	12, { {P0}, {P1}, {P2}, {P3}, {P4}, {P5}, {P6}, {P7}, {P8}, {P9}, {P10}, {P11}, {P12} }
#include "LLVMType.def"
#undef TYPE
#undef FUNCTION
#undef OP
#undef PARAM2
#undef PARAM3
#undef PARAM9
#undef PARAM13

#define CONCAT(x, y) CONCAT2(x, y)
#define CONCAT2(x, y) x##y

#define TYPE(NAME, PARAMS) PARAMS
#define FUNCTION(NAME, RTYPE, PARAMS)
#define OP(TYPE, NAME) NAME
#define DEF(NAME, FIELD, VAL) CONCAT(NAME##_, FIELD)
#define PARAM2(NAME, P0, P1) enum NAME##_FIELD_ID { DEF(NAME,P0, 0), DEF(NAME,P1, 1) };
#define PARAM3(NAME, P0, P1, P2) enum NAME##_FIELD_ID {DEF(NAME,P0, 0), DEF(NAME,P1, 1), DEF(NAME,P2, 2)};
#define PARAM9(NAME, P0, P1, P2, P3, P4, P5, P6, P7, P8)\
	enum NAME##_FIELD_ID {\
	DEF(NAME,P0, 0), DEF(NAME,P1, 1), DEF(NAME,P2, 2), DEF(NAME,P3, 3),\
	DEF(NAME,P4, 4), DEF(NAME,P5, 5), DEF(NAME,P6, 6), DEF(NAME,P7, 7),\
	DEF(NAME,P8, 8)};
#define PARAM13(NAME, P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12) \
	enum NAME##_FIELD_ID {\
	DEF(NAME,P0, 0), DEF(NAME,P1, 1), DEF(NAME,P2, 2), DEF(NAME,P3, 3),\
	DEF(NAME,P4, 4), DEF(NAME,P5, 5), DEF(NAME,P6, 6), DEF(NAME,P7, 7),\
	DEF(NAME,P8, 4), DEF(NAME,P9, 9), DEF(NAME,P10, 10), DEF(NAME,P11, 11),\
	DEF(NAME,P12, 12)};

#include "LLVMType.def"
#undef TYPE
#undef FUNCTION
#undef OP
#undef PARAM2
#undef PARAM3
#undef PARAM9
#undef PARAM13

#define LLVM_CONTEXT() (llvm::getGlobalContext())

static llvm::Type *getLongTy()
{
	if(sizeof(void*) == 4)
		return llvm::Type::getInt32Ty(LLVM_CONTEXT());
	else
		return llvm::Type::getInt64Ty(LLVM_CONTEXT());
}

static llvm::Type *getShortTy()
{
	if(sizeof(void*) == 4)
		return llvm::Type::getInt16Ty(LLVM_CONTEXT());
	else
		return llvm::Type::getInt32Ty(LLVM_CONTEXT());
}

static llvm::Type *LLVMTYPE_ContextPtr = NULL;
static llvm::Type *LLVMTYPE_ObjectHeader = NULL;
static llvm::Type *LLVMTYPE_ObjectPtr = NULL;
static llvm::Type *LLVMTYPE_KonohaValuePtr = NULL;
static llvm::Type *LLVMTYPE_MethodPtr = NULL;
static llvm::Type *LLVMTYPE_MethodFunc = NULL;

static llvm::Type *ToType(enum STRUCT_TYPE_ID ID)
{
	switch(ID) {
	case ID_void:    return llvm::Type::getVoidTy(LLVM_CONTEXT());
	case ID_Ptrvoid: return llvm::PointerType::get(getLongTy(), 0);
	case ID_long:    return getLongTy();
	case ID_short:   return getShortTy();
	case ID_int:     return llvm::Type::getInt32Ty(LLVM_CONTEXT());
	case ID_PtrKonohaContextVar: return LLVMTYPE_ContextPtr;
	case ID_KonohaObjectHeader:  return LLVMTYPE_ObjectHeader;
	case ID_PtrkObjectVar:       return LLVMTYPE_ObjectPtr;
	case ID_PtrPtrkObjectVar:
		if(LLVMTYPE_ObjectPtr)
			return llvm::PointerType::get(LLVMTYPE_ObjectPtr, 0);
		else
			return llvm::PointerType::get(getLongTy(), 0);
	case ID_PtrKonohaValueVar: return LLVMTYPE_KonohaValuePtr;
	case ID_PtrMethodFunc:     return LLVMTYPE_MethodFunc;
	case ID_PtrkMethodVar:     return LLVMTYPE_MethodPtr;
	default:
		return getLongTy();
	}
	assert(0 && "unreachable");
	return 0;
}

static llvm::Type *CreateType(const struct TypeInfo &Info)
{
	std::vector<llvm::Type *> Fields;
	for (unsigned i = 0; i < Info.ParamSize; ++i) {
		llvm::Type *Ty = ToType(Info.Params[i].TypeId);
		Fields.push_back(Ty);
	}
	if (Info.IsFunction) {
		llvm::Type *RetTy = ToType(Info.ReturnTypeId);
		llvm::FunctionType *FnTy = llvm::FunctionType::get(RetTy, Fields, false);
		return llvm::PointerType::get(FnTy, 0);
	}
	else {
		llvm::StructType *structTy = llvm::StructType::create(llvm::getGlobalContext(), Info.Name);
		structTy->setBody(Fields, false);
		return structTy;
	}
}

static void LLVMType_Init()
{
	LLVMTYPE_ObjectHeader = CreateType(KonohaObjectHeaderType);

	llvm::Type *ObjectTy = CreateType(kObjectVarType);
	LLVMTYPE_ObjectPtr = llvm::PointerType::get(ObjectTy, 0);

	llvm::Type *StackTy = CreateType(KonohaValueVarType);
	LLVMTYPE_KonohaValuePtr = llvm::PointerType::get(StackTy, 0);

	llvm::Type *ContextTy = CreateType(KonohaContextVarType);
	LLVMTYPE_ContextPtr = llvm::PointerType::get(ContextTy, 0);

	LLVMTYPE_MethodFunc = CreateType(MethodFuncType);

	llvm::Type *MethodTy = CreateType(kMethodVarType);
	LLVMTYPE_MethodPtr = llvm::PointerType::get(MethodTy, 0);
}

static llvm::Type *convert_type(KonohaContext *kctx, kattrtype_t type)
{
	switch (type) {
		case TY_void:    return llvm::Type::getVoidTy(LLVM_CONTEXT());
		case TY_boolean: return llvm::Type::getInt64Ty(LLVM_CONTEXT());
		case TY_int:     return llvm::Type::getInt64Ty(LLVM_CONTEXT());
	}
	if(KDefinedKonohaCommonModule() && type == TY_float)
		return llvm::Type::getDoubleTy(LLVM_CONTEXT());
	return LLVMTYPE_ObjectPtr;
}

static llvm::Type *convert_type(KonohaContext *kctx, KonohaClass *kclass)
{
	return convert_type(kctx, kclass->typeId);
}

static inline llvm::Type *GetLLVMType(enum STRUCT_TYPE_ID Id)
{
	return ToType(Id);
}

#endif /* end of include guard */
