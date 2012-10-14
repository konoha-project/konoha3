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

// Bytes_init

static void kBytes_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct kBytesVar *ba = (struct kBytesVar*)o;
	DBG_ASSERT((size_t)conf >= 0);
	ba->bytesize = (size_t)conf;
	ba->byteptr = NULL;
	ba->byteptr = (ba->bytesize > 0) ? (const char *)KCALLOC((size_t)conf, 1) : NULL;
}

static void kBytes_free(KonohaContext *kctx, kObject *o)
{
	struct kBytesVar *ba = (struct kBytesVar*)o;
	if(ba->byteptr != NULL) {
		KFREE(ba->buf, ba->bytesize);
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

static kbool_t Kwb_iconv(KonohaContext *kctx, KGrowingBuffer* wb, kiconv_t conv, const char *sourceBuf, size_t sourceSize)
{
	char convBuf[K_PAGESIZE];
	char *presentPtrFrom = (char*)sourceBuf;
	char *presentPtrTo = convBuf;
	char ** inbuf = &presentPtrFrom;
	char ** outbuf = &presentPtrTo;
	size_t inBytesLeft = sourceSize, outBytesLeft = K_PAGESIZE;

	while (inBytesLeft > 0) {
		memset(convBuf, '\0', K_PAGESIZE);
		size_t iconv_ret = PLATAPI iconv_i((uintptr_t)conv, inbuf, &inBytesLeft, outbuf, &outBytesLeft);
		size_t processedSize = K_PAGESIZE - outBytesLeft;
		KLIB Kwb_write(kctx, wb, convBuf, processedSize);
		if(iconv_ret == -1) {
			if(errno == E2BIG) {   // input is too big.
				// reset convbuf
				presentPtrTo = convBuf;
				outBytesLeft = K_PAGESIZE;
				continue;
			}
			return false;
		}
	} /* end of converting loop */
	return true;
}

static kBytes* Convert_newBytes(KonohaContext *kctx, kArray *gcstack, kBytes *sourceBytes, const char *fromCharset, const char *toCharset)
{
	kiconv_t conv;
	KGrowingBuffer wb;

	char convBuf[CONV_BUFSIZE] = {0};
	char *presentPtrFrom = sourceBytes->buf;
	char ** inbuf = &presentPtrFrom;
	char *presentPtrTo = convBuf;
	char ** outbuf = &presentPtrTo;
	size_t inBytesLeft, outBytesLeft;
	inBytesLeft = sourceBytes->bytesize;
	outBytesLeft = CONV_BUFSIZE;
	//DBG_P("from='%s' inBytesLeft=%d, to='%s' outBytesLeft=%d", fromCharset, inBytesLeft, toCharset, outBytesLeft);

	if(strncmp(fromCharset, toCharset, strlen(fromCharset)) == 0) {
		// no need to convert.
		return sourceBytes;
	}
	conv = (kiconv_t)PLATAPI iconv_open_i(toCharset, fromCharset);
	if(conv == (kiconv_t)(-1)) {
		OLDTRACE_SWITCH_TO_KTrace(_UserInputFault,
				LogText("@","iconv_open"),
				LogText("from", fromCharset),
				LogText("to", toCharset)
		);
		// FIXME
		return NULL;
	}
	size_t iconv_ret = -1;
	size_t processedSize = 0;
	size_t processedTotalSize = processedSize;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	while (inBytesLeft > 0 && iconv_ret == -1) {
		iconv_ret = PLATAPI iconv_i((uintptr_t)conv, inbuf, &inBytesLeft, outbuf, &outBytesLeft);
		if(iconv_ret == -1 && errno == E2BIG) {
			// input is too big.
			processedSize = CONV_BUFSIZE - outBytesLeft;
			processedTotalSize += processedSize;
			KLIB Kwb_printf(kctx, &wb, "%s", convBuf);
			// reset convbuf
			presentPtrTo = convBuf;
//			outbuf = &presentPtrTo;
			memset(convBuf, '\0', CONV_BUFSIZE);
			outBytesLeft = CONV_BUFSIZE;
		} else if(iconv_ret == -1) {
			OLDTRACE_SWITCH_TO_KTrace(_DataFault,
				LogText("@","iconv"),
				LogText("from", "UTF-8"),
				LogText("to", toCharset),
				LogText("error", strerror(errno))
			);
			KLIB Kwb_free(&wb);
			return NULL;   // FIXME
			//return (kBytes*)(CT_Bytes->defaultNullValue_OnGlobalConstList);

		} else {
			// finished. iconv_ret != -1
			processedSize = CONV_BUFSIZE - outBytesLeft;
			processedTotalSize += processedSize;
			DBG_P("conv_buf=%d outBytes=%d proceedSize=%d totalsize=%d", CONV_BUFSIZE, outBytesLeft, processedSize, processedTotalSize);
			KLIB Kwb_write(kctx, &wb, convBuf, processedSize);
			//KLIB Kwb_printf(kctx, &wb, "%s", convBuf);
		}
	} /* end of converting loop */
	PLATAPI iconv_close_i((uintptr_t)conv);

	const char *bufferTopChar = KLIB Kwb_top(kctx, &wb, 1);
	struct kBytesVar *targetBytes = NULL; // FIXME (struct kBytesVar*)KLIB new_kObjectDontUseThis(kctx, CT_Bytes, processedTotalSize, gcstack); // ensure bytes ends with Zero
	memcpy(targetBytes->buf, bufferTopChar, processedTotalSize); // including NUL terminate by ensuredZeo
	KLIB Kwb_free(&wb);
	return targetBytes;
}

//## @Const method Bytes Bytes.encodeTo(String toEncoding);
static KMETHOD Bytes_encodeTo(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[0].asBytes;
	kString *toCoding = sfp[1].asString;

	RETURN_(Convert_newBytes(kctx, OnStack, ba, "UTF-8", S_text(toCoding)));
}

static kString *Convert_newString(KonohaContext *kctx, kArray *gcstack, kBytes *ba)
{
	if(ba->buf == NULL || ba->bytesize == 0) {
		return TS_EMPTY;
	} else {
		// At this point, we assuem 'ba' is null terminated.
		return KLIB new_kString(kctx, gcstack, ba->buf, ba->bytesize-1, 0);
	}
}

//## @Const method String Bytes.decodeFrom(String fromEncoding);
static KMETHOD Bytes_decodeFrom(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes* sourceBytes = sfp[0].asBytes;
	kString*fromCharset = sfp[1].asString;
	kBytes *targetBytes = NULL;
//	DBG_P("size=%d, '%s'", sourceBytes->bytesize, sourceBytes->buf);
//	DBG_P("fromCharset:%p, %s", fromCharset, S_text(fromCharset));
	if(sourceBytes->bytesize == 0) {
		RETURN_(KNULL(String));
	}
	INIT_GCSTACK();
	if(fromCharset != (kString*)(CT_String->defaultNullValue_OnGlobalConstList)) {
		targetBytes = Convert_newBytes(kctx, _GcStack, sourceBytes, S_text(fromCharset), "UTF-8");
	} else {
		// conv from default encoding
		targetBytes = Convert_newBytes(kctx, _GcStack, sourceBytes, PLATAPI getSystemCharset(), "UTF-8");
	}
//	DBG_P("size=%d, '%s'", targetBytes->bytesize, targetBytes->buf);
	KReturnWithRESET_GCSTACK(Convert_newString(kctx, _GcStack, targetBytes));
}

//## @Const method Bytes String.toBytes();
static KMETHOD String_toBytes(KonohaContext *kctx, KonohaStack *sfp)
{
	kString* s = sfp[0].asString;
	size_t size = S_size(s);
	kBytes* ba = (kBytes*)KLIB new_kObject(kctx, OnStack, KReturnType(sfp), (size>0) ? size + 1 : 0);
	if(size > 0) {
		memcpy(ba->buf, S_text(s), size+1); // including NUL char
		DBG_ASSERT(ba->buf[S_size(s)] == '\0');
	}
	RETURN_(ba);
}

//## @Const method String Bytes.toString();
static KMETHOD Bytes_toString(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *from = sfp[0].asBytes;
	INIT_GCSTACK();
	kBytes *to = Convert_newBytes(kctx, _GcStack, from, PLATAPI getSystemCharset(), "UTF-8");
	KReturnWithRESET_GCSTACK(Convert_newString(kctx, _GcStack, to));
}

//## Int Bytes.get(Int n);
static KMETHOD Bytes_get(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[0].asBytes;
	size_t n = check_index(kctx, sfp[1].intValue, ba->bytesize, sfp[K_RTNIDX].callerFileLine);
	RETURNi_(ba->utext[n]);
}

//## method Int Bytes.set(Int n, Int c);
static KMETHOD Bytes_set(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[0].asBytes;
	size_t n = check_index(kctx, sfp[1].intValue, ba->bytesize, sfp[K_RTNIDX].callerFileLine);
	ba->buf[n] = sfp[2].intValue;
	RETURNi_(ba->utext[n]);
}

static KMETHOD Bytes_setAll(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[0].asBytes;
	int bytesize = ba->bytesize;
	int i;
	for (i = 0; i < bytesize; i++) {
		ba->buf[i] = sfp[2].intValue;
	}
	RETURNvoid_();

}
static KMETHOD Bytes_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[0].asBytes;
	RETURNi_(ba->bytesize);
}

static KMETHOD Bytes_new(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(KLIB new_kObject(kctx, OnStack, KReturnType(sfp), sfp[1].intValue));
}

/* ------------------------------------------------------------------------ */

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Im       kMethod_Immutable
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

static kbool_t bytes_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
//	kmodiconv_t *base = (kmodiconv_t*)KCALLOC(sizeof(kmodiconv_t), 1);
//	base->h.name     = "iconv";
//	base->h.setup    = kmodiconv_setup;
//	base->h.reftrace = kmodiconv_reftrace;
//	base->h.free     = kmodiconv_free;
//	KLIB KonohaRuntime_setModule(kctx, MOD_iconv, &base->h, pline);

	KDEFINE_CLASS defBytes = {0};
	SETSTRUCTNAME(defBytes, Bytes);
	defBytes.cflag   = kClass_Final;
	defBytes.free    = kBytes_free;
	defBytes.init    = kBytes_init;
	defBytes.p       = kBytes_p;

	KonohaClass *cBytes = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defBytes, pline);
	int TY_Bytes = cBytes->typeId;
	int FN_encoding = FN_("encoding");
	int FN_x = FN_("x");
	int FN_c = FN_("c");
	int FN_size = FN_("size");
	intptr_t methoddata[] = {
		_Public|_Im|_Coercion, _F(String_toBytes), TY_Bytes,  TY_String, MN_("toBytes"),   0,
		_Public|_Const|_Im|_Coercion, _F(Bytes_toString), TY_String, TY_Bytes,  MN_("toString"),  0,
		_Public|_Const, _F(Bytes_encodeTo),   TY_Bytes,  TY_Bytes,  MN_("encodeTo"),    1, TY_String, FN_encoding,
		_Public|_Const, _F(Bytes_decodeFrom),   TY_String, TY_Bytes,  MN_("decodeFrom"),    1, TY_String, FN_encoding,
		_Public|_Const|_Im, _F(Bytes_get), TY_int, TY_Bytes, MN_("get"), 1, TY_int, FN_x,
		_Public|_Const|_Im, _F(Bytes_set), TY_int, TY_Bytes, MN_("set"), 2, TY_int, FN_x, TY_int, FN_c,
		_Public|_Const|_Im, _F(Bytes_setAll), TY_void, TY_Bytes, MN_("setAll"), 1, TY_int, FN_x,
		_Public|_Const|_Im, _F(Bytes_getSize), TY_int, TY_Bytes, MN_("getSize"), 0,
		_Public|_Const|_Im, _F(Bytes_new), TY_Bytes, TY_Bytes, MN_("new"), 1, TY_int, FN_size,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, NULL, methoddata);
	return true;
}

