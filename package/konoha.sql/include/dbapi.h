/****************************************************************************
 * KONOHA COPYRIGHT, LICENSE NOTICE, AND DISCRIMER
 *
 * Copyright (c) 2006-2011, Kimio Kuramitsu <kimio at ynu.ac.jp>
 *           (c) 2008-      Konoha Team konohaken@googlegroups.com
 * All rights reserved.
 *
 * You may choose one of the following two licenses when you use konoha.
 * If you want to use the latter license, please contact us.
 *
 * (1) GNU General Public License 3.0 (with K_UNDER_GPL)
 * (2) Konoha Non-Disclosure License 1.0
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

/* ************************************************************************ */

//#include <mysql.h>

/* ************************************************************************ */

/* ------------------------------------------------------------------------ */
//## method Boolean ResultSet.next();

KMETHOD ResultSet_next(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(knh_ResultSet_next(kctx, (kResultSet*)sfp[0].asObject));
}

/* ------------------------------------------------------------------------ */

static int knh_ResultSet_indexof_(KonohaContext *kctx, KonohaStack *sfp)
{
	kResultSet *o = (kResultSet*)sfp[0].asObject;
	if(IS_Int(sfp[1].asObject)) {
		size_t n = (size_t)sfp[1].intValue;
		if(!(n < o->column_size)) {
			//THROW_OutOfRange(ctx, sfp, sfp[1].ivalue, DP(o)->column_size);
			return -1;
		}
		return n;
	}
	else if(IS_String(sfp[1].asObject)) {
		int loc = knh_ResultSet_findColumn(kctx, o, S_text(sfp[1].asString));
		if(loc == -1) {
			//TODO();
			//KNH_STUPID(ctx, o, STUPID_NOTFOUND);
		}
		return loc;
	}
	//TODO();
	//KNH_STUPID(ctx, o, STUPID_NOTFOUND);
	return -1;
}

///* ------------------------------------------------------------------------ */
////## method Int ResultSet.getInt(dynamic n);
//
//KMETHOD ResultSet_getInt(KonohaContext *kctx, KonohaStack *sfp)
//{
////	int n = knh_ResultSet_indexof_(ctx, sfp);
////	kint_t res = 0;
////	if(n >= 0) {
////		kResultSet *o = (kResultSet*)sfp[0].asObject;
////		const char *p = BA_totext(DP(o)->databuf) + DP(o)->column[n].start;
////		switch(DP(o)->column[n].ctype) {
////		case knh_ResultSet_CTYPE__integer :
////			res = *((kint_t*)p); break;
////		case knh_ResultSet_CTYPE__float :
////			res = (kint_t)(*((kfloat_t*)p)); break;
////		case knh_ResultSet_CTYPE__null :
////		default:
////			KNH_SETv(ctx, sfp[_rix].asObject, KNH_NULVAL(CLASS_Int));
////		}
////	}
////	KReturnUnboxValue(res);
//}
//
///* ------------------------------------------------------------------------ */
////## method Float ResultSet.getFloat(dynamic n);
//
//KMETHOD ResultSet_getFloat(KonohaContext *kctx, KonohaStack *sfp)
//{
////	int n = knh_ResultSet_indexof_(ctx, sfp);
////	kfloat_t res = KFLOAT_ZERO;
////	if(n >= 0) {
////		kResultSet *o = (kResultSet*)sfp[0].asObject;
////		const char *p = BA_totext(DP(o)->databuf) + DP(o)->column[n].start;
////		switch(DP(o)->column[n].ctype) {
////		case knh_ResultSet_CTYPE__integer :
////			res = (kfloat_t)(*((kint_t*)p)); break;
////		case knh_ResultSet_CTYPE__float :
////			res = (*((kfloat_t*)p)); break;
////		case knh_ResultSet_CTYPE__null :
////		default:
////			KNH_SETv(ctx, sfp[_rix].asObject, KNH_NULVAL(CLASS_Float));
////		}
////	}
////	KReturnFloatValue(res);
//}

/* ------------------------------------------------------------------------ */
//## method String ResultSet.getString(dynamic n);

