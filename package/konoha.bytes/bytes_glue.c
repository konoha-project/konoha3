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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/bytes.h>

#include <errno.h> // include this because of E2BIG

#ifdef __cplusplus
extern "C"{
#endif

/* ------------------------------------------------------------------------ */
/* Bytes */

static void kBytes_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct kBytesVar *ba = (struct kBytesVar*)o;
	DBG_ASSERT((size_t)conf >= 0);
	ba->bytesize = (size_t)conf;
	ba->byteptr = NULL;
	ba->byteptr = (ba->bytesize > 0) ? (const char *)KCalloc_UNTRACE((size_t)conf, 1) : NULL;
}

static void kBytes_free(KonohaContext *kctx, kObject *o)
{
	struct kBytesVar *ba = (struct kBytesVar*)o;
	if(ba->byteptr != NULL) {
		KFree(ba->buf, ba->bytesize);
		ba->byteptr = NULL;
		ba->bytesize = 0;
	}
}

static void kBytes_p(KonohaContext *kctx, KonohaValue *v, int pos, KGrowingBuffer *wb)
{
	kBytes *ba = v[pos].asBytes;
	size_t i, j, n;
	for(j = 0; j * 16 < ba->bytesize; j++) {
		KLIB Kwb_printf(kctx, wb, "%08x", (int)(j*16));
		for(i = 0; i < 16; i++) {
			n = j * 16 + i;
			if(n < ba->bytesize) {
				KLIB Kwb_printf(kctx, wb, " %2x", (int)ba->utext[n]);
			}
			else {
				KLIB Kwb_printf(kctx, wb, "%s", "   ");
			}
		}
		KLIB Kwb_printf(kctx, wb, "%s", "    ");
		for(i = 0; i < 16; i++) {
			n = j * 16 + i;
			if(n < ba->bytesize && isprint(ba->utext[n])) {
				KLIB Kwb_printf(kctx, wb, "%c", (int)ba->utext[n]);
			}
			else {
				KLIB Kwb_printf(kctx, wb, "%s", " ");
			}
		}
		KLIB Kwb_printf(kctx, wb, "\n");
	}
}

/* ------------------------------------------------------------------------ */

#define CONV_BUFSIZE 4096 // 4K
#define MAX_STORE_BUFSIZE (CONV_BUFSIZE * 1024)// 4M

