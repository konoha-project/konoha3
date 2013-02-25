/****************************************************************************
 * Copyright (c) 2013, the Konoha project authors. All rights reserved.
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

#include <konoha3/konoha.h>
#include <konoha3/konoha_common.h>
#include <konoha3/sugar.h>
#include <konoha3/import/methoddecl.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#ifdef __cplusplus
extern "C" {
#endif


/* ======================================================================== */
typedef struct cevent_base {
	kObjectHeader h;
	struct event_base *event_base;
} kcevent_base;

typedef struct cevent {
	kObjectHeader h;
	struct event *event;
} kcevent;

typedef struct cbufferevent {
	kObjectHeader h;
	struct bufferevent *bev;
} kcbufferevent;

typedef struct eventCBArg {	//callback-method argument wrapper
	kObjectHeader h;
	KonohaContext *kctx;
	kFunc *kcb;		// konoha call back method
	kObject *arg;
} keventCBArg;

enum e_buffereventCB {BEV_ReadCB, BEV_WriteCB, BEV_EventCB, NUM_BuffereventCB};
typedef struct buffereventCBArg {	//callback-method argument wrapper
	kObjectHeader h;
	KonohaContext *kctx;
	kcbufferevent *cbev;	//'cbev' is set in cbufferevent_setcb() function
	kFunc *kcb[NUM_BuffereventCB];		// konoha call back methods
	kObject *arg;
} kbuffereventCBArg;

#include <sys/time.h>
typedef struct ctimeval {
	kObjectHeader h;
	struct timeval timeval;
} kctimeval;

// TODO ----- should be implement in posix.socket package
#include <sys/socket.h>
typedef struct Sockaddr_in {
	kObjectHeader h;
	struct sockaddr_in sockaddr;
} kSockaddr_in;
// TODO should be implement in posix.socket package -----


/* ======================================================================== */
// cevent_base class
static void cevent_base_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct cevent_base *ev = (struct cevent_base *) o;
	ev->event_base = NULL;
}

static void cevent_base_Free(KonohaContext *kctx, kObject *o)
{
	struct cevent_base *ev = (struct cevent_base *) o;
	if(ev->event_base != NULL) {
		event_base_free(ev->event_base);
		ev->event_base = NULL;
	}
}

//## cevent_base cevent_base.new();
static KMETHOD cevent_base_new(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevent_base *ev = (kcevent_base *)sfp[0].asObject;
	ev->event_base = event_base_new();
	KReturn(ev);
}

//## cevent_base cevent_base.event_dispatch();
static KMETHOD cevent_base_event_dispatch(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevent_base *ev = (kcevent_base *)sfp[0].asObject;
	int ret = event_base_dispatch(ev->event_base);
	KReturnUnboxValue(ret);
}

//## int cevent_base.event_loopbreak();
static KMETHOD cevent_base_event_loopbreak(KonohaContext *kctx, KonohaStack *sfp)
{
	kcevent_base *ev = (kcevent_base *)sfp[0].asObject;
	int ret = event_base_loopbreak(ev->event_base);
	KReturnUnboxValue(ret);
}

//## int cevent_base.evutil_make_socket_nonblocking(int fd);
static KMETHOD cevent_base_evutil_make_socket_nonblocking(KonohaContext *kctx, KonohaStack* sfp)
{
	evutil_socket_t evd = (evutil_socket_t)sfp[1].intValue;
	int ret = evutil_make_socket_nonblocking(evd);
	KReturnUnboxValue(ret);
}

/*
 * cevent_base Class 1st stage callback from event_base_dispatch(), NEVER BE CALLED FROM OTHERS.
 */
static void cevent_callback_1st(evutil_socket_t evd, short event, void *arg) {
	keventCBArg *cbArg = arg;
	KonohaContext *kctx = cbArg->kctx;

	BEGIN_UnusedStack(lsfp);
	KClass *returnType = kMethod_GetReturnType(cbArg->kcb->method);
	KUnsafeFieldSet(lsfp[0].asObject, K_NULL);
	lsfp[1].intValue = evd;
	lsfp[2].intValue = event;
	KUnsafeFieldSet(lsfp[3].asObject, (kObject *)cbArg->arg);

	KStackSetFuncAll(lsfp, KLIB Knull(kctx, returnType), 0/*UL*/, cbArg->kcb, 3);
	KStackCall(lsfp);
	END_UnusedStack();
}

int event_reinit(struct event_base *base);
const char *event_base_get_method(const struct event_base *);
int event_base_get_features(const struct event_base *base);
struct event_base *event_base_new_with_config(const struct event_config *);
int event_base_loop(struct event_base *, int);
int event_base_loopexit(struct event_base *, const struct timeval *);
int event_base_got_exit(struct event_base *);
int event_base_got_break(struct event_base *);
int event_base_once(struct event_base *, evutil_socket_t, short, event_callback_fn, void *, const struct timeval *);	//TODO no need?
int event_base_priority_init(struct event_base *, int);
int event_priority_set(struct event *, int);
const struct timeval *event_base_init_common_timeout(struct event_base *base,
	const struct timeval *duration);
void event_base_dump_events(struct event_base *, FILE *);
int event_base_gettimeofday_cached(struct event_base *base, struct timeval *tv);

-- event_config --
const char **event_get_supported_methods(void);
struct event_config *event_config_new(void);
void event_config_free(struct event_config *cfg);
int event_config_avoid_method(struct event_config *cfg, const char *method);
int event_config_require_features(struct event_config *cfg, int feature);
int event_config_set_flag(struct event_config *cfg, int flag);
int event_config_set_num_cpus_hint(struct event_config *cfg, int cpus);


-- event log --
void event_set_log_callback(event_log_cb cb);


void event_set_fatal_callback(event_fatal_cb cb);


const char *event_get_version(void);
ev_uint32_t event_get_version_number(void);
void event_set_mem_functions(
	void *(*malloc_fn)(size_t sz),
	void *(*realloc_fn)(void *ptr, size_t sz),
	void (*free_fn)(void *ptr));



/* ======================================================================== */
// cevent class

static void cevent_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct cevent *ev = (struct cevent *) o;
	ev->event = NULL;
}

static void cevent_Free(KonohaContext *kctx, kObject *o)
{
	struct cevent *ev = (struct cevent *) o;

	if(ev->event != NULL) {
		event_free(ev->event);
		ev->event = NULL;
	}
}

//static void cevent_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
//{
//	struct cevent *ev = (struct cevent *) o;
//}

