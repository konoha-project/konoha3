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

#define USE_STRINGLIB 1

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/klib.h>
#include <konoha3/konoha_common.h>
#include <konoha3/import/methoddecl.h>
#include <pcre.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KClass_StringArray0        KClass_p0(kctx, KClass_Array, KType_String)

/* ------------------------------------------------------------------------ */
/* regexp module */

//## @Immutable class RegExp Object;
//## flag RegExp Global      1 - is set * *;
//## flag RegExp IgnoreCase  2 - is set * *; // it is used only for RegExp_p
//## flag RegExp Multiline   3 - is set * *; // it is used only for RegExp_p

#define RegExp_isGlobal(o)        (KFlag_Is(uintptr_t,(o)->h.magicflag,kObjectFlag_Local1))
#define RegExp_SetGlobal(o,b)     KFlag_Set(uintptr_t,(o)->h.magicflag,kObjectFlag_Local1,b)
#define RegExp_isIgnoreCase(o)    (KFlag_Is(uintptr_t,(o)->h.magicflag,kObjectFlag_Local2))
#define RegExp_SetIgnoreCase(o,b) KFlag_Set(uintptr_t,(o)->h.magicflag,kObjectFlag_Local2,b)
#define RegExp_isMultiline(o)     (KFlag_Is(uintptr_t,(o)->h.magicflag,kObjectFlag_Local3))
#define RegExp_SetMultiline(o,b)  KFlag_Set(uintptr_t,(o)->h.magicflag,kObjectFlag_Local3,b)

typedef void kregexp_t;

#ifndef KREGEXP_MATCHSIZE
#define KREGEXP_MATCHSIZE    16
#endif

//typedef struct {
//	size_t       len;
//	union {
//		const char *text;
//		const unsigned char *utext;
//		char *buf;
//		//kchar_t *ubuf;
//	};
//} kbytes_t;

static size_t utf8_strlen(const char *text, size_t len)
{
	size_t size = 0;
	const unsigned char *s = (const unsigned char *)text;
	const unsigned char *eos = s + len;
	while(s < eos) {
		size_t ulen = utf8len(s[0]);
		size++;
		s += ulen;
	}
	return size;
}

typedef struct {
	pcre *re;
	const char *err;
	int erroffset;
} PCRE_regexp_t;

/* ------------------------------------------------------------------------ */

typedef struct kRegExpVar kRegExp;
struct kRegExpVar {
	kObjectHeader h;
	kregexp_t *reg;
	int eflags;      // regexp flag
	kString *pattern;
	size_t lastIndex; // last matched index
};

typedef struct {
	int rm_so;   /* start of match */
	int rm_eo;   /* end of match */
	const unsigned char *rm_name;
	size_t      rm_namelen;
} kregmatch_t;

/* ------------------------------------------------------------------------ */

static kregexp_t* pcre_regmalloc(KonohaContext *kctx, kString* s)
{
	PCRE_regexp_t *preg = (PCRE_regexp_t *) KMalloc_UNTRACE(sizeof(PCRE_regexp_t));
	return (kregexp_t *) preg;
}

static void pcre_regfree(KonohaContext *kctx, kregexp_t *reg)
{
	PCRE_regexp_t *preg = (PCRE_regexp_t *)reg;
	pcre_free(preg->re);
	KFree(preg, sizeof(PCRE_regexp_t));
}

static int pcre_nmatchsize(KonohaContext *kctx, kregexp_t *reg)
{
	PCRE_regexp_t *preg = (PCRE_regexp_t *)reg;
	int capsize = 0;
	if(pcre_fullinfo(preg->re, NULL, PCRE_INFO_CAPTURECOUNT, &capsize) != 0) {
		return KREGEXP_MATCHSIZE;
	}
	return capsize + 1;
}

static int pcre_ParseComplflags(KonohaContext *kctx, const char *option)
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

static int pcre_ParseExecflags(KonohaContext *kctx, const char *option)
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
//	PCRE_regexp_t *pcre = (PCRE_regexp_t *)reg;
//	snprintf(ebuf, ebufsize, "[%d]: %s", pcre->erroffset, pcre->err);
//	return (pcre->err != NULL) ? strlen(pcre->err) : 0;
//}

