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
	INIT_GCSTACK();
	{
		TokenRange rangeBuf, *range = new_TokenListRange(kctx, ns, KonohaContext_getSugarContext(kctx)->preparedTokenList, &rangeBuf);
		TokenRange_tokenize(kctx, range, script, uline);
		result = TokenRange_eval(kctx, range);
		TokenRange_pop(kctx, range);
	}
	RESET_GCSTACK();
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
/* [KonohaContext_getSugarContext(kctx)] */

static void SugarContext_reftrace(KonohaContext *kctx, struct KonohaModuleContext *baseh)
{
	SugarContext *base = (SugarContext*)baseh;
	BEGIN_REFTRACE(4);
	KREFTRACEv(base->preparedTokenList);
	KREFTRACEv(base->errorMessageList);
	KREFTRACEv(base->preparedGamma);
	KREFTRACEv(base->definedMethodList);
	END_REFTRACE();
}
static void SugarContext_free(KonohaContext *kctx, struct KonohaModuleContext *baseh)
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
		KINITv(base->definedMethodList, new_(MethodArray, 8));
		KINITv(base->preparedGamma, new_(Gamma, NULL));
		KLIB Karray_init(kctx, &base->errorMessageBuffer, K_PAGESIZE);
		kctx->modlocal[MOD_sugar] = (KonohaModuleContext*)base;
	}
}

static void packageMap_reftrace(KonohaContext *kctx, KUtilsHashMapEntry *p, void *thunk)
{
	KonohaPackage *pack = (KonohaPackage*)p->unboxValue;
	BEGIN_REFTRACE(1);
	KREFTRACEn(pack->packageNameSpace);
	END_REFTRACE();
}

static void packageMap_free(KonohaContext *kctx, void *p)
{
	KFREE(p, sizeof(KonohaPackage));
}

static void SugarModule_reftrace(KonohaContext *kctx, struct KonohaModule *baseh)
{
	KModuleSugar *base = (KModuleSugar*)baseh;
	KLIB Kmap_each(kctx, base->packageMapNO, NULL, packageMap_reftrace);
	BEGIN_REFTRACE(1);
	KREFTRACEv(base->packageList);
	END_REFTRACE();
}

static void SugarModule_free(KonohaContext *kctx, struct KonohaModule *baseh)
{
	KModuleSugar *base = (KModuleSugar*)baseh;
	KLIB Kmap_free(kctx, base->packageMapNO, packageMap_free);
	KFREE(baseh, sizeof(KModuleSugar));
}

void MODSUGAR_init(KonohaContext *kctx, KonohaContextVar *ctx)
{
	KModuleSugar *mod = (KModuleSugar*)KCALLOC(sizeof(KModuleSugar), 1);
	mod->h.name     = "sugar";
	mod->h.setup    = SugarModule_setup;
	mod->h.reftrace = SugarModule_reftrace;
	mod->h.free     = SugarModule_free;
	KLIB Konoha_setModule(kctx, MOD_sugar, (KonohaModule*)mod, 0);

	KonohaLibVar* l = (KonohaLibVar*)ctx->klib;
	l->kNameSpace_getClass   = kNameSpace_getClass;
	l->kNameSpace_loadMethodData = kNameSpace_loadMethodData;
	l->kNameSpace_setConstData   = kNameSpace_setConstData;
	l->kNameSpace_loadConstData  = kNameSpace_loadConstData;
	l->kNameSpace_getMethodNULL  = kNameSpace_getMethodNULL;
	l->kNameSpace_compileAllDefinedMethods    = kNameSpace_compileAllDefinedMethods;

	KINITv(mod->packageList, new_(Array, 8));
	mod->packageMapNO = KLIB Kmap_init(kctx, 0);
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
		.init = kBlock_init,
		.reftrace = kBlock_reftrace,
	};
	KDEFINE_CLASS defGamma = {
		STRUCTNAME(Gamma),
		.init = Gamma_init,
	};
	mod->cNameSpace = KLIB Konoha_defineClass(kctx, PackageId_sugar, PackageId_sugar, NULL, &defNameSpace, 0);
	mod->cToken =     KLIB Konoha_defineClass(kctx, PackageId_sugar, PackageId_sugar, NULL, &defToken, 0);
	mod->cExpr  =     KLIB Konoha_defineClass(kctx, PackageId_sugar, PackageId_sugar, NULL, &defExpr, 0);
	mod->cStmt  =     KLIB Konoha_defineClass(kctx, PackageId_sugar, PackageId_sugar, NULL, &defStmt, 0);
	mod->cBlock =     KLIB Konoha_defineClass(kctx, PackageId_sugar, PackageId_sugar, NULL, &defBlock, 0);
	mod->cGamma =     KLIB Konoha_defineClass(kctx, PackageId_sugar, PackageId_sugar, NULL, &defGamma, 0);
	mod->cTokenArray = CT_p0(kctx, CT_Array, mod->cToken->typeId);

	KLIB Knull(kctx, mod->cNameSpace);
	KLIB Knull(kctx, mod->cToken);
	KLIB Knull(kctx, mod->cExpr);
	KLIB Knull(kctx, mod->cBlock);
	SugarModule_setup(kctx, &mod->h, 0);

