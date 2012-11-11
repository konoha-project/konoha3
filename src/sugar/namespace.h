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

static void kNameSpace_AppendArrayRef(KonohaContext *kctx, kNameSpace *ns, const kArray **arrayRef, kObject *o)
{
	if(arrayRef[0] == NULL) {
		((kArray**)arrayRef)[0] = new_(Array, 0, ns->NameSpaceConstList);
	}
	DBG_ASSERT(IS_Array(arrayRef[0]));
	KLIB kArray_add(kctx, arrayRef[0], o);
}

static void kNameSpace_AppendArrayRefArray(KonohaContext *kctx, kNameSpace *ns, const kArray **arrayRef, kArray *aList)
{
	if(aList != NULL) {
		size_t i;
		if(arrayRef[0] == NULL) {
			((kArray**)arrayRef)[0] = new_(Array, kArray_size(aList), ns->NameSpaceConstList);
		}
		DBG_ASSERT(IS_Array(arrayRef[0]));
		for(i = 0; i < kArray_size(aList); i++) {
			kObject *o = aList->ObjectItems[i];
			KLIB kArray_add(kctx, arrayRef[0], o);
		}
	}
}

static void kNameSpace_AddFuncList(KonohaContext *kctx, kNameSpace *ns, kArray **funcListTable, int index, kFunc *fo)
{
	kArray *a = funcListTable[index];
	KLIB kArray_add(kctx, ns->NameSpaceConstList, fo);
	if(a == NULL) {
		funcListTable[index] = (kArray *)fo;
		return;
	}
	else if(!IS_Array(a)) {
		kArray *newa = new_(Array, 0, ns->NameSpaceConstList);
		funcListTable[index] = newa;
		KLIB kArray_add(kctx, newa, a);
		a = newa;
	}
	KLIB kArray_add(kctx, a, fo);
}

// ---------------------------------------------------------------------------
/* TokenFunc Management */

static kFunc **kNameSpace_tokenFuncMatrix(KonohaContext *kctx, kNameSpace *ns);

static void kNameSpace_SetTokenFuncMatrix(KonohaContext *kctx, kNameSpace *ns, int konohaChar, kFunc *fo)
{
	kArray **list = (kArray**)kNameSpace_tokenFuncMatrix(kctx, ns);
	kNameSpace_AddFuncList(kctx, ns, list, konohaChar, fo);
	KLIB kMethod_DoLazyCompilation(kctx, (fo)->mtd, NULL, HatedLazyCompile);
}

// ---------------------------------------------------------------------------
// Syntax Management

#define kToken_isFirstPattern(tk)   (KW_isPATTERN(tk->resolvedSymbol) && tk->stmtEntryKey != KW_ExprPattern)
static void kNameSpace_parseSyntaxPattern(KonohaContext *kctx, kNameSpace *ns, const char *rule, kfileline_t uline, kArray *ruleList);

static SugarSyntax* kNameSpace_newSyntax(KonohaContext *kctx, kNameSpace *ns, SugarSyntax *parentSyntax, ksymbol_t keyword)
{
	if(ns->syntaxMapNN == NULL) {
		((kNameSpaceVar *)ns)->syntaxMapNN = KLIB Kmap_init(kctx, 0);
	}
	KHashMapEntry *e = KLIB Kmap_newEntry(kctx, ns->syntaxMapNN, (uintptr_t)keyword);
	SugarSyntaxVar *syn = (SugarSyntaxVar *)KCalloc_UNTRACE(sizeof(SugarSyntax), 1);
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
		return syn->sugarFuncListTable[index]->FuncItems;
	}
	sizeRef[0] = 1;
	return (kFunc**)&(syn->sugarFuncTable[index]);
}

static kbool_t kNameSpace_ImportSyntax(KonohaContext *kctx, kNameSpace *ns, SugarSyntax *target, KTraceInfo *trace)
{
	SugarSyntaxVar *syn = (SugarSyntaxVar *)kNameSpace_GetSyntax(kctx, ns, target->keyword, true/*isNew*/);
	//DBG_P("<<<< packageId=%s,%s >>>>", PackageId_t(syn->lastLoadedPackageId), PackageId_t(target->lastLoadedPackageId));
	if(syn->lastLoadedPackageId != target->lastLoadedPackageId) {
		int index;
		KLIB ReportScriptMessage(kctx, trace, DebugTag, "@%s importing syntax %s%s", PackageId_t(ns->packageId), PSYM_t(syn->keyword));
		syn->flag = target->flag;
		syn->precedence_op1 = target->precedence_op1;
		syn->precedence_op2 = target->precedence_op2;
		syn->macroParamSize = target->macroParamSize;
		kNameSpace_AppendArrayRefArray(kctx, ns, &syn->macroDataNULL_OnList, target->macroDataNULL_OnList);
		kNameSpace_AppendArrayRefArray(kctx, ns, &syn->syntaxPatternListNULL_OnList, target->syntaxPatternListNULL_OnList);
		if(syn->syntaxPatternListNULL_OnList != NULL && SAFECHECK(0 < kArray_size(syn->syntaxPatternListNULL_OnList))) {
			kToken *patternToken = syn->syntaxPatternListNULL_OnList->TokenItems[0];
			if(kToken_isFirstPattern(patternToken)) {
				kNameSpace_AppendArrayRef(kctx, ns, &((kNameSpaceVar *)ns)->stmtPatternListNULL_OnList, UPCAST(patternToken));
			}
		}
		for(index = 0; index < SugarFunc_SIZE; index++) {
			int j, size;
			kFunc **FuncItems = SugarSyntax_funcTable(kctx, target, index, &size);
			for(j = 0; j < size; j++) {
				kNameSpace_AddFuncList(kctx, ns, syn->sugarFuncListTable, index, FuncItems[j]);
			}
		}
		if(target->tokenKonohaChar != 0) {
			int j, size;
			kFunc **FuncItems = SugarSyntax_funcTable(kctx, target, SugarFunc_TokenFunc, &size);
			for(j = 0; j < size; j++) {
				kNameSpace_SetTokenFuncMatrix(kctx, ns, target->tokenKonohaChar, FuncItems[j]);
			}
			syn->tokenKonohaChar = target->tokenKonohaChar;
		}
		syn->lastLoadedPackageId = target->lastLoadedPackageId;
	}
	return true;
}

