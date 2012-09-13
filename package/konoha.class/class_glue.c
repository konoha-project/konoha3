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

static void KonohaClass_addMethod(KonohaContext *kctx, KonohaClass *ct, kMethod *mtd)
{
	if(unlikely(ct->methodList == K_EMPTYARRAY)) {
		KINITv(((KonohaClassVar*)ct)->methodList, new_(MethodArray, 8));
	}
	KLIB kArray_add(kctx, ct->methodList, mtd);
}

static void KonohaClass_addField(KonohaContext *kctx, KonohaClassVar *definedClass, int flag, ktype_t ty, ksymbol_t sym)
{
	int pos = definedClass->fieldsize;
	if(pos < definedClass->fieldAllocSize) {
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
		if(FLAG_is(definedClass->fieldItems[pos].flag, kField_Getter)) {
			FLAG_unset(definedClass->fieldItems[pos].flag, kField_Getter);
			kMethod *mtd = new_FieldGetter(kctx, definedClass->typeId, sym, ty, pos);
			KonohaClass_addMethod(kctx, definedClass, mtd);
		}
		if(FLAG_is(definedClass->fieldItems[pos].flag, kField_Setter)) {
			FLAG_unset(definedClass->fieldItems[pos].flag, kField_Setter);
			kMethod *mtd = new_FieldSetter(kctx, definedClass->typeId, sym, ty, pos);
			KonohaClass_addMethod(kctx, definedClass, mtd);
		}
	}
	else {
		kMethod *mtd = new_PrototypeGetter(kctx, definedClass->typeId, sym, ty);
		KonohaClass_addMethod(kctx, definedClass, mtd);
		mtd = new_PrototypeSetter(kctx, definedClass->typeId, sym, ty);
		KonohaClass_addMethod(kctx, definedClass, mtd);
	}
}

static kbool_t KonohaClass_setClassFieldObjectValue(KonohaContext *kctx, KonohaClassVar *definedClass, ksymbol_t sym, kObject *objectValue)
{
	int i;
	for(i = definedClass->fieldsize; i >= 0; i--) {
		if(definedClass->fieldItems[i].fn == sym  && O_ct(definedClass->defaultValueAsNullVar->fieldObjectItems[i]) == O_ct(objectValue)) {
			kObjectVar *o = definedClass->defaultValueAsNullVar;
			KSETv(o, o->fieldObjectItems[i], objectValue);
			return true;
		}
	}
	return false;
}

static kbool_t KonohaClass_setClassFieldUnboxValue(KonohaContext *kctx, KonohaClassVar *definedClass, ksymbol_t sym, uintptr_t unboxValue)
{
	int i;
	for(i = definedClass->fieldsize; i >= 0; i--) {
		if(definedClass->fieldItems[i].fn == sym  && TY_isUnbox(definedClass->fieldItems[i].ty)) {
			definedClass->defaultValueAsNullVar->fieldUnboxItems[i] = unboxValue;
			return true;
		}
	}
	return false;
}

