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

#define USE_STRINGLIB 1

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/klib.h>
#include <pcre.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CT_StringArray0        CT_p0(kctx, CT_Array, TY_String)

/* ------------------------------------------------------------------------ */
/* regexp module */
//## @Immutable class RegExp Object;
//## flag RegExp Global      1 - is set * *;
//## flag RegExp IgnoreCase  2 - is set * *; // it is used only for RegExp_p
//## flag RegExp Multiline   3 - is set * *; // it is used only for RegExp_p

#define RegExp_isGlobal(o)        (TFLAG_is(uintptr_t,(o)->h.magicflag,kObject_Local1))
#define RegExp_setGlobal(o,b)     TFLAG_set(uintptr_t,(o)->h.magicflag,kObject_Local1,b)
#define RegExp_isIgnoreCase(o)    (TFLAG_is(uintptr_t,(o)->h.magicflag,kObject_Local2))
#define RegExp_setIgnoreCase(o,b) TFLAG_set(uintptr_t,(o)->h.magicflag,kObject_Local2,b)
#define RegExp_isMultiline(o)     (TFLAG_is(uintptr_t,(o)->h.magicflag,kObject_Local3))
#define RegExp_setMultiline(o,b)  TFLAG_set(uintptr_t,(o)->h.magicflag,kObject_Local3,b)

typedef void kregexp_t;

/* REGEXP_SPI */
#ifndef KREGEXP_MATCHSIZE
#define KREGEXP_MATCHSIZE    16
#endif

typedef struct {
	size_t       len;
	union {
		const char *text;
		const unsigned char *utext;
		char *buf;
		//kchar_t *ubuf;
	};
} kbytes_t;

static size_t knh_bytes_mlen(kbytes_t v)
{
#ifdef K_USING_UTF8
	size_t size = 0;
	const unsigned char *s = v.utext;
	const unsigned char *e = s + v.len;
	while (s < e) {
		size_t ulen = utf8len(s[0]);
		size ++;
		s += ulen;
	}
	return size;
#else
	return v.len;
#endif
}

static kbytes_t knh_bytes_mofflen(kbytes_t v, size_t moff, size_t mlen)
{
#ifdef K_USING_UTF8
	size_t i;
	const unsigned char *s = v.utext;
	const unsigned char *e = s + v.len;
	for(i = 0; i < moff; i++) {
		s += utf8len(s[0]);
	}
	v.buf = (char*)s;
	for(i = 0; i < mlen; i++) {
		s += utf8len(s[0]);
	}
	KNH_ASSERT(s <= e);
	v.len = (const char*)s - v.text;
	return v;
#else
	return knh_bytes_subbytes(m, moff, mlen); /* if K_ENCODING is not set */
#endif
}

#define _ALWAYS SPOL_POOL
#define _NEVER  SPOL_POOL
#define _ASCII  SPOL_ASCII
#define _UTF8   SPOL_UTF8
#define _SUB(s0) (S_isASCII(s0) ? _ASCII|_ALWAYS : _ALWAYS)
#define _SUBCHAR(s0) (S_isASCII(s0) ? _ASCII : 0)
#define _CHARSIZE(len) (len==1 ? _ASCII : _UTF8)

static kArray *kStringToCharArray(KonohaContext *kctx, kString *bs, int istrim, kint_t limit)
{
	kbytes_t base = {S_size(bs), {S_text(bs)}};
	size_t i, n = (S_isASCII(bs) ? base.len : knh_bytes_mlen(base));
	if(limit >= 0 && limit < n) {
		/* limit array size */
		n = limit;
	}
	kArray *a = (kArray*)KLIB new_kObject(kctx, CT_StringArray0, 0);
	kString *s = NULL;
	for(i = 0; i < n; i++) {
		if(istrim && isspace(base.utext[i])) continue;
		if(S_isASCII(bs)) {
			s = KLIB new_kString(kctx, base.text+i, 1, _ALWAYS|_ASCII);
		}
		else {
			kbytes_t sub = knh_bytes_mofflen(base, i, 1);
			s = KLIB new_kString(kctx, sub.text, sub.len, _ALWAYS|((sub.len == 1) ? _ASCII:_UTF8));
		}
		KLIB kArray_add(kctx, a, s);
	}
	return a;
}

