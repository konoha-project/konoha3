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

#include <curl/curl.h>

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/konoha_common.h>
#include <konoha3/import/methoddecl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kCurlVar {
	kObjectHeader h;
	CURL *curl;
	kString *URLInfoNULL;
	struct curl_slist *headers;
} kCurl;

#define LogCurlStrError(res)  LogText("CurlError", curl_easy_strerror(res))
#define LogWrongOption(opt)   LogText("WrongOption", getOptionSymbol(opt))
#define LogURL(curl)          LogText("Url", kCurl_urlInfo(kctx, curl))

static const char *kCurl_urlInfo(KonohaContext *kctx, kCurl *kcurl)
{
	return (kcurl->URLInfoNULL == NULL) ? "Unknown" : kString_text(kcurl->URLInfoNULL);
}

#define CASE_CURLOPT(T) case CURLOPT_##T : return "CURLOPT_"#T

static const char *getOptionSymbol(CURLoption opt)
{
	switch(opt) {
	CASE_CURLOPT(AUTOREFERER);
	//CASE_CURLOPT(BINARYTRANSFER);
	CASE_CURLOPT(COOKIESESSION);
	CASE_CURLOPT(CRLF);
	CASE_CURLOPT(DNS_USE_GLOBAL_CACHE);
	CASE_CURLOPT(FAILONERROR);
	CASE_CURLOPT(FILETIME);
	CASE_CURLOPT(FOLLOWLOCATION);
	CASE_CURLOPT(FORBID_REUSE);
	CASE_CURLOPT(FRESH_CONNECT);
	CASE_CURLOPT(FTP_USE_EPRT);
	CASE_CURLOPT(FTP_USE_EPSV);
	CASE_CURLOPT(FTPAPPEND);
	//CASE_CURLOPT(FTPASCII);
	CASE_CURLOPT(FTPLISTONLY);
	CASE_CURLOPT(HEADER);
	CASE_CURLOPT(HTTPGET);
	CASE_CURLOPT(HTTPPROXYTUNNEL);
	//CASE_CURLOPT(MUTE);
	CASE_CURLOPT(NETRC);
	CASE_CURLOPT(NOBODY);
	CASE_CURLOPT(NOPROGRESS);
	CASE_CURLOPT(NOSIGNAL);
	CASE_CURLOPT(POST);
	CASE_CURLOPT(PUT);
	//CASE_CURLOPT(RETURNTRANSFER);
	CASE_CURLOPT(SSL_VERIFYPEER);
	CASE_CURLOPT(TRANSFERTEXT);
	CASE_CURLOPT(UNRESTRICTED_AUTH);
	CASE_CURLOPT(UPLOAD);
	CASE_CURLOPT(VERBOSE);
	CASE_CURLOPT(BUFFERSIZE);
	CASE_CURLOPT(CLOSEPOLICY);
	CASE_CURLOPT(CONNECTTIMEOUT);
	CASE_CURLOPT(DNS_CACHE_TIMEOUT);
	CASE_CURLOPT(FTPSSLAUTH);
	CASE_CURLOPT(HTTP_VERSION);
	CASE_CURLOPT(HTTPAUTH);
	//case CURLAUTH_ANY:
	//case CURLAUTH_ANYSAFE:
	CASE_CURLOPT(INFILESIZE);
	CASE_CURLOPT(LOW_SPEED_LIMIT);
	CASE_CURLOPT(LOW_SPEED_TIME);
	CASE_CURLOPT(MAXCONNECTS);
	CASE_CURLOPT(MAXREDIRS);
	CASE_CURLOPT(PORT);
	CASE_CURLOPT(PROXYAUTH);
	CASE_CURLOPT(PROXYPORT);
	CASE_CURLOPT(PROXYTYPE);
	CASE_CURLOPT(RESUME_FROM);
	CASE_CURLOPT(SSL_VERIFYHOST);
	CASE_CURLOPT(SSLVERSION);
	CASE_CURLOPT(TIMECONDITION);
	CASE_CURLOPT(TIMEOUT);
	CASE_CURLOPT(TIMEVALUE);

	CASE_CURLOPT(CAINFO);
	CASE_CURLOPT(CAPATH);
	CASE_CURLOPT(COOKIE);
	CASE_CURLOPT(COOKIEFILE);
	CASE_CURLOPT(COOKIEJAR);
	CASE_CURLOPT(CUSTOMREQUEST);
	//CASE_CURLOPT(EGBSOCKET);
	CASE_CURLOPT(ENCODING);
	CASE_CURLOPT(FTPPORT);
	CASE_CURLOPT(INTERFACE);
	CASE_CURLOPT(KRB4LEVEL);
	CASE_CURLOPT(POSTFIELDS);
	CASE_CURLOPT(PROXY);
	CASE_CURLOPT(PROXYUSERPWD);
	CASE_CURLOPT(RANDOM_FILE);
	CASE_CURLOPT(RANGE);
	CASE_CURLOPT(REFERER);
	CASE_CURLOPT(SSL_CIPHER_LIST);
	CASE_CURLOPT(SSLCERT);
	//CASE_CURLOPT(SSLCERTPASSWD);
	CASE_CURLOPT(SSLCERTTYPE);
	CASE_CURLOPT(SSLENGINE);
	CASE_CURLOPT(SSLENGINE_DEFAULT);
	CASE_CURLOPT(SSLKEY);
	//CASE_CURLOPT(SSLKEYPASSWD);
	CASE_CURLOPT(SSLKEYTYPE);
	CASE_CURLOPT(URL);
	CASE_CURLOPT(USERAGENT);
	CASE_CURLOPT(USERPWD);

	CASE_CURLOPT(FILE);
	CASE_CURLOPT(STDERR);
	CASE_CURLOPT(WRITEHEADER);

	CASE_CURLOPT(READDATA);
	default: break;
	}
	return "UnknownOption";
}

