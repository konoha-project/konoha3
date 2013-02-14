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

class Event;
class Event_base {
	Cevent_base cevbase;
	@Private Map/*TODO Map[Event]*/ evMap;

	Event_base() {
		cevbase = new Cevent_base();
		evMap = new Map();
	}

	@Public int event_dispatch() {
		return cevbase.event_dispatch();
	}

	@Public int event_add(Event ev, TimeVal tv) {
		evMap.set(ev.getKey(), ev);
		int ret =  System.event_add(ev.cev, tv);
		return ret;
	}

	@Public int event_del(Event ev) {
		Event mapev = evMap.get(ev.getKey()) as Event;
		System.p("mapev = " + mapev);
		evMap.remove(ev.getKey());
		return System.event_del(ev.cev);
	}
}

class Event {
	Cevent cev;
	Event_base evBase;

	Event() {}

	Event(Event_base evbase, int evd, int evCat, Func[void, int, int, Object] cb, Object cbArg) {
		cev = new Cevent(evbase.cevbase, evd, evCat, new EventCBArg(cb, cbArg));
		this.evBase = evbase;
	}

	// use this instance as Callback argument
	Event(Event_base evbase, int evd, int evCat, Func[void, int, int, Object] cb) {
		//TODO ideal situation is "this(evbase.evbase, evd, evCat, cb, this);" ---
		cev = new Cevent(evbase.cevbase, evd, evCat, new EventCBArg(cb, this));
		this.evBase = evbase;
		// --- TODO
	}

	String getKey() {
		String ret;
		@Const int evfd =  cev.getEvfd();
		@Const int events = cev.getEvents();

		if ((events & EV_SIGNAL) != 0) {
			ret = "SIGNAL" + evfd.toString();
		} else {
			if (evfd == -1) {
				ret = "TIMER";
			} else {
				ret = "IO" + evfd.toString();
			}
		}
		return ret;
	}
}

class Bufferevent extends Event {
	Cbufferevent cbev;

	Bufferevent() {}

	Bufferevent(Event_base evbase, int fd, int options) {
		cbev = new Cbufferevent(evbase.cevbase, fd, options);
		this.evBase = evbase;
	}

	@Public void setcb(
		Func[void, Object/*TODO Bufferevent*/, Object]	readcb,
		Func[void, Object/*TODO Bufferevent*/, Object]	writecb,
		Func[void, Object/*TODO Bufferevent*/, int, Object]	eventcb,
		Object cbArg) {
		BuffereventCBArg cbarg = new BuffereventCBArg(readcb, writecb, eventcb, cbArg);
		System.bufferevent_setcb(cbev, cbarg);
	}

	@Public int socket_connect(Sockaddr_in sa) {
		int s = System.bufferevent_socket_connect(cbev, sa);
		return s;
	}

	int enable(int event) {
		return System.bufferevent_enable(cbev, event);
	}

	int evbuffer_add_printf(String msg) {
		return System.evbuffer_add_printf(cbev, msg);
	}
}