static kString *kwb_newString(KonohaContext *kctx, KUtilsWriteBuffer *wb, int flg)
{
	return KLIB new_kString(kctx, KLIB Kwb_top(kctx, wb, flg), Kwb_bytesize(wb), SPOL_POOL);
}

typedef struct {
	pcre *re;
	const char *err;
	int erroffset;
} PCRE_regexp_t;

/* ------------------------------------------------------------------------ */
#define kregexpshare      ((kregexpshare_t*)kctx->modshare[MOD_REGEXP])
#define CT_RegExp         kregexpshare->cRegExp
#define TY_RegExp         kregexpshare->cRegExp->typeId

#define IS_RegExp(O)      ((O)->h.ct == CT_RegExp)

typedef struct {
	KonohaModule h;
	KonohaClass *cRegExp;
} kregexpshare_t;

typedef struct kRegExp kRegExp;
struct kRegExp {
	KonohaObjectHeader h;
	kregexp_t *reg;
	int eflags;      // regexp flag
	kString *pattern;
	size_t lastIndex; // last matched index
};

typedef struct {
	int rm_so;   /* start of match */
	int rm_eo;   /* end of match */
	kbytes_t rm_name;  /* {NULL, 0}, if not NAMED */
} kregmatch_t;

/* ------------------------------------------------------------------------ */

static kregexp_t* pcre_regmalloc(KonohaContext *kctx, kString* s)
{
	PCRE_regexp_t *preg = (PCRE_regexp_t*) KMALLOC(sizeof(PCRE_regexp_t));
	return (kregexp_t *) preg;
}

static void pcre_regfree(KonohaContext *kctx, kregexp_t *reg)
{
	PCRE_regexp_t *preg = (PCRE_regexp_t*)reg;
	pcre_free(preg->re);
	KFREE(preg, sizeof(PCRE_regexp_t));
}

static int pcre_nmatchsize(KonohaContext *kctx, kregexp_t *reg)
{
	PCRE_regexp_t *preg = (PCRE_regexp_t*)reg;
	int capsize = 0;
	if (pcre_fullinfo(preg->re, NULL, PCRE_INFO_CAPTURECOUNT, &capsize) != 0) {
		return KREGEXP_MATCHSIZE;
	}
	return capsize + 1;
}

static int pcre_parsecflags(KonohaContext *kctx, const char *option)
{
	int i, cflags = 0;
	int optlen = strlen(option);
	for (i = 0; i < optlen; i++) {
		switch(option[i]) {
		case 'i': // caseless
			cflags |= PCRE_CASELESS;
			break;
		case 'm': // multiline
			cflags |= PCRE_MULTILINE;
			break;
		case 's': // dotall
			cflags |= PCRE_DOTALL;
			break;
		case 'x': //extended
			cflags |= PCRE_EXTENDED;
			break;
		case 'u': //utf
			cflags |= PCRE_UTF8;
			break;
		default: break;
		}
	}
	return cflags;
}

static int pcre_parseeflags(KonohaContext *kctx, const char *option)
{
	int i, eflags = 0;
	int optlen = strlen(option);
	for (i = 0; i < optlen; i++) {
		switch(option[i]){
		default: break;
		}
	}
	return eflags;
}

//static size_t pcre_regerror(int res, kregexp_t *reg, char *ebuf, size_t ebufsize)
//{
//	PCRE_regexp_t *pcre = (PCRE_regexp_t*)reg;
//	snprintf(ebuf, ebufsize, "[%d]: %s", pcre->erroffset, pcre->err);
//	return (pcre->err != NULL) ? strlen(pcre->err) : 0;
//}

static int pcre_regcomp(KonohaContext *kctx, kregexp_t *reg, const char *pattern, int cflags)
{
	PCRE_regexp_t* preg = (PCRE_regexp_t*)reg;
	preg->re = pcre_compile(pattern, cflags, &preg->err, &preg->erroffset, NULL);
	return (preg->re != NULL) ? 0 : -1;
}

