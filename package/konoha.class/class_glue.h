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

#include <konoha2/konoha2.h>
#include <konoha2/sugar.h>

static KMETHOD Fmethod_FieldGetter(CTX, ksfp_t *sfp _RIX)
{
	size_t delta = sfp[K_MTDIDX].mtdNC->delta;
	RETURN_((sfp[0].o)->fields[delta]);
}
static KMETHOD Fmethod_FieldGetterN(CTX, ksfp_t *sfp _RIX)
{
	size_t delta = sfp[K_MTDIDX].mtdNC->delta;
	RETURNd_((sfp[0].o)->ndata[delta]);
}
static KMETHOD Fmethod_FieldSetter(CTX, ksfp_t *sfp _RIX)
{
	size_t delta = sfp[K_MTDIDX].mtdNC->delta;
	KSETv((sfp[0].Wo)->fields[delta], sfp[1].o);
	RETURN_(sfp[1].o);
}
static KMETHOD Fmethod_FieldSetterN(CTX, ksfp_t *sfp _RIX)
{
	size_t delta = sfp[K_MTDIDX].mtdNC->delta;
	(sfp[0].Wo)->ndata[delta] = sfp[1].ndata;
	RETURNd_(sfp[1].ndata);
}

static kMethod *new_FieldGetter(CTX, kcid_t cid, ksymbol_t sym, ktype_t ty, int idx)
{
	kmethodn_t mn = ty == TY_Boolean ? MN_toISBOOL(sym) : MN_toGETTER(sym);
	knh_Fmethod f = (TY_isUnbox(ty)) ? Fmethod_FieldGetterN : Fmethod_FieldGetter;
	kMethod *mtd = new_kMethod(kMethod_Public|kMethod_Immutable, cid, mn, f);
	kMethod_setParam(mtd, ty, 0, NULL);
	((struct _kMethod*)mtd)->delta = idx;  // FIXME
	return mtd;
}

static kMethod *new_FieldSetter(CTX, kcid_t cid, kmethodn_t sym, ktype_t ty, int idx)
{
	kmethodn_t mn = /*(ty == TY_Boolean) ? MN_toISBOOL(sym) :*/ MN_toSETTER(sym);
	knh_Fmethod f = (TY_isUnbox(ty)) ? Fmethod_FieldSetterN : Fmethod_FieldSetter;
	kparam_t p = {ty, FN_("x")};
	kMethod *mtd = new_kMethod(kMethod_Public, cid, mn, f);
	kMethod_setParam(mtd, ty, 1, &p);
	((struct _kMethod*)mtd)->delta = idx;   // FIXME
	return mtd;
}

static intptr_t KLIB2_Method_indexOfField(kMethod *mtd)
{
	knh_Fmethod f = mtd->fcall_1;
	if(f== Fmethod_FieldGetter || f == Fmethod_FieldGetterN || f == Fmethod_FieldSetter || f == Fmethod_FieldSetterN) {
		return (intptr_t)mtd->delta;
	}
	return -1;
}

static void CT_addMethod(CTX, kclass_t *ct, kMethod *mtd)
{
	if(unlikely(ct->methods == K_EMPTYARRAY)) {
		KINITv(((struct _kclass*)ct)->methods, new_(MethodArray, 8));
	}
	kArray_add(ct->methods, mtd);
}

static void KLIB2_setGetterSetter(CTX, kclass_t *ct)
{
	size_t i, fsize = ct->fsize;
	for(i=0; i < fsize; i++) {
		if(FLAG_is(ct->fields[i].flag, kField_Getter)) {
			FLAG_unset(ct->fields[i].flag, kField_Getter);
			kMethod *mtd = new_FieldGetter(_ctx, ct->cid, ct->fields[i].fn, ct->fields[i].ty, i);
			CT_addMethod(_ctx, ct, mtd);
		}
		if(FLAG_is(ct->fields[i].flag, kField_Setter)) {
			FLAG_unset(ct->fields[i].flag, kField_Setter);
			kMethod *mtd = new_FieldSetter(_ctx, ct->cid, ct->fields[i].fn, ct->fields[i].ty, i);
			CT_addMethod(_ctx, ct, mtd);
		}
	}
}

// --------------------------------------------------------------------------

// int NameSpace.getCid(String name, int defval)
static KMETHOD NameSpace_getCid(CTX, ksfp_t *sfp _RIX)
{
	kclass_t *ct = kNameSpace_getCT(sfp[0].ks, NULL/*fixme*/, S_text(sfp[1].s), S_size(sfp[1].s), (kcid_t)sfp[2].ivalue);
	kint_t cid = ct != NULL ? ct->cid : sfp[2].ivalue;
	RETURNi_(cid);
}

