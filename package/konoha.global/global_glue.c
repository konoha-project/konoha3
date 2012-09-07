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

#include<minikonoha/minikonoha.h>
#include<minikonoha/sugar.h>

// --------------------------------------------------------------------------

static	kbool_t global_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	return true;
}

static kbool_t global_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

// --------------------------------------------------------------------------

static KMETHOD MethodFunc_ProtoGetter(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[K_MTDIDX].mtdNC;
	ksymbol_t key = (ksymbol_t)mtd->delta;
	RETURN_(KLIB kObject_getObject(kctx, sfp[0].asObject, key, sfp[K_RTNIDX].o));
}

static KMETHOD MethodFunc_ProtoGetterN(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[K_MTDIDX].mtdNC;
	ksymbol_t key = (ksymbol_t)mtd->delta;
	RETURNd_(KLIB kObject_getUnboxValue(kctx, sfp[0].asObject, key, 0));
}

static KMETHOD MethodFunc_ProtoSetter(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[K_MTDIDX].mtdNC;
	ksymbol_t key = (ksymbol_t)mtd->delta;
	KLIB kObject_setObject(kctx, sfp[0].asObject, key, O_typeId(sfp[1].asObject), sfp[1].asObject);
	RETURN_(sfp[1].asObject);
}

static KMETHOD MethodFunc_ProtoSetterN(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[K_MTDIDX].mtdNC;
	ksymbol_t key = (ksymbol_t)mtd->delta;
	kParam *pa = Method_param(mtd);
	KLIB kObject_setUnboxValue(kctx, sfp[0].asObject, key, pa->paramtypeItems[0].ty, sfp[1].unboxValue);
	RETURNd_(sfp[1].unboxValue);
}

static kMethod *new_ProtoGetter(KonohaContext *kctx, ktype_t cid, ksymbol_t sym, ktype_t ty)
{
	kmethodn_t mn = ty == TY_boolean ? MN_toISBOOL(sym) : MN_toGETTER(sym);
	MethodFunc f = (TY_isUnbox(ty)) ? MethodFunc_ProtoGetterN : MethodFunc_ProtoGetter;
	kMethod *mtd = KLIB new_kMethod(kctx, kMethod_Public|kMethod_Immutable, cid, mn, f);
	KLIB kMethod_setParam(kctx, mtd, ty, 0, NULL);
	((kMethodVar*)mtd)->delta = sym;
	return mtd;
}

static kMethod *new_ProtoSetter(KonohaContext *kctx, ktype_t cid, ksymbol_t sym, ktype_t ty)
{
	kmethodn_t mn = MN_toSETTER(sym);
	MethodFunc f = (TY_isUnbox(ty)) ? MethodFunc_ProtoSetterN : MethodFunc_ProtoSetter;
	kparamtype_t p = {ty, FN_("x")};
	kMethod *mtd = KLIB new_kMethod(kctx, kMethod_Public, cid, mn, f);
	KLIB kMethod_setParam(kctx, mtd, ty, 1, &p);
	((kMethodVar*)mtd)->delta = sym;
	return mtd;
}

static void KonohaClass_addMethod2(KonohaContext *kctx, KonohaClass *ct, kMethod *mtd)
{
	if(unlikely(ct->methodList == K_EMPTYARRAY)) {
		KINITv(((KonohaClassVar*)ct)->methodList, new_(MethodArray, 8));
	}
	KLIB kArray_add(kctx, ct->methodList, mtd);
}

static kMethod *Object_newProtoSetterNULL(KonohaContext *kctx, kObject *o, kStmt *stmt, kNameSpace *ns, ktype_t ty, ksymbol_t fn)
{
	ktype_t cid = O_typeId(o);
	kMethod *mtd = KLIB kNameSpace_getMethodNULL(kctx, ns, cid, MN_toSETTER(fn), ty, MPOL_SETTER|MPOL_CANONICAL);
	if(mtd != NULL) {
		SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, "already defined name: %s.%s", CT_t(O_ct(o)), SYM_t(fn));
		return NULL;
	}
	mtd = KLIB kNameSpace_getMethodNULL(kctx, ns, cid, MN_toGETTER(fn), 0, MPOL_GETTER);
	if(mtd != NULL && Method_returnType(mtd) != ty) {
		SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, "differently defined getter: %s.%s", CT_t(O_ct(o)), SYM_t(fn));
		return NULL;
	}
	if(mtd == NULL) { // no getter
		KonohaClass_addMethod2(kctx, O_ct(o), new_ProtoGetter(kctx, cid, fn, ty));
	}
	mtd = new_ProtoSetter(kctx, cid, fn, ty);
	KonohaClass_addMethod2(kctx, O_ct(o), mtd);
	return mtd;
}

