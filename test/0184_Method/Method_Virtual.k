// written by kimio kuramitsu

import("Syntax.JavaStyleClass");

class C {
	int x;
	C() {}
	@Virtual int succ() {
		return x;
	}
}

void test() {
	C c = new C();
	assert(c.succ() == 0);
}

test();

