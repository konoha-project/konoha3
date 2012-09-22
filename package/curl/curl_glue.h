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

#ifndef CURL_GLUE_H_
#define CURL_GLUE_H_

#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern struct KonohaContextVar *ctx;

#define MAX_STR 12

#define S_tobytes(s) (char *)s->buf

typedef const struct _kCurl kCurl;
struct _kCurl {
	KonohaObjectHeader h;
	struct curl_slist *headers;
	FILE* fp;
	CURL *curl;
};

/* ======================================================================== */
// [private functions]

typedef struct curl_context_t {
	struct KonohaContextVar *ctx;
	union {
		kStringVar *string;
		struct _kBytes *bytes;
	};
} curl_context_t;

#define toCurlContext(context) ((curl_context_t *)context)

/*String call back*/
static size_t write_String(char *buffer, size_t size, size_t nitems, void *string)
{
	KonohaContext *kctx = (KonohaContext *)ctx;
	KUtilsWriteBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	kStringVar *res = (kStringVar *)string;
	kString *str;
	char *buf = S_tobytes(res);
	if(buf) {
		KLIB Kwb_write(kctx, &wb, buf, strlen(buf));
	}
	size *= nitems;
	KLIB Kwb_write(kctx, &wb, buffer, size);
	str = KLIB new_kString(kctx, KLIB Kwb_top(kctx, &wb, 0), Kwb_bytesize(&wb), SPOL_POOL);
//TODO handle SPOL and inline_text
	res->ubuf = str->ubuf;
	res->bytesize = str->bytesize;
	KLIB Kwb_free(&wb);
	return size;
}

/*Bytes call back*/
static size_t write_Bytes(char *buffer, size_t size, size_t nitems, void *bytes)
{
//	KonohaContext_t kctx = (KonohaContext_t)ctx;
//	struct _kBytes *res = (struct _kBytes *)bytes;
//	char *buf = res->buf;
//	res->buf = (char *)KMALLOC(res->bytesize + size);
//	size *= nitems;
//	memcpy(res->buf, (void *)buf, res->bytesize);
//	memcpy(res->buf + res->bytesize, (void *)buffer, size);
//	res->bytesize += size;
//	return size;
	return 0;
}

/* ------------------------------------------------------------------------ */

#define Int_to(T, a)      ((T)a.intValue)
#define String_to(T, a)   ((T)S_text(a.s))
#define toCURL(o)         ((kCurl *)o)->curl

static void Curl_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct _kCurl *c = (struct _kCurl *)o;
	c->headers = NULL;
	c->fp = NULL;
	c->curl = curl_easy_init();
}

static void Curl_free(KonohaContext *kctx, kObject *o)
{
	struct _kCurl *c = (struct _kCurl*)o;
	if(c->curl != NULL) {
		curl_easy_cleanup(c->curl);
		c->curl = NULL;
	}
	if (c->fp != NULL) {
		fclose(c->fp);
	}
}

//## Curl Curl.new();
static KMETHOD Curl_new (KonohaContext *kctx, KonohaStack *sfp)
{
	RETURN_(KLIB new_kObject(kctx, O_ct(sfp[K_RTNIDX].o), 0));
}

