#ifdef __cplusplus
extern "C" {
#endif

void FuelVM_KRuntime_raise(KonohaContext *kctx, int symbol, int fault, kString *optionalErrorInfo, KonohaStack *top)
{
	KLIB KRuntime_raise(kctx, symbol, fault, optionalErrorInfo, top);
}


void FuelVM_UpdateObjectField(KonohaContext *kctx, struct kObjectVar *parent, struct kObjectVar *oldPtr, struct kObjectVar *newVal)
{
	PLATAPI GCModule.UpdateObjectField(parent, oldPtr, newVal);
}

kObject *FuelVM_new_kObject(KonohaContext *kctx, kArray *gcstack, KClass *ct, uintptr_t conf)
{
	return KLIB new_kObject(kctx, gcstack, ct, conf);
}

kMethod *FuelVM_LookupMethod(KonohaContext *kctx, kObject *self, kMethod *mtd, kNameSpace *ns)
{
	KClass *ct = kObject_class(self);
	mtd =  KLIB kNameSpace_GetMethodBySignatureNULL(kctx, ns, ct, mtd->mn, mtd->paramdom, 0, NULL);
	return mtd;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
