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

#define PARAM_void     0
#define PARAMDOM_void                  0
#define PARAMDOM_DefaultGenericsParam  1

static kObject* DEFAULT_fnull(KonohaContext *kctx, KonohaClass *ct);
static void kArray_Add(KonohaContext *kctx, kArray *o, kAbstractObject *);

static void kObject_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kObjectVar *of = (kObjectVar *)o;
	of->fieldUnboxItems[0] = 0;
}

static void kObject_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kObject *of = (kObject *)o;
	KonohaClass *ct = O_ct(of);
	size_t i;
	for(i = 0; i < ct->fieldsize; i++) {
		if(TypeAttr_Is(Boxed, ct->fieldItems[i].attrTypeId)) {
			KRefTrace(of->fieldObjectItems[i]);
		}
	}
}

static kObject *new_kObject(KonohaContext *kctx, kArray *gcstackNULL, KonohaClass *ct, uintptr_t conf)
{
	DBG_ASSERT(ct->cstruct_size > 0);
	kObjectVar *o = PLATAPI AllocObject(kctx, ct->cstruct_size, NULL/*FIXME*/);
	o->h.magicflag = ct->magicflag;
	o->h.ct = ct;
	o->h.prototypePtr = NULL; /* (KProtoMap *) Kprotomap_new(0); */
	if(gcstackNULL != NULL) {
		kArray_Add(kctx, gcstackNULL, o);
	}
	ct->init(kctx, (kObject *)o, (void *)conf);
	return (kObject *)o;
}

static void KonohaClass_WriteUnboxValueToBuffer(KonohaContext *kctx, KonohaClass *c, uintptr_t unboxValue, int isDelim, KGrowingBuffer *wb)
{
	if(isDelim > 0) {
		KLIB KBuffer_Write(kctx, wb, ", ", 2);
	}
	KonohaValue v = {};
	v.unboxValue = unboxValue;
	c->p(kctx, &v, 0, wb);
}

static void kObject_WriteToBuffer(KonohaContext *kctx, kObject *o, int isDelim, KGrowingBuffer *wb, KonohaValue *sfp, int pos)
{
	if(isDelim > 0) {
		KLIB KBuffer_Write(kctx, wb, ", ", 2);
	}
	if(IS_NULL(o)) {
		KLIB KBuffer_Write(kctx, wb, TEXTSIZE("null"));
	}
	else {
		if(sfp == NULL) {
			sfp = (KonohaValue *)kctx->esp;
			pos = 0;
		}
		else {
			int i;
			for(i = 0; i < pos; i++) {
				if(sfp[i].asObject == o) break;
			}
		}
		if(TY_isUnbox(O_typeId(o))) {
			sfp[pos].unboxValue = O_unbox(o);
		}
		else {
			KUnsafeFieldSet(sfp[pos].asObject, o);
		}
		O_ct(o)->p(kctx, sfp, pos, wb);
	}
}

static uintptr_t kNumber_unbox(KonohaContext *kctx, kObject *o)
{
	kNumber *n = (kNumber *)o;
	return (uintptr_t) n->unboxValue;
}

// Boolean
static void kNumber_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kIntVar *n = (kIntVar *)o;
	n->unboxValue = (uintptr_t)conf;
}

static void kBoolean_p(KonohaContext *kctx, KonohaValue *v, int pos, KGrowingBuffer *wb)
{
	if(v[pos].boolValue) {
		KLIB KBuffer_Write(kctx, wb, TEXTSIZE("true"));
	}
	else {
		KLIB KBuffer_Write(kctx, wb, TEXTSIZE("false"));
	}
}

static kObject* kBoolean_fnull(KonohaContext *kctx, KonohaClass *ct)
{
	return (kObject *)K_FALSE;
}

static void kInt_p(KonohaContext *kctx, KonohaValue *v, int pos, KGrowingBuffer *wb)
{
	KLIB KBuffer_printf(kctx, wb, KINT_FMT, v[pos].intValue);
}

// String
static void kString_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kStringVar *s = (kStringVar *)o;
	s->text = "";
	s->bytesize = 0;
	kString_set(TextSgm, s, true);
}

static void kString_Free(KonohaContext *kctx, kObject *o)
{
	kString *s = (kString *)o;
	if(!kString_is(TextSgm, s)) {
		KFree(s->buf, S_size(s) + 1);
	}
}

static void kString_p(KonohaContext *kctx, KonohaValue *v, int pos, KGrowingBuffer *wb)
{
	const char *t = S_text(v[pos].asString);
	size_t i, len = S_size(v[pos].asString);
	KBuffer_Write(kctx, wb, "\"", 1);
	for(i = 0; i < len; i++) {
		int ch = t[i];
		char buf[2] = {'\\', ch};
		if(ch == '\\' || ch == '"') {
			KLIB KBuffer_Write(kctx, wb, buf, 2);
		}
		else if(t[i] == '\n'){
			buf[1] = 'n';
			KLIB KBuffer_Write(kctx, wb, buf, 2);
		}
		else if(t[i] == '\t'){
			buf[1] = 't';
			KLIB KBuffer_Write(kctx, wb, buf, 2);
		}
		else {
			KLIB KBuffer_Write(kctx, wb, buf+1, 1);
		}
	}
	KBuffer_Write(kctx, wb, "\"", 1);
}

static uintptr_t kString_unbox(KonohaContext *kctx, kObject *o)
{
	kString *s = (kString *)o;
	DBG_ASSERT(IS_String(s));
	return (uintptr_t) s->text;
}

static void kString_CheckASCII(KonohaContext *kctx, kString *s)
{
	unsigned char ch = 0;
	long len = S_size(s), n = (len + 3) / 4;
	const unsigned char*p = (const unsigned char *)S_text(s);
	switch(len % 4) { /* Duff's device written by ide */
		case 0: do{ ch |= *p++;
		case 3:     ch |= *p++;
		case 2:     ch |= *p++;
		case 1:     ch |= *p++;
		} while(--n>0);
	}
	kString_set(ASCII, (kStringVar *)s, (ch < 128));
}

