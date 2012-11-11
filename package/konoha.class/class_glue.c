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


static kbool_t KonohaClass_setClassFieldObjectValue(KonohaContext *kctx, KonohaClassVar *definedClass, ksymbol_t sym, kObject *ObjectValue)
{
	int i;
	for(i = definedClass->fieldsize - 1; i >= 0; i--) {
		if(definedClass->fieldItems[i].fn == sym  && O_ct(definedClass->defaultNullValueVar_OnGlobalConstList->fieldObjectItems[i]) == O_ct(ObjectValue)) {
			kObjectVar *o = definedClass->defaultNullValueVar_OnGlobalConstList;
			KFieldSet(o, o->fieldObjectItems[i], ObjectValue);
			return true;
		}
	}
	return false;
}

static kbool_t KonohaClass_setClassFieldUnboxValue(KonohaContext *kctx, KonohaClassVar *definedClass, ksymbol_t sym, uintptr_t unboxValue)
{
	int i;
	for(i = definedClass->fieldsize - 1; i >= 0; i--) {
		if(definedClass->fieldItems[i].fn == sym  && TY_isUnbox(definedClass->fieldItems[i].ty)) {
			definedClass->defaultNullValueVar_OnGlobalConstList->fieldUnboxItems[i] = unboxValue;
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
//	KonohaClass *ct = KLIB kNameSpace_GetClass(kctx, sfp[0].asNameSpace, S_text(sfp[1].asString), S_size(sfp[1].asString), NULL);
//	kint_t cid = ct != NULL ? ct->typeId : sfp[2].intValue;
//	KReturnUnboxValue(cid);
//}

// ----------------------------------------------------------------------------
/* define class */

typedef struct {
	const char *key;
	uintptr_t ty;
	KonohaClass *ct;
} KDEFINE_CLASS_CONST;

static void Object_initToMakeDefaultValueAsNull(KonohaContext *kctx, kObject *o, void *conf)
{
	kObjectVar *of = (kObjectVar *)o;
	KonohaClass *c = O_ct(o);
	bzero(of->fieldObjectItems, c->cstruct_size - sizeof(KonohaObjectHeader));
}

static void ObjectField_init(KonohaContext *kctx, kObject *o, void *conf)
{
	KonohaClass *c = O_ct(o);
	size_t fieldsize = c->fieldsize;
	memcpy(((kObjectVar *)o)->fieldObjectItems, c->defaultNullValue_OnGlobalConstList->fieldObjectItems, fieldsize * sizeof(void *));
}

static void ObjectField_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	KonohaClass *c =O_ct(o);
	KonohaClassField *fieldItems = c->fieldItems;
	size_t i, fieldsize = c->fieldsize;
	BEGIN_REFTRACE(fieldsize);
	for (i = 0; i < fieldsize; i++) {
		if(fieldItems[i].isobj) {
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

static KonohaClassVar* kNameSpace_DefineClassName(KonohaContext *kctx, kNameSpace *ns, kshortflag_t cflag, kString *name, KTraceInfo *trace)
{
	KDEFINE_CLASS defNewClass = {0};
	defNewClass.cflag         = cflag | kClass_Nullable;
	defNewClass.typeId       = TY_newid;
	defNewClass.baseTypeId   = TY_Object;
	defNewClass.superTypeId  = TY_Object; //superClass->typeId;
	defNewClass.init = Object_initToMakeDefaultValueAsNull; // dummy for first generation of DefaultValueAsNull

	KonohaClassVar *definedClass = (KonohaClassVar *)KLIB kNameSpace_DefineClass(kctx, ns, name, &defNewClass, trace);
	KDEFINE_CLASS_CONST ClassData[] = {
		{S_text(name), VirtualType_KonohaClass, definedClass},
		{NULL},
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KonohaConst_(ClassData), 0);
	return definedClass;
}


static void KonohaClass_initField(KonohaContext *kctx, KonohaClassVar *definedClass, KonohaClass *superClass, size_t fieldInitSize)
{
	size_t fieldsize = superClass->fieldsize + fieldInitSize;
	definedClass->cstruct_size = size64((fieldsize * sizeof(kObject *)) + sizeof(KonohaObjectHeader));
	DBG_P("superClass->fieldsize=%d, definedFieldSize=%d, cstruct_size=%d", superClass->fieldsize, fieldInitSize, definedClass->cstruct_size);
	if(fieldsize > 0) {
		definedClass->fieldItems = (KonohaClassField *)KCalloc_UNTRACE(fieldsize, sizeof(KonohaClassField));
		definedClass->fieldAllocSize = fieldsize;
		definedClass->fieldsize = superClass->fieldsize; /* supsize */
		if(superClass->fieldsize > 0) {
			memcpy(definedClass->fieldItems, superClass->fieldItems, sizeof(KonohaClassField) * superClass->fieldsize);
		}
	}
	definedClass->fnull(kctx, definedClass);  // first generation of DefaultValueAsNull
	superClass->fnull(kctx, superClass); // ensure default value of super class
	memcpy(definedClass->defaultNullValueVar_OnGlobalConstList->fieldObjectItems, superClass->defaultNullValueVar_OnGlobalConstList->fieldObjectItems, sizeof(kObject *) * superClass->fieldsize);
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
	kToken *blockToken = (kToken *)kStmt_getObject(kctx, stmt, KW_BlockPattern, NULL);
	if(blockToken != NULL && blockToken->resolvedSyntaxInfo->keyword == KW_BlockPattern) {
		const char *cname = S_text(tokenClassName->text);
		TokenSeq range = {Stmt_ns(stmt), GetSugarContext(kctx)->preparedTokenList};
		TokenSeq_push(kctx, range);
		SUGAR TokenSeq_tokenize(kctx, &range,  S_text(blockToken->text), blockToken->uline);
		{
			TokenSeq sourceRange = {range.ns, range.tokenList, range.endIdx};
			kToken *prevToken = blockToken;
			int i;
			for(i = range.beginIdx; i < range.endIdx; i++) {
				kToken *tk = range.tokenList->TokenItems[i];
				if(tk->topCharHint == '(' && prevToken->unresolvedTokenType == TokenType_SYMBOL && strcmp(cname, S_text(prevToken->text)) == 0) {
					kTokenVar *newToken = new_(TokenVar, TokenType_SYMBOL, sourceRange.tokenList);
					KFieldSet(newToken, newToken->text, SYM_s(MN_new));
				}
				KLIB kArray_add(kctx, sourceRange.tokenList, tk);
				prevToken = tk;
			}
			TokenSeq_end(kctx, (&sourceRange));
			bk = SUGAR new_kBlock(kctx, stmt/*parent*/, NULL, &sourceRange);
			KLIB kObject_setObject(kctx, stmt, KW_BlockPattern, TY_Block, bk);
		}
		TokenSeq_pop(kctx, range);
	}
	return bk;
}

static size_t kBlock_countFieldSize(KonohaContext *kctx, kBlock *bk)
{
	size_t i, c = 0;
	if(bk != NULL) {
		for(i = 0; i < kArray_size(bk->StmtList); i++) {
			kStmt *stmt = bk->StmtList->StmtItems[i];
			DBG_P("stmt->keyword=%s%s", PSYM_t(stmt->syn->keyword));
			if(stmt->syn->keyword == KW_TypeDeclPattern) {
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
		KLIB KonohaClass_addField(kctx, definedClass, flag, ty, symbol);
		return true;
	}
	else if(expr->syn->keyword == KW_LET) {  // String name = "naruto";
		kExpr *lexpr = kExpr_at(expr, 1);
		if(Expr_isTerm(lexpr)) {
			kString *name = lexpr->termToken->text;
			ksymbol_t symbol = ksymbolA(S_text(name), S_size(name), SYM_NEWID);
			kExpr *vexpr =  SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 2, gma, ty, 0);
			if(vexpr == K_NULLEXPR) return false;
			if(vexpr->build == TEXPR_CONST) {
				KLIB KonohaClass_addField(kctx, definedClass, flag, ty, symbol);
				KonohaClass_setClassFieldObjectValue(kctx, definedClass, symbol, vexpr->objectConstValue);
			}
			else if(vexpr->build == TEXPR_NCONST) {
				KLIB KonohaClass_addField(kctx, definedClass, flag, ty, symbol);
				KonohaClass_setClassFieldUnboxValue(kctx, definedClass, symbol, vexpr->unboxConstValue);
			}
			else if(vexpr->build == TEXPR_NULL) {
				KLIB KonohaClass_addField(kctx, definedClass, flag, ty, symbol);
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
	for(i = 0; i < kArray_size(bk->StmtList); i++) {
		kStmt *stmt = bk->StmtList->StmtItems[i];
		if(stmt->syn->keyword == KW_TypeDeclPattern) {
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
		for(i = 0; i < kArray_size(bk->StmtList); i++) {
			kStmt *stmt = bk->StmtList->StmtItems[i];
			if(stmt->syn->keyword == KW_TypeDeclPattern) continue;
			if(stmt->syn->keyword == KW_MethodDeclPattern) {
				kStmt *lastStmt = classStmt;
				KLIB kObject_setObject(kctx, stmt, SYM_("ClassName"), TY_Token, tokenClassName);
				SUGAR kBlock_InsertAfter(kctx, lastStmt->parentBlockNULL, lastStmt, stmt);
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
	return size64(sizeof(KonohaObjectHeader) + (superClass->fieldsize + 4) * sizeof(kObject *)) / sizeof(kObject *);
}

static KMETHOD Statement_class(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Statement(stmt, gma);
	kToken *tokenClassName = SUGAR kStmt_getToken(kctx, stmt, SYM_("$ClassName"), NULL);
	kNameSpace *ns = Stmt_ns(stmt);
	int isNewlyDefinedClass = false;
	KonohaClassVar *definedClass = (KonohaClassVar *)KLIB kNameSpace_GetClass(kctx, ns, S_text(tokenClassName->text), S_size(tokenClassName->text), NULL);
	if(definedClass == NULL) {   // Already defined
		kshortflag_t cflag = kStmt_parseClassFlag(kctx, stmt, kClass_Virtual);
		KMakeTraceUL(trace, sfp, stmt->uline);
		definedClass = kNameSpace_DefineClassName(kctx, ns, cflag, tokenClassName->text, trace);
		isNewlyDefinedClass = true;
	}
	kBlock *bk = kStmt_parseClassBlockNULL(kctx, stmt, tokenClassName);
	size_t declsize = kBlock_countFieldSize(kctx, bk);
	if(isNewlyDefinedClass) {   // Already defined
		KonohaClass *superClass = CT_Object;
		kToken *tokenSuperClass= SUGAR kStmt_getToken(kctx, stmt, SYM_("extends"), NULL);
		if(tokenSuperClass != NULL) {
			DBG_ASSERT(Token_isVirtualTypeLiteral(tokenSuperClass));
			superClass = CT_(Token_typeLiteral(tokenSuperClass));
			if(CT_is(Final, superClass)) {
				SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, "%s is final", CT_t(superClass));
				KReturnUnboxValue(false);
			}
			if(CT_is(Virtual, superClass)) {
				SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, "%s is still virtual", CT_t(superClass));
				KReturnUnboxValue(false);
			}
		}
		size_t initsize = (bk != NULL) ? declsize : initFieldSizeOfVirtualClass(superClass);
		KonohaClass_initField(kctx, definedClass, superClass, initsize);
	}
	else {
		if(declsize > 0 && !CT_is(Virtual, definedClass)) {
			SUGAR kStmt_printMessage2(kctx, stmt, NULL, ErrTag, "%s has already defined", CT_t(definedClass));
			KReturnUnboxValue(false);
		}
	}
	if(bk != NULL) {
		if(!kBlock_declClassField(kctx, bk, gma, definedClass)) {
			KReturnUnboxValue(false);
		}
		CT_set(Virtual, definedClass, false);
	}
	kToken_setTypeId(kctx, tokenClassName, ns, definedClass->typeId);
	kBlock_addMethodDeclStmt(kctx, bk, tokenClassName, stmt);
	kStmt_done(kctx, stmt);
	KReturnUnboxValue(true);
}

static KMETHOD PatternMatch_ClassName(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	kTokenVar *tk = tokenList->TokenVarItems[beginIdx];
	int returnIdx = -1;
	if(tk->resolvedSyntaxInfo->keyword == KW_SymbolPattern || tk->resolvedSyntaxInfo->keyword == KW_TypePattern) {
		KLIB kObject_setObject(kctx, stmt, name, O_typeId(tk), tk);
		returnIdx = beginIdx + 1;
	}
	KReturnUnboxValue(returnIdx);
}

// --------------------------------------------------------------------------

static KMETHOD TypeCheck_Getter(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, expr, gma, reqty);
	kToken *tkN = expr->cons->TokenItems[0];
	ksymbol_t fn = tkN->resolvedSymbol;
	kExpr *self = SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 1, gma, TY_var, 0);
	kNameSpace *ns = Stmt_ns(stmt);
	if(self != K_NULLEXPR) {
		kMethod *mtd = KLIB kNameSpace_GetGetterMethodNULL(kctx, ns, self->ty, fn, TY_var);
		if(mtd != NULL) {
			KFieldSet(expr->cons, expr->cons->MethodItems[0], mtd);
			KReturn(SUGAR kStmtExpr_TypeCheckCallParam(kctx, stmt, expr, mtd, gma, reqty));
		}
		SUGAR kStmt_printMessage2(kctx, stmt, tkN, ErrTag, "undefined field: %s", S_text(tkN->text));
	}
}

static kbool_t class_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("$ClassName"), 0, NULL, 0, 0, PatternMatch_ClassName, NULL, NULL, NULL, NULL, },
		{ SYM_("class"), 0, "\"class\" $ClassName [\"extends\" extends: $Type] [$Block]", 0, 0, NULL, NULL, Statement_class, NULL, NULL, },
		{ SYM_("."), 0, NULL, -1, 0, NULL, NULL, NULL, NULL, TypeCheck_Getter, },
		{ KW_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}

static kbool_t class_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequirePackage("konoha.field", trace);
	//KRequirePackage("konoha.new", trace);
	class_defineSyntax(kctx, ns, trace);
	return true;
}

static kbool_t class_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* class_init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "class", "1.0");
	d.PackupNameSpace    = class_PackupNameSpace;
	d.ExportNameSpace   = class_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
