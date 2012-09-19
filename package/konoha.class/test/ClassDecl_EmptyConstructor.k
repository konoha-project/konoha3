// written by shunsuke shida
// modified by kimio kuramitsu

import("konoha.class");

class C {
	int age;
	C() {
	}
}

void test() {
	C c = new C();
	assert(c.age == 0);
}

test();

