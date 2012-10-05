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

#ifdef _MSC_VER
#define typeof decltype
#endif

static kObject* DEFAULT_fnull(KonohaContext *kctx, KonohaClass *ct);

static void kObject_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kObjectVar *of = (kObjectVar*)o;
	of->fieldUnboxItems[0] = 0;
}

static void kObject_reftrace(KonohaContext *kctx, kObject *o)
{
	kObject *of = (kObject*)o;
	KonohaClass *ct = O_ct(of);
	BEGIN_REFTRACE(ct->fieldsize);
	size_t i;
	for(i = 0; i < ct->fieldsize; i++) {
		if(ct->fieldItems[i].isobj) {
			KREFTRACEv(of->fieldObjectItems[i]);
		}
	}
	END_REFTRACE();
}

static kObject *new_kObject(KonohaContext *kctx, KonohaClass *ct, uintptr_t conf)
{
	DBG_ASSERT(ct->cstruct_size > 0);
	kObjectVar *o = KLIB KallocObject(kctx->gcContext, ct);
	o->h.magicflag = ct->magicflag;
	o->h.ct = ct;
	o->h.kvproto = (KUtilsGrowingArray*) Kprotomap_new(0);
	ct->init(kctx, (kObject*)o, (void*)conf);
	return (kObject*)o;
}

static kObject *new_kObjectOnGCSTACK(KonohaContext *kctx, KonohaClass *ct, uintptr_t conf)
{
	kObject *o = new_kObject(kctx, ct, conf);
	PUSH_GCSTACK(o);  // GCSAFE
	return (kObject*)o;
}

static void KonohaClass_writeUnboxValueToBuffer(KonohaContext *kctx, KonohaClass *c, uintptr_t unboxValue, int isDelim, KUtilsWriteBuffer *wb)
{
	if(isDelim > 0) {
		KLIB Kwb_write(kctx, wb, ", ", 2);
	}
	KonohaValue v = {};
	v.unboxValue = unboxValue;
	c->p(kctx, &v, 0, wb);
}

static void kObject_writeToBuffer(KonohaContext *kctx, kObject *o, int isDelim, KUtilsWriteBuffer *wb, KonohaValue *sfp, int pos)
{
	if(isDelim > 0) {
		KLIB Kwb_write(kctx, wb, ", ", 2);
	}
	if(IS_NULL(o)) {
		KLIB Kwb_write(kctx, wb, TEXTSIZE("null"));
	}
	else {
		if(sfp == NULL) {
			sfp = (KonohaValue*)kctx->esp;
			pos = 0;
		}
		else {
			int i;
			for(i = 0; i < pos; i++) {
				if(sfp[i].asObject == o) break;
			}
		}
		if(TY_isUnbox(O_typeId(o))) {
			DBG_P("sfp[pos]=%d, unbox=%d", sfp[pos].unboxValue, O_unbox(o));
			sfp[pos].unboxValue = O_unbox(o);
		}
		else {
			KSETv_AND_WRITE_BARRIER(NULL, sfp[pos].asObject, o, GC_NO_WRITE_BARRIER);
		}
		O_ct(o)->p(kctx, sfp, pos, wb);
	}
}

static uintptr_t kNumber_unbox(KonohaContext *kctx, kObject *o)
{
	kNumber *n = (kNumber*)o;
	return (uintptr_t) n->unboxValue;
}

// Boolean
static void kNumber_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kIntVar *n = (kIntVar*)o;
	n->unboxValue = (uintptr_t)conf;
}

static void kBoolean_p(KonohaContext *kctx, KonohaValue *v, int pos, KUtilsWriteBuffer *wb)
{
	if(v[pos].boolValue) {
		KLIB Kwb_write(kctx, wb, TEXTSIZE("true"));
	}
	else {
		KLIB Kwb_write(kctx, wb, TEXTSIZE("false"));
	}
}

static kObject* kBoolean_fnull(KonohaContext *kctx, KonohaClass *ct)
{
	return (kObject*)K_FALSE;
}

static void kInt_p(KonohaContext *kctx, KonohaValue *v, int pos, KUtilsWriteBuffer *wb)
{
	KLIB Kwb_printf(kctx, wb, KINT_FMT, v[pos].intValue);
}

// String
static void kString_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kStringVar *s = (kStringVar*)o;
	s->text = "";
	s->bytesize = 0;
	S_setTextSgm(s, true);
}

static void kString_free(KonohaContext *kctx, kObject *o)
{
	kString *s = (kString*)o;
	if(S_isMallocText(s)) {
		KFREE(s->buf, S_size(s) + 1);
	}
}

static void kString_p(KonohaContext *kctx, KonohaValue *v, int pos, KUtilsWriteBuffer *wb)
{
	const char *t = S_text(v[pos].asString);
	size_t i, len = S_size(v[pos].asString);
	Kwb_write(kctx, wb, "\"", 1);
	for(i = 0; i < len; i++) {
		int ch = t[i];
		char buf[2] = {'\\', ch};
		if(ch == '\\' || ch == '"') {
			KLIB Kwb_write(kctx, wb, buf, 2);
		}
		else if(t[i] == '\n'){
			buf[1] = 'n';
			KLIB Kwb_write(kctx, wb, buf, 2);
		}
		else if(t[i] == '\t'){
			buf[1] = 't';
			KLIB Kwb_write(kctx, wb, buf, 2);
		}
		else {
			KLIB Kwb_write(kctx, wb, buf+1, 1);
		}
	}
	Kwb_write(kctx, wb, "\"", 1);
}

