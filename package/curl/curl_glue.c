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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/konoha_common.h>

#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kCurlVar kCurl;

struct kCurlVar {
	KonohaObjectHeader h;
	CURL *curl;
	kString *URLInfoNULL;
//	struct curl_slist *headers;

	/* used by CURLOPT_WRITEFUNCTION */
	KonohaContext *lctx;
	struct kBytesVar *bytesNULL;
};

#define LogCurlStrError(res)  LogText("CurlError", curl_easy_strerror(res))
#define LogWrongOption(opt)   LogText("WrongOption", getOptionSymbol(opt))
#define LogURL(curl)          LogText("Url", kCurl_urlInfo(kctx, curl))

static const char *kCurl_urlInfo(KonohaContext *kctx, kCurl *kcurl)
{
	return (kcurl->URLInfoNULL == NULL) ? "Unknown" : S_text(kcurl->URLInfoNULL);
}

#define CASE_CURLOPT(T) case CURLOPT_##T : return "CURLOPT_"#T

static const char *getOptionSymbol(CURLoption opt)
{
	switch(opt) {
	CASE_CURLOPT(AUTOREFERER);

//		case CURLOPT_AUTOREFERER: return "CURLOPT_AUTOREFERE"
//		//case CURLOPT_BINARYTRANSFER:
//		case CURLOPT_COOKIESESSION:
//		case CURLOPT_CRLF:
//		case CURLOPT_DNS_USE_GLOBAL_CACHE:
//		case CURLOPT_FAILONERROR:
//		case CURLOPT_FILETIME:
//		case CURLOPT_FOLLOWLOCATION:
//		case CURLOPT_FORBID_REUSE:
//		case CURLOPT_FRESH_CONNECT:
//		case CURLOPT_FTP_USE_EPRT:
//		case CURLOPT_FTP_USE_EPSV:
//		case CURLOPT_FTPAPPEND:
//		//case CURLOPT_FTPASCII:
//		case CURLOPT_FTPLISTONLY:
//		case CURLOPT_HEADER:
//		case CURLOPT_HTTPGET:
//		case CURLOPT_HTTPPROXYTUNNEL:
//		//case CURLOPT_MUTE:
//		case CURLOPT_NETRC:
//		case CURLOPT_NOBODY:
//		case CURLOPT_NOPROGRESS: /* default TRUE */
//		case CURLOPT_NOSIGNAL:
//		case CURLOPT_POST:
//		case CURLOPT_PUT:
//		//case CURLOPT_RETURNTRANSFER:
//		case CURLOPT_SSL_VERIFYPEER:
//		case CURLOPT_TRANSFERTEXT:
//		case CURLOPT_UNRESTRICTED_AUTH:
//		case CURLOPT_UPLOAD:
//		case CURLOPT_VERBOSE: {
//	case CURLOPT_BUFFERSIZE:
//	case CURLOPT_CLOSEPOLICY:
//	case CURLOPT_CONNECTTIMEOUT:
//	case CURLOPT_DNS_CACHE_TIMEOUT:
//	case CURLOPT_FTPSSLAUTH:
//	case CURLOPT_HTTP_VERSION:
//	case CURLOPT_HTTPAUTH:
//	case CURLAUTH_ANY:
//	case CURLAUTH_ANYSAFE:
//	case CURLOPT_INFILESIZE:
//	case CURLOPT_LOW_SPEED_LIMIT:
//	case CURLOPT_LOW_SPEED_TIME:
//	case CURLOPT_MAXCONNECTS:
//	case CURLOPT_MAXREDIRS:
//	case CURLOPT_PORT:
//	case CURLOPT_PROXYAUTH:
//	case CURLOPT_PROXYPORT:
//	case CURLOPT_PROXYTYPE:
//	case CURLOPT_RESUME_FROM:
//	case CURLOPT_SSL_VERIFYHOST:
//	case CURLOPT_SSLVERSION:
//	case CURLOPT_TIMECONDITION:
//	case CURLOPT_TIMEOUT:
//	case CURLOPT_TIMEVALUE: {

//	case CURLOPT_CAINFO:
//	case CURLOPT_CAPATH:
//	case CURLOPT_COOKIE:
//	case CURLOPT_COOKIEFILE: /* filename */
//	case CURLOPT_COOKIEJAR:
//	case CURLOPT_CUSTOMREQUEST:
//	//case CURLOPT_EGBSOCKET:
//	case CURLOPT_ENCODING:
//	case CURLOPT_FTPPORT:
//	case CURLOPT_INTERFACE:
//	case CURLOPT_KRB4LEVEL:
//	case CURLOPT_POSTFIELDS:
//	case CURLOPT_PROXY:
//	case CURLOPT_PROXYUSERPWD:
//	case CURLOPT_RANDOM_FILE:
//	case CURLOPT_RANGE:
//	case CURLOPT_REFERER:
//	case CURLOPT_SSL_CIPHER_LIST:
//	case CURLOPT_SSLCERT:
//	//case CURLOPT_SSLCERTPASSWD:
//	case CURLOPT_SSLCERTTYPE:
//	case CURLOPT_SSLENGINE:
//	case CURLOPT_SSLENGINE_DEFAULT:
//	case CURLOPT_SSLKEY:
//	//case CURLOPT_SSLKEYPASSWD:
//	case CURLOPT_SSLKEYTYPE:
//	case CURLOPT_URL:
//	case CURLOPT_USERAGENT:
//	case CURLOPT_USERPWD: {

//	case CURLOPT_FILE:
//	case CURLOPT_STDERR:
//	case CURLOPT_WRITEHEADER:

//	case CURLOPT_READDATA:
	default: break;
	}
	return "UnknownOption";
}

