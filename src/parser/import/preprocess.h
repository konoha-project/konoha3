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

// ---------------------------------------------------------------------------
/* macro */

//static int Preprocess(KonohaContext *kctx, KTokenSeq *tokens, KMacroSet *, KTokenSeq *source, int beginIdx);
//
//static kbool_t KTokenSeq_ExpandMacro(KonohaContext *kctx, KTokenSeq *tokens, ksymbol_t symbol, KMacroSet *macroParam)
//{
//	while(macroParam->symbol != 0) {
//		if(macroParam->symbol == symbol) {
//			KTokenSeq paramtokens = {tokens->ns, macroParam->tokenList, macroParam->beginIdx, macroParam->endIdx};
//			Preprocess(kctx, tokens, NULL, &paramtokens, macroParam->beginIdx);
//			return true;
//		}
//		macroParam++;
//	}
//	return false;
//}
//
//static kArray *new_SubsetArray(KonohaContext *kctx, kArray *gcstack, kArray *a, int beginIdx, int endIdx)
//{
//	kArray *newa = new_(Array, endIdx - beginIdx, gcstack);
//	int i;
//	for(i = beginIdx; i < endIdx; i++) {
//		KLIB kArray_Add(kctx, newa, a->ObjectItems[i]);
//	}
//	return newa;
//}
//
//static kTokenVar* kToken_ExpandGroupMacro(KonohaContext *kctx, kTokenVar *tk, kNameSpace *ns, KMacroSet *macroParam, kArray *gcstack)
//{
//	KTokenSeq source = {ns, tk->GroupTokenList, 0, kArray_size(tk->GroupTokenList)};
//	if(source.endIdx > 0) {
//		int isChanged = true;
//		KTokenSeq group = {ns, KGetParserContext(kctx)->preparedTokenList};
//		KTokenSeq_Push(kctx, group);
//		Preprocess(kctx, &group, macroParam, &source, source.beginIdx);
//		if(group.endIdx - group.beginIdx == source.endIdx) {
//			int i;
//			isChanged = false;
//			for(i = 0; i < source.endIdx; i++) {
//				if(source.tokenList->TokenItems[i] != group.tokenList->TokenItems[group.beginIdx+i]) {
//					isChanged = true;
//				}
//			}
//		}
//		if(isChanged) {
//			kTokenVar *groupToken = new_(TokenVar, tk->symbol, gcstack);
//			KFieldSet(groupToken, groupToken->GroupTokenList, new_SubsetArray(kctx, gcstack, group.tokenList, group.beginIdx, group.endIdx));
//			groupToken->resolvedSyntaxInfo = tk->resolvedSyntaxInfo;
//			groupToken->uline = tk->uline;
//			tk = groupToken;
//		}
//		KTokenSeq_Pop(kctx, group);
//	}
//	return tk;
//}
//
//static kbool_t ApplyMacroData(KonohaContext *kctx, KTokenSeq *tokens, kArray *macroTokenList, int beginIdx, int endIdx, size_t paramsize, KMacroSet *macroParam)
//{
//	KTokenSeq macro = {tokens->ns, macroTokenList, beginIdx + paramsize, endIdx};
//	//SUGAR dumpTokenArray(kctx, 0, macroTokenList, 0, kArray_size(macroTokenList));
//	//DBG_P("macroTokenList=%p", macroTokenList);
//	int dstart = kArray_size(tokens->tokenList);
//	Preprocess(kctx, tokens, macroParam, &macro, beginIdx + paramsize);
//	//DBG_P("<<<<<<<<<<< dstart=%d, tokens->begin,end=%d, %d", dstart, tokens->beginIdx, tokens->endIdx);
//	//KdumpTokenArray(kctx, tokens->tokenList, dstart, tokens->endIdx);
//	return true;
//}
//
//static void ApplyMacroDataGroup(KonohaContext *kctx, KTokenSeq *tokens, kArray *macroTokenList, int paramsize, kToken *groupToken)
//{
//	int i;
//	KMacroSet* mp = ALLOCA(KMacroSet, paramsize+1);
//	DBG_ASSERT(paramsize < kArray_size(macroTokenList));
//	for(i = 0; i < paramsize; i++) {
//		mp[i].symbol = macroTokenList->TokenItems[i]->symbol;
//		mp[i].tokenList = groupToken->GroupTokenList;
//	}
//	mp[paramsize].symbol = 0; /* sentinel */
//
//	int p = 0, start = 0;
//	for(i = 0; i < kArray_size(groupToken->GroupTokenList); i++) {
//		kToken *tk = groupToken->GroupTokenList->TokenItems[i];
//		if(tk->symbol == KSymbol_COMMA/*tk->hintChar == ','*/) {
//			mp[p].beginIdx = start;
//			mp[p].endIdx = i;
//			p++;
//			start = i + 1;
//		}
//	}
//	DBG_ASSERT(p < paramsize);
//	mp[p].beginIdx = start;
//	mp[p].endIdx = kArray_size(groupToken->GroupTokenList);
//	ApplyMacroData(kctx, tokens, macroTokenList, 0, kArray_size(macroTokenList), paramsize, mp);
//}

