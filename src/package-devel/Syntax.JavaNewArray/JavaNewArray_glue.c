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

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>

#ifdef __cplusplus
extern "C"{
#endif

// --------------------------------------------------------------------------

static KMETHOD Expression_new(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(stmt, tokenList, beginIdx, currentIdx, endIdx);
	if(beginIdx + 1 < endIdx) {
		kTokenVar *newToken = tokenList->TokenVarItems[beginIdx];
		KClass *foundClass = NULL;
		kNameSpace *ns = kNode_ns(stmt);
		int nextIdx = SUGAR ParseTypePattern(kctx, ns, tokenList, beginIdx + 1, endIdx, &foundClass);
		if((size_t)nextIdx < kArray_size(tokenList)) {
			kToken *nextTokenAfterClassName = tokenList->TokenItems[nextIdx];
			if(nextTokenAfterClassName->resolvedSyntaxInfo->keyword == KSymbol_BracketGroup) { // new int [100]
				kArray *GroupTokenList = nextTokenAfterClassName->GroupTokenList;
				/* new Type1[] => Array<Type1>.newList */
				KClass *arrayClass = KClass_p0(kctx, KClass_Array, foundClass->typeId);
				newToken->symbol = KSymbol_("newArray");
				kNode *arg0 = new_ConstNode(kctx, ns, NULL, KLIB Knull(kctx, arrayClass));
				SUGAR kNode_Op(kctx, stmt, newToken, 1, arg0);
				SUGAR AppendParsedNode(kctx, stmt, RangeGroup(GroupTokenList), NULL, ParseExpressionOption, NULL);
				KReturnUnboxValue(nextIdx+1);
			}
		}
	}
	KReturnUnboxValue(-1);
}

static kbool_t new_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("new"), SYNFLAG_Suffix|SYNFLAG_CParseFunc, Precedence_CStyleSuffixCall, 0, {SUGARFUNC Expression_new}, {SUGAR methodTypeFunc}},
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

KDEFINE_PACKAGE *JavaNewArray_Init(void)
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
