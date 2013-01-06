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

///* ------------------------------------------------------------------------ */
///* new ast parser */
//
//static int ParseTypePattern(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx, KClass **classRef);
//static KClass* ParseGenericsType(KonohaContext *kctx, kNameSpace *ns, KClass *baseClass, kArray *tokenList, int beginIdx, int endIdx)
//{
//	int currentIdx = beginIdx;
//	size_t psize = 0;
//	kparamtype_t *p = ALLOCA(kparamtype_t, endIdx);
//	while(currentIdx < endIdx) {
//		KClass *paramClass = NULL;
//		currentIdx = ParseTypePattern(kctx, ns, tokenList, currentIdx, endIdx, &paramClass);
//		if(paramClass == NULL) {
//			return NULL;
//		}
//		p[psize].attrTypeId = paramClass->typeId;
//		psize++;
//		if(currentIdx < endIdx && tokenList->TokenItems[currentIdx]->resolvedSyntaxInfo->keyword == KSymbol_COMMA) {
//			currentIdx++;
//		}
//	}
//	if(baseClass->baseTypeId == KType_Func) {
//		return KLIB KClass_Generics(kctx, baseClass, p[0].attrTypeId, psize-1, p+1);
//	}
//	else {
//		return KLIB KClass_Generics(kctx, baseClass, KType_void, psize, p);
//	}
//}
//
//static int ParseTypePattern(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx, KClass **classRef)
//{
//	int nextIdx = -1;
//	kToken *tk = tokenList->TokenItems[beginIdx];
//	KClass *foundClass = NULL;
//	if(tk->resolvedSyntaxInfo->keyword == KSymbol_TypePattern) {
//		foundClass = KClass_(tk->resolvedTypeId);
//		nextIdx = beginIdx + 1;
//	}
//	else if(tk->resolvedSyntaxInfo->keyword == KSymbol_SymbolPattern) { // check
//		foundClass = KLIB kNameSpace_GetClassByFullName(kctx, ns, kString_text(tk->text), kString_size(tk->text), NULL);
//		if(foundClass != NULL) {
//			nextIdx = beginIdx + 1;
//		}
//	}
//	if(foundClass != NULL) {
//		int isAllowedGenerics = true;
//		for(; nextIdx < endIdx; nextIdx++) {
//			tk = tokenList->TokenItems[nextIdx];
//			if(tk->resolvedSyntaxInfo == NULL || tk->resolvedSyntaxInfo->keyword != KSymbol_BracketGroup) {
//				break;
//			}
//			int sizeofBracketTokens = kArray_size(tk->GroupTokenList);
//			if(isAllowedGenerics &&  sizeofBracketTokens > 0) {  // C[T][]
//				KClass *foundGenericClass = ParseGenericsType(kctx, ns, foundClass, tk->GroupTokenList, 0, sizeofBracketTokens);
//				if(foundGenericClass == NULL) break;
//				foundClass = foundGenericClass;
//			}
//			else {
//				if(sizeofBracketTokens > 0) break;   // C[100] is treated as C  and the token [100] is set to nextIdx;
//				foundClass = KClass_p0(kctx, KClass_Array, foundClass->typeId);  // C[] => Array[C]
//			}
//			isAllowedGenerics = false;
//		}
//	}
//	if(classRef != NULL) {
//		classRef[0] = foundClass;
//		if(foundClass==NULL) nextIdx = -1;
//	}
//	return nextIdx;
//}

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
static kArray* new_SubsetArray(KonohaContext *kctx, kArray *gcstack, kArray *a, int beginIdx, int endIdx)
{
	kArray *newa = new_(Array, endIdx - beginIdx, gcstack);
	int i;
	for(i = beginIdx; i < endIdx; i++) {
		KLIB kArray_Add(kctx, newa, a->ObjectItems[i]);
	}
	return newa;
}
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
//			kTokenVar *groupToken = new_(TokenVar, tk->resolvedSymbol, gcstack);
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
//		mp[i].symbol = macroTokenList->TokenItems[i]->resolvedSymbol;
//		mp[i].tokenList = groupToken->GroupTokenList;
//	}
//	mp[paramsize].symbol = 0; /* sentinel */
//
//	int p = 0, start = 0;
//	for(i = 0; i < kArray_size(groupToken->GroupTokenList); i++) {
//		kToken *tk = groupToken->GroupTokenList->TokenItems[i];
//		if(tk->resolvedSymbol == KSymbol_COMMA/*tk->hintChar == ','*/) {
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