//static int TokenUtils_Count(kArray *tokenList, int beginIdx, int endIdx, ksymbol_t keyword)
//{
//	int i, count = 0;
//	for(i = beginIdx; i < endIdx; i++) {
//		kToken *tk = tokenList->TokenItems[i];
//		if(tk->resolvedSyntaxInfo->keyword == keyword) count++;
//	}
//	return count;
//}
//
//static kToken* new_CommaToken(KonohaContext *kctx, kArray *gcstack)
//{
//	kTokenVar *tk = new_(TokenVar, KSymbol_COMMA, gcstack);
//	//kToken_SetOpenCloseChar(tk, 0, ',');
//	return tk;
//}
//
//static int ApplyMacroDataSyntax(KonohaContext *kctx, KTokenSeq *tokens, kSyntax *syn, KMacroSet *macroParam, KTokenSeq *source, int currentIdx)
//{
//	KTokenSeq dummy = {tokens->ns, kctx->stack->gcStack};
//	KTokenSeq_Push(kctx, dummy);
//	int nextIdx = TokenUtils_SkipIndent(source->tokenList, currentIdx+1, source->endIdx);
//	kbool_t isApplied = false;
//	if(nextIdx < source->endIdx) {
//		kToken *groupToken = source->tokenList->TokenItems[nextIdx];
//		if(groupToken->hintChar == '(') {
//			nextIdx = KTokenSeq_AddGroup(kctx, &dummy, macroParam, source, nextIdx+1, groupToken);
//			if(source->SourceConfig.foundErrorToken != NULL) {
//				return source->endIdx;
//			}
//			groupToken = dummy.tokenList->TokenItems[dummy.beginIdx];
//		}
//		if(groupToken->resolvedSyntaxInfo != NULL && groupToken->resolvedSyntaxInfo->keyword == KSymbol_ParenthesisGroup) {
//			int count = TokenUtils_Count(groupToken->GroupTokenList, 0, kArray_size(groupToken->GroupTokenList), KSymbol_COMMA);
//			if(syn->macroParamSize == count + 2) {
//				nextIdx = TokenUtils_SkipIndent(source->tokenList, nextIdx+1, source->endIdx);
//				if(nextIdx < source->endIdx) {
//					kTokenVar *tk = source->tokenList->TokenVarItems[nextIdx];
//					if(tk->tokenType == TokenType_LazyBlock) {
//						tk = kToken_ToBraceGroup(kctx, tk, tokens->ns, macroParam);
//						new_CommaToken(kctx, groupToken->GroupTokenList);
//						KLIB kArray_Add(kctx, groupToken->GroupTokenList, tk);
//						count++;
//					}
//				}
//			}
//			if(syn->macroParamSize == count + 1) {
//				ApplyMacroDataGroup(kctx, tokens, syn->macroDataNULL, syn->macroParamSize, groupToken);
//				isApplied = true;
//			}
//		}
//	}
//	if(!isApplied) {
//		kTokenVar *tk = source->tokenList->TokenVarItems[currentIdx];
//		kToken_ToError(kctx, tk, ErrTag, "macro %s%s takes %d parameter(s)", KSymbol_Fmt2(syn->keyword), (int)syn->macroParamSize);
//		source->SourceConfig.foundErrorToken = tk;
//		nextIdx = source->endIdx;
//	}
//	KTokenSeq_Pop(kctx, dummy);
//	return nextIdx;
//}

typedef void (*TraverseFunc)(KonohaContext *kctx, kTokenVar *tk, void *thunk);
typedef void (*Traverse2Func)(KonohaContext *kctx, kTokenVar *tk, kTokenVar *tk2, void *thunk);

