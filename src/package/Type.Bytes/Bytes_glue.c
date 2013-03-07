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

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/konoha_common.h>
#include <konoha3/import/methoddecl.h>

#ifdef __cplusplus
extern "C"{
#endif
#define I18NAPI PLATAPI I18NModule.
/* ------------------------------------------------------------------------ */
/* Bytes */

static void kBytes_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct kBytesVar *ba = (struct kBytesVar *)o;
	DBG_ASSERT((size_t)conf >= 0);
	ba->bytesize = (size_t)conf;
	ba->byteptr = (ba->bytesize > 0) ? (const char *)KCalloc((size_t)conf, 1, NULL) : NULL;
}

static void kBytes_Free(KonohaContext *kctx, kObject *o)
{
	struct kBytesVar *ba = (struct kBytesVar *)o;
	if(ba->byteptr != NULL) {
		KFree(ba->buf, ba->bytesize);
		ba->byteptr = NULL;
		ba->bytesize = 0;
	}
}

static void kBytes_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kBytes *ba = v[pos].asBytes;
	size_t i, j, n;
	for(j = 0; j * 16 < ba->bytesize; j++) {
		KLIB KBuffer_printf(kctx, wb, "%08x", (int)(j*16));
		for(i = 0; i < 16; i++) {
			n = j * 16 + i;
			if(n < ba->bytesize) {
				KLIB KBuffer_printf(kctx, wb, " %2x", (int)ba->utext[n]);
			}
			else {
				KLIB KBuffer_printf(kctx, wb, "%s", "   ");
			}
		}
		KLIB KBuffer_printf(kctx, wb, "%s", "    ");
		for(i = 0; i < 16; i++) {
			n = j * 16 + i;
			if(n < ba->bytesize && isprint(ba->utext[n])) {
				KLIB KBuffer_printf(kctx, wb, "%c", (int)ba->utext[n]);
			}
			else {
				KLIB KBuffer_printf(kctx, wb, "%s", " ");
			}
		}
		KLIB KBuffer_printf(kctx, wb, "\n");
	}
}

/* ------------------------------------------------------------------------ */

//## Bytes Bytes.new(int size);
static KMETHOD Bytes_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[0].asBytes;
	size_t size = (size_t)sfp[1].intValue;
	if(ba->byteptr == NULL)
		KFree(ba->buf, ba->bytesize);
	((struct kBytesVar *)ba)->byteptr  = (const char *)KCalloc(size, 1, NULL);
	((struct kBytesVar *)ba)->bytesize = size;
	KReturn(ba);
}

//## int Bytes.getSize();
static KMETHOD Bytes_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[0].asBytes;
	KReturnUnboxValue(ba->bytesize);
}

//## int Bytes.get(int n);
static KMETHOD Bytes_get(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[0].asBytes;
	size_t n = (size_t)sfp[1].intValue;
	KCheckIndex(n, ba->bytesize);
	KReturnUnboxValue(ba->utext[n]);
}

//## void Bytes.set(int index, int c);
static KMETHOD Bytes_Set(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[0].asBytes;
	size_t n = (size_t)sfp[1].intValue;
	KCheckIndex(n, ba->bytesize);
	ba->buf[n] = sfp[2].intValue;
	KReturnVoid();
}

//## void Bytes.setAll(int c);
static KMETHOD Bytes_SetAll(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[0].asBytes;
	int bytesize = ba->bytesize;
	memset(ba->buf, sfp[1].intValue, bytesize);
	KReturnVoid();
}

static void KBuffer_convertCharset(KonohaContext *kctx, KBuffer* wb, const char *targetCharset, const char *sourceCharset, const char *sourceBuf, size_t sourceSize, KTraceInfo *trace)
{
	uintptr_t conv = I18NAPI iconv_open_i(kctx, targetCharset, sourceCharset, trace);
	if(conv != ICONV_NULL) {
		KLIB KBuffer_iconv(kctx, wb, conv, sourceBuf, sourceSize, trace);
		I18NAPI iconv_close_i(kctx, conv);
	}
}

