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

#include <libpq-fe.h>
#include <stdio.h> /* for sscanf */

#ifndef POSTGRESQL_DRIVER_H
#define POSTGRESQL_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif
/* ------------------------------------------------------------------------ */
/* [postgresql datatype]    */
/* reference from pg_type.h */

#define BOOLOID       16
#define BYTEAOID      17
#define CHAROID       18
#define NAMEOID       19
#define INT8OID       20
#define INT2OID       21
#define INT2VECTOROID 22
#define INT4OID       23
#define REGPROCOID    24
#define TEXTOID       25
#define OIDOID        26
#define TIDOID        27
#define XIDOID        28
#define CIDOID        29
#define OIDVECTOROID  30

#define FLOAT4OID    700
#define FLOAT8OID    701
#define VARCHAROID   1043
#define DATEOID      1082
#define TIMESTAMPOID 1114

#define PQ_USER_MAXLEN 16
#define PQ_PASS_MAXLEN 255
#define PQ_HOST_MAXLEN 255
#define PQ_PORT_MAXLEN 8
#define PQ_DBNM_MAXLEN 64

/* ------------------------------------------------------------------------ */

static DBHandler *POSTGRESQL_qopen(KonohaContext *kctx, const char *url, KTraceInfo *trace)
{
	char *puser, user[PQ_USER_MAXLEN+1] = {0};
	char *ppass, pass[PQ_PASS_MAXLEN+1] = {0};
	char *phost, host[PQ_HOST_MAXLEN+1] = {0};
	char *pport, port[PQ_PORT_MAXLEN+1] = {0};
	char *pdbnm, dbnm[PQ_DBNM_MAXLEN+1] = {0};

	PGconn *conn;
	url += 13; // skip: 'postgresql://'
	const char *btstr = url;
	sscanf(btstr, "%16[^ :\r\n\t]:%255[^ @\r\n\t]@%255[^ :\r\n\t]:%8[^ /\r\n\t]/%64[^ \r\n\t]",
			(char *)&user, (char *)&pass, (char *)&host, (char *)&port, (char *)&dbnm); // consider to buffer over run

	puser = (user[0]) ? user : NULL;
	ppass = (pass[0]) ? pass : NULL;
	phost = (host[0]) ? host : NULL;
	pport = (port[0]) ? port : NULL;
	pdbnm = (dbnm[0]) ? dbnm : NULL;

	conn = PQsetdbLogin(phost, port, NULL/* option */, NULL /* tty */, pdbnm, NULL /* user */, NULL /* passwd */);
	if(PQstatus(conn) != CONNECTION_OK) {
		/*FIXME Trace */
		fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
		return NULL;
	}
	return (DBHandler *)conn;
}

static int POSTGRESQL_qnext(KonohaContext *kctx, KCursor *qcur, kResultSet *rs, KTraceInfo *trace)
{
	PGresult* res = (PGresult *)qcur;
	size_t i, column_size = (size_t)PQnfields(res), row_size = PQntuples(res);
	if(rs->rowidx < row_size) {
		kint_t ival;
		kfloat_t fval;
		for (i = 0; i < column_size; i++) {
			Oid type = (Oid)rs->column[i].dbtype;
			char *ptr;
			switch (type) {
				case INT8OID:
				case INT2OID:
				case INT4OID:
					ptr = PQgetvalue(res, 0, i);
					ival = parseInt(ptr, strlen(ptr));
					ResultSet_SetInt(kctx, rs, i, ival);
					break;
				case FLOAT4OID:
				case FLOAT8OID:
					ptr = PQgetvalue(res, 0, i);
					fval = parseFloat(ptr, strlen(ptr));
					ResultSet_SetFloat(kctx, rs, i, fval);
					break;
				case TEXTOID:
				case VARCHAROID:
					ptr = PQgetvalue(res, 0, i);
					ResultSet_SetText(kctx, rs, i, ptr, strlen(ptr));
					break;
				case DATEOID:
					break;
				case TIMESTAMPOID:
					break;
				default: 
					//fprintf(stderr, "ERROR: [qnext] (%u)\n", type);
					break;
			}
		}
		return 1;
	}
	return 0; /* NOMORE */
}

static KCursor *POSTGRESQL_query(KonohaContext *kctx, DBHandler *db, const char *sql, kResultSet *rs, KTraceInfo *trace)
{
	PGresult* res;
	if(rs == NULL) {
		res = PQexec((PGconn *)db, sql);
		if(PQresultStatus(res) != PGRES_COMMAND_OK) {
			// ktrace....
		}
		PQclear(res);
		return NULL;
	}
	res = PQexec((PGconn *)db, sql);
	if(PQresultStatus(res) != PGRES_COMMAND_OK) {
		// ktrace....
		return NULL;
	}
	size_t column_size = (size_t)PQnfields(res);
	ResultSet_InitColumn(kctx, rs, column_size);
	if(column_size > 0) {
		size_t i;
		for(i = 0; i < rs->column_size; i++) {
			char *name = (char *)PQfname(res, i);
			if(name != NULL) {
				rs->column[i].dbtype = (int)PQftype(res, i);
				kString *s = KLIB new_kString(kctx, OnField, name, strlen(name), 0);
				DBG_ASSERT(i < rs->column_size);
				KFieldSet(rs, rs->column[i].name, s);
			}
		}
	}
	return (KCursor *)res;
}

static void POSTGRESQL_qclose(DBHandler *db)
{
	PQfinish(db);
}

static void POSTGRESQL_qfree(KCursor *qcur)
{
	PGresult* res = (PGresult *) qcur;
	PQclear(res);
}

const QueryDriver PostgreSQLDriver = {
	"postgresql",
	POSTGRESQL_qopen,
	POSTGRESQL_query,
	POSTGRESQL_qclose,
	POSTGRESQL_qnext,
	POSTGRESQL_qfree
};
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* end of include guard */
