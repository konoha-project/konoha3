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

/* ************************************************************************ */

#include <konoha3/konoha.h>
#include <konoha3/konoha_common.h>
#include <konoha3/klib.h>
#include <konoha3/import/methoddecl.h>
#include <stdio.h> /* for strtoll, strtod */

#ifdef HAVE_KONOHA_SQL_CONFIG_H
#include "konoha_sql.config.h"
#endif

typedef void KCursor;
typedef void DBHandler;
struct kResultSet;

typedef const struct QueryDriverVar {
	const char *name;
	DBHandler *(*qopen)(KonohaContext* kctx, const char *url, KTraceInfo *trace);
	KCursor   *(*qexec)(KonohaContext* kctx, DBHandler *hdr, const char *query, struct kResultSet *rs, KTraceInfo *trace);
	void       (*qclose)(DBHandler *hdl);
	int        (*qcurnext)(KonohaContext* kctx, KCursor *qcur, struct kResultSet *rs, KTraceInfo *trace);
	void       (*qcurfree)(KCursor *);
} QueryDriver;

typedef struct kConnection {
	kObjectHeader  h;
	DBHandler   *db;
	QueryDriver *driver;
} kConnection;

#define KType_Connection     cConnection->typeId

/* [ResultSet] */

typedef struct KColumn {
	ktypeattr_t  type;
	int          dbtype;
	kString     *name;
	krbp_t       val;
} KColumn;

typedef struct kResultSet {
	kObjectHeader  h;
	kConnection  *connectionNULL;
	KCursor      *qcur;
	KColumn      *column;
	size_t        column_size;
	size_t        rowidx;
	QueryDriver  *driver; /* necessary if connection is closed before */
} kResultSet;

#define KType_ResultSet     cResultSet->typeId

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */
/* [NOP DB Driver] */

static DBHandler *NOP_qopen(KonohaContext *kctx, const char *url, KTraceInfo *trace)
{
	return NULL;
}

static KCursor *NOP_query(KonohaContext *kctx, DBHandler *hdr, const char *query, kResultSet *rs, KTraceInfo *trace)
{
	return NULL;
}

static void NOP_qclose(DBHandler *db)
{
}

static int NOP_qnext(KonohaContext *kctx, KCursor *qcur, kResultSet *rs, KTraceInfo *trace)
{
	return 0; /* NOMORE */
}

static void NOP_qfree(KCursor *qcur)
{
}

static QueryDriver NoQueryDriver = {
	"NOP", NOP_qopen, NOP_query, NOP_qclose, NOP_qnext, NOP_qfree
};

/* ------------------------------------------------------------------------ */
/* [Connection Class Define] */

static void kConnection_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kConnection *con = (kConnection *)o;
	con->db = conf;
	con->driver = &NoQueryDriver;
}

static void kConnection_Free(KonohaContext *kctx, kObject *o)
{
	kConnection *con = (kConnection *)o;
	if(con->db != NULL) {
		con->driver->qclose(con->db);
		con->db = NULL;
	}
	con->driver = &NoQueryDriver; // safety
}

static void kResultSet_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kResultSet *rs = (kResultSet *)o;
	if(conf != NULL) {
		KFieldInit(rs, rs->connectionNULL, (kConnection *) conf);
	} else {
		rs->connectionNULL = NULL;
	}
	rs->qcur = NULL;
	rs->column_size = 0;
	rs->column = NULL;
	rs->rowidx = 0;
	rs->driver = NULL;
}

static void kResultSet_Reftrace(KonohaContext *kctx, kObject *p, KObjectVisitor *visitor)
{
	kResultSet *rs = (kResultSet *)p;
	KColumn *col = rs->column;
	KColumn *end = col + rs->column_size;
	while(col < end) {
		KRefTrace(col->name);
		if(KType_Is(UnboxType, col->type)) {
			KRefTrace(col->val.asObject);
		}
	}
	KRefTraceNullable(rs->connectionNULL);
}

static void kResultSet_Free(KonohaContext *kctx, kObject *o)
{
	kResultSet *rs = (kResultSet *)o;
	if(rs->column_size > 0) {
		KFree(rs->column, sizeof(KColumn) * rs->column_size);
	}
	if(rs->qcur != NULL) {
		rs->driver->qcurfree(rs->qcur);
		rs->qcur = NULL;
	}
	rs->connectionNULL = NULL;
}

