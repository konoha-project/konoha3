#include <konoha3/konoha.h>
#include <konoha3/sugar.h>

#ifndef APACHE_GLUE_H
#define APACHE_GLUE_H
#define kmodapache        ((KModuleApache *)kctx->modshare[MOD_APACHE])
#define KClass_Request        kmodapache->cRequest
#define KClass_AprTable       kmodapache->cAprTable
#define KClass_AprTableEntry  kmodapache->cAprTableEntry
#define KClass_Apache         kmodapache->cApache

typedef struct kRequest {
	kObjectHeader h;
	request_rec *r;
} kRequest;

typedef struct kAprTable {
	kObjectHeader h;
	apr_table_t *tbl;
} kAprTable;

typedef struct kAprTableEntry {
	kObjectHeader h;
	apr_table_entry_t *entry;
} kAprTableEntry;

typedef struct kApache {
	kObjectHeader h;
} kApache;

typedef struct {
	KRuntimeModule h;
	KClass *cRequest;
	KClass *cAprTable;
	KClass *cAprTableEntry;
	KClass *cApache;
} KModuleApache;

typedef struct {
	KContextModule h;
	//kRequest *req;
} ctxapache_t;

#endif /* end of include guard */
