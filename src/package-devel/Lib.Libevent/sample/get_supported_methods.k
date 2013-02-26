import("Type.Bytes");
import("JavaScript.Array");
import("Lib.Libevent");

void main() {
	Array methods = cevent_base.get_supported_methods();
	System.p(methods.getSize().toString() + " methods supported");
	System.p(methods);
}

main();
