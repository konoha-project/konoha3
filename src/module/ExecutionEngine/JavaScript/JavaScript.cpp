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

#define USE_EXECUTIONENGINE
#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/klib.h>
#include <konoha3/import/module.h>

#undef HAVE_LIBV8
#ifdef HAVE_LIBV8
#include <v8.h>
#endif

#define LOG_FUNCTION_NAME "p"

extern "C" {

#ifdef HAVE_LIBV8

static v8::Handle<v8::Value> JSLog(const v8::Arguments& args)
{
	if(args.Length() < 1) {
		return v8::Undefined();
	}

	v8::HandleScope scope;
	v8::Handle<v8::Value> arg = args[0];
	v8::String::Utf8Value value(arg);
	PLATAPI printf_i("%s\n", *value);

	return v8::Undefined();
}

v8::Persistent<v8::Context> createContext()
{
	v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();
	global->Set(v8::String::New(LOG_FUNCTION_NAME), v8::FunctionTemplate::New(JSLog));
	return v8::Context::New(NULL, global);
}

class JSContext {
public:
	v8::HandleScope handleScope;
	v8::Persistent<v8::Context> context;
	v8::Context::Scope contextScope;
	JSContext():context(createContext()),contextScope(v8::Context::Scope(context)){
	}
	~JSContext(){
		context.Dispose();
	}
};

static JSContext *globalJSContext = NULL;

#endif

typedef struct JSBuilder {
	struct KBuilderCommon common;
	kbool_t isIndentEmitted;
	kbool_t isExprNode;
	kMethod *visitingMethod;
	int indent;
	KGrowingArray buffer;
	KBuffer jsCodeBuffer;
} JSBuilder;

/* ------------------------------------------------------------------------ */

#define MN_opNOT  KMethodName_("!")
#define MN_opADD  KMethodName_("+")
#define MN_opSUB  KMethodName_("-")
#define MN_opMUL  KMethodName_("*")
#define MN_opDIV  KMethodName_("/")
#define MN_opMOD  KMethodName_("%")
#define MN_opEQ   KMethodName_("==")
#define MN_opNEQ  KMethodName_("!=")
#define MN_opLT   KMethodName_("<")
#define MN_opLTE  KMethodName_("<=")
#define MN_opGT   KMethodName_(">")
#define MN_opGTE  KMethodName_(">=")
#define MN_opLAND KMethodName_("&")
#define MN_opLOR  KMethodName_("|")
#define MN_opLXOR KMethodName_("^")
#define MN_opLSFT KMethodName_("<<")
#define MN_opRSFT KMethodName_(">>")

enum kSymbolPrefix{
	kSymbolPrefix_NONE,
	kSymbolPrefix_SET,
	kSymbolPrefix_GET,
	kSymbolPrefix_AT,
	kSymbolPrefix_IS,
	kSymbolPrefix_UNKNOWN,
	kSymbolPrefix_TO,
	kSymbolPrefix_DOLLAR,
};

static enum kSymbolPrefix KSymbol_prefixText_ID(ksymbol_t sym)
{
	size_t mask = ((size_t)(KSymbol_Attr(sym)) >> ((sizeof(ksymbol_t) * 8)-3));
	DBG_ASSERT(mask < 8);
	return (enum kSymbolPrefix)mask;
}

static int KMethodName_isBinaryOperator(KonohaContext *kctx, kmethodn_t mn)
{
	if(mn == MN_opADD ) return 1;
	if(mn == MN_opSUB ) return 1;
	if(mn == MN_opMUL ) return 1;
	if(mn == MN_opDIV ) return 1;
	if(mn == MN_opMOD ) return 1;
	if(mn == MN_opEQ  ) return 1;
	if(mn == MN_opNEQ ) return 1;
	if(mn == MN_opLT  ) return 1;
	if(mn == MN_opLTE ) return 1;
	if(mn == MN_opGT  ) return 1;
	if(mn == MN_opGTE ) return 1;
	if(mn == MN_opLAND) return 1;
	if(mn == MN_opLOR ) return 1;
	if(mn == MN_opLXOR) return 1;
	if(mn == MN_opLSFT) return 1;
	if(mn == MN_opRSFT) return 1;
	return 0;
}

static int KMethodName_isUnaryOperator(KonohaContext *kctx, kmethodn_t mn)
{
	if(mn == MN_opSUB) return 1;
	if(mn == MN_opNOT) return 1;
	return 0;
}

static kbool_t kNode_isStmt(KonohaContext *kctx, kNode *node)
{
	switch(kNode_node(node)){
		case KNode_Block:
		case KNode_If:
		case KNode_While:
		case KNode_DoWhile:
		case KNode_Return:
		case KNode_Break:
		case KNode_Continue:
		case KNode_Try:
		case KNode_Throw:
			return true;
	}
	return false;
}

static kbool_t kNode_isExpr(KonohaContext *kctx, kNode *node)
{
	return !kNode_isStmt(kctx, node);
}

#if 0
static const char *GetNodeTypeName(kNode *node)
{
	switch(kNode_node(node)){
#define RETURN_NODE_TYPE(NAME) case KNode_##NAME: return #NAME;
		KNodeList(RETURN_NODE_TYPE);
#undef RETURN_NODE_TYPE
	}
	return "";
}
#endif

static void JSBuilder_EmitIndent(KonohaContext *kctx, KBuilder *builder)
{
	JSBuilder *jsBuilder = (JSBuilder *)builder;
	if(!jsBuilder->isIndentEmitted) {
		int i;
		for(i = 0; i < jsBuilder->indent; i++) {
			PLATAPI printf_i("    ");
		}
		jsBuilder->isIndentEmitted = true;
	}
}

static void JSBuilder_EmitNewLineWith(KonohaContext *kctx, KBuilder *builder, const char *endline)
{
	JSBuilder *jsBuilder = (JSBuilder *)builder;
	JSBuilder_EmitIndent(kctx, builder);
	jsBuilder->isIndentEmitted = false;
	PLATAPI printf_i("%s\n", endline);
}

static void JSBuilder_EmitString(KonohaContext *kctx, KBuilder *builder, const char *prefix, const char *str, const char *suffix)
{
	JSBuilder_EmitIndent(kctx, builder);
	PLATAPI printf_i("%s%s%s", prefix, str, suffix);
}

static void JSBuilder_EmitNewLineWithEndOfStatement(KonohaContext *kctx, KBuilder *builder, kNode *node)
{
	if(kNode_node(node) == KNode_Block) {
		JSBuilder_EmitNewLineWith(kctx, builder, ")();");
	}
	else if(kNode_isExpr(kctx, node)){
		JSBuilder_EmitNewLineWith(kctx, builder, ";");
	}
	else {
		JSBuilder_EmitNewLineWith(kctx, builder, "");
	}
}

static void JSBuilder_EmitAssignStmtPrefix(KonohaContext *kctx, KBuilder *builder, kNode *node)
{
	if(kNode_node(kNode_At(node, 1)) == KNode_Field){
		JSBuilder_EmitString(kctx, builder, "this.", "", "");
	}
	else {
		JSBuilder_EmitString(kctx, builder, "var ", "", "");
	}
}

static kbool_t JSBuilder_VisitNodeList(KonohaContext *kctx, KBuilder *builder, kNode *block, void *thunk)
{
	JSBuilder *jsBuilder = (JSBuilder *)builder;
	size_t i;
	for (i = 0; i < kArray_size(block->NodeList); ++i) {
		kNode *node = block->NodeList->NodeItems[i];

		if(node == K_NULLNODE || kNode_node(node) == KNode_Done) {
			continue;
		}

		while(kNode_node(node) == KNode_Block && kArray_size(node->NodeList) == 1) {
			node = node->NodeList->NodeItems[0];
		}

		if(kNode_node(node) == KNode_Assign) {
			JSBuilder_EmitAssignStmtPrefix(kctx, builder, node);	
		}

		jsBuilder->isExprNode = kNode_isExpr(kctx, node);
		kbool_t ret = SUGAR VisitNode(kctx, builder, node, thunk);

		JSBuilder_EmitNewLineWithEndOfStatement(kctx, builder, node);

		if(!ret) return false;
	}
	return true;
}

static kbool_t JSBuilder_VisitBlockNode(KonohaContext *kctx, KBuilder *builder, kNode *block, void *thunk)
{
	JSBuilder *jsBuilder = (JSBuilder *)builder;
	kbool_t isExprBlock = jsBuilder->isExprNode;
	DBG_ASSERT(kNode_node(block) == KNode_Block);
	if(!IS_Array(block->NodeList)) {
		return true;
	}
	if(!kNode_IsRootNode(block)) {
		if(kNode_node(kNode_GetParent(kctx, block)) == 0) {
			// Closure
			return true;
		}
		JSBuilder_EmitNewLineWith(kctx, builder, isExprBlock ? "(function() {" : "{");
		jsBuilder->indent++;
	}
	kbool_t ret = JSBuilder_VisitNodeList(kctx, builder, block, thunk);	
	if(!kNode_IsRootNode(block)) {
		jsBuilder->indent--;
		JSBuilder_EmitString(kctx, builder, isExprBlock ? "})()" : "}", "", "");
	}
	return ret;
}

static kbool_t JSBuilder_VisitExprNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	((JSBuilder *)builder)->isExprNode = true;
	//JSBuilder_EmitString(kctx, builder, "/*", GetNodeTypeName(node), "*/");
	return SUGAR VisitNode(kctx, builder, node, thunk);
}