static kString* new_kString(KonohaContext *kctx, kArray *gcstack, const char *text, size_t len, int spol)
{
	KonohaClass *ct = CT_(TY_String);
	kStringVar *s = NULL;
	if(TFLAG_is(int, spol, StringPolicy_TEXT)) {
		s = (kStringVar *)new_kObject(kctx, gcstack, ct, 0);
		s->text = text;
		s->bytesize = len;
		kString_set(TextSgm, s, 1);
		kString_set(MallocText, s, 1);
	}
	else if(len + 1 < sizeof(void *) * 2) {
		s = (kStringVar *)new_kObject(kctx, gcstack, ct, 0);
		s->text = s->inline_text;
		s->bytesize = len;
		kString_set(TextSgm, s, 1);
		if(text != NULL) {
			DBG_ASSERT(!TFLAG_is(int, spol, StringPolicy_NOCOPY));
			memcpy(s->ubuf, text, len);
		}
		s->buf[len] = '\0';
	}
	else {
		s = (kStringVar *)new_kObject(kctx, gcstack, ct, 0);
		s->bytesize = len;
		s->buf = (char *)KMalloc_UNTRACE(len+1);
		kString_set(TextSgm, s, 0);
		kString_set(MallocText, s, 1);
		if(text != NULL) {
			DBG_ASSERT(!TFLAG_is(int, spol, StringPolicy_NOCOPY));
			memcpy(s->ubuf, text, len);
		}
		s->buf[len] = '\0';
	}
	if(TFLAG_is(int, spol, StringPolicy_ASCII)) {
		kString_set(ASCII, s, 1);
	}
	else if(TFLAG_is(int, spol, StringPolicy_UTF8)) {
		kString_set(ASCII, s, 0);
	}
	else {
		kString_CheckASCII(kctx, s);
	}
	return s;
}

// Array
struct _kAbstractArray {
	KonohaObjectHeader h;
	KGrowingArray a;
};

static void kArray_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct _kAbstractArray *a = (struct _kAbstractArray *)o;
	a->a.bytebuf     = NULL;
	a->a.bytesize    = 0;
	a->a.bytemax = ((size_t)conf * sizeof(void *));
	if(a->a.bytemax > 0) {
		KLIB KArray_Init(kctx, &a->a, a->a.bytemax);
	}
	if(TY_isUnbox(O_p0(a))) {
		kArray_setUnboxData(a, 1);
	}
}

static void kArray_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kArray *a = (kArray *)o;
	if(!kArray_isUnboxData(a)) {
		size_t i;
		for(i = 0; i < kArray_size(a); i++) {
			KRefTrace(a->ObjectItems[i]);
		}
	}
}

static void kArray_Free(KonohaContext *kctx, kObject *o)
{
	struct _kAbstractArray *a = (struct _kAbstractArray *)o;
	KLIB KArray_Free(kctx, &a->a);
}

static void kArray_p(KonohaContext *kctx, KonohaValue *values, int pos, KGrowingBuffer *wb)
{
	size_t i;
	kArray *a = values[pos].asArray;
	KLIB KBuffer_Write(kctx, wb, "[", 1);
	if(kArray_isUnboxData(a)) {
		KonohaClass *c = CT_(O_p0(a));
		for(i = 0; i < kArray_size(a); i++) {
			KonohaClass_WriteUnboxValueToBuffer(kctx, c, a->unboxItems[i], (i > 0)/*delim*/, wb);
		}
	}
	else {
		for(i = 0; i < kArray_size(a); i++) {
			kObject *o = a->ObjectItems[i];
			kObject_WriteToBuffer(kctx, o, (i>0)/*delim*/, wb, values, pos+1);
		}
	}
	KLIB KBuffer_Write(kctx, wb, "]", 1);
}

static void kArray_ensureMinimumSize(KonohaContext *kctx, struct _kAbstractArray *a, size_t min)
{
	size_t minbyte = min * sizeof(void *);
	if(!(minbyte < a->a.bytemax)) {
		if(minbyte < sizeof(kObject)) minbyte = sizeof(kObject);
		KLIB KArray_Expand(kctx, &a->a, minbyte);
	}
}

static void kArray_Add(KonohaContext *kctx, kArray *o, kAbstractObject *value)
{
	size_t asize = kArray_size(o);
	struct _kAbstractArray *a = (struct _kAbstractArray *)o;
	kArray_ensureMinimumSize(kctx, a, asize+1);
	DBG_ASSERT(a->a.ObjectItems[asize] == NULL);
	KFieldInit(a, a->a.ObjectItems[asize], value);
	a->a.bytesize = (asize+1) * sizeof(void *);
}

static void kArray_Insert(KonohaContext *kctx, kArray *o, size_t n, kAbstractObject *v)
{
	size_t asize = kArray_size(o);
	struct _kAbstractArray *a = (struct _kAbstractArray *)o;
	if(!(n < asize)) {
		kArray_Add(kctx, o, v);
	}
	else {
		kArray_ensureMinimumSize(kctx, a, asize+1);
		memmove(a->a.ObjectItems+(n+1), a->a.ObjectItems+n, sizeof(kObject *) * (asize - n));
		KFieldInit(a, a->a.ObjectItems[n], v);
		a->a.bytesize = (asize+1) * sizeof(void *);
	}
}

static void kArray_Clear(KonohaContext *kctx, kArray *o, size_t n)
{
	DBG_ASSERT(IS_Array(o));
	size_t asize = kArray_size(o);
	struct _kAbstractArray *a = (struct _kAbstractArray *)o;
	DBG_ASSERT(asize >= n);
	if(asize > n) {
		bzero(a->a.ObjectItems + n, sizeof(void *) * (asize - n));  // RCGC
		a->a.bytesize = (n) * sizeof(void *);
	}
}