//static kBytes* Convert_newBytes(KonohaContext *kctx, kArray *gcstack, kBytes *sourceBytes, const char *fromCharset, const char *toCharset)
//{
//	kiconv_t conv;
//	KGrowingBuffer wb;
//
//	char convBuf[CONV_BUFSIZE] = {0};
//	char *presentPtrFrom = sourceBytes->buf;
//	char ** inbuf = &presentPtrFrom;
//	char *presentPtrTo = convBuf;
//	char ** outbuf = &presentPtrTo;
//	size_t inBytesLeft, outBytesLeft;
//	inBytesLeft = sourceBytes->bytesize;
//	outBytesLeft = CONV_BUFSIZE;
//	//DBG_P("from='%s' inBytesLeft=%d, to='%s' outBytesLeft=%d", fromCharset, inBytesLeft, toCharset, outBytesLeft);
//
//	if(strncmp(fromCharset, toCharset, strlen(fromCharset)) == 0) {
//		// no need to convert.
//		return sourceBytes;
//	}
//	conv = (kiconv_t)PLATAPI iconv_open_i(toCharset, fromCharset);
//	if(conv == (kiconv_t)(-1)) {
//		OLDTRACE_SWITCH_TO_KTrace(_UserInputFault,
//				LogText("@","iconv_open"),
//				LogText("from", fromCharset),
//				LogText("to", toCharset)
//		);
//		// FIXME
//		return NULL;
//	}
//	size_t iconv_ret = -1;
//	size_t processedSize = 0;
//	size_t processedTotalSize = processedSize;
//	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
//	while (inBytesLeft > 0 && iconv_ret == -1) {
//		iconv_ret = PLATAPI iconv_i((uintptr_t)conv, inbuf, &inBytesLeft, outbuf, &outBytesLeft);
//		if(iconv_ret == -1 && errno == E2BIG) {
//			// input is too big.
//			processedSize = CONV_BUFSIZE - outBytesLeft;
//			processedTotalSize += processedSize;
//			KLIB Kwb_printf(kctx, &wb, "%s", convBuf);
//			// reset convbuf
//			presentPtrTo = convBuf;
////			outbuf = &presentPtrTo;
//			memset(convBuf, '\0', CONV_BUFSIZE);
//			outBytesLeft = CONV_BUFSIZE;
//		} else if(iconv_ret == -1) {
//			OLDTRACE_SWITCH_TO_KTrace(_DataFault,
//				LogText("@","iconv"),
//				LogText("from", "UTF-8"),
//				LogText("to", toCharset),
//				LogText("error", strerror(errno))
//			);
//			KLIB Kwb_free(&wb);
//			return NULL;   // FIXME
//			//return (kBytes*)(CT_Bytes->defaultNullValue_OnGlobalConstList);
//
//		} else {
//			// finished. iconv_ret != -1
//			processedSize = CONV_BUFSIZE - outBytesLeft;
//			processedTotalSize += processedSize;
//			DBG_P("conv_buf=%d outBytes=%d proceedSize=%d totalsize=%d", CONV_BUFSIZE, outBytesLeft, processedSize, processedTotalSize);
//			KLIB Kwb_write(kctx, &wb, convBuf, processedSize);
//			//KLIB Kwb_printf(kctx, &wb, "%s", convBuf);
//		}
//	} /* end of converting loop */
//	PLATAPI iconv_close_i((uintptr_t)conv);
//
//	const char *bufferTopChar = KLIB Kwb_top(kctx, &wb, 1);
//	struct kBytesVar *targetBytes = NULL; // FIXME (struct kBytesVar*)KLIB new_kObjectDontUseThis(kctx, CT_Bytes, processedTotalSize, gcstack); // ensure bytes ends with Zero
//	memcpy(targetBytes->buf, bufferTopChar, processedTotalSize); // including NUL terminate by ensuredZeo
//	KLIB Kwb_free(&wb);
//	return targetBytes;
//}

////## @Const method Bytes Bytes.encodeTo(String charset);
//static KMETHOD Bytes_encodeTo(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kBytes *ba = sfp[0].asBytes;
//	kString *toCoding = sfp[1].asString;
//
//	KReturn(Convert_newBytes(kctx, OnStack, ba, "UTF-8", S_text(toCoding)));
//}
//
//static kString *Convert_newString(KonohaContext *kctx, kArray *gcstack, kBytes *ba)
//{
//	if(ba->buf == NULL || ba->bytesize == 0) {
//		return TS_EMPTY;
//	} else {
//		// At this point, we assuem 'ba' is null terminated.
//		DBG_ASSERT(ba->buf[ba->bytesize-1] == '\0');
//		return KLIB new_kString(kctx, gcstack, ba->buf, ba->bytesize-1, 0);
//	}
//}
//
////## @Const method String Bytes.decodeFrom(String fromEncoding);
//static KMETHOD Bytes_decodeFrom(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kBytes* sourceBytes = sfp[0].asBytes;
//	kString*fromCharset = sfp[1].asString;
//	kBytes *targetBytes = NULL;
////	DBG_P("size=%d, '%s'", sourceBytes->bytesize, sourceBytes->buf);
////	DBG_P("fromCharset:%p, %s", fromCharset, S_text(fromCharset));
//	if(sourceBytes->bytesize == 0) {
//		KReturn(KNULL(String));
//	}
//	INIT_GCSTACK();
//	if(fromCharset != (kString*)(CT_String->defaultNullValue_OnGlobalConstList)) {
//		targetBytes = Convert_newBytes(kctx, _GcStack, sourceBytes, S_text(fromCharset), "UTF-8");
//	} else {
//		// conv from default encoding
//		targetBytes = Convert_newBytes(kctx, _GcStack, sourceBytes, PLATAPI isSystemCharsetUTF8(kctx), "UTF-8");
//	}
////	DBG_P("size=%d, '%s'", targetBytes->bytesize, targetBytes->buf);
//	KReturnWithRESET_GCSTACK(Convert_newString(kctx, _GcStack, targetBytes));
//}

