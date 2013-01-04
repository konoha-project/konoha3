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

//static void syntaxMap_Free(KonohaContext *kctx, void *p)
//{
//	KFree(p, sizeof(kSyntax));
//}

static void kNameSpace_FreeSugarExtension(KonohaContext *kctx, kNameSpaceVar *ns)
{
//	if(ns->syntaxMapNN != NULL) {
//		KLIB KHashMap_Free(kctx, ns->syntaxMapNN, NULL/*syntaxMap_Free*/);
//	}
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

static void kSymbol_p(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	ksymbol_t symbol = (ksymbol_t)v[pos].unboxValue;
	KLIB KBuffer_printf(kctx, wb, "%s%s", KSymbol_Fmt2(symbol));
}

/* --------------- */
/* Token */

static void kToken_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kTokenVar *tk = (kTokenVar *)o;
	tk->uline     =   0;
	tk->tokenType = (ksymbol_t)(intptr_t)conf;
	if(tk->tokenType == 0  || KSymbol_Unmask(tk->tokenType) != tk->tokenType) {
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
		switch(tk->resolvedSymbol) {
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
	if(tk->resolvedSymbol == TokenType_INDENT) {
		KLIB KBuffer_printf(kctx, wb, "$Indent ");
	}
	else {
		ksymbol_t symbolType = tk->resolvedSyntaxInfo == NULL ? tk->resolvedSymbol : tk->resolvedSyntaxInfo->keyword;
		KLIB KBuffer_printf(kctx, wb, "%s%s ", KSymbol_Fmt2(symbolType));
	}
}

static void KBuffer_WriteTokenText(KonohaContext *kctx, KBuffer *wb, kToken *tk)
{
	const char *text = IS_String(tk->text) ? kString_text(tk->text) : "...";
	char c = kToken_GetOpenHintChar(tk);
	if(c != 0) {
		KLIB KBuffer_printf(kctx, wb, "%c%s%c", c, text, kToken_GetCloseHintChar(tk));
	}
	else {
		KLIB KBuffer_printf(kctx, wb, "%s", text);
	}
}
#endif/*USE_SMALLBUILD*/

static void kToken_p(KonohaContext *kctx, KonohaValue *values, int pos, KBuffer *wb)
{
#ifndef USE_SMALLBUILD
	kToken *tk = values[pos].asToken;
	KBuffer_WriteTokenSymbol(kctx, wb, tk);
	if(IS_String(tk->text)) {
		KLIB KBuffer_printf(kctx, wb, "'%s'", kString_text(tk->text));
	}
	else if(IS_Array(tk->GroupTokenList)) {
		size_t i;
		kArray *a = tk->GroupTokenList;
		KLIB KBuffer_Write(kctx, wb, "[", 1);
		if(kArray_size(a) > 0) {
			KUnsafeFieldSet(values[pos+1].asToken, a->TokenItems[0]);
			kToken_p(kctx, values, pos+1, wb);
		}
		for(i = 1; i < kArray_size(a); i++) {
			KLIB KBuffer_Write(kctx, wb, " ", 1);
			KUnsafeFieldSet(values[pos+1].asToken, a->TokenItems[i]);
			kToken_p(kctx, values, pos+1, wb);
		}
		KLIB KBuffer_Write(kctx, wb, "]", 1);
	}
#endif
}

/* Node */

static void kNode_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kNodeVar *node = (kNodeVar *)o;
	node->node             = KNode_Done;
	node->attrTypeId       = KType_var;
	kNameSpace *ns = (conf == NULL) ? KNULL(NameSpace) : (kNameSpace *)conf;
	KFieldInit(node, node->TermToken, K_NULLTOKEN);
	KFieldInit(node, node->RootNodeNameSpace, ns);
	KFieldInit(node, node->StmtNameSpace, ns);
	node->index = 0L;
}

static void kNode_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kNodeVar *node = (kNodeVar *)o;
	KRefTrace(node->TermToken);
	KRefTrace(node->RootNodeNameSpace);
	KRefTrace(node->StmtNameSpace);
	if(kNode_Is(ObjectConst, node)) {
		KRefTrace(node->ObjectConstValue);
	}
}


#ifndef USE_SMALLBUILD
static void kNodeTerm_p(KonohaContext *kctx, kObject *o, KonohaValue *values, int pos, KBuffer *wb)
{
	if(IS_Token(o)) {
		KBuffer_WriteTokenText(kctx, wb, (kToken *)o);
	}
	else {
		KUnsafeFieldSet(values[pos].asObject, o);
		kObject_class(o)->p(kctx, values, pos, wb);
	}
}

#define DefineCase(NAME)  case KNode_##NAME:   return #NAME;