static void TraverseTokenList(KonohaContext *kctx, kArray *tokenList, int beginIdx, int endIdx, TraverseFunc f, void *thunk)
{
	int currentIdx;
	for(currentIdx = beginIdx; currentIdx < endIdx; currentIdx++) {
		kTokenVar *tk = tokenList->TokenVarItems[currentIdx];
		f(kctx, tk, thunk);
		if(IS_Array(tk->GroupTokenList)) {
			TraverseTokenList(kctx, RangeGroup(tk->GroupTokenList), f, thunk);
		}
	}
}

static void TraverseTokenList2(KonohaContext *kctx, kArray *tokenList, int beginIdx, int endIdx, Traverse2Func f, void *thunk)
{
	int currentIdx;
	for(currentIdx = beginIdx; currentIdx < endIdx - 1; currentIdx++) {
		kTokenVar *tk = tokenList->TokenVarItems[currentIdx];
		f(kctx, tk, tokenList->TokenVarItems[currentIdx+1], thunk);
		if(IS_Array(tk->GroupTokenList)) {
			TraverseTokenList2(kctx, RangeGroup(tk->GroupTokenList), f, thunk);
		}
	}
	if(beginIdx < endIdx - 1) {
		kTokenVar *tk = tokenList->TokenVarItems[endIdx - 1];
		if(IS_Array(tk->GroupTokenList)) {
			TraverseTokenList2(kctx, RangeGroup(tk->GroupTokenList), f, thunk);
		}
	}
}

static int GroupTokenList(KonohaContext *kctx, kToken *openToken, kArray *tokenList, int beginIdx, int endIdx, kArray *bufferList)
{
	int currentIdx;
	for(currentIdx = beginIdx; currentIdx < endIdx; currentIdx++) {
		kTokenVar *tk = tokenList->TokenVarItems[currentIdx];
		if(kToken_Is(OpenGroup, tk)) {
			kTokenVar *groupToken = new_(TokenVar, tk->tokenType, bufferList);
			KFieldSet(groupToken, groupToken->GroupTokenList, new_(TokenArray, 0, OnField));
			tk->symbol = tk->tokenType;
			groupToken->uline = tk->uline;
			kToken_Set(OpenGroup, tk, false);
			KLIB kArray_Add(kctx, groupToken->GroupTokenList, tk);
			currentIdx = GroupTokenList(kctx, tk, tokenList, currentIdx + 1, endIdx, groupToken->GroupTokenList);
			continue; // already added
		}
		if(kToken_Is(CloseGroup, tk)) {
			if(openToken != NULL && openToken->tokenType == tk->tokenType) {
				kToken_Set(CloseGroup, tk, false);
				KLIB kArray_Add(kctx, bufferList, tk);
				return currentIdx;
			}
			// ERROR
		}
		KLIB kArray_Add(kctx, bufferList, tk);
	}
	return endIdx;
}

static kArray *kArray_AppendList(KonohaContext *kctx, kArray *a, kArray *tokenList, int beginIdx, int endIdx)
{
	int i;
	for(i = beginIdx; i < endIdx; i++) {
		KLIB kArray_Add(kctx, a, tokenList->ObjectItems[i]);
	}
	return a;
}

static int ReplaceToken(KonohaContext *kctx, kFunc *fo, kNameSpace *ns, kArray *tokenList, int beginIdx, int opIdx, int endIdx, kArray *bufferList)
{
	DBG_ASSERT(IS_Func(fo));
	BEGIN_UnusedStack(lsfp);
	KUnsafeFieldSet(lsfp[1].asNameSpace, ns);
	KUnsafeFieldSet(lsfp[2].asArray, tokenList);
	lsfp[3].unboxValue = (uintptr_t)beginIdx;
	lsfp[4].unboxValue = (uintptr_t)beginIdx;
	lsfp[5].unboxValue = (uintptr_t)beginIdx;
	KUnsafeFieldSet(lsfp[6].asArray, bufferList);
	CallSugarMethod(kctx, lsfp, fo, 6, KLIB Knull(kctx, KClass_Int));
	END_UnusedStack();
	return((int)lsfp[K_RTNIDX].intValue);
}

static kbool_t ExpandMacroParam(KonohaContext *kctx, kNameSpace *ns, ksymbol_t symbol, KMacroSet *macroParam, kArray *bufferList);
//static void ApplyMacro(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx, size_t paramsize, KMacroSet *macroParam, kArray *bufferList);
//static kTokenVar* kToken_Expand(KonohaContext *kctx, kTokenVar *tk, kNameSpace *ns, KMacroSet *macroSet, kArray *bufferList);

