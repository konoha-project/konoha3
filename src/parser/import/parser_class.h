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

#define PACKSUGAR    .packageId = 1, .packageDomain = 1

/* --------------- */
/* NameSpace */

static void kNameSpace_FreeSugarExtension(KonohaContext *kctx, kNameSpaceVar *ns)
{
	if(ns->tokenMatrix != NULL) {
		KFree((void *)ns->tokenMatrix, SIZEOF_TOKENMATRIX);
	}
}

/* --------------- */
/* Symbol */

static void kSyntax_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kSyntaxVar *syn = (kSyntaxVar *)o;
	bzero(&syn->packageNameSpace, sizeof(kSyntax) - sizeof(kObjectHeader));
	kNameSpace *ns = conf == NULL ? KNULL(NameSpace) : (kNameSpace *)conf;
	KFieldInit(syn, syn->packageNameSpace, ns);
}

/* --------------- */
/* Symbol */

static void kSymbol_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	ksymbol_t symbol = (ksymbol_t)v[pos].unboxValue;
	KLIB KBuffer_printf(kctx, wb, "%s%s", KSymbol_Fmt2(symbol));
}

/* --------------- */
/* Token */

static void kToken_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kTokenVar *tk = (kTokenVar *)o;
	tk->uline     = 0;
	tk->tokenType = (ksymbol_t)(intptr_t)conf;
	if(tk->tokenType == 0 || KSymbol_Unmask(tk->tokenType) != tk->tokenType) {
		KUnsafeFieldInit(tk->text, TS_EMPTY);
	}
	else {
		KUnsafeFieldInit(tk->text, KSymbol_GetString(kctx, tk->tokenType));
	}
	tk->resolvedSyntaxInfo = NULL;
}

static void kToken_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kToken *tk = (kToken *)o;
	KRefTrace(tk->text);
}

#define KToken_t(tk) kToken_t(kctx, tk)

static const char *kToken_t(KonohaContext *kctx, kToken *tk)
{
	if(IS_String(tk->text)) {
		if(tk->tokenType == TokenType_LazyBlock) {
			return "{... }";
		}
		return kString_text(tk->text);
	}
	else {
		switch(tk->tokenType) {
			case KSymbol_BraceGroup: return "{... }";
			case KSymbol_ParenthesisGroup: return "(... )";
			case KSymbol_BracketGroup: return "[... ]";
		}
		return "";
	}
}

#ifndef USE_SMALLBUILD
static void KBuffer_WriteTokenSymbol(KonohaContext *kctx, KBuffer *wb, kToken *tk)
{
	KLIB KBuffer_printf(kctx, wb, "%s%s %s%s ", KSymbol_Fmt2(tk->tokenType), KSymbol_Fmt2(tk->symbol));
}

static void KBuffer_WriteTokenText(KonohaContext *kctx, KBuffer *wb, kToken *tk)
{
	const char *text = IS_String(tk->text) ? kString_text(tk->text) : "...";
	char c = kToken_GetOpenChar(tk);
	if(c != 0) {
		KLIB KBuffer_printf(kctx, wb, "%c%s%c", c, text, kToken_GetCloseChar(tk));
	}
	else {
		KLIB KBuffer_printf(kctx, wb, "%s", text);
	}
}
#endif/*USE_SMALLBUILD*/

static void kToken_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
#ifndef USE_SMALLBUILD
	kToken *tk = v[pos].asToken;
	KBuffer_WriteTokenSymbol(kctx, wb, tk);
	if(IS_String(tk->text)) {
		KLIB KBuffer_printf(kctx, wb, "'%s'", kString_text(tk->text));
	}
	else if(IS_Array(tk->GroupTokenList)) {
		size_t i;
		kArray *a = tk->GroupTokenList;
		KLIB KBuffer_Write(kctx, wb, "[", 1);
		if(kArray_size(a) > 0) {
			KStackSetObjectValue(v[pos+1].asToken, a->TokenItems[0]);
			kToken_format(kctx, v, pos+1, wb);
		}
		for(i = 1; i < kArray_size(a); i++) {
			KLIB KBuffer_Write(kctx, wb, " ", 1);
			KStackSetObjectValue(v[pos+1].asToken, a->TokenItems[i]);
			kToken_format(kctx, v, pos+1, wb);
		}
		KLIB KBuffer_Write(kctx, wb, "]", 1);
	}
#endif
}

/* Node */

static void kUntypedNode_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kUntypedNode *node = (kUntypedNode *)o;
	kUntypedNode_setnode(node, KNode_Done);
	node->typeAttr       = KType_var;
	kNameSpace *ns = (conf == NULL) ? KNULL(NameSpace) : (kNameSpace *)conf;
	KFieldInit(node, node->TermToken, K_NULLTOKEN);
	KFieldInit(node, node->RootNodeNameSpace, ns);
	KFieldInit(node, node->StmtNameSpace, ns);
	node->index = 0L;
}

static void kUntypedNode_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kUntypedNode *node = (kUntypedNode *)o;
	KRefTrace(node->TermToken);
	KRefTrace(node->RootNodeNameSpace);
	KRefTrace(node->StmtNameSpace);
	if(kUntypedNode_Is(ObjectConst, node)) {
		KRefTrace(node->ObjectConstValue);
	}
}

#ifndef USE_SMALLBUILD
static void kUntypedNodeTerm_format(KonohaContext *kctx, kObject *o, KonohaValue *v, int pos, KBuffer *wb)
{
	if(IS_Token(o)) {
		KBuffer_WriteTokenText(kctx, wb, (kToken *)o);
	}
	else {
		KStackSetObjectValue(v[pos].asObject, o);
		kObject_class(o)->format(kctx, v, pos, wb);
	}
}

#define DefineCase(NAME)  case KNode_##NAME:   return #NAME;

static const char *KNode_text(knode_t node)
{
	switch(node) {
	NODE_LIST_OP(DefineCase)
	}
	return "unknown";
}

static void KBuffer_WriteIndent(KonohaContext *kctx, KBuffer *wb, int pos)
{
	int i;
	for(i = 0; i < pos; i++) {
		KLIB KBuffer_Write(kctx, wb, " ", 1);
	}
}
#endif


