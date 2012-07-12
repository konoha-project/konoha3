#include "openssl_glue.h"
#include "openssl/md5.h"
#include "openssl/sha.h"

static void RawPtr_free(CTX, kObject *po)
{
	kRawPtr *o = (kRawPtr*)(po);
	if (o->rawptr) {
		free(o->rawptr);
	}
	o->rawptr = NULL;
}
static void RawPtr_init(CTX, kObject *po, void *conf)
{
	kRawPtr *o = (kRawPtr*)(po);
	o->rawptr = conf;
}

static KMETHOD kMD5_Init(CTX, ksfp_t *sfp _RIX)
{
	MD5state_st* c = malloc(sizeof(*c));
	int ret_ = MD5_Init(c);
	RawPtr_init(_ctx, sfp[0].o, c);
	RETURN_(sfp[0].o);
}
static KMETHOD kMD5_Update(CTX, ksfp_t *sfp _RIX)
{
	MD5state_st* c = RawPtr(sfp[0].o);
	unsigned char* data = (unsigned char *) S_text(sfp[1].s);
	unsigned long len = S_size(sfp[1].s);
	int ret_ = MD5_Update(c, data, len);
	RETURNi_(ret_);
}
static KMETHOD kMD5_Final(CTX, ksfp_t *sfp _RIX)
{
	unsigned char MD[MD5_DIGEST_LENGTH];
	MD5state_st* c = RawPtr(sfp[0].o);
	int ret_ = MD5_Final(MD, c);
	int i;
	char MD_S[MD5_DIGEST_LENGTH*2+1];
	for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
		snprintf(MD_S+i*2, MD5_DIGEST_LENGTH*2+1, "%02x", MD[i]);
	}
	RETURN_(new_kString(MD_S, MD5_DIGEST_LENGTH*2, SPOL_ASCII));
}
static KMETHOD kSHA1_Init(CTX, ksfp_t *sfp _RIX)
{
	SHAstate_st* c = malloc(sizeof(*c));
	int ret_ = SHA1_Init(c);
	RawPtr_init(_ctx, sfp[0].o, c);
	RETURN_(sfp[0].o);
}
static KMETHOD kSHA1_Update(CTX, ksfp_t *sfp _RIX)
{
	SHAstate_st* c = RawPtr(sfp[0].o);
	unsigned char* data = (unsigned char *) S_text(sfp[1].s);
	unsigned long len = S_size(sfp[1].s);
	int ret_ = SHA1_Update(c, data, len);
	RETURNi_(ret_);
}
static KMETHOD kSHA1_Final(CTX, ksfp_t *sfp _RIX)
{
	unsigned char SHA[SHA_DIGEST_LENGTH];
	SHAstate_st* c = RawPtr(sfp[0].o);
	int ret_ = SHA1_Final(SHA, c);
	int i;
	char SHA_S[SHA_DIGEST_LENGTH*2+1];
	for (i = 0; i < SHA_DIGEST_LENGTH; i++) {
		snprintf(SHA_S+i*2, SHA_DIGEST_LENGTH*2+1, "%02x", SHA[i]);
	}
	RETURN_(new_kString(SHA_S, SHA_DIGEST_LENGTH*2, SPOL_ASCII));
}
// --------------------------------------------------------------------------

#define _P kMethod_Public
#define _C kMethod_Const
#define _S kMethod_Static
#define _F(F)   (intptr_t)(F)
#define TY_openssl  (ct0->cid)
#define TY_Log      (ct1->cid)

static kbool_t openssl_initPackage(CTX, kNameSpace *ks, int argc, const char**args, kline_t pline)
{
	static const char *names[] = {
		"MD5",
		"SHA1",
	};
	kclass_t *tbls[2];
	static KDEFINE_CLASS Def = {
			.structname = "",
			.cid = CLASS_newid,
			.init = RawPtr_init,
			.free = RawPtr_free,
	};
#define TY_MD5  tbls[0]->cid
#define TY_SHA1 tbls[1]->cid
	int i;
	for (i = 0; i < 2; i++) {
		Def.structname = names[i];
		tbls[i] = Konoha_addClassDef(ks->packid, ks->packdom, NULL, &Def, pline);
	}

	int FN_x = FN_("x");
	KDEFINE_METHOD MethodData[] = {
		_P, _F(kMD5_Init),   TY_SHA1,   TY_MD5, MN_("new"), 0,
		_P, _F(kMD5_Update), TY_Int,   TY_MD5, MN_("update"), 1, TY_String, FN_x,
		_P, _F(kMD5_Final),  TY_String, TY_MD5, MN_("final"), 0,
		_P, _F(kSHA1_Init),   TY_SHA1,   TY_SHA1, MN_("new"), 0,
		_P, _F(kSHA1_Update), TY_Int,   TY_SHA1, MN_("update"), 1, TY_String, FN_x,
		_P, _F(kSHA1_Final),  TY_String, TY_SHA1, MN_("final"), 0,

		DEND,
	};
	kNameSpace_loadMethodData(ks, MethodData);
	return true;
}

static kbool_t openssl_setupPackage(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}

static kbool_t openssl_initNameSpace(CTX,  kNameSpace *ks, kline_t pline)
{
	return true;
}

static kbool_t openssl_setupNameSpace(CTX, kNameSpace *ks, kline_t pline)
{
	return true;
}
KDEFINE_PACKAGE* openssl_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("openssl", "1.0"),
		.initPackage = openssl_initPackage,
		.setupPackage = openssl_setupPackage,
		.initNameSpace = openssl_initNameSpace,
		.setupNameSpace = openssl_setupNameSpace,
	};
	return &d;
}