struct ImportSyntaxArgument {
	kNameSpace *ns;
	KTraceInfo *trace;
};

static void ImportEachSyntax(KonohaContext *kctx, KHashMapEntry *e, void *thunk)
{
	struct ImportSyntaxArgument *argd = (struct ImportSyntaxArgument *)thunk;
	kNameSpace_ImportSyntax(kctx, argd->ns, (SugarSyntax *)e->unboxValue, argd->trace);
}

static kbool_t kNameSpace_ImportSyntaxAll(KonohaContext *kctx, kNameSpace *ns, kNameSpace *targetNS, KTraceInfo *trace)
{
	if(targetNS->syntaxMapNN != NULL) {
		struct ImportSyntaxArgument argumentData = { ns, trace };
		KLIB Kmap_each(kctx, targetNS->syntaxMapNN, &argumentData, ImportEachSyntax);
	}
	return true;
}

static SugarSyntax* kNameSpace_GetSyntax(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, int isNew)
{
	kNameSpace *currentNameSpace = ns;
	uintptr_t hcode = keyword;
	while(currentNameSpace != NULL) {
		if(currentNameSpace->syntaxMapNN != NULL) {
			KHashMapEntry *e = KLIB Kmap_get(kctx, currentNameSpace->syntaxMapNN, hcode);
			while(e != NULL) {
				if(e->hcode == hcode) {
					if(isNew && ns != currentNameSpace) {
						return kNameSpace_newSyntax(kctx, ns, (SugarSyntax *)e->unboxValue, keyword);
					}
					return (SugarSyntax *)e->unboxValue;
				}
				e = e->next;
			}
		}
		currentNameSpace = currentNameSpace->parentNULL;
	}
	return (isNew) ? kNameSpace_newSyntax(kctx, ns, NULL, keyword) : NULL;
}

static kbool_t kNameSpace_RemoveSyntax(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, KTraceInfo *trace)
{
	uintptr_t hcode = keyword;
	if(ns->syntaxMapNN != NULL) {
		KHashMapEntry *e = KLIB Kmap_get(kctx, ns->syntaxMapNN, hcode);
		while(e != NULL) {
			if(e->hcode == hcode) {
				SugarSyntaxVar *syn = (SugarSyntaxVar *)e->unboxValue;
				KLIB ReportScriptMessage(kctx, trace, InfoTag, "@%s removing syntax %s%s", PackageId_t(syn->lastLoadedPackageId), PSYM_t(syn->keyword));
				KLIB Kmap_remove(ns->syntaxMapNN, e);
				bzero(syn, sizeof(SugarSyntax));
				KFree(syn, sizeof(SugarSyntax));
				return true;
			}
			e = e->next;
		}
	}
	return false;
}

static SugarSyntaxVar *kNameSpace_AddSugarFunc(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, size_t idx, kFunc *funcObject)
{
	SugarSyntaxVar *syn = (SugarSyntaxVar *)kNameSpace_GetSyntax(kctx, ns, keyword, 1/*new*/);
	DBG_ASSERT(idx < SugarFunc_SIZE);
	kNameSpace_AddFuncList(kctx, ns, syn->sugarFuncListTable, idx, funcObject);
	KLIB kMethod_DoLazyCompilation(kctx, (funcObject)->mtd, NULL, HatedLazyCompile);
	syn->lastLoadedPackageId = ns->packageId;
	DBG_P("@@@@@@@@@@@@@ %s%s", PSYM_t(keyword));
	return syn;
}

static SugarSyntaxVar *kNameSpace_SetTokenFunc(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, int konohaChar, kFunc *fo)
{
	SugarSyntaxVar *syn = (SugarSyntaxVar *)kNameSpace_GetSyntax(kctx, ns, keyword, 1/*new*/);
	kArray **list = (kArray**)kNameSpace_tokenFuncMatrix(kctx, ns);
	kNameSpace_AddFuncList(kctx, ns, list, konohaChar, fo);
	KLIB kMethod_DoLazyCompilation(kctx, (fo)->mtd, NULL, HatedLazyCompile);
	syn->tokenKonohaChar = konohaChar;
	syn->sugarFuncTable[SugarFunc_TokenFunc] = fo;  // added in addFuncList
	syn->lastLoadedPackageId = ns->packageId;
	return syn;
}

static void SugarSyntax_setMethodFunc(KonohaContext *kctx, kNameSpace *ns, SugarSyntaxVar *syn, MethodFunc definedMethodFunc, size_t index, MethodFunc *previousDefinedFuncRef, kFunc **cachedFuncRef)
{
	if(definedMethodFunc != NULL) {
		if(definedMethodFunc != previousDefinedFuncRef[0]) {
			previousDefinedFuncRef[0] = definedMethodFunc;
			cachedFuncRef[0] = new_SugarFunc(ns, definedMethodFunc);
		}
		KFieldInit(ns, syn->sugarFuncTable[index], cachedFuncRef[0]);
	}
}

