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

static void syntaxMap_Free(KonohaContext *kctx, void *p)
{
	KFree(p, sizeof(KSyntax));
}

static void kNameSpace_FreeSugarExtension(KonohaContext *kctx, kNameSpaceVar *ns)
{
	if(ns->syntaxMapNN != NULL) {
		KLIB KHashMap_Free(kctx, ns->syntaxMapNN, syntaxMap_Free);
	}
	if(ns->tokenMatrix != NULL) {
		KFree((void *)ns->tokenMatrix, SIZEOF_TOKENMATRIX);
	}
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
	tk->unresolvedTokenType = (ksymbol_t)(intptr_t)conf;
	if(tk->unresolvedTokenType == 0  || KSymbol_Unmask(tk->unresolvedTokenType) != tk->unresolvedTokenType) {
		KUnsafeFieldInit(tk->text, TS_EMPTY);
	}
	else {
		KUnsafeFieldInit(tk->text, KSymbol_GetString(kctx, tk->unresolvedTokenType));
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
		if(tk->unresolvedTokenType == TokenType_CODE) {
			return "{... }";
		}
		return kString_text(tk->text);
	}
	else {
		switch(tk->resolvedSymbol) {
			case TokenType_CODE:
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
	else if(IS_Array(tk->subTokenList)) {
		size_t i;
		kArray *a = tk->subTokenList;
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
	KRefTrace(node->Parent);
	KRefTrace(node->TermToken);
	if(kNode_Is(ObjectConst, node)) {
		KRefTrace(node->ObjectConstValue);
	}
}

static kNode* new_UntypedNode(KonohaContext *kctx, kArray *gcstack, KSyntax *syn, kToken *termNULL)
{
	kNode *unode = (kNode *)new_(Node, termNULL, gcstack);
	unode->syn = syn;
	return unode;
}

static kNode* new_TermNode(KonohaContext *kctx, kToken *tk)
{
	kNode *unode = new_UntypedNode(kctx, OnGcStack, tk->resolvedSyntaxInfo, tk);
	unode->uline = tk->uline;
	return unode;
}

static kNode* new_OperatorNode(KonohaContext *kctx, kToken *tk)
{
	kNode *unode = new_UntypedNode(kctx, OnGcStack, tk->resolvedSyntaxInfo, NULL);
	KFieldSet(unode, unode->NodeList, new_(Array, 0, OnField));
	KLIB kArray_Add(kctx, unode->NodeList, tk);
	unode->uline = tk->uline;
	return unode;
}

static void kNode_addNode(KonohaContext *kctx, kNode *self, kNode *node)
{
	if(self->NodeList == K_EMPTYARRAY) {
		KFieldSet(self, self->NodeList, new_(Array, 0, OnField));
	}
	DBG_ASSERT(IS_Array(self->NodeList));
	KLIB kArray_Add(kctx, self->NodeList, node);
	KFieldSet(node, node->Parent, self);
}

static void kNode_InsertAfter(KonohaContext *kctx, kNode *bk, kNode *target, kNode *stmt)
{
	KFieldSet(stmt, ((kNodeVar *)stmt)->Parent, bk);
	if(target != NULL) {
		size_t i;
		for(i = 0; i < kArray_size(bk->NodeList); i++) {
			if(bk->NodeList->NodeItems[i] == target) {
				KLIB kArray_Insert(kctx, bk->NodeList, i+1, stmt);
				return;
			}
		}
		DBG_ABORT("target was not found!!");
	}
	else {
		KLIB kArray_Add(kctx, bk->NodeList, stmt);
	}
}

static kNode* new_BlockNode(KonohaContext *kctx, kNameSpace *ns)
{
	KSyntax *syn = KSyntax_(ns, KSymbol_BlockPattern);
	kNode *unode = new_UntypedNode(kctx, OnGcStack, syn, NULL);
	return unode;
}

static kNode* new_StmtNode(KonohaContext *kctx, kNameSpace *ns, kToken *tk)
{
	kNode *unode = new_UntypedNode(kctx, OnGcStack, tk->resolvedSyntaxInfo, NULL);
	unode->uline = tk->uline;
	return unode;
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

static kNodeVar* new_UntypedCallStyleNode(KonohaContext *kctx, KSyntax *syn, int n, ...)
{
	va_list ap;
	va_start(ap, n);
	DBG_ASSERT(syn != NULL);
	kNodeVar *expr = new_(NodeVar, syn, OnGcStack);
	expr = kNode_AddSeveral(kctx, expr, n, ap);
	va_end(ap);
	return expr;
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
#endif

static void kNode_p(KonohaContext *kctx, KonohaValue *values, int pos, KBuffer *wb)
{
#ifndef USE_SMALLBUILD
	kNode *expr = values[pos].asNode;
	KLIB KBuffer_Write(kctx, wb, "(", 1);
	if(expr->node == KNode_Const) {
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
//	else if(expr->node == KNode_BLOCK) {
//		KLIB KBuffer_printf(kctx, wb, "block %d", expr->index);
//	}
	else if(expr->node == KNode_Field) {
		kshort_t index  = (kshort_t)expr->index;
		kshort_t xindex = (kshort_t)(expr->index >> (sizeof(kshort_t)*8));
		KLIB KBuffer_printf(kctx, wb, "field sfp[%d][%d]", (int)index, (int)xindex);
	}
//	else if(expr->node == KNode_STACKTOP) {
//		KLIB KBuffer_printf(kctx, wb, "stack %d", expr->index);
//	}
	else if(kNode_IsTerm(expr)) {
		KLIB KBuffer_Write(kctx, wb, TEXTSIZE("term "));
		kNodeTerm_p(kctx, (kObject *)expr->TermToken, values, pos+1, wb);
	}
	else if(IS_Array(expr->NodeList)) {
		size_t i;
		for(i = 0; i < kArray_size(expr->NodeList); i++) {
			if(i > 0) {
				KLIB KBuffer_Write(kctx, wb, " ", 1);
			}
			kNodeTerm_p(kctx, expr->NodeList->ObjectItems[i], values, pos+1, wb);
		}
	}
	KLIB KBuffer_Write(kctx, wb, ")", 1);
	if(expr->attrTypeId != KType_var) {
		KLIB KBuffer_printf(kctx, wb, ":%s", KType_text(expr->attrTypeId));
	}
#endif
}

//static void kNode_p(KonohaContext *kctx, KonohaValue *values, int pos, KBuffer *wb)
//{
//	kNode *stmt = values[pos].asNode;
//	if(stmt->syn == NULL) {
//		KLIB KBuffer_printf(kctx, wb, "DONE {uline: %d, ", (kshort_t)kNode_uline(stmt));
//	}
//	else {
//		KLIB KBuffer_printf(kctx, wb, "%s%s {uline: %d, ", KSymbol_Fmt2(stmt->syn->keyword), (kshort_t)kNode_uline(stmt));
//	}
//	KLIB kObjectProto_p(kctx, values, pos, wb, 0);
//	KLIB KBuffer_Write(kctx, wb, "}", 1);
//}

//static kNode* new_TypedConsNode(KonohaContext *kctx, int build, KClass *ty, int n, ...)
//{
//	kNodeVar *expr = new_(NodeVar, NULL, OnGcStack);
//	va_list ap;
//	va_start(ap, n);
//	expr = kNode_AddSeveral(kctx, expr, n, ap);
//	va_end(ap);
//	expr->node = build;
//	expr->attrTypeId = ty->typeId;
//	return (kNode *)expr;
//}
//
//static kNode *kNodekNode_TypeCheckCallParam(KonohaContext *kctx, kNode *stmt, kNodeVar *expr, kMethod *mtd, kGamma *gma, KClass *reqClass);
//
//static kNode* new_TypedCallNode(KonohaContext *kctx, kNode *stmt, kGamma *gma, KClass *reqClass, kMethod *mtd, int n, ...)
//{
//	va_list ap;
//	va_start(ap, n);
//	kNodeVar *expr = new_(NodeVar, NULL, OnGcStack);
//	KFieldSet(expr, expr->NodeList, new_(Array, 8, OnField));
//	KLIB kArray_Add(kctx, expr->NodeList, mtd);
//	expr = kNode_AddSeveral(kctx, expr, n, ap);
//	va_end(ap);
//	expr->node = KNode_MethodCall;
//	//expr->attrTypeId = reqClass->typeId;
//	return kNodekNode_TypeCheckCallParam(kctx, stmt, expr, mtd, gma, reqClass);
//}
//
//static kNode* kNode_Add(KonohaContext *kctx, kNode *expr, kNode *e)
//{
//	DBG_ASSERT(IS_Array(expr->NodeList));
//	if(expr != K_NULLNODE && e != NULL && e != K_NULLNODE) {
//		KLIB kArray_Add(kctx, expr->NodeList, e);
//		return expr;
//	}
//	return K_NULLNODE;
//}

static kNode* SUGAR kNode_SetConstValue(KonohaContext *kctx, kNodeVar *expr, KClass *typedClass, kObject *o)
{
	expr = (expr == NULL) ? new_(NodeVar, 0, OnGcStack) : expr;
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
	return (kNode *)expr;
}

static kNode* SUGAR kNode_SetUnboxConstValue(KonohaContext *kctx, kNodeVar *expr, ktypeattr_t attrTypeId, uintptr_t unboxValue)
{
	expr = (expr == NULL) ? new_(NodeVar, 0, OnGcStack) : (kNodeVar *)expr;
	expr->node = KNode_UnboxConst;
	expr->unboxConstValue = unboxValue;
	expr->attrTypeId = attrTypeId;
	return (kNode *)expr;
}

static kNode* SUGAR kNode_SetVariable(KonohaContext *kctx, kNodeVar *expr, kGamma *gma, knode_t build, ktypeattr_t attrTypeId, intptr_t index)
{
	expr = (expr == NULL) ? new_(NodeVar, 0, OnGcStack) : (kNodeVar *)expr;
	expr->node = build;
	expr->attrTypeId = attrTypeId;
	expr->index = index;
//	if(build == KNode_Local && gma->genv->blockScopeShiftSize > 0 && index >= gma->genv->blockScopeShiftSize) {
//		expr->node = KNode_STACKTOP;
//		expr->index -= gma->genv->blockScopeShiftSize;
//	}
	return (kNode *)expr;
}

//static kNodeVar* new_BlockNode(KonohaContext *kctx, kArray *gcstack, KSyntax *syn, ...)
//{
//	kNodeVar *stmt = new_(NodeVar, 0, gcstack);
//	stmt->syn = syn;
//	va_list ap;
//	va_start(ap, syn);
//	ksymbol_t kw = (ksymbol_t) va_arg(ap, int); /* 'ksymbol_t' is promoted to 'int' through to 'va_arg' */
//	while(kw != 0) {
//		kObject *v = va_arg(ap, kObject *);
//		if(v == NULL) break;
//		kNode_SetObject(kctx, stmt, kw, v);
//		kw = (ksymbol_t) va_arg(ap, int);
//	}
//	va_end(ap);
//	return stmt;
//}

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

static const char* kNode_GetText(KonohaContext *kctx, kNode *stmt, ksymbol_t kw, const char *def)
{
	kNode *expr = (kNode *)kNode_GetObjectNULL(kctx, stmt, kw);
	if(expr != NULL) {
		if(IS_Node(expr) && kNode_IsTerm(expr)) {
			return kString_text(expr->TermToken->text);
		}
		else if(IS_Token(expr)) {
			kToken *tk = (kToken *)expr;
			if(IS_String(tk->text)) return kString_text(tk->text);
		}
	}
	return def;
}

/* --------------- */
/* Gamma */

static void Gamma_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kGammaVar *gma = (kGammaVar *)o;
	gma->genv = NULL;
}