//## cevent cevent.new(cevent_base event_base, int evd, int event, eventCBArg cbArg);
static KMETHOD cevent_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct cevent *ev = (struct cevent *) sfp[0].asObject;
	struct cevent_base *cEvent_base = (struct cevent_base *)sfp[1].asObject;
	evutil_socket_t evd = (evutil_socket_t)sfp[2].intValue;
	short event = (short)(sfp[3].intValue & 0xffff);
	keventCBArg *cbArg = (keventCBArg *)sfp[4].asObject;	//deliver callback method

	ev->event = event_new(cEvent_base->event_base, evd, event, cevent_callback_1st, cbArg);
	KReturn(ev);
}

//## cevent cevent.event_assign(cevent_base event_base, int evd, int event, eventCBArg cbArg);
static KMETHOD cevent_event_assign(KonohaContext *kctx, KonohaStack *sfp)
{
	struct cevent *ev = (struct cevent *) sfp[0].asObject;
	struct cevent_base *cEvent_base = (struct cevent_base *)sfp[1].asObject;
	evutil_socket_t evd = (evutil_socket_t)sfp[2].intValue;
	short event = (short)(sfp[3].intValue & 0xffff);
	keventCBArg *cbArg = (keventCBArg *)sfp[4].asObject;	//deliver callback method

	int ret = event_assign(ev->event, cEvent_base->event_base, evd, event, cevent_callback_1st, cbArg);
	KReturnUnboxValue(ret);
}

#define tvIsNull(tv_p)	((tv_p)->timeval.tv_sec == 0 && (tv_p)->timeval.tv_usec == 0)
//## int cevent.event_add(cevent_base event, ctimeval tv);
static KMETHOD cevent_event_add(KonohaContext *kctx, KonohaStack* sfp)
{
	kcevent *kcev = (kcevent *)sfp[0].asObject;
	kctimeval *tv = (kctimeval *)sfp[1].asObject;
	int ret = event_add(kcev->event, tvIsNull(tv) ? NULL : &tv->timeval);
	KReturnUnboxValue(ret);
}

//## int cevent.event_del(cevent event);
static KMETHOD cevent_event_del(KonohaContext *kctx, KonohaStack* sfp)
{
	kcevent *kcev = (kcevent *)sfp[0].asObject;
	int ret = event_del(kcev->event);
	KReturnUnboxValue(ret);
}

//## int cevent.event_pending(short events, ctimeval tv);
static KMETHOD cevent_event_pending(KonohaContext *kctx, KonohaStack* sfp)
{
	kcevent *kcev = (kcevent *)sfp[0].asObject;
	short events = (short)sfp[1].intValue;
	kctimeval *tv = (kctimeval *)sfp[2].asObject;
	int ret = event_pending(kcev->event, events, tvIsNull(tv) ? NULL : &tv->timeval);
	KReturnUnboxValue(ret);
}

//## int cevent.event_initialized();
static KMETHOD cevent_event_initialized(KonohaContext *kctx, KonohaStack* sfp)
{
	kcevent *kcev = (kcevent *)sfp[0].asObject;
	int ret = event_initialized(kcev->event);
	KReturnUnboxValue(ret);
}

//## void cevent.event_free();
static KMETHOD cevent_event_free(KonohaContext *kctx, KonohaStack *sfp)
{
	struct cevent *ev = (struct cevent *) sfp[0].asObject;
	event_free(ev->event);
	KReturnVoid();
}

//## void cevent.event_active(int res, int ncalls);
static KMETHOD cevent_event_active(KonohaContext *kctx, KonohaStack *sfp)
{
	struct cevent *ev = (struct cevent *) sfp[0].asObject;
	int res = sfp[1].intValue;
	short ncalls = (short)sfp[2].intValue;
	event_active(ev->event, res, ncalls);
	KReturnVoid();
}

/*
USE event.signal_new() in Libevent_kick.k
//## cevent cevent.signal_new(cevent_base event_base, int signo, eventCBArg cbArg);
static KMETHOD cevent_signal_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct cevent *ev = (struct cevent *) sfp[0].asObject;
	struct cevent_base *cEvent_base = (struct cevent_base *)sfp[1].asObject;
	evutil_socket_t signo = (evutil_socket_t)sfp[2].intValue;
	keventCBArg *cbArg = (keventCBArg *)sfp[3].asObject;	//deliver callback method

	ev->event = evsignal_new(cEvent_base->event_base, signo, cevent_callback_1st, cbArg);
	KReturn(ev);
}
*/

/*
USE event.signal_assign() in Libevent_kick.k
//## cevent cevent.signal_assign(cevent_base event_base, int evd, int event, eventCBArg cbArg);
static KMETHOD cevent_signal_assign(KonohaContext *kctx, KonohaStack *sfp)
{
	struct cevent *ev = (struct cevent *) sfp[0].asObject;
	struct cevent_base *cEvent_base = (struct cevent_base *)sfp[1].asObject;
	evutil_socket_t evd = (evutil_socket_t)sfp[2].intValue;
	short event = (short)(sfp[3].intValue & 0xffff);
	keventCBArg *cbArg = (keventCBArg *)sfp[4].asObject;	//deliver callback method

	int ret = evsignal_assign(ev->event, cEvent_base->event_base, evd, event, cevent_callback_1st, cbArg);
	KReturnUnboxValue(ret);
}
*/

/*
USE event.signal_add(), signal_del in Libevent_kick.k
//## int cevent.signal_add(cevent_base event, ctimeval tv);
static KMETHOD cevent_signal_add(KonohaContext *kctx, KonohaStack* sfp)
{
	kcevent *kcev = (kcevent *)sfp[0].asObject;
	kctimeval *tv = (kctimeval *)sfp[1].asObject;
	int ret = event_add(kcev->event, (tv->timeval.tv_sec == 0 && tv->timeval.tv_usec == 0) ? NULL : &tv->timeval);
	KReturnUnboxValue(ret);
}

//## int cevent.signal_del(cevent event);
static KMETHOD cevent_signal_del(KonohaContext *kctx, KonohaStack* sfp)
{
	kcevent *kcev = (kcevent *)sfp[0].asObject;
	int ret = event_del(kcev->event);
	KReturnUnboxValue(ret);
}
*/

//## cevent cevent.getID();
static KMETHOD cevent_getID(KonohaContext *kctx, KonohaStack *sfp)
{
	struct cevent *ev = (struct cevent *) sfp[0].asObject;
	KReturnUnboxValue((uintptr_t)ev->event);
}

//## cevent cevent.getEvents();
// get event category field
static KMETHOD cevent_getEvents(KonohaContext *kctx, KonohaStack *sfp)
{
	struct cevent *ev = (struct cevent *) sfp[0].asObject;
	KReturnUnboxValue(ev->event->ev_events);
}