static void kNameSpace_DefineSyntax(KonohaContext *kctx, kNameSpace *ns, KDEFINE_SYNTAX *syndef, KTraceInfo *trace)
{
	MethodFunc pPatternMatch = NULL, pExpression = NULL, pStatement = NULL, pTypeCheck = NULL;
	kFunc *mPatternMatch = NULL, *mExpression = NULL, *mStatement = NULL, *mTypeCheck = NULL;
	while(syndef->keyword != KW_END) {
		SugarSyntaxVar* syn = (SugarSyntaxVar *)kNameSpace_GetSyntax(kctx, ns, syndef->keyword, 1/*isnew*/);
		DBG_ASSERT(syn != NULL);
		syn->lastLoadedPackageId = ns->packageId;
		syn->flag  |= ((kshortflag_t)syndef->flag);
		if(syndef->precedence_op1 > 0) {
			syn->precedence_op1 = syndef->precedence_op1;
		}
		if(syndef->precedence_op2 > 0) {
			syn->precedence_op2 = syndef->precedence_op2;
		}
		if(syndef->rule != NULL) {
			syn->syntaxPatternListNULL_OnList = new_(TokenArray, 0, ns->NameSpaceConstList);
			kNameSpace_parseSyntaxPattern(kctx, ns, syndef->rule, 0, syn->syntaxPatternListNULL_OnList);
		}
		SugarSyntax_setMethodFunc(kctx, ns, syn, syndef->PatternMatch,   SugarFunc_PatternMatch,   &pPatternMatch, &mPatternMatch);
		SugarSyntax_setMethodFunc(kctx, ns, syn, syndef->Expression,      SugarFunc_Expression,      &pExpression, &mExpression);
		SugarSyntax_setMethodFunc(kctx, ns, syn, syndef->TopLevelStatement, SugarFunc_TopLevelStatement, &pStatement, &mStatement);
		SugarSyntax_setMethodFunc(kctx, ns, syn, syndef->Statement,    SugarFunc_Statement,    &pStatement, &mStatement);
		SugarSyntax_setMethodFunc(kctx, ns, syn, syndef->TypeCheck,    SugarFunc_TypeCheck,    &pTypeCheck, &mTypeCheck);
		// set default function
		if(syn->parentSyntaxNULL == NULL && syn->sugarFuncTable[SugarFunc_Expression] == NULL) {
			if(syn->precedence_op2 > 0 || syn->precedence_op1 > 0) {
				kFunc *fo = SYN_(ns, KW_ExprOperator)->sugarFuncTable[SugarFunc_Expression];
				DBG_ASSERT(fo != NULL);
				KFieldInit(ns, syn->sugarFuncTable[SugarFunc_Expression], fo);
			}
			else if(syn->sugarFuncTable[SugarFunc_TypeCheck] != NULL) {
				kFunc *fo = SYN_(ns, KW_ExprTerm)->sugarFuncTable[SugarFunc_Expression];
				DBG_ASSERT(fo != NULL);
				KFieldInit(ns, syn->sugarFuncTable[SugarFunc_Expression], fo);
			}
		}
		DBG_ASSERT(syn == SYN_(ns, syndef->keyword));
		KLIB ReportScriptMessage(kctx, trace, DebugTag, "@%s new syntax %s%s", PackageId_t(ns->packageId), PSYM_t(syn->keyword));
		syndef++;
	}
}

// --------------------------------------------------------------------------
/* ConstTable */

static int comprKeyVal(const void *a, const void *b)
{
	int akey = SYMKEY_unbox(((KKeyValue *)a)->key);
	int bkey = SYMKEY_unbox(((KKeyValue *)b)->key);
	return akey - bkey;
}

static KKeyValue* kNameSpace_GetLocalConstNULL(KonohaContext *kctx, kNameSpace *ns, ksymbol_t unboxKey)
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

static KKeyValue* kNameSpace_GetConstNULL(KonohaContext *kctx, kNameSpace *ns, ksymbol_t unboxKey)
{
	while(ns != NULL) {
		KKeyValue* foundKeyValue = kNameSpace_GetLocalConstNULL(kctx, ns, unboxKey);
		if(foundKeyValue != NULL) return foundKeyValue;
		ns = ns->parentNULL;
	}
	return NULL;
}

