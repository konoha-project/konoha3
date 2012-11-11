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

#include <minikonoha/minikonoha.h>
#include <minikonoha/konoha_common.h>
#include <minikonoha/klib.h>

//#include <minikonoha/sugar.h>
#include "konoha_sql.config.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MYSQL_INCLUDED_
#include "include/mysql.h"
#endif

#ifdef SQLITE_INCLUDED_
#include "include/sqlite.h"
#endif

#ifdef PSQL_INCLUDED_
#include "include/postgresql.h"
#endif

/* ======================================================================== */
/* [Connection] */

typedef const struct QueryDriver QueryDriver;
typedef struct kConnectionVar kConnection;
typedef struct kResultSetVar  kResultSet;
typedef struct DBschema DBschema;

typedef void kqcur_t;

struct QueryDriver {
	int   type;
	const char *name;
	void* (*qopen)(KonohaContext* kctx, const char* db, KTraceInfo *trace);
	kqcur_t* (*qexec)(KonohaContext* kctx, void* db, const char* query, kResultSet* rs, KTraceInfo *trace);
	void   (*qclose)(void* hdr);
	int    (*qcurnext)(KonohaContext* kctx, kqcur_t * qcur, kResultSet* rs, KTraceInfo *trace);
	void   (*qcurfree)(kqcur_t *);
};

struct kConnectionVar {
	KonohaObjectHeader  h;
	void                *db;
	QueryDriver *driver;
};

#define TY_Connection     cConnection->typeId

/* [ResultSet] */

struct DBschema {
	ktype_t   type;   // konoha defined type
	kushort_t ctype;  // ResultSet defined type. ... Is it obsolate?
	kString   *name;
	size_t    start;
	size_t    len;
	int       dbtype; // DBMS defined type
};

struct kResultSetVar {
	KonohaObjectHeader  h;
	kConnection         *connection;
	void                *qcur;
	void                (*qcurfree)(kqcur_t *); /* necessary if conn is closed before */
	kushort_t           column_size;
	kString             *tableName;
	struct DBschema     *column;
	KGrowingArray       databuf;
	size_t              rowidx;
};

#define TY_ResultSet     cResultSet->typeId

/* ------------------------------------------------------------------------ */
/* [Macros] */

typedef enum {
	kResultSet_CTYPE__null,
	kResultSet_CTYPE__integer,
	kResultSet_CTYPE__float,
	kResultSet_CTYPE__text,
} kResultSet_CTYPE;

#define RESULTSET_BUFSIZE 256
#define K_DSPI_QUERY 1

/* ======================================================================== */
/* ------------------------------------------------------------------------ */
/* [NOP DB Driver] */


void *NOP_qopen(KonohaContext *kctx, const char* url, KTraceInfo *trace)
{
	return NULL;
}
kqcur_t *NOP_query(KonohaContext *kctx, void *hdr, const char* sql, kResultSet *rs, KTraceInfo *trace)
{
	return NULL;
}
void NOP_qclose(void *db)
{
}
int NOP_qnext(KonohaContext *kctx, kqcur_t *qcur, kResultSet *rs, KTraceInfo *trace)
{
	return 0;  /* NOMORE */
}
void NOP_qfree(kqcur_t *qcur)
{
}

static QueryDriver NoQueryDriver = {
	K_DSPI_QUERY, "NOP", NOP_qopen, NOP_query, NOP_qclose, NOP_qnext, NOP_qfree
};

static int HandlerNameLength(const char* p) {
	char ch;
	ch = p[0];
	int i = 0;
	while(ch != '\0') {
		if(ch == ':') {
			return i;
		}
		i++;
		ch = p[i];
	}
	return i;
}