static void kUntypedNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
//#ifndef USE_SMALLBUILD
//	kUntypedNode *expr = v[pos].asNode;
//	KBuffer_WriteIndent(kctx, wb, pos);
//	KLIB KBuffer_Write(kctx, wb, "{", 1);
//	if(expr->typeAttr == KType_var) {
//		if(expr->KeyOperatorToken == K_NULLTOKEN) {
//			KLIB KBuffer_printf(kctx, wb, "nulltoken");
//		}
//		else {
//			KLIB KBuffer_printf(kctx, wb, "%s%s %s ", KSymbol_Fmt2(expr->KeyOperatorToken->resolvedSyntaxInfo->keyword), KToken_t(expr->KeyOperatorToken));
//		}
//	}
//	else {
//		KLIB KBuffer_printf(kctx, wb, "%s %s :%s ", KNode_text(kUntypedNode_node(expr)), KToken_t(expr->KeyOperatorToken), KType_text(expr->typeAttr));
//	}
//	KLIB kObjectProto_format(kctx, v, pos, wb, 0);
//	if(kUntypedNode_node(expr) == KNode_Error) {
//		KLIB KBuffer_printf(kctx, wb, "%s", kString_text(expr->ErrorMessage));
//	}
//	else if(kUntypedNode_node(expr) == KNode_Const) {
//		KLIB KBuffer_Write(kctx, wb, TEXTSIZE("const "));
//		kUntypedNodeTerm_format(kctx, (kObject *)expr->ObjectConstValue, v, pos+1, wb);
//	}
//	else if(kUntypedNode_node(expr) == KNode_New) {
//		KLIB KBuffer_printf(kctx, wb, "new %s", KType_text(expr->typeAttr));
//	}
//	else if(kUntypedNode_node(expr) == KNode_Null) {
//		KLIB KBuffer_Write(kctx, wb, TEXTSIZE("null"));
//	}
//	else if(kUntypedNode_node(expr) == KNode_UnboxConst) {
//		KLIB KBuffer_Write(kctx, wb, TEXTSIZE("const "));
//		v[pos+1].unboxValue = expr->unboxConstValue;
//		KClass_(expr->typeAttr)->format(kctx, v, pos+1, wb);
//	}
//	else if(kUntypedNode_node(expr) == KNode_Local) {
//		KLIB KBuffer_printf(kctx, wb, "local sfp[%d]", (int)expr->index);
//	}
//	else if(kUntypedNode_node(expr) == KNode_Field) {
//		khalfword_t index  = (khalfword_t)expr->index;
//		khalfword_t xindex = (khalfword_t)(expr->index >> (sizeof(khalfword_t)*8));
//		KLIB KBuffer_printf(kctx, wb, "field sfp[%d][%d]", (int)index, (int)xindex);
//	}
//	if(IS_Array(expr->NodeList)) {
//		size_t i;
//		KLIB KBuffer_Write(kctx, wb, "[", 1);
//		for(i = 0; i < kArray_size(expr->NodeList); i++) {
////			if(i > 0) {
////				KLIB KBuffer_Write(kctx, wb, " ", 1);
////			}
//			KLIB KBuffer_Write(kctx, wb, "\n", 1);
//			KBuffer_WriteIndent(kctx, wb, pos+1);
//			KLIB KBuffer_printf(kctx, wb, "#%d :%s ", (int)i, KClass_text(kObject_class(expr->NodeList->ObjectItems[i])));
//			kUntypedNodeTerm_format(kctx, expr->NodeList->ObjectItems[i], v, pos+1, wb);
//		}
//		KLIB KBuffer_Write(kctx, wb, "]", 1);
//	}
//	KLIB KBuffer_Write(kctx, wb, "}", 1);
//#endif
}

// untyped node

static kUntypedNode *new_UntypedNode(KonohaContext *kctx, kArray *gcstack, kNameSpace *ns)
{
	kUntypedNode *unode = (kUntypedNode *) new_(UntypedNode, ns, gcstack);
	unode->syn = KNULL(Syntax);
	return unode;
}

static void kUntypedNode_Reset(KonohaContext *kctx, kUntypedNode *node)
{
	//TODO
}

static void kUntypedNode_AddParsedObject(KonohaContext *kctx, kUntypedNode *stmt, ksymbol_t keyid, kObject *o)
{
	kArray* valueList = (kArray *)KLIB kObject_getObject(kctx, stmt, keyid, NULL);
	if(valueList == NULL) {
		KLIB kObjectProto_SetObject(kctx, stmt, keyid, kObject_typeId(o), o);
	}
	else {
		if(!IS_Array(valueList)) {
			INIT_GCSTACK();
			kArray *newList = new_(Array, 0, _GcStack);
			KLIB kArray_Add(kctx, newList, valueList);
			KLIB kObjectProto_SetObject(kctx, stmt, keyid, kObject_typeId(newList), newList);
			//DBG_P(">>>> value=%s, added=%s", KClass_text(kObject_class(valueList)), KClass_text(kObject_class(o)));
			valueList = newList;
			//DBG_ASSERT(kctx == NULL);
			RESET_GCSTACK();
		}
		KLIB kArray_Add(kctx, valueList, o);
	}
	if(IS_Node(o)) {
		kUntypedNode *node = (kUntypedNode *)o;
		KFieldSet(node, node->Parent, stmt);
	}
}

