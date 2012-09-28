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


static void Request_init(KonohaContext *kctx, kObject *po, void *conf)
{
	(void)kctx;
	((kRequest*)po)->r = (request_rec *) conf;
}

static void Request_free(KonohaContext *kctx, kObject *po)
{
	(void)kctx;
((kRequest*)po)->r = NULL;
}

static void AprTable_init(KonohaContext *kctx, kObject *po, void *conf)
{
	(void)kctx;
	((kAprTable*)po)->tbl = (apr_table_t *) conf;
}

static void AprTable_free(KonohaContext *kctx, kObject *po)
{
	(void)kctx;
	((kAprTable*)po)->tbl = NULL;
}

static void AprTableEntry_init(KonohaContext *kctx, kObject *po, void *conf)
{
	(void)kctx;
	((kAprTableEntry*)po)->entry = (apr_table_entry_t *) conf;
}

static void AprTableEntry_free(KonohaContext *kctx, kObject *po)
{
	(void)kctx;
	((kAprTableEntry*)po)->entry = NULL;
}

static void kapacheshare_setup(KonohaContext *kctx, struct KonohaModule *def, int newctx) {}
static void kapacheshare_reftrace(KonohaContext *kctx, struct KonohaModule *baseh) {}
static void kapacheshare_free(KonohaContext *kctx, struct KonohaModule *baseh)
{
	KFREE(baseh, sizeof(kapacheshare_t));
}


static kbool_t apache_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	static KDEFINE_CLASS Def = {
		STRUCTNAME(Request),
		.init = Request_init,
		.free = Request_free,
	};

	static KDEFINE_CLASS aprTableDef = {
		STRUCTNAME(AprTable),
		.init = AprTable_init,
		.free = AprTable_free,
	};

	static KDEFINE_CLASS aprTableEntryDef = {
		STRUCTNAME(AprTableEntry),
		.init = AprTableEntry_init,
		.free = AprTableEntry_free,
	};

	static KDEFINE_CLASS apacheDef = {
		STRUCTNAME(Apache),
	};

	kapacheshare_t *base = (kapacheshare_t*)KCALLOC(sizeof(kapacheshare_t), 1);
	base->h.name     = "apache";
	base->h.setup    = kapacheshare_setup;
	base->h.reftrace = kapacheshare_reftrace;
	base->h.free     = kapacheshare_free;
	KLIB KonohaRuntime_setModule(kctx, MOD_APACHE, &base->h, pline);
	base->cRequest = KLIB kNameSpace_defineClass(kctx, ns, NULL, &Def, 0);
	base->cAprTable = KLIB kNameSpace_defineClass(kctx, ns, NULL, &aprTableDef, 0);
	base->cAprTableEntry = KLIB kNameSpace_defineClass(kctx, ns, NULL, &aprTableEntryDef, 0);
	base->cApache = KLIB kNameSpace_defineClass(kctx, ns, NULL, &apacheDef, 0);

	KDEFINE_INT_CONST IntData[] = {
#define DEFINE_KEYWORD(KW) {#KW, TY_int, KW}
		{"APACHE_OK", TY_int, OK},
		{"APLOG_EMERG", TY_int, APLOG_EMERG},
		{"APLOG_ALERT", TY_int, APLOG_ALERT},
		{"APLOG_CRIT", TY_int, APLOG_CRIT},
		{"APLOG_ERR", TY_int, APLOG_ERR},
		{"APLOG_WARNING", TY_int, APLOG_WARNING},
		{"APLOG_NOTICE", TY_int, APLOG_NOTICE},
		{"APLOG_INFO", TY_int, APLOG_INFO},
		{"APLOG_DEBUG", TY_int, APLOG_DEBUG},
		{NULL, 0, 0}
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(IntData), 0);
	return true;
}

static kbool_t apache_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t apache_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t apache_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* apache_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("apache", "1.0"),
		.initPackage = apache_initPackage,
		.setupPackage = apache_setupPackage,
		.initNameSpace = apache_initNameSpace,
		.setupNameSpace = apache_setupNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif

