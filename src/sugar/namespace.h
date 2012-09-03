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
// Syntax Management

static void kNameSpace_parseSugarRule2(KonohaContext *kctx, kNameSpace *ns, const char *rule, kfileline_t uline, kArray *ruleList);

//static void copyIfArray(KonohaContext *kctx, kNameSpace *ns, kArray **arrayRef)
//{
//	DBG_P(">>>>>>> arrayRef[0]=%p", arrayRef[0]);
//	if(arrayRef[0] != NULL) {
//		DBG_P(">>>>>> arrayRef[0]=%p, %s", arrayRef[0], CT_t(O_ct(arrayRef[0])));
//		if(IS_Array(arrayRef[0])) {
//			int i;
//			kArray *newa = GCSAFE_new(Array, kArray_size(arrayRef[0]));
//			for(i = 0; i < kArray_size(arrayRef[0]); i++) {
//				KLIB kArray_add(kctx, newa, arrayRef[0]->objectItems[i]);
//			}
//			KSETv(ns, arrayRef[0], newa);
//		}
//	}
//}

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
		kreportf(DebugTag, 0, "redefining syntax %s%s on NameSpace=%p, syntax=%p, parent=%p", PSYM_t(keyword), ns, syn, parentSyntax);
		syn->ty             = parentSyntax->ty;
		syn->precedence_op1 = parentSyntax->precedence_op1;
		syn->precedence_op2 = parentSyntax->precedence_op2;
	}
	else {
		kreportf(DebugTag, 0, "new syntax %s%s on NameSpace=%p, syntax=%p, parent=%p", PSYM_t(keyword), ns, syn, parentSyntax);
		syn->ty             = TY_unknown;
		syn->precedence_op1 = 0;
		syn->precedence_op2 = 0;
	}
	return syn;
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

static void kNameSpace_setSugarFunc(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, size_t idx, kFunc *fo)
{
	assert(idx < SUGARFUNC_SIZE);
	SugarSyntaxVar *syn = (SugarSyntaxVar *)kNameSpace_getSyntax(kctx, ns, keyword, 1/*new*/);
	KINITSETv(ns, syn->sugarFuncTable[idx], fo);
}

static void kNameSpace_addSugarFunc(KonohaContext *kctx, kNameSpace *ns, ksymbol_t keyword, size_t idx, kFunc *funcObject)
{
	SugarSyntaxVar *syn = (SugarSyntaxVar *)kNameSpace_getSyntax(kctx, ns, keyword, 1/*new*/);
	DBG_ASSERT(idx < SUGARFUNC_SIZE);
	if(syn->sugarFuncTable[idx] == NULL) {
		KINITv(syn->sugarFuncTable[idx], funcObject);
		return;
	}
	if(IS_Func(syn->sugarFuncTable[idx])) {
		PUSH_GCSTACK(funcObject);
		kArray *a = GCSAFE_new(Array, 0);
		KLIB kArray_add(kctx, a, syn->sugarFuncTable[idx]);
		KSETv(ns, syn->sugarFuncListTable[idx], a);
	}
	DBG_ASSERT(IS_Array(syn->sugarFuncListTable[idx]));
	KLIB kArray_add(kctx, syn->sugarFuncListTable[idx], funcObject);
}

static void SugarSyntax_setSugarFunc(KonohaContext *kctx, SugarSyntaxVar *syn, MethodFunc definedMethodFunc, size_t index, MethodFunc *previousDefinedFuncRef, kFunc **cachedFuncRef)
{
	if(definedMethodFunc != NULL) {
		if(definedMethodFunc != previousDefinedFuncRef[0]) {
			previousDefinedFuncRef[0] = definedMethodFunc;
			cachedFuncRef[0] = new_SugarFunc(definedMethodFunc);
		}
		KINITv(syn->sugarFuncTable[index], cachedFuncRef[0]);
	}
}

