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
#include <minikonoha/sugar.h>
#include <minikonoha/local.h>

/* ************************************************************************ */

#ifdef __cplusplus
extern "C" {
#endif

// global variable
int verbose_sugar = 0;

#include "perror.h"
#include "sugarclass.h"
#include "namespace.h"
#include "macro.h"
#include "token.h"
#include "ast.h"
#include "tycheck.h"
#include "sugarfunc.h"
#include "sugardump.h"

/* ------------------------------------------------------------------------ */
/* Sugar Global Functions */

static kstatus_t kNameSpace_Eval(KonohaContext *kctx, kNameSpace *ns, const char *script, kfileline_t uline, KTraceInfo *trace)
{
	kstatus_t result;
	kmodsugar->h.setupModuleContext(kctx, (KonohaModule *)kmodsugar, 0/*lazy*/);
	INIT_GCSTACK();
	{
		TokenSeq tokens = {ns, GetSugarContext(kctx)->preparedTokenList};
		TokenSeq_Push(kctx, tokens);
		TokenSeq_Tokenize(kctx, &tokens, script, uline);
		result = TokenSeq_Eval(kctx, &tokens, trace);
		TokenSeq_Pop(kctx, tokens);
	}
	RESET_GCSTACK();
	return result;
}

/* ------------------------------------------------------------------------ */
/* [GetSugarContext(kctx)] */

static void SugarContext_Reftrace(KonohaContext *kctx, struct KonohaModuleContext *baseh, KObjectVisitor *visitor)
{
//	SugarContext *base = (SugarContext *)baseh;
//	BEGIN_REFTRACE(4);
//	KREFTRACEv(base->preparedTokenList);
//	KREFTRACEv(base->errorMessageList);
//	KREFTRACEv(base->preparedGamma);
//	KREFTRACEv(base->definedMethodList);
//	END_REFTRACE();
}
static void SugarContext_Free(KonohaContext *kctx, struct KonohaModuleContext *baseh)
{
	SugarContext *base = (SugarContext *)baseh;
	KLIB Karray_Free(kctx, &base->errorMessageBuffer);
	KFree(base, sizeof(SugarContext));
}

static void SugarModule_Setup(KonohaContext *kctx, struct KonohaModule *def, int newctx)
{
	if(!newctx && kctx->modlocal[MOD_sugar] == NULL) {
		SugarContext *base = (SugarContext *)KCalloc_UNTRACE(sizeof(SugarContext), 1);
		base->h.reftrace = SugarContext_Reftrace;
		base->h.free     = SugarContext_Free;

		base->errorMessageCount = 0;
		base->preparedTokenList = new_(TokenArray, K_PAGESIZE/sizeof(void *), OnContextConstList);
		base->errorMessageList  = new_(StringArray, 8, OnContextConstList);
		base->definedMethodList = new_(MethodArray, 8, OnContextConstList);
		base->preparedGamma     = new_(Gamma, NULL, OnContextConstList);

		KLIB Karray_Init(kctx, &base->errorMessageBuffer, K_PAGESIZE);
		kctx->modlocal[MOD_sugar] = (KonohaModuleContext *)base;
	}
}

kbool_t Konoha_LoadScript(KonohaContext* kctx, const char *scriptname);
kbool_t Konoha_Eval(KonohaContext* kctx, const char *script, kfileline_t uline);

void MODSUGAR_Init(KonohaContext *kctx, KonohaContextVar *ctx)
{
	KModuleSugar *mod = (KModuleSugar *)KCalloc_UNTRACE(sizeof(KModuleSugar), 1);
	mod->h.name     = "sugar";
	mod->h.allocSize = sizeof(KModuleSugar);
	mod->h.setupModuleContext    = SugarModule_Setup;
	KLIB KonohaRuntime_setModule(kctx, MOD_sugar, (KonohaModule *)mod, 0);

	KonohaLibVar* l = (KonohaLibVar *)ctx->klib;
	l->kNameSpace_GetClass       = kNameSpace_GetClass;
	l->kNameSpace_DefineClass    = kNameSpace_DefineClass;
	l->kNameSpace_LoadMethodData = kNameSpace_LoadMethodData;
	l->kNameSpace_SetConstData   = kNameSpace_SetConstData;
	l->kNameSpace_LoadConstData  = kNameSpace_LoadConstData;
	l->kNameSpace_GetGetterMethodNULL  = kNameSpace_GetGetterMethodNULL;
	l->kNameSpace_GetSetterMethodNULL  = kNameSpace_GetSetterMethodNULL;
	l->kNameSpace_GetCoercionMethodNULL = kNameSpace_GetCoercionMethodNULL;
	l->kNameSpace_GetMethodByParamSizeNULL  = kNameSpace_GetMethodByParamSizeNULL;
	l->kNameSpace_GetMethodBySignatureNULL  = kNameSpace_GetMethodBySignatureNULL;
	l->kMethod_DoLazyCompilation = kMethod_DoLazyCompilation;
//	l->kNameSpace_compileAllDefinedMethods  = kNameSpace_compileAllDefinedMethods;
	l->kNameSpace_FreeSugarExtension =  kNameSpace_FreeSugarExtension;
	l->Konoha_LoadScript = Konoha_LoadScript;
	l->Konoha_Eval       = Konoha_Eval;


	KDEFINE_CLASS defToken = {0};
	SETSTRUCTNAME(defToken, Token);
	defToken.init = kToken_Init;
	defToken.reftrace = kToken_Reftrace;
	defToken.p = kToken_p;
	
	KDEFINE_CLASS defExpr = {0};
	SETSTRUCTNAME(defExpr, Expr);
	defExpr.init = Expr_Init;
	defExpr.reftrace = Expr_Reftrace;
	
	KDEFINE_CLASS defStmt = {0};
	SETSTRUCTNAME(defStmt, Stmt);
	defStmt.init = kStmt_Init;
	defStmt.reftrace = kStmt_Reftrace;
	defStmt.p        = kStmt_p;
	
	KDEFINE_CLASS defBlock = {0};
	SETSTRUCTNAME(defBlock, Block);
	defBlock.init = kBlock_Init;
	defBlock.reftrace = kBlock_Reftrace;
	
	KDEFINE_CLASS defGamma = {0};
	SETSTRUCTNAME(defGamma, Gamma);
	defGamma.init = Gamma_Init;

	mod->cToken =     KLIB KonohaClass_define(kctx, PackageId_sugar, NULL, &defToken, 0);
	mod->cExpr  =     KLIB KonohaClass_define(kctx, PackageId_sugar, NULL, &defExpr, 0);
	mod->cStmt  =     KLIB KonohaClass_define(kctx, PackageId_sugar, NULL, &defStmt, 0);
	mod->cBlock =     KLIB KonohaClass_define(kctx, PackageId_sugar, NULL, &defBlock, 0);
	mod->cGamma =     KLIB KonohaClass_define(kctx, PackageId_sugar, NULL, &defGamma, 0);
	mod->cTokenArray = CT_p0(kctx, CT_Array, mod->cToken->typeId);

	KLIB Knull(kctx, mod->cToken);
	KLIB Knull(kctx, mod->cExpr);
	kStmtVar *NullStmt = (kStmtVar *)KLIB Knull(kctx, mod->cStmt);
	KFieldSet(NullStmt, NullStmt->parentBlockNULL, (kBlock *)KLIB Knull(kctx, mod->cBlock));

	SugarModule_Setup(kctx, &mod->h, 0);

	KDEFINE_INT_CONST ClassData[] = {   // minikonoha defined class
		{"void", VirtualType_KonohaClass, (uintptr_t)CT_void},
		{"boolean", VirtualType_KonohaClass, (uintptr_t)CT_Boolean},
		{"int",    VirtualType_KonohaClass, (uintptr_t)CT_Int},
		{"String", VirtualType_KonohaClass, (uintptr_t)CT_String},
		{"Func",   VirtualType_KonohaClass, (uintptr_t)CT_Func},
		{"System", VirtualType_KonohaClass, (uintptr_t)CT_System},
		{NULL},
	};
	kNameSpace_LoadConstData(kctx, KNULL(NameSpace), KonohaConst_(ClassData), 0);

	mod->kNameSpace_SetTokenFunc       = kNameSpace_SetTokenFunc;
	mod->TokenSeq_Tokenize        = TokenSeq_Tokenize;
	mod->TokenSeq_ApplyMacro      = TokenSeq_ApplyMacro;
	mod->kNameSpace_SetMacroData       = kNameSpace_SetMacroData;
	mod->TokenSeq_Resolve        = TokenSeq_Resolve;
	mod->TokenSeq_Eval            = TokenSeq_Eval;
	mod->TokenUtils_ParseTypePattern     = TokenUtils_ParseTypePattern;
	mod->kToken_ToBraceGroup = kToken_ToBraceGroup;
	mod->kStmt_AddParsedObject      = kStmt_AddParsedObject;
	mod->kNameSpace_FindEndOfStatement = kNameSpace_FindEndOfStatement;
	mod->kStmt_ParseFlag            = kStmt_ParseFlag;
	mod->kStmt_GetToken             = kStmt_GetToken;
	mod->kStmt_GetBlock             = kStmt_GetBlock;
	mod->kStmt_GetExpr              = kStmt_GetExpr;
	mod->kStmt_GetText              = kStmt_GetText;
	mod->kExpr_SetConstValue        = kExpr_SetConstValue;
	mod->kExpr_SetUnboxConstValue   = kExpr_SetUnboxConstValue;
	mod->kExpr_SetVariable          = kExpr_SetVariable;
	mod->kStmt_TypeCheckExprAt        = kStmt_TypeCheckExprAt;
	mod->kStmt_TypeCheckByName        = kStmt_TypeCheckByName;
	mod->kBlock_TypeCheckAll          = kBlock_TypeCheckAll;
	mod->kStmtkExpr_TypeCheckCallParam = kStmtkExpr_TypeCheckCallParam;
	mod->new_TypedCallExpr          = new_TypedCallExpr;
	mod->kGamma_AddLocalVariable = kGamma_AddLocalVariable;
	mod->kStmt_DeclType             = kStmt_DeclType;
	mod->kStmt_TypeCheckVariableNULL  = kStmt_TypeCheckVariableNULL;

	mod->kNameSpace_DefineSyntax    = kNameSpace_DefineSyntax;
	mod->kNameSpace_GetSyntax       = kNameSpace_GetSyntax;
	mod->kArray_AddSyntaxRule       = kArray_AddSyntaxPattern;
//	mod->kNameSpace_SetSugarFunc    = kNameSpace_SetSugarFunc;
	mod->kNameSpace_AddSugarFunc    = kNameSpace_AddSugarFunc;
	mod->new_kBlock                 = new_kBlock;
	mod->new_kStmt                  = new_kStmt;
	mod->kBlock_InsertAfter         = kBlock_InsertAfter;
	mod->new_UntypedTermExpr        = new_UntypedTermExpr;
	mod->new_UntypedCallStyleExpr   = new_UntypedCallStyleExpr;
	mod->kStmt_ParseOperatorExpr    = kStmt_ParseOperatorExpr;
	mod->kStmt_ParseExpr            = kStmt_ParseExpr;
	mod->kStmt_AddExprParam         = kStmt_AddExprParam;
	mod->kStmt_RightJoinExpr        = kStmt_RightJoinExpr;
	mod->kToken_ToError        = kToken_ToError;
	mod->kStmt_Message2        = kStmt_Message2;

#ifndef USE_SMALLBUILD
	mod->dumpToken      = dumpToken;
	mod->dumpTokenArray = dumpTokenArray;
	mod->dumpExpr       = dumpExpr;
#endif

	DefineDefaultSyntax(kctx, KNULL(NameSpace));
}

// boolean NameSpace.load(String path);
static KMETHOD NameSpace_loadScript(KonohaContext *kctx, KonohaStack *sfp)
{
	char pathbuf[512];
	const char *path = PLATAPI formatTransparentPath(pathbuf, sizeof(pathbuf), FileId_t(sfp[K_RTNIDX].calledFileLine), S_text(sfp[1].asString));
	KMakeTrace(trace, sfp);
	kNameSpace_LoadScript(kctx, sfp[0].asNameSpace, path, trace);
}

// boolean NameSpace.import(String pkgname);
static KMETHOD NameSpace_ImportPackage(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	kNameSpace_ImportPackage(kctx, sfp[0].asNameSpace, S_text(sfp[1].asString), trace);
}

// boolean NameSpace.import(String pkgname, String symbol);
static KMETHOD NameSpace_ImportPackageSymbol(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *key = sfp[2].asString;
	ksymbol_t keyword = ksymbolA(S_text(key), S_size(key), _NEWID);
	KMakeTrace(trace, sfp);
	kNameSpace_ImportPackageSymbol(kctx, sfp[0].asNameSpace, S_text(sfp[1].asString), keyword, trace);
}

// boolean NameSpace.hate(String symbol);
static KMETHOD NameSpace_hate(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *key = sfp[2].asString;
	ksymbol_t keyword = ksymbolA(S_text(key), S_size(key), _NEWID);
	KMakeTrace(trace, sfp);
	kNameSpace_RemoveSyntax(kctx, sfp[0].asNameSpace, keyword, trace);
}

static void kNameSpace_SetStaticFunction(KonohaContext *kctx, kNameSpace *ns, kArray *list, ktype_t cid, KTraceInfo *trace)
{
	size_t i;
	for(i = 0; i < kArray_size(list); i++) {
		kMethod *mtd = list->MethodItems[i];
		if(kMethod_is(Static, mtd) && mtd->typeId == cid) {
			uintptr_t mtdinfo = ((uintptr_t)cid | (((uintptr_t)mtd->mn) << (sizeof(ktype_t) * 8)));
			KLIB kNameSpace_SetConstData(kctx, ns, mtd->mn, VirtualType_StaticMethod, mtdinfo, trace);
		}
	}
}

//## void NameSpace.useStaticFunc(Object o);
static KMETHOD NameSpace_useStaticFunc(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	KonohaClass *ct = O_ct(sfp[1].asObject);
	kNameSpace *ns = sfp[0].asNameSpace;
	kNameSpace_SetStaticFunction(kctx, ns, ct->methodList_OnGlobalConstList, ct->typeId, trace);
	while(ns != NULL) {
		kNameSpace_SetStaticFunction(kctx, ns, ns->methodList_OnList, ct->typeId, trace);
		ns = ns->parentNULL;
	}
	KReturnVoid();
}

#define _Public kMethod_Public
#define _F(F)   (intptr_t)(F)

void LoadDefaultSugarMethod(KonohaContext *kctx, kNameSpace *ns)
{
	KSetKLibFunc(0, ReportScriptMessage,           TRACE_ReportScriptMessage,           NULL);
	KSetKLibFunc(0, kNameSpace_RequirePackage,      kNameSpace_RequirePackage,      NULL);
	KSetKLibFunc(0, kNameSpace_ImportPackage,       kNameSpace_ImportPackage,       NULL);
	KSetKLibFunc(0, kNameSpace_ImportPackageSymbol, kNameSpace_ImportPackageSymbol, NULL);
	KSetKLibFunc(0, kNameSpace_GetConstNULL,        kNameSpace_GetConstNULL,        NULL);
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(NameSpace_ImportPackage), TY_void, TY_NameSpace, MN_("import"), 1, TY_String, FN_("package"),
		_Public, _F(NameSpace_ImportPackageSymbol), TY_void, TY_NameSpace, MN_("import"), 2, TY_String, FN_("package"), TY_String, FN_("symbol"),
		_Public, _F(NameSpace_hate), TY_boolean, TY_NameSpace, MN_("hate"), 1, TY_String, FN_("symbol"),
		_Public, _F(NameSpace_loadScript), TY_void, TY_NameSpace, MN_("load"), 1, TY_String, FN_("filename"),
		_Public, _F(NameSpace_loadScript), TY_void, TY_NameSpace, MN_("include"), 1, TY_String, FN_("filename"),
		_Public, _F(NameSpace_useStaticFunc), TY_void, TY_NameSpace, MN_("useStaticFunc"), 1, TY_Object, FN_("class"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, NULL);
}


// --------------------------------------------------------------------------
/* Konoha C API */

#define KBeginKonohaContext()  KonohaContext_EnterCStack(kctx, (void**)&kctx)
#define KEndKonohaContext()    KonohaContext_ExitCStack(kctx)

static void KonohaContext_EnterCStack(KonohaContext *kctx, void **bottom)
{
	kctx->stack->cstack_bottom = bottom;
	PLATAPI EnterEventContext(kctx);
}

static void KonohaContext_ExitCStack(KonohaContext *kctx)
{
	PLATAPI ExitEventContext(kctx);
	kctx->stack->cstack_bottom = NULL;
}

static kstatus_t MODSUGAR_loadScript(KonohaContext *kctx, const char *path, size_t len, KTraceInfo *trace)
{
	if(GetSugarContext(kctx) == NULL) {
		kmodsugar->h.setupModuleContext(kctx, (KonohaModule *)kmodsugar, 0/*lazy*/);
	}
	INIT_GCSTACK();
	kpackageId_t packageId = KLIB KpackageId(kctx, "main", sizeof("main")-1, 0, _NEWID);
	kNameSpace *ns = new_PackageNameSpace(kctx, packageId);
	kstatus_t result = (kstatus_t)kNameSpace_LoadScript(kctx, ns, path, trace);
	RESET_GCSTACK();
	return result;
}

kbool_t Konoha_LoadScript(KonohaContext* kctx, const char *scriptname)
{
	KBeginKonohaContext();
	PLATAPI BEFORE_LoadScript(kctx, scriptname);
	kbool_t res = (MODSUGAR_loadScript(kctx, scriptname, strlen(scriptname), 0) == K_CONTINUE);
	PLATAPI AFTER_LoadScript(kctx, scriptname);
	KEndKonohaContext();
	return res;
}

kbool_t Konoha_Eval(KonohaContext* kctx, const char *script, kfileline_t uline)
{
	KBeginKonohaContext();
	if(verbose_sugar) {
		DUMP_P("\n>>>----\n'%s'\n------\n", script);
	}
	kmodsugar->h.setupModuleContext(kctx, (KonohaModule *)kmodsugar, 0/*lazy*/);
	kbool_t res = (kNameSpace_Eval(kctx, KNULL(NameSpace), script, uline, NULL/*trace*/) == K_CONTINUE);    // FIXME
	KEndKonohaContext();
	return res;
}

#ifdef __cplusplus
}
#endif