///* ------------------------------------------------------------------------ */
//
//static int KTokenSeq_AddGroup(KonohaContext *kctx, KTokenSeq *tokens, KMacroSet *macro, KTokenSeq *source, int currentIdx, kToken *openToken)
//{
//	ksymbol_t AST_type = openToken->hintChar == '(' ?  KSymbol_ParenthesisGroup : KSymbol_BracketGroup;
//	int closech = (AST_type == KSymbol_ParenthesisGroup) ? ')': ']';
//	kTokenVar *astToken = new_(TokenVar, AST_type, tokens->tokenList);
//	astToken->resolvedSyntaxInfo = kSyntax_(tokens->ns, AST_type);
//	KFieldSet(astToken, astToken->GroupTokenList, new_(TokenArray, 0, OnField));
//	astToken->uline = openToken->uline;
//	kToken_SetHintChar(astToken, openToken->hintChar, closech);
//	{
//		KTokenSeq nested = {source->ns, astToken->GroupTokenList};
//		kToken *pushOpenToken = source->SourceConfig.openToken;
//		int pushStopChar = source->SourceConfig.stopChar;
//		source->SourceConfig.openToken = openToken;
//		source->SourceConfig.stopChar = closech;
//		nested.TargetPolicy.RemovingIndent = true;
//		int returnIdx = Preprocess(kctx, &nested, macro, source, currentIdx);
//		source->SourceConfig.openToken = pushOpenToken;
//		source->SourceConfig.stopChar = pushStopChar;
//		return returnIdx;
//	}
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
//	//kToken_SetHintChar(tk, 0, ',');
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

//static int Preprocess(KonohaContext *kctx, KTokenSeq *tokens, KMacroSet *macroParam, KTokenSeq *source, int beginIdx)
//{
//	int currentIdx = beginIdx;
//	if(tokens->TargetPolicy.syntaxSymbolPattern == NULL) {
//		tokens->TargetPolicy.syntaxSymbolPattern = kSyntax_(tokens->ns, KSymbol_SymbolPattern);
//	}
//	for(; currentIdx < source->endIdx; currentIdx++) {
//		kTokenVar *tk = source->tokenList->TokenVarItems[currentIdx];
//		/* filter */
//		if(tk->tokenType == TokenType_Indent && tokens->TargetPolicy.RemovingIndent) {
//			continue;  /* filtering indent; */
//		}
//		if(macroParam != NULL && tk->resolvedSyntaxInfo != NULL) {
//			ksymbol_t keyword = tk->resolvedSyntaxInfo->keyword;
//			if(keyword == KSymbol_SymbolPattern && KTokenSeq_ExpandMacro(kctx, tokens, tk->resolvedSymbol, macroParam)) {
//				continue;
//			}
//			if(keyword == KSymbol_ParenthesisGroup || keyword == KSymbol_BracketGroup || keyword == KSymbol_BraceGroup) {
//				tk = kToken_ExpandGroupMacro(kctx, tk, tokens->ns, macroParam, OnGcStack);
//			}
//			if(keyword == KSymbol_BlockPattern) {
//				tk = kToken_ToBraceGroup(kctx, tk, tokens->ns, macroParam);
//			}
//		}
//		if(tk->resolvedSyntaxInfo == NULL) {
//			if(source->SourceConfig.openToken != NULL && source->SourceConfig.stopChar == tk->hintChar) {
//				return currentIdx;
//			}
//			if(tk->hintChar == '(' || tk->hintChar == '[') {
//				currentIdx = KTokenSeq_AddGroup(kctx, tokens, macroParam, source, currentIdx+1, tk);
//				if(source->SourceConfig.foundErrorToken != NULL) return source->endIdx;
//				continue; // already added
//			}
//			if(tk->tokenType == TokenType_Error) {
//				source->SourceConfig.foundErrorToken = tk;
//				return source->endIdx;  // resolved no more
//			}
//			if(tk->tokenType == TokenType_Symbol) {
//				const char *t = kString_text(tk->text);
//				ksymbol_t symbol = KAsciiSymbol(t, kString_size(tk->text), KSymbol_NewId);
//				if(macroParam != NULL && KTokenSeq_ExpandMacro(kctx, tokens, symbol, macroParam)) {
//					continue;
//				}
//				kSyntax *syn = kSyntax_(source->ns, symbol);
//				if(IS_NOTNULL(syn) && FLAG_is(syn->flag, SYNFLAG_Macro)) {
//					if(syn->macroParamSize == 0) {
//						ApplyMacroData(kctx, tokens, syn->macroDataNULL, 0, kArray_size(syn->macroDataNULL), 0, NULL);
//					}
//					else {
//						currentIdx = ApplyMacroDataSyntax(kctx, tokens, syn, macroParam, source, currentIdx);
//					}
//					continue;
//				}
//				tk->resolvedSymbol = symbol;
//				tk->resolvedSyntaxInfo = IS_NOTNULL(syn) ? syn : tokens->TargetPolicy.syntaxSymbolPattern;
//			}
//			else {
//				tk->resolvedSyntaxInfo = kSyntax_(tokens->ns, tk->tokenType);
//				if(tk->tokenType == KSymbol_MemberPattern) {
//					const char *t = kString_text(tk->text);
//					tk->resolvedSymbol = KAsciiSymbol(t, kString_size(tk->text), KSymbol_NewId);
//				}
//			}
//		}
//		DBG_ASSERT(tk->resolvedSyntaxInfo != NULL);
//		KLIB kArray_Add(kctx, tokens->tokenList, tk);
//	}
//	if(source->SourceConfig.openToken != NULL) {
//		char buf[2] = {source->SourceConfig.stopChar, 0};
//		ERROR_UnclosedToken(kctx, (kTokenVar *)source->SourceConfig.openToken, (const char *)buf);
//		source->SourceConfig.foundErrorToken = source->SourceConfig.openToken;
//	}
//	KTokenSeq_End(kctx, tokens);
////	DBG_P(">>>>source");
////	SUGAR dumpTokenArray(kctx, 0, source->tokenList, beginIdx, source->endIdx);
////
////	DBG_P(">>>>tokens");
////	SUGAR dumpTokenArray(kctx, 0, tokens->tokenList, tokens->beginIdx, tokens->endIdx);
//	return source->endIdx;
//}

