/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
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

static void kDoneNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kDoneNode *Node = (kDoneNode *) v[pos].asObject;
	KLIB KBuffer_printf(kctx, wb, "(Done %s)", KType_text(Node->typeAttr));
}

static void kConstNode_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kConstNode *Node = (kConstNode *) o;
	KFieldInit(Node, Node->ConstObject, K_NULL);
}

static void kConstNode_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kConstNode *Node = (kConstNode *) o;
	KRefTrace(Node->ConstObject);
}

static void kConstNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kConstNode *Node = (kConstNode *) v[pos].asObject;
	KLIB KBuffer_printf(kctx, wb, "(Const %s)", KType_text(Node->typeAttr));
}

static void kNewNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kNewNode *Node = (kNewNode *) v[pos].asObject;
	KLIB KBuffer_printf(kctx, wb, "(New %s)", KType_text(Node->typeAttr));
}

static void kNullNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kNullNode *Node = (kNullNode *) v[pos].asObject;
	KLIB KBuffer_printf(kctx, wb, "(Null %s)", KType_text(Node->typeAttr));
}

static void kLocalNode_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kLocalNode *Node = (kLocalNode *) o;
	KFieldInit(Node, Node->TermToken, K_NULL);
}

static void kLocalNode_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kLocalNode *Node = (kLocalNode *) o;
	KRefTrace(Node->TermToken);
}

static void kLocalNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kLocalNode *Node = (kLocalNode *) v[pos].asObject;
	kString    *Text = Node->TermToken->text;
	KLIB KBuffer_printf(kctx, wb, "(Local %s %s %d)", KType_text(Node->typeAttr), kString_text(Text), Node->Index);
}

static void kFieldNode_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kFieldNode *Node = (kFieldNode *) o;
	KFieldInit(Node, Node->TermToken, K_NULL);
}

static void kFieldNode_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kFieldNode *Node = (kFieldNode *) o;
	KRefTrace(Node->TermToken);
}

static void kFieldNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kFieldNode *Node = (kFieldNode *) v[pos].asObject;
	kString    *Text = Node->TermToken->text;
	KLIB KBuffer_printf(kctx, wb, "(Field %s %s %d %d)", KType_text(Node->typeAttr), kString_text(Text), Node->Index, Node->Xindex);
}

static void kBoxNode_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kBoxNode *Node = (kBoxNode *) o;
	KFieldInit(Node, Node->Expr, K_NULL);
}

static void kBoxNode_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kBoxNode *Node = (kBoxNode *) o;
	KRefTrace(Node->Expr);
}

static void kBoxNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kBoxNode *Node = (kBoxNode *) v[pos].asObject;
	KLIB KBuffer_printf(kctx, wb, "(Box %s ", KType_text(Node->typeAttr));
	KStackSetObjectValue(v[pos+1].asObject, UPCAST(Node->Expr));
	kObject_class(Node->Expr)->format(kctx, v, pos+1, wb);
	KLIB KBuffer_printf(kctx, wb, ")");
}

static void kMethodCallNode_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kMethodCallNode *Node = (kMethodCallNode *) o;
	KFieldInit(Node, Node->Method, K_NULL);
	KFieldInit(Node, Node->Params, K_NULL);
}

static void kMethodCallNode_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kMethodCallNode *Node = (kMethodCallNode *) o;
	KRefTrace(Node->Method);
	KRefTrace(Node->Params);
}

static void kMethodCallNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kMethodCallNode *Node = (kMethodCallNode *) v[pos].asObject;
	KLIB KBuffer_printf(kctx, wb, "(MethodCall %s %s.%s%s", KType_text(Node->typeAttr), kMethod_Fmt3(Node->Method));
	KStackSetObjectValue(v[pos+1].asObject, UPCAST(Node->Params));
	kObject_class(Node->Params)->format(kctx, v, pos+1, wb);
	KLIB KBuffer_printf(kctx, wb, ")");

}

static void kAndNode_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kAndNode *Node = (kAndNode *) o;
	KFieldInit(Node, Node->Left, K_NULL);
	KFieldInit(Node, Node->Right, K_NULL);
}

static void kAndNode_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kAndNode *Node = (kAndNode *) o;
	KRefTrace(Node->Left);
	KRefTrace(Node->Right);
}

