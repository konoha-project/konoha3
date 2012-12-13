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
#include <minikonoha/klib.h>

#ifdef __GNUC__
#include <getopt.h>
#else

char *optarg = 0;
int optind   = 1;
int optopt   = 0;
int opterr   = 0;
int optreset = 0;

struct option {
	char *name;
	int has_arg;
	int *flag;
	int val;
};

/* The has_arg field should be one of: */
enum {
	no_argument,       /* no argument to the option is expect        */
	required_argument, /* an argument to the option is required      */
	optional_argument, /* an argument to the option may be presented */
};

static int getopt_long(int argc, char * const *argv, const char *optstring, const struct option *longopts, int *longindex);

#include <string.h>
#include <ctype.h>
static int getopt_long(int argc, char * const *argv, const char *optstring, const struct option *longopts, int *longindex)
{
	if(optind < argc) {
		char *arg = argv[optind];
		if(arg == 0)
			return -1;
		if(arg[0] == '-' && arg[1] == '-') {
			const struct option *opt = longopts;
			arg += 2;
			while (opt->name) {
				char *end = strchr(arg, '=');
				if(end == 0 && opt->has_arg == no_argument) {
					if(strcmp(arg, opt->name) == 0)
						*longindex = opt - longopts;
					optind++;
					return opt->val;
				}
				if(strncmp(arg, opt->name, end - arg) == 0) {
					*longindex = opt - longopts;
					optarg = end+1;
					optind++;
					return opt->val;
				}
				opt++;
			}
		}
		else if(arg[0] == '-') {
			arg += 1;
			const char *c = optstring;
			while (*c != 0) {
				if(*c == arg[0]) {
					if(*(c+1) == ':' && arg[1] == '=') {
						optarg = arg+2;
					}
					optind++;
					return arg[0];
				}
				c++;
			}
		}
	}
	return -1;
}

#endif /*__GNUC__ */

#ifdef __cplusplus
extern "C" {
#endif

// for debug
#include <stdio.h>

extern int verbose_debug;

#define DEBUG_P(fmt, ...) do { \
	if(verbose_debug) fprintf(stdout, "DEBUG(%s:%d) " fmt "\n", __func__, __LINE__, ##__VA_ARGS__); \
} while (0)
#define DEBUG_ABORT() do { \
	if(verbose_debug) fprintf(stderr, "ABORT(%s:%d) ", __func__, __LINE__); \
	perror(NULL); \
	exit(EXIT_FAILURE); \
} while (0)
#define DEBUG_ASSERT(stmt) do { \
	assert(stmt); \
} while (0)

// for memory management
static size_t totalSizeOfAllocatedMemory = 0;

static void *dse_malloc(size_t size)
{
	void *ptr = malloc(size);
	if(ptr == NULL) {
		fprintf(stderr, "malloc failed\n");
		size = 0;
	}
	if(verbose_debug) {
		totalSizeOfAllocatedMemory += size;
		DEBUG_P("totalSizeOfAllocatedMemory:%ld\n", totalSizeOfAllocatedMemory);
	}
	return ptr;
}

static void dse_free(void *ptr, size_t size)
{
	free(ptr);
	if(verbose_debug) {
		totalSizeOfAllocatedMemory -= size;
		DEBUG_P("totalSizeOfAllocatedMemory:%ld\n", totalSizeOfAllocatedMemory);
		DEBUG_ASSERT(totalSizeOfAllocatedMemory >= 0);
	}
}

/* Message */
struct Message {
	unsigned char *data;
	size_t         len;
};
typedef struct Message Message;

static Message *Message_new(unsigned char *requestLine, size_t length);
static void     Message_delete(Message *msg);

/* Scheduler */
#include <pthread.h>

#define MSG_QUEUE_SIZE 16
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
static void       Scheduler_delete(Scheduler *scheduler);
static bool       Scheduler_enqueue(Scheduler *scheduler, Message *msg);
static Message   *Scheduler_dequeue(Scheduler *scheduler);

/* DSE */
#include <event.h>
#include <evhttp.h>

struct DSE {
	int                port;
	int                threadsize;
	char              *scriptdir;
	struct event_base *base;
	struct evhttp     *httpd;
	Scheduler         *scheduler;
	struct KonohaFactory     *factory;
};
typedef struct DSE DSE;

static DSE *DSE_new(void);
static void DSE_start(DSE *dse);
static void DSE_delete(DSE *dse);

//kstatus_t MODSUGAR_Eval(KonohaContext *kctx, const char *script, size_t len, kfileline_t uline);
//kstatus_t MODSUGAR_loadScript(KonohaContext *kctx, const char *path, size_t len, KTraceInfo *trace);

// -------------------------------------------------------------------------
// getopt

#include <minikonoha/platform.h>
//#include <minikonoha/libcode/minishell.h>

// -------------------------------------------------------------------------
// KonohaContext

#ifdef _MSC_VER
#define strcasecmp stricmp
#endif