/* ------------------------------------------------------------------------ */

#define toCURL(o)         ((kCurl *)o)->curl

static void kCurl_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct kCurlVar *c = (struct kCurlVar *)o;
	c->curl = curl_easy_init();
}

static void kCurl_Free(KonohaContext *kctx, kObject *o)
{
	struct kCurlVar *c = (struct kCurlVar *)o;
	if(c->headers != NULL) {
		curl_slist_free_all(c->headers);
	}
	if(c->curl != NULL) {
		curl_easy_cleanup(c->curl);
		c->curl = NULL;
	}
}

static void kCurl_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	struct kCurlVar *c = (struct kCurlVar *)o;
	KRefTraceNullable(c->URLInfoNULL);
}

/* ------------------------------------------------------------------------ */

//## Curl Curl.new();
static KMETHOD Curl_new(KonohaContext *kctx, KonohaStack *sfp)  // Don't remove
{
	KReturn(sfp[0].asObject);
}

//## void Curl.setOpt(int type, boolean data);
static KMETHOD Curl_SetOptBoolean(KonohaContext *kctx, KonohaStack *sfp)
{
	kCurl* kcurl = (kCurl *)sfp[0].asObject;
	CURLoption curlopt = (CURLoption)sfp[1].intValue;
	switch(curlopt) {
	// @FROM http://www.phpmanual.jp/function.curl-setopt.html
		case CURLOPT_AUTOREFERER:
		//case CURLOPT_BINARYTRANSFER:
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
		//case CURLOPT_FTPASCII:
		case CURLOPT_FTPLISTONLY:
		case CURLOPT_HEADER:
		case CURLOPT_HTTPGET:
		case CURLOPT_HTTPPROXYTUNNEL:
		//case CURLOPT_MUTE:
		case CURLOPT_NETRC:
		case CURLOPT_NOBODY:
		case CURLOPT_NOPROGRESS: /* default TRUE */
		case CURLOPT_NOSIGNAL:
		case CURLOPT_POST:
		case CURLOPT_PUT:
		//case CURLOPT_RETURNTRANSFER:
		case CURLOPT_SSL_VERIFYPEER:
		case CURLOPT_TRANSFERTEXT:
		case CURLOPT_UNRESTRICTED_AUTH:
		case CURLOPT_UPLOAD:
		case CURLOPT_VERBOSE: {
			curl_easy_setopt(kcurl->curl, curlopt, sfp[2].boolValue);
			break;
		}
		default: {
			KMakeTrace(trace, sfp);
			KTraceErrorPoint(trace, SoftwareFault, "curl_easy_setopt", LogWrongOption(curlopt));
		}
	}
}

//## void Curl.setOpt(int type, int data);
static KMETHOD Curl_SetOptInt(KonohaContext *kctx, KonohaStack *sfp)
{
	kCurl* kcurl = (kCurl *)sfp[0].asObject;
	CURLoption curlopt = (CURLoption)sfp[1].intValue;
	switch(curlopt) {
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
		curl_easy_setopt(kcurl->curl, curlopt, sfp[2].intValue);
	}
	default: {
		KMakeTrace(trace, sfp);
		KTraceErrorPoint(trace, SoftwareFault, "curl_easy_setopt", LogWrongOption(curlopt));
		}
	}/*switch*/
	KReturnVoid();
}

//## void Curl.setOpt(int type, String data);
static KMETHOD Curl_SetOptString(KonohaContext *kctx, KonohaStack *sfp)
{
	kCurl* kcurl = (kCurl *)sfp[0].asObject;
	CURLoption curlopt = (CURLoption)sfp[1].intValue;
	switch(curlopt) {
	case CURLOPT_URL: {
		KFieldInit(kcurl, kcurl->URLInfoNULL, sfp[2].asString);
	}

	case CURLOPT_CAINFO:
	case CURLOPT_CAPATH:
	case CURLOPT_COOKIE:
	case CURLOPT_COOKIEFILE: /* filename */
	case CURLOPT_COOKIEJAR:
	case CURLOPT_CUSTOMREQUEST:
	//case CURLOPT_EGBSOCKET:
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
	//case CURLOPT_SSLCERTPASSWD:
	case CURLOPT_SSLCERTTYPE:
	case CURLOPT_SSLENGINE:
	case CURLOPT_SSLENGINE_DEFAULT:
	case CURLOPT_SSLKEY:
	//case CURLOPT_SSLKEYPASSWD:
	case CURLOPT_SSLKEYTYPE:
	case CURLOPT_USERAGENT:
	case CURLOPT_USERPWD: {
		curl_easy_setopt(kcurl->curl, curlopt, kString_text(sfp[2].asString));
		break;
	}
	default: {
		KMakeTrace(trace, sfp);
		KTraceErrorPoint(trace, SoftwareFault, "curl_easy_setopt", LogWrongOption(curlopt));
		}
	}/*switch*/
	KReturnVoid();
}

