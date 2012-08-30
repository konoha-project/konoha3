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

/* ************************************************************************ */

#include <logpool/logpool.h>
#include "dse.h"

#define LOG_END 0
#define LOG_s   1
#define KEYVALUE_s(K,V)    LOG_s, (K), (V)

//static void vlplog(int priority, const char *message, va_list ap)
//{
//	void *logpool_args;
//	logpool_t *lp;
//	char *val = va_arg(ap, char *);
//	if(logpoolip) {
//		lp = logpool_open_trace(NULL, logpoolip, 14801);
//	}
//	else {
//		lp = logpool_open_trace(NULL, "0.0.0.0", 14801);
//	}
//	logpool_record(lp, &logpool_args, LOG_NOTICE, "dse",
//			KEYVALUE_s("", ""),
//			LOG_END);
//	logpool_close(lp);
//	return;
//}

static void lplog(int priority, const char *message, ...)
{
//	va_list ap;
//	va_start(ap, trace_id);
//	vlplog(priority, message, ap);
//	va_end(ap);
}

static const char *dse_formatScriptPath(char *buf, size_t bufsiz)
{
	FILE *fp = NULL;
	char *path = getenv("DSE_SCRIPTPATH");
	const char *local = "";
	if(path == NULL) {
		path = getenv("KONOHA_HOME");
		local = "/script";
	}
	if(path == NULL) {
		path = getenv("HOME");
		local = "/.minikonoha/script";
	}
	snprintf(buf, bufsiz, "%s%s/%s", path, local, "dse.k");
#ifdef K_PREFIX
	fp = fopen(buf, "r");
	if(fp != NULL) {
		fclose(fp);
		return (const char*)buf;
	}
	snprintf(buf, bufsiz, "%s/%s", K_PREFIX, "/minikonoha/script/dse.k");
#endif
	fp = fopen(buf, "r");
	if(fp != NULL) {
		fclose(fp);
		return (const char*)buf;
	}
	return NULL;
}

static void dse_define(KonohaContext *kctx, Message *msg)
{
	if(msg != NULL) {
		KDEFINE_TEXT_CONST TextData[] = {
			{"MESSAGE", TY_TEXT, (char *)msg}, {}
		};
		KLIB kNameSpace_loadConstData(kctx, KNULL(NameSpace), KonohaConst_(TextData), 0);
	}
}

static void *dse_dispatch(void *arg)
{
	Scheduler *sched = (Scheduler *)arg;
	Message *msg;
	kbool_t ret;
	char script[256];
	dse_formatScriptPath(script, sizeof(script));
	static bool platformIsInitialized = false;
	PlatformApi *dse_platform = KonohaUtils_getDefaultPlatformApi();
	if(!platformIsInitialized) {
		PlatformApiVar *dse_platformVar = (PlatformApiVar *)dse_platform;
		dse_platformVar->name = "dse";
		dse_platformVar->syslog_i = lplog;
		platformIsInitialized = true;
	}

	while(true) {
		pthread_mutex_lock(&sched->lock);
		if(!(msg = dse_dequeue(sched))) {
			pthread_cond_wait(&sched->cond, &sched->lock);
			msg = dse_dequeue(sched);
		}
		pthread_mutex_unlock(&sched->lock);
//		DEBUG_PRINT("%s", msg);
		KonohaContext* konoha = konoha_open(dse_platform);
		dse_define(konoha, msg);
		ret = konoha_load(konoha, script);
		konoha_close(konoha);
		Message_delete(msg);
	}
	return NULL;
}

#define THREAD_SIZE 16

static void dse_req_handler(struct evhttp_request *req, void *arg)
{
	struct evbuffer *body = evhttp_request_get_input_buffer(req);
	size_t len = evbuffer_get_length(body);
	Scheduler *sched = (Scheduler *)arg;
	Message *msg = NULL;
	unsigned char *requestLine;
	struct evbuffer *buf;
	static bool isFirst = true;
	static pthread_t thread_pool[THREAD_SIZE];
	int i;

	if(isFirst) {
		for(i = 0; i < THREAD_SIZE; i++) {
			pthread_create(&thread_pool[i], NULL, dse_dispatch, (void *)arg);
		}
		isFirst = false;
	}

	switch(req->type) {
		case EVHTTP_REQ_POST :
			// now, we fetch message
			requestLine = evbuffer_pullup(body, -1);
			requestLine[len] = '\0';
			msg = Message_new(requestLine);
			pthread_mutex_lock(&sched->lock);
			if(dse_enqueue(sched, msg)) {
				pthread_mutex_unlock(&sched->lock);
				pthread_cond_signal(&sched->cond);
				buf = evbuffer_new();
				evhttp_send_reply(req, HTTP_OK, "OK", buf);
				evbuffer_free(buf);
				break;
			}
			Message_delete(msg);
			pthread_mutex_unlock(&sched->lock);
			evhttp_send_error(req, HTTP_BADREQUEST, "DSE server's message queue is full");
			break;
		default :
			evhttp_send_error(req, HTTP_BADREQUEST, "Available POST only");
			break;
	}
}

DSE *DSE_new(void)
{
	DSE *newdse = (DSE *)dse_malloc(sizeof(DSE));
	newdse->base = event_base_new();
	newdse->httpd = evhttp_new(newdse->base);
	newdse->sched = Scheduler_new();
	return newdse;
}

void DSE_start(DSE *dse, const char *addr, int ip)
{
	if(evhttp_bind_socket(dse->httpd, addr, ip) < 0){
		perror("evhttp_bind_socket");
		exit(EXIT_FAILURE);
	}
	evhttp_set_gencb(dse->httpd, dse_req_handler, (void *)dse->sched);
	event_base_dispatch(dse->base);
}

void DSE_delete(DSE *dse)
{
	evhttp_free(dse->httpd);
	event_base_free(dse->base);
	Scheduler_delete(dse->sched);
	dse_free(dse, sizeof(DSE));
}