static void kResultSet_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kResultSet *rs = (kResultSet *) v[0].asObject;
	KLIB KBuffer_printf(kctx, wb, "{");
	size_t i;
	for(i = 0; i < (rs)->column_size; i++) {
		if(i > 0) {
			KLIB KBuffer_printf(kctx, wb, ",");
		}
		KLIB KBuffer_printf(kctx, wb, "(%d): ", i);
		ktypeattr_t type = rs->column[i].type;
		krbp_t *val = &rs->column[i].val;
		if(KType_Is(UnboxType, type)) {
			KonohaValue sp[1]; sp[0].unboxValue = val[0].unboxValue;
			KClass_(type)->format(kctx, sp, 0, wb);
		} else {
			KLIB kObject_WriteToBuffer(kctx, val[0].asObject, false/*delim*/, wb, NULL, 0);
		}
	}
	KLIB KBuffer_printf(kctx, wb, "}");
}

static bool String_equal(KonohaContext *kctx, kString *str0, kString *str1)
{
	if(kString_size(str0) == kString_size(str1)) {
		return strncmp(kString_text(str0), kString_text(str1), kString_size(str0)) == 0;
	}
	return false;
}

static int ResultSet_FindColumn(KonohaContext *kctx, kResultSet *rs, kString *name)
{
	size_t i = 0;
	for(i = 0; i < rs->column_size; i++) {
		if(String_equal(kctx, name, rs->column[i].name)) {
			return i;
		}
	}
	return -1;
}

static void ResultSet_InitColumn(KonohaContext *kctx, kResultSet *rs, unsigned column_size)
{
	rs->column_size = column_size;
	if(rs->column_size > 0) {
		rs->column = KMalloc_UNTRACE(sizeof(KColumn) * rs->column_size);
	}
}

static void ResultSet_SetNull(KonohaContext *kctx, kResultSet *rs, unsigned Idx)
{
	rs->column[Idx].type = KType_void;
	rs->column[Idx].val.unboxValue = 0;
}

static void ResultSet_SetText(KonohaContext *kctx, kResultSet *rs, unsigned Idx, const char *text, size_t len)
{
	rs->column[Idx].type = KType_String;
	KFieldInit(rs, rs->column[Idx].val.asString, KLIB new_kString(kctx, GcUnsafe, text, len, 0));
}

static void ResultSet_SetInt(KonohaContext *kctx, kResultSet *rs, unsigned Idx, kint_t val)
{
	rs->column[Idx].type = KType_Int;
	rs->column[Idx].val.intValue = val;
}

static void ResultSet_SetFloat(KonohaContext *kctx, kResultSet *rs, unsigned Idx, kfloat_t val)
{
	rs->column[Idx].type = KType_float;
	rs->column[Idx].val.floatValue = val;
}

static kint_t parseInt(char *ptr, size_t len)
{
	char *endptr = ptr + len;
	return strtoll(ptr, &endptr, 10);
}

static kfloat_t parseFloat(char *ptr, size_t len)
{
	char *endptr = ptr + len;
	return strtod(ptr, &endptr);
}

/* ------------------------------------------------------------------------ */

#ifdef HAVE_MYSQL
#include "mysql_driver.h"
#endif

#ifdef HAVE_LIBSQLITE
#include "sqlite3_driver.h"
#endif

#ifdef HAVE_PSQL
#include "postgresql_driver.h"
#endif

static QueryDriver *FindQueryDriverByScheme(KonohaContext *kctx, const char *dburl)
{
	const char *res = strchr(dburl, ':');
	if(res == NULL) {
		return &NoQueryDriver;
	}
	int len = res - dburl;
#ifdef HAVE_MYSQL
	if(strncmp(dburl, "mysql", len) == 0) {
		return &MySQLDriver;
	}
#endif
#ifdef HAVE_LIBSQLITE
	if(strncmp(dburl, "sqlite3", len) == 0) {
		return &SQLLite3Driver;
	}
#endif
#ifdef HAVE_PSQL
	if(strncmp(dburl, "postgresql", len) == 0) {
		return &PostgreSQLDriver;
	}
#endif
	return &NoQueryDriver;
}

/* ------------------------------------------------------------------------ */
/* [Methods] */

//## Connection Connection.new(String dburl);
static KMETHOD Connection_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kConnection *con = (kConnection *) sfp[0].asObject;
	KMakeTrace(trace, sfp);
	const char  *dburl  = kString_text(sfp[1].asString);
	QueryDriver *driver = FindQueryDriverByScheme(kctx, dburl);
	DBHandler *db = driver->qopen(kctx, dburl, trace);
	if(db != NULL) {
		con->db     = db;
		con->driver = driver;
	}
	KReturn(con);
}

//## ResultSet Connection.query(String query);
static KMETHOD Connection_query(KonohaContext *kctx, KonohaStack *sfp)
{
	INIT_GCSTACK();
	KMakeTrace(trace, sfp);
	kConnection *conn = (kConnection *)sfp[0].asObject;
	const char *query = kString_text(sfp[1].asString);
	kResultSet *rs = (kResultSet *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), (uintptr_t)conn);
	KCursor *qcur = conn->driver->qexec(kctx, conn->db, query, rs, trace);
	if(qcur != NULL) {
		rs->qcur   = qcur;
		rs->driver = conn->driver;
	}
	KReturnWith(rs, RESET_GCSTACK());
}

