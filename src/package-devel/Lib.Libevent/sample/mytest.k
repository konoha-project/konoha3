import("konoha.map");
import("konoha.new");
import("Syntax.JavaStyleClass");
import("konoha.object");
//import("Syntax.GlobalVariable");

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
