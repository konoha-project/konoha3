// written by kimio kuramitsu

import("konoha.new");
import("Syntax.Null");
import("Syntax.JavaStyleClass");

class C {
	C self;
}

void test() {
	C c = new C();
	assert(c.self.isNull());
}

test();

