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
// Utils

static void ArrayNULL_append(KonohaContext *kctx, kObject *p, kArray **arrayRef, kObject *o)
{
	if(arrayRef[0] == NULL) {
		KSETv(p, arrayRef[0], new_(Array, 0));
	}
	KLIB kArray_add(kctx, arrayRef[0], o);
}

static void ArrayNULL_appendArray(KonohaContext *kctx, kObject *p, kArray **arrayRef, kArray *a)
{
	int i;
	if(a != NULL) {
		if(arrayRef[0] == NULL) {
			KSETv(p, arrayRef[0], new_(Array, kArray_size(a)));
		}
		for(i = 0; i < kArray_size(a); i++) {
			kObject *o = a->objectItems[i];
			KLIB kArray_add(kctx, arrayRef[0], o);
		}
	}
}

static void kNameSpace_addFuncList(KonohaContext *kctx, kNameSpace *ns, kArray **funcListTable, int index, kFunc *fo)
{
	kArray *a = funcListTable[index];
	if(a == NULL) {
		KSETv(ns, funcListTable[index], (kArray*)fo);
		return;
	}
	else if(!IS_Array(a)) {
		kArray *newa = new_(Array, 0);
		KLIB kArray_add(kctx, newa, a);
		KSETv(ns, funcListTable[index], newa);
		a = newa;
	}
	KLIB kArray_add(kctx, a, fo);
}

// ---------------------------------------------------------------------------
/* TokenFunc Management */

static kFunc **kNameSpace_tokenFuncMatrix(KonohaContext *kctx, kNameSpace *ns);

static void kNameSpace_setTokenFuncMatrix(KonohaContext *kctx, kNameSpace *ns, int konohaChar, kFunc *fo)
{
	kArray **list = (kArray**)kNameSpace_tokenFuncMatrix(kctx, ns);
	kNameSpace_addFuncList(kctx, ns, list, konohaChar, fo);
}

// ---------------------------------------------------------------------------
// Syntax Management

#define kToken_isFirstPattern(tk)   (KW_isPATTERN(tk->resolvedSymbol) && tk->stmtEntryKey != KW_ExprPattern)
static void kNameSpace_parseSyntaxPattern(KonohaContext *kctx, kNameSpace *ns, const char *rule, kfileline_t uline, kArray *ruleList);

static SugarSyntax* kNameSpace_newSyntax(KonohaContext *kctx, kNameSpace *ns, SugarSyntax *parentSyntax, ksymbol_t keyword)
{
	if(ns->syntaxMapNN == NULL) {
		((kNameSpaceVar*)ns)->syntaxMapNN = KLIB Kmap_init(kctx, 0);
	}
	KUtilsHashMapEntry *e = KLIB Kmap_newEntry(kctx, ns->syntaxMapNN, (uintptr_t)keyword);
	SugarSyntaxVar *syn = (SugarSyntaxVar*)KCALLOC(sizeof(SugarSyntax), 1);
	e->unboxValue = (uintptr_t)syn;
	syn->parentSyntaxNULL = parentSyntax;
	syn->keyword          = keyword;
	if(parentSyntax != NULL) {
		syn->precedence_op1 = parentSyntax->precedence_op1;
		syn->precedence_op2 = parentSyntax->precedence_op2;
	}
	else {
		syn->precedence_op1 = 0;
		syn->precedence_op2 = 0;
	}
	return syn;
}

static kFunc** SugarSyntax_funcTable(KonohaContext *kctx, SugarSyntax *syn, int index, int *sizeRef)
{
	kFunc *fo = syn->sugarFuncTable[index];
	if(fo == NULL) {
		sizeRef[0] = 0;
		return NULL;
	}
	else if(IS_Array(fo)) {
		sizeRef[0] =  kArray_size(syn->sugarFuncListTable[index]);
		return syn->sugarFuncListTable[index]->funcItems;
	}
	sizeRef[0] = 1;
	return (kFunc**)&(syn->sugarFuncTable[index]);
}

static kbool_t kNameSpace_importSyntax(KonohaContext *kctx, kNameSpace *ns, SugarSyntax *target, kfileline_t pline)
{
	SugarSyntaxVar *syn = (SugarSyntaxVar*)kNameSpace_getSyntax(kctx, ns, target->keyword, true/*isNew*/);
	if(syn->lastLoadedPackageId != target->lastLoadedPackageId) {
		int index;
		KLIB Kreportf(kctx, DebugTag, 0, "@%s importing syntax %s%s", PackageId_t(ns->packageId), PSYM_t(syn->keyword));
		syn->flag = target->flag;
		syn->precedence_op1 = target->precedence_op1;
		syn->precedence_op2 = target->precedence_op2;
		syn->macroParamSize = target->macroParamSize;
		ArrayNULL_appendArray(kctx, UPCAST(ns), &syn->macroDataNULL, target->macroDataNULL);
		ArrayNULL_appendArray(kctx, UPCAST(ns), &syn->SyntaxPatternListNULL, target->SyntaxPatternListNULL);
		if(syn->SyntaxPatternListNULL != NULL && SAFECHECK(0 < kArray_size(syn->SyntaxPatternListNULL))) {
			kToken *patternToken = syn->SyntaxPatternListNULL->tokenItems[0];
			if(kToken_isFirstPattern(patternToken)) {
				ArrayNULL_append(kctx, UPCAST(ns), &((kNameSpaceVar*)ns)->StmtPatternListNULL, UPCAST(patternToken));
			}
		}
		for(index = 0; index < SugarFunc_SIZE; index++) {
			int j, size;
			kFunc **funcItems = SugarSyntax_funcTable(kctx, target, index, &size);
			for(j = 0; j < size; j++) {
				kNameSpace_addFuncList(kctx, ns, syn->sugarFuncListTable, index, funcItems[j]);
			}
		}
		if(target->tokenKonohaChar != 0) {
			int j, size;
			kFunc **funcItems = SugarSyntax_funcTable(kctx, target, SugarFunc_TokenFunc, &size);
			for(j = 0; j < size; j++) {
				kNameSpace_setTokenFuncMatrix(kctx, ns, target->tokenKonohaChar, funcItems[j]);
			}
		}
		syn->lastLoadedPackageId = target->lastLoadedPackageId;
	}
	return true;
}