int event_base_set(struct event_base *, struct event *);
evutil_socket_t event_get_fd(const struct event *ev);
struct event_base *event_get_base(const struct event *ev);
short event_get_events(const struct event *ev);
event_callback_fn event_get_callback(const struct event *ev);
void *event_get_callback_arg(const struct event *ev);
void event_get_assignment(const struct event *event,
    struct event_base **base_out, evutil_socket_t *fd_out, short *events_out,
    event_callback_fn *callback_out, void **arg_out);




/* ======================================================================== */
// cbufferevent class

static void cbufferevent_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct cbufferevent *bev = (struct cbufferevent *) o;
	bev->bev = NULL;
}

static void cbufferevent_Free(KonohaContext *kctx, kObject *o)
{
	struct cbufferevent *bev = (struct cbufferevent *) o;

	if (bev->bev != NULL) {
		bufferevent_free(bev->bev);
		bev->bev = NULL;
	}
}

//static void cbufferevent_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
//{
//	struct cbufferevent *bev = (struct cbufferevent *) o;
//}

//## cbufferevent cbufferevent.new(cevent_base event_base, int evd, int option);
static KMETHOD cbufferevent_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct cbufferevent *bev = (struct cbufferevent *)sfp[0].asObject;
	struct cevent_base *cev_base = (struct cevent_base *)sfp[1].asObject;
	evutil_socket_t evd = (evutil_socket_t)sfp[2].intValue;
	int options = sfp[3].intValue;

	bev->bev = bufferevent_socket_new(cev_base->event_base, evd, options);
	KReturn(bev);
}

static void Cbev_dataCB_dispatcher(enum e_buffereventCB cat, struct bufferevent *bev, void *arg)
{
	kbuffereventCBArg *cbArg = arg;
	KonohaContext *kctx = cbArg->kctx;

	BEGIN_UnusedStack(lsfp);
	KClass *returnType = kMethod_GetReturnType(cbArg->kcb[cat]->method);
	KUnsafeFieldSet(lsfp[0].asObject, K_NULL);
	KUnsafeFieldSet(lsfp[1].asObject, (kObject *)cbArg->cbev);
	KUnsafeFieldSet(lsfp[2].asObject, (kObject *)cbArg->arg);

	KStackSetFuncAll(lsfp, KLIB Knull(kctx, returnType), 0/*UL*/, cbArg->kcb[cat], 2);
	KStackCall(lsfp);
	END_UnusedStack();
}

/*
 * cbufferevent Class (*buffer_data_cb)() 1st stage callback from event_base_dispatch(), NEVER BE CALLED FROM OTHERS.
 */
static void Cbev_readCB_1st(struct bufferevent *bev, void *arg)
{ Cbev_dataCB_dispatcher(BEV_ReadCB, bev, arg); }

/*
 * cbufferevent Class (*buffer_data_cb)() 1st stage callback from event_base_dispatch(), NEVER BE CALLED FROM OTHERS.
 */
static void Cbev_writeCB_1st(struct bufferevent *bev, void *arg)
{ Cbev_dataCB_dispatcher(BEV_WriteCB, bev, arg); }

/*
 * cbufferevent Class (*buffer_event_cb)() 1st stage callback from event_base_dispatch(), NEVER BE CALLED FROM OTHERS.
 */
static void Cbev_eventCB_1st(struct bufferevent *bev, short what, void *arg)
{
	kbuffereventCBArg *cbArg = arg;
	KonohaContext *kctx = cbArg->kctx;

	BEGIN_UnusedStack(lsfp);
	KClass *returnType = kMethod_GetReturnType(cbArg->kcb[BEV_EventCB]->method);
	KUnsafeFieldSet(lsfp[0].asObject, K_NULL);
	KUnsafeFieldSet(lsfp[1].asObject, (kObject *)cbArg->cbev);
	lsfp[2].intValue = what;
	KUnsafeFieldSet(lsfp[3].asObject, (kObject *)cbArg->arg);
	KStackSetFuncAll(lsfp, KLIB Knull(kctx, returnType), 0/*UL*/, cbArg->kcb[BEV_EventCB], 3);
	KStackCall(lsfp);
	END_UnusedStack();
}

//## void cbufferevent.setcb(buffereventCBArg cbarg);
static KMETHOD cbufferevent_setcb(KonohaContext *kctx, KonohaStack *sfp)
{
	kcbufferevent *bev = (kcbufferevent *)sfp[0].asObject;
	kbuffereventCBArg *cbArg = (kbuffereventCBArg *)sfp[1].asObject;

	KFieldSet(cbArg, cbArg->cbev, bev);
	bufferevent_setcb(bev->bev, Cbev_readCB_1st, Cbev_writeCB_1st, Cbev_eventCB_1st, cbArg);
	KReturnVoid();
}

//## int cbufferevent.socket_connect(Sockaddr_in sa);
static KMETHOD cbufferevent_socket_connect(KonohaContext *kctx, KonohaStack *sfp)
{
	kcbufferevent *bev = (kcbufferevent *)sfp[0].asObject;
	kSockaddr_in *sa = (kSockaddr_in *)sfp[1].asObject;
	int ret = bufferevent_socket_connect(bev->bev, (struct sockaddr *)&sa->sockaddr, sizeof sa->sockaddr);
	KReturnUnboxValue(ret);
}

//## int cbufferevent.enable(int event);
static KMETHOD cbufferevent_enable(KonohaContext *kctx, KonohaStack *sfp)
{
	kcbufferevent *bev = (kcbufferevent *)sfp[0].asObject;
	short event = (short)sfp[1].intValue;

	int ret = bufferevent_enable(bev->bev, event);
	KReturnUnboxValue(ret);
}

//## int cbufferevent.write(Bytes buf);
static KMETHOD cbufferevent_write(KonohaContext *kctx, KonohaStack *sfp)
{
	kcbufferevent *bev = (kcbufferevent *)sfp[0].asObject;
	kBytes *buf = sfp[1].asBytes;

	int ret = bufferevent_write(bev->bev, buf->byteptr, buf->bytesize);
	KReturnUnboxValue(ret);
}

//## int cbufferevent.bufferevent_read(Bytes buf);
static KMETHOD cbufferevent_read(KonohaContext *kctx, KonohaStack *sfp)
{
	kcbufferevent *bev = (kcbufferevent *)sfp[0].asObject;
	kBytes *buf = sfp[1].asBytes;

	int ret = bufferevent_read(bev->bev, buf->buf, buf->bytesize);
	KReturnUnboxValue(ret);
}


int bufferevent_socket_connect_hostname(struct bufferevent *,
	struct evdns_base *, int, const char *, int);