//	KINITv(mod->UndefinedParseExpr,   new_SugarFunc(UndefinedParseExpr));
//	KINITv(mod->UndefinedStmtTyCheck, new_SugarFunc(UndefinedStmtTyCheck));
//	KINITv(mod->UndefinedExprTyCheck, new_SugarFunc(UndefinedExprTyCheck));
//	KINITv(mod->ParseExpr_Op,         new_SugarFunc(ParseExpr_Op));
//	KINITv(mod->ParseExpr_Term,       new_SugarFunc(ParseExpr_Term));

	mod->new_TokenListRange         = new_TokenListRange;
	mod->new_TokenStackRange        = new_TokenStackRange;
	mod->kNameSpace_setTokenizeFunc = kNameSpace_setTokenizeFunc;
	mod->TokenRange_tokenize        = TokenRange_tokenize;
	mod->TokenRange_eval            = TokenRange_eval;
	mod->TokenRange_resolved        = TokenRange_resolved;
	mod->kStmt_parseTypePattern     = kStmt_parseTypePattern;
	mod->kStmt_parseFlag            = kStmt_parseFlag;
	mod->kStmt_getToken             = kStmt_getToken;
	mod->kStmt_getBlock             = kStmt_getBlock;
	mod->kStmt_getExpr              = kStmt_getExpr;
	mod->kStmt_getText              = kStmt_getText;
	mod->kExpr_setConstValue        = kExpr_setConstValue;
	mod->kExpr_setUnboxConstValue   = kExpr_setUnboxConstValue;
	mod->kExpr_setVariable          = kExpr_setVariable;
	mod->kStmt_tyCheckExprAt        = kStmt_tyCheckExprAt;
	mod->kStmt_tyCheckByName        = kStmt_tyCheckByName;
	mod->kBlock_tyCheckAll          = kBlock_tyCheckAll;
	mod->kStmt_tyCheckCallParamExpr = kStmt_tyCheckCallParamExpr;
	mod->new_TypedCallExpr          = new_TypedCallExpr;
	mod->kStmt_declType             = kStmt_declType;
	mod->kNameSpace_defineSyntax    = kNameSpace_defineSyntax;
	mod->kNameSpace_getSyntax       = kNameSpace_getSyntax;
	mod->kArray_addSyntaxRule       = kArray_addSyntaxRule;
	mod->kNameSpace_setSugarFunc    = kNameSpace_setSugarFunc;
	mod->kNameSpace_addSugarFunc    = kNameSpace_addSugarFunc;
	mod->new_kBlock                  = new_kBlock;
	mod->new_kStmt                  = new_kStmt;
	mod->kBlock_insertAfter         = kBlock_insertAfter;
	mod->new_UntypedTermExpr        = new_UntypedTermExpr;
	mod->new_UntypedCallStyleExpr   = new_UntypedCallStyleExpr;
	mod->kStmt_parseOperatorExpr    = kStmt_parseOperatorExpr;
	mod->kStmt_parseExpr            = kStmt_parseExpr;
	mod->kStmt_addExprParam         = kStmt_addExprParam;
	mod->kStmt_rightJoinExpr        = kStmt_rightJoinExpr;
	mod->Token_pERR                 = Token_pERR;
	mod->kStmt_printMessage                     = kStmt_printMessage;

	defineDefaultSyntax(kctx, KNULL(NameSpace));
}

// boolean NameSpace.loadScript(String path);
static KMETHOD NameSpace_loadScript(KonohaContext *kctx, KonohaStack *sfp)
{
	char pathbuf[256];
	const char *path = PLATAPI formatTransparentPath(pathbuf, sizeof(pathbuf), FileId_t(sfp[K_RTNIDX].uline), S_text(sfp[1].asString));
	RETURNb_(kNameSpace_loadScript(kctx, sfp[0].asNameSpace, path, sfp[K_RTNIDX].uline));
}

// boolean NameSpace.importPackage(String pkgname);
static KMETHOD NameSpace_importPackage(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(kNameSpace_importPackage(kctx, sfp[0].asNameSpace, S_text(sfp[1].asString), sfp[K_RTNIDX].uline));
}

#define _Public kMethod_Public
#define _Static kMethod_Static
#define _F(F)   (intptr_t)(F)

void MODSUGAR_loadMethod(KonohaContext *kctx)
{
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(NameSpace_importPackage), TY_Boolean, TY_NameSpace, MN_("import"), 1, TY_String, FN_("name"),
		_Public, _F(NameSpace_loadScript), TY_Boolean, TY_NameSpace, MN_("load"), 1, TY_String, FN_("path"),
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, NULL, MethodData);
	KSET_KLIB2(kNameSpace_requirePackage, kNameSpace_requirePackage, 0);
	KSET_KLIB2(kNameSpace_importPackage, kNameSpace_importPackage, 0);
}

#ifdef __cplusplus
}
#endif