static kbool_t JSBuilder_VisitStmtNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	((JSBuilder *)builder)->isExprNode = false;
	if(kNode_node(node) == KNode_Assign) {
		if(kNode_node(kNode_At(node, 1)) == KNode_Field){
			JSBuilder_EmitString(kctx, builder, "this.", "", "");
		}
		else {
			JSBuilder_EmitString(kctx, builder, "var ", "", "");
		}
	}
	return SUGAR VisitNode(kctx, builder, node, thunk);
}

static kbool_t JSBuilder_VisitNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk, const char *prefix, const char *suffix)
{
	JSBuilder_EmitString(kctx, builder, prefix, "", "");
	kbool_t ret = JSBuilder_VisitExprNode(kctx, builder, expr, thunk);
	JSBuilder_EmitString(kctx, builder, suffix, "", "");
	return ret;
}

static kbool_t JSBuilder_VisitPushNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	JSBuilder_VisitExprNode(kctx, builder, expr->NodeToPush, thunk);
	return true;
}

static kbool_t JSBuilder_VisitBoxNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	JSBuilder_VisitExprNode(kctx, builder, expr->NodeToPush, thunk);
	return true;
}

static kbool_t JSBuilder_VisitErrorNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	return true;
}

static kbool_t JSBuilder_VisitReturnNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	JSBuilder *jsBuilder = (JSBuilder *)builder;
	kNode *expr = kNode_getFirstNode(kctx, stmt);
	kbool_t isInFunction = (jsBuilder->visitingMethod->mn != 0);
	if(isInFunction) {
		JSBuilder_EmitString(kctx, builder, "return", "", "");
	}
	if(expr != NULL && IS_Node(expr)) {
		if(isInFunction) {
			JSBuilder_EmitString(kctx, builder, " ", "", "");
		}
		if(kNode_node(expr) == KNode_Block && kArray_size(expr->NodeList) == 1) {
			kNode *firstNode = expr->NodeList->NodeItems[0];
			jsBuilder->isExprNode = kNode_isExpr(kctx, firstNode);
			SUGAR VisitNode(kctx, builder, firstNode, thunk);
		}
		else {
			JSBuilder_VisitExprNode(kctx, builder, expr, thunk);
		}
	}
	JSBuilder_EmitNewLineWith(kctx, builder, ";");
	return true;
}