//## void Curl.setOpt(int type, FILE data);
static KMETHOD Curl_SetOptFile(KonohaContext *kctx, KonohaStack *sfp)
{
	kCurl* kcurl = (kCurl *)sfp[0].asObject;
	CURLoption curlopt = (CURLoption)sfp[1].intValue;
	switch(curlopt) {
	case CURLOPT_FILE:
	case CURLOPT_STDERR:
	case CURLOPT_WRITEHEADER: {
		kFile *file = (kFile *)sfp[2].asObject;
		curl_easy_setopt(kcurl->curl, curlopt, file->fp);
	}
	default: {
		KMakeTrace(trace, sfp);
		KTraceErrorPoint(trace, SoftwareFault, "curl_easy_setopt", LogWrongOption(curlopt));
		}
	}/*switch*/
	KReturnVoid();
}

//static KMETHOD Curl_SetOptFunc(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kCurl* kcurl = (kCurl *)sfp[0].asObject;
//	CURLoption curlopt = (CURLoption)sfp[1].intValue;
//	switch(curlopt) {
//		case CURLOPT_WRITEFUNCTION: {
//		kFile *file = (kFile *)sfp[2].asObject;
//		curl_easy_setopt(kcurl->curl, curlopt, file->fp);
//	}
//	default: {
//		KMakeTrace(trace, sfp);
//		KTraceErrorPoint(trace, SoftwareFault, "curl_easy_setopt", LogWrongOption(curlopt));
//		}
//	}/*switch*/
//	KReturnVoid();
//}

//		break;
//	//case CURLOPT_WRITEFUNCTION: {
//	//	if(IS_OutputStream(sfp[2].w)) {
//	//		curl_Write_callback cc = knh_curl_callback;
//	//		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cc);
//	//	}
//	//	break;
//	//}
//	case CURLOPT_READDATA:
//		if(IS_String(sfp[2].asObject)) {
//			FILE* fp = ((kCurl *)sfp[0].asObject)->fp;
//			if((fp = tmpfile()) == NULL) {
//				OLDTRACE_SWITCH_TO_KTrace(_UserFault,   // FIXME
//						LogText("Curl.setOpt", "Could not set body CURLOPT.READDATA"),
//						LogUint("curlopt", curlopt)
//					);
//				break;
//			}
//			fputs(String_to(char*, sfp[2]), fp);
//			rewind(fp);
//			curl_easy_setopt(curl, CURLOPT_READDATA, fp);
//			break;
//		}
//	default: {
//		OLDTRACE_SWITCH_TO_KTrace(_UserFault,   // FIXME
//				LogText("Curl.setOpt", "UnsupportedOption"),
//				LogUint("curlopt", curlopt)
//			  );
//		//KNH_NTRACE2(ctx, "Curl.setOpt:UnsupportedOption", K_FAILED, KNH_LDATA(LOG_i("curlopt", curlopt)));
//		break;
//	}
//	}
//	KReturnVoid();
//}

//## void Curl.appendHeader(String headers);
static KMETHOD Curl_appendHeader(KonohaContext *kctx, KonohaStack *sfp)
{
	kCurl* kcurl = (kCurl *)sfp[0].asObject;

	const char *h = kString_text(sfp[1].asString);
	kcurl->headers = curl_slist_append(kcurl->headers, h);
	KReturnVoid();
}