static QueryDriver* LoadQueryDriver(KonohaContext *kctx, const char *dburl)
{
	int len = HandlerNameLength(dburl);
#ifdef MYSQL_INCLUDED_
	if(strncmp(dburl, "mysql", len) == 0) {
		return &MySQLDriver;
	}
#endif
#ifdef SQLITE_INCLUDED_
	if(strncmp(dburl, "sqlite3", len) == 0) {
		return &SQLLite3Driver;
	}
#endif
#ifdef PSQL_INCLUDED_
	if(strncmp(dburl, "postgresql", len) == 0) {
		con->driver = &PostgreSQLDriver;
	}
#endif
	return &NoQueryDriver;
}

/* ------------------------------------------------------------------------ */
/* [Connection Class Define] */

static void kConnection_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kConnection *con = (kConnection *)o;
	con->db = conf;
	con->driver = &NoQueryDriver;
}

static void kConnection_reftrace(KonohaContext *kctx, kObject *p, KObjectVisitor *visitor)
{
}

static void kConnection_free(KonohaContext *kctx, kObject *o)
{
	kConnection *con = (kConnection *)o;
	if(con->db != NULL) {
		con->driver->qclose(con->db);
		con->db = NULL;
	}
	con->driver = &NoQueryDriver;  // safety
}

/* ------------------------------------------------------------------------ */
/* [Connection API] */


#define DB_NAMESIZE 32

//## Connection Connection.new(String dburl);
static KMETHOD Connection_new(KonohaContext *kctx, KonohaStack *sfp)
{
	KMakeTrace(trace, sfp);
	const char *dburl = S_text(sfp[1].asString);
	QueryDriver *driver = LoadQueryDriver(kctx, dburl);
	void *db = driver->qopen(kctx, dburl, trace);
	if(db != NULL) {
		kConnection* con = (kConnection *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), (uintptr_t)db);
		con->driver = driver;
		KReturn(con);
	}
}

//## ResultSet Connection.query(String query);
static KMETHOD Connection_query(KonohaContext *kctx, KonohaStack *sfp)
{
	INIT_GCSTACK();
	KMakeTrace(trace, sfp);
	kConnection *conn = (kConnection *)sfp[0].asObject;
	const char *query = S_text(sfp[1].asString);
	kResultSet* rs = (kResultSet *)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), (uintptr_t)conn);
	kqcur_t *qcur = conn->driver->qexec(kctx, conn->db, query, rs, trace);
	if(qcur != NULL) {
		rs->qcur = qcur;
		rs->qcurfree = conn->driver->qcurfree;
	}
	else {
		rs->qcur = NULL;
		rs->qcurfree = NULL;
	}
	KReturnWith(rs, RESET_GCSTACK());
}

//## void Connection.close();
static KMETHOD Connection_close(KonohaContext *kctx, KonohaStack *sfp)
{
	kConnection* conn = (kConnection *)sfp[0].asObject;
	conn->driver->qclose(conn->db);
	conn->db = NULL;
}

#ifdef MYSQL_INCLUDED_

//## int Connection.getInsertId();
KMETHOD Connection_getInsertId(KonohaContext *kctx, KonohaStack *sfp)
{
	kConnection *c = (kConnection *)sfp[0].asObject;
	kint_t ret = 0;
	if(strncmp(c->driver->name, "mysql", sizeof("mysql")) == 0) {
		ret = (kint_t)mysql_insert_id((MYSQL *)c->db);
	}
	else {
		// [TODO] throw exeption when another dbms use this method.
	}
	KReturnUnboxValue(ret);
}

#endif

/* ------------------------------------------------------------------------ */
/* [ResultSet Class Define] */

static void kResultSet_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kResultSet *rs = (kResultSet *)o;
	rs->qcur = NULL;
	rs->column_size = 0;
	rs->column = NULL;
	KLIB Karray_init(kctx, &rs->databuf, RESULTSET_BUFSIZE);
	KFieldInit(rs, rs->connection, (kConnection *)conf);
	KFieldInit(rs, rs->tableName, KNULL(String));
	rs->qcurfree = NULL;
	rs->rowidx = 0;
}

static void kResultSet_reftrace(KonohaContext *kctx, kObject *p, KObjectVisitor *visitor)
{
	kResultSet *rs = (kResultSet *)p;
	KREFTRACEv(rs->connection);
	KREFTRACEv(rs->column->name);
	KREFTRACEv(rs->tableName);
}

