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

#include <stdio.h>
#include <minikonoha/bytes.h>

#include <errno.h> // include this because of E2BIG

/* ------------------------------------------------------------------------ */

// Bytes_init
static void Bytes_init(KonohaContext *kctx, kObject *o, void *conf)
{
	if ((size_t)conf <= 0) return;
	struct _kBytes *ba = (struct _kBytes*)o;
	ba->byteptr = NULL;
	ba->byteptr = (const char *)KCALLOC((size_t)conf, 1);
	if (ba->byteptr != NULL)	ba->bytesize = (size_t)conf;
	else ba->bytesize = 0;
}

static void Bytes_free(KonohaContext *kctx, kObject *o)
{
	struct _kBytes *ba = (struct _kBytes*)o;
	if (ba->byteptr != NULL) {
		KFREE(ba->buf, ba->bytesize);
		ba->byteptr = NULL;
		ba->bytesize = 0;
	}
}

static void Bytes_p(KonohaContext *kctx, KonohaStack *sfp, int pos, KUtilsWriteBuffer *wb, int level)
{
	kBytes *ba = (kBytes*)sfp[pos].o;
	DBG_P("level:%d", level);
	if(level == 0) {
		KLIB Kwb_printf(kctx, wb, "byte[%d]", ba->bytesize);
	}
	else if(level == 1) {
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
}

static void kmodiconv_setup(KonohaContext *kctx, struct KonohaModule *def, int newctx)
{
}

static void kmodiconv_reftrace(KonohaContext *kctx, struct KonohaModule *baseh)
{
}

static void kmodiconv_free(KonohaContext *kctx, struct KonohaModule *baseh)
{
	KFREE(baseh, sizeof(kmodiconv_t));
}

/* ------------------------------------------------------------------------ */

#define CONV_BUFSIZE 4096 // 4K
#define MAX_STORE_BUFSIZE (CONV_BUFSIZE * 1024)// 4M

static kBytes* convFromTo(KonohaContext *kctx, kBytes *fromBa, const char *fromCoding, const char *toCoding)
{
	kiconv_t conv;
	KUtilsWriteBuffer wb;

	char convBuf[CONV_BUFSIZE] = {'\0'};
	char *presentPtrFrom = fromBa->buf;
	char ** inbuf = &presentPtrFrom;
	char *presentPtrTo = convBuf;
	char ** outbuf = &presentPtrTo;
	size_t inBytesLeft, outBytesLeft;
	inBytesLeft = fromBa->bytesize;
	outBytesLeft = CONV_BUFSIZE;
	DBG_P("from='%s' inBytesLeft=%d, to='%s' outBytesLeft=%d", fromCoding, inBytesLeft, toCoding, outBytesLeft);

	if (strncmp(fromCoding, toCoding, strlen(fromCoding)) == 0) {
		// no need to convert.
		return fromBa;
	}
	conv = (kiconv_t)PLATAPI iconv_open_i(toCoding, fromCoding);
	if (conv == (kiconv_t)(-1)) {
		OLDTRACE_SWITCH_TO_KTrace(_UserInputFault,
				LogText("@","iconv_open"),
				LogText("from", fromCoding),
				LogText("to", toCoding)
		);
		return KNULL(Bytes);
	}
	size_t iconv_ret = -1;
	size_t processedSize = 0;
	size_t processedTotalSize = processedSize;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	while (inBytesLeft > 0 && iconv_ret == -1) {
		iconv_ret = PLATAPI iconv_i((uintptr_t)conv, inbuf, &inBytesLeft, outbuf, &outBytesLeft);
		if (iconv_ret == -1 && errno == E2BIG) {
			// input is too big.
			processedSize = CONV_BUFSIZE - outBytesLeft;
			processedTotalSize += processedSize;
			KLIB Kwb_printf(kctx, &wb, "%s", convBuf);
			// reset convbuf
			presentPtrTo = convBuf;
//			outbuf = &presentPtrTo;
			memset(convBuf, '\0', CONV_BUFSIZE);
			outBytesLeft = CONV_BUFSIZE;
		} else if (iconv_ret == -1) {
			OLDTRACE_SWITCH_TO_KTrace(_DataFault,
				LogText("@","iconv"),
				LogText("from", "UTF-8"),
				LogText("to", toCoding),
				LogText("error", strerror(errno))
			);
			KLIB Kwb_free(&wb);
			return (kBytes*)(CT_Bytes->defaultValueAsNull);

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

	const char *KUtilsWriteBufferopChar = KLIB Kwb_top(kctx, &wb, 1);
	struct _kBytes *toBa = (struct _kBytes*)KLIB new_kObject(kctx, CT_Bytes, processedTotalSize); // ensure bytes ends with Zero
	memcpy(toBa->buf, KUtilsWriteBufferopChar, processedTotalSize); // including NUL terminate by ensuredZeo
	KLIB Kwb_free(&wb);
	return toBa;
}

//## @Const method Bytes Bytes.encodeTo(String toEncoding);
static KMETHOD Bytes_encodeTo(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[0].ba;
	kString *toCoding = sfp[1].asString;

	RETURN_(convFromTo(kctx, ba, "UTF-8", S_text(toCoding)));
}

static kString *toString(KonohaContext *kctx, kBytes *ba)
{
	// At this point, we assuem 'ba' is null terminated.
	return KLIB new_kString(kctx, ba->buf, ba->bytesize-1, 0);
}

//## @Const method String Bytes.decodeFrom(String fromEncoding);
static KMETHOD Bytes_decodeFrom(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes* fromBa = sfp[0].ba;
	kString*fromCoding = sfp[1].asString;
	kBytes *toBa = NULL;
	DBG_P("size=%d, '%s'", fromBa->bytesize, fromBa->buf);
	DBG_P("fromCoding:%p, %s", fromCoding, S_text(fromCoding));
	if (fromBa->bytesize == 0) {
		RETURN_(KNULL(String));
	}
	if (fromCoding != (kString*)(CT_String->defaultValueAsNull)) {
		toBa = convFromTo(kctx, fromBa, S_text(fromCoding), "UTF-8");
	} else {
		// conv from default encoding
		toBa = convFromTo(kctx, fromBa, PLATAPI getSystemCharset(), "UTF-8");
	}
	DBG_P("size=%d, '%s'", toBa->bytesize, toBa->buf);
	RETURN_(toString(kctx, toBa));
}

//## @Const method Bytes String.asBytes();
static KMETHOD String_asBytes(KonohaContext *kctx, KonohaStack *sfp)
{
	kString* s = sfp[0].s;
	size_t size = S_size(s);
	kBytes* ba = new_(Bytes, (size>0)?size+1:0);
	if (size > 0) {
		memcpy(ba->buf, S_text(s), size+1); // including NUL char
		DBG_ASSERT(ba->buf[S_size(s)] == '\0');
	}
	RETURN_(ba);
}

// this method is same as Bytes.decodeFrom(defaultencoding);
// this methodList needs string_glue.h for counting mlen...
//#include "../konoha.string/string_glue.h"

//## @Const method String Bytes.asString();
static KMETHOD Bytes_asString(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *from = sfp[0].ba;
	kBytes *to = convFromTo(kctx, from, PLATAPI getSystemCharset(), "UTF-8");

	RETURN_(toString(kctx, to));
}

//## Int Bytes.get(Int n);
static KMETHOD Bytes_get(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[0].ba;
	size_t n = check_index(kctx, sfp[1].intValue, ba->bytesize, sfp[K_RTNIDX].uline);
	RETURNi_(ba->utext[n]);
}

//## method Int Bytes.set(Int n, Int c);
static KMETHOD Bytes_set(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[0].ba;
	size_t n = check_index(kctx, sfp[1].intValue, ba->bytesize, sfp[K_RTNIDX].uline);
	ba->buf[n] = sfp[2].intValue;
	RETURNi_(ba->utext[n]);
}

static KMETHOD Bytes_setAll(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[0].ba;
	int bytesize = ba->bytesize;
	int i;
	for (i = 0; i < bytesize; i++) {
		ba->buf[i] = sfp[2].intValue;
	}
	RETURNvoid_();

}
static KMETHOD Bytes_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kBytes *ba = sfp[0].ba;
	RETURNi_(ba->bytesize);
}

static KMETHOD Bytes_new(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), sfp[1].intValue));
}