static void setfield(CTX, KDEFINE_CLASS *ct, int fctsize, kclass_t *supct)
{
	size_t fsize = supct->fsize + fctsize;
	ct->cstruct_size = fctsize * sizeof(kObject*); //size64((fsize * sizeof(void*)) + sizeof(kObjectHeader));
	//DBG_P("supct->fsize=%d, fctsize=%d, cstruct_size=%d", supct->fsize, fctsize, ct->cstruct_size);
	if(fsize > 0) {
		ct->fields = (kfield_t*)KCALLOC(fsize, sizeof(kfield_t));
		ct->fsize = supct->fsize;
		ct->fallocsize = fsize;
		if(supct->fsize > 0) {
			memcpy(ct->fields, supct->fields, sizeof(kfield_t)*ct->fsize);
		}
	}
}

static kclass_t* defineClass(CTX, kNameSpace *ks, kflag_t cflag, kString *name, kclass_t *supct, int fsize, kline_t pline)
{
	KDEFINE_CLASS defNewClass = {
		.cflag  = cflag,
		.cid    = CLASS_newid,
		.bcid   = CLASS_Object,
		.supcid = supct->cid,
	};
	setfield(_ctx, &defNewClass, fsize, supct);
	kclass_t *ct = Konoha_addClassDef(ks->packid, ks->packdom, name, &defNewClass, pline);
	ct->fnull(_ctx, ct);  // create null object
	return ct;
}

// int NameSpace.defineClass(int flag, String name, int supcid, int fieldsize);
static KMETHOD NameSpace_defineClass(CTX, ksfp_t *sfp _RIX)
{
	ktype_t supcid = sfp[3].ivalue == 0 ? TY_Object :(ktype_t)sfp[3].ivalue;
	kclass_t *supct = kclass(supcid, sfp[K_RTNIDX].uline);
	if(CT_isFinal(supct)) {
		kreportf(CRIT_, sfp[K_RTNIDX].uline, "%s is final", TY_t(supcid));
	}
	if(!CT_isDefined(supct)) {
		kreportf(CRIT_, sfp[K_RTNIDX].uline, "%s has undefined field(s)", TY_t(supcid));
	}
	kclass_t *ct = defineClass(_ctx, sfp[0].ks, sfp[1].ivalue, sfp[2].s, supct, sfp[4].ivalue, sfp[K_RTNIDX].uline);
	RETURNi_(ct->cid);
}

static void defineField(CTX, struct _kclass *ct, int flag, ktype_t ty, kString *name, kObject *value, uintptr_t uvalue)
{
	int pos = ct->fsize;
	ct->fsize += 1;
	ct->fields[pos].flag = flag;
	ct->fields[pos].ty = ty;
	ct->fields[pos].fn = ksymbolA(S_text(name), S_size(name), SYM_NEWID);
	if(TY_isUnbox(ty)) {
		if(value != NULL) {
			ct->WnulvalNUL->ndata[pos] = O_unbox(value);
		}
		else {
			ct->WnulvalNUL->ndata[pos] = uvalue;
		}
	}
	else {
		kObject *v = (IS_NULL(value)) ? knull(O_ct(value)) : value;
		KSETv(ct->WnulvalNUL->fields[pos], v);
		ct->fields[pos].isobj = 1;
	}
}