static void kResultSet_free(KonohaContext *kctx, kObject *o)
{
	kResultSet *rs = (kResultSet *)o;
	if(rs->column_size > 0) {
		KFree((void *)rs->column, sizeof(DBschema) * rs->column_size);
	}
	if(rs->qcur != NULL) {
		rs->qcurfree(rs->qcur);
		rs->qcur = NULL;
	}
	rs->connection = NULL;
}


static void kResultSet_p(KonohaContext *kctx, KonohaValue *v, int pos, KGrowingBuffer *wb)
{
	//KLIB Kwb_printf(kctx, wb, "%s", "{");
	//size_t n;
	//for(n = 0; n < (o)->column_size; n++) {
	//	if(n > 0) {
	//		kwrite_delim(ctx,w);
	//	}
	//	KLIB Kwb_printf(kctx, w, "(%d): ", n);
	//	char *p = BA_totext((o)->databuf) + (o)->column[n].start;
	//	switch((o)->column[n].ctype) {
	//		case kResultSet_CTYPE__null :
	//			kwrite(ctx, w, STEXT("null"));
	//			break;
	//		case kResultSet_CTYPE__integer :
	//			kwrite_ifmt(ctx, w, KINT_FMT, (*((kint_t *)p)));
	//			break;
	//		case kResultSet_CTYPE__float :
	//			kwrite_ffmt(ctx, w, KFLOAT_FMT, (*((kfloat_t *)p)));
	//			break;
	//		case kResultSet_CTYPE__text :
	//			kwrite(ctx, w, B2(p, (o)->column[n].len));
	//			break;
	//		case kResultSet_CTYPE__bytes :
	//			kprintf(ctx, w, "BLOB(%dbytes)", (o)->column[n].len);
	//			break;
	//	}
	//}
	//kputc(ctx, w, '}');
}

/* ------------------------------------------------------------------------ */
/* [ResultSet inner function] */

static int _ResultSet_findColumn(KonohaContext *kctx, kResultSet *o, const char* name)
{
	size_t i = 0;
	for(i = 0; i < o->column_size; i++) {
		if(strcasecmp(S_text(o->column[i].name), name) == 0) return i;
	}
	return -1;
}

static int _ResultSet_indexof_(KonohaContext *kctx, KonohaStack *sfp)
{
	kResultSet *o = (kResultSet *)sfp[0].asObject;
	if(IS_Int(sfp[1].asObject)) {
		size_t n = (size_t)sfp[1].intValue;
		if(!(n < o->column_size)) {
			//THROW_OutOfRange(ctx, sfp, sfp[1].intValue, (o)->column_size);
			return -1;
		}
		return n;
	}
	else if(IS_String(sfp[1].asObject)) {
		int loc = _ResultSet_findColumn(kctx, o, S_text(sfp[1].asString));
		if(loc == -1) {
			//THROW_OutOfRange(ctx, sfp, sfp[1].intValue, (o)->column_size);
		}
		return loc;
	}
	return -1;
}

/* ------------------------------------------------------------------------ */
/* [ResultSet API] */

//## Boolean ResultSet.next();

KMETHOD ResultSet_next(KonohaContext *kctx, KonohaStack *sfp)
{
	kResultSet* rs = (kResultSet *)sfp[0].asObject;
	kbool_t ret = false;
	DBG_ASSERT(rs->qcur != NULL);
	KMakeTrace(trace, sfp);
	if(rs->connection->driver->qcurnext(kctx, rs->qcur, rs, trace)) {
		rs->rowidx++;
		ret = true;
	}
	else {
		rs->qcurfree(rs->qcur);
		rs->qcur = NULL;
		//o->qcurfree = kgetQueryDSPI(ctx, t)->qcurfree;
	}
	KReturnUnboxValue(ret);
}

//## int ResultSet.getSize();
KMETHOD ResultSet_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kResultSet* rs = (kResultSet *)sfp[0].asObject;
	KReturnUnboxValue(rs->column_size);
}

