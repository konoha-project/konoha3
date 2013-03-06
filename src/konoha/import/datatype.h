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

#define PARAM_void     0
#define PARAMDOM_void                  0
#define PARAMDOM_DefaultGenericsParam  1

static kObject* DEFAULT_fnull(KonohaContext *kctx, KClass *ct);
static void kArray_Add(KonohaContext *kctx, kArray *o, kAbstractObject *);

static void kObject_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kObjectVar *of = (kObjectVar *)o;
	of->fieldUnboxItems[0] = 0;
}

static void kObject_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kObject *of = (kObject *)o;
	KClass *ct = kObject_class(of);
	size_t i;
	for(i = 0; i < ct->fieldsize; i++) {
		if(KTypeAttr_Is(Boxed, ct->fieldItems[i].attrTypeId)) {
			KRefTrace(of->fieldObjectItems[i]);
		}
	}
}

static kObject *new_kObject(KonohaContext *kctx, kArray *gcstackNULL, KClass *ct, uintptr_t conf)
{
	DBG_ASSERT(ct->cstruct_size > 0);
	kObjectVar *o = PLATAPI GCModule.AllocObject(kctx, ct->cstruct_size, NULL/*FIXME*/);
	o->h.magicflag = ct->magicflag;
	o->h.ct = ct;
	o->h.prototypePtr = NULL; /* (KProtoMap *) Kprotomap_new(0); */
	if(gcstackNULL != NULL) {
		kArray_Add(kctx, gcstackNULL, o);
	}
	ct->init(kctx, (kObject *)o, (void *)conf);
	return (kObject *)o;
}

static void KClass_WriteUnboxValueToBuffer(KonohaContext *kctx, KClass *c, uintptr_t unboxValue, int isDelim, KBuffer *wb)
{
	if(isDelim > 0) {
		KLIB KBuffer_Write(kctx, wb, ", ", 2);
	}
	KonohaValue v = {};
	v.unboxValue = unboxValue;
	c->format(kctx, &v, 0, wb);
}

static void kObject_WriteToBuffer(KonohaContext *kctx, kObject *o, int isDelim, KBuffer *wb, KonohaValue *sfp, int pos)
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
		if(KType_Is(UnboxType, kObject_typeId(o))) {
			sfp[pos].unboxValue = kObject_Unbox(o);
		}
		else {
			KUnsafeFieldSet(sfp[pos].asObject, o);
		}
		kObject_class(o)->format(kctx, sfp, pos, wb);
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

static void kBoolean_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	if(v[pos].boolValue) {
		KLIB KBuffer_Write(kctx, wb, TEXTSIZE("true"));
	}
	else {
		KLIB KBuffer_Write(kctx, wb, TEXTSIZE("false"));
	}
}

static kObject* kBoolean_fnull(KonohaContext *kctx, KClass *ct)
{
	return (kObject *)K_FALSE;
}

static void kInt_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	KLIB KBuffer_printf(kctx, wb, KINT_FMT, v[pos].intValue);
}

// String
static void kString_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kStringVar *s = (kStringVar *)o;
	s->text = "";
	s->bytesize = 0;
	kString_Set(TextSgm, s, true);
}

static void kString_Free(KonohaContext *kctx, kObject *o)
{
	kString *s = (kString *)o;
	if(!kString_Is(TextSgm, s)) {
		KFree(s->buf, kString_size(s) + 1);
	}
}

static void kString_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	const char *t = kString_text(v[pos].asString);
	size_t i, len = kString_size(v[pos].asString);
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

static int kString_compareTo(KonohaContext *kctx, kObject *thisString, kObject *thatString)
{
	size_t llen = kString_size((kString *) thisString);
	size_t rlen = kString_size((kString *) thatString);
	size_t n = (llen > rlen) ? llen : rlen;
	const char *ltext = kString_text((kString *) thisString);
	const char *rtext = kString_text((kString *) thatString);
	int ret = strncmp(ltext, rtext, n);
	return ret == 0 ? (int)(llen - rlen) : ret;
}

static void kString_CheckASCII(KonohaContext *kctx, kString *s)
{
	unsigned char ch = 0;
	long len = kString_size(s), n = (len + 3) / 4;
	const unsigned char*p = (const unsigned char *)kString_text(s);
	switch(len % 4) { /* Duff's device written by ide */
		case 0: do{ ch |= *p++;
		case 3:     ch |= *p++;
		case 2:     ch |= *p++;
		case 1:     ch |= *p++;
		} while(--n>0);
	}
	kString_Set(ASCII, (kStringVar *)s, (ch < 128));
}

