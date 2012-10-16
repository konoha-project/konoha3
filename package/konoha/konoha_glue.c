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

static kbool_t konoha_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	KRequirePackage("konoha.namespace", trace);
	KRequirePackage("konoha.const", trace);
	KRequirePackage("konoha.global", trace);
	KRequirePackage("konoha.class", trace);       // subtype

	KRequirePackage("konoha.null", trace);

	KRequirePackage("konoha.while",  trace);  // continue, break

	KRequirePackage("konoha.var", trace);

	KRequirePackage("konoha.object", trace);  // subtype
	KRequirePackage("konoha.int", trace);
#ifndef K_USING_NOFLOAT
	KRequirePackage("konoha.float", trace);
#endif
	KRequirePackage("konoha.string", trace);
	KRequirePackage("konoha.array", trace);
	KRequirePackage("konoha.map", trace);
	KRequirePackage("konoha.iterator", trace);


	KRequirePackage("konoha.assign", trace);
	KRequirePackage("konoha.io", trace);


	return true;
}

static kbool_t konoha_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	return true;
}

static kbool_t konoha_initNameSpace(KonohaContext *kctx, kNameSpace *packageNS, kNameSpace *ns, KTraceInfo *trace)
{
	KImportPackage(ns, "konoha.namespace", trace);
	KImportPackage(ns, "konoha.const",  trace);  // TopLevel
	KImportPackage(ns, "konoha.global", trace);
	KImportPackage(ns, "konoha.class",  trace);

	KImportPackage(ns, "konoha.null", trace);    // Operator

	KImportPackage(ns, "konoha.while",  trace);

	KImportPackage(ns, "konoha.var",  trace);

	KImportPackage(ns, "konoha.object", trace);  // subtype
	KImportPackage(ns, "konoha.int",  trace);
	KImportPackage(ns, "konoha.float", trace);
	KImportPackage(ns, "konoha.string", trace);

	KImportPackage(ns, "konoha.date", trace);
	KImportPackage(ns, "konoha.array", trace);
	KImportPackage(ns, "konoha.map", trace);
	KImportPackage(ns, "konoha.iterator", trace);

	KImportPackage(ns, "konoha.assign", trace);

	KImportPackage(ns, "konoha.io", trace);

	return true;
}

static kbool_t konoha_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNS, kNameSpace *ns, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* konoha_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", "1.0");
	d.initPackage    = konoha_initPackage;
	d.setupPackage   = konoha_setupPackage;
	d.initNameSpace  = konoha_initNameSpace;
	d.setupNameSpace = konoha_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
} /* extern "C" { */
#endif
