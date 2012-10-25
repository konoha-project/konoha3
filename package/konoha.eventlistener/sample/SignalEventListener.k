import("cstyle", "while");
import("posix.process");
import("konoha.eventlistener");

void callback(Event e) {
	System.p("handler property=" + e.getProperty("event"));
}

void test() {
	EventHandler.setCallBack("hoge", callback);
	SignalEventListener.start();

	while(true) {
		System.usleep(1000);
	}
}

test();