static kbool_t kNameSpace_mergeConstData(KonohaContext *kctx, kNameSpaceVar *ns, KKeyValue *kvs, size_t nitems, KTraceInfo *trace)
{
	size_t i, size = kNameSpace_sizeConstTable(ns);
	if(size == 0) {
		KLIB Karray_init(kctx, &ns->constTable, (nitems + 8) * sizeof(KKeyValue));
		memcpy(ns->constTable.keyValueItems, kvs, nitems * sizeof(KKeyValue));
	}
	else {
		KGrowingBuffer wb;
		KLIB Kwb_init(&(GetSugarContext(kctx)->errorMessageBuffer), &wb);
		for(i = 0; i < nitems; i++) {
			ksymbol_t unboxKey = kvs[i].key;
			KKeyValue* stored = kNameSpace_GetLocalConstNULL(kctx, ns, unboxKey);
			if(stored != NULL) {
				if(kvs[i].ty == stored->ty && kvs[i].unboxValue == stored->unboxValue) {
					continue;  // same value
				}
				SugarContext_printMessage(kctx, ErrTag, Trace_pline(trace), "already defined symbol: %s%s", PSYM_t(SYMKEY_unbox(unboxKey)));
				KLIB Kwb_free(&wb);
				return false;
			}
			KLIB Kwb_write(kctx, &wb, (const char *)(kvs+i), sizeof(KKeyValue));
		}
		kvs = (KKeyValue *)KLIB Kwb_top(kctx, &wb, 0);
		nitems = Kwb_bytesize(&wb)/sizeof(KKeyValue);
		if(nitems > 0) {
			if(!((size + nitems) * sizeof(KKeyValue) < ns->constTable.bytemax)) {
				KLIB Karray_resize(kctx, &ns->constTable, (size + nitems + 8) * sizeof(KKeyValue));
			}
			memcpy(ns->constTable.keyValueItems + size, kvs, nitems * sizeof(KKeyValue));
		}
		KLIB Kwb_free(&wb);
	}
	for(i = size; i < size + nitems; i++) {
		ksymbol_t unboxKey = ns->constTable.keyValueItems[i].key;
		if(Symbol_isBoxedKey(unboxKey)) {
			KLIB kArray_add(kctx, ns->NameSpaceConstList, ns->constTable.keyValueItems[i].ObjectValue);
		}
		KLIB ReportScriptMessage(kctx, trace, DebugTag, "@%s loading const %s%s as %s", PackageId_t(ns->packageId), PSYM_t(SYMKEY_unbox(unboxKey)), TY_t(ns->constTable.keyValueItems[i].ty));
	}
	nitems = size + nitems;
	ns->constTable.bytesize = nitems * sizeof(KKeyValue);
	if(nitems - ns->sortedConstTable > 9) {
		PLATAPI qsort_i(ns->constTable.keyValueItems, nitems, sizeof(KKeyValue), comprKeyVal);
		ns->sortedConstTable = nitems;
	}
	return true;
}

static void SetKeyValue(KonohaContext *kctx, KKeyValue *kv, ksymbol_t key, ktype_t ty, uintptr_t unboxValue, kArray *gcstack)
{
	if(TY_isUnbox(ty) || ty == VirtualType_KonohaClass || ty == VirtualType_StaticMethod ) {
		kv->key = key;
	}
	else {
		kv->key = key | SYMKEY_BOXED;
	}
	if(ty == VirtualType_Text) {
		const char *textData = (const char *)unboxValue;
		kv->ty = TY_String;
		/*FIXME(ide) VirtualType_Text (a.k.a. TY_void) is unboxed type */
		kv->key = key | SYMKEY_BOXED;
		kv->StringValue = KLIB new_kString(kctx, gcstack, textData, strlen(textData), StringPolicy_TEXT);
	}
	else {
		kv->ty = ty;
		kv->unboxValue = unboxValue;
	}
}

static kbool_t kNameSpace_SetConstData(KonohaContext *kctx, kNameSpace *ns, ksymbol_t key, ktype_t ty, uintptr_t unboxValue, KTraceInfo *trace)
{
	INIT_GCSTACK();
	KKeyValue kv;
	SetKeyValue(kctx, &kv, key, ty, unboxValue, _GcStack);
	kbool_t ret = kNameSpace_mergeConstData(kctx, (kNameSpaceVar *)ns, &kv, 1, trace);
	RESET_GCSTACK();
	return ret;
}

static kbool_t kNameSpace_LoadConstData(KonohaContext *kctx, kNameSpace *ns, const char **d, KTraceInfo *trace)
{
	INIT_GCSTACK();
	KKeyValue kv;
	KGrowingBuffer wb;
	kbool_t result = true;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	while(d[0] != NULL) {
		SetKeyValue(kctx, &kv,
			ksymbolSPOL(d[0], strlen(d[0]), StringPolicy_TEXT|StringPolicy_ASCII, _NEWID),
			(ktype_t)(uintptr_t)d[1], (uintptr_t)d[2], _GcStack);
		KLIB Kwb_write(kctx, &wb, (const char *)(&kv), sizeof(KKeyValue));
		d += 3;
	}
	size_t nitems = Kwb_bytesize(&wb) / sizeof(KKeyValue);
	if(nitems > 0) {
		result = kNameSpace_mergeConstData(kctx, (kNameSpaceVar *)ns, (KKeyValue *)KLIB Kwb_top(kctx, &wb, 0), nitems, trace);
	}
	KLIB Kwb_free(&wb);
	RESET_GCSTACK();
	return result;
}

// ---------------------------------------------------------------------------
/* ClassName in ConstTable */

static KonohaClass *kNameSpace_GetClass(KonohaContext *kctx, kNameSpace *ns, const char *name, size_t len, KonohaClass *defaultClass)
{
	KonohaClass *ct = NULL;
	kpackageId_t packageId = PN_konoha;
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
		KKeyValue *kvs = kNameSpace_GetConstNULL(kctx, ns, un);
		if(kvs != NULL && kvs->ty == VirtualType_KonohaClass) {
			return (KonohaClass *)kvs->unboxValue;
		}
	}
	return (ct != NULL) ? ct : defaultClass;
}

