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
	System.p("do_read() executed!!");
	System.p("evd = " + evd + ", event = " + event);
	System.p("arg = " + arg);
	Bytes buf = new Bytes(32);
	int ret = System.recv(evd, buf, 0);
	System.send(evd, buf, 0);
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
			evBase.event_add("do_read", fd, EV_READ | EV_PERSIST, do_read, NULL, new TimeVal(15, 300));
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

	evBase.event_add("do_accept", listener, EV_READ | EV_PERSIST, do_accept, evBase, new TimeVal(15, 300));
	evBase.event_dispatch();
}

main();
