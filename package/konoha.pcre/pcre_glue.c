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

#define USE_STRINGLIB

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/klib.h>
#include <pcre.h>

#ifdef __cplusplus
extern "C" {
#endif
/* ------------------------------------------------------------------------ */
/* regex module */
//## @Immutable class Regex Object;
//## flag Regex GlobalOption  1 - is set * *;

#define Regex_isGlobalOption(o)     (TFLAG_is(uintptr_t,(o)->h.magicflag,kObject_Local1))
#define Regex_setGlobalOption(o,b)  TFLAG_set(uintptr_t,(o)->h.magicflag,kObject_Local1,b)
typedef void kregex_t;

/* REGEX_SPI */
#ifndef KREGEX_MATCHSIZE
#define KREGEX_MATCHSIZE    16
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

static kArray *kStringToCharArray(KonohaContext *kctx, kString *bs, int istrim)
{
	kbytes_t base = {S_size(bs), {S_text(bs)}};
	size_t i, n = base.len;
	kArray *a = new_(Array, n); //TODO new_Array(kctx, TY_String, n);
	if(S_isASCII(bs)) {
		for(i = 0; i < n; i++) {
			if(istrim && isspace(base.utext[i])) continue;
			KLIB kArray_add(kctx, a, KLIB new_kString(kctx, base.text+i, 1, _ALWAYS|_ASCII));
		}
	}
	else {
		n = knh_bytes_mlen(base);
		for(i = 0; i < n; i++) {
			if(istrim && isspace(base.utext[i])) continue;
			kbytes_t sub = knh_bytes_mofflen(base, i, 1);
			KLIB kArray_add(kctx, a, KLIB new_kString(kctx, sub.text, sub.len, _ALWAYS|((sub.len == 1) ? _ASCII:_UTF8)));
		}
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
} PCRE_regex_t;

/* ------------------------------------------------------------------------ */
#define kregexshare      ((kregexshare_t*)kctx->modshare[MOD_REGEX])
#define CT_Regex         kregexshare->cRegex
#define TY_Regex         kregexshare->cRegex->classId

#define IS_Regex(O)      ((O)->h.ct == CT_Regex)

typedef struct {
	KonohaModule h;
	KonohaClass *cRegex;
} kregexshare_t;

typedef struct kRegex kRegex;
struct kRegex {
	KonohaObjectHeader h;
	kregex_t *reg;
	int eflags;      // regex flag
	kString *pattern;
};

typedef struct {
	int rm_so;   /* start of match */
	int rm_eo;   /* end of match */
	kbytes_t rm_name;  /* {NULL, 0}, if not NAMED */
} kregmatch_t;

/* ------------------------------------------------------------------------ */

static kregex_t* pcre_regmalloc(KonohaContext *kctx, kString* s)
{
	PCRE_regex_t *preg = (PCRE_regex_t*) KMALLOC(sizeof(PCRE_regex_t));
	return (kregex_t *) preg;
}

static void pcre_regfree(KonohaContext *kctx, kregex_t *reg)
{
	PCRE_regex_t *preg = (PCRE_regex_t*)reg;
	pcre_free(preg->re);
	KFREE(preg, sizeof(PCRE_regex_t));
}

static int pcre_nmatchsize(KonohaContext *kctx, kregex_t *reg)
{
	PCRE_regex_t *preg = (PCRE_regex_t*)reg;
	int capsize = 0;
	if (pcre_fullinfo(preg->re, NULL, PCRE_INFO_CAPTURECOUNT, &capsize) != 0) {
		return KREGEX_MATCHSIZE;
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

//static size_t pcre_regerror(int res, kregex_t *reg, char *ebuf, size_t ebufsize)
//{
//	PCRE_regex_t *pcre = (PCRE_regex_t*)reg;
//	snprintf(ebuf, ebufsize, "[%d]: %s", pcre->erroffset, pcre->err);
//	return (pcre->err != NULL) ? strlen(pcre->err) : 0;
//}

static int pcre_regcomp(KonohaContext *kctx, kregex_t *reg, const char *pattern, int cflags)
{
	PCRE_regex_t* preg = (PCRE_regex_t*)reg;
	preg->re = pcre_compile(pattern, cflags, &preg->err, &preg->erroffset, NULL);
	return (preg->re != NULL) ? 0 : -1;
}

static int pcre_regexec(KonohaContext *kctx, kregex_t *reg, const char *str, size_t nmatch, kregmatch_t p[], int eflags)
{
	PCRE_regex_t *preg = (PCRE_regex_t*)reg;
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
static void kregexshare_setup(KonohaContext *kctx, struct KonohaModule *def, int newctx)
{
}

static void kregexshare_reftrace(KonohaContext *kctx, struct KonohaModule *baseh)
{
}

static void kregexshare_free(KonohaContext *kctx, struct KonohaModule *baseh)
{
	KFREE(baseh, sizeof(kregexshare_t));
}

/* ------------------------------------------------------------------------ */
static void knh_Regex_setGlobalOption(kRegex *re, const char *opt)
{
	const char *p = opt;
	while(*p != 0) {
		if(*p == 'g') {
			Regex_setGlobalOption(re, 1);
			break;
		}
		p++;
	}
}

static size_t knh_regex_matched(kregmatch_t* r, size_t maxmatch)
{
	size_t n = 0;
	for (; n < maxmatch && r[n].rm_so != -1; n++) ;
	return n;
}

static void WB_write_regexfmt(KonohaContext *kctx, KUtilsWriteBuffer *wb, kbytes_t *fmt, const char *base, kregmatch_t *r, size_t matched)
{
	const char *ch = fmt->text;
	const char *eof = ch + fmt->len; // end of fmt
	for (; ch < eof; ch++) {
		if (*ch == '\\') {
			kwb_putc(wb, *ch);
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
		kwb_putc(wb, *ch);
	}
}

static void Regex_init(KonohaContext *kctx, kObject *o, void *conf)
{
	kRegex *re = (kRegex*)o;
	re->reg = NULL;
	re->eflags = 0;
	re->pattern = KNULL(String);
}

static void Regex_free(KonohaContext *kctx, kObject *o)
{
	kRegex *re = (kRegex*)o;
	if(re->reg != NULL) {
		pcre_regfree(kctx, re->reg);
	}
}

static void Regex_p(KonohaContext *kctx, KonohaStack *sfp, int pos, KUtilsWriteBuffer *wb, int level)
{
	KLIB Kwb_printf(kctx, wb, "/%s/", S_text(sfp[pos].re->pattern));
}

static void Regex_set(KonohaContext *kctx, kRegex *re, kString *ptns, kString *opts)
{
	const char *ptn = S_text(ptns);
	const char *opt = S_text(opts);
	knh_Regex_setGlobalOption(re, opt);
	KSETv(re, re->pattern, ptns);
	re->reg = pcre_regmalloc(kctx, ptns);
	pcre_regcomp(kctx, re->reg, ptn, pcre_parsecflags(kctx, opt));
	re->eflags = pcre_parseeflags(kctx, opt);
}

/* ------------------------------------------------------------------------ */
//## @Const method Regex Regex.new(String pattern, String option);

static KMETHOD Regex_new(KonohaContext *kctx, KonohaStack *sfp)
{
	Regex_set(kctx, sfp[0].re, sfp[1].asString, sfp[2].s);
	RETURN_(sfp[0].asObject);
}

/* ------------------------------------------------------------------------ */
//## @Const method Int String.search(Regex re);

static KMETHOD String_search(KonohaContext *kctx, KonohaStack *sfp)
{
	kRegex *re = sfp[1].re;
	intptr_t loc = -1;
	if(!IS_NULL(re) && S_size(re->pattern) > 0) {
		kregmatch_t pmatch[2]; // modified by @utrhira
		const char *str = S_text(sfp[0].s);  // necessary
		int res = pcre_regexec(kctx, re->reg, str, 1, pmatch, re->eflags);
		if(res == 0) {
			loc = pmatch[0].rm_so;
			if (loc != -1 && !S_isASCII(sfp[0].s)) {
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
//## @Const method String[] String.match(Regex re);

static KMETHOD String_match(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].s;
	kRegex *re = sfp[1].re;
	kArray *a = NULL;
	if(IS_NOTNULL(re) && S_size(re->pattern) > 0) {
		const char *str = S_text(s0);  // necessary
		const char *base = str;
		const char *eos = base + S_size(s0);
		size_t nmatch = pcre_nmatchsize(kctx, re->reg);
		kregmatch_t *p, pmatch[nmatch+1];
		int i, isGlobalOption = Regex_isGlobalOption(re);
		a = new_(Array, nmatch);/*TODO new_Array(TY_String)*/
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
		a = new_(Array, 0);/*TODO new_Array(TY_String)*/
	}
	RETURN_(a);
/*new_(O_ct(sfp[K_RTNIDX].o), 0);  // USE THIS; */
}

/* ------------------------------------------------------------------------ */
//## @Const method String String.replace(Regex re, String s);

static KMETHOD String_replace(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].s;
	kRegex *re = sfp[1].re;
	kbytes_t fmt = {S_size(sfp[2].s), {S_text(sfp[2].s)}};
	kString *s = s0;
	if(IS_NOTNULL(re) && S_size(re->pattern) > 0) {
		KUtilsWriteBuffer wb;
		KLIB Kwb_init(&(kctx->stack->cwb), &wb);
		const char *str = S_text(s0);  // necessary
		const char *base = str;
		const char *eos = str + S_size(s0); // end of str
		kregmatch_t pmatch[KREGEX_MATCHSIZE+1];
		while (str < eos) {
			int res = pcre_regexec(kctx, re->reg, str, KREGEX_MATCHSIZE, pmatch, re->eflags);
			if(res != 0) {
				// TODO
				//LOG_regex(kctx, sfp, res, re, str);
				break;
			}
			size_t len = pmatch[0].rm_eo;
			if (pmatch[0].rm_so > 0) {
				KLIB Kwb_write(kctx, &wb, str, pmatch[0].rm_so);
			}
			size_t matched = knh_regex_matched(pmatch, KREGEX_MATCHSIZE);
			if (len > 0) {
				WB_write_regexfmt(kctx, &wb, &fmt, base, pmatch, matched);
				str += len;
			} else {
				if (str == base) { // 0-length match at head of string
					WB_write_regexfmt(kctx, &wb, &fmt, base, pmatch, matched);
				}
				break;
			}
		}
		KLIB Kwb_write(kctx, &wb, str, strlen(str)); // write out remaining string
		s = kwb_newString(kctx, &wb, 0); // close cwb
	}
	RETURN_(s);
}

/* ------------------------------------------------------------------------ */
//## @Const method String[] String.split(Regex re);

static KMETHOD String_split(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].s;
	kRegex *re = sfp[1].re;
	kArray *a = NULL;
	if (IS_NOTNULL(re) && S_size(re->pattern) > 0) {
		const char *str = S_text(s0);  // necessary
		const char *eos = str + S_size(s0);
		kregmatch_t pmatch[KREGEX_MATCHSIZE+1];
		if (str < eos) {
			a = new_(Array, 0); // TODO new_Array(kctx, TY_String, 0);
			BEGIN_LOCAL(lsfp, 1);
			KSETv_AND_WRITE_BARRIER(NULL, lsfp[0].asArray, a, GC_NO_WRITE_BARRIER);
			while (str <= eos) {
				int res = pcre_regexec(kctx, re->reg, str, KREGEX_MATCHSIZE, pmatch, re->eflags);
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
			a = kStringToCharArray(kctx, KLIB new_kString(kctx, str, S_size(s0), SPOL_POOL), 0);
		}
	}
	else {
		a = kStringToCharArray(kctx, s0, 0);
	}
	RETURN_(a);
}

// --------------------------------------------------------------------------

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _F(F)   (intptr_t)(F)

static kbool_t pcre_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	kregexshare_t *base = (kregexshare_t*)KCALLOC(sizeof(kregexshare_t), 1);
	base->h.name     = "regex";
	base->h.setup    = kregexshare_setup;
	base->h.reftrace = kregexshare_reftrace;
	base->h.free     = kregexshare_free;
	KLIB Konoha_setModule(kctx, MOD_REGEX, &base->h, pline);

	KDEFINE_CLASS RegexDef = {
		STRUCTNAME(Regex),
		.cflag = 0,
		.init = Regex_init,
		.free = Regex_free,
		.p    = Regex_p,
	};
	base->cRegex = KLIB Konoha_defineClass(kctx, ns->packageId, PN_konoha, NULL, &RegexDef, pline);

	kparamtype_t p = { .ty = TY_String,  };
	KonohaClass *cStrArray = KLIB KonohaClass_Generics(kctx, CT_(TY_Array), TY_void, 1, &p);
#define TY_StrArray (cStrArray->classId)
	int FN_x = FN_("x");
	int FN_y = FN_("y");
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const, _F(Regex_new),     TY_Regex,  TY_Regex,  MN_("new"), 2, TY_String, FN_x, TY_String, FN_y,
		_Public|_Const, _F(String_search), TY_Int,    TY_String, MN_("search"),  1, TY_Regex, FN_x,
		_Public|_Const, _F(String_replace),TY_String, TY_String, MN_("replace"), 2, TY_Regex, FN_x, TY_String, FN_y,
		_Public|_Const, _F(String_match),  TY_StrArray, TY_String, MN_("match"), 1, TY_Regex, FN_x,
		_Public|_Const, _F(String_split),  TY_StrArray, TY_String, MN_("split"), 1, TY_Regex, FN_x,
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);
	return true;
}

static kbool_t pcre_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static KMETHOD parseREGEX(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *)sfp[1].o;
	int ch, prev = '/', pos = 1;
	const char *source = S_text(sfp[2].asString);
	if(source[pos] == '*' || source[pos] == '/') {
		RETURNi_(0);
	}
	/*FIXME: we need to care about context sensitive case*/
	//int tokenArrayize = kArray_size(tenv->tokenList);
	//if(tokenArrayize > 0) {
	//	kToken *tkPrev = tenv->tokenList->tokenItems[tokenArrayize - 1];
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
				kArray *a = new_(Array, 2);
				KLIB kArray_add(kctx, a, KLIB new_kString(kctx, source + 1, (pos0-2), 0));
				KLIB kArray_add(kctx, a, KLIB new_kString(kctx, source + pos0, pos-pos0, 0));
				tk->subTokenList = a;
				tk->unresolvedTokenType = SYM_("$regex");
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

static KMETHOD ExprTyCheck_Regex(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_ExprTyCheck(stmt, expr, gma, reqty);
	kToken *tk = expr->termToken;
	kRegex *r = new_(Regex, NULL);
	DBG_ASSERT(kArray_size(tk->subTokenList) == 2);
	Regex_set(kctx, r, tk->subTokenList->stringItems[0], tk->subTokenList->stringItems[1]);
	RETURN_(SUGAR kExpr_setConstValue(kctx, expr, TY_Regex, UPCAST(r)));
}

static kbool_t pcre_initNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	kMethod *mtd = KLIB new_kMethod(kctx, 0, 0, 0, parseREGEX);
	kFunc *fo = GCSAFE_new(Func, (uintptr_t) mtd);
	SUGAR kNameSpace_setTokenizeFunc(kctx, ns, '/', NULL, fo, 0);
	KDEFINE_SYNTAX SYNTAX[] = {
		{ .keyword = SYM_("$regex"), _TERM, ExprTyCheck_(Regex), },
		{ .keyword = KW_END, },
	};
	SUGAR kNameSpace_defineSyntax(kctx, ns, SYNTAX);
	return true;
}

static kbool_t pcre_setupNameSpace(KonohaContext *kctx, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

KDEFINE_PACKAGE* pcre_init(void)
{
	static const KDEFINE_PACKAGE d = {
		KPACKNAME("regex", "1.0"),
		.initPackage = pcre_initPackage,
		.setupPackage = pcre_setupPackage,
		.initNameSpace = pcre_initNameSpace,
		.setupNameSpace = pcre_setupNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif

