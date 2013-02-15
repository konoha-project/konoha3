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

// --------------------------------------------------------------------------
/* ConstTable */

static KKeyValue *kNameSpace_GetLocalConstNULL(KonohaContext *kctx, kNameSpace *ns, ksymbol_t queryKey)
{
	KKeyValue *kvs = KLIB KDict_GetNULL(kctx, &(ns->constTable), queryKey);
	if(kvs != NULL && kvs->attrTypeId == VirtualType_Text) {
		const char *textData = (const char *)kvs->unboxValue;
		kvs->attrTypeId = KType_String | KTypeAttr_Boxed;
		kvs->StringValue = KLIB new_kString(kctx, ns->NameSpaceConstList, textData, strlen(textData), StringPolicy_TEXT);
		KLIB kArray_Add(kctx, ns->NameSpaceConstList, kvs->StringValue);
	}
	return kvs;
}

static void SetKeyValue(KonohaContext *kctx, KKeyValue *kv, ksymbol_t key, ktypeattr_t ty, uintptr_t unboxValue)
{
	kv->key = key;
	kv->unboxValue = unboxValue;
	if(KType_Is(UnboxType, ty) || ty == VirtualType_KClass || ty == VirtualType_StaticMethod || ty == VirtualType_Text) {
		kv->attrTypeId = ty;
	}
	else {
		kv->attrTypeId = ty | KTypeAttr_Boxed;
	}
}

static kbool_t kNameSpace_SetConstData(KonohaContext *kctx, kNameSpace *ns, ksymbol_t key, ktypeattr_t ty, uintptr_t unboxValue, KTraceInfo *trace)
{
	KKeyValue* foundKeyValue = kNameSpace_GetLocalConstNULL(kctx, ns, key);
	if(foundKeyValue == NULL) {
		KKeyValue kvs;
		SetKeyValue(kctx, &kvs, key, ty, unboxValue);
		KLIB KDict_Set(kctx, &(ns->constTable), &kvs);
		if(KTypeAttr_Is(Boxed, kvs.attrTypeId)) {
			KLIB kArray_Add(kctx, ns->NameSpaceConstList, kvs.ObjectValue);
		}
		return true;
	}
	else if(kNameSpace_Is(Override, ns)) {
		uintptr_t origUnboxValue = foundKeyValue->unboxValue;
		SetKeyValue(kctx, foundKeyValue, key, ty, unboxValue);
		if(KTypeAttr_Is(Boxed, foundKeyValue->attrTypeId) && origUnboxValue != unboxValue) {
			KLIB kArray_Add(kctx, ns->NameSpaceConstList, foundKeyValue->ObjectValue);
		}
		return true;
	}
	else {
		return false;
	}
}

static KKeyValue *kNameSpace_GetConstNULL(KonohaContext *kctx, kNameSpace *ns, ksymbol_t queryKey, int isLocalOnly)
{
	KKeyValue* foundKeyValue = kNameSpace_GetLocalConstNULL(kctx, ns, queryKey);
	if(foundKeyValue == NULL) {
		if(!isLocalOnly) {
			size_t i;
			for(i = 0; i < kArray_size(ns->importedNameSpaceList); i++) {
				foundKeyValue = kNameSpace_GetLocalConstNULL(kctx, ns->importedNameSpaceList->NameSpaceItems[i], queryKey);
				if(foundKeyValue != NULL) {
					return foundKeyValue;
				}
			}
		}
		if(ns->parentNULL != NULL) {
			return kNameSpace_GetConstNULL(kctx, ns->parentNULL, queryKey, isLocalOnly);
		}
	}
	return foundKeyValue;
}

//static kbool_t kNameSpace_MergeConstData(KonohaContext *kctx, kNameSpaceVar *ns, KKeyValue *kvs, size_t nitems, KTraceInfo *trace)
//{
//	size_t i;
//	for(i = 0; i < nitems; i++) {
//		if(KTypeAttr_Is(Boxed, kvs[i].attrTypeId)) {
//			KLIB kArray_Add(kctx, ns->NameSpaceConstList, kvs[i].ObjectValue);
//		}
//	}
//	KLIB KDict_MergeData(kctx, &(ns->constTable), kvs, nitems, kNameSpace_Is(Override, ns));
//	return true;
//}

static kbool_t kNameSpace_LoadConstData(KonohaContext *kctx, kNameSpace *ns, const char **d, KTraceInfo *trace)
{
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	while(d[0] != NULL) {
		KKeyValue kvs;
		SetKeyValue(kctx, &kvs, ksymbolSPOL(d[0], strlen(d[0]), StringPolicy_TEXT|StringPolicy_ASCII, _NEWID), (ktypeattr_t)(uintptr_t)d[1], (uintptr_t)d[2]);
		KLIB KBuffer_Write(kctx, &wb, (const char *)(&kvs), sizeof(KKeyValue));
		d += 3;
		if(KTypeAttr_Is(Boxed, kvs.attrTypeId)) {
			KLIB kArray_Add(kctx, ns->NameSpaceConstList, kvs.ObjectValue);
		}
	}
	size_t nitems = KBuffer_bytesize(&wb) / sizeof(KKeyValue);
	if(nitems > 0) {
		KLIB KDict_MergeData(kctx, &(ns->constTable), (KKeyValue *)KLIB KBuffer_text(kctx, &wb, 0), nitems, kNameSpace_Is(Override, ns));
	}
	KLIB KBuffer_Free(&wb);
	return true;
}

