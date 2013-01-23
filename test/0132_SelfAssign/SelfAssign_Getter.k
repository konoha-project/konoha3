// written by kimio kuramitsu

import("Syntax.JavaStyleClass");
import("cstyle");

class C {
	int x;
	C() {}
}
void test() {
	C c = new C();
	assert(c.x == 0);
	c.x += 1;
	assert(c.x == 1);
}

test();
