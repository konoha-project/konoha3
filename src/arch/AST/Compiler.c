/****************************************************************************
 * Copyright (c) 2012-2013, Masahiro Ide <ide@konohascript.org> All rights reserved.
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

#include "konoha3.h"
#include "konoha3/import/module.h"

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/
/* Visitor */

struct KBuilder { /* AST Builder */
	struct KBuilderCommon common;
};

#define NotSupportedAPI() assert(0 && "NotSupportedAPI")
static int indent = 0;

void scope_begin() { indent ++; }
void scope_end()   { indent --; }
void tab()
{
	int i = 0;
	for (i = 0; i < indent; i++) {
		printf("  ");
	}
}

kbool_t end()
{
	//printf("\n");
	return true;
}

/*----------------------------------------------------------------------------*/
/* Visitor API */
static kbool_t AST_VisitDoneNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	tab();
	printf("new DoneNode(%sTy)", KType_text(stmt->typeAttr));
	return end();
}

static kbool_t AST_VisitBoxNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	printf("new BoxNode(%sTy, ", KType_text(expr->typeAttr));
	KLIB VisitNode(kctx, builder, expr->NodeToPush, thunk);
	printf(")");
	return end();
}

static kbool_t AST_VisitPushNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	assert(0 && "Not Implemented");
	return end();
}

static kbool_t AST_VisitErrorNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	kString *Err  = kNode_getErrorMessage(kctx, stmt);
	printf("new ErrorNode(StringTy, \"%s\")", kString_text(Err));
	return end();
}

static kbool_t AST_VisitThrowNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	printf("new ThrowNode()");
	return end();
}

static kbool_t AST_VisitBlockNode(KonohaContext *kctx, KBuilder *builder, kNode *block, void *thunk)
{
	printf("new BlockNode(voidTy, ");
	unsigned i;
	for(i = 0; i < kNode_GetNodeListSize(kctx, block); i++) {
		kNode *stmt = block->NodeList->NodeItems[i];
		if (i != 0) {
			printf(", ");
		}
		if(!KLIB VisitNode(kctx, builder, stmt, thunk))
			break;
	}
	printf(")");
	return end();
}

static kbool_t AST_VisitReturnNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	kNode *expr = KLIB kNode_GetNode(kctx, stmt, KSymbol_ExprPattern, NULL);
	printf("new ReturnNode(%sTy", KType_text(stmt->typeAttr));
	if(expr != NULL && IS_Node(expr) && expr->typeAttr != KType_void) {
		printf(", ");
		KLIB VisitNode(kctx, builder, expr, thunk);
	}
	printf(")");
	return end();
}

static kbool_t AST_VisitIfNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	kNode *expr    = kNode_getFirstNode(kctx, stmt);
	printf("new IfNode(%sTy,", KType_text(stmt->typeAttr));
	/* if */
	KLIB VisitNode(kctx, builder, expr, thunk);
	printf(", ");
	KLIB VisitNode(kctx, builder, kNode_getFirstBlock(kctx, stmt), thunk);
	printf(", ");
	KLIB VisitNode(kctx, builder, kNode_getElseBlock(kctx, stmt), thunk);
	printf(")");
	return end();
}

enum LoopType {
	WhileLoop,
	DoWhileLoop,
	ForLoop
};

static kbool_t AST_VisitLoopNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk, enum LoopType Loop)
{
	kNode *itrBlock = KLIB kNode_GetNode(kctx, stmt, KSymbol_("Iterator"), NULL);

	if(itrBlock != NULL) {
		assert(Loop == ForLoop);
	}
	else if(Loop == WhileLoop) {
	} else {
		assert(0 && "FIXME");
	}

	printf("new LoopNode(%s, ", KType_text(stmt->typeAttr));
	KLIB VisitNode(kctx, builder, kNode_getFirstNode(kctx, stmt), thunk);
	printf(", ");
	KLIB VisitNode(kctx, builder, kNode_getFirstBlock(kctx, stmt), thunk);
	/* Itr */
	if(itrBlock != NULL) {
		printf(", ");
		KLIB VisitNode(kctx, builder, itrBlock, thunk);
	}
	printf(")");
	return end();
}

static kbool_t AST_VisitWhileNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	return AST_VisitLoopNode(kctx, builder, stmt, thunk, WhileLoop);
}

static kbool_t AST_VisitDoWhileNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	return AST_VisitLoopNode(kctx, builder, stmt, thunk, DoWhileLoop);
}

static kbool_t AST_VisitForNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	kNode *initNode = kNode_GetNode(kctx, stmt, KSymbol_("init"));
	if(initNode != NULL) {
		printf("new BlockNode(VoidTy, ");
		KLIB VisitNode(kctx, builder, initNode, thunk);
		printf(", ");
	}
	AST_VisitLoopNode(kctx, builder, stmt, thunk, ForLoop);
	if(initNode != NULL) {
		printf(")");
	}
	return end();
}

