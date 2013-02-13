/*
 * bufferevent sample
 * > telnet localhost 10000
 */

import("Syntax.JavaStyleClass");
import("JavaStyle.Object");
import("Lib.Libevent");

void readCB(Bufferevent bev, Object arg) {
	System.p("called readCB()");
	bev.bufferevent_get_input();



}

void writeCB(Bufferevent bev, Object arg) {
	System.p("called writeCB()");



}

void eventCB(Bufferevent bev, int events, Object arg) {
	System.p("called eventCB()");



}

void main() {
	Event_base evBase = new Event_base();
	Bufferevent bev = new Bufferevent(new evBase, -1, BEV_OPT_CLOSE_ON_FREE);	// == bufferevent_socket_new()
	bev.bufferevent_setcb(readCB, writeCB, eventCB, cbarg);




	TODO



	if (bufferevent_socket_connect() < 0) {
		bev = NULL;
	}
	evBase.event_dispatch();
}

main();