//// --------------------------------------------------------------------------
//
//// int NameSpace.getCid(String name, int defval)
//static KMETHOD NameSpace_getCid(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KonohaClass *ct = KLIB kNameSpace_getClass(kctx, sfp[0].asNameSpace, S_text(sfp[1].asString), S_size(sfp[1].asString), NULL);
//	kint_t cid = ct != NULL ? ct->typeId : sfp[2].intValue;
//	RETURNi_(cid);
//}
//
//static void setfield(KonohaContext *kctx, KDEFINE_CLASS *ct, int fctsize, KonohaClass *supct)
//{
//	size_t fieldsize = supct->fieldsize + fctsize;
//	ct->cstruct_size = fctsize * sizeof(kObject*); //size64((fieldsize * sizeof(void*)) + sizeof(KonohaObjectHeader));
//	//DBG_P("supct->fieldsize=%d, fctsize=%d, cstruct_size=%d", supct->fieldsize, fctsize, ct->cstruct_size);
//	if(fieldsize > 0) {
//		ct->fieldItems = (KonohaClassField*)KCALLOC(fieldsize, sizeof(KonohaClassField));
//		ct->fieldsize = supct->fieldsize;
//		ct->fieldAllocSize = fieldsize;
//		if(supct->fieldsize > 0) {
//			memcpy(ct->fieldItems, supct->fieldItems, sizeof(KonohaClassField)*ct->fieldsize);
//		}
//	}
//}
//
//static KonohaClass* defineClass(KonohaContext *kctx, kNameSpace *ns, kshortflag_t cflag, kString *name, KonohaClass *supct, int fieldsize, kfileline_t pline)
//{
//	KDEFINE_CLASS defNewClass = {
//		.cflag  = cflag,
//		.typeId    = TY_newid,
//		.baseTypeId   = TY_Object,
//		.superTypeId = supct->typeId,
//	};
//	setfield(kctx, &defNewClass, fieldsize, supct);
//	KonohaClass *ct = KLIB kNameSpace_defineClass(kctx, ns, name, &defNewClass, pline);
//	ct->fnull(kctx, ct);  // create null object
//	return ct;
//}
//
//// int NameSpace.defineClass(int flag, String name, int superTypeId, int fieldsize);
//static KMETHOD NameSpace_defineClass(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ktype_t superTypeId = sfp[3].intValue == 0 ? TY_Object :(ktype_t)sfp[3].intValue;
//	KonohaClass *supct = kclass(superTypeId, sfp[K_RTNIDX].uline);
//	if(CT_isFinal(supct)) {
//		kreportf(CritTag, sfp[K_RTNIDX].uline, "%s is final", TY_t(superTypeId));
//	}
//	if(!CT_isDefined(supct)) {
//		kreportf(CritTag, sfp[K_RTNIDX].uline, "%s has undefined field(s)", TY_t(superTypeId));
//	}
//	KonohaClass *ct = defineClass(kctx, sfp[0].asNameSpace, sfp[1].intValue, sfp[2].s, supct, sfp[4].intValue, sfp[K_RTNIDX].uline);
//	RETURNi_(ct->typeId);
//}
//
//
// int NameSpace.defineClassField(int cid, int flag, int ty, String name, Object *value);
//static KMETHOD NameSpace_defineClassField(KonohaContext *kctx, KonohaStack *sfp)
//{
//	ktype_t cid = (ktype_t)sfp[1].intValue;
//	kshortflag_t flag = (kshortflag_t)sfp[2].intValue;
//	ktype_t ty = (ktype_t)sfp[3].intValue;
//	kString *name = sfp[4].s;
//	kObject *value = sfp[5].o;
//	KonohaClassVar *ct = (KonohaClassVar*)kclass(cid, sfp[K_RTNIDX].uline);
//	if(CT_isDefined(ct)) {
//		kreportf(CritTag, sfp[K_RTNIDX].uline, "%s has no undefined field", TY_t(ct->typeId));
//	}
//	KonohaClass_addField(kctx, ct, flag, ty, name, value, 0);
//	if(CT_isDefined(ct)) {
//		DBG_P("all fields are set");
//		KLIB2_setGetterSetter(kctx, ct);
//	}
//}

// --------------------------------------------------------------------------

static kbool_t class_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KRequirePackage("konoha.new", pline);
	KSET_KLIB2(kMethod_indexOfField, KLIB2_Method_indexOfField, pline);
	return true;
}

static kbool_t class_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

// ----------------------------------------------------------------------------
/* define class */

typedef struct {
	const char *key;
	uintptr_t ty;
	KonohaClass *ct;
} KDEFINE_CLASS_CONST;

