
extern "C" {

void FuelVM_KRuntime_raise(KonohaContext *kctx, int symbol, int fault, kString *optionalErrorInfo, KonohaStack *top)
{
	KLIB KRuntime_raise(kctx, symbol, fault, optionalErrorInfo, top);
}


void FuelVM_UpdateObjectField(KonohaContext *kctx, const struct kObjectVar *parent, const struct kObjectVar *oldPtr, const struct kObjectVar *newVal)
{
	PLATAPI UpdateObjectField(parent, oldPtr, newVal);
}

//kObject* (*new_kObject)(KonohaContext*, kArray *gcstack, KClass *, uintptr_t);
kObject *FuelVM_new_kObject(KonohaContext *kctx, uint64_t gcstack, void *ct, uint64_t conf)
{
	KClass *kclass = (KClass *) ct;
	return KLIB new_kObject(kctx, 0, kclass, (uintptr_t) conf);
}


kMethod *FuelVM_LookupMethod(KonohaContext *kctx, kObject *self, kMethod *mtd, kNameSpace *ns)
{
	KClass *ct = kObject_class(self);
	mtd =  KLIB kNameSpace_GetMethodBySignatureNULL(kctx, ns, ct, mtd->mn, mtd->paramdom, 0, NULL);
	return mtd;
}

} /* extern "C" */
