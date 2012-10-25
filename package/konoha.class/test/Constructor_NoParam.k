// written by kimio kuramitsu

import("konoha.new");
import("konoha.class");

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


