/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
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

#include "konoha3.h"

#include "konoha3/import/methoddecl.h"

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

/* copied from src/parser/import/token.h */
static void CallSugarMethod(KonohaContext *kctx, KonohaStack *sfp, kFunc *fo, int argc, kObject *obj)
{
	KStackSetFuncAll(sfp, obj, 0, fo, argc);
	KLIB KRuntime_tryCallMethod(kctx, sfp);
}

/* copied from src/parser/import/typecheck.h */
static kUntypedNode *CallTypeFunc(KonohaContext *kctx, kFunc *fo, kUntypedNode *expr, kNameSpace *ns, kObject *reqType)
{
	INIT_GCSTACK();
	BEGIN_UnusedStack(lsfp);
	KStackSetObjectValue(lsfp[1].asNode, expr);
	KStackSetObjectValue(lsfp[2].asNameSpace, ns);
	KStackSetObjectValue(lsfp[3].asObject, reqType);
	CallSugarMethod(kctx, lsfp, fo, 4, UPCAST(K_NULLNODE));
	END_UnusedStack();
	RESET_GCSTACK();
	if(kUntypedNode_IsError(expr)) return expr;
	if(lsfp[K_RTNIDX].asNode == K_NULLNODE) {
		DBG_ASSERT(expr->typeAttr == KType_var); // untyped
	}
	DBG_ASSERT(IS_Node(lsfp[K_RTNIDX].asObject));
	return (kUntypedNode *)lsfp[K_RTNIDX].asObject;
}

//## @Public void NameSpace.AddMethodDecl(Node methodNode);
static KMETHOD NameSpace_AddMethodDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	kNameSpace *ns = sfp[0].asNameSpace;
	kSyntax *syn = KLIB kNameSpace_GetSyntax(kctx, ns, KSymbol_("$MethodDecl"));
	kFunc *fo = syn->TypeFuncNULL;
	kUntypedNode *methodNode = (kUntypedNode *) sfp[1].asObject;
	CallTypeFunc(kctx, fo, methodNode, ns, K_NULL);
	KReturnVoid();
}

static void class_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Ignored, _F(NameSpace_AllowImplicitField), KType_void, KType_NameSpace, KMethodName_("AllowImplicitField"), 1, KType_Boolean, KFieldName_("allow"),
		_Public|_Const, _F(NameSpace_AddMethodDecl), KType_void, KType_NameSpace, KMethodName_("AddMethodDecl"), 1, KType_Node, KFieldName_("node"),
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
		if(definedClass->fieldItems[i].name == sym  && KType_Is(UnboxType, definedClass->fieldItems[i].typeAttr)) {
			definedClass->defaultNullValueVar->fieldUnboxItems[i] = unboxValue;
			return true;
		}
	}
	return false;
}

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
		if(KTypeAttr_Is(Boxed, fieldItems[i].typeAttr)) {
			KRefTraceNullable(o->fieldObjectItems[i]);   // FIXME:
		}
	}
}

static khalfflag_t kUntypedNode_ParseClassFlag(KonohaContext *kctx, kUntypedNode *stmt, khalfflag_t cflag)
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
	return (khalfflag_t)KLIB kUntypedNode_ParseFlag(kctx, stmt, ClassDeclFlag, cflag);
}

static KClassVar* kNameSpace_DefineClassName(KonohaContext *kctx, kNameSpace *ns, khalfflag_t cflag, kString *name, KTraceInfo *trace)
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
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(ClassData), trace);
	return definedClass;
}


