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

#include <mysql.h>
#include <stdio.h> /* for sscanf */
#ifndef MYSQL_DRIVER_H
#define MYSQL_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#define MYSQL_USER_MAXLEN 16
#define MYSQL_PASS_MAXLEN 255
#define MYSQL_HOST_MAXLEN 255
#define MYSQL_DBNM_MAXLEN 64

/* ------------------------------------------------------------------------ */

static DBHandler *MYSQL_qopen(KonohaContext *kctx, const char *url, KTraceInfo *trace)
{
	char *puser, user[MYSQL_USER_MAXLEN+1] = {0};
	char *ppass, pass[MYSQL_PASS_MAXLEN+1] = {0}; // temporary defined
	char *phost, host[MYSQL_HOST_MAXLEN+1] = {0};
	unsigned int port = 0;
	char *pdbnm, dbnm[MYSQL_DBNM_MAXLEN+1] = {0};

	url += 8; // skip: 'mysql://'
	const char *btstr = url;
	// FIXME: consider to buffer over run
	sscanf(btstr, "%16[^ :\r\n\t]:%255[^ @\r\n\t]@%255[^ :\r\n\t]:%5d/%64[^ \r\n\t]",
			(char *)&user, (char *)&pass, (char *)&host, &port, (char *)&dbnm);

	puser = (user[0]) ? user : NULL;
	ppass = (pass[0]) ? pass : NULL;
	phost = (host[0]) ? host : NULL;
	pdbnm = (dbnm[0]) ? dbnm : NULL;

	MYSQL *db = mysql_init(NULL);
	OLDTRACE_SWITCH_TO_KTrace(_UserInputFault, LogText("@","mysql_Init"));
	db = mysql_real_connect(db, phost, puser, ppass, pdbnm, port, NULL, 0);
	OLDTRACE_SWITCH_TO_KTrace(_UserInputFault, LogText("@","mysql_real_connect"),
			LogText("host", phost),
			LogText("user", user),
			LogText("passwd", ppass),
			LogText("dbname", pdbnm),
			LogUint("port", port),
			LogUint("errno", mysql_errno(db)),
			LogText("error", mysql_error(db)));
	return (DBHandler *) db;
}
/* ------------------------------------------------------------------------ */

static int MYSQL_qnext(KonohaContext *kctx, KCursor *qcursor, kResultSet *rs, KTraceInfo *trace)
{
	//FIXME(ide) we do not use mysql_fetch_row() because row of a result set are stringified.
	MYSQL_ROW row;
	MYSQL_RES* res = (MYSQL_RES *)qcursor;
	int i;
	int num_fields = mysql_num_fields(res);
	if((row = mysql_fetch_row(res)) != NULL) {
		for (i = 0; i < num_fields; i++) {
			if(row[i] == NULL) {
				ResultSet_SetNull(kctx, rs, i);
				continue;
			}
			kint_t ival;
			kfloat_t fval;
			switch (rs->column[i].dbtype) {
			case MYSQL_TYPE_TINY:     case MYSQL_TYPE_SHORT:
			case MYSQL_TYPE_INT24:    case MYSQL_TYPE_LONG:
			case MYSQL_TYPE_LONGLONG: case MYSQL_TYPE_YEAR:
				ival = parseInt((char *)row[i], strlen((char *)row[i]));
				ResultSet_SetInt(kctx, rs, i, ival);
				break;
			case MYSQL_TYPE_FLOAT: case MYSQL_TYPE_DOUBLE:
				fval = parseFloat((char *)row[i], strlen((char *)row[i]));
				ResultSet_SetFloat(kctx, rs, i, fval);
				break;
			case MYSQL_TYPE_NEWDECIMAL: case MYSQL_TYPE_STRING:
			case MYSQL_TYPE_VAR_STRING: case MYSQL_TYPE_TINY_BLOB:
			case MYSQL_TYPE_BLOB:       case MYSQL_TYPE_MEDIUM_BLOB:
			case MYSQL_TYPE_LONG_BLOB:  case MYSQL_TYPE_BIT:
			case MYSQL_TYPE_TIME:       case MYSQL_TYPE_DATE:
			case MYSQL_TYPE_DATETIME:   case MYSQL_TYPE_TIMESTAMP:
				ResultSet_SetText(kctx, rs, i, (char *)row[i], strlen((char *)row[i]));
				break;
			case MYSQL_TYPE_NULL:
			default:
				ResultSet_SetNull(kctx, rs, i);
				break;
			}
		}
		return 1; /* CONTINUE */
	}
	return 0; /* NOMORE */
}
/* ------------------------------------------------------------------------ */

static KCursor *MYSQL_query(KonohaContext *kctx, void *hdr, const char *sql, kResultSet *rs, KTraceInfo *trace)
{
	MYSQL_RES *res = NULL;
	MYSQL *db = (MYSQL *)hdr;
	if(db == NULL) {
		return NULL;
	}
	else if(rs == NULL) {
		/* Connection.exec */
		int r = mysql_query(db, sql);
		if(r != 0) {
			/*FIXME Trace */
			//mysql_error(db)
		}
		return NULL;
	}
	/* Connection.query */
	int r = mysql_query((MYSQL *)db, sql);
	if(r != 0) { // error
		/*FIXME Trace */
		return NULL;
	}
	res = mysql_store_result((MYSQL *)db);
	if(res == NULL) { // NULL RESULT
		if(mysql_errno(db) != 0) {
			/*FIXME Trace */
			mysql_free_result(res);
		}
		return NULL;
	}
	else {
		size_t i = 0;
		MYSQL_FIELD *field = NULL;
		ResultSet_InitColumn(kctx, rs, (size_t)mysql_num_fields(res));
		while((field = mysql_fetch_field(res))) {
			kString *s = KLIB new_kString(kctx, GcUnsafe, field->name, strlen(field->name), 0);
			KFieldSet(rs, rs->column[i].name, s);
			rs->column[i].dbtype = field->type;
			i++;
		}
	}
	return (KCursor *) res;
}
/* ------------------------------------------------------------------------ */

static void MYSQL_qclose(void *db)
{
	mysql_close((MYSQL *)db);
}

/* ------------------------------------------------------------------------ */

static void MYSQL_qfree(KCursor *qcur)
{
	if(qcur != NULL) {
		MYSQL_RES *res = (MYSQL_RES *)qcur;
		mysql_free_result(res);
	}
}

/* ------------------------------------------------------------------------ */
/* [prototype function] */

static const QueryDriver MySQLDriver = {
	"mysql",
	MYSQL_qopen, MYSQL_query, MYSQL_qclose, MYSQL_qnext, MYSQL_qfree
};
/* ------------------------------------------------------------------------ */


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* end of include guard */