KMETHOD ResultSet_getString(KonohaContext *kctx, KonohaStack *sfp)
{
	int n = knh_ResultSet_indexof_(kctx, sfp);
	kResultSet* o = (kResultSet*)sfp[0].asObject;
	DBG_ASSERT(n < o->column_size);
	const char *p = o->databuf->text + o->column[n].start;
	switch(o->column[n].ctype) {
	case knh_ResultSet_CTYPE__integer :
		break;
		//return new_String__int(kctx, (kint_t)(*((kint_t*)p)));
	case knh_ResultSet_CTYPE__float :
		break;
		//return new_String__int(kctx, (kint_t)(*((kint_t*)p)));
		//return new_String__float(kctx, (kfloat_t)(*((kfloat_t*)p)));
	case knh_ResultSet_CTYPE__text : {
		kbytes_t t = {o->column[n].len, {p}};
		//kbytes_t t = {{p}, o->column[n].len};
		//break;
		KReturn(KLIB new_kString(kctx, t.text, t.len, 0));
		}
	case knh_ResultSet_CTYPE__null :
		break;
	}
	KReturn(KLIB new_kString(kctx, "", 0, 0));
//	Object *v = KNH_NULL;
//	if(n >= 0) {
//		v = UPCAST(knh_ResultSet_getString(ctx, (kResultSet*)sfp[0].asObject, n));
//	}
//	KReturn(v);
}

/* ------------------------------------------------------------------------ */
//## method dynamic ResultSet.get(dynamic n);

//KMETHOD ResultSet_get(KonohaContext *kctx, KonohaStack *sfp)
//{
//	
//	int n = knh_ResultSet_indexof_(ctx, sfp);
//	Object *v = KNH_NULL;
//	if(n >= 0) {
//		kResultSet *o = (kResultSet*)sfp[0].asObject;
//		const char *p = BA_totext(DP(o)->databuf) + DP(o)->column[n].start;
//		switch(DP(o)->column[n].ctype) {
//		case knh_ResultSet_CTYPE__integer : {
//			kint_t val;
//			knh_memcpy(&val, p, sizeof(kint_t));
//			KNH_SETv(ctx, sfp[_rix].asObject, new_Int_(ctx, CLASS_Int, val));
//			KReturnUnboxValue((*((kint_t*)p)));
//		}
//		case knh_ResultSet_CTYPE__float : {
//			kfloat_t val;
//			knh_memcpy(&val, p, sizeof(kfloat_t));
//			KNH_SETv(ctx, sfp[_rix].asObject, new_Float_(ctx, CLASS_Float, val));
//			KReturnFloatValue((*((kfloat_t*)p)));
//		}
//		case knh_ResultSet_CTYPE__text : {
//			kbytes_t t = {{BA_totext(DP(o)->databuf) + DP(o)->column[n].start}, DP(o)->column[n].len};
//			v = UPCAST(new_S(t.text, t.len));
//			break;
//		}
//		case knh_ResultSet_CTYPE__bytes :
//			{
//				kBytes *ba = new_Bytes(ctx, BA_totext(DP(o)->databuf) + DP(o)->column[n].start, DP(o)->column[n].len);
//				kbytes_t t = {{BA_totext(DP(o)->databuf) + DP(o)->column[n].start}, DP(o)->column[n].len};
//				knh_Bytes_write(ctx, ba, t);
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
/* ------------------------------------------------------------------------ */
//## method void ResultSet.%dump(OutputStream w, String m);
//
//static void knh_ResultSet__dump(KonohaContext *kctx, kResultSet *o, kOutputStream *w, kString *m)
//{
//	knh_putc(ctx, w, '{');
//	size_t n;
//	for(n = 0; n < DP(o)->column_size; n++) {
//		if(n > 0) {
//			knh_write_delim(ctx,w);
//		}
//		knh_write(ctx, w, S_tobytes(DP(o)->column[n].name));
//		knh_printf(ctx, w, "(%d): ", n);
//		char *p = BA_totext(DP(o)->databuf) + DP(o)->column[n].start;
//		switch(DP(o)->column[n].ctype) {
//			case knh_ResultSet_CTYPE__null :
//				knh_write(ctx, w, STEXT("null"));
//				break;
//			case knh_ResultSet_CTYPE__integer :
//				knh_write_ifmt(ctx, w, KINT_FMT, (*((kint_t*)p)));
//				break;
//			case knh_ResultSet_CTYPE__float :
//				knh_write_ffmt(ctx, w, KFLOAT_FMT, (*((kfloat_t*)p)));
//				break;
//			case knh_ResultSet_CTYPE__text :
//				knh_write(ctx, w, B2(p, DP(o)->column[n].len));
//				break;
//			case knh_ResultSet_CTYPE__bytes :
//				knh_printf(ctx, w, "BLOB(%dbytes)", DP(o)->column[n].len);
//				break;
//		}
//	}
//	knh_putc(ctx, w, '}');
//}