static void Object_initToMakeDefaultValueAsNull(KonohaContext *kctx, kObject *o, void *conf)
{
	kObjectVar *of = (kObjectVar*)o;
	KonohaClass *c = O_ct(o);
	bzero(of->fieldObjectItems, c->cstruct_size - sizeof(KonohaObjectHeader));
}

static void ObjectField_init(KonohaContext *kctx, kObject *o, void *conf)
{
	KonohaClass *c = O_ct(o);
	size_t fieldsize = c->fieldsize;
	memcpy(((kObjectVar *)o)->fieldObjectItems, c->defaultValueAsNull->fieldObjectItems, fieldsize * sizeof(void*));
}

static void ObjectField_reftrace(KonohaContext *kctx, kObject *o)
{
	KonohaClass *c =O_ct(o);
	KonohaClassField *fieldItems = c->fieldItems;
	size_t i, fieldsize = c->fieldsize;
	BEGIN_REFTRACE(fieldsize);
	for (i = 0; i < fieldsize; i++) {
		if (fieldItems[i].isobj) {
			KREFTRACEn(o->fieldObjectItems[i]);
		}
	}
	END_REFTRACE();
}

static kshortflag_t kStmt_parseClassFlag(KonohaContext *kctx, kStmt *stmt, kshortflag_t cflag)
{
	static KonohaFlagSymbolData ClassDeclFlag[] = {
		{kClass_Private}, {kClass_Singleton}, {kClass_Immutable},
		{kClass_Prototype}, {kClass_Interface},
	};
	if(ClassDeclFlag[0].symbol == 0) {   // this is a tricky technique
		ClassDeclFlag[0].symbol = SYM_("@Private");
		ClassDeclFlag[1].symbol = SYM_("@Singleton");
		ClassDeclFlag[2].symbol = SYM_("@Immutable");
		ClassDeclFlag[3].symbol = SYM_("@Prototype");
		ClassDeclFlag[4].symbol = SYM_("@Interface");
	}
	return (kshortflag_t)SUGAR kStmt_parseFlag(kctx, stmt, ClassDeclFlag, cflag);
}

static KonohaClassVar* kNameSpace_defineClassName(KonohaContext *kctx, kNameSpace *ns, kshortflag_t cflag, kString *name, kfileline_t pline)
{
	KDEFINE_CLASS defNewClass = {
		.cflag         = cflag,
		.typeId       = TY_newid,
		.baseTypeId   = TY_Object,
		.superTypeId  = TY_Object, //superClass->typeId,
		.init = Object_initToMakeDefaultValueAsNull, // dummy for first generation of DefaultValueAsNull
	};
	KonohaClassVar *definedClass = (KonohaClassVar*)KLIB kNameSpace_defineClass(kctx, ns, name, &defNewClass, pline);
	KDEFINE_CLASS_CONST ClassData[] = {
		{S_text(name), TY_TYPE, definedClass},
		{NULL},
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(ClassData), 0);
	return definedClass;
}


static void KonohaClass_initField(KonohaContext *kctx, KonohaClassVar *definedClass, KonohaClass *superClass, size_t fieldInitSize)
{
	size_t fieldsize = superClass->fieldsize + fieldInitSize;
	definedClass->cstruct_size = size64((fieldsize * sizeof(kObject*)) + sizeof(KonohaObjectHeader));
	DBG_P("superClass->fieldsize=%d, definedFieldSize=%d, cstruct_size=%d", superClass->fieldsize, fieldInitSize, definedClass->cstruct_size);
	if(fieldsize > 0) {
		definedClass->fieldItems = (KonohaClassField*)KCALLOC(fieldsize, sizeof(KonohaClassField));
		definedClass->fieldAllocSize = fieldsize;
		definedClass->fieldsize = superClass->fieldsize; /* supsize */
		if(superClass->fieldsize > 0) {
			memcpy(definedClass->fieldItems, superClass->fieldItems, sizeof(KonohaClassField) * superClass->fieldsize);
		}
	}
	definedClass->fnull(kctx, definedClass);  // first generation of DefaultValueAsNull
	superClass->fnull(kctx, superClass); // ensure default value of super class
	memcpy(definedClass->defaultValueAsNullVar->fieldObjectItems, superClass->defaultValueAsNullVar->fieldObjectItems, sizeof(kObject*) * superClass->fieldsize);
	definedClass->init     = ObjectField_init;
	definedClass->reftrace = ObjectField_reftrace;
	definedClass->superTypeId = superClass->typeId;
	definedClass->searchSuperMethodClassNULL = superClass;
	// add other functions
}

