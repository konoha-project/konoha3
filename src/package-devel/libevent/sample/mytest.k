import("konoha.map");
import("konoha.new");
import("konoha.class");
import("konoha.object");
//import("konoha.global");

class event {
	Map a;
	
	event(void) {
		a = new Map();
	}

	void main(void) {
		a.set("test", new Object());
		System.p("a.get(\"test\") = " + a.get("test"));
	}
}

new event().main();
