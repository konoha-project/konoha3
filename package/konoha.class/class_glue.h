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

#ifndef CLASS_GLUE_H_
#define CLASS_GLUE_H_

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>

static KMETHOD Fmethod_FieldGetter(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	size_t delta = sfp[K_MTDIDX].mtdNC->delta;
	RETURN_((sfp[0].toObject)->fieldObjectItems[delta]);
}
static KMETHOD Fmethod_FieldGetterN(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	size_t delta = sfp[K_MTDIDX].mtdNC->delta;
	RETURNd_((sfp[0].toObject)->fieldUnboxItems[delta]);
}
static KMETHOD Fmethod_FieldSetter(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	size_t delta = sfp[K_MTDIDX].mtdNC->delta;
	KSETv((sfp[0].toObjectVar)->fieldObjectItems[delta], sfp[1].toObject);
	RETURN_(sfp[1].toObject);
}
static KMETHOD Fmethod_FieldSetterN(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	size_t delta = sfp[K_MTDIDX].mtdNC->delta;
	(sfp[0].toObjectVar)->fieldUnboxItems[delta] = sfp[1].ndata;
	RETURNd_(sfp[1].ndata);
}

static kMethod *new_FieldGetter(KonohaContext *kctx, ktype_t cid, ksymbol_t sym, ktype_t ty, int idx)
{
	kmethodn_t mn = ty == TY_Boolean ? MN_toISBOOL(sym) : MN_toGETTER(sym);
	MethodFunc f = (TY_isUnbox(ty)) ? Fmethod_FieldGetterN : Fmethod_FieldGetter;
	kMethod *mtd = KLIB new_kMethod(kctx, kMethod_Public|kMethod_Immutable, cid, mn, f);
	KLIB kMethod_setParam(kctx, mtd, ty, 0, NULL);
	((kMethodVar*)mtd)->delta = idx;  // FIXME
	return mtd;
}

static kMethod *new_FieldSetter(KonohaContext *kctx, ktype_t cid, kmethodn_t sym, ktype_t ty, int idx)
{
	kmethodn_t mn = /*(ty == TY_Boolean) ? MN_toISBOOL(sym) :*/ MN_toSETTER(sym);
	MethodFunc f = (TY_isUnbox(ty)) ? Fmethod_FieldSetterN : Fmethod_FieldSetter;
	kparam_t p = {ty, FN_("x")};
	kMethod *mtd = KLIB new_kMethod(kctx, kMethod_Public, cid, mn, f);
	KLIB kMethod_setParam(kctx, mtd, ty, 1, &p);
	((kMethodVar*)mtd)->delta = idx;   // FIXME
	return mtd;
}

static intptr_t KLIB2_Method_indexOfField(kMethod *mtd)
{
	MethodFunc f = mtd->fcall_1;
	if(f== Fmethod_FieldGetter || f == Fmethod_FieldGetterN || f == Fmethod_FieldSetter || f == Fmethod_FieldSetterN) {
		return (intptr_t)mtd->delta;
	}
	return -1;
}

static void CT_addMethod(KonohaContext *kctx, KonohaClass *ct, kMethod *mtd)
{
	if(unlikely(ct->methodList == K_EMPTYARRAY)) {
		KINITv(((KonohaClassVar*)ct)->methodList, new_(MethodArray, 8));
	}
	KLIB kArray_add(kctx, ct->methodList, mtd);
}

static void KLIB2_setGetterSetter(KonohaContext *kctx, KonohaClass *ct)
{
	size_t i, fsize = ct->fsize;
	for(i=0; i < fsize; i++) {
		if(FLAG_is(ct->fieldItems[i].flag, kField_Getter)) {
			FLAG_unset(ct->fieldItems[i].flag, kField_Getter);
			kMethod *mtd = new_FieldGetter(kctx, ct->cid, ct->fieldItems[i].fn, ct->fieldItems[i].ty, i);
			CT_addMethod(kctx, ct, mtd);
		}
		if(FLAG_is(ct->fieldItems[i].flag, kField_Setter)) {
			FLAG_unset(ct->fieldItems[i].flag, kField_Setter);
			kMethod *mtd = new_FieldSetter(kctx, ct->cid, ct->fieldItems[i].fn, ct->fieldItems[i].ty, i);
			CT_addMethod(kctx, ct, mtd);
		}
	}
}