static uintptr_t kString_unbox(KonohaContext *kctx, kObject *o)
{
	kString *s = (kString*)o;
	DBG_ASSERT(IS_String(s));
	return (uintptr_t) s->text;
}

static void kString_checkASCII(KonohaContext *kctx, kString *s)
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
	S_setASCII((kStringVar*)s, (ch < 128));
}

static kString* new_kString(KonohaContext *kctx, const char *text, size_t len, int spol)
{
	KonohaClass *ct = CT_(TY_String);
	kStringVar *s = NULL; //knh_PtrMap_getS(kctx, ct->constPoolMapNULL, text, len);
	if(s != NULL) return s;
	if(TFLAG_is(int, spol, StringPolicy_TEXT)) {
		s = (kStringVar*)new_kObject(kctx, ct, 0);
		s->text = text;
		s->bytesize = len;
		S_setTextSgm(s, 1);
	}
	else if(len + 1 < sizeof(void*) * 2) {
		s = (kStringVar*)new_kObject(kctx, ct, 0);
		s->text = s->inline_text;
		s->bytesize = len;
		S_setTextSgm(s, 1);
		if(text != NULL) {
			DBG_ASSERT(!TFLAG_is(int, spol, StringPolicy_NOCOPY));
			memcpy(s->ubuf, text, len);
		}
		s->buf[len] = '\0';
	}
	else {
		s = (kStringVar*)new_kObject(kctx, ct, 0);
		s->bytesize = len;
		s->buf = (char*)KMALLOC(len+1);
		S_setTextSgm(s, 0);
		S_setMallocText(s, 1);
		if(text != NULL) {
			DBG_ASSERT(!TFLAG_is(int, spol, StringPolicy_NOCOPY));
			memcpy(s->ubuf, text, len);
		}
		s->buf[len] = '\0';
	}
	if(TFLAG_is(int, spol, StringPolicy_ASCII)) {
		S_setASCII(s, 1);
	}
	else if(TFLAG_is(int, spol, StringPolicy_UTF8)) {
		S_setASCII(s, 0);
	}
	else {
		kString_checkASCII(kctx, s);
	}
//	if(TFLAG_is(int, policy, StringPolicy_POOL)) {
//		kmapSN_add(kctx, ct->constPoolMapNO, s);
//		S_setPooled(s, 1);
//	}
	return s;
}

static kString* new_kStringf(KonohaContext *kctx, int spol, const char *fmt, ...)
{
	KUtilsWriteBuffer wb;
	Kwb_init(&(kctx->stack->cwb), &wb);
	va_list ap;
	va_start(ap, fmt);
	Kwb_vprintf(kctx, &wb, fmt, ap);
	va_end(ap);
	const char *text = Kwb_top(kctx, &wb, 1);
	kString *s = new_kString(kctx, text, Kwb_bytesize(&wb), spol);
	KLIB Kwb_free(&wb);
	return s;
}

// Array
struct _kAbstractArray {
	KonohaObjectHeader h;
	KUtilsGrowingArray a;
};

static void kArray_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct _kAbstractArray *a = (struct _kAbstractArray*)o;
	a->a.bytebuf     = NULL;
	a->a.bytesize    = 0;
	a->a.bytemax = ((size_t)conf * sizeof(void*));
	if(a->a.bytemax > 0) {
		KLIB Karray_init(kctx, &a->a, a->a.bytemax);
	}
	if(TY_isUnbox(O_p0(a))) {
		kArray_setUnboxData(a, 1);
	}
}

static void kArray_reftrace(KonohaContext *kctx, kObject *o)
{
	kArray *a = (kArray*)o;
	if(!kArray_isUnboxData(a)) {
		size_t i;
		BEGIN_REFTRACE(kArray_size(a));
		for(i = 0; i < kArray_size(a); i++) {
			KREFTRACEv(a->objectItems[i]);
		}
		END_REFTRACE();
	}
}

static void kArray_free(KonohaContext *kctx, kObject *o)
{
	struct _kAbstractArray *a = (struct _kAbstractArray*)o;
	KLIB Karray_free(kctx, &a->a);
}

static void kArray_p(KonohaContext *kctx, KonohaValue *values, int pos, KUtilsWriteBuffer *wb)
{
	size_t i;
	kArray *a = values[pos].asArray;
	KLIB Kwb_write(kctx, wb, "[", 1);
	if(kArray_isUnboxData(a)) {
		KonohaClass *c = CT_(O_p0(a));
		for(i = 0; i < kArray_size(a); i++) {
			KonohaClass_writeUnboxValueToBuffer(kctx, c, a->unboxItems[i], (i > 0)/*delim*/, wb);
		}
	}
	else {
		for(i = 0; i < kArray_size(a); i++) {
			kObject *o = a->objectItems[i];
			kObject_writeToBuffer(kctx, o, (i>0)/*delim*/, wb, values, pos+1);
		}
	}
	KLIB Kwb_write(kctx, wb, "]", 1);
}

static void kArray_ensureMinimumSize(KonohaContext *kctx, struct _kAbstractArray *a, size_t min)
{
	size_t minbyte = min * sizeof(void*);
	if(!(minbyte < a->a.bytemax)) {
		if(minbyte < sizeof(kObject)) minbyte = sizeof(kObject);
		KLIB Karray_expand(kctx, &a->a, minbyte);
	}
}

