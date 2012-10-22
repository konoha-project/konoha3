import("cstyle", "while");
import("posix.process");
import("dscript.eventlistener");

void enqEvent(Event e) {
	System.p(e.getProperty("event"));
}

void test() {
	System.setEnqFunc(enqEvent);
	SignalEventListener.start();
	while(true) {
		System.sleep(1);
	}
}

test();