// int NameSpace.defineClassField(int cid, int flag, int ty, String name, Object *value);
static KMETHOD NameSpace_defineClassField(CTX, ksfp_t *sfp _RIX)
{
	kcid_t cid = (kcid_t)sfp[1].ivalue;
	kflag_t flag = (kflag_t)sfp[2].ivalue;
	ktype_t ty = (ktype_t)sfp[3].ivalue;
	kString *name = sfp[4].s;
	kObject *value = sfp[5].o;
	struct _kclass *ct = (struct _kclass*)kclass(cid, sfp[K_RTNIDX].uline);
	if(CT_isDefined(ct)) {
		kreportf(CRIT_, sfp[K_RTNIDX].uline, "%s has no undefined field", TY_t(ct->cid));
	}
	defineField(_ctx, ct, flag, ty, name, value, 0);
	if(CT_isDefined(ct)) {
		DBG_P("all fields are set");
		KLIB2_setGetterSetter(_ctx, ct);
	}
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

static	kbool_t class_initPackage(CTX, kNameSpace *ks, int argc, const char**args, kline_t pline)
{
	USING_SUGAR;
	int FN_flag = FN_("flag"), FN_cid = FN_("cid"), FN_name = FN_("name"), FN_defval = FN_("defval");
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(NameSpace_getCid), TY_Int, TY_NameSpace, MN_toGETTER(MN_("getCid")), 2, TY_String, FN_name, TY_Int, FN_defval,
		_Public, _F(NameSpace_defineClass), TY_Int, TY_NameSpace, MN_("defineClass"), 4, TY_Int, FN_flag, TY_String, FN_name, TY_Int, FN_("supcid"), TY_Int, FN_("fieldSize"),
		_Public, _F(NameSpace_defineClassField), TY_Int, TY_NameSpace, MN_("defineClassField"), 5, TY_Int, FN_cid, TY_Int, FN_flag, TY_Int, FN_("type"), TY_String, FN_name, TY_Object, FN_defval,
		DEND,
	};
	kNameSpace_loadMethodData(ks, MethodData);
	KSET_KLIB2(Method_indexOfField, KLIB2_Method_indexOfField, pline);
	return true;
}

