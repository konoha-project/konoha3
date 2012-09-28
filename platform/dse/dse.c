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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <event.h>
#include <evhttp.h>
#include <event2/buffer.h>
#include <logpool/logpool.h>
#ifndef K_USE_PTHREAD
#define K_USE_PTHREAD 1
#endif /* K_USE_PTHREAD */
#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/platform_posix.h>
#ifndef K_USE_PTHREAD
#include <pthread.h>
#endif /* K_USE_PTHREAD */

// for debug
#define DEBUG_PRINT(fmt, ...) do { \
	if (verbose_debug) fprintf(stderr, fmt "\n", ##__VA_ARGS__); \
} while (0)
#define DEBUG_ASSERT(stmt) do { \
	if (verbose_debug) assert(stmt); \
} while (0)

// for memory management
static void *dse_malloc(size_t size);
static void  dse_free(void *ptr, size_t size);

/* Message */
struct Message {
	unsigned char *data;
	size_t         len;
};
typedef struct Message Message;

static Message *Message_new(unsigned char *requestLine, size_t length);
static void     Message_delete(Message *msg);

/* Scheduler */
#define MSG_QUEUE_SIZE 16
#define POOL_SIZE      16
#define next(index)    (((index) + 1) % MSG_QUEUE_SIZE)

struct Scheduler {
	Message        *msgQueue[MSG_QUEUE_SIZE];
	int             front;
	int             last;
	pthread_mutex_t lock;
	pthread_cond_t  cond;
};
typedef struct Scheduler Scheduler;

static Scheduler *Scheduler_new(void);
static void       Scheduler_delete(Scheduler *sched);
static bool       dse_enqueue(Scheduler *sched, Message *msg);
static Message   *dse_dequeue(Scheduler *sched);

/* DSE */
struct DSE {
	struct event_base *base;
	struct evhttp     *httpd;
	Scheduler         *sched;
};
typedef struct DSE DSE;

static DSE *DSE_new(void);
static void DSE_start(DSE *dse, const char *addr, int ip);
static void DSE_delete(DSE *dse);

#define PATH_SIZE   256
#define THREAD_SIZE 16
#define LOG_END     0
#define LOG_s       1
#define KEYVALUE_s(K,V) LOG_s, (K), (V)

#define HTTPD_ADDR  "0.0.0.0"
#define HTTPD_PORT  8080

struct targs {
	Scheduler   *sched;
	PlatformApi *platform;
};