//## @Const String Bytes.toString();
static KMETHOD Bytes_toString(KonohaContext *kctx, KonohaStack *sfp)
{
//	kBytes *from = sfp[0].asBytes;
//	INIT_GCSTACK();
//
//	kBytes *to = Convert_newBytes(kctx, _GcStack, from, PLATAPI isSystemCharsetUTF8(kctx), "UTF-8");
//	KReturnWithRESET_GCSTACK(Convert_newString(kctx, _GcStack, to));
}

//## Bytes Bytes.new(int size);
static KMETHOD Bytes_new(KonohaContext *kctx, KonohaStack *sfp)
{
	size_t size = (size_t)sfp[1].intValue;
	KReturn(KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), size));
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
static KMETHOD Bytes_set(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[0].asBytes;
	size_t n = (size_t)sfp[1].intValue;
	KCheckIndex(n, ba->bytesize);
	ba->buf[n] = sfp[2].intValue;
	KReturnVoid();
}

//## void Bytes.setAll(int c);
static KMETHOD Bytes_setAll(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[0].asBytes;
	int bytesize = ba->bytesize;
	int i;
	for (i = 0; i < bytesize; i++) {
		ba->buf[i] = sfp[2].intValue;
	}
	KReturnVoid();
}

static void Kwb_convertCharset(KonohaContext *kctx, KGrowingBuffer* wb, const char *targetCharset, const char *sourceCharset, const char *sourceBuf, size_t sourceSize, KTraceInfo *trace)
{
	uintptr_t iconv = PLATAPI iconv_open_i(kctx, targetCharset, sourceCharset, trace);
	if(iconv != ICONV_NULL) {
		//KLIB Kwb_iconv(kctx, wb, iconv, sourceBuf, sourceSize, trace);
		PLATAPI iconv_close_i(kctx, iconv);
	}
}

static kBytes* new_kBytes(KonohaContext *kctx, kArray *gcstack, KonohaClass *c, const char *buf, size_t bufsiz)
{
	kBytes* ba = (kBytes*) KLIB new_kObject(kctx, gcstack, c, bufsiz);
	if(bufsiz > 0) {
		memcpy(ba->buf, buf, bufsiz);
	}
	return ba;
}

//## @Const Bytes String.toBytes();
static KMETHOD String_toBytes(KonohaContext *kctx, KonohaStack *sfp)
{
	kString* thisString = sfp[0].asString;
	size_t size = S_size(thisString);
	if(PLATAPI isSystemCharsetUTF8(kctx)) {
		KReturn(new_kBytes(kctx, OnStack, KGetReturnType(sfp), S_text(thisString), size));
	}
	else {
		KMakeTrace(trace, sfp);
		KGrowingBuffer wb;
		KLIB Kwb_init(&(kctx->stack->cwb), &wb);
		Kwb_convertCharset(kctx, &wb, PLATAPI systemCharset, "UTF-8", S_text(thisString), size, trace);
		KReturnWith(
			new_kBytes(kctx, OnStack, KGetReturnType(sfp), KLIB Kwb_top(kctx, &wb, 0), Kwb_bytesize(&wb)),
			KLIB Kwb_free(&wb)
		);
	}
}

//## Bytes String.getBytes(String charset);
static KMETHOD String_getBytes(KonohaContext *kctx, KonohaStack *sfp)
{

}



/* ------------------------------------------------------------------------ */

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Im       kMethod_Immutable
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