static inline ksymbol_t tosymbol(KonohaContext *kctx, kExpr *expr)
{
	if(Expr_isSymbolTerm(expr)) {
		return expr->termToken->resolvedSymbol;
	}
	return SYM_NONAME;
}

static KMETHOD StmtTyCheck_var(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_StmtTyCheck(stmt, gma);
	DBG_P("global assign .. ");
	kNameSpace *ns = Stmt_nameSpace(stmt);
	kObject *scr = ns->scriptObject;
	if(O_typeId(scr) == TY_System) {
		SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, " global variables are not available");
		RETURNb_(false);
	}
	kExpr *vexpr = SUGAR kStmt_getExpr(kctx, stmt, SYM_("var"), K_NULLEXPR);
	ksymbol_t fn = tosymbol(kctx, vexpr);
	if(fn == SYM_NONAME) {
		SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, "variable name is expected");
		RETURNb_(false);
	}
	kExpr *expr = SUGAR kStmt_getExpr(kctx, stmt, KW_ExprPattern, K_NULLEXPR);
	if(!SUGAR kStmt_tyCheckByName(kctx, stmt, KW_ExprPattern, gma, TY_var, 0)) {
		RETURNb_(false);
	}
	/*kExpr **/expr = SUGAR kStmt_getExpr(kctx, stmt, KW_ExprPattern, K_NULLEXPR);
	kMethod *mtd = Object_newProtoSetterNULL(kctx, scr, stmt, ns, expr->ty, fn);
	if(mtd == NULL) {
		RETURNb_(false);
	}
	SUGAR kStmt_printMessage2(kctx, stmt, NULL, InfoTag, "%s has type %s", SYM_t(fn), TY_t(expr->ty));
	expr = SUGAR new_TypedCallExpr(kctx, stmt, gma, TY_void, mtd,  2, new_ConstValueExpr(kctx, O_typeId(scr), scr), expr);
	KLIB kObject_setObject(kctx, stmt, KW_ExprPattern, TY_Expr, expr);
	kStmt_typed(stmt, EXPR);
	RETURNb_(true);
}

// ---------------------------------------------------------------------------

//static kMethod* ExprTerm_getSetterNULL(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kObject *scr, kGamma *gma, ktype_t ty)
//{
//	kNameSpace *ns = Stmt_nameSpace(stmt);
//	return Object_newProtoSetterNULL(kctx, scr, stmt, ns, ty, expr->termToken->resolvedSymbol);
//}

static void appendSetterStmt(KonohaContext *kctx, kStmt **lastStmtRef, kStmt *newstmt)
{
	kStmt *lastStmt = lastStmtRef[0];
	SUGAR kBlock_insertAfter(kctx, lastStmt->parentBlockNULL, lastStmt, newstmt);
	DBG_P("laststmt=%p, newstmt=%p", lastStmtRef[0], newstmt);
	lastStmtRef[0] = newstmt;
}

static kbool_t kScriptObject_typeDeclAndSetter(KonohaContext *kctx, kObject *scr, kStmt *stmt, kGamma *gma, ktype_t ty, kExpr *termExpr, kExpr *valueExpr, kStmt **lastStmtRef)
{
	kNameSpace *ns = Stmt_nameSpace(stmt);
	kMethod *mtd = Object_newProtoSetterNULL(kctx, scr, stmt, ns, ty, termExpr->termToken->resolvedSymbol);
	DBG_P("mtd=%p", mtd);
	if(mtd != NULL) {
		kExpr *recvExpr =  new_ConstValueExpr(kctx, O_typeId(scr), scr);
		PUSH_GCSTACK(recvExpr);
		kExpr *setterExpr = SUGAR new_TypedCallExpr(kctx, stmt, gma, TY_void, mtd,  2, recvExpr, valueExpr);
		kStmt *newstmt = GCSAFE_new(Stmt, stmt->uline);
		kStmt_setsyn(newstmt, SYN_(ns, KW_ExprPattern));
		KLIB kObject_setObject(kctx, newstmt, KW_ExprPattern, TY_Expr, setterExpr);
		appendSetterStmt(kctx, lastStmtRef, newstmt);
		return true;
	}
	return false;
}