///* Bytes call back */
//static size_t write_Bytes(char *buffer, size_t size, size_t nitems, void *obj)
//{
//	kCurl *curl = (kCurl *) obj;
//	KonohaContext *kctx = curl->lctx;
//	struct kBytesVar *res = (struct kBytesVar *) curl->bytes;
//	char *buf = res->buf;
//	size *= nitems;
//	res->buf = (char *)KMalloc_UNTRACE(res->bytesize + size);
//	if(res->bytesize) {
//		memcpy(res->buf, (void *)buf, res->bytesize);
//		KFree(buf, res->bytesize);
//	}
//	memcpy(res->buf + res->bytesize, (void *)buffer, size);
//	res->bytesize += size;
//	return size;
//}

/* ------------------------------------------------------------------------ */

#define Int_to(T, a)      ((T)a.intValue)
#define String_to(T, a)   ((T)S_text(a.asString))
#define toCURL(o)         ((kCurl *)o)->curl

static void Curl_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct kCurlVar *c = (struct kCurlVar *)o;
	c->curl = curl_easy_init();
	c->lctx = NULL;
	c->bytesNULL = NULL;
}

static void Curl_free(KonohaContext *kctx, kObject *o)
{
	struct kCurlVar *c = (struct kCurlVar *)o;
	if(c->curl != NULL) {
		curl_easy_cleanup(c->curl);
		c->curl = NULL;
	}
}

static void Curl_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	struct kCurlVar *c = (struct kCurlVar *)o;
	KREFTRACEn(c->URLInfoNULL);
	KREFTRACEn(c->bytesNULL);
}

/* ------------------------------------------------------------------------ */

//## Curl Curl.new();
static KMETHOD Curl_new(KonohaContext *kctx, KonohaStack *sfp)  // Don't remove
{
	KReturn(sfp[0].asObject);
}

//##  void Curl.setOpt(int type, boolean data);
static KMETHOD Curl_setOptBoolean(KonohaContext *kctx, KonohaStack *sfp)
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


static KMETHOD Curl_setOptInt(KonohaContext *kctx, KonohaStack *sfp)
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

static KMETHOD Curl_setOptString(KonohaContext *kctx, KonohaStack *sfp)
{
	kCurl* kcurl = (kCurl *)sfp[0].asObject;
	CURLoption curlopt = (CURLoption)sfp[1].intValue;
	switch(curlopt) {
	case CURLOPT_URL: {

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
		curl_easy_setopt(kcurl->curl, curlopt, S_text(sfp[2].asString));
		break;
	}
	default: {
		KMakeTrace(trace, sfp);
		KTraceErrorPoint(trace, SoftwareFault, "curl_easy_setopt", LogWrongOption(curlopt));
		}
	}/*switch*/
	KReturnVoid();
}

