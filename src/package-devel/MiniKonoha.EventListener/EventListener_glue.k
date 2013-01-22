import("Syntax.JavaStyleClass");
import("JavaScript.Array");
import("MiniKonoha.Map");
import("MiniKonoha.NameSpace");  // konoha.const is obsolete
import("Syntax.GlobalVariable");
import("JavaStyle.Object");
import("posix.thread");
import("Type.Json"); // temporally

class EventQueue {
	Array[Event] data;
	//Array[KonohaContext] ctxlist;

	EventQueue(void) {
		this.data = [];
		//this.ctxlist = new KonohaContext[0];
		return this;
	}

	void enqueue(Event event) {
		this.data.add(event);
		this.setSafepoint();
	}

	Event dequeue() {
		Event event = this.data[0];
		this.data.removeAt(0);
		return event;
	}

	boolean isEmpty() {
		return this.data.size == 0;
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

//const EventMap = Map[Func[void, Event]]; TODO

class EventMap { // temporally
	Json keyToIndexMap;
	Array[Func[void, Event]] values;

	EventMap() {
		this.keyToIndexMap = new Json();
		this.values = [];
		return this; // TODO remove it
	}

	@Public void set(String key, Func[void, Event] func) {
		int index = this.values.getSize();
		this.values.add(func);
		this.keyToIndexMap.setInt(key, index);
	}

	@Public Func[void, Event] get(String key) {
		int index = this.keyToIndexMap.getInt(key);
		return this.values[index];
	}
}

class Dispatcher {
	EventMap handlers;
	EventQueue queue;
	Mutex lock;

	Dispatcher() {
		this.handlers = new EventMap();
		this.queue = new EventQueue();
		this.lock = new Mutex();
		return this; // TODO remove it
	}

	@Public void setCallBack(String eventType, Func[void, Event] func) {
		this.handlers.set(eventType, func);
	}

	void invoke() {
		this.lock.lock();
		if(this.queue.isEmpty()) {
			this.lock.unlock();
			return;
		}
		Event event = this.queue.dequeue();
		String key = event.getProperty("event");
//		Func[void, Event] func = this.handlers[key]; // TODO
		Func[void, Event] func = this.handlers.get(key);
		func(event);
		/* finally */
		this.lock.unlock();
	}
}

Dispatcher dispatcher = new Dispatcher();

/* callback methods */
void invokeEvent() {
	global.dispatcher.invoke();
}

void enqueueEvent(Event event) {
	global.dispatcher.queue.enqueue(event);
	System.setSafepoint();
}

System.setEnqFunc(enqueueEvent);
System.setEventInvokeFunc(invokeEvent);

class EventHandler {
	@Public @Static void setCallBack(String eventType, Func[void, Event] callback) {
		global.dispatcher.setCallBack(eventType, callback);
	}
	@Public @Static void setCallBack(int eventType, Func[void, Event] callback) {
		global.dispatcher.setCallBack(eventType.toString(), callback);
	}
}