static void kAndNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kAndNode *Node = (kAndNode *) v[pos].asObject;
	KLIB KBuffer_printf(kctx, wb, "(And %s ", KType_text(Node->typeAttr));
	KStackSetObjectValue(v[pos+1].asObject, UPCAST(Node->Left));
	kObject_class(Node->Left)->format(kctx, v, pos+1, wb);
	KLIB KBuffer_printf(kctx, wb, ",");
	KStackSetObjectValue(v[pos+1].asObject, UPCAST(Node->Right));
	kObject_class(Node->Right)->format(kctx, v, pos+1, wb);
	KLIB KBuffer_printf(kctx, wb, ")");
}

static void kOrNode_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kAndNode_init(kctx, o, conf);
}

static void kOrNode_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kAndNode_reftrace(kctx, o, visitor);
}

static void kOrNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kOrNode *Node = (kOrNode *) v[pos].asObject;
	KLIB KBuffer_printf(kctx, wb, "(Or %s ", KType_text(Node->typeAttr));
	KStackSetObjectValue(v[pos+1].asObject, UPCAST(Node->Left));
	kObject_class(Node->Left)->format(kctx, v, pos+1, wb);
	KLIB KBuffer_printf(kctx, wb, ",");
	KStackSetObjectValue(v[pos+1].asObject, UPCAST(Node->Right));
	kObject_class(Node->Right)->format(kctx, v, pos+1, wb);
	KLIB KBuffer_printf(kctx, wb, ")");
}

static void kAssignNode_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kAssignNode *Node = (kAssignNode *) o;
	KFieldInit(Node, Node->TermToken, K_NULL);
	KFieldInit(Node, Node->Right, K_NULL);
}

static void kAssignNode_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kAssignNode *Node = (kAssignNode *) o;
	KRefTrace(Node->TermToken);
	KRefTrace(Node->Right);
}

static void kAssignNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kAssignNode *Node = (kAssignNode *) v[pos].asObject;
	kString     *Text = Node->TermToken->text;
	KLIB KBuffer_printf(kctx, wb, "(Assign %s (%s %d) = ", KType_text(Node->typeAttr), kString_text(Text), Node->Index);
	KStackSetObjectValue(v[pos+1].asObject, UPCAST(Node->Right));
	kObject_class(Node->Right)->format(kctx, v, pos+1, wb);
	KLIB KBuffer_printf(kctx, wb, ")");
}

static void kLetNode_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kLetNode *Node = (kLetNode *) o;
	KFieldInit(Node, Node->TermToken, K_NULL);
	KFieldInit(Node, Node->Right, K_NULL);
	KFieldInit(Node, Node->Block, K_NULL);
}

static void kLetNode_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kLetNode *Node = (kLetNode *) o;
	KRefTrace(Node->TermToken);
	KRefTrace(Node->Right);
	KRefTrace(Node->Block);
}

static void kLetNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kLetNode *Node = (kLetNode *) v[pos].asObject;
	kString  *Text = Node->TermToken->text;
	KLIB KBuffer_printf(kctx, wb, "(Let %s (%s %d) = ", KType_text(Node->typeAttr), kString_text(Text), Node->Index);
	KStackSetObjectValue(v[pos+1].asObject, UPCAST(Node->Right));
	kObject_class(Node->Right)->format(kctx, v, pos+1, wb);
	KLIB KBuffer_printf(kctx, wb, " in ");
	KStackSetObjectValue(v[pos+1].asObject, UPCAST(Node->Block));
	kObject_class(Node->Block)->format(kctx, v, pos+1, wb);
	KLIB KBuffer_printf(kctx, wb, ")");
}

static void kBlockNode_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kBlockNode *Node = (kBlockNode *) o;
	KFieldInit(Node, Node->ExprList, K_NULL);
}

static void kBlockNode_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kBlockNode *Node = (kBlockNode *) o;
	KRefTrace(Node->ExprList);
}

static void kBlockNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kBlockNode *Node = (kBlockNode *) v[pos].asObject;
	KLIB KBuffer_printf(kctx, wb, "(Block %s)", KType_text(Node->typeAttr));
}

static void kIfNode_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kIfNode *Node = (kIfNode *) o;
	KFieldInit(Node, Node->CondExpr, K_NULL);
	KFieldInit(Node, Node->ThenBlock, K_NULL);
	KFieldInit(Node, Node->ElseBlock, K_NULL);
}