static int pcre_regcomp(KonohaContext *kctx, kregexp_t *reg, const char *pattern, int cflags)
{
	PCRE_regexp_t* preg = (PCRE_regexp_t *)reg;
	preg->re = pcre_compile(pattern, cflags, &preg->err, &preg->erroffset, NULL);
	return (preg->re != NULL) ? 0 : -1;
}

static int pcre_regexec(KonohaContext *kctx, kregexp_t *reg, const char *str, size_t nmatch, kregmatch_t p[], int eflags)
{
	PCRE_regexp_t *preg = (PCRE_regexp_t *)reg;
	int res, nvector[nmatch*3];
	nvector[0] = 0;
	size_t idx, matched = nmatch;
	if(strlen(str) == 0) return -1;
	if((res = pcre_exec(preg->re, NULL, str, strlen(str), 0, eflags, nvector, nmatch*3)) >= 0) {
		size_t nm_count = 0;
		matched = (res > 0 && (size_t)res < nmatch) ? (size_t)res : nmatch;
		res = 0;
		for (idx = 0; idx < matched; idx++) {
			p[idx].rm_so = nvector[2*idx];
			p[idx].rm_eo = nvector[2*idx+1];
		}
		p[idx].rm_so = -1;
		pcre_fullinfo(preg->re, NULL, PCRE_INFO_NAMECOUNT, &nm_count);
		if(nm_count > 0) {
			unsigned char *nm_table;
			int nm_entry_size = 0;
			pcre_fullinfo(preg->re, NULL, PCRE_INFO_NAMETABLE, &nm_table);
			pcre_fullinfo(preg->re, NULL, PCRE_INFO_NAMEENTRYSIZE, &nm_entry_size);
			unsigned char *tbl_ptr = nm_table;
			for (idx = 0; idx < nm_count; idx++) {
				int n_idx = (tbl_ptr[0] << 8) | tbl_ptr[1];
				unsigned char *n_name = tbl_ptr + 2;
				p[n_idx].rm_name = n_name;
				p[n_idx].rm_namelen = strlen((char *)n_name);
				tbl_ptr += nm_entry_size;
			}
		}
	}
	return res;
}


/* ------------------------------------------------------------------------ */

static void kRegExp_SetOptions(kRegExp *re, const char *option)
{
	size_t i, optlen = strlen(option);
	for(i = 0; i < optlen; i++) {
		switch(option[i]) {
		case 'g':
			RegExp_SetGlobal(re, 1);
			break;
		case 'i':
			RegExp_SetIgnoreCase(re, 1);
			break;
		case 'm':
			RegExp_SetMultiline(re, 1);
		default:
			break;
		}
	}
}

static size_t knh_regexp_Matched(kregmatch_t* r, size_t maxmatch)
{
	size_t n = 0;
	for (; n < maxmatch && r[n].rm_so != -1; n++) {}
	return n;
}

static void KBuffer_WriteRegexFormat(KonohaContext *kctx, KBuffer *wb, const char *fmttext, size_t fmtlen, const char *base, kregmatch_t *r, size_t matched)
{
	const char *ch = fmttext;
	const char *eof = ch + fmtlen; // end of fmt
	char buf[1];
	for (; ch < eof; ch++) {
		if(*ch == '\\') {
			buf[0] = *ch;
			KLIB KBuffer_Write(kctx, wb, buf, 1);
			ch++;
		} else if(*ch == '$' && isdigit(ch[1])) {
			size_t grpidx = (size_t)ch[1] - '0'; // get head of grourp_index
			if(grpidx < matched) {
				ch++;
				while(isdigit(ch[1])) {
					size_t nidx = grpidx * 10 + (ch[1] - '0');
					if(nidx < matched) {
						grpidx = nidx;
						ch++;
						if(ch < eof) {
							continue;
						}
					}
				}
				kregmatch_t *rp = &r[grpidx];
				KLIB KBuffer_Write(kctx, wb, base + rp->rm_so, rp->rm_eo - rp->rm_so);
				continue; // skip putc
			}
		}
		buf[0] = *ch;
		KLIB KBuffer_Write(kctx, wb, buf, 1);
	}
}

