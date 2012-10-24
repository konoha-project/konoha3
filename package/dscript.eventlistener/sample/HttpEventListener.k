import("cstyle", "while");
import("posix.process");
import("dscript.eventlistener");

void callback(Event e) {
	System.p("handler property=" + e.getProperty("event"));
}

void test() {
	EventHandler.setCallBack("hoge", callback);
	HttpEventListener.start("0.0.0.0", 8080);

	while(true) {
		System.usleep(1000);
	}
}

test();