static kBytes* new_kBytes(KonohaContext *kctx, kArray *gcstack, KClass *c, const char *buf, size_t bufsiz)
{
	kBytes* ba = (kBytes *) KLIB new_kObject(kctx, gcstack, c, bufsiz);
	if(bufsiz > 0) {
		((struct kBytesVar *)ba)->byteptr  = (const char *)KCalloc(bufsiz, 1, NULL);
		((struct kBytesVar *)ba)->bytesize = bufsiz;
		memcpy(ba->buf, buf, bufsiz);
	}
	return ba;
}

//## Bytes String.toBytes();
static KMETHOD String_toBytes(KonohaContext *kctx, KonohaStack *sfp)
{
	kString* thisString = sfp[0].asString;
	size_t size = kString_size(thisString);
	if(PLATAPI I18NModule.isSystemCharsetUTF8(kctx)) {
		KReturn(new_kBytes(kctx, OnStack, KGetReturnType(sfp), kString_text(thisString), size));
	}
	else {
		KMakeTrace(trace, sfp);
		KBuffer wb;
		KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
		KBuffer_convertCharset(kctx, &wb, I18NAPI systemCharset, "UTF-8", kString_text(thisString), size, trace);
		KReturnWith(
			new_kBytes(kctx, OnStack, KGetReturnType(sfp), KLIB KBuffer_text(kctx, &wb, NonZero), KBuffer_bytesize(&wb)),
			KLIB KBuffer_Free(&wb)
		);
	}
}

//## String String.new(Bytes ba);
static KMETHOD String_new_fromBytes_withDefaultDecode(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[1].asBytes;
	kString *s = TS_EMPTY;
	if(ba->bytesize != 0) {
		KMakeTrace(trace, sfp);
		KBuffer wb;
		KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
		KBuffer_convertCharset(kctx, &wb, "UTF-8", I18NAPI systemCharset, ba->buf, ba->bytesize, trace);
		s = KLIB KBuffer_Stringfy(kctx, &wb, OnStack, StringPolicy_FreeKBuffer);
	}
	KReturn(s);
}

//## String String.new(Bytes ba, int offset, int length);
static KMETHOD String_new_fromSubBytes_withDefaultDecode(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[1].asBytes;
	int offset = sfp[2].intValue;
	int length = sfp[3].intValue;
	kString *s = TS_EMPTY;
	if(ba->bytesize != 0) {
		// At this point, we assuem 'ba' is null terminated.
		DBG_ASSERT(ba->buf[ba->bytesize-1] == '\0');
		KMakeTrace(trace, sfp);
		KBuffer wb;
		KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
		KBuffer_convertCharset(kctx, &wb, "UTF-8", I18NAPI systemCharset, ba->buf + offset, length, trace);
		s = KLIB KBuffer_Stringfy(kctx, &wb, OnStack, StringPolicy_FreeKBuffer);
	}
	KReturn(s);
}

//## String String.new(Bytes ba, int offset, int length, String charset);
static KMETHOD String_new_fromSubBytes_withSpecifiedDecode(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[1].asBytes;
	int offset = sfp[2].intValue;
	int length = sfp[3].intValue;
	kString *charsetStr = sfp[4].asString;
	const char *charset = kString_text(charsetStr);
	kString *s = TS_EMPTY;
	if(ba->bytesize != 0) {
		// At this point, we assuem 'ba' is null terminated.
		DBG_ASSERT(ba->buf[ba->bytesize-1] == '\0');
		KMakeTrace(trace, sfp);
		KBuffer wb;
		KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
		KBuffer_convertCharset(kctx, &wb, "UTF-8", charset, ba->buf + offset, length, trace);
		s = KLIB KBuffer_Stringfy(kctx, &wb, OnStack, StringPolicy_FreeKBuffer);
	}
	KReturn(s);
}

