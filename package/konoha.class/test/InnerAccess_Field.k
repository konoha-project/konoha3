// written by kimio kuramitsu

import("konoha.assign");
import("konoha.class");

class C {
	int x;
	int y;
	void f() {
		x = 1;
		y = 2;
	}

	void g() {
		int y = 0;
		x = 2;
		y = 4;
	}
}

void test() {
	C c = new C();
	c.f();
	assert(c.x == 1);
	assert(c.y == 2);
	c.g();
	assert(c.x == 2);
	assert(c.y == 2);
}

test();

		