static kbool_t bytes_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	KDEFINE_CLASS defBytes = {0};
	SETSTRUCTNAME(defBytes, Bytes);
	defBytes.cflag   = kClass_Final;
	defBytes.free    = kBytes_free;
	defBytes.init    = kBytes_init;
	defBytes.p       = kBytes_p;

	KonohaClass *cBytes = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defBytes, trace);
	int TY_Bytes = cBytes->typeId;
	int FN_encoding = FN_("encoding");
	int FN_index = FN_("index");
	int FN_x = FN_("x");
	int FN_c = FN_("c");
	int FN_size = FN_("size");
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Bytes_new), TY_Bytes, TY_Bytes, MN_("new"), 1, TY_int, FN_size,
		_Public|_Im, _F(Bytes_getSize), TY_int, TY_Bytes, MN_("getSize"), 0,
		_Public|_Im, _F(Bytes_get), TY_int, TY_Bytes, MN_("get"), 1, TY_int, FN_index,
		_Public, _F(Bytes_set), TY_void, TY_Bytes, MN_("set"), 2, TY_int, FN_index, TY_int, FN_c,
		_Public, _F(Bytes_setAll), TY_void, TY_Bytes, MN_("setAll"), 1, TY_int, FN_c,
		_Public|_Im|_Coercion, _F(String_toBytes), TY_Bytes,  TY_String, MN_to(TY_Bytes),   0,
		_Public|_Im|_Coercion, _F(Bytes_toString), TY_String, TY_Bytes,  MN_to(TY_String),  0,
		//		_Public|_Const, _F(Bytes_encodeTo),   TY_Bytes,  TY_Bytes,  MN_("encodeTo"),    1, TY_String, FN_encoding,
		//		_Public|_Const, _F(Bytes_decodeFrom),   TY_String, TY_Bytes,  MN_("decodeFrom"),    1, TY_String, FN_encoding,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t bytes_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	return true;
}

static KMETHOD TokenFunc_SingleQuotedChar(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *)sfp[1].asObject;
	int ch, prev = '/', pos = 1;
	const char *source = S_text(sfp[2].asString);
	while((ch = source[pos++]) != 0) {
		if(ch == '\n') {
			break;
		}
		if(ch == '\'' && prev != '\\') {
			if(IS_NOTNULL(tk)) {
				KFieldSet(tk, tk->text, KLIB new_kString(kctx, OnField, source + 1, (pos-2), 0));
				tk->unresolvedTokenType = SYM_("$SingleQuotedChar");
			}
			KReturnUnboxValue(pos);
		}
		prev = ch;
	}
	if(IS_NOTNULL(tk)) {
		SUGAR kToken_printMessage(kctx, tk, ErrTag, "must close with %s", "'");
	}
	KReturnUnboxValue(0);
}

static KMETHOD TypeCheck_SingleQuotedChar(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->termToken;
	kString *s = tk->text;
	DBG_P("string:'%s'", S_text(s));
	if(S_size(s) == 1) {
		int ch = S_text(s)[0];
		KReturn(SUGAR kExpr_setUnboxConstValue(kctx, expr, TY_int, ch));
	} else {
		SUGAR kStmt_printMessage2(kctx, stmt, (kToken*)expr, ErrTag, "single quote doesn't accept multi characters, '%s'", S_text(s));
	}
	KReturn(K_NULLEXPR);
}

static kbool_t bytes_initNameSpace(KonohaContext *kctx, kNameSpace *packageNS, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("$SingleQuotedChar"), 0, NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_SingleQuotedChar, },
		{ KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNS);
	SUGAR kNameSpace_setTokenFunc(kctx, ns, SYM_("$SingleQuotedChar"), KonohaChar_Quote, new_SugarFunc(ns, TokenFunc_SingleQuotedChar));
	return true;
}

static kbool_t bytes_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNS, kNameSpace *ns, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* bytes_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "bytes", "1.0");
	d.initPackage    = bytes_initPackage;
	d.setupPackage   = bytes_setupPackage;
	d.initNameSpace  = bytes_initNameSpace;
	d.setupNameSpace = bytes_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