// ---------------
// Param

static KonohaClass *KonohaClass_extendedBody(KonohaContext *kctx, KonohaClass *ct, size_t head, size_t body);

static void kParam_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kParamVar *pa = (kParamVar *)o;
	pa->psize = 0;
	pa->rtype = TY_void;
}

static kParam *new_Param(KonohaContext *kctx, kArray *gcstack, kattrtype_t rtype, int psize, const kparamtype_t *p)
{
	KonohaClass *ct = CT_(TY_Param);
	ct = KonohaClass_extendedBody(kctx, ct, sizeof(void *), psize * sizeof(kparamtype_t));
	kParamVar *pa = (kParamVar *)new_kObject(kctx, gcstack, ct, 0);
	pa->rtype = rtype;
	pa->psize = psize;
	if(psize > 0) {
		memcpy(pa->paramtypeItems, p, sizeof(kparamtype_t) * psize);
	}
	return pa;
}

static uintptr_t hashparamdom(kushort_t psize, const kparamtype_t *p)
{
	kushort_t i;
	uintptr_t hcode = 0;
	for(i = 0; i < psize; i++) {
		hcode = p[i].attrTypeId + (31 * hcode);
	}
	return hcode;
}

static uintptr_t hashparam(kattrtype_t rtype, kushort_t psize, const kparamtype_t *p)
{
	kushort_t i;
	uintptr_t hcode = rtype;
	for(i = 0; i < psize; i++) {
		hcode = (p[i].attrTypeId + p[i].name) + (31 * hcode);
	}
	return hcode;
}

static kbool_t equalsParamDom(kattrtype_t rtype, kushort_t psize, const kparamtype_t *p, kParam *pa)
{
	if(psize == pa->psize) {
		kushort_t i;
		for(i = 0; i < psize; i++) {
			if(p[i].attrTypeId != pa->paramtypeItems[i].attrTypeId) return false;
		}
		return true;
	}
	return false;
}

static kbool_t equalsParam(kattrtype_t rtype, kushort_t psize, const kparamtype_t *p, kParam *pa)
{
	if(rtype == pa->rtype && psize == pa->psize) {
		kushort_t i;
		for(i = 0; i < psize; i++) {
			if(p[i].attrTypeId != pa->paramtypeItems[i].attrTypeId || p[i].name != pa->paramtypeItems[i].name) return false;
		}
		return true;
	}
	return false;
}

typedef kbool_t (*equalsP)(kattrtype_t rtype, kushort_t psize, const kparamtype_t *p, kParam *pa);

static kparamId_t KHashMap_getparamid(KonohaContext *kctx, KHashMap *kmp, kArray *list, uintptr_t hcode, equalsP f, kattrtype_t rtype, kushort_t psize, const kparamtype_t *p)
{
	KHashMapEntry *e = KLIB KHashMap_get(kctx, kmp, hcode);
	while(e != NULL) {
		if(e->hcode == hcode && f(rtype, psize, p, e->paramKey_OnList)) {
			return (kparamId_t)e->unboxValue;
		}
		e = e->next;
	}
	uintptr_t paramid = kArray_size(list);
	kParam *paramkey = new_Param(kctx, list, rtype, psize, p);
	e = KLIB KHashMap_newEntry(kctx, kmp, hcode);
	e->paramKey_OnList   = paramkey;
	e->unboxValue = paramid;
	return (kparamId_t)paramid;
}

static kparamId_t Kparam(KonohaContext *kctx, kattrtype_t rtype, kushort_t psize, const kparamtype_t *p)
{
	uintptr_t hcode = hashparam(rtype, psize, p);
	KLock(kctx->share->paramMutex);
	kparamId_t param = KHashMap_getparamid(kctx, kctx->share->paramMap_KeyOnList, kctx->share->paramList_OnGlobalConstList, hcode, equalsParam, rtype, psize, p);
	KUnlock(kctx->share->paramMutex);
	return param;
}

static kparamId_t Kparamdom(KonohaContext *kctx, kushort_t psize, const kparamtype_t *p)
{
	uintptr_t hcode = hashparamdom(psize, p);
	KLock(kctx->share->paramMutex);
	kparamId_t param = KHashMap_getparamid(kctx, kctx->share->paramdomMap_KeyOnList, kctx->share->paramdomList_OnGlobalConstList, hcode, equalsParamDom, TY_void, psize, p);
	KUnlock(kctx->share->paramMutex);
	return param;
}

/* --------------- */
/* Method */

static uintptr_t methodSerialNumber = 0;

static void kMethod_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kMethodVar *mtd = (kMethodVar *)o;
	bzero(&mtd->invokeMethodFunc, sizeof(kMethod) - sizeof(KonohaObjectHeader));
	KFieldInit(mtd, mtd->SourceToken, (struct kToken *)K_NULL);
	KFieldInit(mtd, mtd->LazyCompileNameSpace, K_NULL);
	mtd->serialNumber = methodSerialNumber++;
}

static void kMethod_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kMethod *mtd = (kMethod *)o;
	KRefTrace(mtd->SourceToken);
	KRefTrace(mtd->LazyCompileNameSpace);
}

static void kMethod_Free(KonohaContext *kctx, kObject *o)
{
	kMethodVar *mtd = (kMethodVar *)o;
	if(mtd->virtualCodeApi_plus1 != NULL && mtd->virtualCodeApi_plus1[-1] != NULL) {
		mtd->virtualCodeApi_plus1[-1]->FreeVirtualCode(kctx, mtd->vcode_start);
		mtd->vcode_start = NULL;
	}
}

#define CT_MethodVar CT_Method
static kMethodVar* new_kMethod(KonohaContext *kctx, kArray *gcstack, uintptr_t flag, kattrtype_t cid, kmethodn_t mn, MethodFunc func)
{
	kMethodVar* mtd = new_(MethodVar, NULL, gcstack);
	mtd->flag       = flag;
	mtd->typeId     = cid;
	mtd->mn         = mn;
	KLIB kMethod_SetFunc(kctx, mtd, func);
	return mtd;
}