static KonohaClass *kNameSpace_DefineClass(KonohaContext *kctx, kNameSpace *ns, kString *name, KDEFINE_CLASS *cdef, KTraceInfo *trace)
{
	KonohaClass *ct = KLIB KonohaClass_define(kctx, ns->packageId, name, cdef, trace);
	if(!KLIB kNameSpace_SetConstData(kctx, ns, ct->classNameSymbol, VirtualType_KonohaClass, (uintptr_t)ct, trace)) {
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
		PLATAPI qsort_i(methodList->MethodItems, max, sizeof(kMethod *), comprMethod);
		((size_t *)sorted)[0] = max;
	}
	while(min < max) {
		size_t p = (max + min) / 2;
		kMethod *mtd = methodList->MethodItems[p];
		long key = Method_id(mtd);
		if(key == optkey) {
			MatchMethod(kctx, mtd, option);
			i = p - 1;
			while(i >= 0) {
				kMethod *mtd = methodList->MethodItems[i];
				if(Method_id(mtd) != optkey) break;
				MatchMethod(kctx, mtd, option);
				i--;
			}
			i = p + 1;
			while(i < sorted[0]) {
				kMethod *mtd = methodList->MethodItems[i];
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
		kMethod *mtd = methodList->MethodItems[i];
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
			kMethodList_matchMethod(kctx, ns->methodList_OnList, &ns->sortedMethodList, ct->typeId, MatchMethod, option);
			if(option->isBreak) {
				return option->foundMethodNULL;
			}
			ns = ns->parentNULL;
		}
		kMethodList_matchMethod(kctx, ct->methodList_OnGlobalConstList, &ct->sortedMethodList, ct->typeId, MatchMethod, option);
		if(option->isBreak) {
			return option->foundMethodNULL;
		}
		ct = ct->searchSuperMethodClassNULL;
	}
	return option->foundMethodNULL;
}

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

static kMethod *kNameSpace_GetCoercionMethodNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t cid, ktype_t tcid)
{
	MethodMatch m = {};
	m.mn = MN_to(tcid);
	m.paramsize = 0;
	kMethod *mtd = kNameSpace_matchMethodNULL(kctx, ns, cid, MethodMatch_Param0, &m);
	DBG_P("finding cast %s => %s: %p", TY_t(cid), TY_t(tcid), mtd);
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
			kushort_t i;
			for(i = 0; i < m->paramsize; i++) {
				if(m->param[i].ty != param->paramtypeItems[i].ty) {
					if(CT_isa(kctx, m->param[i].ty, param->paramtypeItems[i].ty)) {
						continue;
					}
					kMethod *castMethod = kNameSpace_GetCoercionMethodNULL(kctx, m->ns, m->param[i].ty, param->paramtypeItems[i].ty);
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

static kMethod *kNameSpace_GetNameSpaceFuncNULL(KonohaContext *kctx, kNameSpace *ns, ksymbol_t symbol, ktype_t reqty)
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

static size_t CheckAnotherSymbol(KonohaContext *kctx, ksymbol_t *resolved, size_t foundNames, char *buffer, size_t len)
{
	resolved[foundNames] = KLIB Ksymbol(kctx, (const char *)buffer, len, 0, SYM_NONAME);
	if(resolved[foundNames] != SYM_NONAME) {
		DBG_P("CHECKING ANOTHER NAME: '%s'", buffer);
		foundNames++;
	}
	return foundNames;
}

#define ANOTHER_NAME_MAXSIZ 4

static size_t FindAnotherSymbol(KonohaContext *kctx, ksymbol_t symbol, ksymbol_t *resolved)
{
	const char *name = SYM_t(symbol);
	size_t i, foundNames = 0, len = S_size(SYM_s(symbol));
	char *buffer = ALLOCA(char, len+1);
	memcpy(buffer, name, len);
	buffer[len] = 0;
	if(isupper(buffer[0])) {
		buffer[0] = tolower(buffer[0]);  // FirstName => firstName, Firstname  => firstname
		foundNames = CheckAnotherSymbol(kctx, resolved, foundNames, buffer, len);
		for(i = 0; i < len; i++) {
			buffer[i] = tolower(buffer[i]);
		}
		foundNames = CheckAnotherSymbol(kctx, resolved, foundNames, buffer, len);
	}
	else {
		buffer[0] = toupper(buffer[0]);  // firstName => FirstName, firstname => FirstName
		foundNames = CheckAnotherSymbol(kctx, resolved, foundNames, buffer, len);
	}
	DBG_ASSERT(foundNames < ANOTHER_NAME_MAXSIZ);
	return foundNames;
}

static kMethod *kNameSpace_GetGetterMethodNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t cid, ksymbol_t symbol, ktype_t type)
{
	if(symbol != SYM_NONAME) {
		MethodMatch m = {};
		m.mn = MN_toGETTER(symbol);
		m.paramsize = 0;
		kMethod *mtd = kNameSpace_matchMethodNULL(kctx, ns, cid, MethodMatch_Param0, &m);
		if(mtd == NULL) {
			ksymbol_t anotherSymbols[ANOTHER_NAME_MAXSIZ];
			size_t i, foundNames = FindAnotherSymbol(kctx, symbol, anotherSymbols);
			for(i = 0; i < foundNames; i++) {
				m.mn = MN_toGETTER(anotherSymbols[i]);
				mtd = kNameSpace_matchMethodNULL(kctx, ns, cid, MethodMatch_Param0, &m);
				if(mtd != NULL) break;
			}
		}
		return mtd;
	}
	return NULL;
}

static kMethod *kNameSpace_GetSetterMethodNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t cid, ksymbol_t symbol, ktype_t type)
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
		if(mtd == NULL) {
			ksymbol_t anotherSymbols[ANOTHER_NAME_MAXSIZ];
			size_t i, foundNames = FindAnotherSymbol(kctx, symbol, anotherSymbols);
			for(i = 0; i < foundNames; i++) {
				m.mn = MN_toSETTER(anotherSymbols[i]);
				mtd = kNameSpace_matchMethodNULL(kctx, ns, cid, func, &m);
				if(mtd != NULL) break;
			}
		}
		return mtd;
	}
	return NULL;
}

