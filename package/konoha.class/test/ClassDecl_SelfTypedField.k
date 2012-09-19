// written by kimio kuramitsu

import("konoha.class");
import("konoha.null");

class C {
	C self;
}

void test() {
	C c = new C();
	assert(c.self.isNull());
}

test();