// --------------------------------------------------------------------------

// int NameSpace.getCid(String name, int defval)
static KMETHOD NameSpace_getCid(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	KonohaClass *ct = kNameSpace_getCT(sfp[0].toNameSpace, NULL/*fixme*/, S_text(sfp[1].toString), S_size(sfp[1].toString), (ktype_t)sfp[2].ivalue);
	kint_t cid = ct != NULL ? ct->cid : sfp[2].ivalue;
	RETURNi_(cid);
}

static void setfield(KonohaContext *kctx, KDEFINE_CLASS *ct, int fctsize, KonohaClass *supct)
{
	size_t fsize = supct->fsize + fctsize;
	ct->cstruct_size = fctsize * sizeof(kObject*); //size64((fsize * sizeof(void*)) + sizeof(KonohaObjectHeader));
	//DBG_P("supct->fsize=%d, fctsize=%d, cstruct_size=%d", supct->fsize, fctsize, ct->cstruct_size);
	if(fsize > 0) {
		ct->fields = (KonohaClassField*)KCALLOC(fsize, sizeof(KonohaClassField));
		ct->fsize = supct->fsize;
		ct->fallocsize = fsize;
		if(supct->fsize > 0) {
			memcpy(ct->fields, supct->fieldItems, sizeof(KonohaClassField)*ct->fsize);
		}
	}
}

static KonohaClass* defineClass(KonohaContext *kctx, kNameSpace *ns, kshortflag_t cflag, kString *name, KonohaClass *supct, int fsize, kfileline_t pline)
{
	KDEFINE_CLASS defNewClass = {
		.cflag  = cflag,
		.cid    = CLASS_newid,
		.bcid   = CLASS_Object,
		.supcid = supct->cid,
	};
	setfield(kctx, &defNewClass, fsize, supct);
	KonohaClass *ct = Konoha_addClassDef(ns->packageId, ns->packageDomain, name, &defNewClass, pline);
	ct->fnull(kctx, ct);  // create null object
	return ct;
}

// int NameSpace.defineClass(int flag, String name, int supcid, int fieldsize);
static KMETHOD NameSpace_defineClass(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	ktype_t supcid = sfp[3].ivalue == 0 ? TY_Object :(ktype_t)sfp[3].ivalue;
	KonohaClass *supct = kclass(supcid, sfp[K_RTNIDX].uline);
	if(CT_isFinal(supct)) {
		kreportf(CRIT_, sfp[K_RTNIDX].uline, "%s is final", TY_t(supcid));
	}
	if(!CT_isDefined(supct)) {
		kreportf(CRIT_, sfp[K_RTNIDX].uline, "%s has undefined field(s)", TY_t(supcid));
	}
	KonohaClass *ct = defineClass(kctx, sfp[0].toNameSpace, sfp[1].ivalue, sfp[2].s, supct, sfp[4].ivalue, sfp[K_RTNIDX].uline);
	RETURNi_(ct->cid);
}

static void defineField(KonohaContext *kctx, KonohaClassVar *ct, int flag, ktype_t ty, kString *name, kObject *value, uintptr_t uvalue)
{
	int pos = ct->fsize;
	ct->fsize += 1;
	ct->fieldItems[pos].flag = flag;
	ct->fieldItems[pos].ty = ty;
	ct->fieldItems[pos].fn = ksymbolA(S_text(name), S_size(name), SYM_NEWID);
	if(TY_isUnbox(ty)) {
		if(value != NULL) {
			ct->nulvalNULL_->fieldUnboxItems[pos] = O_unbox(value);
		}
		else {
			ct->nulvalNULL_->fieldUnboxItems[pos] = uvalue;
		}
	}
	else {
		kObject *v = (IS_NULL(value)) ? knull(O_ct(value)) : value;
		KSETv(ct->nulvalNULL_->fieldObjectItems[pos], v);
		ct->fieldItems[pos].isobj = 1;
	}
}