static const char* KNode_text(knode_t node)
{
	switch(node) {
	KNodeList(DefineCase)
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


static void kNode_p(KonohaContext *kctx, KonohaValue *values, int pos, KBuffer *wb)
{
#ifndef USE_SMALLBUILD
	kNode *expr = values[pos].asNode;
	KBuffer_WriteIndent(kctx, wb, pos);
	KLIB KBuffer_Write(kctx, wb, "{", 1);
	if(expr->attrTypeId == KType_var) {
		if(expr->KeyOperatorToken == K_NULLTOKEN) {
			KLIB KBuffer_printf(kctx, wb, "nulltoken");
		}
		else {
			KLIB KBuffer_printf(kctx, wb, "%s%s %s ", KSymbol_Fmt2(expr->KeyOperatorToken->resolvedSyntaxInfo->keyword), KToken_t(expr->KeyOperatorToken));
		}
	}
	else {
		KLIB KBuffer_printf(kctx, wb, "%s %s :%s ", KNode_text(expr->node), KToken_t(expr->KeyOperatorToken), KType_text(expr->attrTypeId));
	}
	KLIB kObjectProto_p(kctx, values, pos, wb, 0);
	if(expr->node == KNode_Error) {
		KLIB KBuffer_printf(kctx, wb, "%s", kString_text(expr->ErrorMessage));
	}
	else if(expr->node == KNode_Const) {
		KLIB KBuffer_Write(kctx, wb, TEXTSIZE("const "));
		kNodeTerm_p(kctx, (kObject *)expr->ObjectConstValue, values, pos+1, wb);
	}
	else if(expr->node == KNode_New) {
		KLIB KBuffer_printf(kctx, wb, "new %s", KType_text(expr->attrTypeId));
	}
	else if(expr->node == KNode_Null) {
		KLIB KBuffer_Write(kctx, wb, TEXTSIZE("null"));
	}
	else if(expr->node == KNode_UnboxConst) {
		KLIB KBuffer_Write(kctx, wb, TEXTSIZE("const "));
		values[pos+1].unboxValue = expr->unboxConstValue;
		KClass_(expr->attrTypeId)->p(kctx, values, pos+1, wb);
	}
	else if(expr->node == KNode_Local) {
		KLIB KBuffer_printf(kctx, wb, "local sfp[%d]", (int)expr->index);
	}
	else if(expr->node == KNode_Field) {
		kshort_t index  = (kshort_t)expr->index;
		kshort_t xindex = (kshort_t)(expr->index >> (sizeof(kshort_t)*8));
		KLIB KBuffer_printf(kctx, wb, "field sfp[%d][%d]", (int)index, (int)xindex);
	}
	if(IS_Array(expr->NodeList)) {
		size_t i;
		KLIB KBuffer_Write(kctx, wb, "[", 1);
		for(i = 0; i < kArray_size(expr->NodeList); i++) {
//			if(i > 0) {
//				KLIB KBuffer_Write(kctx, wb, " ", 1);
//			}
			KLIB KBuffer_Write(kctx, wb, "\n", 1);
			KBuffer_WriteIndent(kctx, wb, pos+1);
			KLIB KBuffer_printf(kctx, wb, "#%d :%s ", (int)i, KClass_text(kObject_class(expr->NodeList->ObjectItems[i])));
			kNodeTerm_p(kctx, expr->NodeList->ObjectItems[i], values, pos+1, wb);
		}
		KLIB KBuffer_Write(kctx, wb, "]", 1);
	}
	KLIB KBuffer_Write(kctx, wb, "}", 1);
#endif
}

// untyped node

static kNode* new_UntypedNode(KonohaContext *kctx, kArray *gcstack, kNameSpace *ns)
{
	kNode *unode = (kNode *)new_(Node, ns, gcstack);
	return unode;
}

static void kNode_Reset(KonohaContext *kctx, kNode *node)
{
	//TODO
}

static kNode* kNode_Termnize(KonohaContext *kctx, kNode *node, kToken *termToken)
{
	KFieldSet(node, node->TermToken, termToken);
	node->syn = termToken->resolvedSyntaxInfo;
	return node;
}

static void kNode_AddParsedObject(KonohaContext *kctx, kNode *stmt, ksymbol_t keyid, kObject *o)
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
			valueList = newList;
			RESET_GCSTACK();
		}
		KLIB kArray_Add(kctx, valueList, o);
	}
	if(IS_Node(o)) {
		kNode *node = (kNode*)o;
		KFieldSet(node, node->Parent, stmt);
	}
}

static kNode* kNode_AddNode(KonohaContext *kctx, kNode *self, kNode *node)
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

static kNode* kNode_SetNodeAt(KonohaContext *kctx, kNode *self, size_t n, kNode *node)
{
	DBG_ASSERT(IS_Array(self->NodeList));
	DBG_ASSERT(n < kArray_size(self->NodeList));
	KFieldSet(self->NodeList, self->NodeList->NodeItems[n], node);
	KFieldSet(node, node->Parent, self);
	return self;
}

