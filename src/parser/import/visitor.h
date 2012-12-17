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

static kbool_t VisitNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt)
{
	kbool_t ret = false;
	struct KBuilderCommon *cbuilder = (struct KBuilderCommon *)builder;
	switch(stmt->node) {
		case TSTMT_ERR:   ret = cbuilder->api->visitErrNode(kctx, builder, stmt); break;
		case TSTMT_EXPR:  ret = cbuilder->api->visitNodeNode(kctx, builder, stmt);   break;
		case TSTMT_BLOCK: ret = cbuilder->api->visitNodeNode(kctx, builder, stmt);  break;
		case TSTMT_RETURN: ret = cbuilder->api->visitReturnNode(kctx, builder, stmt); break;
		case TSTMT_IF:    ret = cbuilder->api->visitIfNode(kctx, builder, stmt);     break;
		case TSTMT_LOOP:  ret = cbuilder->api->visitLoopNode(kctx, builder, stmt);   break;
		case TSTMT_JUMP:  ret = cbuilder->api->visitJumpNode(kctx, builder, stmt);   break;
		case TSTMT_TRY:   ret = cbuilder->api->visitTryNode(kctx, builder, stmt);    break;
		default: cbuilder->api->visitUndefinedNode(kctx, builder, stmt);        break;
	}
	return ret;
}

static void VisitNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, kNode *expr)
{
	struct KBuilderCommon *cbuilder = (struct KBuilderCommon *)builder;
	int a = cbuilder->a;
	int espidx = cbuilder->espidx;
	int shift = cbuilder->shift;
	switch(expr->node) {
		case TEXPR_CONST:    cbuilder->api->visitConstNode(kctx, builder, stmt, expr);  break;
		case TEXPR_NEW:      cbuilder->api->visitNewNode(kctx, builder, stmt, expr);    break;
		case TEXPR_NULL:     cbuilder->api->visitNullNode(kctx, builder, stmt, expr);   break;
		case TEXPR_NCONST:   cbuilder->api->visitNConstNode(kctx, builder, stmt, expr); break;
		case TEXPR_LOCAL:    cbuilder->api->visitLocalNode(kctx, builder, stmt, expr);  break;
		case TEXPR_BLOCK:    cbuilder->api->visitNodeNode(kctx, builder, stmt, expr);  break;
		case TEXPR_FIELD:    cbuilder->api->visitFieldNode(kctx, builder, stmt, expr);  break;
		case TEXPR_CALL:     cbuilder->api->visitCallNode(kctx, builder, stmt, expr);   break;
		case TEXPR_AND:      cbuilder->api->visitAndNode(kctx, builder, stmt, expr);    break;
		case TEXPR_OR:       cbuilder->api->visitOrNode(kctx, builder, stmt, expr);     break;
		case TEXPR_LET:      cbuilder->api->visitLetNode(kctx, builder, stmt, expr);    break;
		case TEXPR_STACKTOP: cbuilder->api->visitStackTopNode(kctx, builder, stmt, expr);break;
		default: DBG_ABORT("unknown expr=%d", expr->node);
	}
	cbuilder->a = a;
	cbuilder->espidx = espidx;
	cbuilder->shift = shift;
}

static kbool_t VisitNode(KonohaContext *kctx, KBuilder *builder, kNode *block)
{
	struct KBuilderCommon *cbuilder = (struct KBuilderCommon *)builder;
	int a = cbuilder->a;
	int espidx = cbuilder->espidx;
	int shift = cbuilder->shift;
	cbuilder->espidx = (block->esp->node == TEXPR_STACKTOP) ? shift + block->esp->index : block->esp->index;
	kbool_t ret = true;
	size_t i;
	for (i = 0; i < kArray_size(block->NodeList); i++) {
		kNode *stmt = block->NodeList->NodeItems[i];
		if(stmt->syn == NULL) continue;
		cbuilder->uline = stmt->uline;
		if(!VisitNode(kctx, builder, stmt)) {
			ret = false;
			break;
		}
	}
	cbuilder->a = a;
	cbuilder->espidx = espidx;
	cbuilder->shift = shift;
	return ret;
}

static void kMethod_GenCode(KonohaContext *kctx, kMethod *mtd, kNode *block, int option)
{
	DBG_P("START CODE GENERATION..");
	kNameSpace *ns = block->NodeNameSpace;
	struct KVirtualCode *vcode = ns->builderApi->GenerateKVirtualCode(kctx, mtd, block, option);
	KMethodFunc func = ns->builderApi->GenerateKMethodFunc(kctx, vcode);
	((kMethodVar *)mtd)->invokeKMethodFunc = func;
	((kMethodVar *)mtd)->vcode_start = vcode;
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

