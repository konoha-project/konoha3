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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sys/ioctl.h>
//#include <asm/termbits.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <event.h>
#include <evhttp.h>

#include <konoha3/konoha.h>
#include <konoha3/klib.h>

// -------------------------------------------------------------------------
/* Console */

static const char *getThisFileName(KonohaContext *kctx)
{
	static char shell[] = "shell";
	kNameSpace *ns = (kNameSpace *)KLIB Knull(kctx, KClass_NameSpace);
	ksymbol_t sym = KSymbol_("SCRIPT_ARGV");
	KKeyValue *kv = KLIB kNameSpace_GetConstNULL(kctx, ns, sym, false/*isLocalOnly*/);
	if(kv != NULL) {
		kArray *sa = (kArray *)kv->ObjectValue;
		if(sa->stringItems != NULL) {
			const char *file = kString_text(sa->stringItems[0]);
			return file;
		}
	}
	return shell;
}

static char *file2CId(const char *file, char *cid)
{
	memcpy(cid, file, strlen(file) + 1);
	char *pos = strstr(cid, ":");
	if(pos != NULL) {
		pos[0] = '\0';
	}
	return cid;
}

static void UI_ReportUserMessage(KonohaContext *kctx, kinfotag_t level, kfileline_t pline, const char *msg, int isNewLine)
{
	const char *file = PLATAPI shortFilePath(getThisFileName(kctx));
	char cid[64] = {0};
	file2CId(file, cid);
	PLATAPI LoggerModule.syslog_i(5/*LOG_NOTICE*/, "{\"Method\": \"DScriptMessage\", \"CId\": \"%s\", \"Body\": \"%s\"}" , cid, msg);
}

static void UI_ReportCompilerMessage(KonohaContext *kctx, kinfotag_t taglevel, kfileline_t pline, const char *msg)
{
	const char *file = PLATAPI shortFilePath(getThisFileName(kctx));
	char cid[64] = {0};
	file2CId(file, cid);
	PLATAPI LoggerModule.syslog_i( 5/*LOG_NOTICE*/, "{\"Method\": \"DScriptCompilerMessage\", \"CId\": \"%s\", \"Body\": \"%s\"}", cid, msg);
}

//static void KBuffer_WriteValue(KonohaContext *kctx, KBuffer *wb, KClass *c, KonohaStack *sfp)
//{
//	if(KClass_Is(UnboxType, c)) {
//		c->format(kctx, sfp, 0, wb);
//	}
//	else {
//		KLIB kObject_WriteToBuffer(kctx, sfp[0].asObject, false/*delim*/, wb, NULL, 0);
//	}
//}

static void UI_ReportCaughtException(KonohaContext *kctx, kException *e, KonohaStack *bottomStack, KonohaStack *topStack)
{
	// TODO
}

static void ReportDebugMessage(const char *file, const char *func, int line, const char *fmt, ...)
{
	// TODO
}

typedef struct {
	struct event_base *base;
	char *buff;
} UserInput;

static void userInput2Buff(struct evhttp_request *req, void *args)
{
	UserInput *ui = (UserInput *)args;
	struct event_base *base = ui->base;
	char *buff = ui->buff;
	struct evbuffer *body = evhttp_request_get_input_buffer(req);
	size_t len = evbuffer_get_length(body);
	unsigned char *requestLine;
	struct evbuffer *buf = evbuffer_new();

	switch(req->type) {
		case EVHTTP_REQ_POST:
			requestLine = evbuffer_pullup(body, -1);
			requestLine[len] = '\0';
			memcpy(buff, requestLine, len);
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			break;
		default:
			evhttp_send_error(req, HTTP_BADREQUEST, "Available POST only");
			break;
	}
	evbuffer_free(buf);
	event_base_loopbreak(base);
}

static char *getUserInput(KonohaContext *kctx, char *buff, const char *cid, const char *host, int port)
{
	struct event_base *base = event_base_new();
	struct evhttp *httpd = evhttp_new(base);
	if(evhttp_bind_socket(httpd, host, port) < 0) {
		PLATAPI LoggerModule.syslog_i(5/*LOG_NOTICE*/, "{\"Method\": \"DScriptError\", \"CId\": \"%s\", \"Body\": \"couldn't bind socket\"}", cid);
		exit(1);
	}

	UserInput ui = {};
	ui.base = base;
	ui.buff = buff;
	evhttp_set_gencb(httpd, userInput2Buff, (void *)&ui);
	event_base_dispatch(base);
	evhttp_free(httpd);
	event_base_free(base);

	return buff;
}

