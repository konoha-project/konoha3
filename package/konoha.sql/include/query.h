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
// **************************************************************************

/* ======================================================================== */
/* [private function] */
/* ------------------------------------------------------------------------ */
/* K_DSPI_QUERY */

static void NOP_qfree(kqcur_t *qcur)
{
}

static void *NOP_qopen(KonohaContext *kctx, const char* url)
{
	return NULL;
}
static kqcur_t *NOP_query(KonohaContext *kctx, void *hdr, const char* sql, struct _kResultSet *rs)
{
	return NULL;
}
static void NOP_qclose(kconn_t *hdr)
{
}
static int NOP_qnext(KonohaContext *kctx, kqcur_t *qcur, kResultSet *rs)
{
	return 0;  /* NOMORE */
}

static kQueryDSPI_t NOP_DSPI = {
	K_DSPI_QUERY, "NOP",
	NOP_qopen, NOP_query, NOP_qclose, NOP_qnext, NOP_qfree
};

/* ------------------------------------------------------------------------ */
/* [bytes API] */

static inline kbytes_t new_bytes(char *c_buf)
{
	DBG_ASSERT(c_buf != NULL);
	kbytes_t t;
	t.utext = (unsigned char*)c_buf;
	t.len = strlen(t.text);
	return t;
}

#define B(c)      new_bytes((char*)c)

int knh_bytes_parseint(kbytes_t t, kint_t *value)
{
	kuint_t n = 0, prev = 0, base = 10;
	size_t i = 0;
	if(t.len > 1) {
		if(t.utext[0] == '0') {
			if(t.utext[1] == 'x') {
				base = 16; i = 2;
			}
			else if(t.utext[1] == 'b') {
				base = 2;  i = 2;
			}
			else {
				base = 8;  i = 1;
			}
		} else if(t.utext[0] == '-') {
			base = 10; i = 1;
		}
	}
	for(;i < t.len; i++) {
		int c = t.utext[i];
		if('0' <= c && c <= '9') {
			prev = n;
			n = n * base + (c - '0');
		} else if(base == 16) {
			if('A' <= c && c <= 'F') {
				prev = n;
				n = n * 16 + (10 + c - 'A');
			} else if('a' <= c && c <= 'f') {
				prev = n;
				n = n * 16 + (10 + c - 'a');
			} else {
				break;
			}
		} else if(c == '_') {
			continue;
		} else {
			break;
		}
		if(!(n >= prev)) {
			*value = 0;
			return 0;
		}
	}
	if(t.utext[0] == '-') n = -((kint_t)n);
	*value = n;
	return 1;
}

int knh_bytes_parsefloat(kbytes_t t, kfloat_t *value)
{
#if defined(K_USING_NOFLOAT)
	{
		kint_t v = 0;
		knh_bytes_parseint(t, &v);
		*value = (kfloat_t)v;
	}
#else
	*value = strtod(t.text, NULL);
#endif
	return 1;
}
/* ------------------------------------------------------------------------ */
/* [ResultSet] */

kbool_t knh_ResultSet_next(KonohaContext *kctx, kResultSet *o)
{
	struct _kResultSet* rs = (struct _kResultSet*)o;
	if(o->qcur != NULL) {
		if(rs->connection->dspi->qcurnext(kctx, rs->qcur, rs)) {
			rs->count += 1;
			return 1;
		}
		else {
			//kbytes_t t = {0, {""}};
			rs->qcurfree(o->qcur);
			rs->qcur = NULL;
			//o->qcurfree = knh_getQueryDSPI(ctx, t)->qcurfree;
		}
	}
	return 0;
}

void knh_ResultSet_close(KonohaContext *kctx, kResultSet *o)
{
//	if(DP(o)->qcur != NULL) {
//		kbytes_t t = {{""}, 0};
//		DP(o)->qcurfree(DP(o)->qcur);
//		DP(o)->qcur = NULL;
//		DP(o)->qcurfree = knh_getQueryDSPI(ctx, t)->qcurfree;
//	}
//	KNH_SETv(ctx, DP(o)->conn, KNH_NULL);
}

