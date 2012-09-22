/****************************************************************************
 * KONOHA COPYRIGHT, LICENSE NOTICE, AND DISCRIMER
 *
 * Copyright (c)  2010-      Konoha Team konohaken@googlegroups.com
 * All rights reserved.
 *
 * You may choose one of the following two licenses when you use konoha.
 * See www.konohaware.org/license.html for further information.
 *
 * (1) GNU Lesser General Public License 3.0 (with KONOHA_UNDER_LGPL3)
 * (2) Konoha Software Foundation License 1.0
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

// **************************************************************************
// LIST OF CONTRIBUTERS
//  kimio - Kimio Kuramitsu, Yokohama National University, Japan
//  uh    - Yutaro Hiraoka, Yokohama National University, Japan
//  yoan  - Motoki Yoan, Yokohama National University, Japan
//  shinpei_nkt  - Shinpei Nakata, Yokohama National University, Japan (ntrace)
// **************************************************************************

#include <mysql.h>

#define MYSQL_USER_MAXLEN 16
#define MYSQL_PASS_MAXLEN 255
#define MYSQL_HOST_MAXLEN 255
#define MYSQL_DBNM_MAXLEN 64

/* ======================================================================== */

/*
static void knh_mysql_perror(KonohaContext *kctx, MYSQL *db, int r)
{
//	KNH_SYSLOG(kctx, LOG_WARNING, "MysqlError", "'%s'", mysql_error(db));
}
*/
/* ------------------------------------------------------------------------ */

//static kconn_t *MYSQL_qopen(KonohaContext *kctx, const char* url)
kconn_t *MYSQL_qopen(KonohaContext *kctx, const char* url)
{
	char *puser, user[MYSQL_USER_MAXLEN+1] = {0};
	char *ppass, pass[MYSQL_PASS_MAXLEN+1] = {0}; // temporary defined
	char *phost, host[MYSQL_HOST_MAXLEN+1] = {0};
	unsigned int port = 0;
	char *pdbnm, dbnm[MYSQL_DBNM_MAXLEN+1] = {0};

	url += 8; // skip: 'mysql://'
	const char *btstr = url;
	sscanf(btstr, "%16[^ :\r\n\t]:%255[^ @\r\n\t]@%255[^ :\r\n\t]:%5d/%64[^ \r\n\t]",
			(char*)&user, (char*)&pass, (char*)&host, &port, (char*)&dbnm); // consider to buffer over run

	puser = (user[0]) ? user : NULL;
	ppass = (pass[0]) ? pass : NULL;
	phost = (host[0]) ? host : NULL;
	pdbnm = (dbnm[0]) ? dbnm : NULL;

	MYSQL *db = mysql_init(NULL);
	OLDTRACE_SWITCH_TO_KTrace(_UserInputFault, LogText("@","mysql_init"));
	db = mysql_real_connect(db, phost, puser, ppass, pdbnm, port, NULL, 0);
	OLDTRACE_SWITCH_TO_KTrace(_UserInputFault, LogText("@","mysql_real_connect"),
			LogText("host", phost),
			LogText("user", user),
			LogText("passwd", ppass),
			LogText("dbname", pdbnm),
			LogUint("port", port),
			LogUint("errno", mysql_errno(db)),
			LogText("error", mysql_error(db)));
	//if (!mysql_real_connect(db, phost, puser, ppass, pdbnm, port, NULL, 0)) {
	//	knh_mysql_perror(kctx, db, 0);
	//	mysql_close(db);
	//	db = NULL;
	//}
	return (kconn_t*)db;
}
/* ------------------------------------------------------------------------ */

