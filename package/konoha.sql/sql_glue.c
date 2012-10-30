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
#include <minikonoha/sugar.h>
#include <konoha_sql.config.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ======================================================================== */
/* [struct] */
/* ------------------------------------------------------------------------ */
/* [Connection] */

typedef const struct _kQueryDPI_t kQueryDSPI_t;
typedef const struct _kConnection kConnection;

struct _kConnection {
	KonohaObjectHeader  h;
	void                *db;
	const struct _kQueryDPI_t *dspi;
};

#define TY_Connection     cConnection->typeId

/* ------------------------------------------------------------------------ */
/* [ResultSet] */

typedef const struct DBschema DBschema;

struct DBschema {
	ktype_t   type;   // konoha defined type
	kushort_t ctype;  // ResultSet defined type. ... Is it obsolate?
	kString   *name;
	size_t    start;
	size_t    len;
	int       dbtype; // DBMS defined type
};

typedef void kqcur_t;

typedef const struct _kResultSet kResultSet;
struct _kResultSet{
	KonohaObjectHeader  h;
	kConnection         *connection;
	void                *qcur;
	void                (*qcurfree)(kqcur_t *); /* necessary if conn is closed before */
	kString             *tableName;
	kushort_t           column_size;
	struct DBschema     *column;
	KGrowingArray       databuf;
	size_t              rowidx;
};

#define TY_ResultSet     cResultSet->typeId

/* ------------------------------------------------------------------------ */
/* K_DSPI_QUERY */

struct _kQueryDPI_t {
	int   type;
	const char *name;
	void* (*qopen)(KonohaContext* kctx, const char* db);
	kqcur_t* (*qexec)(KonohaContext* kctx, void* db, const char* query, struct _kResultSet* rs);
	void   (*qclose)(void* hdr);
	int    (*qcurnext)(KonohaContext* kctx, kqcur_t * qcur, struct _kResultSet* rs);
	void   (*qcurfree)(kqcur_t *);
};

/* ------------------------------------------------------------------------ */
/* [Macros] */

#define kResultSet_CTYPE__null    0
#define kResultSet_CTYPE__integer 1
#define kResultSet_CTYPE__float   2
#define kResultSet_CTYPE__text    3  /* UTF-8*/
//#define kResultSet_CTYPE__bytes   4
//#define kResultSet_CTYPE__Object  5

#define RESULTSET_BUFSIZE 256
#define K_DSPI_QUERY 1

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
/* ------------------------------------------------------------------------ */
/* [NOP DB Driver] */

void NOP_qfree(kqcur_t *qcur)
{
}

void *NOP_qopen(KonohaContext *kctx, const char* url)
{
	return NULL;
}
kqcur_t *NOP_query(KonohaContext *kctx, void *hdr, const char* sql, struct _kResultSet *rs)
{
	return NULL;
}
void NOP_qclose(void *db)
{
}
int NOP_qnext(KonohaContext *kctx, kqcur_t *qcur, struct _kResultSet *rs)
{
	return 0;  /* NOMORE */
}

static kQueryDSPI_t DB__NOP = {
	K_DSPI_QUERY, "NOP",
	NOP_qopen, NOP_query, NOP_qclose, NOP_qnext, NOP_qfree
};

/* ======================================================================== */
/* ------------------------------------------------------------------------ */
/* [Connection Class Define] */

static void Connection_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct _kConnection *con = (struct _kConnection*)o;
	con->db = NULL;
	con->dspi = NULL;
}

static void Connection_free(KonohaContext *kctx, kObject *o)
{
	struct _kConnection *con = (struct _kConnection*)o;
	if (con->db != NULL) {
		con->dspi->qclose(con->db);
		con->db = NULL;
	}
	con->dspi = NULL;
}

static void Connection_reftrace(KonohaContext *kctx, kObject *p, KObjectVisitor *visitor)
{
}

/* ------------------------------------------------------------------------ */
/* [Connection API] */

int dbName_Length(const char* p) {
	char ch;
	ch = p[0];
	int i = 0;
	while(ch != '\0') {
		if (ch == ':') {
			return i;
		}
		i++;
		ch = p[i];
	}
	return -1;
}

#define DB_NAMESIZE 32