KMETHOD knh_ResultSet_initColumn(KonohaContext *kctx, kResultSet *o, size_t column_size)
{
	size_t i;
	struct _kResultSet* rs = (struct _kResultSet*)o;
	// [TODO]
	//if(o->column_size != 0) {
	//	for(i = 0; i < o->column_size; i++) {
	//		KNH_FINALv(kctx, o->column[i].name);
	//	}
	//	KFREE(kctx, o->column, sizeof(kDBschema) * DP(o)->column_size);
	//	o->column = NULL;
	//	if(o->qcur != NULL) {
	//		o->qcurfree(o->qcur);
	//		o->qcur = NULL;
	//	}
	//}
	rs->column_size = column_size;
	if(column_size > 0) {
		rs->column = (struct _kDBschema*)KMALLOC(sizeof(kDBschema) * column_size);
		for(i = 0; i < column_size; i++) {
			rs->column[i].type = TY_String;
			KINITv(o->column[i].name, TS_EMPTY);
			rs->column[i].start = 0;
			rs->column[i].len = 0;
		}
	}
	rs->count = 0;
}

/* ------------------------------------------------------------------------ */

int knh_ResultSet_findColumn(KonohaContext *kctx, kResultSet *o, const char* name)
{
	size_t i = 0;
	for(i = 0; i < o->column_size; i++) {
		if(strcasecmp(S_text(o->column[i].name), name) == 0) return i;
	}
	return -1;
}

/* ------------------------------------------------------------------------ */

KMETHOD ResultSet_setInt(KonohaContext *kctx, kResultSet *rs, size_t n, kint_t value)
{
	DBG_ASSERT(n < rs->column_size);
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	rs->column[n].ctype = knh_ResultSet_CTYPE__integer;
	rs->column[n].start = strlen(rs->databuf->text);
	rs->column[n].len = sizeof(kint_t);
	KLIB Kwb_write(kctx, &wb, (const char*)(&value), sizeof(kint_t));
	const char *KUtilsWriteBufferopChar = KLIB Kwb_top(kctx, &wb, 0);
	memcpy((void*)(rs->databuf->text + rs->column[n].start), KUtilsWriteBufferopChar, sizeof(kint_t)); // including NUL terminate by ensuredZeo
	KLIB Kwb_free(&wb);
}

/* ------------------------------------------------------------------------ */

KMETHOD ResultSet_setFloat(KonohaContext *kctx, kResultSet *rs, size_t n, kfloat_t value)
{
	KNH_ASSERT(n < rs->column_size);
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	//kbytes_t t = {sizeof(kfloat_t), {(const char*)(&value)}};
	rs->column[n].ctype = knh_ResultSet_CTYPE__float;
	rs->column[n].start = strlen(rs->databuf->text);
	rs->column[n].len = sizeof(kfloat_t);
	KLIB Kwb_write(kctx, &wb, (const char*)(&value), sizeof(kint_t));
	const char *KUtilsWriteBufferopChar = KLIB Kwb_top(kctx, &wb, 0);
	memcpy((void*)(rs->databuf->text + rs->column[n].start), KUtilsWriteBufferopChar, sizeof(kint_t)); // including NUL terminate by ensuredZeo
	KLIB Kwb_free(&wb);

}

/* ------------------------------------------------------------------------ */

KMETHOD ResultSet_setText(KonohaContext *kctx, kResultSet *o, size_t n, kbytes_t t)
{

	DBG_ASSERT(n < o->column_size);
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	o->column[n].ctype = knh_ResultSet_CTYPE__text;
	o->column[n].start = strlen(o->databuf->text);
	o->column[n].len = t.len;
	KLIB Kwb_write(kctx, &wb, (const char*)(t.text), t.len);
	const char *KUtilsWriteBuffer = KLIB Kwb_top(kctx, &wb, 0);
	memcpy((void*)(o->databuf->text + o->column[n].start), KUtilsWriteBuffer, t.len); // including NUL terminate by ensuredZeo
	KLIB Kwb_free(&wb);
}

///* ------------------------------------------------------------------------ */