static void kNameSpace_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KDEFINE_SYNTAX *syndef)
{
	MethodFunc pPatternMatch = NULL, pParseExpr = NULL, pStmtTyCheck = NULL, pExprTyCheck = NULL;
	kFunc *mPatternMatch = NULL, *mParseExpr = NULL, *mStmtTyCheck = NULL, *mExprTyCheck = NULL;
	while(syndef->keyword != KW_END) {
		SugarSyntaxVar* syn = (SugarSyntaxVar*)kNameSpace_getSyntax(kctx, ns, syndef->keyword, 1/*isnew*/);
		DBG_ASSERT(syn != NULL);
		DBG_P("keyword='%s%s', syn=%p, syn->parent=%p", PSYM_t(syndef->keyword), syn, syn->parentSyntaxNULL);
		syn->flag  |= ((kshortflag_t)syndef->flag);
		if(syndef->type != 0) {
			syn->ty = syndef->type;
		}
		if(syndef->precedence_op1 > 0) {
			syn->precedence_op1 = syndef->precedence_op1;
		}
		if(syndef->precedence_op2 > 0) {
			syn->precedence_op2 = syndef->precedence_op2;
		}
		if(syndef->rule != NULL) {
			KINITv(syn->syntaxRuleNULL, new_(TokenArray, 0));
			kNameSpace_parseSugarRule2(kctx, ns, syndef->rule, 0, syn->syntaxRuleNULL);
		}
		SugarSyntax_setSugarFunc(kctx, syn, syndef->PatternMatch,   SUGARFUNC_PatternMatch,   &pPatternMatch, &mPatternMatch);
		SugarSyntax_setSugarFunc(kctx, syn, syndef->ParseExpr,      SUGARFUNC_ParseExpr,      &pParseExpr, &mParseExpr);
		SugarSyntax_setSugarFunc(kctx, syn, syndef->TopStmtTyCheck, SUGARFUNC_TopStmtTyCheck, &pStmtTyCheck, &mStmtTyCheck);
		SugarSyntax_setSugarFunc(kctx, syn, syndef->StmtTyCheck,    SUGARFUNC_StmtTyCheck,    &pStmtTyCheck, &mStmtTyCheck);
		SugarSyntax_setSugarFunc(kctx, syn, syndef->ExprTyCheck,    SUGARFUNC_ExprTyCheck,    &pExprTyCheck, &mExprTyCheck);
		// set default function
		if(syn->parentSyntaxNULL == NULL && syn->sugarFuncTable[SUGARFUNC_ParseExpr] == NULL) {
			if(syn->precedence_op2 > 0 || syn->precedence_op1 > 0) {
				kFunc *fo = SYN_(ns, KW_ExprOperator)->sugarFuncTable[SUGARFUNC_ParseExpr];
				DBG_ASSERT(fo != NULL);
				DBG_P("syn->keyword=%s%s, fo=%p", PSYM_t(syn->keyword), fo);
				KINITv(syn->sugarFuncTable[SUGARFUNC_ParseExpr], fo);
			}
			else if(syn->ty != TY_unknown || syn->sugarFuncTable[SUGARFUNC_ExprTyCheck] != NULL) {
				kFunc *fo = SYN_(ns, KW_ExprTerm)->sugarFuncTable[SUGARFUNC_ParseExpr];
				DBG_ASSERT(fo != NULL);
				DBG_P("syn->keyword=%s%s, fo=%p", PSYM_t(syn->keyword), fo);
				KINITv(syn->sugarFuncTable[SUGARFUNC_ParseExpr], fo);
			}
		}
		DBG_ASSERT(syn == SYN_(ns, syndef->keyword));
		syndef++;
	}
}

// --------------------------------------------------------------------------
// ConstTable

static int comprKeyVal(const void *a, const void *b)
{
	int akey = SYMKEY_unbox(((KUtilsKeyValue*)a)->key);
	int bkey = SYMKEY_unbox(((KUtilsKeyValue*)b)->key);
	return akey - bkey;
}

