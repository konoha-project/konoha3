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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/bytes.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ======================================================================== */
/* [struct] */
/* ------------------------------------------------------------------------ */
/* [Channel] */

//typedef struct knh_Channel_t {
//	kObjectHeader h;
//	kintptr_t sd;
//	struct kString *urn;
//	struct kInputStream  *in;
//	struct kOutputStream *out;
//} knh_Channel_t;

/* ------------------------------------------------------------------------ */
/* [Connection] */

typedef void kconn_t;
typedef const struct _kQueryDPI_t kQueryDSPI_t;
typedef const struct _kConnection kConnection;

struct _kConnection {
	KonohaObjectHeader  h;
	void                *db;
	const struct _kQueryDPI_t *dspi;
	kString             *urn;
};

#define CT_Connection     cConnection
#define TY_Connection     cConnection->typeId
#define IS_Connection(O)  ((O)->h.ct == CT_Connection)

/* ------------------------------------------------------------------------ */
/* [ResultSet] */

typedef const struct _kDBschema kDBschema;

struct _kDBschema {
	ktype_t   type;
	kushort_t ctype;
	kString   *name;
	size_t    start;
	size_t    len;
	int       dbtype;
};

typedef void kqcur_t;

typedef const struct _kResultSet kResultSet;
struct _kResultSet{
	KonohaObjectHeader  h;
	kConnection         *connection;
	kqcur_t             *qcur;
	void                (*qcurfree)(kqcur_t *); /* necessary if conn is closed before */
	kString             *tableName;
	//kclass_t            tcid;
	kushort_t           column_size;
	struct _kDBschema   *column;
	kBytes              *databuf;
	size_t              count;
};

#define CT_ResultSet     cResultSet
#define TY_ResultSet     cResultSet->typeId
#define IS_ResultSet(O)  ((O)->h.ct == CT_ResultSet)

/* ------------------------------------------------------------------------ */
/* [bytes struct] */

typedef struct {
	size_t       len;
	union {
		const char *text;
		const unsigned char *utext;
		char *buf;
		//kchar_t *ubuf;
	};
} kbytes_t;

/* ------------------------------------------------------------------------ */
/* [prototype define] */

//void knh_Connection_open(KonohaContext *kctx, kConnection *c, kString *urn);
//void knh_Connection_close(KonohaContext *kctx, kConnection *c);
//void knh_ResultSet_initData(KonohaContext *kctx, kResultSet *rs);
//kbool_t knh_ResultSet_next(KonohaContext *kctx, kResultSet *o);
//kString *knh_ResultSet_getName(KonohaContext *kctx, kResultSet *o, size_t n);
//int knh_ResultSet_findColumn(KonohaContext *kctx, kResultSet *o, kbytes_t name);
//kString* knh_ResultSet_getString(KonohaContext *kctx, kResultSet *o, size_t n);
//void knh_ResultSet_close(KonohaContext *kctx, kResultSet *o);
//
//KMETHOD knh_ResultSet_initColumn(KonohaContext *kctx, kResultSet *o, size_t column_size);
//KMETHOD ResultSet_setBlob(KonohaContext *kctx, kResultSet *o, size_t n, kbytes_t t);
//KMETHOD ResultSet_setFloat(KonohaContext *kctx, kResultSet *rs, size_t n, kfloat_t value);
//KMETHOD ResultSet_setInt(KonohaContext *kctx, kResultSet *rs, size_t n, kint_t value);
//KMETHOD ResultSet_setNULL(KonohaContext *kctx, kResultSet *o, size_t n);
//KMETHOD ResultSet_setName(KonohaContext *kctx, kResultSet *o, size_t n, kString *name);
//KMETHOD ResultSet_setText(KonohaContext *kctx, kResultSet *o, size_t n, kbytes_t t);

/* ------------------------------------------------------------------------ */
/* [driver] */
/* [TODO] Now, ResultSet driver is global value */
/* but it should be shared using anything without global value */

typedef struct knh_ResultSetDef_t {
	//KMETHOD (*initColumn)(KonohaContext *kctx, kResultSet *o, size_t column_size);
	//KMETHOD (*setBlob)(KonohaContext *kctx, kResultSet *o, size_t n, kbytes_t t);
	//KMETHOD (*setFloat)(KonohaContext *kctx, kResultSet *rs, size_t n, kfloat_t value);
	//KMETHOD (*setInt)(KonohaContext *kctx, kResultSet *rs, size_t n, kint_t value);
	//KMETHOD (*setNULL)(KonohaContext *kctx, kResultSet *o, size_t n);
	//KMETHOD (*setName)(KonohaContext *kctx, kResultSet *o, size_t n, kString *name);
	//KMETHOD (*setText)(KonohaContext *kctx, kResultSet *o, size_t n, kbytes_t t);
} knh_ResultSetDef_t;

static const knh_ResultSetDef_t ResultSetDef = {
	//knh_ResultSet_initColumn,
	//ResultSet_setBlob,
	//ResultSet_setFloat,
	//ResultSet_setInt,
	//ResultSet_setNULL,
	//ResultSet_setName,
	//ResultSet_setText,
};
/* ------------------------------------------------------------------------ */
/* K_DSPI_QUERY */

struct _kQueryDPI_t {
	int   type;
	const char *name;
	void* (*qopen)(KonohaContext* kctx, const char* db);
	kqcur_t* (*qexec)(KonohaContext* kctx, void* db, const char* query, struct _kResultSet* rs);
	void   (*qclose)(void* hdr);
	//kconn_t* (*qopen)(KonohaContext *kctx, kbytes_t);
	//kqcur_t* (*qexec)(KonohaContext *kctx, kconn_t *, kbytes_t, kResultSet*);
	//void   (*qclose)(KonohaContext *kctx, kconn_t *);
	int    (*qcurnext)(KonohaContext* kctx, kqcur_t * qcur, kResultSet* rs);
	void   (*qcurfree)(kqcur_t *);
};

