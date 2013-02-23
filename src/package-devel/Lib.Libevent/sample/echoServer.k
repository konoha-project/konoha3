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


void do_read(int evd, int evflag, Object arg) {
	event ev = arg as event;
	System.p("do_read() executed!!");
	System.p("evd = " + evd + ", evflag = " + evflag);
	System.p("ev = " + ev);
	Bytes buf = new Bytes(32);
	if (System.recv(evd, buf, 0) <= 0) {
		ev.event_del();
		System.shutdown(evd, SHUT_RDWR);
	} else {
		System.p("buf = \n" + buf);
	}
	if (buf[0] == 0x03 || buf[0] == 0x04) {
		System.send(evd, "recv close request\n".toBytes(), 0);
		ev.event_del();
		System.shutdown(evd, SHUT_RDWR);
	} else {
		System.send(evd, buf, 0);
	}
}

void do_accept(int evd, int evflag, Object arg) {
	event_base evBase = arg as event_base;
	System.p("do_accept() executed!!");
	System.p("evBase = " + evBase);
	System.p("evd = " + evd + ", evflag = " + evflag);

	SockAddr dstInfo = new SockAddr();
	int fd = System.accept(evd, dstInfo);
	if (fd < 0) {	//XXXX eagain??
		System.p("accept() error");	//TODO: LogText?
	/*
	} else if (fd > FD_SETSIZE) {
		close(fd);	//XXX replace all closes with EVUTIL_CLOSESOCKET
	*/
	} else {
		cevent_base.evutil_make_socket_nonblocking(fd);
		//{
			//alloc_fd_state
			event ev = new event(evBase, fd, EV_READ | EV_PERSIST, do_read);
			ev.event_add(NULL/* NULL: no timeout(dispatching forever), timeout sets like "new TimeVal(sec, usec)")*/);
			System.send(fd, "hello\n".toBytes(), 0);
		//}
	}
}

void signal_handler(int sig, int evflag, Object arg) {
	event_base evBase = arg as event_base;
	System.p("signal_handler() executed!!");
	System.p("evBase = " + evBase);
	System.p("signal = " + sig + ", evflag = " + evflag);
	evBase.cevbase.event_loopbreak();
}

void main() {
	event_base evBase = new event_base();

	int listener = System.socket(AF_INET, SOCK_STREAM, 0);
	cevent_base.evutil_make_socket_nonblocking(listener);

	if (System.bind(listener, "", 10000, AF_INET) != 0) {
		System.p("bind() error");	//TODO: LogText?
		return;
	}

	if (System.listen(listener, 16) < 0) {
		System.p("listen() erroro");	//TODO: LogText?
		return;
	}

	//IO event
	event ev = new event(evBase, listener, EV_READ | EV_PERSIST, do_accept, evBase);
	ev.event_add(NULL);

	//signal event
	event sig = event.signal_new(evBase, 1/*HUP*/, signal_handler, evBase);
	sig.event_add(NULL);

	evBase.cevbase.event_dispatch();
}

main();