//## void Connection.close();
static KMETHOD Connection_close(KonohaContext *kctx, KonohaStack *sfp)
{
	kConnection *conn = (kConnection *)sfp[0].asObject;
	conn->driver->qclose(conn->db);
	conn->db = NULL;
}

#ifdef HAVE_MYSQL

//## int Connection.getInsertId();
KMETHOD Connection_getInsertId(KonohaContext *kctx, KonohaStack *sfp)
{
	kConnection *conn = (kConnection *)sfp[0].asObject;
	kint_t ret = 0;
	if(conn->driver == &MySQLDriver) {
		ret = (kint_t) mysql_insert_id((MYSQL *)conn->db);
	}
	else {
		// [TODO] throw exeption when another dbms use this method.
	}
	KReturnUnboxValue(ret);
}

#endif


//## Boolean ResultSet.next();

static KMETHOD ResultSet_next(KonohaContext *kctx, KonohaStack *sfp)
{
	kResultSet *rs = (kResultSet *) sfp[0].asObject;
	kbool_t ret = false;
	DBG_ASSERT(rs->qcur != NULL);
	KMakeTrace(trace, sfp);
	if(rs->driver->qcurnext(kctx, rs->qcur, rs, trace)) {
		rs->rowidx++;
		ret = true;
	}
	else {
		rs->driver->qcurfree(rs->qcur);
		rs->qcur = NULL;
	}
	KReturnUnboxValue(ret);
}

//## int ResultSet.getSize();
static KMETHOD ResultSet_getSize(KonohaContext *kctx, KonohaStack *sfp)
{
	kResultSet *rs = (kResultSet *) sfp[0].asObject;
	KReturnUnboxValue(rs->column_size);
}

//## String ResultSet.getName(Int n);
static KMETHOD ResultSet_getName(KonohaContext *kctx, KonohaStack *sfp)
{
	kResultSet *rs = (kResultSet *)sfp[0].asObject;
	size_t n = (size_t)sfp[1].intValue;
	KCheckIndex(n, rs->column_size);
	KReturn(rs->column[n].name);
}

/* ------------------------------------------------------------------------ */
/* [getter/setter] */

//## int ResultSet.getInt(String n);
static KMETHOD ResultSet_getInt(KonohaContext *kctx, KonohaStack *sfp)
{
	kResultSet *rs = (kResultSet *)sfp[0].asObject;
	int idx = ResultSet_FindColumn(kctx, rs, sfp[1].asString);
	kint_t res = 0;
	if(idx >= 0) {
		ktypeattr_t type = rs->column[idx].type;
		if(type == KType_Int) {
			res = rs->column[idx].val.intValue;
		} else if(KDefinedKonohaCommonModule() && type == KType_float) {
			res = (kint_t) rs->column[idx].val.floatValue;
		}
	}
	KReturnUnboxValue(res);
}

//## boolean ResultSet.getBoolean(String n);
static KMETHOD ResultSet_getBoolean(KonohaContext *kctx, KonohaStack *sfp)
{
	kResultSet *rs = (kResultSet *)sfp[0].asObject;
	int idx = ResultSet_FindColumn(kctx, rs, sfp[1].asString);
	kbool_t res = 0;
	if(idx >= 0) {
		ktypeattr_t type = rs->column[idx].type;
		if(type == KType_Boolean) {
			res = rs->column[idx].val.boolValue;
		} else if(type == KType_Int) {
			res = (kbool_t) rs->column[idx].val.intValue;
		}
	}
	KReturnUnboxValue(res);
}

//## float ResultSet.getFloat(String name);
static KMETHOD ResultSet_getFloat(KonohaContext *kctx, KonohaStack *sfp)
{
	kResultSet *rs = (kResultSet *)sfp[0].asObject;
	int idx = ResultSet_FindColumn(kctx, rs, sfp[1].asString);
	kfloat_t res = 0.0;
	if(idx >= 0) {
		ktypeattr_t type = rs->column[idx].type;
		if(type == KType_Int) {
			res = (kfloat_t) rs->column[idx].val.intValue;
		} else if(KDefinedKonohaCommonModule() && type == KType_float) {
			res = rs->column[idx].val.floatValue;
		}
	}
	KReturnFloatValue(res);
}

