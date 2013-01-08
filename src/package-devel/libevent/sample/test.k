import("konoha.new");
import("konoha.class");
import("konoha.object");
import("libevent");

void main() {
	// Libevent_k e = new Libevent_k();
	MyLibevent e = new MyLibevent();

	e.event_add("listener", NULL);



}

main();