/* ------------------------------------------------------------------------ */

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Im       kMethod_Immutable
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

static kbool_t bytes_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	kmodiconv_t *base = (kmodiconv_t*)KCALLOC(sizeof(kmodiconv_t), 1);
	base->h.name     = "iconv";
	base->h.setup    = kmodiconv_setup;
	base->h.reftrace = kmodiconv_reftrace;
	base->h.free     = kmodiconv_free;
	KLIB KonohaRuntime_setModule(kctx, MOD_iconv, &base->h, pline);

	KDEFINE_CLASS defBytes = {
		STRUCTNAME(Bytes),
		.cflag   = kClass_Final,
		.free    = Bytes_free,
		.init    = Bytes_init,
		.p       = Bytes_p,
	};
	base->cBytes = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defBytes, pline);
	int FN_encoding = FN_("encoding");
	int FN_x = FN_("x");
	int FN_c = FN_("c");
	int FN_size = FN_("size");
	intptr_t methoddata[] = {
		_Public|_Im|_Coercion, _F(String_asBytes), TY_Bytes,  TY_String, MN_("asBytes"),   0,
		_Public|_Const|_Im|_Coercion, _F(Bytes_asString), TY_String, TY_Bytes,  MN_("asString"),  0,
		_Public|_Const,     _F(Bytes_encodeTo),   TY_Bytes,  TY_Bytes,  MN_("encodeTo"),    1, TY_String, FN_encoding,
		_Public|_Const,     _F(Bytes_decodeFrom),   TY_String, TY_Bytes,  MN_("decodeFrom"),    1, TY_String, FN_encoding,
		_Public|_Const|_Im,     _F(Bytes_get), TY_int, TY_Bytes, MN_("get"), 1, TY_int, FN_x,
		_Public|_Const|_Im,     _F(Bytes_set), TY_int, TY_Bytes, MN_("set"), 2, TY_int, FN_x, TY_int, FN_c,
		_Public|_Const|_Im,     _F(Bytes_setAll), TY_void, TY_Bytes, MN_("setAll"), 1, TY_int, FN_x,
		_Public|_Const|_Im,     _F(Bytes_getSize), TY_int, TY_Bytes, MN_("getSize"), 0,
		_Public|_Const|_Im,     _F(Bytes_new), TY_Bytes, TY_Bytes, MN_("new"), 1, TY_int, FN_size,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, NULL, methoddata);
	return true;
}

