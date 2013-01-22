import("Syntax.CStyleWhile");
import("posix.process");
import("MiniKonoha.EventListener");

void callback(Event e) {
	System.p("handler property=" + e.getProperty("event"));
}

void test() {
	EventHandler.setCallBack("SIGHUP", callback);
	SignalEventListener.start();

	while(true) {
		System.usleep(1000);
	}
}

test();
