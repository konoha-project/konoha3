/****************************************************************************
 * Copyright (c) 2012, the Konoha project authors. All rights reserved.
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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/klib.h>
#include <minikonoha/import/methoddecl.h>
#define _JS     kMethod_JSCompatible

#include "string_api.h"
#include "rope_string.h"
#include "java_string.h"

#ifdef __cplusplus
extern "C" {
#endif
/* ------------------------------------------------------------------------ */

//## boolean String.<(String s);
static KMETHOD String_opLT(KonohaContext *kctx, KonohaStack *sfp)
{
	int res = String_compareTo(kctx, sfp[0].asString, sfp[1].asString);
	KReturnUnboxValue(res < 0);
}

//## boolean String.<=(String s);
static KMETHOD String_opLTE(KonohaContext *kctx, KonohaStack *sfp)
{
	int res = String_compareTo(kctx, sfp[0].asString, sfp[1].asString);
	KReturnUnboxValue(res <= 0);
}

//## boolean String.>(String s);
static KMETHOD String_opGT(KonohaContext *kctx, KonohaStack *sfp)
{
	int res = String_compareTo(kctx, sfp[0].asString, sfp[1].asString);
	KReturnUnboxValue(res > 0);
}

//## boolean String.>=(String s);
static KMETHOD String_opGTE(KonohaContext *kctx, KonohaStack *sfp)
{
	int res = String_compareTo(kctx, sfp[0].asString, sfp[1].asString);
	KReturnUnboxValue(res >= 0);
}

//## String String.get(int index);
static KMETHOD String_get(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *self = sfp[0].asString;
	size_t offset = sfp[1].intValue;
	if(offset >= kString_size(self)) {
		Throw_IndexOutOfBoundsException(kctx, sfp, sfp[1].intValue);
	}
	kString *ret = (kString_Is(ASCII, self)) ?
		KLIB new_kString(kctx, OnStack, kString_text(self) + offset, 1, StringPolicy_ASCII) :
		new_UTF8SubString(kctx, self, offset, 1);
	KReturn(ret);
}

//## String String.fromCharCode(int charCode);
static KMETHOD String_fromCharCode(KonohaContext *kctx, KonohaStack *sfp)
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

static kbool_t string_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	LoadRopeMethod(kctx, ns, trace);
	int FN_s = KFieldName_("s");
	int FN_n = KFieldName_("n");

	KDEFINE_METHOD MethodData[] = {
		_JS|_Public|_Const|_Im, _F(String_opLT),  KType_boolean, KType_String, KMethodName_("<"),  1, KType_String, FN_s,
		_JS|_Public|_Const|_Im, _F(String_opLTE),  KType_boolean, KType_String, KMethodName_("<="),  1, KType_String, FN_s,
		_JS|_Public|_Const|_Im, _F(String_opGT),  KType_boolean, KType_String, KMethodName_(">"),  1, KType_String, FN_s,
		_JS|_Public|_Const|_Im, _F(String_opGTE),  KType_boolean, KType_String, KMethodName_(">="),  1, KType_String, FN_s,
		_JS|_Public|_Const|_Im, _F(KString_length), KType_int, KType_String, KMethodName_("getlength"), 0,
		_JS|_Public|_Const|_Im, _F(String_get), KType_String, KType_String, KMethodName_("charAt"), 1, KType_int, FN_n,
		_JS|_Public|_Static|_Const|_Im, _F(String_fromCharCode), KType_String, KType_String, KMethodName_("fromCharCode"), 1, KType_int, FN_n,
		_JS|_Public|_Const|_Im|kMethod_Override, _F(String_get),        KType_String, KType_String, KMethodName_("get"), 1, KType_int, FN_n,  /* [c] */
		_JS|_Public|_Const|_Im, _F(KString_charAt), KType_int, KType_String, KMethodName_("charCodeAt"), 1, KType_int, FN_n,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	// It is good to move these apis to other package such as java.string
	LoadJavaAPI(kctx, ns, trace);
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

KDEFINE_PACKAGE *string_Init(void)
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
