/*
 * bufferevent sample
 * connect to 127.0.0.1:10000
 *
 * Most simple way is to use echoServer.k
 */

import("Syntax.JavaStyleClass");
import("JavaStyle.Object");
import("Syntax.CStyleBitwiseOperator");
import("Type.Bytes");
import("Lib.Libevent");

void readCB(Cbufferevent bev, Object arg) {
	System.p("called readCB()");

	Bytes buf = new Bytes(1024);
	System.bufferevent_read(bev, buf);
	System.p(buf);

	if (buf.toString() == "hello") {
		System.bufferevent_write(bev, "Hi!".toBytes())
	}
}

void writeCB(Cbufferevent bev, Object arg) {
	System.p("called writeCB()");



}

void eventCB(Cbufferevent bev, int events, Object arg) {
	System.p("called eventCB()");
	System.p("bev = " + bev + ", events = " + events + ", arg = " + arg);

	if ((events & BEV_EVENT_CONNECTED) != 0) {
		System.p("Connect okay");
	} else if ((events & (BEV_EVENT_ERROR | BEV_EVENT_EOF)) != 0) {
		System.p("Closing");
		//TODO close
	}



}

int main() {
	Event_base evBase = new Event_base();
	Bufferevent bev = new Bufferevent(evBase, -1, BEV_OPT_CLOSE_ON_FREE);	// == bufferevent_socket_new()
	Object cbarg = bev as Object;
	bev.setcb(readCB, writeCB, eventCB, cbarg);

	int s = bev.socket_connect(new Sockaddr_in(AF_INET, 0x7f000001, 10000));
	if (s > 0) {
		bev = NULL;
		evBase = NULL;
		return -1;
	}

	bev.enable(EV_READ /*| EV_WRITE*/);
	bev.write("from client".toBytes());
	System.p("event_dispatch()");
	evBase.event_dispatch();
	return 0;
}

main();