/* Block */

static kBlock* kStmt_parseClassBlockNULL(KonohaContext *kctx, kStmt *stmt, kToken *tokenClassName)
{
	kBlock *bk = NULL;
	kToken *blockToken = (kToken*)kStmt_getObject(kctx, stmt, KW_BlockPattern, NULL);
	if(blockToken != NULL && blockToken->resolvedSyntaxInfo->keyword == KW_BlockPattern) {
		const char *cname = S_text(tokenClassName->text);
		TokenRange rangeBuf, *range = SUGAR new_TokenListRange(kctx, Stmt_nameSpace(stmt), KonohaContext_getSugarContext(kctx)->preparedTokenList, &rangeBuf);
		SUGAR TokenRange_tokenize(kctx, range,  S_text(blockToken->text), blockToken->uline);
		{
			kToken *prevToken = blockToken;
			TokenRange sourceBuf, *sourceRange = SUGAR new_TokenStackRange(kctx, range, &sourceBuf);
			int i;
			for(i = range->beginIdx; i < range->endIdx; i++) {
				kToken *tk = range->tokenList->tokenItems[i];
				if(tk->topCharHint == '(' && prevToken->unresolvedTokenType == TokenType_SYMBOL && strcmp(cname, S_text(prevToken->text)) == 0) {
					kTokenVar *newToken = GCSAFE_new(TokenVar, TokenType_SYMBOL);
					KLIB kArray_add(kctx, sourceRange->tokenList, newToken);
					KSETv(newToken, newToken->text, SYM_s(MN_new));
				}
				KLIB kArray_add(kctx, sourceRange->tokenList, tk);
				prevToken = tk;
			}
			TokenRange_end(kctx, sourceRange);
			bk = SUGAR new_kBlock(kctx, stmt/*parent*/, sourceRange, NULL);
			KLIB kObject_setObject(kctx, stmt, KW_BlockPattern, TY_Block, bk);
		}
		TokenRange_pop(kctx, range);
	}
	return bk;
}

static size_t kBlock_countFieldSize(KonohaContext *kctx, kBlock *bk)
{
	size_t i, c = 0;
	if(bk != NULL) {
		for(i = 0; i < kArray_size(bk->stmtList); i++) {
			kStmt *stmt = bk->stmtList->stmtItems[i];
			DBG_P("stmt->keyword=%s%s", PSYM_t(stmt->syn->keyword));
			if(stmt->syn->keyword == KW_StmtTypeDecl) {
				kExpr *expr = SUGAR kStmt_getExpr(kctx, stmt, KW_ExprPattern, NULL);
				if(expr->syn->keyword == KW_COMMA) {
					c += (kArray_size(expr->cons) - 1);
				}
				else if(expr->syn->keyword == KW_LET || Expr_isTerm(expr)) {
					c++;
				}
			}
		}
	}
	return c;
}

