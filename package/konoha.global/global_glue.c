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

#define _Public   kMethod_Public
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

static KMETHOD NameSpace_setTransparentGlobalVariable_(KonohaContext *kctx, KonohaStack *sfp)
{
	kNameSpace_set(TransparentGlobalVariable, sfp[0].asNameSpace, sfp[1].boolValue);
}

static	kbool_t global_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KRequirePackage("konoha.field", pline);
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(NameSpace_setTransparentGlobalVariable_), TY_void, TY_NameSpace, MN_("setTransparentGlobalVariable"), 1, TY_boolean, FN_("enabled"),
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t global_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kMethod *Object_newProtoSetterNULL(KonohaContext *kctx, kStmt *stmt, kObject *o, ktype_t ty, ksymbol_t symbol)
{
	ktype_t cid = O_typeId(o);
	kNameSpace *ns = Stmt_nameSpace(stmt);
	kMethod *mtd = KLIB kNameSpace_getSetterMethodNULL(kctx, ns, cid, symbol, TY_var);
	if(mtd != NULL) {
		SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, "already defined name: %s", SYM_t(symbol));
		return NULL;
	}
	mtd = KLIB kNameSpace_getGetterMethodNULL(kctx, ns, cid, symbol, TY_var);
	if(mtd != NULL && Method_returnType(mtd) != ty) {
		SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, "differently defined name: %s", SYM_t(symbol));
		return NULL;
	}
	int flag = kField_Setter;
	if(mtd == NULL) { // no getter
		flag |= kField_Getter;
	}
	KLIB KonohaClass_addField(kctx, O_ct(o), flag, ty, symbol);
	return KLIB kNameSpace_getSetterMethodNULL(kctx, ns, cid, symbol, ty);
}

// ---------------------------------------------------------------------------

static kStmt* TypeDeclAndMakeSetter(KonohaContext *kctx, kStmt *stmt, kGamma *gma, ktype_t ty, kExpr *termExpr, kExpr *valueExpr, kObject *scr)
{
	kNameSpace *ns = Stmt_nameSpace(stmt);
	kMethod *mtd = Object_newProtoSetterNULL(kctx, stmt, scr, ty, termExpr->termToken->resolvedSymbol);
	if(mtd != NULL) {
		kExpr *recvExpr =  new_ConstValueExpr(kctx, O_typeId(scr), scr);
		kExpr *setterExpr = SUGAR new_TypedCallExpr(kctx, stmt, gma, TY_void, mtd,  2, recvExpr, valueExpr);
		kStmt *newstmt = GCSAFE_new(Stmt, stmt->uline);
		kStmt_setsyn(newstmt, SYN_(ns, KW_ExprPattern));
		KLIB kObject_setObject(kctx, newstmt, KW_ExprPattern, TY_Expr, setterExpr);
		return newstmt;
	}
	return NULL;
}

typedef const struct _kGlobalObject kGlobalObject;
struct _kGlobalObject {
	KonohaObjectHeader h;
};

static kbool_t kNameSpace_initGlobalObject(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	if(ns->globalObjectNULL == NULL) {
		KDEFINE_CLASS defGlobalObject = {0};
		defGlobalObject.structname = "GlobalObject";
		defGlobalObject.typeId = TY_newid;
		defGlobalObject.cflag = kClass_Singleton|kClass_Final;
		defGlobalObject.cstruct_size = sizeof(kGlobalObject);

		KonohaClass *cGlobalObject = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defGlobalObject, pline);
		KINITp(ns, ((kNameSpaceVar*)ns)->globalObjectNULL, KLIB Knull(kctx, cGlobalObject));
		return KLIB kNameSpace_setConstData(kctx, ns, SYM_("global"), cGlobalObject->typeId, (uintptr_t)ns->globalObjectNULL, pline);
	}
	return true;
}

static KMETHOD StmtTyCheck_GlobalTypeDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_StmtTyCheck(stmt, gma);
	kbool_t result = false;
	kNameSpace *ns = Stmt_nameSpace(stmt);
	if(kNameSpace_initGlobalObject(kctx, ns, stmt->uline)) {
		kToken *tk  = SUGAR kStmt_getToken(kctx, stmt, KW_TypePattern, NULL);
		kExpr  *expr = SUGAR kStmt_getExpr(kctx, stmt, KW_ExprPattern, NULL);
		kStmt *lastStmt = stmt;
		result = SUGAR kStmt_declType(kctx, stmt, gma, tk->resolvedTypeId, expr, ns->globalObjectNULL, TypeDeclAndMakeSetter, &lastStmt);
	}
	kStmt_done(kctx, stmt);
	RETURNb_(result);
}

static kbool_t global_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	KImportPackage(ns, "konoha.field", pline);
	SUGAR kNameSpace_setSugarFunc(kctx, ns, KW_StmtTypeDecl, SUGARFUNC_TopStmtTyCheck, new_SugarFunc(StmtTyCheck_GlobalTypeDecl));
	return kNameSpace_initGlobalObject(kctx, ns, pline);
}

static kbool_t global_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* global_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSETPACKNAME(d, "global", "1.0");
	d.initPackage    = global_initPackage;
	d.setupPackage   = global_setupPackage;
	d.initNameSpace  = global_initNameSpace;
	d.setupNameSpace = global_setupNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