static void KClass_InitField(KonohaContext *kctx, KClassVar *definedClass, KClass *superClass, size_t fieldInitSize)
{
	size_t fieldsize = superClass->fieldsize + fieldInitSize;
	definedClass->cstruct_size = size64((fieldsize * sizeof(kObject *)) + sizeof(kObjectHeader));
	DBG_P("superClass->fieldsize=%d, definedFieldSize=%d, cstruct_size=%d", superClass->fieldsize, fieldInitSize, definedClass->cstruct_size);
	if(fieldsize > 0) {
		if(definedClass->fieldItems != NULL) {
			KFree(definedClass->fieldItems, definedClass->fieldAllocSize * sizeof(KClassField));
		}
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

static kUntypedNode* kUntypedNode_ParseClassNodeNULL(KonohaContext *kctx, kUntypedNode *stmt, kToken *tokenClassName)
{
	kUntypedNode *block = NULL;
	kTokenVar *blockToken = (kTokenVar *)kUntypedNode_GetObject(kctx, stmt, KSymbol_BlockPattern, NULL);
	if(blockToken != NULL) {
		kNameSpace *ns = kUntypedNode_ns(stmt);
		KLIB kToken_ToBraceGroup(kctx, blockToken, ns, NULL);
		KTokenSeq source = {ns, RangeGroup(blockToken->GroupTokenList)};
		block = KLIB ParseNewNode(kctx, ns, source.tokenList, &source.beginIdx, source.endIdx, ParseMetaPatternOption, NULL);
		KLIB kObjectProto_SetObject(kctx, stmt, KSymbol_BlockPattern, KType_Node, block);
	}
	return block;
}

static size_t kUntypedNode_countFieldSize(KonohaContext *kctx, kUntypedNode *bk)
{
	size_t i, c = 0;
	if(bk != NULL) {
		for(i = 0; i < kUntypedNode_GetNodeListSize(kctx, bk); i++) {
			kUntypedNode *stmt = bk->NodeList->NodeItems[i];
			DBG_P("stmt->keyword=%s%s", KSymbol_Fmt2(stmt->syn->keyword));
			if(stmt->syn->keyword == KSymbol_TypeDeclPattern) {
				kUntypedNode *expr = KLIB kUntypedNode_GetNode(kctx, stmt, KSymbol_ExprPattern, NULL);
				if(expr->syn->keyword == KSymbol_COMMA) {
					c += (kUntypedNode_GetNodeListSize(kctx, expr) - 1);
				}
				else if(expr->syn->keyword == KSymbol_LET || kUntypedNode_IsTerm(expr)) {
					c++;
				}
			}
		}
	}
	return c;
}

static kbool_t kUntypedNode_AddClassField(KonohaContext *kctx, kUntypedNode *stmt, kNameSpace *ns, KClassVar *definedClass, ktypeattr_t ty, kUntypedNode *expr)
{
	if(expr->syn->keyword == KSymbol_LET) {  // String name = "naruto";
		kUntypedNode *lexpr = kUntypedNode_At(expr, 1);
		if(kUntypedNode_IsTerm(lexpr)) {
			kString *name = lexpr->TermToken->text;
			ksymbol_t symbol = KAsciiSymbol(kString_text(name), kString_size(name), KSymbol_NewId);
			kUntypedNode *vexpr =  KLIB TypeCheckNodeAt(kctx, expr, 2, ns, KClass_(ty), 0);
			if(vexpr == K_NULLNODE) return false;
			if(kUntypedNode_node(vexpr) == KNode_Const) {
				KLIB KClass_AddField(kctx, definedClass, ty, symbol);
				KClass_SetClassFieldObjectValue(kctx, definedClass, symbol, vexpr->ObjectConstValue);
			}
			else if(kUntypedNode_node(vexpr) == KNode_UnboxConst) {
				KLIB KClass_AddField(kctx, definedClass, ty, symbol);
				KClass_SetClassFieldUnboxValue(kctx, definedClass, symbol, vexpr->unboxConstValue);
			}
			else if(kUntypedNode_node(vexpr) == KNode_Null) {
				KLIB KClass_AddField(kctx, definedClass, ty, symbol);
			}
			else {
				KLIB MessageNode(kctx, stmt, lexpr->TermToken, ns, ErrTag, "field initial value must be const: %s", kString_text(name));
				return false;
			}
			return true;
		}
	} else if(expr->syn->keyword == KSymbol_COMMA) {   // String (firstName = naruto, lastName)
		size_t i;
		for(i = 1; i < kUntypedNode_GetNodeListSize(kctx, expr); i++) {
			if(!kUntypedNode_AddClassField(kctx, stmt, ns, definedClass, ty, kUntypedNode_At(expr, i))) return false;
		}
		return true;
	}
	else if(kUntypedNode_IsTerm(expr)) {  // String name
		kString *name = expr->TermToken->text;
		ksymbol_t symbol = KAsciiSymbol(kString_text(name), kString_size(name), KSymbol_NewId);
		KLIB KClass_AddField(kctx, definedClass, ty, symbol);
		return true;
	}
	KLIB MessageNode(kctx, stmt, NULL, ns, ErrTag, "field name is expected");
	return false;
}

static kbool_t kUntypedNode_declClassField(KonohaContext *kctx, kUntypedNode *bk, kNameSpace *ns, KClassVar *ct)
{
	size_t i;
	kbool_t failedOnce = false;
	for(i = 0; i < kUntypedNode_GetNodeListSize(kctx, bk); i++) {
		kUntypedNode *stmt = bk->NodeList->NodeItems[i];
		if(stmt->syn->keyword == KSymbol_TypeDeclPattern) {
			kToken *tk  = KLIB kUntypedNode_GetToken(kctx, stmt, KSymbol_TypePattern, NULL);
			kUntypedNode *expr = KLIB kUntypedNode_GetNode(kctx, stmt,  KSymbol_ExprPattern, NULL);
			if(!kUntypedNode_AddClassField(kctx, stmt, ns, ct, Token_typeLiteral(tk), expr)) {
				failedOnce = true;
			}
		}
	}
	return !(failedOnce);
}

static void kUntypedNode_AddMethodDeclNode(KonohaContext *kctx, kUntypedNode *bk, kToken *tokenClassName, kUntypedNode *classNode)
{
	if(bk == NULL) {
		return;
	}
	size_t i;

	kNameSpace *ns = kUntypedNode_ns(classNode);
	kMethod *AddMethod = KLIB kNameSpace_GetMethodByParamSizeNULL(kctx, ns, KClass_NameSpace, KMethodName_("AddMethodDecl"), 1, KMethodMatch_NoOption);

	for(i = 0; i < kUntypedNode_GetNodeListSize(kctx, bk); i++) {
		kUntypedNode *stmt = bk->NodeList->NodeItems[i];
		if(stmt->syn->keyword == KSymbol_TypeDeclPattern)
			continue;
		if(stmt->syn->keyword == KSymbol_MethodDeclPattern) {
			KLIB kObjectProto_SetObject(kctx, stmt, KSymbol_("ClassName"), KType_Token, tokenClassName);
			kUntypedNode *classParentBlock = kUntypedNode_GetParentNULL(classNode);
			if(classParentBlock == NULL) {
				classParentBlock = KNewNode(ns);
				KLIB kUntypedNode_AddNode(kctx, classParentBlock, classNode);
				kUntypedNode_Type(classParentBlock, KNode_Block, KType_void);
			}

			/* Create 'NameSpace.AddMethodDecl(stmt)' */
			kUntypedNode *arg0 = new_ConstNode(kctx, ns, NULL, UPCAST(ns));
			kUntypedNode *arg1 = new_ConstNode(kctx, ns, NULL, UPCAST(stmt));

			kUntypedNode *callNode = KLIB new_MethodNode(kctx, ns, KClass_NameSpace, AddMethod, 2, arg0, arg1);
			KLIB kUntypedNode_AddNode(kctx, classParentBlock, callNode);
		}
		else {
			KLIB MessageNode(kctx, stmt, NULL, NULL, WarnTag, "%s is not available within the class clause", KSymbol_Fmt2(stmt->syn->keyword));
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
	kToken *tokenClassName = KLIB kUntypedNode_GetToken(kctx, stmt, KSymbol_("$ClassName"), NULL);
	KClassVar *definedClass = (KClassVar *)KLIB kNameSpace_GetClassByFullName(kctx, ns, kString_text(tokenClassName->text), kString_size(tokenClassName->text), NULL);
	const int isNewlyDefinedClass = (definedClass == NULL);
	if(isNewlyDefinedClass) {
		khalfflag_t cflag = kUntypedNode_ParseClassFlag(kctx, stmt, KClassFlag_Virtual);
		KMakeTraceUL(trace, sfp, kUntypedNode_uline(stmt));
		definedClass = kNameSpace_DefineClassName(kctx, ns, cflag, tokenClassName->text, trace);
	}
	kUntypedNode *block = kUntypedNode_ParseClassNodeNULL(kctx, stmt, tokenClassName);
	size_t declsize = kUntypedNode_countFieldSize(kctx, block);
	if(isNewlyDefinedClass || (KClass_Is(Virtual, definedClass) && block != NULL)) {
		KClass *superClass = KClass_Object;
		kToken *tokenSuperClass= KLIB kUntypedNode_GetToken(kctx, stmt, KSymbol_("extends"), NULL);
		if(tokenSuperClass != NULL) {
			DBG_ASSERT(Token_isVirtualTypeLiteral(tokenSuperClass));
			superClass = KClass_(Token_typeLiteral(tokenSuperClass));
			if(KClass_Is(Final, superClass)) {
				KReturn(KLIB MessageNode(kctx, stmt, NULL, ns, ErrTag, "%s is final", KClass_text(superClass)));
			}
			if(KClass_Is(Virtual, superClass)) {
				KReturn(KLIB MessageNode(kctx, stmt, NULL, ns, ErrTag, "%s is still virtual", KClass_text(superClass)));
			}
		}
		size_t initsize = (block != NULL) ? declsize : initFieldSizeOfVirtualClass(superClass);
		KClass_InitField(kctx, definedClass, superClass, initsize);
	}
	else { // Already defined
		if(declsize > 0 && !KClass_Is(Virtual, definedClass)) {
			KReturn(KLIB MessageNode(kctx, stmt, NULL, ns, ErrTag, "%s has already defined", KClass_text(definedClass)));
		}
	}
	if(block != NULL) {
		if(!kUntypedNode_declClassField(kctx, block, ns, definedClass)) {
			KReturnUnboxValue(false);
		}
		KClass_Set(Virtual, definedClass, false);
	}
	kToken_SetTypeId(kctx, tokenClassName, ns, definedClass->typeId);
	kUntypedNode_AddMethodDeclNode(kctx, block, tokenClassName, stmt);
	KReturn(kUntypedNode_Type(stmt, KNode_Done, KType_void));
}

static KMETHOD PatternMatch_ClassName(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_PatternMatch(stmt, name, tokenList, beginIdx, endIdx);
	kTokenVar *tk = tokenList->TokenVarItems[beginIdx];
	int returnIdx = -1;
	if(tk->tokenType == KSymbol_SymbolPattern || tk->resolvedSyntaxInfo->keyword == KSymbol_TypePattern) {
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
	KLIB kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	KLIB kSyntax_AddPattern(kctx, kSyntax_(ns, KSymbol_("class")), "\"class\" $ClassName [\"extends\" extends: $Type] [$Block]", 0, trace);
	return true;
}

static kbool_t class_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KImportPackage(ns, "Konoha.ObjectModel", trace);
	class_defineSyntax(kctx, ns, trace);
	class_defineMethod(kctx, ns, trace);
	return true;
}

static kbool_t class_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

// --------------------------------------------------------------------------

KONOHA_EXPORT(KDEFINE_PACKAGE *) JavaStyleClass_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "JavaStyle", "1.2");
	d.PackupNameSpace   = class_PackupNameSpace;
	d.ExportNameSpace   = class_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