static kbool_t JSBuilder_VisitIfNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	JSBuilder_VisitNode(kctx, builder, kNode_getFirstNode(kctx, stmt), thunk, "if(", ") ");
	JSBuilder_VisitStmtNode(kctx, builder, kNode_getFirstBlock(kctx, stmt), thunk);
	kNode *elseNode = kNode_getElseBlock(kctx, stmt);
	if(elseNode != K_NULLBLOCK) {
		JSBuilder_EmitString(kctx, builder, "else ", "", "");
		JSBuilder_VisitStmtNode(kctx, builder, elseNode, thunk);
	}
	return true;
}

static kbool_t JSBuilder_VisitWhileNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	JSBuilder_VisitNode(kctx, builder, kNode_getFirstNode(kctx, stmt), thunk, "while(", ") ");
	JSBuilder_VisitStmtNode(kctx, builder, kNode_getFirstBlock(kctx, stmt), thunk);
	return true;
}

static kbool_t JSBuilder_VisitDoWhileNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	JSBuilder_EmitString(kctx, builder, "do", "", "");
	JSBuilder_VisitStmtNode(kctx, builder, kNode_getFirstBlock(kctx, stmt), thunk);
	JSBuilder_VisitNode(kctx, builder, kNode_getFirstNode(kctx, stmt), thunk, "while(", ");");
	return true;
}

static kbool_t JSBuilder_VisitForNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	kNode *initNode = SUGAR kNode_GetNode(kctx, stmt, KSymbol_("init"), NULL) ;
	kNode *iterNode = SUGAR kNode_GetNode(kctx, stmt, KSymbol_("Iterator"), NULL) ;
	JSBuilder_EmitString(kctx, builder, "for(", "", "");
	if(initNode != NULL) {
		JSBuilder_VisitStmtNode(kctx, builder, initNode, thunk);
	}
	JSBuilder_EmitString(kctx, builder, ";", "", "");
	JSBuilder_VisitExprNode(kctx, builder, kNode_getFirstNode(kctx, stmt), thunk);
	JSBuilder_EmitString(kctx, builder, ";", "", "");
	if(iterNode != NULL) {
		JSBuilder_VisitStmtNode(kctx, builder, iterNode, thunk);
	}
	JSBuilder_EmitString(kctx, builder, ") ", "", "");
	JSBuilder_VisitStmtNode(kctx, builder, kNode_getFirstBlock(kctx, stmt), thunk);
	return true;
}

static kbool_t JSBuilder_VisitBreakNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	JSBuilder_EmitString(kctx, builder, "break;", "", "");
	return true;
}

static kbool_t JSBuilder_VisitContinueNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	JSBuilder_EmitString(kctx, builder, "continue;", "", "");
	return true;
}

static kbool_t JSBuilder_VisitThrowNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	JSBuilder_VisitNode(kctx, builder, kNode_getFirstNode(kctx, node), thunk, "throw ", ";");
	return true;
}

static kbool_t JSBuilder_VisitTryNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	JSBuilder_EmitNewLineWith(kctx, builder, "try ");
	SUGAR VisitNode(kctx, builder, kNode_getFirstBlock(kctx, stmt), thunk);
	kNode *catchNode   = SUGAR kNode_GetNode(kctx, stmt, KSymbol_("catch"),   K_NULLBLOCK);
	kNode *finallyNode = SUGAR kNode_GetNode(kctx, stmt, KSymbol_("finally"), K_NULLBLOCK);
	if(catchNode != K_NULLBLOCK) {
		JSBuilder_EmitString(kctx, builder, "catch(e) ", "", "");
		JSBuilder_VisitStmtNode(kctx, builder, catchNode, thunk);
	}
	if(finallyNode != K_NULLBLOCK) {
		JSBuilder_EmitString(kctx, builder, "finally", "", "");
		JSBuilder_VisitStmtNode(kctx, builder, finallyNode, thunk);
	}
	return true;
}

