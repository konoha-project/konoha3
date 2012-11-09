// written by kimio kuramitsu

import("konoha.new");
import("cstyle"); // konoha.null is obsolate; use cstyle
import("konoha.class");

class C {
	C self;
}

void test() {
	C c = new C();
	assert(c.self.isNull());
}

test();