struct ImportSyntaxArgument {
	kNameSpace *ns;
	kfileline_t pline;
};

static void importEachSyntax(KonohaContext *kctx, KUtilsHashMapEntry *e, void *thunk)
{
	struct ImportSyntaxArgument *argd = (struct ImportSyntaxArgument*)thunk;
	kNameSpace_importSyntax(kctx, argd->ns, (SugarSyntax*)e->unboxValue, argd->pline);
}

static kbool_t kNameSpace_importSyntaxAll(KonohaContext *kctx, kNameSpace *ns, kNameSpace *targetNS, kfileline_t pline)
{
	if(targetNS->syntaxMapNN != NULL) {
		struct ImportSyntaxArgument argumentData = { ns, pline };
		KLIB Kmap_each(kctx, targetNS->syntaxMapNN, &argumentData, importEachSyntax);
	}
	return true;
}

static SugarSyntax* kNameSpace_getSyntax(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, int isNew)
{
	kNameSpace *currentNameSpace = ns;
	uintptr_t hcode = keyword;
	while(currentNameSpace != NULL) {
		if(currentNameSpace->syntaxMapNN != NULL) {
			KUtilsHashMapEntry *e = KLIB Kmap_get(kctx, currentNameSpace->syntaxMapNN, hcode);
			while(e != NULL) {
				if(e->hcode == hcode) {
					if(isNew && ns != currentNameSpace) {
						return kNameSpace_newSyntax(kctx, ns, (SugarSyntax*)e->unboxValue, keyword);
					}
					return (SugarSyntax*)e->unboxValue;
				}
				e = e->next;
			}
		}
		currentNameSpace = currentNameSpace->parentNULL;
	}
	return (isNew) ? kNameSpace_newSyntax(kctx, ns, NULL, keyword) : NULL;
}

static SugarSyntaxVar *kNameSpace_setSugarFunc(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, size_t idx, kFunc *fo)
{
	assert(idx < SugarFunc_SIZE);
	SugarSyntaxVar *syn = (SugarSyntaxVar *)kNameSpace_getSyntax(kctx, ns, keyword, 1/*new*/);
	KINITSETv(ns, syn->sugarFuncTable[idx], fo);
	return syn;
}

static SugarSyntaxVar *kNameSpace_addSugarFunc(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, size_t idx, kFunc *funcObject)
{
	SugarSyntaxVar *syn = (SugarSyntaxVar *)kNameSpace_getSyntax(kctx, ns, keyword, 1/*new*/);
	DBG_ASSERT(idx < SugarFunc_SIZE);
	kNameSpace_addFuncList(kctx, ns, syn->sugarFuncListTable, idx, funcObject);
	return syn;
}

static SugarSyntaxVar *kNameSpace_setTokenFunc(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, int konohaChar, kFunc *fo)
{
	SugarSyntaxVar *syn = (SugarSyntaxVar *)kNameSpace_getSyntax(kctx, ns, keyword, 1/*new*/);
	syn->tokenKonohaChar = konohaChar;
	KINITSETv(ns, syn->sugarFuncTable[SugarFunc_TokenFunc], fo);
	kArray **list = (kArray**)kNameSpace_tokenFuncMatrix(kctx, ns);
	kNameSpace_addFuncList(kctx, ns, list, konohaChar, fo);
	return syn;
}

static void SugarSyntax_setMethodFunc(KonohaContext *kctx, SugarSyntaxVar *syn, MethodFunc definedMethodFunc, size_t index, MethodFunc *previousDefinedFuncRef, kFunc **cachedFuncRef)
{
	if(definedMethodFunc != NULL) {
		if(definedMethodFunc != previousDefinedFuncRef[0]) {
			previousDefinedFuncRef[0] = definedMethodFunc;
			cachedFuncRef[0] = new_SugarFunc(definedMethodFunc);
		}
		KINITv(syn->sugarFuncTable[index], cachedFuncRef[0]);
	}
}

static void kNameSpace_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KDEFINE_SYNTAX *syndef, kNameSpace *packageNameSpace)
{
	MethodFunc pPatternMatch = NULL, pExpression = NULL, pStatement = NULL, pTypeCheck = NULL;
	kFunc *mPatternMatch = NULL, *mExpression = NULL, *mStatement = NULL, *mTypeCheck = NULL;
	while(syndef->keyword != KW_END) {
		SugarSyntaxVar* syn = (SugarSyntaxVar*)kNameSpace_getSyntax(kctx, ns, syndef->keyword, 1/*isnew*/);
		DBG_ASSERT(syn != NULL);
		syn->lastLoadedPackageId = packageNameSpace->packageId;
		syn->flag  |= ((kshortflag_t)syndef->flag);
		if(syndef->precedence_op1 > 0) {
			syn->precedence_op1 = syndef->precedence_op1;
		}
		if(syndef->precedence_op2 > 0) {
			syn->precedence_op2 = syndef->precedence_op2;
		}
		if(syndef->rule != NULL) {
			KINITp(ns, syn->SyntaxPatternListNULL, new_(TokenArray, 0));
			kNameSpace_parseSyntaxPattern(kctx, ns, syndef->rule, 0, syn->SyntaxPatternListNULL);
		}
		SugarSyntax_setMethodFunc(kctx, syn, syndef->PatternMatch,   SugarFunc_PatternMatch,   &pPatternMatch, &mPatternMatch);
		SugarSyntax_setMethodFunc(kctx, syn, syndef->Expression,      SugarFunc_Expression,      &pExpression, &mExpression);
		SugarSyntax_setMethodFunc(kctx, syn, syndef->TopLevelStatement, SugarFunc_TopLevelStatement, &pStatement, &mStatement);
		SugarSyntax_setMethodFunc(kctx, syn, syndef->Statement,    SugarFunc_Statement,    &pStatement, &mStatement);
		SugarSyntax_setMethodFunc(kctx, syn, syndef->TypeCheck,    SugarFunc_TypeCheck,    &pTypeCheck, &mTypeCheck);
		// set default function
		if(syn->parentSyntaxNULL == NULL && syn->sugarFuncTable[SugarFunc_Expression] == NULL) {
			if(syn->precedence_op2 > 0 || syn->precedence_op1 > 0) {
				kFunc *fo = SYN_(ns, KW_ExprOperator)->sugarFuncTable[SugarFunc_Expression];
				DBG_ASSERT(fo != NULL);
				KINITp(ns, syn->sugarFuncTable[SugarFunc_Expression], fo);
			}
			else if(syn->sugarFuncTable[SugarFunc_TypeCheck] != NULL) {
				kFunc *fo = SYN_(ns, KW_ExprTerm)->sugarFuncTable[SugarFunc_Expression];
				DBG_ASSERT(fo != NULL);
				KINITp(ns, syn->sugarFuncTable[SugarFunc_Expression], fo);
			}
		}
		DBG_ASSERT(syn == SYN_(ns, syndef->keyword));
		KLIB Kreportf(kctx, DebugTag, 0, "@%s new syntax %s%s", PackageId_t(packageNameSpace->packageId), PSYM_t(syn->keyword));
		syndef++;
	}
}

// --------------------------------------------------------------------------
/* ConstTable */

static int comprKeyVal(const void *a, const void *b)
{
	int akey = SYMKEY_unbox(((KUtilsKeyValue*)a)->key);
	int bkey = SYMKEY_unbox(((KUtilsKeyValue*)b)->key);
	return akey - bkey;
}

static KUtilsKeyValue* kNameSpace_getLocalConstNULL(KonohaContext *kctx, kNameSpace *ns, ksymbol_t unboxKey)
{
	size_t min = 0, max = ns->sortedConstTable, size = kNameSpace_sizeConstTable(ns);
	while(min < max) {
		size_t p = (max + min) / 2;
		ksymbol_t key = SYMKEY_unbox(ns->constTable.keyValueItems[p].key);
		if(key == unboxKey) return ns->constTable.keyValueItems + p;
		if((int)key < (int)unboxKey) {
			min = p + 1;
		}
		else {
			max = p;
		}
	}
	for(min = ns->sortedConstTable; min < size; min++) {
		ksymbol_t key = SYMKEY_unbox(ns->constTable.keyValueItems[min].key);
		if(key == unboxKey) return ns->constTable.keyValueItems + min;
	}
	return NULL;
}

static KUtilsKeyValue* kNameSpace_getConstNULL(KonohaContext *kctx, kNameSpace *ns, ksymbol_t unboxKey)
{
	while(ns != NULL) {
		KUtilsKeyValue* foundKeyValue = kNameSpace_getLocalConstNULL(kctx, ns, unboxKey);
		if(foundKeyValue != NULL) return foundKeyValue;
		ns = ns->parentNULL;
	}
	return NULL;
}

static kbool_t kNameSpace_mergeConstData(KonohaContext *kctx, kNameSpaceVar *ns, KUtilsKeyValue *kvs, size_t nitems, kfileline_t pline)
{
	size_t i, size = kNameSpace_sizeConstTable(ns);
	if(size == 0) {
		KLIB Karray_init(kctx, &ns->constTable, (nitems + 8) * sizeof(KUtilsKeyValue));
		memcpy(ns->constTable.keyValueItems, kvs, nitems * sizeof(KUtilsKeyValue));
//		for(i = 0; i < nitems; i++) {
//			ksymbol_t unboxKey = kvs[i].key;
//			KLIB Kreportf(kctx, DebugTag, 0, "@%s loading const %s%s as %s", PackageId_t(ns->packageId), PSYM_t(SYMKEY_unbox(unboxKey)), TY_t(kvs[i].ty));
//		}
	}
	else {
		KUtilsWriteBuffer wb;
		KLIB Kwb_init(&(KonohaContext_getSugarContext(kctx)->errorMessageBuffer), &wb);
		for(i = 0; i < nitems; i++) {
			ksymbol_t unboxKey = kvs[i].key;
			KLIB Kreportf(kctx, DebugTag, pline, "loading const %s%s as %s", PSYM_t(SYMKEY_unbox(unboxKey)), TY_t(kvs[i].ty));
			KUtilsKeyValue* stored = kNameSpace_getLocalConstNULL(kctx, ns, unboxKey);
			if(stored != NULL) {
				if(kvs[i].ty == stored->ty && kvs[i].unboxValue == stored->unboxValue) {
					continue;  // same value
				}
				SugarContext_printMessage(kctx, ErrTag, pline, "already defined symbol: %s%s", PSYM_t(SYMKEY_unbox(unboxKey)));
				KLIB Kwb_free(&wb);
				return false;
			}
			KLIB Kwb_write(kctx, &wb, (const char*)(kvs+i), sizeof(KUtilsKeyValue));
		}
		kvs = (KUtilsKeyValue*)KLIB Kwb_top(kctx, &wb, 0);
		nitems = Kwb_bytesize(&wb)/sizeof(KUtilsKeyValue);
		if(nitems > 0) {
			if(!((size + nitems) * sizeof(KUtilsKeyValue) < ns->constTable.bytemax)) {
				KLIB Karray_resize(kctx, &ns->constTable, (size + nitems + 8) * sizeof(KUtilsKeyValue));
			}
			memcpy(ns->constTable.keyValueItems + size, kvs, nitems * sizeof(KUtilsKeyValue));
		}
		KLIB Kwb_free(&wb);
	}
	nitems = size + nitems;
	ns->constTable.bytesize = nitems * sizeof(KUtilsKeyValue);
	if(nitems - ns->sortedConstTable > 9) {
		PLATAPI qsort_i(ns->constTable.keyValueItems, nitems, sizeof(KUtilsKeyValue), comprKeyVal);
		ns->sortedConstTable = nitems;
	}
	return true;
}