static kParam* kMethod_SetParam(KonohaContext *kctx, kMethod *mtd_, kattrtype_t rtype, kushort_t psize, const kparamtype_t *p)
{
	kparamId_t paramId = Kparam(kctx, rtype, psize, p);
	if(mtd_ != NULL) {
		kMethodVar* mtd = (kMethodVar *)mtd_;
		mtd->paramdom = Kparamdom(kctx, psize, p);
		mtd->paramid  = paramId;
	}
	return kctx->share->paramList_OnGlobalConstList->ParamItems[paramId];
}

static intptr_t STUB_Method_indexOfField(kMethod *mtd)
{
	return -1;
}

// ---------------
// NameSpace

static void kNameSpace_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kNameSpaceVar *ns = (kNameSpaceVar *)o;
	bzero(&ns->parentNULL, sizeof(kNameSpace) - sizeof(KonohaObjectHeader));
	KFieldInit(ns, ns->NameSpaceConstList, new_(Array, 0, OnField));
	ns->syntaxOption = kNameSpace_DefaultSyntaxOption;
	if(conf != NULL) {
		KFieldInit(ns, ns->parentNULL, (kNameSpace *)conf);
		ns->packageId     = ns->parentNULL->packageId;
		ns->syntaxOption  = ns->parentNULL->syntaxOption;
	}
	ns->methodList_OnList = K_EMPTYARRAY;
	ns->builderApi = PLATAPI GetDefaultBuilderAPI();
}

static void kNameSpace_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kNameSpace *ns = (kNameSpace *)o;
	KRefTrace(ns->NameSpaceConstList);
}

static void kNameSpace_Free(KonohaContext *kctx, kObject *o)
{
	kNameSpaceVar *ns = (kNameSpaceVar *)o;
	KLIB kNameSpace_FreeSugarExtension(kctx, ns);
	KLIB KDict_Free(kctx, &(ns->constTable));
}

static void kNameSpace_p(KonohaContext *kctx, KonohaValue *v, int pos, KGrowingBuffer *wb)
{
	kNameSpace *ns = v[pos].asNameSpace;
	KLIB KBuffer_printf(kctx, wb, "%s", PackageId_t(ns->packageId));
}

// ---------------
// System

static void Func_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kFuncVar *fo = (kFuncVar *)o;
	KFieldInit(fo, fo->self, K_NULL);
	KFieldInit(fo, fo->mtd, conf == NULL ? KNULL(Method) : (kMethod *)conf);
}

static void Func_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kFunc *fo = (kFunc *)o;
	KRefTrace(fo->self);
	KRefTrace(fo->mtd);
}

// ---------------
// System


// ---------------

static KonohaClass *T_realtype(KonohaContext *kctx, KonohaClass *ct, KonohaClass *self)
{
	kParam *cparam = CT_cparam(self);
	//DBG_P("ct=%s, self=%s", CT_t(ct), CT_t(self));
	DBG_ASSERT(ct->optvalue < cparam->psize);
	KonohaClass *pct = CT_(cparam->paramtypeItems[ct->optvalue].attrTypeId);
	return pct->realtype(kctx, pct, self);
}

// ---------------

static KonohaClass* Kclass(KonohaContext *kctx, kattrtype_t cid, KTraceInfo *trace)
{
	KonohaRuntime *share = kctx->share;
	if(!(cid < (share->classTable.bytesize/sizeof(KonohaClassVar *)))) {
		KLIB KonohaRuntime_raise(kctx, EXPT_("InvalidParameter"), SoftwareFault, NULL, trace->baseStack);
	}
	return share->classTable.classItems[cid];
}

static void DEFAULT_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	(void)kctx;(void)o;(void)conf;
}

static void DEFAULT_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	(void)kctx;(void)o;
}

static void DEFAULT_Free(KonohaContext *kctx, kObject *o)
{
	(void)kctx;(void)o;
}

static void DEFAULT_p(KonohaContext *kctx, KonohaValue *v, int pos, KGrowingBuffer *wb)
{
	KLIB KBuffer_printf(kctx, wb, "&%p(:%s)", v[pos].asObject, TY_t(O_typeId(v[pos].asObject)));
}

static uintptr_t DEFAULT_unbox(KonohaContext *kctx, kObject *o)
{
	return 0;
}

static kbool_t DEFAULT_isSubType(KonohaContext *kctx, KonohaClass* ct, KonohaClass *t)
{
	if(t->typeId == TY_Object) return true;
	while(ct->superTypeId != TY_Object) {
		ct = CT_(ct->superTypeId);
		if(ct->typeId == t->typeId) return true;
	}
	return false;
}

static KonohaClass* DEFAULT_realtype(KonohaContext *kctx, KonohaClass* c, KonohaClass *self)
{
	return c;
}

static kObject* DEFAULT_fnull(KonohaContext *kctx, KonohaClass *ct)
{
	DBG_ASSERT(ct->defaultNullValue_OnGlobalConstList != NULL);
	return ct->defaultNullValue_OnGlobalConstList;
}

static kObject* DEFAULT_fnullinit(KonohaContext *kctx, KonohaClass *ct)
{
	DBG_ASSERT(ct->defaultNullValue_OnGlobalConstList == NULL);
	((KonohaClassVar *)ct)->defaultNullValue_OnGlobalConstList = KLIB new_kObject(kctx, OnGlobalConstList, ct, 0);
	kObject_set(NullObject, ct->defaultNullValue_OnGlobalConstList, true);
	((KonohaClassVar *)ct)->fnull = DEFAULT_fnull;
	return ct->defaultNullValue_OnGlobalConstList;
}

static kObject *Knull(KonohaContext *kctx, KonohaClass *ct)
{
	return ct->fnull(kctx, ct);
}