static void kArray_add(KonohaContext *kctx, kArray *o, kObject *value)
{
	size_t asize = kArray_size(o);
	struct _kAbstractArray *a = (struct _kAbstractArray*)o;
	kArray_ensureMinimumSize(kctx, a, asize+1);
	DBG_ASSERT(a->a.objectItems[asize] == NULL);
	KINITp(a, a->a.objectItems[asize], value);
	a->a.bytesize = (asize+1) * sizeof(void*);
}

static void kArray_insert(KonohaContext *kctx, kArray *o, size_t n, kObject *v)
{
	size_t asize = kArray_size(o);
	struct _kAbstractArray *a = (struct _kAbstractArray*)o;
	if(!(n < asize)) {
		kArray_add(kctx, o, v);
	}
	else {
		kArray_ensureMinimumSize(kctx, a, asize+1);
		memmove(a->a.objectItems+(n+1), a->a.objectItems+n, sizeof(kObject*) * (asize - n));
		KINITp(a, a->a.objectItems[n], v);
		a->a.bytesize = (asize+1) * sizeof(void*);
	}
}

static void kArray_clear(KonohaContext *kctx, kArray *o, size_t n)
{
	DBG_ASSERT(IS_Array(o));
	size_t asize = kArray_size(o);
	struct _kAbstractArray *a = (struct _kAbstractArray*)o;
	DBG_ASSERT(asize >= n);
	if(asize > n) {
		bzero(a->a.objectItems + n, sizeof(void*) * (asize - n));  // RCGC
		a->a.bytesize = (n) * sizeof(void*);
	}
}

// ---------------
// Param

static KonohaClass *KonohaClass_extendedBody(KonohaContext *kctx, KonohaClass *ct, size_t head, size_t body);

static void kParam_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kParamVar *pa = (kParamVar*)o;
	pa->psize = 0;
	pa->rtype = TY_void;
}

static kParam *new_Param(KonohaContext *kctx, ktype_t rtype, int psize, const kparamtype_t *p)
{
	KonohaClass *ct = CT_(TY_Param);
	ct = KonohaClass_extendedBody(kctx, ct, sizeof(void*), psize * sizeof(kparamtype_t));
	kParamVar *pa = (kParamVar*)new_kObject(kctx, ct, 0);
	pa->rtype = rtype;
	pa->psize = psize;
	if(psize > 0) {
		memcpy(pa->paramtypeItems, p, sizeof(kparamtype_t) * psize);
	}
	return pa;
}

static uintptr_t hashparamdom(int psize, const kparamtype_t *p)
{
	uintptr_t i, hcode = 0;
	for(i = 0; i < psize; i++) {
		hcode = p[i].ty + (31 * hcode);
	}
	return hcode;
}

static uintptr_t hashparam(ktype_t rtype, int psize, const kparamtype_t *p)
{
	uintptr_t i, hcode = rtype;
	for(i = 0; i < psize; i++) {
		hcode = (p[i].ty + p[i].fn) + (31 * hcode);
	}
	return hcode;
}

static kbool_t equalsParamDom(ktype_t rtype, int psize, const kparamtype_t *p, kParam *pa)
{
	if(psize == pa->psize) {
		int i;
		for(i = 0; i < psize; i++) {
			if(p[i].ty != pa->paramtypeItems[i].ty) return false;
		}
		return true;
	}
	return false;
}

static kbool_t equalsParam(ktype_t rtype, int psize, const kparamtype_t *p, kParam *pa)
{
	if(rtype == pa->rtype && psize == pa->psize) {
		int i;
		for(i = 0; i < psize; i++) {
			if(p[i].ty != pa->paramtypeItems[i].ty || p[i].fn != pa->paramtypeItems[i].fn) return false;
		}
		return true;
	}
	return false;
}

typedef kbool_t (*equalsP)(ktype_t rtype, int psize, const kparamtype_t *p, kParam *pa);

static kparamid_t Kmap_getparamid(KonohaContext *kctx, KUtilsHashMap *kmp, kArray *list, uintptr_t hcode, equalsP f, ktype_t rtype, int psize, const kparamtype_t *p)
{
	KUtilsHashMapEntry *e = KLIB Kmap_get(kctx, kmp, hcode);
	while(e != NULL) {
		if(e->hcode == hcode && f(rtype, psize, p, e->paramKey)) {
			return (kparamid_t)e->unboxValue;
		}
		e = e->next;
	}
	kParam *pa = new_Param(kctx, rtype, psize, p);
	uintptr_t paramid = kArray_size(list);
	KLIB kArray_add(kctx, list, pa);
	e = KLIB Kmap_newEntry(kctx, kmp, hcode);
	KINITv(e->paramKey, pa);
	e->unboxValue = paramid;
	return (kparamid_t)paramid;
}

static kparamid_t Kparam(KonohaContext *kctx, ktype_t rtype, int psize, const kparamtype_t *p)
{
	uintptr_t hcode = hashparam(rtype, psize, p);
	KLock(kctx->share->paramMutex);
	kparamid_t param = Kmap_getparamid(kctx, kctx->share->paramMapNN, kctx->share->paramList, hcode, equalsParam, rtype, psize, p);
	KUnlock(kctx->share->paramMutex);
	return param;
}

static kparamid_t Kparamdom(KonohaContext *kctx, int psize, const kparamtype_t *p)
{
	uintptr_t hcode = hashparamdom(psize, p);
	KLock(kctx->share->paramMutex);
	kparamid_t param = Kmap_getparamid(kctx, kctx->share->paramdomMapNN, kctx->share->paramdomList, hcode, equalsParamDom, TY_void, psize, p);
	KUnlock(kctx->share->paramMutex);
	return param;
}

/* --------------- */
/* Method */