static kbool_t kNameSpace_setConstData(KonohaContext *kctx, kNameSpace *ns, ksymbol_t key, ktype_t ty, uintptr_t unboxValue, kfileline_t pline)
{
	KUtilsKeyValue kv;
	kv.ty = ty;
	kv.unboxValue = unboxValue;
	if(ty == TY_TEXT) {
		const char *textData = (const char*)unboxValue;
		kv.ty = TY_String;
		kv.stringValue = KLIB new_kString(kctx, textData, strlen(textData), StringPolicy_TEXT);
		PUSH_GCSTACK(kv.objectValue);
	}
	if(TY_isUnbox(kv.ty) || kv.ty == TY_TYPE) {
		kv.key = key;
	}
	else {
		kv.key = key | SYMKEY_BOXED;
	}
	return kNameSpace_mergeConstData(kctx, (kNameSpaceVar*)ns, &kv, 1, pline);
}

static kbool_t kNameSpace_loadConstData(KonohaContext *kctx, kNameSpace *ns, const char **d, kfileline_t pline)
{
	INIT_GCSTACK();
	KUtilsKeyValue kv;
	KUtilsWriteBuffer wb;
	kbool_t result = true;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	while(d[0] != NULL) {
		kv.key = ksymbolSPOL(d[0], strlen(d[0]), StringPolicy_TEXT|StringPolicy_ASCII, _NEWID) | SYMKEY_BOXED;
		kv.ty  = (ktype_t)(uintptr_t)d[1];
		if(kv.ty == TY_TEXT) {
			kv.ty = TY_String;
			kv.stringValue = KLIB new_kString(kctx, d[2], strlen(d[2]), StringPolicy_TEXT);
			PUSH_GCSTACK(kv.objectValue);
		}
		else if(TY_isUnbox(kv.ty) || kv.ty == TY_TYPE) {
			kv.key = SYMKEY_unbox(kv.key);
			kv.unboxValue = (uintptr_t)d[2];
		}
		else {
			kv.objectValue = (kObject*)d[2];
		}
		KLIB Kwb_write(kctx, &wb, (const char*)(&kv), sizeof(KUtilsKeyValue));
		d += 3;
	}
	size_t nitems = Kwb_bytesize(&wb) / sizeof(KUtilsKeyValue);
	if(nitems > 0) {
		result = kNameSpace_mergeConstData(kctx, (kNameSpaceVar*)ns, (KUtilsKeyValue*)KLIB Kwb_top(kctx, &wb, 0), nitems, pline);
	}
	KLIB Kwb_free(&wb);
	RESET_GCSTACK();
	return result;
}

//static kbool_t kNameSpace_importConstData(KonohaContext *kctx, kNameSpace *ns, kNameSpace *targetNS, void *thunk, kbool_t (*match)(KonohaContext *kctx, KUtilsKeyValue *, void *thunk), kfileline_t pline)
//{
//	KUtilsWriteBuffer wb;
//	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
//	size_t i, size = kNameSpace_sizeConstTable(targetNS);
//	kbool_t result = true;
//	for(i = 0; i < size; i++) {
//		KUtilsKeyValue *kvs = targetNS->constTable.keyValueItems + i;
//		if(match(kctx, kvs, thunk)) {
//			KLIB Kwb_write(kctx, &wb, (const char*)(kvs), sizeof(KUtilsKeyValue));
//		}
//	}
//	size_t nitems = Kwb_bytesize(&wb) / sizeof(KUtilsKeyValue);
//	if(nitems > 0) {
//		result = kNameSpace_mergeConstData(kctx, (kNameSpaceVar*)ns, (KUtilsKeyValue*)KLIB Kwb_top(kctx, &wb, 0), nitems, pline);
//	}
//	KLIB Kwb_free(&wb);
//	return result;
//}

// ---------------------------------------------------------------------------
/* ClassName in ConstTable */

static KonohaClass *kNameSpace_getClass(KonohaContext *kctx, kNameSpace *ns, const char *name, size_t len, KonohaClass *defaultClass)
{
	KonohaClass *ct = NULL;
	kpackage_t packageId = PN_konoha;
	ksymbol_t  un = SYM_NONAME;
	const char *p = strrchr(name, '.');
	if(p == NULL) {
		un = ksymbolA(name, len, SYM_NONAME);
	}
	else {
		size_t plen = p - name;
		un = ksymbolA(name + (plen+1), len - (plen+1), SYM_NONAME);
		packageId = KLIB KpackageId(kctx, name, plen, 0, SYM_NONAME);
	}
	if(packageId != SYM_NONAME) {
		KUtilsKeyValue *kvs = kNameSpace_getConstNULL(kctx, ns, un);
		if(kvs != NULL && kvs->ty == TY_TYPE) {
			return (KonohaClass*)kvs->unboxValue;
		}
	}
	return (ct != NULL) ? ct : defaultClass;
}

static KonohaClass *kNameSpace_defineClass(KonohaContext *kctx, kNameSpace *ns, kString *name, KDEFINE_CLASS *cdef, kfileline_t pline)
{
	KonohaClass *ct = KLIB KonohaClass_define(kctx, ns->packageId, name, cdef, pline);
	if(!KLIB kNameSpace_setConstData(kctx, ns, ct->classNameSymbol, TY_TYPE, (uintptr_t)ct, pline)) {
		return NULL;
	}
	return ct;
}

// ---------------------------------------------------------------------------
/* Method Management */

static inline long Method_id(kMethod *mtd)
{
	long id = mtd->typeId;
	return (id << (sizeof(kshort_t)*8)) | mtd->mn;
}

static int comprMethod(const void *a, const void *b)
{
	long aid = Method_id(((kMethod**)a)[0]);
	long bid = Method_id(((kMethod**)b)[0]);
	if(aid == bid) return 0;
	return aid < bid ? -1 : 1;
}