int bufferevent_socket_get_dns_error(struct bufferevent *bev);
int bufferevent_base_set(struct event_base *base, struct bufferevent *bufev);
struct event_base *bufferevent_get_base(struct bufferevent *bev);
int bufferevent_priority_set(struct bufferevent *bufev, int pri);
int bufferevent_setfd(struct bufferevent *bufev, evutil_socket_t fd);
evutil_socket_t bufferevent_getfd(struct bufferevent *bufev);
struct bufferevent *bufferevent_get_underlying(struct bufferevent *bufev);
int bufferevent_write_buffer(struct bufferevent *bufev, struct evbuffer *buf);
int bufferevent_read_buffer(struct bufferevent *bufev, struct evbuffer *buf);
struct evbuffer *bufferevent_get_input(struct bufferevent *bufev);
struct evbuffer *bufferevent_get_output(struct bufferevent *bufev);
int bufferevent_disable(struct bufferevent *bufev, short event);
short bufferevent_get_enabled(struct bufferevent *bufev);
int bufferevent_set_timeouts(struct bufferevent *bufev,
	const struct timeval *timeout_read, const struct timeval *timeout_write);
void bufferevent_setwatermark(struct bufferevent *bufev, short events,
	size_t lowmark, size_t highmark);
void bufferevent_lock(struct bufferevent *bufev);
void bufferevent_unlock(struct bufferevent *bufev);
int bufferevent_flush(struct bufferevent *bufev, short iotype,
	enum bufferevent_flush_mode mode);
struct bufferevent *
bufferevent_filter_new(struct bufferevent *underlying,
		       bufferevent_filter_cb input_filter,
		       bufferevent_filter_cb output_filter,
		       int options,
		       void (*free_context)(void *),
		       void *ctx);
int bufferevent_pair_new(struct event_base *base, int options,
	struct bufferevent *pair[2]);
struct bufferevent *bufferevent_pair_get_partner(struct bufferevent *bev);
int bufferevent_set_rate_limit(struct bufferevent *bev,
    struct ev_token_bucket_cfg *cfg);
struct bufferevent_rate_limit_group *bufferevent_rate_limit_group_new(
	struct event_base *base,
	const struct ev_token_bucket_cfg *cfg);
int bufferevent_rate_limit_group_set_cfg(
	struct bufferevent_rate_limit_group *,
	const struct ev_token_bucket_cfg *);
int bufferevent_add_to_rate_limit_group(struct bufferevent *bev,
	struct bufferevent_rate_limit_group *g);
int bufferevent_remove_from_rate_limit_group(struct bufferevent *bev);
ev_ssize_t bufferevent_get_read_limit(struct bufferevent *bev);
ev_ssize_t bufferevent_get_write_limit(struct bufferevent *bev);
ev_ssize_t bufferevent_get_max_to_read(struct bufferevent *bev);
ev_ssize_t bufferevent_get_max_to_write(struct bufferevent *bev);
int bufferevent_decrement_read_limit(struct bufferevent *bev, ev_ssize_t decr);
int bufferevent_decrement_write_limit(struct bufferevent *bev, ev_ssize_t decr);

-- ev_token_bucket_cfg --
struct ev_token_bucket_cfg *ev_token_bucket_cfg_new(
	size_t read_rate, size_t read_burst,
	size_t write_rate, size_t write_burst,
	const struct timeval *tick_len);
void ev_token_bucket_cfg_free(struct ev_token_bucket_cfg *cfg);


-- bufferevent_rate_limit_group --
int bufferevent_rate_limit_group_set_min_share(
	struct bufferevent_rate_limit_group *, size_t);
void bufferevent_rate_limit_group_free(struct bufferevent_rate_limit_group *);
ev_ssize_t bufferevent_rate_limit_group_get_read_limit(
	struct bufferevent_rate_limit_group *);
ev_ssize_t bufferevent_rate_limit_group_get_write_limit(
	struct bufferevent_rate_limit_group *);
int bufferevent_rate_limit_group_decrement_read(
	struct bufferevent_rate_limit_group *, ev_ssize_t);
int bufferevent_rate_limit_group_decrement_write(
	struct bufferevent_rate_limit_group *, ev_ssize_t);
void bufferevent_rate_limit_group_get_totals(
	struct bufferevent_rate_limit_group *grp,
	ev_uint64_t *total_read_out, ev_uint64_t *total_written_out);
void bufferevent_rate_limit_group_reset_totals(
	struct bufferevent_rate_limit_group *grp);





/* ======================================================================== */
// evbuffer class
int evbuffer_enable_locking(struct evbuffer *buf, void *lock);
void evbuffer_lock(struct evbuffer *buf);
void evbuffer_unlock(struct evbuffer *buf);
int evbuffer_set_flags(struct evbuffer *buf, ev_uint64_t flags);
int evbuffer_clear_flags(struct evbuffer *buf, ev_uint64_t flags);
size_t evbuffer_get_length(const struct evbuffer *buf);
size_t evbuffer_get_contiguous_space(const struct evbuffer *buf);
int evbuffer_expand(struct evbuffer *buf, size_t datlen);
int evbuffer_reserve_space(struct evbuffer *buf, ev_ssize_t size,
	struct evbuffer_iovec *vec, int n_vec);
int evbuffer_commit_space(struct evbuffer *buf,
	struct evbuffer_iovec *vec, int n_vecs);
int evbuffer_add(struct evbuffer *buf, const void *data, size_t datlen);
int evbuffer_remove(struct evbuffer *buf, void *data, size_t datlen);
ev_ssize_t evbuffer_copyout(struct evbuffer *buf, void *data_out, size_t datlen);
int evbuffer_remove_buffer(struct evbuffer *src, struct evbuffer *dst,
	size_t datlen);
char *evbuffer_readln(struct evbuffer *buffer, size_t *n_read_out,
	enum evbuffer_eol_style eol_style);
int evbuffer_add_buffer(struct evbuffer *outbuf, struct evbuffer *inbuf);
int evbuffer_add_reference(struct evbuffer *outbuf,
	const void *data, size_t datlen,
	evbuffer_ref_cleanup_cb cleanupfn, void *cleanupfn_arg);
int evbuffer_add_file(struct evbuffer *outbuf, int fd, ev_off_t offset,
	ev_off_t length);
int evbuffer_add_printf(struct evbuffer *buf, const char *fmt, ...);	//TODO use String?
int evbuffer_add_vprintf(struct evbuffer *buf, const char *fmt, va_list ap);
int evbuffer_drain(struct evbuffer *buf, size_t len);
int evbuffer_write(struct evbuffer *buffer, evutil_socket_t fd);
int evbuffer_write_atmost(struct evbuffer *buffer, evutil_socket_t fd,
	ev_ssize_t howmuch);