// int NameSpace.defineClassField(int cid, int flag, int ty, String name, Object *value);
static KMETHOD NameSpace_defineClassField(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	ktype_t cid = (ktype_t)sfp[1].ivalue;
	kshortflag_t flag = (kshortflag_t)sfp[2].ivalue;
	ktype_t ty = (ktype_t)sfp[3].ivalue;
	kString *name = sfp[4].s;
	kObject *value = sfp[5].o;
	KonohaClassVar *ct = (KonohaClassVar*)kclass(cid, sfp[K_RTNIDX].uline);
	if(CT_isDefined(ct)) {
		kreportf(CRIT_, sfp[K_RTNIDX].uline, "%s has no undefined field", TY_t(ct->cid));
	}
	defineField(kctx, ct, flag, ty, name, value, 0);
	if(CT_isDefined(ct)) {
		DBG_P("all fields are set");
		KLIB2_setGetterSetter(kctx, ct);
	}
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

static	kbool_t class_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	USING_SUGAR;
	int FN_flag = FN_("flag"), FN_cid = FN_("cid"), FN_name = FN_("name"), FN_defval = FN_("defval");
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(NameSpace_getCid), TY_Int, TY_NameSpace, MN_toGETTER(MN_("getCid")), 2, TY_String, FN_name, TY_Int, FN_defval,
		_Public, _F(NameSpace_defineClass), TY_Int, TY_NameSpace, MN_("defineClass"), 4, TY_Int, FN_flag, TY_String, FN_name, TY_Int, FN_("supcid"), TY_Int, FN_("fieldSize"),
		_Public, _F(NameSpace_defineClassField), TY_Int, TY_NameSpace, MN_("defineClassField"), 5, TY_Int, FN_cid, TY_Int, FN_flag, TY_Int, FN_("type"), TY_String, FN_name, TY_Object, FN_defval,
		DEND,
	};
	kNameSpace_loadMethodData(ns, MethodData);
	KSET_KLIB2(kMethod_indexOfField, KLIB2_Method_indexOfField, pline);
	return true;
}

static kbool_t class_setupPackage(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

// --------------------------------------------------------------------------

static kExpr* NewExpr(KonohaContext *kctx, SugarSyntax *syn, kToken *tk, ktype_t ty, uintptr_t val)
{
	kExprVar *expr = new_Var(Expr, syn);
	KSETv(expr->tk, tk);
	Expr_setTerm(expr, 1);
	expr->build = TEXPR_NEW;
	expr->ty = ty;
	expr->ndata = val;
	return (kExpr*)expr;
}

static KMETHOD ParseExpr_new(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	USING_SUGAR;
	VAR_ParseExpr(stmt, tls, s, c, e);
	DBG_ASSERT(s == c);
	kTokenVar *tkNEW = tls->tokenVarItems[s];
	if(s + 2 < kArray_size(tls)) {
		kToken *tk1 = tls->tokenItems[s+1];
		kToken *tk2 = tls->tokenItems[s+2];
		KonohaClass *ct = CT_(TK_type(tk1));
		if (ct->cid == CLASS_Tvoid) {
			RETURN_(SUGAR Stmt_p(kctx, stmt, tk1, ERR_, "undefined class: %s", S_text(tk1->text)));
		} else if (CT_isForward(ct)) {
			SUGAR Stmt_p(kctx, stmt, NULL, ERR_, "invalid application of 'new' to incomplete class %s", CT_t(ct));
		}

		if(TK_isType(tk1) && tk2->keyword == AST_PARENTHESIS) {  // new C (...)
			SugarSyntax *syn = SYN_(kStmt_nameSpace(stmt), KW_ExprMethodCall);
			kExpr *expr = SUGAR new_ConsExpr(kctx, syn, 2, tkNEW, NewExpr(kctx, syn, tk1, TK_type(tk1), 0));
			tkNEW->keyword = MN_new;
			RETURN_(expr);
		}
		if(TK_isType(tk1) && tk2->keyword == AST_BRACKET) {     // new C [...]
			SugarSyntax *syn = SYN_(kStmt_nameSpace(stmt), KW_new);
			KonohaClass *ct = CT_p0(kctx, CT_Array, TK_type(tk1));
			tkNEW->keyword = MN_("newArray");
			kExpr *expr = SUGAR new_ConsExpr(kctx, syn, 2, tkNEW, NewExpr(kctx, syn, tk1, ct->cid, 0));
			RETURN_(expr);
		}
	}
}

static ksymbol_t tosymbolUM(KonohaContext *kctx, kToken *tk)
{
	DBG_ASSERT(tk->keyword == TK_SYMBOL);
	return ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NEWID);
}