static kString* new_kString(KonohaContext *kctx, kArray *gcstack, const char *text, size_t len, int spol)
{
	KClass *ct = KClass_(KType_String);
	kStringVar *s = NULL;
	if(KFlag_Is(int, spol, StringPolicy_TEXT)) {
		s = (kStringVar *)new_kObject(kctx, gcstack, ct, 0);
		s->text = text;
		s->bytesize = len;
		kString_Set(TextSgm, s, 1);
		kString_Set(MallocText, s, 1);
	}
	else if(len + 1 < sizeof(void *) * 2) {
		s = (kStringVar *)new_kObject(kctx, gcstack, ct, 0);
		s->text = s->inline_text;
		s->bytesize = len;
		kString_Set(TextSgm, s, 1);
		if(text != NULL) {
			DBG_ASSERT(!KFlag_Is(int, spol, StringPolicy_NOCOPY));
			memcpy(s->ubuf, text, len);
		}
		s->buf[len] = '\0';
	}
	else {
		s = (kStringVar *)new_kObject(kctx, gcstack, ct, 0);
		s->bytesize = len;
		s->buf = (char *)KMalloc_UNTRACE(len+1);
		kString_Set(TextSgm, s, 0);
		kString_Set(MallocText, s, 1);
		if(text != NULL) {
			DBG_ASSERT(!KFlag_Is(int, spol, StringPolicy_NOCOPY));
			memcpy(s->ubuf, text, len);
		}
		s->buf[len] = '\0';
	}
	if(KFlag_Is(int, spol, StringPolicy_ASCII)) {
		kString_Set(ASCII, s, 1);
	}
	else if(KFlag_Is(int, spol, StringPolicy_UTF8)) {
		kString_Set(ASCII, s, 0);
	}
	else {
		kString_CheckASCII(kctx, s);
	}
	return s;
}

// Array
struct _kAbstractArray {
	kObjectHeader h;
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
	if(KType_Is(UnboxType, kObject_p0(a))) {
		kArray_Set(UnboxData, a, 1);
	}
}

