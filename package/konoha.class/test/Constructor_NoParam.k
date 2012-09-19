// written by kimio kuramitsu

import("konoha.class");
import("konoha.assign");

class C {
	int x;
	C() {
		x = 1;
	}
}

void test() {
	C c = new C();
	assert(c.x==1);
}

test();


