#include "openssl_glue.h"
#include "openssl/md5.h"
#include "openssl/sha.h"

#ifdef __cplusplus
extern "C"{
#endif

static void RawPtr_Free(KonohaContext *kctx, kObject *po)
{
	kRawPtr *o = (kRawPtr *)(po);
	if(o->rawptr) {
		free(o->rawptr);
	}
	o->rawptr = NULL;
}
static void RawPtr_Init(KonohaContext *kctx, kObject *po, void *conf)
{
	kRawPtr *o = (kRawPtr *)(po);
	o->rawptr = conf;
}

static KMETHOD kMD5_Init(KonohaContext *kctx, KonohaStack *sfp)
{
	MD5state_st *c = malloc(sizeof(*c));
	int ret_ = MD5_Init(c); (void)ret_;
	RawPtr_Init(kctx, sfp[0].asObject, c);
	KReturn(sfp[0].asObject);
}
static KMETHOD kMD5_Update(KonohaContext *kctx, KonohaStack *sfp)
{
	MD5state_st *c = RawPtr(sfp[0].asObject);
	unsigned char *data = (unsigned char *) kString_text(sfp[1].asString);
	unsigned long len = kString_size(sfp[1].asString);
	int ret_ = MD5_Update(c, data, len);
	KReturnUnboxValue(ret_);
}
static KMETHOD kMD5_Final(KonohaContext *kctx, KonohaStack *sfp)
{
	unsigned char MD[MD5_DIGEST_LENGTH];
	MD5state_st *c = RawPtr(sfp[0].asObject);
	int ret_ = MD5_Final(MD, c); (void)ret_;
	int i;
	char MD_S[MD5_DIGEST_LENGTH*2+1];
	for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
		snprintf(MD_S+i*2, MD5_DIGEST_LENGTH*2+1, "%02x", MD[i]);
	}
	KReturn(KLIB new_kString(kctx, OnStack, MD_S, MD5_DIGEST_LENGTH*2, StringPolicy_ASCII));
}
static KMETHOD kSHA1_Init(KonohaContext *kctx, KonohaStack *sfp)
{
	SHAstate_st *c = malloc(sizeof(*c));
	int ret_ = SHA1_Init(c); (void)ret_;
	RawPtr_Init(kctx, sfp[0].asObject, c);
	KReturn(sfp[0].asObject);
}
static KMETHOD kSHA1_Update(KonohaContext *kctx, KonohaStack *sfp)
{
	SHAstate_st *c = RawPtr(sfp[0].asObject);
	unsigned char *data = (unsigned char *) kString_text(sfp[1].asString);
	unsigned long len = kString_size(sfp[1].asString);
	int ret_ = SHA1_Update(c, data, len);
	KReturnUnboxValue(ret_);
}
static KMETHOD kSHA1_Final(KonohaContext *kctx, KonohaStack *sfp)
{
	unsigned char SHA[SHA_DIGEST_LENGTH];
	SHAstate_st *c = RawPtr(sfp[0].asObject);
	int ret_ = SHA1_Final(SHA, c); (void)ret_;
	int i;
	char SHA_S[SHA_DIGEST_LENGTH*2+1];
	for (i = 0; i < SHA_DIGEST_LENGTH; i++) {
		snprintf(SHA_S+i*2, SHA_DIGEST_LENGTH*2+1, "%02x", SHA[i]);
	}
	KReturn(KLIB new_kString(kctx, OnStack, SHA_S, SHA_DIGEST_LENGTH*2, StringPolicy_ASCII));
}
// --------------------------------------------------------------------------

//#define KType_openssl  (ct0->typeId)
//#define KType_Log      (ct1->typeId)

static kbool_t openssl_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	static const char *names[] = {
		"MD5",
		"SHA1",
	};
	KClass *tbls[2];
	static KDEFINE_CLASS Def = {
			.structname = "",
			.typeId = KTypeAttr_NewId,
			.init = RawPtr_Init,
			.free = RawPtr_Free,
	};
#define KType_MD5  tbls[0]->typeId
#define KType_SHA1 tbls[1]->typeId
	int i;
	for (i = 0; i < 2; i++) {
		Def.structname = names[i];
		tbls[i] = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &Def, trace);
	}

	int FN_x = KFieldName_("x");
	KDEFINE_METHOD MethodData[] = {
		_Public, _F(kMD5_Init),   KType_SHA1,   KType_MD5, KMethodName_("new"), 0,
		_Public, _F(kMD5_Update), KType_Int,   KType_MD5, KMethodName_("update"), 1, KType_String, FN_x,
		_Public, _F(kMD5_Final),  KType_String, KType_MD5, KMethodName_("final"), 0,
		_Public, _F(kSHA1_Init),   KType_SHA1,   KType_SHA1, KMethodName_("new"), 0,
		_Public, _F(kSHA1_Update), KType_Int,   KType_SHA1, KMethodName_("update"), 1, KType_String, FN_x,
		_Public, _F(kSHA1_Final),  KType_String, KType_SHA1, KMethodName_("final"), 0,

		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

static kbool_t openssl_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *Openssl_Init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("openssl", "1.0"),
		.PackupNameSpace    = openssl_PackupNameSpace,
		.ExportNameSpace   = openssl_ExportNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
