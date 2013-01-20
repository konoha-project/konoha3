import("konoha.new");
import("konoha.class");
import("konoha.object");
import("cstyle");
import("posix.socket");
import("libevent");


void callback_method() {
	System.p("Callback executed!!");
}

void main() {
	int evd = System.socket(AF_INET, SOCK_STREAM, 0);
	if (System.bind(evd, "", 10001, AF_INET) != 0) {
		System.p("bind() error");
		return;
	}
	System.listen(evd, 1);

	Event_base e = new Event_base();
	System.p("--- 1 ----");
	e.event_add("listener", evd, EV_READ | EV_PERSIST, callback_method, new TimeVal(15, 300));
	System.p("--- 2 ----");
	e.event_dispatch();		//SEGV
	System.p("--- 3 ----");
}

main();
