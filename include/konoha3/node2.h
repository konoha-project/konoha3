/****************************************************************************
 * Copyright (c) 2013, the Konoha project authors. All rights reserved.
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

#ifndef NODE2_H_
#define NODE2_H_

struct kUntypedNode {
	kObjectHeader h;
	khalfword_t nodeType; ktypeattr_t typeAttr;
	union {
		struct kUntypedNode *Parent;   /* if parent is a NameSpace, it is a root node */
		kNameSpace *RootNodeNameSpace;
	};
	union {
		kToken        *KeyOperatorToken;     // Node
		kToken        *TermToken;            // Term
	};
	union {
		kArray          *NodeList;       // Node
		kNameSpace      *StmtNameSpace;  // Statement
		kString         *ErrorMessage;   // KNode_Error
	};
	union {
		kSyntax       *syn;  /* untyped */
		uintptr_t      unboxConstValue;
		intptr_t       index;
		kObject*       ObjectConstValue;
	};
};

typedef struct kNodeBase {
#define KNODE_BASE_STRUCT \
	kObjectHeader h;\
	khalfword_t nodeType; ktypeattr_t typeAttr
	KNODE_BASE_STRUCT;
} kNodeBase;

#define NODE_LIST_OP(OP)\
	OP(Done)\
	OP(Const)\
	OP(New)\
	OP(Null)\
	OP(Local)\
	OP(Field)\
	OP(Box)\
	OP(MethodCall)\
	OP(And)\
	OP(Or)\
	OP(Assign)\
	OP(Let)\
	OP(Block)\
	OP(If)\
	OP(Switch)\
	OP(Loop)\
	OP(Return)\
	OP(Label)\
	OP(Jump)\
	OP(Try)\
	OP(Throw)\
	OP(Function)\
	OP(Error)

typedef struct kDoneNode {
	KNODE_BASE_STRUCT;
	/* NOP */
} kDoneNode;

typedef struct kConstNode {
	KNODE_BASE_STRUCT;
	/* IsUnboxType(typeAttr) ? ConstValue : ConstObject */
	kObject   *ConstObject;
	uintptr_t  ConstValue;
} kConstNode;

typedef struct kNewNode {
	KNODE_BASE_STRUCT;
	/* ALLOC $TypeAttr */
} kNewNode;

typedef struct kNullNode {
	KNODE_BASE_STRUCT;
	/* (($TypeAttr) null) */
} kNullNode;

typedef struct kLocalNode {
	KNODE_BASE_STRUCT;
	/* frame[$Index] (or TermToken->text) */
	kToken *TermToken;
	kuhalfword_t Index; kuhalfword_t _unused;
} kLocalNode;

typedef struct kFieldNode {
	KNODE_BASE_STRUCT;
	/* frame[Index][Xindex] (or ($TermToken->text)[Xindex] */
	kToken *TermToken;
	kuhalfword_t Index; kuhalfword_t Xindex;
} kFieldNode;

typedef struct kBoxNode {
	KNODE_BASE_STRUCT;
	/* Object(Expr) */
	kNodeBase *Expr;
} kBoxNode;

typedef struct kMethodCallNode {
	KNODE_BASE_STRUCT;
	/* call self.Method(arg1, arg2, ...) */
	kMethod *Method;
	kArray  *Params; /* [this, arg1, arg2, ...] */
} kMethodCallNode;

typedef struct kAndNode {
	KNODE_BASE_STRUCT;
	/* (Left && Right) */
	kNodeBase *Left;
	kNodeBase *Right;
} kAndNode;

typedef struct kOrNode {
	KNODE_BASE_STRUCT;
	/* (Left || Right) */
	kNodeBase *Left;
	kNodeBase *Right;
} kOrNode;

typedef struct kAssignNode {
	KNODE_BASE_STRUCT;
	/* frame[Index] = Right */
	kToken    *TermToken;
	uintptr_t Index;
	kNodeBase *Left;
	kNodeBase *Right;
} kAssignNode;

typedef struct kLetNode {
	KNODE_BASE_STRUCT;
	/* let frame[Index] = Right in Block end*/
	kToken    *TermToken;
	uintptr_t Index;
	kNodeBase *Right;
	kNodeBase *Block;
} kLetNode;

typedef struct kBlockNode {
	KNODE_BASE_STRUCT;
	/* [Expr1, Expr2, ... ]*/
	kArray *ExprList;
} kBlockNode;

typedef struct kIfNode {
	KNODE_BASE_STRUCT;
	/* If CondExpr then ThenBlock else ElseBlock */
	kNodeBase *CondExpr;
	kNodeBase *ThenBlock;
	kNodeBase *ElseBlock;
} kIfNode;