static void RegExp_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	kRegExp *re = (kRegExp *)o;
	re->reg = NULL;
	re->eflags = 0;
	re->pattern = KNULL(String);
	re->lastIndex = 0;
}

static void RegExp_Free(KonohaContext *kctx, kObject *o)
{
	kRegExp *re = (kRegExp *)o;
	if(re->reg != NULL) {
		pcre_regfree(kctx, re->reg);
	}
}

static void RegExp_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	kRegExp *re = v[pos].asRegExp;
	KLIB KBuffer_printf(kctx, wb, "/%s/%s%s%s", kString_text(re->pattern),
			RegExp_isGlobal(re) ? "g" : "",
			RegExp_isIgnoreCase(re) ? "i" : "",
			RegExp_isMultiline(re) ? "m" : "");
}

static void RegExp_Set(KonohaContext *kctx, kRegExp *re, kString *ptns, kString *opts)
{
	const char *ptn = kString_text(ptns);
	const char *opt = kString_text(opts);
	kRegExp_SetOptions(re, opt);
	KFieldSet(re, re->pattern, ptns);
	re->reg = pcre_regmalloc(kctx, ptns);
	int cflags = pcre_ParseComplflags(kctx, opt);
	if(!kString_Is(ASCII, ptns)) {
		/* Add 'u' option when the pattern is multibyte string. */
		cflags |= PCRE_UTF8;
	}
	pcre_regcomp(kctx, re->reg, ptn, cflags);
	re->eflags = pcre_ParseExecflags(kctx, opt);
}

/* ------------------------------------------------------------------------ */
//## @Const method Boolean RegExp.getglobal();

static KMETHOD RegExp_getglobal(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(RegExp_isGlobal(sfp[0].asRegExp));
}

/* ------------------------------------------------------------------------ */
//## @Const method Boolean RegExp.getignoreCase();

static KMETHOD RegExp_getignoreCase(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(RegExp_isIgnoreCase(sfp[0].asRegExp));
}

/* ------------------------------------------------------------------------ */
//## @Const method Boolean RegExp.getmultiline();

static KMETHOD RegExp_getmultiline(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(RegExp_isMultiline(sfp[0].asRegExp));
}

/* ------------------------------------------------------------------------ */
//## @Const method String RegExp.getsource();

static KMETHOD RegExp_getsource(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(sfp[0].asRegExp->pattern);
}

/* ------------------------------------------------------------------------ */
//## @Const method Int RegExp.getlastIndex();

static KMETHOD RegExp_getlastIndex(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(sfp[0].asRegExp->lastIndex);
}

/* ------------------------------------------------------------------------ */
//## @Const method RegExp RegExp.new(String pattern);

static KMETHOD RegExp_new(KonohaContext *kctx, KonohaStack *sfp)
{
	RegExp_Set(kctx, sfp[0].asRegExp, sfp[1].asString, KNULL(String));
	KReturn(sfp[0].asObject);
}

/* ------------------------------------------------------------------------ */
//## @Const method RegExp RegExp.new(String pattern, String option);

static KMETHOD RegExp_new2(KonohaContext *kctx, KonohaStack *sfp)
{
	RegExp_Set(kctx, sfp[0].asRegExp, sfp[1].asString, sfp[2].asString);
	KReturn(sfp[0].asObject);
}

/* ------------------------------------------------------------------------ */
//## @Const method Int String.search(RegExp searchvalue);

