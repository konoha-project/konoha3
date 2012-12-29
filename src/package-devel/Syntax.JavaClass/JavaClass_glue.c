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
		_Public|_Const|_Ignored, _F(NameSpace_AllowImplicitField), KType_void, KType_NameSpace, KMethodName_("AllowImplicitField"), 1, KType_boolean, KFieldName_("allow"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
}

static kbool_t KClass_SetClassFieldObjectValue(KonohaContext *kctx, KClassVar *definedClass, ksymbol_t sym, kObject *ObjectValue)
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

static kbool_t KClass_SetClassFieldUnboxValue(KonohaContext *kctx, KClassVar *definedClass, ksymbol_t sym, uintptr_t unboxValue)
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
//	KClass *ct = KLIB kNameSpace_GetClassByFullName(kctx, sfp[0].asNameSpace, kString_text(sfp[1].asString), kString_size(sfp[1].asString), NULL);
//	kint_t cid = ct != NULL ? ct->typeId : sfp[2].intValue;
//	KReturnUnboxValue(cid);
//}

// ----------------------------------------------------------------------------
/* define class */

typedef struct {
	const char *key;
	uintptr_t ty;
	KClass *ct;
} KDEFINE_CLASS_CONST;

static void Object_InitToMakeDefaultValueAsNull(KonohaContext *kctx, kObject *o, void *conf)
{
	kObjectVar *of = (kObjectVar *)o;
	KClass *c = kObject_class(o);
	bzero(of->fieldObjectItems, c->cstruct_size - sizeof(kObjectHeader));
}

static void ObjectField_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	KClass *c = kObject_class(o);
	size_t fieldsize = c->fieldsize;
	memcpy(((kObjectVar *)o)->fieldObjectItems, c->defaultNullValue->fieldObjectItems, fieldsize * sizeof(void *));
}

static void ObjectField_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	KClass *c =kObject_class(o);
	KClassField *fieldItems = c->fieldItems;
	size_t i, fieldsize = c->fieldsize;
	for (i = 0; i < fieldsize; i++) {
		if(KTypeAttr_Is(Boxed, fieldItems[i].attrTypeId)) {
			KRefTraceNullable(o->fieldObjectItems[i]);   // FIXME:
		}
	}
}

static kshortflag_t kNode_ParseClassFlag(KonohaContext *kctx, kNode *stmt, kshortflag_t cflag)
{
	static KFlagSymbolData ClassDeclFlag[] = {
		{KClassFlag_Private}, {KClassFlag_Singleton}, {KClassFlag_Immutable},
		{KClassFlag_Prototype}, {KClassFlag_Interface},
	};
	if(ClassDeclFlag[0].symbol == 0) {   // this is a tricky technique
		ClassDeclFlag[0].symbol = KSymbol_("@Private");
		ClassDeclFlag[1].symbol = KSymbol_("@Singleton");
		ClassDeclFlag[2].symbol = KSymbol_("@Immutable");
		ClassDeclFlag[3].symbol = KSymbol_("@Prototype");
		ClassDeclFlag[4].symbol = KSymbol_("@Interface");
	}
	return (kshortflag_t)SUGAR kNode_ParseFlag(kctx, stmt, ClassDeclFlag, cflag);
}

static KClassVar* kNameSpace_DefineClassName(KonohaContext *kctx, kNameSpace *ns, kshortflag_t cflag, kString *name, KTraceInfo *trace)
{
	KDEFINE_CLASS defNewClass = {0};
	defNewClass.cflag         = cflag | KClassFlag_Nullable;
	defNewClass.typeId       = KTypeAttr_NewId;
	defNewClass.baseTypeId   = KType_Object;
	defNewClass.superTypeId  = KType_Object; //superClass->typeId;
	defNewClass.init = Object_InitToMakeDefaultValueAsNull; // dummy for first generation of DefaultValueAsNull

	KClassVar *definedClass = (KClassVar *)KLIB kNameSpace_DefineClass(kctx, ns, name, &defNewClass, trace);
	KDEFINE_CLASS_CONST ClassData[] = {
		{kString_text(name), VirtualType_KClass, definedClass},
		{NULL},
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(ClassData), false/*isOverride*/, trace);
	return definedClass;
}