//## String ResultSet.getName(Int n);
KMETHOD ResultSet_getName(KonohaContext *kctx, KonohaStack *sfp)
{
	kResultSet* rs = (kResultSet *)sfp[0].asObject;
	size_t n = (size_t)sfp[1].intValue;
	KCheckIndex(n, rs->column_size);
	KReturn(rs->column[n].name);
}
/* ------------------------------------------------------------------------ */
/* [getter/setter] */

//## Int ResultSet.getInt(String n);
KMETHOD ResultSet_getInt(KonohaContext *kctx, KonohaStack *sfp)
{
	int n = _ResultSet_indexof_(kctx, sfp);
	kint_t res = 0;
	char data[16];
	if(n >= 0) {
		kResultSet *o = (kResultSet *)sfp[0].asObject;
		const char *p = o->databuf.bytebuf + o->column[n].start;
		switch((o)->column[n].ctype) {
		case kResultSet_CTYPE__integer :
			memset(data, '\0', 16);
			memcpy(data, p, o->column[n].len);
			res = strtoll(data, NULL, 10);
			break;
		case kResultSet_CTYPE__float :
			res = (kint_t)(*((kfloat_t *)p)); break;
		case kResultSet_CTYPE__null :
		default:
			//KSETv(NULL, sfp[_rix].o, KNH_NULVAL(CLASS_Int));
			break;
		}
	}
	KReturnUnboxValue(res);
}

#define RESULTSET_FLOATSIZE 16

//## Float ResultSet.getFloat(String n);
KMETHOD ResultSet_getFloat(KonohaContext *kctx, KonohaStack *sfp)
{
	int n = _ResultSet_indexof_(kctx, sfp);
	kfloat_t res = 0.0;
	char data[RESULTSET_FLOATSIZE];
	if(n >= 0) {
		kResultSet *o = (kResultSet *)sfp[0].asObject;
		const char *p = o->databuf.bytebuf + o->column[n].start;
		switch((o)->column[n].ctype) {
		case kResultSet_CTYPE__integer :
			res = (kfloat_t)(*((kint_t *)p)); break;
		case kResultSet_CTYPE__float :
			memset(data, '\0', RESULTSET_FLOATSIZE);
			memcpy(data, p, o->column[n].len);
			res = atof(data);
			break;
		case kResultSet_CTYPE__null :
		default:
			//KSETv(NULL, sfp[_rix].o, KNH_NULVAL(CLASS_Float));
			break;
		}
	}
	KReturnFloatValue(res);
}

//## String ResultSet.getString(String n);
KMETHOD ResultSet_getString(KonohaContext *kctx, KonohaStack *sfp)
{
	size_t n = (size_t)_ResultSet_indexof_(kctx, sfp);
	kResultSet* rs = (kResultSet *)sfp[0].asObject;
	DBG_ASSERT(n < rs->column_size);
	const char *p = rs->databuf.bytebuf + rs->column[n].start;
	switch(rs->column[n].ctype) {
	case kResultSet_CTYPE__integer :
		KReturn(KLIB new_kString(kctx, GcUnsafe, p, rs->column[n].len, 0));
		break;
	case kResultSet_CTYPE__float :
		break;
		//return new_String__float(kctx, (kfloat_t)(*((kfloat_t *)p)));
	case kResultSet_CTYPE__text : {
		KReturn(KLIB new_kString(kctx, GcUnsafe, p, rs->column[n].len, 0));
		}
	case kResultSet_CTYPE__null :
		break;
	}
	KReturn(TS_EMPTY);
}