//## Connection Connection.new();
static KMETHOD Connection_new(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *query = S_text(sfp[1].asString);
	struct _kConnection* con = (struct _kConnection*)KLIB new_kObject(kctx, OnStack, O_ct(sfp[K_RTNIDX].asObject), 0);
	int len = dbName_Length(query);
	char dbname[DB_NAMESIZE] = {0};
	strncpy(dbname, query, len);
	if (strncmp(dbname, "mysql", len) == 0) {
#ifdef MYSQL_INCLUDED_
		con->dspi = &DB__mysql;
#else
		goto L_ReturnNULL;
#endif
	}
	else if (strncmp(dbname, "sqlite3", len) == 0) {
#ifdef SQLITE_INCLUDED_
		con->dspi = &DB__sqlite3;
#else
		goto L_ReturnNULL;
#endif
	}
	else if (strncmp(dbname, "postgresql", len) == 0) {
#ifdef PSQL_INCLUDED_
		con->dspi = &DB__postgresql;
#else
		goto L_ReturnNULL;
#endif
	}
	else {
#if (!MYSQL_INCLUDED_ | !SQLITE_INCLUDED_ | !PSQL_INCLUDED_)
L_ReturnNULL:
#endif
		con->dspi = &DB__NOP;
		//KReturn((kConnection*)KLIB Knull(kctx, O_ct(sfp[0].asObject)));
	}
	con->db = con->dspi->qopen(kctx, query);
	KReturn(con);
}

//## ResultSet Connection.query(String query);
static KMETHOD Connection_query(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kConnection *c = (struct _kConnection *)sfp[0].asObject;
	const char *query = S_text(sfp[1].asString);
	struct _kResultSet* rs = (struct _kResultSet *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	kqcur_t *qcur = c->dspi->qexec(kctx, c->db, query, rs);
	if(qcur != NULL) {
		rs->qcur = qcur;
		rs->qcurfree = c->dspi->qcurfree;
	}
	else {
		rs->qcur = NULL;
		rs->qcurfree = NULL;
		//DP(rs)->qcurfree = kgetQueryDSPI(kctx, K_DEFAULT_DSPI)->qcurfree;
	}
	KFieldSet(rs, rs->connection, c);
	KReturn(rs);
}

//## void Connection.close();
static KMETHOD Connection_close(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kConnection* con = (struct _kConnection *)sfp[0].asObject;
	con->dspi->qclose(con->db);
	con->db = NULL;
}

#ifdef MYSQL_INCLUDED_

//## int Connection.getInsertId();
KMETHOD Connection_getInsertId(KonohaContext *kctx, KonohaStack *sfp)
{
	kConnection *c = (kConnection*)sfp[0].asObject;
	kint_t ret = 0;
	if (strncmp(c->dspi->name, "mysql", sizeof("mysql")) == 0) {
		ret = (kint_t)mysql_insert_id((MYSQL*)c->db);
	}
	else {
		// [TODO] throw exeption when another dbms use this method.
	}
	KReturnUnboxValue(ret);
}

#endif
/* ======================================================================== */
/* ------------------------------------------------------------------------ */
/* [ResultSet Class Define] */

static void ResultSet_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct _kResultSet *rs = (struct _kResultSet *)o;
	rs->qcur = NULL;
	rs->column_size = 0;
	rs->column = NULL;
	KLIB Karray_init(kctx, &rs->databuf, RESULTSET_BUFSIZE);
	KFieldInit(rs, rs->connection, NULL);
	KFieldInit(rs, rs->tableName, KNULL(String));
	rs->qcurfree = NULL;
	rs->rowidx = 0;
}

static void ResultSet_free(KonohaContext *kctx, kObject *o)
{
	struct _kResultSet *rs = (struct _kResultSet *)o;
	if(rs != NULL && rs->column_size > 0) {
		KFree((void *)rs->column, sizeof(DBschema) * rs->column_size);
	}
	if(rs->qcur != NULL) {
		rs->qcurfree(rs->qcur);
		rs->qcur = NULL;
		//rs->qcurfree = kgetQueryDSPI(ctx, t)->qcurfree;
	}
	rs->connection = NULL;
}

static void ResultSet_reftrace(KonohaContext *kctx, kObject *p, KObjectVisitor *visitor)
{
	struct _kResultSet *rs = (struct _kResultSet *)p;
	BEGIN_REFTRACE(2);
	KREFTRACEv(rs->connection);
	KREFTRACEv(rs->tableName);
	END_REFTRACE();
}

static void ResultSet_p(KonohaContext *kctx, KonohaValue *v, int pos, KGrowingBuffer *wb)
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
	//			kwrite_ifmt(ctx, w, KINT_FMT, (*((kint_t*)p)));
	//			break;
	//		case kResultSet_CTYPE__float :
	//			kwrite_ffmt(ctx, w, KFLOAT_FMT, (*((kfloat_t*)p)));
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

int _ResultSet_findColumn(KonohaContext *kctx, kResultSet *o, const char* name)
{
	size_t i = 0;
	for(i = 0; i < o->column_size; i++) {
		if(strcasecmp(S_text(o->column[i].name), name) == 0) return i;
	}
	return -1;
}

static int _ResultSet_indexof_(KonohaContext *kctx, KonohaStack *sfp)
{
	kResultSet *o = (kResultSet*)sfp[0].asObject;
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
	struct _kResultSet* rs = (struct _kResultSet*)sfp[0].asObject;
	kbool_t ret = false;
	if(rs->qcur != NULL) {
		if(rs->connection->dspi->qcurnext(kctx, rs->qcur, rs)) {
			rs->rowidx++;
			ret = true;
		}
		else {
			rs->qcurfree(rs->qcur);
			rs->qcur = NULL;
			//o->qcurfree = kgetQueryDSPI(ctx, t)->qcurfree;
		}
	}
	KReturnUnboxValue(ret);
}