static uintptr_t methodSerialNumber = 0;

static void kMethod_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kMethodVar *mtd = (kMethodVar*)o;
	bzero(&mtd->invokeMethodFunc, sizeof(kMethod) - sizeof(KonohaObjectHeader));
	KINITv(mtd->sourceCodeToken, (struct kToken*)K_NULL);
	KINITv(mtd->kcode, K_NULL);
	mtd->serialNumber = methodSerialNumber++;
}

static void kMethod_reftrace(KonohaContext *kctx, kObject *o)
{
	BEGIN_REFTRACE(3);
	kMethod *mtd = (kMethod*)o;
	KREFTRACEv(mtd->sourceCodeToken);
	KREFTRACEv(mtd->kcode);
	END_REFTRACE();
}

#define CT_MethodVar CT_Method
static kMethod* new_kMethod(KonohaContext *kctx, uintptr_t flag, ktype_t cid, kmethodn_t mn, MethodFunc func)
{
	kMethodVar* mtd = new_(MethodVar, NULL);
	mtd->flag       = flag;
	mtd->typeId     = cid;
	mtd->mn         = mn;
	KLIB kMethod_setFunc(kctx, mtd, func);
	return mtd;
}

static kParam* kMethod_setParam(KonohaContext *kctx, kMethod *mtd_, ktype_t rtype, int psize, const kparamtype_t *p)
{
	kparamid_t paramId = Kparam(kctx, rtype, psize, p);
	if(mtd_ != NULL) {
		kMethodVar* mtd = (kMethodVar*)mtd_;
		mtd->paramdom = Kparamdom(kctx, psize, p);
		mtd->paramid  = paramId;
	}
	return kctx->share->paramList->paramItems[paramId];
}

static intptr_t STUB_Method_indexOfField(kMethod *mtd)
{
	return -1;
}

// ---------------
// NameSpace

static void kNameSpace_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kNameSpaceVar *ns = (kNameSpaceVar*)o;
	bzero(&ns->parentNULL, sizeof(kNameSpace) - sizeof(KonohaObjectHeader));
	ns->syntaxOption = kNameSpace_DefaultSyntaxOption;
	if(conf != NULL) {
		KINITv(ns->parentNULL, (kNameSpace*)conf);
		ns->packageId     = ns->parentNULL->packageId;
		ns->packageDomain = ns->parentNULL->packageDomain;
		ns->syntaxOption  = ns->parentNULL->syntaxOption;
	}
	KINITv(ns->methodList, K_EMPTYARRAY);
}

static void kNameSpace_reftrace(KonohaContext *kctx, kObject *o)
{
	kNameSpace *ns = (kNameSpace*)o;
	KLIB kNameSpace_reftraceSugarExtension(kctx, ns);
	size_t i, size = kNameSpace_sizeConstTable(ns);
	BEGIN_REFTRACE(size + 4);
	for(i = 0; i < size; i++) {
		if(SYMKEY_isBOXED(ns->constTable.keyvalueItems[i].key)) {
			KREFTRACEv(ns->constTable.keyvalueItems[i].objectValue);
		}
	}
	KREFTRACEv(ns->methodList);
	KREFTRACEn(ns->parentNULL);
	KREFTRACEn(ns->globalObjectNULL);
	KREFTRACEn(ns->StmtPatternListNULL);
	END_REFTRACE();
}

static void kNameSpace_free(KonohaContext *kctx, kObject *o)
{
	kNameSpaceVar *ns = (kNameSpaceVar*)o;
	KLIB kNameSpace_freeSugarExtension(kctx, ns);
	KLIB Karray_free(kctx, &ns->constTable);
}

// ---------------
// System

static void Func_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kFuncVar *fo = (kFuncVar*)o;
	KINITv(fo->self, K_NULL);
	KINITv(fo->mtd, conf == NULL ? KNULL(Method) : (kMethod*)conf);
}

static void Func_reftrace(KonohaContext *kctx, kObject *o)
{
	BEGIN_REFTRACE(2);
	kFunc *fo = (kFunc*)o;
	KREFTRACEv(fo->self);
	KREFTRACEv(fo->mtd);
	END_REFTRACE();
}

// ---------------
// System


// ---------------

static KonohaClass *T_realtype(KonohaContext *kctx, KonohaClass *ct, KonohaClass *self)
{
	kParam *cparam = CT_cparam(self);
	DBG_P("ct=%s, self=%s", CT_t(ct), CT_t(self));
	DBG_ASSERT(ct->optvalue < cparam->psize);
	KonohaClass *pct = CT_(cparam->paramtypeItems[ct->optvalue].ty);
	return pct->realtype(kctx, pct, self);
}

// ---------------

static KonohaClass* Kclass(KonohaContext *kctx, ktype_t cid, kfileline_t pline)
{
	KonohaRuntime *share = kctx->share;
	if(!(cid < (share->classTable.bytesize/sizeof(KonohaClassVar*)))) {
		kreportf(ErrTag, pline, "invalid typeId=%d", (int)cid);
		KLIB KonohaRuntime_raise(kctx, EXPT_("InvalidParameter"), NULL, pline, NULL);
	}
	return share->classTable.classItems[cid];
}

static void DEFAULT_init(KonohaContext *kctx, kObject *o, void *conf)
{
	(void)kctx;(void)o;(void)conf;
}

static void DEFAULT_reftrace(KonohaContext *kctx, kObject *o)
{
	(void)kctx;(void)o;
}

static void DEFAULT_free(KonohaContext *kctx, kObject *o)
{
	(void)kctx;(void)o;
}

