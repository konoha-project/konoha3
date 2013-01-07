import("konoha.new");
import("konoha.class");
import("konoha.object");
import("konoha.map");
import("cstyle");
import("js4.date");

class Libevent {
	Map event_Map;

	Libevent(void) {
		event_Map =  = new Map();
	}

	int event_add(String key, Date tv) {
		if (key.isNull()) {
			event_Map.set(key, new event());
		}
		Libevent_event ev = event_Map.get(key);
		return this.event_add(ev, tv);
	}
}