static KMETHOD String_search(KonohaContext *kctx, KonohaStack *sfp)
{
	kRegExp *re = sfp[1].asRegExp;
	intptr_t loc = -1;
	if(!IS_NULL(re) && kString_size(re->pattern) > 0) {
		kregmatch_t pmatch[2]; // modified by @utrhira
		const char *str = kString_text(sfp[0].asString);  // necessary
		int res = pcre_regexec(kctx, re->reg, str, 1, pmatch, re->eflags);
		if(res == 0) {
			loc = pmatch[0].rm_so;
			if(loc != -1 && !kString_Is(ASCII, sfp[0].asString)) {
				loc = utf8_strlen(str, loc);
			}
		}
		else {
			//TODO
			//LOG_regex(kctx, sfp, res, re, str);
		}
	}
	KReturnUnboxValue(loc);
}

/* ------------------------------------------------------------------------ */

static void kArray_executeRegExp(KonohaContext *kctx, kArray *resultArray, kRegExp *regex, kString *s0)
{
	int stringPolicy = kString_Is(ASCII, s0) ? StringPolicy_ASCII : 0;
	if(IS_NOTNULL(regex) && kString_size(regex->pattern) > 0) {
		const char *s = kString_text(s0);  // necessary
		const char *base = s;
		const char *eos = base + kString_size(s0);
		size_t i, nmatch = pcre_nmatchsize(kctx, regex->reg);
		kregmatch_t *p, pmatch[nmatch+1];
		int isGlobalOption = RegExp_isGlobal(regex);
		do {
			int res = pcre_regexec(kctx, regex->reg, s, nmatch, pmatch, regex->eflags);
			if(res != 0) {
				// FIXME
				//LOG_regex(kctx, sfp, res, regex, s);
				break;
			}
			for(p = pmatch, i = 0; i < nmatch; p++, i++) {
				if(p->rm_so == -1) break;
				KLIB new_kString(kctx, resultArray, s + (p->rm_so), ((p->rm_eo) - (p->rm_so)), stringPolicy);
			}
			if(isGlobalOption) {
				size_t eo = pmatch[0].rm_eo; // shift matched pattern
				s += (eo > 0) ? eo : 1;
				if(!(s < eos)) isGlobalOption = 0; // stop iteration
			}
		} while(isGlobalOption);
	}
}

//## @Const String[] String.match(RegExp regexp);
static KMETHOD String_Match(KonohaContext *kctx, KonohaStack *sfp)
{
	INIT_GCSTACK();
	kArray *resultArray = (kArray *)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), 0);
	kArray_executeRegExp(kctx, resultArray, sfp[1].asRegExp, sfp[0].asString);
	KReturnWith(resultArray, RESET_GCSTACK());
}

//## @Const String[] RegExp.exec(String str);
static KMETHOD RegExp_exec(KonohaContext *kctx, KonohaStack *sfp)
{
	INIT_GCSTACK();
	kArray *resultArray = (kArray *)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), 0);
	kArray_executeRegExp(kctx, resultArray, sfp[0].asRegExp, sfp[1].asString);
	KReturnWith(resultArray, RESET_GCSTACK());
}

/* ------------------------------------------------------------------------ */

//## @Const method String String.replace(RegExp searchvalue, String newvalue);
static KMETHOD String_replace(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[0].asString;
	kRegExp *re = sfp[1].asRegExp;
	const char* fmttext = kString_text(sfp[2].asString);
	size_t fmtlen = kString_size(sfp[2].asString);
	kString *s = s0;
	if(IS_NOTNULL(re) && kString_size(re->pattern) > 0) {
		KBuffer wb;
		KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
		const char *str = kString_text(s0);  // necessary
		const char *base = str;
		const char *eos = str + kString_size(s0); // end of str
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
			if(pmatch[0].rm_so > 0) {
				KLIB KBuffer_Write(kctx, &wb, str, pmatch[0].rm_so);
			}
			size_t matched = knh_regexp_Matched(pmatch, KREGEXP_MATCHSIZE);
			if(len > 0) {
				KBuffer_WriteRegexFormat(kctx, &wb, fmttext, fmtlen, base, pmatch, matched);
				str += len;
			} else {
				if(str == base) { // 0-length match at head of string
					KBuffer_WriteRegexFormat(kctx, &wb, fmttext, fmtlen, base, pmatch, matched);
				}
				break;
			}
		} while(isGlobalOption);
		KLIB KBuffer_Write(kctx, &wb, str, strlen(str)); // write out remaining string
		s = KLIB KBuffer_Stringfy(kctx, &wb, OnStack, StringPolicy_FreeKBuffer);
	}
	KReturn(s);
}

