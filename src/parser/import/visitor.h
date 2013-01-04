/****************************************************************************
 * Copyright (c) 2012, the Konoha project authors. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *	this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
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

#define DefineVisitCase(NAME)  case KNode_##NAME:   ret = cbuilder->api->visit##NAME##Node(kctx, builder, node, thunk); break;

static kbool_t VisitNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	kbool_t ret = false;
	struct KBuilderCommon *cbuilder = (struct KBuilderCommon *)builder;
	switch(node->node) {
		KNodeList(DefineVisitCase)
	}
	return ret;
}

//static kbool_t VisitNode(KonohaContext *kctx, KBuilder *builder, kNode *block)
//{
//	struct KBuilderCommon *cbuilder = (struct KBuilderCommon *)builder;
//	int a = cbuilder->a;
//	int espidx = cbuilder->espidx;
//	int shift = cbuilder->shift;
//	cbuilder->espidx = (block->esp->node == KNode_STACKTOP) ? shift + block->esp->index : block->esp->index;
//	kbool_t ret = true;
//	size_t i;
//	for (i = 0; i < kArray_size(block->NodeList); i++) {
//		kNode *stmt = block->NodeList->NodeItems[i];
//		if(stmt->syn == NULL) continue;
//		cbuilder->uline = kNode_uline(stmt);
//		if(!VisitNode(kctx, builder, stmt)) {
//			ret = false;
//			break;
//		}
//	}
//	cbuilder->a = a;
//	cbuilder->espidx = espidx;
//	cbuilder->shift = shift;
//	return ret;
//}

static void kMethod_GenCode(KonohaContext *kctx, kMethod *mtd, kNode *block, int option)
{
	DBG_P("START CODE GENERATION..");
	kNameSpace *ns = kNode_ns(block);
	struct KVirtualCode *vcode = ns->builderApi->GenerateKVirtualCode(kctx, mtd, block, option);
	KMethodFunc func = ns->builderApi->GenerateKMethodFunc(kctx, vcode);
	ns->builderApi->SetMethodCode(kctx, (kMethodVar *) mtd, vcode, func);
}

static KMETHOD KMethodFunc_invokeAbstractMethod(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(0);
}

static void kMethod_SetFunc(KonohaContext *kctx, kMethod *mtd, KMethodFunc func)
{
	func = (func != NULL) ? func : KMethodFunc_invokeAbstractMethod;
	((kMethodVar *)mtd)->invokeKMethodFunc = func;
	((kMethodVar *)mtd)->vcode_start = PLATAPI GetDefaultBootCode();
}