static void kMethodList_matchMethod(KonohaContext *kctx, kArray *methodList, const size_t *sorted, ktype_t typeId, MethodMatchFunc MatchMethod, MethodMatch *option)
{
	long i, min = 0, max = sorted[0];
	long optkey = ((long)typeId << (sizeof(kshort_t)*8)) | option->mn;
	if(kArray_size(methodList) - max > 8) {
		max = kArray_size(methodList);
		PLATAPI qsort_i(methodList->methodItems, max, sizeof(kMethod*), comprMethod);
		((size_t*)sorted)[0] = max;
	}
	while(min < max) {
		size_t p = (max + min) / 2;
		kMethod *mtd = methodList->methodItems[p];
		long key = Method_id(mtd);
		if(key == optkey) {
			MatchMethod(kctx, mtd, option);
			i = p - 1;
			while(i >= 0) {
				kMethod *mtd = methodList->methodItems[i];
				if(Method_id(mtd) != optkey) break;
				MatchMethod(kctx, mtd, option);
				i--;
			}
			i = p + 1;
			while(i < sorted[0]) {
				kMethod *mtd = methodList->methodItems[i];
				if(Method_id(mtd) != optkey) break;
				MatchMethod(kctx, mtd, option);
				i++;
			}
			break;
		}
		else if(key < optkey) {
			min = p + 1;
		}
		else {
			max = p;
		}
	}
	for(i = sorted[0]; i < kArray_size(methodList); i++) {
		kMethod *mtd = methodList->methodItems[i];
		long key = Method_id(mtd);
		if(key == optkey) {
			MatchMethod(kctx, mtd, option);
		}
	}
}

static kMethod* kNameSpace_matchMethodNULL(KonohaContext *kctx, kNameSpace *startNameSpace, ktype_t typeId, MethodMatchFunc MatchMethod, MethodMatch *option)
{
	KonohaClass *ct = CT_(typeId);
	while(ct != NULL) {
		kNameSpace *ns = startNameSpace;
		while(ns != NULL) {
			kMethodList_matchMethod(kctx, ns->methodList, &ns->sortedMethodList, ct->typeId, MatchMethod, option);
			if(option->isBreak) {
				return option->foundMethodNULL;
			}
			ns = ns->parentNULL;
		}
		kMethodList_matchMethod(kctx, ct->methodList,  &ct->sortedMethodList, ct->typeId, MatchMethod, option);
		if(option->isBreak) {
			return option->foundMethodNULL;
		}
		ct = ct->searchSuperMethodClassNULL;
	}
	return option->foundMethodNULL;
}

//static kbool_t MethodMatch_StaticFunc(KonohaContext *kctx, kMethod *mtd, MethodMatch *m)
//{
//	if(kMethod_is(Static, mtd)) {
//		if(m->foundMethodNULL != NULL) {
//			if(m->foundMethodNULL->serialNumber > mtd->serialNumber) return false;
//		}
//		m->foundMethodNULL = mtd;
//		m->isBreak = true;
//		return true;
//	}
//	return true;
//}

static kbool_t MethodMatch_Func(KonohaContext *kctx, kMethod *mtd, MethodMatch *m)
{
	if(m->foundMethodNULL != NULL) {
		if(m->foundMethodNULL->serialNumber > mtd->serialNumber) return false;
	}
	m->foundMethodNULL = mtd;
	m->isBreak = true;
	return true;
}


static kbool_t MethodMatch_ParamSize(KonohaContext *kctx, kMethod *mtd, MethodMatch *m)
{
	kParam *param = Method_param(mtd);
	if(param->psize == m->paramsize) {
		if(m->foundMethodNULL != NULL) {
			if(m->foundMethodNULL->serialNumber < mtd->serialNumber) return true;
		}
		m->isBreak = true;
		m->foundMethodNULL = mtd;
		return true;
	}
	return false;
}

static kbool_t MethodMatch_Param0(KonohaContext *kctx, kMethod *mtd, MethodMatch *m)
{
	kParam *param = Method_param(mtd);
	if(param->psize == 0) {
		m->foundMethodNULL = mtd;
		m->isBreak = true;
		return true;
	}
	return false;
}

static kbool_t MethodMatch_ParamNoCheck(KonohaContext *kctx, kMethod *mtd, MethodMatch *m)
{
	m->foundMethodNULL = mtd;
	m->isBreak = true;
	return true;
}

static kbool_t CT_isa(KonohaContext *kctx, ktype_t cid1, ktype_t cid2)
{
	DBG_ASSERT(cid1 != cid2); // should be checked
	KonohaClass *ct = CT_(cid1), *t = CT_(cid2);
	return ct->isSubType(kctx, ct, t);
}

static kMethod* kNameSpace_getCastMethodNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t cid, ktype_t tcid)
{
	MethodMatch m = {};
	m.mn = MN_to(tcid);
	m.paramsize = 0;
	kMethod *mtd = kNameSpace_matchMethodNULL(kctx, ns, cid, MethodMatch_Param0, &m);
	if(mtd == NULL) {
		m.mn = MN_as(tcid);
		mtd = kNameSpace_matchMethodNULL(kctx, ns, cid, MethodMatch_Param0, &m);
	}
	return mtd;
}

static kbool_t MethodMatch_Signature(KonohaContext *kctx, kMethod *mtd, MethodMatch *m)
{
	if(mtd->paramdom == m->paramdom) {
		m->isBreak = true;
		m->foundMethodNULL = mtd;
		return true;
	}
	if(m->param != NULL && m->foundMethodNULL == NULL) {
		kParam *param = Method_param(mtd);
		if(param->psize == m->paramsize) {
			int i;
			for(i = 0; i < m->paramsize; i++) {
				if(m->param[i].ty != param->paramtypeItems[i].ty) {
					if(CT_isa(kctx, m->param[i].ty, param->paramtypeItems[i].ty)) {
						continue;
					}
					kMethod *castMethod = kNameSpace_getCastMethodNULL(kctx, m->ns, m->param[i].ty, param->paramtypeItems[i].ty);
					if(castMethod != NULL && (kMethod_is(Coercion, castMethod) || FN_isCOERCION(param->paramtypeItems[i].fn))) {
						continue;
					}
					return false;
				}
			}
			m->foundMethodNULL = mtd;
			return false;
		}
	}
	return false;
}

