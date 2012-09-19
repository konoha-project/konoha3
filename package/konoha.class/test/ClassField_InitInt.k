// written by kimio kuramitsu

import("konoha.class");

class C {
	int age = 10;
}

void test() {
	C c = new C();
	assert(c.age == 10);
}
test();