#ifdef __cplusplus
extern "C" {
#endif

/* global variables */
static size_t totalMalloc = 0;
static int    port = HTTPD_PORT;
static int    threadsize = THREAD_SIZE;
static char  *logpoolip = NULL;
static char  *scriptdir = NULL;

// ---------------------------------------------------------------------------
// [utilities]

static void *dse_malloc(size_t size)
{
	void *ptr = malloc(size);
	if (ptr == NULL) {
		DEBUG_PRINT("malloc failed");
		size = 0;
	}
	if (verbose_debug) {
		totalMalloc += size;
		fprintf(stderr, "totalMalloc:%ld @dse_malloc()\n", totalMalloc);
	}
	return ptr;
}

static void dse_free(void *ptr, size_t size)
{
	free(ptr);
	if (verbose_debug) {
		totalMalloc -= size;
		fprintf(stderr, "totalMalloc:%ld @dse_free()\n", totalMalloc);
		assert(totalMalloc >= 0);
	}
}

// ---------------------------------------------------------------------------
// [Message]

static Message *Message_new(unsigned char *requestLine, size_t length)
{
	Message *msg = (Message *)dse_malloc(sizeof(Message));
	if(length <= 0) {
		length = strlen((const char *)requestLine);
	}
	msg->data = (unsigned char *)dse_malloc(length + 1);
	memcpy(msg->data, requestLine, length);
	msg->data[length] = '\0';
	msg->len = length;
	return msg;
}

static void Message_delete(Message *msg)
{
	if(msg == NULL) return;
	dse_free(msg->data, msg->len + 1);
	dse_free(msg, sizeof(Message));
}

// ---------------------------------------------------------------------------
// [Scheduler]

static Scheduler *Scheduler_new(void)
{
	Scheduler *sched = dse_malloc(sizeof(Scheduler));
	sched->front = 0;
	sched->last = 0;
	pthread_mutex_init(&sched->lock, NULL);
	pthread_cond_init(&sched->cond, NULL);
	return sched;
}

static void Scheduler_delete(Scheduler *sched)
{
	pthread_mutex_destroy(&sched->lock);
	pthread_cond_destroy(&sched->cond),
		dse_free(sched, sizeof(Scheduler));
}

static bool dse_enqueue(Scheduler *sched, Message *msg)
{
	int front = sched->front;
	int last = sched->last;
	if(next(front) == last) return false;
	sched->msgQueue[front] = msg;
	sched->front = next(front);
	return true;
}

static Message *dse_dequeue(Scheduler *sched)
{
	int front = sched->front;
	int last = sched->last;
	if(front == last) return NULL;
	Message *msg = sched->msgQueue[last];
	sched->last = next(last);
	return msg;
}

// ---------------------------------------------------------------------------
// [DSE]

//static void vlplog(int priority, const char *message, va_list ap)
//{
// void *logpool_args;
// logpool_t *lp;
// char *val = va_arg(ap, char *);
// if(logpoolip) {
// lp = logpool_open_trace(NULL, logpoolip, 14801);
// }
// else {
// lp = logpool_open_trace(NULL, "0.0.0.0", 14801);
// }
// logpool_record(lp, &logpool_args, LOG_NOTICE, "dse",
// KEYVALUE_s("", ""),
// LOG_END);
// logpool_close(lp);
// return;
//}

static void lplog(int priority, const char *message, ...)
{
	// va_list ap;
	// va_start(ap, trace_id);
	// vlplog(priority, message, ap);
	// va_end(ap);
}

static const char *dse_formatScriptPath(char *buf, size_t bufsiz)
{
	FILE *fp = NULL;
	char *path = getenv("DSE_SCRIPTPATH");
	const char *local = "";
	if(path == NULL) {
		path = getenv("KONOHA_HOME");
		local = "/dse";
	}
	if(path == NULL) {
		path = getenv("HOME");
		local = "/.minikonoha/dse";
	}
	snprintf(buf, bufsiz, "%s%s/%s", path, local, "dse.k");
#ifdef K_PREFIX
	fp = fopen(buf, "r");
	if(fp != NULL) {
		fclose(fp);
		return (const char*)buf;
	}
	snprintf(buf, bufsiz, "%s/%s", K_PREFIX, "/minikonoha/dse/dse.k");
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
			{"DSE_MESSAGE", TY_TEXT, (char *)msg->data},
			{"DSE_SCRIPT_DIR", TY_TEXT, scriptdir},
			{}
		};
		KLIB kNameSpace_loadConstData(kctx, KNULL(NameSpace), KonohaConst_(TextData), 0);
	}
	else {
		KDEFINE_TEXT_CONST TextData[] = {
			{"DSE_SCRIPT_DIR", TY_TEXT, scriptdir},
			{}
		};
		KLIB kNameSpace_loadConstData(kctx, KNULL(NameSpace), KonohaConst_(TextData), 0);
	}
	KDEFINE_INT_CONST IntData[] = {
		{"DSE_DEBUG", TY_int, verbose_debug}, {}
	};
	KLIB kNameSpace_loadConstData(kctx, KNULL(NameSpace), KonohaConst_(IntData), 0);
}

static void *dse_dispatch(void *arg)
{
	struct targs *args = (struct targs *)arg;
	Scheduler *sched = args->sched;
	PlatformApi *dse_platform = args->platform;
	Message *msg;
	kbool_t ret;
	char script[PATH_SIZE];
	dse_formatScriptPath(script, sizeof(script));

	while(true) {
		pthread_mutex_lock(&sched->lock);
		if(!(msg = dse_dequeue(sched))) {
			pthread_cond_wait(&sched->cond, &sched->lock);
			msg = dse_dequeue(sched);
		}
		pthread_mutex_unlock(&sched->lock);
		// DEBUG_PRINT("%s", msg);
		KonohaContext* konoha = konoha_open(dse_platform);
		dse_define(konoha, msg);
		ret = konoha_load(konoha, script);
		konoha_close(konoha);
		Message_delete(msg);
	}
	return NULL;
}