static KonohaClassVar* new_KonohaClass(KonohaContext *kctx, KonohaClass *bct, KDEFINE_CLASS *s, KTraceInfo *trace)
{
	KonohaRuntimeVar *share = (KonohaRuntimeVar *)kctx->share;
	KonohaClassVar *ct;
	kattrtype_t newid;
	KLock(share->classTableMutex); {
		newid = share->classTable.bytesize / sizeof(KonohaClassVar *);
		if(share->classTable.bytesize == share->classTable.bytemax) {
			KLIB KArray_Expand(kctx, &share->classTable, share->classTable.bytemax * 2);
		}
		share->classTable.bytesize += sizeof(KonohaClassVar *);
		ct = (KonohaClassVar *)KCalloc_UNTRACE(sizeof(KonohaClass), 1);
		share->classTable.classItems[newid] = (KonohaClass *)ct;
	}
	KUnlock(share->classTableMutex);
	if(bct != NULL) {
		DBG_ASSERT(s == NULL);
		memcpy(ct, bct, offsetof(KonohaClass, methodList_OnGlobalConstList));
		ct->typeId = newid;
		if(ct->fnull == DEFAULT_fnull) ct->fnull =  DEFAULT_fnullinit;
	}
	else {
		DBG_ASSERT(s != NULL);
		ct->cflag   = s->cflag;
		ct->typeId = newid;
		ct->baseTypeId    = (s->baseTypeId == 0) ? ct->typeId : s->baseTypeId;
		ct->superTypeId  = (s->superTypeId == 0) ? TY_Object : s->superTypeId;
		ct->fieldItems = s->fieldItems;
		ct->fieldsize  = s->fieldsize;
		ct->fieldAllocSize = s->fieldAllocSize;
		ct->cstruct_size = size64(s->cstruct_size);
		DBG_ASSERT(ct->cstruct_size <= 256);
		ct->DBG_NAME = (s->structname != NULL) ? s->structname : "N/A";
		if(s->cparamsize > 0 && s->cParamItems != NULL) {
			ct->p0 = s->cParamItems[0].attrTypeId;
			ct->cparamdom = Kparamdom(kctx, s->cparamsize, s->cParamItems);
		}
		// function
		ct->init = (s->init != NULL) ? s->init : DEFAULT_Init;
		ct->reftrace = (s->reftrace != NULL) ? s->reftrace : DEFAULT_Reftrace;
		ct->p     = (s->p != NULL) ? s->p : DEFAULT_p;
		ct->unbox = (s->unbox != NULL) ? s->unbox : DEFAULT_unbox;
		ct->free = (s->free != NULL) ? s->free : DEFAULT_Free;
		ct->fnull = (s->fnull != NULL) ? s->fnull : DEFAULT_fnullinit;
		ct->realtype = (s->realtype != NULL) ? s->realtype : DEFAULT_realtype;
		ct->isSubType = (s->isSubType != NULL) ? s->isSubType : DEFAULT_isSubType;
		ct->initdef = s->initdef;
	}
	if(ct->initdef != NULL) {
		ct->initdef(kctx, ct, trace);
	}
	return ct;
}

static KonohaClass *KonohaClass_extendedBody(KonohaContext *kctx, KonohaClass *ct, size_t head, size_t body)
{
	KonohaClass *bct = ct;
	while(ct->cstruct_size < sizeof(KonohaObjectHeader) + head + body) {
		if(ct->searchSimilarClassNULL == NULL) {
			KonohaClassVar *newct = new_KonohaClass(kctx, bct, NULL, NOPLINE);
			newct->cflag |= kClass_Private;
			newct->cstruct_size = ct->cstruct_size * 2;
			newct->methodList_OnGlobalConstList = ct->methodList_OnGlobalConstList;
			((KonohaClassVar *)ct)->searchSimilarClassNULL = (KonohaClass *)newct;
		}
		ct = ct->searchSimilarClassNULL;
	}
	return ct;
}

static KonohaClass *Generics_realtype(KonohaContext *kctx, KonohaClass *ct, KonohaClass *self)
{
	//DBG_P("trying resolve generic type: %s %s", CT_t(ct), CT_t(self));
	kParam *param = CT_cparam(ct);
	kushort_t i;
	kparamtype_t *p = ALLOCA(kparamtype_t, param->psize);
	for(i = 0; i < param->psize; i++) {
		KonohaClass *cParam = CT_(param->paramtypeItems[i].attrTypeId);
		p[i].attrTypeId = cParam->realtype(kctx, cParam, self)->typeId;
	}
	return KLIB KonohaClass_Generics(kctx, ct, TY_void, param->psize, p);
}

static KonohaClass *Func_realtype(KonohaContext *kctx, KonohaClass *ct, KonohaClass *self)
{
	//DBG_P("trying resolve generic type: %s %s", CT_t(ct), CT_t(self));
	KonohaClass *cReturn = CT_(ct->p0);
	kattrtype_t rtype = cReturn->realtype(kctx, cReturn, self)->typeId;
	kParam *param = CT_cparam(ct);
	kushort_t i;
	kparamtype_t *p = ALLOCA(kparamtype_t, param->psize);
	for(i = 0; i < param->psize; i++) {
		KonohaClass *cParam = CT_(param->paramtypeItems[i].attrTypeId);
		p[i].attrTypeId = cParam->realtype(kctx, cParam, self)->typeId;
	}
	return KLIB KonohaClass_Generics(kctx, CT_(ct->baseTypeId), rtype, param->psize, p);
}

#define TY_isTypeVar2(T)   (T != TY_void && TY_is(TypeVar, T))