static KMETHOD Curl_setOptFile(KonohaContext *kctx, KonohaStack *sfp)
{
	kCurl* kcurl = (kCurl *)sfp[0].asObject;
	CURLoption curlopt = (CURLoption)sfp[1].intValue;
	switch(curlopt) {
	case CURLOPT_FILE:
	case CURLOPT_STDERR:
	case CURLOPT_WRITEHEADER: {
		kFile *file = (kFile*)sfp[2].asObject;
		curl_easy_setopt(kcurl->curl, curlopt, file->fp);
	}
	default: {
		KMakeTrace(trace, sfp);
		KTraceErrorPoint(trace, SoftwareFault, "curl_easy_setopt", LogWrongOption(curlopt));
		}
	}/*switch*/
	KReturnVoid();
}

//static KMETHOD Curl_setOptFunc(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kCurl* kcurl = (kCurl *)sfp[0].asObject;
//	CURLoption curlopt = (CURLoption)sfp[1].intValue;
//	switch(curlopt) {
//		case CURLOPT_WRITEFUNCTION: {
//		kFile *file = (kFile*)sfp[2].asObject;
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
//	//		curl_write_callback cc = knh_curl_callback;
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

////## void Curl.appendHeader();
//static KMETHOD Curl_appendHeader(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kCurl* kcurl = (kCurl *)sfp[0].asObject;
//
//	char *h = String_to(char*,sfp[1]);
//	kcurl->headers = curl_slist_append(kcurl->headers, h);
//	KReturnVoid();
//}

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
	case CURLE_OBSOLETE4:               /* 4 - NOT USED */
	case CURLE_COULDNT_RESOLVE_PROXY:   /* 5 */
	case CURLE_COULDNT_RESOLVE_HOST:    /* 6 */
		return UserFault | SoftwareFault | ExternalFault;

	case CURLE_COULDNT_CONNECT:         /* 7 */
		return UserFault | SoftwareFault | SystemFault | ExternalFault;

	case CURLE_FTP_WEIRD_SERVER_REPLY:  /* 8 */
	case CURLE_REMOTE_ACCESS_DENIED:    /* 9 a service was denied by the server
	                                    due to lack of access - when login fails
	                                    this is not returned. */
		return UserFault | SoftwareFault | ExternalFault;
	case CURLE_OBSOLETE10:              /* 10 - NOT USED */
	case CURLE_FTP_WEIRD_PASS_REPLY:    /* 11 */
	case CURLE_OBSOLETE12:              /* 12 - NOT USED */
	case CURLE_FTP_WEIRD_PASV_REPLY:    /* 13 */
	case CURLE_FTP_WEIRD_227_FORMAT:    /* 14 */
	case CURLE_FTP_CANT_GET_HOST:       /* 15 */
	case CURLE_OBSOLETE16:              /* 16 - NOT USED */
	case CURLE_FTP_COULDNT_SET_TYPE:    /* 17 */
	case CURLE_PARTIAL_FILE:            /* 18 */
	case CURLE_FTP_COULDNT_RETR_FILE:   /* 19 */
		return ExternalFault;

	case CURLE_OBSOLETE20:              /* 20 - NOT USED */
	case CURLE_QUOTE_ERROR:             /* 21 - quote command failure */
		return UserFault | SoftwareFault | ExternalFault;

	case CURLE_HTTP_RETURNED_ERROR:     /* 22 */
		return UserFault | SoftwareFault ;

	case CURLE_WRITE_ERROR:             /* 23 */
	case CURLE_OBSOLETE24:              /* 24 - NOT USED */
	case CURLE_UPLOAD_FAILED:           /* 25 - failed upload "command" */
	case CURLE_READ_ERROR:              /* 26 - couldn't open/read from file */
	case CURLE_OUT_OF_MEMORY:           /* 27 */
		/* Note: case CURLE_OUT_OF_MEMORY may sometimes indicate a conversion error
	           instead of a memory allocation error if CURL_DOES_CONVERSIONS
	           is defined
		 */
		return UserFault | SoftwareFault | SystemFault;

	case CURLE_OPERATION_TIMEDOUT:      /* 28 - the timeout time was reached */
		return UserFault | SoftwareFault | SystemFault | ExternalFault;

	case CURLE_OBSOLETE29:              /* 29 - NOT USED */
	case CURLE_FTP_PORT_FAILED:         /* 30 - FTP PORT operation failed */
		return UserFault | SoftwareFault;

	case CURLE_FTP_COULDNT_USE_REST:    /* 31 - the REST command failed */
		return UserFault | SoftwareFault ;

	case CURLE_OBSOLETE32:              /* 32 - NOT USED */
	case CURLE_RANGE_ERROR:             /* 33 - RANGE "command" didn't work */
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

	case CURLE_OBSOLETE40:              /* 40 - NOT USED */
	case CURLE_FUNCTION_NOT_FOUND:      /* 41 */
	case CURLE_ABORTED_BY_CALLBACK:     /* 42 */
		return UserFault | SystemFault;

	case CURLE_BAD_FUNCTION_ARGUMENT:   /* 43 */
		return UserFault | SoftwareFault | SystemFault;

	case CURLE_OBSOLETE44:              /* 44 - NOT USED */
	case CURLE_INTERFACE_FAILED:        /* 45 - CURLOPT_INTERFACE failed */
		return UserFault | SoftwareFault | SystemFault;

	case CURLE_OBSOLETE46:              /* 46 - NOT USED */
	case CURLE_TOO_MANY_REDIRECTS :     /* 47 - catch endless re-direct loops */
		return ExternalFault;

	case CURLE_UNKNOWN_TELNET_OPTION:   /* 48 - User specified an unknown option */
	case CURLE_TELNET_OPTION_SYNTAX :   /* 49 - Malformed telnet option */
	case CURLE_OBSOLETE50:              /* 50 - NOT USED */
		return UserFault | SoftwareFault;

	case CURLE_PEER_FAILED_VERIFICATION: /* 51 - peer's certificate or fingerprint
	                                     wasn't verified fine */
		return SystemFault | ExternalFault;
	case CURLE_GOT_NOTHING:             /* 52 - when this is a specific error */
		return ExternalFault;

	case CURLE_SSL_ENGINE_NOTFOUND:     /* 53 - SSL crypto engine not found */
	case CURLE_SSL_ENGINE_SETFAILED:    /* 54 - can not set SSL crypto engine as
	                                    default */
		return SystemFault;

	case CURLE_SEND_ERROR:              /* 55 - failed sending network data */
	case CURLE_RECV_ERROR:              /* 56 - failure in receiving network data */
	case CURLE_OBSOLETE57:              /* 57 - NOT IN USE */
		return SystemFault | ExternalFault;

	case CURLE_SSL_CERTPROBLEM:         /* 58 - problem with the local certificate */
	case CURLE_SSL_CIPHER:              /* 59 - couldn't use specified cipher */
	case CURLE_SSL_CACERT:              /* 60 - problem with the CA cert (path?) */
	case CURLE_BAD_CONTENT_ENCODING:    /* 61 - Unrecognized transfer encoding */
		return SystemFault;

	case CURLE_LDAP_INVALID_URL:        /* 62 - Invalid LDAP URL */
		return UserFault | SoftwareFault | SystemFault | ExternalFault;

	case CURLE_FILESIZE_EXCEEDED:       /* 63 - Maximum file size exceeded */
	case CURLE_USE_SSL_FAILED:          /* 64 - Requested FTP SSL level failed */
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
	case CURLE_REMOTE_DISK_FULL:        /* 70 - out of disk space on server */
		return ExternalFault;

	case CURLE_TFTP_ILLEGAL:            /* 71 - Illegal TFTP operation */
	case CURLE_TFTP_UNKNOWNID:          /* 72 - Unknown transfer ID */
	case CURLE_REMOTE_FILE_EXISTS:      /* 73 - File already exists */

	case CURLE_TFTP_NOSUCHUSER:         /* 74 - No such user */
		return UserFault | SoftwareFault | ExternalFault;

	case CURLE_CONV_FAILED:             /* 75 - conversion failed */
	case CURLE_CONV_REQD:               /* 76 - caller must register conversion
	                                    callbacks using curl_easy_setopt options
	                                    CURLOPT_CONV_FROM_NETWORK_FUNCTION:
	                                    CURLOPT_CONV_TO_NETWORK_FUNCTION: and
	                                    CURLOPT_CONV_FROM_UTF8_FUNCTION */
		return SystemFault | ExternalFault;

	case CURLE_SSL_CACERT_BADFILE:      /* 77 - could not load CACERT file: missing
	                                    or wrong format */
		return SystemFault;
	case CURLE_REMOTE_FILE_NOT_FOUND:   /* 78 - remote file not found */
		return ExternalFault;

	case CURLE_SSH:                     /* 79 - error from the SSH layer: somewhat
	                                    generic so the error message will be of
	                                    interest when this has happened */
		return SystemFault | ExternalFault;

	case CURLE_SSL_SHUTDOWN_FAILED:     /* 80 - Failed to shut down the SSL
	                                    connection */
		return SystemFault | ExternalFault;

	case CURLE_AGAIN:                   /* 81 - socket is not ready for send/recv:
	                                    wait till it's ready and try again (Added
	                                    in 7.18.2) */
		return SystemFault | ExternalFault;
	case CURLE_SSL_CRL_BADFILE:         /* 82 - could not load CRL file: missing or
	                                    wrong format (Added in 7.19.0) */
		return SystemFault;
	case CURLE_SSL_ISSUER_ERROR:        /* 83 - Issuer check failed.  (Added in
	                                    7.19.0) */
	case CURLE_FTP_PRET_FAILED:         /* 84 - a PRET command failed */
	case CURLE_RTSP_CSEQ_ERROR:         /* 85 - mismatch of RTSP CSeq numbers */
	case CURLE_RTSP_SESSION_ERROR:      /* 86 - mismatch of RTSP Session Identifiers */
	case CURLE_FTP_BAD_FILE_LIST:       /* 87 - unable to parse FTP file list */
	case CURLE_CHUNK_FAILED:            /* 88 - chunk callback reported error */
		return SystemFault | ExternalFault;
	case CURL_LAST: break;
	}
	return UserFault | SystemFault| SoftwareFault | ExternalFault;
}

