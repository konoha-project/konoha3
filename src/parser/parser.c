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

/* ************************************************************************ */

//#define USING_SUGAR_AS_BUILTIN 1
#define USE_AsciiToKonohaChar

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/klib.h>
#include <konoha3/import/methoddecl.h>

/* ************************************************************************ */

#ifdef __cplusplus
extern "C" {
#endif

// global variable
int verbose_sugar = 0;

#include "import/perror.h"
#include "import/parser_class.h"
#include "import/namespace.h"
#include "import/token.h"
#include "import/preprocess.h"
#include "import/ast.h"
#include "import/pattern.h"
#include "import/typecheck.h"
#include "import/syntax.h"
#include "import/eval.h"
#include "import/parser_dump.h"
#include "import/visitor.h"

/* ------------------------------------------------------------------------ */
/* Sugar Global Functions */

static kstatus_t kNameSpace_Eval(KonohaContext *kctx, kNameSpace *ns, const char *script, kfileline_t uline, KTraceInfo *trace)
{
	kstatus_t result;
	KPARSERM->h.setupModuleContext(kctx, (KRuntimeModule *)KPARSERM, 0/*lazy*/);
	INIT_GCSTACK();
	{
		KTokenSeq tokens = {ns, KGetParserContext(kctx)->preparedTokenList};
		KTokenSeq_Push(kctx, tokens);
		Tokenize(kctx, ns, script, uline, 0, tokens.tokenList);
		KTokenSeq_End(kctx, tokens);
		result = SUGAR EvalTokenList(kctx, &tokens, trace);
		KTokenSeq_Pop(kctx, tokens);
	}
	RESET_GCSTACK();
	return result;
}

/* ------------------------------------------------------------------------ */
/* [KGetParserContext(kctx)] */

static void KParserContext_Reftrace(KonohaContext *kctx, struct KContextModule *baseh, KObjectVisitor *visitor)
{
}

static void KParserContext_Free(KonohaContext *kctx, struct KContextModule *baseh)
{
	KParserContext *base = (KParserContext *)baseh;
	KLIB KArray_Free(kctx, &base->errorMessageBuffer);
	KFree(base, sizeof(KParserContext));
}

static void SugarModule_Setup(KonohaContext *kctx, struct KRuntimeModule *def, int newctx)
{
	if(!newctx && kctx->modlocal[MOD_sugar] == NULL) {
		KParserContext *base = (KParserContext *)KCalloc_UNTRACE(sizeof(KParserContext), 1);
		base->h.reftrace = KParserContext_Reftrace;
		base->h.free     = KParserContext_Free;

		base->errorMessageCount = 0;
		base->preparedTokenList = new_(TokenArray, K_PAGESIZE/sizeof(void *), OnContextConstList);
		base->errorMessageList  = new_(StringArray, 8, OnContextConstList);
		base->definedMethodList = new_(MethodArray, 8, OnContextConstList);

		KLIB KArray_Init(kctx, &base->errorMessageBuffer, K_PAGESIZE);
		kctx->modlocal[MOD_sugar] = (KContextModule *)base;
	}
}

kbool_t Konoha_LoadScript(KonohaContext* kctx, const char *scriptname);
kbool_t Konoha_Eval(KonohaContext* kctx, const char *script, kfileline_t uline);

void MODSUGAR_Init(KonohaContext *kctx, KonohaContextVar *ctx)
{
	KParserModule *mod = (KParserModule *)KCalloc_UNTRACE(sizeof(KParserModule), 1);
	mod->h.name     = "sugar";
	mod->h.allocSize = sizeof(KParserModule);
	mod->h.setupModuleContext    = SugarModule_Setup;
	KLIB KRuntime_SetModule(kctx, MOD_sugar, (KRuntimeModule *)mod, 0);

	KonohaLibVar *l = (KonohaLibVar *)ctx->klib;
	l->kNameSpace_GetClassByFullName  = kNameSpace_GetClassByFullName;
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
	l->kNameSpace_FreeSugarExtension =  kNameSpace_FreeSugarExtension;
	l->Konoha_LoadScript = Konoha_LoadScript;
	l->Konoha_Eval       = Konoha_Eval;
	l->kMethod_GenCode   = kMethod_GenCode;
	l->kMethod_SetFunc   = kMethod_SetFunc;

	KDEFINE_CLASS defSymbol = {0};
	defSymbol.structname = "Symbol";
	defSymbol.typeId = KTypeAttr_NewId;
	defSymbol.cflag = KClassFlag_int;
	defSymbol.init = KClass_(KType_Int)->init;
	defSymbol.unbox = KClass_(KType_Int)->unbox;
	defSymbol.format = kSymbol_format;

	KDEFINE_CLASS defSyntax = {0};
	SETSTRUCTNAME(defSyntax, Syntax);
	defSyntax.init = kSyntax_Init;
	//defSyntax.format = kSyntax_format;

	KDEFINE_CLASS defToken = {0};
	SETSTRUCTNAME(defToken, Token);
	defToken.init = kToken_Init;
	defToken.reftrace = kToken_Reftrace;
	defToken.format = kToken_format;

	KDEFINE_CLASS defNode = {0};
	SETSTRUCTNAME(defNode, Node);
	defNode.init = kNode_Init;
	defNode.reftrace = kNode_Reftrace;
	defNode.format        = kNode_format;

	mod->cSymbol =    KLIB KClass_define(kctx, PackageId_sugar, NULL, &defSymbol, 0);
	mod->cSyntax =    KLIB KClass_define(kctx, PackageId_sugar, NULL, &defSyntax, 0);
	mod->cToken =     KLIB KClass_define(kctx, PackageId_sugar, NULL, &defToken, 0);
	mod->cNode  =     KLIB KClass_define(kctx, PackageId_sugar, NULL, &defNode, 0);
	mod->cTokenArray = KClass_p0(kctx, KClass_Array, mod->cToken->typeId);

	KLIB Knull(kctx, mod->cToken);
	KLIB Knull(kctx, mod->cNode);
	SugarModule_Setup(kctx, &mod->h, 0);

	KDEFINE_INT_CONST ClassData[] = {   // konoha defined class
		{"void", VirtualType_KClass,    (uintptr_t)KClass_void},
		{"boolean", VirtualType_KClass, (uintptr_t)KClass_Boolean},
		{"int",    VirtualType_KClass,  (uintptr_t)KClass_Int},
		{"String", VirtualType_KClass,  (uintptr_t)KClass_String},
		{"Func",   VirtualType_KClass,  (uintptr_t)KClass_Func},
		{"System", VirtualType_KClass,  (uintptr_t)KClass_System},
		{NULL},
	};
	kNameSpace_LoadConstData(kctx, KNULL(NameSpace), KConst_(ClassData), 0);

	mod->Tokenize              = Tokenize;
	mod->ApplyMacroData        = ApplyMacroData;
	mod->SetMacroData          = SetMacroData;
	mod->Preprocess            = Preprocess;
	mod->EvalTokenList         = EvalTokenList;
	mod->ParseTypePattern      = ParseTypePattern;
	mod->kToken_ToBraceGroup   = kToken_ToBraceGroup;
	mod->kNode_AddParsedObject = kNode_AddParsedObject;
	mod->FindEndOfStatement    = FindEndOfStatement;
	mod->kNode_ParseFlag       = kNode_ParseFlag;
	mod->kNode_GetToken        = kNode_GetToken;
	mod->kNode_GetNode         = kNode_GetNode;
	mod->kNode_SetConst        = kNode_SetConst;
	mod->kNode_SetUnboxConst   = kNode_SetUnboxConst;
	mod->kNode_SetVariable     = kNode_SetVariable;
	mod->TypeCheckNodeAt       = TypeCheckNodeAt;
	mod->TypeCheckNodeByName   = TypeCheckNodeByName;
	mod->TypeCheckMethodParam  = TypeCheckMethodParam;
	mod->new_MethodNode        = new_MethodNode;
	mod->AddLocalVariable      = AddLocalVariable;
	mod->kNode_DeclType        = kNode_DeclType;
	mod->TypeVariableNULL      = TypeVariableNULL;

	mod->kNameSpace_DefineSyntax = kNameSpace_DefineSyntax;
	mod->kNameSpace_GetSyntax    = kNameSpace_GetSyntax;
	mod->kSyntax_AddPattern      = kSyntax_AddPattern;
	mod->kNameSpace_AddSyntax    = kNameSpace_AddSyntax;
	mod->kNameSpace_UseDefaultVirtualMachine = kNameSpace_UseDefaultVirtualMachine;
	mod->kNode_InsertAfter       = kNode_InsertAfter;
	mod->kNode_AddNode           = kNode_AddNode;
	mod->kNode_Op                = kNode_Op;
	mod->ParseSyntaxNode         = ParseSyntaxNode;
	mod->ParseNode               = ParseNode;
	mod->ParseNewNode            = ParseNewNode;
	mod->AppendParsedNode        = AppendParsedNode;
	mod->kToken_ToError          = kToken_ToError;
	mod->MessageNode             = MessageNode;
	mod->VisitNode               = VisitNode;

#ifndef USE_SMALLBUILD
	mod->dumpToken      = dumpToken;
	mod->dumpTokenArray = dumpTokenArray;
#endif

	DefineDefaultSyntax(kctx, KNULL(NameSpace));
}

// boolean NameSpace.load(String path);
static KMETHOD NameSpace_loadScript(KonohaContext *kctx, KonohaStack *sfp)
{
	char pathbuf[512];
	const char *path = PLATAPI formatTransparentPath(pathbuf, sizeof(pathbuf), KFileLine_textFileName(sfp[K_RTNIDX].calledFileLine), kString_text(sfp[1].asString));
	KMakeTrace(trace, sfp);
	kNameSpace_LoadScript(kctx, sfp[0].asNameSpace, path, trace);
}

// boolean NameSpace.import(String pkgname);
static KMETHOD NameSpace_ImportPackage(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	kNameSpace_ImportPackage(kctx, sfp[0].asNameSpace, kString_text(sfp[1].asString), trace);
}

// boolean NameSpace.hate(String symbol);
static KMETHOD NameSpace_hate(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *key = sfp[2].asString;
	ksymbol_t keyword = KAsciiSymbol(kString_text(key), kString_size(key), _NEWID);
	KMakeTrace(trace, sfp);
	kNameSpace_RemoveSyntax(kctx, sfp[0].asNameSpace, keyword, trace);
}

static void kNameSpace_SetStaticFunction(KonohaContext *kctx, kNameSpace *ns, kArray *list, ktypeattr_t cid, KTraceInfo *trace)
{
	size_t i;
	for(i = 0; i < kArray_size(list); i++) {
		kMethod *mtd = list->MethodItems[i];
		if(kMethod_Is(Static, mtd) && mtd->typeId == cid) {
			uintptr_t mtdinfo = ((uintptr_t)cid | (((uintptr_t)mtd->mn) << (sizeof(ktypeattr_t) * 8)));
			KLIB kNameSpace_SetConstData(kctx, ns, mtd->mn, VirtualType_StaticMethod, mtdinfo, trace);
		}
	}
}

//## void NameSpace.useStaticFunc(Object o);
static KMETHOD NameSpace_useStaticFunc(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	KClass *ct = kObject_class(sfp[1].asObject);
	kNameSpace *ns = sfp[0].asNameSpace;
	kNameSpace_SetStaticFunction(kctx, ns, ct->classMethodList, ct->typeId, trace);
	while(ns != NULL) {
		kNameSpace_SetStaticFunction(kctx, ns, ns->methodList_OnList, ct->typeId, trace);
		ns = ns->parentNULL;
	}
	KReturnVoid();
}

//## @Public @Const @Immutable @Coercion Symbol String.toSymbol();
static KMETHOD String_toSymbol(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(KAsciiSymbol(kString_text(sfp[0].asString), kString_size(sfp[0].asString), _NEWID));
}

//## @Public @Const @Immutable @Coercion String Symbol.toString();
static KMETHOD KSymbol_toString(KonohaContext *kctx, KonohaStack *sfp)
{
	ksymbol_t symbol = (ksymbol_t)sfp[0].intValue;
	kString *s = KSymbol_GetString(kctx, KSymbol_Unmask(symbol));
	if(KSymbol_Attr(symbol) != 0) {
		KBuffer wb;
		const char *prefix = KSymbol_prefixText(symbol);
		KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
		KLIB KBuffer_Write(kctx, &wb, prefix, strlen(prefix));
		KLIB KBuffer_Write(kctx, &wb, kString_text(s), kString_size(s));
		s = KLIB KBuffer_Stringfy(kctx, &wb, OnStack, StringPolicy_FreeKBuffer);
	}
	KReturn(s);
}

void LoadDefaultSugarMethod(KonohaContext *kctx, kNameSpace *ns)
{
	KSetKLibFunc(0, ReportScriptMessage,           TRACE_ReportScriptMessage,           NULL);
	KSetKLibFunc(0, kNameSpace_RequirePackage,      kNameSpace_RequirePackage,      NULL);
	KSetKLibFunc(0, kNameSpace_ImportPackage,       kNameSpace_ImportPackage,       NULL);
	KSetKLibFunc(0, kNameSpace_LoadScript,          kNameSpace_LoadScript,          NULL);
	KSetKLibFunc(0, kNameSpace_GetConstNULL,        kNameSpace_GetConstNULL,        NULL);
	KDEFINE_METHOD MethodData[] = {
		_Public|_Compilation, _F(NameSpace_DefineConst), KType_Boolean, KType_NameSpace, KMethodName_("DefineConst"), 2, KType_Symbol, KFieldName_("symbol"), KType_Object, KFieldName_("value"),
		_Public|_Compilation, _F(NameSpace_ImportPackage), KType_void, KType_NameSpace, KMethodName_("import"), 1, KType_String, KFieldName_("package"),
		_Public, _F(NameSpace_hate), KType_Boolean, KType_NameSpace, KMethodName_("hate"), 1, KType_String, KFieldName_("symbol"),
		_Public|_Compilation, _F(NameSpace_loadScript), KType_void, KType_NameSpace, KMethodName_("load"), 1, KType_String, KFieldName_("filename"),
		_Public|_Compilation, _F(NameSpace_loadScript), KType_void, KType_NameSpace, KMethodName_("include"), 1, KType_String, KFieldName_("filename"),
		_Public, _F(NameSpace_useStaticFunc), KType_void, KType_NameSpace, KMethodName_("UseStaticFunc"), 1, KType_Object, KFieldName_("class"),
		_Public|_Coercion|_Const|_Imm, _F(String_toSymbol), KType_Symbol, KType_String, KMethodName_To(KType_Symbol), 0,
		_Public|_Coercion|_Const|_Imm, _F(KSymbol_toString), KType_String, KType_Symbol, KMethodName_To(KType_String), 0,
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
	PLATAPI EventModule.EnterEventContext(kctx, NULL);
}

static void KonohaContext_ExitCStack(KonohaContext *kctx)
{
	PLATAPI EventModule.ExitEventContext(kctx, NULL);
	kctx->stack->cstack_bottom = NULL;
}

static kstatus_t MODSUGAR_loadScript(KonohaContext *kctx, const char *path, size_t len, KTraceInfo *trace)
{
	if(KGetParserContext(kctx) == NULL) {
		KPARSERM->h.setupModuleContext(kctx, (KRuntimeModule *)KPARSERM, 0/*lazy*/);
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
	KPARSERM->h.setupModuleContext(kctx, (KRuntimeModule *)KPARSERM, 0/*lazy*/);
	kbool_t res = (kNameSpace_Eval(kctx, KNULL(NameSpace), script, uline, NULL/*trace*/) == K_CONTINUE);    // FIXME
	KEndKonohaContext();
	return res;
}

#ifdef __cplusplus
}
#endif