static void JSBuilder_EmitKonohaValue(KonohaContext *kctx, KBuilder *builder, KClass *ct, KonohaStack *sfp)
{
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	ct->format(kctx, sfp, 0, &wb);
	char *str = (char *)KLIB KBuffer_text(kctx, &wb, NonZero);
	JSBuilder_EmitString(kctx, builder, str, "", "");
	KLIB KBuffer_Free(&wb);
}

static void JSBuilder_EmitConstValue(KonohaContext *kctx, KBuilder *builder, kObject *obj)
{
	KonohaStack sfp[1];
	sfp[0].asObject = obj;
	JSBuilder_EmitKonohaValue(kctx, builder, kObject_class(obj), sfp);
}

static void JSBuilder_EmitUnboxConstValue(KonohaContext *kctx, KBuilder *builder, KClass *ct, unsigned long long unboxVal)
{
	KonohaStack sfp[1];
	sfp[0].unboxValue = unboxVal;
	JSBuilder_EmitKonohaValue(kctx, builder, ct, sfp);
}

static kbool_t JSBuilder_VisitConstNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	JSBuilder_EmitConstValue(kctx, builder, node->ObjectConstValue);
	return true;
}

static kbool_t JSBuilder_VisitUnboxConstNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	JSBuilder_EmitUnboxConstValue(kctx, builder, KClass_(node->attrTypeId), node->unboxConstValue);
	return true;
}

static kbool_t JSBuilder_VisitNewNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	JSBuilder_EmitString(kctx, builder, "new", "", "");
	return true;
}

static kbool_t JSBuilder_VisitNullNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	JSBuilder_EmitString(kctx, builder, "null", "", "");
	return true;
}

static kbool_t JSBuilder_VisitLocalNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	kToken *tk = (kToken *)node->TermToken;
	JSBuilder_EmitString(kctx, builder, kString_text(tk->text), "", "");
	return true;
}

static kbool_t JSBuilder_VisitFieldNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	kToken *tk = (kToken *)node->TermToken;
	JSBuilder_EmitString(kctx, builder, kString_text(tk->text), "", "");
	return true;
}

kbool_t LoadJavaScriptModule(KonohaFactory *factory, ModuleType type);
static void compileAllDefinedMethodsInNameSpace(KonohaContext *kctx, kNameSpace *ns);

static bool JSBuilder_importPackage(KonohaContext *kctx, kNameSpace *ns, kString *package, kfileline_t uline)
{
	KBaseTrace(trace);
	KonohaFactory *factory = (KonohaFactory *)kctx->platApi;

	//factory->DeleteVirtualMachine(kctx);
	SUGAR kNameSpace_UseDefaultVirtualMachine(kctx, ns);

	KImportPackage(ns, kString_text(package), trace);
	compileAllDefinedMethodsInNameSpace(kctx, ns);

	//factory->DeleteVirtualMachine(kctx);
	LoadJavaScriptModule(factory, ReleaseModule);
	ns->builderApi = factory->ExecutionEngineModule.GetDefaultBuilderAPI();
	return true;
}

static bool JSBuilder_loadScript(KonohaContext *kctx, kNameSpace *ns, kString *package, kfileline_t uline)
{
	KBaseTrace(trace);
	char pathbuf[512];
	const char *path = PLATAPI formatTransparentPath(pathbuf, sizeof(pathbuf), KFileLine_textFileName(trace->pline), kString_text(package));

	KLIB kNameSpace_LoadScript(kctx, ns, path, trace);

	KonohaFactory *factory = (KonohaFactory *)kctx->platApi;
	ns->builderApi = factory->ExecutionEngineModule.GetDefaultBuilderAPI();
	return true;
}

static kbool_t JSBuilder_VisitNodeParams(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk, int beginIndex, const char *delimiter, const char *leftBracket, const char *rightBracket)
{
	unsigned n = kArray_size(expr->NodeList);
	unsigned i;
	if(leftBracket) {
		JSBuilder_EmitString(kctx, builder, leftBracket, "", "");
	}
	for(i = beginIndex; i < n;) {
		JSBuilder_VisitExprNode(kctx, builder, kNode_At(expr, i), thunk);
		if(++i < n) {
			JSBuilder_EmitString(kctx, builder, delimiter, "", "");
		}
	}
	if(rightBracket) {
		JSBuilder_EmitString(kctx, builder, rightBracket, "", "");
	}
	return true;
}

static void JSBuilder_ConvertAndEmitMethodName(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk, kNode *receiver, kMethod *mtd)
{
	KClass *globalObjectClass = KLIB kNameSpace_GetClassByFullName(kctx, kNode_ns(expr), "GlobalObject", 12, NULL);
	kbool_t isGlobal = (KClass_(receiver->attrTypeId) == globalObjectClass || receiver->attrTypeId == KType_NameSpace);
	const char *methodName = KSymbol_text(mtd->mn);
	const char *className = KSymbol_text(KClass_(receiver->attrTypeId)->classNameSymbol);
	if(receiver->attrTypeId == KType_NameSpace) {
		if(mtd->mn == KMethodName_("import")) {
			kString *packageNameString = (kString *)kNode_At(expr, 2)->ObjectConstValue;
			kNameSpace *ns = (kNameSpace *)receiver->ObjectConstValue;
			JSBuilder_importPackage(kctx, ns, packageNameString, expr->TermToken->uline);
			JSBuilder_EmitString(kctx, builder, "//import", "", "");
			return;
		}
		else if(mtd->mn == KMethodName_("load") || mtd->mn == KMethodName_("include")) {
			kString *packageNameString = (kString *)kNode_At(expr, 2)->ObjectConstValue;
			kNameSpace *ns = (kNameSpace *)receiver->ObjectConstValue;
			JSBuilder_loadScript(kctx, ns, packageNameString, expr->TermToken->uline);
			JSBuilder_EmitString(kctx, builder, "//load", "", "");
			return;
		}
	}
	if(receiver->attrTypeId == KType_System && methodName[0] == 'p') {
		// System.p
		JSBuilder_EmitString(kctx, builder, LOG_FUNCTION_NAME, "", "");
	}
	else if(strcmp(methodName, "new") == 0) {
		if(strcmp(className, "Map") == 0) {
			JSBuilder_EmitString(kctx, builder, "new Object", "", "");
		}
		else {
			JSBuilder_EmitString(kctx, builder, "new ", className, "");
		}
	}
	//else if(strcmp(methodName, "newList") == 0) {
	//}
	else if(strcmp(methodName, "[]") == 0) {
		// [1, 2, 3];
		//JSBuilder_EmitString(kctx, builder, "new Array", "", "");
	}
	else if(strcmp(methodName, "newArray") == 0) {
		// [1, 2, 3];
		//JSBuilder_EmitString(kctx, builder, "new Array", "", "");
	}
	else {
		// Normal functions
		if(!isGlobal) {
			if(kNode_node(receiver) == KNode_Null) {
				// Static methods
				JSBuilder_EmitString(kctx, builder, KClass_text(KClass_(receiver->attrTypeId)), "", "");
			}
			else {
				// Instance methods
				JSBuilder_VisitExprNode(kctx, builder, receiver, thunk);
			}
		}
		kbool_t isReceiverClosure = strcmp(className, "Func") == 0;
		int prefix = KSymbol_prefixText_ID(mtd->mn);
		switch(prefix) {
		case kSymbolPrefix_GET:
			if(kArray_size(expr->NodeList) > 2) {
				if(strlen(methodName) != 0) {
					if(!isGlobal){
						JSBuilder_EmitString(kctx, builder, ".", "", "");
					}
					JSBuilder_EmitString(kctx, builder, "get", methodName, "");
					break;
				}
				JSBuilder_VisitNode(kctx, builder, kNode_At(expr, 2), thunk, "[", "]");
			}
			else {
				if(!isGlobal && !isReceiverClosure) {
					JSBuilder_EmitString(kctx, builder, ".", "", "");
				}
				JSBuilder_EmitString(kctx, builder, methodName, "", "");
			}
			break;
		case kSymbolPrefix_SET:
			if(kArray_size(expr->NodeList) > 3) {
				JSBuilder_VisitNode(kctx, builder, kNode_At(expr, 2), thunk, "[", "]");
			}
			else {
				if(isGlobal) {
					JSBuilder_EmitString(kctx, builder, "var ", "", "");
				}
				else if(!isReceiverClosure) {
					JSBuilder_EmitString(kctx, builder, ".", "", "");
				}
			}
			JSBuilder_EmitString(kctx, builder, methodName, " = ", "");
			break;
		case kSymbolPrefix_TO:
			if(strcmp(className, "float") == 0 && strcmp(methodName, "Number") == 0) {
				JSBuilder_EmitString(kctx, builder, " | 0", "", "");
			}
			break;
		default:
			if(strcmp(className, "Func") == 0) {
				// Invoke closure
			}
			else {
				if(!isGlobal){
					JSBuilder_EmitString(kctx, builder, ".", "", "");
				}
				JSBuilder_EmitString(kctx, builder, methodName, "", "");
			}
			break;
		}
	}
}

static kbool_t JSBuilder_VisitMethodCallNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	kMethod *mtd = CallNode_getMethod(node);
	kbool_t isArray = false;
	size_t nodeListSize = kArray_size(node->NodeList);

	if(nodeListSize == 2 && KMethodName_isUnaryOperator(kctx, mtd->mn)) {
		JSBuilder_EmitString(kctx, builder, KMethodName_Fmt2(mtd->mn), "(");
		JSBuilder_VisitExprNode(kctx, builder, kNode_At(node, 1), thunk);
		JSBuilder_EmitString(kctx, builder, ")", "", "");
	}
	else if(KMethodName_isBinaryOperator(kctx, mtd->mn)) {
		if(mtd->mn == MN_opDIV && kNode_At(node, 1)->attrTypeId == KType_Int && kNode_At(node, 2)->attrTypeId == KType_Int) {
			JSBuilder_VisitNodeParams(kctx, builder, node, thunk, 1, KSymbol_text(mtd->mn),"((", ")|0)");
		}
		else {
			JSBuilder_VisitNodeParams(kctx, builder, node, thunk, 1, KSymbol_text(mtd->mn),"(", ")");
		}
	}
	else {
		kNode *receiver = kNode_At(node, 1);
		if(strcmp(KSymbol_text(mtd->mn), "[]") == 0 || strcmp(KSymbol_text(mtd->mn), "newArray") == 0) {
			isArray = true;
		}
		else {
			JSBuilder_ConvertAndEmitMethodName(kctx, builder, node, thunk, receiver, mtd);
		}
		switch(KSymbol_prefixText_ID(mtd->mn)) {
		case kSymbolPrefix_GET:
			if(kArray_size(node->NodeList) > 2) {
				if(strlen(KSymbol_text(mtd->mn)) != 0) {
					JSBuilder_VisitNodeParams(kctx, builder, node, thunk, 2, ", ", isArray ? "[" : "(", isArray ? "]" : ")");
				}
			}
			break;
		case kSymbolPrefix_TO:
			break;
		case kSymbolPrefix_SET:
			JSBuilder_VisitExprNode(kctx, builder, kNode_At(node, nodeListSize > 3 ? 3 : 2), thunk);
			break;
		default:
			JSBuilder_VisitNodeParams(kctx, builder, node, thunk, 2, ", ", isArray ? "[" : "(", isArray ? "]" : ")");
			break;
		}
	}
	return true;
}

static kbool_t JSBuilder_VisitAndNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	JSBuilder_VisitNodeParams(kctx, builder, node, thunk, 1, " && ", "(", ")");
	return true;
}

static kbool_t JSBuilder_VisitOrNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	JSBuilder_VisitNodeParams(kctx, builder, node, thunk, 1, " || ", "(", ")");
	return true;
}

static kbool_t JSBuilder_VisitAssignNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	JSBuilder_VisitNodeParams(kctx, builder, node, thunk, 1, " = ", NULL, NULL);
	return true;
}

static kbool_t JSBuilder_VisitDoneNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	return true;
}

static void JSBuilder_EmitMethodHeader(KonohaContext *kctx, KBuilder *builder, kMethod *mtd);

static kbool_t JSBuilder_VisitFunctionNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	kMethod *mtd = CallNode_getMethod(expr);
	JSBuilder_EmitMethodHeader(kctx, builder, mtd);
	JSBuilder_VisitStmtNode(kctx, builder, kNode_getFirstBlock(kctx, expr), thunk);
	JSBuilder_EmitString(kctx, builder, ")", "", "");
	return true;
}

static void compileAllDefinedMethodsInNameSpace(KonohaContext *kctx, kNameSpace *ns)
{
	size_t i;
	for(i = 0; i < kArray_size(ns->methodList_OnList); i++) {
		kMethod *mtd = ns->methodList_OnList->MethodItems[i];
		if(IS_NameSpace(mtd->LazyCompileNameSpace)) {
			KLIB kMethod_DoLazyCompilation(kctx, mtd, NULL, HatedLazyCompile|CrossCompile);
		}
	}
}

static void compileAllDefinedMethods(KonohaContext *kctx)
{
	KRuntime *share = kctx->share;
	size_t i;
	for(i = 0; i < kArray_size(share->GlobalConstList); i++) {
		kObject *object = share->GlobalConstList->ObjectItems[i];
		if(kObject_class(object) == KClass_NameSpace) {
			compileAllDefinedMethodsInNameSpace(kctx, (kNameSpace  *)object);
		}
	}
}

static void JSBuilder_EmitExtendFunctionCode(KonohaContext *kctx, KBuilder *builder)
{
	JSBuilder *self = (JSBuilder *)builder;
	JSBuilder_EmitNewLineWith(kctx, builder, "var __extends = this.__extends || function (d, b) {");
	self->indent++;
	JSBuilder_EmitNewLineWith(kctx, builder, "function __() { this.constructor = d; }");
	JSBuilder_EmitNewLineWith(kctx, builder, "__.prototype = b.prototype;");
	JSBuilder_EmitNewLineWith(kctx, builder, "d.prototype = new __();");
	self->indent--;
	JSBuilder_EmitNewLineWith(kctx, builder, "}");
}

static kbool_t JSBuilder_VisitClassFields(KonohaContext *kctx, KBuilder *builder, KClass *kclass)
{
	kushort_t i;
	KClassField *field = kclass->fieldItems;
	kObject *constList = kclass->defaultNullValue;
	for(i = 0; i < kclass->fieldsize; ++i) {
		JSBuilder_EmitString(kctx, builder, "this.", KSymbol_text(field[i].name), " = ");
		if(KType_Is(UnboxType, field[i].attrTypeId)) {
			JSBuilder_EmitUnboxConstValue(kctx, builder, KClass_(field[i].attrTypeId), constList->fieldUnboxItems[i]);
		}else {
			JSBuilder_EmitConstValue(kctx, builder, constList->fieldObjectItems[i]);
		}
		JSBuilder_EmitNewLineWith(kctx, builder, ";");
	}
	return true;
}