/* ------------------------------------------------------------------------ */
/* split */

static void kArray_split(KonohaContext *kctx, kArray *resultArray, kString *str, kRegExp *regex, size_t limit)
{
	int stringPolicy = kString_Is(ASCII, str) ? StringPolicy_ASCII : 0;
	if(IS_NOTNULL(regex) && kString_size(regex->pattern) > 0) {
		const char *s = kString_text(str);  // necessary
		const char *eos = s + kString_size(str);
		kregmatch_t pmatch[2];
		int res = 0;
		while(s < eos && res == 0) {
			res = pcre_regexec(kctx, regex->reg, s, 1, pmatch, regex->eflags);
			if(res != 0) break;
			size_t len = pmatch[0].rm_eo;
			if(len > 0) {
				KLIB new_kString(kctx, resultArray, s, pmatch[0].rm_so, stringPolicy);
				s += len;
			}
			if(!(kArray_size(resultArray) + 1 < limit)) {
				return;
			}
		}
		if(s < eos) {
			KLIB new_kString(kctx, resultArray, s, eos - s, stringPolicy); // append remaining string to array
		}
	}
	else {
		const unsigned char *s = (const unsigned char *)kString_text(str);
		size_t i, n = kString_size(str);
		if(kString_Is(ASCII, str)) {
			for(i = 0; i < n; i++) {
				KLIB new_kString(kctx, resultArray, (const char *)s + i, 1, StringPolicy_ASCII);
			}
		}
		else {
			for(i = 0; i < n; i++) {
				int len = utf8len(s[i]);
				KLIB new_kString(kctx, resultArray, (const char *)s + i, len, len == 1 ? StringPolicy_ASCII: StringPolicy_UTF8);
				i += len;
			}
		}
	}
}

//## @Const method String[] String.split(RegExp regex);
static KMETHOD String_split(KonohaContext *kctx, KonohaStack *sfp)
{
	INIT_GCSTACK();
	kArray *resultArray = (kArray *)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), 0);
	kArray_split(kctx, resultArray, sfp[0].asString, sfp[1].asRegExp, kString_size(sfp[0].asString));
	KReturnWith(resultArray, RESET_GCSTACK());
}

//## @Const method String[] String.split(RegExp regex, Int limit);
static KMETHOD String_splitWithLimit(KonohaContext *kctx, KonohaStack *sfp)
{
	INIT_GCSTACK();
	size_t limit = sfp[2].intValue < 0 ? kString_size(sfp[0].asString) : (size_t) sfp[2].intValue;
	kArray *resultArray = (kArray *)KLIB new_kObject(kctx, _GcStack, KGetReturnType(sfp), 0);
	kArray_split(kctx, resultArray, sfp[0].asString, sfp[1].asRegExp, limit);
	KReturnWith(resultArray, RESET_GCSTACK());
}

/* ------------------------------------------------------------------------ */
//## @Const method Boolean RegExp.test(String str);

