import("konoha.new");
import("konoha.class");
import("konoha.object");
import("cstyle");
import("posix.socket");
import("libevent");


void do_accept(CEvent e) {	//change callback parameters as libevent cb
	System.p("Callback executed!!");
	System.p("CEvent = " + e);
	//close()
}

void main() {
	int evd = System.socket(AF_INET, SOCK_STREAM, 0);
	if (System.bind(evd, "", 10001, AF_INET) != 0) {
		System.p("bind() error");
		return;
	}
	System.listen(evd, 1);

	Event_base e = new Event_base();
	System.p(e);
	e.event_add("do_accept", evd, EV_READ | EV_PERSIST, do_accept, new TimeVal(15, 300));
	e.event_dispatch();
}

main();