static void JSBuilder_EmitMethodHeader(KonohaContext *kctx, KBuilder *builder, kMethod *mtd)
{
	KClass *kclass = KClass_(mtd->typeId);
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	kParam *params = kMethod_GetParam(mtd);
	const char *shortMethodName = KSymbol_text(mtd->mn);
	unsigned int i;
	if(strcmp(shortMethodName, "") == 0) {
		// Closure
		KLIB KBuffer_printf(kctx, &wb, "(function(");
	}
	else if(mtd->typeId == KType_NameSpace) {
		// Top level functions
		KLIB KBuffer_printf(kctx, &wb, "var %s%s = function(", KMethodName_Fmt2(mtd->mn));
	}
	else if(strcmp(KSymbol_text(mtd->mn), "new") == 0) {
		// Constractor
		KLIB KBuffer_printf(kctx, &wb, "function %s(", KClass_text(kclass));
	}
	else {
		// Methods
		compileAllDefinedMethods(kctx);
		if(kMethod_Is(Static, mtd)) {
			KLIB KBuffer_printf(kctx, &wb, "%s.%s%s = function(", KClass_text(KClass_(mtd->typeId)), KMethodName_Fmt2(mtd->mn));
		}
		else {
			KLIB KBuffer_printf(kctx, &wb, "%s.prototype.%s%s = function(", KClass_text(KClass_(mtd->typeId)), KMethodName_Fmt2(mtd->mn));
		}
	}
	// Emit all paramators
	for(i = 0; i < params->psize; ++i) {
		if(i != 0) {
			KLIB KBuffer_printf(kctx, &wb, ", ");
		}
		KLIB KBuffer_printf(kctx, &wb, "%s", KSymbol_text(params->paramtypeItems[i].name));
	}
	KLIB KBuffer_printf(kctx, &wb, ")");

	JSBuilder_EmitString(kctx, builder, KLIB KBuffer_text(kctx, &wb, EnsureZero), "", "");
	KLIB KBuffer_Free(&wb);
}

static void JSBuilder_EmitClassHeader(KonohaContext *kctx, KBuilder *builder, KClass *kclass)
{
	JSBuilder *jsBuilder = (JSBuilder *)builder;
	KClass *base = KClass_(kclass->superTypeId);
	if(base->typeId != KType_Object) {
		JSBuilder_EmitExtendFunctionCode(kctx, builder);
		JSBuilder_EmitString(kctx, builder, "var ", KClass_text(kclass), " = (function(_super) {");
	}else {
		JSBuilder_EmitString(kctx, builder, "var ", KClass_text(kclass), " = (function() {");
	}
	JSBuilder_EmitNewLineWith(kctx, builder, "");
	jsBuilder->indent++;
	if(base->typeId != KType_Object){
		JSBuilder_EmitString(kctx, builder, "__extends(", KClass_text(kclass), ", _super);");
		JSBuilder_EmitNewLineWith(kctx, builder, "");
	}
}

static void JSBuilder_EmitClassFooter(KonohaContext *kctx, KBuilder *builder, KClass *kclass)
{
	JSBuilder *jsBuilder = (JSBuilder *)builder;
	KClass *base = KClass_(kclass->superTypeId);
	JSBuilder_EmitString(kctx, builder, "return ", KClass_text(kclass), ";");
	JSBuilder_EmitNewLineWith(kctx, builder, "");
	jsBuilder->indent--;
	if(base->typeId != KType_Object) {
		JSBuilder_EmitString(kctx, builder, "})(", KClass_text(base), ");");
		JSBuilder_EmitNewLineWith(kctx, builder, "");
	}else {
		JSBuilder_EmitNewLineWith(kctx, builder, "})();");
	}
}

static void JSBuilder_Init(KonohaContext *kctx, KBuilder *builder, kMethod *mtd)
{
	JSBuilder *jsBuilder = (JSBuilder *)builder;
	kbool_t isConstractor = false;
	jsBuilder->visitingMethod = mtd;
	jsBuilder->isIndentEmitted = false;
	jsBuilder->indent = 0;
	jsBuilder->isExprNode = false;
	KLIB KBuffer_Init(&jsBuilder->buffer, &jsBuilder->jsCodeBuffer);

	KClass *kclass = KClass_(mtd->typeId);
	KClass *base   = KClass_(kclass->superTypeId);

	if(mtd->mn != 0) {
		// Functions
		KLIB kMethod_SetFunc(kctx, mtd, NULL);
		if(strcmp(KSymbol_text(mtd->mn), "new") == 0) {
			isConstractor = true;
		}

		if(isConstractor) {
			JSBuilder_EmitClassHeader(kctx, builder, kclass);
			JSBuilder_EmitMethodHeader(kctx, builder, mtd);
			JSBuilder_EmitNewLineWith(kctx, builder, " {");
			jsBuilder->indent++;
			if(base->typeId != KType_Object) {
				JSBuilder_EmitNewLineWith(kctx, builder, "_super.call(this);");
			}
			JSBuilder_VisitClassFields(kctx, builder, kclass);
		}
		else {
			JSBuilder_EmitMethodHeader(kctx, builder, mtd);
			JSBuilder_EmitNewLineWith(kctx, builder, " {");
			jsBuilder->indent++;
		}
	}
	else {
		// TopLevel
		compileAllDefinedMethods(kctx);
	}
}