static void dse_req_handler(struct evhttp_request *req, void *arg)
{
	struct evbuffer *body = evhttp_request_get_input_buffer(req);
	size_t len = evbuffer_get_length(body);
	Scheduler *sched = (Scheduler *)arg;
	Message *msg = NULL;
	unsigned char *requestLine;
	struct evbuffer *buf;

	switch(req->type) {
		case EVHTTP_REQ_POST :
			// now, we fetch message
			requestLine = evbuffer_pullup(body, -1);
			msg = Message_new(requestLine, len);
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

static DSE *DSE_new(void)
{
	DSE *newdse = (DSE *)dse_malloc(sizeof(DSE));
	newdse->base = event_base_new();
	newdse->httpd = evhttp_new(newdse->base);
	newdse->sched = Scheduler_new();
	return newdse;
}

static void DSE_start(DSE *dse, const char *addr, int ip)
{
	if(evhttp_bind_socket(dse->httpd, addr, ip) < 0){
		perror("evhttp_bind_socket");
		exit(EXIT_FAILURE);
	}

	int i;
	pthread_t thread_pool[threadsize];
	PlatformApi *dse_platform = KonohaUtils_getDefaultPlatformApi();
	PlatformApiVar *dse_platformVar = (PlatformApiVar *)dse_platform;
	dse_platformVar->name = "dse";
	dse_platformVar->syslog_i = lplog;
	struct targs args = {
		dse->sched,
		dse_platform
	};

	for(i = 0; i < threadsize; i++) {
		pthread_create(&thread_pool[i], NULL, dse_dispatch, (void *)&args);
	}
	evhttp_set_gencb(dse->httpd, dse_req_handler, (void *)dse->sched);
	event_base_dispatch(dse->base);
}

static void DSE_delete(DSE *dse)
{
	evhttp_free(dse->httpd);
	event_base_free(dse->base);
	Scheduler_delete(dse->sched);
	dse_free(dse, sizeof(DSE));
}

static struct option long_option[] = {
	/* These options set a flag. */
	{"verbose", no_argument, &verbose_debug, 1},
	{"logpool", required_argument, 0, 'l'},
	{"port", required_argument, 0, 'p'},
	{"threadsize", required_argument, 0, 't'},
	{"scriptdir", required_argument, 0, 'D'},
	{0, 0, 0, 0}
};

static void dse_parseopt(int argc, char *argv[])
{
	char *e;
	logpoolip = getenv("LOGPOOL_IP");
	while(true) {
		int option_index = 0;
		int c = getopt_long(argc, argv, "l:p:t:D:", long_option, &option_index);
		if(c == -1) break; /* Detect the end of the options. */
		switch(c) {
			case 'l':
				logpoolip = optarg;
				break;
			case 'p':
				port = (int)strtol(optarg, &e, 10);
				break;
			case 't':
				threadsize = (int)strtol(optarg, &e, 10);
				break;
			case 'D':
				scriptdir = optarg;
				break;
			case '?':
				fprintf(stderr, "Unknown or required argument option -%c\n", optopt);
				fprintf(stderr, "Usage: COMMAND [ --verbose ] [ --port | -p ] port [ --logpool | -l ] logpoolip\n");
				exit(EXIT_FAILURE);
			default:
				break;
		}
	}
	if(!logpoolip) logpoolip = "0.0.0.0";
	if(!scriptdir) scriptdir = "./";
	if(getenv("DSE_DEBUG") != NULL || getenv("KONOHA_DEBUG") != NULL) {
		verbose_debug = 1;
	}
}

int main(int argc, char *argv[])
{
	dse_parseopt(argc, argv);
	DEBUG_PRINT("DSE starts on port %d", port);
	DEBUG_PRINT("LogPool is running on %s", logpoolip);
	logpool_global_init(LOGPOOL_TRACE);
	DSE *dse = DSE_new();
	DSE_start(dse, HTTPD_ADDR, port);
	DSE_delete(dse);
	logpool_global_exit();
	return 0;
}

#ifdef __cplusplus
}
#endif