static void kIfNode_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kIfNode *Node = (kIfNode *) o;
	KRefTrace(Node->CondExpr);
	KRefTrace(Node->ThenBlock);
	KRefTrace(Node->ElseBlock);
}

static void kIfNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kIfNode *Node = (kIfNode *) v[pos].asObject;
	KLIB KBuffer_printf(kctx, wb, "(If %s ", KType_text(Node->typeAttr));
	KStackSetObjectValue(v[pos+1].asObject, UPCAST(Node->CondExpr));
	kObject_class(Node->CondExpr)->format(kctx, v, pos+1, wb);
	KLIB KBuffer_printf(kctx, wb, " then ");
	KStackSetObjectValue(v[pos+1].asObject, UPCAST(Node->ThenBlock));
	kObject_class(Node->ThenBlock)->format(kctx, v, pos+1, wb);
	KLIB KBuffer_printf(kctx, wb, " else ");
	KStackSetObjectValue(v[pos+1].asObject, UPCAST(Node->ElseBlock));
	kObject_class(Node->ElseBlock)->format(kctx, v, pos+1, wb);
	KLIB KBuffer_printf(kctx, wb, ")");

}

static void kSwitchNode_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kSwitchNode *Node = (kSwitchNode *) o;
	KFieldInit(Node, Node->CondExpr, K_NULL);
	KFieldInit(Node, Node->Labels, K_NULL);
	KFieldInit(Node, Node->Blocks, K_NULL);
}

static void kSwitchNode_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kSwitchNode *Node = (kSwitchNode *) o;
	KRefTrace(Node->CondExpr);
	KRefTrace(Node->Labels);
	KRefTrace(Node->Blocks);
}

static void kSwitchNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kSwitchNode *Node = (kSwitchNode *) v[pos].asObject;
	KLIB KBuffer_printf(kctx, wb, "(Switch %s)", KType_text(Node->typeAttr));
}

static void kLoopNode_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kLoopNode *Node = (kLoopNode *) o;
	KFieldInit(Node, Node->CondExpr, K_NULL);
	KFieldInit(Node, Node->LoopBody, K_NULL);
	KFieldInit(Node, Node->IterationExpr, K_NULL);
}

static void kLoopNode_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kLoopNode *Node = (kLoopNode *) o;
	KRefTrace(Node->CondExpr);
	KRefTrace(Node->LoopBody);
	KRefTrace(Node->IterationExpr);
}

static void kLoopNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kLoopNode *Node = (kLoopNode *) v[pos].asObject;
	KLIB KBuffer_printf(kctx, wb, "(Loop %s)", KType_text(Node->typeAttr));
}

static void kReturnNode_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kReturnNode *Node = (kReturnNode *) o;
	KFieldInit(Node, Node->Expr, K_NULL);
}

static void kReturnNode_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kReturnNode *Node = (kReturnNode *) o;
	KRefTrace(Node->Expr);
}

static void kReturnNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kReturnNode *Node = (kReturnNode *) v[pos].asObject;
	KLIB KBuffer_printf(kctx, wb, "(Return %s)", KType_text(Node->typeAttr));
}

static void kLabelNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kLabelNode *Node = (kLabelNode *) v[pos].asObject;
	KLIB KBuffer_printf(kctx, wb, "(Label %s %s)", KType_text(Node->typeAttr), KSymbol_text(Node->Label));
}

static void kJumpNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kJumpNode *Node = (kJumpNode *) v[pos].asObject;
	KLIB KBuffer_printf(kctx, wb, "(Jump %s %s)", KType_text(Node->typeAttr), KSymbol_text(Node->Label));
}

static void kTryNode_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kTryNode *Node = (kTryNode *) o;
	KFieldInit(Node, Node->TryBlock, K_NULL);
	KFieldInit(Node, Node->CatchedExceptionPairs, K_NULL);
	KFieldInit(Node, Node->FinallyBlock, K_NULL);
}

static void kTryNode_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kTryNode *Node = (kTryNode *) o;
	KRefTrace(Node->TryBlock);
	KRefTrace(Node->CatchedExceptionPairs);
	KRefTrace(Node->FinallyBlock);
}

static void kTryNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kTryNode *Node = (kTryNode *) v[pos].asObject;
	KLIB KBuffer_printf(kctx, wb, "(Try %s)", KType_text(Node->typeAttr));
}