static kUntypedNode *kUntypedNode_AddNode(KonohaContext *kctx, kUntypedNode *self, kUntypedNode *node)
{
	if(!IS_Array(self->NodeList)) {
		KFieldSet(self, self->NodeList, new_(Array, 0, OnField));
	}
	KLIB kArray_Add(kctx, self->NodeList, node);
	if(node != K_NULLNODE) {
		KFieldSet(node, node->Parent, self);
	}
	return node;
}

static kUntypedNode *kUntypedNode_SetNodeAt(KonohaContext *kctx, kUntypedNode *self, size_t n, kUntypedNode *node)
{
	DBG_ASSERT(IS_Array(self->NodeList));
	DBG_ASSERT(n < kArray_size(self->NodeList));
	KFieldSet(self->NodeList, self->NodeList->NodeItems[n], node);
	KFieldSet(node, node->Parent, self);
	return self;
}

static void kUntypedNode_InsertAfter(KonohaContext *kctx, kUntypedNode *self, kUntypedNode *target, kUntypedNode *node)
{
	KFieldSet(node, ((kUntypedNode *)node)->Parent, self);
	if(target != NULL) {
		size_t i;
		for(i = 0; i < kArray_size(self->NodeList); i++) {
			if(self->NodeList->NodeItems[i] == target) {
				KLIB kArray_Insert(kctx, self->NodeList, i+1, node);
				return;
			}
		}
		DBG_ABORT("target was not found!!");
	}
	else {
		KLIB kArray_Add(kctx, self->NodeList, node);
	}
}

static kUntypedNode *kUntypedNode_AddSeveral(KonohaContext *kctx, kUntypedNode *self, int n, va_list ap)
{
	int i;
	if(!IS_Array(self->NodeList)) {
		KFieldSet(self, self->NodeList, new_(Array, 0, OnField));
	}
	for(i = 0; i < n; i++) {
		kUntypedNode *node =  va_arg(ap, kUntypedNode *);
		DBG_ASSERT(node != NULL);
		if(IS_Node(node)) {
			if(kUntypedNode_IsError(node)) {
				kUntypedNode_ToError(kctx, self, node->ErrorMessage);
				return node;
			}
			KFieldSet(node, node->Parent, self);
		}
		KLIB kArray_Add(kctx, self->NodeList, node);
	}
	return self;
}

//#define kUntypedNode_Termnize(kctx, node, tk) kUntypedNode_Op(kctx, node, tk, 0)

static kUntypedNode *kUntypedNode_Termnize(KonohaContext *kctx, kUntypedNode *node, kToken *termToken)
{
	KFieldSet(node, node->TermToken, termToken);
	node->syn = termToken->resolvedSyntaxInfo;
	return node;
}

static kUntypedNode *kUntypedNode_Op(KonohaContext *kctx, kUntypedNode *node, kToken *keyToken, int n, ...)
{
	KFieldSet(node, node->KeyOperatorToken, keyToken);
	node->syn = keyToken->resolvedSyntaxInfo;
//	if(n > 0) {
		KFieldSet(node, node->NodeList, new_(Array, 0, OnField));
		KLIB kArray_Add(kctx, node->NodeList, keyToken);
		va_list ap;
		va_start(ap, n);
		node = kUntypedNode_AddSeveral(kctx, node, n, ap);
		va_end(ap);
//	}
	return node;
}

//static kUntypedNode* new_UntypedOperatorNode(KonohaContext *kctx, kSyntax *syn, int n, ...)
//{
//	va_list ap;
//	va_start(ap, n);
//	DBG_ASSERT(syn != NULL);
//	kUntypedNode *expr = new_(UntypedNodeVar, syn, OnGcStack);
//	expr = kUntypedNode_AddSeveral(kctx, expr, n, ap);
//	va_end(ap);
//	return expr;
//}