//##  void Curl.setOpt(int type, dynamic data);
static KMETHOD Curl_setOpt(KonohaContext *kctx, KonohaStack *sfp)
{
	CURL* curl = toCURL(sfp[0].asObject);
	long curlopt = Int_to(long, sfp[1]);
//	FILE* fp = NULL;
	switch(curlopt) {
	// @FROM http://www.phpmanual.jp/function.curl-setopt.html
	case CURLOPT_AUTOREFERER:
		//		case CURLOPT_BINARYTRANSFER:
	case CURLOPT_COOKIESESSION:
	case CURLOPT_CRLF:
	case CURLOPT_DNS_USE_GLOBAL_CACHE:
	case CURLOPT_FAILONERROR:
	case CURLOPT_FILETIME:
	case CURLOPT_FOLLOWLOCATION:
	case CURLOPT_FORBID_REUSE:
	case CURLOPT_FRESH_CONNECT:
	case CURLOPT_FTP_USE_EPRT:
	case CURLOPT_FTP_USE_EPSV:
	case CURLOPT_FTPAPPEND:
		//		case CURLOPT_FTPASCII:
	case CURLOPT_FTPLISTONLY:
	case CURLOPT_HEADER:
	case CURLOPT_HTTPGET:
	case CURLOPT_HTTPPROXYTUNNEL:
		//		case CURLOPT_MUTE:
	case CURLOPT_NETRC:
	case CURLOPT_NOBODY:
	case CURLOPT_NOPROGRESS: /*	default TRUE */
	case CURLOPT_NOSIGNAL:
	case CURLOPT_POST:
	case CURLOPT_PUT:
		//		case CURLOPT_RETURNTRANSFER:
	case CURLOPT_SSL_VERIFYPEER:
	case CURLOPT_TRANSFERTEXT:
	case CURLOPT_UNRESTRICTED_AUTH:
	case CURLOPT_UPLOAD:
	case CURLOPT_VERBOSE: {
		int boolValue = 1;
		if(IS_NULL(sfp[2].o) ||
				((IS_Boolean(sfp[2].o) && sfp[2].boolValue == 0)) ||
				((IS_Int(sfp[2].o) && Int_to(int, sfp[2]) == 0))) {
			boolValue = 0;
		}
		curl_easy_setopt(curl, curlopt, boolValue);
		break;
	}
	case CURLOPT_BUFFERSIZE:
	case CURLOPT_CLOSEPOLICY:
	case CURLOPT_CONNECTTIMEOUT:
	case CURLOPT_DNS_CACHE_TIMEOUT:
	case CURLOPT_FTPSSLAUTH:
	case CURLOPT_HTTP_VERSION:
	case CURLOPT_HTTPAUTH:
	case CURLAUTH_ANY:
	case CURLAUTH_ANYSAFE:
	case CURLOPT_INFILESIZE:
	case CURLOPT_LOW_SPEED_LIMIT:
	case CURLOPT_LOW_SPEED_TIME:
	case CURLOPT_MAXCONNECTS:
	case CURLOPT_MAXREDIRS:
	case CURLOPT_PORT:
	case CURLOPT_PROXYAUTH:
	case CURLOPT_PROXYPORT:
	case CURLOPT_PROXYTYPE:
	case CURLOPT_RESUME_FROM:
	case CURLOPT_SSL_VERIFYHOST:
	case CURLOPT_SSLVERSION:
	case CURLOPT_TIMECONDITION:
	case CURLOPT_TIMEOUT:
	case CURLOPT_TIMEVALUE: {
		if(IS_Int(sfp[2].o)) {
			curl_easy_setopt(curl, curlopt, Int_to(long, sfp[2]));
		}
		else {
			// TODO ktrace
			// KNH_NTRACE2(ctx, "Curl.setOpt", K_FAILED, KNH_LDATA(LOG_msg("TypeError")));
		}
		break;
	}
	case CURLOPT_CAINFO:
	case CURLOPT_CAPATH:
	case CURLOPT_COOKIE:
	case CURLOPT_COOKIEFILE: /* filename */
	case CURLOPT_COOKIEJAR:
	case CURLOPT_CUSTOMREQUEST:
		//		case CURLOPT_EGBSOCKET:
	case CURLOPT_ENCODING:
	case CURLOPT_FTPPORT:
	case CURLOPT_INTERFACE:
	case CURLOPT_KRB4LEVEL:
	case CURLOPT_POSTFIELDS:
	case CURLOPT_PROXY:
	case CURLOPT_PROXYUSERPWD:
	case CURLOPT_RANDOM_FILE:
	case CURLOPT_RANGE:
	case CURLOPT_REFERER:
	case CURLOPT_SSL_CIPHER_LIST:
	case CURLOPT_SSLCERT:
		//		case CURLOPT_SSLCERTPASSWD:
	case CURLOPT_SSLCERTTYPE:
	case CURLOPT_SSLENGINE:
	case CURLOPT_SSLENGINE_DEFAULT:
	case CURLOPT_SSLKEY:
		//		case CURLOPT_SSLKEYPASSWD:
	case CURLOPT_SSLKEYTYPE:
	case CURLOPT_URL:
	case CURLOPT_USERAGENT:
	case CURLOPT_USERPWD: {
		if(IS_String(sfp[2].o)) {
			curl_easy_setopt(curl, curlopt, String_to(char*,sfp[2]));
		}
		else {
			// TODO ktrace
			// KNH_NTRACE2(ctx, "Curl.setOpt", K_FAILED, KNH_LDATA(LOG_msg("TypeError")));
		}
		break;
	}
	case CURLOPT_FILE:
	case CURLOPT_STDERR:
	case CURLOPT_WRITEHEADER:
////		if(IS_OutputStream(sfp[2].o)) {
////			fp = (FILE*)DP(sfp[2].w)->fio;
////			curl_easy_setopt(curl, curlopt, fp);
////			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
////		}
//		else
		if(IS_String(sfp[2].o)){
			//TODO
			//write result to sfp[2] as String
			curl_easy_setopt(curl, curlopt, (void *)sfp[2].s);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_String);
		}
		else if(IS_Bytes(sfp[2].o)){
//			//TODO
//			//write result to sfp[2] as Bytes
			curl_easy_setopt(curl, curlopt, (void *)sfp[2].ba);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_Bytes);
		}
		else {
			//if(knh_Context_isStrict(ctx)) {
			//KNH_THROW__T(ctx, "Type!!: data must be OutputStream");
			//}
			
			// TODO ktrace
			//KNH_NTRACE2(ctx, "Curl.setOpt", K_FAILED, KNH_LDATA(LOG_msg("TypeError")));
		}
		break;