static kbool_t kStmt_addClassField(KonohaContext *kctx, kStmt *stmt, kGamma *gma, KonohaClassVar *definedClass, kshortflag_t flag, ktype_t ty, kExpr *expr)
{
	if(Expr_isTerm(expr)) {  // String name
		kString *name = expr->termToken->text;
		ksymbol_t symbol = ksymbolA(S_text(name), S_size(name), SYM_NEWID);
		KonohaClass_addField(kctx, definedClass, flag, ty, symbol);
		return true;
	}
	else if(expr->syn->keyword == KW_LET) {  // String name = "naruto";
		kExpr *lexpr = kExpr_at(expr, 1);
		if(Expr_isTerm(lexpr)) {
			kString *name = lexpr->termToken->text;
			ksymbol_t symbol = ksymbolA(S_text(name), S_size(name), SYM_NEWID);
			kExpr *vexpr =  SUGAR kStmt_tyCheckExprAt(kctx, stmt, expr, 2, gma, ty, 0);
			if(vexpr == K_NULLEXPR) return false;
			if(vexpr->build == TEXPR_CONST) {
				KonohaClass_addField(kctx, definedClass, flag, ty, symbol);
				KonohaClass_setClassFieldObjectValue(kctx, definedClass, symbol, vexpr->objectConstValue);
			}
			else if(vexpr->build == TEXPR_NCONST) {
				KonohaClass_addField(kctx, definedClass, flag, ty, symbol);
				KonohaClass_setClassFieldUnboxValue(kctx, definedClass, symbol, vexpr->unboxConstValue);
			}
			else if(vexpr->build == TEXPR_NULL) {
				KonohaClass_addField(kctx, definedClass, flag, ty, symbol);
			}
			else {
				SUGAR kStmt_printMessage2(kctx, stmt, lexpr->termToken, ErrTag, "field initial value must be const: %s", S_text(name));
				return false;
			}
			return true;
		}
	} else if(expr->syn->keyword == KW_COMMA) {   // String (firstName = naruto, lastName)
		size_t i;
		for(i = 1; i < kArray_size(expr->cons); i++) {
			if(!kStmt_addClassField(kctx, stmt, gma, definedClass, flag, ty, kExpr_at(expr, i))) return false;
		}
		return true;
	}
	SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, "field name is expected");
	return false;
}

static kbool_t kBlock_declClassField(KonohaContext *kctx, kBlock *bk, kGamma *gma, KonohaClassVar *ct)
{
	size_t i;
	kbool_t failedOnce = false;
	for(i = 0; i < kArray_size(bk->stmtList); i++) {
		kStmt *stmt = bk->stmtList->stmtItems[i];
		if(stmt->syn->keyword == KW_StmtTypeDecl) {
			kshortflag_t flag = kField_Getter | kField_Setter;
			kToken *tk  = SUGAR kStmt_getToken(kctx, stmt, KW_TypePattern, NULL);
			kExpr *expr = SUGAR kStmt_getExpr(kctx, stmt,  KW_ExprPattern, NULL);
			if(!kStmt_addClassField(kctx, stmt, gma, ct, flag, Token_typeLiteral(tk), expr)) {
				failedOnce = true;
			}
		}
	}
	return !(failedOnce);
}

static void kBlock_addMethodDeclStmt(KonohaContext *kctx, kBlock *bk, kToken *tokenClassName, kStmt *classStmt)
{
	if(bk != NULL) {
		size_t i;
		for(i = 0; i < kArray_size(bk->stmtList); i++) {
			kStmt *stmt = bk->stmtList->stmtItems[i];
			if(stmt->syn->keyword == KW_StmtTypeDecl) continue;
			if(stmt->syn->keyword == KW_StmtMethodDecl) {
				kStmt *lastStmt = classStmt;
				KLIB kObject_setObject(kctx, stmt, SYM_("ClassName"), TY_Token, tokenClassName);
				SUGAR kBlock_insertAfter(kctx, lastStmt->parentBlockNULL, lastStmt, stmt);
				lastStmt = stmt;
			}
			else {
				SUGAR kStmt_printMessage2(kctx, stmt, NULL, WarnTag, "%s is not available within the class clause", PSYM_t(stmt->syn->keyword));
			}
		}
	}
}