static kbool_t bytes_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
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
			RETURNi_(pos);
		}
		prev = ch;
	}
	if(IS_NOTNULL(tk)) {
		kreportf(ErrTag, tk->uline, "must close with \'");
	}
	RETURNi_(0);
}

static KMETHOD TypeCheck_SingleQuotedChar(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->termToken;
	kString *s = tk->text;
	DBG_P("string:'%s'", S_text(s));
	if(S_size(s) == 1) {
		int ch = S_text(s)[0];
		RETURN_(SUGAR kExpr_setUnboxConstValue(kctx, expr, TY_int, ch));
	} else {
		SUGAR kStmt_printMessage2(kctx, stmt, (kToken*)expr, ErrTag, "single quote doesn't accept multi characters, '%s'", S_text(s));
	}
	RETURN_(K_NULLEXPR);
}

static kbool_t bytes_initNameSpace(KonohaContext *kctx, kNameSpace *packageNS, kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("$SingleQuotedChar"), 0, NULL, 0, 0, NULL, NULL, NULL, NULL, TypeCheck_SingleQuotedChar, },
		{ KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNS);
	SUGAR kNameSpace_setTokenFunc(kctx, ns, SYM_("$SingleQuotedChar"), KonohaChar_Quote, new_SugarFunc(ns, TokenFunc_SingleQuotedChar));
	return true;
}

static kbool_t bytes_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNS, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* bytes_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSETPACKNAME(d, "bytes", "1.0");
	d.initPackage    = bytes_initPackage;
	d.setupPackage   = bytes_setupPackage;
	d.initNameSpace  = bytes_initNameSpace;
	d.setupNameSpace = bytes_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