//static void CommandLine_Define(KonohaContext *kctx, char *keyvalue, KTraceInfo *trace)
//{
//	char *p = strchr(keyvalue, '=');
//	if(p != NULL) {
//		size_t len = p-keyvalue;
//		char *namebuf = ALLOCA(char, len+1);
//		memcpy(namebuf, keyvalue, len); namebuf[len] = 0;
////		DBG_P("name='%s'", namebuf);
//		ksymbol_t key = KLIB Ksymbol(kctx, namebuf, len, 0, Symbol_NewId);
//		uintptr_t unboxValue;
//		ktypeattr_t ty;
//		if(isdigit(p[1])) {
//			ty = KType_int;
//			unboxValue = (uintptr_t)strtol(p+1, NULL, 0);
//		}
//		else {
//			ty = VirtualType_Text;
//			unboxValue = (uintptr_t)(p+1);
//		}
//		KLIB kNameSpace_SetConstData(kctx, KNULL(NameSpace), key, ty, unboxValue, true/*isOverride*/, trace);
//	}
//	else {
//		fprintf(stdout, "invalid define option: use -D<key>=<value>\n");
//		KExit(EXIT_FAILURE);
//	}
//}

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
	Scheduler *scheduler = dse_malloc(sizeof(Scheduler));
	scheduler->front = 0;
	scheduler->last = 0;
	pthread_mutex_init(&scheduler->lock, NULL);
	pthread_cond_init(&scheduler->cond, NULL);
	return scheduler;
}

static void Scheduler_delete(Scheduler *scheduler)
{
	pthread_mutex_destroy(&scheduler->lock);
	pthread_cond_destroy(&scheduler->cond),
		dse_free(scheduler, sizeof(Scheduler));
}

static bool Scheduler_enqueue(Scheduler *scheduler, Message *msg)
{
	int front = scheduler->front;
	int last = scheduler->last;
	if(next(front) == last) return false;
	scheduler->msgQueue[front] = msg;
	scheduler->front = next(front);
	return true;
}

static Message *Scheduler_dequeue(Scheduler *scheduler)
{
	int front = scheduler->front;
	int last = scheduler->last;
	if(front == last) return NULL;
	Message *msg = scheduler->msgQueue[last];
	scheduler->last = next(last);
	return msg;
}

// ---------------------------------------------------------------------------
// [DSE]

static DSE *DSE_new(void)
{
	static const char scriptdir[] = "./";
	DSE *dse = (DSE *)dse_malloc(sizeof(DSE));
	dse->port = 8080;
	dse->threadsize = 16;
	dse->scriptdir = (char *)scriptdir;
	dse->base = event_base_new();
	dse->httpd = evhttp_new(dse->base);
	dse->scheduler = Scheduler_new();
	return dse;
}

int verbose_code;
extern int verbose_sugar;

static struct option long_options2[] = {
	/* These options set a flag. */
	{"verbose",         no_argument,       &verbose_debug, 1},   // for debug
	{"verbose:sugar",   no_argument,       &verbose_sugar, 1},   // for debug
	{"verbose:code",    no_argument,       &verbose_code,  1},   // for debug
	{"port",            required_argument, 0,              'p'}, // port
	{"threadsize",      required_argument, 0,              't'}, // thread size
	{"scriptdir",       required_argument, 0,              'D'}, // directory of the acquired script
	{NULL, 0, 0, 0},  /* sentinel */
};

static void DSE_ParseCommandOption(DSE* dse, int argc, char **argv)
{
	char *e;

	while (1) {
		int option_index = 0;
		int c = getopt_long (argc, argv, "p:t:D:", long_options2, &option_index);
		if(c == -1) break; /* Detect the end of the options. */
		switch (c) {
			case 'p':
				dse->port = (int)strtol(optarg, &e, 10);
				break;

			case 't':
				dse->threadsize = (int)strtol(optarg, &e, 10);
				break;

			case 'D':
				dse->scriptdir = optarg;
				break;

			case '?':
				/* getopt_long already printed an error message. */
				fprintf(stderr, "Usage: COMMAND [ --verbose ] [ --verbose:sugar ] [ --verbose:code ] [ --port | -p ] port [ --threadsize | -t ] size [ --scriptdir | -D ] directory\n");
				break;

			default:
				DEBUG_ABORT();
				return;
		}
	}
}

static const char *DSE_formatScriptPath(char *buf, size_t bufsiz)
{
	const char *path = getenv("KONOHA_HOME");
	const char *local = "/dse";
	if(path == NULL) {
		path = getenv("HOME");
		local = "/.minikonoha/dse";
	}
	snprintf(buf, bufsiz, "%s%s/dse.k", path, local);
#ifdef K_PREFIX
	if(!HasFile(buf)) {
		snprintf(buf, bufsiz, K_PREFIX "/lib/minikonoha/" K_VERSION "/dse/dse.k");
	}
#endif
	return HasFile(buf) ? (const char *)buf : NULL;
}

