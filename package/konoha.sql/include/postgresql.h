/****************************************************************************
 * Copyright (c) 2012, the Konoha project authors. All rights reserved.
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

/* ************************************************************************ */

#include<libpq-fe.h>
#define _MSC_VER
#include"sql_common.h"
//#include<postgresql/libpq-fe.h>

/* ------------------------------------------------------------------------ */
/* [postgresql datatype]    */
/* reference from pg_type.h */

#define BOOLOID			16
#define BYTEAOID		17
#define CHAROID			18
#define NAMEOID			19
#define INT8OID			20
#define INT2OID			21
#define INT2VECTOROID	22
#define INT4OID			23
#define REGPROCOID		24
#define TEXTOID			25
#define OIDOID			26
#define TIDOID			27
#define XIDOID 			28
#define CIDOID 			29
#define OIDVECTOROID	30

#define  FLOAT4OID   700
#define  FLOAT8OID   701
#define  VARCHAROID   1043
#define  DATEOID   1082
#define  TIMESTAMPOID   1114

#define PQ_USER_MAXLEN 16
#define PQ_PASS_MAXLEN 255
#define PQ_HOST_MAXLEN 255
#define PQ_PORT_MAXLEN 8
#define PQ_DBNM_MAXLEN 64

/* ------------------------------------------------------------------------ */

//static void ksqlite3_perror(KonohaContext *kctx, sqlite3 *db, int r)
//{
//}

static void *POSTGRESQL_qopen(KonohaContext *kctx, const char* url)
{
	char *puser, user[PQ_USER_MAXLEN+1] = {0};
	char *ppass, pass[PQ_PASS_MAXLEN+1] = {0}; // temporary defined
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

	OLDTRACE_SWITCH_TO_KTrace(_UserInputFault, LogText("@","mysql_init"));
	conn = PQsetdbLogin(phost, port, NULL/* option */, NULL /* tty */, pdbnm, NULL /* user */, NULL /* passwd */);
	OLDTRACE_SWITCH_TO_KTrace(_UserInputFault, LogText("@","mysql_real_connect"),
			LogText("host", phost),
			LogText("user", user),
			LogText("passwd", ppass),
			LogText("dbname", pdbnm),
			LogUint("port", port),
			LogUint("errno", mysql_errno(db)),
			LogText("error", mysql_error(db)));
	if(PQstatus(conn) != CONNECTION_OK) {
		fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
		return NULL;
	}
	return (void *)conn;
}

static int POSTGRESQL_qnext(KonohaContext *kctx, kqcur_t *qcur, kResultSet *rs)
{
	PGresult* res = (PGresult *)qcur;
	size_t i, column_size = (size_t)PQnfields(res), row_size = PQntuples(res);
	if(rs->rowidx < row_size) {
		kint_t ival;
		kfloat_t fval;
		for (i = 0; i < column_size; i++) {
			Oid type = (Oid)rs->column[i].dbtype;
			switch (type) {
				case INT8OID:
				case INT2OID:
				case INT4OID:
					ival = parseInt((char *)PQgetvalue(res, 0, i), strlen((char *)PQgetvalue(res, 0, i)));
					_ResultSet_setInt(kctx, rs, i, ival);
					break;
				case FLOAT4OID:
				case FLOAT8OID:
					fval = parseFloat((char *)(PQgetvalue(res, 0, i)), strlen((char *)PQgetvalue(res, 0, i)));
					_ResultSet_setFloat(kctx, rs, i, fval);
					break;
				case TEXTOID:
				case VARCHAROID:
					_ResultSet_setText(kctx, rs, i, (char *)PQgetvalue(res, 0, i), (size_t)strlen((char *)PQgetvalue(res, 0, i)));
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
	} else {
		return 0; /* NOMORE */
	}
}

static kqcur_t *POSTGRESQL_query(KonohaContext *kctx, void *db, const char* sql, kResultSet *rs)
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
	else {
		res = PQexec((PGconn *)db, sql);
		if(PQresultStatus(res) != PGRES_COMMAND_OK) {
			// ktrace....
		}
		size_t column_size = (size_t)PQnfields(res);
		_ResultSet_initColumn(kctx, rs, column_size);
		if(column_size > 0) {
			size_t i;
			for(i = 0; i < rs->column_size; i++) {
				char *name = (char *)PQfname(res, i);
				if(name != NULL) {
					rs->column[i].dbtype = (int)PQftype(res, i);
					kString *s = KLIB new_kString(kctx, GcUnsafe, name, strlen(name), 0);
					DBG_ASSERT(i < rs->column_size);
					KFieldSet(rs, rs->column[i].name, s);
				}
			}
		}
		//PQclear(res);
		return (kqcur_t *)res;
	}
	return NULL;
}

static void POSTGRESQL_qclose(void *db)
{
	PQfinish(db);

}

static void POSTGRESQL_qfree(kqcur_t *qcur)
{
//	sqlite3_stmt *stmt = (sqlite3_stmt *)qcur;
//	sqlite3_finalize(stmt);
}

const QueryDriver PostgreSQLDriver = {
	K_DSPI_QUERY, "postgresql",
	POSTGRESQL_qopen, POSTGRESQL_query, POSTGRESQL_qclose, POSTGRESQL_qnext, POSTGRESQL_qfree
};
