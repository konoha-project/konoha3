#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>

#ifndef APACHE_GLUE_H
#define APACHE_GLUE_H
#define MOD_APACHE 23
#define kapacheshare ((kapacheshare_t*)kctx->modshare[MOD_APACHE])
#define CT_Request   kapacheshare->cRequest
#define CT_AprTable  kapacheshare->cAprTable
#define CT_AprTableEntry  kapacheshare->cAprTableEntry

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

typedef struct {
	kmodshare_t h;
	KonohaClass *cRequest;
	KonohaClass *cAprTable;
	KonohaClass *cAprTableEntry;
} kapacheshare_t;

typedef struct {
	kmodlocal_t h;
	//kRequest *req;
} ctxapache_t;

#endif /* end of include guard */
