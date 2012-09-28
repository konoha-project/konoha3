#include "openssl_glue.h"
#include "openssl/md5.h"
#include "openssl/sha.h"

#ifdef __cplusplus
extern "C"{
#endif

static void RawPtr_free(KonohaContext *kctx, kObject *po)
{
	kRawPtr *o = (kRawPtr*)(po);
	if (o->rawptr) {
		free(o->rawptr);
	}
	o->rawptr = NULL;
}
static void RawPtr_init(KonohaContext *kctx, kObject *po, void *conf)
{
	kRawPtr *o = (kRawPtr*)(po);
	o->rawptr = conf;
}

static KMETHOD kMD5_Init(KonohaContext *kctx, KonohaStack *sfp)
{
	MD5state_st* c = malloc(sizeof(*c));
	int ret_ = MD5_Init(c);
	RawPtr_init(kctx, sfp[0].asObject, c);
	RETURN_(sfp[0].asObject);
}
static KMETHOD kMD5_Update(KonohaContext *kctx, KonohaStack *sfp)
{
	MD5state_st* c = RawPtr(sfp[0].asObject);
	unsigned char* data = (unsigned char *) S_text(sfp[1].asString);
	unsigned long len = S_size(sfp[1].asString);
	int ret_ = MD5_Update(c, data, len);
	RETURNi_(ret_);
}
static KMETHOD kMD5_Final(KonohaContext *kctx, KonohaStack *sfp)
{
	unsigned char MD[MD5_DIGEST_LENGTH];
	MD5state_st* c = RawPtr(sfp[0].asObject);
	int ret_ = MD5_Final(MD, c);
	int i;
	char MD_S[MD5_DIGEST_LENGTH*2+1];
	for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
		snprintf(MD_S+i*2, MD5_DIGEST_LENGTH*2+1, "%02x", MD[i]);
	}
	RETURN_(KLIB new_kString(kctx, MD_S, MD5_DIGEST_LENGTH*2, SPOL_ASCII));
}
static KMETHOD kSHA1_Init(KonohaContext *kctx, KonohaStack *sfp)
{
	SHAstate_st* c = malloc(sizeof(*c));
	int ret_ = SHA1_Init(c);
	RawPtr_init(kctx, sfp[0].asObject, c);
	RETURN_(sfp[0].asObject);
}
static KMETHOD kSHA1_Update(KonohaContext *kctx, KonohaStack *sfp)
{
	SHAstate_st* c = RawPtr(sfp[0].asObject);
	unsigned char* data = (unsigned char *) S_text(sfp[1].asString);
	unsigned long len = S_size(sfp[1].asString);
	int ret_ = SHA1_Update(c, data, len);
	RETURNi_(ret_);
}
static KMETHOD kSHA1_Final(KonohaContext *kctx, KonohaStack *sfp)
{
	unsigned char SHA[SHA_DIGEST_LENGTH];
	SHAstate_st* c = RawPtr(sfp[0].asObject);
	int ret_ = SHA1_Final(SHA, c);
	int i;
	char SHA_S[SHA_DIGEST_LENGTH*2+1];
	for (i = 0; i < SHA_DIGEST_LENGTH; i++) {
		snprintf(SHA_S+i*2, SHA_DIGEST_LENGTH*2+1, "%02x", SHA[i]);
	}
	RETURN_(KLIB new_kString(kctx, SHA_S, SHA_DIGEST_LENGTH*2, SPOL_ASCII));
}
// --------------------------------------------------------------------------

#define _P kMethod_Public
#define _C kMethod_Const
#define _S kMethod_Static
#define _F(F)   (intptr_t)(F)
#define TY_openssl  (ct0->typeId)
#define TY_Log      (ct1->typeId)

static kbool_t openssl_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	static const char *names[] = {
		"MD5",
		"SHA1",
	};
	KonohaClass *tbls[2];
	static KDEFINE_CLASS Def = {
			.structname = "",
			.typeId = TY_newid,
			.init = RawPtr_init,
			.free = RawPtr_free,
	};
#define TY_MD5  tbls[0]->typeId
#define TY_SHA1 tbls[1]->typeId
	int i;
	for (i = 0; i < 2; i++) {
		Def.structname = names[i];
		tbls[i] = KLIB kNameSpace_defineClass(kctx, ns, NULL, &Def, pline);
	}

	int FN_x = FN_("x");
	KDEFINE_METHOD MethodData[] = {
		_P, _F(kMD5_Init),   TY_SHA1,   TY_MD5, MN_("new"), 0,
		_P, _F(kMD5_Update), TY_int,   TY_MD5, MN_("update"), 1, TY_String, FN_x,
		_P, _F(kMD5_Final),  TY_String, TY_MD5, MN_("final"), 0,
		_P, _F(kSHA1_Init),   TY_SHA1,   TY_SHA1, MN_("new"), 0,
		_P, _F(kSHA1_Update), TY_int,   TY_SHA1, MN_("update"), 1, TY_String, FN_x,
		_P, _F(kSHA1_Final),  TY_String, TY_SHA1, MN_("final"), 0,

		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t openssl_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t openssl_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t openssl_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}
KDEFINE_PACKAGE* openssl_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("openssl", "1.0"),
		.initPackage    = openssl_initPackage,
		.setupPackage   = openssl_setupPackage,
		.initNameSpace  = openssl_initNameSpace,
		.setupNameSpace = openssl_setupNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
