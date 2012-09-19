// written by kimio kuramitsu

import("konoha.assign");
import("konoha.class");

class C {
	int x;
}

void test() {
	C c = new C();
	c.x = 10;
	assert(c.x == 10);
	c.x = 0;
	assert(c.x == 0);
}

test();