KMETHOD ResultSet_setBlob(KonohaContext *kctx, kResultSet *o, size_t n, kbytes_t t)
{
	KNH_ASSERT(n < o->column_size);
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	o->column[n].ctype = knh_ResultSet_CTYPE__bytes;
	o->column[n].start = strlen(o->databuf->text);
	o->column[n].len = t.len;
	KLIB Kwb_write(kctx, &wb, (const char*)(t.text), t.len);
	const char *KUtilsWriteBuffer = KLIB Kwb_top(kctx, &wb, 0);
	memcpy((void*)(o->databuf->text + o->column[n].start), KUtilsWriteBuffer, t.len); // including NUL terminate by ensuredZeo
	KLIB Kwb_free(&wb);
}

/* ------------------------------------------------------------------------ */

KMETHOD ResultSet_setNULL(KonohaContext *kctx, kResultSet *o, size_t n)
{
	KNH_ASSERT(n < o->column_size);
	o->column[n].ctype = knh_ResultSet_CTYPE__null;
	o->column[n].start = S_size(o->databuf);
	o->column[n].len = 0;
}

///* ------------------------------------------------------------------------ */
//
//kint_t knh_ResultSet_getInt(KonohaContext *kctx, kResultSet *o, size_t n)
//{
//	KNH_ASSERT(n < DP(o)->column_size);
//	const char *p = BA_totext(DP(o)->databuf) + DP(o)->column[n].start;
//	switch(DP(o)->column[n].ctype) {
//	case knh_ResultSet_CTYPE__null :
//		return 0;
//	case knh_ResultSet_CTYPE__integer :
//		return (kint_t)(*((kint_t*)p));
//	case knh_ResultSet_CTYPE__float :
//		return (kint_t)(*((kfloat_t*)p));
//	case knh_ResultSet_CTYPE__bytes :
//		TODO();
////		return kbytes_toint(B2(p, DP(o)->column[n].len));
//	}
//	return 0;
//}
//
///* ------------------------------------------------------------------------ */
//
//kfloat_t knh_ResultSet_getFloat(KonohaContext *kctx, kResultSet *o, size_t n)
//{
//	KNH_ASSERT(n < DP(o)->column_size);
//	const char *p = BA_totext(DP(o)->databuf) + DP(o)->column[n].start;
//	switch(DP(o)->column[n].ctype) {
//	case knh_ResultSet_CTYPE__null :
//		return KFLOAT_ZERO;
//	case knh_ResultSet_CTYPE__integer :
//		return (kfloat_t)(*((kint_t*)p));
//	case knh_ResultSet_CTYPE__float :
//		return (kfloat_t)(*((kfloat_t*)p));
//	case knh_ResultSet_CTYPE__bytes :
//		TODO();
////		return kbytes_tofloat(B2(p, DP(o)->column[n].len));
//	}
//	return KFLOAT_ZERO;
//}
//
///* ------------------------------------------------------------------------ */

//static kString *new_String__int(KonohaContext *kctx, kint_t n)
//{
//	//CWB_t cwbbuf, *cwb = CWB_open(ctx, &cwbbuf);
//	//knh_write_ifmt(ctx, cwb->w, KINT_FMT, n);
//	//return CWB_newString0(ctx, cwb);
//	return NULL;
//}
//
//static kString *new_String__float(KonohaContext *kctx, kfloat_t n)
//{
//	//CWB_t cwbbuf, *cwb = CWB_open(ctx, &cwbbuf);
//	//knh_write_ffmt(ctx, cwb->w, KFLOAT_FMT, n);
//	//return CWB_newString0(ctx, cwb);
//	return NULL;
//}

///* ------------------------------------------------------------------------ */
//
//kString* _ResultSet_getString(KonohaContext *kctx, kResultSet *o, size_t n)
//{
//}

/* ------------------------------------------------------------------------ */

//static void Connection_reftrace(KonohaContext *kctx, void *po)
//{
//	kConnection *o = (kConnection *)po;
//	KNH_ADDREF(ctx, o->urn);
//	KNH_SIZEREF(ctx);
//	return NULL;
//}

void mysql_qcurfree(kqcur_t* qcur)
{
}

