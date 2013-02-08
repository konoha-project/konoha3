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

#ifdef __cplusplus
extern "C" {
#endif

#include <iconv.h>
#include <errno.h>
#include <konoha3/konoha.h>
#define I18NAPI PLATAPI I18NModule.
// -------------------------------------------------------------------------
/* I18N */

static uintptr_t I18N_iconv_open(KonohaContext *kctx, const char *targetCharset, const char *sourceCharset, KTraceInfo *trace)
{
	uintptr_t ic = (uintptr_t)iconv_open(targetCharset, sourceCharset);
	if(ic == ICONV_NULL) {
		KTraceApi(trace, UserFault|SoftwareFault, "iconv_open",
				  LogText("tocode", targetCharset), LogText("fromcode", sourceCharset), LogErrno
		);
	}
	return (uintptr_t)ic;
}

static size_t I18N_iconv(KonohaContext *kctx, uintptr_t ic, ICONV_INBUF_CONST char **inbuf, size_t *inBytesLeft, char **outbuf, size_t *outBytesLeft, int *isTooBigSourceRef, KTraceInfo *trace)
{
	DBG_ASSERT(ic != ICONV_NULL);
	size_t iconv_ret = iconv((iconv_t)ic, inbuf, inBytesLeft, outbuf, outBytesLeft);
	if(iconv_ret == ((size_t)-1)) {
		if(errno == E2BIG) {   // input is too big.
			isTooBigSourceRef[0] = true;
			return iconv_ret;
		}
		KTraceApi(trace, UserFault, "iconv", LogErrno);
	}
	isTooBigSourceRef[0] = false;
	return iconv_ret;
}

static int I18N_iconv_close(KonohaContext *kctx, uintptr_t ic)
{
	return iconv_close((iconv_t)ic);
}

static kbool_t I18N_isSystemCharsetUTF8(KonohaContext *kctx)
{
	const char *t = I18NAPI systemCharset;
	return (t[0] == 'U' && t[5] == 0 && t[4] == '8' && t[3] == '-' && t[2] == 'F'); // "UTF-8"
}

static uintptr_t I18N_iconvSystemCharsetToUTF8(KonohaContext *kctx, KTraceInfo *trace)
{
	return I18NAPI iconv_open_i(kctx, "UTF-8", I18NAPI systemCharset, trace);
}

static uintptr_t I18N_iconvUTF8ToSystemCharset(KonohaContext *kctx, KTraceInfo *trace)
{
	return I18NAPI iconv_open_i(kctx, I18NAPI systemCharset, "UTF-8", trace);
}

static const char *I18N_formatKonohaPath(KonohaContext *kctx, char *buf, size_t bufsiz, const char *path, size_t pathsize, KTraceInfo *trace)
{
	size_t newsize;
	if(!I18NAPI isSystemCharsetUTF8(kctx)) {
		uintptr_t ic = I18NAPI iconvUTF8ToSystemCharset(kctx, trace);
		int isTooBig;
		ICONV_INBUF_CONST char *presentPtrFrom = (ICONV_INBUF_CONST char *)path;	// too dirty?
		ICONV_INBUF_CONST char **inbuf = &presentPtrFrom;
		char **outbuf = &buf;
		size_t inBytesLeft = pathsize, outBytesLeft = bufsiz - 1;
		I18NAPI iconv_i(kctx, ic, inbuf, &inBytesLeft, outbuf, &outBytesLeft, &isTooBig, trace);
		newsize = (bufsiz - 1) - outBytesLeft;
	}
	else {
		DBG_ASSERT(bufsiz > pathsize);
		memcpy(buf, path, pathsize);
		newsize = pathsize;
	}
	buf[newsize] = 0;
	return (const char *)buf;
}

static const char *I18N_formatSystemPath(KonohaContext *kctx, char *buf, size_t bufsiz, const char *path, size_t pathsize, KTraceInfo *trace)
{
	size_t newsize;
	if(!I18NAPI isSystemCharsetUTF8(kctx)) {
		uintptr_t ic = I18NAPI iconvSystemCharsetToUTF8(kctx, trace);
		int isTooBig;
		ICONV_INBUF_CONST char *presentPtrFrom = (ICONV_INBUF_CONST char *)path;	// too dirty?
		ICONV_INBUF_CONST char **inbuf = &presentPtrFrom;
		char **outbuf = &buf;
		size_t inBytesLeft = pathsize, outBytesLeft = bufsiz - 1;
		I18NAPI iconv_i(kctx, ic, inbuf, &inBytesLeft, outbuf, &outBytesLeft, &isTooBig, trace);
		newsize = (bufsiz - 1) - outBytesLeft;
	}
	else {
		DBG_ASSERT(bufsiz > pathsize);
		memcpy(buf, path, pathsize);
		newsize = pathsize;
	}
	buf[newsize] = 0;
	return (const char *)buf;
}

// -------------------------------------------------------------------------

kbool_t LoadIConvModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"IConv", "0.1", 0, "iconv",
	};
	factory->I18NModule.I18NInfo            = &ModuleInfo;
	factory->I18NModule.systemCharset            = "UTF-8";
	factory->I18NModule.iconv_open_i             = I18N_iconv_open;
	factory->I18NModule.iconv_i                  = I18N_iconv;
	factory->I18NModule.iconv_close_i            = I18N_iconv_close;
	factory->I18NModule.isSystemCharsetUTF8      = I18N_isSystemCharsetUTF8;
	factory->I18NModule.iconvSystemCharsetToUTF8 = I18N_iconvSystemCharsetToUTF8;
	factory->I18NModule.iconvUTF8ToSystemCharset = I18N_iconvUTF8ToSystemCharset;
	factory->I18NModule.formatKonohaPath         = I18N_formatKonohaPath;
	factory->I18NModule.formatSystemPath         = I18N_formatSystemPath;
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