static KMETHOD ExprTyCheck_Getter(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	USING_SUGAR;
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kToken *tkN = expr->cons->tokenItems[0];
	ksymbol_t fn = tosymbolUM(kctx, tkN);
	kExpr *self = SUGAR Expr_tyCheckAt(kctx, stmt, expr, 1, gma, TY_var, 0);
	if(self != K_NULLEXPR) {
		kMethod *mtd = kNameSpace_getMethodNULL(gma->genv->ns, self->ty, MN_toGETTER(fn));
		if(mtd == NULL) {
			mtd = kNameSpace_getMethodNULL(gma->genv->ns, self->ty, MN_toISBOOL(fn));
		}
		if(mtd != NULL) {
			KSETv(expr->cons->methodItems[0], mtd);
			RETURN_(SUGAR Expr_tyCheckCallParams(kctx, stmt, expr, mtd, gma, reqty));
		}
		SUGAR Stmt_p(kctx, stmt, tkN, ERR_, "undefined field: %s", S_text(tkN->text));
	}
	RETURN_(K_NULLEXPR);
}

// ----------------------------------------------------------------------------

static void Stmt_parseClassBlock(KonohaContext *kctx, kStmt *stmt, kToken *tkC)
{
	USING_SUGAR;
	kToken *tkP = (kToken*)kStmt_getObject(kctx, stmt, KW_BlockPattern, NULL);
	if(tkP != NULL && tkP->keyword == TK_CODE) {
		kArray *a = ctxsugar->preparedTokenList;
		size_t atop = kArray_size(a), s, i;
		SUGAR NameSpace_tokenize(kctx, kStmt_nameSpace(stmt), S_text(tkP->text), tkP->uline, a);
		s = kArray_size(a);
		const char *cname = S_text(tkC->text);
		for(i = atop; i < s; i++) {
			kToken *tk = a->tokenItems[i];
			int topch = kToken_topch(tk);
			DBG_P("cname='%s'", cname);
			if(topch == '(' && tkP->keyword == TK_SYMBOL && strcmp(cname, S_text(tkP->text)) == 0) {
				kTokenVar *tkNEW = new_Var(Token, 0);
				tkNEW->keyword = TK_SYMBOL;
				KSETv(tkNEW->text, SYM_s(MN_new));
				tkNEW->uline = tkP->uline;
				KLIB kArray_add(kctx, a, tkNEW);
			}
			KLIB kArray_add(kctx, a, tk);
			tkP = tk;
		}
		kBlock *bk = SUGAR new_Block(kctx, kStmt_nameSpace(stmt), stmt, a, s, kArray_size(a), ';');
		for (i = 0; i < kArray_size(bk->stmtList); i++) {
			kStmt *methodDecl = bk->stmtList->stmtItems[i];
			if(methodDecl->syn->keyword == KW_StmtMethodDecl) {
				KLIB kObject_setObject(kctx, methodDecl, KW_UsymbolPattern, TY_Token, tkC);
			}
		}
		KLIB kObject_setObject(kctx, stmt, KW_BlockPattern, TY_Block, bk);
		KLIB kArray_clear(kctx, a, atop);
	}
}


