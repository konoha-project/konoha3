// written by kimio kuramitsu

import("konoha.new");
import("konoha.null");
import("konoha.class");

class C {
	C self;
}

void test() {
	C c = new C();
	assert(c.self.isNull());
}

test();