static int pcre_regexec(KonohaContext *kctx, kregexp_t *reg, const char *str, size_t nmatch, kregmatch_t p[], int eflags)
{
	PCRE_regexp_t *preg = (PCRE_regexp_t*)reg;
	int res, nm_count, nvector[nmatch*3];
	nvector[0] = 0;
	size_t idx, matched = nmatch;
	if (strlen(str) == 0) return -1;
	if ((res = pcre_exec(preg->re, NULL, str, strlen(str), 0, eflags, nvector, nmatch*3)) >= 0) {
		matched = (res > 0 && res < nmatch) ? res : nmatch;
		res = 0;
		for (idx = 0; idx < matched; idx++) {
			p[idx].rm_so = nvector[2*idx];
			p[idx].rm_eo = nvector[2*idx+1];
		}
		p[idx].rm_so = -1;
		nm_count = 0;
		pcre_fullinfo(preg->re, NULL, PCRE_INFO_NAMECOUNT, &nm_count);
		if (nm_count > 0) {
			unsigned char *nm_table;
			int nm_entry_size = 0;
			pcre_fullinfo(preg->re, NULL, PCRE_INFO_NAMETABLE, &nm_table);
			pcre_fullinfo(preg->re, NULL, PCRE_INFO_NAMEENTRYSIZE, &nm_entry_size);
			unsigned char *tbl_ptr = nm_table;
			for (idx = 0; idx < nm_count; idx++) {
				int n_idx = (tbl_ptr[0] << 8) | tbl_ptr[1];
				unsigned char *n_name = tbl_ptr + 2;
				p[n_idx].rm_name.utext = n_name;
				p[n_idx].rm_name.len = strlen((char*)n_name);
				tbl_ptr += nm_entry_size;
			}
		}
	}
	return res;
}

/* ------------------------------------------------------------------------ */
static void kregexpshare_setup(KonohaContext *kctx, struct KonohaModule *def, int newctx)
{
}

static void kregexpshare_reftrace(KonohaContext *kctx, struct KonohaModule *baseh)
{
}

static void kregexpshare_free(KonohaContext *kctx, struct KonohaModule *baseh)
{
	KFREE(baseh, sizeof(kregexpshare_t));
}

/* ------------------------------------------------------------------------ */
static void knh_RegExp_setOptions(kRegExp *re, const char *option)
{
	size_t i, optlen = strlen(option);
	for(i = 0; i < optlen; i++) {
		switch(option[i]) {
		case 'g':
			RegExp_setGlobal(re, 1);
			break;
		case 'i':
			RegExp_setIgnoreCase(re, 1);
			break;
		case 'm':
			RegExp_setMultiline(re, 1);
		default:
			break;
		}
	}
}

static size_t knh_regexp_matched(kregmatch_t* r, size_t maxmatch)
{
	size_t n = 0;
	for (; n < maxmatch && r[n].rm_so != -1; n++) {}
	return n;
}

static void WB_write_regexpfmt(KonohaContext *kctx, KUtilsWriteBuffer *wb, kbytes_t *fmt, const char *base, kregmatch_t *r, size_t matched)
{
	const char *ch = fmt->text;
	const char *eof = ch + fmt->len; // end of fmt
	char buf[1];
	for (; ch < eof; ch++) {
		if (*ch == '\\') {
			buf[0] = *ch;
			KLIB Kwb_write(kctx, wb, buf, 1);
			ch++;
		} else if (*ch == '$' && isdigit(ch[1])) {
			size_t grpidx = (size_t)ch[1] - '0'; // get head of grourp_index
			if (grpidx < matched) {
				ch++;
				while (isdigit(ch[1])) {
					size_t nidx = grpidx * 10 + (ch[1] - '0');
					if (nidx < matched) {
						grpidx = nidx;
						ch++;
						if (ch < eof) {
							continue;
						}
					}
				}
				kregmatch_t *rp = &r[grpidx];
				KLIB Kwb_write(kctx, wb, base + rp->rm_so, rp->rm_eo - rp->rm_so);
				continue; // skip putc
			}
		}
		buf[0] = *ch;
		KLIB Kwb_write(kctx, wb, buf, 1);
	}
}

