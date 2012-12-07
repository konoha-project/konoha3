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
#define PARAM2(P0, P1) 2, { {P0}, {P1} }
#define PARAM3(P0, P1, P2) 3, { {P0}, {P1}, {P2} }
#define PARAM9(P0, P1, P2, P3, P4, P5, P6, P7, P8)\
	9, { {P0}, {P1}, {P2}, {P3}, {P4}, {P5}, {P6}, {P7}, {P8} }
#define PARAM13(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12) \
	12, { {P0}, {P1}, {P2}, {P3}, {P4}, {P5}, {P6}, {P7}, {P8}, {P9}, {P10}, {P11}, {P12} }
#include "LLVMType.h"
#undef TYPE
#undef FUNCTION
#undef OP
#undef PARAM2
#undef PARAM3
#undef PARAM9
#undef PARAM13

#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Type.h>
#include <llvm/DerivedTypes.h>

using namespace llvm;

#define LLVM_CONTEXT() (getGlobalContext())

static Type *getLongTy()
{
	if(sizeof(void*) == 4)
		return Type::getInt32Ty(LLVM_CONTEXT());
	else
		return Type::getInt64Ty(LLVM_CONTEXT());
}

static Type *getShortTy()
{
	if(sizeof(void*) == 4)
		return Type::getInt16Ty(LLVM_CONTEXT());
	else
		return Type::getInt32Ty(LLVM_CONTEXT());
}

static Type *LLVMTYPE_ContextPtr = NULL;
static Type *LLVMTYPE_ObjectHeader = NULL;
static Type *LLVMTYPE_ObjectPtr = NULL;
static Type *LLVMTYPE_KonohaValue = NULL;
static Type *LLVMTYPE_MethodPtr = NULL;
static Type *LLVMTYPE_MethodFunc = NULL;
#define LLVMTYPE_Void  (Type::getVoidTy(LLVM_CONTEXT()))
#define LLVMTYPE_Int   (Type::getInt64Ty(LLVM_CONTEXT()))
#define LLVMTYPE_Bool  (Type::getInt64Ty(LLVM_CONTEXT()))
#define LLVMTYPE_Float (Type::getDoubleTy(LLVM_CONTEXT()))

static Type *ToType(enum STRUCT_TYPE_ID ID)
{
	switch(ID) {
	case ID_void:    return Type::getVoidTy(LLVM_CONTEXT());
	case ID_Ptrvoid: return PointerType::get(getLongTy(), 0);
	case ID_long:    return getLongTy();
	case ID_short:   return getShortTy();
	case ID_int:     return Type::getInt32Ty(LLVM_CONTEXT());
	case ID_PtrKonohaContextVar: return LLVMTYPE_ContextPtr;
	case ID_KonohaObjectHeader:  return LLVMTYPE_ObjectHeader;
	case ID_PtrkObjectVar:       return LLVMTYPE_ObjectPtr;
	case ID_PtrPtrkObjectVar:
		if(LLVMTYPE_ObjectPtr)
			return PointerType::get(LLVMTYPE_ObjectPtr, 0);
		else
			return PointerType::get(getLongTy(), 0);
	case ID_PtrKonohaValueVar: return LLVMTYPE_KonohaValue;
	case ID_PtrMethodFunc:     return LLVMTYPE_MethodFunc;
	case ID_PtrkMethodVar:     return LLVMTYPE_MethodPtr;
	default:
		return getLongTy();
	}
	assert(0 && "unreachable");
	return 0;
}

static Type *CreateType(Module *m, const struct TypeInfo &Info)
{
	std::vector<Type *> Fields;
	for (unsigned i = 0; i < Info.ParamSize; ++i) {
		Type *Ty = ToType(Info.Params[i].TypeId);
		Fields.push_back(Ty);
	}
	if (Info.IsFunction) {
		Type *RetTy = ToType(Info.ReturnTypeId);
		FunctionType *FnTy = FunctionType::get(RetTy, Fields, false);
		return PointerType::get(FnTy, 0);
	}
	else {
		StructType *structTy = StructType::create(Fields, Info.Name, false);
		return structTy;
	}
}

static void LLVMType_Init(Module *M)
{
	LLVMTYPE_ObjectHeader = CreateType(M, KonohaObjectHeaderType);

	Type *ObjectTy = CreateType(M, kObjectVarType);
	LLVMTYPE_ObjectPtr = PointerType::get(ObjectTy, 0);

	LLVMTYPE_KonohaValue = CreateType(M, KonohaValueVarType);

	Type *ContextTy = CreateType(M, KonohaContextVarType);
	LLVMTYPE_ContextPtr = PointerType::get(ContextTy, 0);

	LLVMTYPE_MethodFunc = CreateType(M, MethodFuncType);

	Type *MethodTy = CreateType(M, kMethodVarType);
	LLVMTYPE_MethodPtr = PointerType::get(MethodTy, 0);
}

static Type *convert_type(KonohaContext *kctx, kattrtype_t Type)
{
	switch (Type) {
		case TY_void:    return LLVMTYPE_Void;
		case TY_boolean: return LLVMTYPE_Bool;
		case TY_int:     return LLVMTYPE_Int;
	}
	if(Type == TY_float)
		return LLVMTYPE_Float;
	return LLVMTYPE_Object;
}

static Type *convert_type(KonohaContext *kctx, KonohaClass *kclass)
{
	return convert_type(kctx, kclass->typeId);
}

int main(int argc, char const* argv[])
{
	LLVMContext &Context = getGlobalContext();
	Module *m = new Module("test", Context);
	LLVMType_Init(m);
	(*m).dump();
	return 0;
}