static KUtilsKeyValue* kNameSpace_getLocalConstNULL(KonohaContext *kctx, kNameSpace *ns, ksymbol_t unboxKey)
{
	size_t min = 0, max = ns->constTable.bytesize / sizeof(KUtilsKeyValue);
	while(min < max) {
		size_t p = (max + min) / 2;
		ksymbol_t key = SYMKEY_unbox(ns->constTable.keyvalueItems[p].key);
		if(key == unboxKey) return ns->constTable.keyvalueItems + p;
		if((int)key < (int)unboxKey) {
			min = p + 1;
		}
		else {
			max = p;
		}
	}
	return NULL;
}

static KUtilsKeyValue* kNameSpace_getConstNULL(KonohaContext *kctx, kNameSpace *ns, ksymbol_t unboxKey)
{
	while(ns != NULL) {
		KUtilsKeyValue* foundKeyValue = kNameSpace_getLocalConstNULL(kctx, ns, unboxKey);
//		DBG_P("rootns=%p, ns=%p, key=%s, value=%p", KNULL(NameSpace), ns, SYM_t(SYMKEY_unbox(unboxKey)), foundKeyValue);
		if(foundKeyValue != NULL) return foundKeyValue;
		ns = ns->parentNULL;
	}
	return NULL;
}

static kbool_t checkLocalConflictedConstValue(KonohaContext *kctx, kNameSpace *ns, KUtilsKeyValue *kvs, kfileline_t pline)
{
	ksymbol_t unboxKey = kvs->key;
	KUtilsKeyValue* stored = kNameSpace_getLocalConstNULL(kctx, ns, unboxKey);
	if(stored != NULL) {
		if(kvs->ty == stored->ty && kvs->unboxValue == stored->unboxValue) {
			return true;  // same value
		}
		kreportf(WarnTag, pline, "conflicted name: %s", SYM_t(SYMKEY_unbox(unboxKey)));
		return true;
	}
	return false;
}

static kbool_t kNameSpace_mergeConstData(KonohaContext *kctx, kNameSpaceVar *ns, KUtilsKeyValue *kvs, size_t nitems, kfileline_t pline)
{
	size_t i, s = ns->constTable.bytesize / sizeof(KUtilsKeyValue);
	if(s == 0) {
		KLIB Karray_init(kctx, &ns->constTable, (nitems + 8) * sizeof(KUtilsKeyValue));
		memcpy(ns->constTable.keyvalueItems, kvs, nitems * sizeof(KUtilsKeyValue));
	}
	else {
		KUtilsWriteBuffer wb;
		KLIB Kwb_init(&(KonohaContext_getSugarContext(kctx)->errorMessageBuffer), &wb);
		for(i = 0; i < nitems; i++) {
			if(checkLocalConflictedConstValue(kctx, ns, kvs+i, pline)) continue;
			KLIB Kwb_write(kctx, &wb, (const char*)(kvs+i), sizeof(KUtilsKeyValue));
		}
		kvs = (KUtilsKeyValue*)KLIB Kwb_top(kctx, &wb, 0);
		nitems = Kwb_bytesize(&wb)/sizeof(KUtilsKeyValue);
		if(nitems > 0) {
			KLIB Karray_resize(kctx, &ns->constTable, (s + nitems + 8) * sizeof(KUtilsKeyValue));
			memcpy(ns->constTable.keyvalueItems + s, kvs, nitems * sizeof(KUtilsKeyValue));
		}
		KLIB Kwb_free(&wb);
	}
	nitems = s + nitems;
	ns->constTable.bytesize = nitems * sizeof(KUtilsKeyValue);
	if(nitems > 0) {
		PLATAPI qsort_i(ns->constTable.keyvalueItems, nitems, sizeof(KUtilsKeyValue), comprKeyVal);
	}
	return true;  // FIXME
}