//## String String.new(Bytes ba, String charset);
static KMETHOD String_new_fromBytes_withSpecifiedDecode(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[1].asBytes;
	kString *charset = sfp[2].asString;
	kString *s = TS_EMPTY;
	if(ba->bytesize != 0) {
		// At this point, we assuem 'ba' is null terminated.
		DBG_ASSERT(ba->buf[ba->bytesize] == '\0');
		KMakeTrace(trace, sfp);
		KBuffer wb;
		KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
		KBuffer_convertCharset(kctx, &wb, kString_text(charset), "UTF-8", ba->buf, ba->bytesize, trace);
		s = KLIB KBuffer_Stringfy(kctx, &wb, OnStack, StringPolicy_FreeKBuffer);
	}
	KReturn(s);
}

/* ------------------------------------------------------------------------ */

static kbool_t bytes_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequireKonohaCommonModule(trace);
	//KImportPackageSymbol(ns, "cstyle", "$SingleQuotedChar", trace);
	if(KClass_Bytes == NULL) {
		KDEFINE_CLASS defBytes = {0};
		SETSTRUCTNAME(defBytes, Bytes);
		defBytes.cflag   = KClassFlag_Final;
		defBytes.free    = kBytes_Free;
		defBytes.init    = kBytes_Init;
		defBytes.format       = kBytes_format;
		KClass_Bytes = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defBytes, trace);
	}
	int FN_index = KFieldName_("index");
	int FN_c     = KFieldName_("c");
	int FN_size  = KFieldName_("size");
	KDEFINE_METHOD MethodData[] = {
		_Public,     _F(Bytes_new),     KType_Bytes,  KType_Bytes, KMethodName_("new"),     1, KType_Int, FN_size,
		_Public|_Im, _F(Bytes_getSize), KType_Int,    KType_Bytes, KMethodName_("getSize"), 0,
		_Public|_Im, _F(Bytes_get),     KType_Int,    KType_Bytes, KMethodName_("get"),     1, KType_Int, FN_index,
		_Public,     _F(Bytes_Set),     KType_void,   KType_Bytes, KMethodName_("set"),     2, KType_Int, FN_index, KType_Int, FN_c,
		_Public,     _F(Bytes_SetAll),  KType_void,   KType_Bytes, KMethodName_("setAll"),  1, KType_Int, FN_c,
		_Public|_Im|_Coercion, _F(String_toBytes), KType_Bytes, KType_String, KMethodName_To(KType_Bytes),   0,
		//_Public|_Im|_Coercion, _F(Bytes_toString), KType_String, KType_Bytes,  KMethodName_To(KType_String),  0,
		//_Public|_Const, _F(Bytes_encodeTo),   KType_Bytes,  KType_Bytes,  KMethodName_("encodeTo"),    1, KType_String, FN_encoding,
		//_Public|_Const, _F(Bytes_decodeFrom),   KType_String, KType_Bytes,  KMethodName_("decodeFrom"),    1, KType_String, FN_encoding,
		_Public, _F(String_new_fromBytes_withDefaultDecode),      KType_String, KType_String, KMethodName_("new"), 1, KType_Bytes, KFieldName_("ba"),
		_Public, _F(String_new_fromBytes_withSpecifiedDecode),    KType_String, KType_String, KMethodName_("new"), 2, KType_Bytes, KFieldName_("ba"), KType_String, KFieldName_("charset"),
		_Public, _F(String_new_fromSubBytes_withDefaultDecode),   KType_String, KType_String, KMethodName_("new"), 3, KType_Bytes, KFieldName_("ba"), KType_Int,    KFieldName_("offset"), KType_Int, KFieldName_("length"),
		_Public, _F(String_new_fromSubBytes_withSpecifiedDecode), KType_String, KType_String, KMethodName_("new"), 4, KType_Bytes, KFieldName_("ba"), KType_Int,    KFieldName_("offset"), KType_Int, KFieldName_("length"), KType_String, KFieldName_("charset"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

static kbool_t bytes_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *Bytes_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", K_VERSION);
	d.PackupNameSpace    = bytes_PackupNameSpace;
	d.ExportNameSpace   = bytes_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