static void checkTypeVar(KonohaContext *kctx, KonohaClassVar *newct, kattrtype_t rtype, int psize, kparamtype_t *p)
{
	int i, isTypeVar = TY_isTypeVar2(rtype);
	if(!isTypeVar) {
		for(i = 0; i < psize; i++) {
			if(TY_is(TypeVar, p[i].attrTypeId)) {
				isTypeVar = true;
			}
		}
	}
	if(isTypeVar) {
		//DBG_P("Generics %s has TypeVar", CT_t(newct));
		newct->cflag |= kClass_TypeVar;
		newct->realtype = newct->baseTypeId == TY_Func ? Func_realtype : Generics_realtype;
	}
}

static KonohaClass *KonohaClass_Generics(KonohaContext *kctx, KonohaClass *ct, kattrtype_t rtype, kushort_t psize, kparamtype_t *p)
{
	kparamId_t paramdom = Kparamdom(kctx, psize, p);
	KonohaClass *ct0 = CT_(ct->baseTypeId);
	ct = ct0;
	int isNotFuncClass = (ct->baseTypeId != TY_Func);
	do {
		if(ct->cparamdom == paramdom && (isNotFuncClass || ct->p0 == rtype)) {
			return ct;
		}
		if(ct->searchSimilarClassNULL == NULL) break;
		ct = ct->searchSimilarClassNULL;
	} while(ct != NULL);

	KonohaClassVar *newct = new_KonohaClass(kctx, ct0, NULL, NOPLINE);
	newct->cparamdom = paramdom;
	newct->p0 = isNotFuncClass ? p[0].attrTypeId : rtype;
	newct->methodList_OnGlobalConstList = K_EMPTYARRAY;
	if(newct->searchSuperMethodClassNULL == NULL) {
		newct->searchSuperMethodClassNULL = ct0;
	}
	checkTypeVar(kctx, newct, rtype, psize, p);
	((KonohaClassVar *)ct)->searchSimilarClassNULL = (KonohaClass *)newct;
	return ct->searchSimilarClassNULL;
}

static kString* KonohaClass_shortName(KonohaContext *kctx, KonohaClass *ct)
{
	if(ct->shortClassNameNULL_OnGlobalConstList == NULL) {
		if(ct->cparamdom == 0 && ct->baseTypeId != TY_Func) {
			((KonohaClassVar *)ct)->shortClassNameNULL_OnGlobalConstList = SYM_s(ct->classNameSymbol);
		}
		else {
			size_t i, c = 0;
			kParam *cparam = CT_cparam(ct);
			KGrowingBuffer wb;
			KonohaClass_shortName(kctx, CT_(ct->p0));
			for(i = 0; i < cparam->psize; i++) {
				KonohaClass_shortName(kctx, CT_(cparam->paramtypeItems[i].attrTypeId));
			}
			KBuffer_Init(&(kctx->stack->cwb), &wb);
			kString *s = SYM_s(ct->classNameSymbol);
			KLIB KBuffer_Write(kctx, &wb, S_text(s), S_size(s));
			KLIB KBuffer_Write(kctx, &wb, "[", 1);
			if(ct->baseTypeId == TY_Func) {
				s = KonohaClass_shortName(kctx, CT_(ct->p0));
				KLIB KBuffer_Write(kctx, &wb, S_text(s), S_size(s)); c++;
			}
			for(i = 0; i < cparam->psize; i++) {
				if(c > 0) KLIB KBuffer_Write(kctx, &wb, ",", 1);
				s = KonohaClass_shortName(kctx, CT_(cparam->paramtypeItems[i].attrTypeId));
				KLIB KBuffer_Write(kctx, &wb, S_text(s), S_size(s));
			}
			KLIB KBuffer_Write(kctx, &wb, "]", 1);
			const char *text = KBuffer_Stringfy(kctx, &wb, 1);
			((KonohaClassVar *)ct)->shortClassNameNULL_OnGlobalConstList = new_kString(kctx, OnGlobalConstList, text, KBuffer_bytesize(&wb), StringPolicy_ASCII);
			KLIB KBuffer_Free(&wb);
		}
	}
	return ct->shortClassNameNULL_OnGlobalConstList;
}

static void KonohaClass_setName(KonohaContext *kctx, KonohaClassVar *ct, KTraceInfo *trace)
{
	if(trace != NULL) {
		/* To avoid SEGV, because this message is called at the initial time. */
		KLIB ReportScriptMessage(kctx, trace, DebugTag, "new class %s.%s", PackageId_t(ct->packageId), SYM_t(ct->classNameSymbol));
	}
	if(ct->methodList_OnGlobalConstList == NULL) {
		ct->methodList_OnGlobalConstList = K_EMPTYARRAY;
		if(ct->typeId > TY_Object) {
			ct->searchSuperMethodClassNULL = CT_(ct->superTypeId);
		}
	}
}

static KonohaClass *KonohaClass_define(KonohaContext *kctx, kpackageId_t packageId, kString *name, KDEFINE_CLASS *cdef, KTraceInfo *trace)
{
	KonohaClassVar *ct = new_KonohaClass(kctx, NULL, cdef, trace);
	ct->packageId  = packageId;
	ct->packageDomain = packageId;
	if(name == NULL) {
		const char *n = cdef->structname;
		assert(n != NULL); // structname must be set;
		ct->classNameSymbol = ksymbolSPOL(n, strlen(n), StringPolicy_ASCII|StringPolicy_TEXT, _NEWID);
	}
	else {
		ct->classNameSymbol = ksymbolA(S_text(name), S_size(name), _NEWID);
	}
	KonohaClass_setName(kctx, ct, trace);
	return (KonohaClass *)ct;
}

#define UnboxTypeName(C) \
	.structname = #C,\
	.typeId = TY_##C,\
	.cflag  = CFLAG_##C\

#define TYNAME(C) \
	.structname = #C,\
	.typeId = TY_##C,\
	.cflag = CFLAG_##C,\
	.cstruct_size = sizeof(k##C)\

#define SetUnboxTypeName(VAR, C) do{\
		VAR.structname = #C;\
		VAR.typeId = TY_##C;\
		VAR.cflag  = CFLAG_##C;\
	}while(0)\