static int InputUserApproval(KonohaContext *kctx, const char *message, const char *yes, const char *no, int defval)
{
	char buff[BUFSIZ] = {0};
	const char *ykey = defval ? "Y" : "y";
	const char *nkey = defval ? "n" : "N";
	if(message == NULL || message[0] == '\0') message = "Do you approve?";
	if(yes == NULL || yes[0] == '\0') yes = "yes";
	if(no == NULL || no[0] == '\0') no = "no";

	const char *file = PLATAPI shortFilePath((getThisFileName(kctx)));
	char cid[64] = {0};
	file2CId(file, cid);

//	char hostname[BUFSIZ] = {0};
//	gethostname(hostname, BUFSIZ);
//	struct addrinfo *res;
//	struct in_Addr addr;
//	int err;
//	if((err = getaddrinfo(hostname, NULL, NULL, &res)) != 0) {
//		PLATAPI syslog_i(5/*LOG_NOTICE*/, "{\"Method\": \"DScriptError\", \"CId\": \"%s\", \"Body\": \"error %d\"}", cid, err);
//		exit(1);
//	}
//	addr.s_Addr = ((struct sockaddr_in *)(res->ai_Addr))->sin_Addr.s_Addr;
//	char host[16] = {0};
//	memcpy(host, inet_ntoa(addr), 16);
//	freeaddrinfo(res);

	const char host[] = "127.0.0.1"; // TODO get localhost IP
	int port = 8090; // TODO random port scan

	PLATAPI LoggerModule.syslog_i(5/*LOG_NOTICE*/, "{\"Method\": \"DScriptApproval\", \"CId\": \"%s\", \"Body\": \"%s (%s %s, %s %s): \", \"Ip\": \"%s:%d\"}" , cid, message, yes, ykey, no, nkey, host, port);
	getUserInput(kctx, buff, cid, host, port);
	if(defval) {
		return ((buff[0] == 'N' || buff[0] == 'n') && buff[1] == 0) ? false : true;
	}
	else {
		return ((buff[0] == 'Y' || buff[0] == 'y') && buff[1] == 0) ? false : true;
	}
}

static char* InputUserText(KonohaContext *kctx, const char *message, int flag)
{
	return "";
}

static char* InputUserPassword(KonohaContext *kctx, const char *message)
{
	char buff[BUFSIZ] = {0};

	const char *file = PLATAPI shortFilePath((getThisFileName(kctx)));
	char cid[64] = {0};
	file2CId(file, cid);

//	char hostname[BUFSIZ] = {0};
//	gethostname(hostname, BUFSIZ);
//	struct addrinfo *res;
//	struct in_Addr addr;
//	int err;
//	if((err = getaddrinfo(hostname, NULL, NULL, &res)) != 0) {
//		PLATAPI syslog_i(5/*LOG_NOTICE*/, "{\"Method\": \"DScriptError\", \"CId\": \"%s\", \"Body\": \"error %d\"}", cid, err);
//		exit(1);
//	}
//	addr.s_Addr = ((struct sockaddr_in *)(res->ai_Addr))->sin_Addr.s_Addr;
//	char host[16] = {0};
//	memcpy(host, inet_ntoa(addr), 16);
//	freeaddrinfo(res);

	const char host[] = "127.0.0.1"; // TODO get localhost IP
	int port = 8090; // TODO random port scan

	PLATAPI LoggerModule.syslog_i(5/*LOG_NOTICE*/, "{\"Method\": \"DScriptPassword\", \"CId\": \"%s\", \"Body\": \"%s\", \"Ip\": \"%s:%d\"}" , cid, message, host, port);
	getUserInput(kctx, buff, cid, host, port);
	size_t len = strlen(buff) + 1;
	char *p = malloc(len);
	if(p != NULL) {
		memcpy(p, buff, len);
	}
	return p;
}

// -------------------------------------------------------------------------
/* Logging */

#define writeToBuffer(CH, buftop, bufend) { buftop[0] = CH; buftop++; }

static char *writeFixedTextToBuffer(const char *text, size_t len, char *buftop, char *bufend)
{
	if((size_t)(bufend - buftop) > len) {
		memcpy(buftop, text, len);
		return buftop+len;
	}
	return buftop;
}

static char *writeTextToBuffer(const char *s, char *buftop, char *bufend)
{
	if(buftop < bufend) {
		buftop[0] = '"';
		buftop++;
	}
	while(*s != 0 && buftop < bufend) {
		if(*s == '"') {
			buftop[0] = '\"'; buftop++;
			if(buftop < bufend) {
				buftop[0] = s[0];
				buftop++;
			}
		}
		else if(*s == '\n') {
			buftop[0] = '\\'; buftop++;
			if(buftop < bufend) {
				buftop[0] = 'n';
				buftop++;
			}
		}
		else {
			buftop[0] = s[0];
			buftop++;
		}
		s++;
	}
	if(buftop < bufend) {
		buftop[0] = '"';
		buftop++;
	}
	return buftop;
}

static void reverse(char *const start, char *const end, const int len)
{
	int i, l = len / 2;
	register char *s = start;
	register char *e = end - 1;
	for (i = 0; i < l; i++) {
		char tmp = *s;
		*s++ = *e;
		*e-- = tmp;
	}
}

static char *writeUnsingedIntToBuffer(uintptr_t uint, char *const buftop, const char *const bufend)
{
	int i = 0;
	while(buftop + i < bufend) {
		int tmp = uint % 10;
		uint /= 10;
		buftop[i] = '0' + tmp;
		++i;
		if(uint == 0)
			break;
	}
	reverse(buftop, buftop + i, i);
	return buftop + i;
}

// the last entry of args must be NULL
static char *writeCharArrayToBuffer(const char** args, char *buftop, char *bufend)
{
	int i;
	buftop[0] = '['; buftop += 1;
	for(i = 0; args[i] != NULL; i++) {
		buftop = writeTextToBuffer(args[i], buftop, bufend);
		if(args[i+1] != NULL) {
			buftop[0] = ','; buftop[1] = ' '; buftop += 2;
		}
	}
	buftop[0] = ']';
	return buftop + 1;
}

static char* writeKeyToBuffer(const char *key, size_t keylen, char *buftop, char *bufend)
{
	if(buftop < bufend) {
		writeToBuffer('"', buftop, bufend);
	}
	buftop = writeFixedTextToBuffer(key, keylen, buftop, bufend);
	if(buftop + 3 < bufend) {
		buftop[0] = '"';
		buftop[1] = ':';
		buftop[2] = ' ';
		buftop+=3;
	}
	return buftop;
}

#define HasFault    (SystemFault|SoftwareFault|UserFault|ExternalFault)
#define HasLocation (PeriodicPoint|ResponseCheckPoint|SystemChangePoint|SecurityAudit)

static char* writePolicyToBuffer(KonohaContext *kctx, logconf_t *logconf, char *buftop, char *bufend, KTraceInfo *trace)
{
	if((logconf->policy & HasLocation)) {
		buftop = writeKeyToBuffer(TEXTSIZE("LogPoint"), buftop, bufend);
		writeToBuffer('"', buftop, bufend);
		if(KFlag_Is(int, logconf->policy, PeriodicPoint)) {
			buftop = writeFixedTextToBuffer(TEXTSIZE("PeriodicPoint,"), buftop, bufend);
		}
		if(KFlag_Is(int, logconf->policy, ResponseCheckPoint)) {
			buftop = writeFixedTextToBuffer(TEXTSIZE("ResponseCheckPoint,"), buftop, bufend);
		}
		if(KFlag_Is(int, logconf->policy, SystemChangePoint)) {
			buftop = writeFixedTextToBuffer(TEXTSIZE("SystemChangePoint,"), buftop, bufend);
		}
		if(KFlag_Is(int, logconf->policy, SecurityAudit)) {
			buftop = writeFixedTextToBuffer(TEXTSIZE("SecurityAudit,"), buftop, bufend);
		}
		buftop[-1] = '"';
		buftop[0] = ',';
		buftop[1] = ' ';
		buftop+=2;
	}
	if((logconf->policy & HasFault)) {
		if(!(logconf->policy & HasLocation)) {
			buftop = writeKeyToBuffer(TEXTSIZE("LogPoint"), buftop, bufend);
			buftop = writeTextToBuffer("ErrorPoint", buftop, bufend);
			buftop[0] = ',';
			buftop[1] = ' ';
			buftop+=2;
		}
		buftop = writeKeyToBuffer(TEXTSIZE("FaultType"), buftop, bufend);
		writeToBuffer('"', buftop, bufend);
		if(KFlag_Is(int, logconf->policy, SystemFault)) {
			buftop = writeFixedTextToBuffer(TEXTSIZE("SystemFault,"), buftop, bufend);
		}
		if(KFlag_Is(int, logconf->policy, SoftwareFault)) {
			buftop = writeFixedTextToBuffer(TEXTSIZE("SoftwareFault,"), buftop, bufend);
		}
		if(KFlag_Is(int, logconf->policy, UserFault)) {
			buftop = writeFixedTextToBuffer(TEXTSIZE("UserFault,"), buftop, bufend);
		}
		if(KFlag_Is(int, logconf->policy, ExternalFault)) {
			buftop = writeFixedTextToBuffer(TEXTSIZE("ExternalFault,"), buftop, bufend);
		}
		buftop[-1] = '"';
		buftop[0] = ',';
		buftop[1] = ' ';
		buftop+=2;
	}
	if(trace != NULL) {
		buftop = writeKeyToBuffer(TEXTSIZE("ScriptName"), buftop, bufend);
		buftop = writeTextToBuffer(KFileLine_textFileName(trace->pline), buftop, bufend);
		buftop[0] = ','; buftop[1] = ' '; buftop += 2;
		buftop = writeKeyToBuffer(TEXTSIZE("ScriptLine"), buftop, bufend);
		buftop = writeUnsingedIntToBuffer((uintptr_t)(kushort_t)trace->pline, buftop, bufend);
		buftop[0] = ','; buftop[1] = ' '; buftop += 2;
	}
	return buftop;
}

static char* writeErrnoToBuffer(logconf_t *logconf, char *buftop, char *bufend)
{
	if((logconf->policy & HasFault)) {
		buftop = writeKeyToBuffer(TEXTSIZE("Errno"), buftop, bufend);
		buftop = writeUnsingedIntToBuffer((uintptr_t)errno, buftop, bufend);
		buftop[0] = ','; buftop[1] = ' '; buftop+=2;
		buftop = writeKeyToBuffer(TEXTSIZE("Message"), buftop, bufend);
		buftop = writeTextToBuffer(strerror(errno), buftop, bufend);
		errno = 0;
	}
	return buftop;
}

static void writeDataLogToBuffer(KonohaContext *kctx, logconf_t *logconf, va_list ap, char *buftop, char *bufend, KTraceInfo *trace)
{
	int c = 0, logtype;
	buftop[0] = '{'; buftop++;
	buftop = writePolicyToBuffer(kctx, logconf, buftop, bufend, trace);
	while((logtype = va_arg(ap, int)) != LOG_END) {
		if(c > 0 && buftop + 3 < bufend) {
			buftop[0] = ',';
			buftop[1] = ' ';
			buftop+=2;
		}
		switch(logtype) {
		case LOG_s: {
			const char *key = va_arg(ap, const char *);
			const char *text = va_arg(ap, const char *);
			buftop = writeKeyToBuffer(key, strlen(key), buftop, bufend);
			buftop = writeTextToBuffer(text, buftop, bufend);
			break;
		}
		case LOG_u: {
			const char *key = va_arg(ap, const char *);
			buftop = writeKeyToBuffer(key, strlen(key), buftop, bufend);
			buftop = writeUnsingedIntToBuffer(va_arg(ap, uintptr_t), buftop, bufend);
			break;
		}
		case LOG_ERRNO : {
			buftop = writeErrnoToBuffer(logconf, buftop, bufend);
			break;
		}
		case LOG_a : {
			const char *key = va_arg(ap, const char *);
			buftop = writeKeyToBuffer(key, strlen(key), buftop, bufend);
			buftop = writeCharArrayToBuffer(va_arg(ap, const char**), buftop, bufend);
			break;
		}
		}
		c++;
	}
	buftop[0] = '}'; buftop++;
	buftop[0] = '\0';
}

#define HasFault    (SystemFault|SoftwareFault|UserFault|ExternalFault)

static void TraceDataLog(KonohaContext *kctx, KTraceInfo *trace, int logkey, logconf_t *logconf, ...)
{
	char buf[K_PAGESIZE];
	va_list ap;
	va_start(ap, logconf);
	writeDataLogToBuffer(kctx, logconf, ap, buf, buf + (K_PAGESIZE - 4), trace);
	int level = (logconf->policy & HasFault) ? LOG_ERR : LOG_NOTICE;
	const char *file = PLATAPI shortFilePath(getThisFileName(kctx));
	char cid[64] = {0};
	file2CId(file, cid);
	PLATAPI LoggerModule.syslog_i(level, "{\"Method\": \"DScriptError\", \"CId\": \"%s\", \"Body\": %s}", cid, buf);
	va_end(ap);
}

// -------------------------------------------------------------------------

kbool_t LoadDScriptConsoleModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"DScriptConsole", "0.1", 0, "dscript",
	};
	factory->ConsoleModule.ConsoleInfo     = &ModuleInfo;
	factory->ConsoleModule.ReportUserMessage        = UI_ReportUserMessage;
	factory->ConsoleModule.ReportCompilerMessage    = UI_ReportCompilerMessage;
	factory->ConsoleModule.ReportCaughtException    = UI_ReportCaughtException;
	factory->ConsoleModule.ReportDebugMessage       = ReportDebugMessage;
	factory->ConsoleModule.InputUserApproval        = InputUserApproval;
	factory->ConsoleModule.InputUserText            = InputUserText;
	factory->ConsoleModule.InputUserPassword        = InputUserPassword;
	factory->LoggerModule.TraceDataLog              = TraceDataLog;
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

