// written by kimio kuramitsu

import("konoha.class");

class C {
	String name = "naruto";
}

void test() {
	C c = new C();
	assert(c.name == "naruto");
}
test();