int evbuffer_read(struct evbuffer *buffer, evutil_socket_t fd, int howmuch);
struct evbuffer_ptr evbuffer_search(struct evbuffer *buffer, const char *what, size_t len, const struct evbuffer_ptr *start);
struct evbuffer_ptr evbuffer_search_range(struct evbuffer *buffer, const char *what, size_t len, const struct evbuffer_ptr *start, const struct evbuffer_ptr *end);
int evbuffer_ptr_set(struct evbuffer *buffer, struct evbuffer_ptr *ptr,
	size_t position, enum evbuffer_ptr_how how);
struct evbuffer_ptr evbuffer_search_eol(struct evbuffer *buffer,
	struct evbuffer_ptr *start, size_t *eol_len_out,
	enum evbuffer_eol_style eol_style);
int evbuffer_peek(struct evbuffer *buffer, ev_ssize_t len, struct evbuffer_ptr *start_at, struct evbuffer_iovec *vec_out, int n_vec);
struct evbuffer_cb_entry *evbuffer_add_cb(struct evbuffer *buffer, evbuffer_cb_func cb, void *cbarg);
int evbuffer_remove_cb_entry(struct evbuffer *buffer, struct evbuffer_cb_entry *ent);
int evbuffer_remove_cb(struct evbuffer *buffer, evbuffer_cb_func cb, void *cbarg);
int evbuffer_cb_set_flags(struct evbuffer *buffer, struct evbuffer_cb_entry *cb, ev_uint32_t flags);
int evbuffer_cb_clear_flags(struct evbuffer *buffer, struct evbuffer_cb_entry *cb, ev_uint32_t flags);
unsigned char *evbuffer_pullup(struct evbuffer *buf, ev_ssize_t size);
int evbuffer_prepend(struct evbuffer *buf, const void *data, size_t size);
int evbuffer_prepend_buffer(struct evbuffer *dst, struct evbuffer* src);
int evbuffer_freeze(struct evbuffer *buf, int at_front);
int evbuffer_unfreeze(struct evbuffer *buf, int at_front);
int evbuffer_defer_callbacks(struct evbuffer *buffer, struct event_base *base);



/* ======================================================================== */
// evhttp class
-- evhttp --
struct evhttp *evhttp_new(struct event_base *base);
int evhttp_bind_socket(struct evhttp *http, const char *address, ev_uint16_t port);
struct evhttp_bound_socket *evhttp_bind_socket_with_handle(struct evhttp *http, const char *address, ev_uint16_t port);
int evhttp_accept_socket(struct evhttp *http, evutil_socket_t fd);
struct evhttp_bound_socket *evhttp_accept_socket_with_handle(struct evhttp *http, evutil_socket_t fd);
struct evhttp_bound_socket *evhttp_bind_listener(struct evhttp *http, struct evconnlistener *listener);
void evhttp_del_accept_socket(struct evhttp *http, struct evhttp_bound_socket *bound_socket);
void evhttp_free(struct evhttp* http);
void evhttp_set_max_headers_size(struct evhttp* http, ev_ssize_t max_headers_size);
void evhttp_set_max_body_size(struct evhttp* http, ev_ssize_t max_body_size);
void evhttp_set_allowed_methods(struct evhttp* http, ev_uint16_t methods);
int evhttp_set_cb(struct evhttp *http, const char *path,
	void (*cb)(struct evhttp_request *, void *), void *cb_arg);
int evhttp_del_cb(struct evhttp *, const char *);
void evhttp_set_gencb(struct evhttp *http,
	void (*cb)(struct evhttp_request *, void *), void *arg);
int evhttp_add_virtual_host(struct evhttp* http, const char *pattern,
	struct evhttp* vhost);
int evhttp_remove_virtual_host(struct evhttp* http, struct evhttp* vhost);
int evhttp_add_server_alias(struct evhttp *http, const char *alias);
int evhttp_remove_server_alias(struct evhttp *http, const char *alias);
void evhttp_set_timeout(struct evhttp *http, int timeout_in_secs);


-- evhttp_bound_socket --
struct evconnlistener *evhttp_bound_socket_get_listener(struct evhttp_bound_socket *bound);
evutil_socket_t evhttp_bound_socket_get_fd(struct evhttp_bound_socket *bound_socket);


-- evhttp_request --
void evhttp_send_error(struct evhttp_request *req, int error,
	const char *reason);
void evhttp_send_reply(struct evhttp_request *req, int code,
	const char *reason, struct evbuffer *databuf);
void evhttp_send_reply_start(struct evhttp_request *req, int code,
	const char *reason);
void evhttp_send_reply_chunk(struct evhttp_request *req,
	struct evbuffer *databuf);
void evhttp_send_reply_end(struct evhttp_request *req);
struct evhttp_request *evhttp_request_new(
	void (*cb)(struct evhttp_request *, void *), void *arg);
void evhttp_request_set_chunked_cb(struct evhttp_request *,
	void (*cb)(struct evhttp_request *, void *));
void evhttp_request_free(struct evhttp_request *req);
void evhttp_request_own(struct evhttp_request *req);
int evhttp_request_is_owned(struct evhttp_request *req);
struct evhttp_connection *evhttp_request_get_connection(struct evhttp_request *req);
void evhttp_cancel_request(struct evhttp_request *req);
const char *evhttp_request_get_uri(const struct evhttp_request *req);
const struct evhttp_uri *evhttp_request_get_evhttp_uri(const struct evhttp_request *req);
enum evhttp_cmd_type evhttp_request_get_command(const struct evhttp_request *req);
int evhttp_request_get_response_code(const struct evhttp_request *req);
struct evkeyvalq *evhttp_request_get_input_headers(struct evhttp_request *req);
struct evkeyvalq *evhttp_request_get_output_headers(struct evhttp_request *req);
struct evbuffer *evhttp_request_get_input_buffer(struct evhttp_request *req);
struct evbuffer *evhttp_request_get_output_buffer(struct evhttp_request *req);
const char *evhttp_request_get_host(struct evhttp_request *req);


-- evhttp_connection --
struct evhttp_connection *evhttp_connection_base_new(
	struct event_base *base, struct evdns_base *dnsbase,
	const char *address, unsigned short port);
struct bufferevent *evhttp_connection_get_bufferevent(
	struct evhttp_connection *evcon);
struct event_base *evhttp_connection_get_base(struct evhttp_connection *req);
void evhttp_connection_set_max_headers_size(struct evhttp_connection *evcon,
	ev_ssize_t new_max_headers_size);
void evhttp_connection_set_max_body_size(struct evhttp_connection* evcon,
	ev_ssize_t new_max_body_size);
void evhttp_connection_free(struct evhttp_connection *evcon);
void evhttp_connection_set_local_address(struct evhttp_connection *evcon,
	const char *address);
void evhttp_connection_set_local_port(struct evhttp_connection *evcon,
	ev_uint16_t port);
void evhttp_connection_set_timeout(struct evhttp_connection *evcon,
	int timeout_in_secs);