typedef struct kSwitchNode {
	KNODE_BASE_STRUCT;
	/* switch CondExpr {
	 *  Label[0]:
	 *    Blocks[0];
	 *  Label[1]:
	 *    Blocks[2];
	 *  ...
	 * }
	 */
	kNodeBase *CondExpr;
	kArray *Labels;
	kArray *Blocks;
} kSwitchNode;

typedef struct kLoopNode {
	KNODE_BASE_STRUCT;
	/* while CondExpr then { LoopBlock; IterationExpr } */
	kNodeBase *CondExpr;
	kNodeBase *LoopBody;
	kNodeBase *IterationExpr;
} kLoopNode;

typedef struct kReturnNode {
	KNODE_BASE_STRUCT;
	/* return Expr */
	kNodeBase *Expr;
} kReturnNode;

typedef struct kLabelNode {
	KNODE_BASE_STRUCT;
	/* Label: */
	ksymbol_t Label;
} kLabelNode;

typedef struct kJumpNode {
	KNODE_BASE_STRUCT;
	/* goto Label */
	ksymbol_t Label;
} kJumpNode;

typedef struct kTryNode {
	KNODE_BASE_STRUCT;
	/* let
	 *   HasException = TRY(TryBlock);
	 * in
	 *   if HasException == CatchedExceptions[0] then CatchBlock[0]
	 *   if HasException == CatchedExceptions[1] then CatchBlock[1]
	 *   ...
	 *   FinallyBlock
	 * end
	 * */
	kNodeBase *TryBlock;
	kArray *CatchedExceptionPairs; /* (ExceptionType, CatchBlock) */
	kNodeBase *FinallyBlock;
} kTryNode;

typedef struct kThrowNode {
	KNODE_BASE_STRUCT;
	/* THROW ExceptionExpr */
	kNodeBase *ExceptionExpr;
} kThrowNode;

typedef struct kFunctionNode {
	KNODE_BASE_STRUCT;
	/* [Method, DefaultObject, [Env1, Env2, ...., EnvN]] */
	/*
	 * void f() {
	 *   int Env1, Env2;
	 *   return function (int Param1, int Param2) int {
	 *     return Env1 + Env2 + Param1 + Param2; } (10, 20);
	 *   }
	 * }
	 */
	kMethod *Method;
	kNameSpace *NS;
	kArray  *EnvList;
	} kFunctionNode;

typedef struct kErrorNode {
	KNODE_BASE_STRUCT;
	kString *ErrorMessage;
} kErrorNode;

typedef union kNodeImpl {
	kUntypedNode asUntypedNode;
	kNodeBase    asBaseNode;
#define DEFINE_UNION_FIELD(T) struct k##T##Node as##T;
	NODE_LIST_OP(DEFINE_UNION_FIELD)
#undef DEFINE_UNION_FIELD
} kNodeImpl;

typedef enum KNodeType {
	KNode_UntypedNode,
#define DEFINE_NODE_TYPE(T) KNode_##T,
	NODE_LIST_OP(DEFINE_NODE_TYPE)
#undef DEFINE_NODE_TYPE
	KNode_MAX
} KNodeType;

#define KClass_DoneNode       SUGAR cDoneNode
#define KClass_ConstNode      SUGAR cConstNode
#define KClass_NewNode        SUGAR cNewNode
#define KClass_NullNode       SUGAR cNullNode
#define KClass_LocalNode      SUGAR cLocalNode
#define KClass_FieldNode      SUGAR cFieldNode
#define KClass_BoxNode        SUGAR cBoxNode
#define KClass_MethodCallNode SUGAR cMethodCallNode
#define KClass_AndNode        SUGAR cAndNode
#define KClass_OrNode         SUGAR cOrNode
#define KClass_AssignNode     SUGAR cAssignNode
#define KClass_LetNode        SUGAR cLetNode
#define KClass_BlockNode      SUGAR cBlockNode
#define KClass_IfNode         SUGAR cIfNode
#define KClass_SwitchNode     SUGAR cSwitchNode
#define KClass_LoopNode       SUGAR cLoopNode
#define KClass_ReturnNode     SUGAR cReturnNode
#define KClass_LabelNode      SUGAR cLabelNode
#define KClass_JumpNode       SUGAR cJumpNode
#define KClass_TryNode        SUGAR cTryNode
#define KClass_ThrowNode      SUGAR cThrowNode
#define KClass_FunctionNode   SUGAR cFunctionNode
#define KClass_ErrorNode      SUGAR cErrorNode
#define KClass_UntypedNode    SUGAR cUntypedNode