// boolean NameSpace.SetConst(Symbol symbol, Object value);
static KMETHOD NameSpace_DefineConst(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	ksymbol_t symbol = (ksymbol_t)sfp[1].intValue;
	KClass *c = kObject_class(sfp[2].asObject);
	uintptr_t unboxValue = KClass_Is(UnboxType, c) ? kObject_Unbox(sfp[2].asObject) : (uintptr_t)sfp[2].asObject;
	KReturnUnboxValue(kNameSpace_SetConstData(kctx, sfp[0].asNameSpace, symbol, c->typeId, unboxValue, trace));
}

// ---------------------------------------------------------------------------

static kSyntax *kNameSpace_GetSyntax(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword)
{
	KKeyValue *kvs = kNameSpace_GetConstNULL(kctx, ns, keyword, false/*isLocalOnly*/);
	if(kvs != NULL && KTypeAttr_Unmask(kvs->attrTypeId) == KType_Syntax) {
		//DBG_P(">>>>>>> ns=%p kvs=%p keyword=%s%s has defined syntax", ns, kvs, KSymbol_Fmt2(keyword));
		return (kSyntax *)kvs->ObjectValue;
	}
	//DBG_P(">>>>>>> ns=%p kvs=%p keyword=%s%s has no defined syntax", ns, kvs, KSymbol_Fmt2(keyword));
	return KNULL(Syntax);
}

static void kNameSpace_ListSyntax(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, kArray *a)
{
	ktypeattr_t tSyntax = KType_Syntax;
	while(ns != NULL) {
		size_t i;
		KKeyValue* foundKeyValue = kNameSpace_GetLocalConstNULL(kctx, ns, keyword);
		if(foundKeyValue != NULL && KTypeAttr_Unmask(foundKeyValue->attrTypeId) == tSyntax) {
			KLIB kArray_Add(kctx, a, foundKeyValue->ObjectValue);
		}
		for(i = 0; i < kArray_size(ns->importedNameSpaceList); i++) {
			foundKeyValue = kNameSpace_GetLocalConstNULL(kctx, ns->importedNameSpaceList->NameSpaceItems[i], keyword);
			if(foundKeyValue != NULL && KTypeAttr_Unmask(foundKeyValue->attrTypeId) == tSyntax) {
				KLIB kArray_Add(kctx, a, foundKeyValue->ObjectValue);
			}
		}
		ns = ns->parentNULL;
	}
}

static kArray *kNameSpace_GetSyntaxList(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword)
{
	ksymbol_t queryKey = keyword | KSymbolAttr_SyntaxList;
	KKeyValue* foundKeyValue = kNameSpace_GetLocalConstNULL(kctx, ns, queryKey);
	kArray *syntaxList = NULL;
	if(foundKeyValue != NULL) {
		syntaxList = (kArray *)foundKeyValue->ObjectValue;
		DBG_ASSERT(IS_Array(syntaxList));
		if(kArray_size(syntaxList) != 0) {
			return syntaxList;
		}
		/* recheck if size(syntaxList) == 0 */
	}
	if(syntaxList == NULL) {
		syntaxList = new_(Array, 0, NULL);
		kNameSpace_SetConstData(kctx, ns, queryKey, KType_Array, (uintptr_t)syntaxList, NULL);
	}
	kNameSpace_ListSyntax(kctx, ns, keyword, syntaxList);
	//DBG_P(">>>> new Syntax List=%s%s size=%d", KSymbol_Fmt2(keyword), kArray_size(syntaxList));
	return syntaxList;
}

static kbool_t kNameSpace_ResetSyntaxList(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword)
{
	ksymbol_t queryKey = keyword | KSymbolAttr_SyntaxList;
	KKeyValue* foundKeyValue = kNameSpace_GetLocalConstNULL(kctx, ns, queryKey);
	if(foundKeyValue != NULL) {
		kArray *syntaxList = (kArray *)foundKeyValue->ObjectValue;
		DBG_ASSERT(IS_Array(syntaxList));
		KLIB kArray_Clear(kctx, syntaxList, 0);
		return true;
	}
	return false;
}

static kFunc **kNameSpace_tokenFuncMatrix(KonohaContext *kctx, kNameSpace *ns);

static void kNameSpace_AddFuncList(KonohaContext *kctx, kNameSpace *ns, kArray **funcListTable, int index, kFunc *fo)
{
	kArray *a = funcListTable[index];
	KLIB kArray_Add(kctx, ns->NameSpaceConstList, fo);
	if(a == NULL) {
		funcListTable[index] = (kArray *)fo;
		return;
	}
	else if(!IS_Array(a)) {
		kArray *newa = new_(Array, 0, ns->NameSpaceConstList);
		funcListTable[index] = newa;
		KLIB kArray_Add(kctx, newa, a);
		a = newa;
	}
	KLIB kArray_Add(kctx, a, fo);
}

static void kNameSpace_SetTokenFuncMatrix(KonohaContext *kctx, kNameSpace *ns, int konohaChar, kFunc *fo)
{
	kArray **list = (kArray**)kNameSpace_tokenFuncMatrix(kctx, ns);
	kNameSpace_AddFuncList(kctx, ns, list, konohaChar, fo);
	KLIB kMethod_DoLazyCompilation(kctx, (fo)->method, NULL, HatedLazyCompile);
}

