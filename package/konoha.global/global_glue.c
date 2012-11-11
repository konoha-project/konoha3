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
	KonohaObjectHeader h;
};

static	kbool_t global_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(NameSpace_AllowImplicitGlobalVariable_), TY_void, TY_NameSpace, MN_("AllowImplicitGlobalVariable"), 1, TY_boolean, FN_("enabled"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

// ---------------------------------------------------------------------------

static kMethod *Object_newProtoSetterNULL(KonohaContext *kctx, kStmt *stmt, kObject *o, ktype_t ty, ksymbol_t symbol)
{
	ktype_t cid = O_typeId(o);
	kNameSpace *ns = Stmt_ns(stmt);
	kMethod *mtd = KLIB kNameSpace_GetSetterMethodNULL(kctx, ns, cid, symbol, TY_var);
	if(mtd != NULL) {
		SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, "already defined name: %s", SYM_t(symbol));
		return NULL;
	}
	mtd = KLIB kNameSpace_GetGetterMethodNULL(kctx, ns, cid, symbol, TY_var);
	if(mtd != NULL && Method_returnType(mtd) != ty) {
		SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, "differently defined name: %s", SYM_t(symbol));
		return NULL;
	}
	int flag = kField_Setter;
	if(mtd == NULL) { // no getter
		flag |= kField_Getter;
	}
	KLIB KonohaClass_addField(kctx, O_ct(o), flag, ty, symbol);
	return KLIB kNameSpace_GetSetterMethodNULL(kctx, ns, cid, symbol, ty);
}

static kStmt* TypeDeclAndMakeSetter(KonohaContext *kctx, kStmt *stmt, kGamma *gma, ktype_t ty, kExpr *termExpr, kExpr *valueExpr, kObject *scr)
{
	kNameSpace *ns = Stmt_ns(stmt);
	kMethod *mtd = Object_newProtoSetterNULL(kctx, stmt, scr, ty, termExpr->termToken->resolvedSymbol);
	if(mtd != NULL) {
		kExpr *recvExpr =  new_ConstValueExpr(kctx, O_typeId(scr), scr);
		kExpr *setterExpr = SUGAR new_TypedCallExpr(kctx, stmt, gma, TY_void, mtd,  2, recvExpr, valueExpr);
		kStmt *newstmt = new_(Stmt, stmt->uline, OnGcStack);
		kStmt_setsyn(newstmt, SYN_(ns, KW_ExprPattern));
		KLIB kObject_setObject(kctx, newstmt, KW_ExprPattern, TY_Expr, setterExpr);
		return newstmt;
	}
	return NULL;
}

static kbool_t kNameSpace_initGlobalObject(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	if(ns->globalObjectNULL_OnList == NULL) {
		KDEFINE_CLASS defGlobalObject = {0};
		defGlobalObject.structname = "GlobalObject";
		defGlobalObject.typeId = TY_newid;
		defGlobalObject.cflag = kClass_Singleton|kClass_Final;
		defGlobalObject.cstruct_size = sizeof(kGlobalObject);
		KonohaClass *cGlobalObject = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defGlobalObject, trace);
		((kNameSpaceVar *)ns)->globalObjectNULL_OnList =  KLIB Knull(kctx, cGlobalObject);
		return KLIB kNameSpace_SetConstData(kctx, ns, SYM_("global"), cGlobalObject->typeId, (uintptr_t)ns->globalObjectNULL_OnList, trace);
	}
	return true;
}

static KMETHOD Statement_GlobalTypeDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kbool_t result = false;
	kNameSpace *ns = Stmt_ns(stmt);
	KMakeTrace(trace, sfp);
	trace->pline = stmt->uline;
	if(kNameSpace_initGlobalObject(kctx, ns, trace)) {
		kToken *tk  = SUGAR kStmt_getToken(kctx, stmt, KW_TypePattern, NULL);
		kExpr  *expr = SUGAR kStmt_getExpr(kctx, stmt, KW_ExprPattern, NULL);
		kStmt *lastStmt = stmt;
		result = SUGAR kStmt_declType(kctx, stmt, gma, tk->resolvedTypeId, expr, ns->globalObjectNULL_OnList, TypeDeclAndMakeSetter, &lastStmt);
	}
	kStmt_done(kctx, stmt);
	KReturnUnboxValue(result);
}

static kbool_t global_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	SUGAR kNameSpace_AddSugarFunc(kctx, ns, KW_TypeDeclPattern, SugarFunc_TopLevelStatement, new_SugarFunc(ns, Statement_GlobalTypeDecl));
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
	return kNameSpace_initGlobalObject(kctx, exportNS, trace);
}

KDEFINE_PACKAGE* global_init(void)
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
