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
/* Console */


// -------------------------------------------------------------------------

kbool_t LoadDScriptConsoleModule(KonohaFactory *factory, ModuleType type)
{
//	factory->Module_I18N              = "IConv";
//	factory->systemCharset            = "UTF-8";
//	factory->iconv_open_i             = I18N_iconv_open;
//	factory->iconv_i                  = I18N_iconv;
//	factory->iconv_i_memcpyStyle      = I18N_iconv_memcpyStyle;
//	factory->iconv_close_i            = I18N_iconv_close;
//	factory->isSystemCharsetUTF8      = I18N_isSystemCharsetUTF8;
//	factory->iconvSystemCharsetToUTF8 = I18N_iconvSystemCharsetToUTF8;
//	factory->iconvUTF8ToSystemCharset = I18N_iconvUTF8ToSystemCharset;
//	factory->formatKonohaPath         = I18N_formatKonohaPath;
//	factory->formatSystemPath         = I18N_formatSystemPath;

	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