// ----------------------------------------------------------------------------

typedef struct {
	const char *key;
	uintptr_t ty;
	KonohaClass *ct;
} KDEFINE_CLASS_CONST;

static void ObjectField_init(KonohaContext *kctx, kObject *o, void *conf)
{
	KonohaClass *ct = O_ct(o);
	DBG_ASSERT(ct->nulvalNULL != NULL);
	size_t fsize = ct->fsize;
	memcpy(((kObjectVar *)o)->fieldObjectItems, ct->nulvalNULL->fieldObjectItems, fsize * sizeof(void*));
}

extern kObjectVar** KONOHA_reftail(KonohaContext *kctx, size_t size);

static void ObjectField_reftrace (KonohaContext *kctx, kObject *o)
{
	KonohaClass *ct =O_ct(o);
	KonohaClassField *fieldItems = ct->fieldItems;
	size_t i, fsize = ct->fsize;
	BEGIN_REFTRACE(fsize);
	for (i = 0; i < fsize; i++) {
		if (fieldItems[i].isobj) {
			KREFTRACEn(o->fieldObjectItems[i]);
		}
	}
	END_REFTRACE();
}

static KonohaClassVar* defineClassName(KonohaContext *kctx, kNameSpace *ns, kshortflag_t cflag, kString *name, ktype_t supcid, kfileline_t pline)
{
	KDEFINE_CLASS defNewClass = {
		.cflag  = cflag,
		.cid    = CLASS_newid,
		.bcid   = CLASS_Object,
		.supcid = supcid,
//		.init   = ObjectField_init,
	};
	KonohaClass *ct = Konoha_addClassDef(ns->packageId, ns->packageDomain, name, &defNewClass, pline);
	KDEFINE_CLASS_CONST ClassData[] = {
		{S_text(name), TY_TYPE, ct},
		{NULL},
	};
	kNameSpace_loadConstData(ns, ClassData, 0); // add class name to this namespace
//	kMethod *mtd = KLIB new_kMethod(kctx, _Public/*flag*/, ct->cid, MN_new, NULL);
//	KLIB kMethod_setParam(kctx, mtd, ct->cid, 0, NULL);
//	CT_addMethod(kctx, ct, mtd);
	return (KonohaClassVar*)ct;
}

static size_t checkFieldSize(KonohaContext *kctx, kBlock *bk)
{
	USING_SUGAR;
	size_t i, c = 0;
	for(i = 0; i < kArray_size(bk->stmtList); i++) {
		kStmt *stmt = bk->stmtList->stmtItems[i];
		DBG_P("stmt->keyword=%s", KW_t(stmt->syn->keyword));
		if(stmt->syn->keyword == KW_StmtTypeDecl) {
			kExpr *expr = kStmt_expr(stmt, KW_ExprPattern, NULL);
			if(expr->syn->keyword == KW_COMMA) {
				c += (kArray_size(expr->cons) - 1);
			}
			else if(expr->syn->keyword == KW_LET || Expr_isTerm(expr)) {
				c++;
			}
		}
	}
	return c;
}

static void CT_setField(KonohaContext *kctx, KonohaClassVar *ct, KonohaClass *supct, int fctsize)
{
	size_t fsize = supct->fsize + fctsize;
	ct->fieldItems = (KonohaClassField*)KCALLOC(fsize, sizeof(KonohaClassField));
	ct->fsize = supct->fsize;
	ct->fallocsize = fsize;
	if(supct->fsize > 0) {
		memcpy(ct->fieldItems, supct->fieldItems, sizeof(KonohaClassField)*supct->fsize);
		memcpy(ct->nulvalNULL_, supct->nulvalNULL_, sizeof(kObject*) * supct->fsize);
	}
}