static kbool_t AST_VisitJumpNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk, ksymbol_t label)
{
	printf("new JumpNode(VoidTy, %s)", KSymbol_text(label));
	return end();
}

static kbool_t AST_VisitContinueNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	ksymbol_t label = KSymbol_("continue");
	return AST_VisitJumpNode(kctx, builder, stmt, thunk, label);
}

static kbool_t AST_VisitBreakNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	ksymbol_t label = KSymbol_("break");
	return AST_VisitJumpNode(kctx, builder, stmt, thunk, label);
}

static kbool_t AST_VisitTryNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	printf("new TryNode(VoidTy)");
	return end();
}

static kbool_t AST_VisitConstNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	kObject *v = expr->ObjectConstValue;
	DBG_ASSERT(!KType_Is(UnboxType, expr->typeAttr));
	printf("new ConstNode(%sTy, %p)", KType_text(expr->typeAttr), v);
	return end();
}

static kbool_t AST_VisitUnboxConstNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	printf("new ConstNode(%sTy, %lu)", KType_text(expr->typeAttr), expr->unboxConstValue);
	return end();
}

static kbool_t AST_VisitNewNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	printf("new NewNode(%sTy)", KType_text(expr->typeAttr));
	return end();
}

static kbool_t AST_VisitNullNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	printf("new NullNode(%sTy)", KType_text(expr->typeAttr));
	return end();
}

static kbool_t AST_VisitLocalNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	printf("new LocalNode(%sTy, %lu, \"%s\")", KType_text(expr->typeAttr), expr->index, KSymbol_text(expr->TermToken->symbol));
	return end();
}

static kbool_t AST_VisitFieldNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	khalfword_t index  = (khalfword_t)expr->index;
	khalfword_t xindex = (khalfword_t)(expr->index >> (sizeof(khalfword_t)*8));
	printf("new FieldNode(%sTy, %u, %u, %s)", KType_text(expr->typeAttr), index, xindex, KSymbol_text(expr->TermToken->symbol));
	return end();
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


kbool_t LoadASTModule(KonohaFactory *factory, ModuleType type);

static bool AST_importPackage(KonohaContext *kctx, kNameSpace *ns, kString *package, kfileline_t uline)
{
	KBaseTrace(trace);
	KonohaFactory *factory = (KonohaFactory *)kctx->platApi;

	KLIB kNameSpace_UseDefaultVirtualMachine(kctx, ns);

	KImportPackage(ns, kString_text(package), trace);
	compileAllDefinedMethodsInNameSpace(kctx, ns);

	LoadASTModule(factory, ReleaseModule);
	ns->builderApi = factory->ExecutionEngineModule.GetDefaultBuilderAPI();
	return true;
}

static bool AST_loadScript(KonohaContext *kctx, kNameSpace *ns, kString *package, kfileline_t uline)
{
	KBaseTrace(trace);
	char pathbuf[512];
	const char *path = PLATAPI formatTransparentPath(pathbuf, sizeof(pathbuf), KFileLine_textFileName(trace->pline), kString_text(package));

	KLIB kNameSpace_LoadScript(kctx, ns, path, trace);

	KonohaFactory *factory = (KonohaFactory *)kctx->platApi;
	ns->builderApi = factory->ExecutionEngineModule.GetDefaultBuilderAPI();
	return true;
}

static kbool_t AST_VisitMethodCallNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	kMethod *mtd = CallNode_getMethod(expr);

	kNode *node = expr;
	kNode *receiver = kNode_At(node, 1);
	if(receiver->typeAttr == KType_NameSpace) {
		if(mtd->mn == KMethodName_("import")) {
			kString *packageNameString = (kString *)kNode_At(node, 2)->ObjectConstValue;
			kNameSpace *ns = (kNameSpace *)receiver->ObjectConstValue;
			AST_importPackage(kctx, ns, packageNameString, node->TermToken->uline);
		}
		else if(mtd->mn == KMethodName_("load") || mtd->mn == KMethodName_("include")) {
			kString *packageNameString = (kString *)kNode_At(node, 2)->ObjectConstValue;
			kNameSpace *ns = (kNameSpace *)receiver->ObjectConstValue;
			AST_loadScript(kctx, ns, packageNameString, node->TermToken->uline);
		}
	}

	printf("new MethodCallNode(%sTy, \"%s%s\", ", KType_text(expr->typeAttr), KMethodName_Fmt2(mtd->mn));
	int i, start = 1;
	int argc = CallNode_getArgCount(expr);

	if(kMethod_Is(Static, mtd)) {
		printf("new NullNode(%sTy), ", KType_text(mtd->typeId));
		start += 1;
	}
	for (i = start; i < argc + 2; i++) {
		kNode *exprN = kNode_At(expr, i);
		if (i != start) {
			printf(", ");
		}
		KLIB VisitNode(kctx, builder, exprN, thunk);
	}
	printf(")");
	return end();
}