//## method dynamic ResultSet.get(dynamic n);
//KMETHOD ResultSet_get(KonohaContext *kctx, KonohaStack *sfp)
//{
//	
//	int n = _ResultSet_indexof_(ctx, sfp);
//	Object *v = KNH_NULL;
//	if(n >= 0) {
//		kResultSet *o = (kResultSet *)sfp[0].o;
//		const char *p = BA_totext((o)->databuf) + (o)->column[n].start;
//		switch((o)->column[n].ctype) {
//		case kResultSet_CTYPE__integer : {
//			kint_t val;
//			memcpy(&val, p, sizeof(kint_t));
//			KNH_SETv(ctx, sfp[_rix].o, new_Int_(ctx, CLASS_Int, val));
//			KReturnUnboxValue((*((kint_t *)p)));
//		}
//		case kResultSet_CTYPE__float : {
//			kfloat_t val;
//			knh_memcpy(&val, p, sizeof(kfloat_t));
//			KNH_SETv(ctx, sfp[_rix].o, new_Float_(ctx, CLASS_Float, val));
//			KReturnFloatValue((*((kfloat_t *)p)));
//		}
//		case kResultSet_CTYPE__text : {
//			kbytes_t t = {{BA_totext((o)->databuf) + (o)->column[n].start}, (o)->column[n].len};
//			v = UPCAST(new_S(t.text, t.len));
//			break;
//		}
//		case kResultSet_CTYPE__bytes :
//			{
//				kBytes *ba = new_Bytes(ctx, BA_totext((o)->databuf) + (o)->column[n].start, (o)->column[n].len);
//				kbytes_t t = {{BA_totext((o)->databuf) + (o)->column[n].start}, (o)->column[n].len};
//				kBytes_write(ctx, ba, t);
//				v = UPCAST(ba);
//			}
//			break;
//		default:
//			v = KNH_NULL;
//		}
//	}
//	KReturn(v);
//}
//

#define _Public   kMethod_Public
#define _F(F)   (intptr_t)(F)

static kbool_t sql_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequirePackage("konoha.float", trace);

	static KDEFINE_CLASS ConnectionDef = {
		STRUCTNAME(Connection),
		.cflag = kClass_Final,
		.init = kConnection_init,
		.free = kConnection_free,
		.reftrace = kConnection_reftrace,
	};

	static KDEFINE_CLASS ResultSetDef = {
		STRUCTNAME(ResultSet),
		.cflag = kClass_Final,
		.init = kResultSet_init,
		.free = kResultSet_free,
		.reftrace = kResultSet_reftrace,
		.p = kResultSet_p,
	};

	KonohaClass *cConnection = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &ConnectionDef, trace);
	KonohaClass *cResultSet = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &ResultSetDef, trace);

	KDEFINE_METHOD MethodData[] = {
		/* Connection method */
		_Public, _F(Connection_new), TY_Connection, TY_Connection, MN_("new"), 1, TY_String, FN_("dbname"),
		_Public, _F(Connection_close), TY_void, TY_Connection, MN_("close"), 0,
		_Public, _F(Connection_query), TY_ResultSet, TY_Connection, MN_("query"), 1, TY_String, FN_("query"),
#ifdef MYSQL_INCLUDED_
		_Public, _F(Connection_getInsertId), TY_int, TY_Connection, MN_("getInsertId"), 0,
#endif

		/* ResultSet method */
		_Public, _F(ResultSet_getInt), TY_int, TY_ResultSet, MN_("getInt"), 1, TY_String, FN_("query"),
		_Public, _F(ResultSet_getFloat), TY_float, TY_ResultSet, MN_("getFloat"), 1, TY_String, FN_("query"),
		_Public, _F(ResultSet_getString), TY_String, TY_ResultSet, MN_("getString"), 1, TY_String, FN_("query"),
		_Public, _F(ResultSet_next), TY_boolean, TY_ResultSet, MN_("next"), 0,
		_Public, _F(ResultSet_getSize), TY_int, TY_ResultSet, MN_("getSize"), 0,
		_Public, _F(ResultSet_getName), TY_String, TY_ResultSet, MN_("getName"), 1, TY_int, FN_("idx"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

static kbool_t sql_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* sql_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("Simple Sql", "1.0"),
		.PackupNameSpace    = sql_PackupNameSpace,
		.ExportNameSpace   = sql_ExportNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
