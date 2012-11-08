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

static const char* BeginTag(KonohaContext *kctx, kinfotag_t t)
{
	DBG_ASSERT(t <= NoneTag);
	if(!KonohaContext_Is(Interactive, kctx)) t = NoneTag;
	static const char* tags[] = {
		"\x1b[1m\x1b[31m", /*CritTag*/
		"\x1b[1m\x1b[31m", /*ErrTag*/
		"\x1b[1m\x1b[31m", /*WarnTag*/
		"\x1b[1m", /*NoticeTag*/
		"\x1b[1m", /*InfoTag*/
		"", /*DebugTag*/
		"", /* NoneTag*/
	};
	return tags[(int)t];
}

static const char* EndTag(KonohaContext *kctx, kinfotag_t t)
{
	DBG_ASSERT(t <= NoneTag);
	if(!KonohaContext_Is(Interactive, kctx)) t = NoneTag;
	static const char* tags[] = {
			"\x1b[0m", /*CritTag*/
			"\x1b[0m", /*ErrTag*/
			"\x1b[0m", /*WarnTag*/
			"\x1b[0m", /*NoticeTag*/
			"\x1b[0m", /*InfoTag*/
			"", /* Debug */
			"", /* NoneTag*/
	};
	return tags[(int)t];
}

static void UI_ReportUserMessage(KonohaContext *kctx, kinfotag_t level, kfileline_t pline, const char *msg, int isNewLine)
{
	const char *beginTag = BeginTag(kctx, level);
	const char *endTag = EndTag(kctx, level);
	const char *kLF = isNewLine ? "\n" : "";
	if(pline > 0) {
		const char *file = FileId_t(pline);
		PLATAPI syslog_i(5/*LOG_NOTICE*/, "{\"Method\": \"DScriptMessage\", \"Body\": \"%s - (%s:%d) %s%s%s\"}" , beginTag, PLATAPI shortFilePath(file), (kushort_t)pline, msg, kLF, endTag);
	}
	else {
		PLATAPI syslog_i(5/*LOG_NOTICE*/, "{\"Method\": \"DScriptMessage\", \"Body\": \"%s%s%s%s\"}", beginTag,  msg, kLF, endTag);
	}
}

static void UI_ReportCompilerMessage(KonohaContext *kctx, kinfotag_t taglevel, kfileline_t pline, const char *msg)
{
	const char *beginTag = BeginTag(kctx, taglevel);
	const char *endTag = EndTag(kctx, taglevel);
	PLATAPI syslog_i( 5/*LOG_NOTICE*/, "{\"Method\": \"DScriptMessage\", \"Body\": \"%s - %s%s\n\"}", beginTag, msg, endTag);
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
} UserApproval;

static void checkUserApproval(struct evhttp_request *req, void *args)
{
	UserApproval *ua = (UserApproval *)args;
	struct event_base *base = ua->base;
	char *buff = ua->buff;
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

static int InputUserApproval(KonohaContext *kctx, const char *message, const char *yes, const char *no, int defval)
{
	char buff[BUFSIZ] = {0};
	const char *ykey = defval ? "Y" : "y";
	const char *nkey = defval ? "n" : "N";
	if(message == NULL) message = "Do you approve?";
	if(yes == NULL) yes = "yes";
	if(no == NULL) no = "no";
	const char host[] = "127.0.0.1";
	int port = 8090;

	PLATAPI syslog_i(5/*LOG_NOTICE*/, "{\"Method\": \"DScriptAsk\", \"Body\": \"%s (%s %s, %s %s): \", \"Ip\", \"%s:%d\"}" , message, yes, ykey, no, nkey, host, port);

	struct event_base *base = event_base_new();
	struct evhttp *httpd = evhttp_new(base);
	if(evhttp_bind_socket(httpd, host, port) < 0) {
		PLATAPI syslog_i(5/*LOG_NOTICE*/, "{\"Method\": \"DScriptMessage\", \"Body\": \"couldn't bind socket\"}");
		exit(1);
	}

	UserApproval ua = {};
	ua.base = base;
	ua.buff = buff;
	evhttp_set_gencb(httpd, checkUserApproval, (void *)&ua);
	event_base_dispatch(base);
	evhttp_free(httpd);
	event_base_free(base);

	if(defval) {
		return ((buff[0] == 'N' || buff[0] == 'n') && buff[1] == 0) ? false : true;
	}
	else {
		return ((buff[0] == 'Y' || buff[0] == 'y') && buff[1] == 0);
	}
}

static const char* InputUserText(KonohaContext *kctx, const char *message, int flag)
{

}

static const char* InputUserPassword(KonohaContext *kctx, const char *message)
{

	return "";
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