//static int MYSQL_qnext(KonohaContext *kctx, kqcur_t *qcur, kResultSet *rs)
int MYSQL_qnext(KonohaContext *kctx, kqcur_t *qcursor, kResultSet *rs)
{
	MYSQL_ROW row;
	if ((row = mysql_fetch_row((MYSQL_RES*)qcursor)) != NULL) {
		OLDTRACE_SWITCH_TO_KTrace(_UserInputFault, LogText("@","mysql_fetch_row"));
		int i;
		kint_t ival;
		kfloat_t fval;
		for (i = 0; i < rs->column_size; i++) {
			if (row[i] == NULL) {
				ResultSet_setNULL(kctx, rs, i);
				continue;
			}
			switch (rs->column[i].dbtype) {
			case MYSQL_TYPE_TINY:     case MYSQL_TYPE_SHORT:
			case MYSQL_TYPE_INT24:    case MYSQL_TYPE_LONG:
			case MYSQL_TYPE_LONGLONG: case MYSQL_TYPE_YEAR:
				knh_bytes_parseint(B(row[i]), &ival);
				ResultSet_setInt(kctx, rs, i, ival);
				break;
			case MYSQL_TYPE_FLOAT: case MYSQL_TYPE_DOUBLE:
				knh_bytes_parsefloat(B(row[i]), &fval);
				ResultSet_setFloat(kctx, rs, i, fval);
				break;
			case MYSQL_TYPE_NEWDECIMAL: case MYSQL_TYPE_STRING:
			case MYSQL_TYPE_VAR_STRING: case MYSQL_TYPE_TINY_BLOB:
			case MYSQL_TYPE_BLOB:       case MYSQL_TYPE_MEDIUM_BLOB:
			case MYSQL_TYPE_LONG_BLOB:  case MYSQL_TYPE_BIT:
			case MYSQL_TYPE_TIME:       case MYSQL_TYPE_DATE:
			case MYSQL_TYPE_DATETIME:   case MYSQL_TYPE_TIMESTAMP:
				ResultSet_setText(kctx, rs, i, B(row[i]));
				break;
			case MYSQL_TYPE_NULL:
			default:
				//KNH_SYSLOG(kctx, LOG_WARNING, "mysql", "mysql_qnext(dbtype)='unknown datatype [%d]'", DP(rs)->column[i].dbtype);
				ResultSet_setNULL(kctx, rs, i);
				break;
			}
		} /* for */
		return 1; /* CONTINUE */
	} else {
		OLDTRACE_SWITCH_TO_KTrace(_UserInputFault, LogText("@","mysql_fetch_row"));
	}
	return 0; /* NOMORE */
}
/* ------------------------------------------------------------------------ */

//static kqcur_t *MYSQL_query(KonohaContext *kctx, kconn_t *hdr, kbytes_t sql, kResultSet *rs)
kqcur_t *MYSQL_query(KonohaContext *kctx, void *hdr, const char* sql, struct _kResultSet *rs)
{
	MYSQL_RES *res = NULL;
	MYSQL *db = (MYSQL*)hdr;
	if (db == NULL) {
		/* return NULL */
	}
	else if (rs == NULL) {
		/* Connection.exec */
		int r = mysql_query(db, sql);
		if(r > 0) {
			OLDTRACE_SWITCH_TO_KTrace(_UserInputFault,
					LogText("@","mysql_query"),
					LogText("query", sql),
					LogUint("errno", mysql_errno(db)),
					LogText("error",mysql_error(db))
			);
		}
	}
	else {
		/* Connection.query */
		int r = mysql_query((MYSQL*)db, sql);
		OLDTRACE_SWITCH_TO_KTrace(_UserInputFault,
				LogText("@","mysql_query"),
				LogText("query", sql),
				LogUint("errno", mysql_errno(db)),
				LogText("error", mysql_error(db))
		);
		if (r == 0) { 
			res = mysql_store_result((MYSQL*)db);
			if (res == NULL) { // NULL RESULT
				if (mysql_errno(db) != 0) {
					mysql_free_result(res);
					OLDTRACE_SWITCH_TO_KTrace(_UserInputFault,
							LogText("@","mysql_sotre_result"),
							LogUint("errno", mysql_errno(db)),
							LogText("error", mysql_error(db))
					);
				} else {
					OLDTRACE_SWITCH_TO_KTrace(_UserInputFault,
							LogText("@","mysql_sotre_result"),
							LogUint("errno", mysql_errno(db)),
							LogText("error", mysql_error(db))
					);
				}
			}
			else {
				knh_ResultSet_initColumn(kctx, rs, (size_t)mysql_num_fields(res));
				OLDTRACE_SWITCH_TO_KTrace(_UserInputFault,
							LogText("@","mysql_sotre_result"),
							LogUint("errno", mysql_errno(db)),
							LogText("error", mysql_error(db))
					);
				int i = 0;
				MYSQL_FIELD *field = NULL;
				while((field = mysql_fetch_field(res))) {
					rs->column[i].dbtype = field->type;
					kString *s = KLIB new_kString(kctx, field->name, strlen(field->name), 0);
					//ResultSet_setName(kctx, rs, i, s);
					DBG_ASSERT(i < rs->column_size);
					KSETv(rs, rs->column[i].name, s);
					i++;
				}
			}
		}
	}
	return (kqcur_t *) res;
}
/* ------------------------------------------------------------------------ */

//static void MYSQL_qclose(KonohaContext *kctx, kconn_t *hdr)
void MYSQL_qclose(kconn_t *db)
{
	mysql_close((MYSQL*)db);
}

/* ------------------------------------------------------------------------ */

//static void MYSQL_qfree(kqcur_t *qcur)
void MYSQL_qfree(kqcur_t *qcur)
{
	if (qcur != NULL) {
		MYSQL_RES *res = (MYSQL_RES*)qcur;
		mysql_free_result(res);
	}
}

//const knh_QueryDSPI_t DB__mysql = {
//	K_DSPI_QUERY, "mysql",
//	MYSQL_qopen, MYSQL_query, MYSQL_qclose, MYSQL_qnext, MYSQL_qfree
//};