static kUntypedNode *new_TypedNode(KonohaContext *kctx, kNameSpace *ns, int build, KClass *ty, int n, ...)
{
	kUntypedNode *node = new_(UntypedNode, ns, OnGcStack);
	va_list ap;
	va_start(ap, n);
	node = kUntypedNode_AddSeveral(kctx, node, n, ap);
	va_end(ap);
	kUntypedNode_setnode(node, build);
	node->typeAttr = ty->typeId;
	return (kUntypedNode *)node;
}

static kUntypedNode *TypeCheckMethodParam(KonohaContext *kctx, kMethod *mtd, kUntypedNode *self, kNameSpace *ns, KClass* reqc);

static kUntypedNode *new_MethodNode(KonohaContext *kctx, kNameSpace *ns, KClass *reqc, kMethod *mtd, int n, ...)
{
	kUntypedNode *expr = new_(UntypedNode, ns, OnGcStack);
	KFieldSet(expr, expr->NodeList, new_(Array, 0, OnField));
	KLIB kArray_Add(kctx, expr->NodeList, mtd);
	va_list ap;
	va_start(ap, n);
	expr = kUntypedNode_AddSeveral(kctx, expr, n, ap);
	va_end(ap);
	kUntypedNode_setnode(expr, KNode_MethodCall);
	return TypeCheckMethodParam(kctx, mtd, expr, ns, reqc);
}

static kUntypedNode *kUntypedNode_SetConst(KonohaContext *kctx, kUntypedNode *expr, KClass *typedClass, kObject *o)
{
	if(typedClass == NULL) typedClass = kObject_class(o);
	expr->typeAttr = typedClass->typeId;
	kUntypedNode_setnode(expr, KNode_Const);
	if(KClass_Is(UnboxType, typedClass)) {
		expr->unboxConstValue = kNumber_ToInt(o);
	}
	else {
		KFieldInit(expr, expr->ObjectConstValue, o);
		kUntypedNode_Set(ObjectConst, expr, true);
	}
	return expr;
}

static kUntypedNode *kUntypedNode_SetUnboxConst(KonohaContext *kctx, kUntypedNode *expr, ktypeattr_t typeAttr, uintptr_t unboxValue)
{
	kUntypedNode_setnode(expr, KNode_Const);
	expr->unboxConstValue = unboxValue;
	expr->typeAttr = typeAttr;
	kUntypedNode_Set(ObjectConst, expr, false);
	return expr;
}

static kUntypedNode *kUntypedNode_SetVariable(KonohaContext *kctx, kUntypedNode *expr, knode_t build, ktypeattr_t typeAttr, intptr_t index)
{
	kUntypedNode_setnode(expr, build);
	expr->typeAttr = typeAttr;
	expr->index = index;
	kUntypedNode_Set(ObjectConst, expr, false);
	return expr;
}

static uintptr_t kUntypedNode_ParseFlag(KonohaContext *kctx, kUntypedNode *stmt, KFlagSymbolData *flagData, uintptr_t flag)
{
	while(flagData->flag != 0) {
		kObject *op = kUntypedNode_GetObjectNULL(kctx, stmt, flagData->symbol);
		if(op != NULL) {
			flag |= flagData->flag;
		}
		flagData++;
	}
	return flag;
}

static kToken* kUntypedNode_GetToken(KonohaContext *kctx, kUntypedNode *stmt, ksymbol_t kw, kToken *def)
{
	kToken *tk = (kToken *)kUntypedNode_GetObjectNULL(kctx, stmt, kw);
	if(tk != NULL && IS_Token(tk)) {
		return tk;
	}
	return def;
}

static kUntypedNode *kUntypedNode_GetNode(KonohaContext *kctx, kUntypedNode *stmt, ksymbol_t kw, kUntypedNode *def)
{
	kUntypedNode *expr = (kUntypedNode *)kUntypedNode_GetObjectNULL(kctx, stmt, kw);
	if(expr != NULL && IS_Node(expr)) {
		return expr;
	}
	return def;
}

