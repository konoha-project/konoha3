/****************************************************************************
 * Copyright (c) 2012, Masahiro Ide <ide@konohascript.org> All rights reserved.
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

#include "codegen.h"

#ifndef VISITOR_H
#define VISITOR_H

#ifdef __cplusplus
extern "C" {
#endif

struct Visitor;
typedef void (*VisitNode)(struct Visitor *visitor, INode *Node);
typedef struct Visitor {
	void *Context;
	void (*Visit)(struct Visitor *visitor, INode *Node, const char *Tag, unsigned, ...);
	void (*VisitList)(struct Visitor *visitor, INode *Node, const char *Tag, unsigned, INodePtr *);
	void (*VisitValue)(struct Visitor *visitor, INode *Node, const char *Tag, SValue Val);
} Visitor;

extern void visitINode(Visitor *visitor, INode *Node);
extern void visitElement(Visitor *visitor, INode *Inst, const char *Tag, unsigned ElmSize, ...);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* end of include guard */