static void kNode_InsertAfter(KonohaContext *kctx, kNode *self, kNode *target, kNode *node)
{
	KFieldSet(node, ((kNodeVar *)node)->Parent, self);
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

static kNodeVar* kNode_AddSeveral(KonohaContext *kctx, kNodeVar *expr, int n, va_list ap)
{
	int i;
	if(!IS_Array(expr->NodeList)) {
		KFieldSet(expr, expr->NodeList, new_(Array, 0, OnField));
	}
	for(i = 0; i < n; i++) {
		kObject *v =  (kObject *)va_arg(ap, kObject *);
		if(v == NULL || v == (kObject *)K_NULLNODE) {
			return (kNodeVar *)K_NULLNODE;
		}
		KLIB kArray_Add(kctx, expr->NodeList, v);
	}
	return expr;
}

static kNode* kNode_Op(KonohaContext *kctx, kNode *node, kToken *keyToken, int n, ...)
{
	KFieldSet(node, node->KeyOperatorToken, keyToken);
	node->syn = keyToken->resolvedSyntaxInfo;
	KFieldSet(node, node->NodeList, new_(Array, 0, OnField));
	KLIB kArray_Add(kctx, node->NodeList, keyToken);
	va_list ap;
	va_start(ap, n);
	node = kNode_AddSeveral(kctx, node, n, ap);
	va_end(ap);
	return node;
}

static kNodeVar* new_UntypedOperatorNode(KonohaContext *kctx, kSyntax *syn, int n, ...)
{
	va_list ap;
	va_start(ap, n);
	DBG_ASSERT(syn != NULL);
	kNodeVar *expr = new_(NodeVar, syn, OnGcStack);
	expr = kNode_AddSeveral(kctx, expr, n, ap);
	va_end(ap);
	return expr;
}

static kNode* new_TypedNode(KonohaContext *kctx, kNameSpace *ns, int build, KClass *ty, int n, ...)
{
	kNode *node = new_(Node, ns, OnGcStack);
	va_list ap;
	va_start(ap, n);
	node = kNode_AddSeveral(kctx, node, n, ap);
	va_end(ap);
	node->node = build;
	node->attrTypeId = ty->typeId;
	return (kNode *)node;
}

static kNode *TypeCheckMethodParam(KonohaContext *kctx, kMethod *mtd, kNode *expr, kNameSpace *ns, KClass* reqc);

static kNode* new_MethodNode(KonohaContext *kctx, kNameSpace *ns, KClass *reqc, kMethod *mtd, int n, ...)
{
	kNode *expr = new_(Node, ns, OnGcStack);
	KFieldSet(expr, expr->NodeList, new_(Array, 0, OnField));
	KLIB kArray_Add(kctx, expr->NodeList, mtd);
	va_list ap;
	va_start(ap, n);
	expr = kNode_AddSeveral(kctx, expr, n, ap);
	va_end(ap);
	expr->node = KNode_MethodCall;
	return TypeCheckMethodParam(kctx, mtd, expr, ns, reqc);
}

static kNode* kNode_SetConst(KonohaContext *kctx, kNode *expr, KClass *typedClass, kObject *o)
{
	if(typedClass == NULL) typedClass = kObject_class(o);
	expr->attrTypeId = typedClass->typeId;
	if(KClass_Is(UnboxType, typedClass)) {
		expr->node = KNode_UnboxConst;
		expr->unboxConstValue = kNumber_ToInt(o);
	}
	else {
		expr->node = KNode_Const;
		KFieldInit(expr, expr->ObjectConstValue, o);
		kNode_Set(ObjectConst, expr, true);
	}
	return expr;
}

static kNode* kNode_SetUnboxConst(KonohaContext *kctx, kNode *expr, ktypeattr_t attrTypeId, uintptr_t unboxValue)
{
	expr->node = KNode_UnboxConst;
	expr->unboxConstValue = unboxValue;
	expr->attrTypeId = attrTypeId;
	kNode_Set(ObjectConst, expr, false);
	return expr;
}

static kNode* kNode_SetVariable(KonohaContext *kctx, kNode *expr, knode_t build, ktypeattr_t attrTypeId, intptr_t index)
{
	expr->node = build;
	expr->attrTypeId = attrTypeId;
	expr->index = index;
	kNode_Set(ObjectConst, expr, false);
	return expr;
}

static uintptr_t kNode_ParseFlag(KonohaContext *kctx, kNode *stmt, KFlagSymbolData *flagData, uintptr_t flag)
{
	while(flagData->flag != 0) {
		kObject *op = kNode_GetObjectNULL(kctx, stmt, flagData->symbol);
		if(op != NULL) {
			flag |= flagData->flag;
		}
		flagData++;
	}
	return flag;
}

static kToken* kNode_GetToken(KonohaContext *kctx, kNode *stmt, ksymbol_t kw, kToken *def)
{
	kToken *tk = (kToken *)kNode_GetObjectNULL(kctx, stmt, kw);
	if(tk != NULL && IS_Token(tk)) {
		return tk;
	}
	return def;
}

static kNode* kNode_GetNode(KonohaContext *kctx, kNode *stmt, ksymbol_t kw, kNode *def)
{
	kNode *expr = (kNode *)kNode_GetObjectNULL(kctx, stmt, kw);
	if(expr != NULL && IS_Node(expr)) {
		return expr;
	}
	return def;
}

