/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/

#include <sqlite3.h>

#ifndef SQLITE3_DRIVER_H
#define SQLITE3_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------ */
static void knh_sqlite3_perror(KonohaContext* kctx, sqlite3 *db, int r)
{
	if(r == SQLITE_PERM || r == SQLITE_AUTH) {
		//msg = "Security";
	}
	//KNH_SYSLOG(ctx, LOG_WARNING, msg, "sqlite3_error='%s'", sqlite3_errmsg(db));
}

static DBHandler *SQLITE3_qopen(KonohaContext* kctx, const char *db, KTraceInfo *trace)
{
	sqlite3 *db_sqlite3 = NULL;
	db += 9;
	int r = sqlite3_open(db, &db_sqlite3);
	if(r != SQLITE_OK) {
		return NULL;
	}
	return (DBHandler *)db_sqlite3;
}

static int SQLITE3_qnext(KonohaContext* kctx, KCursor *qcur, kResultSet *rs, KTraceInfo *trace)
{
	sqlite3_stmt *stmt = (sqlite3_stmt *)qcur;
	int r = sqlite3_step(stmt);
	if(SQLITE_ROW == r) {
		size_t i;
		for(i = 0; i < rs->column_size; i++) {
			int type = sqlite3_column_type(stmt, i);
			switch(type) {
			case SQLITE_INTEGER:
				ResultSet_SetInt(kctx, rs, i, (kint_t)sqlite3_column_int64(stmt, i));
				break;
			case SQLITE_FLOAT:
				ResultSet_SetFloat(kctx, rs, i, (kfloat_t)sqlite3_column_double(stmt, i));
				break;
			case SQLITE_TEXT:
				ResultSet_SetText(kctx, rs, i, (char *)sqlite3_column_text(stmt,i), (size_t)sqlite3_column_bytes(stmt, i));
				break;
			case SQLITE_BLOB:
				//ResultSet_SetBlob(kctx, rs, i, (const char *)sqlite3_column_blob(stmt, i), sqlite3_column_bytes(stmt, i));
				break;
			case SQLITE_NULL:
			default:
				ResultSet_SetNull(kctx, rs, i);
			}
		}
		return 1;
	}
	else if(r != SQLITE_DONE) {
		//
	}
	return 0;  /* NOMORE */
}

static KCursor *SQLITE3_query(KonohaContext* kctx, DBHandler *db, const char *query, kResultSet *rs, KTraceInfo *trace)
{
	if(rs == NULL) {
		int r = sqlite3_exec((sqlite3*)db, query, NULL, NULL, NULL);
		if(r != SQLITE_OK) {
			knh_sqlite3_perror(kctx, (sqlite3*)db, r);
		}
		return NULL;
	}
	else {
		sqlite3_stmt *stmt = NULL;
		sqlite3_prepare((sqlite3*)db, query, strlen(query), &stmt, NULL);
		/*FIXME Error Check & Trace */
		size_t column_size = (size_t) sqlite3_column_count(stmt);
		DBG_P("column_size=%d", column_size);
		ResultSet_InitColumn(kctx, rs, column_size);
		if(column_size == 0) {
			sqlite3_exec((sqlite3 *)db, query, NULL, NULL, NULL);
		}else if(column_size > 0) {
			size_t i;
			for(i = 0; i < column_size; i++) {
				char *n = (char *)sqlite3_column_name(stmt, i);
				DBG_P("(%d) name = '%s'", i, n);
				if(n != NULL) {
					rs->column[i].dbtype = sqlite3_column_type(stmt, i);
					kString *s = KLIB new_kString(kctx, GcUnsafe, n, strlen(n), 0);
					KFieldSet(rs, rs->column[i].name, s);
				}
			}
		}
		return (KCursor *)stmt;
	}
}

static void SQLITE3_qclose(DBHandler *hdr)
{
	sqlite3_close((sqlite3 *)hdr);
}

static void SQLITE3_qfree(KCursor *qcur)
{
	sqlite3_stmt *stmt = (sqlite3_stmt *)qcur;
	sqlite3_finalize(stmt);
}

static QueryDriver SQLLite3Driver = {
	"sqlite3",
	SQLITE3_qopen,
	SQLITE3_query,
	SQLITE3_qclose,
	SQLITE3_qnext,
	SQLITE3_qfree
};


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* end of include guard */