static int diagnosisCurlFaultType(KonohaContext *kctx, CURLcode res, int UserFault)
{
	switch(res) {
	case CURLE_OK:
		return 0;
	case CURLE_UNSUPPORTED_PROTOCOL:    /* 1 */
		return UserFault | SoftwareFault;
	case CURLE_FAILED_INIT:             /* 2 */
		return UserFault | SystemFault;
	case CURLE_URL_MALFORMAT:           /* 3 */
		return UserFault | SoftwareFault;
	//case CURLE_OBSOLETE4:               /* 4 - NOT USED */
	case CURLE_COULDNT_RESOLVE_PROXY:   /* 5 */
	case CURLE_COULDNT_RESOLVE_HOST:    /* 6 */
		return UserFault | SoftwareFault | ExternalFault;

	case CURLE_COULDNT_CONNECT:         /* 7 */
		return UserFault | SoftwareFault | SystemFault | ExternalFault;

	case CURLE_FTP_WEIRD_SERVER_REPLY:  /* 8 */
#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_REMOTE_ACCESS_DENIED:    /* 9 a service was denied by the server
	                                    due to lack of access - when login fails
	                                    this is not returned. */
#else
	case CURLE_FTP_ACCESS_DENIED:
#endif
		return UserFault | SoftwareFault | ExternalFault;
#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_OBSOLETE10:              /* 10 - NOT USED */
#else
	case CURLE_FTP_USER_PASSWORD_INCORRECT:
#endif
	case CURLE_FTP_WEIRD_PASS_REPLY:    /* 11 */
#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_OBSOLETE12:              /* 12 - NOT USED */
#else
	case CURLE_FTP_WEIRD_USER_REPLY:
#endif
	case CURLE_FTP_WEIRD_PASV_REPLY:    /* 13 */
	case CURLE_FTP_WEIRD_227_FORMAT:    /* 14 */
	case CURLE_FTP_CANT_GET_HOST:       /* 15 */
#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_OBSOLETE16:              /* 16 - NOT USED */
#else
	case CURLE_FTP_CANT_RECONNECT:
#endif

#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_FTP_COULDNT_SET_TYPE:    /* 17 */
#else
	case CURLE_FTP_COULDNT_SET_BINARY:
#endif
	case CURLE_PARTIAL_FILE:            /* 18 */
	case CURLE_FTP_COULDNT_RETR_FILE:   /* 19 */
		return ExternalFault;

#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_OBSOLETE20:              /* 20 - NOT USED */
#else
	case CURLE_FTP_WRITE_ERROR:
#endif

#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_QUOTE_ERROR:             /* 21 - quote command failure */
#else
	case CURLE_FTP_QUOTE_ERROR:
#endif
		return UserFault | SoftwareFault | ExternalFault;

	case CURLE_HTTP_RETURNED_ERROR:     /* 22 */
		return UserFault | SoftwareFault ;

	case CURLE_WRITE_ERROR:             /* 23 */

#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_OBSOLETE24:              /* 24 - NOT USED */
#else
	case CURLE_MALFORMAT_USER:
#endif

#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_UPLOAD_FAILED:           /* 25 - failed upload "command" */
#else
	case CURLE_FTP_COULDNT_STOR_FILE:
#endif
	case CURLE_READ_ERROR:              /* 26 - couldn't open/read from file */
	case CURLE_OUT_OF_MEMORY:           /* 27 */
		/* Note: case CURLE_OUT_OF_MEMORY may sometimes indicate a conversion error
	           instead of a memory allocation error if CURL_DOES_CONVERSIONS
	           is defined
		 */
		return UserFault | SoftwareFault | SystemFault;

	case CURLE_OPERATION_TIMEDOUT:      /* 28 - the timeout time was reached */
		return UserFault | SoftwareFault | SystemFault | ExternalFault;

#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_OBSOLETE29:              /* 29 - NOT USED */
#else
	case CURLE_FTP_COULDNT_SET_ASCII:
#endif
	case CURLE_FTP_PORT_FAILED:         /* 30 - FTP PORT operation failed */
		return UserFault | SoftwareFault;

	case CURLE_FTP_COULDNT_USE_REST:    /* 31 - the REST command failed */
		return UserFault | SoftwareFault ;

#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_OBSOLETE32:              /* 32 - NOT USED */
#else
	case CURLE_FTP_COULDNT_GET_SIZE:
#endif

#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_RANGE_ERROR:             /* 33 - RANGE "command" didn't work */
#else
	case CURLE_HTTP_RANGE_ERROR:
#endif
		return UserFault | ExternalFault ;

	case CURLE_HTTP_POST_ERROR:         /* 34 */
		return UserFault | SystemFault ;

	case CURLE_SSL_CONNECT_ERROR:       /* 35 - wrong when connecting with SSL */
		return UserFault | SoftwareFault | ExternalFault;

	case CURLE_BAD_DOWNLOAD_RESUME:     /* 36 - couldn't resume download */
		return UserFault | SoftwareFault | SystemFault | ExternalFault;

	case CURLE_FILE_COULDNT_READ_FILE:  /* 37 */
		return UserFault | SoftwareFault | SystemFault;

	case CURLE_LDAP_CANNOT_BIND:        /* 38 */
	case CURLE_LDAP_SEARCH_FAILED:      /* 39 */
		return UserFault | ExternalFault;

#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_OBSOLETE40:              /* 40 - NOT USED */
#else
	case CURLE_LIBRARY_NOT_FOUND:
#endif
	case CURLE_FUNCTION_NOT_FOUND:      /* 41 */
	case CURLE_ABORTED_BY_CALLBACK:     /* 42 */
		return UserFault | SystemFault;

	case CURLE_BAD_FUNCTION_ARGUMENT:   /* 43 */
		return UserFault | SoftwareFault | SystemFault;

#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_OBSOLETE44:              /* 44 - NOT USED */
#else
	case CURLE_BAD_CALLING_ORDER:
#endif

	case CURLE_INTERFACE_FAILED:        /* 45 - CURLOPT_INTERFACE failed */
		return UserFault | SoftwareFault | SystemFault;

#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_OBSOLETE46:              /* 46 - NOT USED */
#else
	case CURLE_BAD_PASSWORD_ENTERED:
#endif

	case CURLE_TOO_MANY_REDIRECTS :     /* 47 - catch endless re-direct loops */
		return ExternalFault;

	case CURLE_UNKNOWN_TELNET_OPTION:   /* 48 - User specified an unknown option */
	case CURLE_TELNET_OPTION_SYNTAX :   /* 49 - Malformed telnet option */
#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_OBSOLETE50:              /* 50 - NOT USED */
#else
	case CURLE_OBSOLETE:
#endif

		return UserFault | SoftwareFault;

#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_PEER_FAILED_VERIFICATION: /* 51 - peer's certificate or fingerprint
	                                     wasn't verified fine */
#else
	case CURLE_SSL_PEER_CERTIFICATE:
#endif
		return SystemFault | ExternalFault;
	case CURLE_GOT_NOTHING:             /* 52 - when this is a specific error */
		return ExternalFault;

	case CURLE_SSL_ENGINE_NOTFOUND:     /* 53 - SSL crypto engine not found */
	case CURLE_SSL_ENGINE_SETFAILED:    /* 54 - can not set SSL crypto engine as
	                                    default */
		return SystemFault;

	case CURLE_SEND_ERROR:              /* 55 - failed sending network data */
	case CURLE_RECV_ERROR:              /* 56 - failure in receiving network data */
#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_OBSOLETE57:              /* 57 - NOT IN USE */
#else
	case CURLE_SHARE_IN_USE:
#endif
		return SystemFault | ExternalFault;

	case CURLE_SSL_CERTPROBLEM:         /* 58 - problem with the local certificate */
	case CURLE_SSL_CIPHER:              /* 59 - couldn't use specified cipher */
	case CURLE_SSL_CACERT:              /* 60 - problem with the CA cert (path?) */
	case CURLE_BAD_CONTENT_ENCODING:    /* 61 - Unrecognized transfer encoding */
		return SystemFault;

	case CURLE_LDAP_INVALID_URL:        /* 62 - Invalid LDAP URL */
		return UserFault | SoftwareFault | SystemFault | ExternalFault;

	case CURLE_FILESIZE_EXCEEDED:       /* 63 - Maximum file size exceeded */
#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_USE_SSL_FAILED:          /* 64 - Requested FTP SSL level failed */
#else
	case CURLE_FTP_SSL_FAILED:
#endif
	case CURLE_SEND_FAIL_REWIND:        /* 65 - Sending the data requires a rewind
	                                    that failed */
		return SystemFault | ExternalFault;
	case CURLE_SSL_ENGINE_INITFAILED:   /* 66 - failed to initialise ENGINE */
		return SystemFault;

	case CURLE_LOGIN_DENIED:            /* 67 - user: password or similar was not
	                                    accepted and we failed to login */
		return UserFault | SoftwareFault | ExternalFault;

	case CURLE_TFTP_NOTFOUND:           /* 68 - file not found on server */
	case CURLE_TFTP_PERM:               /* 69 - permission problem on server */
		return UserFault | SoftwareFault | ExternalFault;
#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_REMOTE_DISK_FULL:        /* 70 - out of disk space on server */
#else
	case CURLE_TFTP_DISKFULL:
#endif
		return ExternalFault;

	case CURLE_TFTP_ILLEGAL:            /* 71 - Illegal TFTP operation */
	case CURLE_TFTP_UNKNOWNID:          /* 72 - Unknown transfer ID */
#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_REMOTE_FILE_EXISTS:      /* 73 - File already exists */
#else
	case CURLE_TFTP_EXISTS:
#endif


	case CURLE_TFTP_NOSUCHUSER:         /* 74 - No such user */
		return UserFault | SoftwareFault | ExternalFault;

	case CURLE_CONV_FAILED:             /* 75 - conversion failed */
	case CURLE_CONV_REQD:               /* 76 - caller must register conversion
	                                    callbacks using curl_easy_setopt options
	                                    CURLOPT_CONV_FROM_NETWORK_FUNCTION:
	                                    CURLOPT_CONV_TO_NETWORK_FUNCTION: and
	                                    CURLOPT_CONV_FROM_UTF8_FUNCTION */
		return SystemFault | ExternalFault;

#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_SSL_CACERT_BADFILE:      /* 77 - could not load CACERT file: missing
	                                    or wrong format */
		return SystemFault;
#endif

#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_REMOTE_FILE_NOT_FOUND:   /* 78 - remote file not found */
		return ExternalFault;
#endif

#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_SSH:                     /* 79 - error from the SSH layer: somewhat
	                                    generic so the error message will be of
	                                    interest when this has happened */
		return SystemFault | ExternalFault;
#endif

#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_SSL_SHUTDOWN_FAILED:     /* 80 - Failed to shut down the SSL
	                                    connection */
		return SystemFault | ExternalFault;
#endif

#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_AGAIN:                   /* 81 - socket is not ready for send/recv:
	                                    wait till it's ready and try again (Added
	                                    in 7.18.2) */
		return SystemFault | ExternalFault;
#endif
	case CURLE_SSL_CRL_BADFILE:         /* 82 - could not load CRL file: missing or
	                                    wrong format (Added in 7.19.0) */
		return SystemFault;
#if (LIBCURL_VERSION_NUM >= 0x071000) /* curl_version >= 7.16.0 */
	case CURLE_SSL_ISSUER_ERROR:        /* 83 - Issuer check failed.  (Added in
	                                    7.19.0) */
		return SystemFault | ExternalFault;
#endif
#if LIBCURL_VERSION_MAJOR >= 7 && LIBCURL_VERSION_MINOR >= 20
	case CURLE_FTP_PRET_FAILED:         /* 84 - a PRET command failed */
	case CURLE_RTSP_CSEQ_ERROR:         /* 85 - mismatch of RTSP CSeq numbers */
	case CURLE_RTSP_SESSION_ERROR:      /* 86 - mismatch of RTSP Session Identifiers */
	case CURLE_FTP_BAD_FILE_LIST:       /* 87 - unable to parse FTP file list */
	case CURLE_CHUNK_FAILED:            /* 88 - chunk callback reported error */
#endif
		return SystemFault | ExternalFault;
	case CURL_LAST:
	default:
		break;
	}
	return UserFault | SystemFault| SoftwareFault | ExternalFault;
}