#define KType_DoneNode       KClass_DoneNode      ->typeId
#define KType_ConstNode      KClass_ConstNode     ->typeId
#define KType_NewNode        KClass_NewNode       ->typeId
#define KType_NullNode       KClass_NullNode      ->typeId
#define KType_LocalNode      KClass_LocalNode     ->typeId
#define KType_FieldNode      KClass_FieldNode     ->typeId
#define KType_BoxNode        KClass_BoxNode       ->typeId
#define KType_MethodCallNode KClass_MethodCallNode->typeId
#define KType_AndNode        KClass_AndNode       ->typeId
#define KType_OrNode         KClass_OrNode        ->typeId
#define KType_AssignNode     KClass_AssignNode    ->typeId
#define KType_LetNode        KClass_LetNode       ->typeId
#define KType_BlockNode      KClass_BlockNode     ->typeId
#define KType_IfNode         KClass_IfNode        ->typeId
#define KType_SwitchNode     KClass_SwitchNode    ->typeId
#define KType_LoopNode       KClass_LoopNode      ->typeId
#define KType_ReturnNode     KClass_ReturnNode    ->typeId
#define KType_LabelNode      KClass_LabelNode     ->typeId
#define KType_JumpNode       KClass_JumpNode      ->typeId
#define KType_TryNode        KClass_TryNode       ->typeId
#define KType_ThrowNode      KClass_ThrowNode     ->typeId
#define KType_FunctionNode   KClass_FunctionNode  ->typeId
#define KType_ErrorNode      KClass_ErrorNode     ->typeId
#define KType_UntypedNode    KClass_UntypedNode   ->typeId

typedef struct KNodeFactory {
	kNodeBase *(*CreateDoneNode)(KonohaContext *kctx, ktypeattr_t Type);
	kNodeBase *(*CreateConstNode)(KonohaContext *kctx, ktypeattr_t Type, kObject *Obj, uintptr_t Val);
	kNodeBase *(*CreateNewNode)(KonohaContext *kctx, ktypeattr_t Type);
	kNodeBase *(*CreateNullNode)(KonohaContext *kctx, ktypeattr_t Type);
	kNodeBase *(*CreateLocalNode)(KonohaContext *kctx, ktypeattr_t Type, kToken *Term, uintptr_t Index);
	kNodeBase *(*CreateFieldNode)(KonohaContext *kctx, ktypeattr_t Type, kToken *Term, kuhalfword_t Index, kuhalfword_t Xindex);
	kNodeBase *(*CreateBoxNode)(KonohaContext *kctx, ktypeattr_t Type, kNodeBase *Expr);
	kNodeBase *(*CreateMethodCallNode)(KonohaContext *kctx, ktypeattr_t Type, kMethod *Method);
	kNodeBase *(*CreateAndNode)(KonohaContext *kctx, ktypeattr_t Type, kNodeBase *Left, kNodeBase *Right);
	kNodeBase *(*CreateOrNode)(KonohaContext *kctx, ktypeattr_t Type, kNodeBase *Left, kNodeBase *Right);
	kNodeBase *(*CreateAssignNode)(KonohaContext *kctx, ktypeattr_t Type, kToken *Term, uintptr_t Index, kNodeBase *Left, kNodeBase *Right);
	kNodeBase *(*CreateLetNode)(KonohaContext *kctx, ktypeattr_t Type, kToken *Term, uintptr_t Index, kNodeBase *Right, kNodeBase *Block);
	kNodeBase *(*CreateBlockNode)(KonohaContext *kctx, ktypeattr_t Type, kArray *ExprList, unsigned begin, unsigned end);
	kNodeBase *(*CreateIfNode)(KonohaContext *kctx, ktypeattr_t Type, kNodeBase *Cond, kNodeBase *Then, kNodeBase *Else);
	kNodeBase *(*CreateSwitchNode)(KonohaContext *kctx, ktypeattr_t Type, kNodeBase *Cond);
	kNodeBase *(*CreateLoopNode)(KonohaContext *kctx, ktypeattr_t Type, kNodeBase *Cond, kNodeBase *Loop, kNodeBase *Iter);
	kNodeBase *(*CreateReturnNode)(KonohaContext *kctx, ktypeattr_t Type, kNodeBase *Expr);
	kNodeBase *(*CreateLabelNode)(KonohaContext *kctx, ktypeattr_t Type, ksymbol_t Label);
	kNodeBase *(*CreateJumpNode)(KonohaContext *kctx, ktypeattr_t Type, ksymbol_t Label);
	kNodeBase *(*CreateTryNode)(KonohaContext *kctx, ktypeattr_t Type, kNodeBase *TryBlock, kNodeBase *FinallyBlock);
	kNodeBase *(*CreateThrowNode)(KonohaContext *kctx, ktypeattr_t Type, kNodeBase *ExceptionExpr);
	kNodeBase *(*CreateFunctionNode)(KonohaContext *kctx, ktypeattr_t Type, kMethod *Method, kNameSpace *NS);
	kNodeBase *(*CreateErrorNode)(KonohaContext *kctx, ktypeattr_t Type, kString *ErrorMessage);
} KNodeFactory;

#endif /* NODE2_H_ */
