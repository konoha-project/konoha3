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

static kbool_t konoha_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KRequirePackage("konoha.namespace", pline);
	KRequirePackage("konoha.const", pline);
	KRequirePackage("konoha.global", pline);
	KRequirePackage("konoha.class", pline);       // subtype

	KRequirePackage("konoha.null", pline);

	KRequirePackage("konoha.while",  pline);  // continue, break

	KRequirePackage("konoha.var", pline);

	KRequirePackage("konoha.object", pline);  // subtype
	KRequirePackage("konoha.int", pline);
#ifndef K_USING_NOFLOAT
	KRequirePackage("konoha.float", pline);
#endif
	KRequirePackage("konoha.string", pline);
	KRequirePackage("konoha.array", pline);
	KRequirePackage("konoha.map", pline);
	KRequirePackage("konoha.iterator", pline);


	KRequirePackage("konoha.assign", pline);
	KRequirePackage("konoha.io", pline);


	return true;
}

static kbool_t konoha_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t konoha_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	KImportPackage(ns, "konoha.namespace", pline);
	KImportPackage(ns, "konoha.const",  pline);  // TopLevel
	KImportPackage(ns, "konoha.global", pline);
	KImportPackage(ns, "konoha.class",  pline);

	KImportPackage(ns, "konoha.null", pline);    // Operator

	KImportPackage(ns, "konoha.while",  pline);

	KImportPackage(ns, "konoha.var",  pline);

	KImportPackage(ns, "konoha.object", pline);  // subtype
	KImportPackage(ns, "konoha.int",  pline);
	KImportPackage(ns, "konoha.float", pline);
	KImportPackage(ns, "konoha.string", pline);

	KImportPackage(ns, "konoha.date", pline);
	KImportPackage(ns, "konoha.array", pline);
	KImportPackage(ns, "konoha.map", pline);
	KImportPackage(ns, "konoha.iterator", pline);

	KImportPackage(ns, "konoha.assign", pline);

	KImportPackage(ns, "konoha.io", pline);

	return true;
}

static kbool_t konoha_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* konoha_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSETPACKNAME(d, "konoha", "1.0");
	d.initPackage    = konoha_initPackage;
	d.setupPackage   = konoha_setupPackage;
	d.initNameSpace  = konoha_initNameSpace;
	d.setupNameSpace = konoha_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
} /* extern "C" { */
#endif