static void kThrowNode_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kThrowNode *Node = (kThrowNode *) o;
	KFieldInit(Node, Node->ExceptionExpr, K_NULL);
}

static void kThrowNode_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kThrowNode *Node = (kThrowNode *) o;
	KRefTrace(Node->ExceptionExpr);
}

static void kThrowNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kThrowNode *Node = (kThrowNode *) v[pos].asObject;
	KLIB KBuffer_printf(kctx, wb, "(Throw %s)", KType_text(Node->typeAttr));
}

static void kFunctionNode_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kFunctionNode *Node = (kFunctionNode *) o;
	KFieldInit(Node, Node->EnvExprList, K_NULL);
	KFieldInit(Node, Node->ParamList, K_NULL);
}

static void kFunctionNode_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kFunctionNode *Node = (kFunctionNode *) o;
	KRefTrace(Node->EnvExprList);
	KRefTrace(Node->ParamList);
}

static void kFunctionNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kFunctionNode *Node = (kFunctionNode *) v[pos].asObject;
	KLIB KBuffer_printf(kctx, wb, "(Function %s)", KType_text(Node->typeAttr));
}

static void kErrorNode_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kErrorNode *Node = (kErrorNode *) o;
	KFieldInit(Node, Node->ErrorMessage, K_NULL);
}

static void kErrorNode_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kErrorNode *Node = (kErrorNode *) o;
	KRefTrace(Node->ErrorMessage);
}

static void kErrorNode_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kErrorNode *Node   = (kErrorNode *) v[pos].asObject;
	kString    *ErrMsg = Node->ErrorMessage;
	KLIB KBuffer_printf(kctx, wb, "(Error %s '%s')", KType_text(Node->typeAttr), kString_text(ErrMsg));
}