static kMethod *kNameSpace_GetMethodByParamSizeNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t cid, ksymbol_t symbol, int paramsize)
{
	MethodMatch m = {};
	m.mn = symbol;
	m.paramsize = paramsize;
	MethodMatchFunc func = paramsize == 0 ? MethodMatch_Param0 : MethodMatch_ParamSize;
	if(paramsize == -1) func = MethodMatch_ParamNoCheck;
	kMethod *mtd = kNameSpace_matchMethodNULL(kctx, ns, cid, func, &m);
	if(mtd == NULL) {
		ksymbol_t anotherSymbols[ANOTHER_NAME_MAXSIZ];
		size_t i, foundNames = FindAnotherSymbol(kctx, symbol, anotherSymbols);
		for(i = 0; i < foundNames; i++) {
			m.mn = anotherSymbols[i];
			mtd = kNameSpace_matchMethodNULL(kctx, ns, cid, func, &m);
			if(mtd != NULL) break;
		}
	}
	return mtd;
}

static kMethod *kNameSpace_GetMethodToCheckOverloadNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t cid, ksymbol_t symbol, int paramsize)
{
	MethodMatch m = {};
	m.mn = symbol;
	m.paramsize = paramsize;
	MethodMatchFunc func = paramsize == 0 ? MethodMatch_Param0 : MethodMatch_ParamSize;
	if(paramsize == -1) func = MethodMatch_ParamNoCheck;
	return kNameSpace_matchMethodNULL(kctx, ns, cid, func, &m);
}

static kMethod *kNameSpace_GetMethodBySignatureNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t cid, ksymbol_t symbol, int paramdom, int paramsize, kparamtype_t *param)
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

static void kMethod_ReplaceWith(KonohaContext *kctx, kMethodVar *oldMethod, kMethodVar *newMethod)
{
	kMethodVar tempMethod;
	tempMethod = *oldMethod;
	*oldMethod = *newMethod;
	*newMethod = tempMethod;
}

static kbool_t kNameSpace_AddMethod(KonohaContext *kctx, kNameSpace *ns, kMethodVar *mtd, KTraceInfo *trace)
{
	KonohaClass *ct = CT_(mtd->typeId);
	if(mtd->packageId == 0) {
		((kMethodVar *)mtd)->packageId = ns->packageId;
		TRACE_ReportScriptMessage(kctx, trace, DebugTag, "@%s loading method %s.%s%s", PackageId_t(ns->packageId), Method_t(mtd));
	}
	if(CT_is(Final, ct)) {
		kMethod_set(Final, mtd, true);
	}
	kMethod *foundMethod = kNameSpace_GetMethodBySignatureNULL(kctx, ns, ct->typeId, mtd->mn, mtd->paramdom, 0, NULL);
	if(foundMethod != NULL) {  // same signature
		if(foundMethod->typeId == mtd->typeId) {
			if(kMethod_is(Override, mtd)) {
				TRACE_ReportScriptMessage(kctx, trace, DebugTag, "@%s overriding method %s.%s%s on %s", PackageId_t(ns->packageId), Method_t(mtd), PackageId_t(foundMethod->packageId));
				kMethod_ReplaceWith(kctx, (kMethodVar *)foundMethod, mtd);
			}
			else {
				TRACE_ReportScriptMessage(kctx, trace, ErrTag, "duplicated method: %s.%s%s on %s", Method_t(foundMethod), PackageId_t(foundMethod->packageId));
			}
			return false;
		}
		else {
			if(!kMethod_is(Final, foundMethod)) {
				TRACE_ReportScriptMessage(kctx, trace, DebugTag, "@%s overriding method %s.%s%s on %s.%s%s", PackageId_t(ns->packageId), Method_t(mtd), Method_t(foundMethod));
				kMethod_set(Virtual, ((kMethodVar *)foundMethod), true);
			}
			else {
				TRACE_ReportScriptMessage(kctx, trace, ErrTag, "final method: %s.%s%s", Method_t(foundMethod));
				return false;
			}
		}
	}
	else {
		foundMethod = kNameSpace_GetMethodToCheckOverloadNULL(kctx, ns, ct->typeId, mtd->mn, Method_paramsize(mtd));
		if(foundMethod != NULL && foundMethod->mn == mtd->mn) {
			kMethod_set(Overloaded, ((kMethodVar *)foundMethod), true);
			kMethod_set(Overloaded, mtd, true);
		}
	}
	if(kMethod_is(Public, mtd)) {
		if(unlikely(ct->methodList_OnGlobalConstList == K_EMPTYARRAY)) {
			((KonohaClassVar *)ct)->methodList_OnGlobalConstList = new_(MethodArray, 8, OnGlobalConstList);
		}
		KLIB kArray_add(kctx, ct->methodList_OnGlobalConstList, mtd);
	}
	else {
		if(ns->methodList_OnList == K_EMPTYARRAY) {
			((kNameSpaceVar *)ns)->methodList_OnList = new_(MethodArray, 8, ns->NameSpaceConstList);
		}
		KLIB kArray_add(kctx, ns->methodList_OnList, mtd);
	}
	return true;
}