static void RegExp_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kRegExp *re = (kRegExp*)o;
	re->reg = NULL;
	re->eflags = 0;
	re->pattern = KNULL(String);
	re->lastIndex = 0;
}

static void RegExp_free(KonohaContext *kctx, kObject *o)
{
	kRegExp *re = (kRegExp*)o;
	if(re->reg != NULL) {
		pcre_regfree(kctx, re->reg);
	}
}

static void RegExp_p(KonohaContext *kctx, KonohaValue *v, int pos, KUtilsWriteBuffer *wb)
{
	kRegExp *re = v[pos].re;
	KLIB Kwb_printf(kctx, wb, "/%s/%s%s%s", S_text(re->pattern),
			RegExp_isGlobal(re) ? "g" : "",
			RegExp_isIgnoreCase(re) ? "i" : "",
			RegExp_isMultiline(re) ? "m" : "");
}

static void RegExp_set(KonohaContext *kctx, kRegExp *re, kString *ptns, kString *opts)
{
	const char *ptn = S_text(ptns);
	const char *opt = S_text(opts);
	knh_RegExp_setOptions(re, opt);
	KSETv(re, re->pattern, ptns);
	re->reg = pcre_regmalloc(kctx, ptns);
	int cflags = pcre_parsecflags(kctx, opt);
	if(!S_isASCII(ptns)) {
		/* Add 'u' option when the pattern is multibyte string. */
		cflags |= PCRE_UTF8;
	}
	pcre_regcomp(kctx, re->reg, ptn, cflags);
	re->eflags = pcre_parseeflags(kctx, opt);
}

static kArray *RegExp_execute(KonohaContext *kctx, kRegExp *re, kString *s0)
{
	kArray *a = NULL;
	if(IS_NOTNULL(re) && S_size(re->pattern) > 0) {
		const char *str = S_text(s0);  // necessary
		const char *base = str;
		const char *eos = base + S_size(s0);
		size_t nmatch = pcre_nmatchsize(kctx, re->reg);
		kregmatch_t *p, pmatch[nmatch+1];
		int i, isGlobalOption = RegExp_isGlobal(re);
		a = (kArray*)KLIB new_kObject(kctx, CT_StringArray0, nmatch);
		BEGIN_LOCAL(lsfp, 1);
		KSETv_AND_WRITE_BARRIER(NULL, lsfp[0].asArray, a, GC_NO_WRITE_BARRIER);
		do {
			int res = pcre_regexec(kctx, re->reg, str, nmatch, pmatch, re->eflags);
			if(res != 0) {
				// FIXME
				//LOG_regex(kctx, sfp, res, re, str);
				break;
			}
			for(p = pmatch, i = 0; i < nmatch; p++, i++) {
				if (p->rm_so == -1) break;
				//DBG_P("[%d], rm_so=%d, rm_eo=%d", i, p->rm_so, p->rm_eo);
				kbytes_t sub = {((p->rm_eo) - (p->rm_so)), {str + (p->rm_so)}};
				KLIB kArray_add(kctx, a, KLIB new_kString(kctx, sub.text, sub.len, _SUB(s0)));
			}
			if(isGlobalOption) {
				size_t eo = pmatch[0].rm_eo; // shift matched pattern
				str += (eo > 0) ? eo : 1;
				if(!(str < eos)) isGlobalOption = 0; // stop iteration
			}
		} while(isGlobalOption);
		END_LOCAL();
	}
	else {
		a = (kArray*)KLIB new_kObject(kctx, CT_StringArray0, 0);
	}
	return a;
}

/* ------------------------------------------------------------------------ */
//## @Const method Boolean RegExp.getglobal();

static KMETHOD RegExp_getglobal(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(RegExp_isGlobal(sfp[0].re));
}

/* ------------------------------------------------------------------------ */
//## @Const method Boolean RegExp.getignoreCase();

static KMETHOD RegExp_getignoreCase(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(RegExp_isIgnoreCase(sfp[0].re));
}