void evhttp_connection_set_retries(struct evhttp_connection *evcon,
	int retry_max);
void evhttp_connection_set_closecb(struct evhttp_connection *evcon,
	void (*)(struct evhttp_connection *, void *), void *);
void evhttp_connection_get_peer(struct evhttp_connection *evcon,
	char **address, ev_uint16_t *port);
int evhttp_make_request(struct evhttp_connection *evcon,
	struct evhttp_request *req,
	enum evhttp_cmd_type type, const char *uri);


-- evkeyvalq --
const char *evhttp_find_header(const struct evkeyvalq *headers,
    const char *key);
int evhttp_remove_header(struct evkeyvalq *headers, const char *key);
int evhttp_add_header(struct evkeyvalq *headers, const char *key, const char *value);
void evhttp_clear_headers(struct evkeyvalq *headers);


-- uri --
struct evhttp_uri *evhttp_uri_new(void);
void evhttp_uri_set_flags(struct evhttp_uri *uri, unsigned flags);
const char *evhttp_uri_get_scheme(const struct evhttp_uri *uri);
const char *evhttp_uri_get_userinfo(const struct evhttp_uri *uri);
const char *evhttp_uri_get_host(const struct evhttp_uri *uri);
int evhttp_uri_get_port(const struct evhttp_uri *uri);
const char *evhttp_uri_get_path(const struct evhttp_uri *uri);
const char *evhttp_uri_get_query(const struct evhttp_uri *uri);
const char *evhttp_uri_get_fragment(const struct evhttp_uri *uri);
int evhttp_uri_set_scheme(struct evhttp_uri *uri, const char *scheme);
int evhttp_uri_set_userinfo(struct evhttp_uri *uri, const char *userinfo);
int evhttp_uri_set_host(struct evhttp_uri *uri, const char *host);
int evhttp_uri_set_port(struct evhttp_uri *uri, int port);
int evhttp_uri_set_path(struct evhttp_uri *uri, const char *path);
int evhttp_uri_set_query(struct evhttp_uri *uri, const char *query);
int evhttp_uri_set_fragment(struct evhttp_uri *uri, const char *fragment);
struct evhttp_uri *evhttp_uri_parse_with_flags(const char *source_uri,
	unsigned flags);
struct evhttp_uri *evhttp_uri_parse(const char *source_uri);
void evhttp_uri_free(struct evhttp_uri *uri);
char *evhttp_uri_join(struct evhttp_uri *uri, char *buf, size_t limit);


-- util --
char *evhttp_encode_uri(const char *str);
char *evhttp_uriencode(const char *str, ev_ssize_t size, int space_to_plus);
char *evhttp_decode_uri(const char *uri);
char *evhttp_uridecode(const char *uri, int decode_plus, size_t *size_out);
int evhttp_parse_query(const char *uri, struct evkeyvalq *headers);
int evhttp_parse_query_str(const char *uri, struct evkeyvalq *headers);
char *evhttp_htmlescape(const char *html);






/* ======================================================================== */
// eventCBArg class

static void eventCBArg_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct eventCBArg *cbarg = (struct eventCBArg *) o;
	cbarg->kctx = NULL;
	KFieldInit(cbarg, cbarg->kcb, K_NULL);
	KFieldInit(cbarg, cbarg->arg, K_NULL);
}

static void eventCBArg_Free(KonohaContext *kctx, kObject *o)
{
	struct eventCBArg *cbarg = (struct eventCBArg *) o;

	cbarg->kctx = NULL;
	KFieldInit(cbarg, cbarg->kcb, K_NULL);
	KFieldInit(cbarg, cbarg->arg, K_NULL);
}

static void eventCBArg_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	struct eventCBArg *cba = (struct eventCBArg *) o;
	KRefTrace(cba->kcb);
	KRefTrace(cba->arg);
}

//## eventCBArg eventCBArg.new(Func[void, int, Object arg] cb, Object cbArg);
static KMETHOD eventCBArg_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct eventCBArg *cbarg = (struct eventCBArg *) sfp[0].asObject;
	kFunc *cb = sfp[1].asFunc;
	kObjectVar *cbArg = sfp[2].asObjectVar;	//deliver callback method

	cbarg->kctx = kctx;
	KFieldSet(cbarg, cbarg->kcb, cb);
	KFieldSet(cbarg, cbarg->arg, cbArg);
	KReturn(cbarg);
}


/* ======================================================================== */
// buffereventCBArg class

static void buffereventCBArg_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct buffereventCBArg *bcbarg = (struct buffereventCBArg *) o;
	bcbarg->kctx = NULL;
	enum e_buffereventCB i;
	for (i = BEV_ReadCB; i < NUM_BuffereventCB; i++){
		KFieldInit(bcbarg, bcbarg->kcb[i], K_NULL);
	}
	KFieldInit(bcbarg, bcbarg->cbev, K_NULL);
	KFieldInit(bcbarg, bcbarg->arg, K_NULL);
}

static void buffereventCBArg_Free(KonohaContext *kctx, kObject *o)
{
	struct buffereventCBArg *bcbarg = (struct buffereventCBArg *) o;

	bcbarg->kctx = NULL;
	enum e_buffereventCB i;
	for (i = BEV_ReadCB; i < NUM_BuffereventCB; i++){
		KFieldInit(bcbarg, bcbarg->kcb[i], K_NULL);
	}
	KFieldInit(bcbarg, bcbarg->cbev, K_NULL);
	KFieldInit(bcbarg, bcbarg->arg, K_NULL);
}

static void buffereventCBArg_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
	struct buffereventCBArg *bcbarg = (struct buffereventCBArg *) o;
	enum e_buffereventCB i;
	for (i = BEV_ReadCB; i < NUM_BuffereventCB; i++){
		KRefTrace(bcbarg->kcb[i]);
	}
	KRefTrace(bcbarg->cbev);
	KRefTrace(bcbarg->arg);
}

//## buffereventCBArg buffereventCBArg.new(Func[void, int, Object arg] cb, Object cbArg);
static KMETHOD buffereventCBArg_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct buffereventCBArg *bcbarg = (struct buffereventCBArg *) sfp[0].asObject;
	kObjectVar *cbArg = sfp[4].asObjectVar;	//deliver callback method

	bcbarg->kctx = kctx;
	enum e_buffereventCB i;
	for (i = BEV_ReadCB; i < NUM_BuffereventCB; i++){
		kFunc *cb = sfp[i + 1].asFunc;
		KFieldSet(bcbarg, bcbarg->kcb[i], cb);
	}
	/*
	!!ATTENTION!!
	'bcbarg->cbev' will be set in cbufferevent_setcb() function
	*/
	KFieldSet(bcbarg, bcbarg->arg, cbArg);
	KReturn(bcbarg);
}