#define GroupRange(A)  A, 1, (kArray_size(A) - 1)
typedef void (*TraverseFunc)(KonohaContext *kctx, kTokenVar *tk, void *thunk);
typedef void (*Traverse2Func)(KonohaContext *kctx, kTokenVar *tk, kTokenVar *tk2, void *thunk);

static void TraverseTokenList(KonohaContext *kctx, kArray *tokenList, int beginIdx, int endIdx, TraverseFunc f, void *thunk)
{
	int currentIdx;
	for(currentIdx = beginIdx; currentIdx < endIdx; currentIdx++) {
		kTokenVar *tk = tokenList->TokenVarItems[currentIdx];
		f(kctx, tk, thunk);
		if(IS_Array(tk->GroupTokenList)) {
			TraverseTokenList(kctx, GroupRange(tk->GroupTokenList), f, thunk);
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
			TraverseTokenList2(kctx, GroupRange(tk->GroupTokenList), f, thunk);
		}
	}
	if(beginIdx < endIdx - 1) {
		kTokenVar *tk = tokenList->TokenVarItems[endIdx - 1];
		if(IS_Array(tk->GroupTokenList)) {
			TraverseTokenList2(kctx, GroupRange(tk->GroupTokenList), f, thunk);
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
			kToken_Set(OpenGroup, tk, false);
			KLIB kArray_Add(kctx, groupToken->GroupTokenList, tk);
			currentIdx = GroupTokenList(kctx, tk, tokenList, beginIdx + 1, endIdx, groupToken->GroupTokenList);
			continue; // already added
		}
		if(kToken_Is(CloseGroup, tk)) {
			if(openToken != NULL && openToken->tokenType == tk->tokenType) {
				kToken_Set(OpenGroup, tk, false);
				KLIB kArray_Add(kctx, bufferList, tk);
				return currentIdx;
			}
			// ERROR
		}
		KLIB kArray_Add(kctx, bufferList, tk);
	}
	return endIdx;
}

static kArray* kArray_AppendList(KonohaContext *kctx, kArray *a, kArray *tokenList, int beginIdx, int endIdx)
{
	int i;
	for(i = beginIdx; i < endIdx; i++) {
		KLIB kArray_Add(kctx, a, tokenList->ObjectItems[i]);
	}
	return a;
}

static kbool_t ExpandMacroParam(KonohaContext *kctx, kNameSpace *ns, ksymbol_t symbol, KMacroSet *macroParam, kArray *bufferList);
//static void ApplyMacro(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx, size_t paramsize, KMacroSet *macroParam, kArray *bufferList);