static void Preprocess(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx, KMacroSet *macroParam, kArray *bufferList)
{
	DBG_ASSERT(beginIdx < endIdx);
	int currentIdx;
	kSyntax *SymbolSyntax = NULL, *TypeSyntax = NULL;
	for(currentIdx = beginIdx; currentIdx < endIdx; currentIdx++) {
		kTokenVar *tk = tokenList->TokenVarItems[currentIdx];
		if(tk->tokenType == TokenType_Skip) continue;
//		if(tk->tokenType == TokenType_Indent) {
//			if(kString_size(tk->text) > 0) {
//				kToken_Expand(kctx, tk, ns, macroParam, bufferList);
//			}
//			continue;
//		}
		if(kToken_Is(OpenGroup, tk)) {
			kTokenVar *groupToken = new_(TokenVar, tk->tokenType, OnGcStack);
			KFieldSet(groupToken, groupToken->GroupTokenList, new_(TokenArray, 0, OnField));
			kToken_Set(OpenGroup, tk, false);
			tk->symbol = tk->tokenType;
			groupToken->uline = tk->uline;
			KLIB kArray_Add(kctx, groupToken->GroupTokenList, tk);
			currentIdx = GroupTokenList(kctx, tk, tokenList, currentIdx + 1, endIdx, groupToken->GroupTokenList);
			tk = groupToken;
		}
		//		if(tk->tokenType == TokenType_LazyBlock && (tokens->TargetPolicy.ExpandingBraceGroup || macroParam != NULL)) {
		//			tk = kToken_ToBraceGroup(kctx, tk, tokens->ns, macroParam);
		//			DBG_ASSERT(tk->resolvedSyntaxInfo != NULL);
		//			KLIB kArray_Add(kctx, tokens->tokenList, tk);
		//			continue;
		//		}
		if(IS_Array(tk->GroupTokenList)) {
			size_t pushBeginIdx = kArray_size(bufferList), groupsize = kArray_size(tk->GroupTokenList);
			Preprocess(kctx, ns, tk->GroupTokenList, 0, groupsize, macroParam, bufferList);
			size_t pushEndIdx = kArray_size(bufferList);
			if(groupsize != (pushEndIdx - pushBeginIdx) || macroParam != NULL) {
				KLIB kArray_Clear(kctx, tk->GroupTokenList, 0);
				kArray_AppendList(kctx, tk->GroupTokenList, bufferList, pushBeginIdx, pushEndIdx);
			}
			KLIB kArray_Clear(kctx, bufferList, pushBeginIdx);
		}
		if(tk->resolvedSyntaxInfo == NULL) {
			if(tk->tokenType == TokenType_Symbol) {
				if(macroParam != NULL && ExpandMacroParam(kctx, ns, tk->symbol, macroParam, bufferList)) {
					continue;
				}
				KKeyValue *kvs = kNameSpace_GetConstNULL(kctx, ns, tk->symbol, false/*isLocalOnly*/);
				if(kvs != NULL) {
					ktypeattr_t ty = KTypeAttr_Unmask(kvs->attrTypeId);
					if(ty == KType_Syntax) {
						kSyntax *syntax = (kSyntax *)kvs->ObjectValue;
						tk->resolvedSyntaxInfo = syntax;
					}
					if(ty == VirtualType_KClass) {
						if(TypeSyntax == NULL) {
							TypeSyntax = kSyntax_(ns, KSymbol_TypePattern);
						}
						tk->resolvedSyntaxInfo = TypeSyntax;
						tk->resolvedTypeId = ((KClass *)kvs->unboxValue)->typeId;
					}
				}
				if(tk->resolvedSyntaxInfo == NULL) {
					if(SymbolSyntax == NULL) {
						SymbolSyntax = kSyntax_(ns, KSymbol_SymbolPattern);
					}
					tk->resolvedSyntaxInfo = SymbolSyntax;
				}
			}
			else {
				tk->resolvedSyntaxInfo = kSyntax_(ns, tk->tokenType);
			}
		}
		DBG_ASSERT(tk->resolvedSyntaxInfo != NULL);
		if(tk->resolvedSyntaxInfo->ReplaceFuncNULL != NULL) {
			int replacedIdx = ReplaceToken(kctx, tk->resolvedSyntaxInfo->ReplaceFuncNULL, ns, tokenList, beginIdx, currentIdx, endIdx, bufferList);
			if(replacedIdx != -1) {
				currentIdx = replacedIdx;
				continue;
			}
		}
		KLIB kArray_Add(kctx, bufferList, tk);
	}
}

