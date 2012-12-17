/****************************************************************************
 * Copyright (c) 2012, the Konoha project authors. All rights reserved.
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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>

#ifdef __cplusplus
extern "C"{
#endif

// --------------------------------------------------------------------------

static kNode* NewNode(KonohaContext *kctx, KSyntax *syn, kToken *tk, ktypeattr_t ty)
{
	kNodeVar *expr = new_(NodeVar, syn, OnGcStack);
	KFieldSet(expr, expr->TermToken, tk);
	kNode_SetTerm(expr, 1);
	expr->node = TEXPR_NEW;
	expr->attrTypeId = ty;
	return (kNode *)expr;
}

static KMETHOD Nodeession_new(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Nodeession(stmt, tokenList, beginIdx, currentIdx, endIdx);
	DBG_ASSERT(beginIdx == currentIdx);
	if(beginIdx + 1 < endIdx) {
		kTokenVar *newToken = tokenList->TokenVarItems[beginIdx];
		KClass *foundClass = NULL;
		kNameSpace *ns = Node_ns(stmt);
		int nextIdx = SUGAR TokenUtils_ParseTypePattern(kctx, ns, tokenList, beginIdx + 1, endIdx, &foundClass);
		if((size_t)nextIdx < kArray_size(tokenList)) {
			kToken *nextTokenAfterClassName = tokenList->TokenItems[nextIdx];
			if(nextTokenAfterClassName->resolvedSyntaxInfo->keyword == KSymbol_ParenthesisGroup) {  // new C (...)
				KSyntax *syn = KSyntax_(ns, KSymbol_NodeMethodCall);
				kNode *expr = SUGAR new_UntypedCallStyleNode(kctx, syn, 2, newToken, NewNode(kctx, syn, tokenList->TokenVarItems[beginIdx+1], foundClass->typeId));
				newToken->resolvedSymbol = MN_new;
				KReturn(expr);
			}
			KSyntax *newsyn = KSyntax_(ns, KSymbol_("new"));
			if(nextTokenAfterClassName->resolvedSyntaxInfo->keyword == KSymbol_BracketGroup) {     // new int [100]
				kArray *subTokenList = nextTokenAfterClassName->subTokenList;
				KClass *classT0 = NULL;
				kNode *expr;
				int hasGenerics = -1;
				if(kArray_size(subTokenList) > 0) {
					hasGenerics = SUGAR TokenUtils_ParseTypePattern(kctx, ns, subTokenList, 0, kArray_size(subTokenList), &classT0);
				}
				if(hasGenerics != -1) {
					/* new Type1[Type2[]] => Type1<Type2>.new Or Type1<Type2>.newList */
					KClass *realType = KClass_p0(kctx, foundClass, classT0->typeId);
					KSyntax *syn;// = (realType->baseTypeId != KType_Array) ? KSyntax_(ns, KSymbol_NodeMethodCall) : newsyn;
					syn = newsyn;
					newToken->resolvedSymbol = (realType->baseTypeId != KType_Array) ? MN_new : KKMethodName_("newArray");
					expr = SUGAR new_UntypedCallStyleNode(kctx, syn, 2, newToken,
							NewNode(kctx, syn, tokenList->TokenVarItems[beginIdx+1], realType->typeId));
				} else {
					/* new Type1[] => Array<Type1>.newList */
					KClass *arrayClass = KClass_p0(kctx, KClass_Array, foundClass->typeId);
					newToken->resolvedSymbol = KKMethodName_("newArray");
					expr = SUGAR new_UntypedCallStyleNode(kctx, newsyn, 2, newToken,
							NewNode(kctx, newsyn, tokenList->TokenVarItems[beginIdx+1], arrayClass->typeId));
				}
				KReturn(expr);
			}
		}
	}
}

// ----------------------------------------------------------------------------
/* define class */

static kbool_t new_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("new"), 0, NULL, 0, Precedence_CStyleCALL, NULL, Nodeession_new, NULL, NULL, NULL, },
		{ KSymbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}

// --------------------------------------------------------------------------

static kbool_t new_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	new_defineSyntax(kctx, ns, trace);
	return true;
}

static kbool_t new_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* new_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "new", "1.0");
	d.PackupNameSpace    = new_PackupNameSpace;
	d.ExportNameSpace   = new_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif