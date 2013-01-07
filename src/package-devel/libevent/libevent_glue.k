import("konoha.new");
import("konoha.class");
import("konoha.object");
import("konoha.map");
import("cstyle");
import("js4.date");

class Libevent {
	Map event_Map;

	Libevent() {
		event_Map =  = new Map();
	}

	int event_add(String key, Date tv) {
		if (key.isNull()) {
			this.event_Map.set(key, new event());
		}
		Libevent_event ev = event_Map.get(key);
		return System.event_add(ev, tv);
	}
}