//## int ResultSet.getSize();
KMETHOD ResultSet_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kResultSet* rs = (struct _kResultSet*)sfp[0].asObject;
	KReturnUnboxValue(rs->column_size);
}

//## String ResultSet.getName(Int n);
KMETHOD ResultSet_getName(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kResultSet* rs = (struct _kResultSet*)sfp[0].asObject;
	size_t n = (size_t)sfp[1].intValue;
	kString *ret = TS_EMPTY;
	if(n < rs->column_size) {
		KNH_ASSERT(n < rs->column_size);
		const char* raw_name = S_text(rs->column[n].name);
		ret = KLIB new_kString(kctx, GcUnsafe, raw_name, strlen(raw_name), 0);
	}
	else {
		//THROW_OutOfRange(kctx, sfp, sfp[1].intValue, rs->column_size);
	}
	KReturn(ret);
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
		kResultSet *o = (kResultSet*)sfp[0].asObject;
		const char *p = o->databuf.bytebuf + o->column[n].start;
		switch((o)->column[n].ctype) {
		case kResultSet_CTYPE__integer :
			memset(data, '\0', 16);
			memcpy(data, p, o->column[n].len);
			res = strtoll(data, NULL, 10);
			break;
		case kResultSet_CTYPE__float :
			res = (kint_t)(*((kfloat_t*)p)); break;
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
		kResultSet *o = (kResultSet*)sfp[0].asObject;
		const char *p = o->databuf.bytebuf + o->column[n].start;
		switch((o)->column[n].ctype) {
		case kResultSet_CTYPE__integer :
			res = (kfloat_t)(*((kint_t*)p)); break;
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
	kResultSet* rs = (kResultSet*)sfp[0].asObject;
	DBG_ASSERT(n < rs->column_size);
	const char *p = rs->databuf.bytebuf + rs->column[n].start;
	switch(rs->column[n].ctype) {
	case kResultSet_CTYPE__integer :
		KReturn(KLIB new_kString(kctx, GcUnsafe, p, rs->column[n].len, 0));
		break;
	case kResultSet_CTYPE__float :
		break;
		//return new_String__float(kctx, (kfloat_t)(*((kfloat_t*)p)));
	case kResultSet_CTYPE__text : {
		KReturn(KLIB new_kString(kctx, GcUnsafe, p, rs->column[n].len, 0));
		}
	case kResultSet_CTYPE__null :
		break;
	}
	KReturn(KLIB new_kString(kctx, GcUnsafe, "", 0, 0));
}

//## method dynamic ResultSet.get(dynamic n);
//KMETHOD ResultSet_get(KonohaContext *kctx, KonohaStack *sfp)
//{
//	
//	int n = _ResultSet_indexof_(ctx, sfp);
//	Object *v = KNH_NULL;
//	if(n >= 0) {
//		kResultSet *o = (kResultSet*)sfp[0].o;
//		const char *p = BA_totext((o)->databuf) + (o)->column[n].start;
//		switch((o)->column[n].ctype) {
//		case kResultSet_CTYPE__integer : {
//			kint_t val;
//			memcpy(&val, p, sizeof(kint_t));
//			KNH_SETv(ctx, sfp[_rix].o, new_Int_(ctx, CLASS_Int, val));
//			KReturnUnboxValue((*((kint_t*)p)));
//		}
//		case kResultSet_CTYPE__float : {
//			kfloat_t val;
//			knh_memcpy(&val, p, sizeof(kfloat_t));
//			KNH_SETv(ctx, sfp[_rix].o, new_Float_(ctx, CLASS_Float, val));
//			KReturnFloatValue((*((kfloat_t*)p)));
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

static kbool_t sql_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	KRequirePackage("konoha.float", trace);

	static KDEFINE_CLASS ConnectionDef = {
		STRUCTNAME(Connection),
		.cflag = kClass_Final,
		.init = Connection_init,
		.free = Connection_free,
		.reftrace = Connection_reftrace,
	};

	static KDEFINE_CLASS ResultSetDef = {
		STRUCTNAME(ResultSet),
		.cflag = kClass_Final,
		.init = ResultSet_init,
		.free = ResultSet_free,
		.reftrace = ResultSet_reftrace,
		.p = ResultSet_p,
	};

	KonohaClass *cConnection = KLIB kNameSpace_defineClass(kctx, ns, NULL, &ConnectionDef, trace);
	KonohaClass *cResultSet = KLIB kNameSpace_defineClass(kctx, ns, NULL, &ResultSetDef, trace);

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

static kbool_t sql_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* sql_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("sql", "1.0"),
		.initPackage    = sql_initPackage,
		.setupPackage   = sql_setupPackage,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