/* ------------------------------------------------------------------------ */
//## @Const method Boolean RegExp.getmultiline();

static KMETHOD RegExp_getmultiline(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNb_(RegExp_isMultiline(sfp[0].re));
}

/* ------------------------------------------------------------------------ */
//## @Const method String RegExp.getsource();

static KMETHOD RegExp_getsource(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(sfp[0].re->pattern);
}

/* ------------------------------------------------------------------------ */
//## @Const method Int RegExp.getlastIndex();

static KMETHOD RegExp_getlastIndex(KonohaContext *kctx, KonohaStack *sfp)
{
	RETURNi_(sfp[0].re->lastIndex);
}

/* ------------------------------------------------------------------------ */
//## @Const method RegExp RegExp.new(String pattern);

static KMETHOD RegExp_new(KonohaContext *kctx, KonohaStack *sfp)
{
	RegExp_set(kctx, sfp[0].re, sfp[1].asString, KNULL(String));
	RETURN_(sfp[0].asObject);
}

/* ------------------------------------------------------------------------ */
//## @Const method RegExp RegExp.new(String pattern, String option);

static KMETHOD RegExp_newwithOption(KonohaContext *kctx, KonohaStack *sfp)
{
	RegExp_set(kctx, sfp[0].re, sfp[1].asString, sfp[2].asString);
	RETURN_(sfp[0].asObject);
}

/* ------------------------------------------------------------------------ */
//## @Const method Int String.search(RegExp searchvalue);

static KMETHOD String_search(KonohaContext *kctx, KonohaStack *sfp)
{
	kRegExp *re = sfp[1].re;
	intptr_t loc = -1;
	if(!IS_NULL(re) && S_size(re->pattern) > 0) {
		kregmatch_t pmatch[2]; // modified by @utrhira
		const char *str = S_text(sfp[0].asString);  // necessary
		int res = pcre_regexec(kctx, re->reg, str, 1, pmatch, re->eflags);
		if(res == 0) {
			loc = pmatch[0].rm_so;
			if (loc != -1 && !S_isASCII(sfp[0].asString)) {
				kbytes_t base = {loc, {str}};
				loc = knh_bytes_mlen(base);
			}
		}
		else {
			//TODO
			//LOG_regex(kctx, sfp, res, re, str);
		}
	}
	RETURNi_(loc);
}

/* ------------------------------------------------------------------------ */
//## @Const method String[] String.match(RegExp regexp);

static KMETHOD String_match(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kRegExp *re = sfp[1].re;
	RETURN_(RegExp_execute(kctx, re, s0));
}

/* ------------------------------------------------------------------------ */
//## @Const method String String.replace(RegExp searchvalue, String newvalue);

static KMETHOD String_replace(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kRegExp *re = sfp[1].re;
	kbytes_t fmt = {S_size(sfp[2].asString), {S_text(sfp[2].asString)}};
	kString *s = s0;
	if(IS_NOTNULL(re) && S_size(re->pattern) > 0) {
		KUtilsWriteBuffer wb;
		KLIB Kwb_init(&(kctx->stack->cwb), &wb);
		const char *str = S_text(s0);  // necessary
		const char *base = str;
		const char *eos = str + S_size(s0); // end of str
		kregmatch_t pmatch[KREGEXP_MATCHSIZE+1];
		int isGlobalOption = RegExp_isGlobal(re);
		do {
			if(str >= eos) break;
			int res = pcre_regexec(kctx, re->reg, str, KREGEXP_MATCHSIZE, pmatch, re->eflags);
			if(res != 0) {
				// TODO
				//LOG_regex(kctx, sfp, res, re, str);
				break;
			}
			size_t len = pmatch[0].rm_eo;
			if (pmatch[0].rm_so > 0) {
				KLIB Kwb_write(kctx, &wb, str, pmatch[0].rm_so);
			}
			size_t matched = knh_regexp_matched(pmatch, KREGEXP_MATCHSIZE);
			if (len > 0) {
				WB_write_regexpfmt(kctx, &wb, &fmt, base, pmatch, matched);
				str += len;
			} else {
				if (str == base) { // 0-length match at head of string
					WB_write_regexpfmt(kctx, &wb, &fmt, base, pmatch, matched);
				}
				break;
			}
		} while(isGlobalOption);
		KLIB Kwb_write(kctx, &wb, str, strlen(str)); // write out remaining string
		s = kwb_newString(kctx, &wb, 0); // close cwb
	}
	RETURN_(s);
}

