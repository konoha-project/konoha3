// written by kimio kuramitsu

//import("konoha.new"); Don't use this
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