static kbool_t bytes_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}


static int parseSQUOTE(KonohaContext *kctx, kTokenVar *tk, TokenizerEnv *tenv, int tok_start)
{
	int ch, prev = '\'', pos = tok_start + 1;
	while((ch = tenv->source[pos++]) != 0) {
		if(ch == '\n') {
			break;
		}
		if(ch == '\'' && prev != '\\') {
			if(IS_NOTNULL(tk)) {
				KSETv(tk, tk->text, KLIB new_kString(kctx, tenv->source + tok_start + 1, (pos-1)- (tok_start+1), 0));
				tk->unresolvedTokenType = SYM_("$SingleQuote");
			}
			return pos;
		}
		prev = ch;
	}
	if(IS_NOTNULL(tk)) {
		kreportf(ErrTag, tk->uline, "must close with \'");
	}
	return pos-1;
}

static KMETHOD ExprTyCheck_Squote(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->termToken;
	kString *s = tk->text;
	DBG_P("string:'%s'", S_text(s));
	if (S_size(s) == 1) {
		int ch = S_text(s)[0];
		RETURN_(SUGAR kExpr_setUnboxConstValue(kctx, expr, TY_int, ch));
	} else {
		SUGAR kStmt_printMessage2(kctx, stmt, (kToken*)expr, ErrTag, "single quote doesn't accept multi characters, '%s'", S_text(s));
	}
	RETURN_(K_NULLEXPR);
}

static kbool_t bytes_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	SUGAR kNameSpace_setTokenizeFunc(kctx, ns, '\'', parseSQUOTE, NULL, 0);
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("$SingleQuote"),  ExprTyCheck_(Squote)},
		{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNameSpace);
	return true;
}

static kbool_t bytes_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* bytes_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("bytes", "1.0"),
		.initPackage = bytes_initPackage,
		.setupPackage = bytes_setupPackage,
		.initNameSpace = bytes_initNameSpace,
		.setupNameSpace = bytes_setupNameSpace,
	};
	return &d;
}