/* ------------------------------------------------------------------------ */
/* [Macros] */
#define knh_ResultSet_CTYPE__null    0
#define knh_ResultSet_CTYPE__integer 1
#define knh_ResultSet_CTYPE__float   2
#define knh_ResultSet_CTYPE__text    3  /* UTF-8*/
#define knh_ResultSet_CTYPE__bytes   4
#define knh_ResultSet_CTYPE__Object  5

#define RESULTSET_BUFSIZE 256
#define K_DSPI_QUERY 1

#include "include/query.h"
#include "include/dbapi.h"
#include "include/mysql.h"

/* ------------------------------------------------------------------------ */
/* Connection API */

typedef enum {
	USING_MYSQL,
	USING_SQLITE3,
	USING_POSTGRESQL,
	USING_NONE
} DBType;

const kQueryDSPI_t DB__mysql = {
	K_DSPI_QUERY, "mysql",
	MYSQL_qopen, MYSQL_query, MYSQL_qclose, MYSQL_qnext, MYSQL_qfree
};

static void Connection_init(KonohaContext *kctx, kObject *o, void *conf)
{
}

static void Connection_free(KonohaContext *kctx, kObject *o)
{
}

static KMETHOD Connection_new(KonohaContext *kctx, KonohaStack *sfp)
{
	//DBType type = (DBType)sfp[1].intValue;
	const char *dbname = S_text(sfp[1].asString);
	struct _kConnection* con = (struct _kConnection*)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].asObject), 0);
	//switch(type) {
	//	case USING_MYSQL:
	con->dspi = &DB__mysql;
	//		break;
	//	case USING_SQLITE3:
	//		break;
	//	case USING_POSTGRESQL:
	//		break;
	//	default:
	//		break;
	//}
	con->db = con->dspi->qopen(kctx, dbname);
	RETURN_(con);
}

static KMETHOD Connection_query(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kConnection *c = (struct _kConnection*)sfp[0].asObject;
	const char *query = S_text(sfp[1].asString);
	struct _kResultSet* rs = (struct _kResultSet*)KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].asObject), 0);
	kqcur_t *qcur = c->dspi->qexec(kctx, c->db, query, rs);
	if(qcur != NULL) {
		rs->qcur = qcur;
		rs->qcurfree = c->dspi->qcurfree;
	}
	else {
		rs->qcur = NULL;
		rs->qcurfree = NULL;
		//DP(rs)->qcurfree = knh_getQueryDSPI(kctx, K_DEFAULT_DSPI)->qcurfree;
	}
	KSETv(rs, rs->connection, c);
	RETURN_(rs);
}

static KMETHOD Connection_close(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kConnection* con = (struct _kConnection*)sfp[0].asObject;
	con->dspi->qclose(con->db);
}

/* ------------------------------------------------------------------------ */
/* Connection API */

static void ResultSet_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct _kResultSet *rs = (struct _kResultSet *)o;
	rs->qcur = NULL;
	rs->column_size = 0;
	rs->column = NULL;
	kBytes* ba = (kBytes*)KLIB new_kObject(kctx, CT_Bytes, RESULTSET_BUFSIZE);
	KINITv(rs->databuf, ba);
	KINITv(rs->connection, NULL);
	rs->qcurfree = NULL;
	rs->count = 0;
}

static void ResultSet_free(KonohaContext *kctx, kObject *o)
{
	struct _kResultSet *rs = (struct _kResultSet *)o;
	if (rs != NULL && rs->column_size > 0) {
		KFREE((void*)rs->column, sizeof(kDBschema) * rs->column_size);
	}
}

#define _Public   kMethod_Public
#define _F(F)   (intptr_t)(F)
#define _KVi(T)  #T, TY_int, T

static kbool_t sql_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	KRequirePackage("konoha.bytes", pline);

	static KDEFINE_CLASS ConnectionDef = {
		STRUCTNAME(Connection),
		.cflag = kClass_Final,
		.init = Connection_init,
		.free = Connection_free,
	};

	static KDEFINE_CLASS ResultSetDef = {
		STRUCTNAME(ResultSet),
		.cflag = kClass_Final,
		.init = ResultSet_init,
		.free = ResultSet_free,
	};

	KonohaClass *cConnection = KLIB kNameSpace_defineClass(kctx, ns, NULL, &ConnectionDef, pline);
	KonohaClass *cResultSet = KLIB kNameSpace_defineClass(kctx, ns, NULL, &ResultSetDef, pline);

	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Connection_new), TY_Connection, TY_Connection, MN_("new"), 1, TY_String, FN_("dbname"),
		_Public, _F(Connection_close), TY_void, TY_Connection, MN_("close"), 0,
		_Public, _F(Connection_query), TY_ResultSet, TY_Connection, MN_("query"), 1, TY_String, FN_("query"),

		_Public, _F(ResultSet_getString), TY_String, TY_ResultSet, MN_("getString"), 1, TY_String, FN_("query"),
		_Public, _F(ResultSet_next), TY_boolean, TY_ResultSet, MN_("next"), 0,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);

	KDEFINE_INT_CONST IntData[] = {
			_KVi(USING_MYSQL),
			_KVi(USING_SQLITE3),
			_KVi(USING_POSTGRESQL),
			{}
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(IntData), pline);
	return true;
}

static kbool_t sql_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t sql_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t sql_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* sql_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("sql", "1.0"),
		.initPackage    = sql_initPackage,
		.setupPackage   = sql_setupPackage,
		.initNameSpace  = sql_initNameSpace,
		.setupNameSpace = sql_setupNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
