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

import("Syntax.JavaStyleClass");
import("JavaStyle.Object");
import("MiniKonoha.Map");
import("MiniKonoha.NameSpace");
import("Syntax.Null");

class event;
class event_base {
	cevent_base cevbase;
	@Private Map/*TODO Map[event]*/ evMap;

	event_base() {
		cevbase = new cevent_base();
		evMap = new Map();
	}

	void event_add(event ev) {
		evMap.set(ev.getKey(), ev);	//TODO
	}

	void event_del(event ev) {
		evMap.remove(ev.getKey());	//TODO
	}
}

class event {
	@Private cevent cev;
	event_base evBase;

	event() {}

	event(event_base evbase, int evd, int evCat, Func[void, int, int, Object] cb, Object cbArg) {
		cev = new cevent(evbase.cevbase, evd, evCat, new eventCBArg(cb, cbArg));
		this.evBase = evbase;
	}

	// use this instance as Callback argument
	event(event_base evbase, int evd, int evCat, Func[void, int, int, Object] cb) {
		//TODO ideal situation is "this(evbase.evbase, evd, evCat, cb, this);" ---
		cev = new cevent(evbase.cevbase, evd, evCat, new eventCBArg(cb, this));
		this.evBase = evbase;
		// --- TODO
	}

	@Public int event_add(ctimeval tv) {
		this.evBase.event_add(this);
		return cev.event_add(tv);
	}

	@Public int event_del() {
		this.evBase.event_del(this);
		return cev.event_del();
	}

	@Public void event_pending(int events, ctimeval tv) {
		cev.event_pending(events, tv);
	}

	@Public void event_free() {
		cev.event_free();
	}

	@Public void event_active(int res, int ncalls) {
		cev.event_active(res, ncalls);
	}

	@Public int event_initialized() {
		return cev.event_initialized();
	}

	@Public int event_assign(event_base evbase, int evd, int evCat, Func[void, int, int, Object] cb, Object cbArg) {
		return cev.event_assign(evbase.cevbase, evd, evCat, new eventCBArg(cb, cbArg));
	}

	/* --- evtimer ---
	same as C Macro in libevent event2/event.h:

		#define evtimer_assign(ev, b, cb, arg) \
			event_assign((ev), (b), -1, 0, (cb), (arg))
		#define evtimer_new(b, cb, arg)	       event_new((b), -1, 0, (cb), (arg))
		#define evtimer_add(ev, tv)		event_add((ev), (tv))
		#define evtimer_del(ev)			event_del(ev)
		#define evtimer_pending(ev, tv)		event_pending((ev), EV_TIMEOUT, (tv))
		#define evtimer_initialized(ev)		event_initialized(ev)
	*/
	@Static event timer_new(event_base evbase, Func[void, int, int, Object] cb, Object cbArg) {
		event ev = new event(evbase, -1, 0, cb, cbArg);
		return ev;
	}

	@Public int timer_add(ctimeval tv) {
		return event_add(tv);
	}

	@Public int timer_del() {
		return event_del();
	}

	@Public int timer_assign(event_base evbase, Func[void, int, int, Object] cb, Object cbArg) {
		return event_assign(evbase, -1, 0, cb, cbArg);
	}

	@Public int timer_pending(ctimeval tv) {
		return event_pending(EV_TIMEOUT, tv);
	}

	@Public int timer_initialized() {
		return event_initialized();
	}

	/* --- evsignal ---
	same as C Macro in libevent event2/event.h:

		#define evsignal_add(ev, tv)		event_add((ev), (tv))
		#define evsignal_assign(ev, b, x, cb, arg)			\
			event_assign((ev), (b), (x), EV_SIGNAL|EV_PERSIST, cb, (arg))
		#define evsignal_new(b, x, cb, arg)				\
			event_new((b), (x), EV_SIGNAL|EV_PERSIST, (cb), (arg))
		#define evsignal_del(ev)		event_del(ev)
		#define evsignal_pending(ev, tv)	event_pending((ev), EV_SIGNAL, (tv))
		#define evsignal_initialized(ev)	event_initialized(ev)
	*/
	@Static event signal_new(event_base evbase, int signo, Func[void, int, int, Object] cb, Object cbArg) {
		event ev = new event(evbase, signo, EV_SIGNAL | EV_PERSIST, cb, cbArg);
		return ev;
	}

	@Public int signal_add(ctimeval tv) {
		return event_add(tv);
	}

	@Public int signal_del() {
		return event_del();
	}

	@Public int signal_assign(event_base evbase, int evd, Func[void, int, int, Object] cb, Object cbArg) {
		return event_assign(evbase, evd, EV_SIGNAL | EV_PERSIST, cb, cbArg);
	}

	@Public int signal_pending(ctimeval tv) {
		return event_pending(EV_SIGNAL, tv);
	}

	@Public int signal_initialized() {
		return event_initialized();
	}


	@Private String getKey() {
		String ret;
		@Const int id = cev.getID();
		@Const int events = cev.getEvents();

		if ((events & EV_SIGNAL) != 0) {
			ret = "SIGNAL" + id.toString();
		} else {
			if (id == -1) {
				ret = "TIMER" + id.toString();
			} else {
				ret = "IO" + id.toString();
			}
		}
		return ret;
	}
}

class bufferevent extends event {
	cbufferevent cbev;

	bufferevent() {}

	bufferevent(event_base evbase, int fd, int options) {
		cbev = new cbufferevent(evbase.cevbase, fd, options);
		this.evBase = evbase;
	}

	@Public void setcb(
		Func[void, cbufferevent, Object]	readcb,
		Func[void, cbufferevent, Object]	writecb,
		Func[void, cbufferevent, int, Object]	eventcb,
		Object cbArg) {
		buffereventCBArg cbarg = new buffereventCBArg(readcb, writecb, eventcb, cbArg);
		cbev.bufferevent_setcb(cbarg);
	}

	@Public int socket_connect(Sockaddr_in sa) {
		int s = cbev.bufferevent_socket_connect(sa);
		return s;
	}

	int enable(int evflag) {
		return cbev.bufferevent_enable(evflag);
	}

	int write(Bytes msg) {
		return cbev.bufferevent_write(msg);
	}

	int read(Bytes buf) {
		return cbev.bufferevent_read(buf);
	}
}