static void KClass_InitField(KonohaContext *kctx, KClassVar *definedClass, KClass *superClass, size_t fieldInitSize)
{
	size_t fieldsize = superClass->fieldsize + fieldInitSize;
	definedClass->cstruct_size = size64((fieldsize * sizeof(kObject *)) + sizeof(kObjectHeader));
	DBG_P("superClass->fieldsize=%d, definedFieldSize=%d, cstruct_size=%d", superClass->fieldsize, fieldInitSize, definedClass->cstruct_size);
	if(fieldsize > 0) {
		definedClass->fieldItems = (KClassField *)KCalloc_UNTRACE(fieldsize, sizeof(KClassField));
		definedClass->fieldAllocSize = fieldsize;
		definedClass->fieldsize = superClass->fieldsize; /* supsize */
		if(superClass->fieldsize > 0) {
			memcpy(definedClass->fieldItems, superClass->fieldItems, sizeof(KClassField) * superClass->fieldsize);
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

/* Node */

static kNode* kNode_ParseClassNodeNULL(KonohaContext *kctx, kNode *stmt, kToken *tokenClassName)
{
	kNode *bk = NULL;
	kToken *blockToken = (kToken *)kNode_GetObject(kctx, stmt, KSymbol_BlockPattern, NULL);
	if(blockToken != NULL && blockToken->resolvedSyntaxInfo->keyword == KSymbol_BlockPattern) {
		const char *cname = kString_text(tokenClassName->text);
		KTokenSeq range = {kNode_ns(stmt), KGetParserContext(kctx)->preparedTokenList};
		KTokenSeq_Push(kctx, range);
		SUGAR KTokenSeq_Tokenize(kctx, &range,  kString_text(blockToken->text), blockToken->uline);
		{
			KTokenSeq sourceRange = {range.ns, range.tokenList, range.endIdx};
			kToken *prevToken = blockToken;
			int i;
			for(i = range.beginIdx; i < range.endIdx; i++) {
				kToken *tk = range.tokenList->TokenItems[i];
				if(tk->hintChar == '(' && prevToken->tokenType == TokenType_SYMBOL && strcmp(cname, kString_text(prevToken->text)) == 0) {
					kTokenVar *newToken = new_(TokenVar, TokenType_SYMBOL, sourceRange.tokenList);
					KFieldSet(newToken, newToken->text, KSymbol_GetString(kctx, MN_new));
				}
				KLIB kArray_Add(kctx, sourceRange.tokenList, tk);
				prevToken = tk;
			}
			KTokenSeq_End(kctx, (&sourceRange));
			bk = SUGAR ParseNewNode(kctx, range.ns, sourceRange.tokenList, &sourceRange.beginIdx, sourceRange.endIdx, ParseMetaPatternOption, NULL);
			KLIB kObjectProto_SetObject(kctx, stmt, KSymbol_BlockPattern, KType_Node, bk);
		}
		KTokenSeq_Pop(kctx, range);
	}
	return bk;
}

static size_t kNode_countFieldSize(KonohaContext *kctx, kNode *bk)
{
	size_t i, c = 0;
	if(bk != NULL) {
		for(i = 0; i < kArray_size(bk->NodeList); i++) {
			kNode *stmt = bk->NodeList->NodeItems[i];
			DBG_P("stmt->keyword=%s%s", KSymbol_Fmt2(stmt->syn->keyword));
			if(stmt->syn->keyword == KSymbol_TypeDeclPattern) {
				kNode *expr = SUGAR kNode_GetNode(kctx, stmt, KSymbol_ExprPattern, NULL);
				if(expr->syn->keyword == KSymbol_COMMA) {
					c += (kArray_size(expr->NodeList) - 1);
				}
				else if(expr->syn->keyword == KSymbol_LET || kNode_IsTerm(expr)) {
					c++;
				}
			}
		}
	}
	return c;
}

static kbool_t kNode_AddClassField(KonohaContext *kctx, kNode *stmt, kNameSpace *ns, KClassVar *definedClass, ktypeattr_t ty, kNode *expr)
{
	if(kNode_IsTerm(expr)) {  // String name
		kString *name = expr->TermToken->text;
		ksymbol_t symbol = KAsciiSymbol(kString_text(name), kString_size(name), KSymbol_NewId);
		KLIB KClass_AddField(kctx, definedClass, ty, symbol);
		return true;
	}
	else if(expr->syn->keyword == KSymbol_LET) {  // String name = "naruto";
		kNode *lexpr = kNode_At(expr, 1);
		if(kNode_IsTerm(lexpr)) {
			kString *name = lexpr->TermToken->text;
			ksymbol_t symbol = KAsciiSymbol(kString_text(name), kString_size(name), KSymbol_NewId);
			kNode *vexpr =  SUGAR TypeCheckNodeAt(kctx, expr, 2, ns, KClass_(ty), 0);
			if(vexpr == K_NULLNODE) return false;
			if(vexpr->node == KNode_Const) {
				KLIB KClass_AddField(kctx, definedClass, ty, symbol);
				KClass_SetClassFieldObjectValue(kctx, definedClass, symbol, vexpr->ObjectConstValue);
			}
			else if(vexpr->node == KNode_UnboxConst) {
				KLIB KClass_AddField(kctx, definedClass, ty, symbol);
				KClass_SetClassFieldUnboxValue(kctx, definedClass, symbol, vexpr->unboxConstValue);
			}
			else if(vexpr->node == KNode_Null) {
				KLIB KClass_AddField(kctx, definedClass, ty, symbol);
			}
			else {
				SUGAR MessageNode(kctx, stmt, lexpr->TermToken, ns, ErrTag, "field initial value must be const: %s", kString_text(name));
				return false;
			}
			return true;
		}
	} else if(expr->syn->keyword == KSymbol_COMMA) {   // String (firstName = naruto, lastName)
		size_t i;
		for(i = 1; i < kArray_size(expr->NodeList); i++) {
			if(!kNode_AddClassField(kctx, stmt, ns, definedClass, ty, kNode_At(expr, i))) return false;
		}
		return true;
	}
	SUGAR MessageNode(kctx, stmt, NULL, ns, ErrTag, "field name is expected");
	return false;
}

static kbool_t kNode_declClassField(KonohaContext *kctx, kNode *bk, kNameSpace *ns, KClassVar *ct)
{
	size_t i;
	kbool_t failedOnce = false;
	for(i = 0; i < kArray_size(bk->NodeList); i++) {
		kNode *stmt = bk->NodeList->NodeItems[i];
		if(stmt->syn->keyword == KSymbol_TypeDeclPattern) {
			kToken *tk  = SUGAR kNode_GetToken(kctx, stmt, KSymbol_TypePattern, NULL);
			kNode *expr = SUGAR kNode_GetNode(kctx, stmt,  KSymbol_ExprPattern, NULL);
			if(!kNode_AddClassField(kctx, stmt, ns, ct, Token_typeLiteral(tk), expr)) {
				failedOnce = true;
			}
		}
	}
	return !(failedOnce);
}

static void kNode_AddMethodDeclNode(KonohaContext *kctx, kNode *bk, kToken *tokenClassName, kNode *classNode)
{
	if(bk != NULL) {
		size_t i;
		for(i = 0; i < kArray_size(bk->NodeList); i++) {
			kNode *stmt = bk->NodeList->NodeItems[i];
			if(stmt->syn->keyword == KSymbol_TypeDeclPattern) continue;
			if(stmt->syn->keyword == KSymbol_MethodDeclPattern) {
				kNode *lastNode = classNode;
				KLIB kObjectProto_SetObject(kctx, stmt, KSymbol_("ClassName"), KType_Token, tokenClassName);
				SUGAR kNode_InsertAfter(kctx, lastNode->Parent, lastNode, stmt);
				lastNode = stmt;
			}
			else {
				SUGAR MessageNode(kctx, stmt, NULL, NULL, WarnTag, "%s is not available within the class clause", KSymbol_Fmt2(stmt->syn->keyword));
			}
		}
	}
}

static inline size_t initFieldSizeOfVirtualClass(KClass *superClass)
{
	return size64(sizeof(kObjectHeader) + (superClass->fieldsize + 4) * sizeof(kObject *)) / sizeof(kObject *);
}

static KMETHOD Statement_class(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_TypeCheck(stmt, ns, reqc);
	kToken *tokenClassName = SUGAR kNode_GetToken(kctx, stmt, KSymbol_("$ClassName"), NULL);
	int isNewlyDefinedClass = false;
	KClassVar *definedClass = (KClassVar *)KLIB kNameSpace_GetClassByFullName(kctx, ns, kString_text(tokenClassName->text), kString_size(tokenClassName->text), NULL);
	if(definedClass == NULL) {   // Already defined
		kshortflag_t cflag = kNode_ParseClassFlag(kctx, stmt, KClassFlag_Virtual);
		KMakeTraceUL(trace, sfp, kNode_uline(stmt));
		definedClass = kNameSpace_DefineClassName(kctx, ns, cflag, tokenClassName->text, trace);
		isNewlyDefinedClass = true;
	}
	kNode *bk = kNode_ParseClassNodeNULL(kctx, stmt, tokenClassName);
	size_t declsize = kNode_countFieldSize(kctx, bk);
	if(isNewlyDefinedClass) {   // Already defined
		KClass *superClass = KClass_Object;
		kToken *tokenSuperClass= SUGAR kNode_GetToken(kctx, stmt, KSymbol_("extends"), NULL);
		if(tokenSuperClass != NULL) {
			DBG_ASSERT(Token_isVirtualTypeLiteral(tokenSuperClass));
			superClass = KClass_(Token_typeLiteral(tokenSuperClass));
			if(KClass_Is(Final, superClass)) {
				SUGAR MessageNode(kctx, stmt, NULL, ns, ErrTag, "%s is final", KClass_text(superClass));
				KReturnUnboxValue(false);
			}
			if(KClass_Is(Virtual, superClass)) {
				SUGAR MessageNode(kctx, stmt, NULL, ns, ErrTag, "%s is still virtual", KClass_text(superClass));
				KReturnUnboxValue(false);
			}
		}
		size_t initsize = (bk != NULL) ? declsize : initFieldSizeOfVirtualClass(superClass);
		KClass_InitField(kctx, definedClass, superClass, initsize);
	}
	else {
		if(declsize > 0 && !KClass_Is(Virtual, definedClass)) {
			SUGAR MessageNode(kctx, stmt, NULL, ns, ErrTag, "%s has already defined", KClass_text(definedClass));
			KReturnUnboxValue(false);
		}
	}
	if(bk != NULL) {
		if(!kNode_declClassField(kctx, bk, ns, definedClass)) {
			KReturnUnboxValue(false);
		}
		KClass_Set(Virtual, definedClass, false);
	}
	kToken_SetTypeId(kctx, tokenClassName, ns, definedClass->typeId);
	kNode_AddMethodDeclNode(kctx, bk, tokenClassName, stmt);
	KReturn(kNode_Type(kctx, stmt, KNode_Done, KType_void));
}

static KMETHOD PatternMatch_ClassName(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	kTokenVar *tk = tokenList->TokenVarItems[beginIdx];
	int returnIdx = -1;
	if(tk->resolvedSyntaxInfo->keyword == KSymbol_SymbolPattern || tk->resolvedSyntaxInfo->keyword == KSymbol_TypePattern) {
		KLIB kObjectProto_SetObject(kctx, stmt, name, kObject_typeId(tk), tk);
		returnIdx = beginIdx + 1;
	}
	KReturnUnboxValue(returnIdx);
}

// --------------------------------------------------------------------------

static kbool_t class_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("$ClassName"), SYNFLAG_CFunc, 0, 0, {SUGARFUNC PatternMatch_ClassName}, {NULL}},
		{ KSymbol_("class"), SYNFLAG_CTypeFunc, 0, Precedence_Statement, {SUGAR patternParseFunc}, {SUGARFUNC Statement_class}},
		{ KSymbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	SUGAR kNameSpace_AddSyntaxPattern(kctx, ns, KSymbol_("class"), "\"class\" $ClassName [\"extends\" extends: $Type] [$Block]", 0, trace);
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

KDEFINE_PACKAGE* JavaClass_Init(void)
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
