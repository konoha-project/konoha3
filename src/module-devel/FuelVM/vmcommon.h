#include <stdint.h>
#include <stdbool.h>
#include "../../../include/minikonoha/minikonoha.h"
#include "../../../include/minikonoha/konoha_common.h"

#ifndef VMCOMMON_H
#define VMCOMMON_H

enum TypeId {
	TYPE_void = KType_void,
	TYPE_Object = KType_Object,
	TYPE_boolean = KType_boolean,
	TYPE_int = KType_int,
	TYPE_String = KType_String,
	TYPE_Function = KType_Func,
	TYPE_Array = KType_Array,
	TYPE_Method = KType_Method,
	TYPE_NameSpace = KType_NameSpace,
	TYPE_Any = KType_0,
	TYPE_float
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
	return type;
}

#endif /* end of include guard */
