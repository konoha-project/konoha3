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

#include "visitor.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CASE(X) case X: Tag = #X; break

#define VISIT_VALUE(NODE, TAG, VAL) do {\
	visitor->VisitValue(visitor, NODE, TAG, VAL);\
} while(0)

#define VISIT_1(NODE, TAG, NODE1) do {\
	INode *_N1 = (NODE1);\
	visitor->Visit(visitor, NODE, TAG, 1, _N1);\
} while(0)

#define VISIT_2(NODE, TAG, NODE1, NODE2) do {\
	INode *_N1 = (NODE1);\
	INode *_N2 = (NODE2);\
	visitor->Visit(visitor, NODE, TAG, 2, _N1, _N2);\
} while(0)

#define VISIT_3(NODE, TAG, NODE1, NODE2, NODE3) do {\
	INode *_N1 = (NODE1);\
	INode *_N2 = (NODE2);\
	INode *_N3 = (NODE3);\
	visitor->Visit(visitor, NODE, TAG, 3, _N1, _N2, _N3);\
} while(0)

#define VISIT_N(NODE, TAG, ARY) do {\
	visitor->VisitList(visitor, NODE, TAG, ARRAY_size(ARY), ARRAY_list(ARY));\
} while(0)


static void visitBlock(Visitor *visitor, INode *Node)
{
}

static void visitIConstant(Visitor *visitor, INode *Node)
{
	IConstant *Inst = (IConstant *) Node;
	VISIT_VALUE(Node, 0, Inst->Value);
}

static void visitIArgument(Visitor *visitor, INode *Node)
{
	IArgument *Inst = (IArgument *) Node;
	SValue S; S.ival = Inst->Index;
	VISIT_VALUE(Node, 0, S);
}

static void visitIField(Visitor *visitor, INode *Node)
{
	IField *Inst = (IField *) Node;
	const char *Tag = "";
	switch(Inst->Op) {
		CASE(GlobalScope);
		CASE(EnvScope);
		CASE(FieldScope);
		CASE(LocalScope);
	}
	SValue S; S.ival = Inst->Id;
	VISIT_VALUE(Node, Tag, S);
}

static void visitINew(Visitor *visitor, INode *Node)
{
	INew *Inst = (INew *) Node;
	VISIT_N(Node, 0, Inst->Params);
}

static void visitICall(Visitor *visitor, INode *Node)
{
	ICall *Inst = (ICall *) Node;
	const char *Tag = "";
	switch(Inst->Op) {
		CASE(DefaultCall);
		CASE(VirtualCall);
		CASE(NeedTypeCheck);
	}
	VISIT_N(Node, Tag, Inst->Params);
}

static void visitIFunction(Visitor *visitor, INode *Node)
{
	IFunction *Func = (IFunction *) Node;
	VISIT_3(Node, 0, Func->Func, Func->Env, Func->Method);
}

static void visitIUpdate(Visitor *visitor, INode *Node)
{
	IUpdate *Update = (IUpdate *) Node;
	VISIT_2(Node, 0, (INode *)Update->LHS, Update->RHS);
}

static void visitIBranch(Visitor *visitor, INode *Node)
{
	IBranch *Inst = (IBranch *) Node;
	VISIT_3(Node, 0, Inst->Cond, (INode *)Inst->ThenBB, (INode *)Inst->ElseBB);
}

static void visitITest(Visitor *visitor, INode *Node)
{
	ITest *Inst = (ITest *) Node;
	const char *Tag = "";
	switch(Inst->Op) {
		CASE(TypeCheck);
		CASE(TypeGuard);
		CASE(SafePointCheck);
		CASE(Recompilation);
		CASE(BoundaryCheck);
	}
	VISIT_2(Node, Tag, (INode *)Inst->Value, (INode *) Inst->TargetBlock);
}

static void visitIReturn(Visitor *visitor, INode *Node)
{
	IReturn *Inst = (IReturn *) Node;
	VISIT_1(Node, 0, Inst->Inst);
}

static void visitIJump(Visitor *visitor, INode *Node)
{
	IJump *Inst = (IJump *) Node;
	VISIT_1(Node, 0, (INode *)Inst->TargetBlock);
}

static void visitIThrow(Visitor *visitor, INode *Node)
{
	IThrow *Inst = (IThrow *) Node;
	VISIT_1(Node, 0, (INode *)Inst->Val);
}

static void visitITry(Visitor *visitor, INode *Node)
{
	ITry *Inst = (ITry *) Node;
	VISIT_2(Node, 0, (INode *)Inst->TryBB, (INode *)Inst->FinallyBB);
}

static void visitIYield(Visitor *visitor, INode *Node)
{
	IYield *Inst = (IYield *) Node;
	VISIT_1(Node, 0, Inst->Value);
}

static void visitIUnary(Visitor *visitor, INode *Node)
{
	IUnary *Inst = (IUnary *) Node;
	const char *Tag = "";
	switch(Inst->Op) {
		CASE(Not);
		CASE(Neg);
		CASE(Box);
		default: break;
	}
	VISIT_1(Node, Tag, Inst->Node);
}

static void visitIBinary(Visitor *visitor, INode *Node)
{
	IBinary *Inst = (IBinary *) Node;
	const char *Tag = "";
	switch(Inst->Op) {
		CASE(Add); CASE(Sub); CASE(Mul); CASE(Div); CASE(Mod);
		CASE(LShift); CASE(RShift); CASE(And); CASE(Or); CASE(Xor);
		CASE(Eq); CASE(Nq); CASE(Gt); CASE(Ge); CASE(Lt); CASE(Le);
		default: break;
	}
	VISIT_2(Node, Tag, Inst->LHS, Inst->RHS);
}

static void visitIPHI(Visitor *visitor, INode *Node)
{
	IPHI *Inst = (IPHI *) Node;
	VISIT_N(Node, 0, Inst->Args); /* FIXME : trace Inst->Val */
}

static void visitERROR(Visitor *visitor, INode *Node)
{
	assert(0 && "unreachable");
}

typedef void (*VisitFn)(Visitor *visitor, INode *Node);
static const VisitFn Fn[] = {
	visitERROR,
	visitBlock,
#define IR_API_DECL(X) visit##X,
	IR_LIST(IR_API_DECL)
#undef IR_API_DECL
};

void visitINode(Visitor *visitor, INode *Node)
{
	if(Node) {
		Fn[Node->Kind](visitor, Node);
	}
}

void visitElement(Visitor *visitor, INode *Inst, const char *Tag, unsigned ElmSize, ...)
{
	va_list ap;
	unsigned i;
	va_start(ap, ElmSize);
	INodePtr List[ElmSize];
	for(i = 0; i < ElmSize; i++) {
		INode *Node = va_arg(ap, INode *);
		List[i] = Node;
	}
	va_end(ap);
	visitor->VisitList(visitor, Inst, Tag, ElmSize, List);
}

#undef CASE

#ifdef __cplusplus
} /* extern "C" */
#endif