//## boolean Curl.perform();
static KMETHOD Curl_perform(KonohaContext *kctx, KonohaStack *sfp)
{
	kCurl* kcurl = (kCurl *)sfp[0].asObject;
	KMakeTrace(trace, sfp);
	CURLcode res;
	if(kcurl->headers != NULL) {
		curl_easy_setopt(kcurl->curl, CURLOPT_HTTPHEADER, kcurl->headers);
	}
	KTraceResponseCheckPoint(trace, 0, "curl_easy_perform",
		res = curl_easy_perform(kcurl->curl)
	);
	if(res != CURLE_OK){
		int fault = diagnosisCurlFaultType(kctx, res, (kcurl->URLInfoNULL == NULL) ? 0 : kString_GuessUserFault(kcurl->URLInfoNULL));
		KTraceErrorPoint(trace, fault, "curl_easy_perform", LogURL(kcurl), LogCurlStrError(res));
	}
	KReturnUnboxValue((res == CURLE_OK));
}

// writedata for Curl.receiveString()
struct ReceiveBuffer {
	KonohaContext *kctx;
	KBuffer wb;
};

// writefunction for Curl.receiveString()
static size_t writeToBuffer(void *buffer, size_t size, size_t nmemb, void *obj)
{
	struct ReceiveBuffer *rbuf = (struct ReceiveBuffer *)obj;
	KonohaContext *kctx = rbuf->kctx;
	size_t writeSize = size * nmemb;
	KLIB KBuffer_Write(kctx, &rbuf->wb, (char *)buffer, writeSize);
	return writeSize;
}