static void InitNodeClass(KonohaContext *kctx, KParserModel *mod)
{
	KDEFINE_CLASS defDone = {0};
	SETSTRUCTNAME(defDone, DoneNode);
	defDone.format   = kDoneNode_format;
	mod->cDoneNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defDone, 0);

	KDEFINE_CLASS defConst = {0};
	SETSTRUCTNAME(defConst, ConstNode);
	defConst.init     = kConstNode_init;
	defConst.reftrace = kConstNode_reftrace;
	defConst.format   = kConstNode_format;
	mod->cConstNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defConst, 0);

	KDEFINE_CLASS defNew = {0};
	SETSTRUCTNAME(defNew, NewNode);
	defNew.format   = kNewNode_format;
	mod->cNewNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defNew, 0);

	KDEFINE_CLASS defNull = {0};
	SETSTRUCTNAME(defNull, NullNode);
	defNull.format   = kNullNode_format;
	mod->cNullNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defNull, 0);

	KDEFINE_CLASS defLocal = {0};
	SETSTRUCTNAME(defLocal, LocalNode);
	defLocal.init     = kLocalNode_init;
	defLocal.reftrace = kLocalNode_reftrace;
	defLocal.format   = kLocalNode_format;
	mod->cLocalNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defLocal, 0);

	KDEFINE_CLASS defField = {0};
	SETSTRUCTNAME(defField, FieldNode);
	defField.init     = kFieldNode_init;
	defField.reftrace = kFieldNode_reftrace;
	defField.format   = kFieldNode_format;
	mod->cFieldNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defField, 0);

	KDEFINE_CLASS defBox = {0};
	SETSTRUCTNAME(defBox, BoxNode);
	defBox.init     = kBoxNode_init;
	defBox.reftrace = kBoxNode_reftrace;
	defBox.format   = kBoxNode_format;
	mod->cBoxNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defBox, 0);

	KDEFINE_CLASS defMethodCall = {0};
	SETSTRUCTNAME(defMethodCall, MethodCallNode);
	defMethodCall.init     = kMethodCallNode_init;
	defMethodCall.reftrace = kMethodCallNode_reftrace;
	defMethodCall.format   = kMethodCallNode_format;
	mod->cMethodCallNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defMethodCall, 0);

	KDEFINE_CLASS defAnd = {0};
	SETSTRUCTNAME(defAnd, AndNode);
	defAnd.init     = kAndNode_init;
	defAnd.reftrace = kAndNode_reftrace;
	defAnd.format   = kAndNode_format;
	mod->cAndNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defAnd, 0);

	KDEFINE_CLASS defOr = {0};
	SETSTRUCTNAME(defOr, OrNode);
	defOr.init     = kOrNode_init;
	defOr.reftrace = kOrNode_reftrace;
	defOr.format   = kOrNode_format;
	mod->cOrNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defOr, 0);

	KDEFINE_CLASS defAssign = {0};
	SETSTRUCTNAME(defAssign, AssignNode);
	defAssign.init     = kAssignNode_init;
	defAssign.reftrace = kAssignNode_reftrace;
	defAssign.format   = kAssignNode_format;
	mod->cAssignNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defAssign, 0);

	KDEFINE_CLASS defLet = {0};
	SETSTRUCTNAME(defLet, LetNode);
	defLet.init     = kLetNode_init;
	defLet.reftrace = kLetNode_reftrace;
	defLet.format   = kLetNode_format;
	mod->cLetNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defLet, 0);

	KDEFINE_CLASS defBlock = {0};
	SETSTRUCTNAME(defBlock, BlockNode);
	defBlock.init     = kBlockNode_init;
	defBlock.reftrace = kBlockNode_reftrace;
	defBlock.format   = kBlockNode_format;
	mod->cBlockNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defBlock, 0);

	KDEFINE_CLASS defIf = {0};
	SETSTRUCTNAME(defIf, IfNode);
	defIf.init     = kIfNode_init;
	defIf.reftrace = kIfNode_reftrace;
	defIf.format   = kIfNode_format;
	mod->cIfNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defIf, 0);

	KDEFINE_CLASS defSwitch = {0};
	SETSTRUCTNAME(defSwitch, SwitchNode);
	defSwitch.init     = kSwitchNode_init;
	defSwitch.reftrace = kSwitchNode_reftrace;
	defSwitch.format   = kSwitchNode_format;
	mod->cSwitchNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defSwitch, 0);

	KDEFINE_CLASS defLoop = {0};
	SETSTRUCTNAME(defLoop, LoopNode);
	defLoop.init     = kLoopNode_init;
	defLoop.reftrace = kLoopNode_reftrace;
	defLoop.format   = kLoopNode_format;
	mod->cLoopNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defLoop, 0);

	KDEFINE_CLASS defReturn = {0};
	SETSTRUCTNAME(defReturn, ReturnNode);
	defReturn.init     = kReturnNode_init;
	defReturn.reftrace = kReturnNode_reftrace;
	defReturn.format   = kReturnNode_format;
	mod->cReturnNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defReturn, 0);

	KDEFINE_CLASS defLabel = {0};
	SETSTRUCTNAME(defLabel, LabelNode);
	defLabel.format   = kLabelNode_format;
	mod->cLabelNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defLabel, 0);

	KDEFINE_CLASS defJump = {0};
	SETSTRUCTNAME(defJump, JumpNode);
	defJump.format   = kJumpNode_format;
	mod->cJumpNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defJump, 0);

	KDEFINE_CLASS defTry = {0};
	SETSTRUCTNAME(defTry, TryNode);
	defTry.init     = kTryNode_init;
	defTry.reftrace = kTryNode_reftrace;
	defTry.format   = kTryNode_format;
	mod->cTryNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defTry, 0);

	KDEFINE_CLASS defThrow = {0};
	SETSTRUCTNAME(defThrow, ThrowNode);
	defThrow.init     = kThrowNode_init;
	defThrow.reftrace = kThrowNode_reftrace;
	defThrow.format   = kThrowNode_format;
	mod->cThrowNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defThrow, 0);

	KDEFINE_CLASS defFunction = {0};
	SETSTRUCTNAME(defFunction, FunctionNode);
	defFunction.init     = kFunctionNode_init;
	defFunction.reftrace = kFunctionNode_reftrace;
	defFunction.format   = kFunctionNode_format;
	mod->cFunctionNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defFunction, 0);

	KDEFINE_CLASS defError = {0};
	SETSTRUCTNAME(defError, ErrorNode);
	defError.init     = kErrorNode_init;
	defError.reftrace = kErrorNode_reftrace;
	defError.format   = kErrorNode_format;
	mod->cErrorNode = KLIB KClass_define(kctx, PackageId_sugar, NULL, &defError, 0);

}