static void ResetPreprocess(KonohaContext *kctx, kTokenVar *tk, void *thunk)
{
	tk->resolvedSyntaxInfo = NULL;
}

//static kTokenVar* kToken_Expand(KonohaContext *kctx, kTokenVar *tk, kNameSpace *ns, KMacroSet *macroSet, kArray *bufferList)
//{
//	DBG_ASSERT(IS_String(tk->text));
//	KTokenSeq source = {ns, KGetParserContext(kctx)->preparedTokenList};
//	KTokenSeq_Push(kctx, source);
//	Tokenize(kctx, ns, kString_text(tk->text), tk->uline, tk->indent, source.tokenList);
//	KTokenSeq_End(kctx, source);
//	Preprocess(kctx, ns, RangeTokenSeq(source), macroSet, bufferList);
//	KTokenSeq_Pop(kctx, source);
//	return tk;
//}

static kTokenVar* kToken_ToBraceGroup(KonohaContext *kctx, kTokenVar *tk, kNameSpace *ns, KMacroSet *macroSet)
{
	if(!IS_Array(tk->GroupTokenList)) {
		DBG_ASSERT(IS_String(tk->text));
		KTokenSeq source = {ns, KGetParserContext(kctx)->preparedTokenList};
		KTokenSeq_Push(kctx, source);
		//KdumpToken(kctx, tk);
		Tokenize(kctx, ns, kString_text(tk->text), tk->uline, tk->indent, source.tokenList);
		KTokenSeq_End(kctx, source);
		KFieldSet(tk, tk->GroupTokenList, new_(TokenArray, 0, OnField));
		tk->resolvedSyntaxInfo = kSyntax_(ns, KSymbol_BraceGroup);
		KLIB kArray_Add(kctx, tk->GroupTokenList, K_NULLTOKEN);  // K_NULLTOKEN should be harmless
		Preprocess(kctx, ns, RangeTokenSeq(source), macroSet, tk->GroupTokenList);
		KLIB kArray_Add(kctx, tk->GroupTokenList, K_NULLTOKEN);
		KTokenSeq_Pop(kctx, source);
	}
	return tk;
}

static kbool_t ExpandMacroParam(KonohaContext *kctx, kNameSpace *ns, ksymbol_t symbol, KMacroSet *macroParam, kArray *bufferList)
{
	while(macroParam->symbol != 0) {
		if(macroParam->symbol == symbol) {
			Preprocess(kctx, ns, macroParam->tokenList, macroParam->beginIdx, macroParam->endIdx, NULL, bufferList);
			return true;
		}
		macroParam++;
	}
	return false;
}

static void ApplyMacroData(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx, size_t paramsize, KMacroSet *macroParam, kArray *bufferList)
{
	TraverseTokenList(kctx, tokenList, beginIdx + paramsize, endIdx, ResetPreprocess, NULL);
	//SUGAR dumpTokenArray(kctx, 0, tokenList, beginIdx + paramsize, endIdx);
	Preprocess(kctx, ns, tokenList, beginIdx + paramsize, endIdx, macroParam, bufferList);
	//SUGAR dumpTokenArray(kctx, 0, RangeArray(bufferList));
}

static kbool_t SetMacroData(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, int paramsize, const char *data, int optionMacro)
{
	kSyntaxVar *syn = (kSyntaxVar *)SUGAR kNameSpace_GetSyntax(kctx, ns, keyword);
	if(IS_NOTNULL(syn) && syn->macroDataNULL == NULL) {
		KTokenSeq source = {ns, KGetParserContext(kctx)->preparedTokenList};
		KTokenSeq_Push(kctx, source);
		Tokenize(kctx, ns, data, 0, 0, source.tokenList);
		KTokenSeq_End(kctx, source);
		KTokenSeq tokens = {source.ns, source.tokenList, kArray_size(source.tokenList)};
		tokens.TargetPolicy.ExpandingBraceGroup = true;
		Preprocess(kctx, ns, RangeTokenSeq(source), NULL, tokens.tokenList);
		KTokenSeq_End(kctx, tokens);
		syn->macroParamSize = paramsize;
		syn->macroDataNULL =  kArray_AppendList(kctx, new_(Array, 0, ns->NameSpaceConstList), RangeTokenSeq(tokens));
		if(optionMacro) syn->flag |= SYNFLAG_Macro;
		KTokenSeq_Pop(kctx, source);
		return true;
	}
	return false;
}
