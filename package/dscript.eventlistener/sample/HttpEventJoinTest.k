import("konoha.new");
import("konoha.class");
import("konoha.array");
import("konoha.map");
import("konoha.global");
import("konoha.const");
import("cstyle", "while");
import("posix.process");
import("posix.thread");
import("dscript.eventlistener");

class EventQueue {
	Array[Event] data;
	//Array[KonohaContext] ctxlist;
	EventQueue(int size) {
		data = new Event[0];
		ctxlist = new KonohaContext[0];
	}
	void enq(Event v) {
		data.add(v);
		setSafepoint();
	}
	Event deq() {
		Event v = data[0];
		data.removeAt(0);
		return v;
	}
	boolean isEmpty() {
		return data.size == 0;
	}
	//void addEventHandler(KonohaContext ctx) {
	//	//ctxlist.add(ctx);
	//}
	void setSafepoint() {
		//foreach(KonohaContext ctx in ctxlist) {
		//int i=0;
		//while(i < ctxlist.getSize()) {
		//	KonohaContext ctx = ctxlist[i];
		//	//ctx.setSafepoint();
		//	System.setSafepoint();
		//	i += 1;
		//}
		System.setSafepoint();
	}
}

const EventMap = Map[Func[void, Event]];

class Dispatcher {
	Map[Func[void, Event]] handlers;
	EventQueue queue;
	Mutex lock;

	Dispatcher() {
		handlers = new EventMap();
		queue = new EventQueue();
		lock = new Mutex();
	}

	void addHandlerFunc(String type, Func[void, Event] func) {
		handlers.set(type, func);
	}

	void invoke() {
		lock.lock();
		if(queue.isEmpty()) {
			lock.unlock();
			return;
		}
		Event ev = queue.deq();
		String key = ev.getProperty("event");
		Func[void, Event] func = handlers[key];
		func(ev);
		/* finally */
		lock.unlock();
	}
}

Dispatcher d = new Dispatcher();

/* callback methods */
void invokeEvent() {
	System.p("invokeEvent");
	global.d.invoke();
}
void enqEvent(Event e) {
	System.p("hello");
	global.d.queue.enq(e);
	System.setSafepoint();
}

void test_handler(Event e) {
	System.p("handler property=" + e.getProperty("event"));
}

void test() {
	System.setEnqFunc(enqEvent);
	System.setEventInvokeFunc(invokeEvent);
	global.d.addHandlerFunc("hoge", test_handler);
	//global.d.queue.addEventHandler(ctx);//TODO

	HttpEventListener.start("0.0.0.0", 8080);

	while(true) {
		//System.sleep(1);
	}
}

test();
