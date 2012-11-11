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

#ifndef SQL_COMMON_H_
#define SQL_COMMON_H_

#define MSC_VAR
#include<stdio.h>
#include<minikonoha/minikonoha.h>
#include<minikonoha/sugar.h>

/* ------------------------------------------------------------------------ */
/* [bytes API] */

kint_t parseInt(char* utext, size_t len)
{
	kint_t n = 0, prev = 0, base = 10;
	size_t i = 0;
	if(len > 1) {
		if(utext[0] == '0') {
			if(utext[1] == 'x') {
				base = 16; i = 2;
			}
			else if(utext[1] == 'b') {
				base = 2;  i = 2;
			}
			else {
				base = 8;  i = 1;
			}
		}
		else if(utext[0] == '-') {
			base = 10; i = 1;
		}
	}
	for (;i < len; i++) {
		int c = utext[i];
		if('0' <= c && c <= '9') {
			prev = n;
			n = n * base + (c - '0');
		}
		else if(base == 16) {
			if('A' <= c && c <= 'F') {
				prev = n;
				n = n * 16 + (10 + c - 'A');
			}
			else if('a' <= c && c <= 'f') {
				prev = n;
				n = n * 16 + (10 + c - 'a');
			}
			else {
				break;
			}
		}
		else if(c == '_') {
			continue;
		}
		else {
			break;
		}
		//if(!(n >= prev)) {
		//	return 0;
		//}
	}
	if(utext[0] == '-') n = -((kint_t)n);
	return n;
}

kfloat_t parseFloat(char* utext, size_t len)
{
	kfloat_t ret;
#if defined(K_USING_NOFLOAT)
	{
		kint_t v = parseInt(utext, len);
		ret = (kfloat_t)v;
	}
#else
	ret = strtod(utext, NULL);
#endif
	return ret;
}

static size_t _NumberOfDigit(kint_t value, int base)
{
	size_t ret = 1;
	if(value < 0) {
		ret++;
		value = -1 * value;
	}
	while(value >= base) {
		value = value / base;
		ret++;
	}
	return ret;
}

//static void bytesArray_expand(KonohaContext *kctx, kBytes* b, size_t minsize)
//{
//	size_t oldsize = b->bytesize, newsize = oldsize * 2;
//	if(minsize > newsize) newsize = minsize;
//
//	char *newbody = (char *)KMalloc_UNTRACE(newsize);
//	if(oldsize < newsize) {
//		memcpy(newbody, b->buf, oldsize);
//		bzero(newbody + oldsize, newsize - oldsize);
//	}
//	else {
//		memcpy(newbody, b->buf, newsize);
//	}
//	KFree(b->buf, oldsize);
//	((struct kBytesVar *)b)->buf = newbody;
//	((struct kBytesVar *)b)->bytesize = newsize;
//}

/* ------------------------------------------------------------------------ */

void _ResultSet_setInt(KonohaContext *kctx, kResultSet *rs, size_t n, kint_t value)
{
	DBG_ASSERT(n < rs->column_size);
	KGrowingBuffer wb;
	size_t len = _NumberOfDigit(value, 10);
	KLIB Kwb_init(&(((kResultSet *)rs)->databuf), &wb);
	rs->column[n].ctype = kResultSet_CTYPE__integer;
	rs->column[n].start = strlen(rs->databuf.bytebuf);
	rs->column[n].len = len;
	char buf[len];
	memset(&buf, '\0', len);
	sprintf(buf, "%ld", (value));
	KLIB Kwb_write(kctx, &wb, buf, len);
}

void _ResultSet_setFloat(KonohaContext *kctx, kResultSet *rs, size_t n, kfloat_t value)
{
	KNH_ASSERT(n < rs->column_size);
	KGrowingBuffer wb;
	size_t len = 12; // sizeof KFLOAT_FMT
	KLIB Kwb_init(&(((kResultSet *)rs)->databuf), &wb);
	rs->column[n].ctype = kResultSet_CTYPE__float;
	rs->column[n].start = strlen(rs->databuf.bytebuf);
	rs->column[n].len = len;
	char buf[len];
	memset(&buf, '\0', len);
	sprintf(buf, KFLOAT_FMT, (value));
	KLIB Kwb_write(kctx, &wb, buf, len);
}

void _ResultSet_setText(KonohaContext *kctx, kResultSet *rs, size_t n, char* text, size_t len)
{
	DBG_ASSERT(n < rs->column_size);
	KGrowingBuffer wb;
	KLIB Kwb_init(&(((kResultSet *)rs)->databuf), &wb);
	rs->column[n].ctype = kResultSet_CTYPE__text;
	rs->column[n].start = strlen(rs->databuf.bytebuf);
	rs->column[n].len = len;
	KLIB Kwb_write(kctx, &wb, (const char *)(text), len);
}

//void _ResultSet_setBlob(KonohaContext *kctx, kResultSet *o, size_t n, kbytes_t t)
//{
//	KNH_ASSERT(n < o->column_size);
//	KGrowingBuffer wb;
//	KLIB Kwb_init(&(((kResultSet *)rs)->databuf), &wb);
//	o->column[n].ctype = kResultSet_CTYPE__bytes;
//	o->column[n].start = strlen(o->databuf.bytebuf);
//	o->column[n].len = t.len;
//	KLIB Kwb_write(kctx, &wb, (const char *)(t.text), t.len);
//}

void _ResultSet_setNULL(KonohaContext *kctx, kResultSet *o, size_t n)
{
	KNH_ASSERT(n < o->column_size);
	o->column[n].ctype = kResultSet_CTYPE__null;
	o->column[n].start = S_size(&(o->databuf));
	o->column[n].len = 0;
}

#if defined(_MSC_VER)
void _ResultSet_initColumn(KonohaContext *kctx, kResultSet *o, size_t column_size)
{
	size_t i;
	kResultSet* rs = (kResultSet *)o;
	//if(rs->column_size != 0) {
	//	//for(i = 0; i < o->column_size; i++) {
	//	//	KNH_FINALv(kctx, o->column[i].name); // free o->column[i].name
	//	//}
	//	KFree(rs->column, sizeof(DBschema) * (rs)->column_size);
	//	rs->column = NULL;
	//	if(rs->qcur != NULL) {
	//		rs->qcurfree(o->qcur);
	//		rs->qcur = NULL;
	//	}
	//}
	rs->column_size = column_size;
	if(column_size > 0) {
		rs->column = (struct DBschema *)KMalloc_UNTRACE(sizeof(DBschema) * column_size);
		for(i = 0; i < column_size; i++) {
			rs->column[i].type = TY_String;
			/* TS_EMPTY would be tenure, so ignore WriteBarrier. */
			// [TODO] ask ide.
			DBG_ASSERT(true);
			KFieldInit(o, o->column[i].name, TS_EMPTY);
			//KINITv_AND_WRITE_BARRIER(o->column[i].name, TS_EMPTY, 0);
			rs->column[i].start = 0;
			rs->column[i].len = 0;
		}
	}
	rs->rowidx = 0;
}

#endif /* _MSC_VER */

#endif /* SQL_COMMON_H_ */
