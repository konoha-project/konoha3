#include <stdint.h>
#include <stdbool.h>
#include "../../include/minikonoha/minikonoha.h"

#ifndef VMCOMMON_H
#define VMCOMMON_H

enum TypeId {
	TYPE_void = TY_void,
	TYPE_boolean = TY_boolean,
	TYPE_int = TY_int,
	TYPE_String = TY_String,
	TYPE_Function = TY_Func,
	TYPE_Array = TY_Array,
	TYPE_Method = TY_Method,
	TYPE_Any = TY_0,
	TYPE_float
};

typedef union SValue {
	int64_t     ival;
	double      dval;
	bool        bval;
	void       *ptr;
	const char *str;
	uint64_t    bits;
	kObject    *obj;
} SValue;

typedef struct LObject {
	void *Header;
	SValue fields[4];
} LObject;

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

#endif /* end of include guard */