static inline size_t initFieldSizeOfVirtualClass(KonohaClass *superClass)
{
	return size64(sizeof(KonohaObjectHeader) + (superClass->fieldsize + 4) * sizeof(kObject*)) / sizeof(kObject*);
}

static KMETHOD StmtTyCheck_class(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_StmtTyCheck(stmt, gma);
	kToken *tokenClassName = SUGAR kStmt_getToken(kctx, stmt, SYM_("$ClassName"), NULL);
	kNameSpace *ns = Stmt_nameSpace(stmt);
	int isNewlyDefinedClass = false;
	KonohaClassVar *definedClass = (KonohaClassVar*)KLIB kNameSpace_getClass(kctx, ns, S_text(tokenClassName->text), S_size(tokenClassName->text), NULL);
	if (definedClass == NULL) {   // Already defined
		kshortflag_t cflag = kStmt_parseClassFlag(kctx, stmt, kClass_Virtual);
		definedClass = kNameSpace_defineClassName(kctx, ns, cflag, tokenClassName->text, stmt->uline);
		isNewlyDefinedClass = true;
	}
	kBlock *bk = kStmt_parseClassBlockNULL(kctx, stmt, tokenClassName);
	size_t declsize = kBlock_countFieldSize(kctx, bk);
	if (isNewlyDefinedClass) {   // Already defined
		KonohaClass *superClass = CT_Object;
		kToken *tokenSuperClass= SUGAR kStmt_getToken(kctx, stmt, SYM_("extends"), NULL);
		if (tokenSuperClass != NULL) {
			DBG_ASSERT(Token_isVirtualTypeLiteral(tokenSuperClass));
			superClass = CT_(Token_typeLiteral(tokenSuperClass));
			if(CT_isFinal(superClass)) {
				SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, "%s is final", CT_t(superClass));
				RETURNb_(false);
			}
			if(CT_isVirtual(superClass)) {
				SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, "%s is still virtual", CT_t(superClass));
				RETURNb_(false);
			}
		}
		size_t initsize = (bk != NULL) ? declsize : initFieldSizeOfVirtualClass(superClass);
		KonohaClass_initField(kctx, definedClass, superClass, initsize);
	}
	else {
		if(declsize > 0 && !CT_isVirtual(definedClass)) {
			SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, "%s has already defined", CT_t(definedClass));
			RETURNb_(false);
		}
	}
	if(bk != NULL) {
		if(!kBlock_declClassField(kctx, bk, gma, definedClass)) {
			RETURNb_(false);
		}
		CT_setVirtual(definedClass, false);
	}
	kToken_setTypeId(kctx, tokenClassName, ns, definedClass->typeId);
	kBlock_addMethodDeclStmt(kctx, bk, tokenClassName, stmt);
	kStmt_done(kctx, stmt);
	RETURNb_(true);
}

static KMETHOD PatternMatch_ClassName(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	kTokenVar *tk = tokenList->tokenVarItems[beginIdx];
	int returnIdx = -1;
	if(tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern || tk->resolvedSyntaxInfo->keyword == KW_TypePattern) {
		KLIB kObject_setObject(kctx, stmt, name, O_typeId(tk), tk);
		returnIdx = beginIdx + 1;
	}
	RETURNi_(returnIdx);
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

static kbool_t class_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	KImportPackage(ns, "konoha.new", pline);
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("$ClassName"), PatternMatch_(ClassName), },
		{ .keyword = SYM_("class"), .rule = "\"class\" $ClassName [\"extends\" extends: $Type] [$Block]", TopStmtTyCheck_(class), },
		{ .keyword = SYM_("."), ExprTyCheck_(Getter), .precedence_op2 = -1, },
		{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNameSpace);
	return true;
}

static kbool_t class_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* class_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("class", "1.0"),
		.initPackage = class_initPackage,
		.setupPackage = class_setupPackage,
		.initNameSpace = class_initNameSpace,
		.setupNameSpace = class_setupNameSpace,
	};
	return &d;
}