static void DEFAULT_p(KonohaContext *kctx, KonohaValue *v, int pos, KUtilsWriteBuffer *wb)
{
	KLIB Kwb_write(kctx, wb, TEXTSIZE("{\"class\": \""));
	KLIB Kwb_write(kctx, wb, TY_t(O_typeId(v[pos].asObject)), 0);
	KLIB Kwb_write(kctx, wb, TEXTSIZE("\"}"));
	//KLIB Kwb_printf(kctx, wb, "&%p(:%s)", sfp[pos].o, TY_t(O_typeId(sfp[pos].o)));
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
	DBG_ASSERT(ct->defaultValueAsNull != NULL);
	return ct->defaultValueAsNull;
}

static kObject* DEFAULT_fnullinit(KonohaContext *kctx, KonohaClass *ct)
{
	assert(ct->defaultValueAsNull == NULL);
	DBG_P("creating new nulval for %s", CT_t(ct));
	KINITv(((KonohaClassVar*)ct)->defaultValueAsNull, KLIB new_kObject(kctx, ct, 0));
	kObject_set(NullObject, ct->defaultValueAsNull, true);
	((KonohaClassVar*)ct)->fnull = DEFAULT_fnull;
	return ct->defaultValueAsNull;
}

static kObject *Knull(KonohaContext *kctx, KonohaClass *ct)
{
	return ct->fnull(kctx, ct);
}

static KonohaClassVar* new_KonohaClass(KonohaContext *kctx, KonohaClass *bct, KDEFINE_CLASS *s, kfileline_t pline)
{
	KonohaRuntimeVar *share = (KonohaRuntimeVar *)kctx->share;
	KonohaClassVar *ct;
	ktype_t newid;
	KLock(share->classTableMutex); {
		newid = share->classTable.bytesize / sizeof(KonohaClassVar*);
		if(share->classTable.bytesize == share->classTable.bytemax) {
			KLIB Karray_expand(kctx, &share->classTable, share->classTable.bytemax * 2);
		}
		share->classTable.bytesize += sizeof(KonohaClassVar*);
		ct = (KonohaClassVar*)KCALLOC(sizeof(KonohaClass), 1);
		share->classTable.classItems[newid] = (KonohaClass*)ct;
	}
	KUnlock(share->classTableMutex);
	if(bct != NULL) {
		DBG_ASSERT(s == NULL);
		memcpy(ct, bct, offsetof(KonohaClass, methodList));
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
		DBG_ASSERT(ct->cstruct_size <= 128);
		ct->DBG_NAME = (s->structname != NULL) ? s->structname : "N/A";
		if(s->cparamsize > 0 && s->cparamItems != NULL) {
			ct->p0 = s->cparamItems[0].ty;
			ct->cparamdom = Kparamdom(kctx, s->cparamsize, s->cparamItems);
		}
		// function
		ct->init = (s->init != NULL) ? s->init : DEFAULT_init;
		ct->reftrace = (s->reftrace != NULL) ? s->reftrace : DEFAULT_reftrace;
		ct->p     = (s->p != NULL) ? s->p : DEFAULT_p;
		ct->unbox = (s->unbox != NULL) ? s->unbox : DEFAULT_unbox;
		ct->free = (s->free != NULL) ? s->free : DEFAULT_free;
		ct->fnull = (s->fnull != NULL) ? s->fnull : DEFAULT_fnullinit;
		ct->realtype = (s->realtype != NULL) ? s->realtype : DEFAULT_realtype;
		ct->isSubType = (s->isSubType != NULL) ? s->isSubType : DEFAULT_isSubType;
		ct->initdef = s->initdef;
	}
	if(ct->initdef != NULL) {
		ct->initdef(kctx, ct, pline);
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
			KINITv(newct->methodList, ct->methodList);
			((KonohaClassVar*)ct)->searchSimilarClassNULL = (KonohaClass*)newct;
		}
		ct = ct->searchSimilarClassNULL;
	}
	return ct;
}

static KonohaClass *Generics_realtype(KonohaContext *kctx, KonohaClass *ct, KonohaClass *self)
{
	DBG_P("trying resolve generic type: %s %s", CT_t(ct), CT_t(self));
	kParam *param = CT_cparam(ct);
	int i;
	kparamtype_t *p = ALLOCA(kparamtype_t, param->psize);
	for(i = 0; i < param->psize; i++) {
		KonohaClass *cParam = CT_(param->paramtypeItems[i].ty);
		p[i].ty = cParam->realtype(kctx, cParam, self)->typeId;
	}
	return KLIB KonohaClass_Generics(kctx, ct, TY_void, param->psize, p);
}

static KonohaClass *Func_realtype(KonohaContext *kctx, KonohaClass *ct, KonohaClass *self)
{
	DBG_P("trying resolve generic type: %s %s", CT_t(ct), CT_t(self));
	KonohaClass *cReturn = CT_(ct->p0);
	ktype_t rtype = cReturn->realtype(kctx, cReturn, self)->typeId;
	kParam *param = CT_cparam(ct);
	int i;
	kparamtype_t *p = ALLOCA(kparamtype_t, param->psize);
	for(i = 0; i < param->psize; i++) {
		KonohaClass *cParam = CT_(param->paramtypeItems[i].ty);
		p[i].ty = cParam->realtype(kctx, cParam, self)->typeId;
	}
	return KLIB KonohaClass_Generics(kctx, CT_(ct->baseTypeId), rtype, param->psize, p);
}

#define TY_isTypeVar2(T)   (T != TY_void && TY_is(TypeVar, T))

