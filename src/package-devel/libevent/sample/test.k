import("konoha.new");
import("konoha.class");
import("konoha.object");
import("libevent");
import("js4.date");


void callback_method(Object arg) {
	System.p("Callback executed!!");
}

void main() {
    Event_base e = new Event_base();
    e.event_add("listener", callback_method, new TimeVal(15, 300));
	e.event_dispatch();
}

main();