//				case CURLOPT_WRITEFUNCTION: {
//					if(IS_OutputStream(sfp[2].w)) {
//						curl_write_callback cc = knh_curl_callback;
//						curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cc);
//					}
//					break;
//				}
	case CURLOPT_READDATA:
		if (IS_String(sfp[2].o)) {
			FILE* fp = ((kCurl*)sfp[0].asObject)->fp;
			if ((fp = tmpfile()) == NULL) {
				OLDTRACE_SWITCH_TO_KTrace(_DataFault,   // FIXME
						LogText("Curl.setOpt", "Could not set body CURLOPT.READDATA"),
						LogUint("curlopt", curlopt)
					);
				break;
			}
			fputs(String_to(char*, sfp[2]), fp);
			rewind(fp);
			curl_easy_setopt(curl, CURLOPT_READDATA, fp);
			break;
		}
	default: {
		OLDTRACE_SWITCH_TO_KTrace(_DataFault,   // FIXME
				LogText("Curl.setOpt", "UnsupportedOption"),
				LogUint("curlopt", curlopt)
			  );
		// KNH_NTRACE2(ctx, "Curl.setOpt:UnsupportedOption", K_FAILED, KNH_LDATA(LOG_i("curlopt", curlopt)));
		break;
	}
	}
	RETURNvoid_();
}

//## void Curl.appendHeader();
static KMETHOD Curl_appendHeader(KonohaContext *kctx, KonohaStack *sfp)
{
	struct _kCurl* kcurl = (struct _kCurl*)sfp[0].asObject;
	char *h = String_to(char*,sfp[1]);
	kcurl->headers = curl_slist_append(kcurl->headers, h);
	RETURNvoid_();
}

//## boolean Curl.perform();
static KMETHOD Curl_perform(KonohaContext *kctx, KonohaStack *sfp)
{
	kCurl* kcurl = (kCurl*)sfp[0].asObject;
	CURL* curl = toCURL(sfp[0].asObject);
	if (kcurl->headers) {
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, kcurl->headers);
	}
	CURLcode res = curl_easy_perform(curl);
	if(res != CURLE_OK){
		// TODO ktrace
		// KNH_NTRACE2(ctx, "Curl.perform", K_FAILED, KNH_LDATA(LOG_i("CURLcode", res), LOG_s("error", curl_easy_strerror(res))));
	}
	if (kcurl->fp != NULL) {
		fclose(kcurl->fp);
	}
	RETURNb_((res == CURLE_OK));
}

