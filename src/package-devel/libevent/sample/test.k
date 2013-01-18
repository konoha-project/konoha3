import("konoha.new");
import("konoha.class");
import("konoha.object");
import("posix.socket");
import("libevent");


void callback_method(Object arg) {
	System.p("Callback executed!!");
}

void main() {
	int fd = System.socket(AF_INET, SOCK_STREAM, 0);
	int ret = System.bind(fd, "", 10000, AF_INET);
	int evd = System.listen(fd, 1);


	Event_base e = new Event_base();
	System.p("--- 1 ----");
	e.event_add("listener", evd, 2/*EV_READ*/, callback_method, new TimeVal(15, 300));
	System.p("--- 2 ----");
	e.event_dispatch();
	System.p("--- 3 ----");
}

main();
