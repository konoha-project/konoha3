// written by kimio kuramitsu

import("konoha.assign");
import("konoha.class");

class C {
	int x;
}

void test() {
	C c = new C();
	assert(c.x == 0);
	c.x += 1;
	assert(c.x == 1);
}

test();