//## String ResultSet.getString(String n);
static KMETHOD ResultSet_getString(KonohaContext *kctx, KonohaStack *sfp)
{
	kResultSet *rs = (kResultSet *)sfp[0].asObject;
	kString *res = TS_EMPTY;
	int idx = ResultSet_FindColumn(kctx, rs, sfp[1].asString);
	if(idx >= 0) {
		ktypeattr_t type = rs->column[idx].type;
		krbp_t *val = &rs->column[idx].val;
		if(type == KType_String) {
			res = val[0].asString;
		} else if(type == KType_Int) {
			KBuffer wb;
			KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
			KLIB KBuffer_printf(kctx, &wb, KFLOAT_FMT, val[0].floatValue);
			const char *text = KLIB KBuffer_text(kctx, &wb, 0);
			res = KLIB new_kString(kctx, OnStack, text, KBuffer_bytesize(&wb), 0);
			KLIB KBuffer_Free(&wb);
		} else if(KDefinedKonohaCommonModule() && type == KType_float) {
			KBuffer wb;
			KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
			KLIB KBuffer_printf(kctx, &wb, KFLOAT_FMT, val[0].floatValue);
			KLIB KBuffer_Free(&wb);
			const char *text = KLIB KBuffer_text(kctx, &wb, 0);
			res = KLIB new_kString(kctx, OnStack, text, KBuffer_bytesize(&wb), 0);
			KLIB KBuffer_Free(&wb);
		}
	}
	KReturn(res);
}

//## @SmartReturn Object ResultSet.get(String n);
static KMETHOD ResultSet_get(KonohaContext *kctx, KonohaStack *sfp)
{
	KClass *retClass = KGetReturnType(sfp);
	if(retClass->typeId == KType_Int) {
		ResultSet_getInt(kctx, sfp);
	} else if(retClass->typeId == KType_String) {
		ResultSet_getString(kctx, sfp);
	} else if(KDefinedKonohaCommonModule() && retClass->typeId == KType_float) {
		ResultSet_getFloat(kctx, sfp);
	} else {
		kObject *returnValue = KLIB Knull(kctx, retClass);
		sfp[K_RTNIDX].unboxValue = kObject_Unbox(returnValue);
		KReturn(returnValue);
	}
}

static kbool_t sql_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequirePackage("Type.Float", trace);

	static KDEFINE_CLASS ConnectionDef = {
		STRUCTNAME(Connection),
		.cflag = KClassFlag_Final,
		.init = kConnection_Init,
		.free = kConnection_Free,
	};

	static KDEFINE_CLASS ResultSetDef = {
		STRUCTNAME(ResultSet),
		.cflag = KClassFlag_Final,
		.init = kResultSet_Init,
		.free = kResultSet_Free,
		.reftrace = kResultSet_Reftrace,
		.format = kResultSet_format,
	};

	KClass *cConnection = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &ConnectionDef, trace);
	KClass *cResultSet = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &ResultSetDef, trace);

	KDEFINE_METHOD MethodData[] = {
		/* Connection method */
		_Public, _F(Connection_new), KType_Connection, KType_Connection, KMethodName_("new"), 1, KType_String, KFieldName_("dbname"),
		_Public, _F(Connection_close), KType_void, KType_Connection, KMethodName_("close"), 0,
		_Public, _F(Connection_query), KType_ResultSet, KType_Connection, KMethodName_("query"), 1, KType_String, KFieldName_("query"),
#ifdef HAVE_MYSQL
		_Public, _F(Connection_getInsertId), KType_Int, KType_Connection, KMethodName_("getInsertId"), 0,
#endif

		/* ResultSet method */
		_Public, _F(ResultSet_getInt), KType_Int, KType_ResultSet, KMethodName_("getInt"), 1, KType_String, KFieldName_("query"),
		_Public, _F(ResultSet_getBoolean), KType_Boolean, KType_ResultSet, KMethodName_("getBoolean"), 1, KType_String, KFieldName_("query"),
		_Public, _F(ResultSet_getFloat), KType_float, KType_ResultSet, KMethodName_("getFloat"), 1, KType_String, KFieldName_("query"),
		_Public, _F(ResultSet_getString), KType_String, KType_ResultSet, KMethodName_("getString"), 1, KType_String, KFieldName_("query"),
		_Public|kMethod_SmartReturn, _F(ResultSet_get), KType_Object, KType_ResultSet, KMethodName_("get"), 1, KType_String, KFieldName_("query"),
		_Public, _F(ResultSet_next), KType_Boolean, KType_ResultSet, KMethodName_("next"), 0,
		_Public, _F(ResultSet_getSize), KType_Int, KType_ResultSet, KMethodName_("getSize"), 0,
		_Public, _F(ResultSet_getName), KType_String, KType_ResultSet, KMethodName_("getName"), 1, KType_Int, KFieldName_("idx"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

static kbool_t sql_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *Sql_Init(void)
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
