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

#ifndef GLOBAL_GLUE_H_
#define GLOBAL_GLUE_H_

// --------------------------------------------------------------------------

static	kbool_t global_initPackage(KonohaContext *kctx, kNameSpace *ks, int argc, const char**args, kline_t pline)
{
	return true;
}

static kbool_t global_setupPackage(KonohaContext *kctx, kNameSpace *ks, kline_t pline)
{
	return true;
}

// --------------------------------------------------------------------------

static KMETHOD Fmethod_ProtoGetter(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	kMethod *mtd = sfp[K_MTDIDX].mtdNC;
	ksymbol_t key = (ksymbol_t)mtd->delta;
	RETURN_(kObject_getObject(sfp[0].o, key, sfp[K_RTNIDX].o));
}

static KMETHOD Fmethod_ProtoGetterN(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	kMethod *mtd = sfp[K_MTDIDX].mtdNC;
	ksymbol_t key = (ksymbol_t)mtd->delta;
	RETURNd_(kObject_getUnboxedValue(sfp[0].o, key, 0));
}

static KMETHOD Fmethod_ProtoSetter(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	kMethod *mtd = sfp[K_MTDIDX].mtdNC;
	ksymbol_t key = (ksymbol_t)mtd->delta;
	kObject_setObject(sfp[0].o, key, sfp[1].o);
	RETURN_(sfp[1].o);
}

static KMETHOD Fmethod_ProtoSetterN(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	kMethod *mtd = sfp[K_MTDIDX].mtdNC;
	ksymbol_t key = (ksymbol_t)mtd->delta;
	kParam *pa = kMethod_param(mtd);
	kObject_setUnboxedValue(sfp[0].o, key, pa->p[0].ty, sfp[1].ndata);
	RETURNd_(sfp[1].ndata);
}

static kMethod *new_ProtoGetter(KonohaContext *kctx, ktype_t cid, ksymbol_t sym, ktype_t ty)
{
	kmethodn_t mn = ty == TY_Boolean ? MN_toISBOOL(sym) : MN_toGETTER(sym);
	knh_Fmethod f = (TY_isUnbox(ty)) ? Fmethod_ProtoGetterN : Fmethod_ProtoGetter;
	kMethod *mtd = new_kMethod(kMethod_Public|kMethod_Immutable, cid, mn, f);
	kMethod_setParam(mtd, ty, 0, NULL);
	((struct _kMethod*)mtd)->delta = sym;
	return mtd;
}

static kMethod *new_ProtoSetter(KonohaContext *kctx, ktype_t cid, ksymbol_t sym, ktype_t ty)
{
	kmethodn_t mn = MN_toSETTER(sym);
	knh_Fmethod f = (TY_isUnbox(ty)) ? Fmethod_ProtoSetterN : Fmethod_ProtoSetter;
	kparam_t p = {ty, FN_("x")};
	kMethod *mtd = new_kMethod(kMethod_Public, cid, mn, f);
	kMethod_setParam(mtd, ty, 1, &p);
	((struct _kMethod*)mtd)->delta = sym;
	return mtd;
}

static void CT_addMethod2(KonohaContext *kctx, kclass_t *ct, kMethod *mtd)
{
	if(unlikely(ct->methods == K_EMPTYARRAY)) {
		KINITv(((struct _kclass*)ct)->methods, new_(MethodArray, 8));
	}
	kArray_add(ct->methods, mtd);
}

static kMethod *Object_newProtoSetterNULL(KonohaContext *kctx, kObject *o, kStmt *stmt, kNameSpace *ks, ktype_t ty, ksymbol_t fn)
{
	USING_SUGAR;
	ktype_t cid = O_cid(o);
	kMethod *mtd = kNameSpace_getMethodNULL(ks, cid, MN_toSETTER(fn));
	if(mtd != NULL) {
		SUGAR Stmt_p(kctx, stmt, NULL, ERR_, "already defined name: %s.%s", CT_t(O_ct(o)), SYM_t(fn));
		return NULL;
	}
	mtd = kNameSpace_getMethodNULL(ks, cid, MN_toGETTER(fn));
	if(mtd == NULL) {
		mtd = kNameSpace_getMethodNULL(ks, cid, MN_toISBOOL(fn));
	}
	if(mtd != NULL && kMethod_rtype(mtd) != ty) {
		SUGAR Stmt_p(kctx, stmt, NULL, ERR_, "differently defined getter: %s.%s", CT_t(O_ct(o)), SYM_t(fn));
		return NULL;
	}
	if(mtd == NULL) { // no getter
		CT_addMethod2(kctx, O_ct(o), new_ProtoGetter(kctx, cid, fn, ty));
	}
	mtd = new_ProtoSetter(kctx, cid, fn, ty);
	CT_addMethod2(kctx, O_ct(o), mtd);
	return mtd;
}