static void checkTypeVar(KonohaContext *kctx, KonohaClassVar *newct, ktype_t rtype, int psize, kparamtype_t *p)
{
	int i, isTypeVar = TY_isTypeVar2(rtype);
	if(!isTypeVar) {
		for(i = 0; i < psize; i++) {
			if(TY_is(TypeVar, p[i].ty)) {
				isTypeVar = true;
			}
		}
	}
	if(isTypeVar) {
		DBG_P("Generics %s has TypeVar", CT_t(newct));
		newct->cflag |= kClass_TypeVar;
		newct->realtype = newct->baseTypeId == TY_Func ? Func_realtype : Generics_realtype;
	}
}

static KonohaClass *KonohaClass_Generics(KonohaContext *kctx, KonohaClass *ct, ktype_t rtype, int psize, kparamtype_t *p)
{
	kparamid_t paramdom = Kparamdom(kctx, psize, p);
	KonohaClass *ct0 = ct;
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
	newct->p0 = isNotFuncClass ? p[0].ty : rtype;
	KINITv(newct->methodList, K_EMPTYARRAY);
	if(newct->searchSuperMethodClassNULL == NULL) {
		newct->searchSuperMethodClassNULL = ct0;
	}
	checkTypeVar(kctx, newct, rtype, psize, p);
	((KonohaClassVar*)ct)->searchSimilarClassNULL = (KonohaClass*)newct;
	return ct->searchSimilarClassNULL;
}

static kString* KonohaClass_shortName(KonohaContext *kctx, KonohaClass *ct)
{
	if(ct->shortNameNULL == NULL) {
		if(ct->cparamdom == 0 && ct->baseTypeId != TY_Func) {
			KINITv(((KonohaClassVar*)ct)->shortNameNULL, SYM_s(ct->classNameSymbol));
		}
		else {
			size_t i, c = 0;
			kParam *cparam = CT_cparam(ct);
			KUtilsWriteBuffer wb;
			KonohaClass_shortName(kctx, CT_(ct->p0));
			for(i = 0; i < cparam->psize; i++) {
				KonohaClass_shortName(kctx, CT_(cparam->paramtypeItems[i].ty));
			}
			Kwb_init(&(kctx->stack->cwb), &wb);
			kString *s = SYM_s(ct->classNameSymbol);
			KLIB Kwb_write(kctx, &wb, S_text(s), S_size(s));
			KLIB Kwb_write(kctx, &wb, "[", 1);
			if(ct->baseTypeId == TY_Func) {
				s = KonohaClass_shortName(kctx, CT_(ct->p0));
				KLIB Kwb_write(kctx, &wb, S_text(s), S_size(s)); c++;
			}
			for(i = 0; i < cparam->psize; i++) {
				if(c > 0) KLIB Kwb_write(kctx, &wb, ",", 1);
				s = KonohaClass_shortName(kctx, CT_(cparam->paramtypeItems[i].ty));
				KLIB Kwb_write(kctx, &wb, S_text(s), S_size(s));
			}
			KLIB Kwb_write(kctx, &wb, "]", 1);
			const char *text = Kwb_top(kctx, &wb, 1);
			KINITv(((KonohaClassVar*)ct)->shortNameNULL, new_kString(kctx, text, Kwb_bytesize(&wb), StringPolicy_ASCII));
			KLIB Kwb_free(&wb);
		}
	}
	return ct->shortNameNULL;
}

static void KonohaClass_setName(KonohaContext *kctx, KonohaClassVar *ct, kfileline_t pline)
{
//	uintptr_t lname = longid(ct->packageDomain, ct->classNameSymbol);
//	KLock(kctx->share->classTableMutex); {
//		KonohaClass *ct2 = (KonohaClass*)map_getu(kctx, kctx->share->longClassNameMapNN, lname, (uintptr_t)NULL);
//		if(ct2 == NULL) {
//			map_addu(kctx, kctx->share->longClassNameMapNN, lname, (uintptr_t)ct);
//		}
//	}
//	KUnlock(kctx->share->classTableMutex);
	KLIB Kreportf(kctx, DebugTag, pline, "new class %s.%s", PackageId_t(ct->packageId), SYM_t(ct->classNameSymbol));
	if(ct->methodList == NULL) {
		KINITv(ct->methodList, K_EMPTYARRAY);
		if(ct->typeId > TY_Object) {
			ct->searchSuperMethodClassNULL = CT_(ct->superTypeId);
		}
	}
}

