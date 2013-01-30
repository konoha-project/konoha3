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

#include <unistd.h> /* for off64_t @ CentOS on 32bit */
#if defined( __i386__) &&  defined( __linux__) && !defined(__off64_t_defined)
typedef unsigned long long int off64_t;
#endif

#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "http_log.h"
#include "ap_config.h"
#include "apr_strings.h"
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_VERSION
#include "apache_glue.h"

#ifdef __cplusplus
extern "C"{
#endif


static void Request_Init(KonohaContext *kctx, kObject *po, void *conf)
{
	(void)kctx;
	((kRequest *)po)->r = (request_rec *) conf;
}

static void Request_Free(KonohaContext *kctx, kObject *po)
{
	(void)kctx;
((kRequest *)po)->r = NULL;
}

static void AprTable_Init(KonohaContext *kctx, kObject *po, void *conf)
{
	(void)kctx;
	((kAprTable *)po)->tbl = (apr_table_t *) conf;
}

static void AprTable_Free(KonohaContext *kctx, kObject *po)
{
	(void)kctx;
	((kAprTable *)po)->tbl = NULL;
}

static void AprTableEntry_Init(KonohaContext *kctx, kObject *po, void *conf)
{
	(void)kctx;
	((kAprTableEntry *)po)->entry = (apr_table_entry_t *) conf;
}

static void AprTableEntry_Free(KonohaContext *kctx, kObject *po)
{
	(void)kctx;
	((kAprTableEntry *)po)->entry = NULL;
}

static void ApacheModule_Setup(KonohaContext *kctx, struct KRuntimeModule *def, int newctx) {}
static void ApacheModule_Free(KonohaContext *kctx, struct KRuntimeModule *baseh)
{
	KFree(baseh, sizeof(KModuleApache));
}


static kbool_t apache_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	static KDEFINE_CLASS Def = {
		STRUCTNAME(Request),
		.init = Request_Init,
		.free = Request_Free,
	};

	static KDEFINE_CLASS aprTableDef = {
		STRUCTNAME(AprTable),
		.init = AprTable_Init,
		.free = AprTable_Free,
	};

	static KDEFINE_CLASS aprTableEntryDef = {
		STRUCTNAME(AprTableEntry),
		.init = AprTableEntry_Init,
		.free = AprTableEntry_Free,
	};

	static KDEFINE_CLASS apacheDef = {
		STRUCTNAME(Apache),
	};

	KModuleApache *mod = (KModuleApache *)KCalloc_UNTRACE(sizeof(KModuleApache), 1);
	mod->h.name               = "apache";
	mod->h.allocSize          = sizeof(KModuleApache);
	mod->h.setupModuleContext = ApacheModule_Setup;
	mod->h.freeModule         = ApacheModule_Free;
	KLIB KRuntime_SetModule(kctx, MOD_APACHE, (KRuntimeModule *)mod, trace);
	mod->cRequest       = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &Def, 0);
	mod->cAprTable      = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &aprTableDef, 0);
	mod->cAprTableEntry = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &aprTableEntryDef, 0);
	mod->cApache        = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &apacheDef, 0);

	KDEFINE_INT_CONST IntData[] = {
#define DEFINE_KEYWORD(KW) {#KW, KType_Int, KW}
		DEFINE_KEYWORD(OK),
		DEFINE_KEYWORD(APLOG_EMERG),
		DEFINE_KEYWORD(APLOG_ALERT),
		DEFINE_KEYWORD(APLOG_CRIT),
		DEFINE_KEYWORD(APLOG_ERR),
		DEFINE_KEYWORD(APLOG_WARNING),
		DEFINE_KEYWORD(APLOG_NOTICE),
		DEFINE_KEYWORD(APLOG_INFO),
		DEFINE_KEYWORD(APLOG_DEBUG),
		{NULL, 0, 0}
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(IntData), trace);
	return true;
}

static kbool_t apache_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *Apache_Init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("apache", "1.0"),
		.PackupNameSpace    = apache_PackupNameSpace,
		.ExportNameSpace   = apache_ExportNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