static kbool_t kScriptObject_typeDecl(KonohaContext *kctx, kObject *scr, kStmt *stmt, kGamma *gma, ktype_t ty, kExpr *expr, kStmt **lastStmtRef)
{
	if(expr->syn->keyword == KW_LET && Expr_isSymbolTerm(kExpr_at(expr, 1))) {
		if(SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 2, gma, ty, 0) == K_NULLEXPR) {
			// this is neccesarry to avoid 'int a = a + 1;';
			return false;
		}
		return kScriptObject_typeDeclAndSetter(kctx, scr, stmt, gma, ty, kExpr_at(expr, 1), kExpr_at(expr, 2), lastStmtRef);
	} else if(Expr_isSymbolTerm(expr)) {
		kExpr *valueExpr = new_VariableExpr(kctx, gma, TEXPR_NULL, ty, 0);
		return kScriptObject_typeDeclAndSetter(kctx, scr, stmt, gma, ty, expr, valueExpr, lastStmtRef);
	} else if(expr->syn->keyword == KW_COMMA) {
		size_t i;
		for(i = 1; i < kArray_size(expr->cons); i++) {
			if(!kScriptObject_typeDecl(kctx, scr, stmt, gma, ty, kExpr_at(expr, i), lastStmtRef)) return false;
		}
		return true;
	}
	SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, "variable name is expected");
	return false;
}

static KMETHOD StmtTyCheck_GlobalTypeDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_StmtTyCheck(stmt, gma);
	kToken *tk  = SUGAR kStmt_getToken(kctx, stmt, KW_TypePattern, NULL);
	kExpr  *expr = SUGAR kStmt_getExpr(kctx, stmt, KW_ExprPattern, NULL);
//	if(tk == NULL || !Token_isVirtualTypeLiteral(tk) || expr == NULL) {
//		RETURNb_(false);
//	}
	kObject *scr = Stmt_nameSpace(stmt)->scriptObject;
	if(O_typeId(scr) == TY_System) {
		SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, " global variables are not available");
		RETURNb_(false);
	}
	kStmt_done(kctx, stmt);
	RETURNb_(kScriptObject_typeDecl(kctx, scr, stmt, gma, Token_typeLiteral(tk), expr, &stmt));
}

typedef const struct _kScript kScript;

struct _kScript {
	KonohaObjectHeader h;
};

static kbool_t global_initNameSpace(KonohaContext *kctx,  kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("var"), TopStmtTyCheck_(var), .rule = "\"var\" var: $Expr \"=\" $Expr", },
		{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX);

	SUGAR kNameSpace_setSugarFunc(kctx, ns, KW_StmtTypeDecl, SUGARFUNC_TopStmtTyCheck, new_SugarFunc(StmtTyCheck_GlobalTypeDecl));
	if(O_typeId(ns->scriptObject) == TY_System) {
		KDEFINE_CLASS defScript = {
			.structname = "Script",
			.typeId = TY_newid,
			.cflag = kClass_Singleton|kClass_Final,
			.cstruct_size = sizeof(kScript),
		};
		KonohaClass *cScript = KLIB Konoha_defineClass(kctx, ns->packageId, ns->packageDomain, NULL, &defScript, pline);
		KSETv(ns, ((kNameSpaceVar*)ns)->scriptObject, KLIB Knull(kctx, cScript));
	}
	return true;
}

static kbool_t global_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* global_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("global", "1.0"),
		.initPackage = global_initPackage,
		.setupPackage = global_setupPackage,
		.initNameSpace = global_initNameSpace,
		.setupNameSpace = global_setupNameSpace,
	};
	return &d;
}