static kbool_t kNameSpace_setConstData(KonohaContext *kctx, kNameSpace *ns, ksymbol_t key, ktype_t ty, uintptr_t unboxValue)
{
	KUtilsKeyValue kv;
	kv.key = key | SYMKEY_BOXED;
	kv.ty = ty;
	kv.unboxValue = unboxValue;
	if(ty == TY_TEXT) {
		const char *textData = (const char*)unboxValue;
		kv.ty = TY_String;
		kv.stringValue = KLIB new_kString(kctx, textData, strlen(textData), SPOL_TEXT);
		PUSH_GCSTACK(kv.objectValue);
	}
	else if(TY_isUnbox(kv.ty) || kv.ty == TY_TYPE) {
		kv.key = key;
	}
	return kNameSpace_mergeConstData(kctx, (kNameSpaceVar*)ns, &kv, 1, 0);
}

//static size_t strlen_alnum(const char *p)
//{
//	size_t len = 0;
//	while(isalnum(p[len]) || p[len] == '_') len++;
//	return len;
//}

static void kNameSpace_loadConstData(KonohaContext *kctx, kNameSpace *ns, const char **d, kfileline_t pline)
{
	INIT_GCSTACK();
	KUtilsKeyValue kv;
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	while(d[0] != NULL) {
		kv.key = ksymbolSPOL(d[0], strlen(d[0]), SPOL_TEXT|SPOL_ASCII, _NEWID) | SYMKEY_BOXED;
		DBG_P("key='%s', '%s%s'", d[0], PSYM_t(SYMKEY_unbox(kv.key)));
		kv.ty  = (ktype_t)(uintptr_t)d[1];
		if(kv.ty == TY_TEXT) {
			kv.ty = TY_String;
			kv.stringValue = KLIB new_kString(kctx, d[2], strlen(d[2]), SPOL_TEXT);
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
		kNameSpace_mergeConstData(kctx, (kNameSpaceVar*)ns, (KUtilsKeyValue*)KLIB Kwb_top(kctx, &wb, 0), nitems, pline);
	}
	KLIB Kwb_free(&wb);
	RESET_GCSTACK();
}

static void kNameSpace_importClassName(KonohaContext *kctx, kNameSpace *ns, kpackage_t packageId, kfileline_t pline)
{
	KUtilsKeyValue kv;
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	size_t i, size = KARRAYSIZE(kctx->share->classTable.bytesize, uintptr);
	for(i = 0; i < size; i++) {
		KonohaClass *ct = CT_(i);
		if(CT_isPrivate(ct)) continue;
		if(ct->packageId == packageId) {
			DBG_P("importing packageId=%s.%s, %s..", PackageId_t(ct->packageId), SYM_t(ct->nameid), PackageId_t(packageId));
			kv.key = ct->nameid;
			kv.ty  = TY_TYPE;
			kv.unboxValue = (uintptr_t)ct;
			KLIB Kwb_write(kctx, &wb, (const char*)(&kv), sizeof(KUtilsKeyValue));
		}
	}
	size_t nitems = Kwb_bytesize(&wb) / sizeof(KUtilsKeyValue);
	if(nitems > 0) {
		kNameSpace_mergeConstData(kctx, (kNameSpaceVar*)ns, (KUtilsKeyValue*)KLIB Kwb_top(kctx, &wb, 0), nitems, pline);
	}
	KLIB Kwb_free(&wb);
}

static KonohaClass *kNameSpace_getClass(KonohaContext *kctx, kNameSpace *ns, const char *name, size_t len, KonohaClass *defaultClass)
{
	KonohaClass *ct = NULL;
	kpackage_t packageId= PN_konoha;
	ksymbol_t  un = SYM_NONAME;
	char *p = strrchr(name, '.');
	if(p == NULL) {
		un = ksymbolA(name, len, SYM_NONAME);
	}
	else {
		size_t plen = p - name;
		un = ksymbolA(name + (plen+1), len - (plen+1), SYM_NONAME);
		packageId = KLIB KpackageId(kctx, name, plen, 0, SYM_NONAME);
	}
	if(packageId != SYM_NONAME) {
		uintptr_t hcode = longid(packageId, un);
		ct = (KonohaClass*)map_getu(kctx, kctx->share->longClassNameMapNN, hcode, 0);
		if(ct == NULL) {
			KUtilsKeyValue *kvs = kNameSpace_getConstNULL(kctx, ns, un);
			if(kvs != NULL && kvs->ty == TY_TYPE) {
				return (KonohaClass*)kvs->unboxValue;
			}
		}
	}
	return (ct != NULL) ? ct : defaultClass;
}

// --------------------------------------------------------------------------
// Method Management

static int formatLowerCanonicalName(char *buf, size_t bufsiz, const char *name)
{
	size_t i = 0;
	const char* p = name;
	while(p[0] != 0) {
		if(p[0] != '_') {
			buf[i] = tolower(p[0]);
		}
		i++;
		p++;
		if(!(i + 1 < bufsiz)) break;
	}
	buf[i] = 0;
	return i;
}

static kbool_t checkMethodPolicyOption(KonohaContext *kctx, kMethod *mtd, int option, int policy)
{
	if(TFLAG_is(int, policy, MPOL_PARAMSIZE)) {
		kParam *param = Method_param(mtd);
		if(param->psize != option) return false;
	}
	if(TFLAG_is(int, policy, MPOL_SIGNATURE)) {
		if(mtd->paramdom != option) return false;
	}
//	if(TFLAG_is(int, policy, MPOL_SETTER)) {
//		kParam *param = Method_param(mtd);
//		if(param->psize > 0 && param->paramtypeItems[param->psize - 1].ty != (ktype_t)option) return false;
//	}
	return true;
}

static kMethod* kMethodList_getCanonicalMethodNULL(KonohaContext *kctx, kArray *methodList, size_t beginIdx, ktype_t typeId, ksymbol_t mn, int option, int policy)
{
	size_t i;
	const char *name = SYM_t(SYM_UNMASK(mn));
	char canonicalName[80], methodCanonicalName[80];
	int firstChar = tolower(name[0]), namesize = formatLowerCanonicalName(canonicalName, sizeof(canonicalName), name);
	//DBG_P("canonicalName=%s.'%s'", TY_t(typeId), canonicalName);
	kMethod *foundMethod = NULL;
	for(i = beginIdx; i < kArray_size(methodList); i++) {
		kMethod *mtd = methodList->methodItems[i];
		if(SYM_HEAD(mtd->mn) != SYM_HEAD(mn)) continue;
		if(typeId != TY_var && mtd->typeId != typeId) continue;
		const char *n = SYM_t(SYM_UNMASK(mtd->mn));
		if(firstChar == tolower(n[0]) && namesize == formatLowerCanonicalName(methodCanonicalName, sizeof(methodCanonicalName), n)) {
			if(strcmp(canonicalName, methodCanonicalName) != 0) continue;
			if(policy > 1 && !checkMethodPolicyOption(kctx, mtd, option, policy)) {
				continue;
			}
			if(TFLAG_is(int, policy, MPOL_LATEST)) {
				foundMethod = mtd;
				continue;
			}
			return mtd;  // first one;
		}
	}
	return foundMethod;
}

static kMethod* kMethodList_getMethodNULL(KonohaContext *kctx, kArray *methodList, size_t beginIdx, ktype_t typeId, ksymbol_t mn, int option, int policy)
{
	kMethod *foundMethod = NULL;
	int i, filteredPolicy = policy & (~(MPOL_CANONICAL));
	for(i = beginIdx; i < kArray_size(methodList); i++) {
		kMethod *mtd = methodList->methodItems[i];
		if(mtd->mn != mn) continue;
		if(typeId != TY_var && mtd->typeId != typeId) continue;
		if(filteredPolicy > 1 && !checkMethodPolicyOption(kctx, mtd, option, filteredPolicy)) {
			continue;
		}
		foundMethod = mtd;
		if(!TFLAG_is(int, policy, MPOL_LATEST)) {
			break;
		}
	}
	if(foundMethod == NULL && TFLAG_is(int, policy, MPOL_CANONICAL)) {
		foundMethod = kMethodList_getCanonicalMethodNULL(kctx, methodList, beginIdx, typeId, mn, option, filteredPolicy);
//		DBG_P("canonicalName=%s.%s'%s', mtd=%p", TY_t(typeId), PSYM_t(mn), foundMethod);
//		if(foundMethod != NULL) {
//			DBG_P("method=%s.%s%s, mtd=%p", Method_t(foundMethod));
//		}
//		DBG_ASSERT(foundMethod == NULL);
	}
	return foundMethod;
}


static void kMethodList_findMethodList(KonohaContext *kctx, kArray *methodList, ktype_t typeId, ksymbol_t mn, kArray *resultList, int beginIdx)
{
	size_t i;
	for(i = 0; i < kArray_size(methodList); i++) {
		kMethod *mtd = methodList->methodItems[i];
		if(mtd->mn != mn) continue;
		if(typeId != TY_var && mtd->typeId != typeId) continue;
		kMethod *foundMethod = kMethodList_getMethodNULL(kctx, resultList, beginIdx, typeId, mn, 0, MPOL_FIRST);
		if(foundMethod == NULL) {
			KLIB kArray_add(kctx, resultList, mtd);
		}
	}
}

static void kNameSpace_findMethodList(KonohaContext *kctx, kNameSpace *ns, ktype_t typeId, ksymbol_t mn, kArray *resultList, int beginIdx)
{
	KonohaClass *ct = CT_(typeId);
	while(ns != NULL) {
		kMethodList_findMethodList(kctx, ns->methodList, typeId, mn, resultList, beginIdx);
		ns = ns->parentNULL;
	}
	while(ct != NULL) {
		kMethodList_findMethodList(kctx, ct->methodList, TY_var, mn, resultList, beginIdx);
		ct = ct->searchSuperMethodClassNULL;
	}
}

static kMethod* KonohaClass_getMethodNULL(KonohaContext *kctx, KonohaClass *ct, kmethodn_t mn, int option, int policy)
{
	while(ct != NULL) {
		kMethod *mtd = kMethodList_getMethodNULL(kctx, ct->methodList, 0, TY_var, mn, option, policy);
		if(mtd != NULL) return mtd;
		ct = ct->searchSuperMethodClassNULL;
	}
	return NULL;
}

static kMethod* kNameSpace_getFirstMethodNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t typeId, kmethodn_t mn, int option, int policy)
{
	if(ns != NULL) {
		kMethod *mtd = kNameSpace_getFirstMethodNULL(kctx, ns->parentNULL, typeId, mn, option, policy);
		if(mtd != NULL) return mtd;
		mtd = kMethodList_getMethodNULL(kctx, ns->methodList, 0, typeId, mn, option, policy);
		return mtd;
	}
	return NULL;
}

