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

class Event_base {
	CEvent_base evbase;
	@Private Map eventMap;	//no need?

	Event_base() {
		evbase = new CEvent_base();
		eventMap = new Map();
	}

	@Public int event_dispatch() {
		return evbase.event_dispatch();
	}

	@Public int event_add(String key, int evd, int event, Func[void, int, int, Object] cb, Object cbArg, TimeVal tv) {
		if (key.isNotNull()) {
			CEvent ev = new CEvent(evbase, key, evd, event, new CallBackArg(cb, cbArg));
			System.p(eventMap);
			eventMap.set(key, ev);	//keep referance of 'ev', and deliver it to callback method
			int ret =  System.event_add(ev, tv);
			return ret;
		} 
		return -1;	// event_add() error status of libevent
	}

	//TODO
	//@Public int event_add(String key, int evd, int event
	// 				, int timout_set, int timout_usec)
	//{}

	@Public int event_del(String key) {
		CEvent ev = event_Map.get(key) as CEvent;
		eventMap.remove(key);
		return System.event_del(ev);
	}
}