/* ======================================================================== */
// ctimeval class

static void ctimeval_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct ctimeval *tv = (struct ctimeval *) o;
	tv->timeval.tv_sec = 0;
	tv->timeval.tv_usec = 0;
}

//## ctimeval ctimeval.new(int tv_sec, int tv_usec);
static KMETHOD ctimeval_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct ctimeval *tv = (struct ctimeval *) sfp[0].asObject;
	time_t sec = (time_t)sfp[1].intValue;
	suseconds_t usec = (suseconds_t)sfp[2].intValue;
	tv->timeval.tv_sec = sec;
	tv->timeval.tv_usec = usec;
	KReturn(tv);
}


// TODO should be implement in posix.socket package -----
/* ======================================================================== */
// Sockaddr_in class

static void Sockaddr_in_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct Sockaddr_in *sa = (struct Sockaddr_in *) o;
	memset(&sa->sockaddr, 0, sizeof (struct sockaddr));
}

//## Sockaddr_in Sockaddr_in.new(int family, int addr, int port);
static KMETHOD Sockaddr_in_new(KonohaContext *kctx, KonohaStack *sfp)
{
	struct Sockaddr_in *sa = (struct Sockaddr_in *) sfp[0].asObject;
	sa_family_t family	= (sa_family_t)sfp[1].intValue;
	in_addr_t addr		= (in_addr_t)sfp[2].intValue;
	in_port_t port		= (in_port_t)sfp[3].intValue;

	sa->sockaddr.sin_family = family;
	sa->sockaddr.sin_addr.s_addr = htonl(addr);
	sa->sockaddr.sin_port = htons(port);
	KReturn(sa);
}
// TODO should be implement in posix.socket package -----


/* ======================================================================== */

static kbool_t Libevent_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	/* Class Definition */
	/* If you want to create Generic class like Array<T>, see konoha.map package */
	// cevent_base
	KDEFINE_CLASS defcevent_base = {0};
	SETSTRUCTNAME(defcevent_base, cevent_base);
	defcevent_base.cflag     = KClassFlag_Final;	//must be final in C
	defcevent_base.init      = cevent_base_Init;
	defcevent_base.free      = cevent_base_Free;
	KClass *cevent_baseClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defcevent_base, trace);

	// cevent
	KDEFINE_CLASS defcevent = {0};
	SETSTRUCTNAME(defcevent, cevent);
	defcevent.cflag     = KClassFlag_Final;
	defcevent.init      = cevent_Init;
//	defcevent.reftrace  = cevent_Reftrace;
	defcevent.free      = cevent_Free;
	KClass *ceventClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defcevent, trace);

	// cbufferevent
	KDEFINE_CLASS defcbufferevent = {0};
	SETSTRUCTNAME(defcbufferevent, cbufferevent);
	defcbufferevent.cflag     = KClassFlag_Final;
	defcbufferevent.init      = cbufferevent_Init;
