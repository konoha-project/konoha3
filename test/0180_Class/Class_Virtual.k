// written by kimio kuramitsu

import("konoha.new");
import("Syntax.JavaStyleClass");

class C;

class C {
	int x;
}

void test() {
	C c = new C();
	assert(c.x == 0);
}

test();


