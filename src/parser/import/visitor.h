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
	switch(kNode_node(node)) {
		KNodeList(DefineVisitCase)
	}
	return ret;
}

static kbool_t kMethod_GenCode(KonohaContext *kctx, kMethod *mtd, kNode *block, int option)
{
	DBG_P("START CODE GENERATION..");
	kNameSpace *ns = kNode_ns(block);
	struct KVirtualCode *vcode = ns->builderApi->ExecutionEngineModule->GenerateVirtualCode(kctx, mtd, block, option);
	KMethodFunc func = ns->builderApi->ExecutionEngineModule->GenerateMethodFunc(kctx, vcode);
	ns->builderApi->ExecutionEngineModule->SetMethodCode(kctx, (kMethodVar *)mtd, vcode, func);
	return true;
}

static KMETHOD KMethodFunc_invokeAbstractMethod(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(0);
}

static void kMethod_SetFunc(KonohaContext *kctx, kMethod *mtd, KMethodFunc func)
{
	func = (func != NULL) ? func : KMethodFunc_invokeAbstractMethod;
	((kMethodVar *)mtd)->invokeKMethodFunc = func;
	((kMethodVar *)mtd)->vcode_start = PLATAPI ExecutionEngineModule.GetDefaultBootCode();
}
