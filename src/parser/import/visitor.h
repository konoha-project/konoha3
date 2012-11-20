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

static kbool_t VisitStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	kbool_t ret = false;
	struct KBuilderCommon *cbuilder = (struct KBuilderCommon*)builder;
	switch(stmt->build) {
		case TSTMT_ERR:   ret = cbuilder->api->visitErrStmt(kctx, builder, stmt); break;
		case TSTMT_EXPR:  ret = cbuilder->api->visitExprStmt(kctx, builder, stmt);   break;
		case TSTMT_BLOCK: ret = cbuilder->api->visitBlockStmt(kctx, builder, stmt);  break;
		case TSTMT_RETURN: ret = cbuilder->api->visitReturnStmt(kctx, builder, stmt); break;
		case TSTMT_IF:    ret = cbuilder->api->visitIfStmt(kctx, builder, stmt);     break;
		case TSTMT_LOOP:  ret = cbuilder->api->visitLoopStmt(kctx, builder, stmt);   break;
		case TSTMT_JUMP:  ret = cbuilder->api->visitJumpStmt(kctx, builder, stmt);   break;
		case TSTMT_TRY:   ret = cbuilder->api->visitTryStmt(kctx, builder, stmt);    break;
		default: cbuilder->api->visitUndefinedStmt(kctx, builder, stmt);        break;
	}
	return ret;
}

static void VisitExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	struct KBuilderCommon *cbuilder = (struct KBuilderCommon*)builder;
	int a = cbuilder->a;
	int espidx = cbuilder->espidx;
	int shift = cbuilder->shift;
	switch(expr->build) {
	case TEXPR_CONST:    cbuilder->api->visitConstExpr(kctx, builder, stmt, expr);  break;
	case TEXPR_NEW:      cbuilder->api->visitNewExpr(kctx, builder, stmt, expr);    break;
	case TEXPR_NULL:     cbuilder->api->visitNullExpr(kctx, builder, stmt, expr);   break;
	case TEXPR_NCONST:   cbuilder->api->visitNConstExpr(kctx, builder, stmt, expr); break;
	case TEXPR_LOCAL:    cbuilder->api->visitLocalExpr(kctx, builder, stmt, expr);  break;
	case TEXPR_BLOCK:    cbuilder->api->visitBlockExpr(kctx, builder, stmt, expr);  break;
	case TEXPR_FIELD:    cbuilder->api->visitFieldExpr(kctx, builder, stmt, expr);  break;
	case TEXPR_CALL:     cbuilder->api->visitCallExpr(kctx, builder, stmt, expr);   break;
	case TEXPR_AND:      cbuilder->api->visitAndExpr(kctx, builder, stmt, expr);    break;
	case TEXPR_OR:       cbuilder->api->visitOrExpr(kctx, builder, stmt, expr);     break;
	case TEXPR_LET:      cbuilder->api->visitLetExpr(kctx, builder, stmt, expr);    break;
	case TEXPR_STACKTOP: cbuilder->api->visitStackTopExpr(kctx, builder, stmt, expr);break;
	default: DBG_ABORT("unknown expr=%d", expr->build);
	}
	cbuilder->a = a;
	cbuilder->espidx = espidx;
	cbuilder->shift = shift;
}

static kbool_t VisitBlock(KonohaContext *kctx, KBuilder *builder, kBlock *block)
{
	struct KBuilderCommon *cbuilder = (struct KBuilderCommon*)builder;
	int a = cbuilder->a;
	int espidx = cbuilder->espidx;
	int shift = cbuilder->shift;
	cbuilder->espidx = (block->esp->build == TEXPR_STACKTOP) ? shift + block->esp->index : block->esp->index;
	size_t i;
	for (i = 0; i < kArray_size(block->StmtList); i++) {
		kStmt *stmt = block->StmtList->StmtItems[i];
		if(stmt->syn == NULL) continue;
		cbuilder->uline = stmt->uline;
		if(!VisitStmt(kctx, builder, stmt)) return false;
	}
	cbuilder->a = a;
	cbuilder->espidx = espidx;
	cbuilder->shift = shift;
	return true;
}

static void kMethod_GenCode(KonohaContext *kctx, kMethod *mtd, kBlock *block, int option)
{
	DBG_P("START CODE GENERATION..");
	kNameSpace *ns = block->BlockNameSpace;
	struct VirtualCode *vcode = ns->builderApi->GenerateVirtualCode(kctx, mtd, block, option);
	MethodFunc func = ns->builderApi->GenerateMethodFunc(kctx, vcode);
	((kMethodVar *)mtd)->invokeMethodFunc = func;
	((kMethodVar *)mtd)->pc_start = vcode;
}

static KMETHOD MethodFunc_invokeAbstractMethod(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(0);
}

static void kMethod_setFunc(KonohaContext *kctx, kMethod *mtd, MethodFunc func)
{
	func = (func != NULL) ? func : MethodFunc_invokeAbstractMethod;
	((kMethodVar *)mtd)->invokeMethodFunc = func;
	((kMethodVar *)mtd)->pc_start = PLATAPI GetDefaultBootCode();
}

