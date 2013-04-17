import("Type.Bytes");
import("JavaScript.Array");
import("Lib.Libevent");

void main() {
	Array methods = event_base.get_supported_methods();
	System.p(methods.getSize().toString() + " methods supported");
	System.p(methods);
}

main();
