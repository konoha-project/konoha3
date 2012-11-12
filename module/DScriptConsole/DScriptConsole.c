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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sys/ioctl.h>
//#include <asm/termbits.h>
#include <minikonoha/minikonoha.h>
#include <minikonoha/klib.h>

// -------------------------------------------------------------------------
/* Console */

static const char *getThisFileName(KonohaContext *kctx)
{
	static char shell[] = "shell";
	kNameSpace *ns = (kNameSpace *)KLIB Knull(kctx, CT_NameSpace);
	ksymbol_t sym = SYM_("SCRIPT_ARGV");
	KKeyValue *kv = KLIB kNameSpace_GetConstNULL(kctx, ns, sym);
	if(kv != NULL) {
		kArray *sa = (kArray *)kv->ObjectValue;
		if(sa->stringItems != NULL) {
			const char *file = S_text(sa->stringItems[0]);
			return file;
		}
	}
	return shell;
}

static char *file2CId(const char *file, char *cid)
{
	memcpy(cid, file, strlen(file) + 1);
	char *pos = strstr(cid, "\:");
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
	PLATAPI syslog_i(5/*LOG_NOTICE*/, "{\"Method\": \"DScriptMessage\", \"CId\": \"%s\", \"Body\": \"%s\"}" , cid, msg);
}

static void UI_ReportCompilerMessage(KonohaContext *kctx, kinfotag_t taglevel, kfileline_t pline, const char *msg)
{
	const char *file = PLATAPI shortFilePath(getThisFileName(kctx));
	char cid[64] = {0};
	file2CId(file, cid);
	PLATAPI syslog_i( 5/*LOG_NOTICE*/, "{\"Method\": \"DScriptCompilerMessage\", \"CId\": \"%s\", \"Body\": \"%s\"}", cid, msg);
}

static void Kwb_writeValue(KonohaContext *kctx, KGrowingBuffer *wb, KonohaClass *c, KonohaStack *sfp)
{
	if(CT_isUnbox(c)) {
		c->p(kctx, sfp, 0, wb);
	}
	else {
		KLIB kObject_writeToBuffer(kctx, sfp[0].asObject, false/*delim*/, wb, NULL, 0);
	}
}

static void UI_ReportCaughtException(KonohaContext *kctx, const char *exceptionName, int fault, const char *optionalMessage, KonohaStack *bottomStack, KonohaStack *topStack)
{
	// TODO
}

static void ReportDebugMessage(const char *file, const char *func, int line, const char *fmt, ...)
{
	// TODO
}

#include <event.h>
#include <evhttp.h>

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
		PLATAPI syslog_i(5/*LOG_NOTICE*/, "{\"Method\": \"DScriptError\", \"CId\": \"%s\", \"Body\": \"couldn't bind socket\"}", cid);
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

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

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
//	struct in_addr addr;
//	int err;
//	if((err = getaddrinfo(hostname, NULL, NULL, &res)) != 0) {
//		PLATAPI syslog_i(5/*LOG_NOTICE*/, "{\"Method\": \"DScriptError\", \"CId\": \"%s\", \"Body\": \"error %d\"}", cid, err);
//		exit(1);
//	}
//	addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
//	char host[16] = {0};
//	memcpy(host, inet_ntoa(addr), 16);
//	freeaddrinfo(res);

	const char host[] = "127.0.0.1"; // TODO get localhost IP
	int port = 8090; // TODO random port scan

	PLATAPI syslog_i(5/*LOG_NOTICE*/, "{\"Method\": \"DScriptApproval\", \"CId\": \"%s\", \"Body\": \"%s (%s %s, %s %s): \", \"Ip\": \"%s:%d\"}" , cid, message, yes, ykey, no, nkey, host, port);
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
//	struct in_addr addr;
//	int err;
//	if((err = getaddrinfo(hostname, NULL, NULL, &res)) != 0) {
//		PLATAPI syslog_i(5/*LOG_NOTICE*/, "{\"Method\": \"DScriptError\", \"CId\": \"%s\", \"Body\": \"error %d\"}", cid, err);
//		exit(1);
//	}
//	addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
//	char host[16] = {0};
//	memcpy(host, inet_ntoa(addr), 16);
//	freeaddrinfo(res);

	const char host[] = "127.0.0.1"; // TODO get localhost IP
	int port = 8090; // TODO random port scan

	PLATAPI syslog_i(5/*LOG_NOTICE*/, "{\"Method\": \"DScriptPassword\", \"CId\": \"%s\", \"Body\": \"%s\", \"Ip\": \"%s:%d\"}" , cid, message, host, port);
	getUserInput(kctx, buff, cid, host, port);
	size_t len = strlen(buff) + 1;
	char *p = malloc(len);
	if(p != NULL) {
		memcpy(p, buff, len);
	}
	return p;
}

// -------------------------------------------------------------------------

kbool_t LoadDScriptConsoleModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"DScriptConsole", "0.1", 0, "dscript",
	};
	factory->ConsoleInfo     = &ModuleInfo;

	factory->ReportUserMessage        = UI_ReportUserMessage;
	factory->ReportCompilerMessage    = UI_ReportCompilerMessage;
	factory->ReportCaughtException    = UI_ReportCaughtException;
	factory->ReportDebugMessage       = ReportDebugMessage;
	factory->InputUserApproval        = InputUserApproval;

	factory->InputUserText            = InputUserText;
	factory->InputUserPassword        = InputUserPassword;
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