////## dynamic Curl.getInfo(int type);
static KMETHOD Curl_getInfo(KonohaContext *kctx, KonohaStack *sfp)
{
	CURL* curl = toCURL(sfp[0].asObject);
	char *strptr = NULL;
	long lngptr = 0;
	double dblptr = 0;
	if(curl != NULL) {
		kint_t curlinfo = Int_to(int , sfp[1]);
		switch(curlinfo) {
		case CURLINFO_HEADER_SIZE:
		case CURLINFO_REQUEST_SIZE:
			curl_easy_getinfo(curl, curlinfo, &lngptr);
			// RETURN_(new_Int(ctx, lngptr));
			RETURNi_(lngptr);
			break;
		case CURLINFO_REDIRECT_TIME:
		case CURLINFO_TOTAL_TIME:
		case CURLINFO_NAMELOOKUP_TIME:
		case CURLINFO_CONNECT_TIME:
		case CURLINFO_PRETRANSFER_TIME:
		case CURLINFO_STARTTRANSFER_TIME:
		case CURLINFO_SIZE_UPLOAD:
		case CURLINFO_SIZE_DOWNLOAD:
		case CURLINFO_SPEED_DOWNLOAD:
		case CURLINFO_SPEED_UPLOAD:
			curl_easy_getinfo(curl, curlinfo, &dblptr);
			// RETURN_(new_Float(ctx, dblptr));
			RETURNf_(dblptr);
			break;
		case CURLINFO_EFFECTIVE_URL:
		case CURLINFO_CONTENT_TYPE:
			curl_easy_getinfo(curl, curlinfo, &strptr);
			RETURN_(KLIB new_kString(kctx, strptr, strlen(strptr), 0));
			// RETURN_(new_String(ctx, strptr));
			break;
		default: {
			// TODO ktrace
			// KNH_NTRACE2(ctx, "curl_easy_getinfo", K_FAILED, KNH_LDATA(LOG_i("curlinfo", curlinfo)));
			break;
		}
		}
	}
	RETURN_(K_NULL);
}

/* ------------------------------------------------------------------------ */

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _Im kMethod_Immutable
#define _F(F)   (intptr_t)(F)

#define CT_Curl     cCurl
#define TY_Curl     cCurl->typeId
#define IS_Curl(O)  ((O)->h.ct == CT_Curl)

#define _KVi(T)  #T, TY_int, T