//## String Curl.receiveString();
static KMETHOD Curl_receiveString(KonohaContext *kctx, KonohaStack *sfp)
{
	kCurl* kcurl = (kCurl *)sfp[0].asObject;

	/* presets */
	struct ReceiveBuffer rbuf = {0};
	rbuf.kctx = kctx;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &rbuf.wb);
	curl_easy_setopt(kcurl->curl, CURLOPT_WRITEFUNCTION, writeToBuffer);
	curl_easy_setopt(kcurl->curl, CURLOPT_WRITEDATA, &rbuf);

	/* perform */
	KMakeTrace(trace, sfp);
	CURLcode res;
	if(kcurl->headers != NULL) {
		curl_easy_setopt(kcurl->curl, CURLOPT_HTTPHEADER, kcurl->headers);
	}
	KTraceResponseCheckPoint(trace, 0, "curl_easy_perform",
		res = curl_easy_perform(kcurl->curl)
	);
	if(res != CURLE_OK) {
		int fault = diagnosisCurlFaultType(kctx, res, (kcurl->URLInfoNULL == NULL) ? 0 : kString_GuessUserFault(kcurl->URLInfoNULL));
		KTraceErrorPoint(trace, fault, "curl_easy_perform", LogURL(kcurl), LogCurlStrError(res));
	}
	KReturn(KLIB KBuffer_Stringfy(rbuf.kctx, &rbuf.wb, OnStack, StringPolicy_FreeKBuffer));
}