#define SETTYNAME(VAR, C) do{\
		VAR.structname = #C;\
		VAR.typeId = TY_##C;\
		VAR.cflag = CFLAG_##C;\
		VAR.cstruct_size = sizeof(k##C);\
	}while(0)\

static void loadInitStructData(KonohaContext *kctx)
{
	KDEFINE_CLASS defTvoid = {0};
	SetUnboxTypeName(defTvoid, void);

	KDEFINE_CLASS defTvar = {0};
	SetUnboxTypeName(defTvar, var);

	KDEFINE_CLASS defObject = {0};
	SETTYNAME(defObject, Object);
	defObject.init = kObject_Init;
	defObject.reftrace = kObject_Reftrace;

	KDEFINE_CLASS defBoolean = {0};
	SetUnboxTypeName(defBoolean, boolean);
	defBoolean.cstruct_size = sizeof(kBoolean);
	defBoolean.init  = kNumber_Init;
	defBoolean.unbox = kNumber_unbox;
	defBoolean.p     = kBoolean_p;
	defBoolean.fnull = kBoolean_fnull;

	KDEFINE_CLASS defInt = {0};
	SetUnboxTypeName(defInt, int);
	defInt.cstruct_size = sizeof(kInt);
	defInt.init  = kNumber_Init;
	defInt.unbox = kNumber_unbox;
	defInt.p     = kInt_p;

	KDEFINE_CLASS defString = {0};
	SETTYNAME(defString, String);
	defString.init = kString_Init;
	defString.free = kString_Free;
	defString.p    = kString_p;
	defString.unbox = kString_unbox;

	KDEFINE_CLASS defArray = {0};
	SETTYNAME(defArray, Array);
	defArray.init = kArray_Init;
	defArray.reftrace = kArray_Reftrace;
	defArray.free = kArray_Free;
	defArray.p    = kArray_p;

	KDEFINE_CLASS defParam = {0};
	SETTYNAME(defParam, Param);
	defParam.init = kParam_Init;

	KDEFINE_CLASS defMethod = {0};
	SETTYNAME(defMethod, Method);
	defMethod.init = kMethod_Init;
	defMethod.reftrace = kMethod_Reftrace;
	defMethod.free     = kMethod_Free;

	KDEFINE_CLASS defFunc = {0};
	SETTYNAME(defFunc, Func);
	defFunc.init = Func_Init;
	defFunc.reftrace = Func_Reftrace;

	KDEFINE_CLASS defNameSpace = {0};
	SETTYNAME(defNameSpace, NameSpace);
	defNameSpace.init = kNameSpace_Init;
	defNameSpace.reftrace = kNameSpace_Reftrace;
	defNameSpace.free = kNameSpace_Free;
	defNameSpace.p    = kNameSpace_p;

	KDEFINE_CLASS defSystem = {0};
	SETTYNAME(defSystem, System);
	defSystem.init = DEFAULT_Init;

	KDEFINE_CLASS defT0 = {0};
	SetUnboxTypeName(defT0, 0);
	defT0.init = DEFAULT_Init;
	defT0.realtype = T_realtype;

	KDEFINE_CLASS *DATATYPES[] = {
		&defTvoid,
		&defTvar,
		&defObject,
		&defBoolean,
		&defInt,
		&defString,
		&defArray,
		&defParam,
		&defMethod,
		&defFunc,
		&defNameSpace,
		&defSystem,
		&defT0,
		NULL,
	};
	KDEFINE_CLASS **dd = DATATYPES;
	kattrtype_t cid = 0;
	while(dd[cid] != NULL) {
		DBG_ASSERT(dd[cid]->typeId == cid);
		new_KonohaClass(kctx, NULL, dd[cid], 0);
		cid++;
	}
	KonohaClassVar *ct = (KonohaClassVar *)CT_Array;
	ct->p0 = TY_Object;    // don't move (ide)
}

static void defineDefaultKeywordSymbol(KonohaContext *kctx)
{
	size_t i;
	static const char *keywords[] = {
		"", "$Expr", "$Symbol", "$Text", "$Number", "$Type",
		"()", "[]", "{}", "$Block", "$Param", "$TypeDecl", "$MethodDecl", "$Token",
		".", "/", "%", "*", "+", "-", "<", "<=", ">", ">=", "==", "!=",
		"&&", "||", "!", "=", ",", "$", ":", ";", /*"@",*/
		"true", "false", "if", "else", "return", // syn
		"new", "void"
	};
	for(i = 0; i < sizeof(keywords) / sizeof(const char *); i++) {
		ksymbolSPOL(keywords[i], strlen(keywords[i]), StringPolicy_TEXT|StringPolicy_ASCII, SYM_NEWID);
		//ksymbol_t sym = ksymbolSPOL(keywords[i], strlen(keywords[i]), StringPolicy_TEXT|StringPolicy_ASCII, SYM_NEWID);
		//fprintf(stdout, "#define KW_%s (((ksymbol_t)%d)|0) /*%s*/\n", SYM_t(sym), Symbol_Unmask(sym), keywords[i]);
	}
}

static void initStructData(KonohaContext *kctx)
{
	KonohaClass **ctt = (KonohaClass**)kctx->share->classTable.classItems;
	size_t i;//, size = kctx->share->classTable.bytesize/sizeof(KonohaClassVar *);
	for(i = 0; i <= TY_0; i++) {
		KonohaClassVar *ct = (KonohaClassVar *)ctt[i];
		const char *name = ct->DBG_NAME;
		ct->classNameSymbol = ksymbolSPOL(name, strlen(name), StringPolicy_ASCII|StringPolicy_TEXT, _NEWID);
		KonohaClass_setName(kctx, ct, 0);
	}
	KLIB Knull(kctx, CT_NameSpace);
}

