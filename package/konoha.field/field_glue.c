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

static KMETHOD MethodFunc_ObjectFieldGetter(KonohaContext *kctx, KonohaStack *sfp)
{
	size_t delta = sfp[K_MTDIDX].mtdNC->delta;
	RETURN_((sfp[0].asObject)->fieldObjectItems[delta]);
}
static KMETHOD MethodFunc_UnboxFieldGetter(KonohaContext *kctx, KonohaStack *sfp)
{
	size_t delta = sfp[K_MTDIDX].mtdNC->delta;
	RETURNd_((sfp[0].asObject)->fieldUnboxItems[delta]);
}
static KMETHOD MethodFunc_ObjectFieldSetter(KonohaContext *kctx, KonohaStack *sfp)
{
	size_t delta = sfp[K_MTDIDX].mtdNC->delta;
	kObjectVar *o = sfp[0].asObjectVar;
	KSETv(o, o->fieldObjectItems[delta], sfp[1].asObject);
	RETURN_(sfp[1].asObject);
}
static KMETHOD MethodFunc_UnboxFieldSetter(KonohaContext *kctx, KonohaStack *sfp)
{
	size_t delta = sfp[K_MTDIDX].mtdNC->delta;
	(sfp[0].asObjectVar)->fieldUnboxItems[delta] = sfp[1].unboxValue;
	RETURNd_(sfp[1].unboxValue);
}
static kMethod *new_FieldGetter(KonohaContext *kctx, ktype_t cid, ksymbol_t sym, ktype_t ty, int idx)
{
	kmethodn_t mn = MN_toGETTER(sym);
	MethodFunc f = (TY_isUnbox(ty)) ? MethodFunc_UnboxFieldGetter : MethodFunc_ObjectFieldGetter;
	kMethod *mtd = KLIB new_kMethod(kctx, kMethod_Public|kMethod_Immutable, cid, mn, f);
	KLIB kMethod_setParam(kctx, mtd, ty, 0, NULL);
	((kMethodVar*)mtd)->delta = idx;  // FIXME
	return mtd;
}

static kMethod *new_FieldSetter(KonohaContext *kctx, ktype_t cid, kmethodn_t sym, ktype_t ty, int idx)
{
	kmethodn_t mn = MN_toSETTER(sym);
	MethodFunc f = (TY_isUnbox(ty)) ? MethodFunc_UnboxFieldSetter : MethodFunc_ObjectFieldSetter;
	kparamtype_t p = {ty, FN_("x")};
	kMethod *mtd = KLIB new_kMethod(kctx, kMethod_Public, cid, mn, f);
	KLIB kMethod_setParam(kctx, mtd, ty, 1, &p);
	((kMethodVar*)mtd)->delta = idx;   // FIXME
	return mtd;
}

static intptr_t KLIB2_Method_indexOfField(kMethod *mtd)
{
	MethodFunc f = mtd->invokeMethodFunc;
	if(f== MethodFunc_ObjectFieldGetter || f == MethodFunc_UnboxFieldGetter || f == MethodFunc_ObjectFieldSetter || f == MethodFunc_UnboxFieldSetter) {
		return (intptr_t)mtd->delta;
	}
	return -1;
}

static KMETHOD MethodFunc_ObjectPrototypeGetter(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[K_MTDIDX].mtdNC;
	ksymbol_t key = (ksymbol_t)mtd->delta;
	RETURN_(KLIB kObject_getObject(kctx, sfp[0].asObject, key, sfp[K_RTNIDX].o));
}

static KMETHOD MethodFunc_UnboxPrototypeGetter(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[K_MTDIDX].mtdNC;
	ksymbol_t key = (ksymbol_t)mtd->delta;
	RETURNd_(KLIB kObject_getUnboxValue(kctx, sfp[0].asObject, key, 0));
}

static KMETHOD MethodFunc_ObjectPrototypeSetter(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[K_MTDIDX].mtdNC;
	ksymbol_t key = (ksymbol_t)mtd->delta;
	KLIB kObject_setObject(kctx, sfp[0].asObject, key, O_typeId(sfp[1].asObject), sfp[1].asObject);
	RETURN_(sfp[1].asObject);
}

static KMETHOD MethodFunc_UnboxPrototypeSetter(KonohaContext *kctx, KonohaStack *sfp)
{
	kMethod *mtd = sfp[K_MTDIDX].mtdNC;
	ksymbol_t key = (ksymbol_t)mtd->delta;
	kParam *pa = Method_param(mtd);
	KLIB kObject_setUnboxValue(kctx, sfp[0].asObject, key, pa->paramtypeItems[0].ty, sfp[1].unboxValue);
	RETURNd_(sfp[1].unboxValue);
}