static void DSE_defineConstData(DSE *dse, KonohaContext *kctx, Message *msg)
{
	if(msg != NULL) {
		KDEFINE_TEXT_CONST TextData[] = {
			{"DSE_MESSAGE", VirtualType_Text, (char *)msg->data},
			{"DSE_SCRIPT_DIR", VirtualType_Text, dse->scriptdir},
			{}
		};
		KLIB kNameSpace_LoadConstData(kctx, KNULL(NameSpace), KonohaConst_(TextData), false/*isOverride*/, 0);
	}
	else {
		KDEFINE_TEXT_CONST TextData[] = {
			{"DSE_SCRIPT_DIR", VirtualType_Text, dse->scriptdir},
			{}
		};
		KLIB kNameSpace_LoadConstData(kctx, KNULL(NameSpace), KonohaConst_(TextData), false/*isOverride*/, 0);
	}
	KDEFINE_INT_CONST IntData[] = {
		{"DSE_DEBUG", KType_int, verbose_debug}, {}
	};
	KLIB kNameSpace_LoadConstData(kctx, KNULL(NameSpace), KonohaConst_(IntData), false/*isOverride*/, 0);
}

KonohaContext* KonohaFactory_CreateKonoha(KonohaFactory *factory);
int Konoha_Destroy(KonohaContext *kctx);

#define PATH_SIZE 256

static void *DSE_dispatch(void *arg)
{
	DSE *dse = (DSE *)arg;
	Scheduler *scheduler = dse->scheduler;
	KonohaFactory *factory = dse->factory;
	Message *msg;
	kbool_t ret;
	char script[PATH_SIZE];
	DSE_formatScriptPath(script, PATH_SIZE);

	while(true) {
		pthread_mutex_lock(&scheduler->lock);
		if(!(msg = Scheduler_dequeue(scheduler))) {
			pthread_cond_wait(&scheduler->cond, &scheduler->lock);
			msg = Scheduler_dequeue(scheduler);
		}
		pthread_mutex_unlock(&scheduler->lock);
		DEBUG_P("%s", msg->data);
		KonohaContext* konoha = KonohaFactory_CreateKonoha(factory);
		DSE_defineConstData(dse, konoha, msg);
		ret = Konoha_LoadScript(konoha, script);
		Konoha_Destroy(konoha);
		Message_delete(msg);
	}
	return NULL;
}

static void DSE_requestHandler(struct evhttp_request *request, void *arg)
{
	struct evbuffer *body = evhttp_request_get_input_buffer(request);
	size_t len = evbuffer_get_length(body);
	DSE *dse = (DSE *)arg;
	Scheduler *scheduler = dse->scheduler;
	Message *msg = NULL;
	unsigned char *requestLine;
	struct evbuffer *buf;

	switch(request->type) {
		case EVHTTP_REQ_POST :
			// now, we fetch message
			requestLine = evbuffer_pullup(body, -1);
			msg = Message_new(requestLine, len);
			pthread_mutex_lock(&scheduler->lock);
			if(Scheduler_enqueue(scheduler, msg)) {
				pthread_mutex_unlock(&scheduler->lock);
				pthread_cond_signal(&scheduler->cond);
				buf = evbuffer_new();
				evhttp_send_reply(request, HTTP_OK, "OK", buf);
				evbuffer_free(buf);
				break;
			}
			Message_delete(msg);
			pthread_mutex_unlock(&scheduler->lock);
			evhttp_send_error(request, HTTP_BADREQUEST, "DSE's message queue is full");
			break;
		default :
			evhttp_send_error(request, HTTP_BADREQUEST, "Available POST only");
			break;
	}
}

static void DSE_start(DSE *dse)
{
	if(evhttp_bind_socket(dse->httpd, "127.0.0.1", dse->port) < 0){
		DEBUG_ABORT();
	}

	int i;
	pthread_t threadPool[dse->threadsize];
	struct KonohaFactory factory = {};
	PosixFactory(&factory);
	factory.name = "dse";
	dse->factory = &factory;

	for(i = 0; i < dse->threadsize; i++) {
		pthread_create(&threadPool[i], NULL, DSE_dispatch, (void *)dse);
	}
	evhttp_set_gencb(dse->httpd, DSE_requestHandler, (void *)dse);
	event_base_dispatch(dse->base);
}

static void DSE_delete(DSE *dse)
{
	evhttp_free(dse->httpd);
	event_base_free(dse->base);
	Scheduler_delete(dse->scheduler);
	dse_free(dse, sizeof(DSE));
}

// -------------------------------------------------------------------------
// ** main **

int main(int argc, char *argv[])
{
	if(getenv("DSE_DEBUG") != NULL) {
		verbose_debug = 1;
		verbose_sugar = 1;
		verbose_code = 1;
	}
	DSE* dse = DSE_new();
	DSE_ParseCommandOption(dse, argc, argv);
	DSE_start(dse);
	DSE_delete(dse);
	return 0;
}

#ifdef __cplusplus
}
#endif
