#include <konoha2/konoha2.h>
#include <konoha2/sugar.h>

#ifndef APACHE_GLUE_H
#define APACHE_GLUE_H
#define MOD_APACHE 23
#define kapacheshare ((kapacheshare_t*)_ctx->modshare[MOD_APACHE])
#define CT_Request   kapacheshare->cRequest
#define CT_AprTable  kapacheshare->cAprTable
#define CT_AprTableEntry  kapacheshare->cAprTableEntry

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

typedef struct {
	kmodshare_t h;
	kclass_t *cRequest;
	kclass_t *cAprTable;
	kclass_t *cAprTableEntry;
} kapacheshare_t;

typedef struct {
	kmodlocal_t h;
	//kRequest *req;
} ctxapache_t;

#endif /* end of include guard */
