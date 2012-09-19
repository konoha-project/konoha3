// written by kimio kuramitsu

import("konoha.assign");
import("konoha.class");

class C {
	int x;

	void test() {
		x = 10;
		assert(x == 10);
		x = 0;
		assert(x == 0);
	}
}

void test() {
	C c = new C();
	c.test();
}

test();

