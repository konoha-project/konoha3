#include <stdint.h>
#include <stdbool.h>
#include "../../../../include/konoha3/konoha.h"
#include "../../../../include/konoha3/konoha_common.h"
#include "../../../../include/konoha3/sugar.h"

#ifndef VMCOMMON_H
#define VMCOMMON_H

enum TypeId {
	TYPE_void = KType_void,
	TYPE_Object = KType_Object,
	TYPE_boolean = KType_Boolean,
	TYPE_int = KType_Int,
	TYPE_String = KType_String,
	TYPE_Function = KType_Func,
	TYPE_Array = KType_Array,
	TYPE_Method = KType_Method,
	TYPE_NameSpace = KType_NameSpace,
	TYPE_Any = KType_0,
	TYPE_float = -1,
	TYPE_BoolObj = -2,
	TYPE_IntObj = -3,
	TYPE_FloatObj = -4
};

typedef union SValue {
	int64_t     ival;
	double      fval;
	bool        bval;
	void       *ptr;
	const char *str;
	uint64_t    bits;
	kObject    *obj;
} SValue;

static inline bool IsUnBoxedType(enum TypeId Type)
{
	switch(Type) {
		case TYPE_void:
			assert(0 && "FIXME");
		case TYPE_boolean:
		case TYPE_int:
		case TYPE_float:
			return true;
		default:
			return false;
	}
}

#define FloatIsDefined(kctx) (KDefinedKonohaCommonModule() && KClass_Float != NULL)

static inline enum TypeId ConvertToTypeId(KonohaContext *kctx, ktypeattr_t type)
{
	if(FloatIsDefined(kctx) && type == KType_float)
		return TYPE_float;
	if(type == KType_Symbol) {
		return TYPE_int;
	}
	return (enum TypeId) type;
}

static inline ktypeattr_t ToKType(KonohaContext *kctx, enum TypeId Type)
{
	if(FloatIsDefined(kctx) && Type == TYPE_float)
		return KType_float;
	else if(Type == TYPE_BoolObj)  { return KType_Object; }
	else if(Type == TYPE_IntObj)   { return KType_Object; }
	else if(Type == TYPE_FloatObj) { return KType_Object; }
	return (ktypeattr_t) Type;
}

static inline enum TypeId ToBoxType(enum TypeId Type)
{
	if(Type == TYPE_boolean) { return TYPE_BoolObj; }
	if(Type == TYPE_int    ) { return TYPE_IntObj;  }
	if(Type == TYPE_float  ) { return TYPE_FloatObj;}
	return Type;
}

static inline enum TypeId ToUnBoxType(enum TypeId Type)
{
	if(Type == TYPE_BoolObj ) { return TYPE_boolean; }
	if(Type == TYPE_IntObj  ) { return TYPE_int;     }
	if(Type == TYPE_FloatObj) { return TYPE_float;   }
	return Type;
}

#endif /* end of include guard */