static void CT_initField(KonohaContext *kctx, KonohaClassVar *ct, KonohaClass *supct, int fctsize)
{
	size_t fsize = supct->fsize + fctsize;
	ct->cstruct_size = size64(fctsize * sizeof(kObject*) + sizeof(KonohaObjectHeader));
	DBG_P("supct->fsize=%d, fctsize=%d, cstruct_size=%d", supct->fsize, fctsize, ct->cstruct_size);
	if(fsize > 0) {
		ct->fnull(kctx, ct);
		ct->init = ObjectField_init;
		ct->reftrace = ObjectField_reftrace;
		CT_setField(kctx, ct, supct, fctsize);
	}
}

static kbool_t CT_declType(KonohaContext *kctx, KonohaClassVar *ct, kGamma *gma, kStmt *stmt, kExpr *expr, kshortflag_t flag, ktype_t ty, kfileline_t pline)
{
	USING_SUGAR;
	if(Expr_isTerm(expr)) {
		kString *name = expr->tk->text;
		defineField(kctx, ct, flag, ty, name, knull(CT_(ty)), 0);
		return true;
	}
	else if(expr->syn->keyword == KW_LET) {
		kExpr *lexpr = kExpr_at(expr, 1);
		if(Expr_isTerm(lexpr)) {
			kExpr *vexpr = SUGAR Expr_tyCheckAt(kctx, stmt, expr, 2, gma, ty, 0);
			if(vexpr == K_NULLEXPR) {
				return false;
			}
			kString *name = expr->tk->text;
			if(vexpr->build == TEXPR_CONST) {
				defineField(kctx, ct, flag, ty, name, vexpr->data, 0);
			}
			else if(vexpr->build == TEXPR_NCONST) {
				defineField(kctx, ct, flag, ty, name, NULL, vexpr->ndata);
			}
			else if(vexpr->build == TEXPR_NULL) {
				defineField(kctx, ct, flag, ty, name, knull(CT_(ty)), 0);
			}
			else {
				SUGAR Stmt_p(kctx, stmt, NULL, ERR_, "const value is expected as the field initial value: %s", S_text(name));
				return false;
			}
			return true;
		}
	} else if(expr->syn->keyword == KW_COMMA) {
		size_t i;
		for(i = 1; i < kArray_size(expr->cons); i++) {
			if(!CT_declType(kctx, ct, gma, stmt, kExpr_at(expr, i), flag, ty, pline)) return false;
		}
		return true;
	}
	SUGAR Stmt_p(kctx, stmt, NULL, ERR_, "field name is expected");
	return false;
}

static kbool_t CT_addClassFields(KonohaContext *kctx, KonohaClassVar *ct, kGamma *gma, kBlock *bk, kfileline_t pline)
{
	USING_SUGAR;
	size_t i;
	for(i = 0; i < kArray_size(bk->stmtList); i++) {
		kStmt *stmt = bk->stmtList->stmtItems[i];
		if(stmt->syn->keyword == KW_StmtTypeDecl) {
			kshortflag_t flag = kField_Getter | kField_Setter;
			kToken *tk  = kStmt_token(stmt, KW_TypePattern, NULL);
			kExpr *expr = kStmt_expr(stmt, KW_ExprPattern, NULL);
			if(!CT_declType(kctx, ct, gma, stmt, expr, flag, TK_type(tk), pline)) {
				return false;
			}
		}
	}
	DBG_ASSERT(ct->fsize == ct->fallocsize);
	DBG_P("all fields are set");
	KLIB2_setGetterSetter(kctx, ct);
	return true;
}