static void kArray_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kArray *a = (kArray *)o;
	if(!kArray_Is(UnboxData, a)) {
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

static void kArray_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	size_t i;
	kArray *a = v[pos].asArray;
	KLIB KBuffer_Write(kctx, wb, "[", 1);
	if(kArray_Is(UnboxData, a)) {
		KClass *c = KClass_(kObject_p0(a));
		for(i = 0; i < kArray_size(a); i++) {
			KClass_WriteUnboxValueToBuffer(kctx, c, a->unboxItems[i], (i > 0)/*delim*/, wb);
		}
	}
	else {
		for(i = 0; i < kArray_size(a); i++) {
			kObject *o = a->ObjectItems[i];
			kObject_WriteToBuffer(kctx, o, (i>0)/*delim*/, wb, v, pos+1);
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

static KClass *KClass_extendedBody(KonohaContext *kctx, KClass *ct, size_t head, size_t body);

static void kParam_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kParamVar *pa = (kParamVar *)o;
	pa->psize = 0;
	pa->rtype = KType_void;
}

static kParam *new_Param(KonohaContext *kctx, kArray *gcstack, ktypeattr_t rtype, int psize, const kparamtype_t *p)
{
	KClass *ct = KClass_(KType_Param);
	ct = KClass_extendedBody(kctx, ct, sizeof(void *), psize * sizeof(kparamtype_t));
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

static uintptr_t hashparam(ktypeattr_t rtype, kushort_t psize, const kparamtype_t *p)
{
	kushort_t i;
	uintptr_t hcode = rtype;
	for(i = 0; i < psize; i++) {
		hcode = (p[i].attrTypeId + p[i].name) + (31 * hcode);
	}
	return hcode;
}

static kbool_t equalsParamDom(ktypeattr_t rtype, kushort_t psize, const kparamtype_t *p, kParam *pa)
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

static kbool_t equalsParam(ktypeattr_t rtype, kushort_t psize, const kparamtype_t *p, kParam *pa)
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

typedef kbool_t (*equalsP)(ktypeattr_t rtype, kushort_t psize, const kparamtype_t *p, kParam *pa);

static kparamId_t KHashMap_getparamid(KonohaContext *kctx, KHashMap *kmp, kArray *list, uintptr_t hcode, equalsP f, ktypeattr_t rtype, kushort_t psize, const kparamtype_t *p)
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

static kparamId_t Kparam(KonohaContext *kctx, ktypeattr_t rtype, kushort_t psize, const kparamtype_t *p)
{
	uintptr_t hcode = hashparam(rtype, psize, p);
	KLock(kctx->share->paramMutex);
	kparamId_t param = KHashMap_getparamid(kctx, kctx->share->paramMap_KeyOnList, kctx->share->paramList, hcode, equalsParam, rtype, psize, p);
	KUnlock(kctx->share->paramMutex);
	return param;
}

static kparamId_t Kparamdom(KonohaContext *kctx, kushort_t psize, const kparamtype_t *p)
{
	uintptr_t hcode = hashparamdom(psize, p);
	KLock(kctx->share->paramMutex);
	kparamId_t param = KHashMap_getparamid(kctx, kctx->share->paramdomMap_KeyOnList, kctx->share->paramdomList, hcode, equalsParamDom, KType_void, psize, p);
	KUnlock(kctx->share->paramMutex);
	return param;
}

/* --------------- */
/* Method */

static uintptr_t methodSerialNumber = 0;

static void kMethod_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kMethodVar *mtd = (kMethodVar *)o;
	bzero(&mtd->invokeKMethodFunc, sizeof(kMethod) - sizeof(kObjectHeader));
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

static void kMethod_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kMethod *mtd = v[pos].asMethod;
	KBuffer_printf(kctx, wb, "%s.%s%s", kMethod_Fmt3(mtd));
}

#define KClass_MethodVar KClass_Method
static kMethodVar* new_kMethod(KonohaContext *kctx, kArray *gcstack, uintptr_t flag, ktypeattr_t cid, kmethodn_t mn, KMethodFunc func)
{
	kMethodVar* mtd = new_(MethodVar, NULL, gcstack);
	mtd->flag       = flag;
	mtd->typeId     = cid;
	mtd->mn         = mn;
	KLIB kMethod_SetFunc(kctx, mtd, func);
	return mtd;
}

static kParam* kMethod_SetParam(KonohaContext *kctx, kMethod *mtd_, ktypeattr_t rtype, kushort_t psize, const kparamtype_t *p)
{
	kparamId_t paramId = Kparam(kctx, rtype, psize, p);
	if(mtd_ != NULL) {
		kMethodVar* mtd = (kMethodVar *)mtd_;
		mtd->paramdom = Kparamdom(kctx, psize, p);
		mtd->paramid  = paramId;
	}
	return kctx->share->paramList->ParamItems[paramId];
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
	bzero(&ns->parentNULL, sizeof(kNameSpace) - sizeof(kObjectHeader));
	KFieldInit(ns, ns->NameSpaceConstList, new_(Array, 0, OnField));
	ns->syntaxOption = kNameSpace_DefaultSyntaxOption;
	if(conf != NULL) {
		KFieldInit(ns, ns->parentNULL, (kNameSpace *)conf);
		ns->packageId     = ns->parentNULL->packageId;
		ns->syntaxOption  = ns->parentNULL->syntaxOption;
	}
	ns->importedNameSpaceList = K_EMPTYARRAY;
	ns->methodList_OnList = K_EMPTYARRAY;
	ns->metaPatternList = K_EMPTYARRAY;
	ns->builderApi = PLATAPI ExecutionEngineModule.GetDefaultBuilderAPI();
	KLIB KDict_Init(kctx, &(ns->constTable));
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

static void kNameSpace_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kNameSpace *ns = v[pos].asNameSpace;
	KLIB KBuffer_printf(kctx, wb, "%s", KPackage_text(ns->packageId));
}

// ---------------
// System

static void Func_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kFuncVar *fo = (kFuncVar *)o;
//	KFieldInit(fo, fo->self, K_NULL);
	KFieldInit(fo, fo->method, conf == NULL ? KNULL(Method) : (kMethod *)conf);
}

static void Func_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kFunc *fo = (kFunc *)o;
//	KRefTrace(fo->self);
	KRefTrace(fo->method);
}

// ---------------
// Exception

static void kException_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kExceptionVar *e = (kExceptionVar *)o;
	kString *msg = conf == NULL ? TS_EMPTY : (kString *)conf;
	KFieldInit(e, e->Message, msg);
	//DBG_ASSERT(IS_String(msg));
	e->uline  = 0;
	e->symbol = 0;
	e->fault  = 0;
	KFieldInit(e, e->StackTraceList, K_EMPTYARRAY);
}

static void kException_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	kException *e = (kException *)o;
	KRefTrace(e->Message);
	KRefTrace(e->StackTraceList);
}

static void kException_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
//	kException *mtd = v[pos].asException;
//	KBuffer_printf(kctx, wb, "%s.%s%s", kException_Fmt3(mtd));
}

// ---------------