static kbool_t class_setupPackage(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

// --------------------------------------------------------------------------

static kExpr* NewExpr(CTX, ksyntax_t *syn, kToken *tk, ktype_t ty, uintptr_t val)
{
	struct _kExpr *expr = new_W(Expr, syn);
	KSETv(expr->tk, tk);
	Expr_setTerm(expr, 1);
	expr->build = TEXPR_NEW;
	expr->ty = ty;
	expr->ndata = val;
	return (kExpr*)expr;
}

static KMETHOD ParseExpr_new(CTX, ksfp_t *sfp _RIX)
{
	USING_SUGAR;
	VAR_ParseExpr(stmt, tls, s, c, e);
	DBG_ASSERT(s == c);
	struct _kToken *tkNEW = tls->Wtoks[s];
	if(s + 2 < kArray_size(tls)) {
		kToken *tk1 = tls->toks[s+1];
		kToken *tk2 = tls->toks[s+2];
		kclass_t *ct = CT_(TK_type(tk1));
		if (ct->cid == CLASS_Tvoid) {
			RETURN_(SUGAR Stmt_p(_ctx, stmt, tk1, ERR_, "undefined class: %s", S_text(tk1->text)));
		} else if (CT_isForward(ct)) {
			SUGAR Stmt_p(_ctx, stmt, NULL, ERR_, "invalid application of 'new' to incomplete class %s", CT_t(ct));
		}

		if(TK_isType(tk1) && tk2->kw == AST_PARENTHESIS) {  // new C (...)
			ksyntax_t *syn = SYN_(kStmt_ks(stmt), KW_ExprMethodCall);
			kExpr *expr = SUGAR new_ConsExpr(_ctx, syn, 2, tkNEW, NewExpr(_ctx, syn, tk1, TK_type(tk1), 0));
			tkNEW->kw = MN_new;
			RETURN_(expr);
		}
		if(TK_isType(tk1) && tk2->kw == AST_BRACKET) {     // new C [...]
			ksyntax_t *syn = SYN_(kStmt_ks(stmt), KW_new);
			kclass_t *ct = CT_p0(_ctx, CT_Array, TK_type(tk1));
			tkNEW->kw = MN_("newArray");
			kExpr *expr = SUGAR new_ConsExpr(_ctx, syn, 2, tkNEW, NewExpr(_ctx, syn, tk1, ct->cid, 0));
			RETURN_(expr);
		}
	}
}

static ksymbol_t tosymbolUM(CTX, kToken *tk)
{
	DBG_ASSERT(tk->kw == TK_SYMBOL);
	return ksymbolA(S_text(tk->text), S_size(tk->text), SYM_NEWID);
}

static KMETHOD ExprTyCheck_Getter(CTX, ksfp_t *sfp _RIX)
{
	USING_SUGAR;
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kToken *tkN = expr->cons->toks[0];
	ksymbol_t fn = tosymbolUM(_ctx, tkN);
	kExpr *self = SUGAR Expr_tyCheckAt(_ctx, stmt, expr, 1, gma, TY_var, 0);
	if(self != K_NULLEXPR) {
		kMethod *mtd = kNameSpace_getMethodNULL(gma->genv->ks, self->ty, MN_toGETTER(fn));
		if(mtd == NULL) {
			mtd = kNameSpace_getMethodNULL(gma->genv->ks, self->ty, MN_toISBOOL(fn));
		}
		if(mtd != NULL) {
			KSETv(expr->cons->methods[0], mtd);
			RETURN_(SUGAR Expr_tyCheckCallParams(_ctx, stmt, expr, mtd, gma, reqty));
		}
		SUGAR Stmt_p(_ctx, stmt, tkN, ERR_, "undefined field: %s", S_text(tkN->text));
	}
	RETURN_(K_NULLEXPR);
}

// ----------------------------------------------------------------------------

static void Stmt_parseClassBlock(CTX, kStmt *stmt, kToken *tkC)
{
	USING_SUGAR;
	kToken *tkP = (kToken*)kObject_getObject(stmt, KW_BlockPattern, NULL);
	if(tkP != NULL && tkP->kw == TK_CODE) {
		kArray *a = ctxsugar->tokens;
		size_t atop = kArray_size(a), s, i;
		SUGAR NameSpace_tokenize(_ctx, kStmt_ks(stmt), S_text(tkP->text), tkP->uline, a);
		s = kArray_size(a);
		const char *cname = S_text(tkC->text);
		for(i = atop; i < s; i++) {
			kToken *tk = a->toks[i];
			int topch = kToken_topch(tk);
			DBG_P("cname='%s'", cname);
			if(topch == '(' && tkP->kw == TK_SYMBOL && strcmp(cname, S_text(tkP->text)) == 0) {
				struct _kToken *tkNEW = new_W(Token, 0);
				tkNEW->kw = TK_SYMBOL;
				KSETv(tkNEW->text, SYM_s(MN_new));
				tkNEW->uline = tkP->uline;
				kArray_add(a, tkNEW);
			}
			kArray_add(a, tk);
			tkP = tk;
		}
		kBlock *bk = SUGAR new_Block(_ctx, kStmt_ks(stmt), stmt, a, s, kArray_size(a), ';');
		for (i = 0; i < kArray_size(bk->blocks); i++) {
			kStmt *methodDecl = bk->blocks->stmts[i];
			if(methodDecl->syn->kw == KW_StmtMethodDecl) {
				kObject_setObject(methodDecl, KW_UsymbolPattern, tkC);
			}
		}
		kObject_setObject(stmt, KW_BlockPattern, bk);
		kArray_clear(a, atop);
	}
}


// ----------------------------------------------------------------------------

typedef struct {
	const char *key;
	uintptr_t ty;
	kclass_t *ct;
} KDEFINE_CLASS_CONST;

static void ObjectField_init(CTX, const struct _kObject *o, void *conf)
{
	kclass_t *ct = O_ct(o);
	DBG_ASSERT(ct->nulvalNUL != NULL);
	size_t fsize = ct->fsize;
	memcpy(((struct _kObject *)o)->fields, ct->nulvalNUL->fields, fsize * sizeof(void*));
}

extern struct _kObject** KONOHA_reftail(CTX, size_t size);

static void ObjectField_reftrace (CTX, kObject *o)
{
	kclass_t *ct =O_ct(o);
	kfield_t *fields = ct->fields;
	size_t i, fsize = ct->fsize;
	BEGIN_REFTRACE(fsize);
	for (i = 0; i < fsize; i++) {
		if (fields[i].isobj) {
			KREFTRACEn(o->fields[i]);
		}
	}
	END_REFTRACE();
}

static struct _kclass* defineClassName(CTX, kNameSpace *ks, kflag_t cflag, kString *name, kcid_t supcid, kline_t pline)
{
	KDEFINE_CLASS defNewClass = {
		.cflag  = cflag,
		.cid    = CLASS_newid,
		.bcid   = CLASS_Object,
		.supcid = supcid,
//		.init   = ObjectField_init,
	};
	kclass_t *ct = Konoha_addClassDef(ks->packid, ks->packdom, name, &defNewClass, pline);
	KDEFINE_CLASS_CONST ClassData[] = {
		{S_text(name), TY_TYPE, ct},
		{NULL},
	};
	kNameSpace_loadConstData(ks, ClassData, 0); // add class name to this namespace
//	kMethod *mtd = new_kMethod(_Public/*flag*/, ct->cid, MN_new, NULL);
//	kMethod_setParam(mtd, ct->cid, 0, NULL);
//	CT_addMethod(_ctx, ct, mtd);
	return (struct _kclass*)ct;
}

static size_t checkFieldSize(CTX, kBlock *bk)
{
	USING_SUGAR;
	size_t i, c = 0;
	for(i = 0; i < kArray_size(bk->blocks); i++) {
		kStmt *stmt = bk->blocks->stmts[i];
		DBG_P("stmt->kw=%s", KW_t(stmt->syn->kw));
		if(stmt->syn->kw == KW_StmtTypeDecl) {
			kExpr *expr = kStmt_expr(stmt, KW_ExprPattern, NULL);
			if(expr->syn->kw == KW_COMMA) {
				c += (kArray_size(expr->cons) - 1);
			}
			else if(expr->syn->kw == KW_LET || Expr_isTerm(expr)) {
				c++;
			}
		}
	}
	return c;
}

static void CT_setField(CTX, struct _kclass *ct, kclass_t *supct, int fctsize)
{
	size_t fsize = supct->fsize + fctsize;
	ct->fields = (kfield_t*)KCALLOC(fsize, sizeof(kfield_t));
	ct->fsize = supct->fsize;
	ct->fallocsize = fsize;
	if(supct->fsize > 0) {
		memcpy(ct->fields, supct->fields, sizeof(kfield_t)*supct->fsize);
		memcpy(ct->WnulvalNUL, supct->WnulvalNUL, sizeof(kObject*) * supct->fsize);
	}
}

static void CT_initField(CTX, struct _kclass *ct, kclass_t *supct, int fctsize)
{
	size_t fsize = supct->fsize + fctsize;
	ct->cstruct_size = size64(fctsize * sizeof(kObject*) + sizeof(kObjectHeader));
	DBG_P("supct->fsize=%d, fctsize=%d, cstruct_size=%d", supct->fsize, fctsize, ct->cstruct_size);
	if(fsize > 0) {
		ct->fnull(_ctx, ct);
		ct->init = ObjectField_init;
		ct->reftrace = ObjectField_reftrace;
		CT_setField(_ctx, ct, supct, fctsize);
	}
}

static kbool_t CT_declType(CTX, struct _kclass *ct, kGamma *gma, kStmt *stmt, kExpr *expr, kflag_t flag, ktype_t ty, kline_t pline)
{
	USING_SUGAR;
	if(Expr_isTerm(expr)) {
		kString *name = expr->tk->text;
		defineField(_ctx, ct, flag, ty, name, knull(CT_(ty)), 0);
		return true;
	}
	else if(expr->syn->kw == KW_LET) {
		kExpr *lexpr = kExpr_at(expr, 1);
		if(Expr_isTerm(lexpr)) {
			kExpr *vexpr = SUGAR Expr_tyCheckAt(_ctx, stmt, expr, 2, gma, ty, 0);
			if(vexpr == K_NULLEXPR) {
				return false;
			}
			kString *name = expr->tk->text;
			if(vexpr->build == TEXPR_CONST) {
				defineField(_ctx, ct, flag, ty, name, vexpr->data, 0);
			}
			else if(vexpr->build == TEXPR_NCONST) {
				defineField(_ctx, ct, flag, ty, name, NULL, vexpr->ndata);
			}
			else if(vexpr->build == TEXPR_NULL) {
				defineField(_ctx, ct, flag, ty, name, knull(CT_(ty)), 0);
			}
			else {
				SUGAR Stmt_p(_ctx, stmt, NULL, ERR_, "const value is expected as the field initial value: %s", S_text(name));
				return false;
			}
			return true;
		}
	} else if(expr->syn->kw == KW_COMMA) {
		size_t i;
		for(i = 1; i < kArray_size(expr->cons); i++) {
			if(!CT_declType(_ctx, ct, gma, stmt, kExpr_at(expr, i), flag, ty, pline)) return false;
		}
		return true;
	}
	SUGAR Stmt_p(_ctx, stmt, NULL, ERR_, "field name is expected");
	return false;
}

static kbool_t CT_addClassFields(CTX, struct _kclass *ct, kGamma *gma, kBlock *bk, kline_t pline)
{
	USING_SUGAR;
	size_t i;
	for(i = 0; i < kArray_size(bk->blocks); i++) {
		kStmt *stmt = bk->blocks->stmts[i];
		if(stmt->syn->kw == KW_StmtTypeDecl) {
			kflag_t flag = kField_Getter | kField_Setter;
			kToken *tk  = kStmt_token(stmt, KW_TypePattern, NULL);
			kExpr *expr = kStmt_expr(stmt, KW_ExprPattern, NULL);
			if(!CT_declType(_ctx, ct, gma, stmt, expr, flag, TK_type(tk), pline)) {
				return false;
			}
		}
	}
	DBG_ASSERT(ct->fsize == ct->fallocsize);
	DBG_P("all fields are set");
	KLIB2_setGetterSetter(_ctx, ct);
	return true;
}

static void CT_checkMethodDecl(CTX, kToken *tkC, kBlock *bk, kStmt **lastStmtRef)
{
	USING_SUGAR;
	size_t i;
	for(i = 0; i < kArray_size(bk->blocks); i++) {
		kStmt *stmt = bk->blocks->stmts[i];
		if(stmt->syn->kw == KW_StmtTypeDecl) continue;
		if(stmt->syn->kw == KW_StmtMethodDecl) {
			kStmt *lastStmt = lastStmtRef[0];
			SUGAR Block_insertAfter(_ctx, lastStmt->parentNULL, lastStmt, stmt);
			lastStmtRef[0] = stmt;
		}
		else {
			SUGAR Stmt_p(_ctx, stmt, NULL, WARN_, "%s is not available within the class clause", KW_t(stmt->syn->kw));
		}
	}
}

static KMETHOD StmtTyCheck_class(CTX, ksfp_t *sfp _RIX)
{
	USING_SUGAR;
	VAR_StmtTyCheck(stmt, gma);
	kToken *tkC = kStmt_token(stmt, KW_UsymbolPattern, NULL);
	kToken *tkE= kStmt_token(stmt, SYM_("extends"), NULL);
	kflag_t cflag = 0;
	kcid_t supcid = TY_Object;
	kclass_t *supct = CT_Object;
	if (tkE) {
		supcid = TK_type(tkE);
		supct = CT_(supcid);
		if(CT_isFinal(supct)) {
			SUGAR Stmt_p(_ctx, stmt, NULL, ERR_, "%s is final", CT_t(supct));
			RETURNb_(false);
		}
		if(!CT_isDefined(supct)) {
			SUGAR Stmt_p(_ctx, stmt, NULL, ERR_, "%s has undefined field(s)", CT_t(supct));
			RETURNb_(false);
		}
	}
	struct _kclass *ct = (struct _kclass*)kNameSpace_getCT(gma->genv->ks, NULL/*FIXME*/, S_text(tkC->text), S_size(tkC->text), TY_unknown);
	if (ct != NULL) {
		if (!CT_isForward(ct)) {
			SUGAR Stmt_p(_ctx, stmt, NULL, ERR_, "%s is already defined", CT_t(ct));
			RETURNb_(false);
		}
	} else {
		ct = defineClassName(_ctx, gma->genv->ks, cflag, tkC->text, supcid, stmt->uline);
	}
	((struct _kToken*)tkC)->kw = KW_TypePattern;
	((struct _kToken*)tkC)->ty = ct->cid;
	Stmt_parseClassBlock(_ctx, stmt, tkC);
	kBlock *bk = kStmt_block(stmt, KW_BlockPattern, K_NULLBLOCK);
	if (ct->nulvalNUL == NULL) {
		/* ct is created at this time */
		CT_initField(_ctx, ct, supct, checkFieldSize(_ctx, bk));
	} else {
		size_t fsize = checkFieldSize(_ctx, bk);
		CT_setField(_ctx, ct, supct, fsize);
	}
	if (bk == K_NULLBLOCK) {
		/* forward declaration, do nothing */
		FLAG_set(ct->cflag, kClass_Forward);
	} else {
		FLAG_unset(ct->cflag, kClass_Forward);
		if(!CT_addClassFields(_ctx, ct, gma, bk, stmt->uline)) {
			RETURNb_(false);
		}
	}
	kStmt_done(stmt);
	CT_checkMethodDecl(_ctx, tkC, bk, &stmt);
	RETURNb_(true);
}

static kbool_t class_initNameSpace(CTX,  kNameSpace *ks, kline_t pline)
{
	USING_SUGAR;
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .kw = SYM_("new"), ParseExpr_(new), },
		{ .kw = SYM_("class"), .rule = "\"class\" $USYMBOL [\"extends\" extends: $type] [$block]", TopStmtTyCheck_(class), },
		{ .kw = SYM_("extends"), .rule = "\"extends\" $type", },
		{ .kw = SYM_("."), ExprTyCheck_(Getter) },
		{ .kw = KW_END, },
	};
	SUGAR NameSpace_defineSyntax(_ctx, ks, SYNTAX);
	return true;
}

static kbool_t class_setupNameSpace(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

#endif /* CLASS_GLUE_H_ */
