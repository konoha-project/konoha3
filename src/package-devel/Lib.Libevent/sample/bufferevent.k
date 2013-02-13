/*
 * bufferevent sample
 * > telnet localhost 10000
 */

import("Syntax.JavaStyleClass");
import("JavaStyle.Object");
import("Lib.Libevent");

void readCB(Cbufferevent bev, Object arg) {
	System.p("called readCB()");
	bev.bufferevent_get_input();



}

void writeCB(Cbufferevent bev, Object arg) {
	System.p("called writeCB()");



}

void eventCB(Cbufferevent bev, int events, Object arg) {
	System.p("called eventCB()");



}

int main() {
	Event_base evBase = new Event_base();
	Bufferevent bev = new Bufferevent(evBase, -1, BEV_OPT_CLOSE_ON_FREE);	// == bufferevent_socket_new()
	Object cbarg = bev as Object;
	bev.setcb(readCB, writeCB, eventCB, cbarg);

	if (bev.socket_connect(new Sockaddr_in(AF_INET, 0x7f000001, 8080)) < 0) {
		bev = NULL;
		evBase = NULL;
		return -1;
	}

	evBase.event_dispatch();
	return 0;
}

main();
