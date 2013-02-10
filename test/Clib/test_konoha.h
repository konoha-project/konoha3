#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>
#include "konoha3/platform.h"

#ifndef TEST_KONOHA_H_
#define TEST_KONOHA_H_

#ifdef __cplusplus
extern "C" {
#endif

extern void KonohaFactory_LoadPlatformModule(KonohaFactory *factory, const char *name, ModuleType option);
extern void KonohaFactory_SetDefaultFactory(KonohaFactory *factory, void (*SetPlatformApi)(KonohaFactory *), int argc, char **argv);
extern KonohaContext* KonohaFactory_CreateKonoha(KonohaFactory *factory);
extern int Konoha_Destroy(KonohaContext *kctx);

static KonohaContext *CreateContext()
{
	struct KonohaFactory factory = {};
	KonohaFactory_SetDefaultFactory(&factory, PosixFactory, 0, NULL);
	KonohaContext *kctx = KonohaFactory_CreateKonoha(&factory);
	return kctx;
}

static int DeleteContext(KonohaContext *kctx)
{
	return Konoha_Destroy(kctx);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* end of include guard */
