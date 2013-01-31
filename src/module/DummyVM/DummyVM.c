/****************************************************************************
 * Copyright (c) 2013, Masahiro Ide <ide@konohascript.org> All rights reserved.
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

#include "../../../include/minikonoha/minikonoha.h"
#include "../../../include/minikonoha/konoha_common.h"
#include "../../../include/minikonoha/sugar.h"

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/
/* Konoha AST API */
static kNode* kNode_getFirstBlock(KonohaContext *kctx, kNode *stmt)
{
	return SUGAR kNode_GetNode(kctx, stmt, KSymbol_BlockPattern, K_NULLBLOCK);
}

static kNode* kNode_getElseBlock(KonohaContext *kctx, kNode *stmt)
{
	return SUGAR kNode_GetNode(kctx, stmt, KSymbol_else, K_NULLBLOCK);
}

static kNode* kNode_getFirstNode(KonohaContext *kctx, kNode *stmt)
{
	return SUGAR kNode_GetNode(kctx, stmt, KSymbol_ExprPattern, NULL);
}

static kMethod* CallNode_getMethod(kNode *expr)
{
	return expr->NodeList->MethodItems[0];
}

static kNode *kNode_GetNode(KonohaContext *kctx, kNode *stmt, ksymbol_t kw)
{
	return SUGAR kNode_GetNode(kctx, stmt, kw, NULL);
}

/*----------------------------------------------------------------------------*/
/* Visitor */

struct KBuilder { /* DummyVM Builder */
	struct KBuilderCommon common;
};

#define NotSupportedAPI() assert(0 && "NotSupportedAPI")
#define TODO()            assert(0 && "TODO")

/*----------------------------------------------------------------------------*/
/* Visitor API */
static kbool_t DummyVM_VisitDoneNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	return true;
}

static kbool_t DummyVM_VisitBoxNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	return true;
}

static kbool_t DummyVM_VisitPushNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	return true;
}

static kbool_t DummyVM_VisitErrorNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	return false;
}

static kbool_t DummyVM_VisitThrowNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	return true;
}

static kbool_t DummyVM_VisitBlockNode(KonohaContext *kctx, KBuilder *builder, kNode *block, void *thunk)
{
	size_t i;
	for(i = 0; i < kNode_GetNodeListSize(kctx, block); i++) {
		kNode *stmt = block->NodeList->NodeItems[i];
		if(!SUGAR VisitNode(kctx, builder, stmt, thunk))
			break;
	}
	return true;
}

static kbool_t DummyVM_VisitReturnNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	kNode *expr = SUGAR kNode_GetNode(kctx, stmt, KSymbol_ExprPattern, NULL);
	if(expr != NULL && IS_Node(expr) && expr->attrTypeId != KType_void) {
		SUGAR VisitNode(kctx, builder, expr, thunk);
	}
	return true;
}

static kbool_t DummyVM_VisitIfNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	kNode *expr    = kNode_getFirstNode(kctx, stmt);
	SUGAR VisitNode(kctx, builder, expr, thunk);
	SUGAR VisitNode(kctx, builder, kNode_getFirstBlock(kctx, stmt), thunk);
	SUGAR VisitNode(kctx, builder, kNode_getElseBlock(kctx, stmt), thunk);
	return true;
}

static kbool_t DummyVM_VisitLoopNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	kNode *itrBlock = SUGAR kNode_GetNode(kctx, stmt, KSymbol_("Iterator"), NULL);
	SUGAR VisitNode(kctx, builder, kNode_getFirstNode(kctx, stmt), thunk);
	SUGAR VisitNode(kctx, builder, kNode_getFirstBlock(kctx, stmt), thunk);
	if(itrBlock != NULL) {
		SUGAR VisitNode(kctx, builder, itrBlock, thunk);
	}
	return true;
}

static kbool_t DummyVM_VisitWhileNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	return DummyVM_VisitLoopNode(kctx, builder, stmt, thunk);
}

static kbool_t DummyVM_VisitDoWhileNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	return DummyVM_VisitLoopNode(kctx, builder, stmt, thunk);
}

static kbool_t DummyVM_VisitForNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	kNode *initNode = kNode_GetNode(kctx, stmt, KSymbol_("init"));
	if(initNode != NULL) {
		SUGAR VisitNode(kctx, builder, initNode, thunk);
	}
	return DummyVM_VisitLoopNode(kctx, builder, stmt, thunk);
}

static kbool_t DummyVM_VisitContinueNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	return false;
}

static kbool_t DummyVM_VisitBreakNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	return false;
}

static kbool_t DummyVM_VisitTryNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	//FIXME
	TODO();
	return true;
}

static kbool_t DummyVM_VisitConstNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	return true;
}

static kbool_t DummyVM_VisitUnboxConstNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	return true;
}

static kbool_t DummyVM_VisitNewNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	return true;
}

static kbool_t DummyVM_VisitNullNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	return true;
}

static kbool_t DummyVM_VisitLocalNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	return true;
}

static kbool_t DummyVM_VisitFieldNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	return true;
}

kbool_t LoadDummyVMModule(KonohaFactory *factory, ModuleType type);