static void JSBuilder_Free(KonohaContext *kctx, KBuilder *builder, kMethod *mtd)
{
	JSBuilder *jsBuilder = (JSBuilder *)builder;
	if(mtd->mn != 0) {
		if(strcmp(KSymbol_text(mtd->mn), "new") == 0) {
			JSBuilder_EmitNewLineWith(kctx, builder, "");
		}
		jsBuilder->indent--;
		JSBuilder_EmitNewLineWith(kctx, builder, "}");
		if(strcmp(KSymbol_text(mtd->mn), "new") == 0) {
			KClass *kclass = KClass_(mtd->typeId);
			JSBuilder_EmitClassFooter(kctx, builder, kclass);
		}
	}
#if 0
	const char *jsSrc = KLIB KBuffer_text(kctx, &jsBuilder->jsCodeBuffer, EnsureZero);
#ifdef HAVE_LIBV8
	kbool_t isCompileOnly = KonohaContext_Is(CompileOnly, kctx);
#else
#define isCompileOnly (1)
#endif
	if(isCompileOnly) {
		PLATAPI printf_i("%s\n", jsSrc);
	}
#ifdef HAVE_LIBV8
	else {
		v8::Handle<v8::String> v8Src = v8::String::New(jsSrc);
		v8::Handle<v8::Script> v8Script = v8::Script::Compile(v8Src);
		v8::Handle<v8::Value> result = v8Script->Run();
		v8::String::AsciiValue resultStr(result);
	}
#endif
#endif
	KLIB KBuffer_Free(&jsBuilder->jsCodeBuffer);
}

static struct KVirtualCode *V8_GenerateVirtualCode(KonohaContext *kctx, kMethod *mtd, kNode *block, int option)
{
	INIT_GCSTACK();
	JSBuilder builderbuf = {}, *builder = &builderbuf;
	kNameSpace *ns = kNode_ns(block);
	builder->common.api = ns->builderApi;
	builder->visitingMethod = mtd;

	//PLATAPI printf_i("//=>=>=>=>=>=>=>=>\n");
	JSBuilder_Init(kctx, (KBuilder *)builder, mtd);
	SUGAR VisitNode(kctx, (KBuilder *)builder, block, NULL);
	JSBuilder_Free(kctx, (KBuilder *)builder, mtd);
	RESET_GCSTACK();

	//PLATAPI printf_i("//<=<=<=<=<=<=<=<=\n");
	return NULL;
}

static KMETHOD KMethodFunc_RunVirtualMachine(KonohaContext *kctx, KonohaStack *sfp)
{
}

static struct KVirtualCode *V8_RunVirtualMachine(KonohaContext *kctx, struct KonohaValueVar *sfp, struct KVirtualCode *pc)
{
	/*FIXME NotSupportedAPI();*/
	return NULL;
}


static void V8_SetMethodCode(KonohaContext *kctx, kMethodVar *mtd, KVirtualCode *vcode, KMethodFunc func)
{
	KLIB kMethod_SetFunc(kctx, mtd, func);
	mtd->vcode_start = vcode;
}

static KMethodFunc V8_GenerateMethodFunc(KonohaContext *kctx, KVirtualCode *vcode)
{
	return KMethodFunc_RunVirtualMachine;
}

static struct KVirtualCode *GetDefaultBootCode(void)
{
	return NULL;
}

static void V8_DeleteVirtualMachine(KonohaContext *kctx)
{
}

static const struct KBuilderAPI *GetDefaultBuilderAPI(void);

static const KModuleInfo ModuleInfo = {
	"JavaScript", K_VERSION, 0, "JavaScript",
};

static const struct ExecutionEngineModule V8_Module = {
	&ModuleInfo,
	V8_DeleteVirtualMachine,
	GetDefaultBuilderAPI,
	GetDefaultBootCode,
	V8_GenerateVirtualCode,
	V8_GenerateMethodFunc,
	V8_SetMethodCode,
	V8_RunVirtualMachine
};

static const struct KBuilderAPI V8_BuilderAPI = {
	"JavaScript",
	&V8_Module,
#define DEFINE_BUILDER_API(NAME) JSBuilder_Visit##NAME##Node,
	KNodeList(DEFINE_BUILDER_API)
#undef DEFINE_BUILDER_API
};

static const struct KBuilderAPI *GetDefaultBuilderAPI(void)
{
	return &V8_BuilderAPI;
}

// -------------------------------------------------------------------------

kbool_t LoadJavaScriptModule(KonohaFactory *factory, ModuleType type)
{
#ifdef HAVE_LIBV8
	globalJSContext = new JSContext();
#endif

	memcpy(&factory->ExecutionEngineModule, &V8_Module, sizeof(V8_Module));
	return true;
}

} /* extern "C" */