static void kNameSpace_ImportSyntax2(KonohaContext *kctx, kNameSpace *ns, kSyntax *syn)
{
	if(kSyntax_Is(MetaPattern, syn)) {
		if(ns->metaPatternList == K_EMPTYARRAY) {
			ns->metaPatternList = new_(Array, 0, ns->NameSpaceConstList);
		}
		KLIB kArray_Add(kctx, ns->metaPatternList, syn);
	}
	if(syn->tokenKonohaChar > 0) {
		kNameSpace_SetTokenFuncMatrix(kctx, ns, syn->tokenKonohaChar, syn->TokenFuncNULL);
	}
	kNameSpace_ResetSyntaxList(kctx, ns, syn->keyword);
}

static void kNameSpace_ImportSyntaxAsKeyValue(KonohaContext *kctx, void *arg, KKeyValue *kvs)
{
	kNameSpace *ns = (kNameSpace *) arg;
	if(KTypeAttr_Unmask(kvs->attrTypeId) == KType_Syntax) {
		kNameSpace_ImportSyntax2(kctx, ns, (kSyntax *)kvs->ObjectValue);
	}
}

static kbool_t kNameSpace_ImportSyntaxAll(KonohaContext *kctx, kNameSpace *ns, kNameSpace *targetNS, KTraceInfo *trace)
{
	if(ns->importedNameSpaceList == K_EMPTYARRAY) {
		ns->importedNameSpaceList = new_(Array, 0, ns->NameSpaceConstList);
	}
	KLIB kArray_Add(kctx, ns->importedNameSpaceList, targetNS);
	KLIB KDict_DoEach(kctx, &(targetNS->constTable), ns, kNameSpace_ImportSyntaxAsKeyValue);
	return true;
}

static kbool_t kNameSpace_RemoveSyntax(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, KTraceInfo *trace)
{
	if(kNameSpace_ResetSyntaxList(kctx, ns, keyword)) {
		//TODO
		return kNameSpace_SetConstData(kctx, ns, keyword, KType_Symbol, (uintptr_t)KNULL(Syntax), trace);
	}
	return true;
}

static void kNameSpace_AddSyntax(KonohaContext *kctx, kNameSpace *ns, kSyntax *syn, KTraceInfo *trace)
{
	if(kNameSpace_SetConstData(kctx, ns, syn->keyword, KType_Syntax, (uintptr_t)syn, trace)) {
		kNameSpace_ImportSyntax2(kctx, ns, syn);
	}
}

static void kNameSpace_DefineSyntax(KonohaContext *kctx, kNameSpace *ns, KDEFINE_SYNTAX *syndef, KTraceInfo *trace)
{
	while(syndef->keyword != KSymbol_END) {
		kSyntaxVar *syn = new_(SyntaxVar, ns, ns->NameSpaceConstList);
		syn->keyword = syndef->keyword;
		syn->packageNameSpace = ns;
		syn->flag = ((kshortflag_t)syndef->flag);
		if(syndef->precedence_op1 > 0) {
			syn->precedence_op1 = syndef->precedence_op1;
		}
		if(syndef->precedence_op2 > 0) {
			syn->precedence_op2 = syndef->precedence_op2;
		}
		if(syndef->parseFunc != NULL) {
			kFunc *fo = (KFlag_Is(kshortflag_t, syndef->flag, SYNFLAG_CParseFunc)) ? KSugarFunc(ns, syndef->parseMethodFunc) : syndef->parseFunc;
			DBG_ASSERT(IS_Func(fo));
			KFieldInit(ns, syn->ParseFuncNULL, fo);
		}
		if(syndef->typeFunc != NULL) {
			kFunc *fo = (KFlag_Is(kshortflag_t, syndef->flag, SYNFLAG_CTypeFunc)) ? KSugarFunc(ns, syndef->typeMethodFunc) : syndef->typeFunc;
			DBG_ASSERT(IS_Func(fo));
			KFieldInit(ns, syn->TypeFuncNULL, fo);
		}
		if(syndef->tokenChar != 0) {
			syn->tokenKonohaChar = syndef->tokenChar;
			DBG_ASSERT(syndef->tokenFunc != NULL);
			kFunc *fo = (KFlag_Is(kshortflag_t, syndef->flag, SYNFLAG_CTokenFunc)) ? KSugarFunc(ns, syndef->tokenMethodFunc) : syndef->tokenFunc;
			DBG_ASSERT(IS_Func(fo));
			KFieldInit(ns, syn->TokenFuncNULL, fo);
		}
		KLIB ReportScriptMessage(kctx, trace, DebugTag, "@%s new syntax %s%s", KPackage_text(ns->packageId), KSymbol_Fmt2(syn->keyword));
		kNameSpace_AddSyntax(kctx, ns, syn, trace);
		DBG_ASSERT(syn == kSyntax_(ns, syndef->keyword));
		syndef++;
	}
}

// ---------------------------------------------------------------------------
/* ClassName in ConstTable */

static KClass *kNameSpace_GetClassByFullName(KonohaContext *kctx, kNameSpace *ns, const char *name, size_t len, KClass *defaultClass)
{
	KClass *ct = NULL;
	kpackageId_t packageId = PN_konoha;
	ksymbol_t  un = KSymbol_Noname;
	const char *p = strrchr(name, '.');
	if(p == NULL) {
		un = KAsciiSymbol(name, len, KSymbol_Noname);
	}
	else {
		size_t plen = p - name;
		un = KAsciiSymbol(name + (plen+1), len - (plen+1), KSymbol_Noname);
		packageId = KLIB KpackageId(kctx, name, plen, 0, KSymbol_Noname);
	}
	if(packageId != KSymbol_Noname) {
		KKeyValue *kvs = kNameSpace_GetConstNULL(kctx, ns, un, false/*isLocalOnly*/);
		if(kvs != NULL && KTypeAttr_Unmask(kvs->attrTypeId) == VirtualType_KClass) {
			return (KClass *)kvs->unboxValue;
		}
	}
	return (ct != NULL) ? ct : defaultClass;
}