static void Preprocess(KonohaContext *kctx, kNameSpace *ns, kArray *tokenList, int beginIdx, int endIdx, KMacroSet *macroParam, kArray *bufferList)
{
	int currentIdx;
	kSyntax *SymbolSyntax = NULL, *TypeSyntax = NULL;
	for(currentIdx = beginIdx; currentIdx < endIdx; currentIdx++) {
		kTokenVar *tk = tokenList->TokenVarItems[currentIdx];
		if(tk->tokenType == TokenType_Skip) continue;
		if(kToken_Is(OpenGroup, tk)) {
			kTokenVar *groupToken = new_(TokenVar, tk->tokenType, OnGcStack);
			KFieldSet(groupToken, groupToken->GroupTokenList, new_(TokenArray, 0, OnField));
			kToken_Set(OpenGroup, tk, false);
			KLIB kArray_Add(kctx, groupToken->GroupTokenList, tk);
			currentIdx = GroupTokenList(kctx, tk, tokenList, beginIdx + 1, endIdx, groupToken->GroupTokenList);
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
			if(groupsize != (pushEndIdx - pushBeginIdx)) {
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
						kSyntax *syntax = (kSyntax*)kvs->ObjectValue;
//						if(kSyntax_Is(Macro, syntax)) {
//							if(syn->macroParamSize == 0) {
//								ApplyMacroData(kctx, tokens, syntax->macroDataNULL, 0, kArray_size(syntax->macroDataNULL), 0, NULL);
//							}
//							else {
//								currentIdx = ApplyMacroDataSyntax(kctx, tokens, syn, macroParam, source, currentIdx);
//							}
//						}
						tk->resolvedSyntaxInfo = syntax;
					}
					if(ty == VirtualType_KClass) {
						if(TypeSyntax == NULL) {
							TypeSyntax = kSyntax_(ns, KSymbol_TypePattern);
						}
						tk->resolvedSyntaxInfo = TypeSyntax;
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
		KLIB kArray_Add(kctx, bufferList, tk);
	}
}

static void ResetPreprocess(KonohaContext *kctx, kTokenVar *tk, void *thunk)
{
	tk->resolvedSyntaxInfo = NULL;
}

static kTokenVar* kToken_ToBraceGroup(KonohaContext *kctx, kTokenVar *tk, kNameSpace *ns, KMacroSet *macroSet)
{
	KTokenSeq source = {ns, KGetParserContext(kctx)->preparedTokenList};
	KTokenSeq_Push(kctx, source);
	//KdumpToken(kctx, tk);
	Tokenize(kctx, ns, kString_text(tk->text), tk->uline, source.tokenList);
	KTokenSeq_End(kctx, source);
	KFieldSet(tk, tk->GroupTokenList, new_(TokenArray, 0, OnField));
//	tk->resolvedSyntaxInfo = kSyntax_(ns, KSymbol_BraceGroup);
	Preprocess(kctx, ns, RangeTokenSeq(source), macroSet, tk->GroupTokenList);
	KTokenSeq_Pop(kctx, source);
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
	Preprocess(kctx, ns, tokenList, beginIdx + paramsize, endIdx, macroParam, bufferList);
}

static kbool_t kNameSpace_SetMacroData(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, int paramsize, const char *data, int optionMacro)
{
	kSyntaxVar *syn = (kSyntaxVar *)SUGAR kNameSpace_GetSyntax(kctx, ns, keyword);
	if(IS_NOTNULL(syn) && syn->macroDataNULL == NULL) {
		KTokenSeq source = {ns, KGetParserContext(kctx)->preparedTokenList};
		KTokenSeq_Push(kctx, source);
		Tokenize(kctx, ns, data, 0, source.tokenList);
		KTokenSeq tokens = {source.ns, source.tokenList, kArray_size(source.tokenList)};
		tokens.TargetPolicy.ExpandingBraceGroup = true;
		Preprocess(kctx, ns, RangeTokenSeq(source), NULL, tokens.tokenList);
		KTokenSeq_End(kctx, tokens);
		syn->macroParamSize = paramsize;
		syn->macroDataNULL =  new_SubsetArray(kctx, ns->NameSpaceConstList, RangeTokenSeq(tokens));
		if(optionMacro) syn->flag |= SYNFLAG_Macro;
		KTokenSeq_Pop(kctx, source);
		return true;
	}
	return false;
}