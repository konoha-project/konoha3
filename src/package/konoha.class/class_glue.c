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


/* Method */

// void NameSpace_AllowImplicitField(boolean t)
static KMETHOD NameSpace_AllowImplicitField(KonohaContext *kctx, KonohaStack *sfp)
{
	kNameSpaceVar *ns = (kNameSpaceVar *)sfp[0].asNameSpace;
	kNameSpace_Set(ImplicitField, ns, sfp[1].boolValue);
}

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Ignored  kMethod_IgnoredOverride
#define _F(F)   (intptr_t)(F)

static void class_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Ignored, _F(NameSpace_AllowImplicitField), KType_void, KType_NameSpace, MN_("AllowImplicitField"), 1, KType_boolean, FN_("allow"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}



static kbool_t KonohaClass_setClassFieldObjectValue(KonohaContext *kctx, KonohaClassVar *definedClass, ksymbol_t sym, kObject *ObjectValue)
{
	int i;
	for(i = definedClass->fieldsize - 1; i >= 0; i--) {
		if(definedClass->fieldItems[i].name == sym  && kObject_class(definedClass->defaultNullValueVar->fieldObjectItems[i]) == kObject_class(ObjectValue)) {
			kObjectVar *o = definedClass->defaultNullValueVar;
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
		if(definedClass->fieldItems[i].name == sym  && KType_Is(UnboxType, definedClass->fieldItems[i].attrTypeId)) {
			definedClass->defaultNullValueVar->fieldUnboxItems[i] = unboxValue;
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
//	KonohaClass *ct = KLIB kNameSpace_GetClassByFullName(kctx, sfp[0].asNameSpace, kString_text(sfp[1].asString), kString_size(sfp[1].asString), NULL);
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

static void Object_InitToMakeDefaultValueAsNull(KonohaContext *kctx, kObject *o, void *conf)
{
	kObjectVar *of = (kObjectVar *)o;
	KonohaClass *c = kObject_class(o);
	bzero(of->fieldObjectItems, c->cstruct_size - sizeof(KonohaObjectHeader));
}

static void ObjectField_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	KonohaClass *c = kObject_class(o);
	size_t fieldsize = c->fieldsize;
	memcpy(((kObjectVar *)o)->fieldObjectItems, c->defaultNullValue->fieldObjectItems, fieldsize * sizeof(void *));
}

static void ObjectField_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	KonohaClass *c =kObject_class(o);
	KonohaClassField *fieldItems = c->fieldItems;
	size_t i, fieldsize = c->fieldsize;
	for (i = 0; i < fieldsize; i++) {
		if(TypeAttr_Is(Boxed, fieldItems[i].attrTypeId)) {
			KRefTraceNullable(o->fieldObjectItems[i]);   // FIXME:
		}
	}
}

static kshortflag_t kStmt_ParseClassFlag(KonohaContext *kctx, kStmt *stmt, kshortflag_t cflag)
{
	static KonohaFlagSymbolData ClassDeclFlag[] = {
		{KClassFlag_Private}, {KClassFlag_Singleton}, {KClassFlag_Immutable},
		{KClassFlag_Prototype}, {KClassFlag_Interface},
	};
	if(ClassDeclFlag[0].symbol == 0) {   // this is a tricky technique
		ClassDeclFlag[0].symbol = SYM_("@Private");
		ClassDeclFlag[1].symbol = SYM_("@Singleton");
		ClassDeclFlag[2].symbol = SYM_("@Immutable");
		ClassDeclFlag[3].symbol = SYM_("@Prototype");
		ClassDeclFlag[4].symbol = SYM_("@Interface");
	}
	return (kshortflag_t)SUGAR kStmt_ParseFlag(kctx, stmt, ClassDeclFlag, cflag);
}

static KonohaClassVar* kNameSpace_DefineClassName(KonohaContext *kctx, kNameSpace *ns, kshortflag_t cflag, kString *name, KTraceInfo *trace)
{
	KDEFINE_CLASS defNewClass = {0};
	defNewClass.cflag         = cflag | KClassFlag_Nullable;
	defNewClass.typeId       = TypeAttr_NewId;
	defNewClass.baseTypeId   = KType_Object;
	defNewClass.superTypeId  = KType_Object; //superClass->typeId;
	defNewClass.init = Object_InitToMakeDefaultValueAsNull; // dummy for first generation of DefaultValueAsNull

	KonohaClassVar *definedClass = (KonohaClassVar *)KLIB kNameSpace_DefineClass(kctx, ns, name, &defNewClass, trace);
	KDEFINE_CLASS_CONST ClassData[] = {
		{kString_text(name), VirtualType_KonohaClass, definedClass},
		{NULL},
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KonohaConst_(ClassData), false/*isOverride*/, trace);
	return definedClass;
}


static void KonohaClass_InitField(KonohaContext *kctx, KonohaClassVar *definedClass, KonohaClass *superClass, size_t fieldInitSize)
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
	memcpy(definedClass->defaultNullValueVar->fieldObjectItems, superClass->defaultNullValueVar->fieldObjectItems, sizeof(kObject *) * superClass->fieldsize);
	definedClass->init     = ObjectField_Init;
	definedClass->reftrace = ObjectField_Reftrace;
	definedClass->superTypeId = superClass->typeId;
	definedClass->searchSuperMethodClassNULL = superClass;
	// add other functions
}

/* Block */

static kBlock* kStmt_ParseClassBlockNULL(KonohaContext *kctx, kStmt *stmt, kToken *tokenClassName)
{
	kBlock *bk = NULL;
	kToken *blockToken = (kToken *)kStmt_GetObject(kctx, stmt, Symbol_BlockPattern, NULL);
	if(blockToken != NULL && blockToken->resolvedSyntaxInfo->keyword == Symbol_BlockPattern) {
		const char *cname = kString_text(tokenClassName->text);
		TokenSeq range = {Stmt_ns(stmt), GetSugarContext(kctx)->preparedTokenList};
		TokenSeq_Push(kctx, range);
		SUGAR TokenSeq_Tokenize(kctx, &range,  kString_text(blockToken->text), blockToken->uline);
		{
			TokenSeq sourceRange = {range.ns, range.tokenList, range.endIdx};
			kToken *prevToken = blockToken;
			int i;
			for(i = range.beginIdx; i < range.endIdx; i++) {
				kToken *tk = range.tokenList->TokenItems[i];
				if(tk->hintChar == '(' && prevToken->unresolvedTokenType == TokenType_SYMBOL && strcmp(cname, kString_text(prevToken->text)) == 0) {
					kTokenVar *newToken = new_(TokenVar, TokenType_SYMBOL, sourceRange.tokenList);
					KFieldSet(newToken, newToken->text, Symbol_GetString(kctx, MN_new));
				}
				KLIB kArray_Add(kctx, sourceRange.tokenList, tk);
				prevToken = tk;
			}
			TokenSeq_End(kctx, (&sourceRange));
			bk = SUGAR new_kBlock(kctx, stmt/*parent*/, NULL, &sourceRange);
			KLIB kObjectProto_SetObject(kctx, stmt, Symbol_BlockPattern, KType_Block, bk);
		}
		TokenSeq_Pop(kctx, range);
	}
	return bk;
}

static size_t kBlock_countFieldSize(KonohaContext *kctx, kBlock *bk)
{
	size_t i, c = 0;
	if(bk != NULL) {
		for(i = 0; i < kArray_size(bk->StmtList); i++) {
			kStmt *stmt = bk->StmtList->StmtItems[i];
			DBG_P("stmt->keyword=%s%s", Symbol_fmt2(stmt->syn->keyword));
			if(stmt->syn->keyword == Symbol_TypeDeclPattern) {
				kExpr *expr = SUGAR kStmt_GetExpr(kctx, stmt, Symbol_ExprPattern, NULL);
				if(expr->syn->keyword == Symbol_COMMA) {
					c += (kArray_size(expr->cons) - 1);
				}
				else if(expr->syn->keyword == Symbol_LET || Expr_isTerm(expr)) {
					c++;
				}
			}
		}
	}
	return c;
}

static kbool_t kStmt_AddClassField(KonohaContext *kctx, kStmt *stmt, kGamma *gma, KonohaClassVar *definedClass, ktypeattr_t ty, kExpr *expr)
{
	if(Expr_isTerm(expr)) {  // String name
		kString *name = expr->termToken->text;
		ksymbol_t symbol = ksymbolA(kString_text(name), kString_size(name), Symbol_NewId);
		KLIB KonohaClass_AddField(kctx, definedClass, ty, symbol);
		return true;
	}
	else if(expr->syn->keyword == Symbol_LET) {  // String name = "naruto";
		kExpr *lexpr = kExpr_at(expr, 1);
		if(Expr_isTerm(lexpr)) {
			kString *name = lexpr->termToken->text;
			ksymbol_t symbol = ksymbolA(kString_text(name), kString_size(name), Symbol_NewId);
			kExpr *vexpr =  SUGAR kStmt_TypeCheckExprAt(kctx, stmt, expr, 2, gma, KClass_(ty), 0);
			if(vexpr == K_NULLEXPR) return false;
			if(vexpr->build == TEXPR_CONST) {
				KLIB KonohaClass_AddField(kctx, definedClass, ty, symbol);
				KonohaClass_setClassFieldObjectValue(kctx, definedClass, symbol, vexpr->objectConstValue);
			}
			else if(vexpr->build == TEXPR_NCONST) {
				KLIB KonohaClass_AddField(kctx, definedClass, ty, symbol);
				KonohaClass_setClassFieldUnboxValue(kctx, definedClass, symbol, vexpr->unboxConstValue);
			}
			else if(vexpr->build == TEXPR_NULL) {
				KLIB KonohaClass_AddField(kctx, definedClass, ty, symbol);
			}
			else {
				SUGAR kStmt_Message2(kctx, stmt, lexpr->termToken, ErrTag, "field initial value must be const: %s", kString_text(name));
				return false;
			}
			return true;
		}
	} else if(expr->syn->keyword == Symbol_COMMA) {   // String (firstName = naruto, lastName)
		size_t i;
		for(i = 1; i < kArray_size(expr->cons); i++) {
			if(!kStmt_AddClassField(kctx, stmt, gma, definedClass, ty, kExpr_at(expr, i))) return false;
		}
		return true;
	}
	SUGAR kStmt_Message2(kctx, stmt, NULL, ErrTag, "field name is expected");
	return false;
}

static kbool_t kBlock_declClassField(KonohaContext *kctx, kBlock *bk, kGamma *gma, KonohaClassVar *ct)
{
	size_t i;
	kbool_t failedOnce = false;
	for(i = 0; i < kArray_size(bk->StmtList); i++) {
		kStmt *stmt = bk->StmtList->StmtItems[i];
		if(stmt->syn->keyword == Symbol_TypeDeclPattern) {
			kToken *tk  = SUGAR kStmt_GetToken(kctx, stmt, Symbol_TypePattern, NULL);
			kExpr *expr = SUGAR kStmt_GetExpr(kctx, stmt,  Symbol_ExprPattern, NULL);
			if(!kStmt_AddClassField(kctx, stmt, gma, ct, Token_typeLiteral(tk), expr)) {
				failedOnce = true;
			}
		}
	}
	return !(failedOnce);
}

static void kBlock_AddMethodDeclStmt(KonohaContext *kctx, kBlock *bk, kToken *tokenClassName, kStmt *classStmt)
{
	if(bk != NULL) {
		size_t i;
		for(i = 0; i < kArray_size(bk->StmtList); i++) {
			kStmt *stmt = bk->StmtList->StmtItems[i];
			if(stmt->syn->keyword == Symbol_TypeDeclPattern) continue;
			if(stmt->syn->keyword == Symbol_MethodDeclPattern) {
				kStmt *lastStmt = classStmt;
				KLIB kObjectProto_SetObject(kctx, stmt, SYM_("ClassName"), KType_Token, tokenClassName);
				SUGAR kBlock_InsertAfter(kctx, lastStmt->parentBlockNULL, lastStmt, stmt);
				lastStmt = stmt;
			}
			else {
				SUGAR kStmt_Message2(kctx, stmt, NULL, WarnTag, "%s is not available within the class clause", Symbol_fmt2(stmt->syn->keyword));
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
	kToken *tokenClassName = SUGAR kStmt_GetToken(kctx, stmt, SYM_("$ClassName"), NULL);
	kNameSpace *ns = Stmt_ns(stmt);
	int isNewlyDefinedClass = false;
	KonohaClassVar *definedClass = (KonohaClassVar *)KLIB kNameSpace_GetClassByFullName(kctx, ns, kString_text(tokenClassName->text), kString_size(tokenClassName->text), NULL);
	if(definedClass == NULL) {   // Already defined
		kshortflag_t cflag = kStmt_ParseClassFlag(kctx, stmt, KClassFlag_Virtual);
		KMakeTraceUL(trace, sfp, stmt->uline);
		definedClass = kNameSpace_DefineClassName(kctx, ns, cflag, tokenClassName->text, trace);
		isNewlyDefinedClass = true;
	}
	kBlock *bk = kStmt_ParseClassBlockNULL(kctx, stmt, tokenClassName);
	size_t declsize = kBlock_countFieldSize(kctx, bk);
	if(isNewlyDefinedClass) {   // Already defined
		KonohaClass *superClass = KClass_Object;
		kToken *tokenSuperClass= SUGAR kStmt_GetToken(kctx, stmt, SYM_("extends"), NULL);
		if(tokenSuperClass != NULL) {
			DBG_ASSERT(Token_isVirtualTypeLiteral(tokenSuperClass));
			superClass = KClass_(Token_typeLiteral(tokenSuperClass));
			if(KClass_Is(Final, superClass)) {
				SUGAR kStmt_Message2(kctx, stmt, NULL, ErrTag, "%s is final", KClass_t(superClass));
				KReturnUnboxValue(false);
			}
			if(KClass_Is(Virtual, superClass)) {
				SUGAR kStmt_Message2(kctx, stmt, NULL, ErrTag, "%s is still virtual", KClass_t(superClass));
				KReturnUnboxValue(false);
			}
		}
		size_t initsize = (bk != NULL) ? declsize : initFieldSizeOfVirtualClass(superClass);
		KonohaClass_InitField(kctx, definedClass, superClass, initsize);
	}
	else {
		if(declsize > 0 && !KClass_Is(Virtual, definedClass)) {
			SUGAR kStmt_Message2(kctx, stmt, NULL, ErrTag, "%s has already defined", KClass_t(definedClass));
			KReturnUnboxValue(false);
		}
	}
	if(bk != NULL) {
		if(!kBlock_declClassField(kctx, bk, gma, definedClass)) {
			KReturnUnboxValue(false);
		}
		KClass_Set(Virtual, definedClass, false);
	}
	kToken_setTypeId(kctx, tokenClassName, ns, definedClass->typeId);
	kBlock_AddMethodDeclStmt(kctx, bk, tokenClassName, stmt);
	kStmt_done(kctx, stmt);
	KReturnUnboxValue(true);
}

static KMETHOD PatternMatch_ClassName(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	kTokenVar *tk = tokenList->TokenVarItems[beginIdx];
	int returnIdx = -1;
	if(tk->resolvedSyntaxInfo->keyword == Symbol_SymbolPattern || tk->resolvedSyntaxInfo->keyword == Symbol_TypePattern) {
		KLIB kObjectProto_SetObject(kctx, stmt, name, kObject_typeId(tk), tk);
		returnIdx = beginIdx + 1;
	}
	KReturnUnboxValue(returnIdx);
}

// --------------------------------------------------------------------------

static kbool_t class_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ SYM_("$ClassName"), 0, NULL, 0, 0, PatternMatch_ClassName, NULL, NULL, NULL, NULL, },
		{ SYM_("class"), 0, "\"class\" $ClassName [\"extends\" extends: $Type] [$Block]", 0, 0, NULL, NULL, Statement_class, NULL, NULL, },
		{ Symbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}

static kbool_t class_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KImportPackage(ns, "konoha.field", trace);
	class_defineSyntax(kctx, ns, trace);
	class_defineMethod(kctx, ns, trace);
	return true;
}

static kbool_t class_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KDEFINE_PACKAGE* class_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "class", "1.0");
	d.PackupNameSpace   = class_PackupNameSpace;
	d.ExportNameSpace   = class_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
