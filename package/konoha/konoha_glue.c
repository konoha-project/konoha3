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

// --------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

static kbool_t konoha_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KImportPackage(ns, "cstyle",  trace);  // continue, break

	KImportPackage(ns, "konoha.namespace", trace);
	KImportPackage(ns, "konoha.const", trace);
	KImportPackage(ns, "konoha.global", trace);
	KImportPackage(ns, "konoha.class", trace);       // subtype

	KImportPackage(ns, "konoha.null", trace);

	KImportPackage(ns, "konoha.var", trace);

	KImportPackage(ns, "konoha.object", trace);  // subtype
	KImportPackage(ns, "konoha.int", trace);
#ifndef K_USING_NOFLOAT
	KImportPackage(ns, "konoha.float", trace);
#endif
	KImportPackage(ns, "konoha.string", trace);
	KImportPackage(ns, "konoha.array", trace);
	KImportPackage(ns, "konoha.map", trace);
	KImportPackage(ns, "konoha.iterator", trace);

	KImportPackage(ns, "konoha.assign", trace);
	KImportPackage(ns, "konoha.io", trace);

	return true;
}

static kbool_t konoha_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* konoha_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", "1.0");
	d.PackupNameSpace    = konoha_PackupNameSpace;
	d.ExportNameSpace   = konoha_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
} /* extern "C" { */
#endif