/* ------------------------------------------------------------------------ */
//## @Const method String[] String.split(RegExp separator);

static KMETHOD String_split(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kRegExp *re = sfp[1].re;
	kArray *a = NULL;
	if (IS_NOTNULL(re) && S_size(re->pattern) > 0) {
		const char *str = S_text(s0);  // necessary
		const char *eos = str + S_size(s0);
		kregmatch_t pmatch[KREGEXP_MATCHSIZE+1];
		if (str < eos) {
			a = (kArray*)KLIB new_kObject(kctx, CT_StringArray0, 0);
			BEGIN_LOCAL(lsfp, 1);
			KSETv_AND_WRITE_BARRIER(NULL, lsfp[0].asArray, a, GC_NO_WRITE_BARRIER);
			while (str <= eos) {
				int res = pcre_regexec(kctx, re->reg, str, KREGEXP_MATCHSIZE, pmatch, re->eflags);
				if (res == 0) {
					size_t len = pmatch[0].rm_eo;
					if (len > 0) {
						kbytes_t sub = {pmatch[0].rm_so, {str}};
						KLIB kArray_add(kctx, a, KLIB new_kString(kctx, sub.text, sub.len, _SUB(s0)));
						str += len;
						continue;
					}
				}
				KLIB kArray_add(kctx, a, KLIB new_kString(kctx, str, strlen(str), SPOL_POOL)); // append remaining string to array
				break;
			}
			END_LOCAL();
		} else { // for 0-length patterh
			a = kStringToCharArray(kctx, KLIB new_kString(kctx, str, S_size(s0), SPOL_POOL), 0, -1/* no limit */);
		}
	}
	else {
		a = kStringToCharArray(kctx, s0, 0, -1/* no limit */);
	}
	RETURN_(a);
}

/* ------------------------------------------------------------------------ */
//## @Const method String[] String.split(RegExp separator, Int limit);

static KMETHOD String_splitwithSeparatorLimit(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kRegExp *re = sfp[1].re;
	kint_t limit = sfp[2].intValue;
	if(limit < 0) {
		/* ignore limit */
		limit = S_size(s0);
	}
	kArray *a = NULL;
	size_t asize = 0;
	if (IS_NOTNULL(re) && S_size(re->pattern) > 0) {
		const char *str = S_text(s0);  // necessary
		const char *eos = str + S_size(s0);
		kregmatch_t pmatch[KREGEXP_MATCHSIZE+1];
		if (str < eos) {
			a = (kArray*)KLIB new_kObject(kctx, CT_StringArray0, 0);
			BEGIN_LOCAL(lsfp, 1);
			KSETv_AND_WRITE_BARRIER(NULL, lsfp[0].asArray, a, GC_NO_WRITE_BARRIER);
			while (str <= eos && asize < limit) {
				int res = pcre_regexec(kctx, re->reg, str, KREGEXP_MATCHSIZE, pmatch, re->eflags);
				if (res == 0) {
					size_t len = pmatch[0].rm_eo;
					if (len > 0) {
						kbytes_t sub = {pmatch[0].rm_so, {str}};
						KLIB kArray_add(kctx, a, KLIB new_kString(kctx, sub.text, sub.len, _SUB(s0)));
						asize++;
						str += len;
						continue;
					}
				}
				KLIB kArray_add(kctx, a, KLIB new_kString(kctx, str, strlen(str), SPOL_POOL)); // append remaining string to array
				asize++;
				break;
			}
			END_LOCAL();
		} else { // for 0-length patterh
			a = kStringToCharArray(kctx, KLIB new_kString(kctx, str, S_size(s0), SPOL_POOL), 0, limit);
		}
	}
	else {
		a = kStringToCharArray(kctx, s0, 0, limit);
	}
	RETURN_(a);
}