static KonohaClass *KonohaClass_define(KonohaContext *kctx, kpackage_t packageId, kString *name, KDEFINE_CLASS *cdef, kfileline_t pline)
{
	KonohaClassVar *ct = new_KonohaClass(kctx, NULL, cdef, pline);
	ct->packageId  = packageId;
	ct->packageDomain = packageId;
	if(name == NULL) {
		const char *n = cdef->structname;
		assert(n != NULL); // structname must be set;
		ct->classNameSymbol = ksymbolSPOL(n, strlen(n), StringPolicy_ASCII|StringPolicy_POOL|StringPolicy_TEXT, _NEWID);
	}
	else {
		ct->classNameSymbol = ksymbolA(S_text(name), S_size(name), _NEWID);
	}
	KonohaClass_setName(kctx, ct, pline);
	return (KonohaClass*)ct;
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
	defObject.init = kObject_init;
	defObject.reftrace = kObject_reftrace;
	
	KDEFINE_CLASS defBoolean = {0};
	SetUnboxTypeName(defBoolean, boolean);
	defBoolean.cstruct_size = sizeof(kBoolean);
	defBoolean.init  = kNumber_init;
	defBoolean.unbox = kNumber_unbox;
	defBoolean.p     = kBoolean_p;
	defBoolean.fnull = kBoolean_fnull;
	
	KDEFINE_CLASS defInt = {0};
	SetUnboxTypeName(defInt, int);
	defInt.cstruct_size = sizeof(kInt);
	defInt.init  = kNumber_init;
	defInt.unbox = kNumber_unbox;
	defInt.p     = kInt_p;
	
	KDEFINE_CLASS defString = {0};
	SETTYNAME(defString, String);
	defString.init = kString_init;
	defString.free = kString_free;
	defString.p    = kString_p;
	defString.unbox = kString_unbox;
	
	KDEFINE_CLASS defArray = {0};
	SETTYNAME(defArray, Array);
	defArray.init = kArray_init;
	defArray.reftrace = kArray_reftrace;
	defArray.free = kArray_free;
	defArray.p    = kArray_p;
	
	KDEFINE_CLASS defParam = {0};
	SETTYNAME(defParam, Param);
	defParam.init = kParam_init;
	
	KDEFINE_CLASS defMethod = {0};
	SETTYNAME(defMethod, Method);
	defMethod.init = kMethod_init;
	defMethod.reftrace = kMethod_reftrace;
	
	KDEFINE_CLASS defFunc = {0};
	SETTYNAME(defFunc, Func);
	defFunc.init = Func_init;
	defFunc.reftrace = Func_reftrace;
	
	KDEFINE_CLASS defNameSpace = {0};
	SETTYNAME(defNameSpace, NameSpace);
	defNameSpace.init = kNameSpace_init;
	defNameSpace.reftrace = kNameSpace_reftrace;
	defNameSpace.free = kNameSpace_free;
	
	KDEFINE_CLASS defSystem = {0};
	SETTYNAME(defSystem, System);
	defSystem.init = DEFAULT_init;
	
	KDEFINE_CLASS defT0 = {0};
	SetUnboxTypeName(defT0, 0);
	defT0.init = DEFAULT_init;
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
	int cid = 0;
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
		"()", "[]", "{}", "$Block", "$Param", "$TypeDecl", "$MethodDecl", "$TokenSequence",
		".", "/", "%", "*", "+", "-", "<", "<=", ">", ">=", "==", "!=",
		"&&", "||", "!", "=", ",", "$", ":", /*"@",*/
		"true", "false", "if", "else", "return", // syn
		"new",
	};
	for(i = 0; i < sizeof(keywords) / sizeof(const char*); i++) {
		ksymbolSPOL(keywords[i], strlen(keywords[i]), StringPolicy_TEXT|StringPolicy_ASCII, SYM_NEWID);
		//ksymbol_t sym = ksymbolSPOL(keywords[i], strlen(keywords[i]), StringPolicy_TEXT|StringPolicy_ASCII, SYM_NEWID);
		//fprintf(stdout, "#define KW_%s (((ksymbol_t)%d)|0) /*%s*/\n", SYM_t(sym), SYM_UNMASK(sym), keywords[i]);
	}
}

static void initStructData(KonohaContext *kctx)
{
	KonohaClass **ctt = (KonohaClass**)kctx->share->classTable.classItems;
	size_t i;//, size = kctx->share->classTable.bytesize/sizeof(KonohaClassVar*);
	for(i = 0; i <= TY_0; i++) {
		KonohaClassVar *ct = (KonohaClassVar *)ctt[i];
		const char *name = ct->DBG_NAME;
		ct->classNameSymbol = ksymbolSPOL(name, strlen(name), StringPolicy_ASCII|StringPolicy_POOL|StringPolicy_TEXT, _NEWID);
		KonohaClass_setName(kctx, ct, 0);
	}
	KLIB Knull(kctx, CT_NameSpace);
}

static void initKonohaLib(KonohaLibVar *l)
{
	l->Kclass                  = Kclass;
	l->new_kObject             = new_kObject;
	l->new_kObjectOnGCSTACK    = new_kObjectOnGCSTACK;
	l->new_kString             = new_kString;
	l->new_kStringf            = new_kStringf;
	//l->Kconv  = conv;
	l->kArray_add           = (typeof(l->kArray_add))kArray_add;
	l->kArray_insert        = (typeof(l->kArray_insert))kArray_insert;
	l->kArray_clear         = kArray_clear;
	l->new_kMethod          = new_kMethod;
	l->Kparamdom            = Kparamdom;
	l->kMethod_setParam     = kMethod_setParam;
	l->kMethod_indexOfField = STUB_Method_indexOfField;
	l->KonohaClass_define   = KonohaClass_define;
	l->Knull = Knull;
	l->KonohaClass_shortName = KonohaClass_shortName;
	l->KonohaClass_Generics = KonohaClass_Generics;
}