////## dynamic Curl.getInfo(int type);
static KMETHOD Curl_getInfo(KonohaContext *kctx, KonohaStack *sfp)
{
	CURL* curl = toCURL(sfp[0].asObject);
	char *strptr = NULL;
	long lngptr = 0;
	double dblptr = 0;
	if(curl != NULL) {
		kint_t curlinfo = sfp[1].intValue;
		switch(curlinfo) {
		case CURLINFO_HEADER_SIZE:
		case CURLINFO_REQUEST_SIZE:
			curl_easy_getinfo(curl, curlinfo, &lngptr);
			KReturnUnboxValue(lngptr);
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
			KReturnFloatValue(dblptr);
			break;
		case CURLINFO_EFFECTIVE_URL:
		case CURLINFO_CONTENT_TYPE:
			curl_easy_getinfo(curl, curlinfo, &strptr);
			KReturn(KLIB new_kString(kctx, OnStack, strptr, strlen(strptr), 0));
			break;
		default: {
			// TODO ktrace
			// KNH_NTRACE2(ctx, "curl_easy_getinfo", K_FAILED, KNH_LDATA(LOG_i("curlinfo", curlinfo)));
			break;
		}
		}
	}
	KReturn(K_NULL);
}

/* ------------------------------------------------------------------------ */

#define KType_Curl     cCurl->typeId

#define KDefineConstInt(T)  #T, KType_Int, T

