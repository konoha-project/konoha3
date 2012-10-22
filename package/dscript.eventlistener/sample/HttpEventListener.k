import("cstyle", "while");
import("posix.process");
import("dscript.eventlistener");

void enqEvent(Event e) {
	System.p(e.getProperty("event"));
}

void test() {
	System.setEnqFunc(enqEvent);
	HttpEventListener.start("0.0.0.0", 8080);
	while(true) {
		System.sleep(1);
	}
}

test();