static kbool_t curl_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, kfileline_t pline)
{
	ctx = (struct KonohaContextVar *)kctx;

	KDEFINE_CLASS defCurl = {
		STRUCTNAME(Curl),
		.cflag = kClass_Final,
		.init = Curl_init,
		.free = Curl_free,
	};
	KonohaClass *cCurl = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defCurl, pline);

	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im, _F(Curl_new), TY_Curl, TY_Curl, MN_("new"), 0,
		_Public|_Const|_Im, _F(Curl_setOpt), TY_void, TY_Curl, MN_("setOpt"), 2, TY_int, FN_("type"), TY_Object/*FIXME TY_Dynamic*/, FN_("data"),
		_Public|_Const|_Im, _F(Curl_appendHeader), TY_void, TY_Curl, MN_("appendHeader"), 1, TY_String, FN_("header"),
		_Public|_Const|_Im, _F(Curl_perform), TY_boolean, TY_Curl, MN_("perform"), 0,
		_Public|_Const|_Im, _F(Curl_getInfo), TY_Object/*FIXME TY_Dynamic*/, TY_Curl, MN_("getInfo"), 1, TY_int, FN_("type"),
		DEND,
	};
	KLIB kNameSpace_loadMethodData(kctx, ns, MethodData);

	KDEFINE_INT_CONST IntData[] = {
		{_KVi(CURLOPT_AUTOREFERER)},
		{_KVi(CURLOPT_COOKIESESSION)},
		{_KVi(CURLOPT_CRLF)},
		{_KVi(CURLOPT_DNS_USE_GLOBAL_CACHE)},
		{_KVi(CURLOPT_FAILONERROR)},
		{_KVi(CURLOPT_FILETIME)},
		{_KVi(CURLOPT_FOLLOWLOCATION)},
		{_KVi(CURLOPT_FORBID_REUSE)},
		{_KVi(CURLOPT_FRESH_CONNECT)},
		{_KVi(CURLOPT_FTP_USE_EPRT)},
		{_KVi(CURLOPT_FTP_USE_EPSV)},
		{_KVi(CURLOPT_FTPAPPEND)},
		{_KVi(CURLOPT_FTPLISTONLY)},
		{_KVi(CURLOPT_HEADER)},
		{_KVi(CURLOPT_HTTPGET)},
		{_KVi(CURLOPT_HTTPPROXYTUNNEL)},
		{_KVi(CURLOPT_NETRC)},
		{_KVi(CURLOPT_NOBODY)},
		{_KVi(CURLOPT_NOPROGRESS)},
		{_KVi(CURLOPT_NOSIGNAL)},
		{_KVi(CURLOPT_POST)},
		{_KVi(CURLOPT_PUT)},
		{_KVi(CURLOPT_SSL_VERIFYPEER)},
		{_KVi(CURLOPT_TRANSFERTEXT)},
		{_KVi(CURLOPT_UNRESTRICTED_AUTH)},
		{_KVi(CURLOPT_UPLOAD)},
		{_KVi(CURLOPT_VERBOSE)},
		{_KVi(CURLOPT_BUFFERSIZE)},
		{_KVi(CURLOPT_CLOSEPOLICY)},
		{_KVi(CURLOPT_CONNECTTIMEOUT)},
		{_KVi(CURLOPT_DNS_CACHE_TIMEOUT)},
		{_KVi(CURLOPT_FTPSSLAUTH)},
		{_KVi(CURLOPT_HTTP_VERSION)},
		{_KVi(CURLOPT_HTTPAUTH)},
		{_KVi(CURLAUTH_ANY)},
		{_KVi(CURLAUTH_ANYSAFE)},
		{_KVi(CURLOPT_INFILESIZE)},
		{_KVi(CURLOPT_LOW_SPEED_LIMIT)},
		{_KVi(CURLOPT_LOW_SPEED_TIME)},
		{_KVi(CURLOPT_MAXCONNECTS)},
		{_KVi(CURLOPT_MAXREDIRS)},
		{_KVi(CURLOPT_PORT)},
		{_KVi(CURLOPT_PROXYAUTH)},
		{_KVi(CURLOPT_PROXYPORT)},
		{_KVi(CURLOPT_PROXYTYPE)},
		{_KVi(CURLOPT_RESUME_FROM)},
		{_KVi(CURLOPT_SSL_VERIFYHOST)},
		{_KVi(CURLOPT_SSLVERSION)},
		{_KVi(CURLOPT_TIMECONDITION)},
		{_KVi(CURLOPT_TIMEOUT)},
		{_KVi(CURLOPT_TIMEVALUE)},
		{_KVi(CURLOPT_CAINFO)},
		{_KVi(CURLOPT_CAPATH)},
		{_KVi(CURLOPT_COOKIE)},
		{_KVi(CURLOPT_COOKIEFILE)},
		{_KVi(CURLOPT_COOKIEJAR)},
		{_KVi(CURLOPT_CUSTOMREQUEST)},
		{_KVi(CURLOPT_ENCODING)},
		{_KVi(CURLOPT_FTPPORT)},
		{_KVi(CURLOPT_INTERFACE)},
		{_KVi(CURLOPT_KRB4LEVEL)},
		{_KVi(CURLOPT_POSTFIELDS)},
		{_KVi(CURLOPT_PROXY)},
		{_KVi(CURLOPT_PROXYUSERPWD)},
		{_KVi(CURLOPT_RANDOM_FILE)},
		{_KVi(CURLOPT_RANGE)},
		{_KVi(CURLOPT_REFERER)},
		{_KVi(CURLOPT_SSL_CIPHER_LIST)},
		{_KVi(CURLOPT_SSLCERT)},
		{_KVi(CURLOPT_SSLCERTTYPE)},
		{_KVi(CURLOPT_SSLENGINE)},
		{_KVi(CURLOPT_SSLENGINE_DEFAULT)},
		{_KVi(CURLOPT_SSLKEY)},
		{_KVi(CURLOPT_SSLKEYTYPE)},
		{_KVi(CURLOPT_URL)},
		{_KVi(CURLOPT_USERAGENT)},
		{_KVi(CURLOPT_USERPWD)},
		{_KVi(CURLOPT_FILE)},
		{_KVi(CURLOPT_WRITEDATA)},
		{_KVi(CURLOPT_READDATA)},
		{_KVi(CURLOPT_STDERR)},
		{_KVi(CURLOPT_WRITEHEADER)},
		{_KVi(CURLINFO_HEADER_SIZE)},
		{_KVi(CURLINFO_REQUEST_SIZE)},
		{_KVi(CURLINFO_REDIRECT_TIME)},
		{_KVi(CURLINFO_TOTAL_TIME)},
		{_KVi(CURLINFO_NAMELOOKUP_TIME)},
		{_KVi(CURLINFO_CONNECT_TIME)},
		{_KVi(CURLINFO_PRETRANSFER_TIME)},
		{_KVi(CURLINFO_STARTTRANSFER_TIME)},
		{_KVi(CURLINFO_SIZE_UPLOAD)},
		{_KVi(CURLINFO_SIZE_DOWNLOAD)},
		{_KVi(CURLINFO_SPEED_DOWNLOAD)},
		{_KVi(CURLINFO_SPEED_UPLOAD)},
		{_KVi(CURLINFO_EFFECTIVE_URL)},
		{_KVi(CURLINFO_CONTENT_TYPE)},
		{} // end of const data
	};
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(IntData), pline);
	return true;
}

