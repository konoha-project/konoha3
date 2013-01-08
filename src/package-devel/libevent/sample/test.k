import("konoha.new");
import("konoha.class");
import("konoha.object");
import("libevent");
import("js4.date");

void main() {
    MyLibevent e = new MyLibevent();
    e.event_add("listener", new Date());
}

main();
