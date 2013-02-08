/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/

/* ************************************************************************ */

#define USE_STRINGLIB 1

#include <konoha3/konoha.h>
#include <konoha3/klib.h>
#include <konoha3/import/methoddecl.h>

#include "string_api.h"
#include "rope_string.h"
#include "java_string.h"

#ifdef __cplusplus
extern "C" {
#endif
/* ------------------------------------------------------------------------ */

//## boolean String.<(String s);
static KMETHOD StringUtil_opLT(KonohaContext *kctx, KonohaStack *sfp)
{
	int res = String_compareTo(kctx, sfp[0].asString, sfp[1].asString);
	KReturnUnboxValue(res < 0);
}

//## boolean String.<=(String s);
static KMETHOD StringUtil_opLTE(KonohaContext *kctx, KonohaStack *sfp)
{
	int res = String_compareTo(kctx, sfp[0].asString, sfp[1].asString);
	KReturnUnboxValue(res <= 0);
}

//## boolean String.>(String s);
static KMETHOD StringUtil_opGT(KonohaContext *kctx, KonohaStack *sfp)
{
	int res = String_compareTo(kctx, sfp[0].asString, sfp[1].asString);
	KReturnUnboxValue(res > 0);
}

//## boolean String.>=(String s);
static KMETHOD StringUtil_opGTE(KonohaContext *kctx, KonohaStack *sfp)
{
	int res = String_compareTo(kctx, sfp[0].asString, sfp[1].asString);
	KReturnUnboxValue(res >= 0);
}

//## @Static String String.get(String self, int index);
static KMETHOD StringUtil_get(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *self = sfp[1].asString;
	size_t offset = sfp[2].intValue;
	if(offset >= kString_size(self)) {
		Throw_IndexOutOfBoundsException(kctx, sfp, sfp[2].intValue);
	}
	kString *ret = (kString_Is(ASCII, self)) ?
		KLIB new_kString(kctx, OnStack, kString_text(self) + offset, 1, StringPolicy_ASCII) :
		new_UTF8SubString(kctx, self, offset, 1);
	KReturn(ret);
}

//## @Static String String.fromCharCode(int charCode);
static KMETHOD StringUtil_fromCharCode(KonohaContext *kctx, KonohaStack *sfp)
{
	kint_t c = sfp[1].intValue;
	if(c < 0x0000 || c > 0x10FFFF) { /* FIXME: out of unicode range */
		KReturn(KNULL(String));
	}
	char buf[5] = {0};
	size_t length = 0;
	int policy = 0;
	if(c <= 0x007F) { /* 1 byte */
		buf[0] = (char)c;
		length = 1;
		policy |= StringPolicy_ASCII;
	}
	else if(c <= 0x07FF) { /* 2 bytes */
		buf[0] = (char)(0xC0 | (c >> 6));
		buf[1] = (char)(0x80 | (c & 0x3F));
		length = 2;
		policy |= StringPolicy_UTF8;
	}
	else if(c <= 0xFFFF) { /* 3 bytes */
		buf[0] = (char)(0xE0 | (c >> 12));
		buf[1] = (char)(0x80 | ((c >> 6) & 0x3F));
		buf[2] = (char)(0x80 | (c & 0x3F));
		length = 3;
		policy |= StringPolicy_UTF8;
	}
	else { /* 4 bytes */
		buf[0] = (char)(0xF0 | (c >> 18));
		buf[1] = (char)(0x80 | ((c >> 12) & 0x3F));
		buf[2] = (char)(0x80 | ((c >> 6) & 0x3F));
		buf[3] = (char)(0x80 | (c & 0x3F));
		length = 4;
		policy |= StringPolicy_UTF8;
	}
	KReturn(KLIB new_kString(kctx, OnStack, (const char *)buf, length, policy));
}

// --------------------------------------------------------------------------
//
typedef struct kStringUtil {
	kObjectHeader h;
} kStringUtil;

static kbool_t string_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	int FN_t = KFieldName_("t");
	int FN_s = KFieldName_("s");
	int FN_n = KFieldName_("n");
	KDEFINE_CLASS StringUtilDef = {
		STRUCTNAME(StringUtil),
		.cflag = 0,
	};
	KClass *cStringUtil = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &StringUtilDef, trace);
	int KType_StringUtil = cStringUtil->typeId;

	LoadRopeMethod(kctx, ns, trace, KType_StringUtil);
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(StringUtil_opLT),        KType_Boolean,  KType_String, KMethodName_("<"),  1, KType_String, FN_s,
		_Public|_Const|_Im, _F(StringUtil_opLTE),       KType_Boolean,  KType_String, KMethodName_("<="),  1, KType_String, FN_s,
		_Public|_Const|_Im, _F(StringUtil_opGT),        KType_Boolean,  KType_String, KMethodName_(">"),  1, KType_String, FN_s,
		_Public|_Const|_Im, _F(StringUtil_opGTE),       KType_Boolean,  KType_String, KMethodName_(">="),  1, KType_String, FN_s,
		_Static|_Public|_Const|_Im, _F(StringUtil_length),      KType_Int,      KType_StringUtil, KMethodName_("getlength"),   1, KType_String, FN_t, 
		_Static|_Public|_Const|_Im, _F(StringUtil_get),         KType_String,   KType_StringUtil, KMethodName_("charAt"),      2, KType_String, FN_t, KType_Int, FN_n,
		_Static|_Public|_Const|_Im, _F(StringUtil_fromCharCode),KType_String,   KType_StringUtil, KMethodName_("fromCharCode"),1, KType_Int, FN_n,
		_Static|_Public|_Const|_Im, _F(StringUtil_get),         KType_String,   KType_StringUtil, KMethodName_("getString"),         2, KType_String, FN_t, KType_Int, FN_n,  /* [c] */
		_Static|_Public|_Const|_Im, _F(StringUtil_charAt),      KType_Int,      KType_StringUtil, KMethodName_("charCodeAt"),  2, KType_String, FN_t, KType_Int, FN_n,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	LoadJavaAPI(kctx, ns, trace, KType_StringUtil);

	KSetClassFunc(ns->packageId, KClass_String, unbox, String2_unbox, trace);
	KSetClassFunc(ns->packageId, KClass_String, free, String2_Free, trace);
	KSetClassFunc(ns->packageId, KClass_String, reftrace, String2_Reftrace, trace);
	KSetKLibFunc(ns->packageId, new_kString, new_kString, trace);
	return true;
}

static kbool_t string_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *StringUtil_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", "1.0");
	d.PackupNameSpace    = string_PackupNameSpace;
	d.ExportNameSpace   = string_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