static ksymbol_t tosymbol(KonohaContext *kctx, kExpr *expr)
{
	if(Expr_isTerm(expr)) {
		kToken *tk = expr->tk;
		if(tk->kw == TK_SYMBOL) {
			return ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NEWID);
		}
	}
	return SYM_NONAME;
}

static KMETHOD StmtTyCheck_var(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	USING_SUGAR;
	VAR_StmtTyCheck(stmt, gma);
	DBG_P("global assignment .. ");
	kObject *scr = gma->genv->ks->scrobj;
	if(O_cid(scr) == TY_System) {
		SUGAR Stmt_p(kctx, stmt, NULL, ERR_, " global variables are not available");
		RETURNb_(false);
	}
	kExpr *vexpr = kStmt_expr(stmt, SYM_("var"), K_NULLEXPR);
	ksymbol_t fn = tosymbol(kctx, vexpr);
	if(fn == SYM_NONAME) {
		SUGAR Stmt_p(kctx, stmt, NULL, ERR_, "variable name is expected");
		RETURNb_(false);
	}
	kExpr *expr = kStmt_expr(stmt, KW_ExprPattern, K_NULLEXPR);
	if(!SUGAR Stmt_tyCheckExpr(kctx, stmt, KW_ExprPattern, gma, TY_var, 0)) {
		RETURNb_(false);
	}
	/*kExpr **/expr = kStmt_expr(stmt, KW_ExprPattern, K_NULLEXPR);
	kMethod *mtd = Object_newProtoSetterNULL(kctx, scr, stmt, gma->genv->ks, expr->ty, fn);
	if(mtd == NULL) {
		RETURNb_(false);
	}
	SUGAR Stmt_p(kctx, stmt, NULL, INFO_, "%s has type %s", SYM_t(fn), TY_t(expr->ty));
	expr = SUGAR new_TypedMethodCall(kctx, stmt, TY_void, mtd, gma, 2, new_ConstValue(O_cid(scr), scr), expr);
	kObject_setObject(stmt, KW_ExprPattern, expr);
	kStmt_typed(stmt, EXPR);
	RETURNb_(true);
}

// ---------------------------------------------------------------------------

static kMethod* ExprTerm_getSetterNULL(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kObject *scr, kGamma *gma, ktype_t ty)
{
	USING_SUGAR;
	if(Expr_isTerm(expr) && expr->tk->kw == TK_SYMBOL) {
		kToken *tk = expr->tk;
		if(tk->kw != KW_SymbolPattern) {
			SUGAR Stmt_p(kctx, stmt, NULL, ERR_, "%s is keyword", S_text(tk->text));
			return NULL;
		}
		ksymbol_t fn = ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NEWID);
		return Object_newProtoSetterNULL(kctx, scr, stmt, gma->genv->ks, ty, fn);
	}
	SUGAR Stmt_p(kctx, stmt, NULL, ERR_, "variable name is expected");
	return NULL;
}

static kbool_t appendSetterStmt(KonohaContext *kctx, kExpr *expr, kStmt **lastStmtRef)
{
	USING_SUGAR;
	kStmt *lastStmt = lastStmtRef[0];
	kStmt *newstmt = new_(Stmt, lastStmt->uline);
	SUGAR Block_insertAfter(kctx, lastStmt->parentNULL, lastStmt, newstmt);
	kStmt_setsyn(newstmt, SYN_(kStmt_ks(newstmt), KW_ExprPattern));
	kObject_setObject(newstmt, KW_ExprPattern, expr);
	lastStmtRef[0] = newstmt;
	return true;
}

