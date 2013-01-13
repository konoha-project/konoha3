import("konoha.new");
import("konoha.class");
import("konoha.object");
import("libevent");
import("js4.date");

void main() {
    Event_base e = new Event_base();
    e.event_add("listener", new Date());
}

main();
