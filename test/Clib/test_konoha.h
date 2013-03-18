#include <stdio.h>
//#include <syslog.h>
#include <stdlib.h>
#include "konoha3/platform.h"

#ifndef TEST_KONOHA_H_
#define TEST_KONOHA_H_

#ifdef __cplusplus
extern "C" {
#endif

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