static kbool_t Expr_declType(KonohaContext *kctx, kStmt *stmt, kExpr *expr, kGamma *gma, ktype_t ty, kStmt **lastStmtRef)
{
	USING_SUGAR;
	kObject *scr = gma->genv->ks->scrobj;
	if(O_cid(scr) == TY_System) {
		SUGAR Stmt_p(kctx, stmt, NULL, ERR_, " global variables are not available");
		return false;
	}
	if(Expr_isTerm(expr)) {
		kMethod *mtd = ExprTerm_getSetterNULL(kctx, stmt, expr, scr, gma, ty);
		if(mtd != NULL) {
			kExpr *vexpr = new_Variable(NULL, ty, 0, gma);
			expr = SUGAR new_TypedMethodCall(kctx, stmt, TY_void, mtd, gma, 2, new_ConstValue(O_cid(scr), scr), vexpr);
			PUSH_GCSTACK(expr);
			return appendSetterStmt(kctx, expr, lastStmtRef);
		}
		return false;
	}
	else if(expr->syn->kw == KW_LET) {
		kExpr *lexpr = kExpr_at(expr, 1);
		if(SUGAR Expr_tyCheckAt(kctx, stmt, expr, 2, gma, ty, 0) == K_NULLEXPR) {
			// this is neccesarry to avoid 'int a = a + 1;';
			return false;
		}
		kMethod *mtd = ExprTerm_getSetterNULL(kctx, stmt, lexpr, scr, gma, ty);
		if(mtd != NULL) {
			expr = SUGAR new_TypedMethodCall(kctx, stmt, TY_void, mtd, gma, 2, new_ConstValue(O_cid(scr), scr), kExpr_at(expr, 2));
			PUSH_GCSTACK(expr);
			return appendSetterStmt(kctx, expr, lastStmtRef);
		}
		return false;
	} else if(expr->syn->kw == KW_COMMA) {
		size_t i;
		for(i = 1; i < kArray_size(expr->cons); i++) {
			if(!Expr_declType(kctx, stmt, kExpr_at(expr, i), gma, ty, lastStmtRef)) return false;
		}
		return true;
	}
	SUGAR Stmt_p(kctx, stmt, NULL, ERR_, "variable name is expected");
	return false;
}

static KMETHOD StmtTyCheck_GlobalTypeDecl(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	USING_SUGAR;
	VAR_StmtTyCheck(stmt, gma);
	kToken *tk  = kStmt_token(stmt, KW_TypePattern, NULL);
	kExpr  *expr = kStmt_expr(stmt, KW_ExprPattern, NULL);
	if(tk == NULL || !TK_isType(tk) || expr == NULL) {
		RETURNb_(false);
	}
	kStmt_done(stmt);
	RETURNb_(Expr_declType(kctx, stmt, expr, gma, TK_type(tk), &stmt));
}

typedef const struct _kScript kScript;

struct _kScript {
	kObjectHeader h;
};

static kbool_t global_initNameSpace(KonohaContext *kctx,  kNameSpace *ks, kline_t pline)
{
	USING_SUGAR;
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .kw = SYM_("var"), TopStmtTyCheck_(var), .rule = "\"var\" var: $expr \"=\" $expr", },
		{ .kw = KW_END, },
	};
	SUGAR NameSpace_defineSyntax(kctx, ks, SYNTAX);
	SUGAR SYN_setSugarFunc(kctx, ks, KW_StmtTypeDecl, SYNIDX_TopStmtTyCheck, new_SugarFunc(StmtTyCheck_GlobalTypeDecl));
	if(O_cid(ks->scrobj) == TY_System) {
		KDEFINE_CLASS defScript = {
			.structname = "Script",
			.cid = CLASS_newid,
			.cflag = kClass_Singleton|kClass_Final,
			.cstruct_size = sizeof(kScript),
		};
		kclass_t *cScript = Konoha_addClassDef(ks->packid, ks->packdom, NULL, &defScript, pline);
		KSETv(((struct _kNameSpace*)ks)->scrobj, knull(cScript));
	}
	return true;
}

static kbool_t global_setupNameSpace(KonohaContext *kctx, kNameSpace *ks, kline_t pline)
{
	return true;
}


#endif /* ASSIGNMENT_GLUE_H_ */