//## boolean Curl.perform();
static KMETHOD Curl_perform(KonohaContext *kctx, KonohaStack *sfp)
{
	kCurl* kcurl = (kCurl *)sfp[0].asObject;
	KMakeTrace(trace, sfp);
	CURLcode res;
	KTraceResponseCheckPoint(trace, 0, "curl_easy_perform",
		res = curl_easy_perform(kcurl->curl)
	);
	if(res != CURLE_OK){
		int fault = diagnosisCurlFaultType(kctx, res, (kcurl->URLInfoNULL == NULL) ? 0 : kString_guessUserFault(kcurl->URLInfoNULL));
		KTraceErrorPoint(trace, fault, "curl_easy_perform", LogURL(kcurl), LogCurlStrError(res));
	}
	KReturnUnboxValue((res == CURLE_OK));
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

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Im kMethod_Immutable
#define _F(F)   (intptr_t)(F)

#define TY_Curl     cCurl->typeId

#define _KVi(T)  #T, TY_int, T

static kbool_t curl_initPackage(KonohaContext *kctx, kNameSpace *ns, int argc, const char**args, KTraceInfo *trace)
{
	KRequireKonohaCommonModule(trace);

	KDEFINE_CLASS defCurl = {
		STRUCTNAME(Curl),
		.cflag = kClass_Final,
		.init = Curl_init,
		.reftrace = Curl_reftrace,
		.free = Curl_free,
	};
	KonohaClass *cCurl = KLIB kNameSpace_defineClass(kctx, ns, NULL, &defCurl, trace);

	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Curl_new), TY_Curl, TY_Curl, MN_("new"), 0,
		_Public, _F(Curl_setOptBoolean), TY_void, TY_Curl, MN_("setOpt"), 2, TY_int, FN_("option"), TY_boolean, FN_("data"),
		_Public, _F(Curl_setOptInt),     TY_void, TY_Curl, MN_("setOpt"), 2, TY_int, FN_("option"), TY_int,     FN_("data"),
		_Public, _F(Curl_setOptString),  TY_void, TY_Curl, MN_("setOpt"), 2, TY_int, FN_("option"), TY_String,  FN_("data"),
		_Public, _F(Curl_setOptFile),    TY_void, TY_Curl, MN_("setOpt"), 2, TY_int, FN_("option"), TY_File,    FN_("data"),
//		_Public, _F(Curl_appendHeader), TY_void, TY_Curl, MN_("appendHeader"), 1, TY_String, FN_("header"),
		_Public, _F(Curl_perform), TY_boolean, TY_Curl, MN_("perform"), 0,
		_Public|_Im, _F(Curl_getInfo), TY_Object/*FIXME TY_Dynamic*/, TY_Curl, MN_("getInfo"), 1, TY_int, FN_("type"),
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
	KLIB kNameSpace_loadConstData(kctx, ns, KonohaConst_(IntData), trace);
	return true;
}

static kbool_t curl_setupPackage(KonohaContext *kctx, kNameSpace *ns, isFirstTime_t isFirstTime, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE* curl_init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("curl", "1.0"),
		.initPackage    = curl_initPackage,
		.setupPackage   = curl_setupPackage,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