static void kNameSpace_LoadMethodData(KonohaContext *kctx, kNameSpace *ns, intptr_t *data, KTraceInfo *trace)
{
	intptr_t *d = data;
	INIT_GCSTACK();
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
		kMethodVar *mtd = KLIB new_kMethod(kctx, _GcStack, flag, cid, mn, f);
		KLIB kMethod_setParam(kctx, mtd, rtype, psize, p);
		kNameSpace_AddMethod(kctx, ns, mtd, trace);
	}
	RESET_GCSTACK();
}

// ---------------------------------------------------------------------------

static kstatus_t kNameSpace_Eval(KonohaContext *kctx, kNameSpace *ns, const char *script, kfileline_t uline, KTraceInfo *trace);

typedef struct {
	KonohaContext *kctx;
	kNameSpace *ns;
	KTraceInfo *trace;
} SugarThunk;

static int evalHookFunc(const char *script, long uline, int *isBreak, void *thunk)
{
	SugarThunk *t = (SugarThunk *)thunk;
//	if(verbose_sugar) {
//		DUMP_P("\n>>>----\n'%s'\n------\n", script);
//	}
	kstatus_t result = kNameSpace_Eval(t->kctx, t->ns, script, uline, t->trace);
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

static kbool_t kNameSpace_LoadScript(KonohaContext *kctx, kNameSpace *ns, const char *path, KTraceInfo *trace)
{
	SugarThunk thunk = {kctx, ns, trace};
	kfileline_t uline = uline_init(kctx, path, 1, true/*isRealPath*/);
	if(!(PLATAPI loadScript(path, uline, (void *)&thunk, evalHookFunc))) {
		KLIB ReportScriptMessage(kctx, trace, ErrTag, "failed to load script: %s", path);
		return false;
	}
	if(KonohaContext_Is(CompileOnly, kctx)) {
		size_t i, size = kArray_size(GetSugarContext(kctx)->definedMethodList);
		for (i = 0; i < size; ++i) {
			kMethod *mtd = GetSugarContext(kctx)->definedMethodList->MethodItems[i];
			KLIB kMethod_DoLazyCompilation(kctx, mtd, NULL, DefaultCompileOption);
		}
		KLIB kArray_clear(kctx, GetSugarContext(kctx)->definedMethodList, 0);
	}
	return true;
}

// ---------------------------------------------------------------------------
// Package Management */

#define CT_NameSpaceVar CT_NameSpace
static kNameSpace *new_PackageNameSpace(KonohaContext *kctx, kpackageId_t packageId)
{
	kNameSpaceVar *ns = new_(NameSpaceVar, KNULL(NameSpace), OnGlobalConstList);
	ns->packageId = packageId;
	return (kNameSpace *)ns;
}

static KonohaPackage *LoadPackageNULL(KonohaContext *kctx, kpackageId_t packageId, int option, KTraceInfo *trace)
{
	const char *packageName = S_text(PackageId_s(packageId));
	char packupbuf[256], kickbuf[256];
	const char *path = PLATAPI FormatPackagePath(kctx, packupbuf, sizeof(packupbuf), packageName, "_packup.k");
	if(path == NULL) {
		path = PLATAPI FormatPackagePath(kctx, packupbuf, sizeof(packupbuf), packageName, "_glue.k");
	}
	const char *kickpath = PLATAPI FormatPackagePath(kctx, kickbuf, sizeof(kickbuf), packageName, "_kick.k");
	KonohaPackageHandler *packageHandler = PLATAPI LoadPackageHandler(kctx, packageName);
	if(path == NULL && packageHandler == NULL && kickpath == NULL) {
		DBG_ASSERT(trace != NULL);
		KBeginCritical(trace, "PackageNotFound", SoftwareFault|SystemFault);
		PLATAPI FormatPackagePath(kctx, packupbuf, sizeof(packupbuf), packageName, "");
		KLIB ReportScriptMessage(kctx, trace, ErrTag, "package not found: path=%s", packupbuf);
		KEndCritical(trace);
		return NULL;
	}
	else {
		KonohaPackage *pack = (KonohaPackage *)KCalloc(sizeof(KonohaPackage), 1, trace);
		pack->packageId = packageId;
		pack->packageHandler = packageHandler;
		if(kickpath != NULL) {
			pack->kickout_script = uline_init(kctx, kickpath, 1, true/*isRealPath*/);
		}
		KLock(kctx->share->filepackMutex);
		map_addu(kctx, kctx->share->packageMapNO, packageId, (uintptr_t)pack);
		KUnlock(kctx->share->filepackMutex);
		//
		DBG_ASSERT(pack->packageNS_onGlobalConstList == NULL);
		kNameSpace *ns = new_PackageNameSpace(kctx, packageId);
		KBeginCritical(trace, "PackageLoading", SoftwareFault|SystemFault);
		if(packageHandler != NULL && packageHandler->PackupNameSpace != NULL) {
			packageHandler->PackupNameSpace(kctx, ns, option, trace);
		}
		if(path != NULL) {
			kNameSpace_LoadScript(kctx, ns, packupbuf, trace);
		}
		KEndCritical(trace);
		pack->packageNS_onGlobalConstList = ns;
		return pack;
	}
}

static KonohaPackage *GetPackageNULL(KonohaContext *kctx, kpackageId_t packageId, int option, KTraceInfo *trace)
{
	KLock(kctx->share->filepackMutex);
	KonohaPackage *pack = (KonohaPackage *)map_getu(kctx, kctx->share->packageMapNO, packageId, uNULL);
	KUnlock(kctx->share->filepackMutex);
	if(pack == NULL) {
		pack = LoadPackageNULL(kctx, packageId, option, trace);
		if(pack == NULL) return NULL;
	}
	else if(pack->packageNS_onGlobalConstList == NULL) {
		KLIB ReportScriptMessage(kctx, trace, ErrTag, "recursive importing: %s", PackageId_t(packageId));
		return NULL;
	}
	return pack;
}

static kbool_t kNameSpace_ImportSymbol(KonohaContext *kctx, kNameSpace *ns, kNameSpace *packageNS, ksymbol_t keyword, KTraceInfo *trace)
{
	SugarSyntax *syn = SYN_(packageNS, keyword);
	if(syn != NULL) {
		return kNameSpace_ImportSyntax(kctx, ns, syn, trace);
	}
	else {
		KKeyValue *kvs = kNameSpace_GetLocalConstNULL(kctx, packageNS, keyword);
		if(kvs != NULL) {
			if(kNameSpace_mergeConstData(kctx, (kNameSpaceVar *)ns, kvs, 1, trace)) {
				if(kvs->ty == VirtualType_KonohaClass) {
					size_t i;
					ktype_t typeId = ((KonohaClass *)kvs->unboxValue)->typeId;
					for(i = 0; i < kArray_size(packageNS->methodList_OnList); i++) {
						kMethod *mtd = packageNS->methodList_OnList->MethodItems[i];
						if(mtd->typeId == typeId /*&& !kMethod_is(Private, mtd)*/) {
							KLIB kArray_add(kctx, ns->methodList_OnList, mtd);
						}
					}
				}
				return true;
			}
		}
	}
	return false;
}

static kbool_t kNameSpace_isImported(KonohaContext *kctx, kNameSpace *ns, kNameSpace *packageNS, KTraceInfo *trace)
{
	KKeyValue *value = kNameSpace_GetLocalConstNULL(kctx, ns, packageNS->packageId | KW_PATTERN);
	if(value != NULL) {
		KLIB ReportScriptMessage(kctx, trace, DebugTag, "package %s has already imported in %s", PackageId_t(ns->packageId), PackageId_t(packageNS->packageId));
		return true;
	}
	return false;
}

static kbool_t kNameSpace_ImportAll(KonohaContext *kctx, kNameSpace *ns, kNameSpace *packageNS, KTraceInfo *trace)
{
	if(!kNameSpace_isImported(kctx, ns, packageNS, trace)) {
		size_t i;
		if(kNameSpace_sizeConstTable(packageNS) > 0) {
			if(!kNameSpace_mergeConstData(kctx, (kNameSpaceVar *)ns, packageNS->constTable.keyValueItems, kNameSpace_sizeConstTable(packageNS), trace)) {
				return false;
			}
		}
		kNameSpace_AppendArrayRefArray(kctx, ns, &ns->stmtPatternListNULL_OnList, packageNS->stmtPatternListNULL_OnList);
		kNameSpace_ImportSyntaxAll(kctx, ns, packageNS, trace);
		for(i = 0; i < kArray_size(packageNS->methodList_OnList); i++) {
			kMethod *mtd = packageNS->methodList_OnList->MethodItems[i];
			if(kMethod_is(Public, mtd) && mtd->packageId == packageNS->packageId) {
				KLIB kArray_add(kctx, ns->methodList_OnList, mtd);
			}
		}
		// record imported
		return kNameSpace_SetConstData(kctx, ns, packageNS->packageId | KW_PATTERN, TY_int, packageNS->packageId, trace);
	}
	return false;
}

static KonohaPackage *kNameSpace_RequirePackage(KonohaContext *kctx, const char *name, KTraceInfo *trace)
{
	kpackageId_t packageId = KLIB KpackageId(kctx, name, strlen(name), 0, _NEWID);
	return GetPackageNULL(kctx, packageId, 0, trace);
}

static kbool_t kNameSpace_ImportPackage(KonohaContext *kctx, kNameSpace *ns, const char *name, KTraceInfo *trace)
{
	kpackageId_t packageId = KLIB KpackageId(kctx, name, strlen(name), 0, _NEWID);
	int option = 0;
	if(ns->packageId != packageId) {
		KonohaPackage *pack = GetPackageNULL(kctx, packageId, option, trace);
		if(pack != NULL) {
			kNameSpace_ImportAll(kctx, ns, pack->packageNS_onGlobalConstList, trace);
			if(pack->packageHandler != NULL && pack->packageHandler->ExportNameSpace != NULL) {
				pack->packageHandler->ExportNameSpace(kctx, pack->packageNS_onGlobalConstList, ns, option, trace);
			}
			if(pack->kickout_script != 0) {
				kNameSpace_LoadScript(kctx, ns, FileId_t(pack->kickout_script), trace);
			}
		}
	}
	return false;
}

static kbool_t kNameSpace_ImportPackageSymbol(KonohaContext *kctx, kNameSpace *ns, const char *name, ksymbol_t keyword, KTraceInfo *trace)
{
	kpackageId_t packageId = KLIB KpackageId(kctx, name, strlen(name), 0, _NEWID);
	int option = 0;
	if(ns->packageId != packageId) {
		KonohaPackage *pack = GetPackageNULL(kctx, packageId, option, trace);
		if(pack != NULL) {
			return kNameSpace_ImportSymbol(kctx, ns, pack->packageNS_onGlobalConstList, keyword, trace);
		}
	}
	return false;
}

// --------------------------------------------------------------------------