static kMethod* kNameSpace_getNameSpaceFuncNULL(KonohaContext *kctx, kNameSpace *ns, ksymbol_t symbol, ktype_t reqty)
{
	MethodMatch m = {};
	m.mn = symbol;
	if(TY_isFunc(reqty)) {
		m.paramdom = CT_(reqty)->cparamdom;
		kNameSpace_matchMethodNULL(kctx, ns, O_typeId(ns), MethodMatch_Signature, &m);
		if(m.foundMethodNULL != NULL) {
			return m.foundMethodNULL;
		}
	}
	return kNameSpace_matchMethodNULL(kctx, ns, O_typeId(ns), MethodMatch_Func, &m);
}

static ksymbol_t anotherSymbol(KonohaContext *kctx, ksymbol_t symbol)
{
	kString *s = SYM_s(symbol);
	size_t len = S_size(s);
	char *t = ALLOCA(char, len+1);
	memcpy(t, S_text(s), len);
	t[len]=0;
	if(isupper(t[0])) {
		t[0] = tolower(t[0]);
	}
	else {
		t[0] = toupper(t[0]);
	}
	DBG_P("'%s' => '%s'", S_text(s), t);
	return KLIB Ksymbol(kctx, (const char *)t, len, 0, SYM_NONAME);
}

static kMethod* kNameSpace_getGetterMethodNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t cid, ksymbol_t symbol, ktype_t type)
{
	if(symbol != SYM_NONAME) {
		MethodMatch m = {};
		m.mn = MN_toGETTER(symbol);
		m.paramsize = 0;
		kMethod *mtd = kNameSpace_matchMethodNULL(kctx, ns, cid, MethodMatch_Param0, &m);
		if(mtd == NULL && ((symbol = anotherSymbol(kctx, symbol)) != SYM_NONAME)) {
			m.mn = MN_toGETTER(symbol);
			mtd = kNameSpace_matchMethodNULL(kctx, ns, cid, MethodMatch_Param0, &m);
		}
		return mtd;
	}
	return NULL;
}

static kMethod* kNameSpace_getSetterMethodNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t cid, ksymbol_t symbol, ktype_t type)
{
	if(symbol != SYM_NONAME) {
		MethodMatch m = {};
		m.mn = MN_toSETTER(symbol);
		m.paramsize = 1;
		MethodMatchFunc func;
		if(type == TY_var) {
			func = MethodMatch_ParamSize;
		}
		else {
			kparamtype_t p = {type};
			m.paramdom = KLIB Kparamdom(kctx, 1, &p);
			func = MethodMatch_Signature;
		}
		kMethod *mtd = kNameSpace_matchMethodNULL(kctx, ns, cid, func, &m);
		if(mtd == NULL && ((symbol = anotherSymbol(kctx, symbol)) != SYM_NONAME)) {
			m.mn = MN_toSETTER(symbol);
			mtd = kNameSpace_matchMethodNULL(kctx, ns, cid, func, &m);
		}
		return mtd;
	}
	return NULL;
}

static kMethod* kNameSpace_getMethodByParamSizeNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t cid, ksymbol_t symbol, int paramsize)
{
	MethodMatch m = {};
	m.mn = symbol;
	m.paramsize = paramsize;
	MethodMatchFunc func = paramsize == 0 ? MethodMatch_Param0 : MethodMatch_ParamSize;
	if(paramsize == -1) func = MethodMatch_ParamNoCheck;
	return kNameSpace_matchMethodNULL(kctx, ns, cid, func, &m);
}

static kMethod* kNameSpace_getMethodBySignatureNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t cid, ksymbol_t symbol, int paramdom, int paramsize, kparamtype_t *param)
{
	MethodMatch m = {};
	m.ns = ns;
	m.mn = symbol;
	m.paramdom = paramdom;
	m.paramsize = paramsize;
	m.param = param;
	return kNameSpace_matchMethodNULL(kctx, ns, cid, MethodMatch_Signature, &m);
}

// ---------------------------------------------------------------------------

static kMethod* kMethod_replaceWith(KonohaContext *kctx, kMethodVar *oldMethod, kMethodVar *newMethod)
{
	if(kMethod_is(Override, newMethod)) {
		kMethodVar tempMethod;
		tempMethod = *oldMethod;
		*oldMethod = *newMethod;
		*newMethod = tempMethod;
		return NULL;  // when it succeed
	}
	return oldMethod;
}

static kMethod* kNameSpace_addMethod(KonohaContext *kctx, kNameSpace *ns, kMethod *mtd)
{
	KonohaClass *ct = CT_(mtd->typeId);
	if(mtd->packageId == 0 && ns != NULL) {
		((kMethodVar*)mtd)->packageId = ns->packageId;
		KLIB Kreportf(kctx, DebugTag, 0, "@%s loading method %s.%s%s", PackageId_t(ns->packageId), Method_t(mtd));
	}
	kMethod *foundMethod = kNameSpace_getMethodBySignatureNULL(kctx, ns, ct->typeId, mtd->mn, mtd->paramdom, 0, NULL);
	if(foundMethod != NULL) {  // same signature
		if(foundMethod->typeId == mtd->typeId) {
			DBG_P("duplicated method %s.%s%s", Method_t(foundMethod));
			PUSH_GCSTACK(mtd);  // avoid memory leaking
			return kMethod_replaceWith(kctx, (kMethodVar*)foundMethod, (kMethodVar*)mtd);
		}
		else {
			if(!kMethod_is(Final, foundMethod)) {
				DBG_P("Changing Virtual method %s.%s%s by %s.%s%s....", Method_t(foundMethod), Method_t(mtd));
				kMethod_set(Virtual, foundMethod, true);  // FIXME
			}
			if(!kMethod_is(Virtual, foundMethod) || kMethod_is(Final, foundMethod)) {
				DBG_P("Can't override method %s.%s%s <: %s.%s%s ....", Method_t(mtd), Method_t(foundMethod));
				return NULL;
			}
		}
	}
	else {
		foundMethod = kNameSpace_getMethodByParamSizeNULL(kctx, ns, ct->typeId, mtd->mn, Method_paramsize(mtd));
		if(foundMethod != NULL) {
			kMethod_set(Overloaded, foundMethod, true);
			kMethod_set(Overloaded, mtd, true);
		}
	}
	if(kMethod_is(Public, mtd)) {
		if(unlikely(ct->methodList == K_EMPTYARRAY)) {
			KINITv(((KonohaClassVar*)ct)->methodList, new_(MethodArray, 8));
		}
		KLIB kArray_add(kctx, ct->methodList, mtd);
	}
	else {
		if(ns->methodList == K_EMPTYARRAY) {
			KSETv(ns, ((kNameSpaceVar*)ns)->methodList, new_(MethodArray, 8));
		}
		KLIB kArray_add(kctx, ns->methodList, mtd);
	}
	return NULL;
}