/* ------------------------------------------------------------------------ */
//## @Const method String[] RegExp.exec(String str);

static KMETHOD RegExp_exec(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[1].asString;
	kRegExp *re = sfp[0].re;
	RETURN_(RegExp_execute(kctx, re, s0));
}

/* ------------------------------------------------------------------------ */
//## @Const method Boolean RegExp.test(String str);

static KMETHOD RegExp_test(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[1].asString;
	kRegExp *re = sfp[0].re;
	kbool_t matched = false;
	if(IS_NOTNULL(re) && S_size(re->pattern) > 0) {
		const char *str = S_text(s0);  // necessary
		const char *base = str;
		const char *eos = base + S_size(s0);
		size_t nmatch = pcre_nmatchsize(kctx, re->reg);
		kregmatch_t *p, pmatch[nmatch+1];
		int i, isGlobalOption = RegExp_isGlobal(re);
		do {
			int res = pcre_regexec(kctx, re->reg, str, nmatch, pmatch, re->eflags);
			if(res != 0) {
				// FIXME
				//LOG_regex(kctx, sfp, res, re, str);
				break;
			}
			for(p = pmatch, i = 0; i < nmatch; p++, i++) {
				if (p->rm_so == -1) break;
				//DBG_P("[%d], rm_so=%d, rm_eo=%d", i, p->rm_so, p->rm_eo);
				kbytes_t sub = {p->rm_eo, {str}};
				re->lastIndex = knh_bytes_mlen(sub);
				matched = true;
			}
			if(isGlobalOption) {
				size_t eo = pmatch[0].rm_eo; // shift matched pattern
				str += (eo > 0) ? eo : 1;
				if(!(str < eos)) isGlobalOption = 0; // stop iteration
			}
		} while(isGlobalOption);
	}
	else {
		re->lastIndex = 0;
	}
	RETURNb_(matched);
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _Im kMethod_Immutable
#define _F(F)   (intptr_t)(F)

static kbool_t regexp_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	kregexpshare_t *base = (kregexpshare_t*)KCALLOC(sizeof(kregexpshare_t), 1);
	base->h.name     = "regexp";
	base->h.setup    = kregexpshare_setup;
	base->h.reftrace = kregexpshare_reftrace;
	base->h.free     = kregexpshare_free;
	KLIB KonohaRuntime_setModule(kctx, MOD_REGEXP, &base->h, pline);

	KDEFINE_CLASS RegExpDef = {
		STRUCTNAME(RegExp),
		.cflag = 0,
		.init = RegExp_init,
		.free = RegExp_free,
		.p    = RegExp_p,
	};
	base->cRegExp = KLIB kNameSpace_defineClass(kctx, ns, NULL, &RegExpDef, pline);

	ktype_t TY_StringArray0 = CT_StringArray0->typeId;
	KDEFINE_METHOD MethodData[] = {
		/*JS*/_Public|_Const|_Im, _F(RegExp_getglobal), TY_boolean, TY_RegExp, MN_("getglobal"), 0,
		/*JS*/_Public|_Const|_Im, _F(RegExp_getignoreCase), TY_boolean, TY_RegExp, MN_("getignoreCase"), 0,
		/*JS*/_Public|_Const|_Im, _F(RegExp_getmultiline), TY_boolean, TY_RegExp, MN_("getmultiline"), 0,
		/*JS*/_Public|_Const|_Im, _F(RegExp_getsource), TY_String, TY_RegExp, MN_("getsource"), 0,
		/*JS*/_Public|_Const|_Im, _F(RegExp_getlastIndex), TY_int, TY_RegExp, MN_("getlastIndex"), 0,
		/*JS*/_Public|_Im, _F(String_match), TY_StringArray0, TY_String, MN_("match"), 1, TY_RegExp, FN_("regexp"),
		/*JS*/_Public|_Const|_Im, _F(String_replace), TY_String, TY_String, MN_("replace"), 2, TY_RegExp, FN_("searchvalue"), TY_String, FN_("newvalue"),
		/*JS*/_Public|_Const|_Im, _F(String_search), TY_int, TY_String, MN_("search"), 1, TY_RegExp, FN_("searchvalue"),
		/*JS*/_Public|_Im, _F(String_split), TY_StringArray0, TY_String, MN_("split"), 1, TY_RegExp, FN_("separator"),
		/*JS*/_Public|_Im, _F(String_splitwithSeparatorLimit), TY_StringArray0, TY_String, MN_("split"), 2, TY_RegExp, FN_("separator"), TY_int, FN_("limit"),
		/*JS*/_Public|_Const, _F(RegExp_new),     TY_RegExp,  TY_RegExp,  MN_("new"), 1, TY_String, FN_("pattern"),
		/*JS*/_Public|_Const, _F(RegExp_newwithOption),     TY_RegExp,  TY_RegExp,  MN_("new"), 2, TY_String, FN_("pattern"), TY_String, FN_("option"),
		/*JS*/_Public|_Const, _F(RegExp_exec),    TY_StringArray0, TY_RegExp,  MN_("exec"), 1, TY_String, FN_("str"),
		/*JS*/_Public|_Const|_Im, _F(RegExp_test),    TY_boolean, TY_RegExp,  MN_("test"), 1, TY_String, FN_("str"),
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t regexp_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static KMETHOD parseREGEXP(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *)sfp[1].o;
	int ch, prev = '/', pos = 1;
	const char *source = S_text(sfp[2].asString);
	if(source[pos] == '*' || source[pos] == '/') {
		RETURNi_(0);
	}
	/*FIXME: we need to care about context sensitive case*/
	//int tokenListize = kArray_size(tenv->tokenList);
	//if(tokenListize > 0) {
	//	kToken *tkPrev = tenv->tokenList->tokenItems[tokenListize - 1];
	//	if(tkPrev->unresolvedTokenType == TokenType_INT ||
	//		(tkPrev->topCharHint != '(' && tkPrev->unresolvedTokenType == TokenType_SYMBOL)) {
	//		RETURNi_(0);
	//	}
	//}
	while((ch = source[pos++]) != 0) {
		if(ch == '\n') {
			break;
		}
		if(ch == '/' && prev != '\\') {
			int pos0 = pos;
			while(isalpha(source[pos])) pos++;
			if(IS_NOTNULL(tk)) {
				kArray *a = (kArray*)KLIB new_kObject(kctx, CT_StringArray0, 2);
				KLIB kArray_add(kctx, a, KLIB new_kString(kctx, source + 1, (pos0-2), 0));
				KLIB kArray_add(kctx, a, KLIB new_kString(kctx, source + pos0, pos-pos0, 0));
				tk->subTokenList = a;
				tk->unresolvedTokenType = SYM_("$regexp");
			}
			RETURNi_(pos);
		}
		prev = ch;
	}
	if(IS_NOTNULL(tk)) {
		kreportf(ErrTag, tk->uline, "must close with /");
	}
	RETURNi_(pos-1);
}

static KMETHOD ExprTyCheck_RegExp(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->termToken;
	kRegExp *r = new_(RegExp, NULL);
	DBG_ASSERT(kArray_size(tk->subTokenList) == 2);
	RegExp_set(kctx, r, tk->subTokenList->stringItems[0], tk->subTokenList->stringItems[1]);
	RETURN_(SUGAR kExpr_setConstValue(kctx, expr, TY_RegExp, UPCAST(r)));
}

static kbool_t regexp_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	kMethod *mtd = KLIB new_kMethod(kctx, 0, 0, 0, parseREGEXP);
	kFunc *fo = GCSAFE_new(Func, (uintptr_t) mtd);
	SUGAR kNameSpace_setTokenizeFunc(kctx, ns, '/', NULL, fo, 0);
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("$regexp"),  ExprTyCheck_(RegExp), },
		{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX, packageNameSpace);
	return true;
}

static kbool_t regexp_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* regexp_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("regexp", "1.0"),
		.initPackage    = regexp_initPackage,
		.setupPackage   = regexp_setupPackage,
		.initNameSpace  = regexp_initNameSpace,
		.setupNameSpace = regexp_setupNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