static kbool_t AST_VisitAndNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	printf("new AndNode(%s", KType_text(expr->typeAttr));
	KLIB VisitNode(kctx, builder, kNode_At(expr, 1), thunk);
	printf(", ");
	KLIB VisitNode(kctx, builder, kNode_At(expr, 2), thunk);
	printf(")");
	return end();
}

static kbool_t AST_VisitOrNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	printf("new OrNode(%s", KType_text(expr->typeAttr));
	KLIB VisitNode(kctx, builder, kNode_At(expr, 1), thunk);
	printf(", ");
	KLIB VisitNode(kctx, builder, kNode_At(expr, 2), thunk);
	printf(")");

	return end();
}

static kbool_t AST_VisitAssignNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	kNode *left = kNode_At(expr, 1);
	kNode *right = kNode_At(expr, 2);
	const char *type = KType_text(expr->typeAttr);
	if(kNode_node(left) == KNode_Local) {
		printf("new AssignNode(%sTy, new LocalNode(%sTy, %lu, \"%s\"), ", type, type, left->index, KSymbol_text(left->TermToken->symbol));
		KLIB VisitNode(kctx, builder, right, thunk);
	}
	else{
		assert(kNode_node(left) == KNode_Field);
		khalfword_t index  = (khalfword_t)left->index;
		khalfword_t xindex = (khalfword_t)(left->index >> (sizeof(khalfword_t)*8));
		printf("new AssignNode(%sTy, new FieldNode(%sTy, %u, %u, \"%s\"), ", type, type, index, xindex, KSymbol_text(left->TermToken->symbol));
		KLIB VisitNode(kctx, builder, right, thunk);
	}
	printf(")");
	return end();
}

static kbool_t AST_VisitFunctionNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	/*
	 * [FunctionExpr] := new Function(method, env1, env2, ...)
	 * expr->NodeList = [method, defObj, env1, env2, ...]
	 **/
	kMethod *mtd = CallNode_getMethod(expr);
	printf("new FunctionNode(%sTy, \"%s%s\", ", KType_text(expr->typeAttr), KMethodName_Fmt2(mtd->mn));

	size_t i, ParamSize = kArray_size(expr->NodeList)-2;
	for(i = 0; i < ParamSize; i++) {
		kNode *envN = kNode_At(expr, i+2);
		if (i != 0) {
			printf(", ");
		}
		KLIB VisitNode(kctx, builder, envN, thunk);
	}
	printf(")");

	return end();
}

/* end of Visitor */
/*----------------------------------------------------------------------------*/

static struct KVirtualCode *AST_Run(KonohaContext *kctx, struct KonohaValueVar *sfp, struct KVirtualCode *pc)
{
	NotSupportedAPI();
	return NULL;
}

static KMETHOD AST_RunVirtualMachine(KonohaContext *kctx, KonohaStack *sfp)
{
}

static KMethodFunc AST_GenerateMethodFunc(KonohaContext *kctx, struct KVirtualCode *vcode)
{
	return 0;
}

static struct KVirtualCode *GetDefaultBootCode(void)
{
	return NULL;
}

static struct KVirtualCode *AST_GenerateVirtualCode(KonohaContext *kctx, kMethod *mtd, kNode *block, int option)
{
	kNameSpace *ns = kNode_ns(block);
	KBuilder builderbuf = {}, *builder = &builderbuf;
	INIT_GCSTACK();
	builder->common.api = ns->builderApi;
	KLIB VisitNode(kctx, builder, block, NULL);
	printf(";\n");
	RESET_GCSTACK();
	return (struct KVirtualCode *) 0;
}

static void AST_SetMethodCode(KonohaContext *kctx, kMethodVar *mtd, struct KVirtualCode *vcode, KMethodFunc func)
{
	KLIB kMethod_SetFunc(kctx, mtd, AST_RunVirtualMachine);
	mtd->vcode_start = vcode;
}

static void AST_DeleteVirtualMachine(KonohaContext *kctx)
{
}

static const KModuleInfo ModuleInfo = {
	"AST", K_VERSION, 0, "AST",
};

static const struct KBuilderAPI *GetDefaultBuilderAPI(void);

static const struct ExecutionEngineModule AST_Module = {
	&ModuleInfo,
	"C",
	AST_DeleteVirtualMachine,
	GetDefaultBuilderAPI,
	GetDefaultBootCode,
	AST_GenerateVirtualCode,
	AST_GenerateMethodFunc,
	AST_SetMethodCode,
	AST_Run
};

static const struct KBuilderAPI AST_BuilderAPI = {
	"AST",
	&AST_Module,
#define DEFINE_BUILDER_API(NAME) AST_Visit##NAME##Node,
	KNodeList(DEFINE_BUILDER_API)
#undef DEFINE_BUILDER_API
};

static const struct KBuilderAPI *GetDefaultBuilderAPI(void)
{
	return &AST_BuilderAPI;
}

// -------------------------------------------------------------------------

kbool_t LoadASTModule(KonohaFactory *factory, ModuleType type)
{
	memcpy(&factory->ExecutionEngineModule, &AST_Module, sizeof(AST_Module));
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
