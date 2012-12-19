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

#ifdef __cplusplus
extern "C"{
#endif

// --------------------------------------------------------------------------

static KMETHOD NameSpace_AllowImplicitGlobalVariable_(KonohaContext *kctx, KonohaStack *sfp)
{
	kNameSpace_Set(ImplicitGlobalVariable, sfp[0].asNameSpace, sfp[1].boolValue);
}

#define _Public   kMethod_Public
#define _F(F)   (intptr_t)(F)

typedef const struct _kGlobalObject kGlobalObject;
struct _kGlobalObject {
	kObjectHeader h;
};

static	kbool_t global_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(NameSpace_AllowImplicitGlobalVariable_), KType_void, KType_NameSpace, KKMethodName_("AllowImplicitGlobalVariable"), 1, KType_boolean, KFieldName_("enabled"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

// ---------------------------------------------------------------------------

static kMethod *Object_newProtoSetterNULL(KonohaContext *kctx, kNode *stmt, kObject *o, ktypeattr_t ty, ksymbol_t symbol)
{
	kNameSpace *ns = Node_ns(stmt);
	kMethod *mtd = KLIB kNameSpace_GetSetterMethodNULL(kctx, ns, kObject_class(o), symbol, KType_var);
	if(mtd != NULL) {
		SUGAR kNode_Message2(kctx, stmt, NULL, ErrTag, "already defined name: %s", KSymbol_text(symbol));
		return NULL;
	}
	mtd = KLIB kNameSpace_GetGetterMethodNULL(kctx, ns, kObject_class(o), symbol);
	if(mtd != NULL && kMethod_GetReturnType(mtd)->typeId != ty) {
		SUGAR kNode_Message2(kctx, stmt, NULL, ErrTag, "differently defined name: %s", KSymbol_text(symbol));
		return NULL;
	}
	KLIB KClass_AddField(kctx, kObject_class(o), ty, symbol);
	return KLIB kNameSpace_GetSetterMethodNULL(kctx, ns, kObject_class(o), symbol, ty);
}

static kNode* TypeDeclAndMakeSetter(KonohaContext *kctx, kNode *stmt, kGamma *gma, ktypeattr_t ty, kNode *termNode, kNode *valueNode, kObject *scr)
{
	kNameSpace *ns = Node_ns(stmt);
	kMethod *mtd = Object_newProtoSetterNULL(kctx, stmt, scr, ty, termNode->TermToken->resolvedSymbol);
	if(mtd != NULL) {
		kNode *recvNode =  new_ConstValueNode(kctx, NULL, scr);
		kNode *setterNode = SUGAR new_TypedCallNode(kctx, stmt, gma, KType_void, mtd,  2, recvNode, valueNode);
		kNode *newstmt = new_(Node, kNode_uline(stmt), OnGcStack);
		kNode_Setsyn(newstmt, KSyntax_(ns, KSymbol_NodePattern));
		KLIB kObjectProto_SetObject(kctx, newstmt, KSymbol_NodePattern, KType_Node, setterNode);
		return newstmt;
	}
	return NULL;
}

static kbool_t kNameSpace_InitGlobalObject(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	if(ns->globalObjectNULL_OnList == NULL) {
		KDEFINE_CLASS defGlobalObject = {0};
		defGlobalObject.structname = "GlobalObject";
		defGlobalObject.typeId = KTypeAttr_NewId;
		defGlobalObject.cflag = KClassFlag_Singleton|KClassFlag_Final;
		defGlobalObject.cstruct_size = sizeof(kGlobalObject);
		KClass *cGlobalObject = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defGlobalObject, trace);
		((kNameSpaceVar *)ns)->globalObjectNULL_OnList =  KLIB Knull(kctx, cGlobalObject);
		return KLIB kNameSpace_SetConstData(kctx, ns, KSymbol_("global"), cGlobalObject->typeId, (uintptr_t)ns->globalObjectNULL_OnList, true/*isOverride*/, trace);
	}
	return true;
}

static KMETHOD Statement_GlobalTypeDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kbool_t result = false;
	kNameSpace *ns = Node_ns(stmt);
	KMakeTrace(trace, sfp);
	trace->pline = kNode_uline(stmt);
	if(kNameSpace_InitGlobalObject(kctx, ns, trace)) {
		kToken *tk  = SUGAR kNode_GetToken(kctx, stmt, KSymbol_TypePattern, NULL);
		kNode  *expr = SUGAR kNode_GetNode(kctx, stmt, KSymbol_NodePattern, NULL);
		kNode *lastNode = stmt;
		result = SUGAR kNode_DeclType(kctx, stmt, gma, tk->resolvedTypeId, expr, ns->globalObjectNULL_OnList, TypeDeclAndMakeSetter, &lastNode);
	}
	kNode_done(kctx, stmt);
	KReturnUnboxValue(result);
}

static kbool_t global_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	SUGAR kNameSpace_AddSugarFunc(kctx, ns, KSymbol_TypeDeclPattern, SugarFunc_TopLevelStatement, new_SugarFunc(ns, Statement_GlobalTypeDecl));
	return true;
}

// -------

static	kbool_t global_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequirePackage("konoha.field", trace);
	global_defineMethod(kctx, ns, trace);
	global_defineSyntax(kctx, ns, trace);
	return true;
}

static kbool_t global_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	KImportPackage(exportNS, "konoha.field", trace);
	return kNameSpace_InitGlobalObject(kctx, exportNS, trace);
}

KDEFINE_PACKAGE* global_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", K_VERSION);
	d.PackupNameSpace   = global_PackupNameSpace;
	d.ExportNameSpace   = global_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif