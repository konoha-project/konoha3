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
#ifndef DSE_H_
#define DSE_H_

#include <stdbool.h>
#include <unistd.h>
#include <event.h>
#include <evhttp.h>
#include <event2/buffer.h>
#include <sys/queue.h>
#include <sys/wait.h>
#include <jansson.h>
#include "util.h"
#include "logger.h"
#include "protocol.h"
#include "scheduler.h"

struct dDserv;

extern struct dDserv *gdserv;

struct dDserv {
	struct event_base *base;
	struct evhttp *httpd;
	struct dScheduler *dscd;
};

#define JSON_INITGET(O, K) \
	json_t *K = json_object_get(O, #K)

static struct dReq *dse_parseJson(const char *input)
{
	json_t *root;
	json_error_t error;

	root = json_loads(input, 0, &error);
	if(!root) {
		D_("error occurs");
		return NULL;
	}
//	JSON_INITGET(root, taskid);
//	JSON_INITGET(root, type);
	JSON_INITGET(root, context);
	JSON_INITGET(root, method);
	JSON_INITGET(root, logpool);
	JSON_INITGET(root, script);

	struct dReq *ret = newDReq();
	if(strncmp(json_string_value(method), "eval", 4) == 0) {
		ret->method = E_METHOD_EVAL;
	}
	else if(strncmp(json_string_value(method), "tycheck", 7) == 0) {
		ret->method = E_METHOD_TYCHECK;
	}
	else{
		D_("error");
		return NULL;
	}
	// for logpool
	const char *str_logpoolip = json_string_value(logpool);
	size_t logpoolip_len = strlen(str_logpoolip);
	strncpy(ret->logpoolip, str_logpoolip, logpoolip_len + 1);
	// store file
	char *filename = ret->scriptfilepath;
	const char *str_context = json_string_value(context);
	size_t context_len = strlen(str_context);
	ret->context = atoi(str_context);
	strncpy(filename, str_context, context_len);
	snprintf(filename, context_len+3, "%s.k", str_context);
	FILE *fp = fopen(filename, "w");
	char *str_script = (char *)json_string_value(script);
	// replace "'" --> "\"";
	size_t script_len = strlen(str_script);
//	char ch;
//	int idx = 0;
//	for (idx = 0; idx < script_len; idx++) {
//		if (str_script[idx] == '\'') {
//			str_script[idx] ='"';
//		} else if (str_script[idx] == '@') {
//			str_script[idx] = '\n';
//		}
//	}
	fwrite(str_script, script_len, 1, fp);
	fflush(fp);
	fclose(fp);
	json_decref(root);
	return ret;
}

/* ************************************************************************ */

void dump_http_header(struct evhttp_request *req, struct evbuffer *evb, void *ctx)
{
	struct evkeyvalq *head = evhttp_request_get_input_headers(req);
	struct evkeyval *entry;
	TAILQ_FOREACH(entry, head, next) {
		evbuffer_add_printf(evb, "%s:%s<br>", entry->key, entry->value);
	}
	evhttp_add_header(req->output_headers, "Access-Control-Allow-Origin", "http://localhost:8080/");
	evhttp_add_header(req->output_headers, "Access-Control-Max-Age", "6238800");
	evhttp_add_header(req->output_headers, "Access-Control-Allow-Method", "POST");

	evhttp_send_reply(req, HTTP_OK, "OK", evb);
}

static void *dse_dispatch(void *arg);

#define THREAD_SIZE 8

// request handler for DSE Protocol
void dse_req_handler(struct evhttp_request *req, void *arg)
{
	struct evbuffer *body = evhttp_request_get_input_buffer(req);
	size_t len = evbuffer_get_length(body);
	struct dScheduler *dscd = (struct dScheduler *)arg;
	struct dReq *dreq = NULL;
//	struct dRes *dres = NULL;
	static bool isFirst = true;
	static pthread_t thread_pool[THREAD_SIZE];
	int i;
	if(isFirst) {
		for(i = 0; i < THREAD_SIZE; i++) {
			pthread_create(&thread_pool[i], NULL, dse_dispatch, arg);
		}
		isFirst = false;
	}
	if (req->type == EVHTTP_REQ_GET) {
		evhttp_send_error(req, HTTP_BADREQUEST, "DSE server doesn't accept GET request");
	} else if(req->type == EVHTTP_REQ_POST) {
		// now, we fetch key values
		unsigned char *requestLine;
		requestLine = evbuffer_pullup(body, -1);
		requestLine[len] = '\0';
		//now, parse json.
		dreq = dse_parseJson((const char*)requestLine);
		dreq->req = req;
		pthread_mutex_lock(&dscd->lock);
		if(dse_enqueue(dscd, dreq)) {
			pthread_mutex_unlock(&dscd->lock);
			pthread_cond_signal(&dscd->cond);
			struct evbuffer *buf = evbuffer_new();   // FIXME
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			return;
		}
		pthread_mutex_unlock(&dscd->lock);
		evhttp_send_error(req, HTTP_BADREQUEST, "DSE server's request queue is full");
	}
	else{
		evhttp_send_error(req, HTTP_BADREQUEST, "Available POST only");
	}
}

static struct dDserv *dserv_new(void)
{
	struct dDserv *dserv = dse_malloc(sizeof(struct dDserv));
	dserv->base = event_base_new();
	dserv->httpd = evhttp_new(dserv->base);
	dserv->dscd = newDScheduler();
	return dserv;
}

static int dserv_start(struct dDserv *dserv, const char *addr, int ip)
{
	if (evhttp_bind_socket(dserv->httpd, addr, ip) < 0){
		perror("evhttp_bind_socket");
		exit(EXIT_FAILURE);
	}
	evhttp_set_gencb(dserv->httpd, dse_req_handler, (void *)dserv->dscd);
	event_base_dispatch(dserv->base);
	return 0;
}

static void dserv_close(struct dDserv *dserv)
{
	evhttp_free(dserv->httpd);
	event_base_free(dserv->base);
	deleteDScheduler(dserv->dscd);
	dse_free(dserv, sizeof(struct dDserv));
}

//static void dse_send_reply(struct evhttp_request *req, struct dRes *dres)
//{
//	struct evbuffer *buf = evbuffer_new();
//	switch(dres->status){
//		case E_STATUS_OK:
//			evhttp_send_reply(req, HTTP_OK, "OK", buf);
//			break;
//		default:
//			evhttp_send_reply(req, HTTP_OK, "FAIL", buf);
//			break;
//	}
//	evbuffer_free(buf);
//}

static void *dse_dispatch(void *arg)
{
	struct dScheduler *dscd = (struct dScheduler *)arg;
	struct dReq *dreq;
	struct dRes *dres;
	char cmd_konoha[] = "minikonoha";
	char cmd_opt_tycheck[] = "-c";
	char cmd_sh[] = "sh";
	pid_t pid;
	int status = 0;
	logpool_t *lp;
	void *logpool_args;
	while(true) {
		pthread_mutex_lock(&dscd->lock);
		if(!(dreq = dse_dequeue(dscd))) {
			pthread_cond_wait(&dscd->cond, &dscd->lock);
			dreq = dse_dequeue(dscd);
		}
		pthread_mutex_unlock(&dscd->lock);
		D_("scriptpath:%s", dreq->scriptfilepath);
		dres = newDRes();
		pid = fork();
		switch(pid) {
			case -1:
				dserv_close(gdserv);
				D_("error in fork()");
				exit(1);
			case 0:
				switch (dreq->method){
					case E_METHOD_EVAL: case E_METHOD_TYCHECK:
						lp = dse_openlog(dreq->logpoolip);
						dse_record(lp, &logpool_args, "dtask",
								KEYVALUE_u("context", dreq->context),
								KEYVALUE_s("status", "start"),
								KEYVALUE_u("pid", getpid()),
								LOG_END);
						execlp(cmd_konoha, cmd_konoha, dreq->scriptfilepath, NULL);
						dse_record(lp, &logpool_args, "dtask",
								KEYVALUE_u("context", dreq->context),
								KEYVALUE_s("status", "failed"),
								LOG_END);
						dse_closelog(lp);
//						if(ret == 1) {
						// ok;
//							dres->status = E_STATUS_OK;
//						}
						break;
						//case E_METHOD_TYCHECK:
						//	break;
					default:
						D_("there's no such method");
						break;
				}
//				dse_send_reply(dreq->req, dres);
				deleteDReq(dreq);
				deleteDRes(dres);
				exit(1);
			default:
				D_("pid: %d", pid);
				waitpid(pid, &status, 0);
				lp = dse_openlog(dreq->logpoolip);
				if(WIFEXITED(status)) {
					dse_record(lp, &logpool_args, "dtask",
							KEYVALUE_u("context", dreq->context),
							KEYVALUE_s("status", "done"),
							KEYVALUE_u("exit status", WEXITSTATUS(status)),
							LOG_END);
				} else {
					dse_record(lp, &logpool_args, "dtask",
							KEYVALUE_u("context", dreq->context),
							KEYVALUE_s("status", "failed"),
							KEYVALUE_u("exit status", WEXITSTATUS(status)),
							LOG_END);
				}
				dse_closelog(lp);
				break;
		}
	}
}

#endif /* DSE_H_ */
