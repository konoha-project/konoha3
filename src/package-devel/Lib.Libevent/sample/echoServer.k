/*
 * simple echo server
 * connect from telnet client, and echo back it.
 * > telnet localhost 10000
 */
import("Syntax.JavaStyleClass");
import("JavaStyle.Object");
import("cstyle");
import("posix.socket");
import("Type.Bytes");
import("Lib.Libevent");


void do_read(int evd, int event, Object arg) {
	Event ev = arg as Event;
	System.p("do_read() executed!!");
	System.p("evd = " + evd + ", event = " + event);
	System.p("ev = " + ev);
	Bytes buf = new Bytes(32);
	int ret = System.recv(evd, buf, 0);
	if (buf[0] == 0x03 || buf[0] == 0x04) {
		System.send(evd, "socket close\n".toBytes(), 0);
		ev.evBase.event_del(ev);
		System.shutdown(evd, SHUT_RDWR);
	} else {
		System.send(evd, buf, 0);
	}
}

void do_accept(int evd, int event, Object arg) {
	Event_base evBase = arg as Event_base;
	System.p("do_accept() executed!!");
	System.p("evBase = " + evBase);
	System.p("evd = " + evd + ", event = " + event);

	SockAddr dstInfo = new SockAddr();
	int fd = System.accept(evd, dstInfo);
	if (fd < 0) {	//XXXX eagain??
		System.p("accept() error");	//TODO: LogText?
	/*
	} else if (fd > FD_SETSIZE) {
		close(fd);	//XXX replace all closes with EVUTIL_CLOSESOCKET
	*/
	} else {
		System.evutil_make_socket_nonblocking(fd);
		//{
			//alloc_fd_state
			Event ev = new Event(evBase, fd, EV_READ | EV_PERSIST, do_read);
			evBase.event_add(ev, NULL/* NULL: no timeout(dispatching forever), timeout sets like "new TimeVal(sec, usec)")*/);
		//}
	}
}

void main() {
	Event_base evBase = new Event_base();

	int listener = System.socket(AF_INET, SOCK_STREAM, 0);
	System.evutil_make_socket_nonblocking(listener);

	if (System.bind(listener, "", 10000, AF_INET) != 0) {
		System.p("bind() error");	//TODO: LogText?
		return;
	}

	if (System.listen(listener, 16) < 0) {
		System.p("listen() erroro");	//TODO: LogText?
		return;
	}

	Event ev = new Event(evBase, listener, EV_READ | EV_PERSIST, do_accept, evBase);
	evBase.event_add(ev, NULL);
	evBase.event_dispatch();
}

main();
