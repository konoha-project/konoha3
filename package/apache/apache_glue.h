#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>

#ifndef APACHE_GLUE_H
#define APACHE_GLUE_H
#define kapacheshare      ((kapacheshare_t*)kctx->modshare[MOD_APACHE])
#define CT_Request        kapacheshare->cRequest
#define CT_AprTable       kapacheshare->cAprTable
#define CT_AprTableEntry  kapacheshare->cAprTableEntry
#define CT_Apache         kapacheshare->cApache

typedef struct kRequest {
	KonohaObjectHeader h;
	request_rec *r;
} kRequest;

typedef struct kAprTable {
	KonohaObjectHeader h;
	apr_table_t *tbl;
} kAprTable;

typedef struct kAprTableEntry {
	KonohaObjectHeader h;
	apr_table_entry_t *entry;
} kAprTableEntry;

typedef struct kApache {
	KonohaObjectHeader h;
} kApache;

typedef struct {
	KonohaModule h;
	KonohaClass *cRequest;
	KonohaClass *cAprTable;
	KonohaClass *cAprTableEntry;
	KonohaClass *cApache;
} kapacheshare_t;

typedef struct {
	KonohaModuleContext h;
	//kRequest *req;
} ctxapache_t;

#endif /* end of include guard */