static kbool_t curl_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, kfileline_t pline)
{
	return true;
}

static kbool_t curl_initNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

static kbool_t curl_setupNameSpace(KonohaContext *kctx, kNameSpace *packageNameSpace, kNameSpace *ns, kfileline_t pline)
{
	return true;
}

#endif /* CURL_GLUE_H_ */

#ifdef __cplusplus
}
#endif

///* ------------------------------------------------------------------------ */
///* StreamDSPI */
//
//typedef struct {
//	CURL *curl;
//	char *buffer;               /* buffer to store cached data*/
//	const char *charset;
//	const char *contenttype;
//	int pos;
//	int buffer_len;             /* currently allocated buffers length */
//	int buffer_pos;             /* end of data in buffer*/
//	int still_running;          /* Is background url fetch still in progress */
//} CURLFILE;
//
///* This global variable was originated in a sample code in libcurl */
//static CURLM *multi_handle = NULL;
//static CURLM *emulti_handle = NULL;
//
//static size_t write_callback(char *buffer, size_t size, size_t nitems, void *userp)
//{
//	char *newbuff;
//	CURLFILE *url = (CURLFILE *)userp;
//	size *= nitems;
//	long rembuff = url->buffer_len - url->buffer_pos; /* remaining space in buffer */
//	if(size > rembuff) {
//		newbuff = realloc(url->buffer, url->buffer_len + (size - rembuff));
//		if(newbuff == NULL) {
//			size = rembuff;
//		}
//		else {
//			url->buffer_len += size - rembuff;
//			url->buffer = newbuff;
//		}
//	}
//	memcpy(&url->buffer[url->buffer_pos], buffer, size);
//	url->buffer_pos += size;
//	if(url->contenttype == NULL){
//		char *type = NULL;
//		curl_easy_getinfo(url->curl, CURLInfoTagCONTENT_TYPE, &type);
//		if(type != NULL){
//			char *charset = NULL;
//			charset = strrchr(type, '=');
//			if(charset != NULL){
//				charset++;
//				type = strtok(type, ";");
//			}
//			url->charset = (const char*)charset;
//			url->contenttype = (const char*)type;
//		}
//	}
//	return size;
//}
//
//static int fill_buffer(CURLFILE *file, int want, int waittime)
//{
//	fd_set fdread;
//	fd_set fdwrite;
//	fd_set fdexcep;
//	struct timeval timeout;
//	int rc;
//	/* only attempt to fill buffer if transactions still running and buffer
//	 * doesnt exceed required size already
//	 */
//	if((!file->still_running) || (file->buffer_pos > want))
//		return 0;
//
//	/* attempt to fill buffer */
//	do {
//		int maxfd = -1;
//		FD_ZERO(&fdread);
//		FD_ZERO(&fdwrite);
//		FD_ZERO(&fdexcep);
//
//		/* set a suitable timeout to fail on */
//		timeout.tv_sec = 60; /* 1 minute */
//		timeout.tv_usec = 0;
//
//		/* get file descriptors from the transfers */
//		curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
//
//		/* In a real-world program you OF COURSE check the return code of the
//           function calls.  On success, the value of maxfd is guaranteed to be
//           greater or equal than -1.  We call select(maxfd + 1, ...), specially
//           in case of (maxfd == -1), we call select(0, ...), which is basically
//           equal to sleep. */
//
//		rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
//		switch(rc) {
//		case -1: /* select error */
//		case 0: break;
//		default:
//			/* timeout or readable/writable sockets */
//			/* note we *could* be more efficient and not wait for
//			 * CURLM_CALL_MULTI_PERFORM to clear here and check it on re-entry
//			 * but that gets messy */
//			while(curl_multi_perform(multi_handle, &file->still_running) == CURLM_CALL_MULTI_PERFORM);
//			break;
//		}
//	} while(file->still_running && (file->buffer_pos < want));
//	return 1;
//}
//
///* use to remove want bytes from the front of a files buffer */
//static int use_buffer(CURLFILE *file, int want)
//{
//	/* sort out buffer */
//	if((file->buffer_pos - want) <= 0) {
//		/* ditch buffer - write will recreate */
//		if(file->buffer != NULL) free(file->buffer);
//		file->buffer=NULL;
//		file->buffer_pos=0;
//		file->buffer_len=0;
//	} else { /* move rest down make it available for later */
//		memmove(file->buffer, &file->buffer[want], (file->buffer_pos - want));
//		file->buffer_pos -= want;
//	}
//	return 0;
//}
//
//static int CURL_open(KonohaContext *kctx ctx, kbytes_t path, const char *mode)
//{
//	//TODO mode treats as method
//	CURLFILE *file = malloc(sizeof(CURLFILE));
//	if(file == NULL) return (int)NULL;
//	memset(file, 0, sizeof(CURLFILE));
//	file->curl = curl_easy_init();
//	curl_easy_setopt(file->curl, CURLOPT_URL, path.ubuf);
//	curl_easy_setopt(file->curl, CURLOPT_WRITEDATA, file);
//	curl_easy_setopt(file->curl, CURLOPT_VERBOSE, 0L);
//	curl_easy_setopt(file->curl, CURLOPT_WRITEFUNCTION, write_callback);
//	if(!multi_handle) multi_handle = curl_multi_init();
//	curl_multi_add_handle(multi_handle, file->curl);
//
//	/* lets start the fetch */
//	while(curl_multi_perform(multi_handle, &file->still_running) == CURLM_CALL_MULTI_PERFORM );
//
//	if((file->buffer_pos == 0) && (!file->still_running)) {
//		/* if still_running is 0 now, we should return NULL */
//		/* make sure the easy handle is not in the multi handle anymore */
//		curl_multi_remove_handle(multi_handle, file->curl);
//		/* cleanup */
//		curl_easy_cleanup(file->curl);
//		free(file);
//		file = NULL;
//	}
//	return (int)file;
//}
//
////new OutputStream(http://...)
//static int CURL_init(KonohaContext *kctx ctx, kbytes_t path, const char *mode)
//{
//	return (int)NULL;
//}
//
//static void CURL_close(KonohaContext *kctx ctx, int fd)
//{
//	CURLFILE *file = (CURLFILE*)fd;
//	curl_multi_remove_handle(multi_handle, file->curl);
//	curl_easy_cleanup(file->curl);
//	if(file->buffer) free(file->buffer);
//	free(file);
//}
//
//static int url_feof(CURLFILE *file)
//{
//	int ret=0;
//	if((file->buffer_pos == 0) && (!file->still_running))
//		ret = 1;
//	return ret;
//}
//
//static kintptr_t CURL_read(KonohaContext *kctx ctx, int fd, char *buf, size_t bufsiz)
//{
//	CURLFILE *file = (CURLFILE*)fd;
//	if(file == NULL) return 0;
//	fill_buffer(file, bufsiz, 1);
//	if(!file->buffer_pos) return 0;
//	/* ensure only available data is considered */
//	if(file->buffer_pos < bufsiz) bufsiz = file->buffer_pos;
//	/* xfer data to caller */
//	memcpy(buf, file->buffer, bufsiz);
//	use_buffer(file, bufsiz);
//	return bufsiz;
//}
//
//static kintptr_t CURL_write(KonohaContext *kctx ctx, int fd, const char *buf, size_t bufsiz)
//{
//	return 0;
//}
//
//static int CURL_feof(KonohaContext *kctx ctx, int fd)
//{
//	return url_feof((CURLFILE*)fd);
//}
//
//static int pos = 0;
//static int CURL_getc(KonohaContext *kctx ctx, int fd)
//{
//    CURLFILE *file = (CURLFILE*)fd;
//    if(!file->buffer_pos || file->buffer_pos < file->buffer_len) 
//        fill_buffer(file, 1024, 1);
//    return file->buffer[pos++];
//}
//
//static const char* CURL_getContentType(KonohaContext *kctx ctx, int fd)
//{
//	CURLFILE *file = (CURLFILE*)fd;
//	if(file != NULL){
//		if(file->contenttype != NULL){
//			return file->contenttype;
//		}
//	}
//	return NULL;
//}
//
//static const char* CURL_getCharset(KonohaContext *kctx ctx, int fd)
//{
//	CURLFILE *file = (CURLFILE*)fd;
//	if(file != NULL){
//		if(file->charset != NULL){
//			return file->charset;
//		}
//	}
//	return NULL;
//}
//
//static knh_StreamDSPI_t CURLDSPI_STREAM = {
//	K_DSPI_STREAM, "libcurl", 0,
//	CURL_open, CURL_init, CURL_read, CURL_write, CURL_close,
//	CURL_feof, CURL_getc, CURL_getContentType, CURL_getCharset
//};
//
////void url_rewind(CURLFILE *file)
////{
////	curl_multi_remove_handle(multi_handle, file->curl);
////	curl_multi_add_handle(multi_handle, file->curl);
////	if(file->buffer) free(file->buffer);
////	file->buffer=NULL;
////	file->buffer_pos=0;
////	file->buffer_len=0;
////}
//
//static kuintptr_t CURL_exists(KonohaContext *kctx ctx, kbytes_t path, kNameSpace *ns)
//{
//	CURLFILE *file = malloc(sizeof(CURLFILE));
//	if(file == NULL) return (kuintptr_t)0;
//	memset(file, 0, sizeof(CURLFILE));
//	file->curl = curl_easy_init();
//	curl_easy_setopt(file->curl, CURLOPT_URL, path.ubuf);
//	curl_easy_setopt(file->curl, CURLOPT_WRITEDATA, file);
//	curl_easy_setopt(file->curl, CURLOPT_VERBOSE, 0L);
//	curl_easy_setopt(file->curl, CURLOPT_WRITEFUNCTION, write_callback);
//
//	emulti_handle = curl_multi_init();
//	curl_multi_add_handle(emulti_handle, file->curl);
//	
//	/* lets start the fetch */
//	while(curl_multi_perform(emulti_handle, &file->still_running) == CURLM_CALL_MULTI_PERFORM );
//	if((file->buffer_pos == 0) && (!file->still_running)) {
//		/* if still_running is 0 now, we should return NULL */
//		/* make sure the easy handle is not in the multi handle anymore */
//		curl_multi_remove_handle(emulti_handle, file->curl);
//		/* cleanup */
//		curl_easy_cleanup(file->curl);
//		free(file);
//		emulti_handle = NULL;
//		file = NULL;
//		return (kuintptr_t)0;
//	}
//	emulti_handle = NULL;
//	free(file);
//    return (kuintptr_t)1;
//}
//
//static kbool_t CURL_isTyped(KonohaContext *kctx ctx, KonohaClass cid)
//{
//	return PATH_isTyped(cid);
//}
//
//static Object* newObjectNULL(KonohaContext *kctx ctx, KonohaClass cid, kString *s, kNameSpace *n)
//{
//    return (Object*)s;
//}
//
//static knh_PathDSPI_t CURLDSPI_PATH = {
//	K_DSPI_PATH, "libcurl",
//	TY_InputStream, TY_void,
//	CURL_exists, CURL_isTyped, newObjectNULL
//};