//	defcbufferevent.reftrace  = cbufferevent_Reftrace;
	defcbufferevent.free      = cbufferevent_Free;
	KClass *cbuffereventClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defcbufferevent, trace);

	// eventCBArg
	KDEFINE_CLASS defeventCBArg = {0};
	SETSTRUCTNAME(defeventCBArg, eventCBArg);
	defeventCBArg.cflag     = KClassFlag_Final;
	defeventCBArg.init      = eventCBArg_Init;
	defeventCBArg.reftrace  = eventCBArg_Reftrace;
	defeventCBArg.free      = eventCBArg_Free;
	KClass *eventCBArgClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defeventCBArg, trace);

	// buffereventCBArg
	KDEFINE_CLASS defbuffereventCBArg = {0};
	SETSTRUCTNAME(defbuffereventCBArg, buffereventCBArg);
	defbuffereventCBArg.cflag     = KClassFlag_Final;
	defbuffereventCBArg.init      = buffereventCBArg_Init;
	defbuffereventCBArg.reftrace  = buffereventCBArg_Reftrace;
	defbuffereventCBArg.free      = buffereventCBArg_Free;
	KClass *buffereventCBArgClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defbuffereventCBArg, trace);

	// ctimeval
	KDEFINE_CLASS defctimeval = {0};
	SETSTRUCTNAME(defctimeval, ctimeval);
	defctimeval.cflag     = KClassFlag_Final;
	defctimeval.init      = ctimeval_Init;
	KClass *ctimevalClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defctimeval, trace);

	// Sockaddr_in
	KDEFINE_CLASS defSockaddr_in = {0};
	SETSTRUCTNAME(defSockaddr_in, Sockaddr_in);
	defSockaddr_in.cflag     = KClassFlag_Final;
	defSockaddr_in.init      = Sockaddr_in_Init;
	KClass *Sockaddr_inClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defSockaddr_in, trace);


	/* You can define methods with the following procedures. */
	int KType_cevent_base = cevent_baseClass->typeId;
	int KType_cevent = ceventClass->typeId;
	int KType_cbufferevent = cbuffereventClass->typeId;
	int KType_eventCBArg = eventCBArgClass->typeId;
	int KType_buffereventCBArg = buffereventCBArgClass->typeId;
	int KType_ctimeval = ctimevalClass->typeId;
	int KType_Sockaddr_in = Sockaddr_inClass->typeId;

	/* define Generics parameter for callback method */
	//eventCB_p
	kparamtype_t eventCB_p[] = {{KType_Int, 0}, {KType_Int, 0}, {KType_Object, 0}};
	KClass *ceventCBfunc = KLIB KClass_Generics(kctx, KClass_Func, KType_void, 3, eventCB_p);
	int KType_ceventCBfunc = ceventCBfunc->typeId;
	//bev_dataCB_p
	kparamtype_t bev_dataCB_p[] = {{KType_cbufferevent, 0}, {KType_Object, 0}};
	KClass *Cbev_dataCBfunc = KLIB KClass_Generics(kctx, KClass_Func, KType_void, 2, bev_dataCB_p);
	int KType_Cbev_dataCBfunc = Cbev_dataCBfunc->typeId;
	//bev_eventCB_p
	kparamtype_t bev_eventCB_p[] = {{KType_cbufferevent, 0}, {KType_Int, 0}, {KType_Object, 0}};
	KClass *Cbev_eventCBfunc = KLIB KClass_Generics(kctx, KClass_Func, KType_void, 3, bev_eventCB_p);
	int KType_Cbev_eventCBfunc = Cbev_eventCBfunc->typeId;

	KDEFINE_METHOD MethodData[] = {

		// cevent_base
		_Public|_Static, _F(cevent_base_evutil_make_socket_nonblocking), KType_Int, KType_cevent_base, KMethodName_("evutil_make_socket_nonblocking"), 1, KType_Int, KFieldName_("fd"),
		_Public, _F(cevent_base_new), KType_cevent_base, KType_cevent_base, KMethodName_("new"), 0,
		_Public, _F(cevent_base_event_dispatch), KType_Int, KType_cevent_base, KMethodName_("event_dispatch"), 0,
		_Public, _F(cevent_base_event_loopbreak), KType_Int, KType_cevent_base, KMethodName_("event_loopbreak"), 0,

		// cevent
		_Public, _F(cevent_new), KType_cevent, KType_cevent, KMethodName_("new"), 4, KType_cevent_base, KFieldName_("cevent_base"), KType_Int, KFieldName_("evd"), KType_Int, KFieldName_("event"), KType_eventCBArg, KFieldName_("CBarg"),
		_Public, _F(cevent_event_assign), KType_Int, KType_cevent, KMethodName_("event_assign"), 4, KType_cevent_base, KFieldName_("cevent_base"), KType_Int, KFieldName_("evd"), KType_Int, KFieldName_("event"), KType_eventCBArg, KFieldName_("CBarg"),
		_Public, _F(cevent_event_add), KType_Int, KType_cevent, KMethodName_("event_add"), 1, KType_ctimeval, KFieldName_("timeval"),
		_Public, _F(cevent_event_del), KType_Int, KType_cevent, KMethodName_("event_del"), 0,
		_Public, _F(cevent_event_pending), KType_Int, KType_cevent, KMethodName_("event_pending"), 2, KType_Int, KFieldName_("events"), KType_Int, KFieldName_("ctimeval"),
		_Public, _F(cevent_event_initialized), KType_Int, KType_cevent, KMethodName_("event_initialized"), 0,
		_Public, _F(cevent_event_free), KType_void, KType_cevent, KMethodName_("event_free"), 0,
		_Public, _F(cevent_event_active), KType_void, KType_cevent, KMethodName_("event_active"), 2, KType_Int, KFieldName_("res"), KType_Int, KFieldName_("ncalls"),
		/*
		USE event.signal_new() in Libevent_kick.k
		_Public, _F(cevent_signal_new), KType_cevent, KType_cevent, KMethodName_("signal_new"), 3, KType_cevent_base, KFieldName_("cevent_base"), KType_Int, KFieldName_("signo"), KType_eventCBArg, KFieldName_("CBarg"),
		*/
		_Public, _F(cevent_getID), KType_Int, KType_cevent, KMethodName_("getID"), 0, 
		_Public, _F(cevent_getEvents), KType_Int, KType_cevent, KMethodName_("getEvents"), 0, 

		// cbufferevent
		_Public, _F(cbufferevent_new), KType_cbufferevent, KType_cbufferevent, KMethodName_("new"), 3, KType_cevent_base, KFieldName_("cevent_base"), KType_Int, KFieldName_("evd"), KType_Int, KFieldName_("options"),
		_Public, _F(cbufferevent_setcb), KType_void, KType_cbufferevent, KMethodName_("bufferevent_setcb"), 1, KType_buffereventCBArg, KFieldName_("buffereventCBArg"),
		_Public, _F(cbufferevent_socket_connect), KType_Int, KType_cbufferevent, KMethodName_("bufferevent_socket_connect"), 1, KType_Sockaddr_in, KFieldName_("sockaddr"),
		_Public, _F(cbufferevent_enable), KType_Int, KType_cbufferevent, KMethodName_("bufferevent_enable"), 1, KType_Int, KFieldName_("event"),
		_Public, _F(cbufferevent_write), KType_Int, KType_cbufferevent, KMethodName_("bufferevent_write"), 1, KType_Bytes, KFieldName_("writebuffer"),
		_Public, _F(cbufferevent_read), KType_Int, KType_cbufferevent, KMethodName_("bufferevent_read"), 1, KType_Bytes, KFieldName_("readbuffer"),

		// eventCBArg
		_Public, _F(eventCBArg_new), KType_eventCBArg, KType_eventCBArg, KMethodName_("new"), 2, KType_ceventCBfunc, KFieldName_("konoha_CB"), KType_Object, KFieldName_("CBarg"),

		// buffereventCBArg
		_Public, _F(buffereventCBArg_new), KType_buffereventCBArg, KType_buffereventCBArg, KMethodName_("new"), 4, KType_Cbev_dataCBfunc, KFieldName_("readCB"), KType_Cbev_dataCBfunc, KFieldName_("writeCB"), KType_Cbev_eventCBfunc, KFieldName_("eventCB"), KType_Object, KFieldName_("CBarg"),

		// ctimeval
		_Public, _F(ctimeval_new), KType_ctimeval, KType_ctimeval, KMethodName_("new"), 2, KType_Int, KFieldName_("tv_sec"), KType_Int, KFieldName_("tv_usec"),

		// Sockaddr_in
		_Public, _F(Sockaddr_in_new), KType_Sockaddr_in, KType_Sockaddr_in, KMethodName_("new"), 3, KType_Int, KFieldName_("family"), KType_Int, KFieldName_("addr"), KType_Int, KFieldName_("port"),

		DEND, /* <= sentinel */
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);


	KDEFINE_INT_CONST IntData[] = {
		// === for event_new() ===
		{KDefineConstInt(EV_TIMEOUT)},
		{KDefineConstInt(EV_READ)},
		{KDefineConstInt(EV_WRITE)},
		{KDefineConstInt(EV_SIGNAL)},
		{KDefineConstInt(EV_PERSIST)},
		{KDefineConstInt(EV_ET)},

		// === for bufferevent ===
		// bufferevent.h
		{KDefineConstInt(BEV_EVENT_READING)},
		{KDefineConstInt(BEV_EVENT_WRITING)},
		{KDefineConstInt(BEV_EVENT_EOF)},
		{KDefineConstInt(BEV_EVENT_ERROR)},
		{KDefineConstInt(BEV_EVENT_TIMEOUT)},
		{KDefineConstInt(BEV_EVENT_CONNECTED)},

		// bufferevent.h: enum bufferevent_options
		{KDefineConstInt(BEV_OPT_CLOSE_ON_FREE)},
		{KDefineConstInt(BEV_OPT_THREADSAFE)},
		{KDefineConstInt(BEV_OPT_DEFER_CALLBACKS)},
		{KDefineConstInt(BEV_OPT_UNLOCK_CALLBACKS)},

		{KDefineConstInt(AF_INET)},// TODO should be implement in posix.socket package

		{} /* <= sentinel */
	};

	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(IntData), trace);

	return true;
}

static kbool_t Libevent_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *Libevent_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "libevent2.0.19", "0.1"); //TODO use event_get_version();
	d.PackupNameSpace	= Libevent_PackupNameSpace;
	d.ExportNameSpace	= Libevent_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
