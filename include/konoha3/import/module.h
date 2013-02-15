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

#ifndef MODULE_H
#define MODULE_H

#ifndef kunused
#define kunused __attribute__((unused))
#endif /* kunused */

#ifdef USE_EXECUTIONENGINE
/*----------------------------------------------------------------------------*/
/* Konoha AST API */
static kunused kNode* kNode_getFirstBlock(KonohaContext *kctx, kNode *stmt)
{
	return SUGAR kNode_GetNode(kctx, stmt, KSymbol_BlockPattern, K_NULLBLOCK);
}

static kunused kNode* kNode_getElseBlock(KonohaContext *kctx, kNode *stmt)
{
	return SUGAR kNode_GetNode(kctx, stmt, KSymbol_else, K_NULLBLOCK);
}

static kunused kNode* kNode_getFirstNode(KonohaContext *kctx, kNode *stmt)
{
	return SUGAR kNode_GetNode(kctx, stmt, KSymbol_ExprPattern, NULL);
}

static kunused kMethod* CallNode_getMethod(kNode *expr)
{
	return expr->NodeList->MethodItems[0];
}

static kunused kNode *kNode_GetNode(KonohaContext *kctx, kNode *stmt, ksymbol_t kw)
{
	return SUGAR kNode_GetNode(kctx, stmt, kw, NULL);
}

static kunused int CallNode_getArgCount(kNode *node)
{
	return kArray_size(node->NodeList) - 2;
}

static kunused kString *kNode_getErrorMessage(KonohaContext *kctx, kNode *stmt)
{
	kString* msg = stmt->ErrorMessage;
	DBG_ASSERT(IS_String(msg));
	return msg;
}

#endif

#endif /* end of include guard */