static KClass *kNameSpace_DefineClass(KonohaContext *kctx, kNameSpace *ns, kString *name, KDEFINE_CLASS *cdef, KTraceInfo *trace)
{
	KClass *ct = KLIB KClass_define(kctx, ns->packageId, name, cdef, trace);
	if(!KLIB kNameSpace_SetConstData(kctx, ns, ct->classNameSymbol, VirtualType_KClass, (uintptr_t)ct, trace)) {
		return NULL;
	}
	return ct;
}

// ---------------------------------------------------------------------------
/* Method Management */

static inline intptr_t Method_id(kMethod *mtd)
{
	intptr_t id = mtd->typeId;
	return (id << (sizeof(kshort_t)*8)) | mtd->mn;
}

static int comprMethod(const void *a, const void *b)
{
	intptr_t aid = Method_id(((kMethod**)a)[0]);
	intptr_t bid = Method_id(((kMethod**)b)[0]);
	if(aid == bid) return 0;
	return aid < bid ? -1 : 1;
}

static void kMethodList_MatchMethod(KonohaContext *kctx, kArray *methodList, const intptr_t *sorted, ktypeattr_t typeId, KMethodMatchFunc MatchMethod, KMethodMatch *option)
{
	intptr_t i, min = 0, max = sorted[0];
	intptr_t optkey = ((intptr_t)typeId << (sizeof(kshort_t)*8)) | option->mn;
	if(kArray_size(methodList) - max > 8) {
		max = kArray_size(methodList);
		PLATAPI qsort_i(methodList->MethodItems, max, sizeof(kMethod *), comprMethod);
		((size_t *)sorted)[0] = max;
	}
	while(min < max) {
		size_t p = (max + min) / 2;
		kMethod *mtd = methodList->MethodItems[p];
		intptr_t key = Method_id(mtd);
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
	for(i = sorted[0]; i < (intptr_t) kArray_size(methodList); i++) {
		kMethod *mtd = methodList->MethodItems[i];
		intptr_t key = Method_id(mtd);
		if(key == optkey) {
			MatchMethod(kctx, mtd, option);
		}
	}
}

static kMethod* kNameSpace_MatchMethodNULL(KonohaContext *kctx, kNameSpace *startNameSpace, KClass *ct, KMethodMatchFunc MatchMethod, KMethodMatch *option)
{
	while(ct != NULL) {
		kNameSpace *ns = startNameSpace;
		while(ns != NULL) {
			kMethodList_MatchMethod(kctx, ns->methodList_OnList, (intptr_t *) &ns->sortedMethodList, ct->typeId, MatchMethod, option);
			if(option->isBreak) {
				return option->foundMethodNULL;
			}
			ns = ns->parentNULL;
		}
		kMethodList_MatchMethod(kctx, ct->classMethodList, (intptr_t *) &ct->sortedMethodList, ct->typeId, MatchMethod, option);
		if(option->isBreak) {
			return option->foundMethodNULL;
		}
		ct = ct->searchSuperMethodClassNULL;
	}
	return option->foundMethodNULL;
}

static kbool_t KMethodMatch_Func(KonohaContext *kctx, kMethod *mtd, KMethodMatch *m)
{
	if(m->foundMethodNULL != NULL) {
		if(m->foundMethodNULL->serialNumber > mtd->serialNumber) return false;
	}
	m->foundMethodNULL = mtd;
	m->isBreak = true;
	return true;
}


static kbool_t KMethodMatch_ParamSize(KonohaContext *kctx, kMethod *mtd, KMethodMatch *m)
{
	kParam *param = kMethod_GetParam(mtd);
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

static kbool_t KMethodMatch_Param0(KonohaContext *kctx, kMethod *mtd, KMethodMatch *m)
{
	kParam *param = kMethod_GetParam(mtd);
	if(param->psize == 0) {
		m->foundMethodNULL = mtd;
		m->isBreak = true;
		return true;
	}
	return false;
}

static kbool_t KMethodMatch_ParamNoCheck(KonohaContext *kctx, kMethod *mtd, KMethodMatch *m)
{
	m->foundMethodNULL = mtd;
	m->isBreak = true;
	return true;
}

static kbool_t KClass_Isa(KonohaContext *kctx, KClass *ct, KClass *t)
{
	return ct->isSubType(kctx, ct, t);
}

static kMethod *kNameSpace_GetCoercionMethodNULL(KonohaContext *kctx, kNameSpace *ns, KClass *c, KClass *toClass)
{
	KMethodMatch m = {};
	m.mn = KMethodName_To(toClass->typeId);
	m.paramsize = 0;
	kMethod *mtd = kNameSpace_MatchMethodNULL(kctx, ns, c, KMethodMatch_Param0, &m);
	DBG_P("finding cast %s => %s: %p", KClass_text(c), KClass_text(toClass), mtd);
	return mtd;
}

static kbool_t KMethodMatch_Signature(KonohaContext *kctx, kMethod *mtd, KMethodMatch *m)
{
	if(mtd->paramdom == m->paramdom) {
		m->isBreak = true;
		m->foundMethodNULL = mtd;
		return true;
	}
	if(m->param != NULL && m->foundMethodNULL == NULL) {
		kParam *param = kMethod_GetParam(mtd);
		if(param->psize == m->paramsize) {
			kushort_t i;
			for(i = 0; i < m->paramsize; i++) {
				KClass *mtype = KClass_(m->param[i].attrTypeId);
				KClass *ptype = KClass_(param->paramtypeItems[i].attrTypeId);
				if(mtype != ptype) {
					if(KClass_Isa(kctx, mtype, ptype)) {
						continue;
					}
					kMethod *castMethod = kNameSpace_GetCoercionMethodNULL(kctx, m->ns, mtype, ptype);
					if(castMethod != NULL && (kMethod_Is(Coercion, castMethod) || KTypeAttr_Is(Coercion, param->paramtypeItems[i].attrTypeId))) {
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

static kMethod *kNameSpace_GetNameSpaceFuncNULL(KonohaContext *kctx, kNameSpace *ns, ksymbol_t symbol, KClass *reqClass)
{
	KMethodMatch m = {};
	m.mn = symbol;
	if(reqClass->baseTypeId == KType_Func) {
		m.paramdom = reqClass->cparamdom;
		kNameSpace_MatchMethodNULL(kctx, ns, kObject_class(ns), KMethodMatch_Signature, &m);
		if(m.foundMethodNULL != NULL) {
			return m.foundMethodNULL;
		}
	}
	return kNameSpace_MatchMethodNULL(kctx, ns, kObject_class(ns), KMethodMatch_Func, &m);
}

static size_t CheckAnotherSymbol(KonohaContext *kctx, ksymbol_t *resolved, size_t foundNames, char *buffer, size_t len)
{
	resolved[foundNames] = KLIB Ksymbol(kctx, (const char *)buffer, len, 0, KSymbol_Noname);
	if(resolved[foundNames] != KSymbol_Noname) {
		DBG_P("CHECKING ANOTHER NAME: '%s'", buffer);
		foundNames++;
	}
	return foundNames;
}

#define ANOTHER_NAME_MAXSIZ 4

static size_t FindAnotherSymbol(KonohaContext *kctx, ksymbol_t symbol, ksymbol_t *resolved)
{
	const char *name = KSymbol_text(symbol);
	size_t i, foundNames = 0, len = kString_size(KSymbol_GetString(kctx, symbol));
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

static kMethod *kNameSpace_GetGetterMethodNULL(KonohaContext *kctx, kNameSpace *ns, KClass *c, ksymbol_t symbol)
{
	if(symbol != KSymbol_Noname) {
		KMethodMatch m = {};
		m.mn = KMethodName_ToGetter(symbol);
		m.paramsize = 0;
		kMethod *mtd = kNameSpace_MatchMethodNULL(kctx, ns, c, KMethodMatch_Param0, &m);
		if(mtd == NULL) {
			ksymbol_t anotherSymbols[ANOTHER_NAME_MAXSIZ];
			size_t i, foundNames = FindAnotherSymbol(kctx, symbol, anotherSymbols);
			for(i = 0; i < foundNames; i++) {
				m.mn = KMethodName_ToGetter(anotherSymbols[i]);
				mtd = kNameSpace_MatchMethodNULL(kctx, ns, c, KMethodMatch_Param0, &m);
				if(mtd != NULL) break;
			}
		}
		return mtd;
	}
	return NULL;
}

static kMethod *kNameSpace_GetSetterMethodNULL(KonohaContext *kctx, kNameSpace *ns, KClass *c, ksymbol_t symbol, ktypeattr_t type)
{
	if(symbol != KSymbol_Noname) {
		KMethodMatch m = {};
		m.mn = KMethodName_ToSetter(symbol);
		m.paramsize = 1;
		KMethodMatchFunc func;
		if(type == KType_var) {
			func = KMethodMatch_ParamSize;
		}
		else {
			kparamtype_t p = {type};
			m.paramdom = KLIB Kparamdom(kctx, 1, &p);
			func = KMethodMatch_Signature;
		}
		kMethod *mtd = kNameSpace_MatchMethodNULL(kctx, ns, c, func, &m);
		if(mtd == NULL) {
			ksymbol_t anotherSymbols[ANOTHER_NAME_MAXSIZ];
			size_t i, foundNames = FindAnotherSymbol(kctx, symbol, anotherSymbols);
			for(i = 0; i < foundNames; i++) {
				m.mn = KMethodName_ToSetter(anotherSymbols[i]);
				mtd = kNameSpace_MatchMethodNULL(kctx, ns, c, func, &m);
				if(mtd != NULL) break;
			}
		}
		return mtd;
	}
	return NULL;
}

static kMethod *kNameSpace_GetMethodByParamSizeNULL(KonohaContext *kctx, kNameSpace *ns, KClass *c, ksymbol_t symbol, int paramsize, KMethodMatchOption option)
{
	KMethodMatch m = {};
	m.mn = symbol;
	m.paramsize = paramsize;
	KMethodMatchFunc func = paramsize == 0 ? KMethodMatch_Param0 : KMethodMatch_ParamSize;
	if(paramsize == -1) func = KMethodMatch_ParamNoCheck;
	kMethod *mtd = kNameSpace_MatchMethodNULL(kctx, ns, c, func, &m);
	if(mtd == NULL && KFlag_Is(int, option, KMethodMatch_CamelStyle)) {
		ksymbol_t attr = KSymbol_Attr(symbol);
		ksymbol_t anotherSymbols[ANOTHER_NAME_MAXSIZ];
		size_t i, foundNames = FindAnotherSymbol(kctx, KSymbol_Unmask(symbol), anotherSymbols);
		for(i = 0; i < foundNames; i++) {
			m.mn = anotherSymbols[i] | attr;
			mtd = kNameSpace_MatchMethodNULL(kctx, ns, c, func, &m);
			if(mtd != NULL) break;
		}
	}
	return mtd;
}

static kMethod *kNameSpace_GetMethodToCheckOverloadNULL(KonohaContext *kctx, kNameSpace *ns, KClass *c, ksymbol_t symbol, int paramsize)
{
	KMethodMatch m = {};
	m.mn = symbol;
	m.paramsize = paramsize;
	KMethodMatchFunc func = paramsize == 0 ? KMethodMatch_Param0 : KMethodMatch_ParamSize;
	if(paramsize == -1) func = KMethodMatch_ParamNoCheck;
	return kNameSpace_MatchMethodNULL(kctx, ns, c, func, &m);
}

static kMethod *kNameSpace_GetMethodBySignatureNULL(KonohaContext *kctx, kNameSpace *ns, KClass *c, ksymbol_t symbol, int paramdom, int paramsize, kparamtype_t *param)
{
	KMethodMatch m = {};
	m.ns = ns;
	m.mn = symbol;
	m.paramdom = paramdom;
	m.paramsize = paramsize;
	m.param = param;
	return kNameSpace_MatchMethodNULL(kctx, ns, c, KMethodMatch_Signature, &m);
}

// ---------------------------------------------------------------------------

static void kMethod_ReplaceWith(KonohaContext *kctx, kMethodVar *oldMethod, kMethodVar *newMethod)
{
	kMethodVar tempMethod;
	tempMethod = *oldMethod;
	*oldMethod = *newMethod;
	*newMethod = tempMethod;
}

static kMethod *kNameSpace_AddMethod(KonohaContext *kctx, kNameSpace *ns, kMethodVar *mtd, KTraceInfo *trace)
{
	KClass *ct = KClass_(mtd->typeId);
	if(mtd->packageId == 0) {
		((kMethodVar *)mtd)->packageId = ns->packageId;
		TRACE_ReportScriptMessage(kctx, trace, DebugTag, "@%s loading method %s.%s%s", KPackage_text(ns->packageId), kMethod_Fmt3(mtd));
	}
	if(KClass_Is(Final, ct)) {
		kMethod_Set(Final, mtd, true);
	}
	kMethod *foundMethod = kNameSpace_GetMethodBySignatureNULL(kctx, ns, ct, mtd->mn, mtd->paramdom, 0, NULL);
	if(foundMethod != NULL) {  // same signature
		if(foundMethod->typeId == mtd->typeId) {
			if(kMethod_Is(Override, mtd)) {
				TRACE_ReportScriptMessage(kctx, trace, DebugTag, "@%s overriding method %s.%s%s on %s", KPackage_text(ns->packageId), kMethod_Fmt3(mtd), KPackage_text(foundMethod->packageId));
				kMethod_ReplaceWith(kctx, (kMethodVar *)foundMethod, mtd);
				return foundMethod;
			}
			else {
				TRACE_ReportScriptMessage(kctx, trace, ErrTag, "duplicated method: %s.%s%s on %s", kMethod_Fmt3(foundMethod), KPackage_text(foundMethod->packageId));
			}
			return NULL;
		}
		else {
			if(!kMethod_Is(Final, foundMethod)) {
				TRACE_ReportScriptMessage(kctx, trace, DebugTag, "@%s overriding method %s.%s%s on %s.%s%s", KPackage_text(ns->packageId), kMethod_Fmt3(mtd), kMethod_Fmt3(foundMethod));
				kMethod_Set(Virtual, ((kMethodVar *)foundMethod), true);
			}
			else {
				TRACE_ReportScriptMessage(kctx, trace, ErrTag, "final method: %s.%s%s", kMethod_Fmt3(foundMethod));
				return NULL;
			}
		}
	}
	else {
		foundMethod = kNameSpace_GetMethodToCheckOverloadNULL(kctx, ns, ct, mtd->mn, kMethod_ParamSize(mtd));
		if(foundMethod != NULL && foundMethod->mn == mtd->mn) {
			kMethod_Set(Overloaded, ((kMethodVar *)foundMethod), true);
			kMethod_Set(Overloaded, mtd, true);
		}
	}
	if(kMethod_Is(Public, mtd)) {
		if(unlikely(ct->classMethodList == K_EMPTYARRAY)) {
			((KClassVar *)ct)->classMethodList = new_(MethodArray, 8, OnGlobalConstList);
		}
		KLIB kArray_Add(kctx, ct->classMethodList, mtd);
	}
	else {
		if(ns->methodList_OnList == K_EMPTYARRAY) {
			((kNameSpaceVar *)ns)->methodList_OnList = new_(MethodArray, 8, ns->NameSpaceConstList);
		}
		KLIB kArray_Add(kctx, ns->methodList_OnList, mtd);
	}
	return mtd;
}

static void kNameSpace_LoadMethodData(KonohaContext *kctx, kNameSpace *ns, intptr_t *data, KTraceInfo *trace)
{
	intptr_t *d = data;
	INIT_GCSTACK();
	while(d[0] != -1) {
		uintptr_t flag = (uintptr_t)d[0];
		KMethodFunc f = (KMethodFunc)d[1];
		ktypeattr_t rtype = (ktypeattr_t)d[2];
		ktypeattr_t cid  = (ktypeattr_t)d[3];
		kmethodn_t mn = (kmethodn_t)d[4];
		size_t i, psize = (size_t)d[5];
		kparamtype_t *p = ALLOCA(kparamtype_t, psize+1);
		d = d + 6;
		for(i = 0; i < psize; i++) {
			p[i].attrTypeId = (ktypeattr_t)d[0];
			p[i].name       = (ksymbol_t)d[1];
			d += 2;
		}
		kMethodVar *mtd = KLIB new_kMethod(kctx, _GcStack, flag, cid, mn, f);
		KLIB kMethod_SetParam(kctx, mtd, rtype, psize, p);
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

static kfileline_t uline_Init(KonohaContext *kctx, const char *path, int line, int isreal)
{
	kfileline_t uline = line;
	uline |= KLIB KfileId(kctx, path, strlen(path), 0, _NEWID);
	return uline;
}

static kbool_t kNameSpace_LoadScript(KonohaContext *kctx, kNameSpace *ns, const char *path, KTraceInfo *trace)
{
	SugarThunk thunk = {kctx, ns, trace};
	kfileline_t uline = uline_Init(kctx, path, 1, true/*isRealPath*/);
	if(!(PLATAPI loadScript(path, uline, (void *)&thunk, evalHookFunc))) {
		KLIB ReportScriptMessage(kctx, trace, ErrTag, "failed to load script: %s", path);
		return false;
	}
	if(KonohaContext_Is(CompileOnly, kctx)) {
		size_t i, size = kArray_size(KGetParserContext(kctx)->definedMethodList);
		for (i = 0; i < size; ++i) {
			kMethod *mtd = KGetParserContext(kctx)->definedMethodList->MethodItems[i];
			KLIB kMethod_DoLazyCompilation(kctx, mtd, NULL, DefaultCompileOption);
		}
		KLIB kArray_Clear(kctx, KGetParserContext(kctx)->definedMethodList, 0);
	}
	return true;
}

// ---------------------------------------------------------------------------
// Package Management */

#define KClass_NameSpaceVar KClass_NameSpace
static kNameSpace *new_PackageNameSpace(KonohaContext *kctx, kpackageId_t packageId)
{
	kNameSpaceVar *ns = new_(NameSpaceVar, KNULL(NameSpace), OnGlobalConstList);
	ns->packageId = packageId;
	return (kNameSpace *)ns;
}

static KPackage *LoadPackageNULL(KonohaContext *kctx, kpackageId_t packageId, int option, KTraceInfo *trace)
{
	const char *packageName = KPackage_text(packageId);
	char packupbuf[256], kickbuf[256];
	const char *path = PLATAPI FormatPackagePath(kctx, packupbuf, sizeof(packupbuf), packageName, "_packup.k");
	if(path == NULL) {
		path = PLATAPI FormatPackagePath(kctx, packupbuf, sizeof(packupbuf), packageName, "_glue.k");
	}
	const char *kickpath = PLATAPI FormatPackagePath(kctx, kickbuf, sizeof(kickbuf), packageName, "_kick.k");
	KPackageHandler *packageHandler = PLATAPI LoadPackageHandler(kctx, packageName);
	if(path == NULL && packageHandler == NULL && kickpath == NULL) {
		DBG_ASSERT(trace != NULL);
		KBeginCritical(trace, "PackageNotFound", SoftwareFault|SystemFault);
		PLATAPI FormatPackagePath(kctx, packupbuf, sizeof(packupbuf), packageName, "");
		KLIB ReportScriptMessage(kctx, trace, ErrTag, "package not found: path=%s", packupbuf);
		KEndCritical(trace);
		return NULL;
	}
	else {
		KPackage *pack = (KPackage *)KCalloc(sizeof(KPackage), 1, trace);
		pack->packageId = packageId;
		pack->packageHandler = packageHandler;
		if(kickpath != NULL) {
			pack->kickout_script = uline_Init(kctx, kickpath, 1, true/*isRealPath*/);
		}
		KLock(kctx->share->filepackMutex);
		map_Addu(kctx, kctx->share->packageMapNO, packageId, (uintptr_t)pack);
		KUnlock(kctx->share->filepackMutex);
		//
		DBG_ASSERT(pack->packageNS == NULL);
		kNameSpace *ns = new_PackageNameSpace(kctx, packageId);
		KBeginCritical(trace, "PackageLoading", SoftwareFault|SystemFault);
		if(packageHandler != NULL && packageHandler->PackupNameSpace != NULL) {
			packageHandler->PackupNameSpace(kctx, ns, option, trace);
		}
		if(path != NULL) {
			kNameSpace_LoadScript(kctx, ns, packupbuf, trace);
		}
		KEndCritical(trace);
		pack->packageNS = ns;
		return pack;
	}
}

static KPackage *GetPackageNULL(KonohaContext *kctx, kpackageId_t packageId, int option, KTraceInfo *trace)
{
	KLock(kctx->share->filepackMutex);
	KPackage *pack = (KPackage *)map_getu(kctx, kctx->share->packageMapNO, packageId, 0);
	KUnlock(kctx->share->filepackMutex);
	if(pack == NULL) {
		pack = LoadPackageNULL(kctx, packageId, option, trace);
		if(pack == NULL) return NULL;
	}
	else if(pack->packageNS == NULL) {
		KLIB ReportScriptMessage(kctx, trace, ErrTag, "recursive importing: %s", KPackage_text(packageId));
		return NULL;
	}
	return pack;
}

//static kbool_t kNameSpace_ImportSymbol(KonohaContext *kctx, kNameSpace *ns, kNameSpace *packageNS, ksymbol_t keyword, KTraceInfo *trace)
//{
//	kSyntax *syn = kSyntax_(packageNS, keyword);
//	if(syn != NULL) {
//		return kNameSpace_ImportSyntax(kctx, ns, syn, trace);
//	}
//	else {
//		KKeyValue *kvs = kNameSpace_GetLocalConstNULL(kctx, packageNS, keyword);
//		if(kvs != NULL) {
//			if(kNameSpace_MergeConstData(kctx, (kNameSpaceVar *)ns, kvs, 1, trace)) {
//				if(KTypeAttr_Unmask(kvs->attrTypeId) == VirtualType_KClass) {
//					size_t i;
//					ktypeattr_t typeId = ((KClass *)kvs->unboxValue)->typeId;
//					for(i = 0; i < kArray_size(packageNS->methodList_OnList); i++) {
//						kMethod *mtd = packageNS->methodList_OnList->MethodItems[i];
//						if(mtd->typeId == typeId /*&& !kMethod_Is(Private, mtd)*/) {
//							KLIB kArray_Add(kctx, ns->methodList_OnList, mtd);
//						}
//					}
//				}
//				return true;
//			}
//		}
//	}
//	return false;
//}

//static kbool_t kNameSpace_isImported(KonohaContext *kctx, kNameSpace *ns, kNameSpace *packageNS, KTraceInfo *trace)
//{
//	KKeyValue *value = kNameSpace_GetLocalConstNULL(kctx, ns, packageNS->packageId | KSymbolAttr_SyntaxList | KSymbolAttr_Pattern);
//	if(value != NULL) {
//		KLIB ReportScriptMessage(kctx, trace, DebugTag, "package %s has already imported in %s", KPackage_text(ns->packageId), KPackage_text(packageNS->packageId));
//		return true;
//	}
//	return false;
//}

static kbool_t kNameSpace_ImportAll(KonohaContext *kctx, kNameSpace *ns, kNameSpace *packageNS, KTraceInfo *trace)
{
	size_t i;
	for(i = 0; i < kArray_size(ns->importedNameSpaceList); i++) {
		kNameSpace *importedNS = ns->importedNameSpaceList->NameSpaceItems[i];
		if(importedNS == packageNS) {
			KLIB ReportScriptMessage(kctx, trace, DebugTag, "package %s has already imported in %s", KPackage_text(ns->packageId), KPackage_text(packageNS->packageId));
			return false;
		}
	}
	kNameSpace_ImportSyntaxAll(kctx, ns, packageNS, trace);
//		for(i = 0; i < kArray_size(packageNS->methodList_OnList); i++) {
//			kMethod *mtd = packageNS->methodList_OnList->MethodItems[i];
//			if(kMethod_Is(Public, mtd) && mtd->packageId == packageNS->packageId) {
//				KLIB kArray_Add(kctx, ns->methodList_OnList, mtd);
//			}
//		}
	return true;
}

static KPackage *kNameSpace_RequirePackage(KonohaContext *kctx, const char *name, KTraceInfo *trace)
{
	kpackageId_t packageId = KLIB KpackageId(kctx, name, strlen(name), 0, _NEWID);
	return GetPackageNULL(kctx, packageId, 0, trace);
}

static kbool_t kNameSpace_ImportPackage(KonohaContext *kctx, kNameSpace *ns, const char *name, KTraceInfo *trace)
{
	kpackageId_t packageId = KLIB KpackageId(kctx, name, strlen(name), 0, _NEWID);
	int option = 0;
	if(ns->packageId != packageId) {
		KPackage *pack = GetPackageNULL(kctx, packageId, option, trace);
		if(pack != NULL) {
			pack->packageNS->builderApi = ns->builderApi;
			kNameSpace_ImportAll(kctx, ns, pack->packageNS, trace);
			if(pack->packageHandler != NULL && pack->packageHandler->ExportNameSpace != NULL) {
				pack->packageHandler->ExportNameSpace(kctx, pack->packageNS, ns, option, trace);
			}
			if(pack->kickout_script != 0) {
				kNameSpace_LoadScript(kctx, ns, KFileLine_textFileName(pack->kickout_script), trace);
			}
		}
	}
	return false;
}

//static kbool_t kNameSpace_ImportPackageSymbol(KonohaContext *kctx, kNameSpace *ns, const char *name, ksymbol_t keyword, KTraceInfo *trace)
//{
//	kpackageId_t packageId = KLIB KpackageId(kctx, name, strlen(name), 0, _NEWID);
//	int option = 0;
//	if(ns->packageId != packageId) {
//		KPackage *pack = GetPackageNULL(kctx, packageId, option, trace);
//		if(pack != NULL) {
//			return kNameSpace_ImportSymbol(kctx, ns, pack->packageNS, keyword, trace);
//		}
//	}
//	return false;
//}

// --------------------------------------------------------------------------

static void kNameSpace_UseDefaultVirtualMachine(KonohaContext *kctx, kNameSpace *ns)
{
	KonohaFactory *factory = (KonohaFactory *)kctx->platApi;
	factory->LoadPlatformModule(factory, "MiniVM", ReleaseModule);
	ns->builderApi = factory->ExecutionEngineModule.GetDefaultBuilderAPI();
}

// --------------------------------------------------------------------------
/* namespace method */