static KClass *T_realtype(KonohaContext *kctx, KClass *ct, KClass *self)
{
	kParam *cparam = KClass_cparam(self);
	//DBG_P("ct=%s, self=%s", KClass_text(ct), KClass_text(self));
	DBG_ASSERT(ct->optvalue < cparam->psize);
	KClass *pct = KClass_(cparam->paramtypeItems[ct->optvalue].attrTypeId);
	return pct->realtype(kctx, pct, self);
}

// ---------------

static KClass* Kclass(KonohaContext *kctx, ktypeattr_t cid, KTraceInfo *trace)
{
	KRuntime *share = kctx->share;
	if(!(cid < (share->classTable.bytesize/sizeof(KClassVar *)))) {
		KLIB KRuntime_raise(kctx, KException_("InvalidParameter"), SoftwareFault, NULL, trace->baseStack);
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

static void DEFAULT_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	if(IS_NULL(v[pos].asObject)) {
		KLIB KBuffer_Write(kctx, wb, TEXTSIZE("null"));
	}
	else {
		KLIB KBuffer_printf(kctx, wb, "&%p", v[pos].asObject);
	}
}

static uintptr_t DEFAULT_unbox(KonohaContext *kctx, kObject *o)
{
	return 0;
}

static kbool_t DEFAULT_isSubType(KonohaContext *kctx, KClass* ct, KClass *t)
{
	if(t->typeId == KType_Object) return true;
	while(ct->superTypeId != KType_Object) {
		ct = KClass_(ct->superTypeId);
		if(ct->typeId == t->typeId) return true;
	}
	return false;
}

static KClass* DEFAULT_realtype(KonohaContext *kctx, KClass* c, KClass *self)
{
	return c;
}

static kObject* DEFAULT_fnull(KonohaContext *kctx, KClass *ct)
{
	DBG_ASSERT(ct->defaultNullValue != NULL);
	return ct->defaultNullValue;
}

static kObject* DEFAULT_fnullinit(KonohaContext *kctx, KClass *ct)
{
	DBG_ASSERT(ct->defaultNullValue == NULL);
	((KClassVar *)ct)->defaultNullValue = KLIB new_kObject(kctx, OnGlobalConstList, ct, 0);
	kObject_Set(NullObject, ct->defaultNullValue, true);
	((KClassVar *)ct)->fnull = DEFAULT_fnull;
	return ct->defaultNullValue;
}

static int DEFAULT_compareTo(KonohaContext *kctx, kObject *o1, kObject *o2)
{
	return (int)(o1 - o2);
}

static int DEFAULT_compareUnboxValue(uintptr_t v1, uintptr_t v2)
{
	return v1 - v2;
}

static kObject *Knull(KonohaContext *kctx, KClass *ct)
{
	return ct->fnull(kctx, ct);
}

static KClassVar* new_KClass(KonohaContext *kctx, KClass *bct, KDEFINE_CLASS *s, KTraceInfo *trace)
{
	KRuntimeVar *share = (KRuntimeVar *)kctx->share;
	KClassVar *ct;
	ktypeattr_t newid;
	KLock(share->classTableMutex); {
		newid = share->classTable.bytesize / sizeof(KClassVar *);
		if(share->classTable.bytesize == share->classTable.bytemax) {
			KLIB KArray_Expand(kctx, &share->classTable, share->classTable.bytemax * 2);
		}
		share->classTable.bytesize += sizeof(KClassVar *);
		ct = (KClassVar *)KCalloc_UNTRACE(sizeof(KClass), 1);
		share->classTable.classItems[newid] = (KClass *)ct;
	}
	KUnlock(share->classTableMutex);
	if(bct != NULL) {
		DBG_ASSERT(s == NULL);
		memcpy(ct, bct, offsetof(KClass, classMethodList));
		ct->typeId = newid;
		if(ct->fnull == DEFAULT_fnull) ct->fnull =  DEFAULT_fnullinit;
	}
	else {
		DBG_ASSERT(s != NULL);
		ct->cflag   = s->cflag;
		ct->typeId = newid;
		ct->baseTypeId    = (s->baseTypeId == 0) ? ct->typeId : s->baseTypeId;
		ct->superTypeId  = (s->superTypeId == 0) ? KType_Object : s->superTypeId;
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
		ct->format   = (s->format != NULL) ? s->format : DEFAULT_format;
		ct->compareTo   = (s->compareTo != NULL) ? s->compareTo : DEFAULT_compareTo;
		ct->compareUnboxValue = (s->compareUnboxValue != NULL) ? s->compareUnboxValue : DEFAULT_compareUnboxValue;
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

static KClass *KClass_extendedBody(KonohaContext *kctx, KClass *ct, size_t head, size_t body)
{
	KClass *bct = ct;
	while(ct->cstruct_size < sizeof(kObjectHeader) + head + body) {
		if(ct->searchSimilarClassNULL == NULL) {
			KClassVar *newct = new_KClass(kctx, bct, NULL, NOPLINE);
			newct->cflag |= KClassFlag_Private;
			newct->cstruct_size = ct->cstruct_size * 2;
			newct->classMethodList = ct->classMethodList;
			((KClassVar *)ct)->searchSimilarClassNULL = (KClass *)newct;
		}
		ct = ct->searchSimilarClassNULL;
	}
	return ct;
}

static KClass *Generics_realtype(KonohaContext *kctx, KClass *ct, KClass *self)
{
	//DBG_P("trying resolve generic type: %s %s", KClass_text(ct), KClass_text(self));
	kParam *param = KClass_cparam(ct);
	kushort_t i;
	kparamtype_t *p = ALLOCA(kparamtype_t, param->psize);
	for(i = 0; i < param->psize; i++) {
		KClass *cParam = KClass_(param->paramtypeItems[i].attrTypeId);
		p[i].attrTypeId = cParam->realtype(kctx, cParam, self)->typeId;
	}
	return KLIB KClass_Generics(kctx, ct, KType_void, param->psize, p);
}

static KClass *Func_realtype(KonohaContext *kctx, KClass *ct, KClass *self)
{
	//DBG_P("trying resolve generic type: %s %s", KClass_text(ct), KClass_text(self));
	KClass *cReturn = KClass_(ct->p0);
	ktypeattr_t rtype = cReturn->realtype(kctx, cReturn, self)->typeId;
	kParam *param = KClass_cparam(ct);
	kushort_t i;
	kparamtype_t *p = ALLOCA(kparamtype_t, param->psize);
	for(i = 0; i < param->psize; i++) {
		KClass *cParam = KClass_(param->paramtypeItems[i].attrTypeId);
		p[i].attrTypeId = cParam->realtype(kctx, cParam, self)->typeId;
	}
	return KLIB KClass_Generics(kctx, KClass_(ct->baseTypeId), rtype, param->psize, p);
}

#define KType_IsTypeVar2(T)   (T != KType_void && KType_Is(TypeVar, T))

static void checkTypeVar(KonohaContext *kctx, KClassVar *newct, ktypeattr_t rtype, int psize, kparamtype_t *p)
{
	int i, isTypeVar = KType_IsTypeVar2(rtype);
	if(!isTypeVar) {
		for(i = 0; i < psize; i++) {
			if(KType_Is(TypeVar, p[i].attrTypeId)) {
				isTypeVar = true;
			}
		}
	}
	if(isTypeVar) {
		//DBG_P("Generics %s has TypeVar", KClass_text(newct));
		newct->cflag |= KClassFlag_TypeVar;
		newct->realtype = newct->baseTypeId == KType_Func ? Func_realtype : Generics_realtype;
	}
}

static KClass *KClass_Generics(KonohaContext *kctx, KClass *ct, ktypeattr_t rtype, kushort_t psize, kparamtype_t *p)
{
	kparamId_t paramdom = Kparamdom(kctx, psize, p);
	KClass *ct0 = KClass_(ct->baseTypeId);
	ct = ct0;
	int isNotFuncClass = (ct->baseTypeId != KType_Func);
	do {
		if(ct->cparamdom == paramdom && (isNotFuncClass || ct->p0 == rtype)) {
			return ct;
		}
		if(ct->searchSimilarClassNULL == NULL) break;
		ct = ct->searchSimilarClassNULL;
	} while(ct != NULL);

	KClassVar *newct = new_KClass(kctx, ct0, NULL, NOPLINE);
	newct->cparamdom = paramdom;
	newct->p0 = isNotFuncClass ? p[0].attrTypeId : rtype;
	newct->classMethodList = K_EMPTYARRAY;
	if(newct->searchSuperMethodClassNULL == NULL) {
		newct->searchSuperMethodClassNULL = ct0;
	}
	checkTypeVar(kctx, newct, rtype, psize, p);
	((KClassVar *)ct)->searchSimilarClassNULL = (KClass *)newct;
	return ct->searchSimilarClassNULL;
}

static kString* KClass_shortName(KonohaContext *kctx, KClass *ct)
{
	if(ct->shortClassNameNULL == NULL) {
		if(ct->cparamdom == 0 && ct->baseTypeId != KType_Func) {
			((KClassVar *)ct)->shortClassNameNULL = KSymbol_GetString(kctx, ct->classNameSymbol);
		}
		else {
			size_t i, c = 0;
			kParam *cparam = KClass_cparam(ct);
			KBuffer wb;
			KClass_shortName(kctx, KClass_(ct->p0));
			for(i = 0; i < cparam->psize; i++) {
				KClass_shortName(kctx, KClass_(cparam->paramtypeItems[i].attrTypeId));
			}
			KBuffer_Init(&(kctx->stack->cwb), &wb);
			kString *s = KSymbol_GetString(kctx, ct->classNameSymbol);
			KLIB KBuffer_Write(kctx, &wb, kString_text(s), kString_size(s));
			KLIB KBuffer_Write(kctx, &wb, "[", 1);
			if(ct->baseTypeId == KType_Func) {
				s = KClass_shortName(kctx, KClass_(ct->p0));
				KLIB KBuffer_Write(kctx, &wb, kString_text(s), kString_size(s)); c++;
			}
			for(i = 0; i < cparam->psize; i++) {
				if(c > 0) KLIB KBuffer_Write(kctx, &wb, ",", 1);
				s = KClass_shortName(kctx, KClass_(cparam->paramtypeItems[i].attrTypeId));
				KLIB KBuffer_Write(kctx, &wb, kString_text(s), kString_size(s));
			}
			KLIB KBuffer_Write(kctx, &wb, "]", 1);
			((KClassVar *)ct)->shortClassNameNULL = KLIB KBuffer_Stringfy(kctx, &wb, OnGlobalConstList, StringPolicy_ASCII|StringPolicy_FreeKBuffer);
		}
	}
	return ct->shortClassNameNULL;
}

static void KClass_SetName(KonohaContext *kctx, KClassVar *ct, KTraceInfo *trace)
{
	if(trace != NULL) {
		/* To avoid SEGV, because this message is called at the initial time. */
		KLIB ReportScriptMessage(kctx, trace, DebugTag, "new class %s.%s", KPackage_text(ct->packageId), KSymbol_text(ct->classNameSymbol));
	}
	if(ct->classMethodList == NULL) {
		ct->classMethodList = K_EMPTYARRAY;
		if(ct->typeId > KType_Object) {
			ct->searchSuperMethodClassNULL = KClass_(ct->superTypeId);
		}
	}
}

static KClass *KClass_define(KonohaContext *kctx, kpackageId_t packageId, kString *name, KDEFINE_CLASS *cdef, KTraceInfo *trace)
{
	KClassVar *ct = new_KClass(kctx, NULL, cdef, trace);
	ct->packageId  = packageId;
	ct->packageDomain = packageId;
	if(name == NULL) {
		const char *n = cdef->structname;
		assert(n != NULL); // structname must be set;
		ct->classNameSymbol = ksymbolSPOL(n, strlen(n), StringPolicy_ASCII|StringPolicy_TEXT, _NEWID);
	}
	else {
		ct->classNameSymbol = KAsciiSymbol(kString_text(name), kString_size(name), _NEWID);
	}
	KClass_SetName(kctx, ct, trace);
	return (KClass *)ct;
}

#define UnboxTypeName(C) \
	.structname = #C,\
	.typeId = KType_##C,\
	.cflag  = KClassFlag_##C\

#define TYNAME(C) \
	.structname = #C,\
	.typeId = KType_##C,\
	.cflag = KClassFlag_##C,\
	.cstruct_size = sizeof(k##C)\

#define SetUnboxTypeName(VAR, C) do{\
		VAR.structname = #C;\
		VAR.typeId = KType_##C;\
		VAR.cflag  = KClassFlag_##C;\
	}while(0)\

#define SETTYNAME(VAR, C) do{\
		VAR.structname = #C;\
		VAR.typeId = KType_##C;\
		VAR.cflag = KClassFlag_##C;\
		VAR.cstruct_size = sizeof(k##C);\
	}while(0)\

static void LoadInitStructData(KonohaContext *kctx)
{
	KDEFINE_CLASS defTvoid = {0};
	SetUnboxTypeName(defTvoid, void);

	KDEFINE_CLASS defTvar = {0};
	SetUnboxTypeName(defTvar, var);

	KDEFINE_CLASS defObject = {0};
	SETTYNAME(defObject, Object);
	defObject.init = kObject_Init;
	defObject.reftrace = kObject_Reftrace;

#define KType_boolean KType_Boolean
	KDEFINE_CLASS defBoolean = {0};
	SetUnboxTypeName(defBoolean, boolean);
	defBoolean.cstruct_size = sizeof(kBoolean);
	defBoolean.init  = kNumber_Init;
	defBoolean.unbox = kNumber_unbox;
	defBoolean.format     = kBoolean_format;
	defBoolean.fnull = kBoolean_fnull;

#define KType_int KType_Int
	KDEFINE_CLASS defInt = {0};
	SetUnboxTypeName(defInt, int);
	defInt.cstruct_size = sizeof(kInt);
	defInt.init  = kNumber_Init;
	defInt.unbox = kNumber_unbox;
	defInt.format     = kInt_format;

	KDEFINE_CLASS defString = {0};
	SETTYNAME(defString, String);
	defString.init = kString_Init;
	defString.free = kString_Free;
	defString.format    = kString_format;
	defString.compareTo = kString_compareTo;
	defString.unbox = kString_unbox;

	KDEFINE_CLASS defArray = {0};
	SETTYNAME(defArray, Array);
	defArray.init = kArray_Init;
	defArray.reftrace = kArray_Reftrace;
	defArray.free = kArray_Free;
	defArray.format    = kArray_format;

	KDEFINE_CLASS defParam = {0};
	SETTYNAME(defParam, Param);
	defParam.init = kParam_Init;

	KDEFINE_CLASS defMethod = {0};
	SETTYNAME(defMethod, Method);
	defMethod.init = kMethod_Init;
	defMethod.reftrace = kMethod_Reftrace;
	defMethod.free     = kMethod_Free;
	defMethod.format        = kMethod_format;

	KDEFINE_CLASS defFunc = {0};
	SETTYNAME(defFunc, Func);
	defFunc.init = Func_Init;
	defFunc.reftrace = Func_Reftrace;

	KDEFINE_CLASS defException = {0};
	SETTYNAME(defException, Exception);
	defException.init = kException_Init;
	defException.reftrace = kException_Reftrace;
	defException.format = kException_format;

	KDEFINE_CLASS defNameSpace = {0};
	SETTYNAME(defNameSpace, NameSpace);
	defNameSpace.init = kNameSpace_Init;
	defNameSpace.reftrace = kNameSpace_Reftrace;
	defNameSpace.free = kNameSpace_Free;
	defNameSpace.format    = kNameSpace_format;

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
		&defException,
		&defNameSpace,
		&defSystem,
		&defT0,
		NULL,
	};
	KDEFINE_CLASS **dd = DATATYPES;
	ktypeattr_t cid = 0;
	while(dd[cid] != NULL) {
		DBG_ASSERT(dd[cid]->typeId == cid);
		new_KClass(kctx, NULL, dd[cid], 0);
		cid++;
	}
	KClassVar *ct = (KClassVar *)KClass_Array;
	ct->p0 = KType_Object;    // don't move (ide)
}

static void DefineDefaultKeywordSymbol(KonohaContext *kctx)
{
	size_t i;
	for(i = 0; i < sizeof(KEYWORD_LIST) / sizeof(const char *); i++) {
		ksymbolSPOL(KEYWORD_LIST[i], strlen(KEYWORD_LIST[i]), StringPolicy_TEXT|StringPolicy_ASCII, KSymbol_NewId);
		//ksymbol_t sym = ksymbolSPOL(keywords[i], strlen(keywords[i]), StringPolicy_TEXT|StringPolicy_ASCII, KSymbol_NewId);
		//fprintf(stdout, "#define KSymbol_%s (((ksymbol_t)%d)|0) /*%s*/\n", KSymbol_text(sym), KSymbol_Unmask(sym), keywords[i]);
	}
}

static void InitStructData(KonohaContext *kctx)
{
	KClass **ctt = (KClass**)kctx->share->classTable.classItems;
	size_t i;//, size = kctx->share->classTable.bytesize/sizeof(KClassVar *);
	for(i = 0; i <= KType_0; i++) {
		KClassVar *ct = (KClassVar *)ctt[i];
		const char *name = ct->DBG_NAME;
		ct->classNameSymbol = ksymbolSPOL(name, strlen(name), StringPolicy_ASCII|StringPolicy_TEXT, _NEWID);
		KClass_SetName(kctx, ct, 0);
	}
	KLIB Knull(kctx, KClass_NameSpace);
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

	l->KClass_define   = KClass_define;
	l->Knull = Knull;
	l->KClass_shortName = KClass_shortName;
	l->KClass_Generics = KClass_Generics;

	l->new_kMethod          = new_kMethod;
	l->Kparamdom            = Kparamdom;
	l->kMethod_SetParam     = kMethod_SetParam;
	l->kMethod_indexOfField = STUB_Method_indexOfField;
}

static void KRuntime_Init(KonohaContext *kctx, KonohaContextVar *ctx)
{
	KRuntimeVar *share = (KRuntimeVar *)KCalloc_UNTRACE(sizeof(KRuntime), 1);
	ctx->share = share;
	KInitLock(share->classTableMutex);
	KInitLock(share->filepackMutex);
	KInitLock(share->symbolMutex);
	KInitLock(share->paramMutex);

	initKonohaLib((KonohaLibVar *)kctx->klib);
	KLIB KArray_Init(kctx, &share->classTable, K_CLASSTABLE_INITSIZE * sizeof(KClass));
	LoadInitStructData(kctx);

	KUnsafeFieldInit(share->GlobalConstList, new_(Array, 8, OnField));

	share->longClassNameMapNN = KLIB KHashMap_Init(kctx, 0);
	share->fileIdList         = new_(StringArray, 8, OnGlobalConstList);
	share->fileIdMap_KeyOnList        = KLIB KHashMap_Init(kctx, 0);
	share->packageIdList      = new_(StringArray, 8, OnGlobalConstList);
	share->packageIdMap_KeyOnList     = KLIB KHashMap_Init(kctx, 0);
	share->packageMapNO       = KLIB KHashMap_Init(kctx, 0);

	share->symbolList         = new_(StringArray, 32, OnGlobalConstList);
	share->symbolMap_KeyOnList        = KLIB KHashMap_Init(kctx, 0);
	share->paramMap_KeyOnList         = KLIB KHashMap_Init(kctx, 0);
	share->paramList          = new_(Array, 32, OnGlobalConstList);
	share->paramdomMap_KeyOnList      = KLIB KHashMap_Init(kctx, 0);
	share->paramdomList       = new_(Array, 32, OnGlobalConstList);
	//
	share->constNull =  new_(Object, NULL, OnGlobalConstList);
	kObject_Set(NullObject, share->constNull, true);
	share->constTrue =   new_(Boolean, 1, OnGlobalConstList);
	share->constFalse =  new_(Boolean, 0, OnGlobalConstList);
	share->emptyString = new_(String, NULL, OnGlobalConstList);
	share->emptyArray =  new_(Array, 0, OnGlobalConstList);

	Kparam(kctx, KType_void, 0, NULL);  // PARAM_void
	Kparamdom(kctx, 0, NULL);        // PARAMDOM_void
	kparamtype_t p = {KType_Object};
	Kparamdom(kctx, 1, &p);          // PARAMDOM_DefaultGenericsParam  1
	FILEID_("(konoha.c)");
	PN_("konoha");                   // PN_konoha
	PN_("sugar");                    // PKG_sugar
	DefineDefaultKeywordSymbol(kctx);
	InitStructData(kctx);
}

static void KRuntime_Reftrace(KonohaContext *kctx, KonohaContextVar *ctx, KObjectVisitor *visitor)
{
	KRuntime *share = ctx->share;
	KRefTrace(share->GlobalConstList);
}

static void KRuntime_FreeClassTable(KonohaContext *kctx)
{
	KClassVar **cts = (KClassVar**)kctx->share->classTable.classItems;
	size_t i, size = kctx->share->classTable.bytesize/sizeof(KClassVar *);
	for(i = 0; i < size; i++) {
		if(cts[i]->fieldAllocSize > 0) {
			KFree(cts[i]->fieldItems, cts[i]->fieldAllocSize * sizeof(KClassField));
		}
		KFree(cts[i], sizeof(KClass));
	}
}

static void packageMap_Free(KonohaContext *kctx, void *p)
{
	KFree(p, sizeof(KPackage));
}

static void KRuntime_Free(KonohaContext *kctx, KonohaContextVar *ctx)
{
	KRuntimeVar *share = (KRuntimeVar *)ctx->share;
	KLIB KHashMap_Free(kctx, share->longClassNameMapNN, NULL);
	KLIB KHashMap_Free(kctx, share->fileIdMap_KeyOnList, NULL);
	KLIB KHashMap_Free(kctx, share->packageIdMap_KeyOnList, NULL);
	KLIB KHashMap_Free(kctx, share->packageMapNO, packageMap_Free);
	KLIB KHashMap_Free(kctx, share->symbolMap_KeyOnList, NULL);
	KLIB KHashMap_Free(kctx, share->paramMap_KeyOnList, NULL);
	KLIB KHashMap_Free(kctx, share->paramdomMap_KeyOnList, NULL);

	KRuntime_FreeClassTable(kctx);
	KLIB KArray_Free(kctx, &share->classTable);

	KFreeLock(share->classTableMutex);
	KFreeLock(share->filepackMutex);
	KFreeLock(share->symbolMutex);
	KFreeLock(share->paramMutex);
	KFree(share, sizeof(KRuntime));
}

