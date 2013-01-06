// written by kimio kuramitsu

import("konoha.new");
import("Syntax.JavaStyleClass");

class C {
	String name = "naruto";
}

void test() {
	C c = new C();
	assert(c.name == "naruto");
}
test();