static void ImportPackage(KonohaContext *kctx, kNameSpace *ns, kString *packageName, kfileline_t uline)
{
	KBaseTrace(trace);
	SUGAR kNameSpace_UseDefaultVirtualMachine(kctx, ns);
	//static KBuilderAPI *executableVM = NULL;
	//if(executableVM == NULL) {
	//	executableVM = ns->builderApi;
	//}
	//((kNameSpaceVar *)ns)->builderApi = executableApi;
	KImportPackage(ns, kString_text(packageName), trace);
	LoadDummyVMModule((KonohaFactory *) kctx->platApi, ReleaseModule);
	ns->builderApi = PLATAPI GetDefaultBuilderAPI();
}

static int CallExpr_getArgCount(kNode *expr)
{
	return kArray_size(expr->NodeList) - 2;
}

static kbool_t DummyVM_VisitMethodCallNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	kMethod *mtd = CallNode_getMethod(expr);
	if(mtd->typeId == KType_NameSpace && mtd->mn == KMethodName_("import")) {
		if(CallExpr_getArgCount(expr) == 1) { /* param0.mtd(param1); */
			kNode *param0 = kNode_At(expr, 1);
			kNode *param1 = kNode_At(expr, 2);
			assert(kNode_node(param0) == KNode_Const && IS_NameSpace(param0->ObjectConstValue));
			assert(kNode_node(param1) == KNode_Const && IS_String(param1->ObjectConstValue));
			kNameSpace *currentNS = (kNameSpace *) param0->ObjectConstValue;
			kString *packageName  = (kString *)    param1->ObjectConstValue;
			ImportPackage(kctx, currentNS, packageName, kNode_uline(expr));
		}
	}
	return true;
}

static kbool_t DummyVM_VisitAndNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	kNode *LHS = kNode_At(expr, 1);
	kNode *RHS = kNode_At(expr, 2);
	SUGAR VisitNode(kctx, builder, LHS, thunk);
	SUGAR VisitNode(kctx, builder, RHS, thunk);
	return true;
}

static kbool_t DummyVM_VisitOrNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	kNode *LHS = kNode_At(expr, 1);
	kNode *RHS = kNode_At(expr, 2);
	SUGAR VisitNode(kctx, builder, LHS, thunk);
	SUGAR VisitNode(kctx, builder, RHS, thunk);
	return true;
}

static kbool_t DummyVM_VisitAssignNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	kNode *right = kNode_At(expr, 2);
	SUGAR VisitNode(kctx, builder, right, thunk);
	return true;
}

static kbool_t DummyVM_VisitFunctionNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	return true;
}

/* end of Visitor */
/*----------------------------------------------------------------------------*/

static struct KVirtualCode *DummyVM_Run(KonohaContext *kctx, struct KonohaValueVar *sfp, struct KVirtualCode *pc)
{
	NotSupportedAPI();
	return NULL;
}

static KMETHOD DummyVM_RunVirtualMachine(KonohaContext *kctx, KonohaStack *sfp)
{
}

static KMethodFunc DummyVM_GenerateMethodFunc(KonohaContext *kctx, struct KVirtualCode *vcode)
{
	return 0;
}

static struct KVirtualCode *GetDefaultBootCode(void)
{
	return NULL;
}

static struct KVirtualCode *DummyVM_GenerateVirtualCode(KonohaContext *kctx, kMethod *mtd, kNode *block, int option)
{
	kNameSpace *ns = kNode_ns(block);
	KBuilder builderbuf = {}, *builder = &builderbuf;
	builder->common.api = ns->builderApi;
	SUGAR VisitNode(kctx, builder, block, NULL);
	return NULL;
}

static void DummyVM_SetMethodCode(KonohaContext *kctx, kMethodVar *mtd, struct KVirtualCode *vcode, KMethodFunc func)
{
	KLIB kMethod_SetFunc(kctx, mtd, DummyVM_RunVirtualMachine);
	mtd->vcode_start = vcode;
}

static void InitStaticBuilderApi(struct KBuilderAPI *builderApi)
{
	builderApi->target = "DummyVM";
#define DEFINE_BUILDER_API(NAME) builderApi->visit##NAME##Node = DummyVM_Visit##NAME##Node;
	KNodeList(DEFINE_BUILDER_API);
#undef DEFINE_BUILDER_API
	builderApi->GenerateVirtualCode = DummyVM_GenerateVirtualCode;
	builderApi->GenerateMethodFunc  = DummyVM_GenerateMethodFunc;
	builderApi->SetMethodCode       = DummyVM_SetMethodCode;
	builderApi->RunVirtualMachine   = DummyVM_Run;
}

static struct KBuilderAPI *GetDefaultBuilderAPI(void)
{
	static struct KBuilderAPI builderApi = {};
	if(builderApi.target == NULL) {
		InitStaticBuilderApi(&builderApi);
	}
	return &builderApi;
}

static void DummyVMDeleteVirtualMachine(KonohaContext *kctx)
{
}

// -------------------------------------------------------------------------

kbool_t LoadDummyVMModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"DummyVM", K_VERSION, 0, "DummyVM",
	};
	factory->VirtualMachineInfo   = &ModuleInfo;
	factory->GetDefaultBootCode   = GetDefaultBootCode;
	factory->GetDefaultBuilderAPI = GetDefaultBuilderAPI;
	factory->DeleteVirtualMachine = DummyVMDeleteVirtualMachine;
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