static KMETHOD RegExp_test(KonohaContext *kctx, KonohaStack *sfp)
{
	kString *s0 = sfp[1].asString;
	kRegExp *re = sfp[0].asRegExp;
	kbool_t matched = false;
	if(IS_NOTNULL(re) && kString_size(re->pattern) > 0) {
		const char *str = kString_text(s0);  // necessary
		const char *base = str;
		const char *eos = base + kString_size(s0);
		size_t i, nmatch = pcre_nmatchsize(kctx, re->reg);
		kregmatch_t *p, pmatch[nmatch+1];
		int isGlobalOption = RegExp_isGlobal(re);
		do {
			int res = pcre_regexec(kctx, re->reg, str, nmatch, pmatch, re->eflags);
			if(res != 0) {
				// FIXME
				//LOG_regex(kctx, sfp, res, re, str);
				break;
			}
			for(p = pmatch, i = 0; i < nmatch; p++, i++) {
				if(p->rm_so == -1) break;
				re->lastIndex = utf8_strlen(str, p->rm_eo);
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
	KReturnUnboxValue(matched);
}
/* ------------------------------------------------------------------------ */
//## @Const method RegExp RegExp.create(String patter, String option);

static KMETHOD RegExp_create(KonohaContext *kctx, KonohaStack *sfp)
{
	kRegExp *Reg = (kRegExp *) KLIB new_kObject(kctx, OnStack, KClass_RegExp, 0);
	RegExp_Set(kctx, Reg, sfp[1].asString, sfp[2].asString);
	KReturn(Reg);
}

// --------------------------------------------------------------------------

static kbool_t regexp_defineMethod(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	if(KClass_RegExp == NULL) {
		KDEFINE_CLASS RegExpDef = {
			STRUCTNAME(RegExp),
			.cflag = 0,
			.init = RegExp_Init,
			.free = RegExp_Free,
			.format = RegExp_format,
		};
		KClass_RegExp = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &RegExpDef, trace);
	}

	ktypeattr_t KType_StringArray0 = KClass_StringArray0->typeId;
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(RegExp_getglobal), KType_Boolean, KType_RegExp, KMethodName_("getglobal"), 0,
		_Public|_Const|_Im, _F(RegExp_getignoreCase), KType_Boolean, KType_RegExp, KMethodName_("getignoreCase"), 0,
		_Public|_Const|_Im, _F(RegExp_getmultiline), KType_Boolean, KType_RegExp, KMethodName_("getmultiline"), 0,
		_Public|_Const|_Im, _F(RegExp_getsource), KType_String, KType_RegExp, KMethodName_("getsource"), 0,
		_Public|_Const|_Im, _F(RegExp_getlastIndex), KType_Int, KType_RegExp, KMethodName_("getlastIndex"), 0,
		_Public|_Im, _F(String_Match), KType_StringArray0, KType_String, KMethodName_("match"), 1, KType_RegExp, KFieldName_("regexp"),
		_Public|_Const|_Im, _F(String_replace), KType_String, KType_String, KMethodName_("replace"), 2, KType_RegExp, KFieldName_("searchvalue"), KType_String, KFieldName_("newvalue"),
		_Public|_Const|_Im, _F(String_search), KType_Int, KType_String, KMethodName_("search"), 1, KType_RegExp, KFieldName_("searchvalue"),
		_Public|_Im, _F(String_split), KType_StringArray0, KType_String, KMethodName_("split"), 1, KType_RegExp, KFieldName_("separator"),
		_Public|_Im, _F(String_splitWithLimit), KType_StringArray0, KType_String, KMethodName_("split"), 2, KType_RegExp, KFieldName_("separator"), KType_Int, KFieldName_("limit"),
		_Public|_Const, _F(RegExp_new),     KType_RegExp,  KType_RegExp,  KMethodName_("new"), 1, KType_String, KFieldName_("pattern"),
		_Public|_Const, _F(RegExp_new2),     KType_RegExp,  KType_RegExp,  KMethodName_("new"), 2, KType_String, KFieldName_("pattern"), KType_String, KFieldName_("option"),
		_Public|_Const, _F(RegExp_exec),    KType_StringArray0, KType_RegExp,  KMethodName_("exec"), 1, KType_String, KFieldName_("str"),
		_Public|_Const|_Im, _F(RegExp_test),    KType_Boolean, KType_RegExp,  KMethodName_("test"), 1, KType_String, KFieldName_("str"),
		_Static|_Public|_Const, _F(RegExp_create), KType_RegExp, KType_RegExp, KMethodName_("create"), 2, KType_String, KFieldName_("pattern"), KType_String, KFieldName_("option"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	return true;
}

static KMETHOD TokenFunc_JavaScriptRegExp(KonohaContext *kctx, KonohaStack *sfp)
{
	kTokenVar *tk = (kTokenVar *)sfp[1].asObject;
	int ch, prev = '/', pos = 1;
	const char *source = kString_text(sfp[2].asString);
	if(source[pos] == '*' || source[pos] == '/') {
		KReturnUnboxValue(0);
	}
	/*FIXME: we need to care about context sensitive case*/
	//int tokenListize = kArray_size(tenv->tokenList);
	//if(tokenListize > 0) {
	//	kToken *tkPrev = tenv->tokenList->TokenItems[tokenListize - 1];
	//	if(tkPrev->tokenType == TokenType_Number ||
	//		(tkPrev->topCharHint != '(' && tkPrev->tokenType == TokenType_Symbol)) {
	//		KReturnUnboxValue(0);
	//	}
	//}
	while((ch = source[pos++]) != 0) {
		if(ch == '\n') {
			KReturnUnboxValue(-1);
			break;
		}
		if(ch == '/' && prev != '\\') {
			//int pos0 = pos;
			while(isalpha(source[pos])) pos++;
			if(IS_NOTNULL(tk)) {
				KFieldSet(tk, tk->text,
						KLIB new_kString(kctx, 0, source + 1, pos, 0));
				tk->tokenType = KSymbol_("$RegExp");
			}
			KReturnUnboxValue(pos);
		}
		prev = ch;
	}
	if(IS_NOTNULL(tk)) {
		SUGAR kToken_ToError(kctx, tk, ErrTag, "must close with %s", "/");
	}
	KReturnUnboxValue(pos-1);
}

static KMETHOD Expression_RegExp(KonohaContext *kctx, KonohaStack *sfp)
{
	VAR_Expression(expr, tokenList, beginIdx, operatorIdx, endIdx);
	KDump(expr);
	INIT_GCSTACK();
	kTokenVar *tk = tokenList->TokenVarItems[beginIdx];
	const char *source = kString_text(tk->text);
	int ch, pos = 0, prev = 0;
	int returnIdx = -1;
	while((ch = source[pos++]) != 0) {
		if(ch == '\n') {
			break;
		}
		if(ch == '/' && prev != '\\') {
			int pos0 = pos;
			while(isalpha(source[pos])) pos++;
			kString *pattern = KLIB new_kString(kctx, 0, source, (pos0-1), 0);
			kString *option  = KLIB new_kString(kctx, 0, source+pos0, pos-pos0, 0);

			kNameSpace *ns = kNode_ns(expr);
			tk->symbol = KSymbol_("create");
			kNode *arg0 = new_ConstNode(kctx, ns, NULL, KLIB Knull(kctx, KClass_RegExp));
			kNode *arg1 = new_ConstNode(kctx, ns, NULL, UPCAST(pattern));
			kNode *arg2 = new_ConstNode(kctx, ns, NULL, UPCAST(option));
			SUGAR kNode_Op(kctx, expr, tk, 3, arg0, arg1, arg2);
			returnIdx = beginIdx+1;
			break;
		}
		prev = ch;
	}
	KDump(expr);
	RESET_GCSTACK();
	KReturnUnboxValue(returnIdx);
}

static kbool_t regexp_defineSyntax(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)
{
	KDEFINE_SYNTAX SYNTAX[] = {
		{ KSymbol_("$RegExp"), SYNFLAG_CParseFunc|SYNFLAG_CTokenFunc, 0, 0, {SUGARFUNC Expression_RegExp}, {SUGAR methodTypeFunc }, KonohaChar_Slash, {SUGARFUNC TokenFunc_JavaScriptRegExp}},
		{ KSymbol_END, },
	};
	SUGAR kNameSpace_DefineSyntax(kctx, ns, SYNTAX, trace);
	return true;
}

static kbool_t regexp_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequireKonohaCommonModule(trace);
	regexp_defineMethod(kctx, ns, trace);
	regexp_defineSyntax(kctx, ns, trace);
	return true;
}

static kbool_t regexp_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *Regexp_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "JavaScript", "1.4");
	d.PackupNameSpace = regexp_PackupNameSpace;
	d.ExportNameSpace = regexp_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