static kMethod* kNameSpace_getMethodNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t typeId, kmethodn_t mn, int option, int policy)
{
	if(TFLAG_is(int, policy, MPOL_LATEST)) {
		while(ns != NULL) {
			kMethod *mtd = kMethodList_getMethodNULL(kctx, ns->methodList, 0, typeId, mn, option, policy);
			if(mtd != NULL) return mtd;
			ns = ns->parentNULL;
		}
		return KonohaClass_getMethodNULL(kctx, CT_(typeId), mn, option, policy);
	}
	else {
		kMethod *mtd = KonohaClass_getMethodNULL(kctx, CT_(typeId), mn, option, policy);
		if(mtd != NULL) return mtd;
		return kNameSpace_getFirstMethodNULL(kctx, ns, typeId, mn, option, policy);
	}
}

static kMethod* kMethod_replaceWith(KonohaContext *kctx, kMethodVar *oldMethod, kMethodVar *newMethod)
{
	if(Method_isOverride(newMethod)) {
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
	}
	DBG_P("loading method %s.%s%s: @Public=%d", Method_t(mtd), Method_isPublic(mtd), mtd->flag);
	if(Method_isPublic(mtd) /* && ct->packageDomain == ns->packageDomain*/) {
		kMethod *foundMethod = KonohaClass_getMethodNULL(kctx, ct, mtd->mn, mtd->paramdom, MPOL_FIRST|MPOL_SIGNATURE);
		if(foundMethod != NULL) {  // same signature
			if(foundMethod->typeId == mtd->typeId) {
				DBG_P("duplicated method %s.%s%s", Method_t(foundMethod));
				PUSH_GCSTACK(mtd);  // avoid memory leaking
				return kMethod_replaceWith(kctx, (kMethodVar*)foundMethod, (kMethodVar*)mtd);
			}
			else {
				if(!Method_isFinal(foundMethod)) {
					DBG_P("Changing Virtual method %s.%s%s by %s.%s%s....", Method_t(foundMethod), Method_t(mtd));
					Method_setVirtual(foundMethod, true);  // FIXME
				}
				if(!Method_isVirtual(foundMethod) || Method_isFinal(foundMethod)) {
					DBG_P("Can't override method %s.%s%s <: %s.%s%s ....", Method_t(mtd), Method_t(foundMethod));
					return NULL;
				}
			}
		}
		else {
			foundMethod = KonohaClass_getMethodNULL(kctx, ct, mtd->mn, Method_paramsize(mtd), MPOL_FIRST|MPOL_PARAMSIZE);
			if(foundMethod != NULL) {
				DBG_P("set overloading method %s.%s%s", Method_t(foundMethod));
				Method_setOverloaded(foundMethod, true);
				Method_setOverloaded(mtd, true);
			}
		}
		if(unlikely(ct->methodList == K_EMPTYARRAY)) {
			KINITv(((KonohaClassVar*)ct)->methodList, new_(MethodArray, 8));
		}
		KLIB kArray_add(kctx, ct->methodList, mtd);
	}
	else {
		size_t i;
		for(i = 0; i < kArray_size(ns->methodList); i++) {
			kMethod *foundMethod = ns->methodList->methodItems[i];
			if(foundMethod->typeId == mtd->typeId && foundMethod->mn == mtd->mn && foundMethod->paramdom == mtd->paramdom) {
				DBG_P("duplicated method %s.%s%s", Method_t(foundMethod));
				PUSH_GCSTACK(mtd);  // avoid memory leaking
				return kMethod_replaceWith(kctx, (kMethodVar*)foundMethod, (kMethodVar*)mtd);
			}
		}
		kArray *matchedMethodList = kctx->stack->gcstack;
		size_t popMatchedMethodListSize = kArray_size(matchedMethodList);
		kNameSpace_findMethodList(kctx, ns, mtd->typeId, mtd->mn, matchedMethodList, popMatchedMethodListSize);
		if(popMatchedMethodListSize < kArray_size(matchedMethodList)) {
			int count = 0;
			for(i = popMatchedMethodListSize; i < kArray_size(matchedMethodList); i++) {
				kMethod *foundMethod = matchedMethodList->methodItems[i];
				if(Method_paramsize(foundMethod) == Method_paramsize(mtd)) {
					Method_setOverloaded(foundMethod, true);
					count++;
				}
			}
			if(count > 0) {
				DBG_P("set overloading method %s.%s%s", Method_t(mtd));
				Method_setOverloaded(mtd, true);
			}
			KLIB kArray_clear(kctx, matchedMethodList, popMatchedMethodListSize);
		}
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
		kparamtype_t p[psize+1];
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

static kMethod* kNameSpace_getCastMethodNULL(KonohaContext *kctx, kNameSpace *ns, ktype_t cid, ktype_t tcid)
{
	kMethod *mtd = kNameSpace_getMethodNULL(kctx, ns, cid, MN_to(tcid), 0, MPOL_PARAMSIZE|MPOL_FIRST);
	if(mtd == NULL) {
		mtd = kNameSpace_getMethodNULL(kctx, ns, cid, MN_as(tcid), 0, MPOL_PARAMSIZE|MPOL_FIRST);
	}
	return mtd;
}

