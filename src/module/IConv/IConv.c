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

#ifdef __cplusplus
extern "C" {
#endif

#include <iconv.h>
#include <errno.h>
#include <minikonoha/minikonoha.h>

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

static size_t I18N_iconv_memcpyStyle(KonohaContext *kctx, uintptr_t ic, char **outbuf, size_t *outBytesLeft, ICONV_INBUF_CONST char **inbuf, size_t *inBytesLeft, int *isTooBigSourceRef, KTraceInfo *trace)
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
	const char *t = PLATAPI systemCharset;
	return (t[0] == 'U' && t[5] == 0 && t[4] == '8' && t[3] == '-' && t[2] == 'F'); // "UTF-8"
}

static uintptr_t I18N_iconvSystemCharsetToUTF8(KonohaContext *kctx, KTraceInfo *trace)
{
	return PLATAPI iconv_open_i(kctx, "UTF-8", PLATAPI systemCharset, trace);
}

static uintptr_t I18N_iconvUTF8ToSystemCharset(KonohaContext *kctx, KTraceInfo *trace)
{
	return PLATAPI iconv_open_i(kctx, PLATAPI systemCharset, "UTF-8", trace);
}

static const char* I18N_formatKonohaPath(KonohaContext *kctx, char *buf, size_t bufsiz, const char *path, size_t pathsize, KTraceInfo *trace)
{
	size_t newsize;
	if(!PLATAPI isSystemCharsetUTF8(kctx)) {
		uintptr_t ic = PLATAPI iconvUTF8ToSystemCharset(kctx, trace);
		int isTooBig;
		ICONV_INBUF_CONST char *presentPtrFrom = (ICONV_INBUF_CONST char *)path;	// too dirty?
		ICONV_INBUF_CONST char ** inbuf = &presentPtrFrom;
		char ** outbuf = &buf;
		size_t inBytesLeft = pathsize, outBytesLeft = bufsiz - 1;
		PLATAPI iconv_i_memcpyStyle(kctx, ic, outbuf, &outBytesLeft, inbuf, &inBytesLeft, &isTooBig, trace);
		newsize = (bufsiz - 1) - outBytesLeft;
	}
	else {
		DBG_ASSERT(bufsiz > pathsize);
		memcpy(buf, path, pathsize);
		newsize = pathsize;
	}
	buf[newsize] = 0;
	return (const char *)buf;  // stub (in case of no conversion)
}

static const char* I18N_formatSystemPath(KonohaContext *kctx, char *buf, size_t bufsiz, const char *path, size_t pathsize, KTraceInfo *trace)
{
	size_t newsize;
	if(!PLATAPI isSystemCharsetUTF8(kctx)) {
		//DBG_P(">>>>>>>>>>>>>>>>>>>> path = '%s', pathsize=%d", path, pathsize);
		uintptr_t ic = PLATAPI iconvSystemCharsetToUTF8(kctx, trace);
		int isTooBig;
		ICONV_INBUF_CONST char *presentPtrFrom = (ICONV_INBUF_CONST char *)path;	// too dirty?
		ICONV_INBUF_CONST char ** inbuf = &presentPtrFrom;
		char ** outbuf = &buf;
		size_t inBytesLeft = pathsize, outBytesLeft = bufsiz - 1;
		PLATAPI iconv_i_memcpyStyle(kctx, ic, outbuf, &outBytesLeft, inbuf, &inBytesLeft, &isTooBig, trace);
		newsize = (bufsiz - 1) - outBytesLeft;
		//DBG_P(">>>>>>>>>>>>>>>>>>>> buf = '%s', newsize=%d", buf, newsize);
	}
	else {
		DBG_ASSERT(bufsiz > pathsize);
		memcpy(buf, path, pathsize);
		newsize = pathsize;
	}
	buf[newsize] = 0;
	return (const char *)buf;  // stub (in case of no conversion)
}

// -------------------------------------------------------------------------

kbool_t LoadIConvModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"IConv", "0.1", 0, "iconv",
	};
	factory->I18NInfo            = &ModuleInfo;
	factory->systemCharset            = "UTF-8";
	factory->iconv_open_i             = I18N_iconv_open;
	factory->iconv_i                  = I18N_iconv;
	factory->iconv_i_memcpyStyle      = I18N_iconv_memcpyStyle;
	factory->iconv_close_i            = I18N_iconv_close;
	factory->isSystemCharsetUTF8      = I18N_isSystemCharsetUTF8;
	factory->iconvSystemCharsetToUTF8 = I18N_iconvSystemCharsetToUTF8;
	factory->iconvUTF8ToSystemCharset = I18N_iconvUTF8ToSystemCharset;
	factory->formatKonohaPath         = I18N_formatKonohaPath;
	factory->formatSystemPath         = I18N_formatSystemPath;
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