static void kNameSpace_loadMethodData(KonohaContext *kctx, kNameSpace *ns, intptr_t *data)
{
	intptr_t *d = data;
	while(d[0] != -1) {
		uintptr_t flag = (uintptr_t)d[0];
		MethodFunc f = (MethodFunc)d[1];
		ktype_t rtype = (ktype_t)d[2];
		ktype_t cid  = (ktype_t)d[3];
		kmethodn_t mn = (kmethodn_t)d[4];
		size_t i, psize = (size_t)d[5];
		kparamtype_t *p = ALLOCA(kparamtype_t, psize+1);
		d = d + 6;
		for(i = 0; i < psize; i++) {
			p[i].ty = (ktype_t)d[0];
			p[i].fn = (ksymbol_t)d[1];
			d += 2;
		}
		kMethod *mtd = KLIB new_kMethod(kctx, flag, cid, mn, f);
		KLIB kMethod_setParam(kctx, mtd, rtype, psize, p);
		kNameSpace_addMethod(kctx, ns, mtd);
	}
}

// ---------------------------------------------------------------------------

static kstatus_t kNameSpace_eval(KonohaContext *kctx, kNameSpace *ns, const char *script, kfileline_t uline);

typedef struct {
	KonohaContext *kctx;
	kNameSpace *ns;
} SugarThunk;

static int evalHookFunc(const char* script, long uline, int *isBreak, void *thunk)
{
	SugarThunk *t = (SugarThunk*)thunk;
//	if(verbose_sugar) {
//		DUMP_P("\n>>>----\n'%s'\n------\n", script);
//	}
	kstatus_t result = kNameSpace_eval(t->kctx, t->ns, script, uline);
	*isBreak = (result == K_BREAK);
	return (result != K_FAILED);
}

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

static kfileline_t uline_init(KonohaContext *kctx, const char *path, int line, int isreal)
{
	kfileline_t uline = line;
	uline |= KLIB KfileId(kctx, path, strlen(path), 0, _NEWID);
	return uline;
}