static kMethod *new_PrototypeGetter(KonohaContext *kctx, ktype_t cid, ksymbol_t sym, ktype_t ty)
{
	kmethodn_t mn = MN_toGETTER(sym);
	MethodFunc f = (TY_isUnbox(ty)) ? MethodFunc_UnboxPrototypeGetter : MethodFunc_ObjectPrototypeGetter;
	kMethod *mtd = KLIB new_kMethod(kctx, kMethod_Public|kMethod_Immutable, cid, mn, f);
	KLIB kMethod_setParam(kctx, mtd, ty, 0, NULL);
	((kMethodVar*)mtd)->delta = sym;
	return mtd;
}

static kMethod *new_PrototypeSetter(KonohaContext *kctx, ktype_t cid, ksymbol_t sym, ktype_t ty)
{
	kmethodn_t mn = MN_toSETTER(sym);
	MethodFunc f = (TY_isUnbox(ty)) ? MethodFunc_UnboxPrototypeSetter : MethodFunc_ObjectPrototypeSetter;
	kparamtype_t p = {ty, FN_("x")};
	kMethod *mtd = KLIB new_kMethod(kctx, kMethod_Public, cid, mn, f);
	KLIB kMethod_setParam(kctx, mtd, ty, 1, &p);
	((kMethodVar*)mtd)->delta = sym;
	return mtd;
}

static kbool_t KonohaClass_addField(KonohaContext *kctx, KonohaClass *ct, int flag, ktype_t ty, ksymbol_t sym)
{
	int pos = ct->fieldsize;
	if(unlikely(ct->methodList == K_EMPTYARRAY)) {
		KINITv(((KonohaClassVar*)ct)->methodList, new_(MethodArray, 8));
	}

	if(pos < ct->fieldAllocSize) {
		KonohaClassVar *definedClass = (KonohaClassVar*)ct;
		definedClass->fieldsize += 1;
		definedClass->fieldItems[pos].flag = flag;
		definedClass->fieldItems[pos].ty = ty;
		definedClass->fieldItems[pos].fn = sym;
		if(TY_isUnbox(ty)) {
			definedClass->defaultValueAsNullVar->fieldUnboxItems[pos] = 0;
		}
		else {
			kObjectVar *o = definedClass->defaultValueAsNullVar;
			KSETv(o, o->fieldObjectItems[pos], KLIB Knull(kctx, CT_(ty)));
			definedClass->fieldItems[pos].isobj = 1;
		}
		if(FLAG_is(flag, kField_Getter)) {
			FLAG_unset(definedClass->fieldItems[pos].flag, kField_Getter);
			kMethod *mtd = new_FieldGetter(kctx, definedClass->typeId, sym, ty, pos);
			KLIB kArray_add(kctx, ct->methodList, mtd);
		}
		if(FLAG_is(flag, kField_Setter)) {
			FLAG_unset(definedClass->fieldItems[pos].flag, kField_Setter);
			kMethod *mtd = new_FieldSetter(kctx, definedClass->typeId, sym, ty, pos);
			KLIB kArray_add(kctx, ct->methodList, mtd);
		}
	}
	else {
		if(FLAG_is(flag, kField_Getter)) {
			kMethod *mtd = new_PrototypeGetter(kctx, ct->typeId, sym, ty);
			KLIB kArray_add(kctx, ct->methodList, mtd);
		}
		if(FLAG_is(flag, kField_Setter)) {
			kMethod *mtd = new_PrototypeSetter(kctx, ct->typeId, sym, ty);
			KLIB kArray_add(kctx, ct->methodList, mtd);
		}
	}
	return true;
}

// --------------------------------------------------------------------------

static kbool_t field_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KSET_KLIB2(kMethod_indexOfField, KLIB2_Method_indexOfField, pline);
	KSET_KLIB2(KonohaClass_addField, KonohaClass_addField, pline);
	return true;
}

static kbool_t field_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

// --------------------------------------------------------------------------

static KMETHOD ExprTyCheck_Getter(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kToken *tkN = expr->cons->tokenItems[0];
	ksymbol_t fn = tkN->resolvedSymbol;
	kExpr *self = SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 1, gma, TY_var, 0);
	kNameSpace *ns = Stmt_nameSpace(stmt);
	if(self != K_NULLEXPR) {
		kMethod *mtd = KLIB kNameSpace_getGetterMethodNULL(kctx, ns, self->ty, fn, TY_var);
		if(mtd != NULL) {
			KSETv(expr->cons, expr->cons->methodItems[0], mtd);
			RETURN_(SUGAR kStmt_tyCheckCallParamExpr(kctx, stmt, expr, mtd, gma, reqty));
		}
		SUGAR kStmt_printMessage2(kctx, stmt, tkN, ErrTag, "undefined field: %s", S_text(tkN->text));
	}
}

static kbool_t field_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("."), ExprTyCheck_(Getter), .precedence_op2 = -1, },
		{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNameSpace);
	return true;
}

static kbool_t field_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* field_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("konoha", "1.0"),
		.initPackage = field_initPackage,
		.setupPackage = field_setupPackage,
		.initNameSpace = field_initNameSpace,
		.setupNameSpace = field_setupNameSpace,
	};
	return &d;
}
#ifdef __cplusplus
}
#endif