static void initKonohaLib(KonohaLibVar *l)
{
	l->Kclass                  = Kclass;
	l->new_kObject             = new_kObject;
	l->new_kString             = new_kString;
	l->kObject_WriteToBuffer   = kObject_WriteToBuffer;

	l->kArray_Add           = kArray_Add;
	l->kArray_Insert        = kArray_Insert;
	l->kArray_Clear         = kArray_Clear;

	l->KonohaClass_define   = KonohaClass_define;
	l->Knull = Knull;
	l->KonohaClass_shortName = KonohaClass_shortName;
	l->KonohaClass_Generics = KonohaClass_Generics;

	l->new_kMethod          = new_kMethod;
	l->Kparamdom            = Kparamdom;
	l->kMethod_SetParam     = kMethod_SetParam;
	l->kMethod_indexOfField = STUB_Method_indexOfField;
}

static void KonohaRuntime_Init(KonohaContext *kctx, KonohaContextVar *ctx)
{
	KonohaRuntimeVar *share = (KonohaRuntimeVar *)KCalloc_UNTRACE(sizeof(KonohaRuntime), 1);
	ctx->share = share;
	KInitLock(share->classTableMutex);
	KInitLock(share->filepackMutex);
	KInitLock(share->symbolMutex);
	KInitLock(share->paramMutex);

	initKonohaLib((KonohaLibVar *)kctx->klib);
	KLIB KArray_Init(kctx, &share->classTable, K_CLASSTABLE_INITSIZE * sizeof(KonohaClass));
	loadInitStructData(kctx);

	KUnsafeFieldInit(share->GlobalConstList, new_(Array, 8, OnField));

	share->longClassNameMapNN = KLIB KHashMap_Init(kctx, 0);
	share->fileIdList_OnGlobalConstList         = new_(StringArray, 8, OnGlobalConstList);
	share->fileIdMap_KeyOnList        = KLIB KHashMap_Init(kctx, 0);
	share->packageIdList_OnGlobalConstList      = new_(StringArray, 8, OnGlobalConstList);
	share->packageIdMap_KeyOnList     = KLIB KHashMap_Init(kctx, 0);
	share->packageMapNO       = KLIB KHashMap_Init(kctx, 0);

	share->symbolList_OnGlobalConstList         = new_(StringArray, 32, OnGlobalConstList);
	share->symbolMap_KeyOnList        = KLIB KHashMap_Init(kctx, 0);
	share->paramMap_KeyOnList         = KLIB KHashMap_Init(kctx, 0);
	share->paramList_OnGlobalConstList          = new_(Array, 32, OnGlobalConstList);
	share->paramdomMap_KeyOnList      = KLIB KHashMap_Init(kctx, 0);
	share->paramdomList_OnGlobalConstList       = new_(Array, 32, OnGlobalConstList);
	//
	share->constNull_OnGlobalConstList =  new_(Object, NULL, OnGlobalConstList);
	kObject_set(NullObject, share->constNull_OnGlobalConstList, true);
	share->constTrue_OnGlobalConstList =   new_(Boolean, 1, OnGlobalConstList);
	share->constFalse_OnGlobalConstList =  new_(Boolean, 0, OnGlobalConstList);
	share->emptyString_OnGlobalConstList = new_(String, NULL, OnGlobalConstList);
	share->emptyArray_OnGlobalConstList =  new_(Array, 0, OnGlobalConstList);

	Kparam(kctx, TY_void, 0, NULL);  // PARAM_void
	Kparamdom(kctx, 0, NULL);        // PARAMDOM_void
	kparamtype_t p = {TY_Object};
	Kparamdom(kctx, 1, &p);          // PARAMDOM_DefaultGenericsParam  1
	FILEID_("(konoha.c)");
	PN_("konoha");                   // PN_konoha
	PN_("sugar");                    // PKG_sugar
	defineDefaultKeywordSymbol(kctx);
	initStructData(kctx);
}

static void KonohaRuntime_Reftrace(KonohaContext *kctx, KonohaContextVar *ctx, KObjectVisitor *visitor)
{
	KonohaRuntime *share = ctx->share;
	KRefTrace(share->GlobalConstList);
}

static void KonohaRuntime_FreeClassTable(KonohaContext *kctx)
{
	KonohaClassVar **cts = (KonohaClassVar**)kctx->share->classTable.classItems;
	size_t i, size = kctx->share->classTable.bytesize/sizeof(KonohaClassVar *);
	for(i = 0; i < size; i++) {
		if(cts[i]->fieldAllocSize > 0) {
			KFree(cts[i]->fieldItems, cts[i]->fieldAllocSize * sizeof(KonohaClassField));
		}
		KFree(cts[i], sizeof(KonohaClass));
	}
}

static void packageMap_Free(KonohaContext *kctx, void *p)
{
	KFree(p, sizeof(KonohaPackage));
}

static void KonohaRuntime_Free(KonohaContext *kctx, KonohaContextVar *ctx)
{
	KonohaRuntimeVar *share = (KonohaRuntimeVar *)ctx->share;
	KLIB KHashMap_Free(kctx, share->longClassNameMapNN, NULL);
	KLIB KHashMap_Free(kctx, share->fileIdMap_KeyOnList, NULL);
	KLIB KHashMap_Free(kctx, share->packageIdMap_KeyOnList, NULL);
	KLIB KHashMap_Free(kctx, share->packageMapNO, packageMap_Free);
	KLIB KHashMap_Free(kctx, share->symbolMap_KeyOnList, NULL);
	KLIB KHashMap_Free(kctx, share->paramMap_KeyOnList, NULL);
	KLIB KHashMap_Free(kctx, share->paramdomMap_KeyOnList, NULL);

	KonohaRuntime_FreeClassTable(kctx);
	KLIB KArray_Free(kctx, &share->classTable);

	KFreeLock(share->classTableMutex);
	KFreeLock(share->filepackMutex);
	KFreeLock(share->symbolMutex);
	KFreeLock(share->paramMutex);
	KFree(share, sizeof(KonohaRuntime));
}