static kbool_t kNameSpace_loadScript(KonohaContext *kctx, kNameSpace *ns, const char *path, kfileline_t pline)
{
	SugarThunk thunk = {kctx, ns};
	kfileline_t uline = uline_init(kctx, path, 1, true/*isRealPath*/);
	if(!(PLATAPI loadScript(path, uline, (void*)&thunk, evalHookFunc))) {
		kreportf(ErrTag, pline, "failed to load script: %s", PLATAPI shortText(path));
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------
// Package Management */

static kNameSpace* new_PackageNameSpace(KonohaContext *kctx, kpackage_t packageDomain, kpackage_t packageId)
{
	kNameSpaceVar *ns = (kNameSpaceVar*)GCSAFE_new(NameSpace, KNULL(NameSpace));
	ns->packageId = packageId;
	return (kNameSpace*)ns;
}

static KonohaPackage *loadPackageNULL(KonohaContext *kctx, kpackage_t packageId, kfileline_t pline)
{
	const char *packageName = S_text(PackageId_s(packageId));
	char pathbuf[256];
	const char *path = PLATAPI formatPackagePath(pathbuf, sizeof(pathbuf), packageName, "_glue.k");
	KonohaPackageHandler *packageHandler = PLATAPI loadPackageHandler(packageName);
	if(path == NULL && packageHandler == NULL) {
		KLIB Kreportf(kctx, ErrTag, pline, "package not found: %s path=%s", packageName, PLATAPI shortText(pathbuf));
		KLIB KonohaRuntime_raise(kctx, EXPT_("PackageLoader"), NULL, pline, NULL);
		return NULL;
	}
	kNameSpace *ns = new_PackageNameSpace(kctx, packageId, packageId);
	if(packageHandler != NULL && packageHandler->initPackage != NULL) {
		packageHandler->initPackage(kctx, ns, 0, NULL, pline);
	}
	if(path != NULL) {
		if(!kNameSpace_loadScript(kctx, ns, pathbuf, pline)) {
			return NULL;
		}
	}
	KonohaPackage *pack = (KonohaPackage*)KCALLOC(sizeof(KonohaPackage), 1);
	pack->packageId = packageId;
	KINITv(pack->packageNameSpace, ns);
	pack->packageHandler = packageHandler;
	path = PLATAPI formatPackagePath(pathbuf, sizeof(pathbuf), packageName, "_exports.k");
	if(path != NULL) {
		pack->exportScriptUri = KLIB KfileId(kctx, pathbuf, strlen(pathbuf), 0, _NEWID) | 1;
	}
	return pack;
}

static KonohaPackage *getPackageNULL(KonohaContext *kctx, kpackage_t packageId, kfileline_t pline)
{
	KLock(kctx->share->filepackMutex);
	KonohaPackage *pack = (KonohaPackage*)map_getu(kctx, kctx->share->packageMapNO, packageId, uNULL);
	KUnlock(kctx->share->filepackMutex);
	isFirstTime_t flag = FirstTime;
	if(pack == NULL) {
		pack = loadPackageNULL(kctx, packageId, pline);
		if(pack == NULL) return NULL;
		KLock(kctx->share->filepackMutex);
		map_addu(kctx, kctx->share->packageMapNO, packageId, (uintptr_t)pack);
		KUnlock(kctx->share->filepackMutex);
		flag = Nope;
	}
	if(pack->packageHandler != NULL && pack->packageHandler->setupPackage != NULL) {
		pack->packageHandler->setupPackage(kctx, pack->packageNameSpace, flag, pline);
	}
	return pack;
}

static kbool_t kNameSpace_importSymbol(KonohaContext *kctx, kNameSpace *ns, kNameSpace *targetNS, ksymbol_t keyword, kfileline_t pline)
{
	SugarSyntax *syn = SYN_(targetNS, keyword);
	if(syn != NULL) {
		return kNameSpace_importSyntax(kctx, ns, syn, pline);
	}
	else {
		KUtilsKeyValue *kvs = kNameSpace_getLocalConstNULL(kctx, targetNS, keyword);
		if(kvs != NULL) {
			if(kNameSpace_mergeConstData(kctx, (kNameSpaceVar*)ns, kvs, 1, pline)) {
				if(kvs->ty == TY_TYPE) {
					size_t i;
					ktype_t typeId = ((KonohaClass*)kvs->unboxValue)->typeId;
					for(i = 0; i < kArray_size(targetNS->methodList); i++) {
						kMethod *mtd = targetNS->methodList->methodItems[i];
						if(mtd->typeId == typeId /*&& !kMethod_is(Private, mtd)*/) {
							KLIB kArray_add(kctx, ns->methodList, mtd);
						}
					}
				}
				return true;
			}
		}
	}
	return false;
}

static kbool_t kNameSpace_isImported(KonohaContext *kctx, kNameSpace *ns, kNameSpace *target, kfileline_t pline)
{
	KUtilsKeyValue* value = kNameSpace_getLocalConstNULL(kctx, ns, target->packageId | KW_PATTERN);
	if(value != NULL) {
		kreportf(DebugTag, pline, "package %s has already imported in %s", PackageId_t(ns->packageId), PackageId_t(target->packageId));
		return true;
	}
	return false;
}

static kbool_t kNameSpace_importAll(KonohaContext *kctx, kNameSpace *ns, kNameSpace *targetNS, kfileline_t pline)
{
	if(!kNameSpace_isImported(kctx, ns, targetNS, pline)) {
		size_t i;
		if(kNameSpace_sizeConstTable(targetNS) > 0) {
			if(!kNameSpace_mergeConstData(kctx, (kNameSpaceVar*)ns, targetNS->constTable.keyValueItems, kNameSpace_sizeConstTable(targetNS), pline)) {
				return false;
			}
		}
		kNameSpace_importSyntaxAll(kctx, ns, targetNS, pline);
		for(i = 0; i < kArray_size(targetNS->methodList); i++) {
			kMethod *mtd = targetNS->methodList->methodItems[i];
			if(kMethod_is(Public, mtd) && mtd->packageId == targetNS->packageId) {
				KLIB kArray_add(kctx, ns->methodList, mtd);
			}
		}
		// record imported
		return kNameSpace_setConstData(kctx, ns, targetNS->packageId | KW_PATTERN, TY_int, targetNS->packageId, pline);
	}
	return false;
}

static KonohaPackage* kNameSpace_requirePackage(KonohaContext *kctx, const char *name, kfileline_t pline)
{
	kpackage_t packageId = KLIB KpackageId(kctx, name, strlen(name), 0, _NEWID);
	KonohaPackage *pack = getPackageNULL(kctx, packageId, pline);
	return pack;
}

static kbool_t kNameSpace_importPackage(KonohaContext *kctx, kNameSpace *ns, const char *name, kfileline_t pline)
{
	kpackage_t packageId = KLIB KpackageId(kctx, name, strlen(name), 0, _NEWID);
	KonohaPackage *pack = getPackageNULL(kctx, packageId, pline);
	DBG_ASSERT(ns != NULL);
	if(pack != NULL) {
		kbool_t isContinousLoading = kNameSpace_importAll(kctx, ns, pack->packageNameSpace, pline);
		if(isContinousLoading && pack->packageHandler != NULL && pack->packageHandler->initNameSpace != NULL) {
			isContinousLoading = pack->packageHandler->initNameSpace(kctx, pack->packageNameSpace, ns, pline);
		}
		if(isContinousLoading && pack->exportScriptUri != 0) {
			const char *scriptPath = FileId_t(pack->exportScriptUri);
			kfileline_t uline = pack->exportScriptUri | (kfileline_t)1;
			SugarThunk thunk = {kctx, ns};
			isContinousLoading = PLATAPI loadScript(scriptPath, uline, (void*)&thunk, evalHookFunc);
		}
		if(isContinousLoading && pack->packageHandler != NULL && pack->packageHandler->setupNameSpace != NULL) {
			isContinousLoading = pack->packageHandler->setupNameSpace(kctx, pack->packageNameSpace, ns, pline);
		}
		return true;
	}
	return false;
}

static kbool_t kNameSpace_importPackageSymbol(KonohaContext *kctx, kNameSpace *ns, const char *name, ksymbol_t keyword, kfileline_t pline)
{
	kpackage_t packageId = KLIB KpackageId(kctx, name, strlen(name), 0, _NEWID);
	KonohaPackage *pack = getPackageNULL(kctx, packageId, pline);
	if(pack != NULL) {
		return kNameSpace_importSymbol(kctx, ns, pack->packageNameSpace, keyword, pline);
	}
	return false;
}

kstatus_t MODSUGAR_loadScript(KonohaContext *kctx, const char *path, size_t len, kfileline_t pline)
{
	if (KonohaContext_getSugarContext(kctx) == NULL) {
		kmodsugar->h.setup(kctx, (KonohaModule*)kmodsugar, 0/*lazy*/);
	}
	INIT_GCSTACK();
	kpackage_t packageId = KLIB KpackageId(kctx, "main", sizeof("main")-1, 0, _NEWID);
	kNameSpace *ns = new_PackageNameSpace(kctx, packageId, packageId);
	PUSH_GCSTACK(ns);
	kstatus_t result = (kstatus_t)kNameSpace_loadScript(kctx, ns, path, pline);
	RESET_GCSTACK();
	return result;
}