static void CT_checkMethodDecl(KonohaContext *kctx, kToken *tkC, kBlock *bk, kStmt **lastStmtRef)
{
	USING_SUGAR;
	size_t i;
	for(i = 0; i < kArray_size(bk->stmtList); i++) {
		kStmt *stmt = bk->stmtList->stmtItems[i];
		if(stmt->syn->keyword == KW_StmtTypeDecl) continue;
		if(stmt->syn->keyword == KW_StmtMethodDecl) {
			kStmt *lastStmt = lastStmtRef[0];
			SUGAR Block_insertAfter(kctx, lastStmt->parentBlockNULL, lastStmt, stmt);
			lastStmtRef[0] = stmt;
		}
		else {
			SUGAR Stmt_p(kctx, stmt, NULL, WARN_, "%s is not available within the class clause", KW_t(stmt->syn->keyword));
		}
	}
}

static KMETHOD StmtTyCheck_class(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	USING_SUGAR;
	VAR_StmtTyCheck(stmt, gma);
	kToken *tkC = kStmt_token(stmt, KW_UsymbolPattern, NULL);
	kToken *tkE= kStmt_token(stmt, SYM_("extends"), NULL);
	kshortflag_t cflag = 0;
	ktype_t supcid = TY_Object;
	KonohaClass *supct = CT_Object;
	if (tkE) {
		supcid = TK_type(tkE);
		supct = CT_(supcid);
		if(CT_isFinal(supct)) {
			SUGAR Stmt_p(kctx, stmt, NULL, ERR_, "%s is final", CT_t(supct));
			RETURNb_(false);
		}
		if(!CT_isDefined(supct)) {
			SUGAR Stmt_p(kctx, stmt, NULL, ERR_, "%s has undefined field(s)", CT_t(supct));
			RETURNb_(false);
		}
	}
	KonohaClassVar *ct = (KonohaClassVar*)kNameSpace_getCT(gma->genv->ns, NULL/*FIXME*/, S_text(tkC->text), S_size(tkC->text), TY_unknown);
	if (ct != NULL) {
		if (!CT_isForward(ct)) {
			SUGAR Stmt_p(kctx, stmt, NULL, ERR_, "%s is already defined", CT_t(ct));
			RETURNb_(false);
		}
	} else {
		ct = defineClassName(kctx, gma->genv->ns, cflag, tkC->text, supcid, stmt->uline);
	}
	((kTokenVar*)tkC)->keyword = KW_TypePattern;
	((kTokenVar*)tkC)->ty = ct->cid;
	Stmt_parseClassBlock(kctx, stmt, tkC);
	kBlock *bk = kStmt_block(stmt, KW_BlockPattern, K_NULLBLOCK);
	if (ct->nulvalNULL == NULL) {
		/* ct is created at this time */
		CT_initField(kctx, ct, supct, checkFieldSize(kctx, bk));
	} else {
		size_t fsize = checkFieldSize(kctx, bk);
		CT_setField(kctx, ct, supct, fsize);
	}
	if (bk == K_NULLBLOCK) {
		/* forward declaration, do nothing */
		FLAG_set(ct->cflag, kClass_Forward);
	} else {
		FLAG_unset(ct->cflag, kClass_Forward);
		if(!CT_addClassFields(kctx, ct, gma, bk, stmt->uline)) {
			RETURNb_(false);
		}
	}
	kStmt_done(stmt);
	CT_checkMethodDecl(kctx, tkC, bk, &stmt);
	RETURNb_(true);
}

static kbool_t class_initNameSpace(KonohaContext *kctx,  kNameSpace *ns, kfileline_t pline)
{
	USING_SUGAR;
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("new"), ParseExpr_(new), },
		{ .keyword = SYM_("class"), .rule = "\"class\" $USYMBOL [\"extends\" extends: $type] [$block]", TopStmtTyCheck_(class), },
		{ .keyword = SYM_("extends"), .rule = "\"extends\" $type", },
		{ .keyword = SYM_("."), ExprTyCheck_(Getter) },
		{ .keyword = KW_END, },
	};
	SUGAR NameSpace_defineSyntax(kctx, ns, SYNTAX);
	return true;
}

static kbool_t class_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

#endif /* CLASS_GLUE_H_ */
