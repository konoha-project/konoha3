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

/* ************************************************************************ */

#define USING_SUGAR_AS_BUILTIN 1
#include<minikonoha/sugar.h>

/* ************************************************************************ */

#ifdef __cplusplus
extern "C" {
#endif

#include<minikonoha/local.h>

// global variable
int verbose_sugar = 0;

#include "perror.h"
#include "sugarclass.h"
#include "namespace.h"
#include "sugardump.h"
#include "token.h"
#include "ast.h"
#include "tycheck.h"
#include "sugarfunc.h"
#include "loader.h"

/* ------------------------------------------------------------------------ */
/* Sugar Global Functions */

static kstatus_t kNameSpace_eval(KonohaContext *kctx, kNameSpace *ns, const char *script, kfileline_t uline)
{
	kstatus_t result;
	kmodsugar->h.setup(kctx, (KonohaModule*)kmodsugar, 0/*lazy*/);
	{
		INIT_GCSTACK();
		kArray *tokenArray = ctxsugar->preparedTokenList;
		size_t popAlreadUsed = kArray_size(tokenArray);
		kNameSpace_tokenize(kctx, ns, script, uline, tokenArray);
		kBlock *bk = new_Block(kctx, ns, NULL, tokenArray, popAlreadUsed, kArray_size(tokenArray), ';');
		KLIB kArray_clear(kctx, tokenArray, popAlreadUsed);
		result = Block_eval(kctx, bk);
		RESET_GCSTACK();
	}
	return result;
}

kstatus_t MODSUGAR_eval(KonohaContext *kctx, const char *script, kfileline_t uline)
{
	if(verbose_sugar) {
		DUMP_P("\n>>>----\n'%s'\n------\n", script);
	}
	kmodsugar->h.setup(kctx, (KonohaModule*)kmodsugar, 0/*lazy*/);
	return kNameSpace_eval(kctx, KNULL(NameSpace), script, uline);    // FIXME
}

/* ------------------------------------------------------------------------ */
/* [ctxsugar] */

static void SugarContext_reftrace(KonohaContext *kctx, struct KonohaContextModule *baseh)
{
	SugarContext *base = (SugarContext*)baseh;
	BEGIN_REFTRACE(7);
	KREFTRACEv(base->preparedTokenList);
	KREFTRACEv(base->errorMessageList);
	KREFTRACEv(base->gma);
	KREFTRACEv(base->lvarlst);
	KREFTRACEv(base->singleBlock);
	KREFTRACEv(base->definedMethodList);
	END_REFTRACE();
}
static void SugarContext_free(KonohaContext *kctx, struct KonohaContextModule *baseh)
{
	SugarContext *base = (SugarContext*)baseh;
	KLIB Karray_free(kctx, &base->errorMessageBuffer);
	KFREE(base, sizeof(SugarContext));
}

static void SugarModule_setup(KonohaContext *kctx, struct KonohaModule *def, int newctx)
{
	if(!newctx && kctx->modlocal[MOD_sugar] == NULL) {
		SugarContext *base = (SugarContext*)KCALLOC(sizeof(SugarContext), 1);
		base->h.reftrace = SugarContext_reftrace;
		base->h.free     = SugarContext_free;
		KINITv(base->preparedTokenList, new_(TokenArray, K_PAGESIZE/sizeof(void*)));
		base->errorMessageCount = 0;
		KINITv(base->errorMessageList, new_(StringArray, 8));
		KINITv(base->lvarlst, new_(ExprArray, K_PAGESIZE/sizeof(void*)));
		KINITv(base->definedMethodList, new_(MethodArray, 8));

		KINITv(base->gma, new_(Gamma, NULL));
		KINITv(base->singleBlock, new_(Block, NULL));
		KLIB kArray_add(kctx, base->singleBlock->stmtList, K_NULL);
		KLIB Karray_init(kctx, &base->errorMessageBuffer, K_PAGESIZE);
		kctx->modlocal[MOD_sugar] = (KonohaContextModule*)base;
	}
}

static void pack_reftrace(KonohaContext *kctx, KUtilsHashMapEntry *p)
{
	KonohaPackage *pack = (KonohaPackage*)p->uvalue;
	BEGIN_REFTRACE(1);
	KREFTRACEn(pack->packageNameSpace);
	END_REFTRACE();
}

static void pack_free(KonohaContext *kctx, void *p)
{
	KFREE(p, sizeof(KonohaPackage));
}

static void SugarModule_reftrace(KonohaContext *kctx, struct KonohaModule *baseh)
{
	KModuleSugar *base = (KModuleSugar*)baseh;
	KLIB Kmap_reftrace(kctx, base->packageMapNO, pack_reftrace);
	BEGIN_REFTRACE(6);
	KREFTRACEv(base->packageList);
	KREFTRACEv(base->UndefinedParseExpr);
	KREFTRACEv(base->UndefinedStmtTyCheck);
	KREFTRACEv(base->UndefinedExprTyCheck);
	KREFTRACEv(base->ParseExpr_Term);
	KREFTRACEv(base->ParseExpr_Op);
	END_REFTRACE();
}

static void SugarModule_free(KonohaContext *kctx, struct KonohaModule *baseh)
{
	KModuleSugar *base = (KModuleSugar*)baseh;
	KLIB Kmap_free(kctx, base->packageMapNO, pack_free);
	KFREE(baseh, sizeof(KModuleSugar));
}

void MODSUGAR_init(KonohaContext *kctx, KonohaContextVar *ctx)
{
	KModuleSugar *base = (KModuleSugar*)KCALLOC(sizeof(KModuleSugar), 1);
	base->h.name     = "sugar";
	base->h.setup    = SugarModule_setup;
	base->h.reftrace = SugarModule_reftrace;
	base->h.free     = SugarModule_free;
	KLIB Konoha_setModule(kctx, MOD_sugar, (KonohaModule*)base, 0);

	KonohaLibVar* l = (KonohaLibVar*)ctx->klib;
	l->kNameSpace_getClass   = kNameSpace_getClass;
	l->kNameSpace_loadMethodData = kNameSpace_loadMethodData;
	l->kNameSpace_loadConstData  = kNameSpace_loadConstData;
	l->kNameSpace_getMethodNULL  = kNameSpace_getMethodNULL;
	l->kNameSpace_compileAllDefinedMethods    = kNameSpace_compileAllDefinedMethods;

	KINITv(base->packageList, new_(Array, 8));
	base->packageMapNO = KLIB Kmap_init(kctx, 0);

	KDEFINE_CLASS defNameSpace = {
		STRUCTNAME(NameSpace),
		.init = NameSpace_init,
		.reftrace = NameSpace_reftrace,
		.free = NameSpace_free,
	};
	KDEFINE_CLASS defToken = {
		STRUCTNAME(Token),
		.init = Token_init,
		.reftrace = Token_reftrace,
	};
	KDEFINE_CLASS defExpr = {
		STRUCTNAME(Expr),
		.init = Expr_init,
		.reftrace = Expr_reftrace,
	};
	KDEFINE_CLASS defStmt = {
		STRUCTNAME(Stmt),
		.init = Stmt_init,
		.reftrace = Stmt_reftrace,
	};
	KDEFINE_CLASS defBlock = {
		STRUCTNAME(Block),
		.init = Block_init,
		.reftrace = Block_reftrace,
	};
	KDEFINE_CLASS defGamma = {
		STRUCTNAME(Gamma),
		.init = Gamma_init,
	};
	base->cNameSpace = KLIB Konoha_defineClass(kctx, PN_sugar, PN_sugar, NULL, &defNameSpace, 0);
	base->cToken = KLIB Konoha_defineClass(kctx, PN_sugar, PN_sugar, NULL, &defToken, 0);
	base->cExpr  = KLIB Konoha_defineClass(kctx, PN_sugar, PN_sugar, NULL, &defExpr, 0);
	base->cStmt  = KLIB Konoha_defineClass(kctx, PN_sugar, PN_sugar, NULL, &defStmt, 0);
	base->cBlock = KLIB Konoha_defineClass(kctx, PN_sugar, PN_sugar, NULL, &defBlock, 0);
	base->cGamma = KLIB Konoha_defineClass(kctx, PN_sugar, PN_sugar, NULL, &defGamma, 0);
	base->cTokenArray = CT_p0(kctx, CT_Array, base->cToken->classId);

	KLIB Knull(kctx, base->cNameSpace);
	KLIB Knull(kctx, base->cToken);
	KLIB Knull(kctx, base->cExpr);
	KLIB Knull(kctx, base->cBlock);
	SugarModule_setup(kctx, &base->h, 0);

	KINITv(base->UndefinedParseExpr,   new_SugarFunc(UndefinedParseExpr));
	KINITv(base->UndefinedStmtTyCheck, new_SugarFunc(UndefinedStmtTyCheck));
	KINITv(base->UndefinedExprTyCheck, new_SugarFunc(UndefinedExprTyCheck));
	KINITv(base->ParseExpr_Op,   new_SugarFunc(ParseExpr_Op));
	KINITv(base->ParseExpr_Term, new_SugarFunc(ParseExpr_Term));

	defineDefaultSyntax(kctx, KNULL(NameSpace));
	DBG_ASSERT(SYM_("$params") == KW_ParamsPattern);
	DBG_ASSERT(SYM_(".") == KW_DOT);
	DBG_ASSERT(SYM_(",") == KW_COMMA);
	DBG_ASSERT(SYM_("void") == KW_void);
	DBG_ASSERT(SYM_("return") == KW_return);
	DBG_ASSERT(SYM_("new") == KW_new);
	EXPORT_SUGAR(base);
}


// boolean NameSpace.importPackage(String pkgname);
static KMETHOD NameSpace_importPackage_(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(NameSpace_importPackage(kctx, sfp[0].asNameSpace, S_text(sfp[1].asString), sfp[K_RTNIDX].uline));
}

// boolean NameSpace.loadScript(String path);
static KMETHOD NameSpace_loadScript_(KonohaContext *kctx, KonohaStack *sfp)
{
	kfileline_t pline = sfp[K_RTNIDX].uline;
	FILE_i *fp = PLATAPI fopen_i(S_text(sfp[1].asString), "r");
	if(fp != NULL) {
		kfileline_t uline = uline_init(kctx, S_text(sfp[1].asString), S_size(sfp[1].asString), 1, 1);
		kstatus_t status = kNameSpace_loadStream(kctx, sfp[0].asNameSpace, fp, uline, 0);
		PLATAPI fclose_i(fp);
		RETURNb_(status == K_CONTINUE);
	}
	else {
		kreportf(ErrTag, pline, "script not found: %s", S_text(sfp[1].asString));
		RETURNb_(0);
	}
}

#define _Public kMethod_Public
#define _Static kMethod_Static
#define _F(F)   (intptr_t)(F)

void MODSUGAR_loadMethod(KonohaContext *kctx)
{
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(NameSpace_importPackage_), TY_Boolean, TY_NameSpace, MN_("import"), 1, TY_String, FN_("name"),
		_Public, _F(NameSpace_loadScript_), TY_Boolean, TY_NameSpace, MN_("load"), 1, TY_String, FN_("path"),
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, NULL, MethodData);
	KSET_KLIB2(KimportPackage, NameSpace_importPackage, 0);
}

#ifdef __cplusplus
}
#endif