static kbool_t curl_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequireKonohaCommonModule(trace);
	KRequirePackage("Type.File", trace);

	KDEFINE_CLASS defCurl = {
		STRUCTNAME(Curl),
		.cflag = KClassFlag_Final,
		.init = kCurl_Init,
		.reftrace = kCurl_Reftrace,
		.free = kCurl_Free,
	};
	KClass *cCurl = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defCurl, trace);

	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Curl_new), KType_Curl, KType_Curl, KMethodName_("new"), 0,
		_Public, _F(Curl_SetOptBoolean), KType_void, KType_Curl, KMethodName_("setOpt"), 2, KType_Int, KFieldName_("option"), KType_Boolean, KFieldName_("data"),
		_Public, _F(Curl_SetOptInt),     KType_void, KType_Curl, KMethodName_("setOpt"), 2, KType_Int, KFieldName_("option"), KType_Int,     KFieldName_("data"),
		_Public, _F(Curl_SetOptString),  KType_void, KType_Curl, KMethodName_("setOpt"), 2, KType_Int, KFieldName_("option"), KType_String,  KFieldName_("data"),
		_Public, _F(Curl_SetOptFile),    KType_void, KType_Curl, KMethodName_("setOpt"), 2, KType_Int, KFieldName_("option"), KType_File,    KFieldName_("data"),
		_Public, _F(Curl_appendHeader), KType_void, KType_Curl, KMethodName_("appendHeader"), 1, KType_String, KFieldName_("header"),
		_Public, _F(Curl_perform), KType_Boolean, KType_Curl, KMethodName_("perform"), 0,
		_Public, _F(Curl_receiveString), KType_String, KType_Curl, KMethodName_("receiveString"), 0,
		_Public|_Im, _F(Curl_getInfo), KType_Object/*FIXME KType_Dynamic*/, KType_Curl, KMethodName_("getInfo"), 1, KType_Int, KFieldName_("type"),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	KDEFINE_INT_CONST IntData[] = {
		{KDefineConstInt(CURLOPT_AUTOREFERER)},
		{KDefineConstInt(CURLOPT_COOKIESESSION)},
		{KDefineConstInt(CURLOPT_CRLF)},
		{KDefineConstInt(CURLOPT_DNS_USE_GLOBAL_CACHE)},
		{KDefineConstInt(CURLOPT_FAILONERROR)},
		{KDefineConstInt(CURLOPT_FILETIME)},
		{KDefineConstInt(CURLOPT_FOLLOWLOCATION)},
		{KDefineConstInt(CURLOPT_FORBID_REUSE)},
		{KDefineConstInt(CURLOPT_FRESH_CONNECT)},
		{KDefineConstInt(CURLOPT_FTP_USE_EPRT)},
		{KDefineConstInt(CURLOPT_FTP_USE_EPSV)},
		{KDefineConstInt(CURLOPT_FTPAPPEND)},
		{KDefineConstInt(CURLOPT_FTPLISTONLY)},
		{KDefineConstInt(CURLOPT_HEADER)},
		{KDefineConstInt(CURLOPT_HTTPGET)},
		{KDefineConstInt(CURLOPT_HTTPPROXYTUNNEL)},
		{KDefineConstInt(CURLOPT_NETRC)},
		{KDefineConstInt(CURLOPT_NOBODY)},
		{KDefineConstInt(CURLOPT_NOPROGRESS)},
		{KDefineConstInt(CURLOPT_NOSIGNAL)},
		{KDefineConstInt(CURLOPT_POST)},
		{KDefineConstInt(CURLOPT_PUT)},
		{KDefineConstInt(CURLOPT_SSL_VERIFYPEER)},
		{KDefineConstInt(CURLOPT_TRANSFERTEXT)},
		{KDefineConstInt(CURLOPT_UNRESTRICTED_AUTH)},
		{KDefineConstInt(CURLOPT_UPLOAD)},
		{KDefineConstInt(CURLOPT_VERBOSE)},
		{KDefineConstInt(CURLOPT_BUFFERSIZE)},
		{KDefineConstInt(CURLOPT_CLOSEPOLICY)},
		{KDefineConstInt(CURLOPT_CONNECTTIMEOUT)},
		{KDefineConstInt(CURLOPT_DNS_CACHE_TIMEOUT)},
		{KDefineConstInt(CURLOPT_FTPSSLAUTH)},
		{KDefineConstInt(CURLOPT_HTTP_VERSION)},
		{KDefineConstInt(CURLOPT_HTTPAUTH)},
		{KDefineConstInt(CURLAUTH_ANY)},
		{KDefineConstInt(CURLAUTH_ANYSAFE)},
		{KDefineConstInt(CURLOPT_INFILESIZE)},
		{KDefineConstInt(CURLOPT_LOW_SPEED_LIMIT)},
		{KDefineConstInt(CURLOPT_LOW_SPEED_TIME)},
		{KDefineConstInt(CURLOPT_MAXCONNECTS)},
		{KDefineConstInt(CURLOPT_MAXREDIRS)},
		{KDefineConstInt(CURLOPT_PORT)},
		{KDefineConstInt(CURLOPT_PROXYAUTH)},
		{KDefineConstInt(CURLOPT_PROXYPORT)},
		{KDefineConstInt(CURLOPT_PROXYTYPE)},
		{KDefineConstInt(CURLOPT_RESUME_FROM)},
		{KDefineConstInt(CURLOPT_SSL_VERIFYHOST)},
		{KDefineConstInt(CURLOPT_SSLVERSION)},
		{KDefineConstInt(CURLOPT_TIMECONDITION)},
		{KDefineConstInt(CURLOPT_TIMEOUT)},
		{KDefineConstInt(CURLOPT_TIMEVALUE)},
		{KDefineConstInt(CURLOPT_CAINFO)},
		{KDefineConstInt(CURLOPT_CAPATH)},
		{KDefineConstInt(CURLOPT_COOKIE)},
		{KDefineConstInt(CURLOPT_COOKIEFILE)},
		{KDefineConstInt(CURLOPT_COOKIEJAR)},
		{KDefineConstInt(CURLOPT_CUSTOMREQUEST)},
		{KDefineConstInt(CURLOPT_ENCODING)},
		{KDefineConstInt(CURLOPT_FTPPORT)},
		{KDefineConstInt(CURLOPT_INTERFACE)},
		{KDefineConstInt(CURLOPT_KRB4LEVEL)},
		{KDefineConstInt(CURLOPT_POSTFIELDS)},
		{KDefineConstInt(CURLOPT_PROXY)},
		{KDefineConstInt(CURLOPT_PROXYUSERPWD)},
		{KDefineConstInt(CURLOPT_RANDOM_FILE)},
		{KDefineConstInt(CURLOPT_RANGE)},
		{KDefineConstInt(CURLOPT_REFERER)},
		{KDefineConstInt(CURLOPT_SSL_CIPHER_LIST)},
		{KDefineConstInt(CURLOPT_SSLCERT)},
		{KDefineConstInt(CURLOPT_SSLCERTTYPE)},
		{KDefineConstInt(CURLOPT_SSLENGINE)},
		{KDefineConstInt(CURLOPT_SSLENGINE_DEFAULT)},
		{KDefineConstInt(CURLOPT_SSLKEY)},
		{KDefineConstInt(CURLOPT_SSLKEYTYPE)},
		{KDefineConstInt(CURLOPT_URL)},
		{KDefineConstInt(CURLOPT_USERAGENT)},
		{KDefineConstInt(CURLOPT_USERPWD)},
		{KDefineConstInt(CURLOPT_FILE)},
		{KDefineConstInt(CURLOPT_WRITEDATA)},
		{KDefineConstInt(CURLOPT_READDATA)},
		{KDefineConstInt(CURLOPT_STDERR)},
		{KDefineConstInt(CURLOPT_WRITEHEADER)},
		{KDefineConstInt(CURLINFO_HEADER_SIZE)},
		{KDefineConstInt(CURLINFO_REQUEST_SIZE)},
		{KDefineConstInt(CURLINFO_REDIRECT_TIME)},
		{KDefineConstInt(CURLINFO_TOTAL_TIME)},
		{KDefineConstInt(CURLINFO_NAMELOOKUP_TIME)},
		{KDefineConstInt(CURLINFO_CONNECT_TIME)},
		{KDefineConstInt(CURLINFO_PRETRANSFER_TIME)},
		{KDefineConstInt(CURLINFO_STARTTRANSFER_TIME)},
		{KDefineConstInt(CURLINFO_SIZE_UPLOAD)},
		{KDefineConstInt(CURLINFO_SIZE_DOWNLOAD)},
		{KDefineConstInt(CURLINFO_SPEED_DOWNLOAD)},
		{KDefineConstInt(CURLINFO_SPEED_UPLOAD)},
		{KDefineConstInt(CURLINFO_EFFECTIVE_URL)},
		{KDefineConstInt(CURLINFO_CONTENT_TYPE)},
		{} // end of const data
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(IntData), trace);
	return true;
}

static kbool_t curl_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *Curl_Init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("curl", "1.0"),
		.PackupNameSpace    = curl_PackupNameSpace,
		.ExportNameSpace   = curl_ExportNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