static void KonohaRuntime_init(KonohaContext *kctx, KonohaContextVar *ctx)
{
	KonohaRuntimeVar *share = (KonohaRuntimeVar*)KCALLOC(sizeof(KonohaRuntime), 1);
	ctx->share = share;
	KInitLock(share->classTableMutex);
	KInitLock(share->filepackMutex);
	KInitLock(share->symbolMutex);
	KInitLock(share->paramMutex);

	initKonohaLib((KonohaLibVar*)kctx->klib);
	KLIB Karray_init(kctx, &share->classTable, K_TYTABLE_INIT * sizeof(KonohaClass));
	loadInitStructData(kctx);
	share->longClassNameMapNN = KLIB Kmap_init(kctx, 0);
	KINITv(share->fileidList, new_(StringArray, 8));
	share->fileidMapNN = KLIB Kmap_init(kctx, 0);
	KINITv(share->packageIdList, new_(StringArray, 8));
	share->packageIdMapNN = KLIB Kmap_init(kctx, 0);
	share->packageMapNO = KLIB Kmap_init(kctx, 0);

	KINITv(share->symbolList, new_(StringArray, 32));
	share->symbolMapNN = KLIB Kmap_init(kctx, 0);
	share->paramMapNN = KLIB Kmap_init(kctx, 0);
	KINITv(share->paramList, new_(Array, 32));
	share->paramdomMapNN = KLIB Kmap_init(kctx, 0);
	KINITv(share->paramdomList, new_(Array, 32));
	//
	KINITv(share->constNull, new_(Object, NULL));
	kObject_set(NullObject, share->constNull, true);
	KINITv(share->constTrue,   new_(Boolean, 1));
	KINITv(share->constFalse,  new_(Boolean, 0));
	KINITv(share->emptyString, new_(String, NULL));
	KINITv(share->emptyArray,  new_(Array, 0));

	Kparam(kctx, TY_void, 0, NULL);  // PARAM_void
	Kparamdom(kctx, 0, NULL);        // PARAMDOM_void
	kparamtype_t p = {TY_Object};
	Kparamdom(kctx, 1, &p);          // PARAMDOM_DefaultGenericsParam  1
	FILEID_("(konoha.c)");
	PN_("konoha");    // PN_konoha
	PN_("sugar");     // PKG_sugar
	defineDefaultKeywordSymbol(kctx);
	initStructData(kctx);
}

static void constPoolMap_reftrace(KonohaContext *kctx, KUtilsHashMapEntry *p, void *thunk)
{
	BEGIN_REFTRACE(1);
	KREFTRACEv(p->objectValue);
	END_REFTRACE();
}

static void packageMap_reftrace(KonohaContext *kctx, KUtilsHashMapEntry *p, void *thunk)
{
	KonohaPackage *pack = (KonohaPackage*)p->unboxValue;
	BEGIN_REFTRACE(1);
	KREFTRACEn(pack->packageNameSpace);
	END_REFTRACE();
}

static void KonohaRuntime_reftrace(KonohaContext *kctx, KonohaContextVar *ctx)
{
	KonohaRuntime *share = ctx->share;
	KonohaClass **cts = (KonohaClass**)kctx->share->classTable.classItems;
	size_t i, size = kctx->share->classTable.bytesize/sizeof(KonohaClassVar*);
	for(i = 0; i < size; i++) {
		KonohaClass *ct = cts[i];
		{
			BEGIN_REFTRACE(3);
			KREFTRACEv(ct->methodList);
			KREFTRACEn(ct->shortNameNULL);
			KREFTRACEn(ct->defaultValueAsNull);
			END_REFTRACE();
		}
		if (ct->constPoolMapNO != NULL) {
			KLIB Kmap_each(kctx, ct->constPoolMapNO, NULL, constPoolMap_reftrace);
		}
	}
	KLIB Kmap_each(kctx, share->packageMapNO, NULL, packageMap_reftrace);
	BEGIN_REFTRACE(10);
	KREFTRACEv(share->constNull);
	KREFTRACEv(share->constTrue);
	KREFTRACEv(share->constFalse);
	KREFTRACEv(share->emptyString);
	KREFTRACEv(share->emptyArray);
	KREFTRACEv(share->fileidList);
	KREFTRACEv(share->packageIdList);
	KREFTRACEv(share->symbolList);
	KREFTRACEv(share->paramList);
	KREFTRACEv(share->paramdomList);
	END_REFTRACE();
}

static void KonohaRuntime_freeClassTable(KonohaContext *kctx)
{
	KonohaClassVar **cts = (KonohaClassVar**)kctx->share->classTable.classItems;
	size_t i, size = kctx->share->classTable.bytesize/sizeof(KonohaClassVar*);
	for(i = 0; i < size; i++) {
		if(cts[i]->fieldAllocSize > 0) {
			KFREE(cts[i]->fieldItems, cts[i]->fieldAllocSize * sizeof(KonohaClassField));
		}
		KFREE(cts[i], sizeof(KonohaClass));
	}
}

static void packageMap_free(KonohaContext *kctx, void *p)
{
	KFREE(p, sizeof(KonohaPackage));
}

static void KonohaRuntime_free(KonohaContext *kctx, KonohaContextVar *ctx)
{
	KonohaRuntimeVar *share = (KonohaRuntimeVar*)ctx->share;
	KLIB Kmap_free(kctx, share->longClassNameMapNN, NULL);
	KLIB Kmap_free(kctx, share->fileidMapNN, NULL);
	KLIB Kmap_free(kctx, share->packageIdMapNN, NULL);
	KLIB Kmap_free(kctx, share->packageMapNO, packageMap_free);
	KLIB Kmap_free(kctx, share->symbolMapNN, NULL);
	KLIB Kmap_free(kctx, share->paramMapNN, NULL);
	KLIB Kmap_free(kctx, share->paramdomMapNN, NULL);

	KonohaRuntime_freeClassTable(kctx);
	KLIB Karray_free(kctx, &share->classTable);

	KFreeLock(share->classTableMutex);
	KFreeLock(share->filepackMutex);
	KFreeLock(share->symbolMutex);
	KFreeLock(share->paramMutex);
	KFREE(share, sizeof(KonohaRuntime));
}

